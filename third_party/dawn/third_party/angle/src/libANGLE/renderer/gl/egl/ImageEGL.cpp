//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ImageEGL.cpp: Implements the rx::ImageEGL class.

#include "libANGLE/renderer/gl/egl/ImageEGL.h"

#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/RenderbufferGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/egl/ContextEGL.h"
#include "libANGLE/renderer/gl/egl/ExternalImageSiblingEGL.h"
#include "libANGLE/renderer/gl/egl/FunctionsEGL.h"

namespace rx
{

ImageEGL::ImageEGL(const egl::ImageState &state,
                   const gl::Context *context,
                   EGLenum target,
                   const egl::AttributeMap &attribs,
                   const FunctionsEGL *egl)
    : ImageGL(state),
      mEGL(egl),
      mContext(EGL_NO_CONTEXT),
      mTarget(target),
      mPreserveImage(false),
      mImage(EGL_NO_IMAGE)
{
    if (context)
    {
        mContext = GetImplAs<ContextEGL>(context)->getContext();
    }
    mPreserveImage = attribs.get(EGL_IMAGE_PRESERVED, EGL_FALSE) == EGL_TRUE;
}

ImageEGL::~ImageEGL()
{
    mEGL->destroyImageKHR(mImage);
}

egl::Error ImageEGL::initialize(const egl::Display *display)
{
    EGLClientBuffer buffer = nullptr;
    angle::FastVector<EGLint, 8> attributes;

    if (egl::IsTextureTarget(mTarget))
    {
        attributes.push_back(EGL_GL_TEXTURE_LEVEL);
        attributes.push_back(mState.imageIndex.getLevelIndex());

        if (mState.imageIndex.has3DLayer())
        {
            attributes.push_back(EGL_GL_TEXTURE_ZOFFSET);
            attributes.push_back(mState.imageIndex.getLayerIndex());
        }

        const TextureGL *textureGL = GetImplAs<TextureGL>(GetAs<gl::Texture>(mState.source));
        buffer                = gl_egl::GLObjectHandleToEGLClientBuffer(textureGL->getTextureID());
        mNativeInternalFormat = textureGL->getNativeInternalFormat(mState.imageIndex);
    }
    else if (egl::IsRenderbufferTarget(mTarget))
    {
        const RenderbufferGL *renderbufferGL =
            GetImplAs<RenderbufferGL>(GetAs<gl::Renderbuffer>(mState.source));
        buffer = gl_egl::GLObjectHandleToEGLClientBuffer(renderbufferGL->getRenderbufferID());
        mNativeInternalFormat = renderbufferGL->getNativeInternalFormat();
    }
    else if (egl::IsExternalImageTarget(mTarget))
    {
        const ExternalImageSiblingEGL *externalImageSibling =
            GetImplAs<ExternalImageSiblingEGL>(GetAs<egl::ExternalImageSibling>(mState.source));
        buffer                = externalImageSibling->getBuffer();
        mNativeInternalFormat = externalImageSibling->getFormat().info->sizedInternalFormat;

        // Add any additional attributes this type of image sibline requires
        std::vector<EGLint> tmp_attributes;
        externalImageSibling->getImageCreationAttributes(&tmp_attributes);

        attributes.reserve(attributes.size() + tmp_attributes.size());
        for (EGLint attribute : tmp_attributes)
        {
            attributes.push_back(attribute);
        }
    }
    else
    {
        UNREACHABLE();
    }

    attributes.push_back(EGL_IMAGE_PRESERVED);
    attributes.push_back(mPreserveImage ? EGL_TRUE : EGL_FALSE);

    attributes.push_back(EGL_NONE);

    egl::Display::GetCurrentThreadUnlockedTailCall()->add([egl = mEGL, &image = mImage,
                                                           context = mContext, target = mTarget,
                                                           buffer, attributes](void *resultOut) {
        image = egl->createImageKHR(context, target, buffer, attributes.data());

        // If image creation failed, force the return value of eglCreateImage to EGL_NO_IMAGE. This
        // won't delete this image object but a driver error is unexpected at this point.
        if (image == EGL_NO_IMAGE)
        {
            ERR() << "eglCreateImage failed with " << gl::FmtHex(egl->getError());
            *static_cast<EGLImage *>(resultOut) = EGL_NO_IMAGE;
        }
    });

    return egl::NoError();
}

angle::Result ImageEGL::orphan(const gl::Context *context, egl::ImageSibling *sibling)
{
    // Nothing to do, the native EGLImage will orphan automatically.
    return angle::Result::Continue;
}

angle::Result ImageEGL::setTexture2D(const gl::Context *context,
                                     gl::TextureType type,
                                     TextureGL *texture,
                                     GLenum *outInternalFormat)
{
    const FunctionsGL *functionsGL = GetFunctionsGL(context);
    StateManagerGL *stateManager   = GetStateManagerGL(context);

    // Make sure this texture is bound
    stateManager->bindTexture(type, texture->getTextureID());

    // Bind the image to the texture
    functionsGL->eGLImageTargetTexture2DOES(ToGLenum(type), mImage);
    *outInternalFormat = mNativeInternalFormat;

    return angle::Result::Continue;
}

angle::Result ImageEGL::setRenderbufferStorage(const gl::Context *context,
                                               RenderbufferGL *renderbuffer,
                                               GLenum *outInternalFormat)
{
    const FunctionsGL *functionsGL = GetFunctionsGL(context);
    StateManagerGL *stateManager   = GetStateManagerGL(context);

    // Make sure this renderbuffer is bound
    stateManager->bindRenderbuffer(GL_RENDERBUFFER, renderbuffer->getRenderbufferID());

    // Bind the image to the renderbuffer
    functionsGL->eGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, mImage);
    *outInternalFormat = mNativeInternalFormat;

    return angle::Result::Continue;
}

}  // namespace rx
