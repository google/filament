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

#include "source/fuzz/transformation_compute_data_synonym_fact_closure.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationComputeDataSynonymFactClosureTest, DataSynonymFacts) {
  // The SPIR-V types and constants come from the following code.  The body of
  // the SPIR-V function then constructs a composite that is synonymous with
  // myT.
  //
  // #version 310 es
  //
  // precision highp float;
  //
  // struct S {
  //   int a;
  //   uvec2 b;
  // };
  //
  // struct T {
  //   bool c[5];
  //   mat4x2 d;
  //   S e;
  // };
  //
  // void main() {
  //   T myT = T(bool[5](true, false, true, false, true),
  //             mat4x2(vec2(1.0, 2.0), vec2(3.0, 4.0),
  //                    vec2(5.0, 6.0), vec2(7.0, 8.0)),
  //             S(10, uvec2(100u, 200u)));
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %15 "S"
               OpMemberName %15 0 "a"
               OpMemberName %15 1 "b"
               OpName %16 "T"
               OpMemberName %16 0 "c"
               OpMemberName %16 1 "d"
               OpMemberName %16 2 "e"
               OpName %18 "myT"
               OpMemberDecorate %15 0 RelaxedPrecision
               OpMemberDecorate %15 1 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 5
          %9 = OpTypeArray %6 %8
         %10 = OpTypeFloat 32
         %11 = OpTypeVector %10 2
         %12 = OpTypeMatrix %11 4
         %13 = OpTypeInt 32 1
         %14 = OpTypeVector %7 2
         %15 = OpTypeStruct %13 %14
         %16 = OpTypeStruct %9 %12 %15
         %17 = OpTypePointer Function %16
         %19 = OpConstantTrue %6
         %20 = OpConstantFalse %6
         %21 = OpConstantComposite %9 %19 %20 %19 %20 %19
         %22 = OpConstant %10 1
         %23 = OpConstant %10 2
         %24 = OpConstantComposite %11 %22 %23
         %25 = OpConstant %10 3
         %26 = OpConstant %10 4
         %27 = OpConstantComposite %11 %25 %26
         %28 = OpConstant %10 5
         %29 = OpConstant %10 6
         %30 = OpConstantComposite %11 %28 %29
         %31 = OpConstant %10 7
         %32 = OpConstant %10 8
         %33 = OpConstantComposite %11 %31 %32
         %34 = OpConstantComposite %12 %24 %27 %30 %33
         %35 = OpConstant %13 10
         %36 = OpConstant %7 100
         %37 = OpConstant %7 200
         %38 = OpConstantComposite %14 %36 %37
         %39 = OpConstantComposite %15 %35 %38
         %40 = OpConstantComposite %16 %21 %34 %39
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %18 = OpVariable %17 Function
               OpStore %18 %40
        %100 = OpCompositeConstruct %9 %19 %20 %19 %20 %19
        %101 = OpCompositeConstruct %11 %22 %23
        %102 = OpCompositeConstruct %11 %25 %26
        %103 = OpCompositeConstruct %11 %28 %29
        %104 = OpCompositeConstruct %11 %31 %32
        %105 = OpCompositeConstruct %12 %101 %102 %103 %104
        %106 = OpCompositeConstruct %14 %36 %37
        %107 = OpCompositeConstruct %15 %35 %106
        %108 = OpCompositeConstruct %16 %100 %105 %107
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
  ASSERT_TRUE(TransformationComputeDataSynonymFactClosure(100).IsApplicable(
      context.get(), transformation_context));

  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(24, {}), MakeDataDescriptor(101, {})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(24, {0}), MakeDataDescriptor(101, {0})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(24, {1}), MakeDataDescriptor(101, {1})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(24, {0}), MakeDataDescriptor(101, {1})));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(24, {}), MakeDataDescriptor(101, {}));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(24, {}), MakeDataDescriptor(101, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(24, {0}), MakeDataDescriptor(101, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(24, {1}), MakeDataDescriptor(101, {1})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(24, {0}), MakeDataDescriptor(101, {1})));

  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(27, {}), MakeDataDescriptor(102, {})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(27, {0}), MakeDataDescriptor(102, {0})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(27, {1}), MakeDataDescriptor(102, {1})));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(27, {0}), MakeDataDescriptor(102, {0}));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(27, {}), MakeDataDescriptor(102, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(27, {0}), MakeDataDescriptor(102, {0})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(27, {1}), MakeDataDescriptor(102, {1})));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(27, {1}), MakeDataDescriptor(102, {1}));

  ApplyAndCheckFreshIds(TransformationComputeDataSynonymFactClosure(100),
                        context.get(), &transformation_context);

  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(27, {}), MakeDataDescriptor(102, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(27, {0}), MakeDataDescriptor(102, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(27, {1}), MakeDataDescriptor(102, {1})));

  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {}), MakeDataDescriptor(103, {})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {0}), MakeDataDescriptor(103, {0})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {1}), MakeDataDescriptor(103, {1})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(33, {}), MakeDataDescriptor(104, {})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(33, {0}), MakeDataDescriptor(104, {0})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(33, {1}), MakeDataDescriptor(104, {1})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(34, {}), MakeDataDescriptor(105, {})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(34, {0}), MakeDataDescriptor(105, {0})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(34, {1}), MakeDataDescriptor(105, {1})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(34, {2}), MakeDataDescriptor(105, {2})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(34, {3}), MakeDataDescriptor(105, {3})));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(30, {}), MakeDataDescriptor(103, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(33, {}), MakeDataDescriptor(104, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(34, {0}), MakeDataDescriptor(105, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(34, {1}), MakeDataDescriptor(105, {1}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(34, {2}), MakeDataDescriptor(105, {2}));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {}), MakeDataDescriptor(103, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {0}), MakeDataDescriptor(103, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {1}), MakeDataDescriptor(103, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(33, {}), MakeDataDescriptor(104, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(33, {0}), MakeDataDescriptor(104, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(33, {1}), MakeDataDescriptor(104, {1})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(34, {}), MakeDataDescriptor(105, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(34, {0}), MakeDataDescriptor(105, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(34, {1}), MakeDataDescriptor(105, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(34, {2}), MakeDataDescriptor(105, {2})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(34, {3}), MakeDataDescriptor(105, {3})));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(34, {3}), MakeDataDescriptor(105, {3}));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(33, {0}), MakeDataDescriptor(104, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(34, {3}), MakeDataDescriptor(105, {3})));

  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(21, {}), MakeDataDescriptor(100, {})));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(21, {0}), MakeDataDescriptor(100, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(21, {1}), MakeDataDescriptor(100, {1}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(21, {2}), MakeDataDescriptor(100, {2}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(21, {3}), MakeDataDescriptor(100, {3}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(21, {4}), MakeDataDescriptor(100, {4}));

  ApplyAndCheckFreshIds(TransformationComputeDataSynonymFactClosure(100),
                        context.get(), &transformation_context);

  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(21, {}), MakeDataDescriptor(100, {})));

  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(39, {0}), MakeDataDescriptor(107, {0})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(35, {}), MakeDataDescriptor(39, {0})));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(39, {0}), MakeDataDescriptor(35, {}));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(39, {0}), MakeDataDescriptor(107, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(35, {}), MakeDataDescriptor(39, {0})));

  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(38, {0}), MakeDataDescriptor(36, {})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(38, {1}), MakeDataDescriptor(37, {})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(106, {0}), MakeDataDescriptor(36, {})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(106, {1}), MakeDataDescriptor(37, {})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(38, {}), MakeDataDescriptor(106, {})));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(38, {0}), MakeDataDescriptor(36, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(106, {0}), MakeDataDescriptor(36, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(38, {1}), MakeDataDescriptor(37, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(106, {1}), MakeDataDescriptor(37, {}));

  ApplyAndCheckFreshIds(TransformationComputeDataSynonymFactClosure(100),
                        context.get(), &transformation_context);

  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(38, {0}), MakeDataDescriptor(36, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(38, {1}), MakeDataDescriptor(37, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(106, {0}), MakeDataDescriptor(36, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(106, {1}), MakeDataDescriptor(37, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(38, {}), MakeDataDescriptor(106, {})));

  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {}), MakeDataDescriptor(108, {})));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(107, {0}), MakeDataDescriptor(35, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(40, {0}), MakeDataDescriptor(108, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(40, {1}), MakeDataDescriptor(108, {1}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(40, {2}), MakeDataDescriptor(108, {2}));

  ApplyAndCheckFreshIds(TransformationComputeDataSynonymFactClosure(100),
                        context.get(), &transformation_context);

  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {}), MakeDataDescriptor(108, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {0}), MakeDataDescriptor(108, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {1}), MakeDataDescriptor(108, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {2}), MakeDataDescriptor(108, {2})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {0, 0}), MakeDataDescriptor(108, {0, 0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {0, 1}), MakeDataDescriptor(108, {0, 1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {0, 2}), MakeDataDescriptor(108, {0, 2})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {0, 3}), MakeDataDescriptor(108, {0, 3})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {0, 4}), MakeDataDescriptor(108, {0, 4})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {1, 0}), MakeDataDescriptor(108, {1, 0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {1, 1}), MakeDataDescriptor(108, {1, 1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {1, 2}), MakeDataDescriptor(108, {1, 2})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {1, 3}), MakeDataDescriptor(108, {1, 3})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {1, 0, 0}), MakeDataDescriptor(108, {1, 0, 0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {1, 1, 0}), MakeDataDescriptor(108, {1, 1, 0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {1, 2, 0}), MakeDataDescriptor(108, {1, 2, 0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {1, 3, 0}), MakeDataDescriptor(108, {1, 3, 0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {1, 0, 1}), MakeDataDescriptor(108, {1, 0, 1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {1, 1, 1}), MakeDataDescriptor(108, {1, 1, 1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {1, 2, 1}), MakeDataDescriptor(108, {1, 2, 1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {1, 3, 1}), MakeDataDescriptor(108, {1, 3, 1})));

  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {2, 0}), MakeDataDescriptor(108, {2, 0})));

  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {2, 1}), MakeDataDescriptor(108, {2, 1})));

  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {2, 1, 0}), MakeDataDescriptor(108, {2, 1, 0})));

  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {2, 1, 1}), MakeDataDescriptor(108, {2, 1, 1})));
}

