/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/byte_buf.h>
#include <aws/common/error.h>
#include <string.h> /* For memcpy, memset, strlen */
#include <stdlib.h> /* For NULL */
#include <assert.h> /* For assert */

/* Default initial capacity if 0 is provided */
#define AWS_BYTE_BUF_DEFAULT_INITIAL_CAPACITY 128
/* Growth factor when resizing */
#define AWS_BYTE_BUF_GROWTH_FACTOR 2

int aws_byte_buf_init(struct aws_byte_buf *buf, struct aws_allocator *allocator, size_t capacity) {
    assert(allocator != NULL);
    assert(buf != NULL);

    buf->allocator = allocator;
    buf->len = 0;

    if (capacity == 0) {
        capacity = AWS_BYTE_BUF_DEFAULT_INITIAL_CAPACITY;
    }

    buf->buffer = aws_mem_acquire_via(allocator, capacity);
    if (!buf->buffer) {
        buf->capacity = 0;
        return AWS_OP_ERR; /* OOM already raised */
    }
    buf->capacity = capacity;

    return AWS_OP_SUCCESS;
}

void aws_byte_buf_clean_up(struct aws_byte_buf *buf) {
    if (buf && buf->buffer) {
        aws_mem_release_via(buf->allocator, buf->buffer);
        buf->buffer = NULL;
        buf->allocator = NULL;
        buf->capacity = 0;
        buf->len = 0;
    }
}

int aws_byte_buf_reserve(struct aws_byte_buf *buf, size_t requested_capacity) {
    assert(buf != NULL);

    if (buf->capacity >= requested_capacity) {
        return AWS_OP_SUCCESS; /* Already have enough */
    }

    size_t new_capacity = buf->capacity;
    if (new_capacity == 0) {
        new_capacity = AWS_BYTE_BUF_DEFAULT_INITIAL_CAPACITY;
    }
    while (new_capacity < requested_capacity) {
        /* Check for overflow before multiplying */
        if (new_capacity > SIZE_MAX / AWS_BYTE_BUF_GROWTH_FACTOR) {
             aws_raise_error(AWS_ERROR_OOM);
             return AWS_OP_ERR;
        }
        new_capacity *= AWS_BYTE_BUF_GROWTH_FACTOR;
    }

    /* Reallocate */
    void *new_buffer = aws_mem_realloc_via(buf->allocator, buf->buffer, buf->capacity, new_capacity);
    if (!new_buffer) {
        return AWS_OP_ERR; /* OOM already raised */
    }

    buf->buffer = new_buffer;
    buf->capacity = new_capacity;
    return AWS_OP_SUCCESS;
}

int aws_byte_buf_append(struct aws_byte_buf *to, const struct aws_byte_cursor *from) {
    assert(to != NULL);
    assert(from != NULL);

    size_t required_capacity = to->len + from->len;
    /* Check for overflow */
    if (required_capacity < to->len || required_capacity < from->len) {
         aws_raise_error(AWS_ERROR_OOM);
         return AWS_OP_ERR;
    }

    if (aws_byte_buf_reserve(to, required_capacity) != AWS_OP_SUCCESS) {
        return AWS_OP_ERR; /* Error already raised */
    }

    memcpy(to->buffer + to->len, from->ptr, from->len);
    to->len += from->len;
    return AWS_OP_SUCCESS;
}

int aws_byte_buf_append_byte(struct aws_byte_buf *buf, uint8_t byte) {
     assert(buf != NULL);
     size_t required_capacity = buf->len + 1;
     if (required_capacity < buf->len) { /* Overflow check */
          aws_raise_error(AWS_ERROR_OOM);
          return AWS_OP_ERR;
     }
     if (aws_byte_buf_reserve(buf, required_capacity) != AWS_OP_SUCCESS) {
         return AWS_OP_ERR;
     }
     buf->buffer[buf->len] = byte;
     buf->len++;
     return AWS_OP_SUCCESS;
}

void aws_byte_buf_reset(struct aws_byte_buf *buf, bool zero_memory) {
    assert(buf != NULL);
    if (zero_memory && buf->capacity > 0) {
        memset(buf->buffer, 0, buf->capacity);
    }
    buf->len = 0;
}

struct aws_byte_cursor aws_byte_cursor_from_buf(const struct aws_byte_buf *buf) {
    assert(buf != NULL);
    return (struct aws_byte_cursor){ .ptr = buf->buffer, .len = buf->len };
}

struct aws_byte_cursor aws_byte_cursor_from_c_str(const char *c_str) {
    assert(c_str != NULL);
    return (struct aws_byte_cursor){ .ptr = (const uint8_t *)c_str, .len = strlen(c_str) };
}

struct aws_byte_cursor aws_byte_cursor_from_array(const void *bytes, size_t len) {
     assert(bytes != NULL || len == 0);
     return (struct aws_byte_cursor){ .ptr = bytes, .len = len };
}
