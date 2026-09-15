// Copyright (c) 2024-2026 Arm Ltd.
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
#include "spirv/unified1/NonSemanticGraphDebugInfo.h"
#include "test/test_fixture.h"
#include "test/unit_spirv.h"

namespace spvtools {
namespace {

using spvtest::Concatenate;
using spvtest::MakeInstruction;
using spvtest::TextToBinaryTest;
using testing::Eq;
using utils::MakeVector;

TEST_F(TextToBinaryTest, NonSemanticGraphDebugInfoImportTest) {
  const std::string src =
      "%1 = OpExtInstImport \"NonSemantic.Graph.DebugInfo.1\"";
  EXPECT_THAT(CompiledInstructions(src),
              Eq(MakeInstruction(spv::Op::OpExtInstImport, {1},
                                 MakeVector("NonSemantic.Graph.DebugInfo.1"))));
}

TEST_F(TextToBinaryTest, NonSemanticGraphDebugInfoDebugGraph) {
  const std::string src =
      "%1 = OpExtInstImport \"NonSemantic.Graph.DebugInfo.1\"\n"
      "%3 = OpExtInst %2 %1 DebugGraph %4 %5\n";

  // First make sure it assembles correctly.
  EXPECT_THAT(CompiledInstructions(src),
              Eq(Concatenate(
                  {MakeInstruction(spv::Op::OpExtInstImport, {1},
                                   MakeVector("NonSemantic.Graph.DebugInfo.1")),
                   MakeInstruction(
                       spv::Op::OpExtInst,
                       {2, 3, 1, NonSemanticGraphDebugInfoDebugGraph, 4, 5})})))
      << src;
  // Now check the round trip through the disassembler.
  EXPECT_THAT(EncodeAndDecodeSuccessfully(src), src) << src;
}

TEST_F(TextToBinaryTest, NonSemanticGraphDebugInfoDebugOperationSingle) {
  const std::string src =
      "%1 = OpExtInstImport \"NonSemantic.Graph.DebugInfo.1\"\n"
      "%3 = OpExtInst %2 %1 DebugOperation %4 %5 %6\n";

  // First make sure it assembles correctly.
  EXPECT_THAT(
      CompiledInstructions(src),
      Eq(Concatenate(
          {MakeInstruction(spv::Op::OpExtInstImport, {1},
                           MakeVector("NonSemantic.Graph.DebugInfo.1")),
           MakeInstruction(
               spv::Op::OpExtInst,
               {2, 3, 1, NonSemanticGraphDebugInfoDebugOperation, 4, 5, 6})})))
      << src;
  // Now check the round trip through the disassembler.
  EXPECT_THAT(EncodeAndDecodeSuccessfully(src), src) << src;
}

TEST_F(TextToBinaryTest, NonSemanticGraphDebugInfoDebugOperationMultiple) {
  const std::string src =
      "%1 = OpExtInstImport \"NonSemantic.Graph.DebugInfo.1\"\n"
      "%3 = OpExtInst %2 %1 DebugOperation %4 %5 %6 %7 %8\n";

  // First make sure it assembles correctly.
  EXPECT_THAT(
      CompiledInstructions(src),
      Eq(Concatenate(
          {MakeInstruction(spv::Op::OpExtInstImport, {1},
                           MakeVector("NonSemantic.Graph.DebugInfo.1")),
           MakeInstruction(spv::Op::OpExtInst,
                           {2, 3, 1, NonSemanticGraphDebugInfoDebugOperation, 4,
                            5, 6, 7, 8})})))
      << src;
  // Now check the round trip through the disassembler.
  EXPECT_THAT(EncodeAndDecodeSuccessfully(src), src) << src;
}

TEST_F(TextToBinaryTest, NonSemanticGraphDebugInfoDebugTensorSingle) {
  const std::string src =
      "%1 = OpExtInstImport \"NonSemantic.Graph.DebugInfo.1\"\n"
      "%3 = OpExtInst %2 %1 DebugTensor %4 %5\n";

  // First make sure it assembles correctly.
  EXPECT_THAT(
      CompiledInstructions(src),
      Eq(Concatenate(
          {MakeInstruction(spv::Op::OpExtInstImport, {1},
                           MakeVector("NonSemantic.Graph.DebugInfo.1")),
           MakeInstruction(
               spv::Op::OpExtInst,
               {2, 3, 1, NonSemanticGraphDebugInfoDebugTensor, 4, 5})})))
      << src;
  // Now check the round trip through the disassembler.
  EXPECT_THAT(EncodeAndDecodeSuccessfully(src), src) << src;
}

TEST_F(TextToBinaryTest, NonSemanticGraphDebugInfoDebugTensorCompositeElement) {
  const std::string src =
      "%1 = OpExtInstImport \"NonSemantic.Graph.DebugInfo.1\"\n"
      "%3 = OpExtInst %2 %1 DebugTensor %4 %5 %6\n";

  // First make sure it assembles correctly.
  EXPECT_THAT(
      CompiledInstructions(src),
      Eq(Concatenate(
          {MakeInstruction(spv::Op::OpExtInstImport, {1},
                           MakeVector("NonSemantic.Graph.DebugInfo.1")),
           MakeInstruction(
               spv::Op::OpExtInst,
               {2, 3, 1, NonSemanticGraphDebugInfoDebugTensor, 4, 5, 6})})))
      << src;
  // Now check the round trip through the disassembler.
  EXPECT_THAT(EncodeAndDecodeSuccessfully(src), src) << src;
}

}  // namespace
}  // namespace spvtools
