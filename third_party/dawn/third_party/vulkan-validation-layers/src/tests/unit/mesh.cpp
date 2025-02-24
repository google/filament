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

class NegativeMesh : public VkLayerTest {};

TEST_F(NegativeMesh, BasicUsage) {
    TEST_DESCRIPTION("Test VK_EXT_mesh_shader.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexInputDynamicState);
    AddRequiredFeature(vkt::Feature::extendedDynamicState2);
    AddRequiredFeature(vkt::Feature::extendedDynamicState2PatchControlPoints);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::multiview);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    AddRequiredFeature(vkt::Feature::taskShader);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::dynamicRendering);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    static const char vert_shader_text[] = R"glsl(
        #version 460
        vec2 vertices[3];
        void main() {
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           gl_PointSize = 1.0f;
        }
    )glsl";

    static const char mesh_shader_text[] = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;
        void main() {
            SetMeshOutputsEXT(3,1);
            gl_MeshVerticesEXT[0].gl_Position = vec4(-1.0, -1.0, 0, 1);
            gl_MeshVerticesEXT[1].gl_Position = vec4( 1.0, -1.0, 0, 1);
            gl_MeshVerticesEXT[2].gl_Position = vec4( 0.0,  1.0, 0, 1);
            gl_PrimitiveTriangleIndicesEXT[0] = uvec3(0,1,2);
        }
    )glsl";

    static const char mesh_shader_xfb_text[] = R"(
               OpCapability MeshShadingEXT
               OpCapability TransformFeedback
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %tf
               OpExecutionModeId %main LocalSizeId %uint_2 %uint_1 %uint_1
               OpExecutionMode %main Xfb
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_mesh_shader"
               OpName %main "main"
               OpName %tf "tf"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
         %tf = OpVariable %_ptr_Output_float Output
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
         %10 = OpConstantComposite %v3uint %uint_2 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj vs(this, vert_shader_text, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj ms(this, mesh_shader_text, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3);
    VkShaderObj ms_xfb(this, mesh_shader_xfb_text, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Test pipeline creation
    {
        // can't mix mesh with vertex
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo(), ms.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit,
                                          std::vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-pStages-02095"}));

        // vertex or mesh must be present
        // 02096 overlaps with 06896
        const auto break_vp2 = [&](CreatePipelineHelper &helper) { helper.shader_stages_ = {fs.GetStageCreateInfo()}; };
        CreatePipelineHelper::OneshotTest(*this, break_vp2, kErrorBit,
                                          std::vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-stage-02096",
                                                                    "VUID-VkGraphicsPipelineCreateInfo-pStages-06896"}));

        // vertexinput and inputassembly must be valid when vertex stage is present
        const auto break_vp3 = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            helper.gp_ci_.pVertexInputState = nullptr;
            helper.gp_ci_.pInputAssemblyState = nullptr;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp3, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-02097");

        // xfb with mesh shader
        const auto break_vp4 = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {ms_xfb.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp4, kErrorBit,
                                          std::vector<std::string>({" VUID-VkGraphicsPipelineCreateInfo-None-02322"}));

        // invalid dynamic state with mesh shader
        std::vector<VkDynamicState> dyn_states[] = {
            {VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY},       {VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE},
            {VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE}, {VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT},
            {VK_DYNAMIC_STATE_VERTEX_INPUT_EXT},
        };
        const char *err_vuids[] = {
            "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07065", "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07065",
            "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07066", "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07066",
            "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07067"};
        VkPipelineDynamicStateCreateInfo dyn_state = vku::InitStructHelper();
        for (int i = 0; i < 5; i++) {
            dyn_state.dynamicStateCount = dyn_states[i].size();
            dyn_state.pDynamicStates = dyn_states[i].data();
            if (*dyn_state.pDynamicStates == VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT) continue;
            const auto break_vp5 = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
                helper.gp_ci_.pDynamicState = &dyn_state;
            };
            CreatePipelineHelper::OneshotTest(*this, break_vp5, kErrorBit, std::vector<std::string>({err_vuids[i]}));
        }

        // viewMask without enabling multiviewMeshShader feature
        VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
        pipeline_rendering_info.viewMask = 0x2;
        VkFormat color_formats[] = {VK_FORMAT_UNDEFINED};
        pipeline_rendering_info.colorAttachmentCount = 1;
        pipeline_rendering_info.pColorAttachmentFormats = color_formats;

        const auto break_vp5 = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            helper.gp_ci_.pNext = &pipeline_rendering_info;
            helper.gp_ci_.renderPass = VK_NULL_HANDLE;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp5, kErrorBit,
                                          std::vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-renderPass-07720"}));
    }
}

