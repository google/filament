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

#include <utils/Mutex.h> // NOLINT(*-include-cleaner)
#include <utils/Condition.h> // NOLINT(*-include-cleaner)

#include <cstdint>
#include <memory>

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
    ~WebGPUFence() noexcept;
    [[nodiscard]] FenceStatus getStatus() const;
    [[nodiscard]] FenceStatus wait(uint64_t timeout);

    void addMarkerToQueueState(wgpu::Queue const&);

private:
    struct State {
        utils::Mutex lock; // NOLINT(*-include-cleaner)
        utils::Condition cond; // NOLINT(*-include-cleaner)
        FenceStatus status{ FenceStatus::TIMEOUT_EXPIRED };
    };
    // we need a shared_ptr because the Fence could be destroyed while we're waiting
    std::shared_ptr<State> state{ std::make_shared<State>() };
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUFENCE_H
