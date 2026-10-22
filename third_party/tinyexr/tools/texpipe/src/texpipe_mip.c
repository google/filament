/*
 * TinyEXR texpipe - content-aware mip-chain generation via tir.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "texpipe_internal.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------- pixel decoding */

static float tp_half_to_float(uint16_t h) {
    uint32_t sign = (uint32_t)(h & 0x8000u) << 16;
    uint32_t exp = (h >> 10) & 0x1fu;
    uint32_t man = h & 0x3ffu;
    uint32_t bits;
    union { uint32_t u; float f; } v;
    if (exp == 0u) {
        if (man == 0u) {
            bits = sign;
        } else {
            /* subnormal */
            exp = 127u - 15u + 1u;
            while ((man & 0x400u) == 0u) {
                man <<= 1;
                --exp;
            }
            man &= 0x3ffu;
            bits = sign | (exp << 23) | (man << 13);
        }
    } else if (exp == 0x1fu) {
        bits = sign | 0x7f800000u | (man << 13);
    } else {
        bits = sign | ((exp + (127u - 15u)) << 23) | (man << 13);
    }
    v.u = bits;
    return v.f;
}

static size_t tp_type_bytes(tir_pixel_type t) {
    switch (t) {
    case TIR_F32: return 4;
    case TIR_F16: return 2;
    case TIR_U8: return 1;
    case TIR_U16: return 2;
    }
    return 0;
}

static float tp_read_sample(const tir_image_view *v, int x, int y, int c) {
    /* row_stride_bytes == 0 means tightly packed (tir's convention). */
    size_t stride = v->row_stride_bytes
                        ? v->row_stride_bytes
                        : (size_t)v->width * (size_t)v->channels *
                              tp_type_bytes(v->type);
    const uint8_t *row = (const uint8_t *)v->data + (size_t)y * stride;
    size_t idx = (size_t)x * (size_t)v->channels + (size_t)c;
    switch (v->type) {
    case TIR_F32: return ((const float *)row)[idx];
    case TIR_F16: return tp_half_to_float(((const uint16_t *)row)[idx]);
    case TIR_U8: return (float)((const uint8_t *)row)[idx] / 255.0f;
    case TIR_U16: return (float)((const uint16_t *)row)[idx] / 65535.0f;
    }
    return 0.0f;
}

/* Exposed for texpipe_disp.c (min-max pyramid reads the base). */
float tp_read_view_sample(const tir_image_view *v, int x, int y, int c) {
    return tp_read_sample(v, x, y, c);
}

/* ------------------------------------------------------- surface alloc */

static tp_result tp_surface_alloc(const tir_allocator *a, tp_surface *s, int w,
                                  int h, int channels) {
    size_t stride = (size_t)w * (size_t)channels * sizeof(float);
    s->width = w;
    s->height = h;
    s->channels = channels;
    s->stride = stride;
    s->data = (float *)tp_alloc(a, stride * (size_t)h);
    if (!s->data) return TP_ERROR_OUT_OF_MEMORY;
    return TP_SUCCESS;
}

/* Level 0 is the authored image: a straight type-convert, no resample. */
static void tp_copy_base(const tir_image_view *src, tp_surface *dst) {
    int x, y, c;
    for (y = 0; y < dst->height; ++y) {
        float *drow = (float *)((uint8_t *)dst->data + (size_t)y * dst->stride);
        for (x = 0; x < dst->width; ++x)
            for (c = 0; c < dst->channels; ++c)
                drow[x * dst->channels + c] = tp_read_sample(src, x, y, c);
    }
}

static float tp_srgb_to_linear(float c) {
    if (c <= 0.04045f) return c / 12.92f;
    return powf((c + 0.055f) / 1.055f, 2.4f);
}
static float tp_linear_to_srgb(float c) {
    if (c <= 0.0f) return 0.0f;
    if (c <= 0.0031308f) return c * 12.92f;
    return 1.055f * powf(c, 1.0f / 2.4f) - 0.055f;
}

