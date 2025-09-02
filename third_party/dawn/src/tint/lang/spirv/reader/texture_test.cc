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

#include "src/tint/lang/spirv/reader/helper_test.h"

namespace tint::spirv::reader {
namespace {

TEST_F(SpirvReaderTest, Handle_Sampler) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %10 "wg"
           OpDecorate %10 DescriptorSet 0
           OpDecorate %10 Binding 0
%sampler = OpTypeSampler
    %ptr = OpTypePointer UniformConstant %sampler
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void
     %10 = OpVariable %ptr UniformConstant
   %main = OpFunction %void None %voidfn
  %entry = OpLabel
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %wg:ptr<handle, sampler, read> = var undef @binding_point(0, 0)
}

%main = @fragment func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Handle_Image_Texture) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability Sampled1D
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %10 "wg"
           OpDecorate %10 DescriptorSet 0
           OpDecorate %10 Binding 0
  %float = OpTypeFloat 32
    %tex = OpTypeImage %float 1D 0 0 0 1 Unknown
%ptr_tex = OpTypePointer UniformConstant %tex
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void
     %10 = OpVariable %ptr_tex UniformConstant
   %main = OpFunction %void None %voidfn
  %entry = OpLabel
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %wg:ptr<handle, texture_1d<f32>, read> = var undef @binding_point(0, 0)
}

%main = @fragment func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Handle_Image_NonWritable) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability Sampled1D
           OpCapability StorageImageExtendedFormats
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %10 "wg"
           OpDecorate %10 DescriptorSet 0
           OpDecorate %10 Binding 0
           OpDecorate %10 NonWritable
  %float = OpTypeFloat 32
%storage = OpTypeImage %float 1D 0 0 0 2 Rg32f
%ptr_storage = OpTypePointer UniformConstant %storage
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void
     %10 = OpVariable %ptr_storage UniformConstant
   %main = OpFunction %void None %voidfn
  %entry = OpLabel
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %wg:ptr<handle, texture_storage_1d<rg32float, read>, read> = var undef @binding_point(0, 0)
}

%main = @fragment func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Handle_Image_NonReadable) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability Sampled1D
           OpCapability StorageImageExtendedFormats
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %10 "wg"
           OpDecorate %10 DescriptorSet 0
           OpDecorate %10 Binding 0
           OpDecorate %10 NonReadable
  %float = OpTypeFloat 32
%storage = OpTypeImage %float 1D 0 0 0 2 Rg32f
%ptr_storage = OpTypePointer UniformConstant %storage
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void
     %10 = OpVariable %ptr_storage UniformConstant
   %main = OpFunction %void None %voidfn
  %entry = OpLabel
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %wg:ptr<handle, texture_storage_1d<rg32float, write>, read> = var undef @binding_point(0, 0)
}

%main = @fragment func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Handle_SameImageType_TwoVarsWithDifferentAccessModes) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability Sampled1D
           OpCapability StorageImageExtendedFormats
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %write_only_image "write_only_image"
           OpName %read_only_image "read_only_image"
           OpDecorate %write_only_image DescriptorSet 0
           OpDecorate %write_only_image Binding 0
           OpDecorate %write_only_image NonReadable
           OpDecorate %read_only_image DescriptorSet 0
           OpDecorate %read_only_image Binding 1
           OpDecorate %read_only_image NonWritable
  %float = OpTypeFloat 32
%storage = OpTypeImage %float 1D 0 0 0 2 Rg32f
%ptr_storage = OpTypePointer UniformConstant %storage
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void
%write_only_image = OpVariable %ptr_storage UniformConstant
 %read_only_image = OpVariable %ptr_storage UniformConstant
   %main = OpFunction %void None %voidfn
  %entry = OpLabel
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %write_only_image:ptr<handle, texture_storage_1d<rg32float, write>, read> = var undef @binding_point(0, 0)
  %read_only_image:ptr<handle, texture_storage_1d<rg32float, read>, read> = var undef @binding_point(0, 1)
}

%main = @fragment func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Handle_MS_2D) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability ImageQuery
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %10 "wg"
           OpDecorate %10 DescriptorSet 0
           OpDecorate %10 Binding 0
  %float = OpTypeFloat 32
   %uint = OpTypeInt 32 0
    %tex = OpTypeImage %float 2D 0 0 1 1 Unknown
%ptr_tex = OpTypePointer UniformConstant %tex
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void
     %10 = OpVariable %ptr_tex UniformConstant
   %main = OpFunction %void None %voidfn
  %entry = OpLabel
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %wg:ptr<handle, texture_multisampled_2d<f32>, read> = var undef @binding_point(0, 0)
}

%main = @fragment func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Handle_Depth_MS_2D) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability ImageQuery
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %10 "wg"
           OpDecorate %10 DescriptorSet 0
           OpDecorate %10 Binding 0
  %float = OpTypeFloat 32
   %uint = OpTypeInt 32 0
    %tex = OpTypeImage %float 2D 1 0 1 1 Unknown
%ptr_tex = OpTypePointer UniformConstant %tex
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void
     %10 = OpVariable %ptr_tex UniformConstant
   %main = OpFunction %void None %voidfn
  %entry = OpLabel
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %wg:ptr<handle, texture_depth_multisampled_2d, read> = var undef @binding_point(0, 0)
}

%main = @fragment func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Image) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability ImageQuery
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %10 "wg"
           OpDecorate %var_sampler DescriptorSet 0
           OpDecorate %var_sampler Binding 0
           OpDecorate %10 DescriptorSet 2
           OpDecorate %10 Binding 0
    %int = OpTypeInt 32 1
  %float = OpTypeFloat 32
  %v2int = OpTypeVector %int 2
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
    %tex = OpTypeImage %float 2D 0 0 0 1 Unknown
%ptr_tex = OpTypePointer UniformConstant %tex
%sampled_img = OpTypeSampledImage %tex
%sampler = OpTypeSampler
%ptr_sampler = OpTypePointer UniformConstant %sampler
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void

  %int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%coords2 = OpConstantComposite %v2float %float_1 %float_2

%var_sampler = OpVariable %ptr_sampler UniformConstant
     %10 = OpVariable %ptr_tex UniformConstant

   %main = OpFunction %void None %voidfn
  %entry = OpLabel
    %sam = OpLoad %sampler %var_sampler
     %im = OpLoad %tex %10
%sampled_image = OpSampledImage %sampled_img %im %sam
 %result = OpImage %tex %sampled_image
   %size = OpImageQueryLevels %int %result
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %1:ptr<handle, sampler, read> = var undef @binding_point(0, 0)
  %wg:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(2, 0)
}

%main = @fragment func():void {
  $B2: {
    %4:sampler = load %1
    %5:texture_2d<f32> = load %wg
    %6:u32 = textureNumLevels %5
    %7:i32 = convert %6
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, ImageGather) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %10 "wg"
           OpDecorate %var_sampler DescriptorSet 0
           OpDecorate %var_sampler Binding 0
           OpDecorate %10 DescriptorSet 2
           OpDecorate %10 Binding 0
    %int = OpTypeInt 32 1
  %float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
    %tex = OpTypeImage %float 2D 0 0 0 1 Unknown
%ptr_tex = OpTypePointer UniformConstant %tex
%sampled_img = OpTypeSampledImage %tex
%sampler = OpTypeSampler
%ptr_sampler = OpTypePointer UniformConstant %sampler
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void

  %int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%coords2 = OpConstantComposite %v2float %float_1 %float_2

%var_sampler = OpVariable %ptr_sampler UniformConstant
     %10 = OpVariable %ptr_tex UniformConstant

   %main = OpFunction %void None %voidfn
  %entry = OpLabel
    %sam = OpLoad %sampler %var_sampler
     %im = OpLoad %tex %10
%sampled_image = OpSampledImage %sampled_img %im %sam
 %result = OpImageGather %v4float %sampled_image %coords2 %int_1
     %r2 = OpFAdd %v4float %result %result
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %1:ptr<handle, sampler, read> = var undef @binding_point(0, 0)
  %wg:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(2, 0)
}

%main = @fragment func():void {
  $B2: {
    %4:sampler = load %1
    %5:texture_2d<f32> = load %wg
    %6:vec4<f32> = textureGather 1i, %5, %4, vec2<f32>(1.0f, 2.0f)
    %7:vec4<f32> = add %6, %6
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, ImageSampleImplicitLod) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability Sampled1D
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %10 "wg"
           OpDecorate %var_sampler DescriptorSet 0
           OpDecorate %var_sampler Binding 0
           OpDecorate %10 DescriptorSet 2
           OpDecorate %10 Binding 0
    %int = OpTypeInt 32 1
  %float = OpTypeFloat 32
  %v2int = OpTypeVector %int 2
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
    %tex = OpTypeImage %float 2D 0 0 0 1 Unknown
%ptr_tex = OpTypePointer UniformConstant %tex
%sampled_img = OpTypeSampledImage %tex
%sampler = OpTypeSampler
%ptr_sampler = OpTypePointer UniformConstant %sampler
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void

  %int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%coords2 = OpConstantComposite %v2float %float_1 %float_2

%var_sampler = OpVariable %ptr_sampler UniformConstant
     %10 = OpVariable %ptr_tex UniformConstant

 %int_10 = OpConstant %int 10
 %int_11 = OpConstant %int 11
%offset2i = OpConstantComposite %v2int %int_10 %int_11

   %main = OpFunction %void None %voidfn
  %entry = OpLabel
    %sam = OpLoad %sampler %var_sampler
     %im = OpLoad %tex %10
%sampled_image = OpSampledImage %sampled_img %im %sam
 %result = OpImageSampleImplicitLod %v4float %sampled_image %coords2 ConstOffset %offset2i
     %r2 = OpFAdd %v4float %result %result
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %1:ptr<handle, sampler, read> = var undef @binding_point(0, 0)
  %wg:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(2, 0)
}

%main = @fragment func():void {
  $B2: {
    %4:sampler = load %1
    %5:texture_2d<f32> = load %wg
    %6:vec4<f32> = textureSample %5, %4, vec2<f32>(1.0f, 2.0f), vec2<i32>(10i, 11i)
    %7:vec4<f32> = add %6, %6
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, ImageSampleExplicitLod) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability Sampled1D
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %10 "wg"
           OpDecorate %var_sampler DescriptorSet 0
           OpDecorate %var_sampler Binding 0
           OpDecorate %10 DescriptorSet 2
           OpDecorate %10 Binding 0
    %int = OpTypeInt 32 1
  %float = OpTypeFloat 32
  %v2int = OpTypeVector %int 2
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
    %tex = OpTypeImage %float 2D 0 0 0 1 Unknown
%ptr_tex = OpTypePointer UniformConstant %tex
%sampled_img = OpTypeSampledImage %tex
%sampler = OpTypeSampler
%ptr_sampler = OpTypePointer UniformConstant %sampler
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void

  %int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%coords2 = OpConstantComposite %v2float %float_1 %float_2

%var_sampler = OpVariable %ptr_sampler UniformConstant
     %10 = OpVariable %ptr_tex UniformConstant

 %int_10 = OpConstant %int 10
 %int_11 = OpConstant %int 11
%offset2i = OpConstantComposite %v2int %int_10 %int_11

   %main = OpFunction %void None %voidfn
  %entry = OpLabel
    %sam = OpLoad %sampler %var_sampler
     %im = OpLoad %tex %10
%sampled_image = OpSampledImage %sampled_img %im %sam
 %result = OpImageSampleExplicitLod %v4float %sampled_image %coords2 Lod|ConstOffset %float_1 %offset2i
     %r2 = OpFAdd %v4float %result %result
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %1:ptr<handle, sampler, read> = var undef @binding_point(0, 0)
  %wg:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(2, 0)
}

%main = @fragment func():void {
  $B2: {
    %4:sampler = load %1
    %5:texture_2d<f32> = load %wg
    %6:vec4<f32> = textureSampleLevel %5, %4, vec2<f32>(1.0f, 2.0f), 1.0f, vec2<i32>(10i, 11i)
    %7:vec4<f32> = add %6, %6
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, ImageQuerySize) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability Sampled1D
           OpCapability ImageQuery
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %wg "wg"
           OpDecorate %wg DescriptorSet 2
           OpDecorate %wg Binding 0
    %int = OpTypeInt 32 1
  %float = OpTypeFloat 32
    %tex = OpTypeImage %float 1D 0 0 0 2 R32f
%ptr_tex = OpTypePointer UniformConstant %tex
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void

     %wg = OpVariable %ptr_tex UniformConstant

   %main = OpFunction %void None %voidfn
  %entry = OpLabel
     %im = OpLoad %tex %wg
 %result = OpImageQuerySize %int %im
     %r2 = OpIAdd %int %result %result
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %wg:ptr<handle, texture_storage_1d<r32float, read_write>, read> = var undef @binding_point(2, 0)
}

%main = @fragment func():void {
  $B2: {
    %3:texture_storage_1d<r32float, read_write> = load %wg
    %4:u32 = textureDimensions %3
    %5:i32 = convert %4
    %6:i32 = add %5, %5
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, ImageQueryLevels) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability Sampled1D
           OpCapability ImageQuery
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %wg "wg"
           OpDecorate %wg DescriptorSet 2
           OpDecorate %wg Binding 0
    %int = OpTypeInt 32 1
  %float = OpTypeFloat 32
    %tex = OpTypeImage %float 1D 0 0 0 1 R32f
%ptr_tex = OpTypePointer UniformConstant %tex
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void

     %wg = OpVariable %ptr_tex UniformConstant

   %main = OpFunction %void None %voidfn
  %entry = OpLabel
     %im = OpLoad %tex %wg
 %result = OpImageQueryLevels %int %im
     %r2 = OpIAdd %int %result %result
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %wg:ptr<handle, texture_1d<f32>, read> = var undef @binding_point(2, 0)
}

%main = @fragment func():void {
  $B2: {
    %3:texture_1d<f32> = load %wg
    %4:u32 = textureNumLevels %3
    %5:i32 = convert %4
    %6:i32 = add %5, %5
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, ImageQuerySamples) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability Sampled1D
           OpCapability ImageQuery
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %wg "wg"
           OpDecorate %wg DescriptorSet 2
           OpDecorate %wg Binding 0
    %int = OpTypeInt 32 1
  %float = OpTypeFloat 32
    %tex = OpTypeImage %float 2D 0 0 1 1 R32f
%ptr_tex = OpTypePointer UniformConstant %tex
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void

     %wg = OpVariable %ptr_tex UniformConstant

   %main = OpFunction %void None %voidfn
  %entry = OpLabel
     %im = OpLoad %tex %wg
 %result = OpImageQuerySamples %int %im
     %r2 = OpIAdd %int %result %result
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %wg:ptr<handle, texture_multisampled_2d<f32>, read> = var undef @binding_point(2, 0)
}

%main = @fragment func():void {
  $B2: {
    %3:texture_multisampled_2d<f32> = load %wg
    %4:u32 = textureNumSamples %3
    %5:i32 = convert %4
    %6:i32 = add %5, %5
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, ImageQuerySizeLod) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability Sampled1D
           OpCapability ImageQuery
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %wg "wg"
           OpDecorate %wg DescriptorSet 2
           OpDecorate %wg Binding 0
    %int = OpTypeInt 32 1
  %float = OpTypeFloat 32
    %tex = OpTypeImage %float 1D 0 0 0 1 R32f
%ptr_tex = OpTypePointer UniformConstant %tex
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void

     %wg = OpVariable %ptr_tex UniformConstant

  %int_1 = OpConstant %int 1

   %main = OpFunction %void None %voidfn
  %entry = OpLabel
     %im = OpLoad %tex %wg
 %result = OpImageQuerySizeLod %int %im %int_1
     %r2 = OpIAdd %int %result %result
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %wg:ptr<handle, texture_1d<f32>, read> = var undef @binding_point(2, 0)
}

%main = @fragment func():void {
  $B2: {
    %3:texture_1d<f32> = load %wg
    %4:u32 = textureDimensions %3, 1i
    %5:i32 = convert %4
    %6:i32 = add %5, %5
    ret
  }
}
)");
}

struct ImgData {
    std::string name;
    std::string spirv_type;
    std::string spirv_fn;
    std::string wgsl_type;
    std::string wgsl_fn;
};
[[maybe_unused]] std::ostream& operator<<(std::ostream& out, const ImgData& d) {
    out << d.name;
    return out;
}

using SamplerTest = SpirvReaderTestWithParam<ImgData>;
TEST_P(SamplerTest, Handle) {
    auto& params = GetParam();

    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability Sampled1D
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %10 "wg"
           OpDecorate %var_sampler DescriptorSet 0
           OpDecorate %var_sampler Binding 0
           OpDecorate %10 DescriptorSet 2
           OpDecorate %10 Binding 0
    %int = OpTypeInt 32 1
   %uint = OpTypeInt 32 0
  %float = OpTypeFloat 32
  %v2int = OpTypeVector %int 2
 %v2uint = OpTypeVector %uint 2
%v2float = OpTypeVector %float 2
%v3float = OpTypeVector %float 3
%v4float = OpTypeVector %float 4
    %tex = OpTypeImage )" +
                  params.spirv_type + R"(
%ptr_tex = OpTypePointer UniformConstant %tex
%sampled_img = OpTypeSampledImage %tex
%sampler = OpTypeSampler
%ptr_sampler = OpTypePointer UniformConstant %sampler
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void

  %int_1 = OpConstant %int 1
