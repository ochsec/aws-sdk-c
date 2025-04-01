/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/logging.h>
#include <aws/common/error.h> /* For AWS_OP_SUCCESS/ERR */
#include <stdio.h>  /* For fprintf, vfprintf, stderr */
#include <stdlib.h> /* For NULL */
#include <time.h>   /* For timestamp */

/* Global logger state (simple version) */
static enum aws_log_level s_log_level = AWS_LL_NONE;
/* TODO: Add file handling, mutex for thread safety if writing to file */

static const char *s_log_level_strings[] = {
    [AWS_LL_NONE]  = "NONE",
    [AWS_LL_FATAL] = "FATAL",
    [AWS_LL_ERROR] = "ERROR",
    [AWS_LL_WARN]  = "WARN",
    [AWS_LL_INFO]  = "INFO",
    [AWS_LL_DEBUG] = "DEBUG",
    [AWS_LL_TRACE] = "TRACE",
};

int aws_logging_init(struct aws_allocator *allocator, const struct aws_logger_options *options) {
    (void)allocator; /* Not used in this simple version */

    if (!options) {
        /* Default to INFO level logging to stderr */
        s_log_level = AWS_LL_INFO;
    } else {
        s_log_level = options->level;
        if (options->filename) {
            /* TODO: Implement file logging */
            fprintf(stderr, "File logging not yet implemented.\n");
            return AWS_OP_ERR; /* Indicate error */
        }
    }

    if (s_log_level >= AWS_LL_COUNT) {
        s_log_level = AWS_LL_NONE; /* Invalid level */
        return AWS_OP_ERR;
    }

    AWS_LOGF_INFO("Logging", "Logging initialized to level %s", s_log_level_strings[s_log_level]);
    return AWS_OP_SUCCESS;
}

void aws_logging_clean_up(void) {
    AWS_LOGF_INFO("Logging", "Logging cleaned up.");
    s_log_level = AWS_LL_NONE;
    /* TODO: Close log file if open */
}

void aws_logging_set_level(enum aws_log_level level) {
     if (level < AWS_LL_COUNT) {
         AWS_LOGF_INFO("Logging", "Log level changed from %s to %s",
                       s_log_level_strings[s_log_level], s_log_level_strings[level]);
        s_log_level = level;
    }
}

void aws_log_v(enum aws_log_level level, const char *tag, const char *format, va_list args) {
    if (level == AWS_LL_NONE || level > s_log_level) {
        return; /* Skip logging if level is NONE or below current threshold */
    }

    /* Basic timestamp */
    time_t now;
    time(&now);
    char time_buf[32];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&now)); /* Using localtime for readability */

    /* TODO: Add thread ID */
    /* TODO: Add mutex for thread safety if writing to shared resource (like file or even stderr sometimes) */

    fprintf(stderr, "[%s] [%s] [%s] ", time_buf, s_log_level_strings[level], tag ? tag : "Default");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    fflush(stderr); /* Ensure message is written immediately */
}

void aws_log(enum aws_log_level level, const char *tag, const char *format, ...) {
    va_list args;
    va_start(args, format);
    aws_log_v(level, tag, format, args);
    va_end(args);
}
