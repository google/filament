/*
 * TinyEXR v3 - spectral image support (JCGT 2021 "An OpenEXR Layout for
 * Spectral Images"; afichet/spectral-exr).
 *
 * Spectral data is stored as ordinary EXR channels named by wavelength
 * (S{n}.{wl}nm emissive Stokes, T.{wl}nm reflective; comma decimal separator,
 * "nm" suffix), marked by a "spectralLayoutVersion" header attribute. This
 * module provides the channel-name parsing/formatting helpers and a high-level
 * wavelength-cube (exr_spectral_image) with load/save.
 *
 * This is an optional, hosted module (excluded from the freestanding core). It
 * deliberately avoids the stdio header (the freestanding gate forbids that
 * outside exr_stdio.c), so wavelength formatting is done by hand and parsing
 * uses a locale-independent decimal reader (no atof, no snprintf).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

/* Upper bound on distinct wavelengths gathered in a single query (real spectral
 * images carry tens to a few hundred). */
#define EXR_SPECTRAL_MAX_WL 4096

/* ---------------------------------------------------------------- low level */

/* Locale-independent decimal parser. Reads an optional sign, integer digits,
 * and (after '.' or ',') fractional digits; stops at the first other byte and
 * reports it via *endp. */
static float parse_decimal(const char *p, const char **endp) {
    float val = 0.0f, sign = 1.0f;
    if (*p == '-') {
        sign = -1.0f;
        p++;
    } else if (*p == '+') {
        p++;
    }
    while (*p >= '0' && *p <= '9') {
        val = val * 10.0f + (float)(*p - '0');
        p++;
    }
    if (*p == '.' || *p == ',') {
        float scale = 0.1f;
        p++;
        while (*p >= '0' && *p <= '9') {
            val += (float)(*p - '0') * scale;
            scale *= 0.1f;
            p++;
        }
    }
    if (endp) *endp = p;
    return val * sign;
}

/* Append the decimal digits of an unsigned value; returns the count written. */
static size_t u_to_str(unsigned v, char *out) {
    char tmp[12];
    size_t n = 0, i;
    if (v == 0) {
        out[0] = '0';
        return 1;
    }
    while (v) {
        tmp[n++] = (char)('0' + (v % 10));
        v /= 10;
    }
    for (i = 0; i < n; ++i) out[i] = tmp[n - 1 - i];
    return n;
}

float exr_spectral_channel_wavelength(const char *channel_name) {
    const char *p = channel_name, *end;
    float wl;
    if (!p) return -1.0f;
    if (p[0] == 'S' && p[1] >= '0' && p[1] <= '3' && p[2] == '.')
        p += 3;
    else if (p[0] == 'T' && p[1] == '.')
        p += 2;
    else
        return -1.0f;
    wl = parse_decimal(p, &end);
    if (end[0] != 'n' || end[1] != 'm') return -1.0f;
    return wl;
}

int exr_spectral_channel_stokes(const char *channel_name) {
    if (channel_name && channel_name[0] == 'S' && channel_name[1] >= '0' &&
        channel_name[1] <= '3' && channel_name[2] == '.')
        return channel_name[1] - '0';
    return -1;
}

int exr_is_spectral_channel(const char *channel_name) {
    return exr_spectral_channel_wavelength(channel_name) >= 0.0f ? 1 : 0;
}

void exr_spectral_channel_name(char *buf, size_t buf_size, exr_spectrum_type type,
                               int stokes, float wavelength_nm) {
    char tmp[64];
    size_t n = 0, fn, pad, i;
    char fb[12];
    int whole, frac;
    if (!buf || buf_size < 2) return;

    if (type == EXR_SPECTRUM_REFLECTIVE) {
        tmp[n++] = 'T';
        tmp[n++] = '.';
    } else {
        if (stokes < 0) stokes = 0;
        if (stokes > 3) stokes = 3;
        tmp[n++] = 'S';
        tmp[n++] = (char)('0' + stokes);
        tmp[n++] = '.';
    }

    whole = (int)wavelength_nm;
    if (whole < 0) whole = 0;
    frac = (int)((wavelength_nm - (float)whole) * 1000000.0f + 0.5f);
    if (frac >= 1000000) { /* rounding carry */
        whole++;
        frac -= 1000000;
    }
    if (frac < 0) frac = 0;

    n += u_to_str((unsigned)whole, tmp + n);
    tmp[n++] = ',';
    fn = u_to_str((unsigned)frac, fb);
    for (pad = fn; pad < 6; ++pad) tmp[n++] = '0';
    for (i = 0; i < fn; ++i) tmp[n++] = fb[i];
    tmp[n++] = 'n';
    tmp[n++] = 'm';
    tmp[n] = '\0';

    if (n + 1 > buf_size) n = buf_size - 1;
    memcpy(buf, tmp, n);
    buf[n] = '\0';
}

