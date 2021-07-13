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

#ifndef IMAGE_IMAGEENCODER_H_
#define IMAGE_IMAGEENCODER_H_

#include <iosfwd>
#include <string>

#include <image/LinearImage.h>

#include <utils/compiler.h>

namespace image {

class UTILS_PUBLIC ImageEncoder {
public:
    enum class Format {
        PNG,        // 8-bit sRGB, 1 or 3 channels
        PNG_LINEAR, // 8-bit linear RGB, 1 or 3 channels
        HDR,        // 8-bit linear RGBE, 3 channels only
        RGBM,       // 8-bit RGBM, as PNG, 3 channels only
        PSD,        // 16-bit sRGB or 32-bit linear RGB, 3 channels only
                    // Default: 16 bit
        EXR,        // 16-bit linear RGB (half-float), 3 channels only
                    // Default: PIZ compression
        DDS,        // 8-bit sRGB, 1, 2 or 3 channels;
                    // 16-bit or 32-bit linear RGB, 1, 2 or 3 channels
                    // Default: 16 bit
        DDS_LINEAR, // 8-bit, 16-bit or 32-bit linear RGB, 1, 2 or 3 channels
                    // Default: 16 bit
        RGB_10_11_11_REV,   // RGBA PNG file, but containing 11_11_10 data
    };

    // Consumes linear floating-point data, returns false if unable to encode.
    static bool encode(std::ostream& stream, Format format, const LinearImage& image,
            const std::string& compression, const std::string& destName);

    static Format chooseFormat(const std::string& name, bool forceLinear = false);
    static std::string chooseExtension(Format format);

    class Encoder {
    public:
        virtual bool encode(const LinearImage& image) = 0;
        virtual ~Encoder() = default;
    };
};

} // namespace image

#endif /* IMAGE_IMAGEENCODER_H_ */
