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

#include <image/LinearImage.h>

#include <utils/compiler.h>

#include <math/scalar.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/half.h>

#include <algorithm>
#include <memory>

namespace image {

template <typename T>
uint32_t linearToRGB_10_11_11_REV(const T& linear) {
    using fp11 = filament::math::fp<0, 5, 6>;
    using fp10 = filament::math::fp<0, 5, 5>;
    // the max value for a RGB_11_11_10 is {65024, 65024, 64512} :  (2 - 2^-M) * 2^(E-1)
    // we clamp to the min of that
    fp11 r = fp11::fromf(std::min(64512.0f, linear[0]));
    fp11 g = fp11::fromf(std::min(64512.0f, linear[1]));
    fp10 b = fp10::fromf(std::min(64512.0f, linear[2]));
    uint32_t ir = r.bits & 0x7FF;
    uint32_t ig = g.bits & 0x7FF;
    uint32_t ib = b.bits & 0x3FF;
    return (ib << 22) | (ig << 11) | ir;
}

template <typename T>
inline filament::math::float4 linearToRGBM(const T& linear) {
    using filament::math::float4;

    float4 RGBM(linear[0], linear[1], linear[2], 1.0f);

    // Linear to gamma space
    RGBM.rgb = sqrt(RGBM.rgb);
    // Set the range
    RGBM.rgb /= 16.0f;

    float maxComponent = std::max(std::max(RGBM.r, RGBM.g), std::max(RGBM.b, 1e-6f));
    // Don't let M go below 1 in the [0..16] range
    RGBM.a =  filament::math::clamp(maxComponent, 1.0f / 16.0f, 1.0f);
    RGBM.a = std::ceil(RGBM.a * 255.0f) / 255.0f;

    RGBM.rgb = saturate(RGBM.rgb / RGBM.a);

    return RGBM;
}

template <typename T>
inline filament::math::float3 RGBMtoLinear(const T& rgbm) {
    using filament::math::float3;

    float3 linear(rgbm[0], rgbm[1], rgbm[2]);
    linear *= rgbm.a * 16.0f;
    // Gamma to linear space
    return linear * linear;
}

template <typename T>
inline filament::math::float3 linearTosRGB(const T& linear) {
    using filament::math::float3;
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
inline filament::math::float3 sRGBToLinear(const filament::math::float3& sRGB) {
    using filament::math::float3;
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
inline filament::math::float4 sRGBToLinear(const filament::math::float4& sRGB) {
    using filament::math::float4;
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
T linearToSRGB(const T& color);

template<>
inline filament::math::float3 linearToSRGB(const filament::math::float3& color) {
    using filament::math::float3;
    float3 sRGBColor{color};
    #pragma nounroll
    for (size_t i = 0; i < sRGBColor.size(); i++) {
        sRGBColor[i] = (sRGBColor[i] <= 0.0031308f) ?
                sRGBColor[i] * 12.92f : (powf(sRGBColor[i], 1.0f / 2.4f) * 1.055f) - 0.055f;
    }
    return sRGBColor;
}

// Creates a n-channel sRGB image from a linear floating-point image.
// The source image can have more than N channels, but only the first N are converted to sRGB.
template<typename T, int N = 3>
std::unique_ptr<uint8_t[]> fromLinearTosRGB(const LinearImage& image) {
    const size_t w = image.getWidth();
    const size_t h = image.getHeight();
    const size_t nchan = image.getChannels();
    assert(nchan >= N);
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * N * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
    for (size_t y = 0; y < h; ++y) {
        float const* p = image.getPixelRef(0, y);
        for (size_t x = 0; x < w; ++x, p += nchan, d += N) {
            for (int n = 0; n < N; n++) {
                float source = n < 3 ? linearTosRGB(p[n]) : p[n];
                float target =  filament::math::saturate(source) * std::numeric_limits<T>::max() + 0.5f;
                d[n] = T(target);
            }
        }
    }
    return dst;
}

// Creates a N-channel RGB u8 image from a f32 image.
template<typename T, int N = 3>
std::unique_ptr<uint8_t[]> fromLinearToRGB(const LinearImage& image) {
    size_t w = image.getWidth();
    size_t h = image.getHeight();
    size_t channels = image.getChannels();
    assert(channels >= N);
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * N * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
    for (size_t y = 0; y < h; ++y) {
        float const* p = image.getPixelRef(0, y);
        for (size_t x = 0; x < w; ++x, p += channels, d += N) {
            for (int n = 0; n < N; n++) {
                float target =  filament::math::saturate(p[n]) * std::numeric_limits<T>::max() + 0.5f;
                d[n] = T(target);
            }
        }
    }
    return dst;
}

// Creates a 4-channel RGBM u8 image from a f32 image.
// The source image can have three or more channels, but only the first three are honored.
template <typename T>
std::unique_ptr<uint8_t[]> fromLinearToRGBM(const LinearImage& image) {
    using namespace filament::math;
    size_t w = image.getWidth();
    size_t h = image.getHeight();
    UTILS_UNUSED_IN_RELEASE size_t channels = image.getChannels();
    assert(channels >= 3);
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * 4 * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x, d += 4) {
            auto src = image.get<float3>((uint32_t) x, (uint32_t) y);
            float4 l(linearToRGBM(*src) * std::numeric_limits<T>::max() + 0.5f);
            for (size_t i = 0; i < 4; i++) {
                d[i] = T(l[i]);
            }
        }
    }
    return dst;
}

// Creates a 3-channel RGB_10_11_11_REV image from a f32 image.
// The source image can have three or more channels, but only the first three are honored.
inline std::unique_ptr<uint8_t[]> fromLinearToRGB_10_11_11_REV(const LinearImage& image) {
    using namespace filament::math;
    size_t w = image.getWidth();
    size_t h = image.getHeight();
    UTILS_UNUSED_IN_RELEASE size_t channels = image.getChannels();
    assert(channels >= 3);
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * sizeof(uint32_t)]);
    uint8_t* d = dst.get();
    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x, d += sizeof(uint32_t)) {
            auto src = image.get<float3>((uint32_t)x, (uint32_t)y);
            uint32_t v = linearToRGB_10_11_11_REV(*src);
            *reinterpret_cast<uint32_t*>(d) = v;
        }
    }
    return dst;
}

