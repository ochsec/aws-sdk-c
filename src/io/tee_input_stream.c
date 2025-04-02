/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/io/tee_input_stream.h>
#include <aws/common/byte_buf.h> /* Needed for aws_byte_buf_* functions */
#include <aws/common/logging.h>
#include <aws/common/error.h>

/* Define logging source for IO operations */
#define AWS_LS_IO_GENERAL 0x1000
#define AWS_LS_IO_TEE_STREAM (AWS_LS_IO_GENERAL + 1)

/* Define error codes for stream operations */
#define AWS_ERROR_STREAM_READ_FAILED 0x2001
#define AWS_ERROR_STREAM_UNSEEKABLE 0x2002
#define AWS_ERROR_STREAM_UNKNOWN_LENGTH 0x2003
#define AWS_ERROR_STREAM_SEEK_FAILED 0x2004

/**
 * Main tee stream structure that holds the source stream and buffered data.
 */
struct aws_tee_input_stream {
    struct aws_input_stream base;
    struct aws_allocator *allocator;
    struct aws_input_stream *source;
    struct aws_byte_buf buffer;
    bool source_complete;
    bool source_consumed;
};

/**
 * Branch stream structure that reads from the parent tee stream's buffer.
 */
struct aws_tee_branch_stream {
    struct aws_input_stream base;
    struct aws_allocator *allocator;
    struct aws_tee_input_stream *parent;
    size_t read_cursor;
};

/* Forward declarations */
static int s_tee_stream_read(struct aws_input_stream *stream, struct aws_byte_buf *dest);
static int s_tee_stream_seek(struct aws_input_stream *stream, int64_t offset, enum aws_stream_seek_basis basis);
static int s_tee_stream_get_length(struct aws_input_stream *stream, int64_t *out_length);
static int s_tee_stream_get_status(struct aws_input_stream *stream, struct aws_stream_status *out_status);
static void s_tee_stream_destroy(struct aws_input_stream *stream);

static int s_branch_stream_read(struct aws_input_stream *stream, struct aws_byte_buf *dest);
static int s_branch_stream_seek(struct aws_input_stream *stream, int64_t offset, enum aws_stream_seek_basis basis);
static int s_branch_stream_get_length(struct aws_input_stream *stream, int64_t *out_length);
static int s_branch_stream_get_status(struct aws_input_stream *stream, struct aws_stream_status *out_status);
static void s_branch_stream_destroy(struct aws_input_stream *stream);

/* VTable for the main tee stream */
static const struct aws_input_stream_vtable s_tee_stream_vtable = {
    .read = s_tee_stream_read,
    .seek = s_tee_stream_seek,
    .get_length = s_tee_stream_get_length,
    .get_status = s_tee_stream_get_status,
    .destroy = s_tee_stream_destroy,
};

/* VTable for branch streams */
static const struct aws_input_stream_vtable s_branch_stream_vtable = {
    .read = s_branch_stream_read,
    .seek = s_branch_stream_seek,
    .get_length = s_branch_stream_get_length,
    .get_status = s_branch_stream_get_status,
    .destroy = s_branch_stream_destroy,
};

/* Implementation of tee stream functions */

/**
 * Reads from the source stream into the internal buffer, then copies from the buffer to the destination.
 */
static int s_tee_stream_read(struct aws_input_stream *stream, struct aws_byte_buf *dest) {
    struct aws_tee_input_stream *tee_stream = (struct aws_tee_input_stream *)stream->impl;

    /* If we've already consumed the source stream, just read from the buffer */
    if (tee_stream->source_consumed) {
        AWS_LOGF_ERROR("AWS_IO_TEE_STREAM", "Tee stream source already consumed. Cannot read more data.");
        aws_raise_error(AWS_ERROR_STREAM_READ_FAILED);
        return AWS_OP_ERR;
    }

    /* If we haven't read all data from the source yet, read more */
    if (!tee_stream->source_complete) {
        /* Create a temporary buffer for reading from the source */
        struct aws_byte_buf temp_buf;
        if (aws_byte_buf_init(&temp_buf, tee_stream->allocator, 1024)) {
            return AWS_OP_ERR;
        }

        /* Read from the source stream */
        int result = aws_input_stream_read(tee_stream->source, &temp_buf);
        if (result != AWS_OP_SUCCESS) {
            aws_byte_buf_clean_up(&temp_buf);
            return result;
        }

        /* If we read 0 bytes, the source is complete */
        if (temp_buf.len == 0) {
            tee_stream->source_complete = true;
            aws_byte_buf_clean_up(&temp_buf);
        } else {
            /* Append the data to our buffer */
            struct aws_byte_cursor temp_cursor = aws_byte_cursor_from_buf(&temp_buf);
            if (aws_byte_buf_append(&tee_stream->buffer, &temp_cursor)) { /* Use append and pass pointer */
                aws_byte_buf_clean_up(&temp_buf);
                return AWS_OP_ERR;
            }
            aws_byte_buf_clean_up(&temp_buf);
        }
    }

    /* Copy data from our buffer to the destination */
    size_t bytes_to_copy = tee_stream->buffer.len;
    if (bytes_to_copy > dest->capacity - dest->len) {
        bytes_to_copy = dest->capacity - dest->len;
    }

    if (bytes_to_copy > 0) {
        struct aws_byte_cursor src_cursor = aws_byte_cursor_from_array(tee_stream->buffer.buffer, bytes_to_copy);
        if (aws_byte_buf_append(dest, &src_cursor)) { /* Pass pointer */
            return AWS_OP_ERR;
        }

        /* Mark the source as consumed since we've read it */
        tee_stream->source_consumed = true;
    }

    return AWS_OP_SUCCESS;
}

