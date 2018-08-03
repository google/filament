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

#ifndef TNT_FILAMESH_BOX_H
#define TNT_FILAMESH_BOX_H

#include <math/mat4.h>
#include <math/vec3.h>

class Box {
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
        center = (min + max) * 0.5f;
        halfExtent = (max - min) * 0.5f;
        return *this;
    }

    Box& unionSelf(const Box& box) noexcept {
        set(min(getMin(), box.getMin()), max(getMax(), box.getMax()));
        return *this;
    }
};

#endif // TNT_FILAMESH_BOX_H
