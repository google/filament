/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANBUFFER_H
#define TNT_FILAMENT_BACKEND_VULKANBUFFER_H

#include "VulkanConstants.h"
#include "VulkanMemory.h"

#include "memory/Resource.h"

#include "vulkan/utils/CommandBufferAgeCounter.h"

#include <utils/bitset.h>
#include <utils/debug.h>

#include <cstdint>
#include <functional>

namespace filament::backend {

class VulkanBuffer : public fvkmemory::Resource {
public:
    // Because we need to recycle the unused `VulkanGpuBuffer`, we allow for a callback that the
    // "Pool" can use to acquire the buffer back.
    using OnRecycle = std::function<void(VulkanGpuBuffer const*)>;
    using ActiveBuffers = fvkutils::CommandBufferAgeCounter::ActiveBuffers;

    VulkanBuffer(VulkanGpuBuffer const* gpuBuffer, OnRecycle&& onRecycleFn)
        : mGpuBuffer(gpuBuffer),
          mOnRecycleFn(onRecycleFn),
          mLastBarrierAge(0) {}

    ~VulkanBuffer() {
        if (mOnRecycleFn) {
            mOnRecycleFn(mGpuBuffer);
        }
    }

    VulkanGpuBuffer const* getGpuBuffer() const { return mGpuBuffer; }

    // Records that the command buffer of age `age` reads from this buffer. `barrierAge` is the
    // most recent age known to be completed by the command buffer's pool.
    void read(uint64_t barrierAge, uint64_t age) {
        markInFlight(barrierAge, age);
        mLastReadAge = age;
    }

    // Records that the command buffer of age `age` writes into this buffer on the GPU (e.g. via
    // vkCmdCopyBuffer). This keeps the buffer "in flight" without requiring a read barrier.
    void write(uint64_t barrierAge, uint64_t age) {
        markInFlight(barrierAge, age);
    }

    // Returns the ages of the command buffers that read from or wrote into this buffer. Bit `i`
    // corresponds to the age `first + i + 1`.
    std::pair<uint64_t, ActiveBuffers> getAges() const {
        return { mLastBarrierAge, mUsedByAges };
    }

    // Returns true if the command buffer of age `age` has recorded a read of this buffer.
    bool isReadBy(uint64_t age) const {
        return age != 0 && mLastReadAge == age;
    }

    bool isMemcopyable() const {
        assert_invariant(mGpuBuffer);
        return mGpuBuffer->allocationInfo.pMappedData != nullptr;
    }

private:
    void markInFlight(uint64_t barrierAge, uint64_t age) {
        if (barrierAge > mLastBarrierAge) {
            // Every set bit is at most MAX_ACTIVE ages past its base, so moving the base by at
            // least that much means every recorded age has completed.
            if (barrierAge - mLastBarrierAge >= fvkutils::CommandBufferAgeCounter::MAX_ACTIVE) {
                mUsedByAges = {};
            } else {
                mUsedByAges >>= barrierAge - mLastBarrierAge;
            }
        }
        mLastBarrierAge = barrierAge;
        mUsedByAges.set(age - barrierAge - 1);
    }

    VulkanGpuBuffer const* mGpuBuffer;
    OnRecycle mOnRecycleFn;

    ActiveBuffers mUsedByAges;
    uint64_t mLastBarrierAge;
    uint64_t mLastReadAge = 0;
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_VULKANBUFFER_H
