/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

class PositiveProtectedMemory : public VkLayerTest {};

TEST_F(PositiveProtectedMemory, MixProtectedQueue) {
    TEST_DESCRIPTION("Test creating 2 queues, 1 protected, and getting both with vkGetDeviceQueue2");
    all_queue_count_ = true;
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    // Needed for both protected memory and vkGetDeviceQueue2

    VkPhysicalDeviceProtectedMemoryFeatures protected_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(protected_features);
    if (protected_features.protectedMemory == VK_FALSE) {
        GTEST_SKIP() << "test requires protectedMemory";
    }

    // Try to find a protected queue family type
    bool protected_queue = false;
    VkQueueFamilyProperties queue_properties;  // selected queue family used
    uint32_t queue_family_index = 0;
    uint32_t queue_family_count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, queue_families.data());

    for (size_t i = 0; i < queue_families.size(); i++) {
        // need to have at least 2 queues to use
        if (((queue_families[i].queueFlags & VK_QUEUE_PROTECTED_BIT) != 0) && (queue_families[i].queueCount > 1)) {
            protected_queue = true;
            queue_family_index = i;
            queue_properties = queue_families[i];
            break;
        }
    }

    if (protected_queue == false) {
        GTEST_SKIP() << "test requires queue family with VK_QUEUE_PROTECTED_BIT and 2 queues, not available.";
    }

    float queue_priority = 1.0;

    VkDeviceQueueCreateInfo queue_create_info[2];
    queue_create_info[0] = vku::InitStructHelper();
    queue_create_info[0].flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    queue_create_info[0].queueFamilyIndex = queue_family_index;
    queue_create_info[0].queueCount = 1;
    queue_create_info[0].pQueuePriorities = &queue_priority;

    queue_create_info[1] = vku::InitStructHelper();
    queue_create_info[1].flags = 0;  // unprotected because the protected flag is not set
    queue_create_info[1].queueFamilyIndex = queue_family_index;
    queue_create_info[1].queueCount = 1;
    queue_create_info[1].pQueuePriorities = &queue_priority;

    VkDevice test_device = VK_NULL_HANDLE;
    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&protected_features);
    device_create_info.flags = 0;
    device_create_info.pQueueCreateInfos = queue_create_info;
    device_create_info.queueCreateInfoCount = 2;
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.enabledLayerCount = 0;
    device_create_info.enabledExtensionCount = 0;
    ASSERT_EQ(VK_SUCCESS, vk::CreateDevice(Gpu(), &device_create_info, nullptr, &test_device));

    VkQueue test_queue_protected = VK_NULL_HANDLE;
    VkQueue test_queue_unprotected = VK_NULL_HANDLE;

    VkDeviceQueueInfo2 queue_info_2 = vku::InitStructHelper();

    queue_info_2.flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    queue_info_2.queueFamilyIndex = queue_family_index;
    queue_info_2.queueIndex = 0;
    vk::GetDeviceQueue2(test_device, &queue_info_2, &test_queue_protected);

    queue_info_2.flags = 0;
    queue_info_2.queueIndex = 0;
    vk::GetDeviceQueue2(test_device, &queue_info_2, &test_queue_unprotected);

    vk::DestroyDevice(test_device, nullptr);
}
