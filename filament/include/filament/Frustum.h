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

#ifndef TNT_FILAMENT_FRUSTUM_H
#define TNT_FILAMENT_FRUSTUM_H

#include <filament/Box.h>

#include <utils/compiler.h>

#include <math/mat4.h>
#include <math/vec4.h>
#include <math/vec2.h>

#include <utils/unwindows.h> // Because we define NEAR and FAR in the Plane enum.

namespace filament {

namespace details {
class Culler;
} // namespace details;

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

    // create a frustum from a projection matrix (usually the projection * view matrix)
    explicit Frustum(const math::mat4f& pv);

    // set the frustum from the given projection matrix
    void setProjection(const math::mat4f& pv);

    // return the plane equation parameters with normalized normals
    math::float4 getNormalizedPlane(Plane plane) const noexcept;

    // return frustum planes in left, right, bottom, top, far, near order
    void getNormalizedPlanes(math::float4 planes[6]) const noexcept;

    math::float4 const* getNormalizedPlanes() const noexcept { return mPlanes; }

    // returns whether a box intersects the frustum (i.e. is visible)
    bool intersects(const Box& box) const noexcept;

    // returns whether a sphere intersects the frustum (i.e. is visible)
    bool intersects(const math::float4& sphere) const noexcept;

private:
    friend class details::Culler;
    math::float4 mPlanes[6];
};


} // namespace filament

#endif // TNT_FILAMENT_FRUSTUM_H
