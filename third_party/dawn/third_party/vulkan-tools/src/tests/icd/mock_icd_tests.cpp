/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
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
 *
 */

#include "test_common.h"

void setup_mock_icd_env_vars() {
    // Necessary to point the loader at the mock driver
    set_environment_var("VK_DRIVER_FILES", MOCK_ICD_JSON_MANIFEST_PATH);
    // Prevents layers from being loaded at all
    set_environment_var("VK_LOADER_LAYERS_DISABLE", "~all~");
}

// Defines a simple context for tests to use.
// Creates an instance, physical_device, device, and queue

class MockICD : public ::testing::Test {
  protected:
    void SetUp() override {
        setup_mock_icd_env_vars();

        // Create an instance with the latest version & necessary surface extensions
        VkResult res = VK_SUCCESS;
        VkApplicationInfo app_info{};
        app_info.apiVersion = VK_HEADER_VERSION_COMPLETE;
        VkInstanceCreateInfo instance_create_info{};
        instance_create_info.pApplicationInfo = &app_info;
        std::array<const char*, 2> extension_to_enable = {"VK_KHR_surface", "VK_KHR_display"};
        instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extension_to_enable.size());
        instance_create_info.ppEnabledExtensionNames = extension_to_enable.data();
        res = vkCreateInstance(&instance_create_info, nullptr, &instance);
        ASSERT_EQ(res, VK_SUCCESS);
        ASSERT_NE(instance, nullptr);

        uint32_t count = 1;
        res = vkEnumeratePhysicalDevices(instance, &count, &physical_device);
        ASSERT_EQ(res, VK_SUCCESS);
        ASSERT_EQ(count, 1);
        ASSERT_NE(physical_device, nullptr);

        VkDeviceCreateInfo device_create_info{};
        std::array<const char*, 1> device_extension_to_enable = {"VK_KHR_swapchain"};
        device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extension_to_enable.size());
        device_create_info.ppEnabledExtensionNames = device_extension_to_enable.data();
        res = vkCreateDevice(physical_device, &device_create_info, nullptr, &device);
        ASSERT_EQ(res, VK_SUCCESS);
        ASSERT_NE(device, nullptr);

        vkGetDeviceQueue(device, 0, 0, &queue);
        ASSERT_NE(queue, nullptr);
    }

    void TearDown() override {
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    VkInstance instance{};
    VkPhysicalDevice physical_device{};
    VkDevice device{};
    VkQueue queue{};
};

/*
 * Exercises the following commands:
 * vkEnumerateInstanceExtensionProperties
 * vkEnumerateInstanceLayerProperties
 * vkEnumerateInstanceVersion
 * vkCreateInstance
 * vkEnumeratePhysicalDevices
 * vkEnumeratePhysicalDeviceGroups
 * vkEnumerateDeviceExtensionProperties
 * vkGetPhysicalDeviceQueueFamilyProperties
 * vkGetPhysicalDeviceQueueFamilyProperties2
 * vkCreateDevice
 * vkDestroyDevice
 * vkDestroyInstance
 * vkGetDeviceQueue
 * vkGetDeviceQueue2
 */
TEST_F(MockICD, InitializationFunctions) {
    setup_mock_icd_env_vars();
    VkResult res = VK_SUCCESS;
    uint32_t count = 0;
    res = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_GT(count, 0);
    std::vector<VkExtensionProperties> inst_ext_props{count, VkExtensionProperties{}};
    res = vkEnumerateInstanceExtensionProperties(nullptr, &count, inst_ext_props.data());
    ASSERT_EQ(res, VK_SUCCESS);

    // Since we disabled layers, count should stay zero
    count = 0;
    res = vkEnumerateInstanceLayerProperties(&count, nullptr);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, 0);

    uint32_t api_version;
    res = vkEnumerateInstanceVersion(&api_version);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_GE(api_version, VK_API_VERSION_1_0);

    VkInstanceCreateInfo inst_create_info{};
    VkInstance instance{};
    res = vkCreateInstance(&inst_create_info, nullptr, &instance);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(instance, nullptr);

    count = 0;
    VkPhysicalDevice physical_device;
    res = vkEnumeratePhysicalDevices(instance, &count, nullptr);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, 1);

    res = vkEnumeratePhysicalDevices(instance, &count, &physical_device);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, 1);
    ASSERT_NE(physical_device, nullptr);

    count = 0;
    res = vkEnumeratePhysicalDeviceGroups(instance, &count, nullptr);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_GT(count, 0);

    VkPhysicalDeviceGroupProperties physical_device_groups;
    count = 1;
    res = vkEnumeratePhysicalDeviceGroups(instance, &count, &physical_device_groups);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, 1);

    count = 0;
    res = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, nullptr);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_GT(count, 0);
    std::vector<VkExtensionProperties> device_ext_props{count, VkExtensionProperties{}};
    res = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, device_ext_props.data());
    ASSERT_EQ(res, VK_SUCCESS);

    // Device layers are deprecated, should return number of active layers, which is zero
    count = 0;
    res = vkEnumerateDeviceLayerProperties(physical_device, &count, nullptr);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, 0);
    count = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
    ASSERT_EQ(count, 3);
    VkQueueFamilyProperties queue_family_properties[3] = {};
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, queue_family_properties);
    ASSERT_EQ(count, 3);
    ASSERT_EQ(queue_family_properties[0].queueFlags, 1 | 2 | 4 | 8 | 16);
    ASSERT_EQ(queue_family_properties[1].queueFlags, 4 | 16 | 32);
    ASSERT_EQ(queue_family_properties[2].queueFlags, 4 | 16 | 64);
    for (uint32_t i = 0; i < count; ++i) {
        ASSERT_EQ(queue_family_properties[i].queueCount, 1);
        ASSERT_EQ(queue_family_properties[i].timestampValidBits, 16);
        ASSERT_EQ(queue_family_properties[i].minImageTransferGranularity.width, 1);
        ASSERT_EQ(queue_family_properties[i].minImageTransferGranularity.height, 1);
        ASSERT_EQ(queue_family_properties[i].minImageTransferGranularity.depth, 1);
    }

    vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &count, nullptr);
    ASSERT_EQ(count, 3);
    VkQueueFamilyProperties2 queue_family_properties2[3] = {};
    for (uint32_t i = 0; i < count; ++i) {
        queue_family_properties2[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
    }
    vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &count, queue_family_properties2);
    ASSERT_EQ(count, 3);
    ASSERT_EQ(queue_family_properties2[0].queueFamilyProperties.queueFlags, 1 | 2 | 4 | 8 | 16);
    ASSERT_EQ(queue_family_properties2[1].queueFamilyProperties.queueFlags, 4 | 16 | 32);
    ASSERT_EQ(queue_family_properties2[2].queueFamilyProperties.queueFlags, 4 | 16 | 64);
    for (uint32_t i = 0; i < count; ++i) {
        ASSERT_EQ(queue_family_properties2[i].queueFamilyProperties.queueCount, 1);
        ASSERT_EQ(queue_family_properties2[i].queueFamilyProperties.timestampValidBits, 16);
        ASSERT_EQ(queue_family_properties2[i].queueFamilyProperties.minImageTransferGranularity.width, 1);
        ASSERT_EQ(queue_family_properties2[i].queueFamilyProperties.minImageTransferGranularity.height, 1);
        ASSERT_EQ(queue_family_properties2[i].queueFamilyProperties.minImageTransferGranularity.depth, 1);
    }

    VkDeviceCreateInfo dev_create_info{};
    VkDevice device{};
    res = vkCreateDevice(physical_device, &dev_create_info, nullptr, &device);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(device, nullptr);

    VkQueue queue{};
    vkGetDeviceQueue(device, 0, 0, &queue);
    ASSERT_NE(queue, nullptr);

    VkDeviceQueueInfo2 queue_info{};
    vkGetDeviceQueue2(device, &queue_info, &queue);
    ASSERT_NE(queue, nullptr);

    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
}

/*
 * Exercises the following commands:
 * vkCreateCommandPool
 * vkAllocateCommandBuffers
 * vkFreeCommandBuffers
 * vkDestroyCommandPool
 */
TEST_F(MockICD, CommandBufferOperations) {
    VkResult res = VK_SUCCESS;
    VkCommandPoolCreateInfo command_pool_create_info{};
    VkCommandPool command_pool;
    res = vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool);
    ASSERT_EQ(VK_SUCCESS, res);

    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.commandBufferCount = 5;
    std::array<VkCommandBuffer, 5> command_buffers;
    res = vkAllocateCommandBuffers(device, &command_buffer_allocate_info, command_buffers.data());
    ASSERT_EQ(VK_SUCCESS, res);
    for (const auto& command_buffer : command_buffers) {
        ASSERT_NE(nullptr, command_buffer);
    }

    vkFreeCommandBuffers(device, command_pool, 5, command_buffers.data());

    vkDestroyCommandPool(device, command_pool, nullptr);
}

