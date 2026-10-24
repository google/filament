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

#include <math/half.h>
#include <math/scalar.h>
#include <math/vec3.h>
#include <math/vec4.h>

#if defined(__ARM_NEON)
#include <arm_neon.h>
#endif

#include <algorithm>
#include <memory>

using namespace filament;

namespace image {

// Only called when __ARM_NEON is not defined (unsupported).
template <typename T>
uint32_t linearToRGB_10_11_11_REV(const T& linear) {
    using fp11 = math::fp<0, 5, 6>;
    using fp10 = math::fp<0, 5, 5>;
    // the max value for a RGB_11_11_10 is {65024, 65024, 64512} :  (2 - 2^-M) * 2^(E-1)
    // we clamp to the min of that
    const fp11 r = fp11::fromf(std::min(64512.0f, linear[0]));
    const fp11 g = fp11::fromf(std::min(64512.0f, linear[1]));
    const fp10 b = fp10::fromf(std::min(64512.0f, linear[2]));
    const uint32_t ir = r.bits & 0x7FF;
    const uint32_t ig = g.bits & 0x7FF;
    const uint32_t ib = b.bits & 0x3FF;
    return (ib << 22) | (ig << 11) | ir;
}

template <typename T>
math::float4 linearToRGBM(const T& linear) {
    using math::float4;

    float4 RGBM(linear[0], linear[1], linear[2], 1.0f);

    // Linear to gamma space
    RGBM.rgb = sqrt(RGBM.rgb);
    // Set the range
    RGBM.rgb /= 16.0f;

    float maxComponent = std::max(std::max(RGBM.r, RGBM.g), std::max(RGBM.b, 1e-6f));
    // Don't let M go below 1 in the [0..16] range
    RGBM.a =  math::clamp(maxComponent, 1.0f / 16.0f, 1.0f);
    RGBM.a = std::ceil(RGBM.a * 255.0f) / 255.0f;

    RGBM.rgb = saturate(RGBM.rgb / RGBM.a);

    return RGBM;
}

template <typename T>
math::float3 RGBMtoLinear(const T& rgbm) {
    using math::float3;

    float3 linear(rgbm[0], rgbm[1], rgbm[2]);
    linear *= rgbm.a * 16.0f;
    // Gamma to linear space
    return linear * linear;
}

template <typename T>
math::float3 linearToSRGB(const T& linear) {
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

template<>
math::float3 linearToSRGB(const math::float3& color) {
    using math::float3;
    float3 sRGBColor{color};
    UTILS_NOUNROLL
    for (size_t i = 0; i < sRGBColor.size(); i++) {
        sRGBColor[i] = sRGBColor[i] <= 0.0031308f ?
                sRGBColor[i] * 12.92f : powf(sRGBColor[i], 1.0f / 2.4f) * 1.055f - 0.055f;
    }
    return sRGBColor;
}

inline float linearToSRGB(float linear) {
    if (linear <= 0.0031308f) {
        return linear * 12.92f;
    }
    constexpr float a = 0.055f;
    constexpr float a1 = 1.055f;
    constexpr float p = 1 / 2.4f;
    return a1 * std::pow(linear, p) - a;
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
    for (size_t i = 0; i < 3; i++) {
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
    for (size_t i = 0; i < 3; i++) {
        if (sRGB[i] <= 0.04045f) {
            linear[i] = sRGB[i] * (1.0f / 12.92f);
        } else {
            linear[i] = std::pow((sRGB[i] + a) / a1, p);
        }
    }
    linear[3] = sRGB[3];
    return linear;
}

// Creates a N-channel sRGB image from a linear floating-point image.
// The source image can have more than N channels, but only the first 3 are converted to sRGB.
template<typename T, int N = 3>
std::unique_ptr<uint8_t[]> fromLinearToSRGB(const LinearImage& image) {
    const size_t w = image.getWidth();
    const size_t h = image.getHeight();
    const size_t channels = image.getChannels();
    assert(channels >= N);
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * N * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
    for (size_t y = 0; y < h; ++y) {
        float const* p = image.getPixelRef(0, y);
        for (size_t x = 0; x < w; ++x, p += channels, d += N) {
            for (int n = 0; n < N; n++) {
                const float source = n < 3 ? linearToSRGB(p[n]) : p[n];
                float target =  math::saturate(source) * std::numeric_limits<T>::max() + 0.5f;
                d[n] = T(target);
            }
        }
    }
    return dst;
}

// Creates a N-channel RGB u8 image from a f32 image.
template<typename T, int N = 3>
std::unique_ptr<uint8_t[]> fromLinearToRGB(const LinearImage& image) {
    const size_t w = image.getWidth();
    const size_t h = image.getHeight();
    const size_t channels = image.getChannels();
    assert(channels >= N);
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * N * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
    for (size_t y = 0; y < h; ++y) {
        float const* p = image.getPixelRef(0, y);
        for (size_t x = 0; x < w; ++x, p += channels, d += N) {
            for (int n = 0; n < N; n++) {
                float target =  math::saturate(p[n]) * std::numeric_limits<T>::max() + 0.5f;
                d[n] = T(target);
            }
        }
    }
    return dst;
}

// Creates a 3-channel RGB_10_11_11_REV image from a f32 image.
// The source image can have three or more channels, but only the first three are honored.
inline std::unique_ptr<uint8_t[]> fromLinearToRGB_10_11_11_REV(const LinearImage& image) {
    using namespace filament::math;
    const size_t w = image.getWidth();
    const size_t h = image.getHeight();
    const size_t channels = image.getChannels();
    assert(channels == 3 || channels == 4);
    const size_t count = w * h;
    std::unique_ptr<uint8_t[]> dst(new uint8_t[count * sizeof(uint32_t)]);
    uint32_t* UTILS_RESTRICT d = reinterpret_cast<uint32_t*>(dst.get());
    float const* UTILS_RESTRICT p = image.getPixelRef();

    size_t i = 0;
#if defined(__ARM_NEON)
    const float32x4_t maxVal = vdupq_n_f32(64512.0f);
    const float32x4_t zero = vdupq_n_f32(0.0f);
    const uint32x4_t mask11 = vdupq_n_u32(0xffff0000u);
    const uint32x4_t mask10 = vdupq_n_u32(0xfffe0000u);
    const uint32x4_t round11 = vdupq_n_u32(0x00010000u);
    const uint32x4_t round10 = vdupq_n_u32(0x00020000u);
    constexpr float magic = 0x1.0p-112f;

    auto convert4 = [&](float32x4_t r, float32x4_t g, float32x4_t b) -> uint32x4_t {
        r = vmaxq_f32(vminq_f32(r, maxVal), zero);
        g = vmaxq_f32(vminq_f32(g, maxVal), zero);
        b = vmaxq_f32(vminq_f32(b, maxVal), zero);

        uint32x4_t ir = vandq_u32(vreinterpretq_u32_f32(r), mask11);
        uint32x4_t ig = vandq_u32(vreinterpretq_u32_f32(g), mask11);
        uint32x4_t ib = vandq_u32(vreinterpretq_u32_f32(b), mask10);

        ir = vaddq_u32(ir, round11);
        ig = vaddq_u32(ig, round11);
        ib = vaddq_u32(ib, round10);

        ir = vreinterpretq_u32_f32(vmulq_n_f32(vreinterpretq_f32_u32(ir), magic));
        ig = vreinterpretq_u32_f32(vmulq_n_f32(vreinterpretq_f32_u32(ig), magic));
        ib = vreinterpretq_u32_f32(vmulq_n_f32(vreinterpretq_f32_u32(ib), magic));

        ir = vshrq_n_u32(ir, 17);
        ig = vshrq_n_u32(ig, 17);
        ib = vshrq_n_u32(ib, 18);

        return vsliq_n_u32(vsliq_n_u32(ir, ig, 11), ib, 22);
    };

    if (channels == 3) {
        for (; i + 4 <= count; i += 4, p += 12, d += 4) {
            const float32x4x3_t rgb = vld3q_f32(p);
            vst1q_u32(d, convert4(rgb.val[0], rgb.val[1], rgb.val[2]));
        }
    } else {
        for (; i + 4 <= count; i += 4, p += 16, d += 4) {
            const float32x4x4_t rgba = vld4q_f32(p);
            vst1q_u32(d, convert4(rgba.val[0], rgba.val[1], rgba.val[2]));
        }
    }
#endif
    for (; i < count; ++i, p += channels, ++d) {
        *d = linearToRGB_10_11_11_REV(p);
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
            const float gray =  math::saturate(*p) * std::numeric_limits<T>::max() + 0.5f;
            d[0] = T(gray);
        }
    }
    return dst;
}

// Creates a 4-channel RGBM u8 image from a f32 image.
// The source image can have three or more channels, but only the first three are honored.
template <typename T>
std::unique_ptr<uint8_t[]> fromLinearToRGBM(const LinearImage& image) {
    using namespace filament::math;
    const size_t w = image.getWidth();
    const size_t h = image.getHeight();
    assert(image.getChannels() >= 3);
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * 4 * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x, d += 4) {
            const auto src = image.get<float3>(uint32_t(x), uint32_t(y));
            float4 l(linearToRGBM(*src) * std::numeric_limits<T>::max() + 0.5f);
            for (size_t i = 0; i < 4; i++) {
                d[i] = T(l[i]);
            }
        }
    }
    return dst;
}

