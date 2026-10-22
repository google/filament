/*
 * arm/adler32_impl.h - ARM implementations of Adler-32 checksum algorithm
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

#ifndef LIB_ARM_ADLER32_IMPL_H
#define LIB_ARM_ADLER32_IMPL_H

#include "cpu_features.h"

/* Regular NEON implementation */
#if HAVE_NEON_INTRIN && CPU_IS_LITTLE_ENDIAN()
#  define adler32_arm_neon	adler32_arm_neon
#  if HAVE_NEON_NATIVE
     /*
      * Use no attributes if none are needed, to support old versions of clang
      * that don't accept the simd target attribute.
      */
#    define ATTRIBUTES
#  elif defined(ARCH_ARM32)
#    define ATTRIBUTES	_target_attribute("fpu=neon")
#  elif defined(__clang__)
#    define ATTRIBUTES	_target_attribute("simd")
#  else
#    define ATTRIBUTES	_target_attribute("+simd")
#  endif
static ATTRIBUTES MAYBE_UNUSED u32
adler32_arm_neon(u32 adler, const u8 *p, size_t len)
{
	static const u16 _aligned_attribute(16) mults[64] = {
		64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,
		48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,
		32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
		16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,
	};
	const uint16x8_t mults_a = vld1q_u16(&mults[0]);
	const uint16x8_t mults_b = vld1q_u16(&mults[8]);
	const uint16x8_t mults_c = vld1q_u16(&mults[16]);
	const uint16x8_t mults_d = vld1q_u16(&mults[24]);
	const uint16x8_t mults_e = vld1q_u16(&mults[32]);
	const uint16x8_t mults_f = vld1q_u16(&mults[40]);
	const uint16x8_t mults_g = vld1q_u16(&mults[48]);
	const uint16x8_t mults_h = vld1q_u16(&mults[56]);
	u32 s1 = adler & 0xFFFF;
	u32 s2 = adler >> 16;

	/*
	 * If the length is large and the pointer is misaligned, align it.
	 * For smaller lengths, just take the misaligned load penalty.
	 */
	if (unlikely(len > 32768 && ((uintptr_t)p & 15))) {
		do {
			s1 += *p++;
			s2 += s1;
			len--;
		} while ((uintptr_t)p & 15);
		s1 %= DIVISOR;
		s2 %= DIVISOR;
	}

	while (len) {
		/*
		 * Calculate the length of the next data chunk such that s1 and
		 * s2 are guaranteed to not exceed UINT32_MAX.
		 */
		size_t n = MIN(len, MAX_CHUNK_LEN & ~63);

		len -= n;

		if (n >= 64) {
			uint32x4_t v_s1 = vdupq_n_u32(0);
			uint32x4_t v_s2 = vdupq_n_u32(0);
			/*
			 * v_byte_sums_* contain the sum of the bytes at index i
			 * across all 64-byte segments, for each index 0..63.
			 */
			uint16x8_t v_byte_sums_a = vdupq_n_u16(0);
			uint16x8_t v_byte_sums_b = vdupq_n_u16(0);
			uint16x8_t v_byte_sums_c = vdupq_n_u16(0);
			uint16x8_t v_byte_sums_d = vdupq_n_u16(0);
			uint16x8_t v_byte_sums_e = vdupq_n_u16(0);
			uint16x8_t v_byte_sums_f = vdupq_n_u16(0);
			uint16x8_t v_byte_sums_g = vdupq_n_u16(0);
			uint16x8_t v_byte_sums_h = vdupq_n_u16(0);

			s2 += s1 * (n & ~63);

			do {
				/* Load the next 64 data bytes. */
				const uint8x16_t data_a = vld1q_u8(p + 0);
				const uint8x16_t data_b = vld1q_u8(p + 16);
				const uint8x16_t data_c = vld1q_u8(p + 32);
				const uint8x16_t data_d = vld1q_u8(p + 48);
				uint16x8_t tmp;

				/*
				 * Accumulate the previous s1 counters into the
				 * s2 counters.  The needed multiplication by 64
				 * is delayed to later.
				 */
				v_s2 = vaddq_u32(v_s2, v_s1);

				/*
				 * Add the 64 data bytes to their v_byte_sums
				 * counters, while also accumulating the sums of
				 * each adjacent set of 4 bytes into v_s1.
				 */
				tmp = vpaddlq_u8(data_a);
				v_byte_sums_a = vaddw_u8(v_byte_sums_a,
							 vget_low_u8(data_a));
				v_byte_sums_b = vaddw_u8(v_byte_sums_b,
							 vget_high_u8(data_a));
				tmp = vpadalq_u8(tmp, data_b);
				v_byte_sums_c = vaddw_u8(v_byte_sums_c,
							 vget_low_u8(data_b));
				v_byte_sums_d = vaddw_u8(v_byte_sums_d,
							 vget_high_u8(data_b));
				tmp = vpadalq_u8(tmp, data_c);
				v_byte_sums_e = vaddw_u8(v_byte_sums_e,
							 vget_low_u8(data_c));
				v_byte_sums_f = vaddw_u8(v_byte_sums_f,
							 vget_high_u8(data_c));
				tmp = vpadalq_u8(tmp, data_d);
				v_byte_sums_g = vaddw_u8(v_byte_sums_g,
							 vget_low_u8(data_d));
				v_byte_sums_h = vaddw_u8(v_byte_sums_h,
							 vget_high_u8(data_d));
				v_s1 = vpadalq_u16(v_s1, tmp);

				p += 64;
				n -= 64;
			} while (n >= 64);

			/* s2 = 64*s2 + (64*bytesum0 + 63*bytesum1 + ... + 1*bytesum63) */
		#ifdef ARCH_ARM32
		#  define umlal2(a, b, c)  vmlal_u16((a), vget_high_u16(b), vget_high_u16(c))
		#else
		#  define umlal2	   vmlal_high_u16
		#endif
			v_s2 = vqshlq_n_u32(v_s2, 6);
			v_s2 = vmlal_u16(v_s2, vget_low_u16(v_byte_sums_a),
					 vget_low_u16(mults_a));
			v_s2 = umlal2(v_s2, v_byte_sums_a, mults_a);
			v_s2 = vmlal_u16(v_s2, vget_low_u16(v_byte_sums_b),
					 vget_low_u16(mults_b));
			v_s2 = umlal2(v_s2, v_byte_sums_b, mults_b);
			v_s2 = vmlal_u16(v_s2, vget_low_u16(v_byte_sums_c),
					 vget_low_u16(mults_c));
			v_s2 = umlal2(v_s2, v_byte_sums_c, mults_c);
			v_s2 = vmlal_u16(v_s2, vget_low_u16(v_byte_sums_d),
					 vget_low_u16(mults_d));
			v_s2 = umlal2(v_s2, v_byte_sums_d, mults_d);
			v_s2 = vmlal_u16(v_s2, vget_low_u16(v_byte_sums_e),
					 vget_low_u16(mults_e));
			v_s2 = umlal2(v_s2, v_byte_sums_e, mults_e);
			v_s2 = vmlal_u16(v_s2, vget_low_u16(v_byte_sums_f),
					 vget_low_u16(mults_f));
			v_s2 = umlal2(v_s2, v_byte_sums_f, mults_f);
			v_s2 = vmlal_u16(v_s2, vget_low_u16(v_byte_sums_g),
					 vget_low_u16(mults_g));
			v_s2 = umlal2(v_s2, v_byte_sums_g, mults_g);
			v_s2 = vmlal_u16(v_s2, vget_low_u16(v_byte_sums_h),
					 vget_low_u16(mults_h));
			v_s2 = umlal2(v_s2, v_byte_sums_h, mults_h);
		#undef umlal2

			/* Horizontal sum to finish up */
		#ifdef ARCH_ARM32
			s1 += vgetq_lane_u32(v_s1, 0) + vgetq_lane_u32(v_s1, 1) +
			      vgetq_lane_u32(v_s1, 2) + vgetq_lane_u32(v_s1, 3);
			s2 += vgetq_lane_u32(v_s2, 0) + vgetq_lane_u32(v_s2, 1) +
			      vgetq_lane_u32(v_s2, 2) + vgetq_lane_u32(v_s2, 3);
		#else
			s1 += vaddvq_u32(v_s1);
			s2 += vaddvq_u32(v_s2);
		#endif
		}
		/*
		 * Process the last 0 <= n < 64 bytes of the chunk using
		 * scalar instructions and reduce s1 and s2 mod DIVISOR.
		 */
		ADLER32_CHUNK(s1, s2, p, n);
	}
	return (s2 << 16) | s1;
}
#undef ATTRIBUTES
#endif /* Regular NEON implementation */

