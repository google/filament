// Copyright (c) 2023-2025 Arm Ltd.
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

#include "gmock/gmock.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::ContainsRegex;
using ::testing::HasSubstr;

using ValidateTensor = spvtest::ValidateBase<std::string>;

constexpr spv_target_env SPVENV = SPV_ENV_VULKAN_1_3;

std::string GenerateModule(const std::string& body) {
  const std::string header = R"(
                     OpCapability Shader
                     OpCapability VulkanMemoryModel
                     OpCapability Int8
                     OpCapability TensorsARM
                     OpExtension "SPV_ARM_tensors"
                     OpMemoryModel Logical Vulkan
                     OpEntryPoint GLCompute %fnep "main"
                     OpExecutionMode %fnep LocalSize 1 1 1
                     OpDecorate %tensor_var DescriptorSet 0
                     OpDecorate %tensor_var Binding 0
                     OpDecorate %tensor_var_float_unranked DescriptorSet 0
                     OpDecorate %tensor_var_float_unranked Binding 1
             %void = OpTypeVoid
             %uint = OpTypeInt 32 0
            %float = OpTypeFloat 32
             %fnty = OpTypeFunction %void
        %uint_vec4 = OpTypeVector %uint 4
           %uint_0 = OpConstant %uint 0
      %uint_0_spec = OpSpecConstant %uint 0
           %uint_1 = OpConstant %uint 1
           %uint_2 = OpConstant %uint 2
           %uint_3 = OpConstant %uint 3
           %uint_4 = OpConstant %uint 4
          %uint_42 = OpConstant %uint 42
       %uint_1_bis = OpConstant %uint 1
          %float_1 = OpConstant %float 1
%uint_vec4_1_1_1_1 = OpConstantComposite %uint_vec4 %uint_1 %uint_1 %uint_1 %uint_1
        %uint_arr1 = OpTypeArray %uint %uint_1
        %uint_arr2 = OpTypeArray %uint %uint_2
        %uint_arr3 = OpTypeArray %uint %uint_3
        %uint_arr4 = OpTypeArray %uint %uint_4
       %float_arr4 = OpTypeArray %float %uint_4
%uint_arr4_1_1_1_1 = OpConstantComposite %uint_arr4 %uint_1 %uint_1 %uint_1 %uint_1
%uint_arr4_0_0_0_0_spec = OpSpecConstantComposite %uint_arr4 %uint_0_spec %uint_0_spec %uint_0_spec %uint_0_spec
    %uint_arr2_1_1 = OpConstantComposite %uint_arr2 %uint_1 %uint_1
%float_arr4_1_1_1_1 = OpConstantComposite %float_arr4 %float_1 %float_1 %float_1 %float_1
%uint_arr4_1_1_0_1 = OpConstantComposite %uint_arr4 %uint_1 %uint_1 %uint_0 %uint_1
 %uint_ptr_Private = OpTypePointer Private %uint
%uint_arr4_ptr_Private = OpTypePointer Private %uint_arr4
       %uint_var_1 = OpVariable %uint_ptr_Private Private %uint_1
%var_uint_arr4_1_1_1_1 = OpVariable %uint_arr4_ptr_Private Private %uint_arr4_1_1_1_1
    %tensor_uint_4 = OpTypeTensorARM %uint %uint_4
     %tensor_float = OpTypeTensorARM %float
%tensor_uint_4_ptr_UniformConstant = OpTypePointer UniformConstant %tensor_uint_4
%tensor_var = OpVariable %tensor_uint_4_ptr_UniformConstant UniformConstant
%tensor_float_ptr_UniformConstant = OpTypePointer UniformConstant %tensor_float
%tensor_var_float_unranked = OpVariable %tensor_float_ptr_UniformConstant UniformConstant
)";
  const std::string footer = R"(
             %fnep = OpFunction %void None %fnty
            %label = OpLabel
                     OpReturn
                     OpFunctionEnd
)";
  return header + body + footer;
}

//
// Type tests
//

TEST_F(ValidateTensor, ValidTypeElementTypeOnly) {
  const std::string src = R"(
    %test_type = OpTypeTensorARM %uint
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, InvalidTypeTensorElementTypeNotScalar) {
  const std::string src = R"(
    %test_type = OpTypeTensorARM %void
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("OpTypeTensorARM Element Type <id> "
                            "'.*' is not a scalar type.*"));
}

