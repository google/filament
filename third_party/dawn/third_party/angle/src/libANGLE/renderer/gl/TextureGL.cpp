//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureGL.cpp: Implements the class methods for TextureGL.

#include "libANGLE/renderer/gl/TextureGL.h"

#include "common/bitset_utils.h"
#include "common/debug.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/MemoryObject.h"
#include "libANGLE/State.h"
#include "libANGLE/Surface.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/queryconversions.h"
#include "libANGLE/renderer/gl/BlitGL.h"
#include "libANGLE/renderer/gl/BufferGL.h"
#include "libANGLE/renderer/gl/ContextGL.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/ImageGL.h"
#include "libANGLE/renderer/gl/MemoryObjectGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/SurfaceGL.h"
#include "libANGLE/renderer/gl/formatutilsgl.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"
#include "platform/autogen/FeaturesGL_autogen.h"

using angle::CheckedNumeric;

namespace rx
{

namespace
{
// For use with the uploadTextureDataInChunks feature.  See http://crbug.com/1181068
constexpr const size_t kUploadTextureDataInChunksUploadSize = (120 * 1024) - 1;

size_t GetLevelInfoIndex(gl::TextureTarget target, size_t level)
{
    return gl::IsCubeMapFaceTarget(target)
               ? ((level * gl::kCubeFaceCount) + gl::CubeMapTextureTargetToFaceIndex(target))
               : level;
}

bool IsLUMAFormat(GLenum format)
{
    return format == GL_LUMINANCE || format == GL_ALPHA || format == GL_LUMINANCE_ALPHA;
}

LUMAWorkaroundGL GetLUMAWorkaroundInfo(GLenum originalFormat, GLenum destinationFormat)
{
    if (IsLUMAFormat(originalFormat))
    {
        return LUMAWorkaroundGL(!IsLUMAFormat(destinationFormat), destinationFormat);
    }
    else
    {
        return LUMAWorkaroundGL(false, GL_NONE);
    }
}

bool GetDepthStencilWorkaround(GLenum format)
{
    return format == GL_DEPTH_COMPONENT || format == GL_DEPTH_STENCIL;
}

bool GetEmulatedAlphaChannel(const angle::FeaturesGL &features,
                             const gl::InternalFormat &originalInternalFormat)
{
    return (features.RGBDXT1TexturesSampleZeroAlpha.enabled &&
            (originalInternalFormat.sizedInternalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ||
             originalInternalFormat.sizedInternalFormat == GL_COMPRESSED_SRGB_S3TC_DXT1_EXT)) ||
           (features.emulateRGB10.enabled && originalInternalFormat.format == GL_RGB &&
            originalInternalFormat.type == GL_UNSIGNED_INT_2_10_10_10_REV_EXT);
}

LevelInfoGL GetLevelInfo(const angle::FeaturesGL &features,
                         const gl::InternalFormat &originalInternalFormat,
                         GLenum destinationInternalFormat)
{
    GLenum destinationFormat = gl::GetUnsizedFormat(destinationInternalFormat);
    return LevelInfoGL(originalInternalFormat.format, destinationInternalFormat,
                       GetDepthStencilWorkaround(originalInternalFormat.format),
                       GetLUMAWorkaroundInfo(originalInternalFormat.format, destinationFormat),
                       GetEmulatedAlphaChannel(features, originalInternalFormat));
}

gl::Texture::DirtyBits GetLevelWorkaroundDirtyBits()
{
    gl::Texture::DirtyBits bits;
    bits.set(gl::Texture::DIRTY_BIT_SWIZZLE_RED);
    bits.set(gl::Texture::DIRTY_BIT_SWIZZLE_GREEN);
    bits.set(gl::Texture::DIRTY_BIT_SWIZZLE_BLUE);
    bits.set(gl::Texture::DIRTY_BIT_SWIZZLE_ALPHA);
    return bits;
}

size_t GetMaxLevelInfoCountForTextureType(gl::TextureType type)
{
    switch (type)
    {
        case gl::TextureType::CubeMap:
            return (gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS + 1) * gl::kCubeFaceCount;

        case gl::TextureType::External:
            return 1;

        default:
            return gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS + 1;
    }
}

bool FormatHasBorderColorWorkarounds(GLenum format)
{
    switch (format)
    {
        case GL_ALPHA:
        case GL_LUMINANCE_ALPHA:
            return true;
        default:
            return false;
    }
}

}  // anonymous namespace

LUMAWorkaroundGL::LUMAWorkaroundGL() : LUMAWorkaroundGL(false, GL_NONE) {}

LUMAWorkaroundGL::LUMAWorkaroundGL(bool enabled_, GLenum workaroundFormat_)
    : enabled(enabled_), workaroundFormat(workaroundFormat_)
{}

LevelInfoGL::LevelInfoGL() : LevelInfoGL(GL_NONE, GL_NONE, false, LUMAWorkaroundGL(), false) {}

LevelInfoGL::LevelInfoGL(GLenum sourceFormat_,
                         GLenum nativeInternalFormat_,
                         bool depthStencilWorkaround_,
                         const LUMAWorkaroundGL &lumaWorkaround_,
                         bool emulatedAlphaChannel_)
    : sourceFormat(sourceFormat_),
      nativeInternalFormat(nativeInternalFormat_),
      depthStencilWorkaround(depthStencilWorkaround_),
      lumaWorkaround(lumaWorkaround_),
      emulatedAlphaChannel(emulatedAlphaChannel_)
{}

TextureGL::TextureGL(const gl::TextureState &state, GLuint id)
    : TextureImpl(state),
      mAppliedSwizzle(state.getSwizzleState()),
      mAppliedSampler(state.getSamplerState()),
      mAppliedBaseLevel(state.getEffectiveBaseLevel()),
      mAppliedMaxLevel(state.getEffectiveMaxLevel()),
      mAppliedDepthStencilTextureMode(state.getDepthStencilTextureMode()),
      mTextureID(id)
{
    mLevelInfo.resize(GetMaxLevelInfoCountForTextureType(getType()));
}

TextureGL::~TextureGL()
{
    ASSERT(mTextureID == 0);
}

void TextureGL::onDestroy(const gl::Context *context)
{
    GetImplAs<ContextGL>(context)->flushIfNecessaryBeforeDeleteTextures();
    StateManagerGL *stateManager = GetStateManagerGL(context);
    stateManager->deleteTexture(mTextureID);
    mTextureID = 0;
}

angle::Result TextureGL::setImage(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  GLenum internalFormat,
                                  const gl::Extents &size,
                                  GLenum format,
                                  GLenum type,
                                  const gl::PixelUnpackState &unpack,
                                  gl::Buffer *unpackBuffer,
                                  const uint8_t *pixels)
{
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    gl::TextureTarget target = index.getTarget();
    size_t level             = static_cast<size_t>(index.getLevelIndex());

    if (features.unpackOverlappingRowsSeparatelyUnpackBuffer.enabled && unpackBuffer &&
        unpack.rowLength != 0 && unpack.rowLength < size.width)
    {
        // The rows overlap in unpack memory. Upload the texture row by row to work around
        // driver bug.
        ANGLE_TRY(
            reserveTexImageToBeFilled(context, target, level, internalFormat, size, format, type));

        if (size.width == 0 || size.height == 0 || size.depth == 0)
        {
            return angle::Result::Continue;
        }

        gl::Box area(0, 0, 0, size.width, size.height, size.depth);
        return setSubImageRowByRowWorkaround(context, target, level, area, format, type, unpack,
                                             unpackBuffer, 0, pixels);
    }

    if (features.unpackLastRowSeparatelyForPaddingInclusion.enabled)
    {
        bool apply = false;
        ANGLE_TRY(ShouldApplyLastRowPaddingWorkaround(
            GetImplAs<ContextGL>(context), size, unpack, unpackBuffer, format, type,
            nativegl::UseTexImage3D(getType()), pixels, &apply));

        // The driver will think the pixel buffer doesn't have enough data, work around this bug
        // by uploading the last row (and last level if 3D) separately.
        if (apply)
        {
            ANGLE_TRY(reserveTexImageToBeFilled(context, target, level, internalFormat, size,
                                                format, type));

            if (size.width == 0 || size.height == 0 || size.depth == 0)
            {
                return angle::Result::Continue;
            }

            gl::Box area(0, 0, 0, size.width, size.height, size.depth);
            return setSubImagePaddingWorkaround(context, target, level, area, format, type, unpack,
                                                unpackBuffer, pixels);
        }
    }

    ANGLE_TRY(setImageHelper(context, target, level, internalFormat, size, format, type, pixels));

    return angle::Result::Continue;
}

angle::Result TextureGL::setImageHelper(const gl::Context *context,
                                        gl::TextureTarget target,
                                        size_t level,
                                        GLenum internalFormat,
                                        const gl::Extents &size,
                                        GLenum format,
                                        GLenum type,
                                        const uint8_t *pixels)
{
    ASSERT(TextureTargetToType(target) == getType());

    const FunctionsGL *functions      = GetFunctionsGL(context);
    StateManagerGL *stateManager      = GetStateManagerGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    const gl::InternalFormat &originalInternalFormatInfo =
        gl::GetInternalFormatInfo(internalFormat, type);
    nativegl::TexImageFormat texImageFormat =
        nativegl::GetTexImageFormat(functions, features, internalFormat, format, type);

    stateManager->bindTexture(getType(), mTextureID);

    if (features.resetTexImage2DBaseLevel.enabled)
    {
        // setBaseLevel doesn't ever generate errors.
        (void)setBaseLevel(context, 0);
    }

    if (nativegl::UseTexImage2D(getType()))
    {
        ASSERT(size.depth == 1);
        ANGLE_GL_TRY_ALWAYS_CHECK(
            context, functions->texImage2D(nativegl::GetTextureBindingTarget(target),
                                           static_cast<GLint>(level), texImageFormat.internalFormat,
                                           size.width, size.height, 0, texImageFormat.format,
                                           texImageFormat.type, pixels));
    }
    else
    {
        ASSERT(nativegl::UseTexImage3D(getType()));
        ANGLE_GL_TRY_ALWAYS_CHECK(
            context, functions->texImage3D(ToGLenum(target), static_cast<GLint>(level),
                                           texImageFormat.internalFormat, size.width, size.height,
                                           size.depth, 0, texImageFormat.format,
                                           texImageFormat.type, pixels));
    }

    LevelInfoGL levelInfo =
        GetLevelInfo(features, originalInternalFormatInfo, texImageFormat.internalFormat);
    setLevelInfo(context, target, level, 1, levelInfo);

    if (features.setZeroLevelBeforeGenerateMipmap.enabled && getType() == gl::TextureType::_2D &&
        level != 0 && mLevelInfo[0].nativeInternalFormat == GL_NONE)
    {
        // Only fill level zero if it's possible that mipmaps can be generated with this format
        const gl::InternalFormat &internalFormatInfo =
            gl::GetInternalFormatInfo(internalFormat, type);
        if (!internalFormatInfo.sized ||
            (internalFormatInfo.filterSupport(context->getClientVersion(),
                                              context->getExtensions()) &&
             internalFormatInfo.textureAttachmentSupport(context->getClientVersion(),
                                                         context->getExtensions())))
        {
            ANGLE_GL_TRY_ALWAYS_CHECK(
                context,
                functions->texImage2D(nativegl::GetTextureBindingTarget(target), 0,
                                      texImageFormat.internalFormat, 1, 1, 0, texImageFormat.format,
                                      texImageFormat.type, nullptr));
            setLevelInfo(context, target, 0, 1, levelInfo);
        }
    }

    return angle::Result::Continue;
}

angle::Result TextureGL::reserveTexImageToBeFilled(const gl::Context *context,
                                                   gl::TextureTarget target,
                                                   size_t level,
                                                   GLenum internalFormat,
                                                   const gl::Extents &size,
                                                   GLenum format,
                                                   GLenum type)
{
    StateManagerGL *stateManager = GetStateManagerGL(context);
    ANGLE_TRY(stateManager->setPixelUnpackBuffer(context, nullptr));
    ANGLE_TRY(setImageHelper(context, target, level, internalFormat, size, format, type, nullptr));
    return angle::Result::Continue;
}

angle::Result TextureGL::setSubImage(const gl::Context *context,
                                     const gl::ImageIndex &index,
                                     const gl::Box &area,
                                     GLenum format,
                                     GLenum type,
                                     const gl::PixelUnpackState &unpack,
                                     gl::Buffer *unpackBuffer,
                                     const uint8_t *pixels)
{
    ASSERT(TextureTargetToType(index.getTarget()) == getType());

    ContextGL *contextGL              = GetImplAs<ContextGL>(context);
    const FunctionsGL *functions      = GetFunctionsGL(context);
    StateManagerGL *stateManager      = GetStateManagerGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    const gl::InternalFormat &originalInternalFormatInfo = *mState.getImageDesc(index).format.info;
    nativegl::TexSubImageFormat texSubImageFormat =
        nativegl::GetTexSubImageFormat(functions, features, format, type);

    gl::TextureTarget target = index.getTarget();
    size_t level             = static_cast<size_t>(index.getLevelIndex());

    ASSERT(getLevelInfo(target, level).lumaWorkaround.enabled ==
           GetLevelInfo(features, originalInternalFormatInfo, texSubImageFormat.format)
               .lumaWorkaround.enabled);

    stateManager->bindTexture(getType(), mTextureID);
    if (features.unpackOverlappingRowsSeparatelyUnpackBuffer.enabled && unpackBuffer &&
        unpack.rowLength != 0 && unpack.rowLength < area.width)
    {
        ANGLE_TRY(setSubImageRowByRowWorkaround(context, target, level, area, format, type, unpack,
                                                unpackBuffer, 0, pixels));
        contextGL->markWorkSubmitted();
        return angle::Result::Continue;
    }

    if (features.unpackLastRowSeparatelyForPaddingInclusion.enabled)
    {
        gl::Extents size(area.width, area.height, area.depth);

        bool apply = false;
        ANGLE_TRY(ShouldApplyLastRowPaddingWorkaround(
            GetImplAs<ContextGL>(context), size, unpack, unpackBuffer, format, type,
            nativegl::UseTexImage3D(getType()), pixels, &apply));

        // The driver will think the pixel buffer doesn't have enough data, work around this bug
        // by uploading the last row (and last level if 3D) separately.
        if (apply)
        {
            ANGLE_TRY(setSubImagePaddingWorkaround(context, target, level, area, format, type,
                                                   unpack, unpackBuffer, pixels));
            contextGL->markWorkSubmitted();
            return angle::Result::Continue;
        }
    }

    if (features.uploadTextureDataInChunks.enabled)
    {
        ANGLE_TRY(setSubImageRowByRowWorkaround(context, target, level, area, format, type, unpack,
                                                unpackBuffer, kUploadTextureDataInChunksUploadSize,
                                                pixels));
        contextGL->markWorkSubmitted();
        return angle::Result::Continue;
    }

    if (nativegl::UseTexImage2D(getType()))
    {
        ASSERT(area.z == 0 && area.depth == 1);
        ANGLE_GL_TRY(context,
                     functions->texSubImage2D(nativegl::GetTextureBindingTarget(target),
                                              static_cast<GLint>(level), area.x, area.y, area.width,
                                              area.height, texSubImageFormat.format,
                                              texSubImageFormat.type, pixels));
    }
    else
    {
        ASSERT(nativegl::UseTexImage3D(getType()));
        ANGLE_GL_TRY(context, functions->texSubImage3D(
                                  ToGLenum(target), static_cast<GLint>(level), area.x, area.y,
                                  area.z, area.width, area.height, area.depth,
                                  texSubImageFormat.format, texSubImageFormat.type, pixels));
    }

    contextGL->markWorkSubmitted();
    return angle::Result::Continue;
}

angle::Result TextureGL::setSubImageRowByRowWorkaround(const gl::Context *context,
                                                       gl::TextureTarget target,
                                                       size_t level,
                                                       const gl::Box &area,
                                                       GLenum format,
                                                       GLenum type,
                                                       const gl::PixelUnpackState &unpack,
                                                       const gl::Buffer *unpackBuffer,
                                                       size_t maxBytesUploadedPerChunk,
                                                       const uint8_t *pixels)
{
    ContextGL *contextGL              = GetImplAs<ContextGL>(context);
    const FunctionsGL *functions      = GetFunctionsGL(context);
    StateManagerGL *stateManager      = GetStateManagerGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    gl::PixelUnpackState directUnpack = unpack;
    directUnpack.skipRows             = 0;
    directUnpack.skipPixels           = 0;
    directUnpack.skipImages           = 0;
    ANGLE_TRY(stateManager->setPixelUnpackState(context, directUnpack));
    ANGLE_TRY(stateManager->setPixelUnpackBuffer(context, unpackBuffer));

    const gl::InternalFormat &glFormat = gl::GetInternalFormatInfo(format, type);
    GLuint rowBytes                    = 0;
    ANGLE_CHECK_GL_MATH(contextGL, glFormat.computeRowPitch(type, area.width, unpack.alignment,
                                                            unpack.rowLength, &rowBytes));
    GLuint imageBytes = 0;
    ANGLE_CHECK_GL_MATH(contextGL, glFormat.computeDepthPitch(area.height, unpack.imageHeight,
                                                              rowBytes, &imageBytes));

    bool useTexImage3D = nativegl::UseTexImage3D(getType());
    GLuint skipBytes   = 0;
    ANGLE_CHECK_GL_MATH(contextGL, glFormat.computeSkipBytes(type, rowBytes, imageBytes, unpack,
                                                             useTexImage3D, &skipBytes));

    GLint rowsPerChunk =
        std::min(std::max(static_cast<GLint>(maxBytesUploadedPerChunk / rowBytes), 1), area.height);
    if (maxBytesUploadedPerChunk > 0 && rowsPerChunk < area.height)
    {
        ANGLE_PERF_WARNING(contextGL->getDebug(), GL_DEBUG_SEVERITY_LOW,
                           "Chunking upload of texture data to work around driver hangs.");
    }

    nativegl::TexSubImageFormat texSubImageFormat =
        nativegl::GetTexSubImageFormat(functions, features, format, type);

    const uint8_t *pixelsWithSkip = pixels + skipBytes;
    if (useTexImage3D)
    {
        for (GLint image = 0; image < area.depth; ++image)
        {
            GLint imageByteOffset = image * imageBytes;
            for (GLint row = 0; row < area.height; row += rowsPerChunk)
            {
                GLint height             = std::min(rowsPerChunk, area.height - row);
                GLint byteOffset         = imageByteOffset + row * rowBytes;
                const GLubyte *rowPixels = pixelsWithSkip + byteOffset;
                ANGLE_GL_TRY(context,
                             functions->texSubImage3D(
                                 ToGLenum(target), static_cast<GLint>(level), area.x, row + area.y,
                                 image + area.z, area.width, height, 1, texSubImageFormat.format,
                                 texSubImageFormat.type, rowPixels));
            }
        }
    }
    else
    {
        ASSERT(nativegl::UseTexImage2D(getType()));
        for (GLint row = 0; row < area.height; row += rowsPerChunk)
        {
            GLint height             = std::min(rowsPerChunk, area.height - row);
            GLint byteOffset         = row * rowBytes;
            const GLubyte *rowPixels = pixelsWithSkip + byteOffset;
            ANGLE_GL_TRY(context, functions->texSubImage2D(
                                      ToGLenum(target), static_cast<GLint>(level), area.x,
                                      row + area.y, area.width, height, texSubImageFormat.format,
                                      texSubImageFormat.type, rowPixels));
        }
    }
    return angle::Result::Continue;
}

angle::Result TextureGL::setSubImagePaddingWorkaround(const gl::Context *context,
                                                      gl::TextureTarget target,
                                                      size_t level,
                                                      const gl::Box &area,
                                                      GLenum format,
                                                      GLenum type,
                                                      const gl::PixelUnpackState &unpack,
                                                      const gl::Buffer *unpackBuffer,
                                                      const uint8_t *pixels)
{
    ContextGL *contextGL         = GetImplAs<ContextGL>(context);
    const FunctionsGL *functions = GetFunctionsGL(context);
    StateManagerGL *stateManager = GetStateManagerGL(context);

    const gl::InternalFormat &glFormat = gl::GetInternalFormatInfo(format, type);
    GLuint rowBytes                    = 0;
    ANGLE_CHECK_GL_MATH(contextGL, glFormat.computeRowPitch(type, area.width, unpack.alignment,
                                                            unpack.rowLength, &rowBytes));
    GLuint imageBytes = 0;
    ANGLE_CHECK_GL_MATH(contextGL, glFormat.computeDepthPitch(area.height, unpack.imageHeight,
                                                              rowBytes, &imageBytes));
    bool useTexImage3D = nativegl::UseTexImage3D(getType());
    GLuint skipBytes   = 0;
    ANGLE_CHECK_GL_MATH(contextGL, glFormat.computeSkipBytes(type, rowBytes, imageBytes, unpack,
                                                             useTexImage3D, &skipBytes));

    ANGLE_TRY(stateManager->setPixelUnpackState(context, unpack));
    ANGLE_TRY(stateManager->setPixelUnpackBuffer(context, unpackBuffer));

    gl::PixelUnpackState directUnpack;
    directUnpack.alignment = 1;

    if (useTexImage3D)
    {
        // Upload all but the last slice
        if (area.depth > 1)
        {
            ANGLE_GL_TRY(context,
                         functions->texSubImage3D(ToGLenum(target), static_cast<GLint>(level),
                                                  area.x, area.y, area.z, area.width, area.height,
                                                  area.depth - 1, format, type, pixels));
        }

        // Upload the last slice but its last row
        if (area.height > 1)
        {
            // Do not include skipBytes in the last image pixel start offset as it will be done by
            // the driver
            GLint lastImageOffset          = (area.depth - 1) * imageBytes;
            const GLubyte *lastImagePixels = pixels + lastImageOffset;
            ANGLE_GL_TRY(context, functions->texSubImage3D(
                                      ToGLenum(target), static_cast<GLint>(level), area.x, area.y,
                                      area.z + area.depth - 1, area.width, area.height - 1, 1,
                                      format, type, lastImagePixels));
        }

        // Upload the last row of the last slice "manually"
        ANGLE_TRY(stateManager->setPixelUnpackState(context, directUnpack));

        GLint lastRowOffset =
            skipBytes + (area.depth - 1) * imageBytes + (area.height - 1) * rowBytes;
        const GLubyte *lastRowPixels = pixels + lastRowOffset;
        ANGLE_GL_TRY(context,
                     functions->texSubImage3D(ToGLenum(target), static_cast<GLint>(level), area.x,
                                              area.y + area.height - 1, area.z + area.depth - 1,
                                              area.width, 1, 1, format, type, lastRowPixels));
    }
    else
    {
        ASSERT(nativegl::UseTexImage2D(getType()));

        // Upload all but the last row
        if (area.height > 1)
        {
            ANGLE_GL_TRY(context, functions->texSubImage2D(
                                      ToGLenum(target), static_cast<GLint>(level), area.x, area.y,
                                      area.width, area.height - 1, format, type, pixels));
        }

        // Upload the last row "manually"
        ANGLE_TRY(stateManager->setPixelUnpackState(context, directUnpack));

        GLint lastRowOffset          = skipBytes + (area.height - 1) * rowBytes;
        const GLubyte *lastRowPixels = pixels + lastRowOffset;
        ANGLE_GL_TRY(context, functions->texSubImage2D(ToGLenum(target), static_cast<GLint>(level),
                                                       area.x, area.y + area.height - 1, area.width,
                                                       1, format, type, lastRowPixels));
    }

    return angle::Result::Continue;
}

angle::Result TextureGL::setCompressedImage(const gl::Context *context,
                                            const gl::ImageIndex &index,
                                            GLenum internalFormat,
                                            const gl::Extents &size,
                                            const gl::PixelUnpackState &unpack,
                                            size_t imageSize,
                                            const uint8_t *pixels)
{
    ContextGL *contextGL              = GetImplAs<ContextGL>(context);
    const FunctionsGL *functions      = GetFunctionsGL(context);
    StateManagerGL *stateManager      = GetStateManagerGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    gl::TextureTarget target = index.getTarget();
    size_t level             = static_cast<size_t>(index.getLevelIndex());
    ASSERT(TextureTargetToType(target) == getType());

    const gl::InternalFormat &originalInternalFormatInfo =
        gl::GetSizedInternalFormatInfo(internalFormat);
    nativegl::CompressedTexImageFormat compressedTexImageFormat =
        nativegl::GetCompressedTexImageFormat(functions, features, internalFormat);

    stateManager->bindTexture(getType(), mTextureID);
    ANGLE_TRY(stateManager->setPixelUnpackState(context, unpack));
    if (nativegl::UseTexImage2D(getType()))
    {
        ASSERT(size.depth == 1);
        ANGLE_GL_TRY_ALWAYS_CHECK(
            context, functions->compressedTexImage2D(ToGLenum(target), static_cast<GLint>(level),
                                                     compressedTexImageFormat.internalFormat,
                                                     size.width, size.height, 0,
                                                     static_cast<GLsizei>(imageSize), pixels));
    }
    else
    {
        ASSERT(nativegl::UseTexImage3D(getType()));

        ANGLE_GL_TRY_ALWAYS_CHECK(
            context, functions->compressedTexImage3D(ToGLenum(target), static_cast<GLint>(level),
                                                     compressedTexImageFormat.internalFormat,
                                                     size.width, size.height, size.depth, 0,
                                                     static_cast<GLsizei>(imageSize), pixels));
    }

    LevelInfoGL levelInfo =
        GetLevelInfo(features, originalInternalFormatInfo, compressedTexImageFormat.internalFormat);
    ASSERT(!levelInfo.lumaWorkaround.enabled);
    setLevelInfo(context, target, level, 1, levelInfo);

    contextGL->markWorkSubmitted();
    return angle::Result::Continue;
}

angle::Result TextureGL::setCompressedSubImage(const gl::Context *context,
                                               const gl::ImageIndex &index,
                                               const gl::Box &area,
                                               GLenum format,
                                               const gl::PixelUnpackState &unpack,
                                               size_t imageSize,
                                               const uint8_t *pixels)
{
    ContextGL *contextGL              = GetImplAs<ContextGL>(context);
    const FunctionsGL *functions      = GetFunctionsGL(context);
    StateManagerGL *stateManager      = GetStateManagerGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    gl::TextureTarget target = index.getTarget();
    size_t level             = static_cast<size_t>(index.getLevelIndex());
    ASSERT(TextureTargetToType(target) == getType());

    const gl::InternalFormat &originalInternalFormatInfo = gl::GetSizedInternalFormatInfo(format);
    nativegl::CompressedTexSubImageFormat compressedTexSubImageFormat =
        nativegl::GetCompressedSubTexImageFormat(functions, features, format);

    stateManager->bindTexture(getType(), mTextureID);
    ANGLE_TRY(stateManager->setPixelUnpackState(context, unpack));
    if (nativegl::UseTexImage2D(getType()))
    {
        ASSERT(area.z == 0 && area.depth == 1);
        ANGLE_GL_TRY(context, functions->compressedTexSubImage2D(
                                  ToGLenum(target), static_cast<GLint>(level), area.x, area.y,
                                  area.width, area.height, compressedTexSubImageFormat.format,
                                  static_cast<GLsizei>(imageSize), pixels));
    }
    else
    {
        ASSERT(nativegl::UseTexImage3D(getType()));
        ANGLE_GL_TRY(context,
                     functions->compressedTexSubImage3D(
                         ToGLenum(target), static_cast<GLint>(level), area.x, area.y, area.z,
                         area.width, area.height, area.depth, compressedTexSubImageFormat.format,
                         static_cast<GLsizei>(imageSize), pixels));
    }

    ASSERT(!getLevelInfo(target, level).lumaWorkaround.enabled &&
           !GetLevelInfo(features, originalInternalFormatInfo, compressedTexSubImageFormat.format)
                .lumaWorkaround.enabled);

    contextGL->markWorkSubmitted();
    return angle::Result::Continue;
}

angle::Result TextureGL::copyImage(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   const gl::Rectangle &sourceArea,
                                   GLenum internalFormat,
                                   gl::Framebuffer *source)
{
    ContextGL *contextGL              = GetImplAs<ContextGL>(context);
    const FunctionsGL *functions      = GetFunctionsGL(context);
    StateManagerGL *stateManager      = GetStateManagerGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    gl::TextureTarget target = index.getTarget();
    size_t level             = static_cast<size_t>(index.getLevelIndex());
    GLenum type              = source->getImplementationColorReadType(context);
    const gl::InternalFormat &originalInternalFormatInfo =
        gl::GetInternalFormatInfo(internalFormat, type);
    nativegl::CopyTexImageImageFormat copyTexImageFormat =
        nativegl::GetCopyTexImageImageFormat(functions, features, internalFormat, type);

    stateManager->bindTexture(getType(), mTextureID);

    const FramebufferGL *sourceFramebufferGL = GetImplAs<FramebufferGL>(source);
    gl::Extents fbSize = sourceFramebufferGL->getState().getReadAttachment()->getSize();

    // Did the read area go outside the framebuffer?
    bool outside = sourceArea.x < 0 || sourceArea.y < 0 ||
                   sourceArea.x + sourceArea.width > fbSize.width ||
                   sourceArea.y + sourceArea.height > fbSize.height;

    // TODO: Find a way to initialize the texture entirely in the gl level with ensureInitialized.
    // Right now there is no easy way to pre-fill the texture when it is being redefined with
    // partially uninitialized data.
    bool requiresInitialization =
        outside && (context->isRobustResourceInitEnabled() || context->isWebGL());

    // When robust resource initialization is enabled, the area outside the framebuffer must be
    // zeroed. We just zero the whole thing before copying into the area that overlaps the
    // framebuffer.
    if (requiresInitialization)
    {
        GLuint pixelBytes =
            gl::GetInternalFormatInfo(copyTexImageFormat.internalFormat, type).pixelBytes;
        angle::MemoryBuffer *zero;
        ANGLE_CHECK_GL_ALLOC(
            contextGL,
            context->getZeroFilledBuffer(sourceArea.width * sourceArea.height * pixelBytes, &zero));

        gl::PixelUnpackState unpack;
        unpack.alignment = 1;
        ANGLE_TRY(stateManager->setPixelUnpackState(context, unpack));
        ANGLE_TRY(stateManager->setPixelUnpackBuffer(context, nullptr));

        // getImplementationColorReadType aligns the type with ES client version
        if (type == GL_HALF_FLOAT_OES && functions->standard == STANDARD_GL_DESKTOP)
        {
            type = GL_HALF_FLOAT;
        }

        ANGLE_GL_TRY_ALWAYS_CHECK(
            context, functions->texImage2D(ToGLenum(target), static_cast<GLint>(level),
                                           copyTexImageFormat.internalFormat, sourceArea.width,
                                           sourceArea.height, 0,
                                           gl::GetUnsizedFormat(copyTexImageFormat.internalFormat),
                                           type, zero->data()));
    }

    // Clip source area to framebuffer and copy if remaining area is not empty.
    gl::Rectangle clippedArea;
    if (ClipRectangle(sourceArea, gl::Rectangle(0, 0, fbSize.width, fbSize.height), &clippedArea))
    {
        // If fbo's read buffer and the target texture are the same texture but different levels,
        // and if the read buffer is a non-base texture level, then implementations glTexImage2D
        // may change the target texture and make the original texture mipmap incomplete, which in
        // turn makes the fbo incomplete.
        // To avoid that, we clamp BASE_LEVEL and MAX_LEVEL to the same texture level as the fbo's
        // read buffer attachment. See http://crbug.com/797235
        const gl::FramebufferAttachment *readBuffer = source->getReadColorAttachment();
        if (readBuffer && readBuffer->type() == GL_TEXTURE)
        {
            TextureGL *sourceTexture = GetImplAs<TextureGL>(readBuffer->getTexture());
            if (sourceTexture && sourceTexture->mTextureID == mTextureID)
            {
                GLuint attachedTextureLevel = readBuffer->mipLevel();
                if (attachedTextureLevel != mState.getEffectiveBaseLevel())
                {
                    ANGLE_TRY(setBaseLevel(context, attachedTextureLevel));
                    ANGLE_TRY(setMaxLevel(context, attachedTextureLevel));
                }
            }
        }

        LevelInfoGL levelInfo =
            GetLevelInfo(features, originalInternalFormatInfo, copyTexImageFormat.internalFormat);
        gl::Offset destOffset(clippedArea.x - sourceArea.x, clippedArea.y - sourceArea.y, 0);

        if (levelInfo.lumaWorkaround.enabled)
        {
            BlitGL *blitter = GetBlitGL(context);

            if (requiresInitialization)
            {
                ANGLE_TRY(blitter->copySubImageToLUMAWorkaroundTexture(
                    context, mTextureID, getType(), target, levelInfo.sourceFormat, level,
                    destOffset, clippedArea, source));
            }
            else
            {
                ANGLE_TRY(blitter->copyImageToLUMAWorkaroundTexture(
                    context, mTextureID, getType(), target, levelInfo.sourceFormat, level,
                    clippedArea, copyTexImageFormat.internalFormat, source));
            }
        }
        else
        {
            ASSERT(nativegl::UseTexImage2D(getType()));
            stateManager->bindFramebuffer(GL_READ_FRAMEBUFFER,
                                          sourceFramebufferGL->getFramebufferID());
            if (features.emulateCopyTexImage2DFromRenderbuffers.enabled && readBuffer &&
                readBuffer->type() == GL_RENDERBUFFER)
            {
                BlitGL *blitter = GetBlitGL(context);
                ANGLE_TRY(blitter->blitColorBufferWithShader(
                    context, source, mTextureID, target, level, clippedArea,
                    gl::Rectangle(destOffset.x, destOffset.y, clippedArea.width,
                                  clippedArea.height),
                    GL_NEAREST, true));
            }
            else if (requiresInitialization)
            {
                ANGLE_GL_TRY(context, functions->copyTexSubImage2D(
                                          ToGLenum(target), static_cast<GLint>(level), destOffset.x,
                                          destOffset.y, clippedArea.x, clippedArea.y,
                                          clippedArea.width, clippedArea.height));
            }
            else
            {
                if (features.emulateCopyTexImage2D.enabled)
                {
                    if (type == GL_HALF_FLOAT_OES && functions->standard == STANDARD_GL_DESKTOP)
                    {
                        type = GL_HALF_FLOAT;
                    }

                    ANGLE_GL_TRY_ALWAYS_CHECK(
                        context,
                        functions->texImage2D(
                            ToGLenum(target), static_cast<GLint>(level),
                            copyTexImageFormat.internalFormat, sourceArea.width, sourceArea.height,
                            0, gl::GetUnsizedFormat(copyTexImageFormat.internalFormat), type,
                            nullptr));
                    ANGLE_GL_TRY_ALWAYS_CHECK(
                        context,
                        functions->copyTexSubImage2D(ToGLenum(target), static_cast<GLint>(level), 0,
                                                     0, sourceArea.x, sourceArea.y,
                                                     sourceArea.width, sourceArea.height));
                }
                else
                {
                    ANGLE_GL_TRY_ALWAYS_CHECK(
                        context, functions->copyTexImage2D(
                                     ToGLenum(target), static_cast<GLint>(level),
                                     copyTexImageFormat.internalFormat, sourceArea.x, sourceArea.y,
                                     sourceArea.width, sourceArea.height, 0));
                }
            }
        }
        setLevelInfo(context, target, level, 1, levelInfo);
    }

    if (features.flushBeforeDeleteTextureIfCopiedTo.enabled)
    {
        contextGL->setNeedsFlushBeforeDeleteTextures();
    }

    contextGL->markWorkSubmitted();
    return angle::Result::Continue;
}

angle::Result TextureGL::copySubImage(const gl::Context *context,
                                      const gl::ImageIndex &index,
                                      const gl::Offset &destOffset,
                                      const gl::Rectangle &sourceArea,
                                      gl::Framebuffer *source)
{
    ContextGL *contextGL              = GetImplAs<ContextGL>(context);
    const FunctionsGL *functions      = GetFunctionsGL(context);
    StateManagerGL *stateManager      = GetStateManagerGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    gl::TextureTarget target                 = index.getTarget();
    size_t level                             = static_cast<size_t>(index.getLevelIndex());
    const FramebufferGL *sourceFramebufferGL = GetImplAs<FramebufferGL>(source);

    // Clip source area to framebuffer.
    const gl::Extents fbSize = sourceFramebufferGL->getState().getReadAttachment()->getSize();
    gl::Rectangle clippedArea;
    if (!ClipRectangle(sourceArea, gl::Rectangle(0, 0, fbSize.width, fbSize.height), &clippedArea))
    {
        // nothing to do
        return angle::Result::Continue;
    }
    gl::Offset clippedOffset(destOffset.x + clippedArea.x - sourceArea.x,
                             destOffset.y + clippedArea.y - sourceArea.y, destOffset.z);

    stateManager->bindTexture(getType(), mTextureID);
    GLenum framebufferTarget =
        stateManager->getHasSeparateFramebufferBindings() ? GL_READ_FRAMEBUFFER : GL_FRAMEBUFFER;
    stateManager->bindFramebuffer(framebufferTarget, sourceFramebufferGL->getFramebufferID());

    const LevelInfoGL &levelInfo = getLevelInfo(target, level);
    if (levelInfo.lumaWorkaround.enabled)
    {
        BlitGL *blitter = GetBlitGL(context);
        ANGLE_TRY(blitter->copySubImageToLUMAWorkaroundTexture(
            context, mTextureID, getType(), target, levelInfo.sourceFormat, level, clippedOffset,
            clippedArea, source));
    }
    else
    {
        if (nativegl::UseTexImage2D(getType()))
        {
            ASSERT(clippedOffset.z == 0);
            if (features.emulateCopyTexImage2DFromRenderbuffers.enabled &&
                source->getReadColorAttachment() &&
                source->getReadColorAttachment()->type() == GL_RENDERBUFFER)
            {
                BlitGL *blitter = GetBlitGL(context);
                ANGLE_TRY(blitter->blitColorBufferWithShader(
                    context, source, mTextureID, target, level, clippedArea,
                    gl::Rectangle(clippedOffset.x, clippedOffset.y, clippedArea.width,
                                  clippedArea.height),
                    GL_NEAREST, true));
            }
            else
            {
                ANGLE_GL_TRY(context, functions->copyTexSubImage2D(
                                          ToGLenum(target), static_cast<GLint>(level),
                                          clippedOffset.x, clippedOffset.y, clippedArea.x,
                                          clippedArea.y, clippedArea.width, clippedArea.height));
            }
        }
        else
        {
            ASSERT(nativegl::UseTexImage3D(getType()));
            ANGLE_GL_TRY(context, functions->copyTexSubImage3D(
                                      ToGLenum(target), static_cast<GLint>(level), clippedOffset.x,
                                      clippedOffset.y, clippedOffset.z, clippedArea.x,
                                      clippedArea.y, clippedArea.width, clippedArea.height));
        }
    }

    if (features.flushBeforeDeleteTextureIfCopiedTo.enabled)
    {
        contextGL->setNeedsFlushBeforeDeleteTextures();
    }

    contextGL->markWorkSubmitted();
    return angle::Result::Continue;
}

angle::Result TextureGL::copyTexture(const gl::Context *context,
                                     const gl::ImageIndex &index,
                                     GLenum internalFormat,
                                     GLenum type,
                                     GLint sourceLevel,
                                     bool unpackFlipY,
                                     bool unpackPremultiplyAlpha,
                                     bool unpackUnmultiplyAlpha,
                                     const gl::Texture *source)
{
    gl::TextureTarget target  = index.getTarget();
    size_t level              = static_cast<size_t>(index.getLevelIndex());
    const TextureGL *sourceGL = GetImplAs<TextureGL>(source);
    const gl::ImageDesc &sourceImageDesc =
        sourceGL->mState.getImageDesc(NonCubeTextureTypeToTarget(source->getType()), sourceLevel);
    gl::Rectangle sourceArea(0, 0, sourceImageDesc.size.width, sourceImageDesc.size.height);

    ANGLE_TRY(reserveTexImageToBeFilled(context, target, level, internalFormat,
                                        sourceImageDesc.size, gl::GetUnsizedFormat(internalFormat),
                                        type));

    const gl::InternalFormat &destFormatInfo = gl::GetInternalFormatInfo(internalFormat, type);
    return copySubTextureHelper(context, target, level, gl::Offset(0, 0, 0), sourceLevel,
                                sourceArea, destFormatInfo, unpackFlipY, unpackPremultiplyAlpha,
                                unpackUnmultiplyAlpha, source);
}

angle::Result TextureGL::copySubTexture(const gl::Context *context,
                                        const gl::ImageIndex &index,
                                        const gl::Offset &destOffset,
                                        GLint sourceLevel,
                                        const gl::Box &sourceBox,
                                        bool unpackFlipY,
                                        bool unpackPremultiplyAlpha,
                                        bool unpackUnmultiplyAlpha,
                                        const gl::Texture *source)
{
    gl::TextureTarget target                 = index.getTarget();
    size_t level                             = static_cast<size_t>(index.getLevelIndex());
    const gl::InternalFormat &destFormatInfo = *mState.getImageDesc(target, level).format.info;
    return copySubTextureHelper(context, target, level, destOffset, sourceLevel, sourceBox.toRect(),
                                destFormatInfo, unpackFlipY, unpackPremultiplyAlpha,
                                unpackUnmultiplyAlpha, source);
}

angle::Result TextureGL::copySubTextureHelper(const gl::Context *context,
                                              gl::TextureTarget target,
                                              size_t level,
                                              const gl::Offset &destOffset,
                                              GLint sourceLevel,
                                              const gl::Rectangle &sourceArea,
                                              const gl::InternalFormat &destFormat,
                                              bool unpackFlipY,
                                              bool unpackPremultiplyAlpha,
                                              bool unpackUnmultiplyAlpha,
                                              const gl::Texture *source)
{
    ContextGL *contextGL              = GetImplAs<ContextGL>(context);
    const FunctionsGL *functions      = GetFunctionsGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);
    BlitGL *blitter                   = GetBlitGL(context);

