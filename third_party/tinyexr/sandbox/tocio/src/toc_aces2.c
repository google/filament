/*
 * tocio - ACES 2.0 output transform (CAM16 JMh tonescale + gamut compression).
 *
 * Faithful pure-C11 port of the OpenColorIO ACES2 fixed function
 * (src/OpenColorIO/ops/fixedfunction/ACES2/{Common.h,Transform.cpp} and
 * FixedFunctionOpCPU.cpp Renderer_ACES_OutputTransform20). Forward direction:
 *   RGB(AP0) -> CAM16 Aab -> JMh -> tonescale(J) -> chroma compress(M)
 *            -> gamut compress(M, hue-indexed cusp/reach tables) -> JMh
 *            -> Aab -> RGB(limiting primaries)
 * Tables (reach M, hue, cusp J/M/gamma) are built once per output variant.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

/* ---- constants (ACES2/Common.h) ----------------------------------------- */
#define A2_PI 3.14159265358979f
#define A2_HUE_LIMIT 360.0f
#define A2_REF_LUM 100.0f
#define A2_L_A 100.0f
#define A2_Y_B 20.0f
#define A2_J_SCALE 100.0f
#define A2_CAM_NL_OFFSET (0.2713f * 100.0f) /* 27.13 */
#define A2_CAM_NL_SCALE (4.0f * 100.0f)     /* 400 */
static const float A2_SURROUND[3] = {0.9f, 0.59f, 0.9f};

#define A2_CHROMA_COMPRESS 2.4f
#define A2_CHROMA_COMPRESS_FACT 3.3f
#define A2_CHROMA_EXPAND 1.3f
#define A2_CHROMA_EXPAND_FACT 0.69f
#define A2_CHROMA_EXPAND_THR 0.5f

#define A2_SMOOTH_CUSPS 0.12f
#define A2_SMOOTH_M 0.27f
#define A2_CUSP_MID_BLEND 1.3f
#define A2_FOCUS_GAIN_BLEND 0.3f
#define A2_FOCUS_DISTANCE 1.35f
#define A2_FOCUS_DISTANCE_SCALING 1.75f
#define A2_COMPRESSION_THRESHOLD 0.75f

#define A2_GAMMA_MIN 0.0f
#define A2_GAMMA_MAX 5.0f
#define A2_GAMMA_STEP 0.4f
#define A2_GAMMA_ACC 1e-5f
#define A2_CUSP_CORNERS 6
#define A2_TOTAL_CORNERS 8 /* cuspCorners + 2 */
#define A2_MAX_SORTED 12
#define A2_REACH_TOL 1e-3f
#define A2_DISPLAY_TOL 1e-7f

/* table index layout (TableBase) */
#define A2_NOMINAL TOC_ACES2_NOMINAL          /* 360 */
#define A2_BASE 1                              /* first_nominal_index */
#define A2_LOWER 0                             /* lower_wrap_index */
#define A2_UPPER (A2_BASE + A2_NOMINAL)        /* upper_wrap_index = 361 */
#define A2_LASTNOM (A2_UPPER - 1)              /* 360 */

static const float CAM16_PRIM[8] = {0.8336f, 0.1735f, 2.3854f, -1.4659f,
                                    0.087f,  -0.125f, 0.333f,  0.333f};
static const float AP0_PRIM[8] = {0.7347f,  0.2653f,  0.0f,     1.0f,
                                  0.0001f,  -0.077f,  0.32168f, 0.33767f};
static const float AP1_PRIM[8] = {0.713f,   0.293f,   0.165f,   0.830f,
                                  0.128f,   0.044f,   0.32168f, 0.33767f};

/* ---- small math: libm-free sin/cos/atan2/log helpers --------------------- */
static float a2_absf(float x) { return x < 0.0f ? -x : x; }
static float a2_minf(float a, float b) { return a < b ? a : b; }
static float a2_maxf(float a, float b) { return a > b ? a : b; }
static float a2_copysign(float x, float y) {
    float ax = x < 0.0f ? -x : x;
    return y < 0.0f ? -ax : ax;
}
static float a2_roundf(float x) {
    return x >= 0.0f ? (float)(int)(x + 0.5f) : (float)(int)(x - 0.5f);
}
static float a2_lnf(float x) { return toc_log2f(x) * 0.69314718055994531f; }
static float a2_log10f(float x) { return toc_log2f(x) * 0.30102999566398120f; }

/* sin reduced to [-pi/2,pi/2] then Taylor deg9 (~3e-6). */
static float a2_sinf(float x) {
    const float TWO_PI = 6.28318530717958648f, HALF_PI = 1.57079632679489662f;
    float k = x * (1.0f / TWO_PI), x2;
    k = k >= 0.0f ? (float)(int)(k + 0.5f) : (float)(int)(k - 0.5f);
    x -= k * TWO_PI;
    if (x > HALF_PI) x = A2_PI - x;
    else if (x < -HALF_PI) x = -A2_PI - x;
    x2 = x * x;
    return x * (1.0f + x2 * (-1.0f / 6.0f + x2 * (1.0f / 120.0f +
           x2 * (-1.0f / 5040.0f + x2 * (1.0f / 362880.0f)))));
}
static float a2_cosf(float x) { return a2_sinf(x + 1.57079632679489662f); }

/* atan on [-1,1] minimax (~1e-6). */
static float a2_atanf(float z) {
    float z2 = z * z;
    return z * (0.99997726f + z2 * (-0.33262347f + z2 * (0.19354346f +
           z2 * (-0.11643287f + z2 * (0.05265332f + z2 * (-0.01172120f))))));
}
static float a2_atan2f(float y, float x) {
    const float HALF_PI = 1.57079632679489662f;
    float ax = a2_absf(x), ay = a2_absf(y), r;
    if (ax == 0.0f && ay == 0.0f) return 0.0f;
    if (ax >= ay) r = a2_atanf(ay / ax);
    else r = HALF_PI - a2_atanf(ax / ay);
    if (x < 0.0f) r = A2_PI - r;
    if (y < 0.0f) r = -r;
    return r;
}
static float a2_lerpf(float a, float b, float t) { return (b - a) * t + a; }

