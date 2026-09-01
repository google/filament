// Copyright (c) 2026 Google LLC
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

// Validation tests for explicit layout

#include <string>
#include <tuple>
#include <vector>

#include "gmock/gmock.h"
#include "source/val/decoration.h"
#include "spirv-tools/libspirv.h"
#include "test/unit_spirv.h"
#include "test/val/val_code_generator.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::Combine;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Values;

using ValidateExplicitLayout = spvtest::ValidateBase<bool>;

TEST_F(ValidateExplicitLayout, BlockMissingOffsetBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output Block
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
     %Output = OpTypeStruct %float
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be explicitly laid out with Offset or "
                        "OffsetIdEXT decorations"));
}

TEST_F(ValidateExplicitLayout, BufferBlockMissingOffsetBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output BufferBlock
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
     %Output = OpTypeStruct %float
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be explicitly laid out with Offset or "
                        "OffsetIdEXT decorations"));
}

TEST_F(ValidateExplicitLayout, BlockNestedStructMissingOffsetBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 16
               OpMemberDecorate %Output 2 Offset 32
               OpDecorate %Output Block
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v3float = OpTypeVector %float 3
        %int = OpTypeInt 32 1
          %S = OpTypeStruct %v3float %int
     %Output = OpTypeStruct %float %v4float %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be explicitly laid out with Offset or "
                        "OffsetIdEXT decorations"));
}

TEST_F(ValidateExplicitLayout, BufferBlockNestedStructMissingOffsetBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 16
               OpMemberDecorate %Output 2 Offset 32
               OpDecorate %Output BufferBlock
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v3float = OpTypeVector %float 3
        %int = OpTypeInt 32 1
          %S = OpTypeStruct %v3float %int
     %Output = OpTypeStruct %float %v4float %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be explicitly laid out with Offset or "
                        "OffsetIdEXT decorations"));
}

TEST_F(ValidateExplicitLayout, BlockMissingArrayStrideBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output Block
               OpMemberDecorate %Output 0 Offset 0
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
      %array = OpTypeArray %float %int_3
     %Output = OpTypeStruct %array
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be explicitly laid out with ArrayStride or "
                        "ArrayStrideIdEXT decorations"));
}

TEST_F(ValidateExplicitLayout, BufferBlockMissingArrayStrideBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output BufferBlock
               OpMemberDecorate %Output 0 Offset 0
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
      %array = OpTypeArray %float %int_3
     %Output = OpTypeStruct %array
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be explicitly laid out with ArrayStride or "
                        "ArrayStrideIdEXT decorations"));
}

TEST_F(ValidateExplicitLayout, BlockNestedStructMissingArrayStrideBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 16
               OpMemberDecorate %Output 2 Offset 32
               OpDecorate %Output Block
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
      %array = OpTypeArray %float %int_3
          %S = OpTypeStruct %array
     %Output = OpTypeStruct %float %v4float %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be explicitly laid out with ArrayStride or "
                        "ArrayStrideIdEXT decorations"));
}

TEST_F(ValidateExplicitLayout, BufferBlockNestedStructMissingArrayStrideBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 16
               OpMemberDecorate %Output 2 Offset 32
               OpDecorate %Output BufferBlock
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
      %array = OpTypeArray %float %int_3
          %S = OpTypeStruct %array
     %Output = OpTypeStruct %float %v4float %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be explicitly laid out with ArrayStride or "
                        "ArrayStrideIdEXT decorations"));
}

TEST_F(ValidateExplicitLayout, BlockMissingMatrixStrideBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output Block
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 0 ColMajor
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
     %matrix = OpTypeMatrix %v3float 4
     %Output = OpTypeStruct %matrix
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with MatrixStride decorations"));
}

TEST_F(ValidateExplicitLayout, BufferBlockMissingMatrixStrideBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output BufferBlock
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 0 ColMajor
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
     %matrix = OpTypeMatrix %v3float 4
     %Output = OpTypeStruct %matrix
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with MatrixStride decorations"));
}

TEST_F(ValidateExplicitLayout, BlockMissingMatrixStrideArrayBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output Block
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 0 RowMajor
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
     %matrix = OpTypeMatrix %v3float 4
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
      %array = OpTypeArray %matrix %int_3
     %Output = OpTypeStruct %matrix
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with MatrixStride decorations"));
}

TEST_F(ValidateExplicitLayout, BufferBlockMissingMatrixStrideArrayBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output BufferBlock
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 0 RowMajor
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
     %matrix = OpTypeMatrix %v3float 4
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
      %array = OpTypeArray %matrix %int_3
     %Output = OpTypeStruct %matrix
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with MatrixStride decorations"));
}

TEST_F(ValidateExplicitLayout, BlockNestedStructMissingMatrixStrideBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 0 ColMajor
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 16
               OpMemberDecorate %Output 2 Offset 32
               OpDecorate %Output Block
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
    %v4float = OpTypeVector %float 4
     %matrix = OpTypeMatrix %v3float 4
          %S = OpTypeStruct %matrix
     %Output = OpTypeStruct %float %v4float %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with MatrixStride decorations"));
}

TEST_F(ValidateExplicitLayout, BufferBlockNestedStructMissingMatrixStrideBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 0 ColMajor
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 16
               OpMemberDecorate %Output 2 Offset 32
               OpDecorate %Output BufferBlock
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
    %v4float = OpTypeVector %float 4
     %matrix = OpTypeMatrix %v3float 4
          %S = OpTypeStruct %matrix
     %Output = OpTypeStruct %float %v4float %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with MatrixStride decorations"));
}

TEST_F(ValidateExplicitLayout, BlockStandardUniformBufferLayout) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %F 0 Offset 0
               OpMemberDecorate %F 1 Offset 8
               OpDecorate %_arr_float_uint_2 ArrayStride 16
               OpDecorate %_arr_mat3v3float_uint_2 ArrayStride 48
               OpMemberDecorate %O 0 Offset 0
               OpMemberDecorate %O 1 Offset 16
               OpMemberDecorate %O 2 Offset 32
               OpMemberDecorate %O 3 Offset 64
               OpMemberDecorate %O 4 ColMajor
               OpMemberDecorate %O 4 Offset 80
               OpMemberDecorate %O 4 MatrixStride 16
               OpDecorate %_arr_O_uint_2 ArrayStride 176
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 8
               OpMemberDecorate %Output 2 Offset 16
               OpMemberDecorate %Output 3 Offset 32
               OpMemberDecorate %Output 4 Offset 48
               OpMemberDecorate %Output 5 Offset 64
               OpMemberDecorate %Output 6 ColMajor
               OpMemberDecorate %Output 6 Offset 96
               OpMemberDecorate %Output 6 MatrixStride 16
               OpMemberDecorate %Output 7 Offset 128
               OpDecorate %Output Block
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
          %F = OpTypeStruct %int %v2uint
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%mat2v3float = OpTypeMatrix %v3float 2
     %v3uint = OpTypeVector %uint 3
%mat3v3float = OpTypeMatrix %v3float 3
%_arr_mat3v3float_uint_2 = OpTypeArray %mat3v3float %uint_2
          %O = OpTypeStruct %v3uint %v2float %_arr_float_uint_2 %v2float %_arr_mat3v3float_uint_2
%_arr_O_uint_2 = OpTypeArray %O %uint_2
     %Output = OpTypeStruct %float %v2float %v3float %F %float %_arr_float_uint_2 %mat2v3float %_arr_O_uint_2
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateExplicitLayout, BlockLayoutPermitsTightVec3ScalarPackingGood) {
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/1666
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 12
               OpDecorate %S Block
               OpDecorate %B DescriptorSet 0
               OpDecorate %B Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %S = OpTypeStruct %v3float %float
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %B = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0))
      << getDiagnosticString();
}

TEST_F(ValidateExplicitLayout, BlockLayoutForbidsTightScalarVec3PackingBad) {
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/1666
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %S Block
               OpDecorate %B DescriptorSet 0
               OpDecorate %B Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %S = OpTypeStruct %float %v3float
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %B = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 4 is not aligned to 16"));
}

