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

#include <utils/Panic.h>

namespace filament {
namespace backend {

std::array<std::atomic<uint64_t>, TrackedMetalBuffer::TypeCount> TrackedMetalBuffer::aliveBuffers = { 0 };
PlatformMetal* TrackedMetalBuffer::platform = nullptr;
PlatformMetal* ScopedAllocationTimer::platform = nullptr;

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

    MTLResourceOptions options = MTLResourceStorageModePrivate;

    // The buffer will be memory mapped for write operations.
    if (any(usage & BufferUsage::SHARED_WRITE_BIT)) {
#if defined(FILAMENT_IOS) || defined(__arm64__) || defined(__aarch64__)
        // iOS and Apple Silicon devices use UMA (Unified Memory Architecture), so we use Shared memory.
        options = MTLResourceStorageModeShared;
#else
        // Intel Macs require Managed memory for CPU/GPU synchronization.
        options = MTLResourceStorageModeManaged;
#endif
    }

    {
        ScopedAllocationTimer timer("generic");
        mBuffer = { [context.device newBufferWithLength:size options:options],
            TrackedMetalBuffer::Type::GENERIC };
    }

    // mBuffer might fail to be allocated. Clients can check for this by calling
    // wasAllocationSuccessful().
}

MetalBuffer::~MetalBuffer() = default;

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
