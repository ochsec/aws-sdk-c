/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#ifndef AWS_COMMON_COMMON_H
#define AWS_COMMON_COMMON_H

#include <aws/common/exports.h>
#include <aws/common/error.h>

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

AWS_EXTERN_C_BEGIN

/* Define common macros */
#define AWS_PRECONDITION(cond) do { if (!(cond)) { return NULL; } } while (0)
#define AWS_POSTCONDITION(cond) do { if (!(cond)) { return NULL; } } while (0)

/* Define common types */
struct aws_allocator {
    void *(*mem_acquire)(struct aws_allocator *allocator, unsigned int size);
    void (*mem_release)(struct aws_allocator *allocator, void *ptr);
    void *(*mem_realloc)(struct aws_allocator *allocator, void *ptr, unsigned int old_size, unsigned int new_size);
    void *impl;
};

/* Memory management functions */
AWS_COMMON_API void *aws_mem_acquire(struct aws_allocator *allocator, unsigned int size);
AWS_COMMON_API void aws_mem_release(struct aws_allocator *allocator, void *ptr);
AWS_COMMON_API void *aws_mem_calloc(struct aws_allocator *allocator, unsigned int num, unsigned int size);

/* Reset error function (not in error.h) */
AWS_COMMON_API void aws_reset_error(void);

AWS_EXTERN_C_END

#endif /* AWS_COMMON_COMMON_H */
