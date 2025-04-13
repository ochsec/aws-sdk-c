#!/bin/bash
# Comprehensive Dependency Extraction and Vendor Management Script

# Set strict mode for better error handling
set -euo pipefail

# Define dependencies
DEPENDENCIES=(
    "aws-c-common"
    "aws-c-auth"
    "aws-c-http"
    "aws-c-io"
    "aws-c-cal"
    "aws-c-compression"
    "aws-c-sdkutils"
)

# Logging and error handling functions
log_info() {
    echo "[INFO] $*"
}

log_warning() {
    echo "[WARNING] $*" >&2
}

log_error() {
    echo "[ERROR] $*" >&2
    exit 1
}

# Create vendor directory
VENDOR_DIR="vendor"
mkdir -p "$VENDOR_DIR"

# Current script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
WORKSPACE_ROOT="$(dirname "$PROJECT_ROOT")"

# Validate workspace
[ -d "$WORKSPACE_ROOT" ] || log_error "Workspace root does not exist: $WORKSPACE_ROOT"

# Debug: Print current environment
log_info "Current Working Directory: $(pwd)"
log_info "Script Directory: $SCRIPT_DIR"
log_info "Project Root: $PROJECT_ROOT"
log_info "Workspace Root: $WORKSPACE_ROOT"

# Extract and prepare dependencies
EXTRACTED_DEPS=()
MISSING_DEPS=()

for dep in "${DEPENDENCIES[@]}"; do
    log_info "Processing $dep..."
    
    # Expanded search locations
    POSSIBLE_LOCATIONS=(
        "$WORKSPACE_ROOT/$dep"
        "$PROJECT_ROOT/$dep"
        "$WORKSPACE_ROOT/aws-sdk-c/$dep"
        "$WORKSPACE_ROOT/aws-sdk-cpp/crt/$dep"
        "$HOME/github/$dep"
        "/usr/local/include/$dep"
        "/usr/include/$dep"
    )
    
    FOUND_LOCATION=""
    for loc in "${POSSIBLE_LOCATIONS[@]}"; do
        if [ -d "$loc" ]; then
            FOUND_LOCATION="$loc"
            break
        fi
    done
    
    if [ -z "$FOUND_LOCATION" ]; then
        log_warning "Dependency $dep not found in any of these locations:"
        printf '%s\n' "${POSSIBLE_LOCATIONS[@]}" >&2
        
        # Attempt to clone the dependency
        GITHUB_URL="https://github.com/awslabs/$dep.git"
        CLONE_DIR="$WORKSPACE_ROOT/$dep"
        
        log_info "Attempting to clone $dep from $GITHUB_URL"
        if git clone "$GITHUB_URL" "$CLONE_DIR"; then
            FOUND_LOCATION="$CLONE_DIR"
            log_info "Successfully cloned $dep"
        else
            log_error "Failed to clone $dep from $GITHUB_URL"
            MISSING_DEPS+=("$dep")
            continue
        fi
    fi
    
    log_info "Found $dep at $FOUND_LOCATION"
    
    # Create dependency subdirectory
    mkdir -p "$VENDOR_DIR/$dep"
    
    # Copy include files
    if [ -d "$FOUND_LOCATION/include" ]; then
        cp -R "$FOUND_LOCATION/include" "$VENDOR_DIR/$dep/"
        log_info "Copied include files for $dep"
    else
        log_warning "No include directory found for $dep in $FOUND_LOCATION"
    fi
    
    # Copy source files
    if [ -d "$FOUND_LOCATION/source" ]; then
        cp -R "$FOUND_LOCATION/source" "$VENDOR_DIR/$dep/"
        log_info "Copied source files for $dep"
    else
        log_warning "No source directory found for $dep in $FOUND_LOCATION"
    fi
    
    # Generate dependency metadata
    if [ -d "$FOUND_LOCATION/.git" ]; then
        git -C "$FOUND_LOCATION" rev-parse HEAD > "$VENDOR_DIR/$dep/COMMIT_HASH"
        log_info "Captured commit hash for $dep"
    else
        echo "UNKNOWN" > "$VENDOR_DIR/$dep/COMMIT_HASH"
        log_warning "No git repository found for $dep"
    fi
    
    # Capture additional metadata
    {
        echo "Dependency: $dep"
        echo "Location: $FOUND_LOCATION"
        echo "Commit Hash: $(cat "$VENDOR_DIR/$dep/COMMIT_HASH")"
        echo "Extracted At: $(date -u +"%Y-%m-%d %H:%M:%S UTC")"
    } > "$VENDOR_DIR/$dep/DEPENDENCY_INFO.txt"
    
    EXTRACTED_DEPS+=("$dep")
done

# Create comprehensive dependency manifest
{
    echo "# Vendored Dependencies"
    echo ""
    echo "## Dependency Overview"
    echo ""
    echo "### Extracted Dependencies"
    echo ""
    echo "| Dependency | Version | Extracted At | Source |"
    echo "|-----------|---------|--------------|--------|"
    
    for dep in "${EXTRACTED_DEPS[@]}"; do
        if [ -f "$VENDOR_DIR/$dep/COMMIT_HASH" ]; then
            COMMIT_HASH=$(cat "$VENDOR_DIR/$dep/COMMIT_HASH")
            SOURCE=$(grep "Location:" "$VENDOR_DIR/$dep/DEPENDENCY_INFO.txt" | cut -d' ' -f2-)
            echo "| $dep | $COMMIT_HASH | $(date -u +"%Y-%m-%d %H:%M:%S UTC") | $SOURCE |"
        fi
    done
    
    echo ""
    echo "### Missing Dependencies"
    echo ""
    if [ ${#MISSING_DEPS[@]} -eq 0 ]; then
        echo "All dependencies were successfully extracted."
    else
        echo "| Dependency | Status | Recommended Action |"
        echo "|-----------|--------|-------------------|"
        for dep in "${MISSING_DEPS[@]}"; do
            echo "| $dep | Not Found | Manual clone required from https://github.com/awslabs/$dep |"
        done
    fi
    
    echo ""
    echo "## Dependency Management Notes"
    echo ""
    echo "- This manifest is auto-generated during dependency extraction"
    echo "- Missing dependencies require manual intervention"
    echo "- Always verify the integrity and security of dependencies before use"
} > "$VENDOR_DIR/DEPENDENCIES.md"

# Validate vendor directory
if [ ${#EXTRACTED_DEPS[@]} -eq 0 ]; then
    log_error "No dependencies were successfully extracted!"
    exit 1
fi

log_info "Dependency extraction completed."
log_info "Extracted ${#EXTRACTED_DEPS[@]} dependencies"
if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
    log_warning "Missing ${#MISSING_DEPS[@]} dependencies"
fi

log_info "Vendor directory contents:"
ls -R "$VENDOR_DIR"