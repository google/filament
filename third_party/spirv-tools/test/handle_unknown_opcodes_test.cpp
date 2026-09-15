// Copyright (c) 2026 NVIDIA Corporation
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

// Tests for SPV_BINARY_TO_TEXT_OPTION_HANDLE_UNKNOWN_OPCODES. Verifies that
// the binary parser, disassembler, and FriendlyNameMapper correctly handle
// SPIR-V binaries that contain unknown opcodes, unknown extended instruction
// numbers, or known opcodes with unknown enum operands.

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "source/binary.h"
#include "source/disassemble.h"
#include "source/name_mapper.h"
#include "spirv-tools/libspirv.h"
#include "test/unit_spirv.h"

namespace spvtools {
namespace {

using ::testing::HasSubstr;
using ::testing::Not;

// Opcode 0xFFFE is not assigned in any known SPIR-V grammar version.
constexpr uint32_t kUnknownOpcode = 0xFFFEu;

// Returns a SPIR-V module header for a module with the given ID bound.
std::vector<uint32_t> MakeHeader(uint32_t id_bound) {
  return {spv::MagicNumber, 0x00010000u,
          SPV_GENERATOR_WORD(SPV_GENERATOR_KHRONOS_ASSEMBLER, 0), id_bound, 0u};
}

// Returns the packed first word of a SPIR-V instruction.
uint32_t MakeFirstWord(uint32_t word_count, uint32_t opcode) {
  return (word_count << 16) | (opcode & 0xFFFFu);
}

class HandleUnknownOpcodesTest : public ::testing::Test {
 protected:
  void SetUp() override {
    context_ = spvContextCreate(SPV_ENV_UNIVERSAL_1_0);
    ASSERT_NE(nullptr, context_);
  }

  void TearDown() override { spvContextDestroy(context_); }

  spv_context context_ = nullptr;
};

// Binary parser: unknown opcode without the flag -> error.
TEST_F(HandleUnknownOpcodesTest, ParseUnknownOpcodeWithoutFlagFails) {
  std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(1),
      {MakeFirstWord(1, kUnknownOpcode)},
  });
  spv_diagnostic diag = nullptr;
  const spv_result_t result =
      spvBinaryParseWithOptions(context_, nullptr, binary.data(), binary.size(),
                                nullptr, nullptr, &diag, 0u);
  EXPECT_NE(SPV_SUCCESS, result);
  spvDiagnosticDestroy(diag);
}

// Binary parser: unknown opcode with the flag -> success.
TEST_F(HandleUnknownOpcodesTest, ParseUnknownOpcodeWithFlagSucceeds) {
  std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(1),
      {MakeFirstWord(3, kUnknownOpcode), 42u, 99u},
  });
  spv_diagnostic diag = nullptr;
  const spv_result_t result = spvBinaryParseWithOptions(
      context_, nullptr, binary.data(), binary.size(), nullptr, nullptr, &diag,
      SPV_BINARY_TO_TEXT_OPTION_HANDLE_UNKNOWN_OPCODES);
  EXPECT_EQ(SPV_SUCCESS, result);
  spvDiagnosticDestroy(diag);
}

// Disassembler: unknown opcode without the flag -> error.
TEST_F(HandleUnknownOpcodesTest, DisassembleUnknownOpcodeWithoutFlagFails) {
  std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(1),
      {MakeFirstWord(3, kUnknownOpcode), 42u, 99u},
  });
  spv_text text = nullptr;
  spv_diagnostic diag = nullptr;
  const spv_result_t result =
      spvBinaryToText(context_, binary.data(), binary.size(), 0u, &text, &diag);
  EXPECT_NE(SPV_SUCCESS, result);
  spvTextDestroy(text);
  spvDiagnosticDestroy(diag);
}

