# AWS SDK for C

This project aims to provide a comprehensive AWS SDK for the C programming language, ported from the official AWS SDK for C++.

## Project Goals

- Provide a modern C (C11) interface for Amazon Web Services (AWS).
- Offer low-level and high-level APIs for interacting with AWS services.
- Minimize dependencies and ensure platform portability (Linux, macOS, Windows).
- Leverage the existing AWS Common Runtime (CRT) libraries for core functionality.

## Current Status

This project is currently in the initial development phase.

## Building the SDK

### Prerequisites

- CMake (version 3.10 or later)
- C compiler supporting C11 standard (GCC, Clang, MSVC)
- Internet connection (for downloading dependencies)

### Build Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/your-repo/aws-sdk-c.git
   cd aws-sdk-c
   ```

2. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```

3. Configure the project with CMake:
   ```bash
   cmake ..
   ```
   *Optional CMake flags:*
     * `-DBUILD_SHARED_LIBS=ON/OFF`: Build shared or static libraries (default: ON)
     * `-DENABLE_TESTING=ON/OFF`: Enable or disable building tests (default: ON)
     * `-DCMAKE_INSTALL_PREFIX=<path>`: Specify installation directory

4. Build the SDK:
   ```bash
   cmake --build .
   ```

5. (Optional) Run tests:
   ```bash
   ctest
   ```

6. (Optional) Install the SDK:
   ```bash
   cmake --install .
   ```

## Usage

*TODO: Add usage examples.*

## Contributing

*TODO: Add contribution guidelines.*

## License

*TODO: Add license information.*
