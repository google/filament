/*
 * TinyEXR texcomp - ASTC HDR (UASTC HDR 4x4) encoder.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 *
 * The CEM 7 (base+scale) and CEM 11 (RGB direct) HDR endpoint pack/unpack are
 * ports of Arm astcenc's quantize_hdr_rgbo / hdr_rgbo_unpack / hdr_rgb_unpack
 * (Apache-2.0). See tools/texcomp/NOTICE.md.
 *
 * Emits 100%% standard ASTC HDR 4x4 blocks. The current implementation encodes
 * constant-colour (void-extent) FP16 blocks -- the block-average of each 4x4
 * tile. Per-texel HDR endpoint modes (ASTC CEM 7 base+scale, CEM 11 RGB
 * direct, with qlog endpoints) are being ported incrementally on top of this
 * pipeline. Output validates against a conformant ASTC HDR decoder
 * (deps/astcenc) via `make texcomp-astc-hdr-gate`.
 */
#include "texcomp.h"
#include "texcomp_internal.h"

#include <string.h>

/* ---- LNS (logarithmic) HDR value domain ---------------------------------
 * ASTC HDR interpolates endpoints in a 16-bit "LNS" (log) domain and converts
 * the interpolated value to FP16 with lns_to_sf16. These are scalar ports of
 * astcenc's float_to_lns / lns_to_sf16 (astcenc_vecmathlib.h), the reference
 * decoder we validate against. */
static uint32_t tc_f2u(float f) {
    union {
        float f;
        uint32_t u;
    } v;
    v.f = f;
    return v.u;
}

static float tc_u2f(uint32_t u) {
    union {
        float f;
        uint32_t u;
    } v;
    v.u = u;
    return v.f;
}

/* astcenc-compatible frexp: mantissa in [0.5,1), exponent via the raw bits. */
static float tc_astc_frexp(float a, int *expo) {
    uint32_t ai = tc_f2u(a);
    *expo = (int)((ai >> 23) & 0xFFu) - 126;
    return tc_u2f((ai & 0x807FFFFFu) | 0x3F000000u);
}

/* float -> 16-bit LNS value (rounded, clamped to [0, 65535]). */
int tc_astc_float_to_lns16(float a) {
    int expo, ri, expv;
    float mant, av, a2, r;
    int underflow = !(a > (1.0f / 67108864.0f)); /* a <= 2^-26 (incl. neg/NaN) */
    int infinity = a >= 65536.0f;
    mant = tc_astc_frexp(a, &expo);
    if (expo < -13) {
        av = a * 33554432.0f; /* 2^25 */
        expv = 0;
    } else {
        av = (mant - 0.5f) * 4096.0f;
        expv = expo + 14;
    }
    if (av < 384.0f)
        a2 = av * (4.0f / 3.0f);
    else if (av <= 1408.0f)
        a2 = av + 128.0f;
    else
        a2 = (av + 512.0f) * (4.0f / 5.0f);
    r = a2 + (float)expv * 2048.0f + 1.0f;
    if (infinity) r = 65535.0f;
    if (underflow) r = 0.0f;
    ri = (int)(r + 0.5f);
    if (ri < 0) ri = 0;
    if (ri > 65535) ri = 65535;
    return ri;
}

/* 16-bit LNS value -> FP16 bits. */
uint16_t tc_astc_lns16_to_sf16(int p) {
    int mc = p & 0x7FF;
    int ec = (int)((unsigned)p >> 11);
    int mt, res;
    if (mc < 512)
        mt = mc * 3;
    else if (mc < 1536)
        mt = mc * 4 - 512;
    else
        mt = mc * 5 - 2048;
    res = (ec << 10) | (mt >> 3);
    if (res > 0x7BFF) res = 0x7BFF;
    return (uint16_t)res;
}

/* ---- CEM 11 (HDR RGB direct) endpoint codec -----------------------------
 * Endpoints are 16-bit LNS values per channel. This is a scalar port of
 * astcenc's quantize_hdr_rgb / hdr_rgb_unpack (colour quant fixed at 256, so
 * astcenc's quant/retain-top-bits helpers are identity and drop out). It tries
 * the 8 base+offset modes (high precision when the two endpoints are close and
 * channels correlated), falling back to the majcomp==3 direct sub-mode (R/G
 * 8-bit, B 7-bit) when no base+offset mode fits. */
static int tc_rtn(float x) { return (int)(x >= 0.0f ? x + 0.5f : x - 0.5f); }
static float tc_fclamp(float x, float lo, float hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}
static float tc_fabs(float x) { return x < 0.0f ? -x : x; }

/* quant_color / retain-top-bits at colour quant `level` (identity at 256). */
static int tc_cem11_qc(int level, int value) {
    return tc_astc_hdr_color_roundtrip((uint32_t)level, value);
}
static int tc_cem11_retain(int level, int value, int topmask) {
    for (;;) {
        int q = tc_cem11_qc(level, value);
        if ((value & topmask) == (q & topmask) || value <= 0) return q;
        value--;
    }
}

