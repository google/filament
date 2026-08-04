/*
 * TinyEXR - PIZ codec (wavelet + Huffman + range LUT).
 *
 * Pure-C11 port of OpenEXR's PIZ decoder (wav2 inverse lifting, canonical
 * Huffman with RLE, and the bitmap range LUT). Large tables are heap-allocated
 * to keep stack usage small and thread-safe.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"


#define PIZ_BITMAP_SIZE 8192
#define PIZ_USHORT_RANGE 65536
#define PIZ_HUF_ENCSIZE (PIZ_USHORT_RANGE + 1) /* 65537 */

#define HUF_DECBITS 14
#define HUF_DECSIZE (1 << HUF_DECBITS)
#define HUF_DECMASK (HUF_DECSIZE - 1)
#define SHORT_ZEROCODE_RUN 59
#define LONG_ZEROCODE_RUN 63
#define SHORTEST_LONG_RUN (2 + LONG_ZEROCODE_RUN - SHORT_ZEROCODE_RUN)

typedef struct {
    uint32_t *p;  /* long-code symbol list */
    uint32_t lit; /* short: symbol; long: count */
    uint32_t len; /* code length (0 => long-code bucket) */
} HufDec;

typedef struct {
    uint16_t *start;
    int nx, ny, size;
} PizChan;

/* ---- range LUT ------------------------------------------------------------ */

static uint16_t reverse_lut_from_bitmap(const uint8_t *bitmap, uint16_t *lut) {
    int k = 0, i;
    for (i = 0; i < PIZ_USHORT_RANGE; ++i)
        if (i == 0 || (bitmap[i >> 3] & (1 << (i & 7)))) lut[k++] = (uint16_t)i;
    {
        uint16_t n = (uint16_t)(k - 1);
        while (k < PIZ_USHORT_RANGE) lut[k++] = 0;
        return n;
    }
}

static void apply_lut(const uint16_t *lut, uint16_t *data, size_t n) {
    size_t i;
    for (i = 0; i < n; ++i) data[i] = lut[data[i]];
}

/* ---- inverse wavelet (faithful port of OpenEXR wdec14/wdec16/wav2Decode) -- */

#define WAV_NBITS 16
#define WAV_A_OFFSET (1 << (WAV_NBITS - 1))
#define WAV_MOD_MASK ((1 << WAV_NBITS) - 1)

static void wdec14(uint16_t l, uint16_t h, uint16_t *a, uint16_t *b) {
    int hi = (int16_t)h;
    int ai = (int16_t)l + (hi & 1) + (hi >> 1);
    *a = (uint16_t)ai;
    *b = (uint16_t)(ai - hi);
}

static void wdec16(uint16_t l, uint16_t h, uint16_t *a, uint16_t *b) {
    int m = l, d = h;
    int bb = (m - (d >> 1)) & WAV_MOD_MASK;
    int aa = (d + bb - WAV_A_OFFSET) & WAV_MOD_MASK;
    *b = (uint16_t)bb;
    *a = (uint16_t)aa;
}

static void wav2_decode(uint16_t *in, int nx, int ox, int ny, int oy,
                        uint16_t mx) {
    int w14 = (mx < (1 << 14));
    int n = (nx > ny) ? ny : nx;
    int p = 1, p2;
    uint16_t i00, i01, i10, i11;

    while (p <= n) p <<= 1;
    p >>= 1;
    p2 = p;
    p >>= 1;

    while (p >= 1) {
        uint16_t *py = in;
        uint16_t *ey = in + oy * (ny - p2);
        int oy1 = oy * p, oy2 = oy * p2, ox1 = ox * p, ox2 = ox * p2;

        for (; py <= ey; py += oy2) {
            uint16_t *px = py;
            uint16_t *ex = py + ox * (nx - p2);
            for (; px <= ex; px += ox2) {
                uint16_t *p01 = px + ox1;
                uint16_t *p10 = px + oy1;
                uint16_t *p11 = p10 + ox1;
                if (w14) {
                    wdec14(*px, *p10, &i00, &i10);
                    wdec14(*p01, *p11, &i01, &i11);
                    wdec14(i00, i01, px, p01);
                    wdec14(i10, i11, p10, p11);
                } else {
                    wdec16(*px, *p10, &i00, &i10);
                    wdec16(*p01, *p11, &i01, &i11);
                    wdec16(i00, i01, px, p01);
                    wdec16(i10, i11, p10, p11);
                }
            }
            if (nx & p) {
                uint16_t *p10 = px + oy1;
                if (w14)
                    wdec14(*px, *p10, &i00, p10);
                else
                    wdec16(*px, *p10, &i00, p10);
                *px = i00;
            }
        }
        if (ny & p) {
            uint16_t *px = py;
            uint16_t *ex = py + ox * (nx - p2);
            for (; px <= ex; px += ox2) {
                uint16_t *p01 = px + ox1;
                if (w14)
                    wdec14(*px, *p01, &i00, p01);
                else
                    wdec16(*px, *p01, &i00, p01);
                *px = i00;
            }
        }
        p2 = p;
        p >>= 1;
    }
}

