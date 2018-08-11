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

namespace image  {

static std::shared_ptr<float> allocSequence(uint32_t width, uint32_t height, uint32_t channels) {
    const uint32_t nfloats = width * height * channels;
    float* floats = new float[nfloats];
    memset(floats, 0, sizeof(float) * nfloats);
    return std::shared_ptr<float>(floats, std::default_delete<float[]>());
}

LinearImage::LinearImage(uint32_t width, uint32_t height, uint32_t channels) :
    mData(allocSequence(width, height, channels)),
    mWidth(width), mHeight(height), mChannels(channels) {}

LinearImage& LinearImage::operator=(const LinearImage& that) {
    mWidth = that.mWidth;
    mHeight = that.mHeight;
    mChannels = that.mChannels;
    mData = that.mData;
    return *this;
}

}  // namespace image