void tc_astc_cem11_pack(const int lns0[3], const int lns1[3], int level,
                        uint8_t v[6]) {
    static const int mode_bits[8][4] = {{9, 7, 6, 7},  {9, 8, 6, 6},
                                        {10, 6, 7, 7}, {10, 7, 7, 6},
                                        {11, 8, 6, 5}, {11, 6, 8, 6},
                                        {12, 7, 7, 5}, {12, 6, 7, 6}};
    static const float mode_cut[8][4] = {
        {16384, 8192, 8192, 8}, {32768, 8192, 4096, 8}, {4096, 8192, 4096, 4},
        {8192, 8192, 2048, 4},  {8192, 2048, 512, 2},   {2048, 8192, 1024, 2},
        {2048, 2048, 256, 1},   {1024, 2048, 512, 1}};
    static const float mode_scale[8] = {1.0f / 128, 1.0f / 128, 1.0f / 64,
                                        1.0f / 64,  1.0f / 32,  1.0f / 32,
                                        1.0f / 16,  1.0f / 16};
    static const float mode_rscale[8] = {128, 128, 64, 64, 32, 32, 16, 16};
    float c0[3], c1[3];
    float a_base, b0b, b1b, cb, d0b, d1b;
    int majcomp, mode, ci;

    for (ci = 0; ci < 3; ++ci) {
        c0[ci] = tc_fclamp((float)lns0[ci], 0.0f, 65535.0f);
        c1[ci] = tc_fclamp((float)lns1[ci], 0.0f, 65535.0f);
    }
    if (c1[0] > c1[1] && c1[0] > c1[2])
        majcomp = 0;
    else if (c1[1] > c1[2])
        majcomp = 1;
    else
        majcomp = 2;
    if (majcomp == 1) {
        float t0 = c0[0], t1 = c1[0];
        c0[0] = c0[1];
        c1[0] = c1[1];
        c0[1] = t0;
        c1[1] = t1;
    } else if (majcomp == 2) {
        float t0 = c0[0], t1 = c1[0];
        c0[0] = c0[2];
        c1[0] = c1[2];
        c0[2] = t0;
        c1[2] = t1;
    }
    a_base = c1[0];
    b0b = a_base - c1[1];
    b1b = a_base - c1[2];
    cb = a_base - c0[0];
    d0b = a_base - b0b - cb - c0[1];
    d1b = a_base - b1b - cb - c0[2];

    for (mode = 7; mode >= 0; --mode) {
        float ms = mode_scale[mode], mr = mode_rscale[mode];
        int b_ic = 1 << mode_bits[mode][1];
        int c_ic = 1 << mode_bits[mode][2];
        int d_ic = 1 << (mode_bits[mode][3] - 1);
        int a_iv, c_iv, b0_iv, b1_iv, d0_iv, d1_iv;
        int c_lo, b0_lo, b1_lo, d0_lo, d1_lo, bit0, bit1, bit2, bit3, bit4, bit5;
        float a_f, c_f, b0_f, b1_f;
        if (b0b > mode_cut[mode][0] || b1b > mode_cut[mode][0] ||
            cb > mode_cut[mode][1] || tc_fabs(d0b) > mode_cut[mode][2] ||
            tc_fabs(d1b) > mode_cut[mode][2])
            continue;

        a_iv = tc_rtn(a_base * ms);
        a_iv = (a_iv & ~0xFF) | tc_cem11_qc(level, a_iv & 0xFF);
        a_f = (float)a_iv * mr;

        c_f = tc_fclamp(a_f - c0[0], 0.0f, 65535.0f);
        c_iv = tc_rtn(c_f * ms);
        if (c_iv >= c_ic) continue;
        c_lo = (c_iv & 0x3f) | ((mode & 1) << 7) | ((a_iv & 0x100) >> 2);
        c_lo = tc_cem11_retain(level, c_lo, 0xC0);
        c_iv = (c_iv & ~0x3F) | (c_lo & 0x3F);
        c_f = (float)c_iv * mr;

        b0_f = tc_fclamp(a_f - c1[1], 0.0f, 65535.0f);
        b1_f = tc_fclamp(a_f - c1[2], 0.0f, 65535.0f);
        b0_iv = tc_rtn(b0_f * ms);
        b1_iv = tc_rtn(b1_f * ms);
        if (b0_iv >= b_ic || b1_iv >= b_ic) continue;
        b0_lo = b0_iv & 0x3f;
        b1_lo = b1_iv & 0x3f;
        bit0 = (mode == 2 || mode == 5 || mode == 7) ? ((a_iv >> 9) & 1)
                                                     : ((b0_iv >> 6) & 1);
        if (mode == 2)
            bit1 = (c_iv >> 6) & 1;
        else if (mode == 5 || mode == 7)
            bit1 = (a_iv >> 10) & 1;
        else
            bit1 = (b1_iv >> 6) & 1;
        b0_lo |= (bit0 << 6) | (((mode >> 1) & 1) << 7);
        b1_lo |= (bit1 << 6) | (((mode >> 2) & 1) << 7);
        b0_lo = tc_cem11_retain(level, b0_lo, 0xC0);
        b1_lo = tc_cem11_retain(level, b1_lo, 0xC0);
        b0_iv = (b0_iv & ~0x3f) | (b0_lo & 0x3f);
        b1_iv = (b1_iv & ~0x3f) | (b1_lo & 0x3f);
        b0_f = (float)b0_iv * mr;
        b1_f = (float)b1_iv * mr;

        {
            float d0_f =
                tc_fclamp(a_f - b0_f - c_f - c0[1], -65535.0f, 65535.0f);
            float d1_f =
                tc_fclamp(a_f - b1_f - c_f - c0[2], -65535.0f, 65535.0f);
            d0_iv = tc_rtn(d0_f * ms);
            d1_iv = tc_rtn(d1_f * ms);
        }
        if ((d0_iv < 0 ? -d0_iv : d0_iv) >= d_ic ||
            (d1_iv < 0 ? -d1_iv : d1_iv) >= d_ic)
            continue;

        d0_lo = d0_iv & 0x1f;
        d1_lo = d1_iv & 0x1f;
        if (mode == 0 || mode == 2)
            bit2 = (d0_iv >> 6) & 1;
        else if (mode == 1 || mode == 4)
            bit2 = (b0_iv >> 7) & 1;
        else if (mode == 3)
            bit2 = (a_iv >> 9) & 1;
        else if (mode == 5)
            bit2 = (c_iv >> 7) & 1;
        else
            bit2 = (a_iv >> 11) & 1; /* mode 6,7 */
        if (mode == 0 || mode == 2)
            bit3 = (d1_iv >> 6) & 1;
        else if (mode == 1 || mode == 4)
            bit3 = (b1_iv >> 7) & 1;
        else
            bit3 = (c_iv >> 6) & 1; /* mode 3,5,6,7 */
        if (mode == 4 || mode == 6) {
            bit4 = (a_iv >> 9) & 1;
            bit5 = (a_iv >> 10) & 1;
        } else {
            bit4 = (d0_iv >> 5) & 1;
            bit5 = (d1_iv >> 5) & 1;
        }
        d0_lo |= (bit2 << 6) | (bit4 << 5) | ((majcomp & 1) << 7);
        d1_lo |= (bit3 << 6) | (bit5 << 5) | (((majcomp >> 1) & 1) << 7);
        d0_lo = tc_cem11_retain(level, d0_lo, 0xF0);
        d1_lo = tc_cem11_retain(level, d1_lo, 0xF0);

        v[0] = (uint8_t)(a_iv & 0xFF);
        v[1] = (uint8_t)c_lo;
        v[2] = (uint8_t)b0_lo;
        v[3] = (uint8_t)b1_lo;
        v[4] = (uint8_t)d0_lo;
        v[5] = (uint8_t)d1_lo;
        return;
    }

    /* Flat fallback == majcomp==3 direct (R/G 8-bit, B 7-bit qlog). */
    {
        int r0 = tc_cem11_qc(level, tc_rtn(tc_fclamp((float)lns0[0], 0, 65020) / 256.0f));
        int r1 = tc_cem11_qc(level, tc_rtn(tc_fclamp((float)lns1[0], 0, 65020) / 256.0f));
        int g0 = tc_cem11_qc(level, tc_rtn(tc_fclamp((float)lns0[1], 0, 65020) / 256.0f));
        int g1 = tc_cem11_qc(level, tc_rtn(tc_fclamp((float)lns1[1], 0, 65020) / 256.0f));
        int b0 = tc_cem11_retain(level, tc_rtn(tc_fclamp((float)lns0[2], 0, 65020) / 512.0f) + 128, 0xC0);
        int b1 = tc_cem11_retain(level, tc_rtn(tc_fclamp((float)lns1[2], 0, 65020) / 512.0f) + 128, 0xC0);
        v[0] = (uint8_t)r0;
        v[1] = (uint8_t)r1;
        v[2] = (uint8_t)g0;
        v[3] = (uint8_t)g1;
        v[4] = (uint8_t)b0; /* bit7 set by +128 -> majcomp low  */
        v[5] = (uint8_t)b1; /* bit7 set by +128 -> majcomp high  */
    }
}

