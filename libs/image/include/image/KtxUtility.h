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

#ifndef IMAGE_KTXUTILITY_H
#define IMAGE_KTXUTILITY_H

#include <filament/Engine.h>
#include <filament/Texture.h>

#include <image/KtxBundle.h>

namespace image {

/**
 * Allows clients to create Filament textures from KtxBundle objects.
 *
 * Note that libimage does not have a dependency on libfilament, so for simplicity this is a
 * header-only library with inlined functions.
 */
namespace ktx {

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

    /**
     * Creates a Texture object from a KTX file and populates all of its faces and miplevels.
     *
     * @param engine Used to create the Filament Texture
     * @param ktx In-memory representation of a KTX file
     * @param srgb Forces the KTX-specified format into an SRGB format if possible
     * @param callback Gets called after all texture data has been uploaded to the GPU
     * @param userdata Passed into the callback
     */
    inline Texture* createTexture(Engine* engine, const KtxBundle& ktx, bool srgb,
            Callback callback, void* userdata) {
        using Sampler = Texture::Sampler;
        const auto& ktxinfo = ktx.getInfo();
        const uint32_t nmips = ktx.getNumMipLevels();
        const auto cdatatype = toCompressedPixelDataType(ktxinfo);
        const auto datatype = toPixelDataType(ktxinfo);
        const auto dataformat = toPixelDataFormat(ktxinfo);

        auto texformat = toTextureFormat(ktxinfo);
        if (srgb) {
            if (texformat == Texture::InternalFormat::RGB8) {
                texformat = Texture::InternalFormat::SRGB8;
            }
            if (texformat == Texture::InternalFormat::RGBA8) {
                texformat = Texture::InternalFormat::SRGB8_A8;
            }
        }

        Texture* texture = Texture::Builder()
            .width(ktxinfo.pixelWidth)
            .height(ktxinfo.pixelHeight)
            .levels(static_cast<uint8_t>(nmips))
            .sampler(ktx.isCubemap() ? Sampler::SAMPLER_CUBEMAP : Sampler::SAMPLER_2D)
            .format(texformat)
            .build(*engine);

        struct Userdata {
            uint32_t remainingBuffers;
            Callback callback;
            void* userdata;
        };

        Userdata* cbuser = new Userdata({nmips, callback, userdata});

        PixelBufferDescriptor::Callback cb = [](void*, size_t, void* cbuserptr) {
            Userdata* cbuser = (Userdata*) cbuserptr;
            if (--cbuser->remainingBuffers == 0) {
                if (cbuser->callback) {
                    cbuser->callback(cbuser->userdata);
                }
                delete cbuser;
            }
        };

        uint8_t* data;
        uint32_t size;

        if (isCompressed(ktxinfo)) {
            if (ktx.isCubemap()) {
                for (uint32_t level = 0; level < nmips; ++level) {
                    ktx.getBlob({level, 0, 0}, &data, &size);
                    PixelBufferDescriptor pbd(data, size * 6, cdatatype, size, cb, cbuser);
                    texture->setImage(*engine, level, std::move(pbd), Texture::FaceOffsets(size));
                }
                return texture;
            }
            for (uint32_t level = 0; level < nmips; ++level) {
                ktx.getBlob({level, 0, 0}, &data, &size);
                PixelBufferDescriptor pbd(data, size, cdatatype, size, cb, cbuser);
                texture->setImage(*engine, level, std::move(pbd));
            }
            return texture;
        }

        if (ktx.isCubemap()) {
            for (uint32_t level = 0; level < nmips; ++level) {
                ktx.getBlob({level, 0, 0}, &data, &size);
                PixelBufferDescriptor pbd(data, size * 6, dataformat, datatype, cb, cbuser);
                texture->setImage(*engine, level, std::move(pbd), Texture::FaceOffsets(size));
            }
            return texture;
        }

        for (uint32_t level = 0; level < nmips; ++level) {
            ktx.getBlob({level, 0, 0}, &data, &size);
            PixelBufferDescriptor pbd(data, size, dataformat, datatype, cb, cbuser);
            texture->setImage(*engine, level, std::move(pbd));
        }
        return texture;
    }

