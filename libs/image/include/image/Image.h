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

#ifndef IMAGE_IMAGE_H_
#define IMAGE_IMAGE_H_

#include <stdio.h>
#include <memory>

/**
 * This namespace defines types and free functions for both the core "image" library (minimal
 * dependencies) as well as the "imageio" library (for loading and saving files).
 */
namespace image {

class Image {
public:
    Image();
    Image(std::unique_ptr<uint8_t[]> data, size_t w, size_t h,
          size_t bpr, size_t bpp, size_t channels = 3);

    enum {
        FLIP_X = 0x1,
        FLIP_Y = 0x2,
        FLIP_XY = FLIP_X | FLIP_Y
    };

    void reset();

    void set(Image const& image);

    void subset(Image const& image, size_t x, size_t y, size_t w, size_t h, uint32_t flags = 0);

    void setFlags(uint32_t flags);

    bool isValid() const { return mData != nullptr; };
    size_t getWidth() const { return mWidth; };
    size_t getHeight() const { return mHeight; };
    size_t getBytesPerRow() const { return mBpr; };
    size_t getBytesPerPixel() const { return mBpp; };
    size_t getChannelsCount() const { return mChannels; }
    uint32_t getFlags() const { return mFlags; };
    void* getData() const { return mData; };

    void* getPixelRef(size_t x, size_t y) const;

    void* getSampleRef(size_t x, size_t y) const;

private:
    std::unique_ptr<uint8_t[]> mOwnedData;
    void* mData = nullptr;
    size_t mWidth = 0;
    size_t mHeight = 0;
    size_t mBpr = 0;
    size_t mBpp = 0;
    size_t mChannels = 0;
    uint32_t mFlags = 0;
};

inline void* Image::getPixelRef(size_t x, size_t y) const {
    return static_cast<uint8_t*>(mData) + y*mBpr + x*mBpp;
}

inline void* Image::getSampleRef(size_t x, size_t y) const {
    if (mFlags & FLIP_X) {
        x = (mWidth-1)-x;
    }
    if (mFlags & FLIP_Y) {
        y = (mHeight-1)-y;
    }
    return getPixelRef(x, y);
}

} // namespace image

#endif /* IMAGE_IMAGE_H_ */
