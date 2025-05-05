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
 * This is our implementation of GTAO -- it's not standalone because it uses materialParams
 * directly. Therefore it must be included in *.mat file that has all these parameters.
 * The main reason for using a separate file is to be able to have several version of the
 * code with only minor changes.
 */

#include "../utils/geometry.fs"

#define rsqrt inversesqrt

#ifndef COMPUTE_BENT_NORMAL
#error COMPUTE_BENT_NORMAL must be set
#endif

const float kLog2LodRate = 3.0;

// Ambient Occlusion, largely inspired from:
// "Practical Real-Time Strategies for Accurate Indirect Occlusion" by Jimenez et al.
// https://github.com/GameTechDev/XeGTAO
// https://github.com/MaxwellGengYF/Unity-Ground-Truth-Ambient-Occlusion

highp vec3 getViewSpacePosition(vec2 uv, float level) {
    highp float depth = sampleDepthLinear(materialParams_depth, uv, level);
    return computeViewSpacePositionFromDepth(uv, depth, materialParams.positionParams);
}

float integrateArcCosWeight(float h, float n) {
    float arc = -cos(2.0 * h - n) + cos(n) + 2.0 * h * sin(n);
    return 0.25 * arc;
}

// https://blog.selfshadow.com/publications/s2016-shading-course/activision/s2016_pbs_activision_occlusion.pdf slide 93
float spatialDirectionNoise(float2 uv) {
    int2 position = int2(uv * materialParams.resolution.xy);
	return (1.0/16.0) * (float(((position.x + position.y) & 3) << 2) + float(position.x & 3));
}

// https://blog.selfshadow.com/publications/s2016-shading-course/activision/s2016_pbs_activision_occlusion.pdf slide 93
float spatialOffsetsNoise(float2 uv) {
	int2 position = int2(uv * materialParams.resolution.xy);
	return 0.25 * float((position.y - position.x) & 3);
}

// http://h14s.p5r.org/2012/09/0x5f3759df.html, [Drobot2014a] Low Level Optimizations for GCN, https://blog.selfshadow.com/publications/s2016-shading-course/activision/s2016_pbs_activision_occlusion.pdf slide 63
float fastSqrt(float x) {
#if defined(TARGET_MOBILE)
    return sqrt(x);
#else
    return intBitsToFloat(0x1fbd1df5 + (floatBitsToInt(x) >> 1));
#endif
}

void groundTruthAmbientOcclusion(out float obscurance, out vec3 bentNormal,
        highp vec2 uv, highp vec3 origin, vec3 normal) {
    vec2 uvSamplePos = uv;
    highp vec3 viewDir = normalize(-origin);
    highp float ssRadius = -(materialParams.projectionScaleRadius / origin.z);

    float noiseOffset = spatialOffsetsNoise(uv);
    float noiseDirection = spatialDirectionNoise(uv);

    float initialRayStep = fract(noiseOffset);

    // The distance we want to move forward for each step
    float stepRadius = ssRadius / (materialParams.stepsPerSlice + 1.0);

    float visibility = 0.0;
    for (float i = 0.0; i < materialParams.sliceCount; i += 1.0) {
        float slice = (i + noiseDirection) / materialParams.sliceCount;
        float phi = slice * PI;
        float cosPhi = cos(phi);
        float sinPhi = sin(phi);
        vec2 omega = vec2(cosPhi, sinPhi);

        // Calculate the direction of the current slice
        vec3 direction = vec3(cosPhi, sinPhi, 0.0);
        // Project direction onto the plane orthogonal to viewDir.
        vec3 orthoDirection = normalize(direction - (dot(direction, viewDir)*viewDir));
        // axis is orthogonal to direction and viewDir (basically the normal of the slice plane)
        // Used to define projectedNormal
        vec3 axis = cross(orthoDirection, viewDir);
        // Project the normal onto the slice plane
        vec3 projNormal = normal - axis * dot(normal, axis);

        float signNorm = sign(dot(orthoDirection, projNormal));
        float projNormalLength = length(projNormal);
        float cosNorm = saturate(dot(projNormal, viewDir) / projNormalLength);

        float n = signNorm * acosFast(cosNorm);

        float horizonCos0 = -1.0;
        float horizonCos1 = -1.0;
        for (float j = 0.0; j < materialParams.stepsPerSlice; j += 1.0) {
            // At least move 1 pixel forward in the screen-space
            vec2 sampleOffset = max((j + initialRayStep)*stepRadius, 1.0 + j) * omega;
            float sampleOffsetLength = length(sampleOffset);

            float level = clamp(floor(log2(sampleOffsetLength)) - kLog2LodRate, 0.0, float(materialParams.maxLevel));

            vec2 uvSampleOffset = sampleOffset * materialParams.resolution.zw;

            vec2 sampleScreenPos0 = uv + uvSampleOffset;
            vec2 sampleScreenPos1 = uv - uvSampleOffset;

            // Sample the depth and use it to reconstruct the view space position
            highp vec3 samplePos0 = getViewSpacePosition(sampleScreenPos0, level);
            highp vec3 samplePos1 = getViewSpacePosition(sampleScreenPos1, level);

            highp vec3 sampleDelta0 = (samplePos0 - origin);
            highp vec3 sampleDelta1 = (samplePos1 - origin);
            vec2 sqSampleDist = vec2(dot(sampleDelta0, sampleDelta0), dot(sampleDelta1, sampleDelta1));
            vec2 invSampleDist = rsqrt(sqSampleDist);

            // Use the view space radius to calculate the fallOff
            vec2 fallOff = saturate(sqSampleDist.xy * (2.0/sq(materialParams.radius)));

            // sample horizon cos
            float shc0 = dot(sampleDelta0, viewDir) * invSampleDist.x;
            float shc1 = dot(sampleDelta1, viewDir) * invSampleDist.y;

            // If the new sample value is greater then the current one, update the value with some fallOff.
            // Otherwise, apply thicknessHeuristic.
            horizonCos0 = shc0 > horizonCos0 ? mix(shc0, horizonCos0, fallOff.x) : mix(horizonCos0, shc0, materialParams.thicknessHeuristic);
            horizonCos1 = shc1 > horizonCos1 ? mix(shc1, horizonCos1, fallOff.y) : mix(horizonCos1, shc1, materialParams.thicknessHeuristic);
        }

        float h0 = -acosFast(horizonCos1);
        float h1 = acosFast(horizonCos0);
        h0 = n + clamp(h0 - n, -HALF_PI, HALF_PI);
        h1 = n + clamp(h1 - n, -HALF_PI, HALF_PI);

#if COMPUTE_BENT_NORMAL
        float angle = 0.5 * (h0 + h1);
        bentNormal += viewDir * cos(angle) - orthoDirection * sin(angle);
#endif

        visibility += projNormalLength * (integrateArcCosWeight(h0, n) + integrateArcCosWeight(h1, n));
    }

    obscurance = 1.0 - saturate(visibility / materialParams.sliceCount);

#if COMPUTE_BENT_NORMAL
    bentNormal = normalize(bentNormal);
#endif
}