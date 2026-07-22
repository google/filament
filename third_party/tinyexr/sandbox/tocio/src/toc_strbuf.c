/*
 * tocio - allocator-backed growable string builder (no sprintf).
 *
 * Numbers are emitted two ways: toc_sb_hexfloat (bit-exact C hex-float literal,
 * for AOT-C codegen) and toc_sb_decfloat (decimal with a '.', for GLSL which
 * has no hex-float literals).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

void toc_sb_init(toc_sb *sb, const toc_allocator *a) {
    sb->alloc = a ? *a : *toc_default_allocator();
    sb->buf = NULL;
    sb->len = sb->cap = 0;
    sb->oom = 0;
}

static int sb_reserve(toc_sb *sb, size_t extra) {
    size_t need;
    if (sb->oom) return 0;
    need = sb->len + extra + 1;
    if (need <= sb->cap) return 1;
    {
        size_t ncap = sb->cap ? sb->cap * 2 : 256;
        char *n;
        while (ncap < need) ncap *= 2;
        n = (char *)toc_malloc(&sb->alloc, ncap);
        if (!n) { sb->oom = 1; return 0; }
        if (sb->buf) {
            memcpy(n, sb->buf, sb->len);
            toc_free(&sb->alloc, sb->buf);
        }
        sb->buf = n;
        sb->cap = ncap;
    }
    return 1;
}

int toc_sb_putc(toc_sb *sb, char c) {
    if (!sb_reserve(sb, 1)) return 0;
    sb->buf[sb->len++] = c;
    sb->buf[sb->len] = 0;
    return 1;
}
int toc_sb_putn(toc_sb *sb, const char *s, size_t n) {
    if (!sb_reserve(sb, n)) return 0;
    memcpy(sb->buf + sb->len, s, n);
    sb->len += n;
    sb->buf[sb->len] = 0;
    return 1;
}
int toc_sb_puts(toc_sb *sb, const char *s) { return toc_sb_putn(sb, s, strlen(s)); }

int toc_sb_int(toc_sb *sb, long v) {
    char tmp[24];
    int n = 0;
    unsigned long u;
    if (v < 0) {
        if (!toc_sb_putc(sb, '-')) return 0;
        u = (unsigned long)(-(v + 1)) + 1u;
    } else {
        u = (unsigned long)v;
    }
    if (u == 0) tmp[n++] = '0';
    while (u) {
        tmp[n++] = (char)('0' + (int)(u % 10));
        u /= 10;
    }
    while (n--) {
        if (!toc_sb_putc(sb, tmp[n])) return 0;
    }
    return 1;
}

static char hexdig(int v) { return (char)(v < 10 ? '0' + v : 'a' + (v - 10)); }

int toc_sb_hexfloat(toc_sb *sb, float v) {
    union { float f; uint32_t u; } b;
    uint32_t sign, exp, mant, frac24;
    int E, nib[6], i, last, leading;
    b.f = v;
    sign = b.u >> 31;
    exp = (b.u >> 23) & 0xffu;
    mant = b.u & 0x7fffffu;
    if (sign && !(exp == 0 && mant == 0)) {
        if (!toc_sb_putc(sb, '-')) return 0;
    }
    if (exp == 0xffu) {
        /* inf/nan: emit a finite-ish placeholder (LUT data is finite). */
        return toc_sb_puts(sb, mant ? "(0.0f/0.0f)" : "3.4028235e38f");
    }
    if (exp == 0 && mant == 0) return toc_sb_puts(sb, "0x0p+0f");
    leading = (exp == 0) ? 0 : 1;
    E = (exp == 0) ? -126 : (int)exp - 127;
    frac24 = mant << 1;
    for (i = 0; i < 6; ++i) nib[i] = (int)((frac24 >> (20 - 4 * i)) & 0xfu);
    last = 5;
    while (last >= 0 && nib[last] == 0) --last;
    if (!toc_sb_puts(sb, "0x")) return 0;
    if (!toc_sb_putc(sb, (char)('0' + leading))) return 0;
    if (last >= 0) {
        if (!toc_sb_putc(sb, '.')) return 0;
        for (i = 0; i <= last; ++i)
            if (!toc_sb_putc(sb, hexdig(nib[i]))) return 0;
    }
    if (!toc_sb_putc(sb, 'p')) return 0;
    if (E >= 0) {
        if (!toc_sb_putc(sb, '+')) return 0;
    } else {
        if (!toc_sb_putc(sb, '-')) return 0;
        E = -E;
    }
    if (!toc_sb_int(sb, E)) return 0;
    return toc_sb_putc(sb, 'f');
}

int toc_sb_decfloat(toc_sb *sb, float fv) {
    double v = (double)fv;
    int neg = 0, e = 0, i, digc;
    long long di;
    char dig[10];
    if (fv != fv) return toc_sb_puts(sb, "0.0"); /* nan */
    if (v < 0.0) { neg = 1; v = -v; }
    if (v == 0.0) return toc_sb_puts(sb, neg ? "-0.0" : "0.0");
    if (v > 3.0e38) return toc_sb_puts(sb, neg ? "-3.4e38" : "3.4e38");
    /* normalize to [1,10) */
    while (v >= 10.0) { v /= 10.0; ++e; }
    while (v < 1.0) { v *= 10.0; --e; }
    /* 9 significant digits with rounding */
    di = (long long)(v * 1.0e8 + 0.5);
    if (di >= 1000000000LL) { di /= 10; ++e; }
    for (i = 8; i >= 0; --i) { dig[i] = (char)('0' + (int)(di % 10)); di /= 10; }
    digc = 9;
    while (digc > 1 && dig[digc - 1] == '0') --digc; /* trim trailing zeros */

    if (neg && !toc_sb_putc(sb, '-')) return 0;
    if (e >= -4 && e < 9) {
        /* fixed notation */
        if (e >= 0) {
            for (i = 0; i <= e; ++i) {
                char c = (i < digc) ? dig[i] : '0';
                if (!toc_sb_putc(sb, c)) return 0;
            }
            if (!toc_sb_putc(sb, '.')) return 0;
            if (e + 1 >= digc) {
                if (!toc_sb_putc(sb, '0')) return 0;
            } else {
                for (i = e + 1; i < digc; ++i)
                    if (!toc_sb_putc(sb, dig[i])) return 0;
            }
        } else {
            if (!toc_sb_puts(sb, "0.")) return 0;
            for (i = 0; i < -e - 1; ++i)
                if (!toc_sb_putc(sb, '0')) return 0;
            for (i = 0; i < digc; ++i)
                if (!toc_sb_putc(sb, dig[i])) return 0;
        }
    } else {
        /* scientific: d.ddddde+NN */
        if (!toc_sb_putc(sb, dig[0])) return 0;
        if (!toc_sb_putc(sb, '.')) return 0;
        if (digc == 1) {
            if (!toc_sb_putc(sb, '0')) return 0;
        } else {
            for (i = 1; i < digc; ++i)
                if (!toc_sb_putc(sb, dig[i])) return 0;
        }
        if (!toc_sb_putc(sb, 'e')) return 0;
        if (!toc_sb_int(sb, e)) return 0;
    }
    return 1;
}

char *toc_sb_take(toc_sb *sb, size_t *out_len) {
    char *b = sb->buf;
    if (sb->oom) return NULL;
    if (out_len) *out_len = sb->len;
    sb->buf = NULL;
    sb->len = sb->cap = 0;
    return b;
}

void toc_sb_free(toc_sb *sb) {
    if (sb->buf) toc_free(&sb->alloc, sb->buf);
    sb->buf = NULL;
    sb->len = sb->cap = 0;
}
