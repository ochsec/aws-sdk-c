#ifndef AWS_COMMON_LOGGING_H
#define AWS_COMMON_LOGGING_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/exports.h>
#include <aws/common/allocator.h>
#include <stdarg.h> /* For va_list */

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
#define AWS_LOGF_FATAL(tag, format, ...) aws_log(AWS_LL_FATAL, tag, format, ##__VA_ARGS__)
#define AWS_LOGF_ERROR(tag, format, ...) aws_log(AWS_LL_ERROR, tag, format, ##__VA_ARGS__)
#define AWS_LOGF_WARN(tag, format, ...)  aws_log(AWS_LL_WARN,  tag, format, ##__VA_ARGS__)
#define AWS_LOGF_INFO(tag, format, ...)  aws_log(AWS_LL_INFO,  tag, format, ##__VA_ARGS__)
#define AWS_LOGF_DEBUG(tag, format, ...) aws_log(AWS_LL_DEBUG, tag, format, ##__VA_ARGS__)
#define AWS_LOGF_TRACE(tag, format, ...) aws_log(AWS_LL_TRACE, tag, format, ##__VA_ARGS__)


AWS_EXTERN_C_END

#endif /* AWS_COMMON_LOGGING_H */
