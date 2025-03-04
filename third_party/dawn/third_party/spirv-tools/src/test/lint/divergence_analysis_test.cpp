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

#include "source/lint/divergence_analysis.h"

#include <string>

#include "gtest/gtest.h"
#include "source/opt/build_module.h"
#include "source/opt/ir_context.h"
#include "source/opt/module.h"
#include "spirv-tools/libspirv.h"

namespace spvtools {
namespace lint {
namespace {

void CLIMessageConsumer(spv_message_level_t level, const char*,
                        const spv_position_t& position, const char* message) {
  switch (level) {
    case SPV_MSG_FATAL:
    case SPV_MSG_INTERNAL_ERROR:
    case SPV_MSG_ERROR:
      std::cerr << "error: line " << position.index << ": " << message
                << std::endl;
      break;
    case SPV_MSG_WARNING:
      std::cout << "warning: line " << position.index << ": " << message
                << std::endl;
      break;
    case SPV_MSG_INFO:
      std::cout << "info: line " << position.index << ": " << message
                << std::endl;
      break;
    default:
      break;
  }
}

class DivergenceTest : public ::testing::Test {
 protected:
  std::unique_ptr<opt::IRContext> context_;
  std::unique_ptr<DivergenceAnalysis> divergence_;

  void Build(std::string text, uint32_t function_id = 1) {
    context_ = BuildModule(SPV_ENV_UNIVERSAL_1_0, CLIMessageConsumer, text,
                           SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
    ASSERT_NE(nullptr, context_.get());
    opt::Module* module = context_->module();
    ASSERT_NE(nullptr, module);
    // First function should have the given ID.
    ASSERT_NE(module->begin(), module->end());
    opt::Function* function = &*module->begin();
    ASSERT_EQ(function->result_id(), function_id);
    divergence_.reset(new DivergenceAnalysis(*context_));
    divergence_->Run(function);
  }
};

// Makes assertions a bit shorter.
using Level = DivergenceAnalysis::DivergenceLevel;

namespace {
std::string Preamble() {
  return R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "main" %x %y
	       OpExecutionMode %1 OriginLowerLeft
               OpDecorate %y Flat
       %void = OpTypeVoid
     %void_f = OpTypeFunction %void
       %bool = OpTypeBool
      %float = OpTypeFloat 32
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
       %zero = OpConstant %float 0
        %one = OpConstant %float 1
        %x_t = OpTypePointer Input %float
          %x = OpVariable %x_t Input
          %y = OpVariable %x_t Input
          %1 = OpFunction %void None %void_f
  )";
}
}  // namespace

TEST_F(DivergenceTest, SimpleTest) {
  // pseudocode:
  //     %10:
  //     %11 = load x
  //     if (%12 = (%11 < 0)) {
  //       %13:
  //       // do nothing
  //     }
  //     %14:
  //     return
  ASSERT_NO_FATAL_FAILURE(Build(Preamble() + R"(
         %10 = OpLabel
         %11 = OpLoad %float %x
         %12 = OpFOrdLessThan %bool %11 %zero
               OpSelectionMerge %14 None
               OpBranchConditional %12 %13 %14
         %13 = OpLabel
               OpBranch %14
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )"));
  // Control flow divergence.
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(10));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(13));
  EXPECT_EQ(12, divergence_->GetDivergenceSource(13));
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(14));
  // Value divergence.
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(11));
  EXPECT_EQ(0, divergence_->GetDivergenceSource(11));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(12));
  EXPECT_EQ(11, divergence_->GetDivergenceSource(12));
}

