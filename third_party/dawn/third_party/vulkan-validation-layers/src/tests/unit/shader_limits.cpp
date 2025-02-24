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

class NegativeShaderLimits : public VkLayerTest {};

TEST_F(NegativeShaderLimits, MaxSampleMaskWordsInput) {
    TEST_DESCRIPTION("Test limit of maxSampleMaskWords.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (m_device->Physical().limits_.maxSampleMaskWords > 1) {
        GTEST_SKIP() << "maxSampleMaskWords is greater than 1";
    }

    // layout(location = 0) out vec4 uFragColor;
    // void main(){
    //     int x = gl_SampleMaskIn[3]; // Exceed sample mask input array size
    //     uFragColor = vec4(0,1,0,1) * x;
    // }
    char const *source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %gl_SampleMaskIn %uFragColor
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %gl_SampleMaskIn Flat
               OpDecorate %gl_SampleMaskIn BuiltIn SampleMask
               OpDecorate %uFragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_int_uint_4 = OpTypeArray %int %uint_4
%_ptr_Input__arr_int_uint_4 = OpTypePointer Input %_arr_int_uint_4
%gl_SampleMaskIn = OpVariable %_ptr_Input__arr_int_uint_4 Input
      %int_3 = OpConstant %int 3
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
 %uFragColor = OpVariable %_ptr_Output_v4float Output
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
         %24 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
       %main = OpFunction %void None %3
          %5 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
         %16 = OpAccessChain %_ptr_Input_int %gl_SampleMaskIn %int_3
         %17 = OpLoad %int %16
               OpStore %x %17
         %25 = OpLoad %int %x
         %26 = OpConvertSToF %float %25
         %27 = OpVectorTimesScalar %v4float %24 %26
               OpStore %uFragColor %27
               OpReturn
               OpFunctionEnd
    )";
    VkShaderObj fs(this, source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    const auto inputPipeline = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, inputPipeline, kErrorBit,
                                      "VUID-VkPipelineShaderStageCreateInfo-maxSampleMaskWords-00711");
}

TEST_F(NegativeShaderLimits, MaxSampleMaskWordsOutput) {
    TEST_DESCRIPTION("Test limit of maxSampleMaskWords.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (m_device->Physical().limits_.maxSampleMaskWords > 1) {
        GTEST_SKIP() << "maxSampleMaskWords is greater than 1";
    }

    // layout(location = 0) out vec4 uFragColor;
    // void main(){
    //    gl_SampleMask[3] = 1; // Exceed sample mask output array size
    //    uFragColor = vec4(0,1,0,1);
    // }
    char const *source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %gl_SampleMask %uFragColor
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %gl_SampleMask BuiltIn SampleMask
               OpDecorate %uFragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_int_uint_4 = OpTypeArray %int %uint_4
%_ptr_Output__arr_int_uint_4 = OpTypePointer Output %_arr_int_uint_4
%gl_SampleMask = OpVariable %_ptr_Output__arr_int_uint_4 Output
      %int_3 = OpConstant %int 3
      %int_1 = OpConstant %int 1
%_ptr_Output_int = OpTypePointer Output %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
 %uFragColor = OpVariable %_ptr_Output_v4float Output
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
         %22 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %15 = OpAccessChain %_ptr_Output_int %gl_SampleMask %int_3
               OpStore %15 %int_1
               OpStore %uFragColor %22
               OpReturn
               OpFunctionEnd
    )";
    VkShaderObj fs(this, source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    const auto outputPipeline = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, outputPipeline, kErrorBit,
                                      "VUID-VkPipelineShaderStageCreateInfo-maxSampleMaskWords-00711");
}

