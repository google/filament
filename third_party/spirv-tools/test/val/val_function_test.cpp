// Copyright (c) 2019 Google LLC.
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

#include <sstream>
#include <string>
#include <tuple>

#include "gmock/gmock.h"
#include "test/test_fixture.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::Combine;
using ::testing::HasSubstr;
using ::testing::Values;

using ValidateFunctionCall = spvtest::ValidateBase<std::string>;

std::string GenerateShader(const std::string& storage_class,
                           const std::string& capabilities,
                           const std::string& extensions) {
  std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpCapability AtomicStorage
)" + capabilities + R"(
OpExtension "SPV_KHR_storage_buffer_storage_class"
)" +
                      extensions + R"(
OpMemoryModel Logical GLSL450
OpName %var "var"
%void = OpTypeVoid
%int = OpTypeInt 32 0
%ptr = OpTypePointer )" + storage_class + R"( %int
%caller_ty = OpTypeFunction %void
%callee_ty = OpTypeFunction %void %ptr
)";

  if (storage_class != "Function") {
    spirv += "%var = OpVariable %ptr " + storage_class;
  }

  spirv += R"(
%caller = OpFunction %void None %caller_ty
%1 = OpLabel
)";

  if (storage_class == "Function") {
    spirv += "%var = OpVariable %ptr Function";
  }

  spirv += R"(
%call = OpFunctionCall %void %callee %var
OpReturn
OpFunctionEnd
%callee = OpFunction %void None %callee_ty
%param = OpFunctionParameter %ptr
%2 = OpLabel
OpReturn
OpFunctionEnd
)";

  return spirv;
}

std::string GenerateShaderParameter(const std::string& storage_class,
                                    const std::string& capabilities,
                                    const std::string& extensions) {
  std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpCapability AtomicStorage
)" + capabilities + R"(
OpExtension "SPV_KHR_storage_buffer_storage_class"
)" +
                      extensions + R"(
OpMemoryModel Logical GLSL450
OpName %p "p"
%void = OpTypeVoid
%int = OpTypeInt 32 0
%ptr = OpTypePointer )" + storage_class + R"( %int
%func_ty = OpTypeFunction %void %ptr
%caller = OpFunction %void None %func_ty
%p = OpFunctionParameter %ptr
%1 = OpLabel
%call = OpFunctionCall %void %callee %p
OpReturn
OpFunctionEnd
%callee = OpFunction %void None %func_ty
%param = OpFunctionParameter %ptr
%2 = OpLabel
OpReturn
OpFunctionEnd
)";

  return spirv;
}

std::string GenerateShaderAccessChain(const std::string& storage_class,
                                      const std::string& capabilities,
                                      const std::string& extensions) {
  std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpCapability AtomicStorage
)" + capabilities + R"(
OpExtension "SPV_KHR_storage_buffer_storage_class"
)" +
                      extensions + R"(
OpMemoryModel Logical GLSL450
OpName %var "var"
OpName %gep "gep"
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int2 = OpTypeVector %int 2
%int_0 = OpConstant %int 0
%ptr = OpTypePointer )" + storage_class + R"( %int2
%ptr2 = OpTypePointer )" +
                      storage_class + R"( %int
%caller_ty = OpTypeFunction %void
%callee_ty = OpTypeFunction %void %ptr2
)";

  if (storage_class != "Function") {
    spirv += "%var = OpVariable %ptr " + storage_class;
  }

  spirv += R"(
%caller = OpFunction %void None %caller_ty
%1 = OpLabel
)";

  if (storage_class == "Function") {
    spirv += "%var = OpVariable %ptr Function";
  }

  spirv += R"(
%gep = OpAccessChain %ptr2 %var %int_0
%call = OpFunctionCall %void %callee %gep
OpReturn
OpFunctionEnd
%callee = OpFunction %void None %callee_ty
%param = OpFunctionParameter %ptr2
%2 = OpLabel
OpReturn
OpFunctionEnd
)";

  return spirv;
}

