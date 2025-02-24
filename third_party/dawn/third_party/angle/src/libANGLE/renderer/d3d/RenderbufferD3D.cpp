//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderbufferD3d.cpp: Implements the RenderbufferD3D class, a specialization of RenderbufferImpl

#include "libANGLE/renderer/d3d/RenderbufferD3D.h"

#include "libANGLE/Context.h"
#include "libANGLE/Image.h"
#include "libANGLE/renderer/d3d/ContextD3D.h"
#include "libANGLE/renderer/d3d/EGLImageD3D.h"
#include "libANGLE/renderer/d3d/RenderTargetD3D.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"

namespace rx
{
RenderbufferD3D::RenderbufferD3D(const gl::RenderbufferState &state, RendererD3D *renderer)
    : RenderbufferImpl(state), mRenderer(renderer), mRenderTarget(nullptr), mImage(nullptr)
{}

RenderbufferD3D::~RenderbufferD3D()
{
    SafeDelete(mRenderTarget);
    mImage = nullptr;
}

void RenderbufferD3D::onDestroy(const gl::Context *context)
{
    SafeDelete(mRenderTarget);
}

angle::Result RenderbufferD3D::setStorage(const gl::Context *context,
                                          GLenum internalformat,
                                          GLsizei width,
                                          GLsizei height)
{
    return setStorageMultisample(context, 0, internalformat, width, height,
                                 gl::MultisamplingMode::Regular);
}

angle::Result RenderbufferD3D::setStorageMultisample(const gl::Context *context,
                                                     GLsizei samples,
                                                     GLenum internalformat,
                                                     GLsizei width,
                                                     GLsizei height,
                                                     gl::MultisamplingMode mode)
{
    // TODO: Correctly differentiate between normal multisampling and render to texture.  In the
    // latter case, the renderbuffer must be automatically resolved when rendering is broken and
    // operations performed on it (such as blit, copy etc) should use the resolved image.
    // http://anglebug.com/42261786.

    // If the renderbuffer parameters are queried, the calling function
    // will expect one of the valid renderbuffer formats for use in
    // glRenderbufferStorage, but we should create depth and stencil buffers
    // as DEPTH24_STENCIL8
    GLenum creationFormat = internalformat;
    if (internalformat == GL_DEPTH_COMPONENT16 || internalformat == GL_STENCIL_INDEX8)
    {
        creationFormat = GL_DEPTH24_STENCIL8_OES;
    }

    // ANGLE_framebuffer_multisample states GL_OUT_OF_MEMORY is generated on a failure to create
    // the specified storage.
    // Because ES 3.0 already knows the exact number of supported samples, it would already have
    // been validated and generated GL_INVALID_VALUE.
    const gl::TextureCaps &formatCaps = mRenderer->getNativeTextureCaps().get(creationFormat);
    ANGLE_CHECK_GL_ALLOC(GetImplAs<ContextD3D>(context),
                         static_cast<uint32_t>(samples) <= formatCaps.getMaxSamples());

    RenderTargetD3D *newRT = nullptr;
    ANGLE_TRY(
        mRenderer->createRenderTarget(context, width, height, creationFormat, samples, &newRT));

    SafeDelete(mRenderTarget);
    mImage        = nullptr;
    mRenderTarget = newRT;

    return angle::Result::Continue;
}

angle::Result RenderbufferD3D::setStorageEGLImageTarget(const gl::Context *context,
                                                        egl::Image *image)
{
    mImage = GetImplAs<EGLImageD3D>(image);
    SafeDelete(mRenderTarget);

    return angle::Result::Continue;
}

angle::Result RenderbufferD3D::getRenderTarget(const gl::Context *context,
                                               RenderTargetD3D **outRenderTarget)
{
    if (mImage)
    {
        return mImage->getRenderTarget(context, outRenderTarget);
    }
    else
    {
        *outRenderTarget = mRenderTarget;
        return angle::Result::Continue;
    }
}

angle::Result RenderbufferD3D::getAttachmentRenderTarget(const gl::Context *context,
                                                         GLenum binding,
                                                         const gl::ImageIndex &imageIndex,
                                                         GLsizei samples,
                                                         FramebufferAttachmentRenderTarget **rtOut)
{
    return getRenderTarget(context, reinterpret_cast<RenderTargetD3D **>(rtOut));
}

angle::Result RenderbufferD3D::initializeContents(const gl::Context *context,
                                                  GLenum binding,
                                                  const gl::ImageIndex &imageIndex)
{
    RenderTargetD3D *renderTarget = nullptr;
    ANGLE_TRY(getRenderTarget(context, &renderTarget));
    return mRenderer->initRenderTarget(context, renderTarget);
}

}  // namespace rx
