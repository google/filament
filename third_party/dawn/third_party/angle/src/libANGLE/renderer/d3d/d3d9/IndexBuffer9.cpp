//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Indexffer9.cpp: Defines the D3D9 IndexBuffer implementation.

#include "libANGLE/renderer/d3d/d3d9/IndexBuffer9.h"

#include "libANGLE/Context.h"
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"

namespace rx
{

IndexBuffer9::IndexBuffer9(Renderer9 *const renderer) : mRenderer(renderer)
{
    mIndexBuffer = nullptr;
    mBufferSize  = 0;
    mIndexType   = gl::DrawElementsType::InvalidEnum;
    mDynamic     = false;
}

IndexBuffer9::~IndexBuffer9()
{
    SafeRelease(mIndexBuffer);
}

angle::Result IndexBuffer9::initialize(const gl::Context *context,
                                       unsigned int bufferSize,
                                       gl::DrawElementsType indexType,
                                       bool dynamic)
{
    SafeRelease(mIndexBuffer);

    updateSerial();

    if (bufferSize > 0)
    {
        D3DFORMAT format = D3DFMT_UNKNOWN;
        if (indexType == gl::DrawElementsType::UnsignedShort ||
            indexType == gl::DrawElementsType::UnsignedByte)
        {
            format = D3DFMT_INDEX16;
        }
        else if (indexType == gl::DrawElementsType::UnsignedInt)
        {
            ASSERT(mRenderer->getNativeExtensions().elementIndexUintOES);
            format = D3DFMT_INDEX32;
        }
        else
            UNREACHABLE();

        DWORD usageFlags = D3DUSAGE_WRITEONLY;
        if (dynamic)
        {
            usageFlags |= D3DUSAGE_DYNAMIC;
        }

        HRESULT result =
            mRenderer->createIndexBuffer(bufferSize, usageFlags, format, &mIndexBuffer);
        ANGLE_TRY_HR(GetImplAs<Context9>(context), result,
                     "Failed to allocate internal index buffer");
    }

    mBufferSize = bufferSize;
    mIndexType  = indexType;
    mDynamic    = dynamic;

    return angle::Result::Continue;
}

angle::Result IndexBuffer9::mapBuffer(const gl::Context *context,
                                      unsigned int offset,
                                      unsigned int size,
                                      void **outMappedMemory)
{
    ASSERT(mIndexBuffer);

    DWORD lockFlags = mDynamic ? D3DLOCK_NOOVERWRITE : 0;

    void *mapPtr   = nullptr;
    HRESULT result = mIndexBuffer->Lock(offset, size, &mapPtr, lockFlags);
    ANGLE_TRY_HR(GetImplAs<Context9>(context), result, "Failed to lock internal index buffer");

    *outMappedMemory = mapPtr;
    return angle::Result::Continue;
}

angle::Result IndexBuffer9::unmapBuffer(const gl::Context *context)
{
    ASSERT(mIndexBuffer);
    HRESULT result = mIndexBuffer->Unlock();
    ANGLE_TRY_HR(GetImplAs<Context9>(context), result, "Failed to unlock internal index buffer");

    return angle::Result::Continue;
}

gl::DrawElementsType IndexBuffer9::getIndexType() const
{
    return mIndexType;
}

unsigned int IndexBuffer9::getBufferSize() const
{
    return mBufferSize;
}

angle::Result IndexBuffer9::setSize(const gl::Context *context,
                                    unsigned int bufferSize,
                                    gl::DrawElementsType indexType)
{
    if (bufferSize > mBufferSize || indexType != mIndexType)
    {
        return initialize(context, bufferSize, indexType, mDynamic);
    }

    return angle::Result::Continue;
}

angle::Result IndexBuffer9::discard(const gl::Context *context)
{
    ASSERT(mIndexBuffer);

    void *mock;
    HRESULT result;

    Context9 *context9 = GetImplAs<Context9>(context);

    result = mIndexBuffer->Lock(0, 1, &mock, D3DLOCK_DISCARD);
    ANGLE_TRY_HR(context9, result, "Failed to lock internal index buffer");

    result = mIndexBuffer->Unlock();
    ANGLE_TRY_HR(context9, result, "Failed to unlock internal index buffer");

    return angle::Result::Continue;
}

D3DFORMAT IndexBuffer9::getIndexFormat() const
{
    switch (mIndexType)
    {
        case gl::DrawElementsType::UnsignedByte:
            return D3DFMT_INDEX16;
        case gl::DrawElementsType::UnsignedShort:
            return D3DFMT_INDEX16;
        case gl::DrawElementsType::UnsignedInt:
            return D3DFMT_INDEX32;
        default:
            UNREACHABLE();
            return D3DFMT_UNKNOWN;
    }
}

IDirect3DIndexBuffer9 *IndexBuffer9::getBuffer() const
{
    return mIndexBuffer;
}

}  // namespace rx
