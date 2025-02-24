//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureImpl.cpp: Defines the abstract rx::TextureImpl classes.

#include "libANGLE/renderer/TextureImpl.h"

namespace rx
{
TextureImpl::TextureImpl(const gl::TextureState &state) : mState(state) {}

TextureImpl::~TextureImpl() {}

void TextureImpl::onDestroy(const gl::Context *context) {}

angle::Result TextureImpl::copyTexture(const gl::Context *context,
                                       const gl::ImageIndex &index,
                                       GLenum internalFormat,
                                       GLenum type,
                                       GLint sourceLevel,
                                       bool unpackFlipY,
                                       bool unpackPremultiplyAlpha,
                                       bool unpackUnmultiplyAlpha,
                                       const gl::Texture *source)
{
    UNREACHABLE();
    return angle::Result::Stop;
}

angle::Result TextureImpl::copySubTexture(const gl::Context *context,
                                          const gl::ImageIndex &index,
                                          const gl::Offset &destOffset,
                                          GLint sourceLevel,
                                          const gl::Box &sourceBox,
                                          bool unpackFlipY,
                                          bool unpackPremultiplyAlpha,
                                          bool unpackUnmultiplyAlpha,
                                          const gl::Texture *source)
{
    UNREACHABLE();
    return angle::Result::Stop;
}

angle::Result TextureImpl::copyRenderbufferSubData(const gl::Context *context,
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
    UNREACHABLE();
    return angle::Result::Stop;
}

angle::Result TextureImpl::copyTextureSubData(const gl::Context *context,
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
    UNREACHABLE();
    return angle::Result::Stop;
}

angle::Result TextureImpl::copyCompressedTexture(const gl::Context *context,
                                                 const gl::Texture *source)
{
    UNREACHABLE();
    return angle::Result::Stop;
}

angle::Result TextureImpl::copy3DTexture(const gl::Context *context,
                                         gl::TextureTarget target,
                                         GLenum internalFormat,
                                         GLenum type,
                                         GLint sourceLevel,
                                         GLint destLevel,
                                         bool unpackFlipY,
                                         bool unpackPremultiplyAlpha,
                                         bool unpackUnmultiplyAlpha,
                                         const gl::Texture *source)
{
    UNREACHABLE();
    return angle::Result::Stop;
}

angle::Result TextureImpl::copy3DSubTexture(const gl::Context *context,
                                            const gl::TextureTarget target,
                                            const gl::Offset &destOffset,
                                            GLint sourceLevel,
                                            GLint destLevel,
                                            const gl::Box &srcBox,
                                            bool unpackFlipY,
                                            bool unpackPremultiplyAlpha,
                                            bool unpackUnmultiplyAlpha,
                                            const gl::Texture *source)
{
    UNREACHABLE();
    return angle::Result::Stop;
}

angle::Result TextureImpl::setImageExternal(const gl::Context *context,
                                            const gl::ImageIndex &index,
                                            GLenum internalFormat,
                                            const gl::Extents &size,
                                            GLenum format,
                                            GLenum type)
{
    UNREACHABLE();
    return angle::Result::Stop;
}

angle::Result TextureImpl::setBuffer(const gl::Context *context, GLenum internalFormat)
{
    UNREACHABLE();
    return angle::Result::Stop;
}

angle::Result TextureImpl::clearImage(const gl::Context *context,
                                      GLint level,
                                      GLenum format,
                                      GLenum type,
                                      const uint8_t *data)
{
    UNREACHABLE();
    return angle::Result::Stop;
}

angle::Result TextureImpl::clearSubImage(const gl::Context *context,
                                         GLint level,
                                         const gl::Box &area,
                                         GLenum format,
                                         GLenum type,
                                         const uint8_t *data)
{
    UNREACHABLE();
    return angle::Result::Stop;
}

GLint TextureImpl::getMemorySize() const
{
    return 0;
}

GLint TextureImpl::getLevelMemorySize(gl::TextureTarget target, GLint level)
{
    return 0;
}

angle::Result TextureImpl::setStorageAttribs(const gl::Context *context,
                                             gl::TextureType type,
                                             size_t levels,
                                             GLint internalformat,
                                             const gl::Extents &size,
                                             const GLint *attribList)
{
    UNREACHABLE();
    return angle::Result::Stop;
}

GLint TextureImpl::getImageCompressionRate(const gl::Context *context)
{
    UNREACHABLE();
    return 0;
}

GLint TextureImpl::getFormatSupportedCompressionRates(const gl::Context *context,
                                                      GLenum internalformat,
                                                      GLsizei bufSize,
                                                      GLint *rates)
{
    UNREACHABLE();
    return 0;
}

GLint TextureImpl::getNativeID() const
{
    UNREACHABLE();
    return 0;
}

GLenum TextureImpl::getColorReadFormat(const gl::Context *context)
{
    UNREACHABLE();
    return GL_NONE;
}

GLenum TextureImpl::getColorReadType(const gl::Context *context)
{
    UNREACHABLE();
    return GL_NONE;
}

angle::Result TextureImpl::getTexImage(const gl::Context *context,
                                       const gl::PixelPackState &packState,
                                       gl::Buffer *packBuffer,
                                       gl::TextureTarget target,
                                       GLint level,
                                       GLenum format,
                                       GLenum type,
                                       void *pixels)
{
    UNREACHABLE();
    return angle::Result::Stop;
}

angle::Result TextureImpl::getCompressedTexImage(const gl::Context *context,
                                                 const gl::PixelPackState &packState,
                                                 gl::Buffer *packBuffer,
                                                 gl::TextureTarget target,
                                                 GLint level,
                                                 void *pixels)
{
    UNREACHABLE();
    return angle::Result::Stop;
}

GLint TextureImpl::getRequiredExternalTextureImageUnits(const gl::Context *context)
{
    UNREACHABLE();
    return 0;
}

angle::Result TextureImpl::onLabelUpdate(const gl::Context *context)
{
    return angle::Result::Continue;
}

}  // namespace rx
