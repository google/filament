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

#include "Culler.h"

#include <filament/Box.h>

#include <math/fast.h>

using namespace filament::math;

// use 8 if Culler::result_type is 8-bits, on ARMv8 it allows the compiler to write eight
// results in one go.
#define FILAMENT_CULLER_VECTORIZE_HINT 4

namespace filament {

static_assert(Culler::MODULO % FILAMENT_CULLER_VECTORIZE_HINT == 0,
        "MODULO m=must be a multiple of FILAMENT_CULLER_VECTORIZE_HINT");

void Culler::intersects(
        result_type* UTILS_RESTRICT results,
        Frustum const& UTILS_RESTRICT frustum,
        float4 const* UTILS_RESTRICT b,
        size_t count) noexcept {

    float4 const * const UTILS_RESTRICT planes = frustum.mPlanes;

    count = round(count);
#if defined(__clang__)
    #pragma clang loop vectorize_width(FILAMENT_CULLER_VECTORIZE_HINT)
#endif
    for (size_t i = 0; i < count; i++) {
        int visible = ~0;
        float4 const sphere(b[i]);

#if defined(__clang__)
        #pragma clang loop unroll(full)
#endif
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
        size_t count, size_t const bit) noexcept {

    float4 const * UTILS_RESTRICT const planes = frustum.mPlanes;

    count = round(count);
#if defined(__clang__)
    #pragma clang loop vectorize_width(FILAMENT_CULLER_VECTORIZE_HINT)
#endif
    for (size_t i = 0; i < count; i++) {
        int visible = ~0;

#if defined(__clang__)
        #pragma clang loop unroll(full)
#endif
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

        auto r = results[i];
        r &= ~result_type(1u << bit);
        r |= result_type(visible);
        results[i] = r;
    }
}

/*
 * returns whether a box intersects with the frustum
 */

bool Culler::intersects(Frustum const& frustum, Box const& box) noexcept {
    // The main intersection routine assumes multiples of 8 items
    float3 centers[MODULO];
    float3 extents[MODULO];
    result_type results[MODULO];
    centers[0] = box.center;
    extents[0] = box.halfExtent;
    intersects(results, frustum, centers, extents, MODULO, 0);
    return bool(results[0] & 1);
}

/*
 * returns whether a sphere intersects with the frustum
 */
bool Culler::intersects(Frustum const& frustum, float4 const& sphere) noexcept {
    // The main intersection routine assumes multiples of 8 items
    float4 spheres[MODULO];
    result_type results[MODULO];
    spheres[0] = sphere;
    intersects(results, frustum, spheres, MODULO);
    return bool(results[0] & 1);
}

// For testing...

void Culler::Test::intersects(
        result_type* UTILS_RESTRICT results,
        Frustum const& UTILS_RESTRICT frustum,
        float3 const* UTILS_RESTRICT c,
        float3 const* UTILS_RESTRICT e,
        size_t const count) noexcept {
    Culler::intersects(results, frustum, c, e, count, 0);
}

void Culler::Test::intersects(
        result_type* UTILS_RESTRICT results,
        Frustum const& UTILS_RESTRICT frustum,
        float4 const* UTILS_RESTRICT b, size_t const count) noexcept {
    Culler::intersects(results, frustum, b, count);
}

} // namespace filament
