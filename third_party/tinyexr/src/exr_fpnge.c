/*
 * TinyEXR - fpnge-derived DEFLATE literal encoder with PSHUFB Huffman lookup.
 *
 * Ported and adapted from fpnge (https://github.com/veluca93/fpnge).
 * Copyright 2021 Google LLC, Apache-2.0. Modified: reduced to a generic
 * literal-only DEFLATE encoder (no PNG filtering / LZ77), converted from C++
 * to C11, and split into a scalar reference plus an SSE4.1/AVX2 PSHUFB lookup
 * kernel. See the top-level NOTICE file.
 *
 * fpnge's constraint that mid-range symbols [16,239] share a single code
 * length is what makes the per-byte (nbits,bits) lookup expressible as 16-entry
 * PSHUFB tables (first16 / mid-uniform / last16).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"


/* Reversed nibble (deflate codes are emitted LSB-first). */
static const uint8_t kRevNib[16] = {0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE,
                                    0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF};

static uint16_t bit_reverse(size_t nbits, uint16_t bits) {
    uint16_t r = (uint16_t)((kRevNib[bits & 0xF] << 12) |
                            (kRevNib[(bits >> 4) & 0xF] << 8) |
                            (kRevNib[(bits >> 8) & 0xF] << 4) |
                            (kRevNib[(bits >> 12) & 0xF]));
    return (uint16_t)(r >> (16 - nbits));
}

static const uint64_t kBaselineData[286] = {
    113, 54, 28, 18, 12, 9, 7, 6, 5, 4, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1,
    1,   1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1,   1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1,   1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1,   1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1,   1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1,   1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1,   1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1,   1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1,   1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1,   1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1,   1,  1,  1,  1,  1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 4, 5, 6, 7, 9,
    12,  18, 29, 54, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1,   1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1};

/* Length-limited Huffman code lengths via DP (fpnge ComputeCodeLengths). */
static int compute_code_lengths(const exr_allocator *alloc, const uint64_t *freqs,
                                size_t n, uint8_t *min_limit, uint8_t *max_limit,
                                uint8_t *nbits) {
    size_t precision = 0, i, sym, slots;
    uint64_t freqsum = 0, infty;
    uint64_t *dynp;
    size_t off;
    for (i = 0; i < n; i++) {
        freqsum += freqs[i];
        if (min_limit[i] < 1) min_limit[i] = 1;
        if (max_limit[i] > precision) precision = max_limit[i];
    }
    infty = freqsum * precision;
    slots = (((size_t)1 << precision) + 1);
    dynp = (uint64_t *)exr_malloc(alloc, slots * (n + 1) * sizeof(uint64_t));
    if (!dynp) return 0;
    for (i = 0; i < slots * (n + 1); i++) dynp[i] = infty;
#define D(s, o) dynp[(s) * slots + (o)]
    D(0, 0) = 0;
    for (sym = 0; sym < n; sym++) {
        int bits;
        for (bits = min_limit[sym]; bits <= max_limit[sym]; bits++) {
            size_t off_delta = (size_t)1 << (precision - bits);
            for (off = 0; off + off_delta <= ((size_t)1 << precision); off++) {
                uint64_t cand = D(sym, off) + freqs[sym] * (uint64_t)bits;
                if (cand < D(sym + 1, off + off_delta))
                    D(sym + 1, off + off_delta) = cand;
            }
        }
    }
    sym = n;
    off = (size_t)1 << precision;
    while (sym-- > 0) {
        int bits;
        for (bits = min_limit[sym]; bits <= max_limit[sym]; bits++) {
            size_t off_delta = (size_t)1 << (precision - bits);
            if (off_delta <= off &&
                D(sym + 1, off) == D(sym, off - off_delta) + freqs[sym] * (uint64_t)bits) {
                off -= off_delta;
                nbits[sym] = (uint8_t)bits;
                break;
            }
        }
    }
#undef D
    exr_free(alloc, dynp);
    return 1;
}