%float_null = OpConstantNull %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%coords2 = OpConstantComposite %v2float %float_1 %float_2
%coords3 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%coords4 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4

%vf12 = OpConstantComposite %v2float %float_1 %float_2
%vf21 = OpConstantComposite %v2float %float_2 %float_1

 %int_10 = OpConstant %int 10
 %int_11 = OpConstant %int 11
%offset2i = OpConstantComposite %v2int %int_10 %int_11

%uint_20 = OpConstant %uint 20
%uint_21 = OpConstant %uint 21
%offset2u = OpConstantComposite %v2uint %uint_20 %uint_21

%depth = OpConstant %float 1

%var_sampler = OpVariable %ptr_sampler UniformConstant
     %10 = OpVariable %ptr_tex UniformConstant

   %main = OpFunction %void None %voidfn
  %entry = OpLabel
    %sam = OpLoad %sampler %var_sampler
     %im = OpLoad %tex %10
%sampled_image = OpSampledImage %sampled_img %im %sam
 %result = )" + params.spirv_fn +
                  R"(
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %1:ptr<handle, sampler, read> = var undef @binding_point(0, 0)
  %wg:ptr<handle, )" +
                  params.wgsl_type + R"(, read> = var undef @binding_point(2, 0)
}

%main = @fragment func():void {
  $B2: {
    %4:sampler = load %1
    %5:)" + params.wgsl_type +
                  R"( = load %wg)" + params.wgsl_fn +
                  R"(
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_ImageGather,
    SamplerTest,
    ::testing::Values(
        ImgData{
            .name = "2D",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageGather %v4float %sampled_image %coords2 %int_1",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureGather 1i, %5, %4, vec2<f32>(1.0f, 2.0f))",
        },
        ImgData{
            .name = "2D ConstOffset Signed",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn =
                "OpImageGather %v4float %sampled_image %coords2 %int_1 ConstOffset %offset2i",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureGather 1i, %5, %4, vec2<f32>(1.0f, 2.0f), vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D ConstOffset Unsigned",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn =
                "OpImageGather %v4float %sampled_image %coords2 %int_1 ConstOffset %offset2u",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec2<i32> = convert vec2<u32>(20u, 21u)
    %7:vec4<f32> = textureGather 1i, %5, %4, vec2<f32>(1.0f, 2.0f), %6)",
        },
        ImgData{
            .name = "2D Array",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn = "OpImageGather %v4float %sampled_image %coords3 %int_1",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureGather 1i, %5, %4, %6, %8)",
        },
        ImgData{
            .name = "2D Array ConstOffset Signed",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn =
                "OpImageGather %v4float %sampled_image %coords3 %int_1 ConstOffset %offset2i",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureGather 1i, %5, %4, %6, %8, vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D Array ConstOffset Unsigned",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn =
                "OpImageGather %v4float %sampled_image %coords3 %int_1 ConstOffset %offset2u",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec2<i32> = convert vec2<u32>(20u, 21u)
    %10:vec4<f32> = textureGather 1i, %5, %4, %6, %8, %9)",
        },
        ImgData{
            .name = "2D Depth",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn = "OpImageGather %v4float %sampled_image %coords2 %int_1",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureGather %5, %4, vec2<f32>(1.0f, 2.0f))",
        },
        ImgData{
            .name = "2D Depth ConstOffset Signed",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn =
                "OpImageGather %v4float %sampled_image %coords2 %int_1 ConstOffset %offset2i",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureGather %5, %4, vec2<f32>(1.0f, 2.0f), vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D Depth ConstOffset Unsigned",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn =
                "OpImageGather %v4float %sampled_image %coords2 %int_1 ConstOffset %offset2u",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:vec2<i32> = convert vec2<u32>(20u, 21u)
    %7:vec4<f32> = textureGather %5, %4, vec2<f32>(1.0f, 2.0f), %6)",
        },
        ImgData{
            .name = "2D Depth Array",
            .spirv_type = "%float 2D 1 1 0 1 Unknown",
            .spirv_fn = "OpImageGather %v4float %sampled_image %coords3 %int_1",
            .wgsl_type = "texture_depth_2d_array",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureGather %5, %4, %6, %8)",
        },
        ImgData{
            .name = "2D Depth Array ConstOffset Signed",
            .spirv_type = "%float 2D 1 1 0 1 Unknown",
            .spirv_fn =
                "OpImageGather %v4float %sampled_image %coords3 %int_1 ConstOffset %offset2i",
            .wgsl_type = "texture_depth_2d_array",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureGather %5, %4, %6, %8, vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D Depth Array ConstOffset Unsigned",
            .spirv_type = "%float 2D 1 1 0 1 Unknown",
            .spirv_fn =
                "OpImageGather %v4float %sampled_image %coords3 %int_1 ConstOffset %offset2u",
            .wgsl_type = "texture_depth_2d_array",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec2<i32> = convert vec2<u32>(20u, 21u)
    %10:vec4<f32> = textureGather %5, %4, %6, %8, %9)",
        },
        ImgData{
            .name = "Cube",
            .spirv_type = "%float Cube 0 0 0 1 Unknown",
            .spirv_fn = "OpImageGather %v4float %sampled_image %coords3 %int_1",
            .wgsl_type = "texture_cube<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureGather 1i, %5, %4, vec3<f32>(1.0f, 2.0f, 3.0f))",
        },
        ImgData{
            .name = "Cube Depth",
            .spirv_type = "%float Cube 1 0 0 1 Unknown",
            .spirv_fn = "OpImageGather %v4float %sampled_image %coords3 %int_1",
            .wgsl_type = "texture_depth_cube",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureGather %5, %4, vec3<f32>(1.0f, 2.0f, 3.0f))",
        },
        ImgData{
            .name = "Cube Array",
            .spirv_type = "%float Cube 0 1 0 1 Unknown",
            .spirv_fn = "OpImageGather %v4float %sampled_image %coords4 %int_1",
            .wgsl_type = "texture_cube_array<f32>",
            .wgsl_fn = R"(
    %6:vec3<f32> = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), xyz
    %7:f32 = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), w
    %8:i32 = convert %7
    %9:vec4<f32> = textureGather 1i, %5, %4, %6, %8)",
        },
        ImgData{
            .name = "Cube Depth Array",
            .spirv_type = "%float Cube 1 1 0 1 Unknown",
            .spirv_fn = "OpImageGather %v4float %sampled_image %coords4 %int_1",
            .wgsl_type = "texture_depth_cube_array",
            .wgsl_fn = R"(
    %6:vec3<f32> = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), xyz
    %7:f32 = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), w
    %8:i32 = convert %7
    %9:vec4<f32> = textureGather %5, %4, %6, %8)",
        }));

INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_ImageSampleImplicitLod,
    SamplerTest,
    ::testing::Values(
        ImgData{
            .name = "2D",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleImplicitLod %v4float %sampled_image %coords2",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSample %5, %4, vec2<f32>(1.0f, 2.0f))",
        },
        ImgData{
            .name = "2D ConstOffset",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn =
                "OpImageSampleImplicitLod %v4float %sampled_image %coords2 ConstOffset %offset2i",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSample %5, %4, vec2<f32>(1.0f, 2.0f), vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D Bias",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleImplicitLod %v4float %sampled_image %coords2 Bias %float_5",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSampleBias %5, %4, vec2<f32>(1.0f, 2.0f), 5.0f)",
        },
        ImgData{
            .name = "2D Bias ConstOffset Signed",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleImplicitLod %v4float %sampled_image %coords2 "
                        "Bias|ConstOffset %float_5 %offset2i",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSampleBias %5, %4, vec2<f32>(1.0f, 2.0f), 5.0f, vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D Bias ConstOffset Unsigned",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleImplicitLod %v4float %sampled_image %coords2 "
                        "Bias|ConstOffset %float_5 %offset2u",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec2<i32> = convert vec2<u32>(20u, 21u)
    %7:vec4<f32> = textureSampleBias %5, %4, vec2<f32>(1.0f, 2.0f), 5.0f, %6)",
        },
        ImgData{
            .name = "2D Array",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn = "OpImageSampleImplicitLod %v4float %sampled_image %coords3",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureSample %5, %4, %6, %8)",
        },
        ImgData{
            .name = "2D Array ConstOffset",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn =
                "OpImageSampleImplicitLod %v4float %sampled_image %coords3 ConstOffset %offset2i",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureSample %5, %4, %6, %8, vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D Array Bias",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn = "OpImageSampleImplicitLod %v4float %sampled_image %coords3 Bias %float_5",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureSampleBias %5, %4, %6, %8, 5.0f)",
        },
        ImgData{
            .name = "2D Array Bias ConstOffset",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn = "OpImageSampleImplicitLod %v4float %sampled_image %coords3 "
                        "Bias|ConstOffset %float_5 %offset2i",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureSampleBias %5, %4, %6, %8, 5.0f, vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D Array Bias ConstOffset",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn = "OpImageSampleImplicitLod %v4float %sampled_image %coords3 "
                        "Bias|ConstOffset %float_5 %offset2i",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn =
                R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureSampleBias %5, %4, %6, %8, 5.0f, vec2<i32>(10i, 11i))",
        }));

INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_ImageSampleProjImplicitLod,
    SamplerTest,
    ::testing::Values(
        ImgData{
            .name = "1D",
            .spirv_type = "%float 1D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleProjImplicitLod %v4float %sampled_image %coords2",
            .wgsl_type = "texture_1d<f32>",
            .wgsl_fn = R"(
    %6:f32 = swizzle vec2<f32>(1.0f, 2.0f), x
    %7:f32 = swizzle vec2<f32>(1.0f, 2.0f), y
    %8:f32 = div %6, %7
    %9:vec4<f32> = textureSample %5, %4, %8)",
        },
        ImgData{
            .name = "2D",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleProjImplicitLod %v4float %sampled_image %coords3",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:vec2<f32> = div %6, %7
    %9:vec4<f32> = textureSample %5, %4, %8)",
        },
        ImgData{
            .name = "3D",
            .spirv_type = "%float 3D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleProjImplicitLod %v4float %sampled_image %coords4",
            .wgsl_type = "texture_3d<f32>",
            .wgsl_fn = R"(
    %6:vec3<f32> = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), xyz
    %7:f32 = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), w
    %8:vec3<f32> = div %6, %7
    %9:vec4<f32> = textureSample %5, %4, %8)",
        },
        ImgData{
            .name = "2D ConstOffset",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleProjImplicitLod %v4float %sampled_image %coords3 ConstOffset "
                        "%offset2i",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:vec2<f32> = div %6, %7
    %9:vec4<f32> = textureSample %5, %4, %8, vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D Bias",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn =
                "OpImageSampleProjImplicitLod %v4float %sampled_image %coords3 Bias %float_5",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:vec2<f32> = div %6, %7
    %9:vec4<f32> = textureSampleBias %5, %4, %8, 5.0f)",
        },
        ImgData{
            .name = "2D Bias ConstOffset Signed",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleProjImplicitLod %v4float %sampled_image %coords3 "
                        "Bias|ConstOffset %float_5 %offset2i",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn =
                R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:vec2<f32> = div %6, %7
    %9:vec4<f32> = textureSampleBias %5, %4, %8, 5.0f, vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D Bias ConstOffset Unsigned",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleProjImplicitLod %v4float %sampled_image %coords3 "
                        "Bias|ConstOffset %float_5 %offset2u",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn =
                R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:vec2<f32> = div %6, %7
    %9:vec2<i32> = convert vec2<u32>(20u, 21u)
    %10:vec4<f32> = textureSampleBias %5, %4, %8, 5.0f, %9)",
        },
        ImgData{
            .name = "2D Depth",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleProjImplicitLod %v4float %sampled_image %coords3",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:vec2<f32> = div %6, %7
    %9:f32 = textureSample %5, %4, %8
    %10:vec4<f32> = construct %9, 0.0f, 0.0f, 0.0f)",
        }));

INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_ImageSampleExplicitLod,
    SamplerTest,
    ::testing::Values(
        ImgData{
            .name = "2D Lod",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleExplicitLod %v4float %sampled_image %coords2 Lod %float_null",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSampleLevel %5, %4, vec2<f32>(1.0f, 2.0f), 0.0f)",
        },
        ImgData{
            .name = "2D Array Lod",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn = "OpImageSampleExplicitLod %v4float %sampled_image %coords3 Lod %float_null",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureSampleLevel %5, %4, %6, %8, 0.0f)",
        },
        ImgData{
            .name = "2D Lod ConstOffset signed",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleExplicitLod %v4float %sampled_image %coords2 Lod|ConstOffset "
                        "%float_null %offset2i",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSampleLevel %5, %4, vec2<f32>(1.0f, 2.0f), 0.0f, vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D lod ConstOffset unsigned",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleExplicitLod %v4float %sampled_image %coords2 Lod|ConstOffset "
                        "%float_null %offset2u",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec2<i32> = convert vec2<u32>(20u, 21u)
    %7:vec4<f32> = textureSampleLevel %5, %4, vec2<f32>(1.0f, 2.0f), 0.0f, %6)",
        },
        ImgData{
            .name = "2D Array lod ConstOffset",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn = "OpImageSampleExplicitLod %v4float %sampled_image %coords3 Lod|ConstOffset "
                        "%float_null %offset2i",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureSampleLevel %5, %4, %6, %8, 0.0f, vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D Grad",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn =
                "OpImageSampleExplicitLod %v4float %sampled_image %coords2 Grad %vf12 %vf21",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSampleGrad %5, %4, vec2<f32>(1.0f, 2.0f), vec2<f32>(1.0f, 2.0f), vec2<f32>(2.0f, 1.0f))",
        },
        ImgData{
            .name = "2D Array Grad",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn =
                "OpImageSampleExplicitLod %v4float %sampled_image %coords3 Grad %vf12 %vf21",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureSampleGrad %5, %4, %6, %8, vec2<f32>(1.0f, 2.0f), vec2<f32>(2.0f, 1.0f))",
        },
        ImgData{
            .name = "2D Grad ConstOffset signed",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleExplicitLod %v4float %sampled_image %coords2 "
                        "Grad|ConstOffset %vf12 %vf21 %offset2i",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSampleGrad %5, %4, vec2<f32>(1.0f, 2.0f), vec2<f32>(1.0f, 2.0f), vec2<f32>(2.0f, 1.0f), vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D Grad ConstOffset Unsigned",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleExplicitLod %v4float %sampled_image %coords2 "
                        "Grad|ConstOffset %vf12 %vf21 %offset2u",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec2<i32> = convert vec2<u32>(20u, 21u)
    %7:vec4<f32> = textureSampleGrad %5, %4, vec2<f32>(1.0f, 2.0f), vec2<f32>(1.0f, 2.0f), vec2<f32>(2.0f, 1.0f), %6)",
        },
        ImgData{
            .name = "2D Array Grad ConstOffset",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn = "OpImageSampleExplicitLod %v4float %sampled_image %coords3 "
                        "Grad|ConstOffset %vf12 %vf21 %offset2i",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureSampleGrad %5, %4, %6, %8, vec2<f32>(1.0f, 2.0f), vec2<f32>(2.0f, 1.0f), vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D Array Grad ConstOffset Unsigned",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn = "OpImageSampleExplicitLod %v4float %sampled_image %coords3 "
                        "Grad|ConstOffset %vf12 %vf21 %offset2u",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec2<i32> = convert vec2<u32>(20u, 21u)
    %10:vec4<f32> = textureSampleGrad %5, %4, %6, %8, vec2<f32>(1.0f, 2.0f), vec2<f32>(2.0f, 1.0f), %9)",
        },
        ImgData{
            .name = "2D Depth",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleExplicitLod %v4float %sampled_image %vf12 Lod %float_1",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:i32 = convert 1.0f
    %7:f32 = textureSampleLevel %5, %4, vec2<f32>(1.0f, 2.0f), %6
    %8:vec4<f32> = construct %7, 0.0f, 0.0f, 0.0f)",
        }));

INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_ImageSampleProjExplicitLod,
    SamplerTest,
    ::testing::Values(
        ImgData{
            .name = "2D",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn =
                "OpImageSampleProjExplicitLod %v4float %sampled_image %coords3 Lod %float_1",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:vec2<f32> = div %6, %7
    %9:vec4<f32> = textureSampleLevel %5, %4, %8, 1.0f)",
        },
        ImgData{
            .name = "2D Lod ConstOffset",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleProjExplicitLod %v4float %sampled_image %coords3 "
                        "Lod|ConstOffset %float_1 %offset2i",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:vec2<f32> = div %6, %7
    %9:vec4<f32> = textureSampleLevel %5, %4, %8, 1.0f, vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D Grad",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn =
                "OpImageSampleProjExplicitLod %v4float %sampled_image %coords3 Grad %vf12 %vf21",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:vec2<f32> = div %6, %7
    %9:vec4<f32> = textureSampleGrad %5, %4, %8, vec2<f32>(1.0f, 2.0f), vec2<f32>(2.0f, 1.0f))",
        },
        ImgData{
            .name = "2D Lod Grad ConstOffset",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleProjExplicitLod %v4float %sampled_image %coords3 "
                        "Grad|ConstOffset %vf12 %vf21 %offset2i",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:vec2<f32> = div %6, %7
    %9:vec4<f32> = textureSampleGrad %5, %4, %8, vec2<f32>(1.0f, 2.0f), vec2<f32>(2.0f, 1.0f), vec2<i32>(10i, 11i))",
        }));

