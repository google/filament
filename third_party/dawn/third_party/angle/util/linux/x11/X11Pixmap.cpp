//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// X11Pixmap.cpp: Implementation of OSPixmap for X11

#include "util/linux/x11/X11Pixmap.h"

X11Pixmap::X11Pixmap() : mPixmap(0), mDisplay(nullptr) {}

X11Pixmap::~X11Pixmap()
{
    if (mPixmap)
    {
        XFreePixmap(mDisplay, mPixmap);
    }
}

bool X11Pixmap::initialize(EGLNativeDisplayType display,
                           size_t width,
                           size_t height,
                           int nativeVisual)
{
    mDisplay = reinterpret_cast<Display *>(display);

    int screen  = DefaultScreen(mDisplay);
    Window root = RootWindow(mDisplay, screen);
    int depth   = 0;

    XVisualInfo visualTemplate;
    visualTemplate.visualid = nativeVisual;

    int numVisuals    = 0;
    XVisualInfo *info = XGetVisualInfo(mDisplay, VisualIDMask, &visualTemplate, &numVisuals);
    if (numVisuals == 1)
    {
        depth = info->depth;
    }
    XFree(info);

    mPixmap = XCreatePixmap(mDisplay, root, static_cast<unsigned int>(width),
                            static_cast<unsigned int>(height), depth);

    return mPixmap != 0;
}

EGLNativePixmapType X11Pixmap::getNativePixmap() const
{
    return mPixmap;
}

OSPixmap *CreateOSPixmap()
{
    return new X11Pixmap();
}
