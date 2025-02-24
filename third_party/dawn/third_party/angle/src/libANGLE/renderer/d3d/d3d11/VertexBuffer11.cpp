//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexBuffer11.cpp: Defines the D3D11 VertexBuffer implementation.

#include "libANGLE/renderer/d3d/d3d11/VertexBuffer11.h"

#include "libANGLE/Buffer.h"
#include "libANGLE/Context.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/d3d11/Buffer11.h"
#include "libANGLE/renderer/d3d/d3d11/Context11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

namespace rx
{

VertexBuffer11::VertexBuffer11(Renderer11 *const renderer)
    : mRenderer(renderer),
      mBuffer(),
      mBufferSize(0),
      mDynamicUsage(false),
      mMappedResourceData(nullptr)
{}

VertexBuffer11::~VertexBuffer11()
{
    ASSERT(mMappedResourceData == nullptr);
}

angle::Result VertexBuffer11::initialize(const gl::Context *context,
                                         unsigned int size,
                                         bool dynamicUsage)
{
    mBuffer.reset();
    updateSerial();

    if (size > 0)
    {
        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.ByteWidth           = size;
        bufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags           = 0;
        bufferDesc.StructureByteStride = 0;

        ANGLE_TRY(mRenderer->allocateResource(GetImplAs<Context11>(context), bufferDesc, &mBuffer));

        if (dynamicUsage)
        {
            mBuffer.setInternalName("VertexBuffer11(dynamic)");
        }
        else
        {
            mBuffer.setInternalName("VertexBuffer11(static)");
        }
    }

    mBufferSize   = size;
    mDynamicUsage = dynamicUsage;

    return angle::Result::Continue;
}

angle::Result VertexBuffer11::mapResource(const gl::Context *context)
{
    if (mMappedResourceData == nullptr)
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource;

        ANGLE_TRY(mRenderer->mapResource(context, mBuffer.get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0,
                                         &mappedResource));

        mMappedResourceData = static_cast<uint8_t *>(mappedResource.pData);
    }

    return angle::Result::Continue;
}

void VertexBuffer11::hintUnmapResource()
{
    if (mMappedResourceData != nullptr)
    {
        ID3D11DeviceContext *dxContext = mRenderer->getDeviceContext();
        dxContext->Unmap(mBuffer.get(), 0);

        mMappedResourceData = nullptr;
    }
}

angle::Result VertexBuffer11::storeVertexAttributes(const gl::Context *context,
                                                    const gl::VertexAttribute &attrib,
                                                    const gl::VertexBinding &binding,
                                                    gl::VertexAttribType currentValueType,
                                                    GLint start,
                                                    size_t count,
                                                    GLsizei instances,
                                                    unsigned int offset,
                                                    const uint8_t *sourceData)
{
    ASSERT(mBuffer.valid());

    int inputStride = static_cast<int>(ComputeVertexAttributeStride(attrib, binding));

    // This will map the resource if it isn't already mapped.
    ANGLE_TRY(mapResource(context));

    uint8_t *output = mMappedResourceData + offset;

    const uint8_t *input = sourceData;

    if (instances == 0 || binding.getDivisor() == 0)
    {
        input += inputStride * start;
    }

    angle::FormatID vertexFormatID       = gl::GetVertexFormatID(attrib, currentValueType);
    const D3D_FEATURE_LEVEL featureLevel = mRenderer->getRenderer11DeviceCaps().featureLevel;
    const d3d11::VertexFormat &vertexFormatInfo =
        d3d11::GetVertexFormatInfo(vertexFormatID, featureLevel);
    ASSERT(vertexFormatInfo.copyFunction != nullptr);
    vertexFormatInfo.copyFunction(input, inputStride, count, output);

    return angle::Result::Continue;
}

unsigned int VertexBuffer11::getBufferSize() const
{
    return mBufferSize;
}

angle::Result VertexBuffer11::setBufferSize(const gl::Context *context, unsigned int size)
{
    if (size > mBufferSize)
    {
        return initialize(context, size, mDynamicUsage);
    }

    return angle::Result::Continue;
}

angle::Result VertexBuffer11::discard(const gl::Context *context)
{
    ASSERT(mBuffer.valid());

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ANGLE_TRY(mRenderer->mapResource(context, mBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
                                     &mappedResource));

    mRenderer->getDeviceContext()->Unmap(mBuffer.get(), 0);

    return angle::Result::Continue;
}

const d3d11::Buffer &VertexBuffer11::getBuffer() const
{
    return mBuffer;
}

}  // namespace rx