TEST_F(ValidateTensor, InvalidTypeDuplicateElementTypeTensorType) {
  const std::string src = R"(
    %typeA = OpTypeTensorARM %uint
    %typeB = OpTypeTensorARM %uint
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Duplicate non-aggregate type declarations are not allowed."));
}

TEST_F(ValidateTensor, ValidTypeElementTypeAndRank) {
  const std::string src = R"(
    %test_type = OpTypeTensorARM %uint %uint_1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, ValidTypeElementTypeAndRankUsingSpecConstant) {
  const std::string src = R"(
    %test_type = OpTypeTensorARM %uint %uint_0_spec
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, InvalidTypeDuplicateRankedTensorType) {
  const std::string src = R"(
    %typeA = OpTypeTensorARM %uint %uint_1
    %typeB = OpTypeTensorARM %uint %uint_1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Duplicate non-aggregate type declarations are not allowed."));
}

TEST_F(ValidateTensor, ValidTypeEquivalentRankedTensorType) {
  const std::string src = R"(
    %typeA = OpTypeTensorARM %uint %uint_1
    %typeB = OpTypeTensorARM %uint %uint_1_bis
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, InvalidTypeRankNotConstant) {
  const std::string src = R"(
    %test_type = OpTypeTensorARM %uint %uint_var_1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("OpTypeTensorARM Rank <id> '.*' "
                            "is not a constant instruction.*"));
}

TEST_F(ValidateTensor, InvalidTypeRankConstantButNotIntegerType) {
  const std::string src = R"(
    %test_type = OpTypeTensorARM %uint %float_1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("OpTypeTensorARM Rank <id> '.*' "
                            "does not have a scalar integer type.*"));
}

TEST_F(ValidateTensor, InvalidTypeRankConstantIntegerTypeButNotScalar) {
  const std::string src = R"(
    %test_type = OpTypeTensorARM %uint %uint_vec4_1_1_1_1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("OpTypeTensorARM Rank <id> '.*' "
                            "does not have a scalar integer type.*"));
}

TEST_F(ValidateTensor, InvalidTypeRank0) {
  const std::string src = R"(
    %test_type = OpTypeTensorARM %uint %uint_0
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("OpTypeTensorARM Rank <id> '.*' must "
                            "define a value greater than 0.*"));
}

TEST_F(ValidateTensor, ValidTypeElementTypeAndRankAndShape) {
  const std::string src = R"(
    %test_type = OpTypeTensorARM %uint %uint_4 %uint_arr4_1_1_1_1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, ValidTypeElementTypeAndRankAndShapeUsingSpecConstant) {
  const std::string src = R"(
    %test_type = OpTypeTensorARM %uint %uint_0_spec %uint_arr4_0_0_0_0_spec
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, InvalidTypeShapeNotConstant) {
  const std::string src = R"(
    %test_type = OpTypeTensorARM %uint %uint_4 %var_uint_arr4_1_1_1_1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("OpTypeTensorARM Shape <id> '.*' is not "
                            "a constant instruction.*"));
}

TEST_F(ValidateTensor, InvalidTypeShapeConstantButNotArrayOfInteger) {
  const std::string src = R"(
    %test_type = OpTypeTensorARM %uint %uint_4 %uint_vec4_1_1_1_1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex(
                  "OpTypeTensorARM Shape <id> '.*' is not "
                  "an array of integer type whose Length is equal to Rank.*"));
}

TEST_F(ValidateTensor, InvalidTypeShapeConstantArrayOfIntegerWrongLength) {
  const std::string src = R"(
    %test_type = OpTypeTensorARM %uint %uint_1 %uint_arr4_1_1_1_1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex(
                  "OpTypeTensorARM Shape <id> '.*' is not "
                  "an array of integer type whose Length is equal to Rank.*"));
}

TEST_F(ValidateTensor,
       InvalidTypeShapeConstantArrayOfIntegerWithZeroConstituent) {
  const std::string src = R"(
    %test_type = OpTypeTensorARM %uint %uint_4 %uint_arr4_1_1_0_1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      ContainsRegex(
          "OpTypeTensorARM Shape constituent 2 is not greater than 0.*"));
}