    /**
     * Creates a Texture object from a KTX bundle, populates all of its faces and miplevels,
     * and automatically destroys the bundle after all the texture data has been uploaded.
     *
     * @param engine Used to create the Filament Texture
     * @param ktx In-memory representation of a KTX file
     * @param srgb Forces the KTX-specified format into an SRGB format if possible
     */
    inline Texture* createTexture(Engine* engine, KtxBundle* ktx, bool srgb) {
        auto freeKtx = [] (void* userdata) {
            KtxBundle* ktx = (KtxBundle*) userdata;
            delete ktx;
        };
        return createTexture(engine, *ktx, srgb, freeKtx, ktx);
    }

    template<typename T>
    T toCompressedFilamentEnum(uint32_t format) {
        switch (format) {
            case KtxBundle::RGB_S3TC_DXT1: return T::DXT1_RGB;
            case KtxBundle::RGBA_S3TC_DXT1: return T::DXT1_RGBA;
            case KtxBundle::RGBA_S3TC_DXT3: return T::DXT3_RGBA;
            case KtxBundle::RGBA_S3TC_DXT5: return T::DXT5_RGBA;
            case KtxBundle::RGBA_ASTC_4x4: return T::RGBA_ASTC_4x4;
            case KtxBundle::RGBA_ASTC_5x4: return T::RGBA_ASTC_5x4;
            case KtxBundle::RGBA_ASTC_5x5: return T::RGBA_ASTC_5x5;
            case KtxBundle::RGBA_ASTC_6x5: return T::RGBA_ASTC_6x5;
            case KtxBundle::RGBA_ASTC_6x6: return T::RGBA_ASTC_6x6;
            case KtxBundle::RGBA_ASTC_8x5: return T::RGBA_ASTC_8x5;
            case KtxBundle::RGBA_ASTC_8x6: return T::RGBA_ASTC_8x6;
            case KtxBundle::RGBA_ASTC_8x8: return T::RGBA_ASTC_8x8;
            case KtxBundle::RGBA_ASTC_10x5: return T::RGBA_ASTC_10x5;
            case KtxBundle::RGBA_ASTC_10x6: return T::RGBA_ASTC_10x6;
            case KtxBundle::RGBA_ASTC_10x8: return T::RGBA_ASTC_10x8;
            case KtxBundle::RGBA_ASTC_10x10: return T::RGBA_ASTC_10x10;
            case KtxBundle::RGBA_ASTC_12x10: return T::RGBA_ASTC_12x10;
            case KtxBundle::RGBA_ASTC_12x12: return T::RGBA_ASTC_12x12;
            case KtxBundle::SRGB8_ALPHA8_ASTC_4x4: return T::SRGB8_ALPHA8_ASTC_4x4;
            case KtxBundle::SRGB8_ALPHA8_ASTC_5x4: return T::SRGB8_ALPHA8_ASTC_5x4;
            case KtxBundle::SRGB8_ALPHA8_ASTC_5x5: return T::SRGB8_ALPHA8_ASTC_5x5;
            case KtxBundle::SRGB8_ALPHA8_ASTC_6x5: return T::SRGB8_ALPHA8_ASTC_6x5;
            case KtxBundle::SRGB8_ALPHA8_ASTC_6x6: return T::SRGB8_ALPHA8_ASTC_6x6;
            case KtxBundle::SRGB8_ALPHA8_ASTC_8x5: return T::SRGB8_ALPHA8_ASTC_8x5;
            case KtxBundle::SRGB8_ALPHA8_ASTC_8x6: return T::SRGB8_ALPHA8_ASTC_8x6;
            case KtxBundle::SRGB8_ALPHA8_ASTC_8x8: return T::SRGB8_ALPHA8_ASTC_8x8;
            case KtxBundle::SRGB8_ALPHA8_ASTC_10x5: return T::SRGB8_ALPHA8_ASTC_10x5;
            case KtxBundle::SRGB8_ALPHA8_ASTC_10x6: return T::SRGB8_ALPHA8_ASTC_10x6;
            case KtxBundle::SRGB8_ALPHA8_ASTC_10x8: return T::SRGB8_ALPHA8_ASTC_10x8;
            case KtxBundle::SRGB8_ALPHA8_ASTC_10x10: return T::SRGB8_ALPHA8_ASTC_10x10;
            case KtxBundle::SRGB8_ALPHA8_ASTC_12x10: return T::SRGB8_ALPHA8_ASTC_12x10;
            case KtxBundle::SRGB8_ALPHA8_ASTC_12x12: return T::SRGB8_ALPHA8_ASTC_12x12;
            case KtxBundle::R11_EAC: return T::EAC_R11;
            case KtxBundle::SIGNED_R11_EAC: return T::EAC_R11_SIGNED;
            case KtxBundle::RG11_EAC: return T::EAC_RG11;
            case KtxBundle::SIGNED_RG11_EAC: return T::EAC_RG11_SIGNED;
            case KtxBundle::RGB8_ETC2: return T::ETC2_RGB8;
            case KtxBundle::SRGB8_ETC2: return T::ETC2_SRGB8;
            case KtxBundle::RGB8_ALPHA1_ETC2: return T::ETC2_RGB8_A1;
            case KtxBundle::SRGB8_ALPHA1_ETC: return T::ETC2_SRGB8_A1;
            case KtxBundle::RGBA8_ETC2_EAC: return T::ETC2_EAC_RGBA8;
            case KtxBundle::SRGB8_ALPHA8_ETC2_EAC: return T::ETC2_EAC_SRGBA8;
        }
        return (T) 0xffff;
    }

