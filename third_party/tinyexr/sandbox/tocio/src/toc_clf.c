/*
 * tocio - CLF (Common LUT Format) loader: ACES-standard XML LUT format.
 *
 * Parses the XML-based Common LUT Format (SMPTE ST 2065-4 / Academy standard)
 * into a flat op list.  Supports Matrix, Range, Exponent, Log, LUT1D, LUT3D,
 * and Reference process nodes.  Does NOT build a DOM tree — single-pass
 * streaming parser using a minimal XML subset reader.
 *
 * Reimplemented from the Academy CLF reference (BSD-3-Clause compatible).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

/* ============================================================================
 * Minimal XML subset reader
 *
 * Flat cursor-based parser — no tree.  CLF nesting never exceeds 2 levels so
 * this is sufficient.  Supports: <tag>, key="val", <!-- comment -->, <?xml?>,
 * self-close, </close>.
 * ========================================================================== */

typedef struct {
    const char *p, *end;
} xr;

/* skip whitespace + XML comments + processing instructions */
static void xs(xr *r) {
    for (;;) {
        while (r->p < r->end && (unsigned char)*r->p <= 0x20) ++r->p;
        /* <!-- ... --> */
        if (r->p + 3 < r->end && r->p[0] == '<' && r->p[1] == '!' &&
            r->p[2] == '-' && r->p[3] == '-') {
            r->p += 4;
            while (r->p + 2 < r->end &&
                   !(r->p[0] == '-' && r->p[1] == '-' && r->p[2] == '>'))
                ++r->p;
            if (r->p + 2 < r->end) r->p += 3;
            continue;
        }
        /* <? ... ?> */
        if (r->p + 1 < r->end && r->p[0] == '<' && r->p[1] == '?') {
            r->p += 2;
            while (r->p < r->end && !(r->p[0] == '?' && r->p[1] == '>'))
                ++r->p;
            if (r->p < r->end) r->p += 2;
            continue;
        }
        break;
    }
}

/* peek: is next char '<' after skipping? */
static int xat(xr *r) {
    xs(r);
    return r->p < r->end && *r->p == '<';
}

/* read tag name at current position (just after '<').  returns 1 for opening
 * tag, 0 for closing tag or no tag.  On success, buf gets the name and r->p
 * points just past the name (before attrs).  On closing tag, buf="/" and
 * r->p points at the name chars (after '/'). */
static int xtag(xr *r, char *buf, int sz) {
    int n = 0;
    if (!xat(r)) return 0;
    ++r->p; /* skip '<' */
    if (r->p >= r->end) return 0;
    if (*r->p == '/') {
        /* closing tag: consume '/' and read name into buf but return 0 */
        ++r->p;
        while (r->p < r->end && *r->p > 0x20 && *r->p != '>' && n < sz - 1)
            buf[n++] = *r->p++;
        buf[n] = 0;
        return 0;
    }
    while (r->p < r->end && *r->p > 0x20 && *r->p != '>' && *r->p != '/' &&
           n < sz - 1)
        buf[n++] = *r->p++;
    buf[n] = 0;
    return n > 0;
}

/* read next attribute key="val".  returns 1 if found, 0 before '>'/'/>'. */
static int xa(xr *r, char *key, int ksz, char *val, int vsz) {
    int n = 0;
    xs(r);
    if (r->p >= r->end || *r->p == '>' || *r->p == '/') return 0;
    while (r->p < r->end && *r->p != '=' && *r->p > 0x20 && n < ksz - 1)
        key[n++] = *r->p++;
    key[n] = 0;
    if (r->p < r->end && *r->p == '=') ++r->p;
    xs(r);
    if (r->p < r->end && (*r->p == '"' || *r->p == '\'')) {
        char q = *r->p++;
        n = 0;
        while (r->p < r->end && *r->p != q && n < vsz - 1) val[n++] = *r->p++;
        val[n] = 0;
        if (r->p < r->end) ++r->p;
    }
    return 1;
}

/* skip attributes and consume '>' (or '/>').  returns 1 if self-closing. */
static int xc(xr *r) {
    int selfc = 0;
    xs(r);
    while (r->p < r->end && *r->p != '>') {
        if (*r->p == '/') selfc = 1;
        ++r->p;
    }
    if (r->p < r->end) ++r->p; /* consume '>' */
    return selfc;
}

/* read text content (up to next '<').  sets *start / *end to span within source.
 * returns 1 if non-empty content exists. */
