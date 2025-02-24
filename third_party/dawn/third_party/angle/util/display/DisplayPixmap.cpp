//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayPixmap.cpp: Implementation of OSPixmap for Linux Display

#include "util/OSPixmap.h"

#if defined(ANGLE_USE_VULKAN_DISPLAY) && defined(EGL_NO_X11)
OSPixmap *CreateOSPixmap()
{
    return nullptr;
}
#endif
