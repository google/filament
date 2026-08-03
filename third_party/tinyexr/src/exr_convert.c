/*
 * TinyEXR - pixel-format conversion helpers + exr_part <-> interleaved-float
 * bridges (util module).
 *
 * Canonical working format is float32. half<->float reuses the existing F16C
 * dispatch; integer<->float goes through the util SIMD kernels (scalar
 * reference here is the source of truth). Everything is libm-free.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

/* Round-to-nearest-even of a value already clamped to [0, ~2^22] via the
 * add/sub-magic trick (uses the default IEEE rounding mode, no libm). Matches
 * the hardware _mm_cvtps_epi32 / vcvtnq used by the SIMD kernels. */
#define EXR_ROUND_MAGIC 12582912.0f /* 2^23 + 2^22 */

/* ============================================================================
 * Scalar reference kernels (vtbl defaults)
 * ========================================================================== */

void exr_u8_to_f32_scalar(float *dst, const uint8_t *src, size_t n, float scale) {
    size_t i;
    for (i = 0; i < n; ++i) dst[i] = (float)src[i] * scale;
}

void exr_u16_to_f32_scalar(float *dst, const uint16_t *src, size_t n,
                           float scale) {
    size_t i;
    for (i = 0; i < n; ++i) dst[i] = (float)src[i] * scale;
}

void exr_f32_to_u8_scalar(uint8_t *dst, const float *src, size_t n, float scale) {
    size_t i;
    for (i = 0; i < n; ++i) {
        float v = src[i] * scale;
        if (!(v > 0.0f)) v = 0.0f; /* clamps negatives and NaN to 0 */
        if (v > 255.0f) v = 255.0f;
        v = (v + EXR_ROUND_MAGIC) - EXR_ROUND_MAGIC; /* round half to even */
        dst[i] = (uint8_t)v;
    }
}

void exr_f32_to_u16_scalar(uint16_t *dst, const float *src, size_t n,
                           float scale) {
    size_t i;
    for (i = 0; i < n; ++i) {
        float v = src[i] * scale;
        if (!(v > 0.0f)) v = 0.0f;
        if (v > 65535.0f) v = 65535.0f;
        v = (v + EXR_ROUND_MAGIC) - EXR_ROUND_MAGIC;
        dst[i] = (uint16_t)v;
    }
}

/* ============================================================================
 * Public integer<->float helpers (runtime SIMD-dispatched)
 * ========================================================================== */

void exr_u8_to_float(const uint8_t *src, float *dst, size_t n, int normalized) {
    exr_simd_init();
    exr_simd.u8_to_f32(dst, src, n, normalized ? (1.0f / 255.0f) : 1.0f);
}
void exr_u16_to_float(const uint16_t *src, float *dst, size_t n, int normalized) {
    exr_simd_init();
    exr_simd.u16_to_f32(dst, src, n, normalized ? (1.0f / 65535.0f) : 1.0f);
}
void exr_float_to_u8(const float *src, uint8_t *dst, size_t n, int normalized) {
    exr_simd_init();
    exr_simd.f32_to_u8(dst, src, n, normalized ? 255.0f : 1.0f);
}
void exr_float_to_u16(const float *src, uint16_t *dst, size_t n, int normalized) {
    exr_simd_init();
    exr_simd.f32_to_u16(dst, src, n, normalized ? 65535.0f : 1.0f);
}

/* ============================================================================
 * uint32 helpers (kept scalar: normalized needs double precision and the
 * float->uint32 convert does not vectorize cleanly past 2^31)
 * ========================================================================== */

static void u32_to_float(const uint32_t *src, float *dst, size_t n, int norm) {
    size_t i;
    if (norm) {
        const double inv = 1.0 / 4294967295.0;
        for (i = 0; i < n; ++i) dst[i] = (float)((double)src[i] * inv);
    } else {
        for (i = 0; i < n; ++i) dst[i] = (float)src[i]; /* lossy > 2^24 */
    }
}

