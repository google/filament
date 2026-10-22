/*
 * tocio - ACES BuiltinTransform expansion + FixedFunctionTransform lowering.
 *
 * All OCIO FixedFunction styles are implemented: ACES glow/red-mod/dark-to-dim/
 * gamut-comp + Rec.2100 surround + RGB/HSV + XYZ/xyY/uvY/LUV conversions.
 * Unrecognized styles return TOC_ERROR_UNSUPPORTED (loud, never silent).
 *
 * Constants and curves reimplemented from OpenColorIO / ACES (BSD-3-Clause).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

/* Freestanding-friendly approx_atan2f (the RedMod hue weight doesn't need
 * high precision; the B-spline kernel is approximate anyway). */
static float ff_atan2f(float y, float x) {
    float ax = x < 0.0f ? -x : x, ay = y < 0.0f ? -y : y;
    float a, r, a2;
    int q = 0;
    if (ax + ay == 0.0f) return 0.0f;
    a = (ay < ax) ? y / x : x / y;
    if (ay < ax) q = 0; else q = 2;
    if (x < 0.0f && ay < ax) q = 1;
    if (y < 0.0f && ay >= ax) q = 3;
    a2 = a * a;
    r = a * (1.0f - a2 * (1.0f / 3.0f - a2 * (1.0f / 5.0f - a2 *
           (1.0f / 7.0f - a2 * (1.0f / 9.0f)))));
    switch (q) {
        case 1: r = (r < 0.0f ? -3.141592653589793f : 3.141592653589793f) + r; break;
        case 2: r = 1.5707963267948966f - r; break;
        case 3: r = -1.5707963267948966f - r; break;
    }
    return r;
}

static inline float ff_copysignf(float x, float y) {
    uint32_t ux, uy;
    memcpy(&ux, &x, sizeof(ux));
    memcpy(&uy, &y, sizeof(uy));
    ux = (ux & 0x7fffffffu) | (uy & 0x80000000u);
    memcpy(&x, &ux, sizeof(x));
    return x;
}

static inline float ff_fabsf(float x) {
    uint32_t u;
    memcpy(&u, &x, sizeof(u));
    u &= 0x7fffffffu;
    memcpy(&x, &u, sizeof(x));
    return x;
}

static inline float ff_floorf(float x) {
    float r = (float)(int)x;
    return (r > x) ? r - 1.0f : r;
}

static inline float ff_fmaxf(float a, float b) { return a > b ? a : b; }
static inline float ff_fminf(float a, float b) { return a < b ? a : b; }
static inline float ff_clampf(float x, float lo, float hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}

/* ---- Bump-arena helpers for the node interface --------------------------- */
static float parse_param(const toc_node *params, int idx, float def) {
    if (params && params->kind == TOC_NODE_SEQ &&
        params->n_items > (size_t)idx) {
        const char *s = toc_node_scalar(params->items[idx]);
        const char *p = s;
        float v;
        if (s && toc_parse_float(&p, s + strlen(s), &v)) return v;
    }
    return def;
}

/* ---- ACES AP matrices + builtin transform expansion (unchanged) ---------- */
static const float AP1_TO_AP0[9] = {
    0.6954522414f, 0.1406786965f, 0.1638690622f,
    0.0447945634f, 0.8596711185f, 0.0955343182f,
    -0.0055258826f, 0.0040252103f, 1.0015006723f};
static const float AP0_TO_AP1[9] = {
    1.4514393161f, -0.2365107469f, -0.2149285693f,
    -0.0765537734f, 1.1762296998f, -0.0996759264f,
    0.0083161484f, -0.0060324498f, 0.9977163014f};

static toc_op *push_mat3(toc_op_list *list, const float rm3[9]) {
    toc_op *op = toc_op_list_push(list, TOC_OP_MATRIX);
    int r, c;
    if (!op) return NULL;
    for (r = 0; r < 4; ++r)
        for (c = 0; c < 4; ++c)
            op->u.matrix.m[c * 4 + r] =
                (r < 3 && c < 3) ? rm3[r * 3 + c] : (r == c ? 1.0f : 0.0f);
    return op;
}

static toc_op *push_acescct_log(toc_op_list *list, int log_to_lin) {
    toc_op *op = toc_op_list_push(list, TOC_OP_LOG_CAMERA);
    int i;
    if (!op) return NULL;
    op->u.logcam.base = 2.0f;
    for (i = 0; i < 3; ++i) {
        op->u.logcam.log_slope[i] = 1.0f / 17.52f;
        op->u.logcam.log_offset[i] = 9.72f / 17.52f;
        op->u.logcam.lin_slope[i] = 1.0f;
        op->u.logcam.lin_offset[i] = 0.0f;
        op->u.logcam.lin_break[i] = 0.0078125f;
        op->u.logcam.linear_slope[i] = 10.5402377416545f;
        op->u.logcam.linear_offset[i] = 0.0729055341958355f;
    }
    op->u.logcam.inverse = log_to_lin ? 1 : 0;
    return op;
}

/* ===========================================================================
 * Colorimetry: derive linear RGB<->RGB / RGB<->CIE-XYZ matrices from primaries
 * + white point, with Bradford chromatic adaptation. All pure float arithmetic
 * (freestanding-safe). Lets the builtin expander synthesize conversions between
 * sRGB/Rec.709, Display-P3, DCI-P3, Rec.2020/2100, Adobe RGB, ACEScg (AP1),
 * ACES2065-1 (AP0) and CIE-XYZ-D65 without baking every matrix by hand.
 * ========================================================================= */

