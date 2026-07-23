/*
 * arm/cpu_features.h - feature detection for ARM CPUs
 *
 * Copyright 2018 Eric Biggers
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef LIB_ARM_CPU_FEATURES_H
#define LIB_ARM_CPU_FEATURES_H

#include "../lib_common.h"

#if defined(ARCH_ARM32) || defined(ARCH_ARM64)

#define ARM_CPU_FEATURE_NEON		(1 << 0)
#define ARM_CPU_FEATURE_PMULL		(1 << 1)
/*
 * PREFER_PMULL indicates that the CPU has very high pmull throughput, and so
 * the 12x wide pmull-based CRC-32 implementation is likely to be faster than an
 * implementation based on the crc32 instructions.
 */
#define ARM_CPU_FEATURE_PREFER_PMULL	(1 << 2)
#define ARM_CPU_FEATURE_CRC32		(1 << 3)
#define ARM_CPU_FEATURE_SHA3		(1 << 4)
#define ARM_CPU_FEATURE_DOTPROD		(1 << 5)

#if !defined(FREESTANDING) && \
    (defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)) && \
    (defined(__linux__) || \
     (defined(__APPLE__) && defined(ARCH_ARM64)) || \
     (defined(_WIN32) && defined(ARCH_ARM64)))
/* Runtime ARM CPU feature detection is supported. */
#  define ARM_CPU_FEATURES_KNOWN	(1U << 31)
extern volatile u32 libdeflate_arm_cpu_features;

void libdeflate_init_arm_cpu_features(void);

static inline u32 get_arm_cpu_features(void)
{
	if (libdeflate_arm_cpu_features == 0)
		libdeflate_init_arm_cpu_features();
	return libdeflate_arm_cpu_features;
}
#else
static inline u32 get_arm_cpu_features(void) { return 0; }
#endif

/* NEON */
#if defined(__ARM_NEON) || (defined(_MSC_VER) && defined(ARCH_ARM64))
#  define HAVE_NEON(features)	1
#  define HAVE_NEON_NATIVE	1
#else
#  define HAVE_NEON(features)	((features) & ARM_CPU_FEATURE_NEON)
#  define HAVE_NEON_NATIVE	0
#endif
/*
 * With both gcc and clang, NEON intrinsics require that the main target has
 * NEON enabled already.  Exception: with gcc 6.1 and later (r230411 for arm32,
 * r226563 for arm64), hardware floating point support is sufficient.
 */
#if (defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)) && \
	(HAVE_NEON_NATIVE || (GCC_PREREQ(6, 1) && defined(__ARM_FP)))
#  define HAVE_NEON_INTRIN	1
#  include <arm_neon.h>
#else
#  define HAVE_NEON_INTRIN	0
#endif

/* PMULL */
#ifdef __ARM_FEATURE_CRYPTO
#  define HAVE_PMULL(features)	1
#else
#  define HAVE_PMULL(features)	((features) & ARM_CPU_FEATURE_PMULL)
#endif
#if defined(ARCH_ARM64) && HAVE_NEON_INTRIN && \
	(GCC_PREREQ(7, 1) || defined(__clang__) || defined(_MSC_VER)) && \
	CPU_IS_LITTLE_ENDIAN() /* untested on big endian */
#  define HAVE_PMULL_INTRIN	1
   /* Work around MSVC's vmull_p64() taking poly64x1_t instead of poly64_t */
#  ifdef _MSC_VER
#    define compat_vmull_p64(a, b)  vmull_p64(vcreate_p64(a), vcreate_p64(b))
#  else
#    define compat_vmull_p64(a, b)  vmull_p64((a), (b))
#  endif
#else
#  define HAVE_PMULL_INTRIN	0
#endif

/* CRC32 */
#ifdef __ARM_FEATURE_CRC32
#  define HAVE_CRC32(features)	1
#else
#  define HAVE_CRC32(features)	((features) & ARM_CPU_FEATURE_CRC32)
#endif
#if defined(ARCH_ARM64) && \
	(defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER))
#  define HAVE_CRC32_INTRIN	1
#  if defined(__GNUC__) || defined(__clang__)
#    include <arm_acle.h>
#  endif
   /*
    * Use an inline assembly fallback for clang 15 and earlier, which only
    * defined the crc32 intrinsics when crc32 is enabled in the main target.
    */
