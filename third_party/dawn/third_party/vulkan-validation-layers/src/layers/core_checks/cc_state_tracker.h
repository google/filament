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
#pragma once
#include "cc_submit.h"
#include "state_tracker/state_tracker.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/queue_state.h"

namespace core {

// CommandBuffer is over 3 times larger than the next largest state object struct, but the majority of the state is only used in
// CoreChecks. This state object is used by everyone else (best practice, sync val, GPU-AV, etc). For this reason, we have
// CommandBuffer object only for core and keep only the most basic items in the parent class
class CommandBuffer : public vvl::CommandBuffer {
  public:
    CommandBuffer(CoreChecks& core, VkCommandBuffer handle, const VkCommandBufferAllocateInfo* pCreateInfo,
                  const vvl::CommandPool* cmd_pool);

    void RecordWaitEvents(vvl::Func command, uint32_t eventCount, const VkEvent* pEvents,
                          VkPipelineStageFlags2KHR src_stage_mask) override;
};

// Override Retire to validate submissions in the order defined by synchronization
class Queue : public vvl::Queue {
  public:
    Queue(vvl::Device& dev_data, VkQueue handle, uint32_t family_index, uint32_t queue_index, VkDeviceQueueCreateFlags flags,
          const VkQueueFamilyProperties& queue_family_properties, const vvl::Device& error_logger);

  private:
    void Retire(vvl::QueueSubmission&) override;

  private:
    QueueSubmissionValidator queue_submission_validator_;
};

}  // namespace core
