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

#ifndef TNT_FILAMENT_VIEWPORT_H
#define TNT_FILAMENT_VIEWPORT_H

#include <backend/DriverEnums.h>

#include <utils/compiler.h>

#include <math/scalar.h>
#include <math/mathfwd.h>

#include <stddef.h>
#include <stdint.h>

namespace filament {

/**
 * Viewport describes a view port in pixel coordinates
 *
 * A view port is represented by its left-bottom coordinate, width and height in pixels.
 */
class UTILS_PUBLIC Viewport : public backend::Viewport {
public:
    /**
     * Creates a Viewport of zero width and height at the origin.
     */
    Viewport() noexcept : backend::Viewport{} {}

    Viewport(const Viewport& viewport) noexcept = default;
    Viewport(Viewport&& viewport) noexcept = default;
    Viewport& operator=(const Viewport& viewport) noexcept = default;
    Viewport& operator=(Viewport&& viewport) noexcept = default;

    /**
     * Creates a Viewport from its left-bottom coordinates, width and height in pixels
     *
     * @param left left coordinate in pixel
     * @param bottom bottom coordinate in pixel
     * @param width width in pixel
     * @param height height in pixel
     */
    Viewport(int32_t left, int32_t bottom, uint32_t width, uint32_t height) noexcept
            : backend::Viewport{ left, bottom, width, height } {
    }

    /**
     * Returns whether the area of the view port is null.
     *
     * @return true if either width or height is 0 pixel.
     */
    bool empty() const noexcept { return !width || !height; }

    /**
     * Computes a new scaled Viewport
     * @param s scaling factor on the x and y axes.
     * @return A new scaled Viewport. The coordinates and dimensions of the new Viewport are
     * rounded to the nearest integer value.
     */
    Viewport scale(math::float2 s) const noexcept;

private:

    /**
     * Compares two Viewports for equality
     * @param lhs reference to the left hand side Viewport
     * @param rhs reference to the rgiht hand side Viewport
     * @return true if \p rhs and \p lhs are identical.
     */
    friend bool operator==(Viewport const& lhs, Viewport const& rhs) noexcept {
        return (&rhs == &lhs) ||
               (rhs.left == lhs.left && rhs.bottom == lhs.bottom &&
                rhs.width == lhs.width && rhs.height == lhs.height);
    }

    /**
     * Compares two Viewports for inequality
     * @param lhs reference to the left hand side Viewport
     * @param rhs reference to the rgiht hand side Viewport
     * @return true if \p rhs and \p lhs are different.
     */
    friend bool operator!=(Viewport const& lhs, Viewport const& rhs) noexcept {
        return !(rhs == lhs);
    }
};

} // namespace filament

#endif // TNT_FILAMENT_VIEWPORT_H
