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

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::Combine;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Values;
using ::testing::ValuesIn;

using ValidateLogicalPointersTest = spvtest::ValidateBase<bool>;

enum class MatrixTrace : uint32_t {
  kNotAMatrix,
  kColumn,
  kComponent,
};

const MatrixTrace traces[] = {MatrixTrace::kNotAMatrix, MatrixTrace::kColumn,
                              MatrixTrace::kComponent};

using MatrixTraceTypedTest = spvtest::ValidateBase<MatrixTrace>;

TEST_P(MatrixTraceTypedTest, PhiLoopOp1) {
  const auto trace_type = GetParam();
  std::string gep, type;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      type = "%ptr_wg_v2float";
      gep = "OpAccessChain %ptr_wg_v2float %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_0 %index %index %index";
      break;
    default:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg_struct = OpTypePointer Workgroup %struct
%ptr_wg_v2float = OpTypePointer Workgroup %v2float
%ptr_wg_float = OpTypePointer Workgroup %float
%null = OpConstantNull )" + type +
                            R"(
%var = OpVariable %ptr_wg_struct Workgroup
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = )" + gep + R"(
OpBranch %loop
%loop = OpLabel
%phi = OpPhi )" + type + R"( %gep %entry %copy %continue
OpLoopMerge %merge %continue None
OpBranchConditional %bool_cond %merge %continue
%continue = OpLabel
%copy = OpCopyObject )" + type +
                            R"( %phi
OpBranch %loop
%merge = OpLabel
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceTypedTest, PhiLoopOp2) {
  const auto trace_type = GetParam();
  std::string gep, type;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      type = "%ptr_wg_v2float";
      gep = "OpAccessChain %ptr_wg_v2float %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_0 %index %index %index";
      break;
    default:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg_struct = OpTypePointer Workgroup %struct
%ptr_wg_v2float = OpTypePointer Workgroup %v2float
%ptr_wg_float = OpTypePointer Workgroup %float
%null = OpConstantNull )" + type +
                            R"(
%var = OpVariable %ptr_wg_struct Workgroup
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = )" + gep + R"(
OpBranch %loop
%loop = OpLabel
%phi = OpPhi )" + type + R"( %copy %continue %gep %entry
OpLoopMerge %merge %continue None
OpBranchConditional %bool_cond %merge %continue
%continue = OpLabel
%copy = OpCopyObject )" + type +
                            R"( %phi
OpBranch %loop
%merge = OpLabel
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceTypedTest, SelectOp1) {
  const auto trace_type = GetParam();
  std::string gep, type;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      type = "%ptr_wg_v2float";
      gep = "OpAccessChain %ptr_wg_v2float %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_0 %index %index %index";
      break;
    default:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg_struct = OpTypePointer Workgroup %struct
%ptr_wg_v2float = OpTypePointer Workgroup %v2float
%ptr_wg_float = OpTypePointer Workgroup %float
%null = OpConstantNull )" + type +
                            R"(
%var = OpVariable %ptr_wg_struct Workgroup
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = )" + gep + R"(
%copy = OpCopyObject )" + type +
                            R"( %gep
%sel = OpSelect )" + type + R"( %bool_cond %copy %null
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceTypedTest, SelectOp2) {
  const auto trace_type = GetParam();
  std::string gep, type;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      type = "%ptr_wg_v2float";
      gep = "OpAccessChain %ptr_wg_v2float %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_0 %index %index %index";
      break;
    default:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg_struct = OpTypePointer Workgroup %struct
%ptr_wg_v2float = OpTypePointer Workgroup %v2float
%ptr_wg_float = OpTypePointer Workgroup %float
%null = OpConstantNull )" + type +
                            R"(
%var = OpVariable %ptr_wg_struct Workgroup
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = )" + gep + R"(
%copy = OpCopyObject )" + type +
                            R"( %gep
%sel = OpSelect )" + type + R"( %bool_cond %null %copy
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceTypedTest, FunctionVariable) {
  const auto trace_type = GetParam();
  std::string gep, type, ld_type;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      type = "%ptr_func_wg_v2float";
      ld_type = "%ptr_wg_v2float";
      gep = "OpAccessChain %ptr_wg_v2float %var_mat2x2 %index";
      break;
    case MatrixTrace::kComponent:
      type = "%ptr_func_wg_float";
      ld_type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var_mat2x2 %index %index";
      break;
    default:
      type = "%ptr_func_wg_float";
      ld_type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var_float";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg_float = OpTypePointer Workgroup %float
%ptr_wg_v2float = OpTypePointer Workgroup %v2float
%ptr_wg_mat2x2 = OpTypePointer Workgroup %mat2x2
%ptr_func_wg_float = OpTypePointer Function %ptr_wg_float
%ptr_func_wg_v2float = OpTypePointer Function %ptr_wg_v2float
%var_mat2x2 = OpVariable %ptr_wg_mat2x2 Workgroup
%var_float = OpVariable %ptr_wg_float Workgroup
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%func_var = OpVariable )" + type +
                            R"( Function