TEST_F(ValidateExplicitLayout,
       BlockLayoutPermitsTightScalarVec3PackingWithRelaxedLayoutGood) {
  // Same as previous test, but with explicit option to relax block layout.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %S Block
               OpDecorate %B DescriptorSet 0
               OpDecorate %B Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %S = OpTypeStruct %float %v3float
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %B = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetRelaxBlockLayout(getValidatorOptions(), true);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(ValidateExplicitLayout,
       BlockLayoutPermitsTightScalarVec3PackingBadOffsetWithRelaxedLayoutBad) {
  // Same as previous test, but with the vector not aligned to its scalar
  // element. Use offset 5 instead of a multiple of 4.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 5
               OpDecorate %S Block
               OpDecorate %B DescriptorSet 0
               OpDecorate %B Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %S = OpTypeStruct %float %v3float
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %B = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetRelaxBlockLayout(getValidatorOptions(), true);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 5 is not aligned to 4"));
}

TEST_F(ValidateExplicitLayout,
       BlockLayoutPermitsTightScalarVec3PackingWithVulkan1_1Good) {
  // Same as previous test, but with Vulkan 1.1.  Vulkan 1.1 included
  // VK_KHR_relaxed_block_layout in core.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %S Block
               OpDecorate %B DescriptorSet 0
               OpDecorate %B Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %S = OpTypeStruct %float %v3float
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %B = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(ValidateExplicitLayout,
       BlockLayoutPermitsTightScalarVec3PackingWithScalarLayoutGood) {
  // Same as previous test, but with scalar block layout.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %S Block
               OpDecorate %B DescriptorSet 0
               OpDecorate %B Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %S = OpTypeStruct %float %v3float
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %B = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetScalarBlockLayout(getValidatorOptions(), true);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(ValidateExplicitLayout,
       BlockLayoutPermitsScalarAlignedArrayWithScalarLayoutGood) {
  // The array at offset 4 is ok with scalar block layout.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %S Block
               OpDecorate %B DescriptorSet 0
               OpDecorate %B Binding 0
               OpDecorate %arr_float ArrayStride 4
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
  %arr_float = OpTypeArray %float %uint_3
          %S = OpTypeStruct %float %arr_float
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %B = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetScalarBlockLayout(getValidatorOptions(), true);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(ValidateExplicitLayout,
       BlockLayoutPermitsScalarAlignedArrayOfVec3WithScalarLayoutGood) {
  // The array at offset 4 is ok with scalar block layout, even though
  // its elements are vec3.
  // This is the same as the previous case, but the array elements are vec3
  // instead of float.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %S Block
               OpDecorate %B DescriptorSet 0
               OpDecorate %B Binding 0
               OpDecorate %arr_vec3 ArrayStride 12
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
       %vec3 = OpTypeVector %float 3
   %arr_vec3 = OpTypeArray %vec3 %uint_3
          %S = OpTypeStruct %float %arr_vec3
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %B = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetScalarBlockLayout(getValidatorOptions(), true);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(ValidateExplicitLayout,
       BlockLayoutPermitsScalarAlignedStructWithScalarLayoutGood) {
  // Scalar block layout permits the struct at offset 4, even though
  // it contains a vector with base alignment 8 and scalar alignment 4.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpMemberDecorate %st 0 Offset 0
               OpMemberDecorate %st 1 Offset 8
               OpDecorate %S Block
               OpDecorate %B DescriptorSet 0
               OpDecorate %B Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %vec2 = OpTypeVector %float 2
        %st  = OpTypeStruct %vec2 %float
          %S = OpTypeStruct %float %st
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %B = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetScalarBlockLayout(getValidatorOptions(), true);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(
    ValidateExplicitLayout,
    BlockLayoutPermitsFieldsInBaseAlignmentPaddingAtEndOfStructWithScalarLayoutGood) {
  // Scalar block layout permits fields in what would normally be the padding at
  // the end of a struct.
  std::string spirv = R"(
               OpCapability Shader
               OpCapability Float64
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %st 0 Offset 0
               OpMemberDecorate %st 1 Offset 8
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 12
               OpDecorate %S Block
               OpDecorate %B DescriptorSet 0
               OpDecorate %B Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
     %double = OpTypeFloat 64
         %st = OpTypeStruct %double %float
          %S = OpTypeStruct %st %float
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %B = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetScalarBlockLayout(getValidatorOptions(), true);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(
    ValidateExplicitLayout,
    BlockLayoutPermitsStraddlingVectorWithScalarLayoutOverrideRelaxBlockLayoutGood) {
  // Same as previous, but set relaxed block layout first.  Scalar layout always
  // wins.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %S Block
               OpDecorate %B DescriptorSet 0
               OpDecorate %B Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %vec4 = OpTypeVector %float 4
          %S = OpTypeStruct %float %vec4
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %B = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetRelaxBlockLayout(getValidatorOptions(), true);
  spvValidatorOptionsSetScalarBlockLayout(getValidatorOptions(), true);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(
    ValidateExplicitLayout,
    BlockLayoutPermitsStraddlingVectorWithRelaxedLayoutOverridenByScalarBlockLayoutGood) {
  // Same as previous, but set scalar block layout first.  Scalar layout always
  // wins.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %S Block
               OpDecorate %B DescriptorSet 0
               OpDecorate %B Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %vec4 = OpTypeVector %float 4
          %S = OpTypeStruct %float %vec4
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %B = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetScalarBlockLayout(getValidatorOptions(), true);
  spvValidatorOptionsSetRelaxBlockLayout(getValidatorOptions(), true);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(ValidateExplicitLayout, BufferBlock16bitStandardStorageBufferLayout) {
  std::string spirv = R"(
             OpCapability Shader
             OpCapability StorageUniform16
             OpExtension "SPV_KHR_16bit_storage"
             OpMemoryModel Logical GLSL450
             OpEntryPoint GLCompute %main "main"
             OpExecutionMode %main LocalSize 1 1 1
             OpDecorate %f32arr ArrayStride 4
             OpDecorate %f16arr ArrayStride 2
             OpMemberDecorate %SSBO32 0 Offset 0
             OpMemberDecorate %SSBO16 0 Offset 0
             OpDecorate %SSBO32 BufferBlock
             OpDecorate %SSBO16 BufferBlock
             OpDecorate %varSSBO32 DescriptorSet 0
             OpDecorate %varSSBO32 Binding 0
             OpDecorate %varSSBO16 DescriptorSet 0
             OpDecorate %varSSBO16 Binding 1
     %void = OpTypeVoid
    %voidf = OpTypeFunction %void
      %u32 = OpTypeInt 32 0
      %i32 = OpTypeInt 32 1
      %f32 = OpTypeFloat 32
    %uvec3 = OpTypeVector %u32 3
 %c_i32_32 = OpConstant %i32 32
%c_i32_128 = OpConstant %i32 128
   %f32arr = OpTypeArray %f32 %c_i32_128
      %f16 = OpTypeFloat 16
   %f16arr = OpTypeArray %f16 %c_i32_128
   %SSBO32 = OpTypeStruct %f32arr
   %SSBO16 = OpTypeStruct %f16arr
%_ptr_Uniform_SSBO32 = OpTypePointer Uniform %SSBO32
 %varSSBO32 = OpVariable %_ptr_Uniform_SSBO32 Uniform
%_ptr_Uniform_SSBO16 = OpTypePointer Uniform %SSBO16
 %varSSBO16 = OpVariable %_ptr_Uniform_SSBO16 Uniform
     %main = OpFunction %void None %voidf
    %label = OpLabel
             OpReturn
             OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateExplicitLayout, BlockArrayExtendedAlignmentGood) {
  // For uniform buffer, Array base alignment is 16, and ArrayStride
  // must be a multiple of 16.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpDecorate %_arr_float_uint_2 ArrayStride 16
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 16
               OpDecorate %S Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
          %S = OpTypeStruct %v2float %_arr_float_uint_2
%_ptr_PushConstant_S = OpTypePointer PushConstant %S
          %u = OpVariable %_ptr_PushConstant_S PushConstant
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0))
      << getDiagnosticString();
}

TEST_F(ValidateExplicitLayout, BlockArrayBaseAlignmentBad) {
  // For uniform buffer, Array base alignment is 16.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpDecorate %_arr_float_uint_2 ArrayStride 16
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 8
               OpDecorate %S Block
               OpDecorate %u DescriptorSet 0
               OpDecorate %u Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
          %S = OpTypeStruct %v2float %_arr_float_uint_2
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %u = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 8 is not aligned to 16"));
}

TEST_F(ValidateExplicitLayout,
       BlockArrayBaseAlignmentWithRelaxedLayoutStillBad) {
  // For uniform buffer, Array base alignment is 16, and ArrayStride
  // must be a multiple of 16.  This case uses relaxed block layout.  Relaxed
  // layout only relaxes rules for vector alignment, not array alignment.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpDecorate %_arr_float_uint_2 ArrayStride 16
               OpDecorate %u DescriptorSet 0
               OpDecorate %u Binding 0
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 8
               OpDecorate %S Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
          %S = OpTypeStruct %v2float %_arr_float_uint_2
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %u = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetRelaxBlockLayout(getValidatorOptions(), true);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 8 is not aligned to 16"));
}

TEST_F(ValidateExplicitLayout, BlockArrayBaseAlignmentWithVulkan1_1StillBad) {
  // Same as previous test, but with Vulkan 1.1, which includes
  // VK_KHR_relaxed_block_layout in core.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpDecorate %_arr_float_uint_2 ArrayStride 16
               OpDecorate %u DescriptorSet 0
               OpDecorate %u Binding 0
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 8
               OpDecorate %S Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
          %S = OpTypeStruct %v2float %_arr_float_uint_2
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %u = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 8 is not aligned to 16"));
}

TEST_F(ValidateExplicitLayout,
       BlockArrayBaseAlignmentWithBlockStandardLayoutGood) {
  // Same as previous test, but with VK_KHR_uniform_buffer_standard_layout
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpDecorate %_arr_float_uint_2 ArrayStride 16
               OpDecorate %u DescriptorSet 0
               OpDecorate %u Binding 0
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 8
               OpDecorate %S Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
          %S = OpTypeStruct %v2float %_arr_float_uint_2
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %u = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetUniformBufferStandardLayout(getValidatorOptions(),
                                                    true);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(ValidateExplicitLayout, PushConstantArrayBaseAlignmentGood) {
  // Tests https://github.com/KhronosGroup/SPIRV-Tools/issues/1664
  // From GLSL vertex shader:
  // #version 450
  // layout(push_constant) uniform S { vec2 v; float arr[2]; } u;
  // void main() { }

  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpDecorate %_arr_float_uint_2 ArrayStride 4
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 8
               OpDecorate %S Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
          %S = OpTypeStruct %v2float %_arr_float_uint_2
%_ptr_PushConstant_S = OpTypePointer PushConstant %S
          %u = OpVariable %_ptr_PushConstant_S PushConstant
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0))
      << getDiagnosticString();
}

TEST_F(ValidateExplicitLayout, PushConstantArrayBadAlignmentBad) {
  // Like the previous test, but with offset 7 instead of 8.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpDecorate %_arr_float_uint_2 ArrayStride 4
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 7
               OpDecorate %S Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
          %S = OpTypeStruct %v2float %_arr_float_uint_2
%_ptr_PushConstant_S = OpTypePointer PushConstant %S
          %u = OpVariable %_ptr_PushConstant_S PushConstant
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 7 is not aligned to 4"));
}

TEST_F(ValidateExplicitLayout,
       PushConstantLayoutPermitsTightVec3ScalarPackingGood) {
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/1666
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 12
               OpDecorate %S Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %S = OpTypeStruct %v3float %float
%_ptr_PushConstant_S = OpTypePointer PushConstant %S
          %B = OpVariable %_ptr_PushConstant_S PushConstant
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0))
      << getDiagnosticString();
}

TEST_F(ValidateExplicitLayout,
       PushConstantLayoutForbidsTightScalarVec3PackingBad) {
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/1666
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %S Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %S = OpTypeStruct %float %v3float
%_ptr_Uniform_S = OpTypePointer PushConstant %S
          %B = OpVariable %_ptr_Uniform_S PushConstant
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 4 is not aligned to 16"));
}

TEST_F(ValidateExplicitLayout,
       StorageBufferStorageClassArrayBaseAlignmentGood) {
  // Spot check buffer rules when using StorageBuffer storage class with Block
  // decoration.
  std::string spirv = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpDecorate %_arr_float_uint_2 ArrayStride 4
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 8
               OpDecorate %S Block
               OpDecorate %u DescriptorSet 0
               OpDecorate %u Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
          %S = OpTypeStruct %v2float %_arr_float_uint_2
%_ptr_Uniform_S = OpTypePointer StorageBuffer %S
          %u = OpVariable %_ptr_Uniform_S StorageBuffer
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0))
      << getDiagnosticString();
}

TEST_F(ValidateExplicitLayout, StorageBufferStorageClassArrayBadAlignmentBad) {
  // Like the previous test, but with offset 7.
  std::string spirv = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpDecorate %_arr_float_uint_2 ArrayStride 4
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 7
               OpDecorate %S Block
               OpDecorate %u DescriptorSet 0
               OpDecorate %u Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
          %S = OpTypeStruct %v2float %_arr_float_uint_2
%_ptr_Uniform_S = OpTypePointer StorageBuffer %S
          %u = OpVariable %_ptr_Uniform_S StorageBuffer
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 7 is not aligned to 4"));
}

TEST_F(ValidateExplicitLayout, BufferBlockStandardStorageBufferLayout) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %F 0 Offset 0
               OpMemberDecorate %F 1 Offset 8
               OpDecorate %_arr_float_uint_2 ArrayStride 4
               OpDecorate %_arr_mat3v3float_uint_2 ArrayStride 48
               OpMemberDecorate %O 0 Offset 0
               OpMemberDecorate %O 1 Offset 16
               OpMemberDecorate %O 2 Offset 24
               OpMemberDecorate %O 3 Offset 32
               OpMemberDecorate %O 4 ColMajor
               OpMemberDecorate %O 4 Offset 48
               OpMemberDecorate %O 4 MatrixStride 16
               OpDecorate %_arr_O_uint_2 ArrayStride 144
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 8
               OpMemberDecorate %Output 2 Offset 16
               OpMemberDecorate %Output 3 Offset 32
               OpMemberDecorate %Output 4 Offset 48
               OpMemberDecorate %Output 5 Offset 52
               OpMemberDecorate %Output 6 ColMajor
               OpMemberDecorate %Output 6 Offset 64
               OpMemberDecorate %Output 6 MatrixStride 16
               OpMemberDecorate %Output 7 Offset 96
               OpDecorate %Output BufferBlock
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
          %F = OpTypeStruct %int %v2uint
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%mat2v3float = OpTypeMatrix %v3float 2
     %v3uint = OpTypeVector %uint 3
%mat3v3float = OpTypeMatrix %v3float 3
%_arr_mat3v3float_uint_2 = OpTypeArray %mat3v3float %uint_2
          %O = OpTypeStruct %v3uint %v2float %_arr_float_uint_2 %v2float %_arr_mat3v3float_uint_2
%_arr_O_uint_2 = OpTypeArray %O %uint_2
     %Output = OpTypeStruct %float %v2float %v3float %F %float %_arr_float_uint_2 %mat2v3float %_arr_O_uint_2
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateExplicitLayout,
       StorageBufferLayoutPermitsTightVec3ScalarPackingGood) {
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/1666
  std::string spirv = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 12
               OpDecorate %S Block
               OpDecorate %B DescriptorSet 0
               OpDecorate %B Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %S = OpTypeStruct %v3float %float
%_ptr_StorageBuffer_S = OpTypePointer StorageBuffer %S
          %B = OpVariable %_ptr_StorageBuffer_S StorageBuffer
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0))
      << getDiagnosticString();
}

TEST_F(ValidateExplicitLayout,
       StorageBufferLayoutForbidsTightScalarVec3PackingBad) {
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/1666
  std::string spirv = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %S Block
               OpDecorate %B DescriptorSet 0
               OpDecorate %B Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %S = OpTypeStruct %float %v3float
%_ptr_StorageBuffer_S = OpTypePointer StorageBuffer %S
          %B = OpVariable %_ptr_StorageBuffer_S StorageBuffer
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 4 is not aligned to 16"));
}

TEST_F(ValidateExplicitLayout,
       BlockStandardUniformBufferLayoutIncorrectOffset0Bad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %F 0 Offset 0
               OpMemberDecorate %F 1 Offset 8
               OpDecorate %_arr_float_uint_2 ArrayStride 16
               OpDecorate %_arr_mat3v3float_uint_2 ArrayStride 48
               OpMemberDecorate %O 0 Offset 0
               OpMemberDecorate %O 1 Offset 16
               OpMemberDecorate %O 2 Offset 24
               OpMemberDecorate %O 3 Offset 33
               OpMemberDecorate %O 4 ColMajor
               OpMemberDecorate %O 4 Offset 80
               OpMemberDecorate %O 4 MatrixStride 16
               OpDecorate %_arr_O_uint_2 ArrayStride 176
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 8
               OpMemberDecorate %Output 2 Offset 16
               OpMemberDecorate %Output 3 Offset 32
               OpMemberDecorate %Output 4 Offset 48
               OpMemberDecorate %Output 5 Offset 64
               OpMemberDecorate %Output 6 ColMajor
               OpMemberDecorate %Output 6 Offset 96
               OpMemberDecorate %Output 6 MatrixStride 16
               OpMemberDecorate %Output 7 Offset 128
               OpDecorate %Output Block
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
          %F = OpTypeStruct %int %v2uint
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%mat2v3float = OpTypeMatrix %v3float 2
     %v3uint = OpTypeVector %uint 3
%mat3v3float = OpTypeMatrix %v3float 3
%_arr_mat3v3float_uint_2 = OpTypeArray %mat3v3float %uint_2
          %O = OpTypeStruct %v3uint %v2float %_arr_float_uint_2 %v2float %_arr_mat3v3float_uint_2
%_arr_O_uint_2 = OpTypeArray %O %uint_2
     %Output = OpTypeStruct %float %v2float %v3float %F %float %_arr_float_uint_2 %mat2v3float %_arr_O_uint_2
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Structure member 2 at offset 24 is not aligned to 16"));
}

TEST_F(ValidateExplicitLayout,
       BlockStandardUniformBufferLayoutIncorrectOffset1Bad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %F 0 Offset 0
               OpMemberDecorate %F 1 Offset 8
               OpDecorate %_arr_float_uint_2 ArrayStride 16
               OpDecorate %_arr_mat3v3float_uint_2 ArrayStride 48
               OpMemberDecorate %O 0 Offset 0
               OpMemberDecorate %O 1 Offset 16
               OpMemberDecorate %O 2 Offset 32
               OpMemberDecorate %O 3 Offset 64
               OpMemberDecorate %O 4 ColMajor
               OpMemberDecorate %O 4 Offset 80
               OpMemberDecorate %O 4 MatrixStride 16
               OpDecorate %_arr_O_uint_2 ArrayStride 176
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 8
               OpMemberDecorate %Output 2 Offset 16
               OpMemberDecorate %Output 3 Offset 32
               OpMemberDecorate %Output 4 Offset 48
               OpMemberDecorate %Output 5 Offset 71
               OpMemberDecorate %Output 6 ColMajor
               OpMemberDecorate %Output 6 Offset 96
               OpMemberDecorate %Output 6 MatrixStride 16
               OpMemberDecorate %Output 7 Offset 128
               OpDecorate %Output Block
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
          %F = OpTypeStruct %int %v2uint
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%mat2v3float = OpTypeMatrix %v3float 2
     %v3uint = OpTypeVector %uint 3
%mat3v3float = OpTypeMatrix %v3float 3
%_arr_mat3v3float_uint_2 = OpTypeArray %mat3v3float %uint_2
          %O = OpTypeStruct %v3uint %v2float %_arr_float_uint_2 %v2float %_arr_mat3v3float_uint_2
%_arr_O_uint_2 = OpTypeArray %O %uint_2
     %Output = OpTypeStruct %float %v2float %v3float %F %float %_arr_float_uint_2 %mat2v3float %_arr_O_uint_2
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Structure member 5 at offset 71 is not aligned to 16"));
}

TEST_F(ValidateExplicitLayout,
       BlockUniformBufferLayoutIncorrectArrayStrideBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %F 0 Offset 0
               OpMemberDecorate %F 1 Offset 8
               OpDecorate %_arr_float_uint_2 ArrayStride 16
               OpDecorate %_arr_mat3v3float_uint_2 ArrayStride 49
               OpMemberDecorate %O 0 Offset 0
               OpMemberDecorate %O 1 Offset 16
               OpMemberDecorate %O 2 Offset 32
               OpMemberDecorate %O 3 Offset 64
               OpMemberDecorate %O 4 ColMajor
               OpMemberDecorate %O 4 Offset 80
               OpMemberDecorate %O 4 MatrixStride 16
               OpDecorate %_arr_O_uint_2 ArrayStride 177
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 8
               OpMemberDecorate %Output 2 Offset 16
               OpMemberDecorate %Output 3 Offset 32
               OpMemberDecorate %Output 4 Offset 48
               OpMemberDecorate %Output 5 Offset 64
               OpMemberDecorate %Output 6 ColMajor
               OpMemberDecorate %Output 6 Offset 96
               OpMemberDecorate %Output 6 MatrixStride 16
               OpMemberDecorate %Output 7 Offset 128
               OpDecorate %Output Block
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
          %F = OpTypeStruct %int %v2uint
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%mat2v3float = OpTypeMatrix %v3float 2
     %v3uint = OpTypeVector %uint 3
%mat3v3float = OpTypeMatrix %v3float 3
%_arr_mat3v3float_uint_2 = OpTypeArray %mat3v3float %uint_2
          %O = OpTypeStruct %v3uint %v2float %_arr_float_uint_2 %v2float %_arr_mat3v3float_uint_2
%_arr_O_uint_2 = OpTypeArray %O %uint_2
     %Output = OpTypeStruct %float %v2float %v3float %F %float %_arr_float_uint_2 %mat2v3float %_arr_O_uint_2
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Array stride 177 must satisfy alignment 16"));
}

