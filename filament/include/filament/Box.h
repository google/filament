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
     * @return
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
     * @return The boudning box of the union of *this and box
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
     * Computes the bounding box of a box transformed by a rigid transform
     * @param box the box to transfrom
     * @param m a 4x4 matrix that must be a rigid transform
     * @return the bounding box of the transformed box.
     *         Result is undefined if \p m is not a rigid transform
     */
    friend Box rigidTransform(Box const& box, const math::mat4f& m) noexcept;

    /**
     * Computes the bounding box of a box transformed by a rigid transform
     * @param box the box to transfrom
     * @param m a 3x3 matrix that must be a rigid transform
     * @return the bounding box of the transformed box.
     *         Result is undefined if \p m is not a rigid transform
     */
    friend Box rigidTransform(Box const& box, const math::mat3f& m) noexcept;
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
    Corners getCorners() const;

    /**
     * Returns whether the box contains a given point.
     *
     * @param p the point to test
     * @return the maximum signed distance to the box. Negative if p is in the box
     */
    float contains(math::float3 p) const noexcept;

    /**
     * Applies an affine transformation to the AABB.
     *
     * @param m the 4x4 transformation to apply
     * @return the transformed box
     */
    Aabb transform(const math::mat4f& m) const noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_BOX_H
