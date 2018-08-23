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

void Image::subset(Image const& image,
        size_t x, size_t y, size_t w, size_t h, uint32_t flags) {
    mOwnedData.release();
    mWidth = w;
    mHeight = h;
    mBpr = image.mBpr;
    mBpp = image.mBpp;
    mChannels = image.mChannels;
    mData = static_cast<uint8_t*>(image.getPixelRef(x, y));
}

void Image::flip(uint32_t flags) {
    // We verified that these lambdas get inlined by clang in release builds.
    auto getptr = [this](size_t x, size_t y) -> uint8_t* {
        return static_cast<uint8_t*>(mData) + y*mBpr + x*mBpp;
    };
    auto getref = [this, getptr](size_t x, size_t y) -> uint8_t& {
        return *(getptr(x, y));
    };
    if (flags & Image::FLIP_Y) {
        std::unique_ptr<uint8_t[]> tmp(new uint8_t[mBpr]);
        uint8_t* ptmp = tmp.get();
        for (size_t row = 0, nrows = mHeight / 2; row < nrows; ++row) {
            uint8_t* a = getptr(0, row);
            uint8_t* b = getptr(0, mHeight - 1 - row);
            memcpy(ptmp, a, mBpr);
            memcpy(a, b, mBpr);
            memcpy(b, ptmp, mBpr);
        }
    }
    // Our horizontalFlip implementation is inefficient, but it's never invoked in the renderer.
    if (flags & Image::FLIP_X) {
        for (size_t row = 0, nrows = mHeight; row < nrows; ++row) {
            for (size_t src = 0, ncols = mWidth / 2; src < ncols; ++src) {
                size_t dst = mWidth - 1 - src;
                std::swap(getref(src, row), getref(dst, row));
            }
        }
    }
}
