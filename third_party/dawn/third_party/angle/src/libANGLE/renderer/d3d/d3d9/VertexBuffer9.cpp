//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexBuffer9.cpp: Defines the D3D9 VertexBuffer implementation.

#include "libANGLE/renderer/d3d/d3d9/VertexBuffer9.h"

#include "libANGLE/Buffer.h"
#include "libANGLE/Context.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/renderer/d3d/BufferD3D.h"
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"
#include "libANGLE/renderer/d3d/d3d9/formatutils9.h"
#include "libANGLE/renderer/d3d/d3d9/vertexconversion.h"

namespace rx
{

VertexBuffer9::VertexBuffer9(Renderer9 *renderer) : mRenderer(renderer)
{
    mVertexBuffer = nullptr;
    mBufferSize   = 0;
    mDynamicUsage = false;
}

VertexBuffer9::~VertexBuffer9()
{
    SafeRelease(mVertexBuffer);
}

angle::Result VertexBuffer9::initialize(const gl::Context *context,
                                        unsigned int size,
                                        bool dynamicUsage)
{
    SafeRelease(mVertexBuffer);

    updateSerial();

    if (size > 0)
    {
        DWORD flags = D3DUSAGE_WRITEONLY;
        if (dynamicUsage)
        {
            flags |= D3DUSAGE_DYNAMIC;
        }

        HRESULT result = mRenderer->createVertexBuffer(size, flags, &mVertexBuffer);
        ANGLE_TRY_HR(GetImplAs<Context9>(context), result,
                     "Failed to allocate internal vertex buffer");
    }

    mBufferSize   = size;
    mDynamicUsage = dynamicUsage;
    return angle::Result::Continue;
}

angle::Result VertexBuffer9::storeVertexAttributes(const gl::Context *context,
                                                   const gl::VertexAttribute &attrib,
                                                   const gl::VertexBinding &binding,
                                                   gl::VertexAttribType currentValueType,
                                                   GLint start,
                                                   size_t count,
                                                   GLsizei instances,
                                                   unsigned int offset,
                                                   const uint8_t *sourceData)
{
    ASSERT(mVertexBuffer);

    size_t inputStride = gl::ComputeVertexAttributeStride(attrib, binding);
    size_t elementSize = gl::ComputeVertexAttributeTypeSize(attrib);

    DWORD lockFlags = mDynamicUsage ? D3DLOCK_NOOVERWRITE : 0;

    uint8_t *mapPtr = nullptr;

    unsigned int mapSize = 0;
    ANGLE_TRY(
        mRenderer->getVertexSpaceRequired(context, attrib, binding, count, instances, 0, &mapSize));

    HRESULT result =
        mVertexBuffer->Lock(offset, mapSize, reinterpret_cast<void **>(&mapPtr), lockFlags);
    ANGLE_TRY_HR(GetImplAs<Context9>(context), result, "Failed to lock internal vertex buffer");

    const uint8_t *input = sourceData;

    if (instances == 0 || binding.getDivisor() == 0)
    {
        input += inputStride * start;
    }

    angle::FormatID vertexFormatID = gl::GetVertexFormatID(attrib, currentValueType);
    const d3d9::VertexFormat &d3dVertexInfo =
        d3d9::GetVertexFormatInfo(mRenderer->getCapsDeclTypes(), vertexFormatID);
    bool needsConversion = (d3dVertexInfo.conversionType & VERTEX_CONVERT_CPU) > 0;

    if (!needsConversion && inputStride == elementSize)
    {
        size_t copySize = count * inputStride;
        memcpy(mapPtr, input, copySize);
    }
    else
    {
        d3dVertexInfo.copyFunction(input, inputStride, count, mapPtr);
    }

    mVertexBuffer->Unlock();

    return angle::Result::Continue;
}

unsigned int VertexBuffer9::getBufferSize() const
{
    return mBufferSize;
}

angle::Result VertexBuffer9::setBufferSize(const gl::Context *context, unsigned int size)
{
    if (size > mBufferSize)
    {
        return initialize(context, size, mDynamicUsage);
    }
    else
    {
        return angle::Result::Continue;
    }
}

angle::Result VertexBuffer9::discard(const gl::Context *context)
{
    ASSERT(mVertexBuffer);

    void *mock;
    HRESULT result;

    Context9 *context9 = GetImplAs<Context9>(context);

    result = mVertexBuffer->Lock(0, 1, &mock, D3DLOCK_DISCARD);
    ANGLE_TRY_HR(context9, result, "Failed to lock internal vertex buffer for discarding");

    result = mVertexBuffer->Unlock();
    ANGLE_TRY_HR(context9, result, "Failed to unlock internal vertex buffer for discarding");

    return angle::Result::Continue;
}

IDirect3DVertexBuffer9 *VertexBuffer9::getBuffer() const
{
    return mVertexBuffer;
}
}  // namespace rx
