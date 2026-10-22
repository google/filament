/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include <camutils/Bookmark.h>
#include <camutils/Manipulator.h>

#include <math/scalar.h>
#include <math/vec3.h>

using namespace filament::math;

namespace filament {
namespace camutils {

template <typename FLOAT>
Bookmark<FLOAT> Bookmark<FLOAT>::interpolate(Bookmark<FLOAT> a, Bookmark<FLOAT> b, double t) {
    Bookmark<FLOAT> result;
    using float3 = filament::math::vec3<FLOAT>;

    if (a.mode == Mode::MAP) {
        assert(b.mode == Mode::MAP);
        const double rho = sqrt(2.0);
        const double rho2 = 2, rho4 = 4;
        const double ux0 = a.map.center.x, uy0 = a.map.center.y, w0 = a.map.extent;
        const double ux1 = b.map.center.x, uy1 = b.map.center.y, w1 = b.map.extent;
        const double dx = ux1 - ux0, dy = uy1 - uy0, d2 = dx * dx + dy * dy, d1 = sqrt(d2);
        const double b0 = (w1 * w1 - w0 * w0 + rho4 * d2) / (2.0 * w0 * rho2 * d1);
        const double b1 = (w1 * w1 - w0 * w0 - rho4 * d2) / (2.0 * w1 * rho2 * d1);
        const double r0 = log(sqrt(b0 * b0 + 1.0) - b0);
        const double r1 = log(sqrt(b1 * b1 + 1) - b1);
        const double dr = r1 - r0;
        const int valid = !std::isnan(dr) && dr != 0;
        const double S = (valid ? dr : log(w1 / w0)) / rho;
        const double s = t * S;

        // This performs Van Wijk interpolation to animate between two waypoints on a map.
        if (valid) {
            const double coshr0 = cosh(r0);
            const double u = w0 / (rho2 * d1) * (coshr0 * tanh(rho * s + r0) - sinh(r0));
            Bookmark<FLOAT> result;
            result.map.center.x = ux0 + u * dx;
            result.map.center.y = uy0 + u * dy;
            result.map.extent = w0 * coshr0 / cosh(rho * s + r0);
            return result;
        }

        // For degenerate cases, fall back to a simplified interpolation method.
        result.map.center.x = ux0 + t * dx;
        result.map.center.y = uy0 + t * dy;
        result.map.extent = w0 * exp(rho * s);
        return result;
}

    assert(b.mode == Mode::ORBIT);
    result.orbit.phi = lerp(a.orbit.phi, b.orbit.phi, FLOAT(t));
    result.orbit.theta = lerp(a.orbit.theta, b.orbit.theta, FLOAT(t));
    result.orbit.distance = lerp(a.orbit.distance, b.orbit.distance, FLOAT(t));
    result.orbit.pivot = lerp(a.orbit.pivot, b.orbit.pivot, float3(t));
    return result;
}

// Uses the Van Wijk method to suggest a duration for animating between two waypoints on a map.
// This does not have units, so just use it as a multiplier.
template <typename FLOAT>
double Bookmark<FLOAT>::duration(Bookmark<FLOAT> a, Bookmark<FLOAT> b) {
    assert(a.mode == Mode::ORBIT && b.mode == Mode::ORBIT);
    const double rho = sqrt(2.0);
    const double rho2 = 2, rho4 = 4;
    const double ux0 = a.map.center.x, uy0 = a.map.center.y, w0 = a.map.extent;
    const double ux1 = b.map.center.x, uy1 = b.map.center.y, w1 = b.map.extent;
    const double dx = ux1 - ux0, dy = uy1 - uy0, d2 = dx * dx + dy * dy, d1 = sqrt(d2);
    const double b0 = (w1 * w1 - w0 * w0 + rho4 * d2) / (2.0 * w0 * rho2 * d1);
    const double b1 = (w1 * w1 - w0 * w0 - rho4 * d2) / (2.0 * w1 * rho2 * d1);
    const double r0 = log(sqrt(b0 * b0 + 1.0) - b0);
    const double r1 = log(sqrt(b1 * b1 + 1) - b1);
    const double dr = r1 - r0;
    const int valid = !std::isnan(dr) && dr != 0;
    const double S = (valid ? dr : log(w1 / w0)) / rho;
    return fabs(S);
}

template class Bookmark<float>;

} // namespace camutils
} // namespace filament
