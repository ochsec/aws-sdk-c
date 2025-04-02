/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/testing/aws_test_harness.h> // CRT testing framework
#include <aws/auth/sigv4.h> // Include the header for the function we are testing

#include <aws/common/allocator.h>
#include <aws/common/date_time.h>
#include <aws/common/string.h>
#include <aws/common/byte_buf.h>
#include <aws/http/request_response.h> // For aws_http_message
#include <aws/auth/credentials.h>      // For aws_credentials (mock or real)

// Helper to create static credentials for testing
static struct aws_credentials *s_create_static_test_credentials(
    struct aws_allocator *allocator,
    const char *access_key_id,
    const char *secret_access_key,
    const char *session_token) {

    struct aws_byte_cursor access_key_id_cur = aws_byte_cursor_from_c_str(access_key_id);
    struct aws_byte_cursor secret_access_key_cur = aws_byte_cursor_from_c_str(secret_access_key);
    struct aws_byte_cursor session_token_cur = aws_byte_cursor_from_c_str(session_token); // Can be empty

    return aws_credentials_new_static(allocator, &access_key_id_cur, &secret_access_key_cur, &session_token_cur, UINT64_MAX);
}

// Test case based on AWS documentation example: GET request, empty body, basic headers
// See: https://docs.aws.amazon.com/general/latest/gr/sigv4-create-canonical-request.html (and subsequent steps)
static int s_test_sigv4_basic_get_request(struct aws_allocator *allocator, void *ctx) {
    (void)ctx; // Unused context

    // --- Input Data ---
    const char *test_access_key_id = "AKIDEXAMPLE";
    const char *test_secret_key = "wJalrXUtnFEMI/K7MDENG+bPxRfiCYEXAMPLEKEY";
    const char *test_region = "us-east-1";
    const char *test_service = "service"; // Generic service name from example
    const char *test_date_iso8601 = "20150830T123600Z";
    const char *test_host = "example.amazonaws.com";
    const char *test_path = "/";

    // Expected results (derived from AWS documentation example steps)
    const char *expected_canonical_request =
        "GET\n"
        "/\n"
        "\n"
        "host:example.amazonaws.com\n"
        "x-amz-date:20150830T123600Z\n"
        "\n"
        "host;x-amz-date\n"
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"; // SHA256 of empty string

    const char *expected_string_to_sign =
        "AWS4-HMAC-SHA256\n"
        "20150830T123600Z\n"
        "20150830/us-east-1/service/aws4_request\n"
        "f536975d06c0309214f805bb90ccff089219ecd68b2577efef23edd43b7e1a59"; // SHA256 of canonical_request

    const char *expected_signature = "5d672d79c15b13162d9279b0855cfba6789a8edb4c82c400e06b5924a6f2b5d7";

    const char *expected_auth_header_value_format =
        "AWS4-HMAC-SHA256 Credential=%s/20150830/%s/%s/aws4_request, SignedHeaders=host;x-amz-date, Signature=%s";

    // --- Setup ---
    struct aws_credentials *credentials =
        s_create_static_test_credentials(allocator, test_access_key_id, test_secret_key, "");
    ASSERT_NOT_NULL(credentials, "Failed to create test credentials.");

    struct aws_http_message *request = aws_http_message_new_request(allocator);
    ASSERT_NOT_NULL(request, "Failed to create test HTTP request.");

    struct aws_http_header host_header = {
        .name = aws_byte_cursor_from_c_str("Host"),
        .value = aws_byte_cursor_from_c_str(test_host),
    };
    ASSERT_SUCCESS(aws_http_message_add_header(request, host_header), "Failed to add Host header.");
    ASSERT_SUCCESS(aws_http_message_set_request_method(request, aws_byte_cursor_from_c_str("GET")), "Failed to set method.");
    ASSERT_SUCCESS(aws_http_message_set_request_path(request, aws_byte_cursor_from_c_str(test_path)), "Failed to set path.");
    // No body stream needed for empty body

    struct aws_date_time signing_date;
    ASSERT_SUCCESS(
        aws_date_time_init_from_iso8601(&signing_date, aws_byte_cursor_from_c_str(test_date_iso8601)),
        "Failed to parse signing date.");

    struct aws_byte_cursor region_cur = aws_byte_cursor_from_c_str(test_region);
    struct aws_byte_cursor service_cur = aws_byte_cursor_from_c_str(test_service);

    // --- Execute ---
    int result = aws_sigv4_sign_request(allocator, request, credentials, region_cur, service_cur, &signing_date);

    // --- Verify ---
    ASSERT_SUCCESS(result, "aws_sigv4_sign_request failed with error: %s", aws_error_debug_str(aws_last_error()));

    // Verify X-Amz-Date header
    struct aws_byte_cursor x_amz_date_name = aws_byte_cursor_from_c_str("X-Amz-Date");
    struct aws_byte_cursor x_amz_date_value;
    ASSERT_SUCCESS(
        aws_http_message_get_header(request, &x_amz_date_value, &x_amz_date_name), "X-Amz-Date header not found.");
    ASSERT_BIN_ARRAYS_EQUALS(
        test_date_iso8601, strlen(test_date_iso8601), x_amz_date_value.ptr, x_amz_date_value.len, "X-Amz-Date mismatch.");

    // Verify Authorization header
    struct aws_byte_cursor auth_header_name = aws_byte_cursor_from_c_str("Authorization");
    struct aws_byte_cursor auth_header_value;
    ASSERT_SUCCESS(
        aws_http_message_get_header(request, &auth_header_value, &auth_header_name), "Authorization header not found.");

    // Construct the expected Authorization header value
    struct aws_string *expected_auth_header_str = aws_string_new_from_format(
        allocator,
        expected_auth_header_value_format,
        test_access_key_id,
        test_region,
        test_service,
        expected_signature);
    ASSERT_NOT_NULL(expected_auth_header_str, "Failed to format expected Authorization header.");

    ASSERT_BIN_ARRAYS_EQUALS(
        aws_string_bytes(expected_auth_header_str),
        aws_string_length(expected_auth_header_str),
        auth_header_value.ptr,
        auth_header_value.len,
        "Authorization header mismatch.");

    // --- Cleanup ---
    aws_string_destroy(expected_auth_header_str);
    aws_http_message_destroy(request);
    aws_credentials_release(credentials);

    return AWS_OP_SUCCESS;
}

AWS_TEST_CASE(test_sigv4_basic_get_request, s_test_sigv4_basic_get_request)

// TODO: Add more test cases:
// - Test with query parameters
// - Test with different headers (sorting, folding)
// - Test with empty body
// - Test with non-empty body (seekable stream)
// - Test with pre-calculated hash (x-amz-content-sha256)
// - Test with session token
// - Test path normalization edge cases (., .., //, encoded chars)
// - Test error conditions (e.g., non-seekable stream without hash)
