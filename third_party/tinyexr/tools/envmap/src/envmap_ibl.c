/*
 * TinyEXR envmap - image-based lighting: GGX prefiltered specular, diffuse
 * irradiance, and the split-sum BRDF integration LUT (Karis 2013).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "envmap.h"

#include <math.h>

#ifndef EM_PI
#define EM_PI 3.14159265358979323846f
#endif

static void normalize3(float v[3]) {
    float l = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (l > 0.0f) { v[0] /= l; v[1] /= l; v[2] /= l; }
}
static float dot3(const float a[3], const float b[3]) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

/* Orthonormal frame (t,b) around unit normal n. */
static void basis(const float n[3], float t[3], float b[3]) {
    float up[3];
    if (fabsf(n[1]) < 0.999f) { up[0] = 0; up[1] = 1; up[2] = 0; }
    else { up[0] = 1; up[1] = 0; up[2] = 0; }
    /* t = normalize(cross(up,n)) */
    t[0] = up[1] * n[2] - up[2] * n[1];
    t[1] = up[2] * n[0] - up[0] * n[2];
    t[2] = up[0] * n[1] - up[1] * n[0];
    normalize3(t);
    /* b = cross(n,t) */
    b[0] = n[1] * t[2] - n[2] * t[1];
    b[1] = n[2] * t[0] - n[0] * t[2];
    b[2] = n[0] * t[1] - n[1] * t[0];
}

/* local (about +Z) -> world using frame (t,b,n). */
static void to_world(const float local[3], const float t[3], const float b[3],
                     const float n[3], float out[3]) {
    out[0] = local[0] * t[0] + local[1] * b[0] + local[2] * n[0];
    out[1] = local[0] * t[1] + local[1] * b[1] + local[2] * n[1];
    out[2] = local[0] * t[2] + local[1] * b[2] + local[2] * n[2];
}

void em_sample_ggx(const float xi[2], float roughness, float out_h[3]) {
    float a = roughness * roughness;
    float phi = 2.0f * EM_PI * xi[0];
    float ct = sqrtf((1.0f - xi[1]) / (1.0f + (a * a - 1.0f) * xi[1]));
    float st = sqrtf(1.0f - ct * ct > 0.0f ? 1.0f - ct * ct : 0.0f);
    out_h[0] = st * cosf(phi);
    out_h[1] = st * sinf(phi);
    out_h[2] = ct;
}

/* Prefilter one direction R (=N=V) at `roughness`. */
static void prefilter_dir(const em_image *src, const float R[3], float roughness,
                          int num_samples, float out[3]) {
    float t[3], b[3];
    float total = 0.0f, acc[3] = {0, 0, 0};
    int i;
    if (roughness <= 0.0f) { em_sample(src, R, out); return; }
    basis(R, t, b);
    for (i = 0; i < num_samples; ++i) {
        float xi[2], hl[3], H[3], L[3], vdoth, ndotl, rgb[3];
        em_hammersley((uint32_t)i, (uint32_t)num_samples, xi);
        em_sample_ggx(xi, roughness, hl);
        to_world(hl, t, b, R, H);
        /* L = reflect(-V, H) with V = R */
        vdoth = dot3(R, H);
        L[0] = 2.0f * vdoth * H[0] - R[0];
        L[1] = 2.0f * vdoth * H[1] - R[1];
        L[2] = 2.0f * vdoth * H[2] - R[2];
        ndotl = dot3(R, L);
        if (ndotl > 0.0f) {
            em_sample(src, L, rgb);
            acc[0] += rgb[0] * ndotl;
            acc[1] += rgb[1] * ndotl;
            acc[2] += rgb[2] * ndotl;
            total += ndotl;
        }
    }
    if (total > 0.0f) {
        out[0] = acc[0] / total; out[1] = acc[1] / total; out[2] = acc[2] / total;
    } else {
        em_sample(src, R, out);
    }
}

