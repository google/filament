// Copyright (c) 2020 Google Inc.
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

using ValidateSpvExpectAssumeKHR = spvtest::ValidateBase<bool>;

TEST_F(ValidateSpvExpectAssumeKHR, Valid) {
  const std::string str = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability Linkage
    OpCapability ExpectAssumeKHR
    OpExtension "SPV_KHR_expect_assume"
    OpMemoryModel Physical32 OpenCL

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %bool = OpTypeBool
    %true = OpConstantTrue %bool
    %undef = OpUndef %bool

    %uint = OpTypeInt 32 0
    %uint_1 = OpConstant %uint 1
    %uint_2 = OpConstant %uint 2

    %v2bool = OpTypeVector %bool 2
    %v2uint = OpTypeVector %uint 2

    %null_v2bool = OpConstantNull %v2bool
    %null_v2uint = OpConstantNull %v2uint

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpAssumeTrueKHR %true
    OpAssumeTrueKHR %undef       ; probably undefined behaviour
    %bool_val = OpExpectKHR %bool %true %true
    %uint_val = OpExpectKHR %uint %uint_1 %uint_2 ; a bad expectation
    %v2bool_val = OpExpectKHR %v2bool %null_v2bool %null_v2bool
    %v2uint_val = OpExpectKHR %v2uint %null_v2uint %null_v2uint
    OpReturn
    OpFunctionEnd

)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateSpvExpectAssumeKHR, RequiresExtension) {
  const std::string str = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability Linkage
    OpCapability ExpectAssumeKHR
    OpMemoryModel Physical32 OpenCL

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %bool = OpTypeBool
    %true = OpConstantTrue %bool
    %undef = OpUndef %bool

    %uint = OpTypeInt 32 0
    %uint_1 = OpConstant %uint 1
    %uint_2 = OpConstant %uint 2

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpAssumeTrueKHR %true
    OpAssumeTrueKHR %undef       ; probably undefined behaviour
    %val = OpExpectKHR %uint %uint_1 %uint_2 ; a bad expectation
    OpReturn
    OpFunctionEnd

)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Capability: operand ExpectAssumeKHR(5629) requires "
                        "one of these extensions: SPV_KHR_expect_assume"));
}

TEST_F(ValidateSpvExpectAssumeKHR,
       AssumeTrueKHR_RequiresExpectAssumeCapability) {
  const std::string str = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability Linkage
    OpExtension "SPV_KHR_expect_assume"
    OpMemoryModel Physical32 OpenCL

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %bool = OpTypeBool
    %true = OpConstantTrue %bool
    %undef = OpUndef %bool

    %uint = OpTypeInt 32 0
    %uint_1 = OpConstant %uint 1
    %uint_2 = OpConstant %uint 2

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpAssumeTrueKHR %true
    OpAssumeTrueKHR %undef       ; probably undefined behaviour
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Opcode AssumeTrueKHR requires one of these "
                        "capabilities: ExpectAssumeKHR \n"
                        "  OpAssumeTrueKHR %true\n"));
}

TEST_F(ValidateSpvExpectAssumeKHR, AssumeTrueKHR_OperandMustBeBool) {
  const std::string str = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability Linkage
    OpCapability ExpectAssumeKHR
    OpExtension "SPV_KHR_expect_assume"
    OpMemoryModel Physical32 OpenCL

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %bool = OpTypeBool
    %true = OpConstantTrue %bool
    %undef = OpUndef %bool

    %uint = OpTypeInt 32 0
    %uint_1 = OpConstant %uint 1
    %uint_2 = OpConstant %uint 2

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpAssumeTrueKHR %uint_1 ; bad type
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Value operand of OpAssumeTrueKHR must be a boolean scalar\n"
                "  OpAssumeTrueKHR %uint_1\n"));
}

TEST_F(ValidateSpvExpectAssumeKHR, ExpectKHR_RequiresExpectAssumeCapability) {
  const std::string str = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability Linkage
    OpExtension "SPV_KHR_expect_assume"
    OpMemoryModel Physical32 OpenCL

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %bool = OpTypeBool
    %true = OpConstantTrue %bool
    %undef = OpUndef %bool

    %uint = OpTypeInt 32 0
    %uint_1 = OpConstant %uint 1
    %uint_2 = OpConstant %uint 2

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %val = OpExpectKHR %uint %uint_1 %uint_2 ; a bad expectation
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Opcode ExpectKHR requires one of these capabilities: "
                        "ExpectAssumeKHR \n"
                        "  %11 = OpExpectKHR %uint %uint_1 %uint_2\n"));
}

TEST_F(ValidateSpvExpectAssumeKHR, ExpectKHR_ResultMustBeBoolOrIntScalar) {
  const std::string str = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability Linkage
    OpCapability ExpectAssumeKHR
    OpExtension "SPV_KHR_expect_assume"
    OpMemoryModel Physical32 OpenCL

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %float = OpTypeFloat 32

    %float_0 = OpConstant %float 0

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %val = OpExpectKHR %float %float_0 %float_0
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result of OpExpectKHR must be a scalar or vector of "
                        "integer type or boolean type\n"
                        "  %7 = OpExpectKHR %float %float_0 %float_0\n"));
}

TEST_F(ValidateSpvExpectAssumeKHR, ExpectKHR_Value0MustMatchResultType) {
  const std::string str = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability Linkage
    OpCapability ExpectAssumeKHR
    OpExtension "SPV_KHR_expect_assume"
    OpMemoryModel Physical32 OpenCL

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %uint = OpTypeInt 32 0
    %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %val = OpExpectKHR %uint %float_0 %float_0
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Type of Value operand of OpExpectKHR does not match "
                        "the result type \n"
                        "  %8 = OpExpectKHR %uint %float_0 %float_0\n"));
}

TEST_F(ValidateSpvExpectAssumeKHR, ExpectKHR_Value1MustMatchResultType) {
  const std::string str = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability Linkage
    OpCapability ExpectAssumeKHR
    OpExtension "SPV_KHR_expect_assume"
    OpMemoryModel Physical32 OpenCL

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %uint = OpTypeInt 32 0
    %uint_0 = OpConstant %uint 0
    %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %val = OpExpectKHR %uint %uint_0 %float_0
    OpReturn
    OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Type of ExpectedValue operand of OpExpectKHR does not "
                        "match the result type \n"
                        "  %9 = OpExpectKHR %uint %uint_0 %float_0\n"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
