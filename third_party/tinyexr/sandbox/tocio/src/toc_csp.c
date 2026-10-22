/*
 * tocio - CSP (ColorSpace Process) LUT format loader.
 *
 * Parses the Autodesk ColorSpace Process LUT format (CSPLUTV1.0 / CSPLUT0001)
 * into flat ops: optional PRE_1D LUT1D, then LUT3D, then optional POST_1D LUT1D.
 *
 * Reimplemented from the OpenColorIO CSP file format (BSD-3-Clause).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"
#include <string.h>

/* ============================================================================
 * Minimal line-oriented parser (CSP is keyword + data lines)
 * ========================================================================== */

typedef struct {
    const char *p, *end;
} csp_cur;

static int csp_isspc(char c) { return c == ' ' || c == '\t' || c == '\r'; }

/* skip whitespace and #-comments */
static void csp_skip(csp_cur *c) {
    for (;;) {
        while (c->p < c->end && (csp_isspc(*c->p) || *c->p == '\n')) ++c->p;
        if (c->p < c->end && *c->p == '#') {
            while (c->p < c->end && *c->p != '\n') ++c->p;
            continue;
        }
        break;
    }
}

static int csp_peek(csp_cur *c) {
    csp_skip(c);
    return c->p < c->end;
}

/* returns 1 if the next non-whitespace starts a keyword (letter or '3D').
 * Covers both alphabetic keywords and the special "3D" section marker. */
static int csp_isword(csp_cur *c) {
    csp_skip(c);
    if (c->p >= c->end) return 0;
    if ((*c->p >= 'A' && *c->p <= 'Z') || (*c->p >= 'a' && *c->p <= 'z'))
        return 1;
    /* "3D" is a valid CSP keyword */
    if (c->p[0] == '3' && c->end - c->p >= 2 && c->p[1] == 'D') return 1;
    return 0;
}

static int csp_word(csp_cur *c, char *buf, int sz) {
    int n = 0;
    csp_skip(c);
    while (c->p < c->end && !csp_isspc(*c->p) && *c->p != '\n' && n < sz - 1)
        buf[n++] = *c->p++;
    buf[n] = 0;
    return n;
}

static int csp_float(csp_cur *c, float *out) {
    csp_skip(c);
    return toc_parse_float(&c->p, c->end, out);
}

static int csp_int(csp_cur *c, long *out) {
    csp_skip(c);
    return toc_parse_int(&c->p, c->end, out);
}

static int csp_kweq(const char *w, const char *kw) { return strcmp(w, kw) == 0; }

/* ============================================================================
 * CSP loader
 *
 * The sections may appear in any order.  We parse everything into local
 * buffers, then create ops in the correct order (PRE_1D -> 3D -> POST_1D).
 * ========================================================================== */

