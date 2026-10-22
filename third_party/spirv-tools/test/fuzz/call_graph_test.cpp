// Copyright (c) 2020 Google LLC
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

#include "source/fuzz/call_graph.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

// The SPIR-V came from this GLSL, slightly modified
// (main is %2, A is %35, B is %48, C is %50, D is %61):
//
//    #version 310 es
//
//    int A (int x) {
//      return x + 1;
//    }
//
//    void D() {
//    }
//
//    void C() {
//      int x = 0;
//      int y = 0;
//
//      while (x < 10) {
//        while (y < 10) {
//          y = A(y);
//        }
//        x = A(x);
//      }
//    }
//
//    void B () {
//      int x = 0;
//      int y = 0;
//
//      while (x < 10) {
//        D();
//        while (y < 10) {
//          y = A(y);
//          C();
//        }
//        x++;
//      }
//
//    }
//
//    void main()
//    {
//      int x = 0;
//      int y = 0;
//      int z = 0;
//
//      while (x < 10) {
//        while(y < 10) {
//          y = A(x);
//          while (z < 10) {
//            z = A(z);
//          }
//        }
//        x += 2;
//      }
//
//      B();
//      C();
//    }
std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypePointer Function %5
          %7 = OpTypeFunction %5 %6
          %8 = OpConstant %5 1
          %9 = OpConstant %5 0
         %10 = OpConstant %5 10
         %11 = OpTypeBool
         %12 = OpConstant %5 2
          %2 = OpFunction %3 None %4
         %13 = OpLabel
         %14 = OpVariable %6 Function
         %15 = OpVariable %6 Function
         %16 = OpVariable %6 Function
         %17 = OpVariable %6 Function
         %18 = OpVariable %6 Function
               OpStore %14 %9
               OpStore %15 %9
               OpStore %16 %9
               OpBranch %19
         %19 = OpLabel
               OpLoopMerge %20 %21 None
               OpBranch %22
         %22 = OpLabel
         %23 = OpLoad %5 %14
         %24 = OpSLessThan %11 %23 %10
               OpBranchConditional %24 %25 %20
         %25 = OpLabel
               OpBranch %26
         %26 = OpLabel
               OpLoopMerge %27 %28 None
               OpBranch %29
         %29 = OpLabel
         %30 = OpLoad %5 %15
         %31 = OpSLessThan %11 %30 %10
               OpBranchConditional %31 %32 %27
         %32 = OpLabel
         %33 = OpLoad %5 %14
               OpStore %17 %33
         %34 = OpFunctionCall %5 %35 %17
               OpStore %15 %34
               OpBranch %36
         %36 = OpLabel
               OpLoopMerge %37 %38 None
               OpBranch %39
         %39 = OpLabel
         %40 = OpLoad %5 %16
         %41 = OpSLessThan %11 %40 %10
               OpBranchConditional %41 %42 %37
         %42 = OpLabel
         %43 = OpLoad %5 %16
               OpStore %18 %43
         %44 = OpFunctionCall %5 %35 %18
               OpStore %16 %44
               OpBranch %38
         %38 = OpLabel
               OpBranch %36
         %37 = OpLabel
               OpBranch %28
         %28 = OpLabel
               OpBranch %26
         %27 = OpLabel
         %45 = OpLoad %5 %14
         %46 = OpIAdd %5 %45 %12
               OpStore %14 %46
               OpBranch %21
         %21 = OpLabel
               OpBranch %19
         %20 = OpLabel
         %47 = OpFunctionCall %3 %48
         %49 = OpFunctionCall %3 %50
               OpReturn
               OpFunctionEnd
         %35 = OpFunction %5 None %7
         %51 = OpFunctionParameter %6
         %52 = OpLabel
         %53 = OpLoad %5 %51
         %54 = OpIAdd %5 %53 %8
               OpReturnValue %54
               OpFunctionEnd
         %48 = OpFunction %3 None %4
         %55 = OpLabel
         %56 = OpVariable %6 Function
         %57 = OpVariable %6 Function
         %58 = OpVariable %6 Function
               OpStore %56 %9
               OpStore %57 %9
               OpBranch %59
         %59 = OpLabel
         %60 = OpFunctionCall %3 %61
               OpLoopMerge %62 %63 None
               OpBranch %64
         %64 = OpLabel
               OpLoopMerge %65 %66 None
               OpBranch %67
         %67 = OpLabel
         %68 = OpLoad %5 %57
         %69 = OpSLessThan %11 %68 %10
               OpBranchConditional %69 %70 %65
         %70 = OpLabel
         %71 = OpLoad %5 %57
               OpStore %58 %71
         %72 = OpFunctionCall %5 %35 %58
               OpStore %57 %72
         %73 = OpFunctionCall %3 %50
               OpBranch %66
         %66 = OpLabel
               OpBranch %64
         %65 = OpLabel
         %74 = OpLoad %5 %56
         %75 = OpIAdd %5 %74 %8
               OpStore %56 %75
               OpBranch %63
         %63 = OpLabel
         %76 = OpLoad %5 %56
         %77 = OpSLessThan %11 %76 %10
               OpBranchConditional %77 %59 %62
         %62 = OpLabel
               OpReturn
               OpFunctionEnd
         %50 = OpFunction %3 None %4
         %78 = OpLabel
         %79 = OpVariable %6 Function
         %80 = OpVariable %6 Function
         %81 = OpVariable %6 Function
         %82 = OpVariable %6 Function
               OpStore %79 %9
               OpStore %80 %9
               OpBranch %83
         %83 = OpLabel
               OpLoopMerge %84 %85 None
               OpBranch %86
         %86 = OpLabel
         %87 = OpLoad %5 %79
         %88 = OpSLessThan %11 %87 %10
               OpBranchConditional %88 %89 %84
         %89 = OpLabel
               OpBranch %90
         %90 = OpLabel
               OpLoopMerge %91 %92 None
               OpBranch %93
         %93 = OpLabel
         %94 = OpLoad %5 %80
         %95 = OpSLessThan %11 %94 %10
               OpBranchConditional %95 %96 %91
         %96 = OpLabel
         %97 = OpLoad %5 %80
               OpStore %81 %97
         %98 = OpFunctionCall %5 %35 %81
               OpStore %80 %98
               OpBranch %92
         %92 = OpLabel
               OpBranch %90
         %91 = OpLabel
         %99 = OpLoad %5 %79
               OpStore %82 %99
        %100 = OpFunctionCall %5 %35 %82
               OpStore %79 %100
               OpBranch %85
         %85 = OpLabel
               OpBranch %83
         %84 = OpLabel
               OpReturn
               OpFunctionEnd
         %61 = OpFunction %3 None %4
        %101 = OpLabel
               OpReturn
               OpFunctionEnd
)";

