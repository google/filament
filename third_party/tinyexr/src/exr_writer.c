/*
 * TinyEXR - mid-level writer + high-level save.
 *
 * Phase 8: scanline (single + multipart) output for NONE / RLE / ZIP / ZIPS.
 * Tiled and lossy-codec writing build on the same serializer.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

/* ---- growable output buffer ---------------------------------------------- */

typedef struct {
    const exr_allocator *a;
    uint8_t *data;
    size_t len, cap;
    int err;
} obuf;

static void ob_reserve(obuf *b, size_t extra) {
    size_t need;
    if (b->err) return;
    if (exr_add_ovf(b->len, extra, &need)) { b->err = 1; return; }
    if (need <= b->cap) return;
    {
        size_t ncap = b->cap ? b->cap : 256;
        uint8_t *nd;
        while (ncap < need) {
            if (ncap > (SIZE_MAX / 2)) { b->err = 1; return; }
            ncap *= 2;
        }
        nd = (uint8_t *)exr_malloc(b->a, ncap);
        if (!nd) { b->err = 1; return; }
        if (b->data) {
            memcpy(nd, b->data, b->len);
            exr_free(b->a, b->data);
        }
        b->data = nd;
        b->cap = ncap;
    }
}
static void ob_bytes(obuf *b, const void *p, size_t n) {
    ob_reserve(b, n);
    if (b->err) return;
    memcpy(b->data + b->len, p, n);
    b->len += n;
}
static void ob_u8(obuf *b, uint8_t v) { ob_bytes(b, &v, 1); }
static void ob_u32(obuf *b, uint32_t v) {
    uint8_t t[4];
    exr_wr_u32(t, v);
    ob_bytes(b, t, 4);
}
static void ob_i32(obuf *b, int32_t v) { ob_u32(b, (uint32_t)v); }
static void ob_u64(obuf *b, uint64_t v) {
    uint8_t t[8];
    exr_wr_u64(t, v);
    ob_bytes(b, t, 8);
}
static void ob_cstr(obuf *b, const char *s) { ob_bytes(b, s, strlen(s) + 1); }

/* attribute: name\0 type\0 size(i32) data */
static void ob_attr(obuf *b, const char *name, const char *type,
                    const void *data, uint32_t size) {
    ob_cstr(b, name);
    ob_cstr(b, type);
    ob_i32(b, (int32_t)size);
    ob_bytes(b, data, size);
}

/* ---- channel sort -------------------------------------------------------- */

static void sort_channels(const exr_header *h, int *order) {
    int i, j, n = h->num_channels;
    for (i = 0; i < n; ++i) order[i] = i;
    for (i = 1; i < n; ++i) { /* insertion sort by name */
        int v = order[i];
        for (j = i - 1; j >= 0 &&
                        strcmp(h->channels[order[j]].name, h->channels[v].name) > 0;
             --j)
            order[j + 1] = order[j];
        order[j + 1] = v;
    }
}

/* ---- header serialization ------------------------------------------------ */

static const char *part_type_string(exr_part_type t) {
    switch (t) {
    case EXR_PART_SCANLINE: return "scanlineimage";
    case EXR_PART_TILED: return "tiledimage";
    case EXR_PART_DEEP_SCANLINE: return "deepscanline";
    case EXR_PART_DEEP_TILED: return "deeptile";
    }
    return "scanlineimage";
}

static void write_box2i(obuf *b, const char *name, const exr_box2i *w) {
    uint8_t t[16];
    exr_wr_i32(t, w->min_x);
    exr_wr_i32(t + 4, w->min_y);
    exr_wr_i32(t + 8, w->max_x);
    exr_wr_i32(t + 12, w->max_y);
    ob_attr(b, name, "box2i", t, 16);
}

/* Standard attributes the writer emits itself; custom-attribute round-trip
 * skips these so they are never duplicated. */
static int is_reserved_attr_name(const char *name) {
    static const char *const reserved[] = {
        "channels",         "compression",      "dataWindow",
        "displayWindow",    "lineOrder",        "pixelAspectRatio",
        "screenWindowCenter", "screenWindowWidth", "tiles",
        "name",             "type",             "chunkCount",
        "version",          "maxSamplesPerPixel"};
    size_t i;
    for (i = 0; i < sizeof(reserved) / sizeof(reserved[0]); ++i)
        if (strcmp(name, reserved[i]) == 0) return 1;
    return 0;
}

static void write_header(obuf *b, const exr_header *h, const int *order,
                         exr_compression comp, int multipart,
                         uint32_t chunk_count, int32_t max_samples) {
    /* channels */
    {
        obuf cl;
        int i;
        memset(&cl, 0, sizeof(cl));
        cl.a = b->a;
        for (i = 0; i < h->num_channels; ++i) {
            const exr_channel *c = &h->channels[order[i]];
            uint8_t t[16];
            ob_cstr(&cl, c->name);
            exr_wr_i32(t, (int32_t)c->pixel_type);
            t[4] = c->p_linear;
            t[5] = t[6] = t[7] = 0;
            exr_wr_i32(t + 8, c->x_sampling);
            exr_wr_i32(t + 12, c->y_sampling);
            ob_bytes(&cl, t, 16);
        }
        ob_u8(&cl, 0); /* terminator */
        if (cl.err) { b->err = 1; }
        else ob_attr(b, "channels", "chlist", cl.data, (uint32_t)cl.len);
        exr_free(b->a, cl.data);
    }

    {
        uint8_t c8 = (uint8_t)comp;
        ob_attr(b, "compression", "compression", &c8, 1);
    }
    write_box2i(b, "dataWindow", &h->data_window);
    {
        exr_box2i dw = h->display_window;
        if (dw.max_x < dw.min_x || dw.max_y < dw.min_y) dw = h->data_window;
        write_box2i(b, "displayWindow", &dw);
    }
    {
        uint8_t lo = (uint8_t)h->line_order;
        ob_attr(b, "lineOrder", "lineOrder", &lo, 1);
    }
    {
        uint8_t t[4];
        exr_wr_f32(t, h->pixel_aspect_ratio != 0 ? h->pixel_aspect_ratio : 1.0f);
        ob_attr(b, "pixelAspectRatio", "float", t, 4);
    }
    {
        uint8_t t[8];
        exr_wr_f32(t, h->screen_window_center_x);
        exr_wr_f32(t + 4, h->screen_window_center_y);
        ob_attr(b, "screenWindowCenter", "v2f", t, 8);
    }
    {
        uint8_t t[4];
        exr_wr_f32(t, h->screen_window_width != 0 ? h->screen_window_width : 1.0f);
        ob_attr(b, "screenWindowWidth", "float", t, 4);
    }
    if (h->tiled) {
        uint8_t t[9];
        exr_wr_u32(t, h->tile_x_size);
        exr_wr_u32(t + 4, h->tile_y_size);
        t[8] = (uint8_t)((h->level_mode & 0x0f) | ((h->rounding_mode & 0x0f) << 4));
        ob_attr(b, "tiles", "tiledesc", t, 9);
    }
    if (multipart) {
        const char *nm = h->name[0] ? h->name : "part";
        const char *ty = part_type_string(h->part_type);
        ob_attr(b, "name", "string", nm, (uint32_t)strlen(nm));
        ob_attr(b, "type", "string", ty, (uint32_t)strlen(ty));
        {
            uint8_t t[4];
            exr_wr_i32(t, (int32_t)chunk_count);
            ob_attr(b, "chunkCount", "int", t, 4);
        }
    }
    if (h->part_type == EXR_PART_DEEP_SCANLINE ||
        h->part_type == EXR_PART_DEEP_TILED) {
        uint8_t t[4];
        if (!multipart) {
            const char *ty = part_type_string(h->part_type);
            ob_attr(b, "type", "string", ty, (uint32_t)strlen(ty));
        }
        exr_wr_i32(t, 1);
        ob_attr(b, "version", "int", t, 4); /* deep data version */
        exr_wr_i32(t, max_samples);
        ob_attr(b, "maxSamplesPerPixel", "int", t, 4);
    }
    /* Emit any custom attributes attached by the caller (e.g. spectral
     * metadata), skipping the standard ones written above. */
    if (h->attrs) {
        uint32_t i;
        for (i = 0; i < h->attrs->count; ++i) {
            const exr_attr *at = &h->attrs->items[i];
            if (is_reserved_attr_name(at->name)) continue;
            ob_attr(b, at->name, at->type_name, at->data, at->size);
        }
    }
    ob_u8(b, 0); /* end of header */
}

/* ---- scanline gather ----------------------------------------------------- */

static void gather_scanline_block(const exr_header *h, const int *order,
                                  void *const *images, int y0, int nlines,
                                  uint8_t *block) {
    int xmin = h->data_window.min_x, xmax = h->data_window.max_x;
    int ymin = h->data_window.min_y;
    size_t off = 0;
    int line, oi;
    for (line = 0; line < nlines; ++line) {
        int yy = y0 + line;
        for (oi = 0; oi < h->num_channels; ++oi) {
            int c = order[oi];
            int xs = h->channels[c].x_sampling, ys = h->channels[c].y_sampling;
            size_t ps = exr_pixel_size(h->channels[c].pixel_type);
            int nx, cw, row;
            if ((yy % ys) != 0) continue;
            nx = exr_num_samples(xmin, xmax, xs);
            cw = nx;
            row = exr_num_samples(ymin, yy, ys) - 1;
            memcpy(block + off, (const uint8_t *)images[c] + (size_t)row * cw * ps,
                   (size_t)nx * ps);
            off += (size_t)nx * ps;
        }
    }
}

static void gather_tile_block(const exr_header *h, const int *order,
                              void *const *images, int abs_x0, int abs_y0,
                              int tile_w, int tile_h, uint8_t *block) {
    int xmin = h->data_window.min_x, xmax = h->data_window.max_x;
    int ymin = h->data_window.min_y;
    size_t off = 0;
    int row, oi;
    for (row = 0; row < tile_h; ++row) {
        int yy = abs_y0 + row;
        for (oi = 0; oi < h->num_channels; ++oi) {
            int c = order[oi];
            int xs = h->channels[c].x_sampling, ys = h->channels[c].y_sampling;
            size_t ps = exr_pixel_size(h->channels[c].pixel_type);
            int tnx, cw, ch_row, ch_col;
            if ((yy % ys) != 0) continue;
            tnx = exr_num_samples(abs_x0, abs_x0 + tile_w - 1, xs);
            if (tnx <= 0) continue;
            cw = exr_num_samples(xmin, xmax, xs);
            ch_row = exr_num_samples(ymin, yy, ys) - 1;
            ch_col = exr_num_samples(xmin, abs_x0 - 1, xs);
            memcpy(block + off,
                   (const uint8_t *)images[c] + ((size_t)ch_row * cw + ch_col) * ps,
                   (size_t)tnx * ps);
            off += (size_t)tnx * ps;
        }
    }
}

/* Gather a tile from a level image into the canonical block. level_img[c] is
 * the channel's sampled grid (exr_num_samples(0,lw-1,xs) wide). x0/y0 are the
 * tile's level-local pixel origin. */
static void gather_level_tile(const exr_header *h, const int *order,
                              void *const *level_img, int lw, int x0, int y0,
                              int tw, int th, uint8_t *block) {
    size_t off = 0;
    int row, oi;
    for (row = 0; row < th; ++row) {
        int yy = y0 + row;
        for (oi = 0; oi < h->num_channels; ++oi) {
            int c = order[oi];
            int xs = h->channels[c].x_sampling < 1 ? 1 : h->channels[c].x_sampling;
            int ys = h->channels[c].y_sampling < 1 ? 1 : h->channels[c].y_sampling;
            size_t ps = exr_pixel_size(h->channels[c].pixel_type);
            int gw, g0, cnt;
            if ((yy % ys) != 0) continue;
            gw = exr_num_samples(0, lw - 1, xs);    /* grid width */
            g0 = ((x0 + xs - 1) / xs);              /* first sampled col idx */
            cnt = exr_num_samples(x0, x0 + tw - 1, xs);
            if (cnt > 0)
                memcpy(block + off,
                       (const uint8_t *)level_img[c] +
                           ((size_t)(yy / ys) * gw + g0) * ps,
                       (size_t)cnt * ps);
            off += (size_t)cnt * ps;
        }
    }
}