/**
 * Seeks within the tee stream. This is only supported if the source stream is seekable.
 */
static int s_tee_stream_seek(struct aws_input_stream *stream, int64_t offset, enum aws_stream_seek_basis basis) {
    struct aws_tee_input_stream *tee_stream = (struct aws_tee_input_stream *)stream->impl;

    /* Check if the source stream is seekable */
    struct aws_stream_status status;
    if (aws_input_stream_get_status(tee_stream->source, &status)) {
        return AWS_OP_ERR;
    }

    if (!(status.flags & AWS_STREAM_STATUS_SEEKABLE)) {
        AWS_LOGF_ERROR("AWS_IO_TEE_STREAM", "Tee stream source is not seekable.");
        aws_raise_error(AWS_ERROR_STREAM_UNSEEKABLE);
        return AWS_OP_ERR;
    }

    /* Seek the source stream */
    if (aws_input_stream_seek(tee_stream->source, offset, basis)) {
        return AWS_OP_ERR;
    }

    /* Reset our buffer and state */
    aws_byte_buf_clean_up(&tee_stream->buffer);
    if (aws_byte_buf_init(&tee_stream->buffer, tee_stream->allocator, 1024)) {
        return AWS_OP_ERR;
    }

    tee_stream->source_complete = false;
    tee_stream->source_consumed = false;

    return AWS_OP_SUCCESS;
}

/**
 * Gets the length of the tee stream, which is the length of the source stream.
 */
static int s_tee_stream_get_length(struct aws_input_stream *stream, int64_t *out_length) {
    struct aws_tee_input_stream *tee_stream = (struct aws_tee_input_stream *)stream->impl;

    /* If the source stream has a known length, return it */
    struct aws_stream_status status;
    if (aws_input_stream_get_status(tee_stream->source, &status)) {
        return AWS_OP_ERR;
    }

    if (!(status.flags & AWS_STREAM_STATUS_KNOWN_LENGTH)) {
        AWS_LOGF_ERROR("AWS_IO_TEE_STREAM", "Tee stream source does not have a known length.");
        aws_raise_error(AWS_ERROR_STREAM_UNKNOWN_LENGTH);
        return AWS_OP_ERR;
    }

    return aws_input_stream_get_length(tee_stream->source, out_length);
}

/**
 * Gets the status of the tee stream.
 */
static int s_tee_stream_get_status(struct aws_input_stream *stream, struct aws_stream_status *out_status) {
    struct aws_tee_input_stream *tee_stream = (struct aws_tee_input_stream *)stream->impl;

    /* Get the status of the source stream */
    struct aws_stream_status source_status;
    if (aws_input_stream_get_status(tee_stream->source, &source_status)) {
        return AWS_OP_ERR;
    }

    /* Copy the flags from the source stream */
    out_status->flags = source_status.flags;

    /* Set the EOF flag if we've read all data from the source */
    if (tee_stream->source_complete) {
        out_status->flags |= AWS_STREAM_STATUS_EOF;
    }

    return AWS_OP_SUCCESS;
}

/**
 * Destroys the tee stream.
 */
static void s_tee_stream_destroy(struct aws_input_stream *stream) {
    if (!stream) {
        return;
    }

    struct aws_tee_input_stream *tee_stream = (struct aws_tee_input_stream *)stream->impl;

    /* Clean up the buffer */
    aws_byte_buf_clean_up(&tee_stream->buffer);

    /* Destroy the source stream */
    if (tee_stream->source) {
        aws_input_stream_destroy(tee_stream->source);
    }

    /* Free the tee stream */
    aws_mem_release(tee_stream->allocator, tee_stream);
}

/* Implementation of branch stream functions */

/**
 * Reads from the branch stream, which reads from the parent tee stream's buffer.
 */