static void float_to_u32(const float *src, uint32_t *dst, size_t n, int norm) {
    size_t i;
    const double scale = norm ? 4294967295.0 : 1.0;
    for (i = 0; i < n; ++i) {
        double v = (double)src[i] * scale;
        if (!(v > 0.0)) v = 0.0; /* negatives + NaN -> 0 */
        if (v > 4294967295.0) v = 4294967295.0;
        /* round half to even in double, then truncate */
        {
            double f = v - (double)(uint32_t)v; /* fractional part [0,1) */
            uint32_t u = (uint32_t)v;
            if (f > 0.5 || (f == 0.5 && (u & 1u))) u += 1u;
            dst[i] = u;
        }
    }
}

/* ============================================================================
 * Generic any-to-any conversion
 * ========================================================================== */

exr_result exr_convert_pixels(void *dst, exr_pixel_type dst_type,
                              const void *src, exr_pixel_type src_type,
                              size_t count, exr_convert_mode mode) {
    int norm = (mode == EXR_CONVERT_NORMALIZED);
    if (!dst || !src) return EXR_ERROR_INVALID_ARGUMENT;
    if (count == 0) return EXR_SUCCESS;

    if (src_type == dst_type) {
        memcpy(dst, src, count * exr_pixel_size(src_type));
        return EXR_SUCCESS;
    }

    /* Fast direct paths that avoid a float scratch buffer. */
    if (src_type == EXR_PIXEL_HALF && dst_type == EXR_PIXEL_FLOAT) {
        exr_half_to_float((const uint16_t *)src, (float *)dst, count);
        return EXR_SUCCESS;
    }
    if (src_type == EXR_PIXEL_FLOAT && dst_type == EXR_PIXEL_HALF) {
        exr_float_to_half((const float *)src, (uint16_t *)dst, count);
        return EXR_SUCCESS;
    }
    if (src_type == EXR_PIXEL_UINT && dst_type == EXR_PIXEL_FLOAT) {
        u32_to_float((const uint32_t *)src, (float *)dst, count, norm);
        return EXR_SUCCESS;
    }
    if (src_type == EXR_PIXEL_FLOAT && dst_type == EXR_PIXEL_UINT) {
        float_to_u32((const float *)src, (uint32_t *)dst, count, norm);
        return EXR_SUCCESS;
    }

    /* HALF <-> UINT: widen to float in modest chunks, then narrow. */
    {
        float tmp[256];
        size_t off = 0;
        while (off < count) {
            size_t k = count - off;
            if (k > 256) k = 256;
            if (src_type == EXR_PIXEL_HALF)
                exr_half_to_float((const uint16_t *)src + off, tmp, k);
            else /* src UINT */
                u32_to_float((const uint32_t *)src + off, tmp, k, norm);

            if (dst_type == EXR_PIXEL_HALF)
                exr_float_to_half(tmp, (uint16_t *)dst + off, k);
            else /* dst UINT */
                float_to_u32(tmp, (uint32_t *)dst + off, k, norm);
            off += k;
        }
    }
    return EXR_SUCCESS;
}

/* ============================================================================
 * exr_part <-> interleaved float bridges
 * ========================================================================== */

/* Widen one source sample of arbitrary EXR type at linear index `idx`. */
static float load_sample(const void *plane, exr_pixel_type t, size_t idx) {
    switch (t) {
        case EXR_PIXEL_HALF: {
            float f;
            exr_half_to_float((const uint16_t *)plane + idx, &f, 1);
            return f;
        }
        case EXR_PIXEL_FLOAT:
            return ((const float *)plane)[idx];
        default: /* UINT */
            return (float)((const uint32_t *)plane)[idx];
    }
}

