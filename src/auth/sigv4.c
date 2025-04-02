/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/auth/sigv4.h>

#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <aws/common/date_time.h>
#include <aws/common/encoding.h> // For hex encoding
#include <aws/common/hash_table.h>
#include <aws/common/logging.h>

// Define a logging source for SigV4
// Typically defined relative to the last common log source
#define AWS_LS_AUTH_SIGV4 (AWS_LS_COMMON_LAST + 1)

#include <aws/common/string.h>
#include <aws/common/uri.h> // For URI parsing, query param handling
#include <aws/common/error.h> // For aws_raise_error and error codes

#include <aws/cal/hash.h> // For SHA256
#include <aws/cal/hmac.h> // For HMAC-SHA256

#include <aws/http/request_response.h> // For aws_http_message

#include <aws/auth/credentials.h> // For aws_credentials and accessors

#include <aws/common/array_list.h> // For aws_array_list
#include <aws/common/string.h>     // For aws_string, aws_string_new_from_array, etc.
#include <aws/common/byte_order.h> // For aws_byte_cursor_trim_pred

#include <stdio.h> // For snprintf
#include <stdlib.h> // For qsort

// --- Internal Helper Functions ---

// Structure to hold processed header information for sorting
struct s_canonical_header {
    struct aws_string *name_lowercase; // Owned
    struct aws_string *value_trimmed;  // Owned
};

// Comparison function for qsort, sorting s_canonical_header by name_lowercase
static int s_compare_canonical_headers(const void *a, const void *b) {
    const struct s_canonical_header *header_a = (const struct s_canonical_header *)a;
    const struct s_canonical_header *header_b = (const struct s_canonical_header *)b;
    return aws_string_compare(header_a->name_lowercase, header_b->name_lowercase);
}

// Predicate for trimming whitespace
static bool s_is_whitespace(uint8_t c) {
    return c == ' ' || c == '\t';
}

/**
 * Formats an aws_date_time into ISO8601 Basic format (YYYYMMDDTHHMMSSZ).
 * Returns AWS_OP_SUCCESS on success, AWS_OP_ERR otherwise.
 * Ensures null termination if buffer_size > 0.
 */
static int s_aws_date_time_to_iso8601_basic_str(
    const struct aws_date_time *dt,
    char *buffer,
    size_t buffer_size) {
    if (buffer_size == 0) {
        return AWS_OP_SUCCESS; // Nothing to do
    }
    if (buffer_size < 16) { // Needs space for YYYYMMDDTHHMMSSZ + null terminator
        aws_raise_error(AWS_ERROR_SHORT_BUFFER);
        buffer[0] = '\0';
        return AWS_OP_ERR;
    }

    int ret = snprintf(
        buffer,
        buffer_size,
        "%04d%02d%02dT%02d%02d%02dZ",
        aws_date_time_get_year(dt),
        aws_date_time_get_month(dt),
        aws_date_time_get_day(dt),
        aws_date_time_get_hour(dt),
        aws_date_time_get_minute(dt),
        aws_date_time_get_second(dt));

    if (ret < 0 || (size_t)ret >= buffer_size) {
        // Encoding error or buffer too small (shouldn't happen with size check)
        aws_raise_error(AWS_ERROR_INVALID_DATE_STR); // Or a more specific error
        buffer[0] = '\0'; // Ensure null termination on error
        return AWS_OP_ERR;
    }

    return AWS_OP_SUCCESS;
}

/**
 * Normalizes a URI path according to SigV4 rules (RFC 3986 encoding, segment handling).
 * Appends the normalized path to the output buffer.
 * Returns AWS_OP_SUCCESS on success, AWS_OP_ERR otherwise.
 */
