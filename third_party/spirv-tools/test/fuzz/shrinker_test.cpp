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

#include "source/fuzz/shrinker.h"

#include "gtest/gtest.h"
#include "source/fuzz/fact_manager/fact_manager.h"
#include "source/fuzz/fuzzer_context.h"
#include "source/fuzz/fuzzer_pass_donate_modules.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/pseudo_random_generator.h"
#include "source/fuzz/transformation_context.h"
#include "source/opt/ir_context.h"
#include "source/util/make_unique.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(ShrinkerTest, ReduceAddedFunctions) {
  const std::string kReferenceModule = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Private %6
          %8 = OpVariable %7 Private
          %9 = OpConstant %6 2
         %10 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
               OpStore %8 %9
         %12 = OpLoad %6 %8
               OpStore %11 %12
               OpReturn
               OpFunctionEnd
  )";

  const std::string kDonorModule = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %6 %7
         %12 = OpTypeFunction %2 %7
         %17 = OpConstant %6 0
         %26 = OpTypeBool
         %32 = OpConstant %6 1
         %46 = OpTypePointer Private %6
         %47 = OpVariable %46 Private
         %48 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %49 = OpVariable %7 Function
         %50 = OpVariable %7 Function
         %51 = OpLoad %6 %49
               OpStore %50 %51
         %52 = OpFunctionCall %2 %14 %50
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %6 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %16 = OpVariable %7 Function
         %18 = OpVariable %7 Function
               OpStore %16 %17
               OpStore %18 %17
               OpBranch %19
         %19 = OpLabel
               OpLoopMerge %21 %22 None
               OpBranch %23
         %23 = OpLabel
         %24 = OpLoad %6 %18
         %25 = OpLoad %6 %9
         %27 = OpSLessThan %26 %24 %25
               OpBranchConditional %27 %20 %21
         %20 = OpLabel
         %28 = OpLoad %6 %9
         %29 = OpLoad %6 %16
         %30 = OpIAdd %6 %29 %28
               OpStore %16 %30
               OpBranch %22
         %22 = OpLabel
         %31 = OpLoad %6 %18
         %33 = OpIAdd %6 %31 %32
               OpStore %18 %33
               OpBranch %19
         %21 = OpLabel
         %34 = OpLoad %6 %16
         %35 = OpNot %6 %34
               OpReturnValue %35
               OpFunctionEnd
         %14 = OpFunction %2 None %12
         %13 = OpFunctionParameter %7
         %15 = OpLabel
         %37 = OpVariable %7 Function
         %38 = OpVariable %7 Function
         %39 = OpLoad %6 %13
               OpStore %38 %39
         %40 = OpFunctionCall %6 %10 %38
               OpStore %37 %40
         %41 = OpLoad %6 %37
         %42 = OpLoad %6 %13
         %43 = OpSGreaterThan %26 %41 %42
               OpSelectionMerge %45 None
               OpBranchConditional %43 %44 %45
         %44 = OpLabel
               OpStore %47 %48
               OpBranch %45
         %45 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  // Note: |env| should ideally be declared const.  However, due to a known
  // issue with older versions of MSVC we would have to mark |env| as being
  // captured due to its used in a lambda below, and other compilers would warn
  // that such capturing is not necessary.  Not declaring |env| as const means
  // that it needs to be captured to be used in the lambda, and thus all
  // compilers are kept happy.  See:
  // https://developercommunity.visualstudio.com/content/problem/367326/problems-with-capturing-constexpr-in-lambda.html
  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = fuzzerutil::kSilentMessageConsumer;

  SpirvTools tools(env);
  std::vector<uint32_t> reference_binary;
  ASSERT_TRUE(
      tools.Assemble(kReferenceModule, &reference_binary, kFuzzAssembleOption));

  spvtools::ValidatorOptions validator_options;

  const auto variant_ir_context =
      BuildModule(env, consumer, kReferenceModule, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      variant_ir_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_ir_context =
      BuildModule(env, consumer, kDonorModule, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_ir_context.get(), validator_options, kConsoleMessageConsumer));

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  TransformationContext transformation_context(
      MakeUnique<FactManager>(variant_ir_context.get()), validator_options);

  protobufs::TransformationSequence transformations;
  FuzzerPassDonateModules pass(variant_ir_context.get(),
                               &transformation_context, &fuzzer_context,
                               &transformations, {});
  pass.DonateSingleModule(donor_ir_context.get(), true);

  protobufs::FactSequence no_facts;

  Shrinker::InterestingnessFunction interestingness_function =
      [consumer, env](const std::vector<uint32_t>& binary,
                      uint32_t /*unused*/) -> bool {
    bool found_op_not = false;
    uint32_t op_call_count = 0;
    auto temp_ir_context =
        BuildModule(env, consumer, binary.data(), binary.size());
    for (auto& function : *temp_ir_context->module()) {
      for (auto& block : function) {
        for (auto& inst : block) {
          if (inst.opcode() == SpvOpNot) {
            found_op_not = true;
          } else if (inst.opcode() == SpvOpFunctionCall) {
            op_call_count++;
          }
        }
      }
    }
    return found_op_not && op_call_count >= 2;
  };

  auto shrinker_result =
      Shrinker(env, consumer, reference_binary, no_facts, transformations,
               interestingness_function, 1000, true, validator_options)
          .Run();
  ASSERT_EQ(Shrinker::ShrinkerResultStatus::kComplete, shrinker_result.status);

  // We now check that the module after shrinking looks right.
  // The entry point should be identical to what it looked like in the
  // reference, while the other functions should be absolutely minimal,
  // containing only what is needed to satisfy the interestingness function.
  auto ir_context_after_shrinking =
      BuildModule(env, consumer, shrinker_result.transformed_binary.data(),
                  shrinker_result.transformed_binary.size());
  bool first_function = true;
  for (auto& function : *ir_context_after_shrinking->module()) {
    if (first_function) {
      first_function = false;
      bool first_block = true;
      for (auto& block : function) {
        ASSERT_TRUE(first_block);
        uint32_t counter = 0;
        for (auto& inst : block) {
          switch (counter) {
            case 0:
              ASSERT_EQ(SpvOpVariable, inst.opcode());
              ASSERT_EQ(11, inst.result_id());
              break;
            case 1:
              ASSERT_EQ(SpvOpStore, inst.opcode());
              break;
            case 2:
              ASSERT_EQ(SpvOpLoad, inst.opcode());
              ASSERT_EQ(12, inst.result_id());
              break;
            case 3:
              ASSERT_EQ(SpvOpStore, inst.opcode());
              break;
            case 4:
              ASSERT_EQ(SpvOpReturn, inst.opcode());
              break;
            default:
              FAIL();
          }
          counter++;
        }
      }
    } else {
      bool first_block = true;
      for (auto& block : function) {
        ASSERT_TRUE(first_block);
        first_block = false;
        for (auto& inst : block) {
          switch (inst.opcode()) {
            case SpvOpVariable:
            case SpvOpNot:
            case SpvOpReturn:
            case SpvOpReturnValue:
            case SpvOpFunctionCall:
              // These are the only instructions we expect to see.
              break;
            default:
              FAIL();
          }
        }
      }
    }
  }
}