/* ------------------------------------------------------------- header probes */

int exr_is_spectral(const exr_header *hdr) {
    exr_attribute a;
    return exr_header_find_attribute(hdr, "spectralLayoutVersion", &a) ==
           EXR_SUCCESS;
}

exr_spectrum_type exr_spectrum_type_of(const exr_header *hdr) {
    int has_reflective = 0, has_emissive = 0, has_stokes = 0, c;
    if (!hdr) return EXR_SPECTRUM_NONE;
    for (c = 0; c < hdr->num_channels; ++c) {
        const char *name = hdr->channels[c].name;
        if (name[0] == 'T' && name[1] == '.') {
            has_reflective = 1;
        } else if (name[0] == 'S' && name[1] >= '0' && name[1] <= '3' &&
                   name[2] == '.') {
            has_emissive = 1;
            if (name[1] != '0') has_stokes = 1;
        }
    }
    if (has_reflective) return EXR_SPECTRUM_REFLECTIVE;
    if (has_stokes) return EXR_SPECTRUM_POLARISED;
    if (has_emissive) return EXR_SPECTRUM_EMISSIVE;
    return EXR_SPECTRUM_NONE;
}

/* Gather distinct wavelengths into `sorted` (ascending, deduped within
 * 0.01 nm), bounded by EXR_SPECTRAL_MAX_WL. Returns the unique count. */
static int32_t gather_wavelengths(const exr_header *hdr, float *sorted) {
    int32_t k = 0, c, i, j;
    for (c = 0; c < hdr->num_channels; ++c) {
        float wl = exr_spectral_channel_wavelength(hdr->channels[c].name);
        if (wl < 0.0f) continue;
        /* insertion into the sorted unique list */
        i = 0;
        while (i < k && sorted[i] < wl - 0.01f) ++i;
        if (i < k && sorted[i] <= wl + 0.01f) continue; /* duplicate */
        if (k >= EXR_SPECTRAL_MAX_WL) continue;          /* bounded */
        for (j = k; j > i; --j) sorted[j] = sorted[j - 1];
        sorted[i] = wl;
        ++k;
    }
    return k;
}

int32_t exr_spectral_wavelengths(const exr_header *hdr, float *out, int32_t max) {
    /* Gather into a private scratch so the dedupe/sort is never limited by the
     * caller's `max`, then copy what fits. The scratch is function-static (the
     * helpers assume single-threaded use); callers needing concurrency should
     * size their own buffer to the queried count. */
    static float scratch[EXR_SPECTRAL_MAX_WL];
    int32_t k, i, n;
    if (!hdr) return 0;
    if (out && max < 0) return 0;
    k = gather_wavelengths(hdr, scratch);
    if (out) {
        n = (k < max) ? k : max;
        for (i = 0; i < n; ++i) out[i] = scratch[i];
    }
    return k;
}

exr_result exr_spectral_units(const exr_header *hdr, char *buf, size_t buf_size) {
    if (exr_header_get_string_attribute(hdr, "ROOT/units", buf, buf_size, NULL) ==
        EXR_SUCCESS)
        return EXR_SUCCESS;
    return exr_header_get_string_attribute(hdr, "emissiveUnits", buf, buf_size,
                                           NULL);
}

exr_result exr_spectral_set_attributes(const exr_allocator *alloc,
                                       exr_header *hdr, exr_spectrum_type type,
                                       const char *units) {
    exr_result rc;
    if (!hdr) return EXR_ERROR_INVALID_ARGUMENT;
    rc = exr_header_set_string_attribute(alloc, hdr, "spectralLayoutVersion",
                                         EXR_SPECTRAL_LAYOUT_VERSION);
    if (!EXR_OK(rc)) return rc;
    if (units && units[0]) {
        const char *key = (type == EXR_SPECTRUM_REFLECTIVE) ? "ROOT/units"
                                                            : "emissiveUnits";
        rc = exr_header_set_string_attribute(alloc, hdr, key, units);
        if (!EXR_OK(rc)) return rc;
    }
    if (type == EXR_SPECTRUM_POLARISED) {
        rc = exr_header_set_string_attribute(alloc, hdr,
                                             "polarisationHandedness", "left");
        if (!EXR_OK(rc)) return rc;
    }
    return EXR_SUCCESS;
}

