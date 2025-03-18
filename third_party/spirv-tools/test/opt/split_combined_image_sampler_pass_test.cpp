// Copyright (c) 2025 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <array>
#include <iostream>
#include <ostream>

#include "spirv-tools/optimizer.hpp"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

struct SplitCombinedImageSamplerPassTest : public PassTest<::testing::Test> {
  virtual void SetUp() override {
    SetTargetEnv(SPV_ENV_VULKAN_1_0);
    SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
    SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES |
                          SPV_BINARY_TO_TEXT_OPTION_INDENT |
                          SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  }
};

struct TypeCase {
  const char* glsl_type;
  const char* image_type_decl;
};
std::ostream& operator<<(std::ostream& os, const TypeCase& tc) {
  os << tc.glsl_type;
  return os;
}

struct SplitCombinedImageSamplerPassTypeCaseTest
    : public PassTest<::testing::TestWithParam<TypeCase>> {
  virtual void SetUp() override {
    SetTargetEnv(SPV_ENV_VULKAN_1_0);
    SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
    SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES |
                          SPV_BINARY_TO_TEXT_OPTION_INDENT |
                          SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  }
};

std::vector<TypeCase> ImageTypeCases() {
  return std::vector<TypeCase>{
      {"sampler2D", "OpTypeImage %float 2D 0 0 0 1 Unknown"},
      {"sampler2DShadow", "OpTypeImage %float 2D 1 0 0 1 Unknown"},
      {"sampler2DArray", "OpTypeImage %float 2D 0 1 0 1 Unknown"},
      {"sampler2DArrayShadow", "OpTypeImage %float 2D 1 1 0 1 Unknown"},
      {"sampler2DMS", "OpTypeImage %float 2D 0 0 1 1 Unknown"},
      {"sampler2DMSArray", "OpTypeImage %float 2D 0 1 1 1 Unknown"},
      {"sampler3D", "OpTypeImage %float 3D 0 0 0 1 Unknown"},
      {"samplerCube", "OpTypeImage %float Cube 0 0 0 1 Unknown"},
      {"samplerCubeShadow", "OpTypeImage %float Cube 1 0 0 1 Unknown"},
      {"samplerCubeArray", "OpTypeImage %float Cube 0 1 0 1 Unknown"},
      {"samplerCubeArrayShadow", "OpTypeImage %float Cube 1 1 0 1 Unknown"},
      {"isampler2D", "OpTypeImage %int 2D 0 0 0 1 Unknown"},
      {"isampler2DShadow", "OpTypeImage %int 2D 1 0 0 1 Unknown"},
      {"isampler2DArray", "OpTypeImage %int 2D 0 1 0 1 Unknown"},
      {"isampler2DArrayShadow", "OpTypeImage %int 2D 1 1 0 1 Unknown"},
      {"isampler2DMS", "OpTypeImage %int 2D 0 0 1 1 Unknown"},
      {"isampler2DMSArray", "OpTypeImage %int 2D 0 1 1 1 Unknown"},
      {"isampler3D", "OpTypeImage %int 3D 0 0 0 1 Unknown"},
      {"isamplerCube", "OpTypeImage %int Cube 0 0 0 1 Unknown"},
      {"isamplerCubeShadow", "OpTypeImage %int Cube 1 0 0 1 Unknown"},
      {"isamplerCubeArray", "OpTypeImage %int Cube 0 1 0 1 Unknown"},
      {"isamplerCubeArrayShadow", "OpTypeImage %int Cube 1 1 0 1 Unknown"},
      {"usampler2D", "OpTypeImage %uint 2D 0 0 0 1 Unknown"},
      {"usampler2DShadow", "OpTypeImage %uint 2D 1 0 0 1 Unknown"},
      {"usampler2DArray", "OpTypeImage %uint 2D 0 1 0 1 Unknown"},
      {"usampler2DArrayShadow", "OpTypeImage %uint 2D 1 1 0 1 Unknown"},
      {"usampler2DMS", "OpTypeImage %uint 2D 0 0 1 1 Unknown"},
      {"usampler2DMSArray", "OpTypeImage %uint 2D 0 1 1 1 Unknown"},
      {"usampler3D", "OpTypeImage %uint 3D 0 0 0 1 Unknown"},
      {"usamplerCube", "OpTypeImage %uint Cube 0 0 0 1 Unknown"},
      {"usamplerCubeShadow", "OpTypeImage %uint Cube 1 0 0 1 Unknown"},
      {"usamplerCubeArray", "OpTypeImage %uint Cube 0 1 0 1 Unknown"},
      {"usamplerCubeArrayShadow", "OpTypeImage %uint Cube 1 1 0 1 Unknown"},
  };
}

std::string Preamble(const std::string shader_interface = "") {
  return R"(               OpCapability Shader
               OpCapability RuntimeDescriptorArray
               OpExtension "SPV_EXT_descriptor_indexing"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main")" +
         shader_interface + R"(
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
               OpName %main_0 "main_0"
               OpName %voidfn "voidfn"
)";
}

std::string PreambleFragment(const std::string shader_interface = "") {
  return R"(               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main")" +
         shader_interface + R"(
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
               OpName %main_0 "main_0"
               OpName %voidfn "voidfn"
)";
}

std::string BasicTypes() {
  return R"(      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
    %v4float = OpTypeVector %float 4
         %13 = OpConstantNull %v2float
         %14 = OpConstantNull %v3float
         %15 = OpConstantNull %v4float
       %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
)";
}
std::string Main() {
  return R"(
       %main = OpFunction %void None %voidfn
     %main_0 = OpLabel
               OpReturn
               OpFunctionEnd
)";
}
std::string NoCheck() { return "; CHECK-NOT: nothing to see"; }

TEST_F(SplitCombinedImageSamplerPassTest, SamplerOnly_NoChange) {
  const std::string kTest = Preamble() +
                            R"(               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0
)" + BasicTypes() + R"(         %10 = OpTypeSampler
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
        %100 = OpVariable %_ptr_UniformConstant_10 UniformConstant
       %main = OpFunction %void None %voidfn
     %main_0 = OpLabel
          %6 = OpLoad %10 %100
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest + NoCheck(), /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithoutChange)
      << "status" << kTest << "\n -> \n"
      << disasm;
  EXPECT_EQ(disasm, kTest) << "disasm";
}

TEST_F(SplitCombinedImageSamplerPassTest, ImageOnly_NoChange) {
  const std::string kTest = Preamble() +
                            R"(               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0
)" + BasicTypes() + R"(         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
        %100 = OpVariable %_ptr_UniformConstant_10 UniformConstant
       %main = OpFunction %void None %voidfn
     %main_0 = OpLabel
          %6 = OpLoad %10 %100
               OpReturn
               OpFunctionEnd
)";

  SCOPED_TRACE("image only");
  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest + NoCheck(), /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithoutChange);
  EXPECT_EQ(disasm, kTest);
}

TEST_F(SplitCombinedImageSamplerPassTest, PtrSampledImageOnly_DeletesPtrType) {
  const std::string kTest = Preamble() + BasicTypes() + R"(
  ; CHECK: OpCapability Shader
  ; CHECK-NOT: OpTypePointer UniformConstant
  ; CHECK: OpFunction %void
        %100 = OpTypeImage %float 2D 0 0 0 1 Unknown
        %101 = OpTypeSampledImage %100
        %102 = OpTypePointer UniformConstant %101
       %main = OpFunction %void None %voidfn
     %main_0 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest + NoCheck(), /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << "status";
}

TEST_F(SplitCombinedImageSamplerPassTest,
       PtrArraySampledImageOnly_DeletesPtrType) {
  const std::string kTest = Preamble() + BasicTypes() + R"(
  ; CHECK: OpCapability Shader
  ; CHECK-NOT: OpTypePointer UniformConstant
  ; CHECK: OpFunction %void
        %100 = OpTypeImage %float 2D 0 0 0 1 Unknown
        %101 = OpTypeSampledImage %100
        %103 = OpTypeArray %101 %uint_1
        %104 = OpTypePointer UniformConstant %103
       %main = OpFunction %void None %voidfn
     %main_0 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest + NoCheck(), /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << "status";
}

TEST_F(SplitCombinedImageSamplerPassTest,
       PtrRtArraySampledImageOnly_DeletesPtrType) {
  const std::string kTest = Preamble() + BasicTypes() + R"(
  ; CHECK: OpCapability Shader
  ; CHECK-NOT: OpTypePointer UniformConstant
  ; CHECK: OpFunction %void
        %100 = OpTypeImage %float 2D 0 0 0 1 Unknown
        %101 = OpTypeSampledImage %100
        %103 = OpTypeRuntimeArray %101
        %104 = OpTypePointer UniformConstant %103
       %main = OpFunction %void None %voidfn
     %main_0 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest + NoCheck(), /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << "status";
}

