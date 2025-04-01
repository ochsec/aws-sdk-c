#ifndef AWS_COMMON_HTTP_H
#define AWS_COMMON_HTTP_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/exports.h>
#include <aws/common/allocator.h>
#include <aws/common/string.h>
#include <aws/common/byte_buf.h> /* Need to define this for buffers/streams */
#include <aws/common/hash_table.h> /* Need to define this for headers */

AWS_EXTERN_C_BEGIN

/**
 * @brief HTTP Method Verbs
 */
enum aws_http_method {
    AWS_HTTP_METHOD_UNKNOWN = 0,
    AWS_HTTP_METHOD_GET,
    AWS_HTTP_METHOD_POST,
    AWS_HTTP_METHOD_PUT,
    AWS_HTTP_METHOD_DELETE,
    AWS_HTTP_METHOD_HEAD,
    AWS_HTTP_METHOD_PATCH,
    AWS_HTTP_METHOD_OPTIONS,

    AWS_HTTP_METHOD_COUNT, /* Sentinel */
};

/**
 * @brief Standard HTTP Status Codes
 * Partial list, add more as needed.
 */
enum aws_http_status_code {
    AWS_HTTP_STATUS_CODE_UNKNOWN = -1,
    AWS_HTTP_STATUS_CODE_CONTINUE = 100,
    AWS_HTTP_STATUS_CODE_OK = 200,
    AWS_HTTP_STATUS_CODE_ACCEPTED = 202,
    AWS_HTTP_STATUS_CODE_NO_CONTENT = 204,
    AWS_HTTP_STATUS_CODE_PARTIAL_CONTENT = 206,
    AWS_HTTP_STATUS_CODE_MOVED_PERMANENTLY = 301,
    AWS_HTTP_STATUS_CODE_FOUND = 302,
    AWS_HTTP_STATUS_CODE_NOT_MODIFIED = 304,
    AWS_HTTP_STATUS_CODE_TEMPORARY_REDIRECT = 307,
    AWS_HTTP_STATUS_CODE_PERMANENT_REDIRECT = 308,
    AWS_HTTP_STATUS_CODE_BAD_REQUEST = 400,
    AWS_HTTP_STATUS_CODE_UNAUTHORIZED = 401,
    AWS_HTTP_STATUS_CODE_FORBIDDEN = 403,
    AWS_HTTP_STATUS_CODE_NOT_FOUND = 404,
    AWS_HTTP_STATUS_CODE_METHOD_NOT_ALLOWED = 405,
    AWS_HTTP_STATUS_CODE_REQUEST_TIMEOUT = 408,
    AWS_HTTP_STATUS_CODE_CONFLICT = 409,
    AWS_HTTP_STATUS_CODE_LENGTH_REQUIRED = 411,
    AWS_HTTP_STATUS_CODE_PRECONDITION_FAILED = 412,
    AWS_HTTP_STATUS_CODE_PAYLOAD_TOO_LARGE = 413,
    AWS_HTTP_STATUS_CODE_URI_TOO_LONG = 414,
    AWS_HTTP_STATUS_CODE_RANGE_NOT_SATISFIABLE = 416,
    AWS_HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR = 500,
    AWS_HTTP_STATUS_CODE_NOT_IMPLEMENTED = 501,
    AWS_HTTP_STATUS_CODE_BAD_GATEWAY = 502,
    AWS_HTTP_STATUS_CODE_SERVICE_UNAVAILABLE = 503,
    AWS_HTTP_STATUS_CODE_GATEWAY_TIMEOUT = 504,
};

/**
 * @brief Represents a single HTTP header (name-value pair).
 * Uses aws_string for ownership.
 */
struct aws_http_header {
    struct aws_string *name;
    struct aws_string *value;
};

/**
 * @brief Represents a collection of HTTP headers.
 * Typically implemented using a hash table for efficient lookup.
 */
struct aws_http_headers; /* Opaque structure */

/**
 * @brief Creates a new, empty set of HTTP headers.
 * @param allocator Allocator to use.
 * @param initial_capacity Hint for initial hash table size.
 * @return New headers object, or NULL on failure.
 */
AWS_COMMON_API
struct aws_http_headers *aws_http_headers_new(struct aws_allocator *allocator, size_t initial_capacity);

/**
 * @brief Destroys an HTTP headers object.
 * @param headers Headers object to destroy.
 */
AWS_COMMON_API
void aws_http_headers_destroy(struct aws_http_headers *headers);

/**
 * @brief Adds or updates a header. Makes copies of name and value strings.
 * @param headers Headers object.
 * @param name Header name.
 * @param value Header value.
 * @return AWS_OP_SUCCESS or AWS_OP_ERR.
 */
AWS_COMMON_API
int aws_http_headers_set(struct aws_http_headers *headers, const struct aws_string *name, const struct aws_string *value);

/**
 * @brief Adds or updates a header using C strings. Makes copies.
 * @param headers Headers object.
 * @param name Header name (C string).
 * @param value Header value (C string).
 * @return AWS_OP_SUCCESS or AWS_OP_ERR.
 */
AWS_COMMON_API
int aws_http_headers_set_c_str(struct aws_http_headers *headers, const char *name, const char *value);

/**
 * @brief Gets the value of a header.
 * @param headers Headers object.
 * @param name Header name.
 * @param value Pointer to store the header value (pointer to internal string, do not modify/free).
 * @return AWS_OP_SUCCESS if found, AWS_OP_ERR if not found.
 */
AWS_COMMON_API
int aws_http_headers_get(const struct aws_http_headers *headers, const struct aws_string *name, const struct aws_string **value);

/**
 * @brief Gets the value of a header using C string name.
 * @param headers Headers object.
 * @param name Header name (C string).
 * @param value Pointer to store the header value (pointer to internal string, do not modify/free).
 * @return AWS_OP_SUCCESS if found, AWS_OP_ERR if not found.
 */
AWS_COMMON_API
int aws_http_headers_get_c_str(const struct aws_http_headers *headers, const char *name, const struct aws_string **value);

/**
 * @brief Removes a header.
 * @param headers Headers object.
 * @param name Header name.
 * @return AWS_OP_SUCCESS or AWS_OP_ERR.
 */
AWS_COMMON_API
int aws_http_headers_erase(struct aws_http_headers *headers, const struct aws_string *name);

/**
 * @brief Removes a header using C string name.
 * @param headers Headers object.
 * @param name Header name (C string).
 * @return AWS_OP_SUCCESS or AWS_OP_ERR.
 */
AWS_COMMON_API
int aws_http_headers_erase_c_str(struct aws_http_headers *headers, const char *name);

/**
 * @brief Gets the number of headers stored.
 * @param headers Headers object.
 * @return Number of headers.
 */
AWS_COMMON_API
size_t aws_http_headers_count(const struct aws_http_headers *headers);

/* TODO: Add iteration functions for headers */


/**
 * @brief Represents an HTTP request message.
 */
struct aws_http_message; /* Opaque structure */

/* TODO: Add functions for creating, modifying, and destroying aws_http_message */
/* e.g., aws_http_message_new_request, aws_http_message_set_method, */
/*       aws_http_message_set_path, aws_http_message_add_header, */
/*       aws_http_message_set_body, aws_http_message_destroy */


AWS_EXTERN_C_END

#endif /* AWS_COMMON_HTTP_H */
