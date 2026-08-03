/*
 * tocio - file-LUT loaders: Iridas .cube (1D+3D), Sony .spi1d, .spi3d.
 *
 * Produces a LUT1D or LUT3D op whose sample array is owned by the op list.
 * Inverse file-LUTs are reported NONINVERTIBLE (pass 1 is forward-only).
 *
 * Reimplemented from the OpenColorIO file formats (BSD-3-Clause).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

typedef struct {
    const char *p, *end;
} cur;

static int is_space(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static void skip_ws(cur *c) {
    for (;;) {
        while (c->p < c->end && is_space(*c->p)) ++c->p;
        if (c->p < c->end && *c->p == '#') {
            while (c->p < c->end && *c->p != '\n') ++c->p;
            continue;
        }
        break;
    }
}

/* Peek: is the next token alphabetic (a keyword) ? */
static int peek_word(cur *c) {
    skip_ws(c);
    return c->p < c->end &&
           ((*c->p >= 'A' && *c->p <= 'Z') || (*c->p >= 'a' && *c->p <= 'z'));
}

/* Read a bare word into buf; returns length. */
static int read_word(cur *c, char *buf, int bufsz) {
    int n = 0;
    skip_ws(c);
    while (c->p < c->end && !is_space(*c->p) && n < bufsz - 1) {
        char ch = *c->p;
        if (ch == '#') break;
        buf[n++] = ch;
        ++c->p;
    }
    buf[n] = 0;
    return n;
}

static int read_float(cur *c, float *out) {
    skip_ws(c);
    return toc_parse_float(&c->p, c->end, out);
}
static int read_int(cur *c, long *out) {
    skip_ws(c);
    return toc_parse_int(&c->p, c->end, out);
}

static int kweq(const char *w, const char *kw) { return strcmp(w, kw) == 0; }

/* ---- Iridas .cube -------------------------------------------------------- */
static toc_result load_cube(toc_op_list *list, cur *c) {
    int dim = 0, size = 0;
    float dmin[3] = {0, 0, 0}, dmax[3] = {1, 1, 1};
    float *arr = NULL;
    size_t want = 0, got = 0;
    toc_op *op;
    while (c->p < c->end) {
        skip_ws(c);
        if (c->p >= c->end) break;
        if (peek_word(c)) {
            char w[32];
            read_word(c, w, sizeof(w));
            if (kweq(w, "TITLE")) {
                while (c->p < c->end && *c->p != '\n') ++c->p;
            } else if (kweq(w, "LUT_1D_SIZE") || kweq(w, "LUT_3D_SIZE")) {
                long n = 0;
                if (!read_int(c, &n) || n < 2 || n > 256) return TOC_ERROR_PARSE;
                size = (int)n;
                dim = w[4] == '1' ? 1 : 3;
                want = dim == 1 ? (size_t)size * 3
                                : (size_t)size * size * size * 3;
                arr = (float *)toc_malloc(&list->alloc, want * sizeof(float));
                if (!arr) return TOC_ERROR_OUT_OF_MEMORY;
            } else if (kweq(w, "DOMAIN_MIN")) {
                read_float(c, &dmin[0]); read_float(c, &dmin[1]);
                read_float(c, &dmin[2]);
            } else if (kweq(w, "DOMAIN_MAX")) {
                read_float(c, &dmax[0]); read_float(c, &dmax[1]);
                read_float(c, &dmax[2]);
            } else if (kweq(w, "LUT_1D_INPUT_RANGE") ||
                       kweq(w, "LUT_3D_INPUT_RANGE")) {
                read_float(c, &dmin[0]); read_float(c, &dmax[0]);
                dmin[1] = dmin[2] = dmin[0];
                dmax[1] = dmax[2] = dmax[0];
            } else {
                while (c->p < c->end && *c->p != '\n') ++c->p; /* skip */
            }
        } else {
            float r, g, b;
            if (!arr) return TOC_ERROR_PARSE;
            if (!read_float(c, &r) || !read_float(c, &g) || !read_float(c, &b))
                break;
            if (got + 3 > want) return TOC_ERROR_PARSE;
            arr[got++] = r; arr[got++] = g; arr[got++] = b;
        }
    }
    if (!arr || got != want) { toc_free(&list->alloc, arr); return TOC_ERROR_PARSE; }
    if (!toc_op_list_own(list, arr)) { toc_free(&list->alloc, arr); return TOC_ERROR_OUT_OF_MEMORY; }
    if (dim == 1) {
        op = toc_op_list_push(list, TOC_OP_LUT1D);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.lut1d.length = size;
        op->u.lut1d.channels = 3;
        op->u.lut1d.domain_min = dmin[0];
        op->u.lut1d.domain_max = dmax[0];
        op->u.lut1d.data = arr;
        op->u.lut1d.interp = TOC_INTERP_LINEAR;
    } else {
        op = toc_op_list_push(list, TOC_OP_LUT3D);
        if (!op) return TOC_ERROR_OUT_OF_MEMORY;
        op->u.lut3d.size = size;
        memcpy(op->u.lut3d.domain_min, dmin, sizeof(dmin));
        memcpy(op->u.lut3d.domain_max, dmax, sizeof(dmax));
        op->u.lut3d.data = arr;
        op->u.lut3d.interp = TOC_INTERP_TETRAHEDRAL;
    }
    return TOC_SUCCESS;
}