    TextureGL *sourceGL = GetImplAs<TextureGL>(source);
    const gl::ImageDesc &sourceImageDesc =
        sourceGL->mState.getImageDesc(NonCubeTextureTypeToTarget(source->getType()), sourceLevel);

    if (features.flushBeforeDeleteTextureIfCopiedTo.enabled)
    {
        // Conservatively indicate that this workaround is necessary. Not clear
        // if it is on this code path, but added for symmetry.
        contextGL->setNeedsFlushBeforeDeleteTextures();
    }

    // Check is this is a simple copySubTexture that can be done with a copyTexSubImage
    ASSERT(sourceGL->getType() == gl::TextureType::_2D ||
           source->getType() == gl::TextureType::External ||
           source->getType() == gl::TextureType::Rectangle);
    const LevelInfoGL &sourceLevelInfo =
        sourceGL->getLevelInfo(NonCubeTextureTypeToTarget(source->getType()), sourceLevel);
    bool needsLumaWorkaround = sourceLevelInfo.lumaWorkaround.enabled;

    const gl::InternalFormat &sourceFormatInfo = *sourceImageDesc.format.info;
    GLenum sourceFormat                        = sourceFormatInfo.format;
    bool sourceFormatContainSupersetOfDestFormat =
        (sourceFormat == destFormat.format && sourceFormat != GL_BGRA_EXT) ||
        (sourceFormat == GL_RGBA && destFormat.format == GL_RGB);
    bool sourceSRGB = sourceFormatInfo.colorEncoding == GL_SRGB;

