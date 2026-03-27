// Copyright (c) 2024-2025 Arm Ltd.
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

#include <string>

#include "gmock/gmock.h"
#include "source/util/string_utils.h"
#include "spirv/unified1/ArmMotionEngine.100.h"
#include "test/test_fixture.h"
#include "test/unit_spirv.h"

namespace spvtools {
namespace {

using spvtest::Concatenate;
using spvtest::MakeInstruction;
using spvtest::TextToBinaryTest;
using testing::Eq;
using utils::MakeVector;

TEST_F(TextToBinaryTest, ArmMotionEngineImportTest) {
  const std::string src = "%1 = OpExtInstImport \"Arm.MotionEngine.100\"";
  EXPECT_THAT(CompiledInstructions(src),
              Eq(MakeInstruction(spv::Op::OpExtInstImport, {1},
                                 MakeVector("Arm.MotionEngine.100"))));
}

TEST_F(TextToBinaryTest, ArmMotionEngineInstructionMIN_SAD) {
  const std::string src =
      "%1 = OpExtInstImport \"Arm.MotionEngine.100\"\n"
      "%3 = OpExtInst %2 %1 MIN_SAD %4 %5 %6 %7 %8 %9 %10 %11 %12\n";

  // First make sure it assembles correctly.
  EXPECT_THAT(
      CompiledInstructions(src),
      Eq(Concatenate({MakeInstruction(spv::Op::OpExtInstImport, {1},
                                      MakeVector("Arm.MotionEngine.100")),
                      MakeInstruction(spv::Op::OpExtInst,
                                      {2, 3, 1, ArmMotionEngineMIN_SAD, 4, 5, 6,
                                       7, 8, 9, 10, 11, 12})})))
      << src;
  // Now check the round trip through the disassembler.
  EXPECT_THAT(EncodeAndDecodeSuccessfully(src), src) << src;
}

TEST_F(TextToBinaryTest, ArmMotionEngineInstructionMIN_SAD_COST) {
  const std::string src =
      "%1 = OpExtInstImport \"Arm.MotionEngine.100\"\n"
      "%3 = OpExtInst %2 %1 MIN_SAD_COST %4 %5 %6 %7 %8 %9 %10 %11 %12\n";

  // First make sure it assembles correctly.
  EXPECT_THAT(
      CompiledInstructions(src),
      Eq(Concatenate({MakeInstruction(spv::Op::OpExtInstImport, {1},
                                      MakeVector("Arm.MotionEngine.100")),
                      MakeInstruction(spv::Op::OpExtInst,
                                      {2, 3, 1, ArmMotionEngineMIN_SAD_COST, 4,
                                       5, 6, 7, 8, 9, 10, 11, 12})})))
      << src;
  // Now check the round trip through the disassembler.
  EXPECT_THAT(EncodeAndDecodeSuccessfully(src), src) << src;
}

TEST_F(TextToBinaryTest, ArmMotionEngineInstructionRAW_SAD) {
  const std::string src =
      "%1 = OpExtInstImport \"Arm.MotionEngine.100\"\n"
      "%3 = OpExtInst %2 %1 RAW_SAD %4 %5 %6 %7 %8 %9 %10 %11\n";

  // First make sure it assembles correctly.
  EXPECT_THAT(
      CompiledInstructions(src),
      Eq(Concatenate(
          {MakeInstruction(spv::Op::OpExtInstImport, {1},
                           MakeVector("Arm.MotionEngine.100")),
           MakeInstruction(spv::Op::OpExtInst, {2, 3, 1, ArmMotionEngineRAW_SAD,
                                                4, 5, 6, 7, 8, 9, 10, 11})})))
      << src;
  // Now check the round trip through the disassembler.
  EXPECT_THAT(EncodeAndDecodeSuccessfully(src), src) << src;
}

}  // namespace
}  // namespace spvtools
