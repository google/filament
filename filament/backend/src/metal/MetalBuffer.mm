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

namespace filament {
namespace backend {
namespace metal {

MetalBuffer::MetalBuffer(MetalContext& context, size_t size, bool forceGpuBuffer)
        : mBufferSize(size), mContext(context) {
    // If the buffer is less than 4K in size, we don't use an explicit buffer and instead use
    // immediate command encoder methods like setVertexBytes:length:atIndex:.
    // TODO: we shouldn't do this if the data persists for multiple uses.
    if (size <= 4 * 1024 && !forceGpuBuffer) {   // 4K
        mBufferPoolEntry = nullptr;
        mCpuBuffer = malloc(size);
    }
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

void MetalBuffer::copyIntoBuffer(void* src, size_t size) {
    if (size <= 0) {
        return;
    }
    ASSERT_PRECONDITION(size <= mBufferSize, "Attempting to copy %d bytes into a buffer of size %d",
            size, mBufferSize);

    // Either copy into the Metal buffer or into our cpu buffer.
    if (mCpuBuffer) {
        memcpy(mCpuBuffer, src, size);
        return;
    }

    // We're about to acquire a new buffer to hold the new contents. If we previously had obtained a
    // buffer we release it, decrementing its reference count, as we no longer needs it.
    if (mBufferPoolEntry) {
        mContext.bufferPool->releaseBuffer(mBufferPoolEntry);
    }

    mBufferPoolEntry = mContext.bufferPool->acquireBuffer(mBufferSize);
    memcpy(static_cast<uint8_t*>(mBufferPoolEntry->buffer.contents), src, size);
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

    std::vector<id<MTLBuffer>> metalBuffers(count, nil);
    std::vector<size_t> metalOffsets(count, 0);

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

} // namespace metal
} // namespace backend
} // namespace filament