    GLenum sourceComponentType = sourceFormatInfo.componentType;
    GLenum destComponentType   = destFormat.componentType;
    bool destSRGB              = destFormat.colorEncoding == GL_SRGB;
    if (!unpackFlipY && unpackPremultiplyAlpha == unpackUnmultiplyAlpha && !needsLumaWorkaround &&
        sourceFormatContainSupersetOfDestFormat && sourceComponentType == destComponentType &&
        !destSRGB && !sourceSRGB && sourceGL->getType() == gl::TextureType::_2D)
    {
        bool copySucceeded = false;
        ANGLE_TRY(blitter->copyTexSubImage(context, sourceGL, sourceLevel, this, target, level,
                                           sourceArea, destOffset, &copySucceeded));
        if (copySucceeded)
        {
            contextGL->markWorkSubmitted();
            return angle::Result::Continue;
        }
    }

    // Check if the destination is renderable and copy on the GPU
    const LevelInfoGL &destLevelInfo = getLevelInfo(target, level);
    // todo(jonahr): http://crbug.com/773861
    // Behavior for now is to fallback to CPU readback implementation if the destination texture
    // is a luminance format. The correct solution is to handle both source and destination in the
    // luma workaround.
    if (!destSRGB && !destLevelInfo.lumaWorkaround.enabled &&
        nativegl::SupportsNativeRendering(functions, getType(), destLevelInfo.nativeInternalFormat))
    {
        bool copySucceeded = false;
        ANGLE_TRY(blitter->copySubTexture(
            context, sourceGL, sourceLevel, sourceComponentType, mTextureID, target, level,
            destComponentType, sourceImageDesc.size, sourceArea, destOffset, needsLumaWorkaround,
            sourceLevelInfo.sourceFormat, unpackFlipY, unpackPremultiplyAlpha,
            unpackUnmultiplyAlpha, sourceSRGB, &copySucceeded));
        if (copySucceeded)
        {
            contextGL->markWorkSubmitted();
            return angle::Result::Continue;
        }
    }

