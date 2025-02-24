//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Texture.cpp: Implements the gl::Texture class. [OpenGL ES 2.0.24] section 3.7 page 63.

#include "libANGLE/Texture.h"

#include "common/mathutil.h"
#include "common/utilities.h"
#include "libANGLE/Config.h"
#include "libANGLE/Context.h"
#include "libANGLE/Image.h"
#include "libANGLE/State.h"
#include "libANGLE/Surface.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/ContextImpl.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/TextureImpl.h"

namespace gl
{

namespace
{
constexpr angle::SubjectIndex kBufferSubjectIndex = 2;
static_assert(kBufferSubjectIndex != rx::kTextureImageImplObserverMessageIndex, "Index collision");
static_assert(kBufferSubjectIndex != rx::kTextureImageSiblingMessageIndex, "Index collision");

bool IsPointSampled(const SamplerState &samplerState)
{
    return (samplerState.getMagFilter() == GL_NEAREST &&
            (samplerState.getMinFilter() == GL_NEAREST ||
             samplerState.getMinFilter() == GL_NEAREST_MIPMAP_NEAREST));
}

size_t GetImageDescIndex(TextureTarget target, size_t level)
{
    return IsCubeMapFaceTarget(target) ? (level * 6 + CubeMapTextureTargetToFaceIndex(target))
                                       : level;
}

InitState DetermineInitState(const Context *context, Buffer *unpackBuffer, const uint8_t *pixels)
{
    // Can happen in tests.
    if (!context || !context->isRobustResourceInitEnabled())
    {
        return InitState::Initialized;
    }

    return (!pixels && !unpackBuffer) ? InitState::MayNeedInit : InitState::Initialized;
}
}  // namespace

GLenum ConvertToNearestFilterMode(GLenum filterMode)
{
    switch (filterMode)
    {
        case GL_LINEAR:
            return GL_NEAREST;
        case GL_LINEAR_MIPMAP_NEAREST:
            return GL_NEAREST_MIPMAP_NEAREST;
        case GL_LINEAR_MIPMAP_LINEAR:
            return GL_NEAREST_MIPMAP_LINEAR;
        default:
            return filterMode;
    }
}

GLenum ConvertToNearestMipFilterMode(GLenum filterMode)
{
    switch (filterMode)
    {
        case GL_LINEAR_MIPMAP_LINEAR:
            return GL_LINEAR_MIPMAP_NEAREST;
        case GL_NEAREST_MIPMAP_LINEAR:
            return GL_NEAREST_MIPMAP_NEAREST;
        default:
            return filterMode;
    }
}

bool IsMipmapSupported(const TextureType &type)
{
    switch (type)
    {
        case TextureType::_2DMultisample:
        case TextureType::_2DMultisampleArray:
        case TextureType::Buffer:
            return false;
        default:
            return true;
    }
}

SwizzleState::SwizzleState()
    : swizzleRed(GL_RED), swizzleGreen(GL_GREEN), swizzleBlue(GL_BLUE), swizzleAlpha(GL_ALPHA)
{}

SwizzleState::SwizzleState(GLenum red, GLenum green, GLenum blue, GLenum alpha)
    : swizzleRed(red), swizzleGreen(green), swizzleBlue(blue), swizzleAlpha(alpha)
{}

bool SwizzleState::swizzleRequired() const
{
    return swizzleRed != GL_RED || swizzleGreen != GL_GREEN || swizzleBlue != GL_BLUE ||
           swizzleAlpha != GL_ALPHA;
}

bool SwizzleState::operator==(const SwizzleState &other) const
{
    return swizzleRed == other.swizzleRed && swizzleGreen == other.swizzleGreen &&
           swizzleBlue == other.swizzleBlue && swizzleAlpha == other.swizzleAlpha;
}

bool SwizzleState::operator!=(const SwizzleState &other) const
{
    return !(*this == other);
}

TextureState::TextureState(TextureType type)
    : mType(type),
      mSamplerState(SamplerState::CreateDefaultForTarget(type)),
      mSrgbOverride(SrgbOverride::Default),
      mBaseLevel(0),
      mMaxLevel(kInitialMaxLevel),
      mDepthStencilTextureMode(GL_DEPTH_COMPONENT),
      mIsInternalIncompleteTexture(false),
      mHasBeenBoundAsImage(false),
      mHasBeenBoundAsAttachment(false),
      mHasBeenBoundToMSRTTFramebuffer(false),
      mImmutableFormat(false),
      mImmutableLevels(0),
      mUsage(GL_NONE),
      mHasProtectedContent(false),
      mRenderabilityValidation(true),
      mTilingMode(gl::TilingMode::Optimal),
      mImageDescs((IMPLEMENTATION_MAX_TEXTURE_LEVELS + 1) * (type == TextureType::CubeMap ? 6 : 1)),
      mCropRect(0, 0, 0, 0),
      mGenerateMipmapHint(GL_FALSE),
      mInitState(InitState::Initialized),
      mCachedSamplerFormat(SamplerFormat::InvalidEnum),
      mCachedSamplerCompareMode(GL_NONE),
      mCachedSamplerFormatValid(false),
      mCompressionFixedRate(GL_SURFACE_COMPRESSION_FIXED_RATE_NONE_EXT)
{}

TextureState::~TextureState() {}

bool TextureState::swizzleRequired() const
{
    return mSwizzleState.swizzleRequired();
}

GLuint TextureState::getEffectiveBaseLevel() const
{
    if (mImmutableFormat)
    {
        // GLES 3.0.4 section 3.8.10
        return std::min(mBaseLevel, mImmutableLevels - 1);
    }
    // Some classes use the effective base level to index arrays with level data. By clamping the
    // effective base level to max levels these arrays need just one extra item to store properties
    // that should be returned for all out-of-range base level values, instead of needing special
    // handling for out-of-range base levels.
    return std::min(mBaseLevel, static_cast<GLuint>(IMPLEMENTATION_MAX_TEXTURE_LEVELS));
}

GLuint TextureState::getEffectiveMaxLevel() const
{
    if (mImmutableFormat)
    {
        // GLES 3.0.4 section 3.8.10
        GLuint clampedMaxLevel = std::max(mMaxLevel, getEffectiveBaseLevel());
        clampedMaxLevel        = std::min(clampedMaxLevel, mImmutableLevels - 1);
        return clampedMaxLevel;
    }
    return mMaxLevel;
}

GLuint TextureState::getMipmapMaxLevel() const
{
    const ImageDesc &baseImageDesc = getImageDesc(getBaseImageTarget(), getEffectiveBaseLevel());
    GLuint expectedMipLevels       = 0;
    if (mType == TextureType::_3D)
    {
        const int maxDim = std::max(
            {baseImageDesc.size.width, baseImageDesc.size.height, baseImageDesc.size.depth});
        expectedMipLevels = static_cast<GLuint>(log2(maxDim));
    }
    else
    {
        expectedMipLevels = static_cast<GLuint>(
            log2(std::max(baseImageDesc.size.width, baseImageDesc.size.height)));
    }

    return std::min<GLuint>(getEffectiveBaseLevel() + expectedMipLevels, getEffectiveMaxLevel());
}

bool TextureState::setBaseLevel(GLuint baseLevel)
{
    if (mBaseLevel != baseLevel)
    {
        mBaseLevel = baseLevel;
        return true;
    }
    return false;
}

bool TextureState::setMaxLevel(GLuint maxLevel)
{
    if (mMaxLevel != maxLevel)
    {
        mMaxLevel = maxLevel;
        return true;
    }

    return false;
}

// Tests for cube texture completeness. [OpenGL ES 2.0.24] section 3.7.10 page 81.
// According to [OpenGL ES 3.0.5] section 3.8.13 Texture Completeness page 160 any
// per-level checks begin at the base-level.
// For OpenGL ES2 the base level is always zero.
bool TextureState::isCubeComplete() const
{
    ASSERT(mType == TextureType::CubeMap);

    angle::EnumIterator<TextureTarget> face = kCubeMapTextureTargetMin;
    const ImageDesc &baseImageDesc          = getImageDesc(*face, getEffectiveBaseLevel());
    if (baseImageDesc.size.width == 0 || baseImageDesc.size.width != baseImageDesc.size.height)
    {
        return false;
    }

    ++face;

    for (; face != kAfterCubeMapTextureTargetMax; ++face)
    {
        const ImageDesc &faceImageDesc = getImageDesc(*face, getEffectiveBaseLevel());
        if (faceImageDesc.size.width != baseImageDesc.size.width ||
            faceImageDesc.size.height != baseImageDesc.size.height ||
            !Format::SameSized(faceImageDesc.format, baseImageDesc.format))
        {
            return false;
        }
    }

    return true;
}

const ImageDesc &TextureState::getBaseLevelDesc() const
{
    ASSERT(mType != TextureType::CubeMap || isCubeComplete());
    return getImageDesc(getBaseImageTarget(), getEffectiveBaseLevel());
}

const ImageDesc &TextureState::getLevelZeroDesc() const
{
    ASSERT(mType != TextureType::CubeMap || isCubeComplete());
    return getImageDesc(getBaseImageTarget(), 0);
}

void TextureState::setCrop(const Rectangle &rect)
{
    mCropRect = rect;
}

const Rectangle &TextureState::getCrop() const
{
    return mCropRect;
}

void TextureState::setGenerateMipmapHint(GLenum hint)
{
    mGenerateMipmapHint = hint;
}

GLenum TextureState::getGenerateMipmapHint() const
{
    return mGenerateMipmapHint;
}

SamplerFormat TextureState::computeRequiredSamplerFormat(const SamplerState &samplerState) const
{
    const InternalFormat &info =
        *getImageDesc(getBaseImageTarget(), getEffectiveBaseLevel()).format.info;
    if ((info.format == GL_DEPTH_COMPONENT ||
         (info.format == GL_DEPTH_STENCIL && mDepthStencilTextureMode == GL_DEPTH_COMPONENT)) &&
        samplerState.getCompareMode() != GL_NONE)
    {
        return SamplerFormat::Shadow;
    }
    else if (info.format == GL_STENCIL_INDEX ||
             (info.format == GL_DEPTH_STENCIL && mDepthStencilTextureMode == GL_STENCIL_INDEX))
    {
        return SamplerFormat::Unsigned;
    }
    else
    {
        switch (info.componentType)
        {
            case GL_UNSIGNED_NORMALIZED:
            case GL_SIGNED_NORMALIZED:
            case GL_FLOAT:
                return SamplerFormat::Float;
            case GL_INT:
                return SamplerFormat::Signed;
            case GL_UNSIGNED_INT:
                return SamplerFormat::Unsigned;
            default:
                return SamplerFormat::InvalidEnum;
        }
    }
}

bool TextureState::computeSamplerCompleteness(const SamplerState &samplerState,
                                              const State &state) const
{
    // Buffer textures cannot be incomplete. But if they are, the spec says -
    //
    //     If no buffer object is bound to the buffer texture,
    //     the results of the texel access are undefined.
    //
    // Mark as incomplete so we use the default IncompleteTexture instead
    if (mType == TextureType::Buffer)
    {
        return mBuffer.get() != nullptr;
    }

    // Check for all non-format-based completeness rules
    if (!computeSamplerCompletenessForCopyImage(samplerState, state))
    {
        return false;
    }

    // OpenGL ES 3.2, Sections 8.8 and 11.1.3.3
    // Multisample textures do not have mipmaps and filter state is ignored.
    if (IsMultisampled(mType))
    {
        return true;
    }

    // OpenGL ES 3.2, Section 8.17
    // A texture is complete unless either the magnification filter is not NEAREST,
    // or the minification filter is neither NEAREST nor NEAREST_MIPMAP_NEAREST; and any of
    if (IsPointSampled(samplerState))
    {
        return true;
    }

    const InternalFormat *info =
        getImageDesc(getBaseImageTarget(), getEffectiveBaseLevel()).format.info;

    // The effective internal format specified for the texture images
    // is a sized internal color format that is not texture-filterable.
    if (!info->isDepthOrStencil())
    {
        return info->filterSupport(state.getClientVersion(), state.getExtensions());
    }

    // The effective internal format specified for the texture images
    // is a sized internal depth or depth and stencil format (see table 8.11),
    // and the value of TEXTURE_COMPARE_MODE is NONE.
    if (info->depthBits > 0 && samplerState.getCompareMode() == GL_NONE)
    {
        // Note: we restrict this validation to sized types. For the OES_depth_textures
        // extension, due to some underspecification problems, we must allow linear filtering
        // for legacy compatibility with WebGL 1.0.
        // See http://crbug.com/649200
        if (state.getClientMajorVersion() >= 3 && info->sized)
        {
            return false;
        }
    }

    if (info->stencilBits > 0)
    {
        if (info->depthBits > 0)
        {
            // The internal format of the texture is DEPTH_STENCIL,
            // and the value of DEPTH_STENCIL_TEXTURE_MODE for the
            // texture is STENCIL_INDEX.
            if (mDepthStencilTextureMode == GL_STENCIL_INDEX)
            {
                return false;
            }
        }
        else
        {
            // The internal format is STENCIL_INDEX.
            return false;
        }
    }

    return true;
}

// CopyImageSubData has more lax rules for texture completeness: format-based completeness rules are
// ignored, so a texture can still be considered complete even if it violates format-specific
// conditions
bool TextureState::computeSamplerCompletenessForCopyImage(const SamplerState &samplerState,
                                                          const State &state) const
{
    // Buffer textures cannot be incomplete. But if they are, the spec says -
    //
    //     If no buffer object is bound to the buffer texture,
    //     the results of the texel access are undefined.
    //
    // Mark as incomplete so we use the default IncompleteTexture instead
    if (mType == TextureType::Buffer)
    {
        return mBuffer.get() != nullptr;
    }

    if (!mImmutableFormat && mBaseLevel > mMaxLevel)
    {
        return false;
    }
    const ImageDesc &baseImageDesc = getImageDesc(getBaseImageTarget(), getEffectiveBaseLevel());
    if (baseImageDesc.size.width == 0 || baseImageDesc.size.height == 0 ||
        baseImageDesc.size.depth == 0)
    {
        return false;
    }
    // The cases where the texture is incomplete because base level is out of range should be
    // handled by the above condition.
    ASSERT(mBaseLevel < IMPLEMENTATION_MAX_TEXTURE_LEVELS || mImmutableFormat);

    if (mType == TextureType::CubeMap && baseImageDesc.size.width != baseImageDesc.size.height)
    {
        return false;
    }

    bool npotSupport = state.getExtensions().textureNpotOES || state.getClientMajorVersion() >= 3;
    if (!npotSupport)
    {
        if ((samplerState.getWrapS() != GL_CLAMP_TO_EDGE &&
             samplerState.getWrapS() != GL_CLAMP_TO_BORDER && !isPow2(baseImageDesc.size.width)) ||
            (samplerState.getWrapT() != GL_CLAMP_TO_EDGE &&
             samplerState.getWrapT() != GL_CLAMP_TO_BORDER && !isPow2(baseImageDesc.size.height)))
        {
            return false;
        }
    }

    if (IsMipmapSupported(mType) && IsMipmapFiltered(samplerState.getMinFilter()))
    {
        if (!npotSupport)
        {
            if (!isPow2(baseImageDesc.size.width) || !isPow2(baseImageDesc.size.height))
            {
                return false;
            }
        }

        if (!computeMipmapCompleteness())
        {
            return false;
        }
    }
    else
    {
        if (mType == TextureType::CubeMap && !isCubeComplete())
        {
            return false;
        }
    }

    // From GL_OES_EGL_image_external_essl3: If state is present in a sampler object bound to a
    // texture unit that would have been rejected by a call to TexParameter* for the texture bound
    // to that unit, the behavior of the implementation is as if the texture were incomplete. For
    // example, if TEXTURE_WRAP_S or TEXTURE_WRAP_T is set to anything but CLAMP_TO_EDGE on the
    // sampler object bound to a texture unit and the texture bound to that unit is an external
    // texture and EXT_EGL_image_external_wrap_modes is not enabled, the texture will be considered
    // incomplete.
    // Sampler object state which does not affect sampling for the type of texture bound
    // to a texture unit, such as TEXTURE_WRAP_R for an external texture, does not affect
    // completeness.
    if (mType == TextureType::External)
    {
        if (!state.getExtensions().EGLImageExternalWrapModesEXT)
        {
            if (samplerState.getWrapS() != GL_CLAMP_TO_EDGE ||
                samplerState.getWrapT() != GL_CLAMP_TO_EDGE)
            {
                return false;
            }
        }

        if (samplerState.getMinFilter() != GL_LINEAR && samplerState.getMinFilter() != GL_NEAREST)
        {
            return false;
        }
    }

    return true;
}

bool TextureState::computeMipmapCompleteness() const
{
    const GLuint maxLevel = getMipmapMaxLevel();

    for (GLuint level = getEffectiveBaseLevel(); level <= maxLevel; level++)
    {
        if (mType == TextureType::CubeMap)
        {
            for (TextureTarget face : AllCubeFaceTextureTargets())
            {
                if (!computeLevelCompleteness(face, level))
                {
                    return false;
                }
            }
        }
        else
        {
            if (!computeLevelCompleteness(NonCubeTextureTypeToTarget(mType), level))
            {
                return false;
            }
        }
    }

    return true;
}

bool TextureState::computeLevelCompleteness(TextureTarget target, size_t level) const
{
    ASSERT(level < IMPLEMENTATION_MAX_TEXTURE_LEVELS);

    if (mImmutableFormat)
    {
        return true;
    }

    const ImageDesc &baseImageDesc = getImageDesc(getBaseImageTarget(), getEffectiveBaseLevel());
    if (baseImageDesc.size.width == 0 || baseImageDesc.size.height == 0 ||
        baseImageDesc.size.depth == 0)
    {
        return false;
    }

    const ImageDesc &levelImageDesc = getImageDesc(target, level);
    if (levelImageDesc.size.width == 0 || levelImageDesc.size.height == 0 ||
        levelImageDesc.size.depth == 0)
    {
        return false;
    }

    if (!Format::SameSized(levelImageDesc.format, baseImageDesc.format))
    {
        return false;
    }

    ASSERT(level >= getEffectiveBaseLevel());
    const size_t relativeLevel = level - getEffectiveBaseLevel();
    if (levelImageDesc.size.width != std::max(1, baseImageDesc.size.width >> relativeLevel))
    {
        return false;
    }

    if (levelImageDesc.size.height != std::max(1, baseImageDesc.size.height >> relativeLevel))
    {
        return false;
    }

    if (mType == TextureType::_3D)
    {
        if (levelImageDesc.size.depth != std::max(1, baseImageDesc.size.depth >> relativeLevel))
        {
            return false;
        }
    }
    else if (IsArrayTextureType(mType))
    {
        if (levelImageDesc.size.depth != baseImageDesc.size.depth)
        {
            return false;
        }
    }

    return true;
}

TextureTarget TextureState::getBaseImageTarget() const
{
    return mType == TextureType::CubeMap ? kCubeMapTextureTargetMin
                                         : NonCubeTextureTypeToTarget(mType);
}

GLuint TextureState::getEnabledLevelCount() const
{
    GLuint levelCount      = 0;
    const GLuint baseLevel = getEffectiveBaseLevel();
    const GLuint maxLevel  = getMipmapMaxLevel();

    // The mip chain will have either one or more sequential levels, or max levels,
    // but not a sparse one.
    Optional<Extents> expectedSize;
    for (size_t enabledLevel = baseLevel; enabledLevel <= maxLevel; ++enabledLevel, ++levelCount)
    {
        // Note: for cube textures, we only check the first face.
        TextureTarget target     = TextureTypeToTarget(mType, 0);
        size_t descIndex         = GetImageDescIndex(target, enabledLevel);
        const Extents &levelSize = mImageDescs[descIndex].size;

        if (levelSize.empty())
        {
            break;
        }
        if (expectedSize.valid())
        {
            Extents newSize = expectedSize.value();
            newSize.width   = std::max(1, newSize.width >> 1);
            newSize.height  = std::max(1, newSize.height >> 1);

            if (!IsArrayTextureType(mType))
            {
                newSize.depth = std::max(1, newSize.depth >> 1);
            }

            if (newSize != levelSize)
            {
                break;
            }
        }
        expectedSize = levelSize;
    }

    return levelCount;
}

ImageDesc::ImageDesc()
    : ImageDesc(Extents(0, 0, 0), Format::Invalid(), 0, GL_TRUE, InitState::Initialized)
{}

ImageDesc::ImageDesc(const Extents &size, const Format &format, const InitState initState)
    : size(size), format(format), samples(0), fixedSampleLocations(GL_TRUE), initState(initState)
{}

ImageDesc::ImageDesc(const Extents &size,
                     const Format &format,
                     const GLsizei samples,
                     const bool fixedSampleLocations,
                     const InitState initState)
    : size(size),
      format(format),
      samples(samples),
      fixedSampleLocations(fixedSampleLocations),
      initState(initState)
{}

GLint ImageDesc::getMemorySize() const
{
    // Assume allocated size is around width * height * depth * samples * pixelBytes
    angle::CheckedNumeric<GLint> levelSize = 1;
    levelSize *= format.info->pixelBytes;
    levelSize *= size.width;
    levelSize *= size.height;
    levelSize *= size.depth;
    levelSize *= std::max(samples, 1);
    return levelSize.ValueOrDefault(std::numeric_limits<GLint>::max());
}

const ImageDesc &TextureState::getImageDesc(TextureTarget target, size_t level) const
{
    size_t descIndex = GetImageDescIndex(target, level);
    ASSERT(descIndex < mImageDescs.size());
    return mImageDescs[descIndex];
}

void TextureState::setImageDesc(TextureTarget target, size_t level, const ImageDesc &desc)
{
    size_t descIndex = GetImageDescIndex(target, level);
    ASSERT(descIndex < mImageDescs.size());
    mImageDescs[descIndex] = desc;
    if (desc.initState == InitState::MayNeedInit)
    {
        mInitState = InitState::MayNeedInit;
    }
    else
    {
        // Scan for any uninitialized images. If there are none, set the init state of the entire
        // texture to initialized. The cost of the scan is only paid after doing image
        // initialization which is already very expensive.
        bool allImagesInitialized = true;

        for (const ImageDesc &initDesc : mImageDescs)
        {
            if (initDesc.initState == InitState::MayNeedInit)
            {
                allImagesInitialized = false;
                break;
            }
        }

        if (allImagesInitialized)
        {
            mInitState = InitState::Initialized;
        }
    }
}

// Note that an ImageIndex that represents an entire level of a cube map corresponds to 6
// ImageDescs, so if the cube map is cube complete, we return the ImageDesc of the first cube
// face, and we don't allow using this function when the cube map is not cube complete.
const ImageDesc &TextureState::getImageDesc(const ImageIndex &imageIndex) const
{
    if (imageIndex.isEntireLevelCubeMap())
    {
        ASSERT(isCubeComplete());
        const GLint levelIndex = imageIndex.getLevelIndex();
        return getImageDesc(kCubeMapTextureTargetMin, levelIndex);
    }

    return getImageDesc(imageIndex.getTarget(), imageIndex.getLevelIndex());
}

void TextureState::setImageDescChain(GLuint baseLevel,
                                     GLuint maxLevel,
                                     Extents baseSize,
                                     const Format &format,
                                     InitState initState)
{
    for (GLuint level = baseLevel; level <= maxLevel; level++)
    {
        int relativeLevel = (level - baseLevel);
        Extents levelSize(std::max<int>(baseSize.width >> relativeLevel, 1),
                          std::max<int>(baseSize.height >> relativeLevel, 1),
                          (IsArrayTextureType(mType))
                              ? baseSize.depth
                              : std::max<int>(baseSize.depth >> relativeLevel, 1));
        ImageDesc levelInfo(levelSize, format, initState);

        if (mType == TextureType::CubeMap)
        {
            for (TextureTarget face : AllCubeFaceTextureTargets())
            {
                setImageDesc(face, level, levelInfo);
            }
        }
        else
        {
            setImageDesc(NonCubeTextureTypeToTarget(mType), level, levelInfo);
        }
    }
}

void TextureState::setImageDescChainMultisample(Extents baseSize,
                                                const Format &format,
                                                GLsizei samples,
                                                bool fixedSampleLocations,
                                                InitState initState)
{
    ASSERT(mType == TextureType::_2DMultisample || mType == TextureType::_2DMultisampleArray);
    ImageDesc levelInfo(baseSize, format, samples, fixedSampleLocations, initState);
    setImageDesc(NonCubeTextureTypeToTarget(mType), 0, levelInfo);
}

void TextureState::clearImageDesc(TextureTarget target, size_t level)
{
    setImageDesc(target, level, ImageDesc());
}

void TextureState::clearImageDescs()
{
    for (size_t descIndex = 0; descIndex < mImageDescs.size(); descIndex++)
    {
        mImageDescs[descIndex] = ImageDesc();
    }
}

TextureBufferContentsObservers::TextureBufferContentsObservers(Texture *texture) : mTexture(texture)
{}

void TextureBufferContentsObservers::enableForBuffer(Buffer *buffer)
{
    buffer->addContentsObserver(mTexture);
}

void TextureBufferContentsObservers::disableForBuffer(Buffer *buffer)
{
    buffer->removeContentsObserver(mTexture);
}

Texture::Texture(rx::GLImplFactory *factory, TextureID id, TextureType type)
    : RefCountObject(factory->generateSerial(), id),
      mState(type),
      mTexture(factory->createTexture(mState)),
      mImplObserver(this, rx::kTextureImageImplObserverMessageIndex),
      mBufferObserver(this, kBufferSubjectIndex),
      mBoundSurface(nullptr),
      mBoundStream(nullptr),
      mBufferContentsObservers(this)
{
    mImplObserver.bind(mTexture);
    if (mTexture)
    {
        mTexture->setContentsObservers(&mBufferContentsObservers);
    }

    // Initially assume the implementation is dirty.
    mDirtyBits.set(DIRTY_BIT_IMPLEMENTATION);
}

void Texture::onDestroy(const Context *context)
{
    onStateChange(angle::SubjectMessage::TextureIDDeleted);

    if (mBoundSurface)
    {
        ANGLE_SWALLOW_ERR(mBoundSurface->releaseTexImage(context, EGL_BACK_BUFFER));
        mBoundSurface = nullptr;
    }
    if (mBoundStream)
    {
        mBoundStream->releaseTextures();
        mBoundStream = nullptr;
    }

    egl::RefCountObjectReleaser<egl::Image> releaseImage;
    (void)orphanImages(context, &releaseImage);

    mState.mBuffer.set(context, nullptr, 0, 0);

    if (mTexture)
    {
        mTexture->onDestroy(context);
    }
}

Texture::~Texture()
{
    SafeDelete(mTexture);
}

angle::Result Texture::setLabel(const Context *context, const std::string &label)
{
    mState.mLabel = label;
    return mTexture->onLabelUpdate(context);
}

const std::string &Texture::getLabel() const
{
    return mState.mLabel;
}

void Texture::setSwizzleRed(const Context *context, GLenum swizzleRed)
{
    if (mState.mSwizzleState.swizzleRed != swizzleRed)
    {
        mState.mSwizzleState.swizzleRed = swizzleRed;
        signalDirtyState(DIRTY_BIT_SWIZZLE_RED);
    }
}

GLenum Texture::getSwizzleRed() const
{
    return mState.mSwizzleState.swizzleRed;
}

void Texture::setSwizzleGreen(const Context *context, GLenum swizzleGreen)
{
    if (mState.mSwizzleState.swizzleGreen != swizzleGreen)
    {
        mState.mSwizzleState.swizzleGreen = swizzleGreen;
        signalDirtyState(DIRTY_BIT_SWIZZLE_GREEN);
    }
}

GLenum Texture::getSwizzleGreen() const
{
    return mState.mSwizzleState.swizzleGreen;
}

void Texture::setSwizzleBlue(const Context *context, GLenum swizzleBlue)
{
    if (mState.mSwizzleState.swizzleBlue != swizzleBlue)
    {
        mState.mSwizzleState.swizzleBlue = swizzleBlue;
        signalDirtyState(DIRTY_BIT_SWIZZLE_BLUE);
    }
}

GLenum Texture::getSwizzleBlue() const
{
    return mState.mSwizzleState.swizzleBlue;
}

void Texture::setSwizzleAlpha(const Context *context, GLenum swizzleAlpha)
{
    if (mState.mSwizzleState.swizzleAlpha != swizzleAlpha)
    {
        mState.mSwizzleState.swizzleAlpha = swizzleAlpha;
        signalDirtyState(DIRTY_BIT_SWIZZLE_ALPHA);
    }
}

GLenum Texture::getSwizzleAlpha() const
{
    return mState.mSwizzleState.swizzleAlpha;
}

void Texture::setMinFilter(const Context *context, GLenum minFilter)
{
    if (mState.mSamplerState.setMinFilter(minFilter))
    {
        signalDirtyState(DIRTY_BIT_MIN_FILTER);
    }
}

GLenum Texture::getMinFilter() const
{
    return mState.mSamplerState.getMinFilter();
}

void Texture::setMagFilter(const Context *context, GLenum magFilter)
{
    if (mState.mSamplerState.setMagFilter(magFilter))
    {
        signalDirtyState(DIRTY_BIT_MAG_FILTER);
    }
}

GLenum Texture::getMagFilter() const
{
    return mState.mSamplerState.getMagFilter();
}

void Texture::setWrapS(const Context *context, GLenum wrapS)
{
    if (mState.mSamplerState.setWrapS(wrapS))
    {
        signalDirtyState(DIRTY_BIT_WRAP_S);
    }
}

GLenum Texture::getWrapS() const
{
    return mState.mSamplerState.getWrapS();
}

void Texture::setWrapT(const Context *context, GLenum wrapT)
{
    if (mState.mSamplerState.getWrapT() == wrapT)
        return;
    if (mState.mSamplerState.setWrapT(wrapT))
    {
        signalDirtyState(DIRTY_BIT_WRAP_T);
    }
}

GLenum Texture::getWrapT() const
{
    return mState.mSamplerState.getWrapT();
}

void Texture::setWrapR(const Context *context, GLenum wrapR)
{
    if (mState.mSamplerState.setWrapR(wrapR))
    {
        signalDirtyState(DIRTY_BIT_WRAP_R);
    }
}

GLenum Texture::getWrapR() const
{
    return mState.mSamplerState.getWrapR();
}

void Texture::setMaxAnisotropy(const Context *context, float maxAnisotropy)
{
    if (mState.mSamplerState.setMaxAnisotropy(maxAnisotropy))
    {
        signalDirtyState(DIRTY_BIT_MAX_ANISOTROPY);
    }
}

float Texture::getMaxAnisotropy() const
{
    return mState.mSamplerState.getMaxAnisotropy();
}

void Texture::setMinLod(const Context *context, GLfloat minLod)
{
    if (mState.mSamplerState.setMinLod(minLod))
    {
        signalDirtyState(DIRTY_BIT_MIN_LOD);
    }
}

GLfloat Texture::getMinLod() const
{
    return mState.mSamplerState.getMinLod();
}

void Texture::setMaxLod(const Context *context, GLfloat maxLod)
{
    if (mState.mSamplerState.setMaxLod(maxLod))
    {
        signalDirtyState(DIRTY_BIT_MAX_LOD);
    }
}

GLfloat Texture::getMaxLod() const
{
    return mState.mSamplerState.getMaxLod();
}

void Texture::setCompareMode(const Context *context, GLenum compareMode)
{
    if (mState.mSamplerState.setCompareMode(compareMode))
    {
        signalDirtyState(DIRTY_BIT_COMPARE_MODE);
    }
}

GLenum Texture::getCompareMode() const
{
    return mState.mSamplerState.getCompareMode();
}

void Texture::setCompareFunc(const Context *context, GLenum compareFunc)
{
    if (mState.mSamplerState.setCompareFunc(compareFunc))
    {
        signalDirtyState(DIRTY_BIT_COMPARE_FUNC);
    }
}

GLenum Texture::getCompareFunc() const
{
    return mState.mSamplerState.getCompareFunc();
}

void Texture::setSRGBDecode(const Context *context, GLenum sRGBDecode)
{
    if (mState.mSamplerState.setSRGBDecode(sRGBDecode))
    {
        signalDirtyState(DIRTY_BIT_SRGB_DECODE);
    }
}

GLenum Texture::getSRGBDecode() const
{
    return mState.mSamplerState.getSRGBDecode();
}

void Texture::setSRGBOverride(const Context *context, GLenum sRGBOverride)
{
    SrgbOverride oldOverride = mState.mSrgbOverride;
    mState.mSrgbOverride = (sRGBOverride == GL_SRGB) ? SrgbOverride::SRGB : SrgbOverride::Default;
    if (mState.mSrgbOverride != oldOverride)
    {
        signalDirtyState(DIRTY_BIT_SRGB_OVERRIDE);
    }
}

GLenum Texture::getSRGBOverride() const
{
    return (mState.mSrgbOverride == SrgbOverride::SRGB) ? GL_SRGB : GL_NONE;
}

const SamplerState &Texture::getSamplerState() const
{
    return mState.mSamplerState;
}

angle::Result Texture::setBaseLevel(const Context *context, GLuint baseLevel)
{
    if (mState.setBaseLevel(baseLevel))
    {
        ANGLE_TRY(mTexture->setBaseLevel(context, mState.getEffectiveBaseLevel()));
        signalDirtyState(DIRTY_BIT_BASE_LEVEL);
    }

    return angle::Result::Continue;
}

GLuint Texture::getBaseLevel() const
{
    return mState.mBaseLevel;
}

void Texture::setMaxLevel(const Context *context, GLuint maxLevel)
{
    if (mState.setMaxLevel(maxLevel))
    {
        signalDirtyState(DIRTY_BIT_MAX_LEVEL);
    }
}

GLuint Texture::getMaxLevel() const
{
    return mState.mMaxLevel;
}

void Texture::setDepthStencilTextureMode(const Context *context, GLenum mode)
{
    if (mState.mDepthStencilTextureMode != mode)
    {
        mState.mDepthStencilTextureMode = mode;
        signalDirtyState(DIRTY_BIT_DEPTH_STENCIL_TEXTURE_MODE);
    }
}

GLenum Texture::getDepthStencilTextureMode() const
{
    return mState.mDepthStencilTextureMode;
}

bool Texture::getImmutableFormat() const
{
    return mState.mImmutableFormat;
}

GLuint Texture::getImmutableLevels() const
{
    return mState.mImmutableLevels;
}

void Texture::setUsage(const Context *context, GLenum usage)
{
    mState.mUsage = usage;
    signalDirtyState(DIRTY_BIT_USAGE);
}

GLenum Texture::getUsage() const
{
    return mState.mUsage;
}

void Texture::setProtectedContent(Context *context, bool hasProtectedContent)
{
    mState.mHasProtectedContent = hasProtectedContent;
}

bool Texture::hasProtectedContent() const
{
    return mState.mHasProtectedContent;
}

void Texture::setRenderabilityValidation(Context *context, bool renderabilityValidation)
{
    mState.mRenderabilityValidation = renderabilityValidation;
    signalDirtyState(DIRTY_BIT_RENDERABILITY_VALIDATION_ANGLE);
}

void Texture::setTilingMode(Context *context, GLenum tilingMode)
{
    mState.mTilingMode = gl::FromGLenum<gl::TilingMode>(tilingMode);
}

GLenum Texture::getTilingMode() const
{
    return gl::ToGLenum(mState.mTilingMode);
}

const TextureState &Texture::getTextureState() const
{
    return mState;
}

const Extents &Texture::getExtents(TextureTarget target, size_t level) const
{
    ASSERT(TextureTargetToType(target) == mState.mType);
    return mState.getImageDesc(target, level).size;
}

size_t Texture::getWidth(TextureTarget target, size_t level) const
{
    ASSERT(TextureTargetToType(target) == mState.mType);
    return mState.getImageDesc(target, level).size.width;
}

size_t Texture::getHeight(TextureTarget target, size_t level) const
{
    ASSERT(TextureTargetToType(target) == mState.mType);
    return mState.getImageDesc(target, level).size.height;
}

size_t Texture::getDepth(TextureTarget target, size_t level) const
{
    ASSERT(TextureTargetToType(target) == mState.mType);
    return mState.getImageDesc(target, level).size.depth;
}

const Format &Texture::getFormat(TextureTarget target, size_t level) const
{
    ASSERT(TextureTargetToType(target) == mState.mType);
    return mState.getImageDesc(target, level).format;
}

GLsizei Texture::getSamples(TextureTarget target, size_t level) const
{
    ASSERT(TextureTargetToType(target) == mState.mType);
    return mState.getImageDesc(target, level).samples;
}

bool Texture::getFixedSampleLocations(TextureTarget target, size_t level) const
{
    ASSERT(TextureTargetToType(target) == mState.mType);
    return mState.getImageDesc(target, level).fixedSampleLocations;
}

GLuint Texture::getMipmapMaxLevel() const
{
    return mState.getMipmapMaxLevel();
}

bool Texture::isMipmapComplete() const
{
    return mState.computeMipmapCompleteness();
}

GLuint Texture::getFoveatedFeatureBits() const
{
    return mState.mFoveationState.getFoveatedFeatureBits();
}

void Texture::setFoveatedFeatureBits(const GLuint features)
{
    mState.mFoveationState.setFoveatedFeatureBits(features);
}

bool Texture::isFoveationEnabled() const
{
    return (mState.mFoveationState.getFoveatedFeatureBits() & GL_FOVEATION_ENABLE_BIT_QCOM);
}

GLuint Texture::getSupportedFoveationFeatures() const
{
    return mState.mFoveationState.getSupportedFoveationFeatures();
}

GLfloat Texture::getMinPixelDensity() const
{
    return mState.mFoveationState.getMinPixelDensity();
}

void Texture::setMinPixelDensity(const GLfloat density)
{
    mState.mFoveationState.setMinPixelDensity(density);
}

void Texture::setFocalPoint(uint32_t layer,
                            uint32_t focalPointIndex,
                            float focalX,
                            float focalY,
                            float gainX,
                            float gainY,
                            float foveaArea)
{
    gl::FocalPoint newFocalPoint(focalX, focalY, gainX, gainY, foveaArea);
    if (mState.mFoveationState.getFocalPoint(layer, focalPointIndex) == newFocalPoint)
    {
        // Nothing to do, early out.
        return;
    }

    mState.mFoveationState.setFocalPoint(layer, focalPointIndex, newFocalPoint);
    mState.mFoveationState.setFoveatedFeatureBits(GL_FOVEATION_ENABLE_BIT_QCOM);
    onStateChange(angle::SubjectMessage::FoveatedRenderingStateChanged);
}

const FocalPoint &Texture::getFocalPoint(uint32_t layer, uint32_t focalPoint) const
{
    return mState.mFoveationState.getFocalPoint(layer, focalPoint);
}

egl::Surface *Texture::getBoundSurface() const
{
    return mBoundSurface;
}

egl::Stream *Texture::getBoundStream() const
{
    return mBoundStream;
}

GLint Texture::getMemorySize() const
{
    GLint implSize = mTexture->getMemorySize();
    if (implSize > 0)
    {
        return implSize;
    }

    angle::CheckedNumeric<GLint> size = 0;
    for (const ImageDesc &imageDesc : mState.mImageDescs)
    {
        size += imageDesc.getMemorySize();
    }
    return size.ValueOrDefault(std::numeric_limits<GLint>::max());
}

GLint Texture::getLevelMemorySize(TextureTarget target, GLint level) const
{
    GLint implSize = mTexture->getLevelMemorySize(target, level);
    if (implSize > 0)
    {
        return implSize;
    }

    return mState.getImageDesc(target, level).getMemorySize();
}

void Texture::signalDirtyStorage(InitState initState)
{
    mState.mInitState = initState;
    invalidateCompletenessCache();
    mState.mCachedSamplerFormatValid = false;
    onStateChange(angle::SubjectMessage::SubjectChanged);
}

void Texture::signalDirtyState(size_t dirtyBit)
{
    mDirtyBits.set(dirtyBit);
    invalidateCompletenessCache();
    mState.mCachedSamplerFormatValid = false;

    if (dirtyBit == DIRTY_BIT_BASE_LEVEL || dirtyBit == DIRTY_BIT_MAX_LEVEL)
    {
        onStateChange(angle::SubjectMessage::SubjectChanged);
    }
    else
    {
        onStateChange(angle::SubjectMessage::DirtyBitsFlagged);
    }
}

angle::Result Texture::setImage(Context *context,
                                const PixelUnpackState &unpackState,
                                Buffer *unpackBuffer,
                                TextureTarget target,
                                GLint level,
                                GLenum internalFormat,
                                const Extents &size,
                                GLenum format,
                                GLenum type,
                                const uint8_t *pixels)
{
    ASSERT(TextureTargetToType(target) == mState.mType);

    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    ANGLE_TRY(releaseTexImageInternal(context));

    egl::RefCountObjectReleaser<egl::Image> releaseImage;
    ANGLE_TRY(orphanImages(context, &releaseImage));

    ImageIndex index = ImageIndex::MakeFromTarget(target, level, size.depth);

    ANGLE_TRY(mTexture->setImage(context, index, internalFormat, size, format, type, unpackState,
                                 unpackBuffer, pixels));

    InitState initState = DetermineInitState(context, unpackBuffer, pixels);
    mState.setImageDesc(target, level, ImageDesc(size, Format(internalFormat, type), initState));

    ANGLE_TRY(handleMipmapGenerationHint(context, level));

    signalDirtyStorage(initState);

    return angle::Result::Continue;
}

angle::Result Texture::setSubImage(Context *context,
                                   const PixelUnpackState &unpackState,
                                   Buffer *unpackBuffer,
                                   TextureTarget target,
                                   GLint level,
                                   const Box &area,
                                   GLenum format,
                                   GLenum type,
                                   const uint8_t *pixels)
{
    ASSERT(TextureTargetToType(target) == mState.mType);

    ImageIndex index = ImageIndex::MakeFromTarget(target, level, area.depth);
    ANGLE_TRY(ensureSubImageInitialized(context, index, area));

    ANGLE_TRY(mTexture->setSubImage(context, index, area, format, type, unpackState, unpackBuffer,
                                    pixels));

    ANGLE_TRY(handleMipmapGenerationHint(context, level));

    onStateChange(angle::SubjectMessage::ContentsChanged);

    return angle::Result::Continue;
}

angle::Result Texture::setCompressedImage(Context *context,
                                          const PixelUnpackState &unpackState,
                                          TextureTarget target,
                                          GLint level,
                                          GLenum internalFormat,
                                          const Extents &size,
                                          size_t imageSize,
                                          const uint8_t *pixels)
{
    ASSERT(TextureTargetToType(target) == mState.mType);

    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    ANGLE_TRY(releaseTexImageInternal(context));

    egl::RefCountObjectReleaser<egl::Image> releaseImage;
    ANGLE_TRY(orphanImages(context, &releaseImage));

    ImageIndex index = ImageIndex::MakeFromTarget(target, level, size.depth);

    ANGLE_TRY(mTexture->setCompressedImage(context, index, internalFormat, size, unpackState,
                                           imageSize, pixels));

    Buffer *unpackBuffer = context->getState().getTargetBuffer(BufferBinding::PixelUnpack);

    InitState initState = DetermineInitState(context, unpackBuffer, pixels);
    mState.setImageDesc(target, level, ImageDesc(size, Format(internalFormat), initState));
    signalDirtyStorage(initState);

    return angle::Result::Continue;
}

angle::Result Texture::setCompressedSubImage(const Context *context,
                                             const PixelUnpackState &unpackState,
                                             TextureTarget target,
                                             GLint level,
                                             const Box &area,
                                             GLenum format,
                                             size_t imageSize,
                                             const uint8_t *pixels)
{
    ASSERT(TextureTargetToType(target) == mState.mType);

    ImageIndex index = ImageIndex::MakeFromTarget(target, level, area.depth);
    ANGLE_TRY(ensureSubImageInitialized(context, index, area));

    ANGLE_TRY(mTexture->setCompressedSubImage(context, index, area, format, unpackState, imageSize,
                                              pixels));

    onStateChange(angle::SubjectMessage::ContentsChanged);

    return angle::Result::Continue;
}

angle::Result Texture::copyImage(Context *context,
                                 TextureTarget target,
                                 GLint level,
                                 const Rectangle &sourceArea,
                                 GLenum internalFormat,
                                 Framebuffer *source)
{
    ASSERT(TextureTargetToType(target) == mState.mType);

    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    ANGLE_TRY(releaseTexImageInternal(context));

    egl::RefCountObjectReleaser<egl::Image> releaseImage;
    ANGLE_TRY(orphanImages(context, &releaseImage));

    ImageIndex index = ImageIndex::MakeFromTarget(target, level, 1);

    const InternalFormat &internalFormatInfo =
        GetInternalFormatInfo(internalFormat, GL_UNSIGNED_BYTE);

    // Most if not all renderers clip these copies to the size of the source framebuffer, leaving
    // other pixels untouched. For safety in robust resource initialization, assume that that
    // clipping is going to occur when computing the region for which to ensure initialization. If
    // the copy lies entirely off the source framebuffer, initialize as though a zero-size box is
    // going to be set during the copy operation.
    Box destBox;
    bool forceCopySubImage = false;
    if (context->isRobustResourceInitEnabled())
    {
        const FramebufferAttachment *sourceReadAttachment = source->getReadColorAttachment();
        Extents fbSize                                    = sourceReadAttachment->getSize();
        // Force using copySubImage when the source area is out of bounds AND
        // we're not copying to and from the same texture
        forceCopySubImage = ((sourceArea.x < 0) || (sourceArea.y < 0) ||
                             ((sourceArea.x + sourceArea.width) > fbSize.width) ||
                             ((sourceArea.y + sourceArea.height) > fbSize.height)) &&
                            (sourceReadAttachment->getResource() != this);
        Rectangle clippedArea;
        if (ClipRectangle(sourceArea, Rectangle(0, 0, fbSize.width, fbSize.height), &clippedArea))
        {
            const Offset clippedOffset(clippedArea.x - sourceArea.x, clippedArea.y - sourceArea.y,
                                       0);
            destBox = Box(clippedOffset.x, clippedOffset.y, clippedOffset.z, clippedArea.width,
                          clippedArea.height, 1);
        }
    }

    InitState initState = DetermineInitState(context, nullptr, nullptr);

    // If we need to initialize the destination texture we split the call into a create call,
    // an initializeContents call, and then a copySubImage call. This ensures the destination
    // texture exists before we try to clear it.
    Extents size(sourceArea.width, sourceArea.height, 1);
    if (forceCopySubImage || doesSubImageNeedInit(context, index, destBox))
    {
        ANGLE_TRY(mTexture->setImage(context, index, internalFormat, size,
                                     internalFormatInfo.format, internalFormatInfo.type,
                                     PixelUnpackState(), nullptr, nullptr));
        mState.setImageDesc(target, level, ImageDesc(size, Format(internalFormatInfo), initState));
        ANGLE_TRY(ensureSubImageInitialized(context, index, destBox));
        ANGLE_TRY(mTexture->copySubImage(context, index, Offset(), sourceArea, source));
    }
    else
    {
        ANGLE_TRY(mTexture->copyImage(context, index, sourceArea, internalFormat, source));
    }

    mState.setImageDesc(target, level,
                        ImageDesc(size, Format(internalFormatInfo), InitState::Initialized));

    ANGLE_TRY(handleMipmapGenerationHint(context, level));

    // Because this could affect the texture storage we might need to init other layers/levels.
    signalDirtyStorage(initState);

    return angle::Result::Continue;
}

angle::Result Texture::copySubImage(Context *context,
                                    const ImageIndex &index,
                                    const Offset &destOffset,
                                    const Rectangle &sourceArea,
                                    Framebuffer *source)
{
    ASSERT(TextureTargetToType(index.getTarget()) == mState.mType);

    // Most if not all renderers clip these copies to the size of the source framebuffer, leaving
    // other pixels untouched. For safety in robust resource initialization, assume that that
    // clipping is going to occur when computing the region for which to ensure initialization. If
    // the copy lies entirely off the source framebuffer, initialize as though a zero-size box is
    // going to be set during the copy operation. Note that this assumes that
    // ensureSubImageInitialized ensures initialization of the entire destination texture, and not
    // just a sub-region.
    Box destBox;
    if (context->isRobustResourceInitEnabled())
    {
        Extents fbSize = source->getReadColorAttachment()->getSize();
        Rectangle clippedArea;
        if (ClipRectangle(sourceArea, Rectangle(0, 0, fbSize.width, fbSize.height), &clippedArea))
        {
            const Offset clippedOffset(destOffset.x + clippedArea.x - sourceArea.x,
                                       destOffset.y + clippedArea.y - sourceArea.y, 0);
            destBox = Box(clippedOffset.x, clippedOffset.y, clippedOffset.z, clippedArea.width,
                          clippedArea.height, 1);
        }
    }

    ANGLE_TRY(ensureSubImageInitialized(context, index, destBox));

    ANGLE_TRY(mTexture->copySubImage(context, index, destOffset, sourceArea, source));
    ANGLE_TRY(handleMipmapGenerationHint(context, index.getLevelIndex()));

    onStateChange(angle::SubjectMessage::ContentsChanged);

    return angle::Result::Continue;
}

angle::Result Texture::copyRenderbufferSubData(Context *context,
                                               const gl::Renderbuffer *srcBuffer,
                                               GLint srcLevel,
                                               GLint srcX,
                                               GLint srcY,
                                               GLint srcZ,
                                               GLint dstLevel,
                                               GLint dstX,
                                               GLint dstY,
                                               GLint dstZ,
                                               GLsizei srcWidth,
                                               GLsizei srcHeight,
                                               GLsizei srcDepth)
{
    ANGLE_TRY(mTexture->copyRenderbufferSubData(context, srcBuffer, srcLevel, srcX, srcY, srcZ,
                                                dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight,
                                                srcDepth));

    signalDirtyStorage(InitState::Initialized);

    return angle::Result::Continue;
}

angle::Result Texture::copyTextureSubData(Context *context,
                                          const gl::Texture *srcTexture,
                                          GLint srcLevel,
                                          GLint srcX,
                                          GLint srcY,
                                          GLint srcZ,
                                          GLint dstLevel,
                                          GLint dstX,
                                          GLint dstY,
                                          GLint dstZ,
                                          GLsizei srcWidth,
                                          GLsizei srcHeight,
                                          GLsizei srcDepth)
{
    ANGLE_TRY(mTexture->copyTextureSubData(context, srcTexture, srcLevel, srcX, srcY, srcZ,
                                           dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight,
                                           srcDepth));

    signalDirtyStorage(InitState::Initialized);

    return angle::Result::Continue;
}

angle::Result Texture::copyTexture(Context *context,
                                   TextureTarget target,
                                   GLint level,
                                   GLenum internalFormat,
                                   GLenum type,
                                   GLint sourceLevel,
                                   bool unpackFlipY,
                                   bool unpackPremultiplyAlpha,
                                   bool unpackUnmultiplyAlpha,
                                   Texture *source)
{
    ASSERT(TextureTargetToType(target) == mState.mType);
    ASSERT(source->getType() != TextureType::CubeMap);

    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    ANGLE_TRY(releaseTexImageInternal(context));

    egl::RefCountObjectReleaser<egl::Image> releaseImage;
    ANGLE_TRY(orphanImages(context, &releaseImage));

    // Initialize source texture.
    // Note: we don't have a way to notify which portions of the image changed currently.
    ANGLE_TRY(source->ensureInitialized(context));

    ImageIndex index = ImageIndex::MakeFromTarget(target, level, ImageIndex::kEntireLevel);

    ANGLE_TRY(mTexture->copyTexture(context, index, internalFormat, type, sourceLevel, unpackFlipY,
                                    unpackPremultiplyAlpha, unpackUnmultiplyAlpha, source));

    const auto &sourceDesc =
        source->mState.getImageDesc(NonCubeTextureTypeToTarget(source->getType()), sourceLevel);
    const InternalFormat &internalFormatInfo = GetInternalFormatInfo(internalFormat, type);
    mState.setImageDesc(
        target, level,
        ImageDesc(sourceDesc.size, Format(internalFormatInfo), InitState::Initialized));

    signalDirtyStorage(InitState::Initialized);

    return angle::Result::Continue;
}

angle::Result Texture::copySubTexture(const Context *context,
                                      TextureTarget target,
                                      GLint level,
                                      const Offset &destOffset,
                                      GLint sourceLevel,
                                      const Box &sourceBox,
                                      bool unpackFlipY,
                                      bool unpackPremultiplyAlpha,
                                      bool unpackUnmultiplyAlpha,
                                      Texture *source)
{
    ASSERT(TextureTargetToType(target) == mState.mType);

    // Ensure source is initialized.
    ANGLE_TRY(source->ensureInitialized(context));

    Box destBox(destOffset.x, destOffset.y, destOffset.z, sourceBox.width, sourceBox.height,
                sourceBox.depth);
    ImageIndex index = ImageIndex::MakeFromTarget(target, level, sourceBox.depth);
    ANGLE_TRY(ensureSubImageInitialized(context, index, destBox));

    ANGLE_TRY(mTexture->copySubTexture(context, index, destOffset, sourceLevel, sourceBox,
                                       unpackFlipY, unpackPremultiplyAlpha, unpackUnmultiplyAlpha,
                                       source));

    onStateChange(angle::SubjectMessage::ContentsChanged);

    return angle::Result::Continue;
}

angle::Result Texture::copyCompressedTexture(Context *context, const Texture *source)
{
    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    ANGLE_TRY(releaseTexImageInternal(context));

    egl::RefCountObjectReleaser<egl::Image> releaseImage;
    ANGLE_TRY(orphanImages(context, &releaseImage));

    ANGLE_TRY(mTexture->copyCompressedTexture(context, source));

    ASSERT(source->getType() != TextureType::CubeMap && getType() != TextureType::CubeMap);
    const auto &sourceDesc =
        source->mState.getImageDesc(NonCubeTextureTypeToTarget(source->getType()), 0);
    mState.setImageDesc(NonCubeTextureTypeToTarget(getType()), 0, sourceDesc);

    return angle::Result::Continue;
}

angle::Result Texture::setStorage(Context *context,
                                  TextureType type,
                                  GLsizei levels,
                                  GLenum internalFormat,
                                  const Extents &size)
{
    ASSERT(type == mState.mType);

    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    ANGLE_TRY(releaseTexImageInternal(context));

    egl::RefCountObjectReleaser<egl::Image> releaseImage;
    ANGLE_TRY(orphanImages(context, &releaseImage));

    mState.mImmutableFormat = true;
    mState.mImmutableLevels = static_cast<GLuint>(levels);
    mState.clearImageDescs();
    InitState initState = DetermineInitState(context, nullptr, nullptr);
    mState.setImageDescChain(0, static_cast<GLuint>(levels - 1), size, Format(internalFormat),
                             initState);

    ANGLE_TRY(mTexture->setStorage(context, type, levels, internalFormat, size));

    // Changing the texture to immutable can trigger a change in the base and max levels:
    // GLES 3.0.4 section 3.8.10 pg 158:
    // "For immutable-format textures, levelbase is clamped to the range[0;levels],levelmax is then
    // clamped to the range[levelbase;levels].
    mDirtyBits.set(DIRTY_BIT_BASE_LEVEL);
    mDirtyBits.set(DIRTY_BIT_MAX_LEVEL);

    signalDirtyStorage(initState);

    return angle::Result::Continue;
}

angle::Result Texture::setImageExternal(Context *context,
                                        TextureTarget target,
                                        GLint level,
                                        GLenum internalFormat,
                                        const Extents &size,
                                        GLenum format,
                                        GLenum type)
{
    ASSERT(TextureTargetToType(target) == mState.mType);

    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    ANGLE_TRY(releaseTexImageInternal(context));

    egl::RefCountObjectReleaser<egl::Image> releaseImage;
    ANGLE_TRY(orphanImages(context, &releaseImage));

    ImageIndex index = ImageIndex::MakeFromTarget(target, level, size.depth);

    ANGLE_TRY(mTexture->setImageExternal(context, index, internalFormat, size, format, type));

    InitState initState = InitState::Initialized;
    mState.setImageDesc(target, level, ImageDesc(size, Format(internalFormat, type), initState));

    ANGLE_TRY(handleMipmapGenerationHint(context, level));

    signalDirtyStorage(initState);

    return angle::Result::Continue;
}

angle::Result Texture::setStorageMultisample(Context *context,
                                             TextureType type,
                                             GLsizei samplesIn,
                                             GLint internalFormat,
                                             const Extents &size,
                                             bool fixedSampleLocations)
{
    ASSERT(type == mState.mType);

    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    ANGLE_TRY(releaseTexImageInternal(context));

    egl::RefCountObjectReleaser<egl::Image> releaseImage;
    ANGLE_TRY(orphanImages(context, &releaseImage));

    // Potentially adjust "samples" to a supported value
    const TextureCaps &formatCaps = context->getTextureCaps().get(internalFormat);
    GLsizei samples               = formatCaps.getNearestSamples(samplesIn);

    mState.mImmutableFormat = true;
    mState.mImmutableLevels = static_cast<GLuint>(1);
    mState.clearImageDescs();
    InitState initState = DetermineInitState(context, nullptr, nullptr);
    mState.setImageDescChainMultisample(size, Format(internalFormat), samples, fixedSampleLocations,
                                        initState);

    ANGLE_TRY(mTexture->setStorageMultisample(context, type, samples, internalFormat, size,
                                              fixedSampleLocations));
    signalDirtyStorage(initState);

    return angle::Result::Continue;
}

angle::Result Texture::setStorageExternalMemory(Context *context,
                                                TextureType type,
                                                GLsizei levels,
                                                GLenum internalFormat,
                                                const Extents &size,
                                                MemoryObject *memoryObject,
                                                GLuint64 offset,
                                                GLbitfield createFlags,
                                                GLbitfield usageFlags,
                                                const void *imageCreateInfoPNext)
{
    ASSERT(type == mState.mType);

    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    ANGLE_TRY(releaseTexImageInternal(context));

    egl::RefCountObjectReleaser<egl::Image> releaseImage;
    ANGLE_TRY(orphanImages(context, &releaseImage));

    ANGLE_TRY(mTexture->setStorageExternalMemory(context, type, levels, internalFormat, size,
                                                 memoryObject, offset, createFlags, usageFlags,
                                                 imageCreateInfoPNext));

    mState.mImmutableFormat = true;
    mState.mImmutableLevels = static_cast<GLuint>(levels);
    mState.clearImageDescs();
    mState.setImageDescChain(0, static_cast<GLuint>(levels - 1), size, Format(internalFormat),
                             InitState::Initialized);

    // Changing the texture to immutable can trigger a change in the base and max levels:
    // GLES 3.0.4 section 3.8.10 pg 158:
    // "For immutable-format textures, levelbase is clamped to the range[0;levels],levelmax is then
    // clamped to the range[levelbase;levels].
    mDirtyBits.set(DIRTY_BIT_BASE_LEVEL);
    mDirtyBits.set(DIRTY_BIT_MAX_LEVEL);

    signalDirtyStorage(InitState::Initialized);

    return angle::Result::Continue;
}

angle::Result Texture::setStorageAttribs(Context *context,
                                         TextureType type,
                                         GLsizei levels,
                                         GLenum internalFormat,
                                         const Extents &size,
                                         const GLint *attribList)
{
    ASSERT(type == mState.mType);

    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    ANGLE_TRY(releaseTexImageInternal(context));

    egl::RefCountObjectReleaser<egl::Image> releaseImage;
    ANGLE_TRY(orphanImages(context, &releaseImage));

    mState.mImmutableFormat = true;
    mState.mImmutableLevels = static_cast<GLuint>(levels);
    mState.clearImageDescs();
    InitState initState = DetermineInitState(context, nullptr, nullptr);
    mState.setImageDescChain(0, static_cast<GLuint>(levels - 1), size, Format(internalFormat),
                             initState);

    if (nullptr != attribList && GL_SURFACE_COMPRESSION_EXT == *attribList)
    {
        attribList++;
        if (nullptr != attribList && GL_NONE != *attribList)
        {
            mState.mCompressionFixedRate = *attribList;
        }
    }

    ANGLE_TRY(mTexture->setStorageAttribs(context, type, levels, internalFormat, size, attribList));

    // Changing the texture to immutable can trigger a change in the base and max levels:
    // GLES 3.0.4 section 3.8.10 pg 158:
    // "For immutable-format textures, levelbase is clamped to the range[0;levels],levelmax is then
    // clamped to the range[levelbase;levels].
    mDirtyBits.set(DIRTY_BIT_BASE_LEVEL);
    mDirtyBits.set(DIRTY_BIT_MAX_LEVEL);

    signalDirtyStorage(initState);

    return angle::Result::Continue;
}

GLint Texture::getImageCompressionRate(const Context *context) const
{
    return mTexture->getImageCompressionRate(context);
}

GLint Texture::getFormatSupportedCompressionRates(const Context *context,
                                                  GLenum internalformat,
                                                  GLsizei bufSize,
                                                  GLint *rates) const
{
    return mTexture->getFormatSupportedCompressionRates(context, internalformat, bufSize, rates);
}

angle::Result Texture::generateMipmap(Context *context)
{
    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    ANGLE_TRY(releaseTexImageInternal(context));

    // EGL_KHR_gl_image states that images are only orphaned when generating mipmaps if the texture
    // is not mip complete.
    egl::RefCountObjectReleaser<egl::Image> releaseImage;
    if (!isMipmapComplete())
    {
        ANGLE_TRY(orphanImages(context, &releaseImage));
    }

    const GLuint baseLevel = mState.getEffectiveBaseLevel();
    const GLuint maxLevel  = mState.getMipmapMaxLevel();

    if (maxLevel <= baseLevel)
    {
        return angle::Result::Continue;
    }

    // If any dimension is zero, this is a no-op:
    //
    // > Otherwise, if level_base is not defined, or if any dimension is zero, all mipmap levels are
    // > left unchanged. This is not an error.
    const ImageDesc &baseImageInfo = mState.getImageDesc(mState.getBaseImageTarget(), baseLevel);
    if (baseImageInfo.size.empty())
    {
        return angle::Result::Continue;
    }

    // Clear the base image(s) immediately if needed
    if (context->isRobustResourceInitEnabled())
    {
        ImageIndexIterator it =
            ImageIndexIterator::MakeGeneric(mState.mType, baseLevel, baseLevel + 1,
                                            ImageIndex::kEntireLevel, ImageIndex::kEntireLevel);
        while (it.hasNext())
        {
            const ImageIndex index = it.next();
            const ImageDesc &desc  = mState.getImageDesc(index.getTarget(), index.getLevelIndex());

            if (desc.initState == InitState::MayNeedInit)
            {
                ANGLE_TRY(initializeContents(context, GL_NONE, index));
            }
        }
    }

    ANGLE_TRY(syncState(context, Command::GenerateMipmap));
    ANGLE_TRY(mTexture->generateMipmap(context));

    // Propagate the format and size of the base mip to the smaller ones. Cube maps are guaranteed
    // to have faces of the same size and format so any faces can be picked.
    mState.setImageDescChain(baseLevel, maxLevel, baseImageInfo.size, baseImageInfo.format,
                             InitState::Initialized);

    signalDirtyStorage(InitState::Initialized);

    return angle::Result::Continue;
}

angle::Result Texture::clearImage(Context *context,
                                  GLint level,
                                  GLenum format,
                                  GLenum type,
                                  const uint8_t *data)
{
    ANGLE_TRY(mTexture->clearImage(context, level, format, type, data));

    ANGLE_TRY(handleMipmapGenerationHint(context, level));

    ImageIndexIterator it = ImageIndexIterator::MakeGeneric(
        mState.mType, level, level + 1, ImageIndex::kEntireLevel, ImageIndex::kEntireLevel);
    while (it.hasNext())
    {
        const ImageIndex index = it.next();
        setInitState(GL_NONE, index, InitState::Initialized);
    }

    onStateChange(angle::SubjectMessage::ContentsChanged);

    return angle::Result::Continue;
}

angle::Result Texture::clearSubImage(Context *context,
                                     GLint level,
                                     const Box &area,
                                     GLenum format,
                                     GLenum type,
                                     const uint8_t *data)
{
    const ImageIndexIterator allImagesIterator = ImageIndexIterator::MakeGeneric(
        mState.mType, level, level + 1, area.z, area.z + area.depth);

    ImageIndexIterator initImagesIterator = allImagesIterator;
    while (initImagesIterator.hasNext())
    {
        const ImageIndex index     = initImagesIterator.next();
        const Box cubeFlattenedBox = index.getType() == TextureType::CubeMap
                                         ? Box(area.x, area.y, 0, area.width, area.height, 1)
                                         : area;
        ANGLE_TRY(ensureSubImageInitialized(context, index, cubeFlattenedBox));
    }

    ANGLE_TRY(mTexture->clearSubImage(context, level, area, format, type, data));

    ANGLE_TRY(handleMipmapGenerationHint(context, level));

    onStateChange(angle::SubjectMessage::ContentsChanged);

    return angle::Result::Continue;
}

angle::Result Texture::bindTexImageFromSurface(Context *context, egl::Surface *surface)
{
    ASSERT(surface);
    ASSERT(!mBoundSurface);
    mBoundSurface = surface;

    // Set the image info to the size and format of the surface
    ASSERT(mState.mType == TextureType::_2D || mState.mType == TextureType::Rectangle);
    Extents size(surface->getWidth(), surface->getHeight(), 1);
    ImageDesc desc(size, surface->getBindTexImageFormat(), InitState::Initialized);
    mState.setImageDesc(NonCubeTextureTypeToTarget(mState.mType), 0, desc);
    mState.mHasProtectedContent = surface->hasProtectedContent();

    ANGLE_TRY(mTexture->bindTexImage(context, surface));

    signalDirtyStorage(InitState::Initialized);
    return angle::Result::Continue;
}

angle::Result Texture::releaseTexImageFromSurface(const Context *context)
{
    ASSERT(mBoundSurface);
    mBoundSurface = nullptr;
    ANGLE_TRY(mTexture->releaseTexImage(context));

    // Erase the image info for level 0
    ASSERT(mState.mType == TextureType::_2D || mState.mType == TextureType::Rectangle);
    mState.clearImageDesc(NonCubeTextureTypeToTarget(mState.mType), 0);
    mState.mHasProtectedContent = false;
    signalDirtyStorage(InitState::Initialized);
    return angle::Result::Continue;
}

void Texture::bindStream(egl::Stream *stream)
{
    ASSERT(stream);

    // It should not be possible to bind a texture already bound to another stream
    ASSERT(mBoundStream == nullptr);

    mBoundStream = stream;

    ASSERT(mState.mType == TextureType::External);
}

void Texture::releaseStream()
{
    ASSERT(mBoundStream);
    mBoundStream = nullptr;
}

angle::Result Texture::acquireImageFromStream(const Context *context,
                                              const egl::Stream::GLTextureDescription &desc)
{
    ASSERT(mBoundStream != nullptr);
    ANGLE_TRY(mTexture->setImageExternal(context, mState.mType, mBoundStream, desc));

    Extents size(desc.width, desc.height, 1);
    mState.setImageDesc(NonCubeTextureTypeToTarget(mState.mType), 0,
                        ImageDesc(size, Format(desc.internalFormat), InitState::Initialized));
    signalDirtyStorage(InitState::Initialized);
    return angle::Result::Continue;
}

angle::Result Texture::releaseImageFromStream(const Context *context)
{
    ASSERT(mBoundStream != nullptr);
    ANGLE_TRY(mTexture->setImageExternal(context, mState.mType, nullptr,
                                         egl::Stream::GLTextureDescription()));

    // Set to incomplete
    mState.clearImageDesc(NonCubeTextureTypeToTarget(mState.mType), 0);
    signalDirtyStorage(InitState::Initialized);
    return angle::Result::Continue;
}

angle::Result Texture::releaseTexImageInternal(Context *context)
{
    if (mBoundSurface)
    {
        // Notify the surface
        egl::Error eglErr = mBoundSurface->releaseTexImageFromTexture(context);
        // TODO(jmadill): Remove this once refactor is complete. http://anglebug.com/42261727
        if (eglErr.isError())
        {
            context->handleError(GL_INVALID_OPERATION, "Error releasing tex image from texture",
                                 __FILE__, ANGLE_FUNCTION, __LINE__);
        }

        // Then, call the same method as from the surface
        ANGLE_TRY(releaseTexImageFromSurface(context));
    }
    return angle::Result::Continue;
}

angle::Result Texture::setEGLImageTargetImpl(Context *context,
                                             TextureType type,
                                             GLuint levels,
                                             egl::Image *imageTarget)
{
    ASSERT(type == mState.mType);

    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    ANGLE_TRY(releaseTexImageInternal(context));

    egl::RefCountObjectReleaser<egl::Image> releaseImage;
    ANGLE_TRY(orphanImages(context, &releaseImage));

    setTargetImage(context, imageTarget);

    auto initState = imageTarget->sourceInitState();

    mState.clearImageDescs();
    mState.setImageDescChain(0, levels - 1, imageTarget->getExtents(), imageTarget->getFormat(),
                             initState);
    mState.mHasProtectedContent = imageTarget->hasProtectedContent();

    ANGLE_TRY(mTexture->setEGLImageTarget(context, type, imageTarget));

    signalDirtyStorage(initState);

    return angle::Result::Continue;
}

angle::Result Texture::setEGLImageTarget(Context *context,
                                         TextureType type,
                                         egl::Image *imageTarget)
{
    ASSERT(type == TextureType::_2D || type == TextureType::External ||
           type == TextureType::_2DArray);

    return setEGLImageTargetImpl(context, type, 1u, imageTarget);
}

angle::Result Texture::setStorageEGLImageTarget(Context *context,
                                                TextureType type,
                                                egl::Image *imageTarget,
                                                const GLint *attrib_list)
{
    ASSERT(type == TextureType::External || type == TextureType::_3D || type == TextureType::_2D ||
           type == TextureType::_2DArray || type == TextureType::CubeMap ||
           type == TextureType::CubeMapArray);

    ANGLE_TRY(setEGLImageTargetImpl(context, type, imageTarget->getLevelCount(), imageTarget));

    mState.mImmutableLevels = imageTarget->getLevelCount();
    mState.mImmutableFormat = true;

    // Changing the texture to immutable can trigger a change in the base and max levels:
    // GLES 3.0.4 section 3.8.10 pg 158:
    // "For immutable-format textures, levelbase is clamped to the range[0;levels],levelmax is then
    // clamped to the range[levelbase;levels].
    mDirtyBits.set(DIRTY_BIT_BASE_LEVEL);
    mDirtyBits.set(DIRTY_BIT_MAX_LEVEL);

    return angle::Result::Continue;
}

Extents Texture::getAttachmentSize(const ImageIndex &imageIndex) const
{
    // As an ImageIndex that represents an entire level of a cube map corresponds to 6 ImageDescs,
    // we only allow querying ImageDesc on a complete cube map, and this ImageDesc is exactly the
    // one that belongs to the first face of the cube map.
    if (imageIndex.isEntireLevelCubeMap())
    {
        // A cube map texture is cube complete if the following conditions all hold true:
        // - The levelbase arrays of each of the six texture images making up the cube map have
        //   identical, positive, and square dimensions.
        if (!mState.isCubeComplete())
        {
            return Extents();
        }
    }

    return mState.getImageDesc(imageIndex).size;
}

Format Texture::getAttachmentFormat(GLenum /*binding*/, const ImageIndex &imageIndex) const
{
    // As an ImageIndex that represents an entire level of a cube map corresponds to 6 ImageDescs,
    // we only allow querying ImageDesc on a complete cube map, and this ImageDesc is exactly the
    // one that belongs to the first face of the cube map.
    if (imageIndex.isEntireLevelCubeMap())
    {
        // A cube map texture is cube complete if the following conditions all hold true:
        // - The levelbase arrays were each specified with the same effective internal format.
        if (!mState.isCubeComplete())
        {
            return Format::Invalid();
        }
    }
    return mState.getImageDesc(imageIndex).format;
}

GLsizei Texture::getAttachmentSamples(const ImageIndex &imageIndex) const
{
    // We do not allow querying TextureTarget by an ImageIndex that represents an entire level of a
    // cube map (See comments in function TextureTypeToTarget() in ImageIndex.cpp).
    if (imageIndex.isEntireLevelCubeMap())
    {
        return 0;
    }

    return getSamples(imageIndex.getTarget(), imageIndex.getLevelIndex());
}

bool Texture::isRenderable(const Context *context,
                           GLenum binding,
                           const ImageIndex &imageIndex) const
{
    if (isEGLImageTarget())
    {
        return ImageSibling::isRenderable(context, binding, imageIndex);
    }

    // Surfaces bound to textures are always renderable. This avoids issues with surfaces with ES3+
    // formats not being renderable when bound to textures in ES2 contexts.
    if (mBoundSurface)
    {
        return true;
    }

    // Skip the renderability checks if it is set via glTexParameteri and current
    // context is less than GLES3. Note that we should not skip the check if the
    // texture is not renderable at all. Otherwise we would end up rendering to
    // textures like compressed textures that are not really renderable.
    if (context->getImplementation()
            ->getNativeTextureCaps()
            .get(getAttachmentFormat(binding, imageIndex).info->sizedInternalFormat)
            .textureAttachment &&
        !mState.renderabilityValidation() && context->getClientMajorVersion() < 3)
    {
        return true;
    }

    return getAttachmentFormat(binding, imageIndex)
        .info->textureAttachmentSupport(context->getClientVersion(), context->getExtensions());
}

bool Texture::getAttachmentFixedSampleLocations(const ImageIndex &imageIndex) const
{
    // We do not allow querying TextureTarget by an ImageIndex that represents an entire level of a
    // cube map (See comments in function TextureTypeToTarget() in ImageIndex.cpp).
    if (imageIndex.isEntireLevelCubeMap())
    {
        return true;
    }

    // ES3.1 (section 9.4) requires that the value of TEXTURE_FIXED_SAMPLE_LOCATIONS should be
    // the same for all attached textures.
    return getFixedSampleLocations(imageIndex.getTarget(), imageIndex.getLevelIndex());
}

void Texture::setBorderColor(const Context *context, const ColorGeneric &color)
{
    mState.mSamplerState.setBorderColor(color);
    signalDirtyState(DIRTY_BIT_BORDER_COLOR);
}

const ColorGeneric &Texture::getBorderColor() const
{
    return mState.mSamplerState.getBorderColor();
}

GLint Texture::getRequiredTextureImageUnits(const Context *context) const
{
    // Only external texture types can return non-1.
    if (mState.mType != TextureType::External)
    {
        return 1;
    }

    return mTexture->getRequiredExternalTextureImageUnits(context);
}

void Texture::setCrop(const Rectangle &rect)
{
    mState.setCrop(rect);
}

const Rectangle &Texture::getCrop() const
{
    return mState.getCrop();
}

void Texture::setGenerateMipmapHint(GLenum hint)
{
    mState.setGenerateMipmapHint(hint);
}

GLenum Texture::getGenerateMipmapHint() const
{
    return mState.getGenerateMipmapHint();
}

angle::Result Texture::setBuffer(const gl::Context *context,
                                 gl::Buffer *buffer,
                                 GLenum internalFormat)
{
    // Use 0 to indicate that the size is taken from whatever size the buffer has when the texture
    // buffer is used.
    return setBufferRange(context, buffer, internalFormat, 0, 0);
}

angle::Result Texture::setBufferRange(const gl::Context *context,
                                      gl::Buffer *buffer,
                                      GLenum internalFormat,
                                      GLintptr offset,
                                      GLsizeiptr size)
{
    mState.mImmutableFormat = true;
    mState.mBuffer.set(context, buffer, offset, size);
    ANGLE_TRY(mTexture->setBuffer(context, internalFormat));

    mState.clearImageDescs();
    if (buffer == nullptr)
    {
        mBufferObserver.reset();
        InitState initState = DetermineInitState(context, nullptr, nullptr);
        signalDirtyStorage(initState);
        return angle::Result::Continue;
    }

    size = GetBoundBufferAvailableSize(mState.mBuffer);

    mState.mImmutableLevels           = static_cast<GLuint>(1);
    InternalFormat internalFormatInfo = GetSizedInternalFormatInfo(internalFormat);
    Format format(internalFormat);
    Extents extents(static_cast<GLuint>(size / internalFormatInfo.pixelBytes), 1, 1);
    InitState initState = buffer->initState();
    mState.setImageDesc(TextureTarget::Buffer, 0, ImageDesc(extents, format, initState));

    signalDirtyStorage(initState);

    // Observe modifications to the buffer, so that extents can be updated.
    mBufferObserver.bind(buffer);

    return angle::Result::Continue;
}

const OffsetBindingPointer<Buffer> &Texture::getBuffer() const
{
    return mState.mBuffer;
}

void Texture::onAttach(const Context *context, rx::UniqueSerial framebufferSerial)
{
    addRef();

    // Duplicates allowed for multiple attachment points. See the comment in the header.
    mBoundFramebufferSerials.push_back(framebufferSerial);

    if (!mState.mHasBeenBoundAsAttachment)
    {
        mDirtyBits.set(DIRTY_BIT_BOUND_AS_ATTACHMENT);
        mState.mHasBeenBoundAsAttachment = true;
    }
}

void Texture::onDetach(const Context *context, rx::UniqueSerial framebufferSerial)
{
    // Erase first instance. If there are multiple bindings, leave the others.
    ASSERT(isBoundToFramebuffer(framebufferSerial));
    mBoundFramebufferSerials.remove_and_permute(framebufferSerial);

    release(context);
}

GLuint Texture::getId() const
{
    return id().value;
}

GLuint Texture::getNativeID() const
{
    return mTexture->getNativeID();
}

angle::Result Texture::syncState(const Context *context, Command source)
{
    ASSERT(hasAnyDirtyBit() || source == Command::GenerateMipmap);
    ANGLE_TRY(mTexture->syncState(context, mDirtyBits, source));
    mDirtyBits.reset();
    mState.mInitState = InitState::Initialized;
    return angle::Result::Continue;
}

rx::FramebufferAttachmentObjectImpl *Texture::getAttachmentImpl() const
{
    return mTexture;
}

bool Texture::isSamplerComplete(const Context *context, const Sampler *optionalSampler)
{
    const auto &samplerState =
        optionalSampler ? optionalSampler->getSamplerState() : mState.mSamplerState;
    const auto &contextState = context->getState();

    if (contextState.getContextID() != mCompletenessCache.context ||
        !mCompletenessCache.samplerState.sameCompleteness(samplerState))
    {
        mCompletenessCache.context      = context->getState().getContextID();
        mCompletenessCache.samplerState = samplerState;
        mCompletenessCache.samplerComplete =
            mState.computeSamplerCompleteness(samplerState, contextState);
    }

    return mCompletenessCache.samplerComplete;
}

// CopyImageSubData requires that we ignore format-based completeness rules
bool Texture::isSamplerCompleteForCopyImage(const Context *context,
                                            const Sampler *optionalSampler) const
{
    const gl::SamplerState &samplerState =
        optionalSampler ? optionalSampler->getSamplerState() : mState.mSamplerState;
    const gl::State &contextState = context->getState();
    return mState.computeSamplerCompletenessForCopyImage(samplerState, contextState);
}

Texture::SamplerCompletenessCache::SamplerCompletenessCache()
    : context({0}), samplerState(), samplerComplete(false)
{}

void Texture::invalidateCompletenessCache() const
{
    mCompletenessCache.context = {0};
}

angle::Result Texture::ensureInitialized(const Context *context)
{
    if (!context->isRobustResourceInitEnabled() || mState.mInitState == InitState::Initialized)
    {
        return angle::Result::Continue;
    }

    bool anyDirty = false;

    ImageIndexIterator it =
        ImageIndexIterator::MakeGeneric(mState.mType, 0, IMPLEMENTATION_MAX_TEXTURE_LEVELS + 1,
                                        ImageIndex::kEntireLevel, ImageIndex::kEntireLevel);
    while (it.hasNext())
    {
        const ImageIndex index = it.next();
        ImageDesc &desc =
            mState.mImageDescs[GetImageDescIndex(index.getTarget(), index.getLevelIndex())];
        if (desc.initState == InitState::MayNeedInit && !desc.size.empty())
        {
            ASSERT(mState.mInitState == InitState::MayNeedInit);
            ANGLE_TRY(initializeContents(context, GL_NONE, index));
            desc.initState = InitState::Initialized;
            anyDirty       = true;
        }
    }
    if (anyDirty)
    {
        signalDirtyStorage(InitState::Initialized);
    }
    mState.mInitState = InitState::Initialized;

    return angle::Result::Continue;
}

InitState Texture::initState(GLenum /*binding*/, const ImageIndex &imageIndex) const
{
    // As an ImageIndex that represents an entire level of a cube map corresponds to 6 ImageDescs,
    // we need to check all the related ImageDescs.
    if (imageIndex.isEntireLevelCubeMap())
    {
        const GLint levelIndex = imageIndex.getLevelIndex();
        for (TextureTarget cubeFaceTarget : AllCubeFaceTextureTargets())
        {
            if (mState.getImageDesc(cubeFaceTarget, levelIndex).initState == InitState::MayNeedInit)
            {
                return InitState::MayNeedInit;
            }
        }
        return InitState::Initialized;
    }

    return mState.getImageDesc(imageIndex).initState;
}

void Texture::setInitState(GLenum binding, const ImageIndex &imageIndex, InitState initState)
{
    // As an ImageIndex that represents an entire level of a cube map corresponds to 6 ImageDescs,
    // we need to update all the related ImageDescs.
    if (imageIndex.isEntireLevelCubeMap())
    {
        const GLint levelIndex = imageIndex.getLevelIndex();
        for (TextureTarget cubeFaceTarget : AllCubeFaceTextureTargets())
        {
            setInitState(binding, ImageIndex::MakeCubeMapFace(cubeFaceTarget, levelIndex),
                         initState);
        }
    }
    else
    {
        ImageDesc newDesc = mState.getImageDesc(imageIndex);
        newDesc.initState = initState;
        mState.setImageDesc(imageIndex.getTarget(), imageIndex.getLevelIndex(), newDesc);
    }
}

void Texture::setInitState(InitState initState)
{
    for (ImageDesc &imageDesc : mState.mImageDescs)
    {
        // Only modify defined images, undefined images will remain in the initialized state
        if (!imageDesc.size.empty())
        {
            imageDesc.initState = initState;
        }
    }
    mState.mInitState = initState;
}

bool Texture::isEGLImageSource(const ImageIndex &index) const
{
    for (const egl::Image *sourceImage : getSiblingSourcesOf())
    {
        if (sourceImage->getSourceImageIndex() == index)
        {
            return true;
        }
    }
    return false;
}

bool Texture::doesSubImageNeedInit(const Context *context,
                                   const ImageIndex &imageIndex,
                                   const Box &area) const
{
    if (!context->isRobustResourceInitEnabled() || mState.mInitState == InitState::Initialized)
    {
        return false;
    }

    // Pre-initialize the texture contents if necessary.
    const ImageDesc &desc = mState.getImageDesc(imageIndex);
    if (desc.initState != InitState::MayNeedInit)
    {
        return false;
    }

    ASSERT(mState.mInitState == InitState::MayNeedInit);
    return !area.coversSameExtent(desc.size);
}

angle::Result Texture::ensureSubImageInitialized(const Context *context,
                                                 const ImageIndex &imageIndex,
                                                 const Box &area)
{
    if (doesSubImageNeedInit(context, imageIndex, area))
    {
        // NOTE: do not optimize this to only initialize the passed area of the texture, or the
        // initialization logic in copySubImage will be incorrect.
        ANGLE_TRY(initializeContents(context, GL_NONE, imageIndex));
    }
    // Note: binding is ignored for textures.
    setInitState(GL_NONE, imageIndex, InitState::Initialized);
    return angle::Result::Continue;
}

angle::Result Texture::handleMipmapGenerationHint(Context *context, int level)
{
    if (getGenerateMipmapHint() == GL_TRUE && level == 0)
    {
        ANGLE_TRY(generateMipmap(context));
    }

    return angle::Result::Continue;
}

void Texture::onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message)
{
    switch (message)
    {
        case angle::SubjectMessage::ContentsChanged:
            if (index != kBufferSubjectIndex)
            {
                // ContentsChange originates from TextureStorage11::resolveAndReleaseTexture
                // which resolves the underlying multisampled texture if it exists and so
                // Texture will signal dirty storage to invalidate its own cache and the
                // attached framebuffer's cache.
                signalDirtyStorage(InitState::Initialized);
            }
            break;
        case angle::SubjectMessage::DirtyBitsFlagged:
            signalDirtyState(DIRTY_BIT_IMPLEMENTATION);

            // Notify siblings that we are dirty.
            if (index == rx::kTextureImageImplObserverMessageIndex)
            {
                notifySiblings(message);
            }
            break;
        case angle::SubjectMessage::SubjectChanged:
            mState.mInitState = InitState::MayNeedInit;
            signalDirtyState(DIRTY_BIT_IMPLEMENTATION);
            onStateChange(angle::SubjectMessage::ContentsChanged);

            // Notify siblings that we are dirty.
            if (index == rx::kTextureImageImplObserverMessageIndex)
            {
                notifySiblings(message);
            }
            else if (index == kBufferSubjectIndex)
            {
                const gl::Buffer *buffer = mState.mBuffer.get();
                ASSERT(buffer != nullptr);

                // Update cached image desc based on buffer size.
                GLsizeiptr size = GetBoundBufferAvailableSize(mState.mBuffer);

                ImageDesc desc          = mState.getImageDesc(TextureTarget::Buffer, 0);
                const GLuint pixelBytes = desc.format.info->pixelBytes;
                desc.size.width         = static_cast<GLuint>(size / pixelBytes);

                mState.setImageDesc(TextureTarget::Buffer, 0, desc);
            }
            break;
        case angle::SubjectMessage::StorageReleased:
            // When the TextureStorage is released, it needs to update the
            // RenderTargetCache of the Framebuffer attaching this Texture.
            // This is currently only for D3D back-end. See http://crbug.com/1234829
            if (index == rx::kTextureImageImplObserverMessageIndex)
            {
                onStateChange(angle::SubjectMessage::StorageReleased);
            }
            break;
        case angle::SubjectMessage::SubjectMapped:
        case angle::SubjectMessage::SubjectUnmapped:
        case angle::SubjectMessage::BindingChanged:
        {
            ASSERT(index == kBufferSubjectIndex);
            gl::Buffer *buffer = mState.mBuffer.get();
            ASSERT(buffer != nullptr);
            if (buffer->hasContentsObserver(this))
            {
                onBufferContentsChange();
            }
        }
        break;
        case angle::SubjectMessage::InitializationComplete:
            ASSERT(index == rx::kTextureImageImplObserverMessageIndex);
            setInitState(InitState::Initialized);
            break;
        case angle::SubjectMessage::InternalMemoryAllocationChanged:
            // Need to mark the texture dirty to give the back end a chance to handle the new
            // buffer. For example, the Vulkan back end needs to create a new buffer view that
            // points to the newly allocated buffer and update the texture descriptor set.
            signalDirtyState(DIRTY_BIT_IMPLEMENTATION);
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void Texture::onBufferContentsChange()
{
    mState.mInitState = InitState::MayNeedInit;
    signalDirtyState(DIRTY_BIT_IMPLEMENTATION);
    onStateChange(angle::SubjectMessage::ContentsChanged);
}

void Texture::onBindToMSRTTFramebuffer()
{
    if (!mState.mHasBeenBoundToMSRTTFramebuffer)
    {
        mDirtyBits.set(DIRTY_BIT_BOUND_TO_MSRTT_FRAMEBUFFER);
        mState.mHasBeenBoundToMSRTTFramebuffer = true;
    }
}

GLenum Texture::getImplementationColorReadFormat(const Context *context) const
{
    return mTexture->getColorReadFormat(context);
}

GLenum Texture::getImplementationColorReadType(const Context *context) const
{
    return mTexture->getColorReadType(context);
}

angle::Result Texture::getTexImage(const Context *context,
                                   const PixelPackState &packState,
                                   Buffer *packBuffer,
                                   TextureTarget target,
                                   GLint level,
                                   GLenum format,
                                   GLenum type,
                                   void *pixels)
{
    // No-op if the image level is empty.
    if (getExtents(target, level).empty())
    {
        return angle::Result::Continue;
    }

    return mTexture->getTexImage(context, packState, packBuffer, target, level, format, type,
                                 pixels);
}

angle::Result Texture::getCompressedTexImage(const Context *context,
                                             const PixelPackState &packState,
                                             Buffer *packBuffer,
                                             TextureTarget target,
                                             GLint level,
                                             void *pixels)
{
    // No-op if the image level is empty.
    if (getExtents(target, level).empty())
    {
        return angle::Result::Continue;
    }

    return mTexture->getCompressedTexImage(context, packState, packBuffer, target, level, pixels);
}

void Texture::onBindAsImageTexture()
{
    if (!mState.mHasBeenBoundAsImage)
    {
        mDirtyBits.set(DIRTY_BIT_BOUND_AS_IMAGE);
        mState.mHasBeenBoundAsImage = true;
    }
}

}  // namespace gl
