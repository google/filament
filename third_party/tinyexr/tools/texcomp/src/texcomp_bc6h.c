/*
 * TinyEXR texcomp - BC6H encoder
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 *
 * The two-region (mode 9) partition table and block bit layout mirror the
 * corrected BPTC tables and decode in bcdec.h (Sergii Kudlai, MIT). See
 * tools/texcomp/NOTICE.md.
 */

#include "texcomp.h"
#include "texcomp_internal.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

void tc_bc6h_options_init(tc_bc6h_options *opt) {
    if (!opt) return;
    memset(opt, 0, sizeof(*opt));
}

size_t tc_bc6h_compressed_size(uint32_t width, uint32_t height) {
    return tc_bc7_compressed_size(width, height);
}

uint16_t tc_float_to_half_bits(float fv) {
    union {
        float f;
        uint32_t u;
    } v;
    uint32_t sign, mant, exp;
    v.f = fv;
    sign = (v.u >> 16) & 0x8000u;
    exp = (v.u >> 23) & 0xffu;
    mant = v.u & 0x7fffffu;
    if (exp == 255u) return (uint16_t)(sign | 0x7c00u | (mant ? 0x0200u : 0u));
    if (exp > 142u) return (uint16_t)(sign | 0x7c00u);
    if (exp < 113u) {
        uint32_t m;
        if (exp < 103u) return (uint16_t)sign;
        m = mant | 0x800000u;
        m >>= 125u - exp;
        m = (m + 0x1000u) >> 13;
        return (uint16_t)(sign | m);
    }
    exp = exp - 112u;
    mant = (mant + 0x1000u) >> 13;
    if (mant & 0x400u) {
        mant = 0;
        ++exp;
    }
    if (exp >= 31u) return (uint16_t)(sign | 0x7c00u);
    return (uint16_t)(sign | (exp << 10) | (mant & 0x3ffu));
}

static uint32_t tc_bc6h_quant_uf16(float f) {
    uint16_t h;
    uint32_t mag;
    if (!(f > 0.0f)) return 0;
    h = tc_float_to_half_bits(f);
    mag = h & 0x7fffu;
    if (mag > 0x7bffu) mag = 0x7bffu;
    return (mag * 1023u + 15871u) / 31743u;
}

static int32_t tc_bc6h_quant_sf16(float f) {
    uint16_t h;
    uint32_t mag;
    int32_t q;
    if (f == 0.0f) return 0;
    h = tc_float_to_half_bits(f);
    mag = h & 0x7fffu;
    if (mag > 0x7bffu) mag = 0x7bffu;
    q = (int32_t)((mag * 511u + 15871u) / 31743u);
    if (q > 511) q = 511;
    return (h & 0x8000u) ? -q : q;
}

static uint32_t tc_bc6h_unquant_uf16_to_mag(uint32_t q) {
    uint32_t unq;
    if (q == 0u) unq = 0u;
    else if (q >= 1023u) unq = 0xffffu;
    else unq = ((q << 16) + 0x8000u) >> 10;
    return (unq * 31u) >> 6;
}

static int32_t tc_bc6h_unquant_sf16_to_smag(int32_t q) {
    int32_t sign = 0, unq;
    if (q < 0) {
        sign = 1;
        q = -q;
    }
    if (q == 0) unq = 0;
    else if (q >= 511) unq = 0x7fff;
    else unq = (int32_t)(((uint32_t)q << 15) + 0x4000u) >> 9;
    if (sign) unq = -unq;
    if (unq < 0) return -(((-unq) * 31) >> 5);
    return (unq * 31) >> 5;
}

static uint32_t tc_bc6h_err3_mag(const uint32_t a[3], uint32_t r, uint32_t g,
                                 uint32_t b) {
    int32_t dr = (int32_t)a[0] - (int32_t)r;
    int32_t dg = (int32_t)a[1] - (int32_t)g;
    int32_t db = (int32_t)a[2] - (int32_t)b;
    return (uint32_t)(dr * dr + dg * dg + db * db);
}

static uint32_t tc_bc6h_err3_smag(const int32_t a[3], int32_t r, int32_t g,
                                  int32_t b) {
    int64_t dr = (int64_t)a[0] - (int64_t)r;
    int64_t dg = (int64_t)a[1] - (int64_t)g;
    int64_t db = (int64_t)a[2] - (int64_t)b;
    uint64_t e = (uint64_t)(dr * dr + dg * dg + db * db);
    return e > UINT_MAX ? UINT_MAX : (uint32_t)e;
}

/* Generic N-bit signed pack: clamp to [-2^(N-1), 2^(N-1)-1] and return 2's complement. */
static uint32_t tc_bc6h_pack_signed_n(int32_t v, uint32_t bits) {
    int32_t half = (int32_t)(1u << (bits - 1u));
    if (v < -half) v = -half;
    if (v > half - 1) v = half - 1;
    return (uint32_t)(v & ((1u << bits) - 1u));
}

static uint32_t tc_bc6h_pack_signed10(int32_t q) {
    if (q < -512) q = -512;
    if (q > 511) q = 511;
    return (uint32_t)q & 1023u;
}

/* Build the 16-entry interpolated palette (magnitude domain) into SoA arrays. */
static void tc_bc6h_pal16_uf16(const uint32_t lo[3], const uint32_t hi[3],
                               int32_t pr[16], int32_t pg[16], int32_t pb[16]) {
    uint32_t s;
    for (s = 0; s < 16u; ++s) {
        uint32_t w = tc_bc7_weights4[s];
        pr[s] = (int32_t)tc_bc6h_unquant_uf16_to_mag(
            ((64u - w) * lo[0] + w * hi[0] + 32u) >> 6);
        pg[s] = (int32_t)tc_bc6h_unquant_uf16_to_mag(
            ((64u - w) * lo[1] + w * hi[1] + 32u) >> 6);
        pb[s] = (int32_t)tc_bc6h_unquant_uf16_to_mag(
            ((64u - w) * lo[2] + w * hi[2] + 32u) >> 6);
    }
}

static uint64_t tc_bc6h_choose_selectors_uf16_scalar(const uint32_t target[16][3],
                                                     const uint32_t lo[3],
                                                     const uint32_t hi[3],
                                                     uint8_t sel[16]) {
    int32_t pr[16], pg[16], pb[16];
    uint64_t err = 0;
    uint32_t i, s;
    tc_bc6h_pal16_uf16(lo, hi, pr, pg, pb);
    for (i = 0; i < 16u; ++i) {
        uint32_t best = 0, best_err = UINT_MAX;
        for (s = 0; s < 16u; ++s) {
            uint32_t e = tc_bc6h_err3_mag(target[i], (uint32_t)pr[s],
                                          (uint32_t)pg[s], (uint32_t)pb[s]);
            if (e < best_err) {
                best_err = e;
                best = s;
            }
        }
        sel[i] = (uint8_t)best;
        err += best_err;
    }
    return err;
}

#if defined(TC_X86)
/* SSE4.1: four texels at a time; magnitudes fit int32 and the 3-channel error
 * fits uint32, so this is bit-identical to the scalar path (same errors, same
 * first-min tie-break). */
TC_TARGET("sse4.1")
static uint64_t tc_bc6h_choose_selectors_uf16_sse41(const uint32_t target[16][3],
                                                    const uint32_t lo[3],
                                                    const uint32_t hi[3],
                                                    uint8_t sel[16]) {
    int32_t pr[16], pg[16], pb[16];
    uint64_t err = 0;
    uint32_t i, s;
    tc_bc6h_pal16_uf16(lo, hi, pr, pg, pb);
    for (i = 0; i < 16u; i += 4u) {
        int32_t ts[4][3];
        __m128i tr, tg, tb, best = _mm_set1_epi32(-1), bsel = _mm_setzero_si128();
        uint32_t k, eb[4], sb[4];
        for (k = 0; k < 4u; ++k) {
            ts[k][0] = (int32_t)target[i + k][0];
            ts[k][1] = (int32_t)target[i + k][1];
            ts[k][2] = (int32_t)target[i + k][2];
        }
        tr = _mm_set_epi32(ts[3][0], ts[2][0], ts[1][0], ts[0][0]);
        tg = _mm_set_epi32(ts[3][1], ts[2][1], ts[1][1], ts[0][1]);
        tb = _mm_set_epi32(ts[3][2], ts[2][2], ts[1][2], ts[0][2]);
        for (s = 0; s < 16u; ++s) {
            __m128i dr = _mm_sub_epi32(tr, _mm_set1_epi32(pr[s]));
            __m128i dg = _mm_sub_epi32(tg, _mm_set1_epi32(pg[s]));
            __m128i db = _mm_sub_epi32(tb, _mm_set1_epi32(pb[s]));
            __m128i e = _mm_add_epi32(
                _mm_add_epi32(_mm_mullo_epi32(dr, dr), _mm_mullo_epi32(dg, dg)),
                _mm_mullo_epi32(db, db));
            __m128i nm = _mm_min_epu32(best, e);
            __m128i lt = _mm_andnot_si128(_mm_cmpeq_epi32(e, best),
                                          _mm_cmpeq_epi32(nm, e));
            bsel = _mm_or_si128(_mm_and_si128(lt, _mm_set1_epi32((int)s)),
                                _mm_andnot_si128(lt, bsel));
            best = nm;
        }
        _mm_storeu_si128((__m128i *)eb, best);
        _mm_storeu_si128((__m128i *)sb, bsel);
        for (k = 0; k < 4u; ++k) {
            sel[i + k] = (uint8_t)sb[k];
            err += eb[k];
        }
    }
    return err;
}

/* AVX2: eight texels at a time (two groups for the 16-texel block). */
TC_TARGET("avx2")
static uint64_t tc_bc6h_choose_selectors_uf16_avx2(const uint32_t target[16][3],
                                                   const uint32_t lo[3],
                                                   const uint32_t hi[3],
                                                   uint8_t sel[16]) {
    int32_t pr[16], pg[16], pb[16];
    uint64_t err = 0;
    uint32_t i, s;
    tc_bc6h_pal16_uf16(lo, hi, pr, pg, pb);
    for (i = 0; i < 16u; i += 8u) {
        __m256i tr, tg, tb, best = _mm256_set1_epi32(-1),
                            bsel = _mm256_setzero_si256();
        uint32_t k, eb[8], sb[8];
        tr = _mm256_set_epi32((int)target[i + 7][0], (int)target[i + 6][0],
                              (int)target[i + 5][0], (int)target[i + 4][0],
                              (int)target[i + 3][0], (int)target[i + 2][0],
                              (int)target[i + 1][0], (int)target[i][0]);
        tg = _mm256_set_epi32((int)target[i + 7][1], (int)target[i + 6][1],
                              (int)target[i + 5][1], (int)target[i + 4][1],
                              (int)target[i + 3][1], (int)target[i + 2][1],
                              (int)target[i + 1][1], (int)target[i][1]);
        tb = _mm256_set_epi32((int)target[i + 7][2], (int)target[i + 6][2],
                              (int)target[i + 5][2], (int)target[i + 4][2],
                              (int)target[i + 3][2], (int)target[i + 2][2],
                              (int)target[i + 1][2], (int)target[i][2]);
        for (s = 0; s < 16u; ++s) {
            __m256i dr = _mm256_sub_epi32(tr, _mm256_set1_epi32(pr[s]));
            __m256i dg = _mm256_sub_epi32(tg, _mm256_set1_epi32(pg[s]));
            __m256i db = _mm256_sub_epi32(tb, _mm256_set1_epi32(pb[s]));
            __m256i e = _mm256_add_epi32(
                _mm256_add_epi32(_mm256_mullo_epi32(dr, dr),
                                 _mm256_mullo_epi32(dg, dg)),
                _mm256_mullo_epi32(db, db));
            __m256i nm = _mm256_min_epu32(best, e);
            __m256i lt = _mm256_andnot_si256(_mm256_cmpeq_epi32(e, best),
                                             _mm256_cmpeq_epi32(nm, e));
            bsel = _mm256_or_si256(_mm256_and_si256(lt, _mm256_set1_epi32((int)s)),
                                   _mm256_andnot_si256(lt, bsel));
            best = nm;
        }
        _mm256_storeu_si256((__m256i *)eb, best);
        _mm256_storeu_si256((__m256i *)sb, bsel);
        for (k = 0; k < 8u; ++k) {
            sel[i + k] = (uint8_t)sb[k];
            err += eb[k];
        }
    }
    return err;
}
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
static uint64_t tc_bc6h_choose_selectors_uf16_neon(const uint32_t target[16][3],
                                                   const uint32_t lo[3],
                                                   const uint32_t hi[3],
                                                   uint8_t sel[16]) {
    int32_t pr[16], pg[16], pb[16];
    uint64_t err = 0;
    uint32_t i, s;
    tc_bc6h_pal16_uf16(lo, hi, pr, pg, pb);
    for (i = 0; i < 16u; i += 4u) {
        int32_t tra[4], tga[4], tba[4];
        int32x4_t tr, tg, tb;
        uint32x4_t best = vdupq_n_u32(0xffffffffu), bsel = vdupq_n_u32(0);
        uint32_t k, eb[4], sb[4];
        for (k = 0; k < 4u; ++k) {
            tra[k] = (int32_t)target[i + k][0];
            tga[k] = (int32_t)target[i + k][1];
            tba[k] = (int32_t)target[i + k][2];
        }
        tr = vld1q_s32(tra);
        tg = vld1q_s32(tga);
        tb = vld1q_s32(tba);
        for (s = 0; s < 16u; ++s) {
            int32x4_t dr = vsubq_s32(tr, vdupq_n_s32(pr[s]));
            int32x4_t dg = vsubq_s32(tg, vdupq_n_s32(pg[s]));
            int32x4_t db = vsubq_s32(tb, vdupq_n_s32(pb[s]));
            uint32x4_t e = vreinterpretq_u32_s32(vaddq_s32(
                vaddq_s32(vmulq_s32(dr, dr), vmulq_s32(dg, dg)), vmulq_s32(db, db)));
            uint32x4_t lt = vcltq_u32(e, best);
            bsel = vbslq_u32(lt, vdupq_n_u32(s), bsel);
            best = vminq_u32(best, e);
        }
        vst1q_u32(eb, best);
        vst1q_u32(sb, bsel);
        for (k = 0; k < 4u; ++k) {
            sel[i + k] = (uint8_t)sb[k];
            err += eb[k];
        }
    }
    return err;
}
#endif

static uint64_t tc_bc6h_choose_selectors_uf16(const uint32_t target[16][3],
                                              const uint32_t lo[3],
                                              const uint32_t hi[3],
                                              uint8_t sel[16]) {
#if defined(TC_X86)
    if (tc_cpu_caps() & TC_CPU_AVX2)
        return tc_bc6h_choose_selectors_uf16_avx2(target, lo, hi, sel);
    if (tc_cpu_caps() & TC_CPU_SSE41)
        return tc_bc6h_choose_selectors_uf16_sse41(target, lo, hi, sel);
#endif
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    if (tc_cpu_caps() & TC_CPU_NEON)
        return tc_bc6h_choose_selectors_uf16_neon(target, lo, hi, sel);
#endif
    return tc_bc6h_choose_selectors_uf16_scalar(target, lo, hi, sel);
}

/* Signed palette (sign-magnitude domain) into SoA arrays. */
static void tc_bc6h_pal16_sf16(const int32_t lo[3], const int32_t hi[3],
                               int32_t pr[16], int32_t pg[16], int32_t pb[16]) {
    uint32_t s, c;
    for (s = 0; s < 16u; ++s) {
        uint32_t w = tc_bc7_weights4[s];
        int32_t *P[3];
        P[0] = pr;
        P[1] = pg;
        P[2] = pb;
        for (c = 0; c < 3u; ++c) {
            int32_t qv = (int32_t)(((int64_t)(64u - w) * lo[c] +
                                    (int64_t)w * hi[c] + 32) >> 6);
            P[c][s] = tc_bc6h_unquant_sf16_to_smag(qv);
        }
    }
}

static uint64_t tc_bc6h_choose_selectors_sf16_scalar(const int32_t target[16][3],
                                                     const int32_t lo[3],
                                                     const int32_t hi[3],
                                                     uint8_t sel[16]) {
    int32_t pr[16], pg[16], pb[16];
    uint64_t err = 0;
    uint32_t i, s;
    tc_bc6h_pal16_sf16(lo, hi, pr, pg, pb);
    for (i = 0; i < 16u; ++i) {
        uint32_t best = 0, best_err = UINT_MAX;
        int32_t a[3];
        a[0] = target[i][0];
        a[1] = target[i][1];
        a[2] = target[i][2];
        for (s = 0; s < 16u; ++s) {
            uint32_t e = tc_bc6h_err3_smag(a, pr[s], pg[s], pb[s]);
            if (e < best_err) {
                best_err = e;
                best = s;
            }
        }
        sel[i] = (uint8_t)best;
        err += best_err;
    }
    return err;
}

#if defined(TC_X86)
/* Signed error can exceed 32 bits (magnitudes span +/-31743, so dr up to
 * 63486), and the scalar saturates the sum at UINT_MAX. Replicate that with
 * step-wise unsigned-overflow detection, which is exactly equivalent to the
 * scalar's int64-sum-then-clamp, keeping every backend bit-identical. */
TC_TARGET("sse4.1")
static uint64_t tc_bc6h_choose_selectors_sf16_sse41(const int32_t target[16][3],
                                                    const int32_t lo[3],
                                                    const int32_t hi[3],
                                                    uint8_t sel[16]) {
    int32_t pr[16], pg[16], pb[16];
    const __m128i ones = _mm_set1_epi32(-1);
    uint64_t err = 0;
    uint32_t i, s;
    tc_bc6h_pal16_sf16(lo, hi, pr, pg, pb);
    for (i = 0; i < 16u; i += 4u) {
        __m128i tr = _mm_set_epi32((int)target[i + 3][0], (int)target[i + 2][0],
                                   (int)target[i + 1][0], (int)target[i][0]);
        __m128i tg = _mm_set_epi32((int)target[i + 3][1], (int)target[i + 2][1],
                                   (int)target[i + 1][1], (int)target[i][1]);
        __m128i tb = _mm_set_epi32((int)target[i + 3][2], (int)target[i + 2][2],
                                   (int)target[i + 1][2], (int)target[i][2]);
        __m128i best = ones, bsel = _mm_setzero_si128();
        uint32_t k, eb[4], sb[4];
        for (s = 0; s < 16u; ++s) {
            __m128i dr = _mm_sub_epi32(tr, _mm_set1_epi32(pr[s]));
            __m128i dg = _mm_sub_epi32(tg, _mm_set1_epi32(pg[s]));
            __m128i db = _mm_sub_epi32(tb, _mm_set1_epi32(pb[s]));
            __m128i er = _mm_mullo_epi32(dr, dr), eg = _mm_mullo_epi32(dg, dg),
                    eb2 = _mm_mullo_epi32(db, db);
            __m128i s1 = _mm_add_epi32(er, eg), o1, e, o2, nm, lt;
            o1 = _mm_andnot_si128(_mm_cmpeq_epi32(s1, er),
                                  _mm_cmpeq_epi32(_mm_min_epu32(s1, er), s1));
            s1 = _mm_or_si128(_mm_and_si128(o1, ones), _mm_andnot_si128(o1, s1));
            e = _mm_add_epi32(s1, eb2);
            o2 = _mm_andnot_si128(_mm_cmpeq_epi32(e, s1),
                                  _mm_cmpeq_epi32(_mm_min_epu32(e, s1), e));
            e = _mm_or_si128(_mm_and_si128(o2, ones), _mm_andnot_si128(o2, e));
            nm = _mm_min_epu32(best, e);
            lt = _mm_andnot_si128(_mm_cmpeq_epi32(e, best),
                                  _mm_cmpeq_epi32(nm, e));
            bsel = _mm_or_si128(_mm_and_si128(lt, _mm_set1_epi32((int)s)),
                                _mm_andnot_si128(lt, bsel));
            best = nm;
        }
        _mm_storeu_si128((__m128i *)eb, best);
        _mm_storeu_si128((__m128i *)sb, bsel);
        for (k = 0; k < 4u; ++k) {
            sel[i + k] = (uint8_t)sb[k];
            err += eb[k];
        }
    }
    return err;
}

TC_TARGET("avx2")
static uint64_t tc_bc6h_choose_selectors_sf16_avx2(const int32_t target[16][3],
                                                   const int32_t lo[3],
                                                   const int32_t hi[3],
                                                   uint8_t sel[16]) {
    int32_t pr[16], pg[16], pb[16];
    const __m256i ones = _mm256_set1_epi32(-1);
    uint64_t err = 0;
    uint32_t i, s;
    tc_bc6h_pal16_sf16(lo, hi, pr, pg, pb);
    for (i = 0; i < 16u; i += 8u) {
        __m256i tr = _mm256_set_epi32(
            (int)target[i + 7][0], (int)target[i + 6][0], (int)target[i + 5][0],
            (int)target[i + 4][0], (int)target[i + 3][0], (int)target[i + 2][0],
            (int)target[i + 1][0], (int)target[i][0]);
        __m256i tg = _mm256_set_epi32(
            (int)target[i + 7][1], (int)target[i + 6][1], (int)target[i + 5][1],
            (int)target[i + 4][1], (int)target[i + 3][1], (int)target[i + 2][1],
            (int)target[i + 1][1], (int)target[i][1]);
        __m256i tb = _mm256_set_epi32(
            (int)target[i + 7][2], (int)target[i + 6][2], (int)target[i + 5][2],
            (int)target[i + 4][2], (int)target[i + 3][2], (int)target[i + 2][2],
            (int)target[i + 1][2], (int)target[i][2]);
        __m256i best = ones, bsel = _mm256_setzero_si256();
        uint32_t k, eb[8], sb[8];
        for (s = 0; s < 16u; ++s) {
            __m256i dr = _mm256_sub_epi32(tr, _mm256_set1_epi32(pr[s]));
            __m256i dg = _mm256_sub_epi32(tg, _mm256_set1_epi32(pg[s]));
            __m256i db = _mm256_sub_epi32(tb, _mm256_set1_epi32(pb[s]));
            __m256i er = _mm256_mullo_epi32(dr, dr),
                    eg = _mm256_mullo_epi32(dg, dg),
                    eb2 = _mm256_mullo_epi32(db, db);
            __m256i s1 = _mm256_add_epi32(er, eg), o1, e, o2, nm, lt;
            o1 = _mm256_andnot_si256(
                _mm256_cmpeq_epi32(s1, er),
                _mm256_cmpeq_epi32(_mm256_min_epu32(s1, er), s1));
            s1 = _mm256_or_si256(_mm256_and_si256(o1, ones),
                                 _mm256_andnot_si256(o1, s1));
            e = _mm256_add_epi32(s1, eb2);
            o2 = _mm256_andnot_si256(
                _mm256_cmpeq_epi32(e, s1),
                _mm256_cmpeq_epi32(_mm256_min_epu32(e, s1), e));
            e = _mm256_or_si256(_mm256_and_si256(o2, ones),
                                _mm256_andnot_si256(o2, e));
            nm = _mm256_min_epu32(best, e);
            lt = _mm256_andnot_si256(_mm256_cmpeq_epi32(e, best),
                                     _mm256_cmpeq_epi32(nm, e));
            bsel = _mm256_or_si256(
                _mm256_and_si256(lt, _mm256_set1_epi32((int)s)),
                _mm256_andnot_si256(lt, bsel));
            best = nm;
        }
        _mm256_storeu_si256((__m256i *)eb, best);
        _mm256_storeu_si256((__m256i *)sb, bsel);
        for (k = 0; k < 8u; ++k) {
            sel[i + k] = (uint8_t)sb[k];
            err += eb[k];
        }
    }
    return err;
}
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
static uint64_t tc_bc6h_choose_selectors_sf16_neon(const int32_t target[16][3],
                                                   const int32_t lo[3],
                                                   const int32_t hi[3],
                                                   uint8_t sel[16]) {
    int32_t pr[16], pg[16], pb[16];
    const uint32x4_t ones = vdupq_n_u32(0xffffffffu);
    uint64_t err = 0;
    uint32_t i, s;
    tc_bc6h_pal16_sf16(lo, hi, pr, pg, pb);
    for (i = 0; i < 16u; i += 4u) {
        int32_t tra[4], tga[4], tba[4];
        int32x4_t tr, tg, tb;
        uint32x4_t best = ones, bsel = vdupq_n_u32(0);
        uint32_t k, eb[4], sb[4];
        for (k = 0; k < 4u; ++k) {
            tra[k] = target[i + k][0];
            tga[k] = target[i + k][1];
            tba[k] = target[i + k][2];
        }
        tr = vld1q_s32(tra);
        tg = vld1q_s32(tga);
        tb = vld1q_s32(tba);
        for (s = 0; s < 16u; ++s) {
            int32x4_t dr = vsubq_s32(tr, vdupq_n_s32(pr[s]));
            int32x4_t dg = vsubq_s32(tg, vdupq_n_s32(pg[s]));
            int32x4_t db = vsubq_s32(tb, vdupq_n_s32(pb[s]));
            uint32x4_t er = vreinterpretq_u32_s32(vmulq_s32(dr, dr));
            uint32x4_t eg = vreinterpretq_u32_s32(vmulq_s32(dg, dg));
            uint32x4_t eb2 = vreinterpretq_u32_s32(vmulq_s32(db, db));
            uint32x4_t s1 = vaddq_u32(er, eg), e, lt;
            s1 = vbslq_u32(vcltq_u32(s1, er), ones, s1); /* saturate on overflow */
            e = vaddq_u32(s1, eb2);
            e = vbslq_u32(vcltq_u32(e, s1), ones, e);
            lt = vcltq_u32(e, best);
            bsel = vbslq_u32(lt, vdupq_n_u32(s), bsel);
            best = vminq_u32(best, e);
        }
        vst1q_u32(eb, best);
        vst1q_u32(sb, bsel);
        for (k = 0; k < 4u; ++k) {
            sel[i + k] = (uint8_t)sb[k];
            err += eb[k];
        }
    }
    return err;
}
#endif

