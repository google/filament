/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class PositiveShaderSpirv : public VkLayerTest {};

TEST_F(PositiveShaderSpirv, NonSemanticInfo) {
    // This is a positive test, no errors expected
    // Verifies the ability to use non-semantic extended instruction sets when the extension is enabled
    TEST_DESCRIPTION("Create a shader that uses SPV_KHR_non_semantic_info.");
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // compute shader using a non-semantic extended instruction set.

    const char *spv_source = R"(
                   OpCapability Shader
                   OpExtension "SPV_KHR_non_semantic_info"
   %non_semantic = OpExtInstImport "NonSemantic.Validation.Test"
                   OpMemoryModel Logical GLSL450
                   OpEntryPoint GLCompute %main "main"
                   OpExecutionMode %main LocalSize 1 1 1
           %void = OpTypeVoid
              %1 = OpExtInst %void %non_semantic 55 %void
           %func = OpTypeFunction %void
           %main = OpFunction %void None %func
              %2 = OpLabel
                   OpReturn
                   OpFunctionEnd
        )";

    VkShaderObj cs(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
}

TEST_F(PositiveShaderSpirv, GroupDecorations) {
    TEST_DESCRIPTION("Test shader validation support for group decorations.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const std::string spv_source = R"(
              OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %gl_GlobalInvocationID
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpName %main "main"
               OpName %gl_GlobalInvocationID "gl_GlobalInvocationID"
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
               OpDecorate %_runtimearr_float ArrayStride 4
               OpDecorate %4 BufferBlock
               OpDecorate %5 Offset 0
          %4 = OpDecorationGroup
          %5 = OpDecorationGroup
               OpGroupDecorate %4 %_struct_6 %_struct_7 %_struct_8 %_struct_9 %_struct_10 %_struct_11
               OpGroupMemberDecorate %5 %_struct_6 0 %_struct_7 0 %_struct_8 0 %_struct_9 0 %_struct_10 0 %_struct_11 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %13 DescriptorSet 0
               OpDecorate %13 NonWritable
               OpDecorate %13 Restrict
         %14 = OpDecorationGroup
         %12 = OpDecorationGroup
         %13 = OpDecorationGroup
               OpGroupDecorate %12 %15
               OpGroupDecorate %12 %15
               OpGroupDecorate %12 %15
               OpDecorate %15 DescriptorSet 0
               OpDecorate %15 Binding 5
               OpGroupDecorate %14 %16
               OpDecorate %16 DescriptorSet 0
               OpDecorate %16 Binding 0
               OpGroupDecorate %12 %17
               OpDecorate %17 Binding 1
               OpGroupDecorate %13 %18 %19
               OpDecorate %18 Binding 2
               OpDecorate %19 Binding 3
               OpGroupDecorate %14 %20
               OpGroupDecorate %12 %20
               OpGroupDecorate %13 %20
               OpDecorate %20 Binding 4
       %bool = OpTypeBool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
     %v3uint = OpTypeVector %uint 3
    %v3float = OpTypeVector %float 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%_ptr_Uniform_int = OpTypePointer Uniform %int
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_runtimearr_int = OpTypeRuntimeArray %int
%_runtimearr_float = OpTypeRuntimeArray %float
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
      %int_0 = OpConstant %int 0
  %_struct_6 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_6 = OpTypePointer Uniform %_struct_6
         %15 = OpVariable %_ptr_Uniform__struct_6 Uniform
  %_struct_7 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_7 = OpTypePointer Uniform %_struct_7
         %16 = OpVariable %_ptr_Uniform__struct_7 Uniform
  %_struct_8 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_8 = OpTypePointer Uniform %_struct_8
         %17 = OpVariable %_ptr_Uniform__struct_8 Uniform
  %_struct_9 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_9 = OpTypePointer Uniform %_struct_9
         %18 = OpVariable %_ptr_Uniform__struct_9 Uniform
 %_struct_10 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_10 = OpTypePointer Uniform %_struct_10
         %19 = OpVariable %_ptr_Uniform__struct_10 Uniform
 %_struct_11 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_11 = OpTypePointer Uniform %_struct_11
         %20 = OpVariable %_ptr_Uniform__struct_11 Uniform
       %main = OpFunction %void None %23
         %40 = OpLabel
         %41 = OpLoad %v3uint %gl_GlobalInvocationID
         %42 = OpCompositeExtract %uint %41 0
         %43 = OpAccessChain %_ptr_Uniform_float %16 %int_0 %42
         %44 = OpAccessChain %_ptr_Uniform_float %17 %int_0 %42
         %45 = OpAccessChain %_ptr_Uniform_float %18 %int_0 %42
         %46 = OpAccessChain %_ptr_Uniform_float %19 %int_0 %42
         %47 = OpAccessChain %_ptr_Uniform_float %20 %int_0 %42
         %48 = OpAccessChain %_ptr_Uniform_float %15 %int_0 %42
         %49 = OpLoad %float %43
         %50 = OpLoad %float %44
         %51 = OpLoad %float %45
         %52 = OpLoad %float %46
         %53 = OpLoad %float %47
         %54 = OpFAdd %float %49 %50
         %55 = OpFAdd %float %54 %51
         %56 = OpFAdd %float %55 %52
         %57 = OpFAdd %float %56 %53
               OpStore %48 %57
               OpReturn
               OpFunctionEnd
)";

    // CreateDescriptorSetLayout
    VkDescriptorSetLayoutBinding dslb[6] = {};
    size_t dslb_size = std::size(dslb);
    for (size_t i = 0; i < dslb_size; i++) {
        dslb[i].binding = i;
        dslb[i].descriptorCount = 1;
        dslb[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        dslb[i].pImmutableSamplers = NULL;
        dslb[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_ALL;
    }
    if (m_device->Physical().limits_.maxPerStageDescriptorStorageBuffers < dslb_size) {
        GTEST_SKIP() << "Needed storage buffer bindings (" << dslb_size << ") exceeds this devices limit of "
                     << m_device->Physical().limits_.maxPerStageDescriptorStorageBuffers;
    }

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_.resize(dslb_size);
    memcpy(pipe.dsl_bindings_.data(), dslb, dslb_size * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_ =
        std::make_unique<VkShaderObj>(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderSpirv, CapabilityExtension1of2) {
    // This is a positive test, no errors expected
    // Verifies the ability to deal with a shader that declares a non-unique SPIRV capability ID
    TEST_DESCRIPTION("Create a shader in which uses a non-unique capability ID extension, 1 of 2");

    AddRequiredExtensions(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiViewport);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Vertex shader using viewport array capability
    char const *vsSource = R"glsl(
        #version 450
        #extension GL_ARB_shader_viewport_layer_array : enable
        void main() {
            gl_ViewportIndex = 1;
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderSpirv, CapabilityExtension2of2) {
    // This is a positive test, no errors expected
    // Verifies the ability to deal with a shader that declares a non-unique SPIRV capability ID
    TEST_DESCRIPTION("Create a shader in which uses a non-unique capability ID extension, 2 of 2");

    AddRequiredExtensions(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiViewport);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Vertex shader using viewport array capability
    char const *vsSource = R"glsl(
        #version 450
        #extension GL_ARB_shader_viewport_layer_array : enable
        void main() {
            gl_ViewportIndex = 1;
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderSpirv, ShaderDrawParametersWithoutFeature) {
    TEST_DESCRIPTION("Use VK_KHR_shader_draw_parameters in 1.0 before shaderDrawParameters feature was added");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredExtensions(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (DeviceValidationVersion() != VK_API_VERSION_1_0) {
        GTEST_SKIP() << "requires Vulkan 1.0 exactly";
    }

    char const *vsSource = R"glsl(
        #version 460
        void main(){
           gl_Position = vec4(float(gl_BaseVertex));
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

    if (VK_SUCCESS == vs.InitFromGLSLTry()) {
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }
}

TEST_F(PositiveShaderSpirv, ShaderDrawParametersWithoutFeature11) {
    TEST_DESCRIPTION("Use VK_KHR_shader_draw_parameters in 1.1 using the extension");

    // We need to explicitly allow promoted extensions to be enabled as this test relies on this behavior
    AllowPromotedExtensions();

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 460
        void main(){
           gl_Position = vec4(float(gl_BaseVertex));
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

    // make sure using SPIR-V 1.3 as extension is core and not needed in Vulkan then
    if (VK_SUCCESS == vs.InitFromGLSLTry()) {
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }
}

TEST_F(PositiveShaderSpirv, ShaderDrawParametersWithFeature) {
    TEST_DESCRIPTION("Use VK_KHR_shader_draw_parameters in 1.2 with feature bit enabled");
    // use 1.2 to get the feature bit in VkPhysicalDeviceVulkan11Features
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceVulkan11Features features11 = vku::InitStructHelper();
    features11.shaderDrawParameters = VK_TRUE;
    auto features2 = GetPhysicalDeviceFeatures2(features11);
    GetPhysicalDeviceFeatures2(features2);
    if (features11.shaderDrawParameters == VK_FALSE) {
        GTEST_SKIP() << "shaderDrawParameters not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 460
        void main(){
           gl_Position = vec4(float(gl_BaseVertex));
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

    // make sure using SPIR-V 1.3 as extension is core and not needed in Vulkan then
    if (VK_SUCCESS == vs.InitFromGLSLTry()) {
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }
}

TEST_F(PositiveShaderSpirv, Std430SpirvOptFlags10) {
    TEST_DESCRIPTION("Reproduces issue 3442 where spirv-opt fails to set layout flags options using Vulkan 1.0");
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/3442

    AddRequiredExtensions(VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::uniformBufferStandardLayout);
    AddRequiredFeature(vkt::Feature::scalarBlockLayout);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    const char *fragment_source = R"glsl(
#version 450
#extension GL_ARB_separate_shader_objects:enable
#extension GL_EXT_samplerless_texture_functions:require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require

layout(std430, set=0,binding=0)uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
    vec4 lightPositions[1];
    int SliceCutoffs[6];
}ubo;

// this specialization constant triggers the validation layer to recompile the shader
// which causes the error related to the above uniform
layout(constant_id = 0) const float spec = 10.0f;

layout(location=0) out vec4 frag_color;
void main() {
    frag_color = vec4(ubo.lightPositions[0]) * spec;
}
    )glsl";

    // Force a random value to replace the default to trigger shader val logic to replace it
    float data = 2.0f;
    VkSpecializationMapEntry entry = {0, 0, sizeof(float)};
    VkSpecializationInfo specialization_info = {1, &entry, sizeof(float), &data};
    const VkShaderObj fs(this, fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                         &specialization_info);

    CreatePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderSpirv, Std430SpirvOptFlags12) {
    TEST_DESCRIPTION("Reproduces issue 3442 where spirv-opt fails to set layout flags options using Vulkan 1.2");
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/3442

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::scalarBlockLayout);
    AddRequiredFeature(vkt::Feature::uniformBufferStandardLayout);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    const char *fragment_source = R"glsl(
#version 450
#extension GL_ARB_separate_shader_objects:enable
#extension GL_EXT_samplerless_texture_functions:require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require

layout(std430, set=0,binding=0)uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
    vec4 lightPositions[1];
    int SliceCutoffs[6];
}ubo;

// this specialization constant triggers the validation layer to recompile the shader
// which causes the error related to the above uniform
layout(constant_id = 0) const float spec = 10.0f;

layout(location=0) out vec4 frag_color;
void main() {
    frag_color = vec4(ubo.lightPositions[0]) * spec;
}
    )glsl";

    // Force a random value to replace the default to trigger shader val logic to replace it
    float data = 2.0f;
    VkSpecializationMapEntry entry = {0, 0, sizeof(float)};
    VkSpecializationInfo specialization_info = {1, &entry, sizeof(float), &data};
    const VkShaderObj fs(this, fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                         &specialization_info);

    CreatePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderSpirv, SpecializationWordBoundryOffset) {
    TEST_DESCRIPTION("Make sure a specialization constant entry can stide over a word boundry");

    // require to make enable logic simpler
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD, need real device to produce output to check";
    }

    // glslang currenlty turned the GLSL to
    //      %19 = OpSpecConstantOp %uint UConvert %a
    // which causes issue (to be fixed outside scope of this test)
    // but move the UConvert to inside the function as
    //      %19 = OpUConvert %uint %a
    //
    // #version 450
    // #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
    // layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
    // // All spec constants will write zero by default
    // layout (constant_id = 0) const uint8_t a = uint8_t(0);
    // layout (constant_id = 1) const uint b = 0;
    // layout (constant_id = 3) const uint c = 0;
    // layout (constant_id = 4) const uint d = 0;
    // layout (constant_id = 5) const uint8_t e = uint8_t(0);
    //
    // layout(set = 0, binding = 0) buffer ssbo {
    //     uint data[5];
    // };
    //
    // void main() {
    //     data[0] = 0; // clear full word
    //     data[0] = uint(a);
    //     data[1] = b;
    //     data[2] = c;
    //     data[3] = d;
    //     data[4] = 0; // clear full word
    //     data[4] = uint(e);
    // }
    std::string cs_src = R"(
               OpCapability Shader
               OpCapability Int8
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_shader_explicit_arithmetic_types_int8"
               OpDecorate %_arr_uint_uint_5 ArrayStride 4
               OpMemberDecorate %ssbo 0 Offset 0
               OpDecorate %ssbo BufferBlock
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
               OpDecorate %a SpecId 0
               OpDecorate %b SpecId 1
               OpDecorate %c SpecId 3
               OpDecorate %d SpecId 4
               OpDecorate %e SpecId 5
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_5 = OpConstant %uint 5
%_arr_uint_uint_5 = OpTypeArray %uint %uint_5
       %ssbo = OpTypeStruct %_arr_uint_uint_5
%_ptr_Uniform_ssbo = OpTypePointer Uniform %ssbo
          %_ = OpVariable %_ptr_Uniform_ssbo Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %uint_0 = OpConstant %uint 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
      %uchar = OpTypeInt 8 0
          %a = OpSpecConstant %uchar 0
      %int_1 = OpConstant %int 1
          %b = OpSpecConstant %uint 0
      %int_2 = OpConstant %int 2
          %c = OpSpecConstant %uint 0
      %int_3 = OpConstant %int 3
          %d = OpSpecConstant %uint 0
      %int_4 = OpConstant %int 4
          %e = OpSpecConstant %uchar 0
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpUConvert %uint %a
         %33 = OpUConvert %uint %e
         %16 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_0
               OpStore %16 %uint_0
         %20 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_0
               OpStore %20 %19
         %23 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_1
               OpStore %23 %b
         %26 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_2
               OpStore %26 %c
         %29 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_3
               OpStore %29 %d
         %31 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_4
               OpStore %31 %uint_0
         %34 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_4
               OpStore %34 %33
               OpReturn
               OpFunctionEnd
    )";

    // Use strange combinations of size and offsets around word boundry
    VkSpecializationMapEntry entries[5] = {
        {0, 1, 1},  // OpTypeInt 8
        {1, 1, 4},  // OpTypeInt 32
        {3, 2, 4},  // OpTypeInt 32
        {4, 3, 4},  // OpTypeInt 32
        {5, 3, 1},  // OpTypeInt 8
    };

    uint8_t data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    VkSpecializationInfo specialization_info = {
        5,
        entries,
        sizeof(uint8_t) * 8,
        reinterpret_cast<void *>(data),
    };

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_src.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM,
                                             &specialization_info);
    pipe.CreateComputePipeline();

    // Submit shader to see SSBO output
    VkBufferCreateInfo bci = vku::InitStructHelper();
    bci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bci.size = 1024;
    vkt::Buffer buffer(*m_device, bci, kHostVisibleMemProps);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer.handle(), 0, 1024, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // Make sure spec constants were updated correctly
    void *pData;
    ASSERT_EQ(VK_SUCCESS, vk::MapMemory(device(), buffer.Memory().handle(), 0, VK_WHOLE_SIZE, 0, &pData));
    uint32_t *ssbo_data = reinterpret_cast<uint32_t *>(pData);
    ASSERT_EQ(ssbo_data[0], 0x02);
    ASSERT_EQ(ssbo_data[1], 0x05040302);
    ASSERT_EQ(ssbo_data[2], 0x06050403);
    ASSERT_EQ(ssbo_data[3], 0x07060504);
    ASSERT_EQ(ssbo_data[4], 0x04);
    vk::UnmapMemory(device(), buffer.Memory().handle());
}

TEST_F(PositiveShaderSpirv, Spirv16Vulkan13) {
    TEST_DESCRIPTION("Create a shader using 1.3 spirv environment");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init());

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_3);
}

TEST_F(PositiveShaderSpirv, OpTypeArraySpecConstant) {
    TEST_DESCRIPTION("Make sure spec constants for a OpTypeArray doesn't assert");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %storageBuffer 0 Offset 0
               OpDecorate %storageBuffer BufferBlock
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
               OpDecorate %sc SpecId 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
%storageBuffer = OpTypeStruct %int
%_ptr_Uniform_storageBuffer = OpTypePointer Uniform %storageBuffer
          %_ = OpVariable %_ptr_Uniform_storageBuffer Uniform
      %int_0 = OpConstant %int 0
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
         %sc = OpSpecConstant %uint 10
%_arr_int_sc = OpTypeArray %int %sc
%_ptr_Workgroup__arr_int_sc = OpTypePointer Workgroup %_arr_int_sc
  %wg_normal = OpVariable %_ptr_Workgroup__arr_int_sc Workgroup
      %int_3 = OpConstant %int 3
%_ptr_Workgroup_int = OpTypePointer Workgroup %int
         %xx = OpSpecConstant %uint 1
         %yy = OpSpecConstant %uint 1
         %zz = OpSpecConstant %uint 1
%gl_WorkGroupSize = OpSpecConstantComposite %v3uint %xx %yy %zz
         %57 = OpSpecConstantOp %uint CompositeExtract %gl_WorkGroupSize 2
         %58 = OpSpecConstantOp %uint CompositeExtract %gl_WorkGroupSize 1
         %59 = OpSpecConstantOp %uint IMul %57 %58
         %60 = OpSpecConstantOp %uint CompositeExtract %gl_WorkGroupSize 0
         %61 = OpSpecConstantOp %uint IMul %59 %60
%_arr_int_21 = OpTypeArray %int %61
%_ptr_Workgroup__arr_int_21 = OpTypePointer Workgroup %_arr_int_21
      %wg_op = OpVariable %_ptr_Workgroup__arr_int_21 Workgroup
%_ptr_Function__arr_int_sc = OpTypePointer Function %_arr_int_sc
%_ptr_Function_int = OpTypePointer Function %int
         %34 = OpSpecConstantOp %uint IAdd %sc %uint_1
%_arr_int_34 = OpTypeArray %int %34
%_ptr_Function__arr_int_34 = OpTypePointer Function %_arr_int_34
%_ptr_Uniform_int = OpTypePointer Uniform %int
       %main = OpFunction %void None %3
          %5 = OpLabel
%func_normal = OpVariable %_ptr_Function__arr_int_sc Function
    %func_op = OpVariable %_ptr_Function__arr_int_34 Function
         %18 = OpAccessChain %_ptr_Workgroup_int %wg_normal %int_3
         %19 = OpLoad %int %18
         %25 = OpAccessChain %_ptr_Workgroup_int %wg_op %int_3
         %26 = OpLoad %int %25
         %27 = OpIAdd %int %19 %26
         %31 = OpAccessChain %_ptr_Function_int %func_normal %int_3
         %32 = OpLoad %int %31
         %33 = OpIAdd %int %27 %32
         %38 = OpAccessChain %_ptr_Function_int %func_op %int_3
         %39 = OpLoad %int %38
         %40 = OpIAdd %int %33 %39
         %42 = OpAccessChain %_ptr_Uniform_int %_ %int_0
               OpStore %42 %40
               OpReturn
               OpFunctionEnd
    )";

    uint32_t data = 5;

    VkSpecializationMapEntry entry;
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    // Use default value for spec constant
    const auto set_info_nospec = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                   SPV_SOURCE_ASM, nullptr);
        helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info_nospec, kErrorBit);

    // Use spec constant to update value
    const auto set_info_spec = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                   SPV_SOURCE_ASM, &specialization_info);
        helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info_spec, kErrorBit);
}

