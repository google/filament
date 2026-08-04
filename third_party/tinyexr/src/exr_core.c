/*
 * TinyEXR - core: allocator helpers, image lifetime, high-level load/save glue.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

#ifndef EXR_FREESTANDING
#include <stdlib.h> /* malloc/free for the default allocator only */
#endif

/* ============================================================================
 * Result strings
 * ========================================================================== */

const char *exr_result_string(exr_result r) {
    switch (r) {
    case EXR_SUCCESS: return "success";
    case EXR_WOULD_BLOCK: return "would block (streaming)";
    case EXR_ERROR_INVALID_ARGUMENT: return "invalid argument";
    case EXR_ERROR_INVALID_FILE: return "invalid or unsupported EXR file";
    case EXR_ERROR_UNSUPPORTED: return "unsupported feature";
    case EXR_ERROR_OUT_OF_MEMORY: return "out of memory";
    case EXR_ERROR_IO: return "I/O error";
    case EXR_ERROR_CORRUPT: return "corrupt or malformed data";
    }
    return "unknown error";
}

/* ============================================================================
 * Allocator
 * ========================================================================== */

#ifndef EXR_FREESTANDING
static void *default_alloc(void *user, size_t size) {
    (void)user;
    return malloc(size);
}
static void default_free(void *user, void *ptr) {
    (void)user;
    free(ptr);
}
static const exr_allocator g_default_allocator = {NULL, default_alloc,
                                                  default_free};

const exr_allocator *exr_default_allocator(void) { return &g_default_allocator; }
#else
/* Freestanding: no libc allocator. Callers MUST supply an exr_allocator;
 * passing NULL degrades to a clean out-of-memory failure (see exr_malloc). */
const exr_allocator *exr_default_allocator(void) { return NULL; }
#endif

void *exr_malloc(const exr_allocator *a, size_t size) {
    if (!a || !a->alloc) return NULL;
    if (size == 0) size = 1;
    return a->alloc(a->user, size);
}

void *exr_calloc(const exr_allocator *a, size_t count, size_t size) {
    size_t total;
    void *p;
    if (!a || !a->alloc) return NULL;
    if (exr_mul_ovf(count, size, &total)) return NULL;
    if (total == 0) total = 1;
    p = a->alloc(a->user, total);
    if (p) memset(p, 0, total);
    return p;
}

void exr_free(const exr_allocator *a, void *ptr) {
    if (a && a->free && ptr) a->free(a->user, ptr);
}

char *exr_strdup(const exr_allocator *a, const char *s) {
    size_t n;
    char *p;
    if (!a || !a->alloc) return NULL;
    if (!s) s = "";
    n = strlen(s) + 1;
    p = (char *)a->alloc(a->user, n);
    if (p) memcpy(p, s, n);
    return p;
}

/* ============================================================================
 * Compression block geometry
 * ========================================================================== */

int exr_lines_per_block(exr_compression c) {
    switch (c) {
    case EXR_COMPRESSION_NONE:
    case EXR_COMPRESSION_RLE:
    case EXR_COMPRESSION_ZIPS: return 1;
    case EXR_COMPRESSION_ZIP:
    case EXR_COMPRESSION_PXR24: return 16;
    case EXR_COMPRESSION_PIZ:
    case EXR_COMPRESSION_B44:
    case EXR_COMPRESSION_B44A:
    case EXR_COMPRESSION_HTJ2K32:
    case EXR_COMPRESSION_ZSTD: return 32;
    case EXR_COMPRESSION_DWAA: return 32;
    case EXR_COMPRESSION_HTJ2K256:
    case EXR_COMPRESSION_DWAB: return 256;
    }
    return 1;
}

/* Floor division for sample-count math with possibly-negative window origins. */
static int64_t floordiv(int64_t a, int64_t b) {
    int64_t q = a / b, r = a % b;
    if ((r != 0) && ((r < 0) != (b < 0))) q--;
    return q;
}