/* row-major 3x3 */
static void mat3_mul(const float a[9], const float b[9], float o[9]) {
    int r, c, k;
    for (r = 0; r < 3; ++r)
        for (c = 0; c < 3; ++c) {
            float s = 0.0f;
            for (k = 0; k < 3; ++k) s += a[r * 3 + k] * b[k * 3 + c];
            o[r * 3 + c] = s;
        }
}
static void mat3_vec(const float m[9], const float v[3], float o[3]) {
    int r;
    for (r = 0; r < 3; ++r)
        o[r] = m[r * 3 + 0] * v[0] + m[r * 3 + 1] * v[1] + m[r * 3 + 2] * v[2];
}
static int mat3_inv(const float m[9], float o[9]) {
    float det;
    float c00 = m[4] * m[8] - m[5] * m[7];
    float c01 = m[5] * m[6] - m[3] * m[8];
    float c02 = m[3] * m[7] - m[4] * m[6];
    det = m[0] * c00 + m[1] * c01 + m[2] * c02;
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

/* xy -> XYZ at unit luminance (Y=1). */
static void xy_to_xyz(float x, float y, float o[3]) {
    o[0] = x / y;
    o[1] = 1.0f;
    o[2] = (1.0f - x - y) / y;
}

/* A linear RGB encoding: R/G/B chromaticities + white (xy). is_xyz marks the
 * CIE-XYZ-D65 pseudo-space (identity NPM, D65 white). */
typedef struct {
    float rx, ry, gx, gy, bx, by, wx, wy;
    int is_xyz;
} toc_cspace;

/* RGB->XYZ normalized primary matrix (NPM) for `cs`. */
static int cspace_npm(const toc_cspace *cs, float M[9]) {
    float C[9], Cinv[9], W[3], S[3], pr[3], pg[3], pb[3];
    if (cs->is_xyz) {
        int i;
        for (i = 0; i < 9; ++i) M[i] = (i % 4 == 0) ? 1.0f : 0.0f;
        return 1;
    }
    xy_to_xyz(cs->rx, cs->ry, pr);
    xy_to_xyz(cs->gx, cs->gy, pg);
    xy_to_xyz(cs->bx, cs->by, pb);
    C[0] = pr[0]; C[1] = pg[0]; C[2] = pb[0]; /* columns = primaries */
    C[3] = pr[1]; C[4] = pg[1]; C[5] = pb[1];
    C[6] = pr[2]; C[7] = pg[2]; C[8] = pb[2];
    xy_to_xyz(cs->wx, cs->wy, W);
    if (!mat3_inv(C, Cinv)) return 0;
    mat3_vec(Cinv, W, S); /* per-primary scale so RGB(1,1,1) -> white */
    M[0] = C[0] * S[0]; M[1] = C[1] * S[1]; M[2] = C[2] * S[2];
    M[3] = C[3] * S[0]; M[4] = C[4] * S[1]; M[5] = C[5] * S[2];
    M[6] = C[6] * S[0]; M[7] = C[7] * S[1]; M[8] = C[8] * S[2];
    return 1;
}

/* Bradford chromatic adaptation from white ws(xy) to wd(xy). */
static void bradford_cat(float wsx, float wsy, float wdx, float wdy, float M[9]) {
    static const float B[9] = {0.8951f,  0.2664f,  -0.1614f,
                               -0.7502f, 1.7135f,  0.0367f,
                               0.0389f,  -0.0685f, 1.0296f};
    float Binv[9], Ws[3], Wd[3], cs[3], cd[3], D[9], t[9];
    int i;
    mat3_inv(B, Binv);
    xy_to_xyz(wsx, wsy, Ws);
    xy_to_xyz(wdx, wdy, Wd);
    mat3_vec(B, Ws, cs);
    mat3_vec(B, Wd, cd);
    for (i = 0; i < 9; ++i) D[i] = 0.0f;
    D[0] = cd[0] / cs[0];
    D[4] = cd[1] / cs[1];
    D[8] = cd[2] / cs[2];
    mat3_mul(D, B, t);
    mat3_mul(Binv, t, M); /* Binv * D * B */
}

/* M (row-major) = linear src -> linear dst, Bradford-adapting the white. */
static int cspace_convert(const toc_cspace *s, const toc_cspace *d, float M[9]) {
    float Ms[9], Md[9], Mdinv[9];
    int same_w = (s->wx == d->wx && s->wy == d->wy);
    if (!cspace_npm(s, Ms) || !cspace_npm(d, Md) || !mat3_inv(Md, Mdinv))
        return 0;
    if (same_w) {
        mat3_mul(Mdinv, Ms, M);
    } else {
        float cat[9], a[9];
        bradford_cat(s->wx, s->wy, d->wx, d->wy, cat);
        mat3_mul(cat, Ms, a);  /* adapt src XYZ to dst white */
        mat3_mul(Mdinv, a, M);
    }
    return 1;
}

/* White points (xy). */
#define TOC_WP_D65 0.3127f, 0.3290f
#define TOC_WP_D60 0.32168f, 0.33767f /* ACES */
#define TOC_WP_DCI 0.314f, 0.351f

/* Named linear color spaces. Aliases share an entry. */
static const struct { const char *name; toc_cspace cs; } TOC_CSPACES[] = {
    {"Linear-sRGB",   {0.640f, 0.330f, 0.300f, 0.600f, 0.150f, 0.060f, TOC_WP_D65, 0}},
    {"Linear-Rec709", {0.640f, 0.330f, 0.300f, 0.600f, 0.150f, 0.060f, TOC_WP_D65, 0}},
    {"Linear-P3-D65", {0.680f, 0.320f, 0.265f, 0.690f, 0.150f, 0.060f, TOC_WP_D65, 0}},
    {"Linear-Display-P3", {0.680f, 0.320f, 0.265f, 0.690f, 0.150f, 0.060f, TOC_WP_D65, 0}},
    {"Linear-P3-DCI", {0.680f, 0.320f, 0.265f, 0.690f, 0.150f, 0.060f, TOC_WP_DCI, 0}},
    {"Linear-DCI-P3", {0.680f, 0.320f, 0.265f, 0.690f, 0.150f, 0.060f, TOC_WP_DCI, 0}},
    {"Linear-Rec2020", {0.708f, 0.292f, 0.170f, 0.797f, 0.131f, 0.046f, TOC_WP_D65, 0}},
    {"Linear-Rec2100", {0.708f, 0.292f, 0.170f, 0.797f, 0.131f, 0.046f, TOC_WP_D65, 0}},
    {"Linear-AdobeRGB", {0.640f, 0.330f, 0.210f, 0.710f, 0.150f, 0.060f, TOC_WP_D65, 0}},
    {"ACEScg",      {0.713f, 0.293f, 0.165f, 0.830f, 0.128f, 0.044f, TOC_WP_D60, 0}},
    {"Linear-AP1",  {0.713f, 0.293f, 0.165f, 0.830f, 0.128f, 0.044f, TOC_WP_D60, 0}},
    {"ACES2065-1",  {0.7347f, 0.2653f, 0.0f, 1.0f, 0.0001f, -0.077f, TOC_WP_D60, 0}},
    {"Linear-AP0",  {0.7347f, 0.2653f, 0.0f, 1.0f, 0.0001f, -0.077f, TOC_WP_D60, 0}},
    {"CIE-XYZ-D65", {0, 0, 0, 0, 0, 0, TOC_WP_D65, 1}},
};

static const toc_cspace *cspace_lookup(const char *name, size_t len) {
    size_t i;
    for (i = 0; i < sizeof(TOC_CSPACES) / sizeof(TOC_CSPACES[0]); ++i)
        if (strlen(TOC_CSPACES[i].name) == len &&
            memcmp(TOC_CSPACES[i].name, name, len) == 0)
            return &TOC_CSPACES[i].cs;
    return NULL;
}

/* von Kries chromatic adaptation ws(xy)->wd(xy) with cone-response matrix C. */
static void vk_cat(const float C[9], float wsx, float wsy, float wdx, float wdy,
                   float M[9]) {
    float Cinv[9], Ws[3], Wd[3], cs[3], cd[3], D[9], t[9];
    int i;
    mat3_inv(C, Cinv);
    xy_to_xyz(wsx, wsy, Ws);
    xy_to_xyz(wdx, wdy, Wd);
    mat3_vec(C, Ws, cs);
    mat3_vec(C, Wd, cd);
    for (i = 0; i < 9; ++i) D[i] = 0.0f;
    D[0] = cd[0] / cs[0];
    D[4] = cd[1] / cs[1];
    D[8] = cd[2] / cs[2];
    mat3_mul(D, C, t);
    mat3_mul(Cinv, t, M); /* Cinv * D * C */
}

/* Camera-gamut RGB -> ACES AP0 matrix (row-major), adapting the camera white to
 * AP0's D60 with CAT02 (cat02=1) or Bradford (cat02=0), matching OCIO's
 * build_conversion_matrix(camera, AP0, ADAPTATION_*). */
static int cam_to_ap0(const float cam[8], int cat02, float M[9]) {
    static const float BRAD[9] = {0.8951f,  0.2664f,  -0.1614f,
                                  -0.7502f, 1.7135f,  0.0367f,
                                  0.0389f,  -0.0685f, 1.0296f};
    static const float CAT02[9] = {0.7328f,  0.4296f, -0.1624f,
                                   -0.7036f, 1.6975f, 0.0061f,
                                   0.0030f,  0.0136f, 0.9834f};
    toc_cspace c;
    const toc_cspace *ap0 = cspace_lookup("Linear-AP0", 10);
    float Mc[9], Map0[9], Mai[9], cat[9], a[9];
    if (!ap0) return 0;
    c.rx = cam[0]; c.ry = cam[1]; c.gx = cam[2]; c.gy = cam[3];
    c.bx = cam[4]; c.by = cam[5]; c.wx = cam[6]; c.wy = cam[7];
    c.is_xyz = 0;
    if (!cspace_npm(&c, Mc) || !cspace_npm(ap0, Map0) || !mat3_inv(Map0, Mai))
        return 0;
    vk_cat(cat02 ? CAT02 : BRAD, c.wx, c.wy, ap0->wx, ap0->wy, cat);
    mat3_mul(cat, Mc, a);  /* adapt camera XYZ to AP0 white */
    mat3_mul(Mai, a, M);   /* XYZ(AP0 white) -> AP0 RGB */
    return 1;
}

/* Try "<A>_to_<B>" as a linear color-space conversion; push a matrix op. Returns
 * 1 if handled (matrix pushed or OOM via *rc), 0 if the names are unknown. */
static const char *find_to(const char *s) { /* locate "_to_" (no libc strstr) */
    for (; *s; ++s)
        if (s[0] == '_' && s[1] == 't' && s[2] == 'o' && s[3] == '_') return s;
    return NULL;
}
/* Push the linear src->dst conversion matrix (names from the cspace table). */
static int push_cspace_named(toc_op_list *list, const char *src, const char *dst,
                             toc_result *rc) {
    const toc_cspace *s = cspace_lookup(src, strlen(src));
    const toc_cspace *d = cspace_lookup(dst, strlen(dst));
    float M[9];
    if (!s || !d || !cspace_convert(s, d, M)) { *rc = TOC_ERROR_UNSUPPORTED; return 0; }
    if (!push_mat3(list, M)) *rc = TOC_ERROR_OUT_OF_MEMORY;
    return TOC_OK(*rc);
}
static int push_cspace_convert(toc_op_list *list, const char *style,
                               toc_result *rc) {
    const char *sep = find_to(style);
    char src[48];
    size_t n;
    if (!sep) return 0;
    n = (size_t)(sep - style);
    if (n >= sizeof(src)) return 0;
    if (!cspace_lookup(style, n) || !cspace_lookup(sep + 4, strlen(sep + 4)))
        return 0;
    memcpy(src, style, n);
    src[n] = '\0';
    push_cspace_named(list, src, sep + 4, rc);
    return 1;
}

/* ---- display transfer functions (linear <-> display-encoded) ------------- */
/* OCIO MonCurve (GammaOpData MONCURVE) from (gamma, offset): forward op is the
 * EOTF (encoded->linear); encode=1 emits the inverse (linear->encoded). Matches
 * sRGB / Display-P3 display encodings (gamma 2.4, offset 0.055). */
static toc_op *push_moncurve(toc_op_list *list, float gamma, float offset,
                             int encode, int mirror) {
    toc_op *op = toc_op_list_push(list, TOC_OP_EXP_LINEAR);
    double G = gamma < 1.000001f ? 1.000001 : (double)gamma;
    double O = offset < 1e-6f ? 1e-6 : (double)offset;
    double a = (G - 1.0) / O;
    double b = O * G / ((G - 1.0) * (1.0 + O));
    float scale = (float)(1.0 / (1.0 + O));
    float off = (float)(O / (1.0 + O));
    float brk = (float)(O / (G - 1.0));
    float slope = (float)(a * toc_powf((float)b, (float)G));
    int i;
    if (!op) return NULL;
    for (i = 0; i < 4; ++i) {
        op->u.exp_linear.scale[i] = scale;
        op->u.exp_linear.offset[i] = off;
        op->u.exp_linear.gamma[i] = (float)G;
        op->u.exp_linear.breakpoint[i] = brk;
        op->u.exp_linear.slope[i] = slope;
    }
    op->u.exp_linear.inverse = encode ? 1 : 0;
    op->u.exp_linear.mirror = mirror;
    return op;
}
/* Pure-power EOTF (gamma): forward op = display->linear (pow(x,g)); encode emits
 * linear->display (pow(x,1/g)). Alpha unchanged. */
static toc_op *push_gamma(toc_op_list *list, float g, int encode, int mirror) {
    toc_op *op = toc_op_list_push(list, TOC_OP_EXPONENT);
    int i;
    float e = encode ? 1.0f / g : g;
    if (!op) return NULL;
    for (i = 0; i < 4; ++i) op->u.exponent.e[i] = (i < 3) ? e : 1.0f;
    op->u.exponent.mirror = mirror;
    return op;
}
static toc_op *push_ff_style(toc_op_list *list, int style) {
    toc_op *op = toc_op_list_push(list, TOC_OP_FIXEDFUNC);
    if (!op) return NULL;
    op->u.fixedfunc.style = style;
    op->u.fixedfunc.nparams = 0;
    return op;
}

/* True if `s` ends with `suf`. */
static int ends_with(const char *s, const char *suf) {
    size_t ls = strlen(s), lf = strlen(suf);
    return ls >= lf && memcmp(s + ls - lf, suf, lf) == 0;
}

static toc_op *push_scale3(toc_op_list *list, float s); /* defined below */

/* Try a composed display output transform. Accepts the OCIO v2 builtin names
 * ("DISPLAY - CIE-XYZ-D65_to_<display>[ - MIRROR NEGS]") and the bare
 * "CIE-XYZ-D65_to_<display>" form: a primaries matrix then the display transfer
 * function encode (MonCurve / pure gamma / PQ / HLG). Returns 1 if handled.
 * kind: 0=MonCurve(g,off), 1=pure gamma(g), 2=PQ, 3=HLG. */
static int push_display_xform(toc_op_list *list, const char *style,
                              toc_result *rc) {
    /* amir=1: OCIO encodes this display with MONCURVE_MIRROR even without the
     * "- MIRROR NEGS" suffix (Display-P3). kind 4 = scale(48/52.37)+gamma 2.6 over
     * XYZ (DCDM); kind 5 = PQ over XYZ (ST2084-DCDM); both keep XYZ (identity). */
    static const struct {
        const char *name, *prim; int kind; float g, off; int amir;
    } d[] = {
        {"CIE-XYZ-D65_to_sRGB",              "Linear-sRGB",       0, 2.4f, 0.055f, 0},
        {"CIE-XYZ-D65_to_DisplayP3",         "Linear-Display-P3", 0, 2.4f, 0.055f, 1},
        {"CIE-XYZ-D65_to_Display-P3",        "Linear-Display-P3", 0, 2.4f, 0.055f, 1},
        {"CIE-XYZ-D65_to_DisplayP3-HDR",     "Linear-Display-P3", 0, 2.4f, 0.055f, 1},
        {"CIE-XYZ-D65_to_G2.2-REC.709",      "Linear-Rec709",     1, 2.2f, 0.0f, 0},
        {"CIE-XYZ-D65_to_G2.6-P3-D65",       "Linear-P3-D65",     1, 2.6f, 0.0f, 0},
        {"CIE-XYZ-D65_to_DCI-P3",            "Linear-DCI-P3",     1, 2.6f, 0.0f, 0},
        {"CIE-XYZ-D65_to_REC.1886-REC.709",  "Linear-Rec709",     1, 2.4f, 0.0f, 0},
        {"CIE-XYZ-D65_to_Rec.1886-Rec.709",  "Linear-Rec709",     1, 2.4f, 0.0f, 0},
        {"CIE-XYZ-D65_to_REC.1886-REC.2020", "Linear-Rec2020",    1, 2.4f, 0.0f, 0},
        {"CIE-XYZ-D65_to_REC.2100-PQ",       "Linear-Rec2020",    2, 0.0f, 0.0f, 0},
        {"CIE-XYZ-D65_to_Rec.2100-PQ",       "Linear-Rec2020",    2, 0.0f, 0.0f, 0},
        {"CIE-XYZ-D65_to_ST2084-P3-D65",     "Linear-P3-D65",     2, 0.0f, 0.0f, 0},
        {"CIE-XYZ-D65_to_Rec.2100-HLG",      "Linear-Rec2020",    3, 0.0f, 0.0f, 0},
        {"CIE-XYZ-D65_to_DCDM-D65",          "CIE-XYZ-D65",       4, 2.6f, 0.0f, 0},
        {"CIE-XYZ-D65_to_ST2084-DCDM-D65",   "CIE-XYZ-D65",       5, 0.0f, 0.0f, 0},
    };
    char buf[128];
    const char *inner = style;
    size_t i, n;
    int mir = 0; /* "- MIRROR NEGS": odd-extend the encode for negatives */
    /* strip optional "DISPLAY - " prefix and " - MIRROR NEGS" suffix */
    if (strlen(style) >= 10 && memcmp(style, "DISPLAY - ", 10) == 0)
        inner = style + 10;
    n = strlen(inner);
    if (n < sizeof(buf)) {
        memcpy(buf, inner, n + 1);
        if (ends_with(buf, " - MIRROR NEGS")) { buf[n - 14] = '\0'; mir = 1; }
        inner = buf;
    }
    for (i = 0; i < sizeof(d) / sizeof(d[0]); ++i) {
        int m;
        if (strcmp(inner, d[i].name) != 0) continue;
        m = mir || d[i].amir;
        if (!push_cspace_named(list, "CIE-XYZ-D65", d[i].prim, rc)) return 1;
        if (d[i].kind == 0 && !push_moncurve(list, d[i].g, d[i].off, 1, m))
            *rc = TOC_ERROR_OUT_OF_MEMORY;
        else if (d[i].kind == 1 && !push_gamma(list, d[i].g, 1, m))
            *rc = TOC_ERROR_OUT_OF_MEMORY;
        else if (d[i].kind == 2 && !push_ff_style(list, TOC_FF_LIN_TO_PQ))
            *rc = TOC_ERROR_OUT_OF_MEMORY;
        else if (d[i].kind == 3 && !push_ff_style(list, TOC_FF_LIN_TO_HLG))
            *rc = TOC_ERROR_OUT_OF_MEMORY;
        else if (d[i].kind == 4) { /* DCDM: scale(48/52.37) + gamma 2.6 over XYZ */
            if (!push_scale3(list, 48.0f / 52.37f) ||
                !push_gamma(list, d[i].g, 1, 0))
                *rc = TOC_ERROR_OUT_OF_MEMORY;
        } else if (d[i].kind == 5 && !push_ff_style(list, TOC_FF_LIN_TO_PQ)) {
            *rc = TOC_ERROR_OUT_OF_MEMORY; /* ST2084-DCDM: PQ over XYZ */
        }
        return 1;
    }
    return 0;
}

/* ---- ACES 2.0 output transform expansion -------------------------------- */
/* {rx,ry, gx,gy, bx,by, wx,wy}. The D60 variants share the D65 versions'
 * primaries but adopt the ACES D60 white (0.32168, 0.33767) for "simulating D60
 * white" output transforms. CIE-XYZ illuminant E (identity primaries, white E)
 * is an encoding target for the DCDM/XYZ-E outputs. */
static const float ACES2_REC709[8]      = {0.640f, 0.330f, 0.300f, 0.600f,
                                           0.150f, 0.060f, 0.3127f, 0.3290f};
static const float ACES2_P3D65[8]       = {0.680f, 0.320f, 0.265f, 0.690f,
                                           0.150f, 0.060f, 0.3127f, 0.3290f};
static const float ACES2_REC2020[8]     = {0.708f, 0.292f, 0.170f, 0.797f,
                                           0.131f, 0.046f, 0.3127f, 0.3290f};
static const float ACES2_REC709_D60[8]  = {0.640f, 0.330f, 0.300f, 0.600f,
                                           0.150f, 0.060f, 0.32168f, 0.33767f};
static const float ACES2_P3_D60[8]      = {0.680f, 0.320f, 0.265f, 0.690f,
                                           0.150f, 0.060f, 0.32168f, 0.33767f};
static const float ACES2_REC2020_D60[8] = {0.708f, 0.292f, 0.170f, 0.797f,
                                           0.131f, 0.046f, 0.32168f, 0.33767f};
static const float ACES2_XYZ_E[8]       = {1.0f, 0.0f, 0.0f, 1.0f,
                                           0.0f, 0.0f, 1.0f/3.0f, 1.0f/3.0f};

/* RGB->XYZ matrix (row-major) for primaries `p`, white = the space's own (no
 * chromatic adaptation) -- i.e. OCIO build_conversion_matrix_to_XYZ_D65 with
 * ADAPTATION_NONE. Mirrors OCIO's rgb2xyz_from_xy: each column carries the
 * primary chromaticity (x, y, z=1-x-y) and per-primary gains are solved from
 * the white point, so it stays well-conditioned even when a primary has y==0
 * (CIE-XYZ illuminant-E's blue). */
static int aces2_rgb2xyz(const float p[8], float M[9]) {
    float C[9], Cinv[9], W[3], g[3];
    int i, j;
    for (i = 0; i < 3; ++i) {
        C[0 + i] = p[i * 2];                          /* x  (column = primary) */
        C[3 + i] = p[i * 2 + 1];                      /* y */
        C[6 + i] = 1.0f - p[i * 2] - p[i * 2 + 1];    /* z = 1 - x - y */
    }
    if (!mat3_inv(C, Cinv)) return 0;
    W[0] = p[6] / p[7];
    W[1] = 1.0f;
    W[2] = (1.0f - p[6] - p[7]) / p[7];
    for (i = 0; i < 3; ++i)
        g[i] = Cinv[i * 3 + 0] * W[0] + Cinv[i * 3 + 1] * W[1] +
               Cinv[i * 3 + 2] * W[2];
    for (j = 0; j < 3; ++j)
        for (i = 0; i < 3; ++i) M[j * 3 + i] = g[i] * C[j * 3 + i];
    return 1;
}

static toc_op *push_range_clamp(toc_op_list *list, float lo, float hi) {
    toc_op *op = toc_op_list_push(list, TOC_OP_RANGE);
    int c;
    if (!op) return NULL;
    for (c = 0; c < 4; ++c) {
        op->u.range.scale[c] = 1.0f;
        op->u.range.offset[c] = 0.0f;
        op->u.range.min[c] = lo;
        op->u.range.max[c] = hi;
    }
    op->u.range.clamp_lo = op->u.range.clamp_hi = 1;
    return op;
}
static toc_op *push_scale3(toc_op_list *list, float s) {
    toc_op *op = toc_op_list_push(list, TOC_OP_RANGE);
    int c;
    if (!op) return NULL;
    for (c = 0; c < 4; ++c) {
        op->u.range.scale[c] = (c < 3) ? s : 1.0f;
        op->u.range.offset[c] = 0.0f;
        op->u.range.min[c] = 0.0f;
        op->u.range.max[c] = 1.0f;
    }
    op->u.range.clamp_lo = op->u.range.clamp_hi = 0; /* no clamp */
    return op;
}

/* "White point simulation" scale for the D60-sim outputs: OCIO scales RGB by
 * 1/max(channel) of the limiting white (1,1,1) carried through the limiting->
 * encoding primaries matrix (no adaptation), so the simulated white lands at or
 * below the encoding peak. Returns 1 on success (or *rc set on OOM). */
static int push_scale_white(toc_op_list *list, const float lim[8],
                            const float enc[8], toc_result *rc) {
    float Mlim[9], Menc[9], Mencinv[9], wxyz[3], wrgb[3];
    static const float ones[3] = {1.0f, 1.0f, 1.0f};
    float mx;
    if (!aces2_rgb2xyz(lim, Mlim) || !aces2_rgb2xyz(enc, Menc) ||
        !mat3_inv(Menc, Mencinv)) {
        *rc = TOC_ERROR_UNSUPPORTED;
        return 0;
    }
    mat3_vec(Mlim, ones, wxyz);   /* limiting white in XYZ */
    mat3_vec(Mencinv, wxyz, wrgb); /* ... expressed in encoding RGB */
    mx = wrgb[0] > wrgb[1] ? wrgb[0] : wrgb[1];
    if (wrgb[2] > mx) mx = wrgb[2];
    if (!push_scale3(list, 1.0f / mx)) { *rc = TOC_ERROR_OUT_OF_MEMORY; return 0; }
    return 1;
}

/* Build the OCIO ACES 2.0 output-transform op chain (ACES2065-1 -> CIE-XYZ-D65)
 * for a known builtin variant. Returns 1 if `style` named a supported variant
 * (op chain built, or *rc set on inverse/OOM), 0 if not an ACES-OUTPUT style. */
static int push_aces_output(toc_op_list *list, const char *style, int invert,
                            toc_result *rc) {
    /* lim: limiting primaries (drive both the tonescale/gamut fixed function and
     * the final RGB->XYZ matrix). enc: encoding primaries used by the D60 white-
     * point simulation scale, or NULL when there is no white-point scale. */
    static const struct {
        const char *name;
        float peak;
        const float *lim;
        const float *enc;
        float lscale;
    } V[] = {
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - SDR-100nit-REC709_2.0",
         100.0f, ACES2_REC709, NULL, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - SDR-100nit-P3-D65_2.0",
         100.0f, ACES2_P3D65, NULL, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-500nit-P3-D65_2.0",
         500.0f, ACES2_P3D65, NULL, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-1000nit-P3-D65_2.0",
         1000.0f, ACES2_P3D65, NULL, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-1000nit-REC2020_2.0",
         1000.0f, ACES2_REC2020, NULL, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-108nit-P3-D65_2.0",
         225.0f, ACES2_P3D65, NULL, 0.48f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-300nit-P3-D65_2.0",
         625.0f, ACES2_P3D65, NULL, 0.48f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-2000nit-P3-D65_2.0",
         2000.0f, ACES2_P3D65, NULL, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-4000nit-P3-D65_2.0",
         4000.0f, ACES2_P3D65, NULL, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-500nit-REC2020_2.0",
         500.0f, ACES2_REC2020, NULL, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-2000nit-REC2020_2.0",
         2000.0f, ACES2_REC2020, NULL, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-4000nit-REC2020_2.0",
         4000.0f, ACES2_REC2020, NULL, 1.0f},
        /* D60 white-point simulation: limiting primaries adopt the D60 white and
         * the simulated white is scaled into the encoding gamut (scale_white). */
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - SDR-100nit-REC709-D60-in-REC709-D65_2.0",
         100.0f, ACES2_REC709_D60, ACES2_REC709, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - SDR-100nit-REC709-D60-in-P3-D65_2.0",
         100.0f, ACES2_REC709_D60, ACES2_P3D65, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - SDR-100nit-REC709-D60-in-REC2020-D65_2.0",
         100.0f, ACES2_REC709_D60, ACES2_REC2020, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - SDR-100nit-P3-D60-in-P3-D65_2.0",
         100.0f, ACES2_P3_D60, ACES2_P3D65, 1.0f},
        /* XYZ-E encoding: no white-point scale for the 100 nit SDR variant. */
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - SDR-100nit-P3-D60-in-XYZ-E_2.0",
         100.0f, ACES2_P3_D60, NULL, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-108nit-P3-D60-in-P3-D65_2.0",
         225.0f, ACES2_P3_D60, ACES2_P3D65, 0.48f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-300nit-P3-D60-in-XYZ-E_2.0",
         625.0f, ACES2_P3_D60, ACES2_XYZ_E, 0.48f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-500nit-P3-D60-in-P3-D65_2.0",
         500.0f, ACES2_P3_D60, ACES2_P3D65, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-1000nit-P3-D60-in-P3-D65_2.0",
         1000.0f, ACES2_P3_D60, ACES2_P3D65, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-2000nit-P3-D60-in-P3-D65_2.0",
         2000.0f, ACES2_P3_D60, ACES2_P3D65, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-4000nit-P3-D60-in-P3-D65_2.0",
         4000.0f, ACES2_P3_D60, ACES2_P3D65, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-500nit-P3-D60-in-REC2020-D65_2.0",
         500.0f, ACES2_P3_D60, ACES2_REC2020, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-1000nit-P3-D60-in-REC2020-D65_2.0",
         1000.0f, ACES2_P3_D60, ACES2_REC2020, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-2000nit-P3-D60-in-REC2020-D65_2.0",
         2000.0f, ACES2_P3_D60, ACES2_REC2020, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-4000nit-P3-D60-in-REC2020-D65_2.0",
         4000.0f, ACES2_P3_D60, ACES2_REC2020, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-500nit-REC2020-D60-in-REC2020-D65_2.0",
         500.0f, ACES2_REC2020_D60, ACES2_REC2020, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-1000nit-REC2020-D60-in-REC2020-D65_2.0",
         1000.0f, ACES2_REC2020_D60, ACES2_REC2020, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-2000nit-REC2020-D60-in-REC2020-D65_2.0",
         2000.0f, ACES2_REC2020_D60, ACES2_REC2020, 1.0f},
        {"ACES-OUTPUT - ACES2065-1_to_CIE-XYZ-D65 - HDR-4000nit-REC2020-D60-in-REC2020-D65_2.0",
         4000.0f, ACES2_REC2020_D60, ACES2_REC2020, 1.0f},
    };
    size_t i;
    for (i = 0; i < sizeof(V) / sizeof(V[0]); ++i) {
        float U;
        toc_aces2 *blob;
        toc_op *op;
        if (strcmp(style, V[i].name) != 0) continue;
        if (invert) { *rc = TOC_ERROR_UNSUPPORTED; return 1; }
        /* upperBound = 8*(128 + 768*log(peak/100)/log(100)) */
        U = 8.0f * (128.0f + 768.0f *
                    (toc_log2f(V[i].peak / 100.0f) / toc_log2f(100.0f)));
        if (!push_mat3(list, AP0_TO_AP1) || !push_range_clamp(list, 0.0f, U) ||
            !push_mat3(list, AP1_TO_AP0)) {
            *rc = TOC_ERROR_OUT_OF_MEMORY;
            return 1;
        }
        blob = toc_aces2_init(&list->alloc, V[i].peak, V[i].lim);
        if (!blob || !toc_op_list_own(list, (float *)blob)) {
            if (blob) toc_free(&list->alloc, blob);
            *rc = TOC_ERROR_OUT_OF_MEMORY;
            return 1;
        }
        op = toc_op_list_push(list, TOC_OP_ACES_OUTPUT);
        if (!op) { *rc = TOC_ERROR_OUT_OF_MEMORY; return 1; }
        op->u.aces.t = blob;
        op->u.aces.inverse = 0;
        if (!push_range_clamp(list, 0.0f, V[i].peak / 100.0f)) {
            *rc = TOC_ERROR_OUT_OF_MEMORY;
            return 1;
        }
        if (V[i].enc && !push_scale_white(list, V[i].lim, V[i].enc, rc))
            return 1;
        if (V[i].lscale != 1.0f && !push_scale3(list, V[i].lscale)) {
            *rc = TOC_ERROR_OUT_OF_MEMORY;
            return 1;
        }
        /* matrixToXYZ = limiting primaries -> CIE-XYZ-D65 with no adaptation
         * (so a D60-sim's limiting white stays unadapted -- the simulation). */
        {
            float M[9];
            if (!aces2_rgb2xyz(V[i].lim, M) || !push_mat3(list, M))
                *rc = TOC_ERROR_OUT_OF_MEMORY;
        }
        return 1;
    }
    return 0;
}