/* ---- Huffman -------------------------------------------------------------- */

static void hgetchar(uint64_t *c, int *lc, const uint8_t **in) {
    *c = (*c << 8) | (uint64_t)(**in);
    (*in)++;
    *lc += 8;
}
static void hrefill(uint64_t *c, int *lc, const uint8_t **in, const uint8_t *e) {
    while (*lc <= 48 && *in < e) {
        *c = (*c << 8) | (uint64_t)(**in);
        (*in)++;
        *lc += 8;
    }
}
/* Bounds-aware bit reader: refills from [*in, end) and zero-fills once the
 * input is exhausted, so a crafted Huffman table cannot read past `end`. */
static uint64_t hgetbits(int nb, uint64_t *c, int *lc, const uint8_t **in,
                         const uint8_t *end) {
    while (*lc < nb) {
        if (*in < end) {
            *c = (*c << 8) | (uint64_t)(**in);
            (*in)++;
        } else {
            *c = (*c << 8); /* zero-fill past the end */
        }
        *lc += 8;
    }
    *lc -= nb;
    return (*c >> *lc) & ((UINT64_C(1) << nb) - 1u);
}

/* Only hcode[im..iM] can be non-zero (symbols outside that span have length 0);
 * the length-0 count n[0] is never used, so restricting both full-table scans to
 * [im..iM] is equivalent and avoids walking all 65537 slots per block. */
static void canonical_code_table(int64_t *hcode, int im, int iM) {
    int64_t n[59];
    int i;
    int64_t c = 0;
    for (i = 0; i <= 58; ++i) n[i] = 0;
    for (i = im; i <= iM; ++i) n[hcode[i]]++;
    for (i = 58; i > 0; --i) {
        int64_t nc = ((c + n[i]) >> 1);
        n[i] = c;
        c = nc;
    }
    for (i = im; i <= iM; ++i) {
        int l = (int)hcode[i];
        if (l > 0) hcode[i] = l | (n[l]++ << 6);
    }
}

static int unpack_enc_table(const uint8_t **pcode, int ni, int im, int iM,
                            int64_t *hcode) {
    const uint8_t *p = *pcode;
    const uint8_t *end = *pcode + ni;
    uint64_t c = 0;
    int lc = 0;
    int im0 = im;
    memset(hcode, 0, sizeof(int64_t) * PIZ_HUF_ENCSIZE);
    for (; im <= iM; im++) {
        int64_t l;
        if (p - *pcode >= ni) return 0;
        l = (int64_t)hgetbits(6, &c, &lc, &p, end);
        hcode[im] = l;
        if (l == (int64_t)LONG_ZEROCODE_RUN) {
            int zerun;
            if (p - *pcode > ni) return 0;
            zerun = (int)hgetbits(8, &c, &lc, &p, end) + SHORTEST_LONG_RUN;
            if (im + zerun > iM + 1) return 0;
            while (zerun--) hcode[im++] = 0;
            im--;
        } else if (l >= (int64_t)SHORT_ZEROCODE_RUN) {
            int zerun = (int)(l - SHORT_ZEROCODE_RUN + 2);
            if (im + zerun > iM + 1) return 0;
            while (zerun--) hcode[im++] = 0;
            im--;
        }
    }
    *pcode = p;
    canonical_code_table(hcode, im0, iM);
    return 1;
}

static int build_dec_table(const exr_allocator *a, const int64_t *hcode, int im,
                           int iM, HufDec *hdecod, uint32_t **long_storage) {
    uint32_t *counts;
    size_t total_long = 0;
    int i, sym;

    counts = (uint32_t *)exr_calloc(a, HUF_DECSIZE, sizeof(uint32_t));
    if (!counts) return 0;
    for (i = 0; i < HUF_DECSIZE; ++i) {
        hdecod[i].len = 0;
        hdecod[i].lit = 0;
        hdecod[i].p = NULL;
    }
    for (i = im; i <= iM; ++i) {
        uint64_t code_val = ((uint64_t)hcode[i]) >> 6;
        int l = (int)(hcode[i] & 63);
        if (l == 0) continue;
        if (code_val >> l) { exr_free(a, counts); return 0; }
        if (l > HUF_DECBITS) {
            size_t base = (size_t)(code_val >> (l - HUF_DECBITS));
            if (hdecod[base].len) { exr_free(a, counts); return 0; }
            counts[base]++;
        } else {
            size_t base = (size_t)(code_val << (HUF_DECBITS - l));
            size_t fill = (size_t)1u << (HUF_DECBITS - l), k;
            for (k = 0; k < fill; ++k) {
                HufDec *pl = &hdecod[base + k];
                if (counts[base + k] != 0 || pl->len || pl->p) {
                    exr_free(a, counts);
                    return 0;
                }
                pl->len = (uint32_t)l;
                pl->lit = (uint32_t)i;
            }
        }
    }
    for (i = 0; i < HUF_DECSIZE; ++i) {
        if (total_long > SIZE_MAX - counts[i]) { exr_free(a, counts); return 0; }
        total_long += counts[i];
    }
    *long_storage = NULL;
    if (total_long > 0) {
        uint32_t *storage = (uint32_t *)exr_malloc(a, total_long * sizeof(uint32_t));
        if (!storage) { exr_free(a, counts); return 0; }
        *long_storage = storage;
        for (i = 0; i < HUF_DECSIZE; ++i) {
            if (counts[i] != 0) {
                hdecod[i].p = storage;
                hdecod[i].lit = counts[i];
                storage += counts[i];
                counts[i] = 0;
            }
        }
    }
    for (sym = im; sym <= iM; ++sym) {
        uint64_t code_val = ((uint64_t)hcode[sym]) >> 6;
        int l = (int)(hcode[sym] & 63);
        if (l > HUF_DECBITS) {
            size_t base = (size_t)(code_val >> (l - HUF_DECBITS));
            hdecod[base].p[counts[base]++] = (uint32_t)sym;
        }
    }
    exr_free(a, counts);
    return 1;
}

