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
#include "../framework/descriptor_helper.h"
#include "../framework/render_pass_helper.h"

class PositiveShaderInterface : public VkLayerTest {};

TEST_F(PositiveShaderInterface, InputAndOutputComponents) {
    TEST_DESCRIPTION("Test shader layout in and out with different components.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
                #version 450

                layout(location = 0, component = 0) out vec2 rg;
                layout(location = 0, component = 2) out float b;

                layout(location = 1, component = 0) out float r;
                layout(location = 1, component = 1) out vec3 gba;

                layout(location = 2) out vec4 out_color_0;
                layout(location = 3) out vec4 out_color_1;

                layout(location = 4, component = 0) out float x;
                layout(location = 4, component = 1) out vec2 yz;
                layout(location = 4, component = 3) out float w;

                layout(location = 5, component = 0) out vec3 stp;
                layout(location = 5, component = 3) out float q;

                layout(location = 6, component = 0) out vec2 cd;
                layout(location = 6, component = 2) out float e;
                layout(location = 6, component = 3) out float f;

                layout(location = 7, component = 0) out float ar1;
                layout(location = 7, component = 1) out float ar2[2];
                layout(location = 7, component = 3) out float ar3;

                void main() {
                        vec2 xy = vec2((gl_VertexIndex >> 1u) & 1u, gl_VertexIndex & 1u);
                        gl_Position = vec4(xy, 0.0f, 1.0f);
                        out_color_0 = vec4(1.0f, 0.0f, 1.0f, 0.0f);
                        out_color_1 = vec4(0.0f, 1.0f, 0.0f, 1.0f);
                        rg = vec2(0.25f, 0.75f);
                        b = 0.5f;
                        r = 0.75f;
                        gba = vec3(1.0f);
                        x = 1.0f;
                        yz = vec2(0.25f);
                        w = 0.5f;
                        stp = vec3(1.0f);
                        q = 0.1f;
                        ar1 = 1.0f;
                        ar2[0] = 0.5f;
                        ar2[1] = 0.75f;
                        ar3 = 1.0f;
                }
            )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fsSource = R"glsl(
                #version 450

                layout(location = 0, component = 0) in float r;
                layout(location = 0, component = 1) in vec2 gb;

                layout(location = 1, component = 0) in float r1;
                layout(location = 1, component = 1) in float g1;
                layout(location = 1, component = 2) in float b1;
                layout(location = 1, component = 3) in float a1;

                layout(location = 2) in InputBlock {
                    layout(location = 3, component = 3) float one_alpha;
                    layout(location = 2, component = 3) float zero_alpha;
                    layout(location = 3, component = 2) float one_blue;
                    layout(location = 2, component = 2) float zero_blue;
                    layout(location = 3, component = 1) float one_green;
                    layout(location = 2, component = 1) float zero_green;
                    layout(location = 3, component = 0) float one_red;
                    layout(location = 2, component = 0) float zero_red;
                } inBlock;

                layout(location = 4, component = 0) in vec2 xy;
                layout(location = 4, component = 2) in vec2 zw;

                layout(location = 5, component = 0) in vec4 st;

                layout(location = 6, component = 0) in vec4 cdef;

                layout(location = 7, component = 0) in float ar1;
                layout(location = 7, component = 1) in float ar2;
                layout(location = 8, component = 1) in float ar3;
                layout(location = 7, component = 3) in float ar4;

                layout (location = 0) out vec4 color;

                void main() {
                    color = vec4(r, gb, 1.0f) *
                            vec4(r1, g1, 1.0f, a1) *
                            vec4(inBlock.zero_red, inBlock.zero_green, inBlock.zero_blue, inBlock.zero_alpha) *
                            vec4(inBlock.one_red, inBlock.one_green, inBlock.one_blue, inBlock.one_alpha) *
                            vec4(xy, zw) * st * cdef * vec4(ar1, ar2, ar3, ar4);
                }
            )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderInterface, InputAndOutputStructComponents) {
    TEST_DESCRIPTION("Test shader interface with structs.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
                #version 450

                struct R {
                    vec4 rgba;
                };

                layout(location = 0) out R color[3];

                void main() {
                    color[0].rgba = vec4(1.0f);
                    color[1].rgba = vec4(0.5f);
                    color[2].rgba = vec4(0.75f);
                }
            )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fsSource = R"glsl(
                #version 450

                struct R {
                    vec4 rgba;
                };

                layout(location = 0) in R inColor[3];

                layout (location = 0) out vec4 color;

                void main() {
                    color = inColor[0].rgba * inColor[1].rgba * inColor[2].rgba;
                }
            )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderInterface, RelaxedBlockLayout) {
    // This is a positive test, no errors expected
    // Verifies the ability to relax block layout rules with a shader that requires them to be relaxed
    TEST_DESCRIPTION("Create a shader that requires relaxed block layout.");

    AddRequiredExtensions(VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Vertex shader requiring relaxed layout.
    // Without relaxed layout, we would expect a message like:
    // "Structure id 2 decorated as Block for variable in Uniform storage class
    // must follow standard uniform buffer layout rules: member 1 at offset 4 is not aligned to 16"

    const char *spv_source = R"(
                  OpCapability Shader
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint Vertex %main "main"
                  OpSource GLSL 450
                  OpMemberDecorate %S 0 Offset 0
                  OpMemberDecorate %S 1 Offset 4
                  OpDecorate %S Block
                  OpDecorate %B DescriptorSet 0
                  OpDecorate %B Binding 0
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
       %v3float = OpTypeVector %float 3
             %S = OpTypeStruct %float %v3float
%_ptr_Uniform_S = OpTypePointer Uniform %S
             %B = OpVariable %_ptr_Uniform_S Uniform
          %main = OpFunction %void None %3
             %5 = OpLabel
                  OpReturn
                  OpFunctionEnd
        )";
    VkShaderObj vs(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
}

TEST_F(PositiveShaderInterface, UboStd430Layout) {
    // This is a positive test, no errors expected
    // Verifies the ability to scalar block layout rules with a shader that requires them to be relaxed
    TEST_DESCRIPTION("Create a shader that requires UBO std430 layout.");
    AddRequiredExtensions(VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::uniformBufferStandardLayout);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Vertex shader requiring std430 in a uniform buffer.
    // Without uniform buffer standard layout, we would expect a message like:
    // "Structure id 3 decorated as Block for variable in Uniform storage class
    // must follow standard uniform buffer layout rules: member 0 is an array
    // with stride 4 not satisfying alignment to 16"

    const char *spv_source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 460
               OpDecorate %_arr_float_uint_8 ArrayStride 4
               OpMemberDecorate %foo 0 Offset 0
               OpDecorate %foo Block
               OpDecorate %b DescriptorSet 0
               OpDecorate %b Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_float_uint_8 = OpTypeArray %float %uint_8
        %foo = OpTypeStruct %_arr_float_uint_8
%_ptr_Uniform_foo = OpTypePointer Uniform %foo
          %b = OpVariable %_ptr_Uniform_foo Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT);
}

