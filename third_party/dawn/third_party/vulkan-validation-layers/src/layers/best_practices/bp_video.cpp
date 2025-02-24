/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
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

#include "best_practices/best_practices_validation.h"
#include "best_practices/bp_state.h"

bool BestPractices::PreCallValidateGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                                        uint32_t* pMemoryRequirementsCount,
                                                                        VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    if (auto vs_state = Get<vvl::VideoSession>(videoSession)) {
        if (pMemoryRequirements != nullptr && !vs_state->memory_binding_count_queried) {
            skip |= LogWarning("BestPractices-vkGetVideoSessionMemoryRequirementsKHR-count-not-retrieved", videoSession,
                               error_obj.location,
                               "querying list of memory requirements of %s "
                               "but the number of memory requirements has not been queried before by calling this "
                               "command with pMemoryRequirements set to NULL.",
                               FormatHandle(videoSession).c_str());
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                             uint32_t bindSessionMemoryInfoCount,
                                                             const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                             const ErrorObject& error_obj) const {
    bool skip = false;

    if (auto vs_state = Get<vvl::VideoSession>(videoSession)) {
        if (!vs_state->memory_binding_count_queried) {
            skip |= LogWarning("BestPractices-vkBindVideoSessionMemoryKHR-requirements-count-not-retrieved", videoSession,
                               error_obj.location,
                               "binding memory to %s but "
                               "vkGetVideoSessionMemoryRequirementsKHR() has not been called to retrieve the "
                               "number of memory requirements for the video session.",
                               FormatHandle(videoSession).c_str());
        } else if (vs_state->memory_bindings_queried < vs_state->GetMemoryBindingCount()) {
            skip |= LogWarning("BestPractices-vkBindVideoSessionMemoryKHR-requirements-not-all-retrieved", videoSession,
                               error_obj.location,
                               "binding memory to %s but "
                               "not all memory requirements for the video session have been queried using "
                               "vkGetVideoSessionMemoryRequirementsKHR().",
                               FormatHandle(videoSession).c_str());
        }
    }

    return skip;
}
