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

#include <filament/Box.h>

using namespace filament::math;

namespace filament {

Box rigidTransform(Box const& UTILS_RESTRICT box, const mat4f& UTILS_RESTRICT m) noexcept {
    const mat3f u(m.upperLeft());
    return { u * box.center + m[3].xyz, abs(u) * box.halfExtent };
}

Box rigidTransform(Box const& UTILS_RESTRICT box, const mat3f& UTILS_RESTRICT u) noexcept {
    return { u * box.center, abs(u) * box.halfExtent };
}

Aabb::Corners Aabb::getCorners() const {
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

float Aabb::contains(math::float3 p) const noexcept {
    float d = min.x - p.x;
    d = std::max(d, min.y - p.y);
    d = std::max(d, min.z - p.z);
    d = std::max(d, p.x - max.x);
    d = std::max(d, p.y - max.y);
    d = std::max(d, p.z - max.z);
    return d;
}

} // namespace filament
