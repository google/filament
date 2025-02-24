//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Image.cpp: Implements the egl::Image class representing the EGLimage object.

#include "libANGLE/Image.h"

#include "common/debug.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/Texture.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/EGLImplFactory.h"
#include "libANGLE/renderer/ImageImpl.h"

namespace egl
{

namespace
{
gl::ImageIndex GetImageIndex(EGLenum eglTarget, const egl::AttributeMap &attribs)
{
    if (!IsTextureTarget(eglTarget))
    {
        return gl::ImageIndex();
    }

    gl::TextureTarget target = egl_gl::EGLImageTargetToTextureTarget(eglTarget);
    GLint mip                = static_cast<GLint>(attribs.get(EGL_GL_TEXTURE_LEVEL_KHR, 0));
    GLint layer              = static_cast<GLint>(attribs.get(EGL_GL_TEXTURE_ZOFFSET_KHR, 0));

    if (target == gl::TextureTarget::_3D)
    {
        return gl::ImageIndex::Make3D(mip, layer);
    }
    else if (gl::IsCubeMapFaceTarget(target))
    {
        return gl::ImageIndex::MakeCubeMapFace(target, mip);
    }
    else
    {
        ASSERT(layer == 0);
        return gl::ImageIndex::MakeFromTarget(target, mip, 1);
    }
}

const Display *DisplayFromContext(const gl::Context *context)
{
    return (context ? context->getDisplay() : nullptr);
}

angle::SubjectIndex kExternalImageImplSubjectIndex = 0;
}  // anonymous namespace

ImageSibling::ImageSibling() : FramebufferAttachmentObject(), mSourcesOf(), mTargetOf() {}

ImageSibling::~ImageSibling()
{
    // EGL images should hold a ref to their targets and siblings, a Texture should not be deletable
    // while it is attached to an EGL image.
    // Child class should orphan images before destruction.
    ASSERT(mSourcesOf.empty());
    ASSERT(mTargetOf.get() == nullptr);
}

void ImageSibling::setTargetImage(const gl::Context *context, egl::Image *imageTarget)
{
    ASSERT(imageTarget != nullptr);
    mTargetOf.set(DisplayFromContext(context), imageTarget);
    imageTarget->addTargetSibling(this);
}

angle::Result ImageSibling::orphanImages(const gl::Context *context,
                                         RefCountObjectReleaser<Image> *outReleaseImage)
{
    ASSERT(outReleaseImage != nullptr);

    if (mTargetOf.get() != nullptr)
    {
        // Can't be a target and have sources.
        ASSERT(mSourcesOf.empty());

        ANGLE_TRY(mTargetOf->orphanSibling(context, this));
        *outReleaseImage = mTargetOf.set(DisplayFromContext(context), nullptr);
    }
    else
    {
        for (Image *sourceImage : mSourcesOf)
        {
            ANGLE_TRY(sourceImage->orphanSibling(context, this));
        }
        mSourcesOf.clear();
    }

    return angle::Result::Continue;
}

void ImageSibling::addImageSource(egl::Image *imageSource)
{
    ASSERT(imageSource != nullptr);
    mSourcesOf.insert(imageSource);
}

void ImageSibling::removeImageSource(egl::Image *imageSource)
{
    ASSERT(mSourcesOf.find(imageSource) != mSourcesOf.end());
    mSourcesOf.erase(imageSource);
}

bool ImageSibling::isEGLImageTarget() const
{
    return (mTargetOf.get() != nullptr);
}

gl::InitState ImageSibling::sourceEGLImageInitState() const
{
    ASSERT(isEGLImageTarget());
    return mTargetOf->sourceInitState();
}

void ImageSibling::setSourceEGLImageInitState(gl::InitState initState) const
{
    ASSERT(isEGLImageTarget());
    mTargetOf->setInitState(initState);
}

bool ImageSibling::isRenderable(const gl::Context *context,
                                GLenum binding,
                                const gl::ImageIndex &imageIndex) const
{
    ASSERT(isEGLImageTarget());
    return mTargetOf->isRenderable(context);
}

bool ImageSibling::isYUV() const
{
    return mTargetOf.get() && mTargetOf->isYUV();
}

bool ImageSibling::isExternalImageWithoutIndividualSync() const
{
    return mTargetOf.get() && mTargetOf->isExternalImageWithoutIndividualSync();
}

bool ImageSibling::hasFrontBufferUsage() const
{
    return mTargetOf.get() && mTargetOf->hasFrontBufferUsage();
}

bool ImageSibling::hasProtectedContent() const
{
    return mTargetOf.get() && mTargetOf->hasProtectedContent();
}

void ImageSibling::notifySiblings(angle::SubjectMessage message)
{
    if (mTargetOf.get())
    {
        mTargetOf->notifySiblings(this, message);
    }
    for (Image *source : mSourcesOf)
    {
        source->notifySiblings(this, message);
    }
}

ExternalImageSibling::ExternalImageSibling(rx::EGLImplFactory *factory,
                                           const gl::Context *context,
                                           EGLenum target,
                                           EGLClientBuffer buffer,
                                           const AttributeMap &attribs)
    : mImplementation(factory->createExternalImageSibling(context, target, buffer, attribs)),
      mImplObserverBinding(this, kExternalImageImplSubjectIndex)
{
    mImplObserverBinding.bind(mImplementation.get());
}

ExternalImageSibling::~ExternalImageSibling() = default;

void ExternalImageSibling::onDestroy(const egl::Display *display)
{
    mImplementation->onDestroy(display);
}

Error ExternalImageSibling::initialize(const egl::Display *display, const gl::Context *context)
{
    return mImplementation->initialize(display);
}

gl::Extents ExternalImageSibling::getAttachmentSize(const gl::ImageIndex &imageIndex) const
{
    return mImplementation->getSize();
}

gl::Format ExternalImageSibling::getAttachmentFormat(GLenum binding,
                                                     const gl::ImageIndex &imageIndex) const
{
    return mImplementation->getFormat();
}

GLsizei ExternalImageSibling::getAttachmentSamples(const gl::ImageIndex &imageIndex) const
{
    return static_cast<GLsizei>(mImplementation->getSamples());
}

GLuint ExternalImageSibling::getLevelCount() const
{
    return static_cast<GLuint>(mImplementation->getLevelCount());
}

bool ExternalImageSibling::isRenderable(const gl::Context *context,
                                        GLenum binding,
                                        const gl::ImageIndex &imageIndex) const
{
    return mImplementation->isRenderable(context);
}

bool ExternalImageSibling::isTextureable(const gl::Context *context) const
{
    return mImplementation->isTexturable(context);
}

bool ExternalImageSibling::isYUV() const
{
    return mImplementation->isYUV();
}

bool ExternalImageSibling::hasFrontBufferUsage() const
{
    return mImplementation->hasFrontBufferUsage();
}

bool ExternalImageSibling::isCubeMap() const
{
    return mImplementation->isCubeMap();
}

bool ExternalImageSibling::hasProtectedContent() const
{
    return mImplementation->hasProtectedContent();
}

void ExternalImageSibling::onAttach(const gl::Context *context, rx::UniqueSerial framebufferSerial)
{}

void ExternalImageSibling::onDetach(const gl::Context *context, rx::UniqueSerial framebufferSerial)
{}

GLuint ExternalImageSibling::getId() const
{
    UNREACHABLE();
    return 0;
}

gl::InitState ExternalImageSibling::initState(GLenum binding,
                                              const gl::ImageIndex &imageIndex) const
{
    return gl::InitState::Initialized;
}

void ExternalImageSibling::setInitState(GLenum binding,
                                        const gl::ImageIndex &imageIndex,
                                        gl::InitState initState)
{}

rx::ExternalImageSiblingImpl *ExternalImageSibling::getImplementation() const
{
    return mImplementation.get();
}

void ExternalImageSibling::onSubjectStateChange(angle::SubjectIndex index,
                                                angle::SubjectMessage message)
{
    onStateChange(message);
}

rx::FramebufferAttachmentObjectImpl *ExternalImageSibling::getAttachmentImpl() const
{
    return mImplementation.get();
}

ImageState::ImageState(ImageID id,
                       EGLenum target,
                       ImageSibling *buffer,
                       const AttributeMap &attribs)
    : id(id),
      label(nullptr),
      target(target),
      imageIndex(GetImageIndex(target, attribs)),
      source(buffer),
      format(GL_NONE),
      yuv(false),
      cubeMap(false),
      size(),
      samples(),
      levelCount(1),
      colorspace(
          static_cast<EGLenum>(attribs.get(EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_DEFAULT_EXT))),
      hasProtectedContent(static_cast<bool>(attribs.get(EGL_PROTECTED_CONTENT_EXT, EGL_FALSE)))
{}

ImageState::~ImageState() {}

Image::Image(rx::EGLImplFactory *factory,
             ImageID id,
             const gl::Context *context,
             EGLenum target,
             ImageSibling *buffer,
             const AttributeMap &attribs)
    : mState(id, target, buffer, attribs),
      mImplementation(factory->createImage(mState, context, target, attribs)),
      mOrphanedAndNeedsInit(false),
      mContextMutex(nullptr)
{
    ASSERT(mImplementation != nullptr);
    ASSERT(buffer != nullptr);

    if (kIsContextMutexEnabled)
    {
        if (context != nullptr)
        {
            mContextMutex = context->getContextMutex().getRoot();
            ASSERT(mContextMutex->isReferenced());
        }
        else
        {
            mContextMutex = new ContextMutex();
        }
        mContextMutex->addRef();
    }

    mState.source->addImageSource(this);
}

void Image::onDestroy(const Display *display)
{
    // All targets should hold a ref to the egl image and it should not be deleted until there are
    // no siblings left.
    ASSERT([&] {
        std::unique_lock lock(mState.targetsLock);
        return mState.targets.empty();
    }());

    // Make sure the implementation gets a chance to clean up before we delete the source.
    mImplementation->onDestroy(display);

    // Tell the source that it is no longer used by this image
    if (mState.source != nullptr)
    {
        mState.source->removeImageSource(this);

        // If the source is an external object, delete it
        if (IsExternalImageTarget(mState.target))
        {
            ExternalImageSibling *externalSibling = rx::GetAs<ExternalImageSibling>(mState.source);
            externalSibling->onDestroy(display);
            delete externalSibling;
        }

        mState.source = nullptr;
    }
}

Image::~Image()
{
    SafeDelete(mImplementation);

    if (mContextMutex != nullptr)
    {
        mContextMutex->release();
        mContextMutex = nullptr;
    }
}

void Image::setLabel(EGLLabelKHR label)
{
    mState.label = label;
}

EGLLabelKHR Image::getLabel() const
{
    return mState.label;
}

void Image::addTargetSibling(ImageSibling *sibling)
{
    std::unique_lock lock(mState.targetsLock);
    mState.targets.insert(sibling);
}

angle::Result Image::orphanSibling(const gl::Context *context, ImageSibling *sibling)
{
    ASSERT(sibling != nullptr);

    // notify impl
    ANGLE_TRY(mImplementation->orphan(context, sibling));

    if (mState.source == sibling)
    {
        // The external source of an image cannot be redefined so it cannot be orphaned.
        ASSERT(!IsExternalImageTarget(mState.target));

        // If the sibling is the source, it cannot be a target.
        ASSERT([&] {
            std::unique_lock lock(mState.targetsLock);
            return mState.targets.find(sibling) == mState.targets.end();
        }());
        mState.source = nullptr;
        mOrphanedAndNeedsInit =
            (sibling->initState(GL_NONE, mState.imageIndex) == gl::InitState::MayNeedInit);
    }
    else
    {
        std::unique_lock lock(mState.targetsLock);
        mState.targets.erase(sibling);
    }

    return angle::Result::Continue;
}

const gl::Format &Image::getFormat() const
{
    return mState.format;
}

bool Image::isRenderable(const gl::Context *context) const
{
    return mIsRenderable;
}

bool Image::isTexturable(const gl::Context *context) const
{
    return mIsTexturable;
}

bool Image::isYUV() const
{
    return mState.yuv;
}

bool Image::isExternalImageWithoutIndividualSync() const
{
    // Only Vulkan images are individually synced.
    return IsExternalImageTarget(mState.target) && mState.target != EGL_VULKAN_IMAGE_ANGLE;
}

bool Image::hasFrontBufferUsage() const
{
    if (IsExternalImageTarget(mState.target))
    {
        ExternalImageSibling *externalSibling = rx::GetAs<ExternalImageSibling>(mState.source);
        return externalSibling->hasFrontBufferUsage();
    }

    return false;
}

bool Image::isCubeMap() const
{
    return mState.cubeMap;
}

size_t Image::getWidth() const
{
    return mState.size.width;
}

size_t Image::getHeight() const
{
    return mState.size.height;
}

const gl::Extents &Image::getExtents() const
{
    return mState.size;
}

bool Image::isLayered() const
{
    return mState.imageIndex.isLayered();
}

size_t Image::getSamples() const
{
    return mState.samples;
}

GLuint Image::getLevelCount() const
{
    return mState.levelCount;
}

bool Image::hasProtectedContent() const
{
    return mState.hasProtectedContent;
}

bool Image::isFixedRatedCompression(const gl::Context *context) const
{
    return mImplementation->isFixedRatedCompression(context);
}

rx::ImageImpl *Image::getImplementation() const
{
    return mImplementation;
}

Error Image::initialize(const Display *display, const gl::Context *context)
{
    if (IsExternalImageTarget(mState.target))
    {
        ExternalImageSibling *externalSibling = rx::GetAs<ExternalImageSibling>(mState.source);
        ANGLE_TRY(externalSibling->initialize(display, context));

        mState.hasProtectedContent = externalSibling->hasProtectedContent();
        mState.levelCount          = externalSibling->getLevelCount();
        mState.cubeMap             = externalSibling->isCubeMap();

        // External siblings can be YUV
        mState.yuv = externalSibling->isYUV();
    }

    mState.format = mState.source->getAttachmentFormat(GL_NONE, mState.imageIndex);

    if (mState.colorspace != EGL_GL_COLORSPACE_DEFAULT_EXT)
    {
        GLenum nonLinearFormat = mState.format.info->sizedInternalFormat;
        if (!gl::ColorspaceFormatOverride(mState.colorspace, &nonLinearFormat))
        {
            // the colorspace format is not supported
            return egl::EglBadMatch();
        }
        mState.format = gl::Format(nonLinearFormat);
    }

    if (!IsExternalImageTarget(mState.target))
    {
        // Account for the fact that GL_ANGLE_yuv_internal_format extension maybe enabled,
        // in which case the internal format itself could be YUV.
        mState.yuv = gl::IsYuvFormat(mState.format.info->sizedInternalFormat);
    }

    mState.size    = mState.source->getAttachmentSize(mState.imageIndex);
    mState.samples = mState.source->getAttachmentSamples(mState.imageIndex);

    if (IsTextureTarget(mState.target))
    {
        mState.size.depth = 1;
    }

    Error error = mImplementation->initialize(display);
    if (error.isError())
    {
        return error;
    }

    if (IsTextureTarget(mState.target))
    {
        mIsTexturable = true;
        mIsRenderable = mState.format.info->textureAttachmentSupport(context->getClientVersion(),
                                                                     context->getExtensions());
    }
    else if (IsRenderbufferTarget(mState.target))
    {
        mIsTexturable = true;
        mIsRenderable = mState.format.info->renderbufferSupport(context->getClientVersion(),
                                                                context->getExtensions());
    }
    else if (IsExternalImageTarget(mState.target))
    {
        ASSERT(mState.source != nullptr);
        mIsTexturable = rx::GetAs<ExternalImageSibling>(mState.source)->isTextureable(context);
        mIsRenderable = rx::GetAs<ExternalImageSibling>(mState.source)
                            ->isRenderable(context, GL_NONE, gl::ImageIndex());
    }
    else
    {
        UNREACHABLE();
    }

    return NoError();
}

bool Image::orphaned() const
{
    return (mState.source == nullptr);
}

gl::InitState Image::sourceInitState() const
{
    if (orphaned())
    {
        return mOrphanedAndNeedsInit ? gl::InitState::MayNeedInit : gl::InitState::Initialized;
    }

    return mState.source->initState(GL_NONE, mState.imageIndex);
}

void Image::setInitState(gl::InitState initState)
{
    if (orphaned())
    {
        mOrphanedAndNeedsInit = false;
    }

    return mState.source->setInitState(GL_NONE, mState.imageIndex, initState);
}

Error Image::exportVkImage(void *vkImage, void *vkImageCreateInfo)
{
    return mImplementation->exportVkImage(vkImage, vkImageCreateInfo);
}

void Image::notifySiblings(const ImageSibling *notifier, angle::SubjectMessage message)
{
    if (mState.source && mState.source != notifier)
    {
        mState.source->onSubjectStateChange(rx::kTextureImageSiblingMessageIndex, message);
    }

    std::unique_lock lock(mState.targetsLock);
    for (ImageSibling *target : mState.targets)
    {
        if (target != notifier)
        {
            target->onSubjectStateChange(rx::kTextureImageSiblingMessageIndex, message);
        }
    }
}

}  // namespace egl