static uint64_t tc_bc6h_choose_selectors_sf16(const int32_t target[16][3],
                                              const int32_t lo[3],
                                              const int32_t hi[3],
                                              uint8_t sel[16]) {
#if defined(TC_X86)
    if (tc_cpu_caps() & TC_CPU_AVX2)
        return tc_bc6h_choose_selectors_sf16_avx2(target, lo, hi, sel);
    if (tc_cpu_caps() & TC_CPU_SSE41)
        return tc_bc6h_choose_selectors_sf16_sse41(target, lo, hi, sel);
#endif
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    if (tc_cpu_caps() & TC_CPU_NEON)
        return tc_bc6h_choose_selectors_sf16_neon(target, lo, hi, sel);
#endif
    return tc_bc6h_choose_selectors_sf16_scalar(target, lo, hi, sel);
}

/* Sign-aware rounded integer division (den > 0). */
static int64_t tc_bc6h_rdiv(int64_t num, int64_t den) {
    return num >= 0 ? (num + den / 2) / den : -((-num + den / 2) / den);
}

/* Least-squares endpoint refinement for mode 11: given the current selectors,
 * re-solve each channel's two 10-bit endpoints (2x2 normal equations in the
 * quantized domain) so the interpolated palette best fits the block, recompute
 * the selectors, and keep the result while the (magnitude-domain) error drops.
 * `maxq` is the endpoint clamp (1023 for uf16, +/-511 for sf16 handled by the
 * signed variant). Returns the best error found. */
static uint64_t tc_bc6h_refine_uf16(const uint32_t q[16][3],
                                    const uint32_t target[16][3], uint32_t lo[3],
                                    uint32_t hi[3], uint8_t sel[16]) {
    uint64_t best = tc_bc6h_choose_selectors_uf16(target, lo, hi, sel);
    int round;
    for (round = 0; round < 3; ++round) {
        uint32_t nlo[3], nhi[3];
        uint8_t nsel[16];
        uint64_t e;
        uint32_t c, i;
        for (c = 0; c < 3u; ++c) {
            int64_t saa = 0, sab = 0, sbb = 0, sap = 0, sbp = 0, det, l, h;
            for (i = 0; i < 16u; ++i) {
                int64_t b = tc_bc7_weights4[sel[i]], a = 64 - b, p = q[i][c];
                saa += a * a;
                sab += a * b;
                sbb += b * b;
                sap += a * p;
                sbp += b * p;
            }
            det = saa * sbb - sab * sab;
            if (det <= 0) {
                nlo[c] = lo[c];
                nhi[c] = hi[c];
                continue;
            }
            l = tc_bc6h_rdiv((sap * sbb - sbp * sab) * 64, det);
            h = tc_bc6h_rdiv((sbp * saa - sap * sab) * 64, det);
            if (l < 0) l = 0;
            if (l > 1023) l = 1023;
            if (h < 0) h = 0;
            if (h > 1023) h = 1023;
            nlo[c] = (uint32_t)l;
            nhi[c] = (uint32_t)h;
        }
        e = tc_bc6h_choose_selectors_uf16(target, nlo, nhi, nsel);
        if (e < best) {
            best = e;
            memcpy(lo, nlo, sizeof(nlo));
            memcpy(hi, nhi, sizeof(nhi));
            memcpy(sel, nsel, 16u);
        } else {
            break;
        }
    }
    return best;
}

/* Signed (sf16) endpoint refinement, clamped to the [-511,511] mode-11 range. */
static uint64_t tc_bc6h_refine_sf16(const int32_t q[16][3],
                                    const int32_t target[16][3], int32_t lo[3],
                                    int32_t hi[3], uint8_t sel[16]) {
    uint64_t best = tc_bc6h_choose_selectors_sf16(target, lo, hi, sel);
    int round;
    for (round = 0; round < 3; ++round) {
        int32_t nlo[3], nhi[3];
        uint8_t nsel[16];
        uint64_t e;
        uint32_t c, i;
        for (c = 0; c < 3u; ++c) {
            int64_t saa = 0, sab = 0, sbb = 0, sap = 0, sbp = 0, det, l, h;
            for (i = 0; i < 16u; ++i) {
                int64_t b = tc_bc7_weights4[sel[i]], a = 64 - b, p = q[i][c];
                saa += a * a;
                sab += a * b;
                sbb += b * b;
                sap += a * p;
                sbp += b * p;
            }
            det = saa * sbb - sab * sab;
            if (det <= 0) {
                nlo[c] = lo[c];
                nhi[c] = hi[c];
                continue;
            }
            l = tc_bc6h_rdiv((sap * sbb - sbp * sab) * 64, det);
            h = tc_bc6h_rdiv((sbp * saa - sap * sab) * 64, det);
            if (l < -511) l = -511;
            if (l > 511) l = 511;
            if (h < -511) h = -511;
            if (h > 511) h = 511;
            nlo[c] = (int32_t)l;
            nhi[c] = (int32_t)h;
        }
        e = tc_bc6h_choose_selectors_sf16(target, nlo, nhi, nsel);
        if (e < best) {
            best = e;
            memcpy(lo, nlo, sizeof(nlo));
            memcpy(hi, nhi, sizeof(nhi));
            memcpy(sel, nsel, 16u);
        } else {
            break;
        }
    }
    return best;
}

/* --- BC6H mode 9 (two-region, 6.666) --------------------------------------
 * 32 two-region partitions; each region has its own 6-bit endpoint pair and
 * 3-bit indices. The block stores region-0 endpoint-0 absolute and the other
 * three endpoints as 6-bit deltas from it, wrapped mod 64 -- so every pair is
 * representable (unlike the higher-base 2-region modes whose small deltas only
 * reach part of the range). Coarser endpoints than mode 11 but two regions, so
 * it wins on blocks split across a colour/luma boundary that a single line
 * cannot fit. Values map exactly to bcdec's mode-9 decode. */
static const uint8_t tc_bc6h_part2[32][16] = {
    {0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1}, {0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
    {0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1}, {0,0,0,1,0,0,1,1,0,0,1,1,0,1,1,1},
    {0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,1}, {0,0,1,1,0,1,1,1,0,1,1,1,1,1,1,1},
    {0,0,0,1,0,0,1,1,0,1,1,1,1,1,1,1}, {0,0,0,0,0,0,0,1,0,0,1,1,0,1,1,1},
    {0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1}, {0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1}, {0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1},
    {0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1}, {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
    {0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1}, {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
    {0,0,0,0,1,0,0,0,1,1,1,0,1,1,1,1}, {0,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,0,0,0,1,1,1,0}, {0,1,1,1,0,0,1,1,0,0,0,1,0,0,0,0},
    {0,0,1,1,0,0,0,1,0,0,0,0,0,0,0,0}, {0,0,0,0,1,0,0,0,1,1,0,0,1,1,1,0},
    {0,0,0,0,0,0,0,0,1,0,0,0,1,1,0,0}, {0,1,1,1,0,0,1,1,0,0,1,1,0,0,0,1},
    {0,0,1,1,0,0,0,1,0,0,0,1,0,0,0,0}, {0,0,0,0,1,0,0,0,1,0,0,0,1,1,0,0},
    {0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0}, {0,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0},
    {0,0,0,1,0,1,1,1,1,1,1,0,1,0,0,0}, {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
    {0,1,1,1,0,0,0,1,1,0,0,0,1,1,1,0}, {0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0},
};
static const uint8_t tc_bc6h_part2_anchor[32] = {
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,2,8,2,2,8,8,15,2,8,2,2,8,8,2,2};

static uint32_t tc_bc6h_quant_uf16_n(float f, uint32_t bits) {
    uint16_t h;
    uint32_t mag;
    if (!(f > 0.0f)) return 0;
    h = tc_float_to_half_bits(f);
    mag = h & 0x7fffu;
    if (mag > 0x7bffu) mag = 0x7bffu;
    return (mag * ((1u << bits) - 1u) + 15871u) / 31743u;
}
static uint32_t tc_bc6h_unquant_n_to_mag(uint32_t q, uint32_t bits) {
    uint32_t unq;
    if (q == 0u) unq = 0u;
    else if (q >= (1u << bits) - 1u) unq = 0xffffu;
    else unq = ((q << 16) + 0x8000u) >> bits;
    return (unq * 31u) >> 6;
}

/* Signed N-bit quantizer: half → quantized signed N-bit value. */
static int32_t tc_bc6h_quant_sf16_n(float f, uint32_t bits) {
    uint16_t h;
    uint32_t mag;
    int32_t maxq = (int32_t)((1u << (bits - 1u)) - 1u);
    int32_t q;
    if (f == 0.0f) return 0;
    h = tc_float_to_half_bits(f);
    mag = h & 0x7fffu;
    if (mag > 0x7bffu) mag = 0x7bffu;
    q = (int32_t)((mag * (uint32_t)maxq + 15871u) / 31743u);
    if (q > maxq) q = maxq;
    return (h & 0x8000u) ? -q : q;
}

/* Signed N-bit unquantizer: quantized signed N-bit → signed magnitude. */
static int32_t tc_bc6h_unquant_n_to_smag(int32_t q, uint32_t bits) {
    int32_t maxq = (int32_t)((1u << (bits - 1u)) - 1u);
    int32_t sign = 1, unq;
    if (q < 0) { sign = -1; q = -q; }
    if (q == 0) unq = 0;
    else if (q >= maxq) unq = 0x7fff;
    else unq = (int32_t)((((uint32_t)q << 15) + 0x4000u) >> (bits - 1u));
    if (sign < 0) unq = -unq;
    return (unq < 0) ? -(((-unq) * 31) >> 5) : (unq * 31) >> 5;
}

/* Per-texel 3-bit selectors + total error for one candidate partition/endpoints
 * (endpoints ep[region*2 + 0/1], 6-bit). */
static uint64_t tc_bc6h_mode9_selectors(const uint32_t q[16][3],
                                        const uint32_t target[16][3],
                                        const uint8_t part[16],
                                        const uint32_t ep[4][3], uint8_t sel[16]) {
    static const uint32_t w3[8] = {0, 9, 18, 27, 37, 46, 55, 64};
    uint32_t pal[2][8][3], r, s, c, i;
    uint64_t err = 0;
    (void)q;
    for (r = 0; r < 2u; ++r)
        for (s = 0; s < 8u; ++s)
            for (c = 0; c < 3u; ++c) {
                uint32_t qv = ((64u - w3[s]) * ep[r * 2][c] + w3[s] * ep[r * 2 + 1][c] +
                               32u) >> 6;
                pal[r][s][c] = tc_bc6h_unquant_n_to_mag(qv, 6);
            }
    for (i = 0; i < 16u; ++i) {
        uint32_t reg = part[i], best = 0, berr = UINT_MAX;
        for (s = 0; s < 8u; ++s) {
            uint32_t e = tc_bc6h_err3_mag(target[i], pal[reg][s][0], pal[reg][s][1],
                                          pal[reg][s][2]);
            if (e < berr) {
                berr = e;
                best = s;
            }
        }
        sel[i] = (uint8_t)best;
        err += berr;
    }
    return err;
}

static void tc_bc6h_w(uint8_t out[16], uint32_t *bp, uint32_t v, uint32_t n) {
    tc_set_bits(out, bp, v, (int)n);
}

/* Per-texel 3-bit selectors for mode 0 (10-bit endpoints + w3 weight table).
 * Like tc_bc6h_mode9_selectors but uses 10-bit unquant (same as mode 11). */
static uint64_t tc_bc6h_mode0_selectors(const uint32_t target[16][3],
                                        const uint8_t part[16],
                                        const uint32_t ep[4][3], uint8_t sel[16]) {
    static const uint32_t w3[8] = {0, 9, 18, 27, 37, 46, 55, 64};
    uint32_t pal[2][8][3], r, s, c, i;
    uint64_t err = 0;
    for (r = 0; r < 2u; ++r)
        for (s = 0; s < 8u; ++s)
            for (c = 0; c < 3u; ++c) {
                uint32_t qv = ((64u - w3[s]) * ep[r * 2][c] + w3[s] * ep[r * 2 + 1][c] + 32u) >> 6;
                pal[r][s][c] = tc_bc6h_unquant_uf16_to_mag(qv);
            }
    for (i = 0; i < 16u; ++i) {
        uint32_t reg = part[i], best = 0, berr = UINT_MAX;
        for (s = 0; s < 8u; ++s) {
            uint32_t e = tc_bc6h_err3_mag(target[i], pal[reg][s][0], pal[reg][s][1], pal[reg][s][2]);
            if (e < berr) { berr = e; best = s; }
        }
        sel[i] = (uint8_t)best;
        err += berr;
    }
    return err;
}

/* Signed variant of mode0 selectors: uses int32_t targets/endpoints and
 * tc_bc6h_err3_smag / tc_bc6h_unquant_sf16_to_smag for palette reconstruction. */
static uint64_t tc_bc6h_mode0_selectors_sf16(const int32_t target[16][3],
                                              const uint8_t part[16],
                                              const int32_t ep[4][3],
                                              uint8_t sel[16]) {
    static const uint32_t w3[8] = {0, 9, 18, 27, 37, 46, 55, 64};
    int32_t pal[2][8][3];
    uint64_t err = 0;
    uint32_t r, s, c, i;
    for (r = 0; r < 2u; ++r)
        for (s = 0; s < 8u; ++s)
            for (c = 0; c < 3u; ++c) {
                int32_t qv = ((int32_t)(64 - w3[s]) * ep[r * 2][c] +
                              (int32_t)w3[s] * ep[r * 2 + 1][c] + 32) >> 6;
                pal[r][s][c] = tc_bc6h_unquant_sf16_to_smag(qv);
            }
    for (i = 0; i < 16u; ++i) {
        uint32_t reg = part[i], best = 0, berr = UINT_MAX;
        for (s = 0; s < 8u; ++s) {
            uint32_t e = tc_bc6h_err3_smag(target[i], pal[reg][s][0],
                                           pal[reg][s][1], pal[reg][s][2]);
            if (e < berr) { berr = e; best = s; }
        }
        sel[i] = (uint8_t)best;
        err += berr;
    }
    return err;
}

/* Signed variant of mode9 selectors. */
static uint64_t tc_bc6h_mode9_selectors_sf16(const int32_t q[16][3],
                                              const int32_t target[16][3],
                                              const uint8_t part[16],
                                              const int32_t ep[4][3],
                                              uint8_t sel[16]) {
    static const uint32_t w3[8] = {0, 9, 18, 27, 37, 46, 55, 64};
    int32_t pal[2][8][3];
    uint64_t err = 0;
    uint32_t r, s, c, i;
    (void)q;
    for (r = 0; r < 2u; ++r)
        for (s = 0; s < 8u; ++s)
            for (c = 0; c < 3u; ++c) {
                int32_t qv = ((int32_t)(64 - w3[s]) * ep[r * 2][c] +
                              (int32_t)w3[s] * ep[r * 2 + 1][c] + 32) >> 6;
                pal[r][s][c] = tc_bc6h_unquant_n_to_smag(qv, 6);
            }
    for (i = 0; i < 16u; ++i) {
        uint32_t reg = part[i], best = 0, berr = UINT_MAX;
        for (s = 0; s < 8u; ++s) {
            uint32_t e = tc_bc6h_err3_smag(target[i], pal[reg][s][0],
                                           pal[reg][s][1], pal[reg][s][2]);
            if (e < berr) { berr = e; best = s; }
        }
        sel[i] = (uint8_t)best;
        err += berr;
    }
    return err;
}

/* Pack 5-bit signed delta (range -16..15) into the 5-bit field. */
static uint32_t tc_bc6h_pack_delta5(int32_t d) {
    if (d < -16) d = -16;
    if (d > 15) d = 15;
    return (uint32_t)(d & 31u);
}

/* Encode the block as BC6H mode 0 (two-region, 10-bit primary + 5-bit signed
 * deltas); returns the reconstruction SSE, or UINT64_MAX if no valid partition. */
static uint64_t tc_bc6h_mode0_uf16(const float pix[16][3], uint8_t out[16]) {
    uint32_t q[16][3], target[16][3], i, c, p;
    uint32_t bep[4][3];
    uint8_t bsel[16];
    int best_p = -1;
    uint64_t best_err = (uint64_t)-1;
    uint32_t bitpos = 0, anchor;
    int r;
    for (i = 0; i < 16u; ++i)
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_uf16(pix[i][c]);
            target[i][c] = tc_bc6h_unquant_uf16_to_mag(q[i][c]);
        }
    /* Partition search: same TOPK prefilter as mode 9. */
    {
        enum { TOPK = 5u };
        uint32_t cand[TOPK], k, ncand = 0;
        uint64_t cscore[TOPK];
        for (p = 0; p < 32u; ++p) {
            uint32_t rlo[2][3], rhi[2][3];
            uint64_t score = 0;
            int have[2] = {0, 0};
            for (c = 0; c < 3u; ++c) {
                rlo[0][c] = rlo[1][c] = 1023u;
                rhi[0][c] = rhi[1][c] = 0u;
            }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
                have[reg] = 1;
            }
            if (!have[0] || !have[1]) continue;
            for (c = 0; c < 3u; ++c) {
                uint64_t e0 = rhi[0][c] - rlo[0][c], e1 = rhi[1][c] - rlo[1][c];
                score += e0 * e0 + e1 * e1;
            }
            if (ncand < TOPK) {
                cand[ncand] = p;
                cscore[ncand] = score;
                ++ncand;
            } else {
                uint32_t worst = 0;
                for (k = 1; k < TOPK; ++k)
                    if (cscore[k] > cscore[worst]) worst = k;
                if (score < cscore[worst]) { cand[worst] = p; cscore[worst] = score; }
            }
        }
        for (k = 0; k < ncand; ++k) {
            uint32_t ep[4][3], rlo[2][3], rhi[2][3];
            uint8_t sel[16];
            uint64_t err;
            int fit = 1;
            p = cand[k];
            for (c = 0; c < 3u; ++c) {
                rlo[0][c] = rlo[1][c] = 1023u;
                rhi[0][c] = rhi[1][c] = 0u;
            }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
            }
            /* Mode 0 encodes all endpoints as 5-bit signed deltas from rlo[0]
             * (region-0 low endpoint). Check all three deltas fit in [-16,15]. */
            for (c = 0; c < 3u && fit; ++c) {
                int32_t d1 = (int32_t)rhi[0][c] - (int32_t)rlo[0][c];
                int32_t d2 = (int32_t)rlo[1][c] - (int32_t)rlo[0][c];
                int32_t d3 = (int32_t)rhi[1][c] - (int32_t)rlo[0][c];
                if (d1 < -16 || d1 > 15 || d2 < -16 || d2 > 15 || d3 < -16 || d3 > 15)
                    fit = 0;
            }
            if (!fit) continue;
            for (c = 0; c < 3u; ++c) {
                ep[0][c] = rlo[0][c];
                ep[1][c] = rhi[0][c];
                ep[2][c] = rlo[1][c];
                ep[3][c] = rhi[1][c];
            }
            err = tc_bc6h_mode0_selectors(target, tc_bc6h_part2[p], ep, sel);
            if (err < best_err) {
                best_err = err;
                best_p = (int)p;
                memcpy(bep, ep, sizeof(ep));
                memcpy(bsel, sel, 16u);
            }
        }
    }
    if (best_p < 0) return (uint64_t)-1;

    /* Per-region least-squares refinement of the winning partition. */
    {
        static const uint32_t w3[8] = {0, 9, 18, 27, 37, 46, 55, 64};
        int round, rr;
        for (round = 0; round < 2; ++round) {
            uint32_t nep[4][3];
            uint8_t nsel[16];
            uint64_t e;
            int fit = 1;
            for (rr = 0; rr < 2; ++rr)
                for (c = 0; c < 3u; ++c) {
                    int64_t saa = 0, sab = 0, sbb = 0, sap = 0, sbp = 0, det, l, h;
                    for (i = 0; i < 16u; ++i) {
                        int64_t bb, a, pp;
                        if ((int)tc_bc6h_part2[best_p][i] != rr) continue;
                        bb = w3[bsel[i]];
                        a = 64 - bb;
                        pp = q[i][c];
                        saa += a * a;
                        sab += a * bb;
                        sbb += bb * bb;
                        sap += a * pp;
                        sbp += bb * pp;
                    }
                    det = saa * sbb - sab * sab;
                    if (det <= 0) {
                        nep[rr * 2][c] = bep[rr * 2][c];
                        nep[rr * 2 + 1][c] = bep[rr * 2 + 1][c];
                        continue;
                    }
                    l = tc_bc6h_rdiv((sap * sbb - sbp * sab) * 64, det);
                    h = tc_bc6h_rdiv((sbp * saa - sap * sab) * 64, det);
                    if (l < 0) l = 0;
                    if (l > 1023) l = 1023;
                    if (h < 0) h = 0;
                    if (h > 1023) h = 1023;
                    nep[rr * 2][c] = (uint32_t)l;
                    nep[rr * 2 + 1][c] = (uint32_t)h;
                }
            /* After refinement, re-check delta fit against rlo[0]. */
            for (c = 0; c < 3u && fit; ++c) {
                int32_t d1 = (int32_t)nep[1][c] - (int32_t)nep[0][c];
                int32_t d2 = (int32_t)nep[2][c] - (int32_t)nep[0][c];
                int32_t d3 = (int32_t)nep[3][c] - (int32_t)nep[0][c];
                if (d1 < -16 || d1 > 15 || d2 < -16 || d2 > 15 || d3 < -16 || d3 > 15)
                    fit = 0;
            }
            if (!fit) break;
            e = tc_bc6h_mode0_selectors(target, tc_bc6h_part2[best_p], nep, nsel);
            if (e < best_err) {
                best_err = e;
                memcpy(bep, nep, sizeof(nep));
                memcpy(bsel, nsel, 16u);
            } else {
                break;
            }
        }
    }
    anchor = tc_bc6h_part2_anchor[best_p];

    /* Anchor fix-up: region anchor's MSB (bit 2 for 3-bit indices) must be 0. */
    for (r = 0; r < 2; ++r) {
        uint32_t at = (r == 0) ? 0u : anchor;
        if (bsel[at] & 4u) {
            for (c = 0; c < 3u; ++c) {
                uint32_t t = bep[r * 2][c];
                bep[r * 2][c] = bep[r * 2 + 1][c];
                bep[r * 2 + 1][c] = t;
            }
            for (i = 0; i < 16u; ++i)
                if (tc_bc6h_part2[best_p][i] == (uint32_t)r) bsel[i] = (uint8_t)(7u - bsel[i]);
        }
    }

    /* Mode 0 bit layout: first 2 bits of block are 00 (mode ID). */
    memset(out, 0, 16u);
    bitpos = 2;  /* Skip mode field (bits 0-1 = 00) */
    /* g[2] bit 4, b[2] bit 4, b[3] bit 4 (high bits of 5-bit deltas) */
    tc_bc6h_w(out, &bitpos, (bep[2][1] >> 4) & 1u, 1);  /* g2 hi */
    tc_bc6h_w(out, &bitpos, (bep[2][2] >> 4) & 1u, 1);  /* b2 hi */
    tc_bc6h_w(out, &bitpos, (bep[3][2] >> 4) & 1u, 1);  /* b3 hi */
    /* Primary endpoints: r[0], g[0], b[0] = 10-bit */
    tc_bc6h_w(out, &bitpos, bep[0][0], 10);  /* r0 */
    tc_bc6h_w(out, &bitpos, bep[0][1], 10);  /* g0 */
    tc_bc6h_w(out, &bitpos, bep[0][2], 10);  /* b0 */
    /* r[1], g[1], b[1] = 5-bit signed deltas from primary */
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5((int32_t)bep[1][0] - (int32_t)bep[0][0]), 5);  /* r1 */
    tc_bc6h_w(out, &bitpos, (bep[3][1] >> 4) & 1u, 1);  /* g3 hi */
    tc_bc6h_w(out, &bitpos, bep[2][1] & 0xfu, 4);       /* g2 lo */
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5((int32_t)bep[1][1] - (int32_t)bep[0][1]), 5);  /* g1 */
    tc_bc6h_w(out, &bitpos, bep[3][2] & 1u, 1);          /* b3 bit 0 */
    tc_bc6h_w(out, &bitpos, bep[3][1] & 0xfu, 4);        /* g3 lo */
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5((int32_t)bep[1][2] - (int32_t)bep[0][2]), 5);  /* b1 */
    tc_bc6h_w(out, &bitpos, (bep[3][2] >> 1) & 1u, 1);   /* b3 bit 1 */
    tc_bc6h_w(out, &bitpos, bep[2][2] & 0xfu, 4);        /* b2 lo */
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5((int32_t)bep[2][0] - (int32_t)bep[0][0]), 5);  /* r2 */
    tc_bc6h_w(out, &bitpos, (bep[3][2] >> 2) & 1u, 1);   /* b3 bit 2 */
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5((int32_t)bep[3][0] - (int32_t)bep[0][0]), 5);  /* r3 */
    tc_bc6h_w(out, &bitpos, (bep[3][2] >> 3) & 1u, 1);   /* b3 bit 3 */
    tc_bc6h_w(out, &bitpos, (uint32_t)best_p, 5);         /* partition */
    for (i = 0; i < 16u; ++i) {
        uint32_t nb = (i == 0u || i == anchor) ? 2u : 3u;
        tc_bc6h_w(out, &bitpos, bsel[i], nb);
    }
    return best_err;
}

