//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// glx_utils.cpp: Utility routines specific to the G:X->EGL implementation.

#include "libANGLE/renderer/gl/glx/glx_utils.h"

#include "common/angleutils.h"

namespace rx
{

namespace x11
{

std::string XErrorToString(Display *display, int status)
{
    // Write nulls to the buffer so that if XGetErrorText fails, converting to an std::string will
    // be an empty string.
    char buffer[256] = {0};
    XGetErrorText(display, status, buffer, ArraySize(buffer));
    return std::string(buffer);
}

}  // namespace x11

}  // namespace rx