TEST_F(PositiveShaderSpirv, OpTypeStructRuntimeArray) {
    TEST_DESCRIPTION("Make sure variables with a OpTypeStruct can handle a runtime array inside");

    RETURN_IF_SKIP(Init());

    // %float = OpTypeFloat 32
    // %ra = OpTypeRuntimeArray %float
    // %struct = OpTypeStruct %ra
    char const *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer sb {
            float values[];
        };
        void main(){
            values[gl_LocalInvocationIndex] = gl_LocalInvocationIndex;
        }
    )glsl";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
        helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderSpirv, UnnormalizedCoordinatesNotSampled) {
    TEST_DESCRIPTION("If a samper is unnormalizedCoordinates, using COMBINED_IMAGE_SAMPLER, but texelFetch, don't throw error");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // This generates OpImage*Dref* instruction on R8G8B8A8_UNORM format.
    // Verify that it is allowed on this implementation if
    // VK_KHR_format_feature_flags2 is available.
    if (DeviceExtensionSupported(Gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
        VkFormatProperties2 fmt_props = vku::InitStructHelper(&fmt_props_3);

        vk::GetPhysicalDeviceFormatProperties2(Gpu(), VK_FORMAT_R8G8B8A8_UNORM, &fmt_props);

        if (!(fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT)) {
            GTEST_SKIP() << "R8G8B8A8_UNORM does not support OpImage*Dref* operations";
        }
    }

    VkShaderObj vs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    const char *fsSource = R"(
               OpCapability Shader
               OpCapability ImageBuffer
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
    %v4float = OpTypeVector %float 4
      %v3int = OpTypeVector %int 3
 %image_type = OpTypeImage %float 3D 0 0 0 1 Unknown
%sampled_image = OpTypeSampledImage %image_type
        %ptr = OpTypePointer UniformConstant %sampled_image
        %var = OpVariable %ptr UniformConstant
      %int_1 = OpConstant %int 1
      %cords = OpConstantComposite %v3int %int_1 %int_1 %int_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %sampled_image %var
      %image = OpImage %image_type %load
      %fetch = OpImageFetch %v4float %image %cords
               OpReturn
               OpFunctionEnd
        )";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    g_pipe.CreateGraphicsPipeline();

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, format, usage);
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    vkt::Image image_3d(*m_device, image_ci, vkt::set_layout);

    // If the sampler is unnormalizedCoordinates, the imageview type shouldn't be 3D, CUBE, 1D_ARRAY, 2D_ARRAY, CUBE_ARRAY.
    // This causes DesiredFailure.
    vkt::ImageView view = image_3d.CreateView(VK_IMAGE_VIEW_TYPE_3D);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.maxLod = 0;
    vkt::Sampler sampler(*m_device, sampler_ci);

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveShaderSpirv, GeometryShaderPassthroughNV) {
    TEST_DESCRIPTION("Test to validate VK_NV_geometry_shader_passthrough");

    AddRequiredExtensions(VK_NV_GEOMETRY_SHADER_PASSTHROUGH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char vs_src[] = R"glsl(
        #version 450

        out gl_PerVertex {
            vec4 gl_Position;
        };

        layout(location = 0) out ColorBlock {vec4 vertexColor;};

        const vec2 positions[3] = { vec2( 0.0f, -0.5f),
                                    vec2( 0.5f,  0.5f),
                                    vec2(-0.5f,  0.5f)
                                  };

        const vec4 colors[3] = { vec4(1.0f, 0.0f, 0.0f, 1.0f),
                                 vec4(0.0f, 1.0f, 0.0f, 1.0f),
                                 vec4(0.0f, 0.0f, 1.0f, 1.0f)
                               };
        void main()
        {
            vertexColor = colors[gl_VertexIndex % 3];
            gl_Position = vec4(positions[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";

    const char gs_src[] = R"glsl(
        #version 450
        #extension GL_NV_geometry_shader_passthrough: require

        layout(triangles) in;
        layout(triangle_strip, max_vertices = 3) out;

        layout(passthrough) in gl_PerVertex {vec4 gl_Position;};
        layout(location = 0, passthrough) in ColorBlock {vec4 vertexColor;};

        void main()
        {
           gl_Layer = 0;
        }
    )glsl";

    const char fs_src[] = R"glsl(
        #version 450

        layout(location = 0) in ColorBlock {vec4 vertexColor;};
        layout(location = 0) out vec4 outColor;

        void main() {
            outColor = vertexColor;
        }
    )glsl";

    VkShaderObj vs(this, vs_src, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gs_src, VK_SHADER_STAGE_GEOMETRY_BIT);
    VkShaderObj fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), fs.GetStageCreateInfo()};

    // Create pipeline and make sure that the usage of NV_geometry_shader_passthrough
    // in the fragment shader does not cause any errors.
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderSpirv, SpecializeInt8) {
    TEST_DESCRIPTION("Test int8 specialization.");

    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *fs_src = R"(
               OpCapability Shader
               OpCapability Int8
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %v "v"
               OpDecorate %v SpecId 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 8 1
          %v = OpSpecConstant %int 0
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj const fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    const VkSpecializationMapEntry entry = {
        0,               // id
        0,               // offset
        sizeof(uint8_t)  // size
    };
    uint8_t const data = 0x42;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(uint8_t),
        &data,
    };

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.shader_stages_[1].pSpecializationInfo = &specialization_info;

    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderSpirv, SpecializeInt16) {
    TEST_DESCRIPTION("Test int16 specialization.");

    AddRequiredFeature(vkt::Feature::shaderInt16);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *fs_src = R"(
               OpCapability Shader
               OpCapability Int16
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %v "v"
               OpDecorate %v SpecId 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 16 1
          %v = OpSpecConstant %int 0
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj const fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    const VkSpecializationMapEntry entry = {
        0,                // id
        0,                // offset
        sizeof(uint16_t)  // size
    };
    uint16_t const data = 0x4342;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(uint16_t),
        &data,
    };

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.shader_stages_[1].pSpecializationInfo = &specialization_info;

    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderSpirv, SpecializeInt32) {
    TEST_DESCRIPTION("Test int32 specialization.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *fs_src = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %v "v"
               OpDecorate %v SpecId 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
          %v = OpSpecConstant %int 0
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj const fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    const VkSpecializationMapEntry entry = {
        0,                // id
        0,                // offset
        sizeof(uint32_t)  // size
    };
    uint32_t const data = 0x45444342;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(uint32_t),
        &data,
    };

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.shader_stages_[1].pSpecializationInfo = &specialization_info;

    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderSpirv, SpecializeInt64) {
    TEST_DESCRIPTION("Test int64 specialization.");

    AddRequiredFeature(vkt::Feature::shaderInt64);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *fs_src = R"(
               OpCapability Shader
               OpCapability Int64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %v "v"
               OpDecorate %v SpecId 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 64 1
          %v = OpSpecConstant %int 0
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj const fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    const VkSpecializationMapEntry entry = {
        0,                // id
        0,                // offset
        sizeof(uint64_t)  // size
    };
    uint64_t const data = 0x4948474645444342;
    const VkSpecializationInfo specialization_info = {
        1,
        &entry,
        1 * sizeof(uint64_t),
        &data,
    };

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.shader_stages_[1].pSpecializationInfo = &specialization_info;

    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderSpirv, SpecializationUnused) {
    TEST_DESCRIPTION("Make sure an unused spec constant is valid to us");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // layout (constant_id = 2) const int a = 3;
    const char *cs_src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpDecorate %a SpecId 2
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
          %a = OpSpecConstant %int 3
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    VkSpecializationMapEntry entries[4] = {
        {0, 0, 1},  // unused
        {1, 0, 1},  // usued
        {2, 0, 4},  // OpTypeInt 32
        {3, 0, 4},  // usued
    };

    int32_t data = 0;
    VkSpecializationInfo specialization_info = {
        4,
        entries,
        1 * sizeof(decltype(data)),
        &data,
    };

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM,
                                                   &specialization_info);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

    // Even if the ID is never seen in VkSpecializationMapEntry the OpSpecConstant will use the default and still is valid
    specialization_info.mapEntryCount = 1;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

    // try another random unused value other than zero
    entries[0].constantID = 100;
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderSpirv, ShaderFloatControl) {
    TEST_DESCRIPTION("Test VK_KHR_float_controls");

    // Need 1.1 to get SPIR-V 1.3 since OpExecutionModeId was added in SPIR-V 1.2
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceFloatControlsProperties shader_float_control = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(shader_float_control);

    bool signed_zero_inf_nan_preserve = (shader_float_control.shaderSignedZeroInfNanPreserveFloat32 == VK_TRUE);
    bool denorm_preserve = (shader_float_control.shaderDenormPreserveFloat32 == VK_TRUE);
    bool denorm_flush_to_zero = (shader_float_control.shaderDenormFlushToZeroFloat32 == VK_TRUE);
    bool rounding_mode_rte = (shader_float_control.shaderRoundingModeRTEFloat32 == VK_TRUE);
    bool rounding_mode_rtz = (shader_float_control.shaderRoundingModeRTZFloat32 == VK_TRUE);

    // same body for each shader, only the start is different
    // this is just "float a = 1.0 + 2.0;" in SPIR-V
    const std::string source_body = R"(
             OpExecutionMode %main LocalSize 1 1 1
             OpSource GLSL 450
             OpName %main "main"
     %void = OpTypeVoid
        %3 = OpTypeFunction %void
    %float = OpTypeFloat 32
%pFunction = OpTypePointer Function %float
  %float_3 = OpConstant %float 3
     %main = OpFunction %void None %3
        %5 = OpLabel
        %6 = OpVariable %pFunction Function
             OpStore %6 %float_3
             OpReturn
             OpFunctionEnd
)";

    if (signed_zero_inf_nan_preserve) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability SignedZeroInfNanPreserve
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main SignedZeroInfNanPreserve 32
)" + source_body;

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                       SPV_SOURCE_ASM);
        };
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    if (denorm_preserve) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability DenormPreserve
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main DenormPreserve 32
)" + source_body;

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                       SPV_SOURCE_ASM);
        };
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    if (denorm_flush_to_zero) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability DenormFlushToZero
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main DenormFlushToZero 32
)" + source_body;

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                       SPV_SOURCE_ASM);
        };
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    if (rounding_mode_rte) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability RoundingModeRTE
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main RoundingModeRTE 32
)" + source_body;

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                       SPV_SOURCE_ASM);
        };
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    if (rounding_mode_rtz) {
        const std::string spv_source = R"(
            OpCapability Shader
            OpCapability RoundingModeRTZ
            OpExtension "SPV_KHR_float_controls"
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main RoundingModeRTZ 32
)" + source_body;

        const auto set_info = [&](CreateComputePipelineHelper &helper) {
            helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                       SPV_SOURCE_ASM);
        };
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }
}

