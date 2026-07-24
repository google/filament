/*
 * tir - scalar pixel type converters (f16 / u8 / u16 unorm <-> f32).
 *
 * The float16 conversions are branchy bit-exact IEEE 754 binary16
 * round-to-nearest-even; SIMD ISAs with hardware converters (F16C, NEON
 * fp16) produce identical results.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tir_internal.h"

size_t tir__pixel_size(tir_pixel_type t) {
    switch (t) {
        case TIR_F32:
            return 4;
        case TIR_F16:
        case TIR_U16:
            return 2;
        default:
            return 1; /* TIR_U8 */
    }
}

/* ---- unorm ----------------------------------------------------------------- */

void tir__u8_to_f32_sc(float *dst, const uint8_t *src, size_t n) {
    size_t i;
    const float k = 1.0f / 255.0f;
    for (i = 0; i < n; ++i) dst[i] = (float)src[i] * k;
}

void tir__u16_to_f32_sc(float *dst, const uint16_t *src, size_t n) {
    size_t i;
    const float k = 1.0f / 65535.0f;
    for (i = 0; i < n; ++i) dst[i] = (float)src[i] * k;
}

/* Round-to-nearest-even to match SIMD cvtps behavior. */
static float rne(float x) { return nearbyintf(x); }

void tir__f32_to_u8_sc(uint8_t *dst, const float *src, size_t n) {
    size_t i;
    for (i = 0; i < n; ++i) {
        float v = src[i];
        if (!(v > 0.0f)) v = 0.0f; /* also catches NaN */
        if (v > 1.0f) v = 1.0f;
        dst[i] = (uint8_t)rne(v * 255.0f);
    }
}

void tir__f32_to_u16_sc(uint16_t *dst, const float *src, size_t n) {
    size_t i;
    for (i = 0; i < n; ++i) {
        float v = src[i];
        if (!(v > 0.0f)) v = 0.0f;
        if (v > 1.0f) v = 1.0f;
        dst[i] = (uint16_t)rne(v * 65535.0f);
    }
}

/* ---- IEEE binary16 ---------------------------------------------------------- */

void tir__f16_to_f32_sc(float *dst, const uint16_t *src, size_t n) {
    size_t i;
    for (i = 0; i < n; ++i) {
        uint32_t h = src[i];
        uint32_t sign = (h & 0x8000u) << 16;
        uint32_t exp = (h >> 10) & 0x1Fu;
        uint32_t man = h & 0x3FFu;
        uint32_t bits;
        if (exp == 0) {
            if (man == 0) {
                bits = sign; /* +-0 */
            } else { /* subnormal: normalize */
                int e = -1;
                do {
                    man <<= 1;
                    e++;
                } while (!(man & 0x400u));
                man &= 0x3FFu;
                bits = sign | (uint32_t)(127 - 15 - e) << 23 | (man << 13);
            }
        } else if (exp == 31) {
            bits = sign | 0x7F800000u | (man << 13); /* inf / nan */
        } else {
            bits = sign | ((exp + (127 - 15)) << 23) | (man << 13);
        }
        memcpy(&dst[i], &bits, 4);
    }
}

void tir__f32_to_f16_sc(uint16_t *dst, const float *src, size_t n) {
    size_t i;
    for (i = 0; i < n; ++i) {
        uint32_t bits;
        uint32_t sign, exp, man;
        uint16_t h;
        memcpy(&bits, &src[i], 4);
        sign = (bits >> 16) & 0x8000u;
        exp = (bits >> 23) & 0xFFu;
        man = bits & 0x7FFFFFu;
        if (exp == 255) { /* inf / nan */
            h = (uint16_t)(sign | 0x7C00u | (man ? 0x200u | (man >> 13) : 0u));
        } else {
            int e = (int)exp - 127 + 15;
            if (e >= 31) { /* overflow -> inf */
                h = (uint16_t)(sign | 0x7C00u);
            } else if (e <= 0) {
                if (e < -10) { /* underflow -> +-0 */
                    h = (uint16_t)sign;
                } else { /* subnormal with RNE */
                    uint32_t m = man | 0x800000u;
                    uint32_t shift = (uint32_t)(14 - e);
                    uint32_t hm = m >> shift;
                    uint32_t rem = m & ((1u << shift) - 1u);
                    uint32_t half = 1u << (shift - 1);
                    if (rem > half || (rem == half && (hm & 1u))) hm++;
                    h = (uint16_t)(sign | hm);
                }
            } else { /* normal with RNE */
                uint32_t hm = man >> 13;
                uint32_t rem = man & 0x1FFFu;
                if (rem > 0x1000u || (rem == 0x1000u && (hm & 1u))) {
                    hm++;
                    if (hm == 0x400u) {
                        hm = 0;
                        e++;
                        if (e >= 31) {
                            h = (uint16_t)(sign | 0x7C00u);
                            dst[i] = h;
                            continue;
                        }
                    }
                }
                h = (uint16_t)(sign | ((uint32_t)e << 10) | hm);
            }
        }
        dst[i] = h;
    }
}
