/*
 * TinyEXR - x86 SIMD kernels (SSE2 byte interleave, F16C half<->float).
 *
 * These use GCC/Clang function target attributes so the whole file compiles at
 * baseline ISA; the runtime dispatcher in exr_cpu.c only calls a kernel when
 * CPUID reports the matching feature. On MSVC the intrinsics compile directly.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

#if defined(EXR_X86)

#include <immintrin.h>

#if defined(__GNUC__) || defined(__clang__)
#define EXR_TARGET(x) __attribute__((target(x)))
#else
#define EXR_TARGET(x)
#endif

/* De-split: out[2i]=t1[i], out[2i+1]=t2[i] (inverse of the EXR byte split). */
EXR_TARGET("sse2")
void exr_interleave_sse2(const uint8_t *src, uint8_t *dst, size_t n) {
    size_t half = (n + 1) / 2, n2 = n / 2, i = 0;
    const uint8_t *t1 = src, *t2 = src + half;
    for (; i + 16 <= n2; i += 16) {
        __m128i a = _mm_loadu_si128((const __m128i *)(t1 + i));
        __m128i b = _mm_loadu_si128((const __m128i *)(t2 + i));
        _mm_storeu_si128((__m128i *)(dst + 2 * i), _mm_unpacklo_epi8(a, b));
        _mm_storeu_si128((__m128i *)(dst + 2 * i + 16), _mm_unpackhi_epi8(a, b));
    }
    for (; i < n2; ++i) {
        dst[2 * i] = t1[i];
        dst[2 * i + 1] = t2[i];
    }
    if (n & 1) dst[n - 1] = t1[n2];
}

/* Predictor decode: byte prefix-sum mod 256 with a -128 bias per step,
 * bit-identical to exr_predictor_decode_scalar. Within each 16-byte chunk the
 * prefix sum is built by log2(16) lane shifts; `running` carries the cumulative
 * total across chunks. */
EXR_TARGET("sse2")
void exr_predictor_decode_sse2(uint8_t *p, size_t n) {
    const __m128i bias = _mm_set1_epi8((char)128);
    uint8_t running;
    size_t i;
    if (n == 0) return;
    running = p[0];
    i = 1;
    for (; i + 16 <= n; i += 16) {
        __m128i x = _mm_loadu_si128((const __m128i *)(p + i));
        x = _mm_sub_epi8(x, bias);                  /* b = p - 128 */
        x = _mm_add_epi8(x, _mm_slli_si128(x, 1));
        x = _mm_add_epi8(x, _mm_slli_si128(x, 2));
        x = _mm_add_epi8(x, _mm_slli_si128(x, 4));
        x = _mm_add_epi8(x, _mm_slli_si128(x, 8));  /* full 16-lane prefix sum */
        x = _mm_add_epi8(x, _mm_set1_epi8((char)running));
        _mm_storeu_si128((__m128i *)(p + i), x);
        running = p[i + 15];
    }
    for (; i < n; ++i) {
        int d = (int)running + (int)p[i] - 128;
        p[i] = (uint8_t)d;
        running = p[i];
    }
}

/* Predictor encode: per-byte delta (cur - prev + 128) mod 256, in place,
 * bit-identical to exr_predictor_encode_scalar. `prev` is the original previous
 * byte, saved before the store overwrites it. */
EXR_TARGET("sse2")
void exr_predictor_encode_sse2(uint8_t *p, size_t n) {
    const __m128i bias = _mm_set1_epi8((char)128);
    uint8_t prev;
    size_t i;
    if (n == 0) return;
    prev = p[0];
    i = 1;
    for (; i + 16 <= n; i += 16) {
        __m128i cur = _mm_loadu_si128((const __m128i *)(p + i)); /* original */
        uint8_t nextprev = p[i + 15]; /* save before overwrite */
        __m128i sh = _mm_slli_si128(cur, 1); /* lane0=0, lanes1..15=p[i..i+14] */
        __m128i out;
        sh = _mm_or_si128(sh, _mm_cvtsi32_si128((int)prev)); /* lane0 = prev */
        out = _mm_add_epi8(_mm_sub_epi8(cur, sh), bias);
        _mm_storeu_si128((__m128i *)(p + i), out);
        prev = nextprev;
    }
    for (; i < n; ++i) {
        int cur = p[i];
        p[i] = (uint8_t)(cur - prev + (128 + 256));
        prev = cur;
    }
}

