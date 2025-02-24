/*
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
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
#include "../framework/descriptor_helper.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_object_helper.h"

class NegativeDeviceGeneratedCommands : public DeviceGeneratedCommandsTest {};

TEST_F(NegativeDeviceGeneratedCommands, MissingFeature) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    vkt::PipelineLayout pipeline_layout(*m_device, {});
    VkIndirectExecutionSetPipelineInfoEXT exe_set_pipeline_info = vku::InitStructHelper();
    exe_set_pipeline_info.initialPipeline = pipe.Handle();
    exe_set_pipeline_info.maxPipelineCount = 1;

    VkIndirectExecutionSetCreateInfoEXT exe_set_ci = vku::InitStructHelper();
    exe_set_ci.type = VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT;
    exe_set_ci.info.pPipelineInfo = &exe_set_pipeline_info;

    VkIndirectExecutionSetEXT exe_set;
    m_errorMonitor->SetDesiredError("VUID-vkCreateIndirectExecutionSetEXT-deviceGeneratedCommands-11013");
    vk::CreateIndirectExecutionSetEXT(device(), &exe_set_ci, nullptr, &exe_set);
    m_errorMonitor->VerifyFound();

    VkIndirectCommandsLayoutTokenEXT token = vku::InitStructHelper();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    token.offset = 0;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 1;
    command_layout_ci.pTokens = &token;

    VkIndirectCommandsLayoutEXT command_layout;
    m_errorMonitor->SetDesiredError("VUID-vkCreateIndirectCommandsLayoutEXT-deviceGeneratedCommands-11089");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, NullPipelineInfo) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectExecutionSetCreateInfoEXT exe_set_ci = vku::InitStructHelper();
    exe_set_ci.type = VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT;
    exe_set_ci.info.pPipelineInfo = nullptr;

    VkIndirectExecutionSetEXT exe_set;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectExecutionSetCreateInfoEXT-pPipelineInfo-parameter");
    vk::CreateIndirectExecutionSetEXT(device(), &exe_set_ci, nullptr, &exe_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, IndirectCommandShaderStageBinding) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceGeneratedCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dgc_props);
    if (dgc_props.supportedIndirectCommandsShaderStagesPipelineBinding & VK_SHADER_STAGE_COMPUTE_BIT) {
        GTEST_SKIP() << "VK_SHADER_STAGE_COMPUTE_BIT is supported.";
    }

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateGraphicsPipeline();

    vkt::PipelineLayout pipeline_layout(*m_device, {});
    VkIndirectExecutionSetPipelineInfoEXT exe_set_pipeline_info = vku::InitStructHelper();
    exe_set_pipeline_info.initialPipeline = pipe.Handle();
    exe_set_pipeline_info.maxPipelineCount = 1;

    VkIndirectExecutionSetCreateInfoEXT exe_set_ci = vku::InitStructHelper();
    exe_set_ci.type = VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT;
    exe_set_ci.info.pPipelineInfo = &exe_set_pipeline_info;

    VkIndirectExecutionSetEXT exe_set;
    m_errorMonitor->SetDesiredError(
        "VUID-VkIndirectExecutionSetPipelineInfoEXT-supportedIndirectCommandsShaderStagesPipelineBinding-11015");
    vk::CreateIndirectExecutionSetEXT(device(), &exe_set_ci, nullptr, &exe_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, IndirectCommandMaxPipelineCount) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateGraphicsPipeline();

    vkt::PipelineLayout pipeline_layout(*m_device, {});
    VkIndirectExecutionSetPipelineInfoEXT exe_set_pipeline_info = vku::InitStructHelper();
    exe_set_pipeline_info.initialPipeline = pipe.Handle();
    exe_set_pipeline_info.maxPipelineCount = 0;

    VkIndirectExecutionSetCreateInfoEXT exe_set_ci = vku::InitStructHelper();
    exe_set_ci.type = VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT;
    exe_set_ci.info.pPipelineInfo = &exe_set_pipeline_info;

    VkIndirectExecutionSetEXT exe_set;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectExecutionSetPipelineInfoEXT-maxPipelineCount-11018");
    vk::CreateIndirectExecutionSetEXT(device(), &exe_set_ci, nullptr, &exe_set);
    m_errorMonitor->VerifyFound();

    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dgc_props);
    exe_set_pipeline_info.maxPipelineCount = dgc_props.maxIndirectPipelineCount + 1;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectExecutionSetPipelineInfoEXT-maxPipelineCount-11018");
    vk::CreateIndirectExecutionSetEXT(device(), &exe_set_ci, nullptr, &exe_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, IndirectCommandDescriptorType) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set_0(m_device,
                                         {
                                             {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                         });
    OneOffDescriptorSet descriptor_set_1(m_device,
                                         {
                                             {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                             {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
                                         });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_0.layout_, &descriptor_set_1.layout_});

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    VkIndirectExecutionSetPipelineInfoEXT exe_set_pipeline_info = vku::InitStructHelper();
    exe_set_pipeline_info.initialPipeline = pipe.Handle();
    exe_set_pipeline_info.maxPipelineCount = 1;

    VkIndirectExecutionSetCreateInfoEXT exe_set_ci = vku::InitStructHelper();
    exe_set_ci.type = VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT;
    exe_set_ci.info.pPipelineInfo = &exe_set_pipeline_info;

    VkIndirectExecutionSetEXT exe_set;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectExecutionSetPipelineInfoEXT-initialPipeline-11019");
    vk::CreateIndirectExecutionSetEXT(device(), &exe_set_ci, nullptr, &exe_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, IndirectCommandsShaderStages) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dgc_props);
    if (dgc_props.supportedIndirectCommandsShaderStages & VK_SHADER_STAGE_GEOMETRY_BIT) {
        GTEST_SKIP() << "VK_SHADER_STAGE_GEOMETRY_BIT is supported.";
    }

    VkIndirectCommandsLayoutTokenEXT token = vku::InitStructHelper();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    token.offset = 0;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_GEOMETRY_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 1;
    command_layout_ci.pTokens = &token;

    VkIndirectCommandsLayoutEXT command_layout;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-shaderStages-11091");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();

    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-shaderStages-11091");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, IndirectCommandsNonGraphics) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsVertexBufferTokenEXT vertex_buffer_token = {0};
    VkIndirectCommandsLayoutTokenEXT tokens[2];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT;
    tokens[0].data.pVertexBuffer = &vertex_buffer_token;
    tokens[0].offset = 0;

    tokens[1] = vku::InitStructHelper();
    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    tokens[1].offset = 8;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_COMPUTE_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 2;
    command_layout_ci.pTokens = tokens;
    // One for each token
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-shaderStages-11110", 2);
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11104", 2);
    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, IndirectCommandsNullUnionPointer) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsLayoutTokenEXT tokens[2];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT;
    tokens[0].data.pVertexBuffer = nullptr;  // is null for all types
    tokens[0].offset = 0;

    tokens[1] = vku::InitStructHelper();
    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    tokens[1].offset = 8;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 2;
    command_layout_ci.pTokens = tokens;

    {
        m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutTokenEXT-pVertexBuffer-parameter");
        vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);
        m_errorMonitor->VerifyFound();
    }

    {
        tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT;
        m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutTokenEXT-pIndexBuffer-parameter");
        vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);
        m_errorMonitor->VerifyFound();
    }

    {
        tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT;
        m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutTokenEXT-pExecutionSet-parameter");
        vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);
        m_errorMonitor->VerifyFound();
    }

    {
        tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT;
        m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutTokenEXT-pPushConstant-parameter");
        vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);
        m_errorMonitor->VerifyFound();
    }

    {
        tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT;
        m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutTokenEXT-pPushConstant-parameter");
        vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDeviceGeneratedCommands, MaxIndirectCommandsTokenCount) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dgc_props);
    const uint32_t over_max = dgc_props.maxIndirectCommandsTokenCount + 1;

    std::vector<VkIndirectCommandsLayoutTokenEXT> tokens(over_max);
    for (uint32_t i = 0; i < over_max; i++) {
        tokens[i] = vku::InitStructHelper();
        tokens[i].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
        tokens[i].offset = 0;
    }

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 0;
    command_layout_ci.pTokens = tokens.data();

    VkIndirectCommandsLayoutEXT command_layout;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-tokenCount-arraylength");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();

    command_layout_ci.tokenCount = over_max;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-tokenCount-11092");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, NonActionTokens) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsExecutionSetTokenEXT exe_set_token = {VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT,
                                                            VK_SHADER_STAGE_COMPUTE_BIT};

    VkIndirectCommandsLayoutTokenEXT token = vku::InitStructHelper();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT;
    token.data.pExecutionSet = &exe_set_token;
    token.offset = 0;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_COMPUTE_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 1;
    command_layout_ci.pTokens = &token;

    VkIndirectCommandsLayoutEXT command_layout;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11100");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, EndWithExecutionSetToken) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsExecutionSetTokenEXT exe_set_token = {VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT,
                                                            VK_SHADER_STAGE_COMPUTE_BIT};

    VkIndirectCommandsLayoutTokenEXT tokens[4];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT;
    tokens[0].data.pExecutionSet = &exe_set_token;
    tokens[0].offset = 0;
    tokens[1] = tokens[0];
    tokens[2] = tokens[0];
    tokens[3] = tokens[0];

    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT;
    tokens[3].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_COMPUTE_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 4;
    command_layout_ci.pTokens = tokens;

    VkIndirectCommandsLayoutEXT command_layout;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11093");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, NullPipelineLayout) {
    AddRequiredFeature(vkt::Feature::dynamicGeneratedPipelineLayout);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsPushConstantTokenEXT pc_token;
    pc_token.updateRange = {VK_SHADER_STAGE_VERTEX_BIT, 4, 0};

    VkIndirectCommandsLayoutTokenEXT token = vku::InitStructHelper();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT;
    token.data.pPushConstant = &pc_token;
    token.offset = 0;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 1;
    command_layout_ci.pTokens = &token;

    VkIndirectCommandsLayoutEXT command_layout;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11102");
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11100");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, MissingVertex) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsLayoutTokenEXT token = vku::InitStructHelper();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    token.offset = 0;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_FRAGMENT_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 1;
    command_layout_ci.pTokens = &token;

    VkIndirectCommandsLayoutEXT command_layout;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-shaderStages-11113");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, TokenOffsetDecrease) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsLayoutTokenEXT tokens[2];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    tokens[0].offset = 4;

    tokens[1] = tokens[0];
    tokens[1].offset = 0;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 2;
    command_layout_ci.pTokens = tokens;

    VkIndirectCommandsLayoutEXT command_layout;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11103");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, TokenOffsetLimit) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dgc_props);

    if (dgc_props.maxIndirectCommandsTokenOffset == std::numeric_limits<uint32_t>::max()) {
        GTEST_SKIP() << "maxIndirectCommandsTokenOffset is too large";
    }

    VkIndirectCommandsLayoutTokenEXT token = vku::InitStructHelper();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    token.offset = dgc_props.maxIndirectCommandsTokenOffset + 1;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 1;
    command_layout_ci.pTokens = &token;

    VkIndirectCommandsLayoutEXT command_layout;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutTokenEXT-offset-11124");
    m_errorMonitor->SetUnexpectedError("VUID-VkIndirectCommandsLayoutTokenEXT-offset-11125");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, PushConstantNoStage) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsPushConstantTokenEXT pc_token;
    pc_token.updateRange = {VK_SHADER_STAGE_FRAGMENT_BIT, 4, 8};

    VkIndirectCommandsLayoutTokenEXT tokens[2];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT;
    tokens[0].data.pPushConstant = &pc_token;
    tokens[0].offset = 0;

    tokens[1] = vku::InitStructHelper();
    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    tokens[1].offset = 8;

    const std::vector<VkPushConstantRange> pc_range = {{VK_SHADER_STAGE_VERTEX_BIT, 16, 64}};
    vkt::PipelineLayout pipeline_layout(*m_device, {}, pc_range);
    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = pipeline_layout.handle();
    command_layout_ci.tokenCount = 2;
    command_layout_ci.pTokens = tokens;

    VkIndirectCommandsLayoutEXT command_layout;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsPushConstantTokenEXT-updateRange-11132");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, PushConstantOutOfRange) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsPushConstantTokenEXT pc_token;
    pc_token.updateRange = {VK_SHADER_STAGE_VERTEX_BIT, 4, 8};

    VkIndirectCommandsLayoutTokenEXT tokens[2];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT;
    tokens[0].data.pPushConstant = &pc_token;
    tokens[0].offset = 0;

    tokens[1] = vku::InitStructHelper();
    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    tokens[1].offset = 8;

    const std::vector<VkPushConstantRange> pc_range = {{VK_SHADER_STAGE_VERTEX_BIT, 16, 64}};
    vkt::PipelineLayout pipeline_layout(*m_device, {}, pc_range);
    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = pipeline_layout.handle();
    command_layout_ci.tokenCount = 2;
    command_layout_ci.pTokens = tokens;

    VkIndirectCommandsLayoutEXT command_layout;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsPushConstantTokenEXT-updateRange-11132");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, PushConstantOutOfRangeDynamic) {
    AddRequiredFeature(vkt::Feature::dynamicGeneratedPipelineLayout);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsPushConstantTokenEXT pc_token;
    pc_token.updateRange = {VK_SHADER_STAGE_VERTEX_BIT, 4, 8};

    VkIndirectCommandsLayoutTokenEXT tokens[2];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT;
    tokens[0].data.pPushConstant = &pc_token;
    tokens[0].offset = 0;

    tokens[1] = vku::InitStructHelper();
    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    tokens[1].offset = 8;

    VkPushConstantRange pc_range = {VK_SHADER_STAGE_VERTEX_BIT, 16, 64};
    VkPipelineLayoutCreateInfo layout_ci = vku::InitStructHelper();
    layout_ci.pushConstantRangeCount = 1;
    layout_ci.pPushConstantRanges = &pc_range;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper(&layout_ci);
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 2;
    command_layout_ci.pTokens = tokens;

    VkIndirectCommandsLayoutEXT command_layout;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsPushConstantTokenEXT-updateRange-11132");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, PushConstantMultipleTokens) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsPushConstantTokenEXT pc_token_0;
    pc_token_0.updateRange = {VK_SHADER_STAGE_VERTEX_BIT, 4, 16};

    VkIndirectCommandsPushConstantTokenEXT pc_token_1;
    pc_token_1.updateRange = {VK_SHADER_STAGE_VERTEX_BIT, 0, 8};

    VkIndirectCommandsLayoutTokenEXT tokens[3];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT;
    tokens[0].data.pPushConstant = &pc_token_0;
    tokens[0].offset = 0;

    tokens[1] = vku::InitStructHelper();
    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT;
    tokens[1].data.pPushConstant = &pc_token_1;
    tokens[1].offset = 8;

    tokens[2] = vku::InitStructHelper();
    tokens[2].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    tokens[2].offset = 16;

    const std::vector<VkPushConstantRange> pc_range = {{VK_SHADER_STAGE_VERTEX_BIT, 0, 64}};
    vkt::PipelineLayout pipeline_layout(*m_device, {}, pc_range);
    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = pipeline_layout.handle();
    command_layout_ci.tokenCount = 3;
    command_layout_ci.pTokens = tokens;

    VkIndirectCommandsLayoutEXT command_layout;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11099");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, PushConstantSequenceIndex) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsPushConstantTokenEXT pc_token_0;
    pc_token_0.updateRange = {VK_SHADER_STAGE_VERTEX_BIT, 0, 16};

    VkIndirectCommandsPushConstantTokenEXT pc_token_1;
    pc_token_1.updateRange = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};

    VkIndirectCommandsLayoutTokenEXT tokens[3];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT;
    tokens[0].data.pPushConstant = &pc_token_0;
    tokens[0].offset = 0;

    tokens[1] = vku::InitStructHelper();
    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT;
    tokens[1].data.pPushConstant = &pc_token_1;
    tokens[1].offset = 8;

    tokens[2] = vku::InitStructHelper();
    tokens[2].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    tokens[2].offset = 16;

    const std::vector<VkPushConstantRange> pc_range = {{VK_SHADER_STAGE_VERTEX_BIT, 0, 64}};
    vkt::PipelineLayout pipeline_layout(*m_device, {}, pc_range);
    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = pipeline_layout.handle();
    command_layout_ci.tokenCount = 3;
    command_layout_ci.pTokens = tokens;

    VkIndirectCommandsLayoutEXT command_layout;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11099");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, CmdExecuteGeneratedCommandsSecondary) {
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

    vkt::PipelineLayout pipeline_layout(*m_device, {});
    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = pipeline_layout.handle();
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

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    secondary.Begin();
    vk::CmdBindPipeline(secondary.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteGeneratedCommandsEXT-bufferlevel");
    vk::CmdExecuteGeneratedCommandsEXT(secondary.handle(), false, &generated_commands_info);
    m_errorMonitor->VerifyFound();
    secondary.End();
}

TEST_F(NegativeDeviceGeneratedCommands, UpdateIESPipelineFlags) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->SetDesiredError("VUID-VkIndirectExecutionSetPipelineInfoEXT-initialPipeline-11153");
    vkt::IndirectExecutionSet exe_set(*m_device, pipe.Handle(), 2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, UpdateIESPipelineWriteCount) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateGraphicsPipeline();

    vkt::IndirectExecutionSet exe_set(*m_device, pipe.Handle(), 1);

    m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-executionSetWriteCount-arraylength");
    vk::UpdateIndirectExecutionSetPipelineEXT(device(), exe_set.handle(), 0, nullptr);
    m_errorMonitor->VerifyFound();

    VkWriteIndirectExecutionSetPipelineEXT write_exe_sets[2];
    write_exe_sets[0] = vku::InitStructHelper();
    write_exe_sets[0].index = 0;
    write_exe_sets[0].pipeline = pipe.Handle();
    write_exe_sets[1] = vku::InitStructHelper();
    write_exe_sets[1].index = 0;
    write_exe_sets[1].pipeline = pipe.Handle();
    m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-pExecutionSetWrites-11042");
    m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-executionSetWriteCount-11037");
    vk::UpdateIndirectExecutionSetPipelineEXT(device(), exe_set.handle(), 2, write_exe_sets);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, UpdateIESPipelineIndex) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateGraphicsPipeline();

    vkt::IndirectExecutionSet exe_set(*m_device, pipe.Handle(), 2);

    VkWriteIndirectExecutionSetPipelineEXT write_exe_set = vku::InitStructHelper();
    write_exe_set.index = 2;
    write_exe_set.pipeline = pipe.Handle();
    m_errorMonitor->SetDesiredError("VUID-VkWriteIndirectExecutionSetPipelineEXT-index-11026");
    vk::UpdateIndirectExecutionSetPipelineEXT(device(), exe_set.handle(), 1, &write_exe_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, UpdateIESPipelineDynamicState) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper init_pipe(*this, &pipe_flags2);
    init_pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    init_pipe.CreateGraphicsPipeline();

    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR);
    pipe.CreateGraphicsPipeline();

    vkt::IndirectExecutionSet exe_set(*m_device, init_pipe.Handle(), 2);

    VkWriteIndirectExecutionSetPipelineEXT write_exe_set = vku::InitStructHelper();
    write_exe_set.index = 1;
    write_exe_set.pipeline = pipe.Handle();
    m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-None-11040");
    vk::UpdateIndirectExecutionSetPipelineEXT(device(), exe_set.handle(), 1, &write_exe_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, UpdateIESPipelineFragmentOutput) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper init_pipe(*this, &pipe_flags2);
    init_pipe.CreateGraphicsPipeline();

    char const *fs_source = R"glsl(
        #version 460
        layout(location = 1) out vec4 uFragColor;
        void main(){
            uFragColor = vec4(0,1,0,1);
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    vkt::IndirectExecutionSet exe_set(*m_device, init_pipe.Handle(), 2);

    VkWriteIndirectExecutionSetPipelineEXT write_exe_set = vku::InitStructHelper();
    write_exe_set.index = 1;
    write_exe_set.pipeline = pipe.Handle();
    m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-initialPipeline-11147");
    vk::UpdateIndirectExecutionSetPipelineEXT(device(), exe_set.handle(), 1, &write_exe_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, UpdateIESPipelineFragDepth) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    // has no FragDepth
    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper init_pipe(*this, &pipe_flags2);
    init_pipe.CreateGraphicsPipeline();
    vkt::IndirectExecutionSet exe_set(*m_device, init_pipe.Handle(), 1);

    char const *fs_source = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
            uFragColor = vec4(0,1,0,1);
            gl_FragDepth = 1.0f;
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    VkWriteIndirectExecutionSetPipelineEXT write_exe_sets = vku::InitStructHelper();
    write_exe_sets.index = 0;
    write_exe_sets.pipeline = pipe.Handle();
    m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-initialPipeline-11098");
    vk::UpdateIndirectExecutionSetPipelineEXT(device(), exe_set.handle(), 1, &write_exe_sets);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, UpdateIESPipelineSampleMask) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    // has no SampleMask
    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper init_pipe(*this, &pipe_flags2);
    init_pipe.CreateGraphicsPipeline();
    vkt::IndirectExecutionSet exe_set(*m_device, init_pipe.Handle(), 1);

    char const *fs_source = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
            uFragColor = vec4(0,1,0,1);
            gl_SampleMask[0] = 1;
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    VkWriteIndirectExecutionSetPipelineEXT write_exe_sets = vku::InitStructHelper();
    write_exe_sets.index = 0;
    write_exe_sets.pipeline = pipe.Handle();
    m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-initialPipeline-11086");
    vk::UpdateIndirectExecutionSetPipelineEXT(device(), exe_set.handle(), 1, &write_exe_sets);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, UpdateIESPipelineStencilExportEXT) {
    AddRequiredExtensions(VK_EXT_SHADER_STENCIL_EXPORT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    // has no StencilExportEXT
    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper init_pipe(*this, &pipe_flags2);
    init_pipe.CreateGraphicsPipeline();
    vkt::IndirectExecutionSet exe_set(*m_device, init_pipe.Handle(), 1);

    char const *fs_source = R"glsl(
        #version 460
        #extension GL_ARB_shader_stencil_export: enable
        layout(location = 0) out vec4 uFragColor;
        out int gl_FragStencilRefARB;
        void main(){
            uFragColor = vec4(0,1,0,1);
            gl_FragStencilRefARB = 1;
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    VkWriteIndirectExecutionSetPipelineEXT write_exe_sets = vku::InitStructHelper();
    write_exe_sets.index = 0;
    write_exe_sets.pipeline = pipe.Handle();
    m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-initialPipeline-11085");
    vk::UpdateIndirectExecutionSetPipelineEXT(device(), exe_set.handle(), 1, &write_exe_sets);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, UpdateIESPipelineShaderStages) {
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::shaderTessellationAndGeometryPointSize);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dgc_props);
    if ((dgc_props.supportedIndirectCommandsShaderStagesPipelineBinding & VK_SHADER_STAGE_GEOMETRY_BIT) == 0) {
        GTEST_SKIP() << "VK_SHADER_STAGE_GEOMETRY_BIT is not supported.";
    }

    // has no StencilExportEXT
    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper init_pipe(*this, &pipe_flags2);
    init_pipe.CreateGraphicsPipeline();
    vkt::IndirectExecutionSet exe_set(*m_device, init_pipe.Handle(), 1);

    VkShaderObj gs(this, kGeometryMinimalGlsl, VK_SHADER_STAGE_GEOMETRY_BIT);

    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    VkWriteIndirectExecutionSetPipelineEXT write_exe_sets = vku::InitStructHelper();
    write_exe_sets.index = 0;
    write_exe_sets.pipeline = pipe.Handle();
    m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-initialPipeline-11152");
    vk::UpdateIndirectExecutionSetPipelineEXT(device(), exe_set.handle(), 1, &write_exe_sets);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, UpdateIESPipelineCompatible) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    vkt::Buffer uniform_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    OneOffDescriptorSet descriptor_set_vert(m_device,
                                            {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}});
    OneOffDescriptorSet descriptor_set_all(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    descriptor_set_vert.WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set_vert.UpdateDescriptorSets();
    descriptor_set_all.WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set_all.UpdateDescriptorSets();

    const vkt::PipelineLayout pipeline_layout_vert(*m_device, {&descriptor_set_vert.layout_});
    const vkt::PipelineLayout pipeline_layout_all(*m_device, {&descriptor_set_all.layout_});

    char const *vs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer StorageBuffer {
            uint a;
            uint b;
        };
        void main() {
            a = b;
        }
    )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper init_pipe(*this, &pipe_flags2);
    init_pipe.gp_ci_.layout = pipeline_layout_vert.handle();
    init_pipe.shader_stages_ = {vs.GetStageCreateInfo(), init_pipe.fs_->GetStageCreateInfo()};
    init_pipe.CreateGraphicsPipeline();
    vkt::IndirectExecutionSet exe_set(*m_device, init_pipe.Handle(), 1);

    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.gp_ci_.layout = pipeline_layout_all.handle();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    VkWriteIndirectExecutionSetPipelineEXT write_exe_sets = vku::InitStructHelper();
    write_exe_sets.index = 0;
    write_exe_sets.pipeline = pipe.Handle();
    m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-None-11039");
    vk::UpdateIndirectExecutionSetPipelineEXT(device(), exe_set.handle(), 1, &write_exe_sets);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, UpdateIESMixShaderObjectPipeline) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateGraphicsPipeline();
    vkt::IndirectExecutionSet exe_set_pipeline(*m_device, pipe.Handle(), 1);

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
    vkt::IndirectExecutionSet exe_set_shader(*m_device, exe_set_shader_info);

    VkWriteIndirectExecutionSetShaderEXT write_exe_set_shader = vku::InitStructHelper();
    write_exe_set_shader.index = 0;
    write_exe_set_shader.shader = vertShader.handle();
    m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetShaderEXT-indirectExecutionSet-11041");
    vk::UpdateIndirectExecutionSetShaderEXT(device(), exe_set_pipeline.handle(), 1, &write_exe_set_shader);
    m_errorMonitor->VerifyFound();

    VkWriteIndirectExecutionSetPipelineEXT write_exe_set_pipeline = vku::InitStructHelper();
    write_exe_set_pipeline.index = 0;
    write_exe_set_pipeline.pipeline = pipe.Handle();
    m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-indirectExecutionSet-11035");
    vk::UpdateIndirectExecutionSetPipelineEXT(device(), exe_set_shader.handle(), 1, &write_exe_set_pipeline);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, UpdateIESShaderObjectFlags) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));
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
    m_errorMonitor->SetDesiredError("VUID-VkIndirectExecutionSetShaderInfoEXT-pInitialShaders-11154");
    vkt::IndirectExecutionSet exe_set(*m_device, exe_set_shader_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, UpdateIESShaderObjectWriteCount) {
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

    m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetShaderEXT-executionSetWriteCount-arraylength");
    vk::UpdateIndirectExecutionSetShaderEXT(device(), exe_set.handle(), 0, nullptr);
    m_errorMonitor->VerifyFound();

    VkWriteIndirectExecutionSetShaderEXT write_exe_set = vku::InitStructHelper();
    write_exe_set.index = 1;
    write_exe_set.shader = vertShader.handle();
    m_errorMonitor->SetDesiredError("VUID-VkWriteIndirectExecutionSetShaderEXT-index-11031");
    vk::UpdateIndirectExecutionSetShaderEXT(device(), exe_set.handle(), 1, &write_exe_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, UpdateIESShaderObjectDuplicateIndex) {
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

    VkWriteIndirectExecutionSetShaderEXT write_exe_sets[2];
    write_exe_sets[0] = vku::InitStructHelper();
    write_exe_sets[0].index = 0;
    write_exe_sets[0].shader = vertShader.handle();
    write_exe_sets[1] = vku::InitStructHelper();
    write_exe_sets[1].index = 0;
    write_exe_sets[1].shader = vertShader.handle();
    m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetShaderEXT-pExecutionSetWrites-11043");
    vk::UpdateIndirectExecutionSetShaderEXT(device(), exe_set.handle(), 2, write_exe_sets);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, UpdateIESShaderObjectInitialShaders) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
    VkShaderCreateInfoEXT frag_create_info =
        ShaderCreateInfoFlag(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
    const vkt::Shader fragShader(*m_device, frag_create_info);

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
    write_exe_set.shader = fragShader.handle();
    m_errorMonitor->SetDesiredError("VUID-VkWriteIndirectExecutionSetShaderEXT-pInitialShaders-11033");
    vk::UpdateIndirectExecutionSetShaderEXT(device(), exe_set.handle(), 1, &write_exe_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, UpdateIESShaderObjectFragmentOutput) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredExtensions(VK_EXT_SHADER_STENCIL_EXPORT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);  // uses location 0
    VkShaderCreateInfoEXT frag_create_info =
        ShaderCreateInfoFlag(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
    const vkt::Shader fragShader(*m_device, frag_create_info);

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderCreateInfoEXT vert_create_info =
        ShaderCreateInfoFlag(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
    const vkt::Shader vertShader(*m_device, vert_create_info);
    const VkShaderEXT shaders[] = {vertShader.handle(), fragShader.handle()};
    VkIndirectExecutionSetShaderLayoutInfoEXT exe_set_layouts[2];
    exe_set_layouts[0] = vku::InitStructHelper();
    exe_set_layouts[0].setLayoutCount = 0;
    exe_set_layouts[1] = vku::InitStructHelper();
    exe_set_layouts[1].setLayoutCount = 0;

    VkIndirectExecutionSetShaderInfoEXT exe_set_shader_info = vku::InitStructHelper();
    exe_set_shader_info.shaderCount = 2;
    exe_set_shader_info.pInitialShaders = shaders;
    exe_set_shader_info.pSetLayoutInfos = exe_set_layouts;
    exe_set_shader_info.maxShaderCount = 2;
    exe_set_shader_info.pushConstantRangeCount = 0;
    vkt::IndirectExecutionSet exe_set(*m_device, exe_set_shader_info);

    VkWriteIndirectExecutionSetShaderEXT write_exe_set = vku::InitStructHelper();
    write_exe_set.index = 1;

    {
        char const *fs_source_location = R"glsl(
            #version 460
            layout(location = 1) out vec4 uFragColor;
            void main(){
                uFragColor = vec4(0,1,0,1);
            }
        )glsl";

        const auto frag_spv_location = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_source_location);
        VkShaderCreateInfoEXT frag_create_info_location =
            ShaderCreateInfoFlag(frag_spv_location, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
        const vkt::Shader frag_shader_location(*m_device, frag_create_info_location);
        write_exe_set.shader = frag_shader_location.handle();
        m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetShaderEXT-None-11148");
        vk::UpdateIndirectExecutionSetShaderEXT(device(), exe_set.handle(), 1, &write_exe_set);
        m_errorMonitor->VerifyFound();
    }

    {
        char const *fs_source_depth = R"glsl(
            #version 460
            layout(location = 0) out vec4 uFragColor;
            void main(){
                uFragColor = vec4(0,1,0,1);
                gl_FragDepth = 1.0f;
            }
        )glsl";

        const auto frag_spv_depth = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_source_depth);
        VkShaderCreateInfoEXT frag_create_info_depth =
            ShaderCreateInfoFlag(frag_spv_depth, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
        const vkt::Shader frag_shader_depth(*m_device, frag_create_info_depth);

        write_exe_set.shader = frag_shader_depth.handle();
        m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetShaderEXT-FragDepth-11054");
        vk::UpdateIndirectExecutionSetShaderEXT(device(), exe_set.handle(), 1, &write_exe_set);
        m_errorMonitor->VerifyFound();
    }

    {
        char const *fs_source_mask = R"glsl(
            #version 460
            layout(location = 0) out vec4 uFragColor;
            void main(){
                uFragColor = vec4(0,1,0,1);
            gl_SampleMask[0] = 1;
            }
        )glsl";

        const auto frag_spv_mask = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_source_mask);
        VkShaderCreateInfoEXT frag_create_info_mask =
            ShaderCreateInfoFlag(frag_spv_mask, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
        const vkt::Shader frag_shader_mask(*m_device, frag_create_info_mask);

        write_exe_set.shader = frag_shader_mask.handle();
        m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetShaderEXT-SampleMask-11050");
        vk::UpdateIndirectExecutionSetShaderEXT(device(), exe_set.handle(), 1, &write_exe_set);
        m_errorMonitor->VerifyFound();
    }

    {
        char const *fs_source_stencil = R"glsl(
            #version 460
            #extension GL_ARB_shader_stencil_export: enable
            layout(location = 0) out vec4 uFragColor;
            out int gl_FragStencilRefARB;
            void main(){
                uFragColor = vec4(0,1,0,1);
                gl_FragStencilRefARB = 1;
            }
        )glsl";

        const auto frag_spv_stencil = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_source_stencil);
        VkShaderCreateInfoEXT frag_create_info_stencil =
            ShaderCreateInfoFlag(frag_spv_stencil, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
        const vkt::Shader frag_shader_stencil(*m_device, frag_create_info_stencil);

        write_exe_set.shader = frag_shader_stencil.handle();
        m_errorMonitor->SetDesiredError("VUID-vkUpdateIndirectExecutionSetShaderEXT-StencilExportEXT-11003");
        vk::UpdateIndirectExecutionSetShaderEXT(device(), exe_set.handle(), 1, &write_exe_set);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDeviceGeneratedCommands, IESShaderObjectUniqueShaders) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderCreateInfoEXT vert_create_info =
        ShaderCreateInfoFlag(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT);
    const vkt::Shader vertShader(*m_device, vert_create_info);
    const VkShaderEXT shaders[2] = {vertShader.handle(), vertShader.handle()};
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    VkIndirectExecutionSetShaderLayoutInfoEXT exe_set_layouts[2];
    exe_set_layouts[0] = vku::InitStructHelper();
    exe_set_layouts[0].setLayoutCount = 1;
    exe_set_layouts[0].pSetLayouts = &descriptor_set.layout_.handle();
    exe_set_layouts[1] = exe_set_layouts[0];

    VkIndirectExecutionSetShaderInfoEXT exe_set_shader_info = vku::InitStructHelper();
    exe_set_shader_info.shaderCount = 2;
    exe_set_shader_info.pInitialShaders = shaders;
    exe_set_shader_info.pSetLayoutInfos = exe_set_layouts;
    exe_set_shader_info.maxShaderCount = 2;
    exe_set_shader_info.pushConstantRangeCount = 0;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectExecutionSetShaderInfoEXT-stage-11023");
    vkt::IndirectExecutionSet exe_set(*m_device, exe_set_shader_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, IESShaderObjectMaxShaderCount) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));
    const VkShaderEXT shaders[] = {vertShader.handle()};
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    VkIndirectExecutionSetShaderLayoutInfoEXT exe_set_layouts = vku::InitStructHelper();
    exe_set_layouts.setLayoutCount = 1;
    exe_set_layouts.pSetLayouts = &descriptor_set.layout_.handle();

    VkIndirectExecutionSetShaderInfoEXT exe_set_shader_info = vku::InitStructHelper();
    exe_set_shader_info.shaderCount = 1;
    exe_set_shader_info.pInitialShaders = shaders;
    exe_set_shader_info.pSetLayoutInfos = &exe_set_layouts;
    exe_set_shader_info.maxShaderCount = 0;
    exe_set_shader_info.pushConstantRangeCount = 0;

    {
        m_errorMonitor->SetDesiredError("VUID-VkIndirectExecutionSetShaderInfoEXT-maxShaderCount-11021");
        vkt::IndirectExecutionSet exe_set(*m_device, exe_set_shader_info);
        m_errorMonitor->VerifyFound();
    }

    {
        VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
        GetPhysicalDeviceProperties2(dgc_props);
        exe_set_shader_info.maxShaderCount = dgc_props.maxIndirectShaderObjectCount + 1;
        m_errorMonitor->SetDesiredError("VUID-VkIndirectExecutionSetShaderInfoEXT-maxShaderCount-11022");
        vkt::IndirectExecutionSet exe_set(*m_device, exe_set_shader_info);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDeviceGeneratedCommands, IESShaderObjectMaxShaderCount2) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));
    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl));
    const VkShaderEXT shaders[2] = {vertShader.handle(), fragShader.handle()};
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    VkIndirectExecutionSetShaderLayoutInfoEXT exe_set_layouts[2];
    exe_set_layouts[0] = vku::InitStructHelper();
    exe_set_layouts[0].setLayoutCount = 1;
    exe_set_layouts[0].pSetLayouts = &descriptor_set.layout_.handle();
    exe_set_layouts[1] = exe_set_layouts[0];

    VkIndirectExecutionSetShaderInfoEXT exe_set_shader_info = vku::InitStructHelper();
    exe_set_shader_info.shaderCount = 2;
    exe_set_shader_info.pInitialShaders = shaders;
    exe_set_shader_info.pSetLayoutInfos = exe_set_layouts;
    exe_set_shader_info.maxShaderCount = 1;
    exe_set_shader_info.pushConstantRangeCount = 0;

    m_errorMonitor->SetDesiredError("VUID-VkIndirectExecutionSetShaderInfoEXT-maxShaderCount-11036");
    vkt::IndirectExecutionSet exe_set(*m_device, exe_set_shader_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, GetRequirementsExecutionSetTokenStage) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkIndirectCommandsExecutionSetTokenEXT exe_set_token = {VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT,
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
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 2;
    command_layout_ci.pTokens = tokens;
    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper init_pipe(*this, &pipe_flags2);
    init_pipe.CreateGraphicsPipeline();  // vert and frag
    vkt::IndirectExecutionSet exe_set(*m_device, init_pipe.Handle(), 1);

    VkGeneratedCommandsMemoryRequirementsInfoEXT req_info = vku::InitStructHelper();
    req_info.maxSequenceCount = 1;
    req_info.indirectExecutionSet = exe_set.handle();
    req_info.indirectCommandsLayout = command_layout.handle();

    VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsMemoryRequirementsInfoEXT-indirectCommandsLayout-11151");
    vk::GetGeneratedCommandsMemoryRequirementsEXT(device(), &req_info, &mem_req2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, GetRequirementsMaxIndirectSequenceCount) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dgc_props);

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
    req_info.maxSequenceCount = dgc_props.maxIndirectSequenceCount + 1;
    req_info.indirectExecutionSet = VK_NULL_HANDLE;
    req_info.indirectCommandsLayout = command_layout.handle();

    VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsMemoryRequirementsInfoEXT-maxSequencesCount-11009");
    vk::GetGeneratedCommandsMemoryRequirementsEXT(device(), &req_info, &mem_req2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, GetRequirementsExecutionSetToken) {
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
    VkGeneratedCommandsPipelineInfoEXT pipeline_info = vku::InitStructHelper();
    pipeline_info.pipeline = pipe.Handle();

    VkGeneratedCommandsMemoryRequirementsInfoEXT req_info = vku::InitStructHelper(&pipeline_info);
    req_info.maxSequenceCount = 1;
    req_info.indirectExecutionSet = VK_NULL_HANDLE;
    req_info.indirectCommandsLayout = command_layout.handle();

    VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsMemoryRequirementsInfoEXT-indirectCommandsLayout-11010");
    vk::GetGeneratedCommandsMemoryRequirementsEXT(device(), &req_info, &mem_req2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, GetRequirementsNullIES) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsLayoutTokenEXT token = vku::InitStructHelper();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    token.offset = 0;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 1;
    command_layout_ci.pTokens = &token;
    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);

    VkGeneratedCommandsMemoryRequirementsInfoEXT req_info = vku::InitStructHelper();
    req_info.maxSequenceCount = 1;
    req_info.indirectExecutionSet = VK_NULL_HANDLE;
    req_info.indirectCommandsLayout = command_layout.handle();

    VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsMemoryRequirementsInfoEXT-indirectExecutionSet-11012");
    vk::GetGeneratedCommandsMemoryRequirementsEXT(device(), &req_info, &mem_req2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, GetRequirementsNoExecutionTokenNullIES) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkIndirectCommandsLayoutTokenEXT token = vku::InitStructHelper();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    token.offset = 0;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 1;
    command_layout_ci.pTokens = &token;
    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateGraphicsPipeline();
    vkt::IndirectExecutionSet exe_set(*m_device, pipe.Handle(), 1);

    VkGeneratedCommandsMemoryRequirementsInfoEXT req_info = vku::InitStructHelper();
    req_info.maxSequenceCount = 1;
    req_info.indirectExecutionSet = exe_set.handle();
    req_info.indirectCommandsLayout = command_layout.handle();

    VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsMemoryRequirementsInfoEXT-indirectCommandsLayout-11011");
    vk::GetGeneratedCommandsMemoryRequirementsEXT(device(), &req_info, &mem_req2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, ExecuteNoBoundPipeline) {
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
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteGeneratedCommandsEXT-indirectCommandsLayout-11053");
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceGeneratedCommands, ExecuteNoBoundShaderObject) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
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
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteGeneratedCommandsEXT-indirectCommandsLayout-11053");
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceGeneratedCommands, ExecuteIsPreprocessed) {
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
    command_layout_ci.flags = VK_INDIRECT_COMMANDS_LAYOUT_USAGE_EXPLICIT_PREPROCESS_BIT_EXT;
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
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteGeneratedCommandsEXT-indirectCommandsLayout-11141");
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceGeneratedCommands, PreprocessNoBoundPipeline) {
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
    command_layout_ci.flags = VK_INDIRECT_COMMANDS_LAYOUT_USAGE_EXPLICIT_PREPROCESS_BIT_EXT;
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

    vkt::CommandBuffer state_cb(*m_device, m_command_pool);
    state_cb.Begin();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPreprocessGeneratedCommandsEXT-indirectCommandsLayout-11084");
    vk::CmdPreprocessGeneratedCommandsEXT(m_command_buffer.handle(), &generated_commands_info, state_cb.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    state_cb.End();
}

TEST_F(NegativeDeviceGeneratedCommands, PreprocessRecordingState) {
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
    command_layout_ci.flags = VK_INDIRECT_COMMANDS_LAYOUT_USAGE_EXPLICIT_PREPROCESS_BIT_EXT;
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

    vkt::CommandBuffer state_cb(*m_device, m_command_pool);
    state_cb.Begin();
    vk::CmdBindPipeline(state_cb.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    state_cb.End();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPreprocessGeneratedCommandsEXT-stateCommandBuffer-11138");
    vk::CmdPreprocessGeneratedCommandsEXT(m_command_buffer.handle(), &generated_commands_info, state_cb.handle());
    m_errorMonitor->VerifyFound();

    state_cb.Begin();
    vk::CmdBindPipeline(state_cb.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    state_cb.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPreprocessGeneratedCommandsEXT-stateCommandBuffer-11138");
    vk::CmdPreprocessGeneratedCommandsEXT(m_command_buffer.handle(), &generated_commands_info, state_cb.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceGeneratedCommands, PreprocessCommandLayoutFlag) {
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

    vkt::CommandBuffer state_cb(*m_device, m_command_pool);
    state_cb.Begin();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(state_cb.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    m_errorMonitor->SetDesiredError("VUID-vkCmdPreprocessGeneratedCommandsEXT-pGeneratedCommandsInfo-11082");
    vk::CmdPreprocessGeneratedCommandsEXT(m_command_buffer.handle(), &generated_commands_info, state_cb.handle());
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
    state_cb.End();
}

TEST_F(NegativeDeviceGeneratedCommands, GeneratedCommandsInfoDynamicVertex) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkIndirectCommandsExecutionSetTokenEXT exe_set_token = {VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT,
                                                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT};
    VkIndirectCommandsVertexBufferTokenEXT vertex_buffer_token = {0};
    VkIndirectCommandsLayoutTokenEXT tokens[3];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT;
    tokens[0].data.pExecutionSet = &exe_set_token;
    tokens[0].offset = 0;

    tokens[1] = vku::InitStructHelper();
    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT;
    tokens[1].data.pVertexBuffer = &vertex_buffer_token;
    tokens[1].offset = 8;

    tokens[2] = vku::InitStructHelper();
    tokens[2].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    tokens[2].offset = 16;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 3;
    command_layout_ci.pTokens = tokens;
    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);  // Missing VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE
    pipe.CreateGraphicsPipeline();
    vkt::IndirectExecutionSet exe_set(*m_device, pipe.Handle(), 1);

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
    m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsInfoEXT-indirectCommandsLayout-11079");
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceGeneratedCommands, GeneratedCommandsInfoAddresses) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkIndirectCommandsLayoutTokenEXT token;
    token = vku::InitStructHelper();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT;
    token.offset = 0;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_COMPUTE_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 1;
    command_layout_ci.pTokens = &token;
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
    generated_commands_info.indirectAddressSize = 0;
    generated_commands_info.indirectAddress = 0;
    generated_commands_info.preprocessAddress = 0;
    generated_commands_info.sequenceCountAddress = 3;
    generated_commands_info.maxSequenceCount = 0;
    generated_commands_info.maxDrawCount = 1;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsInfoEXT-sequenceCountAddress-11073");
    m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsInfoEXT-maxSequenceCount-10246");
    m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsInfoEXT-indirectAddress-11076");
    m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsInfoEXT-indirectAddressSize-11077");
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceGeneratedCommands, GeneratedCommandsInfoMultiDrawLimit) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkIndirectCommandsExecutionSetTokenEXT exe_set_token = {VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT,
                                                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT};
    VkIndirectCommandsLayoutTokenEXT tokens[2];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT;
    tokens[0].data.pExecutionSet = &exe_set_token;
    tokens[0].offset = 0;

    tokens[1] = vku::InitStructHelper();
    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_COUNT_EXT;
    tokens[1].offset = 8;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 2;
    command_layout_ci.pTokens = tokens;
    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);

    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.CreateGraphicsPipeline();
    vkt::IndirectExecutionSet exe_set(*m_device, pipe.Handle(), 1);

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
    generated_commands_info.maxDrawCount = 1 << 21;
    generated_commands_info.maxSequenceCount = 1 << 4;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsInfoEXT-maxDrawCount-11078");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkGeneratedCommandsInfoEXT-preprocessAddress-11063");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkGeneratedCommandsInfoEXT-preprocessSize-11071");
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceGeneratedCommands, ExecuteStageMismatch) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

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
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 2;
    command_layout_ci.pTokens = tokens;
    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);

    // Pipeline (and IES) have no fragment stage
    VkPipelineCreateFlags2CreateInfo pipe_flags2 = vku::InitStructHelper();
    pipe_flags2.flags = VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT;
    CreatePipelineHelper pipe(*this, &pipe_flags2);
    pipe.VertexShaderOnly();
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();
    vkt::IndirectExecutionSet exe_set(*m_device, pipe.Handle(), 1);

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer block_buffer(*m_device, 64, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, kHostVisibleMemProps, &allocate_flag_info);

    VkGeneratedCommandsInfoEXT generated_commands_info = vku::InitStructHelper();
    generated_commands_info.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    generated_commands_info.indirectExecutionSet = exe_set.handle();
    generated_commands_info.indirectCommandsLayout = command_layout.handle();
    generated_commands_info.indirectAddressSize = 64;
    generated_commands_info.indirectAddress = block_buffer.Address();
    generated_commands_info.preprocessAddress = 0;
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 1;
    generated_commands_info.maxDrawCount = 1;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsInfoEXT-indirectCommandsLayout-11002");
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceGeneratedCommands, ExecutePreprocessBufferUsage) {
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

    {
        generated_commands_info.preprocessAddress = block_buffer.Address();  // missing usage
        m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsInfoEXT-preprocessAddress-11069");
        vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
        m_errorMonitor->VerifyFound();
    }

    {
        VkBufferUsageFlags2CreateInfo buffer_usage_flags = vku::InitStructHelper();
        buffer_usage_flags.usage = VK_BUFFER_USAGE_2_PREPROCESS_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage_flags);
        buffer_ci.size = 1024;
        vkt::Buffer bad_buffer(*m_device, buffer_ci, 0, &allocate_flag_info);

        generated_commands_info.preprocessAddress = bad_buffer.Address();
        bad_buffer.Memory().destroy();
        m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsInfoEXT-preprocessAddress-11070");
        vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
        m_errorMonitor->VerifyFound();
    }
    m_command_buffer.End();
}

TEST_F(NegativeDeviceGeneratedCommands, ExecuteSequenceCountBufferUsage) {
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
    generated_commands_info.maxSequenceCount = 1;
    generated_commands_info.maxDrawCount = 1;
    SetPreProcessBuffer(generated_commands_info);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());

    {
        generated_commands_info.sequenceCountAddress = block_buffer.Address();  // missing usage
        m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsInfoEXT-sequenceCountAddress-11072");
        vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
        m_errorMonitor->VerifyFound();
    }

    {
        VkBufferUsageFlags2CreateInfo buffer_usage_flags = vku::InitStructHelper();
        buffer_usage_flags.usage =
            VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage_flags);
        buffer_ci.size = 1024;
        vkt::Buffer bad_buffer(*m_device, buffer_ci, 0, &allocate_flag_info);

        generated_commands_info.sequenceCountAddress = bad_buffer.Address();
        bad_buffer.Memory().destroy();
        m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsInfoEXT-sequenceCountAddress-11075");
        vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
        m_errorMonitor->VerifyFound();
    }
    m_command_buffer.End();
}

TEST_F(NegativeDeviceGeneratedCommands, ExecuteShaderObjectStages) {
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    InitRenderTarget();

    VkIndirectCommandsExecutionSetTokenEXT exe_set_token = {VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT,
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
    generated_commands_info.preprocessAddress = 0;
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 1;
    generated_commands_info.maxDrawCount = 1;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT};
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1u, stages, shaders);
    SetDefaultDynamicStatesAll(m_command_buffer.handle());
    m_errorMonitor->SetDesiredError("VUID-VkGeneratedCommandsInfoEXT-indirectCommandsLayout-11002");
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceGeneratedCommands, InitialPipelineObject) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    VkPipeline invalid_pipeline = CastToHandle<VkPipeline, uintptr_t>(0xbaadbeef);

    VkIndirectExecutionSetPipelineInfoEXT exe_set_pipeline_info = vku::InitStructHelper();
    exe_set_pipeline_info.initialPipeline = invalid_pipeline;
    exe_set_pipeline_info.maxPipelineCount = 1;

    VkIndirectExecutionSetCreateInfoEXT exe_set_ci = vku::InitStructHelper();
    exe_set_ci.type = VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT;
    exe_set_ci.info.pPipelineInfo = &exe_set_pipeline_info;

    VkIndirectExecutionSetEXT exe_set = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectExecutionSetPipelineInfoEXT-initialPipeline-parameter");
    vk::CreateIndirectExecutionSetEXT(device(), &exe_set_ci, nullptr, &exe_set);
    m_errorMonitor->VerifyFound();

    exe_set_pipeline_info.initialPipeline = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("pCreateInfo->info.pPipelineInfo->initialPipeline is VK_NULL_HANDLE");
    vk::CreateIndirectExecutionSetEXT(device(), &exe_set_ci, nullptr, &exe_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, NullIndirectExecutionSetEXT) {
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());
    VkIndirectExecutionSetEXT exe_set = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-vkCreateIndirectExecutionSetEXT-pCreateInfo-parameter");
    vk::CreateIndirectExecutionSetEXT(device(), nullptr, nullptr, &exe_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceGeneratedCommands, InitialShaderObject) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDeviceGeneratedCommands());

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    VkIndirectExecutionSetShaderLayoutInfoEXT exe_set_layouts = vku::InitStructHelper();
    exe_set_layouts.setLayoutCount = 1;
    exe_set_layouts.pSetLayouts = &descriptor_set.layout_.handle();

    VkShaderEXT invalid_shader = CastToHandle<VkShaderEXT, uintptr_t>(0xbaadbeef);
    VkIndirectExecutionSetShaderInfoEXT exe_set_shader_info = vku::InitStructHelper();
    exe_set_shader_info.shaderCount = 1;
    exe_set_shader_info.pInitialShaders = &invalid_shader;
    exe_set_shader_info.pSetLayoutInfos = &exe_set_layouts;
    exe_set_shader_info.maxShaderCount = 1;
    exe_set_shader_info.pushConstantRangeCount = 0;

    VkIndirectExecutionSetCreateInfoEXT exe_set_ci = vku::InitStructHelper();
    exe_set_ci.type = VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT;
    exe_set_ci.info.pShaderInfo = &exe_set_shader_info;

    VkIndirectExecutionSetEXT exe_set = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectExecutionSetShaderInfoEXT-pInitialShaders-parameter");
    vk::CreateIndirectExecutionSetEXT(device(), &exe_set_ci, nullptr, &exe_set);
    m_errorMonitor->VerifyFound();

    invalid_shader = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectExecutionSetShaderInfoEXT-pInitialShaders-parameter");
    vk::CreateIndirectExecutionSetEXT(device(), &exe_set_ci, nullptr, &exe_set);
    m_errorMonitor->VerifyFound();
}