TEST_P(ValidateFunctionCall, VariableNoVariablePointers) {
  const std::string storage_class = GetParam();

  std::string spirv = GenerateShader(storage_class, "", "");

  const std::vector<std::string> valid_storage_classes = {
      "UniformConstant", "Function", "Private", "Workgroup", "AtomicCounter"};
  bool valid =
      std::find(valid_storage_classes.begin(), valid_storage_classes.end(),
                storage_class) != valid_storage_classes.end();

  CompileSuccessfully(spirv);
  if (valid) {
    EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
  } else {
    EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
    if (storage_class == "StorageBuffer") {
      EXPECT_THAT(
          getDiagnosticString(),
          HasSubstr("StorageBuffer pointer operand '1[%var]' requires a "
                    "variable pointers capability"));
    } else {
      EXPECT_THAT(
          getDiagnosticString(),
          HasSubstr("Invalid storage class for pointer operand '1[%var]'"));
    }
  }
}

TEST_P(ValidateFunctionCall, VariableVariablePointersStorageClass) {
  const std::string storage_class = GetParam();

  std::string spirv = GenerateShader(
      storage_class, "OpCapability VariablePointersStorageBuffer",
      "OpExtension \"SPV_KHR_variable_pointers\"");

  const std::vector<std::string> valid_storage_classes = {
      "UniformConstant", "Function",      "Private",
      "Workgroup",       "StorageBuffer", "AtomicCounter"};
  bool valid =
      std::find(valid_storage_classes.begin(), valid_storage_classes.end(),
                storage_class) != valid_storage_classes.end();

  CompileSuccessfully(spirv);
  if (valid) {
    EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
  } else {
    EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
    EXPECT_THAT(
        getDiagnosticString(),
        HasSubstr("Invalid storage class for pointer operand '1[%var]'"));
  }
}

TEST_P(ValidateFunctionCall, VariableVariablePointers) {
  const std::string storage_class = GetParam();

  std::string spirv =
      GenerateShader(storage_class, "OpCapability VariablePointers",
                     "OpExtension \"SPV_KHR_variable_pointers\"");

  const std::vector<std::string> valid_storage_classes = {
      "UniformConstant", "Function",      "Private",
      "Workgroup",       "StorageBuffer", "AtomicCounter"};
  bool valid =
      std::find(valid_storage_classes.begin(), valid_storage_classes.end(),
                storage_class) != valid_storage_classes.end();

  CompileSuccessfully(spirv);
  if (valid) {
    EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
  } else {
    EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
    EXPECT_THAT(
        getDiagnosticString(),
        HasSubstr("Invalid storage class for pointer operand '1[%var]'"));
  }
}

TEST_P(ValidateFunctionCall, ParameterNoVariablePointers) {
  const std::string storage_class = GetParam();

  std::string spirv = GenerateShaderParameter(storage_class, "", "");

  const std::vector<std::string> valid_storage_classes = {
      "UniformConstant", "Function", "Private", "Workgroup", "AtomicCounter"};
  bool valid =
      std::find(valid_storage_classes.begin(), valid_storage_classes.end(),
                storage_class) != valid_storage_classes.end();

  CompileSuccessfully(spirv);
  if (valid) {
    EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
  } else {
    EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
    if (storage_class == "StorageBuffer") {
      EXPECT_THAT(getDiagnosticString(),
                  HasSubstr("StorageBuffer pointer operand '1[%p]' requires a "
                            "variable pointers capability"));
    } else {
      EXPECT_THAT(
          getDiagnosticString(),
          HasSubstr("Invalid storage class for pointer operand '1[%p]'"));
    }
  }
}

TEST_P(ValidateFunctionCall, ParameterVariablePointersStorageBuffer) {
  const std::string storage_class = GetParam();

  std::string spirv = GenerateShaderParameter(
      storage_class, "OpCapability VariablePointersStorageBuffer",
      "OpExtension \"SPV_KHR_variable_pointers\"");

  const std::vector<std::string> valid_storage_classes = {
      "UniformConstant", "Function",      "Private",
      "Workgroup",       "StorageBuffer", "AtomicCounter"};
  bool valid =
      std::find(valid_storage_classes.begin(), valid_storage_classes.end(),
                storage_class) != valid_storage_classes.end();

  CompileSuccessfully(spirv);
  if (valid) {
    EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
  } else {
    EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Invalid storage class for pointer operand '1[%p]'"));
  }
}

