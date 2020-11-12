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

#include "source/fuzz/fact_manager/fact_manager.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/transformation_merge_blocks.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

void CheckConsistencyOfSynonymFacts(
    opt::IRContext* ir_context,
    const TransformationContext& transformation_context) {
  for (uint32_t id : transformation_context.GetFactManager()
                         ->GetIdsForWhichSynonymsAreKnown()) {
    // Every id reported by the fact manager should exist in the module.
    ASSERT_NE(ir_context->get_def_use_mgr()->GetDef(id), nullptr);
    auto synonyms =
        transformation_context.GetFactManager()->GetSynonymsForId(id);
    for (auto& dd : synonyms) {
      // Every reported synonym should have a base object that exists in the
      // module.
      ASSERT_NE(ir_context->get_def_use_mgr()->GetDef(dd->object()), nullptr);
    }
  }
}

TEST(DataSynonymAndIdEquationFactsTest, RecursiveAdditionOfFacts) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
          %8 = OpTypeMatrix %7 4
          %9 = OpConstant %6 0
         %10 = OpConstantComposite %7 %9 %9 %9 %9
         %11 = OpConstantComposite %8 %10 %10 %10 %10
         %12 = OpFunction %2 None %3
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  FactManager fact_manager(context.get());

  fact_manager.AddFactDataSynonym(MakeDataDescriptor(10, {}),
                                  MakeDataDescriptor(11, {2}));

  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(10, {}),
                                        MakeDataDescriptor(11, {2})));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(10, {0}),
                                        MakeDataDescriptor(11, {2, 0})));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(10, {1}),
                                        MakeDataDescriptor(11, {2, 1})));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(10, {2}),
                                        MakeDataDescriptor(11, {2, 2})));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(10, {3}),
                                        MakeDataDescriptor(11, {2, 3})));
}

TEST(DataSynonymAndIdEquationFactsTest, CorollaryConversionFacts) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpTypeVector %6 2
          %9 = OpTypeVector %7 2
         %10 = OpTypeFloat 32
         %11 = OpTypeVector %10 2
         %15 = OpConstant %6 24 ; synonym of %16
         %16 = OpConstant %6 24
         %17 = OpConstant %7 24 ; synonym of %18
         %18 = OpConstant %7 24
         %19 = OpConstantComposite %8 %15 %15 ; synonym of %20
         %20 = OpConstantComposite %8 %16 %16
         %21 = OpConstantComposite %9 %17 %17 ; synonym of %22
         %22 = OpConstantComposite %9 %18 %18
         %23 = OpConstantComposite %8 %15 %15 ; not a synonym of %19
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %24 = OpConvertSToF %10 %15 ; synonym of %25
         %25 = OpConvertSToF %10 %16
         %26 = OpConvertUToF %10 %17 ; not a synonym of %27 (different opcode)
         %27 = OpConvertSToF %10 %18
         %28 = OpConvertUToF %11 %19 ; synonym of %29
         %29 = OpConvertUToF %11 %20
         %30 = OpConvertSToF %11 %21 ; not a synonym of %31 (different opcode)
         %31 = OpConvertUToF %11 %22
         %32 = OpConvertUToF %11 %23 ; not a synonym of %28 (operand is not synonymous)
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  FactManager fact_manager(context.get());

  // Add equation facts
  fact_manager.AddFactIdEquation(24, SpvOpConvertSToF, {15});
  fact_manager.AddFactIdEquation(25, SpvOpConvertSToF, {16});
  fact_manager.AddFactIdEquation(26, SpvOpConvertUToF, {17});
  fact_manager.AddFactIdEquation(27, SpvOpConvertSToF, {18});
  fact_manager.AddFactIdEquation(28, SpvOpConvertUToF, {19});
  fact_manager.AddFactIdEquation(29, SpvOpConvertUToF, {20});
  fact_manager.AddFactIdEquation(30, SpvOpConvertSToF, {21});
  fact_manager.AddFactIdEquation(31, SpvOpConvertUToF, {22});
  fact_manager.AddFactIdEquation(32, SpvOpConvertUToF, {23});

  fact_manager.AddFactDataSynonym(MakeDataDescriptor(15, {}),
                                  MakeDataDescriptor(16, {}));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(24, {}),
                                        MakeDataDescriptor(25, {})));

  fact_manager.AddFactDataSynonym(MakeDataDescriptor(17, {}),
                                  MakeDataDescriptor(18, {}));
  ASSERT_FALSE(fact_manager.IsSynonymous(MakeDataDescriptor(26, {}),
                                         MakeDataDescriptor(27, {})));

  fact_manager.AddFactDataSynonym(MakeDataDescriptor(19, {}),
                                  MakeDataDescriptor(20, {}));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(28, {}),
                                        MakeDataDescriptor(29, {})));

  fact_manager.AddFactDataSynonym(MakeDataDescriptor(21, {}),
                                  MakeDataDescriptor(22, {}));
  ASSERT_FALSE(fact_manager.IsSynonymous(MakeDataDescriptor(30, {}),
                                         MakeDataDescriptor(31, {})));

  ASSERT_FALSE(fact_manager.IsSynonymous(MakeDataDescriptor(32, {}),
                                         MakeDataDescriptor(28, {})));
  fact_manager.AddFactDataSynonym(MakeDataDescriptor(23, {}),
                                  MakeDataDescriptor(19, {}));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(32, {}),
                                        MakeDataDescriptor(28, {})));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(32, {}),
                                        MakeDataDescriptor(29, {})));
}

