// Copyright (c) 2017 Google Inc.
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

#include <vector>

#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using StrengthReductionBasicTest = PassTest<::testing::Test>;

TEST_F(StrengthReductionBasicTest, ClearDontInline) {
  const std::vector<const char*> text = {
      // clang-format off
               "OpCapability Shader",
          "%1 = OpExtInstImport \"GLSL.std.450\"",
               "OpMemoryModel Logical GLSL450",
               "OpEntryPoint Vertex %main \"main\"",
       "%void = OpTypeVoid",
          "%4 = OpTypeFunction %void",
"; CHECK: OpFunction %void None",
       "%main = OpFunction %void DontInline %4",
          "%8 = OpLabel",
               "OpReturn",
               "OpFunctionEnd"
      // clang-format on
  };

  SinglePassRunAndMatch<RemoveDontInline>(JoinAllInsts(text), true);
}

TEST_F(StrengthReductionBasicTest, LeaveUnchanged1) {
  const std::vector<const char*> text = {
      // clang-format off
      "OpCapability Shader",
      "%1 = OpExtInstImport \"GLSL.std.450\"",
      "OpMemoryModel Logical GLSL450",
      "OpEntryPoint Vertex %main \"main\"",
      "%void = OpTypeVoid",
      "%4 = OpTypeFunction %void",
      "%main = OpFunction %void None %4",
      "%8 = OpLabel",
      "OpReturn",
      "OpFunctionEnd"
      // clang-format on
  };

  EXPECT_EQ(Pass::Status::SuccessWithoutChange,
            std::get<1>(SinglePassRunAndDisassemble<RemoveDontInline>(
                JoinAllInsts(text), false, true)));
}

TEST_F(StrengthReductionBasicTest, LeaveUnchanged2) {
  const std::vector<const char*> text = {
      // clang-format off
      "OpCapability Shader",
      "%1 = OpExtInstImport \"GLSL.std.450\"",
      "OpMemoryModel Logical GLSL450",
      "OpEntryPoint Vertex %main \"main\"",
      "%void = OpTypeVoid",
      "%4 = OpTypeFunction %void",
      "%main = OpFunction %void Inline %4",
      "%8 = OpLabel",
      "OpReturn",
      "OpFunctionEnd"
      // clang-format on
  };

  EXPECT_EQ(Pass::Status::SuccessWithoutChange,
            std::get<1>(SinglePassRunAndDisassemble<RemoveDontInline>(
                JoinAllInsts(text), false, true)));
}

TEST_F(StrengthReductionBasicTest, ClearMultipleDontInline) {
  const std::vector<const char*> text = {
      // clang-format off
      "OpCapability Shader",
      "%1 = OpExtInstImport \"GLSL.std.450\"",
      "OpMemoryModel Logical GLSL450",
      "OpEntryPoint Vertex %main1 \"main1\"",
      "OpEntryPoint Vertex %main2 \"main2\"",
      "OpEntryPoint Vertex %main3 \"main3\"",
      "OpEntryPoint Vertex %main4 \"main4\"",
      "%void = OpTypeVoid",
      "%4 = OpTypeFunction %void",
      "; CHECK: OpFunction %void None",
      "%main1 = OpFunction %void DontInline %4",
      "%8 = OpLabel",
      "OpReturn",
      "OpFunctionEnd",
      "; CHECK: OpFunction %void Inline",
      "%main2 = OpFunction %void Inline %4",
      "%9 = OpLabel",
      "OpReturn",
      "OpFunctionEnd",
      "; CHECK: OpFunction %void Pure",
      "%main3 = OpFunction %void DontInline|Pure %4",
      "%10 = OpLabel",
      "OpReturn",
      "OpFunctionEnd",
      "; CHECK: OpFunction %void None",
      "%main4 = OpFunction %void None %4",
      "%11 = OpLabel",
      "OpReturn",
      "OpFunctionEnd"
      // clang-format on
  };

  SinglePassRunAndMatch<RemoveDontInline>(JoinAllInsts(text), true);
}
}  // namespace
}  // namespace opt
}  // namespace spvtools