em_result em_prefilter_specular(const tir_allocator *a, const em_image *src,
                                int face_size, int num_levels, int num_samples,
                                em_image *out_levels) {
    int l, face, x, y;
    if (!src || !out_levels || face_size < 1 || num_levels < 1)
        return EM_ERROR_INVALID_ARGUMENT;
    if (num_samples < 1) num_samples = 64;
    for (l = 0; l < num_levels; ++l) {
        int dim = face_size >> l;
        float roughness = (num_levels > 1) ? (float)l / (float)(num_levels - 1) : 0.0f;
        em_result r;
        if (dim < 1) dim = 1;
        r = em_image_alloc(a, &out_levels[l], EM_PROJ_CUBE, dim, dim, 3);
        if (!EM_OK(r)) {
            int k;
            for (k = 0; k < l; ++k) em_image_free(a, &out_levels[k]);
            return r;
        }
        for (face = 0; face < 6; ++face)
            for (y = 0; y < dim; ++y)
                for (x = 0; x < dim; ++x) {
                    float u = (x + 0.5f) / dim, v = (y + 0.5f) / dim, R[3], rgb[3];
                    float *t = em_image_texel(&out_levels[l], face, x, y);
                    em_uv_to_dir(EM_PROJ_CUBE, face, u, v, R);
                    prefilter_dir(src, R, roughness, num_samples, rgb);
                    t[0] = rgb[0]; t[1] = rgb[1]; t[2] = rgb[2];
                }
    }
    return EM_SUCCESS;
}

em_result em_irradiance_cube(const tir_allocator *a, const em_image *src,
                             int face_size, int num_samples, em_image *out) {
    int face, x, y, i;
    em_result r;
    if (!src || !out || face_size < 1) return EM_ERROR_INVALID_ARGUMENT;
    if (num_samples < 1) num_samples = 256;
    r = em_image_alloc(a, out, EM_PROJ_CUBE, face_size, face_size, 3);
    if (!EM_OK(r)) return r;
    for (face = 0; face < 6; ++face)
        for (y = 0; y < face_size; ++y)
            for (x = 0; x < face_size; ++x) {
                float u = (x + 0.5f) / face_size, v = (y + 0.5f) / face_size;
                float N[3], t[3], b[3], acc[3] = {0, 0, 0};
                float *dst = em_image_texel(out, face, x, y);
                em_uv_to_dir(EM_PROJ_CUBE, face, u, v, N);
                basis(N, t, b);
                for (i = 0; i < num_samples; ++i) {
                    float xi[2], cl[3], L[3], rgb[3], r2, phi;
                    em_hammersley((uint32_t)i, (uint32_t)num_samples, xi);
                    /* cosine-weighted hemisphere sample about +Z */
                    r2 = sqrtf(xi[0]); phi = 2.0f * EM_PI * xi[1];
                    cl[0] = r2 * cosf(phi); cl[1] = r2 * sinf(phi);
                    cl[2] = sqrtf(1.0f - xi[0] > 0.0f ? 1.0f - xi[0] : 0.0f);
                    to_world(cl, t, b, N, L);
                    em_sample(src, L, rgb);
                    acc[0] += rgb[0]; acc[1] += rgb[1]; acc[2] += rgb[2];
                }
                /* cosine importance: estimator of E/pi is the plain average. */
                dst[0] = acc[0] / num_samples;
                dst[1] = acc[1] / num_samples;
                dst[2] = acc[2] / num_samples;
            }
    return EM_SUCCESS;
}

