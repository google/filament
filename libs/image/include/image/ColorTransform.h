/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef IMAGE_COLORTRANSFORM_H_
#define IMAGE_COLORTRANSFORM_H_

#include <image/Image.h>
#include <image/LinearImage.h>

#include <utils/compiler.h>

#include <math/scalar.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <memory>

namespace image {

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
    RGBM.a = math::clamp(maxComponent, 1.0f / 16.0f, 1.0f);
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

template<typename T>
T linearToSRGB(const T& sRGB);

template<>
inline math::float3 linearToSRGB(const math::float3& color) {
    using math::float3;
    float3 sRGBColor{color};
    #pragma nounroll
    for (size_t i = 0; i < sRGBColor.size(); i++) {
        sRGBColor[i] = (sRGBColor[i] <= 0.0031308f) ?
                sRGBColor[i] * 12.92f : (powf(sRGBColor[i], 1.0f / 2.4f) * 1.055f) - 0.055f;
    }
    return sRGBColor;
}

template <typename T>
std::unique_ptr<uint8_t[]> fromLinearTosRGB(const Image& image) {
    using math::float3;
    size_t w = image.getWidth();
    size_t h = image.getHeight();
    size_t channels = image.getChannelsCount();
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * 3 * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
    for (size_t y = 0; y < h; ++y) {
        float3 const* p = static_cast<float3 const*>(image.getPixelRef(0, y));
        for (size_t x = 0; x < w; ++x, ++p, d += channels) {
            float3 l(linearTosRGB(saturate(*p)) * std::numeric_limits<T>::max());
            for (size_t i = 0; i < 3; i++) {
                d[i] = T(l[i]);
            }
        }
    }
    return dst;
}

template <typename T>
std::unique_ptr<uint8_t[]> fromLinearToRGB(const Image& image) {
    using math::float3;
    size_t w = image.getWidth();
    size_t h = image.getHeight();
    size_t channels = image.getChannelsCount();
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * channels * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
    for (size_t y = 0; y < h; ++y) {
        float3 const* p = static_cast<float3 const*>(image.getPixelRef(0, y));
        for (size_t x = 0; x < w; ++x, ++p, d += channels) {
            float3 l(saturate(*p) * std::numeric_limits<T>::max());
            for (size_t i = 0; i < channels; i++) {
                d[i] = T(l[i]);
            }
        }
    }
    return dst;
}

template <typename T>
std::unique_ptr<uint8_t[]> fromLinearToRGBM(const Image& image) {
    using namespace math;
    size_t w = image.getWidth();
    size_t h = image.getHeight();
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * 4 * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
    for (size_t y = 0; y < h; ++y) {
        float3 const* p = static_cast<float3 const*>(image.getPixelRef(0, y));
        for (size_t x = 0; x < w; ++x, ++p, d += 4) {
            float4 l(linearToRGBM(*p) * std::numeric_limits<T>::max());
            for (size_t i = 0; i < 4; i++) {
                d[i] = T(l[i]);
            }
        }
    }
    return dst;
}

// Creates a 3-channel sRGB u8 image from a linear f32 image.
// The source image can have three or more channels, but only the first three are honored.
template <typename T>
std::unique_ptr<uint8_t[]> fromLinearTosRGB(const LinearImage& image) {
    using math::float3;
    size_t w = image.getWidth();
    size_t h = image.getHeight();
    UTILS_UNUSED_IN_RELEASE size_t channels = image.getChannels();
    assert(channels >= 3);
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * 3 * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x, d += 3) {
            float3 const* src = reinterpret_cast<float3 const*>(image.getPixelRef(x, y));
            float3 l(linearTosRGB(saturate(*src)) * std::numeric_limits<T>::max());
            for (size_t i = 0; i < 3; i++) {
                d[i] = T(l[i]);
            }
        }
    }
    return dst;
}

// Creates a 3-channel RGB u8 image from a f32 image.
// The source image can have three or more channels, but only the first three are honored.
template <typename T>
std::unique_ptr<uint8_t[]> fromLinearToRGB(const LinearImage& image) {
    using math::float3;
    size_t w = image.getWidth();
    size_t h = image.getHeight();
    UTILS_UNUSED_IN_RELEASE size_t channels = image.getChannels();
    assert(channels >= 3);
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * 3 * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x, d += 3) {
            float3 const* src = reinterpret_cast<float3 const*>(image.getPixelRef(x, y));
            float3 l(saturate(*src) * std::numeric_limits<T>::max());
            for (size_t i = 0; i < 3; i++) {
                d[i] = T(l[i]);
            }
        }
    }
    return dst;
}

// Creates a 4-channel RGBM u8 image from a f32 image.
// The source image can have three or more channels, but only the first three are honored.
template <typename T>
std::unique_ptr<uint8_t[]> fromLinearToRGBM(const LinearImage& image) {
    using namespace math;
    size_t w = image.getWidth();
    size_t h = image.getHeight();
    UTILS_UNUSED_IN_RELEASE size_t channels = image.getChannels();
    assert(channels >= 3);
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * 4 * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x, d += 4) {
            float3 const* src = reinterpret_cast<float3 const*>(image.getPixelRef(x, y));
            float4 l(linearToRGBM(*src) * std::numeric_limits<T>::max());
            for (size_t i = 0; i < 4; i++) {
                d[i] = T(l[i]);
            }
        }
    }
    return dst;
}

// Constructs a 3-channel LinearImage from an untyped data blob.
// The "proc" lambda converts a single color component into a float.
// The "transform" lambda performs an arbitrary float-to-float transformation.
template<typename T, typename PROCESS, typename TRANSFORM>
static LinearImage toLinear(size_t w, size_t h, size_t bpr,
        const std::unique_ptr<uint8_t[]>& src, PROCESS proc, TRANSFORM transform) {
    LinearImage result(w, h, 3);
    math::float3* d = reinterpret_cast<math::float3*>(result.getPixelRef());
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src.get() + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 3) {
            math::float3 sRGB(proc(p[0]), proc(p[1]), proc(p[2]));
            sRGB /= std::numeric_limits<T>::max();
            *d++ = transform(sRGB);
        }
    }
    return result;
}

// Constructs a 4-channel LinearImage from an untyped data blob.
// The "proc" lambda converts a single color component into a float.
// the "transform" lambda performs an arbitrary float-to-float transformation.
template<typename T, typename PROCESS, typename TRANSFORM>
static LinearImage toLinearWithAlpha(size_t w, size_t h, size_t bpr,
        const std::unique_ptr<uint8_t[]>& src, PROCESS proc, TRANSFORM transform) {
    LinearImage result(w, h, 4);
    math::float4* d = reinterpret_cast<math::float4*>(result.getPixelRef());
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src.get() + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 4) {
            math::float4 sRGB(proc(p[0]), proc(p[1]), proc(p[2]), proc(p[3]));
            sRGB /= std::numeric_limits<T>::max();
            *d++ = transform(sRGB);
        }
    }
    return result;
}

}

#endif // IMAGE_COLORTRANSFORM_H_