/* ------------------------------------------------------------- image load */

void exr_spectral_image_free(exr_spectral_image *img) {
    const exr_allocator *a;
    int s;
    if (!img) return;
    a = &img->alloc;
    exr_free(a, img->wavelengths);
    for (s = 0; s < 4; ++s) exr_free(a, img->stokes[s]);
    memset(img, 0, sizeof(*img));
}

/* Read one planar sample as float (any pixel type). */
static float grid_sample(const void *src, exr_pixel_type pt, size_t idx) {
    switch (pt) {
    case EXR_PIXEL_HALF: {
        uint16_t hv = ((const uint16_t *)src)[idx];
        float f;
        exr_half_to_float(&hv, &f, 1);
        return f;
    }
    case EXR_PIXEL_FLOAT:
        return ((const float *)src)[idx];
    case EXR_PIXEL_UINT:
        return (float)((const uint32_t *)src)[idx];
    }
    return 0.0f;
}

/* Fill *out from an already-loaded native image (part 0 only). out must be
 * zeroed by the caller with out->alloc set. */
static exr_result spectral_fill(const exr_allocator *a, const exr_image *img,
                                exr_spectral_image *out) {
    const exr_part *part;
    const exr_header *h;
    exr_spectrum_type type;
    int32_t nwl, w, hgt, c, nstokes, s;
    size_t npix;

    if (img->num_parts < 1) return EXR_ERROR_UNSUPPORTED;
    part = &img->parts[0];
    h = &part->header;
    if (!exr_is_spectral(h)) return EXR_ERROR_UNSUPPORTED;
    type = exr_spectrum_type_of(h);
    if (type == EXR_SPECTRUM_NONE) return EXR_ERROR_UNSUPPORTED;

    nwl = exr_spectral_wavelengths(h, NULL, 0);
    if (nwl <= 0) return EXR_ERROR_UNSUPPORTED;

    w = part->width;
    hgt = part->height;
    npix = (size_t)w * (size_t)hgt;

    out->width = w;
    out->height = hgt;
    out->type = type;
    out->num_wavelengths = nwl;
    out->wavelengths = (float *)exr_malloc(a, (size_t)nwl * sizeof(float));
    if (!out->wavelengths) return EXR_ERROR_OUT_OF_MEMORY;
    exr_spectral_wavelengths(h, out->wavelengths, nwl);

    nstokes = (type == EXR_SPECTRUM_POLARISED) ? 4 : 1;
    for (s = 0; s < nstokes; ++s) {
        out->stokes[s] = (float *)exr_calloc(a, npix ? (size_t)nwl * npix : 1,
                                             sizeof(float));
        if (!out->stokes[s]) return EXR_ERROR_OUT_OF_MEMORY;
    }

    for (c = 0; c < h->num_channels; ++c) {
        const char *nm = h->channels[c].name;
        float wl = exr_spectral_channel_wavelength(nm);
        const void *src;
        float *dst;
        int st, wi = -1, k;
        if (wl < 0.0f) continue;
        st = exr_spectral_channel_stokes(nm);
        if (st < 0) st = 0;          /* T. (reflective) */
        if (st >= nstokes) continue; /* S1..S3 on a non-polarised file */
        for (k = 0; k < nwl; ++k) {
            float d = out->wavelengths[k] - wl;
            if (d < 0.0f) d = -d;
            if (d < 0.01f) {
                wi = k;
                break;
            }
        }
        if (wi < 0) continue;
        src = part->images[c];
        if (!src) continue;
        dst = out->stokes[st] + (size_t)wi * npix;
        {
            int xs = h->channels[c].x_sampling, ys = h->channels[c].y_sampling;
            exr_pixel_type pt = h->channels[c].pixel_type;
            if (xs <= 1 && ys <= 1) {
                /* Full resolution: a direct convert/copy. */
                switch (pt) {
                case EXR_PIXEL_HALF:
                    exr_half_to_float((const uint16_t *)src, dst, npix);
                    break;
                case EXR_PIXEL_FLOAT:
                    memcpy(dst, src, npix * sizeof(float));
                    break;
                case EXR_PIXEL_UINT: {
                    size_t i;
                    const uint32_t *u = (const uint32_t *)src;
                    for (i = 0; i < npix; ++i) dst[i] = (float)u[i];
                    break;
                }
                }
            } else {
                /* Subsampled: point-expand the stored cw*chh grid to full res. */
                int minx = h->data_window.min_x, miny = h->data_window.min_y;
                int maxx = h->data_window.max_x, maxy = h->data_window.max_y;
                int cw = exr_num_samples(minx, maxx, xs <= 0 ? 1 : xs);
                int ch_ = exr_num_samples(miny, maxy, ys <= 0 ? 1 : ys);
                int yy, xx;
                if (cw <= 0 || ch_ <= 0) continue;
                for (yy = 0; yy < hgt; ++yy) {
                    int gy = exr_num_samples(miny, miny + yy, ys <= 0 ? 1 : ys) - 1;
                    if (gy < 0) gy = 0;
                    if (gy >= ch_) gy = ch_ - 1;
                    for (xx = 0; xx < w; ++xx) {
                        int gx = exr_num_samples(minx, minx + xx, xs <= 0 ? 1 : xs) - 1;
                        if (gx < 0) gx = 0;
                        if (gx >= cw) gx = cw - 1;
                        dst[(size_t)yy * w + xx] =
                            grid_sample(src, pt, (size_t)gy * cw + gx);
                    }
                }
            }
        }
    }

    exr_spectral_units(h, out->units, sizeof(out->units));
    exr_header_get_string_attribute(h, "polarisationHandedness", out->handedness,
                                    sizeof(out->handedness), NULL);
    return EXR_SUCCESS;
}

