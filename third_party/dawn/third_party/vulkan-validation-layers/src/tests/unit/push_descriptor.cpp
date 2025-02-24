/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2021 ARM, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>
#include <vulkan/vulkan_core.h>
#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/descriptor_helper.h"

class NegativePushDescriptor : public VkLayerTest {};

TEST_F(NegativePushDescriptor, DSBufferInfo) {
    TEST_DESCRIPTION("set that has incorrect parameters in VkDescriptorBufferInfo struct");
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    std::vector<VkDescriptorSetLayoutBinding> ds_bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    OneOffDescriptorSet descriptor_set(m_device, ds_bindings);

    const VkDeviceSize min_alignment = m_device->Physical().limits_.minUniformBufferOffsetAlignment;
    vkt::Buffer buffer(*m_device, min_alignment, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDescriptorBufferInfo buffer_info = {buffer, 0, 0};
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buffer_info;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.dstSet = descriptor_set.set_;

    vkt::DescriptorSetLayout push_dsl(*m_device, ds_bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&push_dsl});

    m_command_buffer.Begin();

    // Cause error due to offset out of range
    buffer_info.offset = min_alignment;
    buffer_info.range = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-offset-00340");
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                                &descriptor_write);
    m_errorMonitor->VerifyFound();

    // Now cause error due to range of 0
    buffer_info.offset = 0;
    buffer_info.range = 0;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-range-00341");
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                                &descriptor_write);
    m_errorMonitor->VerifyFound();

    // Now cause error due to range exceeding buffer size - offset
    buffer_info.offset = 0;
    buffer_info.range = min_alignment + 1;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-range-00342");
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                                &descriptor_write);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativePushDescriptor, DSBufferInfoTemplate) {
    AddRequiredExtensions(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    std::vector<VkDescriptorSetLayoutBinding> ds_bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    OneOffDescriptorSet descriptor_set(m_device, ds_bindings);

    const VkDeviceSize min_alignment = m_device->Physical().limits_.minUniformBufferOffsetAlignment;
    vkt::Buffer buffer(*m_device, min_alignment, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    // Setup for update w/ template tests
    // Create a template of descriptor set updates
    struct SimpleTemplateData {
        uint8_t padding[7];
        VkDescriptorBufferInfo buffer_info;
        uint32_t other_padding[4];
    };
    SimpleTemplateData update_template_data = {};
    update_template_data.buffer_info.buffer = buffer;

    VkDescriptorUpdateTemplateEntry update_template_entry = {};
    update_template_entry.dstBinding = 0;
    update_template_entry.dstArrayElement = 0;
    update_template_entry.descriptorCount = 1;
    update_template_entry.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    update_template_entry.offset = offsetof(SimpleTemplateData, buffer_info);
    update_template_entry.stride = sizeof(SimpleTemplateData);

    vkt::DescriptorSetLayout push_dsl(*m_device, ds_bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&push_dsl});

    VkDescriptorUpdateTemplateCreateInfo push_template_ci = vku::InitStructHelper();
    push_template_ci.descriptorUpdateEntryCount = 1;
    push_template_ci.pDescriptorUpdateEntries = &update_template_entry;
    push_template_ci.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS;
    push_template_ci.descriptorSetLayout = VK_NULL_HANDLE;
    push_template_ci.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    push_template_ci.pipelineLayout = pipeline_layout.handle();
    push_template_ci.set = 0;
    vkt::DescriptorUpdateTemplate push_template(*m_device, push_template_ci);

    m_command_buffer.Begin();

    // Cause error due to offset out of range
    update_template_data.buffer_info.offset = min_alignment;
    update_template_data.buffer_info.range = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-offset-00340");
    vk::CmdPushDescriptorSetWithTemplateKHR(m_command_buffer.handle(), push_template, pipeline_layout, 0, &update_template_data);
    m_errorMonitor->VerifyFound();

    // Now cause error due to range of 0
    update_template_data.buffer_info.offset = 0;
    update_template_data.buffer_info.range = 0;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-range-00341");
    vk::CmdPushDescriptorSetWithTemplateKHR(m_command_buffer.handle(), push_template, pipeline_layout, 0, &update_template_data);
    m_errorMonitor->VerifyFound();

    // Now cause error due to range exceeding buffer size - offset
    update_template_data.buffer_info.offset = 0;
    update_template_data.buffer_info.range = min_alignment + 1;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferInfo-range-00342");
    vk::CmdPushDescriptorSetWithTemplateKHR(m_command_buffer.handle(), push_template, pipeline_layout, 0, &update_template_data);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativePushDescriptor, DestroyDescriptorSetLayout) {
    TEST_DESCRIPTION("Delete the DescriptorSetLayout and then call vkCmdPushDescriptorSetKHR");

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDescriptorSetLayoutBinding ds_binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    VkDescriptorSetLayoutCreateInfo dsl_ci = vku::InitStructHelper();
    dsl_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT;
    dsl_ci.bindingCount = 1;
    dsl_ci.pBindings = &ds_binding;
    vk::CreateDescriptorSetLayout(device(), &dsl_ci, nullptr, &ds_layout);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout;
    pipeline_layout_ci.pushConstantRangeCount = 0;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);

    VkDescriptorBufferInfo buffer_info = {buffer, 0, VK_WHOLE_SIZE};
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buffer_info;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    m_command_buffer.Begin();

    vk::DestroyDescriptorSetLayout(device(), ds_layout, nullptr);
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-dstSet-00320");
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                &descriptor_write);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativePushDescriptor, TemplateDestroyDescriptorSetLayout) {
    TEST_DESCRIPTION("Delete the DescriptorSetLayout and then call vkCmdPushDescriptorSetWithTemplateKHR");

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDescriptorSetLayoutBinding ds_binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    VkDescriptorSetLayoutCreateInfo dsl_ci = vku::InitStructHelper();
    dsl_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT;
    dsl_ci.bindingCount = 1;
    dsl_ci.pBindings = &ds_binding;
    vk::CreateDescriptorSetLayout(device(), &dsl_ci, nullptr, &ds_layout);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout;
    pipeline_layout_ci.pushConstantRangeCount = 0;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);

    struct SimpleTemplateData {
        VkDescriptorBufferInfo buff_info;
    };

    VkDescriptorUpdateTemplateEntry update_template_entry = {};
    update_template_entry.dstBinding = 0;
    update_template_entry.dstArrayElement = 0;
    update_template_entry.descriptorCount = 1;
    update_template_entry.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    update_template_entry.offset = offsetof(SimpleTemplateData, buff_info);
    update_template_entry.stride = sizeof(SimpleTemplateData);

    VkDescriptorUpdateTemplateCreateInfo update_template_ci = vku::InitStructHelper();
    update_template_ci.descriptorUpdateEntryCount = 1;
    update_template_ci.pDescriptorUpdateEntries = &update_template_entry;
    update_template_ci.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS;
    update_template_ci.descriptorSetLayout = ds_layout;
    update_template_ci.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    update_template_ci.pipelineLayout = pipeline_layout;
    vkt::DescriptorUpdateTemplate update_template(*m_device, update_template_ci);

    SimpleTemplateData update_template_data;
    update_template_data.buff_info = {buffer.handle(), 0, 32};

    m_command_buffer.Begin();
    vk::DestroyDescriptorSetLayout(device(), ds_layout, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSetWithTemplate-pData-01686");
    vk::CmdPushDescriptorSetWithTemplateKHR(m_command_buffer.handle(), update_template, pipeline_layout, 0, &update_template_data);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativePushDescriptor, EmptyDescriptorSetLayout) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8065");
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Layout is created with no bindings
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT;
    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

    VkDescriptorPoolSize pool_sizes = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1};
    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &pool_sizes;
    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDescriptorBufferInfo buffer_info = {buffer, 0, VK_WHOLE_SIZE};
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &buffer_info;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout.handle();
    pipeline_layout_ci.pushConstantRangeCount = 0;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-dstBinding-10009");
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                                &descriptor_write);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativePushDescriptor, DSUpdateIndex) {
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkDescriptorBufferInfo buffer_info = {buffer, 0, VK_WHOLE_SIZE};
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstBinding = 2;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buffer_info;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-dstBinding-00315");
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                                &descriptor_write);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativePushDescriptor, DSUpdateEmptyBinding) {
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkDescriptorBufferInfo buffer_info = {buffer, 0, VK_WHOLE_SIZE};
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buffer_info;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-dstBinding-00316");
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                                &descriptor_write);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativePushDescriptor, DSTypeMismatch) {
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);
    vkt::PipelineLayout pipeline_layout(*m_device, {&ds_layout});

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkDescriptorBufferInfo buffer_info = {buffer, 0, VK_WHOLE_SIZE};
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buffer_info;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-00319");
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                                &descriptor_write);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativePushDescriptor, SetLayoutWithoutExtension) {
    TEST_DESCRIPTION("Create a push descriptor set layout without loading the needed extension.");
    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT;
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;

    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-flags-parameter");
    vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePushDescriptor, AllocateSet) {
    TEST_DESCRIPTION("Attempt to allocate a push descriptor set.");
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT;
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;
    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

    VkDescriptorPoolSize pool_size = {binding.descriptorType, binding.descriptorCount};
    VkDescriptorPoolCreateInfo dspci = vku::InitStructHelper();
    dspci.poolSizeCount = 1;
    dspci.pPoolSizes = &pool_size;
    dspci.maxSets = 1;
    vkt::DescriptorPool pool(*m_device, dspci);

    VkDescriptorSetAllocateInfo ds_alloc_info = vku::InitStructHelper();
    ds_alloc_info.descriptorPool = pool.handle();
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &ds_layout.handle();

    VkDescriptorSet ds = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAllocateInfo-pSetLayouts-00308");
    vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePushDescriptor, CreateDescriptorUpdateTemplate) {
    TEST_DESCRIPTION("Verify error messages for invalid vkCreateDescriptorUpdateTemplate calls.");

#ifdef __ANDROID__
    GTEST_SKIP() << "Skipped on Android pending further investigation.";
#endif

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout_ub(*m_device, {dsl_binding});
    const vkt::DescriptorSetLayout ds_layout_ub1(*m_device, {dsl_binding});
    const vkt::DescriptorSetLayout ds_layout_ub_push(*m_device, {dsl_binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {{&ds_layout_ub, &ds_layout_ub1, &ds_layout_ub_push}});

    constexpr uint64_t badhandle = 0xcadecade;
    VkDescriptorUpdateTemplateEntry entries = {0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, sizeof(VkBuffer)};
    VkDescriptorUpdateTemplateCreateInfo create_info = vku::InitStructHelper();
    create_info.flags = 0;
    create_info.descriptorUpdateEntryCount = 1;
    create_info.pDescriptorUpdateEntries = &entries;

    auto do_test = [&](const char* err) {
        VkDescriptorUpdateTemplateKHR dut = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredError(err);
        if (VK_SUCCESS == vk::CreateDescriptorUpdateTemplateKHR(m_device->handle(), &create_info, nullptr, &dut)) {
            vk::DestroyDescriptorUpdateTemplateKHR(m_device->handle(), dut, nullptr);
        }
        m_errorMonitor->VerifyFound();
    };

    // Descriptor set type template
    create_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    // descriptorSetLayout is NULL
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00350");

    // Bad pipelineLayout handle, to be ignored if template type is DESCRIPTOR_SET
    {
        create_info.pipelineLayout = CastFromUint64<VkPipelineLayout>(badhandle);
        create_info.descriptorSetLayout = ds_layout_ub.handle();
        VkDescriptorUpdateTemplateKHR dut = VK_NULL_HANDLE;
        if (VK_SUCCESS == vk::CreateDescriptorUpdateTemplateKHR(m_device->handle(), &create_info, nullptr, &dut)) {
            vk::DestroyDescriptorUpdateTemplateKHR(m_device->handle(), dut, nullptr);
        }
    }

    create_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS;
    // Bad pipelineLayout handle
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00352");

    create_info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
    create_info.pipelineLayout = pipeline_layout.handle();
    create_info.set = 2;

    // Bad bindpoint -- force fuzz the bind point
    memset(&create_info.pipelineBindPoint, 0xFE, sizeof(create_info.pipelineBindPoint));
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00351");
    create_info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

    // Bad pipeline layout
    create_info.pipelineLayout = VK_NULL_HANDLE;
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00352");
    create_info.pipelineLayout = pipeline_layout.handle();

    // Wrong set #
    create_info.set = 0;
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00353");

    // Invalid set #
    create_info.set = 42;
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00353");

    // Bad descriptorSetLayout handle, to be ignored if templateType is PUSH_DESCRIPTORS
    create_info.set = 2;
    create_info.descriptorSetLayout = CastFromUint64<VkDescriptorSetLayout>(badhandle);
    VkDescriptorUpdateTemplateKHR dut = VK_NULL_HANDLE;
    if (VK_SUCCESS == vk::CreateDescriptorUpdateTemplateKHR(m_device->handle(), &create_info, nullptr, &dut)) {
        vk::DestroyDescriptorUpdateTemplateKHR(m_device->handle(), dut, nullptr);
    }
    // Bad descriptorSetLayout handle
    create_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00350");
}

TEST_F(NegativePushDescriptor, CreateDescriptorUpdateTemplate14) {
    TEST_DESCRIPTION("Verify error messages for invalid vkCreateDescriptorUpdateTemplate calls. Rely on 1.4 features");

#ifdef __ANDROID__
    GTEST_SKIP() << "Skipped on Android pending further investigation.";
#endif

    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredFeature(vkt::Feature::pushDescriptor);
    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout_ub(*m_device, {dsl_binding});
    const vkt::DescriptorSetLayout ds_layout_ub1(*m_device, {dsl_binding});
    const vkt::DescriptorSetLayout ds_layout_ub_push(*m_device, {dsl_binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {{&ds_layout_ub, &ds_layout_ub1, &ds_layout_ub_push}});

    constexpr uint64_t badhandle = 0xcadecade;
    VkDescriptorUpdateTemplateEntry entries = {0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, sizeof(VkBuffer)};
    VkDescriptorUpdateTemplateCreateInfo create_info = vku::InitStructHelper();
    create_info.flags = 0;
    create_info.descriptorUpdateEntryCount = 1;
    create_info.pDescriptorUpdateEntries = &entries;

    auto do_test = [&](const char* err) {
        VkDescriptorUpdateTemplateKHR dut = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredError(err);
        if (VK_SUCCESS == vk::CreateDescriptorUpdateTemplate(m_device->handle(), &create_info, nullptr, &dut)) {
            vk::DestroyDescriptorUpdateTemplate(m_device->handle(), dut, nullptr);
        }
        m_errorMonitor->VerifyFound();
    };

    // Descriptor set type template
    create_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    // descriptorSetLayout is NULL
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00350");

    // Bad pipelineLayout handle, to be ignored if template type is DESCRIPTOR_SET
    {
        create_info.pipelineLayout = CastFromUint64<VkPipelineLayout>(badhandle);
        create_info.descriptorSetLayout = ds_layout_ub.handle();
        VkDescriptorUpdateTemplateKHR dut = VK_NULL_HANDLE;
        if (VK_SUCCESS == vk::CreateDescriptorUpdateTemplate(m_device->handle(), &create_info, nullptr, &dut)) {
            vk::DestroyDescriptorUpdateTemplate(m_device->handle(), dut, nullptr);
        }
    }

    create_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS;
    // Bad pipelineLayout handle
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00352");

    create_info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
    create_info.pipelineLayout = pipeline_layout.handle();
    create_info.set = 2;

    // Bad bindpoint -- force fuzz the bind point
    memset(&create_info.pipelineBindPoint, 0xFE, sizeof(create_info.pipelineBindPoint));
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00351");
    create_info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

    // Bad pipeline layout
    create_info.pipelineLayout = VK_NULL_HANDLE;
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00352");
    create_info.pipelineLayout = pipeline_layout.handle();

    // Wrong set #
    create_info.set = 0;
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00353");

    // Invalid set #
    create_info.set = 42;
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00353");

    // Bad descriptorSetLayout handle, to be ignored if templateType is PUSH_DESCRIPTORS
    create_info.set = 2;
    create_info.descriptorSetLayout = CastFromUint64<VkDescriptorSetLayout>(badhandle);
    VkDescriptorUpdateTemplateKHR dut = VK_NULL_HANDLE;
    if (VK_SUCCESS == vk::CreateDescriptorUpdateTemplate(m_device->handle(), &create_info, nullptr, &dut)) {
        vk::DestroyDescriptorUpdateTemplate(m_device->handle(), dut, nullptr);
    }
    // Bad descriptorSetLayout handle
    create_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00350");
}

TEST_F(NegativePushDescriptor, SetLayout) {
    TEST_DESCRIPTION("Create a push descriptor set layout with invalid bindings.");
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT;
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;

    // Starting with the initial VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC type set above..
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-flags-00280");
    vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();

    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-flags-00280");
    vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePushDescriptor, SetLayoutMaxPushDescriptors) {
    TEST_DESCRIPTION("Create a push descriptor set layout with invalid bindings.");
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Get the push descriptor limits
    VkPhysicalDevicePushDescriptorPropertiesKHR push_descriptor_prop = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(push_descriptor_prop);

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT;
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;

    if (push_descriptor_prop.maxPushDescriptors == std::numeric_limits<uint32_t>::max()) {
        GTEST_SKIP() << "maxPushDescriptors is set to maximum unit32_t value";
    }

    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    binding.descriptorCount = push_descriptor_prop.maxPushDescriptors + 1;
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-flags-00281");
    vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePushDescriptor, GetSupportSetLayout) {
    TEST_DESCRIPTION("call vkGetDescriptorSetLayoutSupport on push descriptor set layout with invalid bindings.");
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT;
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;

    VkDescriptorSetLayoutSupport support = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-flags-00280");
    vk::GetDescriptorSetLayoutSupportKHR(device(), &ds_layout_ci, &support);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePushDescriptor, SetLayoutMutableDescriptor) {
    TEST_DESCRIPTION("Create mutable descriptor set layout.");

    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_MUTABLE_EXT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;

    VkDescriptorSetLayout ds_layout;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-pBindings-07303");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();

    VkDescriptorType types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    };

    VkMutableDescriptorTypeListEXT list = {};
    list.descriptorTypeCount = 2;
    list.pDescriptorTypes = types;

    VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &list;

    ds_layout_ci.pNext = &mdtci;

    list.descriptorTypeCount = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_layout_ci.flags =
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT | VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-flags-04590");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();

    ds_layout_ci.flags =
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT | VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-flags-04592");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();

    list.descriptorTypeCount = 2;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-flags-04591");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePushDescriptor, DescriptorUpdateTemplateEntryWithInlineUniformBlock) {
    TEST_DESCRIPTION("Test VkDescriptorUpdateTemplateEntry with descriptor type VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK");

    AddRequiredExtensions(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    std::vector<VkDescriptorSetLayoutBinding> ds_bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    OneOffDescriptorSet descriptor_set(m_device, ds_bindings);

    vkt::Buffer buffer(*m_device, m_device->Physical().limits_.minUniformBufferOffsetAlignment, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    struct SimpleTemplateData {
        VkDescriptorBufferInfo buff_info;
    };

    VkDescriptorUpdateTemplateEntry update_template_entry = {};
    update_template_entry.dstBinding = 0;
    update_template_entry.dstArrayElement = 2;
    update_template_entry.descriptorCount = 1;
    update_template_entry.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
    update_template_entry.offset = offsetof(SimpleTemplateData, buff_info);
    update_template_entry.stride = sizeof(SimpleTemplateData);

    VkDescriptorUpdateTemplateCreateInfo update_template_ci = vku::InitStructHelper();
    update_template_ci.descriptorUpdateEntryCount = 1;
    update_template_ci.pDescriptorUpdateEntries = &update_template_entry;
    update_template_ci.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    update_template_ci.descriptorSetLayout = descriptor_set.layout_.handle();

    VkDescriptorUpdateTemplate update_template = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorUpdateTemplateEntry-descriptor-02226");
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorUpdateTemplateEntry-descriptor-02227");
    vk::CreateDescriptorUpdateTemplateKHR(device(), &update_template_ci, nullptr, &update_template);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePushDescriptor, SetCmdPushQueueFamily) {
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Create ordinary and push descriptor set layout
    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout ds_layout(*m_device, {binding});
    const vkt::DescriptorSetLayout push_ds_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);

    // Now use the descriptor set layouts to create a pipeline layout
    const vkt::PipelineLayout pipeline_layout(*m_device, {&push_ds_layout, &ds_layout});

    vkt::Buffer buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    // Create a "write" struct, noting that the buffer_info cannot be a temporary arg (the return from WriteDescriptorSet
    // references its data), and the DescriptorSet() can be temporary, because the value is ignored
    VkDescriptorBufferInfo buffer_info = {buffer, 0, VK_WHOLE_SIZE};

    VkWriteDescriptorSet descriptor_write =
        vkt::Device::WriteDescriptorSet(vkt::DescriptorSet(), 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &buffer_info);

    // Create command pool on a non-graphics queue
    const std::optional<uint32_t> compute_qfi = m_device->ComputeOnlyQueueFamily();
    const std::optional<uint32_t> transfer_qfi = m_device->TransferOnlyQueueFamily();
    if (!transfer_qfi && !compute_qfi) {
        GTEST_SKIP() << "Queue family type not supported";
    }

    const uint32_t err_qfi = compute_qfi ? compute_qfi.value() : transfer_qfi.value();

    vkt::CommandPool command_pool(*m_device, err_qfi);
    vkt::CommandBuffer command_buffer(*m_device, command_pool);
    command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSet-pipelineBindPoint-00363");
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-00330");
    if (err_qfi == transfer_qfi) {
        // This as this queue neither supports the gfx or compute bindpoints, we'll get two errors
        m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSet-commandBuffer-cmdpool");
    }
    vk::CmdPushDescriptorSetKHR(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_write);
    m_errorMonitor->VerifyFound();
    command_buffer.End();

    // If we succeed in testing only one condition above, we need to test the other below.
    if (transfer_qfi && err_qfi != transfer_qfi.value()) {
        // Need to test the neither compute/gfx supported case separately.
        vkt::CommandPool tran_command_pool(*m_device, transfer_qfi.value());
        vkt::CommandBuffer tran_command_buffer(*m_device, tran_command_pool);
        tran_command_buffer.Begin();

        // We can't avoid getting *both* errors in this case
        m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSet-pipelineBindPoint-00363");
        m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-00330");
        m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSet-commandBuffer-cmdpool");
        vk::CmdPushDescriptorSetKHR(tran_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                                    &descriptor_write);
        m_errorMonitor->VerifyFound();
        tran_command_buffer.End();
    }
}

TEST_F(NegativePushDescriptor, SetCmdPush) {
    TEST_DESCRIPTION("Attempt to push a push descriptor set with incorrect arguments.");
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Create ordinary and push descriptor set layout
    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout ds_layout(*m_device, {binding});
    const vkt::DescriptorSetLayout push_ds_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&push_ds_layout, &ds_layout});

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    // Create a "write" struct, noting that the buffer_info cannot be a temporary arg (the return from WriteDescriptorSet
    // references its data), and the DescriptorSet() can be temporary, because the value is ignored
    VkDescriptorBufferInfo buffer_info = {buffer, 0, VK_WHOLE_SIZE};

    VkWriteDescriptorSet descriptor_write =
        vkt::Device::WriteDescriptorSet(vkt::DescriptorSet(), 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &buffer_info);

    // Push to the non-push binding
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSet-set-00365");
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 1, 1,
                                &descriptor_write);
    m_errorMonitor->VerifyFound();

    // Specify set out of bounds
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSet-set-00364");
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 2, 1,
                                &descriptor_write);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSet-commandBuffer-recording");
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-00330");
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                                &descriptor_write);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePushDescriptor, SetCmdPush14) {
    TEST_DESCRIPTION("Attempt to push a push descriptor set with incorrect arguments. Rely on 1.4 for the feature");
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredFeature(vkt::Feature::pushDescriptor);
    RETURN_IF_SKIP(Init());

    // Create ordinary and push descriptor set layout
    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout ds_layout(*m_device, {binding});
    const vkt::DescriptorSetLayout push_ds_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&push_ds_layout, &ds_layout});

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    // Create a "write" struct, noting that the buffer_info cannot be a temporary arg (the return from WriteDescriptorSet
    // references its data), and the DescriptorSet() can be temporary, because the value is ignored
    VkDescriptorBufferInfo buffer_info = {buffer, 0, VK_WHOLE_SIZE};

    VkWriteDescriptorSet descriptor_write =
        vkt::Device::WriteDescriptorSet(vkt::DescriptorSet(), 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &buffer_info);

    // Push to the non-push binding
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSet-set-00365");
    vk::CmdPushDescriptorSet(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 1, 1, &descriptor_write);
    m_errorMonitor->VerifyFound();

    // Specify set out of bounds
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSet-set-00364");
    vk::CmdPushDescriptorSet(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 2, 1, &descriptor_write);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSet-commandBuffer-recording");
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-00330");
    vk::CmdPushDescriptorSet(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_write);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePushDescriptor, DestoryLayout) {
    TEST_DESCRIPTION("Attempt to push a push descriptor set with incorrect arguments.");
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkDescriptorBufferInfo buffer_info = {buffer.handle(), 0, VK_WHOLE_SIZE};
    VkWriteDescriptorSet descriptor_write =
        vkt::Device::WriteDescriptorSet(vkt::DescriptorSet(), 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &buffer_info);

    m_command_buffer.Begin();
    VkPipelineLayout invalid_layout = CastToHandle<VkPipelineLayout, uintptr_t>(0xbaadbeef);
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSet-layout-parameter");
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, invalid_layout, 1, 1,
                                &descriptor_write);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePushDescriptor, SetCmdBufferOffsetUnaligned) {
    TEST_DESCRIPTION("Attempt to push a push descriptor set buffer with unaligned offset.");
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    auto const min_alignment = m_device->Physical().limits_.minUniformBufferOffsetAlignment;
    if (min_alignment == 0) {
        GTEST_SKIP() << "minUniformBufferOffsetAlignment is zero";
    }

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout push_ds_layout(*m_device, {binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);
    ASSERT_TRUE(push_ds_layout.initialized());

    const vkt::PipelineLayout pipeline_layout(*m_device, {&push_ds_layout});
    ASSERT_TRUE(pipeline_layout.initialized());

    vkt::Buffer buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    // Use an invalid alignment.
    VkDescriptorBufferInfo buffer_info = {buffer.handle(), min_alignment - 1, VK_WHOLE_SIZE};
    VkWriteDescriptorSet descriptor_write =
        vkt::Device::WriteDescriptorSet(vkt::DescriptorSet(), 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &buffer_info);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-00327");
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                &descriptor_write);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativePushDescriptor, DescriptorWriteMissingImageInfo) {
    TEST_DESCRIPTION("Attempt to write descriptor with missing image info");

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = 0u;
    descriptor_write.dstBinding = 0u;
    descriptor_write.dstArrayElement = 0u;
    descriptor_write.descriptorCount = 1u;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_write.pImageInfo = nullptr;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSet-pDescriptorWrites-06494");
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0u, 1u,
                                &descriptor_write);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativePushDescriptor, UnsupportedDescriptorTemplateBindPoint) {
    TEST_DESCRIPTION("Push descriptor set with cmd buffer that doesn't support pipeline bind point");

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const std::optional<uint32_t> compute_qfi = m_device->ComputeOnlyQueueFamily();
    if (!compute_qfi.has_value()) {
        GTEST_SKIP() << "Required queue family capabilities not found.";
    }

    vkt::CommandPool command_pool(*m_device, compute_qfi.value());
    vkt::CommandBuffer command_buffer(*m_device, command_pool);

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    std::vector<VkDescriptorSetLayoutBinding> ds_bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    OneOffDescriptorSet descriptor_set(m_device, ds_bindings);

    vkt::DescriptorSetLayout push_dsl(*m_device, ds_bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);

    vkt::PipelineLayout pipeline_layout(*m_device, {&push_dsl});

    struct SimpleTemplateData {
        VkDescriptorBufferInfo buff_info;
    };

    VkDescriptorUpdateTemplateEntry update_template_entry = {};
    update_template_entry.dstBinding = 0;
    update_template_entry.dstArrayElement = 0;
    update_template_entry.descriptorCount = 1;
    update_template_entry.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    update_template_entry.offset = offsetof(SimpleTemplateData, buff_info);
    update_template_entry.stride = sizeof(SimpleTemplateData);

    VkDescriptorUpdateTemplateCreateInfo update_template_ci = vku::InitStructHelper();
    update_template_ci.descriptorUpdateEntryCount = 1;
    update_template_ci.pDescriptorUpdateEntries = &update_template_entry;
    update_template_ci.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS;
    update_template_ci.descriptorSetLayout = descriptor_set.layout_.handle();
    update_template_ci.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    update_template_ci.pipelineLayout = pipeline_layout.handle();
    vkt::DescriptorUpdateTemplate update_template(*m_device, update_template_ci);

    SimpleTemplateData update_template_data;
    update_template_data.buff_info = {buffer.handle(), 0, 32};

    command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSetWithTemplate-commandBuffer-00366");
    vk::CmdPushDescriptorSetWithTemplateKHR(command_buffer.handle(), update_template, pipeline_layout.handle(), 0,
                                            &update_template_data);
    m_errorMonitor->VerifyFound();
    command_buffer.End();
}

