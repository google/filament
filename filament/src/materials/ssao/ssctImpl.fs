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

#include "ssct.fs"

float dominantLightShadowing(highp vec2 uv, highp vec3 origin, vec3 normal) {
    ConeTraceSetup cone;

    cone.ssStartPos = uv * materialParams.resolution.xy;
    cone.vsStartPos = origin;
    cone.vsNormal = normal;

    cone.vsConeDirection = materialParams.ssctVsLightDirection;
    cone.shadowDistance = materialParams.ssctShadowDistance;
    cone.coneAngleTangeant = materialParams.ssctConeAngleTangeant;
    cone.contactDistanceMaxInv = materialParams.ssctContactDistanceMaxInv;

    cone.screenFromViewMatrix = materialParams.screenFromViewMatrix;
    cone.projectionScale = materialParams.projectionScale;
    cone.resolution = materialParams.resolution;
    cone.maxLevel = float(materialParams.maxLevel);

    cone.intensity = materialParams.ssctIntensity;
    cone.depthBias = materialParams.ssctDepthBias.x;
    cone.slopeScaledDepthBias = materialParams.ssctDepthBias.y;
    cone.sampleCount = materialParams.ssctSampleCount;

    return ssctDominantLightShadowing(uv, origin, normal,
            materialParams_depth, getFragCoord(materialParams.resolution.xy),
            materialParams.ssctRayCount, cone);
}