TEST_F(NegativeMesh, ExtensionDisabled) {
    TEST_DESCRIPTION("Test VK_EXT_mesh_shader VUs with EXT_mesh_shader disabled.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance4);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    // #version 450
    // #extension GL_EXT_mesh_shader : require
    // layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
    // struct Task {
    //     uint baseID;
    // };
    // taskPayloadSharedEXT Task OUT;
    // void main() {
    //     OUT.baseID = 1;
    // }
    const char *task_src = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TaskEXT %main "main" %OUT
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
               OpMemberName %Task 0 "baseID"
               OpName %OUT "OUT"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
       %Task = OpTypeStruct %uint
%_ptr_TaskPayloadWorkgroupEXT_Task = OpTypePointer TaskPayloadWorkgroupEXT %Task
        %OUT = OpVariable %_ptr_TaskPayloadWorkgroupEXT_Task TaskPayloadWorkgroupEXT
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_TaskPayloadWorkgroupEXT_uint = OpTypePointer TaskPayloadWorkgroupEXT %uint
     %v3uint = OpTypeVector %uint 3
         %16 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpAccessChain %_ptr_TaskPayloadWorkgroupEXT_uint %OUT %int_0
               OpStore %14 %uint_1
               OpReturn
               OpFunctionEnd
    )";

    // #version 450
    // #extension GL_EXT_mesh_shader : enable
    // layout (triangles) out;
    // layout (max_vertices = 3, max_primitives = 1) out;
    // struct Task {
    //     uint baseID;
    // };
    // taskPayloadSharedEXT Task IN;
    // void main() {}
    const char *mesh_src = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %IN
               OpExecutionMode %main LocalSize 1 1 1
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_mesh_shader"
               OpName %main "main"
               OpName %Task "Task"
               OpMemberName %Task 0 "baseID"
               OpName %IN "IN"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %Task = OpTypeStruct %uint
%_ptr_TaskPayloadWorkgroupEXT_Task = OpTypePointer TaskPayloadWorkgroupEXT %Task
         %IN = OpVariable %_ptr_TaskPayloadWorkgroupEXT_Task TaskPayloadWorkgroupEXT
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj task_shader(this, task_src, VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    VkShaderObj mesh_shader(this, mesh_src, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    // mesh and task shaders not supported
    const auto break_vp = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task_shader.GetStageCreateInfo(), mesh_shader.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit,
                                      std::vector<std::string>({"VUID-VkPipelineShaderStageCreateInfo-stage-02091",
                                                                "VUID-VkPipelineShaderStageCreateInfo-stage-02092"}));
}

TEST_F(NegativeMesh, RuntimeSpirv) {
    TEST_DESCRIPTION("Test VK_EXT_mesh_shader spirv related VUIDs.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    std::vector<std::string> error_vuids;
    uint32_t max_task_workgroup_size_x = mesh_shader_properties.maxTaskWorkGroupSize[0];
    uint32_t max_task_workgroup_size_y = mesh_shader_properties.maxTaskWorkGroupSize[1];
    uint32_t max_task_workgroup_size_z = mesh_shader_properties.maxTaskWorkGroupSize[2];

    uint32_t max_mesh_workgroup_size_x = mesh_shader_properties.maxMeshWorkGroupSize[0];
    uint32_t max_mesh_workgroup_size_y = mesh_shader_properties.maxMeshWorkGroupSize[1];
    uint32_t max_mesh_workgroup_size_z = mesh_shader_properties.maxMeshWorkGroupSize[2];

    if (max_task_workgroup_size_x < vvl::MaxTypeValue(max_task_workgroup_size_x)) {
        error_vuids.emplace_back("VUID-RuntimeSpirv-TaskEXT-07291");
        max_task_workgroup_size_x += 1;
    }

    if (max_task_workgroup_size_y < vvl::MaxTypeValue(max_task_workgroup_size_y)) {
        error_vuids.emplace_back("VUID-RuntimeSpirv-TaskEXT-07292");
        max_task_workgroup_size_y += 1;
    }

    if (max_task_workgroup_size_z < vvl::MaxTypeValue(max_task_workgroup_size_z)) {
        error_vuids.emplace_back("VUID-RuntimeSpirv-TaskEXT-07293");
        max_task_workgroup_size_z += 1;
    }
    error_vuids.emplace_back("VUID-RuntimeSpirv-TaskEXT-07294");

    if (max_mesh_workgroup_size_x < vvl::MaxTypeValue(max_mesh_workgroup_size_x)) {
        error_vuids.emplace_back("VUID-RuntimeSpirv-MeshEXT-07295");
        max_mesh_workgroup_size_x += 1;
    }

    if (max_mesh_workgroup_size_y < vvl::MaxTypeValue(max_mesh_workgroup_size_y)) {
        error_vuids.emplace_back("VUID-RuntimeSpirv-MeshEXT-07296");
        max_mesh_workgroup_size_y += 1;
    }

    if (max_mesh_workgroup_size_z < vvl::MaxTypeValue(max_mesh_workgroup_size_z)) {
        error_vuids.emplace_back("VUID-RuntimeSpirv-MeshEXT-07297");
        max_mesh_workgroup_size_z += 1;
    }
    error_vuids.emplace_back("VUID-RuntimeSpirv-MeshEXT-07298");

    std::string task_src = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TaskEXT %main "main" %OUT
               OpExecutionMode %main LocalSize )";
    task_src += std::to_string(max_task_workgroup_size_x);
    task_src += " ";
    task_src += std::to_string(max_task_workgroup_size_y);
    task_src += " ";
    task_src += std::to_string(max_task_workgroup_size_z);
    task_src += R"(
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
          %9 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %Task = OpTypeStruct %uint
%_ptr_TaskPayloadWorkgroupEXT_Task = OpTypePointer TaskPayloadWorkgroupEXT %Task
        %OUT = OpVariable %_ptr_TaskPayloadWorkgroupEXT_Task TaskPayloadWorkgroupEXT
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    std::string mesh_src = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main"
               OpExecutionMode %main LocalSize )";
    mesh_src += std::to_string(max_mesh_workgroup_size_x);
    mesh_src += " ";
    mesh_src += std::to_string(max_mesh_workgroup_size_y);
    mesh_src += " ";
    mesh_src += std::to_string(max_mesh_workgroup_size_z);
    mesh_src += R"(
               OpExecutionMode %main OutputVertices 1
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
          %9 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-MeshEXT-07115");
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-MeshEXT-07116");
    VkShaderObj task_shader(this, task_src.c_str(), VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    VkShaderObj mesh_shader(this, mesh_src.c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    // mesh and task shaders which exceeds workgroup size limits
    const auto break_vp = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task_shader.GetStageCreateInfo(), mesh_shader.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, error_vuids);
}

TEST_F(NegativeMesh, RuntimeSpirv2) {
    TEST_DESCRIPTION("Test VK_EXT_mesh_shader spirv related VUIDs.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::meshShader);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    uint32_t max_mesh_output_vertices = mesh_shader_properties.maxMeshOutputVertices;
    uint32_t max_mesh_output_primitives = mesh_shader_properties.maxMeshOutputPrimitives;

    bool skip = true;
    if (max_mesh_output_vertices < vvl::MaxTypeValue(max_mesh_output_vertices)) {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-MeshEXT-07115");
        skip = false;
        max_mesh_output_vertices += 1;
    }

    if (max_mesh_output_primitives < vvl::MaxTypeValue(max_mesh_output_primitives)) {
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-MeshEXT-07116");
        skip = false;
        max_mesh_output_primitives += 1;
    }

    if (skip) {
        GTEST_SKIP() << "No properties are invalid to check";
    }

    std::string mesh_src = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main"
               OpExecutionModeId %main LocalSizeId %uint_2 %uint_1 %uint_1
               OpExecutionMode %main OutputVertices )";
    mesh_src += std::to_string(max_mesh_output_vertices);
    mesh_src += R"(
               OpExecutionMode %main OutputPrimitivesEXT )";
    mesh_src += std::to_string(max_mesh_output_primitives);
    mesh_src += R"(
               OpExecutionMode %main OutputTrianglesEXT
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_mesh_shader"
               OpName %main "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
         %10 = OpConstantComposite %v3uint %uint_2 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj mesh_shader(this, mesh_src.c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMesh, RuntimeSpirvNV) {
    TEST_DESCRIPTION("Test VK_NV_mesh_shader spirv related VUIDs");

    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (mesh_shader_features.meshShader != VK_TRUE) {
        GTEST_SKIP() << "Mesh shader feature not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesNV mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    std::string mesh_src = R"(
               OpCapability MeshShadingNV
               OpExtension "SPV_NV_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshNV %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpExecutionMode %main OutputVertices )";
    mesh_src += std::to_string(mesh_shader_properties.maxMeshOutputVertices + 1);
    mesh_src += R"(
               OpExecutionMode %main OutputPrimitivesNV )";
    mesh_src += std::to_string(mesh_shader_properties.maxMeshOutputPrimitives + 1);
    mesh_src += R"(
               OpExecutionMode %main OutputTrianglesNV
               OpSource GLSL 450
               OpSourceExtension "GL_NV_mesh_shader"
               OpName %main "main"
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-MeshNV-07113");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-MeshNV-07114");
    VkShaderObj::CreateFromASM(this, mesh_src.c_str(), VK_SHADER_STAGE_MESH_BIT_NV, SPV_ENV_VULKAN_1_0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMesh, BasicUsageNV) {
    TEST_DESCRIPTION("Test VK_NV_mesh_shader.");

    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    // Create a device that enables mesh_shader
    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(mesh_shader_features);
    features2.features.multiDrawIndirect = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    static const char vertShaderText[] = R"glsl(
        #version 450
        vec2 vertices[3];
        void main() {
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           gl_PointSize = 1.0f;
        }
    )glsl";

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

    VkShaderObj vs(this, vertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_NV);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Test pipeline creation
    {
        // can't mix mesh with vertex
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo(), ms.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit,
                                          std::vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-pStages-02095"}));

        // vertex or mesh must be present
        // 02096 overlaps with 06896
        const auto break_vp2 = [&](CreatePipelineHelper &helper) { helper.shader_stages_ = {fs.GetStageCreateInfo()}; };
        CreatePipelineHelper::OneshotTest(*this, break_vp2, kErrorBit,
                                          std::vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-stage-02096",
                                                                    "VUID-VkGraphicsPipelineCreateInfo-pStages-06896"}));

        // vertexinput and inputassembly must be valid when vertex stage is present
        const auto break_vp3 = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            helper.gp_ci_.pVertexInputState = nullptr;
            helper.gp_ci_.pInputAssemblyState = nullptr;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp3, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-02097");
    }

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectNV-None-08606");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectNV-buffer-02708");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02157");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02146");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02718");
    vk::CmdDrawMeshTasksIndirectNV(m_command_buffer.handle(), buffer, 0, 2, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeMesh, ExtensionDisabledNV) {
    TEST_DESCRIPTION("Test VK_NV_mesh_shader VUs with NV_mesh_shader disabled.");

    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (mesh_shader_features.meshShader != VK_TRUE) {
        GTEST_SKIP() << "Mesh shader feature not supported";
    }

    mesh_shader_features.meshShader = VK_FALSE;
    mesh_shader_features.taskShader = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &mesh_shader_features));
    InitRenderTarget();

    vkt::Event event_obj(*m_device);
    const auto event = event_obj.handle();
    ASSERT_TRUE(event_obj.initialized());

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent-stageMask-04095");
    vk::CmdSetEvent(m_command_buffer.handle(), event, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent-stageMask-04096");
    vk::CmdSetEvent(m_command_buffer.handle(), event, VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdResetEvent-stageMask-04095");
    vk::CmdResetEvent(m_command_buffer.handle(), event, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdResetEvent-stageMask-04096");
    vk::CmdResetEvent(m_command_buffer.handle(), event, VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-04095");
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-dstStageMask-04095");
    vk::CmdWaitEvents(m_command_buffer.handle(), 1, &event, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV,
                      VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV, 0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-04096");
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-dstStageMask-04096");
    vk::CmdWaitEvents(m_command_buffer.handle(), 1, &event, VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV,
                      VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV, 0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcStageMask-04095");
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-dstStageMask-04095");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV, 0,
                           0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcStageMask-04096");
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-dstStageMask-04096");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV, VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV, 0,
                           0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();

    vkt::Semaphore semaphore_obj(*m_device);
    const auto semaphore = semaphore_obj.handle();
    ASSERT_TRUE(semaphore_obj.initialized());

    VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV | VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV;
    VkSubmitInfo submit_info = vku::InitStructHelper();

    // Signal the semaphore so the next test can wait on it.
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore;
    submit_info.pWaitDstStageMask = &stage_flags;

    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pWaitDstStageMask-04095");
    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pWaitDstStageMask-04096");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    // #version 450
    // #extension GL_NV_mesh_shader : require
    // layout(local_size_x = 1) in;
    // taskNV out Task {
    //     uint baseID;
    // } OUT;
    // void main() {
    //     OUT.baseID = 1;
    // }
    const char *task_src = R"(
               OpCapability MeshShadingNV
               OpExtension "SPV_NV_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TaskNV %main "main" %OUT
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %Task 0 PerTaskNV
               OpMemberDecorate %Task 0 Offset 0
               OpDecorate %Task Block
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %Task = OpTypeStruct %uint
%_ptr_Output_Task = OpTypePointer Output %Task
        %OUT = OpVariable %_ptr_Output_Task Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %uint_1 = OpConstant %uint 1
%_ptr_Output_uint = OpTypePointer Output %uint
     %v3uint = OpTypeVector %uint 3
    %uint_32 = OpConstant %uint 32
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_32 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpAccessChain %_ptr_Output_uint %OUT %int_0
               OpStore %14 %uint_1
               OpReturn
               OpFunctionEnd
    )";

    static const char mesh_src[] = R"glsl(
        #version 450

        #extension GL_NV_mesh_shader : require

        layout(local_size_x = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;

        taskNV in Task {
          uint baseID;
        } IN;

        void main() {
        }
    )glsl";

    VkShaderObj task_shader(this, task_src, VK_SHADER_STAGE_TASK_BIT_NV, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj mesh_shader(this, mesh_src, VK_SHADER_STAGE_MESH_BIT_NV);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    // mesh and task shaders not supported
    const auto break_vp = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task_shader.GetStageCreateInfo(), mesh_shader.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit,
                                      std::vector<std::string>({"VUID-VkPipelineShaderStageCreateInfo-stage-02091",
                                                                "VUID-VkPipelineShaderStageCreateInfo-stage-02092"}));
}

TEST_F(NegativeMesh, DrawCmds) {
    TEST_DESCRIPTION("Test VK_EXT_mesh_shader draw commands.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::meshShader);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    // #version 450
    // #extension GL_EXT_mesh_shader : enable
    // layout (triangles) out;
    // layout (max_vertices = 3, max_primitives = 1) out;
    // struct Task {
    //     uint baseID;
    // };
    // taskPayloadSharedEXT Task IN;
    // void main() {}
    const char *mesh_src = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %IN
               OpExecutionMode %main LocalSize 1 1 1
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_mesh_shader"
               OpName %main "main"
               OpName %Task "Task"
               OpMemberName %Task 0 "baseID"
               OpName %IN "IN"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %Task = OpTypeStruct %uint
%_ptr_TaskPayloadWorkgroupEXT_Task = OpTypePointer TaskPayloadWorkgroupEXT %Task
         %IN = OpVariable %_ptr_TaskPayloadWorkgroupEXT_Task TaskPayloadWorkgroupEXT
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj mesh_shader(this, mesh_src, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);

    vkt::Buffer buffer(*m_device, 2 * sizeof(VkDrawMeshTasksIndirectCommandEXT), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
    vkt::Buffer count_buffer(*m_device, 64, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = mesh_shader.GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    CreatePipelineHelper pipe1(*this);
    pipe1.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    uint32_t max_group_count_X = mesh_shader_properties.maxTaskWorkGroupCount[0];
    uint32_t max_group_count_Y = mesh_shader_properties.maxTaskWorkGroupCount[1];
    uint32_t max_group_count_Z = mesh_shader_properties.maxTaskWorkGroupCount[2];

    if (max_group_count_X < vvl::MaxTypeValue(max_group_count_X)) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07322");
        max_group_count_X = vvl::MaxTypeValue(max_group_count_X);
    }

    if (max_group_count_Y < vvl::MaxTypeValue(max_group_count_Y)) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07323");
        max_group_count_Y = vvl::MaxTypeValue(max_group_count_Y);
    }

    if (max_group_count_Z < vvl::MaxTypeValue(max_group_count_Z)) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07324");
        max_group_count_Z = vvl::MaxTypeValue(max_group_count_Z);
    }

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07325");
    vk::CmdDrawMeshTasksEXT(m_command_buffer.handle(), max_group_count_X, max_group_count_Y, max_group_count_Z);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-02718");
    vk::CmdDrawMeshTasksIndirectEXT(m_command_buffer.handle(), buffer.handle(), 0, 2, sizeof(VkDrawMeshTasksIndirectCommandEXT));
    m_errorMonitor->VerifyFound();

    if (m_device->Physical().limits_.maxDrawIndirectCount < vvl::MaxTypeValue(m_device->Physical().limits_.maxDrawIndirectCount)) {
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-02718");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07090");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-02719");
        vk::CmdDrawMeshTasksIndirectEXT(m_command_buffer.handle(), buffer.handle(), 0,
                                        m_device->Physical().limits_.maxDrawIndirectCount + 1,
                                        sizeof(VkDrawMeshTasksIndirectCommandEXT));
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07089");
    vk::CmdDrawMeshTasksIndirectEXT(m_command_buffer.handle(), buffer.handle(), sizeof(VkDrawMeshTasksIndirectCommandEXT) * 2, 1,
                                    sizeof(VkDrawMeshTasksIndirectCommandEXT));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-stage-06481");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.Handle());
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksEXT-stage-06480");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksEXT-MeshEXT-07087");
    vk::CmdDrawMeshTasksEXT(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectEXT-MeshEXT-07091");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectEXT-stage-06480");
    vk::CmdDrawMeshTasksIndirectEXT(m_command_buffer.handle(), buffer.handle(), 0, 1, sizeof(VkDrawMeshTasksIndirectCommandEXT));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectCountEXT-MeshEXT-07100");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectCountEXT-stage-06480");
    vk::CmdDrawMeshTasksIndirectCountEXT(m_command_buffer.handle(), buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                         sizeof(VkDrawMeshTasksIndirectCommandEXT));
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeMesh, MultiDrawIndirect) {
    TEST_DESCRIPTION("Test VK_EXT_mesh_shader indirect draw command.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    // #version 450
    // #extension GL_EXT_mesh_shader : require
    // layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
    // layout(max_vertices = 3, max_primitives = 1) out;
    // layout(triangles) out;
    // struct Task {
    //   uint baseID;
    // };
    // taskPayloadSharedEXT Task IN;
    // void main() {}
    static const char mesh_src[] = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %IN
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesNV 1
               OpExecutionMode %main OutputTrianglesNV
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_mesh_shader"
               OpName %main "main"
               OpName %Task "Task"
               OpMemberName %Task 0 "baseID"
               OpName %IN "IN"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
          %9 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %Task = OpTypeStruct %uint
%_ptr_TaskPayloadWorkgroupEXT_Task = OpTypePointer TaskPayloadWorkgroupEXT %Task
         %IN = OpVariable %_ptr_TaskPayloadWorkgroupEXT_Task TaskPayloadWorkgroupEXT
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj mesh_shader(this, mesh_src, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    buffer_create_info.size = 2 * sizeof(VkDrawMeshTasksIndirectCommandEXT);
    vkt::Buffer buffer(*m_device, buffer_create_info);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = mesh_shader.GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07088");
    vk::CmdDrawMeshTasksIndirectEXT(m_command_buffer.handle(), buffer.handle(), 0, 2,
                                    sizeof(VkDrawMeshTasksIndirectCommandEXT) - 2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07090");
    vk::CmdDrawMeshTasksIndirectEXT(m_command_buffer.handle(), buffer.handle(), 0, 4, sizeof(VkDrawMeshTasksIndirectCommandEXT));
    m_errorMonitor->VerifyFound();

    vkt::Buffer draw_buffer(*m_device, buffer_create_info, vkt::no_mem);
    draw_buffer.AllocateAndBindMemory(*m_device);

    VkBufferCreateInfo count_buffer_create_info = vku::InitStructHelper();
    count_buffer_create_info.size = 64;
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    vkt::Buffer count_buffer(*m_device, count_buffer_create_info);
    vkt::Buffer count_buffer_unbound(*m_device, count_buffer_create_info, vkt::no_mem);

    count_buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vkt::Buffer count_buffer_wrong_usage(*m_device, count_buffer_create_info);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-02714");
    vk::CmdDrawMeshTasksIndirectCountEXT(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer_unbound.handle(), 0, 1,
                                         sizeof(VkDrawMeshTasksIndirectCommandEXT));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-02715");
    vk::CmdDrawMeshTasksIndirectCountEXT(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer_wrong_usage.handle(), 0,
                                         1, sizeof(VkDrawMeshTasksIndirectCommandEXT));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectCountEXT-stride-07096");
    vk::CmdDrawMeshTasksIndirectCountEXT(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                         sizeof(VkDrawMeshTasksIndirectCommandEXT) - 3);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectCountEXT-maxDrawCount-07097");
    vk::CmdDrawMeshTasksIndirectCountEXT(m_command_buffer.handle(), draw_buffer.handle(),
                                         2 * sizeof(VkDrawMeshTasksIndirectCommandEXT), count_buffer.handle(), 0, 4,
                                         sizeof(VkDrawMeshTasksIndirectCommandEXT));
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeMesh, DrawCmdsNV) {
    TEST_DESCRIPTION("Test VK_NV_mesh_shader draw commands.");

    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (mesh_shader_features.meshShader != VK_TRUE) {
        GTEST_SKIP() << "Mesh shader feature not supported";
    }
    features2.features.multiDrawIndirect = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    InitRenderTarget();

    static const char mesh_src[] = R"glsl(
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

    VkShaderObj mesh_shader(this, mesh_src, VK_SHADER_STAGE_MESH_BIT_NV);

    vkt::Buffer buffer(*m_device, 2 * sizeof(VkDrawMeshTasksIndirectCommandNV), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
    vkt::Buffer count_buffer(*m_device, 64, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = mesh_shader.GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    CreatePipelineHelper pipe1(*this);
    pipe1.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02156");
    vk::CmdDrawMeshTasksIndirectNV(m_command_buffer.handle(), buffer.handle(), sizeof(VkDrawMeshTasksIndirectCommandNV) * 2, 1,
                                   sizeof(VkDrawMeshTasksIndirectCommandNV));
    m_errorMonitor->VerifyFound();

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.Handle());
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksNV-MeshNV-07080");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksNV-stage-06480");
    vk::CmdDrawMeshTasksNV(m_command_buffer.handle(), 1, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectNV-MeshNV-07081");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectNV-stage-06480");
    vk::CmdDrawMeshTasksIndirectNV(m_command_buffer.handle(), buffer.handle(), 0, 1, sizeof(VkDrawMeshTasksIndirectCommandNV));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectCountNV-MeshNV-07082");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectCountNV-stage-06480");
    vk::CmdDrawMeshTasksIndirectCountNV(m_command_buffer.handle(), buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                        sizeof(VkDrawMeshTasksIndirectCommandNV));
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeMesh, MeshTasksWorkgroupCount) {
    TEST_DESCRIPTION("Test mesh tasks emitted from task shader.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    std::string task_src_x = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout (local_size_x=1, local_size_y=1, local_size_z=1) in;
        void main () {
            EmitMeshTasksEXT()glsl";
    task_src_x += std::to_string(mesh_shader_properties.maxMeshWorkGroupCount[0] + 1);
    task_src_x += R"glsl(u, 1u, 1u);
        }
    )glsl";

    std::string task_src_y = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout (local_size_x=1, local_size_y=1, local_size_z=1) in;
        void main () {
            EmitMeshTasksEXT(1u, )glsl";
    task_src_y += std::to_string(mesh_shader_properties.maxMeshWorkGroupCount[1] + 1);
    task_src_y += R"glsl(u, 1u);
        }
    )glsl";

    std::string task_src_z = R"glsl(
        #version 450
        #extension GL_EXT_mesh_shader : require
        layout (local_size_x=1, local_size_y=1, local_size_z=1) in;
        void main () {
            EmitMeshTasksEXT(1u, 1u, )glsl";
    task_src_z += std::to_string(mesh_shader_properties.maxMeshWorkGroupCount[2] + 1);
    task_src_z += R"glsl(u);
        }
    )glsl";

    static const char mesh_src[] = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;
        void main() {
            SetMeshOutputsEXT(3, 1);
            gl_MeshVerticesEXT[0].gl_Position = vec4(-1.0, -1.0, 0.0f, 1.0f);
            gl_MeshVerticesEXT[1].gl_Position = vec4( 3.0, -1.0, 0.0f, 1.0f);
            gl_MeshVerticesEXT[2].gl_Position = vec4(-1.0,  3.0, 0.0f, 1.0f);
            gl_PrimitiveTriangleIndicesEXT[0] = uvec3(0, 1, 2);
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 460
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = vec4(0.2f, 0.4f, 0.6f, 0.8f);
        }
    )glsl";

    VkShaderObj task_shader_x(this, task_src_x.c_str(), VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_3);
    VkShaderObj task_shader_y(this, task_src_y.c_str(), VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_3);
    VkShaderObj task_shader_z(this, task_src_z.c_str(), VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_3);
    VkShaderObj mesh_shader(this, mesh_src, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3);
    VkShaderObj frag_shader(this, frag_src, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_3);

    // mesh and task shaders not supported
    const auto mesh_tasks_x = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task_shader_x.GetStageCreateInfo(), mesh_shader.GetStageCreateInfo(),
                                 frag_shader.GetStageCreateInfo()};
    };
    const auto mesh_tasks_y = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task_shader_y.GetStageCreateInfo(), mesh_shader.GetStageCreateInfo(),
                                 frag_shader.GetStageCreateInfo()};
    };
    const auto mesh_tasks_z = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task_shader_z.GetStageCreateInfo(), mesh_shader.GetStageCreateInfo(),
                                 frag_shader.GetStageCreateInfo()};
    };
    std::vector<std::string> vuids = {"VUID-RuntimeSpirv-TaskEXT-07299"};
    if (mesh_shader_properties.maxMeshWorkGroupCount[0] == mesh_shader_properties.maxMeshWorkGroupTotalCount) {
        vuids.emplace_back("VUID-RuntimeSpirv-TaskEXT-07302");
    }
    if (mesh_shader_properties.maxMeshWorkGroupCount[0] != std::numeric_limits<uint32_t>::max()) {
        CreatePipelineHelper::OneshotTest(*this, mesh_tasks_x, kErrorBit, vuids);
    }
    if (mesh_shader_properties.maxMeshWorkGroupCount[1] != std::numeric_limits<uint32_t>::max()) {
        CreatePipelineHelper::OneshotTest(*this, mesh_tasks_y, kErrorBit, "VUID-RuntimeSpirv-TaskEXT-07300");
    }
    if (mesh_shader_properties.maxMeshWorkGroupCount[2] != std::numeric_limits<uint32_t>::max()) {
        CreatePipelineHelper::OneshotTest(*this, mesh_tasks_z, kErrorBit, "VUID-RuntimeSpirv-TaskEXT-07301");
    }
}