/* ACEScc is a pure affine log2 (no linear toe; the 2^-16 toe only affects
 * lin < 2^-15, treated as extended-domain). Same slope/offset as ACEScct but
 * with no linear-segment break, so a plain LOG op suffices and inverts by flag.
 * log_to_lin=1: ACEScc->linear (decode); 0: linear->ACEScc (encode). */
static toc_op *push_acescc_log(toc_op_list *list, int log_to_lin) {
    toc_op *op = toc_op_list_push(list, TOC_OP_LOG);
    int i;
    if (!op) return NULL;
    op->u.log.base = 2.0f;
    for (i = 0; i < 3; ++i) {
        op->u.log.log_slope[i] = 1.0f / 17.52f;
        op->u.log.log_offset[i] = 9.72f / 17.52f;
        op->u.log.lin_slope[i] = 1.0f;
        op->u.log.lin_offset[i] = 0.0f;
    }
    op->u.log.inverse = log_to_lin ? 1 : 0;
    return op;
}

/* Push a camera LogCamera curve as log->lin (decode), computing the linear
 * segment for C0 continuity exactly like toc_lower_transform's LogCamera path. */
static toc_op *push_camera_logcam(toc_op_list *list, float base, float ls,
                                  float lo, float ns, float no, float brk,
                                  int has_lslope, float lslope) {
    toc_op *op = toc_op_list_push(list, TOC_OP_LOG_CAMERA);
    float lnb = toc_log2f(base) * 0.6931471805599453f; /* ln(base) */
    float xb = ns * brk + no;
    float yb = ls * (toc_log2f(xb > 0.0f ? xb : 1e-30f) / toc_log2f(base)) + lo;
    float lsl = has_lslope ? lslope
                           : (ls * ns / ((xb != 0.0f ? xb : 1e-30f) * lnb));
    int i;
    if (!op) return NULL;
    op->u.logcam.base = base;
    for (i = 0; i < 3; ++i) {
        op->u.logcam.log_slope[i] = ls;
        op->u.logcam.log_offset[i] = lo;
        op->u.logcam.lin_slope[i] = ns;
        op->u.logcam.lin_offset[i] = no;
        op->u.logcam.lin_break[i] = brk;
        op->u.logcam.linear_slope[i] = lsl;
        op->u.logcam.linear_offset[i] = yb - lsl * brk;
    }
    op->u.logcam.inverse = 1; /* log -> lin */
    return op;
}