TEST_F(SplitCombinedImageSamplerPassTest,
       Combined_NoSampler_CreatedBeforeSampledImage) {
  // No OpTypeSampler to begin with.
  const std::string kTest = Preamble() +
                            R"(               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0

     ; A sampler type is created and placed at the start of types.
     ; CHECK: OpDecorate %{{\d+}} Binding 0
     ; CHECK: OpDecorate %{{\d+}} Binding 0
     ; CHECK-NOT: TypeSampledImage
     ; CHECK: TypeSampler
     ; CHECK: TypeSampledImage

)" + BasicTypes() + R"( %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11

        %100 = OpVariable %_ptr_UniformConstant_11 UniformConstant
       %main = OpFunction %void None %voidfn
     %main_0 = OpLabel
          %6 = OpLoad %11 %100
               OpReturn
               OpFunctionEnd
)";
  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest, Combined_SynthesizeVarNames) {
  // Also tests binding info is copied to both variables.
  const std::string kTest = Preamble() +
                            R"(
               OpName %orig_var "orig_var"
               OpDecorate %orig_var DescriptorSet 0
               OpDecorate %orig_var Binding 0

     ; The combined image variable is replaced by an image variable and a sampler variable.

     ; CHECK: OpCapability
     ; The original name is deleted
     ; CHECK-NOT: OpName %orig_var "
     ; CHECK: OpName %orig_var_image "orig_var_image"
     ; CHECK: OpName %orig_var_sampler "orig_var_sampler"
     ; CHECK-NOT: OpName %orig_var "

     ; CHECK: OpDecorate %orig_var_image DescriptorSet 0
     ; CHECK: OpDecorate %orig_var_sampler DescriptorSet 0
     ; CHECK: OpDecorate %orig_var_image Binding 0
     ; CHECK: OpDecorate %orig_var_sampler Binding 0

     ; CHECK: %10 = OpTypeImage %
     ; CHECK: %[[image_ptr_ty:\w+]] = OpTypePointer UniformConstant %10
     ; CHECK: %[[sampler_ty:\d+]] = OpTypeSampler
     ; CHECK: %[[sampler_ptr_ty:\w+]] = OpTypePointer UniformConstant %[[sampler_ty]]


     ; CHECK-NOT: %orig_var = OpVariable
     ; CHECK-DAG: %orig_var_sampler = OpVariable %[[sampler_ptr_ty]] UniformConstant
     ; CHECK-DAG: %orig_var_image = OpVariable %[[image_ptr_ty]] UniformConstant
     ; CHECK: = OpFunction

)" + BasicTypes() + R"(
        %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11

   %orig_var = OpVariable %_ptr_UniformConstant_11 UniformConstant
       %main = OpFunction %void None %voidfn
     %main_0 = OpLabel
        %101 = OpLoad %11 %orig_var
               OpReturn
               OpFunctionEnd
)";
  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_P(SplitCombinedImageSamplerPassTypeCaseTest, Combined_RemapLoad) {
  // Also tests binding info is copied to both variables.
  const std::string kTest = Preamble() +
                            R"(
               OpName %combined "combined"
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0

     ; CHECK: OpName
     ; CHECK-NOT: OpDecorate %100
     ; CHECK: OpDecorate %[[image_var:\d+]] DescriptorSet 0
     ; CHECK: OpDecorate %[[sampler_var:\d+]] DescriptorSet 0
     ; CHECK: OpDecorate %[[image_var]] Binding 0
     ; CHECK: OpDecorate %[[sampler_var]] Binding 0

     ; CHECK: %10 = OpTypeImage %
     ; CHECK: %[[image_ptr_ty:\w+]] = OpTypePointer UniformConstant %10
     ; CHECK: %[[sampler_ty:\d+]] = OpTypeSampler
     ; CHECK: %[[sampler_ptr_ty:\w+]] = OpTypePointer UniformConstant %[[sampler_ty]]

     ; The combined image variable is replaced by an image variable and a sampler variable.

     ; CHECK-NOT: %100 = OpVariable
     ; CHECK-DAG: %[[sampler_var]] = OpVariable %[[sampler_ptr_ty]] UniformConstant
     ; CHECK-DAG: %[[image_var]] = OpVariable %[[image_ptr_ty]] UniformConstant
     ; CHECK: = OpFunction

     ; The load of the combined image+sampler is replaced by a two loads, then
     ; a combination operation.
     ; CHECK: %[[im:\d+]] = OpLoad %10 %[[image_var]]
     ; CHECK: %[[s:\d+]] = OpLoad %[[sampler_ty]] %[[sampler_var]]
     ; CHECK: %combined = OpSampledImage %11 %[[im]] %[[s]]

)" + BasicTypes() +
                            " %10 = " + GetParam().image_type_decl + R"(
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11

        %100 = OpVariable %_ptr_UniformConstant_11 UniformConstant
       %main = OpFunction %void None %voidfn
     %main_0 = OpLabel
   %combined = OpLoad %11 %100

     ; Uses of the combined image sampler are preserved.
     ; CHECK: OpCopyObject %11 %combined

          %7 = OpCopyObject %11 %combined
               OpReturn
               OpFunctionEnd
)";
  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_P(SplitCombinedImageSamplerPassTypeCaseTest,
       Combined_RemapLoad_RelaxedPrecisionCopied) {
  // All decorations are copied. In this case, RelaxedPrecision
  const std::string kTest = Preamble() +
                            R"(
               OpName %combined "combined"
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0
               OpDecorate %100 RelaxedPrecision

     ; CHECK: OpName
     ; CHECK-NOT: OpDecorate %100
     ; CHECK: OpDecorate %[[image_var:\d+]] DescriptorSet 0
     ; CHECK: OpDecorate %[[sampler_var:\d+]] DescriptorSet 0
     ; CHECK: OpDecorate %[[image_var]] Binding 0
     ; CHECK: OpDecorate %[[sampler_var]] Binding 0
     ; CHECK: OpDecorate %[[image_var:\d+]] RelaxedPrecision
     ; CHECK: OpDecorate %[[sampler_var:\d+]] RelaxedPrecision

     ; CHECK: %10 = OpTypeImage %
     ; CHECK: %[[image_ptr_ty:\w+]] = OpTypePointer UniformConstant %10
     ; CHECK: %[[sampler_ty:\d+]] = OpTypeSampler
     ; CHECK: %[[sampler_ptr_ty:\w+]] = OpTypePointer UniformConstant %[[sampler_ty]]

     ; The combined image variable is replaced by an image variable and a sampler variable.

     ; CHECK-NOT: %100 = OpVariable
     ; CHECK-DAG: %[[sampler_var]] = OpVariable %[[sampler_ptr_ty]] UniformConstant
     ; CHECK-DAG: %[[image_var]] = OpVariable %[[image_ptr_ty]] UniformConstant
     ; CHECK: = OpFunction

     ; The load of the combined image+sampler is replaced by a two loads, then
     ; a combination operation.
     ; CHECK: %[[im:\d+]] = OpLoad %10 %[[image_var]]
     ; CHECK: %[[s:\d+]] = OpLoad %[[sampler_ty]] %[[sampler_var]]
     ; CHECK: %combined = OpSampledImage %11 %[[im]] %[[s]]

               %bool = OpTypeBool ; location marker
)" + BasicTypes() +
                            " %10 = " + GetParam().image_type_decl + R"(
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11

        %100 = OpVariable %_ptr_UniformConstant_11 UniformConstant
       %main = OpFunction %void None %voidfn
     %main_0 = OpLabel
   %combined = OpLoad %11 %100

     ; Uses of the combined image sampler are preserved.
     ; CHECK: OpCopyObject %11 %combined

          %7 = OpCopyObject %11 %combined
               OpReturn
               OpFunctionEnd
)";
  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_P(SplitCombinedImageSamplerPassTypeCaseTest,
       Combined_DeletesCopyObjectOfPtr) {
  // OpCopyObject is deleted, and its uses updated.
  const std::string kTest = Preamble() +
                            R"(
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0

     ; CHECK: OpName
     ; CHECK-NOT: OpDecorate %100
     ; CHECK: OpDecorate %[[image_var:\d+]] DescriptorSet 0
     ; CHECK: OpDecorate %[[sampler_var:\d+]] DescriptorSet 0
     ; CHECK: OpDecorate %[[image_var]] Binding 0
     ; CHECK: OpDecorate %[[sampler_var]] Binding 0

     ; CHECK: %10 = OpTypeImage %
     ; CHECK: %[[image_ptr_ty:\w+]] = OpTypePointer UniformConstant %10
     ; CHECK: %[[sampler_ty:\d+]] = OpTypeSampler
     ; CHECK: %[[sampler_ptr_ty:\w+]] = OpTypePointer UniformConstant %[[sampler_ty]]

     ; The combined image variable is replaced by an image variable and a sampler variable.

     ; CHECK-NOT: %100 = OpVariable
     ; CHECK-DAG: %[[sampler_var]] = OpVariable %[[sampler_ptr_ty]] UniformConstant
     ; CHECK-DAG: %[[image_var]] = OpVariable %[[image_ptr_ty]] UniformConstant
     ; CHECK: = OpFunction


)" + BasicTypes() +
                            " %10 = " + GetParam().image_type_decl + R"(
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11

        %100 = OpVariable %_ptr_UniformConstant_11 UniformConstant
       %main = OpFunction %void None %voidfn
     %main_0 = OpLabel
        %101 = OpCopyObject %_ptr_UniformConstant_11 %100
        %102 = OpLoad %11 %101
        %103 = OpCopyObject %_ptr_UniformConstant_11 %101
        %104 = OpCopyObject %11 %102 ;; this copy survives
               OpReturn
               OpFunctionEnd

     ; The OpCopyObject instructions are removed.
     ; The load of the combined image+sampler is replaced by a two loads, then
     ; a combination operation. The only OpCopyObject that remains is the copy
     ; of the copy of the sampled image value.
     ; CHECK: %[[im:\d+]] = OpLoad %10 %[[image_var]]
     ; CHECK: %[[s:\d+]] = OpLoad %[[sampler_ty]] %[[sampler_var]]
     ; CHECK: %[[si:\d+]] = OpSampledImage %11 %[[im]] %[[s]]
     ; CHECK-NEXT: OpCopyObject %11 %[[si]]
     ; CHECK-NEXT: OpReturn
)";

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_P(SplitCombinedImageSamplerPassTypeCaseTest, ArrayCombined_RemapLoad) {
  const std::string kTest = Preamble() +
                            R"(
               OpName %combined "combined"
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0

     ; CHECK: OpName
     ; CHECK-NOT: OpDecorate %100
     ; CHECK: OpDecorate %[[image_var:\d+]] DescriptorSet 0
     ; CHECK: OpDecorate %[[sampler_var:\d+]] DescriptorSet 0
     ; CHECK: OpDecorate %[[image_var]] Binding 0
     ; CHECK: OpDecorate %[[sampler_var]] Binding 0

     ; CHECK: %10 = OpTypeImage %
     ; CHECK: %[[image_ptr_ty:\w+]] = OpTypePointer UniformConstant %10
     ; CHECK: %[[sampler_ty:\d+]] = OpTypeSampler
     ; CHECK: %[[sampler_ptr_ty:\w+]] = OpTypePointer UniformConstant %[[sampler_ty]]

     ; The combined image variable is replaced by an image variable and a sampler variable.

     ; CHECK: %[[array_image_ty:\w+]] = OpTypeArray %10 %uint_3
     ; CHECK: %[[ptr_array_image_ty:\w+]] = OpTypePointer UniformConstant %[[array_image_ty]]

     ; CHECK: %[[array_sampler_ty:\w+]] = OpTypeArray %[[sampler_ty]] %uint_3
     ; CHECK: %[[ptr_array_sampler_ty:\w+]] = OpTypePointer UniformConstant %[[array_sampler_ty]]

     ; CHECK-NOT: %100 = OpVariable
     ; CHECK-DAG: %[[sampler_var]] = OpVariable %[[ptr_array_sampler_ty]] UniformConstant
     ; CHECK-DAG: %[[image_var]] = OpVariable %[[ptr_array_image_ty]] UniformConstant
     ; CHECK: = OpFunction

     ; The access chain and load is replaced by two access chains, two loads, then
     ; a combine operation.
     ; CHECK: %[[ptr_im:\d+]] = OpAccessChain %[[image_ptr_ty]] %[[image_var]] %uint_1
     ; CHECK: %[[ptr_s:\d+]] = OpAccessChain %[[sampler_ptr_ty]] %[[sampler_var]] %uint_1
     ; CHECK: %[[im:\d+]] = OpLoad %10 %[[ptr_im]]
     ; CHECK: %[[s:\d+]] = OpLoad %[[sampler_ty]] %[[ptr_s]]
     ; CHECK: %combined = OpSampledImage %11 %[[im]] %[[s]]

)" + BasicTypes() +
                            " %10 = " + GetParam().image_type_decl + R"(
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
         %12 = OpTypeArray %11 %uint_3
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12

        %100 = OpVariable %_ptr_UniformConstant_12 UniformConstant
       %main = OpFunction %void None %voidfn
     %main_0 = OpLabel
        %ptr = OpAccessChain %_ptr_UniformConstant_11 %100 %uint_1
   %combined = OpLoad %11 %ptr

     ; Uses of the combined image sampler are preserved.
     ; CHECK: OpCopyObject %11 %combined

          %7 = OpCopyObject %11 %combined
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_P(SplitCombinedImageSamplerPassTypeCaseTest, RtArrayCombined_RemapLoad) {
  const std::string kTest = Preamble() +
                            R"(
               OpName %combined "combined"
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0

     ; CHECK: OpName
     ; CHECK-NOT: OpDecorate %100
     ; CHECK: OpDecorate %[[image_var:\d+]] DescriptorSet 0
     ; CHECK: OpDecorate %[[sampler_var:\d+]] DescriptorSet 0
     ; CHECK: OpDecorate %[[image_var]] Binding 0
     ; CHECK: OpDecorate %[[sampler_var]] Binding 0

     ; CHECK: %10 = OpTypeImage %
     ; CHECK: %[[image_ptr_ty:\w+]] = OpTypePointer UniformConstant %10
     ; CHECK: %[[sampler_ty:\d+]] = OpTypeSampler
     ; CHECK: %[[sampler_ptr_ty:\w+]] = OpTypePointer UniformConstant %[[sampler_ty]]

     ; The combined image variable is replaced by an image variable and a sampler variable.

     ; CHECK: %[[array_image_ty:\w+]] = OpTypeRuntimeArray %10
     ; CHECK: %[[ptr_array_image_ty:\w+]] = OpTypePointer UniformConstant %[[array_image_ty]]

     ; CHECK: %[[array_sampler_ty:\w+]] = OpTypeRuntimeArray %[[sampler_ty]]
     ; CHECK: %[[ptr_array_sampler_ty:\w+]] = OpTypePointer UniformConstant %[[array_sampler_ty]]

     ; CHECK-NOT: %100 = OpVariable
     ; CHECK-DAG: %[[sampler_var]] = OpVariable %[[ptr_array_sampler_ty]] UniformConstant
     ; CHECK-DAG: %[[image_var]] = OpVariable %[[ptr_array_image_ty]] UniformConstant
     ; CHECK: = OpFunction

     ; The access chain and load is replaced by two access chains, two loads, then
     ; a combine operation.
     ; CHECK: %[[ptr_im:\d+]] = OpAccessChain %[[image_ptr_ty]] %[[image_var]] %uint_1
     ; CHECK: %[[ptr_s:\d+]] = OpAccessChain %[[sampler_ptr_ty]] %[[sampler_var]] %uint_1
     ; CHECK: %[[im:\d+]] = OpLoad %10 %[[ptr_im]]
     ; CHECK: %[[s:\d+]] = OpLoad %[[sampler_ty]] %[[ptr_s]]
     ; CHECK: %combined = OpSampledImage %11 %[[im]] %[[s]]

)" + BasicTypes() +
                            " %10 = " + GetParam().image_type_decl + R"(
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
         %12 = OpTypeRuntimeArray %11
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12

        %100 = OpVariable %_ptr_UniformConstant_12 UniformConstant
       %main = OpFunction %void None %voidfn
     %main_0 = OpLabel
        %ptr = OpAccessChain %_ptr_UniformConstant_11 %100 %uint_1
   %combined = OpLoad %11 %ptr

     ; Uses of the combined image sampler are preserved.
     ; CHECK: OpCopyObject %11 %combined

          %7 = OpCopyObject %11 %combined
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

