# Copyright Christopher Ochsenreither. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0.

# Define the core common library target
add_library(aws-c-common
    common/allocator.c
    common/memory.c
    common/error.c
    common/string.c
    common/date_time.c
    common/array_list.c
    common/logging.c
    # Add other core source files here
)

# Link dependencies (if any)
# target_link_libraries(aws-c-common PRIVATE ...)

# Set target properties (e.g., include directories)
target_include_directories(aws-c-common PUBLIC
    $<INSTALL_INTERFACE:include> # For installed headers
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include> # For build tree headers
)

# Installation rules for the library
install(TARGETS aws-c-common
    EXPORT aws-sdk-c-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include # This makes target_include_directories work correctly for install
)

# Installation rules for public headers
# Install common headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../include/aws/common
    DESTINATION include/aws
    FILES_MATCHING PATTERN "*.h"
)
# TODO: Add installation rules for service-specific headers later

# Export the target for use by other CMake projects
install(EXPORT aws-sdk-c-targets
    FILE aws-sdk-c-targets.cmake
    NAMESPACE AWS::
    DESTINATION lib/cmake/aws-sdk-c
)

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