/* ---- 3x3 row-major matrix helpers (M*v) --------------------------------- */
static void m3_mul(const float a[9], const float b[9], float o[9]) {
    int r, c, k;
    for (r = 0; r < 3; ++r)
        for (c = 0; c < 3; ++c) {
            float s = 0.0f;
            for (k = 0; k < 3; ++k) s += a[r * 3 + k] * b[k * 3 + c];
            o[r * 3 + c] = s;
        }
}
static void m3_vec(const float m[9], const float v[3], float o[3]) {
    int r;
    for (r = 0; r < 3; ++r)
        o[r] = m[r * 3 + 0] * v[0] + m[r * 3 + 1] * v[1] + m[r * 3 + 2] * v[2];
}
static int m3_inv(const float m[9], float o[9]) {
    float c00 = m[4] * m[8] - m[5] * m[7];
    float c01 = m[5] * m[6] - m[3] * m[8];
    float c02 = m[3] * m[7] - m[4] * m[6];
    float det = m[0] * c00 + m[1] * c01 + m[2] * c02;
    if (det == 0.0f) return 0;
    det = 1.0f / det;
    o[0] = c00 * det;
    o[1] = (m[2] * m[7] - m[1] * m[8]) * det;
    o[2] = (m[1] * m[5] - m[2] * m[4]) * det;
    o[3] = c01 * det;
    o[4] = (m[0] * m[8] - m[2] * m[6]) * det;
    o[5] = (m[2] * m[3] - m[0] * m[5]) * det;
    o[6] = c02 * det;
    o[7] = (m[1] * m[6] - m[0] * m[7]) * det;
    o[8] = (m[0] * m[4] - m[1] * m[3]) * det;
    return 1;
}
/* RGB->XYZ normalized primary matrix (row-major), white = the space's own. */
static void npm_from_prims(const float p[8], float M[9]) {
    float C[9], Cinv[9], W[3], S[3];
    int i;
    for (i = 0; i < 3; ++i) {
        float x = p[i * 2], y = p[i * 2 + 1];
        C[0 + i] = x / y;            /* column i = primary XYZ at Y=1 */
        C[3 + i] = 1.0f;
        C[6 + i] = (1.0f - x - y) / y;
    }
    W[0] = p[6] / p[7];
    W[1] = 1.0f;
    W[2] = (1.0f - p[6] - p[7]) / p[7];
    m3_inv(C, Cinv);
    m3_vec(Cinv, W, S);
    for (i = 0; i < 3; ++i) {
        M[0 + i] = C[0 + i] * S[i];
        M[3 + i] = C[3 + i] * S[i];
        M[6 + i] = C[6 + i] * S[i];
    }
}

/* ---- CAM core (Transform.cpp) ------------------------------------------- */
static float cone_fwd_abs(float Rc) { /* _post_adaptation..._fwd */
    float F = toc_powf(Rc, 0.42f);
    return F / (A2_CAM_NL_OFFSET + F);
}
static float cone_inv_abs(float Ra) {
    float Ra_lim = a2_minf(Ra, 0.99f);
    float F = (A2_CAM_NL_OFFSET * Ra_lim) / (1.0f - Ra_lim);
    return toc_powf(F, 1.0f / 0.42f);
}
static float cone_fwd(float v) { return a2_copysign(cone_fwd_abs(a2_absf(v)), v); }
static float cone_inv(float v) { return a2_copysign(cone_inv_abs(a2_absf(v)), v); }

static float achromatic_n_to_J(float A, float cz) {
    return A2_J_SCALE * toc_powf(A, cz);
}
static float J_to_achromatic_n(float J, float inv_cz) {
    return toc_powf(J * (1.0f / A2_J_SCALE), inv_cz);
}
static float a_to_Y(float A, const toc_jmh *p) {
    return cone_inv_abs(p->A_w_J * A) / p->F_L_n;
}
static float Y_to_J_(float absY, const toc_jmh *p) {
    float Ra = cone_fwd_abs(absY * p->F_L_n);
    return achromatic_n_to_J(Ra * p->inv_A_w_J, p->cz);
}
static float Y_to_J(float Y, const toc_jmh *p) {
    return a2_copysign(Y_to_J_(a2_absf(Y), p), Y);
}

static void rgb_to_aab(const float RGB[3], const toc_jmh *p, float Aab[3]) {
    float m[3], a[3];
    m3_vec(p->rgb_to_cam, RGB, m);
    a[0] = cone_fwd(m[0]);
    a[1] = cone_fwd(m[1]);
    a[2] = cone_fwd(m[2]);
    m3_vec(p->cone_to_aab, a, Aab);
}
static void aab_to_jmh(const float Aab[3], const toc_jmh *p, float JMh[3]) {
    if (Aab[0] <= 0.0f) { JMh[0] = JMh[1] = JMh[2] = 0.0f; return; }
    JMh[0] = achromatic_n_to_J(Aab[0], p->cz);
    JMh[1] = toc_sqrtf(Aab[1] * Aab[1] + Aab[2] * Aab[2]);
    {
        float h = 180.0f * a2_atan2f(Aab[2], Aab[1]) / A2_PI;
        if (h < 0.0f) h += A2_HUE_LIMIT;
        JMh[2] = h;
    }
}
static void jmh_to_aab(const float JMh[3], float cos_hr, float sin_hr,
                       const toc_jmh *p, float Aab[3]) {
    Aab[0] = J_to_achromatic_n(JMh[0], p->inv_cz);
    Aab[1] = JMh[1] * cos_hr;
    Aab[2] = JMh[1] * sin_hr;
}
static void aab_to_rgb(const float Aab[3], const toc_jmh *p, float RGB[3]) {
    float a[3], m[3];
    m3_vec(p->aab_to_cone, Aab, a);
    m[0] = cone_inv(a[0]);
    m[1] = cone_inv(a[1]);
    m[2] = cone_inv(a[2]);
    m3_vec(p->cam_to_rgb, m, RGB);
}
static void jmh_to_rgb(const float JMh[3], const toc_jmh *p, float RGB[3]) {
    float Aab[3];
    float h_rad = A2_PI * JMh[2] / 180.0f;
    jmh_to_aab(JMh, a2_cosf(h_rad), a2_sinf(h_rad), p, Aab);
    aab_to_rgb(Aab, p, RGB);
}

