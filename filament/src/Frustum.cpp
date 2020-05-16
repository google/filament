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

#include "details/Culler.h"

#include <utils/compiler.h>

using namespace filament::math;

namespace filament {

Frustum::Frustum(const mat4f& pv) {
    Frustum::setProjection(pv);
}

Frustum::Frustum(const float3 corners[8]) {
    float3 a = corners[0];
    float3 b = corners[1];
    float3 c = corners[2];
    float3 d = corners[3];
    float3 e = corners[4];
    float3 f = corners[5];
    float3 g = corners[6];
    float3 h = corners[7];

    //     c----d
    //    /|   /|
    //   g----h |
    //   | a--|-b
    //   |/   |/
    //   e----f

    auto plane = [](float3 p1, float3 p2, float3 p3) {
        auto v12 = p2 - p1;
        auto v23 = p3 - p2;
        auto n = normalize(cross(v12, v23));
        return float4{n, -dot(n, p1)};
    };

    mPlanes[0] = plane(a, e, g);   // left
    mPlanes[1] = plane(f, b, d);   // right
    mPlanes[2] = plane(a, b, f);   // bottom
    mPlanes[3] = plane(g, h, d);   // top
    mPlanes[4] = plane(a, c, d);   // far
    mPlanes[5] = plane(e, f, h);   // near
}

// NOTE: if we don't specify noinline here, LLVM inlines this huge function into
// two (?!) version of the Frustum(const mat4f& pv) constructor!

UTILS_NOINLINE
void Frustum::setProjection(const mat4f& pv) {
    const mat4f m(transpose(pv));

    // Note: these "normals" are not normalized -- it's not necessary for the culling tests.
    float4 l = -m[3] - m[0];
    float4 r = -m[3] + m[0];
    float4 b = -m[3] - m[1];
    float4 t = -m[3] + m[1];
    float4 n = -m[3] - m[2];
    float4 f = -m[3] + m[2];

    // NOTE: for our box/frustum intersection routine normalizing these vectors is not required
    // however, they must be normalized for the sphere/frustum tests.
    l *= 1 / length(l.xyz);
    r *= 1 / length(r.xyz);
    b *= 1 / length(b.xyz);
    t *= 1 / length(t.xyz);
    n *= 1 / length(n.xyz);
    f *= 1 / length(f.xyz);

    mPlanes[0] = l;
    mPlanes[1] = r;
    mPlanes[2] = b;
    mPlanes[3] = t;
    mPlanes[4] = f;
    mPlanes[5] = n;
}

float4 Frustum::getNormalizedPlane(Frustum::Plane plane) const noexcept {
    return mPlanes[size_t(plane)];
}

void Frustum::getNormalizedPlanes(float4 planes[6]) const noexcept {
    planes[0] = mPlanes[0];
    planes[1] = mPlanes[1];
    planes[2] = mPlanes[2];
    planes[3] = mPlanes[3];
    planes[4] = mPlanes[4];
    planes[5] = mPlanes[5];
}

bool Frustum::intersects(const Box& box) const noexcept {
    return Culler::intersects(*this, box);
}

bool Frustum::intersects(const float4& sphere) const noexcept {
    return Culler::intersects(*this, sphere);
}

float Frustum::contains(math::float3 p) const noexcept {
    float l = dot(mPlanes[0].xyz, p) + mPlanes[0].w;
    float b = dot(mPlanes[1].xyz, p) + mPlanes[1].w;
    float r = dot(mPlanes[2].xyz, p) + mPlanes[2].w;
    float t = dot(mPlanes[3].xyz, p) + mPlanes[3].w;
    float f = dot(mPlanes[4].xyz, p) + mPlanes[4].w;
    float n = dot(mPlanes[5].xyz, p) + mPlanes[5].w;
    float d = l;
    d = std::max(d, b);
    d = std::max(d, r);
    d = std::max(d, t);
    d = std::max(d, f);
    d = std::max(d, n);
    return d;
}

} // namespace filament