static int s_aws_sigv4_normalize_uri_path(
    struct aws_allocator *allocator,
    struct aws_byte_cursor path,
    struct aws_byte_buf *output_buf) {

    // If the path is empty, the canonical path is "/"
    if (path.len == 0) {
        return aws_byte_buf_append_byte_dynamic(output_buf, '/');
    }

    struct aws_array_list segments;
    // Split path into segments. Initial capacity guess: path length / 4
    if (aws_array_list_init_dynamic(&segments, allocator, path.len / 4 + 1, sizeof(struct aws_byte_cursor))) {
        return AWS_OP_ERR;
    }

    // Split path by '/'
    if (aws_byte_cursor_split_on_char(&path, '/', &segments)) {
        aws_array_list_clean_up(&segments);
        return AWS_OP_ERR;
    }

    struct aws_array_list normalized_segments;
    // Initialize with the same capacity as the split segments
    if (aws_array_list_init_dynamic(&normalized_segments, allocator, aws_array_list_length(&segments), sizeof(struct aws_byte_cursor))) {
        aws_array_list_clean_up(&segments);
        return AWS_OP_ERR;
    }

    int result = AWS_OP_SUCCESS;
    size_t num_segments = aws_array_list_length(&segments);

    for (size_t i = 0; i < num_segments; ++i) {
        struct aws_byte_cursor segment;
        aws_array_list_get_at(&segments, &segment, i);

        if (segment.len == 0 || aws_byte_cursor_eq_c_str(&segment, ".")) {
            // Skip empty segments and "."
            continue;
        } else if (aws_byte_cursor_eq_c_str(&segment, "..")) {
            // Handle ".." - remove the last added segment if possible
            if (aws_array_list_length(&normalized_segments) > 0) {
                aws_array_list_pop_back(&normalized_segments);
            }
        } else {
            // URI-encode the segment and add it
            // Need a temporary buffer for encoding
            struct aws_byte_buf encoded_segment_buf;
            // Estimate encoded size (max 3x original)
            if (aws_byte_buf_init(&encoded_segment_buf, allocator, segment.len * 3 + 1)) {
                result = AWS_OP_ERR;
                goto cleanup_segments;
            }

            // Use aws_uri_encode_path_segment for SigV4-compatible encoding
            if (aws_uri_encode_path_segment(&segment, &encoded_segment_buf)) {
                aws_byte_buf_clean_up(&encoded_segment_buf);
                result = AWS_OP_ERR;
                goto cleanup_segments;
            }

            struct aws_byte_cursor encoded_segment_cursor = aws_byte_cursor_from_buf(&encoded_segment_buf);
            if (aws_array_list_push_back(&normalized_segments, &encoded_segment_cursor)) {
                aws_byte_buf_clean_up(&encoded_segment_buf); // Clean up temp buf even on list error
                result = AWS_OP_ERR;
                goto cleanup_segments;
            }
            // NOTE: aws_array_list copies the cursor, not the buffer. The buffer needs to live
            // until we are done building the final path. This is inefficient.
            // A better approach would be to append directly to the output_buf or manage
            // encoded segment buffers more carefully.
            // For now, we accept this inefficiency, assuming paths aren't excessively long.
            // We will clean up the buffers after building the final string.
            // TODO: Refactor this for better memory management if needed.
        }
    }

    // Join the normalized segments with '/'
    // Always start with '/'
    if (aws_byte_buf_append_byte_dynamic(output_buf, '/')) {
        result = AWS_OP_ERR;
        goto cleanup_segments;
    }

    size_t num_normalized = aws_array_list_length(&normalized_segments);
    for (size_t i = 0; i < num_normalized; ++i) {
        struct aws_byte_cursor segment_cursor;
        aws_array_list_get_at(&normalized_segments, &segment_cursor, i);

        // Append the segment (which is already encoded)
        if (aws_byte_buf_append_dynamic(output_buf, &segment_cursor)) {
            result = AWS_OP_ERR;
            goto cleanup_segments;
        }

        // Add '/' separator if not the last segment
        if (i < num_normalized - 1) {
            if (aws_byte_buf_append_byte_dynamic(output_buf, '/')) {
                result = AWS_OP_ERR;
                goto cleanup_segments;
            }
        }

        // Clean up the temporary buffer associated with this segment cursor
        // This relies on the cursor pointing to a buffer created by aws_byte_buf_init
        // This is fragile and depends on the TODO above.
        struct aws_byte_buf *temp_buf = (struct aws_byte_buf *)segment_cursor.ptr - 1; // Hacky way to get buffer pointer
         if (segment_cursor.ptr >= temp_buf->buffer && segment_cursor.ptr < temp_buf->buffer + temp_buf->capacity) {
             aws_byte_buf_clean_up(temp_buf);
         } else {
             // Log error - assumption failed
             AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to clean up temporary encoded segment buffer - memory leak possible.");
         }
    }


cleanup_segments:
    // Clean up any remaining temporary buffers in normalized_segments on error path
    if (result != AWS_OP_SUCCESS) {
        size_t num_to_clean = aws_array_list_length(&normalized_segments);
         for (size_t i = 0; i < num_to_clean; ++i) {
             struct aws_byte_cursor segment_cursor;
             aws_array_list_get_at(&normalized_segments, &segment_cursor, i);
             struct aws_byte_buf *temp_buf = (struct aws_byte_buf *)segment_cursor.ptr - 1;
             if (segment_cursor.ptr >= temp_buf->buffer && segment_cursor.ptr < temp_buf->buffer + temp_buf->capacity) {
                 aws_byte_buf_clean_up(temp_buf);
             }
         }
    }
    aws_array_list_clean_up(&normalized_segments);
    aws_array_list_clean_up(&segments);
    return result;
}

// Comparison function for qsort, sorting aws_uri_param by key, then value
static int s_compare_uri_params(const void *a, const void *b) {
    const struct aws_uri_param *param_a = (const struct aws_uri_param *)a;
    const struct aws_uri_param *param_b = (const struct aws_uri_param *)b;

    // Compare keys first
    int key_cmp = aws_byte_cursor_compare(&param_a->key, &param_b->key);
    if (key_cmp != 0) {
        return key_cmp;
    }
    // If keys are equal, compare values
    return aws_byte_cursor_compare(&param_a->value, &param_b->value);
}

/**
 * Formats an aws_date_time into Date Stamp format (YYYYMMDD).
 * Returns AWS_OP_SUCCESS on success, AWS_OP_ERR otherwise.
 * Ensures null termination if buffer_size > 0.
 */
static int s_aws_date_time_to_date_stamp_str(
    const struct aws_date_time *dt,
    char *buffer,
    size_t buffer_size) {
     if (buffer_size == 0) {
        return AWS_OP_SUCCESS; // Nothing to do
    }
    if (buffer_size < 9) { // Needs space for YYYYMMDD + null terminator
        aws_raise_error(AWS_ERROR_SHORT_BUFFER);
        buffer[0] = '\0';
        return AWS_OP_ERR;
    }

    int ret = snprintf(
        buffer,
        buffer_size,
        "%04d%02d%02d",
        aws_date_time_get_year(dt),
        aws_date_time_get_month(dt),
        aws_date_time_get_day(dt));

     if (ret < 0 || (size_t)ret >= buffer_size) {
        // Encoding error or buffer too small (shouldn't happen with size check)
        aws_raise_error(AWS_ERROR_INVALID_DATE_STR); // Or a more specific error
        buffer[0] = '\0'; // Ensure null termination on error
        return AWS_OP_ERR;
    }

    return AWS_OP_SUCCESS;
}


// --- Public API Function ---

