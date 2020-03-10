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

// Validation tests for misc instructions

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::Eq;
using ::testing::HasSubstr;

using ValidateMisc = spvtest::ValidateBase<bool>;

TEST_F(ValidateMisc, UndefRestrictedShort) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpCapability StorageBuffer16BitAccess
OpExtension "SPV_KHR_16bit_storage"
OpMemoryModel Logical GLSL450
%short = OpTypeInt 16 0
%undef = OpUndef %short
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Cannot create undefined values with 8- or 16-bit types"));
}

TEST_F(ValidateMisc, UndefRestrictedChar) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpCapability StorageBuffer8BitAccess
OpExtension "SPV_KHR_8bit_storage"
OpMemoryModel Logical GLSL450
%char = OpTypeInt 8 0
%undef = OpUndef %char
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Cannot create undefined values with 8- or 16-bit types"));
}

TEST_F(ValidateMisc, UndefRestrictedHalf) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpCapability StorageBuffer16BitAccess
OpExtension "SPV_KHR_16bit_storage"
OpMemoryModel Logical GLSL450
%half = OpTypeFloat 16
%undef = OpUndef %half
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Cannot create undefined values with 8- or 16-bit types"));
}

const std::string ShaderClockSpriv = R"(
OpCapability Shader
OpCapability Int64
OpCapability ShaderClockKHR
OpExtension "SPV_KHR_shader_clock"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpSourceExtension "GL_ARB_gpu_shader_int64"
OpSourceExtension "GL_ARB_shader_clock"
OpSourceExtension "GL_EXT_shader_realtime_clock"
OpName %main "main"
OpName %time1 "time1"
%void = OpTypeVoid
)";

TEST_F(ValidateMisc, ShaderClockInt64) {
  const std::string spirv = ShaderClockSpriv + R"(
%3 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%uint_3 = OpConstant %uint 3
%uint_1 = OpConstant %uint 1
%main = OpFunction %void None %3
%5 = OpLabel
%time1 = OpVariable %_ptr_Function_uint Function
%11 = OpReadClockKHR %uint %uint_3
OpStore %time1 %11
OpReturn
OpFunctionEnd)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("or 64bit unsigned integer"));
}

TEST_F(ValidateMisc, ShaderClockVec2) {
  const std::string spirv = ShaderClockSpriv + R"(
%3 = OpTypeFunction %void
%ulong = OpTypeInt 64 0
%_ptr_Function_ulong = OpTypePointer Function %ulong
%uint = OpTypeInt 32 0
%uint_3 = OpConstant %uint 3
%v2uint = OpTypeVector %ulong 2
%_ptr_Function_v2uint = OpTypePointer Function %v2uint
%main = OpFunction %void None %3
%5 = OpLabel
%time1 = OpVariable %_ptr_Function_v2uint Function
%15 = OpReadClockKHR %v2uint %uint_3
OpStore %time1 %15
OpReturn
OpFunctionEnd)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("vector of two components"));
}

TEST_F(ValidateMisc, ShaderClockInvalidScopeValue) {
  const std::string spirv = ShaderClockSpriv + R"(
%3 = OpTypeFunction %void
%ulong = OpTypeInt 64 0
%uint = OpTypeInt 32 0
%_ptr_Function_ulong = OpTypePointer Function %ulong
%uint_10 = OpConstant %uint 10
%uint_1 = OpConstant %uint 1
%main = OpFunction %void None %3
%5 = OpLabel
%time1 = OpVariable %_ptr_Function_ulong Function
%11 = OpReadClockKHR %ulong %uint_10
OpStore %time1 %11
OpReturn
OpFunctionEnd)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("Invalid scope value"));
}

TEST_F(ValidateMisc, ShaderClockSubgroupScope) {
  const std::string spirv = ShaderClockSpriv + R"(
%3 = OpTypeFunction %void
%ulong = OpTypeInt 64 0
%uint = OpTypeInt 32 0
%_ptr_Function_ulong = OpTypePointer Function %ulong
%subgroup = OpConstant %uint 3
%uint_1 = OpConstant %uint 1
%main = OpFunction %void None %3
%5 = OpLabel
%time1 = OpVariable %_ptr_Function_ulong Function
%11 = OpReadClockKHR %ulong %subgroup
OpStore %time1 %11
OpReturn
OpFunctionEnd)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMisc, ShaderClockDeviceScope) {
  const std::string spirv = ShaderClockSpriv + R"(
%3 = OpTypeFunction %void
%ulong = OpTypeInt 64 0
%uint = OpTypeInt 32 0
%_ptr_Function_ulong = OpTypePointer Function %ulong
%device = OpConstant %uint 1
%uint_1 = OpConstant %uint 1
%main = OpFunction %void None %3
%5 = OpLabel
%time1 = OpVariable %_ptr_Function_ulong Function
%11 = OpReadClockKHR %ulong %device
OpStore %time1 %11
OpReturn
OpFunctionEnd)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMisc, ShaderClockWorkgroupScope) {
  const std::string spirv = ShaderClockSpriv + R"(
%3 = OpTypeFunction %void
%ulong = OpTypeInt 64 0
%uint = OpTypeInt 32 0
%_ptr_Function_ulong = OpTypePointer Function %ulong
%workgroup = OpConstant %uint 2
%uint_1 = OpConstant %uint 1
%main = OpFunction %void None %3
%5 = OpLabel
%time1 = OpVariable %_ptr_Function_ulong Function
%11 = OpReadClockKHR %ulong %workgroup
OpStore %time1 %11
OpReturn
OpFunctionEnd)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Scope must be Subgroup or Device"));
}
}  // namespace
}  // namespace val
}  // namespace spvtools
