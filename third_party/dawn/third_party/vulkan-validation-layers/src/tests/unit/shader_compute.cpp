/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
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

class NegativeShaderCompute : public VkLayerTest {};

TEST_F(NegativeShaderCompute, SharedMemoryOverLimit) {
    TEST_DESCRIPTION("Validate compute shader shared memory does not exceed maxComputeSharedMemorySize");

    RETURN_IF_SKIP(Init());

    const uint32_t max_shared_memory_size = m_device->Physical().limits_.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream csSource;
    // Make sure compute pipeline has a compute shader stage set
    csSource << R"glsl(
        #version 450
        shared int a[)glsl";
    csSource << (max_shared_ints + 16);
    csSource << R"glsl(];
        void main(){
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Workgroup-06530");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCompute, SharedMemoryBooleanOverLimit) {
    TEST_DESCRIPTION("Validate compute shader shared memory does not exceed maxComputeSharedMemorySize with booleans");

    RETURN_IF_SKIP(Init());

    const uint32_t max_shared_memory_size = m_device->Physical().limits_.maxComputeSharedMemorySize;
    // "Boolean values considered as 32-bit integer values for the purpose of this calculation."
    const uint32_t max_shared_bools = max_shared_memory_size / 4;

    std::stringstream csSource;
    // Make sure compute pipeline has a compute shader stage set
    csSource << R"glsl(
        #version 450
        shared bool a[)glsl";
    csSource << (max_shared_bools + 16);
    csSource << R"glsl(];
        void main(){
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Workgroup-06530");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCompute, SharedMemoryOverLimitWorkgroupMemoryExplicitLayout) {
    TEST_DESCRIPTION(
        "Validate compute shader shared memory does not exceed maxComputeSharedMemorySize when using "
        "VK_KHR_workgroup_memory_explicit_layout");
    // need at least SPIR-V 1.4 for SPV_KHR_workgroup_memory_explicit_layout
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::workgroupMemoryExplicitLayout);
    RETURN_IF_SKIP(Init());

    const uint32_t max_shared_memory_size = m_device->Physical().limits_.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream csSource;
    csSource << R"glsl(
        #version 450
        #extension GL_EXT_shared_memory_block : enable

        shared X {
            int x;
        };

        shared Y {
            int y1[)glsl";
    csSource << (max_shared_ints + 16);
    csSource << R"glsl(];
            int y2;
        };

        void main() {
            x = 0; // prevent dead-code elimination
            y2 = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Workgroup-06530");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCompute, SharedMemorySpecConstantDefault) {
    TEST_DESCRIPTION("Validate shared memory exceed maxComputeSharedMemorySize limit with spec constants default");

    RETURN_IF_SKIP(Init());

    const uint32_t max_shared_memory_size = m_device->Physical().limits_.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream cs_source;
    cs_source << R"glsl(
        #version 450
        layout(constant_id = 0) const uint Condition = 1;
        layout(constant_id = 1) const uint SharedSize = )glsl";
    cs_source << (max_shared_ints + 16);
    cs_source << R"glsl(;

        #define enableSharedMemoryOpt (Condition == 1)
        shared uint arr[enableSharedMemoryOpt ? SharedSize : 1];
        void main(){}
    )glsl";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, cs_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Workgroup-06530");
}

TEST_F(NegativeShaderCompute, SharedMemorySpecConstantSet) {
    TEST_DESCRIPTION("Validate shared memory exceed maxComputeSharedMemorySize limit with spec constants set");

    RETURN_IF_SKIP(Init());

    const uint32_t max_shared_memory_size = m_device->Physical().limits_.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream cs_source;
    cs_source << R"glsl(
        #version 450
        layout(constant_id = 0) const uint Condition = 0;
        layout(constant_id = 1) const uint SharedSize = )glsl";
    cs_source << (max_shared_ints + 16);
    cs_source << R"glsl(;

        #define enableSharedMemoryOpt (Condition == 1)
        shared uint arr[enableSharedMemoryOpt ? SharedSize : 1];
        void main(){}
    )glsl";

    uint32_t data = 1;  // set Condition

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
        helper.cs_ = std::make_unique<VkShaderObj>(this, cs_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                                   SPV_SOURCE_GLSL, &specialization_info);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Workgroup-06530");
}

