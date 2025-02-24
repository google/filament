/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_object_helper.h"

class NegativeShaderPushConstants : public VkLayerTest {};

TEST_F(NegativeShaderPushConstants, NotDeclared) {
    TEST_DESCRIPTION(
        "Create a graphics pipeline in which a push constant range containing a push constant block member is not declared in the "
        "layout.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { float x; } consts;
        void main(){
           gl_Position = vec4(consts.x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    // Set up a push constant range
    VkPushConstantRange push_constant_range = {};
    // Set to the wrong stage to challenge core_validation
    push_constant_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.size = 4;

    const vkt::PipelineLayout pipeline_layout(*m_device, {}, {push_constant_range});

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {}, {push_constant_range});

    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-layout-07987");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderPushConstants, PipelineRange) {
    TEST_DESCRIPTION("Invalid use of VkPushConstantRange structs.");
    RETURN_IF_SKIP(Init());

    // will be at least 256 as required from the spec
    const uint32_t maxPushConstantsSize = m_device->Physical().limits_.maxPushConstantsSize;

    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkPushConstantRange push_constant_range = {0, 0, 4};
    VkPipelineLayoutCreateInfo pipeline_layout_info{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};

    // stageFlags of 0
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-stageFlags-requiredbitmask");
    vk::CreatePipelineLayout(device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // offset over limit
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, maxPushConstantsSize, 8};
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-offset-00294");
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-size-00298");
    vk::CreatePipelineLayout(device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // offset not a multiple of 4
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 1, 8};
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-offset-00295");
    vk::CreatePipelineLayout(device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // size of 0
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 0};
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-size-00296");
    vk::CreatePipelineLayout(device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // size not a multiple of 4
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 7};
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-size-00297");
    vk::CreatePipelineLayout(device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // size over limit
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, maxPushConstantsSize + 4};
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-size-00298");
    vk::CreatePipelineLayout(device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // size over limit of non-zero offset
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 4, maxPushConstantsSize};
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-size-00298");
    vk::CreatePipelineLayout(device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // Sanity check its a valid range before making duplicate
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, maxPushConstantsSize};
    ASSERT_EQ(VK_SUCCESS, vk::CreatePipelineLayout(device(), &pipeline_layout_info, NULL, &pipeline_layout));
    vk::DestroyPipelineLayout(device(), pipeline_layout, nullptr);

    // Duplicate ranges
    VkPushConstantRange push_constant_range_duplicate[2] = {push_constant_range, push_constant_range};
    pipeline_layout_info.pushConstantRangeCount = 2;
    pipeline_layout_info.pPushConstantRanges = push_constant_range_duplicate;
    m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292");
    vk::CreatePipelineLayout(device(), &pipeline_layout_info, nullptr, &pipeline_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderPushConstants, PipelineRangeShaderObject) {
    TEST_DESCRIPTION("Invalid use of VkPushConstantRange structs.");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(Init());

    vkt::Shader shader;
    VkPushConstantRange push_constant_range = {0, 0, 4};
    const auto spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderCreateInfoEXT ci_info = ShaderCreateInfo(spv, VK_SHADER_STAGE_VERTEX_BIT, 0, nullptr, 1, &push_constant_range);

    // stageFlags of 0
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-stageFlags-requiredbitmask");
    shader.init(*m_device, ci_info);
    m_errorMonitor->VerifyFound();

    // will be at least 256 as required from the spec
    const uint32_t maxPushConstantsSize = m_device->Physical().limits_.maxPushConstantsSize;

    // offset over limit
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, maxPushConstantsSize, 8};
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-offset-00294");
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-size-00298");
    shader.init(*m_device, ci_info);
    m_errorMonitor->VerifyFound();

    // offset not a multiple of 4
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 1, 8};
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-offset-00295");
    shader.init(*m_device, ci_info);
    m_errorMonitor->VerifyFound();

    // size of 0
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 0};
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-size-00296");
    shader.init(*m_device, ci_info);
    m_errorMonitor->VerifyFound();

    // size not a multiple of 4
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 7};
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-size-00297");
    shader.init(*m_device, ci_info);
    m_errorMonitor->VerifyFound();

    // size over limit
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, maxPushConstantsSize + 4};
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-size-00298");
    shader.init(*m_device, ci_info);
    m_errorMonitor->VerifyFound();

    // size over limit of non-zero offset
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 4, maxPushConstantsSize};
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-size-00298");
    shader.init(*m_device, ci_info);
    m_errorMonitor->VerifyFound();

    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, maxPushConstantsSize};
    // Duplicate ranges
    VkPushConstantRange push_constant_range_duplicate[2] = {push_constant_range, push_constant_range};
    ci_info.pushConstantRangeCount = 2;
    ci_info.pPushConstantRanges = push_constant_range_duplicate;
    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-pPushConstantRanges-10063");
    shader.init(*m_device, ci_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderPushConstants, NotInLayout) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a shader consuming push constants which are not provided in the pipeline layout");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { float x; } consts;
        void main(){
           gl_Position = vec4(consts.x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {});
    /* should have generated an error -- no push constant ranges provided! */
    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-layout-07987");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderPushConstants, Range) {
    TEST_DESCRIPTION("Invalid use of VkPushConstantRange values in vkCmdPushConstants.");

    RETURN_IF_SKIP(InitFramework());

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    // Set limit to be same max as the shader usages
    const uint32_t maxPushConstantsSize = 16;
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    props.limits.maxPushConstantsSize = maxPushConstantsSize;
    fpvkSetPhysicalDeviceLimitsEXT(Gpu(), &props.limits);

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *const vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { float x[4]; } constants;
        void main(){
           gl_Position = vec4(constants.x[0]);
        }
    )glsl";

    VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj const fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Set up a push constant range
    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, maxPushConstantsSize};
    const vkt::PipelineLayout pipeline_layout(*m_device, {}, {push_constant_range});

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {}, {push_constant_range});
    pipe.CreateGraphicsPipeline();

    const float data[16] = {};  // dummy data to match shader size

    m_command_buffer.Begin();

    // size of 0
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushConstants-size-arraylength");
    vk::CmdPushConstants(m_command_buffer.handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, 0, data);
    m_errorMonitor->VerifyFound();

    // offset not a multiple of 4
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushConstants-offset-00368");
    vk::CmdPushConstants(m_command_buffer.handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 1, 4, data);
    m_errorMonitor->VerifyFound();

    // size not a multiple of 4
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushConstants-size-00369");
    vk::CmdPushConstants(m_command_buffer.handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, 5, data);
    m_errorMonitor->VerifyFound();

    // offset at limit
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushConstants-offset-00370");
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushConstants-size-00371");
    vk::CmdPushConstants(m_command_buffer.handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT,
                         maxPushConstantsSize, 4, data);
    m_errorMonitor->VerifyFound();

    // size at limit
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushConstants-size-00371");
    vk::CmdPushConstants(m_command_buffer.handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                         maxPushConstantsSize + 4, data);
    m_errorMonitor->VerifyFound();

    // Size at limit, should be valid
    vk::CmdPushConstants(m_command_buffer.handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                         maxPushConstantsSize, data);

    m_command_buffer.End();
}

