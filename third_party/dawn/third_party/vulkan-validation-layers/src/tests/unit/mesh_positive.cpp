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

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class PositiveMesh : public VkLayerTest {};

TEST_F(PositiveMesh, BasicUsage) {
    TEST_DESCRIPTION("Test basic VK_EXT_mesh_shader.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);

    RETURN_IF_SKIP(Init());

    InitRenderTarget();

    const char *mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        void main() {
            SetMeshOutputsEXT(3,1);
        }
    )glsl";

    VkShaderObj ms(this, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    // Ensure pVertexInputState and pInputAssembly state are null, as these should be ignored.
    pipe.gp_ci_.pVertexInputState = nullptr;
    pipe.gp_ci_.pInputAssemblyState = nullptr;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDrawMeshTasksEXT(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveMesh, MeshShaderOnly) {
    TEST_DESCRIPTION("Test using a mesh shader without a vertex shader.");

    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    // Create a device that enables mesh_shader
    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(mesh_shader_features);
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    if (mesh_shader_features.meshShader != VK_TRUE) {
        GTEST_SKIP() << "Mesh shader feature not supported";
    }

    InitRenderTarget();

    static const char meshShaderText[] = R"glsl(
        #version 450
        #extension GL_NV_mesh_shader : require
        layout(local_size_x = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;
        void main() {
              gl_MeshVerticesNV[0].gl_Position = vec4(-1.0, -1.0, 0, 1);
              gl_MeshVerticesNV[1].gl_Position = vec4( 1.0, -1.0, 0, 1);
              gl_MeshVerticesNV[2].gl_Position = vec4( 0.0,  1.0, 0, 1);
              gl_PrimitiveIndicesNV[0] = 0;
              gl_PrimitiveIndicesNV[1] = 1;
              gl_PrimitiveIndicesNV[2] = 2;
              gl_PrimitiveCountNV = 1;
        }
    )glsl";

    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_NV);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper helper(*this);
    helper.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};

    // Ensure pVertexInputState and pInputAssembly state are null, as these should be ignored.
    helper.gp_ci_.pVertexInputState = nullptr;
    helper.gp_ci_.pInputAssemblyState = nullptr;
    helper.CreateGraphicsPipeline();
}

TEST_F(PositiveMesh, PointSize) {
    TEST_DESCRIPTION("Test writing point size in a mesh shader.");

    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    // Create a device that enables mesh_shader
    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(mesh_shader_features);
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    if (mesh_shader_features.meshShader != VK_TRUE) {
        GTEST_SKIP() << "Mesh shader feature not supported";
    }

    InitRenderTarget();

    static const char meshShaderText[] = R"glsl(
        #version 460
        #extension GL_NV_mesh_shader : enable
        layout (local_size_x=1) in;
        layout (points) out;
        layout (max_vertices=1, max_primitives=1) out;
        void main ()
        {
            gl_PrimitiveCountNV = 1u;
            gl_PrimitiveIndicesNV[0] = 0;
            gl_MeshVerticesNV[0].gl_Position = vec4(-0.5, -0.5, 0.0, 1.0);
            gl_MeshVerticesNV[0].gl_PointSize = 4;
        }
    )glsl";

    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_NV);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper helper(*this);
    helper.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};

    // Ensure pVertexInputState and pInputAssembly state are null, as these should be ignored.
    helper.gp_ci_.pVertexInputState = nullptr;
    helper.gp_ci_.pInputAssemblyState = nullptr;
    helper.CreateGraphicsPipeline();
}

TEST_F(PositiveMesh, TaskAndMeshShaderNV) {
    TEST_DESCRIPTION("Test task and mesh shader");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.meshShader || !mesh_shader_features.taskShader) {
        GTEST_SKIP() << "Test requires (unsupported) meshShader and taskShader features, skipping test.";
    }
    RETURN_IF_SKIP(InitState(nullptr, &mesh_shader_features));
    InitRenderTarget();

    VkPhysicalDeviceVulkan11Properties vulkan11_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(vulkan11_props);

    if ((vulkan11_props.subgroupSupportedStages & VK_SHADER_STAGE_TASK_BIT_NV) == 0) {
        GTEST_SKIP() << "%s VkPhysicalDeviceVulkan11Properties::subgroupSupportedStages does not include "
                        "VK_SHADER_STAGE_TASK_BIT_NV, skipping test.";
    }

    static const char taskShaderText[] = R"glsl(
        #version 450

        #extension GL_NV_mesh_shader : require
        #extension GL_KHR_shader_subgroup_ballot : require

        #define GROUP_SIZE 32

        layout(local_size_x = 32) in;

        taskNV out Task {
          uint baseID;
          uint subIDs[GROUP_SIZE];
        } OUT;

        void main() {
            uvec4 desc = uvec4(gl_GlobalInvocationID.x);

            // implement some early culling function
            bool render = gl_GlobalInvocationID.x < 32;

            uvec4 vote  = subgroupBallot(render);
            uint tasks = subgroupBallotBitCount(vote);

            if (gl_LocalInvocationID.x == 0) {
                // write the number of surviving meshlets, i.e.
                // mesh workgroups to spawn
                gl_TaskCountNV = tasks;

                // where the meshletIDs started from for this task workgroup
                OUT.baseID = gl_WorkGroupID.x * GROUP_SIZE;
            }
        }
    )glsl";

    static const char meshShaderText[] = R"glsl(
        #version 450

        #extension GL_NV_mesh_shader : require

        layout(local_size_x = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;

        taskNV in Task {
          uint baseID;
          uint subIDs[32];
        } IN;

        void main() {
            uint meshletID = IN.baseID + IN.subIDs[gl_WorkGroupID.x];
            uvec4 desc = uvec4(meshletID);
        }
    )glsl";

    VkShaderObj ts(this, taskShaderText, VK_SHADER_STAGE_TASK_BIT_NV, SPV_ENV_VULKAN_1_2);
    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_NV, SPV_ENV_VULKAN_1_2);

    const auto break_vp = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {ts.GetStageCreateInfo(), ms.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit);
}