TEST_F(NegativeShaderCompute, WorkGroupSizeSpecConstant) {
    TEST_DESCRIPTION("Validate compute shader shared memory does not exceed maxComputeWorkGroupSize");

    RETURN_IF_SKIP(Init());
    const VkPhysicalDeviceLimits limits = m_device->Physical().limits_;

    // Make sure compute pipeline has a compute shader stage set
    const char *cs_source = R"glsl(
        #version 450
        layout(local_size_x_id = 3, local_size_y_id = 4) in;
        void main(){}
    )glsl";

    VkSpecializationMapEntry entries[2];
    entries[0].constantID = 3;
    entries[0].offset = 0;
    entries[0].size = sizeof(uint32_t);
    entries[1].constantID = 4;
    entries[1].offset = sizeof(uint32_t);
    entries[1].size = sizeof(uint32_t);

    uint32_t data[2] = {
        1,
        limits.maxComputeWorkGroupSize[1] + 1,  // Invalid
    };

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 2;
    specialization_info.pMapEntries = entries;
    specialization_info.dataSize = sizeof(uint32_t) * 2;
    specialization_info.pData = data;

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                                   SPV_SOURCE_GLSL, &specialization_info);
    };
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-x-06432");
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-y-06430");

    data[0] = limits.maxComputeWorkGroupSize[0] + 1;  // Invalid
    data[1] = 1;
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-x-06432");
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-x-06429");

    data[0] = limits.maxComputeWorkGroupSize[0];
    data[1] = limits.maxComputeWorkGroupSize[1];
    if ((data[0] + data[1]) > limits.maxComputeWorkGroupInvocations) {
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-x-06432");
    }
}

TEST_F(NegativeShaderCompute, WorkGroupSizeConstantDefault) {
    TEST_DESCRIPTION("Make sure constant are applied for maxComputeWorkGroupSize using WorkgroupSize");

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
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %limit = OpConstant %uint )";
    spv_source << std::to_string(x_size_limit + 1);
    spv_source << R"(
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpConstantComposite %v3uint %limit %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                                   SPV_SOURCE_ASM);
    };
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-x-06432");
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-x-06429");
}

TEST_F(NegativeShaderCompute, WorkGroupSizeSpecConstantDefault) {
    TEST_DESCRIPTION("Make sure spec constant are applied for maxComputeWorkGroupSize using WorkgroupSize");

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
               OpDecorate %limit SpecId 0
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %limit = OpSpecConstant %uint )";
    spv_source << std::to_string(x_size_limit + 1);
    spv_source << R"(
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpSpecConstantComposite %v3uint %limit %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                                   SPV_SOURCE_ASM);
    };
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-x-06432");
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-x-06429");
}

TEST_F(NegativeShaderCompute, WorkGroupSizeLocalSizeId) {
    TEST_DESCRIPTION("Validate LocalSizeId also triggers maxComputeWorkGroupSize limit");
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
               OpExecutionModeId %main LocalSizeId %limit %uint_1 %uint_1
               OpSource GLSL 450
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %limit = OpConstant %uint )";
    spv_source << std::to_string(x_size_limit + 1);
    spv_source << R"(
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
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-x-06432");
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-x-06429");
}

TEST_F(NegativeShaderCompute, WorkGroupSizeLocalSizeIdSpecConstantDefault) {
    TEST_DESCRIPTION("Validate LocalSizeId also triggers maxComputeWorkGroupSize limit with spec constants default");
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
      %spec_x = OpSpecConstant %uint )";
    spv_source << std::to_string(x_size_limit + 1);
    spv_source << R"(
     %uint_1 = OpConstant %uint 1
     %spec_z = OpSpecConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3,
                                                   SPV_SOURCE_ASM);
    };
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-x-06432");
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-x-06429");
}