static float model_gamma(void) {
    return A2_SURROUND[1] * (1.48f + toc_sqrtf(A2_Y_B / A2_REF_LUM));
}

static void init_jmh(toc_jmh *p, const float prims[8]) {
    static const float base_cone_to_aab[9] = {
        2.0f, 1.0f, 1.0f / 20.0f,
        1.0f, -12.0f / 11.0f, 1.0f / 11.0f,
        1.0f / 9.0f, 1.0f / 9.0f, -2.0f / 9.0f};
    float npm_cam[9], npm_prim[9], M16[9]; /* M16 = XYZ->CAM16 RGB */
    float XYZ_w[3], RGB_w[3], D_RGB[3], RGB_WC[3], RGB_AW[3];
    float crta[9]; /* cone_response_to_Aab scaled by 400 */
    float rgb_to_cam[9], rgb_to_cam_c[9];
    float K, K4, F_L, Y_W, A_w;
    int i;

    npm_from_prims(CAM16_PRIM, npm_cam);
    m3_inv(npm_cam, M16);
    npm_from_prims(prims, npm_prim);
    { float ones[3] = {A2_REF_LUM, A2_REF_LUM, A2_REF_LUM};
      m3_vec(npm_prim, ones, XYZ_w); }
    Y_W = XYZ_w[1];
    m3_vec(M16, XYZ_w, RGB_w);

    K = 1.0f / (5.0f * A2_L_A + 1.0f);
    K4 = K * K * K * K;
    F_L = 0.2f * K4 * (5.0f * A2_L_A) +
          0.1f * (1.0f - K4) * (1.0f - K4) * toc_powf(5.0f * A2_L_A, 1.0f / 3.0f);
    p->F_L_n = F_L / A2_REF_LUM;
    p->cz = model_gamma();
    p->inv_cz = 1.0f / p->cz;

    for (i = 0; i < 3; ++i) {
        D_RGB[i] = p->F_L_n * Y_W / RGB_w[i];
        RGB_WC[i] = D_RGB[i] * RGB_w[i];
        RGB_AW[i] = cone_fwd(RGB_WC[i]);
    }
    for (i = 0; i < 9; ++i) crta[i] = base_cone_to_aab[i] * A2_CAM_NL_SCALE;
    A_w = crta[0] * RGB_AW[0] + crta[1] * RGB_AW[1] + crta[2] * RGB_AW[2];
    p->A_w_J = cone_fwd_abs(F_L);
    p->inv_A_w_J = 1.0f / p->A_w_J;

    /* MATRIX_RGB_to_CAM16 = (M16 * NPM(prim)) * 100 */
    m3_mul(M16, npm_prim, rgb_to_cam);
    for (i = 0; i < 9; ++i) rgb_to_cam[i] *= A2_REF_LUM;
    /* MATRIX_RGB_to_CAM16_c = diag(D_RGB) * rgb_to_cam (scale rows) */
    for (i = 0; i < 9; ++i) rgb_to_cam_c[i] = rgb_to_cam[i] * D_RGB[i / 3];
    for (i = 0; i < 9; ++i) p->rgb_to_cam[i] = rgb_to_cam_c[i];
    m3_inv(rgb_to_cam_c, p->cam_to_rgb);

    /* final cone_response_to_Aab: row0 /A_w, rows 1,2 *43*surround[2] */
    for (i = 0; i < 3; ++i) p->cone_to_aab[i] = crta[i] / A_w;
    for (i = 3; i < 9; ++i) p->cone_to_aab[i] = crta[i] * 43.0f * A2_SURROUND[2];
    m3_inv(p->cone_to_aab, p->aab_to_cone);
}

/* ---- tonescale ---------------------------------------------------------- */
static void init_tonescale(toc_aces2 *a, float peak) {
    float n = peak, n_r = 100.0f, g = 1.15f, c = 0.18f, c_d = 10.013f;
    float w_g = 0.14f, t_1 = 0.04f, r_hit_min = 128.0f, r_hit_max = 896.0f;
    float r_hit = r_hit_min + (r_hit_max - r_hit_min) *
                  (a2_lnf(n / n_r) / a2_lnf(10000.0f / 100.0f));
    float m_0 = n / n_r;
    float m_1 = 0.5f * (m_0 + toc_sqrtf(m_0 * (m_0 + 4.0f * t_1)));
    float u = toc_powf((r_hit / m_1) / ((r_hit / m_1) + 1.0f), g);
    float m = m_1 / u;
    float w_i = a2_lnf(n / 100.0f) / a2_lnf(2.0f);
    float c_t = c_d / n_r * (1.0f + w_i * w_g);
    float g_ip = 0.5f * (c_t + toc_sqrtf(c_t * (c_t + 4.0f * t_1)));
    float g_ipp2 = -(m_1 * toc_powf(g_ip / m, 1.0f / g)) /
                   (toc_powf(g_ip / m, 1.0f / g) - 1.0f);
    float w_2 = c / g_ipp2;
    float s_2 = w_2 * m_1 * A2_REF_LUM;
    float u_2 = toc_powf((r_hit / m_1) / ((r_hit / m_1) + w_2), g);
    float m_2 = m_1 / u_2;
    a->ts_n = n;
    a->ts_n_r = n_r;
    a->ts_g = g;
    a->ts_t_1 = t_1;
    a->ts_c_t = c_t;
    a->ts_s_2 = s_2;
    a->ts_u_2 = u_2;
    a->ts_m_2 = m_2;
    a->ts_inv_limit = n / (u_2 * n_r);
    a->ts_fwd_limit = 8.0f * r_hit;
    a->ts_log_peak = a2_log10f(n / n_r);
}
static float aces_tonescale_fwd(float Y_in, const toc_aces2 *a) {
    float f = a->ts_m_2 * toc_powf(Y_in / (Y_in + a->ts_s_2), a->ts_g);
    return a2_maxf(0.0f, f * f / (f + a->ts_t_1)) * a->ts_n_r;
}
static float tonescale_A_to_J_fwd(float A, const toc_jmh *p, const toc_aces2 *a) {
    float Y_in = a_to_Y(A, p);
    float Y_out = aces_tonescale_fwd(Y_in, a);
    return a2_copysign(Y_to_J_(a2_absf(Y_out), p), A);
}

