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

#include <ktxreader/Ktx1Reader.h>
#include <utils/Log.h>

#include <filament/Engine.h>
#include <filament/Texture.h>

namespace ktxreader {
namespace Ktx1Reader {

Texture* createTexture(Engine* engine, const Ktx1Bundle& ktx, bool srgb,
        Callback callback, void* userdata) {
    using Sampler = Texture::Sampler;
    const auto& ktxinfo = ktx.getInfo();
    const uint32_t nmips = ktx.getNumMipLevels();
    const auto cdatatype = toCompressedPixelDataType(ktxinfo);
    const auto datatype = toPixelDataType(ktxinfo);
    const auto dataformat = toPixelDataFormat(ktxinfo);

    auto texformat = toTextureFormat(ktxinfo);

#ifndef NDEBUG
    if (srgb && !isSrgbTextureFormat(texformat)) {
        utils::slog.w << "Requested sRGB format but KTX contains a linear format. "
            << utils::io::endl;
    } else if (!srgb && isSrgbTextureFormat(texformat)) {
        utils::slog.w << "Requested linear format but KTX contains a sRGB format. "
            << utils::io::endl;
    }
#endif

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
                ktx.getBlob({ level, 0, 0 }, &data, &size);
                const uint32_t dim = texture->getWidth(level);
                texture->setImage(*engine, level, 0, 0, 0, dim, dim, 6, {
                        data, size * 6, cdatatype, size, cb, cbuser
                });
            }
            return texture;
        }
        for (uint32_t level = 0; level < nmips; ++level) {
            ktx.getBlob({ level, 0, 0 }, &data, &size);
            texture->setImage(*engine, level, {
                    data, size, cdatatype, size, cb, cbuser
            });
        }
        return texture;
    }

    if (ktx.isCubemap()) {
        for (uint32_t level = 0; level < nmips; ++level) {
            ktx.getBlob({ level, 0, 0 }, &data, &size);
            const uint32_t dim = texture->getWidth(level);
            texture->setImage(*engine, level, 0, 0, 0, dim, dim, 6, {
                    data, size * 6, dataformat, datatype, cb, cbuser
            });
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

Texture* createTexture(Engine* engine, Ktx1Bundle* ktx, bool srgb) {
    auto freeKtx = [] (void* userdata) {
        Ktx1Bundle* ktx = (Ktx1Bundle*) userdata;
        delete ktx;
    };
    return createTexture(engine, *ktx, srgb, freeKtx, ktx);
}

CompressedPixelDataType toCompressedPixelDataType(const KtxInfo& info) {
    return toCompressedFilamentEnum<CompressedPixelDataType>(info.glInternalFormat);
}

PixelDataType toPixelDataType(const KtxInfo& info) {
    switch (info.glType) {
        case Ktx1Bundle::UNSIGNED_BYTE: return PixelDataType::UBYTE;
        case Ktx1Bundle::UNSIGNED_SHORT: return PixelDataType::USHORT;
        case Ktx1Bundle::HALF_FLOAT: return PixelDataType::HALF;
        case Ktx1Bundle::FLOAT: return PixelDataType::FLOAT;
        case Ktx1Bundle::R11F_G11F_B10F: return PixelDataType::UINT_10F_11F_11F_REV;
    }
    return (PixelDataType) 0xff;
}

PixelDataFormat toPixelDataFormat(const KtxInfo& info) {
    switch (info.glFormat) {
        case Ktx1Bundle::LUMINANCE:
        case Ktx1Bundle::RED: return PixelDataFormat::R;
        case Ktx1Bundle::RG: return PixelDataFormat::RG;
        case Ktx1Bundle::RGB: return PixelDataFormat::RGB;
        case Ktx1Bundle::RGBA: return PixelDataFormat::RGBA;
        // glFormat should NOT be a sized format according to the spec
        // however cmgen was generating incorrect files until after Filament 1.8.0
        // so we keep this line here to preserve compatibility with older assets
        case Ktx1Bundle::R11F_G11F_B10F: return PixelDataFormat::RGB;
    }
    return (PixelDataFormat) 0xff;
}

bool isCompressed(const KtxInfo& info) {
    return info.glFormat == 0;
}

bool isSrgbTextureFormat(TextureFormat format) {
    switch(format) {
        // Non-compressed
        case Texture::InternalFormat::RGB8:
        case Texture::InternalFormat::RGBA8:
            return false;

        // ASTC
        case Texture::InternalFormat::RGBA_ASTC_4x4:
        case Texture::InternalFormat::RGBA_ASTC_5x4:
        case Texture::InternalFormat::RGBA_ASTC_5x5:
        case Texture::InternalFormat::RGBA_ASTC_6x5:
        case Texture::InternalFormat::RGBA_ASTC_6x6:
        case Texture::InternalFormat::RGBA_ASTC_8x5:
        case Texture::InternalFormat::RGBA_ASTC_8x6:
        case Texture::InternalFormat::RGBA_ASTC_8x8:
        case Texture::InternalFormat::RGBA_ASTC_10x5:
        case Texture::InternalFormat::RGBA_ASTC_10x6:
        case Texture::InternalFormat::RGBA_ASTC_10x8:
        case Texture::InternalFormat::RGBA_ASTC_10x10:
        case Texture::InternalFormat::RGBA_ASTC_12x10:
        case Texture::InternalFormat::RGBA_ASTC_12x12:
            return false;

        // ETC2
        case Texture::InternalFormat::ETC2_RGB8:
        case Texture::InternalFormat::ETC2_RGB8_A1:
        case Texture::InternalFormat::ETC2_EAC_RGBA8:
            return false;

        // DXT
        case Texture::InternalFormat::DXT1_RGB:
        case Texture::InternalFormat::DXT1_RGBA:
        case Texture::InternalFormat::DXT3_RGBA:
        case Texture::InternalFormat::DXT5_RGBA:
            return false;

        // RGTC
        case Texture::InternalFormat::RED_RGTC1:
        case Texture::InternalFormat::SIGNED_RED_RGTC1:
        case Texture::InternalFormat::RED_GREEN_RGTC2:
        case Texture::InternalFormat::SIGNED_RED_GREEN_RGTC2:
            return false;

        // BPTC
        case Texture::InternalFormat::RGB_BPTC_SIGNED_FLOAT:
        case Texture::InternalFormat::RGB_BPTC_UNSIGNED_FLOAT:
        case Texture::InternalFormat::RGBA_BPTC_UNORM:
            return false;

        default:
            return true;
    }
}

TextureFormat toTextureFormat(const KtxInfo& info) {
    switch (info.glInternalFormat) {
        case Ktx1Bundle::RED: return TextureFormat::R8;
        case Ktx1Bundle::RG: return TextureFormat::RG8;
        case Ktx1Bundle::RGB: return TextureFormat::RGB8;
        case Ktx1Bundle::RGBA: return TextureFormat::RGBA8;
        case Ktx1Bundle::LUMINANCE: return TextureFormat::R8;
        case Ktx1Bundle::LUMINANCE_ALPHA: return TextureFormat::RG8;
        case Ktx1Bundle::R8: return TextureFormat::R8;
        case Ktx1Bundle::R8_SNORM: return TextureFormat::R8_SNORM;
        case Ktx1Bundle::R8UI: return TextureFormat::R8UI;
        case Ktx1Bundle::R8I: return TextureFormat::R8I;
        case Ktx1Bundle::STENCIL_INDEX8: return TextureFormat::STENCIL8;
        case Ktx1Bundle::R16F: return TextureFormat::R16F;
        case Ktx1Bundle::R16UI: return TextureFormat::R16UI;
        case Ktx1Bundle::R16I: return TextureFormat::R16I;
        case Ktx1Bundle::RG8: return TextureFormat::RG8;
        case Ktx1Bundle::RG8_SNORM: return TextureFormat::RG8_SNORM;
        case Ktx1Bundle::RG8UI: return TextureFormat::RG8UI;
        case Ktx1Bundle::RG8I: return TextureFormat::RG8I;
        case Ktx1Bundle::RGB565: return TextureFormat::RGB565;
        case Ktx1Bundle::RGB9_E5: return TextureFormat::RGB9_E5;
        case Ktx1Bundle::RGB5_A1: return TextureFormat::RGB5_A1;
        case Ktx1Bundle::RGBA4: return TextureFormat::RGBA4;
        case Ktx1Bundle::DEPTH_COMPONENT16: return TextureFormat::DEPTH16;
        case Ktx1Bundle::RGB8: return TextureFormat::RGB8;
        case Ktx1Bundle::SRGB8: return TextureFormat::SRGB8;
        case Ktx1Bundle::RGB8_SNORM: return TextureFormat::RGB8_SNORM;
        case Ktx1Bundle::RGB8UI: return TextureFormat::RGB8UI;
        case Ktx1Bundle::RGB8I: return TextureFormat::RGB8I;
        case Ktx1Bundle::R32F: return TextureFormat::R32F;
        case Ktx1Bundle::R32UI: return TextureFormat::R32UI;
        case Ktx1Bundle::R32I: return TextureFormat::R32I;
        case Ktx1Bundle::RG16F: return TextureFormat::RG16F;
        case Ktx1Bundle::RG16UI: return TextureFormat::RG16UI;
        case Ktx1Bundle::RG16I: return TextureFormat::RG16I;
        case Ktx1Bundle::R11F_G11F_B10F: return TextureFormat::R11F_G11F_B10F;
        case Ktx1Bundle::RGBA8: return TextureFormat::RGBA8;
        case Ktx1Bundle::SRGB8_ALPHA8: return TextureFormat::SRGB8_A8;
        case Ktx1Bundle::RGBA8_SNORM: return TextureFormat::RGBA8_SNORM;
        case Ktx1Bundle::RGB10_A2: return TextureFormat::RGB10_A2;
        case Ktx1Bundle::RGBA8UI: return TextureFormat::RGBA8UI;
        case Ktx1Bundle::RGBA8I: return TextureFormat::RGBA8I;
        case Ktx1Bundle::DEPTH24_STENCIL8: return TextureFormat::DEPTH24_STENCIL8;
        case Ktx1Bundle::DEPTH32F_STENCIL8: return TextureFormat::DEPTH32F_STENCIL8;
        case Ktx1Bundle::RGB16F: return TextureFormat::RGB16F;
        case Ktx1Bundle::RGB16UI: return TextureFormat::RGB16UI;
        case Ktx1Bundle::RGB16I: return TextureFormat::RGB16I;
        case Ktx1Bundle::RG32F: return TextureFormat::RG32F;
        case Ktx1Bundle::RG32UI: return TextureFormat::RG32UI;
        case Ktx1Bundle::RG32I: return TextureFormat::RG32I;
        case Ktx1Bundle::RGBA16F: return TextureFormat::RGBA16F;
        case Ktx1Bundle::RGBA16UI: return TextureFormat::RGBA16UI;
        case Ktx1Bundle::RGBA16I: return TextureFormat::RGBA16I;
        case Ktx1Bundle::RGB32F: return TextureFormat::RGB32F;
        case Ktx1Bundle::RGB32UI: return TextureFormat::RGB32UI;
        case Ktx1Bundle::RGB32I: return TextureFormat::RGB32I;
        case Ktx1Bundle::RGBA32F: return TextureFormat::RGBA32F;
        case Ktx1Bundle::RGBA32UI: return TextureFormat::RGBA32UI;
        case Ktx1Bundle::RGBA32I: return TextureFormat::RGBA32I;
    }
    return toCompressedFilamentEnum<TextureFormat>(info.glInternalFormat);
}

} // namespace Ktx1Reader
} // namespace ktxreader
