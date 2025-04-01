#ifndef AWS_COMMON_STRING_H
#define AWS_COMMON_STRING_H

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
 * @brief Represents a dynamically allocated, mutable string.
 *
 * Holds a pointer to a null-terminated C string, its length, and the allocator
 * used to manage its memory.
 */
struct aws_string {
    struct aws_allocator *allocator;
    char *bytes;
    size_t length;
    size_t capacity; /* Optional: track capacity for optimization */
};

/**
 * @brief Creates a new aws_string from a null-terminated C string.
 *
 * The input C string is copied into newly allocated memory.
 *
 * @param allocator The allocator to use for the string's memory.
 * @param c_str A null-terminated C string to copy.
 * @return A pointer to the new aws_string, or NULL on allocation failure.
 *         The caller is responsible for destroying the string using aws_string_destroy.
 */
AWS_COMMON_API
struct aws_string *aws_string_new_from_c_str(struct aws_allocator *allocator, const char *c_str);

/**
 * @brief Creates a new aws_string from a sequence of bytes (not necessarily null-terminated).
 *
 * The input bytes are copied into newly allocated memory, and a null terminator is added.
 *
 * @param allocator The allocator to use for the string's memory.
 * @param bytes A pointer to the sequence of bytes.
 * @param len The number of bytes to copy.
 * @return A pointer to the new aws_string, or NULL on allocation failure.
 *         The caller is responsible for destroying the string using aws_string_destroy.
 */
AWS_COMMON_API
struct aws_string *aws_string_new_from_bytes(struct aws_allocator *allocator, const uint8_t *bytes, size_t len);

/**
 * @brief Destroys an aws_string, freeing its allocated memory.
 *
 * If `str` is NULL, no operation is performed.
 *
 * @param str The aws_string to destroy.
 */
AWS_COMMON_API
void aws_string_destroy(struct aws_string *str);

/**
 * @brief Returns a pointer to the underlying null-terminated C string.
 *
 * @param str The aws_string.
 * @return A const pointer to the C string data.
 */
AWS_COMMON_API
const char *aws_string_c_str(const struct aws_string *str);

/**
 * @brief Returns the length of the string (excluding the null terminator).
 *
 * @param str The aws_string.
 * @return The length of the string.
 */
AWS_COMMON_API
size_t aws_string_length(const struct aws_string *str);

/**
 * @brief Compares two aws_string structures lexicographically.
 *
 * @param a The first string.
 * @param b The second string.
 * @return An integer less than, equal to, or greater than zero if `a` is found,
 *         respectively, to be less than, to match, or be greater than `b`.
 */
AWS_COMMON_API
int aws_string_compare(const struct aws_string *a, const struct aws_string *b);

/**
 * @brief Compares an aws_string with a C string lexicographically.
 *
 * @param str The aws_string.
 * @param c_str The null-terminated C string.
 * @return An integer less than, equal to, or greater than zero if `str` is found,
 *         respectively, to be less than, to match, or be greater than `c_str`.
 */
AWS_COMMON_API
int aws_string_compare_c_str(const struct aws_string *str, const char *c_str);


AWS_EXTERN_C_END

#endif /* AWS_COMMON_STRING_H */