    // Fall back to CPU-readback
    ANGLE_TRY(blitter->copySubTextureCPUReadback(
        context, sourceGL, sourceLevel, sourceFormatInfo.sizedInternalFormat, this, target, level,
        destFormat.format, destFormat.type, sourceImageDesc.size, sourceArea, destOffset,
        needsLumaWorkaround, sourceLevelInfo.sourceFormat, unpackFlipY, unpackPremultiplyAlpha,
        unpackUnmultiplyAlpha));
    contextGL->markWorkSubmitted();
    return angle::Result::Continue;
}

angle::Result TextureGL::setStorage(const gl::Context *context,
                                    gl::TextureType type,
                                    size_t levels,
                                    GLenum internalFormat,
                                    const gl::Extents &size)
{
    ContextGL *contextGL              = GetImplAs<ContextGL>(context);
    const FunctionsGL *functions      = GetFunctionsGL(context);
    StateManagerGL *stateManager      = GetStateManagerGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    const gl::InternalFormat &originalInternalFormatInfo =
        gl::GetSizedInternalFormatInfo(internalFormat);
    nativegl::TexStorageFormat texStorageFormat =
        nativegl::GetTexStorageFormat(functions, features, internalFormat);

    stateManager->bindTexture(getType(), mTextureID);
    if (nativegl::UseTexImage2D(getType()))
    {
        ASSERT(size.depth == 1);
        if (functions->texStorage2D)
        {
            ANGLE_GL_TRY_ALWAYS_CHECK(
                context,
                functions->texStorage2D(ToGLenum(type), static_cast<GLsizei>(levels),
                                        texStorageFormat.internalFormat, size.width, size.height));
        }
        else
        {
            // Make sure no pixel unpack buffer is bound
            stateManager->bindBuffer(gl::BufferBinding::PixelUnpack, 0);

            const gl::InternalFormat &internalFormatInfo =
                gl::GetSizedInternalFormatInfo(internalFormat);

            // Internal format must be sized
            ASSERT(internalFormatInfo.sized);

            for (size_t level = 0; level < levels; level++)
            {
                gl::Extents levelSize(std::max(size.width >> level, 1),
                                      std::max(size.height >> level, 1), 1);

                if (getType() == gl::TextureType::_2D || getType() == gl::TextureType::Rectangle)
                {
                    if (internalFormatInfo.compressed)
                    {
                        nativegl::CompressedTexSubImageFormat compressedTexImageFormat =
                            nativegl::GetCompressedSubTexImageFormat(functions, features,
                                                                     internalFormat);

                        GLuint dataSize = 0;
                        ANGLE_CHECK_GL_MATH(
                            contextGL,
                            internalFormatInfo.computeCompressedImageSize(levelSize, &dataSize));
                        ANGLE_GL_TRY_ALWAYS_CHECK(
                            context,
                            functions->compressedTexImage2D(
                                ToGLenum(type), static_cast<GLint>(level),
                                compressedTexImageFormat.format, levelSize.width, levelSize.height,
                                0, static_cast<GLsizei>(dataSize), nullptr));
                    }
                    else
                    {
                        nativegl::TexImageFormat texImageFormat = nativegl::GetTexImageFormat(
                            functions, features, internalFormat, internalFormatInfo.format,
                            internalFormatInfo.type);

                        ANGLE_GL_TRY_ALWAYS_CHECK(
                            context,
                            functions->texImage2D(ToGLenum(type), static_cast<GLint>(level),
                                                  texImageFormat.internalFormat, levelSize.width,
                                                  levelSize.height, 0, texImageFormat.format,
                                                  texImageFormat.type, nullptr));
                    }
                }
                else
                {
                    ASSERT(getType() == gl::TextureType::CubeMap);
                    for (gl::TextureTarget face : gl::AllCubeFaceTextureTargets())
                    {
                        if (internalFormatInfo.compressed)
                        {
                            nativegl::CompressedTexSubImageFormat compressedTexImageFormat =
                                nativegl::GetCompressedSubTexImageFormat(functions, features,
                                                                         internalFormat);

                            GLuint dataSize = 0;
                            ANGLE_CHECK_GL_MATH(contextGL,
                                                internalFormatInfo.computeCompressedImageSize(
                                                    levelSize, &dataSize));
                            ANGLE_GL_TRY_ALWAYS_CHECK(
                                context,
                                functions->compressedTexImage2D(
                                    ToGLenum(face), static_cast<GLint>(level),
                                    compressedTexImageFormat.format, levelSize.width,
                                    levelSize.height, 0, static_cast<GLsizei>(dataSize), nullptr));
                        }
                        else
                        {
                            nativegl::TexImageFormat texImageFormat = nativegl::GetTexImageFormat(
                                functions, features, internalFormat, internalFormatInfo.format,
                                internalFormatInfo.type);

                            ANGLE_GL_TRY_ALWAYS_CHECK(
                                context, functions->texImage2D(
                                             ToGLenum(face), static_cast<GLint>(level),
                                             texImageFormat.internalFormat, levelSize.width,
                                             levelSize.height, 0, texImageFormat.format,
                                             texImageFormat.type, nullptr));
                        }
                    }
                }
            }
        }
    }
    else
    {
        ASSERT(nativegl::UseTexImage3D(getType()));
        const gl::InternalFormat &internalFormatInfo =
            gl::GetSizedInternalFormatInfo(internalFormat);
        const bool bypassTexStorage3D = type == gl::TextureType::_3D &&
                                        internalFormatInfo.compressed &&
                                        features.emulateImmutableCompressedTexture3D.enabled;
        if (functions->texStorage3D && !bypassTexStorage3D)
        {
            ANGLE_GL_TRY_ALWAYS_CHECK(
                context, functions->texStorage3D(ToGLenum(type), static_cast<GLsizei>(levels),
                                                 texStorageFormat.internalFormat, size.width,
                                                 size.height, size.depth));
        }
        else
        {
            // Make sure no pixel unpack buffer is bound
            stateManager->bindBuffer(gl::BufferBinding::PixelUnpack, 0);

            // Internal format must be sized
            ASSERT(internalFormatInfo.sized);

            for (GLsizei i = 0; i < static_cast<GLsizei>(levels); i++)
            {
                gl::Extents levelSize(
                    std::max(size.width >> i, 1), std::max(size.height >> i, 1),
                    getType() == gl::TextureType::_3D ? std::max(size.depth >> i, 1) : size.depth);

                if (internalFormatInfo.compressed)
                {
                    nativegl::CompressedTexSubImageFormat compressedTexImageFormat =
                        nativegl::GetCompressedSubTexImageFormat(functions, features,
                                                                 internalFormat);

                    GLuint dataSize = 0;
                    ANGLE_CHECK_GL_MATH(contextGL, internalFormatInfo.computeCompressedImageSize(
                                                       levelSize, &dataSize));
                    ANGLE_GL_TRY_ALWAYS_CHECK(
                        context, functions->compressedTexImage3D(
                                     ToGLenum(type), i, compressedTexImageFormat.format,
                                     levelSize.width, levelSize.height, levelSize.depth, 0,
                                     static_cast<GLsizei>(dataSize), nullptr));
                }
                else
                {
                    nativegl::TexImageFormat texImageFormat = nativegl::GetTexImageFormat(
                        functions, features, internalFormat, internalFormatInfo.format,
                        internalFormatInfo.type);

                    ANGLE_GL_TRY_ALWAYS_CHECK(
                        context,
                        functions->texImage3D(ToGLenum(type), i, texImageFormat.internalFormat,
                                              levelSize.width, levelSize.height, levelSize.depth, 0,
                                              texImageFormat.format, texImageFormat.type, nullptr));
                }
            }
        }
    }

    setLevelInfo(
        context, type, 0, levels,
        GetLevelInfo(features, originalInternalFormatInfo, texStorageFormat.internalFormat));

    return angle::Result::Continue;
}