/* fpnge-style PSHUFB Huffman-table lookup: for each byte, gather (nbits, code)
 * where the 16-bit code is split into blo (low 8) + bhi (high). Symbols 0-15
 * and 240-255 are looked up by low nibble; 16-239 share length 12 (mid_nbits),
 * so their code is mid_lowbits[hi_nibble] | (revnib[lo_nibble] << 8). */
EXR_TARGET("sse4.1")
void exr_fpnge_lookup_sse41(const exr_fpnge_table *t, const uint8_t *src,
                            size_t count, uint8_t *nb, uint8_t *blo,
                            uint8_t *bhi) {
    static const uint8_t kRevNib[16] = {0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE,
                                        0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF};
    size_t i = 0;
    if (t->mid_nbits == 12) { /* always true for this construction */
        const __m128i f16n = _mm_loadu_si128((const __m128i *)t->first16_nbits);
        const __m128i f16b = _mm_loadu_si128((const __m128i *)t->first16_bits);
        const __m128i l16n = _mm_loadu_si128((const __m128i *)t->last16_nbits);
        const __m128i l16b = _mm_loadu_si128((const __m128i *)t->last16_bits);
        const __m128i midl = _mm_loadu_si128((const __m128i *)t->mid_lowbits);
        const __m128i rev = _mm_loadu_si128((const __m128i *)kRevNib);
        const __m128i nib = _mm_set1_epi8(0x0F);
        const __m128i midn = _mm_set1_epi8((char)t->mid_nbits);
        const __m128i zero = _mm_setzero_si128();
        const __m128i ones = _mm_set1_epi8((char)0xFF);
        const __m128i c15 = _mm_set1_epi8(15);
        const __m128i c239 = _mm_set1_epi8((char)239);
        for (; i + 16 <= count; i += 16) {
            __m128i b = _mm_loadu_si128((const __m128i *)(src + i));
            __m128i lo = _mm_and_si128(b, nib);
            __m128i hi = _mm_and_si128(_mm_srli_epi16(b, 4), nib);
            __m128i fnb = _mm_shuffle_epi8(f16n, lo);
            __m128i fb = _mm_shuffle_epi8(f16b, lo);
            __m128i lnb = _mm_shuffle_epi8(l16n, lo);
            __m128i lb = _mm_shuffle_epi8(l16b, lo);
            __m128i mlo = _mm_shuffle_epi8(midl, hi);
            __m128i mhi = _mm_shuffle_epi8(rev, lo);
            __m128i is_first = _mm_cmpeq_epi8(_mm_subs_epu8(b, c15), zero);
            __m128i is_last =
                _mm_xor_si128(_mm_cmpeq_epi8(_mm_subs_epu8(b, c239), zero), ones);
            __m128i fl = _mm_or_si128(is_first, is_last);
            __m128i rnb = _mm_blendv_epi8(midn, fnb, is_first);
            __m128i rlo = _mm_blendv_epi8(mlo, fb, is_first);
            rnb = _mm_blendv_epi8(rnb, lnb, is_last);
            rlo = _mm_blendv_epi8(rlo, lb, is_last);
            _mm_storeu_si128((__m128i *)(nb + i), rnb);
            _mm_storeu_si128((__m128i *)(blo + i), rlo);
            _mm_storeu_si128((__m128i *)(bhi + i), _mm_andnot_si128(fl, mhi));
        }
    }
    if (i < count)
        exr_fpnge_lookup_scalar(t, src + i, count - i, nb + i, blo + i, bhi + i);
}

/* AVX2 variant: 32 bytes/iter via _mm256_shuffle_epi8 (per-128-bit-lane PSHUFB
 * with the 16-entry tables broadcast to both lanes). */
