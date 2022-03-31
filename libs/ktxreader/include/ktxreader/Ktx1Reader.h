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
    TextureFormat toSrgbTextureFormat(TextureFormat tex);

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