TEST_F(ValidateExplicitLayout,
       BufferBlockStandardStorageBufferLayoutImproperStraddleBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 8
               OpDecorate %Output BufferBlock
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
     %Output = OpTypeStruct %float %v3float
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 8 is not aligned to 16"));
}

TEST_F(ValidateExplicitLayout,
       BlockUniformBufferLayoutOffsetInsideArrayPaddingBad) {
  // In this case the 2nd member fits entirely within the padding.
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %_arr_float_uint_2 ArrayStride 16
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 20
               OpDecorate %Output Block
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
     %Output = OpTypeStruct %_arr_float_uint_2 %float
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 20 overlaps previous "
                        "member ending at offset 31"));
}

TEST_F(ValidateExplicitLayout,
       BlockUniformBufferLayoutOffsetInsideStructPaddingBad) {
  // In this case the 2nd member fits entirely within the padding.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpMemberDecorate %_struct_6 0 Offset 0
               OpMemberDecorate %_struct_2 0 Offset 0
               OpMemberDecorate %_struct_2 1 Offset 4
               OpDecorate %_struct_2 Block
               OpDecorate %8 DescriptorSet 0
               OpDecorate %8 Binding 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
      %float = OpTypeFloat 32
  %_struct_6 = OpTypeStruct %float
  %_struct_2 = OpTypeStruct %_struct_6 %float
%_ptr_Uniform__struct_2 = OpTypePointer Uniform %_struct_2
          %8 = OpVariable %_ptr_Uniform__struct_2 Uniform
          %1 = OpFunction %void None %4
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 4 overlaps previous "
                        "member ending at offset 15"));
}

TEST_F(ValidateExplicitLayout, BlockLayoutOffsetOutOfOrderGoodUniversal1_0) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %Outer 0 Offset 4
               OpMemberDecorate %Outer 1 Offset 0
               OpDecorate %Outer Block
               OpDecorate %O DescriptorSet 0
               OpDecorate %O Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %Outer = OpTypeStruct %uint %uint
%_ptr_Uniform_Outer = OpTypePointer Uniform %Outer
          %O = OpVariable %_ptr_Uniform_Outer Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_UNIVERSAL_1_0));
}

TEST_F(ValidateExplicitLayout, BlockLayoutOffsetOutOfOrderGoodOpenGL4_5) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %Outer 0 Offset 4
               OpMemberDecorate %Outer 1 Offset 0
               OpDecorate %Outer Block
               OpDecorate %O DescriptorSet 0
               OpDecorate %O Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %Outer = OpTypeStruct %uint %uint
%_ptr_Uniform_Outer = OpTypePointer Uniform %Outer
          %O = OpVariable %_ptr_Uniform_Outer Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_OPENGL_4_5));
}

TEST_F(ValidateExplicitLayout, BlockLayoutOffsetOutOfOrderGoodVulkan1_1) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %Outer 0 Offset 4
               OpMemberDecorate %Outer 1 Offset 0
               OpDecorate %Outer Block
               OpDecorate %O DescriptorSet 0
               OpDecorate %O Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %Outer = OpTypeStruct %uint %uint
%_ptr_Uniform_Outer = OpTypePointer Uniform %Outer
          %O = OpVariable %_ptr_Uniform_Outer Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_1))
      << getDiagnosticString();
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(ValidateExplicitLayout, BlockLayoutOffsetOverlapBad) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %Outer 0 Offset 0
               OpMemberDecorate %Outer 1 Offset 16
               OpMemberDecorate %Inner 0 Offset 0
               OpMemberDecorate %Inner 1 Offset 16
               OpDecorate %Outer Block
               OpDecorate %O DescriptorSet 0
               OpDecorate %O Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %Inner = OpTypeStruct %uint %uint
      %Outer = OpTypeStruct %Inner %uint
%_ptr_Uniform_Outer = OpTypePointer Uniform %Outer
          %O = OpVariable %_ptr_Uniform_Outer Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 16 overlaps previous "
                        "member ending at offset 31"));
}

TEST_F(ValidateExplicitLayout, BufferBlockEmptyStruct) {
  std::string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %Output 0 Offset 0
               OpDecorate %Output BufferBlock
               OpDecorate %dataOutput DescriptorSet 0
               OpDecorate %dataOutput Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
          %S = OpTypeStruct
     %Output = OpTypeStruct %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateExplicitLayout, RowMajorMatrixTightPackingGood) {
  // Row major matrix rule:
  //     A row-major matrix of C columns has a base alignment equal to
  //     the base alignment of a vector of C matrix components.
  // Note: The "matrix component" is the scalar element type.

  // The matrix has 3 columns and 2 rows (C=3, R=2).
  // So the base alignment of b is the same as a vector of 3 floats, which is 16
  // bytes. The matrix consists of two of these, and therefore occupies 2 x 16
  // bytes, or 32 bytes.
  //
  // So the offsets can be:
  // a -> 0
  // b -> 16
  // c -> 48
  // d -> 60 ; d fits at bytes 12-15 after offset of c. Tight (vec3;float)
  // packing

  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %1 "main"
               OpSource GLSL 450
               OpMemberDecorate %_struct_2 0 Offset 0
               OpMemberDecorate %_struct_2 1 RowMajor
               OpMemberDecorate %_struct_2 1 Offset 16
               OpMemberDecorate %_struct_2 1 MatrixStride 16
               OpMemberDecorate %_struct_2 2 Offset 48
               OpMemberDecorate %_struct_2 3 Offset 60
               OpDecorate %_struct_2 Block
               OpDecorate %3 DescriptorSet 0
               OpDecorate %3 Binding 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v2float = OpTypeVector %float 2
%mat3v2float = OpTypeMatrix %v2float 3
    %v3float = OpTypeVector %float 3
  %_struct_2 = OpTypeStruct %v4float %mat3v2float %v3float %float
%_ptr_Uniform__struct_2 = OpTypePointer Uniform %_struct_2
          %3 = OpVariable %_ptr_Uniform__struct_2 Uniform
          %1 = OpFunction %void None %5
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0))
      << getDiagnosticString();
}

TEST_F(ValidateExplicitLayout, ArrayArrayRowMajorMatrixTightPackingGood) {
  // Like the previous case, but we have an array of arrays of matrices.
  // The RowMajor decoration goes on the struct member (surprisingly).

  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %1 "main"
               OpSource GLSL 450
               OpMemberDecorate %_struct_2 0 Offset 0
               OpMemberDecorate %_struct_2 1 RowMajor
               OpMemberDecorate %_struct_2 1 Offset 16
               OpMemberDecorate %_struct_2 1 MatrixStride 16
               OpMemberDecorate %_struct_2 2 Offset 80
               OpMemberDecorate %_struct_2 3 Offset 92
               OpDecorate %arr_mat ArrayStride 32
               OpDecorate %arr_arr_mat ArrayStride 32
               OpDecorate %_struct_2 Block
               OpDecorate %3 DescriptorSet 0
               OpDecorate %3 Binding 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v2float = OpTypeVector %float 2
%mat3v2float = OpTypeMatrix %v2float 3
%uint        = OpTypeInt 32 0
%uint_1      = OpConstant %uint 1
%uint_2      = OpConstant %uint 2
    %arr_mat = OpTypeArray %mat3v2float %uint_1
%arr_arr_mat = OpTypeArray %arr_mat %uint_2
    %v3float = OpTypeVector %float 3
  %_struct_2 = OpTypeStruct %v4float %arr_arr_mat %v3float %float
%_ptr_Uniform__struct_2 = OpTypePointer Uniform %_struct_2
          %3 = OpVariable %_ptr_Uniform__struct_2 Uniform
          %1 = OpFunction %void None %5
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0))
      << getDiagnosticString();
}

TEST_F(ValidateExplicitLayout, ArrayArrayRowMajorMatrixNextMemberOverlapsBad) {
  // Like the previous case, but the offset of member 2 overlaps the matrix.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %1 "main"
               OpSource GLSL 450
               OpMemberDecorate %_struct_2 0 Offset 0
               OpMemberDecorate %_struct_2 1 RowMajor
               OpMemberDecorate %_struct_2 1 Offset 16
               OpMemberDecorate %_struct_2 1 MatrixStride 16
               OpMemberDecorate %_struct_2 2 Offset 64
               OpMemberDecorate %_struct_2 3 Offset 92
               OpDecorate %arr_mat ArrayStride 32
               OpDecorate %arr_arr_mat ArrayStride 32
               OpDecorate %_struct_2 Block
               OpDecorate %3 DescriptorSet 0
               OpDecorate %3 Binding 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v2float = OpTypeVector %float 2
%mat3v2float = OpTypeMatrix %v2float 3
%uint        = OpTypeInt 32 0
%uint_1      = OpConstant %uint 1
%uint_2      = OpConstant %uint 2
    %arr_mat = OpTypeArray %mat3v2float %uint_1
%arr_arr_mat = OpTypeArray %arr_mat %uint_2
    %v3float = OpTypeVector %float 3
  %_struct_2 = OpTypeStruct %v4float %arr_arr_mat %v3float %float
%_ptr_Uniform__struct_2 = OpTypePointer Uniform %_struct_2
          %3 = OpVariable %_ptr_Uniform__struct_2 Uniform
          %1 = OpFunction %void None %5
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 2 at offset 64 overlaps previous "
                        "member ending at offset 79"));
}

TEST_F(ValidateExplicitLayout, StorageBufferArraySizeCalculationPackGood) {
  // Original GLSL

  // #version 450
  // layout (set=0,binding=0) buffer S {
  //   uvec3 arr[2][2]; // first 3 elements are 16 bytes, last is 12
  //   uint i;  // Can't have offset 60 = 3x16 + 12
  // } B;
  // void main() {}

  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %1 "main"
               OpDecorate %_arr_v3uint_uint_2 ArrayStride 16
               OpDecorate %_arr__arr_v3uint_uint_2_uint_2 ArrayStride 32
               OpMemberDecorate %_struct_4 0 Offset 0
               OpMemberDecorate %_struct_4 1 Offset 64
               OpDecorate %_struct_4 BufferBlock
               OpDecorate %5 DescriptorSet 0
               OpDecorate %5 Binding 0
       %void = OpTypeVoid
          %7 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
     %uint_2 = OpConstant %uint 2
%_arr_v3uint_uint_2 = OpTypeArray %v3uint %uint_2
%_arr__arr_v3uint_uint_2_uint_2 = OpTypeArray %_arr_v3uint_uint_2 %uint_2
  %_struct_4 = OpTypeStruct %_arr__arr_v3uint_uint_2_uint_2 %uint
%_ptr_Uniform__struct_4 = OpTypePointer Uniform %_struct_4
          %5 = OpVariable %_ptr_Uniform__struct_4 Uniform
          %1 = OpFunction %void None %7
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateExplicitLayout,
       StorageBufferArraySizeCalculationPackGoodScalar) {
  // Original GLSL

  // #version 450
  // layout (set=0,binding=0) buffer S {
  //   uvec3 arr[2][2]; // first 3 elements are 16 bytes, last is 12
  //   uint i;  // Can have offset 60 = 3x16 + 12
  // } B;
  // void main() {}

  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %1 "main"
               OpDecorate %_arr_v3uint_uint_2 ArrayStride 16
               OpDecorate %_arr__arr_v3uint_uint_2_uint_2 ArrayStride 32
               OpMemberDecorate %_struct_4 0 Offset 0
               OpMemberDecorate %_struct_4 1 Offset 60
               OpDecorate %_struct_4 BufferBlock
               OpDecorate %5 DescriptorSet 0
               OpDecorate %5 Binding 0
       %void = OpTypeVoid
          %7 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
     %uint_2 = OpConstant %uint 2
%_arr_v3uint_uint_2 = OpTypeArray %v3uint %uint_2
%_arr__arr_v3uint_uint_2_uint_2 = OpTypeArray %_arr_v3uint_uint_2 %uint_2
  %_struct_4 = OpTypeStruct %_arr__arr_v3uint_uint_2_uint_2 %uint
%_ptr_Uniform__struct_4 = OpTypePointer Uniform %_struct_4
          %5 = OpVariable %_ptr_Uniform__struct_4 Uniform
          %1 = OpFunction %void None %7
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  options_->scalar_block_layout = true;
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateExplicitLayout, StorageBufferArraySizeCalculationPackBad) {
  // Like previous but, the offset of the second member is too small.

  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %1 "main"
               OpDecorate %_arr_v3uint_uint_2 ArrayStride 16
               OpDecorate %_arr__arr_v3uint_uint_2_uint_2 ArrayStride 32
               OpMemberDecorate %_struct_4 0 Offset 0
               OpMemberDecorate %_struct_4 1 Offset 60
               OpDecorate %_struct_4 BufferBlock
               OpDecorate %5 DescriptorSet 0
               OpDecorate %5 Binding 0
       %void = OpTypeVoid
          %7 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
     %uint_2 = OpConstant %uint 2
%_arr_v3uint_uint_2 = OpTypeArray %v3uint %uint_2
%_arr__arr_v3uint_uint_2_uint_2 = OpTypeArray %_arr_v3uint_uint_2 %uint_2
  %_struct_4 = OpTypeStruct %_arr__arr_v3uint_uint_2_uint_2 %uint
%_ptr_Uniform__struct_4 = OpTypePointer Uniform %_struct_4
          %5 = OpVariable %_ptr_Uniform__struct_4 Uniform
          %1 = OpFunction %void None %7
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 60 overlaps "
                        "previous member ending at offset 63"));
}

