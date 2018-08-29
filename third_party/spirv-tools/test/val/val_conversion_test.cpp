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
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::Not;

using ValidateConversion = spvtest::ValidateBase<bool>;

std::string GenerateShaderCode(
    const std::string& body,
    const std::string& capabilities_and_extensions = "") {
  const std::string capabilities =
      R"(
OpCapability Shader
OpCapability Int64
OpCapability Float64)";

  const std::string after_extension_before_body =
      R"(
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
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

%f32ptr_func = OpTypePointer Function %f32

%main = OpFunction %void None %func
%main_entry = OpLabel)";

  const std::string after_body =
      R"(
OpReturn
OpFunctionEnd)";

  return capabilities + capabilities_and_extensions +
         after_extension_before_body + body + after_body;
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
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to be a pointer: PtrCastToGeneric"));
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

TEST_F(ValidateConversion, BitcastInputHasNoType) {
  const std::string body = R"(
%val = OpBitcast %u32 %f32
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected input to have a type: Bitcast"));
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
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected input to be a pointer or int scalar if Result Type "
                "is pointer: Bitcast"));
}

TEST_F(ValidateConversion, BitcastPtrWrongResultType) {
  const std::string body = R"(
%val = OpBitcast %f32 %f32inp
)";

  CompileSuccessfully(GenerateKernelCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Pointer can only be converted to another pointer or int scalar: "
          "Bitcast"));
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

}  // namespace
}  // namespace val
}  // namespace spvtools
