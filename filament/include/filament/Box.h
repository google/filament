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

//! \file

#ifndef TNT_FILAMENT_BOX_H
#define TNT_FILAMENT_BOX_H

#include <utils/compiler.h>

#include <limits>

#include <math/mat4.h>
#include <math/vec3.h>

namespace filament {

/**
 * An axis aligned 3D box represented by its center and half-extent.
 */
class UTILS_PUBLIC Box {
public:
    /** Center of the 3D box */
    math::float3 center = {};

    /** Half extent from the center on all 3 axis */
    math::float3 halfExtent = {};

    /**
     * Whether the box is empty, i.e.: it's volume is null.
     * @return true if the volume of the box is null
     */
    constexpr bool isEmpty() const noexcept {
        return length2(halfExtent) == 0;
    }

    /**
     * Computes the lowest coordinates corner of the box.
     * @return center - halfExtent
     */
    constexpr math::float3 getMin() const noexcept {
        return center - halfExtent;
    }

    /**
     * Computes the largest coordinates corner of the box.
     * @return center + halfExtent
     */
    constexpr math::float3 getMax() const noexcept {
        return center + halfExtent;
    }

    /**
     * Initializes the 3D box from its min / max coordinates on each axis
     * @param min lowest coordinates corner of the box
     * @param max largest coordinates corner of the box
     * @return This bounding box
     */
    Box& set(const math::float3& min, const math::float3& max) noexcept {
        // float3 ctor needed for visual studio
        center     = (max + min) * math::float3(0.5f);
        halfExtent = (max - min) * math::float3(0.5f);
        return *this;
    }

    /**
     * Computes the bounding box of the union of two boxes
     * @param box The box to be combined with
     * @return The bounding box of the union of *this and box
     */
    Box& unionSelf(const Box& box) noexcept {
        set(min(getMin(), box.getMin()), max(getMax(), box.getMax()));
        return *this;
    }

    /**
     * Translates the box *to* a given center position
     * @param tr position to translate the box to
     * @return A box centered in \p tr with the same extent than *this
     */
    constexpr Box translateTo(const math::float3& tr) const noexcept {
        return Box{ tr, halfExtent };
    }

    /**
     * Computes the smallest bounding sphere of the box.
     * @return The smallest sphere defined by its center (.xyz) and radius (.w) that contains *this
     */
    math::float4 getBoundingSphere() const noexcept {
        return { center, length(halfExtent) };
    }

    /**
     * Transform a Box by a linear transform and a translation.
     *
     * @param m a 3x3 matrix, the linear transform
     * @param t a float3, the translation
     * @param box the box to transform
     * @return the bounding box of the transformed box
     */
    static Box transform(const math::mat3f& m, math::float3 const& t, const Box& box) noexcept {
        return { m * box.center + t, abs(m) * box.halfExtent };
    }

    /**
     * @deprecated Use transform() instead
     * @see transform()
     */
    friend Box rigidTransform(Box const& box, const math::mat4f& m) noexcept {
        return transform(m.upperLeft(), m[3].xyz, box);
    }
};

/**
 * An axis aligned box represented by its min and max coordinates
 */
struct UTILS_PUBLIC Aabb {

    /** min coordinates */
    math::float3 min = std::numeric_limits<float>::max();

    /** max coordinates */
    math::float3 max = std::numeric_limits<float>::lowest();

    /**
     * Computes the center of the box.
     * @return (max + min)/2
     */
    math::float3 center() const noexcept {
        // float3 ctor needed for visual studio
        return (max + min) * math::float3(0.5f);
    }

    /**
     * Computes the half-extent of the box.
     * @return (max - min)/2
     */
    math::float3 extent() const noexcept {
        // float3 ctor needed for visual studio
        return (max - min) * math::float3(0.5f);
    }

    /**
     * Whether the box is empty, i.e.: it's volume is null or negative.
     * @return true if min >= max, i.e: the volume of the box is null or negative
     */
    bool isEmpty() const noexcept {
        return any(greaterThanEqual(min, max));
    }

    struct Corners {
        using value_type = math::float3;
        value_type const* begin() const { return vertices; }
        value_type const* end() const { return vertices + 8; }
        value_type * begin() { return vertices; }
        value_type * end() { return vertices + 8; }
        value_type const* data() const { return vertices; }
        value_type * data() { return vertices; }
        size_t size() const { return 8; }
        value_type vertices[8];
    };

    /**
     * Returns the 8 corner vertices of the AABB.
     */
    Corners getCorners() const {
        return Aabb::Corners{ .vertices = {
                { min.x, min.y, min.z },
                { max.x, min.y, min.z },
                { min.x, max.y, min.z },
                { max.x, max.y, min.z },
                { min.x, min.y, max.z },
                { max.x, min.y, max.z },
                { min.x, max.y, max.z },
                { max.x, max.y, max.z },
        }};
    }

    /**
     * Returns whether the box contains a given point.
     *
     * @param p the point to test
     * @return the maximum signed distance to the box. Negative if p is in the box
     */
    float contains(math::float3 p) const noexcept {
        float d = min.x - p.x;
        d = std::max(d, min.y - p.y);
        d = std::max(d, min.z - p.z);
        d = std::max(d, p.x - max.x);
        d = std::max(d, p.y - max.y);
        d = std::max(d, p.z - max.z);
        return d;
    }

    /**
     * Applies an affine transformation to the AABB.
     *
     * @param m the 3x3 transformation to apply
     * @param t the translation
     * @return the transformed box
     */
    static Aabb transform(const math::mat3f& m, math::float3 const& t, const Aabb& box) noexcept {
        // Fast AABB transformation per Jim Arvo in Graphics Gems (1990).
        Aabb result{ t, t };
        for (size_t col = 0; col < 3; ++col) {
            for (size_t row = 0; row < 3; ++row) {
                const float a = m[col][row] * box.min[col];
                const float b = m[col][row] * box.max[col];
                result.min[row] += a < b ? a : b;
                result.max[row] += a < b ? b : a;
            }
        }
        return result;
    }

    /**
     * @deprecated Use transform() instead
     * @see transform()
     */
    Aabb transform(const math::mat4f& m) const noexcept {
        return transform(m.upperLeft(), m[3].xyz, *this);
    }
};

} // namespace filament

#endif // TNT_FILAMENT_BOX_H