TEST_F(PositiveShaderSpirv, Storage8and16bit) {
    TEST_DESCRIPTION("Test VK_KHR_8bit_storage and VK_KHR_16bit_storage");

    // use 1.1 for SPIR-V version, but keep extension as if 1.0
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitFramework());
    bool support_8_bit = IsExtensionsEnabled(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
    bool support_16_bit = IsExtensionsEnabled(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);

    if ((support_8_bit == false) && (support_16_bit == false)) {
        GTEST_SKIP() << "Extension not supported";
    }

    VkPhysicalDevice8BitStorageFeaturesKHR storage_8_bit_features = vku::InitStructHelper();
    VkPhysicalDevice16BitStorageFeaturesKHR storage_16_bit_features = vku::InitStructHelper(&storage_8_bit_features);
    VkPhysicalDeviceShaderFloat16Int8Features float_16_int_8_features = vku::InitStructHelper(&storage_16_bit_features);
    auto features2 = GetPhysicalDeviceFeatures2(float_16_int_8_features);
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    // 8 bit int test (not 8 bit float support in Vulkan)
    if ((support_8_bit == true) && (float_16_int_8_features.shaderInt8 == VK_TRUE)) {
        if (storage_8_bit_features.storageBuffer8BitAccess == VK_TRUE) {
            char const *vsSource = R"glsl(
                #version 450
                #extension GL_EXT_shader_8bit_storage: enable
                #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
                layout(set = 0, binding = 0) buffer SSBO { int8_t x; } data;
                void main(){
                   int8_t a = data.x + data.x;
                   gl_Position = vec4(float(a) * 0.0);
                }
            )glsl";
            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);

            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }

        if (storage_8_bit_features.uniformAndStorageBuffer8BitAccess == VK_TRUE) {
            char const *vsSource = R"glsl(
                #version 450
                #extension GL_EXT_shader_8bit_storage: enable
                #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
                layout(set = 0, binding = 0) uniform UBO { int8_t x; } data;
                void main(){
                   int8_t a = data.x + data.x;
                   gl_Position = vec4(float(a) * 0.0);
                }
            )glsl";
            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);

            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }

        if (storage_8_bit_features.storagePushConstant8 == VK_TRUE) {
            char const *vsSource = R"glsl(
                #version 450
                #extension GL_EXT_shader_8bit_storage: enable
                #extension GL_EXT_shader_explicit_arithmetic_types_int8: enable
                layout(push_constant) uniform PushConstant { int8_t x; } data;
                void main(){
                   int8_t a = data.x + data.x;
                   gl_Position = vec4(float(a) * 0.0);
                }
            )glsl";
            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);

            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }
    }

    // 16 bit float tests
    if ((support_16_bit == true) && (float_16_int_8_features.shaderFloat16 == VK_TRUE)) {
        if (storage_16_bit_features.storageBuffer16BitAccess == VK_TRUE) {
            char const *vsSource = R"glsl(
                #version 450
                #extension GL_EXT_shader_16bit_storage: enable
                #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
                layout(set = 0, binding = 0) buffer SSBO { float16_t x; } data;
                void main(){
                   float16_t a = data.x + data.x;
                   gl_Position = vec4(float(a) * 0.0);
                }
            )glsl";
            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);

            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }

        if (storage_16_bit_features.uniformAndStorageBuffer16BitAccess == VK_TRUE) {
            char const *vsSource = R"glsl(
                #version 450
                #extension GL_EXT_shader_16bit_storage: enable
                #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
                layout(set = 0, binding = 0) uniform UBO { float16_t x; } data;
                void main(){
                   float16_t a = data.x + data.x;
                   gl_Position = vec4(float(a) * 0.0);
                }
            )glsl";
            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);

            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }

        if (storage_16_bit_features.storagePushConstant16 == VK_TRUE) {
            char const *vsSource = R"glsl(
                #version 450
                #extension GL_EXT_shader_16bit_storage: enable
                #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
                layout(push_constant) uniform PushConstant { float16_t x; } data;
                void main(){
                   float16_t a = data.x + data.x;
                   gl_Position = vec4(float(a) * 0.0);
                }
            )glsl";
            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);

            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }

        if (storage_16_bit_features.storageInputOutput16 == VK_TRUE) {
            char const *vsSource = R"glsl(
                #version 450
                #extension GL_EXT_shader_16bit_storage: enable
                #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
                layout(location = 0) out float16_t outData;
                void main(){
                   outData = float16_t(1);
                   gl_Position = vec4(0.0);
                }
            )glsl";
            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);

            // Need to match in/out
            char const *fsSource = R"glsl(
                #version 450
                #extension GL_EXT_shader_16bit_storage: enable
                #extension GL_EXT_shader_explicit_arithmetic_types_float16: enable
                layout(location = 0) in float16_t x;
                layout(location = 0) out vec4 uFragColor;
                void main(){
                   uFragColor = vec4(0,1,0,1);
                }
            )glsl";
            VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_1);

            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }
    }

    // 16 bit int tests
    if ((support_16_bit == true) && (features2.features.shaderInt16 == VK_TRUE)) {
        if (storage_16_bit_features.storageBuffer16BitAccess == VK_TRUE) {
            char const *vsSource = R"glsl(
                #version 450
                #extension GL_EXT_shader_16bit_storage: enable
                #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
                layout(set = 0, binding = 0) buffer SSBO { int16_t x; } data;
                void main(){
                   int16_t a = data.x + data.x;
                   gl_Position = vec4(float(a) * 0.0);
                }
            )glsl";
            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);

            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }

        if (storage_16_bit_features.uniformAndStorageBuffer16BitAccess == VK_TRUE) {
            char const *vsSource = R"glsl(
                #version 450
                #extension GL_EXT_shader_16bit_storage: enable
                #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
                layout(set = 0, binding = 0) uniform UBO { int16_t x; } data;
                void main(){
                   int16_t a = data.x + data.x;
                   gl_Position = vec4(float(a) * 0.0);
                }
            )glsl";
            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);

            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }

        if (storage_16_bit_features.storagePushConstant16 == VK_TRUE) {
            char const *vsSource = R"glsl(
                #version 450
                #extension GL_EXT_shader_16bit_storage: enable
                #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
                layout(push_constant) uniform PushConstant { int16_t x; } data;
                void main(){
                   int16_t a = data.x + data.x;
                   gl_Position = vec4(float(a) * 0.0);
                }
            )glsl";
            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);

            VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
            VkPipelineLayoutCreateInfo pipeline_layout_info{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};
            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
                helper.pipeline_layout_ci_ = pipeline_layout_info;
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }

        if (storage_16_bit_features.storageInputOutput16 == VK_TRUE) {
            char const *vsSource = R"glsl(
                #version 450
                #extension GL_EXT_shader_16bit_storage: enable
                #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
                layout(location = 0) out int16_t outData;
                void main(){
                   outData = int16_t(1);
                   gl_Position = vec4(0.0);
                }
            )glsl";
            VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1);

            // Need to match in/out
            char const *fsSource = R"glsl(
                #version 450
                #extension GL_EXT_shader_16bit_storage: enable
                #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable
                layout(location = 0) flat in int16_t x;
                layout(location = 0) out vec4 uFragColor;
                void main(){
                   uFragColor = vec4(0,1,0,1);
                }
            )glsl";
            VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_1);

            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }
    }
}

