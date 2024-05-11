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

#ifndef FILAMENT_MATERIALS_GEOMETRY
#define FILAMENT_MATERIALS_GEOMETRY

#include "depthUtils.fs"

// uv             : normalized coordinates
// linearDepth    : linear depth at uv
// positionParams : invProjection[0][0] * 2, invProjection[1][1] * 2
//
highp vec3 computeViewSpacePositionFromDepth(highp vec2 uv, highp float linearDepth,
        highp vec2 positionParams) {
    return vec3((0.5 - uv) * positionParams * linearDepth, linearDepth);
}

highp vec3 faceNormal(highp vec3 dpdx, highp vec3 dpdy) {
    return normalize(cross(dpdx, dpdy));
}

// Compute normals using derivatives, which essentially results in half-resolution normals
// this creates arifacts around geometry edges.
// Note: when using the spirv optimizer, this results in much slower execution time because
//       this whole expression is inlined in the AO loop below.
highp vec3 computeViewSpaceNormalLowQ(const highp vec3 position) {
    return faceNormal(dFdx(position), dFdy(position));
}

// Compute normals directly from the depth texture, resulting in full resolution normals
// Note: This is actually as cheap as using derivatives because the texture fetches
//       are essentially equivalent to textureGather (which we don't have on ES3.0),
//       and this is executed just once.
//
// depthTexture   : the depth texture in reversed-Z
// uv             : normalized coordinates
// position       : view space position at uv
// texel          : 1/depth_width, 1/depth_height
// positionParams : invProjection[0][0] * 2, invProjection[1][1] * 2
//
highp vec3 computeViewSpaceNormalMediumQ(
        const highp sampler2D depthTexture, const highp vec2 uv,
        const highp vec3 position,
        highp vec2 texel, highp vec2 positionParams) {

    precision highp float;

    highp vec2 uvdx = uv + vec2(texel.x, 0.0);
    highp vec2 uvdy = uv + vec2(0.0, texel.y);
    vec3 px = computeViewSpacePositionFromDepth(uvdx,
            sampleDepthLinear(depthTexture, uvdx, 0.0), positionParams);
    vec3 py = computeViewSpacePositionFromDepth(uvdy,
            sampleDepthLinear(depthTexture, uvdy, 0.0), positionParams);
    vec3 dpdx = px - position;
    vec3 dpdy = py - position;
    return faceNormal(dpdx, dpdy);
}

// Accurate view-space normal reconstruction
// Based on Yuwen Wu "Accurate Normal Reconstruction"
// (https://atyuwen.github.io/posts/normal-reconstruction)
//
// depthTexture   : the depth texture in reversed-Z
// uv             : normalized coordinates
// depth          : linear depth at uv
// position       : view space position at uv
// texel          : 1/depth_width, 1/depth_height
// positionParams : invProjection[0][0] * 2, invProjection[1][1] * 2
//
highp vec3 computeViewSpaceNormalHighQ(
        const highp sampler2D depthTexture, const highp vec2 uv,
        const highp float depth, const highp vec3 position,
        highp vec2 texel, highp vec2 positionParams) {

    precision highp float;

    vec3 pos_c = position;
    highp vec2 dx = vec2(texel.x, 0.0);
    highp vec2 dy = vec2(0.0, texel.y);

    vec4 H;
    H.x = sampleDepth(depthTexture, uv - dx, 0.0);
    H.y = sampleDepth(depthTexture, uv + dx, 0.0);
    H.z = sampleDepth(depthTexture, uv - dx * 2.0, 0.0);
    H.w = sampleDepth(depthTexture, uv + dx * 2.0, 0.0);
    vec2 he = abs((2.0 * H.xy - H.zw) - depth);
    vec3 pos_l = computeViewSpacePositionFromDepth(uv - dx,
            linearizeDepth(H.x), positionParams);
    vec3 pos_r = computeViewSpacePositionFromDepth(uv + dx,
            linearizeDepth(H.y), positionParams);
    vec3 dpdx = (he.x < he.y) ? (pos_c - pos_l) : (pos_r - pos_c);

    vec4 V;
    V.x = sampleDepth(depthTexture, uv - dy, 0.0);
    V.y = sampleDepth(depthTexture, uv + dy, 0.0);
    V.z = sampleDepth(depthTexture, uv - dy * 2.0, 0.0);
    V.w = sampleDepth(depthTexture, uv + dy * 2.0, 0.0);
    vec2 ve = abs((2.0 * V.xy - V.zw) - depth);
    vec3 pos_d = computeViewSpacePositionFromDepth(uv - dy,
            linearizeDepth(V.x), positionParams);
    vec3 pos_u = computeViewSpacePositionFromDepth(uv + dy,
            linearizeDepth(V.y), positionParams);
    vec3 dpdy = (ve.x < ve.y) ? (pos_c - pos_d) : (pos_u - pos_c);
    return faceNormal(dpdx, dpdy);
}

// depthTexture   : the depth texture in reversed-Z
// uv             : normalized coordinates
// depth          : linear depth at uv
// position       : view space position at uv
// texel          : 1/depth_width, 1/depth_height
// positionParams : invProjection[0][0] * 2, invProjection[1][1] * 2
//
highp vec3 computeViewSpaceNormal(
        const highp sampler2D depthTexture, const highp vec2 uv,
        const highp float depth, const highp vec3 position,
        highp vec2 texel, highp vec2 positionParams) {
    // todo: maybe make this a quality parameter
#if FILAMENT_QUALITY == FILAMENT_QUALITY_HIGH
    vec3 normal = computeViewSpaceNormalHighQ(depthTexture, uv, depth, position,
            texel, positionParams);
#else
    vec3 normal = computeViewSpaceNormalMediumQ(depthTexture, uv, position,
            texel, positionParams);
#endif
    return normal;
}

#endif // FILAMENT_MATERIALS_GEOMETRY