angle::Result TextureGL::setImageExternal(const gl::Context *context,
                                          const gl::ImageIndex &index,
                                          GLenum internalFormat,
                                          const gl::Extents &size,
                                          GLenum format,
                                          GLenum type)
{
    const FunctionsGL *functions      = GetFunctionsGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    gl::TextureTarget target = index.getTarget();
    size_t level             = static_cast<size_t>(index.getLevelIndex());
    const gl::InternalFormat &originalInternalFormatInfo =
        gl::GetInternalFormatInfo(internalFormat, type);
    nativegl::TexImageFormat texImageFormat =
        nativegl::GetTexImageFormat(functions, features, internalFormat, format, type);

    setLevelInfo(context, target, level, 1,
                 GetLevelInfo(features, originalInternalFormatInfo, texImageFormat.internalFormat));
    return angle::Result::Continue;
}

angle::Result TextureGL::setStorageMultisample(const gl::Context *context,
                                               gl::TextureType type,
                                               GLsizei samples,
                                               GLint internalformat,
                                               const gl::Extents &size,
                                               bool fixedSampleLocations)
{
    const FunctionsGL *functions      = GetFunctionsGL(context);
    StateManagerGL *stateManager      = GetStateManagerGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    const gl::InternalFormat &originalInternalFormatInfo =
        gl::GetSizedInternalFormatInfo(internalformat);
    nativegl::TexStorageFormat texStorageFormat =
        nativegl::GetTexStorageFormat(functions, features, internalformat);

    stateManager->bindTexture(getType(), mTextureID);

    if (nativegl::UseTexImage2D(getType()))
    {
        ASSERT(size.depth == 1);
        if (functions->texStorage2DMultisample)
        {
            ANGLE_GL_TRY_ALWAYS_CHECK(
                context, functions->texStorage2DMultisample(
                             ToGLenum(type), samples, texStorageFormat.internalFormat, size.width,
                             size.height, gl::ConvertToGLBoolean(fixedSampleLocations)));
        }
        else
        {
            // texImage2DMultisample is similar to texStorage2DMultisample of es 3.1 core feature,
            // On macos and some old drivers which doesn't support OpenGL ES 3.1, the function can
            // be supported by ARB_texture_multisample or OpenGL 3.2 core feature.
            ANGLE_GL_TRY_ALWAYS_CHECK(
                context, functions->texImage2DMultisample(
                             ToGLenum(type), samples, texStorageFormat.internalFormat, size.width,
                             size.height, gl::ConvertToGLBoolean(fixedSampleLocations)));
        }
    }
    else
    {
        ASSERT(nativegl::UseTexImage3D(getType()));
        ANGLE_GL_TRY_ALWAYS_CHECK(
            context, functions->texStorage3DMultisample(
                         ToGLenum(type), samples, texStorageFormat.internalFormat, size.width,
                         size.height, size.depth, gl::ConvertToGLBoolean(fixedSampleLocations)));
    }

    setLevelInfo(
        context, type, 0, 1,
        GetLevelInfo(features, originalInternalFormatInfo, texStorageFormat.internalFormat));

    return angle::Result::Continue;
}

angle::Result TextureGL::setStorageExternalMemory(const gl::Context *context,
                                                  gl::TextureType type,
                                                  size_t levels,
                                                  GLenum internalFormat,
                                                  const gl::Extents &size,
                                                  gl::MemoryObject *memoryObject,
                                                  GLuint64 offset,
                                                  GLbitfield createFlags,
                                                  GLbitfield usageFlags,
                                                  const void *imageCreateInfoPNext)
{
    // GL_ANGLE_external_objects_flags not supported.
    ASSERT(createFlags == std::numeric_limits<uint32_t>::max());
    ASSERT(usageFlags == std::numeric_limits<uint32_t>::max());
    ASSERT(imageCreateInfoPNext == nullptr);

    const FunctionsGL *functions      = GetFunctionsGL(context);
    StateManagerGL *stateManager      = GetStateManagerGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    MemoryObjectGL *memoryObjectGL = GetImplAs<MemoryObjectGL>(memoryObject);

    const gl::InternalFormat &originalInternalFormatInfo =
        gl::GetSizedInternalFormatInfo(internalFormat);
    nativegl::TexStorageFormat texStorageFormat =
        nativegl::GetTexStorageFormat(functions, features, internalFormat);

    stateManager->bindTexture(getType(), mTextureID);
    if (nativegl::UseTexImage2D(getType()))
    {
        ANGLE_GL_TRY_ALWAYS_CHECK(
            context,
            functions->texStorageMem2DEXT(ToGLenum(type), static_cast<GLsizei>(levels),
                                          texStorageFormat.internalFormat, size.width, size.height,
                                          memoryObjectGL->getMemoryObjectID(), offset));
    }
    else
    {
        ASSERT(nativegl::UseTexImage3D(getType()));
        ANGLE_GL_TRY_ALWAYS_CHECK(
            context,
            functions->texStorageMem3DEXT(ToGLenum(type), static_cast<GLsizei>(levels),
                                          texStorageFormat.internalFormat, size.width, size.height,
                                          size.depth, memoryObjectGL->getMemoryObjectID(), offset));
    }

    setLevelInfo(
        context, type, 0, levels,
        GetLevelInfo(features, originalInternalFormatInfo, texStorageFormat.internalFormat));

    return angle::Result::Continue;
}

angle::Result TextureGL::setImageExternal(const gl::Context *context,
                                          gl::TextureType type,
                                          egl::Stream *stream,
                                          const egl::Stream::GLTextureDescription &desc)
{
    ANGLE_GL_UNREACHABLE(GetImplAs<ContextGL>(context));
    return angle::Result::Stop;
}

angle::Result TextureGL::generateMipmap(const gl::Context *context)
{
    ContextGL *contextGL              = GetImplAs<ContextGL>(context);
    const FunctionsGL *functions      = GetFunctionsGL(context);
    StateManagerGL *stateManager      = GetStateManagerGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    const GLuint effectiveBaseLevel = mState.getEffectiveBaseLevel();
    const GLuint maxLevel           = mState.getMipmapMaxLevel();

    const gl::ImageDesc &baseLevelDesc                = mState.getBaseLevelDesc();
    const gl::InternalFormat &baseLevelInternalFormat = *baseLevelDesc.format.info;

    const LevelInfoGL &baseLevelInfo = getBaseLevelInfo();

    stateManager->bindTexture(getType(), mTextureID);
    if (getType() == gl::TextureType::_2D &&
        ((baseLevelInternalFormat.colorEncoding == GL_SRGB &&
          features.decodeEncodeSRGBForGenerateMipmap.enabled) ||
         (features.useIntermediateTextureForGenerateMipmap.enabled &&
          nativegl::SupportsNativeRendering(functions, mState.getType(),
                                            baseLevelInfo.nativeInternalFormat))))
    {
        nativegl::TexImageFormat texImageFormat = nativegl::GetTexImageFormat(
            functions, features, baseLevelInternalFormat.internalFormat,
            baseLevelInternalFormat.format, baseLevelInternalFormat.type);

        // Manually allocate the mip levels of this texture if they don't exist
        GLuint levelCount = maxLevel - effectiveBaseLevel + 1;
        for (GLuint levelIdx = 1; levelIdx < levelCount; levelIdx++)
        {
            gl::Extents levelSize(std::max(baseLevelDesc.size.width >> levelIdx, 1),
                                  std::max(baseLevelDesc.size.height >> levelIdx, 1), 1);

            const gl::ImageDesc &levelDesc =
                mState.getImageDesc(gl::TextureTarget::_2D, effectiveBaseLevel + levelIdx);

            if (levelDesc.size != levelSize || *levelDesc.format.info != baseLevelInternalFormat)
            {
                // Make sure no pixel unpack buffer is bound
                stateManager->bindBuffer(gl::BufferBinding::PixelUnpack, 0);

                ANGLE_GL_TRY_ALWAYS_CHECK(
                    context, functions->texImage2D(
                                 ToGLenum(getType()), effectiveBaseLevel + levelIdx,
                                 texImageFormat.internalFormat, levelSize.width, levelSize.height,
                                 0, texImageFormat.format, texImageFormat.type, nullptr));
            }
        }

        // Use the blitter to generate the mips
        BlitGL *blitter = GetBlitGL(context);
        if (baseLevelInternalFormat.colorEncoding == GL_SRGB)
        {
            ANGLE_TRY(blitter->generateSRGBMipmap(context, this, effectiveBaseLevel, levelCount,
                                                  baseLevelDesc.size));
        }
        else
        {
            ANGLE_TRY(blitter->generateMipmap(context, this, effectiveBaseLevel, levelCount,
                                              baseLevelDesc.size, texImageFormat));
        }
    }
    else
    {
        ANGLE_GL_TRY_ALWAYS_CHECK(context, functions->generateMipmap(ToGLenum(getType())));
    }

    setLevelInfo(context, getType(), effectiveBaseLevel, maxLevel - effectiveBaseLevel,
                 getBaseLevelInfo());

    contextGL->markWorkSubmitted();
    return angle::Result::Continue;
}

angle::Result TextureGL::clearImage(const gl::Context *context,
                                    GLint level,
                                    GLenum format,
                                    GLenum type,
                                    const uint8_t *data)
{
    ContextGL *contextGL              = GetImplAs<ContextGL>(context);
    const FunctionsGL *functions      = GetFunctionsGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    nativegl::TexSubImageFormat texSubImageFormat =
        nativegl::GetTexSubImageFormat(functions, features, format, type);

    // Some drivers may use color mask state when clearing textures.
    contextGL->getStateManager()->setColorMask(true, true, true, true);

    ANGLE_GL_TRY(context, functions->clearTexImage(mTextureID, level, texSubImageFormat.format,
                                                   texSubImageFormat.type, data));

    contextGL->markWorkSubmitted();
    return angle::Result::Continue;
}

