#ifndef AWS_COMMON_MEMORY_H
#define AWS_COMMON_MEMORY_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/exports.h> // We'll create this later for symbol visibility
#include <stddef.h> // For size_t

AWS_EXTERN_C_BEGIN // We'll define this later

/**
 * @brief Allocates memory of the specified size.
 *
 * This function behaves like malloc but allows for custom memory tracking or
 * allocation strategies in the future.
 *
 * @param size The number of bytes to allocate.
 * @return A pointer to the allocated memory, or NULL if allocation fails.
 */
AWS_COMMON_API
void *aws_mem_acquire(size_t size);

/**
 * @brief Frees memory previously allocated by aws_mem_acquire.
 *
 * This function behaves like free.
 *
 * @param ptr A pointer to the memory block to free. If ptr is NULL, no
 *            operation is performed.
 */
AWS_COMMON_API
void aws_mem_release(void *ptr);

/**
 * @brief Reallocates a block of memory previously allocated by aws_mem_acquire.
 *
 * This function behaves like realloc.
 *
 * @param ptr A pointer to the memory block to reallocate.
 * @param old_size The original size of the memory block pointed to by ptr.
 *                 (Note: Standard realloc doesn't require old_size, but
 *                  including it can be useful for custom allocators).
 * @param new_size The new size for the memory block, in bytes.
 * @return A pointer to the reallocated memory block, or NULL if reallocation fails.
 *         If reallocation fails, the original block pointed to by ptr is not freed.
 */
AWS_COMMON_API
void *aws_mem_realloc(void *ptr, size_t old_size, size_t new_size);

/**
 * @brief Allocates memory for an array of elements, initializing them to zero.
 *
 * This function behaves like calloc.
 *
 * @param num_elements The number of elements to allocate.
 * @param element_size The size of each element, in bytes.
 * @return A pointer to the allocated memory, or NULL if allocation fails.
 *         The memory is set to zero.
 */
AWS_COMMON_API
void *aws_mem_calloc(size_t num_elements, size_t element_size);

AWS_EXTERN_C_END

#endif /* AWS_COMMON_MEMORY_H */
