/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/io/io.h>
#include <aws/common/error.h>
#include <stdbool.h>

/* Simple error strings for IO library error codes */
static const struct aws_error_info s_io_errors[] = {
    {
        NULL, /* error_str_fn */
        NULL, /* debug_str_fn */
        "AWS_ERROR_STREAM_READ_FAILED",
        "Stream read operation failed"
    },
    {
        NULL, /* error_str_fn */
        NULL, /* debug_str_fn */
        "AWS_ERROR_STREAM_UNSEEKABLE",
        "Stream does not support seeking"
    },
    {
        NULL, /* error_str_fn */
        NULL, /* debug_str_fn */
        "AWS_ERROR_STREAM_UNKNOWN_LENGTH",
        "Stream length is unknown"
    },
    {
        NULL, /* error_str_fn */
        NULL, /* debug_str_fn */
        "AWS_ERROR_STREAM_SEEK_FAILED",
        "Stream seek operation failed"
    }
    /* Add more IO error definitions as needed */
};

/* Error info list for IO library */
static struct aws_error_info_list s_io_error_list = {
    .error_list = s_io_errors,
    .count = sizeof(s_io_errors) / sizeof(s_io_errors[0]),
};

/* Static initialization flag */
static bool s_io_library_initialized = false;

/**
 * Initialize the aws-c-io library.
 * Registers error codes and other resources.
 */
void aws_io_library_init(struct aws_allocator *allocator) {
    (void)allocator; /* Currently unused, but kept for API consistency */

    if (!s_io_library_initialized) {
        aws_register_error_info(&s_io_error_list);
        s_io_library_initialized = true;
    }
}

/**
 * Clean up the aws-c-io library.
 * Unregisters error codes and frees resources.
 */
void aws_io_library_clean_up(void) {
    if (s_io_library_initialized) {
        aws_unregister_error_info(&s_io_error_list);
        s_io_library_initialized = false;
    }
}
