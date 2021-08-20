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

#include <utils/Panic.h>

#include <array>

namespace filament {
namespace backend {
namespace metal {

MetalBuffer::MetalBuffer(MetalContext& context, BufferUsage usage, size_t size, bool forceGpuBuffer)
        : mBufferSize(size), mContext(context) {
    // If the buffer is less than 4K in size and is updated frequently, we don't use an explicit
    // buffer. Instead, we use immediate command encoder methods like setVertexBytes:length:atIndex:.
    if (size <= 4 * 1024) {
        // We'll take the same approach for STREAM buffers under 4K.
        if (usage == BufferUsage::STREAM) {
            usage = BufferUsage::DYNAMIC;
        }

        if (usage == BufferUsage::DYNAMIC && !forceGpuBuffer) {
            mBufferPoolEntry = nullptr;
            mCpuBuffer = malloc(size);
        }
    }

    mUsage = usage;
}

MetalBuffer::~MetalBuffer() {
    if (mCpuBuffer) {
        free(mCpuBuffer);
    }
    // This buffer is being destroyed. If we have a buffer pool entry, release it as it is no longer
    // needed.
    if (mBufferPoolEntry) {
        mContext.bufferPool->releaseBuffer(mBufferPoolEntry);
    }
}

void MetalBuffer::copyIntoBuffer(void* src, size_t size, size_t byteOffset) {
    if (size <= 0) {
        return;
    }
    ASSERT_PRECONDITION(size + byteOffset <= mBufferSize,
            "Attempting to copy %d bytes into a buffer of size %d at offset %d",
            size, mBufferSize, byteOffset);

    if (mUsage == BufferUsage::STREAM) {
        // byteOffset must be 0 for STREAM buffers.
        assert_invariant(byteOffset == 0);
        copyIntoStreamBuffer(src, size);
        return;
    }

    // Either copy into the Metal buffer or into our cpu buffer.
    if (mCpuBuffer) {
        memcpy(static_cast<uint8_t*>(mCpuBuffer) + byteOffset, src, size);
        return;
    }

    // We're about to acquire a new buffer to hold the new contents. If we previously had obtained a
    // buffer we release it, decrementing its reference count, as we no longer needs it.
    if (mBufferPoolEntry) {
        mContext.bufferPool->releaseBuffer(mBufferPoolEntry);
    }

    mBufferPoolEntry = mContext.bufferPool->acquireBuffer(mBufferSize);
    memcpy(static_cast<uint8_t*>(mBufferPoolEntry->buffer.contents) + byteOffset, src, size);
}

void MetalBuffer::copyIntoStreamBuffer(void* src, size_t size) {
    // Stream buffers are handled a bit differently. In STREAM mode, we have a single large buffer.
    // At each update we "advance" into the buffer, keeping track of the where we're at, and bind
    // the buffer at that location. Effectively, "byteOffset" is managed automatically.

    assert_invariant(size <= mBufferSize);
    assert_invariant(!mCpuBuffer);

    if (mCurrentStreamOffset + size > mBufferSize) {
        // Allocate a new buffer and reset the stream offset.
        mCurrentStreamOffset = 0;

        if (mBufferPoolEntry) { mContext.bufferPool->releaseBuffer(mBufferPoolEntry); }
        mBufferPoolEntry = mContext.bufferPool->acquireBuffer(mBufferSize);
    }

    // Lazily acquire a new buffer if we haven't already.
    if (!mBufferPoolEntry) {
        mBufferPoolEntry = mContext.bufferPool->acquireBuffer(mBufferSize);
    }

    memcpy(static_cast<uint8_t*>(mBufferPoolEntry->buffer.contents) + mCurrentStreamOffset, src, size);
}

id<MTLBuffer> MetalBuffer::getGpuBufferForDraw(id<MTLCommandBuffer> cmdBuffer) noexcept {
    if (!mBufferPoolEntry) {
        // If there's a CPU buffer, then we return nil here, as the CPU-side buffer will be bound
        // separately.
        if (mCpuBuffer) {
            return nil;
        }

        // If there isn't a CPU buffer, it means no data has been loaded into this buffer yet. To
        // avoid an error, we'll allocate an empty buffer.
        mBufferPoolEntry = mContext.bufferPool->acquireBuffer(mBufferSize);
    }

    // This buffer is being used in a draw call, so we retain it so it's not released back into the
    // buffer pool until the frame has finished.
    auto uniformDeleter = [bufferPool = mContext.bufferPool] (const void* resource) {
        bufferPool->releaseBuffer((const MetalBufferPoolEntry*) resource);
    };
    if (mContext.resourceTracker.trackResource((__bridge void*) cmdBuffer, mBufferPoolEntry,
            uniformDeleter)) {
        // We only want to retain the buffer once per command buffer- trackResource will return
        // true if this is the first time tracking this uniform for this command buffer.
        mContext.bufferPool->retainBuffer(mBufferPoolEntry);
    }

    return mBufferPoolEntry->buffer;
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
        metalOffsets[b] = offsets[b] + buffer->mCurrentStreamOffset;
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

} // namespace metal
} // namespace backend
} // namespace filament
