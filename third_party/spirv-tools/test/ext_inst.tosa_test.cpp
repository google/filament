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
#include "spirv/unified1/TOSA.001000.1.h"
#include "test/test_fixture.h"
#include "test/unit_spirv.h"

namespace spvtools {
namespace {

using spvtest::Concatenate;
using spvtest::MakeInstruction;
using spvtest::TextToBinaryTest;
using testing::Eq;
using utils::MakeVector;

TEST_F(TextToBinaryTest, TOSAImportTest) {
  const std::string src = "%1 = OpExtInstImport \"TOSA.001000.1\"";
  EXPECT_THAT(CompiledInstructions(src),
              Eq(MakeInstruction(spv::Op::OpExtInstImport, {1},
                                 MakeVector("TOSA.001000.1"))));
}

TEST_F(TextToBinaryTest, TOSAInstructionSmokeTest) {
  const std::string src =
      "%1 = OpExtInstImport \"TOSA.001000.1\"\n"
      "%3 = OpExtInst %2 %1 ARGMAX %4 %5 %6\n";

  // First make sure it assembles correctly.
  EXPECT_THAT(
      CompiledInstructions(src),
      Eq(Concatenate({MakeInstruction(spv::Op::OpExtInstImport, {1},
                                      MakeVector("TOSA.001000.1")),
                      MakeInstruction(spv::Op::OpExtInst,
                                      {2, 3, 1, TOSAARGMAX, 4, 5, 6})})))
      << src;
  // Now check the round trip through the disassembler.
  EXPECT_THAT(EncodeAndDecodeSuccessfully(src), src) << src;
}

}  // namespace
}  // namespace spvtools
