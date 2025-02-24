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

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_object_helper.h"

class NegativeGeometryTessellation : public VkLayerTest {};

TEST_F(NegativeGeometryTessellation, StageMaskGsTsEnabled) {
    TEST_DESCRIPTION(
        "Attempt to use a stageMask w/ geometry shader and tessellation shader bits enabled when those features are disabled on "
        "the device.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    std::vector<const char *> device_extension_names;
    auto features = m_device->Physical().Features();
    // Make sure gs & ts are disabled
    features.geometryShader = false;
    features.tessellationShader = false;
    // The sacrificial device object
    vkt::Device test_device(Gpu(), device_extension_names, &features);

    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = test_device.graphics_queue_node_index_;

    VkCommandPool command_pool;
    vk::CreateCommandPool(test_device.handle(), &pool_create_info, nullptr, &command_pool);

    VkCommandBufferAllocateInfo cmd = vku::InitStructHelper();
    cmd.commandPool = command_pool;
    cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount = 1;

    VkCommandBuffer cmd_buffer;
    VkResult err = vk::AllocateCommandBuffers(test_device.handle(), &cmd, &cmd_buffer);
    ASSERT_EQ(VK_SUCCESS, err);

    VkEvent event;
    VkEventCreateInfo evci = vku::InitStructHelper();
    VkResult result = vk::CreateEvent(test_device.handle(), &evci, NULL, &event);
    ASSERT_EQ(VK_SUCCESS, result);

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    vk::BeginCommandBuffer(cmd_buffer, &cbbi);
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent-stageMask-04090");
    vk::CmdSetEvent(cmd_buffer, event, VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent-stageMask-04091");
    vk::CmdSetEvent(cmd_buffer, event, VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT);
    m_errorMonitor->VerifyFound();

    vk::DestroyEvent(test_device.handle(), event, NULL);
    vk::DestroyCommandPool(test_device.handle(), command_pool, NULL);
}

TEST_F(NegativeGeometryTessellation, GeometryShaderEnabled) {
    TEST_DESCRIPTION("Validate geometry shader feature is enabled if geometry shader stage is used");

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.geometryShader = VK_FALSE;

    RETURN_IF_SKIP(Init(&deviceFeatures));
    InitRenderTarget();

    if (m_device->Physical().limits_.maxGeometryOutputVertices == 0) {
        GTEST_SKIP() << "Device doesn't support geometry shaders";
    }

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkShaderModuleCreateInfo-pCode-08740");
    VkShaderObj gs(this, kGeometryMinimalGlsl, VK_SHADER_STAGE_GEOMETRY_BIT);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };

    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineShaderStageCreateInfo-stage-00704");
}

TEST_F(NegativeGeometryTessellation, TessellationShaderEnabled) {
    TEST_DESCRIPTION(
        "Validate tessellation shader feature is enabled if tessellation control or tessellation evaluation shader stage is used");

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.tessellationShader = VK_FALSE;

    RETURN_IF_SKIP(Init(&deviceFeatures));
    InitRenderTarget();

    if (m_device->Physical().limits_.maxTessellationPatchSize == 0) {
        GTEST_SKIP() << "patchControlPoints not supported";
    }

    char const *tcsSource = R"glsl(
        #version 450
        layout(location=0) out int x[];
        layout(vertices=3) out;
        void main(){
           gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;
           gl_TessLevelInner[0] = 1;
           x[gl_InvocationID] = gl_InvocationID;
        }
    )glsl";
    char const *tesSource = R"glsl(
        #version 450
        layout(triangles, equal_spacing, cw) in;
        layout(location=0) patch in int x;
        void main(){
           gl_Position.xyz = gl_TessCoord;
           gl_Position.w = x;
        }
    )glsl";

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkShaderModuleCreateInfo-pCode-08740");
    VkShaderObj tcs(this, tcsSource, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkShaderModuleCreateInfo-pCode-08740");
    VkShaderObj tes(this, tesSource, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};

    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        helper.gp_ci_.pTessellationState = &tsci;
        helper.gp_ci_.pInputAssemblyState = &iasci;
        helper.shader_stages_.emplace_back(tcs.GetStageCreateInfo());
        helper.shader_stages_.emplace_back(tes.GetStageCreateInfo());
    };
    constexpr std::array vuids = {"VUID-VkPipelineShaderStageCreateInfo-stage-00705",
                                  "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00430"};
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
}

TEST_F(NegativeGeometryTessellation, PointSizeGeomShaderDontWrite) {
    TEST_DESCRIPTION(
        "Create a pipeline using TOPOLOGY_POINT_LIST, set PointSize vertex shader, but not in the final geometry stage.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::shaderTessellationAndGeometryPointSize);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Create GS declaring PointSize and writing to it
    static char const *gsSource = R"glsl(
        #version 450
        layout (points) in;
        layout (points) out;
        layout (max_vertices = 1) out;
        void main() {
           gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
           EmitVertex();
        }
    )glsl";

    VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };

    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "VUID-VkGraphicsPipelineCreateInfo-shaderTessellationAndGeometryPointSize-08776");
}

TEST_F(NegativeGeometryTessellation, PointSizeGeomShaderWrite) {
    TEST_DESCRIPTION(
        "Create a pipeline using TOPOLOGY_POINT_LIST, set PointSize vertex shader, but not in the final geometry stage.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Compiled using the GLSL code below. GlslangValidator rearranges the members, but here they are kept in the order provided.
    // #version 450
    // layout (points) in;
    // layout (points) out;
    // layout (max_vertices = 1) out;
    // void main() {
    //     gl_Position = vec4(1.0f);
    //     gl_PointSize = 1.0f;
    //     EmitVertex();
    // }
    const char *gsSource = R"(
               OpCapability Geometry
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %_
               OpExecutionMode %main InputPoints
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputPoints
               OpExecutionMode %main OutputVertices 1
               OpName %_ ""
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %19 %17
         %22 = OpAccessChain %_ptr_Output_float %_ %int_1
               OpStore %22 %float_1
               OpEmitVertex
               OpReturn
               OpFunctionEnd
        )";

    VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };

    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-Geometry-07726");
}