TEST_P(ValidateFunctionCall, ParameterVariablePointers) {
  const std::string storage_class = GetParam();

  std::string spirv =
      GenerateShaderParameter(storage_class, "OpCapability VariablePointers",
                              "OpExtension \"SPV_KHR_variable_pointers\"");

  const std::vector<std::string> valid_storage_classes = {
      "UniformConstant", "Function",      "Private",
      "Workgroup",       "StorageBuffer", "AtomicCounter"};
  bool valid =
      std::find(valid_storage_classes.begin(), valid_storage_classes.end(),
                storage_class) != valid_storage_classes.end();

  CompileSuccessfully(spirv);
  if (valid) {
    EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
  } else {
    EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Invalid storage class for pointer operand '1[%p]'"));
  }
}

TEST_P(ValidateFunctionCall, NonMemoryObjectDeclarationNoVariablePointers) {
  const std::string storage_class = GetParam();

  std::string spirv = GenerateShaderAccessChain(storage_class, "", "");

  const std::vector<std::string> valid_storage_classes = {
      "Function", "Private", "Workgroup", "AtomicCounter"};
  bool valid_sc =
      std::find(valid_storage_classes.begin(), valid_storage_classes.end(),
                storage_class) != valid_storage_classes.end();

  CompileSuccessfully(spirv);
  spv_result_t expected_result =
      storage_class == "UniformConstant" ? SPV_SUCCESS : SPV_ERROR_INVALID_ID;
  EXPECT_EQ(expected_result, ValidateInstructions());
  if (valid_sc) {
    EXPECT_THAT(
        getDiagnosticString(),
        HasSubstr(
            "Pointer operand '2[%gep]' must be a memory object declaration"));
  } else {
    if (storage_class == "StorageBuffer") {
      EXPECT_THAT(
          getDiagnosticString(),
          HasSubstr("StorageBuffer pointer operand '2[%gep]' requires a "
                    "variable pointers capability"));
    } else if (storage_class != "UniformConstant") {
      EXPECT_THAT(
          getDiagnosticString(),
          HasSubstr("Invalid storage class for pointer operand '2[%gep]'"));
    }
  }
}

TEST_P(ValidateFunctionCall,
       NonMemoryObjectDeclarationVariablePointersStorageBuffer) {
  const std::string storage_class = GetParam();

  std::string spirv = GenerateShaderAccessChain(
      storage_class, "OpCapability VariablePointersStorageBuffer",
      "OpExtension \"SPV_KHR_variable_pointers\"");

  const std::vector<std::string> valid_storage_classes = {
      "Function", "Private", "Workgroup", "StorageBuffer", "AtomicCounter"};
  bool valid_sc =
      std::find(valid_storage_classes.begin(), valid_storage_classes.end(),
                storage_class) != valid_storage_classes.end();
  bool validate =
      storage_class == "StorageBuffer" || storage_class == "UniformConstant";

  CompileSuccessfully(spirv);
  if (validate) {
    EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
  } else {
    EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
    if (valid_sc) {
      EXPECT_THAT(
          getDiagnosticString(),
          HasSubstr(
              "Pointer operand '2[%gep]' must be a memory object declaration"));
    } else {
      EXPECT_THAT(
          getDiagnosticString(),
          HasSubstr("Invalid storage class for pointer operand '2[%gep]'"));
    }
  }
}

