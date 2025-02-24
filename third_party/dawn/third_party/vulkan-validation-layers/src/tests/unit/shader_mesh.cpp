/*
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class NegativeShaderMesh : public VkLayerTest {};

TEST_F(NegativeShaderMesh, SharedMemoryOverLimit) {
    TEST_DESCRIPTION("Validate mesh shader shared memory does not exceed maxMeshSharedMemorySize");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxMeshSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream mesh_source;
    mesh_source << R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        shared int a[)glsl";
    mesh_source << (max_shared_ints + 16);
    mesh_source << R"glsl(];
        void main(){}
    )glsl";

    VkShaderObj mesh(this, mesh_source.str().c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.fs_->GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maxMeshSharedMemorySize-08754");
}

TEST_F(NegativeShaderMesh, SharedMemoryOverLimitWorkgroupMemoryExplicitLayout) {
    TEST_DESCRIPTION(
        "Validate mesh shader shared memory does not exceed maxMeshSharedMemorySize when using "
        "VK_KHR_workgroup_memory_explicit_layout");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::workgroupMemoryExplicitLayout);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxMeshSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream mesh_source;
    mesh_source << R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        #extension GL_EXT_shared_memory_block : enable
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;

        shared X {
            int x;
        };

        shared Y {
            int y1[)glsl";
    mesh_source << (max_shared_ints + 16);
    mesh_source << R"glsl(];
            int y2;
        };

        void main() {
            x = 0; // prevent dead-code elimination
            y2 = 0;
        }
    )glsl";

    VkShaderObj mesh(this, mesh_source.str().c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.fs_->GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maxMeshSharedMemorySize-08754");
}

TEST_F(NegativeShaderMesh, SharedMemorySpecConstantDefault) {
    TEST_DESCRIPTION("Validate shared memory exceed maxMeshSharedMemorySize limit with spec constants default");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxMeshSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream mesh_source;
    mesh_source << R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        layout(constant_id = 0) const uint Condition = 1;
        layout(constant_id = 1) const uint SharedSize = )glsl";
    mesh_source << (max_shared_ints + 16);
    mesh_source << R"glsl(;

        #define enableSharedMemoryOpt (Condition == 1)
        shared uint arr[enableSharedMemoryOpt ? SharedSize : 1];
        void main(){}
    )glsl";

    VkShaderObj mesh(this, mesh_source.str().c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.fs_->GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maxMeshSharedMemorySize-08754");
}

TEST_F(NegativeShaderMesh, SharedMemorySpecConstantSet) {
    TEST_DESCRIPTION("Validate shared memory exceed maxMeshSharedMemorySize limit with spec constants set");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxMeshSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream mesh_source;
    mesh_source << R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        layout(constant_id = 0) const uint Condition = 1;
        layout(constant_id = 1) const uint SharedSize = )glsl";
    mesh_source << (max_shared_ints + 16);
    mesh_source << R"glsl(;

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

    VkShaderObj mesh(this, mesh_source.str().c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_GLSL,
                     &specialization_info);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.fs_->GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maxMeshSharedMemorySize-08754");
}

TEST_F(NegativeShaderMesh, TaskSharedMemoryOverLimit) {
    TEST_DESCRIPTION("Validate Task shader shared memory does not exceed maxTaskSharedMemorySize");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxTaskSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream task_source;
    task_source << R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        shared int a[)glsl";
    task_source << (max_shared_ints + 16);
    task_source << R"glsl(];
        void main(){}
    )glsl";

    VkShaderObj task(this, task_source.str().c_str(), VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj mesh(this, kMeshMinimalGlsl, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task.GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maxTaskSharedMemorySize-08759");
}

TEST_F(NegativeShaderMesh, MeshAndTaskShaderDerivatives) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::computeDerivativeGroupQuads);
    AddRequiredFeature(vkt::Feature::meshShader);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR derivatives_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(derivatives_properties);
    if (derivatives_properties.meshAndTaskShaderDerivatives) {
        GTEST_SKIP() << "meshAndTaskShaderDerivatives is supported";
    }

    const char *ms_source = R"(
               OpCapability ComputeDerivativeGroupQuadsKHR
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpExtension "SPV_KHR_compute_shader_derivatives"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %gl_Position %3
               OpExecutionMode %main LocalSize 4 4 1
               OpExecutionMode %main DerivativeGroupQuadsKHR
               OpExecutionMode %main OutputTrianglesEXT
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpSource HLSL 660
               OpName %main "main"
               OpDecorate %gl_Position BuiltIn Position
               OpDecorate %3 BuiltIn PrimitiveTriangleIndicesEXT
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_arr_v4float_uint_3 = OpTypeArray %v4float %uint_3
%_ptr_Output__arr_v4float_uint_3 = OpTypePointer Output %_arr_v4float_uint_3
     %uint_1 = OpConstant %uint 1
%_arr_v3uint_uint_1 = OpTypeArray %v3uint %uint_1
%_ptr_Output__arr_v3uint_uint_1 = OpTypePointer Output %_arr_v3uint_uint_1
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
%gl_Position = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
          %3 = OpVariable %_ptr_Output__arr_v3uint_uint_1 Output
       %main = OpFunction %void None %15
         %16 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-meshAndTaskShaderDerivatives-10153");
    VkShaderObj ms(this, ms_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}