angle::Result TextureGL::clearSubImage(const gl::Context *context,
                                       GLint level,
                                       const gl::Box &area,
                                       GLenum format,
                                       GLenum type,
                                       const uint8_t *data)
{
    ContextGL *contextGL              = GetImplAs<ContextGL>(context);
    const FunctionsGL *functions      = GetFunctionsGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    // Some drivers may use color mask state when clearing textures.
    contextGL->getStateManager()->setColorMask(true, true, true, true);

    nativegl::TexSubImageFormat texSubImageFormat =
        nativegl::GetTexSubImageFormat(functions, features, format, type);
    ANGLE_GL_TRY(context, functions->clearTexSubImage(
                              mTextureID, level, area.x, area.y, area.z, area.width, area.height,
                              area.depth, texSubImageFormat.format, texSubImageFormat.type, data));

    contextGL->markWorkSubmitted();
    return angle::Result::Continue;
}

angle::Result TextureGL::bindTexImage(const gl::Context *context, egl::Surface *surface)
{
    ASSERT(getType() == gl::TextureType::_2D || getType() == gl::TextureType::Rectangle);

    StateManagerGL *stateManager = GetStateManagerGL(context);

    // Make sure this texture is bound
    stateManager->bindTexture(getType(), mTextureID);

    SurfaceGL *surfaceGL = GetImplAs<SurfaceGL>(surface);

    const gl::Format &surfaceFormat = surface->getBindTexImageFormat();
    setLevelInfo(context, getType(), 0, 1,
                 LevelInfoGL(surfaceFormat.info->format, surfaceFormat.info->internalFormat, false,
                             LUMAWorkaroundGL(), surfaceGL->hasEmulatedAlphaChannel()));
    return angle::Result::Continue;
}

angle::Result TextureGL::releaseTexImage(const gl::Context *context)
{
    ANGLE_TRY(recreateTexture(context));
    return angle::Result::Continue;
}

angle::Result TextureGL::setEGLImageTarget(const gl::Context *context,
                                           gl::TextureType type,
                                           egl::Image *image)
{
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    ImageGL *imageGL = GetImplAs<ImageGL>(image);

    GLenum imageNativeInternalFormat = GL_NONE;
    ANGLE_TRY(imageGL->setTexture2D(context, type, this, &imageNativeInternalFormat));

    const gl::InternalFormat &originalInternalFormatInfo = *image->getFormat().info;

    setLevelInfo(context, type, 0, 1,
                 GetLevelInfo(features, originalInternalFormatInfo, imageNativeInternalFormat));

    return angle::Result::Continue;
}

GLint TextureGL::getNativeID() const
{
    return mTextureID;
}

angle::Result TextureGL::syncState(const gl::Context *context,
                                   const gl::Texture::DirtyBits &dirtyBits,
                                   gl::Command source)
{
    if (dirtyBits.none() && mLocalDirtyBits.none())
    {
        return angle::Result::Continue;
    }

    const FunctionsGL *functions = GetFunctionsGL(context);
    StateManagerGL *stateManager = GetStateManagerGL(context);

    stateManager->bindTexture(getType(), mTextureID);

    gl::Texture::DirtyBits syncDirtyBits = dirtyBits | mLocalDirtyBits;
    if (dirtyBits[gl::Texture::DIRTY_BIT_BASE_LEVEL] || dirtyBits[gl::Texture::DIRTY_BIT_MAX_LEVEL])
    {
        // Don't know if the previous base level was using any workarounds, always re-sync the
        // workaround dirty bits
        syncDirtyBits |= GetLevelWorkaroundDirtyBits();

        // If the base level format has changed, depth stencil texture mode may need to be updated
        if (!mState.getImmutableFormat() && (context->getClientVersion() >= gl::ES_3_1 ||
                                             context->getExtensions().stencilTexturingANGLE))
        {
            syncDirtyBits.set(gl::Texture::DIRTY_BIT_DEPTH_STENCIL_TEXTURE_MODE);
        }

        // If the base level format has changed, border color may need to be updated
        if (!mState.getImmutableFormat() && (context->getClientVersion() >= gl::ES_3_2 ||
                                             context->getExtensions().textureBorderClampAny()))
        {
            syncDirtyBits.set(gl::Texture::DIRTY_BIT_BORDER_COLOR);
        }
    }
    for (auto dirtyBit : syncDirtyBits)
    {

        switch (dirtyBit)
        {
            case gl::Texture::DIRTY_BIT_MIN_FILTER:
                if (mAppliedSampler.setMinFilter(mState.getSamplerState().getMinFilter()))
                {
                    ANGLE_GL_TRY(context,
                                 functions->texParameteri(
                                     nativegl::GetTextureBindingTarget(getType()),
                                     GL_TEXTURE_MIN_FILTER, mAppliedSampler.getMinFilter()));
                }
                break;
            case gl::Texture::DIRTY_BIT_MAG_FILTER:
                if (mAppliedSampler.setMagFilter(mState.getSamplerState().getMagFilter()))
                {
                    ANGLE_GL_TRY(context,
                                 functions->texParameteri(
                                     nativegl::GetTextureBindingTarget(getType()),
                                     GL_TEXTURE_MAG_FILTER, mAppliedSampler.getMagFilter()));
                }
                break;
            case gl::Texture::DIRTY_BIT_WRAP_S:
                if (mAppliedSampler.setWrapS(mState.getSamplerState().getWrapS()))
                {
                    ANGLE_GL_TRY(context, functions->texParameteri(
                                              nativegl::GetTextureBindingTarget(getType()),
                                              GL_TEXTURE_WRAP_S, mAppliedSampler.getWrapS()));
                }
                break;
            case gl::Texture::DIRTY_BIT_WRAP_T:
                if (mAppliedSampler.setWrapT(mState.getSamplerState().getWrapT()))
                {
                    ANGLE_GL_TRY(context, functions->texParameteri(
                                              nativegl::GetTextureBindingTarget(getType()),
                                              GL_TEXTURE_WRAP_T, mAppliedSampler.getWrapT()));
                }
                break;
            case gl::Texture::DIRTY_BIT_WRAP_R:
                if (mAppliedSampler.setWrapR(mState.getSamplerState().getWrapR()))
                {
                    ANGLE_GL_TRY(context, functions->texParameteri(
                                              nativegl::GetTextureBindingTarget(getType()),
                                              GL_TEXTURE_WRAP_R, mAppliedSampler.getWrapR()));
                }
                break;
            case gl::Texture::DIRTY_BIT_MAX_ANISOTROPY:
                if (mAppliedSampler.setMaxAnisotropy(mState.getSamplerState().getMaxAnisotropy()))
                {
                    ANGLE_GL_TRY(context, functions->texParameterf(
                                              nativegl::GetTextureBindingTarget(getType()),
                                              GL_TEXTURE_MAX_ANISOTROPY_EXT,
                                              mAppliedSampler.getMaxAnisotropy()));
                }
                break;
            case gl::Texture::DIRTY_BIT_MIN_LOD:
                if (mAppliedSampler.setMinLod(mState.getSamplerState().getMinLod()))
                {
                    ANGLE_GL_TRY(context, functions->texParameterf(
                                              nativegl::GetTextureBindingTarget(getType()),
                                              GL_TEXTURE_MIN_LOD, mAppliedSampler.getMinLod()));
                }
                break;
            case gl::Texture::DIRTY_BIT_MAX_LOD:
                if (mAppliedSampler.setMaxLod(mState.getSamplerState().getMaxLod()))
                {
                    ANGLE_GL_TRY(context, functions->texParameterf(
                                              nativegl::GetTextureBindingTarget(getType()),
                                              GL_TEXTURE_MAX_LOD, mAppliedSampler.getMaxLod()));
                }
                break;
            case gl::Texture::DIRTY_BIT_COMPARE_MODE:
                if (mAppliedSampler.setCompareMode(mState.getSamplerState().getCompareMode()))
                {
                    ANGLE_GL_TRY(context,
                                 functions->texParameteri(
                                     nativegl::GetTextureBindingTarget(getType()),
                                     GL_TEXTURE_COMPARE_MODE, mAppliedSampler.getCompareMode()));
                }
                break;
            case gl::Texture::DIRTY_BIT_COMPARE_FUNC:
                if (mAppliedSampler.setCompareFunc(mState.getSamplerState().getCompareFunc()))
                {
                    ANGLE_GL_TRY(context,
                                 functions->texParameteri(
                                     nativegl::GetTextureBindingTarget(getType()),
                                     GL_TEXTURE_COMPARE_FUNC, mAppliedSampler.getCompareFunc()));
                }
                break;
            case gl::Texture::DIRTY_BIT_SRGB_DECODE:
                if (mAppliedSampler.setSRGBDecode(mState.getSamplerState().getSRGBDecode()))
                {
                    ANGLE_GL_TRY(context,
                                 functions->texParameteri(
                                     nativegl::GetTextureBindingTarget(getType()),
                                     GL_TEXTURE_SRGB_DECODE_EXT, mAppliedSampler.getSRGBDecode()));
                }
                break;
            case gl::Texture::DIRTY_BIT_BORDER_COLOR:
            {
                const LevelInfoGL &levelInfo    = getBaseLevelInfo();
                angle::ColorGeneric borderColor = mState.getSamplerState().getBorderColor();
                // Formats that have workarounds must be present in FormatHasBorderColorWorkarounds.
                if (levelInfo.sourceFormat == GL_ALPHA)
                {
                    if (levelInfo.lumaWorkaround.enabled)
                    {
                        ASSERT(levelInfo.lumaWorkaround.workaroundFormat == GL_RED);
                        borderColor.colorF.red = borderColor.colorF.alpha;
                    }
                    else
                    {
                        // Some ES drivers treat ALPHA as swizzled RG, triplicating
                        // border's red to RGB and sampling border's green as alpha.
                        borderColor.colorF.red   = 0.0f;
                        borderColor.colorF.green = borderColor.colorF.alpha;
                    }
                }
                else if (levelInfo.sourceFormat == GL_LUMINANCE_ALPHA)
                {
                    if (levelInfo.lumaWorkaround.enabled)
                    {
                        ASSERT(levelInfo.lumaWorkaround.workaroundFormat == GL_RG);
                    }
                    // When using desktop GL, this format is emulated as swizzled RG.
                    // Some ES drivers do the same without adjusting the border color.
                    borderColor.colorF.green = borderColor.colorF.alpha;
                }

                if (mAppliedSampler.setBorderColor(borderColor))
                {
                    switch (borderColor.type)
                    {
                        case angle::ColorGeneric::Type::Float:
                            ANGLE_GL_TRY(context,
                                         functions->texParameterfv(
                                             nativegl::GetTextureBindingTarget(getType()),
                                             GL_TEXTURE_BORDER_COLOR, &borderColor.colorF.red));
                            break;
                        case angle::ColorGeneric::Type::Int:
                            ANGLE_GL_TRY(context,
                                         functions->texParameterIiv(
                                             nativegl::GetTextureBindingTarget(getType()),
                                             GL_TEXTURE_BORDER_COLOR, &borderColor.colorI.red));
                            break;
                        case angle::ColorGeneric::Type::UInt:
                            ANGLE_GL_TRY(context,
                                         functions->texParameterIuiv(
                                             nativegl::GetTextureBindingTarget(getType()),
                                             GL_TEXTURE_BORDER_COLOR, &borderColor.colorUI.red));
                            break;
                        default:
                            UNREACHABLE();
                            break;
                    }
                }
                break;
            }

            // Texture state
            case gl::Texture::DIRTY_BIT_SWIZZLE_RED:
                ANGLE_TRY(syncTextureStateSwizzle(context, functions, GL_TEXTURE_SWIZZLE_R,
                                                  mState.getSwizzleState().swizzleRed,
                                                  &mAppliedSwizzle.swizzleRed));
                break;
            case gl::Texture::DIRTY_BIT_SWIZZLE_GREEN:
                ANGLE_TRY(syncTextureStateSwizzle(context, functions, GL_TEXTURE_SWIZZLE_G,
                                                  mState.getSwizzleState().swizzleGreen,
                                                  &mAppliedSwizzle.swizzleGreen));
                break;
            case gl::Texture::DIRTY_BIT_SWIZZLE_BLUE:
                ANGLE_TRY(syncTextureStateSwizzle(context, functions, GL_TEXTURE_SWIZZLE_B,
                                                  mState.getSwizzleState().swizzleBlue,
                                                  &mAppliedSwizzle.swizzleBlue));
                break;
            case gl::Texture::DIRTY_BIT_SWIZZLE_ALPHA:
                ANGLE_TRY(syncTextureStateSwizzle(context, functions, GL_TEXTURE_SWIZZLE_A,
                                                  mState.getSwizzleState().swizzleAlpha,
                                                  &mAppliedSwizzle.swizzleAlpha));
                break;
            case gl::Texture::DIRTY_BIT_BASE_LEVEL:
                if (mAppliedBaseLevel != mState.getEffectiveBaseLevel())
                {
                    mAppliedBaseLevel = mState.getEffectiveBaseLevel();
                    ANGLE_GL_TRY(context, functions->texParameteri(
                                              nativegl::GetTextureBindingTarget(getType()),
                                              GL_TEXTURE_BASE_LEVEL, mAppliedBaseLevel));
                }
                break;
            case gl::Texture::DIRTY_BIT_MAX_LEVEL:
                if (mAppliedMaxLevel != mState.getEffectiveMaxLevel())
                {
                    mAppliedMaxLevel = mState.getEffectiveMaxLevel();
                    ANGLE_GL_TRY(context, functions->texParameteri(
                                              nativegl::GetTextureBindingTarget(getType()),
                                              GL_TEXTURE_MAX_LEVEL, mAppliedMaxLevel));
                }
                break;
            case gl::Texture::DIRTY_BIT_DEPTH_STENCIL_TEXTURE_MODE:
            {
                ASSERT(context->getClientVersion() >= gl::ES_3_1 ||
                       context->getExtensions().stencilTexturingANGLE);

                // The DEPTH_STENCIL_TEXTURE_MODE state must affect only
                // DEPTH_STENCIL textures (OpenGL ES 3.2, Section 8.16).
                // Some drivers do not follow this rule and exhibit various side effects
                // when this mode is set to STENCIL_INDEX for textures of other formats.
                const GLenum mode = (getBaseLevelInfo().sourceFormat != GL_DEPTH_STENCIL)
                                        ? GL_DEPTH_COMPONENT
                                        : mState.getDepthStencilTextureMode();

                if (mAppliedDepthStencilTextureMode != mode)
                {
                    mAppliedDepthStencilTextureMode = mode;
                    ANGLE_GL_TRY(context, functions->texParameteri(
                                              nativegl::GetTextureBindingTarget(getType()),
                                              GL_DEPTH_STENCIL_TEXTURE_MODE, mode));
                }
                break;
            }
            case gl::Texture::DIRTY_BIT_USAGE:
            case gl::Texture::DIRTY_BIT_RENDERABILITY_VALIDATION_ANGLE:
                break;

            case gl::Texture::DIRTY_BIT_IMPLEMENTATION:
                // This special dirty bit is used to signal the front-end that the implementation
                // has local dirty bits. The real dirty bits are in mLocalDirty bits.
                break;
            case gl::Texture::DIRTY_BIT_BOUND_AS_IMAGE:
            case gl::Texture::DIRTY_BIT_BOUND_AS_ATTACHMENT:
            case gl::Texture::DIRTY_BIT_BOUND_TO_MSRTT_FRAMEBUFFER:
                // Only used for Vulkan.
                break;

            default:
                UNREACHABLE();
        }
    }

    mAllModifiedDirtyBits |= syncDirtyBits;
    mLocalDirtyBits.reset();
    return angle::Result::Continue;
}

