/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef KTXREADER_KTX1READER_H
#define KTXREADER_KTX1READER_H

#include <image/Ktx1Bundle.h>

#include <filament/Texture.h>

namespace filament {
    class Engine;
}

namespace ktxreader {

using KtxInfo = image::KtxInfo;
using Ktx1Bundle = image::Ktx1Bundle;

/**
 * Allows clients to create Filament textures from Ktx1Bundle objects.
 */
namespace Ktx1Reader {

    using Texture = filament::Texture;
    using Engine = filament::Engine;

    using TextureFormat = Texture::InternalFormat;
    using CompressedPixelDataType = Texture::CompressedType;
    using PixelDataType = Texture::Type;
    using PixelDataFormat = Texture::Format;
    using PixelBufferDescriptor = Texture::PixelBufferDescriptor;

    using Callback = void(*)(void* userdata);

    CompressedPixelDataType toCompressedPixelDataType(const KtxInfo& info);
    PixelDataType toPixelDataType(const KtxInfo& info);
    PixelDataFormat toPixelDataFormat(const KtxInfo& info);
    bool isCompressed(const KtxInfo& info);
    TextureFormat toTextureFormat(const KtxInfo& info);

    template<typename T>
    T toCompressedFilamentEnum(uint32_t format) {
        switch (format) {
            case Ktx1Bundle::RGB_S3TC_DXT1: return T::DXT1_RGB;
            case Ktx1Bundle::RGBA_S3TC_DXT1: return T::DXT1_RGBA;
            case Ktx1Bundle::RGBA_S3TC_DXT3: return T::DXT3_RGBA;
            case Ktx1Bundle::RGBA_S3TC_DXT5: return T::DXT5_RGBA;
            case Ktx1Bundle::R_RGTC_BC4_UNORM: return T::RED_RGTC1;
            case Ktx1Bundle::R_RGTC_BC4_SNORM: return T::SIGNED_RED_RGTC1;
            case Ktx1Bundle::RG_RGTC_BC5_UNORM: return T::RED_GREEN_RGTC2;
            case Ktx1Bundle::RG_RGTC_BC5_SNORM: return T::SIGNED_RED_GREEN_RGTC2;
            case Ktx1Bundle::RGBA_BPTC_BC7: return T::RGBA_BPTC_UNORM;
            case Ktx1Bundle::SRGB8_ALPHA8_BPTC_BC7: return T::SRGB_ALPHA_BPTC_UNORM;
            case Ktx1Bundle::RGB_BPTC_BC6H_SNORM: return T::RGB_BPTC_SIGNED_FLOAT;
            case Ktx1Bundle::RGB_BPTC_BC6H_UNORM: return T::RGB_BPTC_UNSIGNED_FLOAT;
            case Ktx1Bundle::RGBA_ASTC_4x4: return T::RGBA_ASTC_4x4;
            case Ktx1Bundle::RGBA_ASTC_5x4: return T::RGBA_ASTC_5x4;
            case Ktx1Bundle::RGBA_ASTC_5x5: return T::RGBA_ASTC_5x5;
            case Ktx1Bundle::RGBA_ASTC_6x5: return T::RGBA_ASTC_6x5;
            case Ktx1Bundle::RGBA_ASTC_6x6: return T::RGBA_ASTC_6x6;
            case Ktx1Bundle::RGBA_ASTC_8x5: return T::RGBA_ASTC_8x5;
            case Ktx1Bundle::RGBA_ASTC_8x6: return T::RGBA_ASTC_8x6;
            case Ktx1Bundle::RGBA_ASTC_8x8: return T::RGBA_ASTC_8x8;
            case Ktx1Bundle::RGBA_ASTC_10x5: return T::RGBA_ASTC_10x5;
            case Ktx1Bundle::RGBA_ASTC_10x6: return T::RGBA_ASTC_10x6;
            case Ktx1Bundle::RGBA_ASTC_10x8: return T::RGBA_ASTC_10x8;
            case Ktx1Bundle::RGBA_ASTC_10x10: return T::RGBA_ASTC_10x10;
            case Ktx1Bundle::RGBA_ASTC_12x10: return T::RGBA_ASTC_12x10;
            case Ktx1Bundle::RGBA_ASTC_12x12: return T::RGBA_ASTC_12x12;
            case Ktx1Bundle::SRGB8_ALPHA8_ASTC_4x4: return T::SRGB8_ALPHA8_ASTC_4x4;
            case Ktx1Bundle::SRGB8_ALPHA8_ASTC_5x4: return T::SRGB8_ALPHA8_ASTC_5x4;
            case Ktx1Bundle::SRGB8_ALPHA8_ASTC_5x5: return T::SRGB8_ALPHA8_ASTC_5x5;
            case Ktx1Bundle::SRGB8_ALPHA8_ASTC_6x5: return T::SRGB8_ALPHA8_ASTC_6x5;
            case Ktx1Bundle::SRGB8_ALPHA8_ASTC_6x6: return T::SRGB8_ALPHA8_ASTC_6x6;
            case Ktx1Bundle::SRGB8_ALPHA8_ASTC_8x5: return T::SRGB8_ALPHA8_ASTC_8x5;
            case Ktx1Bundle::SRGB8_ALPHA8_ASTC_8x6: return T::SRGB8_ALPHA8_ASTC_8x6;
            case Ktx1Bundle::SRGB8_ALPHA8_ASTC_8x8: return T::SRGB8_ALPHA8_ASTC_8x8;
            case Ktx1Bundle::SRGB8_ALPHA8_ASTC_10x5: return T::SRGB8_ALPHA8_ASTC_10x5;
            case Ktx1Bundle::SRGB8_ALPHA8_ASTC_10x6: return T::SRGB8_ALPHA8_ASTC_10x6;
            case Ktx1Bundle::SRGB8_ALPHA8_ASTC_10x8: return T::SRGB8_ALPHA8_ASTC_10x8;
            case Ktx1Bundle::SRGB8_ALPHA8_ASTC_10x10: return T::SRGB8_ALPHA8_ASTC_10x10;
            case Ktx1Bundle::SRGB8_ALPHA8_ASTC_12x10: return T::SRGB8_ALPHA8_ASTC_12x10;
            case Ktx1Bundle::SRGB8_ALPHA8_ASTC_12x12: return T::SRGB8_ALPHA8_ASTC_12x12;
            case Ktx1Bundle::R11_EAC: return T::EAC_R11;
            case Ktx1Bundle::SIGNED_R11_EAC: return T::EAC_R11_SIGNED;
            case Ktx1Bundle::RG11_EAC: return T::EAC_RG11;
            case Ktx1Bundle::SIGNED_RG11_EAC: return T::EAC_RG11_SIGNED;
            case Ktx1Bundle::RGB8_ETC2: return T::ETC2_RGB8;
            case Ktx1Bundle::SRGB8_ETC2: return T::ETC2_SRGB8;
            case Ktx1Bundle::RGB8_ALPHA1_ETC2: return T::ETC2_RGB8_A1;
            case Ktx1Bundle::SRGB8_ALPHA1_ETC: return T::ETC2_SRGB8_A1;
            case Ktx1Bundle::RGBA8_ETC2_EAC: return T::ETC2_EAC_RGBA8;
            case Ktx1Bundle::SRGB8_ALPHA8_ETC2_EAC: return T::ETC2_EAC_SRGBA8;
        }
        return (T) 0xffff;
    }

