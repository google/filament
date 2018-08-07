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

#ifndef IMAGE_UTILITIES_H_
#define IMAGE_UTILITIES_H_

namespace image {

template <typename T>
inline T clamp(T v, T min, T max) {
    return std::max(T(min), std::min(v, T(max)));
}

template <typename T>
inline T saturate(T v) {
    return clamp(v, T(0), T(1));
}

template <typename T>
inline math::float4 linearToRGBM(const T& linear) {
    using math::float4;

    float4 RGBM(linear[0], linear[1], linear[2], 1.0f);

    // Linear to gamma space
    RGBM.rgb = sqrt(RGBM.rgb);
    // Set the range
    RGBM.rgb /= 16.0f;

    float maxComponent = std::max(std::max(RGBM.r, RGBM.g), std::max(RGBM.b, 1e-6f));
    // Don't let M go below 1 in the [0..16] range
    RGBM.a = clamp(maxComponent, 1.0f / 16.0f, 1.0f);
    RGBM.a = std::ceil(RGBM.a * 255.0f) / 255.0f;

    RGBM.rgb = saturate(RGBM.rgb / RGBM.a);

    return RGBM;
}

template <typename T>
inline math::float3 RGBMtoLinear(const T& rgbm) {
    using math::float3;

    float3 linear(rgbm[0], rgbm[1], rgbm[2]);
    linear *= rgbm.a * 16.0f;
    // Gamma to linear space
    return linear * linear;
}

template <typename T>
inline math::float3 linearTosRGB(const T& linear) {
    using math::float3;
    constexpr float a = 0.055f;
    constexpr float a1 = 1.055f;
    constexpr float p = 1 / 2.4f;
    float3 sRGB;
    for (size_t i=0 ; i<3 ; i++) {
        if (linear[i] <= 0.0031308f) {
            sRGB[i] = linear[i] * 12.92f;
        } else {
            sRGB[i] = a1 * std::pow(linear[i], p) - a;
        }
    }
    return sRGB;
}

inline float linearTosRGB(float linear) {
    if (linear <= 0.0031308f) {
        return linear * 12.92f;
    } else {
        constexpr float a = 0.055f;
        constexpr float a1 = 1.055f;
        constexpr float p = 1 / 2.4f;
        return a1 * std::pow(linear, p) - a;
    }
}

template<typename T>
T sRGBToLinear(const T& sRGB);

template<>
inline math::float3 sRGBToLinear(const math::float3& sRGB) {
    using math::float3;
    constexpr float a = 0.055f;
    constexpr float a1 = 1.055f;
    constexpr float p = 2.4f;
    float3 linear;
    for (size_t i=0 ; i<3 ; i++) {
        if (sRGB[i] <= 0.04045f) {
            linear[i] = sRGB[i] * (1.0f / 12.92f);
        } else {
            linear[i] = std::pow((sRGB[i] + a) / a1, p);
        }
    }
    return linear;
}

template<>
inline math::float4 sRGBToLinear(const math::float4& sRGB) {
    using math::float4;
    constexpr float a = 0.055f;
    constexpr float a1 = 1.055f;
    constexpr float p = 2.4f;
    float4 linear;
    for (size_t i=0 ; i<3 ; i++) {
        if (sRGB[i] <= 0.04045f) {
            linear[i] = sRGB[i] * (1.0f / 12.92f);
        } else {
            linear[i] = std::pow((sRGB[i] + a) / a1, p);
        }
    }
    linear[3] = sRGB[3];
    return linear;
}

}

#endif // IMAGE_UTILITIES_H_