/* Encode the block as BC6H mode 1 (two-region, 7-bit primary + 6-bit signed
 * deltas, range ±32); returns reconstruction SSE or UINT64_MAX if no fit. */
static uint64_t tc_bc6h_mode1_uf16(const float pix[16][3], uint8_t out[16]) {
    uint32_t q[16][3], target[16][3], i, c, p;
    uint32_t bep[4][3];
    uint8_t bsel[16];
    int best_p = -1;
    uint64_t best_err = (uint64_t)-1;
    uint32_t bitpos = 0, anchor;
    int r;
    for (i = 0; i < 16u; ++i)
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_uf16_n(pix[i][c], 7);
            target[i][c] = tc_bc6h_unquant_uf16_to_mag(tc_bc6h_quant_uf16(pix[i][c]));
        }
    /* Partition search: TOPK prefilter. */
    {
        enum { TOPK = 5u };
        uint32_t cand[TOPK], k, ncand = 0;
        uint64_t cscore[TOPK];
        for (p = 0; p < 32u; ++p) {
            uint32_t rlo[2][3], rhi[2][3];
            uint64_t score = 0;
            int have[2] = {0, 0};
            for (c = 0; c < 3u; ++c) {
                rlo[0][c] = rlo[1][c] = 127u;
                rhi[0][c] = rhi[1][c] = 0u;
            }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
                have[reg] = 1;
            }
            if (!have[0] || !have[1]) continue;
            for (c = 0; c < 3u; ++c) {
                uint64_t e0 = rhi[0][c] - rlo[0][c], e1 = rhi[1][c] - rlo[1][c];
                score += e0 * e0 + e1 * e1;
            }
            if (ncand < TOPK) {
                cand[ncand] = p;
                cscore[ncand] = score;
                ++ncand;
            } else {
                uint32_t worst = 0;
                for (k = 1; k < TOPK; ++k)
                    if (cscore[k] > cscore[worst]) worst = k;
                if (score < cscore[worst]) { cand[worst] = p; cscore[worst] = score; }
            }
        }
        for (k = 0; k < ncand; ++k) {
            uint32_t ep[4][3], rlo[2][3], rhi[2][3];
            uint8_t sel[16];
            uint64_t err;
            int fit = 1;
            p = cand[k];
            for (c = 0; c < 3u; ++c) {
                rlo[0][c] = rlo[1][c] = 127u;
                rhi[0][c] = rhi[1][c] = 0u;
            }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
            }
            /* Mode 1: 6-bit signed deltas (-32..31) from rlo[0]. */
            for (c = 0; c < 3u && fit; ++c) {
                int32_t d1 = (int32_t)rhi[0][c] - (int32_t)rlo[0][c];
                int32_t d2 = (int32_t)rlo[1][c] - (int32_t)rlo[0][c];
                int32_t d3 = (int32_t)rhi[1][c] - (int32_t)rlo[0][c];
                if (d1 < -32 || d1 > 31 || d2 < -32 || d2 > 31 || d3 < -32 || d3 > 31)
                    fit = 0;
            }
            if (!fit) continue;
            for (c = 0; c < 3u; ++c) {
                ep[0][c] = rlo[0][c];
                ep[1][c] = rhi[0][c];
                ep[2][c] = rlo[1][c];
                ep[3][c] = rhi[1][c];
            }
            err = tc_bc6h_mode0_selectors(target, tc_bc6h_part2[p], ep, sel);
            if (err < best_err) {
                best_err = err;
                best_p = (int)p;
                memcpy(bep, ep, sizeof(ep));
                memcpy(bsel, sel, 16u);
            }
        }
    }
    if (best_p < 0) return (uint64_t)-1;

    /* Per-region least-squares refinement (clamped to 7-bit). */
    {
        static const uint32_t w3[8] = {0, 9, 18, 27, 37, 46, 55, 64};
        int round, rr;
        for (round = 0; round < 2; ++round) {
            uint32_t nep[4][3];
            uint8_t nsel[16];
            uint64_t e;
            int fit = 1;
            for (rr = 0; rr < 2; ++rr)
                for (c = 0; c < 3u; ++c) {
                    int64_t saa = 0, sab = 0, sbb = 0, sap = 0, sbp = 0, det, l, h;
                    for (i = 0; i < 16u; ++i) {
                        int64_t bb, a, pp;
                        if ((int)tc_bc6h_part2[best_p][i] != rr) continue;
                        bb = w3[bsel[i]];
                        a = 64 - bb;
                        pp = q[i][c];
                        saa += a * a;
                        sab += a * bb;
                        sbb += bb * bb;
                        sap += a * pp;
                        sbp += bb * pp;
                    }
                    det = saa * sbb - sab * sab;
                    if (det <= 0) {
                        nep[rr * 2][c] = bep[rr * 2][c];
                        nep[rr * 2 + 1][c] = bep[rr * 2 + 1][c];
                        continue;
                    }
                    l = tc_bc6h_rdiv((sap * sbb - sbp * sab) * 64, det);
                    h = tc_bc6h_rdiv((sbp * saa - sap * sab) * 64, det);
                    if (l < 0) l = 0;
                    if (l > 127) l = 127;
                    if (h < 0) h = 0;
                    if (h > 127) h = 127;
                    nep[rr * 2][c] = (uint32_t)l;
                    nep[rr * 2 + 1][c] = (uint32_t)h;
                }
            for (c = 0; c < 3u && fit; ++c) {
                int32_t d1 = (int32_t)nep[1][c] - (int32_t)nep[0][c];
                int32_t d2 = (int32_t)nep[2][c] - (int32_t)nep[0][c];
                int32_t d3 = (int32_t)nep[3][c] - (int32_t)nep[0][c];
                if (d1 < -32 || d1 > 31 || d2 < -32 || d2 > 31 || d3 < -32 || d3 > 31)
                    fit = 0;
            }
            if (!fit) break;
            e = tc_bc6h_mode0_selectors(target, tc_bc6h_part2[best_p], nep, nsel);
            if (e < best_err) {
                best_err = e;
                memcpy(bep, nep, sizeof(nep));
                memcpy(bsel, nsel, 16u);
            } else {
                break;
            }
        }
    }
    anchor = tc_bc6h_part2_anchor[best_p];

    /* Anchor fix-up (3-bit indices, MSB = bit 2). */
    for (r = 0; r < 2; ++r) {
        uint32_t at = (r == 0) ? 0u : anchor;
        if (bsel[at] & 4u) {
            for (c = 0; c < 3u; ++c) {
                uint32_t t = bep[r * 2][c];
                bep[r * 2][c] = bep[r * 2 + 1][c];
                bep[r * 2 + 1][c] = t;
            }
            for (i = 0; i < 16u; ++i)
                if (tc_bc6h_part2[best_p][i] == (uint32_t)r) bsel[i] = (uint8_t)(7u - bsel[i]);
        }
    }

    /* Mode 1 bit layout (first 2 bits = 01 = mode ID). */
    memset(out, 0, 16u);
    tc_bc6h_w(out, &bitpos, 1u, 2);
    /* Scattered hi-bit fields, then primaries, then deltas. */
    tc_bc6h_w(out, &bitpos, (bep[2][1] >> 5) & 1u, 1);  /* g2 hi (bit 5) */
    tc_bc6h_w(out, &bitpos, (bep[3][1] >> 4) & 1u, 1);  /* g3 bit 4 */
    tc_bc6h_w(out, &bitpos, (bep[3][1] >> 5) & 1u, 1);  /* g3 hi (bit 5) */
    tc_bc6h_w(out, &bitpos, bep[0][0], 7);               /* r0 primary */
    tc_bc6h_w(out, &bitpos, bep[3][2] & 1u, 1);         /* b3 bit 0 */
    tc_bc6h_w(out, &bitpos, (bep[3][2] >> 1) & 1u, 1);  /* b3 bit 1 */
    tc_bc6h_w(out, &bitpos, (bep[2][2] >> 4) & 1u, 1);  /* b2 bit 4 */
    tc_bc6h_w(out, &bitpos, bep[0][1], 7);               /* g0 primary */
    tc_bc6h_w(out, &bitpos, (bep[2][2] >> 5) & 1u, 1);  /* b2 hi (bit 5) */
    tc_bc6h_w(out, &bitpos, (bep[3][2] >> 2) & 1u, 1);  /* b3 bit 2 */
    tc_bc6h_w(out, &bitpos, (bep[2][1] >> 4) & 1u, 1);  /* g2 bit 4 */
    tc_bc6h_w(out, &bitpos, bep[0][2], 7);               /* b0 primary */
    tc_bc6h_w(out, &bitpos, (bep[3][2] >> 3) & 1u, 1);  /* b3 bit 3 */
    tc_bc6h_w(out, &bitpos, (bep[3][2] >> 5) & 1u, 1);  /* b3 bit 5 */
    tc_bc6h_w(out, &bitpos, (bep[3][2] >> 4) & 1u, 1);  /* b3 bit 4 */
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5((int32_t)bep[1][0] - (int32_t)bep[0][0]), 6);  /* r1 6-bit delta */
    tc_bc6h_w(out, &bitpos, bep[2][1] & 0xfu, 4);        /* g2 low 4 bits */
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5((int32_t)bep[1][1] - (int32_t)bep[0][1]), 6);  /* g1 6-bit delta */
    tc_bc6h_w(out, &bitpos, bep[3][1] & 0xfu, 4);        /* g3 low 4 bits */
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5((int32_t)bep[1][2] - (int32_t)bep[0][2]), 6);  /* b1 6-bit delta */
    tc_bc6h_w(out, &bitpos, bep[2][2] & 0xfu, 4);        /* b2 low 4 bits */
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5((int32_t)bep[2][0] - (int32_t)bep[0][0]), 6);  /* r2 6-bit delta */
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5((int32_t)bep[3][0] - (int32_t)bep[0][0]), 6);  /* r3 6-bit delta */
    tc_bc6h_w(out, &bitpos, (uint32_t)best_p, 5);         /* partition */
    for (i = 0; i < 16u; ++i) {
        uint32_t nb = (i == 0u || i == anchor) ? 2u : 3u;
        tc_bc6h_w(out, &bitpos, bsel[i], nb);
    }
    return best_err;
}

/* Generic 8-bit + 6/5/5 asymmetric encoder (modes 6-8). Same structure as
 * mode234 but primary is 8-bit contiguous, deltas 6/5/5 (wider R). */
static uint64_t tc_bc6h_mode234_uf16(const float pix[16][3], const uint8_t dbits[3],
                                     int mode_key, uint8_t out[16]) {
    uint32_t q[16][3], target[16][3], i, c, p;
    uint32_t bep[4][3];
    uint8_t bsel[16];
    int best_p = -1;
    uint64_t best_err = (uint64_t)-1;
    uint32_t bitpos = 0, anchor;
    int r;
    uint32_t maxv = (1u << 11) - 1u; /* 11-bit primary */
    for (i = 0; i < 16u; ++i)
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_uf16_n(pix[i][c], 11);
            target[i][c] = tc_bc6h_unquant_uf16_to_mag(tc_bc6h_quant_uf16(pix[i][c]));
        }
    /* Partition search: TOPK prefilter. */
    {
        enum { TOPK = 5u };
        uint32_t cand[TOPK], k, ncand = 0;
        uint64_t cscore[TOPK];
        for (p = 0; p < 32u; ++p) {
            uint32_t rlo[2][3], rhi[2][3];
            uint64_t score = 0;
            int have[2] = {0, 0};
            for (c = 0; c < 3u; ++c) {
                rlo[0][c] = rlo[1][c] = maxv;
                rhi[0][c] = rhi[1][c] = 0u;
            }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
                have[reg] = 1;
            }
            if (!have[0] || !have[1]) continue;
            for (c = 0; c < 3u; ++c) {
                uint64_t e0 = rhi[0][c] - rlo[0][c], e1 = rhi[1][c] - rlo[1][c];
                score += e0 * e0 + e1 * e1;
            }
            if (ncand < TOPK) {
                cand[ncand] = p;
                cscore[ncand] = score;
                ++ncand;
            } else {
                uint32_t worst = 0;
                for (k = 1; k < TOPK; ++k)
                    if (cscore[k] > cscore[worst]) worst = k;
                if (score < cscore[worst]) { cand[worst] = p; cscore[worst] = score; }
            }
        }
        for (k = 0; k < ncand; ++k) {
            uint32_t ep[4][3], rlo[2][3], rhi[2][3];
            uint8_t sel[16];
            uint64_t err;
            int fit = 1;
            p = cand[k];
            for (c = 0; c < 3u; ++c) {
                rlo[0][c] = rlo[1][c] = maxv;
                rhi[0][c] = rhi[1][c] = 0u;
            }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
            }
            for (c = 0; c < 3u && fit; ++c) {
                int32_t limit = 1 << (dbits[c] - 1);
                int32_t d1 = (int32_t)rhi[0][c] - (int32_t)rlo[0][c];
                int32_t d2 = (int32_t)rlo[1][c] - (int32_t)rlo[0][c];
                int32_t d3 = (int32_t)rhi[1][c] - (int32_t)rlo[0][c];
                if (d1 < -limit || d1 > limit - 1 || d2 < -limit || d2 > limit - 1 ||
                    d3 < -limit || d3 > limit - 1)
                    fit = 0;
            }
            if (!fit) continue;
            for (c = 0; c < 3u; ++c) {
                ep[0][c] = rlo[0][c];
                ep[1][c] = rhi[0][c];
                ep[2][c] = rlo[1][c];
                ep[3][c] = rhi[1][c];
            }
            err = tc_bc6h_mode0_selectors(target, tc_bc6h_part2[p], ep, sel);
            if (err < best_err) {
                best_err = err;
                best_p = (int)p;
                memcpy(bep, ep, sizeof(ep));
                memcpy(bsel, sel, 16u);
            }
        }
    }
    if (best_p < 0) return (uint64_t)-1;

    /* Per-region least-squares refinement (clamped to 11-bit). */
    {
        static const uint32_t w3[8] = {0, 9, 18, 27, 37, 46, 55, 64};
        int round, rr;
        for (round = 0; round < 2; ++round) {
            uint32_t nep[4][3];
            uint8_t nsel[16];
            uint64_t e;
            int fit = 1;
            for (rr = 0; rr < 2; ++rr)
                for (c = 0; c < 3u; ++c) {
                    int64_t saa = 0, sab = 0, sbb = 0, sap = 0, sbp = 0, det, l, h;
                    for (i = 0; i < 16u; ++i) {
                        int64_t bb, a, pp;
                        if ((int)tc_bc6h_part2[best_p][i] != rr) continue;
                        bb = w3[bsel[i]];
                        a = 64 - bb;
                        pp = q[i][c];
                        saa += a * a;
                        sab += a * bb;
                        sbb += bb * bb;
                        sap += a * pp;
                        sbp += bb * pp;
                    }
                    det = saa * sbb - sab * sab;
                    if (det <= 0) {
                        nep[rr * 2][c] = bep[rr * 2][c];
                        nep[rr * 2 + 1][c] = bep[rr * 2 + 1][c];
                        continue;
                    }
                    l = tc_bc6h_rdiv((sap * sbb - sbp * sab) * 64, det);
                    h = tc_bc6h_rdiv((sbp * saa - sap * sab) * 64, det);
                    if (l < 0) l = 0;
                    if (l > (int64_t)maxv) l = (int64_t)maxv;
                    if (h < 0) h = 0;
                    if (h > (int64_t)maxv) h = (int64_t)maxv;
                    nep[rr * 2][c] = (uint32_t)l;
                    nep[rr * 2 + 1][c] = (uint32_t)h;
                }
            for (c = 0; c < 3u && fit; ++c) {
                int32_t limit = 1 << (dbits[c] - 1);
                int32_t d1 = (int32_t)nep[1][c] - (int32_t)nep[0][c];
                int32_t d2 = (int32_t)nep[2][c] - (int32_t)nep[0][c];
                int32_t d3 = (int32_t)nep[3][c] - (int32_t)nep[0][c];
                if (d1 < -limit || d1 > limit - 1 || d2 < -limit || d2 > limit - 1 ||
                    d3 < -limit || d3 > limit - 1)
                    fit = 0;
            }
            if (!fit) break;
            e = tc_bc6h_mode0_selectors(target, tc_bc6h_part2[best_p], nep, nsel);
            if (e < best_err) {
                best_err = e;
                memcpy(bep, nep, sizeof(nep));
                memcpy(bsel, nsel, 16u);
            } else {
                break;
            }
        }
    }
    anchor = tc_bc6h_part2_anchor[best_p];

    for (r = 0; r < 2; ++r) {
        uint32_t at = (r == 0) ? 0u : anchor;
        if (bsel[at] & 4u) {
            for (c = 0; c < 3u; ++c) {
                uint32_t t = bep[r * 2][c];
                bep[r * 2][c] = bep[r * 2 + 1][c];
                bep[r * 2 + 1][c] = t;
            }
            for (i = 0; i < 16u; ++i)
                if (tc_bc6h_part2[best_p][i] == (uint32_t)r) bsel[i] = (uint8_t)(7u - bsel[i]);
        }
    }

    /* Bit packing: 11-bit primary = 10 lo bits + 1 hi bit per channel. */
    memset(out, 0, 16u);
    switch(mode_key){
      case 2: tc_bc6h_w(out, &bitpos, 0x02u, 5); break;
      case 3: tc_bc6h_w(out, &bitpos, 0x06u, 5); break;
      case 4: tc_bc6h_w(out, &bitpos, 0x0au, 5); break;
    }
    /* R, G, B primary low 10 bits always come first. */
    tc_bc6h_w(out, &bitpos, bep[0][0] & 1023u, 10); /* r0 low 10 */
    tc_bc6h_w(out, &bitpos, bep[0][1] & 1023u, 10); /* g0 low 10 */
    tc_bc6h_w(out, &bitpos, bep[0][2] & 1023u, 10); /* b0 low 10 */
    /* Then mode-specific interleaving of deltas, hi bits, and ep3 lo bits. */
    switch (mode_key) {
        case 2: { /* dR=5, dG=4, dB=4 */
            int32_t dr1 = (int32_t)bep[1][0] - (int32_t)bep[0][0];
            int32_t dg1 = (int32_t)bep[1][1] - (int32_t)bep[0][1];
            int32_t db1 = (int32_t)bep[1][2] - (int32_t)bep[0][2];
            int32_t dr2 = (int32_t)bep[2][0] - (int32_t)bep[0][0];
            int32_t dr3 = (int32_t)bep[3][0] - (int32_t)bep[0][0];
            tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(dr1), 5);
            tc_bc6h_w(out, &bitpos, (bep[0][0] >> 10) & 1u, 1);  /* r0 hi */
            tc_bc6h_w(out, &bitpos, bep[2][1] & 0xfu, 4);         /* g2 lo */
            tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(dg1), 4); /* g1 */
            tc_bc6h_w(out, &bitpos, (bep[0][1] >> 10) & 1u, 1);  /* g0 hi */
            tc_bc6h_w(out, &bitpos, bep[3][2] & 1u, 1);           /* b3[0] */
            tc_bc6h_w(out, &bitpos, bep[3][1] & 0xfu, 4);         /* g3 lo */
            tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(db1), 4); /* b1 */
            tc_bc6h_w(out, &bitpos, (bep[0][2] >> 10) & 1u, 1);  /* b0 hi */
            tc_bc6h_w(out, &bitpos, (bep[3][2] >> 1) & 1u, 1);    /* b3[1] */
            tc_bc6h_w(out, &bitpos, bep[2][2] & 0xfu, 4);         /* b2 lo */
            tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(dr2), 5);
            tc_bc6h_w(out, &bitpos, (bep[3][2] >> 2) & 1u, 1);    /* b3[2] */
            tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(dr3), 5);
            tc_bc6h_w(out, &bitpos, (bep[3][2] >> 3) & 1u, 1);    /* b3[3] */
            break;
        }
        case 3: { /* dR=4, dG=5, dB=4 */
            int32_t dr1 = (int32_t)bep[1][0] - (int32_t)bep[0][0];
            int32_t dg1 = (int32_t)bep[1][1] - (int32_t)bep[0][1];
            int32_t db1 = (int32_t)bep[1][2] - (int32_t)bep[0][2];
            int32_t dr2 = (int32_t)bep[2][0] - (int32_t)bep[0][0];
            int32_t dr3 = (int32_t)bep[3][0] - (int32_t)bep[0][0];
            tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(dr1), 4); /* r1 */
            tc_bc6h_w(out, &bitpos, (bep[0][0] >> 10) & 1u, 1);  /* r0 hi */
            tc_bc6h_w(out, &bitpos, (bep[3][1] >> 4) & 1u, 1);   /* g3[4] */
            tc_bc6h_w(out, &bitpos, bep[2][1] & 0xfu, 4);         /* g2 lo */
            tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(dg1), 5); /* g1 */
            tc_bc6h_w(out, &bitpos, (bep[0][1] >> 10) & 1u, 1);  /* g0 hi */
            tc_bc6h_w(out, &bitpos, bep[3][1] & 0xfu, 4);         /* g3 lo */
            tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(db1), 4); /* b1 */
            tc_bc6h_w(out, &bitpos, (bep[0][2] >> 10) & 1u, 1);  /* b0 hi */
            tc_bc6h_w(out, &bitpos, (bep[3][2] >> 1) & 1u, 1);    /* b3[1] */
            tc_bc6h_w(out, &bitpos, bep[2][2] & 0xfu, 4);         /* b2 lo */
            tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(dr2), 4); /* r2 */
            tc_bc6h_w(out, &bitpos, bep[3][2] & 1u, 1);           /* b3[0] */
            tc_bc6h_w(out, &bitpos, (bep[3][2] >> 2) & 1u, 1);    /* b3[2] */
            tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(dr3), 4); /* r3 */
            tc_bc6h_w(out, &bitpos, (bep[2][1] >> 4) & 1u, 1);    /* g2[4] */
            tc_bc6h_w(out, &bitpos, (bep[3][2] >> 3) & 1u, 1);    /* b3[3] */
            break;
        }
        case 4: { /* dR=4, dG=4, dB=5 */
            int32_t dr1 = (int32_t)bep[1][0] - (int32_t)bep[0][0];
            int32_t dg1 = (int32_t)bep[1][1] - (int32_t)bep[0][1];
            int32_t db1 = (int32_t)bep[1][2] - (int32_t)bep[0][2];
            int32_t dr2 = (int32_t)bep[2][0] - (int32_t)bep[0][0];
            int32_t dr3 = (int32_t)bep[3][0] - (int32_t)bep[0][0];
            tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(dr1), 4); /* r1 */
            tc_bc6h_w(out, &bitpos, (bep[0][0] >> 10) & 1u, 1);  /* r0 hi */
            tc_bc6h_w(out, &bitpos, (bep[2][2] >> 4) & 1u, 1);   /* b2[4] */
            tc_bc6h_w(out, &bitpos, bep[2][1] & 0xfu, 4);         /* g2 lo */
            tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(dg1), 4); /* g1 */
            tc_bc6h_w(out, &bitpos, (bep[0][1] >> 10) & 1u, 1);  /* g0 hi */
            tc_bc6h_w(out, &bitpos, bep[3][2] & 1u, 1);           /* b3[0] */
            tc_bc6h_w(out, &bitpos, bep[3][1] & 0xfu, 4);         /* g3 lo */
            tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(db1), 5); /* b1 */
            tc_bc6h_w(out, &bitpos, (bep[0][2] >> 10) & 1u, 1);  /* b0 hi */
            tc_bc6h_w(out, &bitpos, bep[2][2] & 0xfu, 4);         /* b2 lo */
            tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(dr2), 4); /* r2 */
            tc_bc6h_w(out, &bitpos, (bep[3][2] >> 1) & 1u, 1);    /* b3[1] */
            tc_bc6h_w(out, &bitpos, (bep[3][2] >> 2) & 1u, 1);    /* b3[2] */
            tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(dr3), 4); /* r3 */
            tc_bc6h_w(out, &bitpos, (bep[3][2] >> 4) & 1u, 1);    /* b3[4] */
            tc_bc6h_w(out, &bitpos, (bep[3][2] >> 3) & 1u, 1);    /* b3[3] */
            break;
        }
    }
    tc_bc6h_w(out, &bitpos, (uint32_t)best_p, 5);
    for (i = 0; i < 16u; ++i) {
        uint32_t nb = (i == 0u || i == anchor) ? 2u : 3u;
        tc_bc6h_w(out, &bitpos, bsel[i], nb);
    }
    return best_err;
}