TEST(TransformationComputeDataSynonymFactClosureTest,
     ComputeClosureWithMissingIds) {
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
          %7 = OpTypeVector %6 4
         %15 = OpConstant %6 24
         %16 = OpConstantComposite %7 %15 %15 %15 %15
         %17 = OpConstantComposite %7 %15 %15 %15 %15
         %18 = OpTypeStruct %7
         %19 = OpConstantComposite %18 %16
         %30 = OpConstantComposite %18 %17
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %50 = OpCopyObject %7 %16
         %51 = OpCopyObject %7 %17
         %20 = OpCopyObject %6 %15
         %21 = OpCopyObject %6 %15
         %22 = OpCopyObject %6 %15
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(20, {}), MakeDataDescriptor(15, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(21, {}), MakeDataDescriptor(15, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(22, {}), MakeDataDescriptor(15, {}));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(17, {0}), MakeDataDescriptor(15, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(17, {1}), MakeDataDescriptor(15, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(17, {2}), MakeDataDescriptor(15, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(17, {3}), MakeDataDescriptor(15, {}));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(16, {0}), MakeDataDescriptor(20, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(16, {1}), MakeDataDescriptor(21, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(16, {2}), MakeDataDescriptor(22, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(16, {3}), MakeDataDescriptor(15, {}));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(51, {0}), MakeDataDescriptor(15, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(51, {1}), MakeDataDescriptor(15, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(51, {2}), MakeDataDescriptor(15, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(51, {3}), MakeDataDescriptor(15, {}));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(50, {0}), MakeDataDescriptor(20, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(50, {1}), MakeDataDescriptor(21, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(50, {2}), MakeDataDescriptor(22, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(50, {3}), MakeDataDescriptor(15, {}));

  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(19, {}), MakeDataDescriptor(30, {})));

  context->KillDef(20);
  context->KillDef(21);
  context->KillDef(22);
  context->KillDef(50);
  context->KillDef(51);
  context->InvalidateAnalysesExceptFor(opt::IRContext::kAnalysisNone);

  ApplyAndCheckFreshIds(TransformationComputeDataSynonymFactClosure(100),
                        context.get(), &transformation_context);
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(19, {}), MakeDataDescriptor(30, {})));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