// Disassembler: unknown opcode with the flag -> emits OpUnknown.
TEST_F(HandleUnknownOpcodesTest,
       DisassembleUnknownOpcodeWithFlagEmitsOpUnknown) {
  // 3-word instruction: [opcode+wc, 42, 99]
  std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(1),
      {MakeFirstWord(3, kUnknownOpcode), 42u, 99u},
  });
  spv_text text = nullptr;
  spv_diagnostic diag = nullptr;
  const spv_result_t result = spvBinaryToText(
      context_, binary.data(), binary.size(),
      SPV_BINARY_TO_TEXT_OPTION_HANDLE_UNKNOWN_OPCODES, &text, &diag);
  ASSERT_EQ(SPV_SUCCESS, result) << (diag ? diag->error : "(no diagnostic)");
  const std::string output(text->str, text->length);
  // kUnknownOpcode=0xFFFE=65534, word_count=3, operands=42 99
  EXPECT_THAT(output, HasSubstr("OpUnknown(65534, 3) 42 99"));
  EXPECT_THAT(output,
              HasSubstr("; note: ID bound may be incorrect after reassembly"));
  spvTextDestroy(text);
  spvDiagnosticDestroy(diag);
}

// Disassembler: known opcode with the flag -> normal output (flag is a no-op
// for known opcodes).
TEST_F(HandleUnknownOpcodesTest, DisassembleKnownOpcodeWithFlagIsNoOp) {
  // OpCapability Shader: opcode=17, word_count=2, operand=1 (Shader)
  std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(1),
      {MakeFirstWord(2, 17u), 1u},
  });
  spv_text text = nullptr;
  spv_diagnostic diag = nullptr;
  const spv_result_t result = spvBinaryToText(
      context_, binary.data(), binary.size(),
      SPV_BINARY_TO_TEXT_OPTION_HANDLE_UNKNOWN_OPCODES, &text, &diag);
  ASSERT_EQ(SPV_SUCCESS, result) << (diag ? diag->error : "(no diagnostic)");
  const std::string output(text->str, text->length);
  EXPECT_THAT(output, HasSubstr("OpCapability Shader"));
  EXPECT_THAT(output, Not(HasSubstr("OpUnknown")));
  spvTextDestroy(text);
  spvDiagnosticDestroy(diag);
}

// Disassembler: OpExtInst with unknown instruction number in a semantic set,
// without the flag -> error.
TEST_F(HandleUnknownOpcodesTest, DisassembleUnknownExtInstWithoutFlagFails) {
  // OpExtInstImport %1 "GLSL.std.450" (6 words):
  //   opcode=11 word_count=6, result_id=1, "GLSL.std.450\0" (4 words)
  // OpExtInst %2 %3 %1 0xFFFF (5 words):
  //   opcode=12 word_count=5, result_type=2, result_id=3, set_id=1,
  //   inst_number=0xFFFF
  std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(4),
      {0x0006000Bu, 1u, 0x4C534C47u, 0x6474732Eu, 0x3035342Eu, 0x00000000u},
      {MakeFirstWord(5, 12u), 2u, 3u, 1u, 0xFFFFu},
  });
  spv_text text = nullptr;
  spv_diagnostic diag = nullptr;
  const spv_result_t result =
      spvBinaryToText(context_, binary.data(), binary.size(), 0u, &text, &diag);
  EXPECT_NE(SPV_SUCCESS, result);
  spvTextDestroy(text);
  spvDiagnosticDestroy(diag);
}

// Disassembler: OpExtInst with unknown instruction number in a semantic set,
// with the flag -> emits OpUnknown with all instruction words.
TEST_F(HandleUnknownOpcodesTest,
       DisassembleUnknownExtInstWithFlagEmitsOpUnknown) {
  // See DisassembleUnknownExtInstWithoutFlagFails for binary layout.
  std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(4),
      {0x0006000Bu, 1u, 0x4C534C47u, 0x6474732Eu, 0x3035342Eu, 0x00000000u},
      {MakeFirstWord(5, 12u), 2u, 3u, 1u, 0xFFFFu},
  });
  spv_text text = nullptr;
  spv_diagnostic diag = nullptr;
  const spv_result_t result = spvBinaryToText(
      context_, binary.data(), binary.size(),
      SPV_BINARY_TO_TEXT_OPTION_HANDLE_UNKNOWN_OPCODES, &text, &diag);
  ASSERT_EQ(SPV_SUCCESS, result) << (diag ? diag->error : "(no diagnostic)");
  const std::string output(text->str, text->length);
  // OpExtInst opcode=12, word_count=5; operands: result_type=2, result_id=3,
  // set_id=1, inst_number=0xFFFF=65535.
  EXPECT_THAT(output, HasSubstr("OpUnknown(12, 5) 2 3 1 65535"));
  spvTextDestroy(text);
  spvDiagnosticDestroy(diag);
}