/* Mode 5 (9.555): 9-bit primary, all 5-bit signed deltas (±16). */
static uint64_t tc_bc6h_mode5_uf16(const float pix[16][3], uint8_t out[16]) {
    uint32_t q[16][3], target[16][3], i, c, p;
    uint32_t bep[4][3];
    uint8_t bsel[16];
    int best_p = -1;
    uint64_t best_err = (uint64_t)-1;
    uint32_t bitpos = 0, anchor;
    int r;
    uint32_t maxv = (1u << 9) - 1u;
    for (i = 0; i < 16u; ++i)
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_uf16_n(pix[i][c], 9);
            target[i][c] = tc_bc6h_unquant_uf16_to_mag(tc_bc6h_quant_uf16(pix[i][c]));
        }
    /* Partition search: TOPK prefilter. */
    {
        enum { TOPK = 5u };
        uint32_t cand[TOPK], k, ncand = 0;
        uint64_t cscore[TOPK];
        for (p = 0; p < 32u; ++p) {
            uint32_t rlo[2][3], rhi[2][3];
            uint64_t score = 0;
            int have[2] = {0, 0};
            for (c = 0; c < 3u; ++c) { rlo[0][c]=rlo[1][c]=maxv; rhi[0][c]=rhi[1][c]=0u; }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
                have[reg] = 1;
            }
            if (!have[0] || !have[1]) continue;
            for (c = 0; c < 3u; ++c) score += (uint64_t)(rhi[0][c]-rlo[0][c])*(rhi[0][c]-rlo[0][c]) + (uint64_t)(rhi[1][c]-rlo[1][c])*(rhi[1][c]-rlo[1][c]);
            if (ncand<TOPK) { cand[ncand]=p; cscore[ncand]=score; ++ncand; }
            else {
                uint32_t worst=0;
                for(k=1;k<TOPK;++k) if(cscore[k]>cscore[worst]) worst=k;
                if(score<cscore[worst]){ cand[worst]=p; cscore[worst]=score; }
            }
        }
        for (k = 0; k < ncand; ++k) {
            uint32_t ep[4][3], rlo[2][3], rhi[2][3];
            uint8_t sel[16];
            uint64_t err;
            int fit = 1;
            p = cand[k];
            for (c = 0; c < 3u; ++c) { rlo[0][c]=rlo[1][c]=maxv; rhi[0][c]=rhi[1][c]=0u; }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
            }
            for (c = 0; c < 3u && fit; ++c) {
                int32_t d1 = (int32_t)rhi[0][c]-(int32_t)rlo[0][c];
                int32_t d2 = (int32_t)rlo[1][c]-(int32_t)rlo[0][c];
                int32_t d3 = (int32_t)rhi[1][c]-(int32_t)rlo[0][c];
                if (d1<-16||d1>15||d2<-16||d2>15||d3<-16||d3>15) fit=0;
            }
            if (!fit) continue;
            for(c=0;c<3u;++c){ep[0][c]=rlo[0][c];ep[1][c]=rhi[0][c];ep[2][c]=rlo[1][c];ep[3][c]=rhi[1][c];}
            err = tc_bc6h_mode0_selectors(target, tc_bc6h_part2[p], ep, sel);
            if (err < best_err) { best_err=err; best_p=(int)p; memcpy(bep,ep,sizeof(ep)); memcpy(bsel,sel,16u); }
        }
    }
    if (best_p < 0) return (uint64_t)-1;
    /* Refinement (clamped to 9-bit). */
    { static const uint32_t w3[8]={0,9,18,27,37,46,55,64}; int round,rr;
      for(round=0;round<2;++round){ uint32_t nep[4][3]; uint8_t nsel[16]; uint64_t e; int fit=1;
        for(rr=0;rr<2;++rr) for(c=0;c<3u;++c){ int64_t saa=0,sab=0,sbb=0,sap=0,sbp=0,det,l,h;
          for(i=0;i<16u;++i){ int64_t wv,a,pp; if((int)tc_bc6h_part2[best_p][i]!=rr)continue; wv=w3[bsel[i]]; a=64-wv; pp=q[i][c];
            saa+=a*a;sab+=a*wv;sbb+=wv*wv;sap+=a*pp;sbp+=wv*pp;}
          det=saa*sbb-sab*sab; if(det<=0){nep[rr*2][c]=bep[rr*2][c];nep[rr*2+1][c]=bep[rr*2+1][c];continue;}
          l=tc_bc6h_rdiv((sap*sbb-sbp*sab)*64,det);h=tc_bc6h_rdiv((sbp*saa-sap*sab)*64,det);
          if(l<0){l=0;}if(l>(int64_t)maxv){l=(int64_t)maxv;}if(h<0){h=0;}if(h>(int64_t)maxv){h=(int64_t)maxv;}
          nep[rr*2][c]=(uint32_t)l;nep[rr*2+1][c]=(uint32_t)h;}
        for(c=0;c<3u&&fit;++c){int32_t d1=(int32_t)nep[1][c]-(int32_t)nep[0][c];int32_t d2=(int32_t)nep[2][c]-(int32_t)nep[0][c];int32_t d3=(int32_t)nep[3][c]-(int32_t)nep[0][c];
          if(d1<-16||d1>15||d2<-16||d2>15||d3<-16||d3>15)fit=0;}
        if(!fit){break;} e=tc_bc6h_mode0_selectors(target,tc_bc6h_part2[best_p],nep,nsel);
        if(e<best_err){best_err=e;memcpy(bep,nep,sizeof(nep));memcpy(bsel,nsel,16u);}else break;}}
    anchor = tc_bc6h_part2_anchor[best_p];
    for(r=0;r<2;++r){uint32_t at=(r==0)?0u:anchor;if(bsel[at]&4u){
      for(c=0;c<3u;++c){uint32_t t=bep[r*2][c];bep[r*2][c]=bep[r*2+1][c];bep[r*2+1][c]=t;}
      for(i=0;i<16u;++i) if((int)tc_bc6h_part2[best_p][i]==r) bsel[i]=(uint8_t)(7u-bsel[i]);}}
    /* Bit layout: 9-bit primary contiguous, interspersed hi-bit fields. */
    memset(out,0,16u);
    tc_bc6h_w(out, &bitpos, 0x0eu, 5);
    tc_bc6h_w(out, &bitpos, bep[0][0], 9);               /* r0 */
    tc_bc6h_w(out, &bitpos, (bep[2][2]>>4)&1u, 1);       /* b2[4] */
    tc_bc6h_w(out, &bitpos, bep[0][1], 9);               /* g0 */
    tc_bc6h_w(out, &bitpos, (bep[2][1]>>4)&1u, 1);       /* g2[4] */
    tc_bc6h_w(out, &bitpos, bep[0][2], 9);               /* b0 */
    tc_bc6h_w(out, &bitpos, (bep[3][2]>>4)&1u, 1);       /* b3[4] */
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5((int32_t)bep[1][0]-(int32_t)bep[0][0]),5);  /* r1 */
    tc_bc6h_w(out, &bitpos, (bep[3][1]>>4)&1u, 1);       /* g3[4] */
    tc_bc6h_w(out, &bitpos, bep[2][1]&0xfu, 4);          /* g2 low */
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5((int32_t)bep[1][1]-(int32_t)bep[0][1]),5);  /* g1 */
    tc_bc6h_w(out, &bitpos, bep[3][2]&1u, 1);             /* b3[0] */
    tc_bc6h_w(out, &bitpos, bep[3][1]&0xfu, 4);          /* g3 low */
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5((int32_t)bep[1][2]-(int32_t)bep[0][2]),5);  /* b1 */
    tc_bc6h_w(out, &bitpos, (bep[3][2]>>1)&1u, 1);       /* b3[1] */
    tc_bc6h_w(out, &bitpos, bep[2][2]&0xfu, 4);          /* b2 low */
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5((int32_t)bep[2][0]-(int32_t)bep[0][0]),5);  /* r2 */
    tc_bc6h_w(out, &bitpos, (bep[3][2]>>2)&1u, 1);       /* b3[2] */
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5((int32_t)bep[3][0]-(int32_t)bep[0][0]),5);  /* r3 */
    tc_bc6h_w(out, &bitpos, (bep[3][2]>>3)&1u, 1);       /* b3[3] */
    tc_bc6h_w(out, &bitpos, (uint32_t)best_p, 5);
    for(i=0;i<16u;++i){uint32_t nb=(i==0u||i==anchor)?2u:3u; tc_bc6h_w(out,&bitpos,bsel[i],nb);}
    return best_err;
}

/* Generic modes 6-8 (8-bit primary + asymmetric 6/5/5 deltas). */
static uint64_t tc_bc6h_mode678_uf16(const float pix[16][3], const uint8_t dbits[3],
                                     int mode_key, uint8_t out[16]) {
    uint32_t q[16][3], target[16][3], i, c, p;
    uint32_t bep[4][3];
    uint8_t bsel[16];
    int best_p = -1;
    uint64_t best_err = (uint64_t)-1;
    uint32_t bitpos = 0, anchor;
    int r;
    uint32_t maxv = (1u << 8) - 1u;
    for (i = 0; i < 16u; ++i)
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_uf16_n(pix[i][c], 8);
            target[i][c] = tc_bc6h_unquant_uf16_to_mag(tc_bc6h_quant_uf16(pix[i][c]));
        }
    /* Partition search: TOPK prefilter. */
    {
        enum { TOPK = 5u };
        uint32_t cand[TOPK], k, ncand = 0;
        uint64_t cscore[TOPK];
        for (p = 0; p < 32u; ++p) {
            uint32_t rlo[2][3], rhi[2][3];
            uint64_t score = 0;
            int have[2] = {0, 0};
            for (c = 0; c < 3u; ++c) { rlo[0][c]=rlo[1][c]=maxv; rhi[0][c]=rhi[1][c]=0u; }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
                have[reg] = 1;
            }
            if (!have[0] || !have[1]) continue;
            for (c = 0; c < 3u; ++c) score += (uint64_t)(rhi[0][c]-rlo[0][c])*(rhi[0][c]-rlo[0][c]) + (uint64_t)(rhi[1][c]-rlo[1][c])*(rhi[1][c]-rlo[1][c]);
            if (ncand<TOPK) { cand[ncand]=p; cscore[ncand]=score; ++ncand; }
            else { uint32_t worst=0; for(k=1;k<TOPK;++k) if(cscore[k]>cscore[worst]) worst=k; if(score<cscore[worst]){ cand[worst]=p; cscore[worst]=score; } }
        }
        for (k = 0; k < ncand; ++k) {
            uint32_t ep[4][3], rlo[2][3], rhi[2][3];
            uint8_t sel[16];
            uint64_t err;
            int fit = 1;
            p = cand[k];
            for (c = 0; c < 3u; ++c) { rlo[0][c]=rlo[1][c]=maxv; rhi[0][c]=rhi[1][c]=0u; }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
            }
            for (c = 0; c < 3u && fit; ++c) {
                int32_t limit = 1 << (dbits[c] - 1);
                int32_t d1 = (int32_t)rhi[0][c]-(int32_t)rlo[0][c];
                int32_t d2 = (int32_t)rlo[1][c]-(int32_t)rlo[0][c];
                int32_t d3 = (int32_t)rhi[1][c]-(int32_t)rlo[0][c];
                if (d1<-limit||d1>limit-1||d2<-limit||d2>limit-1||d3<-limit||d3>limit-1) fit=0;
            }
            if (!fit) continue;
            for(c=0;c<3u;++c){ep[0][c]=rlo[0][c];ep[1][c]=rhi[0][c];ep[2][c]=rlo[1][c];ep[3][c]=rhi[1][c];}
            err = tc_bc6h_mode0_selectors(target, tc_bc6h_part2[p], ep, sel);
            if (err < best_err) { best_err=err; best_p=(int)p; memcpy(bep,ep,sizeof(ep)); memcpy(bsel,sel,16u); }
        }
    }
    if (best_p < 0) return (uint64_t)-1;
    /* Refinement (clamped to 8-bit). */
    { static const uint32_t w3[8]={0,9,18,27,37,46,55,64}; int round,rr;
      for(round=0;round<2;++round){ uint32_t nep[4][3]; uint8_t nsel[16]; uint64_t e; int fit=1;
        for(rr=0;rr<2;++rr) for(c=0;c<3u;++c){ int64_t saa=0,sab=0,sbb=0,sap=0,sbp=0,det,l,h;
          for(i=0;i<16u;++i){ int64_t wv,a,pp; if((int)tc_bc6h_part2[best_p][i]!=rr)continue; wv=w3[bsel[i]]; a=64-wv; pp=q[i][c];
            saa+=a*a;sab+=a*wv;sbb+=wv*wv;sap+=a*pp;sbp+=wv*pp;}
          det=saa*sbb-sab*sab; if(det<=0){nep[rr*2][c]=bep[rr*2][c];nep[rr*2+1][c]=bep[rr*2+1][c];continue;}
          l=tc_bc6h_rdiv((sap*sbb-sbp*sab)*64,det);h=tc_bc6h_rdiv((sbp*saa-sap*sab)*64,det);
          if(l<0){l=0;}if(l>(int64_t)maxv){l=(int64_t)maxv;}if(h<0){h=0;}if(h>(int64_t)maxv){h=(int64_t)maxv;}
          nep[rr*2][c]=(uint32_t)l;nep[rr*2+1][c]=(uint32_t)h;}
        for(c=0;c<3u&&fit;++c){ int32_t limit=1<<(dbits[c]-1);
          int32_t d1=(int32_t)nep[1][c]-(int32_t)nep[0][c];int32_t d2=(int32_t)nep[2][c]-(int32_t)nep[0][c];int32_t d3=(int32_t)nep[3][c]-(int32_t)nep[0][c];
          if(d1<-limit||d1>limit-1||d2<-limit||d2>limit-1||d3<-limit||d3>limit-1) fit=0;}
        if(!fit){break;} e=tc_bc6h_mode0_selectors(target,tc_bc6h_part2[best_p],nep,nsel);
        if(e<best_err){best_err=e;memcpy(bep,nep,sizeof(nep));memcpy(bsel,nsel,16u);}else break;}}
    anchor = tc_bc6h_part2_anchor[best_p];
    for(r=0;r<2;++r){uint32_t at=(r==0)?0u:anchor;if(bsel[at]&4u){
      for(c=0;c<3u;++c){uint32_t t=bep[r*2][c];bep[r*2][c]=bep[r*2+1][c];bep[r*2+1][c]=t;}
      for(i=0;i<16u;++i) if((int)tc_bc6h_part2[best_p][i]==r) bsel[i]=(uint8_t)(7u-bsel[i]);}}
    /* Bit packing: 8-bit primary contiguous + scattered hi-bit + delta fields. */
    memset(out,0,16u);
    switch(mode_key){
      case 6: tc_bc6h_w(out,&bitpos,0x12u,5); break;
      case 7: tc_bc6h_w(out,&bitpos,0x16u,5); break;
      case 8: tc_bc6h_w(out,&bitpos,0x1au,5); break;
    }
    switch (mode_key) {
      case 6: { /* dR=6, dG=5, dB=5 */
        int32_t dr1=(int32_t)bep[1][0]-(int32_t)bep[0][0], dg1=(int32_t)bep[1][1]-(int32_t)bep[0][1], db1=(int32_t)bep[1][2]-(int32_t)bep[0][2];
        int32_t dr2=(int32_t)bep[2][0]-(int32_t)bep[0][0], dr3=(int32_t)bep[3][0]-(int32_t)bep[0][0];
        tc_bc6h_w(out,&bitpos,bep[0][0],8);
        tc_bc6h_w(out,&bitpos,(bep[3][1]>>4)&1u,1); /* g3[4] */
        tc_bc6h_w(out,&bitpos,(bep[2][2]>>4)&1u,1); /* b2[4] */
        tc_bc6h_w(out,&bitpos,bep[0][1],8);
        tc_bc6h_w(out,&bitpos,(bep[3][2]>>2)&1u,1); /* b3[2] */
        tc_bc6h_w(out,&bitpos,(bep[2][1]>>4)&1u,1); /* g2[4] */
        tc_bc6h_w(out,&bitpos,bep[0][2],8);
        tc_bc6h_w(out,&bitpos,(bep[3][2]>>3)&1u,1); /* b3[3] */
        tc_bc6h_w(out,&bitpos,(bep[3][2]>>4)&1u,1); /* b3[4] */
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr1),6); /* r1 6-bit */
        tc_bc6h_w(out,&bitpos,bep[2][1]&0xfu,4);    /* g2 lo */
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dg1),5); /* g1 */
        tc_bc6h_w(out,&bitpos,bep[3][2]&1u,1);      /* b3[0] */
        tc_bc6h_w(out,&bitpos,bep[3][1]&0xfu,4);    /* g3 lo */
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(db1),5); /* b1 */
        tc_bc6h_w(out,&bitpos,(bep[3][2]>>1)&1u,1);  /* b3[1] */
        tc_bc6h_w(out,&bitpos,bep[2][2]&0xfu,4);    /* b2 lo */
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr2),6); /* r2 6-bit */
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr3),6); /* r3 6-bit */
        break;
      }
      case 7: { /* dR=5, dG=6, dB=5  (G wider) */
        int32_t dr1=(int32_t)bep[1][0]-(int32_t)bep[0][0], dg1=(int32_t)bep[1][1]-(int32_t)bep[0][1], db1=(int32_t)bep[1][2]-(int32_t)bep[0][2];
        int32_t dr2=(int32_t)bep[2][0]-(int32_t)bep[0][0], dr3=(int32_t)bep[3][0]-(int32_t)bep[0][0];
        tc_bc6h_w(out,&bitpos,bep[0][0],8);
        tc_bc6h_w(out,&bitpos,bep[3][2]&1u,1);      /* b3[0] */
        tc_bc6h_w(out,&bitpos,(bep[2][2]>>4)&1u,1); /* b2[4] */
        tc_bc6h_w(out,&bitpos,bep[0][1],8);
        tc_bc6h_w(out,&bitpos,(bep[2][1]>>5)&1u,1); /* g2[5] */
        tc_bc6h_w(out,&bitpos,(bep[2][1]>>4)&1u,1); /* g2[4] */
        tc_bc6h_w(out,&bitpos,bep[0][2],8);
        tc_bc6h_w(out,&bitpos,(bep[3][1]>>5)&1u,1); /* g3[5] */
        tc_bc6h_w(out,&bitpos,(bep[3][2]>>4)&1u,1); /* b3[4] */
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr1),5); /* r1 */
        tc_bc6h_w(out,&bitpos,(bep[3][1]>>4)&1u,1); /* g3[4] */
        tc_bc6h_w(out,&bitpos,bep[2][1]&0xfu,4);    /* g2 lo */
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dg1),6); /* g1 6-bit */
        tc_bc6h_w(out,&bitpos,bep[3][1]&0xfu,4);    /* g3 lo */
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(db1),5); /* b1 */
        tc_bc6h_w(out,&bitpos,(bep[3][2]>>1)&1u,1);  /* b3[1] */
        tc_bc6h_w(out,&bitpos,bep[2][2]&0xfu,4);    /* b2 lo */
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr2),5); /* r2 */
        tc_bc6h_w(out,&bitpos,(bep[3][2]>>2)&1u,1);  /* b3[2] */
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr3),5); /* r3 */
        tc_bc6h_w(out,&bitpos,(bep[3][2]>>3)&1u,1);  /* b3[3] */
        break;
      }
      case 8: { /* dR=5, dG=5, dB=6 (B wider) */
        int32_t dr1=(int32_t)bep[1][0]-(int32_t)bep[0][0], dg1=(int32_t)bep[1][1]-(int32_t)bep[0][1], db1=(int32_t)bep[1][2]-(int32_t)bep[0][2];
        int32_t dr2=(int32_t)bep[2][0]-(int32_t)bep[0][0], dr3=(int32_t)bep[3][0]-(int32_t)bep[0][0];
        tc_bc6h_w(out,&bitpos,bep[0][0],8);
        tc_bc6h_w(out,&bitpos,(bep[3][2]>>1)&1u,1); /* b3[1] */
        tc_bc6h_w(out,&bitpos,(bep[2][2]>>4)&1u,1); /* b2[4] */
        tc_bc6h_w(out,&bitpos,bep[0][1],8);
        tc_bc6h_w(out,&bitpos,(bep[2][2]>>5)&1u,1); /* b2[5] */
        tc_bc6h_w(out,&bitpos,(bep[2][1]>>4)&1u,1); /* g2[4] */
        tc_bc6h_w(out,&bitpos,bep[0][2],8);
        tc_bc6h_w(out,&bitpos,(bep[3][2]>>5)&1u,1); /* b3[5] */
        tc_bc6h_w(out,&bitpos,(bep[3][2]>>4)&1u,1); /* b3[4] */
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr1),5); /* r1 */
        tc_bc6h_w(out,&bitpos,(bep[3][1]>>4)&1u,1); /* g3[4] */
        tc_bc6h_w(out,&bitpos,bep[2][1]&0xfu,4);    /* g2 lo */
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dg1),5); /* g1 */
        tc_bc6h_w(out,&bitpos,bep[3][2]&1u,1);      /* b3[0] */
        tc_bc6h_w(out,&bitpos,bep[3][1]&0xfu,4);    /* g3 lo */
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(db1),6); /* b1 6-bit */
        tc_bc6h_w(out,&bitpos,bep[2][2]&0xfu,4);    /* b2 lo */
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr2),5); /* r2 */
        tc_bc6h_w(out,&bitpos,(bep[3][2]>>2)&1u,1);  /* b3[2] */
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr3),5); /* r3 */
        tc_bc6h_w(out,&bitpos,(bep[3][2]>>3)&1u,1);  /* b3[3] */
        break;
      }
    }
    tc_bc6h_w(out, &bitpos, (uint32_t)best_p, 5);
    for(i=0;i<16u;++i){uint32_t nb=(i==0u||i==anchor)?2u:3u; tc_bc6h_w(out,&bitpos,bsel[i],nb);}
    return best_err;
}