/* ---- chroma compress ---------------------------------------------------- */
static float chroma_compress_norm(float cos1, float sin1, float scale) {
    float cos2 = 2.0f * cos1 * cos1 - 1.0f;
    float sin2 = 2.0f * cos1 * sin1;
    float cos3 = 4.0f * cos1 * cos1 * cos1 - 3.0f * cos1;
    float sin3 = 3.0f * sin1 - 4.0f * sin1 * sin1 * sin1;
    float M = 11.34072f * cos1 + 16.46899f * cos2 + 7.88380f * cos3 +
              14.66441f * sin1 + (-6.37224f) * sin2 + 9.19364f * sin3 + 77.12896f;
    return M * scale;
}
static float toe_fwd(float x, float limit, float k1_in, float k2_in) {
    float k2, k1, k3, mb, mac;
    if (x > limit) return x;
    k2 = a2_maxf(k2_in, 0.001f);
    k1 = toc_sqrtf(k1_in * k1_in + k2 * k2);
    k3 = (limit + k1) / (limit + k2);
    mb = k3 * x - k1;
    mac = k2 * k3 * x;
    return 0.5f * (mb + toc_sqrtf(mb * mb + 4.0f * mac));
}
static void chroma_compress_fwd(float JMh[3], float J_ts, float Mnorm,
                                float limit_J_max, float model_gamma_inv,
                                float reachMaxM, const toc_aces2 *a) {
    float J = JMh[0], M = JMh[1];
    float M_cp = M;
    if (M != 0.0f) {
        float nJ = J_ts / limit_J_max;
        float snJ = a2_maxf(0.0f, 1.0f - nJ);
        float limit = toc_powf(nJ, model_gamma_inv) * reachMaxM / Mnorm;
        M_cp = M * toc_powf(J_ts / J, model_gamma_inv);
        M_cp = M_cp / Mnorm;
        M_cp = limit - toe_fwd(limit - M_cp, limit - 0.001f, snJ * a->cc_sat,
                               toc_sqrtf(nJ * nJ + a->cc_sat_thr));
        M_cp = toe_fwd(M_cp, limit, nJ * a->cc_compr, snJ);
        M_cp = M_cp * Mnorm;
    }
    JMh[0] = J_ts;
    JMh[1] = M_cp;
}
static void init_chroma(toc_aces2 *a, float peak) {
    a->cc_compr = A2_CHROMA_COMPRESS +
                  (A2_CHROMA_COMPRESS * A2_CHROMA_COMPRESS_FACT) * a->ts_log_peak;
    a->cc_sat = a2_maxf(0.2f, A2_CHROMA_EXPAND -
                  (A2_CHROMA_EXPAND * A2_CHROMA_EXPAND_FACT) * a->ts_log_peak);
    a->cc_sat_thr = A2_CHROMA_EXPAND_THR / a->ts_n;
    a->cc_scale = toc_powf(0.03379f * peak, 0.30596f) - 0.45135f;
}

/* ---- reach M table ------------------------------------------------------ */
static int any_below_zero(const float rgb[3]) {
    return rgb[0] < 0.0f || rgb[1] < 0.0f || rgb[2] < 0.0f;
}
static void make_reach_m_table(float *reach, const toc_jmh *reachp, float limJ) {
    unsigned i;
    for (i = 0; i < A2_NOMINAL; ++i) {
        float hue = (float)i;
        float low = 0.0f, high = 50.0f;
        int outside = 0;
        float rgb[3], jmh[3];
        while (!outside && high < 1300.0f) {
            jmh[0] = limJ; jmh[1] = high; jmh[2] = hue;
            jmh_to_rgb(jmh, reachp, rgb);
            outside = any_below_zero(rgb);
            if (!outside) { low = high; high += 50.0f; }
        }
        while (high - low > 1e-2f) {
            float sm = (high + low) / 2.0f;
            jmh[0] = limJ; jmh[1] = sm; jmh[2] = hue;
            jmh_to_rgb(jmh, reachp, rgb);
            if (any_below_zero(rgb)) high = sm; else low = sm;
        }
        reach[i + A2_BASE] = high;
    }
    reach[A2_LOWER] = reach[A2_LASTNOM];
    reach[A2_UPPER] = reach[A2_BASE];
    reach[A2_UPPER + 1] = reach[A2_BASE + 1];
}
static float reach_m_from_table(float h, const float *rt) {
    unsigned base = (unsigned)h;
    float t = h - (float)base;
    unsigned lo = base + A2_BASE;
    return a2_lerpf(rt[lo], rt[lo + 1], t);
}