/* Camera "<X>_to_ACES2065-1" builtins: LogCamera decode + gamut->AP0 matrix.
 * (Canon/Apple use baked LUTs upstream and are not covered here.) Returns 1 if
 * `style` named a supported camera builtin, 0 otherwise. */
static int push_camera(toc_op_list *list, const char *style, toc_result *rc) {
    /* Sony Venice gamuts use OCIO's explicit camera->AP0 matrices (row-major). */
    static const float SVEN[9] = {
        0.7933297411f, 0.0890786256f, 0.1175916333f,
        0.0155810585f, 1.0327123069f, -0.0482933654f,
        -0.0188647478f, 0.0127694121f, 1.0060953358f};
    static const float SVENC[9] = {
        0.6742570921f, 0.2205717359f, 0.1051711720f,
        -0.0093136061f, 1.1059588614f, -0.0966452553f,
        -0.0382090673f, -0.0179383766f, 1.0561474439f};
    static const struct {
        const char *name;
        float prim[8];
        const float *mat; /* explicit matrix, or NULL to derive from prim */
        int cat02;        /* when deriving: 1 = CAT02, 0 = Bradford */
        float base, ls, lo, ns, no, brk;
        int has_lslope;
        float lslope;
    } C[] = {
        {"ARRI_ALEXA-LOGC-EI800-AWG_to_ACES2065-1",
         {0.684f, 0.313f, 0.221f, 0.848f, 0.0861f, -0.102f, 0.3127f, 0.329f},
         NULL, 1, 10.0f, 0.2471896383f, 0.3855369987f, 1.0f / 0.18f,
         0.0522722750f, 0.0105909905f, 0, 0.0f},
        {"ARRI_LOGC4_to_ACES2065-1",
         {0.7347f, 0.2653f, 0.1424f, 0.8576f, 0.0991f, -0.0308f, 0.3127f, 0.329f},
         NULL, 1, 2.0f, 0.0647954196341293f, -0.295908392682586f,
         2231.82630906769f, 64.0f, -0.0180569961199113f, 0, 0.0f},
        {"SONY_SLOG3-SGAMUT3_to_ACES2065-1",
         {0.730f, 0.280f, 0.140f, 0.855f, 0.100f, -0.050f, 0.3127f, 0.329f},
         NULL, 1, 10.0f, 261.5f / 1023.0f, 420.0f / 1023.0f, 1.0f / 0.19f,
         0.01f / 0.19f, 0.01125f, 1, 6.62292117f},
        {"SONY_SLOG3-SGAMUT3.CINE_to_ACES2065-1",
         {0.766f, 0.275f, 0.225f, 0.800f, 0.089f, -0.087f, 0.3127f, 0.329f},
         NULL, 1, 10.0f, 261.5f / 1023.0f, 420.0f / 1023.0f, 1.0f / 0.19f,
         0.01f / 0.19f, 0.01125f, 1, 6.62292117f},
        {"SONY_SLOG3-SGAMUT3-VENICE_to_ACES2065-1",
         {0}, SVEN, 0, 10.0f, 261.5f / 1023.0f, 420.0f / 1023.0f, 1.0f / 0.19f,
         0.01f / 0.19f, 0.01125f, 1, 6.62292117f},
        {"SONY_SLOG3-SGAMUT3.CINE-VENICE_to_ACES2065-1",
         {0}, SVENC, 0, 10.0f, 261.5f / 1023.0f, 420.0f / 1023.0f, 1.0f / 0.19f,
         0.01f / 0.19f, 0.01125f, 1, 6.62292117f},
        {"RED_LOG3G10-RWG_to_ACES2065-1",
         {0.780308f, 0.304253f, 0.121595f, 1.493994f, 0.095612f, -0.084589f,
          0.3127f, 0.329f},
         NULL, 0, 10.0f, 0.224282f, 0.0f, 155.975327f,
         0.01f * 155.975327f + 1.0f, -0.01f, 0, 0.0f},
        {"PANASONIC_VLOG-VGAMUT_to_ACES2065-1",
         {0.730f, 0.280f, 0.165f, 0.840f, 0.100f, -0.030f, 0.3127f, 0.329f},
         NULL, 0, 10.0f, 0.241514f, 0.598206f, 1.0f, 0.00873f, 0.01f, 0, 0.0f},
    };
    size_t i;
    for (i = 0; i < sizeof(C) / sizeof(C[0]); ++i) {
        if (strcmp(style, C[i].name) != 0) continue;
        if (!push_camera_logcam(list, C[i].base, C[i].ls, C[i].lo, C[i].ns,
                                C[i].no, C[i].brk, C[i].has_lslope, C[i].lslope)) {
            *rc = TOC_ERROR_OUT_OF_MEMORY;
            return 1;
        }
        if (C[i].mat) {
            if (!push_mat3(list, C[i].mat)) *rc = TOC_ERROR_OUT_OF_MEMORY;
        } else {
            float M[9];
            if (!cam_to_ap0(C[i].prim, C[i].cat02, M) || !push_mat3(list, M))
                *rc = TOC_ERROR_OUT_OF_MEMORY;
        }
        return 1;
    }
    return 0;
}

