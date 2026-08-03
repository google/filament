/*
 * TinyEXR - mid-level reader: open, parse header(s), read pixels.
 *
 * Phase 1 covers the memory path, scanline parts, and NONE compression.
 * Compressed codecs are wired through exr_decompress_block(); tiled/deep parts
 * and streaming suspend/resume arrive in later phases.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"


/* ============================================================================
 * Open / close
 * ========================================================================== */

exr_result exr_reader_open_memory(const void *data, size_t size,
                                  const exr_allocator *alloc, exr_reader **out) {
    exr_reader *r;
    if (!data || !out) return EXR_ERROR_INVALID_ARGUMENT;
    if (!alloc) alloc = exr_default_allocator();
    r = (exr_reader *)exr_calloc(alloc, 1, sizeof(*r));
    if (!r) return EXR_ERROR_OUT_OF_MEMORY;
    r->alloc = *alloc;
    r->kind = EXR_SRC_MEMORY;
    r->mem = (const uint8_t *)data;
    r->mem_size = size;
    r->free_mem = 0;
    *out = r;
    return EXR_SUCCESS;
}

exr_result exr_reader_open_source(const exr_data_source *src,
                                  const exr_allocator *alloc, exr_reader **out) {
    exr_reader *r;
    uint8_t *buf;
    exr_result rc;
    if (!src || !src->read || !out) return EXR_ERROR_INVALID_ARGUMENT;
    if (!alloc) alloc = exr_default_allocator();

    /* The whole file is buffered into `mem` incrementally; the source `read`
     * callback may return EXR_WOULD_BLOCK, in which case the host fetches the
     * pending range and feeds it via exr_reader_supply(). */
    if (src->total_size == 0) return EXR_ERROR_UNSUPPORTED;
    buf = (uint8_t *)exr_malloc(alloc, (size_t)src->total_size);
    if (!buf) return EXR_ERROR_OUT_OF_MEMORY;
    r = (exr_reader *)exr_calloc(alloc, 1, sizeof(*r));
    if (!r) {
        exr_free(alloc, buf);
        return EXR_ERROR_OUT_OF_MEMORY;
    }
    (void)rc;
    r->alloc = *alloc;
    r->kind = EXR_SRC_CALLBACK;
    r->src = *src;
    r->mem = buf;
    r->mem_size = (size_t)src->total_size;
    r->filled = 0;
    r->free_mem = 1;
    *out = r;
    return EXR_SUCCESS;
}

/* Fill the streaming buffer up to total_size. Returns EXR_WOULD_BLOCK (with
 * r->pending set) when the source has no more bytes available right now. */
exr_result exr_reader_ensure_buffered(exr_reader *r) {
    if (r->kind != EXR_SRC_CALLBACK) return EXR_SUCCESS;
    while (r->filled < r->mem_size) {
        uint64_t off = r->filled, len = r->mem_size - r->filled;
        exr_result rc = r->src.read(r->src.user, off, len,
                                    (uint8_t *)r->mem + r->filled);
        if (rc == EXR_WOULD_BLOCK) {
            r->have_pending = 1;
            r->pending.offset = off;
            r->pending.size = len;
            return EXR_WOULD_BLOCK;
        }
        if (!EXR_OK(rc)) return rc;
        /* a synchronous read fills the entire requested range */
        r->filled = r->mem_size;
    }
    r->have_pending = 0;
    return EXR_SUCCESS;
}

void exr_reader_close(exr_reader *r) {
    const exr_allocator *a;
    int32_t i;
    if (!r) return;
    a = &r->alloc;
    if (r->parts) {
        for (i = 0; i < r->num_parts; ++i) {
            exr_int_part *p = &r->parts[i];
            exr_free(a, p->offsets);
            exr_free(a, p->level_width);
            exr_free(a, p->level_height);
            exr_free(a, p->level_x_tiles);
            exr_free(a, p->level_y_tiles);
            exr_header_free(a, &p->header);
        }
        exr_free(a, r->parts);
    }
    if (r->free_mem) exr_free(a, (void *)r->mem);
    exr_free(a, r);
}

/* ============================================================================
 * Random access
 * ========================================================================== */

exr_result exr_reader_fetch(exr_reader *r, uint64_t offset, size_t size,
                            void *scratch, const uint8_t **out_ptr) {
    /* Both paths read from `mem`; the callback path has buffered `filled`
     * bytes (== mem_size once exr_reader_ensure_buffered has succeeded). */
    size_t avail = (r->kind == EXR_SRC_MEMORY) ? r->mem_size : r->filled;
    (void)scratch;
    if (offset > (uint64_t)avail) return EXR_ERROR_CORRUPT;
    if (size > avail - (size_t)offset) return EXR_ERROR_CORRUPT;
    *out_ptr = r->mem + (size_t)offset;
    return EXR_SUCCESS;
}

/* ============================================================================
 * Header parsing helpers
 * ========================================================================== */

typedef struct {
    const uint8_t *p;
    size_t size;
    size_t pos;
} cursor;

static int cur_need(const cursor *c, size_t n) {
    return (c->pos <= c->size) && (n <= c->size - c->pos);
}

/* Read a NUL-terminated string; returns pointer + length (excludes NUL). */
static exr_result cur_string(cursor *c, const char **out, size_t *out_len) {
    size_t start = c->pos;
    while (c->pos < c->size && c->p[c->pos] != 0) c->pos++;
    if (c->pos >= c->size) return EXR_ERROR_CORRUPT; /* unterminated */
    *out = (const char *)(c->p + start);
    *out_len = c->pos - start;
    c->pos++; /* consume NUL */
    return EXR_SUCCESS;
}

exr_result exr_attr_list_append(const exr_allocator *a, exr_attr_list *list,
                                const char *name, size_t name_len,
                                const char *type, size_t type_len,
                                const uint8_t *data, uint32_t size) {
    exr_attr *slot;
    if (list->count >= EXR_MAX_ATTRIBUTES) return EXR_ERROR_CORRUPT;
    if (list->count == list->capacity) {
        uint32_t ncap = list->capacity ? list->capacity * 2 : 8;
        exr_attr *ni =
            (exr_attr *)exr_calloc(a, ncap, sizeof(exr_attr));
        if (!ni) return EXR_ERROR_OUT_OF_MEMORY;
        if (list->items) {
            memcpy(ni, list->items, list->count * sizeof(exr_attr));
            exr_free(a, list->items);
        }
        list->items = ni;
        list->capacity = ncap;
    }
    slot = &list->items[list->count];
    slot->name = (char *)exr_malloc(a, name_len + 1);
    slot->type_name = (char *)exr_malloc(a, type_len + 1);
    slot->data = (uint8_t *)exr_malloc(a, size ? size : 1);
    if (!slot->name || !slot->type_name || !slot->data) {
        exr_free(a, slot->name);
        exr_free(a, slot->type_name);
        exr_free(a, slot->data);
        memset(slot, 0, sizeof(*slot));
        return EXR_ERROR_OUT_OF_MEMORY;
    }
    memcpy(slot->name, name, name_len);
    slot->name[name_len] = '\0';
    memcpy(slot->type_name, type, type_len);
    slot->type_name[type_len] = '\0';
    if (size) memcpy(slot->data, data, size);
    slot->size = size;
    list->count++;
    return EXR_SUCCESS;
}

/* Parse one header's attribute list (until empty-name terminator). */
static exr_result parse_one_header(exr_reader *r, cursor *c,
                                   exr_int_part *part) {
    const exr_allocator *a = &r->alloc;
    exr_attr_list *list;

    list = (exr_attr_list *)exr_calloc(a, 1, sizeof(*list));
    if (!list) return EXR_ERROR_OUT_OF_MEMORY;
    part->header.attrs = list;

    for (;;) {
        const char *name, *type;
        size_t name_len, type_len;
        int32_t sz;
        exr_result rc;

        if (!cur_need(c, 1)) return EXR_ERROR_CORRUPT;
        if (c->p[c->pos] == 0) {
            c->pos++; /* empty name -> end of header */
            break;
        }
        rc = cur_string(c, &name, &name_len);
        if (!EXR_OK(rc)) return rc;
        if (name_len >= EXR_MAX_NAME) return EXR_ERROR_CORRUPT;
        rc = cur_string(c, &type, &type_len);
        if (!EXR_OK(rc)) return rc;
        if (type_len >= EXR_MAX_NAME) return EXR_ERROR_CORRUPT;
        if (!cur_need(c, 4)) return EXR_ERROR_CORRUPT;
        sz = exr_rd_i32(c->p + c->pos);
        c->pos += 4;
        if (sz < 0 || (uint32_t)sz > EXR_MAX_ATTR_SIZE) return EXR_ERROR_CORRUPT;
        if (!cur_need(c, (size_t)sz)) return EXR_ERROR_CORRUPT;
        rc = exr_attr_list_append(a, list, name, name_len, type, type_len,
                                  c->p + c->pos, (uint32_t)sz);
        if (!EXR_OK(rc)) return rc;
        c->pos += (size_t)sz;
    }
    return EXR_SUCCESS;
}