static int get_code(int po, int rlc, uint64_t *c, int *lc, const uint8_t **in,
                    const uint8_t *ie, uint16_t **out, const uint16_t *ob,
                    const uint16_t *oe) {
    if (po == rlc) {
        uint8_t cs;
        uint16_t s;
        if (*lc < 8) {
            if (*in >= ie) return 0;
            hgetchar(c, lc, in);
        }
        *lc -= 8;
        cs = (uint8_t)((*c >> *lc) & 0xffu);
        if (*out + cs > oe) return 0;
        if ((*out - 1) < ob) return 0;
        s = (*out)[-1];
        while (cs-- > 0) *(*out)++ = s;
    } else if (*out < oe) {
        *(*out)++ = (uint16_t)po;
    } else {
        return 0;
    }
    return 1;
}

static int huf_uncompress(const exr_allocator *a, const uint8_t *in,
                          size_t in_len, uint16_t *out, size_t out_len) {
    uint32_t im_val, iM_val, nBits_val;
    int im, iM, nBits, ni, rlc;
    int64_t *hcode = NULL;
    HufDec *hdecod = NULL;
    uint32_t *long_storage = NULL;
    const uint8_t *ptr, *ie;
    uint64_t c = 0;
    int lc = 0;
    uint16_t *outb, *outp, *oe;
    int ret = 0;

    if (out_len == 0) return in_len == 0;
    if (in_len < 20) return 0;
    memcpy(&im_val, in, 4);
    memcpy(&iM_val, in + 4, 4);
    memcpy(&nBits_val, in + 12, 4);
    im = (int)im_val;
    iM = (int)iM_val;
    nBits = (int)nBits_val;
    if (im < 0 || im >= PIZ_HUF_ENCSIZE || iM < 0 || iM >= PIZ_HUF_ENCSIZE ||
        im > iM)
        return 0;

    hcode = (int64_t *)exr_malloc(a, sizeof(int64_t) * PIZ_HUF_ENCSIZE);
    hdecod = (HufDec *)exr_malloc(a, sizeof(HufDec) * HUF_DECSIZE);
    if (!hcode || !hdecod) goto cleanup;

    ptr = in + 20;
    ni = (int)(in_len - 20);
    if (!unpack_enc_table(&ptr, ni, im, iM, hcode)) goto cleanup;

    ni = (int)(in_len - (size_t)(ptr - in));
    if (nBits < 0 || nBits > 8 * ni) goto cleanup;
    if (!build_dec_table(a, hcode, im, iM, hdecod, &long_storage)) goto cleanup;

    rlc = iM;
    outb = out;
    outp = out;
    oe = out + out_len;
    ie = ptr + (nBits + 7) / 8;
    hrefill(&c, &lc, &ptr, ie);

    while (ptr < ie || lc >= HUF_DECBITS) {
        if (lc < HUF_DECBITS && ptr < ie) hrefill(&c, &lc, &ptr, ie);
        while (lc >= HUF_DECBITS) {
            const HufDec *pl = &hdecod[(c >> (lc - HUF_DECBITS)) & HUF_DECMASK];
            if (pl->len) {
                lc -= (int)pl->len;
                /* Hot path: most symbols are plain literals (not the run-length
                 * code), which get_code() would emit as a single store. Inline
                 * that case to avoid a 9-argument call per decoded symbol; only
                 * the actual RLE marker falls through to get_code(). */
                if ((int)pl->lit != rlc) {
                    if (outp >= oe) goto cleanup;
                    *outp++ = (uint16_t)pl->lit;
                } else if (!get_code((int)pl->lit, rlc, &c, &lc, &ptr, ie, &outp,
                                     outb, oe)) {
                    goto cleanup;
                }
            } else {
                uint32_t j;
                if (!pl->p) goto cleanup;
                for (j = 0; j < pl->lit; ++j) {
                    int l = (int)(hcode[pl->p[j]] & 63);
                    while (lc < l && ptr < ie) hgetchar(&c, &lc, &ptr);
                    if (lc >= l) {
                        uint64_t cv = ((uint64_t)hcode[pl->p[j]]) >> 6;
                        if (cv == ((c >> (lc - l)) & ((UINT64_C(1) << l) - 1u))) {
                            lc -= l;
                            if (!get_code((int)pl->p[j], rlc, &c, &lc, &ptr, ie,
                                          &outp, outb, oe))
                                goto cleanup;
                            break;
                        }
                    }
                }
                if (j == pl->lit) goto cleanup;
            }
            if (lc < HUF_DECBITS && ptr < ie) hrefill(&c, &lc, &ptr, ie);
        }
    }

    {
        int i = (8 - nBits) & 7;
        c >>= i;
        lc -= i;
        while (lc > 0) {
            const HufDec *pl = &hdecod[(c << (HUF_DECBITS - lc)) & HUF_DECMASK];
            if (pl->len) {
                lc -= (int)pl->len;
                if (!get_code((int)pl->lit, rlc, &c, &lc, &ptr, ie, &outp, outb,
                              oe))
                    goto cleanup;
            } else {
                goto cleanup;
            }
        }
    }
    ret = 1;

cleanup:
    exr_free(a, long_storage);
    exr_free(a, hdecod);
    exr_free(a, hcode);
    return ret;
}

