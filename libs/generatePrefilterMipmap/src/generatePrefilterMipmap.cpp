/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "filament-generatePrefilterMipmap/generatePrefilterMipmap.h"

#include <filament/Engine.h>
#include <filament/Texture.h>

#include <backend/DriverEnums.h>
#include <backend/PixelBufferDescriptor.h>

#include <ibl/Cubemap.h>
#include <ibl/CubemapIBL.h>
#include <ibl/CubemapUtils.h>
#include <ibl/Image.h>


#include <utils/compiler.h>
#include <utils/FixedCapacityVector.h>
#include <utils/JobSystem.h>
#include <utils/Panic.h>

#include <utility>

#include <stddef.h>
#include <stdint.h>

namespace filament {

using namespace backend;
using namespace utils;

void generatePrefilterMipmap(Texture* const texture, Engine& engine,
        PixelBufferDescriptor&& buffer, const FaceOffsets& faceOffsets,
        PrefilterOptions const* options) {
    using namespace ibl;
    using namespace backend;
    using namespace math;

    const size_t size = texture->getWidth();
    const size_t stride = buffer.stride ? buffer.stride : size;

    /* validate input data */

    FILAMENT_CHECK_PRECONDITION(
            buffer.format == PixelDataFormat::RGB || buffer.format == PixelDataFormat::RGBA)
            << "input data format must be RGB or RGBA";

    FILAMENT_CHECK_PRECONDITION(buffer.type == PixelDataType::FLOAT ||
            buffer.type == PixelDataType::HALF ||
            buffer.type == PixelDataType::UINT_10F_11F_11F_REV)
            << "input data type must be FLOAT, HALF or UINT_10F_11F_11F_REV";

    /* validate texture */

    FILAMENT_CHECK_PRECONDITION(!(size & (size - 1)))
            << "input data cubemap dimensions must be a power-of-two";

    FILAMENT_CHECK_PRECONDITION(!Texture::isTextureFormatCompressed(texture->getFormat()))
            << "reflections texture cannot be compressed";

    PrefilterOptions const defaultOptions;
    options = options ? options : &defaultOptions;

    JobSystem& js = engine.getJobSystem();

    auto generateMipmaps = [](JobSystem& js,
            FixedCapacityVector<Cubemap>& levels, FixedCapacityVector<Image>& images) {
        Image temp;
        const Cubemap& base(levels[0]);
        size_t dim = base.getDimensions();
        size_t mipLevel = 0;
        while (dim > 1) {
            dim >>= 1u;
            Cubemap dst = CubemapUtils::create(temp, dim);
            const Cubemap& src(levels[mipLevel++]);
            CubemapUtils::downsampleCubemapLevelBoxFilter(js, dst, src);
            dst.makeSeamless();
            images.push_back(std::move(temp));
            levels.push_back(std::move(dst));
        }
    };


    /*
     * Create a Cubemap data structure
     */

    size_t bytesPerPixel = 0;

    switch (buffer.format) {
        case PixelDataFormat::RGB:
            bytesPerPixel = 3;
            break;
        case PixelDataFormat::RGBA:
            bytesPerPixel = 4;
            break;
        default:
            // this cannot happen due to the checks above
            break;
    }

    switch (buffer.type) { // NOLINT
        case PixelDataType::FLOAT:
            bytesPerPixel *= 4;
            break;
        case PixelDataType::HALF:
            bytesPerPixel *= 2;
            break;
        default:
            // this cannot happen due to the checks above
            break;
    }
    assert_invariant(bytesPerPixel);

    Image temp;
    Cubemap cml = CubemapUtils::create(temp, size);
    for (size_t j = 0; j < 6; j++) {
        Cubemap::Face const face = Cubemap::Face(j);
        Image const& image = cml.getImageForFace(face);
        for (size_t y = 0; y < size; y++) {
            Cubemap::Texel* out = static_cast<Cubemap::Texel*>(image.getPixelRef(0, y));
            if (buffer.type == PixelDataType::FLOAT) {
                float3 const* src = pointermath::add((float3 const*)buffer.buffer, faceOffsets[j]);
                src = pointermath::add(src, y * stride * bytesPerPixel);
                for (size_t x = 0; x < size; x++, out++) {
                    Cubemap::writeAt(out, *src);
                    src = pointermath::add(src, bytesPerPixel);
                }
            } else if (buffer.type == PixelDataType::HALF) {
                half3 const* src = pointermath::add((half3 const*)buffer.buffer, faceOffsets[j]);
                src = pointermath::add(src, y * stride * bytesPerPixel);
                for (size_t x = 0; x < size; x++, out++) {
                    Cubemap::writeAt(out, *src);
                    src = pointermath::add(src, bytesPerPixel);
                }
            } else if (buffer.type == PixelDataType::UINT_10F_11F_11F_REV) {
                // this doesn't depend on buffer.format
                uint32_t const* src = reinterpret_cast<uint32_t const*>(
                                              static_cast<char const*>(buffer.buffer)
                                              + faceOffsets[j]) + y * stride;
                for (size_t x = 0; x < size; x++, out++, src++) {
                    using fp10 = fp<0, 5, 5>;
                    using fp11 = fp<0, 5, 6>;
                    fp11 const r{ uint16_t( *src         & 0x7FFu) };
                    fp11 const g{ uint16_t((*src >> 11u) & 0x7FFu) };
                    fp10 const b{ uint16_t((*src >> 22u) & 0x3FFu) };
                    Cubemap::Texel const texel{ fp11::tof(r), fp11::tof(g), fp10::tof(b) };
                    Cubemap::writeAt(out, texel);
                }
            }
        }
    }

    /*
     * Create the mipmap chain
     */

    auto images = FixedCapacityVector<Image>::with_capacity(texture->getLevels());
    auto levels = FixedCapacityVector<Cubemap>::with_capacity(texture->getLevels());

    images.push_back(std::move(temp));
    levels.push_back(std::move(cml));

    const float3 mirror = options->mirror ? float3{ -1, 1, 1 } : float3{ 1, 1, 1 };

    // make the cubemap seamless
    levels[0].makeSeamless();

    // Now generate all the mipmap levels
    generateMipmaps(js, levels, images);

    // Finally generate each pre-filtered mipmap level
    const size_t baseExp = ctz(size);
    size_t const numSamples = options->sampleCount;
    const size_t numLevels = baseExp + 1;
    for (ssize_t i = (ssize_t)baseExp; i >= 0; --i) {
        const size_t dim = 1U << i;
        const size_t level = baseExp - i;
        const float lod = saturate(float(level) / float(numLevels - 1));
        const float linearRoughness = lod * lod;

        Image* image = new Image;
        Cubemap dst = CubemapUtils::create(*image, dim);
        CubemapIBL::roughnessFilter(js, dst, { levels.begin(), uint32_t(levels.size()) },
                linearRoughness, numSamples, mirror, true);

        for (size_t j = 0; j < 6; j++) {
            Image const& faceImage = dst.getImageForFace(Cubemap::Face(j));
            texture->setImage(engine, level, 0, 0, j, dim, dim, 1,
                    PixelBufferDescriptor::make((char*) faceImage.getData(),
                            dim * faceImage.getStride() * 3 * sizeof(float),
                            PixelBufferDescriptor::PixelDataFormat::RGB,
                            PixelBufferDescriptor::PixelDataType::FLOAT, 1,
                            0, 0, uint32_t(faceImage.getStride()),
                            [j, image](void const*, size_t) {
                                // free the buffer only when processing the last image
                                if (j == 5) {
                                    delete image;
                                }
                            }));
        }
    }

    // no need to call the user callback because buffer is a reference, and it'll be destroyed
    // by the caller (without being move()d here).
}

} // namespace filament