/* Decode CEM 11 endpoints to 16-bit LNS per channel (full ASTC hdr_rgb). */
static int tc_sclamp(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
int tc_astc_cem11_unpack(const uint8_t vv[6], int out0[3], int out1[3]) {
    int v0 = vv[0], v1 = vv[1], v2 = vv[2], v3 = vv[3], v4 = vv[4], v5 = vv[5];
    int modeval = ((v1 & 0x80) >> 7) | (((v2 & 0x80) >> 7) << 1) |
                  (((v3 & 0x80) >> 7) << 2);
    int majcomp = ((v4 & 0x80) >> 7) | (((v5 & 0x80) >> 7) << 1);
    int a, b0, b1, c, d0, d1, dbits, ohmod, valsh, r0, g0, bl0, r1, g1, bl1, t;
    static const int dbits_tab[8] = {7, 6, 7, 6, 5, 6, 5, 6};

    if (majcomp == 3) {
        out0[0] = v0 << 8;
        out0[1] = v2 << 8;
        out0[2] = (v4 & 0x7F) << 9;
        out1[0] = v1 << 8;
        out1[1] = v3 << 8;
        out1[2] = (v5 & 0x7F) << 9;
        return 1;
    }

    a = v0 | ((v1 & 0x40) << 2);
    b0 = v2 & 0x3f;
    b1 = v3 & 0x3f;
    c = v1 & 0x3f;
    d0 = v4 & 0x7f;
    d1 = v5 & 0x7f;
    dbits = dbits_tab[modeval];
    {
        int bit0 = (v2 >> 6) & 1, bit1 = (v3 >> 6) & 1, bit2 = (v4 >> 6) & 1;
        int bit3 = (v5 >> 6) & 1, bit4 = (v4 >> 5) & 1, bit5 = (v5 >> 5) & 1;
        ohmod = 1 << modeval;
        if (ohmod & 0xA4) a |= bit0 << 9;
        if (ohmod & 0x8) a |= bit2 << 9;
        if (ohmod & 0x50) a |= bit4 << 9;
        if (ohmod & 0x50) a |= bit5 << 10;
        if (ohmod & 0xA0) a |= bit1 << 10;
        if (ohmod & 0xC0) a |= bit2 << 11;
        if (ohmod & 0x4) c |= bit1 << 6;
        if (ohmod & 0xE8) c |= bit3 << 6;
        if (ohmod & 0x20) c |= bit2 << 7;
        if (ohmod & 0x5B) {
            b0 |= bit0 << 6;
            b1 |= bit1 << 6;
        }
        if (ohmod & 0x12) {
            b0 |= bit2 << 7;
            b1 |= bit3 << 7;
        }
        if (ohmod & 0xAF) {
            d0 |= bit4 << 5;
            d1 |= bit5 << 5;
        }
        if (ohmod & 0x5) {
            d0 |= bit2 << 6;
            d1 |= bit3 << 6;
        }
    }
    /* sign-extend d0/d1 from dbits, then scale all to the 12-bit domain. */
    {
        int sh = 32 - dbits, sc;
        d0 = (int)((uint32_t)d0 << sh) >> sh;
        d1 = (int)((uint32_t)d1 << sh) >> sh;
        valsh = (modeval >> 1) ^ 3;
        sc = 1 << valsh;
        a *= sc;
        b0 *= sc;
        b1 *= sc;
        c *= sc;
        d0 *= sc;
        d1 *= sc;
    }

    r1 = a;
    g1 = a - b0;
    bl1 = a - b1;
    r0 = a - c;
    g0 = a - b0 - c - d0;
    bl0 = a - b1 - c - d1;
    r0 = tc_sclamp(r0, 0, 4095);
    g0 = tc_sclamp(g0, 0, 4095);
    bl0 = tc_sclamp(bl0, 0, 4095);
    r1 = tc_sclamp(r1, 0, 4095);
    g1 = tc_sclamp(g1, 0, 4095);
    bl1 = tc_sclamp(bl1, 0, 4095);
    if (majcomp == 1) {
        t = r0;
        r0 = g0;
        g0 = t;
        t = r1;
        r1 = g1;
        g1 = t;
    } else if (majcomp == 2) {
        t = r0;
        r0 = bl0;
        bl0 = t;
        t = r1;
        r1 = bl1;
        bl1 = t;
    }
    out0[0] = r0 << 4;
    out0[1] = g0 << 4;
    out0[2] = bl0 << 4;
    out1[0] = r1 << 4;
    out1[1] = g1 << 4;
    out1[2] = bl1 << 4;
    return 1;
}

/* HDR alpha endpoint codec (the extra 2 values of CEM 14/15), ports of astcenc
 * quantize_hdr_alpha / hdr_alpha_unpack. Colour quant is fixed at 256 in the
 * texcomp HDR path (level 20), where quant_color is the identity -- so the
 * pack's quantize+reconstruct-check steps drop out and the delta submode
 * succeeds whenever its range check passes, matching astcenc at that level.
 * Output is 16-bit LNS (12-bit qlog << 4), the same scale as the CEM 11 unpack,
 * so tc_astc_lns16_to_sf16 turns it into fp16. */
void tc_astc_hdr_alpha_pack(int alns0, int alns1, uint8_t out[2]) {
    int ia0, ia1, val0, val1, diffval, v6, v7, i, cutoff, mask;
    /* alns are LNS16-domain alpha endpoints (same domain as the RGB endpoints,
     * i.e. tc_astc_float_to_lns16 output), clamped to the encodable range. */
    ia0 = tc_rtn(tc_fclamp((float)alns0, 0.0f, 65280.0f));
    ia1 = tc_rtn(tc_fclamp((float)alns1, 0.0f, 65280.0f));
    /* delta submodes, decreasing precision */
    for (i = 2; i >= 0; --i) {
        val0 = (ia0 + (128 >> i)) >> (8 - i);
        val1 = (ia1 + (128 >> i)) >> (8 - i);
        v6 = (val0 & 0x7F) | ((i & 1) << 7);
        /* quant_color identity at 256: v6d == v6, so val0 is unchanged. */
        diffval = val1 - val0;
        cutoff = 32 >> i;
        mask = 2 * cutoff - 1;
        if (diffval < -cutoff || diffval >= cutoff) continue;
        v7 = ((i & 2) << 6) | ((val0 >> 7) << (6 - i)) | (diffval & mask);
        out[0] = (uint8_t)v6;
        out[1] = (uint8_t)v7;
        return;
    }
    /* flat fallback */
    val0 = (ia0 + 256) >> 9;
    val1 = (ia1 + 256) >> 9;
    out[0] = (uint8_t)(val0 | 0x80);
    out[1] = (uint8_t)(val1 | 0x80);
}

void tc_astc_hdr_alpha_unpack(const uint8_t in[2], int *out0, int *out1) {
    int v6 = in[0], v7 = in[1];
    int modeval = ((v6 >> 7) & 1) | ((v7 >> 6) & 2);
    v6 &= 0x7F;
    v7 &= 0x7F;
    if (modeval == 3) {
        *out0 = (v6 << 5) << 4; /* 12-bit qlog << 4 -> 16-bit LNS */
        *out1 = (v7 << 5) << 4;
    } else {
        v6 |= (v7 << (modeval + 1)) & 0x780;
        v7 &= (0x3F >> modeval);
        v7 ^= 32 >> modeval;
        v7 -= 32 >> modeval;
        v6 = v6 << (4 - modeval);
        /* signed left shift via unsigned to stay UB-free (v7 may be negative) */
        v7 = (int)((uint32_t)v7 << (4 - modeval));
        v7 = tc_sclamp(v6 + v7, 0, 0xFFF);
        *out0 = v6 << 4;
        *out1 = v7 << 4;
    }
}

/* CEM 15 = HDR RGB direct (6 values, CEM 11) + HDR alpha (2 values). Endpoints
 * are LNS; pack/unpack are the CEM 11 RGB codec plus the HDR alpha codec above.
 * lns0/lns1[3] are RGB LNS16 endpoints, a0/a1 the (float) alpha endpoints. */
void tc_astc_cem15_pack(const int lns0[3], const int lns1[3], int alns0,
                        int alns1, int level, uint8_t out[8]) {
    tc_astc_cem11_pack(lns0, lns1, level, out);
    tc_astc_hdr_alpha_pack(alns0, alns1, out + 6);
}

int tc_astc_cem15_unpack(const uint8_t v[8], int out0[4], int out1[4]) {
    int rgb0[3], rgb1[3];
    if (!tc_astc_cem11_unpack(v, rgb0, rgb1)) return 0;
    out0[0] = rgb0[0]; out0[1] = rgb0[1]; out0[2] = rgb0[2];
    out1[0] = rgb1[0]; out1[1] = rgb1[1]; out1[2] = rgb1[2];
    tc_astc_hdr_alpha_unpack(v + 6, &out0[3], &out1[3]);
    return 1;
}

/* CEM 7 (HDR RGB base+scale, "RGBO"): decode 4 packed values to two LNS
 * endpoints. Endpoint1 = base RGB, endpoint0 = base - scale (uniform). Mirrors
 * astcenc hdr_rgbo_unpack; output is 16-bit LNS (value << 4), same scale as
 * tc_astc_cem11_unpack. */
int tc_astc_cem7_unpack(const uint8_t vv[4], int out0[3], int out1[3]) {
    static const int shamts[6] = {1, 1, 2, 3, 4, 5};
    int v0 = vv[0], v1 = vv[1], v2 = vv[2], v3 = vv[3];
    int modeval = ((v0 & 0xC0) >> 6) | (((v1 & 0x80) >> 7) << 2) |
                  (((v2 & 0x80) >> 7) << 3);
    int majcomp, mode, oh, sh;
    int red = v0 & 0x3F, green = v1 & 0x1F, blue = v2 & 0x1F, scale = v3 & 0x1F;
    int bit0 = (v1 >> 6) & 1, bit1 = (v1 >> 5) & 1, bit2 = (v2 >> 6) & 1;
    int bit3 = (v2 >> 5) & 1, bit4 = (v3 >> 7) & 1, bit5 = (v3 >> 6) & 1;
    int bit6 = (v3 >> 5) & 1;
    int r0, g0, b0;
    if ((modeval & 0xC) != 0xC) {
        majcomp = modeval >> 2;
        mode = modeval & 3;
    } else if (modeval != 0xF) {
        majcomp = modeval & 3;
        mode = 4;
    } else {
        majcomp = 0;
        mode = 5;
    }
    oh = 1 << mode;
    if (oh & 0x30) green |= bit0 << 6;
    if (oh & 0x3A) green |= bit1 << 5;
    if (oh & 0x30) blue |= bit2 << 6;
    if (oh & 0x3A) blue |= bit3 << 5;
    if (oh & 0x3D) scale |= bit6 << 5;
    if (oh & 0x2D) scale |= bit5 << 6;
    if (oh & 0x04) scale |= bit4 << 7;
    if (oh & 0x3B) red |= bit4 << 6;
    if (oh & 0x04) red |= bit3 << 6;
    if (oh & 0x10) red |= bit5 << 7;
    if (oh & 0x0F) red |= bit2 << 7;
    if (oh & 0x05) red |= bit1 << 8;
    if (oh & 0x0A) red |= bit0 << 8;
    if (oh & 0x05) red |= bit0 << 9;
    if (oh & 0x02) red |= bit6 << 9;
    if (oh & 0x01) red |= bit3 << 10;
    if (oh & 0x02) red |= bit5 << 10;
    sh = shamts[mode];
    red <<= sh;
    green <<= sh;
    blue <<= sh;
    scale <<= sh;
    if (mode != 5) {
        green = red - green;
        blue = red - blue;
    }
    if (majcomp == 1) {
        int t = red;
        red = green;
        green = t;
    } else if (majcomp == 2) {
        int t = red;
        red = blue;
        blue = t;
    }
    r0 = red - scale;
    g0 = green - scale;
    b0 = blue - scale;
    if (red < 0) red = 0;
    if (green < 0) green = 0;
    if (blue < 0) blue = 0;
    if (r0 < 0) r0 = 0;
    if (g0 < 0) g0 = 0;
    if (b0 < 0) b0 = 0;
    out0[0] = r0 << 4;
    out0[1] = g0 << 4;
    out0[2] = b0 << 4;
    out1[0] = red << 4;
    out1[1] = green << 4;
    out1[2] = blue << 4;
    return 1;
}

/* CEM 7 pack: fit base+scale to the two LNS endpoints (base = e1, scale =
 * mean(e1-e0)) and quantize with the astcenc quantize_hdr_rgbo mode search. */
void tc_astc_cem7_pack(const int e0[3], const int e1[3], int level,
                       uint8_t v[4]) {
    static const int mode_bits[5][3] = {{11, 5, 7}, {11, 6, 5}, {10, 5, 8},
                                        {9, 6, 7},  {8, 7, 6}};
    static const float mode_cut[5][2] = {{1024, 4096},  {2048, 1024},
                                         {2048, 16384}, {8192, 16384},
                                         {32768, 16384}};
    static const float mode_rscale[5] = {32.0f, 32.0f, 64.0f, 128.0f, 256.0f};
    static const float mode_scale[5] = {1.0f / 32,  1.0f / 32, 1.0f / 64,
                                        1.0f / 128, 1.0f / 256};
    float color[4], bak[4], r_base, g_base, b_base, s_base, s = 0.0f;
    int majcomp, mode, c;
    for (c = 0; c < 3; ++c) s += (float)(e1[c] - e0[c]);
    s *= 1.0f / 3.0f;
    if (s < 0.0f) s = 0.0f;
    for (c = 0; c < 3; ++c) color[c] = (float)e1[c] - s; /* += s -> base below */
    color[3] = s;
    for (c = 0; c < 3; ++c) color[c] += color[3];
    for (c = 0; c < 4; ++c) {
        if (color[c] < 0.0f) color[c] = 0.0f;
        if (color[c] > 65535.0f) color[c] = 65535.0f;
        bak[c] = color[c];
    }
    if (color[0] > color[1] && color[0] > color[2])
        majcomp = 0;
    else if (color[1] > color[2])
        majcomp = 1;
    else
        majcomp = 2;
    if (majcomp == 1) {
        float t = color[0];
        color[0] = color[1];
        color[1] = t;
    } else if (majcomp == 2) {
        float t = color[0];
        color[0] = color[2];
        color[2] = t;
    }
    r_base = color[0];
    g_base = color[0] - color[1];
    b_base = color[0] - color[2];
    s_base = color[3];
    for (mode = 0; mode < 5; ++mode) {
        float ms = mode_scale[mode], mr = mode_rscale[mode], r_fval, g_fval,
              b_fval, s_fval, rgb_err;
        int mode_enc, gb_ic, s_ic, r_intval, r_lowbits, r_q, g_intval, b_intval;
        int g_lowbits, b_lowbits, g_q, b_q, s_intval, s_lowbits, s_q;
        int bit0 = 0, bit1 = 0, bit2 = 0, bit3 = 0, bit4 = 0, bit5 = 0, bit6 = 0;
        if (g_base > mode_cut[mode][0] || b_base > mode_cut[mode][0] ||
            s_base > mode_cut[mode][1])
            continue;
        mode_enc = mode < 4 ? (mode | (majcomp << 2)) : (majcomp | 0xC);
        gb_ic = 1 << mode_bits[mode][1];
        s_ic = 1 << mode_bits[mode][2];
        r_intval = (int)(r_base * ms + 0.5f);
        r_lowbits = (r_intval & 0x3f) | ((mode_enc & 3) << 6);
        r_q = tc_cem11_retain(level, r_lowbits, 0xC0);
        r_intval = (r_intval & ~0x3f) | (r_q & 0x3f);
        r_fval = (float)r_intval * mr;
        g_fval = r_fval - color[1];
        b_fval = r_fval - color[2];
        if (g_fval < 0.0f) g_fval = 0.0f;
        if (g_fval > 65535.0f) g_fval = 65535.0f;
        if (b_fval < 0.0f) b_fval = 0.0f;
        if (b_fval > 65535.0f) b_fval = 65535.0f;
        g_intval = (int)(g_fval * ms + 0.5f);
        b_intval = (int)(b_fval * ms + 0.5f);
        if (g_intval >= gb_ic || b_intval >= gb_ic) continue;
        g_lowbits = g_intval & 0x1f;
        b_lowbits = b_intval & 0x1f;
        switch (mode) {
            case 0:
            case 2: bit0 = (r_intval >> 9) & 1; break;
            case 1:
            case 3: bit0 = (r_intval >> 8) & 1; break;
            case 4: bit0 = (g_intval >> 6) & 1; break;
        }
        switch (mode) {
            case 1:
            case 2:
            case 3: bit2 = (r_intval >> 7) & 1; break;
            case 4: bit2 = (b_intval >> 6) & 1; break;
        }
        switch (mode) {
            case 0:
            case 2: bit1 = (r_intval >> 8) & 1; break;
            default: bit1 = (g_intval >> 5) & 1; break;
        }
        switch (mode) {
            case 0: bit3 = (r_intval >> 10) & 1; break;
            case 2: bit3 = (r_intval >> 6) & 1; break;
            default: bit3 = (b_intval >> 5) & 1; break;
        }
        g_lowbits |= (mode_enc & 0x4) << 5;
        b_lowbits |= (mode_enc & 0x8) << 4;
        g_lowbits |= bit0 << 6;
        g_lowbits |= bit1 << 5;
        b_lowbits |= bit2 << 6;
        b_lowbits |= bit3 << 5;
        g_q = tc_cem11_retain(level, g_lowbits, 0xF0);
        b_q = tc_cem11_retain(level, b_lowbits, 0xF0);
        g_intval = (g_intval & ~0x1f) | (g_q & 0x1f);
        b_intval = (b_intval & ~0x1f) | (b_q & 0x1f);
        g_fval = (float)g_intval * mr;
        b_fval = (float)b_intval * mr;
        rgb_err = (r_fval - color[0]) + (r_fval - g_fval - color[1]) +
                  (r_fval - b_fval - color[2]);
        s_fval = s_base + rgb_err * (1.0f / 3.0f);
        if (s_fval < 0.0f) s_fval = 0.0f;
        if (s_fval > 1e9f) s_fval = 1e9f;
        s_intval = (int)(s_fval * ms + 0.5f);
        if (s_intval >= s_ic) continue;
        s_lowbits = s_intval & 0x1f;
        switch (mode) {
            case 1: bit6 = (r_intval >> 9) & 1; break;
            default: bit6 = (s_intval >> 5) & 1; break;
        }
        switch (mode) {
            case 4: bit5 = (r_intval >> 7) & 1; break;
            case 1: bit5 = (r_intval >> 10) & 1; break;
            default: bit5 = (s_intval >> 6) & 1; break;
        }
        switch (mode) {
            case 2: bit4 = (s_intval >> 7) & 1; break;
            default: bit4 = (r_intval >> 6) & 1; break;
        }
        s_lowbits |= bit6 << 5;
        s_lowbits |= bit5 << 6;
        s_lowbits |= bit4 << 7;
        s_q = tc_cem11_retain(level, s_lowbits, 0xF0);
        v[0] = (uint8_t)r_q;
        v[1] = (uint8_t)g_q;
        v[2] = (uint8_t)b_q;
        v[3] = (uint8_t)s_q;
        return;
    }
    /* fallback: mode 5 (absolute, 9-bit). */
    {
        float vals[4], cvals[3], rgb_err;
        int ivals[4], enc[4];
        for (c = 0; c < 4; ++c) vals[c] = bak[c];
        for (c = 0; c < 3; ++c) {
            if (vals[c] < 0.0f) vals[c] = 0.0f;
            if (vals[c] > 65020.0f) vals[c] = 65020.0f;
            ivals[c] = (int)(vals[c] * (1.0f / 512.0f) + 0.5f);
            cvals[c] = (float)ivals[c] * 512.0f;
        }
        rgb_err = (cvals[0] - vals[0]) + (cvals[1] - vals[1]) + (cvals[2] - vals[2]);
        vals[3] += rgb_err * (1.0f / 3.0f);
        if (vals[3] < 0.0f) vals[3] = 0.0f;
        if (vals[3] > 65020.0f) vals[3] = 65020.0f;
        ivals[3] = (int)(vals[3] * (1.0f / 512.0f) + 0.5f);
        enc[0] = (ivals[0] & 0x3f) | 0xC0;
        enc[1] = (ivals[1] & 0x7f) | 0x80;
        enc[2] = (ivals[2] & 0x7f) | 0x80;
        enc[3] = (ivals[3] & 0x7f) | ((ivals[0] & 0x40) << 1);
        for (c = 0; c < 4; ++c) v[c] = (uint8_t)tc_cem11_retain(level, enc[c], 0xF0);
    }
}

void tc_astc_hdr_options_init(tc_astc_hdr_options *opt) {
    if (!opt) return;
    memset(opt, 0, sizeof(*opt));
    opt->quality = 1;
}

size_t tc_astc_hdr_compressed_size(uint32_t width, uint32_t height) {
    size_t bx, by, blocks;
    if (!width || !height) return 0;
    bx = ((size_t)width + 3u) >> 2;  /* 4x4 blocks */
    by = ((size_t)height + 3u) >> 2;
    blocks = bx * by;
    if (blocks > (size_t)-1 / 16u) return 0;
    return blocks * 16u; /* 16 bytes per ASTC block */
}

/* Write a standard ASTC HDR constant-colour (void-extent) block: FP16 RGBA. */
static void tc_astc_hdr_write_void_extent(uint16_t r, uint16_t g, uint16_t b,
                                          uint16_t a, uint8_t out[16]) {
    /* Void-extent marker + "all coordinates" (no extent) for an FP16 (HDR)
     * constant-colour block. See the ASTC spec / astcenc symbolic_to_physical:
     * bytes 0..7 = FC FF FF FF FF FF FF FF, bytes 8..15 = 4x FP16 (little
     * endian) in R,G,B,A order. (An LDR UNORM16 void-extent uses byte 1 = FD.) */
    out[0] = 0xFCu;
    out[1] = 0xFFu;
    out[2] = 0xFFu;
    out[3] = 0xFFu;
    out[4] = 0xFFu;
    out[5] = 0xFFu;
    out[6] = 0xFFu;
    out[7] = 0xFFu;
    out[8] = (uint8_t)(r & 0xFFu);
    out[9] = (uint8_t)(r >> 8);
    out[10] = (uint8_t)(g & 0xFFu);
    out[11] = (uint8_t)(g >> 8);
    out[12] = (uint8_t)(b & 0xFFu);
    out[13] = (uint8_t)(b >> 8);
    out[14] = (uint8_t)(a & 0xFFu);
    out[15] = (uint8_t)(a >> 8);
}

tc_result tc_astc_hdr_compress_rgbf(const float *rgb, uint32_t width,
                                    uint32_t height, size_t stride_bytes,
                                    const tc_astc_hdr_options *opt,
                                    uint8_t *out_astc, size_t out_size) {
    uint32_t bx, by, x, y, xx, yy;
    size_t need, off = 0;
    (void)opt;

    if (!rgb || !out_astc || !width || !height) return TC_ERROR_INVALID_ARGUMENT;
    if (stride_bytes < (size_t)width * 3u * sizeof(float))
        return TC_ERROR_INVALID_ARGUMENT;
    need = tc_astc_hdr_compressed_size(width, height);
    if (!need || out_size < need) return TC_ERROR_INVALID_ARGUMENT;

    for (by = 0; by < height; by += 4) {
        for (bx = 0; bx < width; bx += 4) {
            int lns[16][3];
            float src0[3] = {0.0f, 0.0f, 0.0f};
            int allsame = 1;
            for (yy = 0; yy < 4; ++yy) {
                y = by + yy;
                if (y >= height) y = height - 1u;
                for (xx = 0; xx < 4; ++xx) {
                    const float *src;
                    int idx = (int)(yy * 4u + xx);
                    x = bx + xx;
                    if (x >= width) x = width - 1u;
                    src = (const float *)((const uint8_t *)rgb +
                                          (size_t)y * stride_bytes +
                                          (size_t)x * 3u * sizeof(float));
                    lns[idx][0] = tc_astc_float_to_lns16(src[0]);
                    lns[idx][1] = tc_astc_float_to_lns16(src[1]);
                    lns[idx][2] = tc_astc_float_to_lns16(src[2]);
                    if (idx == 0) {
                        src0[0] = src[0];
                        src0[1] = src[1];
                        src0[2] = src[2];
                    } else if (lns[idx][0] != lns[0][0] ||
                               lns[idx][1] != lns[0][1] ||
                               lns[idx][2] != lns[0][2]) {
                        allsame = 0;
                    }
                }
            }
            if (allsame) {
                /* Constant block: an FP16 void-extent is exact and cheaper. */
                tc_astc_hdr_write_void_extent(tc_float_to_half_bits(src0[0]),
                                              tc_float_to_half_bits(src0[1]),
                                              tc_float_to_half_bits(src0[2]),
                                              0x3C00u, out_astc + off);
            } else {
                /* Try CEM 11 (single/dual plane), 2-subset partitions, and a
                 * constant (block-mean void-extent); keep whichever
                 * reconstructs the block with the least LNS-domain error. */
                uint8_t cem[16], sub[16], cem7[16];
                uint64_t cem_sse, sub_sse, cem7_sse, ve_sse = 0, best_sse;
                const uint8_t *best_buf;
                int mean[3];
                int64_t sum[3] = {0, 0, 0};
                int i, cc;
                cem_sse = tc_encode_astc_hdr_cem11_block(lns, cem);
                sub_sse = tc_encode_astc_hdr_cem11_2subset_block(lns, sub);
                cem7_sse = tc_encode_astc_hdr_cem7_block(lns, cem7);
                best_buf = cem;
                best_sse = cem_sse;
                if (sub_sse < best_sse) {
                    best_sse = sub_sse;
                    best_buf = sub;
                }
                if (cem7_sse < best_sse) {
                    best_sse = cem7_sse;
                    best_buf = cem7;
                }
                for (i = 0; i < 16; ++i)
                    for (cc = 0; cc < 3; ++cc) sum[cc] += lns[i][cc];
                for (cc = 0; cc < 3; ++cc) mean[cc] = (int)(sum[cc] / 16);
                for (i = 0; i < 16; ++i)
                    for (cc = 0; cc < 3; ++cc) {
                        int64_t e = (int64_t)lns[i][cc] - mean[cc];
                        ve_sse += (uint64_t)(e * e);
                    }
                if (ve_sse <= best_sse) {
                    tc_astc_hdr_write_void_extent(
                        tc_astc_lns16_to_sf16(mean[0]),
                        tc_astc_lns16_to_sf16(mean[1]),
                        tc_astc_lns16_to_sf16(mean[2]), 0x3C00u, out_astc + off);
                } else {
                    memcpy(out_astc + off, best_buf, 16u);
                }
            }
            off += 16u;
        }
    }
    return TC_SUCCESS;
}

/* RGBA HDR: like tc_astc_hdr_compress_rgbf but with an HDR alpha channel, using
 * the CEM 15 (HDR RGB + HDR alpha) single-subset block encoder. Constant blocks
 * still use an exact FP16 void-extent (now carrying real alpha). `rgba` is
 * per-texel float RGBA. */
tc_result tc_astc_hdr_compress_rgbaf(const float *rgba, uint32_t width,
                                     uint32_t height, size_t stride_bytes,
                                     const tc_astc_hdr_options *opt,
                                     uint8_t *out_astc, size_t out_size) {
    uint32_t bx, by, x, y, xx, yy;
    size_t need, off = 0;
    (void)opt;

    if (!rgba || !out_astc || !width || !height)
        return TC_ERROR_INVALID_ARGUMENT;
    if (stride_bytes < (size_t)width * 4u * sizeof(float))
        return TC_ERROR_INVALID_ARGUMENT;
    need = tc_astc_hdr_compressed_size(width, height);
    if (!need || out_size < need) return TC_ERROR_INVALID_ARGUMENT;

    for (by = 0; by < height; by += 4) {
        for (bx = 0; bx < width; bx += 4) {
            int lns[16][4];
            float src0[4] = {0.0f, 0.0f, 0.0f, 0.0f};
            int allsame = 1;
            for (yy = 0; yy < 4; ++yy) {
                y = by + yy;
                if (y >= height) y = height - 1u;
                for (xx = 0; xx < 4; ++xx) {
                    const float *src;
                    int idx = (int)(yy * 4u + xx);
                    x = bx + xx;
                    if (x >= width) x = width - 1u;
                    src = (const float *)((const uint8_t *)rgba +
                                          (size_t)y * stride_bytes +
                                          (size_t)x * 4u * sizeof(float));
                    lns[idx][0] = tc_astc_float_to_lns16(src[0]);
                    lns[idx][1] = tc_astc_float_to_lns16(src[1]);
                    lns[idx][2] = tc_astc_float_to_lns16(src[2]);
                    lns[idx][3] = tc_astc_float_to_lns16(src[3]);
                    if (idx == 0) {
                        src0[0] = src[0];
                        src0[1] = src[1];
                        src0[2] = src[2];
                        src0[3] = src[3];
                    } else if (lns[idx][0] != lns[0][0] ||
                               lns[idx][1] != lns[0][1] ||
                               lns[idx][2] != lns[0][2] ||
                               lns[idx][3] != lns[0][3]) {
                        allsame = 0;
                    }
                }
            }
            if (allsame) {
                tc_astc_hdr_write_void_extent(tc_float_to_half_bits(src0[0]),
                                              tc_float_to_half_bits(src0[1]),
                                              tc_float_to_half_bits(src0[2]),
                                              tc_float_to_half_bits(src0[3]),
                                              out_astc + off);
            } else {
                uint8_t cem15[16];
                uint64_t cem15_sse, ve_sse = 0;
                int mean[4], i, cc;
                int64_t sum[4] = {0, 0, 0, 0};
                cem15_sse = tc_encode_astc_hdr_cem15_block(lns, cem15);
                for (i = 0; i < 16; ++i)
                    for (cc = 0; cc < 4; ++cc) sum[cc] += lns[i][cc];
                for (cc = 0; cc < 4; ++cc) mean[cc] = (int)(sum[cc] / 16);
                for (i = 0; i < 16; ++i)
                    for (cc = 0; cc < 4; ++cc) {
                        int64_t e = (int64_t)lns[i][cc] - mean[cc];
                        ve_sse += (uint64_t)(e * e);
                    }
                if (ve_sse <= cem15_sse) {
                    tc_astc_hdr_write_void_extent(
                        tc_astc_lns16_to_sf16(mean[0]),
                        tc_astc_lns16_to_sf16(mean[1]),
                        tc_astc_lns16_to_sf16(mean[2]),
                        tc_astc_lns16_to_sf16(mean[3]), out_astc + off);
                } else {
                    memcpy(out_astc + off, cem15, 16u);
                }
            }
            off += 16u;
        }
    }
    return TC_SUCCESS;
}
