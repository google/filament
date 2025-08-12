// Copyright (c) 2021 Google LLC
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

#include <vector>

#include "gmock/gmock.h"
#include "source/opt/convert_to_sampled_image_pass.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using testing::Eq;
using VectorOfDescriptorSetAndBindingPairs =
    std::vector<DescriptorSetAndBinding>;

struct DescriptorSetAndBindingStringParsingTestCase {
  const char* descriptor_set_binding_str;
  bool expect_success;
  VectorOfDescriptorSetAndBindingPairs expected_descriptor_set_binding_pairs;
};

using DescriptorSetAndBindingStringParsingTest =
    ::testing::TestWithParam<DescriptorSetAndBindingStringParsingTestCase>;

TEST_P(DescriptorSetAndBindingStringParsingTest, TestCase) {
  const auto& tc = GetParam();
  auto actual_descriptor_set_binding_pairs =
      ConvertToSampledImagePass::ParseDescriptorSetBindingPairsString(
          tc.descriptor_set_binding_str);
  if (tc.expect_success) {
    EXPECT_NE(nullptr, actual_descriptor_set_binding_pairs);
    if (actual_descriptor_set_binding_pairs) {
      EXPECT_THAT(*actual_descriptor_set_binding_pairs,
                  Eq(tc.expected_descriptor_set_binding_pairs));
    }
  } else {
    EXPECT_EQ(nullptr, actual_descriptor_set_binding_pairs);
  }
}

INSTANTIATE_TEST_SUITE_P(
    ValidString, DescriptorSetAndBindingStringParsingTest,
    ::testing::ValuesIn(std::vector<
                        DescriptorSetAndBindingStringParsingTestCase>{
        // 0. empty vector
        {"", true, VectorOfDescriptorSetAndBindingPairs({})},
        // 1. one pair
        {"100:1024", true,
         VectorOfDescriptorSetAndBindingPairs({DescriptorSetAndBinding{100,
                                                                       1024}})},
        // 2. two pairs
        {"100:1024 200:2048", true,
         VectorOfDescriptorSetAndBindingPairs(
             {DescriptorSetAndBinding{100, 1024},
              DescriptorSetAndBinding{200, 2048}})},
        // 3. spaces between entries
        {"100:1024 \n \r \t \v \f 200:2048", true,
         VectorOfDescriptorSetAndBindingPairs(
             {DescriptorSetAndBinding{100, 1024},
              DescriptorSetAndBinding{200, 2048}})},
        // 4. \t, \n, \r and spaces before spec id
        {"   \n \r\t \t \v \f 100:1024", true,
         VectorOfDescriptorSetAndBindingPairs({DescriptorSetAndBinding{100,
                                                                       1024}})},
        // 5. \t, \n, \r and spaces after value string
        {"100:1024   \n \r\t \t \v \f ", true,
         VectorOfDescriptorSetAndBindingPairs({DescriptorSetAndBinding{100,
                                                                       1024}})},
        // 6. maximum spec id
        {"4294967295:0", true,
         VectorOfDescriptorSetAndBindingPairs({DescriptorSetAndBinding{
             4294967295, 0}})},
        // 7. minimum spec id
        {"0:100", true,
         VectorOfDescriptorSetAndBindingPairs({DescriptorSetAndBinding{0,
                                                                       100}})},
        // 8. multiple entries
        {"101:1 102:2 103:3 104:4 200:201 9999:1000", true,
         VectorOfDescriptorSetAndBindingPairs(
             {DescriptorSetAndBinding{101, 1}, DescriptorSetAndBinding{102, 2},
              DescriptorSetAndBinding{103, 3}, DescriptorSetAndBinding{104, 4},
              DescriptorSetAndBinding{200, 201},
              DescriptorSetAndBinding{9999, 1000}})},
    }));