static toc_result reverse_invert(toc_op_list *list, size_t start) {
    size_t i, j;
    for (i = start, j = list->count; i < j; ++i, --j) {
        toc_op t = list->ops[i];
        list->ops[i] = list->ops[j - 1];
        list->ops[j - 1] = t;
    }
    for (i = start; i < list->count; ++i) {
        toc_result rc = toc_invert_op(list, &list->ops[i]);
        if (!TOC_OK(rc)) return rc;
    }
    return TOC_SUCCESS;
}

toc_result toc_builtin_expand(toc_op_list *list, const char *style, int invert) {
    size_t start = list->count;
    toc_result rc = TOC_SUCCESS;
    int matched = 1;
    /* ACES 2.0 output transforms build their own (non-trivially-invertible)
     * chain and must not flow through reverse_invert below. */
    if (push_aces_output(list, style, invert, &rc)) return rc;
    if (strcmp(style, "ACEScg_to_ACES2065-1") == 0) {
        if (!push_mat3(list, AP1_TO_AP0)) rc = TOC_ERROR_OUT_OF_MEMORY;
    } else if (strcmp(style, "ACES2065-1_to_ACEScg") == 0) {
        if (!push_mat3(list, AP0_TO_AP1)) rc = TOC_ERROR_OUT_OF_MEMORY;
    } else if (strcmp(style, "ACEScct_to_ACES2065-1") == 0) {
        if (!push_acescct_log(list, 1) || !push_mat3(list, AP1_TO_AP0))
            rc = TOC_ERROR_OUT_OF_MEMORY;
    } else if (strcmp(style, "ACES2065-1_to_ACEScct") == 0) {
        if (!push_mat3(list, AP0_TO_AP1) || !push_acescct_log(list, 0))
            rc = TOC_ERROR_OUT_OF_MEMORY;
    } else if (strcmp(style, "ACEScc_to_ACES2065-1") == 0) {
        /* Clamp the ACEScc input to its minimum ACEScc(0)=(log2(2^-16)+9.72)/
         * 17.52. Forward this is a no-op for valid codes; when this builtin is
         * inverted (lin->ACEScc, the config's ACEScg->ACEScc path) the clamp
         * reverses to floor the encode output, matching OCIO for black/negatives.
         * Exact for lin >= 2^-15; only the tiny toe (lin < 2^-15) differs. */
        if (!push_range_clamp(list, (-16.0f + 9.72f) / 17.52f, 1e30f) ||
            !push_acescc_log(list, 1) || !push_mat3(list, AP1_TO_AP0))
            rc = TOC_ERROR_OUT_OF_MEMORY;
    } else if (strcmp(style, "ACES2065-1_to_ACEScc") == 0) {
        if (!push_mat3(list, AP0_TO_AP1) || !push_acescc_log(list, 0) ||
            !push_range_clamp(list, (-16.0f + 9.72f) / 17.52f, 1e30f))
            rc = TOC_ERROR_OUT_OF_MEMORY;
    } else if (strcmp(style, "UTILITY - ACES-AP0_to_CIE-XYZ-D65_BFD") == 0) {
        /* AP0 (D60) -> CIE-XYZ with Bradford adaptation to D65. */
        push_cspace_named(list, "Linear-AP0", "CIE-XYZ-D65", &rc);
    } else if (strcmp(style, "ACES-LMT - ACES 1.3 Reference Gamut Compression") == 0) {
        /* AP0->AP1, ACES 1.3 gamut compress, AP1->AP0. */
        toc_op *ff;
        if (!push_mat3(list, AP0_TO_AP1)) { rc = TOC_ERROR_OUT_OF_MEMORY; }
        else if (!(ff = toc_op_list_push(list, TOC_OP_FIXEDFUNC))) {
            rc = TOC_ERROR_OUT_OF_MEMORY;
        } else {
            static const float p[7] = {1.147f, 1.264f, 1.312f,
                                       0.815f, 0.803f, 0.880f, 1.2f};
            int k;
            ff->u.fixedfunc.style = TOC_FF_ACES_GAMUTCOMP13;
            for (k = 0; k < 7; ++k) ff->u.fixedfunc.params[k] = p[k];
            ff->u.fixedfunc.nparams = 7;
            if (!push_mat3(list, AP1_TO_AP0)) rc = TOC_ERROR_OUT_OF_MEMORY;
        }
    } else if (push_camera(list, style, &rc)) {
        /* handled: a camera-log "<X>_to_ACES2065-1" builtin */
    } else if (push_display_xform(list, style, &rc)) {
        /* handled: a composed CIE-XYZ-D65 -> display output transform */
    } else if (push_cspace_convert(list, style, &rc)) {
        /* handled: a linear color-space (primaries) conversion */
    } else {
        matched = 0;
    }
    if (!matched) return TOC_ERROR_UNSUPPORTED;
    if (!TOC_OK(rc)) return rc;
    if (invert) return reverse_invert(list, start);
    return TOC_SUCCESS;
}

