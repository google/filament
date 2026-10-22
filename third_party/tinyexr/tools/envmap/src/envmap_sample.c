/*
 * TinyEXR envmap - low-discrepancy + hemisphere/sphere sampling helpers.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "envmap.h"

#include <math.h>

#ifndef EM_PI
#define EM_PI 3.14159265358979323846f
#endif

/* Radical inverse base 2 (van der Corput). */
static float radical_inverse_vdc(uint32_t bits) {
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x55555555u) << 1) | ((bits & 0xAAAAAAAAu) >> 1);
    bits = ((bits & 0x33333333u) << 2) | ((bits & 0xCCCCCCCCu) >> 2);
    bits = ((bits & 0x0F0F0F0Fu) << 4) | ((bits & 0xF0F0F0F0u) >> 4);
    bits = ((bits & 0x00FF00FFu) << 8) | ((bits & 0xFF00FF00u) >> 8);
    return (float)((double)bits * 2.3283064365386963e-10); /* / 2^32 */
}

void em_hammersley(uint32_t i, uint32_t n, float out_xy[2]) {
    out_xy[0] = (n > 0) ? (float)i / (float)n : 0.0f;
    out_xy[1] = radical_inverse_vdc(i);
}

void em_sample_cosine(const float xi[2], float out_dir[3]) {
    float r = sqrtf(xi[0]);
    float phi = 2.0f * EM_PI * xi[1];
    out_dir[0] = r * cosf(phi);
    out_dir[1] = r * sinf(phi);
    out_dir[2] = sqrtf(1.0f - xi[0] > 0.0f ? 1.0f - xi[0] : 0.0f);
}

void em_sample_uniform_sphere(const float xi[2], float out_dir[3]) {
    float z = 1.0f - 2.0f * xi[0];
    float r = sqrtf(1.0f - z * z > 0.0f ? 1.0f - z * z : 0.0f);
    float phi = 2.0f * EM_PI * xi[1];
    out_dir[0] = r * cosf(phi);
    out_dir[1] = r * sinf(phi);
    out_dir[2] = z;
}