void em_shade_point(const em_image *spec_levels, int num_levels,
                    const em_image *irradiance, const float *brdf_lut,
                    int lut_size, const float N[3], const float V[3],
                    const float albedo[3], float roughness, float metallic,
                    float out_rgb[3]) {
    float ndotv = dot3(N, V);
    float R[3], irr[3], spec[3], F0[3];
    float lod, f, A, B;
    int l0, l1, bx, by, c;
    if (ndotv < 1e-4f) ndotv = 1e-4f;
    if (ndotv > 1.0f) ndotv = 1.0f;

    /* diffuse: irradiance stored as E/pi, Lambert = albedo * (E/pi). */
    em_sample(irradiance, N, irr);

    /* specular: reflect V about N, sample the roughness chain (trilinear over
     * the two bracketing levels). */
    R[0] = 2.0f * ndotv * N[0] - V[0];
    R[1] = 2.0f * ndotv * N[1] - V[1];
    R[2] = 2.0f * ndotv * N[2] - V[2];
    lod = roughness * (float)(num_levels - 1);
    if (lod < 0.0f) lod = 0.0f;
    l0 = (int)lod;
    if (l0 > num_levels - 1) l0 = num_levels - 1;
    l1 = l0 + 1 < num_levels ? l0 + 1 : num_levels - 1;
    f = lod - (float)l0;
    {
        float s0[3], s1[3];
        em_sample(&spec_levels[l0], R, s0);
        em_sample(&spec_levels[l1], R, s1);
        spec[0] = s0[0] * (1 - f) + s1[0] * f;
        spec[1] = s0[1] * (1 - f) + s1[1] * f;
        spec[2] = s0[2] * (1 - f) + s1[2] * f;
    }

    for (c = 0; c < 3; ++c) F0[c] = 0.04f * (1.0f - metallic) + albedo[c] * metallic;

    bx = (int)(ndotv * lut_size);
    by = (int)(roughness * lut_size);
    if (bx < 0) bx = 0;
    if (bx > lut_size - 1) bx = lut_size - 1;
    if (by < 0) by = 0;
    if (by > lut_size - 1) by = lut_size - 1;
    A = brdf_lut[(by * lut_size + bx) * 2 + 0];
    B = brdf_lut[(by * lut_size + bx) * 2 + 1];

    for (c = 0; c < 3; ++c) {
        float diffuse = albedo[c] * (1.0f - metallic) * irr[c];
        float specular = spec[c] * (F0[c] * A + B);
        out_rgb[c] = diffuse + specular;
    }
}

static float g_smith_ibl(float ndotv, float ndotl, float roughness) {
    float a = roughness * roughness;
    float k = a * a / 2.0f;
    float gv = ndotv / (ndotv * (1.0f - k) + k);
    float gl = ndotl / (ndotl * (1.0f - k) + k);
    return gv * gl;
}

void em_brdf_lut(int size, int num_samples, float *out_rg) {
    int iy, ix, i;
    if (size < 1 || !out_rg) return;
    if (num_samples < 1) num_samples = 512;
    for (iy = 0; iy < size; ++iy) {
        float roughness = (iy + 0.5f) / size;
        for (ix = 0; ix < size; ++ix) {
            float ndotv = (ix + 0.5f) / size;
            float V[3], A = 0.0f, B = 0.0f;
            float t[3], b[3], N[3] = {0, 0, 1};
            V[0] = sqrtf(1.0f - ndotv * ndotv > 0.0f ? 1.0f - ndotv * ndotv : 0.0f);
            V[1] = 0.0f; V[2] = ndotv;
            basis(N, t, b);
            for (i = 0; i < num_samples; ++i) {
                float xi[2], hl[3], H[3], L[3], vdoth, ndotl, ndoth, g, gvis, fc;
                em_hammersley((uint32_t)i, (uint32_t)num_samples, xi);
                em_sample_ggx(xi, roughness, hl);
                to_world(hl, t, b, N, H);
                vdoth = V[0] * H[0] + V[1] * H[1] + V[2] * H[2];
                L[0] = 2.0f * vdoth * H[0] - V[0];
                L[1] = 2.0f * vdoth * H[1] - V[1];
                L[2] = 2.0f * vdoth * H[2] - V[2];
                ndotl = L[2];
                ndoth = H[2];
                if (ndotl > 0.0f && ndoth > 0.0f && vdoth > 0.0f) {
                    g = g_smith_ibl(ndotv, ndotl, roughness);
                    gvis = g * vdoth / (ndoth * ndotv);
                    fc = powf(1.0f - vdoth, 5.0f);
                    A += (1.0f - fc) * gvis;
                    B += fc * gvis;
                }
            }
            A /= num_samples;
            B /= num_samples;
            /* DFG terms are physically in [0,1]; clamp the grazing-corner MC
             * overshoot. */
            if (A < 0.0f) A = 0.0f;
            if (A > 1.0f) A = 1.0f;
            if (B < 0.0f) B = 0.0f;
            if (B > 1.0f) B = 1.0f;
            out_rg[(iy * size + ix) * 2 + 0] = A;
            out_rg[(iy * size + ix) * 2 + 1] = B;
        }
    }
}
