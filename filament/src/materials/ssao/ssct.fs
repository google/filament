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

#ifndef FILAMENT_MATERIALS_SSCT
#define FILAMENT_MATERIALS_SSCT

/*
 * Largely based on "Dominant Light Shadowing"
 * "Lighting Technology of The Last of Us Part II" by Hawar Doghramachi, Naughty Dog, LLC
 */

#include "ssaoUtils.fs"

const float kSSCTLog2LodRate = 3.0;

struct ConeTraceSetup {
    // fragment info
    highp vec2 ssStartPos;
    highp vec3 vsStartPos;
    vec3 vsNormal;

    // light (cone) info
    vec3 vsConeDirection;
    float shadowDistance;
    float coneAngleTangeant;
    float contactDistanceMaxInv;
    vec2 jitterOffset;          // (x = direction offset, y = step offset)

    // scene infos
    highp mat4 screenFromViewMatrix;
    float projectionScale;
    vec4 resolution;
    float maxLevel;

    // artistic/quality parameters
    float intensity;
    float depthBias;
    float slopeScaledDepthBias;
    uint sampleCount;
};

highp float getWFromProjectionMatrix(const highp mat4 p, const vec3 v) {
    // this essentially returns (p * vec4(v, 1.0)).w, but we make some assumptions
    // this assumes a perspective projection
    return -v.z;
    // this assumes a perspective or ortho projection
    //return p[2][3] * v.z + p[3][3];
}

highp float getViewSpaceZFromW(const highp mat4 p, const float w) {
    // this assumes a perspective projection
    return -w;
    // this assumes a perspective or ortho projection
    //return (w - p[3][3]) / p[2][3];
}

float coneTraceOcclusion(in ConeTraceSetup setup, const highp sampler2D depthTexture) {
    // skip fragments that are back-facing trace direction
    // (avoid overshadowing of translucent surfaces)
    float NoL = dot(setup.vsNormal, setup.vsConeDirection);
    if (NoL < 0.0) {
        return 0.0;
    }

    // start position of cone trace
    highp vec2 ssStartPos = setup.ssStartPos;
    highp vec3 vsStartPos = setup.vsStartPos;
    highp float ssStartPosW = getWFromProjectionMatrix(setup.screenFromViewMatrix, vsStartPos);
    highp float ssStartPosWInv = 1.0 / ssStartPosW;

    // end position of cone trace
    highp vec3 vsEndPos = setup.vsConeDirection * setup.shadowDistance + vsStartPos;
    highp float ssEndPosW = getWFromProjectionMatrix(setup.screenFromViewMatrix, vsEndPos);
    highp float ssEndPosWInv = 1.0 / ssEndPosW;
    highp vec2 ssEndPos = (setup.screenFromViewMatrix * vec4(vsEndPos, 1.0)).xy * ssEndPosWInv;

    // cone trace direction in screen-space
    float ssConeLength = length(ssEndPos - ssStartPos);     // do the math in highp
    highp vec2 ssConeVector = ssEndPos - ssStartPos;

    // direction perpendicular to cone trace direction
    vec2 perpConeDir = normalize(vec2(ssConeVector.y, -ssConeVector.x));
    float vsEndRadius = setup.coneAngleTangeant * setup.shadowDistance;

    // normalized step
    highp float dt = 1.0 / float(setup.sampleCount);

    // normalized (0 to 1) screen-space postion on the ray
    highp float t = dt * setup.jitterOffset.y;

    // calculate depth bias
    float vsDepthBias = saturate(1.0 - NoL) * setup.slopeScaledDepthBias + setup.depthBias;

    float occlusion = 0.0;
    for (uint i = 0u; i < setup.sampleCount; i++, t += dt) {
        float ssTracedDistance = ssConeLength * t;
        float ssSliceRadius = setup.jitterOffset.x * (setup.coneAngleTangeant * ssTracedDistance);
        highp vec2 ssSamplePos = perpConeDir * ssSliceRadius + ssConeVector * t + ssStartPos;

        float level = clamp(floor(log2(ssSliceRadius)) - kSSCTLog2LodRate, 0.0, float(setup.maxLevel));
        float vsSampleDepthLinear = -sampleDepthLinear(depthTexture, ssSamplePos * setup.resolution.zw, 0.0);

        // calculate depth range of cone slice
        float vsSliceRadius = vsEndRadius * t;

        // calculate depth of cone center
        float vsConeAxisDepth = -getViewSpaceZFromW(setup.screenFromViewMatrix, 1.0 / mix(ssStartPosWInv, ssEndPosWInv, t));
        float vsJitteredSampleRadius = vsSliceRadius * setup.jitterOffset.x;
        float vsSliceHalfRange = sqrt(vsSliceRadius * vsSliceRadius - vsJitteredSampleRadius * vsJitteredSampleRadius);
        float vsSampleDepthMax = vsConeAxisDepth + vsSliceHalfRange;

        // calculate overlap of depth buffer height-field with trace cone
        float vsDepthDifference = vsSampleDepthMax - vsSampleDepthLinear;
        float overlap = saturate((vsDepthDifference - vsDepthBias) / (vsSliceHalfRange * 2.0));

        // attenuate by distance to avoid false occlusion
        float attenuation = saturate(1.0 - (vsDepthDifference * setup.contactDistanceMaxInv));
        occlusion = max(occlusion, overlap * attenuation);
        if (occlusion >= 1.0) {  // note: this can't get > 1.0 by construction
            // fully occluded, early exit
            break;
        }
    }
    return occlusion * setup.intensity;
}

float ssctDominantLightShadowing(highp vec2 uv, highp vec3 origin, vec3 normal,
        const highp sampler2D depthTexture, const highp vec2 fragCoord,
        vec2 rayCount, ConeTraceSetup cone) {

    float occlusion = 0.0;
    for (float i = 1.0; i <= rayCount.x; i += 1.0) {
        cone.jitterOffset.x = interleavedGradientNoise(fragCoord * i) * 2.0 - 1.0;      // direction
        cone.jitterOffset.y = interleavedGradientNoise(fragCoord * i * vec2(3, 11));    // step
        occlusion += coneTraceOcclusion(cone, depthTexture);
    }
    return occlusion * rayCount.y;
}


#endif // FILAMENT_MATERIALS_SSCT
