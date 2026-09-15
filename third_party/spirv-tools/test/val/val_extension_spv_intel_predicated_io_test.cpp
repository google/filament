// Copyright (c) 2026 The Khronos Group Inc.
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

// Tests for SPV_INTEL_predicated_io extension

#include <string>

#include "gmock/gmock.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;

using ValidatePredicatedIOINTEL = spvtest::ValidateBase<bool>;

std::string GenerateShaderCode(const std::string& body,
                               const std::string& extra_types_and_vars = "") {
  std::ostringstream ss;
  ss << R"(
OpCapability Shader
OpCapability Linkage
OpCapability PredicatedIOINTEL
OpExtension "SPV_INTEL_predicated_io"
OpMemoryModel Logical GLSL450
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%bool = OpTypeBool
%int = OpTypeInt 32 1
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%int_ptr_func = OpTypePointer Function %int
%float_ptr_func = OpTypePointer Function %float
%v4float_ptr_func = OpTypePointer Function %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_0 = OpConstant %float 0.0
%float_1 = OpConstant %float 1.0
%v4float_0 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
)";
  ss << extra_types_and_vars;
  ss << R"(
%func = OpFunction %void None %void_fn
%entry = OpLabel
)";
  ss << body;
  ss << R"(
OpReturn
OpFunctionEnd
)";
  return ss.str();
}

// ---- OpPredicatedLoadINTEL valid cases ----

TEST_F(ValidatePredicatedIOINTEL, PredicatedLoadScalarFloatSuccess) {
  const std::string body = R"(
%ptr = OpVariable %float_ptr_func Function
%val = OpPredicatedLoadINTEL %float %ptr %true %float_0
)";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidatePredicatedIOINTEL, PredicatedLoadScalarIntSuccess) {
  const std::string body = R"(
%ptr = OpVariable %int_ptr_func Function
%val = OpPredicatedLoadINTEL %int %ptr %true %int_0
)";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidatePredicatedIOINTEL, PredicatedLoadVectorFloatSuccess) {
  const std::string body = R"(
%ptr = OpVariable %v4float_ptr_func Function
%val = OpPredicatedLoadINTEL %v4float %ptr %true %v4float_0
)";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidatePredicatedIOINTEL, PredicatedLoadWithAlignedSuccess) {
  const std::string body = R"(
%ptr = OpVariable %float_ptr_func Function
%val = OpPredicatedLoadINTEL %float %ptr %true %float_0 Aligned 4
)";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidatePredicatedIOINTEL, PredicatedLoadFalsePredicateSuccess) {
  const std::string body = R"(
%ptr = OpVariable %float_ptr_func Function
%val = OpPredicatedLoadINTEL %float %ptr %false %float_1
)";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

// ---- OpPredicatedLoadINTEL error cases ----

TEST_F(ValidatePredicatedIOINTEL, PredicatedLoadBoolResultType) {
  const std::string extra = R"(
%bool_ptr_func = OpTypePointer Function %bool
)";
  const std::string body = R"(
%ptr = OpVariable %bool_ptr_func Function
%val = OpPredicatedLoadINTEL %bool %ptr %true %true
)";
  CompileSuccessfully(GenerateShaderCode(body, extra));
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be a scalar or vector of numerical type"));
}

TEST_F(ValidatePredicatedIOINTEL, PredicatedLoadStructResultType) {
  const std::string extra = R"(
%struct_type = OpTypeStruct %int %float
%struct_ptr_func = OpTypePointer Function %struct_type
%struct_default = OpConstantComposite %struct_type %int_0 %float_0
)";
  const std::string body = R"(
%ptr = OpVariable %struct_ptr_func Function
%val = OpPredicatedLoadINTEL %struct_type %ptr %true %struct_default
)";
  CompileSuccessfully(GenerateShaderCode(body, extra));
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be a scalar or vector of numerical type"));
}

TEST_F(ValidatePredicatedIOINTEL, PredicatedLoadPredicateNotBool) {
  const std::string body = R"(
%ptr = OpVariable %float_ptr_func Function
%val = OpPredicatedLoadINTEL %float %ptr %int_0 %float_0
)";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("Predicate"));
  EXPECT_THAT(getDiagnosticString(), HasSubstr("must be a Boolean scalar"));
}

TEST_F(ValidatePredicatedIOINTEL, PredicatedLoadDefaultValueTypeMismatch) {
  const std::string body = R"(
%ptr = OpVariable %float_ptr_func Function
%val = OpPredicatedLoadINTEL %float %ptr %true %int_0
)";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("Default Value"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("type does not match Result Type"));
}

