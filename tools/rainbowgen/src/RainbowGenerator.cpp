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

#include "RainbowGenerator.h"
#include "CIE.h"
#include "targa.h"

#include <utils/JobSystem.h>

#include <math/scalar.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/mat3.h>

#include <algorithm>
#include <cmath>
#include <iostream>

#include <stdint.h>
#include <stddef.h>

using namespace utils;
using namespace filament::math;

RainbowGenerator::RainbowGenerator() {
}

RainbowGenerator::~RainbowGenerator() {
}

void RainbowGenerator::build(JobSystem& js) {
    uint32_t const angleCount = mAngleCount;
    float const n0 = iorFromWavelength(mMinWavelength, mTemprature);
    float const n1 = iorFromWavelength(mMaxWavelength, mTemprature);
    float const minAngle = maxIncidentAngle(n0);
    float const maxAngle = maxIncidentAngle(n1);


//    for (radian_t Bi = 0; Bi < f::PI_2; Bi += f::DEG_TO_RAD) {
//        float const R0 = fresnel(Bi, std::asin(std::sin(Bi) * n0));
//        float const R1 = fresnel(Bi, std::asin(std::sin(Bi) * n1));
//        std::cout << R0 << ", " << R1 << std::endl;
//    }
//return;

//     sin(2B - phi) = n * sin(B);
// C = B-phi/2
//    sin(2C) = n sin(B)
//    2sin(C)cos(C) = n sin(B)
//    2sin(B-phi/2)cos(B-phi/2) = n sin(B)

//    (cot(phi/2) - cot(B)) * (cot(B)cot(phi/2) + 1) = (n/2)/sin(phi/2)
//    (cot(B)cot(phi/2)cot(phi/2) - cot(B)cot(B)cot(phi/2) + cot(B) = (n/2)/sin(phi/2) - cot(phi/2)
// X = cot(B)
// a = cot(phi/2)
// b = cot(phi/2)cot(phi/2) + 1
// c = - ((n/2)/sin(phi/2) - cot(phi/2))

//  X^2 * a + X * b + c == 0

// x = -b + sqrt(b*b - 4 *a*c)/(2*a)

//    sin(B)cos(phi/2)-cos(B)sin(phi/2) = (n/2) sin(B)
//    (cos(phi/2)/sin(phi/2))-(cos(B)/sin(B)) = (n/2)/sin(phi/2)
//     tan(B) = sin(phi/2) / (cos(phi/2) - n/2))
//     B = atan( sin(phi/2) / (cos(phi/2) - n/2)) )

    auto cot = [](float x) { return 1.0f / std::tan(x); };

    float n = 1.33;
    for (radian_t phi = 0; phi < f::PI_2; phi += f::DEG_TO_RAD) {
//        radian_t phi = 2.0f * beta - std::asin(n * std::sin(beta));

//        float a = cot(phi / 2);
//        float b = a * a + 1;
//        float c = a - n / (2 * std::sin(phi / 2));
//        float x = (-std::sqrt(b * b - 4 * a * c) - b) / (2 * a);
//        float beta_ = std::atan(1/x);
//        radian_t beta_ = incident(n, phi);
//        std::cout << beta << ", " << phi << ", " << beta_ << std::endl;

//        std::cout << 2 * sin(beta - phi / 2) * cos(beta - phi / 2) - n * sin(beta) << std::endl;

//std::cout << 2 * (sin(beta)*cos(phi/2)-cos(beta)*sin(phi/2)) * cos(beta - phi / 2) - n * sin(beta) << std::endl;
//std::cout << (sin(beta)*cos(phi/2)-cos(beta)*sin(phi/2)) * (cos(beta)*cos(phi/2)+sin(beta)*sin(phi/2)) - (n/2) * sin(beta) << std::endl;
//std::cout << (sin(beta)*cos(phi/2)-cos(beta)*sin(phi/2)) * (cot(beta)*cos(phi/2)+sin(phi/2)) - (n/2) << std::endl;


//std::cout << (sin(beta) * a - cos(beta) * b) * (cos(beta)/sin(beta) * a + b) - (n / 2) << std::endl;
//std::cout << (sin(beta) - c * cos(beta)) * (cos(beta) / sin(beta) + c) + d << std::endl;
//std::cout << sin(beta)*(cos(beta) / sin(beta) + c) - c * cos(beta)*(cos(beta) / sin(beta) + c) + d << std::endl;
//std::cout << cos(beta)*sin(beta) + sin(beta)*sin(beta) * c - c * cos(beta) * cos(beta) - c * c * cos(beta)*sin(beta) + d*sin(beta) << std::endl;
//std::cout << cos(beta)*sin(beta) + c*sin(beta)*sin(beta) - c*cos(beta)*cos(beta) - c*c*cos(beta)*sin(beta) + d*sin(beta) << std::endl;


//std::cout << (1 - c * c) * x * sqrt(1 - x * x)
//             - 2 * c * x * x
//             + d * sqrt(1 - x * x)
//             + c
//          << std::endl;


//        std::cout <<
//            sqrt(1 - x * x) * (d + (1 - c * c) * x)
//            - 2 * c * x * x
//            + c
//        << std::endl;


//std::cout << (1 - x * x) *  (d + e * x) * (d + e * x) - (2 * c * x * x - c) * (2 * c * x * x - c) << std::endl;
//std::cout << (d + e * x) * (d + e * x) - x * x * (d + e * x) * (d + e * x) - (2 * c * x * x - c) * (2 * c * x * x - c) << std::endl;

//std::cout <<  d*d +e*e*x*x + 2*d*e*x - x*x*(d*d +e*e*x*x + 2*d*e*x) - 4*c*c*x*x*x*x - c*c + 4*c*c*x*x << std::endl;

//std::cout <<  d*d +e*e*x*x + 2*d*e*x - d*d*x*x - e*e*x*x*x*x - 2*d*e*x*x*x - 4*c*c*x*x*x*x - c*c + 4*c*c*x*x << std::endl;

        float const a = cos(phi / 2);
        float const c = tan(phi / 2);
        float const d = -n / (2 * a * a);
        float const e = 1 - c * c;
        float const A = 4 * c * c + e * e;
        float const B = 2 * d * e;
        float const C = d * d - e * e - 4 * c * c;
        float const D = c * c - d * d;
//        float x = cos(beta);

        auto func = [A,B,C,D](float x) {
            return A*x*x*x*x + B*x*x*x + C*x*x - B*x + D;
        };

        auto dfunc = [A,B,C](float x) {
            return 4*A*x*x*x + 3*B*x*x + 2*C*x - B;
        };

//        std::cout << phi << ", " << beta << ", " << func(cos(beta)) << std::endl;

        float r = 1;
        for (size_t i = 0; i < 4; i++) {
            r = r - func(r) / dfunc(r);
        }
        float const beta = acos(r);

        radian_t const phi_ = 2.0f * beta - std::asin(n * std::sin(beta));

//        std::cout << r << ", " << phi << ", " << phi_ << ", " << abs(phi-phi_) << std::endl;




    }
//    return;

    uint32_t* texels = new uint32_t[angleCount];
    auto* image = tga_new(angleCount, 1);

    auto work = [=](uint32_t start, uint32_t count) {
        for (size_t i = 0; i < count; i++) {
            size_t const index = start + i;
            float const s = (float(index) + 0.5f) / float(angleCount);
            float const two_phi = minAngle + (maxAngle - minAngle) * s;
            float3  c = generate(two_phi/2);

            c = linear_to_sRGB(c);
            std::cout << c.r << ", " << c.g << ", " << c.b << std::endl;

            uint3 const rgb = uint3(saturate(c) * 255);
            texels[index] = rgb.r | (rgb.g << 8) | (rgb.b << 16);

            tga_set_pixel(image, index, 0, {
                    .b = (uint8_t)rgb.b,
                    .g = (uint8_t)rgb.g,
                    .r = (uint8_t)rgb.r
            });

        }
    };

    auto* renderableJob = jobs::parallel_for(js, nullptr,
            0, angleCount,
            std::cref(work),
            jobs::CountSplitter<64, 0>());

    js.runAndWait(renderableJob);


    tga_write("toto.tga", image);
    tga_free(image);

    delete [] texels;
}

