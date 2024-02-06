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

#ifndef TNT_FILAMENT_DRIVER_METALBUFFER_H
#define TNT_FILAMENT_DRIVER_METALBUFFER_H

#include "MetalContext.h"

#include <backend/DriverEnums.h>

#include <Metal/Metal.h>

#include <utils/compiler.h>

#include <utility>
#include <memory>
#include <atomic>

namespace filament::backend {

class TrackedMetalBuffer {
public:
    TrackedMetalBuffer() noexcept : mBuffer(nil) {}
    TrackedMetalBuffer(id<MTLBuffer> buffer) noexcept : mBuffer(buffer) { aliveBuffers++; }

    TrackedMetalBuffer(TrackedMetalBuffer&&) = delete;
    TrackedMetalBuffer(TrackedMetalBuffer const&) = delete;
    TrackedMetalBuffer& operator=(TrackedMetalBuffer const&) = delete;

    ~TrackedMetalBuffer() {
        if (mBuffer) {
            aliveBuffers--;
        }
    }

    TrackedMetalBuffer& operator=(TrackedMetalBuffer&& rhs) noexcept {
        swap(rhs);
        return *this;
    }

    id<MTLBuffer> get() const noexcept { return mBuffer; }
    operator bool() const noexcept { return bool(mBuffer); }

    static uint64_t getAliveBuffers() { return aliveBuffers; }

private:
    void swap(TrackedMetalBuffer& other) noexcept {
        id<MTLBuffer> temp = mBuffer;
        mBuffer = other.mBuffer;
        other.mBuffer = temp;
    }

    id<MTLBuffer> mBuffer;

    static std::atomic<uint64_t> aliveBuffers;
};

class MetalBuffer {
public:

    MetalBuffer(MetalContext& context, BufferObjectBinding bindingType, BufferUsage usage,
         size_t size, bool forceGpuBuffer = false);
    ~MetalBuffer();

    MetalBuffer(const MetalBuffer& rhs) = delete;
    MetalBuffer& operator=(const MetalBuffer& rhs) = delete;

    size_t getSize() const noexcept { return mBufferSize; }

    /**
     * Update the buffer with data inside src. Potentially allocates a new buffer allocation to hold
     * the bytes which will be released when the current frame is finished.
     */
    void copyIntoBuffer(void* src, size_t size, size_t byteOffset);
    void copyIntoBufferUnsynchronized(void* src, size_t size, size_t byteOffset);

    /**
     * Denotes that this buffer is used for a draw call ensuring that its allocation remains valid
     * until the end of the current frame.
     *
     * @return The MTLBuffer representing the current state of the buffer to bind, or nil if there
     * is no device allocation.
     *
     */
    id<MTLBuffer> getGpuBufferForDraw(id<MTLCommandBuffer> cmdBuffer) noexcept;

    void* getCpuBuffer() const noexcept { return mCpuBuffer; }

    enum Stage : uint8_t {
        VERTEX      = 1u << 0u,
        FRAGMENT    = 1u << 1u,
        COMPUTE     = 1u << 2u
    };

    /**
     * Bind multiple buffers to pipeline stages.
     *
     * bindBuffers binds an array of buffers to the given stage(s) of a MTLCommandEncoders's
     * pipeline. The encoder must be either a MTLRenderCommandEncoder or a MTLComputeCommandEncoder.
     * For MTLRenderCommandEncoders, only the VERTEX and FRAGMENT stages may be specified.
     * For MTLComputeCommandEncoders, only the COMPUTE stage may be specified.
     */
    static void bindBuffers(id<MTLCommandBuffer> cmdBuffer, id<MTLCommandEncoder> encoder,
            size_t bufferStart, uint8_t stages, MetalBuffer* const* buffers, size_t const* offsets,
            size_t count);

private:

