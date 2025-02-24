//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Image.h: Defines the egl::Image class representing the EGLimage object.

#ifndef LIBANGLE_IMAGE_H_
#define LIBANGLE_IMAGE_H_

#include "common/FastVector.h"
#include "common/SimpleMutex.h"
#include "common/angleutils.h"
#include "libANGLE/AttributeMap.h"
#include "libANGLE/Debug.h"
#include "libANGLE/Error.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/RefCountObject.h"
#include "libANGLE/formatutils.h"

namespace rx
{
class EGLImplFactory;
class ImageImpl;
class ExternalImageSiblingImpl;

// Used for distinguishing dirty bit messages from gl::Texture/rx::TexureImpl/gl::Image.
constexpr size_t kTextureImageImplObserverMessageIndex = 0;
constexpr size_t kTextureImageSiblingMessageIndex      = 1;
}  // namespace rx

namespace egl
{
class Image;
class Display;
class ContextMutex;

// Only currently Renderbuffers and Textures can be bound with images. This makes the relationship
// explicit, and also ensures that an image sibling can determine if it's been initialized or not,
// which is important for the robust resource init extension with Textures and EGLImages.
class ImageSibling : public gl::FramebufferAttachmentObject
{
  public:
    ImageSibling();
    ~ImageSibling() override;

    bool isEGLImageTarget() const;
    gl::InitState sourceEGLImageInitState() const;
    void setSourceEGLImageInitState(gl::InitState initState) const;

    bool isRenderable(const gl::Context *context,
                      GLenum binding,
                      const gl::ImageIndex &imageIndex) const override;
    bool isYUV() const override;
    bool isExternalImageWithoutIndividualSync() const override;
    bool hasFrontBufferUsage() const override;
    bool hasProtectedContent() const override;
    bool hasFoveatedRendering() const override { return false; }
    const gl::FoveationState *getFoveationState() const override { return nullptr; }

  protected:
    static constexpr size_t kSourcesOfSetSize = 2;
    using UnorderedSetSiblingSource           = angle::FlatUnorderedSet<Image *, kSourcesOfSetSize>;

    const UnorderedSetSiblingSource &getSiblingSourcesOf() const { return mSourcesOf; }
    // Set the image target of this sibling
    void setTargetImage(const gl::Context *context, egl::Image *imageTarget);

    // Orphan all EGL image sources and targets
    angle::Result orphanImages(const gl::Context *context,
                               RefCountObjectReleaser<Image> *outReleaseImage);

    void notifySiblings(angle::SubjectMessage message);

  private:
    friend class Image;

    // Called from Image only to add a new source image
    void addImageSource(egl::Image *imageSource);

    // Called from Image only to remove a source image when the Image is being deleted
    void removeImageSource(egl::Image *imageSource);

    UnorderedSetSiblingSource mSourcesOf;

    BindingPointer<Image> mTargetOf;
};

// Wrapper for EGLImage sources that are not owned by ANGLE, these often have to do
// platform-specific queries for format and size information.
class ExternalImageSibling : public ImageSibling
{
  public:
    ExternalImageSibling(rx::EGLImplFactory *factory,
                         const gl::Context *context,
                         EGLenum target,
                         EGLClientBuffer buffer,
                         const AttributeMap &attribs);
    ~ExternalImageSibling() override;

    void onDestroy(const egl::Display *display);

    Error initialize(const Display *display, const gl::Context *context);

    gl::Extents getAttachmentSize(const gl::ImageIndex &imageIndex) const override;
    gl::Format getAttachmentFormat(GLenum binding, const gl::ImageIndex &imageIndex) const override;
    GLsizei getAttachmentSamples(const gl::ImageIndex &imageIndex) const override;
    GLuint getLevelCount() const;
    bool isRenderable(const gl::Context *context,
                      GLenum binding,
                      const gl::ImageIndex &imageIndex) const override;
    bool isTextureable(const gl::Context *context) const;
    bool isYUV() const override;
    bool hasFrontBufferUsage() const override;
    bool isCubeMap() const;
    bool hasProtectedContent() const override;

