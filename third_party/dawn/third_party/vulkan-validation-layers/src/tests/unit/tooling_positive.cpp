/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

class PositiveTooling : public VkLayerTest {};

TEST_F(PositiveTooling, InfoExt) {
    TEST_DESCRIPTION("Basic usage calling Tooling Extension and verify layer results.");
    AddRequiredExtensions(VK_EXT_TOOLING_INFO_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Tooling Info not supported by MockICD";
    }

    uint32_t tool_count = 0;
    auto result = vk::GetPhysicalDeviceToolPropertiesEXT(Gpu(), &tool_count, nullptr);

    if (tool_count <= 0) {
        m_errorMonitor->SetError("Expected layer tooling data but received none");
    }

    std::vector<VkPhysicalDeviceToolPropertiesEXT> tool_properties(tool_count);
    for (uint32_t i = 0; i < tool_count; i++) {
        tool_properties[i] = vku::InitStructHelper();
    }

    bool found_validation_layer = false;

    if (result == VK_SUCCESS) {
        result = vk::GetPhysicalDeviceToolPropertiesEXT(Gpu(), &tool_count, tool_properties.data());

        for (uint32_t i = 0; i < tool_count; i++) {
            if (strcmp(tool_properties[0].name, "Khronos Validation Layer") == 0) {
                found_validation_layer = true;
                break;
            }
        }
    }
    if (!found_validation_layer) {
        m_errorMonitor->SetError("Expected layer tooling data but received none");
    }
}

TEST_F(PositiveTooling, InfoCore) {
    TEST_DESCRIPTION("Basic usage calling Tooling Extension as core.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Tooling Info not supported by MockICD";
    }

    uint32_t tool_count = 0;
    auto result = vk::GetPhysicalDeviceToolProperties(Gpu(), &tool_count, nullptr);

    if (tool_count <= 0) {
        m_errorMonitor->SetError("Expected layer tooling data but received none");
    }

    std::vector<VkPhysicalDeviceToolProperties> tool_properties(tool_count);
    for (uint32_t i = 0; i < tool_count; i++) {
        tool_properties[i] = vku::InitStructHelper();
    }

    bool found_validation_layer = false;

    if (result == VK_SUCCESS) {
        result = vk::GetPhysicalDeviceToolProperties(Gpu(), &tool_count, tool_properties.data());

        for (uint32_t i = 0; i < tool_count; i++) {
            if (strcmp(tool_properties[0].name, "Khronos Validation Layer") == 0) {
                found_validation_layer = true;
                break;
            }
        }
    }
    if (!found_validation_layer) {
        m_errorMonitor->SetError("Expected layer tooling data but received none");
    }
}

TEST_F(PositiveTooling, PrivateDataExt) {
    TEST_DESCRIPTION("Basic usage calling private data extension.");
    AddRequiredExtensions(VK_EXT_PRIVATE_DATA_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::privateData);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Private data not supported by MockICD";
    }

    VkPrivateDataSlot data_slot;
    VkPrivateDataSlotCreateInfo data_create_info = vku::InitStructHelper();
    data_create_info.flags = 0;
    vk::CreatePrivateDataSlotEXT(m_device->handle(), &data_create_info, nullptr, &data_slot);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    static const uint64_t data_value = 0x70AD;
    vk::SetPrivateDataEXT(m_device->handle(), VK_OBJECT_TYPE_SAMPLER, (uint64_t)sampler.handle(), data_slot, data_value);

    uint64_t data;
    vk::GetPrivateDataEXT(m_device->handle(), VK_OBJECT_TYPE_SAMPLER, (uint64_t)sampler.handle(), data_slot, &data);
    if (data != data_value) {
        m_errorMonitor->SetError("Got unexpected private data, %s.\n");
    }
    vk::DestroyPrivateDataSlotEXT(m_device->handle(), data_slot, nullptr);
}

TEST_F(PositiveTooling, PrivateDataCore) {
    TEST_DESCRIPTION("Basic usage calling private data as core.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::privateData);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Private data not supported by MockICD";
    }

    VkPrivateDataSlot data_slot;
    VkPrivateDataSlotCreateInfo data_create_info = vku::InitStructHelper();
    data_create_info.flags = 0;
    vk::CreatePrivateDataSlot(m_device->handle(), &data_create_info, nullptr, &data_slot);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    static const uint64_t data_value = 0x70AD;
    vk::SetPrivateData(m_device->handle(), VK_OBJECT_TYPE_SAMPLER, (uint64_t)sampler.handle(), data_slot, data_value);

    uint64_t data;
    vk::GetPrivateData(m_device->handle(), VK_OBJECT_TYPE_SAMPLER, (uint64_t)sampler.handle(), data_slot, &data);
    if (data != data_value) {
        m_errorMonitor->SetError("Got unexpected private data, %s.\n");
    }
    vk::DestroyPrivateDataSlot(m_device->handle(), data_slot, nullptr);
}

TEST_F(PositiveTooling, PrivateDataDevice) {
    TEST_DESCRIPTION("Test private data can set VkDevice.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::privateData);
    RETURN_IF_SKIP(Init());

    VkPrivateDataSlot data_slot;
    VkPrivateDataSlotCreateInfo data_create_info = vku::InitStructHelper();
    vk::CreatePrivateDataSlot(m_device->handle(), &data_create_info, NULL, &data_slot);

    static const uint64_t data_value = 0x70AD;
    vk::SetPrivateData(m_device->handle(), VK_OBJECT_TYPE_DEVICE, (uint64_t)device(), data_slot, data_value);
    uint64_t data;
    vk::GetPrivateData(m_device->handle(), VK_OBJECT_TYPE_DEVICE, (uint64_t)device(), data_slot, &data);

    vk::DestroyPrivateDataSlot(m_device->handle(), data_slot, nullptr);
}