/* ---- FixedFunction lowering: parse YAML style + params -> op ------------- */
toc_result toc_lower_fixedfunc(toc_op_list *list, const toc_node *node,
                               int invert) {
    const char *style = toc_node_scalar(toc_node_map_get(node, "style"));
    const toc_node *params = toc_node_map_get(node, "params");
    toc_op *op;

    if (!style) return TOC_ERROR_PARSE;

    /* REC2100_SURROUND: one param (gamma, default 0.78). */
    if (strcmp(style, "REC2100_SURROUND") == 0 ||
        strcmp(style, "Rec2100Surround") == 0) {
        op = toc_op_list_push(list, TOC_OP_FIXEDFUNC);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.fixedfunc.style =
            invert ? TOC_FF_REC2100_SURROUND_INV : TOC_FF_REC2100_SURROUND;
        op->u.fixedfunc.params[0] = parse_param(params, 0, 0.78f);
        op->u.fixedfunc.nparams = 1;
        return TOC_SUCCESS;
    }

    /* ACES_Glow_03: baked glowGain=0.075, glowMid=0.1 */
    if (strcmp(style, "ACES_Glow_03") == 0) {
        op = toc_op_list_push(list, TOC_OP_FIXEDFUNC);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.fixedfunc.style = invert ? TOC_FF_ACES_GLOW03_INV : TOC_FF_ACES_GLOW03;
        op->u.fixedfunc.params[0] = 0.075f;
        op->u.fixedfunc.params[1] = 0.1f;
        op->u.fixedfunc.nparams = 2;
        return TOC_SUCCESS;
    }

    /* ACES_Glow_10: baked glowGain=0.05, glowMid=0.08 */
    if (strcmp(style, "ACES_Glow_10") == 0) {
        op = toc_op_list_push(list, TOC_OP_FIXEDFUNC);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.fixedfunc.style = invert ? TOC_FF_ACES_GLOW10_INV : TOC_FF_ACES_GLOW10;
        op->u.fixedfunc.params[0] = 0.05f;
        op->u.fixedfunc.params[1] = 0.08f;
        op->u.fixedfunc.nparams = 2;
        return TOC_SUCCESS;
    }

    /* ACES_Dark_To_Dim_10: baked gamma */
    if (strcmp(style, "ACES_Dark_To_Dim_10") == 0) {
        op = toc_op_list_push(list, TOC_OP_FIXEDFUNC);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.fixedfunc.style =
            invert ? TOC_FF_ACES_DARKTODIM10_INV : TOC_FF_ACES_DARKTODIM10;
        op->u.fixedfunc.params[0] = invert ? 1.0192640913260627f : 0.9811f;
        op->u.fixedfunc.nparams = 1;
        return TOC_SUCCESS;
    }

    /* ACES_Gamut_Comp_13: 7 params [limC,limM,limY, thrC,thrM,thrY, power] */
    if (strcmp(style, "ACES_Gamut_Comp_13") == 0) {
        float limC = parse_param(params, 0, 0.0f);
        float limM = parse_param(params, 1, 0.0f);
        float limY = parse_param(params, 2, 0.0f);
        float thrC = parse_param(params, 3, 0.0f);
        float thrM = parse_param(params, 4, 0.0f);
        float thrY = parse_param(params, 5, 0.0f);
        float pwr  = parse_param(params, 6, 1.0f);
        op = toc_op_list_push(list, TOC_OP_FIXEDFUNC);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.fixedfunc.style =
            invert ? TOC_FF_ACES_GAMUTCOMP13_INV : TOC_FF_ACES_GAMUTCOMP13;
        op->u.fixedfunc.params[0] = limC;
        op->u.fixedfunc.params[1] = limM;
        op->u.fixedfunc.params[2] = limY;
        op->u.fixedfunc.params[3] = thrC;
        op->u.fixedfunc.params[4] = thrM;
        op->u.fixedfunc.params[5] = thrY;
        op->u.fixedfunc.params[6] = pwr;
        op->u.fixedfunc.nparams = 7;
        return TOC_SUCCESS;
    }

    /* ACES_Red_Mod_03: no params (baked constants) */
    if (strcmp(style, "ACES_Red_Mod_03") == 0) {
        op = toc_op_list_push(list, TOC_OP_FIXEDFUNC);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.fixedfunc.style =
            invert ? TOC_FF_ACES_RED_MOD_03_INV : TOC_FF_ACES_RED_MOD_03;
        op->u.fixedfunc.nparams = 0;
        return TOC_SUCCESS;
    }

    /* ACES_Red_Mod_10: no params (baked constants) */
    if (strcmp(style, "ACES_Red_Mod_10") == 0) {
        op = toc_op_list_push(list, TOC_OP_FIXEDFUNC);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.fixedfunc.style =
            invert ? TOC_FF_ACES_RED_MOD_10_INV : TOC_FF_ACES_RED_MOD_10;
        op->u.fixedfunc.nparams = 0;
        return TOC_SUCCESS;
    }

    /* Colorspace conversions: no params. */
    if (strcmp(style, "RGB_TO_HSV") == 0) {
        op = toc_op_list_push(list, TOC_OP_FIXEDFUNC);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.fixedfunc.style = invert ? TOC_FF_HSV_TO_RGB : TOC_FF_RGB_TO_HSV;
        op->u.fixedfunc.nparams = 0;
        return TOC_SUCCESS;
    }

    if (strcmp(style, "HSV_TO_RGB") == 0) {
        op = toc_op_list_push(list, TOC_OP_FIXEDFUNC);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.fixedfunc.style = invert ? TOC_FF_RGB_TO_HSV : TOC_FF_HSV_TO_RGB;
        op->u.fixedfunc.nparams = 0;
        return TOC_SUCCESS;
    }

    if (strcmp(style, "XYZ_TO_xyY") == 0) {
        op = toc_op_list_push(list, TOC_OP_FIXEDFUNC);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.fixedfunc.style = invert ? TOC_FF_xyY_TO_XYZ : TOC_FF_XYZ_TO_xyY;
        op->u.fixedfunc.nparams = 0;
        return TOC_SUCCESS;
    }

    if (strcmp(style, "xyY_TO_XYZ") == 0) {
        op = toc_op_list_push(list, TOC_OP_FIXEDFUNC);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.fixedfunc.style = invert ? TOC_FF_XYZ_TO_xyY : TOC_FF_xyY_TO_XYZ;
        op->u.fixedfunc.nparams = 0;
        return TOC_SUCCESS;
    }

    if (strcmp(style, "XYZ_TO_uvY") == 0) {
        op = toc_op_list_push(list, TOC_OP_FIXEDFUNC);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.fixedfunc.style = invert ? TOC_FF_uvY_TO_XYZ : TOC_FF_XYZ_TO_uvY;
        op->u.fixedfunc.nparams = 0;
        return TOC_SUCCESS;
    }

    if (strcmp(style, "uvY_TO_XYZ") == 0) {
        op = toc_op_list_push(list, TOC_OP_FIXEDFUNC);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.fixedfunc.style = invert ? TOC_FF_XYZ_TO_uvY : TOC_FF_uvY_TO_XYZ;
        op->u.fixedfunc.nparams = 0;
        return TOC_SUCCESS;
    }

    if (strcmp(style, "XYZ_TO_LUV") == 0) {
        op = toc_op_list_push(list, TOC_OP_FIXEDFUNC);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.fixedfunc.style = invert ? TOC_FF_LUV_TO_XYZ : TOC_FF_XYZ_TO_LUV;
        op->u.fixedfunc.nparams = 0;
        return TOC_SUCCESS;
    }

    if (strcmp(style, "LUV_TO_XYZ") == 0) {
        op = toc_op_list_push(list, TOC_OP_FIXEDFUNC);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.fixedfunc.style = invert ? TOC_FF_XYZ_TO_LUV : TOC_FF_LUV_TO_XYZ;
        op->u.fixedfunc.nparams = 0;
        return TOC_SUCCESS;
    }

    return TOC_ERROR_UNSUPPORTED;
}

/* ---- Per-style private helpers ------------------------------------------- */

/* ---- ACES Red Mod 03/10: hue-weighted saturation modulation --------------- */

/* Calculate hue weight using quadratic B-spline (from OCIO). */
static float redmod_hue_weight(float red, float grn, float blu,
                                float inv_width) {
    float a = 2.0f * red - (grn + blu);
    float b = 1.7320508075688772f * (grn - blu);
    float hue = ff_atan2f(b, a);
    float knot = hue * inv_width + 2.0f;
    int j = (int)knot;
    if (j < 0 || j >= 4) return 0.0f;
    {
        static const float M[4][4] = {
            {0.25f, 0.00f, 0.00f, 0.00f},
            {-0.75f, 0.75f, 0.75f, 0.25f},
            {0.75f, -1.50f, 0.00f, 1.00f},
            {-0.25f, 0.75f, -0.75f, 0.25f}};
        float t = knot - (float)j;
        const float *coefs = M[j];
        return coefs[3] + t * (coefs[2] + t * (coefs[1] + t * coefs[0]));
    }
}