TEST_F(PositiveShaderSpirv, SubgroupRotate) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_SUBGROUP_ROTATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderSubgroupRotate);
    RETURN_IF_SKIP(Init());

    char const *source = R"glsl(
        #version 450
        #extension GL_KHR_shader_subgroup_rotate: enable
        layout(binding = 0) buffer Buffers { vec4  x; } data;
        void main() {
            data.x = subgroupRotate(data.x, 1);
        }
    )glsl";

    VkShaderObj const cs(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
}

TEST_F(PositiveShaderSpirv, SubgroupRotateClustered) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_SUBGROUP_ROTATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderSubgroupRotate);
    AddRequiredFeature(vkt::Feature::shaderSubgroupRotateClustered);
    RETURN_IF_SKIP(Init());

    char const *source = R"glsl(
        #version 450
        #extension GL_KHR_shader_subgroup_rotate: enable
        layout(binding = 0) buffer Buffers { vec4  x; } data;
        void main() {
            data.x = subgroupClusteredRotate(data.x, 1, 1);
        }
    )glsl";

    VkShaderObj const cs(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
}

TEST_F(PositiveShaderSpirv, ReadShaderClockDevice) {
    TEST_DESCRIPTION("Test VK_KHR_shader_clock");

    AddRequiredExtensions(VK_KHR_SHADER_CLOCK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderDeviceClock);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Device scope using GL_EXT_shader_realtime_clock
    char const *vsSourceDevice = R"glsl(
        #version 450
        #extension GL_EXT_shader_realtime_clock: enable
        void main(){
           uvec2 a = clockRealtime2x32EXT();
           gl_Position = vec4(float(a.x) * 0.0);
        }
    )glsl";
    VkShaderObj vs_device(this, vsSourceDevice, VK_SHADER_STAGE_VERTEX_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs_device.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderSpirv, ReadShaderClockSubgroup) {
    TEST_DESCRIPTION("Test VK_KHR_shader_clock");

    AddRequiredExtensions(VK_KHR_SHADER_CLOCK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderSubgroupClock);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Subgroup scope using ARB_shader_clock
    char const *vsSourceScope = R"glsl(
        #version 450
        #extension GL_ARB_shader_clock: enable
        void main(){
           uvec2 a = clock2x32ARB();
           gl_Position = vec4(float(a.x) * 0.0);
        }
    )glsl";
    VkShaderObj vs_subgroup(this, vsSourceScope, VK_SHADER_STAGE_VERTEX_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs_subgroup.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderSpirv, PhysicalStorageBufferStructRecursion) {
    TEST_DESCRIPTION("Make sure shader can have a buffer_reference that contains itself.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *cs_src = R"glsl(
#version 450 core
#extension GL_EXT_buffer_reference : enable

layout(buffer_reference) buffer T1;

layout(set = 0, binding = 0, std140) uniform T2 {
   layout(offset = 0) T1 a[2];
};

// This struct calls itself which needs to be properly handled in the shader validation or it will infinite loop
layout(buffer_reference, std140) buffer T1 {
   layout(offset = 0) T1 b[2];
};

void main() {}
        )glsl";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, cs_src, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderSpirv, OpCopyObjectSampler) {
    TEST_DESCRIPTION("Reproduces a use case involving GL_EXT_nonuniform_qualifier and image samplers found in Doom Eternal trace");

    // https://github.com/KhronosGroup/glslang/pull/1762 appears to be the change that introduces the OpCopyObject in this context.

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderSampledImageArrayNonUniformIndexing);
    AddRequiredFeature(vkt::Feature::shaderStorageTexelBufferArrayNonUniformIndexing);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *vertex_source = R"glsl(
#version 450

layout(location=0) out int idx;

void main() {
    idx = 0;
    gl_Position = vec4(0.0);
}
        )glsl";
    const VkShaderObj vs(this, vertex_source, VK_SHADER_STAGE_VERTEX_BIT);

    const char *fragment_source = R"glsl(
