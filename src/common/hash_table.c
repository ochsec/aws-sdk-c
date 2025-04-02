/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/hash_table.h>
#include <aws/common/private/hash_table_impl.h> /* Internal definitions */
#include <aws/common/math.h> /* For aws_is_power_of_two, aws_round_up_to_power_of_two */
#include <aws/common/error.h>
#include <aws/common/string.h> /* For aws_string_destroy */

#include <string.h> /* For memcpy, memset */
#include <stdlib.h> /* For NULL */
#include <assert.h> /* For assert */

/* Define sentinel values for hash table slots */
/* A slot is EMPTY if key is NULL */
#define AWS_HASH_EMPTY_KEY NULL
/* A slot is DELETED if key is this special pointer */
static const void *DELETED_KEY = (void *)1;

/* Default load factor */
#define AWS_HASH_TABLE_DEFAULT_MAX_LOAD_FACTOR 0.7

/* Internal: Find a slot for a key (for put, find, remove) */
/* Returns AWS_OP_SUCCESS if a slot (empty or matching) is found, stores index in *slot_index */
/* Returns AWS_OP_ERR if table is full and key not found */
static int s_find_slot(
    const struct aws_hash_table *map,
    const void *key,
    uint64_t hash_code,
    size_t *slot_index) {

    assert(map->capacity > 0);
    assert(aws_is_power_of_two(map->capacity));

    size_t mask = map->capacity - 1;
    size_t index = (size_t)hash_code & mask;
    size_t first_deleted = SIZE_MAX; /* Track first deleted slot found */

    for (size_t i = 0; i < map->capacity; ++i) {
        struct aws_hash_element *slot = &map->slots[index];

        if (slot->key == AWS_HASH_EMPTY_KEY) {
            /* Found empty slot */
            *slot_index = (first_deleted != SIZE_MAX) ? first_deleted : index;
            return AWS_OP_SUCCESS; /* Key not present, return empty/deleted slot */
        }

        if (slot->key == DELETED_KEY) {
            /* Found deleted slot, keep searching for key but remember this slot */
            if (first_deleted == SIZE_MAX) {
                first_deleted = index;
            }
        } else if (map->equals_fn(key, slot->key)) {
            /* Found matching key */
            *slot_index = index;
            return AWS_OP_SUCCESS; /* Key present */
        }

        /* Linear probe */
        index = (index + 1) & mask;
    }

    /* Table is full, and key not found */
    /* If we found a deleted slot earlier, we could potentially use it, */
    /* but standard open addressing usually resizes before this point. */
    /* For simplicity here, we'll treat full as an error if key not found. */
     if (first_deleted != SIZE_MAX) {
         *slot_index = first_deleted;
         return AWS_OP_SUCCESS; /* Key not present, return deleted slot */
     }

    return AWS_OP_ERR; /* Table full */
}

/* Internal: Resize the hash table */
static int s_hash_table_resize(struct aws_hash_table *map, size_t new_capacity) {
    assert(aws_is_power_of_two(new_capacity));

    struct aws_hash_element *old_slots = map->slots;
    size_t old_capacity = map->capacity;

    /* Allocate new slots array */
    map->slots = aws_mem_calloc_via(map->allocator, new_capacity, sizeof(struct aws_hash_element));
    if (!map->slots) {
        map->slots = old_slots; /* Restore old state */
        return AWS_OP_ERR; /* OOM already raised */
    }
    map->capacity = new_capacity;
    map->size = 0; /* Reset size, will be incremented during rehash */

    /* Rehash elements from old table to new table */
    for (size_t i = 0; i < old_capacity; ++i) {
        struct aws_hash_element *old_slot = &old_slots[i];
        if (old_slot->key != AWS_HASH_EMPTY_KEY && old_slot->key != DELETED_KEY) {
            /* Find slot in new table */
            uint64_t hash_code = map->hash_fn(old_slot->key);
            size_t new_index;
            /* Resize should guarantee success unless OOM happened above */
            if (s_find_slot(map, old_slot->key, hash_code, &new_index) != AWS_OP_SUCCESS) {
                 /* This shouldn't happen if allocation succeeded and new_capacity > old size */
                 /* Clean up partially resized table? For now, assert */
                 assert(false && "Failed to find slot during resize");
                 aws_mem_release_via(map->allocator, map->slots);
                 map->slots = old_slots;
                 map->capacity = old_capacity;
                 /* Need to restore size too - complex recovery */
                 aws_raise_error(AWS_ERROR_UNKNOWN);
                 return AWS_OP_ERR;
            }
            /* Move element to new slot */
            map->slots[new_index] = *old_slot;
            map->size++;
        }
    }

    /* Free old slots array */
    aws_mem_release_via(map->allocator, old_slots);
    return AWS_OP_SUCCESS;
}


