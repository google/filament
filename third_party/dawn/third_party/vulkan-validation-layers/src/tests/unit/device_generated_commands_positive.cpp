/*
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_object_helper.h"
#include "generated/vk_function_pointers.h"

void DeviceGeneratedCommandsTest::InitBasicDeviceGeneratedCommands() {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceGeneratedCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dgc_props);
    if ((dgc_props.supportedIndirectCommandsShaderStagesPipelineBinding & VK_SHADER_STAGE_COMPUTE_BIT) == 0) {
        GTEST_SKIP() << "VK_SHADER_STAGE_COMPUTE_BIT is not supported.";
    } else if ((dgc_props.supportedIndirectCommandsShaderStagesPipelineBinding & VK_SHADER_STAGE_VERTEX_BIT) == 0) {
        GTEST_SKIP() << "VK_SHADER_STAGE_VERTEX_BIT is not supported.";
    }
}

// "If vkGetGeneratedCommandsMemoryRequirementsEXT returns a non-zero size, preprocessAddress must not be NULL"
// Does the query and updates with preprocessAddress if needed
void DeviceGeneratedCommandsTest::SetPreProcessBuffer(VkGeneratedCommandsInfoEXT& generated_commands_info) {
    VkGeneratedCommandsMemoryRequirementsInfoEXT dgc_mem_reqs = vku::InitStructHelper();
    dgc_mem_reqs.indirectCommandsLayout = generated_commands_info.indirectCommandsLayout;
    dgc_mem_reqs.indirectExecutionSet = generated_commands_info.indirectExecutionSet;
    dgc_mem_reqs.maxSequenceCount = generated_commands_info.maxSequenceCount;
    dgc_mem_reqs.maxDrawCount = generated_commands_info.maxDrawCount;

    VkDeviceAddress pre_process_address = 0;

    VkMemoryRequirements2 mem_reqs2 = vku::InitStructHelper();
    vk::GetGeneratedCommandsMemoryRequirementsEXT(device(), &dgc_mem_reqs, &mem_reqs2);
    const VkDeviceSize pre_process_size = mem_reqs2.memoryRequirements.size;

    if (pre_process_size > 0) {
        VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
        allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

        VkBufferUsageFlags2CreateInfo buffer_usage_flags = vku::InitStructHelper();
        buffer_usage_flags.usage = VK_BUFFER_USAGE_2_PREPROCESS_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage_flags);
        buffer_ci.size = pre_process_size;
        pre_process_buffer_->init(*m_device, buffer_ci, 0, &allocate_flag_info);
        pre_process_address = pre_process_buffer_->Address();
    }

    generated_commands_info.preprocessSize = pre_process_size;
    generated_commands_info.preprocessAddress = pre_process_address;
}

class PositiveDeviceGeneratedCommands : public DeviceGeneratedCommandsTest {};

TEST_F(PositiveDeviceGeneratedCommands, CreateIndirectExecutionSetPipeline) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateGraphicsPipeline();

    vkt::IndirectExecutionSet exe_set(*m_device, pipe.Handle(), 1);
}

TEST_F(PositiveDeviceGeneratedCommands, CreateIndirectExecutionSetShaderObject) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderCreateInfoEXT vert_create_info =
        ShaderCreateInfoFlag(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
    const vkt::Shader vertShader(*m_device, vert_create_info);
    const VkShaderEXT shaders[] = {vertShader.handle()};
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    VkIndirectExecutionSetShaderLayoutInfoEXT exe_set_layouts = vku::InitStructHelper();
    exe_set_layouts.setLayoutCount = 1;
    exe_set_layouts.pSetLayouts = &descriptor_set.layout_.handle();

    VkIndirectExecutionSetShaderInfoEXT exe_set_shader_info = vku::InitStructHelper();
    exe_set_shader_info.shaderCount = 1;
    exe_set_shader_info.pInitialShaders = shaders;
    exe_set_shader_info.pSetLayoutInfos = &exe_set_layouts;
    exe_set_shader_info.maxShaderCount = 1;
    exe_set_shader_info.pushConstantRangeCount = 0;
    vkt::IndirectExecutionSet exe_set(*m_device, exe_set_shader_info);
}

TEST_F(PositiveDeviceGeneratedCommands, CreateIndirectCommandsLayout) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsLayoutTokenEXT token = vku::InitStructHelper();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    token.offset = 0;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 1;
    command_layout_ci.pTokens = &token;

    VkIndirectCommandsLayoutEXT command_layout;
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    vk::DestroyIndirectCommandsLayoutEXT(device(), command_layout, nullptr);
}

TEST_F(PositiveDeviceGeneratedCommands, GetGeneratedCommandsMemoryRequirements) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkIndirectCommandsLayoutTokenEXT token = vku::InitStructHelper();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    token.offset = 0;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 1;
    command_layout_ci.pTokens = &token;

    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateGraphicsPipeline();
    VkGeneratedCommandsPipelineInfoEXT pipeline_info = vku::InitStructHelper();
    pipeline_info.pipeline = pipe.Handle();

    VkGeneratedCommandsMemoryRequirementsInfoEXT req_info = vku::InitStructHelper(&pipeline_info);
    req_info.maxSequenceCount = 1;
    req_info.indirectExecutionSet = VK_NULL_HANDLE;
    req_info.indirectCommandsLayout = command_layout.handle();

    VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();
    vk::GetGeneratedCommandsMemoryRequirementsEXT(device(), &req_info, &mem_req2);
}

TEST_F(PositiveDeviceGeneratedCommands, PushConstant) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsPushConstantTokenEXT pc_token;
    pc_token.updateRange = {VK_SHADER_STAGE_VERTEX_BIT, 8, 8};

    VkIndirectCommandsLayoutTokenEXT tokens[2];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT;
    tokens[0].data.pPushConstant = &pc_token;
    tokens[0].offset = 0;

    tokens[1] = vku::InitStructHelper();
    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    tokens[1].offset = 8;

    const std::vector<VkPushConstantRange> pc_range = {{VK_SHADER_STAGE_VERTEX_BIT, 4, 12}};
    vkt::PipelineLayout pipeline_layout(*m_device, {}, pc_range);
    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = pipeline_layout.handle();
    command_layout_ci.tokenCount = 2;
    command_layout_ci.pTokens = tokens;

    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);
}

TEST_F(PositiveDeviceGeneratedCommands, CmdExecuteGeneratedCommandsGraphics) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateGraphicsPipeline();

    vkt::IndirectExecutionSet exe_set(*m_device, pipe.Handle(), 1);

    VkIndirectCommandsExecutionSetTokenEXT exe_set_token = {VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT,
                                                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT};
    VkIndirectCommandsLayoutTokenEXT tokens[2];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT;
    tokens[0].data.pExecutionSet = &exe_set_token;
    tokens[0].offset = 0;

    tokens[1] = vku::InitStructHelper();
    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    tokens[1].offset = 8;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 2;
    command_layout_ci.pTokens = tokens;

    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer block_buffer(*m_device, 64, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, kHostVisibleMemProps, &allocate_flag_info);

    VkGeneratedCommandsInfoEXT generated_commands_info = vku::InitStructHelper();
    generated_commands_info.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    generated_commands_info.indirectExecutionSet = exe_set.handle();
    generated_commands_info.indirectCommandsLayout = command_layout.handle();
    generated_commands_info.indirectAddressSize = 64;
    generated_commands_info.indirectAddress = block_buffer.Address();
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 1;
    generated_commands_info.maxDrawCount = 1;
    SetPreProcessBuffer(generated_commands_info);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
    m_command_buffer.End();
}

TEST_F(PositiveDeviceGeneratedCommands, UpdateIndirectExecutionSetPipeline) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateGraphicsPipeline();

    vkt::IndirectExecutionSet exe_set(*m_device, pipe.Handle(), 4);

    VkWriteIndirectExecutionSetPipelineEXT write_exe_sets[3];
    write_exe_sets[0] = vku::InitStructHelper();
    write_exe_sets[0].index = 1;
    write_exe_sets[0].pipeline = pipe.Handle();
    write_exe_sets[1] = vku::InitStructHelper();
    write_exe_sets[1].index = 2;
    write_exe_sets[1].pipeline = pipe.Handle();
    write_exe_sets[2] = vku::InitStructHelper();
    write_exe_sets[2].index = 3;
    write_exe_sets[2].pipeline = pipe.Handle();
    vk::UpdateIndirectExecutionSetPipelineEXT(device(), exe_set.handle(), 3, write_exe_sets);
}

TEST_F(PositiveDeviceGeneratedCommands, UpdateIndirectExecutionSetShader) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderCreateInfoEXT vert_create_info =
        ShaderCreateInfoFlag(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
    const vkt::Shader vertShader(*m_device, vert_create_info);
    const VkShaderEXT shaders[] = {vertShader.handle()};
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    VkIndirectExecutionSetShaderLayoutInfoEXT exe_set_layouts = vku::InitStructHelper();
    exe_set_layouts.setLayoutCount = 1;
    exe_set_layouts.pSetLayouts = &descriptor_set.layout_.handle();

    VkIndirectExecutionSetShaderInfoEXT exe_set_shader_info = vku::InitStructHelper();
    exe_set_shader_info.shaderCount = 1;
    exe_set_shader_info.pInitialShaders = shaders;
    exe_set_shader_info.pSetLayoutInfos = &exe_set_layouts;
    exe_set_shader_info.maxShaderCount = 1;
    exe_set_shader_info.pushConstantRangeCount = 0;
    vkt::IndirectExecutionSet exe_set(*m_device, exe_set_shader_info);

    VkWriteIndirectExecutionSetShaderEXT write_exe_set = vku::InitStructHelper();
    write_exe_set.index = 0;
    write_exe_set.shader = vertShader.handle();
    vk::UpdateIndirectExecutionSetShaderEXT(device(), exe_set.handle(), 1, &write_exe_set);
}

TEST_F(PositiveDeviceGeneratedCommands, CmdExecuteGeneratedCommandsCompute) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsExecutionSetTokenEXT exe_set_token = {VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT,
                                                            VK_SHADER_STAGE_COMPUTE_BIT};
    VkIndirectCommandsLayoutTokenEXT tokens[2];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT;
    tokens[0].data.pExecutionSet = &exe_set_token;
    tokens[0].offset = 0;

    tokens[1] = vku::InitStructHelper();
    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT;
    tokens[1].offset = 8;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_COMPUTE_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 2;
    command_layout_ci.pTokens = tokens;
    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreateComputePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateComputePipeline();
    vkt::IndirectExecutionSet exe_set(*m_device, pipe.Handle(), 1);

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer block_buffer(*m_device, 64, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, kHostVisibleMemProps, &allocate_flag_info);

    VkGeneratedCommandsInfoEXT generated_commands_info = vku::InitStructHelper();
    generated_commands_info.shaderStages = VK_SHADER_STAGE_COMPUTE_BIT;
    generated_commands_info.indirectExecutionSet = exe_set.handle();
    generated_commands_info.indirectCommandsLayout = command_layout.handle();
    generated_commands_info.indirectAddressSize = 64;
    generated_commands_info.indirectAddress = block_buffer.Address();
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 1;
    generated_commands_info.maxDrawCount = 1;
    SetPreProcessBuffer(generated_commands_info);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
    m_command_buffer.End();
}

TEST_F(PositiveDeviceGeneratedCommands, ExecuteShaderObjectVertex) {
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkIndirectCommandsExecutionSetTokenEXT exe_set_token = {VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT,
                                                            VK_SHADER_STAGE_VERTEX_BIT};

    VkIndirectCommandsLayoutTokenEXT tokens[2];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT;
    tokens[0].data.pExecutionSet = &exe_set_token;
    tokens[0].offset = 0;

    tokens[1] = vku::InitStructHelper();
    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    tokens[1].offset = 8;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 2;
    command_layout_ci.pTokens = tokens;
    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderCreateInfoEXT vert_create_info =
        ShaderCreateInfoFlag(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
    const vkt::Shader vertShader(*m_device, vert_create_info);
    const VkShaderEXT shaders[] = {vertShader.handle()};
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    VkIndirectExecutionSetShaderLayoutInfoEXT exe_set_layouts = vku::InitStructHelper();
    exe_set_layouts.setLayoutCount = 1;
    exe_set_layouts.pSetLayouts = &descriptor_set.layout_.handle();

    VkIndirectExecutionSetShaderInfoEXT exe_set_shader_info = vku::InitStructHelper();
    exe_set_shader_info.shaderCount = 1;
    exe_set_shader_info.pInitialShaders = shaders;
    exe_set_shader_info.pSetLayoutInfos = &exe_set_layouts;
    exe_set_shader_info.maxShaderCount = 1;
    exe_set_shader_info.pushConstantRangeCount = 0;
    vkt::IndirectExecutionSet exe_set(*m_device, exe_set_shader_info);

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer block_buffer(*m_device, 64, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, kHostVisibleMemProps, &allocate_flag_info);

    VkGeneratedCommandsInfoEXT generated_commands_info = vku::InitStructHelper();
    generated_commands_info.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    generated_commands_info.indirectExecutionSet = exe_set.handle();
    generated_commands_info.indirectCommandsLayout = command_layout.handle();
    generated_commands_info.indirectAddressSize = 64;
    generated_commands_info.indirectAddress = block_buffer.Address();
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 1;
    generated_commands_info.maxDrawCount = 1;
    SetPreProcessBuffer(generated_commands_info);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT};
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, stages, shaders);
    SetDefaultDynamicStatesAll(m_command_buffer.handle());
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}