INSTANTIATE_TEST_SUITE_P(AllCombinedTypes,
                         SplitCombinedImageSamplerPassTypeCaseTest,
                         ::testing::ValuesIn(ImageTypeCases()));

// Remap entry point

struct EntryPointRemapCase {
  const spv_target_env environment = SPV_ENV_VULKAN_1_0;
  const char* initial_interface = "";
  const char* expected_interface = nullptr;
};

std::ostream& operator<<(std::ostream& os, const EntryPointRemapCase& eprc) {
  os << "(env " << spvLogStringForEnv(eprc.environment) << ", init "
     << eprc.initial_interface << " -> expect " << eprc.expected_interface
     << ")";
  return os;
}

struct SplitCombinedImageSamplerPassEntryPointRemapTest
    : public PassTest<::testing::TestWithParam<EntryPointRemapCase>> {
  virtual void SetUp() override {
    SetTargetEnv(GetParam().environment);
    SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
    SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES |
                          SPV_BINARY_TO_TEXT_OPTION_INDENT |
                          SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  }
};

std::vector<EntryPointRemapCase> EntryPointInterfaceCases() {
  return std::vector<EntryPointRemapCase>{
      {SPV_ENV_VULKAN_1_0, " %in_var %out_var", " %in_var %out_var"},
      {SPV_ENV_VULKAN_1_4, " %combined_var",
       " %combined_var_image %combined_var_sampler"},
      {SPV_ENV_VULKAN_1_4, " %combined_var %in_var %out_var",
       " %combined_var_image %in_var %out_var %combined_var_sampler"},
      {SPV_ENV_VULKAN_1_4, " %in_var %combined_var %out_var",
       " %in_var %combined_var_image %out_var %combined_var_sampler"},
      {SPV_ENV_VULKAN_1_4, " %in_var %out_var %combined_var",
       " %in_var %out_var %combined_var_image %combined_var_sampler"},
  };
}

TEST_P(SplitCombinedImageSamplerPassEntryPointRemapTest,
       EntryPoint_Combined_UsedInShader) {
  const std::string kTest = PreambleFragment(GetParam().initial_interface) +
                            R"(
               OpName %combined "combined"
               OpName %combined_var "combined_var"
               OpName %in_var "in_var"
               OpName %out_var "out_var"
               OpDecorate %combined_var DescriptorSet 0
               OpDecorate %combined_var Binding 0
               OpDecorate %in_var BuiltIn FragCoord
               OpDecorate %out_var Location 0

; CHECK: OpEntryPoint Fragment %main "main")" +
                            GetParam().expected_interface + R"(
