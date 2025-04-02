#ifndef AWS_COMMON_HTTP_CLIENT_H
#define AWS_COMMON_HTTP_CLIENT_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/exports.h>
#include <aws/common/allocator.h>
#include <aws/common/http.h> /* For aws_http_message, aws_http_header */
#include <aws/common/byte_buf.h> /* For aws_byte_cursor */

AWS_EXTERN_C_BEGIN

/* Forward declarations */
struct aws_http_client;
struct aws_http_connection; /* Represents a single connection */
struct aws_http_stream; /* Represents a single request/response stream on a connection */

/* --- Enum Definitions --- */

/** Enum for header block type */
enum aws_http_header_block {
    AWS_HTTP_HEADER_BLOCK_UNKNOWN = 0,
    AWS_HTTP_HEADER_BLOCK_MAIN,      /* Main request/response headers */
    AWS_HTTP_HEADER_BLOCK_INFORMATIONAL, /* 1xx headers */
    AWS_HTTP_HEADER_BLOCK_TRAILING,  /* Trailing headers */
};

/* --- Callback Function Types --- */

/** Invoked when connection acquisition completes (successfully or not). */
typedef void(aws_http_on_client_connection_setup_fn)(
    struct aws_http_connection *connection,
    int error_code,
    void *user_data);

/** Invoked when a connection is shut down. */
typedef void(aws_http_on_client_connection_shutdown_fn)(
    struct aws_http_connection *connection,
    int error_code,
    void *user_data);

/** Invoked when an incoming header block is received on a stream. */
typedef int(aws_http_on_incoming_headers_fn)(
    struct aws_http_stream *stream,
    enum aws_http_header_block header_block, /* Now defined */
    const struct aws_http_header *header_array,
    size_t num_headers,
    void *user_data);

/** Invoked when an incoming header block is completely received. */
typedef int(aws_http_on_incoming_header_block_done_fn)(
    struct aws_http_stream *stream,
    enum aws_http_header_block header_block, /* Now defined */
    void *user_data);

/** Invoked when incoming body data is received on a stream. */
typedef int(aws_http_on_incoming_body_fn)(
    struct aws_http_stream *stream,
    const struct aws_byte_cursor *data,
    void *user_data);

/** Invoked when a stream completes (successfully or with an error). */
typedef void(aws_http_on_stream_complete_fn)(
    struct aws_http_stream *stream,
    int error_code,
    void *user_data);


/* --- Options Structures --- */

/** Options for acquiring an HTTP connection. */
struct aws_http_client_connection_options {
    struct aws_allocator *allocator;
    /* TODO: Add actual options like host, port, socket_options, tls_connection_options */
    size_t initial_window_size; /* Optional: Initial window size for HTTP/2 */
    void *user_data;
    aws_http_on_client_connection_shutdown_fn *on_shutdown;
};

/** Options for making an HTTP request on a connection. */
struct aws_http_make_request_options {
    struct aws_http_message *request; /* The request message to send */
    void *user_data;
    aws_http_on_incoming_headers_fn *on_response_headers;
    aws_http_on_incoming_header_block_done_fn *on_response_header_block_done;
    aws_http_on_incoming_body_fn *on_response_body;
    aws_http_on_stream_complete_fn *on_complete;
};


/* --- HTTP Client --- */

/** Opaque handle for an HTTP client implementation. */
struct aws_http_client;

/** Virtual function table for an HTTP client implementation. */
struct aws_http_client_vtable {
    void (*destroy)(struct aws_http_client *client);
    /* Acquires a connection from the client's pool or creates a new one. */
    void (*acquire_connection)(
        struct aws_http_client *client,
        aws_http_on_client_connection_setup_fn *callback,
        void *user_data);
    /* Releases a connection back to the pool or closes it. */
    void (*release_connection)(struct aws_http_client *client, struct aws_http_connection *connection);
};

