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
#include <filament/Texture.h>

using namespace utils;

namespace filament {

using namespace backend;

struct Texture::BuilderDetails {
    intptr_t mImportedId = 0;
    uint32_t mWidth = 1;
    uint32_t mHeight = 1;
    uint32_t mDepth = 1;
    uint8_t mLevels = 1;
    Sampler mTarget = Sampler::SAMPLER_2D;
    InternalFormat mFormat = InternalFormat::RGBA8;
    Usage mUsage = Usage::DEFAULT;
    bool mTextureIsSwizzled = false;
    std::array<Swizzle, 4> mSwizzle = {
           Swizzle::CHANNEL_0, Swizzle::CHANNEL_1,
           Swizzle::CHANNEL_2, Swizzle::CHANNEL_3 };
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

Texture::Builder& Texture::Builder::import(intptr_t id) noexcept {
    assert_invariant(id); // imported id can't be zero
    mImpl->mImportedId = id;
    return *this;
}

Texture::Builder& Texture::Builder::swizzle(Swizzle r, Swizzle g, Swizzle b, Swizzle a) noexcept {
    mImpl->mTextureIsSwizzled = true;
    mImpl->mSwizzle = { r, g, b, a };
    return *this;
}

Texture* Texture::Builder::build(Engine& engine) {
    if (!ASSERT_POSTCONDITION_NON_FATAL(Texture::isTextureFormatSupported(engine, mImpl->mFormat),
            "Texture format %u not supported on this platform", mImpl->mFormat)) {
        return nullptr;
    }

    const bool sampleable = bool(mImpl->mUsage & TextureUsage::SAMPLEABLE);
    const bool swizzled = mImpl->mTextureIsSwizzled;
    const bool imported = mImpl->mImportedId;

    #if defined(__EMSCRIPTEN__)
    ASSERT_POSTCONDITION_NON_FATAL(!swizzled, "WebGL does not support texture swizzling.");
    #endif

    ASSERT_POSTCONDITION_NON_FATAL((swizzled && sampleable) || !swizzled,
            "Swizzled texture must be SAMPLEABLE");

    ASSERT_POSTCONDITION_NON_FATAL((imported && sampleable) || !imported,
            "Imported texture must be SAMPLEABLE");

    return upcast(engine).createTexture(*this);
}

// ------------------------------------------------------------------------------------------------

FTexture::FTexture(FEngine& engine, const Builder& builder) {
    mWidth  = static_cast<uint32_t>(builder->mWidth);
    mHeight = static_cast<uint32_t>(builder->mHeight);
    mFormat = builder->mFormat;
    mUsage = builder->mUsage;
    mTarget = builder->mTarget;
    mDepth  = static_cast<uint32_t>(builder->mDepth);
    mLevelCount = std::min(builder->mLevels, FTexture::maxLevelCount(mWidth, mHeight));

    FEngine::DriverApi& driver = engine.getDriverApi();
    if (UTILS_LIKELY(builder->mImportedId == 0)) {
        if (UTILS_LIKELY(!builder->mTextureIsSwizzled)) {
            mHandle = driver.createTexture(
                    mTarget, mLevelCount, mFormat, mSampleCount, mWidth, mHeight, mDepth, mUsage);
        } else {
            mHandle = driver.createTextureSwizzled(
                    mTarget, mLevelCount, mFormat, mSampleCount, mWidth, mHeight, mDepth, mUsage,
                    builder->mSwizzle[0], builder->mSwizzle[1], builder->mSwizzle[2],
                    builder->mSwizzle[3]);
        }
    } else {
        mHandle = driver.importTexture(builder->mImportedId,
                mTarget, mLevelCount, mFormat, mSampleCount, mWidth, mHeight, mDepth, mUsage);
    }
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
        Texture::PixelBufferDescriptor&& buffer) const {

    auto validateTarget = [](SamplerType sampler) -> bool {
        switch (sampler) {
            case SamplerType::SAMPLER_2D:
            case SamplerType::SAMPLER_EXTERNAL:
                return true;
            case SamplerType::SAMPLER_CUBEMAP:
            case SamplerType::SAMPLER_3D:
            case SamplerType::SAMPLER_2D_ARRAY:
                return false;
        }
    };

    if (!ASSERT_POSTCONDITION_NON_FATAL(buffer.type == PixelDataType::COMPRESSED ||
                         validatePixelFormatAndType(mFormat, buffer.format, buffer.type),
            "The combination of internal format=%u and {format=%u, type=%u} is not supported.",
            unsigned(mFormat), unsigned(buffer.format), unsigned(buffer.type))) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(!mStream, "setImage() called on a Stream texture.")) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(level < mLevelCount,
            "level=%u is >= to levelCount=%u.", unsigned(level), unsigned(mLevelCount))) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(validateTarget(mTarget),
            "Texture Sampler type (%u) not supported for this operation.", unsigned(mTarget))) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(buffer.buffer, "Data buffer is nullptr.")) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(mSampleCount <= 1,
            "Operation not supported with multisample (%u) texture.", unsigned(mSampleCount))) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(xoffset + width <= valueForLevel(level, mWidth),
            "xoffset (%u) + width (%u) > texture width (%u) at level (%u)",
            unsigned(xoffset), unsigned(width), unsigned(valueForLevel(level, mWidth)), unsigned(level))) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(yoffset + height <= valueForLevel(level, mHeight),
            "xoffset (%u) + width (%u) > texture width (%u) at level (%u)",
            unsigned(yoffset), unsigned(height), unsigned(valueForLevel(level, mHeight)), unsigned(level))) {
        return;
    }

    engine.getDriverApi().update2DImage(mHandle,
            uint8_t(level), xoffset, yoffset, width, height, std::move(buffer));
}

