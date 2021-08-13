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

#ifndef TNT_FILAMENT_DETAILS_CULLER_H
#define TNT_FILAMENT_DETAILS_CULLER_H

#include <filament/Frustum.h>

#include <utils/compiler.h>
#include <utils/Slice.h>

#include <math/vec4.h>
#include <math/vec2.h>

namespace filament {

/*
 * This is where culling is implemented.
 *
 * The implementation assumes 'count' below is multiple of 8
 *
 */

class Culler {
public:
    // Culler can only process buffers with a size multiple of MODULO
    static constexpr size_t MODULO = 8;
    static inline size_t round(size_t count) noexcept {
        return (count + (MODULO - 1)) & ~(MODULO - 1);
    }

    // A good loop value to use to amortize the loop overhead
    static constexpr size_t MIN_LOOP_COUNT_HINT = 8;

    using result_type = uint8_t;

    /*
     * returns whether each AABB in an array intersects with the frustum
     */
    static void intersects(result_type* results,
            Frustum const& frustum,
            math::float3 const* center,
            math::float3 const* extent,
            size_t count, size_t bit) noexcept;

    /*
     * returns whether each sphere in an array intersects with the frustum
     */
    static void intersects(
            result_type* results,
            Frustum const& frustum,
            math::float4 const* b,
            size_t count) noexcept;

    /*
     * returns whether an AABB intersects with the frustum
     */
    static bool intersects(
            Frustum const& frustum,
            Box const& box) noexcept;

    /*
     * returns whether an sphere intersects with the frustum
     */
    static bool intersects(
            Frustum const& frustum,
            math::float4 const& sphere) noexcept;


    struct UTILS_PUBLIC Test {
        static void intersects(result_type* results,
                Frustum const& frustum,
                math::float3 const* c,
                math::float3 const* e,
                size_t count) noexcept;

        static void intersects(result_type* results,
                Frustum const& frustum,
                math::float4 const* b,
                size_t count) noexcept;
    };
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_CULLER_H
