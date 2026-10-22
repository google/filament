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

#ifndef TNT_FILAMENT_EXPOSURE_H
#define TNT_FILAMENT_EXPOSURE_H

#include <utils/compiler.h>

namespace filament {

class Camera;

/**
 * A series of utilities to compute exposure, exposure value at ISO 100 (EV100),
 * luminance and illuminance using a physically-based camera model.
 */
namespace Exposure {

/**
 * Returns the exposure value (EV at ISO 100) of the specified camera.
 */
UTILS_PUBLIC
float ev100(const Camera& camera) noexcept;

/**
 * Returns the exposure value (EV at ISO 100) of the specified exposure parameters.
 */
UTILS_PUBLIC
float ev100(float aperture, float shutterSpeed, float sensitivity) noexcept;

/**
 * Returns the exposure value (EV at ISO 100) for the given average luminance (in @f$ \frac{cd}{m^2} @f$).
 */
UTILS_PUBLIC
float ev100FromLuminance(float luminance) noexcept;

/**
* Returns the exposure value (EV at ISO 100) for the given illuminance (in lux).
*/
UTILS_PUBLIC
float ev100FromIlluminance(float illuminance) noexcept;

/**
 * Returns the photometric exposure for the specified camera.
 */
UTILS_PUBLIC
float exposure(const Camera& camera) noexcept;

/**
 * Returns the photometric exposure for the specified exposure parameters.
 * This function is equivalent to calling `exposure(ev100(aperture, shutterSpeed, sensitivity))`
 * but is slightly faster and offers higher precision.
 */
UTILS_PUBLIC
float exposure(float aperture, float shutterSpeed, float sensitivity) noexcept;

/**
 * Returns the photometric exposure for the given EV100.
 */
UTILS_PUBLIC
float exposure(float ev100) noexcept;

/**
 * Returns the incident luminance in @f$ \frac{cd}{m^2} @f$ for the specified camera acting as a spot meter.
 */
UTILS_PUBLIC
float luminance(const Camera& camera) noexcept;

/**
 * Returns the incident luminance in @f$ \frac{cd}{m^2} @f$ for the specified exposure parameters of
 * a camera acting as a spot meter.
 * This function is equivalent to calling `luminance(ev100(aperture, shutterSpeed, sensitivity))`
 * but is slightly faster and offers higher precision.
 */
UTILS_PUBLIC
float luminance(float aperture, float shutterSpeed, float sensitivity) noexcept;

/**
 * Converts the specified EV100 to luminance in @f$ \frac{cd}{m^2} @f$.
 * EV100 is not a measure of luminance, but an EV100 can be used to denote a
 * luminance for which a camera would use said EV100 to obtain the nominally
 * correct exposure
 */
UTILS_PUBLIC
float luminance(float ev100) noexcept;

/**
 * Returns the illuminance in lux for the specified camera acting as an incident light meter.
 */
UTILS_PUBLIC
float illuminance(const Camera& camera) noexcept;

/**
 * Returns the illuminance in lux for the specified exposure parameters of
 * a camera acting as an incident light meter.
 * This function is equivalent to calling `illuminance(ev100(aperture, shutterSpeed, sensitivity))`
 * but is slightly faster and offers higher precision.
 */
UTILS_PUBLIC
float illuminance(float aperture, float shutterSpeed, float sensitivity) noexcept;

/**
 * Converts the specified EV100 to illuminance in lux.
 * EV100 is not a measure of illuminance, but an EV100 can be used to denote an
 * illuminance for which a camera would use said EV100 to obtain the nominally
 * correct exposure.
 */
UTILS_PUBLIC
float illuminance(float ev100) noexcept;

} // namespace exposure
} // namespace filament

#endif // TNT_FILAMENT_EXPOSURE_H
