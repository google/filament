/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
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

#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"

namespace stateless {

bool Device::manual_PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer,
                                                const stateless::Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    skip |= context.ValidateNotZero(pCreateInfo->size == 0, "VUID-VkBufferCreateInfo-size-00912", create_info_loc.dot(Field::size));

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT) {
        // If sharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
        if (pCreateInfo->queueFamilyIndexCount <= 1) {
            skip |= LogError("VUID-VkBufferCreateInfo-sharingMode-00914", device, create_info_loc.dot(Field::sharingMode),
                             "is VK_SHARING_MODE_CONCURRENT, but queueFamilyIndexCount is %" PRIu32 ".",
                             pCreateInfo->queueFamilyIndexCount);
        }

        // If sharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
        // queueFamilyIndexCount uint32_t values
        if (pCreateInfo->pQueueFamilyIndices == nullptr) {
            skip |= LogError("VUID-VkBufferCreateInfo-sharingMode-00913", device, create_info_loc.dot(Field::sharingMode),
                             "is VK_SHARING_MODE_CONCURRENT, but pQueueFamilyIndices is NULL.");
        }
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) && (!enabled_features.sparseBinding)) {
        skip |= LogError("VUID-VkBufferCreateInfo-flags-00915", device, create_info_loc.dot(Field::flags),
                         "includes VK_BUFFER_CREATE_SPARSE_BINDING_BIT, but the sparseBinding feature is not enabled.");
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT) && (!enabled_features.sparseResidencyBuffer)) {
        skip |= LogError("VUID-VkBufferCreateInfo-flags-00916", device, create_info_loc.dot(Field::flags),
                         "includes VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT, but the sparseResidencyBuffer feature is not enabled.");
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_ALIASED_BIT) && (!enabled_features.sparseResidencyAliased)) {
        skip |= LogError("VUID-VkBufferCreateInfo-flags-00917", device, create_info_loc.dot(Field::flags),
                         "includes VK_BUFFER_CREATE_SPARSE_ALIASED_BIT, but the sparseResidencyAliased feature is not enabled.");
    };

    // If flags contains VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT or VK_BUFFER_CREATE_SPARSE_ALIASED_BIT, it must also contain
    // VK_BUFFER_CREATE_SPARSE_BINDING_BIT
    if (((pCreateInfo->flags & (VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT)) != 0) &&
        ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) != VK_BUFFER_CREATE_SPARSE_BINDING_BIT)) {
        skip |= LogError("VUID-VkBufferCreateInfo-flags-00918", device, create_info_loc.dot(Field::flags), "is %s.",
                         string_VkBufferCreateFlags(pCreateInfo->flags).c_str());
    }

    if (!vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfo>(pCreateInfo->pNext)) {
        skip |= context.ValidateFlags(create_info_loc.dot(Field::usage), vvl::FlagBitmask::VkBufferUsageFlagBits,
                                      AllVkBufferUsageFlagBits, pCreateInfo->usage, kRequiredFlags,
                                      "VUID-VkBufferCreateInfo-None-09499", "VUID-VkBufferCreateInfo-None-09500");
    }

    if (pCreateInfo->flags & VK_BUFFER_CREATE_PROTECTED_BIT) {
        const VkBufferUsageFlags invalid =
            VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT | VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT |
            VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
            VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
            VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT |
            VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT;
        if (pCreateInfo->usage & invalid) {
            skip |= LogError("VUID-VkBufferCreateInfo-flags-09641", device, create_info_loc.dot(Field::flags),
                             "includes VK_BUFFER_CREATE_PROTECTED_BIT, but the usage is %s.",
                             string_VkBufferUsageFlags(pCreateInfo->usage).c_str());
        }
    };

    return skip;
}

bool Device::manual_PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkBufferView *pBufferView,
                                                    const Context &context) const {
    bool skip = false;
#ifdef VK_USE_PLATFORM_METAL_EXT
    skip |= ExportMetalObjectsPNextUtil(VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT,
                                        "VUID-VkBufferViewCreateInfo-pNext-06782", context.error_obj.location,
                                        "VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT", pCreateInfo->pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT
    return skip;
}
}  // namespace stateless
