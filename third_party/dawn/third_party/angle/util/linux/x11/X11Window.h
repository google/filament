//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// X11Window.h: Definition of the implementation of OSWindow for X11

#ifndef UTIL_X11_WINDOW_H
#define UTIL_X11_WINDOW_H

#include <string>

#include "util/OSWindow.h"
#include "util/util_export.h"

ANGLE_UTIL_EXPORT OSWindow *CreateX11Window();
ANGLE_UTIL_EXPORT OSWindow *CreateX11WindowWithVisualId(int visualId);

bool IsX11WindowAvailable();

#endif  // UTIL_X11_WINDOW_H
