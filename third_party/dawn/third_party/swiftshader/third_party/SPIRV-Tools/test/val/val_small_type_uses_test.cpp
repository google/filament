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

// Validation tests for 8- and 16-bit type uses.

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "test/unit_spirv.h"
#include "test/val/val_code_generator.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Values;

using ValidateSmallTypeUses = spvtest::ValidateBase<bool>;

CodeGenerator GetSmallTypesGenerator() {
  CodeGenerator generator;
  generator.capabilities_ = R"(
OpCapability Shader
OpCapability StorageBuffer16BitAccess
OpCapability StorageBuffer8BitAccess
)";
  generator.extensions_ = R"(
OpExtension "SPV_KHR_16bit_storage"
OpExtension "SPV_KHR_8bit_storage"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%ext = OpExtInstImport "GLSL.std.450"
)";
  generator.memory_model_ = "OpMemoryModel Logical GLSL450\n";
  std::string body = R"(
%short_gep = OpAccessChain %ptr_ssbo_short %var %int_0 %int_0
%ld_short = OpLoad %short %short_gep
%short_to_int = OpSConvert %int %ld_short
%short_to_uint = OpUConvert %int %ld_short
%short_to_char = OpSConvert %char %ld_short
%short_to_uchar = OpSConvert %char %ld_short
%short2_gep = OpAccessChain %ptr_ssbo_short2 %var %int_0
%ld_short2 = OpLoad %short2 %short2_gep
%short2_to_int2 = OpSConvert %int2 %ld_short2
%short2_to_uint2 = OpUConvert %int2 %ld_short2
%short2_to_char2 = OpSConvert %char2 %ld_short2
%short2_to_uchar2 = OpSConvert %char2 %ld_short2

%char_gep = OpAccessChain %ptr_ssbo_char %var %int_2 %int_0
%ld_char = OpLoad %char %char_gep
%char_to_int = OpSConvert %int %ld_char
%char_to_uint = OpUConvert %int %ld_char
%char_to_short = OpSConvert %short %ld_char
%char_to_ushort = OpSConvert %short %ld_char
%char2_gep = OpAccessChain %ptr_ssbo_char2 %var %int_2
%ld_char2 = OpLoad %char2 %char2_gep
%char2_to_int2 = OpSConvert %int2 %ld_char2
%char2_to_uint2 = OpUConvert %int2 %ld_char2
%char2_to_short2 = OpSConvert %short2 %ld_char2
%char2_to_ushort2 = OpSConvert %short2 %ld_char2

%half_gep = OpAccessChain %ptr_ssbo_half %var %int_1 %int_0
%ld_half = OpLoad %half %half_gep
%half_to_float = OpFConvert %float %ld_half
%half2_gep = OpAccessChain %ptr_ssbo_half2 %var %int_1
%ld_half2 = OpLoad %half2 %half2_gep
%half2_to_float2 = OpFConvert %float2 %ld_half2

%int_to_short = OpSConvert %short %int_0
%int_to_ushort = OpUConvert %short %int_0
%int_to_char = OpSConvert %char %int_0
%int_to_uchar = OpUConvert %char %int_0
%int2_to_short2 = OpSConvert %short2 %int2_0
%int2_to_ushort2 = OpUConvert %short2 %int2_0
%int2_to_char2 = OpSConvert %char2 %int2_0
%int2_to_uchar2 = OpUConvert %char2 %int2_0
%int_gep = OpAccessChain %ptr_ssbo_int %var %int_3 %int_0
%int2_gep = OpAccessChain %ptr_ssbo_int2 %var %int_3

