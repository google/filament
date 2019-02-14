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

#ifndef TNT_FILAMENT_VIEWPORT_H
#define TNT_FILAMENT_VIEWPORT_H

#include <filament/driver/DriverEnums.h>

#include <utils/compiler.h>

#include <math/scalar.h>
#include <math/vec2.h>

#include <stddef.h>
#include <stdint.h>

namespace filament {

/*
 * Viewport describes a view port in pixel coordinates
 */
class UTILS_PUBLIC Viewport : public driver::Viewport {
public:
    Viewport() noexcept = default;
    Viewport(const Viewport& viewport) noexcept = default;
    Viewport(Viewport&& viewport) noexcept = default;
    Viewport& operator=(const Viewport& viewport) noexcept = default;
    Viewport& operator=(Viewport&& viewport) noexcept = default;

    /*
     * Create a Viewport from its left-bottom coordinates and size in pixels
     */
    Viewport(int32_t left, int32_t bottom, uint32_t width, uint32_t height) noexcept
            : driver::Viewport{ left, bottom, width, height } {
    }

    bool empty() const noexcept { return !width || !height; }

    Viewport scale(filament::math::float2 s) const noexcept;

private:
    friend bool operator==(Viewport const& rhs, Viewport const& lhs) noexcept {
        return (&rhs == &lhs) ||
               (rhs.left == lhs.left && rhs.bottom == lhs.bottom &&
                rhs.width == lhs.width && rhs.height == lhs.height);
    }

    friend bool operator!=(Viewport const& rhs, Viewport const& lhs) noexcept {
        return !(rhs == lhs);
    }
};

} // namespace filament

#endif // TNT_FILAMENT_VIEWPORT_H
