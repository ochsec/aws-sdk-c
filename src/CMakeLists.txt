# Copyright Christopher Ochsenreither. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0.

# This file can be used to define core SDK libraries or add further subdirectories.
# Service-specific libraries (like S3) are defined in their respective subdirectories.
# Find required AWS C library packages
find_package(aws-c-common REQUIRED)
find_package(aws-c-cal REQUIRED)
find_package(aws-c-auth REQUIRED)
find_package(aws-c-io REQUIRED)
find_package(aws-c-http REQUIRED)
find_package(aws-c-compression REQUIRED)
find_package(aws-c-sdkutils REQUIRED)


# Define the core SDK library
add_library(aws-sdk-c-core
    # Add core source files here
    # For now, we'll create a placeholder file
    ${CMAKE_CURRENT_SOURCE_DIR}/core_placeholder.c
    ${CMAKE_CURRENT_SOURCE_DIR}/auth/sigv4.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/io/tee_input_stream.c # Remove incompatible source
)

# Link against the AWS C dependencies
target_link_libraries(aws-sdk-c-core PUBLIC
    AWS::aws-c-common
    AWS::aws-c-cal
    AWS::aws-c-auth
    AWS::aws-c-io
    AWS::aws-c-http
    AWS::aws-c-compression
    AWS::aws-c-sdkutils
)

# Set include directories
target_include_directories(aws-sdk-c-core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Installation rules for actual SDK libraries and headers would go here or in subdirectories.
# The headers for aws-c-* dependencies are typically installed by those libraries themselves.
# TODO: Add installation rules for service-specific headers later

# Export rules for actual SDK targets would go here.

# Generate and install the aws-sdk-c-config.cmake and aws-sdk-c-config-version.cmake files
# These files allow other CMake projects to find and use this SDK.
# We need a template file for aws-sdk-c-config.cmake.in first.
# Let's assume it exists in the cmake/ directory for now.
# TODO: Create aws-sdk-c/cmake/aws-sdk-c-config.cmake.in

# include(CMakePackageConfigHelpers)
# write_basic_package_version_file(
#     "${CMAKE_CURRENT_BINARY_DIR}/aws-sdk-c-config-version.cmake"
#     VERSION ${PROJECT_VERSION} # PROJECT_VERSION needs to be defined in the root CMakeLists.txt
#     COMPATIBILITY AnyNewerVersion
# )

# configure_package_config_file(
#     "${CMAKE_SOURCE_DIR}/cmake/aws-sdk-c-config.cmake.in" # Path relative to root source dir
#     "${CMAKE_CURRENT_BINARY_DIR}/aws-sdk-c-config.cmake"
#     INSTALL_DESTINATION lib/cmake/aws-sdk-c
#     # PATH_VARS needs to be set based on install locations
# )

# install(FILES
#     "${CMAKE_CURRENT_BINARY_DIR}/aws-sdk-c-config.cmake"
#     "${CMAKE_CURRENT_BINARY_DIR}/aws-sdk-c-config-version.cmake"
#     DESTINATION lib/cmake/aws-sdk-c
# )
