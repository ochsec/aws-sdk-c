/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#ifndef AWS_IO_INPUT_STREAM_H
#define AWS_IO_INPUT_STREAM_H

#include <aws/common/common.h>
#include <aws/common/byte_buf.h>

/* Define AWS_IO_API if not already defined */
#ifndef AWS_IO_API
#define AWS_IO_API
#endif

AWS_EXTERN_C_BEGIN

/**
 * Seek origin for aws_input_stream_seek.
 */
enum aws_stream_seek_basis {
    AWS_SSB_BEGIN = 0,   /* Seek from the beginning of the stream */
    AWS_SSB_CURRENT = 1, /* Seek from the current position */
    AWS_SSB_END = 2,     /* Seek from the end of the stream */
};

/* Forward declare the vtable */
struct aws_input_stream_vtable;

/**
 * Base structure for input streams.
 * Concrete implementations embed this as their first member.
 */
struct aws_input_stream {
    const struct aws_input_stream_vtable *vtable;
    struct aws_allocator *allocator;
    void *impl; /* Pointer to implementation-specific data */
};

/**
 * Stream capabilities and status flags.
 */
enum aws_stream_status_flags {
    AWS_STREAM_STATUS_SEEKABLE = 0x00000001,     /* Stream supports seeking */
    AWS_STREAM_STATUS_KNOWN_LENGTH = 0x00000002, /* Stream has a known length */
    AWS_STREAM_STATUS_EOF = 0x00000004,          /* Stream is at EOF */
};

/**
 * Stream status information.
 */
struct aws_stream_status {
    uint32_t flags; /* Bitwise OR of aws_stream_status_flags */
};


/**
 * Virtual function table for an input stream implementation.
 */
struct aws_input_stream_vtable {
    /**
     * Reads from the stream into the destination buffer.
     * Returns AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
     * Sets *out_length to the number of bytes read, which may be less than buffer_length.
     * If 0 bytes are read and no error occurred, the stream is at EOF.
     */
    int (*read)(struct aws_input_stream *stream, struct aws_byte_buf *dest);

    /**
     * Seeks to a position in the stream.
     * Returns AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
     * Not all streams support seeking. Check aws_input_stream_get_status for capabilities.
     */
    int (*seek)(struct aws_input_stream *stream, int64_t offset, enum aws_stream_seek_basis basis);

    /**
     * Gets the length of the stream, if known.
     * Returns AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
     * Sets *out_length to the length of the stream.
     * Not all streams have a known length. Check aws_input_stream_get_status for capabilities.
     */
    int (*get_length)(struct aws_input_stream *stream, int64_t *out_length);

    /**
     * Gets the current position in the stream.
     * Returns AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
     * Sets *out_position to the current position in the stream.
     */
    int (*get_position)(struct aws_input_stream *stream, int64_t *out_position);

    /**
     * Gets the status of the stream.
     * Returns AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
     * Sets *out_status to the current status of the stream.
     */
    int (*get_status)(struct aws_input_stream *stream, struct aws_stream_status *out_status);

    /**
     * Destroys the stream.
     */
    void (*destroy)(struct aws_input_stream *stream);
};

/**
 * Reads from the stream into the destination buffer.
 * Returns AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 * Sets *out_length to the number of bytes read, which may be less than buffer_length.
 * If 0 bytes are read and no error occurred, the stream is at EOF.
 */
AWS_IO_API int aws_input_stream_read(
    struct aws_input_stream *stream,
    struct aws_byte_buf *dest);

/**
 * Seeks to a position in the stream.
 * Returns AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 * Not all streams support seeking. Check aws_input_stream_get_status for capabilities.
 */
AWS_IO_API int aws_input_stream_seek(
    struct aws_input_stream *stream,
    int64_t offset,
    enum aws_stream_seek_basis basis);

/**
 * Gets the length of the stream, if known.
 * Returns AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 * Sets *out_length to the length of the stream.
 * Not all streams have a known length. Check aws_input_stream_get_status for capabilities.
 */
AWS_IO_API int aws_input_stream_get_length(
    struct aws_input_stream *stream,
    int64_t *out_length);

/**
 * Gets the current position in the stream.
 * Returns AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 * Sets *out_position to the current position in the stream.
 */
AWS_IO_API int aws_input_stream_get_position(
    struct aws_input_stream *stream,
    int64_t *out_position);

/**
 * Gets the status of the stream.
 * Returns AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 * Sets *out_status to the current status of the stream.
 */
AWS_IO_API int aws_input_stream_get_status(
    struct aws_input_stream *stream,
    struct aws_stream_status *out_status);

/**
 * Destroys the stream.
 */
AWS_IO_API void aws_input_stream_destroy(struct aws_input_stream *stream);

/**
 * Creates a new input stream from a byte cursor.
 * The stream will read from the cursor and is seekable.
 * The cursor's memory must remain valid for the lifetime of the stream.
 */
AWS_IO_API struct aws_input_stream *aws_input_stream_new_from_cursor(
    struct aws_allocator *allocator,
    const struct aws_byte_cursor *cursor);

/**
 * Creates a new input stream from a byte buffer.
 * The stream will read from the buffer and is seekable.
 * The buffer's memory must remain valid for the lifetime of the stream.
 */
AWS_IO_API struct aws_input_stream *aws_input_stream_new_from_buf(
    struct aws_allocator *allocator,
    const struct aws_byte_buf *buffer);

AWS_EXTERN_C_END

#endif /* AWS_IO_INPUT_STREAM_H */
