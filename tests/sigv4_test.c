/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/auth/signing.h>
#include <aws/auth/credentials.h>
#include <aws/common/date_time.h>
#include <aws/http/request_response.h>
#include <aws/io/stream.h>

#include <aws/testing/aws_test_harness.h>

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
    if (!request) {
        return NULL;
    }
    
    struct aws_byte_cursor method = aws_byte_cursor_from_c_str("GET");
    struct aws_byte_cursor path = aws_byte_cursor_from_c_str("/");
    
    int result = aws_http_message_set_request_method(request, method);
    if (result != AWS_OP_SUCCESS) {
        aws_http_message_destroy(request);
        return NULL;
    }
    
    result = aws_http_message_set_request_path(request, path);
    if (result != AWS_OP_SUCCESS) {
        aws_http_message_destroy(request);
        return NULL;
    }
    
    /* Add a host header */
    struct aws_http_header host_header = {
        .name = aws_byte_cursor_from_c_str("Host"),
        .value = aws_byte_cursor_from_c_str("example.amazonaws.com"),
    };
    result = aws_http_message_add_header(request, host_header);
    if (result != AWS_OP_SUCCESS) {
        aws_http_message_destroy(request);
        return NULL;
    }
    
    /* Add a body if requested */
    if (with_body) {
        struct aws_byte_cursor body_content = aws_byte_cursor_from_c_str("Test request body");
        struct aws_input_stream *body_stream = aws_input_stream_new_from_cursor(allocator, &body_content);
        if (!body_stream) {
            aws_http_message_destroy(request);
            return NULL;
        }
        
        aws_http_message_set_body_stream(request, body_stream);
        aws_input_stream_destroy(body_stream);  // Destroy the stream after setting
    }
    
    return request;
}

/* Test basic SigV4 signing without a body */
static int s_test_sigv4_sign_request_no_body(struct aws_allocator *allocator, void *ctx) {
    (void)ctx;
    
    /* Create test request */
    struct aws_http_message *request = s_create_test_request(allocator, false);
    if (!request) {
        return AWS_OP_ERR;
    }
    
    /* Create test credentials */
    struct aws_credentials *credentials = s_create_test_credentials(allocator);
    if (!credentials) {
        aws_http_message_destroy(request);
        return AWS_OP_ERR;
    }
    
    /* Create signing date */
    struct aws_date_time signing_date;
    struct aws_byte_buf date_str_buf;
    struct aws_byte_cursor date_str_cursor = aws_byte_cursor_from_c_str("2015-08-30T12:36:00Z");
    aws_byte_buf_init_from_cursor(&date_str_buf, allocator, &date_str_cursor);
    
    aws_date_time_init_from_str(&signing_date, &date_str_buf, AWS_DATE_FORMAT_ISO_8601);
    
    /* Sign the request */
    struct aws_byte_cursor region = aws_byte_cursor_from_c_str("us-east-1");
    struct aws_byte_cursor service = aws_byte_cursor_from_c_str("service");
    
    int sign_result = aws_sigv4_sign_request(allocator, request, credentials, region, service, &signing_date);
    if (sign_result != AWS_OP_SUCCESS) {
        aws_byte_buf_clean_up(&date_str_buf);
        aws_http_message_destroy(request);
        aws_credentials_release(credentials);
        return AWS_OP_ERR;
    }
    
    /* Verify that the required headers were added */
    struct aws_byte_cursor auth_header_name = aws_byte_cursor_from_c_str("Authorization");
    struct aws_byte_cursor date_header_name = aws_byte_cursor_from_c_str("X-Amz-Date");
    
    struct aws_byte_cursor auth_header_value;
    struct aws_byte_cursor date_header_value;
    
    sign_result = aws_http_message_get_header(request, &auth_header_value, &auth_header_name);
    if (sign_result != AWS_OP_SUCCESS) {
        aws_byte_buf_clean_up(&date_str_buf);
        aws_http_message_destroy(request);
        aws_credentials_release(credentials);
        return AWS_OP_ERR;
    }
    
    sign_result = aws_http_message_get_header(request, &date_header_value, &date_header_name);
    if (sign_result != AWS_OP_SUCCESS) {
        aws_byte_buf_clean_up(&date_str_buf);
        aws_http_message_destroy(request);
        aws_credentials_release(credentials);
        return AWS_OP_ERR;
    }
    
    /* Verify the date header value */
    struct aws_byte_cursor expected_date = aws_byte_cursor_from_c_str("20150830T123600Z");
    if (!aws_byte_cursor_eq(&date_header_value, &expected_date)) {
        aws_byte_buf_clean_up(&date_str_buf);
        aws_http_message_destroy(request);
        aws_credentials_release(credentials);
        return AWS_OP_ERR;
    }
    
    /* Clean up */
    aws_byte_buf_clean_up(&date_str_buf);
    aws_http_message_destroy(request);
    aws_credentials_release(credentials);
    
    return AWS_OP_SUCCESS;
}

