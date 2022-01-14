// Copyright (c) 2021 Google LLC.
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

#include "source/opt/dataflow.h"

#include <map>
#include <set>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "opt/function_utils.h"
#include "source/opt/build_module.h"

namespace spvtools {
namespace opt {
namespace {

using DataFlowTest = ::testing::Test;

// Simple analyses for testing:

// Stores the result IDs of visited instructions in visit order.
struct VisitOrder : public ForwardDataFlowAnalysis {
  std::vector<uint32_t> visited_result_ids;

  VisitOrder(IRContext& context, LabelPosition label_position)
      : ForwardDataFlowAnalysis(context, label_position) {}

  VisitResult Visit(Instruction* inst) override {
    if (inst->HasResultId()) {
      visited_result_ids.push_back(inst->result_id());
    }
    return DataFlowAnalysis::VisitResult::kResultFixed;
  }
};

// For each block, stores the set of blocks it can be preceded by.
// For example, with the following CFG:
//    V-----------.
// -> 11 -> 12 -> 13 -> 15
//            \-> 14 ---^
//
// The answer is:
// 11: 11, 12, 13
// 12: 11, 12, 13
// 13: 11, 12, 13
// 14: 11, 12, 13
// 15: 11, 12, 13, 14
struct BackwardReachability : public ForwardDataFlowAnalysis {
  std::map<uint32_t, std::set<uint32_t>> reachable_from;

  BackwardReachability(IRContext& context)
      : ForwardDataFlowAnalysis(
            context, ForwardDataFlowAnalysis::LabelPosition::kLabelsOnly) {}

  VisitResult Visit(Instruction* inst) override {
    // Conditional branches can be enqueued from labels, so skip them.
    if (inst->opcode() != SpvOpLabel)
      return DataFlowAnalysis::VisitResult::kResultFixed;
    uint32_t id = inst->result_id();
    VisitResult ret = DataFlowAnalysis::VisitResult::kResultFixed;
    std::set<uint32_t>& precedents = reachable_from[id];
    for (uint32_t pred : context().cfg()->preds(id)) {
      bool pred_inserted = precedents.insert(pred).second;
      if (pred_inserted) {
        ret = DataFlowAnalysis::VisitResult::kResultChanged;
      }
      for (uint32_t block : reachable_from[pred]) {
        bool inserted = precedents.insert(block).second;
        if (inserted) {
          ret = DataFlowAnalysis::VisitResult::kResultChanged;
        }
      }
    }
    return ret;
  }

  void InitializeWorklist(Function* function,
                          bool is_first_iteration) override {
    // Since successor function is exact, only need one pass.
    if (is_first_iteration) {
      ForwardDataFlowAnalysis::InitializeWorklist(function, true);
    }
  }
};

TEST_F(DataFlowTest, ReversePostOrder) {
  // Note: labels and IDs are intentionally out of order.
  //
  // CFG: (order of branches is from bottom to top)
  //          V-----------.
  // -> 50 -> 40 -> 20 -> 60 -> 70
  //            \-> 30 ---^

  // DFS tree with RPO numbering:
  // -> 50[0] -> 40[1] -> 20[2]    60[4] -> 70[5]
  //                  \-> 30[3] ---^

  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %6 = OpTypeBool
          %5 = OpConstantTrue %6
          %2 = OpFunction %3 None %4
         %50 = OpLabel
         %51 = OpUndef %6
         %52 = OpUndef %6
               OpBranch %40
         %70 = OpLabel
         %69 = OpUndef %6
               OpReturn
         %60 = OpLabel
         %61 = OpUndef %6
               OpBranchConditional %5 %70 %40
         %30 = OpLabel
         %29 = OpUndef %6
               OpBranch %60
         %20 = OpLabel
         %21 = OpUndef %6
               OpBranch %60
         %40 = OpLabel
         %39 = OpUndef %6
               OpBranchConditional %5 %30 %20
               OpFunctionEnd
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(context, nullptr);

  Function* function = spvtest::GetFunction(context->module(), 2);

  std::map<ForwardDataFlowAnalysis::LabelPosition, std::vector<uint32_t>>
      expected_order;
  expected_order[ForwardDataFlowAnalysis::LabelPosition::kLabelsOnly] = {
      50, 40, 20, 30, 60, 70,
  };
  expected_order[ForwardDataFlowAnalysis::LabelPosition::kLabelsAtBeginning] = {
      50, 51, 52, 40, 39, 20, 21, 30, 29, 60, 61, 70, 69,
  };
  expected_order[ForwardDataFlowAnalysis::LabelPosition::kLabelsAtEnd] = {
      51, 52, 50, 39, 40, 21, 20, 29, 30, 61, 60, 69, 70,
  };
  expected_order[ForwardDataFlowAnalysis::LabelPosition::kNoLabels] = {
      51, 52, 39, 21, 29, 61, 69,
  };

  for (const auto& test_case : expected_order) {
    VisitOrder analysis(*context, test_case.first);
    analysis.Run(function);
    EXPECT_EQ(test_case.second, analysis.visited_result_ids);
  }
}

TEST_F(DataFlowTest, BackwardReachability) {
  // CFG:
  //    V-----------.
  // -> 11 -> 12 -> 13 -> 15
  //            \-> 14 ---^

  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %6 = OpTypeBool
          %5 = OpConstantTrue %6
          %2 = OpFunction %3 None %4
         %11 = OpLabel
               OpBranch %12
         %12 = OpLabel
               OpBranchConditional %5 %14 %13
         %13 = OpLabel
               OpBranchConditional %5 %15 %11
         %14 = OpLabel
               OpBranch %15
         %15 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(context, nullptr);

  Function* function = spvtest::GetFunction(context->module(), 2);

  BackwardReachability analysis(*context);
  analysis.Run(function);

  std::map<uint32_t, std::set<uint32_t>> expected_result;
  expected_result[11] = {11, 12, 13};
  expected_result[12] = {11, 12, 13};
  expected_result[13] = {11, 12, 13};
  expected_result[14] = {11, 12, 13};
  expected_result[15] = {11, 12, 13, 14};
  EXPECT_EQ(expected_result, analysis.reachable_from);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
