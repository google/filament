/*
 * tocio - ARM64 NEON SIMD batch kernels for the interpreter (matrix, range).
 * Each reproduces its scalar reference using the same mul/add order (no FMA),
 * so the parity test matches.  NEON is mandatory on aarch64 so this file only
 * needs the preprocessor guard; no runtime CPUID required.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

#if defined(__aarch64__)

#include <arm_neon.h>

/* Match the scalar reference bit-for-bit: forbid the compiler from fusing the
 * separate vmulq/vaddq pairs below into vfmla (see toc_interp.c parity note).
 * FP_CONTRACT is OFF (enforced by -ffp-contract=no in the Makefile). */

/* matrix: same column-major layout as SSE2 and scalar.
 * Process one RGBA pixel per iteration using 128-bit NEON vectors. */
void toc_matrix_batch_neon(const float *m, const float *off, float *rgba,
                           size_t npix, int ch) {
    float32x4_t c0, c1, c2, c3, vo;
    size_t i;
    if (ch != 4) { toc_matrix_batch_scalar(m, off, rgba, npix, ch); return; }
    c0 = vld1q_f32(m + 0);
    c1 = vld1q_f32(m + 4);
    c2 = vld1q_f32(m + 8);
    c3 = vld1q_f32(m + 12);
    vo = vld1q_f32(off);
    for (i = 0; i < npix; ++i) {
        float *px = rgba + i * 4;
        float32x4_t r = vdupq_n_f32(px[0]);
        float32x4_t g = vdupq_n_f32(px[1]);
        float32x4_t b = vdupq_n_f32(px[2]);
        float32x4_t a = vdupq_n_f32(px[3]);
        float32x4_t v = vaddq_f32(vmulq_f32(c0, r), vmulq_f32(c1, g));
        v = vaddq_f32(v, vmulq_f32(c2, b));
        v = vaddq_f32(v, vmulq_f32(c3, a));
        v = vaddq_f32(v, vo);
        vst1q_f32(px, v);
    }
}

/* range: scale + offset, optional lo/hi clamp.
 * Process one RGBA pixel per iteration. */
void toc_range_batch_neon(const float *scale, const float *offset,
                          const float *vmin, const float *vmax, int clamp_lo,
                          int clamp_hi, float *rgba, size_t npix, int ch) {
    float32x4_t vs, vof, lo_v, hi_v;
    size_t i;
    if (ch != 4) {
        toc_range_batch_scalar(scale, offset, vmin, vmax, clamp_lo, clamp_hi,
                               rgba, npix, ch);
        return;
    }
    vs = vld1q_f32(scale);
    vof = vld1q_f32(offset);
    lo_v = vld1q_f32(vmin);
    hi_v = vld1q_f32(vmax);
    for (i = 0; i < npix; ++i) {
        float *px = rgba + i * 4;
        float32x4_t v = vaddq_f32(vmulq_f32(vld1q_f32(px), vs), vof);
        if (clamp_lo) v = vmaxq_f32(v, lo_v);
        if (clamp_hi) v = vminq_f32(v, hi_v);
        vst1q_f32(px, v);
    }
}

/* ----------------------------------------------------------------------------
 * Vectorized transcendentals: 4-lane NEON versions of toc_log2f / toc_exp2f /
 * toc_powf (toc_math.c), evaluated with the identical polynomial, constant order
 * and integer bit-twiddling as the scalar so each lane is bit-for-bit equal
 * (verified 0 diffs / 2M inputs). FP_CONTRACT is OFF (file top) so the mul/add
 * chains are not fused into FMA. Edge handling matches the scalar: exp2f clamps
 * x>127 -> +inf and x<-126 -> 0; log2f's x<=0 path is never reached because the
 * callers feed strictly-positive arguments (as the scalar kernels do).
 * ------------------------------------------------------------------------- */

static float toc_bits_to_f(uint32_t u) { float f; memcpy(&f, &u, sizeof(f)); return f; }

/* log2 for x>0 (the polynomial path; x<=0 lanes yield finite garbage that the
 * callers mask away, exactly as toc_powf does for its non-positive base). */