TEST_F(NegativeGeometryTessellation, BuiltinBlockOrderMismatchVsGs) {
    TEST_DESCRIPTION("Use different order of gl_Position and gl_PointSize in builtin block interface between VS and GS.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::shaderTessellationAndGeometryPointSize);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Compiled using the GLSL code below. GlslangValidator rearranges the members, but here they are kept in the order provided.
    // #version 450
    // layout (points) in;
    // layout (points) out;
    // layout (max_vertices = 1) out;
    // in gl_PerVertex {
    //     float gl_PointSize;
    //     vec4 gl_Position;
    // } gl_in[];
    // void main() {
    //     gl_Position = gl_in[0].gl_Position;
    //     gl_PointSize = gl_in[0].gl_PointSize;
    //     EmitVertex();
    // }

    const char *gsSource = R"(
               OpCapability Geometry
               OpCapability GeometryPointSize
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %_ %gl_in
               OpExecutionMode %main InputPoints
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputPoints
               OpExecutionMode %main OutputVertices 1
               OpSource GLSL 450
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpMemberDecorate %gl_PerVertex_0 0 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex_0 1 BuiltIn Position
               OpDecorate %gl_PerVertex_0 Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%gl_PerVertex_0 = OpTypeStruct %float %v4float
%_arr_gl_PerVertex_0_uint_1 = OpTypeArray %gl_PerVertex_0 %uint_1
%_ptr_Input__arr_gl_PerVertex_0_uint_1 = OpTypePointer Input %_arr_gl_PerVertex_0_uint_1
      %gl_in = OpVariable %_ptr_Input__arr_gl_PerVertex_0_uint_1 Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %int_1 = OpConstant %int 1
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Output_float = OpTypePointer Output %float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %21 = OpAccessChain %_ptr_Input_v4float %gl_in %int_0 %int_1
         %22 = OpLoad %v4float %21
         %24 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %24 %22
         %27 = OpAccessChain %_ptr_Input_float %gl_in %int_0 %int_0
         %28 = OpLoad %float %27
         %30 = OpAccessChain %_ptr_Output_float %_ %int_1
               OpStore %30 %28
               OpEmitVertex
               OpReturn
               OpFunctionEnd
        )";

    VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpVariable-08746");
}