TEST_F(NegativeShaderLimits, MinAndMaxTexelGatherOffset) {
    TEST_DESCRIPTION("Test shader with offset less than minTexelGatherOffset and greather than maxTexelGatherOffset");

    RETURN_IF_SKIP(Init());

    if (m_device->Physical().limits_.minTexelGatherOffset <= -100 || m_device->Physical().limits_.maxTexelGatherOffset >= 100) {
        GTEST_SKIP() << "test needs minTexelGatherOffset greater than -100 and maxTexelGatherOffset less than 100";
    }

    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450

               ; Annotations
               OpDecorate %samp DescriptorSet 0
               OpDecorate %samp Binding 0

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
       %samp = OpVariable %_ptr_UniformConstant_11 UniformConstant
    %v2float = OpTypeVector %float 2
  %float_0_5 = OpConstant %float 0.5
         %17 = OpConstantComposite %v2float %float_0_5 %float_0_5
              ; set up composite to be validated
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
   %int_n100 = OpConstant %int -100
  %uint_n100 = OpConstant %uint 4294967196
    %int_100 = OpConstant %int 100
     %uint_0 = OpConstant %uint 0
      %int_0 = OpConstant %int 0
 %offset_100 = OpConstantComposite %v2int %int_n100 %int_100
%offset_n100 = OpConstantComposite %v2uint %uint_0 %uint_n100

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
      %color = OpVariable %_ptr_Function_v4float Function
         %14 = OpLoad %11 %samp
               ; Should trigger min and max
         %24 = OpImageGather %v4float %14 %17 %int_0 ConstOffset %offset_100
               ; Should only trigger max since uint
         %25 = OpImageGather %v4float %14 %17 %int_0 ConstOffset %offset_n100
               OpStore %color %24
               OpReturn
               OpFunctionEnd
        )";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpImage-06376");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpImage-06377");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpImage-06377");
    auto cs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderLimits, MinAndMaxTexelOffset) {
    TEST_DESCRIPTION("Test shader with offset less than minTexelOffset and greather than maxTexelOffset");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (m_device->Physical().limits_.minTexelOffset <= -100 || m_device->Physical().limits_.maxTexelOffset >= 100) {
        GTEST_SKIP() << "test needs minTexelGatherOffset greater than -100 and maxTexelGatherOffset less than 100";
    }

    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpDecorate %textureSampler DescriptorSet 0
               OpDecorate %textureSampler Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
%textureSampler = OpVariable %_ptr_UniformConstant_11 UniformConstant
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %17 = OpConstantComposite %v2float %float_0 %float_0
              ; set up composite to be validated
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
     %uint_0 = OpConstant %uint 0
      %int_0 = OpConstant %int 0
   %int_n100 = OpConstant %int -100
  %uint_n100 = OpConstant %uint 4294967196
    %int_100 = OpConstant %int 100
 %offset_100 = OpConstantComposite %v2int %int_n100 %int_100
%offset_n100 = OpConstantComposite %v2uint %uint_0 %uint_n100
         %24 = OpConstantComposite %v2int %int_0 %int_0

       %main = OpFunction %void None %3
      %label = OpLabel
         %14 = OpLoad %11 %textureSampler
         %26 = OpImage %10 %14
               ; Should trigger min and max
    %result0 = OpImageSampleImplicitLod %v4float %14 %17 ConstOffset %offset_100
    %result1 = OpImageFetch %v4float %26 %24 ConstOffset %offset_100
               ; Should only trigger max since uint
    %result2 = OpImageSampleImplicitLod %v4float %14 %17 ConstOffset %offset_n100
    %result3 = OpImageFetch %v4float %26 %24 ConstOffset %offset_n100
               OpReturn
               OpFunctionEnd
        )";

    // OpImageSampleImplicitLod
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpImageSample-06435");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpImageSample-06436", 2);
    // OpImageFetch
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpImageSample-06435");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpImageSample-06436", 2);
    VkShaderObj const fs(this, spv_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderLimits, MaxFragmentDualSrcAttachments) {
    TEST_DESCRIPTION("Test drawing with dual source blending with too many fragment output attachments.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::dualSrcBlend);
    AddRequiredFeature(vkt::Feature::independentBlend);
    RETURN_IF_SKIP(Init());

    const uint32_t count = m_device->Physical().limits_.maxFragmentDualSrcAttachments + 1;
    if (count != 2) {
        GTEST_SKIP() << "Test is designed for a maxFragmentDualSrcAttachments of 1";
    }
    InitRenderTarget(count);

    const char *fs_src = R"glsl(
        #version 460
        layout(location = 0) out vec4 c0;
        layout(location = 1) out vec4 c1;
        void main() {
            c0 = vec4(0.0f);
            c1 = vec4(0.0f);
        }
    )glsl";
    VkShaderObj fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineColorBlendAttachmentState color_blend[2] = {};
    color_blend[0] = DefaultColorBlendAttachmentState();
    color_blend[1] = DefaultColorBlendAttachmentState();
    color_blend[1].srcColorBlendFactor = VK_BLEND_FACTOR_SRC1_COLOR;  // bad!

    CreatePipelineHelper pipe(*this);
    pipe.cb_ci_.attachmentCount = 2;
    pipe.cb_ci_.pAttachments = color_blend;
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-maxFragmentDualSrcAttachments-09239");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeShaderLimits, OffsetMaxComputeSharedMemorySize) {
    TEST_DESCRIPTION("Have an offset that is over maxComputeSharedMemorySize");

    // need at least SPIR-V 1.4 for SPV_KHR_workgroup_memory_explicit_layout
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::workgroupMemoryExplicitLayout);
    RETURN_IF_SKIP(Init());

    const uint32_t max_shared_memory_size = m_device->Physical().limits_.maxComputeSharedMemorySize;

    // layout(constant_id = 0) const uint value = 4;
    // shared X {
    //     vec4 x1[value];
    //     layout(offset = OVER_LIMIT) vec4 x2;
    // };
    std::stringstream csSource;
    csSource << R"asm(
               OpCapability Shader
               OpCapability WorkgroupMemoryExplicitLayoutKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %value SpecId 0
               OpDecorate %_arr_v4float_value ArrayStride 16
               OpMemberDecorate %X 0 Offset 0
               OpMemberDecorate %X 1 Offset )asm";
    // will be over the max if the spec constant uses default value
    csSource << (max_shared_memory_size + 16);
    csSource << R"asm(
               OpDecorate %X Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %value = OpSpecConstant %uint 1
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_arr_v4float_value = OpTypeArray %v4float %value
          %X = OpTypeStruct %_arr_v4float_value %v4float
