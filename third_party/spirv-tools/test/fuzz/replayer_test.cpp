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

#include "source/fuzz/replayer.h"

#include "gtest/gtest.h"
#include "source/fuzz/data_descriptor.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "source/fuzz/transformation_add_constant_scalar.h"
#include "source/fuzz/transformation_add_global_variable.h"
#include "source/fuzz/transformation_add_parameter.h"
#include "source/fuzz/transformation_add_synonym.h"
#include "source/fuzz/transformation_flatten_conditional_branch.h"
#include "source/fuzz/transformation_split_block.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(ReplayerTest, PartialReplay) {
  const std::string kTestShader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "g"
               OpName %11 "x"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Private %6
          %8 = OpVariable %7 Private
          %9 = OpConstant %6 10
         %10 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
               OpStore %8 %9
         %12 = OpLoad %6 %8
               OpStore %11 %12
         %13 = OpLoad %6 %8
               OpStore %11 %13
         %14 = OpLoad %6 %8
               OpStore %11 %14
         %15 = OpLoad %6 %8
               OpStore %11 %15
         %16 = OpLoad %6 %8
               OpStore %11 %16
         %17 = OpLoad %6 %8
               OpStore %11 %17
         %18 = OpLoad %6 %8
               OpStore %11 %18
         %19 = OpLoad %6 %8
               OpStore %11 %19
         %20 = OpLoad %6 %8
               OpStore %11 %20
         %21 = OpLoad %6 %8
               OpStore %11 %21
         %22 = OpLoad %6 %8
               OpStore %11 %22
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  spvtools::ValidatorOptions validator_options;

  std::vector<uint32_t> binary_in;
  SpirvTools t(env);
  t.SetMessageConsumer(kConsoleMessageConsumer);
  ASSERT_TRUE(t.Assemble(kTestShader, &binary_in, kFuzzAssembleOption));
  ASSERT_TRUE(t.Validate(binary_in));

  protobufs::TransformationSequence transformations;
  for (uint32_t id = 12; id <= 22; id++) {
    *transformations.add_transformation() =
        TransformationSplitBlock(
            MakeInstructionDescriptor(id, spv::Op::OpLoad, 0), id + 100)
            .ToMessage();
  }

  {
    // Full replay
    protobufs::FactSequence empty_facts;
    auto replayer_result =
        Replayer(env, kConsoleMessageConsumer, binary_in, empty_facts,
                 transformations, 11, true, validator_options)
            .Run();
    // Replay should succeed.
    ASSERT_EQ(Replayer::ReplayerResultStatus::kComplete,
              replayer_result.status);
    // All transformations should be applied.
    ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(
        transformations, replayer_result.applied_transformations));

    const std::string kFullySplitShader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "g"
               OpName %11 "x"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Private %6
          %8 = OpVariable %7 Private
          %9 = OpConstant %6 10
         %10 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
               OpStore %8 %9
               OpBranch %112
        %112 = OpLabel
         %12 = OpLoad %6 %8
               OpStore %11 %12
               OpBranch %113
        %113 = OpLabel
         %13 = OpLoad %6 %8
               OpStore %11 %13
               OpBranch %114
        %114 = OpLabel
         %14 = OpLoad %6 %8
               OpStore %11 %14
               OpBranch %115
        %115 = OpLabel
         %15 = OpLoad %6 %8
               OpStore %11 %15
               OpBranch %116
        %116 = OpLabel
         %16 = OpLoad %6 %8
               OpStore %11 %16
               OpBranch %117
        %117 = OpLabel
         %17 = OpLoad %6 %8
               OpStore %11 %17
               OpBranch %118
        %118 = OpLabel
         %18 = OpLoad %6 %8
               OpStore %11 %18
               OpBranch %119
        %119 = OpLabel
         %19 = OpLoad %6 %8
               OpStore %11 %19
               OpBranch %120
        %120 = OpLabel
         %20 = OpLoad %6 %8
               OpStore %11 %20
               OpBranch %121
        %121 = OpLabel
         %21 = OpLoad %6 %8
               OpStore %11 %21
               OpBranch %122
        %122 = OpLabel
         %22 = OpLoad %6 %8
               OpStore %11 %22
               OpReturn
               OpFunctionEnd
    )";
    ASSERT_TRUE(IsEqual(env, kFullySplitShader,
                        replayer_result.transformed_module.get()));
  }

  {
    // Half replay
    protobufs::FactSequence empty_facts;
    auto replayer_result =
        Replayer(env, kConsoleMessageConsumer, binary_in, empty_facts,
                 transformations, 5, true, validator_options)
            .Run();
    // Replay should succeed.
    ASSERT_EQ(Replayer::ReplayerResultStatus::kComplete,
              replayer_result.status);
    // The first 5 transformations should be applied
    ASSERT_EQ(5, replayer_result.applied_transformations.transformation_size());
    for (uint32_t i = 0; i < 5; i++) {
      ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(
          transformations.transformation(i),
          replayer_result.applied_transformations.transformation(i)));
    }

    const std::string kHalfSplitShader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "g"
               OpName %11 "x"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Private %6
          %8 = OpVariable %7 Private
          %9 = OpConstant %6 10
         %10 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
               OpStore %8 %9
               OpBranch %112
        %112 = OpLabel
         %12 = OpLoad %6 %8
               OpStore %11 %12
               OpBranch %113
        %113 = OpLabel
         %13 = OpLoad %6 %8
               OpStore %11 %13
               OpBranch %114
        %114 = OpLabel
         %14 = OpLoad %6 %8
               OpStore %11 %14
               OpBranch %115
        %115 = OpLabel
         %15 = OpLoad %6 %8
               OpStore %11 %15
               OpBranch %116
        %116 = OpLabel
         %16 = OpLoad %6 %8
               OpStore %11 %16
         %17 = OpLoad %6 %8
               OpStore %11 %17
         %18 = OpLoad %6 %8
               OpStore %11 %18
         %19 = OpLoad %6 %8
               OpStore %11 %19
         %20 = OpLoad %6 %8
               OpStore %11 %20
         %21 = OpLoad %6 %8
               OpStore %11 %21
         %22 = OpLoad %6 %8
               OpStore %11 %22
               OpReturn
               OpFunctionEnd
    )";
    ASSERT_TRUE(IsEqual(env, kHalfSplitShader,
                        replayer_result.transformed_module.get()));
  }

  {
    // Empty replay
    protobufs::FactSequence empty_facts;
    auto replayer_result =
        Replayer(env, kConsoleMessageConsumer, binary_in, empty_facts,
                 transformations, 0, true, validator_options)
            .Run();
    // Replay should succeed.
    ASSERT_EQ(Replayer::ReplayerResultStatus::kComplete,
              replayer_result.status);
    // No transformations should be applied
    ASSERT_EQ(0, replayer_result.applied_transformations.transformation_size());
    ASSERT_TRUE(
        IsEqual(env, kTestShader, replayer_result.transformed_module.get()));
  }

  {
    // Invalid replay: too many transformations
    protobufs::FactSequence empty_facts;
    // The number of transformations requested to be applied exceeds the number
    // of transformations
    auto replayer_result =
        Replayer(env, kConsoleMessageConsumer, binary_in, empty_facts,
                 transformations, 12, true, validator_options)
            .Run();

    // Replay should not succeed.
    ASSERT_EQ(Replayer::ReplayerResultStatus::kTooManyTransformationsRequested,
              replayer_result.status);
    // No transformations should be applied
    ASSERT_EQ(0, replayer_result.applied_transformations.transformation_size());
    // The output binary should be empty
    ASSERT_EQ(nullptr, replayer_result.transformed_module);
  }
}

