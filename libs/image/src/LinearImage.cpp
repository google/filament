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

#include <image/LinearImage.h>

#include <cstdint>
#include <cstring> // for memset
#include <memory>
#include <stdexcept>

namespace image  {

struct LinearImage::SharedReference {
    SharedReference(uint32_t width, uint32_t height, uint32_t channels) {
        // width * height fits in uint64_t; the subsequent multiplication by
        // channels can still overflow uint64_t, so saturate in two stages.
        const uint64_t wh = uint64_t(width) * height;
        if (channels != 0 && wh > UINT64_MAX / channels) {
            throw std::runtime_error("LinearImage dimensions too large");
        }
        const uint64_t nfloats = wh * channels;
        if (nfloats > SIZE_MAX / sizeof(float)) {
            throw std::runtime_error("LinearImage dimensions too large");
        }
        const size_t n = static_cast<size_t>(nfloats);
        float* floats = new float[n];
        memset(floats, 0, sizeof(float) * n);
        pixels = std::shared_ptr<float>(floats, std::default_delete<float[]>());
    }
    std::shared_ptr<float> pixels;
};

LinearImage::~LinearImage() {
    delete mDataRef;
}

LinearImage::LinearImage(uint32_t width, uint32_t height, uint32_t channels) :
    mDataRef(new SharedReference(width, height, channels)),
    mData(mDataRef->pixels.get()),
    mWidth(width), mHeight(height), mChannels(channels) {}

LinearImage::LinearImage(const LinearImage& that) {
    *this = that;
}

LinearImage& LinearImage::operator=(const LinearImage& that) {
    auto newDataRef = that.mDataRef
        ? new SharedReference(*that.mDataRef)
        : nullptr;
    delete mDataRef;
    mDataRef = newDataRef;
    
    mData = that.mData;
    mWidth = that.mWidth;
    mHeight = that.mHeight;
    mChannels = that.mChannels;
    return *this;
}

}  // namespace image
