//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FramebufferAttachment.h: Defines the wrapper class gl::FramebufferAttachment, as well as the
// objects and related functionality. [OpenGL ES 2.0.24] section 4.4.3 page 108.

#ifndef LIBANGLE_FRAMEBUFFERATTACHMENT_H_
#define LIBANGLE_FRAMEBUFFERATTACHMENT_H_

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/Error.h"
#include "libANGLE/ImageIndex.h"
#include "libANGLE/Observer.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/FramebufferAttachmentObjectImpl.h"

namespace egl
{
class Surface;
}

namespace rx
{
// An implementation-specific object associated with an attachment.

class FramebufferAttachmentRenderTarget : angle::NonCopyable
{
  public:
    FramebufferAttachmentRenderTarget() {}
    virtual ~FramebufferAttachmentRenderTarget() {}
};

class FramebufferAttachmentObjectImpl;
}  // namespace rx

namespace gl
{
class FramebufferAttachmentObject;
class Renderbuffer;
class Texture;

// FramebufferAttachment implements a GL framebuffer attachment.
// Attachments are "light" containers, which store pointers to ref-counted GL objects.
// We support GL texture (2D/3D/Cube/2D array) and renderbuffer object attachments.
// Note: Our old naming scheme used the term "Renderbuffer" for both GL renderbuffers and for
// framebuffer attachments, which confused their usage.

class FramebufferAttachment final
{
  public:
    FramebufferAttachment();

    FramebufferAttachment(const Context *context,
                          GLenum type,
                          GLenum binding,
                          const ImageIndex &textureIndex,
                          FramebufferAttachmentObject *resource,
                          rx::UniqueSerial framebufferSerial);

    FramebufferAttachment(FramebufferAttachment &&other);
    FramebufferAttachment &operator=(FramebufferAttachment &&other);

    ~FramebufferAttachment();

    void detach(const Context *context, rx::UniqueSerial framebufferSerial);
    void attach(const Context *context,
                GLenum type,
                GLenum binding,
                const ImageIndex &textureIndex,
                FramebufferAttachmentObject *resource,
                GLsizei numViews,
                GLuint baseViewIndex,
                bool isMultiview,
                GLsizei samples,
                rx::UniqueSerial framebufferSerial);

    // Helper methods
    GLuint getRedSize() const;
    GLuint getGreenSize() const;
    GLuint getBlueSize() const;
    GLuint getAlphaSize() const;
    GLuint getDepthSize() const;
    GLuint getStencilSize() const;
    GLenum getComponentType() const;
    GLenum getColorEncoding() const;

    bool isTextureWithId(TextureID textureId) const
    {
        return mType == GL_TEXTURE && id() == textureId.value;
    }
    bool isExternalTexture() const
    {
        return mType == GL_TEXTURE && getTextureImageIndex().getType() == gl::TextureType::External;
    }
    bool isRenderbufferWithId(GLuint renderbufferId) const
    {
        return mType == GL_RENDERBUFFER && id() == renderbufferId;
    }

    GLenum getBinding() const { return mTarget.binding(); }
    GLuint id() const;

    // These methods are only legal to call on Texture attachments
    const ImageIndex &getTextureImageIndex() const;
    TextureTarget cubeMapFace() const;
    GLint mipLevel() const;
    GLint layer() const;
    bool isLayered() const;

    GLsizei getNumViews() const { return mNumViews; }

    bool isMultiview() const;
    GLint getBaseViewIndex() const;

    bool isRenderToTexture() const;
    GLsizei getRenderToTextureSamples() const;