// Creates a packed single-channel integer-based image from a floating-point image.
// For example if T is uint8_t, then this performs a transformation from [0,1] to [0,255].
template <typename T>
std::unique_ptr<uint8_t[]> fromLinearToGrayscale(const LinearImage& image) {
    const size_t w = image.getWidth();
    const size_t h = image.getHeight();
    assert(image.getChannels() == 1);
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
    for (size_t y = 0; y < h; ++y) {
        float const* p = image.getPixelRef(0, y);
        for (size_t x = 0; x < w; ++x, ++p, ++d) {
            const float gray =  filament::math::saturate(*p) * std::numeric_limits<T>::max() + 0.5f;
            d[0] = T(gray);
        }
    }
    return dst;
}

// Constructs a 3-channel LinearImage from an untyped data blob.
// The "proc" lambda converts a single color component into a float.
// The "transform" lambda performs an arbitrary float-to-float transformation.
template<typename T, typename PROCESS, typename TRANSFORM>
static LinearImage toLinear(size_t w, size_t h, size_t bpr,
            const uint8_t* src, PROCESS proc, TRANSFORM transform) {
    LinearImage result((uint32_t) w, (uint32_t) h, 3);
    auto d = result.get< filament::math::float3>();
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 3) {
             filament::math::float3 sRGB(proc(p[0]), proc(p[1]), proc(p[2]));
            sRGB /= std::numeric_limits<T>::max();
            *d++ = transform(sRGB);
        }
    }
    return result;
}

// Constructs a 3-channel LinearImage from an untyped data blob.
// The "proc" lambda converts a single color component into a float.
// The "transform" lambda performs an arbitrary float-to-float transformation.
template<typename T, typename PROCESS, typename TRANSFORM>
static LinearImage toLinear(size_t w, size_t h, size_t bpr,
        const std::unique_ptr<uint8_t[]>& src, PROCESS proc, TRANSFORM transform) {
    return toLinear<T>(w, h, bpr, src.get(), proc, transform);
}

// Constructs a 4-channel LinearImage from an untyped data blob.
// The "proc" lambda converts a single color component into a float.
// the "transform" lambda performs an arbitrary float-to-float transformation.
template<typename T, typename PROCESS, typename TRANSFORM>
static LinearImage toLinearWithAlpha(size_t w, size_t h, size_t bpr,
        const uint8_t* src, PROCESS proc, TRANSFORM transform) {
    LinearImage result((uint32_t) w, (uint32_t) h, 4);
    auto d = result.get< filament::math::float4>();
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 4) {
             filament::math::float4 sRGB(proc(p[0]), proc(p[1]), proc(p[2]), proc(p[3]));
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
    return toLinearWithAlpha<T>(w, h, bpr, src.get(), proc, transform);
}

// Constructs a 3-channel LinearImage from RGBM data.
inline LinearImage toLinearFromRGBM( filament::math::float4 const* src, uint32_t w, uint32_t h) {
    LinearImage result(w, h, 3);
    auto dst = result.get< filament::math::float3>();
    for (uint32_t row = 0; row < h; ++row) {
        for (uint32_t col = 0; col < w; ++col, ++src, ++dst) {
            *dst = RGBMtoLinear(*src);
        }
    }
    return result;
}

inline LinearImage fromLinearToRGBM(const LinearImage& image) {
    assert(image.getChannels() == 3);
    const uint32_t w = image.getWidth(), h = image.getHeight();
    LinearImage result(w, h, 4);
    auto src = image.get< filament::math::float3>();
    auto dst = result.get< filament::math::float4>();
    for (uint32_t row = 0; row < h; ++row) {
        for (uint32_t col = 0; col < w; ++col, ++src, ++dst) {
            *dst = linearToRGBM(*src);
        }
    }
    return result;
}

}  // namespace Image

#endif // IMAGE_COLORTRANSFORM_H_