#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(set=0, binding=0) uniform sampler s;
layout(set=0, binding=1) uniform texture2D t[1];
layout(location=0) in flat int idx;

layout(location=0) out vec4 frag_color;

void main() {
    // Using nonuniformEXT on the index into the image array creates the OpCopyObject instead of an OpLoad, which
    // was causing problems with how constants are identified.
    frag_color = texture(sampler2D(t[nonuniformEXT(idx)], s), vec2(0.0));
}

    )glsl";
    const VkShaderObj fs(this, fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
    };
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderSpirv, SpecConstantTextureArrayTessellation) {
    TEST_DESCRIPTION("Reproduces https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6370");
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(Init());
    const char *source = R"glsl(
#version 440
layout(triangles, equal_spacing, cw) in;

layout(constant_id = 0) const int MAX_NUM_DESCRIPTOR_IMAGES = 100;

layout(location = 0) in vec2 inHeightTexCoordinates[];
layout(location = 1) in int inHeightTextureIndex[];

layout(set = 0, binding = 1) uniform sampler textureSampler;
layout(set = 1, binding = 1) uniform texture2D heightTextures[MAX_NUM_DESCRIPTOR_IMAGES];

vec2 mixVec2(const vec2 vectors[gl_MaxPatchVertices]) {
    return gl_TessCoord.x * vectors[0] + gl_TessCoord.y * vectors[1] + gl_TessCoord.z * vectors[2];
}

void main() {
    vec2 heightTexCoordinates = mixVec2(inHeightTexCoordinates);
    int heightTextureIndex = inHeightTextureIndex[0];
    float extraHeight = texture(sampler2D(heightTextures[heightTextureIndex], textureSampler), heightTexCoordinates).r;
}
        )glsl";
    const VkShaderObj tese(this, source, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
}

