/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/auth/sigv4.h>
#include <aws/auth/credentials.h>
#include <aws/common/date_time.h>
#include <aws/http/request_response.h>
#include <aws/io/tee_input_stream.h>
#include <aws/io/input_stream.h>

#include <aws/testing/aws_test_harness.h>

/* Define test macros */
#define ASSERT_SUCCESS(condition, ...) ASSERT_TRUE(condition == AWS_OP_SUCCESS, __VA_ARGS__)
#define ASSERT_FAILS(condition, ...) ASSERT_TRUE(condition != AWS_OP_SUCCESS, __VA_ARGS__)
#define ASSERT_NOT_NULL(condition, ...) ASSERT_TRUE(condition != NULL, __VA_ARGS__)
#define ASSERT_NULL(condition, ...) ASSERT_TRUE(condition == NULL, __VA_ARGS__)

/* Mock credentials for testing */
static struct aws_credentials *s_create_test_credentials(struct aws_allocator *allocator) {
    struct aws_byte_cursor access_key = aws_byte_cursor_from_c_str("AKIAIOSFODNN7EXAMPLE");
    struct aws_byte_cursor secret_key = aws_byte_cursor_from_c_str("wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY");
    struct aws_byte_cursor session_token = aws_byte_cursor_from_c_str("");
    
    return aws_credentials_new(allocator, access_key, secret_key, session_token, UINT64_MAX);
}

/* Helper to create a test HTTP request */
static struct aws_http_message *s_create_test_request(struct aws_allocator *allocator, bool with_body) {
    struct aws_http_message *request = aws_http_message_new_request(allocator);
    ASSERT_NOT_NULL(request, "Failed to create HTTP request");
    
    struct aws_byte_cursor method = aws_byte_cursor_from_c_str("GET");
    struct aws_byte_cursor path = aws_byte_cursor_from_c_str("/");
    
    ASSERT_SUCCESS(aws_http_message_set_request_method(request, method), "Failed to set request method");
    ASSERT_SUCCESS(aws_http_message_set_request_path(request, path), "Failed to set request path");
    
    /* Add a host header */
    struct aws_http_header host_header = {
        .name = aws_byte_cursor_from_c_str("Host"),
        .value = aws_byte_cursor_from_c_str("example.amazonaws.com"),
    };
    ASSERT_SUCCESS(aws_http_message_add_header(request, host_header), "Failed to add Host header");
    
    /* Add a body if requested */
    if (with_body) {
        struct aws_byte_cursor body_content = aws_byte_cursor_from_c_str("Test request body");
        struct aws_input_stream *body_stream = aws_input_stream_new_from_cursor(allocator, &body_content);
        ASSERT_NOT_NULL(body_stream, "Failed to create body stream");
        
        ASSERT_SUCCESS(aws_http_message_set_body_stream(request, body_stream), "Failed to set body stream");
    }
    
    return request;
}

/* Test basic SigV4 signing without a body */
static int s_test_sigv4_sign_request_no_body(struct aws_allocator *allocator, void *ctx) {
    (void)ctx;
    
    /* Create test request */
    struct aws_http_message *request = s_create_test_request(allocator, false);
    ASSERT_NOT_NULL(request, "Failed to create test request");
    
    /* Create test credentials */
    struct aws_credentials *credentials = s_create_test_credentials(allocator);
    ASSERT_NOT_NULL(credentials, "Failed to create test credentials");
    
    /* Create signing date */
    struct aws_date_time signing_date;
    aws_date_time_init_from_str(&signing_date, "2015-08-30T12:36:00Z", AWS_DATE_FORMAT_ISO_8601);
    
    /* Sign the request */
    struct aws_byte_cursor region = aws_byte_cursor_from_c_str("us-east-1");
    struct aws_byte_cursor service = aws_byte_cursor_from_c_str("service");
    
    ASSERT_SUCCESS(
        aws_sigv4_sign_request(allocator, request, credentials, region, service, &signing_date),
        "Failed to sign request");
    
    /* Verify that the required headers were added */
    struct aws_byte_cursor auth_header_name = aws_byte_cursor_from_c_str("Authorization");
    struct aws_byte_cursor date_header_name = aws_byte_cursor_from_c_str("X-Amz-Date");
    
    struct aws_byte_cursor auth_header_value;
    struct aws_byte_cursor date_header_value;
    
    ASSERT_SUCCESS(
        aws_http_message_get_header(request, &auth_header_value, &auth_header_name),
        "Authorization header not found");
    
    ASSERT_SUCCESS(
        aws_http_message_get_header(request, &date_header_value, &date_header_name),
        "X-Amz-Date header not found");
    
    /* Verify the date header value */
    struct aws_byte_cursor expected_date = aws_byte_cursor_from_c_str("20150830T123600Z");
    ASSERT_TRUE(aws_byte_cursor_eq(&date_header_value, &expected_date), "X-Amz-Date header has incorrect value");
    
    /* Clean up */
    aws_http_message_destroy(request);
    aws_credentials_release(credentials);
    
    return AWS_OP_SUCCESS;
}

