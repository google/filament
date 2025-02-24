//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// NativeWindowD3D.cpp: Defines NativeWindowD3D, a class for managing and performing operations on
// an EGLNativeWindowType for the D3D renderers.

#include "libANGLE/renderer/d3d/NativeWindowD3D.h"

namespace rx
{

NativeWindowD3D::NativeWindowD3D(EGLNativeWindowType window) : mWindow(window) {}

NativeWindowD3D::~NativeWindowD3D() {}

}  // namespace rx
