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

#include "private/backend/BackendUtils.h"

#include "FilamentAPI-impl.h"

#include <ibl/Cubemap.h>
#include <ibl/CubemapIBL.h>
#include <ibl/CubemapUtils.h>
#include <ibl/Image.h>

#include <utils/Panic.h>

using namespace utils;

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
    FEngine::assertValid(engine, __PRETTY_FUNCTION__);
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

void FTexture::generateMipmaps(FEngine& engine) const noexcept {
    const bool formatMipmappable = engine.getDriverApi().isTextureFormatMipmappable(mFormat);
    if (!ASSERT_POSTCONDITION_NON_FATAL(formatMipmappable, "Texture format is not mipmappable.")) {
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
    return backend::getFormatSize(format);
}


// this is a hack to be able to create a std::function<> with a non-copyable closure
template<class F>
auto make_copyable_function(F&& f) {
    using dF = std::decay_t<F>;
    auto spf = std::make_shared<dF>(std::forward<F>(f));
    return [spf](auto&& ... args) -> decltype(auto) {
        return (*spf)(decltype(args)(args)...);
    };
}

void FTexture::generatePrefilterMipmap(FEngine& engine,
        PixelBufferDescriptor&& buffer, const FaceOffsets& faceOffsets,
        PrefilterOptions const* options) {
    using namespace ibl;
    using namespace backend;
    using namespace math;

    const size_t size = getWidth();
    const size_t stride = buffer.stride ? buffer.stride : size;

    /* validate input data */

    if (!ASSERT_PRECONDITION_NON_FATAL(buffer.format == PixelDataFormat::RGB ||
                                       buffer.format == PixelDataFormat::RGBA,
            "input data format must be RGB or RGBA")) {
        return;
    }

    if (!ASSERT_PRECONDITION_NON_FATAL(
            buffer.type == PixelDataType::FLOAT ||
            buffer.type == PixelDataType::HALF ||
            buffer.type == PixelDataType::UINT_10F_11F_11F_REV,
            "input data type must be FLOAT, HALF or UINT_10F_11F_11F_REV")) {
        return;
    }

    /* validate texture */

    if (!ASSERT_PRECONDITION_NON_FATAL(!(size & (size-1)),
            "input data cubemap dimensions must be a power-of-two")) {
        return;
    }

    if (!ASSERT_PRECONDITION_NON_FATAL(!isCompressed(),
            "reflections texture cannot be compressed")) {
        return;
    }


    PrefilterOptions defaultOptions;
    options = options ? options : &defaultOptions;

    JobSystem& js = engine.getJobSystem();
    FEngine::DriverApi& driver = engine.getDriverApi();

    auto generateMipmaps = [](JobSystem& js,
            std::vector<Cubemap>& levels, std::vector<Image>& images) {
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
    assert(bytesPerPixel);

    Image temp;
    Cubemap cml = CubemapUtils::create(temp, size);
    for (size_t j = 0; j < 6; j++) {
        Cubemap::Face face = (Cubemap::Face)j;
        Image const& image = cml.getImageForFace(face);
        for (size_t y = 0; y < size; y++) {
            Cubemap::Texel* out = (Cubemap::Texel*)image.getPixelRef(0, y);
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
                    using fp10 = math::fp<0, 5, 5>;
                    using fp11 = math::fp<0, 5, 6>;
                    fp11 r{ uint16_t( *src         & 0x7FFu) };
                    fp11 g{ uint16_t((*src >> 11u) & 0x7FFu) };
                    fp10 b{ uint16_t((*src >> 22u) & 0x3FFu) };
                    Cubemap::Texel texel{ fp11::tof(r), fp11::tof(g), fp10::tof(b) };
                    Cubemap::writeAt(out, texel);
                }
            }
        }
    }

    /*
     * Create the mipmap chain
     */

    std::vector<Image> images;
    std::vector<Cubemap> levels;
    images.reserve(getLevels());
    levels.reserve(getLevels());

    images.push_back(std::move(temp));
    levels.push_back(std::move(cml));

    const float3 mirror = options->mirror ? float3{ -1, 1, 1 } : float3{ 1, 1, 1 };

    // make the cubemap seamless
    levels[0].makeSeamless();

    // Now generate all the mipmap levels
    generateMipmaps(js, levels, images);

    // Finally generate each pre-filtered mipmap level
    const size_t baseExp = ctz(size);
    size_t numSamples = options->sampleCount;
    const size_t numLevels = baseExp + 1;
    for (ssize_t i = baseExp; i >= 0; --i) {
        const size_t dim = 1U << i;
        const size_t level = baseExp - i;
        const float lod = saturate(level / (numLevels - 1.0f));
        const float linearRoughness = lod * lod;

        Image image;
        Cubemap dst = CubemapUtils::create(image, dim);
        CubemapIBL::roughnessFilter(js, dst, levels, linearRoughness, numSamples, mirror, true);

        Texture::PixelBufferDescriptor pbd(image.getData(), image.getSize(),
                Texture::PixelBufferDescriptor::PixelDataFormat::RGB,
                Texture::PixelBufferDescriptor::PixelDataType::FLOAT, 1, 0, 0, image.getStride());

        uintptr_t base = uintptr_t(image.getData());
        backend::FaceOffsets offsets{};
        for (size_t j = 0; j < 6; j++) {
            Image const& faceImage = dst.getImageForFace((Cubemap::Face)j);
            offsets[j] = uintptr_t(faceImage.getData()) - base;
        }
        // upload all 6 faces into the texture
        driver.updateCubeImage(mHandle, level, std::move(pbd), offsets);

        // enqueue a commands that holds the image data until it's executed
        driver.queueCommand(make_copyable_function([data = image.detach()]() {}));
    }

    // no need to call the user callback because buffer is a reference and it'll be destroyed
    // by the caller (without being move()d here).
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

void Texture::generatePrefilterMipmap(Engine& engine, Texture::PixelBufferDescriptor&& buffer,
        const Texture::FaceOffsets& faceOffsets, PrefilterOptions const* options) {
    upcast(this)->generatePrefilterMipmap(upcast(engine), std::move(buffer), faceOffsets, options);
}

} // namespace filament
