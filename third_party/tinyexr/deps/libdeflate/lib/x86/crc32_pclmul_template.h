/*
 * x86/crc32_pclmul_template.h - gzip CRC-32 with PCLMULQDQ instructions
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
 * This file is a "template" for instantiating PCLMULQDQ-based crc32_x86
 * functions.  The "parameters" are:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.  Must satisfy the dependencies of the
 *	other parameters as follows:
 *	   VL=16 && USE_AVX512=0: at least pclmul,sse4.1
 *	   VL=32 && USE_AVX512=0: at least vpclmulqdq,pclmul,avx2
 *	   VL=32 && USE_AVX512=1: at least vpclmulqdq,pclmul,avx512bw,avx512vl
 *	   VL=64 && USE_AVX512=1: at least vpclmulqdq,pclmul,avx512bw,avx512vl
 *	   (Other combinations are not useful and have not been tested.)
 * VL:
 *	Vector length in bytes.  Must be 16, 32, or 64.
 * USE_AVX512:
 *	If 1, take advantage of AVX-512 features such as masking and the
 *	vpternlog instruction.  This doesn't enable the use of 512-bit vectors;
 *	the vector length is controlled by VL.  If 0, assume that the CPU might
 *	not support AVX-512.
 *
 * The overall algorithm used is CRC folding with carryless multiplication
 * instructions.  Note that the x86 crc32 instruction cannot be used, as it is
 * for a different polynomial, not the gzip one.  For an explanation of CRC
 * folding with carryless multiplication instructions, see
 * scripts/gen-crc32-consts.py and the following blog posts and papers:
 *
 *	"An alternative exposition of crc32_4k_pclmulqdq"
 *	https://www.corsix.org/content/alternative-exposition-crc32_4k_pclmulqdq
 *
 *	"Fast CRC Computation for Generic Polynomials Using PCLMULQDQ Instruction"
 *	https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/fast-crc-computation-generic-polynomials-pclmulqdq-paper.pdf
 *
 * The original pclmulqdq instruction does one 64x64 to 128-bit carryless
 * multiplication.  The VPCLMULQDQ feature added instructions that do two
 * parallel 64x64 to 128-bit carryless multiplications in combination with AVX
 * or AVX512VL, or four in combination with AVX512F.
 */

#if VL == 16
#  define vec_t			__m128i
#  define fold_vec		fold_vec128
#  define VLOADU(p)		_mm_loadu_si128((const void *)(p))
#  define VXOR(a, b)		_mm_xor_si128((a), (b))
#  define M128I_TO_VEC(a)	a
#  define MULTS_8V		_mm_set_epi64x(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_4V		_mm_set_epi64x(CRC32_X479_MODG, CRC32_X543_MODG)
#  define MULTS_2V		_mm_set_epi64x(CRC32_X223_MODG, CRC32_X287_MODG)
#  define MULTS_1V		_mm_set_epi64x(CRC32_X95_MODG, CRC32_X159_MODG)
#elif VL == 32
#  define vec_t			__m256i
#  define fold_vec		fold_vec256
#  define VLOADU(p)		_mm256_loadu_si256((const void *)(p))
#  define VXOR(a, b)		_mm256_xor_si256((a), (b))
#  define M128I_TO_VEC(a)	_mm256_zextsi128_si256(a)
#  define MULTS(a, b)		_mm256_set_epi64x(a, b, a, b)
#  define MULTS_8V		MULTS(CRC32_X2015_MODG, CRC32_X2079_MODG)
#  define MULTS_4V		MULTS(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_2V		MULTS(CRC32_X479_MODG, CRC32_X543_MODG)
#  define MULTS_1V		MULTS(CRC32_X223_MODG, CRC32_X287_MODG)
#elif VL == 64
#  define vec_t			__m512i
#  define fold_vec		fold_vec512
#  define VLOADU(p)		_mm512_loadu_si512((const void *)(p))
#  define VXOR(a, b)		_mm512_xor_si512((a), (b))
#  define M128I_TO_VEC(a)	_mm512_zextsi128_si512(a)
#  define MULTS(a, b)		_mm512_set_epi64(a, b, a, b, a, b, a, b)
#  define MULTS_8V		MULTS(CRC32_X4063_MODG, CRC32_X4127_MODG)
#  define MULTS_4V		MULTS(CRC32_X2015_MODG, CRC32_X2079_MODG)
#  define MULTS_2V		MULTS(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_1V		MULTS(CRC32_X479_MODG, CRC32_X543_MODG)
#else
#  error "unsupported vector length"
#endif

