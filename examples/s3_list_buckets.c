/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/s3/s3_client.h>
#include <aws/common/error.h>
#include <aws/common/logging.h> /* Need to create this */
#include <stdio.h> /* For printf */

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    /* TODO: Initialize logging */
    /* aws_logging_init(...); */

    printf("Attempting to list S3 buckets (Proof-of-Concept)...\n");

    /* Use default allocator */
    struct aws_allocator *allocator = aws_default_allocator();

    /* Configure the S3 client (minimal config for now) */
    struct aws_s3_client_config config = {
        .region = "us-east-1", /* Example region */
    };

    /* Create the S3 client */
    struct aws_s3_client *client = aws_s3_client_new(&config, allocator);
    if (!client) {
        fprintf(stderr, "Failed to create S3 client: %s (%s)\n",
                aws_error_str(aws_last_error()),
                aws_error_debug_str());
        return 1;
    }
    printf("S3 client created successfully.\n");

    /* Call the ListBuckets operation */
    printf("Calling aws_s3_list_buckets...\n");
    struct aws_s3_list_buckets_result *result = aws_s3_list_buckets(client, allocator);

    if (!result) {
        fprintf(stderr, "aws_s3_list_buckets failed: %s (%s)\n",
                aws_error_str(aws_last_error()),
                aws_error_debug_str());
        /* Expected failure for now as it's not implemented */
        printf("ListBuckets call failed as expected (not implemented yet).\n");
    } else {
        /* This block shouldn't be reached with the current placeholder implementation */
        printf("ListBuckets succeeded (unexpectedly!).\n");
        size_t num_buckets = aws_array_list_length(&result->buckets);
        printf("Found %zu buckets:\n", num_buckets);

        for (size_t i = 0; i < num_buckets; ++i) {
            struct aws_s3_bucket *bucket_ptr = NULL;
            aws_array_list_get_at_ptr(&result->buckets, (void **)&bucket_ptr, i);
            if (bucket_ptr && bucket_ptr->name) {
                 printf("- %s (Created: %lld)\n",
                        aws_string_c_str(bucket_ptr->name),
                        (long long)aws_date_time_get_epoch_secs(&bucket_ptr->creation_date));
            }
        }

        if (result->owner.display_name) {
             printf("Owner: %s (ID: %s)\n",
                    aws_string_c_str(result->owner.display_name),
                    result->owner.id ? aws_string_c_str(result->owner.id) : "N/A");
        }

        /* Clean up the result structure */
        aws_s3_list_buckets_result_destroy(result);
        printf("ListBuckets result cleaned up.\n");
    }

    /* Destroy the S3 client */
    aws_s3_client_destroy(client);
    printf("S3 client destroyed.\n");

    /* TODO: Clean up logging */
    /* aws_logging_clean_up(); */

    printf("S3 List Buckets PoC finished.\n");
    return 0;
}