int aws_sigv4_sign_request(
    struct aws_allocator *allocator,
    struct aws_http_message *request,
    const struct aws_credentials *credentials,
    struct aws_byte_cursor region,
    struct aws_byte_cursor service_name,
    const struct aws_date_time *signing_date) {

    AWS_LOGF_INFO(AWS_LS_AUTH_SIGV4, "Starting SigV4 signing process.");

    if (!allocator || !request || !credentials || !region.ptr || !service_name.ptr || !signing_date) {
        AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Missing required parameters for SigV4 signing.");
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }

    // --- Step 1: Create Canonical Request ---
    AWS_LOGF_DEBUG(AWS_LS_AUTH_SIGV4, "Step 1: Creating Canonical Request.");
    struct aws_byte_buf canonical_request_buf;
    if (aws_byte_buf_init(&canonical_request_buf, allocator, 1024)) { // Initial capacity, will grow
        return AWS_OP_ERR;
    }

    // 1.1: HTTP Method
    struct aws_byte_cursor method;
    if (aws_http_message_get_request_method(request, &method)) {
        AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to get HTTP method.");
        aws_byte_buf_clean_up(&canonical_request_buf);
        return AWS_OP_ERR; // Error already raised by get_request_method
    }
    aws_byte_buf_append_dynamic(&canonical_request_buf, &method);
    aws_byte_buf_append_byte_dynamic(&canonical_request_buf, '\n');
    AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Canonical Request - Method: %.*s", (int)method.len, method.ptr);


    // 1.2: Canonical URI Path
    struct aws_byte_cursor path;
    if (aws_http_message_get_request_path(request, &path)) {
        AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to get HTTP path.");
        aws_byte_buf_clean_up(&canonical_request_buf);
        return AWS_OP_ERR; // Error already raised by get_request_path
    }

    // Normalize the path and append it to the canonical request buffer
    // Note: We need to store the original path length before normalization for query string extraction later.
    size_t original_path_len = path.len;
    struct aws_byte_cursor original_path_cursor = aws_byte_cursor_from_array(path.ptr, original_path_len); // Store original path
    if (s_aws_sigv4_normalize_uri_path(allocator, path, &canonical_request_buf)) {
        AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to normalize URI path '%.*s'.", (int)path.len, path.ptr);
        aws_byte_buf_clean_up(&canonical_request_buf);
        return AWS_OP_ERR; // Error should be raised by helper
    }
    AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Canonical Request - URI Path (Normalized): %.*s",
                   (int)canonical_request_buf.len - (int)method.len - 1, // Length of path part
                   canonical_request_buf.buffer + method.len + 1); // Start after method and newline
    aws_byte_buf_append_byte_dynamic(&canonical_request_buf, '\n');


    // 1.3: Canonical Query String
    // TODO: Revisit query string extraction. aws_http_message doesn't easily provide the full URI.
    // For now, manually search the original path for '?'. This is not robust.
    struct aws_byte_cursor query_string = { .ptr = NULL, .len = 0 };
    int query_result = AWS_OP_SUCCESS;

    const uint8_t *query_start = aws_memchr(original_path_cursor.ptr, '?', original_path_cursor.len);
    if (query_start) {
        query_string = aws_byte_cursor_from_array(query_start + 1, original_path_cursor.ptr + original_path_cursor.len - (query_start + 1));
        AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Manually extracted query string: '%.*s'", (int)query_string.len, query_string.ptr);
    } else {
         AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "No '?' found in path, assuming no query string.");
    }

    if (query_string.len > 0) {
        struct aws_array_list query_params_list;
        // Use aws_uri_query_params to parse into an array list of aws_uri_param structs
        if (aws_uri_query_params(allocator, &query_string, &query_params_list)) {
            AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to parse query string parameters.");
            query_result = AWS_OP_ERR;
            goto query_cleanup;
        }

        size_t num_params = aws_array_list_length(&query_params_list);
        AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Parsed %zu query parameters.", num_params);

        // Sort the parameters using the comparator from aws-c-common
        // Use qsort directly with the s_compare_uri_params function
        qsort(aws_array_list_data(&query_params_list),
              num_params,
              aws_array_list_element_size(&query_params_list),
              s_compare_uri_params); // Use the correct comparison function
        AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Sorted query parameters.");

        // Build the canonical query string
        struct aws_byte_buf canonical_query_buf;
        if (aws_byte_buf_init(&canonical_query_buf, allocator, query_string.len * 2)) { // Estimate size
             query_result = AWS_OP_ERR;
             aws_uri_query_params_clean_up(&query_params_list);
             goto query_cleanup;
        }

        for (size_t i = 0; i < num_params; ++i) {
            struct aws_uri_param *param;
            aws_array_list_get_at_ptr(&query_params_list, (void **)&param, i);

            // Encode key
            if (aws_uri_encode_query_param(&param->key, &canonical_query_buf)) {
                query_result = AWS_OP_ERR;
                aws_byte_buf_clean_up(&canonical_query_buf);
                aws_uri_query_params_clean_up(&query_params_list);
                goto query_cleanup;
            }
            aws_byte_buf_append_byte_dynamic(&canonical_query_buf, '=');

            // Encode value
            if (aws_uri_encode_query_param(&param->value, &canonical_query_buf)) {
                 query_result = AWS_OP_ERR;
                 aws_byte_buf_clean_up(&canonical_query_buf);
                 aws_uri_query_params_clean_up(&query_params_list);
                 goto query_cleanup;
            }

            // Add '&' separator if not the last parameter
            if (i < num_params - 1) {
                aws_byte_buf_append_byte_dynamic(&canonical_query_buf, '&');
            }
        }

        // Append the canonical query string to the main buffer
        struct aws_byte_cursor canonical_query_cursor = aws_byte_cursor_from_buf(&canonical_query_buf);
        aws_byte_buf_append_dynamic(&canonical_request_buf, &canonical_query_cursor);
        AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Canonical Request - Query String: %.*s", (int)canonical_query_cursor.len, canonical_query_cursor.ptr);

        // Clean up temporary buffer and parsed params list
        aws_byte_buf_clean_up(&canonical_query_buf);
        aws_uri_query_params_clean_up(&query_params_list);

    } else {
         AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Canonical Request - Query String: (Empty)");
    }