%gep = )" + gep + R"(
OpStore %func_var %gep
%ld = OpLoad )" + ld_type + R"( %func_var
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceTypedTest, PrivateVariable) {
  const auto trace_type = GetParam();
  std::string gep, type, ld_type;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      type = "%ptr_priv_wg_v2float";
      ld_type = "%ptr_wg_v2float";
      gep = "OpAccessChain %ptr_wg_v2float %var_mat2x2 %index";
      break;
    case MatrixTrace::kComponent:
      type = "%ptr_priv_wg_float";
      ld_type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var_mat2x2 %index %index";
      break;
    default:
      type = "%ptr_priv_wg_float";
      ld_type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var_float";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg_float = OpTypePointer Workgroup %float
%ptr_wg_v2float = OpTypePointer Workgroup %v2float
%ptr_wg_mat2x2 = OpTypePointer Workgroup %mat2x2
%ptr_priv_wg_float = OpTypePointer Private %ptr_wg_float
%ptr_priv_wg_v2float = OpTypePointer Private %ptr_wg_v2float
%var_mat2x2 = OpVariable %ptr_wg_mat2x2 Workgroup
%var_float = OpVariable %ptr_wg_float Workgroup
%priv_var = OpVariable )" + type +
                            R"( Private
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = )" + gep + R"(
OpStore %priv_var %gep
%ld = OpLoad )" + ld_type + R"( %priv_var
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceTypedTest, FunctionVariableAggregate) {
  const auto trace_type = GetParam();
  std::string gep, var_type, gep_type, ld_type;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      var_type = "%ptr_func_wg_struct_v2float";
      gep_type = "%ptr_func_wg_v2float";
      ld_type = "%ptr_wg_v2float";
      gep = "OpAccessChain %ptr_wg_v2float %var_mat2x2 %index";
      break;
    case MatrixTrace::kComponent:
      var_type = "%ptr_func_wg_struct_float";
      gep_type = "%ptr_func_wg_float";
      ld_type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var_mat2x2 %index %index";
      break;
    default:
      var_type = "%ptr_func_wg_struct_float";
      gep_type = "%ptr_func_wg_float";
      ld_type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var_float";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg_float = OpTypePointer Workgroup %float
%ptr_wg_v2float = OpTypePointer Workgroup %v2float
%ptr_wg_mat2x2 = OpTypePointer Workgroup %mat2x2
%func_array_float = OpTypeArray %ptr_wg_float %int_1
%func_array_v2float = OpTypeArray %ptr_wg_v2float %int_1
%func_struct_float = OpTypeStruct %func_array_float
%func_struct_v2float = OpTypeStruct %func_array_v2float
%ptr_func_wg_struct_float = OpTypePointer Function %func_struct_float
%ptr_func_wg_struct_v2float = OpTypePointer Function %func_struct_v2float
%ptr_func_wg_float = OpTypePointer Function %ptr_wg_float
%ptr_func_wg_v2float = OpTypePointer Function %ptr_wg_v2float
%var_mat2x2 = OpVariable %ptr_wg_mat2x2 Workgroup
%var_float = OpVariable %ptr_wg_float Workgroup
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%func_var = OpVariable )" + var_type +
                            R"( Function
%gep = )" + gep + R"(
%store_gep = OpAccessChain )" +
                            gep_type + R"( %func_var %int_0 %index
%store_gep_copy = OpCopyObject )" +
                            gep_type + R"( %store_gep
OpStore %store_gep_copy %gep
%ld_gep = OpAccessChain )" + gep_type +
                            R"( %func_var %int_0 %index
%ld = OpLoad )" + ld_type + R"( %ld_gep
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceTypedTest, FunctionCallParam1) {
  const auto trace_type = GetParam();
  std::string gep, type;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      type = "%ptr_wg_v2float";
      gep = "OpAccessChain %ptr_wg_v2float %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_0 %index %index %index";
      break;
    default:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg_struct = OpTypePointer Workgroup %struct
%ptr_wg_v2float = OpTypePointer Workgroup %v2float
%ptr_wg_float = OpTypePointer Workgroup %float
%null = OpConstantNull )" + type +
                            R"(
%var = OpVariable %ptr_wg_struct Workgroup
%void_fn = OpTypeFunction %void
%foo_fn = OpTypeFunction %void )" +
                            type + R"( %bool
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = )" + gep + R"(
%copy = OpCopyObject )" + type +
                            R"( %gep
%call = OpFunctionCall %void %foo %copy %bool_cond
OpReturn
OpFunctionEnd
%foo = OpFunction %void None %foo_fn
%ptr_param = OpFunctionParameter )" +
                            type + R"(
