/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/date_time.h>
#include <aws/common/string.h>
#include <aws/common/error.h>

#include <time.h>
#include <stdio.h> /* For snprintf */
#include <stdlib.h> /* For NULL */
#include <inttypes.h> /* For PRIu64 */

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h> /* For gettimeofday */
#endif

#define MILLIS_PER_SEC 1000
#define BUF_SIZE 128 /* Buffer size for formatting */

void aws_date_time_init_now(struct aws_date_time *dt) {
#ifdef _WIN32
    /* Windows specific implementation using GetSystemTimeAsFileTime */
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    /* Convert FILETIME (100-nanosecond intervals since Jan 1, 1601) to Unix epoch */
    uint64_t filetime_intervals = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    /* Difference between Windows epoch (1601) and Unix epoch (1970) in 100-nanosecond intervals */
    const uint64_t EPOCH_DIFF = 116444736000000000ULL;
    uint64_t intervals_since_unix_epoch = filetime_intervals - EPOCH_DIFF;
    uint64_t millis_since_epoch = intervals_since_unix_epoch / 10000; /* Convert 100ns intervals to ms */
    aws_date_time_init_epoch_millis(dt, millis_since_epoch);
#else
    /* POSIX implementation using gettimeofday */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    dt->epoch_secs = tv.tv_sec;
    dt->milliseconds = (uint16_t)(tv.tv_usec / 1000); /* Convert microseconds to milliseconds */
#endif
}

void aws_date_time_init_epoch_secs(struct aws_date_time *dt, time_t epoch_secs) {
    dt->epoch_secs = epoch_secs;
    dt->milliseconds = 0;
}

void aws_date_time_init_epoch_millis(struct aws_date_time *dt, uint64_t epoch_millis) {
    dt->epoch_secs = (time_t)(epoch_millis / MILLIS_PER_SEC);
    dt->milliseconds = (uint16_t)(epoch_millis % MILLIS_PER_SEC);
}

/* Basic parsing/formatting using standard C functions for now */
/* TODO: Implement robust RFC822 and ISO8601 parsing/formatting */
int aws_date_time_init_from_string(
    struct aws_date_time *dt,
    const struct aws_string *date_str,
    enum aws_date_format format) {

    (void)dt;
    (void)date_str;
    (void)format;
    aws_raise_error(AWS_ERROR_UNKNOWN); /* Indicate not implemented */
    return AWS_OP_ERR; /* AWS_OP_ERR needs to be defined */
}

struct aws_string *aws_date_time_to_string(
    const struct aws_date_time *dt,
    enum aws_date_format format,
    struct aws_allocator *allocator) {

    char buffer[BUF_SIZE];
    int len = 0;

    switch (format) {
        case AWS_DATE_FORMAT_RFC822: {
            struct tm timeinfo;
#ifdef _WIN32
            gmtime_s(&timeinfo, &dt->epoch_secs);
#else
            gmtime_r(&dt->epoch_secs, &timeinfo);
#endif
            /* Format: "Mon, 02 Jan 2006 15:04:05 GMT" */
            len = strftime(buffer, BUF_SIZE, "%a, %d %b %Y %H:%M:%S GMT", &timeinfo);
            break;
        }
        case AWS_DATE_FORMAT_ISO_8601: {
             struct tm timeinfo;
#ifdef _WIN32
            gmtime_s(&timeinfo, &dt->epoch_secs);
#else
            gmtime_r(&dt->epoch_secs, &timeinfo);
#endif
            /* Format: "YYYY-MM-DDTHH:MM:SS.sssZ" */
            len = snprintf(buffer, BUF_SIZE, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
                           timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, dt->milliseconds);
            break;
        }
        case AWS_DATE_FORMAT_UNIX_TIMESTAMP:
            len = snprintf(buffer, BUF_SIZE, "%" PRIu64, aws_date_time_get_epoch_millis(dt));
            break;
        default:
            aws_raise_error(AWS_ERROR_INVALID_ARGUMENT);
            return NULL;
    }

    if (len <= 0 || len >= BUF_SIZE) {
        aws_raise_error(AWS_ERROR_UNKNOWN); /* Formatting error */
        return NULL;
    }

    return aws_string_new_from_bytes(allocator, (const uint8_t *)buffer, (size_t)len);
}


time_t aws_date_time_get_epoch_secs(const struct aws_date_time *dt) {
    return dt->epoch_secs;
}

uint64_t aws_date_time_get_epoch_millis(const struct aws_date_time *dt) {
    return ((uint64_t)dt->epoch_secs * MILLIS_PER_SEC) + dt->milliseconds;
}
