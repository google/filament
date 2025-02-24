//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BufferD3D.cpp Defines common functionality between the Buffer9 and Buffer11 classes.

#include "libANGLE/renderer/d3d/BufferD3D.h"

#include "common/mathutil.h"
#include "common/utilities.h"
#include "libANGLE/renderer/d3d/IndexBuffer.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/VertexBuffer.h"

namespace rx
{

unsigned int BufferD3D::mNextSerial = 1;

BufferD3D::BufferD3D(const gl::BufferState &state, BufferFactoryD3D *factory)
    : BufferImpl(state),
      mFactory(factory),
      mStaticIndexBuffer(nullptr),
      mStaticBufferCacheTotalSize(0),
      mStaticVertexBufferOutOfDate(false),
      mUnmodifiedDataUse(0),
      mUsage(D3DBufferUsage::STATIC)
{
    updateSerial();
}

BufferD3D::~BufferD3D()
{
    SafeDelete(mStaticIndexBuffer);
}

void BufferD3D::emptyStaticBufferCache()
{
    mStaticVertexBuffers.clear();
    mStaticBufferCacheTotalSize = 0;
}

void BufferD3D::updateSerial()
{
    mSerial = mNextSerial++;
}

void BufferD3D::updateD3DBufferUsage(const gl::Context *context, gl::BufferUsage usage)
{
    switch (usage)
    {
        case gl::BufferUsage::StaticCopy:
        case gl::BufferUsage::StaticDraw:
        case gl::BufferUsage::StaticRead:
        case gl::BufferUsage::DynamicCopy:
        case gl::BufferUsage::DynamicRead:
        case gl::BufferUsage::StreamCopy:
        case gl::BufferUsage::StreamRead:
            mUsage = D3DBufferUsage::STATIC;
            initializeStaticData(context);
            break;

        case gl::BufferUsage::DynamicDraw:
        case gl::BufferUsage::StreamDraw:
            mUsage = D3DBufferUsage::DYNAMIC;
            break;
        default:
            UNREACHABLE();
    }
}

void BufferD3D::initializeStaticData(const gl::Context *context)
{
    if (mStaticVertexBuffers.empty())
    {
        StaticVertexBufferInterface *newStaticBuffer = new StaticVertexBufferInterface(mFactory);
        mStaticVertexBuffers.push_back(
            std::unique_ptr<StaticVertexBufferInterface>(newStaticBuffer));
    }
    if (!mStaticIndexBuffer)
    {
        mStaticIndexBuffer = new StaticIndexBufferInterface(mFactory);
    }
}

StaticIndexBufferInterface *BufferD3D::getStaticIndexBuffer()
{
    return mStaticIndexBuffer;
}

StaticVertexBufferInterface *BufferD3D::getStaticVertexBuffer(const gl::VertexAttribute &attribute,
                                                              const gl::VertexBinding &binding)
{
    if (mStaticVertexBuffers.empty())
    {
        // Early out if there aren't any static buffers at all
        return nullptr;
    }

    // Early out, the attribute can be added to mStaticVertexBuffer.
    if (mStaticVertexBuffers.size() == 1 && mStaticVertexBuffers[0]->empty())
    {
        return mStaticVertexBuffers[0].get();
    }

    // Cache size limiting: track the total allocated buffer sizes.
    size_t currentTotalSize = 0;

    // At this point, see if any of the existing static buffers contains the attribute data
    // If there is a cached static buffer that already contains the attribute, then return it
    for (const auto &staticBuffer : mStaticVertexBuffers)
    {
        if (staticBuffer->matchesAttribute(attribute, binding))
        {
            return staticBuffer.get();
        }

        currentTotalSize += staticBuffer->getBufferSize();
    }

    // Cache size limiting: Clean-up threshold is four times the base buffer size, with a minimum.
    ASSERT(getSize() < std::numeric_limits<size_t>::max() / 4u);
    size_t sizeThreshold = std::max(getSize() * 4u, static_cast<size_t>(0x1000u));

    // If we're past the threshold, clear the buffer cache. Note that this will release buffers
    // that are currenly bound, and in an edge case can even translate the same attribute twice
    // in the same draw call. It will not delete currently bound buffers, however, because they
    // are ref counted.
    if (currentTotalSize > sizeThreshold)
    {
        emptyStaticBufferCache();
    }

    // At this point, we must create a new static buffer for the attribute data.
    StaticVertexBufferInterface *newStaticBuffer = new StaticVertexBufferInterface(mFactory);
    newStaticBuffer->setAttribute(attribute, binding);
    mStaticVertexBuffers.push_back(std::unique_ptr<StaticVertexBufferInterface>(newStaticBuffer));
    return newStaticBuffer;
}

void BufferD3D::invalidateStaticData(const gl::Context *context)
{
    emptyStaticBufferCache();

    if (mStaticIndexBuffer && mStaticIndexBuffer->getBufferSize() != 0)
    {
        SafeDelete(mStaticIndexBuffer);
    }

    // If the buffer was created with a static usage then we recreate the static
    // buffers so that they are populated the next time we use this buffer.
    if (mUsage == D3DBufferUsage::STATIC)
    {
        initializeStaticData(context);
    }

    mUnmodifiedDataUse = 0;
}

// Creates static buffers if sufficient used data has been left unmodified
void BufferD3D::promoteStaticUsage(const gl::Context *context, size_t dataSize)
{
    if (mUsage == D3DBufferUsage::DYNAMIC)
    {
        // Note: This is not a safe math operation. 'dataSize' can come from the app.
        mUnmodifiedDataUse += dataSize;

        if (mUnmodifiedDataUse > 3 * getSize())
        {
            updateD3DBufferUsage(context, gl::BufferUsage::StaticDraw);
        }
    }
}

angle::Result BufferD3D::getIndexRange(const gl::Context *context,
                                       gl::DrawElementsType type,
                                       size_t offset,
                                       size_t count,
                                       bool primitiveRestartEnabled,
                                       gl::IndexRange *outRange)
{
    const uint8_t *data = nullptr;
    ANGLE_TRY(getData(context, &data));

    *outRange = gl::ComputeIndexRange(type, data + offset, count, primitiveRestartEnabled);
    return angle::Result::Continue;
}

}  // namespace rx
