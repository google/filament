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
    int radius = materialParams.radius;
    ivec2 dir = materialParams.dir;
    highp int atlasMin = materialParams.bounds.x;
    highp int atlasMax = materialParams.bounds.y;

    // 1. Get the exact integer coordinate of the current pixel
    highp ivec2 centerCoord = ivec2(gl_FragCoord.xy);

    // 2. Fetch the center pixel and pre-scale it (Safe FP16 FMA)
    highp ivec2 clampedCenter = clamp(centerCoord, atlasMin, atlasMax);
    highp vec4 result = texelFetch(materialParams_input, ivec3(clampedCenter, layer), 0) * materialParams.kernel[0];

    // 3. Accumulate the positive and negative offsets
    // Using a constant loop bound allows the shader compiler to perfectly unroll this.
    for (int i = 1; i <= radius; ++i) {
        ivec2 offset = dir * i;

        // Clamp the fetch coordinates strictly to the Atlas sub-rectangle boundary
        highp ivec2 posCoord = clamp(centerCoord + offset, atlasMin, atlasMax);
        highp ivec2 negCoord = clamp(centerCoord - offset, atlasMin, atlasMax);

        // Pre-scaled accumulation to strictly prevent FP16 +Inf overflow
        highp float w = materialParams.kernel[i];
        result += texelFetch(materialParams_input, ivec3(posCoord, layer), 0) * w;
        result += texelFetch(materialParams_input, ivec3(negCoord, layer), 0) * w;
    }

    postProcess.color = result;
}
