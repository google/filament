//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderExecutable11.h: Defines a D3D11-specific class to contain shader
// executable implementation details.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_SHADEREXECUTABLE11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_SHADEREXECUTABLE11_H_

#include "libANGLE/renderer/d3d/ShaderExecutableD3D.h"
#include "libANGLE/renderer/d3d/d3d11/ResourceManager11.h"

namespace gl
{
class Context;
}  // namespace gl

namespace rx
{
class Renderer11;
class UniformStorage11;

class ShaderExecutable11 : public ShaderExecutableD3D
{
  public:
    ShaderExecutable11(const void *function, size_t length, d3d11::PixelShader &&executable);
    ShaderExecutable11(const void *function,
                       size_t length,
                       d3d11::VertexShader &&executable,
                       d3d11::GeometryShader &&streamOut);
    ShaderExecutable11(const void *function, size_t length, d3d11::GeometryShader &&executable);
    ShaderExecutable11(const void *function, size_t length, d3d11::ComputeShader &&executable);

    ~ShaderExecutable11() override;

    const d3d11::PixelShader &getPixelShader() const;
    const d3d11::VertexShader &getVertexShader() const;
    const d3d11::GeometryShader &getGeometryShader() const;
    const d3d11::GeometryShader &getStreamOutShader() const;
    const d3d11::ComputeShader &getComputeShader() const;

  private:
    d3d11::PixelShader mPixelExecutable;
    d3d11::VertexShader mVertexExecutable;
    d3d11::GeometryShader mGeometryExecutable;
    d3d11::GeometryShader mStreamOutExecutable;
    d3d11::ComputeShader mComputeExecutable;
};

class UniformStorage11 : public UniformStorageD3D
{
  public:
    UniformStorage11(size_t initialSize);
    ~UniformStorage11() override;

    angle::Result getConstantBuffer(const gl::Context *context,
                                    Renderer11 *renderer,
                                    const d3d11::Buffer **bufferOut);

  private:
    d3d11::Buffer mConstantBuffer;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_SHADEREXECUTABLE11_H_
