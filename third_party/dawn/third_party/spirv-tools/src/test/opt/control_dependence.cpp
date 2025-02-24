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

#include "source/opt/control_dependence.h"

#include <algorithm>
#include <vector>

#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "source/opt/build_module.h"
#include "source/opt/cfg.h"
#include "test/opt/function_utils.h"

namespace spvtools {
namespace opt {

namespace {
void GatherEdges(const ControlDependenceAnalysis& cdg,
                 std::vector<ControlDependence>& ret) {
  cdg.ForEachBlockLabel([&](uint32_t label) {
    ret.reserve(ret.size() + cdg.GetDependenceTargets(label).size());
    ret.insert(ret.end(), cdg.GetDependenceTargets(label).begin(),
               cdg.GetDependenceTargets(label).end());
  });
  std::sort(ret.begin(), ret.end());
  // Verify that reverse graph is the same.
  std::vector<ControlDependence> reverse_edges;
  reverse_edges.reserve(ret.size());
  cdg.ForEachBlockLabel([&](uint32_t label) {
    reverse_edges.insert(reverse_edges.end(),
                         cdg.GetDependenceSources(label).begin(),
                         cdg.GetDependenceSources(label).end());
  });
  std::sort(reverse_edges.begin(), reverse_edges.end());
  ASSERT_THAT(reverse_edges, testing::ElementsAreArray(ret));
}

using ControlDependenceTest = ::testing::Test;

TEST(ControlDependenceTest, DependenceSimpleCFG) {
  const std::string text = R"(
               OpCapability Addresses
               OpCapability Kernel
               OpMemoryModel Physical64 OpenCL
               OpEntryPoint Kernel %1 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeBool
          %5 = OpTypeInt 32 0
          %6 = OpConstant %5 0
          %7 = OpConstantFalse %4
          %8 = OpConstantTrue %4
          %9 = OpConstant %5 1
          %1 = OpFunction %2 None %3
         %10 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpSwitch %6 %12 1 %13
         %12 = OpLabel
               OpBranch %14
         %13 = OpLabel
               OpBranch %14
         %14 = OpLabel
               OpBranchConditional %8 %15 %16
         %15 = OpLabel
               OpBranch %19
         %16 = OpLabel
               OpBranchConditional %8 %17 %18
         %17 = OpLabel
               OpBranch %18
         %18 = OpLabel
               OpBranch %19
         %19 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  // CFG: (all edges pointing downward)
  //   %10
  //    |
  //   %11
  //  /   \ (R: %6 == 1, L: default)
  // %12 %13
  //  \   /
  //   %14
  // T/   \F
  // %15  %16
  //  | T/ |F
  //  | %17|
  //  |  \ |
  //  |   %18
  //  |  /
  // %19

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_0, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  Module* module = context->module();
  EXPECT_NE(nullptr, module) << "Assembling failed for shader:\n"
                             << text << std::endl;
  const Function* fn = spvtest::GetFunction(module, 1);
  const BasicBlock* entry = spvtest::GetBasicBlock(fn, 10);
  EXPECT_EQ(entry, fn->entry().get())
      << "The entry node is not the expected one";

  {
    PostDominatorAnalysis pdom;
    const CFG& cfg = *context->cfg();
    pdom.InitializeTree(cfg, fn);
    ControlDependenceAnalysis cdg;
    cdg.ComputeControlDependenceGraph(cfg, pdom);

    // Test HasBlock.
    for (uint32_t id = 10; id <= 19; id++) {
      EXPECT_TRUE(cdg.HasBlock(id));
    }
    EXPECT_TRUE(cdg.HasBlock(ControlDependenceAnalysis::kPseudoEntryBlock));
    // Check blocks before/after valid range.
    EXPECT_FALSE(cdg.HasBlock(5));
    EXPECT_FALSE(cdg.HasBlock(25));
    EXPECT_FALSE(cdg.HasBlock(UINT32_MAX));

    // Test ForEachBlockLabel.
    std::set<uint32_t> block_labels;
    cdg.ForEachBlockLabel([&block_labels](uint32_t id) {
      bool inserted = block_labels.insert(id).second;
      EXPECT_TRUE(inserted);  // Should have no duplicates.
    });
    EXPECT_THAT(block_labels, testing::ElementsAre(0, 10, 11, 12, 13, 14, 15,
                                                   16, 17, 18, 19));

    {
      // Test WhileEachBlockLabel.
      uint32_t iters = 0;
      EXPECT_TRUE(cdg.WhileEachBlockLabel([&iters](uint32_t) {
        ++iters;
        return true;
      }));
      EXPECT_EQ((uint32_t)block_labels.size(), iters);
      iters = 0;
      EXPECT_FALSE(cdg.WhileEachBlockLabel([&iters](uint32_t) {
        ++iters;
        return false;
      }));
      EXPECT_EQ(1, iters);
    }

    // Test IsDependent.
    EXPECT_TRUE(cdg.IsDependent(12, 11));
    EXPECT_TRUE(cdg.IsDependent(13, 11));
    EXPECT_TRUE(cdg.IsDependent(15, 14));
    EXPECT_TRUE(cdg.IsDependent(16, 14));
    EXPECT_TRUE(cdg.IsDependent(18, 14));
    EXPECT_TRUE(cdg.IsDependent(17, 16));
    EXPECT_TRUE(cdg.IsDependent(10, 0));
    EXPECT_TRUE(cdg.IsDependent(11, 0));
    EXPECT_TRUE(cdg.IsDependent(14, 0));
    EXPECT_TRUE(cdg.IsDependent(19, 0));
    EXPECT_FALSE(cdg.IsDependent(14, 11));
    EXPECT_FALSE(cdg.IsDependent(17, 14));
    EXPECT_FALSE(cdg.IsDependent(19, 14));
    EXPECT_FALSE(cdg.IsDependent(12, 0));

    // Test GetDependenceSources/Targets.
    std::vector<ControlDependence> edges;
    GatherEdges(cdg, edges);
    EXPECT_THAT(edges,
                testing::ElementsAre(
                    ControlDependence(0, 10), ControlDependence(0, 11, 10),
                    ControlDependence(0, 14, 10), ControlDependence(0, 19, 10),
                    ControlDependence(11, 12), ControlDependence(11, 13),
                    ControlDependence(14, 15), ControlDependence(14, 16),
                    ControlDependence(14, 18, 16), ControlDependence(16, 17)));

    const uint32_t expected_condition_ids[] = {
        0, 0, 0, 0, 6, 6, 8, 8, 8, 8,
    };

    for (uint32_t i = 0; i < edges.size(); i++) {
      EXPECT_EQ(expected_condition_ids[i], edges[i].GetConditionID(cfg));
    }
  }
}

TEST(ControlDependenceTest, DependencePaperCFG) {
  const std::string text = R"(
               OpCapability Addresses
               OpCapability Kernel
               OpMemoryModel Physical64 OpenCL
               OpEntryPoint Kernel %101 "main"
        %102 = OpTypeVoid
        %103 = OpTypeFunction %102
        %104 = OpTypeBool
        %108 = OpConstantTrue %104
        %101 = OpFunction %102 None %103
          %1 = OpLabel
               OpBranch %2
          %2 = OpLabel
               OpBranchConditional %108 %3 %7
          %3 = OpLabel
               OpBranchConditional %108 %4 %5
          %4 = OpLabel
               OpBranch %6
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpBranch %8
          %7 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpBranchConditional %108 %10 %11
         %10 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpBranchConditional %108 %12 %9
         %12 = OpLabel
               OpBranchConditional %108 %13 %2
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  // CFG: (edges pointing downward if no arrow)
  //         %1
  //         |
  //         %2 <----+
  //       T/  \F    |
  //      %3    \    |
  //    T/  \F   \   |
  //    %4  %5    %7 |
  //     \  /    /   |
  //      %6    /    |
  //        \  /     |
  //         %8      |
  //         |       |
  //         %9 <-+  |
  //       T/  |  |  |
  //       %10 |  |  |
  //        \  |  |  |
  //         %11-F+  |
  //         T|      |
  //         %12-F---+
  //         T|
  //         %13

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_0, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  Module* module = context->module();
  EXPECT_NE(nullptr, module) << "Assembling failed for shader:\n"
                             << text << std::endl;
  const Function* fn = spvtest::GetFunction(module, 101);
  const BasicBlock* entry = spvtest::GetBasicBlock(fn, 1);
  EXPECT_EQ(entry, fn->entry().get())
      << "The entry node is not the expected one";

  {
    PostDominatorAnalysis pdom;
    const CFG& cfg = *context->cfg();
    pdom.InitializeTree(cfg, fn);
    ControlDependenceAnalysis cdg;
    cdg.ComputeControlDependenceGraph(cfg, pdom);

    std::vector<ControlDependence> edges;
    GatherEdges(cdg, edges);
    EXPECT_THAT(
        edges, testing::ElementsAre(
                   ControlDependence(0, 1), ControlDependence(0, 2, 1),
                   ControlDependence(0, 8, 1), ControlDependence(0, 9, 1),
                   ControlDependence(0, 11, 1), ControlDependence(0, 12, 1),
                   ControlDependence(0, 13, 1), ControlDependence(2, 3),
                   ControlDependence(2, 6, 3), ControlDependence(2, 7),
                   ControlDependence(3, 4), ControlDependence(3, 5),
                   ControlDependence(9, 10), ControlDependence(11, 9),
                   ControlDependence(11, 11, 9), ControlDependence(12, 2),
                   ControlDependence(12, 8, 2), ControlDependence(12, 9, 2),
                   ControlDependence(12, 11, 2), ControlDependence(12, 12, 2)));

    const uint32_t expected_condition_ids[] = {
        0,   0,   0,   0,   0,   0,   0,   108, 108, 108,
        108, 108, 108, 108, 108, 108, 108, 108, 108, 108,
    };

    for (uint32_t i = 0; i < edges.size(); i++) {
      EXPECT_EQ(expected_condition_ids[i], edges[i].GetConditionID(cfg));
    }
  }
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
