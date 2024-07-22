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

#include <filament/Texture.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <ibl/Cubemap.h>
#include <ibl/CubemapIBL.h>
#include <ibl/CubemapUtils.h>
#include <ibl/Image.h>

#include <math/half.h>
#include <math/scalar.h>
#include <math/vec3.h>

#include <utils/Allocator.h>
#include <utils/algorithm.h>
#include <utils/BitmaskEnum.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Panic.h>

#include <algorithm>
#include <array>
#include <type_traits>
#include <utility>

#include <stddef.h>
#include <stdint.h>

using namespace utils;

namespace filament {

using namespace backend;
using namespace math;

// this is a hack to be able to create a std::function<> with a non-copyable closure
template<class F>
auto make_copyable_function(F&& f) {
    using dF = std::decay_t<F>;
    auto spf = std::make_shared<dF>(std::forward<F>(f));
    return [spf](auto&& ... args) -> decltype(auto) {
        return (*spf)(decltype(args)(args)...);
    };
}

struct Texture::BuilderDetails {
    intptr_t mImportedId = 0;
    uint32_t mWidth = 1;
    uint32_t mHeight = 1;
    uint32_t mDepth = 1;
    uint8_t mLevels = 1;
    Sampler mTarget = Sampler::SAMPLER_2D;
    InternalFormat mFormat = InternalFormat::RGBA8;
    Usage mUsage = Usage::NONE;
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
    FILAMENT_CHECK_PRECONDITION(Texture::isTextureFormatSupported(engine, mImpl->mFormat))
            << "Texture format " << uint16_t(mImpl->mFormat) << " not supported on this platform";

    const bool isProtectedTexturesSupported =
            downcast(engine).getDriverApi().isProtectedTexturesSupported();
    const bool useProtectedMemory = bool(mImpl->mUsage & TextureUsage::PROTECTED);

    FILAMENT_CHECK_PRECONDITION(
            (isProtectedTexturesSupported && useProtectedMemory) || !useProtectedMemory)
            << "Texture is PROTECTED but protected textures are not supported";

    uint8_t maxLevelCount;
    switch (mImpl->mTarget) {
        case SamplerType::SAMPLER_2D:
        case SamplerType::SAMPLER_2D_ARRAY:
        case SamplerType::SAMPLER_CUBEMAP:
        case SamplerType::SAMPLER_EXTERNAL:
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            maxLevelCount = FTexture::maxLevelCount(mImpl->mWidth, mImpl->mHeight);
            break;
        case SamplerType::SAMPLER_3D:
            maxLevelCount = FTexture::maxLevelCount(std::max(
                    { mImpl->mWidth, mImpl->mHeight, mImpl->mDepth }));
            break;
    }
    mImpl->mLevels = std::min(mImpl->mLevels, maxLevelCount);

    if (mImpl->mUsage == TextureUsage::NONE) {
        mImpl->mUsage = TextureUsage::DEFAULT;
        if (mImpl->mLevels > 1 &&
            (mImpl->mWidth > 1 || mImpl->mHeight > 1) &&
            mImpl->mTarget != SamplerType::SAMPLER_EXTERNAL) {
            const bool formatMipmappable =
                    downcast(engine).getDriverApi().isTextureFormatMipmappable(mImpl->mFormat);
            if (formatMipmappable) {
                // by default mipmappable textures have the BLIT usage bits set
                mImpl->mUsage |= TextureUsage::BLIT_SRC | TextureUsage::BLIT_DST;
            }
        }
    }

    const bool sampleable = bool(mImpl->mUsage & TextureUsage::SAMPLEABLE);
    const bool swizzled = mImpl->mTextureIsSwizzled;
    const bool imported = mImpl->mImportedId;

    #if defined(__EMSCRIPTEN__)
    FILAMENT_CHECK_PRECONDITION(!swizzled) << "WebGL does not support texture swizzling.";
    #endif

