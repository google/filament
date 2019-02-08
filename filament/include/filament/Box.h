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

#ifndef TNT_FILAMENT_BOX_H
#define TNT_FILAMENT_BOX_H

#include <utils/compiler.h>

#include <limits>

#include <math/mat4.h>
#include <math/vec3.h>

namespace filament {

class UTILS_PUBLIC Box {
public:
    filament::math::float3 center = {};
    filament::math::float3 halfExtent = {};

    constexpr bool isEmpty() const noexcept {
        return length2(halfExtent) == 0;
    }

    constexpr filament::math::float3 getMin() const noexcept {
        return center - halfExtent;
    }

    constexpr filament::math::float3 getMax() const noexcept {
        return center + halfExtent;
    }

    Box& set(const filament::math::float3& min, const filament::math::float3& max) noexcept {
        center     = (max + min) * filament::math::float3(0.5f);
        halfExtent = (max - min) * filament::math::float3(0.5f);
        return *this;
    }

    Box& unionSelf(const Box& box) noexcept {
        set(std::min(getMin(), box.getMin()), std::max(getMax(), box.getMax()));
        return *this;
    }

    constexpr Box translateTo(const filament::math::float3& tr) const noexcept {
        return Box{ tr, halfExtent };
    }

    filament::math::float4 getBoundingSphere() const noexcept {
        return { center, length(halfExtent) };
    }

    friend Box rigidTransform(Box const& box, const filament::math::mat4f& m) noexcept;
    friend Box rigidTransform(Box const& box, const filament::math::mat3f& m) noexcept;
};

struct Aabb {
    filament::math::float3 min = std::numeric_limits<float>::max();
    filament::math::float3 max = std::numeric_limits<float>::lowest();
    filament::math::float3 center() const noexcept { return (min + max) * filament::math::float3(0.5f); }
    bool isEmpty() const noexcept {
        return min >= max;
    }
};

} // namespace filament

#endif // TNT_FILAMENT_BOX_H
