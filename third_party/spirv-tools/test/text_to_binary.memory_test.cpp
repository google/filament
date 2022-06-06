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

// Assembler tests for instructions in the "Memory Instructions" section of
// the SPIR-V spec.

#include <sstream>
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
using ::testing::HasSubstr;

// Test assembly of Memory Access masks

using MemoryAccessTest = spvtest::TextToBinaryTestBase<
    ::testing::TestWithParam<EnumCase<SpvMemoryAccessMask>>>;

TEST_P(MemoryAccessTest, AnySingleMemoryAccessMask) {
  std::stringstream input;
  input << "OpStore %ptr %value " << GetParam().name();
  for (auto operand : GetParam().operands()) input << " " << operand;
  EXPECT_THAT(CompiledInstructions(input.str()),
              Eq(MakeInstruction(SpvOpStore, {1, 2, GetParam().value()},
                                 GetParam().operands())));
}

INSTANTIATE_TEST_SUITE_P(
    TextToBinaryMemoryAccessTest, MemoryAccessTest,
    ::testing::ValuesIn(std::vector<EnumCase<SpvMemoryAccessMask>>{
        {SpvMemoryAccessMaskNone, "None", {}},
        {SpvMemoryAccessVolatileMask, "Volatile", {}},
        {SpvMemoryAccessAlignedMask, "Aligned", {16}},
        {SpvMemoryAccessNontemporalMask, "Nontemporal", {}},
    }));

TEST_F(TextToBinaryTest, CombinedMemoryAccessMask) {
  const std::string input = "OpStore %ptr %value Volatile|Aligned 16";
  const uint32_t expected_mask =
      SpvMemoryAccessVolatileMask | SpvMemoryAccessAlignedMask;
  EXPECT_THAT(expected_mask, Eq(3u));
  EXPECT_THAT(CompiledInstructions(input),
              Eq(MakeInstruction(SpvOpStore, {1, 2, expected_mask, 16})));
}

// Test Storage Class enum values

using StorageClassTest = spvtest::TextToBinaryTestBase<
    ::testing::TestWithParam<EnumCase<SpvStorageClass>>>;

TEST_P(StorageClassTest, AnyStorageClass) {
  const std::string input = "%1 = OpVariable %2 " + GetParam().name();
  EXPECT_THAT(CompiledInstructions(input),
              Eq(MakeInstruction(SpvOpVariable, {1, 2, GetParam().value()})));
}

// clang-format off
#define CASE(NAME) { SpvStorageClass##NAME, #NAME, {} }
INSTANTIATE_TEST_SUITE_P(
    TextToBinaryStorageClassTest, StorageClassTest,
    ::testing::ValuesIn(std::vector<EnumCase<SpvStorageClass>>{
        CASE(UniformConstant),
        CASE(Input),
        CASE(Uniform),
        CASE(Output),
        CASE(Workgroup),
        CASE(CrossWorkgroup),
        CASE(Private),
        CASE(Function),
        CASE(Generic),
        CASE(PushConstant),
        CASE(AtomicCounter),
        CASE(Image),
    }));
#undef CASE
// clang-format on

using MemoryRoundTripTest = RoundTripTest;

// OpPtrEqual appeared in SPIR-V 1.4