/** Base structure for HTTP clients. Concrete implementations embed this. */
struct aws_http_client {
    const struct aws_http_client_vtable *vtable;
    struct aws_allocator *allocator;
    void *impl; /* Pointer to implementation-specific data */
};

/** Destroys an HTTP client. */
AWS_COMMON_API
void aws_http_client_destroy(struct aws_http_client *client);

/** Acquires an HTTP connection asynchronously. */
AWS_COMMON_API
void aws_http_client_acquire_connection(
    struct aws_http_client *client,
    aws_http_on_client_connection_setup_fn *callback,
    void *user_data);

/** Releases an HTTP connection. */
AWS_COMMON_API
void aws_http_client_release_connection(struct aws_http_client *client, struct aws_http_connection *connection);


/* --- HTTP Connection --- */

/** Opaque handle for an active HTTP connection. */
struct aws_http_connection;

/** Virtual function table for an HTTP connection implementation. */
struct aws_http_connection_vtable {
    void (*release)(struct aws_http_connection *connection); /* Called by aws_http_client_release_connection */
    bool (*is_open)(const struct aws_http_connection *connection);
    void (*close)(struct aws_http_connection *connection);
    /* Creates a new stream (request) on this connection. */
    struct aws_http_stream *(*make_request)(
        struct aws_http_connection *connection,
        const struct aws_http_make_request_options *options); /* Options struct now defined */
};

/** Base structure for HTTP connections. */
struct aws_http_connection {
    const struct aws_http_connection_vtable *vtable;
    struct aws_allocator *allocator;
    void *impl;
    void *user_data; /* User data associated via connection options */
    aws_http_on_client_connection_shutdown_fn *on_shutdown; /* Shutdown callback */
};

/** Checks if a connection is open and usable. */
AWS_COMMON_API
bool aws_http_connection_is_open(const struct aws_http_connection *connection);

/** Closes the connection. The shutdown callback will be invoked asynchronously. */
AWS_COMMON_API
void aws_http_connection_close(struct aws_http_connection *connection);

/** Creates a new HTTP stream (request) on this connection. */
AWS_COMMON_API
struct aws_http_stream *aws_http_connection_make_request(
    struct aws_http_connection *connection,
    const struct aws_http_make_request_options *options);


/* --- HTTP Stream --- */

/** Opaque handle for an active HTTP request/response stream. */
struct aws_http_stream;

/** Virtual function table for an HTTP stream implementation. */
struct aws_http_stream_vtable {
    void (*release)(struct aws_http_stream *stream); /* Called by aws_http_stream_release */
    int (*activate)(struct aws_http_stream *stream); /* Starts the request */
    int (*update_window)(struct aws_http_stream *stream, size_t increment_size); /* For flow control */
    int (*get_response_status)(const struct aws_http_stream *stream, int *out_status);
    struct aws_http_connection *(*get_connection)(const struct aws_http_stream *stream);
};

/** Base structure for HTTP streams. */
struct aws_http_stream {
    const struct aws_http_stream_vtable *vtable;
    struct aws_allocator *allocator;
    void *impl;
    void *user_data; /* User data associated via make_request options */
};

/** Releases a reference to the stream. May trigger destruction if ref count hits zero. */
AWS_COMMON_API
void aws_http_stream_release(struct aws_http_stream *stream);

/** Activates the stream, sending the request. */
AWS_COMMON_API
int aws_http_stream_activate(struct aws_http_stream *stream);

/** Updates the receive window for flow control (HTTP/2). */
AWS_COMMON_API
int aws_http_stream_update_window(struct aws_http_stream *stream, size_t increment_size);

/** Gets the HTTP response status code for the completed stream. */
AWS_COMMON_API
int aws_http_stream_get_response_status(const struct aws_http_stream *stream, int *out_status);

/** Gets the connection associated with this stream. */
AWS_COMMON_API
struct aws_http_connection *aws_http_stream_get_connection(const struct aws_http_stream *stream);


AWS_EXTERN_C_END

#endif /* AWS_COMMON_HTTP_CLIENT_H */