#  if defined(__clang__) && !CLANG_PREREQ(16, 0, 16000000) && \
	!defined(__ARM_FEATURE_CRC32)
#    undef __crc32b
#    define __crc32b(a, b)					\
	({ uint32_t res;					\
	   __asm__("crc32b %w0, %w1, %w2"			\
		   : "=r" (res) : "r" (a), "r" (b));		\
	   res; })
#    undef __crc32h
#    define __crc32h(a, b)					\
	({ uint32_t res;					\
	   __asm__("crc32h %w0, %w1, %w2"			\
		   : "=r" (res) : "r" (a), "r" (b));		\
	   res; })
#    undef __crc32w
#    define __crc32w(a, b)					\
	({ uint32_t res;					\
	   __asm__("crc32w %w0, %w1, %w2"			\
		   : "=r" (res) : "r" (a), "r" (b));		\
	   res; })
#    undef __crc32d
#    define __crc32d(a, b)					\
	({ uint32_t res;					\
	   __asm__("crc32x %w0, %w1, %2"			\
		   : "=r" (res) : "r" (a), "r" (b));		\
	   res; })
#    pragma clang diagnostic ignored "-Wgnu-statement-expression"
#  endif
#else
#  define HAVE_CRC32_INTRIN	0
#endif

/* SHA3 (needed for the eor3 instruction) */
#ifdef __ARM_FEATURE_SHA3
#  define HAVE_SHA3(features)	1
#else
#  define HAVE_SHA3(features)	((features) & ARM_CPU_FEATURE_SHA3)
#endif
#if defined(ARCH_ARM64) && HAVE_NEON_INTRIN && \
	(GCC_PREREQ(9, 1) /* r268049 */ || \
	 CLANG_PREREQ(7, 0, 10010463) /* r338010 */)
#  define HAVE_SHA3_INTRIN	1
   /*
    * Use an inline assembly fallback for clang 15 and earlier, which only
    * defined the sha3 intrinsics when sha3 is enabled in the main target.
    */
#  if defined(__clang__) && !CLANG_PREREQ(16, 0, 16000000) && \
	!defined(__ARM_FEATURE_SHA3)
#    undef veor3q_u8
#    define veor3q_u8(a, b, c)					\
	({ uint8x16_t res;					\
	   __asm__("eor3 %0.16b, %1.16b, %2.16b, %3.16b"	\
		   : "=w" (res) : "w" (a), "w" (b), "w" (c));	\
	   res; })
#    pragma clang diagnostic ignored "-Wgnu-statement-expression"
#  endif
#else
#  define HAVE_SHA3_INTRIN	0
#endif

/* dotprod */
#ifdef __ARM_FEATURE_DOTPROD
#  define HAVE_DOTPROD(features)	1
#else
#  define HAVE_DOTPROD(features)	((features) & ARM_CPU_FEATURE_DOTPROD)
#endif
#if defined(ARCH_ARM64) && HAVE_NEON_INTRIN && \
	(GCC_PREREQ(8, 1) || CLANG_PREREQ(7, 0, 10010000) || defined(_MSC_VER))
#  define HAVE_DOTPROD_INTRIN	1
   /*
    * Use an inline assembly fallback for clang 15 and earlier, which only
    * defined the dotprod intrinsics when dotprod is enabled in the main target.
    */
#  if defined(__clang__) && !CLANG_PREREQ(16, 0, 16000000) && \
	!defined(__ARM_FEATURE_DOTPROD)
#    undef vdotq_u32
#    define vdotq_u32(a, b, c)					\
	({ uint32x4_t res = (a);				\
	   __asm__("udot %0.4s, %1.16b, %2.16b"			\
		   : "+w" (res) : "w" (b), "w" (c));		\
	   res; })
#    pragma clang diagnostic ignored "-Wgnu-statement-expression"
#  endif
#else
#  define HAVE_DOTPROD_INTRIN	0
#endif

#endif /* ARCH_ARM32 || ARCH_ARM64 */

#endif /* LIB_ARM_CPU_FEATURES_H */