TEST_F(ValidateExplicitLayout, UniformBufferArraySizeCalculationPackGood) {
  // Like the corresponding buffer block case, but the array padding must
  // count for the last element as well, and so the offset of the second
  // member must be at least 64.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %1 "main"
               OpDecorate %_arr_v3uint_uint_2 ArrayStride 16
               OpDecorate %_arr__arr_v3uint_uint_2_uint_2 ArrayStride 32
               OpMemberDecorate %_struct_4 0 Offset 0
               OpMemberDecorate %_struct_4 1 Offset 64
               OpDecorate %_struct_4 Block
               OpDecorate %5 DescriptorSet 0
               OpDecorate %5 Binding 0
       %void = OpTypeVoid
          %7 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
     %uint_2 = OpConstant %uint 2
%_arr_v3uint_uint_2 = OpTypeArray %v3uint %uint_2
%_arr__arr_v3uint_uint_2_uint_2 = OpTypeArray %_arr_v3uint_uint_2 %uint_2
  %_struct_4 = OpTypeStruct %_arr__arr_v3uint_uint_2_uint_2 %uint
%_ptr_Uniform__struct_4 = OpTypePointer Uniform %_struct_4
          %5 = OpVariable %_ptr_Uniform__struct_4 Uniform
          %1 = OpFunction %void None %7
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateExplicitLayout, UniformBufferArraySizeCalculationPackBad) {
  // Like previous but, the offset of the second member is too small.

  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %1 "main"
               OpDecorate %_arr_v3uint_uint_2 ArrayStride 16
               OpDecorate %_arr__arr_v3uint_uint_2_uint_2 ArrayStride 32
               OpMemberDecorate %_struct_4 0 Offset 0
               OpMemberDecorate %_struct_4 1 Offset 60
               OpDecorate %_struct_4 Block
               OpDecorate %5 DescriptorSet 0
               OpDecorate %5 Binding 0
       %void = OpTypeVoid
          %7 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
     %uint_2 = OpConstant %uint 2
%_arr_v3uint_uint_2 = OpTypeArray %v3uint %uint_2
%_arr__arr_v3uint_uint_2_uint_2 = OpTypeArray %_arr_v3uint_uint_2 %uint_2
  %_struct_4 = OpTypeStruct %_arr__arr_v3uint_uint_2_uint_2 %uint
%_ptr_Uniform__struct_4 = OpTypePointer Uniform %_struct_4
          %5 = OpVariable %_ptr_Uniform__struct_4 Uniform
          %1 = OpFunction %void None %7
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 60 overlaps previous "
                        "member ending at offset 63"));
}

TEST_F(ValidateExplicitLayout, LayoutNotCheckedWhenSkipBlockLayout) {
  // Checks that block layout is not verified in skipping block layout mode.
  // Even for obviously wrong layout.
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 3 ; wrong alignment
               OpMemberDecorate %S 1 Offset 3 ; same offset as before!
               OpDecorate %S Block
               OpDecorate %B DescriptorSet 0
               OpDecorate %B Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %S = OpTypeStruct %float %v3float
%_ptr_Uniform_S = OpTypePointer Uniform %S
          %B = OpVariable %_ptr_Uniform_S Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  spvValidatorOptionsSetSkipBlockLayout(getValidatorOptions(), true);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(ValidateExplicitLayout, RecurseThroughRuntimeArray) {
  const std::string spirv = R"(
OpCapability Shader
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %outer Block
OpMemberDecorate %inner 0 Offset 0
OpMemberDecorate %inner 1 Offset 1
OpDecorate %runtime ArrayStride 16
OpMemberDecorate %outer 0 Offset 0
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%int = OpTypeInt 32 0
%inner = OpTypeStruct %int %int
%runtime = OpTypeRuntimeArray %inner
%outer = OpTypeStruct %runtime
%outer_ptr = OpTypePointer StorageBuffer %outer
%var = OpVariable %outer_ptr StorageBuffer
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 1 is not aligned to 4"));
}

TEST_F(ValidateExplicitLayout, InvalidStraddle) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %inner_struct 0 Offset 0
OpMemberDecorate %inner_struct 1 Offset 4
OpDecorate %outer_struct Block
OpMemberDecorate %outer_struct 0 Offset 0
OpMemberDecorate %outer_struct 1 Offset 8
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%float = OpTypeFloat 32
%float2 = OpTypeVector %float 2
%inner_struct = OpTypeStruct %float %float2
%outer_struct = OpTypeStruct %float2 %inner_struct
%ptr_ssbo_outer = OpTypePointer StorageBuffer %outer_struct
%var = OpVariable %ptr_ssbo_outer StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_1);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Vector has improper straddle due to offset 12"));
}

TEST_F(ValidateExplicitLayout, DescriptorArray) {
  const std::string spirv = R"(
OpCapability Shader
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
OpMemberDecorate %struct 1 Offset 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%float = OpTypeFloat 32
%int = OpTypeInt 32 0
%int_2 = OpConstant %int 2
%float2 = OpTypeVector %float 2
%struct = OpTypeStruct %float %float2
%struct_array = OpTypeArray %struct %int_2
%ptr_ssbo_array = OpTypePointer StorageBuffer %struct_array
%var = OpVariable %ptr_ssbo_array StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 1 is not aligned to 8"));
}

TEST_F(ValidateExplicitLayout, DescriptorRuntimeArray) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability RuntimeDescriptorArrayEXT
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpExtension "SPV_EXT_descriptor_indexing"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
OpMemberDecorate %struct 1 Offset 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%float = OpTypeFloat 32
%int = OpTypeInt 32 0
%float2 = OpTypeVector %float 2
%struct = OpTypeStruct %float %float2
%struct_array = OpTypeRuntimeArray %struct
%ptr_ssbo_array = OpTypePointer StorageBuffer %struct_array
%var = OpVariable %ptr_ssbo_array StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 1 is not aligned to 8"));
}

TEST_F(ValidateExplicitLayout, MultiDimensionalArray) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
OpDecorate %array_4 ArrayStride 4
OpDecorate %array_3 ArrayStride 48
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%array_4 = OpTypeArray %int %int_4
%array_3 = OpTypeArray %array_4 %int_3
%struct = OpTypeStruct %array_3
%ptr_struct = OpTypePointer Uniform %struct
%var = OpVariable %ptr_struct Uniform
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Array stride 4 must satisfy alignment 16"));
}

TEST_F(ValidateExplicitLayout, ImproperStraddleInArray) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
OpDecorate %array ArrayStride 24
OpMemberDecorate %inner 0 Offset 0
OpMemberDecorate %inner 1 Offset 4
OpMemberDecorate %inner 2 Offset 12
OpMemberDecorate %inner 3 Offset 16
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_2 = OpConstant %int 2
%int2 = OpTypeVector %int 2
%inner = OpTypeStruct %int %int2 %int %int
%array = OpTypeArray %inner %int_2
%struct = OpTypeStruct %array
%ptr_struct = OpTypePointer StorageBuffer %struct
%var = OpVariable %ptr_struct StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_1);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Vector has improper straddle due to offset 28"));
}

TEST_F(ValidateExplicitLayout, LargeArray) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
OpDecorate %array ArrayStride 24
OpMemberDecorate %inner 0 Offset 0
OpMemberDecorate %inner 1 Offset 8
OpMemberDecorate %inner 2 Offset 16
OpMemberDecorate %inner 3 Offset 20
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_2000000 = OpConstant %int 2000000
%int2 = OpTypeVector %int 2
%inner = OpTypeStruct %int %int2 %int %int
%array = OpTypeArray %inner %int_2000000
%struct = OpTypeStruct %array
%ptr_struct = OpTypePointer StorageBuffer %struct
%var = OpVariable %ptr_struct StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_1);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_1));
}

TEST_F(ValidateExplicitLayout, VulkanArrayStrideZero) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
OpDecorate %array ArrayStride 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_4 = OpConstant %int 4
%array = OpTypeArray %int %int_4
%struct = OpTypeStruct %array
%ptr_ssbo_struct = OpTypePointer StorageBuffer %struct
%var = OpVariable %ptr_ssbo_struct StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_1);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Array must not have a stride of 0"));
}

TEST_F(ValidateExplicitLayout, VulkanArrayStrideTooSmall) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
OpDecorate %inner ArrayStride 4
OpDecorate %outer ArrayStride 4
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_4 = OpConstant %int 4
%inner = OpTypeArray %int %int_4
%outer = OpTypeArray %inner %int_4
%struct = OpTypeStruct %outer
%ptr_ssbo_struct = OpTypePointer StorageBuffer %struct
%var = OpVariable %ptr_ssbo_struct StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_1);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Array stride 4 is smaller than element type size 16"));
}

TEST_F(ValidateExplicitLayout, WorkgroupSingleBlockVariable) {
  std::string spirv = R"(
               OpCapability Shader
               OpCapability WorkgroupMemoryExplicitLayoutKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 8 1 1
               OpMemberDecorate %first 0 Offset 0
               OpDecorate %first Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %first = OpTypeStruct %int
%_ptr_Workgroup_first = OpTypePointer Workgroup %first
          %_ = OpVariable %_ptr_Workgroup_first Workgroup
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
%_ptr_Workgroup_int = OpTypePointer Workgroup %int
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpAccessChain %_ptr_Workgroup_int %_ %int_0
               OpStore %13 %int_2
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_UNIVERSAL_1_4));
}

TEST_F(ValidateExplicitLayout, WorkgroupSingleNonBlockVariable) {
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %a
               OpExecutionMode %main LocalSize 8 1 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Workgroup_int = OpTypePointer Workgroup %int
          %a = OpVariable %_ptr_Workgroup_int Workgroup
      %int_2 = OpConstant %int 2
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpStore %a %int_2
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_UNIVERSAL_1_4));
}

TEST_F(ValidateExplicitLayout, WorkgroupMultiBlockVariable) {
  std::string spirv = R"(
               OpCapability Shader
               OpCapability WorkgroupMemoryExplicitLayoutKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_ %__0
               OpExecutionMode %main LocalSize 8 1 1
               OpMemberDecorate %first 0 Offset 0
               OpDecorate %first Block
               OpMemberDecorate %second 0 Offset 0
               OpDecorate %second Block
               OpDecorate %_ Aliased
               OpDecorate %__0 Aliased
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %first = OpTypeStruct %int
%_ptr_Workgroup_first = OpTypePointer Workgroup %first
          %_ = OpVariable %_ptr_Workgroup_first Workgroup
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
%_ptr_Workgroup_int = OpTypePointer Workgroup %int
     %second = OpTypeStruct %int
%_ptr_Workgroup_second = OpTypePointer Workgroup %second
        %__0 = OpVariable %_ptr_Workgroup_second Workgroup
      %int_3 = OpConstant %int 3
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpAccessChain %_ptr_Workgroup_int %_ %int_0
               OpStore %13 %int_2
         %18 = OpAccessChain %_ptr_Workgroup_int %__0 %int_0
               OpStore %18 %int_3
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_UNIVERSAL_1_4));
}

TEST_F(ValidateExplicitLayout, WorkgroupBlockVariableWith8BitType) {
  std::string spirv = R"(
               OpCapability Shader
               OpCapability Int8
               OpCapability WorkgroupMemoryExplicitLayout8BitAccessKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 2 1 1
               OpMemberDecorate %first 0 Offset 0
               OpDecorate %first Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %char = OpTypeInt 8 1
      %first = OpTypeStruct %char
%_ptr_Workgroup_first = OpTypePointer Workgroup %first
          %_ = OpVariable %_ptr_Workgroup_first Workgroup
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %char_2 = OpConstant %char 2
%_ptr_Workgroup_char = OpTypePointer Workgroup %char
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpAccessChain %_ptr_Workgroup_char %_ %int_0
               OpStore %14 %char_2
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_UNIVERSAL_1_4));
}

TEST_F(ValidateExplicitLayout, WorkgroupMultiNonBlockVariable) {
  std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %a %b
               OpExecutionMode %main LocalSize 8 1 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Workgroup_int = OpTypePointer Workgroup %int
          %a = OpVariable %_ptr_Workgroup_int Workgroup
      %int_2 = OpConstant %int 2
          %b = OpVariable %_ptr_Workgroup_int Workgroup
      %int_3 = OpConstant %int 3
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpStore %a %int_2
               OpStore %b %int_3
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_UNIVERSAL_1_4));
}

TEST_F(ValidateExplicitLayout, WorkgroupBlockVariableWith16BitType) {
  std::string spirv = R"(
               OpCapability Shader
               OpCapability Float16
               OpCapability Int16
               OpCapability WorkgroupMemoryExplicitLayoutKHR
               OpCapability WorkgroupMemoryExplicitLayout16BitAccessKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 2 1 1
               OpMemberDecorate %first 0 Offset 0
               OpMemberDecorate %first 1 Offset 2
               OpDecorate %first Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %short = OpTypeInt 16 1
       %half = OpTypeFloat 16
      %first = OpTypeStruct %short %half
%_ptr_Workgroup_first = OpTypePointer Workgroup %first
          %_ = OpVariable %_ptr_Workgroup_first Workgroup
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %short_3 = OpConstant %short 3
%_ptr_Workgroup_short = OpTypePointer Workgroup %short
      %int_1 = OpConstant %int 1
%half_0x1_898p_3 = OpConstant %half 0x1.898p+3
%_ptr_Workgroup_half = OpTypePointer Workgroup %half
       %main = OpFunction %void None %3
          %5 = OpLabel
         %15 = OpAccessChain %_ptr_Workgroup_short %_ %int_0
               OpStore %15 %short_3
         %19 = OpAccessChain %_ptr_Workgroup_half %_ %int_1
               OpStore %19 %half_0x1_898p_3
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_UNIVERSAL_1_4));
}

TEST_F(ValidateExplicitLayout, WorkgroupBlockVariableScalarLayout) {
  std::string spirv = R"(
               OpCapability Shader
               OpCapability WorkgroupMemoryExplicitLayoutKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %B
               OpSource GLSL 450
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpMemberDecorate %S 2 Offset 16
               OpMemberDecorate %S 3 Offset 28
               OpDecorate %S Block
               OpDecorate %B Aliased
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %S = OpTypeStruct %float %v3float %v3float %v3float
%_ptr_Workgroup_S = OpTypePointer Workgroup %S
          %B = OpVariable %_ptr_Workgroup_S Workgroup
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_4);
  spvValidatorOptionsSetWorkgroupScalarBlockLayout(getValidatorOptions(), true);
  EXPECT_EQ(SPV_SUCCESS,
            ValidateAndRetrieveValidationState(SPV_ENV_UNIVERSAL_1_4))
      << getDiagnosticString();
}

TEST_F(ValidateExplicitLayout, WorkgroupSingleBlockVariableMissingLayout) {
  std::string spirv = R"(
               OpCapability Shader
               OpCapability WorkgroupMemoryExplicitLayoutKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 8 1 1
               OpDecorate %first Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %first = OpTypeStruct %int
%_ptr_Workgroup_first = OpTypePointer Workgroup %first
          %_ = OpVariable %_ptr_Workgroup_first Workgroup
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
%_ptr_Workgroup_int = OpTypePointer Workgroup %int
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpAccessChain %_ptr_Workgroup_int %_ %int_0
               OpStore %13 %int_2
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_1_SPIRV_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 0 must be explicitly laid out with "
                        "Offset or OffsetIdEXT decorations"));
}