static float32x4_t toc_log2f4(float32x4_t x) {
    uint32x4_t u = vreinterpretq_u32_f32(x);
    int32x4_t e = vsubq_s32(
        vreinterpretq_s32_u32(vandq_u32(vshrq_n_u32(u, 23), vdupq_n_u32(0xffu))),
        vdupq_n_s32(127));
    float32x4_t m = vreinterpretq_f32_u32(vorrq_u32(
        vandq_u32(u, vdupq_n_u32(0x007fffffu)), vdupq_n_u32(0x3f800000u)));
    float32x4_t one = vdupq_n_f32(1.0f);
    float32x4_t t = vdivq_f32(vsubq_f32(m, one), vaddq_f32(m, one));
    float32x4_t t2 = vmulq_f32(t, t);
    float32x4_t l = vdupq_n_f32(1.0f / 9.0f);
    l = vaddq_f32(vdupq_n_f32(1.0f / 7.0f), vmulq_f32(t2, l));
    l = vaddq_f32(vdupq_n_f32(1.0f / 5.0f), vmulq_f32(t2, l));
    l = vaddq_f32(vdupq_n_f32(1.0f / 3.0f), vmulq_f32(t2, l));
    l = vaddq_f32(one, vmulq_f32(t2, l));
    {
        float32x4_t ln = vmulq_f32(vmulq_f32(vdupq_n_f32(2.0f), t), l);
        return vaddq_f32(vcvtq_f32_s32(e),
                         vmulq_f32(ln, vdupq_n_f32(1.4426950408889634f)));
    }
}

static float32x4_t toc_exp2f4(float32x4_t x) {
    uint32x4_t mhi = vcgtq_f32(x, vdupq_n_f32(127.0f));
    uint32x4_t mlo = vcltq_f32(x, vdupq_n_f32(-126.0f));
    float32x4_t bias = vbslq_f32(vcgeq_f32(x, vdupq_n_f32(0.0f)),
                                 vdupq_n_f32(0.5f), vdupq_n_f32(-0.5f));
    int32x4_t ki = vcvtq_s32_f32(vaddq_f32(x, bias)); /* (int)(x +/- 0.5) */
    float32x4_t k = vcvtq_f32_s32(ki);
    float32x4_t f = vsubq_f32(x, k);
    float32x4_t g = vmulq_f32(f, vdupq_n_f32(0.6931471805599453f));
    float32x4_t one = vdupq_n_f32(1.0f);
    float32x4_t p = vdupq_n_f32(1.0f / 720.0f);
    float32x4_t vf;
    p = vaddq_f32(vdupq_n_f32(1.0f / 120.0f), vmulq_f32(g, p));
    p = vaddq_f32(vdupq_n_f32(1.0f / 24.0f), vmulq_f32(g, p));
    p = vaddq_f32(vdupq_n_f32(1.0f / 6.0f), vmulq_f32(g, p));
    p = vaddq_f32(vdupq_n_f32(0.5f), vmulq_f32(g, p));
    p = vaddq_f32(one, vmulq_f32(g, p));
    p = vaddq_f32(one, vmulq_f32(g, p));
    vf = vreinterpretq_f32_s32(vshlq_n_s32(vaddq_s32(ki, vdupq_n_s32(127)), 23));
    {
        float32x4_t inf = vreinterpretq_f32_u32(vdupq_n_u32(0x7f800000u));
        float32x4_t r = vmulq_f32(p, vf);            /* polynomial path */
        r = vbslq_f32(mlo, vdupq_n_f32(0.0f), r);    /* x < -126 -> 0 */
        r = vbslq_f32(mhi, inf, r);                  /* x > 127   -> +inf */
        return r;
    }
}