// Constructs a 3-channel LinearImage from RGBM data.
inline LinearImage toLinearFromRGBM( math::float4 const* src, uint32_t w, uint32_t h) {
    LinearImage result(w, h, 3);
    auto dst = result.get< math::float3>();
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
    auto src = image.get< math::float3>();
    auto dst = result.get< math::float4>();
    for (uint32_t row = 0; row < h; ++row) {
        for (uint32_t col = 0; col < w; ++col, ++src, ++dst) {
            *dst = linearToRGBM(*src);
        }
    }
    return result;
}

template<typename T>
static LinearImage toLinearWithAlpha(size_t w, size_t h, size_t bpr, const uint8_t* src) {
    LinearImage result(w, h, 4);
    math::float4* d = reinterpret_cast<math::float4*>(result.getPixelRef(0, 0));
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 4) {
            math::float4 sRGB(p[0], p[1], p[2], p[3]);
            sRGB /= std::numeric_limits<T>::max();
            *d++ = sRGBToLinear(sRGB);
        }
    }
    return result;
}

template<typename T>
static LinearImage asLinearWithAlpha(size_t w, size_t h, size_t bpr, const uint8_t* src) {
    LinearImage result(w, h, 4);
    math::float4* d = reinterpret_cast<math::float4*>(result.getPixelRef(0, 0));
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 4) {
            math::float4 linear(p[0], p[1], p[2], p[3]);
            linear /= std::numeric_limits<T>::max();
            *d++ = linear;
        }
    }
    return result;
}