TEST_F(DivergenceTest, FlowTypesTest) {
  // pseudocode:
  //   %10:
  //   %11 = load x
  //   %12 = x < 0 // data -> data
  //   if (%12) {
  //     %13: // data -> control
  //     if (true) {
  //       %14: // control -> control
  //     }
  //     %15:
  //     %16 = 1
  //   } else {
  //     %17:
  //     %18 = 2
  //   }
  //   %19:
  //   %19 = phi(%16 from %15, %18 from %17) // control -> data
  //   return
  ASSERT_NO_FATAL_FAILURE(Build(Preamble() + R"(
         %10 = OpLabel
         %11 = OpLoad %float %x
         %12 = OpFOrdLessThan %bool %11 %zero
               OpSelectionMerge %19 None
               OpBranchConditional %12 %13 %17
         %13 = OpLabel
               OpSelectionMerge %15 None
               OpBranchConditional %true %14 %15
         %14 = OpLabel
               OpBranch %15
         %15 = OpLabel
         %16 = OpFAdd %float %zero %zero
               OpBranch %19
         %17 = OpLabel
         %18 = OpFAdd %float %zero %one
               OpBranch %19
         %19 = OpLabel
         %20 = OpPhi %float %16 %15 %18 %17
               OpReturn
               OpFunctionEnd
  )"));
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(10));

  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(11));
  EXPECT_EQ(0, divergence_->GetDivergenceSource(11));

  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(12));
  EXPECT_EQ(11, divergence_->GetDivergenceSource(12));

  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(13));
  EXPECT_EQ(12, divergence_->GetDivergenceSource(13));
  EXPECT_EQ(10, divergence_->GetDivergenceDependenceSource(13));

  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(14));
  EXPECT_EQ(13, divergence_->GetDivergenceSource(14));

  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(15));
  EXPECT_EQ(12, divergence_->GetDivergenceSource(15));
  EXPECT_EQ(10, divergence_->GetDivergenceDependenceSource(15));

  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(16));

  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(17));
  EXPECT_EQ(12, divergence_->GetDivergenceSource(17));
  EXPECT_EQ(10, divergence_->GetDivergenceDependenceSource(17));

  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(18));

  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(19));

  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(20));
  EXPECT_TRUE(divergence_->GetDivergenceSource(20) == 15 ||
              divergence_->GetDivergenceDependenceSource(20) == 17)
      << "Got: " << divergence_->GetDivergenceDependenceSource(20);
}

TEST_F(DivergenceTest, ExitDependenceTest) {
  // pseudocode:
  //   %10:
  //   %11 = load x
  //   %12 = %11 < 0
  //   %13:
  //   do {
  //     %14:
  //     if (%12) {
  //       %15:
  //       continue;
  //     }
  //     %16:
  //     %17:
  //     continue;
  //   } %18: while(false);
  //   %19:
  //   return
  ASSERT_NO_FATAL_FAILURE(Build(Preamble() + R"(
         %10 = OpLabel
         %11 = OpLoad %float %x
         %12 = OpFOrdLessThan %bool %11 %zero ; data -> data
               OpBranch %13
         %13 = OpLabel
               OpLoopMerge %19 %18 None
               OpBranch %14
         %14 = OpLabel
               OpSelectionMerge %16 None
               OpBranchConditional %12 %15 %16
         %15 = OpLabel
               OpBranch %18  ; continue
         %16 = OpLabel
               OpBranch %17
         %17 = OpLabel
               OpBranch %18  ; continue
         %18 = OpLabel
               OpBranchConditional %false %13 %19
         %19 = OpLabel
               OpReturn
               OpFunctionEnd
  )"));

  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(10));

  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(11));
  EXPECT_EQ(0, divergence_->GetDivergenceSource(11));

  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(12));
  EXPECT_EQ(11, divergence_->GetDivergenceSource(12));

  // Since both branches continue, there's no divergent control dependence
  // to 13.
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(13));

  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(14));

  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(15));
  EXPECT_EQ(12, divergence_->GetDivergenceSource(15));
  EXPECT_EQ(14, divergence_->GetDivergenceDependenceSource(15));

  // These two blocks are outside the if but are still control dependent.
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(16));
  EXPECT_EQ(12, divergence_->GetDivergenceSource(16));
  EXPECT_EQ(14, divergence_->GetDivergenceDependenceSource(16));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(17));
  EXPECT_EQ(12, divergence_->GetDivergenceSource(17));
  EXPECT_EQ(14, divergence_->GetDivergenceDependenceSource(17));

  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(18));

  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(19));
}

