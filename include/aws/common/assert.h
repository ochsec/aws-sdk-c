#ifndef AWS_COMMON_ASSERT_H
#define AWS_COMMON_ASSERT_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/exports.h>
#include <assert.h> /* Use standard assert for now */

AWS_EXTERN_C_BEGIN

/*
 * Wrapper around standard assert.
 * Can be replaced with custom assertion logic later (e.g., logging,
 * specific crash handling, or disabling in release builds).
 */
#ifdef NDEBUG
/* Release build: assertions disabled */
#define AWS_ASSERT(condition) ((void)0)
#else
/* Debug build: use standard assert */
#define AWS_ASSERT(condition) assert(condition)
#endif /* NDEBUG */


AWS_EXTERN_C_END

#endif /* AWS_COMMON_ASSERT_H */
