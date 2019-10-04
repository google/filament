// Copyright (c) 2018 Google LLC.
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

#include "gmock/gmock.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;

using ValidateInterfacesTest = spvtest::ValidateBase<bool>;

TEST_F(ValidateInterfacesTest, EntryPointMissingInput) {
  std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "func"
OpExecutionMode %1 OriginUpperLeft
%2 = OpTypeVoid
%3 = OpTypeInt 32 0
%4 = OpTypePointer Input %3
%5 = OpVariable %4 Input
%6 = OpTypeFunction %2
%1 = OpFunction %2 None %6
%7 = OpLabel
%8 = OpLoad %3 %5
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Interface variable id <5> is used by entry point 'func' id <1>, "
          "but is not listed as an interface"));
}

TEST_F(ValidateInterfacesTest, EntryPointMissingOutput) {
  std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "func"
OpExecutionMode %1 OriginUpperLeft
%2 = OpTypeVoid
%3 = OpTypeInt 32 0
%4 = OpTypePointer Output %3
%5 = OpVariable %4 Output
%6 = OpTypeFunction %2
%1 = OpFunction %2 None %6
%7 = OpLabel
%8 = OpLoad %3 %5
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Interface variable id <5> is used by entry point 'func' id <1>, "
          "but is not listed as an interface"));
}

TEST_F(ValidateInterfacesTest, InterfaceMissingUseInSubfunction) {
  std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "func"
OpExecutionMode %1 OriginUpperLeft
%2 = OpTypeVoid
%3 = OpTypeInt 32 0
%4 = OpTypePointer Input %3
%5 = OpVariable %4 Input
%6 = OpTypeFunction %2
%1 = OpFunction %2 None %6
%7 = OpLabel
%8 = OpFunctionCall %2 %9
OpReturn
OpFunctionEnd
%9 = OpFunction %2 None %6
%10 = OpLabel
%11 = OpLoad %3 %5
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Interface variable id <5> is used by entry point 'func' id <1>, "
          "but is not listed as an interface"));
}

TEST_F(ValidateInterfacesTest, TwoEntryPointsOneFunction) {
  std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "func" %2
OpEntryPoint Fragment %1 "func2"
OpExecutionMode %1 OriginUpperLeft
%3 = OpTypeVoid
%4 = OpTypeInt 32 0
%5 = OpTypePointer Input %4
%2 = OpVariable %5 Input
%6 = OpTypeFunction %3
%1 = OpFunction %3 None %6
%7 = OpLabel
%8 = OpLoad %4 %2
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Interface variable id <2> is used by entry point 'func2' id <1>, "
          "but is not listed as an interface"));
}

TEST_F(ValidateInterfacesTest, MissingInterfaceThroughInitializer) {
  const std::string text = R"(
OpCapability Shader
OpCapability VariablePointers
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "func"
OpExecutionMode %1 OriginUpperLeft
%2 = OpTypeVoid
%3 = OpTypeInt 32 0
%4 = OpTypePointer Input %3
%5 = OpTypePointer Function %4
%6 = OpVariable %4 Input
%7 = OpTypeFunction %2
%1 = OpFunction %2 None %7
%8 = OpLabel
%9 = OpVariable %5 Function %6
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_UNIVERSAL_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Interface variable id <6> is used by entry point 'func' id <1>, "
          "but is not listed as an interface"));
}

TEST_F(ValidateInterfacesTest, NonUniqueInterfacesSPV1p3) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var %var
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint3 = OpTypeVector %uint 3
%struct = OpTypeStruct %uint3
%ptr_struct = OpTypePointer Input %struct
%var = OpVariable %ptr_struct Input
%func_ty = OpTypeFunction %void
%main = OpFunction %void None %func_ty
%1 = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
}

