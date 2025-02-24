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

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/render_pass_helper.h"
#include "generated/vk_extension_helper.h"

class PositiveVertexInput : public VkLayerTest {};

TEST_F(PositiveVertexInput, AttributeMatrixType) {
    TEST_DESCRIPTION("Test that pipeline validation accepts matrices passed as vertex attributes");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkVertexInputBindingDescription input_binding;
    memset(&input_binding, 0, sizeof(input_binding));

    VkVertexInputAttributeDescription input_attribs[2];
    memset(input_attribs, 0, sizeof(input_attribs));

    for (int i = 0; i < 2; i++) {
        input_attribs[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        input_attribs[i].location = i;
    }

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in mat2x4 x;
        void main(){
           gl_Position = x[0] + x[1];
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 2;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
    /* expect success */
}

TEST_F(PositiveVertexInput, AttributeArrayType) {
    TEST_DESCRIPTION("Input in OpTypeArray");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkVertexInputBindingDescription input_binding;
    memset(&input_binding, 0, sizeof(input_binding));

    VkVertexInputAttributeDescription input_attribs[2];
    memset(input_attribs, 0, sizeof(input_attribs));

    for (int i = 0; i < 2; i++) {
        input_attribs[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        input_attribs[i].location = i;
    }

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in vec4 x[2];
        void main(){
           gl_Position = x[0] + x[1];
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 2;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, AttributeStructType) {
    TEST_DESCRIPTION("Input is OpTypeStruct");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkVertexInputBindingDescription input_binding = {0, 32, VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription input_attrib;
    memset(&input_attrib, 0, sizeof(input_attrib));
    input_attrib.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    input_attrib.location = 4;

    // This is not valid GLSL (but is valid SPIR-V) - would look like:
    //     in VertexIn {
    //         layout(location = 4) vec4 x;
    //     } x_struct;
    char const *vsSource = R"(
               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Vertex %1 "main" %2
               OpMemberDecorate %_struct_3 0 Location 4
               OpDecorate %_struct_3 Block
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
  %_struct_3 = OpTypeStruct %v4float
%_ptr_Input__struct_3 = OpTypePointer Input %_struct_3
          %2 = OpVariable %_ptr_Input__struct_3 Input
          %1 = OpFunction %void None %5
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, AttributeStructTypeWithArray) {
    TEST_DESCRIPTION("Input is OpTypeStruct that has an OpTypeArray. Locations are not in order netiher");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkVertexInputBindingDescription input_binding = {0, 48, VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription input_attribs[3];
    memset(input_attribs, 0, sizeof(input_attribs));
    input_attribs[0].location = 1;
    input_attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;

    input_attribs[1].location = 4;
    input_attribs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;

    input_attribs[2].location = 5;
    input_attribs[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;

    // This is not valid GLSL (but is valid SPIR-V) - would look like:
    //     in VertexIn {
    //         layout(location = 4) vec4 y[2];
    //         layout(location = 1) vec3 x;
    //     } x_struct;
    char const *vsSource = R"(
               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Vertex %1 "main" %2
               OpMemberDecorate %_struct_3 0 Location 4
               OpMemberDecorate %_struct_3 1 Location 1
               OpDecorate %_struct_3 Block
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
 %array_vec4 = OpTypeArray %v3float %uint_2
  %_struct_3 = OpTypeStruct %array_vec4 %v4float
%_ptr_Input__struct_3 = OpTypePointer Input %_struct_3
          %2 = OpVariable %_ptr_Input__struct_3 Input
          %1 = OpFunction %void None %5
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 3;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, AttributeStructTypeSecondLocation) {
    TEST_DESCRIPTION("Input is OpTypeStruct with 2 locations");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkVertexInputBindingDescription input_binding = {0, 24, VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription input_attribs[2] = {
        {4, 0, VK_FORMAT_R32G32B32A32_SINT, 0},
        {6, 0, VK_FORMAT_R32G32B32A32_UINT, 0},
    };

    // This is not valid GLSL (but is valid SPIR-V) - would look like:
    //     in VertexIn {
    //         layout(location = 4) ivec4 x;
    //         layout(location = 6) uvec4 y;
    //     } x_struct;
    char const *vsSource = R"(
               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Vertex %1 "main" %2
               OpMemberDecorate %_struct_3 0 Location 4
               OpMemberDecorate %_struct_3 1 Location 6
               OpDecorate %_struct_3 Block
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %sint = OpTypeInt 32 1
      %uint  = OpTypeInt 32 0
     %v4sint = OpTypeVector %sint 4
     %v4uint = OpTypeVector %uint 4
  %_struct_3 = OpTypeStruct %v4sint %v4uint
%_ptr_Input__struct_3 = OpTypePointer Input %_struct_3
          %2 = OpVariable %_ptr_Input__struct_3 Input
          %1 = OpFunction %void None %5
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 2;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, AttributeStructTypeBlockLocation) {
    TEST_DESCRIPTION("Input is OpTypeStruct where the Block has the Location");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkVertexInputBindingDescription input_binding = {0, 24, VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription input_attribs[2] = {
        {4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
        {5, 0, VK_FORMAT_R32G32B32A32_UINT, 0},
    };

    // This is not valid GLSL (but is valid SPIR-V) - would look like:
    //     layout(location = 4) in VertexIn {
    //         vec4 x;
    //         uvec4 y;
    //     } x_struct;
    char const *vsSource = R"(
               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Vertex %1 "main" %2
               OpDecorate %_struct_3 Block
               OpDecorate %2 Location 4
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
      %float = OpTypeFloat 32
      %uint  = OpTypeInt 32 0
    %v4float = OpTypeVector %float 4
     %v4uint = OpTypeVector %uint 4
  %_struct_3 = OpTypeStruct %v4float %v4uint
%_ptr_Input__struct_3 = OpTypePointer Input %_struct_3
          %2 = OpVariable %_ptr_Input__struct_3 Input
          %1 = OpFunction %void None %5
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 2;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, AttributeComponents) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts consuming a vertex attribute through multiple vertex shader inputs, each consuming "
        "a different subset of the components, and that fragment shader-attachment validation tolerates multiple duplicate "
        "location outputs");
    AddRequiredFeature(vkt::Feature::independentBlend);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkVertexInputBindingDescription input_binding;
    memset(&input_binding, 0, sizeof(input_binding));

    VkVertexInputAttributeDescription input_attribs[3];
    memset(input_attribs, 0, sizeof(input_attribs));

    for (int i = 0; i < 3; i++) {
        input_attribs[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        input_attribs[i].location = i;
    }

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in vec4 x;
        layout(location=1) in vec3 y1;
        layout(location=1, component=3) in float y2;
        layout(location=2) in vec4 z;
        void main(){
           gl_Position = x + vec4(y1, y2) + z;
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0, component=0) out float color0;
        layout(location=0, component=1) out float color1;
        layout(location=0, component=2) out float color2;
        layout(location=0, component=3) out float color3;
        layout(location=1, component=0) out vec2 second_color0;
        layout(location=1, component=2) out vec2 second_color1;
        void main(){
           color0 = float(1);
           second_color0 = vec2(1);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Create a renderPass with two color attachments
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentDescription(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddColorAttachment(1);
    rp.CreateRenderPass();

    VkPipelineColorBlendAttachmentState cb_attachments[2];
    memset(cb_attachments, 0, sizeof(VkPipelineColorBlendAttachmentState) * 2);
    cb_attachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    cb_attachments[0].blendEnable = VK_FALSE;

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.cb_ci_.attachmentCount = 2;
    pipe.cb_ci_.pAttachments = cb_attachments;
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 3;
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, CreatePipeline64BitAttributes) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts basic use of 64bit vertex attributes. This is interesting because they consume "
        "multiple locations.");

    AddRequiredFeature(vkt::Feature::shaderFloat64);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), VK_FORMAT_R64G64B64A64_SFLOAT, &format_props);
    if (!(format_props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT)) {
        GTEST_SKIP() << "Device does not support VK_FORMAT_R64G64B64A64_SFLOAT vertex buffers";
    }

    VkVertexInputBindingDescription input_bindings[1];
    memset(input_bindings, 0, sizeof(input_bindings));

    VkVertexInputAttributeDescription input_attribs[4];
    memset(input_attribs, 0, sizeof(input_attribs));
    input_attribs[0].location = 0;
    input_attribs[0].offset = 0;
    input_attribs[0].format = VK_FORMAT_R64G64B64A64_SFLOAT;
    input_attribs[1].location = 2;
    input_attribs[1].offset = 32;
    input_attribs[1].format = VK_FORMAT_R64G64B64A64_SFLOAT;
    input_attribs[2].location = 4;
    input_attribs[2].offset = 64;
    input_attribs[2].format = VK_FORMAT_R64G64B64A64_SFLOAT;
    input_attribs[3].location = 6;
    input_attribs[3].offset = 96;
    input_attribs[3].format = VK_FORMAT_R64G64B64A64_SFLOAT;

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in dmat4 x;
        void main(){
           gl_Position = vec4(x[0][0]);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pVertexBindingDescriptions = input_bindings;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 4;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, VertexAttribute64bit) {
    TEST_DESCRIPTION("Use 64-bit Vertex format");

    AddRequiredFeature(vkt::Feature::shaderFloat64);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const VkFormat format = VK_FORMAT_R64_SFLOAT;
    if ((m_device->FormatFeaturesBuffer(format) & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
        GTEST_SKIP() << "Format not supported for Vertex Buffer";
    }

    vkt::Buffer vtx_buf(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    char const *vsSource = R"glsl(
        #version 450 core
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        layout(location = 0) in float64_t pos;
        void main() {}
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    VkVertexInputBindingDescription input_binding = {0, 0, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attribs = {0, 0, format, 0};

    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, AttributeStructTypeBlockLocation64bit) {
    TEST_DESCRIPTION("Input is OpTypeStruct where the Block has the Location with 64-bit Vertex format");

    AddRequiredFeature(vkt::Feature::shaderFloat64);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), VK_FORMAT_R64G64B64A64_SFLOAT, &format_props);
    if (!(format_props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT)) {
        GTEST_SKIP() << "Device does not support VK_FORMAT_R64G64B64A64_SFLOAT vertex buffers";
    }

    VkVertexInputBindingDescription input_binding = {0, 24, VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription input_attribs[3] = {
        {4, 0, VK_FORMAT_R32G32B32A32_SINT, 0},
        {5, 0, VK_FORMAT_R64G64B64A64_SFLOAT, 0},  // takes 2 slots
        {7, 0, VK_FORMAT_R32G32B32A32_SINT, 0},
    };

    // This is not valid GLSL (but is valid SPIR-V) - would look like:
    //     layout(location = 4) in VertexIn {
    //         ivec4 x;
    //         float64 y;
    //         ivec4 z;
    //     } x_struct;
    char const *vsSource = R"(
               OpCapability Shader
               OpCapability Float64
               OpMemoryModel Logical Simple
               OpEntryPoint Vertex %1 "main" %2
               OpDecorate %_struct_3 Block
               OpDecorate %2 Location 4
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
    %float64 = OpTypeFloat 64
      %sint  = OpTypeInt 32 1
  %v4float64 = OpTypeVector %float64 4
     %v4sint = OpTypeVector %sint 4
  %_struct_3 = OpTypeStruct %v4sint %v4float64 %v4sint
%_ptr_Input__struct_3 = OpTypePointer Input %_struct_3
          %2 = OpVariable %_ptr_Input__struct_3 Input
          %1 = OpFunction %void None %5
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 3;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, Attribute64bitMissingComponent) {
    TEST_DESCRIPTION("Shader uses f64vec2, but provides too many component with R64G64B64A64, which is valid");

    AddRequiredFeature(vkt::Feature::shaderFloat64);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const VkFormat format = VK_FORMAT_R64G64B64A64_SFLOAT;
    if ((m_device->FormatFeaturesBuffer(format) & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
        GTEST_SKIP() << "Format not supported for Vertex Buffer";
    }

    char const *vsSource = R"glsl(
        #version 450 core
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        layout(location = 0) in f64vec2 pos;
        void main() {}
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    VkVertexInputBindingDescription input_binding = {0, 32, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attribs = {0, 0, format, 0};

    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};

    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, VertexAttributeDivisorFirstInstance) {
    TEST_DESCRIPTION("Test VK_EXT_vertex_attribute_divisor with non zero first instance");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexAttributeInstanceRateDivisor);
    AddRequiredFeature(vkt::Feature::vertexAttributeInstanceRateZeroDivisor);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT pdvad_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(pdvad_props);

    VkVertexInputBindingDivisorDescription vibdd = {};
    vibdd.divisor = 1;
    VkPipelineVertexInputDivisorStateCreateInfo pvids_ci = vku::InitStructHelper();
    pvids_ci.vertexBindingDivisorCount = 1;
    pvids_ci.pVertexBindingDivisors = &vibdd;
    VkVertexInputBindingDescription vibd = {};
    vibd.stride = 16;
    vibd.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    if (pdvad_props.maxVertexAttribDivisor < pvids_ci.vertexBindingDivisorCount) {
        GTEST_SKIP() << "This device does not support vertexBindingDivisors";
    }

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.pNext = &pvids_ci;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &vibd;
    pipe.CreateGraphicsPipeline();

    vkt::Buffer vertex_buffer(*m_device, 256, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offset = 0u;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0u, 1u, &vertex_buffer.handle(), &offset);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3u, 1u, 0u, 1u);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveVertexInput, VertextBindingNonLinear) {
    TEST_DESCRIPTION("Have Binding not be in a linear order");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    VkVertexInputBindingDescription vtx_binding_des[3] = {
        {3, 0, VK_VERTEX_INPUT_RATE_VERTEX}, {5, 0, VK_VERTEX_INPUT_RATE_VERTEX}, {2, 0, VK_VERTEX_INPUT_RATE_VERTEX}};

    VkVertexInputAttributeDescription vtx_attri_des[3] = {
        {0, 5, VK_FORMAT_R8G8B8A8_UNORM, 0}, {1, 3, VK_FORMAT_R8G8B8A8_UNORM, 0}, {2, 2, VK_FORMAT_R8G8B8A8_UNORM, 0}};
    pipe.vi_ci_.vertexBindingDescriptionCount = 3;
    pipe.vi_ci_.pVertexBindingDescriptions = vtx_binding_des;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 3;
    pipe.vi_ci_.pVertexAttributeDescriptions = vtx_attri_des;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    VkDeviceSize offsets[6] = {0, 0, 0, 0, 0, 0};
    VkBuffer buffers[6] = {buffer.handle(), buffer.handle(), buffer.handle(), buffer.handle(), buffer.handle(), buffer.handle()};
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 6, buffers, offsets);

    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveVertexInput, VertextBindingDynamicState) {
    TEST_DESCRIPTION("Test bad binding with VK_DYNAMIC_STATE_VERTEX_INPUT_EXT");
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexInputDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.CreateGraphicsPipeline();

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offsets[2] = {0, 0};

    VkVertexInputBindingDescription2EXT bindings[3] = {vku::InitStructHelper(), vku::InitStructHelper(), vku::InitStructHelper()};
    bindings[0].binding = 3;
    bindings[0].divisor = 1;
    bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindings[1].binding = 5;
    bindings[1].divisor = 1;
    bindings[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindings[2].binding = 2;
    bindings[2].divisor = 1;
    bindings[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription2EXT attributes[3] = {vku::InitStructHelper(), vku::InitStructHelper(),
                                                           vku::InitStructHelper()};
    attributes[0].location = 1;
    attributes[0].binding = 3;
    attributes[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attributes[1].location = 2;
    attributes[1].binding = 5;
    attributes[1].format = VK_FORMAT_R8G8B8A8_UNORM;
    attributes[2].location = 3;
    attributes[2].binding = 2;
    attributes[2].format = VK_FORMAT_R8G8B8A8_UNORM;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    VkBuffer buffers[2] = {buffer.handle(), buffer.handle()};
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 5, 2, buffers, offsets);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 3, bindings, 3, attributes);
    // set later, shouldn't matter
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 2, 2, buffers, offsets);

    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 1);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveVertexInput, VertexStrideDynamicStride) {
    TEST_DESCRIPTION("set the Stride to fix bad stride in vkCmdBindVertexBuffers2");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    VkVertexInputBindingDescription bindings = {0, 3, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription attributes = {0, 0, VK_FORMAT_R16_UNORM, 0};

    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &bindings;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &attributes;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkDeviceSize offset = 0;
    VkDeviceSize good_stride = 4;
    vk::CmdBindVertexBuffers2EXT(m_command_buffer.handle(), 0, 1, &buffer.handle(), &offset, nullptr, &good_stride);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveVertexInput, VertexStrideDoubleDynamicStride) {
    TEST_DESCRIPTION("set the Stride to invalid, then valid");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexInputDynamicState);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    pipe.CreateGraphicsPipeline();

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkVertexInputBindingDescription2EXT binding = vku::InitStructHelper();
    binding.binding = 0;
    binding.divisor = 1;
    binding.stride = 4;
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription2EXT attribute = vku::InitStructHelper();
    attribute.location = 0;
    attribute.binding = 0;
    attribute.format = VK_FORMAT_R16_UNORM;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    const VkDeviceSize offset = 0;
    const VkDeviceSize bad_stride = 3;
    const VkDeviceSize good_stride = 4;

    vk::CmdBindVertexBuffers2EXT(m_command_buffer.handle(), 0, 1, &buffer.handle(), &offset, nullptr, &bad_stride);
    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 1, &binding, 1, &attribute);  // set to valid
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);

    // flip order around
    binding.stride = static_cast<uint32_t>(bad_stride);
    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 1, &binding, 1, &attribute);
    vk::CmdBindVertexBuffers2EXT(m_command_buffer.handle(), 0, 1, &buffer.handle(), &offset, nullptr, &good_stride);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveVertexInput, InputBindingMaxVertexInputBindingStrideDynamic) {
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Test when stride is greater than VkPhysicalDeviceLimits::maxVertexInputBindingStride.
    VkVertexInputBindingDescription vertex_input_binding_description{};
    vertex_input_binding_description.stride = m_device->Physical().limits_.maxVertexInputBindingStride + 1;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
        helper.vi_ci_.pVertexBindingDescriptions = &vertex_input_binding_description;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveVertexInput, BindVertexBufferNull) {
    TEST_DESCRIPTION("Have null vertex but use nullDescriptor feature");
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    VkVertexInputBindingDescription bindings = {0, 4, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription attributes = {0, 0, VK_FORMAT_R8G8B8A8_UNORM, 0};
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &bindings;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &attributes;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkDeviceSize offsets[2] = {0, 0};
    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkBuffer buffers[2] = {buffer.handle(), VK_NULL_HANDLE};
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 2, buffers, offsets);

    // only uses first binding
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveVertexInput, InterleavedAttributes) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7892");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Buffer vtx_buf(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    // Has item in Binding = 1 which we never bind with CmdBindVertexBuffers
    CreatePipelineHelper pipe0(*this);
    VkVertexInputBindingDescription vtx_binding_des[2] = {{0, 12, VK_VERTEX_INPUT_RATE_VERTEX},
                                                          {1, 12, VK_VERTEX_INPUT_RATE_VERTEX}};
    VkVertexInputAttributeDescription vtx_attri_des[2] = {{0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
                                                          {1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0}};
    pipe0.vi_ci_.vertexBindingDescriptionCount = 2;
    pipe0.vi_ci_.pVertexBindingDescriptions = vtx_binding_des;
    pipe0.vi_ci_.vertexAttributeDescriptionCount = 2;
    pipe0.vi_ci_.pVertexAttributeDescriptions = vtx_attri_des;
    pipe0.CreateGraphicsPipeline();

    vtx_attri_des[1].binding = 0;
    CreatePipelineHelper pipe1(*this);
    pipe1.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe1.vi_ci_.pVertexBindingDescriptions = vtx_binding_des;
    pipe1.vi_ci_.vertexAttributeDescriptionCount = 2;
    pipe1.vi_ci_.pVertexAttributeDescriptions = vtx_attri_des;
    pipe1.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    // We bind, but rebind with valid pipeline
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe0.Handle());  // invalid
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.Handle());  // valid
    VkDeviceSize offset = 0;
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vtx_buf.handle(), &offset);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveVertexInput, LegacyVertexAttributes) {
    AddRequiredExtensions(VK_EXT_LEGACY_VERTEX_ATTRIBUTES_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::legacyVertexAttributes);
    AddRequiredFeature(vkt::Feature::vertexInputDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in int x; /* attrib provided float */
        void main(){
           gl_Position = vec4(x);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    VkVertexInputBindingDescription2EXT binding = vku::InitStructHelper();
    binding.binding = 0;
    binding.stride = 4;
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding.divisor = 1;
    VkVertexInputAttributeDescription2EXT attribute = vku::InitStructHelper();
    attribute.location = 0;
    attribute.binding = 0;
    attribute.format = VK_FORMAT_R32_SFLOAT;
    attribute.offset = 0;

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offset = 0;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &buffer.handle(), &offset);
    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 1, &binding, 1, &attribute);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveVertexInput, ResetCmdSetVertexInput) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8523");
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexInputDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vs_source_int = R"glsl(
        #version 450
        layout(location=0) in uvec4 x;
        void main(){}
    )glsl";
    VkShaderObj vs_int(this, vs_source_int, VK_SHADER_STAGE_VERTEX_BIT);

    char const *vs_source_float = R"glsl(
        #version 450
        layout(location=0) in vec4 x;
        void main(){}
    )glsl";
    VkShaderObj vs_float(this, vs_source_float, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe_int(*this);
    pipe_int.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe_int.shader_stages_ = {vs_int.GetStageCreateInfo(), pipe_int.fs_->GetStageCreateInfo()};
    pipe_int.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_float(*this);
    pipe_float.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe_float.shader_stages_ = {vs_float.GetStageCreateInfo(), pipe_float.fs_->GetStageCreateInfo()};
    pipe_float.CreateGraphicsPipeline();

    vkt::Buffer vertex_buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offset = 0;

    VkVertexInputBindingDescription2EXT bindings = vku::InitStructHelper();
    bindings.binding = 0;
    bindings.divisor = 1;
    bindings.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription2EXT attributes = vku::InitStructHelper();
    attributes.location = 0;
    attributes.binding = 0;
    attributes.format = VK_FORMAT_R8G8B8A8_UINT;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0u, 1u, &vertex_buffer.handle(), &offset);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_int.Handle());
    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 1, &bindings, 1, &attributes);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 1);
    m_command_buffer.EndRenderPass();

    attributes.format = VK_FORMAT_R8G8B8A8_UNORM;
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_float.Handle());
    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 1, &bindings, 1, &attributes);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 1);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveVertexInput, VertexAttributeRobustness) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_ROBUSTNESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexAttributeRobustness);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vs_source = R"glsl(
        #version 450
        layout(location=0) in vec4 x; /* not provided */
        void main(){
           gl_Position = x;
        }
    )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveVertexInput, VertexAttributeRobustnessDynamic) {
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_ROBUSTNESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexInputDynamicState);
    AddRequiredFeature(vkt::Feature::vertexAttributeRobustness);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location = 0) in vec4 x;
        layout(location = 1) in vec4 y;
        layout(location = 0) out vec4 c;
        void main() {
           c = x * y;
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offset = 0u;
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0u, 1u, &buffer.handle(), &offset);

    VkVertexInputBindingDescription2EXT vi_binding_description = vku::InitStructHelper();
    vi_binding_description.binding = 0u;
    vi_binding_description.stride = sizeof(float) * 4;
    vi_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vi_binding_description.divisor = 1u;
    VkVertexInputAttributeDescription2EXT vi_attribute_description = vku::InitStructHelper();
    vi_attribute_description.location = 0u;
    vi_attribute_description.binding = 0u;
    vi_attribute_description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vi_attribute_description.offset = 0u;
    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 1u, &vi_binding_description, 1u, &vi_attribute_description);

    vk::CmdDraw(m_command_buffer.handle(), 4u, 1u, 0u, 0u);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveVertexInput, VertexInputRebinding) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9027");
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexInputDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location = 0) in float a;

        void main(){
            gl_Position = vec4(a);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    VkVertexInputBindingDescription2EXT bindings[2];
    bindings[0] = vku::InitStructHelper();
    bindings[0].binding = 0u;
    bindings[0].stride = sizeof(uint32_t);
    bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindings[0].divisor = 1u;
    bindings[1] = bindings[0];
    bindings[1].binding = 1u;

    VkVertexInputAttributeDescription2EXT attributes[2];
    attributes[0] = vku::InitStructHelper();
    attributes[0].location = 1u;
    attributes[0].binding = 0u;
    attributes[0].format = VK_FORMAT_R32_SFLOAT;
    attributes[0].offset = 0;

    attributes[1] = vku::InitStructHelper();
    attributes[1].location = 0u;
    attributes[1].binding = 1u;
    attributes[1].format = VK_FORMAT_R32_SINT;
    attributes[1].offset = 0;

    vkt::Buffer vertex_buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offsets[2] = {0, 0};
    VkBuffer buffers[2] = {vertex_buffer.handle(), vertex_buffer.handle()};

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 2, bindings, 2, attributes);

    attributes[0].location = 0;
    attributes[0].format = VK_FORMAT_R32_SFLOAT;
    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 1, bindings, 1, attributes);

    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 2, buffers, offsets);
    vk::CmdDraw(m_command_buffer.handle(), 3u, 3u, 0u, 0u);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveVertexInput, UnusedInputBinding) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9305");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in vec4 x;
        layout(location=1) in vec4 y;
        void main() {
           gl_Position = x + y;
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    // input_binding[1] is not accessed from either attributes
    VkVertexInputBindingDescription input_binding[2] = {{0, 16, VK_VERTEX_INPUT_RATE_VERTEX}, {1, 16, VK_VERTEX_INPUT_RATE_VERTEX}};
    VkVertexInputAttributeDescription input_attributes[2] = {{0, 0, VK_FORMAT_R8G8B8A8_UNORM, 0},
                                                             {1, 0, VK_FORMAT_R8G8B8A8_UNORM, 0}};

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.vertexBindingDescriptionCount = 2;
    pipe.vi_ci_.pVertexBindingDescriptions = input_binding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 2;
    pipe.vi_ci_.pVertexAttributeDescriptions = input_attributes;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    VkDeviceSize offset = 0;
    vkt::Buffer vertex_buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    // Only binding 0 is bound because 1 is ignored
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &offset);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveVertexInput, UnusedInputBindingDynamic) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9305");
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexInputDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in vec4 x;
        layout(location=1) in vec4 y;
        void main() {
           gl_Position = x + y;
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    // bindings[1] is not accessed from either attributes
    VkVertexInputBindingDescription2EXT bindings[2] = {vku::InitStructHelper(), vku::InitStructHelper()};
    bindings[0].binding = 0;
    bindings[0].divisor = 1;
    bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindings[1].binding = 1;
    bindings[1].divisor = 1;
    bindings[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription2EXT attributes[2] = {vku::InitStructHelper(), vku::InitStructHelper()};
    attributes[0].location = 0;
    attributes[0].binding = 0;
    attributes[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attributes[0].offset = 0;
    attributes[1].location = 1;
    attributes[1].binding = 0;
    attributes[1].format = VK_FORMAT_R8G8B8A8_UNORM;
    attributes[1].offset = 0;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    VkDeviceSize offset = 0;
    vkt::Buffer vertex_buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 2, bindings, 2, attributes);
    // Only binding 0 is bound because 1 is ignored
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vertex_buffer.handle(), &offset);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveVertexInput, BindVertexBufferNullDraw) {
    TEST_DESCRIPTION("Have null vertex but use nullDescriptor feature");
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    VkVertexInputBindingDescription bindings = {1, 4, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription attributes = {0, 1, VK_FORMAT_R8G8B8A8_UNORM, 0};
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &bindings;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &attributes;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkDeviceSize offsets[2] = {0, 0};
    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkBuffer buffers[2] = {buffer.handle(), VK_NULL_HANDLE};
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 2, buffers, offsets);

    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 1);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}
