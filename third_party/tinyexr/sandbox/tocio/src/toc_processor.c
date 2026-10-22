/*
 * tocio - processor: config + endpoints -> flat op list. Composes a colorspace
 * conversion through the reference space, flattens GroupTransform, resolves
 * FileTransform via the file-reader hook, expands BuiltinTransform, and inverts
 * ops as required by direction.
 *
 * Reimplemented from OpenColorIO (BSD-3-Clause).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

/* ---- scalar / vector parameter reads ------------------------------------- */
static int scalar_to_float(const char *s, float *out) {
    const char *p = s, *end;
    if (!s) return 0;
    end = s + strlen(s);
    return toc_parse_float(&p, end, out);
}

/* Read up to maxn floats from a SEQ value, or broadcast a single SCALAR.
 * Returns the number written (0 if absent). */
static int read_vec(const toc_node *parent, const char *key, float *out,
                    int maxn) {
    const toc_node *v = toc_node_map_get(parent, key);
    int i;
    if (!v) return 0;
    if (v->kind == TOC_NODE_SEQ) {
        int n = (int)v->n_items < maxn ? (int)v->n_items : maxn;
        for (i = 0; i < n; ++i)
            if (!scalar_to_float(toc_node_scalar(v->items[i]), &out[i]))
                out[i] = 0.0f;
        return n;
    }
    if (v->kind == TOC_NODE_SCALAR) {
        float f = 0.0f;
        scalar_to_float(toc_node_scalar(v), &f);
        for (i = 0; i < maxn; ++i) out[i] = f;
        return 1;
    }
    return 0;
}

static int read_scalar(const toc_node *parent, const char *key, float *out) {
    const toc_node *v = toc_node_map_get(parent, key);
    if (!v || v->kind != TOC_NODE_SCALAR) return 0;
    return scalar_to_float(toc_node_scalar(v), out);
}

/* Log/LogCamera params are named with snake_case in OCIO YAML configs
 * (log_side_slope) but camelCase in CLF/CTF (logSideSlope). Accept either. */
static int read_vec2(const toc_node *parent, const char *snake, const char *camel,
                     float *out, int maxn) {
    int n = read_vec(parent, snake, out, maxn);
    return n ? n : read_vec(parent, camel, out, maxn);
}

static void matvec4(const float *m /*row-major*/, const float *v, float *out) {
    int r;
    for (r = 0; r < 4; ++r)
        out[r] = m[r * 4 + 0] * v[0] + m[r * 4 + 1] * v[1] +
                 m[r * 4 + 2] * v[2] + m[r * 4 + 3] * v[3];
}

/* ---- MonCurve (ExponentWithLinear) forward params from (gamma,offset) ----- */
static void moncurve_params(float gamma, float offset, float *scale, float *off,
                            float *g, float *brk, float *slope) {
    double G = gamma < 1.000001 ? 1.000001 : gamma;
    double O = offset < 1e-6 ? 1e-6 : offset;
    double a = (G - 1.0) / O;
    double b = O * G / ((G - 1.0) * (1.0 + O));
    *g = (float)G;
    *scale = (float)(1.0 / (1.0 + O));
    *off = (float)(O / (1.0 + O));
    *brk = (float)(O / (G - 1.0));
    *slope = (float)(a * toc_powf((float)b, (float)G));
}

