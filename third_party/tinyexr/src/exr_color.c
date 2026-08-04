/*
 * TinyEXR - colorspace conversion, transfer functions, and 3D LUT (util
 * module).
 *
 * Primary-conversion matrices are published constants (Bradford-adapted across
 * whitepoints at runtime). Transfer functions use hand-rolled, libm-free
 * exp2/log2/pow so the whole module stays in the freestanding core. Scalar
 * matrix apply is the dispatched mat3 reference.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

/* ============================================================================
 * Bit-pattern constants (avoid libm / division-by-zero literals)
 * ========================================================================== */
static float bits_to_f(uint32_t u) {
    float f;
    memcpy(&f, &u, sizeof(f));
    return f;
}
#define EXR_INF_POS bits_to_f(0x7f800000u)
#define EXR_INF_NEG bits_to_f(0xff800000u)
#define EXR_NANF bits_to_f(0x7fc00000u)

/* ============================================================================
 * Hand-rolled transcendentals (~1e-6 rel; verified vs libm in tests)
 * ========================================================================== */
float exr_util_log2f(float x) {
    union { float f; uint32_t u; } v;
    int e;
    float m, t, t2, ln;
    if (!(x > 0.0f)) return x < 0.0f ? EXR_NANF : EXR_INF_NEG;
    v.f = x;
    e = (int)((v.u >> 23) & 0xffu) - 127;
    v.u = (v.u & 0x007fffffu) | 0x3f800000u; /* mantissa in [1,2) */
    m = v.f;
    /* ln(m) via atanh series: ln(m) = 2*(t + t^3/3 + t^5/5 + ...), t<=1/3. */
    t = (m - 1.0f) / (m + 1.0f);
    t2 = t * t;
    ln = 2.0f * t *
         (1.0f + t2 * (1.0f / 3.0f +
                       t2 * (1.0f / 5.0f +
                             t2 * (1.0f / 7.0f + t2 * (1.0f / 9.0f)))));
    return (float)e + ln * 1.4426950408889634f;
}

float exr_util_exp2f(float x) {
    union { uint32_t u; float f; } v;
    float k, f, g, p;
    int ki;
    if (x > 127.0f) return EXR_INF_POS;
    if (x < -126.0f) return 0.0f;
    k = (x >= 0.0f) ? (float)(int)(x + 0.5f) : (float)(int)(x - 0.5f);
    f = x - k; /* [-0.5, 0.5] */
    g = f * 0.6931471805599453f;
    /* 2^f = e^(f ln2) = sum g^n/n! */
    p = 1.0f +
        g * (1.0f +
             g * (0.5f +
                  g * (1.0f / 6.0f +
                       g * (1.0f / 24.0f +
                            g * (1.0f / 120.0f + g * (1.0f / 720.0f))))));
    ki = (int)k;
    v.u = (uint32_t)((ki + 127) << 23);
    return p * v.f;
}

float exr_util_powf(float x, float y) {
    if (!(x > 0.0f)) {
        if (x == 0.0f) return (y > 0.0f) ? 0.0f : (y == 0.0f ? 1.0f : EXR_INF_POS);
        return EXR_NANF;
    }
    return exr_util_exp2f(y * exr_util_log2f(x));
}

float exr_util_sqrtf(float x) {
    if (!(x > 0.0f)) return 0.0f;
    return exr_util_exp2f(0.5f * exr_util_log2f(x));
}

static float util_lnf(float x) { return exr_util_log2f(x) * 0.6931471805599453f; }
static float util_expf(float x) { return exr_util_exp2f(x * 1.4426950408889634f); }

/* ============================================================================
 * 3x3 matrix apply (scalar reference for the dispatched mat3 kernel)
 * ========================================================================== */
