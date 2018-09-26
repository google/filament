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

//! \file Functions and types related to block-compressed texture formats.

#ifndef IMAGEIO_BLOCKCOMPRESSION_H_
#define IMAGEIO_BLOCKCOMPRESSION_H_

#include <image/LinearImage.h>

#include <memory>
#include <string>

#include <stdint.h>

namespace image {

// Controls how fast compression occurs at the cost of quality in the resulting image.
enum class AstcPreset {
    VERYFAST,
    FAST,
    MEDIUM,
    THOROUGH,
    EXHAUSTIVE,
};

// Informs the encoder what texels represent; this is especially crucial for normal maps.
enum class AstcSemantic {
    COLORS_LDR,
    COLORS_HDR,
    NORMALS,
};

// Gets passed to the encoder function to control the quality and speed of compression.
// The specified bitrate can be anywhere from 0.8 to 8.0.
struct AstcConfig {
    AstcPreset quality;
    AstcSemantic semantic;
    float bitrate;
};

// Represents the result of compression, including which of the 28 internal formats that the
// encoder finally settled on, based on the hints supplied in AstcConfig.
struct AstcTexture {
    const uint32_t gl_internal_format;
    const uint32_t size;
    std::unique_ptr<uint8_t[]> data;
};

// Uses the CPU to compress a linear image (1 to 4 channels) into an ASTC texture. The 16-byte
// header block that ARM uses in their file format is not included.
AstcTexture astcCompress(const LinearImage& source, AstcConfig config);

// Parses a simple underscore-delimited string to produce an ASTC compression configuration. This
// makes it easy to incorporate the compression API into command-line tools. If the string is
// malformed, this returns a config with a 0 bitrate. Example strings: fast_ldr_4.2,
// thorough_normals_8.0, veryfast_hdr_1.0
AstcConfig astcParseOptionString(const std::string& options);

} // namespace image

#endif /* IMAGEIO_BLOCKCOMPRESSION_H_ */