using SamplerComparisonTest = SpirvReaderTestWithParam<ImgData>;
TEST_P(SamplerComparisonTest, Handle) {
    auto& params = GetParam();

    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability Sampled1D
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %10 "wg"
           OpDecorate %var_sampler DescriptorSet 0
           OpDecorate %var_sampler Binding 0
           OpDecorate %10 DescriptorSet 2
           OpDecorate %10 Binding 0
    %int = OpTypeInt 32 1
   %uint = OpTypeInt 32 0
  %float = OpTypeFloat 32
  %v2int = OpTypeVector %int 2
 %v2uint = OpTypeVector %uint 2
%v2float = OpTypeVector %float 2
%v3float = OpTypeVector %float 3
%v4float = OpTypeVector %float 4
    %tex = OpTypeImage )" +
                  params.spirv_type +
                  R"(
%ptr_tex = OpTypePointer UniformConstant %tex
%sampled_img = OpTypeSampledImage %tex
%sampler = OpTypeSampler
%ptr_sampler = OpTypePointer UniformConstant %sampler
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void

  %int_1 = OpConstant %int 1
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_null = OpConstantNull %float
%coords2 = OpConstantComposite %v2float %float_1 %float_2
%coords3 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%coords4 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4

 %int_10 = OpConstant %int 10
 %int_11 = OpConstant %int 11
%offset2i = OpConstantComposite %v2int %int_10 %int_11

%uint_20 = OpConstant %uint 20
%uint_21 = OpConstant %uint 21
%offset2u = OpConstantComposite %v2uint %uint_20 %uint_21

%depth = OpConstant %float 1

%var_sampler = OpVariable %ptr_sampler UniformConstant
     %10 = OpVariable %ptr_tex UniformConstant

   %main = OpFunction %void None %voidfn
  %entry = OpLabel
    %sam = OpLoad %sampler %var_sampler
     %im = OpLoad %tex %10
%sampled_image = OpSampledImage %sampled_img %im %sam
 %result = )" + params.spirv_fn +
                  R"(
           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %1:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 0)
  %wg:ptr<handle, )" +
                  params.wgsl_type +
                  R"(, read> = var undef @binding_point(2, 0)
}

%main = @fragment func():void {
  $B2: {
    %4:sampler_comparison = load %1
    %5:)" + params.wgsl_type +
                  R"( = load %wg)" + params.wgsl_fn +
                  R"(
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_ImageDrefGather,
    SamplerComparisonTest,
    ::testing::Values(
        ImgData{
            .name = "2d Depth",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn = "OpImageDrefGather %v4float %sampled_image %coords2 %depth",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureGatherCompare %5, %4, vec2<f32>(1.0f, 2.0f), 1.0f)",
        },
        ImgData{
            .name = "2d Depth ConstOffset Signed",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn =
                "OpImageDrefGather %v4float %sampled_image %coords2 %depth ConstOffset %offset2i",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureGatherCompare %5, %4, vec2<f32>(1.0f, 2.0f), 1.0f, vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2d Depth ConstOffset Unsigned",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn =
                "OpImageDrefGather %v4float %sampled_image %coords2 %depth ConstOffset %offset2u",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:vec2<i32> = convert vec2<u32>(20u, 21u)
    %7:vec4<f32> = textureGatherCompare %5, %4, vec2<f32>(1.0f, 2.0f), 1.0f, %6)",
        },
        ImgData{
            .name = "2d Depth Array",
            .spirv_type = "%float 2D 1 1 0 1 Unknown",
            .spirv_fn = "OpImageDrefGather %v4float %sampled_image %coords3 %depth",
            .wgsl_type = "texture_depth_2d_array",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureGatherCompare %5, %4, %6, %8, 1.0f)",
        },
        ImgData{
            .name = "2d Depth Array ConstOffset Signed",
            .spirv_type = "%float 2D 1 1 0 1 Unknown",
            .spirv_fn =
                "OpImageDrefGather %v4float %sampled_image %coords3 %depth ConstOffset %offset2i",
            .wgsl_type = "texture_depth_2d_array",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureGatherCompare %5, %4, %6, %8, 1.0f, vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2d Depth Array ConstOffset Unsigned",
            .spirv_type = "%float 2D 1 1 0 1 Unknown",
            .spirv_fn =
                "OpImageDrefGather %v4float %sampled_image %coords3 %depth ConstOffset %offset2u",
            .wgsl_type = "texture_depth_2d_array",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec2<i32> = convert vec2<u32>(20u, 21u)
    %10:vec4<f32> = textureGatherCompare %5, %4, %6, %8, 1.0f, %9)",
        },
        ImgData{
            .name = "Cube Depth",
            .spirv_type = "%float Cube 1 0 0 1 Unknown",
            .spirv_fn = "OpImageDrefGather %v4float %sampled_image %coords3 %depth",
            .wgsl_type = "texture_depth_cube",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureGatherCompare %5, %4, vec3<f32>(1.0f, 2.0f, 3.0f), 1.0f)",
        },
        ImgData{
            .name = "Cube Depth Array",
            .spirv_type = "%float Cube 1 1 0 1 Unknown",
            .spirv_fn = "OpImageDrefGather %v4float %sampled_image %coords4 %depth",
            .wgsl_type = "texture_depth_cube_array",
            .wgsl_fn = R"(
    %6:vec3<f32> = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), xyz
    %7:f32 = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), w
    %8:i32 = convert %7
    %9:vec4<f32> = textureGatherCompare %5, %4, %6, %8, 1.0f)",
        }));

INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_ImageSampleDrefImplicitLod,
    SamplerComparisonTest,
    ::testing::Values(
        ImgData{
            .name = "2D",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleDrefImplicitLod %float %sampled_image %coords2 %depth",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:f32 = textureSampleCompare %5, %4, vec2<f32>(1.0f, 2.0f), 1.0f)",
        },
        ImgData{
            .name = "2D ConstOffset",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleDrefImplicitLod %float %sampled_image %coords2 %depth "
                        "ConstOffset %offset2i",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:f32 = textureSampleCompare %5, %4, vec2<f32>(1.0f, 2.0f), 1.0f, vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D Array",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn = "OpImageSampleDrefImplicitLod %float %sampled_image %coords3 %depth",
            .wgsl_type = "texture_depth_2d_array",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:f32 = textureSampleCompare %5, %4, %6, %8, 1.0f)",
        },
        ImgData{
            .name = "2D Array ConstOffset",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn = "OpImageSampleDrefImplicitLod %float %sampled_image %coords3 %depth "
                        "ConstOffset %offset2i",
            .wgsl_type = "texture_depth_2d_array",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:f32 = textureSampleCompare %5, %4, %6, %8, 1.0f, vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D vec2 depth",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleDrefImplicitLod %float %sampled_image %coords2 %float_1",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:f32 = textureSampleCompare %5, %4, vec2<f32>(1.0f, 2.0f), 1.0f)",
        }));

INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_ImageSampleDrefExplicitLod,
    SamplerComparisonTest,
    ::testing::Values(
        ImgData{
            .name = "2D",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn =
                "OpImageSampleDrefExplicitLod %float %sampled_image %coords2 %depth Lod %float_0",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:f32 = textureSampleCompareLevel %5, %4, vec2<f32>(1.0f, 2.0f), 1.0f)",
        },
        ImgData{
            .name = "2D array",
            .spirv_type = "%float 2D 1 1 0 1 Unknown",
            .spirv_fn =
                "OpImageSampleDrefExplicitLod %float %sampled_image %coords3 %depth Lod %float_0",
            .wgsl_type = "texture_depth_2d_array",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:f32 = textureSampleCompareLevel %5, %4, %6, %8, 1.0f)",
        },
        ImgData{
            .name = "2D ConstOffset",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleDrefExplicitLod %float %sampled_image %coords2 %depth "
                        "Lod|ConstOffset %float_0 %offset2i",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:f32 = textureSampleCompareLevel %5, %4, vec2<f32>(1.0f, 2.0f), 1.0f, vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "2D array ConstOffset",
            .spirv_type = "%float 2D 1 1 0 1 Unknown",
            .spirv_fn = "OpImageSampleDrefExplicitLod %float %sampled_image %coords3 %depth "
                        "Lod|ConstOffset %float_0 %offset2i",
            .wgsl_type = "texture_depth_2d_array",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:f32 = textureSampleCompareLevel %5, %4, %6, %8, 1.0f, vec2<i32>(10i, 11i))",
        },
        ImgData{
            .name = "Cube",
            .spirv_type = "%float Cube 1 0 0 1 Unknown",
            .spirv_fn =
                "OpImageSampleDrefExplicitLod %float %sampled_image %coords3 %depth Lod %float_0",
            .wgsl_type = "texture_depth_cube",
            .wgsl_fn = R"(
    %6:f32 = textureSampleCompareLevel %5, %4, vec3<f32>(1.0f, 2.0f, 3.0f), 1.0f)",
        },
        ImgData{
            .name = "Cube array",
            .spirv_type = "%float Cube 1 1 0 1 Unknown",
            .spirv_fn =
                "OpImageSampleDrefExplicitLod %float %sampled_image %coords4 %depth Lod %float_0",
            .wgsl_type = "texture_depth_cube_array",
            .wgsl_fn = R"(
    %6:vec3<f32> = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), xyz
    %7:f32 = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), w
    %8:i32 = convert %7
    %9:f32 = textureSampleCompareLevel %5, %4, %6, %8, 1.0f)",
        },
        ImgData{
            .name = "2d vec2 depth lod",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn =
                "OpImageSampleDrefExplicitLod %float %sampled_image %coords2 %float_1 Lod %float_0",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:f32 = textureSampleCompareLevel %5, %4, vec2<f32>(1.0f, 2.0f), 1.0f)",
        }

        ));

INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_ImageSampleProjDrefImplicitLod,
    SamplerComparisonTest,
    ::testing::Values(
        ImgData{
            .name = "2D Depth",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleProjDrefImplicitLod %float %sampled_image %coords3 %float_1",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:vec2<f32> = div %6, %7
    %9:f32 = textureSampleCompare %5, %4, %8, 1.0f)",
        },
        ImgData{
            .name = "2D Depth ConstOffset",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleProjDrefImplicitLod %float %sampled_image %coords3 %float_1 "
                        "ConstOffset %offset2i",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:vec2<f32> = div %6, %7
    %9:f32 = textureSampleCompare %5, %4, %8, 1.0f, vec2<i32>(10i, 11i))",
        }));

INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_ImageSampleProjDrefExplicitLod,
    SamplerComparisonTest,
    ::testing::Values(
        ImgData{
            .name = "2D Depth Lod",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleProjDrefExplicitLod %float %sampled_image %coords3 %float_1 "
                        "Lod %float_0",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:vec2<f32> = div %6, %7
    %9:f32 = textureSampleCompareLevel %5, %4, %8, 1.0f)",
        },
        ImgData{
            .name = "2D Depth Lod ConstOffset",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleProjDrefExplicitLod %float %sampled_image %coords3 %float_1 "
                        "Lod|ConstOffset %float_0 %offset2i",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:vec2<f32> = div %6, %7
    %9:f32 = textureSampleCompareLevel %5, %4, %8, 1.0f, vec2<i32>(10i, 11i))",
        }));

// Metal requires comparison sampling with explicit Level-of-detail to use Lod 0.  The SPIR-V reader
// requires the operand to be parsed as a constant 0 value. SPIR-V validation requires the Lod
// parameter to be a floating point value for non-fetch operations. So only test float values.
INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_ImageSampleDrefExplicitLod_CheckForLod0,
    SamplerComparisonTest,
    ::testing::Values(
        ImgData{
            .name = "2d 0.0",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn =
                "OpImageSampleDrefExplicitLod %float %sampled_image %coords4 %float_1 Lod %float_0",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), xy
    %7:f32 = textureSampleCompareLevel %5, %4, %6, 1.0f)",
        },
        ImgData{
            .name = "2D null",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleDrefExplicitLod %float %sampled_image %coords4 %float_1 Lod "
                        "%float_null",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), xy
    %7:f32 = textureSampleCompareLevel %5, %4, %6, 1.0f)",
        }));

// This is like the previous test, but for Projection sampling.
//
// Metal requires comparison sampling with explicit Level-of-detail to use Lod 0.  The SPIR-V reader
// requires the operand to be parsed as a constant 0 value. SPIR-V validation requires the Lod
// parameter to be a floating point value for non-fetch operations. So only test float values.
INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_ImageSampleProjDrefExplicitLod_CheckForLod0,
    SamplerComparisonTest,
    ::testing::Values(
        ImgData{
            .name = "2D 0.0",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleProjDrefExplicitLod %float %sampled_image %coords4 %float_1 "
                        "Lod %float_0",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), xy
    %7:f32 = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), z
    %8:vec2<f32> = div %6, %7
    %9:f32 = textureSampleCompareLevel %5, %4, %8, 1.0f)",
        },
        ImgData{
            .name = "2D null",
            // float null works
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn = "OpImageSampleProjDrefExplicitLod %float %sampled_image %coords4 %float_1 "
                        "Lod %float_null",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), xy
    %7:f32 = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), z
    %8:vec2<f32> = div %6, %7
    %9:f32 = textureSampleCompareLevel %5, %4, %8, 1.0f)",
        }));

// This test shows the use of a sampled image used with both regular
// sampling and depth-reference sampling.  The texture is a depth-texture,
// and we use builtins textureSample and textureSampleCompare
TEST_F(SpirvReaderTest, ImageSampleImplicitLod_BothDrefAndNonDref) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability Sampled1D
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %main "main"
           OpExecutionMode %main OriginUpperLeft
           OpName %10 "wg"
           OpDecorate %var_sampler1 DescriptorSet 0
           OpDecorate %var_sampler1 Binding 0
           OpDecorate %var_sampler2 DescriptorSet 0
           OpDecorate %var_sampler2 Binding 1
           OpDecorate %10 DescriptorSet 2
           OpDecorate %10 Binding 0
  %float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
    %tex = OpTypeImage %float 2D 0 0 0 1 Unknown
%ptr_tex = OpTypePointer UniformConstant %tex
%sampled_img = OpTypeSampledImage %tex
%sampler = OpTypeSampler
%ptr_sampler = OpTypePointer UniformConstant %sampler
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void

%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%coords2 = OpConstantComposite %v2float %float_1 %float_2

%depth = OpConstant %float 1

%var_sampler1 = OpVariable %ptr_sampler UniformConstant
%var_sampler2 = OpVariable %ptr_sampler UniformConstant
     %10 = OpVariable %ptr_tex UniformConstant

   %main = OpFunction %void None %voidfn
  %entry = OpLabel
    %sam = OpLoad %sampler %var_sampler1
     %im = OpLoad %tex %10
%sampled_image = OpSampledImage %sampled_img %im %sam

%sam_dref = OpLoad %sampler %var_sampler2
%sampled_dref_image = OpSampledImage %sampled_img %im %sam_dref

    %200 = OpImageSampleImplicitLod %v4float %sampled_image %coords2
    %210 = OpImageSampleDrefImplicitLod %float %sampled_dref_image %coords2 %depth

           OpReturn
           OpFunctionEnd
        )",
              R"(
$B1: {  # root
  %1:ptr<handle, sampler, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
  %wg:ptr<handle, texture_depth_2d, read> = var undef @binding_point(2, 0)
}

%main = @fragment func():void {
  $B2: {
    %5:sampler = load %1
    %6:texture_depth_2d = load %wg
    %7:sampler_comparison = load %2
    %8:f32 = textureSample %6, %5, vec2<f32>(1.0f, 2.0f)
    %9:vec4<f32> = construct %8, 0.0f, 0.0f, 0.0f
    %10:f32 = textureSampleCompare %6, %7, vec2<f32>(1.0f, 2.0f), 1.0f
    ret
  }
}
)");
}