/* One-region mode 10 (10.10.10 direct, no delta). 6×10-bit endpoints,
 * 4-bit weight indices (first texel 3-bit). Same lo/hi endpoints as mode 11
 * but without the 11.9 delta transform — preserves full precision for ep1 too. */
static uint64_t tc_bc6h_mode10_uf16(const float pix[16][3], uint8_t out[16]) {
    uint32_t q[16][3], target[16][3], lo[3], hi[3], luma_lo[3], luma_hi[3];
    uint32_t i, c, bitpos = 0, min_l = UINT_MAX, max_l = 0, min_i = 0, max_i = 0;
    uint8_t sel[16], box_sel[16], luma_sel[16];
    memset(out, 0, 16);
    for (c = 0; c < 3u; ++c) { lo[c] = UINT_MAX; hi[c] = 0; }
    for (i = 0; i < 16u; ++i) {
        uint32_t l;
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_uf16(pix[i][c]);
            target[i][c] = tc_bc6h_unquant_uf16_to_mag(q[i][c]);
            if (q[i][c] < lo[c]) lo[c] = q[i][c];
            if (q[i][c] > hi[c]) hi[c] = q[i][c];
        }
        l = target[i][0] * 38u + target[i][1] * 76u + target[i][2] * 14u;
        if (l < min_l) { min_l = l; min_i = i; }
        if (l >= max_l) { max_l = l; max_i = i; }
    }
    for (c = 0; c < 3u; ++c) { luma_lo[c] = q[min_i][c]; luma_hi[c] = q[max_i][c]; }
    if (tc_bc6h_choose_selectors_uf16(target, luma_lo, luma_hi, luma_sel) <
        tc_bc6h_choose_selectors_uf16(target, lo, hi, box_sel)) {
        memcpy(lo, luma_lo, sizeof(lo)); memcpy(hi, luma_hi, sizeof(hi));
        memcpy(sel, luma_sel, sizeof(sel));
    } else { memcpy(sel, box_sel, sizeof(sel)); }
    (void)tc_bc6h_refine_uf16(q, target, lo, hi, sel);
    /* Mode 10 anchor: MSB (bit 3 of 4-bit index) must be 0 for texel 0. */
    if (sel[0] & 8u) {
        uint32_t t;
        for (c = 0; c < 3u; ++c) { t = lo[c]; lo[c] = hi[c]; hi[c] = t; }
        for (i = 0; i < 16u; ++i) sel[i] = (uint8_t)(15u - sel[i]);
    }
    tc_bc6h_w(out, &bitpos, 0x03u, 5); /* mode 10 code = 0b00011 */
    tc_bc6h_w(out, &bitpos, lo[0], 10);
    tc_bc6h_w(out, &bitpos, lo[1], 10);
    tc_bc6h_w(out, &bitpos, lo[2], 10);
    tc_bc6h_w(out, &bitpos, hi[0], 10);
    tc_bc6h_w(out, &bitpos, hi[1], 10);
    tc_bc6h_w(out, &bitpos, hi[2], 10);
    tc_bc6h_w(out, &bitpos, sel[0], 3);  /* anchor: 3 bits */
    for (i = 1; i < 16u; ++i) tc_bc6h_w(out, &bitpos, sel[i], 4);
    return tc_bc6h_choose_selectors_uf16(target, lo, hi, sel);
}

/* One-region mode 12 (12.8.x3) — 12-bit primary, 8-bit signed deltas. */
static uint64_t tc_bc6h_mode12_uf16(const float pix[16][3], uint8_t out[16]) {
    uint32_t q[16][3], target[16][3], lo_[3], hi_[3], luma_lo[3], luma_hi[3];
    uint32_t i, c, bitpos = 0, min_l = UINT_MAX, max_l = 0, min_i = 0, max_i = 0;
    uint8_t sel[16], box_sel[16], luma_sel[16];
    uint32_t maxv = (1u << 12) - 1u;
    memset(out, 0, 16);
    for (c = 0; c < 3u; ++c) { lo_[c] = maxv; hi_[c] = 0; }
    for (i = 0; i < 16u; ++i) {
        uint32_t l;
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_uf16_n(pix[i][c], 12);
            target[i][c] = tc_bc6h_unquant_uf16_to_mag(tc_bc6h_quant_uf16(pix[i][c]));
            if (q[i][c] < lo_[c]) lo_[c] = q[i][c];
            if (q[i][c] > hi_[c]) hi_[c] = q[i][c];
        }
        l = target[i][0] * 38u + target[i][1] * 76u + target[i][2] * 14u;
        if (l < min_l) { min_l = l; min_i = i; }
        if (l >= max_l) { max_l = l; max_i = i; }
    }
    for (c = 0; c < 3u; ++c) { luma_lo[c] = q[min_i][c]; luma_hi[c] = q[max_i][c]; }
    if (tc_bc6h_choose_selectors_uf16(target, luma_lo, luma_hi, luma_sel) <
        tc_bc6h_choose_selectors_uf16(target, lo_, hi_, box_sel)) {
        memcpy(lo_, luma_lo, sizeof(lo_)); memcpy(hi_, luma_hi, sizeof(hi_));
        memcpy(sel, luma_sel, sizeof(sel));
    } else { memcpy(sel, box_sel, sizeof(sel)); }
    (void)tc_bc6h_refine_uf16(q, target, lo_, hi_, sel);
    if (sel[0] & 8u) {
        uint32_t t;
        for (c = 0; c < 3u; ++c) { t = lo_[c]; lo_[c] = hi_[c]; hi_[c] = t; }
        for (i = 0; i < 16u; ++i) sel[i] = (uint8_t)(15u - sel[i]);
    }
    /* Mode 12: 12-bit primary (10 lo + 2 bit-reversed hi), 8-bit signed delta.
     * Delta = hi - lo. Must fit in 8-bit signed (-128..127). */
    {
        uint32_t r0_lo = lo_[0] & 1023u, r0_hi = (lo_[0] >> 10) & 3u;
        uint32_t g0_lo = lo_[1] & 1023u, g0_hi = (lo_[1] >> 10) & 3u;
        uint32_t b0_lo = lo_[2] & 1023u, b0_hi = (lo_[2] >> 10) & 3u;
        int32_t dr = (int32_t)hi_[0] - (int32_t)lo_[0];
        int32_t dg = (int32_t)hi_[1] - (int32_t)lo_[1];
        int32_t db = (int32_t)hi_[2] - (int32_t)lo_[2];
        if (dr < -128 || dr > 127 || dg < -128 || dg > 127 || db < -128 || db > 127)
            return (uint64_t)-1;
        tc_bc6h_w(out, &bitpos, 0x0bu, 5); /* mode 12 code = 0b01011 */
        tc_bc6h_w(out, &bitpos, r0_lo, 10);
        tc_bc6h_w(out, &bitpos, g0_lo, 10);
        tc_bc6h_w(out, &bitpos, b0_lo, 10);
        tc_bc6h_w(out, &bitpos, (uint32_t)(dr & 0xffu), 8); /* r1 delta */
        /* High bits reversed: rd_r(2) reads bit0,bit1 → stores bit1<<1|bit0 */
        tc_bc6h_w(out, &bitpos, ((r0_hi & 1u) << 1) | ((r0_hi >> 1) & 1u), 2);
        tc_bc6h_w(out, &bitpos, (uint32_t)(dg & 0xffu), 8); /* g1 delta */
        tc_bc6h_w(out, &bitpos, ((g0_hi & 1u) << 1) | ((g0_hi >> 1) & 1u), 2);
        tc_bc6h_w(out, &bitpos, (uint32_t)(db & 0xffu), 8); /* b1 delta */
        tc_bc6h_w(out, &bitpos, ((b0_hi & 1u) << 1) | ((b0_hi >> 1) & 1u), 2);
        tc_bc6h_w(out, &bitpos, sel[0], 3);
        for (i = 1; i < 16u; ++i) tc_bc6h_w(out, &bitpos, sel[i], 4);
    }
    return tc_bc6h_choose_selectors_uf16(target, lo_, hi_, sel);
}

/* One-region mode 13 (16.4.x3) — 16-bit primary, 4-bit signed deltas (±8). */
static uint64_t tc_bc6h_mode13_uf16(const float pix[16][3], uint8_t out[16]) {
    uint32_t q[16][3], target[16][3], lo_[3], hi_[3], luma_lo[3], luma_hi[3];
    uint32_t i, c, bitpos = 0, min_l = UINT_MAX, max_l = 0, min_i = 0, max_i = 0;
    uint8_t sel[16], box_sel[16], luma_sel[16];
    uint32_t maxv = (1u << 16) - 1u;
    memset(out, 0, 16);
    for (c = 0; c < 3u; ++c) { lo_[c] = maxv; hi_[c] = 0; }
    for (i = 0; i < 16u; ++i) {
        uint32_t l;
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_uf16_n(pix[i][c], 16);
            target[i][c] = tc_bc6h_unquant_uf16_to_mag(tc_bc6h_quant_uf16(pix[i][c]));
            if (q[i][c] < lo_[c]) lo_[c] = q[i][c];
            if (q[i][c] > hi_[c]) hi_[c] = q[i][c];
        }
        l = target[i][0] * 38u + target[i][1] * 76u + target[i][2] * 14u;
        if (l < min_l) { min_l = l; min_i = i; }
        if (l >= max_l) { max_l = l; max_i = i; }
    }
    for (c = 0; c < 3u; ++c) { luma_lo[c] = q[min_i][c]; luma_hi[c] = q[max_i][c]; }
    if (tc_bc6h_choose_selectors_uf16(target, luma_lo, luma_hi, luma_sel) <
        tc_bc6h_choose_selectors_uf16(target, lo_, hi_, box_sel)) {
        memcpy(lo_, luma_lo, sizeof(lo_)); memcpy(hi_, luma_hi, sizeof(hi_));
        memcpy(sel, luma_sel, sizeof(sel));
    } else { memcpy(sel, box_sel, sizeof(sel)); }
    (void)tc_bc6h_refine_uf16(q, target, lo_, hi_, sel);
    if (sel[0] & 8u) {
        uint32_t t;
        for (c = 0; c < 3u; ++c) { t = lo_[c]; lo_[c] = hi_[c]; hi_[c] = t; }
        for (i = 0; i < 16u; ++i) sel[i] = (uint8_t)(15u - sel[i]);
    }
    {
        uint32_t r0_lo = lo_[0] & 1023u, r0_hi = (lo_[0] >> 10) & 63u;
        uint32_t g0_lo = lo_[1] & 1023u, g0_hi = (lo_[1] >> 10) & 63u;
        uint32_t b0_lo = lo_[2] & 1023u, b0_hi = (lo_[2] >> 10) & 63u;
        int32_t dr = (int32_t)hi_[0] - (int32_t)lo_[0];
        int32_t dg = (int32_t)hi_[1] - (int32_t)lo_[1];
        int32_t db = (int32_t)hi_[2] - (int32_t)lo_[2];
        if (dr < -8 || dr > 7 || dg < -8 || dg > 7 || db < -8 || db > 7)
            return (uint64_t)-1;
        tc_bc6h_w(out, &bitpos, 0x0fu, 5); /* mode 13 code = 0b01111 */
        tc_bc6h_w(out, &bitpos, r0_lo, 10);
        tc_bc6h_w(out, &bitpos, g0_lo, 10);
        tc_bc6h_w(out, &bitpos, b0_lo, 10);
        tc_bc6h_w(out, &bitpos, (uint32_t)(dr & 0xfu), 4); /* r1 delta */
        {
            uint32_t rev = 0, bi;
            for (bi = 0; bi < 6; ++bi) rev |= ((r0_hi >> bi) & 1u) << (5 - bi);
            tc_bc6h_w(out, &bitpos, rev, 6);
        }
        tc_bc6h_w(out, &bitpos, (uint32_t)(dg & 0xfu), 4); /* g1 delta */
        {
            uint32_t rev = 0, bi;
            for (bi = 0; bi < 6; ++bi) rev |= ((g0_hi >> bi) & 1u) << (5 - bi);
            tc_bc6h_w(out, &bitpos, rev, 6);
        }
        tc_bc6h_w(out, &bitpos, (uint32_t)(db & 0xfu), 4); /* b1 delta */
        {
            uint32_t rev = 0, bi;
            for (bi = 0; bi < 6; ++bi) rev |= ((b0_hi >> bi) & 1u) << (5 - bi);
            tc_bc6h_w(out, &bitpos, rev, 6);
        }
        tc_bc6h_w(out, &bitpos, sel[0], 3);
        for (i = 1; i < 16u; ++i) tc_bc6h_w(out, &bitpos, sel[i], 4);
    }
    return tc_bc6h_choose_selectors_uf16(target, lo_, hi_, sel);
}

/* Encode the block as BC6H mode 9; returns the reconstruction error. */
static uint64_t tc_bc6h_mode9_uf16(const float pix[16][3], uint8_t out[16]) {
    uint32_t q[16][3], target[16][3], i, c, p;
    uint32_t bep[4][3];
    uint8_t bsel[16];
    int best_p = -1, r;
    uint64_t best_err = (uint64_t)-1;
    uint32_t R[4], G[4], B[4], bitpos = 0, anchor;
    for (i = 0; i < 16u; ++i)
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_uf16_n(pix[i][c], 6);
            /* error reference at 10-bit precision, so mode-9 and mode-11 errors
             * are measured against the same target and are comparable. */
            target[i][c] = tc_bc6h_unquant_uf16_to_mag(tc_bc6h_quant_uf16(pix[i][c]));
        }
    /* Two-pass partition search: a cheap per-region bounding-box "spread" score
     * ranks all 32 partitions, then only the best few get the full selector
     * search. A partition that splits the block into two tight regions scores
     * low and is what wins, so the prefilter rarely drops the true best. */
    {
        enum { TOPK = 5u };
        uint32_t cand[TOPK], k, ncand = 0;
        uint64_t cscore[TOPK];
        for (p = 0; p < 32u; ++p) {
            uint32_t rlo[2][3], rhi[2][3];
            uint64_t score = 0;
            int have[2] = {0, 0};
            for (c = 0; c < 3u; ++c) {
                rlo[0][c] = rlo[1][c] = 63u;
                rhi[0][c] = rhi[1][c] = 0u;
            }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
                have[reg] = 1;
            }
            if (!have[0] || !have[1]) continue;
            for (c = 0; c < 3u; ++c) {
                uint64_t e0 = rhi[0][c] - rlo[0][c], e1 = rhi[1][c] - rlo[1][c];
                score += e0 * e0 + e1 * e1;
            }
            /* insertion into the top-TOPK-smallest-score list */
            if (ncand < TOPK) {
                cand[ncand] = p;
                cscore[ncand] = score;
                ++ncand;
            } else {
                uint32_t worst = 0;
                for (k = 1; k < TOPK; ++k)
                    if (cscore[k] > cscore[worst]) worst = k;
                if (score < cscore[worst]) {
                    cand[worst] = p;
                    cscore[worst] = score;
                }
            }
        }
        for (k = 0; k < ncand; ++k) {
            uint32_t ep[4][3], rlo[2][3], rhi[2][3];
            uint8_t sel[16];
            uint64_t err;
            p = cand[k];
            for (c = 0; c < 3u; ++c) {
                rlo[0][c] = rlo[1][c] = 63u;
                rhi[0][c] = rhi[1][c] = 0u;
            }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
            }
            for (c = 0; c < 3u; ++c) {
                ep[0][c] = rlo[0][c];
                ep[1][c] = rhi[0][c];
                ep[2][c] = rlo[1][c];
                ep[3][c] = rhi[1][c];
            }
            err = tc_bc6h_mode9_selectors(q, target, tc_bc6h_part2[p], ep, sel);
            if (err < best_err) {
                best_err = err;
                best_p = (int)p;
                memcpy(bep, ep, sizeof(ep));
                memcpy(bsel, sel, 16u);
            }
        }
    }
    if (best_p < 0) return (uint64_t)-1;

    /* Per-region least-squares refinement of the winning partition. */
    {
        static const uint32_t w3[8] = {0, 9, 18, 27, 37, 46, 55, 64};
        int round, rr;
        for (round = 0; round < 2; ++round) {
            uint32_t nep[4][3];
            uint8_t nsel[16];
            uint64_t e;
            for (rr = 0; rr < 2; ++rr)
                for (c = 0; c < 3u; ++c) {
                    int64_t saa = 0, sab = 0, sbb = 0, sap = 0, sbp = 0, det, l, h;
                    for (i = 0; i < 16u; ++i) {
                        int64_t bb, a, pp;
                        if ((int)tc_bc6h_part2[best_p][i] != rr) continue;
                        bb = w3[bsel[i]];
                        a = 64 - bb;
                        pp = q[i][c];
                        saa += a * a;
                        sab += a * bb;
                        sbb += bb * bb;
                        sap += a * pp;
                        sbp += bb * pp;
                    }
                    det = saa * sbb - sab * sab;
                    if (det <= 0) {
                        nep[rr * 2][c] = bep[rr * 2][c];
                        nep[rr * 2 + 1][c] = bep[rr * 2 + 1][c];
                        continue;
                    }
                    l = tc_bc6h_rdiv((sap * sbb - sbp * sab) * 64, det);
                    h = tc_bc6h_rdiv((sbp * saa - sap * sab) * 64, det);
                    if (l < 0) l = 0;
                    if (l > 63) l = 63;
                    if (h < 0) h = 0;
                    if (h > 63) h = 63;
                    nep[rr * 2][c] = (uint32_t)l;
                    nep[rr * 2 + 1][c] = (uint32_t)h;
                }
            e = tc_bc6h_mode9_selectors(q, target, tc_bc6h_part2[best_p], nep, nsel);
            if (e < best_err) {
                best_err = e;
                memcpy(bep, nep, sizeof(nep));
                memcpy(bsel, nsel, 16u);
            } else {
                break;
            }
        }
    }
    anchor = tc_bc6h_part2_anchor[best_p];

    /* Anchor fix-up: each region's anchor texel index must have its MSB clear
     * (it is stored with one fewer bit). Region 0's anchor is texel 0. */
    for (r = 0; r < 2; ++r) {
        uint32_t at = (r == 0) ? 0u : anchor;
        if (bsel[at] & 4u) {
            for (c = 0; c < 3u; ++c) {
                uint32_t t = bep[r * 2][c];
                bep[r * 2][c] = bep[r * 2 + 1][c];
                bep[r * 2 + 1][c] = t;
            }
            for (i = 0; i < 16u; ++i)
                if (tc_bc6h_part2[best_p][i] == (uint32_t)r) bsel[i] = (uint8_t)(7u - bsel[i]);
        }
    }

    /* Mode 9 (like mode 11) stores all four endpoints explicitly at 6 bits --
     * no delta transform -- so any two regions are representable. */
    for (c = 0; c < 3u; ++c) {
        uint32_t *E = (c == 0) ? R : (c == 1) ? G : B;
        E[0] = bep[0][c];
        E[1] = bep[1][c];
        E[2] = bep[2][c];
        E[3] = bep[3][c];
    }

    memset(out, 0, 16u);
    tc_bc6h_w(out, &bitpos, 30u, 5); /* mode 9 code 0b11110 */
    tc_bc6h_w(out, &bitpos, R[0], 6);              /* rw[5:0] */
    tc_bc6h_w(out, &bitpos, (G[3] >> 4) & 1u, 1);  /* gz[4]   */
    tc_bc6h_w(out, &bitpos, B[3] & 1u, 1);         /* bz[0]   */
    tc_bc6h_w(out, &bitpos, (B[3] >> 1) & 1u, 1);  /* bz[1]   */
    tc_bc6h_w(out, &bitpos, (B[2] >> 4) & 1u, 1);  /* by[4]   */
    tc_bc6h_w(out, &bitpos, G[0], 6);              /* gw[5:0] */
    tc_bc6h_w(out, &bitpos, (G[2] >> 5) & 1u, 1);  /* gy[5]   */
    tc_bc6h_w(out, &bitpos, (B[2] >> 5) & 1u, 1);  /* by[5]   */
    tc_bc6h_w(out, &bitpos, (B[3] >> 2) & 1u, 1);  /* bz[2]   */
    tc_bc6h_w(out, &bitpos, (G[2] >> 4) & 1u, 1);  /* gy[4]   */
    tc_bc6h_w(out, &bitpos, B[0], 6);              /* bw[5:0] */
    tc_bc6h_w(out, &bitpos, (G[3] >> 5) & 1u, 1);  /* gz[5]   */
    tc_bc6h_w(out, &bitpos, (B[3] >> 3) & 1u, 1);  /* bz[3]   */
    tc_bc6h_w(out, &bitpos, (B[3] >> 5) & 1u, 1);  /* bz[5]   */
    tc_bc6h_w(out, &bitpos, (B[3] >> 4) & 1u, 1);  /* bz[4]   */
    tc_bc6h_w(out, &bitpos, R[1], 6);              /* rx[5:0] */
    tc_bc6h_w(out, &bitpos, G[2] & 0xfu, 4);       /* gy[3:0] */
    tc_bc6h_w(out, &bitpos, G[1], 6);              /* gx[5:0] */
    tc_bc6h_w(out, &bitpos, G[3] & 0xfu, 4);       /* gz[3:0] */
    tc_bc6h_w(out, &bitpos, B[1], 6);              /* bx[5:0] */
    tc_bc6h_w(out, &bitpos, B[2] & 0xfu, 4);       /* by[3:0] */
    tc_bc6h_w(out, &bitpos, R[2], 6);              /* ry[5:0] */
    tc_bc6h_w(out, &bitpos, R[3], 6);              /* rz[5:0] */
    tc_bc6h_w(out, &bitpos, (uint32_t)best_p, 5);  /* partition */
    for (i = 0; i < 16u; ++i) {
        uint32_t nb = (i == 0u || i == anchor) ? 2u : 3u;
        tc_bc6h_w(out, &bitpos, bsel[i], nb);
    }
    return best_err;
}

