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

bool BestPractices::PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer,
                                                const ErrorObject& error_obj) const {
    bool skip = false;

    if ((pCreateInfo->queueFamilyIndexCount > 1) && (pCreateInfo->sharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
        skip |= LogWarning("BestPractices-vkCreateBuffer-sharing-mode-exclusive", device,
                           error_obj.location.dot(Field::pCreateInfo).dot(Field::sharingMode),
                           "is VK_SHARING_MODE_EXCLUSIVE while specifying multiple queues (queueFamilyIndexCount of %" PRIu32 ").",
                           pCreateInfo->queueFamilyIndexCount);
    }

    return skip;
}