/* NEON+dotprod implementation */
#if HAVE_DOTPROD_INTRIN && CPU_IS_LITTLE_ENDIAN() && \
	!defined(LIBDEFLATE_ASSEMBLER_DOES_NOT_SUPPORT_DOTPROD)
#  define adler32_arm_neon_dotprod	adler32_arm_neon_dotprod
#  ifdef __clang__
#    define ATTRIBUTES	_target_attribute("dotprod")
   /*
    * Both gcc and binutils originally considered dotprod to depend on
    * arch=armv8.2-a or later.  This was fixed in gcc 13.2 by commit
    * 9aac37ab8a7b ("aarch64: Remove architecture dependencies from intrinsics")
    * and in binutils 2.41 by commit 205e4380c800 ("aarch64: Remove version
    * dependencies from features").  Unfortunately, always using arch=armv8.2-a
    * causes build errors with some compiler options because it may reduce the
    * arch rather than increase it.  Therefore we try to omit the arch whenever
    * possible.  If gcc is 14 or later, then both gcc and binutils are probably
    * fixed, so we omit the arch.  We also omit the arch if a feature that
    * depends on armv8.2-a or later (in gcc 13.1 and earlier) is present.
    */
#  elif GCC_PREREQ(14, 0) || defined(__ARM_FEATURE_JCVT) \
			  || defined(__ARM_FEATURE_DOTPROD)
