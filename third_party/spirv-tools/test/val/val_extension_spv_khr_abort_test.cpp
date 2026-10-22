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

// Tests for OpExtension validator rules.

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "source/spirv_target_env.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::Values;
using ::testing::ValuesIn;

using ValidateSpvKHRAbort = spvtest::ValidateBase<bool>;

TEST_F(ValidateSpvKHRAbort, Valid) {
  const std::string str = R"(
OpCapability Shader
OpCapability AbortKHR
OpExtension "SPV_KHR_abort"
OpMemoryModel Logical Simple
OpEntryPoint GLCompute %main "main"

%void    = OpTypeVoid
%void_fn = OpTypeFunction %void
%uint32_t = OpTypeInt 32 0
%payload = OpConstant %uint32_t 6
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpAbortKHR %uint32_t %payload
OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateSpvKHRAbort, RequireFinalInstructionInBlock) {
  const std::string str = R"(
            OpCapability Shader
            OpCapability AbortKHR
            OpExtension "SPV_KHR_abort"
            OpMemoryModel Logical Simple
            OpEntryPoint GLCompute %main "main"

%void     = OpTypeVoid
%void_fn  = OpTypeFunction %void
%uint32_t = OpTypeInt 32 0
%payload  = OpConstant %uint32_t 6
%main     = OpFunction %void None %void_fn
%entry    = OpLabel
            OpAbortKHR %uint32_t %payload
            OpReturn
            OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Return must appear in a block"));
}

TEST_F(ValidateSpvKHRAbort, RequiresCapability) {
  const std::string str = R"(
            OpCapability Shader
            OpExtension "SPV_KHR_abort"
            OpMemoryModel Logical Simple
            OpEntryPoint GLCompute %main "main"

%void     = OpTypeVoid
%void_fn  = OpTypeFunction %void
%uint32_t = OpTypeInt 32 0
%payload  = OpConstant %uint32_t 6
%main     = OpFunction %void None %void_fn
%entry    = OpLabel
            OpAbortKHR %uint32_t %payload
            OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Opcode AbortKHR requires one of these capabilities: AbortKHR"));
}

TEST_F(ValidateSpvKHRAbort, RequiresExtension) {
  const std::string str = R"(
            OpCapability Shader
            OpCapability AbortKHR
            OpMemoryModel Logical Simple
            OpEntryPoint GLCompute %main "main"

%void     = OpTypeVoid
%void_fn  = OpTypeFunction %void
%uint32_t = OpTypeInt 32 0
%payload  = OpConstant %uint32_t 6
%main     = OpFunction %void None %void_fn
%entry    = OpLabel
            OpAbortKHR %uint32_t %payload
            OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("1st operand of Capability: operand AbortKHR(5120) "
                        "requires one of these extensions: SPV_KHR_abort"));
}

TEST_F(ValidateSpvKHRAbort, MismatchedOperandTypes) {
  const std::string str = R"(
            OpCapability Shader
            OpCapability AbortKHR
            OpExtension "SPV_KHR_abort"
            OpMemoryModel Logical Simple
            OpEntryPoint GLCompute %main "main"

%void     = OpTypeVoid
%void_fn  = OpTypeFunction %void
%uint32_t = OpTypeInt 32 0
%f32_t    = OpTypeFloat 32
%payload  = OpConstant %uint32_t 6
%main     = OpFunction %void None %void_fn
%entry    = OpLabel
            OpAbortKHR %f32_t %payload
            OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Type of Message operand does not logically match "
                        "the type of the Message Type operand"));
}

