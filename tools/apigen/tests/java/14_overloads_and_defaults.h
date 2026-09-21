/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_FILAMENT_OVERLOADS_DEFAULTS_H
#define TNT_FILAMENT_OVERLOADS_DEFAULTS_H

#include <math/mat4.h>
#include <math/vec3.h>

namespace filament {

class OverloadTest {
public:
    enum class Mode : uint8_t {
        FAST,
        ACCURATE
    };

    /**
     * Configures the system with optional mode.
     * @param a first parameter
     * @param b second parameter
     * @param mode operating mode
     */
    void configure(int a, float b, Mode mode = Mode::FAST) noexcept;

    /**
     * Computes projection matrix with default far plane.
     * @param fov field of view
     * @param aspect aspect ratio
     * @param near near plane distance
     * @param far far plane distance
     * @return projection matrix
     */
    static math::mat4 projection(double fov, double aspect, double near, double far = INFINITY) noexcept;

    /**
     * Sets light intensity using simple intensity.
     * @param intensity luminous intensity
     */
    void setIntensity(float intensity) noexcept;

    /**
     * Sets light intensity using physical power and efficiency.
     * @param watts power in watts
     * @param efficiency luminous efficiency
     */
    void setIntensity(float watts, float efficiency) noexcept;

    /**
     * Computes inverse of double-precision matrix.
     * @param m input matrix
     * @return inverted matrix
     */
    static math::mat4 inverse(math::mat4 const& m) noexcept;

    /**
     * Computes inverse of single-precision matrix.
     * @param m input matrix
     * @return inverted matrix
     */
    static math::mat4f inverse(math::mat4f const& m) noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_OVERLOADS_DEFAULTS_H