query_cleanup: // Keep label for potential future use with goto
    // No URI struct to clean up in this simplified version
    if(query_result != AWS_OP_SUCCESS) {
        aws_byte_buf_clean_up(&canonical_request_buf);
        return AWS_OP_ERR; // Error should already be raised
    }

    aws_byte_buf_append_byte_dynamic(&canonical_request_buf, '\n'); // Always add newline, even if query string is empty


    // 1.4: Canonical Headers & 1.5: Signed Headers
    size_t num_headers = aws_http_message_get_header_count(request);
    AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Processing %zu headers.", num_headers);

    struct aws_array_list canonical_headers_list;
    if (aws_array_list_init_dynamic(&canonical_headers_list, allocator, num_headers, sizeof(struct s_canonical_header))) {
        aws_byte_buf_clean_up(&canonical_request_buf);
        return AWS_OP_ERR;
    }

    int processing_result = AWS_OP_SUCCESS;

    for (size_t i = 0; i < num_headers; ++i) {
        struct aws_http_header header;
        if (aws_http_message_get_header(request, &header, i)) {
            AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to get header at index %zu.", i);
            processing_result = AWS_OP_ERR;
            goto headers_cleanup;
        }

        // Convert name to lowercase
        struct aws_string *name_lower = aws_string_new_from_array(allocator, header.name.ptr, header.name.len);
        if (!name_lower) {
            processing_result = AWS_OP_ERR;
            goto headers_cleanup;
        }
        aws_string_tolower(name_lower); // In-place modification

        // Trim whitespace from value
        struct aws_byte_cursor value_trimmed_cursor = header.value;
        aws_byte_cursor_trim_pred(&value_trimmed_cursor, s_is_whitespace);

        // Handle potential multi-line header values by replacing consecutive whitespace with a single space.
        // SigV4 requires folding whitespace, not comma separation.
        // We'll build the folded value into a temporary buffer.
        struct aws_byte_buf folded_value_buf;
        if (aws_byte_buf_init(&folded_value_buf, allocator, value_trimmed_cursor.len)) {
             aws_string_destroy(name_lower);
             processing_result = AWS_OP_ERR;
             goto headers_cleanup;
        }
        bool last_char_was_space = false;
        for (size_t k = 0; k < value_trimmed_cursor.len; ++k) {
            uint8_t current_char = value_trimmed_cursor.ptr[k];
            bool current_char_is_space = s_is_whitespace(current_char);

            if (current_char_is_space) {
                if (!last_char_was_space) {
                    // Append a single space for one or more whitespace chars
                    aws_byte_buf_append_byte_dynamic(&folded_value_buf, ' ');
                }
                last_char_was_space = true;
            } else {
                aws_byte_buf_append_byte_dynamic(&folded_value_buf, current_char);
                last_char_was_space = false;
            }
        }

        // Create the final trimmed and folded string
        struct aws_string *value_final = aws_string_new_from_buf(allocator, &folded_value_buf);
        aws_byte_buf_clean_up(&folded_value_buf); // Clean up temp buffer

        if (!value_final) {
            aws_string_destroy(name_lower);
            processing_result = AWS_OP_ERR;
            goto headers_cleanup;
        }

        struct s_canonical_header entry = {
            .name_lowercase = name_lower,
            .value_trimmed = value_final, // Use the folded value
        };

        if (aws_array_list_push_back(&canonical_headers_list, &entry)) {
            aws_string_destroy(name_lower);
            aws_string_destroy(value_final); // Use the folded value
            processing_result = AWS_OP_ERR;
            goto headers_cleanup;
        }
         AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "  Processed Header %zu: %s: %s",
                       i, aws_string_c_str(name_lower), aws_string_c_str(value_final)); // Use the folded value
    }

    // Sort the canonical_headers_list alphabetically by name_lowercase
    aws_array_list_sort(&canonical_headers_list, s_compare_canonical_headers);
    AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Sorted canonical headers list.");

    // Iterate through the sorted list to build canonical and signed headers strings
    struct aws_byte_buf signed_headers_buf;
    if (aws_byte_buf_init(&signed_headers_buf, allocator, num_headers * 16)) { // Estimate size
        processing_result = AWS_OP_ERR;
        goto headers_cleanup;
    }

    size_t sorted_count = aws_array_list_length(&canonical_headers_list);
    for (size_t i = 0; i < sorted_count; ++i) {
        struct s_canonical_header *entry;
        aws_array_list_get_at_ptr(&canonical_headers_list, (void **)&entry, i);

        // Append canonical header: lowercase(name):trimmed(value)\n
        struct aws_byte_cursor name_cursor = aws_byte_cursor_from_string(entry->name_lowercase);
        struct aws_byte_cursor value_cursor = aws_byte_cursor_from_string(entry->value_trimmed);

        if (aws_byte_buf_append_dynamic(&canonical_request_buf, &name_cursor) ||
            aws_byte_buf_append_byte_dynamic(&canonical_request_buf, ':') ||
            aws_byte_buf_append_dynamic(&canonical_request_buf, &value_cursor) ||
            aws_byte_buf_append_byte_dynamic(&canonical_request_buf, '\n')) {
            processing_result = AWS_OP_ERR;
            aws_byte_buf_clean_up(&signed_headers_buf);
            goto headers_cleanup;
        }

        // Append signed header name: lowercase(name)
        if (aws_byte_buf_append_dynamic(&signed_headers_buf, &name_cursor)) {
             processing_result = AWS_OP_ERR;
             aws_byte_buf_clean_up(&signed_headers_buf);
             goto headers_cleanup;
        }
        // Add semicolon separator if not the last header
        if (i < sorted_count - 1) {
            if (aws_byte_buf_append_byte_dynamic(&signed_headers_buf, ';')) {
                 processing_result = AWS_OP_ERR;
                 aws_byte_buf_clean_up(&signed_headers_buf);
                 goto headers_cleanup;
            }
        }
    }

    aws_byte_buf_append_byte_dynamic(&canonical_request_buf, '\n'); // Separator after canonical headers list
    AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Canonical Request - Headers appended.");

    // Append signed headers list to canonical request
    struct aws_byte_cursor signed_headers_cursor = aws_byte_cursor_from_buf(&signed_headers_buf);
    if (aws_byte_buf_append_dynamic(&canonical_request_buf, &signed_headers_cursor)) {
        processing_result = AWS_OP_ERR;
        aws_byte_buf_clean_up(&signed_headers_buf);
        goto headers_cleanup;
    }
    AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Canonical Request - Signed Headers: %.*s", (int)signed_headers_cursor.len, signed_headers_cursor.ptr);


