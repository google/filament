/*
 * tocio - freestanding YAML-subset parser -> generic node tree.
 *
 * Supports the subset OCIO configs use: block mappings (indentation), block
 * sequences ("- item"), inline flow maps {k: v} and seqs [a, b], plain and
 * quoted scalars, comments (#), node tags !<TypeName>, and block scalars (| >)
 * which are captured as a scalar. Anchors/aliases (& *), merge keys, and
 * multi-doc (---) are rejected with TOC_ERROR_PARSE. No libc/libm; no strtod.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

typedef struct {
    const char *p;
    int len;
    int indent;
    int lineno;
} yline;

typedef struct {
    yline *lines;
    int n, i;
    toc_arena *ar;
    int err_line;
} yparser;

/* ---- node construction --------------------------------------------------- */
static toc_node *node_new(yparser *y, toc_node_kind k) {
    toc_node *n = (toc_node *)toc_arena_alloc(y->ar, sizeof(toc_node));
    if (!n) return NULL;
    memset(n, 0, sizeof(*n));
    n->kind = k;
    return n;
}

static int seq_push(yparser *y, toc_node *seq, toc_node *item) {
    toc_node **na = (toc_node **)toc_arena_alloc(
        y->ar, (seq->n_items + 1) * sizeof(toc_node *));
    size_t j;
    if (!na) return 0;
    for (j = 0; j < seq->n_items; ++j) na[j] = seq->items[j];
    na[seq->n_items] = item;
    seq->items = na;
    seq->n_items++;
    return 1;
}

static int map_push(yparser *y, toc_node *map, const char *key, toc_node *val) {
    size_t j;
    const char **nk = (const char **)toc_arena_alloc(
        y->ar, (map->n_pairs + 1) * sizeof(char *));
    toc_node **nv = (toc_node **)toc_arena_alloc(
        y->ar, (map->n_pairs + 1) * sizeof(toc_node *));
    if (!nk || !nv) return 0;
    for (j = 0; j < map->n_pairs; ++j) { nk[j] = map->keys[j]; nv[j] = map->vals[j]; }
    nk[map->n_pairs] = key;
    nv[map->n_pairs] = val;
    map->keys = nk;
    map->vals = nv;
    map->n_pairs++;
    return 1;
}

/* ---- scalar helpers ------------------------------------------------------ */
static char *unescape_scalar(yparser *y, const char *s, int len) {
    /* strips matching quotes and minimal escapes */
    char *out;
    int i = 0, o = 0;
    /* trim trailing whitespace */
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t')) --len;
    out = (char *)toc_arena_alloc(y->ar, (size_t)len + 1);
    if (!out) return NULL;
    if (len >= 2 && s[0] == '"' && s[len - 1] == '"') {
        for (i = 1; i < len - 1; ++i) {
            if (s[i] == '\\' && i + 1 < len - 1) {
                char c = s[++i];
                out[o++] = (c == 'n') ? '\n' : (c == 't') ? '\t' : c;
            } else {
                out[o++] = s[i];
            }
        }
    } else if (len >= 2 && s[0] == '\'' && s[len - 1] == '\'') {
        for (i = 1; i < len - 1; ++i) {
            if (s[i] == '\'' && i + 1 < len - 1 && s[i + 1] == '\'') {
                out[o++] = '\'';
                ++i;
            } else {
                out[o++] = s[i];
            }
        }
    } else {
        for (i = 0; i < len; ++i) out[o++] = s[i];
    }
    out[o] = 0;
    return out;
}

/* ---- flow parser (single line): {k: v, ...}, [a, b], scalar ------------- */
static void flow_skip_ws(const char **p, const char *end) {
    while (*p < end && (**p == ' ' || **p == '\t')) ++(*p);
}

static toc_node *parse_flow(yparser *y, const char **pp, const char *end);

/* read a flow scalar token until a delimiter (, } ]) or a key-separator ':'
 * honoring quotes. A ':' only separates when followed by whitespace, a flow
 * delimiter, or end-of-input; otherwise it is part of a plain scalar (e.g. the
 * namespaced alias "ocio:lin_ciexyzd65_display"). Mirrors find_colon's rule. */
static const char *flow_scalar_end(const char *p, const char *end) {
    if (p < end && (*p == '"' || *p == '\'')) {
        char q = *p++;
        while (p < end && *p != q) {
            if (q == '"' && *p == '\\' && p + 1 < end) ++p;
            ++p;
        }
        if (p < end) ++p; /* closing quote */
        return p;
    }
    while (p < end && *p != ',' && *p != '}' && *p != ']') {
        if (*p == ':') {
            const char *n = p + 1;
            if (n >= end || *n == ' ' || *n == '\t' || *n == ',' ||
                *n == '}' || *n == ']')
                break; /* key/value separator */
        }
        ++p;
    }
    return p;
}

