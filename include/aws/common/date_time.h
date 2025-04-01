#ifndef AWS_COMMON_DATE_TIME_H
#define AWS_COMMON_DATE_TIME_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/exports.h>
#include <aws/common/allocator.h>
#include <aws/common/string.h> /* Include string definition */
#include <time.h> /* For time_t */
#include <stdint.h> /* For uint64_t */

AWS_EXTERN_C_BEGIN

/**
 * @brief Represents a specific point in time.
 *
 * Can store time with millisecond precision.
 */
struct aws_date_time {
    time_t epoch_secs;      /* Seconds since epoch */
    uint16_t milliseconds;  /* Milliseconds part (0-999) */
};

/**
 * @brief Date/Time format specifiers for parsing and formatting.
 */
enum aws_date_format {
    AWS_DATE_FORMAT_UNKNOWN = 0,
    AWS_DATE_FORMAT_RFC822,      /* RFC 822 / RFC 1123 */
    AWS_DATE_FORMAT_ISO_8601,    /* ISO 8601 / RFC 3339 */
    AWS_DATE_FORMAT_UNIX_TIMESTAMP /* Seconds since epoch */
};

/**
 * @brief Initializes an aws_date_time structure to the current time.
 *
 * @param dt Pointer to the structure to initialize.
 */
AWS_COMMON_API
void aws_date_time_init_now(struct aws_date_time *dt);

/**
 * @brief Initializes an aws_date_time structure from epoch seconds.
 *
 * @param dt Pointer to the structure to initialize.
 * @param epoch_secs Seconds since the Unix epoch.
 */
AWS_COMMON_API
void aws_date_time_init_epoch_secs(struct aws_date_time *dt, time_t epoch_secs);

/**
 * @brief Initializes an aws_date_time structure from epoch milliseconds.
 *
 * @param dt Pointer to the structure to initialize.
 * @param epoch_millis Milliseconds since the Unix epoch.
 */
AWS_COMMON_API
void aws_date_time_init_epoch_millis(struct aws_date_time *dt, uint64_t epoch_millis);

/**
 * @brief Parses a date/time string in a specified format.
 *
 * @param dt Pointer to the structure to store the parsed time.
 * @param date_str The string to parse.
 * @param format The expected format of the string.
 * @return AWS_OP_SUCCESS on successful parsing, AWS_OP_ERR otherwise.
 *         Check aws_last_error() for details on failure.
 */
AWS_COMMON_API
int aws_date_time_init_from_string(
    struct aws_date_time *dt,
    const struct aws_string *date_str, /* Use aws_string */
    enum aws_date_format format);

/**
 * @brief Formats an aws_date_time structure into a string.
 *
 * @param dt Pointer to the date/time structure to format.
 * @param format The desired output format.
 * @param allocator Allocator for the resulting string.
 * @return A new aws_string containing the formatted date/time, or NULL on failure.
 *         The caller is responsible for destroying the returned string.
 */
AWS_COMMON_API
struct aws_string *aws_date_time_to_string(
    const struct aws_date_time *dt,
    enum aws_date_format format,
    struct aws_allocator *allocator);

/**
 * @brief Gets the time in seconds since the epoch.
 *
 * @param dt Pointer to the date/time structure.
 * @return Time in seconds since the epoch.
 */
AWS_COMMON_API
time_t aws_date_time_get_epoch_secs(const struct aws_date_time *dt);

/**
 * @brief Gets the time in milliseconds since the epoch.
 *
 * @param dt Pointer to the date/time structure.
 * @return Time in milliseconds since the epoch.
 */
AWS_COMMON_API
uint64_t aws_date_time_get_epoch_millis(const struct aws_date_time *dt);


AWS_EXTERN_C_END

#endif /* AWS_COMMON_DATE_TIME_H */
