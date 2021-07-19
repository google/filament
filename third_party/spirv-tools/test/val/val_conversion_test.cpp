// Copyright (c) 2017 Google Inc.
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

// Tests for unique type declaration rules validator.

#include <string>

#include "gmock/gmock.h"
#include "test/unit_spirv.h"
#include "test/val/val_code_generator.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::Values;

using ValidateConversion = spvtest::ValidateBase<bool>;

std::string GenerateShaderCode(
    const std::string& body,
    const std::string& capabilities_and_extensions = "",
    const std::string& decorations = "", const std::string& types = "",
    const std::string& variables = "") {
  const std::string capabilities =
      R"(
OpCapability Shader
OpCapability Int64
OpCapability Float64)";

  const std::string after_extension_before_decorations =
      R"(
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft)";

  const std::string after_decorations_before_types =
      R"(
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f32 = OpTypeFloat 32
%u32 = OpTypeInt 32 0
%s32 = OpTypeInt 32 1
%f64 = OpTypeFloat 64
%u64 = OpTypeInt 64 0
%s64 = OpTypeInt 64 1
%boolvec2 = OpTypeVector %bool 2
%s32vec2 = OpTypeVector %s32 2
%u32vec2 = OpTypeVector %u32 2
%u64vec2 = OpTypeVector %u64 2
%f32vec2 = OpTypeVector %f32 2
%f64vec2 = OpTypeVector %f64 2
%boolvec3 = OpTypeVector %bool 3
%u32vec3 = OpTypeVector %u32 3
%u64vec3 = OpTypeVector %u64 3
%s32vec3 = OpTypeVector %s32 3
%f32vec3 = OpTypeVector %f32 3
%f64vec3 = OpTypeVector %f64 3
%boolvec4 = OpTypeVector %bool 4
%u32vec4 = OpTypeVector %u32 4
%u64vec4 = OpTypeVector %u64 4
%s32vec4 = OpTypeVector %s32 4
%f32vec4 = OpTypeVector %f32 4
%f64vec4 = OpTypeVector %f64 4

%f32_0 = OpConstant %f32 0
%f32_1 = OpConstant %f32 1
%f32_2 = OpConstant %f32 2
%f32_3 = OpConstant %f32 3
%f32_4 = OpConstant %f32 4

%s32_0 = OpConstant %s32 0
%s32_1 = OpConstant %s32 1
%s32_2 = OpConstant %s32 2
%s32_3 = OpConstant %s32 3
%s32_4 = OpConstant %s32 4
%s32_m1 = OpConstant %s32 -1

%u32_0 = OpConstant %u32 0
%u32_1 = OpConstant %u32 1
%u32_2 = OpConstant %u32 2
%u32_3 = OpConstant %u32 3
%u32_4 = OpConstant %u32 4

%f64_0 = OpConstant %f64 0
%f64_1 = OpConstant %f64 1
%f64_2 = OpConstant %f64 2
%f64_3 = OpConstant %f64 3
%f64_4 = OpConstant %f64 4

%s64_0 = OpConstant %s64 0
%s64_1 = OpConstant %s64 1
%s64_2 = OpConstant %s64 2
%s64_3 = OpConstant %s64 3
%s64_4 = OpConstant %s64 4
%s64_m1 = OpConstant %s64 -1

%u64_0 = OpConstant %u64 0
%u64_1 = OpConstant %u64 1
%u64_2 = OpConstant %u64 2
%u64_3 = OpConstant %u64 3
%u64_4 = OpConstant %u64 4

%u32vec2_01 = OpConstantComposite %u32vec2 %u32_0 %u32_1
%u32vec2_12 = OpConstantComposite %u32vec2 %u32_1 %u32_2
%u32vec3_012 = OpConstantComposite %u32vec3 %u32_0 %u32_1 %u32_2
%u32vec3_123 = OpConstantComposite %u32vec3 %u32_1 %u32_2 %u32_3
%u32vec4_0123 = OpConstantComposite %u32vec4 %u32_0 %u32_1 %u32_2 %u32_3
%u32vec4_1234 = OpConstantComposite %u32vec4 %u32_1 %u32_2 %u32_3 %u32_4

%s32vec2_01 = OpConstantComposite %s32vec2 %s32_0 %s32_1
%s32vec2_12 = OpConstantComposite %s32vec2 %s32_1 %s32_2
%s32vec3_012 = OpConstantComposite %s32vec3 %s32_0 %s32_1 %s32_2
%s32vec3_123 = OpConstantComposite %s32vec3 %s32_1 %s32_2 %s32_3
%s32vec4_0123 = OpConstantComposite %s32vec4 %s32_0 %s32_1 %s32_2 %s32_3
%s32vec4_1234 = OpConstantComposite %s32vec4 %s32_1 %s32_2 %s32_3 %s32_4

%f32vec2_01 = OpConstantComposite %f32vec2 %f32_0 %f32_1
%f32vec2_12 = OpConstantComposite %f32vec2 %f32_1 %f32_2
%f32vec3_012 = OpConstantComposite %f32vec3 %f32_0 %f32_1 %f32_2
%f32vec3_123 = OpConstantComposite %f32vec3 %f32_1 %f32_2 %f32_3
%f32vec4_0123 = OpConstantComposite %f32vec4 %f32_0 %f32_1 %f32_2 %f32_3
%f32vec4_1234 = OpConstantComposite %f32vec4 %f32_1 %f32_2 %f32_3 %f32_4

%f64vec2_01 = OpConstantComposite %f64vec2 %f64_0 %f64_1
%f64vec2_12 = OpConstantComposite %f64vec2 %f64_1 %f64_2
%f64vec3_012 = OpConstantComposite %f64vec3 %f64_0 %f64_1 %f64_2
%f64vec3_123 = OpConstantComposite %f64vec3 %f64_1 %f64_2 %f64_3
%f64vec4_0123 = OpConstantComposite %f64vec4 %f64_0 %f64_1 %f64_2 %f64_3
%f64vec4_1234 = OpConstantComposite %f64vec4 %f64_1 %f64_2 %f64_3 %f64_4

%true = OpConstantTrue %bool
%false = OpConstantFalse %bool

%f32ptr_func = OpTypePointer Function %f32)";

  const std::string after_variables_before_body =
      R"(
%main = OpFunction %void None %func
%main_entry = OpLabel)";

  const std::string after_body =
      R"(
OpReturn
OpFunctionEnd)";

  return capabilities + capabilities_and_extensions +
         after_extension_before_decorations + decorations +
         after_decorations_before_types + types + variables +
         after_variables_before_body + body + after_body;
}