INSTANTIATE_TEST_SUITE_P(
    InvalidString, DescriptorSetAndBindingStringParsingTest,
    ::testing::ValuesIn(
        std::vector<DescriptorSetAndBindingStringParsingTestCase>{
            // 0. missing default value
            {"100:", false, VectorOfDescriptorSetAndBindingPairs{}},
            // 1. descriptor set is not an integer
            {"100.0:200", false, VectorOfDescriptorSetAndBindingPairs{}},
            // 2. descriptor set is not a number
            {"something_not_a_number:1", false,
             VectorOfDescriptorSetAndBindingPairs{}},
            // 3. only descriptor set number
            {"100", false, VectorOfDescriptorSetAndBindingPairs{}},
            // 4. empty descriptor set
            {":3", false, VectorOfDescriptorSetAndBindingPairs{}},
            // 5. only colon
            {":", false, VectorOfDescriptorSetAndBindingPairs{}},
            // 6. descriptor set overflow
            {"4294967296:200", false, VectorOfDescriptorSetAndBindingPairs{}},
            // 7. descriptor set less than 0
            {"-1:200", false, VectorOfDescriptorSetAndBindingPairs{}},
            // 8. nullptr
            {nullptr, false, VectorOfDescriptorSetAndBindingPairs{}},
            // 9. only a number is invalid
            {"1234", false, VectorOfDescriptorSetAndBindingPairs{}},
            // 10. invalid entry separator
            {"12:34;23:14", false, VectorOfDescriptorSetAndBindingPairs{}},
            // 11. invalid descriptor set and default value separator
            {"12@34", false, VectorOfDescriptorSetAndBindingPairs{}},
            // 12. spaces before colon
            {"100   :1024", false, VectorOfDescriptorSetAndBindingPairs{}},
            // 13. spaces after colon
            {"100:   1024", false, VectorOfDescriptorSetAndBindingPairs{}},
            // 14. descriptor set represented in hex float format is invalid
            {"0x3p10:200", false, VectorOfDescriptorSetAndBindingPairs{}},
        }));

std::string BuildShader(const char* shader_decorate_instructions,
                        const char* shader_image_and_sampler_variables,
                        const char* shader_body) {
  // Base HLSL code:
  //
  // SamplerState sam : register(s2);
  // Texture2D <float4> texture : register(t5);
  //
  // float4 main() : SV_TARGET {
  //     return texture.SampleLevel(sam, float2(1, 2), 10, 2);
  // }
  std::stringstream ss;
  ss << R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %out_var_SV_TARGET
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpName %type_sampler "type.sampler"
               OpName %type_2d_image "type.2d.image"
               OpName %out_var_SV_TARGET "out.var.SV_TARGET"
               OpName %main "main"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorate %out_var_SV_TARGET Location 0
               )";
  ss << shader_decorate_instructions;
  ss << R"(
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %v2float = OpTypeVector %float 2
         %12 = OpConstantComposite %v2float %float_1 %float_2
   %float_10 = OpConstant %float 10
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
      %v2int = OpTypeVector %int 2
         %17 = OpConstantComposite %v2int %int_2 %int_2
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
%type_sampled_image = OpTypeSampledImage %type_2d_image
               )";
  ss << shader_image_and_sampler_variables;
  ss << R"(
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %23
         %24 = OpLabel
  )";
  ss << shader_body;
  ss << R"(
               OpReturn
               OpFunctionEnd
  )";
  return ss.str();
}

using ConvertToSampledImageTest = PassTest<::testing::Test>;

