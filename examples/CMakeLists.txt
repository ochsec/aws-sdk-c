# Copyright Christopher Ochsenreither. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0.

# Define the example executable
add_executable(s3_list_buckets_example s3_list_buckets.c)

# Link the example against the S3 library
# Linking against aws-c-s3 will automatically bring in aws-c-common
target_link_libraries(s3_list_buckets_example PRIVATE aws-c-s3)

# Optional: Add installation rule for the example
# install(TARGETS s3_list_buckets_example DESTINATION bin)
