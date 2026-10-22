// Copyright (c) 2025 The Khronos Group Inc.
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

#include <cassert>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "source/util/bitutils.h"
#include "test/test_fixture.h"

namespace spvtools {
namespace utils {
namespace {

using spvtest::Concatenate;
using spvtest::MakeInstruction;
using spvtest::ScopedContext;
using spvtest::TextToBinaryTest;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::StrEq;

using OpUnknownTest = TextToBinaryTest;

TEST_F(OpUnknownTest, OpUnknown) {
  SetText("OpUnknown(255, 1)");
  ASSERT_EQ(SPV_SUCCESS, spvTextToBinary(ScopedContext().context, text.str,
                                         text.length, &binary, &diagnostic));
  EXPECT_EQ(0x000100FFu, binary->code[5]);
  if (diagnostic) {
    spvDiagnosticPrint(diagnostic);
  }
}

TEST_F(OpUnknownTest, HandlesOperands) {
  EXPECT_THAT(CompiledInstructions("OpUnknown(24, 4) %a %b %123"),
              Eq(MakeInstruction(spv::Op::OpTypeMatrix, {1, 2, 3})));
  EXPECT_THAT(CompiledInstructions("OpUnknown(24, 4) !1 %b %123"),
              Eq(MakeInstruction(spv::Op::OpTypeMatrix, {1, 1, 2})));
}

TEST_F(OpUnknownTest, HandlesWhitespace) {
  EXPECT_THAT(CompiledInstructions("OpUnknown ( 24 , 4 ) %a %b %123"),
              Eq(MakeInstruction(spv::Op::OpTypeMatrix, {1, 2, 3})));
  EXPECT_THAT(CompiledInstructions("OpUnknown(24,4) %a %b %123"),
              Eq(MakeInstruction(spv::Op::OpTypeMatrix, {1, 2, 3})));
}

TEST_F(OpUnknownTest, MultipleInstructions) {
  EXPECT_THAT(
      CompiledInstructions(
          "%a = OpTypeFunction %b\nOpUnknown(21, 5) %c %d 32 1\nOpNop"),
      Eq(Concatenate({MakeInstruction(spv::Op::OpTypeFunction, {1, 2}),
                      MakeInstruction(spv::Op::OpTypeInt, {3, 4, 32, 1}),
                      MakeInstruction(spv::Op::OpNop, {})})));
}

TEST_F(OpUnknownTest, OpUnknownInAssignment) {
  EXPECT_EQ(
      "OpUnknown not allowed in assignment. Use an explicit result id operand "
      "instead.",
      CompileFailure("%2 = OpUnknown(22, 3) 32"));
  EXPECT_EQ("OpUnknown not allowed before =.",
            CompileFailure("OpUnknown(22, 3) = OpTypeFloat 32"));
}

TEST_F(OpUnknownTest, ParsingErrors) {
  EXPECT_EQ("Expected '(', found end of stream.", CompileFailure("OpUnknown"));
  EXPECT_EQ("'(' expected after OpUnknown but found 'a'.",
            CompileFailure("OpUnknown abc"));

  EXPECT_EQ("Expected opcode enumerant, found end of stream.",
            CompileFailure("OpUnknown("));
  EXPECT_EQ("Invalid opcode enumerant: \"abc\".",
            CompileFailure("OpUnknown(abc"));
  // Opcode enumerant must fit in 16 bits.
  EXPECT_EQ("Invalid opcode enumerant: \"70000\".",
            CompileFailure("OpUnknown(70000"));

  EXPECT_EQ("Expected ',', found end of stream.",
            CompileFailure("OpUnknown(22"));
  EXPECT_EQ("',' expected after opcode enumerant but found 'a'.",
            CompileFailure("OpUnknown(22 abc"));

  EXPECT_EQ("Expected number of words, found end of stream.",
            CompileFailure("OpUnknown(22,"));
  EXPECT_EQ("Invalid number of words: \"abc\".",
            CompileFailure("OpUnknown(22, abc"));
  // Number of words must fit in 16 bits.
  EXPECT_EQ("Invalid number of words: \"70000\".",
            CompileFailure("OpUnknown(22, 70000"));
  EXPECT_EQ(
      "Number of words (which includes the opcode) must be greater than zero.",
      CompileFailure("OpUnknown(22, 0"));

  EXPECT_EQ("Expected ')', found end of stream.",
            CompileFailure("OpUnknown(22, 3"));
  EXPECT_EQ("')' expected after number of words but found 'a'.",
            CompileFailure("OpUnknown(22, 3 abc"));

  EXPECT_EQ(
      "Unexpected start of new instruction: \"OpNop\". Expected 2 more "
      "operands",
      CompileFailure("OpUnknown(22, 3) OpNop"));
  EXPECT_EQ("Expected 2 more operands, found end of stream.",
            CompileFailure("OpUnknown(22, 3)"));

  EXPECT_EQ(CompileFailure("OpUnknown(21, 4) %c %d 32 1"),
            "Expected <opcode> or <result-id> at the beginning of an "
            "instruction, found '1'.");
}

}  // namespace
}  // namespace utils
}  // namespace spvtools