static toc_node *parse_flow(yparser *y, const char **pp, const char *end) {
    const char *p = *pp;
    flow_skip_ws(&p, end);
    if (p >= end) { *pp = p; return node_new(y, TOC_NODE_SCALAR); }
    if (*p == '{') {
        toc_node *m = node_new(y, TOC_NODE_MAP);
        ++p;
        for (;;) {
            const char *ke;
            char *key;
            toc_node *v;
            flow_skip_ws(&p, end);
            if (p < end && *p == '}') { ++p; break; }
            ke = flow_scalar_end(p, end);
            key = unescape_scalar(y, p, (int)(ke - p));
            p = ke;
            flow_skip_ws(&p, end);
            if (p < end && *p == ':') ++p;
            v = parse_flow(y, &p, end);
            if (!m || !key || !v || !map_push(y, m, key, v)) return NULL;
            flow_skip_ws(&p, end);
            if (p < end && *p == ',') { ++p; continue; }
            if (p < end && *p == '}') { ++p; break; }
            if (p >= end) break;
            break; /* unexpected char: stop rather than spin (no forward progress) */
        }
        *pp = p;
        return m;
    }
    if (*p == '[') {
        toc_node *s = node_new(y, TOC_NODE_SEQ);
        ++p;
        for (;;) {
            toc_node *v;
            flow_skip_ws(&p, end);
            if (p < end && *p == ']') { ++p; break; }
            v = parse_flow(y, &p, end);
            if (!s || !v || !seq_push(y, s, v)) return NULL;
            flow_skip_ws(&p, end);
            if (p < end && *p == ',') { ++p; continue; }
            if (p < end && *p == ']') { ++p; break; }
            if (p >= end) break;
            break; /* unexpected char: stop rather than spin (no forward progress) */
        }
        *pp = p;
        return s;
    }
    {
        const char *e = flow_scalar_end(p, end);
        toc_node *sc = node_new(y, TOC_NODE_SCALAR);
        if (!sc) return NULL;
        sc->scalar = unescape_scalar(y, p, (int)(e - p));
        *pp = e;
        return sc;
    }
}

/* ---- block structure ----------------------------------------------------- */
static int find_colon(const char *s, int len) {
    int i, q = 0;
    char qc = 0;
    for (i = 0; i < len; ++i) {
        char c = s[i];
        if (q) {
            if (c == qc) q = 0;
            continue;
        }
        if (c == '"' || c == '\'') { q = 1; qc = c; continue; }
        if (c == '{' || c == '[') return -1; /* flow value, not a block key */
        if (c == ':' && (i + 1 >= len || s[i + 1] == ' ')) return i;
    }
    return -1;
}

static int line_is_dash(const yline *l) {
    return l->len >= 1 && l->p[0] == '-' && (l->len == 1 || l->p[1] == ' ');
}

static toc_node *parse_block(yparser *y, int indent);

/* parse a value that may be inline (after a tag/colon) or a deeper block */
static toc_node *parse_inline_or_block(yparser *y, const char *inl, int inl_len,
                                       const char *tag, int child_indent) {
    /* trim leading ws on inline */
    while (inl_len > 0 && (*inl == ' ' || *inl == '\t')) { ++inl; --inl_len; }
    if (inl_len > 0) {
        if (*inl == '&' || *inl == '*') { y->err_line = -1; return NULL; }
        if (*inl == '|' || *inl == '>') {
            /* block scalar: consume deeper lines, capture concatenated text */
            toc_node *sc = node_new(y, TOC_NODE_SCALAR);
            toc_sb sb;
            toc_sb_init(&sb, &y->ar->alloc);
            while (y->i < y->n && y->lines[y->i].indent >= child_indent) {
                toc_sb_putn(&sb, y->lines[y->i].p, (size_t)y->lines[y->i].len);
                toc_sb_putc(&sb, '\n');
                ++y->i;
            }
            if (sc) {
                size_t l;
                sc->scalar = toc_arena_strndup(y->ar, sb.buf ? sb.buf : "",
                                               sb.buf ? sb.len : 0);
                (void)l;
                sc->tag = tag;
            }
            toc_sb_free(&sb);
            return sc;
        }
        if (*inl == '{' || *inl == '[') {
            const char *p = inl, *end = inl + inl_len;
            toc_node *v = parse_flow(y, &p, end);
            if (v) v->tag = tag;
            return v;
        }
        {
            toc_node *sc = node_new(y, TOC_NODE_SCALAR);
            if (!sc) return NULL;
            sc->scalar = unescape_scalar(y, inl, inl_len);
            sc->tag = tag;
            return sc;
        }
    }
    /* no inline value: the value is the block on following deeper lines */
    if (y->i < y->n && y->lines[y->i].indent >= child_indent) {
        toc_node *child = parse_block(y, y->lines[y->i].indent);
        if (child) child->tag = tag;
        return child;
    }
    {
        /* empty value (e.g. "isdata:") -> empty scalar carrying the tag */
        toc_node *sc = node_new(y, TOC_NODE_SCALAR);
        if (sc) { sc->scalar = ""; sc->tag = tag; }
        return sc;
    }
}