// Disassembler: OpExtInst with an unknown instruction number in a non-semantic
// extended instruction set disassembles normally (not as OpUnknown) both with
// and without the flag. Non-semantic sets handle unknown instruction numbers
// gracefully regardless of the flag; setting the flag must not change that.
TEST_F(HandleUnknownOpcodesTest,
       NonSemanticExtInstUnknownNumberNotEmittedAsOpUnknown) {
  // OpExtInstImport %1 "NonSemantic.DebugPrintf" (8 words):
  //   opcode=11, word_count=8, result_id=1
  //   "NonSemantic.DebugPrintf\0" packed into 6 32-bit words:
  //     "NonS" = 0x536E6F4E, "eman" = 0x6E616D65, "tic." = 0x2E636974,
  //     "Debu" = 0x75626544, "gPri" = 0x69725067, "ntf\0" = 0x0066746E
  // OpExtInst %2 %3 %1 0xFFFF (5 words):
  //   opcode=12, word_count=5, result_type=2, result_id=3, set_id=1,
  //   inst_number=0xFFFF (not present in the NonSemantic.DebugPrintf grammar)
  const std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(4),
      {0x0008000Bu, 1u, 0x536E6F4Eu, 0x6E616D65u, 0x2E636974u, 0x75626544u,
       0x69725067u, 0x0066746Eu},
      {MakeFirstWord(5, 12u), 2u, 3u, 1u, 0xFFFFu},
  });

  // Without the flag: non-semantic sets already handle unknown instruction
  // numbers gracefully; parsing and disassembly succeed.
  {
    spv_text text = nullptr;
    spv_diagnostic diag = nullptr;
    EXPECT_EQ(SPV_SUCCESS, spvBinaryToText(context_, binary.data(),
                                           binary.size(), 0u, &text, &diag))
        << (diag ? diag->error : "(no diagnostic)");
    spvTextDestroy(text);
    spvDiagnosticDestroy(diag);
  }

  // With the flag: the non-semantic graceful path is unchanged.  The
  // instruction must not be emitted as OpUnknown.
  {
    spv_text text = nullptr;
    spv_diagnostic diag = nullptr;
    ASSERT_EQ(SPV_SUCCESS,
              spvBinaryToText(context_, binary.data(), binary.size(),
                              SPV_BINARY_TO_TEXT_OPTION_HANDLE_UNKNOWN_OPCODES,
                              &text, &diag))
        << (diag ? diag->error : "(no diagnostic)");
    const std::string output(text->str, text->length);
    EXPECT_THAT(output, Not(HasSubstr("OpUnknown")));
    spvTextDestroy(text);
    spvDiagnosticDestroy(diag);
  }
}

// FriendlyNameMapper: when a binary contains an unknown opcode, IDs defined
// after the unknown opcode get friendly names only when the flag is set.
TEST_F(HandleUnknownOpcodesTest, FriendlyNameMapperContinuesPastUnknownOpcode) {
  // OpTypeVoid %1    (opcode=19, word_count=2)
  // Unknown opcode 0xFFFE  (word_count=1)
  // OpTypeBool %2    (opcode=20, word_count=2)
  std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(3),
      {MakeFirstWord(2, 19u), 1u},
      {MakeFirstWord(1, kUnknownOpcode)},
      {MakeFirstWord(2, 20u), 2u},
  });

  // With the flag, parsing continues past the unknown opcode so %2 is named.
  FriendlyNameMapper mapper_with_flag(
      context_, binary.data(), binary.size(),
      SPV_BINARY_TO_TEXT_OPTION_HANDLE_UNKNOWN_OPCODES);
  EXPECT_EQ("void", mapper_with_flag.NameForId(1));
  EXPECT_EQ("bool", mapper_with_flag.NameForId(2));

  // Without the flag, parsing stops at the unknown opcode so %2 falls back to
  // its trivial numeric name.
  FriendlyNameMapper mapper_without_flag(context_, binary.data(),
                                         binary.size());
  EXPECT_EQ("void", mapper_without_flag.NameForId(1));
  EXPECT_EQ("2", mapper_without_flag.NameForId(2));
}

