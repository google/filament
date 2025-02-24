//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// NativeWindow9.cpp: Defines NativeWindow9, a class for managing and
// performing operations on an EGLNativeWindowType for the D3D9 renderer.

#include "libANGLE/renderer/d3d/d3d9/NativeWindow9.h"

namespace rx
{
NativeWindow9::NativeWindow9(EGLNativeWindowType window) : NativeWindowD3D(window) {}

bool NativeWindow9::initialize()
{
    return true;
}

bool NativeWindow9::getClientRect(LPRECT rect) const
{
    return GetClientRect(getNativeWindow(), rect) == TRUE;
}

bool NativeWindow9::isIconic() const
{
    return IsIconic(getNativeWindow()) == TRUE;
}

// static
bool NativeWindow9::IsValidNativeWindow(EGLNativeWindowType window)
{
    return IsWindow(window) == TRUE;
}

}  // namespace rx
