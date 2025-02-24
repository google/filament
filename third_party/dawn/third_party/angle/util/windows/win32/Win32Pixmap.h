//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Win32Pixmap.h: Definition of the implementation of OSPixmap for Win32 (Windows)

#ifndef UTIL_WIN32_PIXMAP_H_
#define UTIL_WIN32_PIXMAP_H_

#include <windows.h>

#include "util/OSPixmap.h"

class Win32Pixmap : public OSPixmap
{
  public:
    Win32Pixmap();
    ~Win32Pixmap() override;

    bool initialize(EGLNativeDisplayType display, size_t width, size_t height, int depth) override;

    EGLNativePixmapType getNativePixmap() const override;

  private:
    HBITMAP mBitmap;
};

#endif  // UTIL_WIN32_PIXMAP_H_