TEST_F(NegativeGeometryTessellation, BuiltinBlockSizeMismatchVsGs) {
    TEST_DESCRIPTION("Use different number of elements in builtin block interface between VS and GS.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::shaderTessellationAndGeometryPointSize);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    static const char *gsSource = R"glsl(
        #version 450
        layout (points) in;
        layout (points) out;
        layout (max_vertices = 1) out;
        in gl_PerVertex
        {
            vec4 gl_Position;
            float gl_PointSize;
            float gl_ClipDistance[];
        } gl_in[];
        void main()
        {
            gl_Position = gl_in[0].gl_Position;
            gl_PointSize = gl_in[0].gl_PointSize;
            EmitVertex();
        }
    )glsl";

    VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpVariable-08746");
}

TEST_F(NegativeGeometryTessellation, BuiltinBlockSizeMismatchVsGsShaderObject) {
    TEST_DESCRIPTION("Use different number of elements in builtin block interface between VS and GS.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::shaderTessellationAndGeometryPointSize);
    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();

    static const char *gsSource = R"glsl(
        #version 450
        layout (points) in;
        layout (points) out;
        layout (max_vertices = 1) out;
        in gl_PerVertex
        {
            vec4 gl_Position;
            float gl_PointSize;
            float gl_ClipDistance[];
        } gl_in[];
        void main()
        {
            gl_Position = gl_in[0].gl_Position;
            gl_PointSize = gl_in[0].gl_PointSize;
            EmitVertex();
        }
    )glsl";

    const vkt::Shader vertShader(*m_device, VK_SHADER_STAGE_VERTEX_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexPointSizeGlsl));
    const vkt::Shader geomShader(*m_device, VK_SHADER_STAGE_GEOMETRY_BIT, GLSLToSPV(VK_SHADER_STAGE_GEOMETRY_BIT, gsSource));
    const vkt::Shader fragShader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl));

    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vertShader.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, geomShader.handle(), fragShader.handle()};

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 5, stages, shaders);
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-OpVariable-08746");
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeGeometryTessellation, MaxTessellationControlInputOutputComponents) {
    TEST_DESCRIPTION(
        "Test that errors are produced when the number of per-vertex input and/or output components to the tessellation control "
        "stage exceeds the device limit");

    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // overflow == 0: no overflow, 1: too many components, 2: location number too large
    for (uint32_t overflow = 0; overflow < 3; ++overflow) {
        m_errorMonitor->Reset();

        // Tessellation control stage
        std::string tcsSourceStr =
            "#version 450\n"
            "\n";
        // Input components
        const uint32_t maxTescInComp = m_device->Physical().limits_.maxTessellationControlPerVertexInputComponents + overflow;
        const uint32_t numInVec4 = maxTescInComp / 4;
        uint32_t inLocation = 0;
        if (overflow == 2) {
            tcsSourceStr += "layout(location=" + std::to_string(numInVec4 + 1) + ") in vec4 vnIn[];\n";
        } else if (overflow == 1) {
            for (uint32_t i = 0; i < numInVec4; i++) {
                tcsSourceStr += "layout(location=" + std::to_string(inLocation) + ") in vec4 v" + std::to_string(i) + "In[];\n";
                inLocation += 1;
            }
            const uint32_t inRemainder = maxTescInComp % 4;
            if (inRemainder != 0) {
                if (inRemainder == 1) {
                    tcsSourceStr += "layout(location=" + std::to_string(inLocation) + ") in float" + " vnIn[];\n";
                } else {
                    tcsSourceStr +=
                        "layout(location=" + std::to_string(inLocation) + ") in vec" + std::to_string(inRemainder) + " vnIn[];\n";
                }
                inLocation += 1;
            }
        }

        // Output components
        const uint32_t maxTescOutComp = m_device->Physical().limits_.maxTessellationControlPerVertexOutputComponents + overflow;
        const uint32_t numOutVec4 = maxTescOutComp / 4;
        uint32_t outLocation = 0;
        if (overflow == 2) {
            tcsSourceStr += "layout(location=" + std::to_string(numOutVec4 + 1) + ") out vec4 vnOut[3];\n";
        } else if (overflow == 1) {
            for (uint32_t i = 0; i < numOutVec4; i++) {
                tcsSourceStr += "layout(location=" + std::to_string(outLocation) + ") out vec4 v" + std::to_string(i) + "Out[3];\n";
                outLocation += 1;
            }
            const uint32_t outRemainder = maxTescOutComp % 4;
            if (outRemainder != 0) {
                if (outRemainder == 1) {
                    tcsSourceStr += "layout(location=" + std::to_string(outLocation) + ") out float" + " vnOut[3];\n";
                } else {
                    tcsSourceStr += "layout(location=" + std::to_string(outLocation) + ") out vec" + std::to_string(outRemainder) +
                                    " vnOut[3];\n";
                }
                outLocation += 1;
            }
        }

        tcsSourceStr += "layout(vertices=3) out;\n";
        // Finalize
        tcsSourceStr +=
            "\n"
            "void main(){\n"
            "}\n";

        // maxTessellationControlPerVertexInputComponents and maxTessellationControlPerVertexOutputComponents
        switch (overflow) {
            case 0: {
                VkShaderObj tcs(this, tcsSourceStr.c_str(), VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
                break;
            }
            case 1: {
                // in and out component limit
                m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
                m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
                VkShaderObj tcs(this, tcsSourceStr.c_str(), VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
                m_errorMonitor->VerifyFound();
                break;
            }
            case 2: {
                // in and out component limit
                m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
                m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
                VkShaderObj tcs(this, tcsSourceStr.c_str(), VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
                m_errorMonitor->VerifyFound();
                break;
            }
            default: {
                assert(0);
            }
        }
    }
}

TEST_F(NegativeGeometryTessellation, MaxTessellationEvaluationInputOutputComponents) {
    TEST_DESCRIPTION(
        "Test that errors are produced when the number of input and/or output components to the tessellation evaluation stage "
        "exceeds the device limit");

    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // overflow == 0: no overflow, 1: too many components, 2: location number too large
    for (uint32_t overflow = 0; overflow < 3; ++overflow) {
        m_errorMonitor->Reset();

        // Tessellation evaluation stage
        std::string tesSourceStr =
            "#version 450\n"
            "\n"
            "layout (triangles) in;\n"
            "\n";
        // Input components
        const uint32_t maxTeseInComp = m_device->Physical().limits_.maxTessellationEvaluationInputComponents + overflow;
        const uint32_t numInVec4 = maxTeseInComp / 4;
        uint32_t inLocation = 0;
        if (overflow == 2) {
            tesSourceStr += "layout(location=" + std::to_string(numInVec4 + 1) + ") in vec4 vnIn[];\n";
        } else if (overflow == 1) {
            for (uint32_t i = 0; i < numInVec4; i++) {
                tesSourceStr += "layout(location=" + std::to_string(inLocation) + ") in vec4 v" + std::to_string(i) + "In[];\n";
                inLocation += 1;
            }
            const uint32_t inRemainder = maxTeseInComp % 4;
            if (inRemainder != 0) {
                if (inRemainder == 1) {
                    tesSourceStr += "layout(location=" + std::to_string(inLocation) + ") in float" + " vnIn[];\n";
                } else {
                    tesSourceStr +=
                        "layout(location=" + std::to_string(inLocation) + ") in vec" + std::to_string(inRemainder) + " vnIn[];\n";
                }
                inLocation += 1;
            }
        }

        // Output components
        const uint32_t maxTeseOutComp = m_device->Physical().limits_.maxTessellationEvaluationOutputComponents + overflow;
        const uint32_t numOutVec4 = maxTeseOutComp / 4;
        uint32_t outLocation = 0;
        if (overflow == 2) {
            tesSourceStr += "layout(location=" + std::to_string(numOutVec4 + 1) + ") out vec4 vnOut;\n";
        } else if (overflow == 1) {
            for (uint32_t i = 0; i < numOutVec4; i++) {
                tesSourceStr += "layout(location=" + std::to_string(outLocation) + ") out vec4 v" + std::to_string(i) + "Out;\n";
                outLocation += 1;
            }
            const uint32_t outRemainder = maxTeseOutComp % 4;
            if (outRemainder != 0) {
                if (outRemainder == 1) {
                    tesSourceStr += "layout(location=" + std::to_string(outLocation) + ") out float" + " vnOut;\n";
                } else {
                    tesSourceStr +=
                        "layout(location=" + std::to_string(outLocation) + ") out vec" + std::to_string(outRemainder) + " vnOut;\n";
                }
                outLocation += 1;
            }
        }

        // Finalize
        tesSourceStr +=
            "\n"
            "void main(){\n"
            "}\n";

        // maxTessellationEvaluationInputComponents and maxTessellationEvaluationOutputComponents
        switch (overflow) {
            case 0: {
                VkShaderObj tes(this, tesSourceStr.c_str(), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
                break;
            }
            case 1: {
                // in and out component limit
                m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
                m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
                VkShaderObj tes(this, tesSourceStr.c_str(), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
                m_errorMonitor->VerifyFound();
                break;
            }
            case 2: {
                // in and out component limit
                m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
                m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
                VkShaderObj tes(this, tesSourceStr.c_str(), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
                m_errorMonitor->VerifyFound();
                break;
            }
            default: {
                assert(false);
            }
        }
    }
}

TEST_F(NegativeGeometryTessellation, MaxGeometryInputOutputComponents) {
    TEST_DESCRIPTION(
        "Test that errors are produced when the number of input and/or output components to the geometry stage exceeds the device "
        "limit");

    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // overflow == 0: no overflow, 1: too many components, 2: location number too large
    for (uint32_t overflow = 0; overflow < 3; ++overflow) {
        m_errorMonitor->Reset();

        std::string gsSourceStr =
            "#version 450\n"
            "\n"
            "layout(triangles) in;\n"
            "layout(invocations=1) in;\n";

        // Input components
        const uint32_t maxGeomInComp = m_device->Physical().limits_.maxGeometryInputComponents + overflow;
        const uint32_t numInVec4 = maxGeomInComp / 4;
        uint32_t inLocation = 0;
        if (overflow == 2) {
            gsSourceStr += "layout(location=" + std::to_string(numInVec4 + 1) + ") in vec4 vnIn[];\n";
        } else {
            for (uint32_t i = 0; i < numInVec4; i++) {
                gsSourceStr += "layout(location=" + std::to_string(inLocation) + ") in vec4 v" + std::to_string(i) + "In[];\n";
                inLocation += 1;
            }
            const uint32_t inRemainder = maxGeomInComp % 4;
            if (inRemainder != 0) {
                if (inRemainder == 1) {
                    gsSourceStr += "layout(location=" + std::to_string(inLocation) + ") in float" + " vnIn[];\n";
                } else {
                    gsSourceStr +=
                        "layout(location=" + std::to_string(inLocation) + ") in vec" + std::to_string(inRemainder) + " vnIn[];\n";
                }
                inLocation += 1;
            }
        }

        // Output components
        const uint32_t maxGeomOutComp = m_device->Physical().limits_.maxGeometryOutputComponents + overflow;
        const uint32_t numOutVec4 = maxGeomOutComp / 4;
        uint32_t outLocation = 0;
        if (overflow == 2) {
            gsSourceStr += "layout(location=" + std::to_string(numOutVec4) + ") out vec4 vnOut;\n";
        } else if (overflow == 1) {
            for (uint32_t i = 0; i < numOutVec4; i++) {
                gsSourceStr += "layout(location=" + std::to_string(outLocation) + ") out vec4 v" + std::to_string(i) + "Out;\n";
                outLocation += 1;
            }
            const uint32_t outRemainder = maxGeomOutComp % 4;
            if (outRemainder != 0) {
                if (outRemainder == 1) {
                    gsSourceStr += "layout(location=" + std::to_string(outLocation) + ") out float" + " vnOut;\n";
                } else {
                    gsSourceStr +=
                        "layout(location=" + std::to_string(outLocation) + ") out vec" + std::to_string(outRemainder) + " vnOut;\n";
                }
                outLocation += 1;
            }
        }

        // Finalize
        int max_vertices = overflow ? (m_device->Physical().limits_.maxGeometryTotalOutputComponents / maxGeomOutComp + 1) : 1;
        gsSourceStr += "layout(triangle_strip, max_vertices = " + std::to_string(max_vertices) +
                       ") out;\n"
                       "\n"
                       "void main(){\n"
                       "}\n";

        // maxGeometryInputComponents and maxGeometryOutputComponents
        switch (overflow) {
            case 0: {
                VkShaderObj gs(this, gsSourceStr.c_str(), VK_SHADER_STAGE_GEOMETRY_BIT);
                break;
            }
            case 1: {
                // in and out component limit
                m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
                m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
                VkShaderObj gs(this, gsSourceStr.c_str(), VK_SHADER_STAGE_GEOMETRY_BIT);
                m_errorMonitor->VerifyFound();
                break;
            }
            case 2: {
                // in and out component limit
                m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
                m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-Location-06272");
                VkShaderObj gs(this, gsSourceStr.c_str(), VK_SHADER_STAGE_GEOMETRY_BIT);
                m_errorMonitor->VerifyFound();
                break;
            }
            default: {
                assert(0);
            }
        }
    }
}

TEST_F(NegativeGeometryTessellation, MaxGeometryInstanceVertexCount) {
    TEST_DESCRIPTION(
        "Test that errors are produced when the number of output vertices/instances in the geometry stage exceeds the device "
        "limit");

    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    for (int overflow = 0; overflow < 2; ++overflow) {
        m_errorMonitor->Reset();

        std::string gsSourceStr = R"(
               OpCapability Geometry
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main"
               OpExecutionMode %main InputPoints
               OpExecutionMode %main OutputTriangleStrip
               )";
        if (overflow) {
            gsSourceStr += "OpExecutionMode %main Invocations " +
                           std::to_string(m_device->Physical().limits_.maxGeometryShaderInvocations + 1) +
                           "\n\
                OpExecutionMode %main OutputVertices " +
                           std::to_string(m_device->Physical().limits_.maxGeometryOutputVertices + 1);
        } else {
            gsSourceStr += R"(
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputVertices 1
               )";
        }
        gsSourceStr += R"(
               OpSource GLSL 450
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";
        VkShaderObj gs(this, gsSourceStr.c_str(), VK_SHADER_STAGE_GEOMETRY_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        if (overflow) {
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                              std::vector<std::string>{"VUID-VkPipelineShaderStageCreateInfo-stage-00714",
                                                                       "VUID-VkPipelineShaderStageCreateInfo-stage-00715"});
        } else {
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
        }
    }
}

// Waiting on https://gitlab.khronos.org/vulkan/vulkan/-/issues/3858 to know what is valid or not
TEST_F(NegativeGeometryTessellation, DISABLED_TessellationPatchDecorationMismatch) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a variable output from the TCS without the patch decoration, but consumed in the TES "
        "with the decoration.");

    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *tcsSource = R"glsl(
        #version 450
        layout(location=0) out int x[];
        layout(vertices=3) out;
        void main(){
           gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;
           gl_TessLevelInner[0] = 1;
           x[gl_InvocationID] = gl_InvocationID;
        }
    )glsl";
    char const *tesSource = R"glsl(
        #version 450
        layout(triangles, equal_spacing, cw) in;
        layout(location=0) patch in int x;
        void main(){
           gl_Position.xyz = gl_TessCoord;
           gl_Position.w = x;
        }
    )glsl";
    VkShaderObj tcs(this, tcsSource, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, tesSource, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};

    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.gp_ci_.pTessellationState = &tsci;
        helper.gp_ci_.pInputAssemblyState = &iasci;
        helper.shader_stages_.emplace_back(tcs.GetStageCreateInfo());
        helper.shader_stages_.emplace_back(tes.GetStageCreateInfo());
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpVariable-08746");
}

