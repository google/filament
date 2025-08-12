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
#define SECTOR_COUNT 32u

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

uint bitCount_(uint value) {
    value = value - ((value >> 1u) & 0x55555555u);
    value = (value & 0x33333333u) + ((value >> 2u) & 0x33333333u);
    return ((value + (value >> 4u) & 0xF0F0F0Fu) * 0x1010101u) >> 24u;
}

// If the new sample value is greater then the current one, update the value with some fallOff.
// Otherwise, apply thicknessHeuristic.
float updateHorizon(float sampleHorizonCos, float currentHorizonCos, float fallOff) {
    return sampleHorizonCos > currentHorizonCos
        ? mix(sampleHorizonCos, currentHorizonCos, fallOff)
        : mix(currentHorizonCos, sampleHorizonCos, materialParams.thicknessHeuristic);
}

float calculateHorizonCos(highp vec3 sampleDelta, highp vec3 viewDir, float horizonCos) {
    highp float sqSampleDist = dot(sampleDelta, sampleDelta);
    float invSampleDist = rsqrt(sqSampleDist);

    // Use the view space radius to calculate the fallOff
    float fallOff = saturate(sqSampleDist * materialParams.invRadiusSquared * 2.0);

    // sample horizon cos
    float shc = dot(sampleDelta, viewDir) * invSampleDist;

    return updateHorizon(shc, horizonCos, fallOff);
}

uint updateSectors(float minHorizon, float maxHorizon, uint globalOccludedBitfield) {
    uint startHorizonInt = uint(minHorizon * float(SECTOR_COUNT));
    uint angleHorizonInt = uint(ceil(saturate(maxHorizon-minHorizon) * float(SECTOR_COUNT)));
    if (angleHorizonInt >= SECTOR_COUNT) {
        return 0xFFFFFFFFu;
    }

    uint angleHorizonBitfield = (1u << angleHorizonInt) - 1u;
    uint currentOccludedBitfield = angleHorizonBitfield << startHorizonInt;
    return globalOccludedBitfield | currentOccludedBitfield;
}

uint calculateVisibilityMask(highp vec3 deltaPos, highp vec3 viewDir, float samplingDirection,
    uint globalOccludedBitfield, float n, highp vec3 origin) {
    vec2 frontBackHorizon;
    float linearThicknessMultiplier = materialParams.thickness > 0.0 ? saturate(origin.z * materialParams.invFarPlane) * 100.0 : 1.0;
    vec3 deltaPosBackface = deltaPos - viewDir * materialParams.thickness * linearThicknessMultiplier;

    // Project sample onto the unit circle and compute the angle relative to V
    frontBackHorizon.x = dot(normalize(deltaPos), viewDir);
    frontBackHorizon.y = dot(normalize(deltaPosBackface), viewDir);

    frontBackHorizon.x = acosFast(frontBackHorizon.x);
    frontBackHorizon.y = acosFast(frontBackHorizon.y);

    // Shift sample from V to normal, clamp in [0..1]
    frontBackHorizon = clamp(((samplingDirection * -frontBackHorizon) + n + HALF_PI) / PI, 0.0, 1.0);

    // Sampling direction inverts min/max angles
    frontBackHorizon = samplingDirection >= 0.0 ? frontBackHorizon.yx : frontBackHorizon.xy;

    return updateSectors(frontBackHorizon.x, frontBackHorizon.y, globalOccludedBitfield);
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
    for (float i = 0.0; i < materialParams.sliceCount.x; i += 1.0) {
        float slice = (i + noiseDirection) * materialParams.sliceCount.y;
        float phi = slice * PI;
        float cosPhi = cos(phi);
        float sinPhi = sin(phi);
        vec2 omega = vec2(cosPhi, sinPhi);

        // Calculate the direction of the current slice
        vec3 direction = vec3(cosPhi, sinPhi, 0.0);
        // Project direction onto the plane orthogonal to viewDir.
        vec3 orthoDirection = normalize(direction - (dot(direction, viewDir)*viewDir));
        // `axis` is orthogonal to direction and viewDir (basically the normal of the slice plane)
        // Used to define projectedNormal
        vec3 axis = cross(orthoDirection, viewDir);
        // `projNormal` is the normal projected onto the slice plane
        vec3 projNormal = normal - axis * dot(normal, axis);

        float signNorm = sign(dot(orthoDirection, projNormal));
        float projNormalLength = length(projNormal);
        float cosNorm = saturate(dot(projNormal, viewDir) / projNormalLength);

        // `n` is the signed angle between projNormal and viewDir
        float n = signNorm * acosFast(cosNorm);

        // These are used under non-bitmask mode or bitmask + bentNormal mode
        float horizonCos0 = -1.0;
        float horizonCos1 = -1.0;

        // This is only used in bitmask mode
        uint globalOccludedBitfield = 0u;

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

            if (materialConstants_useVisibilityBitmasks) {
                globalOccludedBitfield = calculateVisibilityMask(sampleDelta0, viewDir, 1.0, globalOccludedBitfield, n, origin);
                globalOccludedBitfield = calculateVisibilityMask(sampleDelta1, viewDir, -1.0, globalOccludedBitfield, n, origin);
            }
            else {
                horizonCos0 = calculateHorizonCos(sampleDelta0, viewDir, horizonCos0);
                horizonCos1 = calculateHorizonCos(sampleDelta1, viewDir, horizonCos1);
            }
        }

        if (materialConstants_useVisibilityBitmasks) {
            // Calculate the portion of the occluded sector
            visibility += float(bitCount_(globalOccludedBitfield)) / float(SECTOR_COUNT);

            // TODO: Calculate bent normal
        }
        else {
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
    }
    if (materialConstants_useVisibilityBitmasks) {
        obscurance = saturate(visibility * materialParams.sliceCount.y);
    }
    else{
        obscurance = 1.0 - saturate(visibility * materialParams.sliceCount.y);
    }

#if COMPUTE_BENT_NORMAL
    bentNormal = normalize(bentNormal);
#endif
}