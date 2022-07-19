/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MetalBuffer.h"

#include "MetalContext.h"

namespace filament {
namespace backend {

MetalBuffer::MetalBuffer(MetalContext& context, BufferUsage usage, size_t size, bool forceGpuBuffer)
        : mBufferSize(size), mContext(context) {
    // If the buffer is less than 4K in size and is updated frequently, we don't use an explicit
    // buffer. Instead, we use immediate command encoder methods like setVertexBytes:length:atIndex:.
   if (size <= 4 * 1024 && usage == BufferUsage::DYNAMIC && !forceGpuBuffer) {
       mBuffer = nil;
       mCpuBuffer = malloc(size);
       return;
   }

    // Otherwise, we allocate a private GPU buffer.
    mBuffer = [context.device newBufferWithLength:size options:MTLResourceStorageModePrivate];
    ASSERT_POSTCONDITION(mBuffer, "Could not allocate Metal buffer of size %zu.", size);
}

MetalBuffer::~MetalBuffer() {
    if (mCpuBuffer) {
        free(mCpuBuffer);
    }
}

void MetalBuffer::copyIntoBuffer(void* src, size_t size, size_t byteOffset) {
    if (size <= 0) {
        return;
    }
    ASSERT_PRECONDITION(size + byteOffset <= mBufferSize,
            "Attempting to copy %zu bytes into a buffer of size %zu at offset %zu",
            size, mBufferSize, byteOffset);

    // Either copy into the Metal buffer or into our cpu buffer.
    if (mCpuBuffer) {
        memcpy(static_cast<uint8_t*>(mCpuBuffer) + byteOffset, src, size);
        return;
    }

    // Acquire a staging buffer to hold the contents of this update.
    MetalBufferPool* bufferPool = mContext.bufferPool;
    const MetalBufferPoolEntry* const staging = bufferPool->acquireBuffer(size);
    memcpy(staging->buffer.contents, src, size);

    // The blit below requires that byteOffset be a multiple of 4.
    ASSERT_PRECONDITION(!(byteOffset & 0x3u), "byteOffset must be a multiple of 4");

    // Encode a blit from the staging buffer into the private GPU buffer.
    id<MTLCommandBuffer> cmdBuffer = getPendingCommandBuffer(&mContext);
    id<MTLBlitCommandEncoder> blitEncoder = [cmdBuffer blitCommandEncoder];
    [blitEncoder copyFromBuffer:staging->buffer
                   sourceOffset:0
                       toBuffer:mBuffer
              destinationOffset:byteOffset
                           size:size];
    [blitEncoder endEncoding];
    [cmdBuffer addCompletedHandler:^(id<MTLCommandBuffer> cb) {
        bufferPool->releaseBuffer(staging);
    }];
}

void MetalBuffer::copyIntoBufferUnsynchronized(void* src, size_t size, size_t byteOffset) {
    // TODO: implement the unsynchronized version
    copyIntoBuffer(src, size, byteOffset);
}

id<MTLBuffer> MetalBuffer::getGpuBufferForDraw(id<MTLCommandBuffer> cmdBuffer) noexcept {
    // If there's a CPU buffer, then we return nil here, as the CPU-side buffer will be bound
    // separately.
    if (mCpuBuffer) {
        return nil;
    }
    assert_invariant(mBuffer);
    return mBuffer;
}

void MetalBuffer::bindBuffers(id<MTLCommandBuffer> cmdBuffer, id<MTLRenderCommandEncoder> encoder,
        size_t bufferStart, uint8_t stages, MetalBuffer* const* buffers, size_t const* offsets,
        size_t count) {
    const NSRange bufferRange = NSMakeRange(bufferStart, count);

    constexpr size_t MAX_BUFFERS = 16;
    assert_invariant(count <= MAX_BUFFERS);

    std::array<id<MTLBuffer>, MAX_BUFFERS> metalBuffers = {};
    std::array<size_t, MAX_BUFFERS> metalOffsets = {};

    for (size_t b = 0; b < count; b++) {
        MetalBuffer* const buffer = buffers[b];
        if (!buffer) {
            continue;
        }
        // getGpuBufferForDraw() might return nil, which means there isn't a device allocation for
        // this buffer. In this case, we'll bind the buffer below with the CPU-side memory.
        id<MTLBuffer> gpuBuffer = buffer->getGpuBufferForDraw(cmdBuffer);
        if (!gpuBuffer) {
            continue;
        }
        metalBuffers[b] = gpuBuffer;
        metalOffsets[b] = offsets[b];
    }

    if (stages & Stage::VERTEX) {
        [encoder setVertexBuffers:metalBuffers.data()
                          offsets:metalOffsets.data()
                        withRange:bufferRange];
    }
    if (stages & Stage::FRAGMENT) {
        [encoder setFragmentBuffers:metalBuffers.data()
                            offsets:metalOffsets.data()
                          withRange:bufferRange];
    }

    for (size_t b = 0; b < count; b++) {
        MetalBuffer* const buffer = buffers[b];
        if (!buffer) {
            continue;
        }

        const void* cpuBuffer = buffer->getCpuBuffer();
        if (!cpuBuffer) {
            continue;
        }

        const size_t bufferIndex = bufferStart + b;
        const size_t offset = offsets[b];
        auto* bytes = static_cast<const uint8_t*>(cpuBuffer);

        if (stages & Stage::VERTEX) {
            [encoder setVertexBytes:(bytes + offset)
                             length:(buffer->getSize() - offset)
                            atIndex:bufferIndex];
        }
        if (stages & Stage::FRAGMENT) {
            [encoder setFragmentBytes:(bytes + offset)
                               length:(buffer->getSize() - offset)
                              atIndex:bufferIndex];
        }
    }
}

} // namespace backend
} // namespace filament