%bool_param = OpFunctionParameter %bool
%foo_entry = OpLabel
%sel = OpSelect )" + type + R"( %bool_param %ptr_param %null
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceTypedTest, FunctionCallParam2) {
  const auto trace_type = GetParam();
  std::string gep, type;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      type = "%ptr_wg_v2float";
      gep = "OpAccessChain %ptr_wg_v2float %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_0 %index %index %index";
      break;
    default:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg_struct = OpTypePointer Workgroup %struct
%ptr_wg_v2float = OpTypePointer Workgroup %v2float
%ptr_wg_float = OpTypePointer Workgroup %float
%null = OpConstantNull )" + type +
                            R"(
%var = OpVariable %ptr_wg_struct Workgroup
%void_fn = OpTypeFunction %void
%foo_fn = OpTypeFunction %void %bool )" +
                            type + R"(
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = )" + gep + R"(
%copy = OpCopyObject )" + type +
                            R"( %gep
%call = OpFunctionCall %void %foo %bool_cond %copy
OpReturn
OpFunctionEnd
%foo = OpFunction %void None %foo_fn
%bool_param = OpFunctionParameter %bool
%ptr_param = OpFunctionParameter )" +
                            type + R"(
%foo_entry = OpLabel
%sel = OpSelect )" + type + R"( %bool_param %ptr_param %null
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceTypedTest, FunctionCall) {
  const auto trace_type = GetParam();
  std::string gep, type;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      type = "%ptr_wg_v2float";
      gep = "OpAccessChain %ptr_wg_v2float %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_0 %index %index %index";
      break;
    default:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg_struct = OpTypePointer Workgroup %struct
%ptr_wg_v2float = OpTypePointer Workgroup %v2float
%ptr_wg_float = OpTypePointer Workgroup %float
%null = OpConstantNull )" + type +
                            R"(
%var = OpVariable %ptr_wg_struct Workgroup
%void_fn = OpTypeFunction %void
%foo_ty = OpTypeFunction )" +
                            type + R"(
%main = OpFunction %void None %void_fn
%entry = OpLabel
%call = OpFunctionCall )" + type +
                            R"( %foo
OpReturn
OpFunctionEnd
%foo = OpFunction )" + type +
                            R"( None %foo_ty
%foo_entry = OpLabel
%gep = )" + gep + R"(
OpReturnValue %gep
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceTypedTest, FunctionCallMultiReturn1) {
  const auto trace_type = GetParam();
  std::string gep, type;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      type = "%ptr_wg_v2float";
      gep = "OpAccessChain %ptr_wg_v2float %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_0 %index %index %index";
      break;
    default:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg_struct = OpTypePointer Workgroup %struct
%ptr_wg_v2float = OpTypePointer Workgroup %v2float
%ptr_wg_float = OpTypePointer Workgroup %float
%null = OpConstantNull )" + type +
                            R"(
%var = OpVariable %ptr_wg_struct Workgroup
%void_fn = OpTypeFunction %void
%foo_ty = OpTypeFunction )" +
                            type + R"(
%main = OpFunction %void None %void_fn
%entry = OpLabel
%call = OpFunctionCall )" + type +
                            R"( %foo
OpReturn
OpFunctionEnd
%foo = OpFunction )" + type +
                            R"( None %foo_ty
%foo_entry = OpLabel
%gep = )" + gep + R"(
OpSelectionMerge %merge None
OpBranchConditional %bool_cond %then %merge
%then = OpLabel
OpReturnValue %gep
%merge = OpLabel
OpReturnValue %null
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceTypedTest, FunctionCallMultiReturn2) {
  const auto trace_type = GetParam();
  std::string gep, type;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      type = "%ptr_wg_v2float";
      gep = "OpAccessChain %ptr_wg_v2float %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_0 %index %index %index";
      break;
    default:
      type = "%ptr_wg_float";
      gep = "OpAccessChain %ptr_wg_float %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg_struct = OpTypePointer Workgroup %struct
%ptr_wg_v2float = OpTypePointer Workgroup %v2float
%ptr_wg_float = OpTypePointer Workgroup %float
%null = OpConstantNull )" + type +
                            R"(
%var = OpVariable %ptr_wg_struct Workgroup
%void_fn = OpTypeFunction %void
%foo_ty = OpTypeFunction )" +
                            type + R"(
%main = OpFunction %void None %void_fn
%entry = OpLabel
%call = OpFunctionCall )" + type +
                            R"( %foo
OpReturn
OpFunctionEnd
%foo = OpFunction )" + type +
                            R"( None %foo_ty