/* Number of samples in [lo, hi] (inclusive) at the given sampling rate. */
static int num_samples(int lo, int hi, int sampling) {
    if (sampling <= 1) return hi - lo + 1;
    return (int)(floordiv(hi, sampling) - floordiv((int64_t)lo - 1, sampling));
}

exr_result exr_block_uncompressed_size(const exr_channel *channels,
                                       int32_t num_channels, int32_t x,
                                       int32_t y, int32_t width,
                                       int32_t num_lines, size_t *out_size) {
    size_t total = 0;
    int32_t line, c;
    if (width <= 0 || num_lines <= 0) {
        *out_size = 0;
        return EXR_SUCCESS;
    }
    for (line = 0; line < num_lines; ++line) {
        int32_t yy = y + line;
        for (c = 0; c < num_channels; ++c) {
            const exr_channel *ch = &channels[c];
            int32_t ys = ch->y_sampling <= 0 ? 1 : ch->y_sampling;
            int32_t xs = ch->x_sampling <= 0 ? 1 : ch->x_sampling;
            int nx;
            size_t row;
            if ((yy % ys) != 0) continue;
            nx = num_samples(x, x + width - 1, xs);
            if (nx < 0) nx = 0;
            if (exr_mul_ovf((size_t)nx, exr_pixel_size(ch->pixel_type), &row))
                return EXR_ERROR_CORRUPT;
            if (exr_add_ovf(total, row, &total)) return EXR_ERROR_CORRUPT;
        }
    }
    *out_size = total;
    return EXR_SUCCESS;
}

/* ============================================================================
 * Attribute list / header lifetime
 * ========================================================================== */

const exr_attr *exr_attr_find(const exr_attr_list *list, const char *name) {
    uint32_t i;
    if (!list) return NULL;
    for (i = 0; i < list->count; ++i) {
        if (strcmp(list->items[i].name, name) == 0) return &list->items[i];
    }
    return NULL;
}

void exr_attr_list_free(const exr_allocator *a, exr_attr_list *list) {
    uint32_t i;
    if (!list) return;
    for (i = 0; i < list->count; ++i) {
        exr_free(a, list->items[i].name);
        exr_free(a, list->items[i].type_name);
        exr_free(a, list->items[i].data);
    }
    exr_free(a, list->items);
    exr_free(a, list);
}

static exr_attr_list *attr_list_copy(const exr_allocator *a,
                                     const exr_attr_list *src) {
    exr_attr_list *dst;
    uint32_t i;
    if (!src) return NULL;
    dst = (exr_attr_list *)exr_calloc(a, 1, sizeof(*dst));
    if (!dst) return NULL;
    dst->items = (exr_attr *)exr_calloc(a, src->count ? src->count : 1,
                                        sizeof(exr_attr));
    if (!dst->items) {
        exr_free(a, dst);
        return NULL;
    }
    dst->capacity = src->count;
    for (i = 0; i < src->count; ++i) {
        const exr_attr *s = &src->items[i];
        exr_attr *d = &dst->items[i];
        d->name = exr_strdup(a, s->name);
        d->type_name = exr_strdup(a, s->type_name);
        d->size = s->size;
        d->data = (uint8_t *)exr_malloc(a, s->size ? s->size : 1);
        if (!d->name || !d->type_name || !d->data) {
            dst->count = i + 1;
            exr_attr_list_free(a, dst);
            return NULL;
        }
        if (s->size) memcpy(d->data, s->data, s->size);
    }
    dst->count = src->count;
    return dst;
}

exr_result exr_header_copy(const exr_allocator *a, exr_header *dst,
                           const exr_header *src) {
    *dst = *src;
    dst->channels = NULL;
    dst->attrs = NULL;
    if (src->num_channels > 0 && src->channels) {
        dst->channels = (exr_channel *)exr_calloc(a, (size_t)src->num_channels,
                                                  sizeof(exr_channel));
        if (!dst->channels) return EXR_ERROR_OUT_OF_MEMORY;
        memcpy(dst->channels, src->channels,
               (size_t)src->num_channels * sizeof(exr_channel));
    }
    if (src->attrs) {
        dst->attrs = attr_list_copy(a, src->attrs);
        if (!dst->attrs) {
            exr_free(a, dst->channels);
            dst->channels = NULL;
            return EXR_ERROR_OUT_OF_MEMORY;
        }
    }
    return EXR_SUCCESS;
}

