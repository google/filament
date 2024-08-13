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

#ifndef IMAGE_IMAGEDECODER_H_
#define IMAGE_IMAGEDECODER_H_

#include <iosfwd>
#include <string>

#include <image/LinearImage.h>

#include <utils/compiler.h>

namespace image {

class UTILS_PUBLIC ImageDecoder {
public:
    enum class ColorSpace {
        LINEAR,
        SRGB
    };

    // Returns linear floating-point data, or a non-valid image if an error occured.
    static LinearImage decode(std::istream& stream, const std::string& sourceName,
            ColorSpace sourceSpace = ColorSpace::SRGB);

    class Decoder {
    public:
        virtual LinearImage decode() = 0;
        virtual ~Decoder() = default;

        ColorSpace getColorSpace() const noexcept {
            return mColorSpace;
        }

        void setColorSpace(ColorSpace colorSpace) noexcept {
            mColorSpace = colorSpace;
        }

    private:
        ColorSpace mColorSpace = ColorSpace::SRGB;
    };

private:
    enum class Format {
        NONE,
        PNG,
        HDR,
        PSD,
        EXR,
        JPG
    };
};

} // namespace image

#endif /* IMAGE_IMAGEDECODER_H_ */
