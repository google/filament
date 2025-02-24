//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexBuffer.cpp: Defines the abstract VertexBuffer class and VertexBufferInterface
// class with derivations, classes that perform graphics API agnostic vertex buffer operations.

#include "libANGLE/renderer/d3d/VertexBuffer.h"

#include "common/mathutil.h"
#include "libANGLE/Context.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/renderer/d3d/BufferD3D.h"
#include "libANGLE/renderer/d3d/ContextD3D.h"

namespace rx
{

// VertexBuffer Implementation
unsigned int VertexBuffer::mNextSerial = 1;

VertexBuffer::VertexBuffer() : mRefCount(1)
{
    updateSerial();
}

VertexBuffer::~VertexBuffer() {}

void VertexBuffer::updateSerial()
{
    mSerial = mNextSerial++;
}

unsigned int VertexBuffer::getSerial() const
{
    return mSerial;
}

void VertexBuffer::addRef()
{
    mRefCount++;
}

void VertexBuffer::release()
{
    ASSERT(mRefCount > 0);
    mRefCount--;

    if (mRefCount == 0)
    {
        delete this;
    }
}

// VertexBufferInterface Implementation
VertexBufferInterface::VertexBufferInterface(BufferFactoryD3D *factory, bool dynamic)
    : mFactory(factory), mVertexBuffer(factory->createVertexBuffer()), mDynamic(dynamic)
{}

VertexBufferInterface::~VertexBufferInterface()
{
    if (mVertexBuffer)
    {
        mVertexBuffer->release();
        mVertexBuffer = nullptr;
    }
}

unsigned int VertexBufferInterface::getSerial() const
{
    ASSERT(mVertexBuffer);
    return mVertexBuffer->getSerial();
}

unsigned int VertexBufferInterface::getBufferSize() const
{
    ASSERT(mVertexBuffer);
    return mVertexBuffer->getBufferSize();
}

angle::Result VertexBufferInterface::setBufferSize(const gl::Context *context, unsigned int size)
{
    ASSERT(mVertexBuffer);
    if (mVertexBuffer->getBufferSize() == 0)
    {
        return mVertexBuffer->initialize(context, size, mDynamic);
    }

    return mVertexBuffer->setBufferSize(context, size);
}

angle::Result VertexBufferInterface::getSpaceRequired(const gl::Context *context,
                                                      const gl::VertexAttribute &attrib,
                                                      const gl::VertexBinding &binding,
                                                      size_t count,
                                                      GLsizei instances,
                                                      GLuint baseInstance,
                                                      unsigned int *spaceInBytesOut) const
{
    unsigned int spaceRequired = 0;
    ANGLE_TRY(mFactory->getVertexSpaceRequired(context, attrib, binding, count, instances,
                                               baseInstance, &spaceRequired));

    // Align to 16-byte boundary
    unsigned int alignedSpaceRequired = roundUpPow2(spaceRequired, 16u);
    ANGLE_CHECK_GL_ALLOC(GetImplAs<ContextD3D>(context), alignedSpaceRequired >= spaceRequired);

    *spaceInBytesOut = alignedSpaceRequired;
    return angle::Result::Continue;
}

angle::Result VertexBufferInterface::discard(const gl::Context *context)
{
    ASSERT(mVertexBuffer);
    return mVertexBuffer->discard(context);
}

VertexBuffer *VertexBufferInterface::getVertexBuffer() const
{
    ASSERT(mVertexBuffer);
    return mVertexBuffer;
}

// StreamingVertexBufferInterface Implementation
StreamingVertexBufferInterface::StreamingVertexBufferInterface(BufferFactoryD3D *factory)
    : VertexBufferInterface(factory, true), mWritePosition(0), mReservedSpace(0)
{}

angle::Result StreamingVertexBufferInterface::initialize(const gl::Context *context,
                                                         std::size_t initialSize)
{
    return setBufferSize(context, static_cast<unsigned int>(initialSize));
}

void StreamingVertexBufferInterface::reset()
{
    if (mVertexBuffer)
    {
        mVertexBuffer->release();
        mVertexBuffer = mFactory->createVertexBuffer();
    }
}

StreamingVertexBufferInterface::~StreamingVertexBufferInterface() {}

angle::Result StreamingVertexBufferInterface::reserveSpace(const gl::Context *context,
                                                           unsigned int size)
{
    unsigned int curBufferSize = getBufferSize();
    if (size > curBufferSize)
    {
        ANGLE_TRY(setBufferSize(context, std::max(size, 3 * curBufferSize / 2)));
        mWritePosition = 0;
    }
    else if (mWritePosition + size > curBufferSize)
    {
        ANGLE_TRY(discard(context));
        mWritePosition = 0;
    }

    mReservedSpace = size;
    return angle::Result::Continue;
}

angle::Result StreamingVertexBufferInterface::storeDynamicAttribute(
    const gl::Context *context,
    const gl::VertexAttribute &attrib,
    const gl::VertexBinding &binding,
    gl::VertexAttribType currentValueType,
    GLint start,
    size_t count,
    GLsizei instances,
    GLuint baseInstance,
    unsigned int *outStreamOffset,
    const uint8_t *sourceData)
{
    unsigned int spaceRequired = 0;
    ANGLE_TRY(
        getSpaceRequired(context, attrib, binding, count, instances, baseInstance, &spaceRequired));

    // Protect against integer overflow
    angle::CheckedNumeric<unsigned int> checkedPosition(mWritePosition);
    checkedPosition += spaceRequired;
    ANGLE_CHECK_GL_ALLOC(GetImplAs<ContextD3D>(context), checkedPosition.IsValid());

    mReservedSpace = 0;

    size_t adjustedCount = count;
    GLuint divisor       = binding.getDivisor();

    if (instances != 0 && divisor != 0)
    {
        // The attribute is an instanced attribute and it's an draw instance call
        // Extra number of elements are copied at the beginning to make sure
        // the driver is referencing the correct data with non-zero baseInstance
        adjustedCount += UnsignedCeilDivide(baseInstance, divisor);
    }

    ANGLE_TRY(mVertexBuffer->storeVertexAttributes(context, attrib, binding, currentValueType,
                                                   start, adjustedCount, instances, mWritePosition,
                                                   sourceData));

    if (outStreamOffset)
    {
        *outStreamOffset = mWritePosition;
    }

    mWritePosition += spaceRequired;

    return angle::Result::Continue;
}

angle::Result StreamingVertexBufferInterface::reserveVertexSpace(const gl::Context *context,
                                                                 const gl::VertexAttribute &attrib,
                                                                 const gl::VertexBinding &binding,
                                                                 size_t count,
                                                                 GLsizei instances,
                                                                 GLuint baseInstance)
{
    unsigned int requiredSpace = 0;
    ANGLE_TRY(mFactory->getVertexSpaceRequired(context, attrib, binding, count, instances,
                                               baseInstance, &requiredSpace));

    // Align to 16-byte boundary
    auto alignedRequiredSpace = rx::CheckedRoundUp(requiredSpace, 16u);
    alignedRequiredSpace += mReservedSpace;

    // Protect against integer overflow
    ANGLE_CHECK_GL_ALLOC(GetImplAs<ContextD3D>(context), alignedRequiredSpace.IsValid());

    ANGLE_TRY(reserveSpace(context, alignedRequiredSpace.ValueOrDie()));

    return angle::Result::Continue;
}

// StaticVertexBufferInterface Implementation
StaticVertexBufferInterface::AttributeSignature::AttributeSignature()
    : formatID(angle::FormatID::NONE), stride(0), offset(0)
{}

bool StaticVertexBufferInterface::AttributeSignature::matchesAttribute(
    const gl::VertexAttribute &attrib,
    const gl::VertexBinding &binding) const
{
    size_t attribStride = ComputeVertexAttributeStride(attrib, binding);

    if (formatID != attrib.format->id || static_cast<GLuint>(stride) != attribStride)
    {
        return false;
    }

    size_t attribOffset =
        (static_cast<size_t>(ComputeVertexAttributeOffset(attrib, binding)) % attribStride);
    return (offset == attribOffset);
}

void StaticVertexBufferInterface::AttributeSignature::set(const gl::VertexAttribute &attrib,
                                                          const gl::VertexBinding &binding)
{
    formatID = attrib.format->id;
    offset = stride = static_cast<GLuint>(ComputeVertexAttributeStride(attrib, binding));
    offset          = static_cast<size_t>(ComputeVertexAttributeOffset(attrib, binding)) %
             ComputeVertexAttributeStride(attrib, binding);
}

StaticVertexBufferInterface::StaticVertexBufferInterface(BufferFactoryD3D *factory)
    : VertexBufferInterface(factory, false)
{}

StaticVertexBufferInterface::~StaticVertexBufferInterface() {}

bool StaticVertexBufferInterface::matchesAttribute(const gl::VertexAttribute &attrib,
                                                   const gl::VertexBinding &binding) const
{
    return mSignature.matchesAttribute(attrib, binding);
}

void StaticVertexBufferInterface::setAttribute(const gl::VertexAttribute &attrib,
                                               const gl::VertexBinding &binding)
{
    return mSignature.set(attrib, binding);
}

angle::Result StaticVertexBufferInterface::storeStaticAttribute(const gl::Context *context,
                                                                const gl::VertexAttribute &attrib,
                                                                const gl::VertexBinding &binding,
                                                                GLint start,
                                                                GLsizei count,
                                                                GLsizei instances,
                                                                const uint8_t *sourceData)
{
    unsigned int spaceRequired = 0;
    ANGLE_TRY(getSpaceRequired(context, attrib, binding, count, instances, 0, &spaceRequired));
    ANGLE_TRY(setBufferSize(context, spaceRequired));

    ASSERT(attrib.enabled);
    ANGLE_TRY(mVertexBuffer->storeVertexAttributes(context, attrib, binding,
                                                   gl::VertexAttribType::InvalidEnum, start, count,
                                                   instances, 0, sourceData));

    mSignature.set(attrib, binding);
    mVertexBuffer->hintUnmapResource();
    return angle::Result::Continue;
}

}  // namespace rx
