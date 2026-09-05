// Copyright (c) 2026 LunarG Inc.
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

#include <ostream>
#include <string>

#include "gmock/gmock.h"
#include "spirv-tools/libspirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;

using ValidateDotProductMixAcc = spvtest::ValidateBase<bool>;

std::string GenerateShaderCode(const std::string& body) {
  std::stringstream ss;
  ss << R"(
OpCapability Shader
OpCapability Float16
OpCapability Float64
OpCapability Float8EXT
OpCapability BFloat16TypeKHR
OpCapability DotProductFloat16AccFloat32VALVE
OpCapability DotProductFloat16AccFloat16VALVE
OpCapability DotProductBFloat16AccVALVE
OpCapability DotProductFloat8AccFloat32VALVE
OpExtension "SPV_VALVE_mixed_float_dot_product"
OpExtension "SPV_EXT_float8"
OpExtension "SPV_KHR_bfloat16"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1

%void = OpTypeVoid
%func = OpTypeFunction %void

%f32 = OpTypeFloat 32
%f16 = OpTypeFloat 16
%bf16 = OpTypeFloat 16 BFloat16KHR
%f64 = OpTypeFloat 64
%f8_e4m3 = OpTypeFloat 8 Float8E4M3EXT
%f8_e5m2 = OpTypeFloat 8 Float8E5M2EXT

%v2f32 = OpTypeVector %f32 2
%v2f16 = OpTypeVector %f16 2
%v2bf16 = OpTypeVector %bf16 2
%v3f16 = OpTypeVector %f16 3
%v2f64 = OpTypeVector %f64 2
%v4f8_e4m3 = OpTypeVector %f8_e4m3 4
%v4f8_e5m2 = OpTypeVector %f8_e5m2 4
%v4f16 = OpTypeVector %f16 4

%f32_0 = OpConstant %f32 0
%f32_1 = OpConstant %f32 1
%f16_0 = OpConstant %f16 0
%f16_1 = OpConstant %f16 1
%bf16_1 = OpConstant %bf16 1
%f8_e4m3_1 = OpConstant %f8_e4m3 1
%f8_e5m2_1 = OpConstant %f8_e5m2 1

%v2f16_0 = OpConstantComposite %v2f16 %f16_0 %f16_0
%v2f16_1 = OpConstantComposite %v2f16 %f16_1 %f16_1
%v2bf16_1 = OpConstantComposite %v2bf16 %bf16_1 %bf16_1
%v3f16_1 = OpConstantComposite %v3f16 %f16_1 %f16_1 %f16_1
%v2f32_1 = OpConstantComposite %v2f32 %f32_1 %f32_1
%v4f8_e4m3_1 = OpConstantComposite %v4f8_e4m3 %f8_e4m3_1 %f8_e4m3_1 %f8_e4m3_1 %f8_e4m3_1
%v4f8_e5m2_1 = OpConstantComposite %v4f8_e5m2 %f8_e5m2_1 %f8_e5m2_1 %f8_e5m2_1 %f8_e5m2_1
%v4f16_1 = OpConstantComposite %v4f16 %f16_1 %f16_1 %f16_1 %f16_1

%main = OpFunction %void None %func
%label = OpLabel
)";
  ss << body;
  ss << R"(
OpReturn
OpFunctionEnd
)";
  return ss.str();
}

TEST_F(ValidateDotProductMixAcc, FDot2MixAcc32Float16Good) {
  const std::string body = R"(
     %res = OpFDot2MixAcc32VALVE %f32 %v2f16_1 %v2f16_1 %f32_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateDotProductMixAcc, FDot2MixAcc32BFloat16Good) {
  const std::string body = R"(
     %res = OpFDot2MixAcc32VALVE %f32 %v2bf16_1 %v2bf16_1 %f32_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateDotProductMixAcc, FDot2MixAcc16Float16Good) {
  const std::string body = R"(
     %res = OpFDot2MixAcc16VALVE %f16 %v2f16_1 %v2f16_1 %f16_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateDotProductMixAcc, FDot2MixAcc16BFloat16Good) {
  const std::string body = R"(
     %res = OpFDot2MixAcc16VALVE %bf16 %v2bf16_1 %v2bf16_1 %bf16_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateDotProductMixAcc, FDot4MixAcc32Float8E4M3Good) {
  const std::string body = R"(
     %res = OpFDot4MixAcc32VALVE %f32 %v4f8_e4m3_1 %v4f8_e4m3_1 %f32_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateDotProductMixAcc, FDot4MixAcc32Float8E5M2Good) {
  const std::string body = R"(
     %res = OpFDot4MixAcc32VALVE %f32 %v4f8_e5m2_1 %v4f8_e5m2_1 %f32_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateDotProductMixAcc, FDot4MixAcc32Float8MixedEncodingGood) {
  const std::string body = R"(
     %res = OpFDot4MixAcc32VALVE %f32 %v4f8_e4m3_1 %v4f8_e5m2_1 %f32_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateDotProductMixAcc, CommonNotVector) {
  const std::string body = R"(
     %res = OpFDot2MixAcc32VALVE %f32 %f16_1 %v2f16_1 %f32_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected 'Vector 1' to be an vector."));
}