std::string GenerateKernelCode(
    const std::string& body,
    const std::string& capabilities_and_extensions = "") {
  const std::string capabilities =
      R"(
OpCapability Addresses
OpCapability Kernel
OpCapability Linkage
OpCapability GenericPointer
OpCapability Int64
OpCapability Float64)";

  const std::string after_extension_before_body =
      R"(
OpMemoryModel Physical32 OpenCL
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f32 = OpTypeFloat 32
%u32 = OpTypeInt 32 0
%f64 = OpTypeFloat 64
%u64 = OpTypeInt 64 0
%boolvec2 = OpTypeVector %bool 2
%u32vec2 = OpTypeVector %u32 2
%u64vec2 = OpTypeVector %u64 2
%f32vec2 = OpTypeVector %f32 2
%f64vec2 = OpTypeVector %f64 2
%boolvec3 = OpTypeVector %bool 3
%u32vec3 = OpTypeVector %u32 3
%u64vec3 = OpTypeVector %u64 3
%f32vec3 = OpTypeVector %f32 3
%f64vec3 = OpTypeVector %f64 3
%boolvec4 = OpTypeVector %bool 4
%u32vec4 = OpTypeVector %u32 4
%u64vec4 = OpTypeVector %u64 4
%f32vec4 = OpTypeVector %f32 4
%f64vec4 = OpTypeVector %f64 4

%f32_0 = OpConstant %f32 0
%f32_1 = OpConstant %f32 1
%f32_2 = OpConstant %f32 2
%f32_3 = OpConstant %f32 3
%f32_4 = OpConstant %f32 4

%u32_0 = OpConstant %u32 0
%u32_1 = OpConstant %u32 1
%u32_2 = OpConstant %u32 2
%u32_3 = OpConstant %u32 3
%u32_4 = OpConstant %u32 4

%f64_0 = OpConstant %f64 0
%f64_1 = OpConstant %f64 1
%f64_2 = OpConstant %f64 2
%f64_3 = OpConstant %f64 3
%f64_4 = OpConstant %f64 4

%u64_0 = OpConstant %u64 0
%u64_1 = OpConstant %u64 1
%u64_2 = OpConstant %u64 2
%u64_3 = OpConstant %u64 3
%u64_4 = OpConstant %u64 4

%u32vec2_01 = OpConstantComposite %u32vec2 %u32_0 %u32_1
%u32vec2_12 = OpConstantComposite %u32vec2 %u32_1 %u32_2
%u32vec3_012 = OpConstantComposite %u32vec3 %u32_0 %u32_1 %u32_2
%u32vec3_123 = OpConstantComposite %u32vec3 %u32_1 %u32_2 %u32_3
%u32vec4_0123 = OpConstantComposite %u32vec4 %u32_0 %u32_1 %u32_2 %u32_3
%u32vec4_1234 = OpConstantComposite %u32vec4 %u32_1 %u32_2 %u32_3 %u32_4

%f32vec2_01 = OpConstantComposite %f32vec2 %f32_0 %f32_1
%f32vec2_12 = OpConstantComposite %f32vec2 %f32_1 %f32_2
%f32vec3_012 = OpConstantComposite %f32vec3 %f32_0 %f32_1 %f32_2
%f32vec3_123 = OpConstantComposite %f32vec3 %f32_1 %f32_2 %f32_3
%f32vec4_0123 = OpConstantComposite %f32vec4 %f32_0 %f32_1 %f32_2 %f32_3
%f32vec4_1234 = OpConstantComposite %f32vec4 %f32_1 %f32_2 %f32_3 %f32_4

%f64vec2_01 = OpConstantComposite %f64vec2 %f64_0 %f64_1
%f64vec2_12 = OpConstantComposite %f64vec2 %f64_1 %f64_2
%f64vec3_012 = OpConstantComposite %f64vec3 %f64_0 %f64_1 %f64_2
%f64vec3_123 = OpConstantComposite %f64vec3 %f64_1 %f64_2 %f64_3
%f64vec4_0123 = OpConstantComposite %f64vec4 %f64_0 %f64_1 %f64_2 %f64_3
%f64vec4_1234 = OpConstantComposite %f64vec4 %f64_1 %f64_2 %f64_3 %f64_4

%u64vec2_01 = OpConstantComposite %u64vec2 %u64_0 %u64_1

%true = OpConstantTrue %bool
%false = OpConstantFalse %bool

%f32ptr_func = OpTypePointer Function %f32
%u32ptr_func = OpTypePointer Function %u32
%f32ptr_gen = OpTypePointer Generic %f32
%f32ptr_inp = OpTypePointer Input %f32
%f32ptr_wg = OpTypePointer Workgroup %f32
%f32ptr_cwg = OpTypePointer CrossWorkgroup %f32

%f32inp = OpVariable %f32ptr_inp Input

%main = OpFunction %void None %func
%main_entry = OpLabel)";

  const std::string after_body =
      R"(
OpReturn
OpFunctionEnd)";

  return capabilities + capabilities_and_extensions +
         after_extension_before_body + body + after_body;
}

TEST_F(ValidateConversion, ConvertFToUSuccess) {
  const std::string body = R"(
%val1 = OpConvertFToU %u32 %f32_1
%val2 = OpConvertFToU %u32 %f64_0
%val3 = OpConvertFToU %u32vec2 %f32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, ConvertFToUWrongResultType) {
  const std::string body = R"(
%val = OpConvertFToU %s32 %f32_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected unsigned int scalar or vector type as Result "
                        "Type: ConvertFToU"));
}

TEST_F(ValidateConversion, ConvertFToUWrongInputType) {
  const std::string body = R"(
%val = OpConvertFToU %u32 %u32_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected input to be float scalar or vector: ConvertFToU"));
}

TEST_F(ValidateConversion, ConvertFToUDifferentDimension) {
  const std::string body = R"(
%val = OpConvertFToU %u32 %f32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to have the same dimension as Result "
                        "Type: ConvertFToU"));
}

TEST_F(ValidateConversion, ConvertFToSSuccess) {
  const std::string body = R"(
%val1 = OpConvertFToS %s32 %f32_1
%val2 = OpConvertFToS %u32 %f64_0
%val3 = OpConvertFToS %s32vec2 %f32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, ConvertFToSWrongResultType) {
  const std::string body = R"(
%val = OpConvertFToS %bool %f32_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected int scalar or vector type as Result Type: ConvertFToS"));
}

TEST_F(ValidateConversion, ConvertFToSWrongInputType) {
  const std::string body = R"(
%val = OpConvertFToS %s32 %u32_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected input to be float scalar or vector: ConvertFToS"));
}

