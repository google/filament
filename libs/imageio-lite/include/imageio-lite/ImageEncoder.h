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

#ifndef IMAGEIO_LITE_IMAGEENCODER_H_
#define IMAGEIO_LITE_IMAGEENCODER_H_

#include <image/LinearImage.h>

#include <utils/CString.h>
#include <utils/compiler.h>

#include <iosfwd>

namespace imageio_lite {

class UTILS_PUBLIC ImageEncoder {
public:
    enum class Format {
        TIFF, // 8-bit sRGB, 4 channels (RGBA)
    };

    // Consumes linear floating-point data, returns false if unable to encode.
    static bool encode(std::ostream& stream, Format format, image::LinearImage const& image,
            utils::CString const& compression, utils::CString const& destName);

    static Format chooseFormat(utils::CString const& name, bool forceLinear = false);
    static utils::CString chooseExtension(Format format);

    class Encoder {
    public:
        virtual bool encode(const image::LinearImage& image) = 0;
        virtual ~Encoder() = default;
    };
};

} // namespace imageio_lite

#endif /* IMAGEIO_LITE_IMAGEENCODER_H_ */
