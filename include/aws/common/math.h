#ifndef AWS_COMMON_MATH_H
#define AWS_COMMON_MATH_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/exports.h>
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* UINT64_MAX */
#include <stdbool.h> /* bool */

AWS_EXTERN_C_BEGIN

/** Returns the lesser of two size_t values */
AWS_COMMON_API
size_t aws_min_size(size_t a, size_t b);

/** Returns the greater of two size_t values */
AWS_COMMON_API
size_t aws_max_size(size_t a, size_t b);

/** Returns true if size is a power of 2, false otherwise. 0 returns false. */
AWS_COMMON_API
bool aws_is_power_of_two(size_t size);

/** Rounds size up to the nearest power of 2. */
/* Returns 0 if size is 0 or rounding up would overflow size_t. */
AWS_COMMON_API
size_t aws_round_up_to_power_of_two(size_t size);

/* TODO: Add more math functions as needed (e.g., safe addition/multiplication with overflow checks) */

/**
 * @brief Multiplies a * b. Returns true on success.
 *        Returns false if overflow would occur. Result is stored in *result.
 */
AWS_COMMON_API
bool aws_mul_size_checked(size_t a, size_t b, size_t *result);

AWS_EXTERN_C_END

#endif /* AWS_COMMON_MATH_H */