exr_result exr_spectral_load_from_memory(const void *data, size_t size,
                                         const exr_allocator *alloc,
                                         exr_spectral_image *out) {
    const exr_allocator *a = alloc ? alloc : exr_default_allocator();
    exr_image img;
    exr_result rc;
    if (!out) return EXR_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out));
    if (a) out->alloc = *a;
    memset(&img, 0, sizeof(img));
    rc = exr_load_from_memory(data, size, alloc, &img);
    if (!EXR_OK(rc)) return rc;
    rc = spectral_fill(a, &img, out);
    exr_image_free(&img);
    if (!EXR_OK(rc)) exr_spectral_image_free(out);
    return rc;
}

exr_result exr_spectral_load_from_file(const char *path,
                                       const exr_allocator *alloc,
                                       exr_spectral_image *out) {
    const exr_allocator *a = alloc ? alloc : exr_default_allocator();
    exr_image img;
    exr_result rc;
    if (!out) return EXR_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out));
    if (a) out->alloc = *a;
    memset(&img, 0, sizeof(img));
    rc = exr_load_from_file(path, alloc, &img);
    if (!EXR_OK(rc)) return rc;
    rc = spectral_fill(a, &img, out);
    exr_image_free(&img);
    if (!EXR_OK(rc)) exr_spectral_image_free(out);
    return rc;
}

/* ------------------------------------------------------------- accessors */

float exr_spectral_sample(const exr_spectral_image *img, int32_t s,
                          int32_t wavelength_index, int32_t x, int32_t y) {
    size_t npix;
    if (!img || s < 0 || s > 3 || !img->stokes[s]) return 0.0f;
    if (wavelength_index < 0 || wavelength_index >= img->num_wavelengths ||
        x < 0 || x >= img->width || y < 0 || y >= img->height)
        return 0.0f;
    npix = (size_t)img->width * (size_t)img->height;
    return img->stokes[s][(size_t)wavelength_index * npix +
                          (size_t)y * (size_t)img->width + (size_t)x];
}

int32_t exr_spectral_pixel(const exr_spectral_image *img, int32_t s, int32_t x,
                           int32_t y, float *out_spectrum) {
    size_t npix, base;
    int32_t wi;
    if (!img || !out_spectrum || s < 0 || s > 3 || !img->stokes[s]) return 0;
    if (x < 0 || x >= img->width || y < 0 || y >= img->height) return 0;
    npix = (size_t)img->width * (size_t)img->height;
    base = (size_t)y * (size_t)img->width + (size_t)x;
    for (wi = 0; wi < img->num_wavelengths; ++wi)
        out_spectrum[wi] = img->stokes[s][(size_t)wi * npix + base];
    return img->num_wavelengths;
}