void exr_header_free(const exr_allocator *a, exr_header *hdr) {
    if (!hdr) return;
    exr_free(a, hdr->channels);
    hdr->channels = NULL;
    if (hdr->attrs) {
        exr_attr_list_free(a, hdr->attrs);
        hdr->attrs = NULL;
    }
}

/* ============================================================================
 * Public custom-attribute access
 * ========================================================================== */

static void attr_view(const exr_attr *a, exr_attribute *out) {
    out->name = a->name;
    out->type_name = a->type_name;
    out->data = a->data;
    out->size = a->size;
}

int32_t exr_header_num_attributes(const exr_header *hdr) {
    if (!hdr || !hdr->attrs) return 0;
    return (int32_t)hdr->attrs->count;
}

exr_result exr_header_get_attribute(const exr_header *hdr, int32_t index,
                                    exr_attribute *out) {
    if (!hdr || !out || !hdr->attrs) return EXR_ERROR_INVALID_ARGUMENT;
    if (index < 0 || (uint32_t)index >= hdr->attrs->count)
        return EXR_ERROR_INVALID_ARGUMENT;
    attr_view(&hdr->attrs->items[index], out);
    return EXR_SUCCESS;
}

exr_result exr_header_find_attribute(const exr_header *hdr, const char *name,
                                     exr_attribute *out) {
    const exr_attr *a;
    if (!hdr || !name || !out) return EXR_ERROR_INVALID_ARGUMENT;
    a = exr_attr_find(hdr->attrs, name);
    if (!a) return EXR_ERROR_INVALID_ARGUMENT;
    attr_view(a, out);
    return EXR_SUCCESS;
}

exr_result exr_header_get_string_attribute(const exr_header *hdr,
                                           const char *name, char *buf,
                                           size_t buf_size, size_t *out_len) {
    const exr_attr *a;
    if (!hdr || !name) return EXR_ERROR_INVALID_ARGUMENT;
    a = exr_attr_find(hdr->attrs, name);
    if (!a) return EXR_ERROR_INVALID_ARGUMENT;
    if (out_len) *out_len = a->size;
    if (buf && buf_size > 0) {
        size_t n = (a->size < buf_size - 1) ? a->size : buf_size - 1;
        if (n) memcpy(buf, a->data, n);
        buf[n] = '\0';
    }
    return EXR_SUCCESS;
}

exr_result exr_header_set_attribute(const exr_allocator *alloc, exr_header *hdr,
                                    const char *name, const char *type_name,
                                    const void *data, uint32_t size) {
    const exr_allocator *a = alloc ? alloc : exr_default_allocator();
    exr_attr_list *list;
    uint32_t i;
    if (!hdr || !name || !type_name || (size && !data))
        return EXR_ERROR_INVALID_ARGUMENT;

    if (!hdr->attrs) {
        hdr->attrs = (exr_attr_list *)exr_calloc(a, 1, sizeof(*hdr->attrs));
        if (!hdr->attrs) return EXR_ERROR_OUT_OF_MEMORY;
    }
    list = hdr->attrs;

    /* Replace in place if an attribute of this name already exists. */
    for (i = 0; i < list->count; ++i) {
        if (strcmp(list->items[i].name, name) == 0) {
            size_t tn = strlen(type_name);
            char *nt = (char *)exr_malloc(a, tn + 1);
            uint8_t *nd = (uint8_t *)exr_malloc(a, size ? size : 1);
            if (!nt || !nd) {
                exr_free(a, nt);
                exr_free(a, nd);
                return EXR_ERROR_OUT_OF_MEMORY;
            }
            memcpy(nt, type_name, tn + 1);
            if (size) memcpy(nd, data, size);
            exr_free(a, list->items[i].type_name);
            exr_free(a, list->items[i].data);
            list->items[i].type_name = nt;
            list->items[i].data = nd;
            list->items[i].size = size;
            return EXR_SUCCESS;
        }
    }
    return exr_attr_list_append(a, list, name, strlen(name), type_name,
                                strlen(type_name), (const uint8_t *)data, size);
}