TEST_F(ValidatePredicatedIOINTEL, PredicatedLoadPointerTypeMismatch) {
  const std::string body = R"(
%ptr = OpVariable %int_ptr_func Function
%val = OpPredicatedLoadINTEL %float %ptr %true %float_0
)";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("does not match Pointer"));
}

TEST_F(ValidatePredicatedIOINTEL, PredicatedLoadVolatileNotAllowed) {
  const std::string body = R"(
%ptr = OpVariable %float_ptr_func Function
%val = OpPredicatedLoadINTEL %float %ptr %true %float_0 Volatile
)";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("does not allow the Volatile memory operand"));
}

// ---- OpPredicatedStoreINTEL valid cases ----

TEST_F(ValidatePredicatedIOINTEL, PredicatedStoreScalarFloatSuccess) {
  const std::string body = R"(
%ptr = OpVariable %float_ptr_func Function
OpPredicatedStoreINTEL %ptr %float_1 %true
)";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidatePredicatedIOINTEL, PredicatedStoreScalarIntSuccess) {
  const std::string body = R"(
%ptr = OpVariable %int_ptr_func Function
OpPredicatedStoreINTEL %ptr %int_1 %true
)";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidatePredicatedIOINTEL, PredicatedStoreVectorFloatSuccess) {
  const std::string body = R"(
%ptr = OpVariable %v4float_ptr_func Function
OpPredicatedStoreINTEL %ptr %v4float_0 %true
)";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidatePredicatedIOINTEL, PredicatedStoreWithAlignedSuccess) {
  const std::string body = R"(
%ptr = OpVariable %float_ptr_func Function
OpPredicatedStoreINTEL %ptr %float_1 %true Aligned 4
)";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

// ---- OpPredicatedStoreINTEL error cases ----

TEST_F(ValidatePredicatedIOINTEL, PredicatedStoreBoolObject) {
  const std::string extra = R"(
%bool_ptr_func = OpTypePointer Function %bool
)";
  const std::string body = R"(
%ptr = OpVariable %bool_ptr_func Function
OpPredicatedStoreINTEL %ptr %true %true
)";
  CompileSuccessfully(GenerateShaderCode(body, extra));
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("type must be a scalar or vector of numerical type"));
}

TEST_F(ValidatePredicatedIOINTEL, PredicatedStorePredicateNotBool) {
  const std::string body = R"(
%ptr = OpVariable %float_ptr_func Function
OpPredicatedStoreINTEL %ptr %float_1 %int_0
)";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("Predicate"));
  EXPECT_THAT(getDiagnosticString(), HasSubstr("must be a Boolean scalar"));
}

TEST_F(ValidatePredicatedIOINTEL, PredicatedStorePointerTypeMismatch) {
  const std::string body = R"(
%ptr = OpVariable %int_ptr_func Function
OpPredicatedStoreINTEL %ptr %float_1 %true
)";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("type does not match Object"));
}

TEST_F(ValidatePredicatedIOINTEL, PredicatedStoreVolatileNotAllowed) {
  const std::string body = R"(
%ptr = OpVariable %float_ptr_func Function
OpPredicatedStoreINTEL %ptr %float_1 %true Volatile
)";
  CompileSuccessfully(GenerateShaderCode(body));
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("does not allow the Volatile memory operand"));
}

// ---- Capability/Extension requirement tests ----

TEST_F(ValidatePredicatedIOINTEL, PredicatedLoadWithoutCapability) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpExtension "SPV_INTEL_predicated_io"
OpMemoryModel Logical GLSL450
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%bool = OpTypeBool
%float = OpTypeFloat 32
%float_ptr_func = OpTypePointer Function %float
%float_0 = OpConstant %float 0.0
%true = OpConstantTrue %bool
%func = OpFunction %void None %void_fn
%entry = OpLabel
%ptr = OpVariable %float_ptr_func Function
%val = OpPredicatedLoadINTEL %float %ptr %true %float_0
OpReturn
OpFunctionEnd
)";
  CompileSuccessfully(spirv);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("PredicatedIOINTEL"));
}

TEST_F(ValidatePredicatedIOINTEL, PredicatedStoreWithoutCapability) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpExtension "SPV_INTEL_predicated_io"
OpMemoryModel Logical GLSL450
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%bool = OpTypeBool
%float = OpTypeFloat 32
%float_ptr_func = OpTypePointer Function %float
%float_1 = OpConstant %float 1.0
%true = OpConstantTrue %bool
%func = OpFunction %void None %void_fn
%entry = OpLabel
%ptr = OpVariable %float_ptr_func Function
OpPredicatedStoreINTEL %ptr %float_1 %true
OpReturn
OpFunctionEnd
)";
  CompileSuccessfully(spirv);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("PredicatedIOINTEL"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