; These clauses ensure the expected interface is the whole interface.
; CHECK-NOT: %{{\d+}}
; CHECK-NOT: %in_var
; CHECK-NOT: %out_var
; CHECK-NOT: %combined_var
; CHECK: OpExecutionMode %main OriginUpperLeft

     ; Check the var names, tracing up through the types.
     ; CHECK: %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
     ; CHECK: %[[image_ptr_ty:\w+]] = OpTypePointer UniformConstant %10
     ; CHECK: %[[sampler_ty:\d+]] = OpTypeSampler
     ; CHECK: %[[sampler_ptr_ty:\w+]] = OpTypePointer UniformConstant %[[sampler_ty]]
     ; The combined image variable is replaced by an image variable and a sampler variable.
     ; CHECK-DAG: %combined_var_sampler = OpVariable %[[sampler_ptr_ty]] UniformConstant
     ; CHECK-DAG: %combined_var_image = OpVariable %[[image_ptr_ty]] UniformConstant
     ; CHECK: = OpFunction

               %bool = OpTypeBool
)" + BasicTypes() + R"(         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
     %in_ptr_v4f = OpTypePointer Input %v4float
     %in_var = OpVariable %in_ptr_v4f Input
    %out_ptr_v4f = OpTypePointer Output %v4float
    %out_var = OpVariable %out_ptr_v4f Output

%combined_var = OpVariable %_ptr_UniformConstant_11 UniformConstant
       %main = OpFunction %void None %voidfn
       ;CHECK:  %main_0 = OpLabel
       ;CHECK: OpLoad

     %main_0 = OpLabel
   %combined = OpLoad %11 %combined_var
               OpReturn
               OpFunctionEnd
)";
  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_P(SplitCombinedImageSamplerPassEntryPointRemapTest,
       EntryPoint_Combined_UsedOnlyInEntryPointInstruction) {
  // If the combined var is in the interface, that is enough to trigger
  // its replacement. Otherwise the entry point interface is untouched
  // when the combined var is not otherwise used.
  const bool combined_var_in_interface =
      std::string(GetParam().initial_interface).find("%combined_var") !=
      std::string::npos;
  if (combined_var_in_interface) {
    const std::string kTest = PreambleFragment(GetParam().initial_interface) +
                              R"(
                 OpName %combined_var "combined_var"
                 OpName %in_var "in_var"
                 OpName %out_var "out_var"
                 OpDecorate %combined_var DescriptorSet 0
                 OpDecorate %combined_var Binding 0
                 OpDecorate %in_var BuiltIn FragCoord
                 OpDecorate %out_var Location 0

  ; CHECK: OpEntryPoint Fragment %main "main")" +
                              GetParam().expected_interface + R"(
  ; These clauses ensure the expected interface is the whole interface.
  ; CHECK-NOT: %{{\d+}}
  ; CHECK-NOT: %in_var
  ; CHECK-NOT: %out_var
  ; CHECK-NOT: %combined_var
  ; CHECK: OpExecutionMode %main OriginUpperLeft

                 %bool = OpTypeBool
  )" + BasicTypes() + R"(         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
           %11 = OpTypeSampledImage %10
  %_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
       %in_ptr_v4f = OpTypePointer Input %v4float
       %in_var = OpVariable %in_ptr_v4f Input
      %out_ptr_v4f = OpTypePointer Output %v4float
      %out_var = OpVariable %out_ptr_v4f Output

  ; %combined_var is not used!
  %combined_var = OpVariable %_ptr_UniformConstant_11 UniformConstant
         %main = OpFunction %void None %voidfn
       %main_0 = OpLabel
                 OpReturn
                 OpFunctionEnd
  )";
    auto [disasm, status] =
        SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
            kTest, /* do_validation= */ true);
    EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
  }
}

TEST_P(SplitCombinedImageSamplerPassEntryPointRemapTest,
       EntryPoint_Combined_Unused) {
  // If the combined var is in the interface, that is enough to trigger
  // its replacement. Otherwise the entry point interface is untouched
  // when the combined var is not otherwise used.
  const bool combined_var_in_interface =
      std::string(GetParam().initial_interface).find("%combined_var") !=
      std::string::npos;
  if (!combined_var_in_interface) {
    const std::string kTest = PreambleFragment(GetParam().initial_interface) +
                              R"(
  ; CHECK: OpEntryPoint Fragment %main "main")" +
                              GetParam().initial_interface  // Note this is the
                                                            // intial interface
                              + R"(
  ; These clauses ensure the expected interface is the whole interface.
  ; CHECK-NOT: %{{\d+}}
  ; CHECK-NOT: %in_var
  ; CHECK-NOT: %out_var
  ; CHECK-NOT: %combined_var
  ; CHECK: OpExecutionMode %main OriginUpperLeft

  ; The variable disappears.
  ; CHECK-NOT: %combined_var =
  ; CHECK: OpFunctionEnd
                 OpName %combined_var "combined_var"
                 OpName %in_var "in_var"
                 OpName %out_var "out_var"
                 OpDecorate %combined_var DescriptorSet 0
                 OpDecorate %combined_var Binding 0
                 OpDecorate %in_var BuiltIn FragCoord
                 OpDecorate %out_var Location 0


                 %bool = OpTypeBool
  )" + BasicTypes() + R"(         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
           %11 = OpTypeSampledImage %10
  %_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
       %in_ptr_v4f = OpTypePointer Input %v4float
       %in_var = OpVariable %in_ptr_v4f Input
      %out_ptr_v4f = OpTypePointer Output %v4float
      %out_var = OpVariable %out_ptr_v4f Output

  ; %combined_var is not used!
  %combined_var = OpVariable %_ptr_UniformConstant_11 UniformConstant
         %main = OpFunction %void None %voidfn
       %main_0 = OpLabel
                 OpReturn
                 OpFunctionEnd
)";
    auto [disasm, status] =
        SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
            kTest, /* do_validation= */ true);
    EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
  }
}

INSTANTIATE_TEST_SUITE_P(EntryPointRemap,
                         SplitCombinedImageSamplerPassEntryPointRemapTest,
                         ::testing::ValuesIn(EntryPointInterfaceCases()));

// Remap function types

struct FunctionTypeCase {
  const char* initial_type_params = "";
  const char* expected_type_params = "";
};

std::ostream& operator<<(std::ostream& os, const FunctionTypeCase& ftc) {
  os << "(init " << ftc.initial_type_params << " -> expect "
     << ftc.expected_type_params << ")";
  return os;
}

struct SplitCombinedImageSamplerPassFunctionTypeTest
    : public PassTest<::testing::TestWithParam<FunctionTypeCase>> {
  virtual void SetUp() override {
    SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
    SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES |
                          SPV_BINARY_TO_TEXT_OPTION_INDENT |
                          SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  }
};

std::vector<FunctionTypeCase> FunctionTypeCases() {
  return std::vector<FunctionTypeCase>{
      {"", ""},
      {" %image_ty", " %image_ty"},
      {" %sampler_ty", " %sampler_ty"},
      {" %sampled_image_ty", " %image_ty %sampler_ty"},
      {" %uint %sampled_image_ty %float",
       " %uint %image_ty %sampler_ty %float"},
      {" %ptr_sampled_image_ty",
       " %_ptr_UniformConstant_image_ty %_ptr_UniformConstant_sampler_ty"},
      {" %uint %ptr_sampled_image_ty %float",
       " %uint %_ptr_UniformConstant_image_ty %_ptr_UniformConstant_sampler_ty "
       "%float"},
      {" %uint %ptr_sampled_image_ty %ptr_sampled_image_ty %float",
       " %uint %_ptr_UniformConstant_image_ty %_ptr_UniformConstant_sampler_ty "
       "%_ptr_UniformConstant_image_ty %_ptr_UniformConstant_sampler_ty "
       "%float"},
  };
}

TEST_P(SplitCombinedImageSamplerPassFunctionTypeTest,
       ReplaceCombinedImageSamplersOnly) {
  const std::string kTest = Preamble() + +R"(
       OpName %f_ty "f_ty"
       OpName %sampler_ty "sampler_ty"
       OpName %image_ty "image_ty"
       OpName %sampled_image_ty "sampled_image_ty"
       OpName %ptr_sampled_image_ty "sampled_image_ty"

  )" + BasicTypes() + R"(

 %sampler_ty = OpTypeSampler
   %image_ty = OpTypeImage %float 2D 0 0 0 1 Unknown
 %sampled_image_ty = OpTypeSampledImage %image_ty
 %ptr_sampled_image_ty = OpTypePointer UniformConstant %sampled_image_ty

       %f_ty = OpTypeFunction %float)" +
                            GetParam().initial_type_params + R"(

  ; CHECK: %f_ty = OpTypeFunction %float)" +
                            GetParam().expected_type_params + R"(
)" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_P(SplitCombinedImageSamplerPassFunctionTypeTest, AvoidDuplicateType) {
  // SPIR-V does not allow duplicate non-aggregate types. That includes function
  // types.  Test that when replacing function type parameters would cause a
  // collision, that the original function type is replaced with the new one.
  const std::string initial_params(GetParam().initial_type_params);
  const std::string expected_params(GetParam().expected_type_params);
  const std::string kTest = Preamble() + +R"(
       OpName %sampler_ty "sampler_ty"
       OpName %image_ty "image_ty"
       OpName %sampled_image_ty "sampled_image_ty"
       OpName %_ptr_UniformConstant_sampler_ty "_ptr_UniformConstant_sampler_ty"
       OpName %_ptr_UniformConstant_image_ty "_ptr_UniformConstant_image_ty"
       OpName %ptr_sampled_image_ty "sampled_image_ty"
       OpName %dest_ty "dest_ty"

  )" + BasicTypes() + R"(

 %sampler_ty = OpTypeSampler
   %image_ty = OpTypeImage %float 2D 0 0 0 1 Unknown
 %sampled_image_ty = OpTypeSampledImage %image_ty
 %ptr_sampled_image_ty = OpTypePointer UniformConstant %sampled_image_ty
 %_ptr_UniformConstant_image_ty = OpTypePointer UniformConstant %image_ty
 %_ptr_UniformConstant_sampler_ty = OpTypePointer UniformConstant %sampler_ty

        %100 = OpTypeFunction %float)" +
                            initial_params + R"(
    %dest_ty = OpTypeFunction %float)" +
                            expected_params + R"(

  ; CHECK: OpTypeSampler
  ; CHECK-NOT: %100 =
  ; CHECK: %dest_ty = OpTypeFunction %float)" +
                            expected_params + R"(
  ; CHECK-NOT: %100 =
  ; CHECK: %main = OpFunction
)" + Main();
  // The original source is invalid if initial and expected params are the same,
  // because the type is already duplicated.
  // Only test when they are different.
  if (initial_params != expected_params) {
    auto [disasm, status] =
        SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
            kTest, /* do_validation= */ true);
    EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
  }
}

