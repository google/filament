/*
 * Copyright (c) 2024 The Khronos Group Inc.
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

class NegativeDebugPrintfShaderDebugInfo : public DebugPrintfTests {};

// These tests print out the verbose info to make sure that info is correct
static const VkBool32 verbose_value = true;
static const VkLayerSettingEXT layer_setting = {OBJECT_LAYER_NAME, "printf_verbose", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                                &verbose_value};
static VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                                  &layer_setting};

TEST_F(NegativeDebugPrintfShaderDebugInfo, PipelineHandle) {
    TEST_DESCRIPTION("Make sure we are printing out which pipeline the error is from");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    const char *object_name = "bad_pipeline";
    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    name_info.objectHandle = (uint64_t)pipe.Handle();
    name_info.pObjectName = object_name;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("Pipeline (bad_pipeline)");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfShaderDebugInfo, ShaderObjectHandle) {
    TEST_DESCRIPTION("Make sure we are printing out which shader object the error is from");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    VkShaderStageFlagBits shader_stages[] = {VK_SHADER_STAGE_COMPUTE_BIT};
    const vkt::Shader comp_shader(*m_device, shader_stages[0], GLSLToSPV(shader_stages[0], shader_source));

    const char *object_name = "bad_shader_object";
    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_SHADER_EXT;
    name_info.objectHandle = (uint64_t)comp_shader.handle();
    name_info.pObjectName = object_name;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    m_command_buffer.Begin();
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1, shader_stages, &comp_shader.handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("Shader Object (bad_shader_object)");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfShaderDebugInfo, OpLine) {
    TEST_DESCRIPTION("Make sure OpLine works");
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %2 = OpExtInstImport "GLSL.std.450"
         %13 = OpExtInstImport "NonSemantic.DebugPrintf"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %1 = OpString "a.comp"
         %11 = OpString "float == %f"
               OpSource GLSL 450 %1 "// OpModuleProcessed client vulkan100
// OpModuleProcessed target-env vulkan1.0
// OpModuleProcessed entry-point main
#line 1
#version 450
#extension GL_EXT_debug_printf : enable
void main() {
    float myfloat = 3.1415f;
    debugPrintfEXT(\"float == %f\", myfloat);
}"
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_3_1415 = OpConstant %float 3.1415
               OpLine %1 3 11
       %main = OpFunction %void None %4
          %6 = OpLabel
    %myfloat = OpVariable %_ptr_Function_float Function
               OpLine %1 4 0
               OpStore %myfloat %float_3_1415
               OpLine %1 5 0
         %12 = OpLoad %float %myfloat
         %14 = OpExtInst %void %13 1 %11 %12
               OpReturn
               OpFunctionEnd
    )";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredFailureMsg(
        kInformationBit,
        "Debug shader printf message generated in file a.comp at line 5\n\n5:     debugPrintfEXT(\"float == %f\", myfloat);");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfShaderDebugInfo, ShaderDebugInfoDebugLine) {
    TEST_DESCRIPTION("Make sure DebugLine works");
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());

    // Manually ran:
    //   glslangValidator -V -gVS in.comp -o out.spv --target-env vulkan1.0
    char const *shader_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
         %44 = OpExtInstImport "NonSemantic.DebugPrintf"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %16 = OpString "main"
         %19 = OpString "#version 450
#extension GL_EXT_debug_printf : enable
void main() {
    float myfloat = 3.1415f;
    debugPrintfEXT(\"float == %f\", myfloat);
}"
         %28 = OpString "float"
         %35 = OpString "myfloat"
         %40 = OpString "float == %f"
               OpSourceExtension "GL_EXT_debug_printf"
               OpName %main "main"
               OpName %myfloat "myfloat"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
     %uint_0 = OpConstant %uint 0
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
     %uint_3 = OpConstant %uint 3
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
         %18 = OpExtInst %void %1 DebugSource %2 %19
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %20 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %18 %uint_2
         %17 = OpExtInst %void %1 DebugFunction %16 %6 %18 %uint_3 %uint_0 %20 %16 %uint_3 %uint_3
      %float = OpTypeFloat 32
         %29 = OpExtInst %void %1 DebugTypeBasic %28 %uint_32 %uint_3 %uint_0
%_ptr_Function_float = OpTypePointer Function %float
     %uint_7 = OpConstant %uint 7
         %32 = OpExtInst %void %1 DebugTypePointer %29 %uint_7 %uint_0
         %34 = OpExtInst %void %1 DebugLocalVariable %35 %29 %18 %uint_4 %uint_0 %17 %uint_4
         %37 = OpExtInst %void %1 DebugExpression
%float_3_1415 = OpConstant %float 3.1415
     %uint_5 = OpConstant %uint 5
       %main = OpFunction %void None %5
         %15 = OpLabel
    %myfloat = OpVariable %_ptr_Function_float Function
         %25 = OpExtInst %void %1 DebugScope %17
         %26 = OpExtInst %void %1 DebugLine %18 %uint_3 %uint_3 %uint_0 %uint_0
         %24 = OpExtInst %void %1 DebugFunctionDefinition %17 %main
         %38 = OpExtInst %void %1 DebugLine %18 %uint_4 %uint_4 %uint_0 %uint_0
         %36 = OpExtInst %void %1 DebugDeclare %34 %myfloat %37
               OpStore %myfloat %float_3_1415
         %42 = OpExtInst %void %1 DebugLine %18 %uint_5 %uint_5 %uint_0 %uint_0
         %41 = OpLoad %float %myfloat
         %45 = OpExtInst %void %44 1 %40 %41
         %46 = OpExtInst %void %1 DebugLine %18 %uint_6 %uint_6 %uint_0 %uint_0
               OpReturn
               OpFunctionEnd
    )";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredFailureMsg(
        kInformationBit,
        "Debug shader printf message generated in file a.comp at line 5\n\n5:     debugPrintfEXT(\"float == %f\", myfloat);");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfShaderDebugInfo, CommandBufferCommandIndex) {
    TEST_DESCRIPTION("Make sure we print which index in the command buffer the issue occured");
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    CreateComputePipelineHelper empty_pipe(*this);
    empty_pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, empty_pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("Compute Dispatch Index 3");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfShaderDebugInfo, CommandBufferCommandIndexMulti) {
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(push_constant) uniform PushConstants { int x; } pc;
        void main() {
            if (pc.x > 0) {
                debugPrintfEXT("int == %u", pc.x);
            }
        }
    )glsl";

    VkPushConstantRange pc_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    VkPipelineLayoutCreateInfo pipe_layout_ci = vku::InitStructHelper();
    pipe_layout_ci.pushConstantRangeCount = 1;
    pipe_layout_ci.pPushConstantRanges = &pc_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipe_layout_ci);

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    uint32_t skip = 0;
    uint32_t good = 4;
    cb0.Begin();
    vk::CmdBindPipeline(cb0.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdPushConstants(cb0.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &skip);
    vk::CmdDispatch(cb0.handle(), 1, 1, 1);
    vk::CmdDispatch(cb0.handle(), 1, 1, 1);
    vk::CmdDispatch(cb0.handle(), 1, 1, 1);
    vk::CmdPushConstants(cb0.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &good);
    vk::CmdDispatch(cb0.handle(), 1, 1, 1);
    vk::CmdPushConstants(cb0.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &skip);
    vk::CmdDispatch(cb0.handle(), 1, 1, 1);
    cb0.End();

    cb1.Begin();
    vk::CmdBindPipeline(cb1.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdPushConstants(cb1.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &good);
    vk::CmdDispatch(cb1.handle(), 1, 1, 1);
    vk::CmdPushConstants(cb1.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &skip);
    vk::CmdDispatch(cb1.handle(), 1, 1, 1);
    cb1.End();

    m_errorMonitor->SetDesiredInfo("Compute Dispatch Index 3");  // cb0
    m_errorMonitor->SetDesiredInfo("Compute Dispatch Index 0");  // cb1

    VkCommandBuffer cbs[2] = {cb0.handle(), cb1.handle()};
    VkSubmitInfo submit = vku::InitStructHelper();
    submit.commandBufferCount = 2;
    submit.pCommandBuffers = cbs;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit, VK_NULL_HANDLE);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfShaderDebugInfo, StageInfo) {
    TEST_DESCRIPTION("Make sure we print the stage info correctly");
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            if (gl_GlobalInvocationID.x == 3 && gl_GlobalInvocationID.y == 1) {
                debugPrintfEXT("float == %f", myfloat);
            }
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    CreateComputePipelineHelper empty_pipe(*this);
    empty_pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 4, 4, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("Global invocation ID (x, y, z) = (3, 1, 0)");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfShaderDebugInfo, Fragment) {
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();
    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(location = 0) out vec4 outColor;

        void main() {
            if (gl_FragCoord.x > 10 && gl_FragCoord.x < 11) {
                if (gl_FragCoord.y > 10 && gl_FragCoord.y < 12) {
                    debugPrintfEXT("gl_FragCoord.xy %1.2f, %1.2f\n", gl_FragCoord.x, gl_FragCoord.y);
                }
            }
            outColor = gl_FragCoord;
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, shader_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("Stage = Fragment.  Fragment coord (x,y) = (10.5, 10.5)");
    m_errorMonitor->SetDesiredInfo("Stage = Fragment.  Fragment coord (x,y) = (10.5, 11.5)");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfShaderDebugInfo, VertexFragmentMultiEntrypoint) {
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // void vert_main() {
    //     gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
    // }
    // layout(location = 0) out vec4 c_out;
    // void frag_main() {
    //     debugPrintfEXT("Fragment value is %i", 8);
    //     c_out = vec4(0.0);
    // }
    const char *shader_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %9 = OpExtInstImport "NonSemantic.DebugPrintf"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %frag_main "frag_main" %c_out
               OpEntryPoint Vertex %vert_main "vert_main" %_ %gl_VertexIndex
               OpExecutionMode %frag_main OriginUpperLeft
   %frag_str = OpString "Fragment value is %i"
               OpDecorate %c_out Location 0
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpDecorate %gl_VertexIndex BuiltIn VertexIndex
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_8 = OpConstant %int 8
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
      %int_0 = OpConstant %int 0
    %v2float = OpTypeVector %float 2
     %uint_3 = OpConstant %uint 3
%_arr_v2float_uint_3 = OpTypeArray %v2float %uint_3
   %float_n1 = OpConstant %float -1
         %24 = OpConstantComposite %v2float %float_n1 %float_n1
    %float_1 = OpConstant %float 1
         %26 = OpConstantComposite %v2float %float_1 %float_n1
    %float_0 = OpConstant %float 0
         %28 = OpConstantComposite %v2float %float_0 %float_1
         %29 = OpConstantComposite %_arr_v2float_uint_3 %24 %26 %28
%_ptr_Input_int = OpTypePointer Input %int
%gl_VertexIndex = OpVariable %_ptr_Input_int Input
      %int_3 = OpConstant %int 3
%_ptr_Function__arr_v2float_uint_3 = OpTypePointer Function %_arr_v2float_uint_3
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %c_out = OpVariable %_ptr_Output_v4float Output
         %16 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %vert_main = OpFunction %void None %3
          %5 = OpLabel
  %indexable = OpVariable %_ptr_Function__arr_v2float_uint_3 Function
         %32 = OpLoad %int %gl_VertexIndex
         %34 = OpSMod %int %32 %int_3
               OpStore %indexable %29
         %38 = OpAccessChain %_ptr_Function_v2float %indexable %34
         %39 = OpLoad %v2float %38
         %40 = OpCompositeExtract %float %39 0
         %41 = OpCompositeExtract %float %39 1
         %42 = OpCompositeConstruct %v4float %40 %41 %float_0 %float_1
         %44 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %44 %42
               OpReturn
               OpFunctionEnd
       %frag_main = OpFunction %void None %3
          %f5 = OpLabel
         %f10 = OpExtInst %void %9 1 %frag_str %int_8
               OpStore %c_out %16
               OpReturn
               OpFunctionEnd
        )";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr, "vert_main");
    VkShaderObj fs(this, shader_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr, "frag_main");

    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    VkRect2D scissor = {{0, 0}, {1, 1}};
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.vp_state_ci_.pViewports = &viewport;
    pipe.vp_state_ci_.pScissors = &scissor;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit,
                                         "Stage has multiple OpEntryPoint (Fragment, Vertex) and could not detect stage");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8924
TEST_F(NegativeDebugPrintfShaderDebugInfo, DISABLED_DebugLabelRegion1) {
    TEST_DESCRIPTION("Print debug label regions in a Debug PrintF message");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            debugPrintfEXT("printf");
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    VkDebugUtilsLabelEXT region_0_label = vku::InitStructHelper();
    region_0_label.pLabelName = "region_0";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer.handle(), &region_0_label);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("region_0");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8924
TEST_F(NegativeDebugPrintfShaderDebugInfo, DISABLED_DebugLabelRegion2) {
    TEST_DESCRIPTION("Print debug label regions in a Debug PrintF message");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDebugPrintfFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
        debugPrintfEXT("printf");
        }
        )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    VkDebugUtilsLabelEXT region_0_label = vku::InitStructHelper();
    region_0_label.pLabelName = "region_0";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer.handle(), &region_0_label);
    VkDebugUtilsLabelEXT region_1_label = vku::InitStructHelper();
    region_1_label.pLabelName = "region_1";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer.handle(), &region_1_label);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredInfo("region_0::region_1");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}