EXR_TARGET("avx2")
void exr_fpnge_lookup_avx2(const exr_fpnge_table *t, const uint8_t *src,
                           size_t count, uint8_t *nb, uint8_t *blo,
                           uint8_t *bhi) {
    static const uint8_t kRevNib[16] = {0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE,
                                        0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF};
    size_t i = 0;
    if (t->mid_nbits == 12) {
        const __m256i f16n = _mm256_broadcastsi128_si256(
            _mm_loadu_si128((const __m128i *)t->first16_nbits));
        const __m256i f16b = _mm256_broadcastsi128_si256(
            _mm_loadu_si128((const __m128i *)t->first16_bits));
        const __m256i l16n = _mm256_broadcastsi128_si256(
            _mm_loadu_si128((const __m128i *)t->last16_nbits));
        const __m256i l16b = _mm256_broadcastsi128_si256(
            _mm_loadu_si128((const __m128i *)t->last16_bits));
        const __m256i midl = _mm256_broadcastsi128_si256(
            _mm_loadu_si128((const __m128i *)t->mid_lowbits));
        const __m256i rev = _mm256_broadcastsi128_si256(
            _mm_loadu_si128((const __m128i *)kRevNib));
        const __m256i nib = _mm256_set1_epi8(0x0F);
        const __m256i midn = _mm256_set1_epi8((char)t->mid_nbits);
        const __m256i zero = _mm256_setzero_si256();
        const __m256i ones = _mm256_set1_epi8((char)0xFF);
        const __m256i c15 = _mm256_set1_epi8(15);
        const __m256i c239 = _mm256_set1_epi8((char)239);
        for (; i + 32 <= count; i += 32) {
            __m256i b = _mm256_loadu_si256((const __m256i *)(src + i));
            __m256i lo = _mm256_and_si256(b, nib);
            __m256i hi = _mm256_and_si256(_mm256_srli_epi16(b, 4), nib);
            __m256i fnb = _mm256_shuffle_epi8(f16n, lo);
            __m256i fb = _mm256_shuffle_epi8(f16b, lo);
            __m256i lnb = _mm256_shuffle_epi8(l16n, lo);
            __m256i lb = _mm256_shuffle_epi8(l16b, lo);
            __m256i mlo = _mm256_shuffle_epi8(midl, hi);
            __m256i mhi = _mm256_shuffle_epi8(rev, lo);
            __m256i is_first = _mm256_cmpeq_epi8(_mm256_subs_epu8(b, c15), zero);
            __m256i is_last = _mm256_xor_si256(
                _mm256_cmpeq_epi8(_mm256_subs_epu8(b, c239), zero), ones);
            __m256i fl = _mm256_or_si256(is_first, is_last);
            __m256i rnb = _mm256_blendv_epi8(midn, fnb, is_first);
            __m256i rlo = _mm256_blendv_epi8(mlo, fb, is_first);
            rnb = _mm256_blendv_epi8(rnb, lnb, is_last);
            rlo = _mm256_blendv_epi8(rlo, lb, is_last);
            _mm256_storeu_si256((__m256i *)(nb + i), rnb);
            _mm256_storeu_si256((__m256i *)(blo + i), rlo);
            _mm256_storeu_si256((__m256i *)(bhi + i), _mm256_andnot_si256(fl, mhi));
        }
    }
    if (i < count)
        exr_fpnge_lookup_scalar(t, src + i, count - i, nb + i, blo + i, bhi + i);
}

/* SIMD bit-pack stage: combine code pairs into 32-bit groups. The variable
 * "odd << nb_even" shift uses the float-exponent trick on SSE (codes <= 15 bits
 * fit exactly in a float mantissa, so multiplying by 2^nb_even is exact). */