static tir_mode tp_tir_mode(tp_content content) {
    switch (content) {
    case TP_CONTENT_NORMAL: return TIR_MODE_NORMAL_MAP;
    case TP_CONTENT_HEIGHT: return TIR_MODE_HEIGHTMAP;
    default: return TIR_MODE_GENERAL;
    }
}

static void tp_view_of_surface(const tp_surface *s, tir_image_view *v) {
    v->data = s->data;
    v->width = s->width;
    v->height = s->height;
    v->channels = s->channels;
    v->type = TIR_F32;
    v->row_stride_bytes = s->stride;
}

/* ------------------------------------------------------- build */

tp_result tp_build_mips(const tir_allocator *a, const tir_image_view *faces,
                        int num_faces, const tp_options *opt,
                        tp_mip_chain *out) {
    tp_codec_desc d;
    int face, level, num_levels, channels, normal_channels = 0;
    tp_result pr;

    if (!faces || !opt || !out || num_faces < 1) return TP_ERROR_INVALID_ARGUMENT;
    /* num_faces is 1 (2D) or 6 (cubemap, order +X,-X,+Y,-Y,+Z,-Z). */
    if (num_faces != 1 && num_faces != 6) return TP_ERROR_UNSUPPORTED;
    pr = tp_codec_describe(opt->codec, opt, &d);
    if (!TP_OK(pr)) return pr;

    if (opt->content == TP_CONTENT_NORMAL) {
        /* tir's normal-map mode filters 2 (RG) or 3 (xyz) components. The chain
         * surfaces hold exactly those; compression pads them back to RGBA. */
        normal_channels = (opt->normal_encoding == TIR_NORMAL_RG) ? 2 : 3;
        channels = normal_channels;
        if (faces[0].channels < normal_channels) return TP_ERROR_INVALID_ARGUMENT;
    } else {
        channels = faces[0].channels;
        if (channels != d.channels_in) return TP_ERROR_INVALID_ARGUMENT;
    }
    if (faces[0].width < 1 || faces[0].height < 1) return TP_ERROR_INVALID_ARGUMENT;
    if (num_faces == 6) {
        int fi;
        /* Cube faces must be square and identically sized. */
        if (faces[0].width != faces[0].height) return TP_ERROR_INVALID_ARGUMENT;
        for (fi = 1; fi < 6; ++fi)
            if (faces[fi].width != faces[0].width ||
                faces[fi].height != faces[0].height ||
                faces[fi].channels != faces[0].channels)
                return TP_ERROR_INVALID_ARGUMENT;
    }

    num_levels = tp_level_count(faces[0].width, faces[0].height, opt->max_levels);

    memset(out, 0, sizeof(*out));
    out->num_faces = num_faces;
    out->num_levels = num_levels;
    out->channels = channels;
    out->level = (tp_surface *)tp_alloc(
        a, (size_t)num_faces * (size_t)num_levels * sizeof(tp_surface));
    if (!out->level) return TP_ERROR_OUT_OF_MEMORY;
    memset(out->level, 0,
           (size_t)num_faces * (size_t)num_levels * sizeof(tp_surface));

    /* Toksvig roughness baking (normal maps only): allocate the per-surface
     * roughness pointer array; each entry filled below. */
    if (opt->content == TP_CONTENT_NORMAL && opt->bake_toksvig_roughness) {
        size_t np = (size_t)num_faces * (size_t)num_levels;
        out->roughness = (float **)tp_alloc(a, np * sizeof(float *));
        if (!out->roughness) {
            tp_mip_chain_free(a, out);
            return TP_ERROR_OUT_OF_MEMORY;
        }
        memset(out->roughness, 0, np * sizeof(float *));
    }

    for (face = 0; face < num_faces; ++face) {
        const tir_image_view *base = &faces[face];
        tir_image_view src_base = *base;
        float *base3 = NULL, *lin_base = NULL, *sq_base = NULL;
        int srgb_aware = 0, rough_op = 0;
        float base_coverage = 0.0f;
        int do_coverage = (opt->content == TP_CONTENT_ALPHA_TESTED &&
                           opt->preserve_alpha_coverage != 0 && channels == 4);

        if (opt->content == TP_CONTENT_NORMAL) {
            /* Repack the base's first `channels` components into a tight F32
             * buffer so tir gets a 2/3-channel normal image (its requirement). */
            int bx, by, bc;
            base3 = (float *)tp_alloc(a, (size_t)base->width * (size_t)base->height *
                                             (size_t)channels * sizeof(float));
            if (!base3) { tp_mip_chain_free(a, out); return TP_ERROR_OUT_OF_MEMORY; }
            for (by = 0; by < base->height; ++by)
                for (bx = 0; bx < base->width; ++bx)
                    for (bc = 0; bc < channels; ++bc)
                        base3[((size_t)by * base->width + bx) * channels + bc] =
                            tp_read_sample(base, bx, by, bc);
            src_base.data = base3;
            src_base.channels = channels;
            src_base.type = TIR_F32;
            src_base.row_stride_bytes = 0;
        }

        /* sRGB-aware albedo: a linear-light copy of the base to filter in. */
        {
            int cc;
            srgb_aware = (opt->srgb_aware && opt->content == TP_CONTENT_COLOR &&
                          channels == 4);
            for (cc = 0; cc < 4; ++cc)
                if (opt->channel_op[cc] == TP_CH_ROUGHNESS) rough_op = 1;
            rough_op = rough_op && channels == 4 &&
                       (opt->content == TP_CONTENT_COLOR ||
                        opt->content == TP_CONTENT_ALPHA_TESTED);
            if (srgb_aware) {
                int bx, by;
                lin_base = (float *)tp_alloc(a, (size_t)base->width *
                                                    (size_t)base->height * 4 *
                                                    sizeof(float));
                if (!lin_base) { tp_dealloc(a, base3); tp_mip_chain_free(a, out); return TP_ERROR_OUT_OF_MEMORY; }
                for (by = 0; by < base->height; ++by)
                    for (bx = 0; bx < base->width; ++bx) {
                        size_t o = ((size_t)by * base->width + bx) * 4;
                        lin_base[o + 0] = tp_srgb_to_linear(tp_read_sample(base, bx, by, 0));
                        lin_base[o + 1] = tp_srgb_to_linear(tp_read_sample(base, bx, by, 1));
                        lin_base[o + 2] = tp_srgb_to_linear(tp_read_sample(base, bx, by, 2));
                        lin_base[o + 3] = tp_read_sample(base, bx, by, 3);
                    }
            }
            if (rough_op) {
                int bx, by, bc;
                sq_base = (float *)tp_alloc(a, (size_t)base->width *
                                                   (size_t)base->height * 4 *
                                                   sizeof(float));
                if (!sq_base) { tp_dealloc(a, lin_base); tp_dealloc(a, base3); tp_mip_chain_free(a, out); return TP_ERROR_OUT_OF_MEMORY; }
                for (by = 0; by < base->height; ++by)
                    for (bx = 0; bx < base->width; ++bx)
                        for (bc = 0; bc < 4; ++bc) {
                            float v = tp_read_sample(base, bx, by, bc);
                            sq_base[((size_t)by * base->width + bx) * 4 + bc] = v * v;
                        }
            }
        }

        for (level = 0; level < num_levels; ++level) {
            int idx = face * num_levels + level;
            tp_surface *s = &out->level[idx];
            int w = tp_level_dim(base->width, level);
            int h = tp_level_dim(base->height, level);
            float *rough = NULL;
            pr = tp_surface_alloc(a, s, w, h, channels);
            if (!TP_OK(pr)) {
                tp_dealloc(a, base3);
                tp_dealloc(a, lin_base);
                tp_dealloc(a, sq_base);
                tp_mip_chain_free(a, out);
                return pr;
            }
            if (out->roughness) {
                rough = (float *)tp_alloc(a, (size_t)w * (size_t)h * sizeof(float));
                if (!rough) {
                    tp_dealloc(a, base3);
                    tp_mip_chain_free(a, out);
                    return TP_ERROR_OUT_OF_MEMORY;
                }
                out->roughness[idx] = rough;
            }

            if (level == 0) {
                tp_copy_base(&src_base, s);
                if (do_coverage)
                    base_coverage = tp_alpha_coverage(s, opt->alpha_test_threshold);
                if (rough) {
                    /* Base normals are unit-length -> roughness == base. */
                    int k, np = w * h;
                    for (k = 0; k < np; ++k) rough[k] = opt->base_roughness;
                }
            } else {
                tir_options topt;
                tir_image_view srcv, dstv;
                tir_result tr;
                tir_options_init(&topt);
                topt.filter_x = opt->filter;
                topt.filter_y = opt->filter;
                topt.edge_x = opt->edge_x;
                topt.edge_y = opt->edge_y;
                topt.mode = tp_tir_mode(opt->content);
                topt.alpha = opt->alpha;
                topt.normal_encoding = opt->normal_encoding;
                topt.normal_renormalize = opt->renormalize;
                topt.num_threads = opt->threads;
                /* Capture pre-renormalize |N| into the roughness buffer, then
                 * map it in place (Toksvig). */
                topt.normal_length_out = rough;

                if (srgb_aware) {
                    /* Filter in linear light (forces from-base). */
                    srcv = *base;
                    srcv.data = lin_base;
                    srcv.channels = channels;
                    srcv.type = TIR_F32;
                    srcv.row_stride_bytes = 0;
                } else if (opt->mip_source == TP_MIP_FROM_PREVIOUS) {
                    tp_view_of_surface(&out->level[face * num_levels + level - 1],
                                       &srcv);
                } else {
                    srcv = src_base;
                }
                tp_view_of_surface(s, &dstv);

                tr = tir_resize(a, &srcv, &dstv, &topt);
                if (!TIR_OK(tr)) {
                    tp_dealloc(a, base3);
                    tp_mip_chain_free(a, out);
                    return TP_ERROR_UNSUPPORTED;
                }
                if (do_coverage)
                    tp_alpha_scale_to_coverage(s, base_coverage,
                                               opt->alpha_test_threshold);
                if (rough)
                    tp_toksvig_roughness(rough, w * h, opt->base_roughness, rough);
                /* Per-channel packed-map rule: threshold MAJORITY channels so a
                 * binary metallic/mask stays crisp instead of averaging gray. */
                if ((opt->content == TP_CONTENT_COLOR ||
                     opt->content == TP_CONTENT_ALPHA_TESTED) && channels == 4) {
                    int cc, xx, yy;
                    for (cc = 0; cc < 4; ++cc) {
                        if (opt->channel_op[cc] != TP_CH_MAJORITY) continue;
                        for (yy = 0; yy < h; ++yy) {
                            float *row = (float *)((uint8_t *)s->data +
                                                   (size_t)yy * s->stride);
                            for (xx = 0; xx < w; ++xx) {
                                float *px = row + xx * channels;
                                px[cc] = px[cc] >= 0.5f ? 1.0f : 0.0f;
                            }
                        }
                    }
                }
                /* Roughness-variance packing: replace ROUGHNESS channels with
                 * the RMS roughness sqrt(E[c^2]) (>= filtered mean), so minified
                 * roughness reduces specular aliasing. E[c^2] = resized squared
                 * base. */
                if (rough_op) {
                    tir_options ropt;
                    tir_image_view sqv, e2v;
                    tp_surface e2s;
                    if (TP_OK(tp_surface_alloc(a, &e2s, w, h, 4))) {
                        tir_options_init(&ropt);
                        ropt.filter_x = opt->filter;
                        ropt.filter_y = opt->filter;
                        ropt.edge_x = opt->edge_x;
                        ropt.edge_y = opt->edge_y;
                        sqv = *base;
                        sqv.data = sq_base; sqv.channels = 4; sqv.type = TIR_F32;
                        sqv.row_stride_bytes = 0;
                        tp_view_of_surface(&e2s, &e2v);
                        if (TIR_OK(tir_resize(a, &sqv, &e2v, &ropt))) {
                            int cc2, xx, yy;
                            for (cc2 = 0; cc2 < 4; ++cc2) {
                                if (opt->channel_op[cc2] != TP_CH_ROUGHNESS) continue;
                                for (yy = 0; yy < h; ++yy) {
                                    float *row = (float *)((uint8_t *)s->data + (size_t)yy * s->stride);
                                    const float *e2r = (const float *)((uint8_t *)e2s.data + (size_t)yy * e2s.stride);
                                    for (xx = 0; xx < w; ++xx) {
                                        float e2 = e2r[xx * 4 + cc2];
                                        if (e2 < 0.0f) e2 = 0.0f;
                                        if (e2 > 1.0f) e2 = 1.0f;
                                        row[xx * channels + cc2] = sqrtf(e2);
                                    }
                                }
                            }
                        }
                        tp_dealloc(a, e2s.data);
                    }
                }
                /* sRGB-aware: re-encode the linear-filtered RGB back to sRGB so a
                 * hardware sRGB sampler decodes the correctly-filtered value. */
                if (srgb_aware) {
                    int xx, yy;
                    for (yy = 0; yy < h; ++yy) {
                        float *row = (float *)((uint8_t *)s->data + (size_t)yy * s->stride);
                        for (xx = 0; xx < w; ++xx) {
                            float *px = row + xx * channels;
                            px[0] = tp_linear_to_srgb(px[0]);
                            px[1] = tp_linear_to_srgb(px[1]);
                            px[2] = tp_linear_to_srgb(px[2]);
                        }
                    }
                }
            }
        }
        tp_dealloc(a, base3);
        tp_dealloc(a, lin_base);
        tp_dealloc(a, sq_base);
    }

    /* Cube seam fixup: make face borders bit-identical per mip level, after all
     * 6 faces of a level exist. cube_fixup_max_level (-1 = all) bounds it. */
    if (num_faces == 6 && opt->cube_seam_fixup) {
        int maxlvl = opt->cube_fixup_max_level;
        for (level = 0; level < num_levels; ++level) {
            tp_surface fs[6];
            int fi;
            if (maxlvl >= 0 && level > maxlvl) break;
            for (fi = 0; fi < 6; ++fi) fs[fi] = out->level[fi * num_levels + level];
            tp_cube_seam_fixup(fs, opt);
        }
    }

    /* Octahedral fold-seam fixup: per-mip border coherence for a 2D octa map. */
    if (num_faces == 1 && opt->projection == TP_PROJ_OCTA && opt->octa_seam_fixup) {
        for (level = 0; level < num_levels; ++level)
            tp_octa_seam_fixup(&out->level[level], opt);
    }

    /* YCoCg decorrelation: store colour as YCoCg (shader inverts) for better
     * chroma compression. Applied last, over every surface. */
    if (opt->ycocg && channels >= 3 &&
        (opt->content == TP_CONTENT_COLOR || opt->content == TP_CONTENT_ALPHA_TESTED)) {
        int n = out->num_faces * out->num_levels, i, xx, yy;
        for (i = 0; i < n; ++i) {
            tp_surface *s = &out->level[i];
            for (yy = 0; yy < s->height; ++yy) {
                float *row = (float *)((uint8_t *)s->data + (size_t)yy * s->stride);
                for (xx = 0; xx < s->width; ++xx) {
                    float *px = row + xx * s->channels, yc[3];
                    tp_rgb_to_ycocg(px, yc);
                    px[0] = yc[0]; px[1] = yc[1]; px[2] = yc[2];
                }
            }
        }
    }
    return TP_SUCCESS;
}

void tp_mip_chain_free(const tir_allocator *a, tp_mip_chain *c) {
    int i, n;
    if (!c) return;
    if (c->level) {
        n = c->num_faces * c->num_levels;
        for (i = 0; i < n; ++i) tp_dealloc(a, c->level[i].data);
        tp_dealloc(a, c->level);
        c->level = NULL;
    }
    if (c->roughness) {
        n = c->num_faces * c->num_levels;
        for (i = 0; i < n; ++i) tp_dealloc(a, c->roughness[i]);
        tp_dealloc(a, c->roughness);
        c->roughness = NULL;
    }
}
