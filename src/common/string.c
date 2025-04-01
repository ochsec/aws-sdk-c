/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/string.h>
#include <aws/common/error.h>
#include <string.h> /* For strlen, memcpy, strcmp */
#include <stdlib.h> /* For NULL */
#include <assert.h> /* For assert */

/* Internal function to allocate and initialize string structure */
static struct aws_string *s_aws_string_new_impl(
    struct aws_allocator *allocator,
    const uint8_t *bytes,
    size_t len) {

    struct aws_string *str = aws_mem_acquire_via(allocator, sizeof(struct aws_string));
    if (!str) {
        return NULL; /* OOM handled by aws_mem_acquire_via */
    }

    /* Allocate buffer: length + 1 for null terminator */
    str->bytes = aws_mem_acquire_via(allocator, len + 1);
    if (!str->bytes) {
        aws_mem_release_via(allocator, str);
        return NULL;
    }

    /* Copy data and add null terminator */
    if (len > 0 && bytes) {
        memcpy(str->bytes, bytes, len);
    }
    str->bytes[len] = '\0';

    str->allocator = allocator;
    str->length = len;
    str->capacity = len; /* Initial capacity matches length */

    return str;
}

struct aws_string *aws_string_new_from_c_str(struct aws_allocator *allocator, const char *c_str) {
    if (!c_str) {
        aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
        return NULL;
    }
    size_t len = strlen(c_str);
    return s_aws_string_new_impl(allocator, (const uint8_t *)c_str, len);
}

struct aws_string *aws_string_new_from_bytes(struct aws_allocator *allocator, const uint8_t *bytes, size_t len) {
     if (!bytes && len > 0) {
        aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
        return NULL;
    }
    return s_aws_string_new_impl(allocator, bytes, len);
}

void aws_string_destroy(struct aws_string *str) {
    if (!str) {
        return;
    }
    /* Use the string's own allocator to free its memory */
    struct aws_allocator *allocator = str->allocator;
    aws_mem_release_via(allocator, str->bytes);
    aws_mem_release_via(allocator, str);
}

const char *aws_string_c_str(const struct aws_string *str) {
    assert(str != NULL);
    return str->bytes;
}

size_t aws_string_length(const struct aws_string *str) {
    assert(str != NULL);
    return str->length;
}

int aws_string_compare(const struct aws_string *a, const struct aws_string *b) {
    assert(a != NULL);
    assert(b != NULL);

    size_t len_a = a->length;
    size_t len_b = b->length;
    size_t min_len = len_a < len_b ? len_a : len_b;

    int cmp = memcmp(a->bytes, b->bytes, min_len);
    if (cmp == 0) {
        if (len_a < len_b) {
            return -1;
        }
        if (len_a > len_b) {
            return 1;
        }
        return 0;
    }
    return cmp;
}

int aws_string_compare_c_str(const struct aws_string *str, const char *c_str) {
    assert(str != NULL);
    assert(c_str != NULL);
    return strcmp(str->bytes, c_str);
}