/* ---- gamut cusp / hue tables -------------------------------------------- */
static void unit_cube_corner(unsigned corner, float o[3]) {
    o[0] = (float)(((corner + 1) % A2_CUSP_CORNERS) < 3);
    o[1] = (float)(((corner + 5) % A2_CUSP_CORNERS) < 3);
    o[2] = (float)(((corner + 3) % A2_CUSP_CORNERS) < 3);
}
/* RGB_corners + JMh_corners for limiting gamut at peak luminance. */
static void build_limiting_corners(float RGBc[A2_TOTAL_CORNERS][3],
                                   float JMhc[A2_TOTAL_CORNERS][3],
                                   const toc_jmh *p, float peak) {
    float tRGB[A2_CUSP_CORNERS][3], tJMh[A2_CUSP_CORNERS][3];
    unsigned i, mn = 0;
    for (i = 0; i < A2_CUSP_CORNERS; ++i) {
        float v[3];
        unit_cube_corner(i, v);
        tRGB[i][0] = v[0] * peak / A2_REF_LUM;
        tRGB[i][1] = v[1] * peak / A2_REF_LUM;
        tRGB[i][2] = v[2] * peak / A2_REF_LUM;
        { float a[3]; rgb_to_aab(tRGB[i], p, a); aab_to_jmh(a, p, tJMh[i]); }
        if (tJMh[i][2] < tJMh[mn][2]) mn = i;
    }
    for (i = 0; i < A2_CUSP_CORNERS; ++i) {
        unsigned s = (i + mn) % A2_CUSP_CORNERS;
        int k;
        for (k = 0; k < 3; ++k) {
            RGBc[i + 1][k] = tRGB[s][k];
            JMhc[i + 1][k] = tJMh[s][k];
        }
    }
    { int k; for (k = 0; k < 3; ++k) {
        RGBc[0][k] = RGBc[A2_CUSP_CORNERS][k];
        RGBc[A2_CUSP_CORNERS + 1][k] = RGBc[1][k];
        JMhc[0][k] = JMhc[A2_CUSP_CORNERS][k];
        JMhc[A2_CUSP_CORNERS + 1][k] = JMhc[1][k];
    } }
    JMhc[0][2] -= A2_HUE_LIMIT;
    JMhc[A2_CUSP_CORNERS + 1][2] += A2_HUE_LIMIT;
}
static void build_reach_corners(float JMhc[A2_TOTAL_CORNERS][3],
                                const toc_jmh *p, float limJ, float maxsrc) {
    float tJMh[A2_CUSP_CORNERS][3];
    float limA = J_to_achromatic_n(limJ, p->inv_cz);
    unsigned i, mn = 0;
    for (i = 0; i < A2_CUSP_CORNERS; ++i) {
        float v[3];
        float lower = 0.0f, upper = maxsrc;
        unit_cube_corner(i, v);
        while ((upper - lower) > A2_REACH_TOL) {
            float test = (lower + upper) / 2.0f;
            float c[3], aab[3];
            c[0] = test * v[0]; c[1] = test * v[1]; c[2] = test * v[2];
            rgb_to_aab(c, p, aab);
            if (aab[0] < limA) lower = test;
            else upper = test;
            if (aab[0] == limA) break;
        }
        { float c[3], aab[3];
          c[0] = upper * v[0]; c[1] = upper * v[1]; c[2] = upper * v[2];
          rgb_to_aab(c, p, aab); aab_to_jmh(aab, p, tJMh[i]); }
        if (tJMh[i][2] < tJMh[mn][2]) mn = i;
    }
    for (i = 0; i < A2_CUSP_CORNERS; ++i) {
        unsigned s = (i + mn) % A2_CUSP_CORNERS;
        int k; for (k = 0; k < 3; ++k) JMhc[i + 1][k] = tJMh[s][k];
    }
    { int k; for (k = 0; k < 3; ++k) {
        JMhc[0][k] = JMhc[A2_CUSP_CORNERS][k];
        JMhc[A2_CUSP_CORNERS + 1][k] = JMhc[1][k];
    } }
    JMhc[0][2] -= A2_HUE_LIMIT;
    JMhc[A2_CUSP_CORNERS + 1][2] += A2_HUE_LIMIT;
}
static unsigned extract_sorted_hues(float *sorted, const float reach[A2_TOTAL_CORNERS][3],
                                    const float disp[A2_TOTAL_CORNERS][3]) {
    unsigned idx = 0, ri = 1, di = 1;
    while (ri < (A2_CUSP_CORNERS + 1) || di < (A2_CUSP_CORNERS + 1)) {
        float rh = reach[ri][2], dh = disp[di][2];
        if (rh == dh) { sorted[idx] = rh; ++ri; ++di; }
        else if (rh < dh) { sorted[idx] = rh; ++ri; }
        else { sorted[idx] = dh; ++di; }
        ++idx;
    }
    return idx;
}
static void hue_sample_interval(unsigned samples, float lower, float upper,
                                float *hue_table, unsigned base) {
    unsigned i;
    float delta = (upper - lower) / (float)samples;
    for (i = 0; i < samples; ++i) hue_table[base + i] = lower + (float)i * delta;
}
static void build_hue_table(float *hue_table, const float *sorted, unsigned uniq) {
    float ideal = (float)A2_NOMINAL / A2_HUE_LIMIT;
    unsigned samples_count[2 * A2_CUSP_CORNERS + 2];
    unsigned last_idx = 0xffffffffu;
    unsigned min_index = (sorted[0] == 0.0f) ? 0 : 1;
    unsigned hue_idx, total = 0, i;
    for (i = 0; i < 2 * A2_CUSP_CORNERS + 2; ++i) samples_count[i] = 0;
    for (hue_idx = 0; hue_idx < uniq; ++hue_idx) {
        unsigned ni = (unsigned)a2_roundf(sorted[hue_idx] * ideal);
        if (ni < min_index) ni = min_index;
        if (ni > A2_NOMINAL - 1) ni = A2_NOMINAL - 1;
        if (last_idx == ni) {
            if (hue_idx > 1 &&
                samples_count[hue_idx - 2] != (samples_count[hue_idx - 1] - 1))
                samples_count[hue_idx - 1] = samples_count[hue_idx - 1] - 1;
            else
                ni = ni + 1;
        }
        samples_count[hue_idx] = a2_minf(ni, A2_NOMINAL - 1U) == ni
                                     ? ni : (A2_NOMINAL - 1U);
        if (samples_count[hue_idx] > A2_NOMINAL - 1U)
            samples_count[hue_idx] = A2_NOMINAL - 1U;
        last_idx = min_index = ni;
    }
    i = 0;
    hue_sample_interval(samples_count[0], 0.0f, sorted[0], hue_table, total + 1);
    total += samples_count[0];
    for (i = 1; i < uniq; ++i) {
        unsigned samples = samples_count[i] - samples_count[i - 1];
        hue_sample_interval(samples, sorted[i - 1], sorted[i], hue_table, total + 1);
        total += samples;
    }
    hue_sample_interval(A2_NOMINAL - total, sorted[uniq - 1], A2_HUE_LIMIT,
                        hue_table, total + 1);
    hue_table[A2_LOWER] = hue_table[A2_LASTNOM] - A2_HUE_LIMIT;
    hue_table[A2_UPPER] = hue_table[A2_BASE] + A2_HUE_LIMIT;
    hue_table[A2_UPPER + 1] = hue_table[A2_BASE + 1] + A2_HUE_LIMIT;
}
static void find_display_cusp(float hue, const float RGBc[A2_TOTAL_CORNERS][3],
                              const float JMhc[A2_TOTAL_CORNERS][3],
                              const toc_jmh *p, float prev[2], float JM[2]) {
    unsigned upper = 1, lower, i;
    float lo_t, hi_t, st, cl[3], cu[3], s[3], jmh[3];
    for (i = 1; i < A2_TOTAL_CORNERS; ++i)
        if (JMhc[i][2] > hue) { upper = i; break; }
    lower = upper - 1;
    if (JMhc[lower][2] == hue) { JM[0] = JMhc[lower][0]; JM[1] = JMhc[lower][1]; return; }
    { int k; for (k = 0; k < 3; ++k) { cl[k] = RGBc[lower][k]; cu[k] = RGBc[upper][k]; } }
    lo_t = ((float)upper == prev[0]) ? prev[1] : 0.0f;
    hi_t = 1.0f;
    while ((hi_t - lo_t) > A2_DISPLAY_TOL) {
        int k; float aab[3];
        st = (lo_t + hi_t) / 2.0f;
        for (k = 0; k < 3; ++k) s[k] = a2_lerpf(cl[k], cu[k], st);
        rgb_to_aab(s, p, aab); aab_to_jmh(aab, p, jmh);
        if (jmh[2] < JMhc[lower][2]) hi_t = st;
        else if (jmh[2] >= JMhc[upper][2]) lo_t = st;
        else if (jmh[2] > hue) hi_t = st;
        else lo_t = st;
    }
    { int k; float aab[3];
      st = (lo_t + hi_t) / 2.0f;
      for (k = 0; k < 3; ++k) s[k] = a2_lerpf(cl[k], cu[k], st);
      rgb_to_aab(s, p, aab); aab_to_jmh(aab, p, jmh); }
    prev[0] = (float)upper; prev[1] = st;
    JM[0] = jmh[0]; JM[1] = jmh[1];
}
static void build_cusp_table(toc_aces2 *a, const float RGBc[A2_TOTAL_CORNERS][3],
                             const float JMhc[A2_TOTAL_CORNERS][3], const toc_jmh *p) {
    float prev[2] = {0.0f, 0.0f};
    unsigned i;
    for (i = A2_BASE; i < A2_UPPER; ++i) {
        float JM[2];
        find_display_cusp(a->hue_table[i], RGBc, JMhc, p, prev, JM);
        a->cusp_J[i] = JM[0];
        a->cusp_M[i] = JM[1] * (1.0f + A2_SMOOTH_M * A2_SMOOTH_CUSPS);
    }
    a->cusp_J[A2_LOWER] = a->cusp_J[A2_LASTNOM];
    a->cusp_M[A2_LOWER] = a->cusp_M[A2_LASTNOM];
    a->cusp_J[A2_UPPER] = a->cusp_J[A2_BASE];
    a->cusp_M[A2_UPPER] = a->cusp_M[A2_BASE];
    a->cusp_J[A2_UPPER + 1] = a->cusp_J[A2_BASE + 1];
    a->cusp_M[A2_UPPER + 1] = a->cusp_M[A2_BASE + 1];
}

