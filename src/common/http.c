/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/http.h>
#include <aws/common/hash_table.h>
#include <aws/common/string.h>
#include <aws/common/error.h> /* Needed for AWS_OP_SUCCESS/ERR */
#include <aws/common/private/hash_table_impl.h> /* Needed for struct definition */

#include <stdlib.h> /* For NULL */
#include <assert.h> /* For assert */

/* Use aws_string for both key and value in the hash table */
/* Need a destroy function that cleans up both key and value aws_strings */
static void s_header_destroy(struct aws_hash_element *pElement) {
    aws_string_destroy((struct aws_string *)pElement->key);
    aws_string_destroy((struct aws_string *)pElement->value);
}

/* Define the opaque structure using a hash table */
struct aws_http_headers {
    struct aws_hash_table map;
};

struct aws_http_headers *aws_http_headers_new(struct aws_allocator *allocator, size_t initial_capacity) {
    assert(allocator != NULL);

    struct aws_http_headers *headers = aws_mem_acquire_via(allocator, sizeof(struct aws_http_headers));
    if (!headers) {
        return NULL;
    }

    if (aws_hash_table_init(
            &headers->map,
            allocator,
            initial_capacity,
            aws_hash_string_ptr,  /* Use string hash */
            aws_string_eq,        /* Use string equals */
            s_header_destroy,     /* Destroy key (aws_string) */
            s_header_destroy) != AWS_OP_SUCCESS) { /* Destroy value (aws_string) */
        aws_mem_release_via(allocator, headers);
        return NULL;
    }

    return headers;
}

void aws_http_headers_destroy(struct aws_http_headers *headers) {
    if (!headers) {
        return;
    }
    struct aws_allocator *allocator = headers->map.allocator;
    aws_hash_table_clean_up(&headers->map);
    aws_mem_release_via(allocator, headers);
}

int aws_http_headers_set(struct aws_http_headers *headers, const struct aws_string *name, const struct aws_string *value) {
    assert(headers != NULL);
    assert(name != NULL);
    assert(value != NULL);

    /* Create copies to store in the hash table */
    struct aws_string *key_copy = aws_string_new_from_bytes(headers->map.allocator, (const uint8_t*)name->bytes, name->length);
    if (!key_copy) return AWS_OP_ERR;

    struct aws_string *value_copy = aws_string_new_from_bytes(headers->map.allocator, (const uint8_t*)value->bytes, value->length);
    if (!value_copy) {
        aws_string_destroy(key_copy);
        return AWS_OP_ERR;
    }

    int was_created = 0;
    int result = aws_hash_table_put(&headers->map, key_copy, value_copy, &was_created);

    if (result != AWS_OP_SUCCESS) {
        /* If put failed, we need to clean up the copies we made */
        aws_string_destroy(key_copy);
        aws_string_destroy(value_copy);
        return AWS_OP_ERR;
    }
     if (!was_created) {
         /* If the key already existed, put replaces the value but keeps the old key ptr. */
         /* The old value was destroyed by the hash table's destroy_value_fn. */
         /* However, the key_copy we just made is now orphaned, so we must destroy it. */
         aws_string_destroy(key_copy);
     }


    return AWS_OP_SUCCESS;
}

int aws_http_headers_set_c_str(struct aws_http_headers *headers, const char *name, const char *value) {
     assert(headers != NULL);
     assert(name != NULL);
     assert(value != NULL);

     /* Create temporary aws_string wrappers (no allocation needed) */
     struct aws_string *name_str = aws_string_new_from_c_str(headers->map.allocator, name);
     if (!name_str) return AWS_OP_ERR;
     struct aws_string *value_str = aws_string_new_from_c_str(headers->map.allocator, value);
      if (!value_str) {
          aws_string_destroy(name_str);
          return AWS_OP_ERR;
      }

     int result = aws_http_headers_set(headers, name_str, value_str);

     /* Clean up the temporary wrappers */
     aws_string_destroy(name_str);
     aws_string_destroy(value_str);

     return result;
}


int aws_http_headers_get(const struct aws_http_headers *headers, const struct aws_string *name, const struct aws_string **value) {
    assert(headers != NULL);
    assert(name != NULL);
    assert(value != NULL);

    struct aws_hash_element *element = NULL;
    if (aws_hash_table_find(&headers->map, name, &element) == AWS_OP_SUCCESS) {
        *value = (const struct aws_string *)element->value;
        return AWS_OP_SUCCESS;
    }
    return AWS_OP_ERR;
}

int aws_http_headers_get_c_str(const struct aws_http_headers *headers, const char *name, const struct aws_string **value) {
     assert(headers != NULL);
     assert(name != NULL);
     assert(value != NULL);

     /* Create a temporary string for lookup (doesn't need to persist) */
     /* We can't use the stack easily, so allocate/free */
     struct aws_string *name_str = aws_string_new_from_c_str(headers->map.allocator, name);
     if (!name_str) return AWS_OP_ERR;

     int result = aws_http_headers_get(headers, name_str, value);

     aws_string_destroy(name_str); /* Clean up temporary lookup string */
     return result;
}

int aws_http_headers_erase(struct aws_http_headers *headers, const struct aws_string *name) {
     assert(headers != NULL);
     assert(name != NULL);
     /* Remove will call the destroy functions for key/value */
     return aws_hash_table_remove(&headers->map, name, NULL);
}

int aws_http_headers_erase_c_str(struct aws_http_headers *headers, const char *name) {
     assert(headers != NULL);
     assert(name != NULL);

     /* Create a temporary string for lookup */
     struct aws_string *name_str = aws_string_new_from_c_str(headers->map.allocator, name);
     if (!name_str) return AWS_OP_ERR;

     int result = aws_http_headers_erase(headers, name_str);

     aws_string_destroy(name_str); /* Clean up temporary lookup string */
     return result;
}

size_t aws_http_headers_count(const struct aws_http_headers *headers) {
    assert(headers != NULL);
    return aws_hash_table_get_entry_count(&headers->map);
}