/* Interpret the standard attributes into header fields. */
static exr_result interpret_header(exr_reader *r, exr_int_part *part) {
    const exr_allocator *a = &r->alloc;
    exr_header *h = &part->header;
    const exr_attr *at;
    int has_data_window = 0;

    /* defaults */
    h->compression = EXR_COMPRESSION_NONE;
    h->line_order = EXR_LINEORDER_INCREASING_Y;
    h->pixel_aspect_ratio = 1.0f;
    h->screen_window_width = 1.0f;
    h->screen_window_center_x = 0.0f;
    h->screen_window_center_y = 0.0f;
    h->name[0] = '\0';

    /* channels */
    at = exr_attr_find(h->attrs, "channels");
    if (!at) return EXR_ERROR_CORRUPT;
    {
        exr_result rc = exr_parse_chlist(a, at->data, at->size, &h->channels,
                                         &h->num_channels);
        if (!EXR_OK(rc)) return rc;
    }

    at = exr_attr_find(h->attrs, "compression");
    if (at && at->size >= 1) {
        if (at->data[0] > 12) return EXR_ERROR_CORRUPT;
        h->compression = (exr_compression)at->data[0];
    }

    at = exr_attr_find(h->attrs, "dataWindow");
    if (at && at->size >= 16) {
        h->data_window.min_x = exr_rd_i32(at->data);
        h->data_window.min_y = exr_rd_i32(at->data + 4);
        h->data_window.max_x = exr_rd_i32(at->data + 8);
        h->data_window.max_y = exr_rd_i32(at->data + 12);
        has_data_window = 1;
    }
    if (!has_data_window) return EXR_ERROR_CORRUPT;

    at = exr_attr_find(h->attrs, "displayWindow");
    if (at && at->size >= 16) {
        h->display_window.min_x = exr_rd_i32(at->data);
        h->display_window.min_y = exr_rd_i32(at->data + 4);
        h->display_window.max_x = exr_rd_i32(at->data + 8);
        h->display_window.max_y = exr_rd_i32(at->data + 12);
    } else {
        h->display_window = h->data_window;
    }

    at = exr_attr_find(h->attrs, "lineOrder");
    if (at && at->size >= 1 && at->data[0] <= 2)
        h->line_order = (exr_line_order)at->data[0];

    at = exr_attr_find(h->attrs, "pixelAspectRatio");
    if (at && at->size >= 4) h->pixel_aspect_ratio = exr_rd_f32(at->data);

    at = exr_attr_find(h->attrs, "screenWindowCenter");
    if (at && at->size >= 8) {
        h->screen_window_center_x = exr_rd_f32(at->data);
        h->screen_window_center_y = exr_rd_f32(at->data + 4);
    }

    at = exr_attr_find(h->attrs, "screenWindowWidth");
    if (at && at->size >= 4) h->screen_window_width = exr_rd_f32(at->data);

    /* chromaticities: 8 floats (red/green/blue/white xy). Used to derive
     * luminance weights for luminance-chroma (Y/RY/BY) reconstruction. */
    at = exr_attr_find(h->attrs, "chromaticities");
    if (at && at->size >= 32) {
        int i;
        for (i = 0; i < 8; ++i)
            h->chromaticities[i] = exr_rd_f32(at->data + (uint32_t)i * 4);
        h->has_chromaticities = 1;
    }

    /* tiles */
    at = exr_attr_find(h->attrs, "tiles");
    if (at && at->size >= 9) {
        h->tiled = 1;
        h->tile_x_size = exr_rd_u32(at->data);
        h->tile_y_size = exr_rd_u32(at->data + 4);
        h->level_mode = (exr_tile_level_mode)(at->data[8] & 0x0f);
        h->rounding_mode = (exr_tile_rounding_mode)((at->data[8] >> 4) & 0x0f);
        if (h->level_mode > EXR_TILE_RIPMAP_LEVELS) return EXR_ERROR_CORRUPT;
    }

    /* name */
    at = exr_attr_find(h->attrs, "name");
    if (at && at->size > 0) {
        uint32_t n = at->size;
        if (n >= EXR_MAX_NAME) n = EXR_MAX_NAME - 1;
        memcpy(h->name, at->data, n);
        h->name[n] = '\0';
    }

    /* part type: from "type" attribute, else from version flags */
    at = exr_attr_find(h->attrs, "type");
    if (at && at->size > 0) {
        const char *t = (const char *)at->data;
        size_t n = at->size;
        if (n == 13 && memcmp(t, "scanlineimage", 13) == 0)
            h->part_type = EXR_PART_SCANLINE;
        else if (n == 10 && memcmp(t, "tiledimage", 10) == 0) {
            h->part_type = EXR_PART_TILED;
            h->tiled = 1;
        } else if (n == 12 && memcmp(t, "deepscanline", 12) == 0)
            h->part_type = EXR_PART_DEEP_SCANLINE;
        else if (n == 8 && memcmp(t, "deeptile", 8) == 0) {
            h->part_type = EXR_PART_DEEP_TILED;
            h->tiled = 1;
        } else
            h->part_type = h->tiled ? EXR_PART_TILED : EXR_PART_SCANLINE;
    } else {
        if (r->is_deep)
            h->part_type =
                h->tiled ? EXR_PART_DEEP_TILED : EXR_PART_DEEP_SCANLINE;
        else
            h->part_type = h->tiled ? EXR_PART_TILED : EXR_PART_SCANLINE;
    }

    /* geometry */
    {
        int64_t w = (int64_t)h->data_window.max_x - h->data_window.min_x + 1;
        int64_t hh = (int64_t)h->data_window.max_y - h->data_window.min_y + 1;
        if (w <= 0 || hh <= 0 || w > EXR_MAX_DIMENSION || hh > EXR_MAX_DIMENSION)
            return EXR_ERROR_CORRUPT;
        part->width = (int32_t)w;
        part->height = (int32_t)hh;
    }
    return EXR_SUCCESS;
}

/* Dimensions and tile counts of one tile level. */
static void tiled_level_size(const exr_int_part *p, int lx, int ly, int *w,
                             int *h, int *nxt, int *nyt) {
    int lw = p->width, lh = p->height, i;
    int up = (p->header.rounding_mode == EXR_TILE_ROUND_UP);
    int tx = (int)p->header.tile_x_size, ty = (int)p->header.tile_y_size;
    if (p->header.level_mode == EXR_TILE_MIPMAP_LEVELS) {
        for (i = 0; i < lx; ++i) {
            lw = up ? (lw + 1) / 2 : lw / 2;
            lh = up ? (lh + 1) / 2 : lh / 2;
            if (lw < 1) lw = 1;
            if (lh < 1) lh = 1;
        }
    } else if (p->header.level_mode == EXR_TILE_RIPMAP_LEVELS) {
        for (i = 0; i < lx; ++i) { lw = up ? (lw + 1) / 2 : lw / 2; if (lw < 1) lw = 1; }
        for (i = 0; i < ly; ++i) { lh = up ? (lh + 1) / 2 : lh / 2; if (lh < 1) lh = 1; }
    }
    if (w) *w = lw;
    if (h) *h = lh;
    if (tx > 0 && nxt) *nxt = (lw + tx - 1) / tx;
    if (ty > 0 && nyt) *nyt = (lh + ty - 1) / ty;
}

/* Total chunk count of a tiled part; also fills num_x/y_levels. */
static exr_result tiled_total_chunks(exr_int_part *p, uint32_t *out) {
    int up = (p->header.rounding_mode == EXR_TILE_ROUND_UP);
    int tx = (int)p->header.tile_x_size, ty = (int)p->header.tile_y_size;
    uint64_t total = 0;
    if (tx <= 0 || ty <= 0) return EXR_ERROR_CORRUPT;

    p->num_x_levels = 1;
    p->num_y_levels = 1;

    if (p->header.level_mode == EXR_TILE_ONE_LEVEL) {
        int nx = (p->width + tx - 1) / tx, ny = (p->height + ty - 1) / ty;
        total = (uint64_t)nx * (uint64_t)ny;
    } else if (p->header.level_mode == EXR_TILE_MIPMAP_LEVELS) {
        int w = p->width, h = p->height, levels = 0;
        for (;;) {
            int nx = (w + tx - 1) / tx, ny = (h + ty - 1) / ty;
            total += (uint64_t)nx * (uint64_t)ny;
            levels++;
            if (w <= 1 && h <= 1) break;
            w = up ? (w + 1) / 2 : w / 2;
            h = up ? (h + 1) / 2 : h / 2;
            if (w < 1) w = 1;
            if (h < 1) h = 1;
        }
        p->num_x_levels = (int32_t)levels;
        p->num_y_levels = (int32_t)levels;
    } else { /* RIPMAP */
        int w = p->width, h = p->height, nxl = 1, nyl = 1, lx, ly;
        while (w > 1) { nxl++; w = up ? (w + 1) / 2 : w / 2; if (w < 1) w = 1; }
        while (h > 1) { nyl++; h = up ? (h + 1) / 2 : h / 2; if (h < 1) h = 1; }
        p->num_x_levels = (int32_t)nxl;
        p->num_y_levels = (int32_t)nyl;
        for (ly = 0; ly < nyl; ++ly)
            for (lx = 0; lx < nxl; ++lx) {
                int nxt, nyt;
                tiled_level_size(p, lx, ly, NULL, NULL, &nxt, &nyt);
                total += (uint64_t)nxt * (uint64_t)nyt;
            }
    }
    if (total > 0xffffffffu) return EXR_ERROR_CORRUPT;
    *out = (uint32_t)total;
    return EXR_SUCCESS;
}

/* Number of chunks (offset-table entries) for a part. */
static exr_result part_chunk_count(exr_reader *r, exr_int_part *part,
                                   uint32_t *out_count) {
    const exr_header *h = &part->header;
    const exr_attr *cc = exr_attr_find(h->attrs, "chunkCount");
    (void)r;
    if (h->part_type == EXR_PART_TILED || h->part_type == EXR_PART_DEEP_TILED) {
        /* Tiled offset tables always have an entry per tile across all levels;
         * compute it (and the level counts) directly. */
        return tiled_total_chunks(part, out_count);
    }
    if (cc && cc->size >= 4) {
        int32_t v = exr_rd_i32(cc->data);
        if (v < 0) return EXR_ERROR_CORRUPT;
        *out_count = (uint32_t)v;
        return EXR_SUCCESS;
    }
    if (h->part_type == EXR_PART_SCANLINE ||
        h->part_type == EXR_PART_DEEP_SCANLINE) {
        int lpb = exr_lines_per_block(h->compression);
        *out_count = (uint32_t)(((int64_t)part->height + lpb - 1) / lpb);
        return EXR_SUCCESS;
    }
    return EXR_ERROR_UNSUPPORTED;
}

