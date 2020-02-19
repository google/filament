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

#include "TextureReshaper.h"

#include "DataReshaper.h"

#include <utils/Panic.h>

namespace filament {
namespace backend {

static void freeDeleter(void* buffer, size_t, void*) {
    free(buffer);
}

TextureReshaper::TextureReshaper(TextureFormat requestedFormat) noexcept {
    reshapedFormat = requestedFormat;

    if (requestedFormat == TextureFormat::RGB16F) {
        reshapedFormat = TextureFormat::RGBA16F;
        reshapeFunction = [](PixelBufferDescriptor&& p) {
            const size_t reshapedSize = p.size / 6 * 8;     // reshaping from 6 to 8 bytes per pixel
            void* reshapeBuffer = malloc(reshapedSize);
            ASSERT_POSTCONDITION(reshapeBuffer, "Could not allocate memory to reshape pixels.");
            // 0x3c00 is 1.0 in 16 bit floating point.
            DataReshaper::reshape<uint16_t, 3, 4, 0x3c00>(reshapeBuffer, p.buffer, p.size);

            PixelBufferDescriptor reshaped(reshapeBuffer, reshapedSize,
                    PixelBufferDescriptor::PixelDataFormat::RGBA,
                    PixelBufferDescriptor::PixelDataType::HALF, 1, p.left, p.top, p.stride,
                    freeDeleter);
            return reshaped;
        };
    };

    if (requestedFormat == TextureFormat::RGB8) {
        reshapedFormat = TextureFormat::RGBA8;
        reshapeFunction = [](PixelBufferDescriptor&& p) {
            const size_t reshapedSize = p.size / 3 * 4;     // reshaping from 3 to 4 bytes per pixel
            void* reshapeBuffer = malloc(reshapedSize);
            ASSERT_POSTCONDITION(reshapeBuffer, "Could not allocate memory to reshape pixels.");
            DataReshaper::reshape<uint8_t, 3, 4>(reshapeBuffer, p.buffer, p.size);

            PixelBufferDescriptor reshaped(reshapeBuffer, reshapedSize,
                    PixelBufferDescriptor::PixelDataFormat::RGBA,
                    PixelBufferDescriptor::PixelDataType::UBYTE, 1, p.left, p.top, p.stride,
                    freeDeleter);
            return reshaped;
        };
    }
}

TextureFormat TextureReshaper::getReshapedFormat() const noexcept {
    return reshapedFormat;
}

PixelBufferDescriptor TextureReshaper::reshape(PixelBufferDescriptor&& p) const {
    return reshapeFunction(std::move(p));
}

bool TextureReshaper::canReshapeTextureFormat(TextureFormat format) noexcept {
    return format == TextureFormat::RGB16F || format == TextureFormat::RGB8;
}

} // namespace backend
} // namespace filament
