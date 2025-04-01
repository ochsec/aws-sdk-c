#ifndef AWS_COMMON_THREAD_LOCAL_H
#define AWS_COMMON_THREAD_LOCAL_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/exports.h>

/*
 * Platform-specific definitions for thread-local storage.
 */

#if defined(_MSC_VER)
    /* Microsoft Visual C++ */
    #define AWS_THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__)
    /* GCC, Clang */
    #define AWS_THREAD_LOCAL __thread
#elif __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
    /* C11 standard */
    #include <threads.h>
    #define AWS_THREAD_LOCAL _Thread_local // or tss_t for more complex scenarios
#else
    /* Fallback or other compilers - requires platform-specific implementation */
    #warning "Thread-local storage not fully supported on this platform/compiler. Using static."
    #define AWS_THREAD_LOCAL static /* This is NOT thread-local, just a fallback */
    /* Consider using pthreads (tss_create, tss_get, tss_set) for POSIX */
#endif


#endif /* AWS_COMMON_THREAD_LOCAL_H */