/* parse one block (map / seq / scalar) at the given indent */
static toc_node *parse_block(yparser *y, int indent) {
    yline *L = &y->lines[y->i];
    const char *blk_tag = NULL;
    /* A leading "!<Tag>" on a non-dash line: the tag applies to the block. If
     * the tag is the whole line, the block body is the following lines (same or
     * deeper indent); otherwise the remainder is the block's first line. */
    if (!line_is_dash(L) && L->len >= 2 && L->p[0] == '!' && L->p[1] == '<') {
        const char *gt = L->p + 2;
        int tl = L->len - 2, k = 0;
        const char *rest;
        int rest_len;
        while (k < tl && gt[k] != '>') ++k;
        blk_tag = toc_arena_strndup(y->ar, gt, (size_t)k);
        rest = gt + (k < tl ? k + 1 : k);
        rest_len = (int)(L->p + L->len - rest);
        while (rest_len > 0 && *rest == ' ') { ++rest; --rest_len; }
        if (rest_len == 0) {
            toc_node *node;
            ++y->i; /* consume the tag-only line */
            if (y->i < y->n && y->lines[y->i].indent >= indent)
                node = parse_block(y, y->lines[y->i].indent);
            else
                node = node_new(y, TOC_NODE_SCALAR);
            if (node) node->tag = blk_tag;
            return node;
        }
        if (*rest == '{' || *rest == '[') {
            const char *p = rest, *e = rest + rest_len;
            toc_node *v = parse_flow(y, &p, e);
            if (v) v->tag = blk_tag;
            ++y->i;
            return v;
        }
        /* rewrite L to the remainder (same indent) and fall through */
        L->p = rest;
        L->len = rest_len;
    }
    if (line_is_dash(L)) {
        toc_node *seq = node_new(y, TOC_NODE_SEQ);
        if (!seq) return NULL;
        while (y->i < y->n && y->lines[y->i].indent == indent &&
               line_is_dash(&y->lines[y->i])) {
            yline *d = &y->lines[y->i];
            int dash_col = d->indent;
            int content_col;
            const char *rest = d->p + 1;
            int rest_len = d->len - 1;
            while (rest_len > 0 && *rest == ' ') { ++rest; --rest_len; }
            content_col = dash_col + (int)(rest - d->p);
            if (rest_len == 0) {
                /* item block on following deeper lines */
                ++y->i;
                if (y->i < y->n && y->lines[y->i].indent > dash_col) {
                    toc_node *it = parse_block(y, y->lines[y->i].indent);
                    if (!it || !seq_push(y, seq, it)) return NULL;
                } else {
                    toc_node *it = node_new(y, TOC_NODE_SCALAR);
                    if (!it || !seq_push(y, seq, it)) return NULL;
                }
            } else {
                /* rewrite this line as content at content_col, parse as block */
                d->p = rest;
                d->len = rest_len;
                d->indent = content_col;
                {
                    toc_node *it = parse_block(y, content_col);
                    if (!it || !seq_push(y, seq, it)) return NULL;
                }
            }
        }
        if (blk_tag) seq->tag = blk_tag;
        return seq;
    }
    {
        int colon = find_colon(L->p, L->len);
        if (colon < 0) {
            /* a bare scalar line */
            toc_node *sc = node_new(y, TOC_NODE_SCALAR);
            if (sc) {
                sc->scalar = unescape_scalar(y, L->p, L->len);
                sc->tag = blk_tag;
            }
            ++y->i;
            return sc;
        }
        /* mapping: collect all key: value lines at this indent */
        {
            toc_node *map = node_new(y, TOC_NODE_MAP);
            if (!map) return NULL;
            while (y->i < y->n && y->lines[y->i].indent == indent &&
                   !line_is_dash(&y->lines[y->i])) {
                yline *ln = &y->lines[y->i];
                int cc = find_colon(ln->p, ln->len);
                char *key;
                const char *val;
                int val_len;
                const char *tag = NULL;
                int child_indent = indent + 1;
                toc_node *v;
                if (cc < 0) break; /* not a map line */
                key = unescape_scalar(y, ln->p, cc);
                val = ln->p + cc + 1;
                val_len = ln->len - cc - 1;
                while (val_len > 0 && *val == ' ') { ++val; --val_len; }
                /* tag on the value? */
                if (val_len >= 2 && val[0] == '!' && val[1] == '<') {
                    const char *gt = val + 2;
                    int tl = val_len - 2, k = 0;
                    while (k < tl && gt[k] != '>') ++k;
                    tag = toc_arena_strndup(y->ar, gt, (size_t)k);
                    val = gt + (k < tl ? k + 1 : k);
                    val_len = (int)(ln->p + ln->len - val);
                    while (val_len > 0 && *val == ' ') { ++val; --val_len; }
                }
                ++y->i; /* consume the key line before reading children */
                if (y->i < y->n && y->lines[y->i].indent > indent)
                    child_indent = y->lines[y->i].indent;
                v = parse_inline_or_block(y, val, val_len, tag, child_indent);
                if (!v || !map_push(y, map, key, v)) return NULL;
            }
            if (blk_tag) map->tag = blk_tag;
            return map;
        }
    }
}

