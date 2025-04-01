/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/allocator.h>
#include <aws/common/memory.h> // For the underlying memory functions
#include <stdlib.h> // For NULL

/* Default allocator implementation functions */
static void *s_default_mem_acquire(struct aws_allocator *allocator, size_t size) {
    (void)allocator; /* Unused */
    return aws_mem_acquire(size);
}

static void s_default_mem_release(struct aws_allocator *allocator, void *ptr) {
    (void)allocator; /* Unused */
    aws_mem_release(ptr);
}

static void *s_default_mem_realloc(struct aws_allocator *allocator, void *ptr, size_t old_size, size_t new_size) {
    (void)allocator; /* Unused */
    return aws_mem_realloc(ptr, old_size, new_size);
}

static void *s_default_mem_calloc(struct aws_allocator *allocator, size_t num_elements, size_t element_size) {
    (void)allocator; /* Unused */
    return aws_mem_calloc(num_elements, element_size);
}

/* The default allocator instance */
static struct aws_allocator s_default_allocator_instance = {
    .mem_acquire = s_default_mem_acquire,
    .mem_release = s_default_mem_release,
    .mem_realloc = s_default_mem_realloc,
    .mem_calloc = s_default_mem_calloc,
    .impl = NULL,
};

struct aws_allocator *aws_default_allocator(void) {
    return &s_default_allocator_instance;
}

/* Convenience function implementations */
void *aws_mem_acquire_via(struct aws_allocator *allocator, size_t size) {
    if (!allocator) {
        allocator = aws_default_allocator();
    }
    return allocator->mem_acquire(allocator, size);
}

void aws_mem_release_via(struct aws_allocator *allocator, void *ptr) {
     if (!allocator) {
        allocator = aws_default_allocator();
    }
   if (ptr) { /* Avoid calling release on NULL */
       allocator->mem_release(allocator, ptr);
   }
}

void *aws_mem_realloc_via(struct aws_allocator *allocator, void *ptr, size_t old_size, size_t new_size) {
     if (!allocator) {
        allocator = aws_default_allocator();
    }
    return allocator->mem_realloc(allocator, ptr, old_size, new_size);
}

void *aws_mem_calloc_via(struct aws_allocator *allocator, size_t num_elements, size_t element_size) {
     if (!allocator) {
        allocator = aws_default_allocator();
    }
    return allocator->mem_calloc(allocator, num_elements, element_size);
}
