//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RenderbufferVk.h:
//    Defines the class interface for RenderbufferVk, implementing RenderbufferImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_RENDERBUFFERVK_H_
#define LIBANGLE_RENDERER_VULKAN_RENDERBUFFERVK_H_

#include "libANGLE/renderer/RenderbufferImpl.h"
#include "libANGLE/renderer/vulkan/RenderTargetVk.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{

class RenderbufferVk : public RenderbufferImpl, public angle::ObserverInterface
{
  public:
    RenderbufferVk(const gl::RenderbufferState &state);
    ~RenderbufferVk() override;

    void onDestroy(const gl::Context *context) override;

    angle::Result setStorage(const gl::Context *context,
                             GLenum internalformat,
                             GLsizei width,
                             GLsizei height) override;
    angle::Result setStorageMultisample(const gl::Context *context,
                                        GLsizei samples,
                                        GLenum internalformat,
                                        GLsizei width,
                                        GLsizei height,
                                        gl::MultisamplingMode mode) override;
    angle::Result setStorageEGLImageTarget(const gl::Context *context, egl::Image *image) override;

    angle::Result copyRenderbufferSubData(const gl::Context *context,
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
                                          GLsizei srcDepth) override;

    angle::Result copyTextureSubData(const gl::Context *context,
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
                                     GLsizei srcDepth) override;

    angle::Result getAttachmentRenderTarget(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex,
                                            GLsizei samples,
                                            FramebufferAttachmentRenderTarget **rtOut) override;

    angle::Result initializeContents(const gl::Context *context,
                                     GLenum binding,
                                     const gl::ImageIndex &imageIndex) override;

    vk::ImageHelper *getImage() const { return mImage; }
    void releaseOwnershipOfImage(const gl::Context *context);

    GLenum getColorReadFormat(const gl::Context *context) override;
    GLenum getColorReadType(const gl::Context *context) override;

    angle::Result getRenderbufferImage(const gl::Context *context,
                                       const gl::PixelPackState &packState,
                                       gl::Buffer *packBuffer,
                                       GLenum format,
                                       GLenum type,
                                       void *pixels) override;

    angle::Result ensureImageInitialized(const gl::Context *context);

  private:
    void releaseAndDeleteImage(ContextVk *contextVk);
    void releaseImage(ContextVk *contextVk);

    angle::Result setStorageImpl(const gl::Context *context,
                                 GLsizei samples,
                                 GLenum internalformat,
                                 GLsizei width,
                                 GLsizei height,
                                 gl::MultisamplingMode mode);

    const gl::InternalFormat &getImplementationSizedFormat() const;

    // We monitor the staging buffer for changes. This handles staged data from outside this class.
    void onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message) override;

    bool mOwnsImage;
    // Generated from ImageVk if EGLImage target.
    UniqueSerial mImageSiblingSerial;

    // |mOwnsImage| indicates that |RenderbufferVk| owns the image.  Otherwise, this is a weak
    // pointer shared with another class.  Due to this sharing, for example through EGL images, the
    // image must always be dynamically allocated as the renderbuffer can release ownership for
    // example and it can be transferred to another |RenderbufferVk|.
    vk::ImageHelper *mImage;
    vk::ImageViewHelper mImageViews;

    // If renderbuffer is created through the EXT_multisampled_render_to_texture API, it is expected
    // that all rendering is done multisampled during the renderpass, and is automatically resolved
    // (into |mImage|) and discarded afterwards.  |mMultisampledImage| is the implicit image that
    // contains the multisampled data.
    vk::ImageHelper mMultisampledImage;
    vk::ImageViewHelper mMultisampledImageViews;

    RenderTargetVk mRenderTarget;

    angle::ObserverBinding mImageObserverBinding;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_RENDERBUFFERVK_H_