/* ---- preprocessing: split into significant lines -------------------------- */
static int strip_comment(const char *s, int len) {
    int i, q = 0;
    char qc = 0;
    for (i = 0; i < len; ++i) {
        char c = s[i];
        if (q) {
            if (c == qc) q = 0;
            continue;
        }
        if (c == '"' || c == '\'') { q = 1; qc = c; continue; }
        if (c == '#' && (i == 0 || s[i - 1] == ' ' || s[i - 1] == '\t'))
            return i;
    }
    return len;
}

toc_result toc_yaml_parse(const char *text, size_t len, toc_arena *ar,
                          toc_node **out_root, int *err_line) {
    yparser y;
    const char *p = text, *end = text + len;
    int cap = 64, ln = 0;
    yline *lines = (yline *)toc_arena_alloc(ar, (size_t)cap * sizeof(yline));
    int count = 0;
    if (!lines) return TOC_ERROR_OUT_OF_MEMORY;
    while (p < end) {
        const char *bol = p;
        int indent = 0, l;
        while (p < end && *p != '\n') ++p;
        l = (int)(p - bol);
        if (p < end) ++p; /* skip newline */
        ++ln;
        /* compute indent; reject tabs in indentation */
        {
            const char *q = bol;
            while (q < bol + l && (*q == ' ' || *q == '\t')) {
                if (*q == '\t') {
                    if (err_line) *err_line = ln;
                    return TOC_ERROR_PARSE;
                }
                ++indent;
                ++q;
            }
            l -= (int)(q - bol);
            bol = q;
        }
        l = strip_comment(bol, l);
        while (l > 0 && (bol[l - 1] == ' ' || bol[l - 1] == '\t' ||
                         bol[l - 1] == '\r'))
            --l;
        if (l == 0) continue; /* blank/comment-only */
        if (l == 3 && bol[0] == '-' && bol[1] == '-' && bol[2] == '-')
            continue; /* tolerate a single doc start */
        if (l >= 3 && bol[0] == '.' && bol[1] == '.' && bol[2] == '.') break;
        if (count == cap) {
            int ncap = cap * 2;
            yline *nl = (yline *)toc_arena_alloc(ar, (size_t)ncap * sizeof(yline));
            if (!nl) return TOC_ERROR_OUT_OF_MEMORY;
            memcpy(nl, lines, (size_t)count * sizeof(yline));
            lines = nl;
            cap = ncap;
        }
        lines[count].p = bol;
        lines[count].len = l;
        lines[count].indent = indent;
        lines[count].lineno = ln;
        ++count;
    }
    y.lines = lines;
    y.n = count;
    y.i = 0;
    y.ar = ar;
    y.err_line = 0;
    if (count == 0) {
        *out_root = node_new(&y, TOC_NODE_MAP);
        return *out_root ? TOC_SUCCESS : TOC_ERROR_OUT_OF_MEMORY;
    }
    *out_root = parse_block(&y, lines[0].indent);
    if (!*out_root) {
        if (err_line) *err_line = y.err_line ? y.err_line : -1;
        return TOC_ERROR_PARSE;
    }
    if (y.i != y.n) {
        /* trailing unparsed content (indentation inconsistency) */
        if (err_line) *err_line = y.lines[y.i].lineno;
        return TOC_ERROR_PARSE;
    }
    return TOC_SUCCESS;
}

/* ---- lookup helpers ------------------------------------------------------ */
const toc_node *toc_node_map_get(const toc_node *n, const char *key) {
    size_t i;
    if (!n || n->kind != TOC_NODE_MAP) return NULL;
    for (i = 0; i < n->n_pairs; ++i)
        if (strcmp(n->keys[i], key) == 0) return n->vals[i];
    return NULL;
}

const char *toc_node_scalar(const toc_node *n) {
    if (!n || n->kind != TOC_NODE_SCALAR) return NULL;
    return n->scalar;
}