TEST(DataSynonymAndIdEquationFactsTest, HandlesCorollariesWithInvalidIds) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %8 = OpTypeInt 32 1
          %9 = OpConstant %8 3
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %14 = OpConvertSToF %6 %9
               OpBranch %16
         %16 = OpLabel
         %17 = OpPhi %6 %14 %13
         %15 = OpConvertSToF %6 %9
         %18 = OpConvertSToF %6 %9
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  // Add required facts.
  transformation_context.GetFactManager()->AddFactIdEquation(
      14, SpvOpConvertSToF, {9});
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(14, {}), MakeDataDescriptor(17, {}));

  CheckConsistencyOfSynonymFacts(context.get(), transformation_context);

  // Apply TransformationMergeBlocks which will remove %17 from the module.
  TransformationMergeBlocks transformation(16);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  transformation.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  CheckConsistencyOfSynonymFacts(context.get(), transformation_context);

  ASSERT_EQ(context->get_def_use_mgr()->GetDef(17), nullptr);

  // Add another equation.
  transformation_context.GetFactManager()->AddFactIdEquation(
      15, SpvOpConvertSToF, {9});

  // Check that two ids are synonymous even though one of them doesn't exist in
  // the module (%17).
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(15, {}), MakeDataDescriptor(17, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(15, {}), MakeDataDescriptor(14, {})));

  CheckConsistencyOfSynonymFacts(context.get(), transformation_context);

  // Remove some instructions from the module. At this point, the equivalence
  // class of %14 has no valid members.
  ASSERT_TRUE(context->KillDef(14));
  ASSERT_TRUE(context->KillDef(15));

  transformation_context.GetFactManager()->AddFactIdEquation(
      18, SpvOpConvertSToF, {9});

  CheckConsistencyOfSynonymFacts(context.get(), transformation_context);

  // We don't create synonyms if at least one of the equivalence classes has no
  // valid members.
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(14, {}), MakeDataDescriptor(18, {})));
}

TEST(DataSynonymAndIdEquationFactsTest, LogicalNotEquationFacts) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %14 = OpLogicalNot %6 %7
         %15 = OpCopyObject %6 %7
         %16 = OpCopyObject %6 %14
         %17 = OpLogicalNot %6 %16
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  FactManager fact_manager(context.get());

  fact_manager.AddFactDataSynonym(MakeDataDescriptor(15, {}),
                                  MakeDataDescriptor(7, {}));
  fact_manager.AddFactDataSynonym(MakeDataDescriptor(16, {}),
                                  MakeDataDescriptor(14, {}));
  fact_manager.AddFactIdEquation(14, SpvOpLogicalNot, {7});
  fact_manager.AddFactIdEquation(17, SpvOpLogicalNot, {16});

  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(15, {}),
                                        MakeDataDescriptor(7, {})));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(17, {}),
                                        MakeDataDescriptor(7, {})));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(15, {}),
                                        MakeDataDescriptor(17, {})));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(16, {}),
                                        MakeDataDescriptor(14, {})));
}

