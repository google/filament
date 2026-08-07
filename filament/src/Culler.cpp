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

#include <cmath>

using namespace filament::math;

namespace filament {

void Culler::intersects(
        result_type* UTILS_RESTRICT results,
        Frustum const& UTILS_RESTRICT frustum,
        float4 const* UTILS_RESTRICT b,
        size_t count) noexcept {

    float4 const * const UTILS_RESTRICT planes = frustum.mPlanes;

    count = round(count);
    for (size_t i = 0; i < count; i++) {
        int visible = ~0;
        float4 const sphere(b[i]);

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
    float const* UTILS_RESTRICT centerX,
    float const* UTILS_RESTRICT centerY,
    float const* UTILS_RESTRICT centerZ,
    float const* UTILS_RESTRICT extentX,
    float const* UTILS_RESTRICT extentY,
    float const* UTILS_RESTRICT extentZ,
        size_t count, size_t const bit) noexcept {

    float4 const * UTILS_RESTRICT const planes = frustum.mPlanes;
    float3 absPlanes[6];
    for (size_t j = 0; j < 6; j++) {
        absPlanes[j].x = std::abs(planes[j].x);
        absPlanes[j].y = std::abs(planes[j].y);
        absPlanes[j].z = std::abs(planes[j].z);
    }

    count = round(count);
    constexpr size_t BLOCK_SIZE = 4;
    static_assert(MODULO % BLOCK_SIZE == 0,
            "MODULO must be a multiple of 4");
    size_t const blockCount = count / BLOCK_SIZE;
    for (size_t i = 0; i < blockCount; i++) {
        int visible[BLOCK_SIZE];
        for (size_t n = 0; n < BLOCK_SIZE; n++) {
            visible[n] = ~0;
        }

        size_t const base = i * BLOCK_SIZE;
        const float4 cx{ centerX[base + 0], centerX[base + 1], centerX[base + 2], centerX[base + 3] };
        const float4 cy{ centerY[base + 0], centerY[base + 1], centerY[base + 2], centerY[base + 3] };
        const float4 cz{ centerZ[base + 0], centerZ[base + 1], centerZ[base + 2], centerZ[base + 3] };
        const float4 ex{ extentX[base + 0], extentX[base + 1], extentX[base + 2], extentX[base + 3] };
        const float4 ey{ extentY[base + 0], extentY[base + 1], extentY[base + 2], extentY[base + 3] };
        const float4 ez{ extentZ[base + 0], extentZ[base + 1], extentZ[base + 2], extentZ[base + 3] };

        for (size_t j = 0; j < 6; j++) {
            const float pX = planes[j].x;
            const float absPX = absPlanes[j].x;
            const float pY = planes[j].y;
            const float absPY = absPlanes[j].y;
            const float pZ = planes[j].z;
            const float absPZ = absPlanes[j].z;
            const float pW = planes[j].w;

            const float4 dots =
                pX * cx - absPX * ex +
                pY * cy - absPY * ey +
                pZ * cz - absPZ * ez +
                pW;

            visible[0] &= fast::signbit(dots.x) << bit;
            visible[1] &= fast::signbit(dots.y) << bit;
            visible[2] &= fast::signbit(dots.z) << bit;
            visible[3] &= fast::signbit(dots.w) << bit;
        }

        for (size_t n = 0; n < BLOCK_SIZE; n++) {
            auto r = results[base + n];
            r &= ~result_type(1u << bit);
            r |= result_type(visible[n]);
            results[base + n] = r;
        }
    }
}

/*
 * returns whether a box intersects with the frustum
 */

bool Culler::intersects(Frustum const& frustum, Box const& box) noexcept {
    // The main intersection routine assumes multiples of 8 items
    float centerX[MODULO] = {};
    float centerY[MODULO] = {};
    float centerZ[MODULO] = {};
    float extentX[MODULO] = {};
    float extentY[MODULO] = {};
    float extentZ[MODULO] = {};
    result_type results[MODULO];
    centerX[0] = box.center.x;
    centerY[0] = box.center.y;
    centerZ[0] = box.center.z;
    extentX[0] = box.halfExtent.x;
    extentY[0] = box.halfExtent.y;
    extentZ[0] = box.halfExtent.z;
    intersects(results, frustum,
            centerX, centerY, centerZ,
            extentX, extentY, extentZ,
            MODULO, 0);
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
    float const* UTILS_RESTRICT centerX,
    float const* UTILS_RESTRICT centerY,
    float const* UTILS_RESTRICT centerZ,
    float const* UTILS_RESTRICT extentX,
    float const* UTILS_RESTRICT extentY,
    float const* UTILS_RESTRICT extentZ,
        size_t const count) noexcept {
    Culler::intersects(results, frustum,
            centerX, centerY, centerZ,
            extentX, extentY, extentZ,
            count, 0);
}

void Culler::Test::intersects(
        result_type* UTILS_RESTRICT results,
        Frustum const& UTILS_RESTRICT frustum,
        float4 const* UTILS_RESTRICT b, size_t const count) noexcept {
    Culler::intersects(results, frustum, b, count);
}

} // namespace filament