INSTANTIATE_TEST_SUITE_P(FunctionTypeRemap,
                         SplitCombinedImageSamplerPassFunctionTypeTest,
                         ::testing::ValuesIn(FunctionTypeCases()));

// Test array and runtime-array cases for function type replacement.

TEST_F(SplitCombinedImageSamplerPassTest, FunctionType_ReplaceSampledImageArg) {
  // The original module has a sampled image type, used only as a function
  // parameter.  We still want to replace it.  But no other sampled-image types
  // exist. This proves that the pass needs a sampled_image_used_as_param_
  // state variable.
  const std::string kTest = Preamble() + +R"(
       OpName %f_ty "f_ty"
       OpName %sampler_ty "sampler_ty"
       OpName %image_ty "image_ty"
       OpName %sampled_image_ty "sampled_image_ty"

  )" + BasicTypes() + R"(

 %sampler_ty = OpTypeSampler
   %image_ty = OpTypeImage %float 2D 0 0 0 1 Unknown
 %sampled_image_ty = OpTypeSampledImage %image_ty

       %f_ty = OpTypeFunction %float %sampled_image_ty %float
  ; CHECK: %f_ty = OpTypeFunction %float %image_ty %sampler_ty %float
)" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest, FunctionType_ReplaceArrayArg) {
  const std::string kTest = Preamble() + +R"(
       OpName %f_ty "f_ty"
       OpName %sampler_ty "sampler_ty"
       OpName %image_ty "image_ty"
       OpName %sampled_image_ty "sampled_image_ty"
       OpName %ptr_array_si_ty "ptr_array_si_ty"

  )" + BasicTypes() + R"(

 %sampler_ty = OpTypeSampler
   %image_ty = OpTypeImage %float 2D 0 0 0 1 Unknown
 %sampled_image_ty = OpTypeSampledImage %image_ty
   %array_si_ty = OpTypeArray %sampled_image_ty %uint_3
 %ptr_array_si_ty = OpTypePointer UniformConstant %array_si_ty

  ; CHECK: %[[array_i_ty:\w+]] = OpTypeArray %image_ty %uint_3
  ; CHECK: %[[ptr_array_i_ty:\w+]] = OpTypePointer UniformConstant %[[array_i_ty]]
  ; CHECK: %[[array_s_ty:\w+]] = OpTypeArray %sampler_ty %uint_3
  ; CHECK: %[[ptr_array_s_ty:\w+]] = OpTypePointer UniformConstant %[[array_s_ty]]

       %f_ty = OpTypeFunction %float %uint %ptr_array_si_ty %float
  ; CHECK: %f_ty = OpTypeFunction %float %uint %[[ptr_array_i_ty]] %[[ptr_array_s_ty]] %float
)" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest, FunctionType_ReplaceRtArrayArg) {
  const std::string kTest = Preamble() + +R"(
       OpName %f_ty "f_ty"
       OpName %sampler_ty "sampler_ty"
       OpName %image_ty "image_ty"
       OpName %sampled_image_ty "sampled_image_ty"
       OpName %ptr_array_si_ty "ptr_array_si_ty"

  )" + BasicTypes() + R"(

 %sampler_ty = OpTypeSampler
   %image_ty = OpTypeImage %float 2D 0 0 0 1 Unknown
 %sampled_image_ty = OpTypeSampledImage %image_ty
   %array_si_ty = OpTypeRuntimeArray %sampled_image_ty
 %ptr_array_si_ty = OpTypePointer UniformConstant %array_si_ty

  ; CHECK: %[[array_i_ty:\w+]] = OpTypeRuntimeArray %image_ty
  ; CHECK: %[[ptr_array_i_ty:\w+]] = OpTypePointer UniformConstant %[[array_i_ty]]
  ; CHECK: %[[array_s_ty:\w+]] = OpTypeRuntimeArray %sampler_ty
  ; CHECK: %[[ptr_array_s_ty:\w+]] = OpTypePointer UniformConstant %[[array_s_ty]]

       %f_ty = OpTypeFunction %float %uint %ptr_array_si_ty %float
  ; CHECK: %f_ty = OpTypeFunction %float %uint %[[ptr_array_i_ty]] %[[ptr_array_s_ty]] %float
)" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

// Remap function bodies

std::string NamedITypes() {
  return R"(
      OpName %f "f"
      OpName %f_ty "f_ty"
      OpName %i_ty "i_ty"
      OpName %s_ty "s_ty"
      OpName %p_i_ty "p_i_ty"
      OpName %p_s_ty "p_s_ty"
)";
}

std::string NamedCombinedTypes() {
  return R"(
      OpName %si_ty "si_ty"
      OpName %p_si_ty "p_si_ty"
      OpName %array_si_ty "array_si_ty"
      OpName %rtarray_si_ty "rtarray_si_ty"
      OpName %p_array_si_ty "p_array_si_ty"
      OpName %p_rtarray_si_ty "p_rtarray_si_ty"
)";
}

std::string NamedCaller() {
  return R"(
      OpName %caller_ty     "caller_ty"
      OpName %caller        "caller"
      OpName %caller_entry  "caller_entry"
      OpName %caller_call   "caller_call"
      OpName %caller_arg    "caller_arg"
)";
}

std::string ITypes() {
  return R"(
      %i_ty = OpTypeImage %float 2D 0 0 0 1 Unknown
      %s_ty = OpTypeSampler
      %p_i_ty = OpTypePointer UniformConstant %i_ty
      %p_s_ty = OpTypePointer UniformConstant %s_ty
)";
}

std::string CombinedTypes() {
  return R"(
      %si_ty = OpTypeSampledImage %i_ty
      %p_si_ty = OpTypePointer UniformConstant %si_ty
      %array_si_ty = OpTypeArray %si_ty %uint_3
      %p_array_si_ty = OpTypePointer UniformConstant %array_si_ty
      %rtarray_si_ty = OpTypeRuntimeArray %si_ty
      %p_rtarray_si_ty = OpTypePointer UniformConstant %rtarray_si_ty
)";
}