bool TextureGL::hasAnyDirtyBit() const
{
    return mLocalDirtyBits.any();
}

angle::Result TextureGL::setBaseLevel(const gl::Context *context, GLuint baseLevel)
{
    if (baseLevel != mAppliedBaseLevel)
    {
        const FunctionsGL *functions = GetFunctionsGL(context);
        StateManagerGL *stateManager = GetStateManagerGL(context);

        mAppliedBaseLevel = baseLevel;
        mLocalDirtyBits.set(gl::Texture::DIRTY_BIT_BASE_LEVEL);

        // Signal to the GL layer that the Impl has dirty bits.
        onStateChange(angle::SubjectMessage::DirtyBitsFlagged);

        stateManager->bindTexture(getType(), mTextureID);
        ANGLE_GL_TRY(context, functions->texParameteri(ToGLenum(getType()), GL_TEXTURE_BASE_LEVEL,
                                                       baseLevel));
    }
    return angle::Result::Continue;
}

angle::Result TextureGL::setMaxLevel(const gl::Context *context, GLuint maxLevel)
{
    if (maxLevel != mAppliedMaxLevel)
    {
        const FunctionsGL *functions = GetFunctionsGL(context);
        StateManagerGL *stateManager = GetStateManagerGL(context);

        mAppliedMaxLevel = maxLevel;
        mLocalDirtyBits.set(gl::Texture::DIRTY_BIT_MAX_LEVEL);

        // Signal to the GL layer that the Impl has dirty bits.
        onStateChange(angle::SubjectMessage::DirtyBitsFlagged);

        stateManager->bindTexture(getType(), mTextureID);
        ANGLE_GL_TRY(context,
                     functions->texParameteri(ToGLenum(getType()), GL_TEXTURE_MAX_LEVEL, maxLevel));
    }
    return angle::Result::Continue;
}

angle::Result TextureGL::setMinFilter(const gl::Context *context, GLenum filter)
{
    if (mAppliedSampler.setMinFilter(filter))
    {
        const FunctionsGL *functions = GetFunctionsGL(context);
        StateManagerGL *stateManager = GetStateManagerGL(context);

        mLocalDirtyBits.set(gl::Texture::DIRTY_BIT_MIN_FILTER);

        // Signal to the GL layer that the Impl has dirty bits.
        onStateChange(angle::SubjectMessage::DirtyBitsFlagged);

        stateManager->bindTexture(getType(), mTextureID);
        ANGLE_GL_TRY(context,
                     functions->texParameteri(ToGLenum(getType()), GL_TEXTURE_MIN_FILTER, filter));
    }
    return angle::Result::Continue;
}
angle::Result TextureGL::setMagFilter(const gl::Context *context, GLenum filter)
{
    if (mAppliedSampler.setMagFilter(filter))
    {
        const FunctionsGL *functions = GetFunctionsGL(context);
        StateManagerGL *stateManager = GetStateManagerGL(context);

        mLocalDirtyBits.set(gl::Texture::DIRTY_BIT_MAG_FILTER);

        // Signal to the GL layer that the Impl has dirty bits.
        onStateChange(angle::SubjectMessage::DirtyBitsFlagged);

        stateManager->bindTexture(getType(), mTextureID);
        ANGLE_GL_TRY(context,
                     functions->texParameteri(ToGLenum(getType()), GL_TEXTURE_MAG_FILTER, filter));
    }
    return angle::Result::Continue;
}

angle::Result TextureGL::setSwizzle(const gl::Context *context, GLint swizzle[4])
{
    gl::SwizzleState resultingSwizzle =
        gl::SwizzleState(swizzle[0], swizzle[1], swizzle[2], swizzle[3]);

    if (resultingSwizzle != mAppliedSwizzle)
    {
        const FunctionsGL *functions = GetFunctionsGL(context);
        StateManagerGL *stateManager = GetStateManagerGL(context);

        mAppliedSwizzle = resultingSwizzle;
        mLocalDirtyBits.set(gl::Texture::DIRTY_BIT_SWIZZLE_RED);
        mLocalDirtyBits.set(gl::Texture::DIRTY_BIT_SWIZZLE_GREEN);
        mLocalDirtyBits.set(gl::Texture::DIRTY_BIT_SWIZZLE_BLUE);
        mLocalDirtyBits.set(gl::Texture::DIRTY_BIT_SWIZZLE_ALPHA);

        // Signal to the GL layer that the Impl has dirty bits.
        onStateChange(angle::SubjectMessage::DirtyBitsFlagged);

        stateManager->bindTexture(getType(), mTextureID);
        if (functions->standard == STANDARD_GL_ES)
        {
            ANGLE_GL_TRY(context, functions->texParameteri(ToGLenum(getType()),
                                                           GL_TEXTURE_SWIZZLE_R, swizzle[0]));
            ANGLE_GL_TRY(context, functions->texParameteri(ToGLenum(getType()),
                                                           GL_TEXTURE_SWIZZLE_G, swizzle[1]));
            ANGLE_GL_TRY(context, functions->texParameteri(ToGLenum(getType()),
                                                           GL_TEXTURE_SWIZZLE_B, swizzle[2]));
            ANGLE_GL_TRY(context, functions->texParameteri(ToGLenum(getType()),
                                                           GL_TEXTURE_SWIZZLE_A, swizzle[3]));
        }
        else
        {
            ANGLE_GL_TRY(context, functions->texParameteriv(ToGLenum(getType()),
                                                            GL_TEXTURE_SWIZZLE_RGBA, swizzle));
        }
    }
    return angle::Result::Continue;
}

angle::Result TextureGL::setBuffer(const gl::Context *context, GLenum internalFormat)
{
    const FunctionsGL *functions                              = GetFunctionsGL(context);
    const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding = mState.getBuffer();
    const gl::Buffer *buffer                                  = bufferBinding.get();
    const GLintptr offset                                     = bufferBinding.getOffset();
    const GLsizeiptr size                                     = bufferBinding.getSize();
    const GLuint bufferID = buffer ? GetImplAs<BufferGL>(buffer)->getBufferID() : 0;

    // If buffer is not bound, use texBuffer to unbind it.  If size is 0, texBuffer was used to
    // create this binding, so use the same function.  This will allow the implementation to take
    // the current size of the buffer on every draw/dispatch call even if the buffer size changes.
    if (buffer == nullptr || size == 0)
    {
        ANGLE_GL_TRY(context, functions->texBuffer(GL_TEXTURE_BUFFER, internalFormat, bufferID));
    }
    else
    {
        ANGLE_GL_TRY(context,
                     functions->texBufferRange(GL_TEXTURE_BUFFER, internalFormat, bufferID, offset,
                                               GetBoundBufferAvailableSize(bufferBinding)));
    }

    return angle::Result::Continue;
}

GLenum TextureGL::getNativeInternalFormat(const gl::ImageIndex &index) const
{
    return getLevelInfo(index.getTarget(), index.getLevelIndex()).nativeInternalFormat;
}

bool TextureGL::hasEmulatedAlphaChannel(const gl::ImageIndex &index) const
{
    return getLevelInfo(index.getTargetOrFirstCubeFace(), index.getLevelIndex())
        .emulatedAlphaChannel;
}

angle::Result TextureGL::recreateTexture(const gl::Context *context)
{
    const FunctionsGL *functions = GetFunctionsGL(context);
    StateManagerGL *stateManager = GetStateManagerGL(context);

    stateManager->bindTexture(getType(), mTextureID);
    stateManager->deleteTexture(mTextureID);

    functions->genTextures(1, &mTextureID);
    stateManager->bindTexture(getType(), mTextureID);

    mLevelInfo.clear();
    mLevelInfo.resize(GetMaxLevelInfoCountForTextureType(getType()));

    mAppliedSwizzle = gl::SwizzleState();
    mAppliedSampler = gl::SamplerState::CreateDefaultForTarget(getType());

    mAppliedBaseLevel = 0;
    mAppliedBaseLevel = gl::kInitialMaxLevel;

    mLocalDirtyBits = mAllModifiedDirtyBits;

    onStateChange(angle::SubjectMessage::SubjectChanged);

    return angle::Result::Continue;
}