TEST_F(ValidateSpvKHRAbort, ValidCompositeOperandTypes) {
  const std::string str = R"(
             OpCapability Shader
             OpCapability Int8
             OpCapability AbortKHR
             OpCapability ConstantDataKHR
             OpExtension "SPV_KHR_abort"
             OpExtension "SPV_KHR_constant_data"
             OpMemoryModel Logical Simple
             OpEntryPoint GLCompute %main "main"

             OpDecorate %string1_t UTFEncodedKHR
             OpDecorate %string2_t UTFEncodedKHR

             OpDecorate %string1_x UTFEncodedKHR
             OpDecorate %string1_x ArrayStride 1
             OpDecorate %string2_x UTFEncodedKHR
             OpDecorate %string2_x ArrayStride 1
             OpMemberDecorate %message_x 0 Offset 0
             OpMemberDecorate %message_x 1 Offset 6
             OpMemberDecorate %message_x 2 Offset 8

%void      = OpTypeVoid
%void_fn   = OpTypeFunction %void

   %char_t = OpTypeInt 8 0
 %uint32_t = OpTypeInt 32 0
  %str1len = OpConstant %uint32_t 6
%string1_t = OpTypeArray %char_t %str1len
  ; "test: "
  %string1 = OpConstantDataKHR %string1_t 0x74736574 0x0000203A
  %str2len = OpSpecConstant %uint32_t 2
%string2_t = OpTypeArray %char_t %str2len
  ; "%u"
  %string2 = OpSpecConstantDataKHR %string2_t 0x00007525
%message_t = OpTypeStruct %string1_t %string2_t %uint32_t
%uintval   = OpConstant %uint32_t 6

%string1_x = OpTypeArray %char_t %str1len
%string2_x = OpTypeArray %char_t %str2len
%message_x = OpTypeStruct %string1_t %string2_t %uint32_t

%main      = OpFunction %void None %void_fn
%entry     = OpLabel
  %message = OpCompositeConstruct %message_t %string1 %string2 %uintval
             OpAbortKHR %message_x %message
             OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateSpvKHRAbort, MismatchedCompositeOperandTypes) {
  const std::string str = R"(
             OpCapability Shader
             OpCapability Int8
             OpCapability AbortKHR
             OpCapability ConstantDataKHR
             OpExtension "SPV_KHR_abort"
             OpExtension "SPV_KHR_constant_data"
             OpMemoryModel Logical Simple
             OpEntryPoint GLCompute %main "main"

             OpDecorate %string1_t UTFEncodedKHR
             OpDecorate %string2_t UTFEncodedKHR

             OpDecorate %string1_x UTFEncodedKHR
             OpDecorate %string1_x ArrayStride 1
             OpDecorate %string2_x UTFEncodedKHR
             OpDecorate %string2_x ArrayStride 1
             OpMemberDecorate %message_x 0 Offset 0
             OpMemberDecorate %message_x 1 Offset 6
             OpMemberDecorate %message_x 2 Offset 8

%void      = OpTypeVoid
%void_fn   = OpTypeFunction %void

   %char_t = OpTypeInt 8 0
 %uint32_t = OpTypeInt 32 0
  %str1len = OpConstant %uint32_t 6
%string1_t = OpTypeArray %char_t %str1len
  ; "test: "
  %string1 = OpConstantDataKHR %string1_t 0x74736574 0x0000203A
  %str2len = OpSpecConstant %uint32_t 2
%string2_t = OpTypeArray %char_t %str2len
  ; "%u"
  %string2 = OpSpecConstantDataKHR %string2_t 0x00007525
%message_t = OpTypeStruct %string1_t %string2_t
%uintval   = OpConstant %uint32_t 6

%string1_x = OpTypeArray %char_t %str1len
%string2_x = OpTypeArray %char_t %str2len
%message_x = OpTypeStruct %string1_t %string2_t %uint32_t

%main      = OpFunction %void None %void_fn
%entry     = OpLabel
  %message = OpCompositeConstruct %message_t %string1 %string2
             OpAbortKHR %message_x %message
             OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Type of Message operand does not logically match "
                        "the type of the Message Type operand"));
}

TEST_F(ValidateSpvKHRAbort, ConstantDataNonArray) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability ConstantDataKHR
               OpExtension "SPV_KHR_constant_data"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
       %data = OpConstantDataKHR %uint 1
  %void_func = OpTypeFunction %void
       %main = OpFunction %void None %void_func
 %main_label = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result type must be an array."));
}

TEST_F(ValidateSpvKHRAbort, ConstantDataFloat) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability ConstantDataKHR
               OpExtension "SPV_KHR_constant_data"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
       %float = OpTypeFloat 32
  %uint_size = OpConstant %uint 1
 %uint_array = OpTypeArray %float %uint_size
       %data = OpConstantDataKHR %uint_array 1
  %void_func = OpTypeFunction %void
       %main = OpFunction %void None %void_func
 %main_label = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result type must be an array of integer scalar type"));
}