/* Signed variant of mode9 (two-region, 6-bit explicit endpoints). */
static uint64_t tc_bc6h_mode9_sf16(const float pix[16][3], uint8_t out[16]) {
    int32_t q[16][3], target[16][3], bep[4][3];
    uint32_t i, c, p;
    uint8_t bsel[16];
    int best_p = -1;
    uint64_t best_err = (uint64_t)-1;
    uint32_t R[4], G[4], B[4], bitpos = 0, anchor, r;
    for (i = 0; i < 16u; ++i)
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_sf16_n(pix[i][c], 6);
            target[i][c] = tc_bc6h_unquant_sf16_to_smag(tc_bc6h_quant_sf16(pix[i][c]));
        }
    {
        enum { TOPK = 5u };
        uint32_t cand[TOPK], k, ncand = 0;
        uint64_t cscore[TOPK];
        for (p = 0; p < 32u; ++p) {
            int32_t rlo[2][3], rhi[2][3];
            uint64_t score = 0;
            int have[2] = {0, 0};
            for (c = 0; c < 3u; ++c) {
                rlo[0][c] = 63; rlo[1][c] = 63;
                rhi[0][c] = -64; rhi[1][c] = -64;
            }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
                have[reg] = 1;
            }
            if (!have[0] || !have[1]) continue;
            for (c = 0; c < 3u; ++c) {
                int64_t e0 = (int64_t)rhi[0][c] - (int64_t)rlo[0][c];
                int64_t e1 = (int64_t)rhi[1][c] - (int64_t)rlo[1][c];
                score += (uint64_t)(e0 * e0 + e1 * e1);
            }
            if (ncand < TOPK) {
                cand[ncand] = p;
                cscore[ncand] = score;
                ++ncand;
            } else {
                uint32_t worst = 0;
                for (k = 1; k < TOPK; ++k)
                    if (cscore[k] > cscore[worst]) worst = k;
                if (score < cscore[worst]) { cand[worst] = p; cscore[worst] = score; }
            }
        }
        for (k = 0; k < ncand; ++k) {
            int32_t ep[4][3], rlo[2][3], rhi[2][3];
            uint8_t sel[16];
            uint64_t err;
            p = cand[k];
            for (c = 0; c < 3u; ++c) {
                rlo[0][c] = 63; rlo[1][c] = 63;
                rhi[0][c] = -64; rhi[1][c] = -64;
            }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
            }
            for (c = 0; c < 3u; ++c) {
                ep[0][c] = rlo[0][c];
                ep[1][c] = rhi[0][c];
                ep[2][c] = rlo[1][c];
                ep[3][c] = rhi[1][c];
            }
            err = tc_bc6h_mode9_selectors_sf16(q, target, tc_bc6h_part2[p], ep, sel);
            if (err < best_err) {
                best_err = err;
                best_p = (int)p;
                memcpy(bep, ep, sizeof(ep));
                memcpy(bsel, sel, 16u);
            }
        }
    }
    if (best_p < 0) return (uint64_t)-1;
    {
        static const uint32_t w3[8] = {0, 9, 18, 27, 37, 46, 55, 64};
        int round, rr;
        for (round = 0; round < 2; ++round) {
            int32_t nep[4][3];
            uint8_t nsel[16];
            uint64_t e;
            for (rr = 0; rr < 2; ++rr)
                for (c = 0; c < 3u; ++c) {
                    int64_t saa = 0, sab = 0, sbb = 0, sap = 0, sbp = 0, det, l, h;
                    for (i = 0; i < 16u; ++i) {
                        int64_t bb, a, pp;
                        if ((int)tc_bc6h_part2[best_p][i] != rr) continue;
                        bb = w3[bsel[i]];
                        a = 64 - bb;
                        pp = q[i][c];
                        saa += a * a;
                        sab += a * bb;
                        sbb += bb * bb;
                        sap += a * pp;
                        sbp += bb * pp;
                    }
                    det = saa * sbb - sab * sab;
                    if (det <= 0) {
                        nep[rr * 2][c] = bep[rr * 2][c];
                        nep[rr * 2 + 1][c] = bep[rr * 2 + 1][c];
                        continue;
                    }
                    l = tc_bc6h_rdiv((sap * sbb - sbp * sab) * 64, det);
                    h = tc_bc6h_rdiv((sbp * saa - sap * sab) * 64, det);
                    if (l < -32) l = -32;
                    if (l > 31) l = 31;
                    if (h < -32) h = -32;
                    if (h > 31) h = 31;
                    nep[rr * 2][c] = (int32_t)l;
                    nep[rr * 2 + 1][c] = (int32_t)h;
                }
            e = tc_bc6h_mode9_selectors_sf16(q, target, tc_bc6h_part2[best_p], nep, nsel);
            if (e < best_err) {
                best_err = e;
                memcpy(bep, nep, sizeof(nep));
                memcpy(bsel, nsel, 16u);
            } else {
                break;
            }
        }
    }
    anchor = tc_bc6h_part2_anchor[best_p];
    for (r = 0; r < 2; ++r) {
        uint32_t at = (r == 0) ? 0u : anchor;
        if (bsel[at] & 4u) {
            for (c = 0; c < 3u; ++c) {
                int32_t t = bep[r * 2][c];
                bep[r * 2][c] = bep[r * 2 + 1][c];
                bep[r * 2 + 1][c] = t;
            }
            for (i = 0; i < 16u; ++i)
                if (tc_bc6h_part2[best_p][i] == (uint32_t)r) bsel[i] = (uint8_t)(7u - bsel[i]);
        }
    }
    for (c = 0; c < 3u; ++c) {
        uint32_t *E = (c == 0) ? R : (c == 1) ? G : B;
        E[0] = (uint32_t)(bep[0][c] & 63u);
        E[1] = (uint32_t)(bep[1][c] & 63u);
        E[2] = (uint32_t)(bep[2][c] & 63u);
        E[3] = (uint32_t)(bep[3][c] & 63u);
    }
    memset(out, 0, 16u);
    tc_bc6h_w(out, &bitpos, 30u, 5);
    tc_bc6h_w(out, &bitpos, R[0], 6);
    tc_bc6h_w(out, &bitpos, (G[3] >> 4) & 1u, 1);
    tc_bc6h_w(out, &bitpos, B[3] & 1u, 1);
    tc_bc6h_w(out, &bitpos, (B[3] >> 1) & 1u, 1);
    tc_bc6h_w(out, &bitpos, (B[2] >> 4) & 1u, 1);
    tc_bc6h_w(out, &bitpos, G[0], 6);
    tc_bc6h_w(out, &bitpos, (G[2] >> 5) & 1u, 1);
    tc_bc6h_w(out, &bitpos, (B[2] >> 5) & 1u, 1);
    tc_bc6h_w(out, &bitpos, (B[3] >> 2) & 1u, 1);
    tc_bc6h_w(out, &bitpos, (G[2] >> 4) & 1u, 1);
    tc_bc6h_w(out, &bitpos, B[0], 6);
    tc_bc6h_w(out, &bitpos, (G[3] >> 5) & 1u, 1);
    tc_bc6h_w(out, &bitpos, (B[3] >> 3) & 1u, 1);
    tc_bc6h_w(out, &bitpos, (B[3] >> 5) & 1u, 1);
    tc_bc6h_w(out, &bitpos, (B[3] >> 4) & 1u, 1);
    tc_bc6h_w(out, &bitpos, R[1], 6);
    tc_bc6h_w(out, &bitpos, G[2] & 0xfu, 4);
    tc_bc6h_w(out, &bitpos, G[1], 6);
    tc_bc6h_w(out, &bitpos, G[3] & 0xfu, 4);
    tc_bc6h_w(out, &bitpos, B[1], 6);
    tc_bc6h_w(out, &bitpos, B[2] & 0xfu, 4);
    tc_bc6h_w(out, &bitpos, R[2], 6);
    tc_bc6h_w(out, &bitpos, R[3], 6);
    tc_bc6h_w(out, &bitpos, (uint32_t)best_p, 5);
    for (i = 0; i < 16u; ++i) {
        uint32_t nb = (i == 0u || i == anchor) ? 2u : 3u;
        tc_bc6h_w(out, &bitpos, bsel[i], nb);
    }
    return best_err;
}

/* Signed variant of mode0 (two-region, 10-bit primary + 5-bit signed deltas). */
static uint64_t tc_bc6h_mode0_sf16(const float pix[16][3], uint8_t out[16]) {
    int32_t q[16][3], target[16][3], bep[4][3];
    uint32_t i, c, p, r;
    uint8_t bsel[16];
    int best_p = -1;
    uint64_t best_err = (uint64_t)-1;
    uint32_t bitpos = 0, anchor;
    for (i = 0; i < 16u; ++i)
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_sf16(pix[i][c]);
            target[i][c] = tc_bc6h_unquant_sf16_to_smag(q[i][c]);
        }
    {
        enum { TOPK = 5u };
        uint32_t cand[TOPK], k, ncand = 0;
        uint64_t cscore[TOPK];
        for (p = 0; p < 32u; ++p) {
            int32_t rlo[2][3], rhi[2][3];
            uint64_t score = 0;
            int have[2] = {0, 0};
            for (c = 0; c < 3u; ++c) {
                rlo[0][c] = 511; rlo[1][c] = 511;
                rhi[0][c] = -512; rhi[1][c] = -512;
            }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
                have[reg] = 1;
            }
            if (!have[0] || !have[1]) continue;
            for (c = 0; c < 3u; ++c) {
                int64_t e0 = (int64_t)rhi[0][c] - (int64_t)rlo[0][c];
                int64_t e1 = (int64_t)rhi[1][c] - (int64_t)rlo[1][c];
                score += (uint64_t)(e0 * e0 + e1 * e1);
            }
            if (ncand < TOPK) {
                cand[ncand] = p;
                cscore[ncand] = score;
                ++ncand;
            } else {
                uint32_t worst = 0;
                for (k = 1; k < TOPK; ++k)
                    if (cscore[k] > cscore[worst]) worst = k;
                if (score < cscore[worst]) { cand[worst] = p; cscore[worst] = score; }
            }
        }
        for (k = 0; k < ncand; ++k) {
            int32_t ep[4][3], rlo[2][3], rhi[2][3];
            uint8_t sel[16];
            uint64_t err;
            int fit = 1;
            p = cand[k];
            for (c = 0; c < 3u; ++c) {
                rlo[0][c] = 511; rlo[1][c] = 511;
                rhi[0][c] = -512; rhi[1][c] = -512;
            }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
            }
            for (c = 0; c < 3u && fit; ++c) {
                int32_t d1 = rhi[0][c] - rlo[0][c];
                int32_t d2 = rlo[1][c] - rlo[0][c];
                int32_t d3 = rhi[1][c] - rlo[0][c];
                if (d1 < -16 || d1 > 15 || d2 < -16 || d2 > 15 || d3 < -16 || d3 > 15)
                    fit = 0;
            }
            if (!fit) continue;
            for (c = 0; c < 3u; ++c) {
                ep[0][c] = rlo[0][c];
                ep[1][c] = rhi[0][c];
                ep[2][c] = rlo[1][c];
                ep[3][c] = rhi[1][c];
            }
            err = tc_bc6h_mode0_selectors_sf16(target, tc_bc6h_part2[p], ep, sel);
            if (err < best_err) {
                best_err = err;
                best_p = (int)p;
                memcpy(bep, ep, sizeof(ep));
                memcpy(bsel, sel, 16u);
            }
        }
    }
    if (best_p < 0) return (uint64_t)-1;
    {
        static const uint32_t w3[8] = {0, 9, 18, 27, 37, 46, 55, 64};
        int round, rr;
        for (round = 0; round < 2; ++round) {
            int32_t nep[4][3];
            uint8_t nsel[16];
            uint64_t e;
            int fit = 1;
            for (rr = 0; rr < 2; ++rr)
                for (c = 0; c < 3u; ++c) {
                    int64_t saa = 0, sab = 0, sbb = 0, sap = 0, sbp = 0, det, l, h;
                    for (i = 0; i < 16u; ++i) {
                        int64_t bb, a, pp;
                        if ((int)tc_bc6h_part2[best_p][i] != rr) continue;
                        bb = w3[bsel[i]];
                        a = 64 - bb;
                        pp = q[i][c];
                        saa += a * a;
                        sab += a * bb;
                        sbb += bb * bb;
                        sap += a * pp;
                        sbp += bb * pp;
                    }
                    det = saa * sbb - sab * sab;
                    if (det <= 0) {
                        nep[rr * 2][c] = bep[rr * 2][c];
                        nep[rr * 2 + 1][c] = bep[rr * 2 + 1][c];
                        continue;
                    }
                    l = tc_bc6h_rdiv((sap * sbb - sbp * sab) * 64, det);
                    h = tc_bc6h_rdiv((sbp * saa - sap * sab) * 64, det);
                    if (l < -512) l = -512;
                    if (l > 511) l = 511;
                    if (h < -512) h = -512;
                    if (h > 511) h = 511;
                    nep[rr * 2][c] = (int32_t)l;
                    nep[rr * 2 + 1][c] = (int32_t)h;
                }
            for (c = 0; c < 3u && fit; ++c) {
                int32_t d1 = nep[1][c] - nep[0][c];
                int32_t d2 = nep[2][c] - nep[0][c];
                int32_t d3 = nep[3][c] - nep[0][c];
                if (d1 < -16 || d1 > 15 || d2 < -16 || d2 > 15 || d3 < -16 || d3 > 15)
                    fit = 0;
            }
            if (!fit) break;
            e = tc_bc6h_mode0_selectors_sf16(target, tc_bc6h_part2[best_p], nep, nsel);
            if (e < best_err) {
                best_err = e;
                memcpy(bep, nep, sizeof(nep));
                memcpy(bsel, nsel, 16u);
            } else {
                break;
            }
        }
    }
    anchor = tc_bc6h_part2_anchor[best_p];
    for (r = 0; r < 2; ++r) {
        uint32_t at = (r == 0) ? 0u : anchor;
        if (bsel[at] & 4u) {
            for (c = 0; c < 3u; ++c) {
                int32_t t = bep[r * 2][c];
                bep[r * 2][c] = bep[r * 2 + 1][c];
                bep[r * 2 + 1][c] = t;
            }
            for (i = 0; i < 16u; ++i)
                if (tc_bc6h_part2[best_p][i] == (uint32_t)r) bsel[i] = (uint8_t)(7u - bsel[i]);
        }
    }
    memset(out, 0, 16u);
    bitpos = 2;  /* Skip 2-bit mode field (00 for mode 0) */
    tc_bc6h_w(out, &bitpos, (bep[2][1] >> 4) & 1u, 1);
    tc_bc6h_w(out, &bitpos, (bep[2][2] >> 4) & 1u, 1);
    tc_bc6h_w(out, &bitpos, (bep[3][2] >> 4) & 1u, 1);
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_signed10(bep[0][0]), 10);
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_signed10(bep[0][1]), 10);
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_signed10(bep[0][2]), 10);
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(bep[1][0] - bep[0][0]), 5);
    tc_bc6h_w(out, &bitpos, (bep[3][1] >> 4) & 1u, 1);
    tc_bc6h_w(out, &bitpos, bep[2][1] & 0xfu, 4);
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(bep[1][1] - bep[0][1]), 5);
    tc_bc6h_w(out, &bitpos, bep[3][2] & 1u, 1);
    tc_bc6h_w(out, &bitpos, bep[3][1] & 0xfu, 4);
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(bep[1][2] - bep[0][2]), 5);
    tc_bc6h_w(out, &bitpos, (bep[3][2] >> 1) & 1u, 1);
    tc_bc6h_w(out, &bitpos, bep[2][2] & 0xfu, 4);
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(bep[2][0] - bep[0][0]), 5);
    tc_bc6h_w(out, &bitpos, (bep[3][2] >> 2) & 1u, 1);
    tc_bc6h_w(out, &bitpos, tc_bc6h_pack_delta5(bep[3][0] - bep[0][0]), 5);
    tc_bc6h_w(out, &bitpos, (bep[3][2] >> 3) & 1u, 1);
    tc_bc6h_w(out, &bitpos, (uint32_t)best_p, 5);
    for (i = 0; i < 16u; ++i) {
        uint32_t nb = (i == 0u || i == anchor) ? 2u : 3u;
        tc_bc6h_w(out, &bitpos, bsel[i], nb);
    }
    return best_err;
}

static uint64_t tc_bc6h_mode1_sf16(const float pix[16][3], uint8_t out[16]) {
    int32_t q[16][3], target[16][3], bep[4][3];
    uint32_t i, c, p;
    uint8_t bsel[16];
    int best_p = -1;
    uint64_t best_err = (uint64_t)-1;
    uint32_t bitpos = 0, anchor;
    int r;
    for (i = 0; i < 16u; ++i)
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_sf16_n(pix[i][c], 7);
            target[i][c] = tc_bc6h_unquant_sf16_to_smag(tc_bc6h_quant_sf16(pix[i][c]));
        }
    {
        enum { TOPK = 5u };
        int32_t half_max = 64;
        uint32_t cand[TOPK], k, ncand = 0;
        uint64_t cscore[TOPK];
        for (p = 0; p < 32u; ++p) {
            int32_t rlo[2][3], rhi[2][3];
            uint64_t score = 0;
            int have[2] = {0, 0};
            for (c = 0; c < 3u; ++c) {
                rlo[0][c] = rlo[1][c] = half_max - 1;
                rhi[0][c] = rhi[1][c] = -half_max;
            }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
                have[reg] = 1;
            }
            if (!have[0] || !have[1]) continue;
            for (c = 0; c < 3u; ++c) {
                uint64_t e0 = rhi[0][c] - rlo[0][c], e1 = rhi[1][c] - rlo[1][c];
                score += e0 * e0 + e1 * e1;
            }
            if (ncand < TOPK) { cand[ncand] = p; cscore[ncand] = score; ++ncand; }
            else { uint32_t worst = 0; for (k = 1; k < TOPK; ++k) if (cscore[k] > cscore[worst]) worst = k; if (score < cscore[worst]) { cand[worst] = p; cscore[worst] = score; } }
        }
        for (k = 0; k < ncand; ++k) {
            int32_t ep[4][3], rlo[2][3], rhi[2][3];
            uint8_t sel[16]; uint64_t err; int fit = 1;
            p = cand[k];
            for (c = 0; c < 3u; ++c) { rlo[0][c]=rlo[1][c]=half_max-1; rhi[0][c]=rhi[1][c]=-half_max; }
            for (i = 0; i < 16u; ++i) { uint32_t reg=tc_bc6h_part2[p][i]; for(c=0;c<3u;++c){if(q[i][c]<rlo[reg][c])rlo[reg][c]=q[i][c];if(q[i][c]>rhi[reg][c])rhi[reg][c]=q[i][c];} }
            for (c = 0; c < 3u && fit; ++c) {
                int32_t d1=rhi[0][c]-rlo[0][c],d2=rlo[1][c]-rlo[0][c],d3=rhi[1][c]-rlo[0][c];
                if (d1<-32||d1>31||d2<-32||d2>31||d3<-32||d3>31) fit=0;
            }
            if (!fit) continue;
            for(c=0;c<3u;++c){ep[0][c]=rlo[0][c];ep[1][c]=rhi[0][c];ep[2][c]=rlo[1][c];ep[3][c]=rhi[1][c];}
            err = tc_bc6h_mode0_selectors_sf16(target, tc_bc6h_part2[p], ep, sel);
            if (err < best_err) { best_err=err; best_p=(int)p; memcpy(bep,ep,sizeof(ep)); memcpy(bsel,sel,16u); }
        }
    }
    if (best_p < 0) return (uint64_t)-1;
    {
        static const uint32_t w3[8]={0,9,18,27,37,46,55,64}; int round,rr; int32_t half_max=64;
        for(round=0;round<2;++round){int32_t nep[4][3];uint8_t nsel[16];uint64_t e;int fit=1;
        for(rr=0;rr<2;++rr) for(c=0;c<3u;++c){int64_t saa=0,sab=0,sbb=0,sap=0,sbp=0,det,l,h;
        for(i=0;i<16u;++i){int64_t wv,a,pp;if((int)tc_bc6h_part2[best_p][i]!=rr)continue;wv=w3[bsel[i]];a=64-wv;pp=q[i][c];
        saa+=a*a;sab+=a*wv;sbb+=wv*wv;sap+=a*pp;sbp+=wv*pp;}
        det=saa*sbb-sab*sab;if(det<=0){nep[rr*2][c]=bep[rr*2][c];nep[rr*2+1][c]=bep[rr*2+1][c];continue;}
        l=tc_bc6h_rdiv((sap*sbb-sbp*sab)*64,det);h=tc_bc6h_rdiv((sbp*saa-sap*sab)*64,det);
        if(l<-half_max){l=-half_max;}if(l>half_max-1){l=half_max-1;}if(h<-half_max){h=-half_max;}if(h>half_max-1){h=half_max-1;}
        nep[rr*2][c]=(int32_t)l;nep[rr*2+1][c]=(int32_t)h;}
        for(c=0;c<3u&&fit;++c){int32_t d1=nep[1][c]-nep[0][c],d2=nep[2][c]-nep[0][c],d3=nep[3][c]-nep[0][c];
        if(d1<-32||d1>31||d2<-32||d2>31||d3<-32||d3>31)fit=0;}
        if(!fit){break;} e=tc_bc6h_mode0_selectors_sf16(target,tc_bc6h_part2[best_p],nep,nsel);
        if(e<best_err){best_err=e;memcpy(bep,nep,sizeof(nep));memcpy(bsel,nsel,16u);}else break;}
    }
    anchor = tc_bc6h_part2_anchor[best_p];
    for(r=0;r<2;++r){uint32_t at=(r==0)?0u:anchor;if(bsel[at]&4u){
      for(c=0;c<3u;++c){int32_t t=bep[r*2][c];bep[r*2][c]=bep[r*2+1][c];bep[r*2+1][c]=t;}
      for(i=0;i<16u;++i) if((int)tc_bc6h_part2[best_p][i]==r) bsel[i]=(uint8_t)(7u-bsel[i]);}}
    memset(out,0,16u);
    tc_bc6h_w(out,&bitpos,1u,2); /* Write mode field 01 at bits 0-1 */
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[2][1]>>5)&1u,1);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][1]>>4)&1u,1);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][1]>>5)&1u,1);
    tc_bc6h_w(out,&bitpos,tc_bc6h_pack_signed_n(bep[0][0],7),7);
    tc_bc6h_w(out,&bitpos,(uint32_t)bep[3][2]&1u,1);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>1)&1u,1);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[2][2]>>4)&1u,1);
    tc_bc6h_w(out,&bitpos,tc_bc6h_pack_signed_n(bep[0][1],7),7);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[2][2]>>5)&1u,1);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>2)&1u,1);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[2][1]>>4)&1u,1);
    tc_bc6h_w(out,&bitpos,tc_bc6h_pack_signed_n(bep[0][2],7),7);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>3)&1u,1);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>5)&1u,1);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>4)&1u,1);
    tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5((int32_t)bep[1][0]-(int32_t)bep[0][0]),6);
    tc_bc6h_w(out,&bitpos,(uint32_t)bep[2][1]&0xfu,4);
    tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5((int32_t)bep[1][1]-(int32_t)bep[0][1]),6);
    tc_bc6h_w(out,&bitpos,(uint32_t)bep[3][1]&0xfu,4);
    tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5((int32_t)bep[1][2]-(int32_t)bep[0][2]),6);
    tc_bc6h_w(out,&bitpos,(uint32_t)bep[2][2]&0xfu,4);
    tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5((int32_t)bep[2][0]-(int32_t)bep[0][0]),6);
    tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5((int32_t)bep[3][0]-(int32_t)bep[0][0]),6);
    tc_bc6h_w(out,&bitpos,(uint32_t)best_p,5);
    for(i=0;i<16u;++i){uint32_t nb=(i==0u||i==anchor)?2u:3u; tc_bc6h_w(out,&bitpos,bsel[i],nb);}
    return best_err;
}

