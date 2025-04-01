/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/http_client.h>
#include <aws/common/assert.h> /* Use assert for preconditions */

/* --- HTTP Client --- */

void aws_http_client_destroy(struct aws_http_client *client) {
    if (client && client->vtable && client->vtable->destroy) {
        client->vtable->destroy(client);
    }
}

void aws_http_client_acquire_connection(
    struct aws_http_client *client,
    aws_http_on_client_connection_setup_fn *callback,
    void *user_data) {

    AWS_ASSERT(client && client->vtable && client->vtable->acquire_connection);
    client->vtable->acquire_connection(client, callback, user_data);
}

void aws_http_client_release_connection(struct aws_http_client *client, struct aws_http_connection *connection) {
    AWS_ASSERT(client && client->vtable && client->vtable->release_connection);
    /* Connection release might be handled internally by the connection's release, */
    /* or the client might need to be explicitly told. Depends on implementation. */
    /* Let's assume the client vtable function handles releasing it back to the pool/closing. */
    client->vtable->release_connection(client, connection);
}


/* --- HTTP Connection --- */

bool aws_http_connection_is_open(const struct aws_http_connection *connection) {
    AWS_ASSERT(connection && connection->vtable && connection->vtable->is_open);
    return connection->vtable->is_open(connection);
}

void aws_http_connection_close(struct aws_http_connection *connection) {
    AWS_ASSERT(connection && connection->vtable && connection->vtable->close);
    connection->vtable->close(connection);
}

struct aws_http_stream *aws_http_connection_make_request(
    struct aws_http_connection *connection,
    const struct aws_http_make_request_options *options) {

    AWS_ASSERT(connection && connection->vtable && connection->vtable->make_request);
    return connection->vtable->make_request(connection, options);
}

/* Internal release function, usually called by client */
void aws_http_connection_release(struct aws_http_connection *connection) {
     if (connection && connection->vtable && connection->vtable->release) {
         connection->vtable->release(connection);
     }
}


/* --- HTTP Stream --- */

void aws_http_stream_release(struct aws_http_stream *stream) {
    if (stream && stream->vtable && stream->vtable->release) {
        stream->vtable->release(stream);
    }
}

int aws_http_stream_activate(struct aws_http_stream *stream) {
    AWS_ASSERT(stream && stream->vtable && stream->vtable->activate);
    return stream->vtable->activate(stream);
}

int aws_http_stream_update_window(struct aws_http_stream *stream, size_t increment_size) {
    AWS_ASSERT(stream && stream->vtable && stream->vtable->update_window);
    return stream->vtable->update_window(stream, increment_size);
}

int aws_http_stream_get_response_status(const struct aws_http_stream *stream, int *out_status) {
    AWS_ASSERT(stream && stream->vtable && stream->vtable->get_response_status);
    return stream->vtable->get_response_status(stream, out_status);
}

struct aws_http_connection *aws_http_stream_get_connection(const struct aws_http_stream *stream) {
    AWS_ASSERT(stream && stream->vtable && stream->vtable->get_connection);
    return stream->vtable->get_connection(stream);
}