void exr_mat3_scalar(float *dst, const float *src, size_t px, int ch,
                     const float *m) {
    size_t i;
    if (ch < 3) {
        if (dst != src) memmove(dst, src, px * (size_t)ch * sizeof(float));
        return;
    }
    for (i = 0; i < px; ++i) {
        const float *s = src + i * (size_t)ch;
        float *d = dst + i * (size_t)ch;
        float r = s[0], g = s[1], b = s[2];
        d[0] = m[0] * r + m[1] * g + m[2] * b;
        d[1] = m[3] * r + m[4] * g + m[5] * b;
        d[2] = m[6] * r + m[7] * g + m[8] * b;
        if (ch == 4) d[3] = s[3];
    }
}

exr_result exr_color_apply_matrix(float *dst, const float *src,
                                  size_t pixel_count, int channels,
                                  const float m[9]) {
    if (!dst || !src || !m || channels < 1 || channels > 4)
        return EXR_ERROR_INVALID_ARGUMENT;
    exr_simd_init();
    exr_simd.mat3(dst, src, pixel_count, channels, m);
    return EXR_SUCCESS;
}

/* ============================================================================
 * Primary matrices (row-major). RGB<->XYZ in each space's own whitepoint.
 * ========================================================================== */
/* whitepoint id per space: 0 = D65, 1 = ACES ~D60. */
static const int g_white_id[5] = {0, 0, 1, 1, 0};
static const float g_white_xyz[2][3] = {
    {0.95047f, 1.0f, 1.08883f},        /* D65 */
    {0.952646f, 1.0f, 1.008825f}       /* ACES ~D60 */
};

static const float g_rgb2xyz[5][9] = {
    /* sRGB / Rec.709 (D65) */
    {0.4123907993f, 0.3575843394f, 0.1804807884f,
     0.2126390059f, 0.7151686788f, 0.0721923154f,
     0.0193308187f, 0.1191947798f, 0.9505321522f},
    /* Rec.2020 (D65) */
    {0.6369580483f, 0.1446169036f, 0.1688809752f,
     0.2627002120f, 0.6779980715f, 0.0593017165f,
     0.0000000000f, 0.0280726930f, 1.0609850577f},
    /* ACES AP0 (~D60) */
    {0.9525523959f, 0.0000000000f, 0.0000936786f,
     0.3439664498f, 0.7281660966f, -0.0721325464f,
     0.0000000000f, 0.0000000000f, 1.0088251844f},
    /* ACES AP1 / ACEScg (~D60) */
    {0.6624541811f, 0.1340042065f, 0.1561876870f,
     0.2722287168f, 0.6740817658f, 0.0536895174f,
     -0.0055746495f, 0.0040607335f, 1.0103391003f},
    /* XYZ (identity) */
    {1, 0, 0, 0, 1, 0, 0, 0, 1}};

static const float g_xyz2rgb[5][9] = {
    /* XYZ(D65) -> sRGB/Rec.709 */
    {3.2409699419f, -1.5373831776f, -0.4986107603f,
     -0.9692436363f, 1.8759675015f, 0.0415550574f,
     0.0556300797f, -0.2039769589f, 1.0569715142f},
    /* XYZ(D65) -> Rec.2020 */
    {1.7166511880f, -0.3556707838f, -0.2533662814f,
     -0.6666843518f, 1.6164812366f, 0.0157685458f,
     0.0176398574f, -0.0427706133f, 0.9421031212f},
    /* XYZ -> AP0 */
    {1.0498110175f, 0.0000000000f, -0.0000974845f,
     -0.4959030231f, 1.3733130458f, 0.0982400361f,
     0.0000000000f, 0.0000000000f, 0.9912520182f},
    /* XYZ -> AP1 */
    {1.6410233797f, -0.3248032942f, -0.2364246952f,
     -0.6636628587f, 1.6153315917f, 0.0167563477f,
     0.0117218943f, -0.0082844420f, 0.9883948585f},
    /* XYZ identity */
    {1, 0, 0, 0, 1, 0, 0, 0, 1}};

/* Bradford cone response and its inverse. */
static const float g_bradford[9] = {
    0.8951000f, 0.2664000f, -0.1614000f,
    -0.7502000f, 1.7135000f, 0.0367000f,
    0.0389000f, -0.0685000f, 1.0296000f};