TEST_P(ValidateFunctionCall, NonMemoryObjectDeclarationVariablePointers) {
  const std::string storage_class = GetParam();

  std::string spirv =
      GenerateShaderAccessChain(storage_class, "OpCapability VariablePointers",
                                "OpExtension \"SPV_KHR_variable_pointers\"");

  const std::vector<std::string> valid_storage_classes = {
      "Function", "Private", "Workgroup", "StorageBuffer", "AtomicCounter"};
  bool valid_sc =
      std::find(valid_storage_classes.begin(), valid_storage_classes.end(),
                storage_class) != valid_storage_classes.end();
  bool validate = storage_class == "StorageBuffer" ||
                  storage_class == "Workgroup" ||
                  storage_class == "UniformConstant";

  CompileSuccessfully(spirv);
  if (validate) {
    EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
  } else {
    EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
    if (valid_sc) {
      EXPECT_THAT(
          getDiagnosticString(),
          HasSubstr(
              "Pointer operand '2[%gep]' must be a memory object declaration"));
    } else {
      EXPECT_THAT(
          getDiagnosticString(),
          HasSubstr("Invalid storage class for pointer operand '2[%gep]'"));
    }
  }
}

TEST_F(ValidateFunctionCall, LogicallyMatchingPointers) {
  std::string spirv =
      R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpSource HLSL 600
               OpDecorate %2 DescriptorSet 0
               OpDecorate %2 Binding 0
               OpMemberDecorate %_struct_3 0 Offset 0
               OpDecorate %_runtimearr__struct_3 ArrayStride 4
               OpMemberDecorate %_struct_5 0 Offset 0
               OpDecorate %_struct_5 BufferBlock
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
  %_struct_3 = OpTypeStruct %int
%_runtimearr__struct_3 = OpTypeRuntimeArray %_struct_3
  %_struct_5 = OpTypeStruct %_runtimearr__struct_3
%_ptr_Uniform__struct_5 = OpTypePointer Uniform %_struct_5
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
 %_struct_15 = OpTypeStruct %int
%_ptr_Function__struct_15 = OpTypePointer Function %_struct_15
%_ptr_Uniform__struct_3 = OpTypePointer Uniform %_struct_3
         %18 = OpTypeFunction %void %_ptr_Function__struct_15
          %2 = OpVariable %_ptr_Uniform__struct_5 Uniform
          %1 = OpFunction %void None %14
         %19 = OpLabel
         %20 = OpAccessChain %_ptr_Uniform__struct_3 %2 %int_0 %uint_0
         %21 = OpFunctionCall %void %22 %20
               OpReturn
               OpFunctionEnd
         %22 = OpFunction %void None %18
         %23 = OpFunctionParameter %_ptr_Function__struct_15
         %24 = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(spirv);
  spvValidatorOptionsSetBeforeHlslLegalization(getValidatorOptions(), true);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateFunctionCall, LogicallyMatchingPointersNestedStruct) {
  std::string spirv =
      R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpSource HLSL 600
               OpDecorate %2 DescriptorSet 0
               OpDecorate %2 Binding 0
               OpMemberDecorate %_struct_3 0 Offset 0
               OpMemberDecorate %_struct_4 0 Offset 0
               OpDecorate %_runtimearr__struct_4 ArrayStride 4
               OpMemberDecorate %_struct_6 0 Offset 0
               OpDecorate %_struct_6 BufferBlock
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
  %_struct_3 = OpTypeStruct %int
  %_struct_4 = OpTypeStruct %_struct_3
%_runtimearr__struct_4 = OpTypeRuntimeArray %_struct_4
  %_struct_6 = OpTypeStruct %_runtimearr__struct_4
%_ptr_Uniform__struct_6 = OpTypePointer Uniform %_struct_6
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
 %_struct_14 = OpTypeStruct %int
 %_struct_15 = OpTypeStruct %_struct_14
%_ptr_Function__struct_15 = OpTypePointer Function %_struct_15
%_ptr_Uniform__struct_4 = OpTypePointer Uniform %_struct_4
         %18 = OpTypeFunction %void %_ptr_Function__struct_15
          %2 = OpVariable %_ptr_Uniform__struct_6 Uniform
          %1 = OpFunction %void None %13
         %19 = OpLabel
         %20 = OpVariable %_ptr_Function__struct_15 Function
         %21 = OpAccessChain %_ptr_Uniform__struct_4 %2 %int_0 %uint_0
         %22 = OpFunctionCall %void %23 %21
               OpReturn
               OpFunctionEnd
         %23 = OpFunction %void None %18
         %24 = OpFunctionParameter %_ptr_Function__struct_15
         %25 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetBeforeHlslLegalization(getValidatorOptions(), true);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateFunctionCall, LogicallyMatchingPointersNestedArray) {
  std::string spirv =
      R"(
              OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpSource HLSL 600
               OpDecorate %2 DescriptorSet 0
               OpDecorate %2 Binding 0
               OpDecorate %_arr_int_uint_10 ArrayStride 4
               OpMemberDecorate %_struct_4 0 Offset 0
               OpDecorate %_runtimearr__struct_4 ArrayStride 40
               OpMemberDecorate %_struct_6 0 Offset 0
               OpDecorate %_struct_6 BufferBlock
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
    %uint_10 = OpConstant %uint 10
%_arr_int_uint_10 = OpTypeArray %int %uint_10
  %_struct_4 = OpTypeStruct %_arr_int_uint_10
%_runtimearr__struct_4 = OpTypeRuntimeArray %_struct_4
  %_struct_6 = OpTypeStruct %_runtimearr__struct_4
%_ptr_Uniform__struct_6 = OpTypePointer Uniform %_struct_6
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
%_ptr_Uniform__struct_4 = OpTypePointer Uniform %_struct_4
%_arr_int_uint_10_0 = OpTypeArray %int %uint_10
 %_struct_17 = OpTypeStruct %_arr_int_uint_10_0
%_ptr_Function__struct_17 = OpTypePointer Function %_struct_17
         %19 = OpTypeFunction %void %_ptr_Function__struct_17
          %2 = OpVariable %_ptr_Uniform__struct_6 Uniform
          %1 = OpFunction %void None %14
         %20 = OpLabel
         %21 = OpAccessChain %_ptr_Uniform__struct_4 %2 %int_0 %uint_0
         %22 = OpFunctionCall %void %23 %21
               OpReturn
               OpFunctionEnd
         %23 = OpFunction %void None %19
         %24 = OpFunctionParameter %_ptr_Function__struct_17
         %25 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetBeforeHlslLegalization(getValidatorOptions(), true);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateFunctionCall, LogicallyMismatchedPointersMissingMember) {
  //  Validation should fail because the formal parameter type has two members,
  //  while the actual parameter only has 1.
  std::string spirv =
      R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpSource HLSL 600
               OpDecorate %2 DescriptorSet 0
               OpDecorate %2 Binding 0
               OpMemberDecorate %_struct_3 0 Offset 0
               OpDecorate %_runtimearr__struct_3 ArrayStride 4
               OpMemberDecorate %_struct_5 0 Offset 0
               OpDecorate %_struct_5 BufferBlock
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
  %_struct_3 = OpTypeStruct %int
%_runtimearr__struct_3 = OpTypeRuntimeArray %_struct_3
  %_struct_5 = OpTypeStruct %_runtimearr__struct_3
%_ptr_Uniform__struct_5 = OpTypePointer Uniform %_struct_5
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
 %_struct_15 = OpTypeStruct %int %int
%_ptr_Function__struct_15 = OpTypePointer Function %_struct_15
%_ptr_Uniform__struct_3 = OpTypePointer Uniform %_struct_3
         %18 = OpTypeFunction %void %_ptr_Function__struct_15
          %2 = OpVariable %_ptr_Uniform__struct_5 Uniform
          %1 = OpFunction %void None %14
         %19 = OpLabel
         %20 = OpAccessChain %_ptr_Uniform__struct_3 %2 %int_0 %uint_0
         %21 = OpFunctionCall %void %22 %20
               OpReturn
               OpFunctionEnd
         %22 = OpFunction %void None %18
         %23 = OpFunctionParameter %_ptr_Function__struct_15
         %24 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetBeforeHlslLegalization(getValidatorOptions(), true);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("OpFunctionCall Argument <id>"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("type does not match Function <id>"));
}