using ImageAccessTest = SpirvReaderTestWithParam<ImgData>;
TEST_P(ImageAccessTest, Variable) {
    auto& params = GetParam();

    EXPECT_IR(R"(
            OpCapability Shader
            OpCapability StorageImageExtendedFormats
            OpMemoryModel Logical Simple
            OpEntryPoint Fragment %main "main"
            OpExecutionMode %main OriginUpperLeft
            OpName %main "main"
            OpName %wg "wg"
            OpName %offset2i "offset2i"
            OpDecorate %wg DescriptorSet 2
            OpDecorate %wg Binding 1
    %void = OpTypeVoid
   %float = OpTypeFloat 32
     %int = OpTypeInt 32  1
    %uint = OpTypeInt 32  0
   %v2int = OpTypeVector %int 2
   %v3int = OpTypeVector %int 3
   %v4int = OpTypeVector %int 4
  %v2uint = OpTypeVector %uint 2
  %v3uint = OpTypeVector %uint 3
  %v4uint = OpTypeVector %uint 4
 %v2float = OpTypeVector %float 2
 %v3float = OpTypeVector %float 3
 %v4float = OpTypeVector %float 4
   %im_ty = OpTypeImage )" +
                  params.spirv_type +
                  R"(
%ptr_im_ty = OpTypePointer UniformConstant %im_ty
  %voidfn = OpTypeFunction %void
   %int_1 = OpConstant %int 1
   %int_2 = OpConstant %int 2
   %int_3 = OpConstant %int 3
   %int_4 = OpConstant %int 4
  %int_10 = OpConstant %int 10
  %int_11 = OpConstant %int 11
  %uint_1 = OpConstant %uint 1
  %uint_2 = OpConstant %uint 2
  %uint_3 = OpConstant %uint 3
  %uint_4 = OpConstant %uint 4
 %float_1 = OpConstant %float 1
 %float_2 = OpConstant %float 2
 %float_3 = OpConstant %float 3
 %float_4 = OpConstant %float 4
      %wg = OpVariable %ptr_im_ty UniformConstant
%offset2i = OpConstantComposite %v2int %int_10 %int_11
    %vi12 = OpConstantComposite %v2int %int_1 %int_2
   %vi123 = OpConstantComposite %v3int %int_1 %int_2 %int_3
  %vi1234 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
    %vf12 = OpConstantComposite %v2float %float_1 %float_2
   %vf123 = OpConstantComposite %v3float %float_1 %float_2 %float_3
  %vf1234 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
    %vu12 = OpConstantComposite %v2uint %uint_1 %uint_2
   %vu123 = OpConstantComposite %v3uint %uint_1 %uint_2 %uint_3
  %vu1234 = OpConstantComposite %v4uint %uint_1 %uint_2 %uint_3 %uint_4

    %main = OpFunction %void None %voidfn
   %entry = OpLabel

      %im = OpLoad %im_ty %wg
)" + params.spirv_fn +
                  R"(
            OpReturn
            OpFunctionEnd
)",
              R"(
$B1: {  # root
  %wg:ptr<handle, )" +
                  params.wgsl_type +
                  R"(, read> = var undef @binding_point(2, 1)
}

%main = @fragment func():void {
  $B2: {
    %3:)" + params.wgsl_type +
                  R"( = load %wg)" + params.wgsl_fn +
                  R"(
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageWrite_OptionalParams,
                         ImageAccessTest,
                         ::testing::Values(ImgData{
                             .name = "No extra params",
                             .spirv_type = "%float 2D 0 0 0 2 Rgba32f",
                             .spirv_fn = "OpImageWrite %im %vi12 %vf1234",
                             .wgsl_type = "texture_storage_2d<rgba32float, read_write>",
                             .wgsl_fn = R"(
    %4:void = textureStore %3, vec2<i32>(1i, 2i), vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f))",
                         }));

// SPIR-V's texel parameter is a scalar or vector with at least as many
// components as there are channels in the underlying format, and the
// component type matches the sampled type (modulo signed/unsigned integer).
// WGSL's texel parameter is a 4-element vector scalar or vector, with
// component type equal to the 32-bit form of the channel type.
INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_ImageWrite_ConvertTexelOperand_Arity_Float,
    ImageAccessTest,
    ::testing::Values(ImgData{.name = "Source 1 component",
                              .spirv_type = "%float 2D 0 0 0 2 R32f",
                              .spirv_fn = "OpImageWrite %im %vi12 %float_1",
                              .wgsl_type = "texture_storage_2d<r32float, read_write>",
                              .wgsl_fn = R"(
    %4:vec4<f32> = construct 1.0f
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)"},
                      ImgData{
                          .name = "Source 2 component, dest 1 component",
                          .spirv_type = "%float 2D 0 0 0 2 R32f",
                          .spirv_fn = "OpImageWrite %im %vi12 %vf12",
                          .wgsl_type = "texture_storage_2d<r32float, read_write>",
                          .wgsl_fn = R"(
    %4:vec4<f32> = construct vec2<f32>(1.0f, 2.0f), vec2<f32>(0.0f)
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)",
                      },
                      ImgData{
                          .name = "Source 3 component, dest 1 component",
                          .spirv_type = "%float 2D 0 0 0 2 R32f",
                          .spirv_fn = "OpImageWrite %im %vi12 %vf123",
                          .wgsl_type = "texture_storage_2d<r32float, read_write>",
                          .wgsl_fn = R"(
    %4:vec4<f32> = construct vec3<f32>(1.0f, 2.0f, 3.0f), 0.0f
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)",
                      },
                      ImgData{
                          .name = "Source 4 component, dest 1 component",
                          .spirv_type = "%float 2D 0 0 0 2 R32f",
                          .spirv_fn = "OpImageWrite %im %vi12 %vf1234",
                          .wgsl_type = "texture_storage_2d<r32float, read_write>",
                          .wgsl_fn = R"(
    %4:void = textureStore %3, vec2<i32>(1i, 2i), vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f))",
                      },
                      ImgData{
                          .name = "Source 2 component, dest 2 component",
                          .spirv_type = "%float 2D 0 0 0 2 Rg32f",
                          .spirv_fn = "OpImageWrite %im %vi12 %vf12",
                          .wgsl_type = "texture_storage_2d<rg32float, read_write>",
                          .wgsl_fn = R"(
    %4:vec4<f32> = construct vec2<f32>(1.0f, 2.0f), vec2<f32>(0.0f)
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)",
                      },
                      ImgData{
                          .name = "Source 3 component, dest 2 component",
                          .spirv_type = "%float 2D 0 0 0 2 Rg32f",
                          .spirv_fn = "OpImageWrite %im %vi12 %vf123",
                          .wgsl_type = "texture_storage_2d<rg32float, read_write>",
                          .wgsl_fn = R"(
    %4:vec4<f32> = construct vec3<f32>(1.0f, 2.0f, 3.0f), 0.0f
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)",
                      },
                      ImgData{
                          .name = "Source 4 component, dest 2 component",
                          .spirv_type = "%float 2D 0 0 0 2 Rg32f",
                          .spirv_fn = "OpImageWrite %im %vi12 %vf1234",
                          .wgsl_type = "texture_storage_2d<rg32float, read_write>",
                          .wgsl_fn = R"(
    %4:void = textureStore %3, vec2<i32>(1i, 2i), vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f))",
                      },
                      ImgData{
                          .name = "Source 4 component, dest 4 component",
                          .spirv_type = "%float 2D 0 0 0 2 Rgba32f",
                          .spirv_fn = "OpImageWrite %im %vi12 %vf1234",
                          .wgsl_type = "texture_storage_2d<rgba32float, read_write>",
                          .wgsl_fn = R"(
    %4:void = textureStore %3, vec2<i32>(1i, 2i), vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f))",
                      }));

// As above, but unsigned integer.
INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageWrite_ConvertTexelOperand_Arity_Uint,
                         ImageAccessTest,
                         ::testing::Values(
                             ImgData{
                                 .name = "Source 1 component",
                                 .spirv_type = "%uint 2D 0 0 0 2 R32ui",
                                 .spirv_fn = "OpImageWrite %im %vi12 %uint_1",
                                 .wgsl_type = "texture_storage_2d<r32uint, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<u32> = construct 1u
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)",
                             },
                             ImgData{
                                 .name = "Source 2 component, dest 1 component",
                                 .spirv_type = "%uint 2D 0 0 0 2 R32ui",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vu12",
                                 .wgsl_type = "texture_storage_2d<r32uint, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<u32> = construct vec2<u32>(1u, 2u), vec2<u32>(0u)
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)",
                             },
                             ImgData{
                                 .name = "Source 3 component, dest 1 component",
                                 .spirv_type = "%uint 2D 0 0 0 2 R32ui",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vu123",
                                 .wgsl_type = "texture_storage_2d<r32uint, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<u32> = construct vec3<u32>(1u, 2u, 3u), 0u
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)",
                             },
                             ImgData{
                                 .name = "Source 4 component, dest 1 component",
                                 .spirv_type = "%uint 2D 0 0 0 2 R32ui",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vu1234",
                                 .wgsl_type = "texture_storage_2d<r32uint, read_write>",
                                 .wgsl_fn = R"(
    %4:void = textureStore %3, vec2<i32>(1i, 2i), vec4<u32>(1u, 2u, 3u, 4u))",
                             },
                             ImgData{
                                 .name = "Source 2 component, dest 2 component",
                                 .spirv_type = "%uint 2D 0 0 0 2 Rg32ui",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vu12",
                                 .wgsl_type = "texture_storage_2d<rg32uint, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<u32> = construct vec2<u32>(1u, 2u), vec2<u32>(0u)
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)",
                             },
                             ImgData{
                                 .name = "Source 3 component, dest 2 component",
                                 .spirv_type = "%uint 2D 0 0 0 2 Rg32ui",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vu123",
                                 .wgsl_type = "texture_storage_2d<rg32uint, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<u32> = construct vec3<u32>(1u, 2u, 3u), 0u
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)",
                             },
                             ImgData{
                                 .name = "Source 4 component, dest 2 component",
                                 .spirv_type = "%uint 2D 0 0 0 2 Rg32ui",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vu1234",
                                 .wgsl_type = "texture_storage_2d<rg32uint, read_write>",
                                 .wgsl_fn = R"(
    %4:void = textureStore %3, vec2<i32>(1i, 2i), vec4<u32>(1u, 2u, 3u, 4u))",
                             },
                             ImgData{
                                 .name = "Source 4 component, dest 4 component",
                                 .spirv_type = "%uint 2D 0 0 0 2 Rgba32ui",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vu1234",
                                 .wgsl_type = "texture_storage_2d<rgba32uint, read_write>",
                                 .wgsl_fn = R"(
    %4:void = textureStore %3, vec2<i32>(1i, 2i), vec4<u32>(1u, 2u, 3u, 4u))",
                             }));

// As above, but signed integer.
INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageWrite_ConvertTexelOperand_Arity_Sint,
                         ImageAccessTest,
                         ::testing::Values(
                             ImgData{
                                 .name = "Source 1 component",
                                 .spirv_type = "%int 2D 0 0 0 2 R32i",
                                 .spirv_fn = "OpImageWrite %im %vi12 %int_1",
                                 .wgsl_type = "texture_storage_2d<r32sint, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<i32> = construct 1i
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)",
                             },
                             ImgData{
                                 .name = "Source 2 component, dest 1 component",
                                 .spirv_type = "%int 2D 0 0 0 2 R32i",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vi12",
                                 .wgsl_type = "texture_storage_2d<r32sint, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<i32> = construct vec2<i32>(1i, 2i), vec2<i32>(0i)
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)",
                             },
                             ImgData{
                                 .name = "Source 3 component, dest 1 component",
                                 .spirv_type = "%int 2D 0 0 0 2 R32i",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vi123",
                                 .wgsl_type = "texture_storage_2d<r32sint, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<i32> = construct vec3<i32>(1i, 2i, 3i), 0i
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)",
                             },
                             ImgData{
                                 .name = "Source 4 component, dest 1 component",
                                 .spirv_type = "%int 2D 0 0 0 2 R32i",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vi1234",
                                 .wgsl_type = "texture_storage_2d<r32sint, read_write>",
                                 .wgsl_fn = R"(
    %4:void = textureStore %3, vec2<i32>(1i, 2i), vec4<i32>(1i, 2i, 3i, 4i))",
                             },
                             ImgData{
                                 .name = "Source 2 component, dest 2 component",
                                 .spirv_type = "%int 2D 0 0 0 2 Rg32i",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vi12",
                                 .wgsl_type = "texture_storage_2d<rg32sint, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<i32> = construct vec2<i32>(1i, 2i), vec2<i32>(0i)
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)",
                             },
                             ImgData{
                                 .name = "Source 3 component, dest 2 component",
                                 .spirv_type = "%int 2D 0 0 0 2 Rg32i",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vi123",
                                 .wgsl_type = "texture_storage_2d<rg32sint, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<i32> = construct vec3<i32>(1i, 2i, 3i), 0i
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)",
                             },
                             ImgData{
                                 .name = "Source 4 component, dest 2 component",
                                 .spirv_type = "%int 2D 0 0 0 2 Rg32i",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vi1234",
                                 .wgsl_type = "texture_storage_2d<rg32sint, read_write>",
                                 .wgsl_fn = R"(
    %4:void = textureStore %3, vec2<i32>(1i, 2i), vec4<i32>(1i, 2i, 3i, 4i))",
                             },
                             ImgData{
                                 .name = "Source 4 component, dest 4 component",
                                 .spirv_type = "%int 2D 0 0 0 2 Rgba32i",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vi1234",
                                 .wgsl_type = "texture_storage_2d<rgba32sint, read_write>",
                                 .wgsl_fn = R"(
    %4:void = textureStore %3, vec2<i32>(1i, 2i), vec4<i32>(1i, 2i, 3i, 4i))",
                             }));

// The texel operand signedness must match the channel type signedness.
// SPIR-V validation checks that.
// This suite is for the cases where they are integral and the same
// signedness.
INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageWrite_ConvertTexelOperand_SameSignedness,
                         ImageAccessTest,
                         ::testing::Values(
                             ImgData{
                                 .name = "Sampled type is unsigned int, texel is unsigned int",
                                 .spirv_type = "%uint 2D 0 0 0 2 Rgba32ui",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vu1234",
                                 .wgsl_type = "texture_storage_2d<rgba32uint, read_write>",
                                 .wgsl_fn = R"(
    %4:void = textureStore %3, vec2<i32>(1i, 2i), vec4<u32>(1u, 2u, 3u, 4u))",
                             },
                             ImgData{
                                 .name = "Sampled type is signed int, texel is signed int",
                                 .spirv_type = "%int 2D 0 0 0 2 Rgba32i",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vi1234",
                                 .wgsl_type = "texture_storage_2d<rgba32sint, read_write>",
                                 .wgsl_fn = R"(
    %4:void = textureStore %3, vec2<i32>(1i, 2i), vec4<i32>(1i, 2i, 3i, 4i))",
                             }));

// Show that zeros of the correct integer signedness are
// created when expanding an integer vector.
INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageWrite_ConvertTexelOperand_Signedness_AndWidening,
                         ImageAccessTest,
                         ::testing::Values(
                             ImgData{
                                 .name = "Source unsigned, dest unsigned",
                                 .spirv_type = "%uint 2D 0 0 0 2 R32ui",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vu12",
                                 .wgsl_type = "texture_storage_2d<r32uint, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<u32> = construct vec2<u32>(1u, 2u), vec2<u32>(0u)
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)",
                             },
                             ImgData{
                                 .name = "Source signed, dest signed",
                                 .spirv_type = "%int 2D 0 0 0 2 R32i",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vi12",
                                 .wgsl_type = "texture_storage_2d<r32sint, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<i32> = construct vec2<i32>(1i, 2i), vec2<i32>(0i)
    %5:void = textureStore %3, vec2<i32>(1i, 2i), %4)",
                             }));

INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_ImageFetch,
    ImageAccessTest,
    ::testing::Values(
        ImgData{
            .name = "OpImageFetch with no extra params, on sampled texture",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "%99 = OpImageFetch %v4float %im %vi12",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %4:vec4<f32> = textureLoad %3, vec2<i32>(1i, 2i), 0i)",
        },
        ImgData{
            .name = "OpImageFetch with explicit level, on sampled texture",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "%99 = OpImageFetch %v4float %im %vi12 Lod %int_3",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %4:vec4<f32> = textureLoad %3, vec2<i32>(1i, 2i), 3i)",
        },
        ImgData{
            .name = "OpImageFetch with no extra params, on depth texture",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn = "%99 = OpImageFetch %v4float %im %vi12",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %4:f32 = textureLoad %3, vec2<i32>(1i, 2i), 0i
    %5:vec4<f32> = construct %4, 0.0f, 0.0f, 0.0f)",
        },
        ImgData{
            .name = "OpImageFetch with extra params, on depth texture",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn = "%99 = OpImageFetch %v4float %im %vi12 Lod %int_3",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %4:f32 = textureLoad %3, vec2<i32>(1i, 2i), 3i
    %5:vec4<f32> = construct %4, 0.0f, 0.0f, 0.0f)",
        },
        // In SPIR-V OpImageFetch always yields a vector of 4 elements, even for depth images.  But
        // in WGSL, textureLoad on a depth image yields f32.
        // https://crbug.com/tint/439
        ImgData{
            .name = "ImageFetch on depth image.",
            .spirv_type = "%float 2D 1 0 0 1 Unknown",
            .spirv_fn = "%99 = OpImageFetch %v4float %im %vi12",
            .wgsl_type = "texture_depth_2d",
            .wgsl_fn = R"(
    %4:f32 = textureLoad %3, vec2<i32>(1i, 2i), 0i
    %5:vec4<f32> = construct %4, 0.0f, 0.0f, 0.0f)",
        },
        // In SPIR-V OpImageFetch always yields a vector of 4 elements, even for depth images.  But
        // in WGSL, textureLoad on a depth image yields f32.
        // https://crbug.com/tint/439
        ImgData{
            .name = "ImageFetch on multisampled depth image.",
            .spirv_type = "%float 2D 1 0 1 1 Unknown",
            .spirv_fn = "%99 = OpImageFetch %v4float %im %vi12 Sample %int_1",
            .wgsl_type = "texture_depth_multisampled_2d",
            .wgsl_fn = R"(
    %4:f32 = textureLoad %3, vec2<i32>(1i, 2i), 1i
    %5:vec4<f32> = construct %4, 0.0f, 0.0f, 0.0f)",
        },
        // SPIR-V requires a Sample image operand when operating on a multisampled image.
        ImgData{
            .name = "ImageFetch non-arrayed",
            .spirv_type = "%float 2D 0 0 1 1 Unknown",
            .spirv_fn = "%99 = OpImageFetch %v4float %im %vi12 Sample %int_1",
            .wgsl_type = "texture_multisampled_2d<f32>",
            .wgsl_fn = R"(
    %4:vec4<f32> = textureLoad %3, vec2<i32>(1i, 2i), 1i)",
        },
        ImgData{
            .name = "2d",
            .spirv_type = "%float 2D 0 0 1 1 Unknown",
            .spirv_fn = "%99 = OpImageFetch %v4float %im %vi12 Sample %uint_1",
            .wgsl_type = "texture_multisampled_2d<f32>",
            .wgsl_fn = R"(
    %4:i32 = convert 1u
    %5:vec4<f32> = textureLoad %3, vec2<i32>(1i, 2i), %4)",
        }));