/* One compressed deep block (offset table + sample data) plus its result, so a
 * worker can produce it while the driver writes earlier blocks in order. */
typedef struct {
    uint8_t *poff, *psamp;
    size_t poff_sz, psamp_sz;
    uint64_t usamp;
    exr_result rc;
} deep_block_out;

static void deep_blocks_free(const exr_allocator *a, deep_block_out *o,
                             uint32_t n) {
    uint32_t i;
    for (i = 0; i < n; ++i) { exr_free(a, o[i].poff); exr_free(a, o[i].psamp); }
    exr_free(a, o);
}

/* Parallel tile encode for one deep level. */
typedef struct {
    const exr_allocator *a;
    exr_compression comp;
    const exr_part *lvl;
    const uint64_t *lpfx;
    int tx, ty, nxt;
    deep_block_out *out;
} deep_tile_enc_ctx;

static void deep_tile_enc_one(deep_tile_enc_ctx *c, uint32_t idx) {
    int txi = (int)(idx % (uint32_t)c->nxt), tyi = (int)(idx / (uint32_t)c->nxt);
    int x0 = txi * c->tx, y0 = tyi * c->ty;
    int tw = (c->tx < c->lvl->width - x0) ? c->tx : (c->lvl->width - x0);
    int th = (c->ty < c->lvl->height - y0) ? c->ty : (c->lvl->height - y0);
    deep_block_out *o = &c->out[idx];
    uint64_t uoff = 0;
    o->rc = exr_deep_encode_tile(c->a, c->comp, c->lvl, c->lpfx, x0, y0, tw, th,
                                 &o->poff, &o->poff_sz, &uoff, &o->psamp,
                                 &o->psamp_sz, &o->usamp);
}

static void deep_tile_enc_job(void *vc, int job) {
    deep_tile_enc_ctx *c = (deep_tile_enc_ctx *)vc;
    deep_tile_enc_one(c, (uint32_t)job + 1u); /* tile 0 done serially first */
}

/* Emit every tile of one deep level (lx,ly) to the output buffer. Tiles are
 * compressed in parallel into a scratch array, then written in row-major order
 * (the stream layout is byte-deterministic). */
static exr_result emit_deep_tile_level(obuf *b, const exr_allocator *a,
                                       exr_compression comp, const exr_part *lvl,
                                       const uint64_t *lpfx, int p, int multipart,
                                       int lx, int ly, uint64_t *offtab,
                                       uint32_t *ci) {
    int tx = (int)lvl->header.tile_x_size, ty = (int)lvl->header.tile_y_size;
    int nxt = (lvl->width + tx - 1) / tx, nyt = (lvl->height + ty - 1) / ty;
    uint32_t ntiles = (uint32_t)nxt * (uint32_t)nyt, idx;
    deep_block_out *out;
    deep_tile_enc_ctx ec;
    exr_result rc = EXR_SUCCESS;
    if (ntiles == 0) return EXR_SUCCESS;
    out = (deep_block_out *)exr_calloc(a, ntiles, sizeof(*out));
    if (!out) return EXR_ERROR_OUT_OF_MEMORY;
    ec.a = a; ec.comp = comp; ec.lvl = lvl; ec.lpfx = lpfx;
    ec.tx = tx; ec.ty = ty; ec.nxt = nxt; ec.out = out;
    exr_simd_init();
    deep_tile_enc_one(&ec, 0); /* warm codec lazy init */
    if (EXR_OK(out[0].rc) && ntiles > 1)
        exr_parallel_for(exr_get_num_threads(), (int)(ntiles - 1u),
                         deep_tile_enc_job, &ec);
    for (idx = 0; idx < ntiles; ++idx) {
        deep_block_out *o = &out[idx];
        int txi = (int)(idx % (uint32_t)nxt), tyi = (int)(idx / (uint32_t)nxt);
        if (!EXR_OK(o->rc)) { rc = o->rc; break; }
        offtab[(*ci)++] = (uint64_t)b->len;
        if (multipart) ob_i32(b, p);
        ob_i32(b, txi);
        ob_i32(b, tyi);
        ob_i32(b, lx);
        ob_i32(b, ly);
        ob_u64(b, (uint64_t)o->poff_sz);
        ob_u64(b, (uint64_t)o->psamp_sz);
        ob_u64(b, o->usamp);
        ob_bytes(b, o->poff, o->poff_sz);
        ob_bytes(b, o->psamp, o->psamp_sz);
        if (b->err) { rc = EXR_ERROR_OUT_OF_MEMORY; break; }
    }
    deep_blocks_free(a, out, ntiles);
    return rc;
}

/* Parallel scanline-block encode for a deep part. */
typedef struct {
    const exr_allocator *a;
    exr_compression comp;
    const exr_part *pt;
    const uint64_t *prefix;
    int ymin, ymax, lpb;
    deep_block_out *out;
} deep_sl_enc_ctx;

static void deep_sl_enc_one(deep_sl_enc_ctx *c, uint32_t ci) {
    int y0 = c->ymin + (int)ci * c->lpb;
    int nlines = (y0 + c->lpb - 1 > c->ymax) ? (c->ymax - y0 + 1) : c->lpb;
    deep_block_out *o = &c->out[ci];
    uint64_t uoff = 0;
    o->rc = exr_deep_encode_block(c->a, c->comp, c->pt, c->prefix, y0, nlines,
                                  &o->poff, &o->poff_sz, &uoff, &o->psamp,
                                  &o->psamp_sz, &o->usamp);
}

static void deep_sl_enc_job(void *vc, int job) {
    deep_sl_enc_ctx *c = (deep_sl_enc_ctx *)vc;
    deep_sl_enc_one(c, (uint32_t)job + 1u); /* block 0 done serially first */
}

/* Compress every deep scanline block in parallel, then append them in order. */
static exr_result emit_deep_scanlines(obuf *b, const exr_allocator *a,
                                      exr_compression comp, const exr_part *pt,
                                      const uint64_t *prefix, int ymin, int ymax,
                                      int lpb, uint32_t nchunks, uint64_t *offtab,
                                      int p, int multipart) {
    deep_block_out *out;
    deep_sl_enc_ctx ec;
    uint32_t ci;
    exr_result rc = EXR_SUCCESS;
    if (nchunks == 0) return EXR_SUCCESS;
    out = (deep_block_out *)exr_calloc(a, nchunks, sizeof(*out));
    if (!out) return EXR_ERROR_OUT_OF_MEMORY;
    ec.a = a; ec.comp = comp; ec.pt = pt; ec.prefix = prefix;
    ec.ymin = ymin; ec.ymax = ymax; ec.lpb = lpb; ec.out = out;
    exr_simd_init();
    deep_sl_enc_one(&ec, 0); /* warm codec lazy init */
    if (EXR_OK(out[0].rc) && nchunks > 1)
        exr_parallel_for(exr_get_num_threads(), (int)(nchunks - 1u),
                         deep_sl_enc_job, &ec);
    for (ci = 0; ci < nchunks; ++ci) {
        deep_block_out *o = &out[ci];
        int y0 = ymin + (int)ci * lpb;
        if (!EXR_OK(o->rc)) { rc = o->rc; break; }
        offtab[ci] = (uint64_t)b->len;
        if (multipart) ob_i32(b, p);
        ob_i32(b, y0);
        ob_u64(b, (uint64_t)o->poff_sz);
        ob_u64(b, (uint64_t)o->psamp_sz);
        ob_u64(b, o->usamp);
        ob_bytes(b, o->poff, o->poff_sz);
        ob_bytes(b, o->psamp, o->psamp_sz);
        if (b->err) { rc = EXR_ERROR_OUT_OF_MEMORY; break; }
    }
    deep_blocks_free(a, out, nchunks);
    return rc;
}

/* Prefix-sum a deep part's sample counts into a freshly allocated array. */
static uint64_t *deep_prefix(const exr_allocator *a, const exr_part *pt) {
    size_t npix = (size_t)pt->width * pt->height, i;
    uint64_t acc = 0, *pfx = (uint64_t *)exr_malloc(a, (npix ? npix : 1) * sizeof(uint64_t));
    if (!pfx) return NULL;
    for (i = 0; i < npix; ++i) { pfx[i] = acc; acc += (uint64_t)pt->deep_sample_counts[i]; }
    return pfx;
}

/* ---- optional parallel block compression (phase 1 of two-phase encode) ---- */
#if defined(EXR_USE_THREADS)
/* Scanline: gather + compress chunk into payloads[ci]/sizes[ci]. */
typedef struct {
    const exr_allocator *a;
    const exr_header *h;
    const int *order;
    const exr_channel *sorted;
    void *const *images;
    int xmin, ymin, ymax, width, lpb;
    exr_compression comp;
    uint8_t **payloads;
    size_t *sizes;
    exr_result *rc;
    int job_base; /* chunk index of job 0 (1 = chunk 0 done inline; 0 = all parallel) */
} sl_enc_ctx;

static exr_result encode_scanline_one(sl_enc_ctx *c, uint32_t ci) {
    int y0 = c->ymin + (int)ci * c->lpb;
    int nlines = (y0 + c->lpb - 1 > c->ymax) ? (c->ymax - y0 + 1) : c->lpb;
    size_t blk_size;
    uint8_t *block;
    exr_codec_ctx cx;
    exr_result rc = exr_block_uncompressed_size(c->h->channels,
                                                c->h->num_channels, c->xmin, y0,
                                                c->width, nlines, &blk_size);
    if (!EXR_OK(rc)) return rc;
    block = (uint8_t *)exr_malloc(c->a, blk_size ? blk_size : 1);
    if (!block) return EXR_ERROR_OUT_OF_MEMORY;
    gather_scanline_block(c->h, c->order, c->images, y0, nlines, block);
    cx.alloc = c->a;
    cx.compression = c->comp;
    cx.channels = c->sorted;
    cx.num_channels = c->h->num_channels;
    cx.x = c->xmin;
    cx.y = y0;
    cx.width = c->width;
    cx.num_lines = nlines;
    rc = exr_compress_block(&cx, block, blk_size, &c->payloads[ci],
                            &c->sizes[ci]);
    exr_free(c->a, block);
    return rc;
}

static void encode_scanline_job(void *vc, int job) {
    sl_enc_ctx *c = (sl_enc_ctx *)vc;
    uint32_t ci = (uint32_t)job + (uint32_t)c->job_base;
    c->rc[ci] = encode_scanline_one(c, ci);
}

/* Simple single-level tiled: gather + compress tile idx. */
typedef struct {
    const exr_allocator *a;
    const exr_header *h;
    const int *order;
    const exr_channel *sorted;
    void *const *images;
    int xmin, ymin, width, height, tx, ty, nxt;
    exr_compression comp;
    uint8_t **payloads;
    size_t *sizes;
    exr_result *rc;
    int job_base; /* tile index of job 0 (1 = tile 0 done inline; 0 = all parallel) */
} tl_enc_ctx;

static exr_result encode_tile_one(tl_enc_ctx *c, uint32_t idx) {
    int txi = (int)(idx % (uint32_t)c->nxt);
    int tyi = (int)(idx / (uint32_t)c->nxt);
    int x0 = txi * c->tx, y0 = tyi * c->ty;
    int tile_w = (c->tx < c->width - x0) ? c->tx : (c->width - x0);
    int tile_h = (c->ty < c->height - y0) ? c->ty : (c->height - y0);
    int abs_x0 = c->xmin + x0, abs_y0 = c->ymin + y0;
    size_t blk_size;
    uint8_t *block;
    exr_codec_ctx cx;
    exr_result rc = exr_block_uncompressed_size(c->h->channels,
                                                c->h->num_channels, abs_x0,
                                                abs_y0, tile_w, tile_h,
                                                &blk_size);
    if (!EXR_OK(rc)) return rc;
    block = (uint8_t *)exr_malloc(c->a, blk_size ? blk_size : 1);
    if (!block) return EXR_ERROR_OUT_OF_MEMORY;
    gather_tile_block(c->h, c->order, c->images, abs_x0, abs_y0, tile_w, tile_h,
                      block);
    cx.alloc = c->a;
    cx.compression = c->comp;
    cx.channels = c->sorted;
    cx.num_channels = c->h->num_channels;
    cx.x = abs_x0;
    cx.y = abs_y0;
    cx.width = tile_w;
    cx.num_lines = tile_h;
    rc = exr_compress_block(&cx, block, blk_size, &c->payloads[idx],
                            &c->sizes[idx]);
    exr_free(c->a, block);
    return rc;
}