%foo_entry = OpLabel
%gep = )" + gep + R"(
OpSelectionMerge %merge None
OpBranchConditional %bool_cond %then %merge
%then = OpLabel
OpReturnValue %null
%merge = OpLabel
OpReturnValue %gep
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

INSTANTIATE_TEST_SUITE_P(ValidateLogicalPointersMatrixTraceTyped,
                         MatrixTraceTypedTest, ValuesIn(traces));

using MatrixTraceUntypedTest = spvtest::ValidateBase<MatrixTrace>;

TEST_P(MatrixTraceUntypedTest, PhiLoopOp1) {
  const auto trace_type = GetParam();
  std::string gep;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      gep =
          "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index "
          "%index";
      break;
    default:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg = OpTypeUntypedPointerKHR Workgroup
%null = OpConstantNull %ptr_wg
%var = OpUntypedVariableKHR %ptr_wg Workgroup %struct
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = )" + gep + R"(
OpBranch %loop
%loop = OpLabel
%phi = OpPhi %ptr_wg %gep %entry %copy %continue
OpLoopMerge %merge %continue None
OpBranchConditional %bool_cond %merge %continue
%continue = OpLabel
%copy = OpCopyObject %ptr_wg %phi
OpBranch %loop
%merge = OpLabel
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}
TEST_P(MatrixTraceUntypedTest, PhiLoopOp2) {
  const auto trace_type = GetParam();
  std::string gep;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      gep =
          "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index "
          "%index";
      break;
    default:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg = OpTypeUntypedPointerKHR Workgroup
%null = OpConstantNull %ptr_wg
%var = OpUntypedVariableKHR %ptr_wg Workgroup %struct
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = )" + gep + R"(
OpBranch %loop
%loop = OpLabel
%phi = OpPhi %ptr_wg %copy %continue %gep %entry
OpLoopMerge %merge %continue None
OpBranchConditional %bool_cond %merge %continue
%continue = OpLabel
%copy = OpCopyObject %ptr_wg %phi
OpBranch %loop
%merge = OpLabel
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceUntypedTest, SelectOp1) {
  const auto trace_type = GetParam();
  std::string gep;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      gep =
          "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index "
          "%index";
      break;
    default:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg = OpTypeUntypedPointerKHR Workgroup
%null = OpConstantNull %ptr_wg
%var = OpUntypedVariableKHR %ptr_wg Workgroup %struct
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = )" + gep + R"(
%copy = OpCopyObject %ptr_wg %gep
%sel = OpSelect %ptr_wg %bool_cond %copy %null
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceUntypedTest, SelectOp2) {
  const auto trace_type = GetParam();
  std::string gep;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      gep =
          "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index "
          "%index";
      break;
    default:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg = OpTypeUntypedPointerKHR Workgroup
%null = OpConstantNull %ptr_wg
%var = OpUntypedVariableKHR %ptr_wg Workgroup %struct
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = )" + gep + R"(
%copy = OpCopyObject %ptr_wg %gep
%sel = OpSelect %ptr_wg %bool_cond %null %copy
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceUntypedTest, FunctionVariable) {
  const auto trace_type = GetParam();
  std::string gep;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      gep = "OpUntypedAccessChainKHR %ptr_wg %mat2x2 %var_mat2x2 %index";
      break;
    case MatrixTrace::kComponent:
      gep = "OpUntypedAccessChainKHR %ptr_wg %mat2x2 %var_mat2x2 %index %index";
      break;
    default:
      gep = "OpUntypedAccessChainKHR %ptr_wg %float %var_float";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg = OpTypeUntypedPointerKHR Workgroup
%ptr_func_wg = OpTypePointer Function %ptr_wg
%var_mat2x2 = OpUntypedVariableKHR %ptr_wg Workgroup %mat2x2
%var_float = OpUntypedVariableKHR %ptr_wg Workgroup %float
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%func_var = OpVariable %ptr_func_wg Function
%gep = )" + gep + R"(
OpStore %func_var %gep
%ld = OpLoad %ptr_wg %func_var
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceUntypedTest, PrivateVariable) {
  const auto trace_type = GetParam();
  std::string gep;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      gep = "OpUntypedAccessChainKHR %ptr_wg %mat2x2 %var_mat2x2 %index";
      break;
    case MatrixTrace::kComponent:
      gep = "OpUntypedAccessChainKHR %ptr_wg %mat2x2 %var_mat2x2 %index %index";
      break;
    default:
      gep = "OpUntypedAccessChainKHR %ptr_wg %float %var_float";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg = OpTypeUntypedPointerKHR Workgroup
%ptr_priv_wg = OpTypePointer Private %ptr_wg
%var_mat2x2 = OpUntypedVariableKHR %ptr_wg Workgroup %mat2x2
%var_float = OpUntypedVariableKHR %ptr_wg Workgroup %float
%priv_var = OpVariable %ptr_priv_wg Private
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = )" + gep + R"(
OpStore %priv_var %gep
%ld = OpLoad %ptr_wg %priv_var
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceUntypedTest, FunctionVariableAggregate) {
  const auto trace_type = GetParam();
  std::string gep;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      gep = "OpUntypedAccessChainKHR %ptr_wg %mat2x2 %var_mat2x2 %index";
      break;
    case MatrixTrace::kComponent:
      gep = "OpUntypedAccessChainKHR %ptr_wg %mat2x2 %var_mat2x2 %index %index";
      break;
    default:
      gep = "OpUntypedAccessChainKHR %ptr_wg %float %var_float";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg = OpTypeUntypedPointerKHR Workgroup
%func_array = OpTypeArray %ptr_wg %int_1
%func_struct = OpTypeStruct %func_array
%ptr_func_wg_struct = OpTypePointer Function %func_struct
%ptr_func_wg = OpTypePointer Function %ptr_wg
%var_mat2x2 = OpUntypedVariableKHR %ptr_wg Workgroup %mat2x2
%var_float = OpUntypedVariableKHR %ptr_wg Workgroup %float
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%func_var = OpVariable %ptr_func_wg_struct Function
%gep = )" + gep + R"(
%store_gep = OpAccessChain %ptr_func_wg %func_var %int_0 %index
%store_gep_copy = OpCopyObject %ptr_func_wg %store_gep
OpStore %store_gep_copy %gep
%ld_gep = OpAccessChain %ptr_func_wg %func_var %int_0 %index
%ld = OpLoad %ptr_wg %ld_gep
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceUntypedTest, FunctionCallParam1) {
  const auto trace_type = GetParam();
  std::string gep;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      gep =
          "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index "
          "%index";
      break;
    default:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg = OpTypeUntypedPointerKHR Workgroup
%null = OpConstantNull %ptr_wg
%var = OpUntypedVariableKHR %ptr_wg Workgroup %struct
%void_fn = OpTypeFunction %void
%foo_fn = OpTypeFunction %void %ptr_wg %bool
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = )" + gep + R"(
%copy = OpCopyObject %ptr_wg %gep
%call = OpFunctionCall %void %foo %copy %bool_cond
OpReturn
OpFunctionEnd
%foo = OpFunction %void None %foo_fn
%ptr_param = OpFunctionParameter %ptr_wg
%bool_param = OpFunctionParameter %bool
%foo_entry = OpLabel
%sel = OpSelect %ptr_wg %bool_param %ptr_param %null
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceUntypedTest, FunctionCallParam2) {
  const auto trace_type = GetParam();
  std::string gep;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      gep =
          "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index "
          "%index";
      break;
    default:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg = OpTypeUntypedPointerKHR Workgroup
%null = OpConstantNull %ptr_wg
%var = OpUntypedVariableKHR %ptr_wg Workgroup %struct
%void_fn = OpTypeFunction %void
%foo_fn = OpTypeFunction %void %bool %ptr_wg
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = )" + gep + R"(
%copy = OpCopyObject %ptr_wg %gep
%call = OpFunctionCall %void %foo %bool_cond %copy
OpReturn
OpFunctionEnd
%foo = OpFunction %void None %foo_fn
%bool_param = OpFunctionParameter %bool
%ptr_param = OpFunctionParameter %ptr_wg
%foo_entry = OpLabel
%sel = OpSelect %ptr_wg %bool_param %ptr_param %null
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceUntypedTest, FunctionCall) {
  const auto trace_type = GetParam();
  std::string gep;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      gep =
          "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index "
          "%index";
      break;
    default:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg = OpTypeUntypedPointerKHR Workgroup
%null = OpConstantNull %ptr_wg
%var = OpUntypedVariableKHR %ptr_wg Workgroup %struct
%void_fn = OpTypeFunction %void
%foo_ty = OpTypeFunction %ptr_wg
%main = OpFunction %void None %void_fn
%entry = OpLabel
%call = OpFunctionCall %ptr_wg %foo
OpReturn
OpFunctionEnd
%foo = OpFunction %ptr_wg None %foo_ty
%foo_entry = OpLabel
%gep = )" + gep + R"(
OpReturnValue %gep
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceUntypedTest, FunctionCallMultiReturn1) {
  const auto trace_type = GetParam();
  std::string gep;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      gep =
          "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index "
          "%index";
      break;
    default:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg = OpTypeUntypedPointerKHR Workgroup
%null = OpConstantNull %ptr_wg
%var = OpUntypedVariableKHR %ptr_wg Workgroup %struct
%void_fn = OpTypeFunction %void
%foo_ty = OpTypeFunction %ptr_wg
%main = OpFunction %void None %void_fn
%entry = OpLabel
%call = OpFunctionCall %ptr_wg %foo
OpReturn
OpFunctionEnd
%foo = OpFunction %ptr_wg None %foo_ty
%foo_entry = OpLabel
%gep = )" + gep + R"(
OpSelectionMerge %merge None
OpBranchConditional %bool_cond %then %merge
%then = OpLabel
OpReturnValue %gep
%merge = OpLabel
OpReturnValue %null
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceUntypedTest, FunctionCallMultiReturn2) {
  const auto trace_type = GetParam();
  std::string gep;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      gep =
          "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_0 %index %index "
          "%index";
      break;
    default:
      gep = "OpUntypedAccessChainKHR %ptr_wg %struct %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg = OpTypeUntypedPointerKHR Workgroup
%null = OpConstantNull %ptr_wg
%var = OpUntypedVariableKHR %ptr_wg Workgroup %struct
%void_fn = OpTypeFunction %void
%foo_ty = OpTypeFunction %ptr_wg
%main = OpFunction %void None %void_fn
%entry = OpLabel
%call = OpFunctionCall %ptr_wg %foo
OpReturn
OpFunctionEnd
%foo = OpFunction %ptr_wg None %foo_ty
%foo_entry = OpLabel
%gep = )" + gep + R"(
OpSelectionMerge %merge None
OpBranchConditional %bool_cond %then %merge
%then = OpLabel
OpReturnValue %null
%merge = OpLabel
OpReturnValue %gep
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

TEST_P(MatrixTraceUntypedTest, MixedTypes) {
  const auto trace_type = GetParam();
  std::string gep;
  switch (trace_type) {
    case MatrixTrace::kColumn:
      gep = "OpAccessChain %ptr_wg_v2float %var %int_0 %index %index";
      break;
    case MatrixTrace::kComponent:
      gep = "OpAccessChain %ptr_wg_float %var %int_0 %index %index %index";
      break;
    default:
      gep = "OpAccessChain %ptr_wg_float %var %int_1";
      break;
  }

  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%index = OpUndef %int
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%array = OpTypeArray %mat2x2 %int_2
%struct = OpTypeStruct %array %float
%ptr_wg = OpTypeUntypedPointerKHR Workgroup
%ptr_wg_struct = OpTypePointer Workgroup %struct
%ptr_wg_v2float = OpTypePointer Workgroup %v2float
%ptr_wg_float = OpTypePointer Workgroup %float
%null = OpConstantNull %ptr_wg
%var = OpVariable %ptr_wg_struct Workgroup
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = )" + gep + R"(
%copy = OpUntypedAccessChainKHR %ptr_wg %struct %gep
%sel = OpSelect %ptr_wg %bool_cond %copy %null
OpReturn
OpFunctionEnd
)";

  const auto expected = trace_type == MatrixTrace::kNotAMatrix
                            ? SPV_SUCCESS
                            : SPV_ERROR_INVALID_DATA;
  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(expected, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  if (expected) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Variable pointer must not point to a column or a "
                          "component of a column of a matrix"));
  }
}