/* ---- gamut compression per-pixel helpers ------------------------------- */
static float get_focus_gain(float J, float athr, float limJ, float fdist) {
    float gain = limJ * fdist;
    if (J > athr) {
        float adj = a2_log10f((limJ - athr) / a2_maxf(0.0001f, limJ - J));
        adj = adj * adj + 1.0f;
        gain = gain * adj;
    }
    return gain;
}
static float solve_J_intersect(float J, float M, float focusJ, float maxJ, float sg) {
    float Ms = M / sg, a = Ms / focusJ;
    if (J < focusJ) {
        float b = 1.0f - Ms, c = -J;
        float det = b * b - 4.0f * a * c;
        return -2.0f * c / (b + toc_sqrtf(det));
    } else {
        float b = -(1.0f + Ms + maxJ * a);
        float c = maxJ * Ms + J;
        float det = b * b - 4.0f * a * c;
        return -2.0f * c / (b - toc_sqrtf(det));
    }
}
static float smin_scaled(float a, float b, float sref) {
    float ss = A2_SMOOTH_CUSPS * sref;
    float h = a2_maxf(ss - a2_absf(a - b), 0.0f) / ss;
    return a2_minf(a, b) - h * h * h * ss * (1.0f / 6.0f);
}
static float comp_slope(float iJ, float focusJ, float maxJ, float sg) {
    float ds = (iJ < focusJ) ? iJ : (maxJ - iJ);
    return ds * (iJ - focusJ) / (focusJ * sg);
}
static float est_boundary_M(float Jai, float slope, float inv_gamma,
                            float Jmax, float Mmax, float Jref) {
    float nJ = Jai / Jref;
    float shifted = Jref * toc_powf(nJ, inv_gamma);
    return shifted * Mmax / (Jmax - slope * Mmax);
}
static float find_boundary(const float JMcusp[2], float Jmax, float gtop_inv,
                           float gbot_inv, float Jis, float slope, float Jic) {
    float Ml = est_boundary_M(Jis, slope, gbot_inv, JMcusp[0], JMcusp[1], Jic);
    float fJic = Jmax - Jic, fJis = Jmax - Jis, fJc = Jmax - JMcusp[0];
    float Mu = est_boundary_M(fJis, -slope, gtop_inv, fJc, JMcusp[1], fJic);
    return smin_scaled(Ml, Mu, JMcusp[1]);
}
static float reinhard_remap(float scale, float nd) {
    return scale * nd / (1.0f + nd);
}
static float remap_M(float M, float gbM, float rbM) {
    float ratio = gbM / rbM;
    float prop = a2_maxf(ratio, A2_COMPRESSION_THRESHOLD);
    float thr = prop * gbM;
    float moff, goff, roff, scale, nd;
    if (M <= thr || prop >= 1.0f) return M;
    moff = M - thr; goff = gbM - thr; roff = rbM - thr;
    scale = roff / ((roff / goff) - 1.0f);
    nd = moff / scale;
    return thr + reinhard_remap(scale, nd);
}
static unsigned lookup_hue_interval(float h, const float *hues, int slo, int shi) {
    unsigned i = A2_BASE + (unsigned)h;
    int ilo_i = (int)i + slo, ihi_i = (int)i + shi;
    unsigned ilo = ilo_i < A2_LOWER ? A2_LOWER : (unsigned)ilo_i;
    unsigned ihi = ihi_i > (int)A2_UPPER ? A2_UPPER : (unsigned)ihi_i;
    while (ilo + 1 < ihi) {
        if (h > hues[i]) ilo = i; else ihi = i;
        i = (ilo + ihi) / 2;
    }
    if (ihi < 1) ihi = 1;
    return ihi;
}
static void compress_gamut(const float JMh[3], float Jx, const toc_aces2 *a,
                           float reachMaxM, const float JMcusp[2], float focusJ,
                           float athr, float gtop_inv, float out[3]) {
    float J = JMh[0], M = JMh[1], h = JMh[2];
    float limJ = a->limit_J_max, fdist = a->focus_dist;
    float sg = get_focus_gain(Jx, athr, limJ, fdist);
    float Jis = solve_J_intersect(J, M, focusJ, limJ, sg);
    float slope = comp_slope(Jis, focusJ, limJ, sg);
    float Jic = solve_J_intersect(JMcusp[0], JMcusp[1], focusJ, limJ, sg);
    float gbM = find_boundary(JMcusp, limJ, gtop_inv, a->lower_hull_gamma_inv,
                              Jis, slope, Jic);
    float rbM, rM;
    if (gbM <= 0.0f) { out[0] = J; out[1] = 0.0f; out[2] = h; return; }
    rbM = est_boundary_M(Jis, slope, a->model_gamma_inv, limJ, reachMaxM, limJ);
    rM = remap_M(M, gbM, rbM);
    out[0] = Jis + rM * slope;
    out[1] = rM;
    out[2] = h;
}
static void gamut_compress_fwd(const float JMh[3], const toc_aces2 *a,
                               float reachMaxM, float out[3]) {
    float J = JMh[0], M = JMh[1], h = JMh[2];
    unsigned i_hi;
    float t, JMcusp[2], gtop_inv, focusJ, athr;
    if (J <= 0.0f) { out[0] = 0.0f; out[1] = 0.0f; out[2] = h; return; }
    if (M <= 0.0f || J > a->limit_J_max) { out[0] = J; out[1] = 0.0f; out[2] = h; return; }
    i_hi = lookup_hue_interval(h, a->hue_table, a->hue_search_lo, a->hue_search_hi);
    t = (h - a->hue_table[i_hi - 1]) / (a->hue_table[i_hi] - a->hue_table[i_hi - 1]);
    JMcusp[0] = a2_lerpf(a->cusp_J[i_hi - 1], a->cusp_J[i_hi], t);
    JMcusp[1] = a2_lerpf(a->cusp_M[i_hi - 1], a->cusp_M[i_hi], t);
    gtop_inv = a2_lerpf(a->cusp_g[i_hi - 1], a->cusp_g[i_hi], t);
    focusJ = a2_lerpf(JMcusp[0], a->mid_J,
                      a2_minf(1.0f, A2_CUSP_MID_BLEND - (JMcusp[0] / a->limit_J_max)));
    athr = a2_lerpf(JMcusp[0], a->limit_J_max, A2_FOCUS_GAIN_BLEND);
    compress_gamut(JMh, JMh[0], a, reachMaxM, JMcusp, focusJ, athr, gtop_inv, out);
}

