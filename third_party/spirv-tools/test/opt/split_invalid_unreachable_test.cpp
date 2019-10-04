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

using SplitInvalidUnreachableTest = PassTest<::testing::Test>;

std::string spirv_header = R"(OpCapability Shader
OpCapability VulkanMemoryModel
OpExtension "SPV_KHR_vulkan_memory_model"
OpMemoryModel Logical Vulkan
OpEntryPoint Vertex %1 "shader"
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%void = OpTypeVoid
%bool = OpTypeBool
%7 = OpTypeFunction %void
)";

std::string function_head = R"(%1 = OpFunction %void None %7
%8 = OpLabel
OpBranch %9
)";

std::string function_tail = "OpFunctionEnd\n";

std::string GetLoopMergeBlock(std::string block_id, std::string merge_id,
                              std::string continue_id, std::string body_id) {
  std::string result;
  result += block_id + " = OpLabel\n";
  result += "OpLoopMerge " + merge_id + " " + continue_id + " None\n";
  result += "OpBranch " + body_id + "\n";
  return result;
}

std::string GetSelectionMergeBlock(std::string block_id,
                                   std::string condition_id,
                                   std::string merge_id, std::string true_id,
                                   std::string false_id) {
  std::string result;
  result += block_id + " = OpLabel\n";
  result += condition_id + " = OpSLessThan %bool %uint_1 %uint_2\n";
  result += "OpSelectionMerge " + merge_id + " None\n";
  result += "OpBranchConditional " + condition_id + " " + true_id + " " +
            false_id + "\n";

  return result;
}

std::string GetReturnBlock(std::string block_id) {
  std::string result;
  result += block_id + " = OpLabel\n";
  result += "OpReturn\n";
  return result;
}

std::string GetUnreachableBlock(std::string block_id) {
  std::string result;
  result += block_id + " = OpLabel\n";
  result += "OpUnreachable\n";
  return result;
}

std::string GetBranchBlock(std::string block_id, std::string target_id) {
  std::string result;
  result += block_id + " = OpLabel\n";
  result += "OpBranch " + target_id + "\n";
  return result;
}

TEST_F(SplitInvalidUnreachableTest, NoInvalidBlocks) {
  std::string input = spirv_header + function_head;
  input += GetLoopMergeBlock("%9", "%10", "%11", "%12");
  input += GetSelectionMergeBlock("%12", "%13", "%14", "%15", "%16");
  input += GetReturnBlock("%15");
  input += GetReturnBlock("%16");
  input += GetUnreachableBlock("%10");
  input += GetBranchBlock("%11", "%9");
  input += GetUnreachableBlock("%14");
  input += function_tail;

  SinglePassRunAndCheck<SplitInvalidUnreachablePass>(input, input,
                                                     /* skip_nop = */ false);
}

TEST_F(SplitInvalidUnreachableTest, SelectionInLoop) {
  std::string input = spirv_header + function_head;
  input += GetLoopMergeBlock("%9", "%10", "%11", "%12");
  input += GetSelectionMergeBlock("%12", "%13", "%11", "%15", "%16");
  input += GetReturnBlock("%15");
  input += GetReturnBlock("%16");
  input += GetUnreachableBlock("%10");
  input += GetBranchBlock("%11", "%9");
  input += function_tail;

  std::string expected = spirv_header + function_head;
  expected += GetLoopMergeBlock("%9", "%10", "%11", "%12");
  expected += GetSelectionMergeBlock("%12", "%13", "%16", "%14", "%15");
  expected += GetReturnBlock("%14");
  expected += GetReturnBlock("%15");
  expected += GetUnreachableBlock("%10");
  expected += GetUnreachableBlock("%16");
  expected += GetBranchBlock("%11", "%9");
  expected += function_tail;

  SinglePassRunAndCheck<SplitInvalidUnreachablePass>(input, expected,
                                                     /* skip_nop = */ false);
}

TEST_F(SplitInvalidUnreachableTest, LoopInSelection) {
  std::string input = spirv_header + function_head;
  input += GetSelectionMergeBlock("%9", "%10", "%11", "%12", "%13");
  input += GetLoopMergeBlock("%12", "%14", "%11", "%15");
  input += GetReturnBlock("%13");
  input += GetUnreachableBlock("%14");
  input += GetBranchBlock("%11", "%12");
  input += GetReturnBlock("%15");
  input += function_tail;

  std::string expected = spirv_header + function_head;
  expected += GetSelectionMergeBlock("%9", "%10", "%16", "%12", "%13");
  expected += GetLoopMergeBlock("%12", "%14", "%11", "%15");
  expected += GetReturnBlock("%13");
  expected += GetUnreachableBlock("%14");
  expected += GetUnreachableBlock("%16");
  expected += GetBranchBlock("%11", "%12");
  expected += GetReturnBlock("%15");
  expected += function_tail;

  SinglePassRunAndCheck<SplitInvalidUnreachablePass>(input, expected,
                                                     /* skip_nop = */ false);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
