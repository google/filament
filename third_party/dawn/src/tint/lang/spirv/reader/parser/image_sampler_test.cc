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
  %1:ptr<handle, spirv.image<f32, 1d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
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
  %1:ptr<handle, spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
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
  %1:ptr<handle, spirv.image<f32, 3d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
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
  %1:ptr<handle, spirv.image<f32, cube, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
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
  %1:ptr<handle, spirv.image<f32, subpass_data, not_depth, non_arrayed, single_sampled, rw_op_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
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
  %1:ptr<handle, spirv.image<f32, 1d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
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
  %1:ptr<handle, spirv.image<f32, 2d, depth_unknown, non_arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
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
  %1:ptr<handle, spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
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
  %1:ptr<handle, spirv.image<f32, 1d, not_depth, non_arrayed, single_sampled, rw_op_compatible, undefined, read>, read> = var undef @binding_point(1, 2)
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
  %1:ptr<handle, spirv.image<f32, 1d, not_depth, non_arrayed, single_sampled, sampling_compatible, rg32float, read>, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Type_SampledImage) {
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
      %inner = OpTypeImage %f32 1D 0 0 0 1 Rg32f
        %img = OpTypeSampledImage %inner
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
  %1:ptr<handle, spirv.sampled_image<spirv.image<f32, 1d, not_depth, non_arrayed, single_sampled, sampling_compatible, rg32float, read>>, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