%_ptr_Workgroup_X = OpTypePointer Workgroup %X
          %_ = OpVariable %_ptr_Workgroup_X Workgroup
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )asm";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2,
                                             SPV_SOURCE_ASM);

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Workgroup-06530");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderLimits, MaxFragmentOutputAttachments) {
    RETURN_IF_SKIP(Init());
    if (m_device->Physical().limits_.maxFragmentOutputAttachments != 4) {
        GTEST_SKIP() << "maxFragmentOutputAttachments is not 4";
    }

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 c0;
        layout(location=1) out vec4 c1;
        layout(location=2) out vec4 c2;
        layout(location=3) out vec4 c3;
        layout(location=4) out vec4 c4;
        void main(){
           c0 = vec4(1.0);
           c1 = vec4(1.0);
           c2 = vec4(1.0);
           c3 = vec4(1.0);
           c4 = vec4(1.0);
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderLimits, MaxFragmentOutputAttachmentsArray) {
    RETURN_IF_SKIP(Init());
    if (m_device->Physical().limits_.maxFragmentOutputAttachments != 4) {
        GTEST_SKIP() << "maxFragmentOutputAttachments is not 4";
    }

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 c[5];
        void main(){
           c[4] = vec4(1.0);
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderLimits, MaxFragmentOutputAttachmentsArrayAtEnd) {
    RETURN_IF_SKIP(Init());
    if (m_device->Physical().limits_.maxFragmentOutputAttachments != 4) {
        GTEST_SKIP() << "maxFragmentOutputAttachments is not 4";
    }

    char const *fsSource = R"glsl(
        #version 450
        layout(location=3) out vec4 c[2];
        void main(){
           c[1] = vec4(1.0);
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderLimits, MaxFragmentCombinedOutputResources) {
    RETURN_IF_SKIP(InitFramework());
    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    props.limits.maxFragmentCombinedOutputResources = 4;
    fpvkSetPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    RETURN_IF_SKIP(InitState());

    char const *fsSource = R"glsl(
        #version 450
        layout(set = 0, binding=0) buffer SSBO_0 {
            uint a;
        };
        layout(set = 0, binding=3) buffer SSBO_1 {
            uint b;
        };
        layout(set = 0, binding = 4, r32f) uniform imageBuffer s_buffer;

        layout(location=1) out vec4 color_0;
        layout(location=3) out vec4 color_1;

        void main(){
           color_0 = vec4(1.0);
           color_1 = vec4(1.0);
           a = b;
           imageStore(s_buffer, 0, vec4(1.0));
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06428");
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    m_errorMonitor->VerifyFound();
}
