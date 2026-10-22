/*
 * Copyright (C) 2026 The Android Open Source Project
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

void postProcess(inout PostProcessInputs postProcess) {

    int layer = materialParams.layer;

    // The starting coordinate and dimensions of the SOURCE mip level
    highp ivec2 srcAtlasStart = materialParams.srcRect.xy;
    highp ivec2 srcAtlasLength = materialParams.srcRect.zw;

    // The starting coordinate of the DESTINATION mip level
    highp ivec2 destAtlasStart = materialParams.dstRect.xy;

    // 1. Reconstruct the inclusive max bound for the source mip
    highp ivec2 srcAtlasMax = srcAtlasStart + srcAtlasLength - ivec2(1);

    // 2. Map the destination pixel back to the source atlas space
    highp ivec2 destCoord = ivec2(gl_FragCoord.xy);
    highp ivec2 localDestCoord = destCoord - destAtlasStart;

    // The top-left pixel of the 2x2 block in the source mip
    ivec2 srcTopLeft = srcAtlasStart + (localDestCoord << 1);

    // 3. Define the normalized 1D Gaussian weights (Sigma = 1.0)
    const highp vec4 weights = vec4(0.1345, 0.3655, 0.3655, 0.1345);

    highp vec4 result = vec4(0.0);

    // 4. Convolve the 4x4 symmetric grid
    // The compiler will fully unroll this 16-tap loop
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            // Offset from the top-left of the 2x2 block (-1, 0, 1, 2)
            ivec2 offset = ivec2(x - 1, y - 1);

            // Clamp strictly to the source atlas sub-rectangle bounds
            highp ivec2 fetchCoord = clamp(srcTopLeft + offset, srcAtlasStart, srcAtlasMax);

            // The 2D Gaussian weight is the product of the 1D weights
            highp float w = weights[x] * weights[y];

            // Pre-scaled Fused Multiply-Add guarantees FP16 safety against +Inf overflow
            result += texelFetch(materialParams_color, ivec3(fetchCoord, layer), 0) * w;
        }
    }

    postProcess.color = result;
}