static int s_branch_stream_read(struct aws_input_stream *stream, struct aws_byte_buf *dest) {
    struct aws_tee_branch_stream *branch = (struct aws_tee_branch_stream *)stream->impl;
    struct aws_tee_input_stream *parent = branch->parent;

    /* If we're at the end of the parent's buffer and the parent isn't complete, wait for more data */
    if (branch->read_cursor >= parent->buffer.len && !parent->source_complete) {
        /* Create a temporary buffer for reading from the source */
        struct aws_byte_buf temp_buf;
        if (aws_byte_buf_init(&temp_buf, parent->allocator, 1024)) {
            return AWS_OP_ERR;
        }

        /* Read from the source stream */
        int result = aws_input_stream_read(parent->source, &temp_buf);
        if (result != AWS_OP_SUCCESS) {
            aws_byte_buf_clean_up(&temp_buf);
            return result;
        }

        /* If we read 0 bytes, the source is complete */
        if (temp_buf.len == 0) {
            parent->source_complete = true;
            aws_byte_buf_clean_up(&temp_buf);
        } else {
            /* Append the data to the parent's buffer */
            struct aws_byte_cursor temp_cursor = aws_byte_cursor_from_buf(&temp_buf);
            if (aws_byte_buf_append(&parent->buffer, &temp_cursor)) { /* Use append and pass pointer */
                aws_byte_buf_clean_up(&temp_buf);
                return AWS_OP_ERR;
            }
            aws_byte_buf_clean_up(&temp_buf);
        }
    }

    /* Copy data from the parent's buffer to the destination */
    size_t bytes_available = parent->buffer.len - branch->read_cursor;
    size_t bytes_to_copy = bytes_available;
    if (bytes_to_copy > dest->capacity - dest->len) {
        bytes_to_copy = dest->capacity - dest->len;
    }

    if (bytes_to_copy > 0) {
        struct aws_byte_cursor src_cursor = aws_byte_cursor_from_array(
            parent->buffer.buffer + branch->read_cursor, bytes_to_copy);
        if (aws_byte_buf_append(dest, &src_cursor)) { /* Pass pointer */
            return AWS_OP_ERR;
        }

        branch->read_cursor += bytes_to_copy;
    }

    return AWS_OP_SUCCESS;
}

/**
 * Seeks within the branch stream.
 */
static int s_branch_stream_seek(struct aws_input_stream *stream, int64_t offset, enum aws_stream_seek_basis basis) {
    struct aws_tee_branch_stream *branch = (struct aws_tee_branch_stream *)stream->impl;
    struct aws_tee_input_stream *parent = branch->parent;

    int64_t new_position = 0;

    switch (basis) {
        case AWS_SSB_BEGIN:
            new_position = offset;
            break;
        case AWS_SSB_CURRENT:
            new_position = (int64_t)branch->read_cursor + offset;
            break;
        case AWS_SSB_END:
            /* If the parent isn't complete, we need to read all data first */
            if (!parent->source_complete) {
                /* Read all data from the source */
                while (!parent->source_complete) {
                    struct aws_byte_buf temp_buf;
                    if (aws_byte_buf_init(&temp_buf, parent->allocator, 1024)) {
                        return AWS_OP_ERR;
                    }

                    int result = aws_input_stream_read(parent->source, &temp_buf);
                    if (result != AWS_OP_SUCCESS) {
                        aws_byte_buf_clean_up(&temp_buf);
                        return result;
                    }

                    if (temp_buf.len == 0) {
                        parent->source_complete = true;
                        aws_byte_buf_clean_up(&temp_buf);
                    } else {
                        /* Append the data to the parent's buffer */
                        struct aws_byte_cursor temp_cursor = aws_byte_cursor_from_buf(&temp_buf);
                        if (aws_byte_buf_append(&parent->buffer, &temp_cursor)) { /* Use append and pass pointer */
                            aws_byte_buf_clean_up(&temp_buf);
                            return AWS_OP_ERR;
                        }
                        aws_byte_buf_clean_up(&temp_buf);
                    }
                } /* end while */
            } /* end if (!parent->source_complete) */

            new_position = (int64_t)parent->buffer.len + offset;
            break; /* Added missing break */
        default:
            aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
            return AWS_OP_ERR;
    } /* end switch */

    /* Check if the new position is valid */
    if (new_position < 0) {
        aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
        return AWS_OP_ERR;
    }

    /* If the new position is beyond the current buffer and the source isn't complete,
     * we need to read more data */
    if ((size_t)new_position > parent->buffer.len && !parent->source_complete) {
        /* Read data until we reach the desired position or the source is complete */
        while ((size_t)new_position > parent->buffer.len && !parent->source_complete) {
            struct aws_byte_buf temp_buf;
            if (aws_byte_buf_init(&temp_buf, parent->allocator, 1024)) {
                return AWS_OP_ERR;
            }

            int result = aws_input_stream_read(parent->source, &temp_buf);
            if (result != AWS_OP_SUCCESS) {
                aws_byte_buf_clean_up(&temp_buf);
                return result;
            }

            if (temp_buf.len == 0) {
                parent->source_complete = true;
                aws_byte_buf_clean_up(&temp_buf);
            } else {
                /* Append the data to the parent's buffer */
                struct aws_byte_cursor temp_cursor = aws_byte_cursor_from_buf(&temp_buf);
                if (aws_byte_buf_append(&parent->buffer, &temp_cursor)) { /* Use append and pass pointer */
                    aws_byte_buf_clean_up(&temp_buf);
                    return AWS_OP_ERR;
                }
                aws_byte_buf_clean_up(&temp_buf);
            }
        } /* end while */
    } /* end if ((size_t)new_position > parent->buffer.len ...) */

    /* If the new position is still beyond the buffer, it's an error */
    if ((size_t)new_position > parent->buffer.len) {
        aws_raise_error(AWS_ERROR_STREAM_SEEK_FAILED);
        return AWS_OP_ERR;
    }

    /* Set the new position */
    branch->read_cursor = (size_t)new_position;

    return AWS_OP_SUCCESS;
}

