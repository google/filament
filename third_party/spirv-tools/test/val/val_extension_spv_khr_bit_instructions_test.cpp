// Copyright (c) 2021 The Khronos Group Inc.
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

using ValidateSpvKHRBitInstructions = spvtest::ValidateBase<bool>;

TEST_F(ValidateSpvKHRBitInstructions, Valid) {
  const std::string str = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability BitInstructions
    OpExtension "SPV_KHR_bit_instructions"
    OpMemoryModel Physical32 OpenCL
    OpEntryPoint Kernel %main "main"
    
    %void    = OpTypeVoid
    %void_fn = OpTypeFunction %void
    %u32 = OpTypeInt 32 0
    %u32_1 = OpConstant %u32 1

    %main = OpFunction %void None %void_fn
    %entry = OpLabel
    %unused = OpBitReverse %u32 %u32_1
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateSpvKHRBitInstructions, RequiresExtension) {
  const std::string str = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability BitInstructions
    OpMemoryModel Physical32 OpenCL
    OpEntryPoint Kernel %main "main"
    
    %void    = OpTypeVoid
    %void_fn = OpTypeFunction %void
    %u32 = OpTypeInt 32 0
    %u32_1 = OpConstant %u32 1

    %main = OpFunction %void None %void_fn
    %entry = OpLabel
    %unused = OpBitReverse %u32 %u32_1
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("1st operand of Capability: operand BitInstructions(6025) "
                "requires one of these extensions: SPV_KHR_bit_instructions"));
}

TEST_F(ValidateSpvKHRBitInstructions, RequiresCapability) {
  const std::string str = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpExtension "SPV_KHR_bit_instructions"
    OpMemoryModel Physical32 OpenCL
    OpEntryPoint Kernel %main "main"
    
    %void    = OpTypeVoid
    %void_fn = OpTypeFunction %void
    %u32 = OpTypeInt 32 0
    %u32_1 = OpConstant %u32 1

    %main = OpFunction %void None %void_fn
    %entry = OpLabel
    %unused = OpBitReverse %u32 %u32_1
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Opcode BitReverse requires one of these capabilities: "
                        "Shader BitInstructions"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