toc_result toc_load_csp(toc_op_list *list, const char *data, size_t len) {
    csp_cur c;
    int mesh = 0, pre_len = 0, post_len = 0;
    float *pre_data = NULL, *post_data = NULL, *lut3d = NULL;
    long got_pre = 0, got_post = 0, got_3d = 0;
    int seen_header = 0;
    toc_result rc;

    c.p = data;
    c.end = data + len;

    {
        size_t _maxit = len + 1;
        while (csp_peek(&c)) {
            if (!_maxit--) return TOC_ERROR_PARSE;
            if (!csp_isword(&c)) {
                /* skip non-word content (e.g. stray chars) */
                ++c.p;
                continue;
            }
            char w[32];
            csp_word(&c, w, sizeof(w));

            if (!seen_header &&
                (csp_kweq(w, "CSPLUTV1.0") || csp_kweq(w, "CSPLUT0001"))) {
                seen_header = 1;
                continue;
            }

            if (csp_kweq(w, "3DMESH") || csp_kweq(w, "3DMESH2")) {
                long n1, n2, n3;
                /* read first N */
                if (!csp_int(&c, &n1)) return TOC_ERROR_PARSE;
                {
                    const char *peek = c.p;
                    while (peek < c.end && csp_isspc(*peek)) ++peek;
                    if (peek < c.end && *peek == '\n') {
                        c.p = peek + 1;
                        if (n1 < 2 || n1 > 256) return TOC_ERROR_PARSE;
                        mesh = (int)n1;
                    } else {
                        c.p = peek;
                        if (!csp_int(&c, &n2) || !csp_int(&c, &n3))
                            return TOC_ERROR_PARSE;
                        if (n1 != n2 || n2 != n3 || n1 < 2 || n1 > 256)
                            return TOC_ERROR_PARSE;
                        mesh = (int)n1;
                    }
                }
                continue;
            }

            if (csp_kweq(w, "PRE_1D")) {
                long n;
                if (!csp_int(&c, &n) || n < 2 || n > 65536) return TOC_ERROR_PARSE;
                pre_len = (int)n;
                pre_data = (float *)toc_malloc(&list->alloc,
                                               (size_t)pre_len * 3 * sizeof(float));
                if (!pre_data) return TOC_ERROR_OUT_OF_MEMORY;
                got_pre = 0;
                while (got_pre < pre_len) {
                    float r, g, b;
                    if (!csp_float(&c, &r) || !csp_float(&c, &g) ||
                        !csp_float(&c, &b))
                        break;
                    pre_data[got_pre * 3 + 0] = r;
                    pre_data[got_pre * 3 + 1] = g;
                    pre_data[got_pre * 3 + 2] = b;
                    ++got_pre;
                }
                if (got_pre != pre_len) { toc_free(&list->alloc, pre_data); return TOC_ERROR_PARSE; }
                continue;
            }

            if (csp_kweq(w, "POST_1D")) {
                long n;
                if (!csp_int(&c, &n) || n < 2 || n > 65536) return TOC_ERROR_PARSE;
                post_len = (int)n;
                post_data = (float *)toc_malloc(&list->alloc,
                                                (size_t)post_len * 3 * sizeof(float));
                if (!post_data) return TOC_ERROR_OUT_OF_MEMORY;
                got_post = 0;
                while (got_post < post_len) {
                    float r, g, b;
                    if (!csp_float(&c, &r) || !csp_float(&c, &g) ||
                        !csp_float(&c, &b))
                        break;
                    post_data[got_post * 3 + 0] = r;
                    post_data[got_post * 3 + 1] = g;
                    post_data[got_post * 3 + 2] = b;
                    ++got_post;
                }
                if (got_post != post_len) { toc_free(&list->alloc, post_data); return TOC_ERROR_PARSE; }
                continue;
            }

            if (csp_kweq(w, "3D")) {
                if (mesh < 2) return TOC_ERROR_PARSE;
                size_t want = (size_t)mesh * mesh * mesh;
                lut3d = (float *)toc_malloc(&list->alloc, want * 3 * sizeof(float));
                if (!lut3d) return TOC_ERROR_OUT_OF_MEMORY;
                got_3d = 0;
                while ((size_t)got_3d < want) {
                    float r, g, b;
                    if (!csp_float(&c, &r) || !csp_float(&c, &g) ||
                        !csp_float(&c, &b))
                        break;
                    lut3d[got_3d * 3 + 0] = r;
                    lut3d[got_3d * 3 + 1] = g;
                    lut3d[got_3d * 3 + 2] = b;
                    ++got_3d;
                }
                if ((size_t)got_3d != want) { toc_free(&list->alloc, lut3d); return TOC_ERROR_PARSE; }
                continue;
            }

            /* unknown keyword: skip line */
            while (c.p < c.end && *c.p != '\n') ++c.p;
        }
    }

    if (mesh < 2 || !lut3d)
        return TOC_ERROR_PARSE; /* CSP requires a 3D mesh + data */

    rc = TOC_SUCCESS;

    /* create PRE_1D op */
    if (pre_data) {
        toc_op *op = toc_op_list_push(list, TOC_OP_LUT1D);
        if (!op) { rc = TOC_ERROR_OUT_OF_MEMORY; goto fail; }
        if (!toc_op_list_own(list, pre_data)) { rc = TOC_ERROR_OUT_OF_MEMORY; goto fail; }
        op->u.lut1d.length = pre_len;
        op->u.lut1d.channels = 3;
        op->u.lut1d.domain_min = 0.0f;
        op->u.lut1d.domain_max = 1.0f;
        op->u.lut1d.data = pre_data;
        op->u.lut1d.interp = TOC_INTERP_LINEAR;
        pre_data = NULL; /* ownership transferred */
    }

    /* create 3D LUT op */
    if (lut3d) {
        toc_op *op = toc_op_list_push(list, TOC_OP_LUT3D);
        if (!op) { rc = TOC_ERROR_OUT_OF_MEMORY; goto fail; }
        if (!toc_op_list_own(list, lut3d)) { rc = TOC_ERROR_OUT_OF_MEMORY; goto fail; }
        op->u.lut3d.size = mesh;
        op->u.lut3d.domain_min[0] = op->u.lut3d.domain_min[1] =
            op->u.lut3d.domain_min[2] = 0.0f;
        op->u.lut3d.domain_max[0] = op->u.lut3d.domain_max[1] =
            op->u.lut3d.domain_max[2] = 1.0f;
        op->u.lut3d.data = lut3d;
        op->u.lut3d.interp = TOC_INTERP_TETRAHEDRAL;
        lut3d = NULL; /* ownership transferred */
    }

    /* create POST_1D op */
    if (post_data) {
        toc_op *op = toc_op_list_push(list, TOC_OP_LUT1D);
        if (!op) { rc = TOC_ERROR_OUT_OF_MEMORY; goto fail; }
        if (!toc_op_list_own(list, post_data)) { rc = TOC_ERROR_OUT_OF_MEMORY; goto fail; }
        op->u.lut1d.length = post_len;
        op->u.lut1d.channels = 3;
        op->u.lut1d.domain_min = 0.0f;
        op->u.lut1d.domain_max = 1.0f;
        op->u.lut1d.data = post_data;
        op->u.lut1d.interp = TOC_INTERP_LINEAR;
        post_data = NULL; /* ownership transferred */
    }

    return TOC_SUCCESS;

fail:
    toc_free(&list->alloc, pre_data);
    toc_free(&list->alloc, lut3d);
    toc_free(&list->alloc, post_data);
    return rc;
}
