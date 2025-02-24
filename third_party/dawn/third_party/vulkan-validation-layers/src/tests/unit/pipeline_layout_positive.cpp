/*
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
 * Modifications Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
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

class PositivePipelineLayout : public VkLayerTest {};

TEST_F(PositivePipelineLayout, DescriptorTypeMismatchNonCombinedImageSampler) {
    TEST_DESCRIPTION("HLSL will sometimes produce a SAMPLED_IMAGE / SAMPLER on the same slot that is same as COMBINED_IMAGE_SAMPLER");

    RETURN_IF_SKIP(Init());
    InitRenderTarget(0, nullptr);

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    char const *fsSource = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 610
               OpDecorate %textureColor DescriptorSet 0
               OpDecorate %textureColor Binding 1
               OpDecorate %samplerColor DescriptorSet 0
               OpDecorate %samplerColor Binding 1
      %float = OpTypeFloat 32
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%ptr_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampler = OpTypeSampler
%ptr_type_sampler = OpTypePointer UniformConstant %type_sampler
    %v2float = OpTypeVector %float 2
%ptr_v2float = OpTypePointer Input %v2float
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
%type_sampled_image = OpTypeSampledImage %type_2d_image
%textureColor = OpVariable %ptr_type_2d_image UniformConstant
%samplerColor = OpVariable %ptr_type_sampler UniformConstant
       %main = OpFunction %void None %16
         %17 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});

    pipe.CreateGraphicsPipeline();
}