TEST_F(ConvertToSampledImageTest, Texture2DAndSamplerToSampledImage) {
  const std::string shader = BuildShader(
      R"(
               OpDecorate %sam DescriptorSet 0
               OpDecorate %sam Binding 5
               OpDecorate %texture DescriptorSet 0
               OpDecorate %texture Binding 5
               )",
      R"(
            ; CHECK-NOT: OpVariable %_ptr_UniformConstant_type_2d_image

            ; CHECK: [[tex:%\w+]] = OpVariable %_ptr_UniformConstant_type_sampled_image UniformConstant
        %sam = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
    %texture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
               )",
      R"(
            ; CHECK: [[load:%\w+]] = OpLoad %type_sampled_image [[tex]]
            ; CHECK: OpImageSampleExplicitLod %v4float [[load]]
         %25 = OpLoad %type_2d_image %texture
         %26 = OpLoad %type_sampler %sam
         %27 = OpSampledImage %type_sampled_image %25 %26
         %28 = OpImageSampleExplicitLod %v4float %27 %12 Lod|ConstOffset %float_10 %17
               OpStore %out_var_SV_TARGET %28
               )");

  auto result = SinglePassRunAndMatch<ConvertToSampledImagePass>(
      shader, /* do_validate = */ true,
      VectorOfDescriptorSetAndBindingPairs{DescriptorSetAndBinding{0, 5}});

  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(ConvertToSampledImageTest, Texture2DToSampledImage) {
  const std::string shader = BuildShader(
      R"(
               OpDecorate %sam DescriptorSet 0
               OpDecorate %sam Binding 2
               OpDecorate %texture DescriptorSet 0
               OpDecorate %texture Binding 5
               )",
      R"(
            ; CHECK: [[tex:%\w+]] = OpVariable %_ptr_UniformConstant_type_sampled_image UniformConstant
        %sam = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
    %texture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
               )",
      R"(
            ; CHECK: [[load:%\w+]] = OpLoad %type_sampled_image [[tex]]
            ; CHECK: [[image_extraction:%\w+]] = OpImage %type_2d_image [[load]]
            ; CHECK: OpSampledImage %type_sampled_image [[image_extraction]]
         %25 = OpLoad %type_2d_image %texture
         %26 = OpLoad %type_sampler %sam
         %27 = OpSampledImage %type_sampled_image %25 %26
         %28 = OpImageSampleExplicitLod %v4float %27 %12 Lod|ConstOffset %float_10 %17
               OpStore %out_var_SV_TARGET %28
               )");

  auto result = SinglePassRunAndMatch<ConvertToSampledImagePass>(
      shader, /* do_validate = */ true,
      VectorOfDescriptorSetAndBindingPairs{DescriptorSetAndBinding{0, 5}});

  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(ConvertToSampledImageTest, SamplerToSampledImage) {
  const std::string shader = BuildShader(
      R"(
               OpDecorate %sam DescriptorSet 0
               OpDecorate %sam Binding 2
               OpDecorate %texture DescriptorSet 0
               OpDecorate %texture Binding 5
               )",
      R"(
        %sam = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
    %texture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
               )",
      R"(
         %25 = OpLoad %type_2d_image %texture
         %26 = OpLoad %type_sampler %sam
         %27 = OpSampledImage %type_sampled_image %25 %26
         %28 = OpImageSampleExplicitLod %v4float %27 %12 Lod|ConstOffset %float_10 %17
               OpStore %out_var_SV_TARGET %28
               )");

  auto result = SinglePassRunToBinary<ConvertToSampledImagePass>(
      shader, /* skip_nop = */ false,
      VectorOfDescriptorSetAndBindingPairs{DescriptorSetAndBinding{0, 2}});

  EXPECT_EQ(std::get<1>(result), Pass::Status::Failure);
}

TEST_F(ConvertToSampledImageTest, TwoImagesWithDuplicatedDescriptorSetBinding) {
  const std::string shader = BuildShader(
      R"(
               OpDecorate %sam DescriptorSet 0
               OpDecorate %sam Binding 2
               OpDecorate %texture0 DescriptorSet 0
               OpDecorate %texture0 Binding 5
               OpDecorate %texture1 DescriptorSet 0
               OpDecorate %texture1 Binding 5
               )",
      R"(
        %sam = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
   %texture0 = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
   %texture1 = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
               )",
      R"(
         %25 = OpLoad %type_2d_image %texture0
         %26 = OpLoad %type_sampler %sam
         %27 = OpSampledImage %type_sampled_image %25 %26
         %28 = OpImageSampleExplicitLod %v4float %27 %12 Lod|ConstOffset %float_10 %17
               OpStore %out_var_SV_TARGET %28
               )");

  auto result = SinglePassRunToBinary<ConvertToSampledImagePass>(
      shader, /* skip_nop = */ false,
      VectorOfDescriptorSetAndBindingPairs{DescriptorSetAndBinding{0, 5}});

  EXPECT_EQ(std::get<1>(result), Pass::Status::Failure);
}

TEST_F(ConvertToSampledImageTest,
       TwoSamplersWithDuplicatedDescriptorSetBinding) {
  const std::string shader = BuildShader(
      R"(
               OpDecorate %sam0 DescriptorSet 0
               OpDecorate %sam0 Binding 2
               OpDecorate %sam1 DescriptorSet 0
               OpDecorate %sam1 Binding 2
               OpDecorate %texture DescriptorSet 0
               OpDecorate %texture Binding 5
               )",
      R"(
       %sam0 = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
       %sam1 = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
    %texture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
               )",
      R"(
         %25 = OpLoad %type_2d_image %texture
         %26 = OpLoad %type_sampler %sam0
         %27 = OpSampledImage %type_sampled_image %25 %26
         %28 = OpImageSampleExplicitLod %v4float %27 %12 Lod|ConstOffset %float_10 %17
               OpStore %out_var_SV_TARGET %28
               )");

  auto result = SinglePassRunToBinary<ConvertToSampledImagePass>(
      shader, /* skip_nop = */ false,
      VectorOfDescriptorSetAndBindingPairs{DescriptorSetAndBinding{0, 2}});

  EXPECT_EQ(std::get<1>(result), Pass::Status::Failure);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
