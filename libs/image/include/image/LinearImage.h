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

#ifndef IMAGE_LINEARIMAGE_H
#define IMAGE_LINEARIMAGE_H

#include <memory>

/**
 * Types and free functions for the Filament core imaging library, primarily used for offline tools,
 * but with minimal dependencies to support potential use by the renderer.
 */
namespace image {

/**
 * LinearImage is a handle to packed floating point data arranged into a row-major grid.
 *
 * We use this object as input/output for core algorithms that wish to be agnostic of source and
 * destination formats. The underlying pixel data has shared ownership semantics to allow clients to
 * easily pass around the image object without incurring a deep copy. The number of channels is
 * arbitrary (1 or more) but we often use 3-channel images to represent color data.
 *
 * By convention, we do not use channel major order (i.e. planar). However we provide a free
 * function in ImageOps to combine planar data. Pixels are stored such that the row stride is simply
 * width * channels * sizeof(float).
 */
class LinearImage {
public:

    /**
     * Allocates a zeroed-out image.
     */
    LinearImage(uint32_t width, uint32_t height, uint32_t channels);

    /**
     * Makes a shallow copy with shared pixel data.
     */
    LinearImage(const LinearImage& that) = default;
    LinearImage& operator=(const LinearImage& that);

    /**
     * Creates an empty (invalid) image.
     */
    LinearImage() : mWidth(0), mHeight(0), mChannels(0) {}

    /**
     * Gets a pointer to the underlying pixel data.
     */
    float* getPixelRef() { return mData.get(); }

    /**
     * Gets a pointer to immutable pixel data.
     */
    float const* getPixelRef() const { return mData.get(); }

    /**
     * Gets a pointer to the pixel data at the given column and row. (not bounds checked)
     */
    float* getPixelRef(uint32_t column, uint32_t row) {
        return mData.get() + (column + row * mWidth) * mChannels;
    }

    /**
     * Gets a pointer to the immutable pixel data at the given column and row. (not bounds checked)
     */
    float const* getPixelRef(uint32_t column, uint32_t row) const {
        return mData.get() + (column + row * mWidth) * mChannels;
    }

    uint32_t getWidth() const { return mWidth; }
    uint32_t getHeight() const { return mHeight; }
    uint32_t getChannels() const { return mChannels; }
    void reset() { *this = LinearImage(); }

private:

    // Pixel data has shared ownership to facilitate shallow copies and a convenient Python-like API
    // that allows clients to easily pass around images.
    std::shared_ptr<float> mData;

    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mChannels;
};

} // namespace image

#endif /* IMAGE_LINEARIMAGE_H */