TEST_F(NegativePushDescriptor, InvalidDescriptorUpdateTemplateType) {
    TEST_DESCRIPTION("Use descriptor template with invalid descriptorType");

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    std::vector<VkDescriptorSetLayoutBinding> ds_bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    OneOffDescriptorSet descriptor_set(m_device, ds_bindings);

    vkt::DescriptorSetLayout push_dsl(*m_device, ds_bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);

    vkt::PipelineLayout pipeline_layout(*m_device, {&push_dsl});

    struct SimpleTemplateData {
        VkDescriptorBufferInfo buff_info;
    };

    VkDescriptorUpdateTemplateEntry update_template_entry = {};
    update_template_entry.dstBinding = 0;
    update_template_entry.dstArrayElement = 0;
    update_template_entry.descriptorCount = 1;
    update_template_entry.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    update_template_entry.offset = offsetof(SimpleTemplateData, buff_info);
    update_template_entry.stride = sizeof(SimpleTemplateData);

    VkDescriptorUpdateTemplateCreateInfo update_template_ci = vku::InitStructHelper();
    update_template_ci.descriptorUpdateEntryCount = 1;
    update_template_ci.pDescriptorUpdateEntries = &update_template_entry;
    update_template_ci.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    update_template_ci.descriptorSetLayout = descriptor_set.layout_.handle();
    update_template_ci.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    update_template_ci.pipelineLayout = pipeline_layout.handle();
    vkt::DescriptorUpdateTemplate update_template(*m_device, update_template_ci);

    SimpleTemplateData update_template_data;
    update_template_data.buff_info = {buffer.handle(), 0, 32};

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSetWithTemplate-descriptorUpdateTemplate-07994");
    vk::CmdPushDescriptorSetWithTemplateKHR(m_command_buffer.handle(), update_template, pipeline_layout.handle(), 0,
                                            &update_template_data);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativePushDescriptor, DescriptorTemplateIncompatibleLayout) {
    TEST_DESCRIPTION("Update descriptor set with template with incompatible pipeline layout");

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    std::vector<VkDescriptorSetLayoutBinding> ds_bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    OneOffDescriptorSet descriptor_set(m_device, ds_bindings);

    vkt::DescriptorSetLayout push_dsl(*m_device, ds_bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);
    vkt::DescriptorSetLayout push_dsl2(*m_device, {{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}},
                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);
    vkt::DescriptorSetLayout normal_dsl(*m_device, ds_bindings);

    vkt::PipelineLayout pipeline_layout(*m_device, {&push_dsl});
    vkt::PipelineLayout pipeline_layout2(*m_device, {&push_dsl2});
    vkt::PipelineLayout pipeline_layout3(*m_device, {&push_dsl, &normal_dsl});

    struct SimpleTemplateData {
        VkDescriptorBufferInfo buff_info;
    };

    VkDescriptorUpdateTemplateEntry update_template_entry = {};
    update_template_entry.dstBinding = 0;
    update_template_entry.dstArrayElement = 0;
    update_template_entry.descriptorCount = 1;
    update_template_entry.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    update_template_entry.offset = offsetof(SimpleTemplateData, buff_info);
    update_template_entry.stride = sizeof(SimpleTemplateData);

    VkDescriptorUpdateTemplateCreateInfo update_template_ci = vku::InitStructHelper();
    update_template_ci.descriptorUpdateEntryCount = 1;
    update_template_ci.pDescriptorUpdateEntries = &update_template_entry;
    update_template_ci.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS;
    update_template_ci.descriptorSetLayout = descriptor_set.layout_.handle();
    update_template_ci.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    update_template_ci.pipelineLayout = pipeline_layout.handle();
    vkt::DescriptorUpdateTemplate update_template(*m_device, update_template_ci);

    update_template_ci.descriptorSetLayout = normal_dsl.handle();
    update_template_ci.pipelineLayout = pipeline_layout3.handle();
    vkt::DescriptorUpdateTemplate update_template2(*m_device, update_template_ci);

    SimpleTemplateData update_template_data;
    update_template_data.buff_info = {buffer.handle(), 0, 32};

    m_command_buffer.Begin();

    // bindings don't match up
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSetWithTemplate-layout-07993");
    vk::CmdPushDescriptorSetWithTemplateKHR(m_command_buffer.handle(), update_template, pipeline_layout2, 0, &update_template_data);
    m_errorMonitor->VerifyFound();

    // OOB
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSetWithTemplate-set-07304");
    vk::CmdPushDescriptorSetWithTemplateKHR(m_command_buffer.handle(), update_template, pipeline_layout, 1, &update_template_data);
    m_errorMonitor->VerifyFound();

    // Missing Push Descriptor Flag
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSetWithTemplate-set-07995");
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushDescriptorSetWithTemplate-set-07305");
    vk::CmdPushDescriptorSetWithTemplateKHR(m_command_buffer.handle(), update_template2, pipeline_layout3, 1,
                                            &update_template_data);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}
