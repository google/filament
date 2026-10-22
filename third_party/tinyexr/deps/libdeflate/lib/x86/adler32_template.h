/*
 * x86/adler32_template.h - template for vectorized Adler-32 implementations
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

/*
 * This file is a "template" for instantiating Adler-32 functions for x86.
 * The "parameters" are:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.  Must satisfy the dependencies of the
 *	other parameters as follows:
 *	   VL=16 && USE_VNNI=0 && USE_AVX512=0: at least sse2
 *	   VL=32 && USE_VNNI=0 && USE_AVX512=0: at least avx2
 *	   VL=32 && USE_VNNI=1 && USE_AVX512=0: at least avx2,avxvnni
 *	   VL=32 && USE_VNNI=1 && USE_AVX512=1: at least avx512bw,avx512vl,avx512vnni
 *	   VL=64 && USE_VNNI=1 && USE_AVX512=1: at least avx512bw,avx512vnni
 *	   (Other combinations are not useful and have not been tested.)
 * VL:
 *	Vector length in bytes.  Must be 16, 32, or 64.
 * USE_VNNI:
 *	If 1, use the VNNI dot product based algorithm.
 *	If 0, use the legacy SSE2 and AVX2 compatible algorithm.
 * USE_AVX512:
 *	If 1, take advantage of AVX-512 features such as masking.  This doesn't
 *	enable the use of 512-bit vectors; the vector length is controlled by
 *	VL.  If 0, assume that the CPU might not support AVX-512.
 */

#if VL == 16
#  define vec_t			__m128i
#  define mask_t		u16
#  define LOG2_VL		4
#  define VADD8(a, b)		_mm_add_epi8((a), (b))
#  define VADD16(a, b)		_mm_add_epi16((a), (b))
#  define VADD32(a, b)		_mm_add_epi32((a), (b))
#  if USE_AVX512
#    define VDPBUSD(a, b, c)	_mm_dpbusd_epi32((a), (b), (c))
#  else
#    define VDPBUSD(a, b, c)	_mm_dpbusd_avx_epi32((a), (b), (c))
#  endif
#  define VLOAD(p)		_mm_load_si128((const void *)(p))
#  define VLOADU(p)		_mm_loadu_si128((const void *)(p))
#  define VMADD16(a, b)		_mm_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm_set1_epi8(a)
#  define VSET1_32(a)		_mm_set1_epi32(a)
#  define VSETZERO()		_mm_setzero_si128()
#  define VSLL32(a, b)		_mm_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm_unpackhi_epi8((a), (b))
#elif VL == 32
#  define vec_t			__m256i
#  define mask_t		u32
#  define LOG2_VL		5
#  define VADD8(a, b)		_mm256_add_epi8((a), (b))
#  define VADD16(a, b)		_mm256_add_epi16((a), (b))
#  define VADD32(a, b)		_mm256_add_epi32((a), (b))
#  if USE_AVX512
#    define VDPBUSD(a, b, c)	_mm256_dpbusd_epi32((a), (b), (c))
#  else
#    define VDPBUSD(a, b, c)	_mm256_dpbusd_avx_epi32((a), (b), (c))
#  endif
#  define VLOAD(p)		_mm256_load_si256((const void *)(p))
#  define VLOADU(p)		_mm256_loadu_si256((const void *)(p))
#  define VMADD16(a, b)		_mm256_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm256_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm256_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm256_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm256_set1_epi8(a)
#  define VSET1_32(a)		_mm256_set1_epi32(a)
#  define VSETZERO()		_mm256_setzero_si256()
#  define VSLL32(a, b)		_mm256_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm256_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm256_unpackhi_epi8((a), (b))
#elif VL == 64
#  define vec_t			__m512i
#  define mask_t		u64
#  define LOG2_VL		6
#  define VADD8(a, b)		_mm512_add_epi8((a), (b))
#  define VADD16(a, b)		_mm512_add_epi16((a), (b))
#  define VADD32(a, b)		_mm512_add_epi32((a), (b))
#  define VDPBUSD(a, b, c)	_mm512_dpbusd_epi32((a), (b), (c))
#  define VLOAD(p)		_mm512_load_si512((const void *)(p))
#  define VLOADU(p)		_mm512_loadu_si512((const void *)(p))
#  define VMADD16(a, b)		_mm512_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm512_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm512_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm512_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm512_set1_epi8(a)
#  define VSET1_32(a)		_mm512_set1_epi32(a)
#  define VSETZERO()		_mm512_setzero_si512()
#  define VSLL32(a, b)		_mm512_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm512_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm512_unpackhi_epi8((a), (b))
#else
#  error "unsupported vector length"
#endif

