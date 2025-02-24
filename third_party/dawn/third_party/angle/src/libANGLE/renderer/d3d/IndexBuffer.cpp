//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexBuffer.cpp: Defines the abstract IndexBuffer class and IndexBufferInterface
// class with derivations, classes that perform graphics API agnostic index buffer operations.

#include "libANGLE/renderer/d3d/IndexBuffer.h"

#include "libANGLE/Context.h"
#include "libANGLE/renderer/d3d/ContextD3D.h"

namespace rx
{

unsigned int IndexBuffer::mNextSerial = 1;

IndexBuffer::IndexBuffer()
{
    updateSerial();
}

IndexBuffer::~IndexBuffer() {}

unsigned int IndexBuffer::getSerial() const
{
    return mSerial;
}

void IndexBuffer::updateSerial()
{
    mSerial = mNextSerial++;
}

IndexBufferInterface::IndexBufferInterface(BufferFactoryD3D *factory, bool dynamic)
{
    mIndexBuffer = factory->createIndexBuffer();

    mDynamic       = dynamic;
    mWritePosition = 0;
}

IndexBufferInterface::~IndexBufferInterface()
{
    if (mIndexBuffer)
    {
        delete mIndexBuffer;
    }
}

gl::DrawElementsType IndexBufferInterface::getIndexType() const
{
    return mIndexBuffer->getIndexType();
}

unsigned int IndexBufferInterface::getBufferSize() const
{
    return mIndexBuffer->getBufferSize();
}

unsigned int IndexBufferInterface::getSerial() const
{
    return mIndexBuffer->getSerial();
}

angle::Result IndexBufferInterface::mapBuffer(const gl::Context *context,
                                              unsigned int size,
                                              void **outMappedMemory,
                                              unsigned int *streamOffset)
{
    // Protect against integer overflow
    bool check = (mWritePosition + size < mWritePosition);
    ANGLE_CHECK(GetImplAs<ContextD3D>(context), !check,
                "Mapping of internal index buffer would cause an integer overflow.",
                GL_OUT_OF_MEMORY);

    angle::Result error = mIndexBuffer->mapBuffer(context, mWritePosition, size, outMappedMemory);
    if (error == angle::Result::Stop)
    {
        if (outMappedMemory)
        {
            *outMappedMemory = nullptr;
        }
        return error;
    }

    if (streamOffset)
    {
        *streamOffset = mWritePosition;
    }

    mWritePosition += size;
    return angle::Result::Continue;
}

angle::Result IndexBufferInterface::unmapBuffer(const gl::Context *context)
{
    return mIndexBuffer->unmapBuffer(context);
}

IndexBuffer *IndexBufferInterface::getIndexBuffer() const
{
    return mIndexBuffer;
}

unsigned int IndexBufferInterface::getWritePosition() const
{
    return mWritePosition;
}

void IndexBufferInterface::setWritePosition(unsigned int writePosition)
{
    mWritePosition = writePosition;
}

angle::Result IndexBufferInterface::discard(const gl::Context *context)
{
    return mIndexBuffer->discard(context);
}

angle::Result IndexBufferInterface::setBufferSize(const gl::Context *context,
                                                  unsigned int bufferSize,
                                                  gl::DrawElementsType indexType)
{
    if (mIndexBuffer->getBufferSize() == 0)
    {
        return mIndexBuffer->initialize(context, bufferSize, indexType, mDynamic);
    }
    else
    {
        return mIndexBuffer->setSize(context, bufferSize, indexType);
    }
}

StreamingIndexBufferInterface::StreamingIndexBufferInterface(BufferFactoryD3D *factory)
    : IndexBufferInterface(factory, true)
{}

StreamingIndexBufferInterface::~StreamingIndexBufferInterface() {}

angle::Result StreamingIndexBufferInterface::reserveBufferSpace(const gl::Context *context,
                                                                unsigned int size,
                                                                gl::DrawElementsType indexType)
{
    unsigned int curBufferSize = getBufferSize();
    unsigned int writePos      = getWritePosition();
    if (size > curBufferSize)
    {
        ANGLE_TRY(setBufferSize(context, std::max(size, 2 * curBufferSize), indexType));
        setWritePosition(0);
    }
    else if (writePos + size > curBufferSize || writePos + size < writePos)
    {
        ANGLE_TRY(discard(context));
        setWritePosition(0);
    }

    return angle::Result::Continue;
}

StaticIndexBufferInterface::StaticIndexBufferInterface(BufferFactoryD3D *factory)
    : IndexBufferInterface(factory, false)
{}

StaticIndexBufferInterface::~StaticIndexBufferInterface() {}

angle::Result StaticIndexBufferInterface::reserveBufferSpace(const gl::Context *context,
                                                             unsigned int size,
                                                             gl::DrawElementsType indexType)
{
    unsigned int curSize = getBufferSize();
    if (curSize == 0)
    {
        return setBufferSize(context, size, indexType);
    }

    ASSERT(curSize >= size && indexType == getIndexType());
    return angle::Result::Continue;
}

}  // namespace rx
