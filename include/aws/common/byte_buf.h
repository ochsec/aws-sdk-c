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

/**
 * @brief Initializes a byte buffer by copying the contents of a byte cursor.
 *
 * @param dest Pointer to the destination buffer structure to initialize.
 * @param allocator Allocator to use for the buffer's memory.
 * @param src Source cursor containing the bytes to copy.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure (e.g., OOM).
 */
AWS_COMMON_API
int aws_byte_buf_init_copy_from_cursor(
    struct aws_byte_buf *dest,
    struct aws_allocator *allocator,
    struct aws_byte_cursor src);

/**
 * @brief Advances a cursor by `len` bytes and returns a new cursor pointing to the
 *        original position. The original cursor `cursor` is modified to point
 *        `len` bytes forward.
 *
 * @param cursor Pointer to the cursor to advance. This cursor will be modified.
 * @param len Number of bytes to advance.
 * @return A new cursor pointing to the original position of `cursor`.
 *         If the cursor cannot be advanced by `len` bytes (i.e., `len > cursor->len`),
 *         the original cursor is unmodified and a cursor with `ptr = NULL, len = 0` is returned.
 */
AWS_COMMON_API
struct aws_byte_cursor aws_byte_cursor_advance(struct aws_byte_cursor *cursor, size_t len);

/**
 * @brief Finds the next occurrence of `delimiter` within `input_str`.
 *        If found, `substr_cursor` is updated to point to the segment before the delimiter,
 *        and `input_str` is advanced past the delimiter.
 *
 * @param input_str Pointer to the cursor to search within. This cursor will be modified.
 * @param delimiter The character to search for.
 * @param substr_cursor Pointer to a cursor that will be updated with the segment before the delimiter.
 * @return `true` if the delimiter was found, `false` otherwise. If `false`, `input_str`
 *         and `substr_cursor` are unmodified.
 */
AWS_COMMON_API
bool aws_byte_cursor_next_split(
    struct aws_byte_cursor *input_str, /* NOTE: Modified from const* in perplexity output based on common usage */
    char delimiter,
    struct aws_byte_cursor *substr_cursor);

/**
* @brief Parses a uint64_t from the beginning of a UTF-8 encoded cursor.
*
* @param cursor Pointer to the cursor to parse from. Will be advanced past the parsed number on success.
* @param value Pointer to store the parsed value.
* @return `true` on success, `false` on failure (e.g., invalid format, overflow).
*/
AWS_COMMON_API
bool aws_byte_cursor_utf8_parse_u64(struct aws_byte_cursor *cursor, uint64_t *value);

/**
* @brief Reads a single byte from the cursor and advances the cursor.
*
* @param cursor Pointer to the cursor to read from. Will be advanced by 1 on success.
* @param var Pointer to store the read byte.
* @return `true` if a byte was successfully read, `false` if the cursor is empty.
*/
AWS_COMMON_API
bool aws_byte_cursor_read_u8(struct aws_byte_cursor *cursor, uint8_t *var);

/**
* @brief Reads two hex characters from the cursor, converts them to a byte, and advances the cursor.
*
* @param cursor Pointer to the cursor to read from. Will be advanced by 2 on success.
* @param value Pointer to store the resulting byte.
* @return `true` on success, `false` on failure (e.g., not enough bytes, invalid hex chars).
*/
AWS_COMMON_API
bool aws_byte_cursor_read_hex_u8(struct aws_byte_cursor *cursor, uint8_t *value);

/**
* @brief Ensures the buffer has space for at least `additional_length` more bytes
*        beyond its current `len`. May reallocate.
*
* @param buf Pointer to the buffer.
* @param additional_length Number of additional bytes required.
* @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure (e.g., OOM, overflow).
*/
AWS_COMMON_API
int aws_byte_buf_reserve_relative(struct aws_byte_buf *buf, size_t additional_length);

/** Appends a single byte, dynamically resizing (growing) the buffer if necessary. */
AWS_COMMON_API
int aws_byte_buf_append_byte_dynamic(struct aws_byte_buf *buffer, uint8_t value);

/** Appends the contents of a cursor, dynamically resizing (growing) the buffer if necessary. */
AWS_COMMON_API
int aws_byte_buf_append_dynamic(struct aws_byte_buf *to, const struct aws_byte_cursor *from);

/** Splits a cursor into segments based on a delimiter, storing results in an array list. */
AWS_COMMON_API
int aws_byte_cursor_split_on_char(
    const struct aws_byte_cursor *input_str,
    char split_on,
    struct aws_array_list *output);

/** Compares a cursor to a C string for equality. */
AWS_COMMON_API
bool aws_byte_cursor_eq_c_str(const struct aws_byte_cursor *const cursor, const char *const c_str);

/** Lexicographically compares two cursors. */
AWS_COMMON_API
int aws_byte_cursor_compare(const struct aws_byte_cursor *lhs, const struct aws_byte_cursor *rhs);

AWS_EXTERN_C_END

#endif /* AWS_COMMON_BYTE_BUF_H */