/* Test SigV4 signing with a body */
static int s_test_sigv4_sign_request_with_body(struct aws_allocator *allocator, void *ctx) {
    (void)ctx;
    
    /* Create test request with body */
    struct aws_http_message *request = s_create_test_request(allocator, true);
    ASSERT_NOT_NULL(request, "Failed to create test request with body");
    
    /* Create test credentials */
    struct aws_credentials *credentials = s_create_test_credentials(allocator);
    ASSERT_NOT_NULL(credentials, "Failed to create test credentials");
    
    /* Create signing date */
    struct aws_date_time signing_date;
    aws_date_time_init_from_str(&signing_date, "2015-08-30T12:36:00Z", AWS_DATE_FORMAT_ISO_8601);
    
    /* Sign the request */
    struct aws_byte_cursor region = aws_byte_cursor_from_c_str("us-east-1");
    struct aws_byte_cursor service = aws_byte_cursor_from_c_str("service");
    
    ASSERT_SUCCESS(
        aws_sigv4_sign_request(allocator, request, credentials, region, service, &signing_date),
        "Failed to sign request with body");
    
    /* Verify that the required headers were added */
    struct aws_byte_cursor auth_header_name = aws_byte_cursor_from_c_str("Authorization");
    struct aws_byte_cursor date_header_name = aws_byte_cursor_from_c_str("X-Amz-Date");
    
    struct aws_byte_cursor auth_header_value;
    struct aws_byte_cursor date_header_value;
    
    ASSERT_SUCCESS(
        aws_http_message_get_header(request, &auth_header_value, &auth_header_name),
        "Authorization header not found");
    
    ASSERT_SUCCESS(
        aws_http_message_get_header(request, &date_header_value, &date_header_name),
        "X-Amz-Date header not found");
    
    /* Verify the date header value */
    struct aws_byte_cursor expected_date = aws_byte_cursor_from_c_str("20150830T123600Z");
    ASSERT_TRUE(aws_byte_cursor_eq(&date_header_value, &expected_date), "X-Amz-Date header has incorrect value");
    
    /* Verify that the body stream is still usable */
    struct aws_input_stream *body_stream = aws_http_message_get_body_stream(request);
    ASSERT_NOT_NULL(body_stream, "Body stream is missing after signing");
    
    /* Read from the body stream to verify it's still usable */
    struct aws_byte_buf body_buf;
    aws_byte_buf_init(&body_buf, allocator, 100);
    
    ASSERT_SUCCESS(aws_input_stream_read(body_stream, &body_buf), "Failed to read from body stream after signing");
    
    /* Verify the body content */
    struct aws_byte_cursor expected_body = aws_byte_cursor_from_c_str("Test request body");
    struct aws_byte_cursor actual_body = aws_byte_cursor_from_buf(&body_buf);
    
    ASSERT_TRUE(aws_byte_cursor_eq(&actual_body, &expected_body), 
                "Body content doesn't match after signing. Expected: %.*s, Got: %.*s",
                (int)expected_body.len, expected_body.ptr,
                (int)actual_body.len, actual_body.ptr);
    
    /* Clean up */
    aws_byte_buf_clean_up(&body_buf);
    aws_http_message_destroy(request);
    aws_credentials_release(credentials);
    
    /* Clean up the IO library */
    aws_io_library_clean_up();
    
    return AWS_OP_SUCCESS;
}

