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

class PositiveShaderCompute : public VkLayerTest {};

TEST_F(PositiveShaderCompute, WorkGroupSizePrecedenceOverLocalSize) {
    // "If an object is decorated with the WorkgroupSize decoration, this takes precedence over any LocalSize or LocalSizeId
    // execution mode."
    TEST_DESCRIPTION("Make sure work WorkgroupSize decoration is used over LocalSize");

    RETURN_IF_SKIP(Init());

    uint32_t x_size_limit = m_device->Physical().limits_.maxComputeWorkGroupSize[0];
    uint32_t y_size_limit = m_device->Physical().limits_.maxComputeWorkGroupSize[1];
    uint32_t z_size_limit = m_device->Physical().limits_.maxComputeWorkGroupSize[2];

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize )";
    spv_source << std::to_string(x_size_limit + 1) << " " << std::to_string(y_size_limit + 1) << " "
               << std::to_string(z_size_limit + 1);
    spv_source << R"(
               OpSource GLSL 450
               OpName %main "main"
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                                   SPV_SOURCE_ASM);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderCompute, WorkGroupSizeSpecConstantUnder) {
    TEST_DESCRIPTION("Make sure spec constants get applied to to be under maxComputeWorkGroupSize");

    RETURN_IF_SKIP(Init());

    uint32_t x_size_limit = m_device->Physical().limits_.maxComputeWorkGroupSize[0];

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpDecorate %7 SpecId 0
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
          %7 = OpSpecConstant %uint )";
    spv_source << std::to_string(x_size_limit + 1);
    spv_source << R"(
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpSpecConstantComposite %v3uint %7 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    uint32_t data = 1;

    VkSpecializationMapEntry entry;
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                                   SPV_SOURCE_ASM, &specialization_info);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderCompute, WorkGroupSizeLocalSizeId) {
    TEST_DESCRIPTION("Validate LocalSizeId doesn't triggers maxComputeWorkGroupSize limit");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::maintenance4);
    RETURN_IF_SKIP(Init());

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %uint_2 %uint_1 %uint_1
               OpSource GLSL 450
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3,
                                                   SPV_SOURCE_ASM);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderCompute, WorkGroupSizeLocalSizeIdSpecConstant) {
    TEST_DESCRIPTION("Validate LocalSizeId doesn't triggers maxComputeWorkGroupSize limit with spec constants");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::maintenance4);
    RETURN_IF_SKIP(Init());

    uint32_t x_size_limit = m_device->Physical().limits_.maxComputeWorkGroupSize[0];

    // layout(local_size_x_id = 18, local_size_z_id = 19) in;
    // layout(local_size_x = 32) in;
    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %spec_x %uint_1 %spec_z
               OpSource GLSL 450
               OpDecorate %spec_x SpecId 18
               OpDecorate %spec_z SpecId 19
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %spec_x = OpSpecConstant %uint 32
     %uint_1 = OpConstant %uint 1
     %spec_z = OpSpecConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    uint32_t data = x_size_limit - 1;

    VkSpecializationMapEntry entry;
    entry.constantID = 18;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3,
                                                   SPV_SOURCE_ASM, &specialization_info);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderCompute, WorkGroupSizePrecedenceOverLocalSizeId) {
    // "If an object is decorated with the WorkgroupSize decoration, this takes precedence over any LocalSize or LocalSizeId
    // execution mode."
    TEST_DESCRIPTION("Make sure work WorkgroupSize decoration is used over LocalSizeId");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::maintenance4);
    RETURN_IF_SKIP(Init());

    uint32_t x_size_limit = m_device->Physical().limits_.maxComputeWorkGroupSize[0];

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %spec_x %uint_1 %uint_1
               OpSource GLSL 450
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
               OpDecorate %spec_x SpecId 18
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %spec_x = OpSpecConstant %uint )";
    spv_source << std::to_string(x_size_limit + 1);
    spv_source << R"(
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3,
                                                   SPV_SOURCE_ASM);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderCompute, SharedMemorySpecConstantOp) {
    TEST_DESCRIPTION("Validate compute shader shared memory");

    RETURN_IF_SKIP(Init());

    const uint32_t max_shared_memory_size = m_device->Physical().limits_.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    if (max_shared_ints < 16 * 7) {
        GTEST_SKIP() << "Supported compute shader shared memory size is too small";
    }

    char const *cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

        layout(constant_id = 0) const uint Condition = 0;
        layout(constant_id = 1) const uint SharedSize = 16;

        #define enableSharedMemoryOpt (Condition == 1 || Condition == 2 || Condition == 3)
        shared uint arr[enableSharedMemoryOpt ? SharedSize : 1][enableSharedMemoryOpt ? 7 : 1];

        void main() {}
    )glsl";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderCompute, SharedMemory) {
    TEST_DESCRIPTION("Validate compute shader shared memory does not exceed maxComputeSharedMemorySize");

    RETURN_IF_SKIP(Init());

    // Make sure compute pipeline has a compute shader stage set
    char const *csSource = R"glsl(
        #version 450
        shared uint a;
        shared float b;
        shared vec2 c;
        shared mat3 d;
        shared mat4 e[3];
        struct A {
            int f;
            float g;
            uint h;
        };
        shared A f;
        void main(){
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderCompute, ZeroInitializeWorkgroupMemoryFeature) {
    TEST_DESCRIPTION("Enable and use shaderZeroInitializeWorkgroupMemory feature");

    AddRequiredExtensions(VK_KHR_ZERO_INITIALIZE_WORKGROUP_MEMORY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderZeroInitializeWorkgroupMemory);
    RETURN_IF_SKIP(Init());

    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpName %main "main"
               OpName %counter "counter"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Workgroup_uint = OpTypePointer Workgroup %uint
  %zero_uint = OpConstantNull %uint
    %counter = OpVariable %_ptr_Workgroup_uint Workgroup %zero_uint
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    auto cs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT);
    const auto set_info = [&cs](CreateComputePipelineHelper &helper) { helper.cs_ = std::move(cs); };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}