/* ============================================================================
 * Parse header (public)
 * ========================================================================== */

exr_result exr_reader_parse_header(exr_reader *r) {
    cursor c;
    uint32_t ver;
    int vnum;
    int32_t i;
    exr_result rc;
    int32_t cap = 0;

    if (!r) return EXR_ERROR_INVALID_ARGUMENT;
    if (r->parsed) return EXR_SUCCESS;
    rc = exr_reader_ensure_buffered(r);
    if (rc != EXR_SUCCESS) return rc; /* EXR_WOULD_BLOCK or error */

    if (r->mem_size < 8) return EXR_ERROR_INVALID_FILE;
    if (exr_rd_u32(r->mem) != EXR_MAGIC) return EXR_ERROR_INVALID_FILE;
    ver = exr_rd_u32(r->mem + 4);
    vnum = (int)(ver & 0xff);
    if (vnum > EXR_VERSION_NUMBER) return EXR_ERROR_UNSUPPORTED;

    r->version_flags = ver;
    r->is_tiled = (ver & EXR_VERSION_FLAG_TILED) != 0;
    r->long_names = (ver & EXR_VERSION_FLAG_LONG_NAMES) != 0;
    r->is_deep = (ver & EXR_VERSION_FLAG_NON_IMAGE) != 0;
    r->is_multipart = (ver & EXR_VERSION_FLAG_MULTIPART) != 0;

    c.p = r->mem;
    c.size = r->mem_size;
    c.pos = 8;

    /* Parse part headers. */
    for (;;) {
        exr_int_part *np;
        if (r->is_multipart) {
            if (!cur_need(&c, 1)) {
                rc = EXR_ERROR_CORRUPT;
                goto fail;
            }
            if (c.p[c.pos] == 0) {
                c.pos++; /* terminating empty header */
                break;
            }
        }
        if (r->num_parts >= EXR_MAX_PARTS) {
            rc = EXR_ERROR_CORRUPT;
            goto fail;
        }
        if (r->num_parts == cap) {
            int32_t ncap = cap ? cap * 2 : 1;
            exr_int_part *npb = (exr_int_part *)exr_calloc(
                &r->alloc, (size_t)ncap, sizeof(exr_int_part));
            if (!npb) {
                rc = EXR_ERROR_OUT_OF_MEMORY;
                goto fail;
            }
            if (r->parts) {
                memcpy(npb, r->parts,
                       (size_t)r->num_parts * sizeof(exr_int_part));
                exr_free(&r->alloc, r->parts);
            }
            r->parts = npb;
            cap = ncap;
        }
        np = &r->parts[r->num_parts];
        memset(np, 0, sizeof(*np));
        rc = parse_one_header(r, &c, np);
        if (!EXR_OK(rc)) {
            r->num_parts++; /* so cleanup frees its attrs */
            goto fail;
        }
        rc = interpret_header(r, np);
        if (!EXR_OK(rc)) {
            r->num_parts++;
            goto fail;
        }
        r->num_parts++;
        if (!r->is_multipart) break;
    }

    if (r->num_parts == 0) {
        rc = EXR_ERROR_CORRUPT;
        goto fail;
    }

    /* Offset tables, in part order. */
    for (i = 0; i < r->num_parts; ++i) {
        exr_int_part *p = &r->parts[i];
        uint32_t n, k;
        size_t need;
        rc = part_chunk_count(r, p, &n);
        if (!EXR_OK(rc)) goto fail;
        if (n > r->mem_size / 8) {
            rc = EXR_ERROR_CORRUPT;
            goto fail;
        }
        if (exr_mul_ovf((size_t)n, 8u, &need)) {
            rc = EXR_ERROR_CORRUPT;
            goto fail;
        }
        if (!cur_need(&c, need)) {
            rc = EXR_ERROR_CORRUPT;
            goto fail;
        }
        p->offsets = (uint64_t *)exr_calloc(&r->alloc, n ? n : 1, sizeof(uint64_t));
        if (!p->offsets) {
            rc = EXR_ERROR_OUT_OF_MEMORY;
            goto fail;
        }
        for (k = 0; k < n; ++k)
            p->offsets[k] = exr_rd_u64(c.p + c.pos + (size_t)k * 8);
        p->num_chunks = n;
        c.pos += need;
    }

    r->parsed = 1;
    return EXR_SUCCESS;

fail:
    return rc;
}

int32_t exr_reader_num_parts(const exr_reader *r) {
    return r ? r->num_parts : 0;
}

const exr_header *exr_reader_part_header(const exr_reader *r, int32_t part) {
    if (!r || part < 0 || part >= r->num_parts) return NULL;
    return &r->parts[part].header;
}

/* ============================================================================
 * Scanline read
 * ========================================================================== */

static int64_t floordiv64(int64_t a, int64_t b) {
    int64_t q = a / b, rem = a % b;
    if ((rem != 0) && ((rem < 0) != (b < 0))) q--;
    return q;
}
static int nsamp(int lo, int hi, int s) {
    if (s <= 1) return hi - lo + 1;
    return (int)(floordiv64(hi, s) - floordiv64((int64_t)lo - 1, s));
}

/* Per-channel sampled width/height of a part's data window. */
static void channel_dims(const exr_header *h, int c, int *cw, int *ch_) {
    int xs = h->channels[c].x_sampling, ys = h->channels[c].y_sampling;
    *cw = nsamp(h->data_window.min_x, h->data_window.max_x, xs);
    *ch_ = nsamp(h->data_window.min_y, h->data_window.max_y, ys);
}

/* Scatter one decompressed scanline block into the planar channel buffers. */
static exr_result scatter_scanline_block(const exr_header *h, void **images,
                                         int32_t y0, int32_t nlines,
                                         const uint8_t *block, size_t block_size) {
    int xmin = h->data_window.min_x, xmax = h->data_window.max_x;
    int ymin = h->data_window.min_y;
    size_t off = 0;
    int line, c;
    for (line = 0; line < nlines; ++line) {
        int yy = y0 + line;
        for (c = 0; c < h->num_channels; ++c) {
            int xs = h->channels[c].x_sampling, ys = h->channels[c].y_sampling;
            size_t ps = exr_pixel_size(h->channels[c].pixel_type);
            int nx, row, cw, ch_;
            size_t bytes;
            if ((yy % ys) != 0) continue;
            nx = nsamp(xmin, xmax, xs);
            if (nx <= 0) continue;
            channel_dims(h, c, &cw, &ch_);
            row = nsamp(ymin, yy, ys) - 1;
            if (row < 0 || row >= ch_) return EXR_ERROR_CORRUPT;
            bytes = (size_t)nx * ps;
            if (off + bytes > block_size) return EXR_ERROR_CORRUPT;
            memcpy((uint8_t *)images[c] + (size_t)row * (size_t)cw * ps,
                   block + off, bytes);
            off += bytes;
        }
    }
    return EXR_SUCCESS;
}

#if defined(EXR_USE_THREADS)
/* Decode a single scanline chunk into the (pre-allocated) output planes, using
 * a private scratch buffer. Self-contained so it can run on any worker thread;
 * chunks write disjoint output rows, so no locking is needed. */
static exr_result decode_scanline_chunk(exr_reader *r, exr_int_part *p,
                                        int32_t part_idx, exr_part *out,
                                        uint32_t ci) {
    const exr_allocator *a = &r->alloc;
    const exr_header *h = &p->header;
    int lpb = exr_lines_per_block(h->compression);
    int ymin = h->data_window.min_y, ymax = h->data_window.max_y;
    uint64_t off = p->offsets[ci];
    const uint8_t *hdr, *cdata;
    int32_t y0, data_size, nlines;
    size_t hdr_size = r->is_multipart ? 12 : 8;
    size_t want, dst_size;
    exr_codec_ctx ctx;
    uint8_t *block;
    exr_result rc;

    rc = exr_reader_fetch(r, off, hdr_size, NULL, &hdr);
    if (!EXR_OK(rc)) return rc;
    if (r->is_multipart) {
        if (exr_rd_i32(hdr) != part_idx) return EXR_ERROR_CORRUPT;
        hdr += 4;
    }
    y0 = exr_rd_i32(hdr);
    data_size = exr_rd_i32(hdr + 4);
    if (data_size < 0 || y0 < ymin || y0 > ymax) return EXR_ERROR_CORRUPT;
    nlines = lpb;
    if (y0 + nlines - 1 > ymax) nlines = ymax - y0 + 1;

    rc = exr_block_uncompressed_size(h->channels, h->num_channels,
                                     h->data_window.min_x, y0, p->width, nlines,
                                     &dst_size);
    if (!EXR_OK(rc)) return rc;
    if (exr_add_ovf((size_t)off, hdr_size, &want)) return EXR_ERROR_CORRUPT;
    rc = exr_reader_fetch(r, want, (size_t)data_size, NULL, &cdata);
    if (!EXR_OK(rc)) return rc;

    block = (uint8_t *)exr_malloc(a, dst_size ? dst_size : 1);
    if (!block) return EXR_ERROR_OUT_OF_MEMORY;
    ctx.alloc = a;
    ctx.compression = h->compression;
    ctx.channels = h->channels;
    ctx.num_channels = h->num_channels;
    ctx.x = h->data_window.min_x;
    ctx.y = y0;
    ctx.width = p->width;
    ctx.num_lines = nlines;
    rc = exr_decompress_block(&ctx, cdata, (size_t)data_size, block, dst_size);
    if (EXR_OK(rc))
        rc = scatter_scanline_block(h, out->images, y0, nlines, block, dst_size);
    exr_free(a, block);
    return rc;
}