void FTexture::setImage(FEngine& engine,
        size_t level, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        Texture::PixelBufferDescriptor&& buffer) const {

    auto validateTarget = [](SamplerType sampler) -> bool {
        switch (sampler) {
            case SamplerType::SAMPLER_3D:
            case SamplerType::SAMPLER_2D_ARRAY:
                return true;
            case SamplerType::SAMPLER_2D:
            case SamplerType::SAMPLER_EXTERNAL:
            case SamplerType::SAMPLER_CUBEMAP:
                return false;
        }
    };

    if (!ASSERT_POSTCONDITION_NON_FATAL(buffer.type == PixelDataType::COMPRESSED ||
                         validatePixelFormatAndType(mFormat, buffer.format, buffer.type),
            "The combination of internal format=%u and {format=%u, type=%u} is not supported.",
            unsigned(mFormat), unsigned(buffer.format), unsigned(buffer.type))) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(!mStream, "setImage() called on a Stream texture.")) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(level < mLevelCount,
            "level=%u is >= to levelCount=%u.", unsigned(level), unsigned(mLevelCount))) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(validateTarget(mTarget),
            "Texture Sampler type (%u) not supported for this operation.", unsigned(mTarget))) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(mSampleCount <= 1,
            "Operation not supported with multisample (%u) texture.", unsigned(mSampleCount))) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(xoffset + width <= valueForLevel(level, mWidth),
            "xoffset (%u) + width (%u) > texture width (%u) at level (%u)",
            unsigned(xoffset), unsigned(width), unsigned(valueForLevel(level, mWidth)), unsigned(level))) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(yoffset + height <= valueForLevel(level, mHeight),
            "yoffset (%u) + height (%u) > texture height (%u) at level (%u)",
            unsigned(yoffset), unsigned(height), unsigned(valueForLevel(level, mHeight)), unsigned(level))) {
        return;
    }

    // effective level is just how we compute the index/depth based on whether we're an array or a 3D texture
    const uint8_t effectiveLevel = mTarget == SamplerType::SAMPLER_3D ? level : 0;
    if (!ASSERT_POSTCONDITION_NON_FATAL(zoffset + depth <= valueForLevel(effectiveLevel, mDepth),
            "zoffset (%u) + depth (%u) > texture depth (%u) at level (%u)",
            unsigned(zoffset), unsigned(depth), unsigned(valueForLevel(effectiveLevel, mDepth)), unsigned(level))) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(buffer.buffer, "Data buffer is nullptr.")) {
        return;
    }

    engine.getDriverApi().update3DImage(mHandle,
            uint8_t(level), xoffset, yoffset, zoffset, width, height, depth, std::move(buffer));
}

