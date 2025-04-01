#ifndef AWS_S3_CLIENT_H
#define AWS_S3_CLIENT_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/exports.h> // Reusing common exports for now
#include <aws/s3/model/list_buckets_result.h> // Define this next

AWS_EXTERN_C_BEGIN

/* Opaque handle for the S3 client */
struct aws_s3_client;

/* Structure for S3 client configuration options */
struct aws_s3_client_config {
    /* TODO: Add configuration options like region, credentials provider, etc. */
    const char *region;
    /* Add other necessary config fields */
};

/**
 * @brief Creates a new S3 client instance.
 *
 * @param config Client configuration options.
 * @param allocator Memory allocator to use (can be NULL for default).
 * @return A pointer to the newly created S3 client, or NULL on failure.
 *         The caller is responsible for destroying the client using aws_s3_client_destroy.
 */
AWS_COMMON_API /* Reuse COMMON for now, maybe define S3_API later */
struct aws_s3_client *aws_s3_client_new(
    const struct aws_s3_client_config *config,
    struct aws_allocator *allocator); /* aws_allocator to be defined in common */

/**
 * @brief Destroys an S3 client instance, freeing associated resources.
 *
 * @param client The S3 client instance to destroy.
 */
AWS_COMMON_API
void aws_s3_client_destroy(struct aws_s3_client *client);

/**
 * @brief Lists all buckets owned by the sender.
 *
 * @param client The S3 client instance.
 * @param allocator Memory allocator for the result structure.
 * @return A pointer to an aws_s3_list_buckets_result structure containing the
 *         list of buckets, or NULL on failure. The caller is responsible for
 *         destroying the result using aws_s3_list_buckets_result_destroy.
 *         Check aws_last_error() for error details on failure.
 */
AWS_COMMON_API
struct aws_s3_list_buckets_result *aws_s3_list_buckets(
    struct aws_s3_client *client,
    struct aws_allocator *allocator);

AWS_EXTERN_C_END

#endif /* AWS_S3_CLIENT_H */