TEST_F(PositiveMesh, MeshPerTaskNV) {
    TEST_DESCRIPTION("Make sure PerTaskNV in handled");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.meshShader || !mesh_shader_features.taskShader) {
        GTEST_SKIP() << "Test requires (unsupported) meshShader and taskShader features, skipping test.";
    }
    RETURN_IF_SKIP(InitState(nullptr, &mesh_shader_features));
    InitRenderTarget();

    static const char taskShaderText[] = R"glsl(
        #version 450
        #extension GL_NV_mesh_shader : require
        layout(local_size_x = 32) in;
        taskNV out Task {
            uint baseID;
        } OUT;
        void main() {}
    )glsl";

    static const char meshShaderText[] = R"glsl(
        #version 460
        #extension GL_NV_mesh_shader : enable

        layout(local_size_x=2) in;
        layout(triangles) out;
        layout(max_vertices=4, max_primitives=2) out;

        struct NestStruct {
            uint z;
        };
        struct FirstStruct {
            float a;
            NestStruct b;
        };

        in taskNV TaskData {
            uint x;
            FirstStruct y;
        } td;

        void main () {}
    )glsl";

    VkShaderObj ts(this, taskShaderText, VK_SHADER_STAGE_TASK_BIT_NV, SPV_ENV_VULKAN_1_2);
    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_NV, SPV_ENV_VULKAN_1_2);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {ts.GetStageCreateInfo(), ms.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveMesh, PrimitiveTopology) {
    TEST_DESCRIPTION("pInputAssemblyState is ignored when pipeline includes a mesh shading stage");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PRIMITIVE_TOPOLOGY_LIST_RESTART_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkShaderObj ms(this, kMeshMinimalGlsl, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;  // requires primitiveTopologyListRestart
    pipe.ia_ci_.primitiveRestartEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveMesh, DrawIndexMesh) {
    TEST_DESCRIPTION("use DrawIndex only with Mesh Shader.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::shaderDrawParameters);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        layout(max_vertices = 32, max_primitives = 32, triangles) out;
        void main() {
            uint compacted_meshlet_index = uint(32768 * gl_DrawID) + gl_WorkGroupID.x;
            SetMeshOutputsEXT(3,1);
        }
    )glsl";

    VkShaderObj ms(this, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveMesh, DrawIndexTask) {
    TEST_DESCRIPTION("use DrawIndex only with Task Shader.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);
    AddRequiredFeature(vkt::Feature::shaderDrawParameters);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *task_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        taskPayloadSharedEXT uint mesh_payload[32];
        void main() {
            mesh_payload[gl_LocalInvocationIndex] =  uint(32768 * gl_DrawID) + gl_GlobalInvocationID.x;
            EmitMeshTasksEXT(32u, 1u, 1u);
        }
    )glsl";

    const char *mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        layout(max_vertices = 32, max_primitives = 32, triangles) out;
        taskPayloadSharedEXT uint mesh_payload[32];
        void main() {
            SetMeshOutputsEXT(3,1);
        }
    )glsl";

    VkShaderObj ts(this, task_source, VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj ms(this, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ts.GetStageCreateInfo(), ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveMesh, MeshAndTaskShaderDerivatives) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::computeDerivativeGroupQuads);
    AddRequiredFeature(vkt::Feature::meshShader);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR derivatives_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(derivatives_properties);
    if (!derivatives_properties.meshAndTaskShaderDerivatives) {
        GTEST_SKIP() << "meshAndTaskShaderDerivatives is not supported";
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
    VkShaderObj ms(this, ms_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
}

TEST_F(PositiveMesh, TessellationDynamicState) {
    TEST_DESCRIPTION("Test basic VK_EXT_mesh_shader.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3TessellationDomainOrigin);
    RETURN_IF_SKIP(Init());

    InitRenderTarget();

    const char *mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        void main() {
            SetMeshOutputsEXT(3,1);
        }
    )glsl";

    VkShaderObj ms(this, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.AddDynamicState(VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDrawMeshTasksEXT(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}