//
// Constants tests
//
TEST_F(ValidateTensor, ValidNullTensorConstantRank1) {
  const std::string src = R"(
   %uint_arr1_4 = OpConstantComposite %uint_arr1 %uint_4
    %ts_uint_r1 = OpTypeTensorARM %uint %uint_1 %uint_arr1_4
           %cst = OpConstantNull %ts_uint_r1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, ValidNullTensorConstantRank4) {
  const std::string src = R"(
%uint_arr4_2_2_2_2 = OpConstantComposite %uint_arr4 %uint_2 %uint_2 %uint_2 %uint_2
       %ts_uint_r4 = OpTypeTensorARM %uint %uint_4 %uint_arr4_2_2_2_2
              %cst = OpConstantNull %ts_uint_r4
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, InvalidNullTensorConstantRankedButNotShaped) {
  const std::string src = R"(
       %ts_uint_r1 = OpTypeTensorARM %uint %uint_1
              %cst = OpConstantNull %ts_uint_r1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("OpConstantNull Result Type <id> '.*' "
                            "cannot have a null value.*"));
}

TEST_F(ValidateTensor, InvalidNullTensorConstantNotRanked) {
  const std::string src = R"(
       %ts_uint_r1 = OpTypeTensorARM %uint
              %cst = OpConstantNull %ts_uint_r1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("OpConstantNull Result Type <id> '.*' "
                            "cannot have a null value.*"));
}

TEST_F(ValidateTensor, ValidTensorConstantRank1) {
  const std::string src = R"(
   %uint_arr1_4 = OpConstantComposite %uint_arr1 %uint_4
    %ts_uint_r1 = OpTypeTensorARM %uint %uint_1 %uint_arr1_4
           %cst = OpConstantComposite %ts_uint_r1 %uint_42 %uint_42 %uint_42 %uint_42
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, ValidTensorConstantRank1SpecConstant) {
  const std::string src = R"(
   %uint_arr1_4 = OpSpecConstantComposite %uint_arr1 %uint_0_spec
    %ts_uint_r1 = OpTypeTensorARM %uint %uint_0_spec %uint_arr1_4
           %cst = OpSpecConstantComposite %ts_uint_r1 %uint_0_spec %uint_0_spec %uint_0_spec %uint_0_spec
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, InvalidTensorConstantRank1NotEnoughConstituents) {
  const std::string src = R"(
   %uint_arr1_4 = OpConstantComposite %uint_arr1 %uint_4
    %ts_uint_r1 = OpTypeTensorARM %uint %uint_1 %uint_arr1_4
           %cst = OpConstantComposite %ts_uint_r1 %uint_42 %uint_42 %uint_42
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      ContainsRegex("OpConstantComposite Constituent count does not match the "
                    "shape of Result Type <id> '.*' "
                    "along its outermost dimension, expected 4 but got 3.*"));
}

TEST_F(ValidateTensor, InvalidTensorConstantRank1TooManyConstituents) {
  const std::string src = R"(
   %uint_arr1_4 = OpConstantComposite %uint_arr1 %uint_4
    %ts_uint_r1 = OpTypeTensorARM %uint %uint_1 %uint_arr1_4
           %cst = OpConstantComposite %ts_uint_r1 %uint_42 %uint_42 %uint_42 %uint_42 %uint_42
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      ContainsRegex("OpConstantComposite Constituent count does not match the "
                    "shape of Result Type <id> '.*' "
                    "along its outermost dimension, expected 4 but got 5.*"));
}

TEST_F(ValidateTensor,
       InvalidTensorConstantRank1ConstituentNotConstantOrUndef) {
  const std::string src = R"(
   %uint_arr1_4 = OpConstantComposite %uint_arr1 %uint_4
    %ts_uint_r1 = OpTypeTensorARM %uint %uint_1 %uint_arr1_4
           %cst = OpConstantComposite %ts_uint_r1 %tensor_var %tensor_var %tensor_var %tensor_var
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("OpConstantComposite Constituent <id> "
                            "'.*' is not a constant or undef.*"));
}