/* ---- upper hull gamma (per-hue cusp_g) --------------------------------- */
static int outside_hull(const float rgb[3], float maxv) {
    return rgb[0] > maxv || rgb[1] > maxv || rgb[2] > maxv;
}
static int eval_gamma_fit(const float JMcusp[2], const float *testJ,
                          const float *Jis, const float *slope, const float *Jic,
                          float hue, float topgamma_inv, float peak, float limJ,
                          float lower_hull_gamma_inv, const toc_jmh *limp) {
    float lim = peak / A2_REF_LUM;
    int t;
    (void)testJ;
    for (t = 0; t < 5; ++t) {
        float aM = find_boundary(JMcusp, limJ, topgamma_inv, lower_hull_gamma_inv,
                                 Jis[t], slope[t], Jic[t]);
        float aJ = Jis[t] + slope[t] * aM;
        float jmh[3] = {aJ, aM, hue}, rgb[3];
        jmh_to_rgb(jmh, limp, rgb);
        if (!outside_hull(rgb, lim)) return 0;
    }
    return 1;
}
static void make_upper_hull_gamma(toc_aces2 *a, const toc_jmh *limp, float peak) {
    static const float pos[5] = {0.01f, 0.1f, 0.5f, 0.8f, 0.99f};
    unsigned i;
    for (i = A2_BASE; i < A2_UPPER; ++i) {
        float hue = a->hue_table[i];
        float JMcusp[2] = {a->cusp_J[i], a->cusp_M[i]};
        float athr = a2_lerpf(JMcusp[0], a->limit_J_max, A2_FOCUS_GAIN_BLEND);
        float focusJ = a2_lerpf(JMcusp[0], a->mid_J,
                       a2_minf(1.0f, A2_CUSP_MID_BLEND - (JMcusp[0] / a->limit_J_max)));
        float tJis[5], tslope[5], tJic[5];
        float low, high, tg;
        int outside = 0, t;
        for (t = 0; t < 5; ++t) {
            float testJ = a2_lerpf(JMcusp[0], a->limit_J_max, pos[t]);
            float sg = get_focus_gain(testJ, athr, a->limit_J_max, a->focus_dist);
            tJis[t] = solve_J_intersect(testJ, JMcusp[1], focusJ, a->limit_J_max, sg);
            tslope[t] = comp_slope(tJis[t], focusJ, a->limit_J_max, sg);
            tJic[t] = solve_J_intersect(JMcusp[0], JMcusp[1], focusJ, a->limit_J_max, sg);
        }
        low = A2_GAMMA_MIN;
        high = low + A2_GAMMA_STEP;
        while (!outside && high < A2_GAMMA_MAX) {
            if (eval_gamma_fit(JMcusp, NULL, tJis, tslope, tJic, hue, 1.0f / high,
                               peak, a->limit_J_max, a->lower_hull_gamma_inv, limp))
                outside = 1;
            else { low = high; high += A2_GAMMA_STEP; }
        }
        while ((high - low) > A2_GAMMA_ACC) {
            tg = (high + low) / 2.0f;
            if (eval_gamma_fit(JMcusp, NULL, tJis, tslope, tJic, hue, 1.0f / tg,
                               peak, a->limit_J_max, a->lower_hull_gamma_inv, limp))
                high = tg;
            else low = tg;
        }
        a->cusp_g[i] = 1.0f / high;
    }
    a->cusp_g[A2_LOWER] = a->cusp_g[A2_LASTNOM];
    a->cusp_g[A2_UPPER] = a->cusp_g[A2_BASE];
    a->cusp_g[A2_UPPER + 1] = a->cusp_g[A2_BASE + 1];
}
static void determine_search_range(toc_aces2 *a) {
    unsigned i;
    int lo = 0, hi = 1;
    for (i = A2_BASE; i < A2_UPPER; ++i) {
        unsigned posu = A2_BASE + (unsigned)a->hue_table[i];
        int delta = (int)i - (int)posu;
        if (delta + 0 < lo) lo = delta + 0;
        if (delta + 1 > hi) hi = delta + 1;
    }
    a->hue_search_lo = lo;
    a->hue_search_hi = hi;
}