    TrackedMetalBuffer mBuffer;
    size_t mBufferSize = 0;
    void* mCpuBuffer = nullptr;
    MetalContext& mContext;
};

template <typename TYPE>
static inline TYPE align(TYPE p, size_t alignment) noexcept {
    // alignment must be a power-of-two
    assert(alignment && !(alignment & alignment-1));
    return (TYPE)((p + alignment - 1) & ~(alignment - 1));
}

/**
 * Manages a single id<MTLBuffer>, allowing sub-allocations in a "ring" fashion. Each slot in the
 * buffer has a fixed size. When a new allocation is made, previous allocations become available
 * when the current id<MTLCommandBuffer> has finished executing on the GPU.
 *
 * If there are no slots available when a new allocation is requested, MetalRingBuffer falls back to
 * allocating a new id<MTLBuffer> per allocation until a slot is freed.
 *
 * All methods must be called from the Metal backend thread.
 */
class MetalRingBuffer {
public:
    // In practice, MetalRingBuffer is used for argument buffers, which are kept in the constant
    // address space. Constant buffers have specific alignment requirements when specifying an
    // offset.
#if defined(IOS)
#if TARGET_OS_SIMULATOR
    // The iOS simulator has differing alignment requirements.
    static constexpr auto METAL_CONSTANT_BUFFER_OFFSET_ALIGNMENT = 256;
#else
    static constexpr auto METAL_CONSTANT_BUFFER_OFFSET_ALIGNMENT = 4;
#endif  // TARGET_OS_SIMULATOR
#else
    static constexpr auto METAL_CONSTANT_BUFFER_OFFSET_ALIGNMENT = 32;
#endif
    static inline auto computeSlotSize(MTLSizeAndAlign layout) {
         return align(align(layout.size, layout.align), METAL_CONSTANT_BUFFER_OFFSET_ALIGNMENT);
    }

    MetalRingBuffer(id<MTLDevice> device, MTLResourceOptions options, MTLSizeAndAlign layout,
            NSUInteger slotCount)
        : mDevice(device),
          mAuxBuffer(nil),
          mBufferOptions(options),
          mSlotSizeBytes(computeSlotSize(layout)),
          mSlotCount(slotCount) {
        mBuffer = [device newBufferWithLength:mSlotSizeBytes * mSlotCount options:mBufferOptions];
        assert_invariant(mBuffer);
    }

    /**
     * Create a new allocation in the buffer.
     * @param cmdBuffer When this command buffer has finished executing on the GPU, the previous
     *                  ring buffer allocation will be freed.
     * @return the id<MTLBuffer> and offset for the new allocation
     */
    std::pair<id<MTLBuffer>, NSUInteger> createNewAllocation(id<MTLCommandBuffer> cmdBuffer) {
        const auto occupiedSlots = mOccupiedSlots->load(std::memory_order_relaxed);
        assert_invariant(occupiedSlots <= mSlotCount);
        if (UTILS_UNLIKELY(occupiedSlots == mSlotCount)) {
            // We don't have any room left, so we fall back to creating a one-off aux buffer.
            // If we already have an aux buffer, it will get freed here, unless it has been retained
            // by a MTLCommandBuffer. In that case, it will be freed when the command buffer
            // finishes executing.
            mAuxBuffer = [mDevice newBufferWithLength:mSlotSizeBytes options:mBufferOptions];
            assert_invariant(mAuxBuffer);
            return {mAuxBuffer.get(), 0};
        }
        mCurrentSlot = (mCurrentSlot + 1) % mSlotCount;
        mOccupiedSlots->fetch_add(1, std::memory_order_relaxed);

        // Release the previous allocation.
        if (UTILS_UNLIKELY(mAuxBuffer)) {
            mAuxBuffer = nil;
        } else {
            // Capture the mOccupiedSlots var via a weak_ptr because the MetalRingBuffer could be
            // destructed before the block executes.
            std::weak_ptr<AtomicCounterType> slots = mOccupiedSlots;
            [cmdBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
                if (auto s = slots.lock()) {
                    s->fetch_sub(1, std::memory_order_relaxed);
                }
            }];
        }
        return getCurrentAllocation();
    }

    /**
     * Returns an allocation (buffer and offset) that is guaranteed not to be in use by the GPU.
     * @param cmdBuffer When this command buffer has finished executing on the GPU, the previous
     *                  ring buffer allocation will be freed.
     * @return the id<MTLBuffer> and offset for the current allocation
     */
    std::pair<id<MTLBuffer>, NSUInteger> getCurrentAllocation() const {
        if (UTILS_UNLIKELY(mAuxBuffer)) {
            return { mAuxBuffer.get(), 0 };
        }
        return { mBuffer.get(), mCurrentSlot * mSlotSizeBytes };
    }

    bool canAccomodateLayout(MTLSizeAndAlign layout) const {
        return mSlotSizeBytes >= computeSlotSize(layout);
    }

private:
    id<MTLDevice> mDevice;
    TrackedMetalBuffer mBuffer;
    TrackedMetalBuffer mAuxBuffer;

    MTLResourceOptions mBufferOptions;

    NSUInteger mSlotSizeBytes;
    NSUInteger mSlotCount;

    NSUInteger mCurrentSlot = 0;
    using AtomicCounterType = std::atomic<NSUInteger>;
    std::shared_ptr<AtomicCounterType> mOccupiedSlots = std::make_shared<AtomicCounterType>(1);
};

} // namespace filament::backend

#endif