/* ---- orchestrator --------------------------------------------------------- */

exr_result exr_piz_decompress(const exr_codec_ctx *ctx, const uint8_t *src,
                              size_t src_size, uint8_t *dst, size_t dst_size) {
    const exr_allocator *a = ctx->alloc;
    int xmin = ctx->x, xmax = ctx->x + ctx->width - 1;
    const uint8_t *ptr = src, *ptr_end = src + src_size;
    uint8_t bitmap[PIZ_BITMAP_SIZE];
    uint16_t *rev_lut = NULL, *tmp = NULL;
    uint16_t minNonZero, maxNonZero, maxValue;
    size_t total = 0;
    PizChan *cd = NULL;
    uint16_t *chan_ptr;
    uint32_t huf_length;
    int c, line;
    uint8_t *out;
    int *emitted = NULL;
    exr_result rc = EXR_SUCCESS;

    if (ctx->num_channels <= 0 || ctx->num_channels > 1024)
        return EXR_ERROR_CORRUPT;
    if (src_size < 4) return EXR_ERROR_CORRUPT;

    minNonZero = exr_rd_u16(ptr);
    maxNonZero = exr_rd_u16(ptr + 2);
    ptr += 4;
    memset(bitmap, 0, sizeof(bitmap));
    if (maxNonZero >= PIZ_BITMAP_SIZE) return EXR_ERROR_CORRUPT;
    if (minNonZero <= maxNonZero) {
        size_t blen = (size_t)(maxNonZero - minNonZero + 1);
        if ((size_t)(ptr_end - ptr) < blen) return EXR_ERROR_CORRUPT;
        if ((size_t)minNonZero + blen > PIZ_BITMAP_SIZE) return EXR_ERROR_CORRUPT;
        memcpy(bitmap + minNonZero, ptr, blen);
        ptr += blen;
    } else if (!(minNonZero == (PIZ_BITMAP_SIZE - 1) && maxNonZero == 0)) {
        return EXR_ERROR_CORRUPT;
    }

    rev_lut = (uint16_t *)exr_malloc(a, sizeof(uint16_t) * PIZ_USHORT_RANGE);
    cd = (PizChan *)exr_calloc(a, (size_t)ctx->num_channels, sizeof(PizChan));
    emitted = (int *)exr_calloc(a, (size_t)ctx->num_channels, sizeof(int));
    if (!rev_lut || !cd || !emitted) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
    maxValue = reverse_lut_from_bitmap(bitmap, rev_lut);

    /* total uint16 samples across channels (FLOAT/UINT = 2 lanes, HALF = 1) */
    for (c = 0; c < ctx->num_channels; ++c) {
        int xs = ctx->channels[c].x_sampling, ys = ctx->channels[c].y_sampling;
        int nx, ny, lanes;
        size_t s;
        if (xs <= 0 || ys <= 0) { rc = EXR_ERROR_CORRUPT; goto done; }
        nx = exr_num_samples(xmin, xmax, xs);
        ny = exr_num_samples(ctx->y, ctx->y + ctx->num_lines - 1, ys);
        if (nx < 0) nx = 0;
        if (ny < 0) ny = 0;
        lanes = (ctx->channels[c].pixel_type == EXR_PIXEL_HALF) ? 1 : 2;
        cd[c].start = NULL;
        cd[c].nx = nx;
        cd[c].ny = ny;
        cd[c].size = lanes;
        if (exr_mul_ovf((size_t)nx * (size_t)ny, (size_t)lanes, &s)) {
            rc = EXR_ERROR_CORRUPT;
            goto done;
        }
        s = (size_t)nx * (size_t)ny * (size_t)lanes;
        if (exr_add_ovf(total, s, &total)) { rc = EXR_ERROR_CORRUPT; goto done; }
    }
    if (total == 0 || total > SIZE_MAX / sizeof(uint16_t)) {
        rc = EXR_ERROR_CORRUPT;
        goto done;
    }

    tmp = (uint16_t *)exr_calloc(a, total, sizeof(uint16_t));
    if (!tmp) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }

    if ((size_t)(ptr_end - ptr) < 4) { rc = EXR_ERROR_CORRUPT; goto done; }
    huf_length = exr_rd_u32(ptr);
    ptr += 4;
    if ((size_t)(ptr_end - ptr) < (size_t)huf_length) {
        rc = EXR_ERROR_CORRUPT;
        goto done;
    }
    if (!huf_uncompress(a, ptr, huf_length, tmp, total)) {
        rc = EXR_ERROR_CORRUPT;
        goto done;
    }

    chan_ptr = tmp;
    for (c = 0; c < ctx->num_channels; ++c) {
        cd[c].start = chan_ptr;
        chan_ptr += (size_t)cd[c].nx * cd[c].ny * cd[c].size;
    }
    for (c = 0; c < ctx->num_channels; ++c) {
        int lane;
        for (lane = 0; lane < cd[c].size; ++lane)
            wav2_decode(cd[c].start + lane, cd[c].nx, cd[c].size, cd[c].ny,
                        cd[c].nx * cd[c].size, maxValue);
    }
    apply_lut(rev_lut, tmp, total);

    /* emit canonical block layout (per line, per sampled channel, dense) */
    out = dst;
    for (line = 0; line < ctx->num_lines; ++line) {
        int yy = ctx->y + line;
        for (c = 0; c < ctx->num_channels; ++c) {
            int ys = ctx->channels[c].y_sampling;
            int row, x, line_samples;
            const uint16_t *ld;
            if ((yy % ys) != 0) continue;
            row = emitted[c]++;
            if (row >= cd[c].ny) { rc = EXR_ERROR_CORRUPT; goto done; }
            line_samples = cd[c].nx * cd[c].size;
            ld = cd[c].start + (size_t)row * line_samples;
            if (out + (size_t)line_samples * 2 > dst + dst_size) {
                rc = EXR_ERROR_CORRUPT;
                goto done;
            }
            for (x = 0; x < line_samples; ++x) {
                out[0] = (uint8_t)(ld[x] & 0xff);
                out[1] = (uint8_t)(ld[x] >> 8);
                out += 2;
            }
        }
    }
    if ((size_t)(out - dst) != dst_size) rc = EXR_ERROR_CORRUPT;

