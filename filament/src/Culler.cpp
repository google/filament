/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "details/Culler.h"

#include <filament/Box.h>

#include <math/fast.h>

using namespace filament::math;

namespace filament {

void Culler::intersects(
        result_type* UTILS_RESTRICT results,
        Frustum const& UTILS_RESTRICT frustum,
        float4 const* UTILS_RESTRICT b,
        size_t count) noexcept {

    float4 const * const UTILS_RESTRICT planes = frustum.mPlanes;

    // we use a vectorize width of 8 because, on ARMv8 it allow the compiler to write 8
    // 8-bits results in one go. Without this it has to do 4 separate byte writes, which
    // ends-up being slower.
    count = round(count); // capacity guaranteed to be multiple of 8
    #pragma clang loop vectorize_width(8)
    for (size_t i = 0; i < count; i++) {
        int visible = ~0;
        float4 const sphere(b[i]);

        #pragma clang loop unroll(full)
        for (size_t j = 0; j < 6; j++) {
            // clang doesn't seem to generate vector * scalar instructions, which leads
            // to increased register pressure and stack spills
            const float dot = planes[j].x * sphere.x +
                              planes[j].y * sphere.y +
                              planes[j].z * sphere.z +
                              planes[j].w - sphere.w;
            visible &= fast::signbit(dot);
        }
        results[i] = result_type(visible);
    }
}

void Culler::intersects(
        result_type* UTILS_RESTRICT results,
        Frustum const& UTILS_RESTRICT frustum,
        float3 const* UTILS_RESTRICT center,
        float3 const* UTILS_RESTRICT extent,
        size_t count, size_t bit) noexcept {

    float4 const * UTILS_RESTRICT const planes = frustum.mPlanes;

    // we use a vectorize width of 8 because, on ARMv8 it allows the compiler to write eight
    // 8-bits results in one go. Without this it has to do 4 separate byte writes, which
    // ends-up being slower.
    count = round(count); // capacity guaranteed to be multiple of 8
    #pragma clang loop vectorize_width(8)
    for (size_t i = 0; i < count; i++) {
        int visible = ~0;

        #pragma clang loop unroll(full)
        for (size_t j = 0; j < 6; j++) {
            // clang doesn't seem to generate vector * scalar instructions, which leads
            // to increased register pressure and stack spills
            const float dot =
                    planes[j].x * center[i].x - std::abs(planes[j].x) * extent[i].x +
                    planes[j].y * center[i].y - std::abs(planes[j].y) * extent[i].y +
                    planes[j].z * center[i].z - std::abs(planes[j].z) * extent[i].z +
                    planes[j].w;

            visible &= fast::signbit(dot) << bit;
        }

        results[i] |= result_type(visible);
    }
}

/*
 * returns whether a box intersects with the frustum
 */

bool Culler::intersects(Frustum const& frustum, Box const& box) noexcept {
    // The main intersection routine assumes multiples of 8 items
    float3 centers[MODULO];
    float3 extents[MODULO];
    Culler::result_type results[MODULO];
    centers[0] = box.center;
    extents[0] = box.halfExtent;
    results[0] = 0;
    Culler::intersects(results, frustum, centers, extents, MODULO, 0);
    return bool(results[0]);
}

/*
 * returns whether an sphere intersects with the frustum
 */
bool Culler::intersects(Frustum const& frustum, float4 const& sphere) noexcept {
    // The main intersection routine assumes multiples of 8 items
    float4 spheres[MODULO];
    Culler::result_type results[MODULO];
    spheres[0] = sphere;
    results[0] = 0;
    Culler::intersects(results, frustum, spheres, MODULO);
    return bool(results[0]);
}

// For testing...

void Culler::Test::intersects(
        result_type* UTILS_RESTRICT results,
        Frustum const& UTILS_RESTRICT frustum,
        float3 const* UTILS_RESTRICT c,
        float3 const* UTILS_RESTRICT e,
        size_t count) noexcept {
    Culler::intersects(results, frustum, c, e, count, 0);
}

void Culler::Test::intersects(
        result_type* UTILS_RESTRICT results,
        Frustum const& UTILS_RESTRICT frustum,
        float4 const* UTILS_RESTRICT b, size_t count) noexcept {
    Culler::intersects(results, frustum, b, count);
}

} // namespace filament
