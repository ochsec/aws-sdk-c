/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/memory.h>
#include <stdlib.h> // For malloc, free, realloc, calloc

void *aws_mem_acquire(size_t size) {
    // TODO: Add custom allocation logic/tracking if needed
    return malloc(size);
}

void aws_mem_release(void *ptr) {
    // TODO: Add custom deallocation logic/tracking if needed
    free(ptr);
}

void *aws_mem_realloc(void *ptr, size_t old_size, size_t new_size) {
    (void)old_size; // Suppress unused parameter warning for now
    // TODO: Add custom reallocation logic/tracking if needed
    return realloc(ptr, new_size);
}

void *aws_mem_calloc(size_t num_elements, size_t element_size) {
    // TODO: Add custom allocation logic/tracking if needed
    return calloc(num_elements, element_size);
}