TEST_F(NegativeGeometryTessellation, Tessellation) {
    TEST_DESCRIPTION("Test various errors when creating a graphics pipeline with tessellation stages active.");

    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *tcsSource = R"glsl(
        #version 450
        layout(vertices=3) out;
        void main(){
           gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;
           gl_TessLevelInner[0] = 1;
        }
    )glsl";
    char const *tesSource = R"glsl(
        #version 450
        layout(triangles, equal_spacing, cw) in;
        void main(){
           gl_Position.xyz = gl_TessCoord;
           gl_Position.w = 0;
        }
    )glsl";
    VkShaderObj tcs(this, tcsSource, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, tesSource, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};

    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {};
    VkPipelineInputAssemblyStateCreateInfo iasci_bad = iasci;
    VkPipelineInputAssemblyStateCreateInfo *p_iasci = nullptr;
    VkPipelineTessellationStateCreateInfo tsci_bad = tsci;
    VkPipelineTessellationStateCreateInfo *p_tsci = nullptr;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.gp_ci_.pTessellationState = p_tsci;
        helper.gp_ci_.pInputAssemblyState = p_iasci;
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        helper.shader_stages_.insert(helper.shader_stages_.end(), shader_stages.begin(), shader_stages.end());
    };

    iasci_bad.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;  // otherwise we get a failure about invalid topology
    p_iasci = &iasci_bad;
    // Pass a tess control shader without a tess eval shader
    shader_stages = {tcs.GetStageCreateInfo()};
    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-pStages-09022");
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-00729");

    // Pass a tess eval shader without a tess control shader
    shader_stages = {tes.GetStageCreateInfo()};
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-00730");

    p_iasci = &iasci;
    shader_stages = {};
    // Pass patch topology without tessellation shaders
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-topology-08889");

    shader_stages = {tcs.GetStageCreateInfo(), tes.GetStageCreateInfo()};
    // Pass a NULL pTessellationState (with active tessellation shader stages)
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-09022");

    // Pass an invalid pTessellationState (bad sType)
    tsci_bad.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    p_tsci = &tsci_bad;
    shader_stages = {tcs.GetStageCreateInfo(), tes.GetStageCreateInfo()};
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineTessellationStateCreateInfo-sType-sType");

    // Pass out-of-range patchControlPoints
    p_iasci = &iasci;
    tsci_bad = tsci;
    tsci_bad.patchControlPoints = 0;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "VUID-VkPipelineTessellationStateCreateInfo-patchControlPoints-01214");

    tsci_bad.patchControlPoints = m_device->Physical().limits_.maxTessellationPatchSize + 1;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "VUID-VkPipelineTessellationStateCreateInfo-patchControlPoints-01214");
}