    inline CompressedPixelDataType toCompressedPixelDataType(const KtxInfo& info) {
        return toCompressedFilamentEnum<CompressedPixelDataType>(info.glInternalFormat);
    }

    inline PixelDataType toPixelDataType(const KtxInfo& info) {
        switch (info.glType) {
            case KtxBundle::UNSIGNED_BYTE: return PixelDataType::UBYTE;
            case KtxBundle::UNSIGNED_SHORT: return PixelDataType::USHORT;
            case KtxBundle::HALF_FLOAT: return PixelDataType::HALF;
            case KtxBundle::FLOAT: return PixelDataType::FLOAT;
            case KtxBundle::R11F_G11F_B10F: return PixelDataType::UINT_10F_11F_11F_REV;
        }
        return (PixelDataType) 0xff;
    }

    inline PixelDataFormat toPixelDataFormat(const KtxInfo& info) {
        switch (info.glFormat) {
            case KtxBundle::LUMINANCE:
            case KtxBundle::RED: return PixelDataFormat::R;
            case KtxBundle::RG: return PixelDataFormat::RG;
            case KtxBundle::RGB: return PixelDataFormat::RGB;
            case KtxBundle::RGBA: return PixelDataFormat::RGBA;
            // glFormat should NOT be a sized format according to the spec
            // however cmgen was generating incorrect files until after Filament 1.8.0
            // so we keep this line here to preserve compatibility with older assets
            case KtxBundle::R11F_G11F_B10F: return PixelDataFormat::RGB;
        }
        return (PixelDataFormat) 0xff;
    }

    inline bool isCompressed(const KtxInfo& info) {
        return info.glFormat == 0;
    }

