/*
 * TinyEXR texpipe - normal-map LOD coherence + Toksvig roughness.
 *
 * tir already decodes normals, filters the vectors and renormalizes per mip, so
 * normal maps stay unit-length across LODs. What it also exposes is the length
 * of the *pre-renormalize* averaged normal |N|: as sub-texel normals disagree,
 * |N| shrinks. Toksvig (2005) turns that shrink into an effective roughness so
 * specular highlights don't alias into shimmer at distance. tp_build_mips
 * captures |N| per level (via tir's normal_length_out) and calls
 * tp_toksvig_roughness to bake it into chain->roughness.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "texpipe_internal.h"

#include <math.h>
#include <string.h>

tp_result tp_toksvig_roughness(const float *normal_len, int count,
                               float base_roughness, float *out_rough) {
    float base = base_roughness, s;
    int i;
    if (!normal_len || !out_rough || count < 0) return TP_ERROR_INVALID_ARGUMENT;
    if (base < 1e-3f) base = 1e-3f; /* avoid a divide-by-zero specular power */
    if (base > 1.0f) base = 1.0f;

    /* Beckmann specular power for the base roughness. */
    s = 2.0f / (base * base) - 2.0f;
    if (s < 0.0f) s = 0.0f;

    for (i = 0; i < count; ++i) {
        float a = normal_len[i];
        float s_eff, rough;
        if (a > 1.0f) a = 1.0f;
        if (a < 1e-4f) a = 1e-4f;
        /* Toksvig effective specular power, then back to roughness. */
        s_eff = a * s / (a + s * (1.0f - a));
        rough = sqrtf(2.0f / (s_eff + 2.0f));
        if (rough < base) rough = base; /* filtering only adds roughness */
        if (rough > 1.0f) rough = 1.0f;
        out_rough[i] = rough;
    }
    return TP_SUCCESS;
}

tp_result tp_build_roughness_chain(const tir_allocator *a,
                                   const tp_mip_chain *src, tp_mip_chain *out) {
    int i, n, x, y;
    if (!src || !out || !src->roughness) return TP_ERROR_UNSUPPORTED;
    memset(out, 0, sizeof(*out));
    out->num_faces = src->num_faces;
    out->num_levels = src->num_levels;
    out->channels = 4;
    n = src->num_faces * src->num_levels;
    out->level = (tp_surface *)tp_alloc(a, (size_t)n * sizeof(tp_surface));
    if (!out->level) return TP_ERROR_OUT_OF_MEMORY;
    memset(out->level, 0, (size_t)n * sizeof(tp_surface));

    for (i = 0; i < n; ++i) {
        const tp_surface *ss = &src->level[i];
        tp_surface *ds = &out->level[i];
        const float *rough = src->roughness[i];
        if (!rough) {
            tp_mip_chain_free(a, out);
            return TP_ERROR_UNSUPPORTED;
        }
        ds->width = ss->width;
        ds->height = ss->height;
        ds->channels = 4;
        ds->stride = (size_t)ss->width * 4u * sizeof(float);
        ds->data = (float *)tp_alloc(a, ds->stride * (size_t)ss->height);
        if (!ds->data) {
            tp_mip_chain_free(a, out);
            return TP_ERROR_OUT_OF_MEMORY;
        }
        for (y = 0; y < ss->height; ++y) {
            float *drow = (float *)((uint8_t *)ds->data + (size_t)y * ds->stride);
            for (x = 0; x < ss->width; ++x) {
                drow[x * 4 + 0] = rough[y * ss->width + x];
                drow[x * 4 + 1] = 0.0f;
                drow[x * 4 + 2] = 0.0f;
                drow[x * 4 + 3] = 1.0f;
            }
        }
    }
    return TP_SUCCESS;
}
