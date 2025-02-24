/*
** Copyright (c) 2015-2018, 2023 The Khronos Group Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#pragma once

#include <stdlib.h>
#include <cstring>

#include <array>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "vulkan/vulkan.h"
#include "vulkan/vk_icd.h"

namespace icd {

using mutex_t = std::mutex;
using lock_guard_t = std::lock_guard<mutex_t>;
using unique_lock_t = std::unique_lock<mutex_t>;

static mutex_t global_lock;
static uint64_t global_unique_handle = 1;
static const uint32_t SUPPORTED_LOADER_ICD_INTERFACE_VERSION = 5;
static uint32_t loader_interface_version = 0;
static bool negotiate_loader_icd_interface_called = false;
static void* CreateDispObjHandle() {
    auto handle = new VK_LOADER_DATA;
    set_loader_magic_value(handle);
    return handle;
}
static void DestroyDispObjHandle(void* handle) { delete reinterpret_cast<VK_LOADER_DATA*>(handle); }

static constexpr uint32_t icd_physical_device_count = 1;
static std::unordered_map<VkInstance, std::array<VkPhysicalDevice, icd_physical_device_count>> physical_device_map;
static std::unordered_map<VkPhysicalDevice, std::unordered_set<VkDisplayKHR>> display_map;

// Map device memory handle to any mapped allocations that we'll need to free on unmap
static std::unordered_map<VkDeviceMemory, std::vector<void*>> mapped_memory_map;

// Map device memory allocation handle to the size
static std::unordered_map<VkDeviceMemory, VkDeviceSize> allocated_memory_size_map;

static std::unordered_map<VkDevice, std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkQueue>>> queue_map;
static VkDeviceAddress current_available_address = 0x10000000;
struct BufferState {
    VkDeviceSize size;
    VkDeviceAddress address;
};
static std::unordered_map<VkDevice, std::unordered_map<VkBuffer, BufferState>> buffer_map;
static std::unordered_map<VkDevice, std::unordered_map<VkImage, VkDeviceSize>> image_memory_size_map;
static std::unordered_map<VkDevice, std::unordered_set<VkCommandPool>> command_pool_map;
static std::unordered_map<VkCommandPool, std::vector<VkCommandBuffer>> command_pool_buffer_map;

static constexpr uint32_t icd_swapchain_image_count = 1;
static std::unordered_map<VkSwapchainKHR, VkImage[icd_swapchain_image_count]> swapchain_image_map;

static VkPhysicalDeviceLimits SetLimits(VkPhysicalDeviceLimits* limits);
void SetBoolArrayTrue(VkBool32* bool_array, uint32_t num_bools);
VkDeviceSize GetImageSizeFromCreateInfo(const VkImageCreateInfo* pCreateInfo);

}  // namespace icd