/* Build nbits[286] with the fpnge structure (mid [16,239] uniform). */
static int compute_nbits(const exr_allocator *a, const uint64_t *collected,
                         uint8_t *nbits) {
    uint64_t data[286];
    uint64_t collapsed_data[48] = {0};
    uint8_t collapsed_min[48] = {0}, collapsed_max[48];
    uint8_t collapsed_nbits[48] = {0};
    uint8_t tail_nbits[29] = {0}, tail_min[29], tail_max[29];
    size_t i, j;

    for (i = 0; i < 286; i++) data[i] = collected[i] + kBaselineData[i];
    for (i = 0; i < 48; i++) collapsed_max[i] = 8;
    for (i = 0; i < 16; i++) collapsed_data[i] = data[i];
    for (i = 0; i < 14; i++) { collapsed_data[16 + i] = 1; collapsed_min[16 + i] = 8; }
    for (j = 0; j < 16; j++) collapsed_data[16 + 14 + j] += data[240 + j];
    collapsed_data[46] = 1;
    collapsed_min[46] = 8;
    collapsed_data[47] = data[285];

    if (!compute_code_lengths(a, collapsed_data, 48, collapsed_min, collapsed_max,
                              collapsed_nbits))
        return 0;
    for (i = 0; i < 29; i++) { tail_min[i] = 4; tail_max[i] = 7; }
    if (!compute_code_lengths(a, data + 256, 29, tail_min, tail_max, tail_nbits))
        return 0;

    for (i = 0; i < 16; i++) nbits[i] = collapsed_nbits[i];
    for (i = 0; i < 14; i++)
        for (j = 0; j < 16; j++) nbits[(i + 1) * 16 + j] = collapsed_nbits[16 + i] + 4;
    for (i = 0; i < 16; i++) nbits[240 + i] = collapsed_nbits[30 + i];
    for (i = 0; i < 29; i++) nbits[256 + i] = collapsed_nbits[46] + tail_nbits[i];
    nbits[285] = collapsed_nbits[47];
    return 1;
}

static void compute_canonical(const uint8_t *nbits, uint16_t *bits) {
    uint8_t cnt[16] = {0};
    uint16_t next[16] = {0}, code = 0;
    size_t i;
    for (i = 0; i < 286; i++) cnt[nbits[i]]++;
    for (i = 1; i < 16; i++) { code = (uint16_t)((code + cnt[i - 1]) << 1); next[i] = code; }
    for (i = 0; i < 286; i++) bits[i] = bit_reverse(nbits[i], next[nbits[i]]++);
}

int exr_fpnge_build_table(const exr_allocator *a, const uint64_t *collected,
                          exr_fpnge_table *t) {
    uint16_t bits[286];
    size_t i;
    if (!compute_nbits(a, collected, t->nbits)) return 0;
    compute_canonical(t->nbits, bits);
    for (i = 0; i < 16; i++) {
        t->first16_nbits[i] = t->nbits[i];
        t->first16_bits[i] = (uint8_t)bits[i];
        t->last16_nbits[i] = t->nbits[240 + i];
        t->last16_bits[i] = (uint8_t)bits[240 + i];
    }
    t->mid_nbits = t->nbits[16];
    t->mid_lowbits[0] = t->mid_lowbits[15] = 0;
    for (i = 16; i < 240; i += 16)
        t->mid_lowbits[i / 16] = (uint8_t)(bits[i] & ((1u << (t->mid_nbits - 4)) - 1));
    t->end_bits = bits[256];
    return 1;
}

/* ---- LSB-first bit writer ------------------------------------------------- */
typedef struct {
    uint8_t *dst;
    size_t cap, ofs;
    uint64_t buf;
    int bits;
    int err;
} fbw;
/* fpnge-style emit: caller guarantees n <= 56 (residual is always < 8 bits, so
 * residual + n <= 63, no overflow). One 8-byte store per call instead of a
 * byte-at-a-time loop; ofs advances by whole bytes, the rest is overwritten by
 * the next call. The eight byte stores fold to a single 64-bit store on
 * little-endian targets. */
