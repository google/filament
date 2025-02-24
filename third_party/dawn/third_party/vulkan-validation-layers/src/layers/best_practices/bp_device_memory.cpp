/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include "best_practices/best_practices_validation.h"
#include "best_practices/bp_state.h"
#include "state_tracker/buffer_state.h"

void BestPractices::PreCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                                const RecordObject& record_obj) {
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        WriteLockGuard guard{memory_free_events_lock_};

        // Release old allocations to avoid overpopulating the container
        const auto now = std::chrono::high_resolution_clock::now();
        const auto last_old = std::find_if(
            memory_free_events_.rbegin(), memory_free_events_.rend(),
            [now](const MemoryFreeEvent& event) { return now - event.time > kAllocateMemoryReuseTimeThresholdNVIDIA; });
        memory_free_events_.erase(memory_free_events_.begin(), last_old.base());
    }
}

bool BestPractices::PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                                  const ErrorObject& error_obj) const {
    bool skip = false;

    const size_t object_count = Count<vvl::DeviceMemory>() + 1;
    if (object_count > kMemoryObjectWarningLimit) {
        skip |= LogPerformanceWarning("BestPractices-vkAllocateMemory-too-many-objects", device, error_obj.location,
                                      "This app has %zu memory objects, recommended max is %" PRIu32 ".", object_count,
                                      kMemoryObjectWarningLimit);
    }

    if (pAllocateInfo->allocationSize < kMinDeviceAllocationSize) {
        skip |= LogPerformanceWarning("BestPractices-vkAllocateMemory-small-allocation", device,
                                      error_obj.location.dot(Field::pAllocateInfo).dot(Field::allocationSize),
                                      "is %" PRIu64
                                      ". This is a very small allocation (current "
                                      "threshold is %" PRIu64
                                      " bytes). "
                                      "You should make large allocations and sub-allocate from one large VkDeviceMemory.",
                                      pAllocateInfo->allocationSize, kMinDeviceAllocationSize);
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        if (!IsExtEnabled(extensions.vk_ext_pageable_device_local_memory) &&
            !vku::FindStructInPNextChain<VkMemoryPriorityAllocateInfoEXT>(pAllocateInfo->pNext)) {
            skip |= LogPerformanceWarning(
                "BestPractices-NVIDIA-AllocateMemory-SetPriority", device, error_obj.location,
                "%s Use VkMemoryPriorityAllocateInfoEXT to provide the operating system information on the allocations that "
                "should stay in video memory and which should be demoted first when video memory is limited. "
                "The highest priority should be given to GPU-written resources like color attachments, depth attachments, "
                "storage images, and buffers written from the GPU.",
                VendorSpecificTag(kBPVendorNVIDIA));
        }

        {
            // Size in bytes for an allocation to be considered "compatible"
            static constexpr VkDeviceSize size_threshold = VkDeviceSize{1} << 20;

            ReadLockGuard guard{memory_free_events_lock_};

            const auto now = std::chrono::high_resolution_clock::now();
            const VkDeviceSize alloc_size = pAllocateInfo->allocationSize;
            const uint32_t memory_type_index = pAllocateInfo->memoryTypeIndex;
            const auto latest_event =
                std::find_if(memory_free_events_.rbegin(), memory_free_events_.rend(), [&](const MemoryFreeEvent& event) {
                    return (memory_type_index == event.memory_type_index) && (alloc_size <= event.allocation_size) &&
                           (alloc_size - event.allocation_size <= size_threshold) &&
                           (now - event.time < kAllocateMemoryReuseTimeThresholdNVIDIA);
                });

            if (latest_event != memory_free_events_.rend()) {
                const auto time_delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - latest_event->time);
                if (time_delta < std::chrono::milliseconds{5}) {
                    skip |= LogPerformanceWarning(
                        "BestPractices-NVIDIA-AllocateMemory-ReuseAllocations", device, error_obj.location,
                        "%s Reuse memory allocations instead of releasing and reallocating. A memory allocation "
                        "has just been released, and it could have been reused in place of this allocation.",
                        VendorSpecificTag(kBPVendorNVIDIA));
                } else {
                    const uint32_t seconds = static_cast<uint32_t>(time_delta.count() / 1000);
                    const uint32_t milliseconds = static_cast<uint32_t>(time_delta.count() % 1000);

                    skip |= LogPerformanceWarning(
                        "BestPractices-NVIDIA-AllocateMemory-ReuseAllocations", device, error_obj.location,
                        "%s Reuse memory allocations instead of releasing and reallocating. A memory allocation has been released "
                        "%" PRIu32 ".%03" PRIu32 " seconds ago, and it could have been reused in place of this allocation.",
                        VendorSpecificTag(kBPVendorNVIDIA), seconds, milliseconds);
                }
            }
        }
    }

    // TODO: Insert get check for GetPhysicalDeviceMemoryProperties once the state is tracked in the StateTracker

    return skip;
}

