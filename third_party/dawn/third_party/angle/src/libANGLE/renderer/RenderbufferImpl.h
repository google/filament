//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderbufferImpl.h: Defines the abstract class gl::RenderbufferImpl

#ifndef LIBANGLE_RENDERER_RENDERBUFFERIMPL_H_
#define LIBANGLE_RENDERER_RENDERBUFFERIMPL_H_

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/Error.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/FramebufferAttachmentObjectImpl.h"

namespace gl
{
struct PixelPackState;
class RenderbufferState;
}  // namespace gl

namespace egl
{
class Image;
}  // namespace egl

namespace rx
{

class RenderbufferImpl : public FramebufferAttachmentObjectImpl
{
  public:
    RenderbufferImpl(const gl::RenderbufferState &state) : mState(state) {}
    ~RenderbufferImpl() override {}
    virtual void onDestroy(const gl::Context *context) {}

    virtual angle::Result setStorage(const gl::Context *context,
                                     GLenum internalformat,
                                     GLsizei width,
                                     GLsizei height)                        = 0;
    virtual angle::Result setStorageMultisample(const gl::Context *context,
                                                GLsizei samples,
                                                GLenum internalformat,
                                                GLsizei width,
                                                GLsizei height,
                                                gl::MultisamplingMode mode) = 0;
    virtual angle::Result setStorageEGLImageTarget(const gl::Context *context,
                                                   egl::Image *image)       = 0;

    virtual angle::Result copyRenderbufferSubData(const gl::Context *context,
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
                                                  GLsizei srcDepth);

    virtual angle::Result copyTextureSubData(const gl::Context *context,
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
                                             GLsizei srcDepth);

    virtual GLenum getColorReadFormat(const gl::Context *context);
    virtual GLenum getColorReadType(const gl::Context *context);

    virtual angle::Result getRenderbufferImage(const gl::Context *context,
                                               const gl::PixelPackState &packState,
                                               gl::Buffer *packBuffer,
                                               GLenum format,
                                               GLenum type,
                                               void *pixels);

    // Override if accurate native memory size information is available
    virtual GLint getMemorySize() const;

    virtual angle::Result onLabelUpdate(const gl::Context *context);

  protected:
    const gl::RenderbufferState &mState;
};

inline angle::Result RenderbufferImpl::copyRenderbufferSubData(const gl::Context *context,
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

inline angle::Result RenderbufferImpl::copyTextureSubData(const gl::Context *context,
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

inline GLint RenderbufferImpl::getMemorySize() const
{
    return 0;
}

inline GLenum RenderbufferImpl::getColorReadFormat(const gl::Context *context)
{
    UNREACHABLE();
    return GL_NONE;
}

inline GLenum RenderbufferImpl::getColorReadType(const gl::Context *context)
{
    UNREACHABLE();
    return GL_NONE;
}

inline angle::Result RenderbufferImpl::getRenderbufferImage(const gl::Context *context,
                                                            const gl::PixelPackState &packState,
                                                            gl::Buffer *packBuffer,
                                                            GLenum format,
                                                            GLenum type,
                                                            void *pixels)
{
    UNREACHABLE();
    return angle::Result::Stop;
}
}  // namespace rx

#endif  // LIBANGLE_RENDERER_RENDERBUFFERIMPL_H_