TEST_F(ValidateDotProductMixAcc, CommonBadLength) {
  const std::string body = R"(
     %res = OpFDot2MixAcc32VALVE %f32 %v3f16_1 %v3f16_1 %f32_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("components, but both need to be 2-components"));
}

TEST_F(ValidateDotProductMixAcc, Acc32BadResultType) {
  const std::string body = R"(
     %res = OpFDot2MixAcc32VALVE %f16 %v2f16_1 %v2f16_1 %f32_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result must be a 32-bit IEEE 754 float scalar type."));
}

TEST_F(ValidateDotProductMixAcc, Acc32BadComponentType) {
  const std::string body = R"(
     %res = OpFDot2MixAcc32VALVE %f32 %v2f32_1 %v2f32_1 %f32_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected 'Vector 1' to be a vector of 16-bit floats."));
}

TEST_F(ValidateDotProductMixAcc, Acc32BadAccType) {
  const std::string body = R"(
     %res = OpFDot2MixAcc32VALVE %f32 %v2f16_1 %v2f16_1 %f16_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Accumulator Type must be the same as the Result Type."));
}

TEST_F(ValidateDotProductMixAcc, Acc32MixedEncodingFloat16) {
  const std::string body = R"(
     %res = OpFDot2MixAcc32VALVE %f32 %v2f16_1 %v2bf16_1 %f32_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("'Vector 1' and 'Vector 2' must be the same float encoding."));
}

TEST_F(ValidateDotProductMixAcc, Acc16BadResultType) {
  const std::string body = R"(
     %res = OpFDot2MixAcc16VALVE %f32 %v2f16_1 %v2f16_1 %f16_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result must be a 16-bit float scalar type."));
}

TEST_F(ValidateDotProductMixAcc, Acc16BadAccType) {
  const std::string body = R"(
     %res = OpFDot2MixAcc16VALVE %f16 %v2f16_1 %v2f16_1 %f32_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Accumulator Type must be the same as the Result Type."));
}

TEST_F(ValidateDotProductMixAcc, Acc16MixedEncodingFloat16) {
  const std::string body = R"(
     %res = OpFDot2MixAcc16VALVE %f16 %v2f16_1 %v2bf16_1 %f16_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("'Vector 1' and 'Vector 2' must be the same float encoding."));
}

TEST_F(ValidateDotProductMixAcc, Acc16ResultWrongEncoding) {
  const std::string body = R"(
     %res = OpFDot2MixAcc16VALVE %f16 %v2bf16_1 %v2bf16_1 %f16_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result must have the same float encoding as 'Vector "
                        "1' and 'Vector 2'."));
}

TEST_F(ValidateDotProductMixAcc, Acc16AccumulatorWrongEncoding) {
  const std::string body = R"(
     %res = OpFDot2MixAcc16VALVE %bf16 %v2bf16_1 %v2bf16_1 %f16_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Accumulator Type must be the same as the Result Type."));
}

TEST_F(ValidateDotProductMixAcc, Acc32Mix4BadVectorType) {
  const std::string body = R"(
     %res = OpFDot4MixAcc32VALVE %f32 %v2f16_1 %v2f16_1 %f32_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("components, but both need to be 4-components"));
}