TEST_F(ValidateTensor, InvalidTensorConstantRank1WrongConstituentType) {
  const std::string src = R"(
   %uint_arr1_4 = OpConstantComposite %uint_arr1 %uint_4
    %ts_uint_r1 = OpTypeTensorARM %uint %uint_1 %uint_arr1_4
           %cst = OpConstantComposite %ts_uint_r1 %uint_42 %uint_42 %float_1 %uint_42
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      ContainsRegex("OpConstantComposite Constituent <id> '.*' "
                    "type does not match the element type of the tensor "
                    ".*"));
}

TEST_F(ValidateTensor, ValidTensorConstantRank4) {
  const std::string src = R"(
      %uint_arr1_2 = OpConstantComposite %uint_arr1 %uint_2
    %uint_arr2_2_2 = OpConstantComposite %uint_arr2 %uint_2 %uint_2
  %uint_arr3_2_2_2 = OpConstantComposite %uint_arr3 %uint_2 %uint_2 %uint_2
%uint_arr4_2_2_2_2 = OpConstantComposite %uint_arr4 %uint_2 %uint_2 %uint_2 %uint_2
       %ts_uint_r1 = OpTypeTensorARM %uint %uint_1 %uint_arr1_2
       %ts_uint_r2 = OpTypeTensorARM %uint %uint_2 %uint_arr2_2_2
       %ts_uint_r3 = OpTypeTensorARM %uint %uint_3 %uint_arr3_2_2_2
       %ts_uint_r4 = OpTypeTensorARM %uint %uint_4 %uint_arr4_2_2_2_2
           %cst_r1 = OpConstantComposite %ts_uint_r1 %uint_42 %uint_42
           %cst_r2 = OpConstantComposite %ts_uint_r2 %cst_r1 %cst_r1
           %cst_r3 = OpConstantComposite %ts_uint_r3 %cst_r2 %cst_r2
           %cst_r4 = OpConstantComposite %ts_uint_r4 %cst_r3 %cst_r3
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, ValidTensorConstantRank2SpecConstantConstituent) {
  const std::string src = R"(
      %uint_arr1_2 = OpSpecConstantComposite %uint_arr1 %uint_0_spec
    %uint_arr2_2_2 = OpSpecConstantComposite %uint_arr2 %uint_0_spec %uint_0_spec
       %ts_uint_r1 = OpTypeTensorARM %uint %uint_0_spec %uint_arr1_2
       %ts_uint_r2 = OpTypeTensorARM %uint %uint_2 %uint_arr2_2_2
           %cst_r1 = OpSpecConstantComposite %ts_uint_r1 %uint_0_spec %uint_0_spec
           %cst_r2 = OpSpecConstantComposite %ts_uint_r2 %cst_r1 %cst_r1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, InvalidTensorConstantRank2ConstituentNotATensor) {
  const std::string src = R"(
      %uint_arr1_2 = OpConstantComposite %uint_arr1 %uint_2
    %uint_arr2_2_2 = OpConstantComposite %uint_arr2 %uint_2 %uint_2
       %ts_uint_r1 = OpTypeTensorARM %uint %uint_1 %uint_arr1_2
       %ts_uint_r2 = OpTypeTensorARM %uint %uint_2 %uint_arr2_2_2
           %cst_r1 = OpConstantComposite %ts_uint_r1 %uint_42 %uint_42
           %cst_r2 = OpConstantComposite %ts_uint_r2 %uint_1 %cst_r1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("OpConstantComposite Constituent <id> "
                            "'.*' must be an OpTypeTensorARM.*"));
}

TEST_F(ValidateTensor,
       InvalidTensorConstantRank2ConstituentTensorWrongElementType) {
  const std::string src = R"(
      %uint_arr1_2 = OpConstantComposite %uint_arr1 %uint_2
    %uint_arr2_2_2 = OpConstantComposite %uint_arr2 %uint_2 %uint_2
       %ts_float_r1 = OpTypeTensorARM %float %uint_1 %uint_arr1_2
       %ts_uint_r2 = OpTypeTensorARM %uint %uint_2 %uint_arr2_2_2
           %cst_r1 = OpConstantComposite %ts_float_r1 %float_1 %float_1
           %cst_r2 = OpConstantComposite %ts_uint_r2 %cst_r1 %cst_r1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("OpConstantComposite Constituent <id> "
                            "'.*' must have the same Element Type "
                            "as Result Type <id> .*"));
}