/* ---- Sony .spi1d --------------------------------------------------------- */
static toc_result load_spi1d(toc_op_list *list, cur *c) {
    float from0 = 0.0f, from1 = 1.0f;
    long length = 0, comps = 1;
    float *arr = NULL;
    size_t want, got = 0;
    toc_op *op;
    char w[32];
    /* header: Version <n> */
    while (peek_word(c)) {
        read_word(c, w, sizeof(w));
        if (kweq(w, "Version")) {
            long v;
            read_int(c, &v);
        } else if (kweq(w, "From")) {
            read_float(c, &from0);
            read_float(c, &from1);
        } else if (kweq(w, "Length")) {
            read_int(c, &length);
        } else if (kweq(w, "Components")) {
            read_int(c, &comps);
        } else {
            break; /* unknown header word */
        }
    }
    if (length < 2 || comps < 1 || comps > 3) return TOC_ERROR_PARSE;
    skip_ws(c);
    if (c->p < c->end && *c->p == '{') ++c->p;
    want = (size_t)length * (size_t)comps;
    arr = (float *)toc_malloc(&list->alloc, want * sizeof(float));
    if (!arr) return TOC_ERROR_OUT_OF_MEMORY;
    while (got < want) {
        float v;
        skip_ws(c);
        if (c->p < c->end && *c->p == '}') break;
        if (!read_float(c, &v)) break;
        arr[got++] = v;
    }
    if (got != want) { toc_free(&list->alloc, arr); return TOC_ERROR_PARSE; }
    if (!toc_op_list_own(list, arr)) { toc_free(&list->alloc, arr); return TOC_ERROR_OUT_OF_MEMORY; }
    op = toc_op_list_push(list, TOC_OP_LUT1D);
    if (!op) return TOC_ERROR_OUT_OF_MEMORY;
    op->u.lut1d.length = (int)length;
    op->u.lut1d.channels = (int)comps;
    op->u.lut1d.domain_min = from0;
    op->u.lut1d.domain_max = from1;
    op->u.lut1d.data = arr;
    op->u.lut1d.interp = TOC_INTERP_LINEAR;
    return TOC_SUCCESS;
}