TEST(ReplayerTest, CheckFactsAfterReplay) {
  const std::string kTestShader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %50 = OpTypePointer Private %8
         %11 = OpConstant %8 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %10 = OpVariable %9 Function
               OpStore %10 %11
         %12 = OpFunctionCall %2 %6
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  spvtools::ValidatorOptions validator_options;

  std::vector<uint32_t> binary_in;
  SpirvTools t(env);
  t.SetMessageConsumer(kConsoleMessageConsumer);
  ASSERT_TRUE(t.Assemble(kTestShader, &binary_in, kFuzzAssembleOption));
  ASSERT_TRUE(t.Validate(binary_in));

  protobufs::TransformationSequence transformations;
  *transformations.add_transformation() =
      TransformationAddConstantScalar(100, 8, {42}, true).ToMessage();
  *transformations.add_transformation() =
      TransformationAddGlobalVariable(101, 50, spv::StorageClass::Private, 100,
                                      true)
          .ToMessage();
  *transformations.add_transformation() =
      TransformationAddParameter(6, 102, 8, {{12, 100}}, 103).ToMessage();
  *transformations.add_transformation() =
      TransformationAddSynonym(
          11,
          protobufs::TransformationAddSynonym::SynonymType::
              TransformationAddSynonym_SynonymType_COPY_OBJECT,
          104, MakeInstructionDescriptor(12, spv::Op::OpFunctionCall, 0))
          .ToMessage();

  // Full replay
  protobufs::FactSequence empty_facts;
  auto replayer_result =
      Replayer(env, kConsoleMessageConsumer, binary_in, empty_facts,
               transformations, transformations.transformation_size(), true,
               validator_options)
          .Run();
  // Replay should succeed.
  ASSERT_EQ(Replayer::ReplayerResultStatus::kComplete, replayer_result.status);
  // All transformations should be applied.
  ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(
      transformations, replayer_result.applied_transformations));

  const std::string kExpected = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %50 = OpTypePointer Private %8
         %11 = OpConstant %8 1
        %100 = OpConstant %8 42
        %101 = OpVariable %50 Private %100
        %103 = OpTypeFunction %2 %8
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %10 = OpVariable %9 Function
               OpStore %10 %11
        %104 = OpCopyObject %8 %11
         %12 = OpFunctionCall %2 %6 %100
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %103
        %102 = OpFunctionParameter %8
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(
      IsEqual(env, kExpected, replayer_result.transformed_module.get()));

  ASSERT_TRUE(
      replayer_result.transformation_context->GetFactManager()->IdIsIrrelevant(
          100));
  ASSERT_TRUE(replayer_result.transformation_context->GetFactManager()
                  ->PointeeValueIsIrrelevant(101));
  ASSERT_TRUE(
      replayer_result.transformation_context->GetFactManager()->IdIsIrrelevant(
          102));
  ASSERT_TRUE(
      replayer_result.transformation_context->GetFactManager()->IsSynonymous(
          MakeDataDescriptor(11, {}), MakeDataDescriptor(104, {})));
}

