# Copyright Christopher Ochsenreither. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0.
# Find required AWS C library packages
find_package(aws-c-common REQUIRED)
find_package(aws-c-cal REQUIRED)
find_package(aws-c-auth REQUIRED)
find_package(aws-c-http REQUIRED)
find_package(aws-c-io REQUIRED)
find_package(aws-c-compression REQUIRED)


# Define the test executable
add_executable(aws-sdk-c-tests
    sigv4_test.c
    # Add other test source files here
)

# Add compiler definition to allow using the AWS test harness
target_compile_definitions(aws-sdk-c-tests PRIVATE AWS_UNSTABLE_TESTING_API)

# Link the test executable against:
# 1. Our SDK core library (aws-sdk-c-core)
# 2. The CRT libraries (common, cal, auth, http, io, compression) - These are fetched via FetchContent in the root CMakeLists.txt
target_link_libraries(aws-sdk-c-tests PRIVATE
    aws-sdk-c-core      # Our library being tested
    AWS::aws-c-common        # Use namespaced target
    AWS::aws-c-cal           # Use namespaced target
    AWS::aws-c-auth          # Use namespaced target
    AWS::aws-c-http          # Use namespaced target
    AWS::aws-c-io            # Use namespaced target
    AWS::aws-c-compression   # Use namespaced target
    # Add other dependencies if needed by tests
)

# Explicitly add include directories from dependencies
target_include_directories(aws-sdk-c-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include # Add project's own include dir first
    /usr/local/include         # Directly add install prefix include dir
    # $<TARGET_PROPERTY:AWS::aws-c-common,INTERFACE_INCLUDE_DIRECTORIES> # Comment out for now
    # $<TARGET_PROPERTY:AWS::aws-c-cal,INTERFACE_INCLUDE_DIRECTORIES>
    # $<TARGET_PROPERTY:AWS::aws-c-auth,INTERFACE_INCLUDE_DIRECTORIES>
    # $<TARGET_PROPERTY:AWS::aws-c-http,INTERFACE_INCLUDE_DIRECTORIES>
    # $<TARGET_PROPERTY:AWS::aws-c-io,INTERFACE_INCLUDE_DIRECTORIES>
    # $<TARGET_PROPERTY:AWS::aws-c-compression,INTERFACE_INCLUDE_DIRECTORIES>
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
