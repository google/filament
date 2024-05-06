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
#include "MetalBufferPool.h"

#include "MetalContext.h"

namespace filament {
namespace backend {

std::array<uint64_t, TrackedMetalBuffer::TypeCount> TrackedMetalBuffer::aliveBuffers = { 0 };
MetalPlatform* TrackedMetalBuffer::platform = nullptr;

MetalBuffer::MetalBuffer(MetalContext& context, BufferObjectBinding bindingType, BufferUsage usage,
        size_t size, bool forceGpuBuffer) : mBufferSize(size), mContext(context) {
    // If the buffer is less than 4K in size and is updated frequently, we don't use an explicit
    // buffer. Instead, we use immediate command encoder methods like setVertexBytes:length:atIndex:.
    // This won't work for SSBOs, since they are read/write.
    if (size <= 4 * 1024 && bindingType != BufferObjectBinding::SHADER_STORAGE &&
            usage == BufferUsage::DYNAMIC && !forceGpuBuffer) {
        mBuffer = nil;
        mCpuBuffer = malloc(size);
        return;
    }

    // Otherwise, we allocate a private GPU buffer.
    mBuffer = { [context.device newBufferWithLength:size options:MTLResourceStorageModePrivate],
        TrackedMetalBuffer::Type::GENERIC };
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
    memcpy(staging->buffer.get().contents, src, size);

    // The blit below requires that byteOffset be a multiple of 4.
    ASSERT_PRECONDITION(!(byteOffset & 0x3u), "byteOffset must be a multiple of 4");

    // Encode a blit from the staging buffer into the private GPU buffer.
    id<MTLCommandBuffer> cmdBuffer = getPendingCommandBuffer(&mContext);
    id<MTLBlitCommandEncoder> blitEncoder = [cmdBuffer blitCommandEncoder];
    blitEncoder.label = @"Buffer upload blit";
    [blitEncoder copyFromBuffer:staging->buffer.get()
                   sourceOffset:0
                       toBuffer:mBuffer.get()
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
    return mBuffer.get();
}

void MetalBuffer::bindBuffers(id<MTLCommandBuffer> cmdBuffer, id<MTLCommandEncoder> encoder,
        size_t bufferStart, uint8_t stages, MetalBuffer* const* buffers, size_t const* offsets,
        size_t count) {
    // Ensure we were given the correct type of encoder:
    // either a MTLRenderCommandEncoder or a MTLComputeCommandEncoder.
    if (stages & MetalBuffer::Stage::VERTEX || stages & MetalBuffer::Stage::FRAGMENT) {
        assert_invariant([encoder respondsToSelector:@selector(setVertexBuffers:offsets:withRange:)]);
        assert_invariant([encoder respondsToSelector:@selector(setFragmentBuffers:offsets:withRange:)]);
        assert_invariant([encoder respondsToSelector:@selector(setVertexBytes:length:atIndex:)]);
        assert_invariant([encoder respondsToSelector:@selector(setFragmentBytes:length:atIndex:)]);
        assert_invariant(!(stages & MetalBuffer::Stage::COMPUTE));
    }
    if (stages & MetalBuffer::Stage::COMPUTE) {
        assert_invariant([encoder respondsToSelector:@selector(setBuffers:offsets:withRange:)]);
        assert_invariant([encoder respondsToSelector:@selector(setBytes:length:atIndex:)]);
        assert_invariant(!(stages & (MetalBuffer::Stage::FRAGMENT | MetalBuffer::Stage::VERTEX)));
    }

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
        [(id<MTLRenderCommandEncoder>) encoder setVertexBuffers:metalBuffers.data()
                                                        offsets:metalOffsets.data()
                                                      withRange:bufferRange];
    }
    if (stages & Stage::FRAGMENT) {
        [(id<MTLRenderCommandEncoder>) encoder setFragmentBuffers:metalBuffers.data()
                                                          offsets:metalOffsets.data()
                                                        withRange:bufferRange];
    }
    if (stages & Stage::COMPUTE) {
        [(id<MTLComputeCommandEncoder>) encoder setBuffers:metalBuffers.data()
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
            [(id<MTLRenderCommandEncoder>) encoder setVertexBytes:(bytes + offset)
                                                           length:(buffer->getSize() - offset)
                                                          atIndex:bufferIndex];
        }
        if (stages & Stage::FRAGMENT) {
            [(id<MTLRenderCommandEncoder>) encoder setFragmentBytes:(bytes + offset)
                                                             length:(buffer->getSize() - offset)
                                                            atIndex:bufferIndex];
        }
        if (stages & Stage::COMPUTE) {
            // TODO: using setBytes means the data is read-only, which currently isn't enforced.
            // In practice this won't be an issue since MetalBuffer ensures all SSBOs are realized
            // through actual id<MTLBuffer> allocations.
            [(id<MTLComputeCommandEncoder>) encoder setBytes:(bytes + offset)
                                                      length:(buffer->getSize() - offset)
                                                     atIndex:bufferIndex];
        }
    }
}

} // namespace backend
} // namespace filament
