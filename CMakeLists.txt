cmake_minimum_required(VERSION 3.10)

project(aws-sdk-c C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)


# --- AWS C Library Dependencies ---
# Removed incorrect aws-c-build-tools declaration

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
    GIT_TAG v0.6.5 # Replace with the desired stable release tag
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    aws-c-io
    GIT_REPOSITORY https://github.com/awslabs/aws-c-io.git
    GIT_TAG v0.13.36 # Replace with the desired stable release tag
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    aws-c-auth
    GIT_REPOSITORY https://github.com/awslabs/aws-c-auth.git
    GIT_TAG v0.7.10 # Replace with the desired stable release tag
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    aws-sdkutils # Note: Repository name might differ slightly if it's aws-c-sdkutils
    GIT_REPOSITORY https://github.com/awslabs/aws-c-sdkutils.git
    GIT_TAG v0.2.1 # Replace with the desired stable release tag
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
# --- End AWS C Library Dependencies ---

FetchContent_Declare(
    aws-c-compression
    GIT_REPOSITORY https://github.com/awslabs/aws-c-compression.git
    GIT_TAG v0.3.1  # Latest stable version as of April 2025
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    aws-c-http
    GIT_REPOSITORY https://github.com/awslabs/aws-c-http.git
    GIT_TAG v0.9.6  # Latest stable version as of April 2025
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

# Make aws-c-common available first, as others depend on its CMake modules
FetchContent_MakeAvailable(aws-c-common)
list(PREPEND CMAKE_MODULE_PATH "${aws-c-common_SOURCE_DIR}/cmake") # Add its cmake dir to the path

# Make the remaining dependencies available
# They can now find the modules from aws-c-common
FetchContent_MakeAvailable(
    aws-c-cal
    aws-c-io
    aws-c-auth
    aws-sdkutils
    aws-c-compression
    aws-c-http
)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ENABLE_TESTING "Enable testing" ON)

# Include directories
include_directories(include)

# Add subdirectories
add_subdirectory(src)
# Add service subdirectories (add more as they are created)
add_subdirectory(src/s3)

if(ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests) # Uncommented to enable tests
endif()

# Add examples subdirectory
add_subdirectory(examples)

# Installation
# TODO: Add installation rules

# Packaging
# TODO: Add packaging rules (CPack)