/* toc_powf: x>0 -> exp2f(y*log2f(x)); x==0 -> y>0?0:(y==0?1:+inf); x<0 -> NaN. */
static float32x4_t toc_powf4(float32x4_t x, float32x4_t y) {
    float32x4_t zero = vdupq_n_f32(0.0f);
    uint32x4_t pos = vcgtq_f32(x, zero);
    uint32x4_t xeq0 = vceqq_f32(x, zero);
    uint32x4_t ypos = vcgtq_f32(y, zero);
    uint32x4_t yzero = vceqq_f32(y, zero);
    float32x4_t inf = vreinterpretq_f32_u32(vdupq_n_u32(0x7f800000u));
    float32x4_t nan = vreinterpretq_f32_u32(vdupq_n_u32(0x7fc00000u));
    float32x4_t gen = toc_exp2f4(vmulq_f32(y, toc_log2f4(x)));
    float32x4_t zc = vbslq_f32(ypos, zero, vbslq_f32(yzero, vdupq_n_f32(1.0f), inf));
    float32x4_t special = vbslq_f32(xeq0, zc, nan);
    return vbslq_f32(pos, gen, special);
}

/* exponent: per channel toc_powf(x>0?x:0, e[c]); all `ch` channels (alpha too). */
void toc_exponent_batch_neon(const toc_op *op, float *rgba, size_t npix, int ch) {
    float32x4_t e, zero, special;
    float sp[4];
    size_t i;
    int c;
    if (ch != 4) { toc_exponent_batch_scalar(op, rgba, npix, ch); return; }
    e = vld1q_f32(op->u.exponent.e);
    zero = vdupq_n_f32(0.0f);
    for (c = 0; c < 4; ++c) {
        float ec = op->u.exponent.e[c];
        sp[c] = (ec > 0.0f) ? 0.0f : (ec == 0.0f ? 1.0f : toc_bits_to_f(0x7f800000u));
    }
    special = vld1q_f32(sp);
    for (i = 0; i < npix; ++i) {
        float *px = rgba + i * 4;
        float32x4_t v = vld1q_f32(px);
        float32x4_t arg = vmaxq_f32(v, zero); /* x>0?x:0 */
        uint32x4_t posm = vcgtq_f32(arg, zero);
        float32x4_t gen = toc_exp2f4(vmulq_f32(e, toc_log2f4(arg)));
        vst1q_f32(px, vbslq_f32(posm, gen, special));
    }
}

/* log: lin->log = log_slope*log2(lin_slope*x+lin_offset)/log2(base)+log_offset;
 * inverse = (base^((x-log_offset)/log_slope) - lin_offset)/lin_slope. RGB only
 * (alpha preserved). Per-channel params are [3]; pad lane 3 to avoid over-read. */
void toc_log_batch_neon(const toc_op *op, float *rgba, size_t npix, int ch) {
    float ls[4], lo[4], gs[4], go[4];
    float32x4_t v_ls, v_lo, v_gs, v_go, v_lb, fltmin;
    size_t i;
    int inv = op->u.log.inverse;
    if (ch != 4) { toc_log_batch_scalar(op, rgba, npix, ch); return; }
    for (i = 0; i < 3; ++i) {
        ls[i] = op->u.log.lin_slope[i]; lo[i] = op->u.log.lin_offset[i];
        gs[i] = op->u.log.log_slope[i]; go[i] = op->u.log.log_offset[i];
    }
    ls[3] = lo[3] = gs[3] = go[3] = 0.0f;
    v_ls = vld1q_f32(ls); v_lo = vld1q_f32(lo);
    v_gs = vld1q_f32(gs); v_go = vld1q_f32(go);
    v_lb = vdupq_n_f32(toc_log2f(op->u.log.base));
    fltmin = vdupq_n_f32(1.1754944e-38f);
    for (i = 0; i < npix; ++i) {
        float *px = rgba + i * 4;
        float32x4_t v = vld1q_f32(px);
        float a3 = vgetq_lane_f32(v, 3);
        float32x4_t res;
        if (!inv) {
            float32x4_t a = vaddq_f32(vmulq_f32(v_ls, v), v_lo);
            float32x4_t ac = vbslq_f32(vcgtq_f32(a, vdupq_n_f32(0.0f)), a, fltmin);
            float32x4_t l = vdivq_f32(toc_log2f4(ac), v_lb);
            res = vaddq_f32(vmulq_f32(v_gs, l), v_go);
        } else {
            float32x4_t e = vdivq_f32(vsubq_f32(v, v_go), v_gs);
            float32x4_t p = toc_exp2f4(vmulq_f32(e, v_lb));
            res = vdivq_f32(vsubq_f32(p, v_lo), v_ls);
        }
        res = vsetq_lane_f32(a3, res, 3); /* preserve alpha */
        vst1q_f32(px, res);
    }
}

