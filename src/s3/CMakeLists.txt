# Copyright Christopher Ochsenreither. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0.
# Find required AWS C library packages
find_package(aws-c-common REQUIRED)
find_package(aws-c-cal REQUIRED)
find_package(aws-c-io REQUIRED)
find_package(aws-c-auth REQUIRED)
find_package(aws-c-sdkutils REQUIRED)


# Define the S3 library target
add_library(aws-c-s3
    s3_client.c
    model/list_buckets_result.c
    # Add other S3 source files here
)

# Link against the common library
target_link_libraries(aws-c-s3 PUBLIC
    AWS::aws-c-common
    AWS::aws-c-cal
    AWS::aws-c-io
    AWS::aws-c-auth
    AWS::aws-c-sdkutils
)

# Set target properties (e.g., include directories)
# The common library already sets up the include path via PUBLIC
# target_include_directories(aws-c-s3 PUBLIC
#     $<INSTALL_INTERFACE:include>
#     $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
# )

# Installation rules for the library
install(TARGETS aws-c-s3
    EXPORT aws-sdk-c-targets # Add to the main export set
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Installation rules for public S3 headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/aws/s3
    DESTINATION include/aws
    FILES_MATCHING PATTERN "*.h"
)