using SampledImageAccessTest = SpirvReaderTestWithParam<ImgData>;
TEST_P(SampledImageAccessTest, Variable) {
    auto& params = GetParam();
    EXPECT_IR(R"(
            OpCapability Shader
            OpCapability ImageQuery
            OpCapability Sampled1D
            OpMemoryModel Logical Simple
            OpEntryPoint Fragment %main "main"
            OpExecutionMode %main OriginUpperLeft
            OpName %20 "wg"
            OpDecorate %20 DescriptorSet 2
            OpDecorate %20 Binding 1

   %float = OpTypeFloat 32
     %int = OpTypeInt 32  1
    %uint = OpTypeInt 32  0
   %v2int = OpTypeVector %int 2
   %v3int = OpTypeVector %int 3
   %v4int = OpTypeVector %int 4
  %v2uint = OpTypeVector %uint 2
  %v4uint = OpTypeVector %uint 4
 %v2float = OpTypeVector %float 2
 %v4float = OpTypeVector %float 4
    %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

   %im_ty = OpTypeImage )" +
                  params.spirv_type + R"(
%ptr_im_ty = OpTypePointer UniformConstant %im_ty

      %20 = OpVariable %ptr_im_ty UniformConstant

   %int_1 = OpConstant %int 1
   %int_2 = OpConstant %int 2
   %int_3 = OpConstant %int 3
   %int_4 = OpConstant %int 4
  %uint_1 = OpConstant %uint 1
  %uint_3 = OpConstant %uint 3
  %uint_4 = OpConstant %uint 4

    %vi12 = OpConstantComposite %v2int %int_1 %int_2

  %float_1 = OpConstant %float 1
  %float_2 = OpConstant %float 2
    %vf12 = OpConstantComposite %v2float %float_1 %float_2

    %main = OpFunction %void None %voidfn
   %entry = OpLabel

      %im = OpLoad %im_ty %20
)" + params.spirv_fn +
                  R"(
     OpReturn
     OpFunctionEnd
  )",
              R"(
$B1: {  # root
  %wg:ptr<handle, )" +
                  params.wgsl_type +
                  R"(, read> = var undef @binding_point(2, 1)
}

%main = @fragment func():void {
  $B2: {
    %3:)" + params.wgsl_type +
                  R"( = load %wg)" + params.wgsl_fn +
                  R"(
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ConvertResultSignedness,
                         SampledImageAccessTest,
                         ::testing::Values(
                             ImgData{
                                 .name = "2d no conversion float",
                                 .spirv_type = "%float 2D 0 0 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageFetch %v4float %im %vi12",
                                 .wgsl_type = "texture_2d<f32>",
                                 .wgsl_fn = R"(
    %4:vec4<f32> = textureLoad %3, vec2<i32>(1i, 2i), 0i)",
                             },
                             ImgData{
                                 .name = "2d no conversion uint",
                                 .spirv_type = "%uint 2D 0 0 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageFetch %v4uint %im %vi12",
                                 .wgsl_type = "texture_2d<u32>",
                                 .wgsl_fn = R"(
    %4:vec4<u32> = textureLoad %3, vec2<i32>(1i, 2i), 0i)",
                             },
                             ImgData{
                                 .name = "2d no conversion int",
                                 .spirv_type = "%int 2D 0 0 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageFetch %v4int %im %vi12",
                                 .wgsl_type = "texture_2d<i32>",
                                 .wgsl_fn = R"(
    %4:vec4<i32> = textureLoad %3, vec2<i32>(1i, 2i), 0i)",
                             }));

// From VUID-StandaloneSpirv-OpImageQuerySizeLod-04659:
//  ImageQuerySizeLod requires Sampled=1
INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageQuerySizeLod_NonArrayed_SignedResult_SignedLevel,
                         SampledImageAccessTest,
                         ::testing::Values(
                             ImgData{
                                 .name = "1D",
                                 .spirv_type = "%float 1D 0 0 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQuerySizeLod %int %im %int_1\n",
                                 .wgsl_type = "texture_1d<f32>",
                                 .wgsl_fn = R"(
    %4:u32 = textureDimensions %3, 1i
    %5:i32 = convert %4)",
                             },
                             ImgData{
                                 .name = "2D",
                                 .spirv_type = "%float 2D 0 0 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQuerySizeLod %v2int %im %int_1\n",
                                 .wgsl_type = "texture_2d<f32>",
                                 .wgsl_fn = R"(
    %4:vec2<u32> = textureDimensions %3, 1i
    %5:vec2<i32> = convert %4)",
                             },
                             ImgData{
                                 .name = "3D",
                                 .spirv_type = "%float 3D 0 0 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQuerySizeLod %v3int %im %int_1\n",
                                 .wgsl_type = "texture_3d<f32>",
                                 .wgsl_fn = R"(
    %4:vec3<u32> = textureDimensions %3, 1i
    %5:vec3<i32> = convert %4)",
                             },
                             ImgData{
                                 .name = "Cube",
                                 .spirv_type = "%float Cube 0 0 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQuerySizeLod %v2int %im %int_1\n",
                                 .wgsl_type = "texture_cube<f32>",
                                 .wgsl_fn = R"(
    %4:vec2<u32> = textureDimensions %3, 1i
    %5:vec2<i32> = convert %4)",
                             },
                             ImgData{
                                 .name = "Depth 2D",
                                 .spirv_type = "%float 2D 1 0 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQuerySizeLod %v2int %im %int_1\n",
                                 .wgsl_type = "texture_depth_2d",
                                 .wgsl_fn = R"(
    %4:vec2<u32> = textureDimensions %3, 1i
    %5:vec2<i32> = convert %4)",
                             },
                             ImgData{
                                 .name = "Depth Cube",
                                 .spirv_type = "%float Cube 1 0 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQuerySizeLod %v2int %im %int_1\n",
                                 .wgsl_type = "texture_depth_cube",
                                 .wgsl_fn = R"(
    %4:vec2<u32> = textureDimensions %3, 1i
    %5:vec2<i32> = convert %4)",
                             }));

// ImageQuerySize requires storage image or multisampled
// For storage image, use another instruction to indicate whether it
// is readonly or writeonly.
INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageQuerySizeLod_Arrayed_SignedResult_SignedLevel,
                         SampledImageAccessTest,
                         ::testing::Values(
                             ImgData{
                                 .name = "2D array",
                                 .spirv_type = "%float 2D 0 1 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQuerySizeLod %v3int %im %int_1\n",
                                 .wgsl_type = "texture_2d_array<f32>",
                                 .wgsl_fn =
                                     R"(
    %4:vec2<u32> = textureDimensions %3, 1i
    %5:u32 = textureNumLayers %3
    %6:vec3<u32> = construct %4, %5
    %7:vec3<i32> = convert %6)",
                             },
                             ImgData{
                                 .name = "Cube array",
                                 .spirv_type = "%float Cube 0 1 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQuerySizeLod %v3int %im %int_1\n",
                                 .wgsl_type = "texture_cube_array<f32>",
                                 .wgsl_fn =
                                     R"(
    %4:vec2<u32> = textureDimensions %3, 1i
    %5:u32 = textureNumLayers %3
    %6:vec3<u32> = construct %4, %5
    %7:vec3<i32> = convert %6)",
                             },
                             ImgData{
                                 .name = "Depth 2D array",
                                 .spirv_type = "%float 2D 1 1 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQuerySizeLod %v3int %im %int_1\n",
                                 .wgsl_type = "texture_depth_2d_array",
                                 .wgsl_fn =
                                     R"(
    %4:vec2<u32> = textureDimensions %3, 1i
    %5:u32 = textureNumLayers %3
    %6:vec3<u32> = construct %4, %5
    %7:vec3<i32> = convert %6)",
                             },
                             ImgData{
                                 .name = "Depth Cube Array",
                                 .spirv_type = "%float Cube 1 1 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQuerySizeLod %v3int %im %int_1\n",
                                 .wgsl_type = "texture_depth_cube_array",
                                 .wgsl_fn =
                                     R"(
    %4:vec2<u32> = textureDimensions %3, 1i
    %5:u32 = textureNumLayers %3
    %6:vec3<u32> = construct %4, %5
    %7:vec3<i32> = convert %6)",
                             }));

// textureDimensions accepts both signed and unsigned the level-of-detail values.
INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageQuerySizeLod_NonArrayed_SignedResult_UnsignedLevel,
                         SampledImageAccessTest,
                         ::testing::Values(ImgData{
                             .name = "1D",
                             .spirv_type = "%float 1D 0 0 0 1 Unknown",
                             .spirv_fn = "%99 = OpImageQuerySizeLod %int %im %uint_1\n",
                             .wgsl_type = "texture_1d<f32>",
                             .wgsl_fn = R"(
    %4:u32 = textureDimensions %3, 1u
    %5:i32 = convert %4)",
                         }));

// When SPIR-V wants the result type to be unsigned, we have to insert a value constructor
// for WGSL to do the type coercion.
INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageQuerySizeLod_NonArrayed_UnsignedResult_SignedLevel,
                         SampledImageAccessTest,
                         ::testing::Values(ImgData{
                             .name = "1D",
                             .spirv_type = "%float 1D 0 0 0 1 Unknown",
                             .spirv_fn = "%99 = OpImageQuerySizeLod %uint %im %int_1",
                             .wgsl_type = "texture_1d<f32>",
                             .wgsl_fn = R"(
    %4:u32 = textureDimensions %3, 1i)",
                         }));

INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageQueryLevels_SignedResult,
                         SampledImageAccessTest,
                         ::testing::Values(
                             ImgData{
                                 .name = "2D",
                                 .spirv_type = "%float 2D 0 0 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQueryLevels %int %im",
                                 .wgsl_type = "texture_2d<f32>",
                                 .wgsl_fn = R"(
    %4:u32 = textureNumLevels %3
    %5:i32 = convert %4)",
                             },
                             ImgData{
                                 .name = "2D array",
                                 .spirv_type = "%float 2D 0 1 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQueryLevels %int %im",
                                 .wgsl_type = "texture_2d_array<f32>",
                                 .wgsl_fn = R"(
    %4:u32 = textureNumLevels %3
    %5:i32 = convert %4)",
                             },
                             ImgData{
                                 .name = "3D",
                                 .spirv_type = "%float 3D 0 0 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQueryLevels %int %im",
                                 .wgsl_type = "texture_3d<f32>",
                                 .wgsl_fn = R"(
    %4:u32 = textureNumLevels %3
    %5:i32 = convert %4)",
                             },
                             ImgData{
                                 .name = "Cube",
                                 .spirv_type = "%float Cube 0 0 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQueryLevels %int %im",
                                 .wgsl_type = "texture_cube<f32>",
                                 .wgsl_fn = R"(
    %4:u32 = textureNumLevels %3
    %5:i32 = convert %4)",
                             },
                             ImgData{
                                 .name = "Cube array",
                                 .spirv_type = "%float Cube 0 1 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQueryLevels %int %im",
                                 .wgsl_type = "texture_cube_array<f32>",
                                 .wgsl_fn = R"(
    %4:u32 = textureNumLevels %3
    %5:i32 = convert %4)",
                             },
                             ImgData{
                                 .name = "depth 2d",
                                 .spirv_type = "%float 2D 1 0 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQueryLevels %int %im",
                                 .wgsl_type = "texture_depth_2d",
                                 .wgsl_fn = R"(
    %4:u32 = textureNumLevels %3
    %5:i32 = convert %4)",
                             },
                             ImgData{
                                 .name = "depth 2d array",
                                 .spirv_type = "%float 2D 1 1 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQueryLevels %int %im",
                                 .wgsl_type = "texture_depth_2d_array",
                                 .wgsl_fn = R"(
    %4:u32 = textureNumLevels %3
    %5:i32 = convert %4)",
                             },
                             ImgData{
                                 .name = "depth cube",
                                 .spirv_type = "%float Cube 1 0 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQueryLevels %int %im",
                                 .wgsl_type = "texture_depth_cube",
                                 .wgsl_fn = R"(
    %4:u32 = textureNumLevels %3
    %5:i32 = convert %4)",
                             },
                             ImgData{
                                 .name = "depth cube array",
                                 .spirv_type = "%float Cube 1 1 0 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQueryLevels %int %im",
                                 .wgsl_type = "texture_depth_cube_array",
                                 .wgsl_fn = R"(
    %4:u32 = textureNumLevels %3
    %5:i32 = convert %4)",
                             }));

// Spot check that a value conversion is inserted when SPIR-V asks for an unsigned int result.
INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageQueryLevels_UnsignedResult,
                         SampledImageAccessTest,
                         ::testing::Values(ImgData{
                             .name = "2D",
                             .spirv_type = "%float 2D 0 0 0 1 Unknown",
                             .spirv_fn = "%99 = OpImageQueryLevels %uint %im\n",
                             .wgsl_type = "texture_2d<f32>",
                             .wgsl_fn = R"(
    %4:u32 = textureNumLevels %3)",
                         }));

using NoSamplerImageAccessTest = SpirvReaderTestWithParam<ImgData>;
TEST_P(NoSamplerImageAccessTest, Variable) {
    auto& params = GetParam();
    EXPECT_IR(R"(
            OpCapability Shader
            OpCapability ImageQuery
            OpCapability Image1D
            OpCapability Sampled1D
            OpMemoryModel Logical Simple
            OpEntryPoint Fragment %main "main"
            OpExecutionMode %main OriginUpperLeft
            OpName %wg "wg"
            OpDecorate %wg DescriptorSet 2
            OpDecorate %wg Binding 1

   %float = OpTypeFloat 32
     %int = OpTypeInt 32  1
    %uint = OpTypeInt 32  0
   %v2int = OpTypeVector %int 2
   %v3int = OpTypeVector %int 3
   %v4int = OpTypeVector %int 4
  %v2uint = OpTypeVector %uint 2
  %v3uint = OpTypeVector %uint 3
  %v4uint = OpTypeVector %uint 4
 %v4float = OpTypeVector %float 4
    %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

   %im_ty = OpTypeImage )" +
                  params.spirv_type + R"(
%ptr_im_ty = OpTypePointer UniformConstant %im_ty

   %int_1 = OpConstant %int 1
   %int_2 = OpConstant %int 2
   %int_3 = OpConstant %int 3
    %vi12 = OpConstantComposite %v2int %int_1 %int_2
   %vi123 = OpConstantComposite %v3int %int_1 %int_2 %int_3

     %wg = OpVariable %ptr_im_ty UniformConstant

   %main = OpFunction %void None %voidfn
  %entry = OpLabel

     %im = OpLoad %im_ty %wg
)" + params.spirv_fn +
                  R"(
     OpReturn
     OpFunctionEnd
  )",
              R"(
$B1: {  # root
  %wg:ptr<handle, )" +
                  params.wgsl_type +
                  R"(, read> = var undef @binding_point(2, 1)
}

%main = @fragment func():void {
  $B2: {
    %3:)" + params.wgsl_type +
                  R"( = load %wg)" + params.wgsl_fn +
                  R"(
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_CheckResultSignedness,
                         NoSamplerImageAccessTest,
                         ::testing::Values(
                             ImgData{
                                 .name = "no conversion, float -> v4float",
                                 .spirv_type = "%float 2D 0 0 0 2 Rgba32f",
                                 .spirv_fn = "%99 = OpImageRead %v4float %im %vi12",
                                 .wgsl_type = "texture_storage_2d<rgba32float, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<f32> = textureLoad %3, vec2<i32>(1i, 2i))",
                             },
                             ImgData{
                                 .name = "no conversion, uint -> v4uint",
                                 .spirv_type = "%uint 2D 0 0 0 2 Rgba32ui",
                                 .spirv_fn = "%99 = OpImageRead %v4uint %im %vi12",
                                 .wgsl_type = "texture_storage_2d<rgba32uint, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<u32> = textureLoad %3, vec2<i32>(1i, 2i))",
                             },
                             ImgData{
                                 .name = "no conversion, int -> v4int",
                                 .spirv_type = "%int 2D 0 0 0 2 Rgba32i",
                                 .spirv_fn = "%99 = OpImageRead %v4int %im %vi12",
                                 .wgsl_type = "texture_storage_2d<rgba32sint, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<i32> = textureLoad %3, vec2<i32>(1i, 2i))",
                             }));

