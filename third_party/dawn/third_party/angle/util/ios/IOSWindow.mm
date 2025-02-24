//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IOSWindow.mm: Implementation of OSWindow for iOS

#include "util/ios/IOSWindow.h"

#include <set>

#include "anglebase/no_destructor.h"
#include "common/debug.h"

#import <UIKit/UIKit.h>

static CALayer *rootLayer()
{
    return [[[[[UIApplication sharedApplication] delegate] window] rootViewController] view].layer;
}

bool IOSWindow::initializeImpl(const std::string &name, int width, int height)
{
    resize(width, height);
    return true;
}

EGLNativeWindowType IOSWindow::getNativeWindow() const
{
    return rootLayer();
}

bool IOSWindow::setOrientation(int width, int height)
{
    UNIMPLEMENTED();
    return false;
}

bool IOSWindow::resize(int width, int height)
{
    rootLayer().frame = CGRectMake(0, 0, width, height);
    return true;
}

// static
OSWindow *OSWindow::New()
{
    return new IOSWindow;
}
