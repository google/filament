/*
 * TinyEXR - half <-> float conversion (scalar kernels + public dispatch).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

static float half_bits_to_float(uint16_t h) {
    uint32_t sign = (uint32_t)(h & 0x8000u) << 16;
    uint32_t exp = (h >> 10) & 0x1fu;
    uint32_t mant = h & 0x3ffu;
    uint32_t f;
    float out;
    if (exp == 0) {
        if (mant == 0) {
            f = sign; /* +/- zero */
        } else {
            int e = -1;
            do {
                mant <<= 1;
                ++e;
            } while ((mant & 0x400u) == 0);
            mant &= 0x3ffu;
            f = sign | ((uint32_t)(127 - 15 - e) << 23) | (mant << 13);
        }
    } else if (exp == 31) {
        f = sign | 0x7f800000u | (mant << 13);
    } else {
        f = sign | ((exp - 15 + 127) << 23) | (mant << 13);
    }
    memcpy(&out, &f, sizeof(out));
    return out;
}

static uint16_t float_to_half_bits(float fv) {
    uint32_t x;
    uint32_t sign, mant;
    int32_t exp;
    memcpy(&x, &fv, sizeof(x));
    sign = (x >> 16) & 0x8000u;
    exp = (int32_t)((x >> 23) & 0xffu);
    mant = x & 0x7fffffu;

    if (exp == 0xff) /* inf / nan */
        return (uint16_t)(sign | 0x7c00u | (mant ? (0x200u | (mant >> 13)) : 0u));

    exp = exp - 127 + 15;
    if (exp >= 31) return (uint16_t)(sign | 0x7c00u); /* overflow -> inf */

    if (exp <= 0) {
        uint32_t r, rem, halfbit;
        int shift;
        if (exp < -10) return (uint16_t)sign; /* underflow -> 0 */
        mant |= 0x800000u;
        shift = 14 - exp;
        r = mant >> shift;
        rem = mant & ((1u << shift) - 1u);
        halfbit = 1u << (shift - 1);
        if (rem > halfbit || (rem == halfbit && (r & 1u))) ++r;
        return (uint16_t)(sign | r);
    } else {
        uint32_t r = mant >> 13;
        uint32_t rem = mant & 0x1fffu;
        if (rem > 0x1000u || (rem == 0x1000u && (r & 1u))) {
            ++r;
            if (r == 0x400u) {
                r = 0;
                ++exp;
                if (exp >= 31) return (uint16_t)(sign | 0x7c00u);
            }
        }
        return (uint16_t)(sign | ((uint32_t)exp << 10) | r);
    }
}

void exr_half_to_float_scalar(const uint16_t *src, float *dst, size_t count) {
    size_t i;
    for (i = 0; i < count; ++i) dst[i] = half_bits_to_float(src[i]);
}

void exr_float_to_half_scalar(const float *src, uint16_t *dst, size_t count) {
    size_t i;
    for (i = 0; i < count; ++i) dst[i] = float_to_half_bits(src[i]);
}

void exr_interleave_scalar(const uint8_t *src, uint8_t *dst, size_t n) {
    const uint8_t *t1 = src;
    const uint8_t *t2 = src + (n + 1) / 2;
    uint8_t *s = dst;
    uint8_t *stop = dst + n;
    while (s < stop) {
        *s++ = *t1++;
        if (s < stop) *s++ = *t2++;
    }
}

/* Public dispatch entry points. */
void exr_half_to_float(const uint16_t *src, float *dst, size_t count) {
    exr_simd_init();
    exr_simd.half_to_float(src, dst, count);
}
void exr_float_to_half(const float *src, uint16_t *dst, size_t count) {
    exr_simd_init();
    exr_simd.float_to_half(src, dst, count);
}