void BestPractices::PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {
    if (memory != VK_NULL_HANDLE && VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto mem_info = Get<vvl::DeviceMemory>(memory);
        ASSERT_AND_RETURN(mem_info);

        // Exclude memory free events on dedicated allocations, or imported/exported allocations.
        if (!mem_info->IsDedicatedBuffer() && !mem_info->IsDedicatedImage() && !mem_info->IsExport() && !mem_info->IsImport()) {
            MemoryFreeEvent event;
            event.time = std::chrono::high_resolution_clock::now();
            event.memory_type_index = mem_info->allocate_info.memoryTypeIndex;
            event.allocation_size = mem_info->allocate_info.allocationSize;

            WriteLockGuard guard{memory_free_events_lock_};
            memory_free_events_.push_back(event);
        }
    }

    BaseClass::PreCallRecordFreeMemory(device, memory, pAllocator, record_obj);
}

bool BestPractices::PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                                              const ErrorObject& error_obj) const {
    bool skip = false;
    if (memory == VK_NULL_HANDLE) return skip;

    auto mem_info = Get<vvl::DeviceMemory>(memory);
    ASSERT_AND_RETURN_SKIP(mem_info);

    for (const auto& item : mem_info->ObjectBindings()) {
        const auto& obj = item.first;
        const LogObjectList objlist(device, obj, mem_info->Handle());
        skip |= LogWarning("BestPractices", objlist, error_obj.location, "VK Object %s still has a reference to mem obj %s.",
                           FormatHandle(obj).c_str(), FormatHandle(mem_info->Handle()).c_str());
    }

    return skip;
}

bool BestPractices::ValidateBindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, const Location& loc) const {
    bool skip = false;
    auto buffer_state = Get<vvl::Buffer>(buffer);
    auto memory_state = Get<vvl::DeviceMemory>(memory);
    ASSERT_AND_RETURN_SKIP(memory_state && buffer_state);

    if (memory_state->allocate_info.allocationSize == buffer_state->create_info.size &&
        memory_state->allocate_info.allocationSize < kMinDedicatedAllocationSize) {
        skip |= LogPerformanceWarning("BestPractices-vkBindBufferMemory-small-dedicated-allocation", device, loc,
                                      "Trying to bind %s to a memory block which is fully consumed by the buffer. "
                                      "The required size of the allocation is %" PRIu64
                                      ", but smaller buffers like this should be sub-allocated from "
                                      "larger memory blocks. (Current threshold is %" PRIu64 " bytes.)",
                                      FormatHandle(buffer).c_str(), memory_state->allocate_info.allocationSize,
                                      kMinDedicatedAllocationSize);
    }

    skip |= ValidateBindMemory(device, memory, loc);

    return skip;
}

bool BestPractices::PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                                    VkDeviceSize memoryOffset, const ErrorObject& error_obj) const {
    bool skip = false;

    skip |= ValidateBindBufferMemory(buffer, memory, error_obj.location);

    return skip;
}

bool BestPractices::PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                     const VkBindBufferMemoryInfo* pBindInfos, const ErrorObject& error_obj) const {
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        skip |= ValidateBindBufferMemory(pBindInfos[i].buffer, pBindInfos[i].memory, error_obj.location.dot(Field::pBindInfos, i));
    }

    return skip;
}