TEST_F(PositiveShaderSpirv, SpecConstantTextureArrayVertex) {
    TEST_DESCRIPTION("Reproduces https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6370");
    RETURN_IF_SKIP(Init());
    const char *source = R"glsl(
#version 450
layout(constant_id = 0) const int MAX_NUM_DESCRIPTOR_IMAGES = 100;

layout(location = 0) in int index;

layout(set = 0, binding = 1) uniform sampler textureSampler;
layout(set = 1, binding = 1) uniform texture2D heightTextures[MAX_NUM_DESCRIPTOR_IMAGES];

void main() {
    float extraHeight = texture(sampler2D(heightTextures[index], textureSampler), vec2(0.0)).r;
}
        )glsl";
    const VkShaderObj vs(this, source, VK_SHADER_STAGE_VERTEX_BIT);
}

TEST_F(PositiveShaderSpirv, SpecConstantTextureIndexDefault) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6293");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *fragment_source = R"glsl(
        #version 450
        layout (location = 0) out vec4 out_color;

        layout (constant_id = 0) const int num_textures = 2;
        layout (binding = 0) uniform sampler2D textures[num_textures];

        void main() {
            out_color = texture(textures[0], vec2(0.0));
        }
    )glsl";

    const VkShaderObj fs(this, fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr}};
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderSpirv, SpecConstantTextureIndexValue) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *fragment_source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %out_color
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %out_color Location 0
               OpDecorate %num_textures SpecId 0
               OpDecorate %textures DescriptorSet 0
               OpDecorate %textures Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %out_color = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
        %int = OpTypeInt 32 1
        ;; index is invalid