static void encode_tile_job(void *vc, int job) {
    tl_enc_ctx *c = (tl_enc_ctx *)vc;
    uint32_t idx = (uint32_t)job + (uint32_t)c->job_base;
    c->rc[idx] = encode_tile_one(c, idx);
}

/* Codecs whose first-use lazy global init is not thread-safe must have it warmed
 * on one thread before the workers run; then all chunks/tiles can be compressed
 * in parallel (no serial chunk 0). Currently only HTJ2K (the VLC/UVLC encode
 * tables). Returns 1 if warmed (=> caller may parallelize all jobs). */
static int encode_warmup_for_parallel(exr_compression comp) {
    if (comp == EXR_COMPRESSION_HTJ2K256 || comp == EXR_COMPRESSION_HTJ2K32) {
        /* Warm every process-global lazy init the workers could touch, on this
         * one thread, so the parallel encode only reads them (no data race):
         * the SIMD-tier cache, the SIMD function-pointer table, and the HTJ2K
         * VLC/UVLC encode tables. */
        exr_cpu_caps();
        exr_simd_init();
        exr_jph_warmup_encode_tables();
        return 1;
    }
    return 0;
}

/* Two-phase scanline encode: compress all chunks in parallel into per-chunk
 * buffers, then write them to `b` in order (identical bytes to the serial path).
 * Sets *threaded=1 when the parallel path was taken (0 => caller runs serial). */
static exr_result encode_parallel_scanline(
    const exr_allocator *a, const exr_header *h, const int *order,
    const exr_channel *sorted, const exr_part *pt, int xmin, int ymin, int ymax,
    int lpb, exr_compression comp, obuf *b, uint64_t *offsets, uint32_t n,
    int multipart, int p, int *threaded) {
    uint8_t **payloads = (uint8_t **)exr_calloc(a, n, sizeof(uint8_t *));
    size_t *sizes = (size_t *)exr_calloc(a, n, sizeof(size_t));
    exr_result *rcs = (exr_result *)exr_calloc(a, n, sizeof(exr_result));
    exr_result rc = EXR_SUCCESS;
    uint32_t ci;
    if (!payloads || !sizes || !rcs) {
        exr_free(a, payloads);
        exr_free(a, sizes);
        exr_free(a, rcs);
        *threaded = 0;
        return EXR_SUCCESS; /* fall back to the serial path */
    }
    *threaded = 1;
    {
        sl_enc_ctx ec;
        ec.a = a; ec.h = h; ec.order = order; ec.sorted = sorted;
        ec.images = (void *const *)pt->images; ec.xmin = xmin; ec.ymin = ymin;
        ec.ymax = ymax; ec.width = pt->width; ec.lpb = lpb; ec.comp = comp;
        ec.payloads = payloads; ec.sizes = sizes; ec.rc = rcs;
        if (encode_warmup_for_parallel(comp)) {
            /* Lazy inits warmed on this thread: compress every chunk in parallel
             * (no serial chunk 0), so scaling is not capped by one serial chunk. */
            ec.job_base = 0;
            exr_parallel_for(exr_get_num_threads(), (int)n,
                             encode_scanline_job, &ec);
        } else {
            /* Compress chunk 0 inline first to warm any lazy init the codec does,
             * then run the rest in parallel. */
            ec.job_base = 1;
            rcs[0] = encode_scanline_one(&ec, 0);
            if (EXR_OK(rcs[0]))
                exr_parallel_for(exr_get_num_threads(), (int)(n - 1),
                                 encode_scanline_job, &ec);
        }
    }
    for (ci = 0; ci < n; ++ci)
        if (!EXR_OK(rcs[ci])) { rc = rcs[ci]; break; }
    if (EXR_OK(rc)) {
        for (ci = 0; ci < n; ++ci) {
            int y0 = ymin + (int)ci * lpb;
            offsets[ci] = (uint64_t)b->len;
            if (multipart) ob_i32(b, p);
            ob_i32(b, y0);
            ob_i32(b, (int32_t)sizes[ci]);
            ob_bytes(b, payloads[ci], sizes[ci]);
            if (b->err) { rc = EXR_ERROR_OUT_OF_MEMORY; break; }
        }
    }
    for (ci = 0; ci < n; ++ci) exr_free(a, payloads[ci]);
    exr_free(a, payloads);
    exr_free(a, sizes);
    exr_free(a, rcs);
    return rc;
}

/* Two-phase single-level tiled encode (mirror of the scanline driver). */
static exr_result encode_parallel_tiled(
    const exr_allocator *a, const exr_header *h, const int *order,
    const exr_channel *sorted, const exr_part *pt, int xmin, int ymin, int tx,
    int ty, int nxt, exr_compression comp, obuf *b, uint64_t *offsets,
    uint32_t n, int multipart, int p, int *threaded) {
    uint8_t **payloads = (uint8_t **)exr_calloc(a, n, sizeof(uint8_t *));
    size_t *sizes = (size_t *)exr_calloc(a, n, sizeof(size_t));
    exr_result *rcs = (exr_result *)exr_calloc(a, n, sizeof(exr_result));
    exr_result rc = EXR_SUCCESS;
    uint32_t idx;
    if (!payloads || !sizes || !rcs) {
        exr_free(a, payloads);
        exr_free(a, sizes);
        exr_free(a, rcs);
        *threaded = 0;
        return EXR_SUCCESS;
    }
    *threaded = 1;
    {
        tl_enc_ctx ec;
        ec.a = a; ec.h = h; ec.order = order; ec.sorted = sorted;
        ec.images = (void *const *)pt->images; ec.xmin = xmin; ec.ymin = ymin;
        ec.width = pt->width; ec.height = pt->height; ec.tx = tx; ec.ty = ty;
        ec.nxt = nxt; ec.comp = comp;
        ec.payloads = payloads; ec.sizes = sizes; ec.rc = rcs;
        if (encode_warmup_for_parallel(comp)) {
            ec.job_base = 0;
            exr_parallel_for(exr_get_num_threads(), (int)n,
                             encode_tile_job, &ec);
        } else {
            ec.job_base = 1;
            rcs[0] = encode_tile_one(&ec, 0); /* warm lazy inits */
            if (EXR_OK(rcs[0]))
                exr_parallel_for(exr_get_num_threads(), (int)(n - 1),
                                 encode_tile_job, &ec);
        }
    }
    for (idx = 0; idx < n; ++idx)
        if (!EXR_OK(rcs[idx])) { rc = rcs[idx]; break; }
    if (EXR_OK(rc)) {
        for (idx = 0; idx < n; ++idx) {
            int txi = (int)(idx % (uint32_t)nxt);
            int tyi = (int)(idx / (uint32_t)nxt);
            offsets[idx] = (uint64_t)b->len;
            if (multipart) ob_i32(b, p);
            ob_i32(b, txi);
            ob_i32(b, tyi);
            ob_i32(b, 0); /* level x */
            ob_i32(b, 0); /* level y */
            ob_i32(b, (int32_t)sizes[idx]);
            ob_bytes(b, payloads[idx], sizes[idx]);
            if (b->err) { rc = EXR_ERROR_OUT_OF_MEMORY; break; }
        }
    }
    for (idx = 0; idx < n; ++idx) exr_free(a, payloads[idx]);
    exr_free(a, payloads);
    exr_free(a, sizes);
    exr_free(a, rcs);
    return rc;
}
#endif /* EXR_USE_THREADS */

/* ---- serialize a set of parts -------------------------------------------- */

