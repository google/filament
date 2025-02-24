//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceGL.cpp: OpenGL implementation of egl::Surface

#include "libANGLE/renderer/gl/SurfaceGL.h"

#include "libANGLE/Context.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/gl/BlitGL.h"
#include "libANGLE/renderer/gl/ContextGL.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"

namespace rx
{

SurfaceGL::SurfaceGL(const egl::SurfaceState &state) : SurfaceImpl(state) {}

SurfaceGL::~SurfaceGL() {}

egl::Error SurfaceGL::getSyncValues(EGLuint64KHR *ust, EGLuint64KHR *msc, EGLuint64KHR *sbc)
{
    UNREACHABLE();
    return egl::EglBadSurface();
}

egl::Error SurfaceGL::getMscRate(EGLint *numerator, EGLint *denominator)
{
    UNIMPLEMENTED();
    return egl::EglBadAccess();
}

angle::Result SurfaceGL::initializeContents(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex)
{
    FramebufferGL *framebufferGL = GetImplAs<FramebufferGL>(context->getFramebuffer({0}));
    ASSERT(framebufferGL->isDefault());

    BlitGL *blitter = GetBlitGL(context);

    switch (binding)
    {
        case GL_BACK:
        {
            gl::DrawBufferMask colorAttachments{0};
            ANGLE_TRY(
                blitter->clearFramebuffer(context, colorAttachments, false, false, framebufferGL));
        }
        break;

        case GL_DEPTH:
        case GL_STENCIL:
            ANGLE_TRY(blitter->clearFramebuffer(context, gl::DrawBufferMask(), true, true,
                                                framebufferGL));
            break;

        default:
            UNREACHABLE();
            break;
    }

    return angle::Result::Continue;
}

bool SurfaceGL::hasEmulatedAlphaChannel() const
{
    return false;
}

egl::Error SurfaceGL::attachToFramebuffer(const gl::Context *context, gl::Framebuffer *framebuffer)
{
    return egl::NoError();
}

egl::Error SurfaceGL::detachFromFramebuffer(const gl::Context *context,
                                            gl::Framebuffer *framebuffer)
{
    return egl::NoError();
}

}  // namespace rx