typedef struct {
    exr_reader *r;
    exr_int_part *p;
    int32_t part_idx;
    exr_part *out;
    exr_result *rc; /* per-chunk results */
} scanline_job_ctx;

static void decode_scanline_job(void *vctx, int job) {
    scanline_job_ctx *jc = (scanline_job_ctx *)vctx;
    uint32_t ci = (uint32_t)job + 1u; /* chunk 0 is decoded inline first */
    jc->rc[ci] = decode_scanline_chunk(jc->r, jc->p, jc->part_idx, jc->out, ci);
}
#endif /* EXR_USE_THREADS */

static exr_result read_scanline_part(exr_reader *r, exr_int_part *p,
                                     int32_t part_idx, exr_part *out) {
    const exr_allocator *a = &r->alloc;
    const exr_header *h = &p->header;
    int lpb = exr_lines_per_block(h->compression);
    int ymin = h->data_window.min_y, ymax = h->data_window.max_y;
    uint8_t *block = NULL;
    size_t block_cap = 0;
    exr_result rc = EXR_SUCCESS;
    uint32_t ci;
    int c;

    /* allocate planar channel buffers */
    out->images = (void **)exr_calloc(a, (size_t)h->num_channels, sizeof(void *));
    if (!out->images) return EXR_ERROR_OUT_OF_MEMORY;
    for (c = 0; c < h->num_channels; ++c) {
        int cw, ch_;
        size_t n, bytes;
        channel_dims(h, c, &cw, &ch_);
        if (cw < 0) cw = 0;
        if (ch_ < 0) ch_ = 0;
        if (exr_mul_ovf((size_t)cw, (size_t)ch_, &n) ||
            exr_mul_ovf(n, exr_pixel_size(h->channels[c].pixel_type), &bytes))
            return EXR_ERROR_CORRUPT;
        out->images[c] = exr_calloc(a, bytes ? bytes : 1, 1);
        if (!out->images[c]) return EXR_ERROR_OUT_OF_MEMORY;
    }

#if defined(EXR_USE_THREADS)
    /* Parallel path: only for fully-in-memory sources (concurrent fetch is a
     * read-only pointer return) with more than one chunk. */
    if (exr_get_num_threads() > 1 && p->num_chunks > 1 &&
        r->kind == EXR_SRC_MEMORY) {
        exr_result *rcs =
            (exr_result *)exr_calloc(a, p->num_chunks, sizeof(exr_result));
        if (rcs) {
            uint32_t k;
            exr_result first = EXR_SUCCESS;
            /* Decode chunk 0 inline so lazy global inits (SIMD table, B44/JPH
             * perceptual tables) are warmed by one thread before the fan-out. */
            rcs[0] = decode_scanline_chunk(r, p, part_idx, out, 0);
            if (EXR_OK(rcs[0])) {
                scanline_job_ctx jc;
                jc.r = r; jc.p = p; jc.part_idx = part_idx; jc.out = out;
                jc.rc = rcs;
                exr_parallel_for(exr_get_num_threads(),
                                 (int)(p->num_chunks - 1), decode_scanline_job,
                                 &jc);
            }
            for (k = 0; k < p->num_chunks; ++k)
                if (!EXR_OK(rcs[k])) { first = rcs[k]; break; }
            exr_free(a, rcs);
            return first;
        }
        /* allocation failed -> fall through to the serial path */
    }
#endif

    for (ci = 0; ci < p->num_chunks; ++ci) {
        uint64_t off = p->offsets[ci];
        const uint8_t *hdr;
        int32_t y0, data_size, nlines;
        size_t hdr_size = r->is_multipart ? 12 : 8; /* [part]+y+size */
        size_t want;
        const uint8_t *cdata;
        size_t dst_size;
        exr_codec_ctx ctx;

        rc = exr_reader_fetch(r, off, hdr_size, NULL, &hdr);
        if (!EXR_OK(rc)) goto done;
        if (r->is_multipart) {
            if (exr_rd_i32(hdr) != part_idx) {
                rc = EXR_ERROR_CORRUPT;
                goto done;
            }
            hdr += 4;
        }
        y0 = exr_rd_i32(hdr);
        data_size = exr_rd_i32(hdr + 4);
        if (data_size < 0 || y0 < ymin || y0 > ymax) {
            rc = EXR_ERROR_CORRUPT;
            goto done;
        }
        nlines = lpb;
        if (y0 + nlines - 1 > ymax) nlines = ymax - y0 + 1;

        rc = exr_block_uncompressed_size(h->channels, h->num_channels,
                                         h->data_window.min_x, y0, p->width,
                                         nlines, &dst_size);
        if (!EXR_OK(rc)) goto done;

        if (exr_add_ovf((size_t)off, hdr_size, &want)) {
            rc = EXR_ERROR_CORRUPT;
            goto done;
        }
        rc = exr_reader_fetch(r, want, (size_t)data_size, NULL, &cdata);
        if (!EXR_OK(rc)) goto done;

        if (dst_size > block_cap) {
            exr_free(a, block);
            block = (uint8_t *)exr_malloc(a, dst_size ? dst_size : 1);
            if (!block) {
                rc = EXR_ERROR_OUT_OF_MEMORY;
                goto done;
            }
            block_cap = dst_size;
        }

        ctx.alloc = a;
        ctx.compression = h->compression;
        ctx.channels = h->channels;
        ctx.num_channels = h->num_channels;
        ctx.x = h->data_window.min_x;
        ctx.y = y0;
        ctx.width = p->width;
        ctx.num_lines = nlines;
        rc = exr_decompress_block(&ctx, cdata, (size_t)data_size, block,
                                  dst_size);
        if (!EXR_OK(rc)) goto done;

        rc = scatter_scanline_block(h, out->images, y0, nlines, block, dst_size);
        if (!EXR_OK(rc)) goto done;
    }

done:
    exr_free(a, block);
    return rc;
}

/* Scatter one decompressed tile block into the planar channel buffers. */
static exr_result scatter_tile_block(const exr_header *h, void **images,
                                     int abs_x0, int abs_y0, int tile_w,
                                     int tile_h, const uint8_t *block,
                                     size_t block_size) {
    int xmin = h->data_window.min_x, ymin = h->data_window.min_y;
    size_t off = 0;
    int row, c;
    for (row = 0; row < tile_h; ++row) {
        int yy = abs_y0 + row;
        for (c = 0; c < h->num_channels; ++c) {
            int xs = h->channels[c].x_sampling, ys = h->channels[c].y_sampling;
            size_t ps = exr_pixel_size(h->channels[c].pixel_type);
            int tnx, cw, ch_, ch_row, ch_col;
            size_t bytes;
            if ((yy % ys) != 0) continue;
            tnx = nsamp(abs_x0, abs_x0 + tile_w - 1, xs);
            if (tnx <= 0) continue;
            channel_dims(h, c, &cw, &ch_);
            ch_row = nsamp(ymin, yy, ys) - 1;
            ch_col = nsamp(xmin, abs_x0 - 1, xs);
            if (ch_row < 0 || ch_row >= ch_) return EXR_ERROR_CORRUPT;
            if (ch_col < 0 || ch_col + tnx > cw) return EXR_ERROR_CORRUPT;
            bytes = (size_t)tnx * ps;
            if (off + bytes > block_size) return EXR_ERROR_CORRUPT;
            memcpy((uint8_t *)images[c] +
                       ((size_t)ch_row * cw + ch_col) * ps,
                   block + off, bytes);
            off += bytes;
        }
    }
    return EXR_SUCCESS;
}

#if defined(EXR_USE_THREADS)
/* Decode a single level-0 tile (chunk index idx, grid width nxt) into the
 * output planes with a private scratch buffer; tiles write disjoint regions. */
static exr_result decode_tile_chunk(exr_reader *r, exr_int_part *p,
                                    int32_t part_idx, exr_part *out, int nxt,
                                    uint32_t idx) {
    const exr_allocator *a = &r->alloc;
    const exr_header *h = &p->header;
    int tx = (int)h->tile_x_size, ty = (int)h->tile_y_size;
    int txi = (int)(idx % (uint32_t)nxt);
    int tyi = (int)(idx / (uint32_t)nxt);
    uint64_t off = p->offsets[idx];
    const uint8_t *hdr, *cdata;
    size_t hdr_size = r->is_multipart ? 24 : 20;
    int32_t htx, hty, hlx, hly, dsize;
    int x0, y0, tile_w, tile_h, abs_x0, abs_y0;
    size_t dst_size, want;
    exr_codec_ctx ctx;
    uint8_t *block;
    exr_result rc;

    rc = exr_reader_fetch(r, off, hdr_size, NULL, &hdr);
    if (!EXR_OK(rc)) return rc;
    if (r->is_multipart) {
        if (exr_rd_i32(hdr) != part_idx) return EXR_ERROR_CORRUPT;
        hdr += 4;
    }
    htx = exr_rd_i32(hdr);
    hty = exr_rd_i32(hdr + 4);
    hlx = exr_rd_i32(hdr + 8);
    hly = exr_rd_i32(hdr + 12);
    dsize = exr_rd_i32(hdr + 16);
    if (hlx != 0 || hly != 0 || htx != txi || hty != tyi || dsize < 0)
        return EXR_ERROR_CORRUPT;
    x0 = txi * tx;
    y0 = tyi * ty;
    tile_w = (tx < p->width - x0) ? tx : (p->width - x0);
    tile_h = (ty < p->height - y0) ? ty : (p->height - y0);
    abs_x0 = h->data_window.min_x + x0;
    abs_y0 = h->data_window.min_y + y0;
    rc = exr_block_uncompressed_size(h->channels, h->num_channels, abs_x0,
                                     abs_y0, tile_w, tile_h, &dst_size);
    if (!EXR_OK(rc)) return rc;
    if (exr_add_ovf((size_t)off, hdr_size, &want)) return EXR_ERROR_CORRUPT;
    rc = exr_reader_fetch(r, want, (size_t)dsize, NULL, &cdata);
    if (!EXR_OK(rc)) return rc;
    block = (uint8_t *)exr_malloc(a, dst_size ? dst_size : 1);
    if (!block) return EXR_ERROR_OUT_OF_MEMORY;
    ctx.alloc = a;
    ctx.compression = h->compression;
    ctx.channels = h->channels;
    ctx.num_channels = h->num_channels;
    ctx.x = abs_x0;
    ctx.y = abs_y0;
    ctx.width = tile_w;
    ctx.num_lines = tile_h;
    rc = exr_decompress_block(&ctx, cdata, (size_t)dsize, block, dst_size);
    if (EXR_OK(rc))
        rc = scatter_tile_block(h, out->images, abs_x0, abs_y0, tile_w, tile_h,
                                block, dst_size);
    exr_free(a, block);
    return rc;
}

