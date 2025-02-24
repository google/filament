/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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

namespace stateless {

bool Device::manual_PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory,
                                                  const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!pAllocateInfo) {
        return skip;
    }
    const Location allocate_info_loc = error_obj.location.dot(Field::pAllocateInfo);
    auto chained_prio_struct = vku::FindStructInPNextChain<VkMemoryPriorityAllocateInfoEXT>(pAllocateInfo->pNext);
    if (chained_prio_struct && (chained_prio_struct->priority < 0.0f || chained_prio_struct->priority > 1.0f)) {
        skip |= LogError("VUID-VkMemoryPriorityAllocateInfoEXT-priority-02602", device,
                         allocate_info_loc.pNext(Struct::VkMemoryPriorityAllocateInfoEXT, Field::priority), "is %f",
                         chained_prio_struct->priority);
    }

    auto flags_info = vku::FindStructInPNextChain<VkMemoryAllocateFlagsInfo>(pAllocateInfo->pNext);
    const VkMemoryAllocateFlags flags = flags_info ? flags_info->flags : 0;

    skip |= ValidateAllocateMemoryExternal(device, *pAllocateInfo, flags, allocate_info_loc);

    if (flags) {
        const Location flags_loc = allocate_info_loc.pNext(Struct::VkMemoryAllocateFlagsInfo, Field::flags);
        if ((flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT) && !enabled_features.bufferDeviceAddressCaptureReplay) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-flags-03330", device, flags_loc,
                             "has VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT set, but"
                             "bufferDeviceAddressCaptureReplay feature is not enabled.");
        }
        if ((flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT) && !enabled_features.bufferDeviceAddress) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-flags-03331", device, flags_loc,
                             "has VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT set, but bufferDeviceAddress feature is not enabled.");
        }
    }
    return skip;
}

bool Device::ValidateDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements &memory_requirements,
                                                   const Location &loc) const {
    bool skip = false;

    const auto &create_info = *(memory_requirements.pCreateInfo);
    if (vku::FindStructInPNextChain<VkImageSwapchainCreateInfoKHR>(create_info.pNext)) {
        skip |= LogError("VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06416", device, loc,
                         "pNext chain contains VkImageSwapchainCreateInfoKHR.");
    }
    if (vku::FindStructInPNextChain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(create_info.pNext)) {
        skip |= LogError("VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06776", device, loc,
                         "pNext chain contains VkImageDrmFormatModifierExplicitCreateInfoEXT.");
    }

    if (vkuFormatIsMultiplane(create_info.format) && (create_info.flags & VK_IMAGE_CREATE_DISJOINT_BIT) != 0) {
        if (memory_requirements.planeAspect == VK_IMAGE_ASPECT_NONE) {
            skip |= LogError("VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06417", device, loc.dot(Field::planeAspect),
                             "is VK_IMAGE_ASPECT_NONE with a multi-planar format and disjoint flag.");
        } else if ((create_info.tiling == VK_IMAGE_TILING_LINEAR || create_info.tiling == VK_IMAGE_TILING_OPTIMAL) &&
                   !IsOnlyOneValidPlaneAspect(create_info.format, memory_requirements.planeAspect)) {
            skip |= LogError("VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06419", device, loc.dot(Field::planeAspect),
                             "is %s but is invalid for %s.", string_VkImageAspectFlags(memory_requirements.planeAspect).c_str(),
                             string_VkFormat(create_info.format));
        }
    }
    const uint64_t external_format = GetExternalFormat(memory_requirements.pCreateInfo->pNext);
    if (external_format != 0) {
        skip |= LogError("VUID-VkDeviceImageMemoryRequirements-pNext-06996", device, loc.dot(Field::pCreateInfo),
                         "pNext chain contains VkExternalFormatANDROID with externalFormat %" PRIu64 ".", external_format);
    }

    return skip;
}

bool Device::manual_PreCallValidateGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo,
                                                                    VkMemoryRequirements2 *pMemoryRequirements,
                                                                    const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    skip |= ValidateDeviceImageMemoryRequirements(device, *pInfo, error_obj.location.dot(Field::pInfo));

    return skip;
}

bool Device::manual_PreCallValidateGetDeviceImageSparseMemoryRequirements(
    VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    skip |= ValidateDeviceImageMemoryRequirements(device, *pInfo, error_obj.location.dot(Field::pInfo));

    return skip;
}

bool Device::manual_PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo,
                                                   VkFence fence, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    for (uint32_t bind_info_i = 0; bind_info_i < bindInfoCount; ++bind_info_i) {
        const VkBindSparseInfo &bind_info = pBindInfo[bind_info_i];
        for (uint32_t image_bind_i = 0; image_bind_i < bind_info.imageBindCount; ++image_bind_i) {
            const VkSparseImageMemoryBindInfo &image_bind = bind_info.pImageBinds[image_bind_i];
            for (uint32_t bind_i = 0; bind_i < image_bind.bindCount; ++bind_i) {
                const VkSparseImageMemoryBind &bind = image_bind.pBinds[bind_i];
                if (bind.extent.width == 0) {
                    const LogObjectList objlist(queue, image_bind.image);
                    skip |= LogError("VUID-VkSparseImageMemoryBind-extent-09388", objlist,
                                     error_obj.location.dot(Field::pBindInfo, bind_info_i)
                                         .dot(Field::pImageBinds, image_bind_i)
                                         .dot(Field::pBinds, bind_i)
                                         .dot(Field::extent)
                                         .dot(Field::width),
                                     "is zero.");
                }

                if (bind.extent.height == 0) {
                    const LogObjectList objlist(queue, image_bind.image);
                    skip |= LogError("VUID-VkSparseImageMemoryBind-extent-09389", objlist,
                                     error_obj.location.dot(Field::pBindInfo, bind_info_i)
                                         .dot(Field::pImageBinds, image_bind_i)
                                         .dot(Field::pBinds, bind_i)
                                         .dot(Field::extent)
                                         .dot(Field::height),
                                     "is zero.");
                }

                if (bind.extent.depth == 0) {
                    const LogObjectList objlist(queue, image_bind.image);
                    skip |= LogError("VUID-VkSparseImageMemoryBind-extent-09390", objlist,
                                     error_obj.location.dot(Field::pBindInfo, bind_info_i)
                                         .dot(Field::pImageBinds, image_bind_i)
                                         .dot(Field::pBinds, bind_i)
                                         .dot(Field::extent)
                                         .dot(Field::depth),
                                     "is zero.");
                }
            }
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                                              const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (!IsBetweenInclusive(priority, 0.0F, 1.0F)) {
        skip |= LogError("VUID-vkSetDeviceMemoryPriorityEXT-priority-06258", device, error_obj.location.dot(Field::priority),
                         "is %f.", priority);
    }
    return skip;
}
}  // namespace stateless