template<typename T>
static LinearImage toLinear(size_t w, size_t h, size_t bpr, const uint8_t* src) {
    LinearImage result(w, h, 3);
    math::float3* d = reinterpret_cast<math::float3*>(result.getPixelRef(0, 0));
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 3) {
            math::float3 sRGB(p[0], p[1], p[2]);
            sRGB /= std::numeric_limits<T>::max();
            *d++ = sRGBToLinear(sRGB);
        }
    }
    return result;
}

template<typename T>
static LinearImage asLinear(size_t w, size_t h, size_t bpr, const uint8_t* src) {
    LinearImage result(w, h, 3);
    math::float3* d = reinterpret_cast<math::float3*>(result.getPixelRef(0, 0));
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 3) {
            math::float3 linear(p[0], p[1], p[2]);
            linear /= std::numeric_limits<T>::max();
            *d++ = linear;
        }
    }
    return result;
}

template<typename T>
static LinearImage toLinearWithAlphaNtohs(size_t w, size_t h, size_t bpr, const uint8_t* src) {
    LinearImage result(w, h, 4);
    math::float4* d = reinterpret_cast<math::float4*>(result.getPixelRef(0, 0));
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 4) {
            math::float4 sRGB(ntohs(p[0]), ntohs(p[1]), ntohs(p[2]), ntohs(p[3]));
            sRGB /= std::numeric_limits<T>::max();
            *d++ = sRGBToLinear(sRGB);
        }
    }
    return result;
}

template<typename T>
static LinearImage asLinearWithAlphaNtohs(size_t w, size_t h, size_t bpr, const uint8_t* src) {
    LinearImage result(w, h, 4);
    math::float4* d = reinterpret_cast<math::float4*>(result.getPixelRef(0, 0));
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 4) {
            math::float4 linear(ntohs(p[0]), ntohs(p[1]), ntohs(p[2]), ntohs(p[3]));
            linear /= std::numeric_limits<T>::max();
            *d++ = linear;
        }
    }
    return result;
}

template<typename T>
static LinearImage toLinearNtohs(size_t w, size_t h, size_t bpr, const uint8_t* src) {
    LinearImage result(w, h, 3);
    math::float3* d = reinterpret_cast<math::float3*>(result.getPixelRef(0, 0));
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 3) {
            math::float3 sRGB(ntohs(p[0]), ntohs(p[1]), ntohs(p[2]));
            sRGB /= std::numeric_limits<T>::max();
            *d++ = sRGBToLinear(sRGB);
        }
    }
    return result;
}

template<typename T>
static LinearImage asLinearNtohs(size_t w, size_t h, size_t bpr, const uint8_t* src) {
    LinearImage result(w, h, 3);
    math::float3* d = reinterpret_cast<math::float3*>(result.getPixelRef(0, 0));
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 3) {
            math::float3 linear(ntohs(p[0]), ntohs(p[1]), ntohs(p[2]));
            linear /= std::numeric_limits<T>::max();
            *d++ = linear;
        }
    }
    return result;
}

}  // namespace Image

#endif // IMAGE_COLORTRANSFORM_H_