int aws_hash_table_init(
    struct aws_hash_table *map,
    struct aws_allocator *allocator,
    size_t initial_capacity,
    aws_hash_fn hash_fn,
    aws_hash_equals_fn equals_fn,
    aws_hash_element_destroy_fn destroy_key_fn,
    aws_hash_element_destroy_fn destroy_value_fn) {

    assert(allocator != NULL);
    assert(map != NULL);
    assert(hash_fn != NULL);
    assert(equals_fn != NULL);

    memset(map, 0, sizeof(struct aws_hash_table));
    map->allocator = allocator;
    map->hash_fn = hash_fn;
    map->equals_fn = equals_fn;
    map->destroy_key_fn = destroy_key_fn;
    map->destroy_value_fn = destroy_value_fn;

    /* Ensure capacity is a power of 2 */
    size_t capacity = aws_max_size(AWS_HASH_TABLE_MIN_CAPACITY, initial_capacity);
    if (!aws_is_power_of_two(capacity)) {
        capacity = aws_round_up_to_power_of_two(capacity);
    }
    map->capacity = capacity;

    map->slots = aws_mem_calloc_via(allocator, map->capacity, sizeof(struct aws_hash_element));
    if (!map->slots) {
        map->capacity = 0;
        return AWS_OP_ERR; /* OOM already raised */
    }

    return AWS_OP_SUCCESS;
}

void aws_hash_table_clean_up(struct aws_hash_table *map) {
    if (!map) {
        return;
    }

    if (map->slots) {
        for (size_t i = 0; i < map->capacity; ++i) {
            struct aws_hash_element *slot = &map->slots[i];
            if (slot->key != AWS_HASH_EMPTY_KEY && slot->key != DELETED_KEY) {
                if (map->destroy_key_fn) {
                    map->destroy_key_fn(slot); /* Pass element for context if needed */
                }
                if (map->destroy_value_fn) {
                    map->destroy_value_fn(slot);
                }
            }
        }
        aws_mem_release_via(map->allocator, map->slots);
    }
    memset(map, 0, sizeof(struct aws_hash_table)); /* Clear struct */
}

int aws_hash_table_find(const struct aws_hash_table *map, const void *key, struct aws_hash_element **p_element) {
    assert(map != NULL);
    assert(key != NULL);
    assert(p_element != NULL);

    *p_element = NULL;
    if (map->size == 0) {
        return AWS_OP_ERR; /* Not found */
    }

    uint64_t hash_code = map->hash_fn(key);
    size_t index;
    if (s_find_slot(map, key, hash_code, &index) != AWS_OP_SUCCESS) {
        return AWS_OP_ERR; /* Should only happen if table is full and key not found */
    }

    struct aws_hash_element *slot = &map->slots[index];
    if (slot->key != AWS_HASH_EMPTY_KEY && slot->key != DELETED_KEY) {
        /* Key found */
        *p_element = slot;
        return AWS_OP_SUCCESS;
    }

    return AWS_OP_ERR; /* Not found (empty or deleted slot returned) */
}

