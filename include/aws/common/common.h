/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#ifndef AWS_COMMON_COMMON_H
#define AWS_COMMON_COMMON_H

#include <aws/common/exports.h>
#include <aws/common/error.h>

/* Package IDs used for error code and log subject ranges */
#define AWS_C_COMMON_PACKAGE_ID 0

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h> /* For memset */

AWS_EXTERN_C_BEGIN

/* Define common macros */
/* Use variadic macros to ignore optional description argument */
#define AWS_PRECONDITION(...) AWS_ASSERT(AWS_INTERNAL_FIRST_ARG(__VA_ARGS__))
#define AWS_POSTCONDITION(...) AWS_ASSERT(AWS_INTERNAL_FIRST_ARG(__VA_ARGS__))
/* Helper macro to extract the first argument from __VA_ARGS__ */
#define AWS_INTERNAL_FIRST_ARG(first, ...) first

/* AWS_ZERO_STRUCT is defined in zero.h */

/* Compiler hints for branch prediction */
#define AWS_LIKELY(x) __builtin_expect(!!(x), 1)
#define AWS_UNLIKELY(x) __builtin_expect(!!(x), 0)

#ifdef _MSC_VER
#    define AWS_PUSH_SANE_WARNING_LEVEL                                                                                    \
        __pragma(warning(push)) __pragma(warning(disable : 4820)) /* padding added to struct */                            \
            __pragma(warning(disable : 4514)) /* unreferenced inline function has been removed */                          \
                __pragma(warning(disable : 5039)) /* reference to potentially throwing function passed to extern C function \
                                                   */
#    define AWS_POP_SANE_WARNING_LEVEL __pragma(warning(pop))
#else
#    define AWS_PUSH_SANE_WARNING_LEVEL
#    define AWS_POP_SANE_WARNING_LEVEL
#endif /* _MSC_VER */

/* Controls inline behavior for static functions in headers */
#if defined(AWS_COMMON_EXPORTS)
#    define AWS_STATIC_IMPL AWS_COMMON_API
#else
#    define AWS_STATIC_IMPL static inline
#endif

/* Define AWS_RESTRICT for C99+ (Assuming C11 based on CMakeLists.txt) */
#define AWS_RESTRICT restrict

/* Forward declare common types */
struct aws_allocator;

/* Memory management functions */
AWS_COMMON_API void *aws_mem_acquire(struct aws_allocator *allocator, unsigned int size);
AWS_COMMON_API void aws_mem_release(struct aws_allocator *allocator, void *ptr);
AWS_COMMON_API void *aws_mem_calloc(struct aws_allocator *allocator, unsigned int num, unsigned int size);

/* Reset error function (not in error.h) */
AWS_COMMON_API void aws_reset_error(void);

AWS_EXTERN_C_END

#endif /* AWS_COMMON_COMMON_H */