TEST_F(DivergenceTest, ReconvergencePromotionTest) {
  // pseudocode:
  // %10:
  // %11 = load y
  // %12 = %11 < 0
  // if (%12) {
  //   %13:
  //   %14:
  //   %15:
  //   if (true) {
  //     %16:
  //   }
  //   // Reconvergence *not* guaranteed as
  //   // control is not uniform on the IG level
  //   // at %15.
  //   %17:
  //   %18:
  //   %19:
  //   %20 = load x
  // }
  // %21:
  // %22 = phi(%11, %20)
  // return
  ASSERT_NO_FATAL_FAILURE(Build(Preamble() + R"(
         %10 = OpLabel
         %11 = OpLoad %float %y
         %12 = OpFOrdLessThan %bool %11 %zero
               OpSelectionMerge %21 None
               OpBranchConditional %12 %13 %21
         %13 = OpLabel
               OpBranch %14
         %14 = OpLabel
               OpBranch %15
         %15 = OpLabel
               OpSelectionMerge %17 None
               OpBranchConditional %true %16 %17
         %16 = OpLabel
               OpBranch %17
         %17 = OpLabel
               OpBranch %18
         %18 = OpLabel
               OpBranch %19
         %19 = OpLabel
         %20 = OpLoad %float %y
               OpBranch %21
         %21 = OpLabel
         %22 = OpPhi %float %11 %10 %20 %19
               OpReturn
               OpFunctionEnd
  )"));
  ASSERT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(10));
  ASSERT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(21));

  ASSERT_EQ(Level::kPartiallyUniform, divergence_->GetDivergenceLevel(11));
  ASSERT_EQ(0, divergence_->GetDivergenceSource(11));
  ASSERT_EQ(Level::kPartiallyUniform, divergence_->GetDivergenceLevel(12));
  ASSERT_EQ(11, divergence_->GetDivergenceSource(12));
  ASSERT_EQ(Level::kPartiallyUniform, divergence_->GetDivergenceLevel(13));
  ASSERT_EQ(12, divergence_->GetDivergenceSource(13));
  ASSERT_EQ(10, divergence_->GetDivergenceDependenceSource(13));
  ASSERT_EQ(Level::kPartiallyUniform, divergence_->GetDivergenceLevel(14));
  ASSERT_EQ(12, divergence_->GetDivergenceSource(14));
  ASSERT_EQ(10, divergence_->GetDivergenceDependenceSource(14));
  ASSERT_EQ(Level::kPartiallyUniform, divergence_->GetDivergenceLevel(15));
  ASSERT_EQ(12, divergence_->GetDivergenceSource(15));
  ASSERT_EQ(10, divergence_->GetDivergenceDependenceSource(15));
  ASSERT_EQ(Level::kPartiallyUniform, divergence_->GetDivergenceLevel(16));
  ASSERT_EQ(15, divergence_->GetDivergenceSource(16));

  ASSERT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(17));
  ASSERT_EQ(12, divergence_->GetDivergenceSource(17));
  ASSERT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(18));
  ASSERT_EQ(12, divergence_->GetDivergenceSource(18));
  ASSERT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(19));
  ASSERT_EQ(12, divergence_->GetDivergenceSource(19));

  ASSERT_EQ(Level::kPartiallyUniform, divergence_->GetDivergenceLevel(20));
  ASSERT_EQ(0, divergence_->GetDivergenceSource(20));
  ASSERT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(22));
  ASSERT_EQ(19, divergence_->GetDivergenceSource(22));
  ASSERT_EQ(10, divergence_->GetDivergenceDependenceSource(15));
}

TEST_F(DivergenceTest, FunctionCallTest) {
  // pseudocode:
  // %2() {
  //   %20:
  //   %21 = load x
  //   %22 = %21 < 0
  //   if (%22) {
  //     %23:
  //     return
  //   }
  //   %24:
  //   return
  // }
  //
  // main() {
  //   %10:
  //   %11 = %2();
  //   // Reconvergence *not* guaranteed.
  //   %12:
  //   return
  // }
  ASSERT_NO_FATAL_FAILURE(Build(Preamble() + R"(
         %10 = OpLabel
         %11 = OpFunctionCall %void %2
               OpBranch %12
         %12 = OpLabel
               OpReturn
               OpFunctionEnd

          %2 = OpFunction %void None %void_f
         %20 = OpLabel
         %21 = OpLoad %float %x
         %22 = OpFOrdLessThan %bool %21 %zero
               OpSelectionMerge %24 None
               OpBranchConditional %22 %23 %24
         %23 = OpLabel
               OpReturn
         %24 = OpLabel
               OpReturn
               OpFunctionEnd
  )"));

  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(10));
  // Conservatively assume function return value is uniform.
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(11));
  // TODO(dongja): blocks reachable from diverging function calls should be
  // divergent.
  // EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(12));
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(12));  // Wrong!
}