float3 RainbowGenerator::generate(radian_t phi) const noexcept {
    float y = 0.0f;
    float3 f0 = 0.0f;
    for (size_t i = 0; i < CIE_XYZ_COUNT; i++) {
        // Current wavelength
        float const w = float(CIE_XYZ_START + i);
        float const d65 = illuminantD65(w);

        float const n = iorFromWavelength(w, mTemprature);

        if (2*phi >= maxIncidentAngle(n)) {
            y += CIE_XYZ[i].y * d65;
            continue;
        }

        auto func = [n, phi](float x){
            return 2.0f * x - std::asin(n * std::sin(x)) - phi;
        };
        auto dfunc = [n](float x){
            return 2 - n * std::cos(x) / std::sqrt(1 - n * n * sin(x) * sin(x));
        };

//        float const a = cos(phi / 2);
//        float const c = tan(phi / 2);
//        float const d = -n / (2 * a * a);
//        float const e = 1 - c * c;
//        float const A = 4 * c * c + e * e;
//        float const B = 2 * d * e;
//        float const C = d * d - e * e - 4 * c * c;
//        float const D = c * c - d * d;
//        auto func = [A,B,C,D](float x) {
//            return A*x*x*x*x + B*x*x*x + C*x*x - B*x + D;
//        };
//        auto dfunc = [A,B,C](float x) {
//            return 4*A*x*x*x + 3*B*x*x + 2*C*x - B;
//        };
        float r = phi / 2.0f;
        for (size_t j = 0; j < 8; j++) {
            r = r - func(r) / dfunc(r);
        }
//        float const beta = acos(r);
        float const beta = r;

        if (std::abs(std::sin(beta) * n) >= 1) {
            y += CIE_XYZ[i].y * d65;
            continue;
        }

        float const phi_ = 2.0f * beta - std::asin(n * std::sin(beta));
        std::cout << abs(phi_-phi) << std::endl;


        float const Bi = beta;
        float const Bt_w = std::asin(std::sin(Bi) * n);

        float const R_w = fresnel(Bi, Bt_w);
        float const T_w = 1.0f - R_w;

        float const R_a = fresnel(Bt_w, Bi);
        float const T_a = 1.0f - R_a;

        f0 += (T_w * R_a * T_a) * CIE_XYZ[i] * d65;
        y += CIE_XYZ[i].y * d65;
    }
    f0  /= y;
    f0  = XYZ_to_sRGB(f0*8);
    f0 = saturate(f0);
    return f0;
}