TEST_F(NegativeShaderPushConstants, DrawWithoutUpdate) {
    TEST_DESCRIPTION("Not every bytes in used push constant ranges has been set before Draw ");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // push constant range: 0-99
    char const *const vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo {
           bool b;
           float f2[3];
           vec3 v;
           vec4 v2[2];
           mat3 m;
        } constants;
        void func1( float f ){
           // use the whole v2[1]. byte: 48-63.
           vec2 v2 = constants.v2[1].yz;
        }
        void main(){
            // use only v2[0].z. byte: 40-43.
            func1( constants.v2[0].z);
            // index of m is variable. The all m is used. byte: 64-99.
            for(int i=1;i<2;++i) {
                vec3 v3 = constants.m[i];
            }
        }
    )glsl";

    // push constant range: 0 - 95
    char const *const fsSource = R"glsl(
        #version 450
        struct foo1{
           int i[4];
        }f;
        layout(push_constant, std430) uniform foo {
           float x[2][2][2];
           foo1 s;
           foo1 ss[3];
        } constants;
        void main(){
            // use s. byte: 32-47.
            f = constants.s;
            // use every i[3] in ss. byte: 60-63, 76-79, 92-95.
            for(int i=1;i<2;++i) {
                int ii = constants.ss[i].i[3];
            }
        }
    )glsl";

    VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj const fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, 128};
    VkPushConstantRange push_constant_range_small = {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 4, 4};

    VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_info);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.pipeline_layout_ci_ = pipeline_layout_info;
    g_pipe.CreateGraphicsPipeline();

    pipeline_layout_info.pPushConstantRanges = &push_constant_range_small;
    vkt::PipelineLayout pipeline_layout_small(*m_device, pipeline_layout_info);

    CreatePipelineHelper g_pipe_small_range(*this);
    g_pipe_small_range.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe_small_range.pipeline_layout_ci_ = pipeline_layout_info;

    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-layout-10069");
    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-layout-10069");
    g_pipe_small_range.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-maintenance4-08602");  // vertex
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-maintenance4-08602");  // fragment
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    const float dummy_values[128] = {};

    // NOTE: these are commented out due to ambiguity around VUID 02698 and push constant lifetimes
    //       See https://gitlab.khronos.org/vulkan/vulkan/-/issues/2602 and
    //       https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2689
    //       for more details.
    // m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-maintenance4-08602");
    // vk::CmdPushConstants(m_command_buffer.handle(), g_pipe.pipeline_layout_.handle(),
    //                     VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, 96, dummy_values);
    // vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    // m_errorMonitor->VerifyFound();

    // m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-maintenance4-08602");
    // vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout_small, VK_SHADER_STAGE_VERTEX_BIT, 4, 4, dummy_values);
    // vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    // m_errorMonitor->VerifyFound();

    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(),
                         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 32, 68, dummy_values);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeShaderPushConstants, MultipleEntryPoint) {
    TEST_DESCRIPTION("Test push-constant detect the write entrypoint with the push constants.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // #version 460
    // layout(push_constant) uniform Material {
    //     vec4 color;
    // }constants;
    // void main() {
    //     gl_Position = constants.color;
    // }
    //
    // #version 460
    // layout(location = 0) out vec4 uFragColor;
    // void main(){
    //     uFragColor = vec4(0.0);
    // }
    const char *source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main_f "main" %4
               OpEntryPoint Vertex %main_v "main" %2
               OpExecutionMode %main_f OriginUpperLeft
               OpMemberDecorate %builtin_vert 0 BuiltIn Position
               OpMemberDecorate %builtin_vert 1 BuiltIn PointSize
               OpMemberDecorate %builtin_vert 2 BuiltIn ClipDistance
               OpMemberDecorate %builtin_vert 3 BuiltIn CullDistance
               OpDecorate %builtin_vert Block
               OpMemberDecorate %struct_pc 0 Offset 0
               OpDecorate %struct_pc Block
               OpDecorate %4 Location 0
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
      %float = OpTypeFloat 32
            ; Vertex types
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
      %array = OpTypeArray %float %uint_1
  %builtin_vert = OpTypeStruct %v4float %float %array %array
%ptr_builtin_vert = OpTypePointer Output %builtin_vert
          %2 = OpVariable %ptr_builtin_vert Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
  %struct_pc = OpTypeStruct %v4float
%ptr_pc_struct = OpTypePointer PushConstant %struct_pc
         %18 = OpVariable %ptr_pc_struct PushConstant
%ptr_pc_vec4 = OpTypePointer PushConstant %v4float
%ptr_output_vert = OpTypePointer Output %v4float
            ; Fragment types
%ptr_output_frag = OpTypePointer Output %v4float
          %4 = OpVariable %ptr_output_frag Output
    %float_0 = OpConstant %float 0
         %23 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0

     %main_v = OpFunction %void None %8
         %24 = OpLabel
         %25 = OpAccessChain %ptr_pc_vec4 %18 %int_0
         %26 = OpLoad %v4float %25
         %27 = OpAccessChain %ptr_output_vert %2 %int_0
               OpStore %27 %26
               OpReturn
               OpFunctionEnd

     %main_f = OpFunction %void None %8
         %28 = OpLabel
               OpStore %4 %23
               OpReturn
               OpFunctionEnd
    )";

    // Push constant are in the vertex Entrypoint
    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_FRAGMENT_BIT, 0, 16};
    VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_info);

    VkShaderObj const vs(this, source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj const fs(this, source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();

    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-layout-07987");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    // Make sure Vertex is ok when used
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    vkt::PipelineLayout pipeline_layout_good(*m_device, pipeline_layout_info);
    pipe.gp_ci_.layout = pipeline_layout_good.handle();
    pipe.CreateGraphicsPipeline();
}

// This is not working because of a bug in the Spec Constant logic
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5911
TEST_F(NegativeShaderPushConstants, DISABLED_SpecConstantSize) {
    TEST_DESCRIPTION("Use SpecConstant to adjust size of Push Constant Block");
    RETURN_IF_SKIP(Init());

    const char *cs_source = R"glsl(
        #version 460
        layout (constant_id = 0) const int my_array_size = 1;
        layout (push_constant) uniform my_buf {
            float my_array[my_array_size];
        } pc;

        void main() {
            float a = pc.my_array[0];
        }
    )glsl";

    uint32_t data = 32;

    VkSpecializationMapEntry entry;
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    // With spec constant set, this should be 32, not 16
    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, 16};
    const vkt::PipelineLayout pipeline_layout(*m_device, {}, {push_constant_range});

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                                             &specialization_info);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {}, {push_constant_range});
    m_errorMonitor->SetDesiredError("VUID-VkComputePipelineCreateInfo-layout-07987");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderPushConstants, ArrayOf8Bit) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9364");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    // storagePushConstant8 is not enabled
    RETURN_IF_SKIP(Init());

    const char *vs_source = R"glsl(
        #version 450
        #extension GL_EXT_shader_8bit_storage: enable
        #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
        layout(push_constant) uniform PushConstant {
            int8_t x[4];
        } data;

        void main(){
            gl_Position = vec4(float(data.x[0]) * 0.0);
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storagePushConstant8-06330");  // feature
    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");     // capability
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderPushConstants, StructOf8Bit) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9364");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    // storagePushConstant8 is not enabled
    RETURN_IF_SKIP(Init());

    const char *vs_source = R"glsl(
        #version 450
        #extension GL_EXT_shader_8bit_storage: enable
        #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable

        struct Foo {
            uint a;
            int8_t b;
            vec4 c;
        };

        layout(push_constant) uniform PushConstant {
            Foo x;
        } data;

        void main(){
            gl_Position = data.x.c;
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storagePushConstant8-06330");  // feature
    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");     // capability
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderPushConstants, ArrayOfStructOf8Bit) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9364");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    // storagePushConstant8 is not enabled
    RETURN_IF_SKIP(Init());

    const char *vs_source = R"glsl(
        #version 450
        #extension GL_EXT_shader_8bit_storage: enable
        #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable

        struct Foo {
            uint a;
            int8_t b[2];
            vec4 c;
        };

        layout(push_constant) uniform PushConstant {
            Foo x[2];
        } data;

        void main(){
            gl_Position = data.x[1].c;
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-storagePushConstant8-06330");  // feature
    m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");     // capability
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    m_errorMonitor->VerifyFound();
}