static int xt(xr *r, const char **start, const char **end) {
    xs(r);
    *start = r->p;
    while (r->p < r->end && *r->p != '<') ++r->p;
    *end = r->p;
    return *start < *end;
}

/* skip forward past the closing tag that follows (the `<` was already consumed
 * by xtag, positioning r->p at the name chars after `/`).  Returns 1 on
 * success, 0 if '>' was not found (truncated). */
static int xskip_close(xr *r) {
    while (r->p < r->end && *r->p != '>') ++r->p;
    if (r->p >= r->end) return 0;
    ++r->p;
    return 1;
}

/* read space-separated floats from text content into arr; returns count. */
static int xfloats(xr *r, float *arr, int max) {
    const char *s, *e;
    int n = 0;
    if (!xt(r, &s, &e)) return 0;
    while (s < e && n < max) {
        float v;
        if (!toc_parse_float(&s, e, &v)) break;
        arr[n++] = v;
    }
    return n;
}

/* ============================================================================
 * CLF process-node handlers.
 * Each is called with position just after > of the opening <tag> (attributes
 * consumed).  The handler reads children, creates an op, and returns with
 * r->p past the closing </tag>.
 * ========================================================================== */

/* --- <Matrix> ------------------------------------------------------------- */
static toc_result clf_matrix(toc_op_list *list, xr *r) {
    float m[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    float off[4] = {0,0,0,0};
    char tag[64];
    while (xtag(r, tag, sizeof(tag))) {
        if (strcmp(tag, "Array") == 0) {
            char key[64], val[64];
            while (xa(r, key, sizeof(key), val, sizeof(val))) {}
            xc(r); /* consume > */
            {
                float buf[32];
                int nf = xfloats(r, buf, 20);
                if (nf == 9) {
                    memset(m, 0, sizeof(m));
                    for (int i = 0; i < 3; ++i)
                        for (int j = 0; j < 3; ++j)
                            m[j * 4 + i] = buf[i * 3 + j];
                } else if (nf == 20) {
                    for (int i = 0; i < 4; ++i) {
                        for (int j = 0; j < 4; ++j)
                            m[j * 4 + i] = buf[i * 5 + j];
                        off[i] = buf[i * 5 + 4];
                    }
                } else if (nf >= 16) {
                    memcpy(m, buf, sizeof(m));
                }
            }
            /* skip </Array> */
            if (xat(r)) { ++r->p; if (r->p < r->end && *r->p == '/') xskip_close(r); }
        } else if (strcmp(tag, "Offset") == 0) {
            xc(r);
            xfloats(r, off, 4);
            if (xat(r)) { ++r->p; if (r->p < r->end && *r->p == '/') xskip_close(r); }
        } else {
            xc(r);
        }
    }
    /* consume </Matrix> (xtag already saw '<' + '/' + "Matrix") */
    if (!xskip_close(r)) return TOC_ERROR_PARSE;
    {
        toc_op *op = toc_op_list_push(list, TOC_OP_MATRIX);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        memcpy(op->u.matrix.m, m, sizeof(m));
        memcpy(op->u.matrix.off, off, sizeof(off));
    }
    return TOC_SUCCESS;
}

/* --- <Range> --------------------------------------------------------------- */
static toc_result clf_range(toc_op_list *list, xr *r) {
    float min_in[4] = {-1e30f,-1e30f,-1e30f,-1e30f};
    float max_in[4] = {1e30f,1e30f,1e30f,1e30f};
    float min_out[4] = {-1e30f,-1e30f,-1e30f,-1e30f};
    float max_out[4] = {1e30f,1e30f,1e30f,1e30f};
    char tag[64];
    while (xtag(r, tag, sizeof(tag))) {
        float v = 0.0f;
        char key[64], val[64];
        (void)xa(r, key, sizeof(key), val, sizeof(val));
        xc(r);
        {
            const char *s, *e;
            if (xt(r, &s, &e)) toc_parse_float(&s, e, &v);
        }
        if (xat(r)) { ++r->p; if (r->p < r->end && *r->p == '/') xskip_close(r); }
        if (strcmp(tag, "minInValue") == 0) {
            for (int i = 0; i < 4; ++i) { min_in[i] = v; max_in[i] = v; }
        } else if (strcmp(tag, "maxInValue") == 0) {
            for (int i = 0; i < 4; ++i) max_in[i] = v;
        } else if (strcmp(tag, "minOutValue") == 0) {
            for (int i = 0; i < 4; ++i) { min_out[i] = v; max_out[i] = v; }
        } else if (strcmp(tag, "maxOutValue") == 0) {
            for (int i = 0; i < 4; ++i) max_out[i] = v;
        }
    }
    if (!xskip_close(r)) return TOC_ERROR_PARSE;
    {
        toc_op *op = toc_op_list_push(list, TOC_OP_RANGE);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        for (int i = 0; i < 4; ++i) {
            float din = max_in[i] - min_in[i];
            float dout = max_out[i] - min_out[i];
            if (din > 0.0f) {
                op->u.range.scale[i] = dout / din;
                op->u.range.offset[i] = min_out[i] - min_in[i] * op->u.range.scale[i];
            } else {
                op->u.range.scale[i] = 1.0f;
                op->u.range.offset[i] = 0.0f;
            }
            op->u.range.min[i] = min_out[i];
            op->u.range.max[i] = max_out[i];
        }
        op->u.range.clamp_lo = 1;
        op->u.range.clamp_hi = 1;
    }
    return TOC_SUCCESS;
}

/* --- <Exponent> ------------------------------------------------------------ */
static toc_result clf_exponent(toc_op_list *list, xr *r) {
    float gamma[4] = {1,1,1,1};
    int invert = 0;
    char tag[64];
    while (xtag(r, tag, sizeof(tag))) {
        if (strcmp(tag, "ExponentParams") == 0) {
            char key[64], val[64];
            while (xa(r, key, sizeof(key), val, sizeof(val))) {
                if (strcmp(key, "style") == 0 && strcmp(val, "basicRev") == 0)
                    invert = 1;
            }
            xc(r);
            xfloats(r, gamma, 4);
            if (gamma[1] == 0.0f) gamma[1] = gamma[0];
            if (gamma[2] == 0.0f) gamma[2] = gamma[0];
            if (xat(r)) { ++r->p; if (r->p < r->end && *r->p == '/') xskip_close(r); }
        } else {
            xc(r);
        }
    }
    if (!xskip_close(r)) return TOC_ERROR_PARSE;
    {
        toc_op *op = toc_op_list_push(list, TOC_OP_EXPONENT);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        for (int i = 0; i < 4; ++i)
            op->u.exponent.e[i] = (invert && gamma[i] != 0.0f)
                                      ? 1.0f / gamma[i]
                                      : gamma[i];
    }
    return TOC_SUCCESS;
}

/* --- <Log> ----------------------------------------------------------------- */
static toc_result clf_log(toc_op_list *list, xr *r) {
    float base = 2.0f;
    float ls[3] = {1,1,1}, lo[3] = {0,0,0};
    float ns[3] = {1,1,1}, no[3] = {0,0,0};
    int invert = 0;
    char tag[64];
    while (xtag(r, tag, sizeof(tag))) {
        if (strcmp(tag, "LogParams") == 0) {
            char key[64], val[64];
            while (xa(r, key, sizeof(key), val, sizeof(val))) {
                if (strcmp(key, "base") == 0) {
                    const char *vp = val;
                    toc_parse_float(&vp, vp + strlen(vp), &base);
                } else if (strcmp(key, "style") == 0 &&
                           strcmp(val, "basicRev") == 0)
                    invert = 1;
            }
            xc(r);
            /* parse LogParams children */
            while (xtag(r, tag, sizeof(tag))) {
                char k2[64], v2[64];
                (void)xa(r, k2, sizeof(k2), v2, sizeof(v2));
                xc(r);
                if (strcmp(tag, "logSideSlope") == 0) xfloats(r, ls, 3);
                else if (strcmp(tag, "logSideOffset") == 0) xfloats(r, lo, 3);
                else if (strcmp(tag, "linSideSlope") == 0) xfloats(r, ns, 3);
                else if (strcmp(tag, "linSideOffset") == 0) xfloats(r, no, 3);
                if (xat(r)) { ++r->p; if (r->p < r->end && *r->p == '/')
                    xskip_close(r); }
            }
            xskip_close(r); /* </LogParams> */
        } else {
            xc(r);
        }
    }
    if (!xskip_close(r)) return TOC_ERROR_PARSE; /* </Log> */
    {
        toc_op *op = toc_op_list_push(list, TOC_OP_LOG);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.log.base = base;
        op->u.log.inverse = invert;
        for (int i = 0; i < 3; ++i) {
            op->u.log.log_slope[i] = ls[i];
            op->u.log.log_offset[i] = lo[i];
            op->u.log.lin_slope[i] = ns[i];
            op->u.log.lin_offset[i] = no[i];
        }
    }
    return TOC_SUCCESS;
}

/* --- <LUT1D> --------------------------------------------------------------- */
static toc_result clf_lut1d(toc_op_list *list, xr *r) {
    int length = 0, channels = 3;
    float *data = NULL;
    int half_domain = 0;
    char tag[64];
    while (xtag(r, tag, sizeof(tag))) {
        if (strcmp(tag, "Array") == 0) {
            char key[64], val[64];
            int dims[4] = {0,0,0,0}, nd = 0;
            while (xa(r, key, sizeof(key), val, sizeof(val))) {
                if (strcmp(key, "dim") == 0) {
                    const char *vp = val;
                    while (*vp && nd < 4) {
                        long d;
                        if (toc_parse_int(&vp, vp + strlen(vp), &d) && d > 0)
                            dims[nd++] = (int)d;
                        while (*vp == ' ') ++vp;
                    }
                }
            }
            xc(r);
            if (nd >= 2) {
                channels = dims[0];
                length = dims[1];
                if (channels < 1 || channels > 4 || length < 2 ||
                    length > 65536)
                    return TOC_ERROR_PARSE;
            }
            if (length < 2) return TOC_ERROR_PARSE;
            data = (float *)toc_malloc(&list->alloc,
                                       (size_t)length * 3 * sizeof(float));
            if (!data) return TOC_ERROR_OUT_OF_MEMORY;
            {
                float *tmp = (float *)toc_malloc(
                    &list->alloc, (size_t)length * channels * sizeof(float));
                int n;
                if (!tmp) { toc_free(&list->alloc, data); return TOC_ERROR_OUT_OF_MEMORY; }
                n = xfloats(r, tmp, length * channels);
                if (n < length) {
                    toc_free(&list->alloc, tmp);
                    toc_free(&list->alloc, data);
                    return TOC_ERROR_PARSE;
                }
                for (int i = 0; i < length; ++i) {
                    for (int c = 0; c < 3; ++c)
                        data[i * 3 + c] =
                            (c < channels) ? tmp[i * channels + c]
                                           : tmp[i * channels + channels - 1];
                }
                toc_free(&list->alloc, tmp);
            }
            if (xat(r)) { ++r->p; if (r->p < r->end && *r->p == '/')
                xskip_close(r); } /* </Array> */
        } else if (strcmp(tag, "HalfDomain") == 0) {
            xc(r);
            half_domain = 1;
            if (xat(r)) { ++r->p; if (r->p < r->end && *r->p == '/')
                xskip_close(r); }
        } else {
            xc(r);
        }
    }
    if (!xskip_close(r)) return TOC_ERROR_PARSE; /* </LUT1D> */
    if (!data) return TOC_ERROR_PARSE;
    if (!toc_op_list_own(list, data)) { toc_free(&list->alloc, data); return TOC_ERROR_OUT_OF_MEMORY; }
    {
        toc_op *op = toc_op_list_push(list, TOC_OP_LUT1D);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.lut1d.length = length;
        op->u.lut1d.channels = 3;
        op->u.lut1d.domain_min = half_domain ? -65504.0f : 0.0f;
        op->u.lut1d.domain_max = half_domain ? 65504.0f : 1.0f;
        op->u.lut1d.data = data;
        op->u.lut1d.interp = TOC_INTERP_LINEAR;
    }
    return TOC_SUCCESS;
}

/* --- <LUT3D> --------------------------------------------------------------- */
static toc_result clf_lut3d(toc_op_list *list, xr *r) {
    int size = 0;
    float *data = NULL;
    char tag[64];
    while (xtag(r, tag, sizeof(tag))) {
        if (strcmp(tag, "Array") == 0) {
            char key[64], val[64];
            int dims[4] = {0,0,0,0}, nd = 0;
            while (xa(r, key, sizeof(key), val, sizeof(val))) {
                if (strcmp(key, "dim") == 0) {
                    const char *vp = val;
                    while (*vp && nd < 4) {
                        long d;
                        if (toc_parse_int(&vp, vp + strlen(vp), &d) && d > 0)
                            dims[nd++] = (int)d;
                        while (*vp == ' ') ++vp;
                    }
                }
            }
            xc(r);
            if (nd < 4 || dims[0] != 3) return TOC_ERROR_UNSUPPORTED;
            size = dims[1];
            if (dims[2] != size || dims[3] != size || size < 2 || size > 256)
                return TOC_ERROR_PARSE;
            data = (float *)toc_malloc(&list->alloc,
                                       (size_t)size * size * size * 3 * sizeof(float));
            if (!data) return TOC_ERROR_OUT_OF_MEMORY;
            xfloats(r, data, size * size * size * 3);
            if (xat(r)) { ++r->p; if (r->p < r->end && *r->p == '/')
                xskip_close(r); }
        } else {
            xc(r);
        }
    }
    if (!xskip_close(r)) return TOC_ERROR_PARSE; /* </LUT3D> */
    if (!data) return TOC_ERROR_PARSE;
    if (!toc_op_list_own(list, data)) { toc_free(&list->alloc, data); return TOC_ERROR_OUT_OF_MEMORY; }
    {
        toc_op *op = toc_op_list_push(list, TOC_OP_LUT3D);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.lut3d.size = size;
        op->u.lut3d.domain_min[0] = op->u.lut3d.domain_min[1] =
            op->u.lut3d.domain_min[2] = 0.0f;
        op->u.lut3d.domain_max[0] = op->u.lut3d.domain_max[1] =
            op->u.lut3d.domain_max[2] = 1.0f;
        op->u.lut3d.data = data;
        op->u.lut3d.interp = TOC_INTERP_TETRAHEDRAL;
    }
    return TOC_SUCCESS;
}

/* ============================================================================
 * Top-level CLF entry point
 * ========================================================================== */

toc_result toc_load_clf(toc_op_list *list, const char *data, size_t len) {
    xr r;
    char tag[64], key[64], val[64];
    r.p = data;
    r.end = data + len;
    /* find <ProcessList> */
    while (xtag(&r, tag, sizeof(tag))) {
        if (strcmp(tag, "ProcessList") == 0) break;
        xc(&r); /* skip unknown top-level tags */
    }
    if (strcmp(tag, "ProcessList") != 0) return TOC_ERROR_PARSE;
    /* consume ProcessList attributes */
    while (xa(&r, key, sizeof(key), val, sizeof(val))) {}
    xc(&r); /* consume > */
    /* iterate process nodes */
    {
        size_t _maxit = len + 1;
        while (xtag(&r, tag, sizeof(tag)) && _maxit--) {
            toc_result rc = TOC_SUCCESS;
            if (strcmp(tag, "InputDescriptor") == 0 ||
                strcmp(tag, "OutputDescriptor") == 0 ||
                strcmp(tag, "Description") == 0) {
                xc(&r);
                if (xat(&r)) { ++r.p; if (r.p < r.end && *r.p == '/') xskip_close(&r); }
                continue;
            }
            if (strcmp(tag, "Matrix") == 0) { xc(&r); rc = clf_matrix(list, &r); }
            else if (strcmp(tag, "Range") == 0) { xc(&r); rc = clf_range(list, &r); }
            else if (strcmp(tag, "Exponent") == 0) { xc(&r); rc = clf_exponent(list, &r); }
            else if (strcmp(tag, "Log") == 0) { xc(&r); rc = clf_log(list, &r); }
            else if (strcmp(tag, "LUT1D") == 0) { xc(&r); rc = clf_lut1d(list, &r); }
            else if (strcmp(tag, "LUT3D") == 0) { xc(&r); rc = clf_lut3d(list, &r); }
            else {
                /* unknown node: skip entire element (content + closing tag) */
                char skip_tag[64];
                size_t stn = strlen(tag);
                if (stn >= sizeof(skip_tag)) stn = sizeof(skip_tag) - 1;
                memcpy(skip_tag, tag, stn);
                skip_tag[stn] = 0;
                int sc = xc(&r);
                if (!sc) {
                    size_t _maxit2 = (size_t)(r.end - r.p) + 1;
                    while (r.p < r.end) {
                        if (!_maxit2--) return TOC_ERROR_PARSE;
                        char ct[64];
                        if (xtag(&r, ct, sizeof(ct))) {
                            xc(&r);
                        } else {
                            if (strcmp(ct, skip_tag) == 0) {
                                if (r.p < r.end) ++r.p;
                                break;
                            }
                            if (r.p < r.end) ++r.p;
                        }
                    }
                }
            }
            if (!TOC_OK(rc)) return rc;
        }
    }
    return TOC_SUCCESS;
}