#undef fold_vec128
static forceinline ATTRIBUTES __m128i
ADD_SUFFIX(fold_vec128)(__m128i src, __m128i dst, __m128i /* __v2du */ mults)
{
	dst = _mm_xor_si128(dst, _mm_clmulepi64_si128(src, mults, 0x00));
	dst = _mm_xor_si128(dst, _mm_clmulepi64_si128(src, mults, 0x11));
	return dst;
}
#define fold_vec128	ADD_SUFFIX(fold_vec128)

#if VL >= 32
#undef fold_vec256
static forceinline ATTRIBUTES __m256i
ADD_SUFFIX(fold_vec256)(__m256i src, __m256i dst, __m256i /* __v4du */ mults)
{
#if USE_AVX512
	/* vpternlog with immediate 0x96 is a three-argument XOR. */
	return _mm256_ternarylogic_epi32(
			_mm256_clmulepi64_epi128(src, mults, 0x00),
			_mm256_clmulepi64_epi128(src, mults, 0x11),
			dst,
			0x96);
#else
	return _mm256_xor_si256(
			_mm256_xor_si256(dst,
					 _mm256_clmulepi64_epi128(src, mults, 0x00)),
			_mm256_clmulepi64_epi128(src, mults, 0x11));
#endif
}
#define fold_vec256	ADD_SUFFIX(fold_vec256)
#endif /* VL >= 32 */

#if VL >= 64
#undef fold_vec512
static forceinline ATTRIBUTES __m512i
ADD_SUFFIX(fold_vec512)(__m512i src, __m512i dst, __m512i /* __v8du */ mults)
{
	/* vpternlog with immediate 0x96 is a three-argument XOR. */
	return _mm512_ternarylogic_epi32(
			_mm512_clmulepi64_epi128(src, mults, 0x00),
			_mm512_clmulepi64_epi128(src, mults, 0x11),
			dst,
			0x96);
}
#define fold_vec512	ADD_SUFFIX(fold_vec512)
#endif /* VL >= 64 */

/*
 * Given 'x' containing a 16-byte polynomial, and a pointer 'p' that points to
 * the next '1 <= len <= 15' data bytes, rearrange the concatenation of 'x' and
 * the data into vectors x0 and x1 that contain 'len' bytes and 16 bytes,
 * respectively.  Then fold x0 into x1 and return the result.
 * Assumes that 'p + len - 16' is in-bounds.
 */
#undef fold_lessthan16bytes
static forceinline ATTRIBUTES __m128i
ADD_SUFFIX(fold_lessthan16bytes)(__m128i x, const u8 *p, size_t len,
				 __m128i /* __v2du */ mults_128b)
{
	__m128i lshift = _mm_loadu_si128((const void *)&shift_tab[len]);
	__m128i rshift = _mm_loadu_si128((const void *)&shift_tab[len + 16]);
	__m128i x0, x1;

	/* x0 = x left-shifted by '16 - len' bytes */
	x0 = _mm_shuffle_epi8(x, lshift);

	/*
	 * x1 = the last '16 - len' bytes from x (i.e. x right-shifted by 'len'
	 * bytes) followed by the remaining data.
	 */
	x1 = _mm_blendv_epi8(_mm_shuffle_epi8(x, rshift),
			     _mm_loadu_si128((const void *)(p + len - 16)),
			     /* msb 0/1 of each byte selects byte from arg1/2 */
			     rshift);

	return fold_vec128(x0, x1, mults_128b);
}
#define fold_lessthan16bytes	ADD_SUFFIX(fold_lessthan16bytes)

