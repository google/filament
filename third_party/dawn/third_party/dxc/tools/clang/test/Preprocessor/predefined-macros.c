// This test verifies that the correct macros are predefined.
//
// RUN: %clang_cc1 %s -E -dM -triple i686-pc-win32 -fms-extensions -fms-compatibility \
// RUN:     -fms-compatibility-version=13.00 -o - | FileCheck %s --check-prefix=CHECK-MS
// CHECK-MS: #define _INTEGRAL_MAX_BITS 64
// CHECK-MS: #define _MSC_EXTENSIONS 1
// CHECK-MS: #define _MSC_VER 1300
// CHECK-MS: #define _M_IX86 600
// CHECK-MS: #define _M_IX86_FP
// CHECK-MS: #define _WIN32 1
// CHECK-MS-NOT: #define __STRICT_ANSI__
// CHECK-MS-NOT: GCC
// CHECK-MS-NOT: GNU
// CHECK-MS-NOT: GXX
//
// RUN: %clang_cc1 %s -E -dM -triple x86_64-pc-win32 -fms-extensions -fms-compatibility \
// RUN:     -fms-compatibility-version=13.00 -o - | FileCheck %s --check-prefix=CHECK-MS64
// CHECK-MS64: #define _INTEGRAL_MAX_BITS 64
// CHECK-MS64: #define _MSC_EXTENSIONS 1
// CHECK-MS64: #define _MSC_VER 1300
// CHECK-MS64: #define _M_AMD64 1
// CHECK-MS64: #define _M_X64 1
// CHECK-MS64: #define _WIN64 1
// CHECK-MS64-NOT: #define __STRICT_ANSI__
// CHECK-MS64-NOT: GCC
// CHECK-MS64-NOT: GNU
// CHECK-MS64-NOT: GXX
//
// RUN: %clang_cc1 %s -E -dM -triple i686-pc-win32 -fms-compatibility \
// RUN:     -o - | FileCheck %s --check-prefix=CHECK-MS-STDINT
// CHECK-MS-STDINT:#define __INT16_MAX__ 32767
// CHECK-MS-STDINT:#define __INT32_MAX__ 2147483647
// CHECK-MS-STDINT:#define __INT64_MAX__ 9223372036854775807LL
// CHECK-MS-STDINT:#define __INT8_MAX__ 127
// CHECK-MS-STDINT:#define __INTPTR_MAX__ 2147483647
// CHECK-MS-STDINT:#define __INT_FAST16_MAX__ 32767
// CHECK-MS-STDINT:#define __INT_FAST16_TYPE__ short
// CHECK-MS-STDINT:#define __INT_FAST32_MAX__ 2147483647
// CHECK-MS-STDINT:#define __INT_FAST32_TYPE__ int
// CHECK-MS-STDINT:#define __INT_FAST64_MAX__ 9223372036854775807LL
// CHECK-MS-STDINT:#define __INT_FAST64_TYPE__ long long int
// CHECK-MS-STDINT:#define __INT_FAST8_MAX__ 127
// CHECK-MS-STDINT:#define __INT_FAST8_TYPE__ signed char
// CHECK-MS-STDINT:#define __INT_LEAST16_MAX__ 32767
// CHECK-MS-STDINT:#define __INT_LEAST16_TYPE__ short
// CHECK-MS-STDINT:#define __INT_LEAST32_MAX__ 2147483647
// CHECK-MS-STDINT:#define __INT_LEAST32_TYPE__ int
// CHECK-MS-STDINT:#define __INT_LEAST64_MAX__ 9223372036854775807LL
// CHECK-MS-STDINT:#define __INT_LEAST64_TYPE__ long long int
// CHECK-MS-STDINT:#define __INT_LEAST8_MAX__ 127
// CHECK-MS-STDINT:#define __INT_LEAST8_TYPE__ signed char
// CHECK-MS-STDINT-NOT:#define __UINT16_C_SUFFIX__ U
// CHECK-MS-STDINT:#define __UINT16_MAX__ 65535
// CHECK-MS-STDINT:#define __UINT16_TYPE__ unsigned short
// CHECK-MS-STDINT:#define __UINT32_C_SUFFIX__ U
// CHECK-MS-STDINT:#define __UINT32_MAX__ 4294967295U
// CHECK-MS-STDINT:#define __UINT32_TYPE__ unsigned int
// CHECK-MS-STDINT:#define __UINT64_C_SUFFIX__ ULL
// CHECK-MS-STDINT:#define __UINT64_MAX__ 18446744073709551615ULL
// CHECK-MS-STDINT:#define __UINT64_TYPE__ long long unsigned int
// CHECK-MS-STDINT-NOT:#define __UINT8_C_SUFFIX__ U
// CHECK-MS-STDINT:#define __UINT8_MAX__ 255
// CHECK-MS-STDINT:#define __UINT8_TYPE__ unsigned char
// CHECK-MS-STDINT:#define __UINTMAX_MAX__ 18446744073709551615ULL
// CHECK-MS-STDINT:#define __UINTPTR_MAX__ 4294967295U
// CHECK-MS-STDINT:#define __UINTPTR_TYPE__ unsigned int
// CHECK-MS-STDINT:#define __UINTPTR_WIDTH__ 32
// CHECK-MS-STDINT:#define __UINT_FAST16_MAX__ 65535
// CHECK-MS-STDINT:#define __UINT_FAST16_TYPE__ unsigned short
// CHECK-MS-STDINT:#define __UINT_FAST32_MAX__ 4294967295U
// CHECK-MS-STDINT:#define __UINT_FAST32_TYPE__ unsigned int
// CHECK-MS-STDINT:#define __UINT_FAST64_MAX__ 18446744073709551615ULL
// CHECK-MS-STDINT:#define __UINT_FAST64_TYPE__ long long unsigned int
// CHECK-MS-STDINT:#define __UINT_FAST8_MAX__ 255
// CHECK-MS-STDINT:#define __UINT_FAST8_TYPE__ unsigned char
// CHECK-MS-STDINT:#define __UINT_LEAST16_MAX__ 65535
// CHECK-MS-STDINT:#define __UINT_LEAST16_TYPE__ unsigned short
// CHECK-MS-STDINT:#define __UINT_LEAST32_MAX__ 4294967295U
// CHECK-MS-STDINT:#define __UINT_LEAST32_TYPE__ unsigned int
// CHECK-MS-STDINT:#define __UINT_LEAST64_MAX__ 18446744073709551615ULL
// CHECK-MS-STDINT:#define __UINT_LEAST64_TYPE__ long long unsigned int
// CHECK-MS-STDINT:#define __UINT_LEAST8_MAX__ 255
// CHECK-MS-STDINT:#define __UINT_LEAST8_TYPE__ unsigned char
//
// RUN: %clang_cc1 %s -E -dM -ffast-math -o - \
// RUN:   | FileCheck %s --check-prefix=CHECK-FAST-MATH
// CHECK-FAST-MATH: #define __FAST_MATH__
// CHECK-FAST-MATH: #define __FINITE_MATH_ONLY__ 1
//
// RUN: %clang_cc1 %s -E -dM -ffinite-math-only -o - \
// RUN:   | FileCheck %s --check-prefix=CHECK-FINITE-MATH-ONLY
// CHECK-FINITE-MATH-ONLY: #define __FINITE_MATH_ONLY__ 1
//
// RUN: %clang %s -E -dM -fno-finite-math-only -o - \
// RUN:   | FileCheck %s --check-prefix=CHECK-NO-FINITE-MATH-ONLY
// CHECK-NO-FINITE-MATH-ONLY: #define __FINITE_MATH_ONLY__ 0
//
// RUN: %clang_cc1 %s -E -dM -o - \
// RUN:   | FileCheck %s --check-prefix=CHECK-FINITE-MATH-FLAG-UNDEFINED
// CHECK-FINITE-MATH-FLAG-UNDEFINED: #define __FINITE_MATH_ONLY__ 0
//
// RUN: %clang_cc1 %s -E -dM -o - -triple i686 -target-cpu i386 \
// RUN:   | FileCheck %s --check-prefix=CHECK-SYNC_CAS_I386
// CHECK-SYNC_CAS_I386-NOT: __GCC_HAVE_SYNC_COMPARE_AND_SWAP
//
// RUN: %clang_cc1 %s -E -dM -o - -triple i686 -target-cpu i486 \
// RUN:   | FileCheck %s --check-prefix=CHECK-SYNC_CAS_I486
// CHECK-SYNC_CAS_I486: __GCC_HAVE_SYNC_COMPARE_AND_SWAP_1
// CHECK-SYNC_CAS_I486: __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2
// CHECK-SYNC_CAS_I486: __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
// CHECK-SYNC_CAS_I486-NOT: __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8
//
// RUN: %clang_cc1 %s -E -dM -o - -triple i686 -target-cpu i586 \
// RUN:   | FileCheck %s --check-prefix=CHECK-SYNC_CAS_I586
// CHECK-SYNC_CAS_I586: __GCC_HAVE_SYNC_COMPARE_AND_SWAP_1
// CHECK-SYNC_CAS_I586: __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2
// CHECK-SYNC_CAS_I586: __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
// CHECK-SYNC_CAS_I586: __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8
//
// RUN: %clang_cc1 %s -E -dM -o - -triple armv6 -target-cpu arm1136j-s \
// RUN:   | FileCheck %s --check-prefix=CHECK-SYNC_CAS_ARM
// CHECK-SYNC_CAS_ARM: __GCC_HAVE_SYNC_COMPARE_AND_SWAP_1
// CHECK-SYNC_CAS_ARM: __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2
// CHECK-SYNC_CAS_ARM: __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
// CHECK-SYNC_CAS_ARM: __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8
//
// RUN: %clang_cc1 %s -E -dM -o - -triple armv7 -target-cpu cortex-a8 \
// RUN:   | FileCheck %s --check-prefix=CHECK-SYNC_CAS_ARMv7
// CHECK-SYNC_CAS_ARMv7: __GCC_HAVE_SYNC_COMPARE_AND_SWAP_1
// CHECK-SYNC_CAS_ARMv7: __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2
// CHECK-SYNC_CAS_ARMv7: __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
// CHECK-SYNC_CAS_ARMv7: __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8
//
// RUN: %clang_cc1 %s -E -dM -o - -triple armv6 -target-cpu cortex-m0 \
// RUN:   | FileCheck %s --check-prefix=CHECK-SYNC_CAS_ARMv6
// CHECK-SYNC_CAS_ARMv6-NOT: __GCC_HAVE_SYNC_COMPARE_AND_SWAP
