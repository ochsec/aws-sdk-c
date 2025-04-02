/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#ifndef AWS_AUTH_SIGV4_H
#define AWS_AUTH_SIGV4_H

#include <aws/common/common.h>
#include <aws/http/request_response.h>

/* Define AWS_AUTH_API if not already defined */
#ifndef AWS_AUTH_API
#define AWS_AUTH_API
#endif

struct aws_credentials;
struct aws_date_time;

AWS_EXTERN_C_BEGIN

/**
 * Signs an AWS request using Signature Version 4 (SigV4).
 *
 * This function implements the AWS Signature Version 4 signing process as described in:
 * https://docs.aws.amazon.com/general/latest/gr/signature-version-4.html
 *
 * The function will:
 * 1. Create a canonical request from the HTTP request
 * 2. Create a string to sign using the canonical request
 * 3. Calculate the signature using the AWS credentials
 * 4. Add the signature and related headers to the request
 *
 * @param allocator Memory allocator to use
 * @param request The HTTP request to sign (will be modified with signature headers)
 * @param credentials AWS credentials to use for signing
 * @param region AWS region for the request (e.g., "us-east-1")
 * @param service_name AWS service name for the request (e.g., "s3", "dynamodb")
 * @param signing_date Date/time to use for signing
 *
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure with error code set
 *
 * @note For requests with streaming bodies, the caller should either:
 *   1. Provide a seekable stream that can be rewound after hashing, or
 *   2. Pre-calculate the payload hash and set it in the "x-amz-content-sha256" header
 */
AWS_AUTH_API int aws_sigv4_sign_request(
    struct aws_allocator *allocator,
    struct aws_http_message *request,
    const struct aws_credentials *credentials,
    struct aws_byte_cursor region,
    struct aws_byte_cursor service_name,
    const struct aws_date_time *signing_date);

AWS_EXTERN_C_END

#endif /* AWS_AUTH_SIGV4_H */