    inline TextureFormat toTextureFormat(const KtxInfo& info) {
        switch (info.glInternalFormat) {
            case KtxBundle::RED: return TextureFormat::R8;
            case KtxBundle::RG: return TextureFormat::RG8;
            case KtxBundle::RGB: return TextureFormat::RGB8;
            case KtxBundle::RGBA: return TextureFormat::RGBA8;
            case KtxBundle::LUMINANCE: return TextureFormat::R8;
            case KtxBundle::LUMINANCE_ALPHA: return TextureFormat::RG8;
            case KtxBundle::R8: return TextureFormat::R8;
            case KtxBundle::R8_SNORM: return TextureFormat::R8_SNORM;
            case KtxBundle::R8UI: return TextureFormat::R8UI;
            case KtxBundle::R8I: return TextureFormat::R8I;
            case KtxBundle::STENCIL_INDEX8: return TextureFormat::STENCIL8;
            case KtxBundle::R16F: return TextureFormat::R16F;
            case KtxBundle::R16UI: return TextureFormat::R16UI;
            case KtxBundle::R16I: return TextureFormat::R16I;
            case KtxBundle::RG8: return TextureFormat::RG8;
            case KtxBundle::RG8_SNORM: return TextureFormat::RG8_SNORM;
            case KtxBundle::RG8UI: return TextureFormat::RG8UI;
            case KtxBundle::RG8I: return TextureFormat::RG8I;
            case KtxBundle::RGB565: return TextureFormat::RGB565;
            case KtxBundle::RGB9_E5: return TextureFormat::RGB9_E5;
            case KtxBundle::RGB5_A1: return TextureFormat::RGB5_A1;
            case KtxBundle::RGBA4: return TextureFormat::RGBA4;
            case KtxBundle::DEPTH_COMPONENT16: return TextureFormat::DEPTH16;
            case KtxBundle::RGB8: return TextureFormat::RGB8;
            case KtxBundle::SRGB8: return TextureFormat::SRGB8;
            case KtxBundle::RGB8_SNORM: return TextureFormat::RGB8_SNORM;
            case KtxBundle::RGB8UI: return TextureFormat::RGB8UI;
            case KtxBundle::RGB8I: return TextureFormat::RGB8I;
            case KtxBundle::R32F: return TextureFormat::R32F;
            case KtxBundle::R32UI: return TextureFormat::R32UI;
            case KtxBundle::R32I: return TextureFormat::R32I;
            case KtxBundle::RG16F: return TextureFormat::RG16F;
            case KtxBundle::RG16UI: return TextureFormat::RG16UI;
            case KtxBundle::RG16I: return TextureFormat::RG16I;
            case KtxBundle::R11F_G11F_B10F: return TextureFormat::R11F_G11F_B10F;
            case KtxBundle::RGBA8: return TextureFormat::RGBA8;
            case KtxBundle::SRGB8_ALPHA8: return TextureFormat::SRGB8_A8;
            case KtxBundle::RGBA8_SNORM: return TextureFormat::RGBA8_SNORM;
            case KtxBundle::RGB10_A2: return TextureFormat::RGB10_A2;
            case KtxBundle::RGBA8UI: return TextureFormat::RGBA8UI;
            case KtxBundle::RGBA8I: return TextureFormat::RGBA8I;
            case KtxBundle::DEPTH24_STENCIL8: return TextureFormat::DEPTH24_STENCIL8;
            case KtxBundle::DEPTH32F_STENCIL8: return TextureFormat::DEPTH32F_STENCIL8;
            case KtxBundle::RGB16F: return TextureFormat::RGB16F;
            case KtxBundle::RGB16UI: return TextureFormat::RGB16UI;
            case KtxBundle::RGB16I: return TextureFormat::RGB16I;
            case KtxBundle::RG32F: return TextureFormat::RG32F;
            case KtxBundle::RG32UI: return TextureFormat::RG32UI;
            case KtxBundle::RG32I: return TextureFormat::RG32I;
            case KtxBundle::RGBA16F: return TextureFormat::RGBA16F;
            case KtxBundle::RGBA16UI: return TextureFormat::RGBA16UI;
            case KtxBundle::RGBA16I: return TextureFormat::RGBA16I;
            case KtxBundle::RGB32F: return TextureFormat::RGB32F;
            case KtxBundle::RGB32UI: return TextureFormat::RGB32UI;
            case KtxBundle::RGB32I: return TextureFormat::RGB32I;
            case KtxBundle::RGBA32F: return TextureFormat::RGBA32F;
            case KtxBundle::RGBA32UI: return TextureFormat::RGBA32UI;
            case KtxBundle::RGBA32I: return TextureFormat::RGBA32I;
        }
        return toCompressedFilamentEnum<TextureFormat>(info.glInternalFormat);
    }

} // namespace ktx

} // namespace image

#endif