VkResult create_surface(VkInstance instance, VkSurfaceKHR& surface) {
    VkDisplaySurfaceCreateInfoKHR surf_create_info{};
    return vkCreateDisplayPlaneSurfaceKHR(instance, &surf_create_info, nullptr, &surface);
}

TEST_F(MockICD, vkGetPhysicalDeviceSurfacePresentModesKHR) {
    VkResult res = VK_SUCCESS;
    VkSurfaceKHR surface{};
    res = create_surface(instance, surface);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(surface, VK_NULL_HANDLE);
    uint32_t count = 0;
    std::array<VkPresentModeKHR, 6> present_modes{};
    res = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &count, nullptr);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, present_modes.size());
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &count, present_modes.data());
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(present_modes[0], VK_PRESENT_MODE_IMMEDIATE_KHR);
    ASSERT_EQ(present_modes[1], VK_PRESENT_MODE_MAILBOX_KHR);
    ASSERT_EQ(present_modes[2], VK_PRESENT_MODE_FIFO_KHR);
    ASSERT_EQ(present_modes[3], VK_PRESENT_MODE_FIFO_RELAXED_KHR);
    ASSERT_EQ(present_modes[4], VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR);
    ASSERT_EQ(present_modes[5], VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR);
    vkDestroySurfaceKHR(instance, surface, nullptr);
}

