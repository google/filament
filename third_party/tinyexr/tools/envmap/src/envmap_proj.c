/*
 * TinyEXR envmap - projection math (equirect / cube / octahedral), sampling and
 * cross-projection resampling.
 *
 * Cube convention matches tools/texpipe/src/texpipe_cube.c exactly so envmap
 * cubes interoperate with texpipe's seam fixup:
 *   +X:( 1,-v,-u) -X:(-1,-v, u) +Y:( u, 1, v)
 *   -Y:( u,-1,-v) +Z:( u,-v, 1) -Z:(-u,-v,-1)   (u=col, v=row, both in [-1,1])
 *
 * Octahedral encode/decode uses the standard unit-vector parameterization from
 * Cigolle, Donow, Evangelakos, Mara, McGuire & Meyer, "A Survey of Efficient
 * Representations for Independent Unit Vectors", JCGT 2014.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "envmap.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef EM_PI
#define EM_PI 3.14159265358979323846f
#endif

const char *em_result_string(em_result r) {
    switch (r) {
    case EM_SUCCESS: return "success";
    case EM_ERROR_INVALID_ARGUMENT: return "invalid argument";
    case EM_ERROR_OUT_OF_MEMORY: return "out of memory";
    case EM_ERROR_UNSUPPORTED: return "unsupported";
    case EM_ERROR_IO: return "i/o error";
    }
    return "unknown";
}

static void *em_alloc(const tir_allocator *a, size_t n) {
    if (a && a->alloc) return a->alloc(a->user, n);
    return malloc(n);
}
static void em_dealloc(const tir_allocator *a, void *p) {
    if (!p) return;
    if (a && a->free) { a->free(a->user, p); return; }
    free(p);
}

em_result em_image_alloc(const tir_allocator *a, em_image *img, em_proj proj,
                         int width, int height, int channels) {
    size_t n;
    if (!img || width < 1 || height < 1 || channels < 1) return EM_ERROR_INVALID_ARGUMENT;
    img->proj = proj;
    img->width = width;
    img->height = height;
    img->channels = channels;
    img->faces = (proj == EM_PROJ_CUBE) ? 6 : 1;
    n = (size_t)img->faces * (size_t)width * (size_t)height * (size_t)channels;
    img->data = (float *)em_alloc(a, n * sizeof(float));
    if (!img->data) return EM_ERROR_OUT_OF_MEMORY;
    memset(img->data, 0, n * sizeof(float));
    return EM_SUCCESS;
}

void em_image_free(const tir_allocator *a, em_image *img) {
    if (!img) return;
    em_dealloc(a, img->data);
    img->data = NULL;
}

float *em_image_texel(const em_image *img, int face, int x, int y) {
    size_t off = ((size_t)face * (size_t)img->height + (size_t)y) * (size_t)img->width +
                 (size_t)x;
    return img->data + off * (size_t)img->channels;
}

/* -------------------------------------------------------- direction<->uv */

static float sgn(float a) { return a >= 0.0f ? 1.0f : -1.0f; }