static uint64_t tc_bc6h_mode5_sf16(const float pix[16][3], uint8_t out[16]) {
    int32_t q[16][3], target[16][3];
    uint32_t i, c, p;
    int32_t bep[4][3];
    uint8_t bsel[16];
    int best_p = -1;
    uint64_t best_err = (uint64_t)-1;
    uint32_t bitpos = 0, anchor;
    int r;
    for (i = 0; i < 16u; ++i)
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_sf16_n(pix[i][c], 9);
            target[i][c] = tc_bc6h_unquant_sf16_to_smag(tc_bc6h_quant_sf16(pix[i][c]));
        }
    {
        enum { TOPK = 5u };
        int32_t half_max = 256;
        uint32_t cand[TOPK], k, ncand = 0;
        uint64_t cscore[TOPK];
        for (p = 0; p < 32u; ++p) {
            int32_t rlo[2][3], rhi[2][3];
            uint64_t score = 0;
            int have[2] = {0, 0};
            for (c = 0; c < 3u; ++c) {
                rlo[0][c]=rlo[1][c]=half_max-1; rhi[0][c]=rhi[1][c]=-half_max;
            }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
                have[reg] = 1;
            }
            if (!have[0] || !have[1]) continue;
            for (c = 0; c < 3u; ++c) score += (uint64_t)(rhi[0][c]-rlo[0][c])*(rhi[0][c]-rlo[0][c]) + (uint64_t)(rhi[1][c]-rlo[1][c])*(rhi[1][c]-rlo[1][c]);
            if (ncand<TOPK) { cand[ncand]=p; cscore[ncand]=score; ++ncand; }
            else { uint32_t worst=0; for(k=1;k<TOPK;++k) if(cscore[k]>cscore[worst]) worst=k; if(score<cscore[worst]){ cand[worst]=p; cscore[worst]=score; } }
        }
        for (k = 0; k < ncand; ++k) {
            int32_t ep[4][3], rlo[2][3], rhi[2][3];
            uint8_t sel[16]; uint64_t err; int fit = 1;
            p = cand[k];
            for (c = 0; c < 3u; ++c) { rlo[0][c]=rlo[1][c]=half_max-1; rhi[0][c]=rhi[1][c]=-half_max; }
            for (i = 0; i < 16u; ++i) { uint32_t reg=tc_bc6h_part2[p][i]; for(c=0;c<3u;++c){if(q[i][c]<rlo[reg][c])rlo[reg][c]=q[i][c];if(q[i][c]>rhi[reg][c])rhi[reg][c]=q[i][c];} }
            for (c = 0; c < 3u && fit; ++c) {
                int32_t d1=rhi[0][c]-rlo[0][c],d2=rlo[1][c]-rlo[0][c],d3=rhi[1][c]-rlo[0][c];
                if (d1<-16||d1>15||d2<-16||d2>15||d3<-16||d3>15) fit=0;
            }
            if (!fit) continue;
            for(c=0;c<3u;++c){ep[0][c]=rlo[0][c];ep[1][c]=rhi[0][c];ep[2][c]=rlo[1][c];ep[3][c]=rhi[1][c];}
            err = tc_bc6h_mode0_selectors_sf16(target, tc_bc6h_part2[p], ep, sel);
            if (err < best_err) { best_err=err; best_p=(int)p; memcpy(bep,ep,sizeof(ep)); memcpy(bsel,sel,16u); }
        }
    }
    if (best_p < 0) return (uint64_t)-1;
    {
        static const uint32_t w3[8]={0,9,18,27,37,46,55,64}; int round,rr; int32_t half_max=256;
        for(round=0;round<2;++round){int32_t nep[4][3];uint8_t nsel[16];uint64_t e;int fit=1;
        for(rr=0;rr<2;++rr) for(c=0;c<3u;++c){int64_t saa=0,sab=0,sbb=0,sap=0,sbp=0,det,l,h;
        for(i=0;i<16u;++i){int64_t wv,a,pp;if((int)tc_bc6h_part2[best_p][i]!=rr)continue;wv=w3[bsel[i]];a=64-wv;pp=q[i][c];
        saa+=a*a;sab+=a*wv;sbb+=wv*wv;sap+=a*pp;sbp+=wv*pp;}
        det=saa*sbb-sab*sab;if(det<=0){nep[rr*2][c]=bep[rr*2][c];nep[rr*2+1][c]=bep[rr*2+1][c];continue;}
        l=tc_bc6h_rdiv((sap*sbb-sbp*sab)*64,det);h=tc_bc6h_rdiv((sbp*saa-sap*sab)*64,det);
        if(l<-half_max){l=-half_max;}if(l>half_max-1){l=half_max-1;}if(h<-half_max){h=-half_max;}if(h>half_max-1){h=half_max-1;}
        nep[rr*2][c]=(int32_t)l;nep[rr*2+1][c]=(int32_t)h;}
        for(c=0;c<3u&&fit;++c){int32_t d1=nep[1][c]-nep[0][c],d2=nep[2][c]-nep[0][c],d3=nep[3][c]-nep[0][c];
        if(d1<-16||d1>15||d2<-16||d2>15||d3<-16||d3>15)fit=0;}
        if(!fit){break;} e=tc_bc6h_mode0_selectors_sf16(target,tc_bc6h_part2[best_p],nep,nsel);
        if(e<best_err){best_err=e;memcpy(bep,nep,sizeof(nep));memcpy(bsel,nsel,16u);}else break;}
    }
    anchor = tc_bc6h_part2_anchor[best_p];
    for(r=0;r<2;++r){uint32_t at=(r==0)?0u:anchor;if(bsel[at]&4u){
      for(c=0;c<3u;++c){int32_t t=bep[r*2][c];bep[r*2][c]=bep[r*2+1][c];bep[r*2+1][c]=t;}
      for(i=0;i<16u;++i) if((int)tc_bc6h_part2[best_p][i]==r) bsel[i]=(uint8_t)(7u-bsel[i]);}}
    memset(out,0,16u);
    tc_bc6h_w(out,&bitpos,0x0eu,5); /* Write mode field 01110 */
    tc_bc6h_w(out,&bitpos,tc_bc6h_pack_signed_n(bep[0][0],9),9);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[2][2]>>4)&1u,1);
    tc_bc6h_w(out,&bitpos,tc_bc6h_pack_signed_n(bep[0][1],9),9);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[2][1]>>4)&1u,1);
    tc_bc6h_w(out,&bitpos,tc_bc6h_pack_signed_n(bep[0][2],9),9);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>4)&1u,1);
    tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5((int32_t)bep[1][0]-(int32_t)bep[0][0]),5);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][1]>>4)&1u,1);
    tc_bc6h_w(out,&bitpos,(uint32_t)bep[2][1]&0xfu,4);
    tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5((int32_t)bep[1][1]-(int32_t)bep[0][1]),5);
    tc_bc6h_w(out,&bitpos,(uint32_t)bep[3][2]&1u,1);
    tc_bc6h_w(out,&bitpos,(uint32_t)bep[3][1]&0xfu,4);
    tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5((int32_t)bep[1][2]-(int32_t)bep[0][2]),5);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>1)&1u,1);
    tc_bc6h_w(out,&bitpos,(uint32_t)bep[2][2]&0xfu,4);
    tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5((int32_t)bep[2][0]-(int32_t)bep[0][0]),5);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>2)&1u,1);
    tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5((int32_t)bep[3][0]-(int32_t)bep[0][0]),5);
    tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>3)&1u,1);
    tc_bc6h_w(out,&bitpos,(uint32_t)best_p,5);
    for(i=0;i<16u;++i){uint32_t nb=(i==0u||i==anchor)?2u:3u; tc_bc6h_w(out,&bitpos,bsel[i],nb);}
    return best_err;
}

static uint64_t tc_bc6h_mode12_sf16(const float pix[16][3], uint8_t out[16]) {
    int32_t q[16][3], target[16][3], lo_[3], hi_[3], luma_lo[3], luma_hi[3];
    uint32_t i, c, bitpos = 0, min_l = UINT_MAX, max_l = 0, min_i = 0, max_i = 0;
    uint8_t sel[16], box_sel[16], luma_sel[16];
    memset(out, 0, 16);
    for (c = 0; c < 3u; ++c) { lo_[c] = 2047; hi_[c] = -2048; }
    for (i = 0; i < 16u; ++i) {
        uint32_t l;
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_sf16_n(pix[i][c], 12);
            target[i][c] = tc_bc6h_unquant_sf16_to_smag(tc_bc6h_quant_sf16(pix[i][c]));
            if (q[i][c] < lo_[c]) lo_[c] = q[i][c];
            if (q[i][c] > hi_[c]) hi_[c] = q[i][c];
        }
        l = (uint32_t)(target[i][0] * 38 + target[i][1] * 76 + target[i][2] * 14);
        if (l < min_l) { min_l = l; min_i = i; }
        if (l >= max_l) { max_l = l; max_i = i; }
    }
    for (c = 0; c < 3u; ++c) { luma_lo[c] = q[min_i][c]; luma_hi[c] = q[max_i][c]; }
    if (tc_bc6h_choose_selectors_sf16(target, luma_lo, luma_hi, luma_sel) <
        tc_bc6h_choose_selectors_sf16(target, lo_, hi_, box_sel)) {
        memcpy(lo_, luma_lo, sizeof(lo_)); memcpy(hi_, luma_hi, sizeof(hi_));
        memcpy(sel, luma_sel, sizeof(sel));
    } else { memcpy(sel, box_sel, sizeof(sel)); }
    (void)tc_bc6h_refine_sf16(q, target, lo_, hi_, sel);
    if (sel[0] & 8u) {
        uint32_t t;
        for (c = 0; c < 3u; ++c) { t = (uint32_t)lo_[c]; lo_[c] = hi_[c]; hi_[c] = (int32_t)t; }
        for (i = 0; i < 16u; ++i) sel[i] = (uint8_t)(15u - sel[i]);
    }
    {
        int32_t r0_p = tc_bc6h_pack_signed_n(lo_[0], 12);
        int32_t g0_p = tc_bc6h_pack_signed_n(lo_[1], 12);
        int32_t b0_p = tc_bc6h_pack_signed_n(lo_[2], 12);
        uint32_t r0_lo = (uint32_t)r0_p & 1023u, r0_hi = (uint32_t)(r0_p >> 10) & 3u;
        uint32_t g0_lo = (uint32_t)g0_p & 1023u, g0_hi = (uint32_t)(g0_p >> 10) & 3u;
        uint32_t b0_lo = (uint32_t)b0_p & 1023u, b0_hi = (uint32_t)(b0_p >> 10) & 3u;
        int32_t dr = hi_[0] - lo_[0];
        int32_t dg = hi_[1] - lo_[1];
        int32_t db = hi_[2] - lo_[2];
        if (dr < -128 || dr > 127 || dg < -128 || dg > 127 || db < -128 || db > 127)
            return (uint64_t)-1;
        tc_bc6h_w(out, &bitpos, 0x0bu, 5);
        tc_bc6h_w(out, &bitpos, r0_lo, 10);
        tc_bc6h_w(out, &bitpos, g0_lo, 10);
        tc_bc6h_w(out, &bitpos, b0_lo, 10);
        tc_bc6h_w(out, &bitpos, (uint32_t)(dr & 0xffu), 8);
        tc_bc6h_w(out, &bitpos, r0_hi, 2);
        tc_bc6h_w(out, &bitpos, (uint32_t)(dg & 0xffu), 8);
        tc_bc6h_w(out, &bitpos, g0_hi, 2);
        tc_bc6h_w(out, &bitpos, (uint32_t)(db & 0xffu), 8);
        tc_bc6h_w(out, &bitpos, b0_hi, 2);
        tc_bc6h_w(out, &bitpos, sel[0], 3);
        for (i = 1; i < 16u; ++i) tc_bc6h_w(out, &bitpos, sel[i], 4);
    }
    return tc_bc6h_choose_selectors_sf16(target, lo_, hi_, sel);
}

static uint64_t tc_bc6h_mode13_sf16(const float pix[16][3], uint8_t out[16]) {
    int32_t q[16][3], target[16][3], lo_[3], hi_[3], luma_lo[3], luma_hi[3];
    uint32_t i, c, bitpos = 0, min_l = UINT_MAX, max_l = 0, min_i = 0, max_i = 0;
    uint8_t sel[16], box_sel[16], luma_sel[16];
    memset(out, 0, 16);
    for (c = 0; c < 3u; ++c) { lo_[c] = 32767; hi_[c] = -32768; }
    for (i = 0; i < 16u; ++i) {
        uint32_t l;
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_sf16_n(pix[i][c], 16);
            target[i][c] = tc_bc6h_unquant_sf16_to_smag(tc_bc6h_quant_sf16(pix[i][c]));
            if (q[i][c] < lo_[c]) lo_[c] = q[i][c];
            if (q[i][c] > hi_[c]) hi_[c] = q[i][c];
        }
        l = (uint32_t)(target[i][0] * 38 + target[i][1] * 76 + target[i][2] * 14);
        if (l < min_l) { min_l = l; min_i = i; }
        if (l >= max_l) { max_l = l; max_i = i; }
    }
    for (c = 0; c < 3u; ++c) { luma_lo[c] = q[min_i][c]; luma_hi[c] = q[max_i][c]; }
    if (tc_bc6h_choose_selectors_sf16(target, luma_lo, luma_hi, luma_sel) <
        tc_bc6h_choose_selectors_sf16(target, lo_, hi_, box_sel)) {
        memcpy(lo_, luma_lo, sizeof(lo_)); memcpy(hi_, luma_hi, sizeof(hi_));
        memcpy(sel, luma_sel, sizeof(sel));
    } else { memcpy(sel, box_sel, sizeof(sel)); }
    (void)tc_bc6h_refine_sf16(q, target, lo_, hi_, sel);
    if (sel[0] & 8u) {
        uint32_t t;
        for (c = 0; c < 3u; ++c) { t = (uint32_t)lo_[c]; lo_[c] = hi_[c]; hi_[c] = (int32_t)t; }
        for (i = 0; i < 16u; ++i) sel[i] = (uint8_t)(15u - sel[i]);
    }
    {
        int32_t r0_p = tc_bc6h_pack_signed_n(lo_[0], 16);
        int32_t g0_p = tc_bc6h_pack_signed_n(lo_[1], 16);
        int32_t b0_p = tc_bc6h_pack_signed_n(lo_[2], 16);
        uint32_t r0_lo = (uint32_t)r0_p & 1023u, r0_hi = (uint32_t)(r0_p >> 10) & 63u;
        uint32_t g0_lo = (uint32_t)g0_p & 1023u, g0_hi = (uint32_t)(g0_p >> 10) & 63u;
        uint32_t b0_lo = (uint32_t)b0_p & 1023u, b0_hi = (uint32_t)(b0_p >> 10) & 63u;
        int32_t dr = hi_[0] - lo_[0];
        int32_t dg = hi_[1] - lo_[1];
        int32_t db = hi_[2] - lo_[2];
        if (dr < -8 || dr > 7 || dg < -8 || dg > 7 || db < -8 || db > 7)
            return (uint64_t)-1;
        tc_bc6h_w(out, &bitpos, 0x0fu, 5);
        tc_bc6h_w(out, &bitpos, r0_lo, 10);
        tc_bc6h_w(out, &bitpos, g0_lo, 10);
        tc_bc6h_w(out, &bitpos, b0_lo, 10);
        tc_bc6h_w(out, &bitpos, (uint32_t)(dr & 0xfu), 4);
        tc_bc6h_w(out, &bitpos, r0_hi, 6);
        tc_bc6h_w(out, &bitpos, (uint32_t)(dg & 0xfu), 4);
        tc_bc6h_w(out, &bitpos, g0_hi, 6);
        tc_bc6h_w(out, &bitpos, (uint32_t)(db & 0xfu), 4);
        tc_bc6h_w(out, &bitpos, b0_hi, 6);
        tc_bc6h_w(out, &bitpos, sel[0], 3);
        for (i = 1; i < 16u; ++i) tc_bc6h_w(out, &bitpos, sel[i], 4);
    }
    return tc_bc6h_choose_selectors_sf16(target, lo_, hi_, sel);
}

static uint64_t tc_bc6h_mode234_sf16(const float pix[16][3], const uint8_t dbits[3],
                                     int mode_key, uint8_t out[16]) {
    int32_t q[16][3], target[16][3];
    uint32_t i, c, p;
    int32_t bep[4][3];
    uint8_t bsel[16];
    int best_p = -1;
    uint64_t best_err = (uint64_t)-1;
    uint32_t bitpos = 0, anchor;
    int r;
    for (i = 0; i < 16u; ++i)
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_sf16_n(pix[i][c], 11);
            target[i][c] = tc_bc6h_unquant_sf16_to_smag(tc_bc6h_quant_sf16(pix[i][c]));
        }
    {
        enum { TOPK = 5u };
        int32_t half_max = 1024;
        uint32_t cand[TOPK], k, ncand = 0;
        uint64_t cscore[TOPK];
        for (p = 0; p < 32u; ++p) {
            int32_t rlo[2][3], rhi[2][3];
            uint64_t score = 0;
            int have[2] = {0, 0};
            for (c = 0; c < 3u; ++c) {
                rlo[0][c] = rlo[1][c] = half_max - 1;
                rhi[0][c] = rhi[1][c] = -half_max;
            }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
                have[reg] = 1;
            }
            if (!have[0] || !have[1]) continue;
            for (c = 0; c < 3u; ++c) {
                uint64_t e0 = rhi[0][c] - rlo[0][c], e1 = rhi[1][c] - rlo[1][c];
                score += e0 * e0 + e1 * e1;
            }
            if (ncand < TOPK) { cand[ncand] = p; cscore[ncand] = score; ++ncand; }
            else { uint32_t worst = 0; for (k = 1; k < TOPK; ++k) if (cscore[k] > cscore[worst]) worst = k; if (score < cscore[worst]) { cand[worst] = p; cscore[worst] = score; } }
        }
        for (k = 0; k < ncand; ++k) {
            int32_t ep[4][3], rlo[2][3], rhi[2][3];
            uint8_t sel[16]; uint64_t err; int fit = 1;
            p = cand[k];
            for (c = 0; c < 3u; ++c) { rlo[0][c]=rlo[1][c]=half_max-1; rhi[0][c]=rhi[1][c]=-half_max; }
            for (i = 0; i < 16u; ++i) { uint32_t reg=tc_bc6h_part2[p][i]; for(c=0;c<3u;++c){if(q[i][c]<rlo[reg][c])rlo[reg][c]=q[i][c];if(q[i][c]>rhi[reg][c])rhi[reg][c]=q[i][c];} }
            for (c = 0; c < 3u && fit; ++c) {
                int32_t limit = 1 << (dbits[c] - 1);
                int32_t d1 = rhi[0][c] - rlo[0][c], d2 = rlo[1][c] - rlo[0][c], d3 = rhi[1][c] - rlo[0][c];
                if (d1 < -limit || d1 > limit - 1 || d2 < -limit || d2 > limit - 1 || d3 < -limit || d3 > limit - 1) fit = 0;
            }
            if (!fit) continue;
            for(c=0;c<3u;++c){ep[0][c]=rlo[0][c];ep[1][c]=rhi[0][c];ep[2][c]=rlo[1][c];ep[3][c]=rhi[1][c];}
            err = tc_bc6h_mode0_selectors_sf16(target, tc_bc6h_part2[p], ep, sel);
            if (err < best_err) { best_err=err; best_p=(int)p; memcpy(bep,ep,sizeof(ep)); memcpy(bsel,sel,16u); }
        }
    }
    if (best_p < 0) return (uint64_t)-1;
    {
        static const uint32_t w3[8]={0,9,18,27,37,46,55,64}; int round,rr; int32_t half_max=1024;
        for(round=0;round<2;++round){int32_t nep[4][3];uint8_t nsel[16];uint64_t e;int fit=1;
        for(rr=0;rr<2;++rr) for(c=0;c<3u;++c){int64_t saa=0,sab=0,sbb=0,sap=0,sbp=0,det,l,h;
        for(i=0;i<16u;++i){int64_t wv,a,pp;if((int)tc_bc6h_part2[best_p][i]!=rr)continue;wv=w3[bsel[i]];a=64-wv;pp=q[i][c];
        saa+=a*a;sab+=a*wv;sbb+=wv*wv;sap+=a*pp;sbp+=wv*pp;}
        det=saa*sbb-sab*sab;if(det<=0){nep[rr*2][c]=bep[rr*2][c];nep[rr*2+1][c]=bep[rr*2+1][c];continue;}
        l=tc_bc6h_rdiv((sap*sbb-sbp*sab)*64,det);h=tc_bc6h_rdiv((sbp*saa-sap*sab)*64,det);
        if(l<-half_max){l=-half_max;}if(l>half_max-1){l=half_max-1;}if(h<-half_max){h=-half_max;}if(h>half_max-1){h=half_max-1;}
        nep[rr*2][c]=(int32_t)l;nep[rr*2+1][c]=(int32_t)h;}
        for(c=0;c<3u&&fit;++c){int32_t limit=1<<(dbits[c]-1);int32_t d1=nep[1][c]-nep[0][c],d2=nep[2][c]-nep[0][c],d3=nep[3][c]-nep[0][c];
        if(d1<-limit||d1>limit-1||d2<-limit||d2>limit-1||d3<-limit||d3>limit-1)fit=0;}
        if(!fit){break;} e=tc_bc6h_mode0_selectors_sf16(target,tc_bc6h_part2[best_p],nep,nsel);
        if(e<best_err){best_err=e;memcpy(bep,nep,sizeof(nep));memcpy(bsel,nsel,16u);}else break;}
    }
    anchor = tc_bc6h_part2_anchor[best_p];
    for(r=0;r<2;++r){uint32_t at=(r==0)?0u:anchor;if(bsel[at]&4u){
      for(c=0;c<3u;++c){int32_t t=bep[r*2][c];bep[r*2][c]=bep[r*2+1][c];bep[r*2+1][c]=t;}
      for(i=0;i<16u;++i) if((int)tc_bc6h_part2[best_p][i]==r) bsel[i]=(uint8_t)(7u-bsel[i]);}}
    memset(out,0,16u);
    switch(mode_key){
      case 2: tc_bc6h_w(out,&bitpos,0x02u,5); break;
      case 3: tc_bc6h_w(out,&bitpos,0x06u,5); break;
      case 4: tc_bc6h_w(out,&bitpos,0x0au,5); break;
    }
    tc_bc6h_w(out,&bitpos,(uint32_t)bep[0][0]&1023u,10);
    tc_bc6h_w(out,&bitpos,(uint32_t)bep[0][1]&1023u,10);
    tc_bc6h_w(out,&bitpos,(uint32_t)bep[0][2]&1023u,10);
    switch(mode_key){
      case 2:{int32_t dr1=bep[1][0]-bep[0][0],dg1=bep[1][1]-bep[0][1],db1=bep[1][2]-bep[0][2],dr2=bep[2][0]-bep[0][0],dr3=bep[3][0]-bep[0][0];
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr1),5);tc_bc6h_w(out,&bitpos,((uint32_t)bep[0][0]>>10)&1u,1);
        tc_bc6h_w(out,&bitpos,(uint32_t)bep[2][1]&0xfu,4);tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dg1),4);
        tc_bc6h_w(out,&bitpos,((uint32_t)bep[0][1]>>10)&1u,1);tc_bc6h_w(out,&bitpos,(uint32_t)bep[3][2]&1u,1);
        tc_bc6h_w(out,&bitpos,(uint32_t)bep[3][1]&0xfu,4);tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(db1),4);
        tc_bc6h_w(out,&bitpos,((uint32_t)bep[0][2]>>10)&1u,1);tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>1)&1u,1);
        tc_bc6h_w(out,&bitpos,(uint32_t)bep[2][2]&0xfu,4);tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr2),5);
        tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>2)&1u,1);tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr3),5);
        tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>3)&1u,1);break;}
      case 3:{int32_t dr1=bep[1][0]-bep[0][0],dg1=bep[1][1]-bep[0][1],db1=bep[1][2]-bep[0][2],dr2=bep[2][0]-bep[0][0],dr3=bep[3][0]-bep[0][0];
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr1),4);tc_bc6h_w(out,&bitpos,((uint32_t)bep[0][0]>>10)&1u,1);
        tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][1]>>4)&1u,1);tc_bc6h_w(out,&bitpos,(uint32_t)bep[2][1]&0xfu,4);
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dg1),5);tc_bc6h_w(out,&bitpos,((uint32_t)bep[0][1]>>10)&1u,1);
        tc_bc6h_w(out,&bitpos,(uint32_t)bep[3][1]&0xfu,4);tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(db1),4);
        tc_bc6h_w(out,&bitpos,((uint32_t)bep[0][2]>>10)&1u,1);tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>1)&1u,1);
        tc_bc6h_w(out,&bitpos,(uint32_t)bep[2][2]&0xfu,4);tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr2),4);
        tc_bc6h_w(out,&bitpos,(uint32_t)bep[3][2]&1u,1);tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>2)&1u,1);
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr3),4);tc_bc6h_w(out,&bitpos,((uint32_t)bep[2][1]>>4)&1u,1);
        tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>3)&1u,1);break;}
      case 4:{int32_t dr1=bep[1][0]-bep[0][0],dg1=bep[1][1]-bep[0][1],db1=bep[1][2]-bep[0][2],dr2=bep[2][0]-bep[0][0],dr3=bep[3][0]-bep[0][0];
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr1),4);tc_bc6h_w(out,&bitpos,((uint32_t)bep[0][0]>>10)&1u,1);
        tc_bc6h_w(out,&bitpos,((uint32_t)bep[2][2]>>4)&1u,1);tc_bc6h_w(out,&bitpos,(uint32_t)bep[2][1]&0xfu,4);
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dg1),4);tc_bc6h_w(out,&bitpos,((uint32_t)bep[0][1]>>10)&1u,1);
        tc_bc6h_w(out,&bitpos,(uint32_t)bep[3][2]&1u,1);tc_bc6h_w(out,&bitpos,(uint32_t)bep[3][1]&0xfu,4);
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(db1),5);tc_bc6h_w(out,&bitpos,((uint32_t)bep[0][2]>>10)&1u,1);
        tc_bc6h_w(out,&bitpos,(uint32_t)bep[2][2]&0xfu,4);tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr2),4);
        tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>1)&1u,1);tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>2)&1u,1);
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5(dr3),4);tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>4)&1u,1);
        tc_bc6h_w(out,&bitpos,((uint32_t)bep[3][2]>>3)&1u,1);break;}
    }
    tc_bc6h_w(out,&bitpos,(uint32_t)best_p,5);
    for(i=0;i<16u;++i){uint32_t nb=(i==0u||i==anchor)?2u:3u; tc_bc6h_w(out,&bitpos,bsel[i],nb);}
    return best_err;
}