const float *exr_spectral_wavelength_array(const exr_spectral_image *img,
                                           int32_t *out_count) {
    if (!img) {
        if (out_count) *out_count = 0;
        return NULL;
    }
    if (out_count) *out_count = img->num_wavelengths;
    return img->wavelengths;
}

/* ------------------------------------------------------------- image save */

static exr_result spectral_setup(const exr_allocator *a0, exr_spectrum_type type,
                                 int32_t width, int32_t height,
                                 int32_t num_wavelengths, const float *wavelengths,
                                 const float *samples, const char *units,
                                 exr_image *out_img) {
    const exr_allocator *a = a0 ? a0 : exr_default_allocator();
    exr_image img;
    exr_part *part;
    exr_header *h;
    int32_t c;
    size_t npix;
    exr_result rc;

    if (!out_img || width <= 0 || height <= 0 || num_wavelengths <= 0 ||
        !wavelengths || !samples)
        return EXR_ERROR_INVALID_ARGUMENT;
    if (!a) return EXR_ERROR_INVALID_ARGUMENT;

    memset(&img, 0, sizeof(img));
    img.alloc = *a;
    img.num_parts = 1;
    img.parts = (exr_part *)exr_calloc(a, 1, sizeof(exr_part));
    if (!img.parts) return EXR_ERROR_OUT_OF_MEMORY;

    part = &img.parts[0];
    h = &part->header;
    h->part_type = EXR_PART_SCANLINE;
    h->compression = EXR_COMPRESSION_ZIP; /* overridden by save's comp arg */
    h->line_order = EXR_LINEORDER_INCREASING_Y;
    h->data_window.min_x = 0;
    h->data_window.min_y = 0;
    h->data_window.max_x = width - 1;
    h->data_window.max_y = height - 1;
    h->display_window = h->data_window;
    h->pixel_aspect_ratio = 1.0f;
    h->screen_window_width = 1.0f;
    h->num_channels = num_wavelengths;
    h->channels =
        (exr_channel *)exr_calloc(a, (size_t)num_wavelengths, sizeof(exr_channel));
    part->width = width;
    part->height = height;
    part->images = (void **)exr_calloc(a, (size_t)num_wavelengths, sizeof(void *));
    if (!h->channels || !part->images) {
        exr_image_free(&img);
        return EXR_ERROR_OUT_OF_MEMORY;
    }

    npix = (size_t)width * (size_t)height;
    for (c = 0; c < num_wavelengths; ++c) {
        exr_channel *ch = &h->channels[c];
        float *plane;
        exr_spectral_channel_name(ch->name, sizeof(ch->name), type, 0,
                                  wavelengths[c]);
        ch->pixel_type = EXR_PIXEL_FLOAT;
        ch->x_sampling = 1;
        ch->y_sampling = 1;
        ch->p_linear = 0;
        plane = (float *)exr_malloc(a, npix ? npix * sizeof(float) : 1);
        if (!plane) {
            exr_image_free(&img);
            return EXR_ERROR_OUT_OF_MEMORY;
        }
        memcpy(plane, samples + (size_t)c * npix, npix * sizeof(float));
        part->images[c] = plane;
    }

    rc = exr_spectral_set_attributes(a, h, type, units);
    if (!EXR_OK(rc)) {
        exr_image_free(&img);
        return rc;
    }

    *out_img = img;
    return EXR_SUCCESS;
}

exr_result exr_spectral_setup_emissive(const exr_allocator *alloc, int32_t width,
                                       int32_t height, int32_t num_wavelengths,
                                       const float *wavelengths,
                                       const float *samples, const char *units,
                                       exr_image *out_img) {
    return spectral_setup(alloc, EXR_SPECTRUM_EMISSIVE, width, height,
                          num_wavelengths, wavelengths, samples, units, out_img);
}

exr_result exr_spectral_setup_reflective(const exr_allocator *alloc,
                                         int32_t width, int32_t height,
                                         int32_t num_wavelengths,
                                         const float *wavelengths,
                                         const float *samples, const char *units,
                                         exr_image *out_img) {
    return spectral_setup(alloc, EXR_SPECTRUM_REFLECTIVE, width, height,
                          num_wavelengths, wavelengths, samples, units, out_img);
}