%float_to_half = OpFConvert %half %float_0
%float2_to_half2 = OpFConvert %half2 %float2_0
%float_gep = OpAccessChain %ptr_ssbo_float %var %int_4 %int_0
%float2_gep = OpAccessChain %ptr_ssbo_float2 %var %int_4
)";
  generator.entry_points_.push_back(
      {"foo", "GLCompute", "OpExecutionMode %foo LocalSize 1 1 1", body, ""});
  generator.before_types_ = R"(
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpMemberDecorate %block 1 Offset 8
OpMemberDecorate %block 2 Offset 16
OpMemberDecorate %block 3 Offset 32
OpMemberDecorate %block 4 Offset 64
)";
  generator.types_ = R"(
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int2 = OpTypeVector %int 2
%float = OpTypeFloat 32
%float2 = OpTypeVector %float 2
%bool = OpTypeBool
%bool2 = OpTypeVector %bool 2
%char = OpTypeInt 8 0
%char2 = OpTypeVector %char 2
%ptr_ssbo_char = OpTypePointer StorageBuffer %char
%ptr_ssbo_char2 = OpTypePointer StorageBuffer %char2
%short = OpTypeInt 16 0
%short2 = OpTypeVector %short 2
%ptr_ssbo_short = OpTypePointer StorageBuffer %short
%ptr_ssbo_short2 = OpTypePointer StorageBuffer %short2
%half = OpTypeFloat 16
%half2 = OpTypeVector %half 2
%ptr_ssbo_half = OpTypePointer StorageBuffer %half
%ptr_ssbo_half2 = OpTypePointer StorageBuffer %half2
%ptr_ssbo_int = OpTypePointer StorageBuffer %int
%ptr_ssbo_int2 = OpTypePointer StorageBuffer %int2
%ptr_ssbo_float = OpTypePointer StorageBuffer %float
%ptr_ssbo_float2 = OpTypePointer StorageBuffer %float2
%block = OpTypeStruct %short2 %half2 %char2 %int2 %float2
%ptr_ssbo_block = OpTypePointer StorageBuffer %block
%func = OpTypeFunction %void
)";
  generator.after_types_ = R"(
%var = OpVariable %ptr_ssbo_block StorageBuffer
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%int2_0 = OpConstantComposite %int2 %int_0 %int_0
%float_0 = OpConstant %float 0
%float2_0 = OpConstantComposite %float2 %float_0 %float_0

%short_func_ty = OpTypeFunction %void %short
%char_func_ty = OpTypeFunction %void %char
%half_func_ty = OpTypeFunction %void %half
)";
  generator.add_at_the_end_ = R"(
%short_func = OpFunction %void None %short_func_ty
%short_param = OpFunctionParameter %short
%short_func_entry = OpLabel
OpReturn
OpFunctionEnd
%char_func = OpFunction %void None %char_func_ty
%char_param = OpFunctionParameter %char
%char_func_entry = OpLabel
OpReturn
OpFunctionEnd
%half_func = OpFunction %void None %half_func_ty
%half_param = OpFunctionParameter %half
%half_func_entry = OpLabel
OpReturn
OpFunctionEnd
)";

  return generator;
}

TEST_F(ValidateSmallTypeUses, BadCharPhi) {
  CodeGenerator generator = GetSmallTypesGenerator();
  generator.entry_points_[0].body += R"(
OpBranch %next_block
%next_block = OpLabel
%phi = OpPhi %char %ld_char %foo_entry
)";

  CompileSuccessfully(generator.Build());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid use of 8- or 16-bit result"));
}

TEST_F(ValidateSmallTypeUses, BadShortPhi) {
  CodeGenerator generator = GetSmallTypesGenerator();
  generator.entry_points_[0].body += R"(
OpBranch %next_block
%next_block = OpLabel
%phi = OpPhi %short %ld_short %foo_entry
)";

  CompileSuccessfully(generator.Build());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid use of 8- or 16-bit result"));
}

TEST_F(ValidateSmallTypeUses, BadHalfPhi) {
  CodeGenerator generator = GetSmallTypesGenerator();
  generator.entry_points_[0].body += R"(
OpBranch %next_block
%next_block = OpLabel
%phi = OpPhi %half %ld_half %foo_entry
)";

  CompileSuccessfully(generator.Build());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid use of 8- or 16-bit result"));
}

