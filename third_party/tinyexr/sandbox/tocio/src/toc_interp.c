/*
 * tocio - CPU interpreter (scalar reference). Walks the op list applying each
 * op to interleaved float RGB(A). SSE2/AVX2 kernels (later) must match these.
 *
 * Op math reimplemented from OpenColorIO (BSD-3-Clause).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

/* The scalar matrix/range kernels here are the bit-exact reference the SIMD
 * tiers (SSE2/NEON) must reproduce. Disable FP contraction so the compiler
 * cannot fuse a*b+c into an FMA: on aarch64 it otherwise emits fmla here (and
 * in the NEON kernels), and the two contraction patterns diverge, breaking the
 * bit-exact parity test. x86-64 has no baseline packed FMA so it was unaffected.
 * The OFF state is enforced by -ffp-contract=no in the Makefile, so the pragma
 * is intentionally omitted here — GCC's C11 pedantic mode does not recognize it. */

static float clampf(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

/* ---- per-pixel kernels (operate on up to 4 interleaved floats) ----------- */

static void k_matrix(const toc_op *op, float *px, int ch) {
    const float *m = op->u.matrix.m, *o = op->u.matrix.off;
    float r = px[0], g = px[1], b = px[2];
    float a = (ch == 4) ? px[3] : 1.0f;
    px[0] = m[0] * r + m[4] * g + m[8] * b + m[12] * a + o[0];
    px[1] = m[1] * r + m[5] * g + m[9] * b + m[13] * a + o[1];
    px[2] = m[2] * r + m[6] * g + m[10] * b + m[14] * a + o[2];
    if (ch == 4) px[3] = m[3] * r + m[7] * g + m[11] * b + m[15] * a + o[3];
}

static void k_range(const toc_op *op, float *px, int ch) {
    int c;
    for (c = 0; c < ch; ++c) {
        float v = px[c] * op->u.range.scale[c] + op->u.range.offset[c];
        if (op->u.range.clamp_lo && v < op->u.range.min[c]) v = op->u.range.min[c];
        if (op->u.range.clamp_hi && v > op->u.range.max[c]) v = op->u.range.max[c];
        px[c] = v;
    }
}

static void k_exponent(const toc_op *op, float *px, int ch) {
    int c, mir = op->u.exponent.mirror;
    for (c = 0; c < ch; ++c) {
        float x = px[c];
        if (mir) {
            float s = x < 0.0f ? -1.0f : 1.0f;
            px[c] = s * toc_powf(x < 0.0f ? -x : x, op->u.exponent.e[c]);
        } else {
            px[c] = toc_powf(x > 0.0f ? x : 0.0f, op->u.exponent.e[c]);
        }
    }
}

static void k_exp_linear(const toc_op *op, float *px, int ch) {
    int c, n = ch < 3 ? ch : 3, mir = op->u.exp_linear.mirror;
    for (c = 0; c < n; ++c) {
        float x = px[c], s = 1.0f;
        float scale = op->u.exp_linear.scale[c];
        float off = op->u.exp_linear.offset[c];
        float g = op->u.exp_linear.gamma[c];
        float brk = op->u.exp_linear.breakpoint[c];
        float slope = op->u.exp_linear.slope[c];
        if (mir && x < 0.0f) { s = -1.0f; x = -x; } /* odd extension for x<0 */
        if (!op->u.exp_linear.inverse) {
            px[c] = s * ((x > brk) ? toc_powf(x * scale + off, g) : x * slope);
        } else {
            float ybrk = brk * slope; /* y at the breakpoint */
            px[c] = s * ((x > ybrk) ? (toc_powf(x, 1.0f / g) - off) / scale
                                    : x / slope);
        }
    }
}

static void k_log(const toc_op *op, float *px, int ch) {
    int c, n = ch < 3 ? ch : 3;
    float lb = toc_log2f(op->u.log.base);
    for (c = 0; c < n; ++c) {
        float x = px[c];
        if (!op->u.log.inverse) {
            /* lin -> log: ls * log_base(lns*x + lno) + lo */
            float a = op->u.log.lin_slope[c] * x + op->u.log.lin_offset[c];
            float l = toc_log2f(a > 0.0f ? a : 1.1754944e-38f) / lb;
            px[c] = op->u.log.log_slope[c] * l + op->u.log.log_offset[c];
        } else {
            /* log -> lin: (base^((y-lo)/ls) - lno) / lns */
            float e = (x - op->u.log.log_offset[c]) / op->u.log.log_slope[c];
            float p = toc_raisef(op->u.log.base, e);
            px[c] = (p - op->u.log.lin_offset[c]) / op->u.log.lin_slope[c];
        }
    }
}

static void k_cdl(const toc_op *op, float *px, int ch) {
    float v[3];
    float luma;
    int c;
    const float *L = op->u.cdl.luma;
    float lr = L[0], lg = L[1], lb = L[2];
    if (lr == 0.0f && lg == 0.0f && lb == 0.0f) {
        lr = 0.2126f; lg = 0.7152f; lb = 0.0722f;
    }
    if (!op->u.cdl.inverse) {
        /* forward ASC CDL: slope/offset -> power -> saturation about luma. */
        for (c = 0; c < 3; ++c) {
            float x = px[c] * op->u.cdl.slope[c] + op->u.cdl.offset[c];
            if (op->u.cdl.clamp) x = clampf(x, 0.0f, 1.0f);
            if (x >= 0.0f) x = toc_powf(x, op->u.cdl.power[c]);
            /* else: leave negative as-is when not clamped */
            v[c] = x;
        }
        luma = lr * v[0] + lg * v[1] + lb * v[2];
        for (c = 0; c < 3; ++c) {
            float x = luma + op->u.cdl.saturation * (v[c] - luma);
            if (op->u.cdl.clamp) x = clampf(x, 0.0f, 1.0f);
            px[c] = x;
        }
    } else {
        /* inverse: saturation preserves luma, so luma = dot(out); undo in
         * reverse order (saturation -> power -> slope/offset). */
        float sat = op->u.cdl.saturation;
        luma = lr * px[0] + lg * px[1] + lb * px[2];
        for (c = 0; c < 3; ++c) {
            float x = luma + (sat != 0.0f ? (px[c] - luma) / sat : 0.0f);
            float s = op->u.cdl.slope[c];
            if (op->u.cdl.clamp) x = clampf(x, 0.0f, 1.0f);
            if (x >= 0.0f) x = toc_powf(x, 1.0f / op->u.cdl.power[c]);
            px[c] = (x - op->u.cdl.offset[c]) / (s != 0.0f ? s : 1.0f);
        }
    }
    (void)ch;
}

static void k_logcam(const toc_op *op, float *px, int ch) {
    int c, n = ch < 3 ? ch : 3;
    float lb = toc_log2f(op->u.logcam.base);
    for (c = 0; c < n; ++c) {
        float x = px[c];
        if (!op->u.logcam.inverse) {
            /* lin -> log with a linear segment below the breakpoint */
            if (x <= op->u.logcam.lin_break[c]) {
                px[c] = op->u.logcam.linear_slope[c] * x +
                        op->u.logcam.linear_offset[c];
            } else {
                float a =
                    op->u.logcam.lin_slope[c] * x + op->u.logcam.lin_offset[c];
                float l = toc_log2f(a > 0.0f ? a : 1.1754944e-38f) / lb;
                px[c] = op->u.logcam.log_slope[c] * l +
                        op->u.logcam.log_offset[c];
            }
        } else {
            float ybrk = op->u.logcam.linear_slope[c] *
                             op->u.logcam.lin_break[c] +
                         op->u.logcam.linear_offset[c];
            if (x <= ybrk) {
                px[c] = (x - op->u.logcam.linear_offset[c]) /
                        op->u.logcam.linear_slope[c];
            } else {
                float e = (x - op->u.logcam.log_offset[c]) /
                          op->u.logcam.log_slope[c];
                float p = toc_raisef(op->u.logcam.base, e);
                px[c] =
                    (p - op->u.logcam.lin_offset[c]) / op->u.logcam.lin_slope[c];
            }
        }
    }
}

/* LUT kernels (toc_lutfile / phase 4) declared here, defined in toc_lut.c. */
void toc_lut1d_apply_pixel(const toc_op *op, float *px, int ch);
void toc_lut3d_apply_pixel(const toc_op *op, float *px, int ch);
void toc_fixedfunc_apply_pixel(const toc_op *op, float *px, int ch);

/* ---- scalar batch reference kernels (vtbl defaults) ---------------------- */
void toc_matrix_batch_scalar(const float *m, const float *off, float *rgba,
                             size_t npix, int ch) {
    toc_op tmp;
    size_t i;
    tmp.kind = TOC_OP_MATRIX;
    memcpy(tmp.u.matrix.m, m, 16 * sizeof(float));
    memcpy(tmp.u.matrix.off, off, 4 * sizeof(float));
    for (i = 0; i < npix; ++i) k_matrix(&tmp, rgba + i * (size_t)ch, ch);
}
void toc_range_batch_scalar(const float *scale, const float *offset,
                            const float *vmin, const float *vmax, int clamp_lo,
                            int clamp_hi, float *rgba, size_t npix, int ch) {
    toc_op tmp;
    size_t i;
    tmp.kind = TOC_OP_RANGE;
    memcpy(tmp.u.range.scale, scale, 4 * sizeof(float));
    memcpy(tmp.u.range.offset, offset, 4 * sizeof(float));
    memcpy(tmp.u.range.min, vmin, 4 * sizeof(float));
    memcpy(tmp.u.range.max, vmax, 4 * sizeof(float));
    tmp.u.range.clamp_lo = clamp_lo;
    tmp.u.range.clamp_hi = clamp_hi;
    for (i = 0; i < npix; ++i) k_range(&tmp, rgba + i * (size_t)ch, ch);
}

/* Scalar batch references for the transcendental ops: the bit-exact source of
 * truth the NEON tier must reproduce. Each just walks the buffer per pixel. */
void toc_exponent_batch_scalar(const toc_op *op, float *rgba, size_t npix, int ch) {
    size_t i;
    for (i = 0; i < npix; ++i) k_exponent(op, rgba + i * (size_t)ch, ch);
}
void toc_log_batch_scalar(const toc_op *op, float *rgba, size_t npix, int ch) {
    size_t i;
    for (i = 0; i < npix; ++i) k_log(op, rgba + i * (size_t)ch, ch);
}
void toc_logcam_batch_scalar(const toc_op *op, float *rgba, size_t npix, int ch) {
    size_t i;
    for (i = 0; i < npix; ++i) k_logcam(op, rgba + i * (size_t)ch, ch);
}
void toc_explin_batch_scalar(const toc_op *op, float *rgba, size_t npix, int ch) {
    size_t i;
    for (i = 0; i < npix; ++i) k_exp_linear(op, rgba + i * (size_t)ch, ch);
}
void toc_cdl_batch_scalar(const toc_op *op, float *rgba, size_t npix, int ch) {
    size_t i;
    for (i = 0; i < npix; ++i) k_cdl(op, rgba + i * (size_t)ch, ch);
}

/* ---- SIMD dispatch table ------------------------------------------------- */
toc_simd_vtbl toc_simd = {toc_matrix_batch_scalar, toc_range_batch_scalar,
                          toc_exponent_batch_scalar, toc_log_batch_scalar,
                          toc_logcam_batch_scalar,   toc_explin_batch_scalar,
                          toc_cdl_batch_scalar};

void toc_simd_force(int level) {
    toc_simd.matrix = toc_matrix_batch_scalar;
    toc_simd.range = toc_range_batch_scalar;
    toc_simd.exponent = toc_exponent_batch_scalar;
    toc_simd.log_ = toc_log_batch_scalar;
    toc_simd.log_camera = toc_logcam_batch_scalar;
    toc_simd.exp_linear = toc_explin_batch_scalar;
    toc_simd.cdl = toc_cdl_batch_scalar;
#if defined(TOC_X86)
    if (level >= 1) {
        toc_simd.matrix = toc_matrix_batch_sse2;
        toc_simd.range = toc_range_batch_sse2;
    }
    if (level >= 2 && toc_cpu_has_avx2()) toc_simd.range = toc_range_batch_avx2;
#elif defined(TOC_NEON)
    if (level >= 1) {
        toc_simd.matrix = toc_matrix_batch_neon;
        toc_simd.range = toc_range_batch_neon;
        toc_simd.exponent = toc_exponent_batch_neon;
        toc_simd.log_ = toc_log_batch_neon;
        toc_simd.log_camera = toc_logcam_batch_neon;
        toc_simd.exp_linear = toc_explin_batch_neon;
        toc_simd.cdl = toc_cdl_batch_neon;
    }
#else
    (void)level;
#endif
}

void toc_simd_init(void) {
    static int done = 0;
    if (done) return;
#if defined(TOC_X86)
    toc_simd.matrix = toc_matrix_batch_sse2; /* SSE2 is baseline on x86-64 */
    toc_simd.range = toc_range_batch_sse2;
    if (toc_cpu_has_avx2()) toc_simd.range = toc_range_batch_avx2;
#elif defined(TOC_NEON)
    toc_simd.matrix = toc_matrix_batch_neon; /* NEON is baseline on aarch64 */
    toc_simd.range = toc_range_batch_neon;
    toc_simd.exponent = toc_exponent_batch_neon;
    toc_simd.log_ = toc_log_batch_neon;
    toc_simd.log_camera = toc_logcam_batch_neon;
    toc_simd.exp_linear = toc_explin_batch_neon;
    toc_simd.cdl = toc_cdl_batch_neon;
#endif
    done = 1;
}

/* Apply one op to a single pixel (the JIT calls this for ops it does not
 * inline; also the per-pixel fallback for the batch path). */
void toc_apply_op_pixel(const toc_op *op, float *px, int ch) {
    switch (op->kind) {
        case TOC_OP_MATRIX: k_matrix(op, px, ch); break;
        case TOC_OP_RANGE: k_range(op, px, ch); break;
        case TOC_OP_EXPONENT: k_exponent(op, px, ch); break;
        case TOC_OP_EXP_LINEAR: k_exp_linear(op, px, ch); break;
        case TOC_OP_LOG: k_log(op, px, ch); break;
        case TOC_OP_LOG_CAMERA: k_logcam(op, px, ch); break;
        case TOC_OP_CDL: k_cdl(op, px, ch); break;
        case TOC_OP_LUT1D: toc_lut1d_apply_pixel(op, px, ch); break;
        case TOC_OP_LUT3D: toc_lut3d_apply_pixel(op, px, ch); break;
        case TOC_OP_FIXEDFUNC: toc_fixedfunc_apply_pixel(op, px, ch); break;
        case TOC_OP_ACES_OUTPUT: toc_aces2_apply_pixel(op, px, ch); break;
        case TOC_OP_NOOP:
        default: break;
    }
}

/* Apply one op to the whole buffer (op-major; enables SIMD batch kernels). */
static void batch_op(const toc_op *op, float *rgba, size_t npix, int ch) {
    size_t i;
    switch (op->kind) {
        case TOC_OP_MATRIX:
            toc_simd.matrix(op->u.matrix.m, op->u.matrix.off, rgba, npix, ch);
            return;
        case TOC_OP_RANGE:
            toc_simd.range(op->u.range.scale, op->u.range.offset,
                           op->u.range.min, op->u.range.max,
                           op->u.range.clamp_lo, op->u.range.clamp_hi, rgba,
                           npix, ch);
            return;
        /* mirror (odd-extension) exponent/MonCurve isn't in the SIMD kernels;
         * fall through to the per-pixel scalar path below. */
        case TOC_OP_EXPONENT:
            if (op->u.exponent.mirror) break;
            toc_simd.exponent(op, rgba, npix, ch); return;
        case TOC_OP_LOG: toc_simd.log_(op, rgba, npix, ch); return;
        case TOC_OP_LOG_CAMERA: toc_simd.log_camera(op, rgba, npix, ch); return;
        case TOC_OP_EXP_LINEAR:
            if (op->u.exp_linear.mirror) break;
            toc_simd.exp_linear(op, rgba, npix, ch); return;
        case TOC_OP_CDL: toc_simd.cdl(op, rgba, npix, ch); return;
        default: break;
    }
    for (i = 0; i < npix; ++i) {
        float *px = rgba + i * (size_t)ch;
        switch (op->kind) {
            case TOC_OP_LUT1D: toc_lut1d_apply_pixel(op, px, ch); break;
            case TOC_OP_LUT3D: toc_lut3d_apply_pixel(op, px, ch); break;
            case TOC_OP_FIXEDFUNC: toc_fixedfunc_apply_pixel(op, px, ch); break;
            case TOC_OP_ACES_OUTPUT: toc_aces2_apply_pixel(op, px, ch); break;
            case TOC_OP_EXPONENT: k_exponent(op, px, ch); break; /* mirror path */
            case TOC_OP_EXP_LINEAR: k_exp_linear(op, px, ch); break;
            case TOC_OP_NOOP:
            default: break;
        }
    }
}

toc_result toc_apply(const toc_op_list *ops, float *rgba, size_t pixel_count,
                     int channels) {
    size_t k;
    if (!ops || !rgba || (channels != 3 && channels != 4))
        return TOC_ERROR_INVALID_ARGUMENT;
    toc_simd_init();
    for (k = 0; k < ops->count; ++k)
        batch_op(&ops->ops[k], rgba, pixel_count, channels);
    return TOC_SUCCESS;
}
