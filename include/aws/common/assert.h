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

/*
 * Assertion that terminates the program if the condition is false.
 * Calls aws_fatal_assert() function (which should be declared below).
 */
#define AWS_FATAL_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            aws_fatal_assert(#cond, __FILE__, __LINE__); \
        } \
    } while (0)
   
   /* Function called by AWS_FATAL_ASSERT macro on failure */
   AWS_COMMON_API void aws_fatal_assert(const char *cond_str, const char *file, int line);
   
AWS_EXTERN_C_END

#endif /* AWS_COMMON_ASSERT_H */
