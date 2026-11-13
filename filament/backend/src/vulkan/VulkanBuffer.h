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

#include <utils/debug.h>

#include <cstdint>
#include <functional>
#include <utility>

namespace filament::backend {

class VulkanBuffer : public fvkmemory::Resource {
public:
    // Because we need to recycle the unused `VulkanGpuBuffer`, we allow for a callback that the
    // "Pool" can use to acquire the buffer back.
    using OnRecycle = std::function<void(VulkanGpuBuffer const*)>;

    VulkanBuffer(VulkanGpuBuffer const* gpuBuffer, OnRecycle&& onRecycleFn)
        : mGpuBuffer(gpuBuffer),
          mOnRecycleFn(std::move(onRecycleFn)) {}

    ~VulkanBuffer() {
        if (mOnRecycleFn) {
            mOnRecycleFn(mGpuBuffer);
        }
    }

    VulkanGpuBuffer const* getGpuBuffer() const { return mGpuBuffer; }

    // Records that the command buffer of age `age`, which must still be recording, reads from or
    // writes into this buffer (e.g. a draw that reads it, or a vkCmdCopyBuffer into it).
    void markInFlight(fvkutils::CommandBufferAgeCounter const& ageCounter, uint64_t age) {
        ageCounter.markInFlight(mAges, age);
    }

    // Returns the ages of the command buffers that read from or wrote into this buffer. Some of
    // them may have completed since; use CommandBufferAgeCounter::isInFlight() to
    // find the ones that are still in flight.
    fvkutils::AgeSet const& getAges() const {
        return mAges;
    }

    bool isMemcopyable() const {
        assert_invariant(mGpuBuffer);
        return mGpuBuffer->allocationInfo.pMappedData != nullptr;
    }

private:
    VulkanGpuBuffer const* mGpuBuffer;
    OnRecycle mOnRecycleFn;

    fvkutils::AgeSet mAges;
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_VULKANBUFFER_H
