/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
#include "state_tracker/device_memory_state.h"

#include <algorithm>

using BufferRange = vvl::BindableMemoryTracker::BufferRange;
using MemoryRange = vvl::BindableMemoryTracker::MemoryRange;
using BoundMemoryRange = vvl::BindableMemoryTracker::BoundMemoryRange;
using BoundRanges = vvl::BindableLinearMemoryTracker::BoundRanges;
using DeviceMemoryState = vvl::BindableMemoryTracker::DeviceMemoryState;

// It is allowed to export memory into the handles of different types,
// that's why we use set of flags (VkExternalMemoryHandleTypeFlags)
static VkExternalMemoryHandleTypeFlags GetExportHandleTypes(const VkMemoryAllocateInfo *p_alloc_info) {
    auto export_info = vku::FindStructInPNextChain<VkExportMemoryAllocateInfo>(p_alloc_info->pNext);
    return export_info ? export_info->handleTypes : 0;
}

// Import works with a single handle type, that's why VkExternalMemoryHandleTypeFlagBits type is used.
// Since FlagBits-type cannot have a value of 0, we use std::optional to indicate the presense of an import operation.
static std::optional<VkExternalMemoryHandleTypeFlagBits> GetImportHandleType(const VkMemoryAllocateInfo *p_alloc_info) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    auto win32_import = vku::FindStructInPNextChain<VkImportMemoryWin32HandleInfoKHR>(p_alloc_info->pNext);
    if (win32_import) {
        return win32_import->handleType;
    }
#endif
    auto fd_import = vku::FindStructInPNextChain<VkImportMemoryFdInfoKHR>(p_alloc_info->pNext);
    if (fd_import) {
        return fd_import->handleType;
    }
    auto host_pointer_import = vku::FindStructInPNextChain<VkImportMemoryHostPointerInfoEXT>(p_alloc_info->pNext);
    if (host_pointer_import) {
        return host_pointer_import->handleType;
    }
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    // AHB Import doesn't have handle in the pNext chain
    // It should be assumed that all imported AHB can only have the same, single handleType
    auto ahb_import = vku::FindStructInPNextChain<VkImportAndroidHardwareBufferInfoANDROID>(p_alloc_info->pNext);
    if ((ahb_import) && (ahb_import->buffer != nullptr)) {
        return VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
    return std::nullopt;
}

static bool IsMultiInstance(const VkMemoryAllocateInfo *p_alloc_info, const VkMemoryHeap &memory_heap,
                            uint32_t physical_device_count) {
    auto alloc_flags = vku::FindStructInPNextChain<VkMemoryAllocateFlagsInfo>(p_alloc_info->pNext);
    if (alloc_flags && (alloc_flags->flags & VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT)) {
        auto dev_mask = alloc_flags->deviceMask;
        return ((dev_mask != 0) && (dev_mask & (dev_mask - 1))) != 0;
    } else if (memory_heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) {
        return physical_device_count > 1;
    }
    return false;
}

