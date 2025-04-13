/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#ifndef AWS_AUTH_SIGV4_H
#define AWS_AUTH_SIGV4_H

#include <aws/common/common.h>
#include <aws/http/request_response.h>
#include <aws/common/date_time.h>
#include <aws/auth/credentials.h>

AWS_EXTERN_C_BEGIN

/**
 * Signs an HTTP request using AWS Signature Version 4 (SigV4) algorithm.
 *
 * @param allocator Memory allocator to use for any required allocations.
 * @param request The HTTP request to sign.
 * @param credentials AWS credentials to use for signing.
 * @param region AWS region for the request.
 * @param service AWS service identifier.
 * @param signing_date Timestamp to use for the signature.
 *
 * @return AWS_OP_SUCCESS if the request was successfully signed, AWS_OP_ERR otherwise.
 */
AWS_AUTH_API int aws_sigv4_sign_request(
    struct aws_allocator *allocator, 
    struct aws_http_message *request, 
    struct aws_credentials *credentials, 
    struct aws_byte_cursor region, 
    struct aws_byte_cursor service, 
    struct aws_date_time *signing_date
);

AWS_EXTERN_C_END

#endif /* AWS_AUTH_SIGV4_H */