INSTANTIATE_TEST_SUITE_P(ValidateLogicalPointersMatrixTraceUntyped,
                         MatrixTraceUntypedTest, ValuesIn(traces));

TEST_F(ValidateLogicalPointersTest, SelectDifferentBuffersTyped) {
  const std::string spirv = R"(
OpCapability VariablePointersStorageBuffer
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%struct = OpTypeStruct %int
%ptr_struct = OpTypePointer StorageBuffer %struct
%v1 = OpVariable %ptr_struct StorageBuffer
%v2 = OpVariable %ptr_struct StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%select = OpSelect %ptr_struct %bool_cond %v1 %v2
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Variable pointers must point into the same structure "
                        "(or OpConstantNull)"));
}

TEST_F(ValidateLogicalPointersTest, SelectDifferentBuffersUntyped) {
  const std::string spirv = R"(
OpCapability VariablePointersStorageBuffer
OpCapability Shader
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%struct = OpTypeStruct %int
%ptr = OpTypeUntypedPointerKHR StorageBuffer
%v1 = OpUntypedVariableKHR %ptr StorageBuffer %struct
%v2 = OpUntypedVariableKHR %ptr StorageBuffer %struct
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%select = OpSelect %ptr %bool_cond %v1 %v2
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Variable pointers must point into the same structure "
                        "(or OpConstantNull)"));
}

