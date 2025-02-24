//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SwapChainD3D.cpp: Defines a back-end specific class that hides the details of the
// implementation-specific swapchain.

#include "libANGLE/renderer/d3d/SwapChainD3D.h"

namespace rx
{

SwapChainD3D::SwapChainD3D(HANDLE shareHandle,
                           IUnknown *d3dTexture,
                           GLenum backBufferFormat,
                           GLenum depthBufferFormat)
    : mOffscreenRenderTargetFormat(backBufferFormat),
      mDepthBufferFormat(depthBufferFormat),
      mShareHandle(shareHandle),
      mD3DTexture(d3dTexture)
{
    if (mD3DTexture)
    {
        mD3DTexture->AddRef();
    }
}

SwapChainD3D::~SwapChainD3D()
{
    SafeRelease(mD3DTexture);
}
}  // namespace rx