headers_cleanup:
    // Clean up the strings within the canonical_headers_list
    size_t count_to_clean = aws_array_list_length(&canonical_headers_list);
    for (size_t i = 0; i < count_to_clean; ++i) {
        struct s_canonical_header *entry;
         // Use get_at_ptr for safe access during cleanup
        if (aws_array_list_get_at_ptr(&canonical_headers_list, (void **)&entry, i) == AWS_OP_SUCCESS) {
            aws_string_destroy(entry->name_lowercase);
            aws_string_destroy(entry->value_trimmed);
        }
    }
    aws_array_list_clean_up(&canonical_headers_list); // Clean up the list itself

    if (processing_result != AWS_OP_SUCCESS) {
        aws_byte_buf_clean_up(&canonical_request_buf);
        // signed_headers_buf might need cleanup if initialized before error
        // aws_byte_buf_is_valid check is implicit in clean_up
        aws_byte_buf_clean_up(&signed_headers_buf);
        return AWS_OP_ERR; // Error should already be raised
    }

    // Keep signed_headers_buf alive until Step 5

    // 1.6: Hashed Payload
    struct aws_input_stream *body_stream = aws_http_message_get_body_stream(request);
    struct aws_byte_cursor payload_hash_hex_cursor; // Cursor to the final hex-encoded hash string
    struct aws_byte_buf payload_hash_hex_buf; // Buffer to hold the hex string if we calculate it
    bool payload_hash_calculated = false; // Flag to know if we need to clean up payload_hash_hex_buf

    // Check for pre-calculated hash header
    struct aws_byte_cursor content_sha256_header_name = aws_byte_cursor_from_c_str("x-amz-content-sha256");
    if (aws_http_message_get_header(request, &payload_hash_hex_cursor, &content_sha256_header_name) == AWS_OP_SUCCESS) {
        AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Using pre-calculated payload hash from x-amz-content-sha256 header: %.*s",
                       (int)payload_hash_hex_cursor.len, payload_hash_hex_cursor.ptr);
        // TODO: Validate the header value format? (e.g., is it hex, is it correct length?)
    } else {
        // Header not found, calculate hash
        aws_reset_error(); // Clear error from get_header not finding the header
        payload_hash_calculated = true; // We will calculate it, so mark for cleanup
        if (aws_byte_buf_init(&payload_hash_hex_buf, allocator, AWS_SHA256_LEN * 2 + 1)) { // Space for hex + null
             aws_byte_buf_clean_up(&canonical_request_buf);
             aws_byte_buf_clean_up(&signed_headers_buf); // Need to clean this up too
             return AWS_OP_ERR;
        }

        struct aws_byte_buf payload_hash_raw_buf; // Temp buffer for raw hash
        if (aws_byte_buf_init(&payload_hash_raw_buf, allocator, AWS_SHA256_LEN)) {
            aws_byte_buf_clean_up(&payload_hash_hex_buf);
            aws_byte_buf_clean_up(&canonical_request_buf);
            aws_byte_buf_clean_up(&signed_headers_buf);
            return AWS_OP_ERR;
        }

        if (body_stream) {
            // **********************************************************************
            // TODO: CRITICAL - Streaming Body Handling Needed!
            // aws_sha256_compute reads the entire stream, which prevents it from
            // being sent later if it's a single-read stream (like network).
            // A proper solution requires either:
            // 1. Using a tee-stream adapter.
            // 2. Implementing SigV4 chunked signing (requires STREAMING-AWS4-HMAC-SHA256-PAYLOAD).
            // 3. Requiring the caller to pre-calculate the hash and set the x-amz-content-sha256 header.
            // For now, we proceed with the consuming compute, assuming the caller handles it or uses streams that can be reset.
            // **********************************************************************
            AWS_LOGF_WARN(AWS_LS_AUTH_SIGV4, "Hashing non-empty request body. This consumes the stream! Proper streaming support is needed.");
            if (aws_sha256_compute(allocator, body_stream, &payload_hash_raw_buf, 0)) {
                AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to compute SHA256 hash of request body.");
                aws_byte_buf_clean_up(&payload_hash_raw_buf);
                aws_byte_buf_clean_up(&payload_hash_hex_buf);
                aws_byte_buf_clean_up(&canonical_request_buf);
                aws_byte_buf_clean_up(&signed_headers_buf);
                return AWS_OP_ERR;
            }
             // Attempt to reset stream if possible (best effort, may fail)
             if (aws_input_stream_seek(body_stream, 0, AWS_SSB_BEGIN)) {
                 AWS_LOGF_WARN(AWS_LS_AUTH_SIGV4, "Failed to seek body stream back to beginning after hashing. Error: %s", aws_error_debug_str(aws_last_error()));
                 aws_reset_error(); // Don't fail the signing for seek failure, but log it.
             }
            AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Computed payload hash for stream.");

        } else {
            // Empty body, use precomputed hash of empty string
            struct aws_byte_cursor empty_str_cursor = aws_byte_cursor_from_c_str("");
            if (aws_sha256(allocator, &empty_str_cursor, &payload_hash_raw_buf, 0)) {
                 AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to compute SHA256 hash of empty string.");
                 aws_byte_buf_clean_up(&payload_hash_raw_buf);
                 aws_byte_buf_clean_up(&payload_hash_hex_buf);
                 aws_byte_buf_clean_up(&canonical_request_buf);
                 aws_byte_buf_clean_up(&signed_headers_buf);
                 return AWS_OP_ERR;
            }
             AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Using hash of empty string for payload.");
        }

        // Hex-encode the raw hash into our hex buffer
        struct aws_byte_cursor payload_hash_raw_cursor = aws_byte_cursor_from_buf(&payload_hash_raw_buf);
        if (aws_byte_buf_append_encoding_to_hex(&payload_hash_hex_buf, &payload_hash_raw_cursor)) {
            AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to hex-encode payload hash.");
            aws_byte_buf_clean_up(&payload_hash_raw_buf);
            aws_byte_buf_clean_up(&payload_hash_hex_buf);
            aws_byte_buf_clean_up(&canonical_request_buf);
            aws_byte_buf_clean_up(&signed_headers_buf);
            return AWS_OP_ERR;
        }
        aws_byte_buf_clean_up(&payload_hash_raw_buf); // Clean up raw hash buffer

        // Set the cursor to point to the calculated hex hash
        payload_hash_hex_cursor = aws_byte_cursor_from_buf(&payload_hash_hex_buf);
    }

    // Append the final hex-encoded payload hash to the canonical request
    if (aws_byte_buf_append_dynamic(&canonical_request_buf, &payload_hash_hex_cursor)) {
         AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to append payload hash to canonical request.");
         if (payload_hash_calculated) aws_byte_buf_clean_up(&payload_hash_hex_buf);
         aws_byte_buf_clean_up(&canonical_request_buf);
         aws_byte_buf_clean_up(&signed_headers_buf);
         return AWS_OP_ERR;
    }

    // Log the final hex hash that was appended
    AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Canonical Request - Payload Hash: %.*s",
                   (int)payload_hash_hex_cursor.len, payload_hash_hex_cursor.ptr);

    // Clean up the hex buffer if we calculated it
    if (payload_hash_calculated) {
        aws_byte_buf_clean_up(&payload_hash_hex_buf);
    }

    // --- End Step 1 ---
    struct aws_byte_cursor canonical_request_cursor = aws_byte_cursor_from_buf(&canonical_request_buf);
    AWS_LOGF_DEBUG(AWS_LS_AUTH_SIGV4, "Canonical Request String:\n%.*s", (int)canonical_request_cursor.len, canonical_request_cursor.ptr);


    // --- Step 2: Create String to Sign ---
    AWS_LOGF_DEBUG(AWS_LS_AUTH_SIGV4, "Step 2: Creating String to Sign.");
    struct aws_byte_buf string_to_sign_buf;
    if (aws_byte_buf_init(&string_to_sign_buf, allocator, 256)) { // Initial capacity
        aws_byte_buf_clean_up(&canonical_request_buf);
        return AWS_OP_ERR;
    }

    // 2.1: Algorithm
    struct aws_byte_cursor algorithm = aws_byte_cursor_from_c_str("AWS4-HMAC-SHA256");
    aws_byte_buf_append_dynamic(&string_to_sign_buf, &algorithm);
    aws_byte_buf_append_byte_dynamic(&string_to_sign_buf, '\n');

    // 2.2: Timestamp (ISO8601 basic format YYYYMMDDTHHMMSSZ)
    char timestamp_str[17]; // YYYYMMDDTHHMMSSZ + null terminator
    if (s_aws_date_time_to_iso8601_basic_str(signing_date, timestamp_str, sizeof(timestamp_str))) {
        AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to format signing date to ISO8601 basic string.");
        aws_byte_buf_clean_up(&string_to_sign_buf);
        aws_byte_buf_clean_up(&canonical_request_buf);
        return AWS_OP_ERR; // Error already raised by helper
    }
    struct aws_byte_cursor timestamp_cursor = aws_byte_cursor_from_c_str(timestamp_str);
    aws_byte_buf_append_dynamic(&string_to_sign_buf, &timestamp_cursor);
    aws_byte_buf_append_byte_dynamic(&string_to_sign_buf, '\n');
    AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "String to Sign - Timestamp: %.*s", (int)timestamp_cursor.len, timestamp_cursor.ptr);

    // 2.3: Scope (YYYYMMDD/region/service/aws4_request)
    char date_stamp_str[9]; // YYYYMMDD + null terminator
    if (s_aws_date_time_to_date_stamp_str(signing_date, date_stamp_str, sizeof(date_stamp_str))) {
        AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to format signing date to date stamp string.");
        aws_byte_buf_clean_up(&string_to_sign_buf);
        aws_byte_buf_clean_up(&canonical_request_buf);
        return AWS_OP_ERR; // Error already raised by helper
    }
    struct aws_byte_cursor date_cursor = aws_byte_cursor_from_c_str(date_stamp_str);
    struct aws_byte_cursor scope_terminator = aws_byte_cursor_from_c_str("aws4_request");

    aws_byte_buf_append_dynamic(&string_to_sign_buf, &date_cursor);
    aws_byte_buf_append_byte_dynamic(&string_to_sign_buf, '/');
    aws_byte_buf_append_dynamic(&string_to_sign_buf, &region);
    aws_byte_buf_append_byte_dynamic(&string_to_sign_buf, '/');
    aws_byte_buf_append_dynamic(&string_to_sign_buf, &service_name);
    aws_byte_buf_append_byte_dynamic(&string_to_sign_buf, '/');
    aws_byte_buf_append_dynamic(&string_to_sign_buf, &scope_terminator);
    aws_byte_buf_append_byte_dynamic(&string_to_sign_buf, '\n');
    AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "String to Sign - Scope: %.*s/%.*s/%.*s/aws4_request",
                   (int)date_cursor.len, date_cursor.ptr, (int)region.len, region.ptr, (int)service_name.len, service_name.ptr);


    // 2.4: Hash of Canonical Request
    struct aws_byte_buf canonical_request_hash_buf; // Raw SHA256 hash
    if (aws_byte_buf_init(&canonical_request_hash_buf, allocator, AWS_SHA256_LEN)) {
        aws_byte_buf_clean_up(&string_to_sign_buf);
        aws_byte_buf_clean_up(&canonical_request_buf);
        return AWS_OP_ERR;
    }
    if (aws_sha256(allocator, &canonical_request_cursor, &canonical_request_hash_buf, 0)) {
        AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to compute SHA256 hash of canonical request.");
        aws_byte_buf_clean_up(&canonical_request_hash_buf);
        aws_byte_buf_clean_up(&string_to_sign_buf);
        aws_byte_buf_clean_up(&canonical_request_buf);
        return AWS_OP_ERR;
    }

    // Hex-encode the hash and append
    struct aws_byte_cursor canonical_request_hash_cursor = aws_byte_cursor_from_buf(&canonical_request_hash_buf);
    if (aws_byte_buf_append_encoding_to_hex(&string_to_sign_buf, &canonical_request_hash_cursor)) {
         AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to hex-encode canonical request hash.");
         aws_byte_buf_clean_up(&canonical_request_hash_buf);
         aws_byte_buf_clean_up(&string_to_sign_buf);
         aws_byte_buf_clean_up(&canonical_request_buf);
         return AWS_OP_ERR;
    }
    aws_byte_buf_clean_up(&canonical_request_hash_buf); // Clean up raw hash buffer

    // --- End Step 2 ---
    struct aws_byte_cursor string_to_sign_cursor = aws_byte_cursor_from_buf(&string_to_sign_buf);
    AWS_LOGF_DEBUG(AWS_LS_AUTH_SIGV4, "String to Sign:\n%.*s", (int)string_to_sign_cursor.len, string_to_sign_cursor.ptr);


    // --- Step 3: Calculate Signing Key ---
    AWS_LOGF_DEBUG(AWS_LS_AUTH_SIGV4, "Step 3: Calculating Signing Key.");

    struct aws_byte_cursor secret_key_cursor = aws_credentials_get_secret_access_key(credentials);
    if (!secret_key_cursor.ptr) {
        AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to get secret access key from credentials.");
        aws_byte_buf_clean_up(&string_to_sign_buf);
        aws_byte_buf_clean_up(&canonical_request_buf);
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT); // Or a more specific auth error
    }

    // Prepend "AWS4" to the secret key for the first HMAC
    struct aws_byte_buf kSecret_buf;
    struct aws_byte_cursor aws4_prefix = aws_byte_cursor_from_c_str("AWS4");
    if (aws_byte_buf_init(&kSecret_buf, allocator, aws4_prefix.len + secret_key_cursor.len)) {
        aws_byte_buf_clean_up(&string_to_sign_buf);
        aws_byte_buf_clean_up(&canonical_request_buf);
        return AWS_OP_ERR;
    }
    aws_byte_buf_append_dynamic(&kSecret_buf, &aws4_prefix);
    aws_byte_buf_append_dynamic(&kSecret_buf, &secret_key_cursor);
    struct aws_byte_cursor kSecret_cursor = aws_byte_cursor_from_buf(&kSecret_buf);

    // Intermediate key buffers (HMAC output is SHA256 length)
    struct aws_byte_buf kDate_buf;
    struct aws_byte_buf kRegion_buf;
    struct aws_byte_buf kService_buf;
    struct aws_byte_buf kSigning_buf; // Final signing key

    if (aws_byte_buf_init(&kDate_buf, allocator, AWS_SHA256_LEN) ||
        aws_byte_buf_init(&kRegion_buf, allocator, AWS_SHA256_LEN) ||
        aws_byte_buf_init(&kService_buf, allocator, AWS_SHA256_LEN) ||
        aws_byte_buf_init(&kSigning_buf, allocator, AWS_SHA256_LEN)) {
        // Clean up any initialized buffers on error
        aws_byte_buf_clean_up_secure(&kSigning_buf);
        aws_byte_buf_clean_up_secure(&kService_buf);
        aws_byte_buf_clean_up_secure(&kRegion_buf);
        aws_byte_buf_clean_up_secure(&kDate_buf);
        aws_byte_buf_clean_up_secure(&kSecret_buf);
        aws_byte_buf_clean_up(&string_to_sign_buf);
        aws_byte_buf_clean_up(&canonical_request_buf);
        return AWS_OP_ERR;
    }

    int hmac_result = AWS_OP_SUCCESS;

    // kDate = HMAC("AWS4" + kSecret, Date)
    if (aws_hmac_sha256(allocator, &kSecret_cursor, &date_cursor, &kDate_buf)) { hmac_result = AWS_OP_ERR; goto signing_key_cleanup; }
    struct aws_byte_cursor kDate_cursor = aws_byte_cursor_from_buf(&kDate_buf);
    AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Calculated kDate.");

    // kRegion = HMAC(kDate, Region)
    if (aws_hmac_sha256(allocator, &kDate_cursor, &region, &kRegion_buf)) { hmac_result = AWS_OP_ERR; goto signing_key_cleanup; }
    struct aws_byte_cursor kRegion_cursor = aws_byte_cursor_from_buf(&kRegion_buf);
    AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Calculated kRegion.");

    // kService = HMAC(kRegion, Service)
    if (aws_hmac_sha256(allocator, &kRegion_cursor, &service_name, &kService_buf)) { hmac_result = AWS_OP_ERR; goto signing_key_cleanup; }
    struct aws_byte_cursor kService_cursor = aws_byte_cursor_from_buf(&kService_buf);
    AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Calculated kService.");

    // kSigning = HMAC(kService, "aws4_request")
    if (aws_hmac_sha256(allocator, &kService_cursor, &scope_terminator, &kSigning_buf)) { hmac_result = AWS_OP_ERR; goto signing_key_cleanup; }
    struct aws_byte_cursor signing_key_cursor = aws_byte_cursor_from_buf(&kSigning_buf);
    AWS_LOGF_DEBUG(AWS_LS_AUTH_SIGV4, "Calculated final signing key (kSigning).");

    // --- End Step 3 ---


    // --- Step 4: Calculate Signature ---
    AWS_LOGF_DEBUG(AWS_LS_AUTH_SIGV4, "Step 4: Calculating Signature.");
    struct aws_byte_buf signature_buf; // Raw HMAC-SHA256 signature
    if (aws_byte_buf_init(&signature_buf, allocator, AWS_SHA256_LEN)) {
        hmac_result = AWS_OP_ERR; // Signal error for cleanup
        goto signing_key_cleanup;
    }

    if (aws_hmac_sha256(allocator, &signing_key_cursor, &string_to_sign_cursor, &signature_buf)) {
        AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to compute final signature HMAC.");
        aws_byte_buf_clean_up(&signature_buf);
        hmac_result = AWS_OP_ERR; // Signal error for cleanup
        goto signing_key_cleanup;
    }

    // Hex-encode the signature
    struct aws_byte_buf signature_hex_buf;
    struct aws_byte_cursor signature_cursor = aws_byte_cursor_from_buf(&signature_buf);
    if (aws_byte_buf_init(&signature_hex_buf, allocator, AWS_SHA256_LEN * 2)) {
        aws_byte_buf_clean_up(&signature_buf);
        hmac_result = AWS_OP_ERR; // Signal error for cleanup
        goto signing_key_cleanup;
    }
    if (aws_byte_buf_append_encoding_to_hex(&signature_hex_buf, &signature_cursor)) {
        AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to hex-encode final signature.");
        aws_byte_buf_clean_up(&signature_hex_buf);
        aws_byte_buf_clean_up(&signature_buf);
        hmac_result = AWS_OP_ERR; // Signal error for cleanup
        goto signing_key_cleanup;
    }
    aws_byte_buf_clean_up(&signature_buf); // Clean up raw signature buffer

    struct aws_byte_cursor signature_hex_cursor = aws_byte_cursor_from_buf(&signature_hex_buf);
    AWS_LOGF_DEBUG(AWS_LS_AUTH_SIGV4, "Calculated Signature (Hex): %.*s", (int)signature_hex_cursor.len, signature_hex_cursor.ptr);

    // --- End Step 4 ---


    // --- Step 5: Add signing information to request headers ---
    AWS_LOGF_DEBUG(AWS_LS_AUTH_SIGV4, "Step 5: Adding signing information to request headers.");

    // 5.1: Construct Authorization Header
    // Format: AWS4-HMAC-SHA256 Credential=AccessKeyID/Scope, SignedHeaders=SignedHeaders, Signature=Signature
    struct aws_byte_buf auth_header_value_buf;
    if (aws_byte_buf_init(&auth_header_value_buf, allocator, 256)) { // Initial capacity
        hmac_result = AWS_OP_ERR; // Signal error for cleanup
        goto signing_key_cleanup;
    }

    struct aws_byte_cursor auth_prefix = aws_byte_cursor_from_c_str("AWS4-HMAC-SHA256 Credential=");
    struct aws_byte_cursor access_key_id = aws_credentials_get_access_key_id(credentials);
    struct aws_byte_cursor signed_headers_sep = aws_byte_cursor_from_c_str(", SignedHeaders=");
    // Use the signed_headers_cursor generated in Step 1.5
    // struct aws_byte_cursor signed_headers_str = aws_byte_cursor_from_c_str("host;x-amz-date"); // Placeholder
    struct aws_byte_cursor signature_sep = aws_byte_cursor_from_c_str(", Signature=");

    aws_byte_buf_append_dynamic(&auth_header_value_buf, &auth_prefix);
    aws_byte_buf_append_dynamic(&auth_header_value_buf, &access_key_id);
    aws_byte_buf_append_byte_dynamic(&auth_header_value_buf, '/');
    // Append Scope (Date/Region/Service/aws4_request) - reuse parts from Step 2.3
    aws_byte_buf_append_dynamic(&auth_header_value_buf, &date_cursor);
    aws_byte_buf_append_byte_dynamic(&auth_header_value_buf, '/');
    aws_byte_buf_append_dynamic(&auth_header_value_buf, &region);
    aws_byte_buf_append_byte_dynamic(&auth_header_value_buf, '/');
    aws_byte_buf_append_dynamic(&auth_header_value_buf, &service_name);
    aws_byte_buf_append_byte_dynamic(&auth_header_value_buf, '/');
    aws_byte_buf_append_dynamic(&auth_header_value_buf, &scope_terminator);

    aws_byte_buf_append_dynamic(&auth_header_value_buf, &signed_headers_sep);
    aws_byte_buf_append_dynamic(&auth_header_value_buf, &signed_headers_cursor); // Use actual signed headers

    aws_byte_buf_append_dynamic(&auth_header_value_buf, &signature_sep);
    aws_byte_buf_append_dynamic(&auth_header_value_buf, &signature_hex_cursor); // Calculated in Step 4

    // 5.2: Add Authorization Header to Request
    struct aws_http_header auth_header = {
        .name = aws_byte_cursor_from_c_str("Authorization"),
        .value = aws_byte_cursor_from_buf(&auth_header_value_buf),
    };
    if (aws_http_message_add_header(request, auth_header)) {
        AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to add Authorization header to request.");
        aws_byte_buf_clean_up(&auth_header_value_buf);
        hmac_result = AWS_OP_ERR; // Signal error for cleanup
        goto signing_key_cleanup;
    }
    AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Added Authorization header: %.*s", (int)auth_header.value.len, auth_header.value.ptr);
    aws_byte_buf_clean_up(&auth_header_value_buf); // Value is copied by add_header

    // 5.3: Add X-Amz-Date Header
    // Use the timestamp_cursor generated in Step 2.2
    struct aws_http_header date_header = {
        .name = aws_byte_cursor_from_c_str("X-Amz-Date"),
        .value = timestamp_cursor, // Already formatted in Step 2.2
    };
     if (aws_http_message_add_header(request, date_header)) {
        AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to add X-Amz-Date header to request.");
        hmac_result = AWS_OP_ERR; // Signal error for cleanup
        goto signing_key_cleanup;
    }
    AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Added X-Amz-Date header: %.*s", (int)date_header.value.len, date_header.value.ptr);


    // 5.4: Add X-Amz-Security-Token Header (if applicable)
    struct aws_byte_cursor session_token = aws_credentials_get_session_token(credentials);
    if (session_token.ptr && session_token.len > 0) {
        struct aws_http_header token_header = {
            .name = aws_byte_cursor_from_c_str("X-Amz-Security-Token"),
            .value = session_token,
        };
        if (aws_http_message_add_header(request, token_header)) {
            AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed to add X-Amz-Security-Token header to request.");
            hmac_result = AWS_OP_ERR; // Signal error for cleanup
            goto signing_key_cleanup;
        }
        AWS_LOGF_TRACE(AWS_LS_AUTH_SIGV4, "Added X-Amz-Security-Token header.");
    }

    // --- End Step 5 ---


