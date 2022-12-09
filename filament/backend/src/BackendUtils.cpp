/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "private/backend/BackendUtils.h"

#include "DataReshaper.h"

#include <utils/CString.h>

#include <string_view>

namespace filament::backend {

bool requestsGoogleLineDirectivesExtension(std::string_view source) noexcept {
    return source.find("GL_GOOGLE_cpp_style_line_directive") != std::string_view::npos;
}

void removeGoogleLineDirectives(char* shader, size_t length) noexcept {
    std::string_view const s{ shader, length };

    size_t pos = 0;
    while (true) {
        pos = s.find("#line", pos);
        if (pos == std::string_view::npos) {
            break;
        }

        pos += 5;

        bool googleStyleDirective = false;
        size_t len = 0;
        size_t start = 0;
        while (pos < length) {
            if (shader[pos] == '"' && !googleStyleDirective) {
                googleStyleDirective = true;
                start = pos;
            }
            if (shader[pos] == '\n') {
                break;
            }
            if (googleStyleDirective) {
                len++;
            }
            pos++;
        }

        // There's no point in trying to splice the shader to remove the quoted filename, just
        // replace the filename with spaces.
        for (size_t i = start; i < start + len; i++) {
            shader[i] = ' ';
        }
    }
}

size_t getFormatSize(TextureFormat format) noexcept {
    switch (format) {
        // 8-bits per element
        case TextureFormat::R8:
        case TextureFormat::R8_SNORM:
        case TextureFormat::R8UI:
        case TextureFormat::R8I:
        case TextureFormat::STENCIL8:
            return 1;

        // 16-bits per element
        case TextureFormat::R16F:
        case TextureFormat::R16UI:
        case TextureFormat::R16I:
        case TextureFormat::RG8:
        case TextureFormat::RG8_SNORM:
        case TextureFormat::RG8UI:
        case TextureFormat::RG8I:
        case TextureFormat::RGB565:
        case TextureFormat::RGB5_A1:
        case TextureFormat::RGBA4:
        case TextureFormat::DEPTH16:
            return 2;

        // 24-bits per element
        case TextureFormat::RGB8:
        case TextureFormat::SRGB8:
        case TextureFormat::RGB8_SNORM:
        case TextureFormat::RGB8UI:
        case TextureFormat::RGB8I:
        case TextureFormat::DEPTH24:
            return 3;

        // 32-bits per element
        case TextureFormat::R32F:
        case TextureFormat::R32UI:
        case TextureFormat::R32I:
        case TextureFormat::RG16F:
        case TextureFormat::RG16UI:
        case TextureFormat::RG16I:
        case TextureFormat::R11F_G11F_B10F:
        case TextureFormat::RGB9_E5:
        case TextureFormat::RGBA8:
        case TextureFormat::SRGB8_A8:
        case TextureFormat::RGBA8_SNORM:
        case TextureFormat::RGB10_A2:
        case TextureFormat::RGBA8UI:
        case TextureFormat::RGBA8I:
        case TextureFormat::DEPTH32F:
        case TextureFormat::DEPTH24_STENCIL8:
        case TextureFormat::DEPTH32F_STENCIL8:
            return 4;

        // 48-bits per element
        case TextureFormat::RGB16F:
        case TextureFormat::RGB16UI:
        case TextureFormat::RGB16I:
            return 6;

        // 64-bits per element
        case TextureFormat::RG32F:
        case TextureFormat::RG32UI:
        case TextureFormat::RG32I:
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA16UI:
        case TextureFormat::RGBA16I:
            return 8;

        // 96-bits per element
        case TextureFormat::RGB32F:
        case TextureFormat::RGB32UI:
        case TextureFormat::RGB32I:
            return 12;

        // 128-bits per element
        case TextureFormat::RGBA32F:
        case TextureFormat::RGBA32UI:
        case TextureFormat::RGBA32I:
            return 16;

        // Compressed formats ---------------------------------------------------------------------

        case TextureFormat::EAC_RG11:
        case TextureFormat::EAC_RG11_SIGNED:
        case TextureFormat::ETC2_EAC_RGBA8:
        case TextureFormat::ETC2_EAC_SRGBA8:
            return 16;

        case TextureFormat::EAC_R11:
        case TextureFormat::EAC_R11_SIGNED:
        case TextureFormat::ETC2_RGB8:
        case TextureFormat::ETC2_SRGB8:
        case TextureFormat::ETC2_RGB8_A1:
        case TextureFormat::ETC2_SRGB8_A1:
            return 8;

        case TextureFormat::DXT1_RGB:
        case TextureFormat::DXT1_RGBA:
        case TextureFormat::DXT1_SRGB:
        case TextureFormat::DXT1_SRGBA:
            return 8;

        case TextureFormat::DXT3_RGBA:
        case TextureFormat::DXT3_SRGBA:
        case TextureFormat::DXT5_RGBA:
        case TextureFormat::DXT5_SRGBA:
            return 16;

        case TextureFormat::RED_RGTC1:
        case TextureFormat::SIGNED_RED_RGTC1:
            return 8;

        case TextureFormat::RED_GREEN_RGTC2:
        case TextureFormat::SIGNED_RED_GREEN_RGTC2:
            return 16;

        case TextureFormat::RGB_BPTC_SIGNED_FLOAT:
        case TextureFormat::RGB_BPTC_UNSIGNED_FLOAT:
        case TextureFormat::RGBA_BPTC_UNORM:
        case TextureFormat::SRGB_ALPHA_BPTC_UNORM:
            return 16;

        // The block size for ASTC compression is always 16 bytes.
        case TextureFormat::RGBA_ASTC_4x4:
        case TextureFormat::RGBA_ASTC_5x4:
        case TextureFormat::RGBA_ASTC_5x5:
        case TextureFormat::RGBA_ASTC_6x5:
        case TextureFormat::RGBA_ASTC_6x6:
        case TextureFormat::RGBA_ASTC_8x5:
        case TextureFormat::RGBA_ASTC_8x6:
        case TextureFormat::RGBA_ASTC_8x8:
        case TextureFormat::RGBA_ASTC_10x5:
        case TextureFormat::RGBA_ASTC_10x6:
        case TextureFormat::RGBA_ASTC_10x8:
        case TextureFormat::RGBA_ASTC_10x10:
        case TextureFormat::RGBA_ASTC_12x10:
        case TextureFormat::RGBA_ASTC_12x12:
        case TextureFormat::SRGB8_ALPHA8_ASTC_4x4:
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x4:
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x8:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x8:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x10:
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x10:
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x12:
            return 16;

        default:
            return 0;
    }
}

size_t getFormatComponentCount(TextureFormat format) noexcept {
    switch (format) {
        case TextureFormat::R8:
        case TextureFormat::R8_SNORM:
        case TextureFormat::R8UI:
        case TextureFormat::R8I:
        case TextureFormat::R16F:
        case TextureFormat::R16UI:
        case TextureFormat::R16I:
        case TextureFormat::R32F:
        case TextureFormat::R32I:
        case TextureFormat::R32UI:
        case TextureFormat::STENCIL8:
        case TextureFormat::DEPTH16:
        case TextureFormat::DEPTH24:
        case TextureFormat::DEPTH32F:
            return 1;

        case TextureFormat::RG8:
        case TextureFormat::RG8_SNORM:
        case TextureFormat::RG8UI:
        case TextureFormat::RG8I:
        case TextureFormat::RG16F:
        case TextureFormat::RG16UI:
        case TextureFormat::RG16I:
        case TextureFormat::RG32F:
        case TextureFormat::RG32UI:
        case TextureFormat::RG32I:
        case TextureFormat::DEPTH24_STENCIL8:
        case TextureFormat::DEPTH32F_STENCIL8:
            return 2;

        case TextureFormat::RGB565:
        case TextureFormat::RGB8:
        case TextureFormat::SRGB8:
        case TextureFormat::RGB8_SNORM:
        case TextureFormat::RGB8UI:
        case TextureFormat::RGB8I:
        case TextureFormat::R11F_G11F_B10F:
        case TextureFormat::RGB16F:
        case TextureFormat::RGB16UI:
        case TextureFormat::RGB16I:
        case TextureFormat::RGB32F:
        case TextureFormat::RGB32UI:
        case TextureFormat::RGB32I:
            return 3;

        case TextureFormat::RGB5_A1:
        case TextureFormat::RGBA4:
        case TextureFormat::RGB9_E5:
        case TextureFormat::RGBA8:
        case TextureFormat::SRGB8_A8:
        case TextureFormat::RGBA8_SNORM:
        case TextureFormat::RGB10_A2:
        case TextureFormat::RGBA8UI:
        case TextureFormat::RGBA8I:
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA16UI:
        case TextureFormat::RGBA16I:
        case TextureFormat::RGBA32F:
        case TextureFormat::RGBA32UI:
        case TextureFormat::RGBA32I:
            return 4;

        // Compressed formats ---------------------------------------------------------------------
        case TextureFormat::EAC_R11:
        case TextureFormat::EAC_R11_SIGNED:
        case TextureFormat::RED_RGTC1:
        case TextureFormat::SIGNED_RED_RGTC1:
            return 1;

        case TextureFormat::EAC_RG11:
        case TextureFormat::EAC_RG11_SIGNED:
        case TextureFormat::RED_GREEN_RGTC2:
        case TextureFormat::SIGNED_RED_GREEN_RGTC2:
            return 2;

        case TextureFormat::ETC2_RGB8:
        case TextureFormat::ETC2_SRGB8:
        case TextureFormat::DXT1_RGB:
        case TextureFormat::DXT1_SRGB:
        case TextureFormat::RGB_BPTC_SIGNED_FLOAT:
        case TextureFormat::RGB_BPTC_UNSIGNED_FLOAT:
            return 3;

        case TextureFormat::ETC2_EAC_RGBA8:
        case TextureFormat::ETC2_EAC_SRGBA8:
        case TextureFormat::ETC2_RGB8_A1:
        case TextureFormat::ETC2_SRGB8_A1:
        case TextureFormat::DXT1_RGBA:
        case TextureFormat::DXT1_SRGBA:
        case TextureFormat::DXT3_RGBA:
        case TextureFormat::DXT3_SRGBA:
        case TextureFormat::DXT5_RGBA:
        case TextureFormat::DXT5_SRGBA:
        case TextureFormat::RGBA_BPTC_UNORM:
        case TextureFormat::SRGB_ALPHA_BPTC_UNORM:
        case TextureFormat::RGBA_ASTC_4x4:
        case TextureFormat::RGBA_ASTC_5x4:
        case TextureFormat::RGBA_ASTC_5x5:
        case TextureFormat::RGBA_ASTC_6x5:
        case TextureFormat::RGBA_ASTC_6x6:
        case TextureFormat::RGBA_ASTC_8x5:
        case TextureFormat::RGBA_ASTC_8x6:
        case TextureFormat::RGBA_ASTC_8x8:
        case TextureFormat::RGBA_ASTC_10x5:
        case TextureFormat::RGBA_ASTC_10x6:
        case TextureFormat::RGBA_ASTC_10x8:
        case TextureFormat::RGBA_ASTC_10x10:
        case TextureFormat::RGBA_ASTC_12x10:
        case TextureFormat::RGBA_ASTC_12x12:
        case TextureFormat::SRGB8_ALPHA8_ASTC_4x4:
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x4:
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x8:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x8:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x10:
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x10:
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x12:
            return 4;
        case TextureFormat::UNUSED:
            return 0;
    }
}

size_t getBlockWidth(TextureFormat format) noexcept {
    switch (format) {
        case TextureFormat::EAC_RG11:
        case TextureFormat::EAC_RG11_SIGNED:
        case TextureFormat::ETC2_EAC_RGBA8:
        case TextureFormat::ETC2_EAC_SRGBA8:
        case TextureFormat::EAC_R11:
        case TextureFormat::EAC_R11_SIGNED:
        case TextureFormat::ETC2_RGB8:
        case TextureFormat::ETC2_SRGB8:
        case TextureFormat::ETC2_RGB8_A1:
        case TextureFormat::ETC2_SRGB8_A1:
            return 4;

        case TextureFormat::DXT1_RGB:
        case TextureFormat::DXT1_RGBA:
        case TextureFormat::DXT1_SRGB:
        case TextureFormat::DXT1_SRGBA:
        case TextureFormat::DXT3_RGBA:
        case TextureFormat::DXT3_SRGBA:
        case TextureFormat::DXT5_RGBA:
        case TextureFormat::DXT5_SRGBA:
            return 4;

        case TextureFormat::RED_RGTC1:
        case TextureFormat::SIGNED_RED_RGTC1:
        case TextureFormat::RED_GREEN_RGTC2:
        case TextureFormat::SIGNED_RED_GREEN_RGTC2:
            return 4;

        case TextureFormat::RGB_BPTC_SIGNED_FLOAT:
        case TextureFormat::RGB_BPTC_UNSIGNED_FLOAT:
        case TextureFormat::RGBA_BPTC_UNORM:
        case TextureFormat::SRGB_ALPHA_BPTC_UNORM:
            return 4;

        case TextureFormat::RGBA_ASTC_4x4:
        case TextureFormat::SRGB8_ALPHA8_ASTC_4x4:
            return 4;

        case TextureFormat::RGBA_ASTC_5x4:
        case TextureFormat::RGBA_ASTC_5x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x4:
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x5:
            return 5;

        case TextureFormat::RGBA_ASTC_6x5:
        case TextureFormat::RGBA_ASTC_6x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x6:
            return 6;

        case TextureFormat::RGBA_ASTC_8x5:
        case TextureFormat::RGBA_ASTC_8x6:
        case TextureFormat::RGBA_ASTC_8x8:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x8:
            return 8;

        case TextureFormat::RGBA_ASTC_10x5:
        case TextureFormat::RGBA_ASTC_10x6:
        case TextureFormat::RGBA_ASTC_10x8:
        case TextureFormat::RGBA_ASTC_10x10:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x8:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x10:
            return 10;

        case TextureFormat::RGBA_ASTC_12x10:
        case TextureFormat::RGBA_ASTC_12x12:
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x10:
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x12:
            return 12;

        default:
            return 0;
    }
}

size_t getBlockHeight(TextureFormat format) noexcept {
    switch (format) {
        case TextureFormat::RGBA_ASTC_4x4:
        case TextureFormat::SRGB8_ALPHA8_ASTC_4x4:
        case TextureFormat::RGBA_ASTC_5x4:
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x4:
            return 4;

        case TextureFormat::RGBA_ASTC_5x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x5:
        case TextureFormat::RGBA_ASTC_6x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x5:
        case TextureFormat::RGBA_ASTC_8x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x5:
        case TextureFormat::RGBA_ASTC_10x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x5:
            return 5;

        case TextureFormat::RGBA_ASTC_6x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x6:
        case TextureFormat::RGBA_ASTC_8x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x6:
        case TextureFormat::RGBA_ASTC_10x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x6:
            return 6;

        case TextureFormat::RGBA_ASTC_8x8:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x8:
        case TextureFormat::RGBA_ASTC_10x8:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x8:
            return 8;

        case TextureFormat::RGBA_ASTC_10x10:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x10:
        case TextureFormat::RGBA_ASTC_12x10:
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x10:
            return 10;

        case TextureFormat::RGBA_ASTC_12x12:
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x12:
            return 12;

        default:
            // Most compressed formats use square blocks, only ASTC is special.
            return getBlockWidth(format);
    }
}

bool reshape(const PixelBufferDescriptor& data, PixelBufferDescriptor& reshaped) {
    // We only reshape 3 component pixel buffers: either RGB or RGB_INTEGER.
    if (!(data.format == PixelDataFormat::RGB || data.format == PixelDataFormat::RGB_INTEGER)) {
        return false;
    }

    const auto freeFunc = [](void* buffer,
            UTILS_UNUSED size_t size, UTILS_UNUSED void* user) { free(buffer); };
    const size_t reshapedSize = 4 * data.size / 3;
    const PixelDataFormat reshapedFormat =
        data.format == PixelDataFormat::RGB ? PixelDataFormat::RGBA : PixelDataFormat::RGBA_INTEGER;
    switch (data.type) {
        case PixelDataType::BYTE:
        case PixelDataType::UBYTE: {
            uint8_t* bytes = (uint8_t*) malloc(reshapedSize);
            DataReshaper::reshape<uint8_t, 3, 4>(bytes, data.buffer, data.size);
            PixelBufferDescriptor pbd(bytes, reshapedSize, reshapedFormat, data.type,
                    data.alignment, data.left, data.top, data.stride, freeFunc);
            reshaped = std::move(pbd);
            return true;
        }
        case PixelDataType::SHORT:
        case PixelDataType::USHORT:
        case PixelDataType::HALF: {
            uint8_t* bytes = (uint8_t*) malloc(reshapedSize);
            DataReshaper::reshape<uint16_t, 3, 4>(bytes, data.buffer, data.size);
            PixelBufferDescriptor pbd(bytes, reshapedSize, reshapedFormat, data.type,
                    data.alignment, data.left, data.top, data.stride, freeFunc);
            reshaped = std::move(pbd);
            return true;
        }
        case PixelDataType::INT:
        case PixelDataType::UINT: {
            uint8_t* bytes = (uint8_t*) malloc(reshapedSize);
            DataReshaper::reshape<uint32_t, 3, 4>(bytes, data.buffer, data.size);
            PixelBufferDescriptor pbd(bytes, reshapedSize, reshapedFormat, data.type,
                    data.alignment, data.left, data.top, data.stride, freeFunc);
            reshaped = std::move(pbd);
            return true;
        }
        case PixelDataType::FLOAT: {
            uint8_t* bytes = (uint8_t*) malloc(reshapedSize);
            DataReshaper::reshape<float, 3, 4>(bytes, data.buffer, data.size);
            PixelBufferDescriptor pbd(bytes, reshapedSize, reshapedFormat, data.type,
                    data.alignment, data.left, data.top, data.stride, freeFunc);
            reshaped = std::move(pbd);
            return true;
        }
        default:
           return false;
    }
}

} // namespace backend::filament