// We have that:
// main calls:
// - A (maximum loop nesting depth of function call: 3)
// - B (0)
// - C (0)
// A calls nothing.
// B calls:
// - A (2)
// - C (2)
// - D (1)
// C calls:
// - A (2)
// D calls nothing.

TEST(CallGraphTest, FunctionInDegree) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;

  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  const auto graph = CallGraph(context.get());

  const auto& function_in_degree = graph.GetFunctionInDegree();
  // Check the in-degrees of, in order: main, A, B, C, D.
  ASSERT_EQ(function_in_degree.at(2), 0);
  ASSERT_EQ(function_in_degree.at(35), 3);
  ASSERT_EQ(function_in_degree.at(48), 1);
  ASSERT_EQ(function_in_degree.at(50), 2);
  ASSERT_EQ(function_in_degree.at(61), 1);
}

TEST(CallGraphTest, DirectCallees) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;

  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  const auto graph = CallGraph(context.get());

  // Check the callee sets of, in order: main, A, B, C, D.
  ASSERT_EQ(graph.GetDirectCallees(2), std::set<uint32_t>({35, 48, 50}));
  ASSERT_EQ(graph.GetDirectCallees(35), std::set<uint32_t>({}));
  ASSERT_EQ(graph.GetDirectCallees(48), std::set<uint32_t>({35, 50, 61}));
  ASSERT_EQ(graph.GetDirectCallees(50), std::set<uint32_t>({35}));
  ASSERT_EQ(graph.GetDirectCallees(61), std::set<uint32_t>({}));
}

TEST(CallGraphTest, IndirectCallees) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;

  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  const auto graph = CallGraph(context.get());

  // Check the callee sets of, in order: main, A, B, C, D.
  ASSERT_EQ(graph.GetIndirectCallees(2), std::set<uint32_t>({35, 48, 50, 61}));
  ASSERT_EQ(graph.GetDirectCallees(35), std::set<uint32_t>({}));
  ASSERT_EQ(graph.GetDirectCallees(48), std::set<uint32_t>({35, 50, 61}));
  ASSERT_EQ(graph.GetDirectCallees(50), std::set<uint32_t>({35}));
  ASSERT_EQ(graph.GetDirectCallees(61), std::set<uint32_t>({}));
}

TEST(CallGraphTest, TopologicalOrder) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;

  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  const auto graph = CallGraph(context.get());

  const auto& topological_ordering = graph.GetFunctionsInTopologicalOrder();

  // The possible topological orderings are:
  //  - main, B, D, C, A
  //  - main, B, C, D, A
  //  - main, B, C, A, D
  ASSERT_TRUE(
      topological_ordering == std::vector<uint32_t>({2, 48, 61, 50, 35}) ||
      topological_ordering == std::vector<uint32_t>({2, 48, 50, 61, 35}) ||
      topological_ordering == std::vector<uint32_t>({2, 48, 50, 35, 61}));
}

TEST(CallGraphTest, LoopNestingDepth) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;

  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  const auto graph = CallGraph(context.get());

  // Check the maximum loop nesting depth for function calls to, in order:
  // main, A, B, C, D
  ASSERT_EQ(graph.GetMaxCallNestingDepth(2), 0);
  ASSERT_EQ(graph.GetMaxCallNestingDepth(35), 4);
  ASSERT_EQ(graph.GetMaxCallNestingDepth(48), 0);
  ASSERT_EQ(graph.GetMaxCallNestingDepth(50), 2);
  ASSERT_EQ(graph.GetMaxCallNestingDepth(61), 1);
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