TEST_F(ValidateConversion, ConvertFToSDifferentDimension) {
  const std::string body = R"(
%val = OpConvertFToS %u32 %f32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to have the same dimension as Result "
                        "Type: ConvertFToS"));
}

TEST_F(ValidateConversion, ConvertSToFSuccess) {
  const std::string body = R"(
%val1 = OpConvertSToF %f32 %u32_1
%val2 = OpConvertSToF %f32 %s64_0
%val3 = OpConvertSToF %f32vec2 %s32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, ConvertSToFWrongResultType) {
  const std::string body = R"(
%val = OpConvertSToF %u32 %s32_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected float scalar or vector type as Result Type: ConvertSToF"));
}

TEST_F(ValidateConversion, ConvertSToFWrongInputType) {
  const std::string body = R"(
%val = OpConvertSToF %f32 %f32_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected input to be int scalar or vector: ConvertSToF"));
}

TEST_F(ValidateConversion, ConvertSToFDifferentDimension) {
  const std::string body = R"(
%val = OpConvertSToF %f32 %u32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to have the same dimension as Result "
                        "Type: ConvertSToF"));
}

TEST_F(ValidateConversion, UConvertSuccess) {
  const std::string body = R"(
%val1 = OpUConvert %u32 %u64_1
%val2 = OpUConvert %u64 %s32_0
%val3 = OpUConvert %u64vec2 %s32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, UConvertWrongResultType) {
  const std::string body = R"(
%val = OpUConvert %s32 %s32_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected unsigned int scalar or vector type as Result "
                        "Type: UConvert"));
}

TEST_F(ValidateConversion, UConvertWrongInputType) {
  const std::string body = R"(
%val = OpUConvert %u32 %f32_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to be int scalar or vector: UConvert"));
}

TEST_F(ValidateConversion, UConvertDifferentDimension) {
  const std::string body = R"(
%val = OpUConvert %u32 %u32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to have the same dimension as Result "
                        "Type: UConvert"));
}

TEST_F(ValidateConversion, UConvertSameBitWidth) {
  const std::string body = R"(
%val = OpUConvert %u32 %s32_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to have different bit width from "
                        "Result Type: UConvert"));
}

TEST_F(ValidateConversion, SConvertSuccess) {
  const std::string body = R"(
%val1 = OpSConvert %s32 %u64_1
%val2 = OpSConvert %s64 %s32_0
%val3 = OpSConvert %u64vec2 %s32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, SConvertWrongResultType) {
  const std::string body = R"(
%val = OpSConvert %f32 %s32_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected int scalar or vector type as Result Type: SConvert"));
}

TEST_F(ValidateConversion, SConvertWrongInputType) {
  const std::string body = R"(
%val = OpSConvert %u32 %f32_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to be int scalar or vector: SConvert"));
}

TEST_F(ValidateConversion, SConvertDifferentDimension) {
  const std::string body = R"(
%val = OpSConvert %s32 %u32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to have the same dimension as Result "
                        "Type: SConvert"));
}

TEST_F(ValidateConversion, SConvertSameBitWidth) {
  const std::string body = R"(
%val = OpSConvert %u32 %s32_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to have different bit width from "
                        "Result Type: SConvert"));
}

TEST_F(ValidateConversion, FConvertSuccess) {
  const std::string body = R"(
%val1 = OpFConvert %f32 %f64_1
%val2 = OpFConvert %f64 %f32_0
%val3 = OpFConvert %f64vec2 %f32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, FConvertWrongResultType) {
  const std::string body = R"(
%val = OpFConvert %u32 %f32_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected float scalar or vector type as Result Type: FConvert"));
}

TEST_F(ValidateConversion, FConvertWrongInputType) {
  const std::string body = R"(
%val = OpFConvert %f32 %u64_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected input to be float scalar or vector: FConvert"));
}

TEST_F(ValidateConversion, FConvertDifferentDimension) {
  const std::string body = R"(
%val = OpFConvert %f64 %f32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to have the same dimension as Result "
                        "Type: FConvert"));
}

TEST_F(ValidateConversion, FConvertSameBitWidth) {
  const std::string body = R"(
%val = OpFConvert %f32 %f32_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to have different bit width from "
                        "Result Type: FConvert"));
}

TEST_F(ValidateConversion, QuantizeToF16Success) {
  const std::string body = R"(
%val1 = OpQuantizeToF16 %f32 %f32_1
%val2 = OpQuantizeToF16 %f32 %f32_0
%val3 = OpQuantizeToF16 %f32vec2 %f32vec2_01
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, QuantizeToF16WrongResultType) {
  const std::string body = R"(
%val = OpQuantizeToF16 %u32 %f32_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected 32-bit float scalar or vector type as Result Type: "
                "QuantizeToF16"));
}

TEST_F(ValidateConversion, QuantizeToF16WrongResultTypeBitWidth) {
  const std::string body = R"(
%val = OpQuantizeToF16 %u64 %f64_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected 32-bit float scalar or vector type as Result Type: "
                "QuantizeToF16"));
}

TEST_F(ValidateConversion, QuantizeToF16WrongInputType) {
  const std::string body = R"(
%val = OpQuantizeToF16 %f32 %f64_1
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected input type to be equal to Result Type: QuantizeToF16"));
}

TEST_F(ValidateConversion, ConvertPtrToUSuccess) {
  const std::string body = R"(
%ptr = OpVariable %f32ptr_func Function
%val1 = OpConvertPtrToU %u32 %ptr
%val2 = OpConvertPtrToU %u64 %ptr
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, ConvertPtrToUWrongResultType) {
  const std::string body = R"(
%ptr = OpVariable %f32ptr_func Function
%val = OpConvertPtrToU %f32 %ptr
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected unsigned int scalar type as Result Type: "
                        "ConvertPtrToU"));
}

TEST_F(ValidateConversion, ConvertPtrToUNotPointer) {
  const std::string body = R"(
%val = OpConvertPtrToU %u32 %f32_1
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to be a pointer: ConvertPtrToU"));
}

TEST_F(ValidateConversion, SatConvertSToUSuccess) {
  const std::string body = R"(
%val1 = OpSatConvertSToU %u32 %u64_2
%val2 = OpSatConvertSToU %u64 %u32_1
%val3 = OpSatConvertSToU %u64vec2 %u32vec2_12
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, SatConvertSToUWrongResultType) {
  const std::string body = R"(
%val = OpSatConvertSToU %f32 %u32_1
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected int scalar or vector type as Result Type: "
                        "SatConvertSToU"));
}