TEST_F(ValidateLogicalPointersTest, PhiDifferentBuffersTyped) {
  const std::string spirv = R"(
OpCapability VariablePointersStorageBuffer
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%struct = OpTypeStruct %int
%ptr_struct = OpTypePointer StorageBuffer %struct
%v1 = OpVariable %ptr_struct StorageBuffer
%v2 = OpVariable %ptr_struct StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpSelectionMerge %merge None
OpBranchConditional %bool_cond %then %merge
%then = OpLabel
OpBranch %merge
%merge = OpLabel
%phi = OpPhi %ptr_struct %v1 %entry %v2 %then
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Variable pointers must point into the same structure "
                        "(or OpConstantNull)"));
}

TEST_F(ValidateLogicalPointersTest, PhiDifferentBuffersUntyped) {
  const std::string spirv = R"(
OpCapability VariablePointersStorageBuffer
OpCapability Shader
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%int = OpTypeInt 32 0
%struct = OpTypeStruct %int
%ptr = OpTypeUntypedPointerKHR StorageBuffer
%v1 = OpUntypedVariableKHR %ptr StorageBuffer %struct
%v2 = OpUntypedVariableKHR %ptr StorageBuffer %struct
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpSelectionMerge %merge None
OpBranchConditional %bool_cond %then %merge
%then = OpLabel
OpBranch %merge
%merge = OpLabel
%phi = OpPhi %ptr %v1 %entry %v2 %then
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Variable pointers must point into the same structure "
                        "(or OpConstantNull)"));
}