TEST_F(ValidateExplicitLayout, WorkgroupSingleBlockVariableBadLayout) {
  std::string spirv = R"(
               OpCapability Shader
               OpCapability WorkgroupMemoryExplicitLayoutKHR
               OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 8 1 1
               OpMemberDecorate %first 0 Offset 1
               OpDecorate %first Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %first = OpTypeStruct %int
%_ptr_Workgroup_first = OpTypePointer Workgroup %first
          %_ = OpVariable %_ptr_Workgroup_first Workgroup
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
%_ptr_Workgroup_int = OpTypePointer Workgroup %int
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpAccessChain %_ptr_Workgroup_int %_ %int_0
               OpStore %13 %int_2
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            ValidateAndRetrieveValidationState(SPV_ENV_VULKAN_1_1_SPIRV_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 0 at offset 1 is not aligned to 4"));
}

TEST_F(ValidateExplicitLayout, BadMatrixStrideUniform) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 MatrixStride 3
OpMemberDecorate %block 0 ColMajor
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%float = OpTypeFloat 32
%float4 = OpTypeVector %float 4
%matrix4x4 = OpTypeMatrix %float4 4
%block = OpTypeStruct %matrix4x4
%block_ptr = OpTypePointer Uniform %block
%var = OpVariable %block_ptr Uniform
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Matrix with a stride 3 not satisfying alignment to 16"));
}

TEST_F(ValidateExplicitLayout, BadMatrixStrideStorageBuffer) {
  const std::string spirv = R"(
OpCapability Shader
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 MatrixStride 3
OpMemberDecorate %block 0 ColMajor
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%float = OpTypeFloat 32
%float4 = OpTypeVector %float 4
%matrix4x4 = OpTypeMatrix %float4 4
%block = OpTypeStruct %matrix4x4
%block_ptr = OpTypePointer StorageBuffer %block
%var = OpVariable %block_ptr StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Matrix with a stride 3 not satisfying alignment to 16"));
}

TEST_F(ValidateExplicitLayout, BadMatrixStridePushConstant) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 MatrixStride 3
OpMemberDecorate %block 0 ColMajor
%void = OpTypeVoid
%float = OpTypeFloat 32
%float4 = OpTypeVector %float 4
%matrix4x4 = OpTypeMatrix %float4 4
%block = OpTypeStruct %matrix4x4
%block_ptr = OpTypePointer PushConstant %block
%var = OpVariable %block_ptr PushConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Matrix with a stride 3 not satisfying alignment to 16"));
}

TEST_F(ValidateExplicitLayout, BadMatrixStrideStorageBufferScalarLayout) {
  const std::string spirv = R"(
OpCapability Shader
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 MatrixStride 3
OpMemberDecorate %block 0 RowMajor
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%float = OpTypeFloat 32
%float4 = OpTypeVector %float 4
%matrix4x4 = OpTypeMatrix %float4 4
%block = OpTypeStruct %matrix4x4
%block_ptr = OpTypePointer StorageBuffer %block
%var = OpVariable %block_ptr StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  options_->scalar_block_layout = true;
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Matrix with a stride 3 not satisfying alignment to 4"));
}

TEST_F(ValidateExplicitLayout, MissingOffsetStructNestedInArray) {
  const std::string spirv = R"(
OpCapability Shader
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %array ArrayStride 4
OpDecorate %outer Block
OpMemberDecorate %outer 0 Offset 0
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_4 = OpConstant %int 4
%inner = OpTypeStruct %int
%array = OpTypeArray %inner %int_4
%outer = OpTypeStruct %array
%ptr_ssbo_outer = OpTypePointer StorageBuffer %outer
%var = OpVariable %ptr_ssbo_outer StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 0 must be explicitly laid out with "
                        "Offset or OffsetIdEXT decorations"));
}

TEST_F(ValidateExplicitLayout, Std140ColMajorMat2x2) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 ColMajor
OpMemberDecorate %block 0 MatrixStride 8
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%float = OpTypeFloat 32
%float2 = OpTypeVector %float 2
%matrix = OpTypeMatrix %float2 2
%block = OpTypeStruct %matrix
%ptr_block = OpTypePointer Uniform %block
%var = OpVariable %ptr_block Uniform
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Matrix with a stride 8 not satisfying alignment to 16"));
}

TEST_F(ValidateExplicitLayout, Std140RowMajorMat2x2) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 RowMajor
OpMemberDecorate %block 0 MatrixStride 8
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%float = OpTypeFloat 32
%float2 = OpTypeVector %float 2
%matrix = OpTypeMatrix %float2 2
%block = OpTypeStruct %matrix
%ptr_block = OpTypePointer Uniform %block
%var = OpVariable %ptr_block Uniform
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Matrix with a stride 8 not satisfying alignment to 16"));
}

TEST_F(ValidateExplicitLayout, Std140ColMajorMat4x2) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 ColMajor
OpMemberDecorate %block 0 MatrixStride 8
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%float = OpTypeFloat 32
%float2 = OpTypeVector %float 2
%matrix = OpTypeMatrix %float2 4
%block = OpTypeStruct %matrix
%ptr_block = OpTypePointer Uniform %block
%var = OpVariable %ptr_block Uniform
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Matrix with a stride 8 not satisfying alignment to 16"));
}

TEST_F(ValidateExplicitLayout, Std140ColMajorMat2x3) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 ColMajor
OpMemberDecorate %block 0 MatrixStride 12
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%float = OpTypeFloat 32
%float3 = OpTypeVector %float 3
%matrix = OpTypeMatrix %float3 2
%block = OpTypeStruct %matrix
%ptr_block = OpTypePointer Uniform %block
%var = OpVariable %ptr_block Uniform
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Matrix with a stride 12 not satisfying "
                        "alignment to 16"));
}

TEST_F(ValidateExplicitLayout, MatrixMissingMajornessUniform) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 MatrixStride 16
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%float = OpTypeFloat 32
%float2 = OpTypeVector %float 2
%matrix = OpTypeMatrix %float2 2
%block = OpTypeStruct %matrix
%ptr_block = OpTypePointer Uniform %block
%var = OpVariable %ptr_block Uniform
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "must be explicitly laid out with RowMajor or ColMajor decorations"));
}

TEST_F(ValidateExplicitLayout, MatrixMissingMajornessStorageBuffer) {
  const std::string spirv = R"(
OpCapability Shader
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 MatrixStride 16
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%float = OpTypeFloat 32
%float2 = OpTypeVector %float 2
%matrix = OpTypeMatrix %float2 2
%block = OpTypeStruct %matrix
%ptr_block = OpTypePointer StorageBuffer %block
%var = OpVariable %ptr_block StorageBuffer
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "must be explicitly laid out with RowMajor or ColMajor decorations"));
}

TEST_F(ValidateExplicitLayout, MatrixMissingMajornessPushConstant) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 MatrixStride 16
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%float = OpTypeFloat 32
%float2 = OpTypeVector %float 2
%matrix = OpTypeMatrix %float2 2
%block = OpTypeStruct %matrix
%ptr_block = OpTypePointer PushConstant %block
%var = OpVariable %ptr_block PushConstant
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "must be explicitly laid out with RowMajor or ColMajor decorations"));
}

TEST_F(ValidateExplicitLayout, StructWithRowAndColMajor) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 MatrixStride 16
OpMemberDecorate %block 0 ColMajor
OpMemberDecorate %block 1 Offset 32
OpMemberDecorate %block 1 MatrixStride 16
OpMemberDecorate %block 1 RowMajor
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%float = OpTypeFloat 32
%float2 = OpTypeVector %float 2
%matrix = OpTypeMatrix %float2 2
%block = OpTypeStruct %matrix %matrix
%ptr_block = OpTypePointer PushConstant %block
%var = OpVariable %ptr_block PushConstant
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateExplicitLayout, PhysicalStorageBufferWithOffset) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Int64
OpCapability PhysicalStorageBufferAddresses
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint GLCompute %main "main" %pc
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %pc_block Block
OpMemberDecorate %pc_block 0 Offset 0
OpMemberDecorate %pssbo_struct 0 Offset 0
%void = OpTypeVoid
%long = OpTypeInt 64 0
%float = OpTypeFloat 32
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%pc_block = OpTypeStruct %long
%pc_block_ptr = OpTypePointer PushConstant %pc_block
%pc_long_ptr = OpTypePointer PushConstant %long
%pc = OpVariable %pc_block_ptr PushConstant
%pssbo_struct = OpTypeStruct %float
%pssbo_ptr = OpTypePointer PhysicalStorageBuffer %pssbo_struct
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%pc_gep = OpAccessChain %pc_long_ptr %pc %int_0
%addr = OpLoad %long %pc_gep
%ptr = OpConvertUToPtr %pssbo_ptr %addr
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
}

TEST_F(ValidateExplicitLayout, PhysicalStorageBufferMissingOffset) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Int64
OpCapability PhysicalStorageBufferAddresses
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint GLCompute %main "main" %pc
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %pc_block Block
OpMemberDecorate %pc_block 0 Offset 0
%void = OpTypeVoid
%long = OpTypeInt 64 0
%float = OpTypeFloat 32
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%pc_block = OpTypeStruct %long
%pc_block_ptr = OpTypePointer PushConstant %pc_block
%pc_long_ptr = OpTypePointer PushConstant %long
%pc = OpVariable %pc_block_ptr PushConstant
%pssbo_struct = OpTypeStruct %float
%pssbo_ptr = OpTypePointer PhysicalStorageBuffer %pssbo_struct
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%pc_gep = OpAccessChain %pc_long_ptr %pc %int_0
%addr = OpLoad %long %pc_gep
%ptr = OpConvertUToPtr %pssbo_ptr %addr
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 0 must be explicitly laid out with "
                        "Offset or OffsetIdEXT decorations"));
}

TEST_F(ValidateExplicitLayout, PhysicalStorageBufferMissingArrayStride) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Int64
OpCapability PhysicalStorageBufferAddresses
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint GLCompute %main "main" %pc
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %pc_block Block
OpMemberDecorate %pc_block 0 Offset 0
%void = OpTypeVoid
%long = OpTypeInt 64 0
%float = OpTypeFloat 32
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%pc_block = OpTypeStruct %long
%pc_block_ptr = OpTypePointer PushConstant %pc_block
%pc_long_ptr = OpTypePointer PushConstant %long
%pc = OpVariable %pc_block_ptr PushConstant
%pssbo_array = OpTypeArray %float %int_4
%pssbo_ptr = OpTypePointer PhysicalStorageBuffer %pssbo_array
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%pc_gep = OpAccessChain %pc_long_ptr %pc %int_0
%addr = OpLoad %long %pc_gep
%ptr = OpConvertUToPtr %pssbo_ptr %addr
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Array must be explicitly laid out with ArrayStride or "
                        "ArrayStrideIdEXT decorations"));
}

TEST_F(ValidateExplicitLayout, MatrixArrayMissingMajorness) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 MatrixStride 16
OpDecorate %array ArrayStride 32
%void = OpTypeVoid
%float = OpTypeFloat 32
%int = OpTypeInt 32 0
%int_2 = OpConstant %int 2
%vec = OpTypeVector %float 2
%mat = OpTypeMatrix %vec 2
%array = OpTypeArray %mat %int_2
%block = OpTypeStruct %array
%ptr = OpTypePointer Uniform %block
%var = OpVariable %ptr Uniform
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_1);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "must be explicitly laid out with RowMajor or ColMajor decorations"));
}

TEST_F(ValidateExplicitLayout, MatrixArrayMissingStride) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 ColMajor
OpDecorate %array ArrayStride 32
%void = OpTypeVoid
%float = OpTypeFloat 32
%int = OpTypeInt 32 0
%int_2 = OpConstant %int 2
%vec = OpTypeVector %float 2
%mat = OpTypeMatrix %vec 2
%array = OpTypeArray %mat %int_2
%block = OpTypeStruct %array
%ptr = OpTypePointer Uniform %block
%var = OpVariable %ptr Uniform
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_1);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with MatrixStride decorations"));
}

TEST_F(ValidateExplicitLayout, MatrixArrayBadStride) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 ColMajor
OpMemberDecorate %block 0 MatrixStride 8
OpDecorate %array ArrayStride 32
%void = OpTypeVoid
%float = OpTypeFloat 32
%int = OpTypeInt 32 0
%int_2 = OpConstant %int 2
%vec = OpTypeVector %float 2
%mat = OpTypeMatrix %vec 2
%array = OpTypeArray %mat %int_2
%block = OpTypeStruct %array
%ptr = OpTypePointer Uniform %block
%var = OpVariable %ptr Uniform
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_1);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Matrix with a stride 8 not satisfying alignment to 16"));
}

TEST_F(ValidateExplicitLayout, MatrixArrayArrayMissingMajorness) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 MatrixStride 16
OpDecorate %array ArrayStride 32
OpDecorate %rta ArrayStride 64
%void = OpTypeVoid
%float = OpTypeFloat 32
%int = OpTypeInt 32 0
%int_2 = OpConstant %int 2
%vec = OpTypeVector %float 2
%mat = OpTypeMatrix %vec 2
%array = OpTypeArray %mat %int_2
%rta = OpTypeRuntimeArray %array
%block = OpTypeStruct %rta
%ptr = OpTypePointer StorageBuffer %block
%var = OpVariable %ptr StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_1);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "must be explicitly laid out with RowMajor or ColMajor decorations"));
}

TEST_F(ValidateExplicitLayout, MatrixArrayArrayMissingStride) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 ColMajor
OpDecorate %array ArrayStride 32
OpDecorate %rta ArrayStride 64
%void = OpTypeVoid
%float = OpTypeFloat 32
%int = OpTypeInt 32 0
%int_2 = OpConstant %int 2
%vec = OpTypeVector %float 2
%mat = OpTypeMatrix %vec 2
%array = OpTypeArray %mat %int_2
%rta = OpTypeRuntimeArray %array
%block = OpTypeStruct %rta
%ptr = OpTypePointer StorageBuffer %block
%var = OpVariable %ptr StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_1);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with MatrixStride decorations"));
}

TEST_F(ValidateExplicitLayout, MatrixArrayArrayBadStride) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 0 ColMajor
OpMemberDecorate %block 0 MatrixStride 8
OpDecorate %array ArrayStride 32
OpDecorate %a ArrayStride 64
%void = OpTypeVoid
%float = OpTypeFloat 32
%int = OpTypeInt 32 0
%int_2 = OpConstant %int 2
%vec = OpTypeVector %float 2
%mat = OpTypeMatrix %vec 2
%array = OpTypeArray %mat %int_2
%a = OpTypeArray %array %int_2
%block = OpTypeStruct %a
%ptr = OpTypePointer Uniform %block
%var = OpVariable %ptr Uniform
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_1);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Matrix with a stride 8 not satisfying alignment to 16"));
}

