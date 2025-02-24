//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Win32Pixmap.cpp: Implementation of OSPixmap for Win32 (Windows)

#include "util/windows/win32/Win32Pixmap.h"

Win32Pixmap::Win32Pixmap() : mBitmap(nullptr) {}

Win32Pixmap::~Win32Pixmap()
{
    if (mBitmap)
    {
        DeleteObject(mBitmap);
    }
}

bool Win32Pixmap::initialize(EGLNativeDisplayType display, size_t width, size_t height, int depth)
{
    BITMAPINFO bitmapInfo;
    memset(&bitmapInfo, 0, sizeof(bitmapInfo));

    if (depth != 24 && depth != 32)
    {
        return false;
    }

    bitmapInfo.bmiHeader.biSize          = sizeof(bitmapInfo);
    bitmapInfo.bmiHeader.biWidth         = static_cast<LONG>(width);
    bitmapInfo.bmiHeader.biHeight        = static_cast<LONG>(height);
    bitmapInfo.bmiHeader.biPlanes        = 1;
    bitmapInfo.bmiHeader.biBitCount      = static_cast<WORD>(depth);
    bitmapInfo.bmiHeader.biCompression   = BI_RGB;
    bitmapInfo.bmiHeader.biSizeImage     = 0;
    bitmapInfo.bmiHeader.biXPelsPerMeter = 1;
    bitmapInfo.bmiHeader.biYPelsPerMeter = 1;
    bitmapInfo.bmiHeader.biClrUsed       = 0;
    bitmapInfo.bmiHeader.biClrImportant  = 0;

    void *bitmapPtr = nullptr;
    mBitmap = CreateDIBSection(display, &bitmapInfo, DIB_RGB_COLORS, &bitmapPtr, nullptr, 0);

    return mBitmap != nullptr;
}

EGLNativePixmapType Win32Pixmap::getNativePixmap() const
{
    return mBitmap;
}

OSPixmap *CreateOSPixmap()
{
    return new Win32Pixmap();
}
