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
MetalPlatform* ScopedAllocationTimer::platform = nullptr;

MetalBuffer::MetalBuffer(MetalContext& context, BufferObjectBinding bindingType, BufferUsage usage,
        size_t size, bool forceGpuBuffer)
    : mBufferSize(size), mContext(context) {
    const MetalBumpAllocator& allocator = *mContext.bumpAllocator;
    // VERTEX is also used for index buffers
    if (allocator.getCapacity() > 0 && bindingType == BufferObjectBinding::VERTEX) {
        mUploadStrategy = UploadStrategy::BUMP_ALLOCATOR;
    } else {
        mUploadStrategy = UploadStrategy::POOL;
    }

    // If the buffer is less than 4K in size and is updated frequently, we don't use an explicit
    // buffer. Instead, we use immediate command encoder methods like setVertexBytes:length:atIndex:.
    // This won't work for SSBOs, since they are read/write.

    /*
    if (size <= 4 * 1024 && bindingType != BufferObjectBinding::SHADER_STORAGE &&
            usage == BufferUsage::DYNAMIC && !forceGpuBuffer) {
        mBuffer = nil;
        mCpuBuffer = malloc(size);
        return;
    }
    */

    // Otherwise, we allocate a private GPU buffer.
    {
        ScopedAllocationTimer timer("generic");
        mBuffer = { [context.device newBufferWithLength:size options:MTLResourceStorageModePrivate],
            TrackedMetalBuffer::Type::GENERIC };
    }
    // mBuffer might fail to be allocated. Clients can check for this by calling
    // wasAllocationSuccessful().
}

MetalBuffer::~MetalBuffer() {
    if (mCpuBuffer) {
        free(mCpuBuffer);
    }
}

void MetalBuffer::copyIntoBuffer(
        void* src, size_t size, size_t byteOffset, TagResolver&& getHandleTag) {
    if (size <= 0) {
        return;
    }

    FILAMENT_CHECK_PRECONDITION(src)
            << "copyIntoBuffer called with a null src, tag=" << getHandleTag();
    FILAMENT_CHECK_PRECONDITION(size + byteOffset <= mBufferSize)
            << "Attempting to copy " << size << " bytes into a buffer of size " << mBufferSize
            << " at offset " << byteOffset << ", tag=" << getHandleTag();
    // The copy blit requires that byteOffset be a multiple of 4.
    FILAMENT_CHECK_PRECONDITION(!(byteOffset & 0x3))
            << "byteOffset must be a multiple of 4, tag=" << getHandleTag();

    // If we have a cpu buffer, we can directly copy into it.
    if (mCpuBuffer) {
        memcpy(static_cast<uint8_t*>(mCpuBuffer) + byteOffset, src, size);
        return;
    }

    switch (mUploadStrategy) {
        case UploadStrategy::BUMP_ALLOCATOR:
            uploadWithBumpAllocator(src, size, byteOffset, std::move(getHandleTag));
            break;
        case UploadStrategy::POOL:
            uploadWithPoolBuffer(src, size, byteOffset, std::move(getHandleTag));
            break;
    }
}

void MetalBuffer::copyIntoBufferUnsynchronized(
        void* src, size_t size, size_t byteOffset, TagResolver&& getHandleTag) {
    // TODO: implement the unsynchronized version
    copyIntoBuffer(src, size, byteOffset, std::move(getHandleTag));
}

id<MTLBuffer> MetalBuffer::getGpuBufferForDraw() noexcept {
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
        id<MTLBuffer> gpuBuffer = buffer->getGpuBufferForDraw();
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

void MetalBuffer::uploadWithPoolBuffer(
        void* src, size_t size, size_t byteOffset, TagResolver&& getHandleTag) const {
    MetalBufferPool* bufferPool = mContext.bufferPool;
    const MetalBufferPoolEntry* const staging = bufferPool->acquireBuffer(size);
    FILAMENT_CHECK_POSTCONDITION(staging)
            << "uploadWithPoolbuffer unable to acquire staging buffer of size " << size
            << ", tag=" << getHandleTag();
    memcpy(staging->buffer.get().contents, src, size);

    // Encode a blit from the staging buffer into the private GPU buffer.
    id<MTLCommandBuffer> cmdBuffer = getPendingCommandBuffer(&mContext);
    id<MTLBlitCommandEncoder> blitEncoder = [cmdBuffer blitCommandEncoder];
    blitEncoder.label = @"Buffer upload blit - pool buffer";
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

void MetalBuffer::uploadWithBumpAllocator(
        void* src, size_t size, size_t byteOffset, TagResolver&& getHandleTag) const {
    MetalBumpAllocator& allocator = *mContext.bumpAllocator;
    auto [buffer, offset] = allocator.allocateStagingArea(size);
    FILAMENT_CHECK_POSTCONDITION(buffer)
            << "uploadWithBumpAllocator unable to acquire staging area of size " << size
            << ", tag=" << getHandleTag();
    void* const contents = buffer.contents;
    FILAMENT_CHECK_POSTCONDITION(contents)
            << "uploadWithBumpAllocator unable to acquire pointer to staging area, size " << size
            << ", tag=" << getHandleTag();
    memcpy(static_cast<char*>(contents) + offset, src, size);

    // Encode a blit from the staging buffer into the private GPU buffer.
    id<MTLCommandBuffer> cmdBuffer = getPendingCommandBuffer(&mContext);
    id<MTLBlitCommandEncoder> blitEncoder = [cmdBuffer blitCommandEncoder];
    blitEncoder.label = @"Buffer upload blit - bump allocator";
    [blitEncoder copyFromBuffer:buffer
                   sourceOffset:offset
                       toBuffer:mBuffer.get()
              destinationOffset:byteOffset
                           size:size];
    [blitEncoder endEncoding];
}

} // namespace backend
} // namespace filament