int aws_hash_table_put(struct aws_hash_table *map, const void *key, void *value, int *was_created) {
    assert(map != NULL);
    assert(key != NULL);

    /* Check load factor and resize if necessary */
    if ((map->size + 1) > (size_t)(map->capacity * AWS_HASH_TABLE_DEFAULT_MAX_LOAD_FACTOR)) {
        size_t new_capacity = map->capacity == 0 ? AWS_HASH_TABLE_MIN_CAPACITY : map->capacity * 2;
        if (s_hash_table_resize(map, new_capacity) != AWS_OP_SUCCESS) {
            return AWS_OP_ERR; /* Resize failed (OOM) */
        }
    }

    uint64_t hash_code = map->hash_fn(key);
    size_t index;
    if (s_find_slot(map, key, hash_code, &index) != AWS_OP_SUCCESS) {
        /* Should not happen after resize check unless table was already full */
        aws_raise_error(AWS_ERROR_UNKNOWN); /* Or a specific "table full" error */
        return AWS_OP_ERR;
    }

    struct aws_hash_element *slot = &map->slots[index];
    bool is_new_element = (slot->key == AWS_HASH_EMPTY_KEY || slot->key == DELETED_KEY);

    if (is_new_element) {
        /* Insert new element */
        slot->key = key;
        slot->value = value;
        map->size++;
        if (was_created) *was_created = 1;
    } else {
        /* Update existing element */
        /* Destroy old value if applicable */
        if (map->destroy_value_fn) {
            map->destroy_value_fn(slot);
        }
        slot->value = value;
        /* Key remains the same, destroy_key_fn not called */
        if (was_created) *was_created = 0;
    }

    return AWS_OP_SUCCESS;
}

int aws_hash_table_remove(struct aws_hash_table *map, const void *key, struct aws_hash_element *p_element_out) {
     assert(map != NULL);
     assert(key != NULL);

     if (map->size == 0) {
         return AWS_OP_ERR; /* Not found */
     }

     uint64_t hash_code = map->hash_fn(key);
     size_t index;
     if (s_find_slot(map, key, hash_code, &index) != AWS_OP_SUCCESS) {
         return AWS_OP_ERR; /* Table full and key not found */
     }

     struct aws_hash_element *slot = &map->slots[index];
     if (slot->key == AWS_HASH_EMPTY_KEY || slot->key == DELETED_KEY) {
         return AWS_OP_ERR; /* Key not found */
     }

     /* Key found, mark as deleted */
     if (p_element_out) {
         /* Copy element data out if requested */
         *p_element_out = *slot;
     } else {
         /* Destroy element if not returned to caller */
         if (map->destroy_key_fn) {
             map->destroy_key_fn(slot);
         }
         if (map->destroy_value_fn) {
             map->destroy_value_fn(slot);
         }
     }

     slot->key = DELETED_KEY; /* Mark as deleted */
     slot->value = NULL;
     map->size--;

     return AWS_OP_SUCCESS;
}

size_t aws_hash_table_get_entry_count(const struct aws_hash_table *map) {
    assert(map != NULL);
    return map->size;
}


/* --- Predefined hash/equals/destroy --- */

/* FNV-1a 64-bit constants */
#define FNV_OFFSET_BASIS_64 0xCBF29CE484222325ULL
#define FNV_PRIME_64 0x100000001B3ULL

uint64_t aws_hash_byte_cursor_ptr(const void *item) {
    const struct aws_byte_cursor *cursor = item;
    uint64_t hash = FNV_OFFSET_BASIS_64;
    for (size_t i = 0; i < cursor->len; ++i) {
        hash ^= cursor->ptr[i];
        hash *= FNV_PRIME_64;
    }
    return hash;
}

bool aws_byte_cursor_eq(const void *a, const void *b) {
    const struct aws_byte_cursor *cur_a = a;
    const struct aws_byte_cursor *cur_b = b;
    if (cur_a->len != cur_b->len) {
        return false;
    }
    return memcmp(cur_a->ptr, cur_b->ptr, cur_a->len) == 0;
}

uint64_t aws_hash_string_ptr(const void *item) {
     const struct aws_string *str = item;
     /* Hash the underlying bytes */
     struct aws_byte_cursor cursor = aws_byte_cursor_from_array(str->bytes, str->length);
     return aws_hash_byte_cursor_ptr(&cursor);
}

bool aws_string_eq(const void *a, const void *b) {
    const struct aws_string *str_a = a;
    const struct aws_string *str_b = b;
    /* Use existing string compare */
    return aws_string_compare(str_a, str_b) == 0;
}

void aws_hash_string_destroy(struct aws_hash_element *pElement) {
     /* Assumes the key/value stored is the aws_string pointer itself */
     aws_string_destroy((struct aws_string *)pElement->key);
     aws_string_destroy((struct aws_string *)pElement->value);
     /* Be careful: this assumes both key and value are aws_strings */
     /* If only one is, need separate destroy functions */
}