TEST_F(SplitCombinedImageSamplerPassTest, FunctionBody_ScalarNoChange) {
  const std::string kTest = Preamble() + NamedITypes() + NamedCombinedTypes() +
                            BasicTypes() + ITypes() + CombinedTypes() + R"(

      ; CHECK: %f_ty = OpTypeFunction %float %i_ty %s_ty %p_i_ty %p_s_ty
      %f_ty = OpTypeFunction %float %i_ty %s_ty %p_i_ty %p_s_ty

      ; CHECK: %f = OpFunction %float None %f_ty
      ; CHECK-NEXT: OpFunctionParameter %i_ty
      ; CHECK-NEXT: OpFunctionParameter %s_ty
      ; CHECK-NEXT: OpFunctionParameter %p_i_ty
      ; CHECK-NEXT: OpFunctionParameter %p_s_ty
      ; CHECK-NEXT: OpLabel
      %f = OpFunction %float None %f_ty
      %100 = OpFunctionParameter %i_ty
      %101 = OpFunctionParameter %s_ty
      %102 = OpFunctionParameter %p_i_ty
      %103 = OpFunctionParameter %p_s_ty
      %110 = OpLabel
      OpReturnValue %float_0
      OpFunctionEnd
      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest,
       FunctionBody_SampledImage_OpImageSample) {
  const std::string kTest = Preamble() + NamedITypes() + NamedCombinedTypes() +
                            BasicTypes() + ITypes() + CombinedTypes() + R"(

      ; CHECK: %f_ty = OpTypeFunction %v4float %uint %i_ty %s_ty %float
      %f_ty = OpTypeFunction %v4float %uint %si_ty %float

      ; CHECK: %f = OpFunction %v4float None %f_ty
      ; CHECK: OpFunctionParameter %uint
      ; CHECK-NEXT: %[[i:\w+]] = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %[[s:\w+]] = OpFunctionParameter %s_ty
      ; CHECK-NEXT: OpFunctionParameter %float
      ; CHECK-NEXT: OpLabel
      ; CHECK-NEXT: %[[si:\w+]] = OpSampledImage %si_ty %[[i]] %[[s]]
      ; CHECK-NEXT: %200 = OpImageSampleExplicitLod %v4float %[[si]] %13 Lod %float_0
      ; CHECK-NEXT: OpReturnValue %200

      %f = OpFunction %v4float None %f_ty
      %100 = OpFunctionParameter %uint
      %101 = OpFunctionParameter %si_ty ; replace this
      %110 = OpFunctionParameter %float
      %120 = OpLabel
      %200 = OpImageSampleExplicitLod %v4float %101 %13 Lod %float_0
      OpReturnValue %200
      OpFunctionEnd

      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest, FunctionBody_SampledImage_OpImage) {
  const std::string kTest = Preamble() + NamedITypes() + NamedCombinedTypes() +
                            BasicTypes() + ITypes() + CombinedTypes() + R"(

      ; CHECK: %f_ty = OpTypeFunction %void %uint %i_ty %s_ty %float
      %f_ty = OpTypeFunction %void %uint %si_ty %float

      ; CHECK: %f = OpFunction %void None %f_ty
      ; CHECK: OpFunctionParameter %uint
      ; CHECK-NEXT: %[[i:\w+]] = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %[[s:\w+]] = OpFunctionParameter %s_ty
      ; CHECK-NEXT: OpFunctionParameter %float
      ; CHECK-NEXT: OpLabel
      ; CHECK-NEXT: %[[si:\w+]] = OpSampledImage %si_ty %[[i]] %[[s]]
      ; CHECK-NEXT: %200 = OpImage %i_ty %[[si]]
      ; CHECK-NEXT: OpReturn

      %f = OpFunction %void None %f_ty
      %100 = OpFunctionParameter %uint
      %101 = OpFunctionParameter %si_ty ; replace this
      %110 = OpFunctionParameter %float
      %120 = OpLabel
      %200 = OpImage %i_ty %101
      OpReturn
      OpFunctionEnd

      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest, FunctionBody_PtrSampledImage) {
  const std::string kTest = Preamble() + NamedITypes() + NamedCombinedTypes() +
                            BasicTypes() + ITypes() + CombinedTypes() + R"(

      ; CHECK: %f_ty = OpTypeFunction %v4float %uint %p_i_ty %p_s_ty %float
      %f_ty = OpTypeFunction %v4float %uint %p_si_ty %float

      ; CHECK: %f = OpFunction %v4float None %f_ty
      ; CHECK-NEXT: OpFunctionParameter %uint
      ; CHECK-NEXT: %[[pi:\w+]] = OpFunctionParameter %p_i_ty
      ; CHECK-NEXT: %[[ps:\w+]] = OpFunctionParameter %p_s_ty
      ; CHECK-NEXT: OpFunctionParameter %float
      ; CHECK-NEXT: OpLabel
      ; CHECK-NEXT: %[[i:\w+]] = OpLoad %i_ty %[[pi]]
      ; CHECK-NEXT: %[[s:\w+]] = OpLoad %s_ty %[[ps]]
      ; CHECK-NEXT: %[[si:\w+]] = OpSampledImage %si_ty %[[i]] %[[s]]
      ; CHECK-NEXT: %200 = OpImageSampleExplicitLod %v4float %[[si]] %13 Lod %float_0
      ; CHECK-NEXT: OpReturnValue %200

      %f = OpFunction %v4float None %f_ty
      %100 = OpFunctionParameter %uint
      %101 = OpFunctionParameter %p_si_ty ; replace this
      %110 = OpFunctionParameter %float
      %120 = OpLabel
      %121 = OpLoad %si_ty %101
      %200 = OpImageSampleExplicitLod %v4float %121 %13 Lod %float_0
      OpReturnValue %200
      OpFunctionEnd
      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest,
       FunctionCall_NoImageOrSampler_NoChange) {
  const std::string kTest = Preamble() + NamedITypes() + NamedCaller() +
                            BasicTypes() + ITypes() + CombinedTypes() + R"(

      ; CHECK: %f_ty = OpTypeFunction %void %uint %float
      %f_ty = OpTypeFunction %void %uint %float
      %caller_ty = OpTypeFunction %float  ; make it return non-void otherwise it's just like main

      ; The called function does not change
      ; CHECK: %f = OpFunction %void None %f_ty
      ; CHECK-NEXT: = OpFunctionParameter %uint
      ; CHECK-NEXT: = OpFunctionParameter %float
      ; CHECK-NEXT: = OpLabel
      ; CHECK-NEXT: OpReturn
      ; CHECK-NEXT: OpFunctionEnd

      %f = OpFunction %void None %f_ty
      %100 = OpFunctionParameter %uint
      %101 = OpFunctionParameter %float
      %110 = OpLabel
      OpReturn
      OpFunctionEnd

      ; The caller does not change
      ; CHECK: %caller = OpFunction %float None %caller_ty
      ; CHECK-NEXT: %caller_entry = OpLabel
      ; CHECK-NEXT: %caller_arg = OpCopyObject %uint %uint_0
      ; CHECK-NEXT: OpFunctionCall %void %f %caller_arg %float_0
      ; CHECK-NEXT: OpReturnValue %float_0
      ; CHECK-NEXT: OpFunctionEnd

      %caller = OpFunction %float None %caller_ty
%caller_entry = OpLabel
  %caller_arg = OpCopyObject %uint %uint_0
 %caller_call = OpFunctionCall %void %f %caller_arg %float_0
                OpReturnValue %float_0
                OpFunctionEnd

      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  // We still get a success-with-change result because the boilerplate included
  // combined types, which were removed.
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest, FunctionCall_Image_NoChange) {
  const std::string kTest = Preamble() + NamedITypes() + NamedCaller() +
                            BasicTypes() + ITypes() + R"(

      ; CHECK: %f_ty = OpTypeFunction %void %i_ty
      %f_ty = OpTypeFunction %void %i_ty
      %caller_ty = OpTypeFunction %float %i_ty

      ; The called function does not change
      ; CHECK: %f = OpFunction %void None %f_ty
      ; CHECK-NEXT: = OpFunctionParameter %i_ty
      ; CHECK-NEXT: = OpLabel
      ; CHECK-NEXT: OpReturn
      ; CHECK-NEXT: OpFunctionEnd

      %f = OpFunction %void None %f_ty
      %100 = OpFunctionParameter %i_ty
      %110 = OpLabel
      OpReturn
      OpFunctionEnd

      ; The caller does not change
      ; CHECK: %caller = OpFunction %float None %caller_ty
      ; CHECK-NEXT: %caller_arg = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %caller_entry = OpLabel
      ; CHECK-NEXT: OpFunctionCall %void %f %caller_arg
      ; CHECK-NEXT: OpReturnValue %float_0
      ; CHECK-NEXT: OpFunctionEnd

      %caller = OpFunction %float None %caller_ty
  %caller_arg = OpFunctionParameter %i_ty
%caller_entry = OpLabel
 %caller_call = OpFunctionCall %void %f %caller_arg
                OpReturnValue %float_0
                OpFunctionEnd

      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithoutChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest, FunctionCall_Sampler_NoChange) {
  const std::string kTest = Preamble() + NamedITypes() + NamedCaller() +
                            BasicTypes() + ITypes() + R"(

      ; CHECK: %f_ty = OpTypeFunction %void %s_ty
      %f_ty = OpTypeFunction %void %s_ty
      %caller_ty = OpTypeFunction %float %s_ty

      ; The called function does not change
      ; CHECK: %f = OpFunction %void None %f_ty
      ; CHECK-NEXT: = OpFunctionParameter %s_ty
      ; CHECK-NEXT: = OpLabel
      ; CHECK-NEXT: OpReturn
      ; CHECK-NEXT: OpFunctionEnd

      %f = OpFunction %void None %f_ty
      %100 = OpFunctionParameter %s_ty
      %110 = OpLabel
      OpReturn
      OpFunctionEnd

      ; The caller does not change
      ; CHECK: %caller = OpFunction %float None %caller_ty
      ; CHECK-NEXT: %caller_arg = OpFunctionParameter %s_ty
      ; CHECK-NEXT: %caller_entry = OpLabel
      ; CHECK-NEXT: OpFunctionCall %void %f %caller_arg
      ; CHECK-NEXT: OpReturnValue %float_0
      ; CHECK-NEXT: OpFunctionEnd

      %caller = OpFunction %float None %caller_ty
  %caller_arg = OpFunctionParameter %s_ty
%caller_entry = OpLabel
 %caller_call = OpFunctionCall %void %f %caller_arg
                OpReturnValue %float_0
                OpFunctionEnd

      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithoutChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest, FunctionCall_PtrImage_NoChange) {
  const std::string kTest = Preamble() + NamedITypes() + NamedCaller() +
                            BasicTypes() + ITypes() + R"(

      ; CHECK: %f_ty = OpTypeFunction %void %p_i_ty
      %f_ty = OpTypeFunction %void %p_i_ty
      %caller_ty = OpTypeFunction %float %p_i_ty

      ; The called function does not change
      ; CHECK: %f = OpFunction %void None %f_ty
      ; CHECK-NEXT: = OpFunctionParameter %p_i_ty
      ; CHECK-NEXT: = OpLabel
      ; CHECK-NEXT: OpReturn
      ; CHECK-NEXT: OpFunctionEnd

      %f = OpFunction %void None %f_ty
      %100 = OpFunctionParameter %p_i_ty
      %110 = OpLabel
      OpReturn
      OpFunctionEnd

      ; The caller does not change
      ; CHECK: %caller = OpFunction %float None %caller_ty
      ; CHECK-NEXT: %caller_arg = OpFunctionParameter %p_i_ty
      ; CHECK-NEXT: %caller_entry = OpLabel
      ; CHECK-NEXT: OpFunctionCall %void %f %caller_arg
      ; CHECK-NEXT: OpReturnValue %float_0
      ; CHECK-NEXT: OpFunctionEnd

      %caller = OpFunction %float None %caller_ty
  %caller_arg = OpFunctionParameter %p_i_ty
%caller_entry = OpLabel
 %caller_call = OpFunctionCall %void %f %caller_arg
                OpReturnValue %float_0
                OpFunctionEnd

      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithoutChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest, FunctionCall_PtrSampler_NoChange) {
  const std::string kTest = Preamble() + NamedITypes() + NamedCaller() +
                            BasicTypes() + ITypes() + R"(

      ; CHECK: %f_ty = OpTypeFunction %void %p_s_ty
      %f_ty = OpTypeFunction %void %p_s_ty
      %caller_ty = OpTypeFunction %float %p_s_ty

      ; The called function does not change
      ; CHECK: %f = OpFunction %void None %f_ty
      ; CHECK-NEXT: = OpFunctionParameter %p_s_ty
      ; CHECK-NEXT: = OpLabel
      ; CHECK-NEXT: OpReturn
      ; CHECK-NEXT: OpFunctionEnd

      %f = OpFunction %void None %f_ty
      %100 = OpFunctionParameter %p_s_ty
      %110 = OpLabel
      OpReturn
      OpFunctionEnd

      ; The caller does not change
      ; CHECK: %caller = OpFunction %float None %caller_ty
      ; CHECK-NEXT: %caller_arg = OpFunctionParameter %p_s_ty
      ; CHECK-NEXT: %caller_entry = OpLabel
      ; CHECK-NEXT: OpFunctionCall %void %f %caller_arg
      ; CHECK-NEXT: OpReturnValue %float_0
      ; CHECK-NEXT: OpFunctionEnd

      %caller = OpFunction %float None %caller_ty
  %caller_arg = OpFunctionParameter %p_s_ty
%caller_entry = OpLabel
 %caller_call = OpFunctionCall %void %f %caller_arg
                OpReturnValue %float_0
                OpFunctionEnd

      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithoutChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest, FunctionCall_SampledImage_Split) {
  const std::string kTest = Preamble() + NamedITypes() + NamedCombinedTypes() +
                            NamedCaller() + BasicTypes() + ITypes() +
                            CombinedTypes() + R"(

      ; CHECK: %f_ty = OpTypeFunction %void %i_ty %s_ty
      %f_ty = OpTypeFunction %void %si_ty
      %caller_ty = OpTypeFunction %float %si_ty

      ; Call function arg is split. We've checked these details in other tests.
      ; CHECK: %f = OpFunction %void None %f_ty
      ; CHECK-NEXT: %[[callee_i:\w+]] = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %[[callee_s:\w+]] = OpFunctionParameter %s_ty
      ; CHECK-NEXT: = OpLabel
      ; CHECK-NEXT: OpReturn
      ; CHECK-NEXT: OpFunctionEnd

      %f = OpFunction %void None %f_ty
      %100 = OpFunctionParameter %si_ty
      %110 = OpLabel
      OpReturn
      OpFunctionEnd

      ; CHECK: %caller = OpFunction %float None %caller_ty
      ; CHECK-NEXT: %[[caller_i:\w+]] = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %[[caller_s:\w+]] = OpFunctionParameter %s_ty
      ; CHECK-NEXT: %caller_entry = OpLabel
      ; CHECK-NEXT: %caller_call = OpFunctionCall %void %f %[[caller_i]] %[[caller_s]]
      ; CHECK-NEXT: OpReturnValue %float_0
      ; CHECK-NEXT: OpFunctionEnd

      %caller = OpFunction %float None %caller_ty
  %caller_arg = OpFunctionParameter %si_ty
%caller_entry = OpLabel
 %caller_call = OpFunctionCall %void %f %caller_arg
                OpReturnValue %float_0
                OpFunctionEnd

      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest,
       FunctionCall_SampledImageDuplicatedArg_Split) {
  const std::string kTest = Preamble() + NamedITypes() + NamedCombinedTypes() +
                            NamedCaller() + BasicTypes() + ITypes() +
                            CombinedTypes() + R"(

      ; CHECK: %f_ty = OpTypeFunction %void %i_ty %s_ty %i_ty %s_ty
      %f_ty = OpTypeFunction %void %si_ty %si_ty
      %caller_ty = OpTypeFunction %float %si_ty

      ; Call function arg is split. We've checked these details in other tests.
      ; CHECK: %f = OpFunction %void None %f_ty
      ; CHECK-NEXT: %[[callee_i_0:\w+]] = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %[[callee_s_0:\w+]] = OpFunctionParameter %s_ty
      ; CHECK-NEXT: %[[callee_i_1:\w+]] = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %[[callee_s_1:\w+]] = OpFunctionParameter %s_ty
      ; CHECK-NEXT: = OpLabel
      ; CHECK-NEXT: OpReturn
      ; CHECK-NEXT: OpFunctionEnd

      %f = OpFunction %void None %f_ty
      %100 = OpFunctionParameter %si_ty
      %101 = OpFunctionParameter %si_ty
      %110 = OpLabel
      OpReturn
      OpFunctionEnd

      ; CHECK: %caller = OpFunction %float None %caller_ty
      ; CHECK-NEXT: %[[caller_i:\w+]] = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %[[caller_s:\w+]] = OpFunctionParameter %s_ty
      ; CHECK-NEXT: %caller_entry = OpLabel
      ; CHECK-NEXT: %caller_call = OpFunctionCall %void %f %[[caller_i]] %[[caller_s]] %[[caller_i]] %[[caller_s]]
      ; CHECK-NEXT: OpReturnValue %float_0
      ; CHECK-NEXT: OpFunctionEnd

      %caller = OpFunction %float None %caller_ty
  %caller_arg = OpFunctionParameter %si_ty
%caller_entry = OpLabel
 %caller_call = OpFunctionCall %void %f %caller_arg %caller_arg
                OpReturnValue %float_0
                OpFunctionEnd

      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest,
       FunctionCall_SampledImageTwoDistinct_Split) {
  const std::string kTest = Preamble() + NamedITypes() + NamedCombinedTypes() +
                            NamedCaller() + BasicTypes() + ITypes() +
                            CombinedTypes() + R"(

      ; CHECK: %f_ty = OpTypeFunction %void %i_ty %s_ty %i_ty %s_ty
      %f_ty = OpTypeFunction %void %si_ty %si_ty
      %caller_ty = OpTypeFunction %float %si_ty %si_ty

      ; Call function arg is split. We've checked these details in other tests.
      ; CHECK: %f = OpFunction %void None %f_ty
      ; CHECK-NEXT: %[[callee_i_0:\w+]] = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %[[callee_s_0:\w+]] = OpFunctionParameter %s_ty
      ; CHECK-NEXT: %[[callee_i_1:\w+]] = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %[[callee_s_1:\w+]] = OpFunctionParameter %s_ty
      ; CHECK-NEXT: = OpLabel
      ; CHECK-NEXT: OpReturn
      ; CHECK-NEXT: OpFunctionEnd

      %f = OpFunction %void None %f_ty
      %100 = OpFunctionParameter %si_ty
      %101 = OpFunctionParameter %si_ty
      %110 = OpLabel
      OpReturn
      OpFunctionEnd

      ; CHECK: %caller = OpFunction %float None %caller_ty
      ; CHECK-NEXT: %[[caller_i_0:\w+]] = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %[[caller_s_0:\w+]] = OpFunctionParameter %s_ty
      ; CHECK-NEXT: %[[caller_i_1:\w+]] = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %[[caller_s_1:\w+]] = OpFunctionParameter %s_ty
      ; CHECK-NEXT: %caller_entry = OpLabel
      ; CHECK-NEXT: %caller_call = OpFunctionCall %void %f %[[caller_i_0]] %[[caller_s_0]] %[[caller_i_1]] %[[caller_s_1]]
      ; CHECK-NEXT: OpReturnValue %float_0
      ; CHECK-NEXT: OpFunctionEnd

      %caller = OpFunction %float None %caller_ty
  %caller_arg = OpFunctionParameter %si_ty
         %201 = OpFunctionParameter %si_ty
%caller_entry = OpLabel
 %caller_call = OpFunctionCall %void %f %caller_arg %201
                OpReturnValue %float_0
                OpFunctionEnd

      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest,
       FunctionCall_SampledImageAndCopy_Split) {
  const std::string kTest = Preamble() + NamedITypes() + NamedCombinedTypes() +
                            NamedCaller() + BasicTypes() + ITypes() +
                            CombinedTypes() + R"(

      ; CHECK: %f_ty = OpTypeFunction %void %i_ty %s_ty %i_ty %s_ty
      %f_ty = OpTypeFunction %void %si_ty %si_ty
      ; CHECK: %caller_ty = OpTypeFunction %float %i_ty %s_ty
      %caller_ty = OpTypeFunction %float %si_ty

      ; Call function arg is split. We've checked these details in other tests.
      ; CHECK: %f = OpFunction %void None %f_ty
      ; CHECK-NEXT: %[[callee_i_0:\w+]] = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %[[callee_s_0:\w+]] = OpFunctionParameter %s_ty
      ; CHECK-NEXT: %[[callee_i_1:\w+]] = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %[[callee_s_1:\w+]] = OpFunctionParameter %s_ty
      ; CHECK-NEXT: = OpLabel
      ; CHECK-NEXT: OpReturn
      ; CHECK-NEXT: OpFunctionEnd

      %f = OpFunction %void None %f_ty
      %100 = OpFunctionParameter %si_ty
      %101 = OpFunctionParameter %si_ty
      %110 = OpLabel
      OpReturn
      OpFunctionEnd

      ; CHECK: %caller = OpFunction %float None %caller_ty
      ; CHECK-NEXT: %[[caller_i:\w+]] = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %[[caller_s:\w+]] = OpFunctionParameter %s_ty
      ; CHECK-NEXT: %caller_entry = OpLabel
      ; CHECK-NEXT: %caller_call = OpFunctionCall %void %f %[[caller_i]] %[[caller_s]] %[[caller_i]] %[[caller_s]]
      ; CHECK-NEXT: OpReturnValue %float_0
      ; CHECK-NEXT: OpFunctionEnd

      %caller = OpFunction %float None %caller_ty
  %caller_arg = OpFunctionParameter %si_ty
%caller_entry = OpLabel
        %copy = OpCopyObject %si_ty %caller_arg
 %caller_call = OpFunctionCall %void %f %caller_arg %copy
                OpReturnValue %float_0
                OpFunctionEnd

      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest,
       FunctionCall_SampledImageSurrounded_Split) {
  // Test indexing by surrounding the sampled image parameter with other
  // arguments that should not be touched.
  const std::string kTest = Preamble() + NamedITypes() + NamedCombinedTypes() +
                            NamedCaller() + BasicTypes() + ITypes() +
                            CombinedTypes() + R"(

      ; CHECK: %f_ty = OpTypeFunction %void %float %i_ty %s_ty %uint
      %f_ty = OpTypeFunction %void %float %si_ty %uint
      ; CHECK: %caller_ty = OpTypeFunction %float %uint %i_ty %s_ty %float
      %caller_ty = OpTypeFunction %float %uint %si_ty %float

      ; Call function arg is split. We've checked these details in other tests.
      ; CHECK: %f = OpFunction %void None %f_ty
      ; CHECK-NEXT: %[[callee_f:\w+]] = OpFunctionParameter %float
      ; CHECK-NEXT: %[[callee_i:\w+]] = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %[[callee_s:\w+]] = OpFunctionParameter %s_ty
      ; CHECK-NEXT: %[[callee_u:\w+]] = OpFunctionParameter %uint
      ; CHECK-NEXT: = OpLabel
      ; CHECK-NEXT: OpReturn
      ; CHECK-NEXT: OpFunctionEnd

      %f = OpFunction %void None %f_ty
       %99 = OpFunctionParameter %float
      %100 = OpFunctionParameter %si_ty
      %101 = OpFunctionParameter %uint
      %110 = OpLabel
      OpReturn
      OpFunctionEnd

      ; CHECK: %caller = OpFunction %float None %caller_ty
      ; CHECK-NEXT: %[[u_param:\w+]] = OpFunctionParameter %uint
      ; CHECK-NEXT: %[[caller_i:\w+]] = OpFunctionParameter %i_ty
      ; CHECK-NEXT: %[[caller_s:\w+]] = OpFunctionParameter %s_ty
      ; CHECK-NEXT: %[[f_param:\w+]] = OpFunctionParameter %float
      ; CHECK-NEXT: %caller_entry = OpLabel
      ; CHECK-NEXT: %caller_call = OpFunctionCall %void %f %[[f_param]] %[[caller_i]] %[[caller_s]] %[[u_param]]
      ; CHECK-NEXT: OpReturnValue %float_0
      ; CHECK-NEXT: OpFunctionEnd

      %caller = OpFunction %float None %caller_ty
         %200 = OpFunctionParameter %uint
  %caller_arg = OpFunctionParameter %si_ty
         %201 = OpFunctionParameter %float
%caller_entry = OpLabel
 %caller_call = OpFunctionCall %void %f %201 %caller_arg %200
                OpReturnValue %float_0
                OpFunctionEnd

      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest, FunctionCall_PtrSampledImage_Split) {
  const std::string kTest = Preamble() + NamedITypes() + NamedCombinedTypes() +
                            NamedCaller() + BasicTypes() + ITypes() +
                            CombinedTypes() + R"(

      ; CHECK: %f_ty = OpTypeFunction %void %p_i_ty %p_s_ty
      %f_ty = OpTypeFunction %void %p_si_ty
      %caller_ty = OpTypeFunction %float %p_si_ty

      ; Call function arg is split. We've checked these details in other tests.
      ; CHECK: %f = OpFunction %void None %f_ty
      ; CHECK-NEXT: %[[callee_i:\w+]] = OpFunctionParameter %p_i_ty
      ; CHECK-NEXT: %[[callee_s:\w+]] = OpFunctionParameter %p_s_ty
      ; CHECK-NEXT: = OpLabel
      ; CHECK-NEXT: OpReturn
      ; CHECK-NEXT: OpFunctionEnd

      %f = OpFunction %void None %f_ty
      %100 = OpFunctionParameter %p_si_ty
      %110 = OpLabel
      OpReturn
      OpFunctionEnd

      ; CHECK: %caller = OpFunction %float None %caller_ty
      ; CHECK-NEXT: %[[caller_i:\w+]] = OpFunctionParameter %p_i_ty
      ; CHECK-NEXT: %[[caller_s:\w+]] = OpFunctionParameter %p_s_ty
      ; CHECK-NEXT: %caller_entry = OpLabel
      ; CHECK-NEXT: %caller_call = OpFunctionCall %void %f %[[caller_i]] %[[caller_s]]
      ; CHECK-NEXT: OpReturnValue %float_0
      ; CHECK-NEXT: OpFunctionEnd

      %caller = OpFunction %float None %caller_ty
  %caller_arg = OpFunctionParameter %p_si_ty
%caller_entry = OpLabel
 %caller_call = OpFunctionCall %void %f %caller_arg
                OpReturnValue %float_0
                OpFunctionEnd

      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest,
       FunctionCall_PtrArraySampledImage_Split) {
  const std::string kTest = Preamble() + NamedITypes() + NamedCombinedTypes() +
                            NamedCaller() + BasicTypes() + ITypes() +
                            CombinedTypes() + R"(

      ; CHECK: %[[array_i_ty:\w+]] = OpTypeArray %i_ty %uint_3
      ; CHECK: %[[p_array_i_ty:\w+]] = OpTypePointer UniformConstant %[[array_i_ty]]
      ; CHECK: %[[array_s_ty:\w+]] = OpTypeArray %s_ty %uint_3
      ; CHECK: %[[p_array_s_ty:\w+]] = OpTypePointer UniformConstant %[[array_s_ty]]

      ; CHECK: %f_ty = OpTypeFunction %void %[[p_array_i_ty]] %[[p_array_s_ty]]
      %f_ty = OpTypeFunction %void %p_array_si_ty
      %caller_ty = OpTypeFunction %float %p_array_si_ty

      ; Call function arg is split. We've checked these details in other tests.
      ; CHECK: %f = OpFunction %void None %f_ty
      ; CHECK-NEXT: %[[callee_i:\w+]] = OpFunctionParameter %[[p_array_i_ty]]
      ; CHECK-NEXT: %[[callee_s:\w+]] = OpFunctionParameter %[[p_array_s_ty]]
      ; CHECK-NEXT: = OpLabel
      ; CHECK-NEXT: OpReturn
      ; CHECK-NEXT: OpFunctionEnd

      %f = OpFunction %void None %f_ty
      %100 = OpFunctionParameter %p_array_si_ty
      %110 = OpLabel
      OpReturn
      OpFunctionEnd

      ; CHECK: %caller = OpFunction %float None %caller_ty
      ; CHECK-NEXT: %[[caller_i:\w+]] = OpFunctionParameter %[[p_array_i_ty]]
      ; CHECK-NEXT: %[[caller_s:\w+]] = OpFunctionParameter %[[p_array_s_ty]]
      ; CHECK-NEXT: %caller_entry = OpLabel
      ; CHECK-NEXT: %caller_call = OpFunctionCall %void %f %[[caller_i]] %[[caller_s]]
      ; CHECK-NEXT: OpReturnValue %float_0
      ; CHECK-NEXT: OpFunctionEnd

      %caller = OpFunction %float None %caller_ty
  %caller_arg = OpFunctionParameter %p_array_si_ty
%caller_entry = OpLabel
 %caller_call = OpFunctionCall %void %f %caller_arg
                OpReturnValue %float_0
                OpFunctionEnd

      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(SplitCombinedImageSamplerPassTest,
       FunctionCall_PtrRtArraySampledImage_Split) {
  const std::string kTest = Preamble() + NamedITypes() + NamedCombinedTypes() +
                            NamedCaller() + BasicTypes() + ITypes() +
                            CombinedTypes() + R"(

      ; CHECK: %[[array_i_ty:\w+]] = OpTypeRuntimeArray %i_ty
      ; CHECK: %[[p_array_i_ty:\w+]] = OpTypePointer UniformConstant %[[array_i_ty]]
      ; CHECK: %[[array_s_ty:\w+]] = OpTypeRuntimeArray
      ; CHECK: %[[p_array_s_ty:\w+]] = OpTypePointer UniformConstant %[[array_s_ty]]

      ; CHECK: %f_ty = OpTypeFunction %void %[[p_array_i_ty]] %[[p_array_s_ty]]
      %f_ty = OpTypeFunction %void %p_rtarray_si_ty
      %caller_ty = OpTypeFunction %float %p_rtarray_si_ty

      ; Call function arg is split. We've checked these details in other tests.
      ; CHECK: %f = OpFunction %void None %f_ty
      ; CHECK-NEXT: %[[callee_i:\w+]] = OpFunctionParameter %[[p_array_i_ty]]
      ; CHECK-NEXT: %[[callee_s:\w+]] = OpFunctionParameter %[[p_array_s_ty]]
      ; CHECK-NEXT: = OpLabel
      ; CHECK-NEXT: OpReturn
      ; CHECK-NEXT: OpFunctionEnd

      %f = OpFunction %void None %f_ty
      %100 = OpFunctionParameter %p_rtarray_si_ty
      %110 = OpLabel
      OpReturn
      OpFunctionEnd

      ; CHECK: %caller = OpFunction %float None %caller_ty
      ; CHECK-NEXT: %[[caller_i:\w+]] = OpFunctionParameter %[[p_array_i_ty]]
      ; CHECK-NEXT: %[[caller_s:\w+]] = OpFunctionParameter %[[p_array_s_ty]]
      ; CHECK-NEXT: %caller_entry = OpLabel
      ; CHECK-NEXT: %caller_call = OpFunctionCall %void %f %[[caller_i]] %[[caller_s]]
      ; CHECK-NEXT: OpReturnValue %float_0
      ; CHECK-NEXT: OpFunctionEnd

      %caller = OpFunction %float None %caller_ty
  %caller_arg = OpFunctionParameter %p_rtarray_si_ty
%caller_entry = OpLabel
 %caller_call = OpFunctionCall %void %f %caller_arg
                OpReturnValue %float_0
                OpFunctionEnd

      )" + Main();

  auto [disasm, status] = SinglePassRunAndMatch<SplitCombinedImageSamplerPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