    auto validateSamplerType = [&engine = downcast(engine)](SamplerType sampler) -> bool {
        switch (sampler) {
            case SamplerType::SAMPLER_2D:
            case SamplerType::SAMPLER_CUBEMAP:
            case SamplerType::SAMPLER_EXTERNAL:
                return true;
            case SamplerType::SAMPLER_3D:
            case SamplerType::SAMPLER_2D_ARRAY:
                return engine.hasFeatureLevel(FeatureLevel::FEATURE_LEVEL_1);
            case SamplerType::SAMPLER_CUBEMAP_ARRAY:
                return engine.hasFeatureLevel(FeatureLevel::FEATURE_LEVEL_2);
        }
    };

    FILAMENT_CHECK_PRECONDITION(validateSamplerType(mImpl->mTarget))
            << "SamplerType " << uint8_t(mImpl->mTarget) << " not support at feature level "
            << uint8_t(engine.getActiveFeatureLevel());

    FILAMENT_CHECK_PRECONDITION((swizzled && sampleable) || !swizzled)
            << "Swizzled texture must be SAMPLEABLE";

    FILAMENT_CHECK_PRECONDITION((imported && sampleable) || !imported)
            << "Imported texture must be SAMPLEABLE";

    return downcast(engine).createTexture(*this);
}

// ------------------------------------------------------------------------------------------------

FTexture::FTexture(FEngine& engine, const Builder& builder) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    mDriver = &driver; // this is unfortunately needed for getHwHandleForSampling()
    mWidth  = static_cast<uint32_t>(builder->mWidth);
    mHeight = static_cast<uint32_t>(builder->mHeight);
    mDepth  = static_cast<uint32_t>(builder->mDepth);
    mFormat = builder->mFormat;
    mUsage = builder->mUsage;
    mTarget = builder->mTarget;
    mLevelCount = builder->mLevels;

    if (mTarget == SamplerType::SAMPLER_EXTERNAL) {
        // mHandle and mHandleForSampling will be created in setExternalImage()
        return;
    }

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
    mHandleForSampling = mHandle;
}

// frees driver resources, object becomes invalid
void FTexture::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    if (mHandleForSampling != mHandle) {
        driver.destroyTexture(mHandleForSampling);
    }
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