TEST_F(DivergenceTest, LateMergeTest) {
  // pseudocode:
  // %10:
  // %11 = load y
  // %12 = %11 < 0
  // [merge: %15]
  // if (%12) {
  //   %13:
  // }
  // %14: // Reconvergence hasn't happened by here.
  // %15:
  // return
  ASSERT_NO_FATAL_FAILURE(Build(Preamble() + R"(
         %10 = OpLabel
         %11 = OpLoad %float %x
         %12 = OpFOrdLessThan %bool %11 %zero
               OpSelectionMerge %15 None
               OpBranchConditional %12 %13 %14
         %13 = OpLabel
               OpBranch %14
         %14 = OpLabel
               OpBranch %15
         %15 = OpLabel
               OpReturn
               OpFunctionEnd
  )"));

  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(10));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(11));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(12));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(13));
  // TODO(dongja):
  // EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(14));
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(14));  // Wrong!
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(15));
}

// The following series of tests makes sure that we find the least fixpoint.
TEST_F(DivergenceTest, UniformFixpointTest) {
  // pseudocode:
  //   %10:
  //   %20 = load x
  //   %21 = load y
  //   do {
  //     %11:
  //     %12:
  //     %13 = phi(%zero from %11, %14 from %16)
  //     %14 = %13 + 1
  //     %15 = %13 < 1
  //   } %16: while (%15)
  //   %17:
  ASSERT_NO_FATAL_FAILURE(Build(Preamble() + R"(
         %10 = OpLabel
         %20 = OpLoad %float %x
         %21 = OpLoad %float %y
               OpBranch %11
         %11 = OpLabel
         %13 = OpPhi %float %zero %10 %14 %16
               OpLoopMerge %17 %16 None
               OpBranch %12
         %12 = OpLabel
         %14 = OpFAdd %float %13 %one
         %15 = OpFOrdLessThan %bool %13 %one
               OpBranch %16
         %16 = OpLabel
               OpBranchConditional %15 %11 %17
         %17 = OpLabel
               OpReturn
               OpFunctionEnd
  )"));

  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(10));
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(11));
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(12));
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(13));
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(14));
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(15));
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(16));
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(17));
}

TEST_F(DivergenceTest, PartiallyUniformFixpointTest) {
  // pseudocode:
  //   %10:
  //   %20 = load x
  //   %21 = load y
  //   do {
  //     %11:
  //     %12:
  //     %13 = phi(%zero from %11, %14 from %16)
  //     %14 = %13 + 1
  //     %15 = %13 < %21
  //   } %16: while (%15)
  //   %17:
  ASSERT_NO_FATAL_FAILURE(Build(Preamble() + R"(
         %10 = OpLabel
         %20 = OpLoad %float %x
         %21 = OpLoad %float %y
               OpBranch %11
         %11 = OpLabel
         %13 = OpPhi %float %zero %10 %14 %16
               OpLoopMerge %17 %16 None
               OpBranch %12
         %12 = OpLabel
         %14 = OpFAdd %float %13 %one
         %15 = OpFOrdLessThan %bool %13 %21
               OpBranch %16
         %16 = OpLabel
               OpBranchConditional %15 %11 %17
         %17 = OpLabel
               OpReturn
               OpFunctionEnd
  )"));

  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(10));
  EXPECT_EQ(Level::kPartiallyUniform, divergence_->GetDivergenceLevel(11));
  EXPECT_EQ(Level::kPartiallyUniform, divergence_->GetDivergenceLevel(12));
  EXPECT_EQ(Level::kPartiallyUniform, divergence_->GetDivergenceLevel(13));
  EXPECT_EQ(Level::kPartiallyUniform, divergence_->GetDivergenceLevel(14));
  EXPECT_EQ(Level::kPartiallyUniform, divergence_->GetDivergenceLevel(15));
  EXPECT_EQ(Level::kPartiallyUniform, divergence_->GetDivergenceLevel(16));
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(17));
}