TEST_F(PositiveShaderInterface, ScalarBlockLayout) {
    // This is a positive test, no errors expected
    // Verifies the ability to scalar block layout rules with a shader that requires them to be relaxed
    TEST_DESCRIPTION("Create a shader that requires scalar block layout.");
    AddRequiredExtensions(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::scalarBlockLayout);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Vertex shader requiring scalar layout.
    // Without scalar layout, we would expect a message like:
    // "Structure id 2 decorated as Block for variable in Uniform storage class
    // must follow standard uniform buffer layout rules: member 1 at offset 4 is not aligned to 16"

    const char *spv_source = R"(
                  OpCapability Shader
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint Vertex %main "main"
                  OpSource GLSL 450
                  OpMemberDecorate %S 0 Offset 0
                  OpMemberDecorate %S 1 Offset 4
                  OpMemberDecorate %S 2 Offset 8
                  OpDecorate %S Block
                  OpDecorate %B DescriptorSet 0
                  OpDecorate %B Binding 0
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
       %v3float = OpTypeVector %float 3
             %S = OpTypeStruct %float %float %v3float
%_ptr_Uniform_S = OpTypePointer Uniform %S
             %B = OpVariable %_ptr_Uniform_S Uniform
          %main = OpFunction %void None %3
             %5 = OpLabel
                  OpReturn
                  OpFunctionEnd
        )";

    VkShaderObj vs(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
}