// Binary parser: unknown opcode that claims more words than remain in the
// binary -> error, even with the flag set.
TEST_F(HandleUnknownOpcodesTest, ParseTruncatedUnknownOpcodeFails) {
  // Instruction claims 3 words but only 1 is present.
  std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(1),
      {MakeFirstWord(3, kUnknownOpcode)},
  });
  spv_diagnostic diag = nullptr;
  const spv_result_t result = spvBinaryParseWithOptions(
      context_, nullptr, binary.data(), binary.size(), nullptr, nullptr, &diag,
      SPV_BINARY_TO_TEXT_OPTION_HANDLE_UNKNOWN_OPCODES);
  EXPECT_NE(SPV_SUCCESS, result);
  spvDiagnosticDestroy(diag);
}

// Disassembler: known opcode with an unknown flat enum operand, without the
// flag -> error.
TEST_F(HandleUnknownOpcodesTest, DisassembleUnknownFlatEnumWithoutFlagFails) {
  // OpCapability 0xFFFF: opcode=17, word_count=2, capability=0xFFFF (unknown).
  std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(1),
      {MakeFirstWord(2, 17u), 0xFFFFu},
  });
  spv_text text = nullptr;
  spv_diagnostic diag = nullptr;
  const spv_result_t result =
      spvBinaryToText(context_, binary.data(), binary.size(), 0u, &text, &diag);
  EXPECT_NE(SPV_SUCCESS, result);
  spvTextDestroy(text);
  spvDiagnosticDestroy(diag);
}

// Disassembler: known opcode with an unknown flat enum operand, with the flag
// -> emits the whole instruction as OpUnknown.
TEST_F(HandleUnknownOpcodesTest,
       DisassembleUnknownFlatEnumWithFlagEmitsOpUnknown) {
  // OpCapability 0xFFFF: opcode=17, word_count=2, capability=0xFFFF (unknown).
  std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(1),
      {MakeFirstWord(2, 17u), 0xFFFFu},
  });
  spv_text text = nullptr;
  spv_diagnostic diag = nullptr;
  const spv_result_t result = spvBinaryToText(
      context_, binary.data(), binary.size(),
      SPV_BINARY_TO_TEXT_OPTION_HANDLE_UNKNOWN_OPCODES, &text, &diag);
  ASSERT_EQ(SPV_SUCCESS, result) << (diag ? diag->error : "(no diagnostic)");
  const std::string output(text->str, text->length);
  // OpCapability opcode=17, word_count=2; operand=0xFFFF=65535.
  EXPECT_THAT(output, HasSubstr("OpUnknown(17, 2) 65535"));
  spvTextDestroy(text);
  spvDiagnosticDestroy(diag);
}

// Disassembler: known opcode with an unknown mask enum operand, without the
// flag -> error.
TEST_F(HandleUnknownOpcodesTest, DisassembleUnknownMaskEnumWithoutFlagFails) {
  // OpFunction %1 %2 FunctionControl(0x80000000) %3:
  //   opcode=54, word_count=5, result_type=1, result_id=2,
  //   function_control=0x80000000 (unknown bit), function_type=3.
  std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(4),
      {MakeFirstWord(5, 54u), 1u, 2u, 0x80000000u, 3u},
  });
  spv_text text = nullptr;
  spv_diagnostic diag = nullptr;
  const spv_result_t result =
      spvBinaryToText(context_, binary.data(), binary.size(), 0u, &text, &diag);
  EXPECT_NE(SPV_SUCCESS, result);
  spvTextDestroy(text);
  spvDiagnosticDestroy(diag);
}

