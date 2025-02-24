//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BufferWgpu.cpp:
//    Implements the class methods for BufferWgpu.
//

#include "libANGLE/renderer/wgpu/BufferWgpu.h"

#include "common/debug.h"
#include "common/mathutil.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/wgpu/ContextWgpu.h"
#include "libANGLE/renderer/wgpu/wgpu_utils.h"

namespace rx
{
namespace
{
// Based on a buffer binding target, compute the default wgpu usage flags. More can be added if the
// buffer is used in new ways.
wgpu::BufferUsage GetDefaultWGPUBufferUsageForBinding(gl::BufferBinding binding)
{
    switch (binding)
    {
        case gl::BufferBinding::Array:
        case gl::BufferBinding::ElementArray:
            return wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Index |
                   wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;

        case gl::BufferBinding::Uniform:
            return wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopySrc |
                   wgpu::BufferUsage::CopyDst;

        case gl::BufferBinding::PixelPack:
            return wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;

        case gl::BufferBinding::PixelUnpack:
            return wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;

        case gl::BufferBinding::CopyRead:
        case gl::BufferBinding::CopyWrite:
        case gl::BufferBinding::ShaderStorage:
        case gl::BufferBinding::Texture:
        case gl::BufferBinding::TransformFeedback:
        case gl::BufferBinding::DispatchIndirect:
        case gl::BufferBinding::DrawIndirect:
        case gl::BufferBinding::AtomicCounter:
            UNIMPLEMENTED();
            return wgpu::BufferUsage::None;

        default:
            UNREACHABLE();
            return wgpu::BufferUsage::None;
    }
}

}  // namespace

BufferWgpu::BufferWgpu(const gl::BufferState &state) : BufferImpl(state) {}

BufferWgpu::~BufferWgpu() {}

angle::Result BufferWgpu::setData(const gl::Context *context,
                                  gl::BufferBinding target,
                                  const void *data,
                                  size_t size,
                                  gl::BufferUsage usage)
{
    ContextWgpu *contextWgpu = webgpu::GetImpl(context);
    wgpu::Device device      = webgpu::GetDevice(context);

    bool hasData = data && size > 0;

    // Allocate a new buffer if the current one is invalid, the size is different, or the current
    // buffer cannot be mapped for writing when data needs to be uploaded.
    if (!mBuffer.valid() || mBuffer.requestedSize() != size ||
        (hasData && !mBuffer.canMapForWrite()))
    {
        // Allocate a new buffer
        ANGLE_TRY(mBuffer.initBuffer(device, size, GetDefaultWGPUBufferUsageForBinding(target),
                                     webgpu::MapAtCreation::Yes));
    }

    if (hasData)
    {
        ASSERT(mBuffer.canMapForWrite());

        if (!mBuffer.getMappedState().has_value())
        {
            ANGLE_TRY(mBuffer.mapImmediate(contextWgpu, wgpu::MapMode::Write, 0, size));
        }

        uint8_t *mappedData = mBuffer.getMapWritePointer(0, size);
        memcpy(mappedData, data, size);
    }

    return angle::Result::Continue;
}

angle::Result BufferWgpu::setSubData(const gl::Context *context,
                                     gl::BufferBinding target,
                                     const void *data,
                                     size_t size,
                                     size_t offset)
{
    ContextWgpu *contextWgpu = webgpu::GetImpl(context);
    wgpu::Device device      = webgpu::GetDevice(context);

    ASSERT(mBuffer.valid());
    if (mBuffer.canMapForWrite())
    {
        if (!mBuffer.getMappedState().has_value())
        {
            ANGLE_TRY(mBuffer.mapImmediate(contextWgpu, wgpu::MapMode::Write, offset, size));
        }

        uint8_t *mappedData = mBuffer.getMapWritePointer(offset, size);
        memcpy(mappedData, data, size);
    }
    else
    {
        // TODO: Upload into a staging buffer and copy to the destination buffer so that the copy
        // happens at the right point in time for command buffer recording.
        wgpu::Queue &queue = contextWgpu->getQueue();
        queue.WriteBuffer(mBuffer.getBuffer(), offset, data, size);
    }

    return angle::Result::Continue;
}

angle::Result BufferWgpu::copySubData(const gl::Context *context,
                                      BufferImpl *source,
                                      GLintptr sourceOffset,
                                      GLintptr destOffset,
                                      GLsizeiptr size)
{
    return angle::Result::Continue;
}

angle::Result BufferWgpu::map(const gl::Context *context, GLenum access, void **mapPtr)
{
    return angle::Result::Continue;
}

angle::Result BufferWgpu::mapRange(const gl::Context *context,
                                   size_t offset,
                                   size_t length,
                                   GLbitfield access,
                                   void **mapPtr)
{
    return angle::Result::Continue;
}

angle::Result BufferWgpu::unmap(const gl::Context *context, GLboolean *result)
{
    *result = GL_TRUE;
    return angle::Result::Continue;
}

angle::Result BufferWgpu::getIndexRange(const gl::Context *context,
                                        gl::DrawElementsType type,
                                        size_t offset,
                                        size_t count,
                                        bool primitiveRestartEnabled,
                                        gl::IndexRange *outRange)
{
    ContextWgpu *contextWgpu = webgpu::GetImpl(context);
    wgpu::Device device      = webgpu::GetDevice(context);

    if (mBuffer.getMappedState())
    {
        ANGLE_TRY(mBuffer.unmap());
    }

    // Create a staging buffer just big enough for this index range
    const GLuint typeBytes = gl::GetDrawElementsTypeSize(type);
    const size_t stagingBufferSize =
        roundUpPow2(count * typeBytes, webgpu::kBufferCopyToBufferAlignment);

    webgpu::BufferHelper stagingBuffer;
    ANGLE_TRY(stagingBuffer.initBuffer(device, stagingBufferSize,
                                       wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead,
                                       webgpu::MapAtCreation::No));

    // Copy the source buffer to staging and flush the commands
    contextWgpu->ensureCommandEncoderCreated();
    wgpu::CommandEncoder &commandEncoder = contextWgpu->getCurrentCommandEncoder();
    ASSERT(offset == rx::roundDownPow2(offset, webgpu::kBufferCopyToBufferAlignment));
    commandEncoder.CopyBufferToBuffer(mBuffer.getBuffer(), offset, stagingBuffer.getBuffer(), 0,
                                      stagingBufferSize);

    ANGLE_TRY(contextWgpu->flush(webgpu::RenderPassClosureReason::IndexRangeReadback));

    // Read back from the staging buffer and compute the index range
    ANGLE_TRY(stagingBuffer.mapImmediate(contextWgpu, wgpu::MapMode::Read, 0, stagingBufferSize));
    const uint8_t *data = stagingBuffer.getMapReadPointer(0, stagingBufferSize);
    *outRange           = gl::ComputeIndexRange(type, data, count, primitiveRestartEnabled);
    ANGLE_TRY(stagingBuffer.unmap());

    return angle::Result::Continue;
}

}  // namespace rx
