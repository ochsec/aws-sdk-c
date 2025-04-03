#ifndef AWS_COMMON_LOG_CHANNEL_H
#define AWS_COMMON_LOG_CHANNEL_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

/**
 * @brief Log channel definitions for the AWS SDK for C.
 * 
 * Each module should define its log channels relative to the last channel
 * of the previous module to avoid conflicts.
 */

/* Common library log channels */
#define AWS_LS_COMMON_GENERAL 0
#define AWS_LS_COMMON_TASK_SCHEDULER 1
#define AWS_LS_COMMON_THREAD 2
#define AWS_LS_COMMON_MEMTRACE 3
#define AWS_LS_COMMON_IO 4
/* Add more common log channels as needed */
#define AWS_LS_COMMON_LAST 99

/* IO library log channels start at 100 */
#define AWS_LS_IO_GENERAL 100
#define AWS_LS_IO_EVENT_LOOP 101
#define AWS_LS_IO_SOCKET 102
#define AWS_LS_IO_SOCKET_HANDLER 103
#define AWS_LS_IO_TLS 104
#define AWS_LS_IO_ALPN 105
#define AWS_LS_IO_DNS 106
#define AWS_LS_IO_PKI 107
#define AWS_LS_IO_CHANNEL 108
#define AWS_LS_IO_CHANNEL_HANDLER 109
#define AWS_LS_IO_LAST 199

/* HTTP library log channels start at 200 */
#define AWS_LS_HTTP_GENERAL 200
#define AWS_LS_HTTP_CONNECTION 201
#define AWS_LS_HTTP_SERVER 202
#define AWS_LS_HTTP_STREAM 203
#define AWS_LS_HTTP_LAST 299

/* Auth library log channels start at 300 */
#define AWS_LS_AUTH_GENERAL 300
#define AWS_LS_AUTH_CREDENTIALS 301
#define AWS_LS_AUTH_SIGNING 302
#define AWS_LS_AUTH_LAST 399

/* S3 library log channels start at 400 */
#define AWS_LS_S3_GENERAL 400
#define AWS_LS_S3_CLIENT 401
#define AWS_LS_S3_LAST 499

#endif /* AWS_COMMON_LOG_CHANNEL_H */
