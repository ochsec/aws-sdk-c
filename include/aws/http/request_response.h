/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#ifndef AWS_HTTP_REQUEST_RESPONSE_H
#define AWS_HTTP_REQUEST_RESPONSE_H

#include <aws/common/common.h>
#include <aws/common/byte_buf.h>
#include <aws/io/input_stream.h>

/* Define AWS_HTTP_API if not already defined */
#ifndef AWS_HTTP_API
#define AWS_HTTP_API
#endif

AWS_EXTERN_C_BEGIN

/**
 * Represents an HTTP header (name-value pair).
 */
struct aws_http_header {
    struct aws_byte_cursor name;
    struct aws_byte_cursor value;
};

/**
 * Represents an HTTP message (request or response).
 * This is an opaque structure; use the accessor functions to interact with it.
 */
struct aws_http_message;

/**
 * Creates a new HTTP request message.
 * @param allocator Memory allocator to use.
 * @return A new HTTP request message, or NULL on failure.
 */
AWS_HTTP_API struct aws_http_message *aws_http_message_new_request(struct aws_allocator *allocator);

/**
 * Creates a new HTTP response message.
 * @param allocator Memory allocator to use.
 * @return A new HTTP response message, or NULL on failure.
 */
AWS_HTTP_API struct aws_http_message *aws_http_message_new_response(struct aws_allocator *allocator);

/**
 * Destroys an HTTP message.
 * @param message Message to destroy.
 */
AWS_HTTP_API void aws_http_message_destroy(struct aws_http_message *message);

/**
 * Sets the request method.
 * @param request HTTP request message.
 * @param method Method to set (e.g., "GET", "POST").
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 */
AWS_HTTP_API int aws_http_message_set_request_method(
    struct aws_http_message *request,
    struct aws_byte_cursor method);

/**
 * Gets the request method.
 * @param request HTTP request message.
 * @param out_method Where to store the method.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 */
AWS_HTTP_API int aws_http_message_get_request_method(
    const struct aws_http_message *request,
    struct aws_byte_cursor *out_method);

/**
 * Sets the request path.
 * @param request HTTP request message.
 * @param path Path to set (e.g., "/index.html").
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 */
AWS_HTTP_API int aws_http_message_set_request_path(
    struct aws_http_message *request,
    struct aws_byte_cursor path);

/**
 * Gets the request path.
 * @param request HTTP request message.
 * @param out_path Where to store the path.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 */
AWS_HTTP_API int aws_http_message_get_request_path(
    const struct aws_http_message *request,
    struct aws_byte_cursor *out_path);

/**
 * Sets the response status code.
 * @param response HTTP response message.
 * @param status_code Status code to set (e.g., 200, 404).
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 */
AWS_HTTP_API int aws_http_message_set_response_status(
    struct aws_http_message *response,
    int status_code);

/**
 * Gets the response status code.
 * @param response HTTP response message.
 * @param out_status_code Where to store the status code.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 */
AWS_HTTP_API int aws_http_message_get_response_status(
    const struct aws_http_message *response,
    int *out_status_code);

/**
 * Adds a header to the message.
 * @param message HTTP message.
 * @param header Header to add.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 */
AWS_HTTP_API int aws_http_message_add_header(
    struct aws_http_message *message,
    struct aws_http_header header);

/**
 * Gets a header from the message by index.
 * @param message HTTP message.
 * @param out_header Where to store the header.
 * @param index Index of the header to get.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 */
AWS_HTTP_API int aws_http_message_get_header_at(
    const struct aws_http_message *message,
    struct aws_http_header *out_header,
    unsigned int index);

/**
 * Gets a header from the message by name.
 * @param message HTTP message.
 * @param out_header_value Where to store the header value.
 * @param name Name of the header to get.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 */
AWS_HTTP_API int aws_http_message_get_header(
    const struct aws_http_message *message,
    struct aws_byte_cursor *out_header_value,
    const struct aws_byte_cursor *name);

/**
 * Gets the number of headers in the message.
 * @param message HTTP message.
 * @return Number of headers.
 */
AWS_HTTP_API unsigned int aws_http_message_get_header_count(
    const struct aws_http_message *message);

/**
 * Sets the body stream for the message.
 * @param message HTTP message.
 * @param body_stream Body stream to set.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 */
AWS_HTTP_API int aws_http_message_set_body_stream(
    struct aws_http_message *message,
    struct aws_input_stream *body_stream);

/**
 * Gets the body stream for the message.
 * @param message HTTP message.
 * @return Body stream, or NULL if not set.
 */
AWS_HTTP_API struct aws_input_stream *aws_http_message_get_body_stream(
    const struct aws_http_message *message);

AWS_EXTERN_C_END

#endif /* AWS_HTTP_REQUEST_RESPONSE_H */