done:
    exr_free(a, rev_lut);
    exr_free(a, tmp);
    exr_free(a, cd);
    exr_free(a, emitted);
    return rc;
}

/* ===========================================================================
 * PIZ encode (forward wavelet + range LUT + canonical Huffman).
 * Ported from the legacy single-header tinyexr (OpenEXR-derived). The STL heap
 * in hufBuildEncTable is reimplemented as a plain C index min-heap.
 * ========================================================================= */

static void wenc14(uint16_t a, uint16_t b, uint16_t *l, uint16_t *h) {
    int16_t as = (int16_t)a, bs = (int16_t)b;
    *l = (uint16_t)(int16_t)((as + bs) >> 1);
    *h = (uint16_t)(int16_t)(as - bs);
}
static void wenc16(uint16_t a, uint16_t b, uint16_t *l, uint16_t *h) {
    int ao = (a + WAV_A_OFFSET) & WAV_MOD_MASK;
    int m = (ao + b) >> 1;
    int d = ao - b;
    if (d < 0) m = (m + WAV_A_OFFSET) & WAV_MOD_MASK;
    d &= WAV_MOD_MASK;
    *l = (uint16_t)m;
    *h = (uint16_t)d;
}

static void wav2_encode(uint16_t *in, int nx, int ox, int ny, int oy,
                        uint16_t mx) {
    int w14 = (mx < (1 << 14));
    int n = (nx > ny) ? ny : nx;
    int p = 1, p2 = 2;
    uint16_t i00, i01, i10, i11;
    while (p2 <= n) {
        uint16_t *py = in;
        uint16_t *ey = in + oy * (ny - p2);
        int oy1 = oy * p, oy2 = oy * p2, ox1 = ox * p, ox2 = ox * p2;
        for (; py <= ey; py += oy2) {
            uint16_t *px = py;
            uint16_t *ex = py + ox * (nx - p2);
            for (; px <= ex; px += ox2) {
                uint16_t *p01 = px + ox1, *p10 = px + oy1, *p11 = p10 + ox1;
                if (w14) {
                    wenc14(*px, *p01, &i00, &i01);
                    wenc14(*p10, *p11, &i10, &i11);
                    wenc14(i00, i10, px, p10);
                    wenc14(i01, i11, p01, p11);
                } else {
                    wenc16(*px, *p01, &i00, &i01);
                    wenc16(*p10, *p11, &i10, &i11);
                    wenc16(i00, i10, px, p10);
                    wenc16(i01, i11, p01, p11);
                }
            }
            if (nx & p) {
                uint16_t *p10 = px + oy1;
                if (w14) wenc14(*px, *p10, &i00, p10);
                else wenc16(*px, *p10, &i00, p10);
                *px = i00;
            }
        }
        if (ny & p) {
            uint16_t *px = py;
            uint16_t *ex = py + ox * (nx - p2);
            for (; px <= ex; px += ox2) {
                uint16_t *p01 = px + ox1;
                if (w14) wenc14(*px, *p01, &i00, p01);
                else wenc16(*px, *p01, &i00, p01);
                *px = i00;
            }
        }
        p = p2;
        p2 <<= 1;
    }
}

