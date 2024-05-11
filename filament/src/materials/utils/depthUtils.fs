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

#ifndef FILAMENT_MATERIALS_DEPTH_UTILS
#define FILAMENT_MATERIALS_DEPTH_UTILS

highp float linearizeDepth(highp float depth) {
    // Our far plane is at infinity, which causes a division by zero below, which in turn
    // causes some issues on some GPU. We workaround it by replacing "infinity" by the closest
    // value representable in  a 24 bit depth buffer.
    const highp float preventDiv0 = 1.0 / 16777216.0;
    mat4 p = getViewFromClipMatrix();
    // this works with perspective and ortho projections, for a perspective projection
    // this resolves to -near/depth, for an ortho projection this resolves to depth*(far - near) - far
    return (depth * p[2].z + p[3].z) / max(depth * p[2].w + p[3].w, preventDiv0);
}

highp float sampleDepth(const highp sampler2D depthTexture, const highp vec2 uv, float lod) {
    return textureLod(depthTexture, uvToRenderTargetUV(uv), lod).r;
}

highp float sampleDepthLinear(const highp sampler2D depthTexture,
        const highp vec2 uv, float lod) {
    return linearizeDepth(sampleDepth(depthTexture, uv, lod));
}

#endif // #define FILAMENT_MATERIALS_DEPTH_UTILS

