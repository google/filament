/*
 * x86/adler32_impl.h - x86 implementations of Adler-32 checksum algorithm
 *
 * Copyright 2016 Eric Biggers
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

#ifndef LIB_X86_ADLER32_IMPL_H
#define LIB_X86_ADLER32_IMPL_H

#include "cpu_features.h"

/* SSE2 and AVX2 implementations.  Used on older CPUs. */
#if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
#  define adler32_x86_sse2	adler32_x86_sse2
#  define SUFFIX			   _sse2
#  define ATTRIBUTES		_target_attribute("sse2")
#  define VL			16
#  define USE_VNNI		0
#  define USE_AVX512		0
#  include "adler32_template.h"

#  define adler32_x86_avx2	adler32_x86_avx2
#  define SUFFIX			   _avx2
#  define ATTRIBUTES		_target_attribute("avx2")
#  define VL			32
#  define USE_VNNI		0
#  define USE_AVX512		0
#  include "adler32_template.h"
#endif

/*
 * AVX-VNNI implementation.  This is used on CPUs that have AVX2 and AVX-VNNI
 * but don't have AVX-512, for example Intel Alder Lake.
 *
 * Unusually for a new CPU feature, gcc added support for the AVX-VNNI
 * intrinsics (in gcc 11.1) slightly before binutils added support for
 * assembling AVX-VNNI instructions (in binutils 2.36).  Distros can reasonably
 * have gcc 11 with binutils 2.35.  Because of this issue, we check for gcc 12
 * instead of gcc 11.  (libdeflate supports direct compilation without a
 * configure step, so checking the binutils version is not always an option.)
 */
#if (GCC_PREREQ(12, 1) || CLANG_PREREQ(12, 0, 13000000) || MSVC_PREREQ(1930)) && \
	!defined(LIBDEFLATE_ASSEMBLER_DOES_NOT_SUPPORT_AVX_VNNI)
#  define adler32_x86_avx2_vnni	adler32_x86_avx2_vnni
#  define SUFFIX			   _avx2_vnni
#  define ATTRIBUTES		_target_attribute("avx2,avxvnni")
#  define VL			32
#  define USE_VNNI		1
#  define USE_AVX512		0
#  include "adler32_template.h"
#endif

#if (GCC_PREREQ(8, 1) || CLANG_PREREQ(6, 0, 10000000) || MSVC_PREREQ(1920)) && \
	!defined(LIBDEFLATE_ASSEMBLER_DOES_NOT_SUPPORT_AVX512VNNI)
/*
 * AVX512VNNI implementation using 256-bit vectors.  This is very similar to the
 * AVX-VNNI implementation but takes advantage of masking and more registers.
 * This is used on certain older Intel CPUs, specifically Ice Lake and Tiger
 * Lake, which support AVX512VNNI but downclock a bit too eagerly when ZMM
 * registers are used.
 */
#  define adler32_x86_avx512_vl256_vnni	adler32_x86_avx512_vl256_vnni
#  define SUFFIX				   _avx512_vl256_vnni
#  define ATTRIBUTES		_target_attribute("avx512bw,avx512vl,avx512vnni")
#  define VL			32
#  define USE_VNNI		1
#  define USE_AVX512		1
#  include "adler32_template.h"

/*
 * AVX512VNNI implementation using 512-bit vectors.  This is used on CPUs that
 * have a good AVX-512 implementation including AVX512VNNI.
 */
#  define adler32_x86_avx512_vl512_vnni	adler32_x86_avx512_vl512_vnni
#  define SUFFIX				   _avx512_vl512_vnni
#  define ATTRIBUTES		_target_attribute("avx512bw,avx512vnni")
#  define VL			64
#  define USE_VNNI		1
#  define USE_AVX512		1
#  include "adler32_template.h"
#endif

static inline adler32_func_t
arch_select_adler32_func(void)
{
	const u32 features MAYBE_UNUSED = get_x86_cpu_features();

#ifdef adler32_x86_avx512_vl512_vnni
	if ((features & X86_CPU_FEATURE_ZMM) &&
	    HAVE_AVX512BW(features) && HAVE_AVX512VNNI(features))
		return adler32_x86_avx512_vl512_vnni;
#endif
#ifdef adler32_x86_avx512_vl256_vnni
	if (HAVE_AVX512BW(features) && HAVE_AVX512VL(features) &&
	    HAVE_AVX512VNNI(features))
		return adler32_x86_avx512_vl256_vnni;
#endif
#ifdef adler32_x86_avx2_vnni
	if (HAVE_AVX2(features) && HAVE_AVXVNNI(features))
		return adler32_x86_avx2_vnni;
#endif
#ifdef adler32_x86_avx2
	if (HAVE_AVX2(features))
		return adler32_x86_avx2;
#endif
#ifdef adler32_x86_sse2
	if (HAVE_SSE2(features))
		return adler32_x86_sse2;
#endif
	return NULL;
}
#define arch_select_adler32_func	arch_select_adler32_func

#endif /* LIB_X86_ADLER32_IMPL_H */
