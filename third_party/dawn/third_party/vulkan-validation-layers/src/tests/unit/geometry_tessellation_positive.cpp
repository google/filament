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

class PositiveGeometryTessellation : public VkLayerTest {};

TEST_F(PositiveGeometryTessellation, PointSizeGeomShaderDontWriteMaintenance5) {
    TEST_DESCRIPTION(
        "Create a pipeline using TOPOLOGY_POINT_LIST, set PointSize vertex shader, but not in the final geometry stage, but have maintenance5.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
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

    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveGeometryTessellation, IncompatibleDynamicPrimitiveTopology) {
    TEST_DESCRIPTION("Create pipeline with primitive topology incompatible with shaders, but use VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY.");

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
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveGeometryTessellation, DrawDynamicPrimitiveTopology) {
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
    vk::CmdSetPrimitiveTopologyEXT(m_command_buffer.handle(), VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveGeometryTessellation, TessellationPointMode) {
    TEST_DESCRIPTION("Create pipeline with tessellation evaluation shader using point mode");

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

    static char const geom_src[] = R"glsl(
        #version 450
        layout (points) in;
        layout (points) out;
        layout (max_vertices = 1) out;
        void main() {
            gl_Position = vec4(1.0f, 0.5f, 0.5f, 0.0f);
            gl_PointSize = 1.0f;
            EmitVertex();
        }
    )glsl";

    VkShaderObj tcs(this, kTessellationControlMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, tess_src, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    VkShaderObj gs(this, geom_src, VK_SHADER_STAGE_GEOMETRY_BIT);

    VkPipelineTessellationStateCreateInfo tess_ci = vku::InitStructHelper();
    tess_ci.patchControlPoints = 4u;

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(),
                           gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    pipe.tess_ci_ = tess_ci;
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveGeometryTessellation, InterfaceComponents) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8443");
    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vs_source = R"glsl(
        #version 450
        layout(location = 0) out ivec4 a;
        void main() {
            a = ivec4(1);
        }
    )glsl";

    char const *geom_source = R"glsl(
        #version 450
        layout(triangles) in;
        layout(triangle_strip) out;
        layout(location = 0) in ivec4 a[3];
        layout(location = 0) out vec4 b;
        layout(max_vertices = 3) out;

        void main() {
            b = vec4(a[0]);
            gl_Position = a[0];
            EmitVertex();

            b = vec4(a[1]);
            gl_Position = a[1];
            EmitVertex();

            b = vec4(a[2]);
            gl_Position = a[2];
            EmitVertex();

            EndPrimitive();
        }
    )glsl";

    char const *fs_source = R"glsl(
        #version 450
        layout(location = 0) in vec4 b;
        layout(location = 0) out vec4 c;
        void main() {
            c = b;
        }
    )glsl";

    VkShaderObj vert(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj geom(this, geom_source, VK_SHADER_STAGE_GEOMETRY_BIT);
    VkShaderObj frag(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vert.GetStageCreateInfo(), frag.GetStageCreateInfo(), geom.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}