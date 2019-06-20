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

#include "details/Texture.h"

#include "details/Engine.h"
#include "details/Stream.h"

#include "FilamentAPI-impl.h"

#include <utils/Panic.h>

namespace filament {

using namespace details;
using namespace backend;

struct Texture::BuilderDetails {
    uint32_t mWidth = 1;
    uint32_t mHeight = 1;
    uint32_t mDepth = 1;
    uint8_t mLevels = 1;
    Sampler mTarget = Sampler::SAMPLER_2D;
    InternalFormat mFormat = InternalFormat::RGBA8;
    Usage mUsage = Usage::DEFAULT;
};

using BuilderType = Texture;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;


Texture::Builder& Texture::Builder::width(uint32_t width) noexcept {
    mImpl->mWidth = width;
    return *this;
}

Texture::Builder& Texture::Builder::height(uint32_t height) noexcept {
    mImpl->mHeight = height;
    return *this;
}

Texture::Builder& Texture::Builder::depth(uint32_t depth) noexcept {
    mImpl->mDepth = depth;
    return *this;
}

Texture::Builder& Texture::Builder::levels(uint8_t levels) noexcept {
    mImpl->mLevels = std::max(uint8_t(1), levels);
    return *this;
}

Texture::Builder& Texture::Builder::sampler(Texture::Sampler target) noexcept {
    mImpl->mTarget = target;
    return *this;
}

Texture::Builder& Texture::Builder::format(Texture::InternalFormat format) noexcept {
    mImpl->mFormat = format;
    return *this;
}

Texture::Builder& Texture::Builder::usage(Texture::Usage usage) noexcept {
    mImpl->mUsage = Texture::Usage(usage);
    return *this;
}

Texture* Texture::Builder::build(Engine& engine) {
    if (!ASSERT_POSTCONDITION_NON_FATAL(Texture::isTextureFormatSupported(engine, mImpl->mFormat),
            "Texture format %u not supported on this platform", mImpl->mFormat)) {
        return nullptr;
    }
    return upcast(engine).createTexture(*this);
}

// ------------------------------------------------------------------------------------------------

namespace details {

FTexture::FTexture(FEngine& engine, const Builder& builder) {
    mWidth  = static_cast<uint32_t>(builder->mWidth);
    mHeight = static_cast<uint32_t>(builder->mHeight);
    mFormat = builder->mFormat;
    mUsage = builder->mUsage;
    mTarget = builder->mTarget;
    mDepth  = static_cast<uint32_t>(builder->mDepth);
    mLevelCount = std::min(builder->mLevels,
            static_cast<uint8_t>(std::ilogbf(std::max(mWidth, mHeight)) + 1));

    FEngine::DriverApi& driver = engine.getDriverApi();
    mHandle = driver.createTexture(
            mTarget, mLevelCount, mFormat, mSampleCount, mWidth, mHeight, mDepth, mUsage);
}

// frees driver resources, object becomes invalid
void FTexture::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.destroyTexture(mHandle);
}

size_t FTexture::getWidth(size_t level) const noexcept {
    return valueForLevel(level, mWidth);
}

size_t FTexture::getHeight(size_t level) const noexcept {
    return valueForLevel(level, mHeight);
}

size_t FTexture::getDepth(size_t level) const noexcept {
    return valueForLevel(level, mDepth);
}

void FTexture::setImage(FEngine& engine,
        size_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        Texture::PixelBufferDescriptor&& buffer) const noexcept {
    if (!mStream && mTarget != Sampler::SAMPLER_CUBEMAP && level < mLevelCount) {
        if (buffer.buffer) {
            engine.getDriverApi().update2DImage(mHandle,
                    uint8_t(level), xoffset, yoffset, width, height, std::move(buffer));
        }
    }
}

void FTexture::setImage(FEngine& engine, size_t level,
        Texture::PixelBufferDescriptor&& buffer, const FaceOffsets& faceOffsets) const noexcept {
    if (!mStream && mTarget == Sampler::SAMPLER_CUBEMAP && level < mLevelCount) {
        if (buffer.buffer) {
            engine.getDriverApi().updateCubeImage(mHandle, uint8_t(level),
                    std::move(buffer), faceOffsets);
        }
    }
}

void FTexture::setExternalImage(FEngine& engine, void* image) noexcept {
    if (mTarget == Sampler::SAMPLER_EXTERNAL) {
        // The call to setupExternalImage is synchronous, and allows the driver to take ownership of
        // the external image on this thread, if necessary.
        engine.getDriverApi().setupExternalImage(image);
        engine.getDriverApi().setExternalImage(mHandle, image);
    }
}

void FTexture::setExternalStream(FEngine& engine, FStream* stream) noexcept {
    if (stream) {
        if (!ASSERT_POSTCONDITION_NON_FATAL(mTarget == Sampler::SAMPLER_EXTERNAL,
                "Texture target must be SAMPLER_EXTERNAL")) {
            return;
        }
        mStream = stream;
        engine.getDriverApi().setExternalStream(mHandle, stream->getHandle());
    } else {
        mStream = nullptr;
        engine.getDriverApi().setExternalStream(mHandle, backend::Handle<backend::HwStream>());
    }
}

static bool isColorRenderable(FEngine& engine, Texture::InternalFormat format) {
    switch (format) {
        case Texture::InternalFormat::DEPTH16:
        case Texture::InternalFormat::DEPTH24:
        case Texture::InternalFormat::DEPTH32F:
        case Texture::InternalFormat::DEPTH24_STENCIL8:
        case Texture::InternalFormat::DEPTH32F_STENCIL8:
            return false;
        default:
            return engine.getDriverApi().isRenderTargetFormatSupported(format);
    }
}

void FTexture::generateMipmaps(FEngine& engine) const noexcept {
    // The OpenGL spec for GenerateMipmap stipulates that it returns INVALID_OPERATION unless
    // the sized internal format is both color-renderable and texture-filterable.
    if (!ASSERT_POSTCONDITION_NON_FATAL(isColorRenderable(engine, mFormat),
            "Texture format must be color renderable")) {
        return;
    }
    if (mLevelCount == 1 || (mWidth == 1 && mHeight == 1)) {
        return;
    }

    if (engine.getDriverApi().canGenerateMipmaps()) {
         engine.getDriverApi().generateMipmaps(mHandle);
         return;
    }

    auto generateMipsForLayer = [this, &engine](uint16_t layer) {
        FEngine::DriverApi& driver = engine.getDriverApi();

        // Wrap miplevel 0 in a render target so that we can use it as a blit source.
        uint8_t level = 0;
        uint32_t srcw = mWidth;
        uint32_t srch = mHeight;
        backend::Handle<backend::HwRenderTarget> srcrth = driver.createRenderTarget(TargetBufferFlags::COLOR,
                srcw, srch, mSampleCount, { mHandle, level++, layer }, {}, {});

        // Perform a blit for all miplevels down to 1x1.
        backend::Handle<backend::HwRenderTarget> dstrth;
        do {
            uint32_t dstw = std::max(srcw >> 1u, 1u);
            uint32_t dsth = std::max(srch >> 1u, 1u);
            dstrth = driver.createRenderTarget(TargetBufferFlags::COLOR, dstw, dsth, mSampleCount,
                    { mHandle, level++, layer }, {}, {});
            driver.blit(TargetBufferFlags::COLOR,
                    dstrth, { 0, 0, dstw, dsth },
                    srcrth, { 0, 0, srcw, srch },
                    SamplerMagFilter::LINEAR);
            driver.destroyRenderTarget(srcrth);
            srcrth = dstrth;
            srcw = dstw;
            srch = dsth;
        } while ((srcw > 1 || srch > 1) && level < mLevelCount);
        driver.destroyRenderTarget(dstrth);
    };

    if (mTarget == Sampler::SAMPLER_2D) {
        generateMipsForLayer(0);
    } else if (mTarget == Sampler::SAMPLER_CUBEMAP) {
        for (uint16_t layer = 0; layer < 6; ++layer) {
            generateMipsForLayer(layer);
        }
    }
}

bool FTexture::isTextureFormatSupported(FEngine& engine, InternalFormat format) noexcept {
    return engine.getDriverApi().isTextureFormatSupported(format);
}

size_t FTexture::computeTextureDataSize(Texture::Format format, Texture::Type type,
        size_t stride, size_t height, size_t alignment) noexcept {
    return PixelBufferDescriptor::computeDataSize(format, type, stride, height, alignment);
}

size_t FTexture::getFormatSize(InternalFormat format) noexcept {
    using TextureFormat = InternalFormat;
    switch (format) {
        // 8-bits per element
        case TextureFormat::R8:
        case TextureFormat::R8_SNORM:
        case TextureFormat::R8UI:
        case TextureFormat::R8I:
        case TextureFormat::STENCIL8:
            return 1;

        // 16-bits per element
        case TextureFormat::R16F:
        case TextureFormat::R16UI:
        case TextureFormat::R16I:
        case TextureFormat::RG8:
        case TextureFormat::RG8_SNORM:
        case TextureFormat::RG8UI:
        case TextureFormat::RG8I:
        case TextureFormat::RGB565:
        case TextureFormat::RGB5_A1:
        case TextureFormat::RGBA4:
        case TextureFormat::DEPTH16:
            return 2;

        // 24-bits per element
        case TextureFormat::RGB8:
        case TextureFormat::SRGB8:
        case TextureFormat::RGB8_SNORM:
        case TextureFormat::RGB8UI:
        case TextureFormat::RGB8I:
        case TextureFormat::DEPTH24:
            return 3;

        // 32-bits per element
        case TextureFormat::R32F:
        case TextureFormat::R32UI:
        case TextureFormat::R32I:
        case TextureFormat::RG16F:
        case TextureFormat::RG16UI:
        case TextureFormat::RG16I:
        case TextureFormat::R11F_G11F_B10F:
        case TextureFormat::RGB9_E5:
        case TextureFormat::RGBA8:
        case TextureFormat::SRGB8_A8:
        case TextureFormat::RGBA8_SNORM:
        case TextureFormat::RGB10_A2:
        case TextureFormat::RGBA8UI:
        case TextureFormat::RGBA8I:
        case TextureFormat::DEPTH32F:
        case TextureFormat::DEPTH24_STENCIL8:
        case TextureFormat::DEPTH32F_STENCIL8:
            return 4;

        // 48-bits per element
        case TextureFormat::RGB16F:
        case TextureFormat::RGB16UI:
        case TextureFormat::RGB16I:
            return 6;

        // 64-bits per element
        case TextureFormat::RG32F:
        case TextureFormat::RG32UI:
        case TextureFormat::RG32I:
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA16UI:
        case TextureFormat::RGBA16I:
            return 8;

        // 96-bits per element
        case TextureFormat::RGB32F:
        case TextureFormat::RGB32UI:
        case TextureFormat::RGB32I:
            return 12;

        // 128-bits per element
        case TextureFormat::RGBA32F:
        case TextureFormat::RGBA32UI:
        case TextureFormat::RGBA32I:
            return 16;

        default:
            return 0;
    }
}

} // namespace details

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

