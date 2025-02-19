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

#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <filament/Texture.h>

#include <utils/compiler.h>

#include <stddef.h>
#include <stdint.h>

namespace filament {

class FEngine;
class FStream;

class FTexture : public Texture {
public:
    FTexture(FEngine& engine, const Builder& builder);

    // frees driver resources, object becomes invalid
    void terminate(FEngine& engine);

    backend::Handle<backend::HwTexture> getHwHandle() const noexcept { return mHandle; }
    backend::Handle<backend::HwTexture> getHwHandleForSampling() const noexcept;

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

    void setExternalImage(FEngine& engine, ExternalImageHandleRef image) noexcept;
    void setExternalImage(FEngine& engine, void* image) noexcept;
    void setExternalImage(FEngine& engine, void* image, size_t plane) noexcept;
    void setExternalStream(FEngine& engine, FStream* stream) noexcept;

    void generateMipmaps(FEngine& engine) const noexcept;

    void setSampleCount(size_t const sampleCount) noexcept { mSampleCount = uint8_t(sampleCount); }
    size_t getSampleCount() const noexcept { return mSampleCount; }
    bool isMultisample() const noexcept { return mSampleCount > 1; }
    bool isCompressed() const noexcept { return isCompressedFormat(mFormat); }

    bool isCubemap() const noexcept { return mTarget == Sampler::SAMPLER_CUBEMAP; }

    FStream const* getStream() const noexcept { return mStream; }

    /*
     * Utilities
     */

    // Synchronous call to the backend. Returns whether a backend supports a particular format.
    static bool isTextureFormatSupported(FEngine& engine, InternalFormat format) noexcept;

    // Synchronous call to the backend. Returns whether a backend supports mipmapping of a particular format.
    static bool isTextureFormatMipmappable(FEngine& engine, InternalFormat format) noexcept;

    // Synchronous call to the backend. Returns whether a backend supports protected textures.
    static bool isProtectedTexturesSupported(FEngine& engine) noexcept;

    // Synchronous call to the backend. Returns whether a backend supports texture swizzling.
    static bool isTextureSwizzleSupported(FEngine& engine) noexcept;

    // storage needed on the CPU side for texture data uploads
    static size_t computeTextureDataSize(Format format, Type type,
            size_t stride, size_t height, size_t alignment) noexcept;

    // Size a of a pixel in bytes for the given format
    static size_t getFormatSize(InternalFormat format) noexcept;

    // Returns the with or height for a given mipmap level from the base value.
    static inline size_t valueForLevel(uint8_t const level, size_t const baseLevelValue) {
        return std::max(size_t(1), baseLevelValue >> level);
    }

    // Returns the max number of levels for a texture of given max dimensions
    static inline uint8_t maxLevelCount(uint32_t const maxDimension) noexcept {
        return std::max(1, std::ilogbf(float(maxDimension)) + 1);
    }

    // Returns the max number of levels for a texture of given dimensions
    static inline uint8_t maxLevelCount(uint32_t const width, uint32_t const height) noexcept {
        uint32_t const maxDimension = std::max(width, height);
        return maxLevelCount(maxDimension);
    }

    static bool validatePixelFormatAndType(backend::TextureFormat internalFormat,
            backend::PixelDataFormat format, backend::PixelDataType type) noexcept;

    bool textureHandleCanMutate() const noexcept;
    void updateLodRange(uint8_t level) noexcept;

    // TODO: remove in a future filament release.  See below for description.
    inline bool hasBlitSrcUsage() const noexcept {
        return mHasBlitSrc;
    }

private:
    friend class Texture;
    struct LodRange {
        // 0,0 means lod-range unset (all levels are available)
        uint8_t first = 0;  // first lod
        uint8_t last = 0;   // 1 past last lod
        bool empty() const noexcept { return first == last; }
        size_t size() const noexcept { return last - first; }
    };

    bool hasAllLods(LodRange const range) const noexcept {
        return range.first == 0 && range.last == mLevelCount;
    }

    void updateLodRange(uint8_t baseLevel, uint8_t levelCount) noexcept;
    void setHandles(backend::Handle<backend::HwTexture> handle) noexcept;
    backend::Handle<backend::HwTexture> setHandleForSampling(
            backend::Handle<backend::HwTexture> handle) const noexcept;
    static backend::Handle<backend::HwTexture> createPlaceholderTexture(
            backend::DriverApi& driver) noexcept;

    backend::Handle<backend::HwTexture> mHandle;
    mutable backend::Handle<backend::HwTexture> mHandleForSampling;
    backend::DriverApi* mDriver = nullptr; // this is only needed for getHwHandleForSampling()
    LodRange mLodRange{};
    mutable LodRange mActiveLodRange{};

    uint32_t mWidth = 1;
    uint32_t mHeight = 1;
    uint32_t mDepth = 1;
    InternalFormat mFormat = InternalFormat::RGBA8;
    Sampler mTarget = Sampler::SAMPLER_2D;
    uint8_t mLevelCount = 1;
    uint8_t mSampleCount = 1;
    std::array<Swizzle, 4> mSwizzle = {
           Swizzle::CHANNEL_0, Swizzle::CHANNEL_1,
           Swizzle::CHANNEL_2, Swizzle::CHANNEL_3 };
    bool mTextureIsSwizzled;

    Usage mUsage = Usage::DEFAULT;

    // TODO: remove in a future filament release.
    // Indicates whether the user has set the TextureUsage::BLIT_SRC usage. This will be used to
    // temporarily validate whether this texture can be used for readPixels.
    bool mHasBlitSrc = false;
    bool mExternal = false;
    // there is 4 bytes of padding here

    FStream* mStream = nullptr; // only needed for streaming textures
};

FILAMENT_DOWNCAST(Texture)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_TEXTURE_H
