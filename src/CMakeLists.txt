# Copyright Christopher Ochsenreither. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0.

# This file can be used to define core SDK libraries or add further subdirectories.
# Service-specific libraries (like S3) are defined in their respective subdirectories.

# Core SDK library definitions would go here if needed.
# Linking against fetched dependencies (aws-c-common, aws-c-cal, etc.)
# should happen within the specific library targets (e.g., in src/s3/CMakeLists.txt).

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
