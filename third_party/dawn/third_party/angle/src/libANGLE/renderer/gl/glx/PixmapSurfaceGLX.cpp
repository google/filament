//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PixmapSurfaceGLX.cpp: GLX implementation of egl::Surface for Pixmaps

#include "common/debug.h"
#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"

#include "libANGLE/renderer/gl/glx/DisplayGLX.h"

#include "libANGLE/renderer/gl/glx/FunctionsGLX.h"
#include "libANGLE/renderer/gl/glx/PixmapSurfaceGLX.h"
#include "libANGLE/renderer/gl/glx/glx_utils.h"

#include <iostream>

namespace rx
{

namespace
{

int EGLTextureFormatToGLXTextureFormat(EGLint textureFormat)
{
    switch (textureFormat)
    {
        case EGL_NO_TEXTURE:
            return GLX_TEXTURE_FORMAT_NONE_EXT;
        case EGL_TEXTURE_RGB:
            return GLX_TEXTURE_FORMAT_RGB_EXT;
        case EGL_TEXTURE_RGBA:
            return GLX_TEXTURE_FORMAT_RGBA_EXT;
        default:
            UNREACHABLE();
            return GLX_TEXTURE_FORMAT_NONE_EXT;
    }
}

int EGLTextureTargetToGLXTextureTarget(EGLint textureTarget)
{
    switch (textureTarget)
    {
        case EGL_NO_TEXTURE:
            return 0;
        case EGL_TEXTURE_2D:
            return GLX_TEXTURE_2D_EXT;
        default:
            UNREACHABLE();
            return 0;
    }
}

int EGLBufferToGLXBuffer(EGLint buffer)
{
    switch (buffer)
    {
        case EGL_BACK_BUFFER:
            return GLX_BACK_EXT;
        default:
            UNREACHABLE();
            return 0;
    }
}

}  // namespace

PixmapSurfaceGLX::PixmapSurfaceGLX(const egl::SurfaceState &state,
                                   Pixmap pixmap,
                                   Display *display,
                                   const FunctionsGLX &glx,
                                   glx::FBConfig fbConfig)
    : SurfaceGLX(state),
      mWidth(0),
      mHeight(0),
      mGLX(glx),
      mFBConfig(fbConfig),
      mXPixmap(pixmap),
      mGLXPixmap(0),
      mDisplay(display)
{}

PixmapSurfaceGLX::~PixmapSurfaceGLX()
{
    if (mGLXPixmap)
    {
        mGLX.destroyPixmap(mGLXPixmap);
    }
}

egl::Error PixmapSurfaceGLX::initialize(const egl::Display *display)
{
    DisplayGLX *displayGLX = GetImplAs<DisplayGLX>(display);

    {
        Window rootWindow;
        int x                    = 0;
        int y                    = 0;
        unsigned int borderWidth = 0;
        unsigned int depth       = 0;
        int status = XGetGeometry(mDisplay, mXPixmap, &rootWindow, &x, &y, &mWidth, &mHeight,
                                  &borderWidth, &depth);
        if (!status)
        {
            return egl::EglBadSurface() << "XGetGeometry query failed on pixmap surface: "
                                        << x11::XErrorToString(mDisplay, status);
        }
    }

    std::vector<int> pixmapAttribs;
    if (mState.attributes.contains(EGL_TEXTURE_FORMAT))
    {
        pixmapAttribs.push_back(GLX_TEXTURE_FORMAT_EXT);
        pixmapAttribs.push_back(
            EGLTextureFormatToGLXTextureFormat(mState.attributes.getAsInt(EGL_TEXTURE_FORMAT)));
    }
    if (mState.attributes.contains(EGL_TEXTURE_TARGET))
    {
        pixmapAttribs.push_back(GLX_TEXTURE_TARGET_EXT);
        pixmapAttribs.push_back(
            EGLTextureTargetToGLXTextureTarget(mState.attributes.getAsInt(EGL_TEXTURE_TARGET)));
    }

    pixmapAttribs.push_back(None);

    mGLXPixmap = mGLX.createPixmap(mFBConfig, mXPixmap, pixmapAttribs.data());
    if (!mGLXPixmap)
    {
        return egl::EglBadAlloc() << "Failed to create a native GLX pixmap.";
    }

    XFlush(mDisplay);
    displayGLX->syncXCommands(false);

    return egl::NoError();
}

egl::Error PixmapSurfaceGLX::makeCurrent(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error PixmapSurfaceGLX::swap(const gl::Context *context)
{
    UNREACHABLE();
    return egl::NoError();
}

egl::Error PixmapSurfaceGLX::postSubBuffer(const gl::Context *context,
                                           EGLint x,
                                           EGLint y,
                                           EGLint width,
                                           EGLint height)
{
    UNREACHABLE();
    return egl::NoError();
}

egl::Error PixmapSurfaceGLX::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNREACHABLE();
    return egl::NoError();
}

egl::Error PixmapSurfaceGLX::bindTexImage(const gl::Context *context,
                                          gl::Texture *texture,
                                          EGLint buffer)
{
    const int attribs[] = {None};
    mGLX.bindTexImageEXT(mGLXPixmap, EGLBufferToGLXBuffer(buffer), attribs);
    return egl::NoError();
}

egl::Error PixmapSurfaceGLX::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    mGLX.releaseTexImageEXT(mGLXPixmap, EGLBufferToGLXBuffer(buffer));
    return egl::NoError();
}

void PixmapSurfaceGLX::setSwapInterval(const egl::Display *display, EGLint interval) {}

EGLint PixmapSurfaceGLX::getWidth() const
{
    return mWidth;
}

EGLint PixmapSurfaceGLX::getHeight() const
{
    return mHeight;
}

EGLint PixmapSurfaceGLX::isPostSubBufferSupported() const
{
    UNREACHABLE();
    return EGL_FALSE;
}

EGLint PixmapSurfaceGLX::getSwapBehavior() const
{
    return EGL_BUFFER_DESTROYED;
}

egl::Error PixmapSurfaceGLX::checkForResize()
{
    // The size of pbuffers never change
    return egl::NoError();
}

glx::Drawable PixmapSurfaceGLX::getDrawable() const
{
    return mGLXPixmap;
}

}  // namespace rx
