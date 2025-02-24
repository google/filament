/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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
#include <vulkan/utility/vk_safe_struct.hpp>

vku::safe_VkRenderPassCreateInfo2 ConvertVkRenderPassCreateInfoToV2KHR(const VkRenderPassCreateInfo& create_info);

vku::safe_VkImageMemoryBarrier2 ConvertVkImageMemoryBarrierToV2(const VkImageMemoryBarrier& barrier,
                                                               VkPipelineStageFlags2 srcStageMask,
                                                               VkPipelineStageFlags2 dstStageMask);

// Converts array of VkSubmitInfo into array of VkSubmitInfo2.
// Constructor performs the conversion. The result is stored into submit_infos2.
struct SubmitInfoConverter {
    SubmitInfoConverter(const VkSubmitInfo* submit_infos, uint32_t count);

    // That's the conversion result
    std::vector<VkSubmitInfo2> submit_infos2;

    // Helper structures referenced by VkSubmitInfo2 objects
    std::vector<VkSemaphoreSubmitInfo> wait_infos;
    std::vector<VkCommandBufferSubmitInfo> cb_infos;
    std::vector<VkSemaphoreSubmitInfo> signal_infos;
};