TEST_F(NegativeShaderCompute, WorkGroupSizeLocalSizeIdSpecConstantSet) {
    TEST_DESCRIPTION("Validate LocalSizeId also triggers maxComputeWorkGroupSize limit with spec constants");
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

    uint32_t data = x_size_limit + 1;

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
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-x-06432");
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-x-06429");
}

TEST_F(NegativeShaderCompute, WorkgroupMemoryExplicitLayout) {
    TEST_DESCRIPTION("Test VK_KHR_workgroup_memory_explicit_layout");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::shaderInt16);
    AddRequiredFeature(vkt::Feature::shaderFloat16);
    RETURN_IF_SKIP(Init());

    // WorkgroupMemoryExplicitLayoutKHR
    {
        const char *spv_source = R"(
               OpCapability Shader
               OpCapability WorkgroupMemoryExplicitLayoutKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 8 1 1
               OpMemberDecorate %first 0 Offset 0
               OpDecorate %first Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %first = OpTypeStruct %int
%_ptr_Workgroup_first = OpTypePointer Workgroup %first
          %_ = OpVariable %_ptr_Workgroup_first Workgroup
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
%_ptr_Workgroup_int = OpTypePointer Workgroup %int
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpAccessChain %_ptr_Workgroup_int %_ %int_0
               OpStore %13 %int_2
               OpReturn
               OpFunctionEnd
        )";
        // Both missing enabling the extension and capability feature
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
        m_errorMonitor->VerifyFound();
    }

    // WorkgroupMemoryExplicitLayout8BitAccessKHR (shaderInt8)
    {
        const char *spv_source = R"(
               OpCapability Shader
               OpCapability Int8
               OpCapability WorkgroupMemoryExplicitLayout8BitAccessKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 2 1 1
               OpMemberDecorate %first 0 Offset 0
               OpDecorate %first Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %char = OpTypeInt 8 1
      %first = OpTypeStruct %char
%_ptr_Workgroup_first = OpTypePointer Workgroup %first
          %_ = OpVariable %_ptr_Workgroup_first Workgroup
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %char_2 = OpConstant %char 2
%_ptr_Workgroup_char = OpTypePointer Workgroup %char
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpAccessChain %_ptr_Workgroup_char %_ %int_0
               OpStore %14 %char_2
               OpReturn
               OpFunctionEnd
        )";

        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
        m_errorMonitor->VerifyFound();
    }

    // WorkgroupMemoryExplicitLayout16BitAccessKHR (shaderInt16)
    {
        const char *spv_source = R"(
               OpCapability Shader
               OpCapability Float16
               OpCapability Int16
               OpCapability WorkgroupMemoryExplicitLayout16BitAccessKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 2 1 1
               OpMemberDecorate %first 0 Offset 0
               OpMemberDecorate %first 1 Offset 2
               OpDecorate %first Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %short = OpTypeInt 16 1
       %half = OpTypeFloat 16
      %first = OpTypeStruct %short %half
%_ptr_Workgroup_first = OpTypePointer Workgroup %first
          %_ = OpVariable %_ptr_Workgroup_first Workgroup
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %short_3 = OpConstant %short 3
%_ptr_Workgroup_short = OpTypePointer Workgroup %short
      %int_1 = OpConstant %int 1
%half_0x1_898p_3 = OpConstant %half 0x1.898p+3
%_ptr_Workgroup_half = OpTypePointer Workgroup %half
       %main = OpFunction %void None %3
          %5 = OpLabel
         %15 = OpAccessChain %_ptr_Workgroup_short %_ %int_0
               OpStore %15 %short_3
         %19 = OpAccessChain %_ptr_Workgroup_half %_ %int_1
               OpStore %19 %half_0x1_898p_3
               OpReturn
               OpFunctionEnd
        )";

        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08742");
        VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
        m_errorMonitor->VerifyFound();
    }

    // workgroupMemoryExplicitLayoutScalarBlockLayout feature
    // will fail from not passing --workgroup-scalar-block-layout in spirv-val
    {
        const char *spv_source = R"(
               OpCapability Shader
               OpCapability WorkgroupMemoryExplicitLayoutKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %B
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpMemberDecorate %S 2 Offset 16
               OpMemberDecorate %S 3 Offset 28
               OpDecorate %S Block
               OpDecorate %B Aliased
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %S = OpTypeStruct %float %v3float %v3float %v3float
%_ptr_Workgroup_S = OpTypePointer Workgroup %S
          %B = OpVariable %_ptr_Workgroup_S Workgroup
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

        m_errorMonitor->SetDesiredError("VUID-VkShaderModuleCreateInfo-pCode-08737");
        VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeShaderCompute, ZeroInitializeWorkgroupMemory) {
    TEST_DESCRIPTION("Test initializing workgroup memory in compute shader");
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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderZeroInitializeWorkgroupMemory-06372");
    VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCompute, LocalSizeIdExecutionMode) {
    TEST_DESCRIPTION("Test LocalSizeId spirv execution mode");
    SetTargetApiVersion(VK_API_VERSION_1_3);

    RETURN_IF_SKIP(Init());

    const char *source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
               OpSource GLSL 450
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-LocalSizeId-06434");
    VkShaderObj::CreateFromASM(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_UNIVERSAL_1_6);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCompute, LocalSizeIdExecutionModeMaintenance5) {
    TEST_DESCRIPTION("Test SPIRV is still checked if using new pNext in VkPipelineShaderStageCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);

    RETURN_IF_SKIP(Init());

    const char *source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
               OpSource GLSL 450
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";
    std::vector<uint32_t> shader;
    ASMtoSPV(SPV_ENV_UNIVERSAL_1_6, 0, source, shader);

    VkShaderModuleCreateInfo module_create_info = vku::InitStructHelper();
    module_create_info.pCode = shader.data();
    module_create_info.codeSize = shader.size() * sizeof(uint32_t);

    VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&module_create_info);
    stage_ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    vkt::PipelineLayout layout(*m_device, {});
    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.stage = stage_ci;
    pipe.cp_ci_.layout = layout.handle();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-LocalSizeId-06434");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderCompute, CmdDispatchExceedLimits) {
    TEST_DESCRIPTION("Compute dispatch with dimensions that exceed device limits");

    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool device_group_creation = IsExtensionsEnabled(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);

    uint32_t x_count_limit = m_device->Physical().limits_.maxComputeWorkGroupCount[0];
    uint32_t y_count_limit = m_device->Physical().limits_.maxComputeWorkGroupCount[1];
    uint32_t z_count_limit = m_device->Physical().limits_.maxComputeWorkGroupCount[2];
    if (std::max({x_count_limit, y_count_limit, z_count_limit}) == vvl::kU32Max) {
        GTEST_SKIP() << "device maxComputeWorkGroupCount limit reports UINT32_MAX";
    }

    uint32_t x_size_limit = m_device->Physical().limits_.maxComputeWorkGroupSize[0];
    uint32_t y_size_limit = m_device->Physical().limits_.maxComputeWorkGroupSize[1];
    uint32_t z_size_limit = m_device->Physical().limits_.maxComputeWorkGroupSize[2];

    std::string spv_source = R"(
        OpCapability Shader
        OpMemoryModel Logical GLSL450
        OpEntryPoint GLCompute %main "main"
        OpExecutionMode %main LocalSize )";
    spv_source.append(std::to_string(x_size_limit + 1) + " " + std::to_string(y_size_limit + 1) + " " +
                      std::to_string(z_size_limit + 1));
    spv_source.append(R"(
        %void = OpTypeVoid
           %3 = OpTypeFunction %void
        %main = OpFunction %void None %3
           %5 = OpLabel
                OpReturn
                OpFunctionEnd)");

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ =
        std::make_unique<VkShaderObj>(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-x-06429");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-y-06430");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-z-06431");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-x-06432");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    // Create a minimal compute pipeline
    x_size_limit = (x_size_limit > 1024) ? 1024 : x_size_limit;
    y_size_limit = (y_size_limit > 1024) ? 1024 : y_size_limit;
    z_size_limit = (z_size_limit > 64) ? 64 : z_size_limit;

    uint32_t invocations_limit = m_device->Physical().limits_.maxComputeWorkGroupInvocations;
    x_size_limit = (x_size_limit > invocations_limit) ? invocations_limit : x_size_limit;
    invocations_limit /= x_size_limit;
    y_size_limit = (y_size_limit > invocations_limit) ? invocations_limit : y_size_limit;
    invocations_limit /= y_size_limit;
    z_size_limit = (z_size_limit > invocations_limit) ? invocations_limit : z_size_limit;

    std::stringstream cs_text;
    cs_text << "#version 450\n";
    cs_text << "layout(local_size_x = " << x_size_limit << ", ";
    cs_text << "local_size_y = " << y_size_limit << ",";
    cs_text << "local_size_z = " << z_size_limit << ") in;\n";
    cs_text << "void main() {}\n";

    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_text.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.flags = VK_PIPELINE_CREATE_DISPATCH_BASE;
    pipe.CreateComputePipeline();

    // Bind pipeline to command buffer
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());

    // Dispatch counts that exceed device limits
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-groupCountX-00386");
    vk::CmdDispatch(m_command_buffer.handle(), x_count_limit + 1, y_count_limit, z_count_limit);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-groupCountY-00387");
    vk::CmdDispatch(m_command_buffer.handle(), x_count_limit, y_count_limit + 1, z_count_limit);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-groupCountZ-00388");
    vk::CmdDispatch(m_command_buffer.handle(), x_count_limit, y_count_limit, z_count_limit + 1);
    m_errorMonitor->VerifyFound();

    if (device_group_creation) {
        // Base equals or exceeds limit
        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatchBase-baseGroupX-00421");
        vk::CmdDispatchBaseKHR(m_command_buffer.handle(), x_count_limit, y_count_limit - 1, z_count_limit - 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatchBase-baseGroupX-00422");
        vk::CmdDispatchBaseKHR(m_command_buffer.handle(), x_count_limit - 1, y_count_limit, z_count_limit - 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatchBase-baseGroupZ-00423");
        vk::CmdDispatchBaseKHR(m_command_buffer.handle(), x_count_limit - 1, y_count_limit - 1, z_count_limit, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        // (Base + count) exceeds limit
        uint32_t x_base = x_count_limit / 2;
        uint32_t y_base = y_count_limit / 2;
        uint32_t z_base = z_count_limit / 2;
        x_count_limit -= x_base;
        y_count_limit -= y_base;
        z_count_limit -= z_base;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatchBase-groupCountX-00424");
        vk::CmdDispatchBaseKHR(m_command_buffer.handle(), x_base, y_base, z_base, x_count_limit + 1, y_count_limit, z_count_limit);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatchBase-groupCountY-00425");
        vk::CmdDispatchBaseKHR(m_command_buffer.handle(), x_base, y_base, z_base, x_count_limit, y_count_limit + 1, z_count_limit);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatchBase-groupCountZ-00426");
        vk::CmdDispatchBaseKHR(m_command_buffer.handle(), x_base, y_base, z_base, x_count_limit, y_count_limit, z_count_limit + 1);
        m_errorMonitor->VerifyFound();
    } else {
        printf("KHR_DEVICE_GROUP_* extensions not supported, skipping CmdDispatchBaseKHR() tests.\n");
    }
}

TEST_F(NegativeShaderCompute, DispatchBaseFlag) {
    TEST_DESCRIPTION("Compute dispatch without VK_PIPELINE_CREATE_DISPATCH_BASE");

    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    CreateComputePipelineHelper pipe(*this);
    pipe.CreateComputePipeline();

    // Bind pipeline to command buffer
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatchBase-baseGroupX-00427");
    vk::CmdDispatchBaseKHR(m_command_buffer.handle(), 1, 1, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}
