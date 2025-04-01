/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/array_list.h>
#include <aws/common/error.h>
#include <string.h> /* For memcpy */
#include <stdlib.h> /* For NULL */
#include <assert.h> /* For assert */

/* Default initial capacity if 0 is provided */
#define AWS_ARRAY_LIST_DEFAULT_INITIAL_CAPACITY 16
/* Growth factor when resizing */
#define AWS_ARRAY_LIST_GROWTH_FACTOR 2

/* Internal function to calculate the required memory size */
static size_t s_calculate_allocation_size(size_t capacity, size_t item_size) {
    /* Check for potential overflow */
    if (capacity > 0 && item_size > (SIZE_MAX / capacity)) {
        return 0; /* Indicate overflow */
    }
    return capacity * item_size;
}

/* Internal function to resize the list's buffer */
static int s_array_list_resize(struct aws_array_list *list, size_t new_capacity) {
    size_t required_size = s_calculate_allocation_size(new_capacity, list->item_size);
    if (required_size == 0) {
        aws_raise_error(AWS_ERROR_OOM); /* Overflow */
        return AWS_OP_ERR;
    }

    void *new_data = aws_mem_realloc_via(list->allocator, list->data, list->current_size * list->item_size, required_size);
    if (!new_data) {
        /* aws_mem_realloc_via should have raised AWS_ERROR_OOM */
        return AWS_OP_ERR;
    }

    list->data = new_data;
    list->current_size = new_capacity;
    return AWS_OP_SUCCESS;
}

int aws_array_list_init_dynamic(
    struct aws_array_list *list,
    struct aws_allocator *allocator,
    size_t initial_capacity,
    size_t item_size) {

    assert(allocator != NULL);
    assert(item_size > 0);
    assert(list != NULL);

    list->allocator = allocator;
    list->item_size = item_size;
    list->length = 0;

    if (initial_capacity == 0) {
        initial_capacity = AWS_ARRAY_LIST_DEFAULT_INITIAL_CAPACITY;
    }

    list->current_size = initial_capacity;
    size_t required_size = s_calculate_allocation_size(list->current_size, item_size);
    if (required_size == 0) {
        list->data = NULL;
        list->current_size = 0;
        aws_raise_error(AWS_ERROR_OOM); /* Overflow */
        return AWS_OP_ERR;
    }

    list->data = aws_mem_acquire_via(allocator, required_size);
    if (!list->data) {
        list->current_size = 0;
        /* aws_mem_acquire_via should have raised AWS_ERROR_OOM */
        return AWS_OP_ERR;
    }

    return AWS_OP_SUCCESS;
}

void aws_array_list_clean_up(struct aws_array_list *list) {
    if (list && list->data) {
        aws_mem_release_via(list->allocator, list->data);
        list->data = NULL;
        list->allocator = NULL;
        list->current_size = 0;
        list->length = 0;
        list->item_size = 0;
    }
}

int aws_array_list_get_at_ptr(const struct aws_array_list *list, void **val, size_t index) {
    assert(list != NULL);
    assert(val != NULL);

    if (index >= list->length) {
        aws_raise_error(AWS_ERROR_INVALID_INDEX); /* Defined this error */
        return AWS_OP_ERR;
    }

    *val = (uint8_t *)list->data + (index * list->item_size);
    return AWS_OP_SUCCESS;
}

int aws_array_list_get_at(const struct aws_array_list *list, void *val, size_t index) {
     assert(list != NULL);
     assert(val != NULL);

    void *src_ptr = NULL;
    if (aws_array_list_get_at_ptr(list, &src_ptr, index) != AWS_OP_SUCCESS) {
        return AWS_OP_ERR; /* Error already raised by get_at_ptr */
    }

    memcpy(val, src_ptr, list->item_size);
    return AWS_OP_SUCCESS;
}

int aws_array_list_push_back(struct aws_array_list *list, const void *val) {
    assert(list != NULL);
    assert(val != NULL);

    if (list->length == list->current_size) {
        /* Resize needed */
        size_t new_capacity = list->current_size == 0 ? AWS_ARRAY_LIST_DEFAULT_INITIAL_CAPACITY
                                                     : list->current_size * AWS_ARRAY_LIST_GROWTH_FACTOR;
        if (s_array_list_resize(list, new_capacity) != AWS_OP_SUCCESS) {
            return AWS_OP_ERR; /* Error already raised */
        }
    }

    /* Copy element to the end */
    void *dest_ptr = (uint8_t *)list->data + (list->length * list->item_size);
    memcpy(dest_ptr, val, list->item_size);
    list->length++;

    return AWS_OP_SUCCESS;
}

int aws_array_list_pop_back(struct aws_array_list *list) {
    assert(list != NULL);
    if (list->length == 0) {
        aws_raise_error(AWS_ERROR_LIST_EMPTY); /* Defined this error */
        return AWS_OP_ERR;
    }
    list->length--;
    /* Note: We don't shrink the list automatically on pop */
    return AWS_OP_SUCCESS;
}

size_t aws_array_list_length(const struct aws_array_list *list) {
    assert(list != NULL);
    return list->length;
}

size_t aws_array_list_capacity(const struct aws_array_list *list) {
    assert(list != NULL);
    return list->current_size;
}

int aws_array_list_reserve(struct aws_array_list *list, size_t capacity) {
    assert(list != NULL);
    if (capacity > list->current_size) {
        return s_array_list_resize(list, capacity);
    }
    return AWS_OP_SUCCESS; /* Already have enough capacity */
}