exr_result exr_header_set_string_attribute(const exr_allocator *alloc,
                                           exr_header *hdr, const char *name,
                                           const char *value) {
    if (!value) return EXR_ERROR_INVALID_ARGUMENT;
    return exr_header_set_attribute(alloc, hdr, name, "string", value,
                                    (uint32_t)strlen(value));
}

/* ============================================================================
 * Image lifetime
 * ========================================================================== */

void exr_part_free(const exr_allocator *a, exr_part *p) {
    int32_t c, nch;
    if (!p) return;
    if (!a) a = exr_default_allocator();
    nch = p->header.num_channels;
    if (p->images) {
        for (c = 0; c < nch; ++c) exr_free(a, p->images[c]);
        exr_free(a, p->images);
    }
    if (p->deep_images) {
        for (c = 0; c < nch; ++c) exr_free(a, p->deep_images[c]);
        exr_free(a, p->deep_images);
    }
    exr_free(a, p->deep_sample_counts);
    exr_header_free(a, &p->header);
    memset(p, 0, sizeof(*p));
}

void exr_image_free(exr_image *img) {
    int32_t i;
    const exr_allocator *a;
    if (!img || !img->parts) {
        if (img) memset(img, 0, sizeof(*img));
        return;
    }
    a = &img->alloc;
    for (i = 0; i < img->num_parts; ++i) exr_part_free(a, &img->parts[i]);
    exr_free(a, img->parts);
    memset(img, 0, sizeof(*img));
}

/* ============================================================================
 * High-level load
 * ========================================================================== */

int exr_is_exr_memory(const void *data, size_t size) {
    if (!data || size < 4) return 0;
    return exr_rd_u32((const uint8_t *)data) == EXR_MAGIC;
}

static exr_result load_all_parts(exr_reader *r, const exr_allocator *a,
                                 exr_image *out) {
    int32_t np, i;
    exr_result rc;

    rc = exr_reader_parse_header(r);
    if (!EXR_OK(rc)) return rc;
    if (rc == EXR_WOULD_BLOCK) return EXR_ERROR_UNSUPPORTED;

    np = exr_reader_num_parts(r);
    out->num_parts = np;
    out->alloc = *a;
    out->parts = (exr_part *)exr_calloc(a, np ? (size_t)np : 1, sizeof(exr_part));
    if (!out->parts) return EXR_ERROR_OUT_OF_MEMORY;

    for (i = 0; i < np; ++i) {
        rc = exr_reader_read_part(r, i, &out->parts[i]);
        if (!EXR_OK(rc)) return rc;
        if (rc == EXR_WOULD_BLOCK) return EXR_ERROR_UNSUPPORTED;
    }
    return EXR_SUCCESS;
}

exr_result exr_load_from_memory(const void *data, size_t size,
                                const exr_allocator *alloc, exr_image *out) {
    exr_reader *r = NULL;
    exr_result rc;
    if (!data || !out) return EXR_ERROR_INVALID_ARGUMENT;
    if (!alloc) alloc = exr_default_allocator();
    memset(out, 0, sizeof(*out));

    rc = exr_reader_open_memory(data, size, alloc, &r);
    if (!EXR_OK(rc)) return rc;

    rc = load_all_parts(r, alloc, out);
    if (!EXR_OK(rc)) exr_image_free(out);
    exr_reader_close(r);
    return rc;
}

/* exr_load_from_file lives in src/exr_stdio.c (the only stdio translation unit). */