/* log_camera: log-affine with a linear segment below the breakpoint. RGB only;
 * branch selected per lane with vbslq. Per-channel params are [3] (padded). */
void toc_logcam_batch_neon(const toc_op *op, float *rgba, size_t npix, int ch) {
    float gs[4], go[4], ls[4], lo[4], lb_[4], lsl[4], lof[4];
    float32x4_t v_gs, v_go, v_ls, v_lo, v_lbk, v_lsl, v_lof, v_base, fltmin;
    size_t i;
    int inv = op->u.logcam.inverse, c;
    if (ch != 4) { toc_logcam_batch_scalar(op, rgba, npix, ch); return; }
    for (c = 0; c < 3; ++c) {
        gs[c] = op->u.logcam.log_slope[c];  go[c] = op->u.logcam.log_offset[c];
        ls[c] = op->u.logcam.lin_slope[c];  lo[c] = op->u.logcam.lin_offset[c];
        lb_[c] = op->u.logcam.lin_break[c];
        lsl[c] = op->u.logcam.linear_slope[c];
        lof[c] = op->u.logcam.linear_offset[c];
    }
    gs[3] = go[3] = ls[3] = lo[3] = lb_[3] = lsl[3] = lof[3] = 0.0f;
    v_gs = vld1q_f32(gs); v_go = vld1q_f32(go); v_ls = vld1q_f32(ls);
    v_lo = vld1q_f32(lo); v_lbk = vld1q_f32(lb_); v_lsl = vld1q_f32(lsl);
    v_lof = vld1q_f32(lof);
    v_base = vdupq_n_f32(toc_log2f(op->u.logcam.base));
    fltmin = vdupq_n_f32(1.1754944e-38f);
    for (i = 0; i < npix; ++i) {
        float *px = rgba + i * 4;
        float32x4_t v = vld1q_f32(px);
        float a3 = vgetq_lane_f32(v, 3);
        float32x4_t res;
        if (!inv) {
            float32x4_t lin = vaddq_f32(vmulq_f32(v_lsl, v), v_lof);
            float32x4_t a = vaddq_f32(vmulq_f32(v_ls, v), v_lo);
            float32x4_t ac = vbslq_f32(vcgtq_f32(a, vdupq_n_f32(0.0f)), a, fltmin);
            float32x4_t l = vdivq_f32(toc_log2f4(ac), v_base);
            float32x4_t lg = vaddq_f32(vmulq_f32(v_gs, l), v_go);
            res = vbslq_f32(vcleq_f32(v, v_lbk), lin, lg);
        } else {
            float32x4_t ybrk = vaddq_f32(vmulq_f32(v_lsl, v_lbk), v_lof);
            float32x4_t lin = vdivq_f32(vsubq_f32(v, v_lof), v_lsl);
            float32x4_t e = vdivq_f32(vsubq_f32(v, v_go), v_gs);
            float32x4_t p = toc_exp2f4(vmulq_f32(e, v_base));
            float32x4_t lg = vdivq_f32(vsubq_f32(p, v_lo), v_ls);
            res = vbslq_f32(vcleq_f32(v, ybrk), lin, lg);
        }
        res = vsetq_lane_f32(a3, res, 3);
        vst1q_f32(px, res);
    }
}

/* exp_linear (MonCurve): power above the breakpoint, linear below. RGB only;
 * per-channel params are [4]. */
