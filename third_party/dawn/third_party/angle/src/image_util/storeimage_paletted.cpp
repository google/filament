//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// storeimage_paletted.cpp: Encodes GL_PALETTE_* textures.

#include <unordered_map>

#include "image_util/storeimage.h"

#include <type_traits>
#include "common/mathutil.h"

#include "image_util/imageformats.h"

namespace angle
{

namespace
{

template <typename T>
inline T *OffsetDataPointer(uint8_t *data, size_t y, size_t z, size_t rowPitch, size_t depthPitch)
{
    return reinterpret_cast<T *>(data + (y * rowPitch) + (z * depthPitch));
}

template <typename T>
inline const T *OffsetDataPointer(const uint8_t *data,
                                  size_t y,
                                  size_t z,
                                  size_t rowPitch,
                                  size_t depthPitch)
{
    return reinterpret_cast<const T *>(data + (y * rowPitch) + (z * depthPitch));
}

void EncodeColor(R8G8B8A8 rgba,
                 uint32_t redBlueBits,
                 uint32_t greenBits,
                 uint32_t alphaBits,
                 void *dst)
{
    gl::ColorF color;
    R8G8B8A8::readColor(&color, &rgba);

    switch (redBlueBits)
    {
        case 8:
            ASSERT(greenBits == 8);
            switch (alphaBits)
            {
                case 0:
                    return R8G8B8::writeColor(reinterpret_cast<R8G8B8 *>(dst), &color);
                case 8:
                    return R8G8B8A8::writeColor(reinterpret_cast<R8G8B8A8 *>(dst), &color);
                default:
                    UNREACHABLE();
                    break;
            }
            break;

        case 5:
            switch (greenBits)
            {
                case 6:
                    ASSERT(alphaBits == 0);
                    return R5G6B5::writeColor(reinterpret_cast<R5G6B5 *>(dst), &color);
                case 5:
                    ASSERT(alphaBits == 1);
                    return R5G5B5A1::writeColor(reinterpret_cast<R5G5B5A1 *>(dst), &color);
                default:
                    UNREACHABLE();
                    break;
            }
            break;

        case 4:
            ASSERT(greenBits == 4 && alphaBits == 4);
            return R4G4B4A4::writeColor(reinterpret_cast<R4G4B4A4 *>(dst), &color);

        default:
            UNREACHABLE();
            break;
    }
}

uint32_t R8G8B8A8Key(R8G8B8A8 rgba)
{
    uint32_t key;
    static_assert(sizeof(key) == sizeof(rgba));
    memcpy(&key, &rgba, sizeof(key));
    return key;
}

}  // namespace

void StoreRGBA8ToPalettedImpl(size_t width,
                              size_t height,
                              size_t depth,
                              uint32_t indexBits,
                              uint32_t redBlueBits,
                              uint32_t greenBits,
                              uint32_t alphaBits,
                              const uint8_t *input,
                              size_t inputRowPitch,
                              size_t inputDepthPitch,
                              uint8_t *output,
                              size_t outputRowPitch,
                              size_t outputDepthPitch)
{
    std::unordered_map<uint32_t, size_t> invPalette;

    ASSERT((redBlueBits + greenBits + redBlueBits + alphaBits) % 8 == 0);

    size_t colorBytes   = (redBlueBits + greenBits + redBlueBits + alphaBits) / 8;
    size_t paletteSize  = 1 << indexBits;
    size_t paletteBytes = paletteSize * colorBytes;

    uint8_t *palette = output;

    // We might not fill-out the entire palette.
    memset(palette, 0xab, paletteBytes);

    uint8_t *texels = output + paletteBytes;  // + TODO(http://anglebug.com/42266155): mip levels

    for (size_t z = 0; z < depth; z++)
    {
        for (size_t y = 0; y < height; y++)
        {
            const R8G8B8A8 *srcRow =
                OffsetDataPointer<R8G8B8A8>(input, y, z, inputRowPitch, inputDepthPitch);
            uint8_t *dstRow =
                OffsetDataPointer<uint8_t>(texels, y, z, outputRowPitch, outputDepthPitch);

            for (size_t x = 0; x < width; x++)
            {
                auto inversePaletteEntry = invPalette.insert(
                    std::pair<uint32_t, size_t>(R8G8B8A8Key(srcRow[x]), invPalette.size()));
                size_t paletteIndex = inversePaletteEntry.first->second;
                ASSERT(paletteIndex < paletteSize);
                if (inversePaletteEntry.second)
                {
                    EncodeColor(srcRow[x], redBlueBits, greenBits, alphaBits,
                                palette + paletteIndex * colorBytes);
                }

                switch (indexBits)
                {
                    case 4:
                        // On even xses, initialize the location and store the high
                        // bits, on odd (which always follows even) store the low
                        // bits.
                        if (x % 2 == 0)
                            dstRow[x / 2] = static_cast<uint8_t>(paletteIndex) << 4;
                        else
                            dstRow[x / 2] |= static_cast<uint8_t>(paletteIndex);
                        break;

                    case 8:
                        dstRow[x] = static_cast<uint8_t>(paletteIndex);
                        break;

                    default:
                        UNREACHABLE();
                }
            }
        }
    }
}

}  // namespace angle
