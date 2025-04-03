#ifndef AWS_COMMON_ENCODING_H
#define AWS_COMMON_ENCODING_H

/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/byte_buf.h>
#include <aws/common/exports.h>

#include <stddef.h>
#include <stdint.h>

AWS_EXTERN_C_BEGIN

/**
 * Given the length of a base64 encoded string, computes the maximum length of the decoded data.
 */
AWS_COMMON_API
int aws_base64_compute_decoded_len(const struct aws_byte_cursor *encoded, size_t *decoded_len);

/**
 * Encodes data in src as base64 into dest. dest->capacity must be large enough to hold the encoded data. Use
 * aws_base64_compute_encoded_len() to compute the required capacity. dest->len will be updated with the amount of
 * data written.
 */
AWS_COMMON_API
int aws_base64_encode(const struct aws_byte_cursor *to_encode, struct aws_byte_buf *dest);

/**
 * Decodes the base64 encoded data in src into dest. dest->capacity must be large enough to hold the decoded data. Use
 * aws_base64_compute_decoded_len() to compute the required capacity. dest->len will be updated with the amount of
 * data written.
 */
AWS_COMMON_API
int aws_base64_decode(const struct aws_byte_cursor *to_decode, struct aws_byte_buf *dest);

/**
 * Given the length of a hex encoded string, computes the maximum length of the decoded data.
 */
AWS_COMMON_API
int aws_hex_compute_decoded_len(const struct aws_byte_cursor *encoded, size_t *decoded_len);

/**
 * Encodes data in src as hex into dest. dest->capacity must be large enough to hold the encoded data. Use
 * aws_hex_compute_encoded_len() to compute the required capacity. dest->len will be updated with the amount of
 * data written.
 */
AWS_COMMON_API
int aws_hex_encode(const struct aws_byte_cursor *to_encode, struct aws_byte_buf *dest);

/**
 * Decodes the hex encoded data in src into dest. dest->capacity must be large enough to hold the decoded data. Use
 * aws_hex_compute_decoded_len() to compute the required capacity. dest->len will be updated with the amount of
 * data written.
 */
AWS_COMMON_API
int aws_hex_decode(const struct aws_byte_cursor *to_decode, struct aws_byte_buf *dest);

/**
 * Appends the hex representation of the `to_encode` cursor to the `dest` buffer.
 * `dest` must have enough capacity to hold the encoded data. Use aws_hex_compute_encoded_len()
 * to compute the required capacity. `dest->len` will be updated with the amount of data written.
 */
AWS_COMMON_API
int aws_byte_buf_append_encoding_to_hex(struct aws_byte_buf *dest, const struct aws_byte_cursor *to_encode);

/**
 * Given the length of binary data, computes the length of the hex encoded string.
 */
AWS_COMMON_API
int aws_hex_compute_encoded_len(size_t data_len, size_t *encoded_len);

/**
 * Given the length of binary data, computes the length of the base64 encoded string.
 */
AWS_COMMON_API
int aws_base64_compute_encoded_len(size_t data_len, size_t *encoded_len);

/**
 * @brief Checks if a character is alphanumeric (a-z, A-Z, 0-9).
 *
 * @param c Character code (int).
 * @return `true` if alphanumeric, `false` otherwise.
 */
AWS_COMMON_API
bool aws_isalnum(int c);

AWS_EXTERN_C_END

#endif /* AWS_COMMON_ENCODING_H */
