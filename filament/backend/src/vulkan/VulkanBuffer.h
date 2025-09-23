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

#include "VulkanMemory.h"
#include "memory/Resource.h"

#include <functional>

namespace filament::backend {

class VulkanBuffer : public fvkmemory::Resource {
public:
    // Because we need to recycle the unused `VulkanGpuBuffer`, we allow for a callback that the
    // "Pool" can use to acquire the buffer back.
    using OnRecycle = std::function<void(VulkanGpuBuffer const*)>;

    VulkanBuffer(VulkanGpuBuffer const* gpuBuffer, OnRecycle&& onRecycleFn)
        : mGpuBuffer(gpuBuffer),
          mOnRecycleFn(onRecycleFn) {}

    ~VulkanBuffer() {
        if (mOnRecycleFn) {
            mOnRecycleFn(mGpuBuffer);
        }
    }

    VulkanGpuBuffer const* getGpuBuffer() const { return mGpuBuffer; }

private:
    VulkanGpuBuffer const* mGpuBuffer;
    OnRecycle mOnRecycleFn;
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_VULKANBUFFER_H