angle::Result TextureGL::syncTextureStateSwizzle(const gl::Context *context,
                                                 const FunctionsGL *functions,
                                                 GLenum name,
                                                 GLenum value,
                                                 GLenum *currentlyAppliedValue)
{
    const LevelInfoGL &levelInfo = getBaseLevelInfo();
    GLenum resultSwizzle         = value;
    if (levelInfo.lumaWorkaround.enabled)
    {
        switch (value)
        {
            case GL_RED:
            case GL_GREEN:
            case GL_BLUE:
                if (levelInfo.sourceFormat == GL_LUMINANCE ||
                    levelInfo.sourceFormat == GL_LUMINANCE_ALPHA)
                {
                    // Texture is backed by a RED or RG texture, point all color channels at the
                    // red channel.
                    ASSERT(levelInfo.lumaWorkaround.workaroundFormat == GL_RED ||
                           levelInfo.lumaWorkaround.workaroundFormat == GL_RG);
                    resultSwizzle = GL_RED;
                }
                else
                {
                    ASSERT(levelInfo.sourceFormat == GL_ALPHA);
                    // Color channels are not supposed to exist, make them always sample 0.
                    resultSwizzle = GL_ZERO;
                }
                break;

            case GL_ALPHA:
                if (levelInfo.sourceFormat == GL_LUMINANCE)
                {
                    // Alpha channel is not supposed to exist, make it always sample 1.
                    resultSwizzle = GL_ONE;
                }
                else if (levelInfo.sourceFormat == GL_ALPHA)
                {
                    // Texture is backed by a RED texture, point the alpha channel at the red
                    // channel.
                    ASSERT(levelInfo.lumaWorkaround.workaroundFormat == GL_RED);
                    resultSwizzle = GL_RED;
                }
                else
                {
                    ASSERT(levelInfo.sourceFormat == GL_LUMINANCE_ALPHA);
                    // Texture is backed by an RG texture, point the alpha channel at the green
                    // channel.
                    ASSERT(levelInfo.lumaWorkaround.workaroundFormat == GL_RG);
                    resultSwizzle = GL_GREEN;
                }
                break;

            case GL_ZERO:
            case GL_ONE:
                // Don't modify the swizzle state when requesting ZERO or ONE.
                resultSwizzle = value;
                break;

            default:
                UNREACHABLE();
                break;
        }
    }
    else if (levelInfo.depthStencilWorkaround)
    {
        switch (value)
        {
            case GL_RED:
                // Don't modify the swizzle state when requesting the red channel.
                resultSwizzle = value;
                break;

            case GL_GREEN:
            case GL_BLUE:
                if (context->getClientMajorVersion() <= 2)
                {
                    // In OES_depth_texture/ARB_depth_texture, depth
                    // textures are treated as luminance.
                    resultSwizzle = GL_RED;
                }
                else
                {
                    // In GLES 3.0, depth textures are treated as RED
                    // textures, so green and blue should be 0.
                    resultSwizzle = GL_ZERO;
                }
                break;

            case GL_ALPHA:
                // Depth textures should sample 1 from the alpha channel.
                resultSwizzle = GL_ONE;
                break;

            case GL_ZERO:
            case GL_ONE:
                // Don't modify the swizzle state when requesting ZERO or ONE.
                resultSwizzle = value;
                break;

            default:
                UNREACHABLE();
                break;
        }
    }
    else if (levelInfo.emulatedAlphaChannel)
    {
        if (value == GL_ALPHA)
        {
            resultSwizzle = GL_ONE;
        }
    }

    if (*currentlyAppliedValue != resultSwizzle)
    {
        *currentlyAppliedValue = resultSwizzle;
        ANGLE_GL_TRY(context, functions->texParameteri(ToGLenum(getType()), name, resultSwizzle));
    }

    return angle::Result::Continue;
}

void TextureGL::setLevelInfo(const gl::Context *context,
                             gl::TextureTarget target,
                             size_t level,
                             size_t levelCount,
                             const LevelInfoGL &levelInfo)
{
    ASSERT(levelCount > 0);

    bool updateWorkarounds = levelInfo.depthStencilWorkaround || levelInfo.lumaWorkaround.enabled ||
                             levelInfo.emulatedAlphaChannel;

    bool updateDepthStencilTextureMode = false;
    const bool setToDepthStencil       = levelInfo.sourceFormat == GL_DEPTH_STENCIL;

    bool updateBorderColor = false;
    const bool targetFormatHasBorderColorWorkarounds =
        FormatHasBorderColorWorkarounds(levelInfo.sourceFormat);

    for (size_t i = level; i < level + levelCount; i++)
    {
        size_t index = GetLevelInfoIndex(target, i);
        ASSERT(index < mLevelInfo.size());
        auto &curLevelInfo = mLevelInfo[index];

        updateWorkarounds |= curLevelInfo.depthStencilWorkaround;
        updateWorkarounds |= curLevelInfo.lumaWorkaround.enabled;
        updateWorkarounds |= curLevelInfo.emulatedAlphaChannel;

        // When redefining a level to or from DEPTH_STENCIL
        // format, ensure that the texture mode is synced.
        const bool setFromDepthStencil = curLevelInfo.sourceFormat == GL_DEPTH_STENCIL;
        if (setFromDepthStencil != setToDepthStencil)
        {
            updateDepthStencilTextureMode = true;
        }

        // When redefining a level to or from a format that has border color workarounds,
        // ensure that the texture border color is synced.
        if (FormatHasBorderColorWorkarounds(curLevelInfo.sourceFormat) ||
            targetFormatHasBorderColorWorkarounds)
        {
            updateBorderColor = true;
        }

        curLevelInfo = levelInfo;
    }

    // Skip this step when unsupported
    updateDepthStencilTextureMode =
        updateDepthStencilTextureMode && (context->getClientVersion() >= gl::ES_3_1 ||
                                          context->getExtensions().stencilTexturingANGLE);

    // Skip this step when unsupported
    updateBorderColor = updateBorderColor && (context->getClientVersion() >= gl::ES_3_2 ||
                                              context->getExtensions().textureBorderClampAny());

    if (updateWorkarounds || updateDepthStencilTextureMode || updateBorderColor)
    {
        if (updateWorkarounds)
        {
            mLocalDirtyBits |= GetLevelWorkaroundDirtyBits();
        }
        if (updateDepthStencilTextureMode)
        {
            mLocalDirtyBits.set(gl::Texture::DIRTY_BIT_DEPTH_STENCIL_TEXTURE_MODE);
        }
        if (updateBorderColor)
        {
            mLocalDirtyBits.set(gl::Texture::DIRTY_BIT_BORDER_COLOR);
        }
        onStateChange(angle::SubjectMessage::DirtyBitsFlagged);
    }
}

void TextureGL::setLevelInfo(const gl::Context *context,
                             gl::TextureType type,
                             size_t level,
                             size_t levelCount,
                             const LevelInfoGL &levelInfo)
{
    if (type == gl::TextureType::CubeMap)
    {
        for (gl::TextureTarget target : gl::AllCubeFaceTextureTargets())
        {
            setLevelInfo(context, target, level, levelCount, levelInfo);
        }
    }
    else
    {
        setLevelInfo(context, NonCubeTextureTypeToTarget(type), level, levelCount, levelInfo);
    }
}

const LevelInfoGL &TextureGL::getLevelInfo(gl::TextureTarget target, size_t level) const
{
    return mLevelInfo[GetLevelInfoIndex(target, level)];
}

const LevelInfoGL &TextureGL::getBaseLevelInfo() const
{
    GLint effectiveBaseLevel = mState.getEffectiveBaseLevel();
    gl::TextureTarget target = getType() == gl::TextureType::CubeMap
                                   ? gl::kCubeMapTextureTargetMin
                                   : gl::NonCubeTextureTypeToTarget(getType());
    return getLevelInfo(target, effectiveBaseLevel);
}

gl::TextureType TextureGL::getType() const
{
    return mState.getType();
}

angle::Result TextureGL::initializeContents(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex)
{
    ContextGL *contextGL              = GetImplAs<ContextGL>(context);
    const FunctionsGL *functions      = GetFunctionsGL(context);
    StateManagerGL *stateManager      = GetStateManagerGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    bool shouldUseClear = !nativegl::SupportsTexImage(getType());
    GLenum nativeInternalFormat =
        getLevelInfo(imageIndex.getTarget(), imageIndex.getLevelIndex()).nativeInternalFormat;
    if ((features.allowClearForRobustResourceInit.enabled || shouldUseClear) &&
        nativegl::SupportsNativeRendering(functions, mState.getType(), nativeInternalFormat))
    {
        BlitGL *blitter = GetBlitGL(context);

        int levelDepth = mState.getImageDesc(imageIndex).size.depth;

        bool clearSucceeded = false;
        ANGLE_TRY(blitter->clearRenderableTexture(context, this, nativeInternalFormat, levelDepth,
                                                  imageIndex, &clearSucceeded));
        if (clearSucceeded)
        {
            contextGL->markWorkSubmitted();
            return angle::Result::Continue;
        }
    }

    // Either the texture is not renderable or was incomplete when clearing, fall back to a data
    // upload
    ASSERT(nativegl::SupportsTexImage(getType()));
    const gl::ImageDesc &desc                    = mState.getImageDesc(imageIndex);
    const gl::InternalFormat &internalFormatInfo = *desc.format.info;

    gl::PixelUnpackState unpackState;
    unpackState.alignment = 1;
    ANGLE_TRY(stateManager->setPixelUnpackState(context, unpackState));

    GLuint prevUnpackBuffer = stateManager->getBufferID(gl::BufferBinding::PixelUnpack);
    stateManager->bindBuffer(gl::BufferBinding::PixelUnpack, 0);

    stateManager->bindTexture(getType(), mTextureID);
    if (internalFormatInfo.compressed)
    {
        nativegl::CompressedTexSubImageFormat nativeSubImageFormat =
            nativegl::GetCompressedSubTexImageFormat(functions, features,
                                                     internalFormatInfo.internalFormat);

        GLuint imageSize = 0;
        ANGLE_CHECK_GL_MATH(contextGL,
                            internalFormatInfo.computeCompressedImageSize(desc.size, &imageSize));

        angle::MemoryBuffer *zero;
        ANGLE_CHECK_GL_ALLOC(contextGL, context->getZeroFilledBuffer(imageSize, &zero));

        // WebGL spec requires that zero data is uploaded to compressed textures even if it might
        // not result in zero color data.
        if (nativegl::UseTexImage2D(getType()))
        {
            ANGLE_GL_TRY(context, functions->compressedTexSubImage2D(
                                      ToGLenum(imageIndex.getTarget()), imageIndex.getLevelIndex(),
                                      0, 0, desc.size.width, desc.size.height,
                                      nativeSubImageFormat.format, imageSize, zero->data()));
        }
        else
        {
            ASSERT(nativegl::UseTexImage3D(getType()));
            ANGLE_GL_TRY(context, functions->compressedTexSubImage3D(
                                      ToGLenum(imageIndex.getTarget()), imageIndex.getLevelIndex(),
                                      0, 0, 0, desc.size.width, desc.size.height, desc.size.depth,
                                      nativeSubImageFormat.format, imageSize, zero->data()));
        }
    }
    else
    {
        nativegl::TexSubImageFormat nativeSubImageFormat = nativegl::GetTexSubImageFormat(
            functions, features, internalFormatInfo.format, internalFormatInfo.type);

        GLuint imageSize = 0;
        ANGLE_CHECK_GL_MATH(contextGL, internalFormatInfo.computePackUnpackEndByte(
                                           nativeSubImageFormat.type, desc.size, unpackState,
                                           nativegl::UseTexImage3D(getType()), &imageSize));

        angle::MemoryBuffer *zero;
        ANGLE_CHECK_GL_ALLOC(contextGL, context->getZeroFilledBuffer(imageSize, &zero));

        if (nativegl::UseTexImage2D(getType()))
        {
            if (features.uploadTextureDataInChunks.enabled)
            {
                gl::Box area(0, 0, 0, desc.size.width, desc.size.height, 1);
                ANGLE_TRY(setSubImageRowByRowWorkaround(
                    context, imageIndex.getTarget(), imageIndex.getLevelIndex(), area,
                    nativeSubImageFormat.format, nativeSubImageFormat.type, unpackState, nullptr,
                    kUploadTextureDataInChunksUploadSize, zero->data()));
            }
            else
            {
                ANGLE_GL_TRY(context,
                             functions->texSubImage2D(
                                 ToGLenum(imageIndex.getTarget()), imageIndex.getLevelIndex(), 0, 0,
                                 desc.size.width, desc.size.height, nativeSubImageFormat.format,
                                 nativeSubImageFormat.type, zero->data()));
            }
        }
        else
        {
            ASSERT(nativegl::UseTexImage3D(getType()));
            ANGLE_GL_TRY(context,
                         functions->texSubImage3D(
                             ToGLenum(imageIndex.getTarget()), imageIndex.getLevelIndex(), 0, 0, 0,
                             desc.size.width, desc.size.height, desc.size.depth,
                             nativeSubImageFormat.format, nativeSubImageFormat.type, zero->data()));
        }
    }

    // Reset the pixel unpack state.  Because this call is made after synchronizing dirty bits in a
    // glTexImage call, we need to make sure that the texture data to be uploaded later has the
    // expected unpack state.
    ANGLE_TRY(stateManager->setPixelUnpackState(context, context->getState().getUnpackState()));
    stateManager->bindBuffer(gl::BufferBinding::PixelUnpack, prevUnpackBuffer);

    contextGL->markWorkSubmitted();
    return angle::Result::Continue;
}

GLint TextureGL::getRequiredExternalTextureImageUnits(const gl::Context *context)
{
    const FunctionsGL *functions = GetFunctionsGL(context);
    StateManagerGL *stateManager = GetStateManagerGL(context);

    ASSERT(getType() == gl::TextureType::External);
    stateManager->bindTexture(getType(), mTextureID);

    GLint result = 0;
    functions->getTexParameteriv(ToGLenum(gl::NonCubeTextureTypeToTarget(getType())),
                                 GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES, &result);
    return result;
}

}  // namespace rx