static exr_result serialize(const exr_allocator *a, const exr_part *parts,
                            int num_parts, int comp_override, uint8_t **out_data,
                            size_t *out_size) {
    obuf b;
    int p;
    int multipart = (num_parts > 1);
    int any_tiled = 0, any_deep = 0;
    uint64_t **offset_tables = NULL;
    size_t *offset_positions = NULL;
    uint32_t *chunk_counts = NULL;
    int **orders = NULL;
    exr_channel **sorted_chans = NULL;
    exr_result rc = EXR_SUCCESS;

    if (num_parts <= 0) return EXR_ERROR_INVALID_ARGUMENT;

    memset(&b, 0, sizeof(b));
    b.a = a;

    for (p = 0; p < num_parts; ++p) {
        if (parts[p].header.tiled) {
            any_tiled = 1;
            /* ONE_LEVEL + MIPMAP + RIPMAP supported (flat and deep). */
            if (parts[p].header.tile_x_size == 0 || parts[p].header.tile_y_size == 0)
                return EXR_ERROR_INVALID_ARGUMENT;
        }
        if (parts[p].is_deep) any_deep = 1; /* deep tiled allowed (ONE_LEVEL) */
    }

    orders = (int **)exr_calloc(a, (size_t)num_parts, sizeof(int *));
    sorted_chans = (exr_channel **)exr_calloc(a, (size_t)num_parts, sizeof(exr_channel *));
    chunk_counts = (uint32_t *)exr_calloc(a, (size_t)num_parts, sizeof(uint32_t));
    offset_tables = (uint64_t **)exr_calloc(a, (size_t)num_parts, sizeof(uint64_t *));
    offset_positions = (size_t *)exr_calloc(a, (size_t)num_parts, sizeof(size_t));
    if (!orders || !sorted_chans || !chunk_counts || !offset_tables ||
        !offset_positions) {
        rc = EXR_ERROR_OUT_OF_MEMORY;
        goto done;
    }
    for (p = 0; p < num_parts; ++p) {
        const exr_part *pt = &parts[p];
        int nch = pt->header.num_channels;
        int lpb, ci2;
        exr_compression comp =
            comp_override >= 0 ? (exr_compression)comp_override : pt->header.compression;
        orders[p] = (int *)exr_calloc(a, nch ? (size_t)nch : 1, sizeof(int));
        sorted_chans[p] =
            (exr_channel *)exr_calloc(a, nch ? (size_t)nch : 1, sizeof(exr_channel));
        if (!orders[p] || !sorted_chans[p]) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
        sort_channels(&pt->header, orders[p]);
        for (ci2 = 0; ci2 < nch; ++ci2)
            sorted_chans[p][ci2] = pt->header.channels[orders[p][ci2]];
        if (pt->header.tiled) {
            int tx = (int)pt->header.tile_x_size, ty = (int)pt->header.tile_y_size;
            if (pt->header.level_mode == EXR_TILE_MIPMAP_LEVELS) {
                int up = (pt->header.rounding_mode == EXR_TILE_ROUND_UP);
                int ww = pt->width, hh = pt->height;
                uint32_t total = 0;
                for (;;) {
                    uint32_t nx = (uint32_t)((ww + tx - 1) / tx);
                    uint32_t ny = (uint32_t)((hh + ty - 1) / ty);
                    total += nx * ny;
                    if (ww <= 1 && hh <= 1) break;
                    ww = up ? (ww + 1) / 2 : ww / 2; if (ww < 1) ww = 1;
                    hh = up ? (hh + 1) / 2 : hh / 2; if (hh < 1) hh = 1;
                }
                chunk_counts[p] = total;
            } else if (pt->header.level_mode == EXR_TILE_RIPMAP_LEVELS) {
                /* sum_{lx,ly} ceil(lw/tx)*ceil(lh/ty) = (sum_lx ..)*(sum_ly ..) */
                int up = (pt->header.rounding_mode == EXR_TILE_ROUND_UP);
                uint32_t sx = 0, sy = 0;
                int s;
                for (s = pt->width;;) {
                    sx += (uint32_t)((s + tx - 1) / tx);
                    if (s <= 1) break;
                    s = up ? (s + 1) / 2 : s / 2; if (s < 1) s = 1;
                }
                for (s = pt->height;;) {
                    sy += (uint32_t)((s + ty - 1) / ty);
                    if (s <= 1) break;
                    s = up ? (s + 1) / 2 : s / 2; if (s < 1) s = 1;
                }
                chunk_counts[p] = sx * sy;
            } else {
                uint32_t nx = (uint32_t)((pt->width + tx - 1) / tx);
                uint32_t ny = (uint32_t)((pt->height + ty - 1) / ty);
                chunk_counts[p] = nx * ny;
            }
        } else {
            lpb = exr_lines_per_block(comp);
            chunk_counts[p] = (uint32_t)(((int64_t)pt->height + lpb - 1) / lpb);
        }
        (void)lpb;
    }

    /* magic + version */
    ob_u32(&b, EXR_MAGIC);
    {
        uint32_t ver = EXR_VERSION_NUMBER;
        if (multipart) ver |= EXR_VERSION_FLAG_MULTIPART;
        else if (any_tiled && !any_deep) ver |= EXR_VERSION_FLAG_TILED;
        if (any_deep) ver |= EXR_VERSION_FLAG_NON_IMAGE;
        ob_u32(&b, ver);
    }

    /* headers */
    for (p = 0; p < num_parts; ++p) {
        const exr_part *pt = &parts[p];
        exr_compression comp =
            comp_override >= 0 ? (exr_compression)comp_override : pt->header.compression;
        int32_t max_samples = 0;
        if (pt->is_deep && pt->deep_sample_counts) {
            size_t npix = (size_t)pt->width * pt->height, i;
            for (i = 0; i < npix; ++i)
                if (pt->deep_sample_counts[i] > max_samples)
                    max_samples = pt->deep_sample_counts[i];
        }
        write_header(&b, &pt->header, orders[p], comp, multipart,
                     chunk_counts[p], max_samples);
    }
    if (multipart) ob_u8(&b, 0); /* end of header list */

    /* offset tables (reserve, backpatch later) */
    for (p = 0; p < num_parts; ++p) {
        uint32_t k;
        offset_positions[p] = b.len;
        offset_tables[p] =
            (uint64_t *)exr_calloc(a, chunk_counts[p] ? chunk_counts[p] : 1,
                                   sizeof(uint64_t));
        if (!offset_tables[p]) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
        for (k = 0; k < chunk_counts[p]; ++k) ob_u64(&b, 0);
    }
    if (b.err) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }

    /* chunks */
    for (p = 0; p < num_parts; ++p) {
        const exr_part *pt = &parts[p];
        const exr_header *h = &pt->header;
        exr_compression comp =
            comp_override >= 0 ? (exr_compression)comp_override : h->compression;
        int xmin = h->data_window.min_x, ymin = h->data_window.min_y;
        int ymax = h->data_window.max_y;

        if (pt->is_deep) {
            int lpb = exr_lines_per_block(comp);
            uint64_t *prefix = NULL;
            size_t npix = (size_t)pt->width * pt->height, i;
            uint64_t acc = 0;
            prefix = (uint64_t *)exr_malloc(a, (npix ? npix : 1) * sizeof(uint64_t));
            if (!prefix) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
            for (i = 0; i < npix; ++i) {
                prefix[i] = acc;
                acc += (uint64_t)pt->deep_sample_counts[i];
            }
            if (h->tiled) {
                int up = (h->rounding_mode == EXR_TILE_ROUND_UP);
                uint32_t ci = 0;
                if (h->level_mode == EXR_TILE_MIPMAP_LEVELS) {
                    int ww = pt->width, hh = pt->height, L = 0;
                    for (;;) {
                        if (L == 0) {
                            rc = emit_deep_tile_level(&b, a, comp, pt, prefix, p,
                                                      multipart, 0, 0,
                                                      offset_tables[p], &ci);
                        } else {
                            exr_part lvl;
                            uint64_t *lpfx;
                            rc = exr_deep_build_level(a, pt, prefix, ww, hh, &lvl);
                            if (EXR_OK(rc)) {
                                lpfx = deep_prefix(a, &lvl);
                                if (!lpfx) rc = EXR_ERROR_OUT_OF_MEMORY;
                                else {
                                    rc = emit_deep_tile_level(&b, a, comp, &lvl,
                                                              lpfx, p, multipart, L,
                                                              L, offset_tables[p],
                                                              &ci);
                                    exr_free(a, lpfx);
                                }
                                exr_deep_level_free(a, &lvl);
                            }
                        }
                        if (!EXR_OK(rc) || (ww <= 1 && hh <= 1)) break;
                        ww = up ? (ww + 1) / 2 : ww / 2; if (ww < 1) ww = 1;
                        hh = up ? (hh + 1) / 2 : hh / 2; if (hh < 1) hh = 1;
                        L++;
                    }
                } else if (h->level_mode == EXR_TILE_RIPMAP_LEVELS) {
                    int nxl = 0, nyl = 0, s, lx, ly;
                    for (s = pt->width;;) { nxl++; if (s <= 1) break; s = up ? (s + 1) / 2 : s / 2; if (s < 1) s = 1; }
                    for (s = pt->height;;) { nyl++; if (s <= 1) break; s = up ? (s + 1) / 2 : s / 2; if (s < 1) s = 1; }
                    for (ly = 0; ly < nyl && EXR_OK(rc); ++ly) {
                        for (lx = 0; lx < nxl && EXR_OK(rc); ++lx) {
                            int lw = pt->width, lh = pt->height, t;
                            for (t = 0; t < lx; ++t) { lw = up ? (lw + 1) / 2 : lw / 2; if (lw < 1) lw = 1; }
                            for (t = 0; t < ly; ++t) { lh = up ? (lh + 1) / 2 : lh / 2; if (lh < 1) lh = 1; }
                            if (lx == 0 && ly == 0) {
                                rc = emit_deep_tile_level(&b, a, comp, pt, prefix, p,
                                                          multipart, 0, 0,
                                                          offset_tables[p], &ci);
                            } else {
                                exr_part lvl;
                                uint64_t *lpfx;
                                rc = exr_deep_build_level(a, pt, prefix, lw, lh, &lvl);
                                if (EXR_OK(rc)) {
                                    lpfx = deep_prefix(a, &lvl);
                                    if (!lpfx) rc = EXR_ERROR_OUT_OF_MEMORY;
                                    else {
                                        rc = emit_deep_tile_level(&b, a, comp, &lvl,
                                                                  lpfx, p, multipart,
                                                                  lx, ly,
                                                                  offset_tables[p],
                                                                  &ci);
                                        exr_free(a, lpfx);
                                    }
                                    exr_deep_level_free(a, &lvl);
                                }
                            }
                        }
                    }
                } else {
                    rc = emit_deep_tile_level(&b, a, comp, pt, prefix, p, multipart,
                                              0, 0, offset_tables[p], &ci);
                }
                exr_free(a, prefix);
                if (!EXR_OK(rc)) goto done;
                continue;
            }
            rc = emit_deep_scanlines(&b, a, comp, pt, prefix, ymin, ymax, lpb,
                                     chunk_counts[p], offset_tables[p], p,
                                     multipart);
            exr_free(a, prefix);
            if (!EXR_OK(rc)) goto done;
        } else if (h->tiled && h->level_mode == EXR_TILE_MIPMAP_LEVELS) {
            int tx = (int)h->tile_x_size, ty = (int)h->tile_y_size;
            int L, txi, tyi;
            uint32_t ci = 0;
            exr_mip_pyramid pyr;
            rc = exr_mip_generate(a, pt, h->rounding_mode == EXR_TILE_ROUND_UP,
                                  &pyr);
            if (!EXR_OK(rc)) goto done;
            for (L = 0; L < pyr.num_levels; ++L) {
                int lw = pyr.lw[L], lh = pyr.lh[L];
                int nxt = (lw + tx - 1) / tx, nyt = (lh + ty - 1) / ty;
                for (tyi = 0; tyi < nyt && EXR_OK(rc); ++tyi) {
                    for (txi = 0; txi < nxt; ++txi) {
                        int x0 = txi * tx, y0 = tyi * ty;
                        int tw = (tx < lw - x0) ? tx : (lw - x0);
                        int th = (ty < lh - y0) ? ty : (lh - y0);
                        size_t blk_size, payload_size = 0;
                        uint8_t *block, *payload = NULL;
                        exr_codec_ctx cx;
                        rc = exr_block_uncompressed_size(sorted_chans[p],
                                                         h->num_channels, x0, y0,
                                                         tw, th, &blk_size);
                        if (!EXR_OK(rc)) break;
                        block = (uint8_t *)exr_malloc(a, blk_size ? blk_size : 1);
                        if (!block) { rc = EXR_ERROR_OUT_OF_MEMORY; break; }
                        gather_level_tile(h, orders[p], pyr.img[L], lw, x0, y0, tw,
                                          th, block);
                        cx.alloc = a;
                        cx.compression = comp;
                        cx.channels = sorted_chans[p];
                        cx.num_channels = h->num_channels;
                        cx.x = x0;
                        cx.y = y0;
                        cx.width = tw;
                        cx.num_lines = th;
                        rc = exr_compress_block(&cx, block, blk_size, &payload,
                                                &payload_size);
                        exr_free(a, block);
                        if (!EXR_OK(rc)) break;
                        offset_tables[p][ci++] = (uint64_t)b.len;
                        if (multipart) ob_i32(&b, p);
                        ob_i32(&b, txi);
                        ob_i32(&b, tyi);
                        ob_i32(&b, L);
                        ob_i32(&b, L);
                        ob_i32(&b, (int32_t)payload_size);
                        ob_bytes(&b, payload, payload_size);
                        exr_free(a, payload);
                        if (b.err) { rc = EXR_ERROR_OUT_OF_MEMORY; break; }
                    }
                }
                if (!EXR_OK(rc)) break;
            }
            exr_mip_free(&pyr);
            if (!EXR_OK(rc)) goto done;
            continue;
        } else if (h->tiled && h->level_mode == EXR_TILE_RIPMAP_LEVELS) {
            int tx = (int)h->tile_x_size, ty = (int)h->tile_y_size;
            int lx, ly, txi, tyi;
            uint32_t ci = 0;
            exr_ripmap_pyramid pyr;
            rc = exr_ripmap_generate(a, pt, h->rounding_mode == EXR_TILE_ROUND_UP,
                                     &pyr);
            if (!EXR_OK(rc)) goto done;
            /* offset-table order: ly outer, lx inner (matches the reader). */
            for (ly = 0; ly < pyr.num_y_levels && EXR_OK(rc); ++ly) {
                for (lx = 0; lx < pyr.num_x_levels && EXR_OK(rc); ++lx) {
                    int lw = pyr.lw[lx], lh = pyr.lh[ly];
                    void **limg = pyr.img[lx * pyr.num_y_levels + ly];
                    int nxt = (lw + tx - 1) / tx, nyt = (lh + ty - 1) / ty;
                    for (tyi = 0; tyi < nyt && EXR_OK(rc); ++tyi) {
                        for (txi = 0; txi < nxt; ++txi) {
                            int x0 = txi * tx, y0 = tyi * ty;
                            int tw = (tx < lw - x0) ? tx : (lw - x0);
                            int th = (ty < lh - y0) ? ty : (lh - y0);
                            size_t blk_size, payload_size = 0;
                            uint8_t *block, *payload = NULL;
                            exr_codec_ctx cx;
                            rc = exr_block_uncompressed_size(sorted_chans[p],
                                                             h->num_channels, x0,
                                                             y0, tw, th, &blk_size);
                            if (!EXR_OK(rc)) break;
                            block = (uint8_t *)exr_malloc(a, blk_size ? blk_size : 1);
                            if (!block) { rc = EXR_ERROR_OUT_OF_MEMORY; break; }
                            gather_level_tile(h, orders[p], limg, lw, x0, y0, tw,
                                              th, block);
                            cx.alloc = a;
                            cx.compression = comp;
                            cx.channels = sorted_chans[p];
                            cx.num_channels = h->num_channels;
                            cx.x = x0;
                            cx.y = y0;
                            cx.width = tw;
                            cx.num_lines = th;
                            rc = exr_compress_block(&cx, block, blk_size, &payload,
                                                    &payload_size);
                            exr_free(a, block);
                            if (!EXR_OK(rc)) break;
                            offset_tables[p][ci++] = (uint64_t)b.len;
                            if (multipart) ob_i32(&b, p);
                            ob_i32(&b, txi);
                            ob_i32(&b, tyi);
                            ob_i32(&b, lx);
                            ob_i32(&b, ly);
                            ob_i32(&b, (int32_t)payload_size);
                            ob_bytes(&b, payload, payload_size);
                            exr_free(a, payload);
                            if (b.err) { rc = EXR_ERROR_OUT_OF_MEMORY; break; }
                        }
                    }
                }
            }
            exr_ripmap_free(&pyr);
            if (!EXR_OK(rc)) goto done;
            continue;
        } else if (h->tiled) {
            int tx = (int)h->tile_x_size, ty = (int)h->tile_y_size;
            int nxt = (pt->width + tx - 1) / tx;
            int nyt = (pt->height + ty - 1) / ty;
            int txi, tyi;
            int threaded = 0;
#if defined(EXR_USE_THREADS)
            if (exr_get_num_threads() > 1 && chunk_counts[p] > 1 &&
                chunk_counts[p] == (uint32_t)nxt * (uint32_t)nyt) {
                rc = encode_parallel_tiled(a, h, orders[p], sorted_chans[p], pt,
                                           xmin, ymin, tx, ty, nxt, comp, &b,
                                           offset_tables[p], chunk_counts[p],
                                           multipart, p, &threaded);
                if (threaded && !EXR_OK(rc)) goto done;
            }
#endif
            if (!threaded)
            for (tyi = 0; tyi < nyt; ++tyi) {
                for (txi = 0; txi < nxt; ++txi) {
                    uint32_t ci = (uint32_t)tyi * (uint32_t)nxt + (uint32_t)txi;
                    int x0 = txi * tx, y0 = tyi * ty;
                    int tile_w = (tx < pt->width - x0) ? tx : (pt->width - x0);
                    int tile_h = (ty < pt->height - y0) ? ty : (pt->height - y0);
                    int abs_x0 = xmin + x0, abs_y0 = ymin + y0;
                    size_t blk_size, payload_size = 0;
                    uint8_t *block, *payload = NULL;

                    rc = exr_block_uncompressed_size(h->channels, h->num_channels,
                                                     abs_x0, abs_y0, tile_w,
                                                     tile_h, &blk_size);
                    if (!EXR_OK(rc)) goto done;
                    block = (uint8_t *)exr_malloc(a, blk_size ? blk_size : 1);
                    if (!block) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
                    gather_tile_block(h, orders[p], (void *const *)pt->images,
                                      abs_x0, abs_y0, tile_w, tile_h, block);
                    {
                        exr_codec_ctx cx;
                        cx.alloc = a;
                        cx.compression = comp;
                        cx.channels = sorted_chans[p];
                        cx.num_channels = h->num_channels;
                        cx.x = abs_x0;
                        cx.y = abs_y0;
                        cx.width = tile_w;
                        cx.num_lines = tile_h;
                        rc = exr_compress_block(&cx, block, blk_size, &payload,
                                                &payload_size);
                    }
                    exr_free(a, block);
                    if (!EXR_OK(rc)) goto done;

                    offset_tables[p][ci] = (uint64_t)b.len;
                    if (multipart) ob_i32(&b, p);
                    ob_i32(&b, txi);
                    ob_i32(&b, tyi);
                    ob_i32(&b, 0); /* level x */
                    ob_i32(&b, 0); /* level y */
                    ob_i32(&b, (int32_t)payload_size);
                    ob_bytes(&b, payload, payload_size);
                    exr_free(a, payload);
                    if (b.err) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
                }
            }
        } else {
            int lpb = exr_lines_per_block(comp);
            uint32_t ci;
            int threaded = 0;
#if defined(EXR_USE_THREADS)
            if (exr_get_num_threads() > 1 && chunk_counts[p] > 1) {
                rc = encode_parallel_scanline(a, h, orders[p], sorted_chans[p],
                                              pt, xmin, ymin, ymax, lpb, comp,
                                              &b, offset_tables[p],
                                              chunk_counts[p], multipart, p,
                                              &threaded);
                if (threaded && !EXR_OK(rc)) goto done;
            }
#endif
            if (!threaded)
            for (ci = 0; ci < chunk_counts[p]; ++ci) {
                int y0 = ymin + (int)ci * lpb;
                int nlines = (y0 + lpb - 1 > ymax) ? (ymax - y0 + 1) : lpb;
                size_t blk_size, payload_size = 0;
                uint8_t *block, *payload = NULL;

                rc = exr_block_uncompressed_size(h->channels, h->num_channels,
                                                 xmin, y0, pt->width, nlines,
                                                 &blk_size);
                if (!EXR_OK(rc)) goto done;
                block = (uint8_t *)exr_malloc(a, blk_size ? blk_size : 1);
                if (!block) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
                gather_scanline_block(h, orders[p], (void *const *)pt->images, y0,
                                      nlines, block);
                {
                    exr_codec_ctx cx;
                    cx.alloc = a;
                    cx.compression = comp;
                    cx.channels = sorted_chans[p];
                    cx.num_channels = h->num_channels;
                    cx.x = xmin;
                    cx.y = y0;
                    cx.width = pt->width;
                    cx.num_lines = nlines;
                    rc = exr_compress_block(&cx, block, blk_size, &payload,
                                            &payload_size);
                }
                exr_free(a, block);
                if (!EXR_OK(rc)) goto done;

                offset_tables[p][ci] = (uint64_t)b.len;
                if (multipart) ob_i32(&b, p);
                ob_i32(&b, y0);
                ob_i32(&b, (int32_t)payload_size);
                ob_bytes(&b, payload, payload_size);
                exr_free(a, payload);
                if (b.err) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
            }
        }
    }

    /* backpatch offset tables */
    for (p = 0; p < num_parts; ++p) {
        uint32_t k;
        for (k = 0; k < chunk_counts[p]; ++k)
            exr_wr_u64(b.data + offset_positions[p] + (size_t)k * 8,
                       offset_tables[p][k]);
    }

    *out_data = b.data;
    *out_size = b.len;
    b.data = NULL; /* ownership transferred */

