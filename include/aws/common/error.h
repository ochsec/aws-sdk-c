#ifndef AWS_COMMON_ERROR_H
#define AWS_COMMON_ERROR_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/exports.h>
#include <stdint.h> // For int32_t

AWS_EXTERN_C_BEGIN

/* Represents the range of error codes for the aws-c-common library */
#define AWS_C_COMMON_ERROR_CODE_BEGIN 0
#define AWS_C_COMMON_ERROR_CODE_END 1023

/**
 * @brief Common error codes used across the SDK.
 *
 * Service-specific errors will be defined elsewhere.
 */
enum aws_common_error {
    AWS_ERROR_SUCCESS = 0,

    /* Memory allocation failures */
    AWS_ERROR_OOM = AWS_C_COMMON_ERROR_CODE_BEGIN + 1,

    /* Unknown error */
    AWS_ERROR_UNKNOWN = AWS_C_COMMON_ERROR_CODE_BEGIN + 2,

    /* Invalid argument */
    AWS_ERROR_INVALID_ARGUMENT = AWS_C_COMMON_ERROR_CODE_BEGIN + 3,

    /* TODO: Add more common error codes as needed */

    AWS_ERROR_LAST = AWS_C_COMMON_ERROR_CODE_END
};

/**
 * @brief Returns the last error code set for the current thread.
 *
 * This function is thread-local. Each thread maintains its own last error code.
 *
 * @return The last error code set by an SDK function call in the current thread.
 */
AWS_COMMON_API
int aws_last_error(void);

/**
 * @brief Returns a string representation of the given error code.
 *
 * @param error_code The error code to convert to a string.
 * @return A constant string describing the error code. Returns "Unknown Error"
 *         if the error code is not recognized.
 */
AWS_COMMON_API
const char *aws_error_str(int error_code);

/**
 * @brief Returns a more detailed description of the last error that occurred
 *        on the current thread.
 *
 * This function is thread-local.
 *
 * @return A constant string providing more details about the last error, or
 *         NULL if no detailed description is available.
 */
AWS_COMMON_API
const char *aws_error_debug_str(void);

/**
 * @brief Sets the last error code for the current thread.
 *
 * This is typically called internally by SDK functions when an error occurs.
 *
 * @param error_code The error code to set.
 */
AWS_COMMON_API
void aws_raise_error(int error_code);

/* Internal function to register error info */
typedef const char *(*aws_error_str_fn)(int error_code);
typedef const char *(*aws_error_debug_str_fn)(int error_code);

struct aws_error_info {
    aws_error_str_fn error_str_fn;
    aws_error_debug_str_fn debug_str_fn;
    const char *literal_name;
    const char *description;
};

struct aws_error_info_list {
    const struct aws_error_info *error_list;
    uint16_t count;
};

/**
 * @internal Registers error strings for a specific error code range.
 * Typically called by libraries during initialization.
 */
AWS_COMMON_API
void aws_register_error_info(const struct aws_error_info_list *error_info_list);

/**
 * @internal Unregisters error strings. Typically called during library shutdown.
 */
AWS_COMMON_API
void aws_unregister_error_info(const struct aws_error_info_list *error_info_list);


AWS_EXTERN_C_END

#endif /* AWS_COMMON_ERROR_H */