using ValidateGoodUses = spvtest::ValidateBase<std::string>;

TEST_P(ValidateGoodUses, Inst) {
  const std::string inst = GetParam();
  CodeGenerator generator = GetSmallTypesGenerator();
  generator.entry_points_[0].body += inst + "\n";

  CompileSuccessfully(generator.Build());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

INSTANTIATE_TEST_SUITE_P(
    SmallTypeUsesValid, ValidateGoodUses,
    Values(
        "%inst = OpIAdd %int %short_to_int %int_0",
        "%inst = OpIAdd %int %short_to_uint %int_0",
        "%inst = OpIAdd %int2 %short2_to_int2 %int2_0",
        "%inst = OpIAdd %int2 %short2_to_uint2 %int2_0",
        "%inst = OpIAdd %int %char_to_int %int_0",
        "%inst = OpIAdd %int %char_to_uint %int_0",
        "%inst = OpIAdd %int2 %char2_to_int2 %int2_0",
        "%inst = OpIAdd %int2 %char2_to_uint2 %int2_0",
        "%inst = OpUConvert %int %ld_short",
        "%inst = OpSConvert %int %ld_short",
        "%inst = OpUConvert %char %ld_short",
        "%inst = OpSConvert %char %ld_short",
        "%inst = OpUConvert %int %ld_char", "%inst = OpSConvert %int %ld_char",
        "%inst = OpUConvert %short %ld_char",
        "%inst = OpSConvert %short %ld_char",
        "%inst = OpUConvert %int2 %ld_short2",
        "%inst = OpSConvert %int2 %ld_short2",
        "%inst = OpUConvert %char2 %ld_short2",
        "%inst = OpSConvert %char2 %ld_short2",
        "%inst = OpUConvert %int2 %ld_char2",
        "%inst = OpSConvert %int2 %ld_char2",
        "%inst = OpUConvert %short2 %ld_char2",
        "%inst = OpSConvert %short2 %ld_char2",
        "OpStore %short_gep %int_to_short", "OpStore %short_gep %int_to_ushort",
        "OpStore %short_gep %char_to_short",
        "OpStore %short_gep %char_to_ushort",
        "OpStore %short2_gep %int2_to_short2",
        "OpStore %short2_gep %int2_to_ushort2",
        "OpStore %short2_gep %char2_to_short2",
        "OpStore %short2_gep %char2_to_ushort2",
        "OpStore %char_gep %int_to_char", "OpStore %char_gep %int_to_uchar",
        "OpStore %char_gep %short_to_char", "OpStore %char_gep %short_to_uchar",
        "OpStore %char2_gep %int2_to_char2",
        "OpStore %char2_gep %int2_to_uchar2",
        "OpStore %char2_gep %short2_to_char2",
        "OpStore %char2_gep %short2_to_uchar2",
        "OpStore %int_gep %short_to_int", "OpStore %int_gep %short_to_uint",
        "OpStore %int_gep %char_to_int", "OpStore %int2_gep %char2_to_uint2",
        "OpStore %int2_gep %short2_to_int2",
        "OpStore %int2_gep %short2_to_uint2",
        "OpStore %int2_gep %char2_to_int2", "OpStore %int2_gep %char2_to_uint2",
        "%inst = OpFAdd %float %half_to_float %float_0",
        "%inst = OpFAdd %float2 %half2_to_float2 %float2_0",
        "%inst = OpFConvert %float %ld_half",
        "%inst = OpFConvert %float2 %ld_half2",
        "OpStore %half_gep %float_to_half", "OpStore %half_gep %ld_half",
        "OpStore %half2_gep %float2_to_half2", "OpStore %half2_gep %ld_half2",
        "OpStore %float_gep %half_to_float",
        "OpStore %float2_gep %half2_to_float2"));

using ValidateBadUses = spvtest::ValidateBase<std::string>;

TEST_P(ValidateBadUses, Inst) {
  const std::string inst = GetParam();
  CodeGenerator generator = GetSmallTypesGenerator();
  generator.entry_points_[0].body += inst + "\n";

  CompileSuccessfully(generator.Build());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Invalid use of 8- or 16-bit result"));
}

// A smattering of unacceptable use cases. Far too vast to cover exhaustively.
INSTANTIATE_TEST_SUITE_P(
    SmallTypeUsesInvalid, ValidateBadUses,
    Values("%inst = OpIAdd %short %ld_short %ld_short",
           "%inst = OpIAdd %short %char_to_short %char_to_short",
           "%inst = OpIAdd %short %char_to_ushort %char_to_ushort",
           "%inst = OpIAdd %short %int_to_short %int_to_short",
           "%inst = OpIAdd %short %int_to_ushort %int_to_ushort",
           "%inst = OpIAdd %short2 %ld_short2 %ld_short2",
           "%inst = OpIAdd %short2 %char2_to_short2 %char2_to_short2",
           "%inst = OpIAdd %short2 %char2_to_ushort2 %char2_to_ushort2",
           "%inst = OpIAdd %short2 %int2_to_short2 %int2_to_short2",
           "%inst = OpIAdd %short2 %int2_to_ushort2 %int2_to_ushort2",
           "%inst = OpIEqual %bool %ld_short %ld_short",
           "%inst = OpIEqual %bool %char_to_short %char_to_short",
           "%inst = OpIEqual %bool %char_to_ushort %char_to_ushort",
           "%inst = OpIEqual %bool %int_to_short %int_to_short",
           "%inst = OpIEqual %bool %int_to_ushort %int_to_ushort",
           "%inst = OpIEqual %bool2 %ld_short2 %ld_short2",
           "%inst = OpIEqual %bool2 %char2_to_short2 %char2_to_short2",
           "%inst = OpIEqual %bool2 %char2_to_ushort2 %char2_to_ushort2",
           "%inst = OpIEqual %bool2 %int2_to_short2 %int2_to_short2",
           "%inst = OpIEqual %bool2 %int2_to_ushort2 %int2_to_ushort2",
           "%inst = OpFAdd %half %ld_half %ld_half",
           "%inst = OpFAdd %half %float_to_half %float_to_half",
           "%inst = OpFAdd %half2 %ld_half2 %ld_half2",
           "%inst = OpFAdd %half2 %float2_to_half2 %float2_to_half2",
           "%inst = OpFOrdGreaterThan %bool %ld_half %ld_half",
           "%inst = OpFOrdGreaterThan %bool %float_to_half %float_to_half",
           "%inst = OpFOrdGreaterThan %bool2 %ld_half2 %ld_half2",
           "%inst = OpFOrdGreaterThan %bool2 %float2_to_half2 %float2_to_half2",
           "%inst = OpFunctionCall %void %short_func %ld_short",
           "%inst = OpFunctionCall %void %short_func %char_to_short",
           "%inst = OpFunctionCall %void %short_func %char_to_ushort",
           "%inst = OpFunctionCall %void %short_func %int_to_short",
           "%inst = OpFunctionCall %void %short_func %int_to_ushort",
           "%inst = OpFunctionCall %void %char_func %ld_char",
           "%inst = OpFunctionCall %void %char_func %short_to_char",
           "%inst = OpFunctionCall %void %char_func %short_to_uchar",
           "%inst = OpFunctionCall %void %char_func %int_to_char",
           "%inst = OpFunctionCall %void %char_func %int_to_uchar",
           "%inst = OpFunctionCall %void %half_func %ld_half",
           "%inst = OpFunctionCall %void %half_func %float_to_half"));

}  // namespace
}  // namespace val
}  // namespace spvtools