TEST_F(ValidateSpvKHRAbort, ConstantDataIntVector) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability ConstantDataKHR
               OpExtension "SPV_KHR_constant_data"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
      %uvec3 = OpTypeVector %uint 3
  %uint_size = OpConstant %uint 1
 %uint_array = OpTypeArray %uvec3 %uint_size
       %data = OpConstantDataKHR %uint_array 1
  %void_func = OpTypeFunction %void
       %main = OpFunction %void None %void_func
 %main_label = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result type must be an array of integer scalar type"));
}

TEST_F(ValidateSpvKHRAbort, ConstantDataMultiLength) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability ConstantDataKHR
               OpCapability Int8
               OpExtension "SPV_KHR_constant_data"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
       %char = OpTypeInt 8 1
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
     %uint_4 = OpConstant %uint 4
 %char_array_1 = OpTypeArray %char %uint_1
 %char_array_2 = OpTypeArray %char %uint_2
 %char_array_3 = OpTypeArray %char %uint_3
 %char_array_4 = OpTypeArray %char %uint_4
       %data_1 = OpConstantDataKHR %char_array_1 0
       %data_2 = OpConstantDataKHR %char_array_2 0
       %data_3 = OpConstantDataKHR %char_array_3 0
       %data_4 = OpConstantDataKHR %char_array_4 0
  %void_func = OpTypeFunction %void
       %main = OpFunction %void None %void_func
 %main_label = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateSpvKHRAbort, ConstantDataSpecLength) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability ConstantDataKHR
               OpExtension "SPV_KHR_constant_data"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
  %uint_size = OpSpecConstant %uint 4444
 %uint_array = OpTypeArray %uint %uint_size
       %data = OpConstantDataKHR %uint_array 0
  %void_func = OpTypeFunction %void
       %main = OpFunction %void None %void_func
 %main_label = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateSpvKHRAbort, ConstantDataSpecLengthAndData) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability ConstantDataKHR
               OpExtension "SPV_KHR_constant_data"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %uint_size SpecId 1
               OpDecorate %data SpecId 2
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
  %uint_size = OpSpecConstant %uint 4444
 %uint_array = OpTypeArray %uint %uint_size
       %data = OpSpecConstantDataKHR %uint_array 1
    ; No SpecID for this one
     %data_2 = OpSpecConstantDataKHR %uint_array 2
  %void_func = OpTypeFunction %void
       %main = OpFunction %void None %void_func
 %main_label = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateSpvKHRAbort, ConstantDataNull) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability ConstantDataKHR
               OpExtension "SPV_KHR_constant_data"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
  %uint_size = OpConstant %uint 1
 %uint_array = OpTypeArray %uint %uint_size
       %data = OpConstantDataKHR %uint_array
  %void_func = OpTypeFunction %void
       %main = OpFunction %void None %void_func
 %main_label = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("There must be at least 1 literal integer"));
}

TEST_F(ValidateSpvKHRAbort, ConstantDataLengthOverUint32) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability ConstantDataKHR
               OpExtension "SPV_KHR_constant_data"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
  %uint_size = OpConstant %uint 2
 %uint_array = OpTypeArray %uint %uint_size
       %data = OpConstantDataKHR %uint_array 1
  %void_func = OpTypeFunction %void
       %main = OpFunction %void None %void_func
 %main_label = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("contains 1 words of data, but needs to have 2 words "
                        "to match the array of 2 of 32-bit ints"));
}

TEST_F(ValidateSpvKHRAbort, ConstantDataLengthUnderUint32) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability ConstantDataKHR
               OpExtension "SPV_KHR_constant_data"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
  %uint_size = OpConstant %uint 2
 %uint_array = OpTypeArray %uint %uint_size
       %data = OpConstantDataKHR %uint_array 1 2 3
  %void_func = OpTypeFunction %void
       %main = OpFunction %void None %void_func
 %main_label = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("contains 3 words of data, but needs to have 2 words "
                        "to match the array of 2 of 32-bit ints"));
}