void FTexture::setImage(FEngine& engine, size_t level,
        uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        FTexture::PixelBufferDescriptor&& p) const {

    if (UTILS_UNLIKELY(!engine.hasFeatureLevel(FeatureLevel::FEATURE_LEVEL_1))) {
        FILAMENT_CHECK_PRECONDITION(p.stride == 0 || p.stride == width)
                << "PixelBufferDescriptor stride must be 0 (or width) at FEATURE_LEVEL_0";
    }

    // this should have been validated already
    assert_invariant(isTextureFormatSupported(engine, mFormat));

    FILAMENT_CHECK_PRECONDITION(p.type == PixelDataType::COMPRESSED ||
            validatePixelFormatAndType(mFormat, p.format, p.type))
            << "The combination of internal format=" << unsigned(mFormat)
            << " and {format=" << unsigned(p.format) << ", type=" << unsigned(p.type)
            << "} is not supported.";

    FILAMENT_CHECK_PRECONDITION(!mStream) << "setImage() called on a Stream texture.";

    FILAMENT_CHECK_PRECONDITION(level < mLevelCount)
            << "level=" << unsigned(level) << " is >= to levelCount=" << unsigned(mLevelCount)
            << ".";

    FILAMENT_CHECK_PRECONDITION(mTarget != SamplerType::SAMPLER_EXTERNAL)
            << "Texture SamplerType::SAMPLER_EXTERNAL not supported for this operation.";

    FILAMENT_CHECK_PRECONDITION(mSampleCount <= 1) << "Operation not supported with multisample ("
                                                   << unsigned(mSampleCount) << ") texture.";

    FILAMENT_CHECK_PRECONDITION(xoffset + width <= valueForLevel(level, mWidth))
            << "xoffset (" << unsigned(xoffset) << ") + width (" << unsigned(width)
            << ") > texture width (" << valueForLevel(level, mWidth) << ") at level ("
            << unsigned(level) << ")";

    FILAMENT_CHECK_PRECONDITION(yoffset + height <= valueForLevel(level, mHeight))
            << "yoffset (" << unsigned(yoffset) << ") + height (" << unsigned(height)
            << ") > texture height (" << valueForLevel(level, mHeight) << ") at level ("
            << unsigned(level) << ")";

    FILAMENT_CHECK_PRECONDITION(p.buffer) << "Data buffer is nullptr.";

    uint32_t effectiveTextureDepthOrLayers;
    switch (mTarget) {
        case SamplerType::SAMPLER_EXTERNAL:
            // can't happen by construction, fallthrough...
        case SamplerType::SAMPLER_2D:
            assert_invariant(mDepth == 1);
            effectiveTextureDepthOrLayers = 1;
            break;
        case SamplerType::SAMPLER_3D:
            effectiveTextureDepthOrLayers = valueForLevel(level, mDepth);
            break;
        case SamplerType::SAMPLER_2D_ARRAY:
            effectiveTextureDepthOrLayers = mDepth;
            break;
        case SamplerType::SAMPLER_CUBEMAP:
            effectiveTextureDepthOrLayers = 6;
            break;
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            effectiveTextureDepthOrLayers = mDepth * 6;
            break;
    }

    FILAMENT_CHECK_PRECONDITION(zoffset + depth <= effectiveTextureDepthOrLayers)
            << "zoffset (" << unsigned(zoffset) << ") + depth (" << unsigned(depth)
            << ") > texture depth (" << effectiveTextureDepthOrLayers << ") at level ("
            << unsigned(level) << ")";

    using PBD = PixelBufferDescriptor;
    size_t const stride = p.stride ? p.stride : width;
    size_t const bpp = PBD::computeDataSize(p.format, p.type, 1, 1, 1);
    size_t const bpr = PBD::computeDataSize(p.format, p.type, stride, 1, p.alignment);
    size_t const bpl = bpr * height; // TODO: PBD should have a "layer stride"
    // TODO: PBD should have a p.depth (# layers to skip)
    FILAMENT_CHECK_PRECONDITION(bpp * p.left + bpr * p.top + bpl * (0 + depth) <= p.size)
            << "buffer overflow: (size=" << size_t(p.size) << ", stride=" << size_t(p.stride)
            << ", left=" << unsigned(p.left) << ", top=" << unsigned(p.top)
            << ") smaller than specified region "
               "{{"
            << unsigned(xoffset) << "," << unsigned(yoffset) << "," << unsigned(zoffset) << "},{"
            << unsigned(width) << "," << unsigned(height) << "," << unsigned(depth) << ")}}";

    engine.getDriverApi().update3DImage(mHandle,
            uint8_t(level), xoffset, yoffset, zoffset, width, height, depth, std::move(p));

    // this method shouldn't have been const
    const_cast<FTexture*>(this)->updateLodRange(level);
}