static const float g_bradford_inv[9] = {
    0.9869929f, -0.1470543f, 0.1599627f,
    0.4323053f, 0.5183603f, 0.0492912f,
    -0.0085287f, 0.0400428f, 0.9684867f};

static void mat3_mul(const float *a, const float *b, float *out) {
    int r, c, k;
    float tmp[9];
    for (r = 0; r < 3; ++r)
        for (c = 0; c < 3; ++c) {
            float s = 0.0f;
            for (k = 0; k < 3; ++k) s += a[r * 3 + k] * b[k * 3 + c];
            tmp[r * 3 + c] = s;
        }
    memcpy(out, tmp, sizeof(tmp));
}
static void mat3_vec(const float *m, const float *v, float *out) {
    out[0] = m[0] * v[0] + m[1] * v[1] + m[2] * v[2];
    out[1] = m[3] * v[0] + m[4] * v[1] + m[5] * v[2];
    out[2] = m[6] * v[0] + m[7] * v[1] + m[8] * v[2];
}

/* Bradford adaptation matrix from white wf to white wt (both XYZ). */
static void adaptation(const float *wf, const float *wt, float *out) {
    float cs[3], cd[3], d[9], tmp[9];
    int i;
    mat3_vec(g_bradford, wf, cs);
    mat3_vec(g_bradford, wt, cd);
    memset(d, 0, sizeof(d));
    for (i = 0; i < 3; ++i) d[i * 3 + i] = (cs[i] != 0.0f) ? cd[i] / cs[i] : 1.0f;
    mat3_mul(d, g_bradford, tmp);
    mat3_mul(g_bradford_inv, tmp, out);
}

