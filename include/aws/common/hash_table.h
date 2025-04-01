#ifndef AWS_COMMON_HASH_TABLE_H
#define AWS_COMMON_HASH_TABLE_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/exports.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h> /* For aws_byte_cursor used as keys */
#include <stddef.h> /* size_t */
#include <stdint.h> /* uint64_t */

AWS_EXTERN_C_BEGIN

/* Forward declarations */
struct aws_hash_table;
struct aws_hash_element; /* Represents an item stored in the table */

/**
 * @brief Function pointer for hashing keys.
 * @param key Pointer to the key data.
 * @return A 64-bit hash code.
 */
typedef uint64_t (*aws_hash_fn)(const void *key);

/**
 * @brief Function pointer for comparing keys for equality.
 * @param a Pointer to the first key.
 * @param b Pointer to the second key.
 * @return True if keys are equal, false otherwise.
 */
typedef bool (*aws_hash_equals_fn)(const void *a, const void *b);

/**
 * @brief Function pointer for destroying key/value data when an element is removed or the table is destroyed.
 * @param pElement Pointer to the element whose data needs destroying.
 */
typedef void (*aws_hash_element_destroy_fn)(struct aws_hash_element *pElement);

/**
 * @brief Represents an element (key-value pair) stored in the hash table.
 * Users typically interact with key/value directly, this is internal.
 */
struct aws_hash_element {
    const void *key;
    void *value;
    /* Internal bookkeeping follows */
};

/**
 * @brief Represents a hash table implementation.
 * Uses open addressing with linear probing for collision resolution (example).
 */
struct aws_hash_table {
    struct aws_allocator *allocator;
    aws_hash_fn hash_fn;
    aws_hash_equals_fn equals_fn;
    aws_hash_element_destroy_fn destroy_key_fn;   /* Optional: called if key needs cleanup */
    aws_hash_element_destroy_fn destroy_value_fn; /* Optional: called if value needs cleanup */
    size_t size;        /* Number of elements currently stored */
    size_t capacity;    /* Number of slots in the table */
    struct aws_hash_element *slots; /* Array of slots */
    /* TODO: Add load factor, resize thresholds etc. */
};

/**
 * @brief Initializes a hash table.
 *
 * @param map Pointer to the hash table structure to initialize.
 * @param allocator Allocator to use.
 * @param initial_capacity Initial number of slots. Will be rounded up to a power of 2.
 * @param hash_fn Function to compute hash codes for keys.
 * @param equals_fn Function to compare keys for equality.
 * @param destroy_key_fn Optional function to clean up keys when removed/table destroyed.
 * @param destroy_value_fn Optional function to clean up values when removed/table destroyed.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 */
AWS_COMMON_API
int aws_hash_table_init(
    struct aws_hash_table *map,
    struct aws_allocator *allocator,
    size_t initial_capacity,
    aws_hash_fn hash_fn,
    aws_hash_equals_fn equals_fn,
    aws_hash_element_destroy_fn destroy_key_fn,
    aws_hash_element_destroy_fn destroy_value_fn);

/**
 * @brief Cleans up the resources used by a hash table, freeing internal memory
 *        and calling destroy functions for remaining elements.
 *        Does NOT free the map structure itself if it was heap-allocated.
 *
 * @param map Pointer to the hash table to clean up.
 */
AWS_COMMON_API
void aws_hash_table_clean_up(struct aws_hash_table *map);

/**
 * @brief Finds an element in the hash table.
 *
 * @param map Pointer to the hash table.
 * @param key Pointer to the key to find.
 * @param p_element Pointer to store the found element (output). NULL if not found.
 * @return AWS_OP_SUCCESS if found, AWS_OP_ERR if not found.
 */
AWS_COMMON_API
int aws_hash_table_find(const struct aws_hash_table *map, const void *key, struct aws_hash_element **p_element);

/**
 * @brief Creates or updates an element in the hash table.
 *        If the key already exists, its value is updated (old value destroyed if applicable).
 *        If the key doesn't exist, a new element is created.
 *        Makes copies of the key and value according to destroy functions (or assumes ownership if NULL).
 *
 * @param map Pointer to the hash table.
 * @param key Pointer to the key.
 * @param value Pointer to the value.
 * @param was_created Optional output parameter. If non-NULL, set to 1 if a new element was created, 0 otherwise.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 */
AWS_COMMON_API
int aws_hash_table_put(struct aws_hash_table *map, const void *key, void *value, int *was_created);

/**
 * @brief Removes an element from the hash table.
 *
 * @param map Pointer to the hash table.
 * @param key Pointer to the key of the element to remove.
 * @param p_element Optional output parameter. If non-NULL and element found, stores the removed element.
 *                  The caller is responsible for destroying the key/value if destroy functions were NULL.
 * @return AWS_OP_SUCCESS if removed, AWS_OP_ERR if not found.
 */
AWS_COMMON_API
int aws_hash_table_remove(struct aws_hash_table *map, const void *key, struct aws_hash_element *p_element);

/**
 * @brief Gets the number of elements in the hash table.
 * @param map Pointer to the hash table.
 * @return Number of elements.
 */
AWS_COMMON_API
size_t aws_hash_table_get_entry_count(const struct aws_hash_table *map);

/* --- Predefined hash/equals/destroy for common types --- */

/** Hash function for aws_byte_cursor (FNV-1a) */
AWS_COMMON_API
uint64_t aws_hash_byte_cursor_ptr(const void *item);
/** Equals function for aws_byte_cursor */
AWS_COMMON_API
bool aws_byte_cursor_eq(const void *a, const void *b);

/** Hash function for aws_string (FNV-1a) */
AWS_COMMON_API
uint64_t aws_hash_string_ptr(const void *item);
/** Equals function for aws_string */
AWS_COMMON_API
bool aws_string_eq(const void *a, const void *b);
/** Destroy function for aws_string key/value */
AWS_COMMON_API
void aws_hash_string_destroy(struct aws_hash_element *pElement);


AWS_EXTERN_C_END

#endif /* AWS_COMMON_HASH_TABLE_H */