// deprecated
void FTexture::setImage(FEngine& engine, size_t level,
        Texture::PixelBufferDescriptor&& buffer, const FaceOffsets& faceOffsets) const {

    auto validateTarget = [](SamplerType sampler) -> bool {
        switch (sampler) {
            case SamplerType::SAMPLER_CUBEMAP:
                return true;
            case SamplerType::SAMPLER_2D:
            case SamplerType::SAMPLER_3D:
            case SamplerType::SAMPLER_2D_ARRAY:
            case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            case SamplerType::SAMPLER_EXTERNAL:
                return false;
        }
    };

    // this should have been validated already
    assert_invariant(isTextureFormatSupported(engine, mFormat));

    FILAMENT_CHECK_PRECONDITION(buffer.type == PixelDataType::COMPRESSED ||
            validatePixelFormatAndType(mFormat, buffer.format, buffer.type))
            << "The combination of internal format=" << unsigned(mFormat)
            << " and {format=" << unsigned(buffer.format) << ", type=" << unsigned(buffer.type)
            << "} is not supported.";

    FILAMENT_CHECK_PRECONDITION(!mStream) << "setImage() called on a Stream texture.";

    FILAMENT_CHECK_PRECONDITION(level < mLevelCount)
            << "level=" << unsigned(level) << " is >= to levelCount=" << unsigned(mLevelCount)
            << ".";

    FILAMENT_CHECK_PRECONDITION(validateTarget(mTarget))
            << "Texture Sampler type (" << unsigned(mTarget)
            << ") not supported for this operation.";

    FILAMENT_CHECK_PRECONDITION(buffer.buffer) << "Data buffer is nullptr.";

    auto w = std::max(1u, mWidth >> level);
    auto h = std::max(1u, mHeight >> level);
    assert_invariant(w == h);
    const size_t faceSize = PixelBufferDescriptor::computeDataSize(buffer.format, buffer.type,
            buffer.stride ? buffer.stride : w, h, buffer.alignment);

    if (faceOffsets[0] == 0 &&
        faceOffsets[1] == 1 * faceSize &&
        faceOffsets[2] == 2 * faceSize &&
        faceOffsets[3] == 3 * faceSize &&
        faceOffsets[4] == 4 * faceSize &&
        faceOffsets[5] == 5 * faceSize) {
        // in this special case, we can upload all 6 faces in one call
        engine.getDriverApi().update3DImage(mHandle, uint8_t(level),
                0, 0, 0, w, h, 6, std::move(buffer));
    } else {
        UTILS_NOUNROLL
        for (size_t face = 0; face < 6; face++) {
            engine.getDriverApi().update3DImage(mHandle, uint8_t(level), 0, 0, face, w, h, 1, {
                    (char*)buffer.buffer + faceOffsets[face],
                    faceSize, buffer.format, buffer.type, buffer.alignment,
                    buffer.left, buffer.top, buffer.stride });
        }
        engine.getDriverApi().queueCommand(
                make_copyable_function([buffer = std::move(buffer)]() {}));
    }

    // this method shouldn't been const
    const_cast<FTexture*>(this)->updateLodRange(level);
}

void FTexture::setExternalImage(FEngine& engine, void* image) noexcept {
    if (mTarget != Sampler::SAMPLER_EXTERNAL) {
        return;
    }
    // The call to setupExternalImage is synchronous, and allows the driver to take ownership of the
    // external image on this thread, if necessary.
    auto& api = engine.getDriverApi();
    api.setupExternalImage(image);
    if (mHandle) {
        api.destroyTexture(mHandle);
        assert_invariant(mHandleForSampling == mHandle);
    }
    mHandle = api.createTextureExternalImage(mFormat, mWidth, mHeight, mUsage, image);
    mHandleForSampling = mHandle;
}

void FTexture::setExternalImage(FEngine& engine, void* image, size_t plane) noexcept {
    if (mTarget != Sampler::SAMPLER_EXTERNAL) {
        return;
    }
    // The call to setupExternalImage is synchronous, and allows the driver to take ownership of
    // the external image on this thread, if necessary.
    auto& api = engine.getDriverApi();
    api.setupExternalImage(image);
    if (mHandle) {
        api.destroyTexture(mHandle);
        assert_invariant(mHandleForSampling == mHandle);
    }
    mHandle = api.createTextureExternalImagePlane(mFormat, mWidth, mHeight, mUsage, image, plane);
    mHandleForSampling = mHandle;
}

void FTexture::setExternalStream(FEngine& engine, FStream* stream) noexcept {
    if (stream) {
        FILAMENT_CHECK_PRECONDITION(mTarget == Sampler::SAMPLER_EXTERNAL)
                << "Texture target must be SAMPLER_EXTERNAL";

        mStream = stream;
        engine.getDriverApi().setExternalStream(mHandle, stream->getHandle());
    } else {
        mStream = nullptr;
        engine.getDriverApi().setExternalStream(mHandle, backend::Handle<backend::HwStream>());
    }
}

void FTexture::generateMipmaps(FEngine& engine) const noexcept {
    FILAMENT_CHECK_PRECONDITION(mTarget != SamplerType::SAMPLER_EXTERNAL)
            << "External Textures are not mipmappable.";

    FILAMENT_CHECK_PRECONDITION(mTarget != SamplerType::SAMPLER_3D)
            << "3D Textures are not mipmappable.";

    const bool formatMipmappable = engine.getDriverApi().isTextureFormatMipmappable(mFormat);
    FILAMENT_CHECK_PRECONDITION(formatMipmappable)
            << "Texture format " << (unsigned)mFormat << " is not mipmappable.";

    if (mLevelCount < 2 || (mWidth == 1 && mHeight == 1)) {
        return;
    }

    engine.getDriverApi().generateMipmaps(mHandle);
    // this method shouldn't have been const
    const_cast<FTexture*>(this)->updateLodRange(0, mLevelCount);
}

