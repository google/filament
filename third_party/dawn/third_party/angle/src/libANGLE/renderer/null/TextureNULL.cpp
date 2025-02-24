//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TextureNULL.cpp:
//    Implements the class methods for TextureNULL.
//

#include "libANGLE/renderer/null/TextureNULL.h"

#include "common/debug.h"

namespace rx
{

TextureNULL::TextureNULL(const gl::TextureState &state) : TextureImpl(state) {}

TextureNULL::~TextureNULL() {}

angle::Result TextureNULL::setImage(const gl::Context *context,
                                    const gl::ImageIndex &index,
                                    GLenum internalFormat,
                                    const gl::Extents &size,
                                    GLenum format,
                                    GLenum type,
                                    const gl::PixelUnpackState &unpack,
                                    gl::Buffer *unpackBuffer,
                                    const uint8_t *pixels)
{
    // TODO(geofflang): Read all incoming pixel data (maybe hash it?) to make sure we don't read out
    // of bounds due to validation bugs.
    return angle::Result::Continue;
}

angle::Result TextureNULL::setSubImage(const gl::Context *context,
                                       const gl::ImageIndex &index,
                                       const gl::Box &area,
                                       GLenum format,
                                       GLenum type,
                                       const gl::PixelUnpackState &unpack,
                                       gl::Buffer *unpackBuffer,
                                       const uint8_t *pixels)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::setCompressedImage(const gl::Context *context,
                                              const gl::ImageIndex &index,
                                              GLenum internalFormat,
                                              const gl::Extents &size,
                                              const gl::PixelUnpackState &unpack,
                                              size_t imageSize,
                                              const uint8_t *pixels)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::setCompressedSubImage(const gl::Context *context,
                                                 const gl::ImageIndex &index,
                                                 const gl::Box &area,
                                                 GLenum format,
                                                 const gl::PixelUnpackState &unpack,
                                                 size_t imageSize,
                                                 const uint8_t *pixels)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::copyImage(const gl::Context *context,
                                     const gl::ImageIndex &index,
                                     const gl::Rectangle &sourceArea,
                                     GLenum internalFormat,
                                     gl::Framebuffer *source)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::copySubImage(const gl::Context *context,
                                        const gl::ImageIndex &index,
                                        const gl::Offset &destOffset,
                                        const gl::Rectangle &sourceArea,
                                        gl::Framebuffer *source)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::copyTexture(const gl::Context *context,
                                       const gl::ImageIndex &index,
                                       GLenum internalFormat,
                                       GLenum type,
                                       GLint sourceLevel,
                                       bool unpackFlipY,
                                       bool unpackPremultiplyAlpha,
                                       bool unpackUnmultiplyAlpha,
                                       const gl::Texture *source)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::copySubTexture(const gl::Context *context,
                                          const gl::ImageIndex &index,
                                          const gl::Offset &destOffset,
                                          GLint sourceLevel,
                                          const gl::Box &sourceBox,
                                          bool unpackFlipY,
                                          bool unpackPremultiplyAlpha,
                                          bool unpackUnmultiplyAlpha,
                                          const gl::Texture *source)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::copyRenderbufferSubData(const gl::Context *context,
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
    return angle::Result::Continue;
}

angle::Result TextureNULL::copyTextureSubData(const gl::Context *context,
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
    return angle::Result::Continue;
}

angle::Result TextureNULL::copyCompressedTexture(const gl::Context *context,
                                                 const gl::Texture *source)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::setStorage(const gl::Context *context,
                                      gl::TextureType type,
                                      size_t levels,
                                      GLenum internalFormat,
                                      const gl::Extents &size)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::setStorageExternalMemory(const gl::Context *context,
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
    return angle::Result::Continue;
}

angle::Result TextureNULL::setEGLImageTarget(const gl::Context *context,
                                             gl::TextureType type,
                                             egl::Image *image)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::setImageExternal(const gl::Context *context,
                                            gl::TextureType type,
                                            egl::Stream *stream,
                                            const egl::Stream::GLTextureDescription &desc)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::generateMipmap(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::setBaseLevel(const gl::Context *context, GLuint baseLevel)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::bindTexImage(const gl::Context *context, egl::Surface *surface)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::releaseTexImage(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::syncState(const gl::Context *context,
                                     const gl::Texture::DirtyBits &dirtyBits,
                                     gl::Command source)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::setStorageMultisample(const gl::Context *context,
                                                 gl::TextureType type,
                                                 GLsizei samples,
                                                 GLint internalformat,
                                                 const gl::Extents &size,
                                                 bool fixedSampleLocations)
{
    return angle::Result::Continue;
}

angle::Result TextureNULL::initializeContents(const gl::Context *context,
                                              GLenum binding,
                                              const gl::ImageIndex &imageIndex)
{
    return angle::Result::Continue;
}

}  // namespace rx