TEST_F(NegativeGeometryTessellation, PatchListTopology) {
    TEST_DESCRIPTION("Need to have VK_PRIMITIVE_TOPOLOGY_PATCH_LIST.");

    AddOptionalExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkShaderObj tcs(this, kTessellationControlMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, kTessellationEvalMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE};
    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.gp_ci_.pTessellationState = &tsci;
        helper.gp_ci_.pInputAssemblyState = &iasci;
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo(), tcs.GetStageCreateInfo(),
                                 tes.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-08888");
}

/*// TODO : This test should be good, but needs Tess support in compiler to run
TEST_F(NegativeGeometryTessellation, PatchControlPoints)
{
    // Attempt to Create Gfx Pipeline w/o a VS
    VkResult err;

    m_errorMonitor->SetDesiredError("Invalid Pipeline CreateInfo State: VK_PRIMITIVE_TOPOLOGY_PATCH primitive ");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkDescriptorPoolSize ds_type_count = {};
        ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
        ds_pool_ci.poolSizeCount = 1;
        ds_pool_ci.pPoolSizes = &ds_type_count;
    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {};
        dsl_binding.binding = 0;
        dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        dsl_binding.descriptorCount = 1;
        dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
        dsl_binding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
        ds_layout_ci.bindingCount = 1;
        ds_layout_ci.pBindings = &dsl_binding;

    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

    VkDescriptorSet descriptorSet;
    err = vk::AllocateDescriptorSets(device(), ds_pool.handle(),
VK_DESCRIPTOR_SET_USAGE_NON_FREE, 1, &ds_layout.handle(), &descriptorSet);
    ASSERT_EQ(VK_SUCCESS, err);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
        pipeline_layout_ci.pNext = NULL;
        pipeline_layout_ci.setLayoutCount = 1;
        pipeline_layout_ci.pSetLayouts = &ds_layout.handle();
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
    ASSERT_EQ(VK_SUCCESS, err);

    VkPipelineShaderStageCreateInfo shaderStages[3];
    memset(&shaderStages, 0, 3 * sizeof(VkPipelineShaderStageCreateInfo));

    VkShaderObj vs(this,kVertexMinimalGlsl,VK_SHADER_STAGE_VERTEX_BIT);
    // Just using VS txt for Tess shaders as we don't care about functionality
    VkShaderObj tc(this,kVertexMinimalGlsl,VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj te(this,kVertexMinimalGlsl,VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    shaderStages[0] = vku::InitStructHelper();
    shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].shader = vs.handle();
    shaderStages[1] = vku::InitStructHelper();
    shaderStages[1].stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    shaderStages[1].shader = tc.handle();
    shaderStages[2] = vku::InitStructHelper();
    shaderStages[2].stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    shaderStages[2].shader = te.handle();

    VkPipelineInputAssemblyStateCreateInfo iaCI = vku::InitStructHelper();
        iaCI.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

    VkPipelineTessellationStateCreateInfo tsCI = vku::InitStructHelper();
        tsCI.patchControlPoints = 0; // This will cause an error

    VkGraphicsPipelineCreateInfo gp_ci = vku::InitStructHelper();
        gp_ci.stageCount = 3;
        gp_ci.pStages = shaderStages;
        gp_ci.pVertexInputState = NULL;
        gp_ci.pInputAssemblyState = &iaCI;
        gp_ci.pTessellationState = &tsCI;
        gp_ci.pViewportState = NULL;
        gp_ci.pRasterizationState = NULL;
        gp_ci.pMultisampleState = NULL;
        gp_ci.pDepthStencilState = NULL;
        gp_ci.pColorBlendState = NULL;
        gp_ci.flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT;
        gp_ci.layout = pipeline_layout.handle();
        gp_ci.renderPass = RenderPass();

    VkPipelineCacheCreateInfo pc_ci = vku::InitStructHelper();
        pc_ci.initialSize = 0;
        pc_ci.initialData = 0;
        pc_ci.maxSize = 0;

    VkPipeline pipeline;
    VkPipelineCache pipelineCache;

    err = vk::CreatePipelineCache(device(), &pc_ci, NULL,
&pipelineCache);
    ASSERT_EQ(VK_SUCCESS, err);
    err = vk::CreateGraphicsPipelines(device(), pipelineCache, 1,
&gp_ci, NULL, &pipeline);

    m_errorMonitor->VerifyFound();

    vk::DestroyPipelineCache(device(), pipelineCache, NULL);
}
*/