bool FTexture::textureHandleCanMutate() const noexcept {
    // TODO: this will eventually include swizzling
    return (any(mUsage & Usage::SAMPLEABLE) && mLevelCount > 1) ||
            mTarget == SamplerType::SAMPLER_EXTERNAL;
}

void FTexture::updateLodRange(uint8_t baseLevel, uint8_t levelCount) noexcept {
    assert_invariant(mTarget != SamplerType::SAMPLER_EXTERNAL);
    if (any(mUsage & Usage::SAMPLEABLE) && mLevelCount > 1) {
        auto& range = mLodRange;
        uint8_t const last = int8_t(baseLevel + levelCount);
        if (range.first > baseLevel || range.last < last) {
            if (range.empty()) {
                range = { baseLevel, last };
            } else {
                range.first = std::min(range.first, baseLevel);
                range.last = std::max(range.last, last);
            }
            if (range.first == 0 && range.last == mLevelCount) {
                // the whole range lod range is used, we don't need the view anymore
                range.first = range.last = 0;
            }
            // We defer the creation of the texture view to getHwHandleForSampling() because it
            // is a common case that by then, the view won't be needed. Creating the first view on a
            // texture has a backend cost.
        }
    }
}

backend::Handle<backend::HwTexture> FTexture::getHwHandleForSampling() const noexcept {
    auto const& range = mLodRange;
    auto& activeRange = mActiveLodRange;
    if (UTILS_UNLIKELY(activeRange.first != range.first || activeRange.last != range.last)) {
        activeRange = range;
        if (mHandleForSampling != mHandle) {
            mDriver->destroyTexture(mHandleForSampling);
        }
        if (range.empty()) {
            mHandleForSampling = mHandle;
        } else {
            mHandleForSampling = mDriver->createTextureView(mHandle,
                    range.first,
                    range.last - range.first);
        }
    }
    return mHandleForSampling;
}

void FTexture::updateLodRange(uint8_t level) noexcept {
    updateLodRange(level, 1);
}

bool FTexture::isTextureFormatSupported(FEngine& engine, InternalFormat format) noexcept {
    return engine.getDriverApi().isTextureFormatSupported(format);
}