TEST_F(ValidateLogicalPointersTest, BlockArrayTyped) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%struct = OpTypeStruct %int
%array = OpTypeArray %struct %int_4
%ptr_array = OpTypePointer StorageBuffer %array
%ptr_int = OpTypePointer StorageBuffer %int
%var = OpVariable %ptr_array StorageBuffer
%null = OpConstantNull %ptr_array
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%select = OpSelect %ptr_array %bool_cond %null %var
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Variable pointer must not point to an array of Block- "
                        "or BufferBlock-decorated structs"));
}

TEST_F(ValidateLogicalPointersTest, BlockArrayUntyped) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%bool = OpTypeBool
%bool_cond = OpUndef %bool
%struct = OpTypeStruct %int
%array = OpTypeArray %struct %int_4
%ptr = OpTypeUntypedPointerKHR StorageBuffer
%var = OpUntypedVariableKHR %ptr StorageBuffer %array
%null = OpConstantNull %ptr
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%select = OpSelect %ptr %bool_cond %null %var
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Variable pointer must not point to an array of Block- "
                        "or BufferBlock-decorated structs"));
}

TEST_F(ValidateLogicalPointersTest, UntypedMatrixLoad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%struct = OpTypeStruct %mat2x2
%ptr = OpTypeUntypedPointerKHR StorageBuffer
%var = OpUntypedVariableKHR %ptr StorageBuffer %struct
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%ld = OpLoad %struct %var
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
}

TEST_F(ValidateLogicalPointersTest, UntypedMatrixLoadVariablePointer) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%struct = OpTypeStruct %mat2x2
%ptr = OpTypeUntypedPointerKHR StorageBuffer
%var = OpUntypedVariableKHR %ptr StorageBuffer %struct
%null = OpConstantNull %ptr
%bool = OpTypeBool
%cond = OpUndef %bool
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%sel = OpSelect %ptr %cond %var %null
%ld = OpLoad %struct %sel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Variable pointer must not point to an object that is "
                        "or contains a matrix"));
}

TEST_F(ValidateLogicalPointersTest, UntypedMatrixStore) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%zero = OpConstantNull %mat2x2
%struct = OpTypeStruct %mat2x2
%ptr = OpTypeUntypedPointerKHR StorageBuffer
%var = OpUntypedVariableKHR %ptr StorageBuffer %struct
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpStore %var %zero
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
}

TEST_F(ValidateLogicalPointersTest, UntypedMatrixStoreVariablePointer) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%zero = OpConstantNull %mat2x2
%struct = OpTypeStruct %mat2x2
%ptr = OpTypeUntypedPointerKHR StorageBuffer
%var = OpUntypedVariableKHR %ptr StorageBuffer %struct
%null = OpConstantNull %ptr
%bool = OpTypeBool
%cond = OpUndef %bool
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%sel = OpSelect %ptr %cond %var %null
OpStore %sel %zero
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Variable pointer must not point to an object that is "
                        "or contains a matrix"));
}

TEST_F(ValidateLogicalPointersTest, LogicalPointerOperandFailure) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Kernel
OpCapability Addresses
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%v2int = OpTypeVector %int 2
%ptr_v2int = OpTypePointer StorageBuffer %v2int
%var = OpVariable %ptr_v2int StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpInBoundsPtrAccessChain %ptr_v2int %var %int_0
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Instruction may not have a logical pointer operand"));
}

TEST_F(ValidateLogicalPointersTest,
       LogicalPointerOperandVariablePointerStorageBuffer) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpCapability Addresses
OpMemoryModel Logical GLSL450
OpDecorate %ptr_int ArrayStride 4
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%array = OpTypeArray %int %int_4
%ptr_array = OpTypePointer StorageBuffer %array
%ptr_int = OpTypePointer StorageBuffer %int
%var = OpVariable %ptr_array StorageBuffer
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpAccessChain %ptr_int %var %int_0
%ptr_gep = OpPtrAccessChain %ptr_int %gep %int_0
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Instruction may only have a logical pointer operand in the "
                "StorageBuffer or "
                "Workgroup storage classes with appropriate variable pointers "
                "capability"));
}

TEST_F(ValidateLogicalPointersTest,
       LogicalPointerOperandVariablePointerWorkgroup) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpCapability VariablePointersStorageBuffer
OpMemoryModel Logical GLSL450
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%array = OpTypeArray %int %int_4
%ptr_array = OpTypePointer Workgroup %array
%ptr_int = OpTypePointer Workgroup %int
%var = OpVariable %ptr_array Workgroup
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpAccessChain %ptr_int %var %int_0
%ptr_gep = OpPtrAccessChain %ptr_int %gep %int_0
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Instruction may only have a logical pointer operand in the "
                "StorageBuffer or "
                "Workgroup storage classes with appropriate variable pointers "
                "capability"));
}