/* ---- lower a primitive transform into an op ------------------------------ */
toc_result toc_lower_transform(const toc_config *cfg, toc_op_list *list,
                               const toc_node *node, int invert) {
    const char *tag = node->tag;
    toc_op *op;
    (void)cfg;
    if (!tag) return TOC_SUCCESS;

    if (strcmp(tag, "MatrixTransform") == 0) {
        float rm[16], off[4] = {0, 0, 0, 0};
        int i;
        float ident[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        if (read_vec(node, "matrix", rm, 16) != 16) memcpy(rm, ident, sizeof(rm));
        read_vec(node, "offset", off, 4);
        if (invert) {
            float inv[16], noff[4], tmp[4];
            if (!toc_inv4x4(rm, inv)) return TOC_ERROR_NONINVERTIBLE;
            matvec4(inv, off, tmp);
            for (i = 0; i < 4; ++i) noff[i] = -tmp[i];
            memcpy(rm, inv, sizeof(rm));
            memcpy(off, noff, sizeof(off));
        }
        op = toc_op_list_push(list, TOC_OP_MATRIX);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        /* config matrix is row-major; interp uses col-major m[c*4+r] */
        for (i = 0; i < 4; ++i) {
            int j;
            for (j = 0; j < 4; ++j) op->u.matrix.m[j * 4 + i] = rm[i * 4 + j];
            op->u.matrix.off[i] = off[i];
        }
        return TOC_SUCCESS;
    }

    if (strcmp(tag, "ExponentTransform") == 0) {
        float e[4] = {1, 1, 1, 1};
        int i;
        read_vec(node, "value", e, 4);
        op = toc_op_list_push(list, TOC_OP_EXPONENT);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        for (i = 0; i < 4; ++i)
            op->u.exponent.e[i] = invert ? (e[i] != 0.0f ? 1.0f / e[i] : 1.0f)
                                         : e[i];
        return TOC_SUCCESS;
    }

    if (strcmp(tag, "ExponentWithLinearTransform") == 0) {
        float gamma[4] = {1, 1, 1, 1}, offset[4] = {0, 0, 0, 0};
        int i;
        read_vec(node, "gamma", gamma, 4);
        read_vec(node, "offset", offset, 4);
        op = toc_op_list_push(list, TOC_OP_EXP_LINEAR);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        for (i = 0; i < 4; ++i)
            moncurve_params(gamma[i], offset[i], &op->u.exp_linear.scale[i],
                            &op->u.exp_linear.offset[i],
                            &op->u.exp_linear.gamma[i],
                            &op->u.exp_linear.breakpoint[i],
                            &op->u.exp_linear.slope[i]);
        op->u.exp_linear.inverse = invert ? 1 : 0;
        return TOC_SUCCESS;
    }

    if (strcmp(tag, "LogTransform") == 0 ||
        strcmp(tag, "LogAffineTransform") == 0) {
        float base = 2.0f;
        float ls[3] = {1, 1, 1}, lo[3] = {0, 0, 0};
        float ns[3] = {1, 1, 1}, no[3] = {0, 0, 0};
        int i;
        read_scalar(node, "base", &base);
        read_vec2(node, "log_side_slope", "logSideSlope", ls, 3);
        read_vec2(node, "log_side_offset", "logSideOffset", lo, 3);
        read_vec2(node, "lin_side_slope", "linSideSlope", ns, 3);
        read_vec2(node, "lin_side_offset", "linSideOffset", no, 3);
        op = toc_op_list_push(list, TOC_OP_LOG);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.log.base = base;
        for (i = 0; i < 3; ++i) {
            op->u.log.log_slope[i] = ls[i];
            op->u.log.log_offset[i] = lo[i];
            op->u.log.lin_slope[i] = ns[i];
            op->u.log.lin_offset[i] = no[i];
        }
        /* LogTransform forward is lin->log; "direction: inverse" => log->lin */
        op->u.log.inverse = invert ? 1 : 0;
        return TOC_SUCCESS;
    }

    if (strcmp(tag, "LogCameraTransform") == 0) {
        float base = 2.0f;
        float ls[3] = {1, 1, 1}, lo[3] = {0, 0, 0};
        float ns[3] = {1, 1, 1}, no[3] = {0, 0, 0};
        float brk[3] = {0, 0, 0}, lslope[3];
        int has_lslope, i;
        read_scalar(node, "base", &base);
        read_vec2(node, "log_side_slope", "logSideSlope", ls, 3);
        read_vec2(node, "log_side_offset", "logSideOffset", lo, 3);
        read_vec2(node, "lin_side_slope", "linSideSlope", ns, 3);
        read_vec2(node, "lin_side_offset", "linSideOffset", no, 3);
        if (read_vec2(node, "lin_side_break", "linSideBreak", brk, 3) == 0)
            return TOC_ERROR_PARSE; /* required */
        has_lslope = read_vec2(node, "linear_slope", "linearSlope", lslope, 3) != 0;
        op = toc_op_list_push(list, TOC_OP_LOG_CAMERA);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.logcam.base = base;
        for (i = 0; i < 3; ++i) {
            float lnb = toc_log2f(base) * 0.6931471805599453f; /* ln(base) */
            float xb = ns[i] * brk[i] + no[i];
            float yb = ls[i] * (toc_log2f(xb > 0 ? xb : 1e-30f) / toc_log2f(base)) + lo[i];
            float lsl = has_lslope ? lslope[i]
                                   : (ls[i] * ns[i] / ((xb != 0 ? xb : 1e-30f) * lnb));
            op->u.logcam.log_slope[i] = ls[i];
            op->u.logcam.log_offset[i] = lo[i];
            op->u.logcam.lin_slope[i] = ns[i];
            op->u.logcam.lin_offset[i] = no[i];
            op->u.logcam.lin_break[i] = brk[i];
            op->u.logcam.linear_slope[i] = lsl;
            op->u.logcam.linear_offset[i] = yb - lsl * brk[i]; /* C0 continuity */
        }
        op->u.logcam.inverse = invert ? 1 : 0;
        return TOC_SUCCESS;
    }

    if (strcmp(tag, "CDLTransform") == 0) {
        float sl[3] = {1, 1, 1}, of[3] = {0, 0, 0}, pw[3] = {1, 1, 1};
        float sat = 1.0f;
        const toc_node *style;
        int i, clamp_val = 1;
        read_vec(node, "slope", sl, 3);
        read_vec(node, "offset", of, 3);
        read_vec(node, "power", pw, 3);
        read_scalar(node, "sat", &sat);
        style = toc_node_map_get(node, "style");
        if (style) {
            const char *s = toc_node_scalar(style);
            if (s && strcmp(s, "noclamp") == 0) clamp_val = 0;
        }
        if (invert) {
            /* Decompose CDL inverse into basic ops for backend-agnostic support.
             * Forward: slope*in+offset -> power -> saturation.
             * Inverse: inv_saturation -> inv_power -> inv_slope+offset. */
            float luma[3] = {0.2126f, 0.7152f, 0.0593f};
            float ny = 1.0f / sat;
            /* 1. Inverse saturation as a matrix:
             *    c' = c/s - ((1-s)/s)*broadcast(luma·c)
             *  => M[i][j] = delta(i,j)/s + (1-1/s)*luma[j]   (col-major). */
            {
                toc_op *m = toc_op_list_push(list, TOC_OP_MATRIX);
                if (!m) return TOC_ERROR_OUT_OF_MEMORY;
                memset(m->u.matrix.m, 0, sizeof(m->u.matrix.m));
                for (i = 0; i < 3; ++i) {
                    int j;
                    for (j = 0; j < 3; ++j)
                        m->u.matrix.m[j * 4 + i] =
                            (i == j ? ny : 0.0f) + (1.0f - ny) * luma[j];
                }
                m->u.matrix.m[15] = 1.0f;
                memset(m->u.matrix.off, 0, sizeof(m->u.matrix.off));
            }
            /* 2. Inverse power: pow(c, 1/power) */
            {
                toc_op *e = toc_op_list_push(list, TOC_OP_EXPONENT);
                if (!e) return TOC_ERROR_OUT_OF_MEMORY;
                for (i = 0; i < 3; ++i)
                    e->u.exponent.e[i] = (pw[i] != 0.0f) ? 1.0f / pw[i] : 1.0f;
                e->u.exponent.e[3] = 1.0f;
            }
            /* 3. Inverse (slope,offset): (c - offset) / slope = (1/slope)*c - offset/slope */
            {
                toc_op *r = toc_op_list_push(list, TOC_OP_RANGE);
                if (!r) return TOC_ERROR_OUT_OF_MEMORY;
                for (i = 0; i < 4; ++i) {
                    float s = (i < 3 && sl[i] != 0.0f) ? 1.0f / sl[i] : 1.0f;
                    r->u.range.scale[i] = s;
                    r->u.range.offset[i] = (i < 3) ? -of[i] * s : 0.0f;
                    r->u.range.min[i] = 0.0f;
                    r->u.range.max[i] = 1.0f;
                }
                r->u.range.clamp_lo = clamp_val;
                r->u.range.clamp_hi = clamp_val;
            }
            return TOC_SUCCESS;
        }
        op = toc_op_list_push(list, TOC_OP_CDL);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        for (i = 0; i < 3; ++i) {
            op->u.cdl.slope[i] = sl[i];
            op->u.cdl.offset[i] = of[i];
            op->u.cdl.power[i] = pw[i];
        }
        op->u.cdl.saturation = sat;
        op->u.cdl.clamp = clamp_val;
        return TOC_SUCCESS;
    }

    if (strcmp(tag, "RangeTransform") == 0) {
        float minIn, maxIn, minOut, maxOut, scale = 1.0f, offset = 0.0f;
        int hasIn = read_scalar(node, "minInValue", &minIn);
        int hasInH = read_scalar(node, "maxInValue", &maxIn);
        int hasOut = read_scalar(node, "minOutValue", &minOut);
        int hasOutH = read_scalar(node, "maxOutValue", &maxOut);
        int i;
        if (hasIn && hasInH && hasOut && hasOutH && (maxIn - minIn) != 0.0f) {
            scale = (maxOut - minOut) / (maxIn - minIn);
            offset = minOut - minIn * scale;
        }
        op = toc_op_list_push(list, TOC_OP_RANGE);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        for (i = 0; i < 4; ++i) {
            op->u.range.scale[i] = scale;
            op->u.range.offset[i] = offset;
            op->u.range.min[i] = hasOut ? minOut : 0.0f;
            op->u.range.max[i] = hasOutH ? maxOut : 1.0f;
        }
        op->u.range.clamp_lo = hasOut;
        op->u.range.clamp_hi = hasOutH;
        if (invert) return toc_invert_op(list, op);
        return TOC_SUCCESS;
    }

    return TOC_ERROR_UNSUPPORTED;
}

/* ---- invert an already-lowered op in place ------------------------------- */
/* Build the inverse of a monotonic (non-decreasing) LUT1D into newly-allocated
 * owned storage. Per channel f maps the input domain to [omin_c, omax_c]; the
 * inverse is sampled on a common output domain [min omin_c, max omax_c] and, for
 * each target, the input is found by binary search + linear interpolation on the
 * forward curve. Non-monotonic or degenerate curves are not invertible. */
static toc_result lut1d_invert(toc_op_list *list, toc_lut1d *lut) {
    int N = lut->length, ch = lut->channels, c, j;
    const float *fwd = lut->data;
    float dmin = lut->domain_min, dmax = lut->domain_max;
    float omin = 1e30f, omax = -1e30f, *inv;
    if (!list || N < 2 || (ch != 1 && ch != 3) || !fwd)
        return TOC_ERROR_NONINVERTIBLE;
    for (c = 0; c < ch; ++c) {
        int i;
        for (i = 0; i + 1 < N; ++i)
            if (fwd[(i + 1) * ch + c] < fwd[i * ch + c])
                return TOC_ERROR_NONINVERTIBLE; /* not non-decreasing */
        if (fwd[c] < omin) omin = fwd[c];
        if (fwd[(N - 1) * ch + c] > omax) omax = fwd[(N - 1) * ch + c];
    }
    if (!(omax > omin)) return TOC_ERROR_NONINVERTIBLE; /* flat / degenerate */
    inv = (float *)toc_malloc(&list->alloc, (size_t)N * ch * sizeof(float));
    if (!inv) return TOC_ERROR_OUT_OF_MEMORY;
    for (c = 0; c < ch; ++c) {
        for (j = 0; j < N; ++j) {
            float t = omin + (omax - omin) * (float)j / (float)(N - 1);
            float x;
            if (t <= fwd[c]) {
                x = dmin;
            } else if (t >= fwd[(N - 1) * ch + c]) {
                x = dmax;
            } else {
                int lo = 0, hi = N - 1;
                float y0, y1, x0, x1, frac;
                while (hi - lo > 1) {
                    int mid = (lo + hi) / 2;
                    if (fwd[mid * ch + c] <= t) lo = mid; else hi = mid;
                }
                y0 = fwd[lo * ch + c]; y1 = fwd[hi * ch + c];
                x0 = dmin + (dmax - dmin) * (float)lo / (float)(N - 1);
                x1 = dmin + (dmax - dmin) * (float)hi / (float)(N - 1);
                frac = (y1 != y0) ? (t - y0) / (y1 - y0) : 0.0f;
                x = x0 + frac * (x1 - x0);
            }
            inv[j * ch + c] = x;
        }
    }
    if (!toc_op_list_own(list, inv)) {
        toc_free(&list->alloc, inv);
        return TOC_ERROR_OUT_OF_MEMORY;
    }
    lut->data = inv;
    lut->domain_min = omin;
    lut->domain_max = omax;
    lut->interp = TOC_INTERP_LINEAR;
    return TOC_SUCCESS;
}

/* Evaluate the forward LUT3D at an arbitrary input (reads the original op). */
static void lut3d_eval(const toc_op *op, const float in[3], float out[3]) {
    float p[4];
    p[0] = in[0]; p[1] = in[1]; p[2] = in[2]; p[3] = 1.0f;
    toc_lut3d_apply_pixel(op, p, 3);
    out[0] = p[0]; out[1] = p[1]; out[2] = p[2];
}

/* Build the inverse of a LUT3D by, for each point of a regular grid over the
 * forward output box, solving f(x)=y: a coarse nearest-output search seeds
 * Newton's method (finite-difference Jacobian, solved via toc_inv4x4). The
 * result samples the inverse map and is stored as a new owned LUT3D whose domain
 * is the forward output box. Targets outside the achievable gamut converge to
 * the nearest reachable input. Capped at size 64 (the search is O(M^3*N) and
 * runs once at processor-build time). */
static toc_result lut3d_invert(toc_op_list *list, toc_op *op) {
    toc_lut3d *L = &op->u.lut3d;
    int N = L->size, M, Kc, ncand, k, it;
    float idmin[3], idmax[3], omin[3], omax[3], eps[3];
    float *cand_out = NULL, *cand_in = NULL, *inv = NULL;
    if (!list || N < 2 || N > 64 || !L->data) return TOC_ERROR_NONINVERTIBLE;
    M = N;
    for (k = 0; k < 3; ++k) {
        idmin[k] = L->domain_min[k];
        idmax[k] = L->domain_max[k];
        omin[k] = 1e30f;
        omax[k] = -1e30f;
        if (idmax[k] == idmin[k]) return TOC_ERROR_NONINVERTIBLE;
    }
    {   /* forward output bounding box */
        size_t cnt = (size_t)N * N * N, i;
        for (i = 0; i < cnt; ++i)
            for (k = 0; k < 3; ++k) {
                float v = L->data[i * 3 + k];
                if (v < omin[k]) omin[k] = v;
                if (v > omax[k]) omax[k] = v;
            }
    }
    for (k = 0; k < 3; ++k)
        if (!(omax[k] > omin[k])) return TOC_ERROR_NONINVERTIBLE;

    Kc = N < 16 ? N : 16; /* coarse seed grid for the initial guess */
    ncand = Kc * Kc * Kc;
    cand_out = (float *)toc_malloc(&list->alloc, (size_t)ncand * 3 * sizeof(float));
    cand_in = (float *)toc_malloc(&list->alloc, (size_t)ncand * 3 * sizeof(float));
    inv = (float *)toc_malloc(&list->alloc, (size_t)M * M * M * 3 * sizeof(float));
    if (!cand_out || !cand_in || !inv) {
        toc_free(&list->alloc, cand_out);
        toc_free(&list->alloc, cand_in);
        toc_free(&list->alloc, inv);
        return TOC_ERROR_OUT_OF_MEMORY;
    }
    {   /* sample the forward map on the coarse seed grid */
        int a, b, c, q = 0;
        for (c = 0; c < Kc; ++c)
            for (b = 0; b < Kc; ++b)
                for (a = 0; a < Kc; ++a, ++q) {
                    float in[3];
                    in[0] = idmin[0] + (idmax[0] - idmin[0]) * (float)a / (float)(Kc - 1);
                    in[1] = idmin[1] + (idmax[1] - idmin[1]) * (float)b / (float)(Kc - 1);
                    in[2] = idmin[2] + (idmax[2] - idmin[2]) * (float)c / (float)(Kc - 1);
                    cand_in[q * 3] = in[0]; cand_in[q * 3 + 1] = in[1]; cand_in[q * 3 + 2] = in[2];
                    lut3d_eval(op, in, &cand_out[q * 3]);
                }
    }
    for (k = 0; k < 3; ++k)
        eps[k] = (idmax[k] - idmin[k]) / (float)(N - 1) * 0.5f;

    {   /* solve f(x)=y for every inverse grid point */
        int gr, gg, gb, gi = 0, ci;
        for (gb = 0; gb < M; ++gb)
            for (gg = 0; gg < M; ++gg)
                for (gr = 0; gr < M; ++gr, ++gi) {
                    float y[3], x[3], bestx[3], bestr = 1e30f, bd = 1e30f;
                    int besti = 0;
                    y[0] = omin[0] + (omax[0] - omin[0]) * (float)gr / (float)(M - 1);
                    y[1] = omin[1] + (omax[1] - omin[1]) * (float)gg / (float)(M - 1);
                    y[2] = omin[2] + (omax[2] - omin[2]) * (float)gb / (float)(M - 1);
                    for (ci = 0; ci < ncand; ++ci) {
                        float ex = cand_out[ci * 3] - y[0];
                        float ey = cand_out[ci * 3 + 1] - y[1];
                        float ez = cand_out[ci * 3 + 2] - y[2];
                        float d = ex * ex + ey * ey + ez * ez;
                        if (d < bd) { bd = d; besti = ci; }
                    }
                    x[0] = cand_in[besti * 3]; x[1] = cand_in[besti * 3 + 1];
                    x[2] = cand_in[besti * 3 + 2];
                    bestx[0] = x[0]; bestx[1] = x[1]; bestx[2] = x[2];
                    for (it = 0; it < 12; ++it) {
                        float fx[3], r[3], rr, J[9], Mm[16], Mi[16], dxv[3];
                        lut3d_eval(op, x, fx);
                        r[0] = y[0] - fx[0]; r[1] = y[1] - fx[1]; r[2] = y[2] - fx[2];
                        rr = r[0] * r[0] + r[1] * r[1] + r[2] * r[2];
                        if (rr < bestr) {
                            bestr = rr;
                            bestx[0] = x[0]; bestx[1] = x[1]; bestx[2] = x[2];
                        }
                        if (rr < 1e-12f) break;
                        for (k = 0; k < 3; ++k) {
                            float xp[3], fp[3];
                            xp[0] = x[0]; xp[1] = x[1]; xp[2] = x[2];
                            xp[k] += eps[k];
                            lut3d_eval(op, xp, fp);
                            J[0 * 3 + k] = (fp[0] - fx[0]) / eps[k];
                            J[1 * 3 + k] = (fp[1] - fx[1]) / eps[k];
                            J[2 * 3 + k] = (fp[2] - fx[2]) / eps[k];
                        }
                        memset(Mm, 0, sizeof(Mm));
                        Mm[0] = J[0]; Mm[1] = J[1]; Mm[2] = J[2];
                        Mm[4] = J[3]; Mm[5] = J[4]; Mm[6] = J[5];
                        Mm[8] = J[6]; Mm[9] = J[7]; Mm[10] = J[8];
                        Mm[15] = 1.0f;
                        if (!toc_inv4x4(Mm, Mi)) break; /* singular: keep best */
                        dxv[0] = Mi[0] * r[0] + Mi[1] * r[1] + Mi[2] * r[2];
                        dxv[1] = Mi[4] * r[0] + Mi[5] * r[1] + Mi[6] * r[2];
                        dxv[2] = Mi[8] * r[0] + Mi[9] * r[1] + Mi[10] * r[2];
                        x[0] += dxv[0]; x[1] += dxv[1]; x[2] += dxv[2];
                        for (k = 0; k < 3; ++k) {
                            float lo = idmin[k] < idmax[k] ? idmin[k] : idmax[k];
                            float hi = idmin[k] < idmax[k] ? idmax[k] : idmin[k];
                            if (x[k] < lo) x[k] = lo;
                            if (x[k] > hi) x[k] = hi;
                        }
                    }
                    inv[gi * 3] = bestx[0]; inv[gi * 3 + 1] = bestx[1];
                    inv[gi * 3 + 2] = bestx[2];
                }
    }
    toc_free(&list->alloc, cand_out);
    toc_free(&list->alloc, cand_in);
    if (!toc_op_list_own(list, inv)) {
        toc_free(&list->alloc, inv);
        return TOC_ERROR_OUT_OF_MEMORY;
    }
    L->data = inv;
    L->size = M;
    for (k = 0; k < 3; ++k) { L->domain_min[k] = omin[k]; L->domain_max[k] = omax[k]; }
    return TOC_SUCCESS;
}

toc_result toc_invert_op(toc_op_list *list, toc_op *op) {
    int i;
    switch (op->kind) {
        case TOC_OP_RANGE: {
            float ns[4], no[4];
            for (i = 0; i < 4; ++i) {
                float s = op->u.range.scale[i];
                ns[i] = (s != 0.0f) ? 1.0f / s : 1.0f;
                no[i] = -op->u.range.offset[i] * ns[i];
            }
            for (i = 0; i < 4; ++i) {
                float lo = op->u.range.min[i], hi = op->u.range.max[i];
                /* output bounds become input bounds; recompute via new map */
                op->u.range.scale[i] = ns[i];
                op->u.range.offset[i] = no[i];
                op->u.range.min[i] = lo; /* clamp range is symmetric for [0,1] */
                op->u.range.max[i] = hi;
            }
            { int t = op->u.range.clamp_lo; op->u.range.clamp_lo =
                  op->u.range.clamp_hi; op->u.range.clamp_hi = t; }
            return TOC_SUCCESS;
        }
        case TOC_OP_EXPONENT:
            for (i = 0; i < 4; ++i)
                op->u.exponent.e[i] =
                    op->u.exponent.e[i] != 0.0f ? 1.0f / op->u.exponent.e[i]
                                                : 1.0f;
            return TOC_SUCCESS;
        case TOC_OP_EXP_LINEAR:
            op->u.exp_linear.inverse = !op->u.exp_linear.inverse;
            return TOC_SUCCESS;
        case TOC_OP_LOG:
            op->u.log.inverse = !op->u.log.inverse;
            return TOC_SUCCESS;
        case TOC_OP_LOG_CAMERA:
            op->u.logcam.inverse = !op->u.logcam.inverse;
            return TOC_SUCCESS;
        case TOC_OP_MATRIX: {
            /* m is stored col-major; invert via row-major round-trip */
            float rm[16], inv[16], off[4], tmp[4];
            int r, c;
            for (r = 0; r < 4; ++r)
                for (c = 0; c < 4; ++c) rm[r * 4 + c] = op->u.matrix.m[c * 4 + r];
            if (!toc_inv4x4(rm, inv)) return TOC_ERROR_NONINVERTIBLE;
            for (i = 0; i < 4; ++i) off[i] = op->u.matrix.off[i];
            matvec4(inv, off, tmp);
            for (r = 0; r < 4; ++r) {
                for (c = 0; c < 4; ++c)
                    op->u.matrix.m[c * 4 + r] = inv[r * 4 + c];
                op->u.matrix.off[r] = -tmp[r];
            }
            return TOC_SUCCESS;
        }
        case TOC_OP_FIXEDFUNC:
            /* every style is an even/odd forward/inverse pair, so toggling the
             * low bit selects the inverse (PQ<->lin, HSV<->RGB, glow fwd<->inv). */
            op->u.fixedfunc.style ^= 1;
            return TOC_SUCCESS;
        case TOC_OP_CDL:
            /* ASC CDL is analytically invertible (saturation preserves luma). */
            op->u.cdl.inverse = !op->u.cdl.inverse;
            return TOC_SUCCESS;
        case TOC_OP_LUT1D:
            return lut1d_invert(list, &op->u.lut1d);
        case TOC_OP_LUT3D:
            return lut3d_invert(list, op);
        default:
            return TOC_ERROR_NONINVERTIBLE;
    }
}

/* ---- recursive walk ------------------------------------------------------ */
static toc_result emit_to_ref(const toc_config *, toc_op_list *, const char *,
                              int);
static toc_result emit_from_ref(const toc_config *, toc_op_list *, const char *,
                                int);

static toc_result walk(const toc_config *cfg, toc_op_list *list,
                       const toc_node *node, int invert) {
    const char *tag;
    const toc_node *dir;
    int dir_inv = invert;
    toc_result rc;
    if (!node) return TOC_SUCCESS;
    tag = node->tag;
    dir = toc_node_map_get(node, "direction");
    if (dir) {
        const char *ds = toc_node_scalar(dir);
        if (ds && strcmp(ds, "inverse") == 0) dir_inv = !dir_inv;
    }
    if (!tag) return TOC_SUCCESS;

    if (strcmp(tag, "GroupTransform") == 0) {
        const toc_node *ch = toc_node_map_get(node, "children");
        size_t i;
        if (!ch || ch->kind != TOC_NODE_SEQ) return TOC_SUCCESS;
        if (!dir_inv) {
            for (i = 0; i < ch->n_items; ++i) {
                rc = walk(cfg, list, ch->items[i], 0);
                if (!TOC_OK(rc)) return rc;
            }
        } else {
            for (i = ch->n_items; i-- > 0;) {
                rc = walk(cfg, list, ch->items[i], 1);
                if (!TOC_OK(rc)) return rc;
            }
        }
        return TOC_SUCCESS;
    }
    if (strcmp(tag, "ColorSpaceTransform") == 0) {
        const char *src = toc_node_scalar(toc_node_map_get(node, "src"));
        const char *dst = toc_node_scalar(toc_node_map_get(node, "dst"));
        if (!src || !dst) return TOC_ERROR_PARSE;
        if (dir_inv) { const char *t = src; src = dst; dst = t; }
        rc = emit_to_ref(cfg, list, src, 0);
        if (!TOC_OK(rc)) return rc;
        return emit_from_ref(cfg, list, dst, 0);
    }
    if (strcmp(tag, "FileTransform") == 0) {
        const char *src = toc_node_scalar(toc_node_map_get(node, "src"));
        char *data = NULL;
        size_t len = 0;
        if (!src) return TOC_ERROR_PARSE;
        if (!cfg->reader) return TOC_ERROR_IO;
        rc = cfg->reader(cfg->reader_user, src, &cfg->alloc, &data, &len);
        if (!TOC_OK(rc)) return rc;
        rc = toc_load_lutfile(list, src, data, len, dir_inv);
        toc_free(&cfg->alloc, data);
        return rc;
    }
    if (strcmp(tag, "BuiltinTransform") == 0) {
        const char *style = toc_node_scalar(toc_node_map_get(node, "style"));
        if (!style) return TOC_ERROR_PARSE;
        return toc_builtin_expand(list, style, dir_inv);
    }
    if (strcmp(tag, "FixedFunctionTransform") == 0) {
        return toc_lower_fixedfunc(list, node, dir_inv);
    }
    return toc_lower_transform(cfg, list, node, dir_inv);
}

static toc_result emit_to_ref(const toc_config *cfg, toc_op_list *list,
                              const char *cs_name, int invert) {
    const char *name = toc_cfg_resolve_role(cfg, cs_name);
    const toc_node *cs = toc_cfg_find_colorspace(cfg, name);
    const toc_node *t;
    int needinv = 0;
    if (!cs) return TOC_ERROR_NOT_FOUND;
    if (toc_cfg_is_data(cs)) return TOC_SUCCESS;
    t = toc_cfg_cs_transform(cs, 1, &needinv);
    if (!t) return TOC_SUCCESS;
    return walk(cfg, list, t, invert ^ needinv);
}

static toc_result emit_from_ref(const toc_config *cfg, toc_op_list *list,
                                const char *cs_name, int invert) {
    const char *name = toc_cfg_resolve_role(cfg, cs_name);
    const toc_node *cs = toc_cfg_find_colorspace(cfg, name);
    const toc_node *t;
    int needinv = 0;
    if (!cs) return TOC_ERROR_NOT_FOUND;
    if (toc_cfg_is_data(cs)) return TOC_SUCCESS;
    t = toc_cfg_cs_transform(cs, 0, &needinv);
    if (!t) return TOC_SUCCESS;
    return walk(cfg, list, t, invert ^ needinv);
}

/* A colorspace flagged `isdata: true` is not color-managed: OCIO passes data
 * colorspaces through unchanged, so ANY conversion touching one (as src or dst)
 * is the identity. */
static int cs_is_data(const toc_config *cfg, const char *name) {
    const toc_node *cs =
        toc_cfg_find_colorspace(cfg, toc_cfg_resolve_role(cfg, name));
    return cs && toc_cfg_is_data(cs);
}

/* Reference domain of a colorspace: 1 = display-referred, 0 = scene-referred.
 * OCIO v2 has two reference spaces; a plain colorspace<->colorspace conversion
 * stays within one. Bridging scene<->display requires a view transform (and the
 * interchange roles), which toc_processor_from_colorspaces does not model - so
 * such a request is reported UNSUPPORTED rather than silently mis-converted. */
static int cs_ref_domain(const toc_config *cfg, const char *name) {
    const char *rn = toc_cfg_resolve_role(cfg, name);
    const toc_node *cs = toc_cfg_find_colorspace(cfg, rn);
    if (!cs) return 0;
    if (toc_node_map_get(cs, "to_display_reference") ||
        toc_node_map_get(cs, "from_display_reference"))
        return 1;
    if (toc_node_map_get(cs, "to_scene_reference") ||
        toc_node_map_get(cs, "from_scene_reference") ||
        toc_node_map_get(cs, "to_reference") ||
        toc_node_map_get(cs, "from_reference"))
        return 0;
    /* No explicit transform (the colorspace *is* a reference): classify by the
     * section it was declared in. */
    return toc_cfg_cs_in_display_section(cfg, rn);
}

/* ---- public builders ----------------------------------------------------- */
static toc_op_list *new_list(const toc_allocator *a) {
    toc_op_list *l = (toc_op_list *)toc_malloc(a, sizeof(*l));
    if (!l) return NULL;
    memset(l, 0, sizeof(*l));
    l->alloc = *a;
    return l;
}

toc_result toc_processor_from_colorspaces(const toc_config *cfg, const char *src,
                                          const char *dst,
                                          const toc_allocator *a,
                                          toc_op_list **out) {
    toc_op_list *list;
    toc_result rc;
    if (!cfg || !src || !dst || !out) return TOC_ERROR_INVALID_ARGUMENT;
    if (!a) a = toc_default_allocator();
    *out = NULL;
    list = new_list(a);
    if (!list) return TOC_ERROR_OUT_OF_MEMORY;
    if (cs_is_data(cfg, src) || cs_is_data(cfg, dst)) {
        *out = list; /* data colorspace -> identity passthrough */
        return TOC_SUCCESS;
    }
    rc = emit_to_ref(cfg, list, src, 0); /* src -> its own (scene or display) ref */
    if (TOC_OK(rc) && cs_ref_domain(cfg, src) != cs_ref_domain(cfg, dst)) {
        /* Bridge scene<->display via the config's default view transform, the way
         * OCIO's getProcessor connects the two reference spaces. */
        const toc_node *vt = toc_cfg_default_view_transform(cfg);
        const toc_node *tf = NULL;
        int vt_inv = (cs_ref_domain(cfg, src) == 1); /* display src -> invert VT */
        if (vt) {
            tf = toc_node_map_get(vt, "from_scene_reference");
            if (!tf) tf = toc_node_map_get(vt, "from_reference");
            if (!tf) {
                tf = toc_node_map_get(vt, "to_scene_reference");
                if (!tf) tf = toc_node_map_get(vt, "to_reference");
                vt_inv = !vt_inv;
            }
        }
        if (!tf) rc = TOC_ERROR_UNSUPPORTED;
        else rc = walk(cfg, list, tf, vt_inv);
    }
    if (TOC_OK(rc)) rc = emit_from_ref(cfg, list, dst, 0);
    if (!TOC_OK(rc)) {
        toc_op_list_free(list);
        return rc;
    }
    *out = list;
    return TOC_SUCCESS;
}

toc_result toc_processor_from_display_view(const toc_config *cfg,
                                           const char *src_cs,
                                           const char *display, const char *view,
                                           const toc_allocator *a,
                                           toc_op_list **out) {
    const toc_node *d, *views, *vnode;
    const char *view_cs, *view_vt, *view_dc;
    size_t i;
    toc_op_list *list;
    toc_result rc;
    if (!cfg || !src_cs || !display || !view || !out)
        return TOC_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    d = toc_node_map_get(cfg->root, "displays");
    views = d ? toc_node_map_get(d, display) : NULL;
    if (!views || views->kind != TOC_NODE_SEQ) return TOC_ERROR_NOT_FOUND;
    vnode = NULL;
    for (i = 0; i < views->n_items && !vnode; ++i) {
        const toc_node *it = views->items[i];
        if (it->kind == TOC_NODE_SEQ) {
            /* `!<Views> [name, ...]` references views defined in shared_views. */
            size_t j;
            for (j = 0; j < it->n_items; ++j) {
                const char *nm = toc_node_scalar(it->items[j]);
                if (nm && strcmp(nm, view) == 0) {
                    const toc_node *sv = toc_node_map_get(cfg->root, "shared_views");
                    size_t k;
                    if (sv && sv->kind == TOC_NODE_SEQ)
                        for (k = 0; k < sv->n_items; ++k) {
                            const char *svn = toc_node_scalar(
                                toc_node_map_get(sv->items[k], "name"));
                            if (svn && strcmp(svn, view) == 0) {
                                vnode = sv->items[k];
                                break;
                            }
                        }
                    break;
                }
            }
        } else {
            const char *vn = toc_node_scalar(toc_node_map_get(it, "name"));
            if (vn && strcmp(vn, view) == 0) vnode = it;
        }
    }
    if (!vnode) return TOC_ERROR_NOT_FOUND;
    /* Determine view type, build pipeline.
     * Simple view:  src -> reference -> [looks] -> display_colorspace
     * VT view:      src -> reference -> [looks] -> view_transform -> [display_colorspace] */
    view_cs = toc_node_scalar(toc_node_map_get(vnode, "colorspace"));
    view_vt = toc_node_scalar(toc_node_map_get(vnode, "view_transform"));
    if (!view_cs && !view_vt) return TOC_ERROR_UNSUPPORTED;
    if (!a) a = toc_default_allocator();
    list = new_list(a);
    if (!list) return TOC_ERROR_OUT_OF_MEMORY;
    /* A view onto a data colorspace (e.g. "Raw"), or a data source, is an
     * identity passthrough - data is never color-managed. */
    if (cs_is_data(cfg, src_cs) || (view_cs && cs_is_data(cfg, view_cs))) {
        *out = list;
        return TOC_SUCCESS;
    }
    /* 1. src -> reference */
    rc = emit_to_ref(cfg, list, src_cs, 0);
    if (!TOC_OK(rc)) goto fail;
    /* 2. apply looks (comma-separated) in order */
    {
        const char *lk_names[8];
        int nlk = toc_cfg_view_looks(vnode, lk_names, 8);
        int li;
        for (li = 0; li < nlk; ++li) {
            const toc_node *lk = toc_cfg_find_look(cfg, lk_names[li]);
            const toc_node *tf, *ps;
            if (!lk) { rc = TOC_ERROR_NOT_FOUND; goto fail; }
            ps = toc_node_map_get(lk, "process_space");
            if (ps) {
                const char *ps_name = toc_node_scalar(ps);
                if (ps_name) {
                    rc = emit_from_ref(cfg, list, ps_name, 0);
                    if (!TOC_OK(rc)) goto fail;
                    tf = toc_node_map_get(lk, "transform");
                    if (tf) { rc = walk(cfg, list, tf, 0); if (!TOC_OK(rc)) goto fail; }
                    rc = emit_to_ref(cfg, list, ps_name, 0);
                    if (!TOC_OK(rc)) goto fail;
                    continue;
                }
            }
            tf = toc_node_map_get(lk, "transform");
            if (tf) { rc = walk(cfg, list, tf, 0); if (!TOC_OK(rc)) goto fail; }
        }
    }
    /* 3. apply view_transform (if present). OCIO v2 ViewTransforms convert the
     * scene reference to the display reference via from_scene_reference (or the
     * inverse of to_scene_reference); accept the legacy *_reference too. */
    if (view_vt) {
        const toc_node *vt = toc_cfg_find_view_transform(cfg, view_vt);
        const toc_node *tf;
        int vt_inv = 0;
        if (!vt) { rc = TOC_ERROR_NOT_FOUND; goto fail; }
        tf = toc_node_map_get(vt, "from_scene_reference");
        if (!tf) tf = toc_node_map_get(vt, "from_reference");
        if (!tf) {
            tf = toc_node_map_get(vt, "to_scene_reference");
            if (!tf) tf = toc_node_map_get(vt, "to_reference");
            vt_inv = 1;
        }
        if (tf) {
            rc = walk(cfg, list, tf, vt_inv);
        } else {
            /* This VT operates from the display reference (e.g. "Video
             * (colorimetric)"); bridge scene->display via the default VT, then
             * apply this VT's display-reference op. */
            const toc_node *dtf = toc_node_map_get(vt, "from_display_reference");
            const toc_node *dvt = toc_cfg_default_view_transform(cfg);
            const toc_node *btf = dvt ? toc_node_map_get(dvt, "from_scene_reference")
                                      : NULL;
            if (btf == NULL && dvt) btf = toc_node_map_get(dvt, "from_reference");
            if (!dtf || !btf) { rc = TOC_ERROR_UNSUPPORTED; goto fail; }
            rc = walk(cfg, list, btf, 0);             /* scene ref -> display ref */
            if (TOC_OK(rc)) rc = walk(cfg, list, dtf, 0); /* this VT (display ref) */
        }
        if (!TOC_OK(rc)) goto fail;
    }
    /* 4. display colorspace (view_cs for simple, display_colorspace for VT views).
     * The token <USE_DISPLAY_NAME> means "the display's own colorspace". */
    view_dc = view_cs ? view_cs
                      : toc_node_scalar(toc_node_map_get(vnode, "display_colorspace"));
    if (view_dc && strcmp(view_dc, "<USE_DISPLAY_NAME>") == 0) view_dc = display;
    if (view_dc) {
        rc = emit_from_ref(cfg, list, view_dc, 0);
        if (!TOC_OK(rc)) goto fail;
    }
    *out = list;
    return TOC_SUCCESS;
fail:
    toc_op_list_free(list);
    return rc;
}