static ATTRIBUTES u32
ADD_SUFFIX(crc32_x86)(u32 crc, const u8 *p, size_t len)
{
	/*
	 * mults_{N}v are the vectors of multipliers for folding across N vec_t
	 * vectors, i.e. N*VL*8 bits.  mults_128b are the two multipliers for
	 * folding across 128 bits.  mults_128b differs from mults_1v when
	 * VL != 16.  All multipliers are 64-bit, to match what pclmulqdq needs,
	 * but since this is for CRC-32 only their low 32 bits are nonzero.
	 * For more details, see scripts/gen-crc32-consts.py.
	 */
	const vec_t mults_8v = MULTS_8V;
	const vec_t mults_4v = MULTS_4V;
	const vec_t mults_2v = MULTS_2V;
	const vec_t mults_1v = MULTS_1V;
	const __m128i mults_128b = _mm_set_epi64x(CRC32_X95_MODG, CRC32_X159_MODG);
	const __m128i barrett_reduction_constants =
		_mm_set_epi64x(CRC32_BARRETT_CONSTANT_2, CRC32_BARRETT_CONSTANT_1);
	vec_t v0, v1, v2, v3, v4, v5, v6, v7;
	__m128i x0 = _mm_cvtsi32_si128(crc);
	__m128i x1;

	if (len < 8*VL) {
		if (len < VL) {
			STATIC_ASSERT(VL == 16 || VL == 32 || VL == 64);
			if (len < 16) {
			#if USE_AVX512
				if (len < 4)
					return crc32_slice1(crc, p, len);
				/*
				 * Handle 4 <= len <= 15 bytes by doing a masked
				 * load, XOR'ing the current CRC with the first
				 * 4 bytes, left-shifting by '16 - len' bytes to
				 * align the result to the end of x0 (so that it
				 * becomes the low-order coefficients of a
				 * 128-bit polynomial), and then doing the usual
				 * reduction from 128 bits to 32 bits.
				 */
				x0 = _mm_xor_si128(
					x0, _mm_maskz_loadu_epi8((1 << len) - 1, p));
				x0 = _mm_shuffle_epi8(
					x0, _mm_loadu_si128((const void *)&shift_tab[len]));
				goto reduce_x0;
			#else
				return crc32_slice1(crc, p, len);
			#endif
			}
			/*
			 * Handle 16 <= len < VL bytes where VL is 32 or 64.
			 * Use 128-bit instructions so that these lengths aren't
			 * slower with VL > 16 than with VL=16.
			 */
			x0 = _mm_xor_si128(_mm_loadu_si128((const void *)p), x0);
			if (len >= 32) {
				x0 = fold_vec128(x0, _mm_loadu_si128((const void *)(p + 16)),
						 mults_128b);
				if (len >= 48)
					x0 = fold_vec128(x0, _mm_loadu_si128((const void *)(p + 32)),
							 mults_128b);
			}
			p += len & ~15;
			goto less_than_16_remaining;
		}
		v0 = VXOR(VLOADU(p), M128I_TO_VEC(x0));
		if (len < 2*VL) {
			p += VL;
			goto less_than_vl_remaining;
		}
		v1 = VLOADU(p + 1*VL);
		if (len < 4*VL) {
			p += 2*VL;
			goto less_than_2vl_remaining;
		}
		v2 = VLOADU(p + 2*VL);
		v3 = VLOADU(p + 3*VL);
		p += 4*VL;
	} else {
		/*
		 * If the length is large and the pointer is misaligned, align
		 * it.  For smaller lengths, just take the misaligned load
		 * penalty.  Note that on recent x86 CPUs, vmovdqu with an
		 * aligned address is just as fast as vmovdqa, so there's no
		 * need to use vmovdqa in the main loop.
		 */
		if (len > 65536 && ((uintptr_t)p & (VL-1))) {
			size_t align = -(uintptr_t)p & (VL-1);

			len -= align;
			x0 = _mm_xor_si128(_mm_loadu_si128((const void *)p), x0);
			p += 16;
			if (align & 15) {
				x0 = fold_lessthan16bytes(x0, p, align & 15,
							  mults_128b);
				p += align & 15;
				align &= ~15;
			}
			while (align) {
				x0 = fold_vec128(x0, *(const __m128i *)p,
						 mults_128b);
				p += 16;
				align -= 16;
			}
			v0 = M128I_TO_VEC(x0);
		#  if VL == 32
			v0 = _mm256_inserti128_si256(v0, *(const __m128i *)p, 1);
		#  elif VL == 64
			v0 = _mm512_inserti32x4(v0, *(const __m128i *)p, 1);
			v0 = _mm512_inserti64x4(v0, *(const __m256i *)(p + 16), 1);
		#  endif
			p -= 16;
		} else {
			v0 = VXOR(VLOADU(p), M128I_TO_VEC(x0));
		}
		v1 = VLOADU(p + 1*VL);
		v2 = VLOADU(p + 2*VL);
		v3 = VLOADU(p + 3*VL);
		v4 = VLOADU(p + 4*VL);
		v5 = VLOADU(p + 5*VL);
		v6 = VLOADU(p + 6*VL);
		v7 = VLOADU(p + 7*VL);
		p += 8*VL;

		/*
		 * This is the main loop, processing 8*VL bytes per iteration.
		 * 4*VL is usually enough and would result in smaller code, but
		 * Skylake and Cascade Lake need 8*VL to get full performance.
		 */
		while (len >= 16*VL) {
			v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_8v);
			v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_8v);
			v2 = fold_vec(v2, VLOADU(p + 2*VL), mults_8v);
			v3 = fold_vec(v3, VLOADU(p + 3*VL), mults_8v);
			v4 = fold_vec(v4, VLOADU(p + 4*VL), mults_8v);
			v5 = fold_vec(v5, VLOADU(p + 5*VL), mults_8v);
			v6 = fold_vec(v6, VLOADU(p + 6*VL), mults_8v);
			v7 = fold_vec(v7, VLOADU(p + 7*VL), mults_8v);
			p += 8*VL;
			len -= 8*VL;
		}

		/* Fewer than 8*VL bytes remain. */
		v0 = fold_vec(v0, v4, mults_4v);
		v1 = fold_vec(v1, v5, mults_4v);
		v2 = fold_vec(v2, v6, mults_4v);
		v3 = fold_vec(v3, v7, mults_4v);
		if (len & (4*VL)) {
			v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_4v);
			v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_4v);
			v2 = fold_vec(v2, VLOADU(p + 2*VL), mults_4v);
			v3 = fold_vec(v3, VLOADU(p + 3*VL), mults_4v);
			p += 4*VL;
		}
	}
	/* Fewer than 4*VL bytes remain. */
	v0 = fold_vec(v0, v2, mults_2v);
	v1 = fold_vec(v1, v3, mults_2v);
	if (len & (2*VL)) {
		v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_2v);
		v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_2v);
		p += 2*VL;
	}
