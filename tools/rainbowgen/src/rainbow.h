/*
 * Copyright (C) 2024 The Android Open Source Project
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

#pragma once

#include <math/mat3.h>
#include <math/vec3.h>

#include <cmath>

namespace rainbow {

using namespace filament::math;

using radian_t = float;
using nanometer_t = float;
using celcius_t = float;

// fresnel at water droplet interface
inline float fresnel(radian_t Bi, radian_t Bt) noexcept {
    float const r_pe = std::sin(Bi - Bt) / std::sin(Bi + Bt);
    float const r_pa = std::tan(Bi - Bt) / std::tan(Bi + Bt);
    float const r = 0.5f * (r_pe * r_pe + r_pa * r_pa);
    return r;
}

// refraction angle
inline radian_t refract(float n, radian_t incident) noexcept {
    return std::asin(std::sin(incident) / n);
}

// max incident angle for a given index of refraction
inline radian_t maxIncidentAngle(float n) noexcept {
    return std::acos(std::sqrt((n * n - 1.0f) / 3.0f));
}

// deviation: angle between ray exiting the droplet and the ground
// impactAngle: angle between the ground and the incident ray
inline radian_t deviation(int order, radian_t incident, radian_t refracted, radian_t impactAngle) noexcept  {
    if (order == 0) {
        return 4.0f * refracted - 2.0f * incident - impactAngle;
    }
    if (order == 1) {
        return f::PI + 6.0f * refracted - 2.0f * incident - impactAngle;
    }
    return -1;
}

// deviation: angle between ray entering the droplet and ray exiting the droplet
inline radian_t deviation(float n, radian_t incident) noexcept  {
    return deviation(0, incident, refract(n, incident), 0.0f);
}

// index of refraction for a wavelength
inline float indexOfRefraction(nanometer_t wavelength) noexcept {
    celcius_t const temperature = 20;
    constexpr float a0 = 0.244257733;
    constexpr float a1 = 0.00974634476;
    constexpr float a2 = -0.00373234996;
    constexpr float a3 = 0.000268678472;
    constexpr float a4 = 0.0015892057;
    constexpr float a5 = 0.00245934259;
    constexpr float a6 = 0.90070492;
    constexpr float a7 = -0.0166626219;
    constexpr float Ts = 273.15;    // [K]
    constexpr float Ps = 1000;      // [kg/m3]
    constexpr float Ls = 589;       // [nm]
    constexpr float Lir = 5.432937;
    constexpr float Luv = 0.229202;

    float const T = Ts + temperature;   // temperature
    float const P = 998.2;              // FIXME: this also depends on the temperature (20)
    float const L = wavelength;
    float const T_ = T / Ts;
    float const P_ = P / Ps;
    float const L_ = L / Ls;

    float const R =
            a0 +
            a1 * P_ +
            a2 * T_ +
            a3 * L_ * L_ * T_ +
            a4 / (L_ * L_) +
            a5 / ((L_ * L_) - (Luv * Luv)) +
            a6 / ((L_ * L_) - (Lir * Lir)) +
            a7 * P_ * P_;

    float const n = std::sqrt((1.0f + 2.0f * R * P_) / (1.0f - R * P_));

    return n;
}

inline constexpr float3 XYZ_to_sRGB(float3 const v) noexcept {
    constexpr mat3f const XYZ_sRGB{
            3.2404542f, -0.9692660f,  0.0556434f,
            -1.5371385f,  1.8760108f, -0.2040259f,
            -0.4985314f,  0.0415560f,  1.0572252f
    };
    return XYZ_sRGB * v;
}

inline float3 linear_to_sRGB(float3 c) noexcept {
    for (auto i = 0; i < c.size(); i++) {
        c[i] = (c[i] <= 0.0031308f) ?
                c[i] * 12.92f : (std::pow(c[i], 1.0f / 2.4f) * 1.055f) - 0.055f;
    }
    return c;
}

inline float3 sRGB_to_linear(float3 c) noexcept {
    for (auto i = 0; i < c.size(); i++) {
        c[i] = (c[i] <= 0.04045f) ?
                c[i] / 12.92f : std::pow((c[i] + 0.055f) / 1.055f, 2.4f);
    }
    return c;
}

} // namespace rainbow