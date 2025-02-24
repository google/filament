//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// X11Pixmap.h: Definition of the implementation of OSPixmap for X11

#ifndef UTIL_X11_PIXMAP_H_
#define UTIL_X11_PIXMAP_H_

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "util/OSPixmap.h"

class X11Pixmap : public OSPixmap
{
  public:
    X11Pixmap();
    ~X11Pixmap() override;

    bool initialize(EGLNativeDisplayType display,
                    size_t width,
                    size_t height,
                    int nativeVisual) override;

    EGLNativePixmapType getNativePixmap() const override;

  private:
    Pixmap mPixmap;
    Display *mDisplay;
};

#endif  // UTIL_X11_PIXMAP_H_