TEST_F(ValidateFunctionCall, LogicallyMismatchedPointersDifferentMemberType) {
  //  Validation should fail because the formal parameter has a member that is
  // a different type than the actual parameter.
  std::string spirv =
      R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpSource HLSL 600
               OpDecorate %2 DescriptorSet 0
               OpDecorate %2 Binding 0
               OpMemberDecorate %_struct_3 0 Offset 0
               OpDecorate %_runtimearr__struct_3 ArrayStride 4
               OpMemberDecorate %_struct_5 0 Offset 0
               OpDecorate %_struct_5 BufferBlock
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
  %_struct_3 = OpTypeStruct %uint
%_runtimearr__struct_3 = OpTypeRuntimeArray %_struct_3
  %_struct_5 = OpTypeStruct %_runtimearr__struct_3
%_ptr_Uniform__struct_5 = OpTypePointer Uniform %_struct_5
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
 %_struct_15 = OpTypeStruct %int
%_ptr_Function__struct_15 = OpTypePointer Function %_struct_15
%_ptr_Uniform__struct_3 = OpTypePointer Uniform %_struct_3
         %18 = OpTypeFunction %void %_ptr_Function__struct_15
          %2 = OpVariable %_ptr_Uniform__struct_5 Uniform
          %1 = OpFunction %void None %14
         %19 = OpLabel
         %20 = OpAccessChain %_ptr_Uniform__struct_3 %2 %int_0 %uint_0
         %21 = OpFunctionCall %void %22 %20
               OpReturn
               OpFunctionEnd
         %22 = OpFunction %void None %18
         %23 = OpFunctionParameter %_ptr_Function__struct_15
         %24 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetBeforeHlslLegalization(getValidatorOptions(), true);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("OpFunctionCall Argument <id>"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("type does not match Function <id>"));
}