static void bitmap_from_data(const uint16_t *data, size_t n, uint8_t *bitmap,
                             uint16_t *minNZ, uint16_t *maxNZ) {
    size_t i;
    int mn = PIZ_BITMAP_SIZE - 1, mx = 0;
    memset(bitmap, 0, PIZ_BITMAP_SIZE);
    for (i = 0; i < n; ++i) bitmap[data[i] >> 3] |= (uint8_t)(1 << (data[i] & 7));
    bitmap[0] &= (uint8_t)~1; /* zero never explicitly stored */
    for (i = 0; i < PIZ_BITMAP_SIZE; ++i)
        if (bitmap[i]) {
            if (mn > (int)i) mn = (int)i;
            if (mx < (int)i) mx = (int)i;
        }
    *minNZ = (uint16_t)mn;
    *maxNZ = (uint16_t)mx;
}

static uint16_t forward_lut_from_bitmap(const uint8_t *bitmap, uint16_t *lut) {
    int i, k = 0;
    for (i = 0; i < PIZ_USHORT_RANGE; ++i) {
        if (i == 0 || (bitmap[i >> 3] & (1 << (i & 7))))
            lut[i] = (uint16_t)k++;
        else
            lut[i] = 0;
    }
    return (uint16_t)(k - 1);
}

/* MSB-first bit writer for the PIZ Huffman stream. */
typedef struct {
    uint8_t *p;
    uint64_t c;
    int lc;
} pizbw;
static void piz_outbits(pizbw *w, int nbits, uint64_t bits) {
    w->c <<= nbits;
    w->lc += nbits;
    w->c |= bits;
    while (w->lc >= 8) *w->p++ = (uint8_t)(w->c >> (w->lc -= 8));
}

/* index min-heap keyed on frq[idx] */
static void heap_sift_down(int *heap, int n, int i, const int64_t *frq) {
    for (;;) {
        int l = 2 * i + 1, r = 2 * i + 2, s = i, t;
        if (l < n && frq[heap[l]] < frq[heap[s]]) s = l;
        if (r < n && frq[heap[r]] < frq[heap[s]]) s = r;
        if (s == i) break;
        t = heap[i]; heap[i] = heap[s]; heap[s] = t;
        i = s;
    }
}
static void heap_sift_up(int *heap, int i, const int64_t *frq) {
    while (i > 0) {
        int parent = (i - 1) / 2, t;
        if (frq[heap[parent]] <= frq[heap[i]]) break;
        t = heap[i]; heap[i] = heap[parent]; heap[parent] = t;
        i = parent;
    }
}

static int huf_build_enc_table(const exr_allocator *a, int64_t *frq, int *pim,
                               int *piM) {
    int *hlink = (int *)exr_malloc(a, sizeof(int) * PIZ_HUF_ENCSIZE);
    int *heap = (int *)exr_malloc(a, sizeof(int) * PIZ_HUF_ENCSIZE);
    int64_t *scode = (int64_t *)exr_calloc(a, PIZ_HUF_ENCSIZE, sizeof(int64_t));
    int im = 0, iM = 0, nf = 0, i, k;
    int ret = 0;
    if (!hlink || !heap || !scode) goto done;

    while (im < PIZ_HUF_ENCSIZE && !frq[im]) im++;
    if (im >= PIZ_HUF_ENCSIZE) goto done;
    for (i = im; i < PIZ_HUF_ENCSIZE; ++i) {
        hlink[i] = i;
        if (frq[i]) { heap[nf++] = i; iM = i; }
    }
    iM++;
    frq[iM] = 1;
    heap[nf++] = iM;

    for (k = nf / 2 - 1; k >= 0; --k) heap_sift_down(heap, nf, k, frq);

    while (nf > 1) {
        int mm = heap[0];
        int m, j;
        heap[0] = heap[--nf];
        heap_sift_down(heap, nf, 0, frq);
        m = heap[0];
        heap[0] = heap[--nf];
        heap_sift_down(heap, nf, 0, frq);
        frq[m] += frq[mm];
        heap[nf] = m;
        heap_sift_up(heap, nf, frq);
        nf++;

        for (j = m;; j = hlink[j]) {
            scode[j]++;
            if (scode[j] > 58) goto done;
            if (hlink[j] == j) { hlink[j] = mm; break; }
        }
        for (j = mm;; j = hlink[j]) {
            scode[j]++;
            if (scode[j] > 58) goto done;
            if (hlink[j] == j) break;
        }
    }

    canonical_code_table(scode, im, iM);
    memcpy(frq, scode, sizeof(int64_t) * PIZ_HUF_ENCSIZE);
    *pim = im;
    *piM = iM;
    ret = 1;
done:
    exr_free(a, hlink);
    exr_free(a, heap);
    exr_free(a, scode);
    return ret;
}

