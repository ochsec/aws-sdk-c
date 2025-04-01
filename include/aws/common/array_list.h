#ifndef AWS_COMMON_ARRAY_LIST_H
#define AWS_COMMON_ARRAY_LIST_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/exports.h>
#include <aws/common/allocator.h>
#include <stddef.h> /* size_t */

AWS_EXTERN_C_BEGIN

/**
 * @brief Represents a dynamic array (similar to std::vector).
 *
 * Stores elements of a fixed size contiguously in memory. Automatically
 * resizes as needed.
 */
struct aws_array_list {
    struct aws_allocator *allocator;
    size_t current_size; /* Current number of elements stored */
    size_t length;       /* Number of elements currently initialized/used */
    size_t item_size;    /* Size of each element in bytes */
    void *data;          /* Pointer to the contiguous block of memory */
};

/**
 * @brief Initializes a dynamic array list.
 *
 * @param list Pointer to the list structure to initialize.
 * @param allocator Allocator to use for the list's memory.
 * @param initial_capacity Initial number of elements to allocate space for.
 * @param item_size Size of each element in bytes.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure (e.g., OOM).
 */
AWS_COMMON_API
int aws_array_list_init_dynamic(
    struct aws_array_list *list,
    struct aws_allocator *allocator,
    size_t initial_capacity,
    size_t item_size);

/**
 * @brief Cleans up the resources used by an array list, freeing its internal memory.
 *        Does NOT free the list structure itself if it was heap-allocated.
 *
 * @param list Pointer to the list to clean up.
 */
AWS_COMMON_API
void aws_array_list_clean_up(struct aws_array_list *list);

/**
 * @brief Gets a pointer to the element at the specified index.
 *
 * Performs bounds checking.
 *
 * @param list Pointer to the list.
 * @param index Index of the element to retrieve.
 * @param val Pointer to a void* where the address of the element will be stored.
 * @return AWS_OP_SUCCESS if the index is valid, AWS_OP_ERR otherwise (index out of bounds).
 */
AWS_COMMON_API
int aws_array_list_get_at_ptr(const struct aws_array_list *list, void **val, size_t index);

/**
 * @brief Copies the element at the specified index into the provided memory location.
 *
 * Performs bounds checking.
 *
 * @param list Pointer to the list.
 * @param val Pointer to the memory location where the element should be copied.
 *            Must have enough space allocated (list->item_size).
 * @param index Index of the element to copy.
 * @return AWS_OP_SUCCESS if the index is valid, AWS_OP_ERR otherwise.
 */
AWS_COMMON_API
int aws_array_list_get_at(const struct aws_array_list *list, void *val, size_t index);

/**
 * @brief Adds an element to the end of the list.
 *
 * Copies the data pointed to by `val` into the list. Resizes the internal
 * buffer if necessary.
 *
 * @param list Pointer to the list.
 * @param val Pointer to the element data to add.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure (e.g., OOM).
 */
AWS_COMMON_API
int aws_array_list_push_back(struct aws_array_list *list, const void *val);

/**
 * @brief Removes the last element from the list.
 *
 * @param list Pointer to the list.
 * @return AWS_OP_SUCCESS if an element was removed, AWS_OP_ERR if the list was empty.
 */
AWS_COMMON_API
int aws_array_list_pop_back(struct aws_array_list *list);

/**
 * @brief Returns the current number of elements in the list.
 *
 * @param list Pointer to the list.
 * @return The number of elements.
 */
AWS_COMMON_API
size_t aws_array_list_length(const struct aws_array_list *list);

/**
 * @brief Returns the current capacity (allocated size) of the list.
 *
 * @param list Pointer to the list.
 * @return The number of elements the list can hold before resizing.
 */
AWS_COMMON_API
size_t aws_array_list_capacity(const struct aws_array_list *list);

/**
 * @brief Reserves capacity for at least `capacity` elements.
 *
 * If the current capacity is already sufficient, no action is taken.
 *
 * @param list Pointer to the list.
 * @param capacity The desired minimum capacity.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure (e.g., OOM).
 */
AWS_COMMON_API
int aws_array_list_reserve(struct aws_array_list *list, size_t capacity);


AWS_EXTERN_C_END

#endif /* AWS_COMMON_ARRAY_LIST_H */