TEST_F(NegativeGeometryTessellation, IncompatiblePrimitiveTopology) {
    TEST_DESCRIPTION("Create pipeline with primitive topology incompatible with shaders.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::shaderTessellationAndGeometryPointSize);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    static const char *gsSource = R"glsl(
        #version 450
        layout (points) in;
        layout (triangle_strip) out;
        layout (max_vertices = 3) out;
        in gl_PerVertex
        {
            vec4 gl_Position;
            float gl_PointSize;
        } gl_in[];
        void main()
        {
            gl_Position = gl_in[0].gl_Position;
            gl_PointSize = gl_in[0].gl_PointSize;
            EmitVertex();
        }
    )glsl";

    VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-00738");
}

TEST_F(NegativeGeometryTessellation, IncompatibleTessGeomPrimitiveTopology) {
    TEST_DESCRIPTION("Create pipeline with incompatible topology between tess and geom shaders.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::shaderTessellationAndGeometryPointSize);
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *tcsSource = R"glsl(
        #version 450
        layout(location=0) out int x[];
        layout(vertices=3) out;
        void main(){
           gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;
           gl_TessLevelInner[0] = 1;
           x[gl_InvocationID] = gl_InvocationID;
        }
    )glsl";
    char const *tesSource = R"glsl(
        #version 450
        layout(triangles, equal_spacing, cw) in;
        layout(location=0) patch in int x;
        void main(){
           gl_Position.xyz = gl_TessCoord;
           gl_Position.w = x;
        }
    )glsl";
    static const char *gsSource = R"glsl(
        #version 450
        layout (points) in;
        layout (triangle_strip) out;
        layout (max_vertices = 3) out;
        in gl_PerVertex
        {
            vec4 gl_Position;
            float gl_PointSize;
            float gl_ClipDistance[];
        } gl_in[];
        void main()
        {
            gl_Position = gl_in[0].gl_Position;
            gl_PointSize = gl_in[0].gl_PointSize;
            EmitVertex();
        }
    )glsl";

    VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj tcs(this, tcsSource, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, tesSource, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        helper.gp_ci_.pTessellationState = &tsci;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), tcs.GetStageCreateInfo(),
                                 tes.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pStages-00739");
}

