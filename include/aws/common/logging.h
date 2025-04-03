#ifndef AWS_COMMON_LOGGING_H
#define AWS_COMMON_LOGGING_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/exports.h>
#include <aws/common/common.h> /* For AWS_C_COMMON_PACKAGE_ID */
#include <aws/common/allocator.h>
#include <stdarg.h> /* For va_list */

/* Stride between log subject ranges for different libraries */
#define AWS_LOG_SUBJECT_STRIDE 1024

/* Macros to calculate the begin/end of a log subject range for a package */
#define AWS_LOG_SUBJECT_BEGIN_RANGE(x) ((x)*AWS_LOG_SUBJECT_STRIDE)
#define AWS_LOG_SUBJECT_END_RANGE(x) (((x) + 1) * AWS_LOG_SUBJECT_STRIDE - 1)

AWS_EXTERN_C_BEGIN

/**
 * @brief Logging level definitions.
 */
enum aws_log_level {
    AWS_LL_NONE = 0,
    AWS_LL_FATAL,
    AWS_LL_ERROR,
    AWS_LL_WARN,
    AWS_LL_INFO,
    AWS_LL_DEBUG,
    AWS_LL_TRACE,

    AWS_LL_COUNT /* Sentinel value */
};

/** Log subjects (categories) */
enum aws_log_subject {
    AWS_LS_COMMON_GENERAL = AWS_LOG_SUBJECT_BEGIN_RANGE(AWS_C_COMMON_PACKAGE_ID),
    AWS_LS_COMMON_TASK_SCHEDULER,
    AWS_LS_COMMON_THREAD,
    AWS_LS_COMMON_MEMTRACE,
    AWS_LS_COMMON_XML_PARSER,
    AWS_LS_COMMON_IO,
    AWS_LS_COMMON_BUS,
    AWS_LS_COMMON_TEST,
    AWS_LS_COMMON_JSON_PARSER,
    AWS_LS_COMMON_CBOR,
    AWS_LS_COMMON_LAST = AWS_LOG_SUBJECT_END_RANGE(AWS_C_COMMON_PACKAGE_ID)
};

/**
 * @brief Structure representing the logging system configuration.
 */
struct aws_logger_options {
    enum aws_log_level level;
    const char *filename; /* NULL for stderr/stdout */
    /* TODO: Add options like max file size, rotation, etc. */
};

/**
 * @brief Initializes the logging system.
 *
 * Must be called before any logging macros are used.
 *
 * @param allocator Allocator for the logging system.
 * @param options Configuration options for the logger.
 * @return AWS_OP_SUCCESS on success, AWS_OP_ERR on failure.
 */
AWS_COMMON_API
int aws_logging_init(struct aws_allocator *allocator, const struct aws_logger_options *options);

/**
 * @brief Cleans up the logging system.
 */
AWS_COMMON_API
void aws_logging_clean_up(void);

/**
 * @brief Sets the logging level dynamically.
 *
 * @param level The new logging level to set.
 */
AWS_COMMON_API
void aws_logging_set_level(enum aws_log_level level);

/**
 * @brief Internal logging function. Use the macros below instead.
 */
AWS_COMMON_API
void aws_log(enum aws_log_level level, const char *tag, const char *format, ...);

/**
 * @brief Internal logging function with va_list. Use the macros below instead.
 */
AWS_COMMON_API
void aws_log_v(enum aws_log_level level, const char *tag, const char *format, va_list args);

/* Logging Macros */
/* Note: The 'subject' parameter should be an enum aws_log_subject value */
#define AWS_LOGF_FATAL(subject, format, ...) aws_log(AWS_LL_FATAL, subject, format, ##__VA_ARGS__)
#define AWS_LOGF_ERROR(subject, format, ...) aws_log(AWS_LL_ERROR, subject, format, ##__VA_ARGS__)
#define AWS_LOGF_WARN(subject, format, ...)  aws_log(AWS_LL_WARN,  subject, format, ##__VA_ARGS__)
#define AWS_LOGF_INFO(subject, format, ...)  aws_log(AWS_LL_INFO,  subject, format, ##__VA_ARGS__)
#define AWS_LOGF_DEBUG(subject, format, ...) aws_log(AWS_LL_DEBUG, subject, format, ##__VA_ARGS__)
#define AWS_LOGF_TRACE(subject, format, ...) aws_log(AWS_LL_TRACE, subject, format, ##__VA_ARGS__)


AWS_EXTERN_C_END

#endif /* AWS_COMMON_LOGGING_H */
