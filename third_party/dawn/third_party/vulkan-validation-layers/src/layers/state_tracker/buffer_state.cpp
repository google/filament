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
#include "state_tracker/buffer_state.h"
#include "generated/dispatch_functions.h"
#include "state_tracker/state_tracker.h"

static VkExternalMemoryHandleTypeFlags GetExternalHandleTypes(const VkBufferCreateInfo *create_info) {
    const auto *external_memory_info = vku::FindStructInPNextChain<VkExternalMemoryBufferCreateInfo>(create_info->pNext);
    return external_memory_info ? external_memory_info->handleTypes : 0;
}

static VkMemoryRequirements GetMemoryRequirements(vvl::Device &dev_data, VkBuffer buffer) {
    VkMemoryRequirements result{};
    DispatchGetBufferMemoryRequirements(dev_data.device, buffer, &result);
    return result;
}

static VkBufferUsageFlags2KHR GetBufferUsageFlags(const VkBufferCreateInfo &create_info) {
    const auto *usage_flags2 = vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfo>(create_info.pNext);
    return usage_flags2 ? usage_flags2->usage : create_info.usage;
}

#ifdef VK_USE_PLATFORM_METAL_EXT
static bool GetMetalExport(const VkBufferViewCreateInfo *info) {
    bool retval = false;
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(info->pNext);
    while (export_metal_object_info) {
        if (export_metal_object_info->exportObjectType == VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT) {
            retval = true;
            break;
        }
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    return retval;
}
#endif

namespace vvl {

// TODO https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9384
// Fix GCC 13 issues with how we emplace BindableSparseMemoryTracker
#if defined(__GNUC__) && (__GNUC__ > 12)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

Buffer::Buffer(Device &dev_data, VkBuffer handle, const VkBufferCreateInfo *pCreateInfo)
    : Bindable(handle, kVulkanObjectTypeBuffer, (pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) != 0,
               (pCreateInfo->flags & VK_BUFFER_CREATE_PROTECTED_BIT) == 0, GetExternalHandleTypes(pCreateInfo)),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      requirements(GetMemoryRequirements(dev_data, handle)),
      usage(GetBufferUsageFlags(create_info)),
      supported_video_profiles(dev_data.video_profile_cache_.Get(
          dev_data.physical_device, vku::FindStructInPNextChain<VkVideoProfileListInfoKHR>(pCreateInfo->pNext))) {
    if (pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) {
        tracker_.emplace<BindableSparseMemoryTracker>(&requirements,
                                                      (pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT) != 0);
        SetMemoryTracker(&std::get<BindableSparseMemoryTracker>(tracker_));
    } else {
        tracker_.emplace<BindableLinearMemoryTracker>(&requirements);
        SetMemoryTracker(&std::get<BindableLinearMemoryTracker>(tracker_));
    }
}

#if defined(__GNUC__) && (__GNUC__ > 12)
#pragma GCC diagnostic pop
#endif

bool Buffer::CompareCreateInfo(const Buffer &other) const {
    bool valid_queue_family = true;
    if (create_info.sharingMode == VK_SHARING_MODE_CONCURRENT) {
        if (create_info.queueFamilyIndexCount != other.create_info.queueFamilyIndexCount) {
            valid_queue_family = false;
        } else {
            for (uint32_t i = 0; i < create_info.queueFamilyIndexCount; i++) {
                if (create_info.pQueueFamilyIndices[i] != other.create_info.pQueueFamilyIndices[i]) {
                    valid_queue_family = false;
                    break;
                }
            }
        }
    }

    // There are limitations what actually needs to be compared, so for simplicity (until found otherwise needed), we only need to
    // check the ExternalHandleType and not other pNext chains
    const bool valid_external = GetExternalHandleTypes(&create_info) == GetExternalHandleTypes(&other.create_info);

    return (create_info.flags == other.create_info.flags) && (create_info.size == other.create_info.size) &&
           (usage == other.usage) && (create_info.sharingMode == other.create_info.sharingMode) && valid_external &&
           valid_queue_family;
}

BufferView::BufferView(const std::shared_ptr<vvl::Buffer> &bf, VkBufferView handle, const VkBufferViewCreateInfo *pCreateInfo,
                       VkFormatFeatureFlags2KHR format_features)
    : StateObject(handle, kVulkanObjectTypeBufferView),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      buffer_state(bf),
#ifdef VK_USE_PLATFORM_METAL_EXT
      metal_bufferview_export(GetMetalExport(pCreateInfo)),
#endif
      buffer_format_features(format_features) {
}

}  // namespace vvl