TEST_F(PositiveShaderInterface, FragmentOutputNotWrittenButMasked) {
    TEST_DESCRIPTION(
        "Test that no error is produced when the fragment shader fails to declare an output, but the corresponding attachment's "
        "write mask is 0.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        void main() {}
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderInterface, RelaxedTypeMatch) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts the relaxed type matching rules set out in VK_KHR_maintenance4 (default in Vulkan "
        "1.3) device extension:"
        "fundamental type must match, and producer side must have at least as many components");

    SetTargetApiVersion(VK_API_VERSION_1_1); // At least 1.1 is required for maintenance4
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance4);
    RETURN_IF_SKIP(Init());

    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out vec3 x;
        layout(location=1) out ivec3 y;
        layout(location=2) out vec3 z;
        void main(){
           gl_Position = vec4(0);
           x = vec3(0); y = ivec3(0); z = vec3(0);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        layout(location=0) in float x;
        layout(location=1) flat in int y;
        layout(location=2) in vec2 z;
        void main(){
           color = vec4(1 + x + y + z.x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderInterface, TessPerVertex) {
    TEST_DESCRIPTION("Test that pipeline validation accepts per-vertex variables passed between the TCS and TES stages");

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
        layout(location=0) in int x[];
        void main(){
           gl_Position.xyz = gl_TessCoord;
           gl_Position.w = x[0] + x[1] + x[2];
        }
    )glsl";

    VkShaderObj vs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj tcs(this, tcsSource, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, tesSource, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};

    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pTessellationState = &tsci;
    pipe.gp_ci_.pInputAssemblyState = &iasci;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderInterface, GeometryInputBlockPositive) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts a user-defined interface block passed into the geometry shader. This is interesting "
        "because the 'extra' array level is not present on the member type, but on the block instance.");

    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450

        layout(location = 0) out VertexData { vec4 x; } gs_out;

        void main(){
           gs_out.x = vec4(1.0f);
        }
    )glsl";

    char const *gsSource = R"glsl(
        #version 450
        layout(triangles) in;
        layout(triangle_strip, max_vertices=3) out;
        layout(location=0) in VertexData { vec4 x; } gs_in[];
        void main() {
           gl_Position = gs_in[0].x;
           EmitVertex();
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderInterface, InputAttachment) {
    TEST_DESCRIPTION("Positive test for a correctly matched input attachment");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;
        layout(location=0) out vec4 color;
        void main() {
           color = subpassLoad(x);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddInputAttachment(1);
    rp.CreateRenderPass();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderInterface, InputAttachmentMissingNotRead) {
    TEST_DESCRIPTION("Input Attachment would be missing, but it is not read from in shader");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput xs[1];
    // layout(location=0) out vec4 color;
    // void main() {
    //     // (not actually called) color = subpassLoad(xs[0]);
    // }
    const char *fsSource = R"(
               OpCapability Shader
               OpCapability InputAttachment
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %color
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %color Location 0
               OpDecorate %xs DescriptorSet 0
               OpDecorate %xs Binding 0
               OpDecorate %xs InputAttachmentIndex 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %color = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float SubpassData 0 0 0 2 Unknown
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_10_uint_1 = OpTypeArray %10 %uint_1
%_ptr_UniformConstant__arr_10_uint_1 = OpTypePointer UniformConstant %_arr_10_uint_1
         %xs = OpVariable %_ptr_UniformConstant__arr_10_uint_1 UniformConstant
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
      %v2int = OpTypeVector %int 2
         %22 = OpConstantComposite %v2int %int_0 %int_0
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd)";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderInterface, InputAttachmentArray) {
    TEST_DESCRIPTION("Input Attachment array where need to follow the index into the array");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    RETURN_IF_SKIP(Init());

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(m_render_target_fmt);
    // index 0 is unused
    rp.AddAttachmentReference({VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_GENERAL});
    // index 1 is is valid (for both color and input)
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    // index 2 and 3 point to same image as index 1
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddInputAttachment(0);
    rp.AddInputAttachment(1);
    rp.AddInputAttachment(2);
    rp.AddInputAttachment(3);
    rp.AddColorAttachment(1);
    rp.CreateRenderPass();

    // use static array of 2 and index into element 1 to read
    {
        const char *fs_source = R"glsl(
            #version 460
            layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput xs[2];
            layout(location=0) out vec4 color;
            void main() {
                color = subpassLoad(xs[1]);
            }
        )glsl";
        VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
            helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
            helper.gp_ci_.renderPass = rp.Handle();
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    // use undefined size array and index into element 1 to read
    {
        const char *fs_source = R"glsl(
            #version 460
            layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput xs[];
            layout(location=0) out vec4 color;
            void main() {
                color = subpassLoad(xs[1]);
            }
        )glsl";
        VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
            helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
            helper.gp_ci_.renderPass = rp.Handle();
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    // Array of size 1
    // loads from index 0, but not the invalid index 0 since has offest of 3
    {
        const char *fs_source = R"glsl(
            #version 460
            layout(input_attachment_index=3, set=0, binding=0) uniform subpassInput xs[1];
            layout(location=0) out vec4 color;
            void main() {
                color = subpassLoad(xs[0]);
            }
        )glsl";
        VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
            helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
            helper.gp_ci_.renderPass = rp.Handle();
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    // Index from non-zero
    {
        const char *fs_source = R"glsl(
            #version 460
            layout(input_attachment_index=2, set=0, binding=0) uniform subpassInput xs[2];
            layout(location=0) out vec4 color;
            void main() {
                color = subpassLoad(xs[0]) + subpassLoad(xs[1]);
            }
        )glsl";
        VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
            helper.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
            helper.gp_ci_.renderPass = rp.Handle();
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }
}