TEST_F(ValidateInterfacesTest, NonUniqueInterfacesSPV1p4) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var %var
OpExecutionMode %main LocalSize 1 1 1
OpName %main "main"
OpName %var "var"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint3 = OpTypeVector %uint 3
%struct = OpTypeStruct %uint3
%ptr_struct = OpTypePointer Input %struct
%var = OpVariable %ptr_struct Input
%func_ty = OpTypeFunction %void
%main = OpFunction %void None %func_ty
%1 = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_4));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Non-unique OpEntryPoint interface 2[%var] is disallowed"));
}

TEST_F(ValidateInterfacesTest, MissingGlobalVarSPV1p3) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint3 = OpTypeVector %uint 3
%struct = OpTypeStruct %uint3
%ptr_struct = OpTypePointer StorageBuffer %struct
%var = OpVariable %ptr_struct StorageBuffer
%func_ty = OpTypeFunction %void
%main = OpFunction %void None %func_ty
%1 = OpLabel
%ld = OpLoad %struct %var
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
}

TEST_F(ValidateInterfacesTest, MissingGlobalVarSPV1p4) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpName %var "var"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint3 = OpTypeVector %uint 3
%struct = OpTypeStruct %uint3
%ptr_struct = OpTypePointer StorageBuffer %struct
%var = OpVariable %ptr_struct StorageBuffer
%func_ty = OpTypeFunction %void
%main = OpFunction %void None %func_ty
%1 = OpLabel
%ld = OpLoad %struct %var
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Interface variable id <2> is used by entry point "
                        "'main' id <1>, but is not listed as an interface"));
}

TEST_F(ValidateInterfacesTest, FunctionInterfaceVarSPV1p3) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var
OpExecutionMode %main LocalSize 1 1 1
OpName %var "var"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint3 = OpTypeVector %uint 3
%struct = OpTypeStruct %uint3
%ptr_struct = OpTypePointer Function %struct
%func_ty = OpTypeFunction %void
%main = OpFunction %void None %func_ty
%1 = OpLabel
%var = OpVariable %ptr_struct Function
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpEntryPoint interfaces must be OpVariables with "
                        "Storage Class of Input(1) or Output(3). Found Storage "
                        "Class 7 for Entry Point id 1."));
}

TEST_F(ValidateInterfacesTest, FunctionInterfaceVarSPV1p4) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var
OpExecutionMode %main LocalSize 1 1 1
OpName %var "var"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint3 = OpTypeVector %uint 3
%struct = OpTypeStruct %uint3
%ptr_struct = OpTypePointer Function %struct
%func_ty = OpTypeFunction %void
%main = OpFunction %void None %func_ty
%1 = OpLabel
%var = OpVariable %ptr_struct Function
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_4));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpEntryPoint interfaces should only list global variables"));
}

TEST_F(ValidateInterfacesTest, ModuleSPV1p3ValidateSPV1p4_NotAllUsedGlobals) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpName %var "var"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint3 = OpTypeVector %uint 3
%struct = OpTypeStruct %uint3
%ptr_struct = OpTypePointer StorageBuffer %struct
%var = OpVariable %ptr_struct StorageBuffer
%func_ty = OpTypeFunction %void
%main = OpFunction %void None %func_ty
%1 = OpLabel
%ld = OpLoad %struct %var
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_4));
}

TEST_F(ValidateInterfacesTest, ModuleSPV1p3ValidateSPV1p4_DuplicateInterface) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %gid %gid
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %gid BuiltIn GlobalInvocationId
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int3 = OpTypeVector %int 3
%ptr_input_int3 = OpTypePointer Input %int3
%gid = OpVariable %ptr_input_int3 Input
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_4));
}

TEST_F(ValidateInterfacesTest, SPV14MultipleEntryPointsSameFunction) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main1" %gid
OpEntryPoint GLCompute %main "main2" %gid
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %gid BuiltIn GlobalInvocationId
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int3 = OpTypeVector %int 3
%ptr_input_int3 = OpTypePointer Input %int3
%gid = OpVariable %ptr_input_int3 Input
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_4));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