float RainbowGenerator::fresnel(radian_t Bi, radian_t Bt) noexcept {
    if (Bi == 0.0f) return 0.0214;
    float const s = std::sin(Bi - Bt) / std::sin(Bi + Bt);
    float const t = std::tan(Bi - Bt) / std::tan(Bi + Bt);
    float const R = 0.5f * (s * s + t * t);
    return R;
}

float RainbowGenerator::iorFromWavelength(nanometer_t wavelength,
        celcius_t temperature) noexcept {

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

auto RainbowGenerator::lowestAngle(float n) noexcept -> radian_t {
    float const beta = maxIncidentAngle(n);
    float const phi = 2.0f * beta - std::asin(n * std::sin(beta));
    return 2.0f * phi;
}

auto RainbowGenerator::incident(float n, radian_t phi) noexcept -> radian_t {
    return std::atan( std::sin(phi/2) / (std::cos(phi/2) - n/2) );
}

auto RainbowGenerator::maxIncidentAngle(float n) noexcept -> radian_t {
    float const beta = std::acos(2.0f * std::sqrt(n * n - 1.0f) / (n * std::sqrt(3.0f)));
    return beta;
}


// ------------------------------------------------------------------------------------------------

float RainbowGenerator::illuminantD65(float w) noexcept {
    auto i0 = size_t((w - CIE_D65_START) / CIE_D65_INTERVAL);
    uint2 const indexBounds{i0, std::min(i0 + 1, CIE_D65_END)};
    float2 const wavelengthBounds = CIE_D65_START + float2{indexBounds} * CIE_D65_INTERVAL;
    float const t = (w - wavelengthBounds.x) / (wavelengthBounds.y - wavelengthBounds.x);
    return lerp(CIE_D65[indexBounds.x], CIE_D65[indexBounds.y], t);
}

float3 RainbowGenerator::XYZ_to_sRGB(const float3& v) noexcept {
    const mat3f XYZ_sRGB{
            3.2404542f, -0.9692660f,  0.0556434f,
            -1.5371385f,  1.8760108f, -0.2040259f,
            -0.4985314f,  0.0415560f,  1.0572252f
    };
    return XYZ_sRGB * v;
}

float3 RainbowGenerator::linear_to_sRGB(float3 linear) noexcept {
    float3 sRGB{ linear };
    for (size_t i = 0; i < sRGB.size(); i++) {
        sRGB[i] = (sRGB[i] <= 0.0031308f) ?
                  sRGB[i] * 12.92f : (powf(sRGB[i], 1.0f / 2.4f) * 1.055f) - 0.055f;
    }
    return sRGB;
}


#define TARGALIB_IMPLEMENTATION
#include "targa.h"