done:
    if (orders) {
        for (p = 0; p < num_parts; ++p) exr_free(a, orders[p]);
        exr_free(a, orders);
    }
    if (sorted_chans) {
        for (p = 0; p < num_parts; ++p) exr_free(a, sorted_chans[p]);
        exr_free(a, sorted_chans);
    }
    if (offset_tables) {
        for (p = 0; p < num_parts; ++p) exr_free(a, offset_tables[p]);
        exr_free(a, offset_tables);
    }
    exr_free(a, offset_positions);
    exr_free(a, chunk_counts);
    exr_free(a, b.data);
    return rc;
}

/* ---- high-level save ----------------------------------------------------- */

exr_result exr_save_to_memory(void **out_data, size_t *out_size,
                              const exr_allocator *alloc, const exr_image *img,
                              exr_compression compression) {
    if (!out_data || !out_size || !img || img->num_parts <= 0)
        return EXR_ERROR_INVALID_ARGUMENT;
    if (!alloc) alloc = exr_default_allocator();
    *out_data = NULL;
    *out_size = 0;
    return serialize(alloc, img->parts, img->num_parts, (int)compression,
                     (uint8_t **)out_data, out_size);
}

/* exr_save_to_file lives in src/exr_stdio.c (the only stdio translation unit). */

/* ---- mid-level writer ---------------------------------------------------- */

struct exr_writer {
    exr_allocator alloc;
    exr_part *parts;
    int num_parts, cap;

    /* streaming-encode state (active between begin_stream and end_stream) */
    int streaming;
    exr_data_sink sink;
    int sink_open;        /* 1 once the writer owns the sink (call close once) */
    uint64_t pos;         /* current write offset */
    exr_compression scomp;
    int smultipart;
    int **sorder;          /* [part][nch] name-sorted channel order */
    exr_channel **ssorted; /* [part][nch] sorted channel descriptors */
    uint32_t *schunk;      /* [part] chunk count */
    uint64_t **soff;       /* [part][chunk] recorded chunk offsets */
    uint64_t *soff_pos;    /* [part] file offset of the reserved offset table */
    uint64_t *smax_pos;    /* [part] file offset of maxSamplesPerPixel value (deep) */
    int32_t *smax;         /* [part] running max sample count (deep) */
    /* Optional GPU HTJ2K block-encode hook (set by the GPU backend); used by
     * exr_writer_write_scanline_block_canon for HTJ2K parts. */
    exr_jph_gpu_block_encode_fn gpu_jph_fn;
    void *gpu_jph_user;
};

void exr_writer_set_gpu_jph_encoder(exr_writer *w,
                                    exr_jph_gpu_block_encode_fn fn, void *user) {
    if (!w) return;
    w->gpu_jph_fn = fn;
    w->gpu_jph_user = user;
}

/* Internal: the allocator a writer was created with (used by exr_stdio.c to free
 * buffers returned by exr_writer_finalize_to_memory). */
const exr_allocator *exr_writer_allocator(const exr_writer *w) {
    return &w->alloc;
}

exr_result exr_writer_create(const exr_allocator *alloc, exr_writer **out) {
    exr_writer *w;
    if (!out) return EXR_ERROR_INVALID_ARGUMENT;
    if (!alloc) alloc = exr_default_allocator();
    w = (exr_writer *)exr_calloc(alloc, 1, sizeof(*w));
    if (!w) return EXR_ERROR_OUT_OF_MEMORY;
    w->alloc = *alloc;
    *out = w;
    return EXR_SUCCESS;
}

static void stream_free_state(exr_writer *w);

void exr_writer_destroy(exr_writer *w) {
    int p;
    if (!w) return;
    if (w->streaming || w->sorder) stream_free_state(w);
    for (p = 0; p < w->num_parts; ++p) {
        exr_free(&w->alloc, w->parts[p].images); /* pixels are caller-owned */
        exr_header_free(&w->alloc, &w->parts[p].header);
    }
    exr_free(&w->alloc, w->parts);
    exr_free(&w->alloc, w);
}

exr_result exr_writer_add_part(exr_writer *w, const exr_header *hdr,
                               int32_t *out_part) {
    exr_part *pt;
    exr_result rc;
    if (!w || !hdr) return EXR_ERROR_INVALID_ARGUMENT;
    if (w->num_parts == w->cap) {
        int ncap = w->cap ? w->cap * 2 : 2;
        exr_part *np = (exr_part *)exr_calloc(&w->alloc, (size_t)ncap, sizeof(exr_part));
        if (!np) return EXR_ERROR_OUT_OF_MEMORY;
        if (w->parts) {
            memcpy(np, w->parts, (size_t)w->num_parts * sizeof(exr_part));
            exr_free(&w->alloc, w->parts);
        }
        w->parts = np;
        w->cap = ncap;
    }
    pt = &w->parts[w->num_parts];
    memset(pt, 0, sizeof(*pt));
    rc = exr_header_copy(&w->alloc, &pt->header, hdr);
    if (!EXR_OK(rc)) return rc;
    pt->width = hdr->data_window.max_x - hdr->data_window.min_x + 1;
    pt->height = hdr->data_window.max_y - hdr->data_window.min_y + 1;
    pt->images = (void **)exr_calloc(&w->alloc,
                                     hdr->num_channels ? (size_t)hdr->num_channels : 1,
                                     sizeof(void *));
    if (!pt->images) {
        exr_header_free(&w->alloc, &pt->header);
        return EXR_ERROR_OUT_OF_MEMORY;
    }
    if (out_part) *out_part = w->num_parts;
    w->num_parts++;
    return EXR_SUCCESS;
}

