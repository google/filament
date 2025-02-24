/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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
#pragma once
#include "state_tracker/state_object.h"
#include <vulkan/utility/vk_safe_struct.hpp>
#include <vector>

class QueueFamilyPerfCounters {
  public:
    std::vector<VkPerformanceCounterKHR> counters;
};

class SurfacelessQueryState {
  public:
    std::vector<vku::safe_VkSurfaceFormat2KHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
    vku::safe_VkSurfaceCapabilities2KHR capabilities;
};

namespace vvl {

class PhysicalDevice : public StateObject {
  public:
    uint32_t queue_family_known_count = 1;  // spec implies one QF must always be supported
    const std::vector<VkQueueFamilyProperties> queue_family_properties;
    const VkQueueFlags supported_queues;
    // TODO These are currently used by CoreChecks, but should probably be refactored
    bool vkGetPhysicalDeviceDisplayPlanePropertiesKHR_called = false;
    uint32_t display_plane_property_count = 0;

    // Map of queue family index to QueueFamilyPerfCounters
    unordered_map<uint32_t, std::unique_ptr<QueueFamilyPerfCounters>> perf_counters;

    // Surfaceless Query extension needs 'global' surface_state data
    SurfacelessQueryState surfaceless_query_state{};

    PhysicalDevice(VkPhysicalDevice handle);

    VkPhysicalDevice VkHandle() const { return handle_.Cast<VkPhysicalDevice>(); }

  private:
    const std::vector<VkQueueFamilyProperties> GetQueueFamilyProps(VkPhysicalDevice phys_dev);
    VkQueueFlags GetSupportedQueues();
};

class DisplayMode : public StateObject {
  public:
    const VkPhysicalDevice physical_device;

    DisplayMode(VkDisplayModeKHR handle, VkPhysicalDevice phys_dev)
        : StateObject(handle, kVulkanObjectTypeDisplayModeKHR), physical_device(phys_dev) {}

    VkDisplayModeKHR VkHandle() const { return handle_.Cast<VkDisplayModeKHR>(); }
};

}  // namespace vvl