TEST(ShrinkerTest, HitStepLimitWhenReducingAddedFunctions) {
  const std::string kReferenceModule = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Private %6
          %8 = OpVariable %7 Private
          %9 = OpConstant %6 2
         %10 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
               OpStore %8 %9
         %12 = OpLoad %6 %8
               OpStore %11 %12
               OpReturn
               OpFunctionEnd
  )";

  const std::string kDonorModule = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %48 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %52 = OpCopyObject %6 %48
         %53 = OpCopyObject %6 %52
         %54 = OpCopyObject %6 %53
         %55 = OpCopyObject %6 %54
         %56 = OpCopyObject %6 %55
         %57 = OpCopyObject %6 %56
         %58 = OpCopyObject %6 %48
         %59 = OpCopyObject %6 %58
         %60 = OpCopyObject %6 %59
         %61 = OpCopyObject %6 %60
         %62 = OpCopyObject %6 %61
         %63 = OpCopyObject %6 %62
         %64 = OpCopyObject %6 %48
               OpReturn
               OpFunctionEnd
  )";

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = fuzzerutil::kSilentMessageConsumer;

  SpirvTools tools(env);
  std::vector<uint32_t> reference_binary;
  ASSERT_TRUE(
      tools.Assemble(kReferenceModule, &reference_binary, kFuzzAssembleOption));

  spvtools::ValidatorOptions validator_options;

  const auto variant_ir_context =
      BuildModule(env, consumer, kReferenceModule, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      variant_ir_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_ir_context =
      BuildModule(env, consumer, kDonorModule, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_ir_context.get(), validator_options, kConsoleMessageConsumer));

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  TransformationContext transformation_context(
      MakeUnique<FactManager>(variant_ir_context.get()), validator_options);

  protobufs::TransformationSequence transformations;
  FuzzerPassDonateModules pass(variant_ir_context.get(),
                               &transformation_context, &fuzzer_context,
                               &transformations, {});
  pass.DonateSingleModule(donor_ir_context.get(), true);

  protobufs::FactSequence no_facts;

  Shrinker::InterestingnessFunction interestingness_function =
      [consumer, env](const std::vector<uint32_t>& binary,
                      uint32_t /*unused*/) -> bool {
    auto temp_ir_context =
        BuildModule(env, consumer, binary.data(), binary.size());
    uint32_t copy_object_count = 0;
    temp_ir_context->module()->ForEachInst(
        [&copy_object_count](opt::Instruction* inst) {
          if (inst->opcode() == SpvOpCopyObject) {
            copy_object_count++;
          }
        });
    return copy_object_count >= 8;
  };

  auto shrinker_result =
      Shrinker(env, consumer, reference_binary, no_facts, transformations,
               interestingness_function, 30, true, validator_options)
          .Run();
  ASSERT_EQ(Shrinker::ShrinkerResultStatus::kStepLimitReached,
            shrinker_result.status);
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
