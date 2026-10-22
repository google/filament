/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef IMAGEIO_LITE_IMAGEDECODER_H_
#define IMAGEIO_LITE_IMAGEDECODER_H_

#include <image/LinearImage.h>

#include <utils/CString.h>
#include <utils/compiler.h>

#include <iosfwd>

namespace imageio_lite {

class UTILS_PUBLIC ImageDecoder {
public:
    enum class ColorSpace { LINEAR, SRGB };

    // Returns linear floating-point data, or a non-valid image if an error occurred.
    static image::LinearImage decode(std::istream& stream, utils::CString const& sourceName,
            ColorSpace sourceSpace = ColorSpace::SRGB);

    class Decoder {
    public:
        virtual image::LinearImage decode() = 0;
        virtual ~Decoder() = default;
        ColorSpace getColorSpace() const noexcept { return mColorSpace; }
        void setColorSpace(ColorSpace colorSpace) noexcept { mColorSpace = colorSpace; }

    private:
        ColorSpace mColorSpace = ColorSpace::SRGB;
    };

private:
    enum class Format { NONE, TIFF };
};

} // namespace imageio_lite

#endif /* IMAGEIO_LITE_IMAGEDECODER_H_ */