bool BestPractices::PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                        const VkBindBufferMemoryInfo* pBindInfos,
                                                        const ErrorObject& error_obj) const {
    return PreCallValidateBindBufferMemory2(device, bindInfoCount, pBindInfos, error_obj);
}

bool BestPractices::ValidateBindImageMemory(VkImage image, VkDeviceMemory memory, const Location& loc) const {
    bool skip = false;
    auto image_state = Get<vvl::Image>(image);
    auto memory_state = Get<vvl::DeviceMemory>(memory);
    ASSERT_AND_RETURN_SKIP(memory_state && image_state);

    if (memory_state->allocate_info.allocationSize == image_state->requirements[0].size &&
        memory_state->allocate_info.allocationSize < kMinDedicatedAllocationSize) {
        skip |= LogPerformanceWarning("BestPractices-vkBindImageMemory-small-dedicated-allocation", device, loc,
                                      "Trying to bind %s to a memory block which is fully consumed by the image. "
                                      "The required size of the allocation is %" PRIu64
                                      ", but smaller images like this should be sub-allocated from "
                                      "larger memory blocks. (Current threshold is %" PRIu64 " bytes.)",
                                      FormatHandle(image).c_str(), memory_state->allocate_info.allocationSize,
                                      kMinDedicatedAllocationSize);
    }

    // If we're binding memory to a image which was created as TRANSIENT and the image supports LAZY allocation,
    // make sure this type is actually used.
    // This warning will only trigger if this layer is run on a platform that supports LAZILY_ALLOCATED_BIT
    // (i.e.most tile - based renderers)
    if (image_state->create_info.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
        bool supports_lazy = false;
        uint32_t suggested_type = 0;

        for (uint32_t i = 0; i < phys_dev_mem_props.memoryTypeCount; i++) {
            if ((1u << i) & image_state->requirements[0].memoryTypeBits) {
                if (phys_dev_mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
                    supports_lazy = true;
                    suggested_type = i;
                    break;
                }
            }
        }

        uint32_t allocated_properties = phys_dev_mem_props.memoryTypes[memory_state->allocate_info.memoryTypeIndex].propertyFlags;

        if (supports_lazy && (allocated_properties & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) == 0) {
            skip |= LogPerformanceWarning(
                "BestPractices-vkBindImageMemory-non-lazy-transient-image", device, loc,
                "ttempting to bind memory type %u to VkImage which was created with TRANSIENT_ATTACHMENT_BIT,"
                "but this memory type is not LAZILY_ALLOCATED_BIT. You should use memory type %u here instead to save "
                "%" PRIu64 " bytes of physical memory.",
                memory_state->allocate_info.memoryTypeIndex, suggested_type, image_state->requirements[0].size);
        }
    }

    skip |= ValidateBindMemory(device, memory, loc);

    return skip;
}

bool BestPractices::PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                                   const ErrorObject& error_obj) const {
    bool skip = false;

    skip |= ValidateBindImageMemory(image, memory, error_obj.location);

    return skip;
}

bool BestPractices::PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindImageMemoryInfo* pBindInfos, const ErrorObject& error_obj) const {
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        if (!vku::FindStructInPNextChain<VkBindImageMemorySwapchainInfoKHR>(pBindInfos[i].pNext)) {
            skip |=
                ValidateBindImageMemory(pBindInfos[i].image, pBindInfos[i].memory, error_obj.location.dot(Field::pBindInfos, i));
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                       const VkBindImageMemoryInfo* pBindInfos,
                                                       const ErrorObject& error_obj) const {
    return PreCallValidateBindImageMemory2(device, bindInfoCount, pBindInfos, error_obj);
}

void BestPractices::PostCallRecordSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                                             const RecordObject& record_obj) {
    auto mem_info = std::static_pointer_cast<bp_state::DeviceMemory>(Get<vvl::DeviceMemory>(memory));
    mem_info->dynamic_priority.emplace(priority);
}