exr_result exr_part_to_rgba_float(const exr_allocator *a, const exr_part *part,
                                  float **out, int *out_width, int *out_height,
                                  int *out_channels) {
    int w, h, nch, c, x, y;
    size_t total;
    float *buf;
    if (!a) a = exr_default_allocator();
    if (!part || !out || part->is_deep || !part->images)
        return EXR_ERROR_INVALID_ARGUMENT;
    w = part->width;
    h = part->height;
    nch = part->header.num_channels;
    if (w <= 0 || h <= 0 || nch <= 0) return EXR_ERROR_INVALID_ARGUMENT;
    if (exr_mul_ovf((size_t)w, (size_t)h, &total) ||
        exr_mul_ovf(total, (size_t)nch, &total) ||
        exr_mul_ovf(total, sizeof(float), &total))
        return EXR_ERROR_CORRUPT;

    buf = (float *)exr_malloc(a, total);
    if (!buf) return EXR_ERROR_OUT_OF_MEMORY;

    for (c = 0; c < nch; ++c) {
        const exr_channel *ch = &part->header.channels[c];
        int xs = ch->x_sampling < 1 ? 1 : ch->x_sampling;
        int ys = ch->y_sampling < 1 ? 1 : ch->y_sampling;
        int cw = exr_num_samples(0, w - 1, xs);
        const void *plane = part->images[c];
        for (y = 0; y < h; ++y) {
            int cy = (ys == 1) ? y : (y / ys);
            for (x = 0; x < w; ++x) {
                int cx = (xs == 1) ? x : (x / xs);
                size_t sidx = (size_t)cy * (size_t)cw + (size_t)cx;
                buf[((size_t)y * w + x) * nch + c] =
                    load_sample(plane, ch->pixel_type, sidx);
            }
        }
    }
    *out = buf;
    if (out_width) *out_width = w;
    if (out_height) *out_height = h;
    if (out_channels) *out_channels = nch;
    return EXR_SUCCESS;
}

/* ============================================================================
 * Luminance-chroma (Y / RY / BY) -> RGBA reconstruction
 * ========================================================================== */

int exr_part_is_luminance_chroma(const exr_part *part) {
    int has_y = 0, has_ry = 0, has_by = 0, has_rgb = 0, c;
    if (!part || part->is_deep || !part->images) return 0;
    for (c = 0; c < part->header.num_channels; ++c) {
        const char *n = part->header.channels[c].name;
        if (strcmp(n, "Y") == 0)
            has_y = 1;
        else if (strcmp(n, "RY") == 0)
            has_ry = 1;
        else if (strcmp(n, "BY") == 0)
            has_by = 1;
        else if (strcmp(n, "R") == 0 || strcmp(n, "G") == 0 ||
                 strcmp(n, "B") == 0)
            has_rgb = 1;
    }
    return has_y && has_ry && has_by && !has_rgb;
}

/* Separable linear (bilinear) upsample of a cw x ch subsampled plane to w x h.
 * Chroma sample (cx,cy) sits at full-res pixel (cx*xs, cy*ys). OpenEXR uses a
 * sharper 13-tap reconstruction filter (ImfRgbaYca); bilinear is chosen here
 * for simplicity and to stay libm-free, and is visually equivalent on the
 * smoothly-varying chroma these images carry. */
static void yc_upsample_plane(const void *plane, exr_pixel_type t, int cw,
                              int ch, int w, int h, int xs, int ys,
                              float *out) {
    int x, y;
    for (y = 0; y < h; ++y) {
        int gy = y / ys;
        int gy1 = (gy + 1 < ch) ? gy + 1 : gy;
        float fy = (float)(y - gy * ys) / (float)ys;
        for (x = 0; x < w; ++x) {
            int gx = x / xs;
            int gx1 = (gx + 1 < cw) ? gx + 1 : gx;
            float fx = (float)(x - gx * xs) / (float)xs;
            float v00 = load_sample(plane, t, (size_t)gy * cw + gx);
            float v01 = load_sample(plane, t, (size_t)gy * cw + gx1);
            float v10 = load_sample(plane, t, (size_t)gy1 * cw + gx);
            float v11 = load_sample(plane, t, (size_t)gy1 * cw + gx1);
            float top = v00 + (v01 - v00) * fx;
            float bot = v10 + (v11 - v10) * fx;
            out[(size_t)y * w + x] = top + (bot - top) * fy;
        }
    }
}