    // The size of the underlying resource the attachment points to. The 'depth' value will
    // correspond to a 3D texture depth or the layer count of a 2D array texture. For Surfaces and
    // Renderbuffers, it will always be 1.
    Extents getSize() const;
    Format getFormat() const;
    GLsizei getSamples() const;
    // This will always return the actual sample count of the attachment even if
    // render_to_texture extension is active on this FBattachment object.
    GLsizei getResourceSamples() const;
    GLenum type() const { return mType; }
    bool isAttached() const { return mType != GL_NONE; }
    bool isRenderable(const Context *context) const;
    bool isYUV() const;
    // Checks whether the attachment is an external image such as AHB or dmabuf, where
    // synchronization is done without individually naming the objects that are involved.  This
    // excludes Vulkan images (EXT_external_objects) as they are individually listed during
    // synchronization.
    //
    // This function is used to disable optimizations that involve deferring operations, as there is
    // no efficient way to perform those deferred operations on sync if the specific objects are
    // unknown.
    bool isExternalImageWithoutIndividualSync() const;
    bool hasFrontBufferUsage() const;
    bool hasFoveatedRendering() const;
    const gl::FoveationState *getFoveationState() const;

    Renderbuffer *getRenderbuffer() const;
    Texture *getTexture() const;
    const egl::Surface *getSurface() const;
    FramebufferAttachmentObject *getResource() const;
    InitState initState() const;
    angle::Result initializeContents(const Context *context) const;
    void setInitState(InitState initState) const;

    // "T" must be static_castable from FramebufferAttachmentRenderTarget
    template <typename T>
    angle::Result getRenderTarget(const Context *context, GLsizei samples, T **rtOut) const
    {
        static_assert(std::is_base_of<rx::FramebufferAttachmentRenderTarget, T>(),
                      "Invalid RenderTarget class.");
        return getRenderTargetImpl(
            context, samples, reinterpret_cast<rx::FramebufferAttachmentRenderTarget **>(rtOut));
    }

    bool operator==(const FramebufferAttachment &other) const;
    bool operator!=(const FramebufferAttachment &other) const;

    static const GLsizei kDefaultNumViews;
    static const GLint kDefaultBaseViewIndex;
    static const GLint kDefaultRenderToTextureSamples;

  private:
    angle::Result getRenderTargetImpl(const Context *context,
                                      GLsizei samples,
                                      rx::FramebufferAttachmentRenderTarget **rtOut) const;

    // A framebuffer attachment points to one of three types of resources: Renderbuffers,
    // Textures and egl::Surface. The "Target" struct indicates which part of the
    // object an attachment references. For the three types:
    //   - a Renderbuffer has a unique renderable target, and needs no target index
    //   - a Texture has targets for every image and uses an ImageIndex
    //   - a Surface has targets for Color and Depth/Stencil, and uses the attachment binding
    class Target
    {
      public:
        Target();
        Target(GLenum binding, const ImageIndex &imageIndex);
        Target(const Target &other);
        Target &operator=(const Target &other);

        GLenum binding() const { return mBinding; }
        const ImageIndex &textureIndex() const { return mTextureIndex; }

      private:
        GLenum mBinding;
        ImageIndex mTextureIndex;
    };

    GLenum mType;
    Target mTarget;
    FramebufferAttachmentObject *mResource;
    GLsizei mNumViews;
    bool mIsMultiview;
    GLint mBaseViewIndex;
    // A single-sampled texture can be attached to a framebuffer either as single-sampled or as
    // multisampled-render-to-texture.  In the latter case, |mRenderToTextureSamples| will contain
    // the number of samples.  For renderbuffers, the number of samples is inherited from the
    // renderbuffer itself.
    //
    // Note that textures cannot change storage between single and multisample once attached to a
    // framebuffer.  Renderbuffers instead can, and caching the number of renderbuffer samples here
    // can lead to stale data.
    GLsizei mRenderToTextureSamples;
};

// A base class for objects that FBO Attachments may point to.
class FramebufferAttachmentObject : public angle::Subject, public angle::ObserverInterface
{
  public:
    FramebufferAttachmentObject();
    ~FramebufferAttachmentObject() override;

