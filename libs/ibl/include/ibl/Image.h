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

#ifndef IBL_IMAGE_H
#define IBL_IMAGE_H

#include <math/scalar.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/compiler.h>

#include <memory>

namespace filament {
namespace ibl {

class UTILS_PUBLIC Image {
public:
    Image();
    Image(size_t w, size_t h, size_t stride = 0);

    void reset();

    void set(Image const& image);

    void subset(Image const& image, size_t x, size_t y, size_t w, size_t h);

    bool isValid() const { return mData != nullptr; }

    size_t getWidth() const { return mWidth; }

    size_t getStride() const { return mBpr / getBytesPerPixel(); }

    size_t getHeight() const { return mHeight; }

    size_t getBytesPerRow() const { return mBpr; }

    size_t getBytesPerPixel() const { return sizeof(math::float3); }

    void* getData() const { return mData; }

    size_t getSize() const { return mBpr * mHeight; }

    void* getPixelRef(size_t x, size_t y) const;

    std::unique_ptr<uint8_t[]> detach() { return std::move(mOwnedData); }

private:
    size_t mBpr = 0;
    size_t mWidth = 0;
    size_t mHeight = 0;
    std::unique_ptr<uint8_t[]> mOwnedData;
    void* mData = nullptr;
};

inline void* Image::getPixelRef(size_t x, size_t y) const {
    return static_cast<uint8_t*>(mData) + y * getBytesPerRow() + x * getBytesPerPixel();
}

} // namespace ibl
} // namespace filament

#endif /* IBL_IMAGE_H */
