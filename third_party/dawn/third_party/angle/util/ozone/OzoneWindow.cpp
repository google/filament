//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// OzoneWindow.cpp: Implementation of OSWindow for Ozone

#include "util/ozone/OzoneWindow.h"

#include "common/debug.h"

int OzoneWindow::sLastDepth = 0;

OzoneWindow::OzoneWindow() {}

OzoneWindow::~OzoneWindow() {}

bool OzoneWindow::initializeImpl(const std::string &name, int width, int height)
{
    mNative.x = mX = 0;
    mNative.y = mY = 0;
    mNative.width = mWidth = width;
    mNative.height = mHeight = height;
    mNative.borderWidth      = 5;
    mNative.borderHeight     = 5;
    mNative.visible          = 0;
    mNative.depth            = sLastDepth++;
    return true;
}

void OzoneWindow::disableErrorMessageDialog() {}

void OzoneWindow::destroy() {}

void OzoneWindow::resetNativeWindow() {}

EGLNativeWindowType OzoneWindow::getNativeWindow() const
{
    return reinterpret_cast<EGLNativeWindowType>(&mNative);
}

EGLNativeDisplayType OzoneWindow::getNativeDisplay() const
{
    return EGL_DEFAULT_DISPLAY;
}

void OzoneWindow::messageLoop() {}

void OzoneWindow::setMousePosition(int x, int y) {}

bool OzoneWindow::setOrientation(int width, int height)
{
    UNIMPLEMENTED();
    return false;
}

bool OzoneWindow::setPosition(int x, int y)
{
    mNative.x = mX = x;
    mNative.y = mY = y;
    return true;
}

bool OzoneWindow::resize(int width, int height)
{
    mNative.width = mWidth = width;
    mNative.height = mHeight = height;
    return true;
}

void OzoneWindow::setVisible(bool isVisible)
{
    mNative.visible = isVisible;
}

void OzoneWindow::signalTestEvent() {}

// static
OSWindow *OSWindow::New()
{
    return new OzoneWindow();
}
