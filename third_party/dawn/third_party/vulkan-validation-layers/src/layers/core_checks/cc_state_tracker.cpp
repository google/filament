/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "cc_state_tracker.h"
#include "core_validation.h"

core::CommandBuffer::CommandBuffer(CoreChecks& core, VkCommandBuffer handle, const VkCommandBufferAllocateInfo* allocate_info,
                                   const vvl::CommandPool* pool)
    : vvl::CommandBuffer(core, handle, allocate_info, pool) {}

// Much of the data stored in vvl::CommandBuffer is only used by core validation, and is
// set up by Record calls in class CoreChecks. Because both the state tracker and
// core methods must lock vvl::CommandBuffer, it is possible for a Validate call to
// 'interrupt' a Record call and get only the state updated by whichever code
// locked and unlocked the CB first. This can only happen if the application
// is violating section 3.6 'Threading Behavior' of the specification, which
// requires that command buffers be externally synchronized. Still, we'd prefer
// not to crash if that happens. In most cases the core Record method is operating
// on separate data members from the state tracker. But in the case of vkCmdWaitEvents*,
// both methods operate on the same state in ways that could very easily crash if
// not done within the same lock guard. Overriding RecordWaitEvents() allows
// this to all happen completely while the state tracker is holding the lock.
// Eventually we'll probably want to move all of the core state into this derived
// class.
void core::CommandBuffer::RecordWaitEvents(vvl::Func command, uint32_t eventCount, const VkEvent* pEvents,
                                           VkPipelineStageFlags2KHR srcStageMask) {
    // vvl::CommandBuffer will add to the events vector.
    auto first_event_index = events.size();
    vvl::CommandBuffer::RecordWaitEvents(command, eventCount, pEvents, srcStageMask);
    auto event_added_count = events.size() - first_event_index;
    event_updates.emplace_back(
        [command, event_added_count, first_event_index, srcStageMask](
            vvl::CommandBuffer& cb_state, bool do_validate, EventMap& local_event_signal_info, VkQueue queue, const Location& loc) {
            if (!do_validate) return false;
            return CoreChecks::ValidateWaitEventsAtSubmit(command, cb_state, event_added_count, first_event_index, srcStageMask,
                                                          local_event_signal_info, queue, loc);
        });
}

std::shared_ptr<vvl::CommandBuffer> CoreChecks::CreateCmdBufferState(VkCommandBuffer handle,
                                                                     const VkCommandBufferAllocateInfo* allocate_info,
                                                                     const vvl::CommandPool* pool) {
    return std::static_pointer_cast<vvl::CommandBuffer>(std::make_shared<core::CommandBuffer>(*this, handle, allocate_info, pool));
}

core::Queue::Queue(vvl::Device& dev_data, VkQueue handle, uint32_t family_index, uint32_t queue_index,
                   VkDeviceQueueCreateFlags flags, const VkQueueFamilyProperties& queue_family_properties,
                   const vvl::Device& error_logger)
    : vvl::Queue(dev_data, handle, family_index, queue_index, flags, queue_family_properties),
      queue_submission_validator_(error_logger) {}

void core::Queue::Retire(vvl::QueueSubmission& submission) {
    // Call validation before parent call. Validation needs initial submission state (Retire updates state)
    queue_submission_validator_.Validate(submission);

    vvl::Queue::Retire(submission);
}

std::shared_ptr<vvl::Queue> CoreChecks::CreateQueue(VkQueue handle, uint32_t family_index, uint32_t queue_index,
                                                    VkDeviceQueueCreateFlags flags,
                                                    const VkQueueFamilyProperties& queue_family_properties) {
    return std::static_pointer_cast<vvl::Queue>(
        std::make_shared<core::Queue>(*this, handle, family_index, queue_index, flags, queue_family_properties, *this));
}