/* ---- Sony .spi3d --------------------------------------------------------- */
static toc_result load_spi3d(toc_op_list *list, cur *c) {
    long n1, n2, n3, nin, nout;
    int N;
    float *arr = NULL;
    size_t want, filled = 0;
    toc_op *op;
    char w[32];
    /* "SPILUT 1.0" then "3 3" then "N N N" */
    if (peek_word(c)) read_word(c, w, sizeof(w)); /* SPILUT */
    { float ver; read_float(c, &ver); }
    if (!read_int(c, &nin) || !read_int(c, &nout)) return TOC_ERROR_PARSE;
    if (!read_int(c, &n1) || !read_int(c, &n2) || !read_int(c, &n3))
        return TOC_ERROR_PARSE;
    if (n1 != n2 || n2 != n3 || n1 < 2 || n1 > 256) return TOC_ERROR_UNSUPPORTED;
    N = (int)n1;
    want = (size_t)N * N * N * 3;
    arr = (float *)toc_calloc(&list->alloc, want, sizeof(float));
    if (!arr) return TOC_ERROR_OUT_OF_MEMORY;
    while (filled < (size_t)N * N * N) {
        long ir, ig, ib;
        float r, g, b;
        size_t idx;
        skip_ws(c);
        if (c->p >= c->end) break;
        if (!read_int(c, &ir) || !read_int(c, &ig) || !read_int(c, &ib))
            break;
        if (!read_float(c, &r) || !read_float(c, &g) || !read_float(c, &b))
            break;
        if (ir < 0 || ir >= N || ig < 0 || ig >= N || ib < 0 || ib >= N) {
            toc_free(&list->alloc, arr);
            return TOC_ERROR_PARSE;
        }
        idx = (((size_t)ib * N + ig) * N + ir) * 3;
        arr[idx + 0] = r; arr[idx + 1] = g; arr[idx + 2] = b;
        ++filled;
    }
    if (filled != (size_t)N * N * N) { toc_free(&list->alloc, arr); return TOC_ERROR_PARSE; }
    if (!toc_op_list_own(list, arr)) { toc_free(&list->alloc, arr); return TOC_ERROR_OUT_OF_MEMORY; }
    op = toc_op_list_push(list, TOC_OP_LUT3D);
    if (!op) return TOC_ERROR_OUT_OF_MEMORY;
    op->u.lut3d.size = N;
    op->u.lut3d.domain_min[0] = op->u.lut3d.domain_min[1] =
        op->u.lut3d.domain_min[2] = 0.0f;
    op->u.lut3d.domain_max[0] = op->u.lut3d.domain_max[1] =
        op->u.lut3d.domain_max[2] = 1.0f;
    op->u.lut3d.data = arr;
    op->u.lut3d.interp = TOC_INTERP_TETRAHEDRAL;
    return TOC_SUCCESS;
}

/* ---- dispatch ------------------------------------------------------------ */
static int ends_with(const char *s, const char *suf) {
    size_t ls = strlen(s), lf = strlen(suf);
    if (lf > ls) return 0;
    return strcmp(s + (ls - lf), suf) == 0;
}

toc_result toc_load_lutfile(toc_op_list *list, const char *name,
                            const char *data, size_t len, int invert) {
    cur c;
    c.p = data;
    c.end = data + len;
    if (invert) return TOC_ERROR_NONINVERTIBLE; /* pass 1: forward only */
    if (name && (ends_with(name, ".cube") || ends_with(name, ".CUBE")))
        return load_cube(list, &c);
    if (name && ends_with(name, ".spi1d")) return load_spi1d(list, &c);
    if (name && ends_with(name, ".spi3d")) return load_spi3d(list, &c);
    if (name && (ends_with(name, ".clf") || ends_with(name, ".CLF")))
        return toc_load_clf(list, data, len);
    if (name && (ends_with(name, ".csp") || ends_with(name, ".CSP")))
        return toc_load_csp(list, data, len);
    /* sniff by content */
    {
        cur s = c;
        char w[32];
        skip_ws(&s);
        if (peek_word(&s)) {
            read_word(&s, w, sizeof(w));
            if (kweq(w, "SPILUT")) return load_spi3d(list, &c);
            if (kweq(w, "Version")) return load_spi1d(list, &c);
        }
    }
    /* sniff for CSP: CSPLUTV1.0 or CSPLUT0001 */
    if (len > 9) {
        const char *p = data;
        while (p < data + len && (unsigned char)*p <= 0x20) ++p;
        if ((size_t)(data + len - p) > 9 &&
            memcmp(p, "CSPLUT", 6) == 0)
            return toc_load_csp(list, data, len);
    }
    /* sniff for CLF: look for <?xml or <ProcessList */
    if (len > 12) {
        const char *p = data;
        while (p < data + len && (unsigned char)*p <= 0x20) ++p;
        if ((size_t)(data + len - p) > 12) {
            if ((p[0] == '<' && p[1] == '?') ||
                (p[0] == '<' && memcmp(p + 1, "ProcessList", 11) == 0))
                return toc_load_clf(list, data, len);
        }
    }
    return load_cube(list, &c);
}