EXR_TARGET("sse4.1")
size_t exr_fpnge_pack16_sse41(const uint8_t *nb, const uint8_t *blo,
                              const uint8_t *bhi, size_t count,
                              uint32_t *combined, uint32_t *cnb) {
    size_t i = 0, np = count / 2, k;
    const __m128i m16 = _mm_set1_epi32(0xFFFF);
    const __m128i one16 = _mm_set1_epi16(1);
    const __m128i zero = _mm_setzero_si128();
    for (; i + 16 <= count; i += 16) {
        __m128i vblo = _mm_loadu_si128((const __m128i *)(blo + i));
        __m128i vbhi = _mm_loadu_si128((const __m128i *)(bhi + i));
        __m128i vnb = _mm_loadu_si128((const __m128i *)(nb + i));
        __m128i clo = _mm_unpacklo_epi8(vblo, vbhi); /* codes for bytes 0..7 */
        __m128i chi = _mm_unpackhi_epi8(vblo, vbhi); /* codes for bytes 8..15 */
        __m128i nlo = _mm_unpacklo_epi8(vnb, zero);
        __m128i nhi = _mm_unpackhi_epi8(vnb, zero);
        __m128i e0 = _mm_and_si128(clo, m16), o0 = _mm_srli_epi32(clo, 16);
        __m128i ne0 = _mm_and_si128(nlo, m16);
        __m128i ob0 = _mm_add_epi32(_mm_castps_si128(_mm_cvtepi32_ps(o0)),
                                    _mm_slli_epi32(ne0, 23));
        __m128i cm0 = _mm_or_si128(e0, _mm_cvtps_epi32(_mm_castsi128_ps(ob0)));
        __m128i e1 = _mm_and_si128(chi, m16), o1 = _mm_srli_epi32(chi, 16);
        __m128i ne1 = _mm_and_si128(nhi, m16);
        __m128i ob1 = _mm_add_epi32(_mm_castps_si128(_mm_cvtepi32_ps(o1)),
                                    _mm_slli_epi32(ne1, 23));
        __m128i cm1 = _mm_or_si128(e1, _mm_cvtps_epi32(_mm_castsi128_ps(ob1)));
        _mm_storeu_si128((__m128i *)(combined + i / 2), cm0);
        _mm_storeu_si128((__m128i *)(combined + i / 2 + 4), cm1);
        _mm_storeu_si128((__m128i *)(cnb + i / 2), _mm_madd_epi16(nlo, one16));
        _mm_storeu_si128((__m128i *)(cnb + i / 2 + 4), _mm_madd_epi16(nhi, one16));
    }
    for (k = i / 2; k < np; ++k) {
        uint32_t e = (uint32_t)blo[2 * k] | ((uint32_t)bhi[2 * k] << 8);
        uint32_t o = (uint32_t)blo[2 * k + 1] | ((uint32_t)bhi[2 * k + 1] << 8);
        combined[k] = e | (o << nb[2 * k]);
        cnb[k] = (uint32_t)nb[2 * k] + nb[2 * k + 1];
    }
    return np;
}

