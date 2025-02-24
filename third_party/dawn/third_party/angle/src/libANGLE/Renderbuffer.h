//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Renderbuffer.h: Defines the renderer-agnostic container class gl::Renderbuffer.
// Implements GL renderbuffer objects and related functionality.
// [OpenGL ES 2.0.24] section 4.4.3 page 108.

#ifndef LIBANGLE_RENDERBUFFER_H_
#define LIBANGLE_RENDERBUFFER_H_

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/Debug.h"
#include "libANGLE/Error.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/Image.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/RenderbufferImpl.h"

namespace rx
{
class GLImplFactory;
}  // namespace rx

namespace gl
{
// A GL renderbuffer object is usually used as a depth or stencil buffer attachment
// for a framebuffer object. The renderbuffer itself is a distinct GL object, see
// FramebufferAttachment and Framebuffer for how they are applied to an FBO via an
// attachment point.

class RenderbufferState final : angle::NonCopyable
{
  public:
    RenderbufferState();
    ~RenderbufferState();

    GLsizei getWidth() const;
    GLsizei getHeight() const;
    const Format &getFormat() const;
    GLsizei getSamples() const;
    MultisamplingMode getMultisamplingMode() const;
    InitState getInitState() const;
    void setProtectedContent(bool hasProtectedContent);

  private:
    friend class Renderbuffer;

    void update(GLsizei width,
                GLsizei height,
                const Format &format,
                GLsizei samples,
                MultisamplingMode multisamplingMode,
                InitState initState);

    GLsizei mWidth;
    GLsizei mHeight;
    Format mFormat;
    GLsizei mSamples;
    MultisamplingMode mMultisamplingMode;
    bool mHasProtectedContent;

    // For robust resource init.
    InitState mInitState;
};

class Renderbuffer final : public RefCountObject<RenderbufferID>,
                           public egl::ImageSibling,
                           public LabeledObject
{
  public:
    Renderbuffer(rx::GLImplFactory *implFactory, RenderbufferID id);
    ~Renderbuffer() override;

    void onDestroy(const Context *context) override;

    angle::Result setLabel(const Context *context, const std::string &label) override;
    const std::string &getLabel() const override;

    angle::Result setStorage(const Context *context,
                             GLenum internalformat,
                             GLsizei width,
                             GLsizei height);
    angle::Result setStorageMultisample(const Context *context,
                                        GLsizei samplesIn,
                                        GLenum internalformat,
                                        GLsizei width,
                                        GLsizei height,
                                        MultisamplingMode mode);
    angle::Result setStorageEGLImageTarget(const Context *context, egl::Image *imageTarget);

    angle::Result copyRenderbufferSubData(Context *context,
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

    angle::Result copyTextureSubData(Context *context,
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

    rx::RenderbufferImpl *getImplementation() const;

    GLsizei getWidth() const;
    GLsizei getHeight() const;
    const Format &getFormat() const;
    GLsizei getSamples() const;
    MultisamplingMode getMultisamplingMode() const;
    GLuint getRedSize() const;
    GLuint getGreenSize() const;
    GLuint getBlueSize() const;
    GLuint getAlphaSize() const;
    GLuint getDepthSize() const;
    GLuint getStencilSize() const;
    const RenderbufferState &getState() const;

    GLint getMemorySize() const;

    // FramebufferAttachmentObject Impl
    Extents getAttachmentSize(const ImageIndex &imageIndex) const override;
    Format getAttachmentFormat(GLenum binding, const ImageIndex &imageIndex) const override;
    GLsizei getAttachmentSamples(const ImageIndex &imageIndex) const override;
    bool isRenderable(const Context *context,
                      GLenum binding,
                      const ImageIndex &imageIndex) const override;
    bool isEGLImageSource() const;

    void onAttach(const Context *context, rx::UniqueSerial framebufferSerial) override;
    void onDetach(const Context *context, rx::UniqueSerial framebufferSerial) override;
    GLuint getId() const override;

    InitState initState(GLenum binding, const ImageIndex &imageIndex) const override;
    void setInitState(GLenum binding, const ImageIndex &imageIndex, InitState initState) override;

    GLenum getImplementationColorReadFormat(const Context *context) const;
    GLenum getImplementationColorReadType(const Context *context) const;

    // We pass the pack buffer and state explicitly so they can be overridden during capture.
    angle::Result getRenderbufferImage(const Context *context,
                                       const PixelPackState &packState,
                                       Buffer *packBuffer,
                                       GLenum format,
                                       GLenum type,
                                       void *pixels) const;

  private:
    // ObserverInterface implementation.
    void onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message) override;

    rx::FramebufferAttachmentObjectImpl *getAttachmentImpl() const override;

    RenderbufferState mState;
    std::unique_ptr<rx::RenderbufferImpl> mImplementation;

    std::string mLabel;
    angle::ObserverBinding mImplObserverBinding;
};

}  // namespace gl

#endif  // LIBANGLE_RENDERBUFFER_H_