TEST_F(PositiveShaderInterface, InputAttachmentRuntimeArray) {
    TEST_DESCRIPTION("Input Attachment array where need to follow the index into the array");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    AddRequiredFeature(vkt::Feature::shaderInputAttachmentArrayNonUniformIndexing);
    RETURN_IF_SKIP(Init());

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(m_render_target_fmt);
    // index 0 is unused
    rp.AddAttachmentReference({VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_GENERAL});
    // index 1 is is valid (for both color and input)
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    // index 2 and 3 point to same image as index 1
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddInputAttachment(0);
    rp.AddInputAttachment(1);
    rp.AddInputAttachment(2);
    rp.AddInputAttachment(3);
    rp.AddColorAttachment(1);
    rp.CreateRenderPass();

    // use OpTypeRuntimeArray and index into it
    // This is something that is needed to be validated at draw time, so should not be an error
    const char *fs_source = R"glsl(
        #version 460
        #extension GL_EXT_nonuniform_qualifier : require
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput xs[];
        layout(set = 0, binding = 3) buffer ssbo { int rIndex; };
        layout(location=0) out vec4 color;
        void main() {
            color = subpassLoad(xs[nonuniformEXT(rIndex)]);
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
        helper.gp_ci_.renderPass = rp.Handle();
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

}
TEST_F(PositiveShaderInterface, InputAttachmentDepthStencil) {
    TEST_DESCRIPTION("Input Attachment sharing same variable, but different aspect");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    const VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(m_render_target_fmt);
    rp.AddAttachmentDescription(ds_format);
    // index 0 = color | index 1 = depth | index 2 = stencil
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddInputAttachment(0);
    rp.AddInputAttachment(1);
    rp.AddInputAttachment(2);
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    // Depth and Stencil use same index, but valid because differnet image aspect masks
    const char *fs_source = R"glsl(
            #version 460
            layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput i_color;
            layout(input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput i_depth;
            layout(input_attachment_index = 1, set = 0, binding = 2) uniform usubpassInput i_stencil;
            layout(location=0) out vec4 color;

            void main(void)
            {
                color = subpassLoad(i_color);
                vec4 depth = subpassLoad(i_depth);
                uvec4 stencil = subpassLoad(i_stencil);
            }
        )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                {1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                {2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
        helper.gp_ci_.renderPass = rp.Handle();
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderInterface, FragmentOutputNotConsumedButAlphaToCoverageEnabled) {
    TEST_DESCRIPTION(
        "Test that no warning is produced when writing to non-existing color attachment if alpha to coverage is enabled.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget(0u);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ms_ci_ = ms_state_ci;
        helper.cb_ci_.attachmentCount = 0;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderInterface, AlphaToCoverageOutputIndex0) {
    TEST_DESCRIPTION("DualSource blend has two outputs at location zero, so Index 0 is the one that's required");
    AddRequiredFeature(vkt::Feature::dualSrcBlend);
    RETURN_IF_SKIP(Init());
    InitRenderTarget(0u);

    const char *fs_src = R"glsl(
        #version 460
        layout(location = 0, index = 0) out vec4 c0;
        void main() {
            c0 = vec4(0.0f);
        }
    )glsl";
    VkShaderObj fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.ms_ci_ = ms_state_ci;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

// Spec doesn't clarify if this is valid or not
// https://gitlab.khronos.org/vulkan/vulkan/-/issues/3445
TEST_F(PositiveShaderInterface, DISABLED_InputOutputMatch2) {
    TEST_DESCRIPTION("Test matching vertex shader output with fragment shader input.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char vsSource[] = R"glsl(#version 450
        layout(location = 0) out vec2 v1;
        layout(location = 1) out vec2 v2;
        layout(location = 2) out vec2 v3;

        void main() {
            v1 = vec2(0.0f);
            v2 = vec2(1.0f);
            v3 = vec2(0.5f);
        }
    )glsl";

    const char fsSource[] = R"glsl(#version 450
        layout(location = 0) in mat3x2 v;
        layout(location = 0) out vec4 color;

        void main() {
            color = vec4(v[0][0], v[0][1], v[1][0], v[1][1]);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderInterface, InputOutputMatch) {
    TEST_DESCRIPTION("Test matching vertex shader output with fragment shader input.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char vsSource[] = R"glsl(#version 450

        layout(location = 0) in vec4 dEQP_Position;
        layout(location = 1) in mat3 in0;
        layout(location = 0) out vec4 v1;
        layout(location = 1) out vec4 v2;
        layout(location = 2) out vec4 v3;
        layout(location = 3) out vec4 v4;

        void main() {
            v1 = mat4(in0)[0];
            v2 = mat4(in0)[1];
            v3 = mat4(in0)[2];
            v4 = mat4(in0)[3];
            gl_Position = dEQP_Position;
        }
    )glsl";

    const char fsSource[] = R"glsl(#version 450

        bool isOk (mat4 a, mat4 b, float eps) {
            vec4 diff = max(max(abs(a[0]-b[0]), abs(a[1]-b[1])), max(abs(a[2]-b[2]), abs(a[3]-b[3])));
            return all(lessThanEqual(diff, vec4(eps)));
        }

        layout(location = 0) in mat4 out0;
        layout(set = 0, binding = 0) uniform block { mat4 ref_out0; };
        layout(location = 0) out vec4 color;

        void main() {
            bool RES = isOk(out0, ref_out0, 0.05);
            color = vec4(RES, RES, RES, 1.0);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkVertexInputBindingDescription vertex_input_binding_description{};
    vertex_input_binding_description.binding = 0;
    vertex_input_binding_description.stride = 0;
    vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vertex_input_attribute_descriptions[4];
    vertex_input_attribute_descriptions[0].location = 0;
    vertex_input_attribute_descriptions[0].binding = 0;
    vertex_input_attribute_descriptions[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    vertex_input_attribute_descriptions[0].offset = 0;
    vertex_input_attribute_descriptions[1].location = 1;
    vertex_input_attribute_descriptions[1].binding = 0;
    vertex_input_attribute_descriptions[1].format = VK_FORMAT_R8G8B8A8_UNORM;
    vertex_input_attribute_descriptions[1].offset = 32;
    vertex_input_attribute_descriptions[2].location = 2;
    vertex_input_attribute_descriptions[2].binding = 0;
    vertex_input_attribute_descriptions[2].format = VK_FORMAT_R8G8B8A8_UNORM;
    vertex_input_attribute_descriptions[2].offset = 64;
    vertex_input_attribute_descriptions[3].location = 3;
    vertex_input_attribute_descriptions[3].binding = 0;
    vertex_input_attribute_descriptions[3].format = VK_FORMAT_R8G8B8A8_UNORM;
    vertex_input_attribute_descriptions[3].offset = 96;

    OneOffDescriptorSet ds(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                  });

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &vertex_input_binding_description;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 4;
    pipe.vi_ci_.pVertexAttributeDescriptions = vertex_input_attribute_descriptions;
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds.layout_});
    pipe.CreateGraphicsPipeline();

    vkt::Buffer uniform_buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    ds.WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, 1024);
    ds.UpdateDescriptorSets();

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkBuffer buffer_handle = buffer.handle();
    VkDeviceSize offset = 0;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &buffer_handle, &offset);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &ds.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveShaderInterface, NestedStructs) {
    TEST_DESCRIPTION("Use nested structs between shaders.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char vsSource[] = R"glsl(
        #version 450
        struct TestStruct {
            vec2 dummy;
            vec4 variableInStruct;
        };
        layout(location = 0) out block {
            vec2 dummy;
            TestStruct structInBlock;
        } testBlock;
        void main(void) {}
    )glsl";

    const char fsSource[] = R"glsl(
        #version 450
        layout(location = 0) out vec4 color;
        struct TestStruct {
            vec2 dummy;
            vec4 variableInStruct;
        };
        layout(location = 0) in block {
            vec2 dummy;
            noperspective TestStruct structInBlock;
        } testBlock;
        void main(void) {}
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderInterface, AlphaToCoverageOffsetToAlpha) {
    TEST_DESCRIPTION("Only set the needed component and nothing else.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget(0u);

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0, component = 3) out float x;
        void main(){
            x = 1.0;
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.ms_ci_ = ms_state_ci;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderInterface, AlphaToCoverageArray) {
    TEST_DESCRIPTION("Have array out outputs");

    RETURN_IF_SKIP(Init());
    InitRenderTarget(0u);

    char const *fsSource = R"glsl(
        #version 450
        // Just need to declare variable
        layout(location=0) out vec4 fragData[4];
        void main() {
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.ms_ci_ = ms_state_ci;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderInterface, VsFsTypeMismatchBlockStructArray) {
    TEST_DESCRIPTION("Have an struct inside a block between shaders");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (m_device->Physical().limits_.maxVertexOutputComponents <= 64) {
        GTEST_SKIP() << "maxVertexOutputComponents is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        struct S {
            float b;
            int[2] c;
        };

        out block {
            layout(location=0) float x;
            layout(location=6) S[3] y; // difference, but can have extra output locations
            layout(location=16) int[4] z;
        } outBlock;

        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct S {
            float b;
            int[2] c;
        };

        in block {
            layout(location=0) float x;
            layout(location=6) S[2] y;
            layout(location=16) int[4] z;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderInterface, VsFsTypeMismatchBlockNestedStructLastElementArray) {
    TEST_DESCRIPTION("Have nested struct inside a block between shaders");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        struct A {
            float a0_;
        };
        struct B {
            int b0_;
            A b1_[2]; // on last element, so just a dangling vertex output
        };
        struct C {
            vec4 c0_[2];
            A c1_;
            B c2_;
        };

        out block {
            layout(location=0) float x;
            layout(location=1) C y;
        } outBlock;

        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct A {
            float a0_;
        };
        struct B {
            int b0_;
            A b1_;
        };
        struct C {
            vec4 c0_[2];
            A c1_;
            B c2_;
        };

        in block {
            layout(location=0) float x;
            layout(location=1) C y;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderInterface, VsFsTypeMismatchBlockNestedStructArray) {
    TEST_DESCRIPTION("Have nested struct inside a block between shaders");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        struct A {
            float a0_;
        };
        struct B {
            int b0_;
            A b1_[2][3];
        };
        struct C {
            vec4 c0_[2];
            A c1_;
            B c2_;
        };

        out block {
            layout(location=0) float x;
            layout(location=1) C y;
        } outBlock;

        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct A {
            float a0_;
        };
        struct B {
            int b0_;
            A b1_[3][2];
        };
        struct C {
            vec4 c0_[2];
            A c1_;
            B c2_;
        };

        in block {
            layout(location=0) float x;
            layout(location=1) C y;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderInterface, MultidimensionalArray) {
    TEST_DESCRIPTION("Make sure multidimensional arrays are handled");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out float[4][2][2] x;
        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float[4][2][2] x;
        layout(location=0) out float color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderInterface, MultidimensionalArrayVertex) {
    TEST_DESCRIPTION("multidimensional arrays but have lingering vertex output");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (m_device->Physical().limits_.maxVertexOutputComponents <= 64) {
        GTEST_SKIP() << "maxVertexOutputComponents is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out float[4][3][2] x;
        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float[4][2][2] x;
        layout(location=0) out float color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderInterface, MultidimensionalArrayDims) {
    TEST_DESCRIPTION("multidimensional arrays but have lingering vertex output");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (m_device->Physical().limits_.maxVertexOutputComponents <= 64) {
        GTEST_SKIP() << "maxVertexOutputComponents is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out float[4][3][2] x; // 24 locations
        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float[3][2][4] x; // 24 locations
        layout(location=0) out float color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderInterface, MultidimensionalArrayDims2) {
    TEST_DESCRIPTION("multidimensional arrays but have lingering vertex output");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (m_device->Physical().limits_.maxVertexOutputComponents <= 64) {
        GTEST_SKIP() << "maxVertexOutputComponents is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out float[4][3][2] x; // 24 locations
        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float[24] x;
        layout(location=0) out float color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderInterface, MultidimensionalArray64bit) {
    TEST_DESCRIPTION("Make sure multidimensional arrays are handled for 64bits");

    AddRequiredFeature(vkt::Feature::shaderFloat64);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (m_device->Physical().limits_.maxFragmentOutputAttachments < 9) {
        GTEST_SKIP() << "maxFragmentOutputAttachments is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        layout(location=0) out f64vec3[2][2] x; // take 2 locations each (total 8)
        layout(location=8) out float y;
        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        layout(location=0) flat in f64vec3[2][2] x;
        layout(location=8) out float color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderInterface, PackingInsideArray) {
    TEST_DESCRIPTION("From https://gitlab.khronos.org/vulkan/vulkan/-/issues/3558");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // GLSL use to allow alias location of different types
    // https://github.com/KhronosGroup/glslang/pull/3438
    //
    // layout(location = 0, component = 1) out float[2] x;
    // layout(location = 1, component = 0) out int y;
    // layout(location = 1, component = 2) out int z;
    char const *vsSource = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %x %y %z
               OpDecorate %x Component 1
               OpDecorate %x Location 0
               OpDecorate %y Component 0
               OpDecorate %y Location 1
               OpDecorate %z Component 2
               OpDecorate %z Location 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
          %x = OpVariable %_ptr_Output__arr_float_uint_2 Output
        %int = OpTypeInt 32 1
%_ptr_Output_int = OpTypePointer Output %int
          %y = OpVariable %_ptr_Output_int Output
          %z = OpVariable %_ptr_Output_int Output
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    // layout(location = 0, component = 1) in float x1;
    // layout(location = 1, component = 0) flat in int y;
    // layout(location = 1, component = 1) in float x2;
    // layout(location = 1, component = 2) flat in int z;
    // layout(location=0) out float color;
    char const *fsSource = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %x1 %y %x2 %z %color
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %x1 Component 1
               OpDecorate %x1 Location 0
               OpDecorate %y Flat
               OpDecorate %y Component 0
               OpDecorate %y Location 1
               OpDecorate %x2 Component 1
               OpDecorate %x2 Location 1
               OpDecorate %z Flat
               OpDecorate %z Component 2
               OpDecorate %z Location 1
               OpDecorate %color Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Input_float = OpTypePointer Input %float
         %x1 = OpVariable %_ptr_Input_float Input
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
          %y = OpVariable %_ptr_Input_int Input
         %x2 = OpVariable %_ptr_Input_float Input
          %z = OpVariable %_ptr_Input_int Input
%_ptr_Output_float = OpTypePointer Output %float
      %color = OpVariable %_ptr_Output_float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderInterface, MultipleFragmentAttachment) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7923");
    RETURN_IF_SKIP(Init());
    m_errorMonitor->ExpectSuccess(kWarningBit | kErrorBit);

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color[2];
        void main() {
           color[0] = vec4(1.0);
           color[1] = vec4(1.0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.AddColorAttachment(1);
    rp.CreateRenderPass();

    VkPipelineColorBlendAttachmentState cb_as[2];
    cb_as[0] = DefaultColorBlendAttachmentState();
    cb_as[1] = DefaultColorBlendAttachmentState();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.cb_ci_.attachmentCount = 2;
    pipe.cb_ci_.pAttachments = cb_as;
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderInterface, MissingInputAttachmentIndex) {
    TEST_DESCRIPTION("You don't need InputAttachmentIndex if using dynamicRenderingLocalRead");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::dynamicRenderingLocalRead);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput xs;
    // layout(location=0) out vec4 color;
    // void main() {
    //     color = subpassLoad(xs);
    // }
    //
    // missing OpDecorate %xs InputAttachmentIndex 0
    const char *fsSource = R"(
               OpCapability Shader
               OpCapability InputAttachment
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %color %xs
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %color Location 0
               OpDecorate %xs DescriptorSet 0
               OpDecorate %xs Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %color = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float SubpassData 0 0 0 2 Unknown
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
         %xs = OpVariable %_ptr_UniformConstant_10 UniformConstant
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %v2int = OpTypeVector %int 2
         %17 = OpConstantComposite %v2int %int_0 %int_0
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpLoad %10 %xs
         %18 = OpImageRead %v4float %13 %17
               OpStore %color %18
               OpReturn
               OpFunctionEnd
    )";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddInputAttachment(0);
    rp.CreateRenderPass();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveShaderInterface, PhysicalStorageBufferGlslang3) {
    TEST_DESCRIPTION(
        "Taken from glslang spv.bufferhandle3.frag test - just creating the shader is valid, interface is tested elsewhere");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference, std430) buffer t3 {
            int h;
        };

        layout(set = 1, binding = 2, buffer_reference, std430) buffer t4 {
            layout(offset = 0)  int j;
            t3 k;
        } x;

        layout(set = 0, binding = 0, std430) buffer t5 {
            t4 m;
        } s5;

        layout(location = 0) flat in t4 k;

        t4 foo(t4 y) { return y; }
        void main() {}
    )glsl";

    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);
}