TEST_F(ValidateTensor, InvalidTensorConstantRank2ConstituentTensorWrongRank) {
  const std::string src = R"(
      %uint_arr1_2 = OpConstantComposite %uint_arr1 %uint_2
    %uint_arr2_2_2 = OpConstantComposite %uint_arr2 %uint_2 %uint_2
       %ts_uint_r1 = OpTypeTensorARM %uint %uint_1 %uint_arr1_2
       %ts_uint_r2 = OpTypeTensorARM %uint %uint_2 %uint_arr2_2_2
           %cst_r1 = OpConstantComposite %ts_uint_r1 %uint_42 %uint_42
           %cst_r2 = OpConstantComposite %ts_uint_r2 %cst_r1 %cst_r1
              %cst = OpConstantComposite %ts_uint_r2 %cst_r1 %cst_r2
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("OpConstantComposite Constituent <id> "
                            "'.*' must have a Rank that is "
                            "1 less than the Rank of Result Type <id> "
                            "'.*', expected 1 but got 2.*"));
}

TEST_F(ValidateTensor, InvalidTensorConstantRank2ConstituentTensorWrongShape) {
  const std::string src = R"(
      %uint_arr1_3 = OpConstantComposite %uint_arr1 %uint_3
    %uint_arr2_2_2 = OpConstantComposite %uint_arr2 %uint_2 %uint_2
       %ts_uint_r1 = OpTypeTensorARM %uint %uint_1 %uint_arr1_3
       %ts_uint_r2 = OpTypeTensorARM %uint %uint_2 %uint_arr2_2_2
           %cst_r1 = OpConstantComposite %ts_uint_r1 %uint_42 %uint_42 %uint_42
           %cst_r2 = OpConstantComposite %ts_uint_r2 %cst_r1 %cst_r1
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      ContainsRegex(
          "OpConstantComposite Constituent <id> "
          "'.*' must have a Shape that matches "
          "that of Result Type <id> '.*' along "
          "all inner dimensions of Result Type, expected 2 for dimension 0 of "
          "Constituent but got 3.*"));
}

//
// Read tests
//

TEST_F(ValidateTensor, ValidTensorReadScalar) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
            %val = OpTensorReadARM %uint %tensor %uint_arr4_1_1_1_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, ValidTensorReadArray) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
            %val = OpTensorReadARM %uint_arr2 %tensor %uint_arr4_1_1_1_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, InvalidTensorReadResultTypeVoid) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
            %val = OpTensorReadARM %void %tensor %uint_arr4_1_1_1_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected Result Type to be a scalar type or array of scalar type"));
}

TEST_F(ValidateTensor, InvalidTensorReadResultTypeVector) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
            %val = OpTensorReadARM %uint_vec4 %tensor %uint_arr4_1_1_1_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected Result Type to be a scalar type or array of scalar type"));
}

TEST_F(ValidateTensor,
       InvalidTensorReadResultTypeDoesNotMatchTensorElementType) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
            %val = OpTensorReadARM %float %tensor %uint_arr4_1_1_1_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected Result Type to be the same as the Element Type of Tensor"));
}

TEST_F(ValidateTensor, InvalidTensorReadTensorNotRanked) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_float %tensor_var_float_unranked
            %val = OpTensorReadARM %float %tensor %uint_arr4_1_1_1_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected Tensor to be an OpTypeTensorARM whose Rank is specified"));
}

TEST_F(ValidateTensor, InvalidTensorReadCoordinatesNotArray) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
            %val = OpTensorReadARM %uint %tensor %uint_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected Coordinates to be an array whose Element Type is an "
          "integer type and whose Length is equal to the Rank of Tensor."));
}

TEST_F(ValidateTensor, InvalidTensorReadCoordinatesArrayNotInteger) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
            %val = OpTensorReadARM %uint %tensor %float_arr4_1_1_1_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected Coordinates to be an array whose Element Type is an "
          "integer type and whose Length is equal to the Rank of Tensor."));
}