static void fbw_put(fbw *w, uint32_t val, int n) {
    size_t o = w->ofs, nbytes;
    uint64_t b;
    w->buf |= ((uint64_t)val) << w->bits;
    w->bits += n;
    b = w->buf;
    if (o + 8 > w->cap) { w->err = 1; return; }
    w->dst[o + 0] = (uint8_t)b;        w->dst[o + 1] = (uint8_t)(b >> 8);
    w->dst[o + 2] = (uint8_t)(b >> 16); w->dst[o + 3] = (uint8_t)(b >> 24);
    w->dst[o + 4] = (uint8_t)(b >> 32); w->dst[o + 5] = (uint8_t)(b >> 40);
    w->dst[o + 6] = (uint8_t)(b >> 48); w->dst[o + 7] = (uint8_t)(b >> 56);
    nbytes = (size_t)(w->bits >> 3);
    w->ofs = o + nbytes;
    w->bits &= 7;
    w->buf = b >> (nbytes * 8);
}
static void fbw_flush(fbw *w) {
    while (w->bits > 0) {
        if (w->ofs >= w->cap) { w->err = 1; return; }
        w->dst[w->ofs++] = (uint8_t)w->buf;
        w->buf >>= 8;
        w->bits -= 8;
    }
    w->bits = 0;
    w->buf = 0;
}

/* Write the fixed-structure dynamic block header (fpnge WriteHuffmanCode). */
static void write_huffman_header(fbw *w, const exr_fpnge_table *t) {
    size_t i;
    fbw_put(w, 29, 5); /* HLIT: 286 lit codes */
    fbw_put(w, 0, 5);  /* HDIST: 1 dist code */
    fbw_put(w, 15, 4); /* HCLEN: 19 code-length codes */
    for (i = 0; i < 19; i++) {
        /* code-length-code lengths: 4 for symbols 0..15, 0 for 16,17,18 */
        static const uint8_t order[19] = {16, 17, 18, 0, 8, 7, 9,  6, 10, 5,
                                          11, 4, 12, 3, 13, 2, 14, 1, 15};
        fbw_put(w, (order[i] < 16) ? 4 : 0, 3);
    }
    for (i = 0; i < 286; i++) fbw_put(w, kRevNib[t->nbits[i]], 4);
    fbw_put(w, 0x8, 4); /* single dist code, length 1 */
}

/* Scalar per-byte (nbits, code) lookup, producing the 16-bit code as a low
 * byte (blo) and high byte (bhi). Mirrors the SSE4.1 PSHUFB kernel exactly. */
void exr_fpnge_lookup_scalar(const exr_fpnge_table *t, const uint8_t *src,
                             size_t count, uint8_t *nb, uint8_t *blo,
                             uint8_t *bhi) {
    size_t i;
    for (i = 0; i < count; ++i) {
        uint8_t b = src[i];
        uint32_t v;
        if (b < 16) {
            nb[i] = t->first16_nbits[b];
            v = t->first16_bits[b];
        } else if (b >= 240) {
            nb[i] = t->last16_nbits[b - 240];
            v = t->last16_bits[b - 240];
        } else {
            nb[i] = t->mid_nbits;
            v = (uint32_t)t->mid_lowbits[b >> 4] |
                ((uint32_t)kRevNib[b & 0xF] << (t->mid_nbits - 4));
        }
        blo[i] = (uint8_t)v;
        bhi[i] = (uint8_t)(v >> 8);
    }
}

/* Public: literal-only zlib stream over src[0..n). Allocates *out. */
/* Scalar reference for the SIMD pair-combine bit-pack stage. */
size_t exr_fpnge_pack16_scalar(const uint8_t *nb, const uint8_t *blo,
                               const uint8_t *bhi, size_t count,
                               uint32_t *combined, uint32_t *cnb) {
    size_t k, np = count / 2;
    for (k = 0; k < np; ++k) {
        uint32_t e = (uint32_t)blo[2 * k] | ((uint32_t)bhi[2 * k] << 8);
        uint32_t o = (uint32_t)blo[2 * k + 1] | ((uint32_t)bhi[2 * k + 1] << 8);
        combined[k] = e | (o << nb[2 * k]);
        cnb[k] = (uint32_t)nb[2 * k] + nb[2 * k + 1];
    }
    return np;
}

