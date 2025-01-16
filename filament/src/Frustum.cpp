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
    setProjection(pv);
}

// NOTE: if we don't specify noinline here, LLVM inlines this huge function into
// two (?!) version of the Frustum(const mat4f& pv) constructor!

UTILS_NOINLINE
void Frustum::setProjection(const mat4f& pv) {
    // see: "Fast Extraction of Viewing Frustum Planes from the WorldView-Projection Matrix"
    // by Gil Gribb & Klaus Hartmann
    //
    // Another way to think about this is that we're transforming each plane in clip-space to
    // view-space. Such transform is performed as:
    //      transpose(inverse(viewFromClipMatrix)), i.e.: transpose(projection)

    const mat4f m(transpose(pv));

    // Note: these "normals" are not normalized -- it's not necessary for the culling tests.
    float4 l = -m[3] - m[0];    // m * { -1,  0,  0, -1 }
    float4 r = -m[3] + m[0];    // m * {  1,  0,  0, -1 }
    float4 b = -m[3] - m[1];    // m * {  0, -1,  0, -1 }
    float4 t = -m[3] + m[1];    // m * {  0,  1,  0, -1 }
    float4 n = -m[3] - m[2];    // m * {  0,  0, -1, -1 }
    float4 f = -m[3] + m[2];    // m * {  0,  0,  1, -1 }

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

float4 Frustum::getNormalizedPlane(Plane plane) const noexcept {
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

float Frustum::contains(float3 p) const noexcept {
    float const l = dot(mPlanes[0].xyz, p) + mPlanes[0].w;
    float const b = dot(mPlanes[1].xyz, p) + mPlanes[1].w;
    float const r = dot(mPlanes[2].xyz, p) + mPlanes[2].w;
    float const t = dot(mPlanes[3].xyz, p) + mPlanes[3].w;
    float const f = dot(mPlanes[4].xyz, p) + mPlanes[4].w;
    float const n = dot(mPlanes[5].xyz, p) + mPlanes[5].w;
    float d = l;
    d = std::max(d, b);
    d = std::max(d, r);
    d = std::max(d, t);
    d = std::max(d, f);
    d = std::max(d, n);
    return d;
}

} // namespace filament

#if !defined(NDEBUG)

utils::io::ostream& operator<<(utils::io::ostream& out, filament::Frustum const& frustum) {
    float4 planes[6];
    frustum.getNormalizedPlanes(planes);
    out     << planes[0] << '\n'
            << planes[1] << '\n'
            << planes[2] << '\n'
            << planes[3] << '\n'
            << planes[4] << '\n'
            << planes[5] << utils::io::endl;
    return out;
}

#endif