TEST_F(ValidateFunctionCall,
       LogicallyMismatchedPointersIncompatableDecorations) {
  //  Validation should fail because the formal parameter has an incompatible
  //  decoration.
  std::string spirv =
      R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpSource HLSL 600
               OpDecorate %2 DescriptorSet 0
               OpDecorate %2 Binding 0
               OpMemberDecorate %_struct_3 0 Offset 0
               OpDecorate %_runtimearr__struct_3 ArrayStride 4
               OpMemberDecorate %_struct_5 0 Offset 0
               OpDecorate %_struct_5 Block
               OpMemberDecorate %_struct_15 0 NonWritable
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
  %_struct_3 = OpTypeStruct %int
%_runtimearr__struct_3 = OpTypeRuntimeArray %_struct_3
  %_struct_5 = OpTypeStruct %_runtimearr__struct_3
%_ptr_StorageBuffer__struct_5 = OpTypePointer StorageBuffer %_struct_5
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
 %_struct_15 = OpTypeStruct %int
%_ptr_Function__struct_15 = OpTypePointer Function %_struct_15
%_ptr_StorageBuffer__struct_3 = OpTypePointer StorageBuffer %_struct_3
         %18 = OpTypeFunction %void %_ptr_Function__struct_15
          %2 = OpVariable %_ptr_StorageBuffer__struct_5 StorageBuffer
          %1 = OpFunction %void None %14
         %19 = OpLabel
         %20 = OpAccessChain %_ptr_StorageBuffer__struct_3 %2 %int_0 %uint_0
         %21 = OpFunctionCall %void %22 %20
               OpReturn
               OpFunctionEnd
         %22 = OpFunction %void None %18
         %23 = OpFunctionParameter %_ptr_Function__struct_15
         %24 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_4);
  spvValidatorOptionsSetBeforeHlslLegalization(getValidatorOptions(), true);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_4));
  EXPECT_THAT(getDiagnosticString(), HasSubstr("OpFunctionCall Argument <id>"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("type does not match Function <id>"));
}