typedef struct {
    exr_reader *r;
    exr_int_part *p;
    int32_t part_idx;
    exr_part *out;
    int nxt;
    exr_result *rc;
} tile_job_ctx;

static void decode_tile_job(void *vctx, int job) {
    tile_job_ctx *jc = (tile_job_ctx *)vctx;
    uint32_t idx = (uint32_t)job + 1u; /* tile 0 decoded inline first */
    jc->rc[idx] =
        decode_tile_chunk(jc->r, jc->p, jc->part_idx, jc->out, jc->nxt, idx);
}
#endif /* EXR_USE_THREADS */

static exr_result read_tiled_part(exr_reader *r, exr_int_part *p,
                                  int32_t part_idx, exr_part *out) {
    const exr_allocator *a = &r->alloc;
    const exr_header *h = &p->header;
    int tx = (int)h->tile_x_size, ty = (int)h->tile_y_size;
    int nxt, nyt, txi, tyi, c;
    uint8_t *block = NULL;
    size_t block_cap = 0;
    exr_result rc = EXR_SUCCESS;

    if (tx <= 0 || ty <= 0) return EXR_ERROR_CORRUPT;
    nxt = (p->width + tx - 1) / tx;
    nyt = (p->height + ty - 1) / ty;

    out->images = (void **)exr_calloc(a, (size_t)h->num_channels, sizeof(void *));
    if (!out->images) return EXR_ERROR_OUT_OF_MEMORY;
    for (c = 0; c < h->num_channels; ++c) {
        int cw, ch_;
        size_t n, bytes;
        channel_dims(h, c, &cw, &ch_);
        if (cw < 0) cw = 0;
        if (ch_ < 0) ch_ = 0;
        if (exr_mul_ovf((size_t)cw, (size_t)ch_, &n) ||
            exr_mul_ovf(n, exr_pixel_size(h->channels[c].pixel_type), &bytes))
            return EXR_ERROR_CORRUPT;
        out->images[c] = exr_calloc(a, bytes ? bytes : 1, 1);
        if (!out->images[c]) return EXR_ERROR_OUT_OF_MEMORY;
    }

#if defined(EXR_USE_THREADS)
    /* Parallel path for single-level tiled, fully-in-memory sources only. */
    if (exr_get_num_threads() > 1 && p->num_chunks > 1 &&
        r->kind == EXR_SRC_MEMORY &&
        p->num_chunks == (uint32_t)nxt * (uint32_t)nyt) {
        exr_result *rcs =
            (exr_result *)exr_calloc(a, p->num_chunks, sizeof(exr_result));
        if (rcs) {
            uint32_t k;
            exr_result first = EXR_SUCCESS;
            rcs[0] = decode_tile_chunk(r, p, part_idx, out, nxt, 0); /* warm */
            if (EXR_OK(rcs[0])) {
                tile_job_ctx jc;
                jc.r = r; jc.p = p; jc.part_idx = part_idx; jc.out = out;
                jc.nxt = nxt; jc.rc = rcs;
                exr_parallel_for(exr_get_num_threads(),
                                 (int)(p->num_chunks - 1), decode_tile_job, &jc);
            }
            for (k = 0; k < p->num_chunks; ++k)
                if (!EXR_OK(rcs[k])) { first = rcs[k]; break; }
            exr_free(a, rcs);
            return first;
        }
    }
#endif

    for (tyi = 0; tyi < nyt; ++tyi) {
        for (txi = 0; txi < nxt; ++txi) {
            uint32_t idx = (uint32_t)tyi * (uint32_t)nxt + (uint32_t)txi;
            uint64_t off;
            const uint8_t *hdr, *cdata;
            size_t hdr_size = r->is_multipart ? 24 : 20;
            int32_t htx, hty, hlx, hly, dsize;
            int x0, y0, tile_w, tile_h, abs_x0, abs_y0;
            size_t dst_size, want;
            exr_codec_ctx ctx;

            if (idx >= p->num_chunks) { rc = EXR_ERROR_CORRUPT; goto done; }
            off = p->offsets[idx];
            rc = exr_reader_fetch(r, off, hdr_size, NULL, &hdr);
            if (!EXR_OK(rc)) goto done;
            if (r->is_multipart) {
                if (exr_rd_i32(hdr) != part_idx) { rc = EXR_ERROR_CORRUPT; goto done; }
                hdr += 4;
            }
            htx = exr_rd_i32(hdr);
            hty = exr_rd_i32(hdr + 4);
            hlx = exr_rd_i32(hdr + 8);
            hly = exr_rd_i32(hdr + 12);
            dsize = exr_rd_i32(hdr + 16);
            if (hlx != 0 || hly != 0 || htx != txi || hty != tyi || dsize < 0) {
                rc = EXR_ERROR_CORRUPT;
                goto done;
            }

            x0 = txi * tx;
            y0 = tyi * ty;
            tile_w = (tx < p->width - x0) ? tx : (p->width - x0);
            tile_h = (ty < p->height - y0) ? ty : (p->height - y0);
            abs_x0 = h->data_window.min_x + x0;
            abs_y0 = h->data_window.min_y + y0;

            rc = exr_block_uncompressed_size(h->channels, h->num_channels,
                                             abs_x0, abs_y0, tile_w, tile_h,
                                             &dst_size);
            if (!EXR_OK(rc)) goto done;

            if (exr_add_ovf((size_t)off, hdr_size, &want)) { rc = EXR_ERROR_CORRUPT; goto done; }
            rc = exr_reader_fetch(r, want, (size_t)dsize, NULL, &cdata);
            if (!EXR_OK(rc)) goto done;

            if (dst_size > block_cap) {
                exr_free(a, block);
                block = (uint8_t *)exr_malloc(a, dst_size ? dst_size : 1);
                if (!block) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
                block_cap = dst_size;
            }

            ctx.alloc = a;
            ctx.compression = h->compression;
            ctx.channels = h->channels;
            ctx.num_channels = h->num_channels;
            ctx.x = abs_x0;
            ctx.y = abs_y0;
            ctx.width = tile_w;
            ctx.num_lines = tile_h;
            rc = exr_decompress_block(&ctx, cdata, (size_t)dsize, block, dst_size);
            if (!EXR_OK(rc)) goto done;

            rc = scatter_tile_block(h, out->images, abs_x0, abs_y0, tile_w,
                                    tile_h, block, dst_size);
            if (!EXR_OK(rc)) goto done;
        }
    }

done:
    exr_free(a, block);
    return rc;
}

exr_result exr_reader_read_part(exr_reader *r, int32_t part, exr_part *out) {
    exr_int_part *p;
    exr_result rc;
    if (!r || !out) return EXR_ERROR_INVALID_ARGUMENT;
    rc = exr_reader_parse_header(r);
    if (rc != EXR_SUCCESS) return rc; /* propagate EXR_WOULD_BLOCK / error */
    if (part < 0 || part >= r->num_parts) return EXR_ERROR_INVALID_ARGUMENT;
    p = &r->parts[part];

    memset(out, 0, sizeof(*out));
    rc = exr_header_copy(&r->alloc, &out->header, &p->header);
    if (!EXR_OK(rc)) return rc;
    out->width = p->width;
    out->height = p->height;

    switch (p->header.part_type) {
    case EXR_PART_SCANLINE:
        rc = read_scanline_part(r, p, part, out);
        break;
    case EXR_PART_TILED:
        rc = read_tiled_part(r, p, part, out);
        break;
    case EXR_PART_DEEP_SCANLINE:
        rc = exr_read_deep_scanline_part(r, p, part, out);
        break;
    case EXR_PART_DEEP_TILED:
        rc = exr_read_deep_tiled_part(r, p, part, out);
        break;
    default:
        rc = EXR_ERROR_UNSUPPORTED;
        break;
    }
    if (!EXR_OK(rc)) exr_part_free(&r->alloc, out); /* on error, out owns nothing */
    return rc;
}