/* Test SigV4 signing with a pre-calculated payload hash */
static int s_test_sigv4_sign_request_with_precalculated_hash(struct aws_allocator *allocator, void *ctx) {
    (void)ctx;
    
    /* Create test request with body */
    struct aws_http_message *request = s_create_test_request(allocator, true);
    ASSERT_NOT_NULL(request, "Failed to create test request with body");
    
    /* Add pre-calculated hash header */
    /* Hash of "Test request body" */
    struct aws_http_header hash_header = {
        .name = aws_byte_cursor_from_c_str("x-amz-content-sha256"),
        .value = aws_byte_cursor_from_c_str("9b7a28bdd098b4b42887609d12a9a0a776a8f73839c40c5c9f5a202e3f5dc03a"),
    };
    ASSERT_SUCCESS(aws_http_message_add_header(request, hash_header), "Failed to add hash header");
    
    /* Create test credentials */
    struct aws_credentials *credentials = s_create_test_credentials(allocator);
    ASSERT_NOT_NULL(credentials, "Failed to create test credentials");
    
    /* Create signing date */
    struct aws_date_time signing_date;
    aws_date_time_init_from_str(&signing_date, "2015-08-30T12:36:00Z", AWS_DATE_FORMAT_ISO_8601);
    
    /* Sign the request */
    struct aws_byte_cursor region = aws_byte_cursor_from_c_str("us-east-1");
    struct aws_byte_cursor service = aws_byte_cursor_from_c_str("service");
    
    ASSERT_SUCCESS(
        aws_sigv4_sign_request(allocator, request, credentials, region, service, &signing_date),
        "Failed to sign request with pre-calculated hash");
    
    /* Verify that the required headers were added */
    struct aws_byte_cursor auth_header_name = aws_byte_cursor_from_c_str("Authorization");
    struct aws_byte_cursor date_header_name = aws_byte_cursor_from_c_str("X-Amz-Date");
    
    struct aws_byte_cursor auth_header_value;
    struct aws_byte_cursor date_header_value;
    
    ASSERT_SUCCESS(
        aws_http_message_get_header(request, &auth_header_value, &auth_header_name),
        "Authorization header not found");
    
    ASSERT_SUCCESS(
        aws_http_message_get_header(request, &date_header_value, &date_header_name),
        "X-Amz-Date header not found");
    
    /* Clean up */
    aws_http_message_destroy(request);
    aws_credentials_release(credentials);
    
    return AWS_OP_SUCCESS;
}

/* Test SigV4 signing with a tee stream */
static int s_test_sigv4_sign_request_with_tee_stream(struct aws_allocator *allocator, void *ctx) {
    (void)ctx;
    
    /* Initialize the IO library */
    aws_io_library_init(allocator);
    
    /* Create a body stream */
    struct aws_byte_cursor body_content = aws_byte_cursor_from_c_str("Test request body");
    struct aws_input_stream *body_stream = aws_input_stream_new_from_cursor(allocator, &body_content);
    ASSERT_NOT_NULL(body_stream, "Failed to create body stream");
    
    /* Create a tee stream from the body stream */
    struct aws_input_stream *tee_stream = aws_tee_input_stream_new(allocator, body_stream);
    ASSERT_NOT_NULL(tee_stream, "Failed to create tee stream");
    
    /* Create test request */
    struct aws_http_message *request = aws_http_message_new_request(allocator);
    ASSERT_NOT_NULL(request, "Failed to create HTTP request");
    
    struct aws_byte_cursor method = aws_byte_cursor_from_c_str("GET");
    struct aws_byte_cursor path = aws_byte_cursor_from_c_str("/");
    
    ASSERT_SUCCESS(aws_http_message_set_request_method(request, method), "Failed to set request method");
    ASSERT_SUCCESS(aws_http_message_set_request_path(request, path), "Failed to set request path");
    
    /* Add a host header */
    struct aws_http_header host_header = {
        .name = aws_byte_cursor_from_c_str("Host"),
        .value = aws_byte_cursor_from_c_str("example.amazonaws.com"),
    };
    ASSERT_SUCCESS(aws_http_message_add_header(request, host_header), "Failed to add Host header");
    
    /* Set the tee stream as the body */
    ASSERT_SUCCESS(aws_http_message_set_body_stream(request, tee_stream), "Failed to set tee stream as body");
    
    /* Create test credentials */
    struct aws_credentials *credentials = s_create_test_credentials(allocator);
    ASSERT_NOT_NULL(credentials, "Failed to create test credentials");
    
    /* Create signing date */
    struct aws_date_time signing_date;
    aws_date_time_init_from_str(&signing_date, "2015-08-30T12:36:00Z", AWS_DATE_FORMAT_ISO_8601);
    
    /* Sign the request */
    struct aws_byte_cursor region = aws_byte_cursor_from_c_str("us-east-1");
    struct aws_byte_cursor service = aws_byte_cursor_from_c_str("service");
    
    ASSERT_SUCCESS(
        aws_sigv4_sign_request(allocator, request, credentials, region, service, &signing_date),
        "Failed to sign request with tee stream");
    
    /* Verify that the required headers were added */
    struct aws_byte_cursor auth_header_name = aws_byte_cursor_from_c_str("Authorization");
    struct aws_byte_cursor date_header_name = aws_byte_cursor_from_c_str("X-Amz-Date");
    
    struct aws_byte_cursor auth_header_value;
    struct aws_byte_cursor date_header_value;
    
    ASSERT_SUCCESS(
        aws_http_message_get_header(request, &auth_header_value, &auth_header_name),
        "Authorization header not found");
    
    ASSERT_SUCCESS(
        aws_http_message_get_header(request, &date_header_value, &date_header_name),
        "X-Amz-Date header not found");
    
    /* Verify that the body stream is still usable */
    struct aws_input_stream *final_body_stream = aws_http_message_get_body_stream(request);
    ASSERT_NOT_NULL(final_body_stream, "Body stream is missing after signing");
    
    /* Read from the body stream to verify it's still usable */
    struct aws_byte_buf body_buf;
    aws_byte_buf_init(&body_buf, allocator, 100);
    
    ASSERT_SUCCESS(aws_input_stream_read(final_body_stream, &body_buf), "Failed to read from body stream after signing");
    
    /* Verify the body content */
    struct aws_byte_cursor expected_body = aws_byte_cursor_from_c_str("Test request body");
    struct aws_byte_cursor actual_body = aws_byte_cursor_from_buf(&body_buf);
    
    ASSERT_TRUE(aws_byte_cursor_eq(&actual_body, &expected_body), 
                "Body content doesn't match after signing. Expected: %.*s, Got: %.*s",
                (int)expected_body.len, expected_body.ptr,
                (int)actual_body.len, actual_body.ptr);
    
    /* Clean up */
    aws_byte_buf_clean_up(&body_buf);
    aws_http_message_destroy(request);
    aws_credentials_release(credentials);
    
    return AWS_OP_SUCCESS;
}