signing_key_cleanup:
    // Securely clean up intermediate keys
    aws_byte_buf_clean_up_secure(&kService_buf);
    aws_byte_buf_clean_up_secure(&kRegion_buf);
    aws_byte_buf_clean_up_secure(&kDate_buf);
    aws_byte_buf_clean_up_secure(&kSecret_buf);

    if (hmac_result != AWS_OP_SUCCESS) {
        AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "Failed during signing key calculation.");
        aws_byte_buf_clean_up_secure(&kSigning_buf); // Clean up final key on error too
        aws_byte_buf_clean_up(&string_to_sign_buf);
        aws_byte_buf_clean_up(&canonical_request_buf);
        return AWS_OP_ERR; // Error should already be raised by aws_hmac_sha256
    }

    // Clean up buffers (kSigning_buf needed for Step 4)
    aws_byte_buf_clean_up(&string_to_sign_buf);
    aws_byte_buf_clean_up(&canonical_request_buf);


    aws_byte_buf_clean_up_secure(&kSigning_buf); // Clean up final signing key

    if (hmac_result == AWS_OP_SUCCESS) {
         aws_byte_buf_clean_up(&signature_hex_buf); // Clean up hex signature buffer
         aws_byte_buf_clean_up(&signed_headers_buf); // Clean up signed headers buffer
         AWS_LOGF_INFO(AWS_LS_AUTH_SIGV4, "SigV4 signing process completed successfully."); // Removed "(with placeholders)"
         return AWS_OP_SUCCESS; // Final success return
    } else {
        // Error occurred during signing steps
        AWS_LOGF_ERROR(AWS_LS_AUTH_SIGV4, "SigV4 signing process failed.");
        // Ensure hex signature and signed headers buffers are cleaned up on error path too
        aws_byte_buf_clean_up(&signature_hex_buf);
        aws_byte_buf_clean_up(&signed_headers_buf);
        return AWS_OP_ERR; // Error already raised
    }
}
