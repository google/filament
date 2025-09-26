/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUFENCE_H
#define TNT_FILAMENT_BACKEND_WEBGPUFENCE_H

#include "DriverBase.h"
#include <backend/DriverEnums.h>

#include <condition_variable>
#include <mutex>

namespace wgpu {
class Queue;
} // namespace wgpu

namespace filament::backend {

/**
  * A WebGPU-specific implementation of the HwFence.
  * This class is used to synchronize the CPU and GPU. It allows the CPU to know when the GPU has
  * finished a set of commands.
  */
class WebGPUFence final : public HwFence {
public:
    [[nodiscard]] FenceStatus getStatus();
    [[nodiscard]] FenceStatus wait(uint64_t timeout);

    void addMarkerToQueueState(wgpu::Queue const&);

private:
    std::mutex mLock;
    std::condition_variable mCondition;
    FenceStatus mStatus{ FenceStatus::TIMEOUT_EXPIRED };
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUFENCE_H