exr_result exr_writer_set_channel(exr_writer *w, int32_t part, const char *name,
                                  const void *pixels) {
    int c;
    exr_part *pt;
    if (!w || !name || part < 0 || part >= w->num_parts)
        return EXR_ERROR_INVALID_ARGUMENT;
    pt = &w->parts[part];
    for (c = 0; c < pt->header.num_channels; ++c)
        if (strcmp(pt->header.channels[c].name, name) == 0) {
            pt->images[c] = (void *)pixels;
            return EXR_SUCCESS;
        }
    return EXR_ERROR_INVALID_ARGUMENT;
}

exr_result exr_writer_finalize_to_memory(exr_writer *w, void **out_data,
                                         size_t *out_size) {
    if (!w || !out_data || !out_size) return EXR_ERROR_INVALID_ARGUMENT;
    *out_data = NULL;
    *out_size = 0;
    return serialize(&w->alloc, w->parts, w->num_parts, -1,
                     (uint8_t **)out_data, out_size);
}

/* exr_writer_finalize_to_file lives in src/exr_stdio.c (the only stdio TU). */

/* ---- streaming (block-at-a-time) writer ---------------------------------- */

/* Deepness is carried by the header's part_type (the writer part is built from
 * a header alone, so exr_part::is_deep is not populated for streaming). */
static int header_is_deep(const exr_header *h) {
    return h->part_type == EXR_PART_DEEP_SCANLINE ||
           h->part_type == EXR_PART_DEEP_TILED;
}

/* Gather one block from caller-provided block-local planar channels into the
 * canonical block layout (per scanline, then per sorted channel). chan[c] holds
 * this block's samples for original channel c, row 0 = block top. */
static void gather_block_local(const exr_header *h, const int *order,
                               const void *const *chan, int x0, int y0, int w,
                               int hgt, uint8_t *block) {
    size_t off = 0;
    int line, oi;
    for (line = 0; line < hgt; ++line) {
        int yy = y0 + line;
        for (oi = 0; oi < h->num_channels; ++oi) {
            int c = order[oi];
            int xs = h->channels[c].x_sampling, ys = h->channels[c].y_sampling;
            size_t ps = exr_pixel_size(h->channels[c].pixel_type);
            int nx, row;
            if (xs < 1) xs = 1;
            if (ys < 1) ys = 1;
            if ((yy % ys) != 0) continue;
            nx = exr_num_samples(x0, x0 + w - 1, xs);
            if (nx <= 0) continue;
            row = exr_num_samples(y0, yy, ys) - 1;
            memcpy(block + off,
                   (const uint8_t *)chan[c] + (size_t)row * (size_t)nx * ps,
                   (size_t)nx * ps);
            off += (size_t)nx * ps;
        }
    }
}

static int w_num_levels_axis(int s, int up) {
    int n = 1;
    while (s > 1) { s = up ? (s + 1) / 2 : s / 2; if (s < 1) s = 1; n++; }
    return n;
}

/* Pixel dims and tile counts of level (lx,ly), mirroring tiled_level_size(). */
static void w_level_dims(const exr_header *h, int W, int H, int lx, int ly,
                         int *lw, int *lh, int *nxt, int *nyt) {
    int up = (h->rounding_mode == EXR_TILE_ROUND_UP);
    int tx = (int)h->tile_x_size, ty = (int)h->tile_y_size, i;
    int w = W, hh = H;
    if (h->level_mode == EXR_TILE_MIPMAP_LEVELS) {
        for (i = 0; i < lx; ++i) {
            w = up ? (w + 1) / 2 : w / 2;
            hh = up ? (hh + 1) / 2 : hh / 2;
            if (w < 1) w = 1;
            if (hh < 1) hh = 1;
        }
    } else if (h->level_mode == EXR_TILE_RIPMAP_LEVELS) {
        for (i = 0; i < lx; ++i) { w = up ? (w + 1) / 2 : w / 2; if (w < 1) w = 1; }
        for (i = 0; i < ly; ++i) { hh = up ? (hh + 1) / 2 : hh / 2; if (hh < 1) hh = 1; }
    }
    if (lw) *lw = w;
    if (lh) *lh = hh;
    if (tx > 0 && nxt) *nxt = (w + tx - 1) / tx;
    if (ty > 0 && nyt) *nyt = (hh + ty - 1) / ty;
}

/* Offset-table index of tile (tx,ty) at level (lx,ly); mirrors tile_index(). */
static exr_result w_tile_index(const exr_header *h, int W, int H, int tx, int ty,
                               int lx, int ly, uint32_t *out) {
    int up = (h->rounding_mode == EXR_TILE_ROUND_UP);
    uint32_t index = 0;
    int nxt, nyt;
    if (h->level_mode == EXR_TILE_MIPMAP_LEVELS) {
        int l, nlev = w_num_levels_axis(W > H ? W : H, up);
        if (lx != ly || lx < 0 || lx >= nlev) return EXR_ERROR_INVALID_ARGUMENT;
        for (l = 0; l < lx; ++l) {
            w_level_dims(h, W, H, l, l, NULL, NULL, &nxt, &nyt);
            index += (uint32_t)(nxt * nyt);
        }
        w_level_dims(h, W, H, lx, ly, NULL, NULL, &nxt, NULL);
        *out = index + (uint32_t)(ty * nxt + tx);
        return EXR_SUCCESS;
    }
    if (h->level_mode == EXR_TILE_RIPMAP_LEVELS) {
        int nxl = w_num_levels_axis(W, up), nyl = w_num_levels_axis(H, up), xx, yy;
        if (lx < 0 || lx >= nxl || ly < 0 || ly >= nyl)
            return EXR_ERROR_INVALID_ARGUMENT;
        for (yy = 0; yy < ly; ++yy)
            for (xx = 0; xx < nxl; ++xx) {
                w_level_dims(h, W, H, xx, yy, NULL, NULL, &nxt, &nyt);
                index += (uint32_t)(nxt * nyt);
            }
        for (xx = 0; xx < lx; ++xx) {
            w_level_dims(h, W, H, xx, ly, NULL, NULL, &nxt, &nyt);
            index += (uint32_t)(nxt * nyt);
        }
        w_level_dims(h, W, H, lx, ly, NULL, NULL, &nxt, NULL);
        *out = index + (uint32_t)(ty * nxt + tx);
        return EXR_SUCCESS;
    }
    if (lx != 0 || ly != 0) return EXR_ERROR_INVALID_ARGUMENT;
    w_level_dims(h, W, H, 0, 0, NULL, NULL, &nxt, NULL);
    *out = (uint32_t)(ty * nxt + tx);
    return EXR_SUCCESS;
}

/* Chunk count of one part under compression `comp` (mirrors serialize()). */
static exr_result w_chunk_count(const exr_part *pt, exr_compression comp,
                                uint32_t *out) {
    const exr_header *h = &pt->header;
    if (h->tiled) {
        int tx = (int)h->tile_x_size, ty = (int)h->tile_y_size;
        int up = (h->rounding_mode == EXR_TILE_ROUND_UP);
        if (tx <= 0 || ty <= 0) return EXR_ERROR_INVALID_ARGUMENT;
        if (h->level_mode == EXR_TILE_MIPMAP_LEVELS) {
            int ww = pt->width, hh = pt->height;
            uint32_t total = 0;
            for (;;) {
                total += (uint32_t)((ww + tx - 1) / tx) *
                         (uint32_t)((hh + ty - 1) / ty);
                if (ww <= 1 && hh <= 1) break;
                ww = up ? (ww + 1) / 2 : ww / 2; if (ww < 1) ww = 1;
                hh = up ? (hh + 1) / 2 : hh / 2; if (hh < 1) hh = 1;
            }
            *out = total;
        } else if (h->level_mode == EXR_TILE_RIPMAP_LEVELS) {
            uint32_t sx = 0, sy = 0;
            int s;
            for (s = pt->width;;) {
                sx += (uint32_t)((s + tx - 1) / tx);
                if (s <= 1) break;
                s = up ? (s + 1) / 2 : s / 2; if (s < 1) s = 1;
            }
            for (s = pt->height;;) {
                sy += (uint32_t)((s + ty - 1) / ty);
                if (s <= 1) break;
                s = up ? (s + 1) / 2 : s / 2; if (s < 1) s = 1;
            }
            *out = sx * sy;
        } else {
            *out = (uint32_t)((pt->width + tx - 1) / tx) *
                   (uint32_t)((pt->height + ty - 1) / ty);
        }
    } else {
        int lpb = exr_lines_per_block(comp);
        *out = (uint32_t)(((int64_t)pt->height + lpb - 1) / lpb);
    }
    return EXR_SUCCESS;
}

static exr_result sink_write(exr_writer *w, const void *p, size_t n) {
    exr_result rc;
    if (n == 0) return EXR_SUCCESS;
    rc = w->sink.write(w->sink.user, p, n);
    if (EXR_OK(rc)) w->pos += n;
    return rc;
}

static void stream_free_state(exr_writer *w) {
    int p;
    if (w->sorder) {
        for (p = 0; p < w->num_parts; ++p) exr_free(&w->alloc, w->sorder[p]);
        exr_free(&w->alloc, w->sorder);
        w->sorder = NULL;
    }
    if (w->ssorted) {
        for (p = 0; p < w->num_parts; ++p) exr_free(&w->alloc, w->ssorted[p]);
        exr_free(&w->alloc, w->ssorted);
        w->ssorted = NULL;
    }
    if (w->soff) {
        for (p = 0; p < w->num_parts; ++p) exr_free(&w->alloc, w->soff[p]);
        exr_free(&w->alloc, w->soff);
        w->soff = NULL;
    }
    exr_free(&w->alloc, w->schunk); w->schunk = NULL;
    exr_free(&w->alloc, w->soff_pos); w->soff_pos = NULL;
    exr_free(&w->alloc, w->smax_pos); w->smax_pos = NULL;
    exr_free(&w->alloc, w->smax); w->smax = NULL;
    w->streaming = 0;
    /* Release the sink once (e.g. fclose for the file-backed sink). Ownership
     * is taken only when begin_stream fully succeeds (sink_open == 1). */
    if (w->sink_open && w->sink.close) w->sink.close(w->sink.user);
    w->sink_open = 0;
}

/* Find maxSamplesPerPixel's 4-byte value offset within a serialized header. */
static int find_max_samples_pos(const uint8_t *hdr, size_t len, size_t *rel) {
    /* Anchor on the full attribute prologue "maxSamplesPerPixel\0int\0" so a
     * stray substring inside some other attribute's value can't false-match. */
    static const char key[] = "maxSamplesPerPixel"; /* name (no NUL) */
    size_t klen = sizeof(key) - 1, need = klen + 1 + 4 + 4, i; /* +\0int\0 +size */
    if (len < need) return 0;
    for (i = 0; i + need <= len; ++i) {
        if (memcmp(hdr + i, key, klen) == 0 && hdr[i + klen] == 0 &&
            memcmp(hdr + i + klen + 1, "int", 3) == 0 && hdr[i + klen + 4] == 0) {
            *rel = i + klen + 1 + 4 + 4; /* name\0 + "int\0" + size(i32) -> value */
            return 1;
        }
    }
    return 0;
}