TEST_F(ValidateDotProductMixAcc, Acc32Mix4BadComponentType) {
  const std::string body = R"(
     %res = OpFDot4MixAcc32VALVE %f32 %v4f16_1 %v4f16_1 %f32_1
  )";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected 'Vector 1' to be a vector of 8-bit floats."));
}

TEST_F(ValidateDotProductMixAcc, Acc32MissingCapFloat16) {
  const std::string ss = R"(
OpCapability Shader
OpCapability Float16
OpCapability DotProductBFloat16AccVALVE
OpExtension "SPV_VALVE_mixed_float_dot_product"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1

%void = OpTypeVoid
%func = OpTypeFunction %void
%f32 = OpTypeFloat 32
%f16 = OpTypeFloat 16
%v2f16 = OpTypeVector %f16 2

%f32_1 = OpConstant %f32 1
%f16_1 = OpConstant %f16 1
%v2f16_1 = OpConstantComposite %v2f16 %f16_1 %f16_1

%main = OpFunction %void None %func
%label = OpLabel
%res = OpFDot2MixAcc32VALVE %f32 %v2f16_1 %v2f16_1 %f32_1
OpReturn
OpFunctionEnd
  )";
  CompileSuccessfully(ss);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("DotProductFloat16AccFloat32VALVE capability is required"));
}

TEST_F(ValidateDotProductMixAcc, Acc32MissingCapBFloat16) {
  const std::string ss = R"(
OpCapability Shader
OpCapability BFloat16TypeKHR
OpCapability DotProductFloat16AccFloat32VALVE
OpExtension "SPV_VALVE_mixed_float_dot_product"
OpExtension "SPV_KHR_bfloat16"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1

%void = OpTypeVoid
%func = OpTypeFunction %void
%f32 = OpTypeFloat 32
%bf16 = OpTypeFloat 16 BFloat16KHR
%v2bf16 = OpTypeVector %bf16 2

%f32_1 = OpConstant %f32 1
%bf16_1 = OpConstant %bf16 1
%v2bf16_1 = OpConstantComposite %v2bf16 %bf16_1 %bf16_1

%main = OpFunction %void None %func
%label = OpLabel
%res = OpFDot2MixAcc32VALVE %f32 %v2bf16_1 %v2bf16_1 %f32_1
OpReturn
OpFunctionEnd
  )";
  CompileSuccessfully(ss);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("DotProductBFloat16AccVALVE capability is required"));
}

TEST_F(ValidateDotProductMixAcc, Acc16MissingCapFloat16) {
  const std::string ss = R"(
OpCapability Shader
OpCapability Float16
OpCapability DotProductBFloat16AccVALVE
OpExtension "SPV_VALVE_mixed_float_dot_product"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1

%void = OpTypeVoid
%func = OpTypeFunction %void
%f16 = OpTypeFloat 16
%v2f16 = OpTypeVector %f16 2

%f16_1 = OpConstant %f16 1
%v2f16_1 = OpConstantComposite %v2f16 %f16_1 %f16_1

%main = OpFunction %void None %func
%label = OpLabel
%res = OpFDot2MixAcc16VALVE %f16 %v2f16_1 %v2f16_1 %f16_1
OpReturn
OpFunctionEnd
  )";
  CompileSuccessfully(ss);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("DotProductFloat16AccFloat16VALVE capability is required"));
}

TEST_F(ValidateDotProductMixAcc, Acc16MissingCapBFloat16) {
  const std::string ss = R"(
OpCapability Shader
OpCapability BFloat16TypeKHR
OpCapability DotProductFloat16AccFloat16VALVE
OpExtension "SPV_VALVE_mixed_float_dot_product"
OpExtension "SPV_KHR_bfloat16"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1

%void = OpTypeVoid
%func = OpTypeFunction %void
%bf16 = OpTypeFloat 16 BFloat16KHR
%v2bf16 = OpTypeVector %bf16 2

%bf16_1 = OpConstant %bf16 1
%v2bf16_1 = OpConstantComposite %v2bf16 %bf16_1 %bf16_1

%main = OpFunction %void None %func
%label = OpLabel
%res = OpFDot2MixAcc16VALVE %bf16 %v2bf16_1 %v2bf16_1 %bf16_1
OpReturn
OpFunctionEnd
  )";
  CompileSuccessfully(ss);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("DotProductBFloat16AccVALVE capability is required"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