TEST_F(DivergenceTest, DivergentFixpointTest) {
  // pseudocode:
  //   %10:
  //   %20 = load x
  //   %21 = load y
  //   do {
  //     %11:
  //     %12:
  //     %13 = phi(%zero from %11, %14 from %16)
  //     %14 = %13 + 1
  //     %15 = %13 < %20
  //   } %16: while (%15)
  //   %17:
  ASSERT_NO_FATAL_FAILURE(Build(Preamble() + R"(
         %10 = OpLabel
         %20 = OpLoad %float %x
         %21 = OpLoad %float %y
               OpBranch %11
         %11 = OpLabel
         %13 = OpPhi %float %zero %10 %14 %16
               OpLoopMerge %17 %16 None
               OpBranch %12
         %12 = OpLabel
         %14 = OpFAdd %float %13 %one
         %15 = OpFOrdLessThan %bool %13 %20
               OpBranch %16
         %16 = OpLabel
               OpBranchConditional %15 %11 %17
         %17 = OpLabel
               OpReturn
               OpFunctionEnd
  )"));

  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(10));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(11));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(12));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(13));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(14));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(15));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(16));
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(17));
}

TEST_F(DivergenceTest, DivergentOverridesPartiallyUniformTest) {
  // pseudocode:
  //   %10:
  //   %20 = load x
  //   %21 = load y
  //   %11:
  //   do {
  //     %12:
  //     %13 = phi(%21 from %11, %14 from %16)
  //     %14 = %13 + 1
  //     %15 = %13 < %20
  //   } %16: while (%15)
  //   %17:
  ASSERT_NO_FATAL_FAILURE(Build(Preamble() + R"(
         %10 = OpLabel
         %20 = OpLoad %float %x
         %21 = OpLoad %float %y
               OpBranch %11
         %11 = OpLabel
         %13 = OpPhi %float %zero %10 %14 %16
               OpLoopMerge %17 %16 None
               OpBranch %12
         %12 = OpLabel
         %14 = OpFAdd %float %13 %one
         %15 = OpFOrdLessThan %bool %13 %20
               OpBranch %16
         %16 = OpLabel
               OpBranchConditional %15 %11 %17
         %17 = OpLabel
               OpReturn
               OpFunctionEnd
  )"));

  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(10));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(11));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(12));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(13));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(14));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(15));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(16));
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(17));
}

TEST_F(DivergenceTest, NestedFixpointTest) {
  // pseudocode:
  //   %10:
  //   %20 = load x
  //   %21 = load y
  //   do {
  //     %22:
  //     %23:
  //     %24 = phi(%zero from %22, %25 from %26)
  //     %11:
  //     do {
  //       %12:
  //       %13 = phi(%zero from %11, %14 from %16)
  //       %14 = %13 + 1
  //       %15 = %13 < %24
  //     } %16: while (%15)
  //     %17:
  //     %25 = load x
  //   } %26: while (false)
  //   %27:
  //   return
  ASSERT_NO_FATAL_FAILURE(Build(Preamble() + R"(
         %10 = OpLabel
         %20 = OpLoad %float %x
         %21 = OpLoad %float %y
               OpBranch %22
         %22 = OpLabel
         %24 = OpPhi %float %zero %10 %25 %26
               OpLoopMerge %27 %26 None
               OpBranch %23
         %23 = OpLabel
               OpBranch %11
         %11 = OpLabel
         %13 = OpPhi %float %zero %23 %14 %16
               OpLoopMerge %17 %16 None
               OpBranch %12
         %12 = OpLabel
         %14 = OpFAdd %float %13 %one
         %15 = OpFOrdLessThan %bool %13 %24
               OpBranch %16
         %16 = OpLabel
               OpBranchConditional %15 %11 %17
         %17 = OpLabel
         %25 = OpLoad %float %x
               OpBranch %26
         %26 = OpLabel
               OpBranchConditional %false %22 %27
         %27 = OpLabel
               OpReturn
               OpFunctionEnd
  )"));
  // This test makes sure that divergent values flowing upward can influence the
  // fixpoint of a loop.
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(10));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(11));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(12));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(13));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(14));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(15));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(16));
  // Control of the outer loop is still uniform.
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(17));
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(22));
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(23));
  // Seed divergent values.
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(24));
  EXPECT_EQ(Level::kDivergent, divergence_->GetDivergenceLevel(25));
  // Outer loop control.
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(26));
  // Merged.
  EXPECT_EQ(Level::kUniform, divergence_->GetDivergenceLevel(27));
}

}  // namespace
}  // namespace lint
}  // namespace spvtools