#    define ATTRIBUTES	_target_attribute("+dotprod")
#  else
#    define ATTRIBUTES	_target_attribute("arch=armv8.2-a+dotprod")
#  endif
static ATTRIBUTES u32
adler32_arm_neon_dotprod(u32 adler, const u8 *p, size_t len)
{
	static const u8 _aligned_attribute(16) mults[64] = {
		64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,
		48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,
		32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
		16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,
	};
	const uint8x16_t mults_a = vld1q_u8(&mults[0]);
	const uint8x16_t mults_b = vld1q_u8(&mults[16]);
	const uint8x16_t mults_c = vld1q_u8(&mults[32]);
	const uint8x16_t mults_d = vld1q_u8(&mults[48]);
	const uint8x16_t ones = vdupq_n_u8(1);
	u32 s1 = adler & 0xFFFF;
	u32 s2 = adler >> 16;

	/*
	 * If the length is large and the pointer is misaligned, align it.
	 * For smaller lengths, just take the misaligned load penalty.
	 */
	if (unlikely(len > 32768 && ((uintptr_t)p & 15))) {
		do {
			s1 += *p++;
			s2 += s1;
			len--;
		} while ((uintptr_t)p & 15);
		s1 %= DIVISOR;
		s2 %= DIVISOR;
	}

	while (len) {
		/*
		 * Calculate the length of the next data chunk such that s1 and
		 * s2 are guaranteed to not exceed UINT32_MAX.
		 */
		size_t n = MIN(len, MAX_CHUNK_LEN & ~63);

		len -= n;

		if (n >= 64) {
			uint32x4_t v_s1_a = vdupq_n_u32(0);
			uint32x4_t v_s1_b = vdupq_n_u32(0);
			uint32x4_t v_s1_c = vdupq_n_u32(0);
			uint32x4_t v_s1_d = vdupq_n_u32(0);
			uint32x4_t v_s2_a = vdupq_n_u32(0);
			uint32x4_t v_s2_b = vdupq_n_u32(0);
			uint32x4_t v_s2_c = vdupq_n_u32(0);
			uint32x4_t v_s2_d = vdupq_n_u32(0);
			uint32x4_t v_s1_sums_a = vdupq_n_u32(0);
			uint32x4_t v_s1_sums_b = vdupq_n_u32(0);
			uint32x4_t v_s1_sums_c = vdupq_n_u32(0);
			uint32x4_t v_s1_sums_d = vdupq_n_u32(0);
			uint32x4_t v_s1;
			uint32x4_t v_s2;
			uint32x4_t v_s1_sums;

			s2 += s1 * (n & ~63);

			do {
				uint8x16_t data_a = vld1q_u8(p + 0);
				uint8x16_t data_b = vld1q_u8(p + 16);
				uint8x16_t data_c = vld1q_u8(p + 32);
				uint8x16_t data_d = vld1q_u8(p + 48);

				v_s1_sums_a = vaddq_u32(v_s1_sums_a, v_s1_a);
				v_s1_a = vdotq_u32(v_s1_a, data_a, ones);
				v_s2_a = vdotq_u32(v_s2_a, data_a, mults_a);

				v_s1_sums_b = vaddq_u32(v_s1_sums_b, v_s1_b);
				v_s1_b = vdotq_u32(v_s1_b, data_b, ones);
				v_s2_b = vdotq_u32(v_s2_b, data_b, mults_b);

				v_s1_sums_c = vaddq_u32(v_s1_sums_c, v_s1_c);
				v_s1_c = vdotq_u32(v_s1_c, data_c, ones);
				v_s2_c = vdotq_u32(v_s2_c, data_c, mults_c);

				v_s1_sums_d = vaddq_u32(v_s1_sums_d, v_s1_d);
				v_s1_d = vdotq_u32(v_s1_d, data_d, ones);
				v_s2_d = vdotq_u32(v_s2_d, data_d, mults_d);

				p += 64;
				n -= 64;
			} while (n >= 64);

			v_s1 = vaddq_u32(vaddq_u32(v_s1_a, v_s1_b),
					 vaddq_u32(v_s1_c, v_s1_d));
			v_s2 = vaddq_u32(vaddq_u32(v_s2_a, v_s2_b),
					 vaddq_u32(v_s2_c, v_s2_d));
			v_s1_sums = vaddq_u32(vaddq_u32(v_s1_sums_a,
							v_s1_sums_b),
					      vaddq_u32(v_s1_sums_c,
							v_s1_sums_d));
			v_s2 = vaddq_u32(v_s2, vqshlq_n_u32(v_s1_sums, 6));

			s1 += vaddvq_u32(v_s1);
			s2 += vaddvq_u32(v_s2);
		}
		/*
		 * Process the last 0 <= n < 64 bytes of the chunk using
		 * scalar instructions and reduce s1 and s2 mod DIVISOR.
		 */
		ADLER32_CHUNK(s1, s2, p, n);
	}
	return (s2 << 16) | s1;
}
#undef ATTRIBUTES
#endif /* NEON+dotprod implementation */

#if defined(adler32_arm_neon_dotprod) && defined(__ARM_FEATURE_DOTPROD)
#define DEFAULT_IMPL	adler32_arm_neon_dotprod
#else
static inline adler32_func_t
arch_select_adler32_func(void)
{
	const u32 features MAYBE_UNUSED = get_arm_cpu_features();

#ifdef adler32_arm_neon_dotprod
	if (HAVE_NEON(features) && HAVE_DOTPROD(features))
		return adler32_arm_neon_dotprod;
#endif
#ifdef adler32_arm_neon
	if (HAVE_NEON(features))
		return adler32_arm_neon;
#endif
	return NULL;
}
#define arch_select_adler32_func	arch_select_adler32_func
#endif

#endif /* LIB_ARM_ADLER32_IMPL_H */