exr_result exr_color_matrix(exr_colorspace from, exr_colorspace to,
                            float m[9]) {
    float adapt[9], tmp[9];
    if (!m || (int)from < 0 || (int)from > 4 || (int)to < 0 || (int)to > 4)
        return EXR_ERROR_INVALID_ARGUMENT;
    if (from == to) {
        static const float I[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
        memcpy(m, I, sizeof(I));
        return EXR_SUCCESS;
    }
    if (g_white_id[from] == g_white_id[to]) {
        mat3_mul(g_xyz2rgb[to], g_rgb2xyz[from], m);
        return EXR_SUCCESS;
    }
    adaptation(g_white_xyz[g_white_id[from]], g_white_xyz[g_white_id[to]],
               adapt);
    mat3_mul(adapt, g_rgb2xyz[from], tmp);
    mat3_mul(g_xyz2rgb[to], tmp, m);
    return EXR_SUCCESS;
}

/* ============================================================================
 * Luminance weights from CIE chromaticities (for Y/RY/BY reconstruction)
 * ========================================================================== */
/* Rec.709 / sRGB luminance weights (used when no chromaticities are present). */
static const float g_yw_rec709[3] = {0.2126f, 0.7152f, 0.0722f};

exr_result exr_luminance_weights(const float chroma[8], int present,
                                 float yw[3]) {
    /* Port of OpenEXR ImfChromaticities computeYw(): the Y (luminance) row of
     * the chromaticity-derived RGB->XYZ matrix (with Y(white)=1), normalised so
     * the three weights sum to 1. Falls back to Rec.709 when chromaticities are
     * absent or degenerate. Pure arithmetic, libm-free. */
    float rx, ry, gx, gy, bx, by, wx, wy;
    float X, Z, d, Sr, Sg, Sb, sum;
    if (!yw) return EXR_ERROR_INVALID_ARGUMENT;
    if (!present || !chroma) {
        yw[0] = g_yw_rec709[0]; yw[1] = g_yw_rec709[1]; yw[2] = g_yw_rec709[2];
        return EXR_SUCCESS;
    }
    rx = chroma[0]; ry = chroma[1];
    gx = chroma[2]; gy = chroma[3];
    bx = chroma[4]; by = chroma[5];
    wx = chroma[6]; wy = chroma[7];
    d = rx * (by - gy) + bx * (gy - ry) + gx * (ry - by);
    if (wy == 0.0f || d == 0.0f) {
        yw[0] = g_yw_rec709[0]; yw[1] = g_yw_rec709[1]; yw[2] = g_yw_rec709[2];
        return EXR_SUCCESS;
    }
    X = wx / wy;                  /* white X, with Y == 1 */
    Z = (1.0f - wx - wy) / wy;    /* white Z */
    Sr = (X * (by - gy) - gx * ((by - 1.0f) + by * (X + Z)) +
          bx * ((gy - 1.0f) + gy * (X + Z))) / d;
    Sg = (X * (ry - by) + rx * ((by - 1.0f) + by * (X + Z)) -
          bx * ((ry - 1.0f) + ry * (X + Z))) / d;
    Sb = (X * (gy - ry) - rx * ((gy - 1.0f) + gy * (X + Z)) +
          gx * ((ry - 1.0f) + ry * (X + Z))) / d;
    yw[0] = Sr * ry; yw[1] = Sg * gy; yw[2] = Sb * by;
    sum = yw[0] + yw[1] + yw[2];
    if (sum != 0.0f) { yw[0] /= sum; yw[1] /= sum; yw[2] /= sum; }
    return EXR_SUCCESS;
}

/* ============================================================================
 * Transfer functions
 * ========================================================================== */
static float nonneg(float x) { return (x > 0.0f) ? x : 0.0f; }

static float oetf_one(float L, exr_transfer tf) {
    switch (tf) {
        case EXR_TF_SRGB:
            L = nonneg(L);
            return L <= 0.0031308f ? 12.92f * L
                                   : 1.055f * exr_util_powf(L, 1.0f / 2.4f) -
                                         0.055f;
        case EXR_TF_GAMMA_22:
            return exr_util_powf(nonneg(L), 1.0f / 2.2f);
        case EXR_TF_GAMMA_24:
            return exr_util_powf(nonneg(L), 1.0f / 2.4f);
        case EXR_TF_REC709:
            L = nonneg(L);
            return L < 0.018f ? 4.5f * L
                              : 1.099f * exr_util_powf(L, 0.45f) - 0.099f;
        case EXR_TF_PQ: {
            float Lp, num, den;
            L = nonneg(L);
            Lp = exr_util_powf(L, 0.1593017578125f);
            num = 0.8359375f + 18.8515625f * Lp;
            den = 1.0f + 18.6875f * Lp;
            return exr_util_powf(num / den, 78.84375f);
        }
        case EXR_TF_HLG:
            L = nonneg(L);
            return L <= 1.0f / 12.0f
                       ? exr_util_sqrtf(3.0f * L)
                       : 0.17883277f * util_lnf(12.0f * L - 0.28466892f) +
                             0.55991073f;
        default: /* LINEAR */
            return L;
    }
}

static float eotf_one(float V, exr_transfer tf) {
    switch (tf) {
        case EXR_TF_SRGB:
            return V <= 0.04045f
                       ? V / 12.92f
                       : exr_util_powf((V + 0.055f) / 1.055f, 2.4f);
        case EXR_TF_GAMMA_22:
            return exr_util_powf(nonneg(V), 2.2f);
        case EXR_TF_GAMMA_24:
            return exr_util_powf(nonneg(V), 2.4f);
        case EXR_TF_REC709:
            return V < 0.081f
                       ? V / 4.5f
                       : exr_util_powf((V + 0.099f) / 1.099f, 1.0f / 0.45f);
        case EXR_TF_PQ: {
            float Vp, num, den;
            Vp = exr_util_powf(nonneg(V), 1.0f / 78.84375f);
            num = nonneg(Vp - 0.8359375f);
            den = 18.8515625f - 18.6875f * Vp;
            return exr_util_powf(num / den, 1.0f / 0.1593017578125f);
        }
        case EXR_TF_HLG:
            return V <= 0.5f
                       ? V * V / 3.0f
                       : (util_expf((V - 0.55991073f) / 0.17883277f) +
                          0.28466892f) /
                             12.0f;
        default:
            return V;
    }
}

exr_result exr_encode_transfer(float *dst, const float *src, size_t count,
                               exr_transfer tf) {
    size_t i;
    if (!dst || !src) return EXR_ERROR_INVALID_ARGUMENT;
    if (tf == EXR_TF_LINEAR) {
        if (dst != src) memmove(dst, src, count * sizeof(float));
        return EXR_SUCCESS;
    }
    for (i = 0; i < count; ++i) dst[i] = oetf_one(src[i], tf);
    return EXR_SUCCESS;
}

exr_result exr_decode_transfer(float *dst, const float *src, size_t count,
                               exr_transfer tf) {
    size_t i;
    if (!dst || !src) return EXR_ERROR_INVALID_ARGUMENT;
    if (tf == EXR_TF_LINEAR) {
        if (dst != src) memmove(dst, src, count * sizeof(float));
        return EXR_SUCCESS;
    }
    for (i = 0; i < count; ++i) dst[i] = eotf_one(src[i], tf);
    return EXR_SUCCESS;
}

/* ============================================================================
 * 3D LUT (trilinear + tetrahedral), R-fastest storage
 * ========================================================================== */
static float lerp(float a, float b, float t) { return a + (b - a) * t; }

static void lut_index(const exr_lut3d *lut, int ir, int ig, int ib,
                      const float **rgb) {
    size_t idx = (((size_t)ib * lut->size + ig) * lut->size + ir) * 3;
    *rgb = lut->data + idx;
}

exr_result exr_lut3d_apply(float *dst, const float *src, size_t pixel_count,
                           int channels, const exr_lut3d *lut,
                           exr_lut_interp interp) {
    size_t i;
    int N;
    if (!dst || !src || !lut || !lut->data || channels < 3 || channels > 4)
        return EXR_ERROR_INVALID_ARGUMENT;
    N = lut->size;
    if (N < 2) return EXR_ERROR_INVALID_ARGUMENT;

    for (i = 0; i < pixel_count; ++i) {
        const float *s = src + i * (size_t)channels;
        float *d = dst + i * (size_t)channels;
        float p[3], f[3];
        int ic[3], c;
        for (c = 0; c < 3; ++c) {
            float lo = lut->domain_min[c], hi = lut->domain_max[c];
            float den = (hi - lo);
            float u = (den != 0.0f) ? (s[c] - lo) / den : 0.0f;
            float g = u * (float)(N - 1);
            if (!(g > 0.0f)) g = 0.0f;
            if (g > (float)(N - 1)) g = (float)(N - 1);
            ic[c] = (int)g;
            if (ic[c] >= N - 1) ic[c] = N - 2;
            f[c] = g - (float)ic[c];
            p[c] = g;
        }
        (void)p;
        if (interp == EXR_LUT_TETRAHEDRAL) {
            /* 6-case barycentric over the cube cell. */
            const float *c000, *c111;
            float fr = f[0], fg = f[1], fb = f[2], out[3];
            int ir = ic[0], ig = ic[1], ib = ic[2], k;
            const float *p100, *p010, *p001, *p110, *p101, *p011;
            lut_index(lut, ir, ig, ib, &c000);
            lut_index(lut, ir + 1, ig + 1, ib + 1, &c111);
            lut_index(lut, ir + 1, ig, ib, &p100);
            lut_index(lut, ir, ig + 1, ib, &p010);
            lut_index(lut, ir, ig, ib + 1, &p001);
            lut_index(lut, ir + 1, ig + 1, ib, &p110);
            lut_index(lut, ir + 1, ig, ib + 1, &p101);
            lut_index(lut, ir, ig + 1, ib + 1, &p011);
            for (k = 0; k < 3; ++k) {
                float v;
                if (fr >= fg && fg >= fb)
                    v = c000[k] + fr * (p100[k] - c000[k]) +
                        fg * (p110[k] - p100[k]) + fb * (c111[k] - p110[k]);
                else if (fr >= fb && fb >= fg)
                    v = c000[k] + fr * (p100[k] - c000[k]) +
                        fb * (p101[k] - p100[k]) + fg * (c111[k] - p101[k]);
                else if (fb >= fr && fr >= fg)
                    v = c000[k] + fb * (p001[k] - c000[k]) +
                        fr * (p101[k] - p001[k]) + fg * (c111[k] - p101[k]);
                else if (fg >= fr && fr >= fb)
                    v = c000[k] + fg * (p010[k] - c000[k]) +
                        fr * (p110[k] - p010[k]) + fb * (c111[k] - p110[k]);
                else if (fg >= fb && fb >= fr)
                    v = c000[k] + fg * (p010[k] - c000[k]) +
                        fb * (p011[k] - p010[k]) + fr * (c111[k] - p011[k]);
                else /* fb >= fg >= fr */
                    v = c000[k] + fb * (p001[k] - c000[k]) +
                        fg * (p011[k] - p001[k]) + fr * (c111[k] - p011[k]);
                out[k] = v;
            }
            d[0] = out[0]; d[1] = out[1]; d[2] = out[2];
        } else { /* trilinear */
            const float *c[8];
            float c00[3], c01[3], c10[3], c11[3], c0[3], c1[3];
            int k;
            lut_index(lut, ic[0], ic[1], ic[2], &c[0]);
            lut_index(lut, ic[0] + 1, ic[1], ic[2], &c[1]);
            lut_index(lut, ic[0], ic[1] + 1, ic[2], &c[2]);
            lut_index(lut, ic[0] + 1, ic[1] + 1, ic[2], &c[3]);
            lut_index(lut, ic[0], ic[1], ic[2] + 1, &c[4]);
            lut_index(lut, ic[0] + 1, ic[1], ic[2] + 1, &c[5]);
            lut_index(lut, ic[0], ic[1] + 1, ic[2] + 1, &c[6]);
            lut_index(lut, ic[0] + 1, ic[1] + 1, ic[2] + 1, &c[7]);
            for (k = 0; k < 3; ++k) {
                c00[k] = lerp(c[0][k], c[1][k], f[0]);
                c01[k] = lerp(c[4][k], c[5][k], f[0]);
                c10[k] = lerp(c[2][k], c[3][k], f[0]);
                c11[k] = lerp(c[6][k], c[7][k], f[0]);
                c0[k] = lerp(c00[k], c10[k], f[1]);
                c1[k] = lerp(c01[k], c11[k], f[1]);
                d[k] = lerp(c0[k], c1[k], f[2]);
            }
        }
        if (channels == 4) d[3] = s[3];
    }
    return EXR_SUCCESS;
}

/* ============================================================================
 * .cube parser (libm-free; no strtod)
 * ========================================================================== */
static int parse_float(const char **pp, const char *end, float *out) {
    const char *p = *pp;
    int sign = 1, any = 0;
    double v = 0.0, frac = 0.0, scale = 1.0;
    int exp = 0, esign = 1, has_exp = 0;
    while (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
        ++p;
    if (p < end && (*p == '+' || *p == '-')) {
        if (*p == '-') sign = -1;
        ++p;
    }
    while (p < end && *p >= '0' && *p <= '9') {
        v = v * 10.0 + (*p - '0');
        ++p;
        any = 1;
    }
    if (p < end && *p == '.') {
        ++p;
        while (p < end && *p >= '0' && *p <= '9') {
            scale *= 10.0;
            frac = frac + (*p - '0') / scale;
            ++p;
            any = 1;
        }
    }
    if (!any) return 0;
    if (p < end && (*p == 'e' || *p == 'E')) {
        ++p;
        has_exp = 1;
        if (p < end && (*p == '+' || *p == '-')) {
            if (*p == '-') esign = -1;
            ++p;
        }
        while (p < end && *p >= '0' && *p <= '9') {
            exp = exp * 10 + (*p - '0');
            ++p;
        }
    }
    v = (v + frac) * sign;
    if (has_exp) {
        int i;
        double m = 1.0;
        for (i = 0; i < exp; ++i) m *= 10.0;
        v = esign < 0 ? v / m : v * m;
    }
    *out = (float)v;
    *pp = p;
    return 1;
}

/* Match a keyword at line start (case-sensitive), return ptr past it or NULL. */
static const char *match_kw(const char *p, const char *end, const char *kw) {
    size_t k = strlen(kw);
    if ((size_t)(end - p) < k) return NULL;
    if (memcmp(p, kw, k) != 0) return NULL;
    return p + k;
}

exr_result exr_lut3d_parse_cube(const exr_allocator *a, const char *text,
                                size_t len, exr_lut3d *out, float **out_owned) {
    const char *p, *end;
    int N = 0;
    size_t want = 0, got = 0;
    float dmin[3] = {0, 0, 0}, dmax[3] = {1, 1, 1};
    float *data = NULL;
    if (!a) a = exr_default_allocator();
    if (!text || !out || !out_owned) return EXR_ERROR_INVALID_ARGUMENT;
    *out_owned = NULL;
    p = text;
    end = text + len;

    while (p < end) {
        const char *kw;
        /* skip leading whitespace */
        while (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
            ++p;
        if (p >= end) break;
        if (*p == '#') { /* comment to EOL */
            while (p < end && *p != '\n') ++p;
            continue;
        }
        if (match_kw(p, end, "TITLE")) {
            while (p < end && *p != '\n') ++p;
            continue;
        }
        if (match_kw(p, end, "LUT_1D_SIZE")) {
            if (data) exr_free(a, data);
            return EXR_ERROR_UNSUPPORTED;
        }
        if ((kw = match_kw(p, end, "LUT_3D_SIZE")) != NULL) {
            float fn;
            p = kw;
            if (!parse_float(&p, end, &fn)) return EXR_ERROR_CORRUPT;
            N = (int)fn;
            if (N < 2 || N > 256) return EXR_ERROR_UNSUPPORTED;
            if (exr_mul_ovf((size_t)N * N, (size_t)N * 3, &want))
                return EXR_ERROR_CORRUPT;
            data = (float *)exr_malloc(a, want * sizeof(float));
            if (!data) return EXR_ERROR_OUT_OF_MEMORY;
            continue;
        }
        if ((kw = match_kw(p, end, "DOMAIN_MIN")) != NULL) {
            p = kw;
            if (!parse_float(&p, end, &dmin[0]) ||
                !parse_float(&p, end, &dmin[1]) ||
                !parse_float(&p, end, &dmin[2]))
                goto corrupt;
            continue;
        }
        if ((kw = match_kw(p, end, "DOMAIN_MAX")) != NULL) {
            p = kw;
            if (!parse_float(&p, end, &dmax[0]) ||
                !parse_float(&p, end, &dmax[1]) ||
                !parse_float(&p, end, &dmax[2]))
                goto corrupt;
            continue;
        }
        /* Otherwise a data triple (or stray token). */
        if (*p == '+' || *p == '-' || *p == '.' || (*p >= '0' && *p <= '9')) {
            float r, g, b;
            if (!N || !data) goto corrupt;
            if (!parse_float(&p, end, &r) || !parse_float(&p, end, &g) ||
                !parse_float(&p, end, &b))
                goto corrupt;
            if (got + 3 > want) goto corrupt;
            data[got++] = r;
            data[got++] = g;
            data[got++] = b;
        } else {
            /* unknown keyword: skip line */
            while (p < end && *p != '\n') ++p;
        }
    }
    if (!data || got != want) goto corrupt;
    out->size = N;
    out->data = data;
    out->domain_min[0] = dmin[0]; out->domain_min[1] = dmin[1];
    out->domain_min[2] = dmin[2];
    out->domain_max[0] = dmax[0]; out->domain_max[1] = dmax[1];
    out->domain_max[2] = dmax[2];
    *out_owned = data;
    return EXR_SUCCESS;
corrupt:
    if (data) exr_free(a, data);
    return EXR_ERROR_CORRUPT;
}