    void onAttach(const gl::Context *context, rx::UniqueSerial framebufferSerial) override;
    void onDetach(const gl::Context *context, rx::UniqueSerial framebufferSerial) override;
    GLuint getId() const override;

    gl::InitState initState(GLenum binding, const gl::ImageIndex &imageIndex) const override;
    void setInitState(GLenum binding,
                      const gl::ImageIndex &imageIndex,
                      gl::InitState initState) override;

    rx::ExternalImageSiblingImpl *getImplementation() const;

  protected:
    rx::FramebufferAttachmentObjectImpl *getAttachmentImpl() const override;

  private:
    // ObserverInterface implementation.
    void onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message) override;

    std::unique_ptr<rx::ExternalImageSiblingImpl> mImplementation;
    angle::ObserverBinding mImplObserverBinding;
};

struct ImageState : private angle::NonCopyable
{
    ImageState(ImageID id, EGLenum target, ImageSibling *buffer, const AttributeMap &attribs);
    ~ImageState();

    ImageID id;

    EGLLabelKHR label;
    EGLenum target;
    gl::ImageIndex imageIndex;
    ImageSibling *source;

    gl::Format format;
    bool yuv;
    bool cubeMap;
    gl::Extents size;
    size_t samples;
    GLuint levelCount;
    EGLenum colorspace;
    bool hasProtectedContent;

    mutable angle::SimpleMutex targetsLock;

    static constexpr size_t kTargetsSetSize = 2;
    angle::FlatUnorderedSet<ImageSibling *, kTargetsSetSize> targets;
};

class Image final : public ThreadSafeRefCountObject, public LabeledObject
{
  public:
    Image(rx::EGLImplFactory *factory,
          ImageID id,
          const gl::Context *context,
          EGLenum target,
          ImageSibling *buffer,
          const AttributeMap &attribs);

    void onDestroy(const Display *display) override;
    ~Image() override;

    ImageID id() const { return mState.id; }

    void setLabel(EGLLabelKHR label) override;
    EGLLabelKHR getLabel() const override;

    const gl::Format &getFormat() const;
    bool isRenderable(const gl::Context *context) const;
    bool isTexturable(const gl::Context *context) const;
    bool isYUV() const;
    bool isExternalImageWithoutIndividualSync() const;
    bool hasFrontBufferUsage() const;
    // Returns true only if the eglImage contains a complete cubemap
    bool isCubeMap() const;
    size_t getWidth() const;
    size_t getHeight() const;
    const gl::Extents &getExtents() const;
    bool isLayered() const;
    size_t getSamples() const;
    GLuint getLevelCount() const;
    bool hasProtectedContent() const;
    bool isFixedRatedCompression(const gl::Context *context) const;
    EGLenum getColorspaceAttribute() const { return mState.colorspace; }

    Error initialize(const Display *display, const gl::Context *context);

    rx::ImageImpl *getImplementation() const;

    bool orphaned() const;
    gl::InitState sourceInitState() const;
    void setInitState(gl::InitState initState);

    Error exportVkImage(void *vkImage, void *vkImageCreateInfo);

    ContextMutex *getContextMutex() const { return mContextMutex; }

    const gl::ImageIndex &getSourceImageIndex() const { return mState.imageIndex; }

  private:
    friend class ImageSibling;

    // Called from ImageSibling only notify the image that a new target sibling exists for state
    // tracking.
    void addTargetSibling(ImageSibling *sibling);

    // Called from ImageSibling only to notify the image that a sibling (source or target) has
    // been respecified and state tracking should be updated.
    angle::Result orphanSibling(const gl::Context *context, ImageSibling *sibling);

    void notifySiblings(const ImageSibling *notifier, angle::SubjectMessage message);

    ImageState mState;
    rx::ImageImpl *mImplementation;
    bool mOrphanedAndNeedsInit;
    bool mIsTexturable = false;
    bool mIsRenderable = false;

    ContextMutex *mContextMutex;  // Reference counted
};
}  // namespace egl

#endif  // LIBANGLE_IMAGE_H_