EXR_TARGET("avx2")
size_t exr_fpnge_pack16_avx2(const uint8_t *nb, const uint8_t *blo,
                             const uint8_t *bhi, size_t count,
                             uint32_t *combined, uint32_t *cnb) {
    size_t i = 0, np = count / 2, k;
    const __m256i m16 = _mm256_set1_epi32(0xFFFF);
    const __m256i one16 = _mm256_set1_epi16(1);
    const __m256i zero = _mm256_setzero_si256();
    for (; i + 32 <= count; i += 32) {
        __m256i vblo = _mm256_loadu_si256((const __m256i *)(blo + i));
        __m256i vbhi = _mm256_loadu_si256((const __m256i *)(bhi + i));
        __m256i vnb = _mm256_loadu_si256((const __m256i *)(nb + i));
        /* unpack is per-128-lane: clo = bytes {0..7, 16..23}, chi = {8..15,24..31} */
        __m256i clo = _mm256_unpacklo_epi8(vblo, vbhi);
        __m256i chi = _mm256_unpackhi_epi8(vblo, vbhi);
        __m256i nlo = _mm256_unpacklo_epi8(vnb, zero);
        __m256i nhi = _mm256_unpackhi_epi8(vnb, zero);
        __m256i e0 = _mm256_and_si256(clo, m16), o0 = _mm256_srli_epi32(clo, 16);
        __m256i cm0 = _mm256_or_si256(e0, _mm256_sllv_epi32(o0, _mm256_and_si256(nlo, m16)));
        __m256i e1 = _mm256_and_si256(chi, m16), o1 = _mm256_srli_epi32(chi, 16);
        __m256i cm1 = _mm256_or_si256(e1, _mm256_sllv_epi32(o1, _mm256_and_si256(nhi, m16)));
        __m256i sn0 = _mm256_madd_epi16(nlo, one16);
        __m256i sn1 = _mm256_madd_epi16(nhi, one16);
        /* reassemble sequential pair order across the 128-bit lanes */
        _mm256_storeu_si256((__m256i *)(combined + i / 2),
                            _mm256_permute2x128_si256(cm0, cm1, 0x20));
        _mm256_storeu_si256((__m256i *)(combined + i / 2 + 8),
                            _mm256_permute2x128_si256(cm0, cm1, 0x31));
        _mm256_storeu_si256((__m256i *)(cnb + i / 2),
                            _mm256_permute2x128_si256(sn0, sn1, 0x20));
        _mm256_storeu_si256((__m256i *)(cnb + i / 2 + 8),
                            _mm256_permute2x128_si256(sn0, sn1, 0x31));
    }
    for (k = i / 2; k < np; ++k) {
        uint32_t e = (uint32_t)blo[2 * k] | ((uint32_t)bhi[2 * k] << 8);
        uint32_t o = (uint32_t)blo[2 * k + 1] | ((uint32_t)bhi[2 * k + 1] << 8);
        combined[k] = e | (o << nb[2 * k]);
        cnb[k] = (uint32_t)nb[2 * k] + nb[2 * k + 1];
    }
    return np;
}

EXR_TARGET("avx2")
void exr_interleave_avx2(const uint8_t *src, uint8_t *dst, size_t n) {
    size_t half = (n + 1) / 2, n2 = n / 2, i = 0;
    const uint8_t *t1 = src, *t2 = src + half;
    for (; i + 32 <= n2; i += 32) {
        __m256i a = _mm256_loadu_si256((const __m256i *)(t1 + i));
        __m256i b = _mm256_loadu_si256((const __m256i *)(t2 + i));
        __m256i lo = _mm256_unpacklo_epi8(a, b);
        __m256i hi = _mm256_unpackhi_epi8(a, b);
        _mm256_storeu_si256((__m256i *)(dst + 2 * i),
                            _mm256_permute2x128_si256(lo, hi, 0x20));
        _mm256_storeu_si256((__m256i *)(dst + 2 * i + 32),
                            _mm256_permute2x128_si256(lo, hi, 0x31));
    }
    for (; i < n2; ++i) {
        dst[2 * i] = t1[i];
        dst[2 * i + 1] = t2[i];
    }
    if (n & 1) dst[n - 1] = t1[n2];
}

EXR_TARGET("avx2,f16c")
void exr_half_to_float_f16c(const uint16_t *src, float *dst, size_t count) {
    size_t i = 0;
    for (; i + 8 <= count; i += 8) {
        __m128i h = _mm_loadu_si128((const __m128i *)(src + i));
        _mm256_storeu_ps(dst + i, _mm256_cvtph_ps(h));
    }
    for (; i < count; ++i) {
        __m128i h = _mm_cvtsi32_si128(src[i]);
        dst[i] = _mm_cvtss_f32(_mm_cvtph_ps(h));
    }
}

#define EXR_F16_RND (_MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)
EXR_TARGET("avx2,f16c")
void exr_float_to_half_f16c(const float *src, uint16_t *dst, size_t count) {
    size_t i = 0;
    for (; i + 8 <= count; i += 8) {
        __m256 f = _mm256_loadu_ps(src + i);
        _mm_storeu_si128((__m128i *)(dst + i), _mm256_cvtps_ph(f, EXR_F16_RND));
    }
    for (; i < count; ++i) {
        __m128 f = _mm_set_ss(src[i]);
        __m128i h = _mm_cvtps_ph(f, EXR_F16_RND);
        dst[i] = (uint16_t)_mm_extract_epi16(h, 0);
    }
}

#endif /* EXR_X86 */
