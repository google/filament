//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// LinuxWindow.cpp: Implementation of OSWindow::New for Linux

#include "util/OSWindow.h"

#if defined(ANGLE_USE_WAYLAND)
#    include "wayland/WaylandWindow.h"
#endif

#if defined(ANGLE_USE_X11)
#    include "x11/X11Window.h"
#endif

// static
#if defined(ANGLE_USE_X11) || defined(ANGLE_USE_WAYLAND)
OSWindow *OSWindow::New()
{
#    if defined(ANGLE_USE_X11)
    // Prefer X11
    if (IsX11WindowAvailable())
    {
        return CreateX11Window();
    }
#    endif

#    if defined(ANGLE_USE_WAYLAND)
    if (IsWaylandWindowAvailable())
    {
        return new WaylandWindow();
    }
#    endif

    return nullptr;
}
#endif
