// Copyright (c) 2018 Google LLC
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

#include <memory>
#include <vector>

#include "function_utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "source/opt/build_module.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace opt {
namespace {

using ::testing::Eq;

TEST(FunctionTest, HasEarlyReturn) {
  std::string shader = R"(
          OpCapability Shader
     %1 = OpExtInstImport "GLSL.std.450"
          OpMemoryModel Logical GLSL450
          OpEntryPoint Vertex %6 "main"

; Types
     %2 = OpTypeBool
     %3 = OpTypeVoid
     %4 = OpTypeFunction %3

; Constants
     %5 = OpConstantTrue %2

; main function without early return
     %6 = OpFunction %3 None %4
     %7 = OpLabel
          OpBranch %8
     %8 = OpLabel
          OpBranch %9
     %9 = OpLabel
          OpBranch %10
    %10 = OpLabel
          OpReturn
          OpFunctionEnd

; function with early return
    %11 = OpFunction %3 None %4
    %12 = OpLabel
          OpSelectionMerge %15 None
          OpBranchConditional %5 %13 %14
    %13 = OpLabel
          OpReturn
    %14 = OpLabel
          OpBranch %15
    %15 = OpLabel
          OpReturn
          OpFunctionEnd
  )";

  const auto context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, shader,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  // Tests |function| without early return.
  auto* function = spvtest::GetFunction(context->module(), 6);
  ASSERT_FALSE(function->HasEarlyReturn());

  // Tests |function| with early return.
  function = spvtest::GetFunction(context->module(), 11);
  ASSERT_TRUE(function->HasEarlyReturn());
}

TEST(FunctionTest, IsNotRecursive) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "main"
OpExecutionMode %1 OriginUpperLeft
OpDecorate %2 DescriptorSet 439418829
%void = OpTypeVoid
%4 = OpTypeFunction %void
%float = OpTypeFloat 32
%_struct_6 = OpTypeStruct %float %float
%7 = OpTypeFunction %_struct_6
%1 = OpFunction %void Pure|Const %4
%8 = OpLabel
%2 = OpFunctionCall %_struct_6 %9
OpKill
OpFunctionEnd
%9 = OpFunction %_struct_6 None %7
%10 = OpLabel
%11 = OpFunctionCall %_struct_6 %12
OpUnreachable
OpFunctionEnd
%12 = OpFunction %_struct_6 None %7
%13 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  std::unique_ptr<IRContext> ctx =
      spvtools::BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                            SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  auto* func = spvtest::GetFunction(ctx->module(), 9);
  EXPECT_FALSE(func->IsRecursive());

  func = spvtest::GetFunction(ctx->module(), 12);
  EXPECT_FALSE(func->IsRecursive());
}

TEST(FunctionTest, IsDirectlyRecursive) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "main"
OpExecutionMode %1 OriginUpperLeft
OpDecorate %2 DescriptorSet 439418829
%void = OpTypeVoid
%4 = OpTypeFunction %void
%float = OpTypeFloat 32
%_struct_6 = OpTypeStruct %float %float
%7 = OpTypeFunction %_struct_6
%1 = OpFunction %void Pure|Const %4
%8 = OpLabel
%2 = OpFunctionCall %_struct_6 %9
OpKill
OpFunctionEnd
%9 = OpFunction %_struct_6 None %7
%10 = OpLabel
%11 = OpFunctionCall %_struct_6 %9
OpUnreachable
OpFunctionEnd
)";

  std::unique_ptr<IRContext> ctx =
      spvtools::BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                            SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  auto* func = spvtest::GetFunction(ctx->module(), 9);
  EXPECT_TRUE(func->IsRecursive());
}

TEST(FunctionTest, IsIndirectlyRecursive) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "main"
OpExecutionMode %1 OriginUpperLeft
OpDecorate %2 DescriptorSet 439418829
%void = OpTypeVoid
%4 = OpTypeFunction %void
%float = OpTypeFloat 32
%_struct_6 = OpTypeStruct %float %float
%7 = OpTypeFunction %_struct_6
%1 = OpFunction %void Pure|Const %4
%8 = OpLabel
%2 = OpFunctionCall %_struct_6 %9
OpKill
OpFunctionEnd
%9 = OpFunction %_struct_6 None %7
%10 = OpLabel
%11 = OpFunctionCall %_struct_6 %12
OpUnreachable
OpFunctionEnd
%12 = OpFunction %_struct_6 None %7
%13 = OpLabel
%14 = OpFunctionCall %_struct_6 %9
OpUnreachable
OpFunctionEnd
)";

  std::unique_ptr<IRContext> ctx =
      spvtools::BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                            SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  auto* func = spvtest::GetFunction(ctx->module(), 9);
  EXPECT_TRUE(func->IsRecursive());

  func = spvtest::GetFunction(ctx->module(), 12);
  EXPECT_TRUE(func->IsRecursive());
}

