#ifndef AWS_COMMON_ALLOCATOR_H
#define AWS_COMMON_ALLOCATOR_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/exports.h>
#include <stddef.h> // For size_t

AWS_EXTERN_C_BEGIN

/* Forward declaration */
struct aws_allocator;

/**
 * Function pointer types for memory allocation operations.
 */
typedef void *(*aws_mem_acquire_fn)(struct aws_allocator *allocator, size_t size);
typedef void (*aws_mem_release_fn)(struct aws_allocator *allocator, void *ptr);
typedef void *(*aws_mem_realloc_fn)(struct aws_allocator *allocator, void *ptr, size_t old_size, size_t new_size);
typedef void *(*aws_mem_calloc_fn)(struct aws_allocator *allocator, size_t num_elements, size_t element_size);

/**
 * @brief Represents a memory allocator implementation.
 *
 * Allows users to provide custom memory management strategies.
 * The `impl` pointer can point to implementation-specific data.
 */
struct aws_allocator {
    aws_mem_acquire_fn mem_acquire;
    aws_mem_release_fn mem_release;
    aws_mem_realloc_fn mem_realloc;
    aws_mem_calloc_fn mem_calloc;
    void *impl; /* Implementation-specific data */
};

/**
 * @brief Returns the default memory allocator (using standard library functions).
 *
 * @return A pointer to the default allocator instance.
 */
AWS_COMMON_API
struct aws_allocator *aws_default_allocator(void);

/* Convenience functions using a specific allocator */

AWS_COMMON_API
void *aws_mem_acquire_via(struct aws_allocator *allocator, size_t size);

AWS_COMMON_API
void aws_mem_release_via(struct aws_allocator *allocator, void *ptr);

AWS_COMMON_API
void *aws_mem_realloc_via(struct aws_allocator *allocator, void *ptr, size_t old_size, size_t new_size);

AWS_COMMON_API
void *aws_mem_calloc_via(struct aws_allocator *allocator, size_t num_elements, size_t element_size);


AWS_EXTERN_C_END

#endif /* AWS_COMMON_ALLOCATOR_H */
