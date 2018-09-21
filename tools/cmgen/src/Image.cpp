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

#include <cstring>
#include <utility>

#include "Image.h"

Image::Image() {
}

Image::Image(std::unique_ptr<uint8_t[]> data,
        size_t w, size_t h, size_t bpr, size_t bpp, size_t channels)
    : mOwnedData(std::move(data)),
      mData(mOwnedData.get()),
      mWidth(w),
      mHeight(h),
      mBpr(bpr),
      mBpp(bpp),
      mChannels(channels)
{
}

void Image::reset() {
    mOwnedData.release();
    mWidth = 0;
    mHeight = 0;
    mBpr = 0;
    mBpp = 0;
    mChannels = 0;
    mData = nullptr;
}

void Image::set(Image const& image) {
    mOwnedData.release();
    mWidth = image.mWidth;
    mHeight = image.mHeight;
    mBpr = image.mBpr;
    mBpp = image.mBpp;
    mChannels = image.mChannels;
    mData = image.mData;
}

void Image::subset(Image const& image, size_t x, size_t y, size_t w, size_t h) {
    mOwnedData.release();
    mWidth = w;
    mHeight = h;
    mBpr = image.mBpr;
    mBpp = image.mBpp;
    mChannels = image.mChannels;
    mData = static_cast<uint8_t*>(image.getPixelRef(x, y));
}