TEST_F(NegativeGeometryTessellation, PipelineTessellationMissingPointSize) {
    TEST_DESCRIPTION("Create pipeline with tessellation shader with missing point size");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework());
    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_subset_features = vku::InitStructHelper();
        VkPhysicalDeviceFeatures2 features2;
        features2 = GetPhysicalDeviceFeatures2(portability_subset_features);
        if (!features2.features.tessellationShader || !features2.features.shaderTessellationAndGeometryPointSize) {
            GTEST_SKIP() << "tessellationShader or shaderTessellationAndGeometryPointSize not supported";
        }
        if (!portability_subset_features.tessellationPointMode) {
            GTEST_SKIP() << "tessellationPointMode not supported";
        }
        RETURN_IF_SKIP(InitState(nullptr, &features2));
    } else {
        VkPhysicalDeviceFeatures features;
        GetPhysicalDeviceFeatures(&features);
        if (!features.tessellationShader || !features.shaderTessellationAndGeometryPointSize) {
            GTEST_SKIP() << "tessellationShader or shaderTessellationAndGeometryPointSize not supported";
        }
        RETURN_IF_SKIP(InitState(&features));
    }
    InitRenderTarget();

    static const char tess_src[] = R"glsl(
        #version 460
        layout(triangles, equal_spacing, cw, point_mode) in;
        void main() { gl_Position = vec4(1); }
    )glsl";

    VkShaderObj tcs(this, kTessellationControlMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, tess_src, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkPipelineTessellationStateCreateInfo tess_ci = vku::InitStructHelper();
    tess_ci.patchControlPoints = 4u;

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(),
                           pipe.fs_->GetStageCreateInfo()};
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    pipe.tess_ci_ = tess_ci;
    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-TessellationEvaluation-07723");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGeometryTessellation, PipelineTessellationPointSize) {
    TEST_DESCRIPTION("Create pipeline with tessellation shader with missing point size");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework());
    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_subset_features = vku::InitStructHelper();
        VkPhysicalDeviceFeatures2 features2;
        features2 = GetPhysicalDeviceFeatures2(portability_subset_features);
        if (!features2.features.tessellationShader) {
            GTEST_SKIP() << "tessellationShader not supported";
        }
        if (!portability_subset_features.tessellationPointMode) {
            GTEST_SKIP() << "tessellationPointMode not supported";
        }
        features2.features.shaderTessellationAndGeometryPointSize = VK_FALSE;
        RETURN_IF_SKIP(InitState(nullptr, &features2));
    } else {
        VkPhysicalDeviceFeatures features;
        GetPhysicalDeviceFeatures(&features);
        if (!features.tessellationShader) {
            GTEST_SKIP() << "tessellationShader not supported";
        }
        features.shaderTessellationAndGeometryPointSize = VK_FALSE;
        RETURN_IF_SKIP(InitState(&features));
    }
    InitRenderTarget();

    static const char tess_src[] = R"glsl(
        #version 460
        layout(triangles, equal_spacing, cw, point_mode) in;
        void main() {
            gl_Position = vec4(1);
            gl_PointSize = 1.0f;
        }
    )glsl";

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkShaderModuleCreateInfo-pCode-08740");
    VkShaderObj tcs(this, kTessellationControlMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkShaderModuleCreateInfo-pCode-08740");
    VkShaderObj tes(this, tess_src, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkPipelineTessellationStateCreateInfo tess_ci = vku::InitStructHelper();
    tess_ci.patchControlPoints = 4u;

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(),
                           pipe.fs_->GetStageCreateInfo()};
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    pipe.tess_ci_ = tess_ci;
    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-TessellationEvaluation-07724");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGeometryTessellation, GeometryStreamsCapability) {
    TEST_DESCRIPTION("Use geometry shader with geometryStreams capability, but geometryStreams feature not enabled");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::transformFeedback);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceTransformFeedbackPropertiesEXT xfb_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(xfb_props);

    if (xfb_props.maxTransformFeedbackStreams <= 1) {
        GTEST_SKIP() << "maxTransformFeedbackStreams lower than required";
    }

    static char const geom_src[] = R"glsl(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %output1 %_
               OpExecutionMode %main Xfb
               OpExecutionMode %main InputPoints
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputPoints
               OpExecutionMode %main OutputVertices 1

               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4
               OpName %output1 "output1"  ; id %8
               OpName %gl_PerVertex "gl_PerVertex"  ; id %14
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""  ; id %16

               ; Annotations
               OpDecorate %output1 Location 0
               OpDecorate %output1 Stream 1
               OpDecorate %output1 XfbBuffer 0
               OpDecorate %output1 XfbStride 4
               OpDecorate %output1 Offset 0
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpDecorate %_ Stream 0
               OpDecorate %_ XfbBuffer 0
               OpDecorate %_ XfbStride 4

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
    %output1 = OpVariable %_ptr_Output_float Output
    %float_1 = OpConstant %float 1
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %19 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpStore %output1 %float_1
         %21 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %21 %19
               OpEmitVertex
               OpReturn
               OpFunctionEnd
    )glsl";

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkShaderModuleCreateInfo-pCode-08740");
    VkShaderObj gs(this, geom_src, VK_SHADER_STAGE_GEOMETRY_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-geometryStreams-02321");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGeometryTessellation, MismatchedTessellationExecutionModes) {
    TEST_DESCRIPTION("Test mismatched tessellation shaders execution modes");

    RETURN_IF_SKIP(InitFramework());
    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_subset_features = vku::InitStructHelper();
        auto features2 = GetPhysicalDeviceFeatures2(portability_subset_features);
        if (!portability_subset_features.tessellationPointMode) {
            GTEST_SKIP() << "tessellationPointMode not supported";
        }
        if (features2.features.tessellationShader == VK_FALSE) {
            GTEST_SKIP() << "geometryShader not supported";
        }
        features2.features.shaderTessellationAndGeometryPointSize = VK_FALSE;
        RETURN_IF_SKIP(InitState(nullptr, &features2));
    } else {
        VkPhysicalDeviceFeatures features{};
        vk::GetPhysicalDeviceFeatures(Gpu(), &features);
        if (features.tessellationShader == VK_FALSE) {
            GTEST_SKIP() << "geometryShader not supported";
        }
        features.shaderTessellationAndGeometryPointSize = VK_FALSE;
        RETURN_IF_SKIP(InitState(&features));
    }
    if (m_device->Physical().limits_.maxTessellationPatchSize == 0) {
        GTEST_SKIP() << "Tessellation shaders not supported";
    }
    InitRenderTarget();

    std::string vuids[4] = {
        "VUID-VkGraphicsPipelineCreateInfo-pStages-00732",
        "VUID-VkGraphicsPipelineCreateInfo-pStages-00733",
        "VUID-VkGraphicsPipelineCreateInfo-pStages-00734",
        "VUID-VkGraphicsPipelineCreateInfo-pStages-00735",
    };

    std::string tesc_subdivision[4] = {
        "",
        "OpExecutionMode %main Quads",
        "",
        "",
    };

    std::string tese_subdivision[4] = {
        "",
        "OpExecutionMode %main Triangles",
        "OpExecutionMode %main Triangles",
        "OpExecutionMode %main Triangles",
    };

    std::string tesc_output_vertices[4] = {
        "OpExecutionMode %main OutputVertices 4",
        "OpExecutionMode %main OutputVertices 4",
        "",
        "OpExecutionMode %main OutputVertices 4",
    };

    std::string tese_output_vertices[4] = {
        "",
        "",
        "",
        "OpExecutionMode %main OutputVertices 3",
    };

    for (uint32_t i = 0; i < 4; ++i) {
        std::string tesc_source = R"(
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_TessLevelOuter %gl_TessLevelInner
)";
        tesc_source += tesc_output_vertices[i];
        tesc_source += R"(
)";
        tesc_source += tesc_subdivision[i];
        tesc_source += R"(

               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4
               OpName %gl_TessLevelOuter "gl_TessLevelOuter"  ; id %11
               OpName %gl_TessLevelInner "gl_TessLevelInner"  ; id %24

               ; Annotations
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
               OpDecorate %gl_TessLevelInner Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
