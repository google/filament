//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexBuffer11.cpp: Defines the D3D11 IndexBuffer implementation.

#include "libANGLE/renderer/d3d/d3d11/IndexBuffer11.h"

#include "libANGLE/Context.h"
#include "libANGLE/renderer/d3d/d3d11/Context11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

namespace rx
{

IndexBuffer11::IndexBuffer11(Renderer11 *const renderer)
    : mRenderer(renderer),
      mBuffer(),
      mBufferSize(0),
      mIndexType(gl::DrawElementsType::InvalidEnum),
      mDynamicUsage(false)
{}

IndexBuffer11::~IndexBuffer11() {}

angle::Result IndexBuffer11::initialize(const gl::Context *context,
                                        unsigned int bufferSize,
                                        gl::DrawElementsType indexType,
                                        bool dynamic)
{
    mBuffer.reset();

    updateSerial();

    if (bufferSize > 0)
    {
        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.ByteWidth           = bufferSize;
        bufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags           = 0;
        bufferDesc.StructureByteStride = 0;

        ANGLE_TRY(mRenderer->allocateResource(GetImplAs<Context11>(context), bufferDesc, &mBuffer));

        if (dynamic)
        {
            mBuffer.setInternalName("IndexBuffer11(dynamic)");
        }
        else
        {
            mBuffer.setInternalName("IndexBuffer11(static)");
        }
    }

    mBufferSize   = bufferSize;
    mIndexType    = indexType;
    mDynamicUsage = dynamic;

    return angle::Result::Continue;
}

angle::Result IndexBuffer11::mapBuffer(const gl::Context *context,
                                       unsigned int offset,
                                       unsigned int size,
                                       void **outMappedMemory)
{
    Context11 *context11 = GetImplAs<Context11>(context);
    ANGLE_CHECK_HR(context11, mBuffer.valid(), "Internal index buffer is not initialized.",
                   E_OUTOFMEMORY);

    // Check for integer overflows and out-out-bounds map requests
    bool outOfBounds = (offset + size < offset || offset + size > mBufferSize);
    ANGLE_CHECK_HR(context11, !outOfBounds, "Index buffer map range is not inside the buffer.",
                   E_OUTOFMEMORY);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ANGLE_TRY(mRenderer->mapResource(context, mBuffer.get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0,
                                     &mappedResource));

    *outMappedMemory = static_cast<char *>(mappedResource.pData) + offset;
    return angle::Result::Continue;
}

angle::Result IndexBuffer11::unmapBuffer(const gl::Context *context)
{
    Context11 *context11 = GetImplAs<Context11>(context);
    ANGLE_CHECK_HR(context11, mBuffer.valid(), "Internal index buffer is not initialized.",
                   E_OUTOFMEMORY);

    ID3D11DeviceContext *dxContext = mRenderer->getDeviceContext();
    dxContext->Unmap(mBuffer.get(), 0);
    return angle::Result::Continue;
}

gl::DrawElementsType IndexBuffer11::getIndexType() const
{
    return mIndexType;
}

unsigned int IndexBuffer11::getBufferSize() const
{
    return mBufferSize;
}

angle::Result IndexBuffer11::setSize(const gl::Context *context,
                                     unsigned int bufferSize,
                                     gl::DrawElementsType indexType)
{
    if (bufferSize > mBufferSize || indexType != mIndexType)
    {
        return initialize(context, bufferSize, indexType, mDynamicUsage);
    }

    return angle::Result::Continue;
}

angle::Result IndexBuffer11::discard(const gl::Context *context)
{
    Context11 *context11 = GetImplAs<Context11>(context);
    ANGLE_CHECK_HR(context11, mBuffer.valid(), "Internal index buffer is not initialized.",
                   E_OUTOFMEMORY);

    ID3D11DeviceContext *dxContext = mRenderer->getDeviceContext();

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ANGLE_TRY(mRenderer->mapResource(context, mBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
                                     &mappedResource));

    dxContext->Unmap(mBuffer.get(), 0);

    return angle::Result::Continue;
}

DXGI_FORMAT IndexBuffer11::getIndexFormat() const
{
    switch (mIndexType)
    {
        case gl::DrawElementsType::UnsignedByte:
            return DXGI_FORMAT_R16_UINT;
        case gl::DrawElementsType::UnsignedShort:
            return DXGI_FORMAT_R16_UINT;
        case gl::DrawElementsType::UnsignedInt:
            return DXGI_FORMAT_R32_UINT;
        default:
            UNREACHABLE();
            return DXGI_FORMAT_UNKNOWN;
    }
}

const d3d11::Buffer &IndexBuffer11::getBuffer() const
{
    return mBuffer;
}

}  // namespace rx