TEST_F(ValidateExplicitLayout, UntypedVariableWorkgroupRequiresStruct) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability WorkgroupMemoryExplicitLayoutKHR
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var
%void = OpTypeVoid
%int = OpTypeInt 32 0
%ptr = OpTypeUntypedPointerKHR Workgroup
%var = OpUntypedVariableKHR %ptr Workgroup %int
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Untyped workgroup variables in shaders must be block "
                        "decorated structs"));
}

TEST_F(ValidateExplicitLayout, UntypedVariableWorkgroupRequiresBlockStruct) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability WorkgroupMemoryExplicitLayoutKHR
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var
%void = OpTypeVoid
%int = OpTypeInt 32 0
%struct = OpTypeStruct %int
%ptr = OpTypeUntypedPointerKHR Workgroup
%var = OpUntypedVariableKHR %ptr Workgroup %struct
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Untyped workgroup variables in shaders must be block "
                        "decorated"));
}

TEST_F(ValidateExplicitLayout, UntypedArrayLengthMissingOffset) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %struct Block
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpDecorate %array ArrayStride 4
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%array = OpTypeRuntimeArray %int
%struct = OpTypeStruct %array
%block = OpTypeStruct %array
%ptr = OpTypeUntypedPointerKHR StorageBuffer
%var = OpUntypedVariableKHR %ptr StorageBuffer %block
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%len = OpUntypedArrayLengthKHR %int %struct %var 0
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 0 must be explicitly laid out with "
                        "Offset or OffsetIdEXT decorations"));
}

TEST_F(ValidateExplicitLayout, BlockArrayWithoutStride) {
  const std::string spirv = R"(
OpCapability Shader
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%int = OpTypeInt 32 0
%int_4 = OpConstant %int 4
%struct = OpTypeStruct %int
%array = OpTypeArray %struct %int_4
%ptr = OpTypePointer StorageBuffer %array
%var = OpVariable %ptr StorageBuffer
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateExplicitLayout, BlockArrayWithoutStrideUntypedAccessChain) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%int = OpTypeInt 32 0
%int_4 = OpConstant %int 4
%struct = OpTypeStruct %int
%array = OpTypeArray %struct %int_4
%void = OpTypeVoid
%ptr = OpTypeUntypedPointerKHR StorageBuffer
%var = OpUntypedVariableKHR %ptr StorageBuffer %array
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpUntypedAccessChainKHR %ptr %array %var
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateExplicitLayout, InvalidLayoutBlockFunctionPre1p4) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
%void = OpTypeVoid
%int = OpTypeInt 32 0
%block = OpTypeStruct %int
%ptr_function_block = OpTypePointer Function %block
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%var = OpVariable %ptr_function_block Function
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateExplicitLayout, InvalidLayoutBlockFunctionPost1p4) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
%void = OpTypeVoid
%int = OpTypeInt 32 0
%block = OpTypeStruct %int
%ptr_function_block = OpTypePointer Function %block
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%var = OpVariable %ptr_function_block Function
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-None-10684"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid explicit layout decorations on type"));
}

TEST_F(ValidateExplicitLayout, InvalidLayoutOffsetPrivatePre1p4) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %block 0 Offset 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%block = OpTypeStruct %int
%ptr_private_block = OpTypePointer Private %block
%void_fn = OpTypeFunction %void
%var = OpVariable %ptr_private_block Private
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateExplicitLayout, InvalidLayoutOffsetPrivatePost1p4) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %block 0 Offset 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%block = OpTypeStruct %int
%ptr_private_block = OpTypePointer Private %block
%void_fn = OpTypeFunction %void
%var = OpVariable %ptr_private_block Private
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-None-10684"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid explicit layout decorations on type"));
}

TEST_F(ValidateExplicitLayout,
       InvalidLayoutArrayStrideWorkgroupExplicitLayout_MissingBlock) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability WorkgroupMemoryExplicitLayoutKHR
OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %array ArrayStride 4
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_4 = OpConstant %int 4
%array = OpTypeArray %int %int_4
%ptr_wg_block = OpTypePointer Workgroup %array
%void_fn = OpTypeFunction %void
%var = OpVariable %ptr_wg_block Workgroup
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid explicit layout decorations on type"));
}

TEST_F(ValidateExplicitLayout,
       InvalidLayoutArrayStrideWorkgroupExplicitLayout) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability WorkgroupMemoryExplicitLayoutKHR
OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %array ArrayStride 4
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_4 = OpConstant %int 4
%array = OpTypeArray %int %int_4
%block = OpTypeStruct %array
%ptr_wg_block = OpTypePointer Workgroup %block
%void_fn = OpTypeFunction %void
%var = OpVariable %ptr_wg_block Workgroup
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
}

TEST_F(ValidateExplicitLayout, InvalidLayoutArrayStrideWorkgroup) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %array ArrayStride 4
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_4 = OpConstant %int 4
%array = OpTypeArray %int %int_4
%ptr_wg_block = OpTypePointer Workgroup %array
%void_fn = OpTypeFunction %void
%var = OpVariable %ptr_wg_block Workgroup
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-None-10684"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid explicit layout decorations on type"));
}

TEST_F(ValidateExplicitLayout, InvalidLayoutArrayStrideUniformConstant) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %array ArrayStride 4
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_4 = OpConstant %int 4
%sampler = OpTypeSampler
%array = OpTypeArray %sampler %int_4
%ptr_uc_block = OpTypePointer UniformConstant %array
%void_fn = OpTypeFunction %void
%var = OpVariable %ptr_uc_block UniformConstant
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-None-10684"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid explicit layout decorations on type"));
}

TEST_F(ValidateExplicitLayout, InvalidLayoutMatrixStrideFunctionPost1p4) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %block 0 MatrixStride 16
%void = OpTypeVoid
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%mat4x4 = OpTypeMatrix %v4float 4
%block = OpTypeStruct %mat4x4
%ptr_function_block = OpTypePointer Function %block
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%var = OpVariable %ptr_function_block Function
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-None-10684"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid explicit layout decorations on type"));
}

TEST_F(ValidateExplicitLayout, InvalidLayoutNestedMatrixStrideFunctionPost1p4) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorate %block 0 MatrixStride 16
%void = OpTypeVoid
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%mat4x4 = OpTypeMatrix %v4float 4
%block = OpTypeStruct %mat4x4
%block2 = OpTypeStruct %block
%int = OpTypeInt 32 0
%int_2 = OpConstant %int 2
%array = OpTypeArray %block2 %int_2
%ptr_function_array = OpTypePointer Function %array
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%var = OpVariable %ptr_function_array Function
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-None-10684"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid explicit layout decorations on type"));
}

TEST_F(ValidateExplicitLayout, InvalidLayoutBufferBlockWorkgroup) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block BufferBlock
%void = OpTypeVoid
%int = OpTypeInt 32 0
%block = OpTypeStruct %int
%ptr_wg_block = OpTypePointer Workgroup %block
%void_fn = OpTypeFunction %void
%var = OpVariable %ptr_wg_block Workgroup
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-None-10684"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid explicit layout decorations on type"));
}

TEST_F(ValidateExplicitLayout, InvalidLayoutUntypedStore) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%block = OpTypeStruct %int
%block_null = OpConstantNull %block
%ptr = OpTypeUntypedPointerKHR StorageBuffer
%var = OpUntypedVariableKHR %ptr StorageBuffer %block
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpStore %var %block_null
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateExplicitLayout, ExplicitLayoutOnPtrPhysicalStorageBuffer) {
  const std::string spirv = R"(
OpCapability PhysicalStorageBufferAddresses
OpCapability Int64
OpCapability Shader
OpExtension "SPV_KHR_physical_storage_buffer"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %_ptr_PhysicalStorageBuffer_int ArrayStride 4
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int  ; ArrayStride 4
%Foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_int
%_ptr_Function_Foo = OpTypePointer Function %Foo
%int_0 = OpConstant %int 0
%_ptr_Function__ptr_PhysicalStorageBuffer_int = OpTypePointer Function %_ptr_PhysicalStorageBuffer_int
%ulong = OpTypeInt 64 0
%ulong_0 = OpConstant %ulong 0
%main = OpFunction %void None %void_fn
%entry = OpLabel
%obj = OpVariable %_ptr_Function_Foo Function
%obj_member = OpAccessChain %_ptr_Function__ptr_PhysicalStorageBuffer_int %obj %int_0
%nullptr = OpConvertUToPtr %_ptr_PhysicalStorageBuffer_int %ulong_0
OpStore %obj_member %nullptr
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateExplicitLayout, RuntimeArrayNotLargestOffsetInBlock) {
  const std::string spirv = R"(
OpCapability Shader
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 16
OpMemberDecorate %block 1 Offset 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%int = OpTypeInt 32 0
%array = OpTypeRuntimeArray %int
%block = OpTypeStruct %int %array
%ptr = OpTypePointer StorageBuffer %block
%var = OpVariable %ptr StorageBuffer
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("has a runtime array at offset 0, but other members at "
                        "larger offsets"));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-OpTypeRuntimeArray-04680"));
}

TEST_F(ValidateExplicitLayout, RuntimeArrayNotLargestOffsetInBufferBlock) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %block BufferBlock
OpMemberDecorate %block 0 Offset 16
OpMemberDecorate %block 1 Offset 0
OpDecorate %array ArrayStride 4
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%int = OpTypeInt 32 0
%array = OpTypeRuntimeArray %int
%block = OpTypeStruct %int %array
%ptr = OpTypePointer Uniform %block
%var = OpVariable %ptr Uniform
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("has a runtime array at offset 0, but other members at "
                        "larger offsets"));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-OpTypeRuntimeArray-04680"));
}

TEST_F(ValidateExplicitLayout, LongVectorUniformPass_ImproperStraddle) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Int8
OpCapability UniformAndStorageBuffer8BitAccess
OpCapability LongVectorEXT

OpExtension "SPV_KHR_8bit_storage"
OpExtension "SPV_EXT_long_vector"

OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %BP_main "main"

OpDecorate %input0 DescriptorSet 0
OpDecorate %input0 Binding 0
OpDecorate %a10testtype ArrayStride 16
OpDecorate %buf BufferBlock
OpMemberDecorate %buf 0 Offset 0

%void = OpTypeVoid
%bool = OpTypeBool
%u32 = OpTypeInt 32 0
%voidf = OpTypeFunction %void
%c_u32_10 = OpConstant %u32 10
%vectorSizeConst = OpConstant %u32 12

%scalartype = OpTypeInt 8 1
%testtype = OpTypeVectorIdEXT %scalartype %vectorSizeConst

%a10testtype = OpTypeArray %testtype %c_u32_10
%buf = OpTypeStruct %a10testtype
%bufptr = OpTypePointer Uniform %buf

%input0 = OpVariable %bufptr Uniform


%BP_main = OpFunction %void None %voidf
%BP_label = OpLabel
OpReturn
OpFunctionEnd

)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateExplicitLayout, LongVectorUniformPass) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Int8
OpCapability UniformAndStorageBuffer8BitAccess
OpCapability LongVectorEXT

OpExtension "SPV_KHR_8bit_storage"
OpExtension "SPV_EXT_long_vector"

OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %BP_main "main"

OpDecorate %input0 DescriptorSet 0
OpDecorate %input0 Binding 0
OpDecorate %a10testtype ArrayStride 12
OpDecorate %buf BufferBlock
OpMemberDecorate %buf 0 Offset 0

%void = OpTypeVoid
%bool = OpTypeBool
%u32 = OpTypeInt 32 0
%voidf = OpTypeFunction %void
%c_u32_10 = OpConstant %u32 10
%vectorSizeConst = OpConstant %u32 12

%scalartype = OpTypeInt 8 1
%testtype = OpTypeVectorIdEXT %scalartype %vectorSizeConst

%a10testtype = OpTypeArray %testtype %c_u32_10
%buf = OpTypeStruct %a10testtype
%bufptr = OpTypePointer Uniform %buf

%input0 = OpVariable %bufptr Uniform


%BP_main = OpFunction %void None %voidf
%BP_label = OpLabel
OpReturn
OpFunctionEnd

)";

  options_->scalar_block_layout = true;
  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_F(ValidateExplicitLayout, BufferPointerEXTMissingOffsetBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn ResourceHeapEXT
OpDecorate %Struct Block
OpDecorate %storage_buffer_array_type ArrayStride 16
%void = OpTypeVoid
%int = OpTypeInt 32 0
%float = OpTypeFloat 32
%int_0 = OpConstant %int 0
%storage_buffer_type = OpTypeBufferEXT StorageBuffer
%storage_buffer_array_type = OpTypeRuntimeArray %storage_buffer_type
%Struct = OpTypeStruct %float
%_ptr_StorageBuffer_Struct = OpTypePointer StorageBuffer %Struct
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%ptr_storagebuffer = OpTypeUntypedPointerKHR StorageBuffer
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%buffer_ptr = OpUntypedAccessChainKHR %ptr_uniformconstant %storage_buffer_array_type %resource_heap %int_0
%buffer_data_ptr = OpBufferPointerEXT %_ptr_StorageBuffer_Struct %buffer_ptr
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 0 must be explicitly laid out with "
                        "Offset or OffsetIdEXT decorations"));
}

TEST_F(ValidateExplicitLayout, BufferPointerEXTUnalignedOffsetBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn ResourceHeapEXT
OpDecorate %Struct Block
OpMemberDecorate %Struct 0 Offset 2
OpDecorate %storage_buffer_array_type ArrayStride 16
%void = OpTypeVoid
%int = OpTypeInt 32 0
%float = OpTypeFloat 32
%int_0 = OpConstant %int 0
%storage_buffer_type = OpTypeBufferEXT StorageBuffer
%storage_buffer_array_type = OpTypeRuntimeArray %storage_buffer_type
%Struct = OpTypeStruct %float
%_ptr_StorageBuffer_Struct = OpTypePointer StorageBuffer %Struct
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%ptr_storagebuffer = OpTypeUntypedPointerKHR StorageBuffer
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%buffer_ptr = OpUntypedAccessChainKHR %ptr_uniformconstant %storage_buffer_array_type %resource_heap %int_0
%buffer_data_ptr = OpBufferPointerEXT %_ptr_StorageBuffer_Struct %buffer_ptr
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 0 at offset 2 is not aligned to 4"));
}

