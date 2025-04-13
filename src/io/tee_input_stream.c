/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/io/stream.h>
#include <aws/common/byte_buf.h>
#include <aws/common/error.h>
#include <aws/common/logging.h>
#include <aws/common/ref_count.h>

// Fallback definitions if not provided by the library
#ifndef AWS_STREAM_SEEK_FROM_BEGIN
#define AWS_STREAM_SEEK_FROM_BEGIN 0
#define AWS_STREAM_SEEK_FROM_CURRENT 1
#define AWS_STREAM_SEEK_FROM_END 2
#endif

#ifndef AWS_STREAM_STATUS_FLAG_END_OF_STREAM
#define AWS_STREAM_STATUS_FLAG_END_OF_STREAM (1 << 0)
#endif

#define AWS_LS_IO_TEE_STREAM 0x1000

/**
 * Tee input stream structure that manages buffering and branching
 */
struct aws_tee_input_stream {
    struct aws_input_stream base;
    struct aws_allocator *allocator;
    struct aws_input_stream *source;
    struct aws_byte_buf buffer;
    bool source_complete;
};

/**
 * Branch stream structure for independent reading
 */
struct aws_tee_branch_stream {
    struct aws_input_stream base;
    struct aws_tee_input_stream *parent;
    size_t read_cursor;
};

// Forward declarations
static int s_tee_stream_seek(struct aws_input_stream *stream, int64_t offset, enum aws_stream_seek_basis basis);
static int s_tee_stream_read(struct aws_input_stream *stream, struct aws_byte_buf *dest);
static int s_tee_stream_get_status(struct aws_input_stream *stream, struct aws_stream_status *status);

static int s_branch_stream_seek(struct aws_input_stream *stream, int64_t offset, enum aws_stream_seek_basis basis);
static int s_branch_stream_read(struct aws_input_stream *stream, struct aws_byte_buf *dest);
static int s_branch_stream_get_status(struct aws_input_stream *stream, struct aws_stream_status *status);

// Vtable definitions
static struct aws_input_stream_vtable s_tee_stream_vtable = {
    .seek = s_tee_stream_seek,
    .read = s_tee_stream_read,
    .get_status = s_tee_stream_get_status,
};

static struct aws_input_stream_vtable s_branch_stream_vtable = {
    .seek = s_branch_stream_seek,
    .read = s_branch_stream_read,
    .get_status = s_branch_stream_get_status,
};

struct aws_input_stream *aws_tee_input_stream_create(
    struct aws_allocator *allocator,
    struct aws_input_stream *source) {
    
    AWS_PRECONDITION(allocator);
    AWS_PRECONDITION(source);

    struct aws_tee_input_stream *tee_stream = aws_mem_calloc(allocator, 1, sizeof(struct aws_tee_input_stream));
    if (!tee_stream) {
        return NULL;
    }

    tee_stream->allocator = allocator;
    tee_stream->source = source;

    // Initialize the base stream
    aws_input_stream_init_base(&tee_stream->base, &s_tee_stream_vtable, tee_stream);

    // Initialize buffer
    if (aws_byte_buf_init(&tee_stream->buffer, allocator, 4096) != AWS_OP_SUCCESS) {
        aws_mem_release(allocator, tee_stream);
        return NULL;
    }

    return &tee_stream->base;
}

struct aws_input_stream *aws_tee_input_stream_new_branch(
    struct aws_input_stream *stream) {
    
    AWS_PRECONDITION(stream);
    
    if (!aws_input_stream_is_tee_stream(stream)) {
        aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
        return NULL;
    }

    struct aws_tee_input_stream *parent = AWS_CONTAINER_OF(stream, struct aws_tee_input_stream, base);
    struct aws_tee_branch_stream *branch = aws_mem_calloc(parent->allocator, 1, sizeof(struct aws_tee_branch_stream));
    
    if (!branch) {
        return NULL;
    }

    branch->parent = parent;
    aws_input_stream_init_base(&branch->base, &s_branch_stream_vtable, branch);

    return &branch->base;
}

bool aws_input_stream_is_tee_stream(const struct aws_input_stream *stream) {
    return stream && stream->vtable == &s_tee_stream_vtable;
}

// Tee Stream Implementation
static int s_tee_stream_seek(struct aws_input_stream *stream, int64_t offset, enum aws_stream_seek_basis basis) {
    struct aws_tee_input_stream *tee_stream = AWS_CONTAINER_OF(stream, struct aws_tee_input_stream, base);
    
    // Reset buffer and state
    aws_byte_buf_reset(&tee_stream->buffer, true);
    tee_stream->source_complete = false;

    // Seek source stream
    return aws_input_stream_seek(tee_stream->source, offset, basis);
}