TEST_F(ValidateFunctionCall,
       LogicallyMismatchedPointersIncompatableDecorations2) {
  //  Validation should fail because the formal parameter has an incompatible
  //  decoration.
  std::string spirv =
      R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpSource HLSL 600
               OpDecorate %2 DescriptorSet 0
               OpDecorate %2 Binding 0
               OpMemberDecorate %_struct_3 0 Offset 0
               OpDecorate %_runtimearr__struct_3 ArrayStride 4
               OpMemberDecorate %_struct_5 0 Offset 0
               OpDecorate %_struct_5 BufferBlock
               OpDecorate %_ptr_Uniform__struct_3 ArrayStride 4
               OpDecorate %_ptr_Uniform__struct_3_0 ArrayStride 8
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
  %_struct_3 = OpTypeStruct %int
%_runtimearr__struct_3 = OpTypeRuntimeArray %_struct_3
  %_struct_5 = OpTypeStruct %_runtimearr__struct_3
%_ptr_Uniform__struct_5 = OpTypePointer Uniform %_struct_5
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
%_ptr_Uniform__struct_3 = OpTypePointer Uniform %_struct_3
%_ptr_Uniform__struct_3_0 = OpTypePointer Uniform %_struct_3
         %18 = OpTypeFunction %void %_ptr_Uniform__struct_3_0
          %2 = OpVariable %_ptr_Uniform__struct_5 Uniform
          %1 = OpFunction %void None %14
         %19 = OpLabel
         %20 = OpAccessChain %_ptr_Uniform__struct_3 %2 %int_0 %uint_0
         %21 = OpFunctionCall %void %22 %20
               OpReturn
               OpFunctionEnd
         %22 = OpFunction %void None %18
         %23 = OpFunctionParameter %_ptr_Uniform__struct_3_0
         %24 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetBeforeHlslLegalization(getValidatorOptions(), true);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("OpFunctionCall Argument <id>"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("type does not match Function <id>"));
}

TEST_F(ValidateFunctionCall, LogicallyMismatchedPointersArraySize) {
  //  Validation should fail because the formal parameter array has a different
  // number of element than the actual parameter.
  std::string spirv =
      R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpSource HLSL 600
               OpDecorate %2 DescriptorSet 0
               OpDecorate %2 Binding 0
               OpDecorate %_arr_int_uint_10 ArrayStride 4
               OpMemberDecorate %_struct_4 0 Offset 0
               OpDecorate %_runtimearr__struct_4 ArrayStride 40
               OpMemberDecorate %_struct_6 0 Offset 0
               OpDecorate %_struct_6 BufferBlock
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
    %uint_5 = OpConstant %uint 5
    %uint_10 = OpConstant %uint 10
%_arr_int_uint_10 = OpTypeArray %int %uint_10
  %_struct_4 = OpTypeStruct %_arr_int_uint_10
%_runtimearr__struct_4 = OpTypeRuntimeArray %_struct_4
  %_struct_6 = OpTypeStruct %_runtimearr__struct_4
%_ptr_Uniform__struct_6 = OpTypePointer Uniform %_struct_6
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
%_ptr_Uniform__struct_4 = OpTypePointer Uniform %_struct_4
%_arr_int_uint_5 = OpTypeArray %int %uint_5
 %_struct_17 = OpTypeStruct %_arr_int_uint_5
%_ptr_Function__struct_17 = OpTypePointer Function %_struct_17
         %19 = OpTypeFunction %void %_ptr_Function__struct_17
          %2 = OpVariable %_ptr_Uniform__struct_6 Uniform
          %1 = OpFunction %void None %14
         %20 = OpLabel
         %21 = OpAccessChain %_ptr_Uniform__struct_4 %2 %int_0 %uint_0
         %22 = OpFunctionCall %void %23 %21
               OpReturn
               OpFunctionEnd
         %23 = OpFunction %void None %19
         %24 = OpFunctionParameter %_ptr_Function__struct_17
         %25 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetBeforeHlslLegalization(getValidatorOptions(), true);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("OpFunctionCall Argument <id>"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("type does not match Function <id>"));
}

INSTANTIATE_TEST_SUITE_P(StorageClass, ValidateFunctionCall,
                         Values("UniformConstant", "Input", "Uniform", "Output",
                                "Workgroup", "Private", "Function",
                                "PushConstant", "Image", "StorageBuffer",
                                "AtomicCounter"));
}  // namespace
}  // namespace val
}  // namespace spvtools
