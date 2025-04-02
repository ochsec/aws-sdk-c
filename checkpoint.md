# AWS SDK for C - Project Checkpoint (2025-04-02 - SigV4 Implementation Completed)

## Goal

Implement the core AWS Signature Version 4 (SigV4) request signing logic within the `aws_sigv4_sign_request` function in `src/auth/sigv4.c`, utilizing helper functions from the AWS Common Runtime (CRT) libraries (`aws-c-common`, `aws-c-cal`, `aws-c-auth`).

## Summary of Work Completed (SigV4 Implementation)

Based on the initial structure and TODOs from the previous checkpoint, the following components of the `aws_sigv4_sign_request` function have been implemented:

1.  **Canonical Request Construction (Step 1)**:
    *   **HTTP Method (1.1)**: Appended correctly.
    *   **Canonical URI Path (1.2)**: Implemented path normalization logic (`s_aws_sigv4_normalize_uri_path`) handling segments, `.`, `..`, and URI encoding using `aws-c-common` helpers.
    *   **Canonical Query String (1.3)**: Implemented logic to extract the query string (using manual search as a fallback), parse parameters (`aws_uri_query_params`), sort parameters (`qsort` with `s_compare_uri_params`), URI-encode keys/values (`aws_uri_encode_query_param`), and append the canonical string. *Note: Query string extraction relies on finding '?' in the path and needs further investigation for robustness.*
    *   **Canonical Headers (1.4)**: Implemented logic to process request headers: convert names to lowercase (`aws_string_tolower`), trim whitespace (`aws_byte_cursor_trim_pred`), fold multi-line header whitespace (`s_is_whitespace` helper), sort headers alphabetically (`aws_array_list_sort` with `s_compare_canonical_headers`), and append the `name:value\n` pairs.
    *   **Signed Headers (1.5)**: Implemented logic to generate the semicolon-separated list of lowercase header names based on the sorted canonical headers. This list is appended to the canonical request and stored for the `Authorization` header.
    *   **Hashed Payload (1.6)**: Implemented logic to check for the `x-amz-content-sha256` header. If not present, it calculates the SHA256 hash of the body using a tee stream approach that preserves the original stream for transmission.

2.  **String to Sign Construction (Step 2)**:
    *   **Algorithm (2.1)**: Hardcoded `AWS4-HMAC-SHA256`.
    *   **Timestamp (2.2)**: Implemented using a helper function (`s_aws_date_time_to_iso8601_basic_str`) based on `aws-c-common` date/time functions.
    *   **Scope (2.3)**: Implemented using a helper function (`s_aws_date_time_to_date_stamp_str`) and the provided region/service cursors.
    *   **Canonical Request Hash (2.4)**: Calculated using `aws_sha256` on the generated canonical request string (Step 1) and hex-encoded.

3.  **Signing Key Calculation (Step 3)**: Implemented the HMAC chain (`kSecret` -> `kDate` -> `kRegion` -> `kService` -> `kSigning`) using `aws_hmac_sha256`. Assumes credential accessor functions (`aws_credentials_get_secret_access_key`, etc.) are correct.

4.  **Signature Calculation (Step 4)**: Implemented using `aws_hmac_sha256` with the signing key (Step 3) and the string to sign (Step 2), followed by hex-encoding.

5.  **Adding Headers to Request (Step 5)**:
    *   Constructed the `Authorization` header using calculated components (Credential Scope, Signed Headers, Signature).
    *   Added `Authorization`, `X-Amz-Date` (using formatted timestamp), and `X-Amz-Security-Token` (if present in credentials) headers to the `aws_http_message`.

6.  **Final Return**: Updated the function to return `AWS_OP_SUCCESS` on the successful path.

## New Implementations

### Tee Input Stream for Streaming Body Support

The critical TODO regarding stream consumption has been addressed by implementing a tee stream adapter:

1. **Tee Stream Implementation**:
   * Created `aws_tee_input_stream` in `src/io/tee_input_stream.c` and corresponding header in `include/aws/io/tee_input_stream.h`.
   * The tee stream wraps an original input stream and buffers data as it's read.
   * Implemented branch streams that can read from the same source independently.
   * Added proper error handling and resource cleanup.

2. **SigV4 Integration**:
   * Modified the SigV4 implementation to use tee streams for handling request bodies.
   * Added detection of existing tee streams to avoid unnecessary wrapping.
   * When a body stream is present, the implementation:
     * Checks if it's already a tee stream
     * If not, creates a new tee stream wrapping the original body stream
     * Creates a branch stream for hashing purposes
     * Hashes the branch stream, leaving the original stream intact
     * Replaces the original body stream with the tee stream in the request

### Comprehensive Test Suite

A comprehensive test suite has been implemented in `tests/sigv4_test.c`:

1. **Basic Tests**:
   * Test for SigV4 signing without a body
   * Test for SigV4 signing with a body

2. **Advanced Tests**:
   * Test for SigV4 signing with a pre-calculated payload hash
   * Test for SigV4 signing with a tee stream
   * Test for SigV4 signing with invalid parameters

3. **Test Helpers**:
   * Created mock credentials for testing
   * Implemented helper functions to create test HTTP requests

## Next Steps / Remaining TODOs

1. **Query String Extraction (Step 1.3)**:
   * **Verify Robustness**: The current manual extraction of the query string by searching for '?' is a fallback. Investigate if `aws_http_message` or related CRT components offer a more reliable way to access the original request URI or its query string component.

2. **Credentials Function Verification**:
   * **Confirm Accessors**: Double-check that the assumed function names (`aws_credentials_get_access_key_id`, `aws_credentials_get_secret_access_key`, `aws_credentials_get_session_token`) exactly match those provided by the `aws-c-auth` library being used via `FetchContent`.

3. **Multi-line Header Folding (Step 1.4)**:
   * **Verify Implementation**: Ensure the current whitespace folding logic correctly implements the SigV4 specification for handling multi-line header values (replacing consecutive whitespace with a single space).

4. **Integration Testing**:
   * **Real-world Testing**: Test the implementation with actual AWS services to verify correct signing behavior.
   * **Test Vectors**: Use official AWS SigV4 test vectors to validate the implementation against known inputs and outputs.
   * **Edge Cases**: Test additional scenarios, including paths with special characters, complex query parameters, etc.

5. **Performance Optimization**:
   * **Memory Usage**: Review the tee stream implementation for potential memory optimizations, especially for large request bodies.
   * **Buffer Management**: Consider implementing buffer size limits or chunked buffering for very large streams.

6. **Documentation**:
   * **API Documentation**: Add comprehensive documentation for the tee stream API.
   * **Usage Examples**: Provide examples of how to use the SigV4 signing functionality.