TEST(FunctionTest, IsNotRecuriseCallingRecursive) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "main"
OpExecutionMode %1 OriginUpperLeft
OpDecorate %2 DescriptorSet 439418829
%void = OpTypeVoid
%4 = OpTypeFunction %void
%float = OpTypeFloat 32
%_struct_6 = OpTypeStruct %float %float
%7 = OpTypeFunction %_struct_6
%1 = OpFunction %void Pure|Const %4
%8 = OpLabel
%2 = OpFunctionCall %_struct_6 %9
OpKill
OpFunctionEnd
%9 = OpFunction %_struct_6 None %7
%10 = OpLabel
%11 = OpFunctionCall %_struct_6 %9
OpUnreachable
OpFunctionEnd
)";

  std::unique_ptr<IRContext> ctx =
      spvtools::BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                            SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  auto* func = spvtest::GetFunction(ctx->module(), 1);
  EXPECT_FALSE(func->IsRecursive());
}

TEST(FunctionTest, NonSemanticInfoSkipIteration) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpExtension "SPV_KHR_non_semantic_info"
%1 = OpExtInstImport "NonSemantic.Test"
OpMemoryModel Logical GLSL450
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%4 = OpFunction %2 None %3
%5 = OpLabel
%6 = OpExtInst %2 %1 1
OpReturn
OpFunctionEnd
%7 = OpExtInst %2 %1 2
%8 = OpExtInst %2 %1 3
)";

  std::unique_ptr<IRContext> ctx =
      spvtools::BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                            SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  auto* func = spvtest::GetFunction(ctx->module(), 4);
  ASSERT_TRUE(func != nullptr);
  std::unordered_set<uint32_t> non_semantic_ids;
  func->ForEachInst(
      [&non_semantic_ids](const Instruction* inst) {
        if (inst->opcode() == spv::Op::OpExtInst) {
          non_semantic_ids.insert(inst->result_id());
        }
      },
      true, false);

  EXPECT_EQ(1, non_semantic_ids.count(6));
  EXPECT_EQ(0, non_semantic_ids.count(7));
  EXPECT_EQ(0, non_semantic_ids.count(8));
}

TEST(FunctionTest, NonSemanticInfoIncludeIteration) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpExtension "SPV_KHR_non_semantic_info"
%1 = OpExtInstImport "NonSemantic.Test"
OpMemoryModel Logical GLSL450
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%4 = OpFunction %2 None %3
%5 = OpLabel
%6 = OpExtInst %2 %1 1
OpReturn
OpFunctionEnd
%7 = OpExtInst %2 %1 2
%8 = OpExtInst %2 %1 3
)";

  std::unique_ptr<IRContext> ctx =
      spvtools::BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                            SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  auto* func = spvtest::GetFunction(ctx->module(), 4);
  ASSERT_TRUE(func != nullptr);
  std::unordered_set<uint32_t> non_semantic_ids;
  func->ForEachInst(
      [&non_semantic_ids](const Instruction* inst) {
        if (inst->opcode() == spv::Op::OpExtInst) {
          non_semantic_ids.insert(inst->result_id());
        }
      },
      true, true);

  EXPECT_EQ(1, non_semantic_ids.count(6));
  EXPECT_EQ(1, non_semantic_ids.count(7));
  EXPECT_EQ(1, non_semantic_ids.count(8));
}

TEST(FunctionTest, ReorderBlocksinStructuredOrder) {
  // The spir-v has the basic block in a random order.  We want to reorder them
  // in structured order.
  const std::string text = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %100 "PSMain"
               OpExecutionMode %PSMain OriginUpperLeft
               OpSource HLSL 600
        %int = OpTypeInt 32 1
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
       %bool = OpTypeBool
%undef_bool = OpUndef %bool
%undef_int = OpUndef %int
        %100 = OpFunction %void None %19
          %11 = OpLabel
               OpSelectionMerge %10 None
               OpSwitch %undef_int %3 0 %2 10 %1
          %2 = OpLabel
               OpReturn
          %7 = OpLabel
               OpBranch %8
          %3 = OpLabel
               OpBranch %4
         %10 = OpLabel
               OpReturn
          %9 = OpLabel
               OpBranch %10
          %8 = OpLabel
               OpBranch %4
          %4 = OpLabel
               OpLoopMerge %9 %8 None
               OpBranchConditional %undef_bool %5 %9
          %1 = OpLabel
               OpReturn
          %6 = OpLabel
               OpBranch %7
          %5 = OpLabel
               OpSelectionMerge %7 None
               OpBranchConditional %undef_bool %6 %7
               OpFunctionEnd
)";

  std::unique_ptr<IRContext> ctx =
      spvtools::BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                            SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_TRUE(ctx);
  auto* func = spvtest::GetFunction(ctx->module(), 100);
  ASSERT_TRUE(func);
  func->ReorderBasicBlocksInStructuredOrder();

  auto first_block = func->begin();
  auto bb = first_block;
  for (++bb; bb != func->end(); ++bb) {
    EXPECT_EQ(bb->id(), (bb - first_block));
  }
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
