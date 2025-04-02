/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/encoding.h>

#include <aws/common/byte_buf.h>
#include <aws/common/common.h>

#include <ctype.h>

static const uint8_t s_base64_encoding_table[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
};

static const uint8_t s_base64_decoding_table[256] = {
    /* clang-format off */
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x3E, 0x80, 0x80, 0x80, 0x3F,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    /* clang-format on */
};

static const uint8_t s_hex_encoding_table[] = "0123456789abcdef";

static const uint8_t s_hex_decoding_table[256] = {
    /* clang-format off */
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    /* clang-format on */
};

int aws_base64_compute_decoded_len(const struct aws_byte_cursor *encoded, size_t *decoded_len) {
    if (!encoded) {
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }
    if (!decoded_len) {
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }

    size_t len = encoded->len;
    if (len == 0) {
        *decoded_len = 0;
        return AWS_OP_SUCCESS;
    }

    if (len % 4 != 0) {
        aws_raise_error(AWS_ERROR_INVALID_BASE64_STR);
        return AWS_OP_ERR;
    }

    size_t padding = 0;
    if (len >= 1 && encoded->ptr[len - 1] == '=') {
        padding++;
    }
    if (len >= 2 && encoded->ptr[len - 2] == '=') {
        padding++;
    }

    *decoded_len = (len / 4) * 3 - padding;
    return AWS_OP_SUCCESS;
}

int aws_base64_compute_encoded_len(size_t data_len, size_t *encoded_len) {
    if (!encoded_len) {
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }
    *encoded_len = (data_len + 2) / 3 * 4;
    return AWS_OP_SUCCESS;
}

int aws_base64_encode(const struct aws_byte_cursor *to_encode, struct aws_byte_buf *dest) {
    if (!to_encode) {
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }
    if (!dest) {
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }

    size_t encoded_len;
    if (aws_base64_compute_encoded_len(to_encode->len, &encoded_len)) {
        return AWS_OP_ERR;
    }

    if (encoded_len > dest->capacity - dest->len) {
        aws_raise_error(AWS_ERROR_SHORT_BUFFER);
        return AWS_OP_ERR;
    }

    size_t cur_input_index = 0;
    size_t input_len = to_encode->len;
    const uint8_t *input_ptr = to_encode->ptr;
    uint8_t *output_ptr = dest->buffer + dest->len;

    while (cur_input_index < input_len) {
        uint32_t byte_a = cur_input_index < input_len ? (unsigned char)input_ptr[cur_input_index++] : 0;
        uint32_t byte_b = cur_input_index < input_len ? (unsigned char)input_ptr[cur_input_index++] : 0;
        uint32_t byte_c = cur_input_index < input_len ? (unsigned char)input_ptr[cur_input_index++] : 0;
        uint32_t concat_bytes = (byte_a << 16) | (byte_b << 8) | byte_c;

        *output_ptr++ = s_base64_encoding_table[(concat_bytes >> 18) & 0x3F];
        *output_ptr++ = s_base64_encoding_table[(concat_bytes >> 12) & 0x3F];
        *output_ptr++ = s_base64_encoding_table[(concat_bytes >> 6) & 0x3F];
        *output_ptr++ = s_base64_encoding_table[concat_bytes & 0x3F];
    }

    size_t padding = (3 - (input_len % 3)) % 3;
    for (size_t i = 0; i < padding; ++i) {
        output_ptr[-1 - i] = '=';
    }

    dest->len += encoded_len;
    return AWS_OP_SUCCESS;
}