static int64_t huf_length(int64_t code) { return code & 63; }
static int64_t huf_code(int64_t code) { return code >> 6; }

static void huf_pack_enc_table(const int64_t *hcode, int im, int iM,
                               uint8_t **pcode) {
    pizbw w;
    w.p = *pcode;
    w.c = 0;
    w.lc = 0;
    for (; im <= iM; ++im) {
        int l = (int)huf_length(hcode[im]);
        if (l == 0) {
            int zerun = 1;
            while ((im < iM) && (zerun < 255 + SHORTEST_LONG_RUN)) {
                if (huf_length(hcode[im + 1]) > 0) break;
                ++im;
                ++zerun;
            }
            if (zerun >= 2) {
                if (zerun >= SHORTEST_LONG_RUN) {
                    piz_outbits(&w, 6, LONG_ZEROCODE_RUN);
                    piz_outbits(&w, 8, (uint64_t)(zerun - SHORTEST_LONG_RUN));
                } else {
                    piz_outbits(&w, 6, (uint64_t)(SHORT_ZEROCODE_RUN + zerun - 2));
                }
                continue;
            }
        }
        piz_outbits(&w, 6, (uint64_t)l);
    }
    if (w.lc > 0) *w.p++ = (uint8_t)(w.c << (8 - w.lc));
    *pcode = w.p;
}

static void huf_output_code(pizbw *w, int64_t code) {
    piz_outbits(w, (int)huf_length(code), (uint64_t)huf_code(code));
}
static void huf_send_code(pizbw *w, int64_t sCode, int runCount, int64_t rCode) {
    if (huf_length(sCode) + huf_length(rCode) + 8 < huf_length(sCode) * runCount) {
        huf_output_code(w, sCode);
        huf_output_code(w, rCode);
        piz_outbits(w, 8, (uint64_t)runCount);
    } else {
        while (runCount-- >= 0) huf_output_code(w, sCode);
    }
}

static int huf_encode(const int64_t *hcode, const uint16_t *in, int ni, int rlc,
                      uint8_t *out) {
    pizbw w;
    int s = in[0], cs = 0, i;
    w.p = out;
    w.c = 0;
    w.lc = 0;
    for (i = 1; i < ni; ++i) {
        if (s == in[i] && cs < 255) {
            cs++;
        } else {
            huf_send_code(&w, hcode[s], cs, hcode[rlc]);
            cs = 0;
        }
        s = in[i];
    }
    huf_send_code(&w, hcode[s], cs, hcode[rlc]);
    if (w.lc) *w.p = (uint8_t)((w.c << (8 - w.lc)) & 0xff);
    return (int)((w.p - out) * 8 + w.lc);
}

/* Compress nRaw uint16 values; returns total bytes written to `out`. */
static int huf_compress(const exr_allocator *a, const uint16_t *raw, int nRaw,
                        uint8_t *out, int *err) {
    int64_t *freq;
    int im = 0, iM = 0;
    uint8_t *tableStart, *tableEnd, *dataStart;
    int tableLength, nBits, data_length, i;

    *err = 0;
    if (nRaw == 0) return 0;
    freq = (int64_t *)exr_calloc(a, PIZ_HUF_ENCSIZE, sizeof(int64_t));
    if (!freq) { *err = 1; return 0; }
    for (i = 0; i < nRaw; ++i) freq[raw[i]]++;

    if (!huf_build_enc_table(a, freq, &im, &iM)) {
        exr_free(a, freq);
        *err = 1;
        return 0;
    }

    tableStart = out + 20;
    tableEnd = tableStart;
    huf_pack_enc_table(freq, im, iM, &tableEnd);
    tableLength = (int)(tableEnd - tableStart);

    dataStart = tableEnd;
    nBits = huf_encode(freq, raw, nRaw, iM, dataStart);
    data_length = (nBits + 7) / 8;

    exr_wr_u32(out, (uint32_t)im);
    exr_wr_u32(out + 4, (uint32_t)iM);
    exr_wr_u32(out + 8, (uint32_t)tableLength);
    exr_wr_u32(out + 12, (uint32_t)nBits);
    exr_wr_u32(out + 16, 0);

    exr_free(a, freq);
    return (int)(dataStart + data_length - out);
}