TEST_F(ValidateExplicitLayout, BufferPointerEXTMissingArrayStrideBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn ResourceHeapEXT
OpDecorate %Struct Block
OpMemberDecorate %Struct 0 Offset 0
OpDecorate %storage_buffer_array_type ArrayStride 16
%void = OpTypeVoid
%int = OpTypeInt 32 0
%float = OpTypeFloat 32
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%array = OpTypeArray %float %int_4
%storage_buffer_type = OpTypeBufferEXT StorageBuffer
%storage_buffer_array_type = OpTypeRuntimeArray %storage_buffer_type
%Struct = OpTypeStruct %array
%_ptr_StorageBuffer_Struct = OpTypePointer StorageBuffer %Struct
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%ptr_storagebuffer = OpTypeUntypedPointerKHR StorageBuffer
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%buffer_ptr = OpUntypedAccessChainKHR %ptr_uniformconstant %storage_buffer_array_type %resource_heap %int_0
%buffer_data_ptr = OpBufferPointerEXT %_ptr_StorageBuffer_Struct %buffer_ptr
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Array must be explicitly laid out with ArrayStride or "
                        "ArrayStrideIdEXT decorations"));
}

TEST_F(ValidateExplicitLayout, BufferPointerEXTMissingMatrixStrideBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn ResourceHeapEXT
OpDecorate %Struct Block
OpMemberDecorate %Struct 0 Offset 0
OpDecorate %storage_buffer_array_type ArrayStride 16
%void = OpTypeVoid
%int = OpTypeInt 32 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%int_0 = OpConstant %int 0
%mat = OpTypeMatrix %v4float 4
%storage_buffer_type = OpTypeBufferEXT StorageBuffer
%storage_buffer_array_type = OpTypeRuntimeArray %storage_buffer_type
%Struct = OpTypeStruct %mat
%_ptr_Uniform_Struct = OpTypePointer Uniform %Struct
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%ptr_uniform = OpTypeUntypedPointerKHR Uniform
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%buffer_ptr = OpUntypedAccessChainKHR %ptr_uniformconstant %storage_buffer_array_type %resource_heap %int_0
%buffer_data_ptr = OpBufferPointerEXT %_ptr_Uniform_Struct %buffer_ptr
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 0 containing a matrix must be "
                        "explicitly laid out with RowMajor or ColMajor "
                        "decorations"));
}

TEST_F(ValidateExplicitLayout, BufferPointerEXTStorageBufferScalarGood) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn ResourceHeapEXT
OpDecorate %Struct Block
OpMemberDecorate %Struct 0 Offset 0
OpMemberDecorate %Struct 1 Offset 12
OpDecorate %storage_buffer_array_type ArrayStride 16
%void = OpTypeVoid
%int = OpTypeInt 32 0
%float = OpTypeFloat 32
%v3float = OpTypeVector %float 3
%int_0 = OpConstant %int 0
%storage_buffer_type = OpTypeBufferEXT StorageBuffer
%storage_buffer_array_type = OpTypeRuntimeArray %storage_buffer_type
%Struct = OpTypeStruct %v3float %float
%_ptr_StorageBuffer_Struct = OpTypePointer StorageBuffer %Struct
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%ptr_storagebuffer = OpTypeUntypedPointerKHR StorageBuffer
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%buffer_ptr = OpUntypedAccessChainKHR %ptr_uniformconstant %storage_buffer_array_type %resource_heap %int_0
%buffer_data_ptr = OpBufferPointerEXT %_ptr_StorageBuffer_Struct %buffer_ptr
OpReturn
OpFunctionEnd
)";

  options_->scalar_block_layout = true;
  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
}

TEST_F(ValidateExplicitLayout, BufferPointerEXTUniformExtendedAlignmentBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn ResourceHeapEXT
OpDecorate %Struct Block
OpMemberDecorate %Struct 0 Offset 0
OpMemberDecorate %Struct 1 Offset 4
OpDecorate %storage_buffer_array_type ArrayStride 16
OpDecorate %array ArrayStride 16
%void = OpTypeVoid
%int = OpTypeInt 32 0
%float = OpTypeFloat 32
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%array = OpTypeArray %float %int_4
%storage_buffer_type = OpTypeBufferEXT StorageBuffer
%storage_buffer_array_type = OpTypeRuntimeArray %storage_buffer_type
%Struct = OpTypeStruct %float %array
%_ptr_Uniform_Struct = OpTypePointer Uniform %Struct
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%buffer_ptr = OpUntypedAccessChainKHR %ptr_uniformconstant %storage_buffer_array_type %resource_heap %int_0
%buffer_data_ptr = OpBufferPointerEXT %_ptr_Uniform_Struct %buffer_ptr
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 4 is not aligned to 16"));
}

TEST_F(ValidateExplicitLayout, BufferPointerEXTArrayResultBadStrideBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn ResourceHeapEXT
OpDecorate %storage_buffer_array_type ArrayStride 16
OpDecorate %array ArrayStride 2
%void = OpTypeVoid
%int = OpTypeInt 32 0
%float = OpTypeFloat 32
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%array = OpTypeArray %float %int_4
%storage_buffer_type = OpTypeBufferEXT StorageBuffer
%storage_buffer_array_type = OpTypeRuntimeArray %storage_buffer_type
%_ptr_StorageBuffer_array = OpTypePointer StorageBuffer %array
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%buffer_ptr = OpUntypedAccessChainKHR %ptr_uniformconstant %storage_buffer_array_type %resource_heap %int_0
%buffer_data_ptr = OpBufferPointerEXT %_ptr_StorageBuffer_array %buffer_ptr
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Array stride 2 must satisfy alignment 4"));
}

TEST_F(ValidateExplicitLayout, StructOffsetIdEXTSamplerHeapGood) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn SamplerHeapEXT
OpMemberDecorateIdEXT %Struct 0 OffsetIdEXT %int_0
OpMemberDecorateIdEXT %Struct 1 OffsetIdEXT %int_4
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%sampler = OpTypeSampler
%Struct = OpTypeStruct %sampler %int
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpUntypedAccessChainKHR %ptr_uniformconstant %Struct %resource_heap %int_0
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
}

TEST_F(ValidateExplicitLayout, StructOffsetIdEXTSamplerHeapUnalignedBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn SamplerHeapEXT
OpMemberDecorateIdEXT %Struct 0 OffsetIdEXT %int_0
OpMemberDecorateIdEXT %Struct 1 OffsetIdEXT %int_2
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%sampler = OpTypeSampler
%Struct = OpTypeStruct %sampler %int
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpUntypedAccessChainKHR %ptr_uniformconstant %Struct %resource_heap %int_0
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 2 is not aligned to 4"));
}

TEST_F(ValidateExplicitLayout, StructOffsetIdEXTResourceHeapOverlapBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpCapability SampledBuffer
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn ResourceHeapEXT
OpMemberDecorateIdEXT %Struct 0 OffsetIdEXT %int_0
OpMemberDecorateIdEXT %Struct 1 OffsetIdEXT %int_4
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%image = OpTypeImage %int Buffer 0 0 0 2 R32ui
%Struct = OpTypeStruct %image %int
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpUntypedAccessChainKHR %ptr_uniformconstant %Struct %resource_heap %int_0
OpReturn
OpFunctionEnd
)";

  options_->image_descriptor_layout.size = 8;
  options_->image_descriptor_layout.alignment = 8;
  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 4 overlaps previous "
                        "member ending at offset 7"));
}

TEST_F(ValidateExplicitLayout, StructOffsetIdEXTResourceHeapOverlapSpecId) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpCapability SampledBuffer
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn ResourceHeapEXT
OpMemberDecorateIdEXT %Struct 0 OffsetIdEXT %int_0
OpMemberDecorateIdEXT %Struct 1 OffsetIdEXT %int_4
OpDecorate %int_4 SpecId 1
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpSpecConstant %int 4
%image = OpTypeImage %int Buffer 0 0 0 2 R32ui
%Struct = OpTypeStruct %image %int
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpUntypedAccessChainKHR %ptr_uniformconstant %Struct %resource_heap %int_0
OpReturn
OpFunctionEnd
)";

  options_->image_descriptor_layout.size = 8;
  options_->image_descriptor_layout.alignment = 8;
  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
}

TEST_F(ValidateExplicitLayout, ArrayStrideIdEXTSamplerHeapGood) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn SamplerHeapEXT
OpDecorateId %array ArrayStrideIdEXT %int_4
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%int_10 = OpConstant %int 10
%sampler = OpTypeSampler
%array = OpTypeArray %sampler %int_10
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpUntypedAccessChainKHR %ptr_uniformconstant %array %resource_heap %int_0
OpReturn
OpFunctionEnd
)";

  options_->sampler_descriptor_layout.size = 4;
  options_->sampler_descriptor_layout.alignment = 4;
  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
}

TEST_F(ValidateExplicitLayout, ArrayStrideIdEXTSamplerHeapTooSmallBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn SamplerHeapEXT
OpDecorateId %array ArrayStrideIdEXT %int_2
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%int_10 = OpConstant %int 10
%sampler = OpTypeSampler
%array = OpTypeArray %sampler %int_10
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpUntypedAccessChainKHR %ptr_uniformconstant %array %resource_heap %int_0
OpReturn
OpFunctionEnd
)";

  options_->sampler_descriptor_layout.size = 4;
  options_->sampler_descriptor_layout.alignment = 2;
  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Array stride 2 is smaller than element type size 4"));
}

TEST_F(ValidateExplicitLayout,
       ArrayStrideIdEXTSamplerHeapTooSmallSpecConstant) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn SamplerHeapEXT
OpDecorateId %array ArrayStrideIdEXT %int_2
OpDecorate %int_2 SpecId 1
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_2 = OpSpecConstant %int 2
%int_10 = OpConstant %int 10
%sampler = OpTypeSampler
%array = OpTypeArray %sampler %int_10
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpUntypedAccessChainKHR %ptr_uniformconstant %array %resource_heap %int_0
OpReturn
OpFunctionEnd
)";

  options_->sampler_descriptor_layout.size = 4;
  options_->sampler_descriptor_layout.alignment = 2;
  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
}

TEST_F(ValidateExplicitLayout, ImageArraySpecConstant) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpCapability ImageBuffer
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "main" %2
OpExecutionMode %1 LocalSize 1 1 1
OpDecorate %2 BuiltIn ResourceHeapEXT
OpDecorate %3 SpecId 0
OpDecorateId %4 ArrayStrideIdEXT %3
%5 = OpTypeVoid
%6 = OpTypeFunction %5
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%9 = OpConstant %7 2
%10 = OpConstant %7 51966
%11 = OpConstant %7 0
%3 = OpSpecConstant %7 0
%12 = OpTypeUntypedPointerKHR UniformConstant
%2 = OpUntypedVariableKHR %12 UniformConstant
%13 = OpTypeImage %7 Buffer 0 0 0 2 R32ui
%4 = OpTypeRuntimeArray %13
%1 = OpFunction %5 None %6
%14 = OpLabel
%15 = OpUntypedAccessChainKHR %12 %4 %2 %9
%16 = OpLoad %13 %15
OpImageWrite %16 %8 %10
OpReturn
OpFunctionEnd
)";

  options_->image_descriptor_layout.size = 64;
  options_->image_descriptor_layout.alignment = 64;
  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
}

TEST_F(ValidateExplicitLayout, CheckNoLayoutWithOffsetIdEXTBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpMemberDecorateIdEXT %Struct 0 OffsetIdEXT %int_0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%sampler = OpTypeSampler
%Struct = OpTypeStruct %sampler %int
%_ptr_Function_Struct = OpTypePointer Function %Struct
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%var = OpVariable %_ptr_Function_Struct Function
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid explicit layout decorations on type"));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("has an explicit layout from the OffsetIdEXT decoration"));
}

TEST_F(ValidateExplicitLayout, CheckNoLayoutWithArrayStrideIdEXTBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorateId %array ArrayStrideIdEXT %int_4
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_4 = OpConstant %int 4
%int_10 = OpConstant %int 10
%sampler = OpTypeSampler
%array = OpTypeArray %sampler %int_10
%_ptr_Workgroup_array = OpTypePointer Workgroup %array
%var = OpVariable %_ptr_Workgroup_array Workgroup
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid explicit layout decorations on type"));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("has an explicit layout from the ArrayStrideIdEXT decoration"));
}

TEST_F(ValidateExplicitLayout, DescriptorArrayWithArrayStrideBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability DescriptorHeapEXT
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %array ArrayStride 16
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_4 = OpConstant %int 4
%sampler = OpTypeSampler
%array = OpTypeArray %sampler %int_4
%_ptr_UniformConstant_array = OpTypePointer UniformConstant %array
%var = OpVariable %_ptr_UniformConstant_array UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid explicit layout decorations on type"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("the UniformConstant storage class has an explicit "
                        "layout from the ArrayStride decoration"));
}

TEST_F(ValidateExplicitLayout, DescriptorArrayWithArrayStrideIdEXTBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpCapability SampledBuffer
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorateId %array ArrayStrideIdEXT %int_8
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_4 = OpConstant %int 4
%int_8 = OpConstant %int 8
%image = OpTypeImage %int Buffer 0 0 0 2 R32ui
%array = OpTypeArray %image %int_4
%_ptr_UniformConstant_array = OpTypePointer UniformConstant %array
%var = OpVariable %_ptr_UniformConstant_array UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid explicit layout decorations on type"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("the UniformConstant storage class has an explicit "
                        "layout from the ArrayStrideIdEXT decoration"));
}

TEST_F(ValidateExplicitLayout, ResourceHeapMissingArrayStride) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_EXT_descriptor_heap"
OpExtension "SPV_KHR_untyped_pointers"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap %_
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn ResourceHeapEXT
OpDecorate %Heap Block
OpMemberDecorate %Heap 0 Offset 0
OpDecorate %UBO Block
OpMemberDecorate %UBO 0 Offset 0
OpDecorate %_ Binding 0
OpDecorate %_ DescriptorSet 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%uint = OpTypeInt 32 0
%Heap = OpTypeStruct %uint
%int_0 = OpConstant %int 0
%uint_0 = OpConstant %uint 0
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
%16 = OpTypeBufferEXT StorageBuffer
%17 = OpConstantSizeOfEXT %int %16
%_runtimearr_16 = OpTypeRuntimeArray %16
%UBO = OpTypeStruct %uint
%_ptr_Uniform_UBO = OpTypePointer Uniform %UBO
%_ = OpVariable %_ptr_Uniform_UBO Uniform
%main = OpFunction %void None %3
%5 = OpLabel
%15 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_16 %resource_heap %int_3
%19 = OpBufferPointerEXT %_ptr_StorageBuffer %15
%20 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %Heap %19 %int_0
OpStore %20 %uint_0
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Array must be explicitly laid out with ArrayStride or "
                        "ArrayStrideIdEXT decorations"));
}

TEST_F(ValidateExplicitLayout, SamplerDescriptorLayoutSamplerHeapGood) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn SamplerHeapEXT
OpDecorate %Struct Block
OpMemberDecorate %Struct 0 Offset 0
OpMemberDecorate %Struct 1 Offset 8
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%sampler = OpTypeSampler
%Struct = OpTypeStruct %sampler %int
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpUntypedAccessChainKHR %ptr_uniformconstant %Struct %resource_heap %int_0
OpReturn
OpFunctionEnd
)";

  options_->sampler_descriptor_layout.size = 8;
  options_->sampler_descriptor_layout.alignment = 8;
  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
}