/* ---- public init + apply ------------------------------------------------ */
toc_aces2 *toc_aces2_init(const toc_allocator *a, float peak, const float lim[8]) {
    toc_aces2 *t;
    toc_jmh reach;
    float RGBc[A2_TOTAL_CORNERS][3], JMhc[A2_TOTAL_CORNERS][3];
    float reachJMh[A2_TOTAL_CORNERS][3];
    float sorted[A2_MAX_SORTED];
    unsigned uniq;
    if (!a) a = toc_default_allocator();
    t = (toc_aces2 *)toc_malloc(a, sizeof(*t));
    if (!t) return NULL;
    memset(t, 0, sizeof(*t));
    init_jmh(&t->in, AP0_PRIM);
    init_jmh(&t->out, lim);
    init_jmh(&reach, AP1_PRIM);
    init_tonescale(t, peak);
    init_chroma(t, peak);
    t->limit_J_max = Y_to_J(peak, &t->in);
    t->model_gamma_inv = 1.0f / model_gamma();
    make_reach_m_table(t->reach_m, &reach, t->limit_J_max);
    t->mid_J = Y_to_J(t->ts_c_t * A2_REF_LUM, &t->in);
    t->focus_dist = A2_FOCUS_DISTANCE +
                    A2_FOCUS_DISTANCE * A2_FOCUS_DISTANCE_SCALING * t->ts_log_peak;
    t->lower_hull_gamma_inv = 1.0f / (1.14f + 0.07f * t->ts_log_peak);
    /* gamut cusp tables: hue table from merged reach+limiting corners */
    build_reach_corners(reachJMh, &reach, t->limit_J_max, t->ts_fwd_limit);
    build_limiting_corners(RGBc, JMhc, &t->out, peak);
    uniq = extract_sorted_hues(sorted, reachJMh, JMhc);
    build_hue_table(t->hue_table, sorted, uniq);
    build_cusp_table(t, RGBc, JMhc, &t->out);
    determine_search_range(t);
    make_upper_hull_gamma(t, &t->out, peak);
    return t;
}

void toc_aces2_apply_pixel(const toc_op *op, float *px, int ch) {
    const toc_aces2 *a = (const toc_aces2 *)op->u.aces.t;
    float RGB[3], Aab[3], JMh[3], tone[3], comp[3], Aabout[3], out[3];
    float reachMaxM, h_rad, cos1, sin1, Mnorm, J_ts;
    (void)ch;
    if (!a) return;
    RGB[0] = px[0]; RGB[1] = px[1]; RGB[2] = px[2];
    rgb_to_aab(RGB, &a->in, Aab);
    aab_to_jmh(Aab, &a->in, JMh);
    reachMaxM = reach_m_from_table(JMh[2], a->reach_m);
    h_rad = A2_PI * JMh[2] / 180.0f;
    cos1 = a2_cosf(h_rad);
    sin1 = a2_sinf(h_rad);
    Mnorm = chroma_compress_norm(cos1, sin1, a->cc_scale);
    J_ts = tonescale_A_to_J_fwd(Aab[0], &a->in, a);
    tone[0] = JMh[0]; tone[1] = JMh[1]; tone[2] = JMh[2];
    chroma_compress_fwd(tone, J_ts, Mnorm, a->limit_J_max, a->model_gamma_inv,
                        reachMaxM, a);
    gamut_compress_fwd(tone, a, reachMaxM, comp);
    jmh_to_aab(comp, cos1, sin1, &a->out, Aabout);
    aab_to_rgb(Aabout, &a->out, out);
    px[0] = out[0]; px[1] = out[1]; px[2] = out[2];
}
