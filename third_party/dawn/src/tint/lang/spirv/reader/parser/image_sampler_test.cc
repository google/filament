// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/spirv/reader/parser/helper_test.h"

namespace tint::spirv::reader {
namespace {

TEST_F(SpirvParserTest, Type_Sampler) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
       %samp = OpTypeSampler
%_ptr_Uniform = OpTypePointer UniformConstant %samp
          %5 = OpTypeFunction %void
          %6 = OpVariable %_ptr_Uniform UniformConstant
          %1 = OpFunction %void None %5
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<handle, sampler, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Type_Image_1d) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Sampled1D
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %img = OpTypeImage %f32 1D 0 0 0 1 Unknown
%_ptr_Uniform = OpTypePointer UniformConstant %img
          %6 = OpVariable %_ptr_Uniform UniformConstant
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<handle, spirv.image<f32, 1d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Type_Image_2d) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Sampled1D
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %img = OpTypeImage %f32 2D 0 0 0 1 Unknown
%_ptr_Uniform = OpTypePointer UniformConstant %img
          %6 = OpVariable %_ptr_Uniform UniformConstant
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Type_Image_3d) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Sampled1D
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %img = OpTypeImage %f32 3D 0 0 0 1 Unknown
%_ptr_Uniform = OpTypePointer UniformConstant %img
          %6 = OpVariable %_ptr_Uniform UniformConstant
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<handle, spirv.image<f32, 3d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Type_Image_Cube) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Sampled1D
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %img = OpTypeImage %f32 Cube 0 0 0 1 Unknown
%_ptr_Uniform = OpTypePointer UniformConstant %img
          %6 = OpVariable %_ptr_Uniform UniformConstant
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<handle, spirv.image<f32, cube, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Type_Image_SubpassData) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability InputAttachment
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %img = OpTypeImage %f32 SubpassData 0 0 0 2 Unknown
%_ptr_Uniform = OpTypePointer UniformConstant %img
          %6 = OpVariable %_ptr_Uniform UniformConstant
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<handle, spirv.image<f32, subpass_data, not_depth, non_arrayed, single_sampled, rw_op_compatible, undefined, read_write>, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Type_Image_Depth) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Sampled1D
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %img = OpTypeImage %f32 1D 1 0 0 1 Unknown
%_ptr_Uniform = OpTypePointer UniformConstant %img
          %6 = OpVariable %_ptr_Uniform UniformConstant
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<handle, spirv.image<f32, 1d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Type_Image_DepthUnknown) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Sampled1D
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %img = OpTypeImage %f32 2D 2 0 0 1 Unknown
%_ptr_Uniform = OpTypePointer UniformConstant %img
          %6 = OpVariable %_ptr_Uniform UniformConstant
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<handle, spirv.image<f32, 2d, depth_unknown, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Type_Image_Arrayed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Sampled1D
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %img = OpTypeImage %f32 2D 0 1 0 1 Unknown
%_ptr_Uniform = OpTypePointer UniformConstant %img
          %6 = OpVariable %_ptr_Uniform UniformConstant
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<handle, spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Type_Image_RW_Ops) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Sampled1D
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %img = OpTypeImage %f32 1D 0 0 0 2 Unknown
%_ptr_Uniform = OpTypePointer UniformConstant %img
          %6 = OpVariable %_ptr_Uniform UniformConstant
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<handle, spirv.image<f32, 1d, not_depth, non_arrayed, single_sampled, rw_op_compatible, undefined, read_write>, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Type_Image_TexelFormat) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Sampled1D
               OpCapability StorageImageExtendedFormats
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %img = OpTypeImage %f32 1D 0 0 0 1 Rg32f
%_ptr_Uniform = OpTypePointer UniformConstant %img
          %6 = OpVariable %_ptr_Uniform UniformConstant
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<handle, spirv.image<f32, 1d, not_depth, non_arrayed, single_sampled, sampling_compatible, rg32float, read_write>, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Type_Image_TexelFormat_R8) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Sampled1D
               OpCapability StorageImageExtendedFormats
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %img = OpTypeImage %f32 1D 0 0 0 1 R8
%_ptr_Uniform = OpTypePointer UniformConstant %img
          %6 = OpVariable %_ptr_Uniform UniformConstant
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<handle, spirv.image<f32, 1d, not_depth, non_arrayed, single_sampled, sampling_compatible, r8unorm, read_write>, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Type_SampledImage) {
    options.sampler_mappings.insert({BindingPoint{1, 2}, BindingPoint{5, 6}});

    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Sampled1D
               OpCapability StorageImageExtendedFormats
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
        %f32 = OpTypeFloat 32
      %inner = OpTypeImage %f32 2D 0 0 0 1 Rg32f
        %img = OpTypeSampledImage %inner
    %v2float = OpTypeVector %f32 2
    %v4float = OpTypeVector %f32 4
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %f32 1
    %float_2 = OpConstant %f32 2
    %coords2 = OpConstantComposite %v2float %float_1 %float_2
%_ptr_Uniform = OpTypePointer UniformConstant %img
          %6 = OpVariable %_ptr_Uniform UniformConstant
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
         %10 = OpLoad %img %6
     %result = OpImageGather %v4float %10 %coords2 %int_1
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<handle, sampler, read> = var undef @binding_point(5, 6)
  %2:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, rg32float, read_write>, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, rg32float, read_write> = load %2
    %5:sampler = load %1
    %6:spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, rg32float, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, rg32float, read_write>> %4, %5
    %7:vec4<f32> = spirv.image_gather %6, vec2<f32>(1.0f, 2.0f), 1i, 0u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Type_OpSampledImage) {
    options.sampler_mappings.insert({BindingPoint{1, 2}, BindingPoint{5, 6}});

    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Sampled1D
               OpCapability StorageImageExtendedFormats
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var_tex DescriptorSet 1
               OpDecorate %var_tex Binding 2
               OpDecorate %var_sampler DescriptorSet 1
               OpDecorate %var_sampler Binding 1
       %void = OpTypeVoid
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
    %sampler = OpTypeSampler
%ptr_sampler = OpTypePointer UniformConstant %sampler
        %tex = OpTypeImage %float 2D 0 0 0 1 Unknown
    %ptr_tex = OpTypePointer UniformConstant %tex
%sampled_img = OpTypeSampledImage %tex
    %v2float = OpTypeVector %float 2
    %v4float = OpTypeVector %float 4
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %coords2 = OpConstantComposite %v2float %float_1 %float_2
    %var_tex = OpVariable %ptr_tex UniformConstant
%var_sampler = OpVariable %ptr_sampler UniformConstant
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %sam = OpLoad %sampler %var_sampler
         %im = OpLoad %tex %var_tex
%sampled_image = OpSampledImage %sampled_img %im %sam
     %result = OpImageGather %v4float %sampled_image %coords2 %int_1
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, read> = var undef @binding_point(1, 2)
  %2:ptr<handle, sampler, read> = var undef @binding_point(1, 1)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:sampler = load %2
    %5:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write> = load %1
    %6:spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %5, %4
    %7:vec4<f32> = spirv.image_gather %6, vec2<f32>(1.0f, 2.0f), 1i, 0u
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