TEST_F(ValidateSpvKHRAbort, ConstantDataLengthOverUint8) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability ConstantDataKHR
               OpCapability Int8
               OpExtension "SPV_KHR_constant_data"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
       %char = OpTypeInt 8 1
  %uint_size = OpConstant %uint 5
 %char_array = OpTypeArray %char %uint_size
       %data = OpConstantDataKHR %char_array 1
  %void_func = OpTypeFunction %void
       %main = OpFunction %void None %void_func
 %main_label = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("contains 1 words of data, but needs to have 2 words "
                        "to match the array of 5 of 8-bit ints"));
}

TEST_F(ValidateSpvKHRAbort, ConstantDataLengthUnderUint8) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability ConstantDataKHR
               OpCapability Int8
               OpExtension "SPV_KHR_constant_data"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
       %char = OpTypeInt 8 1
  %uint_size = OpConstant %uint 4
 %char_array = OpTypeArray %char %uint_size
       %data = OpConstantDataKHR %char_array 1 2
  %void_func = OpTypeFunction %void
       %main = OpFunction %void None %void_func
 %main_label = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("contains 2 words of data, but needs to have 1 words "
                        "to match the array of 4 of 8-bit ints"));
}

TEST_F(ValidateSpvKHRAbort, ConstantDataLengthUint64Good) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability ConstantDataKHR
               OpCapability Int64
               OpExtension "SPV_KHR_constant_data"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %u64 = OpTypeInt 64 0
  %uint_size = OpConstant %uint 2
 %u64_array = OpTypeArray %u64 %uint_size
       %data = OpConstantDataKHR %u64_array 1 2 3 4
  %void_func = OpTypeFunction %void
       %main = OpFunction %void None %void_func
 %main_label = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateSpvKHRAbort, ConstantDataLengthUint64Short) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability ConstantDataKHR
               OpCapability Int64
               OpExtension "SPV_KHR_constant_data"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %u64 = OpTypeInt 64 0
  %uint_size = OpConstant %uint 1
 %u64_array = OpTypeArray %u64 %uint_size
       %data = OpConstantDataKHR %u64_array 1
  %void_func = OpTypeFunction %void
       %main = OpFunction %void None %void_func
 %main_label = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("contains 1 words of data, but needs to have 2 words "
                        "to match the array of 1 of 64-bit ints"));
}

TEST_F(ValidateSpvKHRAbort, ConstantDataLengthUint64Short2) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability ConstantDataKHR
               OpCapability Int64
               OpExtension "SPV_KHR_constant_data"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %u64 = OpTypeInt 64 0
  %uint_size = OpConstant %uint 2
 %u64_array = OpTypeArray %u64 %uint_size
       %data = OpConstantDataKHR %u64_array 1 2 3
  %void_func = OpTypeFunction %void
       %main = OpFunction %void None %void_func
 %main_label = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("contains 3 words of data, but needs to have 4 words "
                        "to match the array of 2 of 64-bit ints"));
}

TEST_F(ValidateSpvKHRAbort, ConstantDataLengthOverUint64) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability ConstantDataKHR
               OpCapability Int64
               OpExtension "SPV_KHR_constant_data"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %u64 = OpTypeInt 64 0
  %uint_size = OpConstant %uint 1
 %u64_array = OpTypeArray %u64 %uint_size
       %data = OpConstantDataKHR %u64_array 1 2 3 4
  %void_func = OpTypeFunction %void
       %main = OpFunction %void None %void_func
 %main_label = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("contains 4 words of data, but needs to have 2 words "
                        "to match the array of 1 of 64-bit ints"));
}

TEST_F(ValidateSpvKHRAbort, ConstantDataLengthUnderUint64) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability ConstantDataKHR
               OpCapability Int64
               OpExtension "SPV_KHR_constant_data"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %u64 = OpTypeInt 64 0
  %uint_size = OpConstant %uint 2
 %u64_array = OpTypeArray %u64 %uint_size
       %data = OpConstantDataKHR %u64_array 1 2
  %void_func = OpTypeFunction %void
       %main = OpFunction %void None %void_func
 %main_label = OpLabel
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("contains 2 words of data, but needs to have 4 words "
                        "to match the array of 2 of 64-bit ints"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
