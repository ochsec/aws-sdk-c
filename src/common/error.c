/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/error.h>
#include <aws/common/thread_local.h> // We'll need to create this for thread-local storage

#include <stdio.h> // For snprintf if needed for debug strings
#include <stdlib.h> // For NULL
#include <string.h> // For strcmp

/* Using a simple thread-local variable for the last error */
AWS_THREAD_LOCAL int s_last_error = AWS_ERROR_SUCCESS;

/* TODO: Implement a more robust error info registration system */
/* For now, just basic handling */
static const struct aws_error_info s_common_error_info[] = {
    {NULL, NULL, "AWS_ERROR_SUCCESS", "No error"},
    {NULL, NULL, "AWS_ERROR_OOM", "Out of memory"},
    {NULL, NULL, "AWS_ERROR_UNKNOWN", "Unknown error"},
    {NULL, NULL, "AWS_ERROR_INVALID_ARGUMENT", "Invalid argument"},
};

static const struct aws_error_info_list s_common_error_list = {
    .error_list = s_common_error_info,
    .count = sizeof(s_common_error_info) / sizeof(struct aws_error_info),
};

/* Placeholder for registered error info lists */
#define MAX_REGISTERED_ERROR_LISTS 16
static const struct aws_error_info_list *s_registered_error_lists[MAX_REGISTERED_ERROR_LISTS] = {&s_common_error_list};
static int s_registered_error_list_count = 1; // Start with common errors registered

int aws_last_error(void) {
    return s_last_error;
}

void aws_raise_error(int error_code) {
    s_last_error = error_code;
}

const char *aws_error_str(int error_code) {
    if (error_code == AWS_ERROR_SUCCESS) {
        return "Success";
    }

    for (int i = 0; i < s_registered_error_list_count; ++i) {
        const struct aws_error_info_list *list = s_registered_error_lists[i];
        if (list) {
            for (uint16_t j = 0; j < list->count; ++j) {
                // Assuming error codes within a list are sequential starting from the base
                int base_error_code = list->error_list[0].literal_name ? // Hacky way to get base, needs improvement
                                      (strcmp(list->error_list[0].literal_name, "AWS_ERROR_SUCCESS") == 0 ? 0 : AWS_C_COMMON_ERROR_CODE_BEGIN + 1) // Adjust based on actual base
                                      : 0; // Default or handle error
                 if (base_error_code + j == error_code) {
                     if (list->error_list[j].error_str_fn) {
                         return list->error_list[j].error_str_fn(error_code);
                     }
                     return list->error_list[j].description ? list->error_list[j].description : "No description available";
                 }
            }
        }
    }

    return "Unknown Error Code";
}

const char *aws_error_debug_str(void) {
    int error_code = aws_last_error();
     if (error_code == AWS_ERROR_SUCCESS) {
        return "No error";
    }

    for (int i = 0; i < s_registered_error_list_count; ++i) {
        const struct aws_error_info_list *list = s_registered_error_lists[i];
        if (list) {
             for (uint16_t j = 0; j < list->count; ++j) {
                 // Assuming error codes within a list are sequential starting from the base
                 int base_error_code = list->error_list[0].literal_name ? // Hacky way to get base, needs improvement
                                       (strcmp(list->error_list[0].literal_name, "AWS_ERROR_SUCCESS") == 0 ? 0 : AWS_C_COMMON_ERROR_CODE_BEGIN + 1) // Adjust based on actual base
                                       : 0; // Default or handle error
                 if (base_error_code + j == error_code) {
                     if (list->error_list[j].debug_str_fn) {
                         return list->error_list[j].debug_str_fn(error_code);
                     }
                     // Maybe return literal name if no debug string?
                     return list->error_list[j].literal_name ? list->error_list[j].literal_name : "No debug string available";
                 }
             }
        }
    }
    // TODO: Provide a buffer for unknown error codes?
    return "Unknown Error Code (debug)";
}


void aws_register_error_info(const struct aws_error_info_list *error_info_list) {
    if (s_registered_error_list_count < MAX_REGISTERED_ERROR_LISTS) {
        s_registered_error_lists[s_registered_error_list_count++] = error_info_list;
    } else {
        // Handle error: too many lists registered
        aws_raise_error(AWS_ERROR_OOM); // Or a specific error for this case
    }
}

void aws_unregister_error_info(const struct aws_error_info_list *error_info_list) {
     for (int i = 0; i < s_registered_error_list_count; ++i) {
         if (s_registered_error_lists[i] == error_info_list) {
             // Shift remaining elements down
             for (int j = i; j < s_registered_error_list_count - 1; ++j) {
                 s_registered_error_lists[j] = s_registered_error_lists[j + 1];
             }
             s_registered_error_list_count--;
             s_registered_error_lists[s_registered_error_list_count] = NULL; // Clear last slot
             return;
         }
     }
     // Handle error: list not found?
}