%num_textures = OpSpecConstant %int 2
%_arr_11_num_textures = OpTypeArray %11 %num_textures
%_ptr_UniformConstant__arr_11_num_textures = OpTypePointer UniformConstant %_arr_11_num_textures
   %textures = OpVariable %_ptr_UniformConstant__arr_11_num_textures UniformConstant
      %int_2 = OpConstant %int 2
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %23 = OpConstantComposite %v2float %float_0 %float_0
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_UniformConstant_11 %textures %int_2
         %20 = OpLoad %11 %19
         %24 = OpImageSampleImplicitLod %v4float %20 %23
               OpStore %out_color %24
               OpReturn
               OpFunctionEnd
        )";

    uint32_t data = 3;
    VkSpecializationMapEntry entry = {0, 0, sizeof(uint32_t)};
    VkSpecializationInfo specialization_info = {1, &entry, sizeof(uint32_t), &data};
    const VkShaderObj fs(this, fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM,
                         &specialization_info);

    CreatePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr}};
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderSpirv, DescriptorCountSpecConstant) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        // over VkDescriptorSetLayoutBinding::descriptorCount
        layout (constant_id = 0) const int index = 4;
        layout (set = 0, binding = 0) uniform sampler2D tex[index];
        layout (location = 0) out vec4 out_color;
        void main() {
            out_color = textureLodOffset(tex[1], vec2(0), 0, ivec2(0));
        }
    )glsl";

    uint32_t data = 2;
    VkSpecializationMapEntry entry = {0, 0, sizeof(uint32_t)};
    VkSpecializationInfo specialization_info = {1, &entry, sizeof(uint32_t), &data};
    const VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, &specialization_info);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderSpirv, PhysicalStorageBufferGlslang6) {
    TEST_DESCRIPTION("Taken from glslang spv.bufferhandle6.frag test");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);

    RETURN_IF_SKIP(Init());

    char const *fsSource = R"glsl(
        #version 450 core
        #extension GL_EXT_buffer_reference : enable
        layout (push_constant, std430) uniform Block { int identity[32]; } pc;
        layout(r32ui, set = 3, binding = 0) uniform uimage2D image0_0;
        layout(buffer_reference) buffer T1;
        layout(set = 3, binding = 1, buffer_reference) buffer T1 {
        layout(offset = 0) int a[2]; // stride = 4 for std430, 16 for std140
        layout(offset = 32) int b;
        layout(offset = 48) T1  c[2]; // stride = 8 for std430, 16 for std140
        layout(offset = 80) T1  d;
        } x;
        void main() {}
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
}

