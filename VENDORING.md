# Dependency Vendoring

## Overview
This project vendors the following AWS SDK C dependencies to ensure consistent builds and reduce external dependencies:

- aws-c-http
- aws-c-io
- aws-c-cal
- aws-c-compression
- aws-c-sdkutils
- aws-c-auth
- aws-c-common

## Integration Process
1. Dependencies were copied from their original repositories into the `vendor/` directory
2. Git metadata was removed to prevent version control conflicts
3. Build scripts were updated to reference vendored dependencies

## Dependency Versions
Vendored on: 4/12/2025

### Commit Hashes
- aws-c-http: [Insert actual commit hash]
- aws-c-io: [Insert actual commit hash]
- aws-c-cal: [Insert actual commit hash]
- aws-c-compression: [Insert actual commit hash]
- aws-c-sdkutils: [Insert actual commit hash]
- aws-c-auth: [Insert actual commit hash]
- aws-c-common: [Insert actual commit hash]

## Maintenance
- Update these dependencies periodically to incorporate bug fixes and improvements
- When updating, repeat the vendoring process and update this documentation
- Verify build compatibility after each update

## Rationale
Vendoring these dependencies:
- Ensures consistent builds across different environments
- Reduces external dependency management complexity
- Provides a stable, reproducible build process
