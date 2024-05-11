/*
 * Copyright (C) 2023 The Android Open Source Project
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
 *
 */

#ifndef TNT_BACKENDTESTUTILS_H
#define TNT_BACKENDTESTUTILS_H

#include <cstddef>

#include <backend/PixelBufferDescriptor.h>

using namespace filament;
using namespace filament::backend;

inline void getPixelInfo(PixelDataFormat format, PixelDataType type, size_t& outComponents, int& outBpp) {
    assert_invariant(type != PixelDataType::COMPRESSED);
    switch (format) {
        case PixelDataFormat::UNUSED:
        case PixelDataFormat::R:
        case PixelDataFormat::R_INTEGER:
        case PixelDataFormat::DEPTH_COMPONENT:
        case PixelDataFormat::ALPHA:
            outComponents = 1;
            break;
        case PixelDataFormat::RG:
        case PixelDataFormat::RG_INTEGER:
        case PixelDataFormat::DEPTH_STENCIL:
            outComponents = 2;
            break;
        case PixelDataFormat::RGB:
        case PixelDataFormat::RGB_INTEGER:
            outComponents = 3;
            break;
        case PixelDataFormat::RGBA:
        case PixelDataFormat::RGBA_INTEGER:
            outComponents = 4;
            break;
    }

    outBpp = outComponents;
    switch (type) {
        case PixelDataType::COMPRESSED: // Impossible -- to squash the IDE warnings
        case PixelDataType::UBYTE:
        case PixelDataType::BYTE:
            // nothing to do
            break;
        case PixelDataType::USHORT:
        case PixelDataType::SHORT:
        case PixelDataType::HALF:
            outBpp *= 2;
            break;
        case PixelDataType::UINT:
        case PixelDataType::INT:
        case PixelDataType::FLOAT:
            outBpp *= 4;
            break;
        case PixelDataType::UINT_10F_11F_11F_REV:
            // Special case, format must be RGB and uses 4 bytes
            assert_invariant(format == PixelDataFormat::RGB);
            outBpp = 4;
            break;
        case PixelDataType::UINT_2_10_10_10_REV:
            // Special case, format must be RGBA and uses 4 bytes
            assert_invariant(format == PixelDataFormat::RGBA);
            outBpp = 4;
            break;
        case PixelDataType::USHORT_565:
            // Special case, format must be RGB and uses 2 bytes
            assert_invariant(format == PixelDataFormat::RGB);
            outBpp = 2;
            break;
    }
}

template<typename ComponentType>
static void fillCheckerboard(void* buffer, size_t size, size_t stride, size_t components,
        ComponentType value) {
    ComponentType* row = (ComponentType*)buffer;
    int p = 0;
    for (int r = 0; r < size; r++) {
        ComponentType* pixel = row;
        for (int col = 0; col < size; col++) {
            // Generate a checkerboard pattern.
            if ((p & 0x0010) ^ ((p / size) & 0x0010)) {
                // Turn on the first component (red).
                pixel[0] = value;
            }
            pixel += components;
            p++;
        }
        row += stride * components;
    }
}

static PixelBufferDescriptor checkerboardPixelBuffer(PixelDataFormat format, PixelDataType type,
        size_t size, size_t bufferPadding = 0) {
    size_t components; int bpp;
    getPixelInfo(format, type, components, bpp);

    size_t bufferSize = size + bufferPadding * 2;
    uint8_t* buffer = (uint8_t*) calloc(1, bufferSize * bufferSize * bpp);

    uint8_t* ptr = buffer + (bufferSize * bufferPadding * bpp) + (bufferPadding * bpp);

    switch (type) {
        case PixelDataType::BYTE:
            fillCheckerboard<int8_t>(ptr, size, bufferSize, components, 1);
            break;

        case PixelDataType::UBYTE:
            fillCheckerboard<uint8_t>(ptr, size, bufferSize, components, 0xFF);
            break;

        case PixelDataType::SHORT:
            fillCheckerboard<int16_t>(ptr, size, bufferSize, components, 1);
            break;

        case PixelDataType::USHORT:
            fillCheckerboard<uint16_t>(ptr, size, bufferSize, components, 1u);
            break;

        case PixelDataType::UINT:
            fillCheckerboard<uint32_t>(ptr, size, bufferSize, components, 1u);
            break;

        case PixelDataType::INT:
            fillCheckerboard<int32_t>(ptr, size, bufferSize, components, 1);
            break;

        case PixelDataType::FLOAT:
            fillCheckerboard<float>(ptr, size, bufferSize, components, 1.0f);
            break;

        case PixelDataType::HALF:
            fillCheckerboard<math::half>(ptr, size, bufferSize, components, math::half(1.0f));
            break;

        case PixelDataType::UINT_2_10_10_10_REV:
            fillCheckerboard<uint32_t>(ptr, size, bufferSize, 1, 0xC00003FF /* red */);
            break;

        case PixelDataType::USHORT_565:
            fillCheckerboard<uint16_t>(ptr, size, bufferSize, 1, 0xF800 /* red */);
            break;

        case PixelDataType::UINT_10F_11F_11F_REV:
            fillCheckerboard<uint32_t>(ptr, size, bufferSize, 1, 0x000003C0 /* red */);
            break;

        case PixelDataType::COMPRESSED:
            break;
    }

    PixelBufferDescriptor descriptor(buffer, bufferSize * bufferSize * bpp, format, type,
            1, bufferPadding, bufferPadding, bufferSize, [](void* buffer, size_t size, void* user) {
                free(buffer);
            }, nullptr);
    return descriptor;
}

#endif  // TNT_BACKENDTESTUTILS_H