exr_result exr_writer_begin_stream(exr_writer *w, const exr_data_sink *sink,
                                   exr_compression comp) {
    int p, any_tiled = 0, any_deep = 0;
    exr_result rc = EXR_SUCCESS;
    obuf pre;

    if (!w || !sink || !sink->write || !sink->seek)
        return EXR_ERROR_INVALID_ARGUMENT;
    if (w->streaming || w->num_parts <= 0) return EXR_ERROR_INVALID_ARGUMENT;

    w->sink = *sink;
    w->pos = 0;
    w->scomp = comp;
    w->smultipart = (w->num_parts > 1);

    w->sorder = (int **)exr_calloc(&w->alloc, (size_t)w->num_parts, sizeof(int *));
    w->ssorted = (exr_channel **)exr_calloc(&w->alloc, (size_t)w->num_parts, sizeof(exr_channel *));
    w->schunk = (uint32_t *)exr_calloc(&w->alloc, (size_t)w->num_parts, sizeof(uint32_t));
    w->soff = (uint64_t **)exr_calloc(&w->alloc, (size_t)w->num_parts, sizeof(uint64_t *));
    w->soff_pos = (uint64_t *)exr_calloc(&w->alloc, (size_t)w->num_parts, sizeof(uint64_t));
    w->smax_pos = (uint64_t *)exr_calloc(&w->alloc, (size_t)w->num_parts, sizeof(uint64_t));
    w->smax = (int32_t *)exr_calloc(&w->alloc, (size_t)w->num_parts, sizeof(int32_t));
    if (!w->sorder || !w->ssorted || !w->schunk || !w->soff || !w->soff_pos ||
        !w->smax_pos || !w->smax) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }

    for (p = 0; p < w->num_parts; ++p) {
        const exr_part *pt = &w->parts[p];
        int nch = pt->header.num_channels, k;
        if (pt->header.tiled) {
            any_tiled = 1;
            if (pt->header.tile_x_size == 0 || pt->header.tile_y_size == 0) {
                rc = EXR_ERROR_INVALID_ARGUMENT; goto fail;
            }
        }
        if (header_is_deep(&pt->header)) any_deep = 1;
        w->sorder[p] = (int *)exr_calloc(&w->alloc, nch ? (size_t)nch : 1, sizeof(int));
        w->ssorted[p] = (exr_channel *)exr_calloc(&w->alloc, nch ? (size_t)nch : 1, sizeof(exr_channel));
        if (!w->sorder[p] || !w->ssorted[p]) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
        sort_channels(&pt->header, w->sorder[p]);
        for (k = 0; k < nch; ++k)
            w->ssorted[p][k] = pt->header.channels[w->sorder[p][k]];
        rc = w_chunk_count(pt, comp, &w->schunk[p]);
        if (!EXR_OK(rc)) goto fail;
        w->soff[p] = (uint64_t *)exr_calloc(&w->alloc, w->schunk[p] ? w->schunk[p] : 1, sizeof(uint64_t));
        if (!w->soff[p]) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
    }

    /* magic + version */
    memset(&pre, 0, sizeof(pre));
    pre.a = &w->alloc;
    ob_u32(&pre, EXR_MAGIC);
    {
        uint32_t ver = EXR_VERSION_NUMBER;
        if (w->smultipart) ver |= EXR_VERSION_FLAG_MULTIPART;
        else if (any_tiled && !any_deep) ver |= EXR_VERSION_FLAG_TILED;
        if (any_deep) ver |= EXR_VERSION_FLAG_NON_IMAGE;
        ob_u32(&pre, ver);
    }
    if (pre.err) { exr_free(&w->alloc, pre.data); rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
    rc = sink_write(w, pre.data, pre.len);
    exr_free(&w->alloc, pre.data);
    if (!EXR_OK(rc)) goto fail;

    /* part headers (maxSamplesPerPixel patched at end_stream for deep parts) */
    for (p = 0; p < w->num_parts; ++p) {
        const exr_part *pt = &w->parts[p];
        obuf hp;
        uint64_t base = w->pos;
        memset(&hp, 0, sizeof(hp));
        hp.a = &w->alloc;
        write_header(&hp, &pt->header, w->sorder[p], comp, w->smultipart,
                     w->schunk[p], 0);
        if (hp.err) { exr_free(&w->alloc, hp.data); rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
        if (header_is_deep(&pt->header)) {
            size_t rel;
            if (find_max_samples_pos(hp.data, hp.len, &rel))
                w->smax_pos[p] = base + rel;
        }
        rc = sink_write(w, hp.data, hp.len);
        exr_free(&w->alloc, hp.data);
        if (!EXR_OK(rc)) goto fail;
    }
    if (w->smultipart) {
        uint8_t z = 0;
        rc = sink_write(w, &z, 1);
        if (!EXR_OK(rc)) goto fail;
    }

    /* reserve (zeroed) offset tables */
    for (p = 0; p < w->num_parts; ++p) {
        size_t need = (size_t)w->schunk[p] * 8;
        uint8_t *zeros;
        w->soff_pos[p] = w->pos;
        if (need == 0) continue;
        zeros = (uint8_t *)exr_calloc(&w->alloc, need, 1);
        if (!zeros) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
        rc = sink_write(w, zeros, need);
        exr_free(&w->alloc, zeros);
        if (!EXR_OK(rc)) goto fail;
    }

    w->streaming = 1;
    w->sink_open = 1; /* writer now owns the sink; close it exactly once */
    return EXR_SUCCESS;

fail:
    /* sink_open is still 0: ownership has not transferred, so we do NOT close
     * the caller's sink here (the caller, e.g. begin_stream_file, owns it). */
    stream_free_state(w);
    return rc;
}

/* exr_writer_begin_stream_file lives in src/exr_stdio.c (the only stdio TU). */

/* Emit one flat chunk: record its offset, write the chunk header + payload. */
static exr_result stream_emit_flat(exr_writer *w, int part, uint32_t ci,
                                   int tiled, int tx, int ty, int lx, int ly,
                                   int y0, const uint8_t *payload,
                                   size_t payload_size) {
    uint8_t hdr[24];
    size_t hl = 0;
    exr_result rc;
    w->soff[part][ci] = w->pos;
    if (w->smultipart) { exr_wr_i32(hdr + hl, part); hl += 4; }
    if (tiled) {
        exr_wr_i32(hdr + hl, tx); hl += 4;
        exr_wr_i32(hdr + hl, ty); hl += 4;
        exr_wr_i32(hdr + hl, lx); hl += 4;
        exr_wr_i32(hdr + hl, ly); hl += 4;
    } else {
        exr_wr_i32(hdr + hl, y0); hl += 4;
    }
    exr_wr_i32(hdr + hl, (int32_t)payload_size); hl += 4;
    rc = sink_write(w, hdr, hl);
    if (!EXR_OK(rc)) return rc;
    return sink_write(w, payload, payload_size);
}

exr_result exr_writer_write_scanline_block(exr_writer *w, int32_t part,
                                           int32_t y0,
                                           const void *const *channel_rows) {
    const exr_part *pt;
    const exr_header *h;
    const exr_allocator *a;
    int ymin, ymax, lpb, nlines;
    uint32_t ci;
    size_t blk_size, payload_size = 0;
    uint8_t *block, *payload = NULL;
    exr_codec_ctx cx;
    exr_result rc;

    if (!w || !channel_rows || !w->streaming) return EXR_ERROR_INVALID_ARGUMENT;
    if (part < 0 || part >= w->num_parts) return EXR_ERROR_INVALID_ARGUMENT;
    pt = &w->parts[part];
    h = &pt->header;
    a = &w->alloc;
    if (h->tiled || header_is_deep(h)) return EXR_ERROR_INVALID_ARGUMENT;
    ymin = h->data_window.min_y;
    ymax = h->data_window.max_y;
    lpb = exr_lines_per_block(w->scomp);
    if (y0 < ymin || y0 > ymax || ((y0 - ymin) % lpb) != 0)
        return EXR_ERROR_INVALID_ARGUMENT;
    ci = (uint32_t)((y0 - ymin) / lpb);
    if (ci >= w->schunk[part]) return EXR_ERROR_INVALID_ARGUMENT;
    nlines = lpb;
    if (y0 + nlines - 1 > ymax) nlines = ymax - y0 + 1;

    rc = exr_block_uncompressed_size(h->channels, h->num_channels,
                                     h->data_window.min_x, y0, pt->width, nlines,
                                     &blk_size);
    if (!EXR_OK(rc)) return rc;
    block = (uint8_t *)exr_malloc(a, blk_size ? blk_size : 1);
    if (!block) return EXR_ERROR_OUT_OF_MEMORY;
    gather_block_local(h, w->sorder[part], channel_rows, h->data_window.min_x, y0,
                       pt->width, nlines, block);
    cx.alloc = a;
    cx.compression = w->scomp;
    cx.channels = w->ssorted[part];
    cx.num_channels = h->num_channels;
    cx.x = h->data_window.min_x;
    cx.y = y0;
    cx.width = pt->width;
    cx.num_lines = nlines;
    rc = exr_compress_block(&cx, block, blk_size, &payload, &payload_size);
    exr_free(a, block);
    if (!EXR_OK(rc)) return rc;
    rc = stream_emit_flat(w, part, ci, 0, 0, 0, 0, 0, y0, payload, payload_size);
    exr_free(a, payload);
    return rc;
}

/* Internal: stream one scanline block from an already-gathered canonical block
 * (per-scanline, then per-channel in sorted order). Used by the GPU backend,
 * which performs the planar->canonical gather on the device. `block` must be
 * exactly the size exr_block_uncompressed_size() reports for this block. */
exr_result exr_writer_write_scanline_block_canon(exr_writer *w, int32_t part,
                                                 int32_t y0, const uint8_t *block,
                                                 size_t block_size) {
    const exr_part *pt;
    const exr_header *h;
    const exr_allocator *a;
    int ymin, ymax, lpb, nlines;
    uint32_t ci;
    size_t blk_size, payload_size = 0;
    uint8_t *payload = NULL;
    exr_codec_ctx cx;
    exr_result rc;

    if (!w || !block || !w->streaming) return EXR_ERROR_INVALID_ARGUMENT;
    if (part < 0 || part >= w->num_parts) return EXR_ERROR_INVALID_ARGUMENT;
    pt = &w->parts[part];
    h = &pt->header;
    a = &w->alloc;
    if (h->tiled || header_is_deep(h)) return EXR_ERROR_INVALID_ARGUMENT;
    ymin = h->data_window.min_y;
    ymax = h->data_window.max_y;
    lpb = exr_lines_per_block(w->scomp);
    if (y0 < ymin || y0 > ymax || ((y0 - ymin) % lpb) != 0)
        return EXR_ERROR_INVALID_ARGUMENT;
    ci = (uint32_t)((y0 - ymin) / lpb);
    if (ci >= w->schunk[part]) return EXR_ERROR_INVALID_ARGUMENT;
    nlines = lpb;
    if (y0 + nlines - 1 > ymax) nlines = ymax - y0 + 1;

    rc = exr_block_uncompressed_size(h->channels, h->num_channels,
                                     h->data_window.min_x, y0, pt->width, nlines,
                                     &blk_size);
    if (!EXR_OK(rc)) return rc;
    if (block_size != blk_size) return EXR_ERROR_INVALID_ARGUMENT;
    cx.alloc = a;
    cx.compression = w->scomp;
    cx.channels = w->ssorted[part];
    cx.num_channels = h->num_channels;
    cx.x = h->data_window.min_x;
    cx.y = y0;
    cx.width = pt->width;
    cx.num_lines = nlines;
    rc = EXR_ERROR_UNSUPPORTED;
    if (w->gpu_jph_fn &&
        (w->scomp == EXR_COMPRESSION_HTJ2K256 ||
         w->scomp == EXR_COMPRESSION_HTJ2K32)) {
        rc = exr_jph_compress_gpu(&cx, block, blk_size, &payload, &payload_size,
                                  w->gpu_jph_fn, w->gpu_jph_user);
    }
    if (rc == EXR_ERROR_UNSUPPORTED) /* not HTJ2K, or a non-i32 block: CPU path */
        rc = exr_compress_block(&cx, block, blk_size, &payload, &payload_size);
    if (!EXR_OK(rc)) return rc;
    rc = stream_emit_flat(w, part, ci, 0, 0, 0, 0, 0, y0, payload, payload_size);
    exr_free(a, payload);
    return rc;
}

/* Internal: the sorted channel order the writer uses for `part` (maps sorted
 * output position -> header.channels index), so the GPU gather can build the
 * canonical block in the same order. Returns NULL if part is out of range. */
const int *exr_writer_sorted_order(exr_writer *w, int32_t part) {
    if (!w || part < 0 || part >= w->num_parts) return NULL;
    return w->sorder ? w->sorder[part] : NULL;
}

exr_result exr_writer_write_tile(exr_writer *w, int32_t part, int32_t tile_x,
                                 int32_t tile_y, int32_t level_x, int32_t level_y,
                                 const void *const *channel_data) {
    const exr_part *pt;
    const exr_header *h;
    const exr_allocator *a;
    int lw, lh, nxt, nyt, tsx, tsy, x0l, y0l, tw, th, abs_x0, abs_y0;
    uint32_t ci;
    size_t blk_size, payload_size = 0;
    uint8_t *block, *payload = NULL;
    exr_codec_ctx cx;
    exr_result rc;

    if (!w || !channel_data || !w->streaming) return EXR_ERROR_INVALID_ARGUMENT;
    if (part < 0 || part >= w->num_parts) return EXR_ERROR_INVALID_ARGUMENT;
    pt = &w->parts[part];
    h = &pt->header;
    a = &w->alloc;
    if (!h->tiled || header_is_deep(h)) return EXR_ERROR_INVALID_ARGUMENT;
    rc = w_tile_index(h, pt->width, pt->height, tile_x, tile_y, level_x, level_y,
                      &ci);
    if (!EXR_OK(rc)) return rc;
    if (ci >= w->schunk[part]) return EXR_ERROR_INVALID_ARGUMENT;
    w_level_dims(h, pt->width, pt->height, level_x, level_y, &lw, &lh, &nxt, &nyt);
    if (tile_x < 0 || tile_x >= nxt || tile_y < 0 || tile_y >= nyt)
        return EXR_ERROR_INVALID_ARGUMENT;
    tsx = (int)h->tile_x_size;
    tsy = (int)h->tile_y_size;
    x0l = tile_x * tsx;
    y0l = tile_y * tsy;
    tw = (tsx < lw - x0l) ? tsx : (lw - x0l);
    th = (tsy < lh - y0l) ? tsy : (lh - y0l);
    abs_x0 = h->data_window.min_x + x0l;
    abs_y0 = h->data_window.min_y + y0l;

    rc = exr_block_uncompressed_size(h->channels, h->num_channels, abs_x0, abs_y0,
                                     tw, th, &blk_size);
    if (!EXR_OK(rc)) return rc;
    block = (uint8_t *)exr_malloc(a, blk_size ? blk_size : 1);
    if (!block) return EXR_ERROR_OUT_OF_MEMORY;
    gather_block_local(h, w->sorder[part], channel_data, abs_x0, abs_y0, tw, th,
                       block);
    cx.alloc = a;
    cx.compression = w->scomp;
    cx.channels = w->ssorted[part];
    cx.num_channels = h->num_channels;
    cx.x = abs_x0;
    cx.y = abs_y0;
    cx.width = tw;
    cx.num_lines = th;
    rc = exr_compress_block(&cx, block, blk_size, &payload, &payload_size);
    exr_free(a, block);
    if (!EXR_OK(rc)) return rc;
    rc = stream_emit_flat(w, part, ci, 1, tile_x, tile_y, level_x, level_y, 0,
                          payload, payload_size);
    exr_free(a, payload);
    return rc;
}

/* Track the running max sample count for the deep header backpatch. */
static void stream_track_max(exr_writer *w, int part, const int32_t *counts,
                             size_t n) {
    size_t i;
    for (i = 0; i < n; ++i)
        if (counts[i] > w->smax[part]) w->smax[part] = counts[i];
}

/* Emit one deep chunk (scanline or tile). */
static exr_result stream_emit_deep(exr_writer *w, int part, uint32_t ci,
                                   int tiled, int tx, int ty, int lx, int ly,
                                   int y0, const uint8_t *poff, size_t poff_sz,
                                   const uint8_t *psamp, size_t psamp_sz,
                                   uint64_t usamp) {
    uint8_t hdr[44];
    size_t hl = 0;
    exr_result rc;
    w->soff[part][ci] = w->pos;
    if (w->smultipart) { exr_wr_i32(hdr + hl, part); hl += 4; }
    if (tiled) {
        exr_wr_i32(hdr + hl, tx); hl += 4;
        exr_wr_i32(hdr + hl, ty); hl += 4;
        exr_wr_i32(hdr + hl, lx); hl += 4;
        exr_wr_i32(hdr + hl, ly); hl += 4;
    } else {
        exr_wr_i32(hdr + hl, y0); hl += 4;
    }
    exr_wr_u64(hdr + hl, (uint64_t)poff_sz); hl += 8;
    exr_wr_u64(hdr + hl, (uint64_t)psamp_sz); hl += 8;
    exr_wr_u64(hdr + hl, usamp); hl += 8;
    rc = sink_write(w, hdr, hl);
    if (!EXR_OK(rc)) return rc;
    rc = sink_write(w, poff, poff_sz);
    if (!EXR_OK(rc)) return rc;
    return sink_write(w, psamp, psamp_sz);
}

exr_result exr_writer_write_deep_scanline_block(exr_writer *w, int32_t part,
                                                int32_t y0, const int32_t *counts,
                                                const void *const *chan_samp) {
    const exr_part *pt;
    const exr_header *h;
    const exr_allocator *a;
    int ymin, ymax, lpb, nlines;
    uint32_t ci;
    exr_part blk;
    uint64_t *prefix = NULL;
    uint8_t *poff = NULL, *psamp = NULL;
    size_t poff_sz = 0, psamp_sz = 0;
    uint64_t uoff = 0, usamp = 0;
    exr_result rc;

    if (!w || !counts || !chan_samp || !w->streaming)
        return EXR_ERROR_INVALID_ARGUMENT;
    if (part < 0 || part >= w->num_parts) return EXR_ERROR_INVALID_ARGUMENT;
    pt = &w->parts[part];
    h = &pt->header;
    a = &w->alloc;
    if (h->tiled || !header_is_deep(h)) return EXR_ERROR_INVALID_ARGUMENT;
    ymin = h->data_window.min_y;
    ymax = h->data_window.max_y;
    lpb = exr_lines_per_block(w->scomp);
    if (y0 < ymin || y0 > ymax || ((y0 - ymin) % lpb) != 0)
        return EXR_ERROR_INVALID_ARGUMENT;
    ci = (uint32_t)((y0 - ymin) / lpb);
    if (ci >= w->schunk[part]) return EXR_ERROR_INVALID_ARGUMENT;
    nlines = lpb;
    if (y0 + nlines - 1 > ymax) nlines = ymax - y0 + 1;

    /* a block-local deep part: full-width counts/samples for these scanlines */
    memset(&blk, 0, sizeof(blk));
    blk.header = *h;                 /* shallow; channels shared, not owned */
    blk.width = pt->width;
    blk.height = nlines;
    blk.is_deep = 1;
    blk.deep_sample_counts = (int32_t *)counts;
    blk.deep_images = (void **)chan_samp;
    blk.header.data_window.min_y = 0; /* so encode's row = y0-ymin maps block-local */

    {
        size_t npix = (size_t)pt->width * nlines, i;
        uint64_t acc = 0;
        prefix = (uint64_t *)exr_malloc(a, (npix ? npix : 1) * sizeof(uint64_t));
        if (!prefix) return EXR_ERROR_OUT_OF_MEMORY;
        for (i = 0; i < npix; ++i) { prefix[i] = acc; acc += (uint64_t)counts[i]; }
    }
    rc = exr_deep_encode_block(a, w->scomp, &blk, prefix, 0, nlines, &poff,
                               &poff_sz, &uoff, &psamp, &psamp_sz, &usamp);
    exr_free(a, prefix);
    if (!EXR_OK(rc)) return rc;
    stream_track_max(w, part, counts, (size_t)pt->width * nlines);
    rc = stream_emit_deep(w, part, ci, 0, 0, 0, 0, 0, y0, poff, poff_sz, psamp,
                          psamp_sz, usamp);
    exr_free(a, poff);
    exr_free(a, psamp);
    return rc;
}

exr_result exr_writer_write_deep_tile(exr_writer *w, int32_t part, int32_t tile_x,
                                      int32_t tile_y, int32_t level_x,
                                      int32_t level_y, const int32_t *counts,
                                      const void *const *chan_samp) {
    const exr_part *pt;
    const exr_header *h;
    const exr_allocator *a;
    int lw, lh, nxt, nyt, tsx, tsy, x0l, y0l, tw, th;
    uint32_t ci;
    exr_part blk;
    uint64_t *prefix = NULL;
    uint8_t *poff = NULL, *psamp = NULL;
    size_t poff_sz = 0, psamp_sz = 0;
    uint64_t uoff = 0, usamp = 0;
    exr_result rc;

    if (!w || !counts || !chan_samp || !w->streaming)
        return EXR_ERROR_INVALID_ARGUMENT;
    if (part < 0 || part >= w->num_parts) return EXR_ERROR_INVALID_ARGUMENT;
    pt = &w->parts[part];
    h = &pt->header;
    a = &w->alloc;
    if (!h->tiled || !header_is_deep(h)) return EXR_ERROR_INVALID_ARGUMENT;
    rc = w_tile_index(h, pt->width, pt->height, tile_x, tile_y, level_x, level_y,
                      &ci);
    if (!EXR_OK(rc)) return rc;
    if (ci >= w->schunk[part]) return EXR_ERROR_INVALID_ARGUMENT;
    w_level_dims(h, pt->width, pt->height, level_x, level_y, &lw, &lh, &nxt, &nyt);
    if (tile_x < 0 || tile_x >= nxt || tile_y < 0 || tile_y >= nyt)
        return EXR_ERROR_INVALID_ARGUMENT;
    tsx = (int)h->tile_x_size;
    tsy = (int)h->tile_y_size;
    x0l = tile_x * tsx;
    y0l = tile_y * tsy;
    tw = (tsx < lw - x0l) ? tsx : (lw - x0l);
    th = (tsy < lh - y0l) ? tsy : (lh - y0l);

    memset(&blk, 0, sizeof(blk));
    blk.header = *h;
    blk.width = tw;
    blk.height = th;
    blk.is_deep = 1;
    blk.deep_sample_counts = (int32_t *)counts;
    blk.deep_images = (void **)chan_samp;

    {
        size_t npix = (size_t)tw * th, i;
        uint64_t acc = 0;
        prefix = (uint64_t *)exr_malloc(a, (npix ? npix : 1) * sizeof(uint64_t));
        if (!prefix) return EXR_ERROR_OUT_OF_MEMORY;
        for (i = 0; i < npix; ++i) { prefix[i] = acc; acc += (uint64_t)counts[i]; }
    }
    rc = exr_deep_encode_tile(a, w->scomp, &blk, prefix, 0, 0, tw, th, &poff,
                              &poff_sz, &uoff, &psamp, &psamp_sz, &usamp);
    exr_free(a, prefix);
    if (!EXR_OK(rc)) return rc;
    stream_track_max(w, part, counts, (size_t)tw * th);
    rc = stream_emit_deep(w, part, ci, 1, tile_x, tile_y, level_x, level_y, 0,
                          poff, poff_sz, psamp, psamp_sz, usamp);
    exr_free(a, poff);
    exr_free(a, psamp);
    return rc;
}

exr_result exr_writer_end_stream(exr_writer *w) {
    int p;
    exr_result rc = EXR_SUCCESS;
    if (!w || !w->streaming) return EXR_ERROR_INVALID_ARGUMENT;

    /* backpatch offset tables */
    for (p = 0; p < w->num_parts && EXR_OK(rc); ++p) {
        size_t need = (size_t)w->schunk[p] * 8, k;
        uint8_t *buf;
        if (need == 0) continue;
        buf = (uint8_t *)exr_malloc(&w->alloc, need);
        if (!buf) { rc = EXR_ERROR_OUT_OF_MEMORY; break; }
        for (k = 0; k < w->schunk[p]; ++k)
            exr_wr_u64(buf + k * 8, w->soff[p][k]);
        rc = w->sink.seek(w->sink.user, w->soff_pos[p]);
        if (EXR_OK(rc)) rc = w->sink.write(w->sink.user, buf, need);
        exr_free(&w->alloc, buf);
    }

    /* backpatch deep maxSamplesPerPixel */
    for (p = 0; p < w->num_parts && EXR_OK(rc); ++p) {
        uint8_t t[4];
        if (!w->smax_pos[p]) continue;
        exr_wr_i32(t, w->smax[p]);
        rc = w->sink.seek(w->sink.user, w->smax_pos[p]);
        if (EXR_OK(rc)) rc = w->sink.write(w->sink.user, t, 4);
    }

    stream_free_state(w);
    return rc;
}