TEST_F(ValidateExplicitLayout, ImageDescriptorLayoutResourceHeapGood) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpCapability SampledBuffer
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn ResourceHeapEXT
OpDecorate %array ArrayStride 16
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%image = OpTypeImage %int Buffer 0 0 0 2 R32ui
%array = OpTypeArray %image %int_4
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpUntypedAccessChainKHR %ptr_uniformconstant %array %resource_heap %int_0
OpReturn
OpFunctionEnd
)";

  options_->image_descriptor_layout.size = 16;
  options_->image_descriptor_layout.alignment = 16;
  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_4));
}

TEST_F(ValidateExplicitLayout, BufferDescriptorLayoutResourceHeapTooSmallBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn ResourceHeapEXT
OpDecorate %array ArrayStride 8
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%buffer = OpTypeBufferEXT StorageBuffer
%array = OpTypeArray %buffer %int_4
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpUntypedAccessChainKHR %ptr_uniformconstant %array %resource_heap %int_0
OpReturn
OpFunctionEnd
)";

  options_->buffer_descriptor_layout.size = 16;
  options_->buffer_descriptor_layout.alignment = 8;
  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Array stride 8 is smaller than element type size 16"));
}

TEST_F(ValidateExplicitLayout, BindlessTextureNVSamplerHeapUnalignedBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability UntypedPointersKHR
OpCapability DescriptorHeapEXT
OpCapability BindlessTextureNV
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_EXT_descriptor_heap"
OpExtension "SPV_NV_bindless_texture"
OpMemoryModel Logical GLSL450
OpSamplerImageAddressingModeNV 64
OpEntryPoint GLCompute %main "main" %resource_heap
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %resource_heap BuiltIn SamplerHeapEXT
OpDecorate %Struct Block
OpMemberDecorate %Struct 0 Offset 4
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%sampler = OpTypeSampler
%Struct = OpTypeStruct %sampler
%ptr_uniformconstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %ptr_uniformconstant UniformConstant
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpUntypedAccessChainKHR %ptr_uniformconstant %Struct %resource_heap %int_0
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 0 at offset 4 is not aligned to 8"));
}

TEST_F(ValidateExplicitLayout,
       StorageBufferPtrInFunctionVariableWithArrayStrideGood) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointersStorageBuffer
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %ptr_sb ArrayStride 8
%void = OpTypeVoid
%int = OpTypeInt 32 0
%float = OpTypeFloat 32
%ptr_sb = OpTypePointer StorageBuffer %float
%_ptr_Function_ptr_sb = OpTypePointer Function %ptr_sb
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%var = OpVariable %_ptr_Function_ptr_sb Function
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
}

TEST_F(ValidateExplicitLayout, NonPointerInFunctionVariableWithArrayStrideBad) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %array ArrayStride 4
%void = OpTypeVoid
%int = OpTypeInt 32 0
%float = OpTypeFloat 32
%int_4 = OpConstant %int 4
%array = OpTypeArray %float %int_4
%_ptr_Function_array = OpTypePointer Function %array
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%var = OpVariable %_ptr_Function_array Function
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid explicit layout decorations on type"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("the Function storage class has an explicit layout "
                        "from the ArrayStride decoration"));
}

TEST_F(ValidateExplicitLayout, MatrixStride_TooSmall) {
  const std::string spirv = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %PushConstant Block
               OpMemberDecorate %PushConstant 0 Offset 0
               OpMemberDecorate %Transform_0 0 ColMajor
               OpMemberDecorate %Transform_0 0 MatrixStride 12
               OpMemberDecorate %Transform_0 0 Offset 0
               OpMemberDecorate %Transform_0 1 Offset 36
               OpMemberDecorate %Transform_0 2 Offset 48
               OpMemberDecorate %Transform_0 3 Offset 60
               OpDecorate %_runtimearr_Transform_0 ArrayStride 64
               OpDecorate %Transforms Block
               OpMemberDecorate %Transforms 0 NonWritable
               OpMemberDecorate %Transforms 0 Offset 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
  %Transform = OpTypeStruct %mat3v3float %v3float %v3float %float
%_ptr_Function_Transform = OpTypePointer Function %Transform
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_Transforms PhysicalStorageBuffer
%PushConstant = OpTypeStruct %_ptr_PhysicalStorageBuffer_Transforms
%Transform_0 = OpTypeStruct %mat3v3float %v3float %v3float %float
%_runtimearr_Transform_0 = OpTypeRuntimeArray %Transform_0
 %Transforms = OpTypeStruct %_runtimearr_Transform_0
%_ptr_PhysicalStorageBuffer_Transforms = OpTypePointer PhysicalStorageBuffer %Transforms
%_ptr_PushConstant_PushConstant = OpTypePointer PushConstant %PushConstant
          %_ = OpVariable %_ptr_PushConstant_PushConstant PushConstant
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_PushConstant__ptr_PhysicalStorageBuffer_Transforms = OpTypePointer PushConstant %_ptr_PhysicalStorageBuffer_Transforms
%_ptr_PhysicalStorageBuffer_Transform_0 = OpTypePointer PhysicalStorageBuffer %Transform_0
       %main = OpFunction %void None %3
          %5 = OpLabel
  %transform = OpVariable %_ptr_Function_Transform Function
         %22 = OpAccessChain %_ptr_PushConstant__ptr_PhysicalStorageBuffer_Transforms %_ %int_0
         %23 = OpLoad %_ptr_PhysicalStorageBuffer_Transforms %22
         %25 = OpAccessChain %_ptr_PhysicalStorageBuffer_Transform_0 %23 %int_0 %int_0
         %26 = OpLoad %Transform_0 %25 Aligned 64
         %27 = OpCopyLogical %Transform %26
               OpStore %transform %27
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Matrix with a stride 12 not satisfying alignment to 16"));
}

TEST_F(ValidateExplicitLayout, Offset_Unaligned) {
  const std::string spirv = R"(
               OpCapability PhysicalStorageBufferAddresses
               OpCapability Shader
               OpExtension "SPV_KHR_physical_storage_buffer"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %uniforms
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_ptr_PhysicalStorageBuffer_BDA_natural ArrayStride 260
               OpDecorate %Uniforms_std430 Block
               OpMemberDecorate %Uniforms_std430 0 Offset 0
               OpMemberDecorate %Uniforms_std430 1 Offset 8
               OpDecorate %_ptr_PhysicalStorageBuffer_uint ArrayStride 4
               OpDecorate %_ptr_PhysicalStorageBuffer__Array_natural_vector_uint_4_16 ArrayStride 256
               OpDecorate %_arr_v4uint_int_16 ArrayStride 16
               OpDecorate %_ptr_PhysicalStorageBuffer__arr_v4uint_int_16 ArrayStride 256
               OpDecorate %_ptr_PhysicalStorageBuffer_v4uint ArrayStride 16
               OpMemberDecorate %_Array_natural_vector_uint_4_16 0 Offset 0
               OpMemberDecorate %BDA_natural 0 Offset 0
               OpMemberDecorate %BDA_natural 1 Offset 4
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_BDA_natural PhysicalStorageBuffer
%Uniforms_std430 = OpTypeStruct %_ptr_PhysicalStorageBuffer_BDA_natural %_ptr_PhysicalStorageBuffer_BDA_natural
%_ptr_PushConstant_Uniforms_std430 = OpTypePointer PushConstant %Uniforms_std430
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_PushConstant_6 = OpTypePointer PushConstant %_ptr_PhysicalStorageBuffer_BDA_natural
       %uint = OpTypeInt 32 0
%_ptr_PhysicalStorageBuffer_uint = OpTypePointer PhysicalStorageBuffer %uint
      %int_1 = OpConstant %int 1
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer__Array_natural_vector_uint_4_16 PhysicalStorageBuffer
     %v4uint = OpTypeVector %uint 4
     %int_16 = OpConstant %int 16
%_arr_v4uint_int_16 = OpTypeArray %v4uint %int_16
%_ptr_PhysicalStorageBuffer__arr_v4uint_int_16 = OpTypePointer PhysicalStorageBuffer %_arr_v4uint_int_16
%_ptr_PhysicalStorageBuffer_v4uint = OpTypePointer PhysicalStorageBuffer %v4uint
%_Array_natural_vector_uint_4_16 = OpTypeStruct %_arr_v4uint_int_16
%BDA_natural = OpTypeStruct %uint %_Array_natural_vector_uint_4_16
%_ptr_PhysicalStorageBuffer_BDA_natural = OpTypePointer PhysicalStorageBuffer %BDA_natural
%_ptr_PhysicalStorageBuffer__Array_natural_vector_uint_4_16 = OpTypePointer PhysicalStorageBuffer %_Array_natural_vector_uint_4_16
   %uniforms = OpVariable %_ptr_PushConstant_Uniforms_std430 PushConstant
       %main = OpFunction %void None %13
         %22 = OpLabel
         %23 = OpAccessChain %_ptr_PushConstant_6 %uniforms %int_0
         %24 = OpLoad %_ptr_PhysicalStorageBuffer_BDA_natural %23
         %25 = OpAccessChain %_ptr_PhysicalStorageBuffer_uint %24 %int_0
         %26 = OpLoad %_ptr_PhysicalStorageBuffer_BDA_natural %23
         %27 = OpAccessChain %_ptr_PhysicalStorageBuffer__Array_natural_vector_uint_4_16 %26 %int_1
         %28 = OpAccessChain %_ptr_PhysicalStorageBuffer__arr_v4uint_int_16 %27 %int_0
         %29 = OpAccessChain %_ptr_PhysicalStorageBuffer_v4uint %28 %int_0
         %30 = OpLoad %v4uint %29 Aligned 4
         %31 = OpCompositeExtract %uint %30 0
               OpStore %25 %31 Aligned 4
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure member 1 at offset 4 is not aligned to 16"));
}

using UntypedPointerLayout =
    spvtest::ValidateBase<std::tuple<std::string, std::string>>;

TEST_P(UntypedPointerLayout, BadOffset) {
  const auto sc = std::get<0>(GetParam());
  const auto op = std::get<1>(GetParam());
  const std::string set = (sc == "StorageBuffer" || sc == "Uniform"
                               ? R"(OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
)"
                               : R"()");
  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpCapability WorkgroupMemoryExplicitLayoutKHR
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_KHR_variable_pointers"
OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var
OpExecutionMode %main LocalSize 1 1 1
OpName %var "var"
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
OpMemberDecorate %struct 1 Offset 4
)" + set + R"(OpMemberDecorate %test_type 0 Offset 0
OpMemberDecorate %test_type 1 Offset 1
OpDecorate %ptr ArrayStride 16
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%struct = OpTypeStruct %int %int
%test_type = OpTypeStruct %int %int
%test_val = OpConstantNull %test_type
%ptr = OpTypeUntypedPointerKHR )" +
                            sc + R"(
%var = OpUntypedVariableKHR %ptr )" +
                            sc + R"( %struct
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
)" + op + R"(
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  const bool read_only = sc == "Uniform" || sc == "PushConstant";
  if (!read_only || op.find("OpStore") == std::string::npos) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("member 1 at offset 1 is not aligned to"));
  }
}

TEST_P(UntypedPointerLayout, BadStride_TooSmall) {
  const auto sc = std::get<0>(GetParam());
  const auto op = std::get<1>(GetParam());
  const std::string set = (sc == "StorageBuffer" || sc == "Uniform"
                               ? R"(OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
)"
                               : R"()");
  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpCapability WorkgroupMemoryExplicitLayoutKHR
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_KHR_variable_pointers"
OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var
OpExecutionMode %main LocalSize 1 1 1
OpName %var "var"
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
OpMemberDecorate %struct 1 Offset 4
)" + set + R"(OpDecorate %test_type ArrayStride 4
OpDecorate %ptr ArrayStride 16
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%int4 = OpTypeVector %int 4
%test_type = OpTypeArray %int4 %int_4
%test_val = OpConstantNull %test_type
%struct = OpTypeStruct %int %int
%ptr = OpTypeUntypedPointerKHR )" +
                            sc + R"(
%var = OpUntypedVariableKHR %ptr )" +
                            sc + R"( %struct
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
)" + op + R"(
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  const bool read_only = sc == "Uniform" || sc == "PushConstant";
  if (!read_only || op.find("OpStore") == std::string::npos) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Array stride 4 must satisfy alignment 16"));
  }
}

TEST_P(UntypedPointerLayout, BadStride_Unaligned) {
  const auto sc = std::get<0>(GetParam());
  const auto op = std::get<1>(GetParam());
  const std::string set = (sc == "StorageBuffer" || sc == "Uniform"
                               ? R"(OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
)"
                               : R"()");
  const std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointers
OpCapability UntypedPointersKHR
OpCapability WorkgroupMemoryExplicitLayoutKHR
OpExtension "SPV_KHR_untyped_pointers"
OpExtension "SPV_KHR_variable_pointers"
OpExtension "SPV_KHR_workgroup_memory_explicit_layout"
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var
OpExecutionMode %main LocalSize 1 1 1
OpName %var "var"
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
OpMemberDecorate %struct 1 Offset 4
)" + set + R"(OpDecorate %test_type ArrayStride 15
OpDecorate %ptr ArrayStride 16
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%int4 = OpTypeVector %int 4
%test_type = OpTypeArray %int4 %int_4
%test_val = OpConstantNull %test_type
%struct = OpTypeStruct %int %int
%ptr = OpTypeUntypedPointerKHR )" +
                            sc + R"(
%var = OpUntypedVariableKHR %ptr )" +
                            sc + R"( %struct
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
)" + op + R"(
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  const bool read_only = sc == "Uniform" || sc == "PushConstant";
  if (!read_only || op.find("OpStore") == std::string::npos) {
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("Array stride 15 must satisfy alignment 16"));
  }
}

INSTANTIATE_TEST_SUITE_P(
    ValidateUntypedPointerLayout, UntypedPointerLayout,
    Combine(Values("StorageBuffer", "Uniform", "PushConstant", "Workgroup"),
            Values("%gep = OpUntypedAccessChainKHR %ptr %test_type %var %int_0",
                   "%gep = OpUntypedInBoundsAccessChainKHR %ptr %test_type "
                   "%var %int_0",
                   "%gep = OpUntypedPtrAccessChainKHR %ptr %test_type %var "
                   "%int_0 %int_0",
                   "%gep = OpUntypedInBoundsPtrAccessChainKHR %ptr %test_type "
                   "%var %int_0 %int_0",
                   "%ld = OpLoad %test_type %var", "OpStore %var %test_val")));

}  // namespace
}  // namespace val
}  // namespace spvtools
