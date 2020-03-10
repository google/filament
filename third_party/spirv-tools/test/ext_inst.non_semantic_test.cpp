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

// Assembler tests for non-semantic extended instructions

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "test/test_fixture.h"
#include "test/unit_spirv.h"

using ::testing::Eq;

namespace spvtools {
namespace {

using NonSemanticRoundTripTest = RoundTripTest;
using NonSemanticTextToBinaryTest = spvtest::TextToBinaryTest;

TEST_F(NonSemanticRoundTripTest, NonSemanticInsts) {
  std::string spirv = R"(OpExtension "SPV_KHR_non_semantic_info"
%1 = OpExtInstImport "NonSemantic.Testing.ExtInst"
%2 = OpTypeVoid
%3 = OpExtInst %2 %1 132384681 %2
%4 = OpTypeInt 32 0
%5 = OpConstant %4 123
%6 = OpString "Test string"
%7 = OpExtInst %4 %1 82198732 %5 %6
%8 = OpExtInstImport "NonSemantic.Testing.AnotherUnknownExtInstSet"
%9 = OpExtInst %4 %8 613874321 %7 %5 %6
)";
  std::string disassembly = EncodeAndDecodeSuccessfully(
      spirv, SPV_BINARY_TO_TEXT_OPTION_NONE, SPV_ENV_UNIVERSAL_1_0);
  EXPECT_THAT(disassembly, Eq(spirv));
}

TEST_F(NonSemanticTextToBinaryTest, InvalidExtInstSetName) {
  std::string spirv = R"(OpExtension "SPV_KHR_non_semantic_info"
%1 = OpExtInstImport "NonSemantic_Testing_ExtInst"
)";

  EXPECT_THAT(
      CompileFailure(spirv),
      Eq("Invalid extended instruction import 'NonSemantic_Testing_ExtInst'"));
}

TEST_F(NonSemanticTextToBinaryTest, NonSemanticIntParameter) {
  std::string spirv = R"(OpExtension "SPV_KHR_non_semantic_info"
%1 = OpExtInstImport "NonSemantic.Testing.ExtInst"
%2 = OpTypeVoid
%3 = OpExtInst %2 %1 1 99999
)";

  EXPECT_THAT(CompileFailure(spirv), Eq("Expected id to start with %."));
}

TEST_F(NonSemanticTextToBinaryTest, NonSemanticFloatParameter) {
  std::string spirv = R"(OpExtension "SPV_KHR_non_semantic_info"
%1 = OpExtInstImport "NonSemantic.Testing.ExtInst"
%2 = OpTypeVoid
%3 = OpExtInst %2 %1 1 3.141592
)";

  EXPECT_THAT(CompileFailure(spirv), Eq("Expected id to start with %."));
}

TEST_F(NonSemanticTextToBinaryTest, NonSemanticStringParameter) {
  std::string spirv = R"(OpExtension "SPV_KHR_non_semantic_info"
%1 = OpExtInstImport "NonSemantic.Testing.ExtInst"
%2 = OpTypeVoid
%3 = OpExtInst %2 %1 1 "foobar"
)";

  EXPECT_THAT(CompileFailure(spirv), Eq("Expected id to start with %."));
}

}  // namespace
}  // namespace spvtools
