// Copyright 2025 The Khronos Group Inc.
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

// Tests for SPV_INTEL_inline_assembly

#include <string>

#include "gmock/gmock.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;

using ValidateSpvINTELInlineAssembly = spvtest::ValidateBase<bool>;

TEST_F(ValidateSpvINTELInlineAssembly, Valid) {
  const std::string str = R"(
         OpCapability Kernel
         OpCapability Addresses
         OpCapability Linkage
         OpCapability AsmINTEL
         OpExtension "SPV_INTEL_inline_assembly"
         OpMemoryModel Physical32 OpenCL
         OpDecorate %1 SideEffectsINTEL
    %2 = OpTypeVoid
    %3 = OpTypeFunction %2
    %4 = OpAsmTargetINTEL "spirv64-unknown-unknown"
    %1 = OpAsmINTEL %2 %3 %4 "nop" ""
    %5 = OpFunction %2 None %3
    %6 = OpLabel
    %7 = OpAsmCallINTEL %2 %1
         OpReturn
         OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateSpvINTELInlineAssembly, RequiresExtension) {
  const std::string str = R"(
         OpCapability Kernel
         OpCapability Addresses
         OpCapability Linkage
         OpCapability AsmINTEL
         OpMemoryModel Physical32 OpenCL
         OpDecorate %1 SideEffectsINTEL
    %2 = OpTypeVoid
    %3 = OpTypeFunction %2
    %4 = OpAsmTargetINTEL "spirv64-unknown-unknown"
    %1 = OpAsmINTEL %2 %3 %4 "nop" ""
    %5 = OpFunction %2 None %3
    %6 = OpLabel
    %7 = OpAsmCallINTEL %2 %1
         OpReturn
         OpFunctionEnd
  )";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(
      diag,
      HasSubstr("1st operand of Capability: operand AsmINTEL(5606) requires "
                "one of these extensions: SPV_INTEL_inline_assembly"));
  EXPECT_THAT(diag, HasSubstr("OpCapability AsmINTEL"));
}

TEST_F(ValidateSpvINTELInlineAssembly, RequiresCapability) {
  const std::string str = R"(
         OpCapability Kernel
         OpCapability Addresses
         OpCapability Linkage
         OpExtension "SPV_INTEL_inline_assembly"
         OpMemoryModel Physical32 OpenCL
         OpDecorate %1 SideEffectsINTEL
    %2 = OpTypeVoid
    %3 = OpTypeFunction %2
    %4 = OpAsmTargetINTEL "spirv64-unknown-unknown"
    %1 = OpAsmINTEL %2 %3 %4 "nop" ""
    %5 = OpFunction %2 None %3
    %6 = OpLabel
    %7 = OpAsmCallINTEL %2 %1
         OpReturn
         OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(diag, HasSubstr("Operand 2 of Decorate requires one of these "
                              "capabilities: AsmINTEL"));
  EXPECT_THAT(diag, HasSubstr("OpDecorate %1 SideEffectsINTEL"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
