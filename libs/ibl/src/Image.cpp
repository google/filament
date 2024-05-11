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

#include <ibl/Image.h>

#include <utility>

namespace filament {
namespace ibl {

Image::Image() = default;

Image::Image(size_t w, size_t h, size_t stride)
        : mBpr((stride ? stride : w) * sizeof(math::float3)),
          mWidth(w),
          mHeight(h),
          mOwnedData(new uint8_t[mBpr * h]),
          mData(mOwnedData.get()) {
}

void Image::reset() {
    mOwnedData.release();
    mWidth = 0;
    mHeight = 0;
    mBpr = 0;
    mData = nullptr;
}

void Image::set(Image const& image) {
    mOwnedData.release();
    mWidth = image.mWidth;
    mHeight = image.mHeight;
    mBpr = image.mBpr;
    mData = image.mData;
}

void Image::subset(Image const& image, size_t x, size_t y, size_t w, size_t h) {
    mOwnedData.release();
    mWidth = w;
    mHeight = h;
    mBpr = image.mBpr;
    mData = static_cast<uint8_t*>(image.getPixelRef(x, y));
}

} // namespace ibl
} // namespace filament