TEST_F(ValidateTensor, InvalidTensorReadCoordinatesArrayIntegerWrongLength) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
            %val = OpTensorReadARM %uint %tensor %uint_arr2_1_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected Coordinates to be an array whose Element Type is an "
          "integer type and whose Length is equal to the Rank of Tensor."));
}

TEST_F(ValidateTensor, ValidTensorReadScalarWithNoneTensorOperands) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
            %val = OpTensorReadARM %uint %tensor %uint_arr4_1_1_1_1 NoneARM
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, ValidTensorReadScalarWithNontemporalTensorOperands) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
            %val = OpTensorReadARM %uint %tensor %uint_arr4_1_1_1_1 NontemporalARM
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, ValidTensorReadScalarOutOfBoundsValueTensorOperands) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
            %val = OpTensorReadARM %uint %tensor %uint_arr4_1_1_1_1 OutOfBoundsValueARM %uint_42
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor,
       InvalidTensorReadScalarOutOfBoundsValueTensorOperandsValueWrongType) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
            %val = OpTensorReadARM %uint %tensor %uint_arr4_1_1_1_1 OutOfBoundsValueARM %float_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected the type of the OutOfBoundsValueARM value to be the same "
          "as the Element Type of Tensor."));
}

TEST_F(ValidateTensor,
       InvalidTensorReadScalarMakeElementAvailableTensorOperands) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
            %val = OpTensorReadARM %uint %tensor %uint_arr4_1_1_1_1 MakeElementAvailableARM %uint_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "MakeElementAvailableARM cannot be used with OpTensorReadARM."));
}

TEST_F(ValidateTensor,
       ValidTensorReadScalarWithMakeElementVisibleTensorOperands) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
            %val = OpTensorReadARM %uint %tensor %uint_arr4_1_1_1_1 MakeElementVisibleARM|NonPrivateElementARM %uint_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(
    ValidateTensor,
    InvalidTensorReadScalarMakeElementVisibleTensorOperandsWithoutNonPrivateElement) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
            %val = OpTensorReadARM %uint %tensor %uint_arr4_1_1_1_1 MakeElementVisibleARM %uint_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("MakeElementAvailableARM requires NonPrivateElementARM"));
}

//
// Write tests
//

TEST_F(ValidateTensor, ValidTensorWriteScalar) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
                   OpTensorWriteARM %tensor %uint_arr4_1_1_1_1 %uint_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, ValidTensorWriteArray) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
                   OpTensorWriteARM %tensor %uint_arr4_1_1_1_1 %uint_arr2_1_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, InvalidTensorWriteObjectNotScalarOrArrayOfScalar) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
                   OpTensorWriteARM %tensor %uint_arr4_1_1_1_1 %uint_vec4_1_1_1_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected Object to be a scalar type or array of scalar type "
                "that is the same as the Element Type of Tensor."));
}

TEST_F(ValidateTensor, InvalidTensorWriteObjectDoesNotMatchTensorElementType) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
                   OpTensorWriteARM %tensor %uint_arr4_1_1_1_1 %float_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected Object to be a scalar type or array of scalar type "
                "that is the same as the Element Type of Tensor."));
}

TEST_F(ValidateTensor, InvalidTensorWriteTensorNotRanked) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_float %tensor_var_float_unranked
                   OpTensorWriteARM %tensor %uint_arr4_1_1_1_1 %float_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected Tensor to be an OpTypeTensorARM whose Rank is specified"));
}

TEST_F(ValidateTensor, InvalidTensorWriteCoordinatesNotArray) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
                   OpTensorWriteARM %tensor %uint_1 %uint_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected Coordinates to be an array whose Element Type is an "
          "integer type and whose Length is equal to the Rank of Tensor."));
}

TEST_F(ValidateTensor, InvalidTensorWriteCoordinatesArrayNotInteger) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
                   OpTensorWriteARM %tensor %float_arr4_1_1_1_1 %uint_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected Coordinates to be an array whose Element Type is an "
          "integer type and whose Length is equal to the Rank of Tensor."));
}

TEST_F(ValidateTensor, InvalidTensorWriteCoordinatesArrayIntegerWrongLength) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
                   OpTensorWriteARM %tensor %uint_arr2_1_1 %uint_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected Coordinates to be an array whose Element Type is an "
          "integer type and whose Length is equal to the Rank of Tensor."));
}