TEST_F(ValidateConversion, SatConvertSToUWrongInputType) {
  const std::string body = R"(
%val = OpSatConvertSToU %u32 %f32_1
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected int scalar or vector as input: SatConvertSToU"));
}

TEST_F(ValidateConversion, SatConvertSToUDifferentDimension) {
  const std::string body = R"(
%val = OpSatConvertSToU %u32 %u32vec2_12
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected input to have the same dimension as Result Type: "
                "SatConvertSToU"));
}

TEST_F(ValidateConversion, ConvertUToPtrSuccess) {
  const std::string body = R"(
%val1 = OpConvertUToPtr %f32ptr_func %u32_1
%val2 = OpConvertUToPtr %f32ptr_func %u64_1
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, ConvertUToPtrWrongResultType) {
  const std::string body = R"(
%val = OpConvertUToPtr %f32 %u32_1
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Result Type to be a pointer: ConvertUToPtr"));
}

TEST_F(ValidateConversion, ConvertUToPtrNotInt) {
  const std::string body = R"(
%val = OpConvertUToPtr %f32ptr_func %f32_1
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected int scalar as input: ConvertUToPtr"));
}

TEST_F(ValidateConversion, ConvertUToPtrNotIntScalar) {
  const std::string body = R"(
%val = OpConvertUToPtr %f32ptr_func %u32vec2_12
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected int scalar as input: ConvertUToPtr"));
}

TEST_F(ValidateConversion, PtrCastToGenericSuccess) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%val = OpPtrCastToGeneric %f32ptr_gen %ptr_func
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, PtrCastToGenericWrongResultType) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%val = OpPtrCastToGeneric %f32 %ptr_func
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected Result Type to be a pointer: PtrCastToGeneric"));
}

TEST_F(ValidateConversion, PtrCastToGenericWrongResultStorageClass) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%val = OpPtrCastToGeneric %f32ptr_func %ptr_func
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Result Type to have storage class Generic: "
                        "PtrCastToGeneric"));
}

TEST_F(ValidateConversion, PtrCastToGenericWrongInputType) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%val = OpPtrCastToGeneric %f32ptr_gen %f32
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("Operand 4[%float] cannot be a "
                                               "type"));
}

TEST_F(ValidateConversion, PtrCastToGenericWrongInputStorageClass) {
  const std::string body = R"(
%val = OpPtrCastToGeneric %f32ptr_gen %f32inp
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to have storage class Workgroup, "
                        "CrossWorkgroup or Function: PtrCastToGeneric"));
}

TEST_F(ValidateConversion, PtrCastToGenericPointToDifferentType) {
  const std::string body = R"(
%ptr_func = OpVariable %u32ptr_func Function
%val = OpPtrCastToGeneric %f32ptr_gen %ptr_func
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected input and Result Type to point to the same type: "
                "PtrCastToGeneric"));
}

TEST_F(ValidateConversion, GenericCastToPtrSuccess) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%ptr_gen = OpPtrCastToGeneric %f32ptr_gen %ptr_func
%ptr_func2 = OpGenericCastToPtr %f32ptr_func %ptr_gen
%ptr_wg = OpGenericCastToPtr %f32ptr_wg %ptr_gen
%ptr_cwg = OpGenericCastToPtr %f32ptr_cwg %ptr_gen
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, GenericCastToPtrWrongResultType) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%ptr_gen = OpPtrCastToGeneric %f32ptr_gen %ptr_func
%ptr_func2 = OpGenericCastToPtr %f32 %ptr_gen
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected Result Type to be a pointer: GenericCastToPtr"));
}

TEST_F(ValidateConversion, GenericCastToPtrWrongResultStorageClass) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%ptr_gen = OpPtrCastToGeneric %f32ptr_gen %ptr_func
%ptr_func2 = OpGenericCastToPtr %f32ptr_gen %ptr_gen
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Result Type to have storage class Workgroup, "
                        "CrossWorkgroup or Function: GenericCastToPtr"));
}

TEST_F(ValidateConversion, GenericCastToPtrWrongInputType) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%ptr_gen = OpPtrCastToGeneric %f32ptr_gen %ptr_func
%ptr_func2 = OpGenericCastToPtr %f32ptr_func %f32_1
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to be a pointer: GenericCastToPtr"));
}

TEST_F(ValidateConversion, GenericCastToPtrWrongInputStorageClass) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%ptr_func2 = OpGenericCastToPtr %f32ptr_func %ptr_func
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to have storage class Generic: "
                        "GenericCastToPtr"));
}

TEST_F(ValidateConversion, GenericCastToPtrPointToDifferentType) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%ptr_gen = OpPtrCastToGeneric %f32ptr_gen %ptr_func
%ptr_func2 = OpGenericCastToPtr %u32ptr_func %ptr_gen
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected input and Result Type to point to the same type: "
                "GenericCastToPtr"));
}

TEST_F(ValidateConversion, GenericCastToPtrExplicitSuccess) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%ptr_gen = OpPtrCastToGeneric %f32ptr_gen %ptr_func
%ptr_func2 = OpGenericCastToPtrExplicit %f32ptr_func %ptr_gen Function
%ptr_wg = OpGenericCastToPtrExplicit %f32ptr_wg %ptr_gen Workgroup
%ptr_cwg = OpGenericCastToPtrExplicit %f32ptr_cwg %ptr_gen CrossWorkgroup
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, GenericCastToPtrExplicitWrongResultType) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%ptr_gen = OpPtrCastToGeneric %f32ptr_gen %ptr_func
%ptr_func2 = OpGenericCastToPtrExplicit %f32 %ptr_gen Function
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected Result Type to be a pointer: GenericCastToPtrExplicit"));
}

TEST_F(ValidateConversion, GenericCastToPtrExplicitResultStorageClassDiffers) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%ptr_gen = OpPtrCastToGeneric %f32ptr_gen %ptr_func
%ptr_func2 = OpGenericCastToPtrExplicit %f32ptr_func %ptr_gen Workgroup
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Result Type to be of target storage class: "
                        "GenericCastToPtrExplicit"));
}

TEST_F(ValidateConversion, GenericCastToPtrExplicitWrongResultStorageClass) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%ptr_gen = OpPtrCastToGeneric %f32ptr_gen %ptr_func
%ptr_func2 = OpGenericCastToPtrExplicit %f32ptr_gen %ptr_gen Generic
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected target storage class to be Workgroup, "
                "CrossWorkgroup or Function: GenericCastToPtrExplicit"));
}

TEST_F(ValidateConversion, GenericCastToPtrExplicitWrongInputType) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%ptr_gen = OpPtrCastToGeneric %f32ptr_gen %ptr_func
%ptr_func2 = OpGenericCastToPtrExplicit %f32ptr_func %f32_1 Function
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected input to be a pointer: GenericCastToPtrExplicit"));
}