exr_result exr_part_yc_to_rgba_float(const exr_allocator *a,
                                     const exr_part *part, float **out,
                                     int *out_width, int *out_height) {
    int w, h, c, x, y;
    int yi = -1, ryi = -1, byi = -1, ai = -1;
    float yw[3];
    float *ry_full = NULL, *by_full = NULL, *buf = NULL;
    size_t npx, total;
    exr_result rc = EXR_SUCCESS;
    if (!a) a = exr_default_allocator();
    if (!part || !out) return EXR_ERROR_INVALID_ARGUMENT;
    if (!exr_part_is_luminance_chroma(part)) return EXR_ERROR_INVALID_ARGUMENT;
    w = part->width;
    h = part->height;
    if (w <= 0 || h <= 0) return EXR_ERROR_INVALID_ARGUMENT;

    for (c = 0; c < part->header.num_channels; ++c) {
        const char *n = part->header.channels[c].name;
        if (strcmp(n, "Y") == 0)
            yi = c;
        else if (strcmp(n, "RY") == 0)
            ryi = c;
        else if (strcmp(n, "BY") == 0)
            byi = c;
        else if (strcmp(n, "A") == 0)
            ai = c;
    }
    if (yi < 0 || ryi < 0 || byi < 0) return EXR_ERROR_INVALID_ARGUMENT;

    exr_luminance_weights(part->header.chromaticities,
                          part->header.has_chromaticities, yw);
    if (yw[1] == 0.0f) return EXR_ERROR_CORRUPT; /* would divide by zero */

    if (exr_mul_ovf((size_t)w, (size_t)h, &npx) ||
        exr_mul_ovf(npx, sizeof(float) * 4, &total))
        return EXR_ERROR_CORRUPT; /* guards npx (and npx*4 for the chroma planes) */
    buf = (float *)exr_malloc(a, total);
    ry_full = (float *)exr_malloc(a, npx * sizeof(float));
    by_full = (float *)exr_malloc(a, npx * sizeof(float));
    if (!buf || !ry_full || !by_full) {
        rc = EXR_ERROR_OUT_OF_MEMORY;
        goto done;
    }

    /* Upsample the two subsampled chroma planes to full resolution. */
    {
        const exr_channel *cry = &part->header.channels[ryi];
        const exr_channel *cby = &part->header.channels[byi];
        int rxs = cry->x_sampling < 1 ? 1 : cry->x_sampling;
        int rys = cry->y_sampling < 1 ? 1 : cry->y_sampling;
        int bxs = cby->x_sampling < 1 ? 1 : cby->x_sampling;
        int bys = cby->y_sampling < 1 ? 1 : cby->y_sampling;
        yc_upsample_plane(part->images[ryi], cry->pixel_type,
                          exr_num_samples(0, w - 1, rxs),
                          exr_num_samples(0, h - 1, rys), w, h, rxs, rys,
                          ry_full);
        yc_upsample_plane(part->images[byi], cby->pixel_type,
                          exr_num_samples(0, w - 1, bxs),
                          exr_num_samples(0, h - 1, bys), w, h, bxs, bys,
                          by_full);
    }

    /* Convert Y + reconstructed chroma to RGB(A). */
    {
        const exr_channel *cy = &part->header.channels[yi];
        int yxs = cy->x_sampling < 1 ? 1 : cy->x_sampling;
        int yys = cy->y_sampling < 1 ? 1 : cy->y_sampling;
        int ycw = exr_num_samples(0, w - 1, yxs);
        const void *yplane = part->images[yi];
        const void *aplane = (ai >= 0) ? part->images[ai] : NULL;
        exr_pixel_type aty =
            (ai >= 0) ? part->header.channels[ai].pixel_type : EXR_PIXEL_HALF;
        int axs = 1, ays = 1, acw = w;
        if (ai >= 0) {
            const exr_channel *ca = &part->header.channels[ai];
            axs = ca->x_sampling < 1 ? 1 : ca->x_sampling;
            ays = ca->y_sampling < 1 ? 1 : ca->y_sampling;
            acw = exr_num_samples(0, w - 1, axs);
        }
        for (y = 0; y < h; ++y) {
            int ycy = y / yys;
            for (x = 0; x < w; ++x) {
                size_t p = (size_t)y * (size_t)w + (size_t)x;
                float Y = load_sample(yplane, cy->pixel_type,
                                      (size_t)ycy * ycw + (x / yxs));
                float RY = ry_full[p];
                float BY = by_full[p];
                float r = (RY + 1.0f) * Y;
                float b = (BY + 1.0f) * Y;
                float g = (Y - r * yw[0] - b * yw[2]) / yw[1];
                float A = 1.0f;
                if (aplane)
                    A = load_sample(aplane, aty,
                                    (size_t)(y / ays) * acw + (x / axs));
                buf[p * 4 + 0] = r;
                buf[p * 4 + 1] = g;
                buf[p * 4 + 2] = b;
                buf[p * 4 + 3] = A;
            }
        }
    }

    *out = buf;
    buf = NULL;
    if (out_width) *out_width = w;
    if (out_height) *out_height = h;
done:
    exr_free(a, ry_full);
    exr_free(a, by_full);
    exr_free(a, buf);
    return rc;
}

