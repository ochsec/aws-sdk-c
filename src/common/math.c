/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/math.h>
#include <assert.h> /* For assert */

size_t aws_min_size(size_t a, size_t b) {
    return a < b ? a : b;
}

size_t aws_max_size(size_t a, size_t b) {
    return a > b ? a : b;
}

bool aws_is_power_of_two(size_t size) {
    /* 0 is not a power of 2 */
    if (size == 0) {
        return false;
    }
    /* Check if size has only one bit set */
    return (size & (size - 1)) == 0;
}

size_t aws_round_up_to_power_of_two(size_t size) {
    if (size == 0) {
        return 0; /* Or maybe return 1? Depends on desired behavior for 0. */
    }

    /* Check if already a power of two */
    if (aws_is_power_of_two(size)) {
        return size;
    }

    /* Start with 1 and keep shifting left until >= size */
    size_t power_of_two = 1;
    while (power_of_two < size) {
        /* Check for potential overflow before shifting */
        if (power_of_two > (SIZE_MAX >> 1)) {
            return 0; /* Overflow */
        }
        power_of_two <<= 1;
    }
    return power_of_two;
}
