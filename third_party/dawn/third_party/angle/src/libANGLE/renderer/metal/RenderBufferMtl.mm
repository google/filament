//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RenderBufferMtl.mm:
//    Implements the class methods for RenderBufferMtl.
//

#include "libANGLE/renderer/metal/RenderBufferMtl.h"

#include "libANGLE/ErrorStrings.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/ImageMtl.h"
#include "libANGLE/renderer/metal/mtl_format_utils.h"
#include "libANGLE/renderer/metal/mtl_utils.h"

namespace rx
{

RenderbufferMtl::RenderbufferMtl(const gl::RenderbufferState &state) : RenderbufferImpl(state) {}

RenderbufferMtl::~RenderbufferMtl() {}

void RenderbufferMtl::onDestroy(const gl::Context *context)
{
    releaseTexture();
}

void RenderbufferMtl::releaseTexture()
{
    mTexture           = nullptr;
    mImplicitMSTexture = nullptr;
}

angle::Result RenderbufferMtl::setStorageImpl(const gl::Context *context,
                                              GLsizei samples,
                                              GLenum internalformat,
                                              GLsizei width,
                                              GLsizei height,
                                              gl::MultisamplingMode mode)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);

    if (mTexture != nullptr && mTexture->valid())
    {
        // Check against the state if we need to recreate the storage.
        if (internalformat != mState.getFormat().info->internalFormat ||
            width != mState.getWidth() || height != mState.getHeight() ||
            samples != mState.getSamples())
        {
            releaseTexture();
        }
    }

    const gl::InternalFormat &internalFormat = gl::GetSizedInternalFormatInfo(internalformat);
    angle::FormatID angleFormatId =
        angle::Format::InternalFormatToID(internalFormat.sizedInternalFormat);
    mFormat = contextMtl->getPixelFormat(angleFormatId);

    uint32_t actualSamples;
    if (samples == 0)
    {
        actualSamples = 1;
    }
    else
    {
        // We always start at at least 2 samples
        actualSamples = static_cast<uint32_t>(std::max<size_t>(2, samples));

        const gl::TextureCaps &textureCaps =
            contextMtl->getTextureCaps().get(mFormat.intendedFormatId);
        actualSamples = textureCaps.getNearestSamples(actualSamples);
        ANGLE_CHECK(contextMtl, actualSamples != 0, gl::err::kInternalError, GL_INVALID_VALUE);
    }

    if ((mTexture == nullptr || !mTexture->valid()) && (width != 0 && height != 0))
    {
        if (actualSamples == 1 || mode == gl::MultisamplingMode::MultisampledRenderToTexture)
        {
            ANGLE_TRY(mtl::Texture::Make2DTexture(
                contextMtl, mFormat, static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1,
                /* renderTargetOnly */ false,
                /* allowFormatView */ mFormat.hasDepthAndStencilBits(), &mTexture));

            if (mode == gl::MultisamplingMode::MultisampledRenderToTexture)
            {
                // This format must supports implicit resolve
                ASSERT(mFormat.getCaps().resolve);

                ANGLE_TRY(mtl::Texture::MakeMemoryLess2DMSTexture(
                    contextMtl, mFormat, static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height), actualSamples, &mImplicitMSTexture));
            }
        }
        else
        {
            ANGLE_TRY(mtl::Texture::Make2DMSTexture(
                contextMtl, mFormat, static_cast<uint32_t>(width), static_cast<uint32_t>(height),
                actualSamples,
                /* renderTargetOnly */ false,
                /* allowFormatView */ mFormat.hasDepthAndStencilBits(), &mTexture));
        }

        mRenderTarget.setWithImplicitMSTexture(mTexture, mImplicitMSTexture,
                                               mtl::kZeroNativeMipLevel, 0, mFormat);

        // For emulated channels that GL texture intends to not have,
        // we need to initialize their content.
        bool emulatedChannels = mtl::IsFormatEmulated(mFormat);
        if (emulatedChannels)
        {
            gl::ImageIndex index;

            if (mTexture->samples() > 1)
            {
                index = gl::ImageIndex::Make2DMultisample();
            }
            else
            {
                index = gl::ImageIndex::Make2D(0);
            }

            ANGLE_TRY(mtl::InitializeTextureContents(context, mTexture, mFormat,
                                                     mtl::ImageNativeIndex(index, 0)));
        }  // if (emulatedChannels)
        bool isDepthStencil = mFormat.hasDepthOrStencilBits();
        if (isDepthStencil)
        {
            gl::ImageIndex index;
            if (mTexture->samples() > 1)
            {
                index = gl::ImageIndex::Make2DMultisample();
            }
            else
            {
                index = gl::ImageIndex::Make2D(0);
            }
            ANGLE_TRY(mtl::InitializeDepthStencilTextureContentsGPU(
                context, mTexture, mFormat, mtl::ImageNativeIndex(index, 0)));
        }
    }

    return angle::Result::Continue;
}

angle::Result RenderbufferMtl::setStorage(const gl::Context *context,
                                          GLenum internalformat,
                                          GLsizei width,
                                          GLsizei height)
{
    return setStorageImpl(context, 0, internalformat, width, height,
                          gl::MultisamplingMode::Regular);
}

angle::Result RenderbufferMtl::setStorageMultisample(const gl::Context *context,
                                                     GLsizei samples,
                                                     GLenum internalformat,
                                                     GLsizei width,
                                                     GLsizei height,
                                                     gl::MultisamplingMode mode)
{
    return setStorageImpl(context, samples, internalformat, width, height, mode);
}

angle::Result RenderbufferMtl::setStorageEGLImageTarget(const gl::Context *context,
                                                        egl::Image *image)
{
    releaseTexture();

    ContextMtl *contextMtl = mtl::GetImpl(context);

    ImageMtl *imageMtl = mtl::GetImpl(image);
    mTexture           = imageMtl->getTexture();

    const angle::FormatID angleFormatId =
        angle::Format::InternalFormatToID(image->getFormat().info->sizedInternalFormat);
    mFormat = contextMtl->getPixelFormat(angleFormatId);

    mRenderTarget.set(mTexture, mtl::kZeroNativeMipLevel, 0, mFormat);

    return angle::Result::Continue;
}

angle::Result RenderbufferMtl::getAttachmentRenderTarget(const gl::Context *context,
                                                         GLenum binding,
                                                         const gl::ImageIndex &imageIndex,
                                                         GLsizei samples,
                                                         FramebufferAttachmentRenderTarget **rtOut)
{
    ASSERT(mTexture && mTexture->valid());
    *rtOut = &mRenderTarget;
    return angle::Result::Continue;
}

angle::Result RenderbufferMtl::initializeContents(const gl::Context *context,
                                                  GLenum binding,
                                                  const gl::ImageIndex &imageIndex)
{
    if (imageIndex.valid())
        return mtl::InitializeTextureContents(
            context, mTexture, mFormat, mtl::ImageNativeIndex::FromBaseZeroGLIndex(imageIndex));
    else
        return mtl::InitializeTextureContents(
            context, mTexture, mFormat,
            mtl::ImageNativeIndex::FromBaseZeroGLIndex(gl::ImageIndex::Make2D(0)));
}
}  // namespace rx
