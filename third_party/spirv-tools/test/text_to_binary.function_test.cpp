// Copyright (c) 2015-2016 The Khronos Group Inc.
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

// Assembler tests for instructions in the "Function" section of the
// SPIR-V spec.

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "test/test_fixture.h"
#include "test/unit_spirv.h"

namespace spvtools {
namespace {

using spvtest::EnumCase;
using spvtest::MakeInstruction;
using spvtest::TextToBinaryTest;
using ::testing::Eq;

// Test OpFunction

using OpFunctionControlTest = spvtest::TextToBinaryTestBase<
    ::testing::TestWithParam<EnumCase<spv::FunctionControlMask>>>;

TEST_P(OpFunctionControlTest, AnySingleFunctionControlMask) {
  const std::string input = "%result_id = OpFunction %result_type " +
                            GetParam().name() + " %function_type ";
  EXPECT_THAT(CompiledInstructions(input),
              Eq(MakeInstruction(spv::Op::OpFunction,
                                 {1, 2, (uint32_t)GetParam().value(), 3})));
}

// clang-format off
#define CASE(VALUE,NAME) { spv::FunctionControlMask::VALUE, NAME }
INSTANTIATE_TEST_SUITE_P(TextToBinaryFunctionTest, OpFunctionControlTest,
                        ::testing::ValuesIn(std::vector<EnumCase<spv::FunctionControlMask>>{
                            CASE(MaskNone, "None"),
                            CASE(Inline, "Inline"),
                            CASE(DontInline, "DontInline"),
                            CASE(Pure, "Pure"),
                            CASE(Const, "Const"),
                        }));
#undef CASE
// clang-format on

TEST_F(OpFunctionControlTest, CombinedFunctionControlMask) {
  // Sample a single combination.  This ensures we've integrated
  // the instruction parsing logic with spvTextParseMask.
  const std::string input =
      "%result_id = OpFunction %result_type Inline|Pure|Const %function_type";
  const uint32_t expected_mask = uint32_t(spv::FunctionControlMask::Inline |
                                          spv::FunctionControlMask::Pure |
                                          spv::FunctionControlMask::Const);
  EXPECT_THAT(
      CompiledInstructions(input),
      Eq(MakeInstruction(spv::Op::OpFunction, {1, 2, expected_mask, 3})));
}

TEST_F(OpFunctionControlTest, WrongFunctionControl) {
  EXPECT_THAT(CompileFailure("%r = OpFunction %t Inline|Unroll %ft"),
              Eq("Invalid function control operand 'Inline|Unroll'."));
}

// TODO(dneto): OpFunctionParameter
// TODO(dneto): OpFunctionEnd
// TODO(dneto): OpFunctionCall

}  // namespace
}  // namespace spvtools
