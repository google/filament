/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>
#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"

class PositiveDeviceFeatureProperty : public VkLayerTest {
  public:
    VkDeviceCreateInfo m_second_device_ci = vku::InitStructHelper();
    vkt::QueueCreateInfoArray *m_queue_info = nullptr;
    void InitDeviceFeatureProperty();

    ~PositiveDeviceFeatureProperty() {
        if (m_queue_info) {
            delete m_queue_info;
        }
    }
};

void PositiveDeviceFeatureProperty::InitDeviceFeatureProperty() {
    RETURN_IF_SKIP(Init());
    m_queue_info = new vkt::QueueCreateInfoArray(m_device->Physical().queue_properties_);
    m_second_device_ci.queueCreateInfoCount = m_queue_info->Size();
    m_second_device_ci.pQueueCreateInfos = m_queue_info->Data();
    m_second_device_ci.enabledExtensionCount = static_cast<uint32_t>(m_device_extension_names.size());
    m_second_device_ci.ppEnabledExtensionNames = m_device_extension_names.data();
}

TEST_F(PositiveDeviceFeatureProperty, VertexAttributeDivisor) {
    AddRequiredExtensions(VK_KHR_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());
    VkPhysicalDeviceVertexAttributeDivisorFeaturesKHR vadf = vku::InitStructHelper();
    m_second_device_ci.pNext = &vadf;
    VkDevice second_device = VK_NULL_HANDLE;
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &second_device);
    vk::DestroyDevice(second_device, nullptr);
}

TEST_F(PositiveDeviceFeatureProperty, PropertyWithoutExtension) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework());

    // query several properties which are unlikely to all be supported by the same physical device
    VkPhysicalDeviceDrmPropertiesEXT drm_props = vku::InitStructHelper();
    VkPhysicalDeviceLayeredDriverPropertiesMSFT layered_props = vku::InitStructHelper(&drm_props);

    GetPhysicalDeviceProperties2(layered_props);
}
