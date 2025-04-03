/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/s3/model/list_buckets_result.h>
#include <aws/common/string.h>
#include <aws/common/error.h>
#include <stdlib.h> /* For NULL */
#include <assert.h> /* For assert */

/* Internal function to clean up a single bucket within the list */
static void s_bucket_clean_up(void *item) {
    struct aws_s3_bucket *bucket = item;
    aws_string_destroy(bucket->name);
    /* aws_date_time doesn't need explicit cleanup */
}

int aws_s3_list_buckets_result_init(
    struct aws_s3_list_buckets_result *result,
    struct aws_allocator *allocator) {

    assert(result != NULL);
    assert(allocator != NULL);

    result->allocator = allocator;
    result->owner.display_name = NULL;
    result->owner.id = NULL;

    /* Initialize the dynamic array for buckets */
    if (aws_array_list_init_dynamic(&result->buckets, allocator, 0, sizeof(struct aws_s3_bucket)) != AWS_OP_SUCCESS) {
        return AWS_OP_ERR; /* Error already raised */
    }

    return AWS_OP_SUCCESS;
}

void aws_s3_list_buckets_result_clean_up(struct aws_s3_list_buckets_result *result) {
    if (!result) {
        return;
    }

    /* Clean up owner strings */
    aws_string_destroy(result->owner.display_name);
    aws_string_destroy(result->owner.id);
    result->owner.display_name = NULL;
    result->owner.id = NULL;

    /* Clean up each bucket in the list */
    size_t num_buckets = aws_array_list_length(&result->buckets);
    for (size_t i = 0; i < num_buckets; ++i) {
        struct aws_s3_bucket *bucket_ptr = NULL;
        /* We get a pointer, no need to check return code if index is valid */
        aws_array_list_get_at_ptr(&result->buckets, (void **)&bucket_ptr, i);
        if (bucket_ptr) {
             s_bucket_clean_up(bucket_ptr);
        }
    }

    /* Clean up the list itself */
    aws_array_list_clean_up(&result->buckets);

    result->allocator = NULL;
}

void aws_s3_list_buckets_result_destroy(struct aws_s3_list_buckets_result *result) {
     if (!result) {
        return;
    }
    struct aws_allocator *allocator = result->allocator;
    aws_s3_list_buckets_result_clean_up(result);
    aws_mem_release(allocator, result);
}