TEST_F(ValidateConversion, GenericCastToPtrExplicitWrongInputStorageClass) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%ptr_func2 = OpGenericCastToPtrExplicit %f32ptr_func %ptr_func Function
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to have storage class Generic: "
                        "GenericCastToPtrExplicit"));
}

TEST_F(ValidateConversion, GenericCastToPtrExplicitPointToDifferentType) {
  const std::string body = R"(
%ptr_func = OpVariable %f32ptr_func Function
%ptr_gen = OpPtrCastToGeneric %f32ptr_gen %ptr_func
%ptr_func2 = OpGenericCastToPtrExplicit %u32ptr_func %ptr_gen Function
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected input and Result Type to point to the same type: "
                "GenericCastToPtrExplicit"));
}

TEST_F(ValidateConversion, CoopMatConversionSuccess) {
  const std::string body =
      R"(
OpCapability Shader
OpCapability Float16
OpCapability Int16
OpCapability CooperativeMatrixNV
OpExtension "SPV_NV_cooperative_matrix"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f16 = OpTypeFloat 16
%f32 = OpTypeFloat 32
%u16 = OpTypeInt 16 0
%u32 = OpTypeInt 32 0
%s16 = OpTypeInt 16 1
%s32 = OpTypeInt 32 1

%u32_8 = OpConstant %u32 8
%subgroup = OpConstant %u32 3

%f16mat = OpTypeCooperativeMatrixNV %f16 %subgroup %u32_8 %u32_8
%f32mat = OpTypeCooperativeMatrixNV %f32 %subgroup %u32_8 %u32_8
%u16mat = OpTypeCooperativeMatrixNV %u16 %subgroup %u32_8 %u32_8
%u32mat = OpTypeCooperativeMatrixNV %u32 %subgroup %u32_8 %u32_8
%s16mat = OpTypeCooperativeMatrixNV %s16 %subgroup %u32_8 %u32_8
%s32mat = OpTypeCooperativeMatrixNV %s32 %subgroup %u32_8 %u32_8

%f16_1 = OpConstant %f16 1
%f32_1 = OpConstant %f32 1
%u16_1 = OpConstant %u16 1
%u32_1 = OpConstant %u32 1
%s16_1 = OpConstant %s16 1
%s32_1 = OpConstant %s32 1

%f16mat_1 = OpConstantComposite %f16mat %f16_1
%f32mat_1 = OpConstantComposite %f32mat %f32_1
%u16mat_1 = OpConstantComposite %u16mat %u16_1
%u32mat_1 = OpConstantComposite %u32mat %u32_1
%s16mat_1 = OpConstantComposite %s16mat %s16_1
%s32mat_1 = OpConstantComposite %s32mat %s32_1

%main = OpFunction %void None %func
%main_entry = OpLabel

%val11 = OpConvertFToU %u16mat %f16mat_1
%val12 = OpConvertFToU %u32mat %f16mat_1
%val13 = OpConvertFToS %s16mat %f16mat_1
%val14 = OpConvertFToS %s32mat %f16mat_1
%val15 = OpFConvert %f32mat %f16mat_1

%val21 = OpConvertFToU %u16mat %f32mat_1
%val22 = OpConvertFToU %u32mat %f32mat_1
%val23 = OpConvertFToS %s16mat %f32mat_1
%val24 = OpConvertFToS %s32mat %f32mat_1
%val25 = OpFConvert %f16mat %f32mat_1

%val31 = OpConvertUToF %f16mat %u16mat_1
%val32 = OpConvertUToF %f32mat %u16mat_1
%val33 = OpUConvert %u32mat %u16mat_1
%val34 = OpSConvert %s32mat %u16mat_1

%val41 = OpConvertSToF %f16mat %s16mat_1
%val42 = OpConvertSToF %f32mat %s16mat_1
%val43 = OpUConvert %u32mat %s16mat_1
%val44 = OpSConvert %s32mat %s16mat_1

OpReturn
OpFunctionEnd)";

  CompileSuccessfully(body.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, CoopMatConversionShapesMismatchFail) {
  const std::string body =
      R"(
OpCapability Shader
OpCapability Float16
OpCapability Int16
OpCapability CooperativeMatrixNV
OpExtension "SPV_NV_cooperative_matrix"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f16 = OpTypeFloat 16
%f32 = OpTypeFloat 32
%u16 = OpTypeInt 16 0
%u32 = OpTypeInt 32 0
%s16 = OpTypeInt 16 1
%s32 = OpTypeInt 32 1

%u32_8 = OpConstant %u32 8
%u32_4 = OpConstant %u32 4
%subgroup = OpConstant %u32 3

%f16mat = OpTypeCooperativeMatrixNV %f16 %subgroup %u32_8 %u32_8
%f32mat = OpTypeCooperativeMatrixNV %f32 %subgroup %u32_4 %u32_4

%f16_1 = OpConstant %f16 1

%f16mat_1 = OpConstantComposite %f16mat %f16_1

%main = OpFunction %void None %func
%main_entry = OpLabel

%val15 = OpFConvert %f32mat %f16mat_1

OpReturn
OpFunctionEnd)";

  CompileSuccessfully(body.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected rows of Matrix type and Result Type to be identical"));
}

