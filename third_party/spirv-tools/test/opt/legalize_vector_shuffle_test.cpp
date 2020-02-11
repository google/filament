// Copyright (c) 2019 Google LLC.
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

using LegalizeVectorShuffleTest = PassTest<::testing::Test>;

void operator+=(std::vector<const char*>& lhs, const char* rhs) {
  lhs.push_back(rhs);
}

void operator+=(std::vector<const char*>& lhs,
                const std::vector<const char*> rhs) {
  for (auto elem : rhs) lhs.push_back(elem);
}

std::vector<const char*> header = {
    "OpCapability Shader",
    "OpCapability VulkanMemoryModel",
    "OpExtension \"SPV_KHR_vulkan_memory_model\"",
    "OpMemoryModel Logical Vulkan",
    "OpEntryPoint Vertex %1 \"shader\"",
    "%uint = OpTypeInt 32 0",
    "%v3uint = OpTypeVector %uint 3"};

std::string GetTestString(const char* shuffle) {
  std::vector<const char*> result = header;
  result += {"%_ptr_Function_v3uint = OpTypePointer Function %v3uint",
             "%void = OpTypeVoid",
             "%6 = OpTypeFunction %void",
             "%1 = OpFunction %void None %6",
             "%7 = OpLabel",
             "%8 = OpVariable %_ptr_Function_v3uint Function",
             "%9 = OpLoad %v3uint %8",
             "%10 = OpLoad %v3uint %8"};
  result += shuffle;
  result += {"OpReturn", "OpFunctionEnd"};
  return JoinAllInsts(result);
}

TEST_F(LegalizeVectorShuffleTest, Changed) {
  std::string input =
      GetTestString("%11 = OpVectorShuffle %v3uint %9 %10 2 1 0xFFFFFFFF");
  std::string expected =
      GetTestString("%11 = OpVectorShuffle %v3uint %9 %10 2 1 0");

  SinglePassRunAndCheck<LegalizeVectorShufflePass>(input, expected,
                                                   /* skip_nop = */ false);
}

TEST_F(LegalizeVectorShuffleTest, FunctionUnchanged) {
  std::string input =
      GetTestString("%11 = OpVectorShuffle %v3uint %9 %10 2 1 0");
  std::string expected =
      GetTestString("%11 = OpVectorShuffle %v3uint %9 %10 2 1 0");

  SinglePassRunAndCheck<LegalizeVectorShufflePass>(input, expected,
                                                   /* skip_nop = */ false);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