void FTexture::setImage(FEngine& engine, size_t level,
        Texture::PixelBufferDescriptor&& buffer, const FaceOffsets& faceOffsets) const {

    auto validateTarget = [](SamplerType sampler) -> bool {
        switch (sampler) {
            case SamplerType::SAMPLER_CUBEMAP:
                return true;
            case SamplerType::SAMPLER_3D:
            case SamplerType::SAMPLER_2D_ARRAY:
            case SamplerType::SAMPLER_2D:
            case SamplerType::SAMPLER_EXTERNAL:
                return false;
        }
    };

    if (!ASSERT_POSTCONDITION_NON_FATAL(buffer.type == PixelDataType::COMPRESSED ||
                        validatePixelFormatAndType(mFormat, buffer.format, buffer.type),
            "The combination of internal format=%u and {format=%u, type=%u} is not supported.",
            unsigned(mFormat), unsigned(buffer.format), unsigned(buffer.type))) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(!mStream, "setImage() called on a Stream texture.")) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(level < mLevelCount,
            "level=%u is >= to levelCount=%u.", unsigned(level), unsigned(mLevelCount))) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(validateTarget(mTarget),
            "Texture Sampler type (%u) not supported for this operation.", unsigned(mTarget))) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(buffer.buffer, "Data buffer is nullptr.")) {
        return;
    }

    engine.getDriverApi().updateCubeImage(mHandle, uint8_t(level),
            std::move(buffer), faceOffsets);
}

void FTexture::setExternalImage(FEngine& engine, void* image) noexcept {
    if (mTarget == Sampler::SAMPLER_EXTERNAL) {
        // The call to setupExternalImage is synchronous, and allows the driver to take ownership of
        // the external image on this thread, if necessary.
        engine.getDriverApi().setupExternalImage(image);
        engine.getDriverApi().setExternalImage(mHandle, image);
    }
}