TEST_F(ValidateConversion, CoopMatConversionShapesMismatchPass) {
  const std::string body =
      R"(
OpCapability Shader
OpCapability Float16
OpCapability Int16
OpCapability CooperativeMatrixNV
OpExtension "SPV_NV_cooperative_matrix"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f16 = OpTypeFloat 16
%f32 = OpTypeFloat 32
%u16 = OpTypeInt 16 0
%u32 = OpTypeInt 32 0
%s16 = OpTypeInt 16 1
%s32 = OpTypeInt 32 1

%u32_8 = OpConstant %u32 8
%u32_4 = OpSpecConstant %u32 4
%subgroup = OpConstant %u32 3

%f16mat = OpTypeCooperativeMatrixNV %f16 %subgroup %u32_8 %u32_8
%f32mat = OpTypeCooperativeMatrixNV %f32 %subgroup %u32_4 %u32_4

%f16_1 = OpConstant %f16 1

%f16mat_1 = OpConstantComposite %f16mat %f16_1

%main = OpFunction %void None %func
%main_entry = OpLabel

%val15 = OpFConvert %f32mat %f16mat_1

OpReturn
OpFunctionEnd)";

  CompileSuccessfully(body.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, BitcastSuccess) {
  const std::string body = R"(
%ptr = OpVariable %f32ptr_func Function
%val1 = OpBitcast %u32 %ptr
%val2 = OpBitcast %u64 %ptr
%val3 = OpBitcast %f32ptr_func %u32_1
%val4 = OpBitcast %f32ptr_wg %u64_1
%val5 = OpBitcast %f32 %u32_1
%val6 = OpBitcast %f32vec2 %u32vec2_12
%val7 = OpBitcast %f32vec2 %u64_1
%val8 = OpBitcast %f64 %u32vec2_12
%val9 = OpBitcast %f32vec4 %f64vec2_12
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, BitcastSuccessSPV1p5) {
  const std::string body = R"(
%ptr = OpVariable %f32ptr_func Function
%val1 = OpBitcast %u32 %ptr
%val2 = OpBitcast %u64 %ptr
%val3 = OpBitcast %f32ptr_func %u32_1
%val4 = OpBitcast %f32ptr_wg %u64_1
%val5 = OpBitcast %f32 %u32_1
%val6 = OpBitcast %f32vec2 %u32vec2_12
%val7 = OpBitcast %f32vec2 %u64_1
%val8 = OpBitcast %f64 %u32vec2_12
%val9 = OpBitcast %f32vec4 %f64vec2_12
%val10 = OpBitcast %u32ptr_func %u32vec2_01
%val11 = OpBitcast %u32vec2 %ptr
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str(), SPV_ENV_UNIVERSAL_1_5);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
}

TEST_F(ValidateConversion, BitcastSuccessPhysicalStorageBufferKHR) {
  const std::string body = R"(
%ptr = OpVariable %f32ptr_func Function
%val1 = OpBitcast %u32 %ptr
%val2 = OpBitcast %u64 %ptr
%val3 = OpBitcast %f32ptr_func %u32_1
%val4 = OpBitcast %f32ptr_wg %u64_1
%val5 = OpBitcast %f32 %u32_1
%val6 = OpBitcast %f32vec2 %u32vec2_12
%val7 = OpBitcast %f32vec2 %u64_1
%val8 = OpBitcast %f64 %u32vec2_12
%val9 = OpBitcast %f32vec4 %f64vec2_12
%val10 = OpBitcast %u32ptr_func %u32vec2_01
%val11 = OpBitcast %u32vec2 %ptr
)";

  CompileSuccessfully(
      GenerateKernelCode(body,
                         "\nOpExtension \"SPV_KHR_physical_storage_buffer\"")
          .c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, BitcastInputHasNoType) {
  const std::string body = R"(
%val = OpBitcast %u32 %f32
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("Operand 4[%float] cannot be a "
                                               "type"));
}

TEST_F(ValidateConversion, BitcastWrongResultType) {
  const std::string body = R"(
%val = OpBitcast %bool %f32_1
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected Result Type to be a pointer or int or float vector "
                "or scalar type: Bitcast"));
}

TEST_F(ValidateConversion, BitcastWrongInputType) {
  const std::string body = R"(
%val = OpBitcast %u32 %true
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to be a pointer or int or float vector "
                        "or scalar: Bitcast"));
}

TEST_F(ValidateConversion, BitcastPtrWrongInputType) {
  const std::string body = R"(
%val = OpBitcast %u32ptr_func %f32_1
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to be a pointer or int scalar if "
                        "Result Type is pointer: Bitcast"));
}

TEST_F(ValidateConversion, BitcastPtrWrongInputTypeSPV1p5) {
  const std::string body = R"(
%val = OpBitcast %u32ptr_func %f32_1
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str(), SPV_ENV_UNIVERSAL_1_5);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to be a pointer, int scalar or 32-bit "
                        "int vector if Result Type is pointer: Bitcast"));
}

TEST_F(ValidateConversion, BitcastPtrWrongInputTypePhysicalStorageBufferKHR) {
  const std::string body = R"(
%val = OpBitcast %u32ptr_func %f32_1
)";

  CompileSuccessfully(
      GenerateKernelCode(body,
                         "\nOpExtension \"SPV_KHR_physical_storage_buffer\"")
          .c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to be a pointer, int scalar or 32-bit "
                        "int vector if Result Type is pointer: Bitcast"));
}

TEST_F(ValidateConversion, BitcastPtrWrongInputTypeIntVectorSPV1p5) {
  const std::string body = R"(
%val = OpBitcast %u32ptr_func %u64vec2_01
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str(), SPV_ENV_UNIVERSAL_1_5);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to be a pointer, int scalar or 32-bit "
                        "int vector if Result Type is pointer: Bitcast"));
}

TEST_F(ValidateConversion,
       BitcastPtrWrongInputTypeIntVectorPhysicalStorageBufferKHR) {
  const std::string body = R"(
%val = OpBitcast %u32ptr_func %u64vec2_01
)";

  CompileSuccessfully(
      GenerateKernelCode(body,
                         "\nOpExtension \"SPV_KHR_physical_storage_buffer\"")
          .c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to be a pointer, int scalar or 32-bit "
                        "int vector if Result Type is pointer: Bitcast"));
}

TEST_F(ValidateConversion, BitcastPtrWrongResultType) {
  const std::string body = R"(
%val = OpBitcast %f32 %f32inp
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Pointer can only be converted to another pointer or "
                        "int scalar: Bitcast"));
}

TEST_F(ValidateConversion, BitcastPtrWrongResultTypeSPV1p5) {
  const std::string body = R"(
%val = OpBitcast %f32 %f32inp
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str(), SPV_ENV_UNIVERSAL_1_5);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Pointer can only be converted to another pointer, int "
                        "scalar or 32-bit int vector: Bitcast"));
}

TEST_F(ValidateConversion, BitcastPtrWrongResultTypePhysicalStorageBufferKHR) {
  const std::string body = R"(
%val = OpBitcast %f32 %f32inp
)";

  CompileSuccessfully(
      GenerateKernelCode(body,
                         "\nOpExtension \"SPV_KHR_physical_storage_buffer\"")
          .c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Pointer can only be converted to another pointer, int "
                        "scalar or 32-bit int vector: Bitcast"));
}

TEST_F(ValidateConversion, BitcastPtrWrongResultTypeIntVectorSPV1p5) {
  const std::string body = R"(
%val = OpBitcast %u64vec2 %f32inp
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str(), SPV_ENV_UNIVERSAL_1_5);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Pointer can only be converted to another pointer, int "
                        "scalar or 32-bit int vector: Bitcast"));
}

