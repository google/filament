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

#include <math/vec2.h>

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

// The encoder configuration controls the quality and speed of compression, as well as the resulting
// format. The specified block size must be one of the 14 block sizes that can be consumed by ES 3.2
// as per https://www.khronos.org/registry/OpenGL-Refpages/es3/html/glCompressedTexImage2D.xhtml
struct AstcConfig {
    AstcPreset quality;
    AstcSemantic semantic;
    math::ushort2 blocksize;
    bool srgb;
};

enum class AstcFormat {
    RGBA_ASTC_4x4 = 0x93B0,
    RGBA_ASTC_5x4 = 0x93B1,
    RGBA_ASTC_5x5 = 0x93B2,
    RGBA_ASTC_6x5 = 0x93B3,
    RGBA_ASTC_6x6 = 0x93B4,
    RGBA_ASTC_8x5 = 0x93B5,
    RGBA_ASTC_8x6 = 0x93B6,
    RGBA_ASTC_8x8 = 0x93B7,
    RGBA_ASTC_10x5 = 0x93B8,
    RGBA_ASTC_10x6 = 0x93B9,
    RGBA_ASTC_10x8 = 0x93BA,
    RGBA_ASTC_10x10 = 0x93BB,
    RGBA_ASTC_12x10 = 0x93BC,
    RGBA_ASTC_12x12 = 0x93BD,
    SRGB8_ALPHA8_ASTC_4x4 = 0x93D0,
    SRGB8_ALPHA8_ASTC_5x4 = 0x93D1,
    SRGB8_ALPHA8_ASTC_5x5 = 0x93D2,
    SRGB8_ALPHA8_ASTC_6x5 = 0x93D3,
    SRGB8_ALPHA8_ASTC_6x6 = 0x93D4,
    SRGB8_ALPHA8_ASTC_8x5 = 0x93D5,
    SRGB8_ALPHA8_ASTC_8x6 = 0x93D6,
    SRGB8_ALPHA8_ASTC_8x8 = 0x93D7,
    SRGB8_ALPHA8_ASTC_10x5 = 0x93D8,
    SRGB8_ALPHA8_ASTC_10x6 = 0x93D9,
    SRGB8_ALPHA8_ASTC_10x8 = 0x93DA,
    SRGB8_ALPHA8_ASTC_10x10 = 0x93DB,
    SRGB8_ALPHA8_ASTC_12x10 = 0x93DC,
    SRGB8_ALPHA8_ASTC_12x12 = 0x93DD,
};

// Represents the result of compression, including which of the 28 internal formats that the
// encoder finally settled on, based on the hints supplied in AstcConfig.
struct AstcTexture {
    const AstcFormat format;
    const uint32_t size;
    std::unique_ptr<uint8_t[]> data;
};

// Uses the CPU to compress a linear image (1 to 4 channels) into an ASTC texture. The 16-byte
// header block that ARM uses in their file format is not included.
AstcTexture astcCompress(const LinearImage& source, AstcConfig config);

// Parses a simple underscore-delimited string to produce an ASTC compression configuration. This
// makes it easy to incorporate the compression API into command-line tools. If the string is
// malformed, this returns a config with a 0x0 blocksize. Example strings: fast_ldr_4x4,
// thorough_normals_6x6, veryfast_hdr_12x10
AstcConfig astcParseOptionString(const std::string& options);

} // namespace image

#endif /* IMAGEIO_BLOCKCOMPRESSION_H_ */