exr_result exr_fpnge_deflate(const exr_allocator *a, const uint8_t *src,
                             size_t n, uint8_t **out_data, size_t *out_size,
                             int use_simd) {
    enum { CH = 8192 };
    uint64_t collected[286];
    exr_fpnge_table t;
    uint8_t *out, *nb = NULL, *blo = NULL, *bhi = NULL;
    uint32_t *combined = NULL, *cnb = NULL;
    size_t cap, i, off;
    fbw w;
    uint32_t adler;
    int simd;

    *out_data = NULL;
    *out_size = 0;
    memset(collected, 0, sizeof(collected));
    for (i = 0; i < n; i++) collected[src[i]]++;

    if (!exr_fpnge_build_table(a, collected, &t)) return EXR_ERROR_OUT_OF_MEMORY;

    cap = 2 + 286 + 64 + n * 2 + 16; /* header + worst-case literals + framing */
    out = (uint8_t *)exr_malloc(a, cap);
    nb = (uint8_t *)exr_malloc(a, CH);
    blo = (uint8_t *)exr_malloc(a, CH);
    bhi = (uint8_t *)exr_malloc(a, CH);
    combined = (uint32_t *)exr_malloc(a, (CH / 2) * sizeof(uint32_t));
    cnb = (uint32_t *)exr_malloc(a, (CH / 2) * sizeof(uint32_t));
    if (!out || !nb || !blo || !bhi || !combined || !cnb) {
        exr_free(a, out); exr_free(a, nb); exr_free(a, blo); exr_free(a, bhi);
        exr_free(a, combined); exr_free(a, cnb);
        return EXR_ERROR_OUT_OF_MEMORY;
    }
    out[0] = 0x78;
    out[1] = 0x01;
    w.dst = out;
    w.cap = cap;
    w.ofs = 2;
    w.buf = 0;
    w.bits = 0;
    w.err = 0;

    fbw_put(&w, 1, 1); /* BFINAL */
    fbw_put(&w, 2, 2); /* BTYPE = dynamic */
    write_huffman_header(&w, &t);

    simd = 0; /* 0 scalar, 1 sse4.1, 2 avx2 */
#if defined(EXR_X86)
    if (use_simd) {
        uint32_t caps = exr_cpu_caps();
        if (caps & EXR_SIMD_AVX2) simd = 2;
        else if (caps & EXR_SIMD_SSE41) simd = 1;
    }
#else
    (void)use_simd;
    (void)simd; /* non-x86: the scalar path is taken unconditionally below */
#endif

    for (off = 0; off < n; off += CH) {
        size_t c = (n - off < CH) ? (n - off) : CH;
        size_t np, k;
#if defined(EXR_X86)
        if (simd == 2) exr_fpnge_lookup_avx2(&t, src + off, c, nb, blo, bhi);
        else if (simd == 1) exr_fpnge_lookup_sse41(&t, src + off, c, nb, blo, bhi);
        else exr_fpnge_lookup_scalar(&t, src + off, c, nb, blo, bhi);
        if (simd == 2) np = exr_fpnge_pack16_avx2(nb, blo, bhi, c, combined, cnb);
        else if (simd == 1) np = exr_fpnge_pack16_sse41(nb, blo, bhi, c, combined, cnb);
        else np = exr_fpnge_pack16_scalar(nb, blo, bhi, c, combined, cnb);
#else
        exr_fpnge_lookup_scalar(&t, src + off, c, nb, blo, bhi);
        np = exr_fpnge_pack16_scalar(nb, blo, bhi, c, combined, cnb);
#endif
        /* each combined group holds two codes (<= 30 bits): safe for fbw_put */
        for (k = 0; k < np; ++k) fbw_put(&w, combined[k], (int)cnb[k]);
        if (c & 1) /* trailing odd byte */
            fbw_put(&w, (uint32_t)blo[c - 1] | ((uint32_t)bhi[c - 1] << 8), nb[c - 1]);
    }

    fbw_put(&w, t.end_bits, t.nbits[256]); /* end of block */
    fbw_flush(&w);

    exr_free(a, nb);
    exr_free(a, blo);
    exr_free(a, bhi);
    exr_free(a, combined);
    exr_free(a, cnb);
    if (w.err) { exr_free(a, out); return EXR_ERROR_CORRUPT; }

    adler = exr_adler32(src, n, 1);
    if (w.ofs + 4 > cap) { exr_free(a, out); return EXR_ERROR_CORRUPT; }
    out[w.ofs++] = (uint8_t)(adler >> 24);
    out[w.ofs++] = (uint8_t)(adler >> 16);
    out[w.ofs++] = (uint8_t)(adler >> 8);
    out[w.ofs++] = (uint8_t)(adler);

    *out_data = out;
    *out_size = w.ofs;
    return EXR_SUCCESS;
}
