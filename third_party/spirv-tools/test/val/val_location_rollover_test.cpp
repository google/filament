// Copyright (c) 2026 Google Inc.
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

// Tests location integer rollover is correctly handles

#include <string>

#include "gmock/gmock.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;

using ValidateLocationRollover = spvtest::ValidateBase<bool>;

TEST_F(ValidateLocationRollover, VulkanLocationRollover) {
  std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "func" %var1 %var2
OpExecutionMode %1 OriginUpperLeft
OpDecorate %var1 Location 0
OpDecorate %var2 Location 1073741824
%2 = OpTypeVoid
%3 = OpTypeInt 32 0
%4 = OpTypePointer Output %3
%var1 = OpVariable %4 Output
%var2 = OpVariable %4 Output
%6 = OpTypeFunction %2
%1 = OpFunction %2 None %6
%7 = OpLabel
OpReturn
OpFunctionEnd
)";

  // 1073741824 * 4 = 4294967296 (2^32), which overflows to 0 in 32-bit.
  // This would cause it to be treated as Location 0 and conflict with %var1.

  CompileSuccessfully(text, SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2))
      << getDiagnosticString();
}

TEST_F(ValidateLocationRollover, VulkanLocationRolloverStructMember) {
  std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "func" %var
OpExecutionMode %1 OriginUpperLeft
OpMemberDecorate %struct 0 Location 0
OpMemberDecorate %struct 1 Location 1073741824
OpDecorate %struct Block
%2 = OpTypeVoid
%3 = OpTypeInt 32 0
%struct = OpTypeStruct %3 %3
%4 = OpTypePointer Output %struct
%var = OpVariable %4 Output
%6 = OpTypeFunction %2
%1 = OpFunction %2 None %6
%7 = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2))
      << getDiagnosticString();
}

TEST_F(ValidateLocationRollover, VulkanLocationRolloverStructMemberStrided) {
  std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "func" %var
OpExecutionMode %1 OriginUpperLeft
OpMemberDecorate %struct 0 Location 0
OpMemberDecorate %struct 1 Location 1073741824
OpDecorate %struct Block
%void = OpTypeVoid
%u32 = OpTypeInt 32 0
%u32_1 = OpConstant %u32 1
%u32_2 = OpTypeVector %u32 2
%array = OpTypeArray %u32_2 %u32_1
%struct = OpTypeStruct %u32 %array
%ptr = OpTypePointer Output %struct
%var = OpVariable %ptr Output
%func_type = OpTypeFunction %void
%1 = OpFunction %void None %func_type
%label = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2))
      << getDiagnosticString();
}

}  // namespace
}  // namespace val
}  // namespace spvtools