static void cube_uv_to_dir(int face, float u01, float v01, float d[3]) {
    float u = 2.0f * u01 - 1.0f, v = 2.0f * v01 - 1.0f, l;
    switch (face) {
    case 0: d[0] = 1;  d[1] = -v; d[2] = -u; break;
    case 1: d[0] = -1; d[1] = -v; d[2] = u;  break;
    case 2: d[0] = u;  d[1] = 1;  d[2] = v;  break;
    case 3: d[0] = u;  d[1] = -1; d[2] = -v; break;
    case 4: d[0] = u;  d[1] = -v; d[2] = 1;  break;
    default: d[0] = -u; d[1] = -v; d[2] = -1; break;
    }
    l = sqrtf(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
    d[0] /= l; d[1] /= l; d[2] /= l;
}

static void cube_dir_to_uv(const float d[3], int *face, float *u01, float *v01) {
    float ax = fabsf(d[0]), ay = fabsf(d[1]), az = fabsf(d[2]);
    float ma, u, v;
    int f;
    if (ax >= ay && ax >= az) {
        ma = ax;
        if (d[0] > 0) { f = 0; u = -d[2] / ma; v = -d[1] / ma; }
        else          { f = 1; u = d[2] / ma;  v = -d[1] / ma; }
    } else if (ay >= ax && ay >= az) {
        ma = ay;
        if (d[1] > 0) { f = 2; u = d[0] / ma; v = d[2] / ma; }
        else          { f = 3; u = d[0] / ma; v = -d[2] / ma; }
    } else {
        ma = az;
        if (d[2] > 0) { f = 4; u = d[0] / ma;  v = -d[1] / ma; }
        else          { f = 5; u = -d[0] / ma; v = -d[1] / ma; }
    }
    *face = f;
    *u01 = 0.5f * (u + 1.0f);
    *v01 = 0.5f * (v + 1.0f);
}

static void equirect_uv_to_dir(float u01, float v01, float d[3]) {
    float phi = (u01 - 0.5f) * 2.0f * EM_PI;
    float theta = v01 * EM_PI;
    float st = sinf(theta);
    d[0] = st * cosf(phi);
    d[1] = cosf(theta);
    d[2] = -st * sinf(phi);
}

static void equirect_dir_to_uv(const float d[3], float *u01, float *v01) {
    float y = d[1];
    if (y < -1.0f) y = -1.0f;
    if (y > 1.0f) y = 1.0f;
    {
        float theta = acosf(y);
        float phi = atan2f(-d[2], d[0]);
        float u = 0.5f + phi / (2.0f * EM_PI);
        if (u < 0.0f) u += 1.0f;
        if (u >= 1.0f) u -= 1.0f;
        *u01 = u;
        *v01 = theta / EM_PI;
    }
}

static void octa_uv_to_dir(float u01, float v01, float d[3]) {
    float u = 2.0f * u01 - 1.0f, v = 2.0f * v01 - 1.0f, l;
    float x = u, z = v, y = 1.0f - fabsf(u) - fabsf(v);
    if (y < 0.0f) {
        x = (1.0f - fabsf(v)) * sgn(u);
        z = (1.0f - fabsf(u)) * sgn(v);
    }
    l = sqrtf(x * x + y * y + z * z);
    d[0] = x / l; d[1] = y / l; d[2] = z / l;
}

static void octa_dir_to_uv(const float d[3], float *u01, float *v01) {
    float s = fabsf(d[0]) + fabsf(d[1]) + fabsf(d[2]);
    float nx = d[0] / s, ny = d[1] / s, nz = d[2] / s;
    float ou, ov;
    if (ny >= 0.0f) { ou = nx; ov = nz; }
    else {
        ou = (1.0f - fabsf(nz)) * sgn(nx);
        ov = (1.0f - fabsf(nx)) * sgn(nz);
    }
    *u01 = 0.5f * (ou + 1.0f);
    *v01 = 0.5f * (ov + 1.0f);
}

void em_dir_to_uv(em_proj proj, const float dir[3], int *face, float *u, float *v) {
    *face = 0;
    if (proj == EM_PROJ_CUBE) { cube_dir_to_uv(dir, face, u, v); return; }
    if (proj == EM_PROJ_OCTA) { octa_dir_to_uv(dir, u, v); return; }
    equirect_dir_to_uv(dir, u, v);
}

void em_uv_to_dir(em_proj proj, int face, float u, float v, float dir[3]) {
    if (proj == EM_PROJ_CUBE) { cube_uv_to_dir(face, u, v, dir); return; }
    if (proj == EM_PROJ_OCTA) { octa_uv_to_dir(u, v, dir); return; }
    equirect_uv_to_dir(u, v, dir);
    {
        float l = sqrtf(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
        dir[0] /= l; dir[1] /= l; dir[2] /= l;
    }
}

/* -------------------------------------------------------- solid angle */

float em_texel_solid_angle(em_proj proj, int width, int height, int face, int x,
                           int y) {
    if (proj == EM_PROJ_EQUIRECT) {
        float theta = ((float)y + 0.5f) / (float)height * EM_PI;
        return sinf(theta) * (EM_PI / (float)height) *
               (2.0f * EM_PI / (float)width);
    }
    if (proj == EM_PROJ_CUBE) {
        float u = 2.0f * ((float)x + 0.5f) / (float)width - 1.0f;
        float v = 2.0f * ((float)y + 0.5f) / (float)height - 1.0f;
        float du = 2.0f / (float)width, dv = 2.0f / (float)height;
        float t = 1.0f + u * u + v * v;
        (void)face;
        return du * dv / (t * sqrtf(t));
    }
    /* octa: local Jacobian via finite differences of uv_to_dir. */
    {
        float du = 1.0f / (float)width, dv = 1.0f / (float)height;
        float u0 = ((float)x + 0.5f) / (float)width;
        float v0 = ((float)y + 0.5f) / (float)height;
        float d0[3], d1[3], d2[3], a[3], b[3], cx, cy, cz;
        octa_uv_to_dir(u0, v0, d0);
        octa_uv_to_dir(u0 + du, v0, d1);
        octa_uv_to_dir(u0, v0 + dv, d2);
        a[0] = d1[0] - d0[0]; a[1] = d1[1] - d0[1]; a[2] = d1[2] - d0[2];
        b[0] = d2[0] - d0[0]; b[1] = d2[1] - d0[1]; b[2] = d2[2] - d0[2];
        cx = a[1] * b[2] - a[2] * b[1];
        cy = a[2] * b[0] - a[0] * b[2];
        cz = a[0] * b[1] - a[1] * b[0];
        (void)face;
        return sqrtf(cx * cx + cy * cy + cz * cz);
    }
}

/* -------------------------------------------------------- sampling */

static void fetch_bilinear(const em_image *img, int face, float u01, float v01,
                           float out[3]) {
    int W = img->width, H = img->height, ch = img->channels, c;
    float fx = u01 * (float)W - 0.5f, fy = v01 * (float)H - 0.5f;
    int x0 = (int)floorf(fx), y0 = (int)floorf(fy);
    float tx = fx - (float)x0, ty = fy - (float)y0;
    int x1 = x0 + 1, y1 = y0 + 1, xi[2], yi[2], k;
    int wrap_u = (img->proj == EM_PROJ_EQUIRECT);
    xi[0] = x0; xi[1] = x1; yi[0] = y0; yi[1] = y1;
    for (k = 0; k < 2; ++k) {
        if (wrap_u) {
            int m = xi[k] % W;
            xi[k] = m < 0 ? m + W : m;
        } else {
            xi[k] = xi[k] < 0 ? 0 : (xi[k] >= W ? W - 1 : xi[k]);
        }
        yi[k] = yi[k] < 0 ? 0 : (yi[k] >= H ? H - 1 : yi[k]);
    }
    for (c = 0; c < ch && c < 3; ++c) {
        const float *p00 = em_image_texel(img, face, xi[0], yi[0]);
        const float *p10 = em_image_texel(img, face, xi[1], yi[0]);
        const float *p01 = em_image_texel(img, face, xi[0], yi[1]);
        const float *p11 = em_image_texel(img, face, xi[1], yi[1]);
        float top = p00[c] * (1 - tx) + p10[c] * tx;
        float bot = p01[c] * (1 - tx) + p11[c] * tx;
        out[c] = top * (1 - ty) + bot * ty;
    }
    for (; c < 3; ++c) out[c] = 0.0f;
}

void em_sample(const em_image *img, const float dir[3], float out_rgb[3]) {
    int face;
    float u, v;
    em_dir_to_uv(img->proj, dir, &face, &u, &v);
    fetch_bilinear(img, face, u, v, out_rgb);
}

em_result em_convert(const tir_allocator *a, const em_image *src,
                     em_proj dst_proj, int dst_size, em_image *out) {
    int W, H, face, x, y;
    em_result r;
    if (!src || !out || dst_size < 1) return EM_ERROR_INVALID_ARGUMENT;
    if (dst_proj == EM_PROJ_EQUIRECT) { W = dst_size; H = dst_size / 2; if (H < 1) H = 1; }
    else { W = dst_size; H = dst_size; }
    r = em_image_alloc(a, out, dst_proj, W, H, 3);
    if (!EM_OK(r)) return r;
    for (face = 0; face < out->faces; ++face)
        for (y = 0; y < H; ++y)
            for (x = 0; x < W; ++x) {
                float u = ((float)x + 0.5f) / (float)W;
                float v = ((float)y + 0.5f) / (float)H;
                float dir[3], rgb[3];
                float *t = em_image_texel(out, face, x, y);
                em_uv_to_dir(dst_proj, face, u, v, dir);
                em_sample(src, dir, rgb);
                t[0] = rgb[0]; t[1] = rgb[1]; t[2] = rgb[2];
            }
    return EM_SUCCESS;
}

/* -------------------------------------------------------- texel iterator */

void em_foreach_texel(const em_image *src, em_texel_fn cb, void *user) {
    int face, x, y;
    if (!src || !cb) return;
    for (face = 0; face < src->faces; ++face)
        for (y = 0; y < src->height; ++y)
            for (x = 0; x < src->width; ++x) {
                float u = ((float)x + 0.5f) / (float)src->width;
                float v = ((float)y + 0.5f) / (float)src->height;
                float dir[3];
                const float *t = em_image_texel(src, face, x, y);
                float sa = em_texel_solid_angle(src->proj, src->width, src->height,
                                                face, x, y);
                em_uv_to_dir(src->proj, face, u, v, dir);
                cb(dir, t, sa, user);
            }
}