using namespace details;


size_t Texture::getWidth(size_t level) const noexcept {
    return upcast(this)->getWidth(level);
}

size_t Texture::getHeight(size_t level) const noexcept {
    return upcast(this)->getHeight(level);
}

size_t Texture::getDepth(size_t level) const noexcept {
    return upcast(this)->getDepth(level);
}

size_t Texture::getLevels() const noexcept {
    return upcast(this)->getLevelCount();
}

Texture::Sampler Texture::getTarget() const noexcept {
    return upcast(this)->getTarget();
}

Texture::InternalFormat Texture::getFormat() const noexcept {
    return upcast(this)->getFormat();
}

void Texture::setImage(Engine& engine, size_t level,
        Texture::PixelBufferDescriptor&& buffer) const noexcept {
    upcast(this)->setImage(upcast(engine),
            level, 0, 0, uint32_t(getWidth(level)), uint32_t(getHeight(level)), std::move(buffer));
}

void Texture::setImage(Engine& engine,
        size_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& buffer) const noexcept {
    upcast(this)->setImage(upcast(engine),
            level, xoffset, yoffset, width, height, std::move(buffer));
}

void Texture::setImage(Engine& engine, size_t level,
        Texture::PixelBufferDescriptor&& buffer, const FaceOffsets& faceOffsets) const noexcept {
    upcast(this)->setImage(upcast(engine), level, std::move(buffer), faceOffsets);
}

void Texture::setExternalImage(Engine& engine, void* image) noexcept {
    upcast(this)->setExternalImage(upcast(engine), image);
}

void Texture::setExternalStream(Engine& engine, Stream* stream) noexcept {
    upcast(this)->setExternalStream(upcast(engine), upcast(stream));
}

void Texture::generateMipmaps(Engine& engine) const noexcept {
    upcast(this)->generateMipmaps(upcast(engine));
}

bool Texture::isTextureFormatSupported(Engine& engine, InternalFormat format) noexcept {
    return FTexture::isTextureFormatSupported(upcast(engine), format);
}

size_t Texture::computeTextureDataSize(Texture::Format format, Texture::Type type, size_t stride,
        size_t height, size_t alignment) noexcept {
    return FTexture::computeTextureDataSize(format, type, stride, height, alignment);
}

} // namespace filament