TEST(DataSynonymAndIdEquationFactsTest, SignedNegateEquationFacts) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 24
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %14 = OpSNegate %6 %7
         %15 = OpSNegate %6 %14
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  FactManager fact_manager(context.get());

  fact_manager.AddFactIdEquation(14, SpvOpSNegate, {7});
  fact_manager.AddFactIdEquation(15, SpvOpSNegate, {14});

  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(7, {}),
                                        MakeDataDescriptor(15, {})));
}

TEST(DataSynonymAndIdEquationFactsTest, AddSubNegateFacts1) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %15 = OpConstant %6 24
         %16 = OpConstant %6 37
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %14 = OpIAdd %6 %15 %16
         %17 = OpCopyObject %6 %15
         %18 = OpCopyObject %6 %16
         %19 = OpISub %6 %14 %18 ; ==> synonymous(%19, %15)
         %20 = OpISub %6 %14 %17 ; ==> synonymous(%20, %16)
         %21 = OpCopyObject %6 %14
         %22 = OpISub %6 %16 %21
         %23 = OpCopyObject %6 %22
         %24 = OpSNegate %6 %23 ; ==> synonymous(%24, %15)
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  FactManager fact_manager(context.get());

  fact_manager.AddFactIdEquation(14, SpvOpIAdd, {15, 16});
  fact_manager.AddFactDataSynonym(MakeDataDescriptor(17, {}),
                                  MakeDataDescriptor(15, {}));
  fact_manager.AddFactDataSynonym(MakeDataDescriptor(18, {}),
                                  MakeDataDescriptor(16, {}));
  fact_manager.AddFactIdEquation(19, SpvOpISub, {14, 18});
  fact_manager.AddFactIdEquation(20, SpvOpISub, {14, 17});
  fact_manager.AddFactDataSynonym(MakeDataDescriptor(21, {}),
                                  MakeDataDescriptor(14, {}));
  fact_manager.AddFactIdEquation(22, SpvOpISub, {16, 21});
  fact_manager.AddFactDataSynonym(MakeDataDescriptor(23, {}),
                                  MakeDataDescriptor(22, {}));
  fact_manager.AddFactIdEquation(24, SpvOpSNegate, {23});

  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(19, {}),
                                        MakeDataDescriptor(15, {})));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(20, {}),
                                        MakeDataDescriptor(16, {})));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(24, {}),
                                        MakeDataDescriptor(15, {})));
}

TEST(DataSynonymAndIdEquationFactsTest, AddSubNegateFacts2) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %15 = OpConstant %6 24
         %16 = OpConstant %6 37
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %14 = OpISub %6 %15 %16
         %17 = OpIAdd %6 %14 %16 ; ==> synonymous(%17, %15)
         %18 = OpIAdd %6 %16 %14 ; ==> synonymous(%17, %18, %15)
         %19 = OpISub %6 %14 %15
         %20 = OpSNegate %6 %19 ; ==> synonymous(%20, %16)
         %21 = OpISub %6 %14 %19 ; ==> synonymous(%21, %15)
         %22 = OpISub %6 %14 %18
         %23 = OpSNegate %6 %22 ; ==> synonymous(%23, %16)
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  FactManager fact_manager(context.get());

  fact_manager.AddFactIdEquation(14, SpvOpISub, {15, 16});
  fact_manager.AddFactIdEquation(17, SpvOpIAdd, {14, 16});

  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(17, {}),
                                        MakeDataDescriptor(15, {})));

  fact_manager.AddFactIdEquation(18, SpvOpIAdd, {16, 14});

  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(18, {}),
                                        MakeDataDescriptor(15, {})));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(17, {}),
                                        MakeDataDescriptor(18, {})));

  fact_manager.AddFactIdEquation(19, SpvOpISub, {14, 15});
  fact_manager.AddFactIdEquation(20, SpvOpSNegate, {19});

  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(20, {}),
                                        MakeDataDescriptor(16, {})));

  fact_manager.AddFactIdEquation(21, SpvOpISub, {14, 19});
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(21, {}),
                                        MakeDataDescriptor(15, {})));

  fact_manager.AddFactIdEquation(22, SpvOpISub, {14, 18});
  fact_manager.AddFactIdEquation(23, SpvOpSNegate, {22});
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(23, {}),
                                        MakeDataDescriptor(16, {})));
}

