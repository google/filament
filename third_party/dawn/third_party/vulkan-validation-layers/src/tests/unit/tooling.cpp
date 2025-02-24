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

class NegativeTooling : public VkLayerTest {};

TEST_F(NegativeTooling, PrivateDataFeature) {
    TEST_DESCRIPTION("Test privateData feature not being enabled.");
    AddRequiredExtensions(VK_EXT_PRIVATE_DATA_EXTENSION_NAME);
    // feature not enabled
    RETURN_IF_SKIP(Init());

    bool vulkan_13 = (DeviceValidationVersion() >= VK_API_VERSION_1_3);

    VkPrivateDataSlotEXT data_slot;
    VkPrivateDataSlotCreateInfoEXT data_create_info = vku::InitStructHelper();
    data_create_info.flags = 0;
    m_errorMonitor->SetDesiredError("VUID-vkCreatePrivateDataSlot-privateData-04564");
    vk::CreatePrivateDataSlotEXT(m_device->handle(), &data_create_info, NULL, &data_slot);
    m_errorMonitor->VerifyFound();
    if (vulkan_13) {
        m_errorMonitor->SetDesiredError("VUID-vkCreatePrivateDataSlot-privateData-04564");
        vk::CreatePrivateDataSlot(m_device->handle(), &data_create_info, NULL, &data_slot);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeTooling, PrivateDataSetNonDevice) {
    TEST_DESCRIPTION("Use Private Data on a non-Device object type.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::privateData);
    RETURN_IF_SKIP(Init());

    VkPrivateDataSlot data_slot;
    VkPrivateDataSlotCreateInfo data_create_info = vku::InitStructHelper();
    vk::CreatePrivateDataSlot(m_device->handle(), &data_create_info, NULL, &data_slot);

    static const uint64_t data_value = 0x70AD;
    m_errorMonitor->SetDesiredError("VUID-vkSetPrivateData-objectHandle-04016");
    vk::SetPrivateData(m_device->handle(), VK_OBJECT_TYPE_PHYSICAL_DEVICE, (uint64_t)Gpu(), data_slot, data_value);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkSetPrivateData-objectHandle-04016");
    vk::SetPrivateData(m_device->handle(), VK_OBJECT_TYPE_UNKNOWN, (uint64_t)Gpu(), data_slot, data_value);
    m_errorMonitor->VerifyFound();

    vk::DestroyPrivateDataSlot(m_device->handle(), data_slot, nullptr);
}

TEST_F(NegativeTooling, PrivateDataSetBadHandle) {
    TEST_DESCRIPTION("Use Private Data a non valid handle.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::privateData);
    RETURN_IF_SKIP(Init());

    VkPrivateDataSlot data_slot;
    VkPrivateDataSlotCreateInfo data_create_info = vku::InitStructHelper();
    vk::CreatePrivateDataSlot(m_device->handle(), &data_create_info, NULL, &data_slot);

    static const uint64_t data_value = 0x70AD;
    m_errorMonitor->SetDesiredError("VUID-vkSetPrivateData-objectHandle-04017");
    // valid handle, but not a vkSample
    vk::SetPrivateData(m_device->handle(), VK_OBJECT_TYPE_SAMPLER, (uint64_t)device(), data_slot, data_value);
    m_errorMonitor->VerifyFound();

    vk::DestroyPrivateDataSlot(m_device->handle(), data_slot, nullptr);
}

TEST_F(NegativeTooling, PrivateDataSetSecondDevice) {
    TEST_DESCRIPTION("Test private data can set VkDevice.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::privateData);
    RETURN_IF_SKIP(Init());

    auto features = m_device->Physical().Features();
    vkt::Device second_device(gpu_, m_device_extension_names, &features, nullptr);

    VkPrivateDataSlot data_slot;
    VkPrivateDataSlotCreateInfo data_create_info = vku::InitStructHelper();
    vk::CreatePrivateDataSlot(m_device->handle(), &data_create_info, NULL, &data_slot);

    static const uint64_t data_value = 0x70AD;
    m_errorMonitor->SetDesiredError("VUID-vkSetPrivateData-objectHandle-04016");
    vk::SetPrivateData(m_device->handle(), VK_OBJECT_TYPE_DEVICE, (uint64_t)second_device.handle(), data_slot, data_value);
    m_errorMonitor->VerifyFound();

    vkt::Sampler sampler(second_device, SafeSaneSamplerCreateInfo());
    m_errorMonitor->SetDesiredError("VUID-vkSetPrivateData-objectHandle-04016");
    vk::SetPrivateData(m_device->handle(), VK_OBJECT_TYPE_SAMPLER, (uint64_t)sampler.handle(), data_slot, data_value);
    m_errorMonitor->VerifyFound();

    vk::DestroyPrivateDataSlot(m_device->handle(), data_slot, nullptr);
}

TEST_F(NegativeTooling, PrivateDataGetNonDevice) {
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

    uint64_t data;
    m_errorMonitor->SetDesiredError("VUID-vkGetPrivateData-objectType-04018");
    vk::GetPrivateData(m_device->handle(), VK_OBJECT_TYPE_PHYSICAL_DEVICE, (uint64_t)Gpu(), data_slot, &data);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkGetPrivateData-objectType-04018");
    vk::GetPrivateData(m_device->handle(), VK_OBJECT_TYPE_UNKNOWN, (uint64_t)Gpu(), data_slot, &data);
    m_errorMonitor->VerifyFound();

    vk::DestroyPrivateDataSlot(m_device->handle(), data_slot, nullptr);
}

TEST_F(NegativeTooling, PrivateDataGetBadHandle) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::privateData);
    RETURN_IF_SKIP(Init());

    VkPrivateDataSlot data_slot;
    VkPrivateDataSlotCreateInfo data_create_info = vku::InitStructHelper();
    vk::CreatePrivateDataSlot(m_device->handle(), &data_create_info, NULL, &data_slot);

    uint64_t data;
    m_errorMonitor->SetDesiredError("VUID-vkGetPrivateData-objectHandle-09498");
    // valid handle, but not a vkSample
    vk::GetPrivateData(m_device->handle(), VK_OBJECT_TYPE_SAMPLER, (uint64_t)device(), data_slot, &data);
    m_errorMonitor->VerifyFound();

    vk::DestroyPrivateDataSlot(m_device->handle(), data_slot, nullptr);
}

TEST_F(NegativeTooling, PrivateDataGetDestroyedHandle) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::privateData);
    RETURN_IF_SKIP(Init());

    VkPrivateDataSlot data_slot;
    VkPrivateDataSlotCreateInfo data_create_info = vku::InitStructHelper();
    vk::CreatePrivateDataSlot(m_device->handle(), &data_create_info, NULL, &data_slot);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    uint64_t bad_handle = (uint64_t)sampler.handle();
    sampler.destroy();

    uint64_t data;
    m_errorMonitor->SetDesiredError("VUID-vkGetPrivateData-objectHandle-09498");
    // valid handle, but not a vkSample
    vk::GetPrivateData(m_device->handle(), VK_OBJECT_TYPE_SAMPLER, bad_handle, data_slot, &data);
    m_errorMonitor->VerifyFound();

    vk::DestroyPrivateDataSlot(m_device->handle(), data_slot, nullptr);
}

TEST_F(NegativeTooling, ValidateNVDeviceDiagnosticCheckpoints) {
    TEST_DESCRIPTION("General testing of VK_NV_device_diagnostic_checkpoints");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    uint32_t data = 100;
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetCheckpointNV-commandBuffer-recording");
    vk::CmdSetCheckpointNV(m_command_buffer.handle(), &data);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTooling, PrivateDataDestroyHandle) {
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

    vk::DestroyPrivateDataSlot(m_device->handle(), data_slot, nullptr);

    uint64_t data;
    m_errorMonitor->SetDesiredError("VUID-vkGetPrivateData-privateDataSlot-parameter");
    vk::GetPrivateData(m_device->handle(), VK_OBJECT_TYPE_SAMPLER, (uint64_t)sampler.handle(), data_slot, &data);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkSetPrivateData-privateDataSlot-parameter");
    vk::SetPrivateData(m_device->handle(), VK_OBJECT_TYPE_SAMPLER, (uint64_t)sampler.handle(), data_slot, data_value);
    m_errorMonitor->VerifyFound();
}