TEST_F(ValidateLogicalPointersTest,
       LogicalPointerReturnVariablePointerStorageBuffer) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
%int = OpTypeInt 32 0
%ptr = OpTypePointer StorageBuffer %int
%null = OpConstantNull %ptr
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Instruction may only return a logical pointer in the "
                "StorageBuffer or "
                "Workgroup storage classes with appropriate variable pointers "
                "capability"));
}

TEST_F(ValidateLogicalPointersTest,
       LogicalPointerReturnVariablePointerWorkgroup) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpCapability VariablePointersStorageBuffer
OpMemoryModel Logical GLSL450
%int = OpTypeInt 32 0
%ptr = OpTypePointer Workgroup %int
%null = OpConstantNull %ptr
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Instruction may only return a logical pointer in the "
                "StorageBuffer or "
                "Workgroup storage classes with appropriate variable pointers "
                "capability"));
}

TEST_F(ValidateLogicalPointersTest, ArrayLengthInvalidVariablePointer) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%int = OpTypeInt 32 0
%array = OpTypeRuntimeArray %int
%struct = OpTypeStruct %array
%ptr = OpTypePointer StorageBuffer %struct
%var = OpVariable %ptr StorageBuffer
%null = OpConstantNull %ptr
%bool = OpTypeBool
%cond = OpUndef %bool
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%sel = OpSelect %ptr %cond %null %var
%len = OpArrayLength %int %sel 0
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Pointer operand must not be a variable pointer"));
}

TEST_F(ValidateLogicalPointersTest, ArrayLengthUntypedInvalidVariablePointer) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%int = OpTypeInt 32 0
%array = OpTypeRuntimeArray %int
%struct = OpTypeStruct %array
%ptr = OpTypeUntypedPointerKHR StorageBuffer
%var = OpUntypedVariableKHR %ptr StorageBuffer %struct
%null = OpConstantNull %ptr
%bool = OpTypeBool
%cond = OpUndef %bool
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%sel = OpSelect %ptr %cond %null %var
%len = OpUntypedArrayLengthKHR %int %struct %sel 0
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Pointer operand must not be a variable pointer"));
}

TEST_F(ValidateLogicalPointersTest, FunctionParameterOperandOfFunctionCall) {
  const std::string spirv = R"(
               OpCapability ClipDistance
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "               "
               OpExecutionMode %2 OriginUpperLeft
       %void = OpTypeVoid
         %32 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
         %35 = OpTypeFunction %float %_ptr_Function_float
%float_1_35631564en19 = OpConstant %float 1.35631564e-19
%float_1_35631564en19_0 = OpConstant %float 1.35631564e-19
          %2 = OpFunction %void None %32
       %8447 = OpLabel
               OpUnreachable
               OpFunctionEnd
          %9 = OpFunction %float None %35
         %10 = OpFunctionParameter %_ptr_Function_float
      %65535 = OpLabel
        %217 = OpVariable %_ptr_Function_float Function
        %218 = OpLoad %float %10
    %2097407 = OpFunctionCall %float %9 %10
        %231 = OpLoad %float %217
        %232 = OpFDiv %float %218 %231
        %233 = OpExtInst %float %1 SmoothStep %float_1_35631564en19 %float_1_35631564en19_0 %232
               OpReturnValue %233
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateLogicalPointersTest, RecursiveCalls) {
  const std::string spirv = R"(
               OpCapability ClipDistance
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "               "
               OpExecutionMode %2 OriginUpperLeft
       %void = OpTypeVoid
         %32 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
         %35 = OpTypeFunction %float %_ptr_Function_float
%float_1_35631564en19 = OpConstant %float 1.35631564e-19
%float_1_35631564en19_0 = OpConstant %float 1.35631564e-19
          %2 = OpFunction %void None %32
       %8447 = OpLabel
               OpUnreachable
               OpFunctionEnd
          %9 = OpFunction %float None %35
         %10 = OpFunctionParameter %_ptr_Function_float
      %65535 = OpLabel
        %217 = OpVariable %_ptr_Function_float Function
        %218 = OpLoad %float %10
    %2097407 = OpFunctionCall %float %20 %10
        %231 = OpLoad %float %217
        %232 = OpFDiv %float %218 %231
        %233 = OpExtInst %float %1 SmoothStep %float_1_35631564en19 %float_1_35631564en19_0 %232
               OpReturnValue %233
               OpFunctionEnd
         %20 = OpFunction %float None %35
         %21 = OpFunctionParameter %_ptr_Function_float
         %22 = OpLabel
         %23 = OpFunctionCall %float %9 %21
               OpReturnValue %float_1_35631564en19
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

}  // namespace
}  // namespace val
}  // namespace spvtools
