/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <filament/Frustum.h>

#include "Culler.h"

#include <utils/compiler.h>
#include <utils/Log.h>

using namespace filament::math;

namespace filament {

Frustum::Frustum(const mat4f& pv) {
    Frustum::setProjection(pv);
}

UTILS_NOINLINE
void Frustum::setProjection(const mat4f& pv) {
    const mat4f m(transpose(pv));

    // Calculate the unnormalized plane vectors
    float4 planes[6];
    planes[0] = -m[3] - m[0];
    planes[1] = -m[3] + m[0];
    planes[2] = -m[3] - m[1];
    planes[3] = -m[3] + m[1];
    planes[4] = -m[3] - m[2];
    planes[5] = -m[3] + m[2];

    // Normalize the plane vectors
    for (int i = 0; i < 6; ++i) {
        float invLength = 1 / length(planes[i].xyz);
        planes[i] *= invLength;
    }

    // Copy the normalized plane vectors to the class member array
    std::copy(std::begin(planes), std::end(planes), std::begin(mPlanes));
}

float4 Frustum::getNormalizedPlane(Frustum::Plane plane) const noexcept {
    return mPlanes[size_t(plane)];
}

void Frustum::getNormalizedPlanes(float4 planes[6]) const noexcept {
    // Copy the normalized plane vectors to the output array
    std::copy(std::begin(mPlanes), std::end(mPlanes), std::begin(planes));
}

bool Frustum::intersects(const Box& box) const noexcept {
    return Culler::intersects(*this, box);
}

bool Frustum::intersects(const float4& sphere) const noexcept {
    return Culler::intersects(*this, sphere);
}

float Frustum::contains(float3 p) const noexcept {
    float d = dot(mPlanes[0], p);

    // Loop through the remaining planes and find the maximum distance
    for (int i = 1; i < 6; ++i) {
        d = std::max(d, dot(mPlanes[i], p));
    }

    return d;
}

} // namespace filament

#if !defined(NDEBUG)

utils::io::ostream& operator<<(utils::io::ostream& out, filament::Frustum const& frustum) {
    float4 planes[6];
    frustum.getNormalizedPlanes(planes);

    // Output the normalized plane vectors
    out << planes[0] << '\n'
        << planes[1] << '\n'
        << planes[2] << '\n'
        << planes[3] << '\n'
        << planes[4] << '\n'
        << planes[5] << utils::io::endl;

    return out;
}

#endif