// Disassembler: known opcode with an unknown mask enum operand, with the flag
// -> emits the whole instruction as OpUnknown.
TEST_F(HandleUnknownOpcodesTest,
       DisassembleUnknownMaskEnumWithFlagEmitsOpUnknown) {
  // OpFunction %1 %2 FunctionControl(0x80000000) %3:
  //   opcode=54, word_count=5, result_type=1, result_id=2,
  //   function_control=0x80000000 (unknown bit), function_type=3.
  std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(4),
      {MakeFirstWord(5, 54u), 1u, 2u, 0x80000000u, 3u},
  });
  spv_text text = nullptr;
  spv_diagnostic diag = nullptr;
  const spv_result_t result = spvBinaryToText(
      context_, binary.data(), binary.size(),
      SPV_BINARY_TO_TEXT_OPTION_HANDLE_UNKNOWN_OPCODES, &text, &diag);
  ASSERT_EQ(SPV_SUCCESS, result) << (diag ? diag->error : "(no diagnostic)");
  const std::string output(text->str, text->length);
  // OpFunction opcode=54, word_count=5; operands: 1, 2, 2147483648, 3.
  EXPECT_THAT(output, HasSubstr("OpUnknown(54, 5) 1 2 2147483648 3"));
  spvTextDestroy(text);
  spvDiagnosticDestroy(diag);
}

// Round-trip: disassemble with the flag, reassemble, and verify the
// instruction words are preserved byte-for-byte.
TEST_F(HandleUnknownOpcodesTest, RoundTripUnknownOpcode) {
  // 3-word instruction: [opcode+wc, 42, 99]
  const std::vector<uint32_t> inst_words = {MakeFirstWord(3, kUnknownOpcode),
                                            42u, 99u};
  std::vector<uint32_t> original =
      spvtest::Concatenate({MakeHeader(1), inst_words});

  // Disassemble to text with the flag.
  spv_text text = nullptr;
  spv_diagnostic dis_diag = nullptr;
  ASSERT_EQ(SPV_SUCCESS,
            spvBinaryToText(context_, original.data(), original.size(),
                            SPV_BINARY_TO_TEXT_OPTION_HANDLE_UNKNOWN_OPCODES,
                            &text, &dis_diag))
      << (dis_diag ? dis_diag->error : "(no diagnostic)");

  // Reassemble the text back to binary.
  spv_binary reassembled = nullptr;
  spv_diagnostic asm_diag = nullptr;
  ASSERT_EQ(SPV_SUCCESS, spvTextToBinary(context_, text->str, text->length,
                                         &reassembled, &asm_diag))
      << (asm_diag ? asm_diag->error : "(no diagnostic)");

  // The instruction words start after the 5-word SPIR-V module header and
  // must be byte-for-byte identical to the original instruction.
  ASSERT_GE(reassembled->wordCount, 5u + inst_words.size());
  for (size_t i = 0; i < inst_words.size(); i++) {
    EXPECT_EQ(inst_words[i], reassembled->code[5 + i])
        << "Word mismatch at instruction word " << i;
  }

  spvTextDestroy(text);
  spvBinaryDestroy(reassembled);
  spvDiagnosticDestroy(dis_diag);
  spvDiagnosticDestroy(asm_diag);
}

// FriendlyNameMapper: when a known instruction has an unknown enum operand and
// the flag is set, the instruction is retried as unknown.  inst.result_id is
// not decoded (stays 0) so the real result ID falls back to its trivial numeric
// name.  IDs defined by preceding and following instructions are still named.
TEST_F(HandleUnknownOpcodesTest,
       FriendlyNameMapperUnknownEnumOperandDropsResultName) {
  // OpTypeVoid %1        (opcode=19, word_count=2)
  // OpCapability 0xFFFF  (opcode=17, word_count=2; unknown Capability value)
  // OpTypeBool %2        (opcode=20, word_count=2)
  std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(3),
      {MakeFirstWord(2, 19u), 1u},
      {MakeFirstWord(2, 17u), 0xFFFFu},
      {MakeFirstWord(2, 20u), 2u},
  });

  // With the flag, parsing continues past the unknown-enum instruction.
  // %1 and %2 are both named; the OpCapability result (none for that opcode)
  // does not affect naming.
  FriendlyNameMapper mapper_with_flag(
      context_, binary.data(), binary.size(),
      SPV_BINARY_TO_TEXT_OPTION_HANDLE_UNKNOWN_OPCODES);
  EXPECT_EQ("void", mapper_with_flag.NameForId(1));
  EXPECT_EQ("bool", mapper_with_flag.NameForId(2));

  // Without the flag, parsing stops at OpCapability 0xFFFF so %2 is unnamed.
  FriendlyNameMapper mapper_without_flag(context_, binary.data(),
                                         binary.size());
  EXPECT_EQ("void", mapper_without_flag.NameForId(1));
  EXPECT_EQ("2", mapper_without_flag.NameForId(2));
}

