#ifndef AWS_IO_IO_H
#define AWS_IO_IO_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/common.h> // Include common definitions

/* Define AWS_IO_API as empty for now, similar to AWS_COMMON_API */
#define AWS_IO_API


AWS_EXTERN_C_BEGIN

/** IO Library specific error codes */
#define AWS_C_IO_ERROR_CODE_BEGIN 2048
#define AWS_C_IO_ERROR_CODE_END 3071

/* Stream specific errors */
#define AWS_ERROR_STREAM_READ_FAILED (AWS_C_IO_ERROR_CODE_BEGIN + 1)
#define AWS_ERROR_STREAM_UNSEEKABLE (AWS_C_IO_ERROR_CODE_BEGIN + 2)
#define AWS_ERROR_STREAM_UNKNOWN_LENGTH (AWS_C_IO_ERROR_CODE_BEGIN + 3)
#define AWS_ERROR_STREAM_SEEK_FAILED (AWS_C_IO_ERROR_CODE_BEGIN + 4)

/* Add other IO errors here */

/**
 * Call this function to initialize the aws-c-io library.
 * Needs to be called before using any other functions in the library.
 */
AWS_IO_API void aws_io_library_init(struct aws_allocator *allocator);

/**
 * Call this function to shut down the aws-c-io library.
 */
AWS_IO_API void aws_io_library_clean_up(void);

AWS_EXTERN_C_END

#endif /* AWS_IO_IO_H */
