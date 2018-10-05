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

#include "upcast.h"

#include "driver/Handle.h"

#include <filament/Texture.h>

#include <utils/compiler.h>

namespace filament {
namespace details {

class FEngine;
class FStream;

class FTexture : public Texture {
public:
    static bool isTextureFormatSupported(FEngine& engine, InternalFormat format) noexcept;
    static size_t computeTextureDataSize(Texture::Format format, Texture::Type type,
            size_t stride, size_t height, size_t alignment) noexcept;

    FTexture(FEngine& engine, const Builder& builder);

    // frees driver resources, object becomes invalid
    void terminate(FEngine& engine);

    Handle<HwTexture> getHwHandle() const noexcept { return mHandle; }

    size_t getWidth(size_t level = 0) const noexcept;
    size_t getHeight(size_t level = 0) const noexcept;
    size_t getDepth(size_t level = 0) const noexcept;
    size_t getLevels() const noexcept { return mLevels; }
    Sampler getTarget() const noexcept { return mTarget; }
    InternalFormat getFormat() const noexcept { return mFormat; }
    bool isRgbm() const noexcept { return mRgbm; }

    void setImage(FEngine& engine, size_t level,
            uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
            PixelBufferDescriptor&& buffer) const noexcept;

    void setImage(FEngine& engine, size_t level,
            PixelBufferDescriptor&& buffer, const FaceOffsets& faceOffsets) const noexcept;

    void setExternalImage(FEngine& engine, void* image) noexcept;
    void setExternalStream(FEngine& engine, FStream* stream) noexcept;

    void generateMipmaps(FEngine& engine) const noexcept;

    void setSampleCount(size_t sampleCount) noexcept { mSampleCount = uint8_t(sampleCount); }
    size_t getSampleCount() const noexcept { return mSampleCount; }
    bool isMultisample() const noexcept { return mSampleCount > 1; }

    bool isCubemap() const noexcept { return mTarget == Sampler::SAMPLER_CUBEMAP; }

    FStream const* getStream() const noexcept { return mStream; }

    static size_t getFormatSize(InternalFormat format) noexcept;

private:
    friend class Texture;
    Handle<HwTexture> mHandle;
    uint32_t mWidth = 1;
    uint32_t mHeight = 1;
    uint32_t mDepth = 1;
    InternalFormat mFormat = InternalFormat::RGBA8;
    bool mRgbm = false;
    Sampler mTarget = Sampler::SAMPLER_2D;
    uint8_t mLevels = 1;
    uint8_t mSampleCount = 1;
    FStream* mStream = nullptr;
    Usage mUsage = Usage::DEFAULT;
};


FILAMENT_UPCAST(Texture)

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_TEXTURE_H