#define VADD32_3X(a, b, c)	VADD32(VADD32((a), (b)), (c))
#define VADD32_4X(a, b, c, d)	VADD32(VADD32((a), (b)), VADD32((c), (d)))
#define VADD32_5X(a, b, c, d, e) VADD32((a), VADD32_4X((b), (c), (d), (e)))
#define VADD32_7X(a, b, c, d, e, f, g)	\
	VADD32(VADD32_3X((a), (b), (c)), VADD32_4X((d), (e), (f), (g)))

/* Sum the 32-bit elements of v_s1 and add them to s1, and likewise for s2. */
#undef reduce_to_32bits
static forceinline ATTRIBUTES void
ADD_SUFFIX(reduce_to_32bits)(vec_t v_s1, vec_t v_s2, u32 *s1_p, u32 *s2_p)
{
	__m128i v_s1_128, v_s2_128;
#if VL == 16
	{
		v_s1_128 = v_s1;
		v_s2_128 = v_s2;
	}
#else
	{
		__m256i v_s1_256, v_s2_256;
	#if VL == 32
		v_s1_256 = v_s1;
		v_s2_256 = v_s2;
	#else
		/* Reduce 512 bits to 256 bits. */
		v_s1_256 = _mm256_add_epi32(_mm512_extracti64x4_epi64(v_s1, 0),
					    _mm512_extracti64x4_epi64(v_s1, 1));
		v_s2_256 = _mm256_add_epi32(_mm512_extracti64x4_epi64(v_s2, 0),
					    _mm512_extracti64x4_epi64(v_s2, 1));
	#endif
		/* Reduce 256 bits to 128 bits. */
		v_s1_128 = _mm_add_epi32(_mm256_extracti128_si256(v_s1_256, 0),
					 _mm256_extracti128_si256(v_s1_256, 1));
		v_s2_128 = _mm_add_epi32(_mm256_extracti128_si256(v_s2_256, 0),
					 _mm256_extracti128_si256(v_s2_256, 1));
	}
#endif

	/*
	 * Reduce 128 bits to 32 bits.
	 *
	 * If the bytes were summed into v_s1 using psadbw + paddd, then ignore
	 * the odd-indexed elements of v_s1_128 since they are zero.
	 */
#if USE_VNNI
	v_s1_128 = _mm_add_epi32(v_s1_128, _mm_shuffle_epi32(v_s1_128, 0x31));
#endif
	v_s2_128 = _mm_add_epi32(v_s2_128, _mm_shuffle_epi32(v_s2_128, 0x31));
	v_s1_128 = _mm_add_epi32(v_s1_128, _mm_shuffle_epi32(v_s1_128, 0x02));
	v_s2_128 = _mm_add_epi32(v_s2_128, _mm_shuffle_epi32(v_s2_128, 0x02));

	*s1_p += (u32)_mm_cvtsi128_si32(v_s1_128);
	*s2_p += (u32)_mm_cvtsi128_si32(v_s2_128);
}
#define reduce_to_32bits	ADD_SUFFIX(reduce_to_32bits)

