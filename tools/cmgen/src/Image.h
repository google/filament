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

#ifndef CMGEN_IMAGE_H_
#define CMGEN_IMAGE_H_

#include <image/ColorTransform.h>

#include <stdio.h>
#include <memory>

#include <math/scalar.h>
#include <math/vec3.h>
#include <math/vec4.h>

/**
 * \deprecated
 * We are phasing out this class in favor of LinearImage. The latter has a stable and well-defined
 * pixel format, making it easier to implement image-based algorithms.
 */
class Image {
public:
    Image();
    Image(std::unique_ptr<uint8_t[]> data, size_t w, size_t h,
          size_t bpr, size_t bpp, size_t channels = 3);

    void reset();

    void set(Image const& image);

    void subset(Image const& image, size_t x, size_t y, size_t w, size_t h);

    bool isValid() const { return mData != nullptr; }
    size_t getWidth() const { return mWidth; }
    size_t getHeight() const { return mHeight; }
    size_t getBytesPerRow() const { return mBpr; }
    size_t getBytesPerPixel() const { return mBpp; }
    size_t getChannelsCount() const { return mChannels; }
    void* getData() const { return mData; }

    void* getPixelRef(size_t x, size_t y) const;

private:
    std::unique_ptr<uint8_t[]> mOwnedData;
    void* mData = nullptr;
    size_t mWidth = 0;
    size_t mHeight = 0;
    size_t mBpr = 0;
    size_t mBpp = 0;
    size_t mChannels = 0;
};

inline void* Image::getPixelRef(size_t x, size_t y) const {
    return static_cast<uint8_t*>(mData) + y*mBpr + x*mBpp;
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
            float3 l(image::linearTosRGB(saturate(*p)) * std::numeric_limits<T>::max());
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
            float4 l(image::linearToRGBM(*p) * std::numeric_limits<T>::max());
            for (size_t i = 0; i < 4; i++) {
                d[i] = T(l[i]);
            }
        }
    }
    return dst;
}

#endif /* CMGEN_IMAGE_H_ */
