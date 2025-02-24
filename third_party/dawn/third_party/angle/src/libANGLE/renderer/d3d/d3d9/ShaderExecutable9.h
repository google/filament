//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderExecutable9.h: Defines a D3D9-specific class to contain shader
// executable implementation details.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_SHADEREXECUTABLE9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_SHADEREXECUTABLE9_H_

#include "libANGLE/renderer/d3d/ShaderExecutableD3D.h"

namespace rx
{

class ShaderExecutable9 : public ShaderExecutableD3D
{
  public:
    ShaderExecutable9(const void *function, size_t length, IDirect3DPixelShader9 *executable);
    ShaderExecutable9(const void *function, size_t length, IDirect3DVertexShader9 *executable);
    ~ShaderExecutable9() override;

    IDirect3DPixelShader9 *getPixelShader() const;
    IDirect3DVertexShader9 *getVertexShader() const;

  private:
    IDirect3DPixelShader9 *mPixelExecutable;
    IDirect3DVertexShader9 *mVertexExecutable;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D9_SHADEREXECUTABLE9_H_