// ImageQuerySize requires storage image or multisampled
// For storage image, use another instruction to indicate whether it is readonly or writeonly.
INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageQuerySize_NonArrayed_SignedResult,
                         NoSamplerImageAccessTest,
                         ::testing::Values(
                             ImgData{
                                 .name = "1D storage image",
                                 .spirv_type = "%float 1D 0 0 0 2 Rgba32f",
                                 .spirv_fn = "%99 = OpImageQuerySize %int %im",
                                 .wgsl_type = "texture_storage_1d<rgba32float, read_write>",
                                 .wgsl_fn = R"(
    %4:u32 = textureDimensions %3
    %5:i32 = convert %4)",
                             },
                             ImgData{
                                 .name = "2D storage image",
                                 .spirv_type = "%float 2D 0 0 0 2 Rgba32f",
                                 .spirv_fn = "%99 = OpImageQuerySize %v2int %im",
                                 .wgsl_type = "texture_storage_2d<rgba32float, read_write>",
                                 .wgsl_fn = R"(
    %4:vec2<u32> = textureDimensions %3
    %5:vec2<i32> = convert %4)",
                             },
                             ImgData{
                                 .name = "3D storage image",
                                 .spirv_type = "%float 3D 0 0 0 2 Rgba32f",
                                 .spirv_fn = "%99 = OpImageQuerySize %v3int %im",
                                 .wgsl_type = "texture_storage_3d<rgba32float, read_write>",
                                 .wgsl_fn = R"(
    %4:vec3<u32> = textureDimensions %3
    %5:vec3<i32> = convert %4)",
                             },
                             ImgData{
                                 .name = "Multisampled",
                                 .spirv_type = "%float 2D 0 0 1 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQuerySize %v2int %im",
                                 .wgsl_type = "texture_multisampled_2d<f32>",
                                 .wgsl_fn = R"(
    %4:vec2<u32> = textureDimensions %3
    %5:vec2<i32> = convert %4)",
                             }));

// ImageQuerySize requires storage image or multisampled
// For storage image, use another instruction to indicate whether it is readonly or writeonly.
INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageQuerySize_Arrayed_SignedResult,
                         NoSamplerImageAccessTest,
                         ::testing::Values(ImgData{
                             .name = "2D array storage image",
                             .spirv_type = "%float 2D 0 1 0 2 Rgba32f",
                             .spirv_fn = "%99 = OpImageQuerySize %v3int %im",
                             .wgsl_type = "texture_storage_2d_array<rgba32float, read_write>",
                             .wgsl_fn = R"(
    %4:vec2<u32> = textureDimensions %3
    %5:u32 = textureNumLayers %3
    %6:vec3<u32> = construct %4, %5
    %7:vec3<i32> = convert %6)",
                         }));

// ImageQuerySize requires storage image or multisampled
// For storage image, use another instruction to indicate whether it is readonly or writeonly.
INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageQuerySize_NonArrayed_UnsignedResult,
                         NoSamplerImageAccessTest,
                         ::testing::Values(
                             ImgData{
                                 .name = "1D storage image",
                                 .spirv_type = "%float 1D 0 0 0 2 Rgba32f",
                                 .spirv_fn = "%99 = OpImageQuerySize %uint %im",
                                 .wgsl_type = "texture_storage_1d<rgba32float, read_write>",
                                 .wgsl_fn = R"(
    %4:u32 = textureDimensions %3)",
                             },
                             ImgData{
                                 .name = "2D storage image",
                                 .spirv_type = "%float 2D 0 0 0 2 Rgba32f",
                                 .spirv_fn = "%99 = OpImageQuerySize %v2uint %im",
                                 .wgsl_type = "texture_storage_2d<rgba32float, read_write>",
                                 .wgsl_fn = R"(
    %4:vec2<u32> = textureDimensions %3)",
                             },
                             ImgData{
                                 .name = "3D storage image",
                                 .spirv_type = "%float 3D 0 0 0 2 Rgba32f",
                                 .spirv_fn = "%99 = OpImageQuerySize %v3uint %im",
                                 .wgsl_type = "texture_storage_3d<rgba32float, read_write>",
                                 .wgsl_fn = R"(
    %4:vec3<u32> = textureDimensions %3)",
                             },
                             ImgData{
                                 .name = "Multisampled",
                                 .spirv_type = "%float 2D 0 0 1 1 Unknown",
                                 .spirv_fn = "%99 = OpImageQuerySize %v2uint %im",
                                 .wgsl_type = "texture_multisampled_2d<f32>",
                                 .wgsl_fn = R"(
    %4:vec2<u32> = textureDimensions %3)",
                             }));

INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageQuerySize_Arrayed_UnsignedResult,
                         NoSamplerImageAccessTest,
                         ::testing::Values(ImgData{
                             .name = "2D array storage image",
                             .spirv_type = "%float 2D 0 1 0 2 Rgba32f",
                             .spirv_fn = "%99 = OpImageQuerySize %v3uint %im",
                             .wgsl_type = "texture_storage_2d_array<rgba32float, read_write>",
                             .wgsl_fn = R"(
    %4:vec2<u32> = textureDimensions %3
    %5:u32 = textureNumLayers %3
    %6:vec3<u32> = construct %4, %5)",
                         }));

INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageQuerySamples_SignedResult,
                         NoSamplerImageAccessTest,
                         ::testing::Values(ImgData{
                             .name = "Multsample 2D",
                             .spirv_type = "%float 2D 0 0 1 1 Unknown",
                             .spirv_fn = "%99 = OpImageQuerySamples %int %im",
                             .wgsl_type = "texture_multisampled_2d<f32>",
                             .wgsl_fn = R"(
    %4:u32 = textureNumSamples %3
    %5:i32 = convert %4)",
                         }));

// Translation must inject a type coercion from unsigned to signed.
INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageQuerySamples_UnsignedResult,
                         NoSamplerImageAccessTest,
                         ::testing::Values(ImgData{
                             .name = "Multisample 2D",
                             .spirv_type = "%float 2D 0 0 1 1 Unknown",
                             .spirv_fn = "%99 = OpImageQuerySamples %uint %im",
                             .wgsl_type = "texture_multisampled_2d<f32>",
                             .wgsl_fn = R"(
    %4:u32 = textureNumSamples %3)",
                         }));

using SampledImageCoordsTest = SpirvReaderTestWithParam<ImgData>;
TEST_P(SampledImageCoordsTest, MakeCoordinateOperandsForImageAccess) {
    auto& params = GetParam();

    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability Sampled1D
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %100 "main"
           OpExecutionMode %100 OriginUpperLeft
           OpName %20 "wg"
           OpDecorate %10 DescriptorSet 0
           OpDecorate %10 Binding 0
           OpDecorate %20 DescriptorSet 2
           OpDecorate %20 Binding 0

   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void
  %float = OpTypeFloat 32
    %int = OpTypeInt 32 1
   %uint = OpTypeInt 32 0

  %v2int = OpTypeVector %int 2
  %v3int = OpTypeVector %int 3
 %v2uint = OpTypeVector %uint 2
 %v3uint = OpTypeVector %uint 3
%v2float = OpTypeVector %float 2
%v3float = OpTypeVector %float 3
%v4float = OpTypeVector %float 4

%float_null = OpConstantNull %float

%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
  %vi12 = OpConstantComposite %v2int %int_1 %int_2
 %vi123 = OpConstantComposite %v3int %int_1 %int_2 %int_3
  %vu12 = OpConstantComposite %v2uint %uint_1 %uint_2
 %vu123 = OpConstantComposite %v3uint %uint_1 %uint_2 %uint_3
  %vf12 = OpConstantComposite %v2float %float_1 %float_2
 %vf123 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%vf1234 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4

%sampler = OpTypeSampler
%ptr_sampler = OpTypePointer UniformConstant %sampler
  %im_ty = OpTypeImage )" +
                  params.spirv_type + R"(
%ptr_im_ty = OpTypePointer UniformConstant %im_ty
  %si_ty = OpTypeSampledImage %im_ty

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_im_ty UniformConstant

     %100 = OpFunction %void None %voidfn
   %entry = OpLabel

     %sam = OpLoad %sampler %10
      %im = OpLoad %im_ty %20

%sampled_image = OpSampledImage %si_ty %im %sam
)" + params.spirv_fn +
                  R"(
           OpReturn
           OpFunctionEnd
  )",
              R"(
$B1: {  # root
  %1:ptr<handle, sampler, read> = var undef @binding_point(0, 0)
  %wg:ptr<handle, )" +
                  params.wgsl_type +
                  R"(, read> = var undef @binding_point(2, 0)
}

%main = @fragment func():void {
  $B2: {
    %4:sampler = load %1
    %5:)" + params.wgsl_type +
                  R"( = load %wg)" + params.wgsl_fn +
                  R"(
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_ImageSampleImplicitLod,
    SampledImageCoordsTest,
    ::testing::Values(
        ImgData{
            .name = "1D",
            .spirv_type = "%float 1D 0 0 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %float_1",
            .wgsl_type = "texture_1d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSample %5, %4, 1.0f)",
        },
        ImgData{
            .name = "1D one extra arg",
            .spirv_type = "%float 1D 0 0 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf12",
            .wgsl_type = "texture_1d<f32>",
            .wgsl_fn = R"(
    %6:f32 = swizzle vec2<f32>(1.0f, 2.0f), x
    %7:vec4<f32> = textureSample %5, %4, %6)",
        },
        ImgData{
            .name = "1d two extra args",
            .spirv_type = "%float 1D 0 0 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf123",
            .wgsl_type = "texture_1d<f32>",
            .wgsl_fn = R"(
    %6:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), x
    %7:vec4<f32> = textureSample %5, %4, %6)",
        },
        ImgData{
            .name = "1D three extra args",
            .spirv_type = "%float 1D 0 0 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf1234",
            .wgsl_type = "texture_1d<f32>",
            .wgsl_fn = R"(
    %6:f32 = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), x
    %7:vec4<f32> = textureSample %5, %4, %6)",
        },
        ImgData{
            .name = "2D",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf12",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSample %5, %4, vec2<f32>(1.0f, 2.0f))",
        },
        ImgData{
            .name = "2D one excess arg",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf123",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:vec4<f32> = textureSample %5, %4, %6)",
        },
        ImgData{
            .name = "2D two excess args",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf1234",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), xy
    %7:vec4<f32> = textureSample %5, %4, %6)",
        },
        ImgData{
            .name = "2D array",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf123",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureSample %5, %4, %6, %8)",
        },
        ImgData{
            .name = "2D array one excess arg",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf1234",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), xy
    %7:f32 = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureSample %5, %4, %6, %8)",
        },
        ImgData{
            .name = "3D",
            .spirv_type = "%float 3D 0 0 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf123",
            .wgsl_type = "texture_3d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSample %5, %4, vec3<f32>(1.0f, 2.0f, 3.0f))",
        },
        ImgData{
            .name = "3D one excess arg",
            .spirv_type = "%float 3D 0 0 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf1234",
            .wgsl_type = "texture_3d<f32>",
            .wgsl_fn = R"(
    %6:vec3<f32> = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), xyz
    %7:vec4<f32> = textureSample %5, %4, %6)",
        },
        ImgData{
            .name = "Cube",
            .spirv_type = "%float Cube 0 0 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf123",
            .wgsl_type = "texture_cube<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSample %5, %4, vec3<f32>(1.0f, 2.0f, 3.0f))",
        },
        ImgData{
            .name = "Cube one excess arg",
            .spirv_type = "%float Cube 0 0 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf1234",
            .wgsl_type = "texture_cube<f32>",
            .wgsl_fn = R"(
    %6:vec3<f32> = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), xyz
    %7:vec4<f32> = textureSample %5, %4, %6)",
        },
        ImgData{
            .name = "Cube array",
            .spirv_type = "%float Cube 0 1 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf1234",
            .wgsl_type = "texture_cube_array<f32>",
            .wgsl_fn = R"(
    %6:vec3<f32> = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), xyz
    %7:f32 = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), w
    %8:i32 = convert %7
    %9:vec4<f32> = textureSample %5, %4, %6, %8)",
        },
        ImgData{
            .name = "2d no conversion float sampled lod",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "%99 = OpImageSampleImplicitLod %v4float %sampled_image %vf12",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSample %5, %4, vec2<f32>(1.0f, 2.0f))",
        }));

// In SPIR-V, sampling and dref sampling operations use floating point coordinates.  Prove that we
// preserve floating point-ness. Test across all such instructions.
INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_PreserveFloatCoords_NonArrayed,
    SampledImageCoordsTest,
    ::testing::Values(
        // Scalar cases
        ImgData{
            .name = "1D",
            .spirv_type = "%float 1D 0 0 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %float_1",
            .wgsl_type = "texture_1d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSample %5, %4, 1.0f)",
        },
        ImgData{
            .name = "1D lod",
            .spirv_type = "%float 1D 0 0 0 1 Unknown",
            .spirv_fn =
                "%result = OpImageSampleExplicitLod %v4float %sampled_image %float_1 Lod %float_1",
            .wgsl_type = "texture_1d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSampleLevel %5, %4, 1.0f, 1.0f)",
        },
        ImgData{
            .name = "2D vec2",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf12",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSample %5, %4, vec2<f32>(1.0f, 2.0f))",
        },
        ImgData{
            .name = "2D vec2 lod",
            .spirv_type = "%float 2D 0 0 0 1 Unknown",
            .spirv_fn =
                "%result = OpImageSampleExplicitLod %v4float %sampled_image %vf12 Lod %float_1",
            .wgsl_type = "texture_2d<f32>",
            .wgsl_fn = R"(
    %6:vec4<f32> = textureSampleLevel %5, %4, vec2<f32>(1.0f, 2.0f), 1.0f)",
        }));

// In SPIR-V, sampling and dref sampling operations use floating point coordinates.  Prove that we
// preserve floating point-ness of the coordinate part, but convert the array index to signed
// integer.Test across all such instructions.
INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_PreserveFloatCoords_Arrayed,
    SampledImageCoordsTest,
    ::testing::Values(
        ImgData{
            .name = "2D vec3",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn = "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf1234",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), xy
    %7:f32 = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureSample %5, %4, %6, %8)",
        },
        ImgData{
            .name = "2D lod",
            .spirv_type = "%float 2D 0 1 0 1 Unknown",
            .spirv_fn =
                "%result = OpImageSampleExplicitLod %v4float %sampled_image %vf1234 Lod %float_1",
            .wgsl_type = "texture_2d_array<f32>",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), xy
    %7:f32 = swizzle vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), z
    %8:i32 = convert %7
    %9:vec4<f32> = textureSampleLevel %5, %4, %6, %8, 1.0f)",
        }));

// In SPIR-V, sampling and dref sampling operations use floating point coordinates.  Prove that we
// preserve floating point-ness of the coordinate part, but convert the array index to signed
// integer.Test across all such instructions.
INSTANTIATE_TEST_SUITE_P(
    SpirvReaderTest_PreserveFloatCoords_Arrayed,
    SamplerComparisonTest,
    ::testing::Values(
        ImgData{
            .name = "2D Depth",
            .spirv_type = "%float 2D 1 1 0 1 Unknown",
            .spirv_fn = "OpImageSampleDrefImplicitLod %float %sampled_image %coords3 %float_1",
            .wgsl_type = "texture_depth_2d_array",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:f32 = textureSampleCompare %5, %4, %6, %8, 1.0f)",
        },
        ImgData{
            .name = "2D depth lod",
            .spirv_type = "%float 2D 1 1 0 1 Unknown",
            .spirv_fn = "OpImageSampleDrefExplicitLod %float %sampled_image %coords3 %float_1 Lod "
                        "%float_0",
            .wgsl_type = "texture_depth_2d_array",
            .wgsl_fn = R"(
    %6:vec2<f32> = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), xy
    %7:f32 = swizzle vec3<f32>(1.0f, 2.0f, 3.0f), z
    %8:i32 = convert %7
    %9:f32 = textureSampleCompareLevel %5, %4, %6, %8, 1.0f)",
        }));

using NonSampledImageCoordsTest = SpirvReaderTestWithParam<ImgData>;
TEST_P(NonSampledImageCoordsTest, MakeCoordinateOperandsForImageAccess) {
    auto& params = GetParam();
    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability Image1D
           OpCapability Sampled1D
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %100 "main"
           OpExecutionMode %100 OriginUpperLeft
           OpName %20 "wg"
           OpDecorate %20 DescriptorSet 2
           OpDecorate %20 Binding 0

   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void
  %float = OpTypeFloat 32
    %int = OpTypeInt 32 1
   %uint = OpTypeInt 32 0

  %v2int = OpTypeVector %int 2
  %v3int = OpTypeVector %int 3
 %v2uint = OpTypeVector %uint 2
 %v3uint = OpTypeVector %uint 3
%v4float = OpTypeVector %float 4

  %int_1 = OpConstant %int 1
  %int_2 = OpConstant %int 2
  %int_3 = OpConstant %int 3
 %uint_1 = OpConstant %uint 1
 %uint_2 = OpConstant %uint 2
 %uint_3 = OpConstant %uint 3
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4

   %vi12 = OpConstantComposite %v2int %int_1 %int_2
  %vi123 = OpConstantComposite %v3int %int_1 %int_2 %int_3
   %vu12 = OpConstantComposite %v2uint %uint_1 %uint_2
  %vu123 = OpConstantComposite %v3uint %uint_1 %uint_2 %uint_3
 %vf1234 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4

  %im_ty = OpTypeImage )" +
                  params.spirv_type + R"(
%ptr_im_ty = OpTypePointer UniformConstant %im_ty

     %20 = OpVariable %ptr_im_ty UniformConstant

     %100 = OpFunction %void None %voidfn
   %entry = OpLabel

     %im = OpLoad %im_ty %20
)" + params.spirv_fn +
                  R"(
           OpReturn
           OpFunctionEnd
)",
              R"(