%_ptr_Output_float = OpTypePointer Output %float
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
         %18 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_2
               OpStore %18 %float_1
         %19 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_1
               OpStore %19 %float_1
         %20 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_0
               OpStore %20 %float_1
         %25 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_0
               OpStore %25 %float_1
               OpReturn
               OpFunctionEnd)";

        std::string tese_source = R"(
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationEvaluation %main "main" %_
)";
        tese_source += tese_output_vertices[i];
        tese_source += R"(
)";
        tese_source += tese_subdivision[i];
        tese_source += R"(
               OpExecutionMode %main SpacingEqual
               OpExecutionMode %main VertexOrderCw
               OpExecutionMode %main PointMode

               ; Debug Information
               OpSource GLSL 460
               OpName %main "main"  ; id %4
               OpName %gl_PerVertex "gl_PerVertex"  ; id %11
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""  ; id %13

               ; Annotations
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %19 %17
               OpReturn
               OpFunctionEnd)";

        VkShaderObj tesc(this, tesc_source.c_str(), VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
        VkShaderObj tese(this, tese_source.c_str(), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, SPV_ENV_VULKAN_1_0,
                         SPV_SOURCE_ASM);

        VkPipelineTessellationStateCreateInfo tess_ci = vku::InitStructHelper();
        tess_ci.patchControlPoints = 4u;

        CreatePipelineHelper pipe(*this);
        pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        pipe.tess_ci_ = tess_ci;
        pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), tesc.GetStageCreateInfo(), tese.GetStageCreateInfo(),
                               pipe.fs_->GetStageCreateInfo()};
        m_errorMonitor->SetDesiredError(vuids[i].c_str());
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGeometryTessellation, WritingToLayerWithSingleFramebufferLayer) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/3019");
    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();  // Creates a framebuffer with a single layer
    if (m_device->Physical().limits_.maxGeometryOutputVertices == 0) {
        GTEST_SKIP() << "Device doesn't support required maxGeometryOutputVertices";
    }
    const std::string_view gsSource = R"glsl(
        #version 450
        layout (triangles) in;
        layout (triangle_strip) out;
        layout (max_vertices = 1) out;
        void main() {
            gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
            EmitVertex();
            gl_Layer = 4;
        }
    )glsl";
    const VkShaderObj gs(this, gsSource.data(), VK_SHADER_STAGE_GEOMETRY_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->SetDesiredWarning("Undefined-Layer-Written");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeGeometryTessellation, DrawDynamicPrimitiveTopology) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8319");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::shaderTessellationAndGeometryPointSize);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    static const char *gsSource = R"glsl(
        #version 450
        layout (points) in;
        layout (triangle_strip) out;
        layout (max_vertices = 3) out;
        in gl_PerVertex
        {
            vec4 gl_Position;
            float gl_PointSize;
        } gl_in[];
        void main()
        {
            gl_Position = gl_in[0].gl_Position;
            gl_PointSize = gl_in[0].gl_PointSize;
            EmitVertex();
        }
    )glsl";

    VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    // Invalid - matches the Geometry output, but we need to match the input
    vk::CmdSetPrimitiveTopologyEXT(m_command_buffer.handle(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-dynamicPrimitiveTopologyUnrestricted-07500");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}