exr_result exr_reader_read_scanlines(exr_reader *r, int32_t part,
                                     int32_t y_start, int32_t y_count,
                                     exr_part *out) {
    exr_int_part *p;
    const exr_header *h;
    const exr_allocator *a;
    int lpb, ymin, ymax, y_end, c, blk, first_blk, last_blk;
    uint8_t *block = NULL;
    size_t block_cap = 0;
    exr_result rc;

    if (!r || !out) return EXR_ERROR_INVALID_ARGUMENT;
    rc = exr_reader_parse_header(r);
    if (rc != EXR_SUCCESS) return rc;
    if (part < 0 || part >= r->num_parts) return EXR_ERROR_INVALID_ARGUMENT;
    p = &r->parts[part];
    h = &p->header;
    a = &r->alloc;
    if (h->part_type != EXR_PART_SCANLINE)
        return exr_reader_read_part(r, part, out); /* only scanline is partial */

    ymin = h->data_window.min_y;
    ymax = h->data_window.max_y;
    if (y_count <= 0 || y_start < ymin || y_start + y_count - 1 > ymax)
        return EXR_ERROR_INVALID_ARGUMENT;
    y_end = y_start + y_count - 1;

    memset(out, 0, sizeof(*out));
    rc = exr_header_copy(a, &out->header, h);
    if (!EXR_OK(rc)) return rc;
    out->width = p->width;
    out->height = y_count;

    out->images = (void **)exr_calloc(a, (size_t)h->num_channels, sizeof(void *));
    if (!out->images) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
    for (c = 0; c < h->num_channels; ++c) {
        int xs = h->channels[c].x_sampling, ys = h->channels[c].y_sampling;
        int cw = nsamp(h->data_window.min_x, h->data_window.max_x, xs);
        int ch = nsamp(y_start, y_end, ys);
        size_t n, bytes;
        if (cw < 0) cw = 0;
        if (ch < 0) ch = 0;
        if (exr_mul_ovf((size_t)cw, (size_t)ch, &n) ||
            exr_mul_ovf(n, exr_pixel_size(h->channels[c].pixel_type), &bytes)) {
            rc = EXR_ERROR_CORRUPT;
            goto fail;
        }
        out->images[c] = exr_calloc(a, bytes ? bytes : 1, 1);
        if (!out->images[c]) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
    }

    lpb = exr_lines_per_block(h->compression);
    first_blk = (y_start - ymin) / lpb;
    last_blk = (y_end - ymin) / lpb;
    for (blk = first_blk; blk <= last_blk; ++blk) {
        uint64_t off;
        const uint8_t *hdr, *cdata;
        size_t hdr_size = r->is_multipart ? 12 : 8;
        int32_t y0, data_size, nlines, line;
        size_t dst_size, want, boff;
        exr_codec_ctx ctx;

        if ((uint32_t)blk >= p->num_chunks) { rc = EXR_ERROR_CORRUPT; goto fail; }
        off = p->offsets[blk];
        rc = exr_reader_fetch(r, off, hdr_size, NULL, &hdr);
        if (!EXR_OK(rc)) goto fail;
        if (r->is_multipart) {
            if (exr_rd_i32(hdr) != part) { rc = EXR_ERROR_CORRUPT; goto fail; }
            hdr += 4;
        }
        y0 = exr_rd_i32(hdr);
        data_size = exr_rd_i32(hdr + 4);
        if (data_size < 0 || y0 < ymin || y0 > ymax) { rc = EXR_ERROR_CORRUPT; goto fail; }
        nlines = lpb;
        if (y0 + nlines - 1 > ymax) nlines = ymax - y0 + 1;

        rc = exr_block_uncompressed_size(h->channels, h->num_channels,
                                         h->data_window.min_x, y0, p->width,
                                         nlines, &dst_size);
        if (!EXR_OK(rc)) goto fail;
        if (exr_add_ovf((size_t)off, hdr_size, &want)) { rc = EXR_ERROR_CORRUPT; goto fail; }
        rc = exr_reader_fetch(r, want, (size_t)data_size, NULL, &cdata);
        if (!EXR_OK(rc)) goto fail;
        if (dst_size > block_cap) {
            exr_free(a, block);
            block = (uint8_t *)exr_malloc(a, dst_size ? dst_size : 1);
            if (!block) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
            block_cap = dst_size;
        }
        ctx.alloc = a;
        ctx.compression = h->compression;
        ctx.channels = h->channels;
        ctx.num_channels = h->num_channels;
        ctx.x = h->data_window.min_x;
        ctx.y = y0;
        ctx.width = p->width;
        ctx.num_lines = nlines;
        rc = exr_decompress_block(&ctx, cdata, (size_t)data_size, block, dst_size);
        if (!EXR_OK(rc)) goto fail;

        /* scatter only rows within [y_start, y_end] into the output */
        boff = 0;
        for (line = 0; line < nlines; ++line) {
            int yy = y0 + line;
            for (c = 0; c < h->num_channels; ++c) {
                int xs = h->channels[c].x_sampling, ys = h->channels[c].y_sampling;
                size_t ps = exr_pixel_size(h->channels[c].pixel_type);
                int nx, cw, row;
                size_t bytes;
                if ((yy % ys) != 0) continue;
                nx = nsamp(h->data_window.min_x, h->data_window.max_x, xs);
                if (nx <= 0) continue;
                cw = nx;
                bytes = (size_t)nx * ps;
                if (boff + bytes > dst_size) { rc = EXR_ERROR_CORRUPT; goto fail; }
                if (yy >= y_start && yy <= y_end) {
                    row = nsamp(y_start, yy, ys) - 1;
                    memcpy((uint8_t *)out->images[c] + (size_t)row * cw * ps,
                           block + boff, bytes);
                }
                boff += bytes;
            }
        }
    }
    exr_free(a, block);
    return EXR_SUCCESS;

fail:
    exr_free(a, block);
    exr_part_free(a, out); /* on error, out owns nothing */
    return rc;
}

/* Offset-table index of a tile (tx,ty) at level (lx,ly). */
static uint32_t tile_index(const exr_int_part *p, int tx, int ty, int lx, int ly) {
    uint32_t index = 0;
    int l, nxt, nyt, mode = p->header.level_mode;
    if (mode == EXR_TILE_MIPMAP_LEVELS) {
        for (l = 0; l < lx; ++l) {
            tiled_level_size(p, l, l, NULL, NULL, &nxt, &nyt);
            index += (uint32_t)(nxt * nyt);
        }
        tiled_level_size(p, lx, ly, NULL, NULL, &nxt, NULL);
        return index + (uint32_t)(ty * nxt + tx);
    }
    if (mode == EXR_TILE_RIPMAP_LEVELS) {
        int yy, xx;
        for (yy = 0; yy < ly; ++yy)
            for (xx = 0; xx < p->num_x_levels; ++xx) {
                tiled_level_size(p, xx, yy, NULL, NULL, &nxt, &nyt);
                index += (uint32_t)(nxt * nyt);
            }
        for (xx = 0; xx < lx; ++xx) {
            tiled_level_size(p, xx, ly, NULL, NULL, &nxt, &nyt);
            index += (uint32_t)(nxt * nyt);
        }
        tiled_level_size(p, lx, ly, NULL, NULL, &nxt, NULL);
        return index + (uint32_t)(ty * nxt + tx);
    }
    /* ONE_LEVEL */
    tiled_level_size(p, 0, 0, NULL, NULL, &nxt, NULL);
    return (uint32_t)(ty * nxt + tx);
}