TEST_F(ValidateTensor, ValidTensorWriteScalarWithNoneTensorOperands) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
                   OpTensorWriteARM %tensor %uint_arr4_1_1_1_1 %uint_1 NoneARM
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, ValidTensorWriteScalarWithNontemporalTensorOperands) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
                   OpTensorWriteARM %tensor %uint_arr4_1_1_1_1 %uint_1 NontemporalARM
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor,
       ValidTensorWriteScalarWithMakeElementAvailableTensorOperands) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
                   OpTensorWriteARM %tensor %uint_arr4_1_1_1_1 %uint_1 MakeElementAvailableARM|NonPrivateElementARM %uint_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor,
       InvalidTensorWriteScalarWithOutOfVoundsValueTensorOperands) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
                   OpTensorWriteARM %tensor %uint_arr4_1_1_1_1 %uint_1 OutOfBoundsValueARM %uint_42
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OutOfBoundsValue Tensor Operand not allowed with "
                        "OpTensorWriteARM."));
}

TEST_F(ValidateTensor,
       InvalidTensorWriteScalarWithMakeElementVisibleTensorOperands) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
                   OpTensorWriteARM %tensor %uint_arr4_1_1_1_1 %uint_1 MakeElementVisibleARM %uint_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("MakeElementVisibleARM not allowed with OpTensorWriteARM."));
}

TEST_F(
    ValidateTensor,
    InvalidTensorWriteScalarWithMakeElementAvailableTensorOperandsWithoutNonPrivateElement) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
                   OpTensorWriteARM %tensor %uint_arr4_1_1_1_1 %uint_1 MakeElementAvailableARM %uint_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("MakeElementAvailableARM requires NonPrivateElementARM."));
}

//
// Query Size tests
//

TEST_F(ValidateTensor, ValidTensorQuerySize) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
           %size = OpTensorQuerySizeARM %uint %tensor %uint_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, ValidTensorQuerySizeSpecConstant) {
  const std::string src = R"(
%tensor_uint_4_spec = OpTypeTensorARM %uint %uint_0_spec
%tensor_uint_4_spec_ptr_UniformConstant = OpTypePointer UniformConstant %tensor_uint_4_spec
%tensor_var_spec = OpVariable %tensor_uint_4_spec_ptr_UniformConstant UniformConstant
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4_spec %tensor_var_spec
           %size = OpTensorQuerySizeARM %uint %tensor %uint_0_spec
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateTensor, InvalidTensorQuerySizeTensorNotRanked) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_float %tensor_var_float_unranked
           %size = OpTensorQuerySizeARM %uint %tensor %uint_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected Tensor to be an OpTypeTensorARM whose Rank is specified"));
}

TEST_F(ValidateTensor, InvalidTensorQuerySizeResultNotIntegerScalarType) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_float %tensor_var_float_unranked
           %size = OpTensorQuerySizeARM %float %tensor %uint_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Result Type to be an integer type scalar"));
}

TEST_F(ValidateTensor, InvalidTensorQuerySizeDimensionNotIntegerType) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
           %size = OpTensorQuerySizeARM %uint %tensor %float_1
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Dimension must come from a constant instruction of "
                        "scalar integer type."));
}

TEST_F(ValidateTensor, InvalidTensorQuerySizeDimensionNotConstant) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
           %dims = OpLoad %uint_arr4 %var_uint_arr4_1_1_1_1
            %dim = OpCompositeExtract %uint %dims 0
           %size = OpTensorQuerySizeARM %uint %tensor %dim
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Dimension must come from a constant instruction of "
                        "scalar integer type."));
}

TEST_F(ValidateTensor, InvalidTensorQuerySizeDimension) {
  const std::string src = R"(
             %fn = OpFunction %void None %fnty
         %label1 = OpLabel
         %tensor = OpLoad %tensor_uint_4 %tensor_var
           %size = OpTensorQuerySizeARM %uint %tensor %uint_42
                   OpReturn
                   OpFunctionEnd
)";
  std::string spvasm = GenerateModule(src);
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Dimension (42) must be less than the Rank of Tensor (4)"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