static float redmod_sat_weight(float red, float grn, float blu) {
    float mn = ff_fminf(red, ff_fminf(grn, blu));
    float mx = ff_fmaxf(red, ff_fmaxf(grn, blu));
    float s_num = ff_fmaxf(1e-10f, mx) - ff_fmaxf(1e-10f, mn);
    float s_den = ff_fmaxf(1e-2f, mx);
    return s_num / s_den;
}

static void apply_redmod_fwd(float *px, float scale, float pivot,
                              float inv_width) {
    float red = px[0], grn = px[1], blu = px[2];
    float f_H = redmod_hue_weight(red, grn, blu, inv_width);
    if (f_H > 0.0f) {
        float f_S = redmod_sat_weight(red, grn, blu);
        float one_minus_scale = 1.0f - scale;
        float newRed = red + f_H * f_S * (pivot - red) * one_minus_scale;
        if (grn >= blu) {
            float hue_fac = (grn - blu) / ff_fmaxf(1e-10f, red - blu);
            grn = hue_fac * (newRed - blu) + blu;
        } else {
            float hue_fac = (blu - grn) / ff_fmaxf(1e-10f, red - grn);
            blu = hue_fac * (newRed - grn) + grn;
        }
        red = newRed;
    }
    px[0] = red; px[1] = grn; px[2] = blu;
}

static void apply_redmod_inv(float *px, float scale, float pivot,
                              float inv_width) {
    float red = px[0], grn = px[1], blu = px[2];
    float f_H = redmod_hue_weight(red, grn, blu, inv_width);
    if (f_H > 0.0f) {
        float one_minus_scale = 1.0f - scale;
        float minChan = ff_fminf(grn, blu);
        float a = f_H * one_minus_scale - 1.0f;
        float b = red - f_H * (pivot + minChan) * one_minus_scale;
        float c_val = f_H * pivot * minChan * one_minus_scale;
        float disc = b * b - 4.0f * a * c_val;
        float newRed = disc >= 0.0f ? (-b - toc_sqrtf(disc)) / (2.0f * a) : red;
        if (grn >= blu) {
            float hue_fac = (grn - blu) / ff_fmaxf(1e-10f, red - blu);
            grn = hue_fac * (newRed - blu) + blu;
        } else {
            float hue_fac = (blu - grn) / ff_fmaxf(1e-10f, red - grn);
            blu = hue_fac * (newRed - grn) + grn;
        }
        red = newRed;
    }
    px[0] = red; px[1] = grn; px[2] = blu;
}

/* ---- ACES Glow: sigmoid-weighted luminance-dependent glow ----------------- */
static float glow_yc(float red, float grn, float blu) {
    float c = blu * (blu - grn) + grn * (grn - red) + red * (red - blu);
    float chroma = c > 0.0f ? toc_sqrtf(c) : 0.0f;
    return (blu + grn + red + 1.75f * chroma) / 3.0f;
}

static float glow_sigmoid(float sat) {
    float x = (sat - 0.4f) * 5.0f;
    float sign = ff_copysignf(1.0f, x);
    float t = ff_fmaxf(0.0f, 1.0f - 0.5f * sign * x);
    return (1.0f + sign * (1.0f - t * t)) * 0.5f;
}

static void apply_glow_fwd(float *px, float glowGain, float glowMid) {
    float red = px[0], grn = px[1], blu = px[2];
    float YC = glow_yc(red, grn, blu);
    float sat = redmod_sat_weight(red, grn, blu);
    float s = glow_sigmoid(sat);
    float GG = glowGain * s;
    float gm = glowMid;
    float glowGainOut;
    if (YC >= gm * 2.0f) {
        glowGainOut = 0.0f;
    } else if (YC <= gm * 2.0f / 3.0f) {
        glowGainOut = GG;
    } else {
        glowGainOut = GG * (gm / YC - 0.5f);
    }
    float addedGlow = 1.0f + glowGainOut;
    px[0] = red * addedGlow; px[1] = grn * addedGlow; px[2] = blu * addedGlow;
}

static void apply_glow_inv(float *px, float glowGain, float glowMid) {
    float red = px[0], grn = px[1], blu = px[2];
    float YC = glow_yc(red, grn, blu);
    float sat = redmod_sat_weight(red, grn, blu);
    float s = glow_sigmoid(sat);
    float GG = glowGain * s;
    float gm = glowMid;
    float glowGainOut;
    if (YC >= gm * 2.0f) {
        glowGainOut = 0.0f;
    } else if (YC <= (1.0f + GG) * gm * 2.0f / 3.0f) {
        glowGainOut = -GG / (1.0f + GG);
    } else {
        glowGainOut = GG * (gm / YC - 0.5f) / (GG * 0.5f - 1.0f);
    }
    float reducedGlow = 1.0f + glowGainOut;
    px[0] = red * reducedGlow; px[1] = grn * reducedGlow; px[2] = blu * reducedGlow;
}

/* ---- ACES Dark-to-Dim: apply Y^(gamma-1) scaling on AP1 luminance -------- */
static void apply_darktodim(float *px, float gamma_minus_one) {
    float Y = ff_fmaxf(1e-10f,
        0.27222871678091454f * px[0] +
        0.67408176581114831f * px[1] +
        0.053689517407937051f * px[2]);
    float s = toc_powf(Y, gamma_minus_one);
    px[0] *= s; px[1] *= s; px[2] *= s;
}

/* ---- ACES Gamut Comp 13: parametric distance compression per axis --------- */
static float gc_compress(float dist, float thr, float scale, float power) {
    float nd = (dist - thr) / scale;
    float p = toc_powf(nd, power);
    float ip = 1.0f / power;
    return thr + scale * nd / toc_powf(1.0f + p, ip);
}

static float gc_uncompress(float dist, float thr, float scale, float power) {
    if (dist >= thr + scale) return dist;
    float nd = (dist - thr) / scale;
    float p = toc_powf(nd, power);
    float ip = 1.0f / power;
    return thr + scale * toc_powf(-(p / (p - 1.0f)), ip);
}

static float gc_apply(float val, float ach, float thr, float scale,
                       float power, int invert) {
    if (ach == 0.0f) return 0.0f;
    float dist = (ach - val) / ff_fabsf(ach);
    if (dist < thr) return val;
    float compr = invert ? gc_uncompress(dist, thr, scale, power)
                         : gc_compress(dist, thr, scale, power);
    return ach - compr * ff_fabsf(ach);
}

/* Gamut-compression scale: chosen so the limit distance maps exactly to the
 * gamut boundary, f(lim)=1, which gives
 *   scale = (lim-thr) / ((((lim-thr)/(1-thr))^power) - 1)^(1/power).
 * The real ACES limits are > 1 (distance from achromatic at the cusp); for a
 * degenerate lim <= 1 there is nothing outside the gamut to compress. */
static float gc_scale(float lim, float thr, float power) {
    float ip, t, d;
    if (lim <= 1.0f || thr >= 1.0f) return 1.0f;
    ip = 1.0f / power;
    t = (lim - thr) / (1.0f - thr); /* > 1, so t^power - 1 > 0 */
    d = toc_powf(toc_powf(t, power) - 1.0f, ip);
    return (d != 0.0f) ? (lim - thr) / d : 1.0f;
}

/* ---- RGB/HSV conversions ------------------------------------------------- */
static void apply_rgb_to_hsv(float *px) {
    float r = px[0], g = px[1], b = px[2];
    float mn = ff_fminf(r, ff_fminf(g, b));
    float mx = ff_fmaxf(r, ff_fmaxf(g, b));
    float val = mx, sat = 0.0f, hue = 0.0f;
    if (mn != mx) {
        float delta = mx - mn;
        if (mx != 0.0f) sat = delta / mx;
        if (r == mx)
            hue = (g - b) / delta;
        else if (g == mx)
            hue = 2.0f + (b - r) / delta;
        else
            hue = 4.0f + (r - g) / delta;
        if (hue < 0.0f) hue += 6.0f;
        hue *= 0.16666666666666666f;
    }
    if (mn < 0.0f) val += mn;
    if (-mn > mx) sat = (mx - mn) / -mn;
    px[0] = hue; px[1] = sat; px[2] = val;
}

static void apply_hsv_to_rgb(float *px) {
    float h = (px[0] - ff_floorf(px[0])) * 6.0f;
    float s = ff_clampf(px[1], 0.0f, 1.999f);
    float v = px[2];
    float r = ff_clampf(ff_fabsf(h - 3.0f) - 1.0f, 0.0f, 1.0f);
    float g = ff_clampf(2.0f - ff_fabsf(h - 2.0f), 0.0f, 1.0f);
    float b = ff_clampf(2.0f - ff_fabsf(h - 4.0f), 0.0f, 1.0f);
    float rgb_max = v, rgb_min = v * (1.0f - s);
    if (s > 1.0f) {
        rgb_min = v * (1.0f - s) / (2.0f - s);
        rgb_max = v - rgb_min;
    }
    if (v < 0.0f) {
        rgb_min = v / (2.0f - s);
        rgb_max = v - rgb_min;
    }
    float delta = rgb_max - rgb_min;
    px[0] = r * delta + rgb_min;
    px[1] = g * delta + rgb_min;
    px[2] = b * delta + rgb_min;
}

/* ---- XYZ / xyY conversions ------------------------------------------------ */
static void apply_xyz_to_xyy(float *px) {
    float X = px[0], Y = px[1], Z = px[2];
    float d = (X + Y + Z);
    d = (d == 0.0f) ? 0.0f : 1.0f / d;
    px[0] = X * d;
    px[1] = Y * d;
    px[2] = Y;
}

