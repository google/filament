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

#include <utils/Logger.h>

#include <cstdint>
#include <cstring> // for memset
#include <memory>
#include <new>
#include <limits>

namespace image  {

struct LinearImage::SharedReference {
    SharedReference(uint32_t const width, uint32_t const height, uint32_t const channels) {
        if (channels == 0) {
            return;
        }
        // Calculate width * height in 64-bit to prevent intermediate overflow
        const uint64_t pixelsCount = uint64_t(width) * height;
        if (width > 0 && pixelsCount / width != height) {
            return;
        }

        // Calculate floats count with channels overflow validation
        if (pixelsCount > std::numeric_limits<uint64_t>::max() / channels) {
            return;
        }
        const uint64_t nfloats = pixelsCount * channels;

        // Validate against size_t max / sizeof(float) before allocating
        if (nfloats > std::numeric_limits<size_t>::max() / sizeof(float)) {
            return;
        }

        float* floats = new(std::nothrow) float[nfloats];
        if (!floats) {
            return;
        }
        memset(floats, 0, sizeof(float) * nfloats);
        pixels = std::shared_ptr<float>(floats, std::default_delete<float[]>());
    }
    std::shared_ptr<float> pixels;
};

LinearImage::~LinearImage() {
    delete mDataRef;
}

LinearImage::LinearImage(uint32_t const width, uint32_t const height, uint32_t const channels) :
    mDataRef(new(std::nothrow) SharedReference(width, height, channels)),
    mData(mDataRef && mDataRef->pixels ? mDataRef->pixels.get() : nullptr),
    mWidth(mData && mDataRef ? width : 0),
    mHeight(mData && mDataRef ? height : 0),
    mChannels(mData && mDataRef ? channels : 0) {
    if (!mData) {
        LOG(ERROR) << "LinearImage allocation failed (overflow or out of memory).";
        delete mDataRef;
        mDataRef = nullptr;
    }
}

LinearImage::LinearImage(const LinearImage& that) :
    mDataRef(that.mDataRef ? new(std::nothrow) SharedReference(*that.mDataRef) : nullptr),
    mData(mDataRef && mDataRef->pixels ? mDataRef->pixels.get() : nullptr),
    mWidth(mData ? that.mWidth : 0),
    mHeight(mData ? that.mHeight : 0),
    mChannels(mData ? that.mChannels : 0) {}

LinearImage& LinearImage::operator=(const LinearImage& that) {
    if (this != &that) {
        SharedReference* const newDataRef = that.mDataRef
            ? new(std::nothrow) SharedReference(*that.mDataRef)
            : nullptr;

        delete mDataRef;
        mDataRef = newDataRef;

        mData = mDataRef && mDataRef->pixels ? mDataRef->pixels.get() : nullptr;
        mWidth = mData ? that.mWidth : 0;
        mHeight = mData ? that.mHeight : 0;
        mChannels = mData ? that.mChannels : 0;
    }
    return *this;
}

LinearImage::LinearImage(LinearImage&& that) noexcept :
    mDataRef(that.mDataRef),
    mData(that.mData),
    mWidth(that.mWidth),
    mHeight(that.mHeight),
    mChannels(that.mChannels) {
    that.mDataRef = nullptr;
    that.mData = nullptr;
    that.mWidth = 0;
    that.mHeight = 0;
    that.mChannels = 0;
}

LinearImage& LinearImage::operator=(LinearImage&& that) noexcept {
    if (this != &that) {
        delete mDataRef;
        mDataRef = that.mDataRef;
        mData = that.mData;
        mWidth = that.mWidth;
        mHeight = that.mHeight;
        mChannels = that.mChannels;

        that.mDataRef = nullptr;
        that.mData = nullptr;
        that.mWidth = 0;
        that.mHeight = 0;
        that.mChannels = 0;
    }
    return *this;
}

}  // namespace image
