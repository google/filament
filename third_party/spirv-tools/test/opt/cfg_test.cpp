// Copyright (c) 2019 Google LLC
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
#include "gtest/gtest.h"
#include "source/opt/ir_context.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using ::testing::ContainerEq;

using CFGTest = PassTest<::testing::Test>;

TEST_F(CFGTest, ForEachBlockInPostOrderIf) {
  const std::string test = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpName %main "main"
%bool = OpTypeBool
%true = OpConstantTrue %bool
%void = OpTypeVoid
%4 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%5 = OpConstant %uint 5
%main = OpFunction %void None %4
%8 = OpLabel
OpSelectionMerge %10 None
OpBranchConditional %true %9 %10
%9 = OpLabel
OpBranch %10
%10 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, test,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(nullptr, context);

  CFG* cfg = context->cfg();
  Module* module = context->module();
  Function* function = &*module->begin();
  std::vector<uint32_t> order;
  cfg->ForEachBlockInPostOrder(&*function->begin(), [&order](BasicBlock* bb) {
    order.push_back(bb->id());
  });

  std::vector<uint32_t> expected_result = {10, 9, 8};
  EXPECT_THAT(order, ContainerEq(expected_result));
}

TEST_F(CFGTest, ForEachBlockInPostOrderLoop) {
  const std::string test = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpName %main "main"
%bool = OpTypeBool
%true = OpConstantTrue %bool
%void = OpTypeVoid
%4 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%5 = OpConstant %uint 5
%main = OpFunction %void None %4
%8 = OpLabel
OpBranch %9
%9 = OpLabel
OpLoopMerge %11 %10 None
OpBranchConditional %true %11 %10
%10 = OpLabel
OpBranch %9
%11 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, test,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(nullptr, context);

  CFG* cfg = context->cfg();
  Module* module = context->module();
  Function* function = &*module->begin();
  std::vector<uint32_t> order;
  cfg->ForEachBlockInPostOrder(&*function->begin(), [&order](BasicBlock* bb) {
    order.push_back(bb->id());
  });

  std::vector<uint32_t> expected_result1 = {10, 11, 9, 8};
  std::vector<uint32_t> expected_result2 = {11, 10, 9, 8};
  EXPECT_THAT(order, AnyOf(ContainerEq(expected_result1),
                           ContainerEq(expected_result2)));
}

TEST_F(CFGTest, ForEachBlockInReversePostOrderIf) {
  const std::string test = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpName %main "main"
%bool = OpTypeBool
%true = OpConstantTrue %bool
%void = OpTypeVoid
%4 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%5 = OpConstant %uint 5
%main = OpFunction %void None %4
%8 = OpLabel
OpSelectionMerge %10 None
OpBranchConditional %true %9 %10
%9 = OpLabel
OpBranch %10
%10 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, test,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(nullptr, context);

  CFG* cfg = context->cfg();
  Module* module = context->module();
  Function* function = &*module->begin();
  std::vector<uint32_t> order;
  cfg->ForEachBlockInReversePostOrder(
      &*function->begin(),
      [&order](BasicBlock* bb) { order.push_back(bb->id()); });

  std::vector<uint32_t> expected_result = {8, 9, 10};
  EXPECT_THAT(order, ContainerEq(expected_result));
}

TEST_F(CFGTest, ForEachBlockInReversePostOrderLoop) {
  const std::string test = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpName %main "main"
%bool = OpTypeBool
%true = OpConstantTrue %bool
%void = OpTypeVoid
%4 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%5 = OpConstant %uint 5
%main = OpFunction %void None %4
%8 = OpLabel
OpBranch %9
%9 = OpLabel
OpLoopMerge %11 %10 None
OpBranchConditional %true %11 %10
%10 = OpLabel
OpBranch %9
%11 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, test,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(nullptr, context);

  CFG* cfg = context->cfg();
  Module* module = context->module();
  Function* function = &*module->begin();
  std::vector<uint32_t> order;
  cfg->ForEachBlockInReversePostOrder(
      &*function->begin(),
      [&order](BasicBlock* bb) { order.push_back(bb->id()); });

  std::vector<uint32_t> expected_result1 = {8, 9, 10, 11};
  std::vector<uint32_t> expected_result2 = {8, 9, 11, 10};
  EXPECT_THAT(order, AnyOf(ContainerEq(expected_result1),
                           ContainerEq(expected_result2)));
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
