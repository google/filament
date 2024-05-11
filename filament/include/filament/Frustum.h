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

#ifndef TNT_FILAMENT_FRUSTUM_H
#define TNT_FILAMENT_FRUSTUM_H

#include <utils/compiler.h>

#include <math/mat4.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/unwindows.h> // Because we define NEAR and FAR in the Plane enum.

#include <stdint.h>

namespace filament {

class Box;
class Culler;

/**
 * A frustum defined by six planes
 */
class UTILS_PUBLIC Frustum {
public:
    enum class Plane : uint8_t {
        LEFT,
        RIGHT,
        BOTTOM,
        TOP,
        FAR,
        NEAR
    };

    Frustum() = default;
    Frustum(const Frustum& rhs) = default;
    Frustum(Frustum&& rhs) noexcept = default;
    Frustum& operator=(const Frustum& rhs) = default;
    Frustum& operator=(Frustum&& rhs) noexcept = default;

    /**
     * Creates a frustum from a projection matrix in GL convention
     * (usually the projection * view matrix)
     * @param pv a 4x4 projection matrix in GL convention
     */
    explicit Frustum(const math::mat4f& pv);

    /**
     * Sets the frustum from the given projection matrix
     * @param pv a 4x4 projection matrix
     */
    void setProjection(const math::mat4f& pv);

    /**
     * Returns the plane equation parameters with normalized normals
     * @param plane Identifier of the plane to retrieve the equation of
     * @return A plane equation encoded a float4 R such as R.x*x + R.y*y + R.z*z + R.w = 0
     */
    math::float4 getNormalizedPlane(Plane plane) const noexcept;

    /**
     * Returns a copy of all six frustum planes in left, right, bottom, top, far, near order
     * @param planes six plane equations encoded as in getNormalizedPlane() in
     *              left, right, bottom, top, far, near order
     */
    void getNormalizedPlanes(math::float4 planes[UTILS_NONNULL 6]) const noexcept;

    /**
     * Returns all six frustum planes in left, right, bottom, top, far, near order
     * @return six plane equations encoded as in getNormalizedPlane() in
     *              left, right, bottom, top, far, near order
     */
    math::float4 const* UTILS_NONNULL getNormalizedPlanes() const noexcept { return mPlanes; }

    /**
     * Returns whether a box intersects the frustum (i.e. is visible)
     * @param box The box to test against the frustum
     * @return true if the box may intersects the frustum, false otherwise. In some situations
     * a box that doesn't intersect the frustum might be reported as though it does. However,
     * a box that does intersect the frustum is always reported correctly (true).
     */
    bool intersects(const Box& box) const noexcept;

    /**
     * Returns whether a sphere intersects the frustum (i.e. is visible)
     * @param sphere A sphere encoded as a center + radius.
     * @return true if the sphere may intersects the frustum, false otherwise. In some situations
     * a sphere that doesn't intersect the frustum might be reported as though it does. However,
     * a sphere that does intersect the frustum is always reported correctly (true).
     */
    bool intersects(const math::float4& sphere) const noexcept;

    /**
     * Returns whether the frustum contains a given point.
     * @param p the point to test
     * @return the maximum signed distance to the frustum. Negative if p is inside.
     */
    float contains(math::float3 p) const noexcept;

private:
    friend class Culler;
    math::float4 mPlanes[6];
};

} // namespace filament

#if !defined(NDEBUG)
namespace utils::io {
class ostream;
} // namespace utils::io
utils::io::ostream& operator<<(utils::io::ostream& out, filament::Frustum const& frustum);
#endif

#endif // TNT_FILAMENT_FRUSTUM_H
