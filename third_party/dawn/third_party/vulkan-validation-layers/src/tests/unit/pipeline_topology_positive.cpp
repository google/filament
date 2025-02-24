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

class PositivePipelineTopology : public VkLayerTest {};

TEST_F(PositivePipelineTopology, PointSizeWriteInFunction) {
    TEST_DESCRIPTION("Create a pipeline using TOPOLOGY_POINT_LIST and write PointSize in vertex shader function.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Create VS declaring PointSize and write to it in a function call.
    VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj ps(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    {
        CreatePipelineHelper pipe(*this);
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), ps.GetStageCreateInfo()};
        pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        pipe.CreateGraphicsPipeline();
    }
}

TEST_F(PositivePipelineTopology, PointSizeGeomShaderSuccess) {
    TEST_DESCRIPTION(
        "Create a pipeline using TOPOLOGY_POINT_LIST, set PointSize vertex shader, and write in the final geometry stage.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::shaderTessellationAndGeometryPointSize);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Create VS declaring PointSize and writing to it
    VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, kGeometryPointSizeGlsl, VK_SHADER_STAGE_GEOMETRY_BIT);
    VkShaderObj ps(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), ps.GetStageCreateInfo()};
    // Set Input Assembly to TOPOLOGY POINT LIST
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositivePipelineTopology, PointSizeGeomShaderDontEmit) {
    TEST_DESCRIPTION("If vertex is not emitted, don't need Point Size in Geometry shader");

    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::shaderTessellationAndGeometryPointSize);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Never calls OpEmitVertex
    static char const *gsSource = R"glsl(
        #version 450
        layout (points) in;
        layout (points) out;
        layout (max_vertices = 1) out;
        void main() {
           gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
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

TEST_F(PositivePipelineTopology, LoosePointSizeWrite) {
    TEST_DESCRIPTION("Create a pipeline using TOPOLOGY_POINT_LIST and write PointSize outside of a structure.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *LoosePointSizeWrite = R"(
                                       OpCapability Shader
                                  %1 = OpExtInstImport "GLSL.std.450"
                                       OpMemoryModel Logical GLSL450
                                       OpEntryPoint Vertex %main "main" %glposition %glpointsize %gl_VertexIndex
                                       OpSource GLSL 450
                                       OpName %main "main"
                                       OpName %vertices "vertices"
                                       OpName %glposition "glposition"
                                       OpName %glpointsize "glpointsize"
                                       OpName %gl_VertexIndex "gl_VertexIndex"
                                       OpDecorate %glposition BuiltIn Position
                                       OpDecorate %glpointsize BuiltIn PointSize
                                       OpDecorate %gl_VertexIndex BuiltIn VertexIndex
                               %void = OpTypeVoid
                                  %3 = OpTypeFunction %void
                              %float = OpTypeFloat 32
                            %v2float = OpTypeVector %float 2
                               %uint = OpTypeInt 32 0
                             %uint_3 = OpConstant %uint 3
                %_arr_v2float_uint_3 = OpTypeArray %v2float %uint_3
   %_ptr_Private__arr_v2float_uint_3 = OpTypePointer Private %_arr_v2float_uint_3
                           %vertices = OpVariable %_ptr_Private__arr_v2float_uint_3 Private
                                %int = OpTypeInt 32 1
                              %int_0 = OpConstant %int 0
                           %float_n1 = OpConstant %float -1
                                 %16 = OpConstantComposite %v2float %float_n1 %float_n1
               %_ptr_Private_v2float = OpTypePointer Private %v2float
                              %int_1 = OpConstant %int 1
                            %float_1 = OpConstant %float 1
                                 %21 = OpConstantComposite %v2float %float_1 %float_n1
                              %int_2 = OpConstant %int 2
                            %float_0 = OpConstant %float 0
                                 %25 = OpConstantComposite %v2float %float_0 %float_1
                            %v4float = OpTypeVector %float 4
            %_ptr_Output_gl_Position = OpTypePointer Output %v4float
                         %glposition = OpVariable %_ptr_Output_gl_Position Output
           %_ptr_Output_gl_PointSize = OpTypePointer Output %float
                        %glpointsize = OpVariable %_ptr_Output_gl_PointSize Output
                     %_ptr_Input_int = OpTypePointer Input %int
                     %gl_VertexIndex = OpVariable %_ptr_Input_int Input
                              %int_3 = OpConstant %int 3
                %_ptr_Output_v4float = OpTypePointer Output %v4float
                  %_ptr_Output_float = OpTypePointer Output %float
                               %main = OpFunction %void None %3
                                  %5 = OpLabel
                                 %18 = OpAccessChain %_ptr_Private_v2float %vertices %int_0
                                       OpStore %18 %16
                                 %22 = OpAccessChain %_ptr_Private_v2float %vertices %int_1
                                       OpStore %22 %21
                                 %26 = OpAccessChain %_ptr_Private_v2float %vertices %int_2
                                       OpStore %26 %25
                                 %33 = OpLoad %int %gl_VertexIndex
                                 %35 = OpSMod %int %33 %int_3
                                 %36 = OpAccessChain %_ptr_Private_v2float %vertices %35
                                 %37 = OpLoad %v2float %36
                                 %38 = OpCompositeExtract %float %37 0
                                 %39 = OpCompositeExtract %float %37 1
                                 %40 = OpCompositeConstruct %v4float %38 %39 %float_0 %float_1
                                 %42 = OpAccessChain %_ptr_Output_v4float %glposition
                                       OpStore %42 %40
                                       OpStore %glpointsize %float_1
                                       OpReturn
                                       OpFunctionEnd
        )";

    // Create VS declaring PointSize and write to it in a function call.
    VkShaderObj vs(this, LoosePointSizeWrite, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj ps(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    {
        CreatePipelineHelper pipe(*this);
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), ps.GetStageCreateInfo()};
        // Set Input Assembly to TOPOLOGY POINT LIST
        pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        pipe.CreateGraphicsPipeline();
    }
}

TEST_F(PositivePipelineTopology, PointSizeStructMemberWritten) {
    TEST_DESCRIPTION("Write built-in PointSize within a struct");

    SetTargetApiVersion(VK_API_VERSION_1_1); // At least 1.1 is required for maintenance4
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance4);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const std::string vs_src = R"asm(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %14 %25 %47 %52
               OpSource GLSL 450
               OpMemberDecorate %12 0 BuiltIn Position
               OpMemberDecorate %12 1 BuiltIn PointSize
               OpMemberDecorate %12 2 BuiltIn ClipDistance
               OpMemberDecorate %12 3 BuiltIn CullDistance
               OpDecorate %12 Block
               OpMemberDecorate %18 0 ColMajor
               OpMemberDecorate %18 0 Offset 0
               OpMemberDecorate %18 0 MatrixStride 16
               OpMemberDecorate %18 1 Offset 64
               OpMemberDecorate %18 2 Offset 80
               OpDecorate %18 Block
               OpDecorate %25 Location 0
               OpDecorate %47 Location 1
               OpDecorate %52 Location 0
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %7 = OpTypeFloat 32
          %8 = OpTypeVector %7 4
          %9 = OpTypeInt 32 0
         %10 = OpConstant %9 1
         %11 = OpTypeArray %7 %10
         %12 = OpTypeStruct %8 %7 %11 %11
         %13 = OpTypePointer Output %12
         %14 = OpVariable %13 Output
         %15 = OpTypeInt 32 1
         %16 = OpConstant %15 0
         %17 = OpTypeMatrix %8 4
         %18 = OpTypeStruct %17 %7 %8
         %19 = OpTypePointer PushConstant %18
         %20 = OpVariable %19 PushConstant
         %21 = OpTypePointer PushConstant %17
         %24 = OpTypePointer Input %8
         %25 = OpVariable %24 Input
         %28 = OpTypePointer Output %8
         %30 = OpConstant %7 0.5
         %31 = OpConstant %9 2
         %32 = OpTypePointer Output %7
         %36 = OpConstant %9 3
         %46 = OpConstant %15 1
         %47 = OpVariable %24 Input
         %48 = OpTypePointer Input %7
         %52 = OpVariable %28 Output
         %53 = OpTypeVector %7 3
         %56 = OpConstant %7 1
          %main = OpFunction %3 None %4
          %6 = OpLabel

               ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
               ; For the following, only the _first_ index of the access chain
               ; should be used for output validation, as subsequent indices refer
               ; to individual components within the output variable of interest.
               ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
         %22 = OpAccessChain %21 %20 %16
         %23 = OpLoad %17 %22
         %26 = OpLoad %8 %25
         %27 = OpMatrixTimesVector %8 %23 %26
         %29 = OpAccessChain %28 %14 %16
               OpStore %29 %27
         %33 = OpAccessChain %32 %14 %16 %31
         %34 = OpLoad %7 %33
         %35 = OpFMul %7 %30 %34
         %37 = OpAccessChain %32 %14 %16 %36
         %38 = OpLoad %7 %37
         %39 = OpFMul %7 %30 %38
         %40 = OpFAdd %7 %35 %39
         %41 = OpAccessChain %32 %14 %16 %31
               OpStore %41 %40
         %42 = OpAccessChain %32 %14 %16 %10
         %43 = OpLoad %7 %42
         %44 = OpFNegate %7 %43
         %45 = OpAccessChain %32 %14 %16 %10
               OpStore %45 %44
         %49 = OpAccessChain %48 %47 %36
         %50 = OpLoad %7 %49
         %51 = OpAccessChain %32 %14 %46
               OpStore %51 %50

         %54 = OpLoad %8 %47
         %55 = OpVectorShuffle %53 %54 %54 0 1 2
         %57 = OpCompositeExtract %7 %55 0
         %58 = OpCompositeExtract %7 %55 1
         %59 = OpCompositeExtract %7 %55 2
         %60 = OpCompositeConstruct %8 %57 %58 %59 %56
               OpStore %52 %60
               OpReturn
               OpFunctionEnd
    )asm";
    auto vs = VkShaderObj::CreateFromASM(this, vs_src.c_str(), VK_SHADER_STAGE_VERTEX_BIT);

    if (!vs) {
        GTEST_SKIP() << "Error creating shader from assembly";
    }

    // struct has {
    //     mat4x4
    //     float
    //     vec4
    // }
    // but std140 padding so the vec4 is offset 80
    VkPushConstantRange push_constant_ranges[1]{{VK_SHADER_STAGE_VERTEX_BIT, 0, 96}};

    VkPipelineLayoutCreateInfo const pipeline_layout_info{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, push_constant_ranges};

    VkVertexInputBindingDescription input_binding[2] = {
        {0, 16, VK_VERTEX_INPUT_RATE_VERTEX},
        {1, 16, VK_VERTEX_INPUT_RATE_VERTEX},
    };
    VkVertexInputAttributeDescription input_attribs[2] = {
        {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
        {1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
    };

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs->GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.pipeline_layout_ci_ = pipeline_layout_info;
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    pipe.vi_ci_.pVertexBindingDescriptions = input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 2;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 2;
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositivePipelineTopology, PolygonModeValid) {
    TEST_DESCRIPTION("Verify that using a solid polygon fill mode works correctly.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    std::vector<const char *> device_extension_names;
    auto features = m_device->Physical().Features();
    // Artificially disable support for non-solid fill modes
    features.fillModeNonSolid = false;
    // The sacrificial device object
    vkt::Device test_device(Gpu(), device_extension_names, &features);

    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attach;
    subpass.colorAttachmentCount = 1;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vkt::RenderPass render_pass(test_device, rpci);

    const vkt::PipelineLayout pipeline_layout(test_device);

    VkPipelineRasterizationStateCreateInfo rs_ci = vku::InitStructHelper();
    rs_ci.lineWidth = 1.0f;
    rs_ci.rasterizerDiscardEnable = false;

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    vs.InitFromGLSLTry(&test_device);
    fs.InitFromGLSLTry(&test_device);

    // Set polygonMode=FILL. No error is expected
    {
        CreatePipelineHelper pipe(*this);
        pipe.device_ = &test_device;
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.gp_ci_.layout = pipeline_layout.handle();
        pipe.gp_ci_.renderPass = render_pass.handle();
        // Set polygonMode to a good value
        rs_ci.polygonMode = VK_POLYGON_MODE_FILL;
        pipe.gp_ci_.pRasterizationState = &rs_ci;
        pipe.CreateGraphicsPipeline();
    }
}

TEST_F(PositivePipelineTopology, NotPointSizeGeometry) {
    TEST_DESCRIPTION("Create a pipeline using TOPOLOGY_POINT_LIST, but geometry shader doesn't include PointSize.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    static char const geom_src[] = R"glsl(
        #version 460
        layout(points) in;
        layout(triangle_strip, max_vertices=3) out;
        void main() {
           gl_Position = vec4(1);
           EmitVertex();
        }
    )glsl";

    VkShaderObj gs(this, geom_src, VK_SHADER_STAGE_GEOMETRY_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

    pipe.CreateGraphicsPipeline();
}

TEST_F(PositivePipelineTopology, Rasterizer) {
    TEST_DESCRIPTION("Test topology set when creating a pipeline with tessellation and geometry shader.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(Init());

    InitRenderTarget();

    char const *tcsSource = R"glsl(
        #version 450
        layout(vertices = 3) out;
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
           gl_Position.w = 1.0f;
        }
    )glsl";
    static char const *gsSource = R"glsl(
        #version 450
        layout (triangles) in;
        layout (triangle_strip) out;
        layout (max_vertices = 1) out;
        void main() {
           gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
           EmitVertex();
        }
    )glsl";
    VkShaderObj tcs(this, tcsSource, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, tesSource, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};

    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pTessellationState = &tsci;
    pipe.gp_ci_.pInputAssemblyState = &iasci;
    pipe.shader_stages_.emplace_back(gs.GetStageCreateInfo());
    pipe.shader_stages_.emplace_back(tcs.GetStageCreateInfo());
    pipe.shader_stages_.emplace_back(tes.GetStageCreateInfo());
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(RenderPass(), Framebuffer(), 32, 32, m_renderPassClearValues.size(),
                                     m_renderPassClearValues.data());
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositivePipelineTopology, LineTopologyClasses) {
    TEST_DESCRIPTION("Check different line topologies within the same topology class");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Verify each vkCmdSet command
    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    VkVertexInputBindingDescription inputBinding = {0, sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    pipe.vi_ci_.pVertexBindingDescriptions = &inputBinding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    VkVertexInputAttributeDescription attribute = {0, 0, VK_FORMAT_R32_SFLOAT, 0};
    pipe.vi_ci_.pVertexAttributeDescriptions = &attribute;
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    pipe.CreateGraphicsPipeline();

    vkt::Buffer vbo(*m_device, sizeof(float) * 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    vkt::CommandBuffer cb(*m_device, m_command_pool);
    cb.Begin();
    cb.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(cb.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindVertexBuffers(cb.handle(), 0, 1, &vbo.handle(), &kZeroDeviceSize);
    vk::CmdSetPrimitiveTopologyEXT(cb.handle(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY);
    vk::CmdDraw(cb.handle(), 1, 1, 0, 0);

    cb.EndRenderPass();

    cb.End();
}

TEST_F(PositivePipelineTopology, PointSizeDynamicAndUnestricted) {
    TEST_DESCRIPTION(
        "Create a pipeline using TOPOLOGY_POINT_LIST but do not set PointSize in vertex shader, but it is valid with both dynamic "
        "state and dynamicPrimitiveTopologyUnrestricted is true.");

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT dynamic_state_3_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dynamic_state_3_props);
    if (!dynamic_state_3_props.dynamicPrimitiveTopologyUnrestricted) {
        GTEST_SKIP() << "dynamicPrimitiveTopologyUnrestricted is VK_TRUE";
    }

    static const char *vs_source = R"glsl(
        #version 450
        void main() {
            gl_PointSize = 1.0;
        }
    )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    const VkDynamicState dyn_state = VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY;
    VkPipelineDynamicStateCreateInfo dyn_state_ci = vku::InitStructHelper();
    dyn_state_ci.dynamicStateCount = 1;
    dyn_state_ci.pDynamicStates = &dyn_state;

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        helper.dyn_state_ci_ = dyn_state_ci;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositivePipelineTopology, PointSizeMaintenance5) {
    TEST_DESCRIPTION(
        "Create a pipeline using TOPOLOGY_POINT_LIST but do not set PointSize in vertex shader, but have maintenance5.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char *source = R"glsl(
        #version 450
        vec2 vertices[3];
        out gl_PerVertex
        {
            vec4 gl_Position;
            float gl_PointSize;
        };
        void main() {
            vertices[0] = vec2(-1.0, -1.0);
            vertices[1] = vec2( 1.0, -1.0);
            vertices[2] = vec2( 0.0,  1.0);
            gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";

    VkShaderObj vs(this, source, VK_SHADER_STAGE_VERTEX_BIT);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositivePipelineTopology, PrimitiveTopologyListRestartDynamic) {
    TEST_DESCRIPTION("Ignore invalid primitiveRestartEnable");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PRIMITIVE_TOPOLOGY_LIST_RESTART_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState2);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    pipe.ia_ci_.primitiveRestartEnable = VK_TRUE;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}