exr_result exr_reader_read_tile(exr_reader *r, int32_t part, int32_t tile_x,
                                int32_t tile_y, int32_t level_x, int32_t level_y,
                                exr_part *out) {
    exr_int_part *p;
    const exr_header *h;
    const exr_allocator *a;
    int lw, lh, nxt, nyt, tw, th, abs_x0, abs_y0, c, line;
    int tx, ty;
    uint32_t idx;
    uint64_t off;
    const uint8_t *hdr, *cdata;
    size_t hdr_size, dst_size, want, boff;
    int32_t data_size;
    uint8_t *block = NULL;
    exr_codec_ctx ctx;
    exr_result rc;

    if (!r || !out) return EXR_ERROR_INVALID_ARGUMENT;
    rc = exr_reader_parse_header(r);
    if (rc != EXR_SUCCESS) return rc;
    if (part < 0 || part >= r->num_parts) return EXR_ERROR_INVALID_ARGUMENT;
    p = &r->parts[part];
    h = &p->header;
    a = &r->alloc;
    if (h->part_type != EXR_PART_TILED) return EXR_ERROR_INVALID_ARGUMENT;
    if (level_x < 0 || level_x >= p->num_x_levels || level_y < 0 ||
        level_y >= p->num_y_levels)
        return EXR_ERROR_INVALID_ARGUMENT;

    tiled_level_size(p, level_x, level_y, &lw, &lh, &nxt, &nyt);
    if (tile_x < 0 || tile_x >= nxt || tile_y < 0 || tile_y >= nyt)
        return EXR_ERROR_INVALID_ARGUMENT;
    tx = (int)h->tile_x_size;
    ty = (int)h->tile_y_size;
    tw = (tx < lw - tile_x * tx) ? tx : (lw - tile_x * tx);
    th = (ty < lh - tile_y * ty) ? ty : (lh - tile_y * ty);
    abs_x0 = h->data_window.min_x + tile_x * tx;
    abs_y0 = h->data_window.min_y + tile_y * ty;

    idx = tile_index(p, tile_x, tile_y, level_x, level_y);
    if (idx >= p->num_chunks) return EXR_ERROR_CORRUPT;
    off = p->offsets[idx];
    hdr_size = r->is_multipart ? 24 : 20;
    rc = exr_reader_fetch(r, off, hdr_size, NULL, &hdr);
    if (!EXR_OK(rc)) return rc;
    if (r->is_multipart) {
        if (exr_rd_i32(hdr) != part) return EXR_ERROR_CORRUPT;
        hdr += 4;
    }
    if (exr_rd_i32(hdr) != tile_x || exr_rd_i32(hdr + 4) != tile_y ||
        exr_rd_i32(hdr + 8) != level_x || exr_rd_i32(hdr + 12) != level_y)
        return EXR_ERROR_CORRUPT;
    data_size = exr_rd_i32(hdr + 16);
    if (data_size < 0) return EXR_ERROR_CORRUPT;

    memset(out, 0, sizeof(*out));
    rc = exr_header_copy(a, &out->header, h);
    if (!EXR_OK(rc)) return rc;
    out->width = tw;
    out->height = th;
    out->images = (void **)exr_calloc(a, (size_t)h->num_channels, sizeof(void *));
    if (!out->images) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
    for (c = 0; c < h->num_channels; ++c) {
        int xs = h->channels[c].x_sampling, ys = h->channels[c].y_sampling;
        int cw = nsamp(abs_x0, abs_x0 + tw - 1, xs);
        int ch = nsamp(abs_y0, abs_y0 + th - 1, ys);
        size_t n, bytes;
        if (cw < 0) cw = 0;
        if (ch < 0) ch = 0;
        if (exr_mul_ovf((size_t)cw, (size_t)ch, &n) ||
            exr_mul_ovf(n, exr_pixel_size(h->channels[c].pixel_type), &bytes)) {
            rc = EXR_ERROR_CORRUPT;
            goto fail;
        }
        out->images[c] = exr_calloc(a, bytes ? bytes : 1, 1);
        if (!out->images[c]) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
    }

    rc = exr_block_uncompressed_size(h->channels, h->num_channels, abs_x0, abs_y0,
                                     tw, th, &dst_size);
    if (!EXR_OK(rc)) goto fail;
    if (exr_add_ovf((size_t)off, hdr_size, &want)) { rc = EXR_ERROR_CORRUPT; goto fail; }
    rc = exr_reader_fetch(r, want, (size_t)data_size, NULL, &cdata);
    if (!EXR_OK(rc)) goto fail;
    block = (uint8_t *)exr_malloc(a, dst_size ? dst_size : 1);
    if (!block) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
    ctx.alloc = a;
    ctx.compression = h->compression;
    ctx.channels = h->channels;
    ctx.num_channels = h->num_channels;
    ctx.x = abs_x0;
    ctx.y = abs_y0;
    ctx.width = tw;
    ctx.num_lines = th;
    rc = exr_decompress_block(&ctx, cdata, (size_t)data_size, block, dst_size);
    if (!EXR_OK(rc)) goto fail;

    /* scatter the tile block into the tile-sized output (origin 0,0) */
    boff = 0;
    for (line = 0; line < th; ++line) {
        int yy = abs_y0 + line;
        for (c = 0; c < h->num_channels; ++c) {
            int xs = h->channels[c].x_sampling, ys = h->channels[c].y_sampling;
            size_t ps = exr_pixel_size(h->channels[c].pixel_type);
            int nx, cw, row;
            size_t bytes;
            if ((yy % ys) != 0) continue;
            nx = nsamp(abs_x0, abs_x0 + tw - 1, xs);
            if (nx <= 0) continue;
            cw = nx;
            row = nsamp(abs_y0, yy, ys) - 1;
            bytes = (size_t)nx * ps;
            if (boff + bytes > dst_size) { rc = EXR_ERROR_CORRUPT; goto fail; }
            memcpy((uint8_t *)out->images[c] + (size_t)row * cw * ps, block + boff,
                   bytes);
            boff += bytes;
        }
    }
    exr_free(a, block);
    return EXR_SUCCESS;

fail:
    exr_free(a, block);
    exr_part_free(a, out); /* on error, out owns nothing */
    return rc;
}

/* ============================================================================
 * Streaming block I/O (bounded working memory)
 * ========================================================================== */

/* Inverse of tile_index(): map a tiled chunk `idx` to its (level,tile) coords
 * by walking the offset table in the same order tile_index() lays it out. */
static exr_result block_locate(const exr_int_part *p, uint32_t idx, int *out_lx,
                               int *out_ly, int *out_tx, int *out_ty) {
    int mode = p->header.level_mode;
    uint32_t base = 0;
    int nxt, nyt;
    if (mode == EXR_TILE_MIPMAP_LEVELS) {
        int l;
        for (l = 0; l < p->num_x_levels; ++l) {
            uint32_t cnt;
            tiled_level_size(p, l, l, NULL, NULL, &nxt, &nyt);
            cnt = (uint32_t)nxt * (uint32_t)nyt;
            if (idx < base + cnt) {
                uint32_t w = idx - base;
                *out_lx = l; *out_ly = l;
                *out_tx = (int)(w % (uint32_t)nxt);
                *out_ty = (int)(w / (uint32_t)nxt);
                return EXR_SUCCESS;
            }
            base += cnt;
        }
    } else if (mode == EXR_TILE_RIPMAP_LEVELS) {
        int lx, ly;
        for (ly = 0; ly < p->num_y_levels; ++ly)
            for (lx = 0; lx < p->num_x_levels; ++lx) {
                uint32_t cnt;
                tiled_level_size(p, lx, ly, NULL, NULL, &nxt, &nyt);
                cnt = (uint32_t)nxt * (uint32_t)nyt;
                if (idx < base + cnt) {
                    uint32_t w = idx - base;
                    *out_lx = lx; *out_ly = ly;
                    *out_tx = (int)(w % (uint32_t)nxt);
                    *out_ty = (int)(w / (uint32_t)nxt);
                    return EXR_SUCCESS;
                }
                base += cnt;
            }
    } else { /* ONE_LEVEL */
        tiled_level_size(p, 0, 0, NULL, NULL, &nxt, &nyt);
        if (nxt <= 0) return EXR_ERROR_CORRUPT;
        *out_lx = 0; *out_ly = 0;
        *out_tx = (int)(idx % (uint32_t)nxt);
        *out_ty = (int)(idx / (uint32_t)nxt);
        return EXR_SUCCESS;
    }
    return EXR_ERROR_CORRUPT;
}

/* Block geometry for chunk `idx` (no pixel I/O). */
static exr_result block_geometry(const exr_int_part *p, uint32_t idx,
                                 exr_block_info *bi) {
    const exr_header *h = &p->header;
    int deep = (h->part_type == EXR_PART_DEEP_SCANLINE ||
                h->part_type == EXR_PART_DEEP_TILED);
    memset(bi, 0, sizeof(*bi));
    bi->is_deep = (uint8_t)deep;
    if (idx >= p->num_chunks) return EXR_ERROR_INVALID_ARGUMENT;
    if (h->tiled) {
        int lx, ly, tx, ty, lw, lh, nxt, nyt, tsx, tsy, x0l, y0l, tw, th;
        exr_result rc = block_locate(p, idx, &lx, &ly, &tx, &ty);
        if (!EXR_OK(rc)) return rc;
        tiled_level_size(p, lx, ly, &lw, &lh, &nxt, &nyt);
        tsx = (int)h->tile_x_size;
        tsy = (int)h->tile_y_size;
        if (tsx <= 0 || tsy <= 0) return EXR_ERROR_CORRUPT;
        x0l = tx * tsx;
        y0l = ty * tsy;
        tw = (tsx < lw - x0l) ? tsx : (lw - x0l);
        th = (tsy < lh - y0l) ? tsy : (lh - y0l);
        if (tw <= 0 || th <= 0) return EXR_ERROR_CORRUPT;
        bi->is_tiled = 1;
        bi->tile_x = tx; bi->tile_y = ty;
        bi->level_x = lx; bi->level_y = ly;
        bi->x0 = h->data_window.min_x + x0l;
        bi->y0 = h->data_window.min_y + y0l;
        bi->width = tw;
        bi->height = th;
    } else {
        int lpb = exr_lines_per_block(h->compression);
        int ymin = h->data_window.min_y, ymax = h->data_window.max_y;
        /* compute in 64-bit: idx*lpb can exceed INT_MAX for a crafted file with
         * a huge chunkCount, which would be signed-overflow UB in int. */
        int64_t y0_64 = (int64_t)ymin + (int64_t)idx * lpb;
        int y0, nlines;
        if (y0_64 > ymax) return EXR_ERROR_CORRUPT;
        y0 = (int)y0_64;
        nlines = lpb;
        if (y0 + nlines - 1 > ymax) nlines = ymax - y0 + 1;
        bi->x0 = h->data_window.min_x;
        bi->y0 = y0;
        bi->width = p->width;
        bi->height = nlines;
    }
    if (!deep) {
        exr_result rc = exr_block_uncompressed_size(
            h->channels, h->num_channels, bi->x0, bi->y0, bi->width, bi->height,
            &bi->uncompressed_size);
        if (!EXR_OK(rc)) return rc;
    }
    return EXR_SUCCESS;
}

exr_result exr_reader_num_blocks(exr_reader *r, int32_t part, uint32_t *out) {
    exr_result rc;
    if (!r || !out) return EXR_ERROR_INVALID_ARGUMENT;
    rc = exr_reader_parse_header(r);
    if (rc != EXR_SUCCESS) return rc;
    if (part < 0 || part >= r->num_parts) return EXR_ERROR_INVALID_ARGUMENT;
    *out = r->parts[part].num_chunks;
    return EXR_SUCCESS;
}

exr_result exr_reader_block_info(exr_reader *r, int32_t part, uint32_t idx,
                                 exr_block_info *out) {
    exr_result rc;
    if (!r || !out) return EXR_ERROR_INVALID_ARGUMENT;
    rc = exr_reader_parse_header(r);
    if (rc != EXR_SUCCESS) return rc;
    if (part < 0 || part >= r->num_parts) return EXR_ERROR_INVALID_ARGUMENT;
    rc = block_geometry(&r->parts[part], idx, out);
    out->part = part;
    return rc;
}

