//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderExecutable9.cpp: Implements a D3D9-specific class to contain shader
// executable implementation details.

#include "libANGLE/renderer/d3d/d3d9/ShaderExecutable9.h"

#include "common/debug.h"

namespace rx
{

ShaderExecutable9::ShaderExecutable9(const void *function,
                                     size_t length,
                                     IDirect3DPixelShader9 *executable)
    : ShaderExecutableD3D(function, length)
{
    mPixelExecutable  = executable;
    mVertexExecutable = nullptr;
}

ShaderExecutable9::ShaderExecutable9(const void *function,
                                     size_t length,
                                     IDirect3DVertexShader9 *executable)
    : ShaderExecutableD3D(function, length)
{
    mVertexExecutable = executable;
    mPixelExecutable  = nullptr;
}

ShaderExecutable9::~ShaderExecutable9()
{
    SafeRelease(mVertexExecutable);
    SafeRelease(mPixelExecutable);
}

IDirect3DVertexShader9 *ShaderExecutable9::getVertexShader() const
{
    return mVertexExecutable;
}

IDirect3DPixelShader9 *ShaderExecutable9::getPixelShader() const
{
    return mPixelExecutable;
}

}  // namespace rx