void FTexture::setExternalImage(FEngine& engine, void* image, size_t plane) noexcept {
    if (mTarget == Sampler::SAMPLER_EXTERNAL) {
        // The call to setupExternalImage is synchronous, and allows the driver to take ownership of
        // the external image on this thread, if necessary.
        engine.getDriverApi().setupExternalImage(image);
        engine.getDriverApi().setExternalImagePlane(mHandle, image, plane);
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
    if (!ASSERT_POSTCONDITION_NON_FATAL(mTarget != SamplerType::SAMPLER_EXTERNAL,
            "External Textures are not mipmappable.")) {
        return;
    }

    const bool formatMipmappable = engine.getDriverApi().isTextureFormatMipmappable(mFormat);
    if (!ASSERT_POSTCONDITION_NON_FATAL(formatMipmappable,
            "Texture format %u is not mipmappable.", (unsigned)mFormat)) {
        return;
    }

    if (mLevelCount < 2 || (mWidth == 1 && mHeight == 1)) {
        return;
    }

    if (engine.getDriverApi().canGenerateMipmaps()) {
        engine.getDriverApi().generateMipmaps(mHandle);
        return;
    }

    auto generateMipsForLayer = [this, &engine](TargetBufferInfo proto) {
        FEngine::DriverApi& driver = engine.getDriverApi();

        // Wrap miplevel 0 in a render target so that we can use it as a blit source.
        uint8_t level = 0;
        uint32_t srcw = mWidth;
        uint32_t srch = mHeight;
        proto.handle = mHandle;
        proto.level = level++;
        backend::Handle<backend::HwRenderTarget> srcrth = driver.createRenderTarget(
                TargetBufferFlags::COLOR, srcw, srch, mSampleCount, proto, {}, {});

        // Perform a blit for all miplevels down to 1x1.
        backend::Handle<backend::HwRenderTarget> dstrth;
        do {
            uint32_t dstw = std::max(srcw >> 1u, 1u);
            uint32_t dsth = std::max(srch >> 1u, 1u);
            proto.level = level++;
            dstrth = driver.createRenderTarget(
                    TargetBufferFlags::COLOR, dstw, dsth, mSampleCount, proto, {}, {});
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

    switch (mTarget) {
        case SamplerType::SAMPLER_2D:
            generateMipsForLayer(TargetBufferInfo{ 0 });
            break;
        case SamplerType::SAMPLER_2D_ARRAY:
            for (uint16_t layer = 0, c = mDepth; layer < c; ++layer) {
                generateMipsForLayer(TargetBufferInfo{ layer });
            }
            break;
        case SamplerType::SAMPLER_CUBEMAP:
            for (uint8_t face = 0; face < 6; ++face) {
                generateMipsForLayer(TargetBufferInfo{ TextureCubemapFace(face) });
            }
            break;
        case SamplerType::SAMPLER_EXTERNAL:
            // not mipmapable
            break;
        case SamplerType::SAMPLER_3D:
            // TODO: handle SAMPLER_3D -- this can't be done with a 2D blit, this would require
            //       a fragment shader
            slog.w << "Texture::generateMipmap does not support SAMPLER_3D yet on this h/w." << io::endl;
            break;
    }
}

bool FTexture::isTextureFormatSupported(FEngine& engine, InternalFormat format) noexcept {
    return engine.getDriverApi().isTextureFormatSupported(format);
}

bool FTexture::isTextureSwizzleSupported(FEngine& engine) noexcept {
    return engine.getDriverApi().isTextureSwizzleSupported();
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
    assert_invariant(bytesPerPixel);

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

bool FTexture::validatePixelFormatAndType(TextureFormat internalFormat,
        PixelDataFormat format, PixelDataType type) noexcept {

    switch (internalFormat) {
        case TextureFormat::R8:
        case TextureFormat::R8_SNORM:
        case TextureFormat::R16F:
        case TextureFormat::R32F:
            if (format != PixelDataFormat::R) {
                return false;
            }
            break;

        case TextureFormat::R8UI:
        case TextureFormat::R8I:
        case TextureFormat::R16UI:
        case TextureFormat::R16I:
        case TextureFormat::R32UI:
        case TextureFormat::R32I:
            if (format != PixelDataFormat::R_INTEGER) {
                return false;
            }
            break;

        case TextureFormat::RG8:
        case TextureFormat::RG8_SNORM:
        case TextureFormat::RG16F:
        case TextureFormat::RG32F:
            if (format != PixelDataFormat::RG) {
                return false;
            }
            break;

        case TextureFormat::RG8UI:
        case TextureFormat::RG8I:
        case TextureFormat::RG16UI:
        case TextureFormat::RG16I:
        case TextureFormat::RG32UI:
        case TextureFormat::RG32I:
            if (format != PixelDataFormat::RG_INTEGER) {
                return false;
            }
            break;

        case TextureFormat::RGB565:
        case TextureFormat::RGB9_E5:
        case TextureFormat::RGB5_A1:
        case TextureFormat::RGBA4:
        case TextureFormat::RGB8:
        case TextureFormat::SRGB8:
        case TextureFormat::RGB8_SNORM:
        case TextureFormat::R11F_G11F_B10F:
        case TextureFormat::RGB16F:
        case TextureFormat::RGB32F:
            if (format != PixelDataFormat::RGB) {
                return false;
            }
            break;

        case TextureFormat::RGB8UI:
        case TextureFormat::RGB8I:
        case TextureFormat::RGB16UI:
        case TextureFormat::RGB16I:
        case TextureFormat::RGB32UI:
        case TextureFormat::RGB32I:
            if (format != PixelDataFormat::RGB_INTEGER) {
                return false;
            }
            break;

        case TextureFormat::RGBA8:
        case TextureFormat::SRGB8_A8:
        case TextureFormat::RGBA8_SNORM:
        case TextureFormat::RGB10_A2:
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA32F:
            if (format != PixelDataFormat::RGBA) {
                return false;
            }
            break;

        case TextureFormat::RGBA8UI:
        case TextureFormat::RGBA8I:
        case TextureFormat::RGBA16UI:
        case TextureFormat::RGBA16I:
        case TextureFormat::RGBA32UI:
        case TextureFormat::RGBA32I:
            if (format != PixelDataFormat::RGBA_INTEGER) {
                return false;
            }
            break;

        case TextureFormat::STENCIL8:
            // there is no pixel data type that can be used for this format
            return false;

        case TextureFormat::DEPTH16:
        case TextureFormat::DEPTH24:
        case TextureFormat::DEPTH32F:
            if (format != PixelDataFormat::DEPTH_COMPONENT) {
                return false;
            }
            break;

        case TextureFormat::DEPTH24_STENCIL8:
        case TextureFormat::DEPTH32F_STENCIL8:
            if (format != PixelDataFormat::DEPTH_STENCIL) {
                return false;
            }
            break;

        case TextureFormat::UNUSED:
        case TextureFormat::EAC_R11:
        case TextureFormat::EAC_R11_SIGNED:
        case TextureFormat::EAC_RG11:
        case TextureFormat::EAC_RG11_SIGNED:
        case TextureFormat::ETC2_RGB8:
        case TextureFormat::ETC2_SRGB8:
        case TextureFormat::ETC2_RGB8_A1:
        case TextureFormat::ETC2_SRGB8_A1:
        case TextureFormat::ETC2_EAC_RGBA8:
        case TextureFormat::ETC2_EAC_SRGBA8:
        case TextureFormat::DXT1_RGB:
        case TextureFormat::DXT1_RGBA:
        case TextureFormat::DXT3_RGBA:
        case TextureFormat::DXT5_RGBA:
        case TextureFormat::DXT1_SRGB:
        case TextureFormat::DXT1_SRGBA:
        case TextureFormat::DXT3_SRGBA:
        case TextureFormat::DXT5_SRGBA:
        case TextureFormat::RGBA_ASTC_4x4:
        case TextureFormat::RGBA_ASTC_5x4:
        case TextureFormat::RGBA_ASTC_5x5:
        case TextureFormat::RGBA_ASTC_6x5:
        case TextureFormat::RGBA_ASTC_6x6:
        case TextureFormat::RGBA_ASTC_8x5:
        case TextureFormat::RGBA_ASTC_8x6:
        case TextureFormat::RGBA_ASTC_8x8:
        case TextureFormat::RGBA_ASTC_10x5:
        case TextureFormat::RGBA_ASTC_10x6:
        case TextureFormat::RGBA_ASTC_10x8:
        case TextureFormat::RGBA_ASTC_10x10:
        case TextureFormat::RGBA_ASTC_12x10:
        case TextureFormat::RGBA_ASTC_12x12:
        case TextureFormat::SRGB8_ALPHA8_ASTC_4x4:
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x4:
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x8:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x8:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x10:
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x10:
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x12:
            return false;
    }

    switch (internalFormat) {
        case TextureFormat::R8:
        case TextureFormat::R8UI:
        case TextureFormat::RG8:
        case TextureFormat::RG8UI:
        case TextureFormat::RGB8:
        case TextureFormat::SRGB8:
        case TextureFormat::RGB8UI:
        case TextureFormat::RGBA8:
        case TextureFormat::SRGB8_A8:
        case TextureFormat::RGBA8UI:
            if (type != PixelDataType::UBYTE) {
                return false;
            }
            break;

        case TextureFormat::R8_SNORM:
        case TextureFormat::R8I:
        case TextureFormat::RG8_SNORM:
        case TextureFormat::RG8I:
        case TextureFormat::RGB8_SNORM:
        case TextureFormat::RGB8I:
        case TextureFormat::RGBA8_SNORM:
        case TextureFormat::RGBA8I:
            if (type != PixelDataType::BYTE) {
                return false;
            }
            break;

        case TextureFormat::R16F:
        case TextureFormat::RG16F:
        case TextureFormat::RGB16F:
        case TextureFormat::RGBA16F:
            if (type != PixelDataType::FLOAT && type != PixelDataType::HALF) {
                return false;
            }
            break;

        case TextureFormat::R32F:
        case TextureFormat::RG32F:
        case TextureFormat::RGB32F:
        case TextureFormat::RGBA32F:
        case TextureFormat::DEPTH32F:
            if (type != PixelDataType::FLOAT) {
                return false;
            }
            break;

        case TextureFormat::R16UI:
        case TextureFormat::RG16UI:
        case TextureFormat::RGB16UI:
        case TextureFormat::RGBA16UI:
            if (type != PixelDataType::USHORT) {
                return false;
            }
            break;

        case TextureFormat::R16I:
        case TextureFormat::RG16I:
        case TextureFormat::RGB16I:
        case TextureFormat::RGBA16I:
            if (type != PixelDataType::SHORT) {
                return false;
            }
            break;


        case TextureFormat::R32UI:
        case TextureFormat::RG32UI:
        case TextureFormat::RGB32UI:
        case TextureFormat::RGBA32UI:
            if (type != PixelDataType::UINT) {
                return false;
            }
            break;

        case TextureFormat::R32I:
        case TextureFormat::RG32I:
        case TextureFormat::RGB32I:
        case TextureFormat::RGBA32I:
            if (type != PixelDataType::INT) {
                return false;
            }
            break;

        case TextureFormat::RGB565:
            if (type != PixelDataType::UBYTE && type != PixelDataType::USHORT_565) {
                return false;
            }
            break;


        case TextureFormat::RGB9_E5:
            // TODO: we're missing UINT_5_9_9_9_REV
            if (type != PixelDataType::FLOAT && type != PixelDataType::HALF) {
                return false;
            }
            break;

        case TextureFormat::RGB5_A1:
            // TODO: we're missing USHORT_5_5_5_1
            if (type != PixelDataType::UBYTE && type != PixelDataType::UINT_2_10_10_10_REV) {
                return false;
            }
            break;

        case TextureFormat::RGBA4:
            // TODO: we're missing USHORT_4_4_4_4
            if (type != PixelDataType::UBYTE) {
                return false;
            }
            break;

        case TextureFormat::R11F_G11F_B10F:
            if (type != PixelDataType::FLOAT && type != PixelDataType::HALF
                && type != PixelDataType::UINT_10F_11F_11F_REV) {
                return false;
            }
            break;

        case TextureFormat::RGB10_A2:
            if (type != PixelDataType::UINT_2_10_10_10_REV) {
                return false;
            }
            break;

        case TextureFormat::STENCIL8:
            // there is no pixel data type that can be used for this format
            return false;

        case TextureFormat::DEPTH16:
            if (type != PixelDataType::UINT && type != PixelDataType::USHORT) {
                return false;
            }
            break;

        case TextureFormat::DEPTH24:
            if (type != PixelDataType::UINT) {
                return false;
            }
            break;

        case TextureFormat::DEPTH24_STENCIL8:
            // TODO: we're missing UINT_24_8
            return false;

        case TextureFormat::DEPTH32F_STENCIL8:
            // TODO: we're missing FLOAT_UINT_24_8_REV
            return false;

        case TextureFormat::UNUSED:
        case TextureFormat::EAC_R11:
        case TextureFormat::EAC_R11_SIGNED:
        case TextureFormat::EAC_RG11:
        case TextureFormat::EAC_RG11_SIGNED:
        case TextureFormat::ETC2_RGB8:
        case TextureFormat::ETC2_SRGB8:
        case TextureFormat::ETC2_RGB8_A1:
        case TextureFormat::ETC2_SRGB8_A1:
        case TextureFormat::ETC2_EAC_RGBA8:
        case TextureFormat::ETC2_EAC_SRGBA8:
        case TextureFormat::DXT1_RGB:
        case TextureFormat::DXT1_RGBA:
        case TextureFormat::DXT3_RGBA:
        case TextureFormat::DXT5_RGBA:
        case TextureFormat::DXT1_SRGB:
        case TextureFormat::DXT1_SRGBA:
        case TextureFormat::DXT3_SRGBA:
        case TextureFormat::DXT5_SRGBA:
        case TextureFormat::RGBA_ASTC_4x4:
        case TextureFormat::RGBA_ASTC_5x4:
        case TextureFormat::RGBA_ASTC_5x5:
        case TextureFormat::RGBA_ASTC_6x5:
        case TextureFormat::RGBA_ASTC_6x6:
        case TextureFormat::RGBA_ASTC_8x5:
        case TextureFormat::RGBA_ASTC_8x6:
        case TextureFormat::RGBA_ASTC_8x8:
        case TextureFormat::RGBA_ASTC_10x5:
        case TextureFormat::RGBA_ASTC_10x6:
        case TextureFormat::RGBA_ASTC_10x8:
        case TextureFormat::RGBA_ASTC_10x10:
        case TextureFormat::RGBA_ASTC_12x10:
        case TextureFormat::RGBA_ASTC_12x12:
        case TextureFormat::SRGB8_ALPHA8_ASTC_4x4:
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x4:
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x8:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x8:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x10:
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x10:
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x12:
            return false;
    }

    return true;
}

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

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
        Texture::PixelBufferDescriptor&& buffer) const {
    upcast(this)->setImage(upcast(engine),
            level, 0, 0,
            uint32_t(getWidth(level)), uint32_t(getHeight(level)), std::move(buffer));
}

void Texture::setImage(Engine& engine,
        size_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& buffer) const {
    upcast(this)->setImage(upcast(engine),
            level, xoffset, yoffset, width, height, std::move(buffer));
}

void Texture::setImage(Engine& engine, size_t level,
        uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& buffer) const {
    upcast(this)->setImage(upcast(engine),
            level, xoffset, yoffset, zoffset, width, height, depth, std::move(buffer));
}

void Texture::setImage(Engine& engine, size_t level,
        Texture::PixelBufferDescriptor&& buffer, const FaceOffsets& faceOffsets) const {
    upcast(this)->setImage(upcast(engine), level, std::move(buffer), faceOffsets);
}

void Texture::setExternalImage(Engine& engine, void* image) noexcept {
    upcast(this)->setExternalImage(upcast(engine), image);
}

void Texture::setExternalImage(Engine& engine, void* image, size_t plane) noexcept {
    upcast(this)->setExternalImage(upcast(engine), image, plane);
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

bool Texture::isTextureSwizzleSupported(Engine& engine) noexcept {
    return FTexture::isTextureSwizzleSupported(upcast(engine));
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