static uint64_t tc_bc6h_mode678_sf16(const float pix[16][3], const uint8_t dbits[3],
                                     int mode_key, uint8_t out[16]) {
    (void)mode_key;
    int32_t q[16][3], target[16][3];
    uint32_t i, c, p;
    int32_t bep[4][3];
    uint8_t bsel[16];
    int best_p = -1;
    uint64_t best_err = (uint64_t)-1;
    uint32_t bitpos = 0, anchor;
    int r;
    for (i = 0; i < 16u; ++i)
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_sf16_n(pix[i][c], 8);
            target[i][c] = tc_bc6h_unquant_sf16_to_smag(tc_bc6h_quant_sf16(pix[i][c]));
        }
    {
        enum { TOPK = 5u };
        int32_t half_max = 128;
        uint32_t cand[TOPK], k, ncand = 0;
        uint64_t cscore[TOPK];
        for (p = 0; p < 32u; ++p) {
            int32_t rlo[2][3], rhi[2][3];
            uint64_t score = 0;
            int have[2] = {0, 0};
            for (c = 0; c < 3u; ++c) {
                rlo[0][c] = rlo[1][c] = half_max - 1;
                rhi[0][c] = rhi[1][c] = -half_max;
            }
            for (i = 0; i < 16u; ++i) {
                uint32_t reg = tc_bc6h_part2[p][i];
                for (c = 0; c < 3u; ++c) {
                    if (q[i][c] < rlo[reg][c]) rlo[reg][c] = q[i][c];
                    if (q[i][c] > rhi[reg][c]) rhi[reg][c] = q[i][c];
                }
                have[reg] = 1;
            }
            if (!have[0] || !have[1]) continue;
            for (c = 0; c < 3u; ++c) {
                uint64_t e0 = rhi[0][c] - rlo[0][c], e1 = rhi[1][c] - rlo[1][c];
                score += e0 * e0 + e1 * e1;
            }
            if (ncand < TOPK) { cand[ncand] = p; cscore[ncand] = score; ++ncand; }
            else { uint32_t worst = 0; for (k = 1; k < TOPK; ++k) if (cscore[k] > cscore[worst]) worst = k; if (score < cscore[worst]) { cand[worst] = p; cscore[worst] = score; } }
        }
        for (k = 0; k < ncand; ++k) {
            int32_t ep[4][3], rlo[2][3], rhi[2][3];
            uint8_t sel[16]; uint64_t err; int fit = 1;
            p = cand[k];
            for (c = 0; c < 3u; ++c) { rlo[0][c]=rlo[1][c]=half_max-1; rhi[0][c]=rhi[1][c]=-half_max; }
            for (i = 0; i < 16u; ++i) { uint32_t reg=tc_bc6h_part2[p][i]; for(c=0;c<3u;++c){if(q[i][c]<rlo[reg][c])rlo[reg][c]=q[i][c];if(q[i][c]>rhi[reg][c])rhi[reg][c]=q[i][c];} }
            for (c = 0; c < 3u && fit; ++c) {
                int32_t limit = 1 << (dbits[c] - 1);
                int32_t d1 = rhi[0][c] - rlo[0][c], d2 = rlo[1][c] - rlo[0][c], d3 = rhi[1][c] - rlo[0][c];
                if (d1 < -limit || d1 > limit - 1 || d2 < -limit || d2 > limit - 1 || d3 < -limit || d3 > limit - 1) fit = 0;
            }
            if (!fit) continue;
            for(c=0;c<3u;++c){ep[0][c]=rlo[0][c];ep[1][c]=rhi[0][c];ep[2][c]=rlo[1][c];ep[3][c]=rhi[1][c];}
            err = tc_bc6h_mode0_selectors_sf16(target, tc_bc6h_part2[p], ep, sel);
            if (err < best_err) { best_err=err; best_p=(int)p; memcpy(bep,ep,sizeof(ep)); memcpy(bsel,sel,16u); }
        }
    }
    if (best_p < 0) return (uint64_t)-1;
    {
        static const uint32_t w3[8]={0,9,18,27,37,46,55,64}; int round,rr; int32_t half_max=128;
        for(round=0;round<2;++round){int32_t nep[4][3];uint8_t nsel[16];uint64_t e;int fit=1;
        for(rr=0;rr<2;++rr) for(c=0;c<3u;++c){int64_t saa=0,sab=0,sbb=0,sap=0,sbp=0,det,l,h;
        for(i=0;i<16u;++i){int64_t wv,a,pp;if((int)tc_bc6h_part2[best_p][i]!=rr)continue;wv=w3[bsel[i]];a=64-wv;pp=q[i][c];
        saa+=a*a;sab+=a*wv;sbb+=wv*wv;sap+=a*pp;sbp+=wv*pp;}
        det=saa*sbb-sab*sab;if(det<=0){nep[rr*2][c]=bep[rr*2][c];nep[rr*2+1][c]=bep[rr*2+1][c];continue;}
        l=tc_bc6h_rdiv((sap*sbb-sbp*sab)*64,det);h=tc_bc6h_rdiv((sbp*saa-sap*sab)*64,det);
        if(l<-half_max){l=-half_max;}if(l>half_max-1){l=half_max-1;}if(h<-half_max){h=-half_max;}if(h>half_max-1){h=half_max-1;}
        nep[rr*2][c]=(int32_t)l;nep[rr*2+1][c]=(int32_t)h;}
        for(c=0;c<3u&&fit;++c){int32_t limit=1<<(dbits[c]-1);int32_t d1=nep[1][c]-nep[0][c],d2=nep[2][c]-nep[0][c],d3=nep[3][c]-nep[0][c];
        if(d1<-limit||d1>limit-1||d2<-limit||d2>limit-1||d3<-limit||d3>limit-1)fit=0;}
        if(!fit){break;} e=tc_bc6h_mode0_selectors_sf16(target,tc_bc6h_part2[best_p],nep,nsel);
        if(e<best_err){best_err=e;memcpy(bep,nep,sizeof(nep));memcpy(bsel,nsel,16u);}else break;}
    }
    anchor = tc_bc6h_part2_anchor[best_p];
    for(r=0;r<2;++r){uint32_t at=(r==0)?0u:anchor;if(bsel[at]&4u){
      for(c=0;c<3u;++c){int32_t t=bep[r*2][c];bep[r*2][c]=bep[r*2+1][c];bep[r*2+1][c]=t;}
      for(i=0;i<16u;++i) if((int)tc_bc6h_part2[best_p][i]==r) bsel[i]=(uint8_t)(7u-bsel[i]);}}
    memset(out,0,16u);
    {
        uint32_t r0_lo = (uint32_t)bep[0][0] & 255u, r0_hi = ((uint32_t)bep[0][0] >> 8) & 3u;
        uint32_t g0_lo = (uint32_t)bep[0][1] & 255u, g0_hi = ((uint32_t)bep[0][1] >> 8) & 3u;
        uint32_t b0_lo = (uint32_t)bep[0][2] & 255u, b0_hi = ((uint32_t)bep[0][2] >> 8) & 3u;
        switch(mode_key){
          case 6: tc_bc6h_w(out,&bitpos,0x12u,5); break;
          case 7: tc_bc6h_w(out,&bitpos,0x16u,5); break;
          case 8: tc_bc6h_w(out,&bitpos,0x1au,5); break;
        }
        tc_bc6h_w(out,&bitpos,((r0_hi>>1)&1u),1);tc_bc6h_w(out,&bitpos,(r0_hi&1u),1);tc_bc6h_w(out,&bitpos,r0_lo,8);
        tc_bc6h_w(out,&bitpos,((g0_hi>>1)&1u),1);tc_bc6h_w(out,&bitpos,(g0_hi&1u),1);tc_bc6h_w(out,&bitpos,g0_lo,8);
        tc_bc6h_w(out,&bitpos,((b0_hi>>1)&1u),1);tc_bc6h_w(out,&bitpos,(b0_hi&1u),1);tc_bc6h_w(out,&bitpos,b0_lo,8);
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5((int32_t)bep[1][0]-(int32_t)bep[0][0]),dbits[0]);
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5((int32_t)bep[1][1]-(int32_t)bep[0][1]),dbits[1]);
        tc_bc6h_w(out,&bitpos,tc_bc6h_pack_delta5((int32_t)bep[1][2]-(int32_t)bep[0][2]),dbits[2]);
    }
    tc_bc6h_w(out,&bitpos,(uint32_t)best_p,5);
    for(i=0;i<16u;++i){uint32_t nb=(i==0u||i==anchor)?2u:3u; tc_bc6h_w(out,&bitpos,bsel[i],nb);}
    return best_err;
}

static void tc_encode_bc6h_block_uf16(const float pix[16][3], uint8_t out[16]) {
    uint32_t q[16][3], target[16][3], lo[3], hi[3], luma_lo[3], luma_hi[3];
    uint32_t i, c, bitpos = 0, min_l = UINT_MAX, max_l = 0, min_i = 0, max_i = 0;
    uint8_t sel[16], box_sel[16], luma_sel[16];
    memset(out, 0, 16);
    for (c = 0; c < 3u; ++c) {
        lo[c] = UINT_MAX;
        hi[c] = 0;
    }
    for (i = 0; i < 16u; ++i) {
        uint32_t l;
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_uf16(pix[i][c]);
            target[i][c] = tc_bc6h_unquant_uf16_to_mag(q[i][c]);
            if (q[i][c] < lo[c]) lo[c] = q[i][c];
            if (q[i][c] > hi[c]) hi[c] = q[i][c];
        }
        l = target[i][0] * 38u + target[i][1] * 76u + target[i][2] * 14u;
        if (l < min_l) {
            min_l = l;
            min_i = i;
        }
        if (l >= max_l) {
            max_l = l;
            max_i = i;
        }
    }
    for (c = 0; c < 3u; ++c) {
        luma_lo[c] = q[min_i][c];
        luma_hi[c] = q[max_i][c];
    }
    if (tc_bc6h_choose_selectors_uf16(target, luma_lo, luma_hi, luma_sel) <
        tc_bc6h_choose_selectors_uf16(target, lo, hi, box_sel)) {
        memcpy(lo, luma_lo, sizeof(lo));
        memcpy(hi, luma_hi, sizeof(hi));
        memcpy(sel, luma_sel, sizeof(sel));
    } else {
        memcpy(sel, box_sel, sizeof(sel));
    }
    {
        uint64_t err11 = tc_bc6h_refine_uf16(q, target, lo, hi, sel);
        uint64_t best_err = err11;
        uint8_t out9[16], out0[16];
        if (sel[0] & 8u) {
            uint32_t t;
            for (c = 0; c < 3u; ++c) {
                t = lo[c];
                lo[c] = hi[c];
                hi[c] = t;
            }
            for (i = 0; i < 16u; ++i) sel[i] = (uint8_t)(15u - sel[i]);
        }
        tc_set_bits(out, &bitpos, 0x03u, 5); /* mode 11: bit pattern 00011 */
        tc_set_bits(out, &bitpos, lo[0], 10);
        tc_set_bits(out, &bitpos, lo[1], 10);
        tc_set_bits(out, &bitpos, lo[2], 10);
        tc_set_bits(out, &bitpos, hi[0], 10);
        tc_set_bits(out, &bitpos, hi[1], 10);
        tc_set_bits(out, &bitpos, hi[2], 10);
        tc_set_bits(out, &bitpos, sel[0], 3);
        for (i = 1; i < 16u; ++i) tc_set_bits(out, &bitpos, sel[i], 4);
        /* Two-region mode 9 only helps when a single endpoint line leaves real
         * error, so skip its partition search on blocks mode 11 already fits
         * well (~per-channel RMS below ~256 of the 0..31744 magnitude range). */
        if (best_err > 48ull * 256 * 256) {
            uint64_t err9 = tc_bc6h_mode9_uf16(pix, out9);
            if (err9 < best_err) { best_err = err9; memcpy(out, out9, 16u); }
        }
        /* Mode 0 (10-bit + 5-bit signed deltas) — very restrictive (all four
         * endpoints within ±16), but when it fits it beats mode 9's 6-bit base
         * on near-uniform two-region blocks. Only try if still above threshold. */
        if (best_err > 48ull * 256 * 256) {
            uint64_t err0 = tc_bc6h_mode0_uf16(pix, out0);
            if (err0 < best_err) { best_err = err0; memcpy(out, out0, 16u); }
        }
        /* Mode 1 (7-bit + 6-bit signed deltas ±32) — wider range than mode 0
         * but coarser primary. Catches blocks where deltas barely overflow ±16. */
        if (best_err > 48ull * 256 * 256) {
            uint64_t err1 = tc_bc6h_mode1_uf16(pix, out0);
            if (err1 < best_err) { best_err = err1; memcpy(out, out0, 16u); }
        }
        /* Modes 2-4 (11-bit primary + 5/4 asymmetric deltas). Very tight range
         * (4-bit delta = ±8 in 2047 = 0.4%) but higher precision when it fits. */
        if (best_err > 48ull * 256 * 256) {
            static const uint8_t dbits2[3] = {5, 4, 4};
            static const uint8_t dbits3[3] = {4, 5, 4};
            static const uint8_t dbits4[3] = {4, 4, 5};
            uint64_t e2 = tc_bc6h_mode234_uf16(pix, dbits2, 2, out0);
            if (e2 < best_err) { best_err = e2; memcpy(out, out0, 16u); }
            if (best_err > 48ull * 256 * 256) {
                uint64_t e3 = tc_bc6h_mode234_uf16(pix, dbits3, 3, out0);
                if (e3 < best_err) { best_err = e3; memcpy(out, out0, 16u); }
            }
            if (best_err > 48ull * 256 * 256) {
                uint64_t e4 = tc_bc6h_mode234_uf16(pix, dbits4, 4, out0);
                if (e4 < best_err) { best_err = e4; memcpy(out, out0, 16u); }
            }
        }
        /* Mode 5 (9-bit + 5-bit deltas). Wider range than 10-bit ±16 in ~half
         * the primary range; catches blocks where primary fits 9-bit but mode 0's
         * 10-bit primary pushes a delta just over the edge. */
        if (best_err > 48ull * 256 * 256) {
            uint64_t e5 = tc_bc6h_mode5_uf16(pix, out0);
            if (e5 < best_err) { best_err = e5; memcpy(out, out0, 16u); }
        }
        /* One-region modes 10/12/13 (higher primary precision, 4-bit weights).
         * Mode 10 (10.10.10 direct) — same precision as mode 11 but without
         * delta encoding; mode 12 (12.8) higher primary but limited deltas;
         * mode 13 (16.4) max primary but very tight deltas. */
        if (best_err > 48ull * 256 * 256) {
            uint64_t e10 = tc_bc6h_mode10_uf16(pix, out0);
            if (e10 < best_err) { best_err = e10; memcpy(out, out0, 16u); }
        }
        if (best_err > 48ull * 256 * 256) {
            uint64_t e12 = tc_bc6h_mode12_uf16(pix, out0);
            if (e12 < best_err) { best_err = e12; memcpy(out, out0, 16u); }
        }
        if (best_err > 48ull * 256 * 256) {
            uint64_t e13 = tc_bc6h_mode13_uf16(pix, out0);
            if (e13 < best_err) { best_err = e13; memcpy(out, out0, 16u); }
        }
        /* Modes 6-8 (8-bit + asymmetric 6/5/5 deltas). Wider delta range (±32)
         * than ±16 for one channel, useful when one channel varies more. */
        if (best_err > 48ull * 256 * 256) {
            static const uint8_t d6[3]={6,5,5}, d7[3]={5,6,5}, d8[3]={5,5,6};
            uint64_t e6 = tc_bc6h_mode678_uf16(pix, d6, 6, out0);
            if (e6 < best_err) { best_err = e6; memcpy(out, out0, 16u); }
            if (best_err > 48ull * 256 * 256) {
                uint64_t e7 = tc_bc6h_mode678_uf16(pix, d7, 7, out0);
                if (e7 < best_err) { best_err = e7; memcpy(out, out0, 16u); }
            }
            if (best_err > 48ull * 256 * 256) {
                uint64_t e8 = tc_bc6h_mode678_uf16(pix, d8, 8, out0);
                if (e8 < best_err) { best_err = e8; memcpy(out, out0, 16u); }
            }
        }
    }
}

static void tc_encode_bc6h_block_sf16(const float pix[16][3], uint8_t out[16]) {
    int32_t q[16][3], target[16][3], lo[3], hi[3], luma_lo[3], luma_hi[3];
    uint32_t i, c, bitpos = 0, min_i = 0, max_i = 0;
    int32_t min_l = INT_MAX, max_l = INT_MIN;
    uint8_t sel[16], box_sel[16], luma_sel[16];
    uint64_t best_err;
    memset(out, 0, 16);
    for (c = 0; c < 3u; ++c) {
        lo[c] = INT_MAX;
        hi[c] = INT_MIN;
    }
    for (i = 0; i < 16u; ++i) {
        int32_t l;
        for (c = 0; c < 3u; ++c) {
            q[i][c] = tc_bc6h_quant_sf16(pix[i][c]);
            target[i][c] = tc_bc6h_unquant_sf16_to_smag(q[i][c]);
            if (q[i][c] < lo[c]) lo[c] = q[i][c];
            if (q[i][c] > hi[c]) hi[c] = q[i][c];
        }
        l = target[i][0] * 38 + target[i][1] * 76 + target[i][2] * 14;
        if (l < min_l) {
            min_l = l;
            min_i = i;
        }
        if (l >= max_l) {
            max_l = l;
            max_i = i;
        }
    }
    for (c = 0; c < 3u; ++c) {
        luma_lo[c] = q[min_i][c];
        luma_hi[c] = q[max_i][c];
    }
    if (tc_bc6h_choose_selectors_sf16(target, luma_lo, luma_hi, luma_sel) <
        tc_bc6h_choose_selectors_sf16(target, lo, hi, box_sel)) {
        memcpy(lo, luma_lo, sizeof(lo));
        memcpy(hi, luma_hi, sizeof(hi));
        memcpy(sel, luma_sel, sizeof(sel));
    } else {
        memcpy(sel, box_sel, sizeof(sel));
    }
    best_err = tc_bc6h_refine_sf16(q, target, lo, hi, sel);
    if (sel[0] & 8u) {
        int32_t t;
        for (c = 0; c < 3u; ++c) {
            t = lo[c];
            lo[c] = hi[c];
            hi[c] = t;
        }
        for (i = 0; i < 16u; ++i) sel[i] = (uint8_t)(15u - sel[i]);
    }
    /* Pack mode10 as initial output, then try alternative modes. */
    tc_set_bits(out, &bitpos, 0x03u, 5);
    tc_set_bits(out, &bitpos, tc_bc6h_pack_signed10(lo[0]), 10);
    tc_set_bits(out, &bitpos, tc_bc6h_pack_signed10(lo[1]), 10);
    tc_set_bits(out, &bitpos, tc_bc6h_pack_signed10(lo[2]), 10);
    tc_set_bits(out, &bitpos, tc_bc6h_pack_signed10(hi[0]), 10);
    tc_set_bits(out, &bitpos, tc_bc6h_pack_signed10(hi[1]), 10);
    tc_set_bits(out, &bitpos, tc_bc6h_pack_signed10(hi[2]), 10);
    tc_set_bits(out, &bitpos, sel[0], 3);
    for (i = 1; i < 16u; ++i) tc_set_bits(out, &bitpos, sel[i], 4);
    {
        uint8_t tmp[16];
        uint64_t e;
        /* One-region split modes (12/13) have 4-bit indices, same as mode10,
         * and higher endpoint precision. Try them unconditionally — they
         * either improve quality or fail their tight delta check and return -1. */
        e = tc_bc6h_mode12_sf16(pix, tmp);
        if (e < best_err) { best_err = e; memcpy(out, tmp, 16u); }
        e = tc_bc6h_mode13_sf16(pix, tmp);
        if (e < best_err) { best_err = e; memcpy(out, tmp, 16u); }
    }
    if (best_err > 48ull * 256 * 256) {
        uint8_t tmp[16];
        uint64_t e;
        e = tc_bc6h_mode9_sf16(pix, tmp);
        if (e < best_err) { best_err = e; memcpy(out, tmp, 16u); }
        e = tc_bc6h_mode0_sf16(pix, tmp);
        if (e < best_err) { best_err = e; memcpy(out, tmp, 16u); }
        e = tc_bc6h_mode1_sf16(pix, tmp);
        if (e < best_err) { best_err = e; memcpy(out, tmp, 16u); }
        e = tc_bc6h_mode5_sf16(pix, tmp);
        if (e < best_err) { best_err = e; memcpy(out, tmp, 16u); }
        { static const uint8_t d234[3][3] = {{5,4,4},{4,5,4},{4,4,5}};
          int m; for(m=0;m<3;++m){ e = tc_bc6h_mode234_sf16(pix,d234[m],2+m,tmp); if(e<best_err){best_err=e;memcpy(out,tmp,16u);} } }
        { static const uint8_t d678[3][3] = {{6,5,5},{5,6,5},{5,5,6}};
          int m; for(m=0;m<3;++m){ e = tc_bc6h_mode678_sf16(pix,d678[m],6+m,tmp); if(e<best_err){best_err=e;memcpy(out,tmp,16u);} } }
    }
}

tc_result tc_bc6h_compress_rgb32f(const float *rgb, uint32_t width,
                                  uint32_t height, size_t stride_bytes,
                                  const tc_bc6h_options *opt,
                                  uint8_t *out_bc6h, size_t out_size) {
    uint32_t bx, by, x, y, xx, yy;
    float block[16][3];
    size_t need, off = 0;
    (void)opt;

    if (!rgb || !out_bc6h || !width || !height) return TC_ERROR_INVALID_ARGUMENT;
    if (stride_bytes < (size_t)width * 3u * sizeof(float))
        return TC_ERROR_INVALID_ARGUMENT;
    need = tc_bc6h_compressed_size(width, height);
    if (!need || out_size < need) return TC_ERROR_INVALID_ARGUMENT;

    for (by = 0; by < height; by += 4) {
        for (bx = 0; bx < width; bx += 4) {
            for (yy = 0; yy < 4; ++yy) {
                y = by + yy;
                if (y >= height) y = height - 1u;
                for (xx = 0; xx < 4; ++xx) {
                    const float *src;
                    x = bx + xx;
                    if (x >= width) x = width - 1u;
                    src = (const float *)((const uint8_t *)rgb +
                                          (size_t)y * stride_bytes) +
                          (size_t)x * 3u;
                    block[yy * 4u + xx][0] = src[0];
                    block[yy * 4u + xx][1] = src[1];
                    block[yy * 4u + xx][2] = src[2];
                }
            }
            if (opt && opt->signed_float)
                tc_encode_bc6h_block_sf16(block, out_bc6h + off);
            else
                tc_encode_bc6h_block_uf16(block, out_bc6h + off);
            off += 16u;
        }
    }

    return TC_SUCCESS;
}