exr_result exr_rgba_float_to_part(const exr_allocator *a, const float *rgba,
                                  int width, int height, int channels,
                                  exr_pixel_type dst_type, exr_part *out) {
    static const char *names1[] = {"Y"};
    static const char *names2[] = {"R", "G"};
    static const char *names3[] = {"R", "G", "B"};
    static const char *names4[] = {"R", "G", "B", "A"};
    const char **names;
    size_t npx, plane_bytes;
    int c, i;
    exr_result rc = EXR_SUCCESS;
    if (!a) a = exr_default_allocator();
    if (!rgba || !out || width <= 0 || height <= 0 || channels < 1 ||
        channels > 4)
        return EXR_ERROR_INVALID_ARGUMENT;
    names = channels == 1 ? names1
                          : channels == 2 ? names2
                                          : channels == 3 ? names3 : names4;

    memset(out, 0, sizeof(*out));
    out->width = width;
    out->height = height;
    out->header.part_type = EXR_PART_SCANLINE;
    out->header.compression = EXR_COMPRESSION_ZIP;
    out->header.num_channels = channels;
    out->header.data_window.min_x = 0;
    out->header.data_window.min_y = 0;
    out->header.data_window.max_x = width - 1;
    out->header.data_window.max_y = height - 1;
    out->header.display_window = out->header.data_window;
    out->header.pixel_aspect_ratio = 1.0f;
    out->header.screen_window_width = 1.0f;

    out->header.channels =
        (exr_channel *)exr_calloc(a, (size_t)channels, sizeof(exr_channel));
    out->images = (void **)exr_calloc(a, (size_t)channels, sizeof(void *));
    if (!out->header.channels || !out->images) {
        rc = EXR_ERROR_OUT_OF_MEMORY;
        goto fail;
    }
    npx = (size_t)width * (size_t)height;
    plane_bytes = npx * exr_pixel_size(dst_type);
    for (c = 0; c < channels; ++c) {
        exr_channel *ch = &out->header.channels[c];
        size_t k = strlen(names[c]);
        memcpy(ch->name, names[c], k + 1);
        ch->pixel_type = dst_type;
        ch->x_sampling = 1;
        ch->y_sampling = 1;
        out->images[c] = exr_malloc(a, plane_bytes ? plane_bytes : 1);
        if (!out->images[c]) {
            rc = EXR_ERROR_OUT_OF_MEMORY;
            goto fail;
        }
    }
    /* Deinterleave + narrow each channel. */
    {
        float *scratch = (float *)exr_malloc(a, npx * sizeof(float));
        if (!scratch) {
            rc = EXR_ERROR_OUT_OF_MEMORY;
            goto fail;
        }
        for (c = 0; c < channels; ++c) {
            for (i = 0; (size_t)i < npx; ++i)
                scratch[i] = rgba[(size_t)i * channels + c];
            rc = exr_convert_pixels(out->images[c], dst_type, scratch,
                                    EXR_PIXEL_FLOAT, npx, EXR_CONVERT_RAW);
            if (!EXR_OK(rc)) break;
        }
        exr_free(a, scratch);
        if (!EXR_OK(rc)) goto fail;
    }
    return EXR_SUCCESS;

fail:
    exr_part_free(a, out);
    return rc;
}