/* Test SigV4 signing with a body */
static int s_test_sigv4_sign_request_with_body(struct aws_allocator *allocator, void *ctx) {
    (void)ctx;
    
    /* Create test request with body */
    struct aws_http_message *request = s_create_test_request(allocator, true);
    if (!request) {
        return AWS_OP_ERR;
    }
    
    /* Create test credentials */
    struct aws_credentials *credentials = s_create_test_credentials(allocator);
    if (!credentials) {
        aws_http_message_destroy(request);
        return AWS_OP_ERR;
    }
    
    /* Create signing date */
    struct aws_date_time signing_date;
    struct aws_byte_buf date_str_buf_body;
    struct aws_byte_cursor date_str_cursor_body = aws_byte_cursor_from_c_str("2015-08-30T12:36:00Z");
    aws_byte_buf_init_from_cursor(&date_str_buf_body, allocator, &date_str_cursor_body);
    
    aws_date_time_init_from_str(&signing_date, &date_str_buf_body, AWS_DATE_FORMAT_ISO_8601);
    
    /* Sign the request */
    struct aws_byte_cursor region = aws_byte_cursor_from_c_str("us-east-1");
    struct aws_byte_cursor service = aws_byte_cursor_from_c_str("service");
    
    int sign_result = aws_sigv4_sign_request(allocator, request, credentials, region, service, &signing_date);
    if (sign_result != AWS_OP_SUCCESS) {
        aws_byte_buf_clean_up(&date_str_buf_body);
        aws_http_message_destroy(request);
        aws_credentials_release(credentials);
        return AWS_OP_ERR;
    }
    
    /* Verify that the required headers were added */
    struct aws_byte_cursor auth_header_name = aws_byte_cursor_from_c_str("Authorization");
    struct aws_byte_cursor date_header_name = aws_byte_cursor_from_c_str("X-Amz-Date");
    
    struct aws_byte_cursor auth_header_value;
    struct aws_byte_cursor date_header_value;
    
    sign_result = aws_http_message_get_header(request, &auth_header_value, &auth_header_name);
    if (sign_result != AWS_OP_SUCCESS) {
        aws_byte_buf_clean_up(&date_str_buf_body);
        aws_http_message_destroy(request);
        aws_credentials_release(credentials);
        return AWS_OP_ERR;
    }
    
    sign_result = aws_http_message_get_header(request, &date_header_value, &date_header_name);
    if (sign_result != AWS_OP_SUCCESS) {
        aws_byte_buf_clean_up(&date_str_buf_body);
        aws_http_message_destroy(request);
        aws_credentials_release(credentials);
        return AWS_OP_ERR;
    }
    
    /* Verify the date header value */
    struct aws_byte_cursor expected_date = aws_byte_cursor_from_c_str("20150830T123600Z");
    if (!aws_byte_cursor_eq(&date_header_value, &expected_date)) {
        aws_byte_buf_clean_up(&date_str_buf_body);
        aws_http_message_destroy(request);
        aws_credentials_release(credentials);
        return AWS_OP_ERR;
    }
    
    /* Verify that the body stream is still usable */
    struct aws_input_stream *body_stream = aws_http_message_get_body_stream(request);
    if (!body_stream) {
        aws_byte_buf_clean_up(&date_str_buf_body);
        aws_http_message_destroy(request);
        aws_credentials_release(credentials);
        return AWS_OP_ERR;
    }
    
    /* Read from the body stream to verify it's still usable */
    struct aws_byte_buf body_buf;
    aws_byte_buf_init(&body_buf, allocator, 100);
    
    sign_result = aws_input_stream_read(body_stream, &body_buf);
    if (sign_result != AWS_OP_SUCCESS) {
        aws_byte_buf_clean_up(&date_str_buf_body);
        aws_byte_buf_clean_up(&body_buf);
        aws_http_message_destroy(request);
        aws_credentials_release(credentials);
        return AWS_OP_ERR;
    }
    
    /* Verify the body content */
    struct aws_byte_cursor expected_body = aws_byte_cursor_from_c_str("Test request body");
    struct aws_byte_cursor actual_body = aws_byte_cursor_from_buf(&body_buf);
    
    if (!aws_byte_cursor_eq(&actual_body, &expected_body)) {
        aws_byte_buf_clean_up(&date_str_buf_body);
        aws_byte_buf_clean_up(&body_buf);
        aws_http_message_destroy(request);
        aws_credentials_release(credentials);
        return AWS_OP_ERR;
    }
    
    /* Clean up */
    aws_byte_buf_clean_up(&date_str_buf_body);
    aws_byte_buf_clean_up(&body_buf);
    aws_http_message_destroy(request);
    aws_credentials_release(credentials);
    
    return AWS_OP_SUCCESS;
}

/* Register the tests */
AWS_TEST_CASE(sigv4_sign_request_no_body, s_test_sigv4_sign_request_no_body);
AWS_TEST_CASE(sigv4_sign_request_with_body, s_test_sigv4_sign_request_with_body);
