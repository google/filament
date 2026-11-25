/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FILAMENT_MATERIALS_SSAO_UTILS
#define FILAMENT_MATERIALS_SSAO_UTILS

#include "../utils/depthUtils.fs"

vec2 pack(highp float normalizedDepth) {
    // we need 16-bits of precision
    highp float z = clamp(normalizedDepth, 0.0, 1.0);
    highp float t = floor(256.0 * z);
    mediump float hi = t * (1.0 / 256.0);   // we only need 8-bits of precision
    mediump float lo = (256.0 * z) - t;     // we only need 8-bits of precision
    return vec2(hi, lo);
}

highp float unpack(highp vec2 depth) {
    // depth here only has 8-bits of precision, but the unpacked depth is highp
    // this is equivalent to (x8 * 256 + y8) / 65535, which gives a value between 0 and 1
    return (depth.x * (256.0 / 257.0) + depth.y * (1.0 / 257.0));
}

// returns the uv of the level-0 texel that the depth sampled at uv comes from
highp vec2 depthTexelUV(const highp sampler2D depthTexture, const highp vec2 uv, int lod) {
    highp vec2 st = uvToRenderTargetUV(uv);
    ivec2 size = textureSize(depthTexture, lod);
    ivec2 texel = clamp(ivec2(floor(st * vec2(size))), ivec2(0), size - 1);
    // follow mipmapDepth.mat's rotated grid sub-sample down to level 0
    for (int i = 0; i < lod; i++) {
        texel = 2 * texel + ivec2(texel.y & 1, texel.x & 1);
    }
    return uvToRenderTargetUV((vec2(texel) + 0.5) / vec2(textureSize(depthTexture, 0)));
}

vec3 packBentNormal(vec3 bn) {
    return bn * 0.5 + 0.5;
}

vec3 unpackBentNormal(vec3 bn) {
    return bn * 2.0 - 1.0;
}


#endif // FILAMENT_MATERIALS_SSAO_UTILS