// Disassembler: a binary with a valid instruction before and after an unknown
// opcode disassembles all three instructions correctly.
TEST_F(HandleUnknownOpcodesTest, DisassembleContinuesPastUnknownOpcode) {
  // OpTypeVoid %1 (opcode=19, word_count=2)
  // Unknown opcode 0xFFFE (word_count=1)
  // OpTypeBool %2 (opcode=20, word_count=2)
  std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(3),
      {MakeFirstWord(2, 19u), 1u},
      {MakeFirstWord(1, kUnknownOpcode)},
      {MakeFirstWord(2, 20u), 2u},
  });
  spv_text text = nullptr;
  spv_diagnostic diag = nullptr;
  ASSERT_EQ(SPV_SUCCESS,
            spvBinaryToText(context_, binary.data(), binary.size(),
                            SPV_BINARY_TO_TEXT_OPTION_HANDLE_UNKNOWN_OPCODES,
                            &text, &diag))
      << (diag ? diag->error : "(no diagnostic)");
  const std::string output(text->str, text->length);
  EXPECT_THAT(output, HasSubstr("OpTypeVoid"));
  EXPECT_THAT(output, HasSubstr("OpUnknown(65534, 1)"));
  EXPECT_THAT(output, HasSubstr("OpTypeBool"));
  spvTextDestroy(text);
  spvDiagnosticDestroy(diag);
}

// Disassembler: FRIENDLY_NAMES and HANDLE_UNKNOWN_OPCODES work together.
// Known instructions use friendly names; the unknown opcode emits OpUnknown.
TEST_F(HandleUnknownOpcodesTest, DisassembleFriendlyNamesWithUnknownOpcode) {
  // OpTypeVoid %1 (opcode=19, word_count=2)
  // Unknown opcode 0xFFFE (word_count=1)
  // OpTypeBool %2 (opcode=20, word_count=2)
  std::vector<uint32_t> binary = spvtest::Concatenate({
      MakeHeader(3),
      {MakeFirstWord(2, 19u), 1u},
      {MakeFirstWord(1, kUnknownOpcode)},
      {MakeFirstWord(2, 20u), 2u},
  });
  spv_text text = nullptr;
  spv_diagnostic diag = nullptr;
  ASSERT_EQ(SPV_SUCCESS,
            spvBinaryToText(context_, binary.data(), binary.size(),
                            SPV_BINARY_TO_TEXT_OPTION_HANDLE_UNKNOWN_OPCODES |
                                SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES,
                            &text, &diag))
      << (diag ? diag->error : "(no diagnostic)");
  const std::string output(text->str, text->length);
  EXPECT_THAT(output, HasSubstr("%void"));
  EXPECT_THAT(output, HasSubstr("%bool"));
  EXPECT_THAT(output, HasSubstr("OpUnknown(65534, 1)"));
  spvTextDestroy(text);
  spvDiagnosticDestroy(diag);
}

// spvInstructionBinaryToText: unknown opcode with the flag emits OpUnknown.
TEST_F(HandleUnknownOpcodesTest, InstructionBinaryToTextUnknownOpcode) {
  const std::vector<uint32_t> inst_words = {MakeFirstWord(3, kUnknownOpcode),
                                            42u, 99u};
  std::vector<uint32_t> binary =
      spvtest::Concatenate({MakeHeader(1), inst_words});
  const std::string output = spvInstructionBinaryToText(
      SPV_ENV_UNIVERSAL_1_0, inst_words.data(), inst_words.size(),
      binary.data(), binary.size(),
      SPV_BINARY_TO_TEXT_OPTION_HANDLE_UNKNOWN_OPCODES);
  EXPECT_THAT(output, HasSubstr("OpUnknown(65534, 3) 42 99"));
}

}  // namespace
}  // namespace spvtools
