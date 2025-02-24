//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceGLX.cpp: GLX implementation of egl::Surface for windows

#include "common/debug.h"

#include "libANGLE/renderer/gl/glx/DisplayGLX.h"

#include "libANGLE/renderer/gl/glx/FunctionsGLX.h"
#include "libANGLE/renderer/gl/glx/WindowSurfaceGLX.h"

namespace rx
{

static int IgnoreX11Errors(Display *, XErrorEvent *)
{
    return 0;
}

WindowSurfaceGLX::WindowSurfaceGLX(const egl::SurfaceState &state,
                                   const FunctionsGLX &glx,
                                   DisplayGLX *glxDisplay,
                                   Window window,
                                   Display *display,
                                   glx::FBConfig fbConfig)
    : SurfaceGLX(state),
      mParent(window),
      mWindow(0),
      mDisplay(display),
      mUseChildWindow(false),
      mParentWidth(0),
      mParentHeight(0),
      mGLX(glx),
      mGLXDisplay(glxDisplay),
      mFBConfig(fbConfig),
      mGLXWindow(0)
{}

WindowSurfaceGLX::~WindowSurfaceGLX()
{
    if (mGLXWindow)
    {
        mGLX.destroyWindow(mGLXWindow);
    }

    if (mUseChildWindow && mWindow)
    {
        // When destroying the window, it may happen that the window has already been
        // destroyed by the application (this happens in Chromium). There is no way to
        // atomically check that a window exists and to destroy it so instead we call
        // XDestroyWindow, ignoring any errors.
        auto oldErrorHandler = XSetErrorHandler(IgnoreX11Errors);
        XDestroyWindow(mDisplay, mWindow);
        XSync(mDisplay, False);
        XSetErrorHandler(oldErrorHandler);
    }

    mGLXDisplay->syncXCommands(true);
}

egl::Error WindowSurfaceGLX::initialize(const egl::Display *display)
{
    mUseChildWindow = !mGLXDisplay->isWindowVisualIdSpecified();

    XVisualInfo *visualInfo = nullptr;
    Colormap colormap       = 0;
    if (!mUseChildWindow)
    {
        XWindowAttributes windowAttributes;
        XGetWindowAttributes(mDisplay, mParent, &windowAttributes);
        unsigned long visualId = windowAttributes.visual->visualid;
        // If the window's visual ID is different from the one provided by
        // ANGLE_X11_VISUAL_ID, fallback to using a child window.
        if (!mGLXDisplay->isMatchingWindowVisualId(visualId))
        {
            mUseChildWindow = true;
        }
    }
    if (mUseChildWindow)
    {
        // The visual of the X window, GLX window and GLX context must match,
        // however we received a user-created window that can have any visual
        // and wouldn't work with our GLX context. To work in all cases, we
        // create a child window with the right visual that covers all of its
        // parent.
        visualInfo = mGLX.getVisualFromFBConfig(mFBConfig);
        if (!visualInfo)
        {
            return egl::EglBadNativeWindow()
                   << "Failed to get the XVisualInfo for the child window.";
        }
        Visual *visual = visualInfo->visual;

        if (!getWindowDimensions(mParent, &mParentWidth, &mParentHeight))
        {
            return egl::EglBadNativeWindow() << "Failed to get the parent window's dimensions.";
        }

        // The depth, colormap and visual must match otherwise we get a X error
        // so we specify the colormap attribute. Also we do not want the window
        // to be taken into account for input so we specify the event and
        // do-not-propagate masks to 0 (the defaults). Finally we specify the
        // border pixel attribute so that we can use a different visual depth
        // than our parent (seems like X uses that as a condition to render
        // the subwindow in a different buffer)
        XSetWindowAttributes attributes;
        unsigned long attributeMask = CWColormap | CWBorderPixel;

        colormap = XCreateColormap(mDisplay, mParent, visual, AllocNone);
        if (!colormap)
        {
            XFree(visualInfo);
            return egl::EglBadNativeWindow()
                   << "Failed to create the Colormap for the child window.";
        }
        attributes.colormap     = colormap;
        attributes.border_pixel = 0;

        // TODO(cwallez) set up our own error handler to see if the call failed
        mWindow = XCreateWindow(mDisplay, mParent, 0, 0, mParentWidth, mParentHeight, 0,
                                visualInfo->depth, InputOutput, visual, attributeMask, &attributes);
    }

    mGLXWindow = mGLX.createWindow(mFBConfig, (mUseChildWindow ? mWindow : mParent), nullptr);

    if (mUseChildWindow)
    {
        XMapWindow(mDisplay, mWindow);
    }

    XFlush(mDisplay);

    if (mUseChildWindow)
    {
        XFree(visualInfo);
        XFreeColormap(mDisplay, colormap);
    }

    mGLXDisplay->syncXCommands(true);

    return egl::NoError();
}

egl::Error WindowSurfaceGLX::makeCurrent(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error WindowSurfaceGLX::swap(const gl::Context *context)
{
    // We need to swap before resizing as some drivers clobber the back buffer
    // when the window is resized.
    mGLXDisplay->setSwapInterval(mGLXWindow, &mSwapControl);
    mGLX.swapBuffers(mGLXWindow);

    if (mUseChildWindow)
    {
        egl::Error error = checkForResize();
        if (error.isError())
        {
            return error;
        }
    }

    return egl::NoError();
}

egl::Error WindowSurfaceGLX::postSubBuffer(const gl::Context *context,
                                           EGLint x,
                                           EGLint y,
                                           EGLint width,
                                           EGLint height)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

egl::Error WindowSurfaceGLX::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

egl::Error WindowSurfaceGLX::bindTexImage(const gl::Context *context,
                                          gl::Texture *texture,
                                          EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

egl::Error WindowSurfaceGLX::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

void WindowSurfaceGLX::setSwapInterval(const egl::Display *display, EGLint interval)
{
    mSwapControl.targetSwapInterval = interval;
}

EGLint WindowSurfaceGLX::getWidth() const
{
    if (mUseChildWindow)
    {
        // If there's a child window, the size of the window is always the same as the cached
        // size of its parent.
        return mParentWidth;
    }
    else
    {
        unsigned int parentWidth, parentHeight;
        if (!getWindowDimensions(mParent, &parentWidth, &parentHeight))
        {
            return mParentWidth;
        }
        return parentWidth;
    }
}

EGLint WindowSurfaceGLX::getHeight() const
{
    if (mUseChildWindow)
    {
        // If there's a child window, the size of the window is always the same as the cached
        // size of its parent.
        return mParentHeight;
    }
    else
    {
        unsigned int parentWidth, parentHeight;
        if (!getWindowDimensions(mParent, &parentWidth, &parentHeight))
        {
            return mParentHeight;
        }
        return parentHeight;
    }
}

EGLint WindowSurfaceGLX::isPostSubBufferSupported() const
{
    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLint WindowSurfaceGLX::getSwapBehavior() const
{
    return EGL_BUFFER_DESTROYED;
}

egl::Error WindowSurfaceGLX::checkForResize()
{
    // TODO(cwallez) set up our own error handler to see if the call failed
    unsigned int newParentWidth, newParentHeight;
    if (!getWindowDimensions(mParent, &newParentWidth, &newParentHeight))
    {
        return egl::EglBadCurrentSurface() << "Failed to retrieve the size of the parent window.";
    }

    if (mParentWidth != newParentWidth || mParentHeight != newParentHeight)
    {
        mParentWidth  = newParentWidth;
        mParentHeight = newParentHeight;

        mGLX.waitGL();
        XResizeWindow(mDisplay, mWindow, mParentWidth, mParentHeight);
        mGLX.waitX();
        XSync(mDisplay, False);
    }

    return egl::NoError();
}

glx::Drawable WindowSurfaceGLX::getDrawable() const
{
    return mGLXWindow;
}

bool WindowSurfaceGLX::getWindowDimensions(Window window,
                                           unsigned int *width,
                                           unsigned int *height) const
{
    Window root;
    int x, y;
    unsigned int border, depth;
    return XGetGeometry(mDisplay, window, &root, &x, &y, width, height, &border, &depth) != 0;
}

egl::Error WindowSurfaceGLX::getSyncValues(EGLuint64KHR *ust, EGLuint64KHR *msc, EGLuint64KHR *sbc)
{
    if (!mGLX.getSyncValuesOML(mGLXWindow, reinterpret_cast<int64_t *>(ust),
                               reinterpret_cast<int64_t *>(msc), reinterpret_cast<int64_t *>(sbc)))
    {
        return egl::EglBadSurface() << "glXGetSyncValuesOML failed.";
    }
    return egl::NoError();
}

egl::Error WindowSurfaceGLX::getMscRate(EGLint *numerator, EGLint *denominator)
{
    if (!mGLX.getMscRateOML(mGLXWindow, reinterpret_cast<int32_t *>(numerator),
                            reinterpret_cast<int32_t *>(denominator)))
    {
        return egl::EglBadSurface() << "glXGetMscRateOML failed.";
    }
    if (mGLXDisplay->getRenderer()->getFeatures().clampMscRate.enabled)
    {
        // Clamp any refresh rate under 2Hz to 30Hz
        if (*numerator < *denominator * 2)
        {
            *numerator   = 30;
            *denominator = 1;
        }
    }
    return egl::NoError();
}

}  // namespace rx