static void apply_xyy_to_xyz(float *px) {
    float x = px[0], y = px[1], Y = px[2];
    float d = (y == 0.0f) ? 0.0f : 1.0f / y;
    px[0] = Y * x * d;
    px[1] = Y;
    px[2] = Y * (1.0f - x - y) * d;
}

/* ---- XYZ / uvY conversions ------------------------------------------------ */
static void apply_xyz_to_uvy(float *px) {
    float X = px[0], Y = px[1], Z = px[2];
    float d = X + 15.0f * Y + 3.0f * Z;
    d = (d == 0.0f) ? 0.0f : 1.0f / d;
    px[0] = 4.0f * X * d;
    px[1] = 9.0f * Y * d;
    px[2] = Y;
}

static void apply_uvy_to_xyz(float *px) {
    float u = px[0], v = px[1], Y = px[2];
    float d = (v == 0.0f) ? 0.0f : 1.0f / v;
    float X = (9.0f / 4.0f) * Y * u * d;
    float Z = (3.0f / 4.0f) * Y * (4.0f - u - 6.666666666666667f * v) * d;
    px[0] = X;
    px[1] = Y;
    px[2] = Z;
}

/* ---- XYZ / LUV (CIELUV, D65 white) ---------------------------------------- */
static void apply_xyz_to_luv(float *px) {
    float X = px[0], Y = px[1], Z = px[2];
    float d = X + 15.0f * Y + 3.0f * Z;
    d = (d == 0.0f) ? 0.0f : 1.0f / d;
    float u = 4.0f * X * d;
    float v = 9.0f * Y * d;
    float Lstar = (Y <= 0.008856451679f)
                      ? 9.0329629629629608f * Y
                      : 1.16f * toc_powf(Y, 0.333333333f) - 0.16f;
    float ustar = 13.0f * Lstar * (u - 0.19783001f);
    float vstar = 13.0f * Lstar * (v - 0.46831999f);
    px[0] = Lstar; px[1] = ustar; px[2] = vstar;
}

static void apply_luv_to_xyz(float *px) {
    float Lstar = px[0], ustar = px[1], vstar = px[2];
    float d = (Lstar == 0.0f) ? 0.0f : 0.076923076923076927f / Lstar;
    float u = ustar * d + 0.19783001f;
    float v = vstar * d + 0.46831999f;
    float tmp = (Lstar + 0.16f) * 0.86206896551724144f;
    float Y = (Lstar <= 0.08f) ? 0.11070564598794539f * Lstar
                               : tmp * tmp * tmp;
    float dd = (v == 0.0f) ? 0.0f : 0.25f / v;
    float X = 9.0f * Y * u * dd;
    float Z = Y * (12.0f - 3.0f * u - 20.0f * v) * dd;
    px[0] = X; px[1] = Y; px[2] = Z;
}

/* ---- FixedFunction apply (the single dispatch point) ----------------------- */
/* ---- HDR display transfer functions (per-channel) ------------------------ */
#define TOC_LN2 0.6931471805599453f
#define TOC_LOG2E 1.4426950408889634f

/* SMPTE ST 2084 (PQ). Input is in nits/100 (1.0 == 100 cd/m^2, the OCIO/ACES
 * convention); internally scaled by 0.01 so 1.0 PQ == 10000 cd/m^2. */
static float pq_encode(float L) {
    float m1 = 0.1593017578125f, m2 = 78.84375f;
    float c1 = 0.8359375f, c2 = 18.8515625f, c3 = 18.6875f;
    float Lm;
    if (L <= 0.0f) return 0.0f;
    Lm = toc_powf(L * 0.01f, m1);
    return toc_powf((c1 + c2 * Lm) / (1.0f + c3 * Lm), m2);
}
static float pq_decode(float N) {
    float m1 = 0.1593017578125f, m2 = 78.84375f;
    float c1 = 0.8359375f, c2 = 18.8515625f, c3 = 18.6875f;
    float Np, num, den;
    if (N <= 0.0f) return 0.0f;
    Np = toc_powf(N, 1.0f / m2);
    num = Np - c1;
    if (num < 0.0f) num = 0.0f;
    den = c2 - c3 * Np;
    if (den <= 0.0f) return 0.0f;
    return toc_powf(num / den, 1.0f / m1) * 100.0f;
}
/* Rec.2100 HLG OETF (per-channel; the OOTF/system-gamma is not applied here). */
static float hlg_encode(float E) {
    float a = 0.17883277f, b = 0.28466892f, c = 0.55991073f;
    if (E <= 0.0f) return 0.0f;
    if (E <= 1.0f / 12.0f) return toc_sqrtf(3.0f * E);
    return a * (toc_log2f(12.0f * E - b) * TOC_LN2) + c;
}
static float hlg_decode(float Ep) {
    float a = 0.17883277f, b = 0.28466892f, c = 0.55991073f;
    if (Ep <= 0.0f) return 0.0f;
    if (Ep <= 0.5f) return Ep * Ep / 3.0f;
    return (toc_exp2f(((Ep - c) / a) * TOC_LOG2E) + b) / 12.0f;
}

void toc_fixedfunc_apply_pixel(const toc_op *op, float *px, int ch) {
    (void)ch;
    switch (op->u.fixedfunc.style) {
        case TOC_FF_LIN_TO_PQ:
            px[0] = pq_encode(px[0]); px[1] = pq_encode(px[1]);
            px[2] = pq_encode(px[2]); return;
        case TOC_FF_PQ_TO_LIN:
            px[0] = pq_decode(px[0]); px[1] = pq_decode(px[1]);
            px[2] = pq_decode(px[2]); return;
        case TOC_FF_LIN_TO_HLG:
            px[0] = hlg_encode(px[0]); px[1] = hlg_encode(px[1]);
            px[2] = hlg_encode(px[2]); return;
        case TOC_FF_HLG_TO_LIN:
            px[0] = hlg_decode(px[0]); px[1] = hlg_decode(px[1]);
            px[2] = hlg_decode(px[2]); return;
        case TOC_FF_REC2100_SURROUND:
        case TOC_FF_REC2100_SURROUND_INV: {
            float g = op->u.fixedfunc.params[0];
            float Y = 0.2627f * px[0] + 0.6780f * px[1] + 0.0593f * px[2];
            float e = (op->u.fixedfunc.style == TOC_FF_REC2100_SURROUND)
                          ? (g - 1.0f)
                          : (1.0f / g - 1.0f);
            if (Y <= 0.0f) return;
            Y = toc_powf(Y, e);
            px[0] *= Y; px[1] *= Y; px[2] *= Y;
            return;
        }

        /* ACES Glow 03/10 forward */
        case TOC_FF_ACES_GLOW03:
        case TOC_FF_ACES_GLOW10:
            apply_glow_fwd(px, op->u.fixedfunc.params[0],
                           op->u.fixedfunc.params[1]);
            return;

        /* ACES Glow 03/10 inverse */
        case TOC_FF_ACES_GLOW03_INV:
        case TOC_FF_ACES_GLOW10_INV:
            apply_glow_inv(px, op->u.fixedfunc.params[0],
                           op->u.fixedfunc.params[1]);
            return;

        /* ACES Dark-to-Dim 10 */
        case TOC_FF_ACES_DARKTODIM10:
            apply_darktodim(px, op->u.fixedfunc.params[0] - 1.0f);
            return;
        case TOC_FF_ACES_DARKTODIM10_INV: {
            float g = op->u.fixedfunc.params[0];
            apply_darktodim(px, (1.0f / g) - 1.0f);
            return;
        }

        /* ACES Gamut Comp 13 */
        case TOC_FF_ACES_GAMUTCOMP13:
        case TOC_FF_ACES_GAMUTCOMP13_INV: {
            int inv = (op->u.fixedfunc.style == TOC_FF_ACES_GAMUTCOMP13_INV);
            const float *p = op->u.fixedfunc.params;
            float limC = p[0], limM = p[1], limY = p[2];
            float thrC = p[3], thrM = p[4], thrY = p[5];
            float power = p[6];
            float sc = gc_scale(limC, thrC, power);
            float sm = gc_scale(limM, thrM, power);
            float sy = gc_scale(limY, thrY, power);
            float ach = ff_fmaxf(px[0], ff_fmaxf(px[1], px[2]));
            px[0] = gc_apply(px[0], ach, thrC, sc, power, inv);
            px[1] = gc_apply(px[1], ach, thrM, sm, power, inv);
            px[2] = gc_apply(px[2], ach, thrY, sy, power, inv);
            return;
        }

        /* ACES Red Mod 03 */
        case TOC_FF_ACES_RED_MOD_03:
            apply_redmod_fwd(px, 0.85f, 0.03f, 1.9098593171027443f);
            return;
        case TOC_FF_ACES_RED_MOD_03_INV:
            apply_redmod_inv(px, 0.85f, 0.03f, 1.9098593171027443f);
            return;

        /* ACES Red Mod 10 */
        case TOC_FF_ACES_RED_MOD_10:
            apply_redmod_fwd(px, 0.82f, 0.03f, 1.6976527263135504f);
            return;
        case TOC_FF_ACES_RED_MOD_10_INV:
            apply_redmod_inv(px, 0.82f, 0.03f, 1.6976527263135504f);
            return;

        /* Colorspace conversions */
        case TOC_FF_RGB_TO_HSV: apply_rgb_to_hsv(px); return;
        case TOC_FF_HSV_TO_RGB: apply_hsv_to_rgb(px); return;
        case TOC_FF_XYZ_TO_xyY: apply_xyz_to_xyy(px); return;
        case TOC_FF_xyY_TO_XYZ: apply_xyy_to_xyz(px); return;
        case TOC_FF_XYZ_TO_uvY: apply_xyz_to_uvy(px); return;
        case TOC_FF_uvY_TO_XYZ: apply_uvy_to_xyz(px); return;
        case TOC_FF_XYZ_TO_LUV: apply_xyz_to_luv(px); return;
        case TOC_FF_LUV_TO_XYZ: apply_luv_to_xyz(px); return;

        default:
            break;
    }
}
