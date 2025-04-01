#ifndef AWS_S3_MODEL_LIST_BUCKETS_RESULT_H
#define AWS_S3_MODEL_LIST_BUCKETS_RESULT_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/exports.h>
#include <aws/common/allocator.h>
#include <aws/common/date_time.h> /* Need to define this for timestamps */
#include <aws/common/string.h> /* Need to define this for string types */
#include <aws/common/array_list.h> /* Need to define this for lists */

AWS_EXTERN_C_BEGIN

/**
 * @brief Represents the owner of the buckets listed.
 */
struct aws_s3_owner {
    struct aws_string *display_name; /* Use aws_string for managed strings */
    struct aws_string *id;
};

/**
 * @brief Represents a single S3 bucket.
 */
struct aws_s3_bucket {
    struct aws_string *name;
    struct aws_date_time creation_date; /* Use aws_date_time for timestamps */
};

/**
 * @brief Represents the result of a ListBuckets operation.
 */
struct aws_s3_list_buckets_result {
    struct aws_allocator *allocator; /* Allocator used for this result */
    struct aws_array_list buckets; /* List of aws_s3_bucket structures */
    struct aws_s3_owner owner;
};

/**
 * @brief Initializes a list buckets result structure.
 *
 * @param result The result structure to initialize.
 * @param allocator The allocator to use for internal lists and strings.
 * @return AWS_OP_SUCCESS on success, or an error code on failure.
 */
AWS_COMMON_API
int aws_s3_list_buckets_result_init(
    struct aws_s3_list_buckets_result *result,
    struct aws_allocator *allocator);

/**
 * @brief Cleans up resources associated with a list buckets result structure.
 *        Does NOT free the result structure itself.
 *
 * @param result The result structure to clean up.
 */
AWS_COMMON_API
void aws_s3_list_buckets_result_clean_up(struct aws_s3_list_buckets_result *result);

/**
 * @brief Destroys a list buckets result structure, including freeing the structure itself.
 *        Must have been allocated using the allocator passed to aws_s3_list_buckets.
 *
 * @param result The result structure to destroy.
 */
AWS_COMMON_API
void aws_s3_list_buckets_result_destroy(struct aws_s3_list_buckets_result *result);


AWS_EXTERN_C_END

#endif /* AWS_S3_MODEL_LIST_BUCKETS_RESULT_H */