namespace utils {

template<>
CString to_string<filament::backend::TextureUsage>(filament::backend::TextureUsage value) noexcept {
    using namespace filament::backend;
    char string[7] = {'-', '-', '-', '-', '-', '-', 0};
    if (any(value & TextureUsage::UPLOADABLE)) {
        string[0]='U';
    }
    if (any(value & TextureUsage::SAMPLEABLE)) {
        string[1]='S';
    }
    if (any(value & TextureUsage::COLOR_ATTACHMENT)) {
        string[2]='c';
    }
    if (any(value & TextureUsage::DEPTH_ATTACHMENT)) {
        string[3]='d';
    }
    if (any(value & TextureUsage::STENCIL_ATTACHMENT)) {
        string[4] = 's';
    }
    if (any(value & TextureUsage::SUBPASS_INPUT)) {
        string[5]='f';
    }
    return { string, 6 };
}

template<>
CString to_string<filament::backend::TargetBufferFlags>(filament::backend::TargetBufferFlags value) noexcept {
    using namespace filament::backend;
    char string[7] = {'-', '-', '-', '-', '-', '-', 0};
    if (any(value & TargetBufferFlags::COLOR0)) {
        string[0]='0';
    }
    if (any(value & TargetBufferFlags::COLOR1)) {
        string[1]='1';
    }
    if (any(value & TargetBufferFlags::COLOR2)) {
        string[2]='2';
    }
    if (any(value & TargetBufferFlags::COLOR3)) {
        string[3]='3';
    }
    if (any(value & TargetBufferFlags::DEPTH)) {
        string[4]='D';
    }
    if (any(value & TargetBufferFlags::STENCIL)) {
        string[5]='S';
    }
    return { string, 6 };
}

} // namespace utils