bool BestPractices::ValidateBindMemory(VkDevice device, VkDeviceMemory memory, const Location& loc) const {
    bool skip = false;

    if (VendorCheckEnabled(kBPVendorNVIDIA) && IsExtEnabled(extensions.vk_ext_pageable_device_local_memory)) {
        auto mem_info = std::static_pointer_cast<const bp_state::DeviceMemory>(Get<vvl::DeviceMemory>(memory));
        bool has_static_priority = vku::FindStructInPNextChain<VkMemoryPriorityAllocateInfoEXT>(mem_info->allocate_info.pNext);
        if (!mem_info->dynamic_priority && !has_static_priority) {
            skip |=
                LogPerformanceWarning("BestPractices-NVIDIA-BindMemory-NoPriority", device, loc,
                                      "%s Use vkSetDeviceMemoryPriorityEXT to provide the OS with information on which allocations "
                                      "should stay in memory and which should be demoted first when video memory is limited. The "
                                      "highest priority should be given to GPU-written resources like color attachments, depth "
                                      "attachments, storage images, and buffers written from the GPU.",
                                      VendorSpecificTag(kBPVendorNVIDIA));
        }
    }

    return skip;
}

std::shared_ptr<vvl::DeviceMemory> BestPractices::CreateDeviceMemoryState(
    VkDeviceMemory handle, const VkMemoryAllocateInfo* allocate_info, uint64_t fake_address, const VkMemoryType& memory_type,
    const VkMemoryHeap& memory_heap, std::optional<vvl::DedicatedBinding>&& dedicated_binding, uint32_t physical_device_count) {
    return std::static_pointer_cast<vvl::DeviceMemory>(std::make_shared<bp_state::DeviceMemory>(
        handle, allocate_info, fake_address, memory_type, memory_heap, std::move(dedicated_binding), physical_device_count));
}

void BestPractices::ManualPostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                          const VkBindBufferMemoryInfo* pBindInfos,
                                                          const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS && bindInfoCount > 1) {
        bool found_status = false;
        for (uint32_t i = 0; i < bindInfoCount; i++) {
            if (auto* bind_memory_status = vku::FindStructInPNextChain<VkBindMemoryStatus>(pBindInfos[i].pNext)) {
                found_status = true;
                if (bind_memory_status->pResult && *bind_memory_status->pResult != VK_SUCCESS) {
                    LogWarning("BestPractices-Partial-Bound-Buffer-Status", device,
                               record_obj.location.dot(Field::pBindInfos, i).pNext(vvl::Struct::VkBindMemoryStatus, Field::pResult),
                               "was %s", string_VkResult(*bind_memory_status->pResult));
                }
            }
        }

        if (!found_status) {
            // Details of check found in https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5527
            LogWarning("BestPractices-Partial-Bound-Buffer", device, record_obj.location,
                       "all buffer are now in an indeterminate state because the call failed to return VK_SUCCESS. The best action "
                       "to take "
                       "is to destroy the buffers instead of trying to rebind");
        }
    }
}

void BestPractices::ManualPostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                                         const VkBindImageMemoryInfo* pBindInfos, const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS && bindInfoCount > 1) {
        bool found_status = false;
        for (uint32_t i = 0; i < bindInfoCount; i++) {
            if (auto* bind_memory_status = vku::FindStructInPNextChain<VkBindMemoryStatus>(pBindInfos[i].pNext)) {
                found_status = true;
                if (bind_memory_status->pResult && *bind_memory_status->pResult != VK_SUCCESS) {
                    LogWarning("BestPractices-Partial-Bound-Image-Status", device,
                               record_obj.location.dot(Field::pBindInfos, i).pNext(vvl::Struct::VkBindMemoryStatus, Field::pResult),
                               "was %s", string_VkResult(*bind_memory_status->pResult));
                }
            }
        }

        if (!found_status) {
            // Details of check found in https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5527
            LogWarning("BestPractices-Partial-Bound-Image", device, record_obj.location,
                       "all image are now in an indeterminate state because the call failed to return VK_SUCCESS. The best action "
                       "to take is "
                       "to destroy the images instead of trying to rebind");
        }
    }
}
