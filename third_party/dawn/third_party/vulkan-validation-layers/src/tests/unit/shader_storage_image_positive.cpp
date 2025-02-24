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
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

class PositiveShaderStorageImage : public VkLayerTest {};

TEST_F(PositiveShaderStorageImage, WriteMoreComponent) {
    TEST_DESCRIPTION("Test writing to image with less components.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderStorageImageExtendedFormats);
    RETURN_IF_SKIP(Init());

    // not valid GLSL, but would look like:
    // layout(set = 0, binding = 0, Rg32ui) uniform uimage2D storageImage;
    // imageStore(storageImage, ivec2(1, 1), uvec3(1, 1, 1));
    //
    // Rg32ui == 2-component but writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
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
      %image = OpTypeImage %uint 2D 0 0 0 2 Rg32ui
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
      %coord = OpConstantComposite %v2int %int_1 %int_1
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
    %texelU3 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %coord %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    const VkFormat format = VK_FORMAT_R32G32_UINT;  // Rg32ui
    if (!FormatFeaturesAreSupported(Gpu(), format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage image";
    }

    vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_STORAGE_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();

    ds.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    ds.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds.layout_});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &ds.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
}

TEST_F(PositiveShaderStorageImage, UnknownWriteMoreComponent) {
    TEST_DESCRIPTION("Test writing to image with less components for Unknown for OpTypeImage.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderStorageImageExtendedFormats);
    AddRequiredFeature(vkt::Feature::shaderStorageImageWriteWithoutFormat);
    RETURN_IF_SKIP(Init());

    // not valid GLSL, but would look like:
    // layout(set = 0, binding = 0, Unknown) readonly uniform uimage2D storageImage;
    // imageStore(storageImage, ivec2(1, 1), uvec3(1, 1, 1));
    //
    // Unknown will become a 2-component but writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
               OpCapability StorageImageExtendedFormats
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
      %image = OpTypeImage %uint 2D 0 0 0 2 Unknown
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
      %coord = OpConstantComposite %v2int %int_1 %int_1
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
    %texelU3 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %coord %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    const VkFormat format = VK_FORMAT_R32G32_UINT;
    if (!FormatFeaturesAreSupported(Gpu(), format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage image";
    }

    VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
    VkFormatProperties2 fmt_props = vku::InitStructHelper(&fmt_props_3);
    vk::GetPhysicalDeviceFormatProperties2(Gpu(), format, &fmt_props);
    if ((fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT) == 0) {
        GTEST_SKIP() << "Format doesn't support storage write without format";
    }

    vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_STORAGE_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();

    ds.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    ds.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds.layout_});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &ds.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
}

TEST_F(PositiveShaderStorageImage, WriteSpecConstantMoreComponent) {
    TEST_DESCRIPTION("Test writing to image with less components with Texel being a spec constant.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderStorageImageExtendedFormats);
    RETURN_IF_SKIP(Init());

    // not valid GLSL, but would look like:
    // layout (constant_id = 0) const uint sc = 1;
    // layout(set = 0, binding = 0, Rg32ui) uniform uimage2D storageImage;
    // imageStore(storageImage, ivec2(1, 1), uvec3(1, sc, sc + 1));
    //
    // Rg32ui == 2-component but writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
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
      %image = OpTypeImage %uint 2D 0 0 0 2 Rg32ui
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
      %coord = OpConstantComposite %v2int %int_1 %int_1
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
         %sc = OpSpecConstant %uint 1
      %sc_p1 = OpSpecConstantOp %uint IAdd %sc %uint_1
    %texelU3 = OpSpecConstantComposite %v3uint %uint_1 %sc %sc_p1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %coord %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    const VkFormat format = VK_FORMAT_R32G32_UINT;  // Rg32ui
    if (!FormatFeaturesAreSupported(Gpu(), format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage image";
    }

    vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_STORAGE_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();

    ds.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    ds.UpdateDescriptorSets();

    uint32_t data = 2;
    VkSpecializationMapEntry entry;
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);
    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM,
                                             &specialization_info);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds.layout_});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &ds.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
}

TEST_F(PositiveShaderStorageImage, UnknownWriteLessComponentMultiEntrypoint) {
    TEST_DESCRIPTION("Test writing to image unknown format with less components, but in unused Entrypoint.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderStorageImageWriteWithoutFormat);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // The vertex and fragment shader are just a passthrough
    // The compute shader has the invalid OpImageWrite
    const char *source = R"(
               OpCapability Shader
               OpCapability StorageImageWriteWithoutFormat
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main_f "main" %4
               OpEntryPoint Vertex %main_v "main" %2
               OpEntryPoint GLCompute %main_c "main" %var_image
               OpExecutionMode %main_f OriginUpperLeft
               OpExecutionMode %main_c LocalSize 1 1 1
               OpMemberDecorate %builtin_vert 0 BuiltIn Position
               OpMemberDecorate %builtin_vert 1 BuiltIn PointSize
               OpMemberDecorate %builtin_vert 2 BuiltIn ClipDistance
               OpMemberDecorate %builtin_vert 3 BuiltIn CullDistance
               OpDecorate %builtin_vert Block

               OpDecorate %4 Location 0

               OpDecorate %var_image DescriptorSet 0
               OpDecorate %var_image Binding 0
               OpDecorate %var_image NonReadable

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

            ; Fragment types
%ptr_output_frag = OpTypePointer Output %v4float
          %4 = OpVariable %ptr_output_frag Output
    %float_0 = OpConstant %float 0
         %23 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0

            ; Compute types
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
      %coord = OpConstantComposite %v2int %int_1 %int_1
     %v3uint = OpTypeVector %uint 3
    %texelU3 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
      %image = OpTypeImage %uint 2D 0 0 0 2 Unknown
  %ptr_image = OpTypePointer UniformConstant %image
  %var_image = OpVariable %ptr_image UniformConstant

     %main_v = OpFunction %void None %8
         %24 = OpLabel
               OpReturn
               OpFunctionEnd

     %main_f = OpFunction %void None %8
         %28 = OpLabel
               OpStore %4 %23
               OpReturn
               OpFunctionEnd

     %main_c = OpFunction %void None %8
         %29 = OpLabel
 %load_image = OpLoad %image %var_image
               OpImageWrite %load_image %coord %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
    )";
    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    const VkFormat format = VK_FORMAT_R8G8B8A8_UINT;
    if (!FormatFeaturesAreSupported(Gpu(), format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage image";
    }

    vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_STORAGE_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();

    ds.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    ds.UpdateDescriptorSets();

    VkShaderObj const vs(this, source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    VkShaderObj const fs(this, source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds.layout_});
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &ds.set_, 0, nullptr);
    // This does not invoke the Compute Entrypoint where the bad write would be
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}
