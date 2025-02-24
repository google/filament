/* Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "state_tracker/semaphore_state.h"

class CoreChecks;

// Tracks semaphore state changes during the validation phase of QueueSubmit commands.
// Semaphore state object (vvl::Semaphore) is updated later in the record phase.
struct SemaphoreSubmitState {
    const CoreChecks &core;
    const VkQueue queue;
    const VkQueueFlags queue_flags;

    // Track binary semaphore payload
    vvl::unordered_map<VkSemaphore, bool> binary_signaling_state;

    // Track semaphores that were temporary external and become internal after wait operation.
    vvl::unordered_set<VkSemaphore> internal_semaphores;

    // Track timeline operations
    vvl::unordered_map<VkSemaphore, uint64_t> timeline_signals;
    vvl::unordered_map<VkSemaphore, uint64_t> timeline_waits;

    SemaphoreSubmitState(const CoreChecks &core_, VkQueue q_, VkQueueFlags queue_flags_)
        : core(core_), queue(q_), queue_flags(queue_flags_) {}

    bool CanWaitBinary(const vvl::Semaphore &semaphore_state) const;
    bool CanSignalBinary(const vvl::Semaphore &semaphore_state, VkQueue &other_queue, vvl::Func &other_acquire_command) const;

    bool CheckSemaphoreValue(const vvl::Semaphore &semaphore_state, std::string &where, uint64_t &bad_value,
                             std::function<bool(const vvl::Semaphore::OpType, uint64_t, bool is_pending)> compare_func);

    VkQueue AnotherQueueWaits(const vvl::Semaphore &semaphore_state) const;

    bool ValidateBinaryWait(const Location &loc, VkQueue queue, const vvl::Semaphore &semaphore_state);
    bool ValidateWaitSemaphore(const Location &wait_semaphore_loc, const vvl::Semaphore &semaphore_state, uint64_t value);
    bool ValidateSignalSemaphore(const Location &signal_semaphore_loc, const vvl::Semaphore &semaphore_state, uint64_t value);
};