/* Test SigV4 signing with invalid parameters */
static int s_test_sigv4_sign_request_invalid_params(struct aws_allocator *allocator, void *ctx) {
    (void)ctx;
    
    /* Create test request */
    struct aws_http_message *request = s_create_test_request(allocator, false);
    ASSERT_NOT_NULL(request, "Failed to create test request");
    
    /* Create test credentials */
    struct aws_credentials *credentials = s_create_test_credentials(allocator);
    ASSERT_NOT_NULL(credentials, "Failed to create test credentials");
    
    /* Create signing date */
    struct aws_date_time signing_date;
    aws_date_time_init_from_str(&signing_date, "2015-08-30T12:36:00Z", AWS_DATE_FORMAT_ISO_8601);
    
    /* Test with NULL allocator */
    struct aws_byte_cursor region = aws_byte_cursor_from_c_str("us-east-1");
    struct aws_byte_cursor service = aws_byte_cursor_from_c_str("service");
    
    ASSERT_FAILS(
        aws_sigv4_sign_request(NULL, request, credentials, region, service, &signing_date),
        "Signing with NULL allocator should fail");
    
    /* Test with NULL request */
    ASSERT_FAILS(
        aws_sigv4_sign_request(allocator, NULL, credentials, region, service, &signing_date),
        "Signing with NULL request should fail");
    
    /* Test with NULL credentials */
    ASSERT_FAILS(
        aws_sigv4_sign_request(allocator, request, NULL, region, service, &signing_date),
        "Signing with NULL credentials should fail");
    
    /* Test with empty region */
    struct aws_byte_cursor empty_region = aws_byte_cursor_from_c_str("");
    ASSERT_FAILS(
        aws_sigv4_sign_request(allocator, request, credentials, empty_region, service, &signing_date),
        "Signing with empty region should fail");
    
    /* Test with empty service */
    struct aws_byte_cursor empty_service = aws_byte_cursor_from_c_str("");
    ASSERT_FAILS(
        aws_sigv4_sign_request(allocator, request, credentials, region, empty_service, &signing_date),
        "Signing with empty service should fail");
    
    /* Test with NULL signing date */
    ASSERT_FAILS(
        aws_sigv4_sign_request(allocator, request, credentials, region, service, NULL),
        "Signing with NULL signing date should fail");
    
    /* Clean up */
    aws_http_message_destroy(request);
    aws_credentials_release(credentials);
    
    return AWS_OP_SUCCESS;
}

/* Register the tests */
AWS_TEST_CASE(sigv4_sign_request_no_body, s_test_sigv4_sign_request_no_body);
AWS_TEST_CASE(sigv4_sign_request_with_body, s_test_sigv4_sign_request_with_body);
AWS_TEST_CASE(sigv4_sign_request_with_precalculated_hash, s_test_sigv4_sign_request_with_precalculated_hash);
AWS_TEST_CASE(sigv4_sign_request_with_tee_stream, s_test_sigv4_sign_request_with_tee_stream);
AWS_TEST_CASE(sigv4_sign_request_invalid_params, s_test_sigv4_sign_request_invalid_params);
