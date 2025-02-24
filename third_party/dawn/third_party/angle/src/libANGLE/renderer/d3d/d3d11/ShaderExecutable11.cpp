//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderExecutable11.cpp: Implements a D3D11-specific class to contain shader
// executable implementation details.

#include "libANGLE/renderer/d3d/d3d11/ShaderExecutable11.h"

#include "libANGLE/Context.h"
#include "libANGLE/renderer/d3d/d3d11/Context11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"

namespace rx
{

ShaderExecutable11::ShaderExecutable11(const void *function,
                                       size_t length,
                                       d3d11::PixelShader &&executable)
    : ShaderExecutableD3D(function, length),
      mPixelExecutable(std::move(executable)),
      mVertexExecutable(),
      mGeometryExecutable(),
      mStreamOutExecutable(),
      mComputeExecutable()
{}

ShaderExecutable11::ShaderExecutable11(const void *function,
                                       size_t length,
                                       d3d11::VertexShader &&executable,
                                       d3d11::GeometryShader &&streamOut)
    : ShaderExecutableD3D(function, length),
      mPixelExecutable(),
      mVertexExecutable(std::move(executable)),
      mGeometryExecutable(),
      mStreamOutExecutable(std::move(streamOut)),
      mComputeExecutable()
{}

ShaderExecutable11::ShaderExecutable11(const void *function,
                                       size_t length,
                                       d3d11::GeometryShader &&executable)
    : ShaderExecutableD3D(function, length),
      mPixelExecutable(),
      mVertexExecutable(),
      mGeometryExecutable(std::move(executable)),
      mStreamOutExecutable(),
      mComputeExecutable()
{}

ShaderExecutable11::ShaderExecutable11(const void *function,
                                       size_t length,
                                       d3d11::ComputeShader &&executable)
    : ShaderExecutableD3D(function, length),
      mPixelExecutable(),
      mVertexExecutable(),
      mGeometryExecutable(),
      mStreamOutExecutable(),
      mComputeExecutable(std::move(executable))
{}

ShaderExecutable11::~ShaderExecutable11() {}

const d3d11::VertexShader &ShaderExecutable11::getVertexShader() const
{
    return mVertexExecutable;
}

const d3d11::PixelShader &ShaderExecutable11::getPixelShader() const
{
    return mPixelExecutable;
}

const d3d11::GeometryShader &ShaderExecutable11::getGeometryShader() const
{
    return mGeometryExecutable;
}

const d3d11::GeometryShader &ShaderExecutable11::getStreamOutShader() const
{
    return mStreamOutExecutable;
}

const d3d11::ComputeShader &ShaderExecutable11::getComputeShader() const
{
    return mComputeExecutable;
}

UniformStorage11::UniformStorage11(size_t initialSize)
    : UniformStorageD3D(initialSize), mConstantBuffer()
{}

UniformStorage11::~UniformStorage11() {}

angle::Result UniformStorage11::getConstantBuffer(const gl::Context *context,
                                                  Renderer11 *renderer,
                                                  const d3d11::Buffer **bufferOut)
{
    if (size() > 0 && !mConstantBuffer.valid())
    {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth         = static_cast<unsigned int>(size());
        desc.Usage             = D3D11_USAGE_DEFAULT;
        desc.BindFlags         = D3D11_BIND_CONSTANT_BUFFER;

        ANGLE_TRY(
            renderer->allocateResource(GetImplAs<Context11>(context), desc, &mConstantBuffer));
    }

    *bufferOut = &mConstantBuffer;
    return angle::Result::Continue;
}

}  // namespace rx
