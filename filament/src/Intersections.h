/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_FILAMENT_INTERSECTIONS_H
#define TNT_FILAMENT_INTERSECTIONS_H

#include <utils/compiler.h>

#include <math/mat4.h>
#include <math/vec4.h>

namespace filament {

// sphere radius must be squared
// plane equation must be normalized, sphere radius must be squared
// return float4.w <= 0 if no intersection
inline constexpr math::float4 spherePlaneIntersection(math::float4 s, math::float4 p) noexcept {
    const float d = dot(s.xyz, p.xyz) + p.w;
    const float rr = s.w - d * d;
    s.x -= p.x * d;
    s.y -= p.y * d;
    s.z -= p.z * d;
    s.w = rr;   // new-circle/sphere radius is squared
    return s;
}

// sphere radius must be squared
// plane equation must be normalized and have the form {x,0,z,0}, sphere radius must be squared
// return float4.w <= 0 if no intersection
inline float spherePlaneDistanceSquared(math::float4 s, float px, float pz) noexcept {
    return spherePlaneIntersection(s, { px, 0.f, pz, 0.f }).w;
}

// sphere radius must be squared
// plane equation must be normalized and have the form {0,y,z,0}, sphere radius must be squared
// return float4.w <= 0 if no intersection
inline math::float4 spherePlaneIntersection(math::float4 s, float py, float pz) noexcept {
    return spherePlaneIntersection(s, { 0.f, py, pz, 0.f });
}

// sphere radius must be squared
// plane equation must be normalized and have the form {0,0,1,w}, sphere radius must be squared
// return float4.w <= 0 if no intersection
inline math::float4 spherePlaneIntersection(math::float4 s, float pw) noexcept {
    return spherePlaneIntersection(s, { 0.f, 0.f, 1.f, pw });
}

// sphere radius must be squared
// this version returns a false-positive intersection in a small area near the origin
// of the cone extended outward by the sphere's radius.
inline bool sphereConeIntersectionFast(
        math::float4 const& sphere,
        math::float3 const& conePosition,
        math::float3 const& coneAxis,
        float coneSinInverse,
        float coneCosSquared) noexcept {
    const math::float3 u = conePosition - (sphere.w * coneSinInverse) * coneAxis;
    math::float3 d = sphere.xyz - u;
    float e = dot(coneAxis, d);
    float dd = dot(d, d);
    // we do the e>0 last here to avoid a branch
    return (e * e >= dd * coneCosSquared && e > 0);
}

inline bool sphereConeIntersection(
        math::float4 const& sphere,
        math::float3 const& conePosition,
        math::float3 const& coneAxis,
        float coneSinInverse,
        float coneCosSquared) noexcept {
    if (sphereConeIntersectionFast(sphere,
            conePosition, coneAxis, coneSinInverse, coneCosSquared)) {
        math::float3 d = sphere.xyz - conePosition;
        float e = -dot(coneAxis, d);
        float dd = dot(d, d);
        if (e * e >= dd * (1 - coneCosSquared) && e > 0) {
            return dd <= sphere.w * sphere.w;
        }
        return true;
    }
    return false;
}

} // namespace filament

#endif //TNT_FILAMENT_INTERSECTIONS_H
