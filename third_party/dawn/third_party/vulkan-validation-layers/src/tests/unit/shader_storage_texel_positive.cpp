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

class PositiveShaderStorageTexel : public VkLayerTest {};

TEST_F(PositiveShaderStorageTexel, BufferWriteMoreComponent) {
    TEST_DESCRIPTION("Test writing to image with less components.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderStorageImageExtendedFormats);
    RETURN_IF_SKIP(Init());

    // not valid GLSL, but would look like:
    // layout(set = 0, binding = 0, Rg32ui) uniform uimageBuffer storageTexelBuffer;
    // imageStore(storageTexelBuffer, 1, uvec3(1, 1, 1));
    //
    // Rg32ui == 2-component but writing 3 texels to it
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
      %image = OpTypeImage %uint Buffer 0 0 0 2 Rg32ui
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

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    const VkFormat format = VK_FORMAT_R32G32_UINT;  // Rg32ui
    if (!BufferFormatAndFeaturesSupported(Gpu(), format, VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage texel buffer";
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
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
}