int aws_base64_decode(const struct aws_byte_cursor *to_decode, struct aws_byte_buf *dest) {
    if (!to_decode) {
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }
    if (!dest) {
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }

    size_t decoded_len;
    if (aws_base64_compute_decoded_len(to_decode, &decoded_len)) {
        return AWS_OP_ERR;
    }

    if (decoded_len > dest->capacity - dest->len) {
        aws_raise_error(AWS_ERROR_SHORT_BUFFER);
        return AWS_OP_ERR;
    }

    size_t cur_input_index = 0;
    size_t input_len = to_decode->len;
    const uint8_t *input_ptr = to_decode->ptr;
    uint8_t *output_ptr = dest->buffer + dest->len;

    while (cur_input_index < input_len) {
        uint8_t byte_a = s_base64_decoding_table[input_ptr[cur_input_index++]];
        uint8_t byte_b = s_base64_decoding_table[input_ptr[cur_input_index++]];
        uint8_t byte_c = s_base64_decoding_table[input_ptr[cur_input_index++]];
        uint8_t byte_d = s_base64_decoding_table[input_ptr[cur_input_index++]];

        if ((byte_a | byte_b | byte_c | byte_d) & 0x80) {
            aws_raise_error(AWS_ERROR_INVALID_BASE64_STR);
            return AWS_OP_ERR;
        }

        uint32_t concat_bytes = (uint32_t)(byte_a << 18) + (uint32_t)(byte_b << 12) + (uint32_t)(byte_c << 6) + byte_d;

        *output_ptr++ = (concat_bytes >> 16) & 0xFF;
        if (byte_c != 0x80) {
            *output_ptr++ = (concat_bytes >> 8) & 0xFF;
        }
        if (byte_d != 0x80) {
            *output_ptr++ = concat_bytes & 0xFF;
        }
    }

    dest->len += decoded_len;
    return AWS_OP_SUCCESS;
}

int aws_hex_compute_decoded_len(const struct aws_byte_cursor *encoded, size_t *decoded_len) {
    if (!encoded) {
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }
    if (!decoded_len) {
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }

    if (encoded->len % 2 != 0) {
        aws_raise_error(AWS_ERROR_INVALID_HEX_STR);
        return AWS_OP_ERR;
    }

    *decoded_len = encoded->len / 2;
    return AWS_OP_SUCCESS;
}

int aws_hex_compute_encoded_len(size_t data_len, size_t *encoded_len) {
    if (!encoded_len) {
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }
    *encoded_len = data_len * 2;
    return AWS_OP_SUCCESS;
}

int aws_hex_encode(const struct aws_byte_cursor *to_encode, struct aws_byte_buf *dest) {
    if (!to_encode) {
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }
    if (!dest) {
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }

    size_t encoded_len;
    if (aws_hex_compute_encoded_len(to_encode->len, &encoded_len)) {
        return AWS_OP_ERR;
    }

    if (encoded_len > dest->capacity - dest->len) {
        aws_raise_error(AWS_ERROR_SHORT_BUFFER);
        return AWS_OP_ERR;
    }

    const uint8_t *input_ptr = to_encode->ptr;
    uint8_t *output_ptr = dest->buffer + dest->len;

    for (size_t i = 0; i < to_encode->len; ++i) {
        uint8_t byte = input_ptr[i];
        *output_ptr++ = s_hex_encoding_table[byte >> 4];
        *output_ptr++ = s_hex_encoding_table[byte & 0x0F];
    }

    dest->len += encoded_len;
    return AWS_OP_SUCCESS;
}

int aws_byte_buf_append_encoding_to_hex(struct aws_byte_buf *dest, const struct aws_byte_cursor *to_encode) {
    return aws_hex_encode(to_encode, dest);
}

int aws_hex_decode(const struct aws_byte_cursor *to_decode, struct aws_byte_buf *dest) {
    if (!to_decode) {
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }
    if (!dest) {
        return aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
    }

    size_t decoded_len;
    if (aws_hex_compute_decoded_len(to_decode, &decoded_len)) {
        return AWS_OP_ERR;
    }

    if (decoded_len > dest->capacity - dest->len) {
        aws_raise_error(AWS_ERROR_SHORT_BUFFER);
        return AWS_OP_ERR;
    }

    const uint8_t *input_ptr = to_decode->ptr;
    uint8_t *output_ptr = dest->buffer + dest->len;

    for (size_t i = 0; i < decoded_len; ++i) {
        uint8_t byte_a = s_hex_decoding_table[input_ptr[i * 2]];
        uint8_t byte_b = s_hex_decoding_table[input_ptr[i * 2 + 1]];

        if ((byte_a | byte_b) & 0x80) {
            aws_raise_error(AWS_ERROR_INVALID_HEX_STR);
            return AWS_OP_ERR;
        }

        *output_ptr++ = (byte_a << 4) + byte_b;
    }

    dest->len += decoded_len;
    return AWS_OP_SUCCESS;
}