less_than_2vl_remaining:
	/* Fewer than 2*VL bytes remain. */
	v0 = fold_vec(v0, v1, mults_1v);
	if (len & VL) {
		v0 = fold_vec(v0, VLOADU(p), mults_1v);
		p += VL;
	}
less_than_vl_remaining:
	/*
	 * Fewer than VL bytes remain.  Reduce v0 (length VL bytes) to x0
	 * (length 16 bytes) and fold in any 16-byte data segments that remain.
	 */
#if VL == 16
	x0 = v0;
#else
	{
	#if VL == 32
		__m256i y0 = v0;
	#else
		const __m256i mults_256b =
			_mm256_set_epi64x(CRC32_X223_MODG, CRC32_X287_MODG,
					  CRC32_X223_MODG, CRC32_X287_MODG);
		__m256i y0 = fold_vec256(_mm512_extracti64x4_epi64(v0, 0),
					 _mm512_extracti64x4_epi64(v0, 1),
					 mults_256b);
		if (len & 32) {
			y0 = fold_vec256(y0, _mm256_loadu_si256((const void *)p),
					 mults_256b);
			p += 32;
		}
	#endif
		x0 = fold_vec128(_mm256_extracti128_si256(y0, 0),
				 _mm256_extracti128_si256(y0, 1), mults_128b);
	}
	if (len & 16) {
		x0 = fold_vec128(x0, _mm_loadu_si128((const void *)p),
				 mults_128b);
		p += 16;
	}
#endif
less_than_16_remaining:
	len &= 15;

	/* Handle any remainder of 1 to 15 bytes. */
	if (len)
		x0 = fold_lessthan16bytes(x0, p, len, mults_128b);
#if USE_AVX512
reduce_x0:
#endif
	/*
	 * Multiply the remaining 128-bit message polynomial 'x0' by x^32, then
	 * reduce it modulo the generator polynomial G.  This gives the CRC.
	 *
	 * This implementation matches that used in crc-pclmul-template.S from
	 * https://lore.kernel.org/r/20250210174540.161705-4-ebiggers@kernel.org/
	 * with the parameters n=32 and LSB_CRC=1 (what the gzip CRC uses).  See
	 * there for a detailed explanation of the math used here.
	 */
	x0 = _mm_xor_si128(_mm_clmulepi64_si128(x0, mults_128b, 0x10),
			   _mm_bsrli_si128(x0, 8));
	x1 = _mm_clmulepi64_si128(x0, barrett_reduction_constants, 0x00);
	x1 = _mm_clmulepi64_si128(x1, barrett_reduction_constants, 0x10);
	x0 = _mm_xor_si128(x0, x1);
	return _mm_extract_epi32(x0, 2);
}

#undef vec_t
#undef fold_vec
#undef VLOADU
#undef VXOR
#undef M128I_TO_VEC
#undef MULTS
#undef MULTS_8V
#undef MULTS_4V
#undef MULTS_2V
#undef MULTS_1V

#undef SUFFIX
#undef ATTRIBUTES
#undef VL
#undef USE_AVX512
