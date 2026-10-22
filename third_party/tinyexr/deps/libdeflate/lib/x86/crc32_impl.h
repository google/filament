/*
 * x86/crc32_impl.h - x86 implementations of the gzip CRC-32 algorithm
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

#ifndef LIB_X86_CRC32_IMPL_H
#define LIB_X86_CRC32_IMPL_H

#include "cpu_features.h"

/*
 * pshufb(x, shift_tab[len..len+15]) left shifts x by 16-len bytes.
 * pshufb(x, shift_tab[len+16..len+31]) right shifts x by len bytes.
 */
static const u8 MAYBE_UNUSED shift_tab[48] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

#if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
/*
 * PCLMULQDQ implementation.  This targets PCLMULQDQ+SSE4.1, since in practice
 * all CPUs that support PCLMULQDQ also support SSE4.1.
 */
#  define crc32_x86_pclmulqdq	crc32_x86_pclmulqdq
#  define SUFFIX			 _pclmulqdq
#  define ATTRIBUTES		_target_attribute("pclmul,sse4.1")
#  define VL			16
#  define USE_AVX512		0
#  include "crc32_pclmul_template.h"

/*
 * PCLMULQDQ/AVX implementation.  Same as above, but this is compiled with AVX
 * enabled so that the compiler can generate VEX-coded instructions which can be
 * slightly more efficient.  It still uses 128-bit vectors.
 */
#  define crc32_x86_pclmulqdq_avx	crc32_x86_pclmulqdq_avx
#  define SUFFIX				 _pclmulqdq_avx
#  define ATTRIBUTES		_target_attribute("pclmul,avx")
#  define VL			16
#  define USE_AVX512		0
#  include "crc32_pclmul_template.h"
#endif

/*
 * VPCLMULQDQ/AVX2 implementation.  This is used on CPUs that have AVX2 and
 * VPCLMULQDQ but don't have AVX-512, for example Intel Alder Lake.
 *
 * Currently this can't be enabled with MSVC because MSVC has a bug where it
 * incorrectly assumes that VPCLMULQDQ implies AVX-512:
 * https://developercommunity.visualstudio.com/t/Compiler-incorrectly-assumes-VAES-and-VP/10578785
 *
 * gcc 8.1 and 8.2 had a similar bug where they assumed that
 * _mm256_clmulepi64_epi128() always needed AVX512.  It's fixed in gcc 8.3.
 *
 * _mm256_zextsi128_si256() requires gcc 10.
 */
#if (GCC_PREREQ(10, 1) || CLANG_PREREQ(6, 0, 10000000)) && \
	!defined(LIBDEFLATE_ASSEMBLER_DOES_NOT_SUPPORT_VPCLMULQDQ)
#  define crc32_x86_vpclmulqdq_avx2	crc32_x86_vpclmulqdq_avx2
#  define SUFFIX				 _vpclmulqdq_avx2
#  define ATTRIBUTES		_target_attribute("vpclmulqdq,pclmul,avx2")
#  define VL			32
#  define USE_AVX512		0
#  include "crc32_pclmul_template.h"
#endif

#if (GCC_PREREQ(10, 1) || CLANG_PREREQ(6, 0, 10000000) || MSVC_PREREQ(1920)) && \
	!defined(LIBDEFLATE_ASSEMBLER_DOES_NOT_SUPPORT_VPCLMULQDQ)
/*
 * VPCLMULQDQ/AVX512 implementation using 256-bit vectors.  This is very similar
 * to the VPCLMULQDQ/AVX2 implementation but takes advantage of the vpternlog
 * instruction and more registers.  This is used on certain older Intel CPUs,
 * specifically Ice Lake and Tiger Lake, which support VPCLMULQDQ and AVX512 but
 * downclock a bit too eagerly when ZMM registers are used.
 *
 * _mm256_zextsi128_si256() requires gcc 10.
 */
#  define crc32_x86_vpclmulqdq_avx512_vl256  crc32_x86_vpclmulqdq_avx512_vl256
#  define SUFFIX				      _vpclmulqdq_avx512_vl256
#  define ATTRIBUTES		_target_attribute("vpclmulqdq,pclmul,avx512bw,avx512vl")
#  define VL			32
#  define USE_AVX512		1
#  include "crc32_pclmul_template.h"

/*
 * VPCLMULQDQ/AVX512 implementation using 512-bit vectors.  This is used on CPUs
 * that have a good AVX-512 implementation including VPCLMULQDQ.
 *
 * _mm512_zextsi128_si512() requires gcc 10.
 */
#  define crc32_x86_vpclmulqdq_avx512_vl512  crc32_x86_vpclmulqdq_avx512_vl512
#  define SUFFIX				      _vpclmulqdq_avx512_vl512
#  define ATTRIBUTES		_target_attribute("vpclmulqdq,pclmul,avx512bw,avx512vl")
#  define VL			64
#  define USE_AVX512		1
#  include "crc32_pclmul_template.h"
#endif

static inline crc32_func_t
arch_select_crc32_func(void)
{
	const u32 features MAYBE_UNUSED = get_x86_cpu_features();

#ifdef crc32_x86_vpclmulqdq_avx512_vl512
	if ((features & X86_CPU_FEATURE_ZMM) &&
	    HAVE_VPCLMULQDQ(features) && HAVE_PCLMULQDQ(features) &&
	    HAVE_AVX512BW(features) && HAVE_AVX512VL(features))
		return crc32_x86_vpclmulqdq_avx512_vl512;
#endif
#ifdef crc32_x86_vpclmulqdq_avx512_vl256
	if (HAVE_VPCLMULQDQ(features) && HAVE_PCLMULQDQ(features) &&
	    HAVE_AVX512BW(features) && HAVE_AVX512VL(features))
		return crc32_x86_vpclmulqdq_avx512_vl256;
#endif
#ifdef crc32_x86_vpclmulqdq_avx2
	if (HAVE_VPCLMULQDQ(features) && HAVE_PCLMULQDQ(features) &&
	    HAVE_AVX2(features))
		return crc32_x86_vpclmulqdq_avx2;
#endif
#ifdef crc32_x86_pclmulqdq_avx
	if (HAVE_PCLMULQDQ(features) && HAVE_AVX(features))
		return crc32_x86_pclmulqdq_avx;
#endif
#ifdef crc32_x86_pclmulqdq
	if (HAVE_PCLMULQDQ(features))
		return crc32_x86_pclmulqdq;
#endif
	return NULL;
}
#define arch_select_crc32_func	arch_select_crc32_func

#endif /* LIB_X86_CRC32_IMPL_H */