/* Internal: locate + fetch the raw compressed chunk bytes for block `idx`
 * (zero-copy for the memory reader; the returned pointer is valid until the next
 * reader fetch) and fill its codec context, WITHOUT decompressing. The GPU
 * backend uses this to run the reconstruction passes (predictor/deinterleave)
 * on the device for ZIP/ZIPS/RLE. May return EXR_WOULD_BLOCK in streaming mode. */
exr_result exr_reader_block_raw(exr_reader *r, int32_t part, uint32_t idx,
                                exr_block_info *out_bi, const uint8_t **out_cdata,
                                size_t *out_csize, exr_codec_ctx *out_ctx) {
    exr_int_part *p;
    const exr_header *h;
    const exr_allocator *a;
    exr_block_info bi;
    exr_result rc;
    uint64_t off;
    const uint8_t *hdr, *cdata;
    size_t hdr_size, want;
    int32_t data_size;
    exr_codec_ctx ctx;

    if (!r || !out_bi || !out_cdata || !out_csize || !out_ctx)
        return EXR_ERROR_INVALID_ARGUMENT;
    rc = exr_reader_parse_header(r);
    if (rc != EXR_SUCCESS) return rc;
    if (part < 0 || part >= r->num_parts) return EXR_ERROR_INVALID_ARGUMENT;
    p = &r->parts[part];
    h = &p->header;
    a = &r->alloc;
    if (h->part_type == EXR_PART_DEEP_SCANLINE ||
        h->part_type == EXR_PART_DEEP_TILED)
        return EXR_ERROR_INVALID_ARGUMENT; /* use the deep block API */
    rc = block_geometry(p, idx, &bi);
    if (!EXR_OK(rc)) return rc;

    off = p->offsets[idx];
    if (h->tiled) {
        hdr_size = r->is_multipart ? 24 : 20;
        rc = exr_reader_fetch(r, off, hdr_size, NULL, &hdr);
        if (!EXR_OK(rc)) return rc;
        if (r->is_multipart) {
            if (exr_rd_i32(hdr) != part) return EXR_ERROR_CORRUPT;
            hdr += 4;
        }
        if (exr_rd_i32(hdr) != bi.tile_x || exr_rd_i32(hdr + 4) != bi.tile_y ||
            exr_rd_i32(hdr + 8) != bi.level_x ||
            exr_rd_i32(hdr + 12) != bi.level_y)
            return EXR_ERROR_CORRUPT;
        data_size = exr_rd_i32(hdr + 16);
    } else {
        hdr_size = r->is_multipart ? 12 : 8;
        rc = exr_reader_fetch(r, off, hdr_size, NULL, &hdr);
        if (!EXR_OK(rc)) return rc;
        if (r->is_multipart) {
            if (exr_rd_i32(hdr) != part) return EXR_ERROR_CORRUPT;
            hdr += 4;
        }
        if (exr_rd_i32(hdr) != bi.y0) return EXR_ERROR_CORRUPT;
        data_size = exr_rd_i32(hdr + 4);
    }
    if (data_size < 0) return EXR_ERROR_CORRUPT;
    if (exr_add_ovf((size_t)off, hdr_size, &want)) return EXR_ERROR_CORRUPT;
    rc = exr_reader_fetch(r, want, (size_t)data_size, NULL, &cdata);
    if (!EXR_OK(rc)) return rc;

    ctx.alloc = a;
    ctx.compression = h->compression;
    ctx.channels = h->channels;
    ctx.num_channels = h->num_channels;
    ctx.x = bi.x0;
    ctx.y = bi.y0;
    ctx.width = bi.width;
    ctx.num_lines = bi.height;

    *out_bi = bi;
    *out_cdata = cdata;
    *out_csize = (size_t)data_size;
    *out_ctx = ctx;
    return EXR_SUCCESS;
}

exr_result exr_reader_decode_block(exr_reader *r, int32_t part, uint32_t idx,
                                   void *dst, size_t dst_size) {
    exr_block_info bi;
    const uint8_t *cdata;
    size_t csize;
    exr_codec_ctx ctx;
    exr_result rc;

    if (!r || !dst) return EXR_ERROR_INVALID_ARGUMENT;
    rc = exr_reader_block_raw(r, part, idx, &bi, &cdata, &csize, &ctx);
    if (rc != EXR_SUCCESS) return rc; /* incl. EXR_WOULD_BLOCK (which is EXR_OK) */
    if (dst_size < bi.uncompressed_size) return EXR_ERROR_INVALID_ARGUMENT;
    return exr_decompress_block(&ctx, cdata, csize, (uint8_t *)dst,
                                bi.uncompressed_size);
}

exr_result exr_block_extract_channel(const exr_header *h,
                                     const exr_block_info *bi, const void *block,
                                     size_t block_size, int32_t channel,
                                     void *dst) {
    const uint8_t *b = (const uint8_t *)block;
    size_t off = 0;
    int line, c, x0, y0, w, hgt;
    if (!h || !bi || !block || !dst) return EXR_ERROR_INVALID_ARGUMENT;
    if (channel < 0 || channel >= h->num_channels)
        return EXR_ERROR_INVALID_ARGUMENT;
    x0 = bi->x0; y0 = bi->y0; w = bi->width; hgt = bi->height;
    for (line = 0; line < hgt; ++line) {
        int yy = y0 + line;
        for (c = 0; c < h->num_channels; ++c) {
            int xs = h->channels[c].x_sampling, ys = h->channels[c].y_sampling;
            size_t ps = exr_pixel_size(h->channels[c].pixel_type);
            int nx, row;
            size_t bytes;
            if (xs < 1) xs = 1;
            if (ys < 1) ys = 1;
            if ((yy % ys) != 0) continue;
            nx = nsamp(x0, x0 + w - 1, xs);
            if (nx <= 0) continue;
            bytes = (size_t)nx * ps;
            if (off + bytes > block_size) return EXR_ERROR_CORRUPT;
            if (c == channel) {
                row = nsamp(y0, yy, ys) - 1;
                memcpy((uint8_t *)dst + (size_t)row * (size_t)nx * ps, b + off,
                       bytes);
            }
            off += bytes;
        }
    }
    return EXR_SUCCESS;
}

exr_result exr_reader_decode_deep_counts(exr_reader *r, int32_t part,
                                         uint32_t idx, int32_t *counts) {
    exr_int_part *p;
    exr_block_info bi;
    exr_result rc;
    if (!r || !counts) return EXR_ERROR_INVALID_ARGUMENT;
    rc = exr_reader_parse_header(r);
    if (rc != EXR_SUCCESS) return rc;
    if (part < 0 || part >= r->num_parts) return EXR_ERROR_INVALID_ARGUMENT;
    p = &r->parts[part];
    if (p->header.part_type != EXR_PART_DEEP_SCANLINE &&
        p->header.part_type != EXR_PART_DEEP_TILED)
        return EXR_ERROR_INVALID_ARGUMENT;
    rc = block_geometry(p, idx, &bi);
    if (!EXR_OK(rc)) return rc;
    return exr_deep_decode_counts(r, p, part, idx, bi.width, bi.height,
                                  bi.is_tiled, counts);
}

exr_result exr_reader_decode_deep_samples(exr_reader *r, int32_t part,
                                          uint32_t idx, void *const *chan_dst) {
    exr_int_part *p;
    exr_block_info bi;
    exr_result rc;
    if (!r || !chan_dst) return EXR_ERROR_INVALID_ARGUMENT;
    rc = exr_reader_parse_header(r);
    if (rc != EXR_SUCCESS) return rc;
    if (part < 0 || part >= r->num_parts) return EXR_ERROR_INVALID_ARGUMENT;
    p = &r->parts[part];
    if (p->header.part_type != EXR_PART_DEEP_SCANLINE &&
        p->header.part_type != EXR_PART_DEEP_TILED)
        return EXR_ERROR_INVALID_ARGUMENT;
    rc = block_geometry(p, idx, &bi);
    if (!EXR_OK(rc)) return rc;
    return exr_deep_decode_samples(r, p, part, idx, bi.width, bi.height,
                                   bi.is_tiled, chan_dst);
}

/* ============================================================================
 * Streaming suspend/resume. With an EXR_SRC_CALLBACK source, any reader call
 * (parse_header / decode_block / decode_deep_*) runs exr_reader_ensure_buffered
 * first; when the source has no bytes ready it returns EXR_WOULD_BLOCK and
 * records the pending byte range. The host fetches that range, feeds it via
 * exr_reader_supply(), and re-issues the same call to resume.
 * ========================================================================== */

exr_result exr_reader_pending(const exr_reader *r, exr_pending_read *out) {
    if (!r || !out) return EXR_ERROR_INVALID_ARGUMENT;
    if (!r->have_pending) return EXR_ERROR_INVALID_ARGUMENT;
    *out = r->pending;
    return EXR_SUCCESS;
}

exr_result exr_reader_supply(exr_reader *r, const void *data, size_t size) {
    if (!r || (!data && size)) return EXR_ERROR_INVALID_ARGUMENT;
    if (r->kind != EXR_SRC_CALLBACK) return EXR_ERROR_INVALID_ARGUMENT;
    /* Append the fetched bytes to the streaming buffer (host may feed the
     * pending range in pieces). Then re-call the reader function to resume. */
    if (size > r->mem_size - r->filled) return EXR_ERROR_INVALID_ARGUMENT;
    memcpy((uint8_t *)r->mem + r->filled, data, size);
    r->filled += size;
    r->have_pending = 0;
    return EXR_SUCCESS;
}
