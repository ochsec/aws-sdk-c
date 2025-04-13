cmake_minimum_required(VERSION 3.10)

project(aws-sdk-c C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Disable unnecessary tests and warnings
set(BUILD_TESTING OFF CACHE BOOL "Disable testing" FORCE)
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "Disable deprecation warnings" FORCE)

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)
set(FETCHCONTENT_QUIET OFF)

# Fetch AWS C Libraries with stable release tags as of April 2025
FetchContent_Declare(
    aws-c-common
    GIT_REPOSITORY https://github.com/awslabs/aws-c-common.git
    GIT_TAG v0.9.17 # Updated to a more recent stable release tag
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    aws-c-cal
    GIT_REPOSITORY https://github.com/awslabs/aws-c-cal.git
    GIT_TAG v0.6.5 # Stable release tag as of April 2025
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    aws-c-io
    GIT_REPOSITORY https://github.com/awslabs/aws-c-io.git
    GIT_TAG v0.13.36 # Stable release tag as of April 2025
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    aws-c-auth
    GIT_REPOSITORY https://github.com/awslabs/aws-c-auth.git
    GIT_TAG v0.7.10 # Stable release tag as of April 2025
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

# Make aws-c-common available first, as others depend on its CMake modules
FetchContent_MakeAvailable(aws-c-common)
list(PREPEND CMAKE_MODULE_PATH "${aws-c-common_SOURCE_DIR}/cmake") # Add its cmake dir to the path

# Make the remaining dependencies available
FetchContent_MakeAvailable(
    aws-c-cal
    aws-c-io
    aws-c-auth
)

# Define the core SDK library
add_library(aws-sdk-c-core
    src/core_placeholder.c
    src/auth/sigv4.c
)

# Link against the AWS C dependencies
target_link_libraries(aws-sdk-c-core PUBLIC
    aws-c-common
    aws-c-cal
    aws-c-auth
    aws-c-io
)

# Set include directories
target_include_directories(aws-sdk-c-core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ENABLE_TESTING "Enable testing" ON)

# Add tests if testing is enabled
if(ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# Add service subdirectories
add_subdirectory(src/s3)
add_subdirectory(examples)