    /**
     * Creates a Texture object from a KTX file and populates all of its faces and miplevels.
     *
     * @param engine Used to create the Filament Texture
     * @param ktx In-memory representation of a KTX file
     * @param srgb Requests an sRGB format from the KTX file
     * @param callback Gets called after all texture data has been uploaded to the GPU
     * @param userdata Passed into the callback
     */
    Texture* createTexture(Engine* engine, const Ktx1Bundle& ktx, bool srgb,
            Callback callback, void* userdata);

    /**
     * Creates a Texture object from a KTX bundle, populates all of its faces and miplevels,
     * and automatically destroys the bundle after all the texture data has been uploaded.
     *
     * @param engine Used to create the Filament Texture
     * @param ktx In-memory representation of a KTX file
     * @param srgb Requests an sRGB format from the KTX file
     */
    Texture* createTexture(Engine* engine, Ktx1Bundle* ktx, bool srgb);

    CompressedPixelDataType toCompressedPixelDataType(const KtxInfo& info);

    PixelDataType toPixelDataType(const KtxInfo& info);

    PixelDataFormat toPixelDataFormat(const KtxInfo& info);

    bool isCompressed(const KtxInfo& info);

    bool isSrgbTextureFormat(TextureFormat format);

    TextureFormat toTextureFormat(const KtxInfo& info);

} // namespace Ktx1Reader
} // namespace ktxreader

#endif
