/*
 * Copyright (C) 2025 The Android Open Source Project
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

/*
 * This is our implementation of SAO -- it's not standalone because it uses materialParams
 * directly. Therefore it must be included in *.mat file that has all these parameters.
 * The main reason for using a separate file is to be able to have several version of the
 * code with only minor changes.
 */

#include "../utils/geometry.fs"

const float kLog2LodRate = 3.0;

float integrateArcCosWeight(float h, float n) {
    float Arc = -cos(2.0 * h - n) + cos(n) + 2.0 * h * sin(n);
    return 0.25 * Arc;
}

float spatialDirectionNoise(float2 uv) {
    int2 position = int2(uv * materialParams.resolution.xy);
	return (1.0/16.0) * (float(((position.x + position.y) & 3) << 2) + float(position.x & 3));
}

float spatialOffsetsNoise(float2 uv) {
	int2 position = int2(uv * materialParams.resolution.xy);
	return 0.25 * float((position.y - position.x) & 3);
}

float fastSqrt(float x) {
    return intBitsToFloat(0x1fbd1df5 + (floatBitsToInt(x) >> 1));
}

float fastACos(float inX) {
    float x = abs(inX);
    float res = -0.156583 * x + HALF_PI;
    res *= fastSqrt(1.0 - x);
    return (inX >= 0.0) ? res : PI - res;
}

void groundTruthAmbientOcclusion(out float obscurance, out vec3 bentNormal,
        highp vec2 uv, highp vec3 origin, vec3 normal) {
    vec2 uvSamplePos = uv;
    highp vec3 viewDir = normalize(-origin);
    highp float ssRadius = -(materialParams.projectionScaleRadius / origin.z);
    ssRadius = max(min(ssRadius, 512.0), materialParams.sliceCount);

    float noiseOffset = spatialOffsetsNoise(uv);
    float noiseDirection = spatialDirectionNoise(uv);

    float initialRayStep = fract(noiseOffset);

    float visibility = 0.0;
    float stepRadius = ssRadius / (materialParams.stepsPerSlice + 1.0);
    for (float i = 0.0; i < materialParams.sliceCount; i += 1.0) {
        float slice = (i + noiseDirection) / materialParams.sliceCount;
        float phi = slice * PI;
        float cosPhi = cos(phi);
        float sinPhi = sin(phi);
        vec2 omega = vec2(cosPhi, sinPhi);

        vec3 directionV = vec3(cosPhi, sinPhi, 0.0);
        vec3 orthoDirectionV = directionV - (dot(directionV, viewDir)*viewDir);
        vec3 axisV = normalize(cross(orthoDirectionV, viewDir));
        vec3 projNormalV = normal - axisV * dot(normal, axisV);

        float signNorm = sign(dot(orthoDirectionV, projNormalV));
        float projNormalLength = length(projNormalV);
        float cosNorm = saturate(dot(projNormalV, viewDir) / projNormalLength);

        float n = signNorm * fastACos(cosNorm);

        float horizonCos0 = -1.0;
        float horizonCos1 = -1.0;
        for (float j = 0.0; j < materialParams.stepsPerSlice; j += 1.0) {
            // At least move 1 pixel forward in the screen-space
            vec2 sampleOffset = max((j + initialRayStep)*stepRadius, 1.0 + j) * omega;
            float sampleOffsetLength = length(sampleOffset);

            float level = clamp(floor(log2(sampleOffsetLength)) - kLog2LodRate, 0.0, float(materialParams.maxLevel));

            sampleOffset *= materialParams.resolution.zw;

            vec2 sampleScreenPos0 = uv + sampleOffset;
            highp float sampleDepth0 = sampleDepthLinear(materialParams_depth, sampleScreenPos0, level);
            highp vec3 samplePos0 = computeViewSpacePositionFromDepth(sampleScreenPos0, sampleDepth0,
                materialParams.positionParams);

            float2 sampleScreenPos1 = uv - sampleOffset;
            highp float sampleDepth1 = sampleDepthLinear(materialParams_depth, sampleScreenPos1, level);
            highp vec3 samplePos1 = computeViewSpacePositionFromDepth(sampleScreenPos1, sampleDepth1,
                materialParams.positionParams);

            highp vec3 sampleDelta0 = (samplePos0 - origin);
            highp vec3 sampleDelta1 = (samplePos1 - origin);
            float sampleDist0 = length(sampleDelta0);
            float sampleDist1 = length(sampleDelta1);

            vec3 sampleHorizonV0 = sampleDelta0/sampleDist0;
            vec3 sampleHorizonV1 = sampleDelta1/sampleDist1;

            float wsRadius = materialParams.radius;
            vec2 fallOff = saturate(float2(sampleDist0*sampleDist0, sampleDist1*sampleDist1) * (2.0/(wsRadius*wsRadius)));

            float shc0 = dot(sampleHorizonV0, viewDir);
            float shc1 = dot(sampleHorizonV1, viewDir);

            horizonCos0 = shc0 > horizonCos0 ? mix(shc0, horizonCos0, fallOff.x) : mix(horizonCos0, shc0, materialParams.thicknessHeuristic);
            horizonCos1 = shc1 > horizonCos1 ? mix(shc1, horizonCos1, fallOff.y) : mix(horizonCos1, shc1, materialParams.thicknessHeuristic);
        }

        float h0 = -fastACos(horizonCos1);
        float h1 = fastACos(horizonCos0);
        h0 = n + clamp(h0-n, -HALF_PI, HALF_PI);
        h1 = n + clamp(h1-n, -HALF_PI, HALF_PI);

        float angle = 0.5 * (h0 + h1);
        bentNormal += viewDir * cos(angle) - axisV * sin(angle);

        visibility += projNormalLength * (integrateArcCosWeight(h0, n) + integrateArcCosWeight(h1, n));
    }

    obscurance = 1.0 - saturate(visibility / materialParams.sliceCount);

    if (materialConstants_bentNormals) {
        bentNormal = normalize(bentNormal);
    }
}