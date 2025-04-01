#ifndef AWS_COMMON_EXPORTS_H
#define AWS_COMMON_EXPORTS_H

/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

/*
 * Defines macros for symbol visibility and C linkage.
 * These are placeholders and will be properly defined by the build system (CMake).
 */

#ifdef __cplusplus
# define AWS_EXTERN_C_BEGIN extern "C" {
# define AWS_EXTERN_C_END }
#else
# define AWS_EXTERN_C_BEGIN
# define AWS_EXTERN_C_END
#endif /* __cplusplus */

/* For now, define API visibility as empty. CMake will handle this later. */
#define AWS_COMMON_API

#endif /* AWS_COMMON_EXPORTS_H */