TEST_F(MockICD, vkGetPhysicalDeviceSurfaceFormatsKHR) {
    VkResult res = VK_SUCCESS;
    VkSurfaceKHR surface{};
    res = create_surface(instance, surface);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(surface, VK_NULL_HANDLE);
    uint32_t count = 0;
    std::array<VkSurfaceFormatKHR, 2> surface_formats{};
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count, nullptr);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, surface_formats.size());
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count, surface_formats.data());
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(surface_formats[0].format, VK_FORMAT_B8G8R8A8_UNORM);
    ASSERT_EQ(surface_formats[0].colorSpace, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    ASSERT_EQ(surface_formats[1].format, VK_FORMAT_R8G8B8A8_UNORM);
    ASSERT_EQ(surface_formats[1].colorSpace, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    vkDestroySurfaceKHR(instance, surface, nullptr);
}

TEST_F(MockICD, vkGetPhysicalDeviceSurfaceFormats2KHR) {
    VkResult res = VK_SUCCESS;
    VkSurfaceKHR surface{};
    res = create_surface(instance, surface);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(surface, VK_NULL_HANDLE);
    uint32_t count = 0;
    std::array<VkSurfaceFormat2KHR, 2> surface_formats2{};
    VkPhysicalDeviceSurfaceInfo2KHR surface_info{};
    surface_info.surface = surface;
    res = vkGetPhysicalDeviceSurfaceFormats2KHR(physical_device, &surface_info, &count, nullptr);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, surface_formats2.size());
    vkGetPhysicalDeviceSurfaceFormats2KHR(physical_device, &surface_info, &count, surface_formats2.data());
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(surface_formats2[0].pNext, nullptr);
    ASSERT_EQ(surface_formats2[0].surfaceFormat.format, VK_FORMAT_B8G8R8A8_UNORM);
    ASSERT_EQ(surface_formats2[0].surfaceFormat.colorSpace, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    ASSERT_EQ(surface_formats2[1].pNext, nullptr);
    ASSERT_EQ(surface_formats2[1].surfaceFormat.format, VK_FORMAT_R8G8B8A8_UNORM);
    ASSERT_EQ(surface_formats2[1].surfaceFormat.colorSpace, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    vkDestroySurfaceKHR(instance, surface, nullptr);
}

TEST_F(MockICD, vkGetPhysicalDeviceSurfaceSupportKHR) {
    VkResult res = VK_SUCCESS;
    VkSurfaceKHR surface{};
    res = create_surface(instance, surface);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(surface, VK_NULL_HANDLE);
    VkBool32 supported = false;
    res = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, 0, surface, &supported);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(supported, true);
    vkDestroySurfaceKHR(instance, surface, nullptr);
}

TEST_F(MockICD, vkGetPhysicalDeviceSurfaceCapabilitiesKHR) {
    VkResult res = VK_SUCCESS;
    VkSurfaceKHR surface{};
    res = create_surface(instance, surface);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(surface, VK_NULL_HANDLE);
    VkSurfaceCapabilitiesKHR surface_capabilities{};
    res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(surface_capabilities.minImageCount, 1);
    ASSERT_EQ(surface_capabilities.currentExtent.width, std::numeric_limits<uint32_t>::max());
    ASSERT_EQ(surface_capabilities.currentExtent.height, std::numeric_limits<uint32_t>::max());
    ASSERT_EQ(surface_capabilities.minImageExtent.width, 1);
    ASSERT_EQ(surface_capabilities.minImageExtent.height, 1);
    ASSERT_EQ(surface_capabilities.maxImageArrayLayers, 128);
    ASSERT_EQ(surface_capabilities.currentTransform, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
    vkDestroySurfaceKHR(instance, surface, nullptr);
}

TEST_F(MockICD, vkGetPhysicalDeviceSurfaceCapabilities2KHR) {
    VkResult res = VK_SUCCESS;
    VkSurfaceKHR surface{};
    res = create_surface(instance, surface);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(surface, VK_NULL_HANDLE);
    VkSurfaceCapabilities2KHR surface_capabilities2{};
    VkPhysicalDeviceSurfaceInfo2KHR surface_info{};
    surface_info.surface = surface;
    res = vkGetPhysicalDeviceSurfaceCapabilities2KHR(physical_device, &surface_info, &surface_capabilities2);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(surface_capabilities2.surfaceCapabilities.minImageCount, 1);
    ASSERT_EQ(surface_capabilities2.surfaceCapabilities.currentExtent.width, std::numeric_limits<uint32_t>::max());
    ASSERT_EQ(surface_capabilities2.surfaceCapabilities.currentExtent.height, std::numeric_limits<uint32_t>::max());
    ASSERT_EQ(surface_capabilities2.surfaceCapabilities.minImageExtent.width, 1);
    ASSERT_EQ(surface_capabilities2.surfaceCapabilities.minImageExtent.height, 1);
    ASSERT_EQ(surface_capabilities2.surfaceCapabilities.maxImageArrayLayers, 128);
    ASSERT_EQ(surface_capabilities2.surfaceCapabilities.currentTransform, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
    vkDestroySurfaceKHR(instance, surface, nullptr);
}

TEST_F(MockICD, vkGetPhysicalDeviceMemoryProperties) {
    VkPhysicalDeviceMemoryProperties memory_properties{};
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
    ASSERT_EQ(memory_properties.memoryTypeCount, 6);
    ASSERT_EQ(memory_properties.memoryTypes[0].propertyFlags,
              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    ASSERT_EQ(memory_properties.memoryTypes[0].heapIndex, 0);
    ASSERT_EQ(memory_properties.memoryTypes[5].propertyFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    ASSERT_EQ(memory_properties.memoryTypes[5].heapIndex, 1);
    ASSERT_EQ(memory_properties.memoryHeapCount, 2);
    ASSERT_EQ(memory_properties.memoryHeaps[0].flags, VK_MEMORY_HEAP_MULTI_INSTANCE_BIT);
    ASSERT_EQ(memory_properties.memoryHeaps[0].size, 8000000000);
    ASSERT_EQ(memory_properties.memoryHeaps[1].flags, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
    ASSERT_EQ(memory_properties.memoryHeaps[1].size, 8000000000);
}

TEST_F(MockICD, vkGetPhysicalDeviceMemoryProperties2) {
    VkPhysicalDeviceMemoryProperties2 memory_properties2{};
    vkGetPhysicalDeviceMemoryProperties2(physical_device, &memory_properties2);
    ASSERT_EQ(memory_properties2.memoryProperties.memoryTypeCount, 6);
    ASSERT_EQ(memory_properties2.memoryProperties.memoryTypes[0].propertyFlags,
              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    ASSERT_EQ(memory_properties2.memoryProperties.memoryTypes[0].heapIndex, 0);
    ASSERT_EQ(memory_properties2.memoryProperties.memoryTypes[5].propertyFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    ASSERT_EQ(memory_properties2.memoryProperties.memoryTypes[5].heapIndex, 1);
    ASSERT_EQ(memory_properties2.memoryProperties.memoryHeapCount, 2);
    ASSERT_EQ(memory_properties2.memoryProperties.memoryHeaps[0].flags, VK_MEMORY_HEAP_MULTI_INSTANCE_BIT);
    ASSERT_EQ(memory_properties2.memoryProperties.memoryHeaps[0].size, 8000000000);
    ASSERT_EQ(memory_properties2.memoryProperties.memoryHeaps[1].flags, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
    ASSERT_EQ(memory_properties2.memoryProperties.memoryHeaps[1].size, 8000000000);
}

TEST_F(MockICD, vkGetPhysicalDeviceFeatures) {
    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(physical_device, &features);
    // Make sure the first and last elements are set to true
    ASSERT_EQ(features.robustBufferAccess, true);
    ASSERT_EQ(features.inheritedQueries, true);
}

TEST_F(MockICD, vkGetPhysicalDeviceFeatures2) {
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptor_indexing_features{};
    descriptor_indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

    VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT blending_operation_advanced_features{};
    blending_operation_advanced_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT;
    blending_operation_advanced_features.pNext = static_cast<void*>(&descriptor_indexing_features);

    VkPhysicalDeviceFeatures2 features2{};
    features2.pNext = static_cast<void*>(&blending_operation_advanced_features);
    vkGetPhysicalDeviceFeatures2(physical_device, &features2);
    // Make sure the first and last elements are set to true
    ASSERT_EQ(features2.features.robustBufferAccess, true);
    ASSERT_EQ(features2.features.inheritedQueries, true);
    ASSERT_EQ(descriptor_indexing_features.shaderInputAttachmentArrayDynamicIndexing, true);
    ASSERT_EQ(descriptor_indexing_features.runtimeDescriptorArray, true);
    ASSERT_EQ(blending_operation_advanced_features.advancedBlendCoherentOperations, true);
}

TEST_F(MockICD, vkGetPhysicalDeviceFormatProperties) {
    VkFormatProperties format_properties{};
    vkGetPhysicalDeviceFormatProperties(physical_device, VK_FORMAT_R8G8B8A8_SRGB, &format_properties);
    ASSERT_EQ(format_properties.bufferFeatures, 0x00FFFDFF);
    ASSERT_EQ(format_properties.linearTilingFeatures, 0x00FFFDFF);
    ASSERT_EQ(format_properties.optimalTilingFeatures, 0x00FFFDFF);
}

TEST_F(MockICD, vkGetPhysicalDeviceFormatProperties2) {
    VkFormatProperties3 format_properties3{};
    format_properties3.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3;
    VkFormatProperties2 format_properties2{};
    format_properties2.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    format_properties2.pNext = static_cast<void*>(&format_properties3);
    vkGetPhysicalDeviceFormatProperties2(physical_device, VK_FORMAT_R8G8B8A8_SRGB, &format_properties2);
    ASSERT_EQ(format_properties2.formatProperties.bufferFeatures, 0x00FFFDFF);
    ASSERT_EQ(format_properties2.formatProperties.linearTilingFeatures, 0x00FFFDFF);
    ASSERT_EQ(format_properties2.formatProperties.optimalTilingFeatures, 0x00FFFDFF);
    ASSERT_EQ(format_properties3.bufferFeatures, 0x00FFFDFF);
    ASSERT_EQ(format_properties3.linearTilingFeatures, 0x00FFFDFF);
    ASSERT_EQ(format_properties3.optimalTilingFeatures, 0x400000FFFDFF);
}

TEST_F(MockICD, vkGetPhysicalDeviceImageFormatProperties) {
    VkImageFormatProperties image_format_properties{};
    vkGetPhysicalDeviceImageFormatProperties(physical_device, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_LINEAR,
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &image_format_properties);
    ASSERT_EQ(image_format_properties.maxExtent.width, 4096);
    ASSERT_EQ(image_format_properties.maxExtent.height, 4096);
    ASSERT_EQ(image_format_properties.maxExtent.depth, 256);
    ASSERT_EQ(image_format_properties.maxMipLevels, 1);
    ASSERT_EQ(image_format_properties.maxArrayLayers, 1);
    ASSERT_EQ(image_format_properties.sampleCounts, VK_SAMPLE_COUNT_1_BIT);
    ASSERT_EQ(image_format_properties.maxResourceSize, 4294967296 /* this is max of uint32_t + 1*/);
}

TEST_F(MockICD, vkGetPhysicalDeviceImageFormatProperties2) {
    VkImageFormatProperties2 image_format_properties2{};
    VkPhysicalDeviceImageFormatInfo2 image_format_info2{};
    image_format_info2.format = VK_FORMAT_R8G8B8A8_SRGB;
    image_format_info2.type = VK_IMAGE_TYPE_2D;
    image_format_info2.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_format_info2.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_format_info2.flags = 0;
    vkGetPhysicalDeviceImageFormatProperties2(physical_device, &image_format_info2, &image_format_properties2);
    ASSERT_EQ(image_format_properties2.imageFormatProperties.maxExtent.width, 4096);
    ASSERT_EQ(image_format_properties2.imageFormatProperties.maxExtent.height, 4096);
    ASSERT_EQ(image_format_properties2.imageFormatProperties.maxExtent.depth, 256);
    ASSERT_EQ(image_format_properties2.imageFormatProperties.maxMipLevels, 12);
    ASSERT_EQ(image_format_properties2.imageFormatProperties.maxArrayLayers, 256);
    ASSERT_EQ(image_format_properties2.imageFormatProperties.sampleCounts, 0x7F & ~VK_SAMPLE_COUNT_64_BIT);
    ASSERT_EQ(image_format_properties2.imageFormatProperties.maxResourceSize, 4294967296 /* this is max of uint32_t + 1*/);
}

TEST_F(MockICD, vkGetPhysicalDeviceSparseImageFormatProperties) {
    uint32_t count = 0;
    VkSparseImageFormatProperties sparse_image_format_properties{};
    vkGetPhysicalDeviceSparseImageFormatProperties(physical_device, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TYPE_2D,
                                                   VK_SAMPLE_COUNT_64_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                   VK_IMAGE_TILING_OPTIMAL, &count, nullptr);
    ASSERT_EQ(count, 1);
    vkGetPhysicalDeviceSparseImageFormatProperties(physical_device, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TYPE_2D,
                                                   VK_SAMPLE_COUNT_64_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                   VK_IMAGE_TILING_OPTIMAL, &count, &sparse_image_format_properties);
    ASSERT_EQ(sparse_image_format_properties.aspectMask, VK_IMAGE_ASPECT_COLOR_BIT);
    ASSERT_EQ(sparse_image_format_properties.imageGranularity.width, 4);
    ASSERT_EQ(sparse_image_format_properties.imageGranularity.height, 4);
    ASSERT_EQ(sparse_image_format_properties.imageGranularity.depth, 4);
    ASSERT_EQ(sparse_image_format_properties.flags, VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT);
}

TEST_F(MockICD, vkGetPhysicalDeviceSparseImageFormatProperties2) {
    uint32_t count = 0;
    VkSparseImageFormatProperties2 sparse_image_format_properties2{};
    VkPhysicalDeviceSparseImageFormatInfo2 sparse_image_format_info2{};
    sparse_image_format_info2.format = VK_FORMAT_R8G8B8A8_SRGB;
    sparse_image_format_info2.type = VK_IMAGE_TYPE_2D;
    sparse_image_format_info2.samples = VK_SAMPLE_COUNT_64_BIT;
    sparse_image_format_info2.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sparse_image_format_info2.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkGetPhysicalDeviceSparseImageFormatProperties2(physical_device, &sparse_image_format_info2, &count, nullptr);
    ASSERT_EQ(count, 1);
    vkGetPhysicalDeviceSparseImageFormatProperties2(physical_device, &sparse_image_format_info2, &count,
                                                    &sparse_image_format_properties2);
    ASSERT_EQ(sparse_image_format_properties2.properties.aspectMask, VK_IMAGE_ASPECT_COLOR_BIT);
    ASSERT_EQ(sparse_image_format_properties2.properties.imageGranularity.width, 4);
    ASSERT_EQ(sparse_image_format_properties2.properties.imageGranularity.height, 4);
    ASSERT_EQ(sparse_image_format_properties2.properties.imageGranularity.depth, 4);
    ASSERT_EQ(sparse_image_format_properties2.properties.flags, VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT);
}

TEST_F(MockICD, vkGetPhysicalDeviceProperties) {
    VkPhysicalDeviceProperties physical_device_properties{};
    vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
    ASSERT_EQ(physical_device_properties.apiVersion, VK_HEADER_VERSION_COMPLETE);
    ASSERT_EQ(physical_device_properties.driverVersion, 1);
    ASSERT_EQ(physical_device_properties.vendorID, 0xba5eba11);
    ASSERT_EQ(physical_device_properties.deviceID, 0xf005ba11);
    ASSERT_EQ(physical_device_properties.deviceType, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
    ASSERT_STREQ(&physical_device_properties.deviceName[0], "Vulkan Mock Device");
    ASSERT_EQ(physical_device_properties.pipelineCacheUUID[0], 18);
    ASSERT_EQ(physical_device_properties.limits.maxImageDimension1D, 4096);
    ASSERT_EQ(physical_device_properties.limits.nonCoherentAtomSize, 256);
    ASSERT_EQ(physical_device_properties.sparseProperties.residencyAlignedMipSize, VK_TRUE);
    ASSERT_EQ(physical_device_properties.sparseProperties.residencyNonResidentStrict, VK_TRUE);
    ASSERT_EQ(physical_device_properties.sparseProperties.residencyStandard2DBlockShape, VK_TRUE);
    ASSERT_EQ(physical_device_properties.sparseProperties.residencyStandard2DMultisampleBlockShape, VK_TRUE);
    ASSERT_EQ(physical_device_properties.sparseProperties.residencyStandard3DBlockShape, VK_TRUE);
}

TEST_F(MockICD, vkGetPhysicalDeviceProperties2) {
    VkPhysicalDeviceVulkan11Properties properties11{};
    properties11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;

    VkPhysicalDeviceVulkan12Properties properties12{};
    properties12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
    properties12.pNext = static_cast<void*>(&properties11);

    VkPhysicalDeviceVulkan13Properties properties13{};
    properties13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
    properties13.pNext = static_cast<void*>(&properties12);

    VkPhysicalDeviceProtectedMemoryProperties protected_memory_properties{};
    protected_memory_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES;
    protected_memory_properties.pNext = static_cast<void*>(&properties13);

    VkPhysicalDeviceFloatControlsProperties float_controls_properties{};
    float_controls_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES;
    float_controls_properties.pNext = static_cast<void*>(&protected_memory_properties);

    VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservative_rasterization_properties{};
    conservative_rasterization_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;
    conservative_rasterization_properties.pNext = static_cast<void*>(&float_controls_properties);

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracing_pipeline_properties{};
    raytracing_pipeline_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
    raytracing_pipeline_properties.pNext = static_cast<void*>(&conservative_rasterization_properties);

    VkPhysicalDeviceRayTracingPropertiesNV ray_tracing_properties{};
    ray_tracing_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
    ray_tracing_properties.pNext = static_cast<void*>(&raytracing_pipeline_properties);

    VkPhysicalDeviceTexelBufferAlignmentProperties texel_buffer_alignment_properties{};
    texel_buffer_alignment_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES;
    texel_buffer_alignment_properties.pNext = static_cast<void*>(&ray_tracing_properties);

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties{};
    descriptor_buffer_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;
    descriptor_buffer_properties.pNext = static_cast<void*>(&texel_buffer_alignment_properties);

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties{};
    mesh_shader_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;
    mesh_shader_properties.pNext = static_cast<void*>(&descriptor_buffer_properties);

    VkPhysicalDeviceFragmentDensityMap2PropertiesEXT fragment_density_map2_properties{};
    fragment_density_map2_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_PROPERTIES_EXT;
    fragment_density_map2_properties.pNext = static_cast<void*>(&mesh_shader_properties);

    VkPhysicalDeviceDriverProperties driver_properties{};
    driver_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
    driver_properties.pNext = static_cast<void*>(&fragment_density_map2_properties);

    VkPhysicalDeviceProperties2 properties2{};
    properties2.pNext = static_cast<void*>(&driver_properties);
    vkGetPhysicalDeviceProperties2(physical_device, &properties2);
    ASSERT_EQ(properties2.properties.apiVersion, VK_HEADER_VERSION_COMPLETE);
    ASSERT_EQ(properties2.properties.driverVersion, 1);
    ASSERT_EQ(properties2.properties.vendorID, 0xba5eba11);
    ASSERT_EQ(properties2.properties.deviceID, 0xf005ba11);
    ASSERT_EQ(properties2.properties.deviceType, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
    ASSERT_STREQ(&properties2.properties.deviceName[0], "Vulkan Mock Device");
    ASSERT_EQ(properties2.properties.pipelineCacheUUID[0], 18);
    ASSERT_EQ(properties2.properties.limits.maxImageDimension1D, 4096);
    ASSERT_EQ(properties2.properties.limits.nonCoherentAtomSize, 256);
    ASSERT_EQ(properties2.properties.sparseProperties.residencyAlignedMipSize, VK_TRUE);
    ASSERT_EQ(properties2.properties.sparseProperties.residencyNonResidentStrict, VK_TRUE);
    ASSERT_EQ(properties2.properties.sparseProperties.residencyStandard2DBlockShape, VK_TRUE);
    ASSERT_EQ(properties2.properties.sparseProperties.residencyStandard2DMultisampleBlockShape, VK_TRUE);
    ASSERT_EQ(properties2.properties.sparseProperties.residencyStandard3DBlockShape, VK_TRUE);

    ASSERT_EQ(properties11.protectedNoFault, VK_FALSE);
    ASSERT_EQ(properties12.denormBehaviorIndependence, VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL);
    ASSERT_EQ(properties12.roundingModeIndependence, VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL);
    ASSERT_EQ(properties13.storageTexelBufferOffsetSingleTexelAlignment, VK_TRUE);
    ASSERT_EQ(properties13.uniformTexelBufferOffsetSingleTexelAlignment, VK_TRUE);
    ASSERT_EQ(properties13.storageTexelBufferOffsetAlignmentBytes, 16);
    ASSERT_EQ(properties13.uniformTexelBufferOffsetAlignmentBytes, 16);
    ASSERT_EQ(protected_memory_properties.protectedNoFault, VK_FALSE);
    ASSERT_EQ(float_controls_properties.denormBehaviorIndependence, VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL);
    ASSERT_EQ(float_controls_properties.roundingModeIndependence, VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL);
    ASSERT_EQ(conservative_rasterization_properties.primitiveOverestimationSize, 0.00195313f);
    ASSERT_EQ(conservative_rasterization_properties.conservativePointAndLineRasterization, VK_TRUE);
    ASSERT_EQ(conservative_rasterization_properties.degenerateTrianglesRasterized, VK_TRUE);
    ASSERT_EQ(conservative_rasterization_properties.degenerateLinesRasterized, VK_TRUE);
    ASSERT_EQ(raytracing_pipeline_properties.shaderGroupHandleSize, 32);
    ASSERT_EQ(raytracing_pipeline_properties.shaderGroupBaseAlignment, 64);
    ASSERT_EQ(raytracing_pipeline_properties.shaderGroupHandleCaptureReplaySize, 32);
    ASSERT_EQ(ray_tracing_properties.shaderGroupHandleSize, 32);
    ASSERT_EQ(ray_tracing_properties.shaderGroupBaseAlignment, 64);
    ASSERT_EQ(texel_buffer_alignment_properties.storageTexelBufferOffsetSingleTexelAlignment, VK_TRUE);
    ASSERT_EQ(texel_buffer_alignment_properties.uniformTexelBufferOffsetSingleTexelAlignment, VK_TRUE);
    ASSERT_EQ(texel_buffer_alignment_properties.storageTexelBufferOffsetAlignmentBytes, 16);
    ASSERT_EQ(texel_buffer_alignment_properties.uniformTexelBufferOffsetAlignmentBytes, 16);
    ASSERT_EQ(descriptor_buffer_properties.combinedImageSamplerDescriptorSingleArray, VK_TRUE);
    ASSERT_EQ(descriptor_buffer_properties.bufferlessPushDescriptors, VK_TRUE);
    ASSERT_EQ(descriptor_buffer_properties.allowSamplerImageViewPostSubmitCreation, VK_TRUE);
    ASSERT_EQ(descriptor_buffer_properties.descriptorBufferOffsetAlignment, 4);
    ASSERT_EQ(mesh_shader_properties.meshOutputPerVertexGranularity, 32);
    ASSERT_EQ(mesh_shader_properties.meshOutputPerPrimitiveGranularity, 32);
    ASSERT_EQ(mesh_shader_properties.prefersLocalInvocationVertexOutput, VK_TRUE);
    ASSERT_EQ(mesh_shader_properties.prefersLocalInvocationPrimitiveOutput, VK_TRUE);
    ASSERT_EQ(mesh_shader_properties.prefersCompactVertexOutput, VK_TRUE);
    ASSERT_EQ(mesh_shader_properties.prefersCompactPrimitiveOutput, VK_TRUE);
    ASSERT_EQ(fragment_density_map2_properties.subsampledLoads, VK_FALSE);
    ASSERT_EQ(fragment_density_map2_properties.subsampledCoarseReconstructionEarlyAccess, VK_FALSE);
    ASSERT_EQ(fragment_density_map2_properties.maxSubsampledArrayLayers, 2);
    ASSERT_EQ(fragment_density_map2_properties.maxDescriptorSetSubsampledSamplers, 1);
    ASSERT_EQ(std::string(driver_properties.driverName), "Vulkan Mock Device");
    ASSERT_EQ(std::string(driver_properties.driverInfo), "Branch: " GIT_BRANCH_NAME " Tag Info: " GIT_TAG_INFO);
}

TEST_F(MockICD, vkGetPhysicalDeviceExternalSemaphoreProperties) {
    VkPhysicalDeviceExternalSemaphoreInfo external_semaphore_info{};
    VkExternalSemaphoreProperties external_semaphore_properties{};
    vkGetPhysicalDeviceExternalSemaphoreProperties(physical_device, &external_semaphore_info, &external_semaphore_properties);
    ASSERT_EQ(external_semaphore_properties.exportFromImportedHandleTypes, 0x1F);
    ASSERT_EQ(external_semaphore_properties.compatibleHandleTypes, 0x1F);
    ASSERT_EQ(external_semaphore_properties.externalSemaphoreFeatures, 0x3);
}

TEST_F(MockICD, vkGetPhysicalDeviceExternalFenceProperties) {
    VkPhysicalDeviceExternalFenceInfo external_fence_info{};
    VkExternalFenceProperties external_fence_properties{};
    vkGetPhysicalDeviceExternalFenceProperties(physical_device, &external_fence_info, &external_fence_properties);
    ASSERT_EQ(external_fence_properties.exportFromImportedHandleTypes, 0xF);
    ASSERT_EQ(external_fence_properties.compatibleHandleTypes, 0xF);
    ASSERT_EQ(external_fence_properties.externalFenceFeatures, 0x3);
}
TEST_F(MockICD, vkGetPhysicalDeviceExternalBufferProperties) {
    VkPhysicalDeviceExternalBufferInfo external_buffer_info{};
    external_buffer_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    VkExternalBufferProperties external_buffer_properties{};
    vkGetPhysicalDeviceExternalBufferProperties(physical_device, &external_buffer_info, &external_buffer_properties);
    ASSERT_EQ(external_buffer_properties.externalMemoryProperties.externalMemoryFeatures, 0x7);
    ASSERT_EQ(external_buffer_properties.externalMemoryProperties.exportFromImportedHandleTypes, 0x1FF);
    ASSERT_EQ(external_buffer_properties.externalMemoryProperties.compatibleHandleTypes, 0x1FF);

    external_buffer_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_SCREEN_BUFFER_BIT_QNX;
    vkGetPhysicalDeviceExternalBufferProperties(physical_device, &external_buffer_info, &external_buffer_properties);
    ASSERT_EQ(external_buffer_properties.externalMemoryProperties.externalMemoryFeatures, 0);
    ASSERT_EQ(external_buffer_properties.externalMemoryProperties.exportFromImportedHandleTypes, 0);
    ASSERT_EQ(external_buffer_properties.externalMemoryProperties.compatibleHandleTypes,
              VK_EXTERNAL_MEMORY_HANDLE_TYPE_SCREEN_BUFFER_BIT_QNX);
}

/*
 * Exercises the following commands:
 * vkCreateBuffer
 * vkGetBufferMemoryRequirements
 * vkGetBufferMemoryRequirements2
 * vkGetDeviceBufferMemoryRequirements
 * vkAllocateMemory
 * vkMapMemory
 * vkUnmapMemory
 * vkGetBufferDeviceAddress
 * vkGetBufferDeviceAddressKHR
 * vkGetBufferDeviceAddressEXT
 * vkDestroyBuffer
 * vkFreeMemory
 */
TEST_F(MockICD, BufferOperations) {
    VkResult res = VK_SUCCESS;

    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.size = 128;
    VkBuffer buffer{};
    res = vkCreateBuffer(device, &buffer_create_info, nullptr, &buffer);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(buffer, VK_NULL_HANDLE);

    VkMemoryRequirements memory_requirements{};
    vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);
    ASSERT_EQ(memory_requirements.size, 4096);
    ASSERT_EQ(memory_requirements.alignment, 1);
    ASSERT_EQ(memory_requirements.memoryTypeBits, 0xFFFF);

    VkBufferMemoryRequirementsInfo2 memory_requirements_info2{};
    VkMemoryRequirements2 memory_requirements2{};
    vkGetBufferMemoryRequirements2(device, &memory_requirements_info2, &memory_requirements2);
    ASSERT_EQ(memory_requirements2.memoryRequirements.size, 4096);
    ASSERT_EQ(memory_requirements2.memoryRequirements.alignment, 1);
    ASSERT_EQ(memory_requirements2.memoryRequirements.memoryTypeBits, 0xFFFF);

    VkDeviceBufferMemoryRequirements buffer_memory_requirements{};
    buffer_memory_requirements.pCreateInfo = &buffer_create_info;
    vkGetDeviceBufferMemoryRequirements(device, &buffer_memory_requirements, &memory_requirements2);
    ASSERT_EQ(memory_requirements2.memoryRequirements.size, 4096);
    ASSERT_EQ(memory_requirements2.memoryRequirements.alignment, 1);
    ASSERT_EQ(memory_requirements2.memoryRequirements.memoryTypeBits, 0xFFFF);

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.allocationSize = memory_requirements.size;
    VkDeviceMemory memory{};
    res = vkAllocateMemory(device, &allocate_info, nullptr, &memory);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(memory, VK_NULL_HANDLE);

    std::array<uint32_t, 32> source_data;
    void* data = nullptr;
    res = vkMapMemory(device, memory, 0, memory_requirements.size, 0, &data);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(data, nullptr);
    memcpy(data, source_data.data(), source_data.size());
    vkUnmapMemory(device, memory);

    VkBufferDeviceAddressInfo buffer_device_address_info{};
    buffer_device_address_info.buffer = buffer;
    VkDeviceAddress device_address = vkGetBufferDeviceAddress(device, &buffer_device_address_info);
    ASSERT_NE(device_address, 0);

    auto vkGetBufferDeviceAddressEXT =
        reinterpret_cast<PFN_vkGetBufferDeviceAddressEXT>(vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressEXT"));
    ASSERT_NE(vkGetBufferDeviceAddressEXT, nullptr);
    device_address = vkGetBufferDeviceAddressEXT(device, &buffer_device_address_info);
    ASSERT_NE(device_address, 0);

    auto vkGetBufferDeviceAddressKHR =
        reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR"));
    ASSERT_NE(vkGetBufferDeviceAddressKHR, nullptr);
    device_address = vkGetBufferDeviceAddressKHR(device, &buffer_device_address_info);
    ASSERT_NE(device_address, 0);

    vkDestroyBuffer(device, buffer, nullptr);
    vkFreeMemory(device, memory, nullptr);
}

/*
 * Exercises the following commands:
 * vkCreateImage
 * vkGetImageSubresourceLayout
 * vkGetImageMemoryRequirements
 * vkGetImageMemoryRequirements2
 * vkGetDeviceImageMemoryRequirements
 * vkGetImageSparseMemoryRequirements
 * vkGetImageSparseMemoryRequirements2
 * vkAllocateMemory
 * vkMapMemory2KHR
 * vkUnmapMemory2KHR
 * vkDestroyImage
 * vkFreeMemory
 */
TEST_F(MockICD, ImageOperations) {
    VkResult res = VK_SUCCESS;

    VkImageCreateInfo image_create_info{};
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    image_create_info.extent = {8, 8, 8};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = nullptr;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImage image{};
    res = vkCreateImage(device, &image_create_info, nullptr, &image);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(image, VK_NULL_HANDLE);

    VkImageSubresource image_subresource{};
    VkSubresourceLayout subresource_layout{};
    vkGetImageSubresourceLayout(device, image, &image_subresource, &subresource_layout);
    ASSERT_EQ(subresource_layout.arrayPitch, 0);
    ASSERT_EQ(subresource_layout.depthPitch, 0);
    ASSERT_EQ(subresource_layout.offset, 0);
    ASSERT_EQ(subresource_layout.rowPitch, 0);
    ASSERT_EQ(subresource_layout.size, 0);

    VkMemoryRequirements memory_requirements{};
    vkGetImageMemoryRequirements(device, image, &memory_requirements);
    ASSERT_EQ(memory_requirements.size, 8 * 8 * 8 * 32 /*refer to GetImageSizeFromCreateInfo for size calc*/);
    ASSERT_EQ(memory_requirements.alignment, 1);
    ASSERT_EQ(memory_requirements.memoryTypeBits, 0xFFFF & ~(0x1 << 3));

    VkImageMemoryRequirementsInfo2 memory_requirements_info2{};
    VkMemoryRequirements2 memory_requirements2{};
    vkGetImageMemoryRequirements2(device, &memory_requirements_info2, &memory_requirements2);
    ASSERT_EQ(memory_requirements2.memoryRequirements.size, 0);
    ASSERT_EQ(memory_requirements2.memoryRequirements.alignment, 1);
    ASSERT_EQ(memory_requirements2.memoryRequirements.memoryTypeBits, 0xFFFF & ~(0x1 << 3));

    VkDeviceImageMemoryRequirements image_memory_requirements{};
    image_memory_requirements.pCreateInfo = &image_create_info;
    vkGetDeviceImageMemoryRequirements(device, &image_memory_requirements, &memory_requirements2);
    ASSERT_EQ(memory_requirements2.memoryRequirements.size, 8 * 8 * 8 * 32 /*refer to GetImageSizeFromCreateInfo for size calc*/);
    ASSERT_EQ(memory_requirements2.memoryRequirements.alignment, 1);
    ASSERT_EQ(memory_requirements2.memoryRequirements.memoryTypeBits, 0xFFFF & ~(0x1 << 3));

    uint32_t count = 0;
    vkGetImageSparseMemoryRequirements(device, image, &count, nullptr);
    ASSERT_EQ(count, 1);
    VkSparseImageMemoryRequirements sparse_image_memory_requirements{};
    vkGetImageSparseMemoryRequirements(device, image, &count, &sparse_image_memory_requirements);
    ASSERT_EQ(count, 1);
    ASSERT_EQ(sparse_image_memory_requirements.imageMipTailFirstLod, 0);
    ASSERT_EQ(sparse_image_memory_requirements.imageMipTailSize, 8);
    ASSERT_EQ(sparse_image_memory_requirements.imageMipTailOffset, 0);
    ASSERT_EQ(sparse_image_memory_requirements.imageMipTailStride, 4);
    ASSERT_EQ(sparse_image_memory_requirements.formatProperties.imageGranularity.width, 4);
    ASSERT_EQ(sparse_image_memory_requirements.formatProperties.imageGranularity.height, 4);
    ASSERT_EQ(sparse_image_memory_requirements.formatProperties.imageGranularity.depth, 4);
    ASSERT_EQ(sparse_image_memory_requirements.formatProperties.flags, VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT);
    ASSERT_EQ(sparse_image_memory_requirements.formatProperties.aspectMask, 1 | 2 | 4 | 8);

    count = 0;
    VkImageSparseMemoryRequirementsInfo2 sparse_memory_requirement_info2{};
    sparse_memory_requirement_info2.image = image;
    vkGetImageSparseMemoryRequirements2(device, &sparse_memory_requirement_info2, &count, nullptr);
    ASSERT_EQ(count, 1);
    VkSparseImageMemoryRequirements2 sparse_image_memory_reqs2{};
    vkGetImageSparseMemoryRequirements2(device, &sparse_memory_requirement_info2, &count, &sparse_image_memory_reqs2);
    ASSERT_EQ(sparse_image_memory_reqs2.memoryRequirements.imageMipTailFirstLod, 0);
    ASSERT_EQ(sparse_image_memory_reqs2.memoryRequirements.imageMipTailSize, 8);
    ASSERT_EQ(sparse_image_memory_reqs2.memoryRequirements.imageMipTailOffset, 0);
    ASSERT_EQ(sparse_image_memory_reqs2.memoryRequirements.imageMipTailStride, 4);
    ASSERT_EQ(sparse_image_memory_reqs2.memoryRequirements.formatProperties.imageGranularity.width, 4);
    ASSERT_EQ(sparse_image_memory_reqs2.memoryRequirements.formatProperties.imageGranularity.height, 4);
    ASSERT_EQ(sparse_image_memory_reqs2.memoryRequirements.formatProperties.imageGranularity.depth, 4);
    ASSERT_EQ(sparse_image_memory_reqs2.memoryRequirements.formatProperties.flags, VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT);
    ASSERT_EQ(sparse_image_memory_reqs2.memoryRequirements.formatProperties.aspectMask, 1 | 2 | 4 | 8);

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.allocationSize = memory_requirements.size;
    VkDeviceMemory memory{};
    res = vkAllocateMemory(device, &allocate_info, nullptr, &memory);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(memory, VK_NULL_HANDLE);

    auto vkMapMemory2KHR = reinterpret_cast<PFN_vkMapMemory2KHR>(vkGetDeviceProcAddr(device, "vkMapMemory2KHR"));
    auto vkUnmapMemory2KHR = reinterpret_cast<PFN_vkUnmapMemory2KHR>(vkGetDeviceProcAddr(device, "vkUnmapMemory2KHR"));
    ASSERT_NE(vkMapMemory2KHR, nullptr);
    ASSERT_NE(vkUnmapMemory2KHR, nullptr);

    std::array<uint32_t, 32> source_data;
    void* data = nullptr;
    VkMemoryMapInfoKHR memory_map_info{};
    memory_map_info.memory = memory;
    memory_map_info.size = memory_requirements.size;
    res = vkMapMemory2KHR(device, &memory_map_info, &data);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(data, nullptr);
    memcpy(data, source_data.data(), source_data.size());
    VkMemoryUnmapInfoKHR memory_unmap_info{};
    memory_unmap_info.memory = memory;
    vkUnmapMemory2KHR(device, &memory_unmap_info);

    vkDestroyImage(device, image, nullptr);
    vkFreeMemory(device, memory, nullptr);
}

/*
 * Exercises the following commands:
 * vkCreateSwapchainKHR
 * vkGetSwapchainImagesKHR
 * vkDestroySwapchainKHR
 * vkAcquireNextImageKHR
 * vkAcquireNextImage2KHR
 */
TEST_F(MockICD, SwapchainLifeCycle) {
    VkResult res = VK_SUCCESS;
    VkSurfaceKHR surface{};
    res = create_surface(instance, surface);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(surface, VK_NULL_HANDLE);

    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = 1;
    VkSwapchainKHR swapchain{};
    res = vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(swapchain, VK_NULL_HANDLE);

    uint32_t count = 0;
    res = vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, 1);
    std::array<VkImage, 1> swapchain_images;
    res = vkGetSwapchainImagesKHR(device, swapchain, &count, swapchain_images.data());
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_NE(swapchain_images[0], VK_NULL_HANDLE);

    uint32_t image_index = 10;  // arbitrary non zero value
    res = vkAcquireNextImageKHR(device, swapchain, 0, VK_NULL_HANDLE, VK_NULL_HANDLE, &image_index);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(image_index, 0);

    VkAcquireNextImageInfoKHR acquire_info{};
    acquire_info.swapchain = swapchain;
    res = vkAcquireNextImage2KHR(device, &acquire_info, &image_index);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(image_index, 0);

    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
}

TEST_F(MockICD, vkGetPhysicalDeviceMultisamplePropertiesEXT) {
    auto vkGetPhysicalDeviceMultisamplePropertiesEXT = reinterpret_cast<PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT>(
        vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMultisamplePropertiesEXT"));
    ASSERT_NE(vkGetPhysicalDeviceMultisamplePropertiesEXT, nullptr);
    VkMultisamplePropertiesEXT multisample_properties{};
    vkGetPhysicalDeviceMultisamplePropertiesEXT(physical_device, VK_SAMPLE_COUNT_16_BIT, &multisample_properties);
    ASSERT_EQ(multisample_properties.maxSampleLocationGridSize.width, 32);
    ASSERT_EQ(multisample_properties.maxSampleLocationGridSize.height, 32);
}

TEST_F(MockICD, vkGetPhysicalDeviceFragmentShadingRatesKHR) {
    auto vkGetPhysicalDeviceFragmentShadingRatesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR>(
        vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFragmentShadingRatesKHR"));
    ASSERT_NE(vkGetPhysicalDeviceFragmentShadingRatesKHR, nullptr);

    VkResult res = VK_SUCCESS;
    uint32_t count = 0;
    res = vkGetPhysicalDeviceFragmentShadingRatesKHR(physical_device, &count, nullptr);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, 1);

    VkPhysicalDeviceFragmentShadingRateKHR fragment_shading_rates{};
    res = vkGetPhysicalDeviceFragmentShadingRatesKHR(physical_device, &count, &fragment_shading_rates);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, 1);
    ASSERT_EQ(fragment_shading_rates.sampleCounts, VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT);
    ASSERT_EQ(fragment_shading_rates.fragmentSize.width, 8);
    ASSERT_EQ(fragment_shading_rates.fragmentSize.height, 8);
}

TEST_F(MockICD, vkGetPhysicalDeviceCalibrateableTimeDomainsEXT) {
    auto vkGetPhysicalDeviceCalibrateableTimeDomainsEXT = reinterpret_cast<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT>(
        vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT"));
    ASSERT_NE(vkGetPhysicalDeviceCalibrateableTimeDomainsEXT, nullptr);

    VkResult res = VK_SUCCESS;
    uint32_t count = 0;
    res = vkGetPhysicalDeviceCalibrateableTimeDomainsEXT(physical_device, &count, nullptr);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, 1);

    VkTimeDomainEXT time_domain{};
    res = vkGetPhysicalDeviceCalibrateableTimeDomainsEXT(physical_device, &count, &time_domain);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, 1);
    ASSERT_EQ(time_domain, VK_TIME_DOMAIN_DEVICE_EXT);
}

#if defined(WIN32)
TEST_F(MockICD, vkGetFenceWin32HandleKHR) {
    auto vkGetFenceWin32HandleKHR =
        reinterpret_cast<PFN_vkGetFenceWin32HandleKHR>(vkGetDeviceProcAddr(device, "vkGetFenceWin32HandleKHR"));
    ASSERT_NE(vkGetFenceWin32HandleKHR, nullptr);
    VkFenceGetWin32HandleInfoKHR get_win32_handle_info{};
    HANDLE handle{};
    VkResult res = vkGetFenceWin32HandleKHR(device, &get_win32_handle_info, &handle);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(handle, (HANDLE)0x12345678);
}
#endif  // defined(WIN32)

TEST_F(MockICD, vkGetFenceFdKHR) {
    auto vkGetFenceFdKHR = reinterpret_cast<PFN_vkGetFenceFdKHR>(vkGetDeviceProcAddr(device, "vkGetFenceFdKHR"));
    ASSERT_NE(vkGetFenceFdKHR, nullptr);
    VkFenceGetFdInfoKHR get_win32_handle_info{};
    int handle{};
    VkResult res = vkGetFenceFdKHR(device, &get_win32_handle_info, &handle);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(handle, 0x42);
}

TEST_F(MockICD, vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR) {
    auto vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
        reinterpret_cast<PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR>(
            vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR"));
    ASSERT_NE(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR, nullptr);

    VkResult res = VK_SUCCESS;
    uint32_t count = 0;
    res = vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physical_device, 0, &count, nullptr, nullptr);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, 3);

    std::array<VkPerformanceCounterKHR, 3> counters{};
    std::array<VkPerformanceCounterDescriptionKHR, 3> counter_descriptions{};
    res = vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physical_device, 0, &count, counters.data(),
                                                                          counter_descriptions.data());
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, 3);
    ASSERT_EQ(counters[0].unit, VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR);
    ASSERT_EQ(counters[0].scope, VK_QUERY_SCOPE_COMMAND_BUFFER_KHR);
    ASSERT_EQ(counters[0].storage, VK_PERFORMANCE_COUNTER_STORAGE_INT32_KHR);
    ASSERT_EQ(counters[0].uuid[0], 0x01);
    ASSERT_EQ(counters[1].unit, VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR);
    ASSERT_EQ(counters[1].scope, VK_QUERY_SCOPE_RENDER_PASS_KHR);
    ASSERT_EQ(counters[1].storage, VK_PERFORMANCE_COUNTER_STORAGE_INT32_KHR);
    ASSERT_EQ(counters[1].uuid[0], 0x02);
    ASSERT_EQ(counters[2].unit, VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR);
    ASSERT_EQ(counters[2].scope, VK_QUERY_SCOPE_COMMAND_KHR);
    ASSERT_EQ(counters[2].storage, VK_PERFORMANCE_COUNTER_STORAGE_INT32_KHR);
    ASSERT_EQ(counters[2].uuid[0], 0x03);
}

TEST_F(MockICD, vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR) {
    auto vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR =
        reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR>(
            vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR"));
    ASSERT_NE(vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR, nullptr);
    VkQueryPoolPerformanceCreateInfoKHR performance_query_create_info{};
    uint32_t num_passes = 0;
    vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physical_device, &performance_query_create_info, &num_passes);
    ASSERT_EQ(num_passes, 1);
}

TEST_F(MockICD, vkGetShaderModuleIdentifierEXT) {
    auto vkGetShaderModuleIdentifierEXT =
        reinterpret_cast<PFN_vkGetShaderModuleIdentifierEXT>(vkGetDeviceProcAddr(device, "vkGetShaderModuleIdentifierEXT"));
    ASSERT_NE(vkGetShaderModuleIdentifierEXT, nullptr);
    VkShaderModule shader_module{};
    VkShaderModuleIdentifierEXT identifier{};
    vkGetShaderModuleIdentifierEXT(device, shader_module, &identifier);
    ASSERT_EQ(identifier.identifierSize, 1);
    ASSERT_EQ(identifier.identifier[0], 0x01);
}

TEST_F(MockICD, vkGetDescriptorSetLayoutSizeEXT) {
    auto vkGetDescriptorSetLayoutSizeEXT =
        reinterpret_cast<PFN_vkGetDescriptorSetLayoutSizeEXT>(vkGetDeviceProcAddr(device, "vkGetDescriptorSetLayoutSizeEXT"));
    ASSERT_NE(vkGetDescriptorSetLayoutSizeEXT, nullptr);

    VkDescriptorSetLayout layout{};
    VkDeviceSize layout_size_in_bytes = 0;
    vkGetDescriptorSetLayoutSizeEXT(device, layout, &layout_size_in_bytes);
    ASSERT_EQ(layout_size_in_bytes, 4);
}

TEST_F(MockICD, vkGetAccelerationStructureBuildSizesKHR) {
    auto vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
        vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));
    ASSERT_NE(vkGetAccelerationStructureBuildSizesKHR, nullptr);

    VkAccelerationStructureBuildGeometryInfoKHR build_info{};
    uint32_t max_primitive_count = 0;
    VkAccelerationStructureBuildSizesInfoKHR size_info{};
    vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info,
                                            &max_primitive_count, &size_info);
    ASSERT_EQ(size_info.accelerationStructureSize, 4);
    ASSERT_EQ(size_info.updateScratchSize, 4);
    ASSERT_EQ(size_info.buildScratchSize, 4);
}

TEST_F(MockICD, vkGetAccelerationStructureMemoryRequirementsNV) {
    auto vkGetAccelerationStructureMemoryRequirementsNV = reinterpret_cast<PFN_vkGetAccelerationStructureMemoryRequirementsNV>(
        vkGetDeviceProcAddr(device, "vkGetAccelerationStructureMemoryRequirementsNV"));
    ASSERT_NE(vkGetAccelerationStructureMemoryRequirementsNV, nullptr);

    VkAccelerationStructureMemoryRequirementsInfoNV acceleration_structure_memory_requirements_info{};
    VkMemoryRequirements2KHR memory_requirements{};
    vkGetAccelerationStructureMemoryRequirementsNV(device, &acceleration_structure_memory_requirements_info, &memory_requirements);
    ASSERT_EQ(memory_requirements.memoryRequirements.size, 4096);
    ASSERT_EQ(memory_requirements.memoryRequirements.alignment, 1);
    ASSERT_EQ(memory_requirements.memoryRequirements.memoryTypeBits, 0xFFFF);
}

TEST_F(MockICD, vkGetVideoSessionMemoryRequirementsKHR) {
    auto vkGetVideoSessionMemoryRequirementsKHR = reinterpret_cast<PFN_vkGetVideoSessionMemoryRequirementsKHR>(
        vkGetDeviceProcAddr(device, "vkGetVideoSessionMemoryRequirementsKHR"));
    ASSERT_NE(vkGetVideoSessionMemoryRequirementsKHR, nullptr);

    VkVideoSessionKHR video_session{};
    uint32_t count = 0;
    VkResult res = vkGetVideoSessionMemoryRequirementsKHR(device, video_session, &count, nullptr);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, 1);
    VkVideoSessionMemoryRequirementsKHR memory_requirements{};
    res = vkGetVideoSessionMemoryRequirementsKHR(device, video_session, &count, &memory_requirements);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, 1);
    ASSERT_EQ(memory_requirements.memoryBindIndex, 0);
    ASSERT_EQ(memory_requirements.memoryRequirements.size, 4096);
    ASSERT_EQ(memory_requirements.memoryRequirements.alignment, 1);
    ASSERT_EQ(memory_requirements.memoryRequirements.memoryTypeBits, 0xFFFF);
}

TEST_F(MockICD, vkGetPhysicalDeviceVideoFormatPropertiesKHR) {
    auto vkGetPhysicalDeviceVideoFormatPropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR>(
        vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceVideoFormatPropertiesKHR"));
    ASSERT_NE(vkGetPhysicalDeviceVideoFormatPropertiesKHR, nullptr);

    VkVideoDecodeH264ProfileInfoKHR decode_h264_profile_info{};
    decode_h264_profile_info.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR;
    decode_h264_profile_info.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_BASELINE;
    decode_h264_profile_info.pictureLayout = VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR;
    VkVideoProfileInfoKHR video_profile_info{};
    video_profile_info.sType = VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR;
    video_profile_info.pNext = &decode_h264_profile_info;
    video_profile_info.videoCodecOperation = VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR;
    video_profile_info.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR;
    video_profile_info.lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;
    video_profile_info.chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;
    VkVideoProfileListInfoKHR video_profile_list{};
    video_profile_list.sType = VK_STRUCTURE_TYPE_VIDEO_PROFILE_LIST_INFO_KHR;
    video_profile_list.profileCount = 1;
    video_profile_list.pProfiles = &video_profile_info;
    VkPhysicalDeviceVideoFormatInfoKHR video_format_info{};
    video_format_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_FORMAT_INFO_KHR;
    video_format_info.pNext = &video_profile_list;
    uint32_t count = 0;
    VkResult res = vkGetPhysicalDeviceVideoFormatPropertiesKHR(physical_device, &video_format_info, &count, nullptr);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, 3);
    std::array<VkVideoFormatPropertiesKHR, 3> video_format_properties{};
    res = vkGetPhysicalDeviceVideoFormatPropertiesKHR(physical_device, &video_format_info, &count, video_format_properties.data());
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(count, 3);
    ASSERT_EQ(video_format_properties[0].format, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM);
    ASSERT_EQ(video_format_properties[0].imageCreateFlags, VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_ALIAS_BIT |
                                                               VK_IMAGE_CREATE_EXTENDED_USAGE_BIT | VK_IMAGE_CREATE_PROTECTED_BIT |
                                                               VK_IMAGE_CREATE_DISJOINT_BIT);
    ASSERT_EQ(video_format_properties[0].imageType, VK_IMAGE_TYPE_2D);
    ASSERT_EQ(video_format_properties[0].imageTiling, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_EQ(video_format_properties[0].imageUsageFlags,
              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR);
    ASSERT_EQ(video_format_properties[1].format, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM);
    ASSERT_EQ(video_format_properties[1].imageCreateFlags, VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_ALIAS_BIT |
                                                               VK_IMAGE_CREATE_EXTENDED_USAGE_BIT | VK_IMAGE_CREATE_PROTECTED_BIT |
                                                               VK_IMAGE_CREATE_DISJOINT_BIT);
    ASSERT_EQ(video_format_properties[1].imageType, VK_IMAGE_TYPE_2D);
    ASSERT_EQ(video_format_properties[1].imageTiling, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_EQ(video_format_properties[1].imageUsageFlags,
              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                  VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR |
                  VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR);
    ASSERT_EQ(video_format_properties[2].format, VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM);
    ASSERT_EQ(video_format_properties[2].imageCreateFlags, VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_ALIAS_BIT |
                                                               VK_IMAGE_CREATE_EXTENDED_USAGE_BIT | VK_IMAGE_CREATE_PROTECTED_BIT |
                                                               VK_IMAGE_CREATE_DISJOINT_BIT);
    ASSERT_EQ(video_format_properties[2].imageType, VK_IMAGE_TYPE_2D);
    ASSERT_EQ(video_format_properties[2].imageTiling, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_EQ(video_format_properties[2].imageUsageFlags,
              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                  VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR |
                  VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR);
}

TEST_F(MockICD, vkGetPhysicalDeviceVideoCapabilitiesKHR) {
    auto vkGetPhysicalDeviceVideoCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR>(
        vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceVideoCapabilitiesKHR"));
    ASSERT_NE(vkGetPhysicalDeviceVideoCapabilitiesKHR, nullptr);

    VkVideoDecodeH264ProfileInfoKHR decode_h264_profile_info{};
    decode_h264_profile_info.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR;
    decode_h264_profile_info.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_BASELINE;
    decode_h264_profile_info.pictureLayout = VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR;
    VkVideoProfileInfoKHR video_profile_info{};
    video_profile_info.sType = VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR;
    video_profile_info.pNext = &decode_h264_profile_info;
    video_profile_info.videoCodecOperation = VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR;
    video_profile_info.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR;
    video_profile_info.lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;
    video_profile_info.chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;
    VkVideoDecodeH264CapabilitiesKHR decode_h264_capabilities{};
    decode_h264_capabilities.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR;
    VkVideoDecodeCapabilitiesKHR decode_capabilities{};
    decode_capabilities.sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR;
    decode_capabilities.pNext = &decode_h264_capabilities;
    VkVideoCapabilitiesKHR video_capabilities{};
    video_capabilities.sType = VK_STRUCTURE_TYPE_VIDEO_CAPABILITIES_KHR;
    video_capabilities.pNext = &decode_capabilities;
    VkResult res = vkGetPhysicalDeviceVideoCapabilitiesKHR(physical_device, &video_profile_info, &video_capabilities);
    ASSERT_EQ(res, VK_SUCCESS);
    ASSERT_EQ(video_capabilities.flags, VK_VIDEO_CAPABILITY_PROTECTED_CONTENT_BIT_KHR);
    ASSERT_EQ(video_capabilities.minBitstreamBufferOffsetAlignment, 256);
    ASSERT_EQ(video_capabilities.minBitstreamBufferSizeAlignment, 256);
    ASSERT_EQ(video_capabilities.pictureAccessGranularity.width, 16);
    ASSERT_EQ(video_capabilities.pictureAccessGranularity.height, 16);
    ASSERT_EQ(video_capabilities.minCodedExtent.width, 16);
    ASSERT_EQ(video_capabilities.minCodedExtent.height, 16);
    ASSERT_EQ(video_capabilities.maxCodedExtent.width, 1920);
    ASSERT_EQ(video_capabilities.maxCodedExtent.height, 1080);
    ASSERT_EQ(video_capabilities.maxDpbSlots, 33);
    ASSERT_EQ(video_capabilities.maxActiveReferencePictures, 32);
    ASSERT_EQ(decode_capabilities.flags, VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR);
    ASSERT_EQ(decode_h264_capabilities.maxLevelIdc, STD_VIDEO_H264_LEVEL_IDC_6_2);
    ASSERT_EQ(decode_h264_capabilities.fieldOffsetGranularity.x, 0);
    ASSERT_EQ(decode_h264_capabilities.fieldOffsetGranularity.y, 0);
}

TEST_F(MockICD, vkGetDescriptorSetLayoutSupport) {
    VkDescriptorSetLayoutCreateInfo create_info{};
    VkDescriptorSetLayoutSupport support{};
    vkGetDescriptorSetLayoutSupport(device, &create_info, &support);
    ASSERT_EQ(support.supported, VK_TRUE);
}

TEST_F(MockICD, vkGetDescriptorSetLayoutSupportKHR) {
    auto vkGetDescriptorSetLayoutSupportKHR =
        reinterpret_cast<PFN_vkGetDescriptorSetLayoutSupportKHR>(vkGetDeviceProcAddr(device, "vkGetDescriptorSetLayoutSupportKHR"));
    ASSERT_NE(vkGetDescriptorSetLayoutSupportKHR, nullptr);

    VkDescriptorSetLayoutCreateInfo create_info{};
    VkDescriptorSetLayoutSupport support{};
    vkGetDescriptorSetLayoutSupportKHR(device, &create_info, &support);
    ASSERT_EQ(support.supported, VK_TRUE);
}

TEST_F(MockICD, vkGetRenderAreaGranularity) {
    VkRenderPass render_pass{};
    VkExtent2D granularity{};
    vkGetRenderAreaGranularity(device, render_pass, &granularity);
    ASSERT_EQ(granularity.width, 1);
    ASSERT_EQ(granularity.height, 1);
}