TEST_F(ValidateConversion,
       BitcastPtrWrongResultTypeIntVectorPhysicalStorageBufferKHR) {
  const std::string body = R"(
%val = OpBitcast %u64vec2 %f32inp
)";

  CompileSuccessfully(
      GenerateKernelCode(body,
                         "\nOpExtension \"SPV_KHR_physical_storage_buffer\"")
          .c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Pointer can only be converted to another pointer, int "
                        "scalar or 32-bit int vector: Bitcast"));
}

TEST_F(ValidateConversion, BitcastDifferentTotalBitWidth) {
  const std::string body = R"(
%val = OpBitcast %f32 %u64_1
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected input to have the same total bit width as Result Type: "
          "Bitcast"));
}

TEST_F(ValidateConversion, ConvertUToPtrInputIsAType) {
  const std::string spirv = R"(
OpCapability Addresses
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
%int = OpTypeInt 32 0
%ptr_int = OpTypePointer Function %int
%void = OpTypeVoid
%voidfn = OpTypeFunction %void
%func = OpFunction %void None %voidfn
%entry = OpLabel
%1 = OpConvertUToPtr %ptr_int %int
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("Operand 1[%uint] cannot be a "
                                               "type"));
}

TEST_F(ValidateConversion, ConvertUToPtrPSBSuccess) {
  const std::string body = R"(
OpCapability PhysicalStorageBufferAddresses
OpCapability Int64
OpCapability Shader
OpExtension "SPV_EXT_physical_storage_buffer"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%uint64 = OpTypeInt 64 0
%u64_1 = OpConstant %uint64 1
%ptr = OpTypePointer PhysicalStorageBuffer %uint64
%void = OpTypeVoid
%voidfn = OpTypeFunction %void
%main = OpFunction %void None %voidfn
%entry = OpLabel
%val1 = OpConvertUToPtr %ptr %u64_1
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(body.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, ConvertUToPtrPSBStorageClass) {
  const std::string body = R"(
OpCapability PhysicalStorageBufferAddresses
OpCapability Int64
OpCapability Shader
OpExtension "SPV_EXT_physical_storage_buffer"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%uint64 = OpTypeInt 64 0
%u64_1 = OpConstant %uint64 1
%ptr = OpTypePointer Function %uint64
%void = OpTypeVoid
%voidfn = OpTypeFunction %void
%main = OpFunction %void None %voidfn
%entry = OpLabel
%val1 = OpConvertUToPtr %ptr %u64_1
%val2 = OpConvertPtrToU %uint64 %val1
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(body.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Pointer storage class must be "
                        "PhysicalStorageBuffer: ConvertUToPtr"));
}

TEST_F(ValidateConversion, ConvertUToPtrVulkanWrongWidth) {
  const std::string body = R"(
OpCapability PhysicalStorageBufferAddresses
OpCapability Int64
OpCapability Shader
OpExtension "SPV_EXT_physical_storage_buffer"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%uint32 = OpTypeInt 32 0
%uint64 = OpTypeInt 64 0
%u32_1 = OpConstant %uint32 1
%ptr = OpTypePointer PhysicalStorageBuffer %uint64
%void = OpTypeVoid
%voidfn = OpTypeFunction %void
%main = OpFunction %void None %voidfn
%entry = OpLabel
%val1 = OpConvertUToPtr %ptr %u32_1
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(body.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-PhysicalStorageBuffer64-04710"));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("PhysicalStorageBuffer64 addressing mode requires the input "
                "integer to have a 64-bit width for Vulkan environment."));
}

TEST_F(ValidateConversion, ConvertPtrToUPSBSuccess) {
  const std::string body = R"(
OpCapability PhysicalStorageBufferAddresses
OpCapability Int64
OpCapability Shader
OpExtension "SPV_EXT_physical_storage_buffer"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpDecorate %val1 RestrictPointerEXT
%uint64 = OpTypeInt 64 0
%u64_1 = OpConstant %uint64 1
%ptr = OpTypePointer PhysicalStorageBuffer %uint64
%pptr_f = OpTypePointer Function %ptr
%void = OpTypeVoid
%voidfn = OpTypeFunction %void
%main = OpFunction %void None %voidfn
%entry = OpLabel
%val1 = OpVariable %pptr_f Function
%val2 = OpLoad %ptr %val1
%val3 = OpConvertPtrToU %uint64 %val2
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(body.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConversion, ConvertPtrToUPSBStorageClass) {
  const std::string body = R"(
OpCapability PhysicalStorageBufferAddresses
OpCapability Int64
OpCapability Shader
OpExtension "SPV_EXT_physical_storage_buffer"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%uint64 = OpTypeInt 64 0
%u64_1 = OpConstant %uint64 1
%ptr = OpTypePointer Function %uint64
%void = OpTypeVoid
%voidfn = OpTypeFunction %void
%main = OpFunction %void None %voidfn
%entry = OpLabel
%val1 = OpVariable %ptr Function
%val2 = OpConvertPtrToU %uint64 %val1
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(body.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Pointer storage class must be "
                        "PhysicalStorageBuffer: ConvertPtrToU"));
}

TEST_F(ValidateConversion, ConvertPtrToUVulkanWrongWidth) {
  const std::string body = R"(
OpCapability PhysicalStorageBufferAddresses
OpCapability Int64
OpCapability Shader
OpExtension "SPV_EXT_physical_storage_buffer"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpDecorate %val1 RestrictPointerEXT
%uint32 = OpTypeInt 32 0
%uint64 = OpTypeInt 64 0
%ptr = OpTypePointer PhysicalStorageBuffer %uint64
%pptr_f = OpTypePointer Function %ptr
%void = OpTypeVoid
%voidfn = OpTypeFunction %void
%main = OpFunction %void None %voidfn
%entry = OpLabel
%val1 = OpVariable %pptr_f Function
%val2 = OpLoad %ptr %val1
%val3 = OpConvertPtrToU %uint32 %val2
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(body.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-PhysicalStorageBuffer64-04710"));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("PhysicalStorageBuffer64 addressing mode requires the result "
                "integer type to have a 64-bit width for Vulkan environment."));
}

using ValidateSmallConversions = spvtest::ValidateBase<std::string>;

CodeGenerator GetSmallConversionsCodeGenerator() {
  CodeGenerator generator;
  generator.capabilities_ = R"(
OpCapability Shader
OpCapability Linkage
OpCapability UniformAndStorageBuffer16BitAccess
OpCapability UniformAndStorageBuffer8BitAccess
)";
  generator.extensions_ = R"(
OpExtension "SPV_KHR_16bit_storage"
OpExtension "SPV_KHR_8bit_storage"
)";
  generator.memory_model_ = "OpMemoryModel Logical GLSL450\n";
  generator.before_types_ = R"(