static ATTRIBUTES u32
ADD_SUFFIX(adler32_x86)(u32 adler, const u8 *p, size_t len)
{
#if USE_VNNI
	/* This contains the bytes [VL, VL-1, VL-2, ..., 1]. */
	static const u8 _aligned_attribute(VL) raw_mults[VL] = {
	#if VL == 64
		64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,
		48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,
	#endif
	#if VL >= 32
		32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
	#endif
		16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,
	};
	const vec_t ones = VSET1_8(1);
#else
	/*
	 * This contains the 16-bit values [2*VL, 2*VL - 1, 2*VL - 2, ..., 1].
	 * For VL==32 the ordering is weird because it has to match the way that
	 * vpunpcklbw and vpunpckhbw work on 128-bit lanes separately.
	 */
	static const u16 _aligned_attribute(VL) raw_mults[4][VL / 2] = {
	#if VL == 16
		{ 32, 31, 30, 29, 28, 27, 26, 25 },
		{ 24, 23, 22, 21, 20, 19, 18, 17 },
		{ 16, 15, 14, 13, 12, 11, 10, 9  },
		{ 8,  7,  6,  5,  4,  3,  2,  1  },
	#elif VL == 32
		{ 64, 63, 62, 61, 60, 59, 58, 57, 48, 47, 46, 45, 44, 43, 42, 41 },
		{ 56, 55, 54, 53, 52, 51, 50, 49, 40, 39, 38, 37, 36, 35, 34, 33 },
		{ 32, 31, 30, 29, 28, 27, 26, 25, 16, 15, 14, 13, 12, 11, 10,  9 },
		{ 24, 23, 22, 21, 20, 19, 18, 17,  8,  7,  6,  5,  4,  3,  2,  1 },
	#else
	#  error "unsupported parameters"
	#endif
	};
	const vec_t mults_a = VLOAD(raw_mults[0]);
	const vec_t mults_b = VLOAD(raw_mults[1]);
	const vec_t mults_c = VLOAD(raw_mults[2]);
	const vec_t mults_d = VLOAD(raw_mults[3]);
#endif
	const vec_t zeroes = VSETZERO();
	u32 s1 = adler & 0xFFFF;
	u32 s2 = adler >> 16;

	/*
	 * If the length is large and the pointer is misaligned, align it.
	 * For smaller lengths, just take the misaligned load penalty.
	 */
	if (unlikely(len > 65536 && ((uintptr_t)p & (VL-1)))) {
		do {
			s1 += *p++;
			s2 += s1;
			len--;
		} while ((uintptr_t)p & (VL-1));
		s1 %= DIVISOR;
		s2 %= DIVISOR;
	}

#if USE_VNNI
	/*
	 * This is Adler-32 using the vpdpbusd instruction from AVX512VNNI or
	 * AVX-VNNI.  vpdpbusd multiplies the unsigned bytes of one vector by
	 * the signed bytes of another vector and adds the sums in groups of 4
	 * to the 32-bit elements of a third vector.  We use it in two ways:
	 * multiplying the data bytes by a sequence like 64,63,62,...,1 for
	 * calculating part of s2, and multiplying the data bytes by an all-ones
	 * sequence 1,1,1,...,1 for calculating s1 and part of s2.  The all-ones
	 * trick seems to be faster than the alternative of vpsadbw + vpaddd.
	 */
	while (len) {
		/*
		 * Calculate the length of the next data chunk such that s1 and
		 * s2 are guaranteed to not exceed UINT32_MAX.
		 */
		size_t n = MIN(len, MAX_CHUNK_LEN & ~(4*VL - 1));
		vec_t mults = VLOAD(raw_mults);
		vec_t v_s1 = zeroes;
		vec_t v_s2 = zeroes;

		s2 += s1 * n;
		len -= n;

		if (n >= 4*VL) {
			vec_t v_s1_b = zeroes;
			vec_t v_s1_c = zeroes;
			vec_t v_s1_d = zeroes;
			vec_t v_s2_b = zeroes;
			vec_t v_s2_c = zeroes;
			vec_t v_s2_d = zeroes;
			vec_t v_s1_sums   = zeroes;
			vec_t v_s1_sums_b = zeroes;
			vec_t v_s1_sums_c = zeroes;
			vec_t v_s1_sums_d = zeroes;
			vec_t tmp0, tmp1;

			do {
				vec_t data_a = VLOADU(p + 0*VL);
				vec_t data_b = VLOADU(p + 1*VL);
				vec_t data_c = VLOADU(p + 2*VL);
				vec_t data_d = VLOADU(p + 3*VL);

				/*
				 * Workaround for gcc bug where it generates
				 * unnecessary move instructions
				 * (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107892)
				 */
			#if GCC_PREREQ(1, 0)
				__asm__("" : "+v" (data_a), "+v" (data_b),
					     "+v" (data_c), "+v" (data_d));
			#endif

				v_s2   = VDPBUSD(v_s2,   data_a, mults);
				v_s2_b = VDPBUSD(v_s2_b, data_b, mults);
				v_s2_c = VDPBUSD(v_s2_c, data_c, mults);
				v_s2_d = VDPBUSD(v_s2_d, data_d, mults);

				v_s1_sums   = VADD32(v_s1_sums,   v_s1);
				v_s1_sums_b = VADD32(v_s1_sums_b, v_s1_b);
				v_s1_sums_c = VADD32(v_s1_sums_c, v_s1_c);
				v_s1_sums_d = VADD32(v_s1_sums_d, v_s1_d);

				v_s1   = VDPBUSD(v_s1,   data_a, ones);
				v_s1_b = VDPBUSD(v_s1_b, data_b, ones);
				v_s1_c = VDPBUSD(v_s1_c, data_c, ones);
				v_s1_d = VDPBUSD(v_s1_d, data_d, ones);

				/* Same gcc bug workaround.  See above */
			#if GCC_PREREQ(1, 0) && !defined(ARCH_X86_32)
				__asm__("" : "+v" (v_s2), "+v" (v_s2_b),
					     "+v" (v_s2_c), "+v" (v_s2_d),
					     "+v" (v_s1_sums),
					     "+v" (v_s1_sums_b),
					     "+v" (v_s1_sums_c),
					     "+v" (v_s1_sums_d),
					     "+v" (v_s1), "+v" (v_s1_b),
					     "+v" (v_s1_c), "+v" (v_s1_d));
			#endif
				p += 4*VL;
				n -= 4*VL;
			} while (n >= 4*VL);

			/*
			 * Reduce into v_s1 and v_s2 as follows:
			 *
			 * v_s2 = v_s2 + v_s2_b + v_s2_c + v_s2_d +
			 *	  (4*VL)*(v_s1_sums   + v_s1_sums_b +
			 *		  v_s1_sums_c + v_s1_sums_d) +
			 *	  (3*VL)*v_s1 + (2*VL)*v_s1_b + VL*v_s1_c
			 * v_s1 = v_s1 + v_s1_b + v_s1_c + v_s1_d
			 */
			tmp0 = VADD32(v_s1, v_s1_b);
			tmp1 = VADD32(v_s1, v_s1_c);
			v_s1_sums = VADD32_4X(v_s1_sums, v_s1_sums_b,
					      v_s1_sums_c, v_s1_sums_d);
			v_s1 = VADD32_3X(tmp0, v_s1_c, v_s1_d);
			v_s2 = VADD32_7X(VSLL32(v_s1_sums, LOG2_VL + 2),
					 VSLL32(tmp0, LOG2_VL + 1),
					 VSLL32(tmp1, LOG2_VL),
					 v_s2, v_s2_b, v_s2_c, v_s2_d);
		}

		/* Process the last 0 <= n < 4*VL bytes of the chunk. */
		if (n >= 2*VL) {
			const vec_t data_a = VLOADU(p + 0*VL);
			const vec_t data_b = VLOADU(p + 1*VL);

			v_s2 = VADD32(v_s2, VSLL32(v_s1, LOG2_VL + 1));
			v_s1 = VDPBUSD(v_s1, data_a, ones);
			v_s1 = VDPBUSD(v_s1, data_b, ones);
			v_s2 = VDPBUSD(v_s2, data_a, VSET1_8(VL));
			v_s2 = VDPBUSD(v_s2, data_a, mults);
			v_s2 = VDPBUSD(v_s2, data_b, mults);
			p += 2*VL;
			n -= 2*VL;
		}
		if (n) {
			/* Process the last 0 < n < 2*VL bytes of the chunk. */
			vec_t data;

			v_s2 = VADD32(v_s2, VMULLO32(v_s1, VSET1_32(n)));

			mults = VADD8(mults, VSET1_8((int)n - VL));
			if (n > VL) {
				data = VLOADU(p);
				v_s1 = VDPBUSD(v_s1, data, ones);
				v_s2 = VDPBUSD(v_s2, data, mults);
				p += VL;
				n -= VL;
				mults = VADD8(mults, VSET1_8(-VL));
			}
			/*
			 * Process the last 0 < n <= VL bytes of the chunk.
			 * Utilize a masked load if it's available.
			 */
		#if USE_AVX512
			data = VMASKZ_LOADU((mask_t)-1 >> (VL - n), p);
		#else
			data = zeroes;
			memcpy(&data, p, n);
		#endif
			v_s1 = VDPBUSD(v_s1, data, ones);
			v_s2 = VDPBUSD(v_s2, data, mults);
			p += n;
		}

		reduce_to_32bits(v_s1, v_s2, &s1, &s2);
		s1 %= DIVISOR;
		s2 %= DIVISOR;
	}
#else /* USE_VNNI */
	/*
	 * This is Adler-32 for SSE2 and AVX2.
	 *
	 * To horizontally sum bytes, use psadbw + paddd, where one of the
	 * arguments to psadbw is all-zeroes.
	 *
	 * For the s2 contribution from (2*VL - i)*data[i] for each of the 2*VL
	 * bytes of each iteration of the inner loop, use punpck{l,h}bw + paddw
	 * to sum, for each i across iterations, byte i into a corresponding
	 * 16-bit counter in v_byte_sums_*.  After the inner loop, use pmaddwd
	 * to multiply each counter by (2*VL - i), then add the products to s2.
	 *
	 * An alternative implementation would use pmaddubsw and pmaddwd in the
	 * inner loop to do (2*VL - i)*data[i] directly and add the products in
	 * groups of 4 to 32-bit counters.  However, on average that approach
	 * seems to be slower than the current approach which delays the
	 * multiplications.  Also, pmaddubsw requires SSSE3; the current
	 * approach keeps the implementation aligned between SSE2 and AVX2.
	 *
	 * The inner loop processes 2*VL bytes per iteration.  Increasing this
	 * to 4*VL doesn't seem to be helpful here.
	 */
	while (len) {
		/*
		 * Calculate the length of the next data chunk such that s1 and
		 * s2 are guaranteed to not exceed UINT32_MAX, and every
		 * v_byte_sums_* counter is guaranteed to not exceed INT16_MAX.
		 * It's INT16_MAX, not UINT16_MAX, because v_byte_sums_* are
		 * used with pmaddwd which does signed multiplication.  In the
		 * SSE2 case this limits chunks to 4096 bytes instead of 5536.
		 */
		size_t n = MIN(len, MIN(2 * VL * (INT16_MAX / UINT8_MAX),
					MAX_CHUNK_LEN) & ~(2*VL - 1));
		len -= n;

		if (n >= 2*VL) {
			vec_t v_s1 = zeroes;
			vec_t v_s1_sums = zeroes;
			vec_t v_byte_sums_a = zeroes;
			vec_t v_byte_sums_b = zeroes;
			vec_t v_byte_sums_c = zeroes;
			vec_t v_byte_sums_d = zeroes;
			vec_t v_s2;

			s2 += s1 * (n & ~(2*VL - 1));

			do {
				vec_t data_a = VLOADU(p + 0*VL);
				vec_t data_b = VLOADU(p + 1*VL);

				v_s1_sums = VADD32(v_s1_sums, v_s1);
				v_byte_sums_a = VADD16(v_byte_sums_a,
						       VUNPACKLO8(data_a, zeroes));
				v_byte_sums_b = VADD16(v_byte_sums_b,
						       VUNPACKHI8(data_a, zeroes));
				v_byte_sums_c = VADD16(v_byte_sums_c,
						       VUNPACKLO8(data_b, zeroes));
				v_byte_sums_d = VADD16(v_byte_sums_d,
						       VUNPACKHI8(data_b, zeroes));
				v_s1 = VADD32(v_s1,
					      VADD32(VSAD8(data_a, zeroes),
						     VSAD8(data_b, zeroes)));
				/*
				 * Workaround for gcc bug where it generates
				 * unnecessary move instructions
				 * (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107892)
				 */
			#if GCC_PREREQ(1, 0)
				__asm__("" : "+x" (v_s1), "+x" (v_s1_sums),
					     "+x" (v_byte_sums_a),
					     "+x" (v_byte_sums_b),
					     "+x" (v_byte_sums_c),
					     "+x" (v_byte_sums_d));
			#endif
				p += 2*VL;
				n -= 2*VL;
			} while (n >= 2*VL);

			/*
			 * Calculate v_s2 as (2*VL)*v_s1_sums +
			 * [2*VL, 2*VL - 1, 2*VL - 2, ..., 1] * v_byte_sums.
			 * Then update s1 and s2 from v_s1 and v_s2.
			 */
			v_s2 = VADD32_5X(VSLL32(v_s1_sums, LOG2_VL + 1),
					 VMADD16(v_byte_sums_a, mults_a),
					 VMADD16(v_byte_sums_b, mults_b),
					 VMADD16(v_byte_sums_c, mults_c),
					 VMADD16(v_byte_sums_d, mults_d));
			reduce_to_32bits(v_s1, v_s2, &s1, &s2);
		}
		/*
		 * Process the last 0 <= n < 2*VL bytes of the chunk using
		 * scalar instructions and reduce s1 and s2 mod DIVISOR.
		 */
		ADLER32_CHUNK(s1, s2, p, n);
	}
#endif /* !USE_VNNI */
	return (s2 << 16) | s1;
}

#undef vec_t
#undef mask_t
#undef LOG2_VL
#undef VADD8
#undef VADD16
#undef VADD32
#undef VDPBUSD
#undef VLOAD
#undef VLOADU
#undef VMADD16
#undef VMASKZ_LOADU
#undef VMULLO32
#undef VSAD8
#undef VSET1_8
#undef VSET1_32
#undef VSETZERO
#undef VSLL32
#undef VUNPACKLO8
#undef VUNPACKHI8

#undef SUFFIX
#undef ATTRIBUTES
#undef VL
#undef USE_VNNI
#undef USE_AVX512