TEST_F(NegativeMesh, MeshShaderConservativeRasterization) {
    TEST_DESCRIPTION("Use mesh shader with invalid conservative rasterization mode");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::meshShader);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    static const char meshShaderText[] = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        layout(max_vertices = 1) out;
        layout(max_primitives = 1) out;
        layout(points) out;
        void main() {
        }
    )glsl";

    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3);

    VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservative_rasterization_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(conservative_rasterization_props);
    if (conservative_rasterization_props.conservativePointAndLineRasterization) {
        GTEST_SKIP() << "Test requires conservativePointAndLineRasterization to be VK_FALSE";
    }

    VkPipelineRasterizationConservativeStateCreateInfoEXT conservative_state = vku::InitStructHelper();
    conservative_state.conservativeRasterizationMode = VK_CONSERVATIVE_RASTERIZATION_MODE_UNDERESTIMATE_EXT;

    CreatePipelineHelper pipe(*this);
    pipe.rs_state_ci_.pNext = &conservative_state;
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-conservativePointAndLineRasterization-06761");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMesh, MeshIncompatibleActiveQueries) {
    TEST_DESCRIPTION("Draw with mesh shaders when xfb or primitives generated queries are enabled");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::primitivesGeneratedQuery);
    AddRequiredFeature(vkt::Feature::transformFeedback);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkShaderObj ms(this, kMeshMinimalGlsl, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    vkt::QueryPool xfb_query_pool(*m_device, VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT, 1);
    vkt::QueryPool pg_query_pool(*m_device, VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, 1);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdBeginQuery(m_command_buffer.handle(), xfb_query_pool.handle(), 0u, 0u);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksEXT-None-07074");
    vk::CmdDrawMeshTasksEXT(m_command_buffer.handle(), 1u, 1u, 1u);
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(m_command_buffer.handle(), xfb_query_pool.handle(), 0u);

    vk::CmdBeginQuery(m_command_buffer.handle(), pg_query_pool.handle(), 0u, 0u);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksEXT-None-07075");
    vk::CmdDrawMeshTasksEXT(m_command_buffer.handle(), 1u, 1u, 1u);
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(m_command_buffer.handle(), pg_query_pool.handle(), 0u);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeMesh, DrawIndexMesh) {
    TEST_DESCRIPTION("use DrawIndex in Mesh shader but there is a Task Shader.");
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
            mesh_payload[gl_LocalInvocationIndex] = gl_GlobalInvocationID.x;
            EmitMeshTasksEXT(32u, 1u, 1u);
        }
    )glsl";

    const char *mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        layout(max_vertices = 32, max_primitives = 32, triangles) out;
        taskPayloadSharedEXT uint mesh_payload[32];
        void main() {
            uint compacted_meshlet_index = uint(32768 * gl_DrawID) + gl_WorkGroupID.x;
            SetMeshOutputsEXT(3,1);
        }
    )glsl";

    VkShaderObj ts(this, task_source, VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj ms(this, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ts.GetStageCreateInfo(), ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-pStages-09631");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMesh, DrawIndexMeshShaderObject) {
    TEST_DESCRIPTION("use DrawIndex in Mesh shader but there is a Task Shader.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::shaderDrawParameters);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *mesh_src = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        layout(max_vertices = 32, max_primitives = 32, triangles) out;
        taskPayloadSharedEXT uint mesh_payload[32];
        void main() {
            uint compacted_meshlet_index = uint(32768 * gl_DrawID) + gl_WorkGroupID.x;
            SetMeshOutputsEXT(3,1);
        }
    )glsl";

    m_errorMonitor->SetDesiredError("VUID-vkCreateShadersEXT-pCreateInfos-09632");
    const vkt::Shader meshShader(*m_device, VK_SHADER_STAGE_MESH_BIT_EXT,
                                 GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, mesh_src, SPV_ENV_VULKAN_1_2));
    m_errorMonitor->VerifyFound();
}
