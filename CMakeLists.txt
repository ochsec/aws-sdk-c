cmake_minimum_required(VERSION 3.10)

project(aws-sdk-c C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

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
    # add_subdirectory(tests) # TODO: Add tests later
endif()

# Add examples subdirectory
add_subdirectory(examples)

# Installation
# TODO: Add installation rules

# Packaging
# TODO: Add packaging rules (CPack)
