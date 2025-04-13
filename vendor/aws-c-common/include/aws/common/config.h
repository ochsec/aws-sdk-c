/* Manually generated configuration for cross-compilation */
#ifndef AWS_COMMON_CONFIG_H
#define AWS_COMMON_CONFIG_H

/* System Architecture */
#define AWS_ARCH_ARM64 1
#define AWS_ARCH_64BIT 1

/* Compiler Capabilities */
#define AWS_HAVE_GCC_INLINE_ASM 1
#define AWS_HAVE_LINUX_IF_LINK_H 1

/* Standard Headers */
#define HAVE_STDINT_H 1
#define HAVE_STDBOOL_H 1
#define HAVE_SYSCONF 1

/* Threading Support */
#define CMAKE_HAVE_LIBC_PTHREAD 1
#define HAVE_PTHREAD_H 1
#define _POSIX_THREADS 1
#define _POSIX_THREAD_SAFE_FUNCTIONS 1

/* Disable Problematic Features */
#define AWS_HAVE_AVX2_INTRINSICS 0
#define AWS_HAVE_AVX512_INTRINSICS 0
#define AWS_HAVE_MM256_EXTRACT_EPI64 0
#define AWS_HAVE_CLMUL 0
#define AWS_HAVE_ARM32_CRC 0
#define AWS_HAVE_ARMv8_1 0

/* File Offset Configuration */
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE64_SOURCE 1

/* Disable Specific Warnings */
#define AWS_DISABLE_IMPLICIT_CONSTRUCTOR_BITFIELDS 1

#endif /* AWS_COMMON_CONFIG_H */