TEST(ReplayerTest, ReplayWithOverflowIds) {
  const std::string kTestShader = R"(
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
         %50 = OpTypePointer Private %6
          %9 = OpConstant %6 2
         %11 = OpConstant %6 0
         %12 = OpTypeBool
         %17 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpStore %8 %9
         %10 = OpLoad %6 %8
         %13 = OpSGreaterThan %12 %10 %11
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %15
         %14 = OpLabel
         %16 = OpLoad %6 %8
         %18 = OpIAdd %6 %16 %17
               OpStore %8 %18
               OpBranch %15
         %15 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  spvtools::ValidatorOptions validator_options;

  std::vector<uint32_t> binary_in;
  SpirvTools t(env);
  t.SetMessageConsumer(kConsoleMessageConsumer);
  ASSERT_TRUE(t.Assemble(kTestShader, &binary_in, kFuzzAssembleOption));
  ASSERT_TRUE(t.Validate(binary_in));

  protobufs::TransformationSequence transformations;
  *transformations.add_transformation() =
      TransformationFlattenConditionalBranch(5, true, 0, 0, 0, {}).ToMessage();
  *transformations.add_transformation() =
      TransformationAddGlobalVariable(101, 50, spv::StorageClass::Private, 11,
                                      true)
          .ToMessage();

  protobufs::FactSequence empty_facts;
  auto replayer_result =
      Replayer(env, kConsoleMessageConsumer, binary_in, empty_facts,
               transformations, transformations.transformation_size(), true,
               validator_options)
          .Run();
  // Replay should succeed.
  ASSERT_EQ(Replayer::ReplayerResultStatus::kComplete, replayer_result.status);
  // All transformations should be applied.
  ASSERT_EQ(2, replayer_result.applied_transformations.transformation_size());
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