exr_result exr_piz_compress(const exr_codec_ctx *ctx, const uint8_t *block,
                            size_t n, uint8_t **out_data, size_t *out_size) {
    const exr_allocator *a = ctx->alloc;
    int xmin = ctx->x, xmax = ctx->x + ctx->width - 1;
    size_t total = n / 2;
    uint16_t *tmp = NULL, *lut = NULL;
    uint8_t bitmap[PIZ_BITMAP_SIZE];
    uint16_t minNZ, maxNZ, maxValue;
    PizChan *cd = NULL;
    uint16_t *chan_ptr;
    int *emitted = NULL;
    uint8_t *out = NULL;
    size_t cap, pos;
    int c, line, herr = 0, hlen;
    exr_result rc = EXR_SUCCESS;

    *out_data = NULL;
    *out_size = 0;
    if (ctx->num_channels <= 0 || ctx->num_channels > 1024 || (n & 1))
        return EXR_ERROR_INVALID_ARGUMENT;

    tmp = (uint16_t *)exr_calloc(a, total ? total : 1, sizeof(uint16_t));
    lut = (uint16_t *)exr_malloc(a, sizeof(uint16_t) * PIZ_USHORT_RANGE);
    cd = (PizChan *)exr_calloc(a, (size_t)ctx->num_channels, sizeof(PizChan));
    emitted = (int *)exr_calloc(a, (size_t)ctx->num_channels, sizeof(int));
    if (!tmp || !lut || !cd || !emitted) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }

    /* per-channel planar layout */
    for (c = 0; c < ctx->num_channels; ++c) {
        int xs = ctx->channels[c].x_sampling, ys = ctx->channels[c].y_sampling;
        int nx = exr_num_samples(xmin, xmax, xs);
        int ny = exr_num_samples(ctx->y, ctx->y + ctx->num_lines - 1, ys);
        if (nx < 0) nx = 0;
        if (ny < 0) ny = 0;
        cd[c].nx = nx;
        cd[c].ny = ny;
        cd[c].size = (ctx->channels[c].pixel_type == EXR_PIXEL_HALF) ? 1 : 2;
    }
    chan_ptr = tmp;
    for (c = 0; c < ctx->num_channels; ++c) {
        cd[c].start = chan_ptr;
        chan_ptr += (size_t)cd[c].nx * cd[c].ny * cd[c].size;
    }

    /* reorganize the canonical block (dense per line/channel) into planar tmp */
    {
        const uint8_t *bp = block;
        const uint8_t *bend = block + n;
        for (line = 0; line < ctx->num_lines; ++line) {
            int yy = ctx->y + line;
            for (c = 0; c < ctx->num_channels; ++c) {
                int ys = ctx->channels[c].y_sampling;
                int row, ls = cd[c].nx * cd[c].size, x;
                uint16_t *dstp;
                if ((yy % ys) != 0) continue;
                row = emitted[c]++;
                if (row >= cd[c].ny) { rc = EXR_ERROR_CORRUPT; goto done; }
                if (bp + (size_t)ls * 2 > bend) { rc = EXR_ERROR_CORRUPT; goto done; }
                dstp = cd[c].start + (size_t)row * ls;
                for (x = 0; x < ls; ++x) {
                    dstp[x] = exr_rd_u16(bp);
                    bp += 2;
                }
            }
        }
    }

    bitmap_from_data(tmp, total, bitmap, &minNZ, &maxNZ);
    maxValue = forward_lut_from_bitmap(bitmap, lut);
    apply_lut(lut, tmp, total);

    for (c = 0; c < ctx->num_channels; ++c) {
        int lane;
        for (lane = 0; lane < cd[c].size; ++lane)
            wav2_encode(cd[c].start + lane, cd[c].nx, cd[c].size, cd[c].ny,
                        cd[c].nx * cd[c].size, maxValue);
    }

    /* output: minNZ(2) maxNZ(2) bitmap[min..max] hufLength(4) hufdata */
    cap = 4 + PIZ_BITMAP_SIZE + 4 + total * 2 + PIZ_HUF_ENCSIZE * 2 + 256;
    out = (uint8_t *)exr_malloc(a, cap);
    if (!out) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
    exr_wr_u16(out, minNZ);
    exr_wr_u16(out + 2, maxNZ);
    pos = 4;
    if (minNZ <= maxNZ) {
        size_t blen = (size_t)(maxNZ - minNZ + 1);
        memcpy(out + pos, bitmap + minNZ, blen);
        pos += blen;
    }
    hlen = huf_compress(a, tmp, (int)total, out + pos + 4, &herr);
    if (herr) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
    exr_wr_u32(out + pos, (uint32_t)hlen);
    pos += 4 + (size_t)hlen;

    if (pos >= n) { /* PIZ did not help: store the canonical block raw */
        exr_free(a, out);
        out = (uint8_t *)exr_malloc(a, n ? n : 1);
        if (!out) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
        memcpy(out, block, n);
        *out_data = out;
        *out_size = n;
        out = NULL;
        goto done;
    }
    *out_data = out;
    *out_size = pos;
    out = NULL;

done:
    exr_free(a, tmp);
    exr_free(a, lut);
    exr_free(a, cd);
    exr_free(a, emitted);
    exr_free(a, out);
    return rc;
}
