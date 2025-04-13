#!/bin/bash

# Cosmopolitan Build Script for AWS SDK C
# Version: 1.0.0
# Date: 4/12/2025

# Strict error handling
set -euo pipefail

# Logging function
log() {
    echo "[$(date +'%Y-%m-%d %H:%M:%S')] $*" >&2
}

# Error handling function
handle_error() {
    log "ERROR: $*"
    exit 1
}

# Dependency directories
DEPS=(
    "vendor/aws-c-common"
    "vendor/aws-c-cal"
    "vendor/aws-c-io"
    "vendor/aws-c-auth"
    "vendor/aws-c-compression"
    "vendor/aws-c-http"
    "vendor/aws-c-sdkutils"
)

# Compilation flags
CFLAGS=(
    "-std=c11"
    "-Wall"
    "-Wextra"
    "-Werror"
    "-O2"
    "-g"
)

# Include paths
INCLUDE_PATHS=()
for dep in "${DEPS[@]}"; do
    INCLUDE_PATHS+=("-I" "$dep/include")
done

# Source files to compile
SOURCE_FILES=(
    # Core SDK sources
    "src/core_placeholder.c"
    "src/auth/sigv4.c"
)

# Collect source files from dependencies
for dep in "${DEPS[@]}"; do
    # Add source files from each dependency
    SOURCE_FILES+=($(find "$dep/source" -name "*.c"))
done

# Compilation function
compile_sdk() {
    log "Starting AWS SDK C compilation..."
    
    # Cosmopolitan compilation
    cosmocc "${CFLAGS[@]}" \
            "${INCLUDE_PATHS[@]}" \
            "${SOURCE_FILES[@]}" \
            -o aws-sdk-c.a \
            || handle_error "Compilation failed"
    
    log "Compilation completed successfully."
}

# Main execution
main() {
    # Validate dependencies
    for dep in "${DEPS[@]}"; do
        if [[ ! -d "$dep" ]]; then
            handle_error "Dependency $dep not found"
        fi
    done

    # Compile
    compile_sdk
}

# Run main function with error trapping
if ! main; then
    log "Build process encountered an error."
    exit 1
fi

log "AWS SDK C build process completed."