TEST(DataSynonymAndIdEquationFactsTest, ConversionEquations) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 1
          %5 = OpTypeInt 32 0
          %6 = OpTypeFloat 32
         %14 = OpTypeVector %4 2
         %15 = OpTypeVector %5 2
         %24 = OpTypeVector %6 2
         %16 = OpConstant %4 32 ; synonym of %17
         %17 = OpConstant %4 32
         %18 = OpConstant %5 32 ; synonym of %19
         %19 = OpConstant %5 32
         %20 = OpConstantComposite %14 %16 %16 ; synonym of %21
         %21 = OpConstantComposite %14 %17 %17
         %22 = OpConstantComposite %15 %18 %18 ; synonym of %23
         %23 = OpConstantComposite %15 %19 %19
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %25 = OpConvertUToF %6 %16 ; synonym of %26
         %26 = OpConvertUToF %6 %17
         %27 = OpConvertSToF %24 %20 ; not a synonym of %28 (wrong opcode)
         %28 = OpConvertUToF %24 %21
         %29 = OpConvertSToF %6 %18 ; not a synonym of %30 (wrong opcode)
         %30 = OpConvertUToF %6 %19
         %31 = OpConvertSToF %24 %22 ; synonym of %32
         %32 = OpConvertSToF %24 %23
         %33 = OpConvertUToF %6 %17 ; synonym of %26
         %34 = OpConvertSToF %24 %23 ; synonym of %32
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  FactManager fact_manager(context.get());

  fact_manager.AddFactDataSynonym(MakeDataDescriptor(16, {}),
                                  MakeDataDescriptor(17, {}));
  fact_manager.AddFactDataSynonym(MakeDataDescriptor(18, {}),
                                  MakeDataDescriptor(19, {}));
  fact_manager.AddFactDataSynonym(MakeDataDescriptor(20, {}),
                                  MakeDataDescriptor(21, {}));
  fact_manager.AddFactDataSynonym(MakeDataDescriptor(22, {}),
                                  MakeDataDescriptor(23, {}));

  fact_manager.AddFactIdEquation(25, SpvOpConvertUToF, {16});
  fact_manager.AddFactIdEquation(26, SpvOpConvertUToF, {17});
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(25, {}),
                                        MakeDataDescriptor(26, {})));

  fact_manager.AddFactIdEquation(27, SpvOpConvertSToF, {20});
  fact_manager.AddFactIdEquation(28, SpvOpConvertUToF, {21});
  ASSERT_FALSE(fact_manager.IsSynonymous(MakeDataDescriptor(27, {}),
                                         MakeDataDescriptor(28, {})));

  fact_manager.AddFactIdEquation(29, SpvOpConvertSToF, {18});
  fact_manager.AddFactIdEquation(30, SpvOpConvertUToF, {19});
  ASSERT_FALSE(fact_manager.IsSynonymous(MakeDataDescriptor(29, {}),
                                         MakeDataDescriptor(30, {})));

  fact_manager.AddFactIdEquation(31, SpvOpConvertSToF, {22});
  fact_manager.AddFactIdEquation(32, SpvOpConvertSToF, {23});
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(31, {}),
                                        MakeDataDescriptor(32, {})));

  fact_manager.AddFactIdEquation(33, SpvOpConvertUToF, {17});
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(33, {}),
                                        MakeDataDescriptor(26, {})));

  fact_manager.AddFactIdEquation(34, SpvOpConvertSToF, {23});
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(32, {}),
                                        MakeDataDescriptor(34, {})));
}