    virtual Extents getAttachmentSize(const ImageIndex &imageIndex) const                  = 0;
    virtual Format getAttachmentFormat(GLenum binding, const ImageIndex &imageIndex) const = 0;
    virtual GLsizei getAttachmentSamples(const ImageIndex &imageIndex) const               = 0;
    virtual bool isRenderable(const Context *context,
                              GLenum binding,
                              const ImageIndex &imageIndex) const                          = 0;
    virtual bool isYUV() const                                                             = 0;
    virtual bool isExternalImageWithoutIndividualSync() const                              = 0;
    virtual bool hasFrontBufferUsage() const                                               = 0;
    virtual bool hasProtectedContent() const                                               = 0;
    virtual bool hasFoveatedRendering() const                                              = 0;
    virtual const gl::FoveationState *getFoveationState() const                            = 0;

    virtual void onAttach(const Context *context, rx::UniqueSerial framebufferSerial) = 0;
    virtual void onDetach(const Context *context, rx::UniqueSerial framebufferSerial) = 0;
    virtual GLuint getId() const                                                      = 0;

    // These are used for robust resource initialization.
    virtual InitState initState(GLenum binding, const ImageIndex &imageIndex) const = 0;
    virtual void setInitState(GLenum binding,
                              const ImageIndex &imageIndex,
                              InitState initState)                                  = 0;

    angle::Result getAttachmentRenderTarget(const Context *context,
                                            GLenum binding,
                                            const ImageIndex &imageIndex,
                                            GLsizei samples,
                                            rx::FramebufferAttachmentRenderTarget **rtOut) const;

    angle::Result initializeContents(const Context *context,
                                     GLenum binding,
                                     const ImageIndex &imageIndex);

  protected:
    virtual rx::FramebufferAttachmentObjectImpl *getAttachmentImpl() const = 0;
};

inline const ImageIndex &FramebufferAttachment::getTextureImageIndex() const
{
    ASSERT(type() == GL_TEXTURE);
    return mTarget.textureIndex();
}

inline Extents FramebufferAttachment::getSize() const
{
    ASSERT(mResource);
    return mResource->getAttachmentSize(mTarget.textureIndex());
}

inline Format FramebufferAttachment::getFormat() const
{
    ASSERT(mResource);
    return mResource->getAttachmentFormat(mTarget.binding(), mTarget.textureIndex());
}

inline GLsizei FramebufferAttachment::getSamples() const
{
    return isRenderToTexture() ? getRenderToTextureSamples() : getResourceSamples();
}

inline GLsizei FramebufferAttachment::getResourceSamples() const
{
    ASSERT(mResource);
    return mResource->getAttachmentSamples(mTarget.textureIndex());
}

inline angle::Result FramebufferAttachment::getRenderTargetImpl(
    const Context *context,
    GLsizei samples,
    rx::FramebufferAttachmentRenderTarget **rtOut) const
{
    ASSERT(mResource);
    return mResource->getAttachmentRenderTarget(context, mTarget.binding(), mTarget.textureIndex(),
                                                samples, rtOut);
}

inline bool FramebufferAttachment::isRenderable(const Context *context) const
{
    ASSERT(mResource);
    return mResource->isRenderable(context, mTarget.binding(), mTarget.textureIndex());
}

inline bool FramebufferAttachment::isYUV() const
{
    ASSERT(mResource);
    return mResource->isYUV();
}

inline bool FramebufferAttachment::isExternalImageWithoutIndividualSync() const
{
    ASSERT(mResource);
    return mResource->isExternalImageWithoutIndividualSync();
}

inline bool FramebufferAttachment::hasFrontBufferUsage() const
{
    ASSERT(mResource);
    return mResource->hasFrontBufferUsage();
}

inline bool FramebufferAttachment::hasFoveatedRendering() const
{
    ASSERT(mResource);
    return mResource->hasFoveatedRendering();
}

inline const gl::FoveationState *FramebufferAttachment::getFoveationState() const
{
    ASSERT(mResource);
    return mResource->getFoveationState();
}
}  // namespace gl

#endif  // LIBANGLE_FRAMEBUFFERATTACHMENT_H_