#ifdef VK_USE_PLATFORM_METAL_EXT
static bool GetMetalExport(const VkMemoryAllocateInfo *info) {
    bool retval = false;
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(info->pNext);
    while (export_metal_object_info) {
        if (export_metal_object_info->exportObjectType == VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT) {
            retval = true;
            break;
        }
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    return retval;
}
#endif  // VK_USE_PLATFORM_METAL_EXT

namespace vvl {
DeviceMemory::DeviceMemory(VkDeviceMemory handle, const VkMemoryAllocateInfo *allocate_info, uint64_t fake_address,
                           const VkMemoryType &memory_type, const VkMemoryHeap &memory_heap,
                           std::optional<DedicatedBinding> &&dedicated_binding, uint32_t physical_device_count)
    : StateObject(handle, kVulkanObjectTypeDeviceMemory),
      safe_allocate_info(allocate_info),
      allocate_info(*safe_allocate_info.ptr()),
      export_handle_types(GetExportHandleTypes(allocate_info)),
      import_handle_type(GetImportHandleType(allocate_info)),
      unprotected((memory_type.propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) == 0),
      multi_instance(IsMultiInstance(allocate_info, memory_heap, physical_device_count)),
      dedicated(std::move(dedicated_binding)),
      mapped_range{},
#ifdef VK_USE_PLATFORM_METAL_EXT
      metal_buffer_export(GetMetalExport(allocate_info)),
#endif  // VK_USE_PLATFORM_METAL_EXT
      p_driver_data(nullptr),
      fake_base_address(fake_address) {
}
}  // namespace vvl

void vvl::BindableLinearMemoryTracker::BindMemory(StateObject *parent, std::shared_ptr<vvl::DeviceMemory> &memory_state,
                                                  VkDeviceSize memory_offset, VkDeviceSize resource_offset, VkDeviceSize size) {
    ASSERT_AND_RETURN(memory_state);

    memory_state->AddParent(parent);
    binding_ = {memory_state, memory_offset, 0u};
}

DeviceMemoryState vvl::BindableLinearMemoryTracker::GetBoundMemoryStates() const {
    return binding_.memory_state ? DeviceMemoryState{binding_.memory_state} : DeviceMemoryState{};
}

BoundMemoryRange vvl::BindableLinearMemoryTracker::GetBoundMemoryRange(const MemoryRange &range) const {
    return binding_.memory_state ? BoundMemoryRange{BoundMemoryRange::value_type{
                                       binding_.memory_state->VkHandle(),
                                       BoundMemoryRange::value_type::second_type{
                                           {binding_.memory_offset + range.begin, binding_.memory_offset + range.end}}}}
                                 : BoundMemoryRange{};
}

BoundRanges vvl::BindableLinearMemoryTracker::GetBoundRanges(const BufferRange &ranges_bounds,
                                                             const std::vector<BufferRange> &ranges) const {
    BoundRanges memory_to_bound_ranges_map;
    if (!binding_.memory_state) {
        return memory_to_bound_ranges_map;
    }

    const VkDeviceMemory bound_memory = binding_.memory_state->VkHandle();
    std::vector<std::pair<MemoryRange, BufferRange>> &bound_ranges = memory_to_bound_ranges_map[bound_memory];
    bound_ranges.reserve(ranges.size());

    for (const BufferRange &buffer_range : ranges) {
        const MemoryRange memory_range_bounds(binding_.memory_offset,
                                              binding_.memory_offset + buffer_range.begin + buffer_range.distance());
        bound_ranges.emplace_back(memory_range_bounds, buffer_range);
    }

    return memory_to_bound_ranges_map;
}

unsigned vvl::BindableSparseMemoryTracker::CountDeviceMemory(VkDeviceMemory memory) const {
    unsigned count = 0u;
    auto guard = ReadLockGuard{binding_lock_};
    for (const auto &range_state : binding_map_) {
        count += (range_state.second.memory_state && range_state.second.memory_state->VkHandle() == memory);
    }
    return count;
}

bool vvl::BindableSparseMemoryTracker::HasFullRangeBound() const {
    if (!is_resident_) {
        VkDeviceSize current_offset = 0u;
        {
            auto guard = ReadLockGuard{binding_lock_};
            for (const auto &range : binding_map_) {
                if (current_offset != range.first.begin || !range.second.memory_state || range.second.memory_state->Invalid()) {
                    return false;
                }
                current_offset = range.first.end;
            }
        }

        if (current_offset != resource_size_) return false;
    }

    return true;
}

void vvl::BindableSparseMemoryTracker::BindMemory(StateObject *parent, std::shared_ptr<vvl::DeviceMemory> &memory_state,
                                                  VkDeviceSize memory_offset, VkDeviceSize resource_offset, VkDeviceSize size) {
    MemoryBinding memory_data{memory_state, memory_offset, resource_offset};
    BindingMap::value_type item{{resource_offset, resource_offset + size}, memory_data};

    auto guard = WriteLockGuard{binding_lock_};

    // Since we don't know which ranges will be removed, we need to unbind everything and rebind later
    for (auto &value_pair : binding_map_) {
        if (value_pair.second.memory_state) value_pair.second.memory_state->RemoveParent(parent);
    }
    binding_map_.overwrite_range(item);

    for (auto &value_pair : binding_map_) {
        if (value_pair.second.memory_state) value_pair.second.memory_state->AddParent(parent);
    }
}

BoundMemoryRange vvl::BindableSparseMemoryTracker::GetBoundMemoryRange(const MemoryRange &range) const {
    BoundMemoryRange mem_ranges;
    auto guard = ReadLockGuard{binding_lock_};
    auto range_bounds = binding_map_.bounds(range);

    for (auto it = range_bounds.begin; it != range_bounds.end; ++it) {
        const auto &[resource_range, memory_data] = *it;
        if (memory_data.memory_state && memory_data.memory_state->VkHandle() != VK_NULL_HANDLE) {
            const VkDeviceSize memory_range_start = std::max(range.begin, memory_data.resource_offset) -
                memory_data.resource_offset + memory_data.memory_offset;
            const VkDeviceSize memory_range_end =
                std::min(range.end, memory_data.resource_offset + resource_range.distance()) - memory_data.resource_offset +
                memory_data.memory_offset;

            mem_ranges[memory_data.memory_state->VkHandle()].emplace_back(memory_range_start, memory_range_end);
        }
    }
    return mem_ranges;
}

BoundRanges vvl::BindableSparseMemoryTracker::GetBoundRanges(const BufferRange &ranges_bounds,
                                                             const std::vector<BufferRange> &buffer_ranges) const {
    BoundRanges memory_to_bound_ranges_map;
    auto guard = ReadLockGuard{binding_lock_};

    auto bound_memory_ranges = binding_map_.bounds(ranges_bounds);

    for (auto it = bound_memory_ranges.begin; it != bound_memory_ranges.end; ++it) {
        const auto &[bounds_buffer_range, bounds_buffer_range_memory] = *it;

        if (bounds_buffer_range_memory.memory_state && bounds_buffer_range_memory.memory_state->VkHandle() != VK_NULL_HANDLE) {
            MemoryRange bounds_memory_range;
            bounds_memory_range.begin = std::max(ranges_bounds.begin, bounds_buffer_range_memory.resource_offset) -
                                        bounds_buffer_range_memory.resource_offset + bounds_buffer_range_memory.memory_offset;
            bounds_memory_range.end =
                std::min(ranges_bounds.end, bounds_buffer_range_memory.resource_offset + bounds_buffer_range.distance()) -
                bounds_buffer_range_memory.resource_offset + bounds_buffer_range_memory.memory_offset;

            std::pair<MemoryRange, BufferRange> bounds_mem_and_buffer_range;
            bounds_mem_and_buffer_range.first = bounds_memory_range;
            bounds_mem_and_buffer_range.second =
                BufferRange(bounds_buffer_range_memory.resource_offset,
                            bounds_buffer_range_memory.resource_offset + bounds_buffer_range.distance());

            for (const BufferRange &buffer_range : buffer_ranges) {
                if (!bounds_mem_and_buffer_range.second.intersects(buffer_range)) {
                    continue;
                }

                MemoryRange memory_range;
                memory_range.begin = std::max(buffer_range.begin, bounds_buffer_range_memory.resource_offset) -
                                     bounds_buffer_range_memory.resource_offset + bounds_buffer_range_memory.memory_offset;
                memory_range.end =
                    std::min(buffer_range.end, bounds_buffer_range_memory.resource_offset + bounds_buffer_range.distance()) -
                    bounds_buffer_range_memory.resource_offset + bounds_buffer_range_memory.memory_offset;

                std::pair<MemoryRange, BufferRange> mem_and_buffer_range;
                mem_and_buffer_range.first = bounds_mem_and_buffer_range.first & memory_range;
                mem_and_buffer_range.second = bounds_mem_and_buffer_range.second & buffer_range;

                std::vector<std::pair<MemoryRange, BufferRange>> &vk_memory_ranges_vec =
                    memory_to_bound_ranges_map[bounds_buffer_range_memory.memory_state->VkHandle()];
                auto insert_pos =
                    std::lower_bound(vk_memory_ranges_vec.begin(), vk_memory_ranges_vec.end(), mem_and_buffer_range,
                                     [](const std::pair<MemoryRange, BufferRange> &lhs,
                                        const std::pair<MemoryRange, BufferRange> &rhs) { return lhs.first < rhs.first; });
                vk_memory_ranges_vec.insert(insert_pos, mem_and_buffer_range);
            }
        }
    }
    return memory_to_bound_ranges_map;
}

DeviceMemoryState vvl::BindableSparseMemoryTracker::GetBoundMemoryStates() const {
    DeviceMemoryState dev_memory_states;

    {
        auto guard = ReadLockGuard{binding_lock_};
        for (auto &binding : binding_map_) {
            if (binding.second.memory_state) dev_memory_states.emplace(binding.second.memory_state);
        }
    }

    return dev_memory_states;
}


vvl::BindableMultiplanarMemoryTracker::BindableMultiplanarMemoryTracker(const VkMemoryRequirements *requirements, uint32_t num_planes)
    : planes_(num_planes) {
    for (unsigned i = 0; i < num_planes; ++i) {
        planes_[i].size = requirements[i].size;
    }
}
unsigned vvl::BindableMultiplanarMemoryTracker::CountDeviceMemory(VkDeviceMemory memory) const {
    unsigned count = 0u;
    for (size_t i = 0u; i < planes_.size(); i++) {
        const auto &plane = planes_[i];
        count += (plane.binding.memory_state && plane.binding.memory_state->VkHandle() == memory);
    }

    return count;
}

bool vvl::BindableMultiplanarMemoryTracker::HasFullRangeBound() const {
    bool full_range_bound = true;

    for (unsigned i = 0u; i < planes_.size(); ++i) {
        full_range_bound &= (planes_[i].binding.memory_state != nullptr);
    }

    return full_range_bound;
}

// resource_offset is the plane index
void vvl::BindableMultiplanarMemoryTracker::BindMemory(StateObject *parent, std::shared_ptr<vvl::DeviceMemory> &memory_state,
                                                       VkDeviceSize memory_offset, VkDeviceSize resource_offset,
                                                       VkDeviceSize size) {
    ASSERT_AND_RETURN(memory_state);

    assert(resource_offset < planes_.size());
    memory_state->AddParent(parent);
    planes_[static_cast<size_t>(resource_offset)].binding = {memory_state, memory_offset, 0u};
}

// range needs to be between [0, planes_[0].size + planes_[1].size + planes_[2].size)
// To access plane 0 range must be [0, planes_[0].size)
// To access plane 1 range must be [planes_[0].size, planes_[1].size)
// To access plane 2 range must be [planes_[1].size, planes_[2].size)
BoundMemoryRange vvl::BindableMultiplanarMemoryTracker::GetBoundMemoryRange(const MemoryRange &range) const {
    BoundMemoryRange mem_ranges;
    VkDeviceSize start_offset = 0u;
    for (unsigned i = 0u; i < planes_.size(); ++i) {
        const auto &plane = planes_[i];
        MemoryRange plane_range{start_offset, start_offset + plane.size};
        if (plane.binding.memory_state && range.intersects(plane_range)) {
            VkDeviceSize range_end = range.end > plane_range.end ? plane_range.end : range.end;
            const VkDeviceMemory dev_mem = plane.binding.memory_state->VkHandle();
            mem_ranges[dev_mem].emplace_back((plane.binding.memory_offset + range.begin),
                                             (plane.binding.memory_offset + range_end));
        }
        start_offset += plane.size;
    }

    return mem_ranges;
}

DeviceMemoryState vvl::BindableMultiplanarMemoryTracker::GetBoundMemoryStates() const {
    DeviceMemoryState dev_memory_states;

    for (unsigned i = 0u; i < planes_.size(); ++i) {
        if (planes_[i].binding.memory_state) {
            dev_memory_states.insert(planes_[i].binding.memory_state);
        }
    }

    return dev_memory_states;
}

std::pair<VkDeviceMemory, MemoryRange> vvl::Bindable::GetResourceMemoryOverlap(
        const MemoryRange &memory_region, const Bindable *other_resource,
        const MemoryRange &other_memory_region) const {
    if (!other_resource) return {VK_NULL_HANDLE, {}};

    auto ranges = GetBoundMemoryRange(memory_region);
    auto other_ranges = other_resource->GetBoundMemoryRange(other_memory_region);

    for (const auto &[memory, memory_ranges] : ranges) {
        // Check if we have memory from same VkDeviceMemory bound
        if (auto it = other_ranges.find(memory); it != other_ranges.end()) {
            // Check if any of the bound memory ranges overlap
            for (const auto &memory_range : memory_ranges) {
                for (const auto &other_memory_range : it->second) {
                    if (other_memory_range.intersects(memory_range)) {
                        auto memory_space_intersection = other_memory_range & memory_range;
                        return {memory, memory_space_intersection};
                    }
                }
            }
        }
    }
    return {VK_NULL_HANDLE, {}};
}

VkDeviceSize vvl::Bindable::GetFakeBaseAddress() const {
    // TODO: Sparse resources are not implemented yet
    const auto *binding = Binding();
    return binding ? binding->memory_offset + binding->memory_state->fake_base_address : 0;
}

void vvl::Bindable::CacheInvalidMemory() const {
    need_to_recache_invalid_memory_ = false;
    cached_invalid_memory_.clear();
    for (auto const &bindable : GetBoundMemoryStates()) {
        if (bindable->Invalid()) {
            cached_invalid_memory_.insert(bindable);
        }
    }
}
