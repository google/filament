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

#ifndef TNT_FILAMENT_DETAILS_TEXTURE_H
#define TNT_FILAMENT_DETAILS_TEXTURE_H

#include "downcast.h"

#include <backend/Handle.h>

#include <filament/Texture.h>

#include <utils/compiler.h>

namespace filament {

class FEngine;
class FStream;

class FTexture : public Texture {
public:
    FTexture(FEngine& engine, const Builder& builder);

    // frees driver resources, object becomes invalid
    void terminate(FEngine& engine);

    backend::Handle<backend::HwTexture> getHwHandle() const noexcept { return mHandle; }

    size_t getWidth(size_t level = 0) const noexcept;
    size_t getHeight(size_t level = 0) const noexcept;
    size_t getDepth(size_t level = 0) const noexcept;
    size_t getLevelCount() const noexcept { return mLevelCount; }
    Sampler getTarget() const noexcept { return mTarget; }
    InternalFormat getFormat() const noexcept { return mFormat; }
    Usage getUsage() const noexcept { return mUsage; }

    void setImage(FEngine& engine, size_t level,
            uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
            uint32_t width, uint32_t height, uint32_t depth,
            PixelBufferDescriptor&& buffer) const;

    UTILS_DEPRECATED
    void setImage(FEngine& engine, size_t level,
            PixelBufferDescriptor&& buffer, const FaceOffsets& faceOffsets) const;

    void generatePrefilterMipmap(FEngine& engine,
            PixelBufferDescriptor&& buffer, const FaceOffsets& faceOffsets,
            PrefilterOptions const* options);

    void setExternalImage(FEngine& engine, void* image) noexcept;
    void setExternalImage(FEngine& engine, void* image, size_t plane) noexcept;
    void setExternalStream(FEngine& engine, FStream* stream) noexcept;

    void generateMipmaps(FEngine& engine) const noexcept;

    void setSampleCount(size_t sampleCount) noexcept { mSampleCount = uint8_t(sampleCount); }
    size_t getSampleCount() const noexcept { return mSampleCount; }
    bool isMultisample() const noexcept { return mSampleCount > 1; }
    bool isCompressed() const noexcept { return backend::isCompressedFormat(mFormat); }

    bool isCubemap() const noexcept { return mTarget == Sampler::SAMPLER_CUBEMAP; }

    FStream const* getStream() const noexcept { return mStream; }

    /*
     * Utilities
     */

    // synchronous call to the backend. returns whether a backend supports a particular format.
    static bool isTextureFormatSupported(FEngine& engine, InternalFormat format) noexcept;

    // synchronous call to the backend. returns whether a backend supports texture swizzling.
    static bool isTextureSwizzleSupported(FEngine& engine) noexcept;

    // storage needed on the CPU side for texture data uploads
    static size_t computeTextureDataSize(Texture::Format format, Texture::Type type,
            size_t stride, size_t height, size_t alignment) noexcept;

    // Size a of a pixel in bytes for the given format
    static size_t getFormatSize(InternalFormat format) noexcept;

    // Returns the with or height for a given mipmap level from the base value.
    static inline size_t valueForLevel(uint8_t level, size_t baseLevelValue) {
        return std::max(size_t(1), baseLevelValue >> level);
    }

    // Returns the max number of levels for a texture of given max dimensions
    static inline uint8_t maxLevelCount(uint32_t maxDimension) noexcept {
        return std::max(1, std::ilogbf(float(maxDimension)) + 1);
    }

    // Returns the max number of levels for a texture of given dimensions
    static inline uint8_t maxLevelCount(uint32_t width, uint32_t height) noexcept {
        uint32_t maxDimension = std::max(width, height);
        return maxLevelCount(maxDimension);
    }

    static bool validatePixelFormatAndType(backend::TextureFormat internalFormat,
            backend::PixelDataFormat format, backend::PixelDataType type) noexcept;

private:
    friend class Texture;
    FStream* mStream = nullptr;
    backend::Handle<backend::HwTexture> mHandle;
    uint32_t mWidth = 1;
    uint32_t mHeight = 1;
    uint32_t mDepth = 1;
    InternalFormat mFormat = InternalFormat::RGBA8;
    Sampler mTarget = Sampler::SAMPLER_2D;
    uint8_t mLevelCount = 1;
    uint8_t mSampleCount = 1;
    Usage mUsage = Usage::DEFAULT;
};


FILAMENT_DOWNCAST(Texture)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_TEXTURE_H
