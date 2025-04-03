# Plan: Managing AWS C Library Dependencies with CMake FetchContent

This document outlines the plan to switch from manually copying AWS C library source files to using CMake's `FetchContent` module for dependency management in the `aws-sdk-c` project.

**Goal:** Improve maintainability, simplify updates, and ensure reproducible builds by managing external AWS C library dependencies (`aws-c-common`, `aws-c-cal`, `aws-c-io`, `aws-c-auth`, `aws-sdk-utils`) directly within the CMake build system.

**Chosen Method:** CMake `FetchContent` module.

**Rationale:**
*   Integrates dependency management directly into the build process.
*   Simplifies the cloning and building experience for users compared to Git submodules.
*   Allows pinning dependencies to specific versions (tags/commits) for stability.
*   Suitable as we are not modifying the upstream AWS library code.

**Steps:**

1.  **Clean Up Existing Files:**
    *   Carefully remove the directories corresponding to the AWS C libraries from `include/aws/`. This includes:
        *   `include/aws/common/`
        *   `include/aws/cal/`
        *   `include/aws/io/`
        *   `include/aws/auth/`
        *   `include/aws/sdkutils/`
    *   Carefully remove the corresponding source directories and files from `src/`. This includes:
        *   `src/common/`
        *   `src/cal/`
        *   `src/io/`
        *   `src/auth/`
        *   `src/sdkutils/`
    *   Ensure these removals are committed to version control before proceeding.

2.  **Modify Root `CMakeLists.txt`:**
    *   Add `include(FetchContent)` near the top if not already present.
    *   Declare each AWS C library dependency using `FetchContent_Declare`. Use specific, stable release tags for `GIT_TAG`.
        ```cmake
        include(FetchContent)

        # --- AWS C Library Dependencies ---
        FetchContent_Declare(
            aws-c-common
            GIT_REPOSITORY https://github.com/awslabs/aws-c-common.git
            GIT_TAG v0.9.9 # Replace with the desired stable release tag
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

        # Make the dependencies available (downloads/configures them)
        FetchContent_MakeAvailable(
            aws-c-common
            aws-c-cal
            aws-c-io
            aws-c-auth
            aws-sdkutils
        )
        ```
    *   **Note:** Verify the exact repository URLs and choose appropriate, compatible release tags for each library. The tags listed above are examples and might need adjustment based on current releases and inter-library compatibility.

3.  **Modify `src/CMakeLists.txt` (and others as needed):**
    *   Identify your library or executable targets defined in `src/CMakeLists.txt` (and potentially `examples/CMakeLists.txt`, `tests/CMakeLists.txt`).
    *   Update the `target_link_libraries` commands for these targets. Remove any links to the old, manually managed source files/targets and add links to the CMake targets exported by the fetched AWS libraries. Common target names are `aws-c-common`, `aws-c-cal`, `aws-c-io`, `aws-c-auth`, and `aws-sdkutils-lib` (or similar - check the library's CMake files if unsure).
        ```cmake
        # Example: Linking a library defined in src/CMakeLists.txt
        # add_library(my_aws_sdk_lib ...) # Your library definition

        target_link_libraries(my_aws_sdk_lib PRIVATE
            aws-c-common
            aws-c-cal
            aws-c-io
            aws-c-auth
            aws-sdkutils-lib # Verify this target name from aws-c-sdkutils CMakeLists.txt
        )

        # Example: Linking an executable in examples/CMakeLists.txt
        # add_executable(my_example examples/my_example.c)

        target_link_libraries(my_example PRIVATE
            my_aws_sdk_lib # Link your own SDK library if needed
            aws-c-common
            aws-c-cal
            aws-c-io
            aws-c-auth
            aws-sdkutils-lib
        )
        ```
    *   Update `target_include_directories` if necessary, although `FetchContent_MakeAvailable` often handles this implicitly by making the dependencies' include directories available globally or via the linked targets.

4.  **Build and Test:**
    *   Remove your existing build directory (e.g., `rm -rf build`).
    *   Re-run CMake configuration (`cmake -S . -B build`). Observe the output to ensure `FetchContent` downloads the dependencies.
    *   Build the project (`cmake --build build`).
    *   Run any tests to confirm functionality.

**Next Steps:** Implement this plan using Code mode.