OpDecorate %char_block Block
OpMemberDecorate %char_block 0 Offset 0
OpDecorate %short_block Block
OpMemberDecorate %short_block 0 Offset 0
OpDecorate %half_block Block
OpMemberDecorate %half_block 0 Offset 0
OpDecorate %int_block Block
OpMemberDecorate %int_block 0 Offset 0
OpDecorate %float_block Block
OpMemberDecorate %float_block 0 Offset 0
)";
  generator.types_ = R"(
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int2 = OpTypeVector %int 2
%float = OpTypeFloat 32
%float_0 = OpConstant %float 0
%float2 = OpTypeVector %float 2
%char = OpTypeInt 8 0
%char2 = OpTypeVector %char 2
%short = OpTypeInt 16 0
%short2 = OpTypeVector %short 2
%half = OpTypeFloat 16
%half2 = OpTypeVector %half 2
%char_block = OpTypeStruct %char2
%short_block = OpTypeStruct %short2
%half_block = OpTypeStruct %half2
%int_block = OpTypeStruct %int2
%float_block = OpTypeStruct %float2
%ptr_ssbo_char_block = OpTypePointer StorageBuffer %char_block
%ptr_ssbo_char2 = OpTypePointer StorageBuffer %char2
%ptr_ssbo_char = OpTypePointer StorageBuffer %char
%ptr_ssbo_short_block = OpTypePointer StorageBuffer %short_block
%ptr_ssbo_short2 = OpTypePointer StorageBuffer %short2
%ptr_ssbo_short = OpTypePointer StorageBuffer %short
%ptr_ssbo_half_block = OpTypePointer StorageBuffer %half_block
%ptr_ssbo_half2 = OpTypePointer StorageBuffer %half2
%ptr_ssbo_half = OpTypePointer StorageBuffer %half
%ptr_ssbo_int_block = OpTypePointer StorageBuffer %int_block
%ptr_ssbo_int2 = OpTypePointer StorageBuffer %int2
%ptr_ssbo_int = OpTypePointer StorageBuffer %int
%ptr_ssbo_float_block = OpTypePointer StorageBuffer %float_block
%ptr_ssbo_float2 = OpTypePointer StorageBuffer %float2
%ptr_ssbo_float = OpTypePointer StorageBuffer %float
%void_fn = OpTypeFunction %void
%char_var = OpVariable %ptr_ssbo_char_block StorageBuffer
%short_var = OpVariable %ptr_ssbo_short_block StorageBuffer
%half_var = OpVariable %ptr_ssbo_half_block StorageBuffer
%int_var = OpVariable %ptr_ssbo_int_block StorageBuffer
%float_var = OpVariable %ptr_ssbo_float_block StorageBuffer
)";
  generator.after_types_ = R"(
%func = OpFunction %void None %void_fn
%entry = OpLabel
%char2_gep = OpAccessChain %ptr_ssbo_char2 %char_var %int_0
%ld_char2 = OpLoad %char2 %char2_gep
%char_gep = OpAccessChain %ptr_ssbo_char %char_var %int_0 %int_0
%ld_char = OpLoad %char %char_gep
%short2_gep = OpAccessChain %ptr_ssbo_short2 %short_var %int_0
%ld_short2 = OpLoad %short2 %short2_gep
%short_gep = OpAccessChain %ptr_ssbo_short %short_var %int_0 %int_0
%ld_short = OpLoad %short %short_gep
%half2_gep = OpAccessChain %ptr_ssbo_half2 %half_var %int_0
%ld_half2 = OpLoad %half2 %half2_gep
%half_gep = OpAccessChain %ptr_ssbo_half %half_var %int_0 %int_0
%ld_half = OpLoad %half %half_gep
%int2_gep = OpAccessChain %ptr_ssbo_int2 %int_var %int_0
%ld_int2 = OpLoad %int2 %int2_gep
%int_gep = OpAccessChain %ptr_ssbo_int %int_var %int_0 %int_0
%ld_int = OpLoad %int %int_gep
%float2_gep = OpAccessChain %ptr_ssbo_float2 %float_var %int_0
%ld_float2 = OpLoad %float2 %float2_gep
%float_gep = OpAccessChain %ptr_ssbo_float %float_var %int_0 %int_0
%ld_float = OpLoad %float %float_gep
)";
  generator.add_at_the_end_ = R"(
OpReturn
OpFunctionEnd
)";
  return generator;
}

TEST_P(ValidateSmallConversions, Instruction) {
  CodeGenerator generator = GetSmallConversionsCodeGenerator();
  generator.after_types_ += GetParam() + "\n";
  CompileSuccessfully(generator.Build(), SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "8- or 16-bit types can only be used with width-only conversions"));
}

INSTANTIATE_TEST_SUITE_P(SmallConversionInstructions, ValidateSmallConversions,
                         Values("%inst = OpConvertFToU %char %ld_float",
                                "%inst = OpConvertFToU %char2 %ld_float2",
                                "%inst = OpConvertFToU %short %ld_float",
                                "%inst = OpConvertFToU %short2 %ld_float2",
                                "%inst = OpConvertFToU %int %ld_half",
                                "%inst = OpConvertFToU %int2 %ld_half2",
                                "%inst = OpConvertFToS %char %ld_float",
                                "%inst = OpConvertFToS %char2 %ld_float2",
                                "%inst = OpConvertFToS %short %ld_float",
                                "%inst = OpConvertFToS %short2 %ld_float2",
                                "%inst = OpConvertFToS %int %ld_half",
                                "%inst = OpConvertFToS %int2 %ld_half2",
                                "%inst = OpConvertSToF %float %ld_char",
                                "%inst = OpConvertSToF %float2 %ld_char2",
                                "%inst = OpConvertSToF %float %ld_short",
                                "%inst = OpConvertSToF %float2 %ld_short2",
                                "%inst = OpConvertSToF %half %ld_int",
                                "%inst = OpConvertSToF %half2 %ld_int2",
                                "%inst = OpConvertUToF %float %ld_char",
                                "%inst = OpConvertUToF %float2 %ld_char2",
                                "%inst = OpConvertUToF %float %ld_short",
                                "%inst = OpConvertUToF %float2 %ld_short2",
                                "%inst = OpConvertUToF %half %ld_int",
                                "%inst = OpConvertUToF %half2 %ld_int2",
                                "%inst = OpBitcast %half %ld_short",
                                "%inst = OpBitcast %half2 %ld_short2",
                                "%inst = OpBitcast %short %ld_half",
                                "%inst = OpBitcast %short2 %ld_half2"));

}  // namespace
}  // namespace val
}  // namespace spvtools
