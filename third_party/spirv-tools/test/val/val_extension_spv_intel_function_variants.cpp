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

using ValidateSpvINTELFunctionVariants = spvtest::ValidateBase<bool>;

TEST_F(ValidateSpvINTELFunctionVariants, Valid) {
  const std::string str = R"(
         OpCapability Kernel
         OpCapability Linkage
         OpCapability SpecConditionalINTEL
         OpCapability FunctionVariantsINTEL
         OpConditionalCapabilityINTEL %1 Int8
         OpExtension "SPV_INTEL_function_variants"
         OpMemoryModel Logical OpenCL
 %bool = OpTypeBool
    %1 = OpSpecConstantTrue %bool
  )";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateSpvINTELFunctionVariants, RequiresExtension1) {
  const std::string str = R"(
         OpCapability Kernel
         OpCapability Linkage
         OpCapability SpecConditionalINTEL
         OpMemoryModel Logical OpenCL
  )";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(
      diag, HasSubstr("1st operand of Capability: operand "
                      "SpecConditionalINTEL(6245) requires "
                      "one of these extensions: SPV_INTEL_function_variants"));
  EXPECT_THAT(diag, HasSubstr("OpCapability SpecConditionalINTEL"));
}

TEST_F(ValidateSpvINTELFunctionVariants, RequiresExtension2) {
  const std::string str = R"(
         OpCapability Kernel
         OpCapability Linkage
         OpCapability FunctionVariantsINTEL
         OpMemoryModel Logical OpenCL
  )";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(
      diag, HasSubstr("1st operand of Capability: operand "
                      "FunctionVariantsINTEL(6246) requires "
                      "one of these extensions: SPV_INTEL_function_variants"));
  EXPECT_THAT(diag, HasSubstr("OpCapability FunctionVariantsINTEL"));
}

TEST_F(ValidateSpvINTELFunctionVariants, RequiresCapability1) {
  const std::string str = R"(
         OpCapability Kernel
         OpCapability Linkage
         OpConditionalCapabilityINTEL %1 Int8
         OpExtension "SPV_INTEL_function_variants"
         OpMemoryModel Logical OpenCL
 %bool = OpTypeBool
    %1 = OpSpecConstantTrue %bool
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(
      diag, HasSubstr("Opcode ConditionalCapabilityINTEL requires one of these "
                      "capabilities: SpecConditionalINTEL"));
  EXPECT_THAT(diag, HasSubstr("OpConditionalCapabilityINTEL %1 Int8"));
}

TEST_F(ValidateSpvINTELFunctionVariants, RequiresCapability2) {
  const std::string str = R"(
         OpCapability Kernel
         OpCapability Linkage
         OpCapability SpecConditionalINTEL
         OpConditionalCapabilityINTEL %1 Int8
         OpExtension "SPV_INTEL_function_variants"
         OpMemoryModel Logical OpenCL
 %bool = OpTypeBool
    %1 = OpSpecConstantArchitectureINTEL %bool 0 0 170 0
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  const std::string diag = getDiagnosticString();
  EXPECT_THAT(
      diag,
      HasSubstr("Opcode SpecConstantArchitectureINTEL requires one of these "
                "capabilities: FunctionVariantsINTEL"));
  EXPECT_THAT(
      diag, HasSubstr("%1 = OpSpecConstantArchitectureINTEL %bool 0 0 170 0"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