$B1: {  # root
  %wg:ptr<handle, )" +
                  params.wgsl_type +
                  R"(, read> = var undef @binding_point(2, 0)
}

%main = @fragment func():void {
  $B2: {
    %3:)" + params.wgsl_type +
                  R"( = load %wg)" + params.wgsl_fn + R"(
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvReaderTest_ImageFetch,
                         NonSampledImageCoordsTest,
                         ::testing::Values(
                             ImgData{
                                 .name = "1d signed",
                                 .spirv_type = "%float 1D 0 0 0 1 Unknown",
                                 .spirv_fn = "%result = OpImageFetch %v4float %im %int_1",
                                 .wgsl_type = "texture_1d<f32>",
                                 .wgsl_fn = R"(
    %4:vec4<f32> = textureLoad %3, 1i, 0i)",
                             },
                             ImgData{
                                 .name = "2d signed",
                                 .spirv_type = "%float 2D 0 0 0 1 Unknown",
                                 .spirv_fn = "%result = OpImageFetch %v4float %im %vi12",
                                 .wgsl_type = "texture_2d<f32>",
                                 .wgsl_fn = R"(
    %4:vec4<f32> = textureLoad %3, vec2<i32>(1i, 2i), 0i)",
                             },
                             ImgData{
                                 .name = "2D array signed",
                                 .spirv_type = "%float 2D 0 1 0 1 Unknown",
                                 .spirv_fn = "%result = OpImageFetch %v4float %im %vi123",
                                 .wgsl_type = "texture_2d_array<f32>",
                                 .wgsl_fn = R"(
    %4:vec2<i32> = swizzle vec3<i32>(1i, 2i, 3i), xy
    %5:i32 = swizzle vec3<i32>(1i, 2i, 3i), z
    %6:vec4<f32> = textureLoad %3, %4, %5, 0i)",
                             },
                             ImgData{
                                 .name = "1D unsigned",
                                 .spirv_type = "%float 1D 0 0 0 1 Unknown",
                                 .spirv_fn = "%result = OpImageFetch %v4float %im %uint_1",
                                 .wgsl_type = "texture_1d<f32>",
                                 .wgsl_fn = R"(
    %4:vec4<f32> = textureLoad %3, 1u, 0i)",
                             },
                             ImgData{
                                 .name = "2D unsigned",
                                 .spirv_type = "%float 2D 0 0 0 1 Unknown",
                                 .spirv_fn = "%result = OpImageFetch %v4float %im %vu12",
                                 .wgsl_type = "texture_2d<f32>",
                                 .wgsl_fn = R"(
    %4:vec4<f32> = textureLoad %3, vec2<u32>(1u, 2u), 0i)",
                             },
                             ImgData{
                                 .name = "2D array unsigned",
                                 .spirv_type = "%float 2D 0 1 0 1 Unknown",
                                 .spirv_fn = "%result = OpImageFetch %v4float %im %vu123",
                                 .wgsl_type = "texture_2d_array<f32>",
                                 .wgsl_fn = R"(
    %4:vec2<u32> = swizzle vec3<u32>(1u, 2u, 3u), xy
    %5:u32 = swizzle vec3<u32>(1u, 2u, 3u), z
    %6:i32 = convert %5
    %7:vec4<f32> = textureLoad %3, %4, %6, 0i)",
                             }));

INSTANTIATE_TEST_SUITE_P(SpirvReaderTest,
                         NonSampledImageCoordsTest,
                         ::testing::Values(
                             ImgData{
                                 .name = "1D Read signed",
                                 .spirv_type = "%float 1D 0 0 0 2 R32f",
                                 .spirv_fn = "%result = OpImageRead %v4float %im %int_1",
                                 .wgsl_type = "texture_storage_1d<r32float, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<f32> = textureLoad %3, 1i)",
                             },
                             ImgData{
                                 .name = "1D Write signed",
                                 .spirv_type = "%float 1D 0 0 0 2 R32f",
                                 .spirv_fn = "OpImageWrite %im %int_1 %vf1234",
                                 .wgsl_type = "texture_storage_1d<r32float, read_write>",
                                 .wgsl_fn = R"(
    %4:void = textureStore %3, 1i, vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f))",
                             },
                             ImgData{
                                 .name = "2D read signed",
                                 .spirv_type = "%float 2D 0 0 0 2 R32f",
                                 .spirv_fn = "%result = OpImageRead %v4float %im %vi12",
                                 .wgsl_type = "texture_storage_2d<r32float, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<f32> = textureLoad %3, vec2<i32>(1i, 2i))",
                             },
                             ImgData{
                                 .name = "2D write signed",
                                 .spirv_type = "%float 2D 0 0 0 2 R32f",
                                 .spirv_fn = "OpImageWrite %im %vi12 %vf1234",
                                 .wgsl_type = "texture_storage_2d<r32float, read_write>",
                                 .wgsl_fn = R"(
    %4:void = textureStore %3, vec2<i32>(1i, 2i), vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f))",
                             },
                             ImgData{
                                 .name = "2D array read signed",
                                 .spirv_type = "%float 2D 0 1 0 2 R32f",
                                 .spirv_fn = "%result = OpImageRead %v4float %im %vi123",
                                 .wgsl_type = "texture_storage_2d_array<r32float, read_write>",
                                 .wgsl_fn = R"(
    %4:vec2<i32> = swizzle vec3<i32>(1i, 2i, 3i), xy
    %5:i32 = swizzle vec3<i32>(1i, 2i, 3i), z
    %6:vec4<f32> = textureLoad %3, %4, %5)",
                             },
                             ImgData{
                                 .name = "2D array write signed",
                                 .spirv_type = "%float 2D 0 1 0 2 R32f",
                                 .spirv_fn = "OpImageWrite %im %vi123 %vf1234",
                                 .wgsl_type = "texture_storage_2d_array<r32float, read_write>",
                                 .wgsl_fn = R"(
    %4:vec2<i32> = swizzle vec3<i32>(1i, 2i, 3i), xy
    %5:i32 = swizzle vec3<i32>(1i, 2i, 3i), z
    %6:void = textureStore %3, %4, %5, vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f))",
                             },
                             ImgData{
                                 .name = "1D read unsigned",
                                 .spirv_type = "%float 1D 0 0 0 2 R32f",
                                 .spirv_fn = "%result = OpImageRead %v4float %im %uint_1",
                                 .wgsl_type = "texture_storage_1d<r32float, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<f32> = textureLoad %3, 1u)",
                             },
                             ImgData{
                                 .name = "1D write unsigned",
                                 .spirv_type = "%float 1D 0 0 0 2 R32f",
                                 .spirv_fn = "OpImageWrite %im %uint_1 %vf1234",
                                 .wgsl_type = "texture_storage_1d<r32float, read_write>",
                                 .wgsl_fn = R"(
    %4:void = textureStore %3, 1u, vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f))",
                             },
                             ImgData{
                                 .name = "2D read unsigned",
                                 .spirv_type = "%float 2D 0 0 0 2 R32f",
                                 .spirv_fn = "%result = OpImageRead %v4float %im %vu12",
                                 .wgsl_type = "texture_storage_2d<r32float, read_write>",
                                 .wgsl_fn = R"(
    %4:vec4<f32> = textureLoad %3, vec2<u32>(1u, 2u))",
                             },
                             ImgData{
                                 .name = "2D write unsigned",
                                 .spirv_type = "%float 2D 0 0 0 2 R32f",
                                 .spirv_fn = "OpImageWrite %im %vu12 %vf1234",
                                 .wgsl_type = "texture_storage_2d<r32float, read_write>",
                                 .wgsl_fn = R"(
    %4:void = textureStore %3, vec2<u32>(1u, 2u), vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f))",
                             },
                             ImgData{
                                 .name = "2D array read unsigned",
                                 .spirv_type = "%float 2D 0 1 0 2 R32f",
                                 .spirv_fn = "%result = OpImageRead %v4float %im %vu123",
                                 .wgsl_type = "texture_storage_2d_array<r32float, read_write>",
                                 .wgsl_fn = R"(
    %4:vec2<u32> = swizzle vec3<u32>(1u, 2u, 3u), xy
    %5:u32 = swizzle vec3<u32>(1u, 2u, 3u), z
    %6:i32 = convert %5
    %7:vec4<f32> = textureLoad %3, %4, %6)",
                             },
                             ImgData{
                                 .name = "2D array write unsigned",
                                 .spirv_type = "%float 2D 0 1 0 2 R32f",
                                 .spirv_fn = "OpImageWrite %im %vu123 %vf1234",
                                 .wgsl_type = "texture_storage_2d_array<r32float, read_write>",
                                 .wgsl_fn = R"(
    %4:vec2<u32> = swizzle vec3<u32>(1u, 2u, 3u), xy
    %5:u32 = swizzle vec3<u32>(1u, 2u, 3u), z
    %6:i32 = convert %5
    %7:void = textureStore %3, %4, %6, vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f))",
                             }));

// An ad-hoc test to prove we never had the issue feared in crbug.com/tint/265.
// Never create a const-declaration for a pointer to a texture or sampler. Code generation always
// traces back to the memory object declaration.
TEST_F(SpirvReaderTest, NeverGenerateConstDeclForHandle_UseVariableDirectly) {
    EXPECT_IR(R"(
    OpCapability Shader
    OpCapability Sampled1D
    OpCapability Image1D
    OpCapability StorageImageExtendedFormats
    OpCapability ImageQuery
    OpMemoryModel Logical Simple
     OpEntryPoint Fragment %100 "main"
     OpExecutionMode %100 OriginUpperLeft

     OpName %var "var"
     OpDecorate %var_im DescriptorSet 0
     OpDecorate %var_im Binding 0
     OpDecorate %var_s DescriptorSet 0
     OpDecorate %var_s Binding 1
  %float = OpTypeFloat 32
  %v4float = OpTypeVector %float 4
  %v2float = OpTypeVector %float 2
  %v2_0 = OpConstantNull %v2float
     %im = OpTypeImage %float 2D 0 0 0 1 Unknown
     %si = OpTypeSampledImage %im
      %s = OpTypeSampler
 %ptr_im = OpTypePointer UniformConstant %im
  %ptr_s = OpTypePointer UniformConstant %s
 %var_im = OpVariable %ptr_im UniformConstant
  %var_s = OpVariable %ptr_s UniformConstant
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void
 %ptr_v4 = OpTypePointer Function %v4float

    %100 = OpFunction %void None %voidfn
  %entry = OpLabel
    %var = OpVariable %ptr_v4 Function

; Try to induce generating a const-declaration of a pointer to
; a sampler or texture.

 %var_im_copy = OpCopyObject %ptr_im %var_im
  %var_s_copy = OpCopyObject %ptr_s %var_s

         %im0 = OpLoad %im %var_im_copy
          %s0 = OpLoad %s %var_s_copy
         %si0 = OpSampledImage %si %im0 %s0
          %t0 = OpImageSampleImplicitLod %v4float %si0 %v2_0


         %im1 = OpLoad %im %var_im_copy
          %s1 = OpLoad %s %var_s_copy
         %si1 = OpSampledImage %si %im1 %s1
          %t1 = OpImageSampleImplicitLod %v4float %si1 %v2_0

         %sum = OpFAdd %v4float %t0 %t1
           OpStore %var %sum

           OpReturn
           OpFunctionEnd
  )",
              R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%main = @fragment func():void {
  $B2: {
    %var:ptr<function, vec4<f32>, read_write> = var undef
    %5:texture_2d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSample %5, %6, vec2<f32>(0.0f)
    %8:texture_2d<f32> = load %1
    %9:sampler = load %2
    %10:vec4<f32> = textureSample %8, %9, vec2<f32>(0.0f)
    %11:vec4<f32> = add %7, %10
    store %var, %11
    ret
  }
}
)");
}