static int s_tee_stream_read(struct aws_input_stream *stream, struct aws_byte_buf *dest) {
    struct aws_tee_input_stream *tee_stream = AWS_CONTAINER_OF(stream, struct aws_tee_input_stream, base);
    
    // If source is complete and buffer is empty, return EOF
    if (tee_stream->source_complete && tee_stream->buffer.len == 0) {
        return AWS_OP_SUCCESS;
    }

    // Read from source if not complete
    if (!tee_stream->source_complete) {
        struct aws_byte_buf temp_buf;
        aws_byte_buf_init(&temp_buf, tee_stream->allocator, 4096);
        
        int result = aws_input_stream_read(tee_stream->source, &temp_buf);
        if (result != AWS_OP_SUCCESS) {
            aws_byte_buf_clean_up(&temp_buf);
            return result;
        }

        // Check if source is complete
        if (temp_buf.len == 0) {
            tee_stream->source_complete = true;
            aws_byte_buf_clean_up(&temp_buf);
        } else {
            // Append to buffer
            struct aws_byte_cursor temp_cursor = aws_byte_cursor_from_buf(&temp_buf);
            aws_byte_buf_append_dynamic(&tee_stream->buffer, &temp_cursor);
            aws_byte_buf_clean_up(&temp_buf);
        }
    }

    // Copy from buffer to destination
    size_t bytes_to_copy = AWS_MIN(dest->capacity - dest->len, tee_stream->buffer.len);
    if (bytes_to_copy > 0) {
        struct aws_byte_cursor src_cursor = aws_byte_cursor_from_array(
            tee_stream->buffer.buffer, bytes_to_copy);
        aws_byte_buf_append_dynamic(dest, &src_cursor);
        
        // Remove copied bytes from buffer
        memmove(tee_stream->buffer.buffer, 
                tee_stream->buffer.buffer + bytes_to_copy, 
                tee_stream->buffer.len - bytes_to_copy);
        tee_stream->buffer.len -= bytes_to_copy;
    }

    return AWS_OP_SUCCESS;
}

static int s_tee_stream_get_status(struct aws_input_stream *stream, struct aws_stream_status *status) {
    struct aws_tee_input_stream *tee_stream = AWS_CONTAINER_OF(stream, struct aws_tee_input_stream, base);
    
    memset(status, 0, sizeof(struct aws_stream_status));
    status->is_end_of_stream = tee_stream->source_complete && tee_stream->buffer.len == 0;

    return AWS_OP_SUCCESS;
}

// Branch Stream Implementation
static int s_branch_stream_seek(struct aws_input_stream *stream, int64_t offset, enum aws_stream_seek_basis basis) {
    struct aws_tee_branch_stream *branch = AWS_CONTAINER_OF(stream, struct aws_tee_branch_stream, base);
    struct aws_tee_input_stream *parent = branch->parent;

    int64_t new_position = 0;
    switch (basis) {
        case AWS_STREAM_SEEK_FROM_BEGIN:
            new_position = offset;
            break;
        case AWS_STREAM_SEEK_FROM_CURRENT:
            new_position = (int64_t)branch->read_cursor + offset;
            break;
        case AWS_STREAM_SEEK_FROM_END:
            new_position = (int64_t)parent->buffer.len + offset;
            break;
        default:
            return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }

    if (new_position < 0 || (size_t)new_position > parent->buffer.len) {
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }

    branch->read_cursor = (size_t)new_position;
    return AWS_OP_SUCCESS;
}

static int s_branch_stream_read(struct aws_input_stream *stream, struct aws_byte_buf *dest) {
    struct aws_tee_branch_stream *branch = AWS_CONTAINER_OF(stream, struct aws_tee_branch_stream, base);
    struct aws_tee_input_stream *parent = branch->parent;

    // If parent is complete and no more data, return EOF
    if (parent->source_complete && branch->read_cursor >= parent->buffer.len) {
        return AWS_OP_SUCCESS;
    }

    // Copy from parent's buffer
    size_t bytes_available = parent->buffer.len - branch->read_cursor;
    size_t bytes_to_copy = AWS_MIN(bytes_available, dest->capacity - dest->len);

    if (bytes_to_copy > 0) {
        struct aws_byte_cursor src_cursor = aws_byte_cursor_from_array(
            parent->buffer.buffer + branch->read_cursor, bytes_to_copy);
        aws_byte_buf_append_dynamic(dest, &src_cursor);
        branch->read_cursor += bytes_to_copy;
    }

    return AWS_OP_SUCCESS;
}

static int s_branch_stream_get_status(struct aws_input_stream *stream, struct aws_stream_status *status) {
    struct aws_tee_branch_stream *branch = AWS_CONTAINER_OF(stream, struct aws_tee_branch_stream, base);
    struct aws_tee_input_stream *parent = branch->parent;

    memset(status, 0, sizeof(struct aws_stream_status));
    status->is_end_of_stream = parent->source_complete && 
        branch->read_cursor >= parent->buffer.len;

    return AWS_OP_SUCCESS;
}