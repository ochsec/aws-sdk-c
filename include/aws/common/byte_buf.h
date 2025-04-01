#ifndef AWS_COMMON_BYTE_BUF_H
#define AWS_COMMON_BYTE_BUF_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/exports.h>
#include <aws/common/allocator.h>
#include <stddef.h> /* size_t */
#include <stdint.h> /* uint8_t */
#include <stdbool.h> /* bool */

AWS_EXTERN_C_BEGIN

/**
 * @brief Represents a mutable buffer of bytes with capacity and length.
 * Useful for I/O operations, string building, etc.
 */
struct aws_byte_buf {
    struct aws_allocator *allocator;
    uint8_t *buffer;    /* Pointer to the allocated memory */
    size_t len;         /* Number of bytes currently initialized/written */
    size_t capacity;    /* Total number of bytes allocated */
};

/**
 * @brief Represents an immutable view of a sequence of bytes.
 */
struct aws_byte_cursor {
    const uint8_t *ptr;
    size_t len;
};

/**
 * @brief Initializes a byte buffer with a specified capacity.
 *
 * @param buf Pointer to the buffer structure to initialize.
 * @param allocator Allocator to use for the buffer's memory.
 * @param capacity Initial capacity in bytes.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure (e.g., OOM).
 */
AWS_COMMON_API
int aws_byte_buf_init(struct aws_byte_buf *buf, struct aws_allocator *allocator, size_t capacity);

/**
 * @brief Cleans up the resources used by a byte buffer, freeing its internal memory.
 *        Does NOT free the buffer structure itself if it was heap-allocated.
 *
 * @param buf Pointer to the buffer to clean up.
 */
AWS_COMMON_API
void aws_byte_buf_clean_up(struct aws_byte_buf *buf);

/**
 * @brief Ensures the buffer has at least `requested_capacity`.
 *        May reallocate memory. Existing data is preserved.
 *
 * @param buf Pointer to the buffer.
 * @param requested_capacity The desired minimum capacity.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure (e.g., OOM).
 */
AWS_COMMON_API
int aws_byte_buf_reserve(struct aws_byte_buf *buf, size_t requested_capacity);

/**
 * @brief Appends bytes from a source buffer to the destination buffer.
 *        Resizes the destination buffer if necessary.
 *
 * @param to Destination buffer.
 * @param from Source buffer containing bytes to append.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 */
AWS_COMMON_API
int aws_byte_buf_append(struct aws_byte_buf *to, const struct aws_byte_cursor *from);

/**
 * @brief Appends a single byte. Resizes if necessary.
 * @param buf Buffer to append to.
 * @param byte Byte value to append.
 * @return AWS_OP_SUCCESS or AWS_OP_ERR.
 */
AWS_COMMON_API
int aws_byte_buf_append_byte(struct aws_byte_buf *buf, uint8_t byte);

/**
 * @brief Resets the length of the buffer to zero, but keeps the allocated capacity.
 * @param buf Buffer to reset.
 */
AWS_COMMON_API
void aws_byte_buf_reset(struct aws_byte_buf *buf, bool zero_memory);

/**
 * @brief Initializes a byte cursor from a byte buffer.
 * @param buf Source buffer.
 * @return An initialized byte cursor viewing the buffer's valid data.
 */
AWS_COMMON_API
struct aws_byte_cursor aws_byte_cursor_from_buf(const struct aws_byte_buf *buf);

/**
 * @brief Initializes a byte cursor from a C string (null-terminated).
 * @param c_str Source C string. Length is determined by strlen.
 * @return An initialized byte cursor viewing the string data (excluding null terminator).
 */
AWS_COMMON_API
struct aws_byte_cursor aws_byte_cursor_from_c_str(const char *c_str);

/**
 * @brief Initializes a byte cursor from raw bytes and length.
 * @param bytes Pointer to the start of the byte sequence.
 * @param len Length of the sequence.
 * @return An initialized byte cursor.
 */
AWS_COMMON_API
struct aws_byte_cursor aws_byte_cursor_from_array(const void *bytes, size_t len);


AWS_EXTERN_C_END

#endif /* AWS_COMMON_BYTE_BUF_H */