bool FTexture::isProtectedTexturesSupported(FEngine& engine) noexcept {
    return engine.getDriverApi().isProtectedTexturesSupported();
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


void FTexture::generatePrefilterMipmap(FEngine& engine,
        PixelBufferDescriptor&& buffer, const FaceOffsets& faceOffsets,
        PrefilterOptions const* options) {
    using namespace ibl;
    using namespace backend;
    using namespace math;

    const size_t size = getWidth();
    const size_t stride = buffer.stride ? buffer.stride : size;

    /* validate input data */

    FILAMENT_CHECK_PRECONDITION(
            buffer.format == PixelDataFormat::RGB || buffer.format == PixelDataFormat::RGBA)
            << "input data format must be RGB or RGBA";

    FILAMENT_CHECK_PRECONDITION(buffer.type == PixelDataType::FLOAT ||
            buffer.type == PixelDataType::HALF ||
            buffer.type == PixelDataType::UINT_10F_11F_11F_REV)
            << "input data type must be FLOAT, HALF or UINT_10F_11F_11F_REV";

    /* validate texture */

    FILAMENT_CHECK_PRECONDITION(!(size & (size - 1)))
            << "input data cubemap dimensions must be a power-of-two";

    FILAMENT_CHECK_PRECONDITION(!isCompressed()) << "reflections texture cannot be compressed";

    PrefilterOptions const defaultOptions;
    options = options ? options : &defaultOptions;

    JobSystem& js = engine.getJobSystem();
    FEngine::DriverApi& driver = engine.getDriverApi();

    auto generateMipmaps = [](JobSystem& js,
            FixedCapacityVector<Cubemap>& levels, FixedCapacityVector<Image>& images) {
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
        Cubemap::Face const face = (Cubemap::Face)j;
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
                    using fp10 = fp<0, 5, 5>;
                    using fp11 = fp<0, 5, 6>;
                    fp11 const r{ uint16_t( *src         & 0x7FFu) };
                    fp11 const g{ uint16_t((*src >> 11u) & 0x7FFu) };
                    fp10 const b{ uint16_t((*src >> 22u) & 0x3FFu) };
                    Cubemap::Texel const texel{ fp11::tof(r), fp11::tof(g), fp10::tof(b) };
                    Cubemap::writeAt(out, texel);
                }
            }
        }
    }

    /*
     * Create the mipmap chain
     */

    auto images = FixedCapacityVector<Image>::with_capacity(getLevels());
    auto levels = FixedCapacityVector<Cubemap>::with_capacity(getLevels());

    images.push_back(std::move(temp));
    levels.push_back(std::move(cml));

    const float3 mirror = options->mirror ? float3{ -1, 1, 1 } : float3{ 1, 1, 1 };

    // make the cubemap seamless
    levels[0].makeSeamless();

    // Now generate all the mipmap levels
    generateMipmaps(js, levels, images);

    // Finally generate each pre-filtered mipmap level
    const size_t baseExp = ctz(size);
    size_t const numSamples = options->sampleCount;
    const size_t numLevels = baseExp + 1;
    for (ssize_t i = (ssize_t)baseExp; i >= 0; --i) {
        const size_t dim = 1U << i;
        const size_t level = baseExp - i;
        const float lod = saturate(float(level) / float(numLevels - 1));
        const float linearRoughness = lod * lod;

        Image image;
        Cubemap dst = CubemapUtils::create(image, dim);
        CubemapIBL::roughnessFilter(js, dst, { levels.begin(), uint32_t(levels.size()) },
                linearRoughness, numSamples, mirror, true);

        Texture::PixelBufferDescriptor const pbd(image.getData(), image.getSize(),
                Texture::PixelBufferDescriptor::PixelDataFormat::RGB,
                Texture::PixelBufferDescriptor::PixelDataType::FLOAT, 1, 0, 0, image.getStride());

        uintptr_t const base = uintptr_t(image.getData());
        for (size_t j = 0; j < 6; j++) {
            Image const& faceImage = dst.getImageForFace((Cubemap::Face)j);
            auto offset = uintptr_t(faceImage.getData()) - base;
            driver.update3DImage(mHandle, level, 0, 0, j, dim, dim, 1, {
                    (char*)image.getData() + offset, dim * dim * 3 * sizeof(float),
                    Texture::PixelBufferDescriptor::PixelDataFormat::RGB,
                    Texture::PixelBufferDescriptor::PixelDataType::FLOAT, 1,
                    0, 0, uint32_t(image.getStride())
            });
        }

        // enqueue a commands that holds the image data until it's executed
        driver.queueCommand(make_copyable_function([data = image.detach()]() {}));
    }

    // no need to call the user callback because buffer is a reference, and it'll be destroyed
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
        case TextureFormat::RED_RGTC1:
        case TextureFormat::SIGNED_RED_RGTC1:
        case TextureFormat::RED_GREEN_RGTC2:
        case TextureFormat::SIGNED_RED_GREEN_RGTC2:
        case TextureFormat::RGB_BPTC_SIGNED_FLOAT:
        case TextureFormat::RGB_BPTC_UNSIGNED_FLOAT:
        case TextureFormat::RGBA_BPTC_UNORM:
        case TextureFormat::SRGB_ALPHA_BPTC_UNORM:
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
        case TextureFormat::RED_RGTC1:
        case TextureFormat::SIGNED_RED_RGTC1:
        case TextureFormat::RED_GREEN_RGTC2:
        case TextureFormat::SIGNED_RED_GREEN_RGTC2:
        case TextureFormat::RGB_BPTC_SIGNED_FLOAT:
        case TextureFormat::RGB_BPTC_UNSIGNED_FLOAT:
        case TextureFormat::RGBA_BPTC_UNORM:
        case TextureFormat::SRGB_ALPHA_BPTC_UNORM:
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

} // namespace filament