/**
 * Gets the length of the branch stream, which is the length of the parent tee stream.
 */
static int s_branch_stream_get_length(struct aws_input_stream *stream, int64_t *out_length) {
    struct aws_tee_branch_stream *branch = (struct aws_tee_branch_stream *)stream->impl;
    struct aws_tee_input_stream *parent = branch->parent;

    /* If the parent source is complete, return the buffer length */
    if (parent->source_complete) {
        *out_length = (int64_t)parent->buffer.len;
        return AWS_OP_SUCCESS;
    }

    /* Otherwise, get the length from the source stream */
    return aws_input_stream_get_length(&parent->base, out_length);
}

/**
 * Gets the status of the branch stream.
 */
static int s_branch_stream_get_status(struct aws_input_stream *stream, struct aws_stream_status *out_status) {
    struct aws_tee_branch_stream *branch = (struct aws_tee_branch_stream *)stream->impl;
    struct aws_tee_input_stream *parent = branch->parent;

    /* Get the status of the parent stream */
    return aws_input_stream_get_status(&parent->base, out_status);
}

/**
 * Destroys the branch stream.
 */
static void s_branch_stream_destroy(struct aws_input_stream *stream) {
    if (!stream) {
        return;
    }

    struct aws_tee_branch_stream *branch = (struct aws_tee_branch_stream *)stream->impl;

    /* Free the branch stream */
    aws_mem_release(branch->allocator, branch);
}

/* Public API functions */

struct aws_input_stream *aws_tee_input_stream_new(
    struct aws_allocator *allocator,
    struct aws_input_stream *source) {

    AWS_PRECONDITION(allocator);
    AWS_PRECONDITION(source);

    /* Allocate the tee stream */
    struct aws_tee_input_stream *tee_stream = aws_mem_calloc(allocator, 1, sizeof(struct aws_tee_input_stream));
    if (!tee_stream) {
        return NULL;
    }

    /* Initialize the base stream */
    tee_stream->base.vtable = &s_tee_stream_vtable;
    tee_stream->base.allocator = allocator;
    tee_stream->base.impl = tee_stream;

    /* Initialize the tee stream */
    tee_stream->allocator = allocator;
    tee_stream->source = source;
    tee_stream->source_complete = false;
    tee_stream->source_consumed = false;

    /* Initialize the buffer */
    if (aws_byte_buf_init(&tee_stream->buffer, allocator, 1024)) {
        aws_mem_release(allocator, tee_stream);
        return NULL;
    }

    return &tee_stream->base;
}

struct aws_input_stream *aws_tee_input_stream_new_branch(
    struct aws_input_stream *tee_stream) {

    AWS_PRECONDITION(tee_stream);

    /* Check if the stream is a tee stream */
    if (!aws_input_stream_is_tee_stream(tee_stream)) {
        aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
        return NULL;
    }

    struct aws_tee_input_stream *parent = (struct aws_tee_input_stream *)tee_stream->impl;

    /* Allocate the branch stream */
    struct aws_tee_branch_stream *branch = aws_mem_calloc(parent->allocator, 1, sizeof(struct aws_tee_branch_stream));
    if (!branch) {
        return NULL;
    }

    /* Initialize the base stream */
    branch->base.vtable = &s_branch_stream_vtable;
    branch->base.allocator = parent->allocator;
    branch->base.impl = branch;

    /* Initialize the branch stream */
    branch->allocator = parent->allocator;
    branch->parent = parent;
    branch->read_cursor = 0;

    return &branch->base;
}

bool aws_input_stream_is_tee_stream(
    const struct aws_input_stream *stream) {

    return stream && stream->vtable == &s_tee_stream_vtable;
}
