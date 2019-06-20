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

//! \file

#ifndef TNT_FILAMENT_DRIVER_PIXEL_BUFFERDESCRIPTOR_H
#define TNT_FILAMENT_DRIVER_PIXEL_BUFFERDESCRIPTOR_H

#include <backend/BufferDescriptor.h>
#include <backend/DriverEnums.h>

#include <utils/compiler.h>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

namespace filament {
namespace backend {

class UTILS_PUBLIC PixelBufferDescriptor : public BufferDescriptor {
public:
    using PixelDataFormat = backend::PixelDataFormat;
    using PixelDataType = backend::PixelDataType;

    PixelBufferDescriptor(void const* buffer, size_t size,
            PixelDataFormat format, PixelDataType type, uint8_t alignment = 1,
            uint32_t left = 0, uint32_t top = 0, uint32_t stride = 0,
            Callback callback = nullptr, void* user = nullptr) noexcept
            : BufferDescriptor(buffer, size, callback, user),
              left(left), top(top), stride(stride),
              format(format), type(type), alignment(alignment) {
    }

    PixelBufferDescriptor(void const* buffer, size_t size,
            PixelDataFormat format, PixelDataType type,
            Callback callback, void* user = nullptr) noexcept
            : BufferDescriptor(buffer, size, callback, user),
              stride(0), format(format), type(type), alignment(1) {
    }

    PixelBufferDescriptor(void const* buffer, size_t size,
            backend::CompressedPixelDataType format, uint32_t imageSize,
            Callback callback, void* user = nullptr) noexcept
            : BufferDescriptor(buffer, size, callback, user),
              imageSize(imageSize), compressedFormat(format), type(PixelDataType::COMPRESSED),
              alignment(1) {
    }

    static constexpr size_t computeDataSize(PixelDataFormat format, PixelDataType type,
            size_t stride, size_t height, size_t alignment) noexcept {
        assert(alignment);

        if (type == PixelDataType::COMPRESSED) {
            return 0;
        }

        size_t n = 0;
        switch (format) {
            case PixelDataFormat::R:
            case PixelDataFormat::R_INTEGER:
            case PixelDataFormat::DEPTH_COMPONENT:
            case PixelDataFormat::ALPHA:
                n = 1;
                break;
            case PixelDataFormat::RG:
            case PixelDataFormat::RG_INTEGER:
            case PixelDataFormat::DEPTH_STENCIL:
                n = 2;
                break;
            case PixelDataFormat::RGB:
            case PixelDataFormat::RGB_INTEGER:
                n = 3;
                break;
            case PixelDataFormat::UNUSED: // shouldn't happen (used to be rgbm)
            case PixelDataFormat::RGBA:
            case PixelDataFormat::RGBA_INTEGER:
                n = 4;
                break;
        }

        size_t bpp = n;
        switch (type) {
            case PixelDataType::COMPRESSED: // Impossible -- to squash the IDE warnings
            case PixelDataType::UBYTE:
            case PixelDataType::BYTE:
                // nothing to do
                break;
            case PixelDataType::USHORT:
            case PixelDataType::SHORT:
            case PixelDataType::HALF:
                bpp *= 2;
                break;
            case PixelDataType::UINT:
            case PixelDataType::INT:
            case PixelDataType::FLOAT:
                bpp *= 4;
                break;
            case PixelDataType::UINT_10F_11F_11F_REV:
                // Special case, format must be RGB and uses 4 bytes
                assert(format == PixelDataFormat::RGB);
                bpp = 4;
                break;
        }

        size_t bpr = bpp * stride;
        size_t bprAligned = (bpr + (alignment - 1)) & -alignment;
        return bprAligned * height;
    }

    uint32_t left   = 0;
    uint32_t top    = 0;
    union {
        struct {
            uint32_t stride;
            PixelDataFormat format;
        };
        struct {
            uint32_t imageSize;
            backend::CompressedPixelDataType compressedFormat;
        };
    };
    PixelDataType type : 4;
    uint8_t alignment  : 4;
};

} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_PIXEL_BUFFERDESCRIPTOR_H