TEST(DataSynonymAndIdEquationFactsTest, BitcastEquationFacts) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 1
          %5 = OpTypeInt 32 0
          %8 = OpTypeFloat 32
          %9 = OpTypeVector %4 2
         %10 = OpTypeVector %5 2
         %11 = OpTypeVector %8 2
          %6 = OpConstant %4 23
          %7 = OpConstant %5 23
         %19 = OpConstant %8 23
         %20 = OpConstantComposite %9 %6 %6
         %21 = OpConstantComposite %10 %7 %7
         %22 = OpConstantComposite %11 %19 %19
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %30 = OpBitcast %8 %6
         %31 = OpBitcast %5 %6
         %32 = OpBitcast %8 %7
         %33 = OpBitcast %4 %7
         %34 = OpBitcast %4 %19
         %35 = OpBitcast %5 %19
         %36 = OpBitcast %10 %20
         %37 = OpBitcast %11 %20
         %38 = OpBitcast %9 %21
         %39 = OpBitcast %11 %21
         %40 = OpBitcast %9 %22
         %41 = OpBitcast %10 %22
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  FactManager fact_manager(context.get());

  uint32_t lhs_id = 30;
  for (uint32_t rhs_id : {6, 6, 7, 7, 19, 19, 20, 20, 21, 21, 22, 22}) {
    fact_manager.AddFactIdEquation(lhs_id, SpvOpBitcast, {rhs_id});
    ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(lhs_id, {}),
                                          MakeDataDescriptor(rhs_id, {})));
    ++lhs_id;
  }
}

TEST(DataSynonymAndIdEquationFactsTest, EquationAndEquivalenceFacts) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %15 = OpConstant %6 24
         %16 = OpConstant %6 37
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %14 = OpISub %6 %15 %16
        %114 = OpCopyObject %6 %14
         %17 = OpIAdd %6 %114 %16 ; ==> synonymous(%17, %15)
         %18 = OpIAdd %6 %16 %114 ; ==> synonymous(%17, %18, %15)
         %19 = OpISub %6 %114 %15
        %119 = OpCopyObject %6 %19
         %20 = OpSNegate %6 %119 ; ==> synonymous(%20, %16)
         %21 = OpISub %6 %14 %19 ; ==> synonymous(%21, %15)
         %22 = OpISub %6 %14 %18
        %220 = OpCopyObject %6 %22
         %23 = OpSNegate %6 %220 ; ==> synonymous(%23, %16)
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  FactManager fact_manager(context.get());

  fact_manager.AddFactIdEquation(14, SpvOpISub, {15, 16});
  fact_manager.AddFactDataSynonym(MakeDataDescriptor(114, {}),
                                  MakeDataDescriptor(14, {}));
  fact_manager.AddFactIdEquation(17, SpvOpIAdd, {114, 16});

  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(17, {}),
                                        MakeDataDescriptor(15, {})));

  fact_manager.AddFactIdEquation(18, SpvOpIAdd, {16, 114});

  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(18, {}),
                                        MakeDataDescriptor(15, {})));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(17, {}),
                                        MakeDataDescriptor(18, {})));

  fact_manager.AddFactIdEquation(19, SpvOpISub, {14, 15});
  fact_manager.AddFactDataSynonym(MakeDataDescriptor(119, {}),
                                  MakeDataDescriptor(19, {}));
  fact_manager.AddFactIdEquation(20, SpvOpSNegate, {119});

  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(20, {}),
                                        MakeDataDescriptor(16, {})));

  fact_manager.AddFactIdEquation(21, SpvOpISub, {14, 19});
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(21, {}),
                                        MakeDataDescriptor(15, {})));

  fact_manager.AddFactIdEquation(22, SpvOpISub, {14, 18});
  fact_manager.AddFactDataSynonym(MakeDataDescriptor(22, {}),
                                  MakeDataDescriptor(220, {}));
  fact_manager.AddFactIdEquation(23, SpvOpSNegate, {220});
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(23, {}),
                                        MakeDataDescriptor(16, {})));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
