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
#include "../framework/descriptor_helper.h"

class NegativeShaderStorageTexel : public VkLayerTest {};

TEST_F(NegativeShaderStorageTexel, WriteLessComponent) {
    TEST_DESCRIPTION("Test writing to texel buffer with less components.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    // not valid GLSL, but would look like:
    // layout(set = 0, binding = 0, Rgba8ui) uniform uimageBuffer storageTexelBuffer;
    // imageStore(storageTexelBuffer, 1, uvec3(1, 1, 1));
    //
    // Rgba8ui == 4-component but only writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
               OpCapability ImageBuffer
               OpCapability StorageImageExtendedFormats
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %image = OpTypeImage %uint Buffer 0 0 0 2 Rgba8ui
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
     %v3uint = OpTypeVector %uint 3
      %int_1 = OpConstant %int 1
     %uint_1 = OpConstant %uint 1
    %texelU3 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %int_1 %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";

    const VkFormat format = VK_FORMAT_R8G8B8A8_UINT;  // Rgba8ui
    if (!BufferFormatAndFeaturesSupported(Gpu(), format, VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage texel buffer";
    }

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
        helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpImageWrite-07112");
}

TEST_F(NegativeShaderStorageTexel, UnknownWriteLessComponent) {
    TEST_DESCRIPTION("Test writing to texel buffer unknown format with less components.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // not valid GLSL, but would look like:
    // layout(set = 0, binding = 0, Unknown) uniform uimageBuffer storageTexelBuffer;
    // imageStore(storageTexelBuffer, 1, uvec3(1, 1, 1));
    //
    // Unknown will become a 4-component but writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
               OpCapability ImageBuffer
               OpCapability StorageImageWriteWithoutFormat
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
               OpDecorate %var NonReadable
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %image = OpTypeImage %uint Buffer 0 0 0 2 Unknown
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
     %v3uint = OpTypeVector %uint 3
     %v4uint = OpTypeVector %uint 4
      %int_1 = OpConstant %int 1
     %uint_1 = OpConstant %uint 1
    %texelU3 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
    %texelU4 = OpConstantComposite %v4uint %uint_1 %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %int_1 %texelU4 ZeroExtend ;; Valid
               OpImageWrite %load %int_1 %texelU3 ZeroExtend ;; Invalid
               OpImageWrite %load %int_1 %texelU4 ZeroExtend ;; Valid
               OpImageWrite %load %int_1 %texelU3 ZeroExtend ;; Invalid
               OpReturn
               OpFunctionEnd
        )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    const VkFormat format = VK_FORMAT_R8G8B8A8_UINT;  // Rgba8ui
    if (!BufferFormatAndFeaturesSupported(Gpu(), format, VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage texel buffer";
    }

    VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
    VkFormatProperties2 fmt_props = vku::InitStructHelper(&fmt_props_3);
    vk::GetPhysicalDeviceFormatProperties2(Gpu(), VK_FORMAT_R8G8B8A8_UINT, &fmt_props);
    if ((fmt_props_3.bufferFeatures & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT) == 0) {
        GTEST_SKIP() << "Format doesn't support storage write without format";
    }

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);

    VkBufferViewCreateInfo buff_view_ci = vku::InitStructHelper();
    buff_view_ci.buffer = buffer.handle();
    buff_view_ci.format = format;
    buff_view_ci.range = VK_WHOLE_SIZE;
    vkt::BufferView buffer_view(*m_device, buff_view_ci);

    ds.WriteDescriptorBufferView(0, buffer_view);
    ds.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds.layout_});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &ds.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-OpImageWrite-04469");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeShaderStorageTexel, MissingFormatWriteForFormat) {
    TEST_DESCRIPTION("Create a shader writing a storage texel buffer without an image format");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    PFN_vkSetPhysicalDeviceFormatProperties2EXT fpvkSetPhysicalDeviceFormatProperties2EXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatProperties2EXT fpvkGetOriginalPhysicalDeviceFormatProperties2EXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatProperties2EXT, fpvkGetOriginalPhysicalDeviceFormatProperties2EXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormatProperties3 fmt_props_3 = vku::InitStructHelper();
    VkFormatProperties2 fmt_props = vku::InitStructHelper(&fmt_props_3);

    // set so format can be used as a storage texel buffer, but no WITHOUT_FORMAT support
    fpvkGetOriginalPhysicalDeviceFormatProperties2EXT(Gpu(), format, &fmt_props);
    fmt_props.formatProperties.bufferFeatures |= VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT;
    fmt_props.formatProperties.bufferFeatures &= ~VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT;
    fmt_props_3.bufferFeatures |= VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT;
    fmt_props_3.bufferFeatures &= ~VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT;
    fpvkSetPhysicalDeviceFormatProperties2EXT(Gpu(), format, fmt_props);

    const char *csSource = R"(
                  OpCapability Shader
                  OpCapability ImageBuffer
                  OpCapability StorageImageWriteWithoutFormat
             %1 = OpExtInstImport "GLSL.std.450"
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint GLCompute %main "main"
                  OpExecutionMode %main LocalSize 1 1 1
                  OpSource GLSL 450
                  OpDecorate %img DescriptorSet 0
                  OpDecorate %img Binding 0
                  OpDecorate %img NonWritable
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
             %7 = OpTypeImage %float Buffer 0 0 0 2 Unknown
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
           %img = OpVariable %_ptr_UniformConstant_7 UniformConstant
           %int = OpTypeInt 32 1
         %v2int = OpTypeVector %int 2
         %int_0 = OpConstant %int 0
            %14 = OpConstantComposite %v2int %int_0 %int_0
       %v4float = OpTypeVector %float 4
       %float_0 = OpConstant %float 0
            %17 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
          %uint = OpTypeInt 32 0
        %v3uint = OpTypeVector %uint 3
        %uint_1 = OpConstant %uint 1
          %main = OpFunction %void None %3
             %5 = OpLabel
            %10 = OpLoad %7 %img
                  OpImageWrite %10 %14 %17
                  OpReturn
                  OpFunctionEnd
                  )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.cs_ =
        std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    cs_pipeline.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds.layout_});
    cs_pipeline.LateBindPipelineInfo();
    cs_pipeline.cp_ci_.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;  // override with wrong value
    cs_pipeline.CreateComputePipeline(false);                      // need false to prevent late binding

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);

    VkBufferViewCreateInfo buff_view_ci = vku::InitStructHelper();
    buff_view_ci.buffer = buffer.handle();
    buff_view_ci.format = format;
    buff_view_ci.range = VK_WHOLE_SIZE;
    vkt::BufferView buffer_view(*m_device, buff_view_ci);
    if (!buffer_view.initialized()) {
        // device profile layer might hide fact this is not a supported buffer view format
        GTEST_SKIP() << "Device will not be able to initialize buffer view skipped";
    }

    ds.WriteDescriptorBufferView(0, buffer_view);
    ds.UpdateDescriptorSets();

    m_command_buffer.Reset();
    m_command_buffer.Begin();

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.pipeline_layout_.handle(), 0,
                              1, &ds.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-OpTypeImage-07029");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}
