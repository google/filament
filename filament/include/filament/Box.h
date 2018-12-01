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
    math::float3 center = {};
    math::float3 halfExtent = {};

    constexpr bool isEmpty() const noexcept {
        return length2(halfExtent) == 0;
    }

    constexpr math::float3 getMin() const noexcept {
        return center - halfExtent;
    }

    constexpr math::float3 getMax() const noexcept {
        return center + halfExtent;
    }

    Box& set(const math::float3& min, const math::float3& max) noexcept {
        center     = (max + min) * math::float3(0.5f);
        halfExtent = (max - min) * math::float3(0.5f);
        return *this;
    }

    Box& unionSelf(const Box& box) noexcept {
        set(std::min(getMin(), box.getMin()), std::max(getMax(), box.getMax()));
        return *this;
    }

    constexpr Box translateTo(const math::float3& tr) const noexcept {
        return Box{ tr, halfExtent };
    }

    math::float4 getBoundingSphere() const noexcept {
        return { center, length(halfExtent) };
    }

    friend Box rigidTransform(Box const& box, const math::mat4f& m) noexcept;
    friend Box rigidTransform(Box const& box, const math::mat3f& m) noexcept;
};

struct Aabb {
    math::float3 min = std::numeric_limits<float>::max();
    math::float3 max = std::numeric_limits<float>::lowest();
    math::float3 center() const noexcept { return (min + max) * math::float3(0.5f); }
    bool isEmpty() const noexcept {
        return min >= max;
    }
};

} // namespace filament

#endif // TNT_FILAMENT_BOX_H