TEST_F(MemoryRoundTripTest, OpPtrEqualGood) {
  std::string spirv = "%2 = OpPtrEqual %1 %3 %4\n";
  EXPECT_THAT(CompiledInstructions(spirv, SPV_ENV_UNIVERSAL_1_4),
              Eq(MakeInstruction(SpvOpPtrEqual, {1, 2, 3, 4})));
  std::string disassembly = EncodeAndDecodeSuccessfully(
      spirv, SPV_BINARY_TO_TEXT_OPTION_NONE, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpPtrEqualV13Bad) {
  std::string spirv = "%2 = OpPtrEqual %1 %3 %4\n";
  std::string err = CompileFailure(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_THAT(err, HasSubstr("Invalid Opcode name 'OpPtrEqual'"));
}

// OpPtrNotEqual appeared in SPIR-V 1.4

TEST_F(MemoryRoundTripTest, OpPtrNotEqualGood) {
  std::string spirv = "%2 = OpPtrNotEqual %1 %3 %4\n";
  EXPECT_THAT(CompiledInstructions(spirv, SPV_ENV_UNIVERSAL_1_4),
              Eq(MakeInstruction(SpvOpPtrNotEqual, {1, 2, 3, 4})));
  std::string disassembly = EncodeAndDecodeSuccessfully(
      spirv, SPV_BINARY_TO_TEXT_OPTION_NONE, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpPtrNotEqualV13Bad) {
  std::string spirv = "%2 = OpPtrNotEqual %1 %3 %4\n";
  std::string err = CompileFailure(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_THAT(err, HasSubstr("Invalid Opcode name 'OpPtrNotEqual'"));
}

// OpPtrDiff appeared in SPIR-V 1.4

TEST_F(MemoryRoundTripTest, OpPtrDiffGood) {
  std::string spirv = "%2 = OpPtrDiff %1 %3 %4\n";
  EXPECT_THAT(CompiledInstructions(spirv, SPV_ENV_UNIVERSAL_1_4),
              Eq(MakeInstruction(SpvOpPtrDiff, {1, 2, 3, 4})));
  std::string disassembly = EncodeAndDecodeSuccessfully(
      spirv, SPV_BINARY_TO_TEXT_OPTION_NONE, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpPtrDiffV13Good) {
  // OpPtrDiff is enabled by a capability as well, so we can assemble
  // it even in older SPIR-V environments.  We do that so we can
  // write tests.
  std::string spirv = "%2 = OpPtrDiff %1 %3 %4\n";
  std::string disassembly = EncodeAndDecodeSuccessfully(
      spirv, SPV_BINARY_TO_TEXT_OPTION_NONE, SPV_ENV_UNIVERSAL_1_4);
}

// OpCopyMemory

TEST_F(MemoryRoundTripTest, OpCopyMemoryNoMemAccessGood) {
  std::string spirv = "OpCopyMemory %1 %2\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemory, {1, 2})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemoryTooFewArgsBad) {
  std::string spirv = "OpCopyMemory %1\n";
  std::string err = CompileFailure(spirv);
  EXPECT_THAT(err, HasSubstr("Expected operand for OpCopyMemory instruction, "
                             "but found the end of the stream."));
}

TEST_F(MemoryRoundTripTest, OpCopyMemoryTooManyArgsBad) {
  std::string spirv = "OpCopyMemory %1 %2 %3\n";
  std::string err = CompileFailure(spirv);
  EXPECT_THAT(err, HasSubstr("Invalid memory access operand '%3'"));
}

TEST_F(MemoryRoundTripTest, OpCopyMemoryAccessNoneGood) {
  std::string spirv = "OpCopyMemory %1 %2 None\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemory, {1, 2, 0})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemoryAccessVolatileGood) {
  std::string spirv = "OpCopyMemory %1 %2 Volatile\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemory, {1, 2, 1})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemoryAccessAligned8Good) {
  std::string spirv = "OpCopyMemory %1 %2 Aligned 8\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemory, {1, 2, 2, 8})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemoryAccessNontemporalGood) {
  std::string spirv = "OpCopyMemory %1 %2 Nontemporal\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemory, {1, 2, 4})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemoryAccessAvGood) {
  std::string spirv = "OpCopyMemory %1 %2 MakePointerAvailable %3\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemory, {1, 2, 8, 3})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemoryAccessVisGood) {
  std::string spirv = "OpCopyMemory %1 %2 MakePointerVisible %3\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemory, {1, 2, 16, 3})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemoryAccessNonPrivateGood) {
  std::string spirv = "OpCopyMemory %1 %2 NonPrivatePointer\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemory, {1, 2, 32})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemoryAccessMixedGood) {
  std::string spirv =
      "OpCopyMemory %1 %2 "
      "Volatile|Aligned|Nontemporal|MakePointerAvailable|"
      "MakePointerVisible|NonPrivatePointer 16 %3 %4\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemory, {1, 2, 63, 16, 3, 4})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemoryTwoAccessV13Good) {
  std::string spirv = "OpCopyMemory %1 %2 Volatile Volatile\n";
  // Note: This will assemble but should not validate for SPIR-V 1.3
  EXPECT_THAT(CompiledInstructions(spirv, SPV_ENV_UNIVERSAL_1_3),
              Eq(MakeInstruction(SpvOpCopyMemory, {1, 2, 1, 1})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemoryTwoAccessV14Good) {
  std::string spirv = "OpCopyMemory %1 %2 Volatile Volatile\n";
  EXPECT_THAT(CompiledInstructions(spirv, SPV_ENV_UNIVERSAL_1_4),
              Eq(MakeInstruction(SpvOpCopyMemory, {1, 2, 1, 1})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemoryTwoAccessMixedV14Good) {
  std::string spirv =
      "OpCopyMemory %1 %2 Volatile|Nontemporal|"
      "MakePointerVisible %3 "
      "Aligned|MakePointerAvailable|NonPrivatePointer 16 %4\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemory, {1, 2, 21, 3, 42, 16, 4})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

// OpCopyMemorySized

TEST_F(MemoryRoundTripTest, OpCopyMemorySizedNoMemAccessGood) {
  std::string spirv = "OpCopyMemorySized %1 %2 %3\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemorySized, {1, 2, 3})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemorySizedTooFewArgsBad) {
  std::string spirv = "OpCopyMemorySized %1 %2\n";
  std::string err = CompileFailure(spirv);
  EXPECT_THAT(err, HasSubstr("Expected operand for OpCopyMemorySized "
                             "instruction, but found the end of the stream."));
}

TEST_F(MemoryRoundTripTest, OpCopyMemorySizedTooManyArgsBad) {
  std::string spirv = "OpCopyMemorySized %1 %2 %3 %4\n";
  std::string err = CompileFailure(spirv);
  EXPECT_THAT(err, HasSubstr("Invalid memory access operand '%4'"));
}

TEST_F(MemoryRoundTripTest, OpCopyMemorySizedAccessNoneGood) {
  std::string spirv = "OpCopyMemorySized %1 %2 %3 None\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemorySized, {1, 2, 3, 0})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemorySizedAccessVolatileGood) {
  std::string spirv = "OpCopyMemorySized %1 %2 %3 Volatile\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemorySized, {1, 2, 3, 1})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemorySizedAccessAligned8Good) {
  std::string spirv = "OpCopyMemorySized %1 %2 %3 Aligned 8\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemorySized, {1, 2, 3, 2, 8})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemorySizedAccessNontemporalGood) {
  std::string spirv = "OpCopyMemorySized %1 %2 %3 Nontemporal\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemorySized, {1, 2, 3, 4})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemorySizedAccessAvGood) {
  std::string spirv = "OpCopyMemorySized %1 %2 %3 MakePointerAvailable %4\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemorySized, {1, 2, 3, 8, 4})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemorySizedAccessVisGood) {
  std::string spirv = "OpCopyMemorySized %1 %2 %3 MakePointerVisible %4\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemorySized, {1, 2, 3, 16, 4})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemorySizedAccessNonPrivateGood) {
  std::string spirv = "OpCopyMemorySized %1 %2 %3 NonPrivatePointer\n";
  EXPECT_THAT(CompiledInstructions(spirv),
              Eq(MakeInstruction(SpvOpCopyMemorySized, {1, 2, 3, 32})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemorySizedAccessMixedGood) {
  std::string spirv =
      "OpCopyMemorySized %1 %2 %3 "
      "Volatile|Aligned|Nontemporal|MakePointerAvailable|"
      "MakePointerVisible|NonPrivatePointer 16 %4 %5\n";
  EXPECT_THAT(
      CompiledInstructions(spirv),
      Eq(MakeInstruction(SpvOpCopyMemorySized, {1, 2, 3, 63, 16, 4, 5})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemorySizedTwoAccessV13Good) {
  std::string spirv = "OpCopyMemorySized %1 %2 %3 Volatile Volatile\n";
  // Note: This will assemble but should not validate for SPIR-V 1.3
  EXPECT_THAT(CompiledInstructions(spirv, SPV_ENV_UNIVERSAL_1_3),
              Eq(MakeInstruction(SpvOpCopyMemorySized, {1, 2, 3, 1, 1})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemorySizedTwoAccessV14Good) {
  std::string spirv = "OpCopyMemorySized %1 %2 %3 Volatile Volatile\n";
  EXPECT_THAT(CompiledInstructions(spirv, SPV_ENV_UNIVERSAL_1_4),
              Eq(MakeInstruction(SpvOpCopyMemorySized, {1, 2, 3, 1, 1})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(MemoryRoundTripTest, OpCopyMemorySizedTwoAccessMixedV14Good) {
  std::string spirv =
      "OpCopyMemorySized %1 %2 %3 Volatile|Nontemporal|"
      "MakePointerVisible %4 "
      "Aligned|MakePointerAvailable|NonPrivatePointer 16 %5\n";
  EXPECT_THAT(
      CompiledInstructions(spirv),
      Eq(MakeInstruction(SpvOpCopyMemorySized, {1, 2, 3, 21, 4, 42, 16, 5})));
  std::string disassembly =
      EncodeAndDecodeSuccessfully(spirv, SPV_BINARY_TO_TEXT_OPTION_NONE);
  EXPECT_THAT(disassembly, Eq(spirv));
}

// TODO(dneto): OpVariable with initializers
// TODO(dneto): OpImageTexelPointer
// TODO(dneto): OpLoad
// TODO(dneto): OpStore
// TODO(dneto): OpAccessChain
// TODO(dneto): OpInBoundsAccessChain
// TODO(dneto): OpPtrAccessChain
// TODO(dneto): OpArrayLength
// TODO(dneto): OpGenercPtrMemSemantics

}  // namespace
}  // namespace spvtools