// When a sampler is loaded in an enclosing structured construct, don't generate a variable for it.
// The ordinary tracing logic will find the originating variable anyway.
// https://crbug.com/tint/1839
TEST_F(SpirvReaderTest, SamplerLoadedInEnclosingConstruct_DontGenerateVar) {
    EXPECT_IR(R"(
    OpCapability Shader
    OpCapability Sampled1D
    OpCapability Image1D
    OpCapability StorageImageExtendedFormats
    OpCapability ImageQuery
    OpMemoryModel Logical Simple
     OpEntryPoint Fragment %100 "main"
     OpExecutionMode %100 OriginUpperLeft

     OpName %var_im "var_im"
     OpName %var_s "var_s"
     OpDecorate %var_im DescriptorSet 0
     OpDecorate %var_im Binding 0
     OpDecorate %var_s DescriptorSet 0
     OpDecorate %var_s Binding 1
  %float = OpTypeFloat 32
  %v4float = OpTypeVector %float 4
  %v2float = OpTypeVector %float 2
  %v2_0 = OpConstantNull %v2float
     %im = OpTypeImage %float 2D 0 0 0 1 Unknown
     %si = OpTypeSampledImage %im
      %s = OpTypeSampler
 %ptr_im = OpTypePointer UniformConstant %im
  %ptr_s = OpTypePointer UniformConstant %s
 %var_im = OpVariable %ptr_im UniformConstant
  %var_s = OpVariable %ptr_s UniformConstant
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void
 %ptr_v4 = OpTypePointer Function %v4float
   %bool = OpTypeBool
   %true = OpConstantTrue %bool
    %int = OpTypeInt 32 1
  %int_0 = OpConstant %int 0

    %100 = OpFunction %void None %voidfn
  %entry = OpLabel
      %1 = OpLoad %im %var_im
      %2 = OpLoad %s %var_s
           OpSelectionMerge %90 None
           OpSwitch %int_0 %20

     %20 = OpLabel
           OpSelectionMerge %80 None
           OpBranchConditional %true %30 %80

     %30 = OpLabel
    %si0 = OpSampledImage %si %1 %2
     %t0 = OpImageSampleImplicitLod %v4float %si0 %v2_0
     %t1 = OpImageSampleImplicitLod %v4float %si0 %v2_0
           OpBranch %80

     %80 = OpLabel
           OpBranch %90

     %90 = OpLabel
           OpReturn
           OpFunctionEnd
  )",
              R"(
$B1: {  # root
  %var_im:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %var_s:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%main = @fragment func():void {
  $B2: {
    %4:texture_2d<f32> = load %var_im
    %5:sampler = load %var_s
    switch 0i [c: (default, $B3)] {  # switch_1
      $B3: {  # case
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %6:vec4<f32> = textureSample %4, %5, vec2<f32>(0.0f)
            %7:vec4<f32> = textureSample %4, %5, vec2<f32>(0.0f)
            exit_if  # if_1
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, SamplerLoaded_MultiUsage) {
    EXPECT_IR(R"(
           OpCapability Shader
           OpCapability Sampled1D
           OpCapability Image1D
           OpCapability StorageImageExtendedFormats
           OpCapability ImageQuery
           OpMemoryModel Logical Simple
           OpEntryPoint Fragment %100 "main"
           OpExecutionMode %100 OriginUpperLeft

           OpName %var_im "var_im"
           OpName %var_s "var_s"
           OpDecorate %var_im DescriptorSet 0
           OpDecorate %var_im Binding 0
           OpDecorate %var_s DescriptorSet 0
           OpDecorate %var_s Binding 1
  %float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%v2float = OpTypeVector %float 2
   %v2_0 = OpConstantNull %v2float
     %im = OpTypeImage %float 2D 0 0 0 1 Unknown
     %si = OpTypeSampledImage %im
      %s = OpTypeSampler
 %ptr_im = OpTypePointer UniformConstant %im
  %ptr_s = OpTypePointer UniformConstant %s
 %var_im = OpVariable %ptr_im UniformConstant
  %var_s = OpVariable %ptr_s UniformConstant
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void

    %100 = OpFunction %void None %voidfn
  %entry = OpLabel
      %1 = OpLoad %im %var_im
      %2 = OpLoad %s %var_s
    %si0 = OpSampledImage %si %1 %2
     %t0 = OpImageSampleImplicitLod %v4float %si0 %v2_0
    %si1 = OpSampledImage %si %1 %2
     %t2 = OpImageSampleImplicitLod %v4float %si1 %v2_0
           OpReturn
           OpFunctionEnd
  )",
              R"(
$B1: {  # root
  %var_im:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %var_s:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%main = @fragment func():void {
  $B2: {
    %4:texture_2d<f32> = load %var_im
    %5:sampler = load %var_s
    %6:vec4<f32> = textureSample %4, %5, vec2<f32>(0.0f)
    %7:vec4<f32> = textureSample %4, %5, vec2<f32>(0.0f)
    ret
  }
}
)");
}

// Demonstrates fix for crbug.com/tint/1646
// The problem is the coordinate for an image operation can be a combinatorial value that has been
// hoisted out to a 'var' declaration.
//
// In this test (and the original case form the bug), the definition for the value is in an outer
// construct, and the image operation using it is in a doubly nested construct.
//
// The coordinate handling has to unwrap the ref type it made for the 'var' declaration.
TEST_F(SpirvReaderTest, ImageCoordinateCanBeHoistedConstant) {
    EXPECT_IR(R"(
    OpCapability Shader
    OpCapability Sampled1D
    OpCapability Image1D
    OpCapability StorageImageExtendedFormats
    OpCapability ImageQuery
    OpMemoryModel Logical Simple
OpEntryPoint Fragment %100 "main"
OpExecutionMode %100 OriginUpperLeft
OpDecorate %10 DescriptorSet 0
OpDecorate %10 Binding 0
OpDecorate %20 DescriptorSet 2
OpDecorate %20 Binding 1

%void = OpTypeVoid
%voidfn = OpTypeFunction %void

%bool = OpTypeBool
%true = OpConstantTrue %bool
%float = OpTypeFloat 32

%v4float = OpTypeVector %float 4

%float_null = OpConstantNull %float

%sampler = OpTypeSampler
%ptr_sampler = OpTypePointer UniformConstant %sampler
%im_ty = OpTypeImage %float 1D 0 0 0 1 Unknown
%ptr_im_ty = OpTypePointer UniformConstant %im_ty
%si_ty = OpTypeSampledImage %im_ty

%10 = OpVariable %ptr_sampler UniformConstant
%20 = OpVariable %ptr_im_ty UniformConstant

%100 = OpFunction %void None %voidfn
%entry = OpLabel
%900 = OpCopyObject %float %float_null        ; definition here
OpSelectionMerge %99 None
OpBranchConditional %true %40 %99

  %40 = OpLabel
  OpSelectionMerge %80 None
  OpBranchConditional %true %50 %80

    %50 = OpLabel
    %sam = OpLoad %sampler %10
    %im = OpLoad %im_ty %20
    %sampled_image = OpSampledImage %si_ty %im %sam
    %result = OpImageSampleImplicitLod %v4float %sampled_image %900 ; usage here
    OpBranch %80

  %80 = OpLabel
  OpBranch %99

%99 = OpLabel
OpReturn
OpFunctionEnd
  )",
              R"(
$B1: {  # root
  %1:ptr<handle, sampler, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, texture_1d<f32>, read> = var undef @binding_point(2, 1)
}

%main = @fragment func():void {
  $B2: {
    %4:f32 = let 0.0f
    if true [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        if true [t: $B5, f: $B6] {  # if_2
          $B5: {  # true
            %5:sampler = load %1
            %6:texture_1d<f32> = load %2
            %7:vec4<f32> = textureSample %6, %5, %4
            exit_if  # if_2
          }
          $B6: {  # false
            exit_if  # if_2
          }
        }
        exit_if  # if_1
      }
      $B4: {  # false
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

// Demonstrates fix for crbug.com/tint/1712
// The problem is the coordinate for an image operation can be a combinatorial value that has been
// hoisted out to a 'var' declaration.
//
// In this test (and the original case form the bug), the definition for the value is in an outer
// construct, and the image operation using it is in a doubly nested construct.
//
// The coordinate handling has to unwrap the ref type it made for the 'var' declaration.
TEST_F(SpirvReaderTest, ImageCoordinateCanBeHoistedVector) {
    EXPECT_IR(R"(
    OpCapability Shader
    OpCapability Sampled1D
    OpCapability Image1D
    OpCapability StorageImageExtendedFormats
    OpCapability ImageQuery
    OpMemoryModel Logical Simple
OpEntryPoint Fragment %100 "main"
OpExecutionMode %100 OriginUpperLeft
OpDecorate %10 DescriptorSet 0
OpDecorate %10 Binding 0
OpDecorate %20 DescriptorSet 2
OpDecorate %20 Binding 1

%void = OpTypeVoid
%voidfn = OpTypeFunction %void

%bool = OpTypeBool
%true = OpConstantTrue %bool
%float = OpTypeFloat 32

%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4

%v2float_null = OpConstantNull %v2float

%sampler = OpTypeSampler
%ptr_sampler = OpTypePointer UniformConstant %sampler
%im_ty = OpTypeImage %float 1D 0 0 0 1 Unknown
%ptr_im_ty = OpTypePointer UniformConstant %im_ty
%si_ty = OpTypeSampledImage %im_ty

%10 = OpVariable %ptr_sampler UniformConstant
%20 = OpVariable %ptr_im_ty UniformConstant

%100 = OpFunction %void None %voidfn
%entry = OpLabel
%900 = OpCopyObject %v2float %v2float_null        ; definition here
OpSelectionMerge %99 None
OpBranchConditional %true %40 %99

  %40 = OpLabel
  OpSelectionMerge %80 None
  OpBranchConditional %true %50 %80

    %50 = OpLabel
    %sam = OpLoad %sampler %10
    %im = OpLoad %im_ty %20
    %sampled_image = OpSampledImage %si_ty %im %sam
    %result = OpImageSampleImplicitLod %v4float %sampled_image %900 ; usage here
    OpBranch %80

  %80 = OpLabel
  OpBranch %99

%99 = OpLabel
OpReturn
OpFunctionEnd
  )",
              R"(
$B1: {  # root
  %1:ptr<handle, sampler, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, texture_1d<f32>, read> = var undef @binding_point(2, 1)
}

%main = @fragment func():void {
  $B2: {
    %4:vec2<f32> = let vec2<f32>(0.0f)
    if true [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        if true [t: $B5, f: $B6] {  # if_2
          $B5: {  # true
            %5:sampler = load %1
            %6:texture_1d<f32> = load %2
            %7:f32 = swizzle %4, x
            %8:vec4<f32> = textureSample %6, %5, %7
            exit_if  # if_2
          }
          $B6: {  # false
            exit_if  # if_2
          }
        }
        exit_if  # if_1
      }
      $B4: {  # false
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

// Demonstrates fix for crbug.com/tint/1642 The problem is the texel value for an image write can be
// given in 'var' declaration.
//
// The texel value handling has to unwrap the ref type first.
TEST_F(SpirvReaderTest, TexelTypeWhenLoop) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability StorageImageExtendedFormats
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %100 "main"
               OpExecutionMode %100 LocalSize 8 8 1
               OpSource HLSL 600
               OpName %type_2d_image "type.2d.image"
               OpName %Output2Texture2D "Output2Texture2D"
               OpName %100 "main"
               OpDecorate %Output2Texture2D DescriptorSet 0
               OpDecorate %Output2Texture2D Binding 0
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
          %7 = OpConstantComposite %v2float %float_0 %float_0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
         %12 = OpConstantComposite %v2float %float_1 %float_1
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v2uint = OpTypeVector %uint 2
         %17 = OpConstantComposite %v2uint %uint_1 %uint_1
%type_2d_image = OpTypeImage %float 2D 2 0 0 2 Rg32f
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
       %bool = OpTypeBool
%Output2Texture2D = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
        %100 = OpFunction %void None %20
         %22 = OpLabel
               OpBranch %23
         %23 = OpLabel
         %24 = OpPhi %v2float %7 %22 %12 %25
         %26 = OpPhi %int %int_0 %22 %27 %25
         %28 = OpSLessThan %bool %26 %int_2
               OpLoopMerge %29 %25 None
               OpBranchConditional %28 %25 %29
         %25 = OpLabel
         %27 = OpIAdd %int %26 %int_1
               OpBranch %23
         %29 = OpLabel
         %30 = OpLoad %type_2d_image %Output2Texture2D
               OpImageWrite %30 %17 %24 None
               OpReturn
               OpFunctionEnd
  )",
              R"(
$B1: {  # root
  %Output2Texture2D:ptr<handle, texture_storage_2d<rg32float, read_write>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(8u, 8u, 1u) func():void {
  $B2: {
    %3:ptr<function, vec2<f32>, read_write> = var undef
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        %4:ptr<function, vec2<f32>, read_write> = var undef
        store %4, vec2<f32>(0.0f)
        %5:ptr<function, i32, read_write> = var undef
        store %5, 0i
        next_iteration  # -> $B4
      }
      $B4: {  # body
        %6:i32 = load %5
        %7:vec2<f32> = load %4
        %8:bool = lt %6, 2i
        if %8 [t: $B6, f: $B7] {  # if_1
          $B6: {  # true
            continue  # -> $B5
          }
          $B7: {  # false
            store %3, %7
            exit_loop  # loop_1
          }
        }
        unreachable
      }
      $B5: {  # continuing
        %9:i32 = add %6, 1i
        store %4, vec2<f32>(1.0f)
        store %5, %9
        next_iteration  # -> $B4
      }
    }
    %10:vec2<f32> = load %3
    %11:texture_storage_2d<rg32float, read_write> = load %Output2Texture2D
    %12:vec4<f32> = construct %10, vec2<f32>(0.0f)
    %13:void = textureStore %11, vec2<u32>(1u), %12
    ret
  }
)");
}

TEST_F(SpirvReaderTest, ReadWriteStorageTexture) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability StorageImageExtendedFormats
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %100 "main"
               OpExecutionMode %100 LocalSize 8 8 1
               OpSource HLSL 600
               OpName %type_2d_image "type.2d.image"
               OpName %RWTexture2D "RWTexture2D"
               OpName %100 "main"
               OpDecorate %RWTexture2D DescriptorSet 0
               OpDecorate %RWTexture2D Binding 0
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v2uint = OpTypeVector %uint 2
      %coord = OpConstantComposite %v2uint %uint_1 %uint_1
%type_2d_image = OpTypeImage %float 2D 2 0 0 2 Rgba32f
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
%RWTexture2D = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
        %100 = OpFunction %void None %20
         %22 = OpLabel
         %30 = OpLoad %type_2d_image %RWTexture2D
         %31 = OpImageRead %v4float %30 %coord None
         %32 = OpFAdd %v4float %31 %31
               OpImageWrite %30 %coord %32 None
               OpReturn
               OpFunctionEnd
  )",
              R"(
$B1: {  # root
  %RWTexture2D:ptr<handle, texture_storage_2d<rgba32float, read_write>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(8u, 8u, 1u) func():void {
  $B2: {
    %3:texture_storage_2d<rgba32float, read_write> = load %RWTexture2D
    %4:vec4<f32> = textureLoad %3, vec2<u32>(1u)
    %5:vec4<f32> = add %4, %4
    %6:void = textureStore %3, vec2<u32>(1u), %5
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Image_UnknownDepth_NonDepthUse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "main"
               OpExecutionMode %1 OriginUpperLeft
               OpDecorate %7 DescriptorSet 0
               OpDecorate %7 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %8 Binding 1
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %4 = OpTypeImage %float 2D 2 0 0 1 Unknown
          %5 = OpTypeSampler
    %v2float = OpTypeVector %float 2
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
          %6 = OpTypeSampledImage %4
%_ptr_UniformConstant_4 = OpTypePointer UniformConstant %4
          %7 = OpVariable %_ptr_UniformConstant_4 UniformConstant
%_ptr_UniformConstant_5 = OpTypePointer UniformConstant %5
          %8 = OpVariable %_ptr_UniformConstant_5 UniformConstant
      %const = OpConstantNull %v2float
          %1 = OpFunction %void None %15
         %18 = OpLabel
         %20 = OpLoad %4 %7
         %21 = OpLoad %5 %8
         %22 = OpSampledImage %6 %20 %21
         %23 = OpImageSampleImplicitLod %v4float %22 %const None
               OpReturn
               OpFunctionEnd
    )",
              R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%main = @fragment func():void {
  $B2: {
    %4:texture_2d<f32> = load %1
    %5:sampler = load %2
    %6:vec4<f32> = textureSample %4, %5, vec2<f32>(0.0f)
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Image_UnknownDepth_DepthUse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "main"
               OpExecutionMode %1 OriginUpperLeft
               OpDecorate %7 DescriptorSet 0
               OpDecorate %7 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %8 Binding 1
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %4 = OpTypeImage %float 2D 2 0 0 1 Unknown
          %5 = OpTypeSampler
    %v2float = OpTypeVector %float 2
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
          %6 = OpTypeSampledImage %4
%_ptr_UniformConstant_4 = OpTypePointer UniformConstant %4
          %7 = OpVariable %_ptr_UniformConstant_4 UniformConstant
%_ptr_UniformConstant_5 = OpTypePointer UniformConstant %5
          %8 = OpVariable %_ptr_UniformConstant_5 UniformConstant
      %const = OpConstantNull %v2float
    %float_0 = OpConstantNull %float
          %1 = OpFunction %void None %15
         %18 = OpLabel
         %20 = OpLoad %4 %7
         %21 = OpLoad %5 %8
         %22 = OpSampledImage %6 %20 %21
         %23 = OpImageSampleDrefImplicitLod %float %22 %const %float_0 None
               OpReturn
               OpFunctionEnd
    )",
              R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%main = @fragment func():void {
  $B2: {
    %4:texture_depth_2d = load %1
    %5:sampler_comparison = load %2
    %6:f32 = textureSampleCompare %4, %5, vec2<f32>(0.0f), 0.0f
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Let_Sampler) {
    EXPECT_IR(R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
               OpName %t "t"
               OpName %s "s"
               OpDecorate %t Binding 0
               OpDecorate %t DescriptorSet 0
               OpDecorate %s Binding 1
               OpDecorate %s DescriptorSet 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
          %t = OpVariable %_ptr_UniformConstant_10 UniformConstant
         %14 = OpTypeSampler
%_ptr_UniformConstant_14 = OpTypePointer UniformConstant %14
          %s = OpVariable %_ptr_UniformConstant_14 UniformConstant
         %18 = OpTypeSampledImage %10
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %22 = OpConstantComposite %v2float %float_0 %float_0
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpLoad %10 %t
         %17 = OpLoad %14 %s
        %100 = OpCopyObject %14 %17
         %19 = OpSampledImage %18 %13 %100
         %23 = OpImageSampleExplicitLod %v4float %19 %22 Lod %float_0
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %t:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %s:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:texture_2d<f32> = load %t
    %5:sampler = load %s
    %6:vec4<f32> = textureSampleLevel %4, %5, vec2<f32>(0.0f), 0.0f
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Let_SampledImage) {
    EXPECT_IR(R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
               OpName %t "t"
               OpName %s "s"
               OpDecorate %t Binding 0
               OpDecorate %t DescriptorSet 0
               OpDecorate %s Binding 1
               OpDecorate %s DescriptorSet 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
          %t = OpVariable %_ptr_UniformConstant_10 UniformConstant
         %14 = OpTypeSampler
%_ptr_UniformConstant_14 = OpTypePointer UniformConstant %14
          %s = OpVariable %_ptr_UniformConstant_14 UniformConstant
         %18 = OpTypeSampledImage %10
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %22 = OpConstantComposite %v2float %float_0 %float_0
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpLoad %10 %t
         %17 = OpLoad %14 %s
         %19 = OpSampledImage %18 %13 %17
        %100 = OpCopyObject %18 %19
         %23 = OpImageSampleExplicitLod %v4float %100 %22 Lod %float_0
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %t:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %s:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:texture_2d<f32> = load %t
    %5:sampler = load %s
    %6:vec4<f32> = textureSampleLevel %4, %5, vec2<f32>(0.0f), 0.0f
    ret
  }
}
)");
}

// https://crbug.com/430358345
TEST_F(SpirvReaderTest, Image_UserCall_Params) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %main_position_Input
               OpExecutionMode %main OriginUpperLeft
               OpName %sampler0 "sampler0"
               OpName %texture0 "texture0"
               OpName %main_position_Input "main_position_Input"
               OpName %foo "foo"
               OpName %t "t"
               OpName %s "s"
               OpName %main "main"
               OpDecorate %sampler0 DescriptorSet 0
               OpDecorate %sampler0 Binding 0
               OpDecorate %texture0 DescriptorSet 0
               OpDecorate %texture0 Binding 1
               OpDecorate %main_position_Input BuiltIn FragCoord
          %3 = OpTypeSampler
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
   %sampler0 = OpVariable %_ptr_UniformConstant_3 UniformConstant
      %float = OpTypeFloat 32
          %6 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_6 = OpTypePointer UniformConstant %6
   %texture0 = OpVariable %_ptr_UniformConstant_6 UniformConstant
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%main_position_Input = OpVariable %_ptr_Input_v4float Input
         %17 = OpTypeFunction %v4float %6 %3
    %v2float = OpTypeVector %float 2
    %float_2 = OpConstant %float 2
         %22 = OpConstantComposite %v2float %float_2 %float_2
         %25 = OpTypeSampledImage %6
         %29 = OpTypeFunction %v4float %v4float
       %void = OpTypeVoid
         %36 = OpTypeFunction %void

        %foo = OpFunction %v4float None %17
          %t = OpFunctionParameter %6
          %s = OpFunctionParameter %3
         %18 = OpLabel
         %24 = OpSampledImage %25 %t %s
         %26 = OpImageSampleImplicitLod %v4float %24 %22 None
               OpReturnValue %26
               OpFunctionEnd

       %main = OpFunction %void None %36
         %37 = OpLabel
         %31 = OpLoad %6 %texture0 None
         %32 = OpLoad %3 %sampler0 None
         %33 = OpFunctionCall %v4float %foo %31 %32
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %sampler0:ptr<handle, sampler, read> = var undef @binding_point(0, 0)
  %texture0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 1)
}

%main = @fragment func(%main_position_Input:vec4<f32> [@position]):void {
  $B2: {
    %5:texture_2d<f32> = load %texture0
    %6:sampler = load %sampler0
    %7:vec4<f32> = call %foo, %5, %6
    ret
  }
}
%foo = func(%t:texture_2d<f32>, %s:sampler):vec4<f32> {
  $B3: {
    %11:vec4<f32> = textureSample %t, %s, vec2<f32>(2.0f)
    ret %11
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