void toc_explin_batch_neon(const toc_op *op, float *rgba, size_t npix, int ch) {
    float32x4_t sc, of, gm, bk, sl;
    size_t i;
    int inv = op->u.exp_linear.inverse;
    if (ch != 4) { toc_explin_batch_scalar(op, rgba, npix, ch); return; }
    sc = vld1q_f32(op->u.exp_linear.scale);
    of = vld1q_f32(op->u.exp_linear.offset);
    gm = vld1q_f32(op->u.exp_linear.gamma);
    bk = vld1q_f32(op->u.exp_linear.breakpoint);
    sl = vld1q_f32(op->u.exp_linear.slope);
    for (i = 0; i < npix; ++i) {
        float *px = rgba + i * 4;
        float32x4_t v = vld1q_f32(px);
        float a3 = vgetq_lane_f32(v, 3);
        float32x4_t res;
        if (!inv) {
            float32x4_t pw = toc_powf4(vaddq_f32(vmulq_f32(v, sc), of), gm);
            float32x4_t lin = vmulq_f32(v, sl);
            res = vbslq_f32(vcgtq_f32(v, bk), pw, lin);
        } else {
            float32x4_t ybrk = vmulq_f32(bk, sl);
            float32x4_t invg = vdivq_f32(vdupq_n_f32(1.0f), gm);
            float32x4_t pw = vdivq_f32(vsubq_f32(toc_powf4(v, invg), of), sc);
            float32x4_t lin = vdivq_f32(v, sl);
            res = vbslq_f32(vcgtq_f32(v, ybrk), pw, lin);
        }
        res = vsetq_lane_f32(a3, res, 3);
        vst1q_f32(px, res);
    }
}

/* CDL (ASC): (in*slope+offset)^power then saturation around luma. RGB only. The
 * luma is a 3-lane dot product done with the same left-to-right association as
 * the scalar so the result is bit-exact. */
void toc_cdl_batch_neon(const toc_op *op, float *rgba, size_t npix, int ch) {
    float spc[4], ofc[4], pwc[4];
    float32x4_t sc, of, pw, satv, lvec;
    float lr = op->u.cdl.luma[0], lg = op->u.cdl.luma[1], lb = op->u.cdl.luma[2];
    int clamp = op->u.cdl.clamp, c;
    size_t i;
    if (ch != 4 || op->u.cdl.inverse) { /* inverse: scalar k_cdl path */
        toc_cdl_batch_scalar(op, rgba, npix, ch);
        return;
    }
    if (lr == 0.0f && lg == 0.0f && lb == 0.0f) {
        lr = 0.2126f; lg = 0.7152f; lb = 0.0722f;
    }
    for (c = 0; c < 3; ++c) {
        spc[c] = op->u.cdl.slope[c]; ofc[c] = op->u.cdl.offset[c];
        pwc[c] = op->u.cdl.power[c];
    }
    spc[3] = ofc[3] = pwc[3] = 0.0f;
    sc = vld1q_f32(spc); of = vld1q_f32(ofc); pw = vld1q_f32(pwc);
    satv = vdupq_n_f32(op->u.cdl.saturation);
    { float lv[4]; lv[0] = lr; lv[1] = lg; lv[2] = lb; lv[3] = 0.0f;
      lvec = vld1q_f32(lv); }
    for (i = 0; i < npix; ++i) {
        float *px = rgba + i * 4;
        float32x4_t v = vld1q_f32(px);
        float a3 = vgetq_lane_f32(v, 3);
        float32x4_t x = vaddq_f32(vmulq_f32(v, sc), of);
        float32x4_t vv, prod, x2;
        float luma;
        if (clamp) x = vminq_f32(vmaxq_f32(x, vdupq_n_f32(0.0f)), vdupq_n_f32(1.0f));
        vv = vbslq_f32(vcgeq_f32(x, vdupq_n_f32(0.0f)), toc_powf4(x, pw), x);
        prod = vmulq_f32(vv, lvec);
        luma = vgetq_lane_f32(prod, 0) + vgetq_lane_f32(prod, 1); /* (p0+p1) */
        luma = luma + vgetq_lane_f32(prod, 2);                    /* +p2 */
        {
            float32x4_t lumav = vdupq_n_f32(luma);
            x2 = vaddq_f32(lumav, vmulq_f32(satv, vsubq_f32(vv, lumav)));
        }
        if (clamp) x2 = vminq_f32(vmaxq_f32(x2, vdupq_n_f32(0.0f)), vdupq_n_f32(1.0f));
        x2 = vsetq_lane_f32(a3, x2, 3);
        vst1q_f32(px, x2);
    }
}

#endif /* __aarch64__ */
