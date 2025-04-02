# Copyright Christopher Ochsenreither. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0.

# Define the test executable
add_executable(aws-sdk-c-tests
    sigv4_test.c
    # Add other test source files here
)

# Link the test executable against:
# 1. Our SDK core library (aws-sdk-c-core)
# 2. The CRT libraries (common, cal, auth, http, io, compression) - These are fetched via FetchContent in the root CMakeLists.txt
target_link_libraries(aws-sdk-c-tests PRIVATE
    aws-sdk-c-core      # Our library being tested
    aws-c-common        # CRT common target from FetchContent (includes test harness)
    aws-c-cal           # CRT cal target from FetchContent
    aws-c-auth          # CRT auth target from FetchContent
    aws-c-http          # CRT http target from FetchContent
    aws-c-io            # CRT io target from FetchContent
    aws-c-compression   # CRT compression target from FetchContent
    # Add other dependencies if needed by tests
)

# Add the test to CTest
include(CTest)

# Check if aws_add_test function is available from aws-c-common
if(COMMAND aws_add_test)
    aws_add_test(aws-sdk-c-tests)
else()
    # Fallback to standard add_test if aws_add_test is not available
    add_test(NAME aws-sdk-c-tests COMMAND aws-sdk-c-tests)
endif()
