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

#include "source/fuzz/transformation_access_chain.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAccessChainTest, BasicTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %48 %54
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 2
         %50 = OpTypeMatrix %7 2
         %70 = OpTypePointer Function %7
         %71 = OpTypePointer Function %50
          %8 = OpTypeStruct %7 %6
          %9 = OpTypePointer Function %8
         %10 = OpTypeInt 32 1
         %11 = OpTypePointer Function %10
         %12 = OpTypeFunction %10 %9 %11
         %17 = OpConstant %10 0
         %18 = OpTypeInt 32 0
         %19 = OpConstant %18 0
         %20 = OpTypePointer Function %6
         %99 = OpTypePointer Private %6
         %29 = OpConstant %6 0
         %30 = OpConstant %6 1
         %31 = OpConstantComposite %7 %29 %30
         %32 = OpConstant %6 2
         %33 = OpConstantComposite %8 %31 %32
         %35 = OpConstant %10 10
         %51 = OpConstant %18 10
         %80 = OpConstant %18 0
         %81 = OpConstant %10 1
         %82 = OpConstant %18 2
         %83 = OpConstant %10 3
         %84 = OpConstant %18 4
         %85 = OpConstant %10 5
         %52 = OpTypeArray %50 %51
         %53 = OpTypePointer Private %52
         %45 = OpUndef %9
         %46 = OpConstantNull %9
         %47 = OpTypePointer Private %8
         %48 = OpVariable %47 Private
         %54 = OpVariable %53 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %28 = OpVariable %9 Function
         %34 = OpVariable %11 Function
         %36 = OpVariable %9 Function
         %38 = OpVariable %11 Function
         %44 = OpCopyObject %9 %36
               OpStore %28 %33
               OpStore %34 %35
         %37 = OpLoad %8 %28
               OpStore %36 %37
         %39 = OpLoad %10 %34
               OpStore %38 %39
         %40 = OpFunctionCall %10 %15 %36 %38
         %41 = OpLoad %10 %34
         %42 = OpIAdd %10 %41 %40
               OpStore %34 %42
               OpReturn
               OpFunctionEnd
         %15 = OpFunction %10 None %12
         %13 = OpFunctionParameter %9
         %14 = OpFunctionParameter %11
         %16 = OpLabel
         %21 = OpAccessChain %20 %13 %17 %19
         %43 = OpCopyObject %9 %13
         %22 = OpLoad %6 %21
         %23 = OpConvertFToS %10 %22
         %24 = OpLoad %10 %14
         %25 = OpIAdd %10 %23 %24
               OpReturnValue %25
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Types:
  // Ptr | Pointee | Storage class | GLSL for pointee    | Ids of this type
  // ----+---------+---------------+---------------------+------------------
  //  9  |    8    | Function      | struct(vec2, float) | 28, 36, 44, 13, 43
  // 11  |   10    | Function      | int                 | 34, 38, 14
  // 20  |    6    | Function      | float               | -
  // 99  |    6    | Private       | float               | -
  // 53  |   52    | Private       | mat2x2[10]          | 54
  // 47  |    8    | Private       | struct(vec2, float) | 48
  // 70  |    7    | Function      | vec2                | -
  // 71  |   59    | Function      | mat2x2              | -

  // Indices 0-5 are in ids 80-85

  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      54);

  // Bad: id is not fresh
  ASSERT_FALSE(TransformationAccessChain(
                   43, 43, {80}, MakeInstructionDescriptor(24, SpvOpLoad, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Bad: pointer id does not exist
  ASSERT_FALSE(TransformationAccessChain(
                   100, 1000, {80}, MakeInstructionDescriptor(24, SpvOpLoad, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Bad: pointer id is not a type
  ASSERT_FALSE(TransformationAccessChain(
                   100, 5, {80}, MakeInstructionDescriptor(24, SpvOpLoad, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Bad: pointer id is not a pointer
  ASSERT_FALSE(TransformationAccessChain(
                   100, 23, {80}, MakeInstructionDescriptor(24, SpvOpLoad, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Bad: index id does not exist
  ASSERT_FALSE(TransformationAccessChain(
                   100, 43, {1000}, MakeInstructionDescriptor(24, SpvOpLoad, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Bad: index id is not a constant and the pointer refers to a struct
  ASSERT_FALSE(TransformationAccessChain(
                   100, 43, {24}, MakeInstructionDescriptor(25, SpvOpIAdd, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Bad: too many indices
  ASSERT_FALSE(
      TransformationAccessChain(100, 43, {80, 80, 80},
                                MakeInstructionDescriptor(24, SpvOpLoad, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: index id is out of bounds when accessing a struct
  ASSERT_FALSE(
      TransformationAccessChain(100, 43, {83, 80},
                                MakeInstructionDescriptor(24, SpvOpLoad, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: attempt to insert before variable
  ASSERT_FALSE(TransformationAccessChain(
                   100, 34, {}, MakeInstructionDescriptor(36, SpvOpVariable, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Bad: OpTypeBool must be present in the module to clamp an index
  ASSERT_FALSE(
      TransformationAccessChain(100, 36, {80, 81},
                                MakeInstructionDescriptor(37, SpvOpStore, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: pointer not available
  ASSERT_FALSE(
      TransformationAccessChain(
          100, 43, {80}, MakeInstructionDescriptor(21, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: instruction descriptor does not identify anything
  ASSERT_FALSE(TransformationAccessChain(
                   100, 43, {80}, MakeInstructionDescriptor(24, SpvOpLoad, 100))
                   .IsApplicable(context.get(), transformation_context));

#ifndef NDEBUG
  // Bad: pointer is null
  ASSERT_DEATH(
      TransformationAccessChain(100, 45, {80},
                                MakeInstructionDescriptor(24, SpvOpLoad, 0))
          .IsApplicable(context.get(), transformation_context),
      "Access chains should not be created from null/undefined pointers");
#endif

#ifndef NDEBUG
  // Bad: pointer is undef
  ASSERT_DEATH(
      TransformationAccessChain(100, 46, {80},
                                MakeInstructionDescriptor(24, SpvOpLoad, 0))
          .IsApplicable(context.get(), transformation_context),
      "Access chains should not be created from null/undefined pointers");
#endif

  // Bad: pointer to result type does not exist
  ASSERT_FALSE(TransformationAccessChain(
                   100, 52, {0}, MakeInstructionDescriptor(24, SpvOpLoad, 0))
                   .IsApplicable(context.get(), transformation_context));

  {
    TransformationAccessChain transformation(
        100, 43, {80}, MakeInstructionDescriptor(24, SpvOpLoad, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    ASSERT_FALSE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(100));
  }

  {
    TransformationAccessChain transformation(
        101, 28, {81}, MakeInstructionDescriptor(42, SpvOpReturn, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    ASSERT_FALSE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(101));
  }

  {
    TransformationAccessChain transformation(
        102, 44, {}, MakeInstructionDescriptor(44, SpvOpStore, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    ASSERT_FALSE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(103));
  }

  {
    TransformationAccessChain transformation(
        103, 13, {80}, MakeInstructionDescriptor(21, SpvOpAccessChain, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    ASSERT_FALSE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(104));
  }

  {
    TransformationAccessChain transformation(
        104, 34, {}, MakeInstructionDescriptor(44, SpvOpStore, 1));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    ASSERT_FALSE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(105));
  }

  {
    TransformationAccessChain transformation(
        105, 38, {}, MakeInstructionDescriptor(40, SpvOpFunctionCall, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    ASSERT_FALSE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(106));
  }

  {
    TransformationAccessChain transformation(
        106, 14, {}, MakeInstructionDescriptor(24, SpvOpLoad, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    ASSERT_FALSE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(107));
  }

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %48 %54
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 2
         %50 = OpTypeMatrix %7 2
         %70 = OpTypePointer Function %7
         %71 = OpTypePointer Function %50
          %8 = OpTypeStruct %7 %6
          %9 = OpTypePointer Function %8
         %10 = OpTypeInt 32 1
         %11 = OpTypePointer Function %10
         %12 = OpTypeFunction %10 %9 %11
         %17 = OpConstant %10 0
         %18 = OpTypeInt 32 0
         %19 = OpConstant %18 0
         %20 = OpTypePointer Function %6
         %99 = OpTypePointer Private %6
         %29 = OpConstant %6 0
         %30 = OpConstant %6 1
         %31 = OpConstantComposite %7 %29 %30
         %32 = OpConstant %6 2
         %33 = OpConstantComposite %8 %31 %32
         %35 = OpConstant %10 10
         %51 = OpConstant %18 10
         %80 = OpConstant %18 0
         %81 = OpConstant %10 1
         %82 = OpConstant %18 2
         %83 = OpConstant %10 3
         %84 = OpConstant %18 4
         %85 = OpConstant %10 5
         %52 = OpTypeArray %50 %51
         %53 = OpTypePointer Private %52
         %45 = OpUndef %9
         %46 = OpConstantNull %9
         %47 = OpTypePointer Private %8
         %48 = OpVariable %47 Private
         %54 = OpVariable %53 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %28 = OpVariable %9 Function
         %34 = OpVariable %11 Function
         %36 = OpVariable %9 Function
         %38 = OpVariable %11 Function
         %44 = OpCopyObject %9 %36
        %102 = OpAccessChain %9 %44
               OpStore %28 %33
        %104 = OpAccessChain %11 %34
               OpStore %34 %35
         %37 = OpLoad %8 %28
               OpStore %36 %37
         %39 = OpLoad %10 %34
               OpStore %38 %39
        %105 = OpAccessChain %11 %38
         %40 = OpFunctionCall %10 %15 %36 %38
         %41 = OpLoad %10 %34
         %42 = OpIAdd %10 %41 %40
               OpStore %34 %42
        %101 = OpAccessChain %20 %28 %81
               OpReturn
               OpFunctionEnd
         %15 = OpFunction %10 None %12
         %13 = OpFunctionParameter %9
         %14 = OpFunctionParameter %11
         %16 = OpLabel
        %103 = OpAccessChain %70 %13 %80
         %21 = OpAccessChain %20 %13 %17 %19
         %43 = OpCopyObject %9 %13
         %22 = OpLoad %6 %21
         %23 = OpConvertFToS %10 %22
        %100 = OpAccessChain %70 %43 %80
        %106 = OpAccessChain %11 %14
         %24 = OpLoad %10 %14
         %25 = OpIAdd %10 %23 %24
               OpReturnValue %25
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAccessChainTest, IsomorphicStructs) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %11 %12
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeStruct %6
          %8 = OpTypePointer Private %7
          %9 = OpTypeStruct %6
         %10 = OpTypePointer Private %9
         %11 = OpVariable %8 Private
         %12 = OpVariable %10 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  {
    TransformationAccessChain transformation(
        100, 11, {}, MakeInstructionDescriptor(5, SpvOpReturn, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }
  {
    TransformationAccessChain transformation(
        101, 12, {}, MakeInstructionDescriptor(5, SpvOpReturn, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %11 %12
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeStruct %6
          %8 = OpTypePointer Private %7
          %9 = OpTypeStruct %6
         %10 = OpTypePointer Private %9
         %11 = OpVariable %8 Private
         %12 = OpVariable %10 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %100 = OpAccessChain %8 %11
        %101 = OpAccessChain %10 %12
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAccessChainTest, ClampingVariables) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main" %3
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %4 = OpTypeVoid
          %5 = OpTypeBool
          %6 = OpTypeFunction %4
          %7 = OpTypeInt 32 1
          %8 = OpTypeVector %7 4
          %9 = OpTypePointer Function %8
         %10 = OpConstant %7 0
         %11 = OpConstant %7 1
         %12 = OpConstant %7 3
         %13 = OpConstant %7 2
         %14 = OpConstantComposite %8 %10 %11 %12 %13
         %15 = OpTypePointer Function %7
         %16 = OpTypeInt 32 0
         %17 = OpConstant %16 1
         %18 = OpConstant %16 3
         %19 = OpTypeStruct %8
         %20 = OpTypePointer Function %19
         %21 = OpConstant %7 9
         %22 = OpConstant %16 10
         %23 = OpTypeArray %19 %22
         %24 = OpTypePointer Function %23
         %25 = OpTypeFloat 32
         %26 = OpTypeVector %25 4
         %27 = OpTypePointer Output %26
          %3 = OpVariable %27 Output
          %2 = OpFunction %4 None %6
         %28 = OpLabel
         %29 = OpVariable %9 Function
         %30 = OpVariable %15 Function
         %31 = OpVariable %15 Function
         %32 = OpVariable %20 Function
         %33 = OpVariable %15 Function
         %34 = OpVariable %24 Function
               OpStore %29 %14
               OpStore %30 %10
         %36 = OpLoad %7 %30
         %38 = OpLoad %8 %29
         %39 = OpCompositeConstruct %19 %38
         %40 = OpLoad %7 %30
         %42 = OpLoad %8 %29
         %43 = OpCompositeConstruct %19 %42
         %45 = OpLoad %7 %30
         %46 = OpLoad %7 %33
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Bad: no ids given for clamping
  ASSERT_FALSE(TransformationAccessChain(
                   100, 29, {17}, MakeInstructionDescriptor(36, SpvOpLoad, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Bad: an id given for clamping is not fresh
  ASSERT_FALSE(TransformationAccessChain(
                   100, 29, {17}, MakeInstructionDescriptor(36, SpvOpLoad, 0),
                   {{46, 201}})
                   .IsApplicable(context.get(), transformation_context));

  // Bad: an id given for clamping is not fresh
  ASSERT_FALSE(TransformationAccessChain(
                   100, 29, {17}, MakeInstructionDescriptor(36, SpvOpLoad, 0),
                   {{200, 46}})
                   .IsApplicable(context.get(), transformation_context));

  // Bad: an id given for clamping is the same as the id for the access chain
  ASSERT_FALSE(TransformationAccessChain(
                   100, 29, {17}, MakeInstructionDescriptor(36, SpvOpLoad, 0),
                   {{100, 201}})
                   .IsApplicable(context.get(), transformation_context));

  // Bad: the fresh ids given are not distinct
  ASSERT_FALSE(TransformationAccessChain(
                   100, 29, {17}, MakeInstructionDescriptor(36, SpvOpLoad, 0),
                   {{200, 200}})
                   .IsApplicable(context.get(), transformation_context));

  // Bad: not enough ids given for clamping (2 pairs needed)
  ASSERT_FALSE(
      TransformationAccessChain(104, 34, {45, 10, 46},
                                MakeInstructionDescriptor(46, SpvOpReturn, 0),
                                {{208, 209}, {209, 211}})
          .IsApplicable(context.get(), transformation_context));

  // Bad: the fresh ids given are not distinct
  ASSERT_FALSE(
      TransformationAccessChain(104, 34, {45, 10, 46},
                                MakeInstructionDescriptor(46, SpvOpReturn, 0),
                                {{208, 209}, {209, 211}})
          .IsApplicable(context.get(), transformation_context));

  {
    TransformationAccessChain transformation(
        100, 29, {17}, MakeInstructionDescriptor(36, SpvOpLoad, 0),
        {{200, 201}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  {
    TransformationAccessChain transformation(
        101, 29, {36}, MakeInstructionDescriptor(38, SpvOpLoad, 0),
        {{202, 203}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  {
    TransformationAccessChain transformation(
        102, 32, {10, 40}, MakeInstructionDescriptor(42, SpvOpLoad, 0),
        {{204, 205}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  {
    TransformationAccessChain transformation(
        103, 34, {11}, MakeInstructionDescriptor(45, SpvOpLoad, 0),
        {{206, 207}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  {
    TransformationAccessChain transformation(
        104, 34, {45, 10, 46}, MakeInstructionDescriptor(46, SpvOpReturn, 0),
        {{208, 209}, {210, 211}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main" %3
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %4 = OpTypeVoid
          %5 = OpTypeBool
          %6 = OpTypeFunction %4
          %7 = OpTypeInt 32 1
          %8 = OpTypeVector %7 4
          %9 = OpTypePointer Function %8
         %10 = OpConstant %7 0
         %11 = OpConstant %7 1
         %12 = OpConstant %7 3
         %13 = OpConstant %7 2
         %14 = OpConstantComposite %8 %10 %11 %12 %13
         %15 = OpTypePointer Function %7
         %16 = OpTypeInt 32 0
         %17 = OpConstant %16 1
         %18 = OpConstant %16 3
         %19 = OpTypeStruct %8
         %20 = OpTypePointer Function %19
         %21 = OpConstant %7 9
         %22 = OpConstant %16 10
         %23 = OpTypeArray %19 %22
         %24 = OpTypePointer Function %23
         %25 = OpTypeFloat 32
         %26 = OpTypeVector %25 4
         %27 = OpTypePointer Output %26
          %3 = OpVariable %27 Output
          %2 = OpFunction %4 None %6
         %28 = OpLabel
         %29 = OpVariable %9 Function
         %30 = OpVariable %15 Function
         %31 = OpVariable %15 Function
         %32 = OpVariable %20 Function
         %33 = OpVariable %15 Function
         %34 = OpVariable %24 Function
               OpStore %29 %14
               OpStore %30 %10
        %200 = OpULessThanEqual %5 %17 %18
        %201 = OpSelect %16 %200 %17 %18
        %100 = OpAccessChain %15 %29 %201
         %36 = OpLoad %7 %30
        %202 = OpULessThanEqual %5 %36 %12
        %203 = OpSelect %7 %202 %36 %12
        %101 = OpAccessChain %15 %29 %203
         %38 = OpLoad %8 %29
         %39 = OpCompositeConstruct %19 %38
         %40 = OpLoad %7 %30
        %204 = OpULessThanEqual %5 %40 %12
        %205 = OpSelect %7 %204 %40 %12
        %102 = OpAccessChain %15 %32 %10 %205
         %42 = OpLoad %8 %29
         %43 = OpCompositeConstruct %19 %42
        %206 = OpULessThanEqual %5 %11 %21
        %207 = OpSelect %7 %206 %11 %21
        %103 = OpAccessChain %20 %34 %207
         %45 = OpLoad %7 %30
         %46 = OpLoad %7 %33
        %208 = OpULessThanEqual %5 %45 %21
        %209 = OpSelect %7 %208 %45 %21
        %210 = OpULessThanEqual %5 %46 %12
        %211 = OpSelect %7 %210 %46 %12
        %104 = OpAccessChain %15 %34 %209 %10 %211
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