TEST_F(PositiveShaderSpirv, ShaderFloatControl2) {
    TEST_DESCRIPTION("Basic usage of VK_KHR_shader_float_controls2");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT_CONTROLS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloatControls2);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFloatControlsProperties shader_float_control = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(shader_float_control);
    if (!shader_float_control.shaderSignedZeroInfNanPreserveFloat32) {
        GTEST_SKIP() << "shaderSignedZeroInfNanPreserveFloat32 not supported";
    }

    const char *spv_source = R"(
        OpCapability Shader
        OpCapability FloatControls2
        OpExtension "SPV_KHR_float_controls2"
        OpMemoryModel Logical GLSL450
        OpEntryPoint GLCompute %main "main"
        OpExecutionModeId %main FPFastMathDefault %float %constant
        OpExecutionMode %main LocalSize 1 1 1
        OpDecorate %add FPFastMathMode Fast
        %void = OpTypeVoid
        %int = OpTypeInt 32 0
        %constant = OpConstant %int 0
        %float = OpTypeFloat 32
        %zero = OpConstant %float 0
        %void_fn = OpTypeFunction %void
        %main = OpFunction %void None %void_fn
        %entry = OpLabel
        OpReturn
        OpFunctionEnd
        %func = OpFunction %void None %void_fn
        %func_entry = OpLabel
        %add = OpFAdd %float %zero %zero
        OpReturn
        OpFunctionEnd
        )";

    VkShaderObj cs(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_ASM);
}

TEST_F(PositiveShaderSpirv, FPFastMathMode) {
    TEST_DESCRIPTION("Use NSZ, NotInf, and NotNaN for all fast math mode");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT_CONTROLS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloatControls2);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFloatControlsProperties shader_float_control = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(shader_float_control);
    if (shader_float_control.shaderSignedZeroInfNanPreserveFloat32) {
        GTEST_SKIP() << "shaderSignedZeroInfNanPreserveFloat32 is supported";
    }

    const char *spv_source = R"(
        OpCapability Shader
        OpCapability FloatControls2
        OpExtension "SPV_KHR_float_controls2"
        OpMemoryModel Logical GLSL450
        OpEntryPoint GLCompute %main "main"
        OpExecutionModeId %main FPFastMathDefault %float %constant
        OpExecutionMode %main LocalSize 1 1 1
        OpDecorate %add FPFastMathMode NSZ|NotInf|NotNaN
        %void = OpTypeVoid
        %int = OpTypeInt 32 0
        %constant = OpConstant %int 7
        %float = OpTypeFloat 32
        %zero = OpConstant %float 0
        %void_fn = OpTypeFunction %void
        %main = OpFunction %void None %void_fn
        %entry = OpLabel
        OpReturn
        OpFunctionEnd
        %func = OpFunction %void None %void_fn
        %func_entry = OpLabel
        %add = OpFAdd %float %zero %zero
        OpReturn
        OpFunctionEnd
        )";

    VkShaderObj cs(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_ASM);
}

TEST_F(PositiveShaderSpirv, ScalarBlockLayoutShaderCache) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8031");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::scalarBlockLayout);  // will set --scalar-block-layout
    RETURN_IF_SKIP(Init());

    // Matches glsl from other ScalarBlockLayoutShaderCache test
    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_buffer_reference : require
        #extension GL_EXT_scalar_block_layout : require

        struct Transform {
            mat3x3 rotScaMatrix; //  0, 36
            vec3 pos;            // 36, 12
            vec3 pos_err;        // 48, 12
            float padding;       // 60, 4
        };

        layout(scalar, buffer_reference, buffer_reference_align = 64) readonly buffer Transforms {
            Transform transforms[];
        };
        layout(std430, push_constant) uniform PushConstant {
            Transforms pTransforms;
        };

        void main() {
            Transform transform = pTransforms.transforms[0];
        }
    )glsl";
    VkShaderObj cs(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
}

TEST_F(PositiveShaderSpirv, ExtendedTypesEnabled) {
    TEST_DESCRIPTION("Test VK_KHR_shader_subgroup_extended_types.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    AddRequiredFeature(vkt::Feature::shaderSubgroupExtendedTypes);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceSubgroupProperties subgroup_prop = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(subgroup_prop);
    if (!(subgroup_prop.supportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT) ||
        !(subgroup_prop.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT)) {
        GTEST_SKIP() << "Required features not supported";
    }

    std::vector<VkDescriptorSetLayoutBinding> bindings(0);
    const vkt::DescriptorSetLayout dsl(*m_device, bindings);
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    char const *csSource = R"glsl(
        #version 450
        #extension GL_KHR_shader_subgroup_arithmetic : enable
        #extension GL_EXT_shader_subgroup_extended_types_float16 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        layout(local_size_x = 32) in;
        void main() {
           subgroupAdd(float16_t(0.0));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderSpirv, RayQueryPositionFetch) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8055");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredFeature(vkt::Feature::rayTracingPositionFetch);
    RETURN_IF_SKIP(Init());

    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_ray_query : enable
        #extension GL_EXT_ray_tracing_position_fetch : enable

        layout(set = 0, binding = 0) buffer Log { uint x; };
        layout(set = 0, binding = 1) uniform accelerationStructureEXT rtas;

        void main() {
            rayQueryEXT rayQuery;
            rayQueryInitializeEXT(rayQuery, rtas, gl_RayFlagsNoneEXT, 0xFF, vec3(0,0,0), 0.0, vec3(1,0,0), 1.0);

            vec3 positions[3];
            rayQueryGetIntersectionTriangleVertexPositionsEXT(rayQuery, true, positions);
            if (positions[0].x > 0) {
                x = 2;
            } else {
                x = 1;
            }
        }
    )glsl";
    VkShaderObj cs(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
}

TEST_F(PositiveShaderSpirv, ImageGatherOffsetMaintenance8) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::shaderImageGatherExtended);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance8);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *spv_source = R"(
               OpCapability Shader
               OpCapability ImageGatherExtended
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %tex DescriptorSet 0
               OpDecorate %tex Binding 2
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %8 = OpTypeImage %float 2D 0 0 0 1 Unknown
          %9 = OpTypeSampledImage %8
%_ptr_UniformConstant_9 = OpTypePointer UniformConstant %9
        %tex = OpVariable %_ptr_UniformConstant_9 UniformConstant
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %15 = OpConstantComposite %v2float %float_0 %float_0
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
         %19 = OpConstantComposite %v2int %int_0 %int_0
    %v4float = OpTypeVector %float 4
       %main = OpFunction %void None %4
          %6 = OpLabel
         %12 = OpLoad %9 %tex
         %21 = OpImageSampleExplicitLod %v4float %12 %15 Lod|Offset %float_0 %19
               OpReturn
               OpFunctionEnd
    )";
    VkShaderObj const fs(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
}
