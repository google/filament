/*
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "utils/vk_layer_utils.h"

class PositiveGpuAVDescriptorBuffer : public GpuAVTest {};

TEST_F(PositiveGpuAVDescriptorBuffer, Basic) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vkt::Buffer buffer_data(*m_device, 16, 0, vkt::device_address);
    uint32_t *data = (uint32_t *)buffer_data.Memory().Map();
    data[0] = 8;
    data[1] = 12;
    data[2] = 1;
    buffer_data.Memory().Unmap();

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);

    VkPipelineLayoutCreateInfo pipe_layout_ci = vku::InitStructHelper();
    pipe_layout_ci.setLayoutCount = 1;
    pipe_layout_ci.pSetLayouts = &ds_layout.handle();
    vkt::PipelineLayout pipeline_layout(*m_device, pipe_layout_ci);

    VkDeviceSize ds_layout_size = 0;
    vk::GetDescriptorSetLayoutSizeEXT(device(), ds_layout.handle(), &ds_layout_size);

    ds_layout_size = Align(ds_layout_size, descriptor_buffer_properties.descriptorBufferOffsetAlignment);

    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    VkDescriptorAddressInfoEXT addr_info = vku::InitStructHelper();
    addr_info.address = buffer_data.Address();
    addr_info.range = 16;
    addr_info.format = VK_FORMAT_UNDEFINED;

    VkDescriptorGetInfoEXT buffer_descriptor_info = vku::InitStructHelper();
    buffer_descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    buffer_descriptor_info.data.pStorageBuffer = &addr_info;

    void *mapped_descriptor_data = descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), &buffer_descriptor_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data);
    descriptor_buffer.Memory().Unmap();

    char const *cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer SSBO_0 {
            uint a;
            uint b;
            uint c;
        };

        void main() {
            c = a + b;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_info = vku::InitStructHelper();
    descriptor_buffer_binding_info.address = descriptor_buffer.Address();
    descriptor_buffer_binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &descriptor_buffer_binding_info);

    uint32_t buffer_index = 0;
    VkDeviceSize buffer_offset = 0;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                                         &buffer_index, &buffer_offset);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    if (!IsPlatformMockICD()) {
        data = (uint32_t *)buffer_data.Memory().Map();
        ASSERT_TRUE(data[0] == 8);
        ASSERT_TRUE(data[1] == 12);
        ASSERT_TRUE(data[2] == 20);
        buffer_data.Memory().Unmap();
    }
}
