// Copyright (c) 2021 Shiyu Liu
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

#include "source/fuzz/transformation_wrap_vector_synonym.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationWrapVectorSynonym, BasicTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %97
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 10
         %11 = OpConstant %6 -5
         %12 = OpTypeVector %6 2
         %13 = OpTypePointer Function %12
         %18 = OpTypeInt 32 0
         %19 = OpTypePointer Function %18
         %21 = OpConstant %18 8
         %23 = OpConstant %18 2
         %24 = OpTypeVector %18 3
         %25 = OpTypePointer Function %24
         %31 = OpTypeFloat 32
         %32 = OpTypePointer Function %31
         %34 = OpConstant %31 3.29999995
         %36 = OpConstant %31 1.10000002
         %37 = OpTypeVector %31 4
         %38 = OpTypePointer Function %37
         %96 = OpTypePointer Input %31
         %97 = OpVariable %96 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %20 = OpVariable %19 Function
         %22 = OpVariable %19 Function
         %26 = OpVariable %25 Function
         %33 = OpVariable %32 Function
         %35 = OpVariable %32 Function
         %39 = OpVariable %38 Function
         %47 = OpVariable %7 Function
         %51 = OpVariable %7 Function
         %55 = OpVariable %7 Function
         %59 = OpVariable %7 Function
         %63 = OpVariable %19 Function
         %67 = OpVariable %19 Function
         %71 = OpVariable %19 Function
         %75 = OpVariable %19 Function
         %79 = OpVariable %32 Function
         %83 = OpVariable %32 Function
         %87 = OpVariable %32 Function
         %91 = OpVariable %32 Function
               OpStore %8 %9
               OpStore %10 %11
         %15 = OpLoad %6 %8
         %16 = OpLoad %6 %10
         %17 = OpCompositeConstruct %12 %15 %16
               OpStore %14 %17
               OpStore %20 %21
               OpStore %22 %23
         %27 = OpLoad %18 %20
         %28 = OpLoad %18 %20
         %29 = OpLoad %18 %22
         %30 = OpCompositeConstruct %24 %27 %28 %29
               OpStore %26 %30
               OpStore %33 %34
               OpStore %35 %36
         %40 = OpLoad %31 %33
         %41 = OpLoad %31 %33
         %42 = OpLoad %31 %35
         %43 = OpLoad %31 %35
         %44 = OpCompositeConstruct %37 %40 %41 %42 %43
         %45 = OpLoad %37 %39
         %46 = OpVectorShuffle %37 %45 %44 5 6 7 4
               OpStore %39 %46
         %48 = OpLoad %6 %8
         %49 = OpLoad %6 %10
        %100 = OpCompositeConstruct %12 %48 %48
        %101 = OpCompositeConstruct %12 %49 %49
         %50 = OpIAdd %6 %48 %49
               OpStore %47 %50
         %52 = OpLoad %6 %8
         %53 = OpLoad %6 %10
         %54 = OpISub %6 %52 %53
               OpStore %51 %54
         %56 = OpLoad %6 %8
         %57 = OpLoad %6 %10
         %58 = OpIMul %6 %56 %57
               OpStore %55 %58
         %60 = OpLoad %6 %8
         %61 = OpLoad %6 %10
         %62 = OpSDiv %6 %60 %61
               OpStore %59 %62
         %64 = OpLoad %18 %20
         %65 = OpLoad %18 %22
         %66 = OpIAdd %18 %64 %65
               OpStore %63 %66
         %68 = OpLoad %18 %20
         %69 = OpLoad %18 %22
         %70 = OpISub %18 %68 %69
               OpStore %67 %70
         %72 = OpLoad %18 %20
         %73 = OpLoad %18 %22
         %74 = OpIMul %18 %72 %73
               OpStore %71 %74
         %76 = OpLoad %18 %20
         %77 = OpLoad %18 %22
         %78 = OpUDiv %18 %76 %77
               OpStore %75 %78
         %80 = OpLoad %31 %33
         %81 = OpLoad %31 %35
         %82 = OpFAdd %31 %80 %81
               OpStore %79 %82
         %84 = OpLoad %31 %33
         %85 = OpLoad %31 %35
         %86 = OpFSub %31 %84 %85
               OpStore %83 %86
         %88 = OpLoad %31 %33
         %89 = OpLoad %31 %35
         %90 = OpFMul %31 %88 %89
               OpStore %87 %90
         %92 = OpLoad %31 %33
         %93 = OpLoad %31 %35
         %94 = OpFDiv %31 %92 %93
               OpStore %91 %94
               OpReturn
               OpFunctionEnd
  )";
  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;

  // Check context validity.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  // Vec Type Id |   Vector Type  |  Element Type id |   Element Type  |
  // ------------+----------------+------------------+-----------------+
  //     12      |      vec2      |         6        |      int32      |
  //     24      |      vec3      |        18        |     uint32      |
  //     37      |      vec4      |        31        |      float      |

  // Instruction Id | Opcode  | Type Id | constant id 1 | constant id 2 |
  // ---------------+---------+---------+---------------+---------------+
  //       50       | OpIAdd  |    6    |      48       |      49       |
  //       54       | OpISub  |    6    |      52       |      53       |
  //       58       | OpIMul  |    6    |      56       |      57       |
  //       62       | OpSDiv  |    6    |      60       |      61       |
  //       66       | OpIAdd  |    18   |      64       |      65       |
  //       70       | OpISub  |    18   |      68       |      69       |
  //       74       | OpIMul  |    18   |      72       |      73       |
  //       78       | OpUDiv  |    18   |      76       |      77       |
  //       82       | OpFAdd  |    31   |      80       |      81       |
  //       86       | OpFSub  |    31   |      84       |      85       |
  //       90       | OpFMul  |    31   |      88       |      89       |
  //       94       | OpFDiv  |    31   |      92       |      93       |

  // Assert that the target scalar instruction result id is relevant.
  ASSERT_FALSE(transformation_context.GetFactManager()->IdIsIrrelevant(50));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(100, {1}), MakeDataDescriptor(48, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(101, {1}), MakeDataDescriptor(49, {}));

  // The following are all invalid use.
  {
    // Bad: Instruction id does not exist.
    TransformationWrapVectorSynonym wrap_add_int_bad1(103, 100, 101, 102, 1);
    ASSERT_FALSE(
        wrap_add_int_bad1.IsApplicable(context.get(), transformation_context));

    // Bad: Instruction id given is not of a valid arithmetic operation typed
    // instruction.
    TransformationWrapVectorSynonym wrap_add_int_bad2(80, 100, 101, 102, 1);
    ASSERT_FALSE(
        wrap_add_int_bad1.IsApplicable(context.get(), transformation_context));

    // Bad: the id for the first vector does not exist.
    TransformationWrapVectorSynonym wrap_add_int_bad3(50, 105, 101, 102, 1);
    ASSERT_FALSE(
        wrap_add_int_bad3.IsApplicable(context.get(), transformation_context));

    // Bad: the id for the second vector does not exist.
    TransformationWrapVectorSynonym wrap_add_int_bad4(50, 100, 105, 102, 1);
    ASSERT_FALSE(
        wrap_add_int_bad4.IsApplicable(context.get(), transformation_context));

    // Bad: vector id is not fresh.
    TransformationWrapVectorSynonym wrap_add_int_bad6(50, 100, 101, 94, 1);
    ASSERT_FALSE(
        wrap_add_int_bad6.IsApplicable(context.get(), transformation_context));

    // Bad: The position goes out of bound for the given vector type.
    TransformationWrapVectorSynonym wrap_add_int_bad8(50, 100, 101, 102, 2);
    ASSERT_FALSE(
        wrap_add_int_bad8.IsApplicable(context.get(), transformation_context));

    // Bad: The original instruction is not a valid scalar operation
    // instruction.
    TransformationWrapVectorSynonym wrap_add_int(27, 100, 101, 102, 1);
    ASSERT_FALSE(
        wrap_add_int.IsApplicable(context.get(), transformation_context));
  }

  // Good: The following transformation should be applicable.
  TransformationWrapVectorSynonym wrap_add_int(50, 100, 101, 102, 1);
  ASSERT_TRUE(wrap_add_int.IsApplicable(context.get(), transformation_context));
  // Insert an arithmetic instruction of the same type to add two vectors.
  ApplyAndCheckFreshIds(wrap_add_int, context.get(), &transformation_context);

  // |instruction_id| and id at |scalar_position of the result vector should be
  // synonyms.
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(102, {1}), MakeDataDescriptor(50, {})));

  // After applying transformations, the instruction:
  //
  // %102 = OpIAdd %12 %100 %101
  //
  // should be added before:
  //
  // %50 = OpIAdd %6 %48 %49
  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %97
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 10
         %11 = OpConstant %6 -5
         %12 = OpTypeVector %6 2
         %13 = OpTypePointer Function %12
         %18 = OpTypeInt 32 0
         %19 = OpTypePointer Function %18
         %21 = OpConstant %18 8
         %23 = OpConstant %18 2
         %24 = OpTypeVector %18 3
         %25 = OpTypePointer Function %24
         %31 = OpTypeFloat 32
         %32 = OpTypePointer Function %31
         %34 = OpConstant %31 3.29999995
         %36 = OpConstant %31 1.10000002
         %37 = OpTypeVector %31 4
         %38 = OpTypePointer Function %37
         %96 = OpTypePointer Input %31
         %97 = OpVariable %96 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %20 = OpVariable %19 Function
         %22 = OpVariable %19 Function
         %26 = OpVariable %25 Function
         %33 = OpVariable %32 Function
         %35 = OpVariable %32 Function
         %39 = OpVariable %38 Function
         %47 = OpVariable %7 Function
         %51 = OpVariable %7 Function
         %55 = OpVariable %7 Function
         %59 = OpVariable %7 Function
         %63 = OpVariable %19 Function
         %67 = OpVariable %19 Function
         %71 = OpVariable %19 Function
         %75 = OpVariable %19 Function
         %79 = OpVariable %32 Function
         %83 = OpVariable %32 Function
         %87 = OpVariable %32 Function
         %91 = OpVariable %32 Function
               OpStore %8 %9
               OpStore %10 %11
         %15 = OpLoad %6 %8
         %16 = OpLoad %6 %10
         %17 = OpCompositeConstruct %12 %15 %16
               OpStore %14 %17
               OpStore %20 %21
               OpStore %22 %23
         %27 = OpLoad %18 %20
         %28 = OpLoad %18 %20
         %29 = OpLoad %18 %22
         %30 = OpCompositeConstruct %24 %27 %28 %29
               OpStore %26 %30
               OpStore %33 %34
               OpStore %35 %36
         %40 = OpLoad %31 %33
         %41 = OpLoad %31 %33
         %42 = OpLoad %31 %35
         %43 = OpLoad %31 %35
         %44 = OpCompositeConstruct %37 %40 %41 %42 %43
         %45 = OpLoad %37 %39
         %46 = OpVectorShuffle %37 %45 %44 5 6 7 4
               OpStore %39 %46
         %48 = OpLoad %6 %8
         %49 = OpLoad %6 %10
        %100 = OpCompositeConstruct %12 %48 %48
        %101 = OpCompositeConstruct %12 %49 %49
        %102 = OpIAdd %12 %100 %101
         %50 = OpIAdd %6 %48 %49
               OpStore %47 %50
         %52 = OpLoad %6 %8
         %53 = OpLoad %6 %10
         %54 = OpISub %6 %52 %53
               OpStore %51 %54
         %56 = OpLoad %6 %8
         %57 = OpLoad %6 %10
         %58 = OpIMul %6 %56 %57
               OpStore %55 %58
         %60 = OpLoad %6 %8
         %61 = OpLoad %6 %10
         %62 = OpSDiv %6 %60 %61
               OpStore %59 %62
         %64 = OpLoad %18 %20
         %65 = OpLoad %18 %22
         %66 = OpIAdd %18 %64 %65
               OpStore %63 %66
         %68 = OpLoad %18 %20
         %69 = OpLoad %18 %22
         %70 = OpISub %18 %68 %69
               OpStore %67 %70
         %72 = OpLoad %18 %20
         %73 = OpLoad %18 %22
         %74 = OpIMul %18 %72 %73
               OpStore %71 %74
         %76 = OpLoad %18 %20
         %77 = OpLoad %18 %22
         %78 = OpUDiv %18 %76 %77
               OpStore %75 %78
         %80 = OpLoad %31 %33
         %81 = OpLoad %31 %35
         %82 = OpFAdd %31 %80 %81
               OpStore %79 %82
         %84 = OpLoad %31 %33
         %85 = OpLoad %31 %35
         %86 = OpFSub %31 %84 %85
               OpStore %83 %86
         %88 = OpLoad %31 %33
         %89 = OpLoad %31 %35
         %90 = OpFMul %31 %88 %89
               OpStore %87 %90
         %92 = OpLoad %31 %33
         %93 = OpLoad %31 %35
         %94 = OpFDiv %31 %92 %93
               OpStore %91 %94
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationWrapVectorSynonym, OperationSupportTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %97
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 10
         %11 = OpConstant %6 -5
         %12 = OpTypeVector %6 2
         %13 = OpTypePointer Function %12
         %18 = OpTypeInt 32 0
         %19 = OpTypePointer Function %18
         %21 = OpConstant %18 8
         %23 = OpConstant %18 2
         %24 = OpTypeVector %18 3
         %25 = OpTypePointer Function %24
         %31 = OpTypeFloat 32
         %32 = OpTypePointer Function %31
         %34 = OpConstant %31 3.29999995
         %36 = OpConstant %31 1.10000002
         %37 = OpTypeVector %31 4
         %38 = OpTypePointer Function %37
         %96 = OpTypePointer Input %31
         %97 = OpVariable %96 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %20 = OpVariable %19 Function
         %22 = OpVariable %19 Function
         %26 = OpVariable %25 Function
         %33 = OpVariable %32 Function
         %35 = OpVariable %32 Function
         %39 = OpVariable %38 Function
         %47 = OpVariable %7 Function
         %51 = OpVariable %7 Function
         %55 = OpVariable %7 Function
         %59 = OpVariable %7 Function
         %63 = OpVariable %19 Function
         %67 = OpVariable %19 Function
         %71 = OpVariable %19 Function
         %75 = OpVariable %19 Function
         %79 = OpVariable %32 Function
         %83 = OpVariable %32 Function
         %87 = OpVariable %32 Function
         %91 = OpVariable %32 Function
               OpStore %8 %9
               OpStore %10 %11
         %15 = OpLoad %6 %8
         %16 = OpLoad %6 %10
         %17 = OpCompositeConstruct %12 %15 %16
               OpStore %14 %17
               OpStore %20 %21
               OpStore %22 %23
         %27 = OpLoad %18 %20
         %28 = OpLoad %18 %20
         %29 = OpLoad %18 %22
         %30 = OpCompositeConstruct %24 %27 %28 %29
               OpStore %26 %30
               OpStore %33 %34
               OpStore %35 %36
         %40 = OpLoad %31 %33
         %41 = OpLoad %31 %33
         %42 = OpLoad %31 %35
         %43 = OpLoad %31 %35
         %44 = OpCompositeConstruct %37 %40 %41 %42 %43
         %45 = OpLoad %37 %39
         %46 = OpVectorShuffle %37 %45 %44 5 6 7 4
               OpStore %39 %46
         %48 = OpLoad %6 %8
         %49 = OpLoad %6 %10
         %50 = OpIAdd %6 %48 %49
               OpStore %47 %50
         %52 = OpLoad %6 %8
         %53 = OpLoad %6 %10
        %100 = OpCompositeConstruct %12 %52 %52
        %101 = OpCompositeConstruct %12 %53 %53
         %54 = OpISub %6 %52 %53
               OpStore %51 %54
         %56 = OpLoad %6 %8
         %57 = OpLoad %6 %10
        %103 = OpCompositeConstruct %12 %56 %56
        %104 = OpCompositeConstruct %12 %57 %57
         %58 = OpIMul %6 %56 %57
               OpStore %55 %58
         %60 = OpLoad %6 %8
         %61 = OpLoad %6 %10
         %62 = OpSDiv %6 %60 %61
               OpStore %59 %62
         %64 = OpLoad %18 %20
         %65 = OpLoad %18 %22
        %106 = OpCompositeConstruct %24 %64 %64 %64
        %107 = OpCompositeConstruct %24 %65 %65 %65
         %66 = OpIAdd %18 %64 %65
               OpStore %63 %66
         %68 = OpLoad %18 %20
         %69 = OpLoad %18 %22
        %109 = OpCompositeConstruct %24 %68 %68 %68
        %110 = OpCompositeConstruct %24 %69 %69 %69
         %70 = OpISub %18 %68 %69
               OpStore %67 %70
         %72 = OpLoad %18 %20
         %73 = OpLoad %18 %22
        %112 = OpCompositeConstruct %24 %72 %72 %72
        %113 = OpCompositeConstruct %24 %73 %73 %73
         %74 = OpIMul %18 %72 %73
               OpStore %71 %74
         %76 = OpLoad %18 %20
         %77 = OpLoad %18 %22
         %78 = OpUDiv %18 %76 %77
               OpStore %75 %78
         %80 = OpLoad %31 %33
         %81 = OpLoad %31 %35
        %115 = OpCompositeConstruct %37 %80 %80 %80 %80
        %116 = OpCompositeConstruct %37 %81 %81 %81 %81
         %82 = OpFAdd %31 %80 %81
               OpStore %79 %82
         %84 = OpLoad %31 %33
         %85 = OpLoad %31 %35
        %118 = OpCompositeConstruct %37 %84 %84 %84 %84
        %119 = OpCompositeConstruct %37 %85 %85 %85 %85
         %86 = OpFSub %31 %84 %85
               OpStore %83 %86
         %88 = OpLoad %31 %33
         %89 = OpLoad %31 %35
        %121 = OpCompositeConstruct %37 %88 %88 %88 %88
        %122 = OpCompositeConstruct %37 %89 %89 %89 %89
         %90 = OpFMul %31 %88 %89
               OpStore %87 %90
         %92 = OpLoad %31 %33
         %93 = OpLoad %31 %35
         %94 = OpFDiv %31 %92 %93
               OpStore %91 %94
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

  {
    // Add synonym facts between the vector operands at pos and the operands to
    // the scalar instruction.
    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(100, {1}), MakeDataDescriptor(52, {}));
    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(101, {1}), MakeDataDescriptor(53, {}));

    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(103, {0}), MakeDataDescriptor(56, {}));
    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(104, {0}), MakeDataDescriptor(57, {}));

    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(106, {2}), MakeDataDescriptor(64, {}));
    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(107, {2}), MakeDataDescriptor(65, {}));

    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(109, {2}), MakeDataDescriptor(68, {}));
    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(110, {2}), MakeDataDescriptor(69, {}));

    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(112, {1}), MakeDataDescriptor(72, {}));
    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(113, {1}), MakeDataDescriptor(73, {}));

    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(115, {2}), MakeDataDescriptor(80, {}));
    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(116, {2}), MakeDataDescriptor(81, {}));

    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(118, {3}), MakeDataDescriptor(84, {}));
    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(119, {3}), MakeDataDescriptor(85, {}));

    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(121, {1}), MakeDataDescriptor(88, {}));
    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(122, {1}), MakeDataDescriptor(89, {}));
  }

  // Test OpISub for signed integer.
  {
    // Good: The following transformation should be applicable.
    TransformationWrapVectorSynonym wrap_sub_int(54, 100, 101, 102, 1);
    ASSERT_TRUE(
        wrap_sub_int.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(wrap_sub_int, context.get(), &transformation_context);

    // |instruction_id| and id at |scalar_position of the result vector should
    // be synonyms.
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(102, {1}), MakeDataDescriptor(54, {})));
  }

  // Test OpIMul for signed integer.
  {
    // Good: The following transformation should be applicable.
    TransformationWrapVectorSynonym wrap_mul_int(58, 103, 104, 105, 0);
    ASSERT_TRUE(
        wrap_mul_int.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(wrap_mul_int, context.get(), &transformation_context);

    // |instruction_id| and id at |scalar_position of the result vector should
    // be synonyms.
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(105, {0}), MakeDataDescriptor(58, {})));
  }

  // Test OpIAdd for unsigned integer.
  {
    // Good: The following transformation should be applicable.
    TransformationWrapVectorSynonym wrap_add_uint(66, 106, 107, 108, 2);
    ASSERT_TRUE(
        wrap_add_uint.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(wrap_add_uint, context.get(),
                          &transformation_context);

    // |instruction_id| and id at |scalar_position of the result vector should
    // be synonyms.
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(108, {2}), MakeDataDescriptor(66, {})));
  }

  // Test OpISub for signed integer.
  {
    // Good: The following transformation should be applicable.
    TransformationWrapVectorSynonym wrap_sub_uint(70, 109, 110, 111, 2);
    ASSERT_TRUE(
        wrap_sub_uint.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(wrap_sub_uint, context.get(),
                          &transformation_context);

    // |instruction_id| and id at |scalar_position of the result vector should
    // be synonyms.
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(111, {2}), MakeDataDescriptor(70, {})));
  }

  // Test OpIMul for signed integer.
  {
    // Good: The following transformation should be applicable.
    TransformationWrapVectorSynonym wrap_mul_uint(74, 112, 113, 114, 1);
    ASSERT_TRUE(
        wrap_mul_uint.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(wrap_mul_uint, context.get(),
                          &transformation_context);

    // |instruction_id| and id at |scalar_position of the result vector should
    // be synonyms.
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(114, {1}), MakeDataDescriptor(74, {})));
  }

  // Test OpFAdd for float.
  {
    // Good: The following transformation should be applicable.
    TransformationWrapVectorSynonym wrap_add_float(82, 115, 116, 117, 2);
    ASSERT_TRUE(
        wrap_add_float.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(wrap_add_float, context.get(),
                          &transformation_context);

    // |instruction_id| and id at |scalar_position of the result vector should
    // be synonyms.
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(117, {2}), MakeDataDescriptor(82, {})));
  }

  // Test OpFSub for float.
  {
    // Good: The following transformation should be applicable.
    TransformationWrapVectorSynonym wrap_add_float(86, 118, 119, 120, 3);
    ASSERT_TRUE(
        wrap_add_float.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(wrap_add_float, context.get(),
                          &transformation_context);

    // |instruction_id| and id at |scalar_position of the result vector should
    // be synonyms.
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(120, {3}), MakeDataDescriptor(86, {})));
  }

  // Test OpFMul for float.
  {
    // Good: The following transformation should be applicable.
    TransformationWrapVectorSynonym wrap_mul_float(90, 121, 122, 123, 1);
    ASSERT_TRUE(
        wrap_mul_float.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(wrap_mul_float, context.get(),
                          &transformation_context);

    // |instruction_id| and id at |scalar_position of the result vector should
    // be synonyms.
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(123, {1}), MakeDataDescriptor(90, {})));
  }

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %97
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 10
         %11 = OpConstant %6 -5
         %12 = OpTypeVector %6 2
         %13 = OpTypePointer Function %12
         %18 = OpTypeInt 32 0
         %19 = OpTypePointer Function %18
         %21 = OpConstant %18 8
         %23 = OpConstant %18 2
         %24 = OpTypeVector %18 3
         %25 = OpTypePointer Function %24
         %31 = OpTypeFloat 32
         %32 = OpTypePointer Function %31
         %34 = OpConstant %31 3.29999995
         %36 = OpConstant %31 1.10000002
         %37 = OpTypeVector %31 4
         %38 = OpTypePointer Function %37
         %96 = OpTypePointer Input %31
         %97 = OpVariable %96 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %20 = OpVariable %19 Function
         %22 = OpVariable %19 Function
         %26 = OpVariable %25 Function
         %33 = OpVariable %32 Function
         %35 = OpVariable %32 Function
         %39 = OpVariable %38 Function
         %47 = OpVariable %7 Function
         %51 = OpVariable %7 Function
         %55 = OpVariable %7 Function
         %59 = OpVariable %7 Function
         %63 = OpVariable %19 Function
         %67 = OpVariable %19 Function
         %71 = OpVariable %19 Function
         %75 = OpVariable %19 Function
         %79 = OpVariable %32 Function
         %83 = OpVariable %32 Function
         %87 = OpVariable %32 Function
         %91 = OpVariable %32 Function
               OpStore %8 %9
               OpStore %10 %11
         %15 = OpLoad %6 %8
         %16 = OpLoad %6 %10
         %17 = OpCompositeConstruct %12 %15 %16
               OpStore %14 %17
               OpStore %20 %21
               OpStore %22 %23
         %27 = OpLoad %18 %20
         %28 = OpLoad %18 %20
         %29 = OpLoad %18 %22
         %30 = OpCompositeConstruct %24 %27 %28 %29
               OpStore %26 %30
               OpStore %33 %34
               OpStore %35 %36
         %40 = OpLoad %31 %33
         %41 = OpLoad %31 %33
         %42 = OpLoad %31 %35
         %43 = OpLoad %31 %35
         %44 = OpCompositeConstruct %37 %40 %41 %42 %43
         %45 = OpLoad %37 %39
         %46 = OpVectorShuffle %37 %45 %44 5 6 7 4
               OpStore %39 %46
         %48 = OpLoad %6 %8
         %49 = OpLoad %6 %10
         %50 = OpIAdd %6 %48 %49
               OpStore %47 %50
         %52 = OpLoad %6 %8
         %53 = OpLoad %6 %10
        %100 = OpCompositeConstruct %12 %52 %52
        %101 = OpCompositeConstruct %12 %53 %53
        %102 = OpISub %12 %100 %101
         %54 = OpISub %6 %52 %53
               OpStore %51 %54
         %56 = OpLoad %6 %8
         %57 = OpLoad %6 %10
        %103 = OpCompositeConstruct %12 %56 %56
        %104 = OpCompositeConstruct %12 %57 %57
        %105 = OpIMul %12 %103 %104
         %58 = OpIMul %6 %56 %57
               OpStore %55 %58
         %60 = OpLoad %6 %8
         %61 = OpLoad %6 %10
         %62 = OpSDiv %6 %60 %61
               OpStore %59 %62
         %64 = OpLoad %18 %20
         %65 = OpLoad %18 %22
        %106 = OpCompositeConstruct %24 %64 %64 %64
        %107 = OpCompositeConstruct %24 %65 %65 %65
        %108 = OpIAdd %24 %106 %107
         %66 = OpIAdd %18 %64 %65
               OpStore %63 %66
         %68 = OpLoad %18 %20
         %69 = OpLoad %18 %22
        %109 = OpCompositeConstruct %24 %68 %68 %68
        %110 = OpCompositeConstruct %24 %69 %69 %69
        %111 = OpISub %24 %109 %110
         %70 = OpISub %18 %68 %69
               OpStore %67 %70
         %72 = OpLoad %18 %20
         %73 = OpLoad %18 %22
        %112 = OpCompositeConstruct %24 %72 %72 %72
        %113 = OpCompositeConstruct %24 %73 %73 %73
        %114 = OpIMul %24 %112 %113
         %74 = OpIMul %18 %72 %73
               OpStore %71 %74
         %76 = OpLoad %18 %20
         %77 = OpLoad %18 %22
         %78 = OpUDiv %18 %76 %77
               OpStore %75 %78
         %80 = OpLoad %31 %33
         %81 = OpLoad %31 %35
        %115 = OpCompositeConstruct %37 %80 %80 %80 %80
        %116 = OpCompositeConstruct %37 %81 %81 %81 %81
        %117 = OpFAdd %37 %115 %116
         %82 = OpFAdd %31 %80 %81
               OpStore %79 %82
         %84 = OpLoad %31 %33
         %85 = OpLoad %31 %35
        %118 = OpCompositeConstruct %37 %84 %84 %84 %84
        %119 = OpCompositeConstruct %37 %85 %85 %85 %85
        %120 = OpFSub %37 %118 %119
         %86 = OpFSub %31 %84 %85
               OpStore %83 %86
         %88 = OpLoad %31 %33
         %89 = OpLoad %31 %35
        %121 = OpCompositeConstruct %37 %88 %88 %88 %88
        %122 = OpCompositeConstruct %37 %89 %89 %89 %89
        %123 = OpFMul %37 %121 %122
         %90 = OpFMul %31 %88 %89
               OpStore %87 %90
         %92 = OpLoad %31 %33
         %93 = OpLoad %31 %35
         %94 = OpFDiv %31 %92 %93
               OpStore %91 %94
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationWrapVectorSynonym, DivSupportTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %97
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 10
         %11 = OpConstant %6 -5
         %12 = OpTypeVector %6 2
         %13 = OpTypePointer Function %12
         %18 = OpTypeInt 32 0
         %19 = OpTypePointer Function %18
         %21 = OpConstant %18 8
         %23 = OpConstant %18 2
         %24 = OpTypeVector %18 3
         %25 = OpTypePointer Function %24
         %31 = OpTypeFloat 32
         %32 = OpTypePointer Function %31
         %34 = OpConstant %31 3.29999995
         %36 = OpConstant %31 1.10000002
         %37 = OpTypeVector %31 4
         %38 = OpTypePointer Function %37
         %96 = OpTypePointer Input %31
         %97 = OpVariable %96 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %20 = OpVariable %19 Function
         %22 = OpVariable %19 Function
         %26 = OpVariable %25 Function
         %33 = OpVariable %32 Function
         %35 = OpVariable %32 Function
         %39 = OpVariable %38 Function
         %47 = OpVariable %7 Function
         %51 = OpVariable %7 Function
         %55 = OpVariable %7 Function
         %59 = OpVariable %7 Function
         %63 = OpVariable %19 Function
         %67 = OpVariable %19 Function
         %71 = OpVariable %19 Function
         %75 = OpVariable %19 Function
         %79 = OpVariable %32 Function
         %83 = OpVariable %32 Function
         %87 = OpVariable %32 Function
         %91 = OpVariable %32 Function
               OpStore %8 %9
               OpStore %10 %11
         %15 = OpLoad %6 %8
         %16 = OpLoad %6 %10
         %17 = OpCompositeConstruct %12 %15 %16
               OpStore %14 %17
               OpStore %20 %21
               OpStore %22 %23
         %27 = OpLoad %18 %20
         %28 = OpLoad %18 %20
         %29 = OpLoad %18 %22
         %30 = OpCompositeConstruct %24 %27 %28 %29
               OpStore %26 %30
               OpStore %33 %34
               OpStore %35 %36
         %40 = OpLoad %31 %33
         %41 = OpLoad %31 %33
         %42 = OpLoad %31 %35
         %43 = OpLoad %31 %35
         %44 = OpCompositeConstruct %37 %40 %41 %42 %43
         %45 = OpLoad %37 %39
         %46 = OpVectorShuffle %37 %45 %44 5 6 7 4
               OpStore %39 %46
         %48 = OpLoad %6 %8
         %49 = OpLoad %6 %10
         %50 = OpIAdd %6 %48 %49
               OpStore %47 %50
         %52 = OpLoad %6 %8
         %53 = OpLoad %6 %10
         %54 = OpISub %6 %52 %53
               OpStore %51 %54
         %56 = OpLoad %6 %8
         %57 = OpLoad %6 %10
         %58 = OpIMul %6 %56 %57
               OpStore %55 %58
         %60 = OpLoad %6 %8
         %61 = OpLoad %6 %10
        %100 = OpCompositeConstruct %12 %60 %60
        %101 = OpCompositeConstruct %12 %61 %61
         %62 = OpSDiv %6 %60 %61
               OpStore %59 %62
         %64 = OpLoad %18 %20
         %65 = OpLoad %18 %22
         %66 = OpIAdd %18 %64 %65
               OpStore %63 %66
         %68 = OpLoad %18 %20
         %69 = OpLoad %18 %22
         %70 = OpISub %18 %68 %69
               OpStore %67 %70
         %72 = OpLoad %18 %20
         %73 = OpLoad %18 %22
         %74 = OpIMul %18 %72 %73
               OpStore %71 %74
         %76 = OpLoad %18 %20
         %77 = OpLoad %18 %22
        %102 = OpCompositeConstruct %24 %76 %76 %76
        %103 = OpCompositeConstruct %24 %77 %77 %77
         %78 = OpUDiv %18 %76 %77
               OpStore %75 %78
         %80 = OpLoad %31 %33
         %81 = OpLoad %31 %35
         %82 = OpFAdd %31 %80 %81
               OpStore %79 %82
         %84 = OpLoad %31 %33
         %85 = OpLoad %31 %35
         %86 = OpFSub %31 %84 %85
               OpStore %83 %86
         %88 = OpLoad %31 %33
         %89 = OpLoad %31 %35
         %90 = OpFMul %31 %88 %89
               OpStore %87 %90
         %92 = OpLoad %31 %33
         %93 = OpLoad %31 %35
        %104 = OpCompositeConstruct %37 %92 %92 %92 %92
        %105 = OpCompositeConstruct %37 %93 %93 %93 %93
         %94 = OpFDiv %31 %92 %93
               OpStore %91 %94
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

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(100, {1}), MakeDataDescriptor(60, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(101, {1}), MakeDataDescriptor(61, {}));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(102, {1}), MakeDataDescriptor(76, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(103, {1}), MakeDataDescriptor(77, {}));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(104, {1}), MakeDataDescriptor(92, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(105, {1}), MakeDataDescriptor(93, {}));

  // Div operations are not currently supported.
  {
    TransformationWrapVectorSynonym wrap_div_bad1(62, 100, 101, 106, 1);
    ASSERT_FALSE(
        wrap_div_bad1.IsApplicable(context.get(), transformation_context));

    TransformationWrapVectorSynonym wrap_div_bad2(78, 102, 103, 106, 1);
    ASSERT_FALSE(
        wrap_div_bad2.IsApplicable(context.get(), transformation_context));

    TransformationWrapVectorSynonym wrap_div_bad3(94, 104, 105, 106, 1);
    ASSERT_FALSE(
        wrap_div_bad3.IsApplicable(context.get(), transformation_context));
  }
}

TEST(TransformationWrapVectorSynonym, AdditionalWidthSupportTest) {
  std::string shader = R"(
               OpCapability Shader
               OpCapability Int64
               OpCapability Float64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %97
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 64 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 10
         %11 = OpConstant %6 -5
         %12 = OpTypeVector %6 2
         %13 = OpTypePointer Function %12
         %18 = OpTypeInt 64 0
         %19 = OpTypePointer Function %18
         %21 = OpConstant %18 8
         %23 = OpConstant %18 2
         %24 = OpTypeVector %18 3
         %25 = OpTypePointer Function %24
         %31 = OpTypeFloat 64
         %32 = OpTypePointer Function %31
         %34 = OpConstant %31 3.29999995
         %36 = OpConstant %31 1.10000002
         %37 = OpTypeVector %31 4
         %38 = OpTypePointer Function %37
         %96 = OpTypePointer Input %31
         %97 = OpVariable %96 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %20 = OpVariable %19 Function
         %22 = OpVariable %19 Function
         %26 = OpVariable %25 Function
         %33 = OpVariable %32 Function
         %35 = OpVariable %32 Function
         %39 = OpVariable %38 Function
         %47 = OpVariable %7 Function
         %51 = OpVariable %7 Function
         %55 = OpVariable %7 Function
         %59 = OpVariable %7 Function
         %63 = OpVariable %19 Function
         %67 = OpVariable %19 Function
         %71 = OpVariable %19 Function
         %75 = OpVariable %19 Function
         %79 = OpVariable %32 Function
         %83 = OpVariable %32 Function
         %87 = OpVariable %32 Function
         %91 = OpVariable %32 Function
               OpStore %8 %9
               OpStore %10 %11
         %15 = OpLoad %6 %8
         %16 = OpLoad %6 %10
         %17 = OpCompositeConstruct %12 %15 %16
               OpStore %14 %17
               OpStore %20 %21
               OpStore %22 %23
         %27 = OpLoad %18 %20
         %28 = OpLoad %18 %20
         %29 = OpLoad %18 %22
         %30 = OpCompositeConstruct %24 %27 %28 %29
               OpStore %26 %30
               OpStore %33 %34
               OpStore %35 %36
         %40 = OpLoad %31 %33
         %41 = OpLoad %31 %33
         %42 = OpLoad %31 %35
         %43 = OpLoad %31 %35
         %44 = OpCompositeConstruct %37 %40 %41 %42 %43
         %45 = OpLoad %37 %39
         %46 = OpVectorShuffle %37 %45 %44 5 6 7 4
               OpStore %39 %46
         %48 = OpLoad %6 %8
         %49 = OpLoad %6 %10
        %100 = OpCompositeConstruct %12 %48 %48
        %101 = OpCompositeConstruct %12 %49 %49
         %50 = OpIAdd %6 %48 %49
               OpStore %47 %50
         %52 = OpLoad %6 %8
         %53 = OpLoad %6 %10
         %54 = OpISub %6 %52 %53
               OpStore %51 %54
         %56 = OpLoad %6 %8
         %57 = OpLoad %6 %10
         %58 = OpIMul %6 %56 %57
               OpStore %55 %58
         %60 = OpLoad %6 %8
         %61 = OpLoad %6 %10
         %62 = OpSDiv %6 %60 %61
               OpStore %59 %62
         %64 = OpLoad %18 %20
         %65 = OpLoad %18 %22
         %66 = OpIAdd %18 %64 %65
               OpStore %63 %66
         %68 = OpLoad %18 %20
         %69 = OpLoad %18 %22
        %103 = OpCompositeConstruct %24 %68 %68 %68
        %104 = OpCompositeConstruct %24 %69 %69 %69
         %70 = OpISub %18 %68 %69
               OpStore %67 %70
         %72 = OpLoad %18 %20
         %73 = OpLoad %18 %22
         %74 = OpIMul %18 %72 %73
               OpStore %71 %74
         %76 = OpLoad %18 %20
         %77 = OpLoad %18 %22
         %78 = OpUDiv %18 %76 %77
               OpStore %75 %78
         %80 = OpLoad %31 %33
         %81 = OpLoad %31 %35
         %82 = OpFAdd %31 %80 %81
               OpStore %79 %82
         %84 = OpLoad %31 %33
         %85 = OpLoad %31 %35
         %86 = OpFSub %31 %84 %85
               OpStore %83 %86
         %88 = OpLoad %31 %33
         %89 = OpLoad %31 %35
        %106 = OpCompositeConstruct %37 %88 %88 %88 %88
        %107 = OpCompositeConstruct %37 %89 %89 %89 %89
         %90 = OpFMul %31 %88 %89
               OpStore %87 %90
         %92 = OpLoad %31 %33
         %93 = OpLoad %31 %35
         %94 = OpFDiv %31 %92 %93
               OpStore %91 %94
               OpReturn
               OpFunctionEnd
  )";
  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;

  // Check context validity.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  // Vec Type Id |   Vector Type  |  Element Type id |   Element Type  |
  // ------------+----------------+------------------+-----------------+
  //     12      |      vec2      |         6        |      int64      |
  //     24      |      vec3      |        18        |     uint64      |
  //     37      |      vec4      |        31        |    float64      |

  // Test support for 64-bit signed int.
  {
    // Assert that the target scalar instruction result id is relevant.
    ASSERT_FALSE(transformation_context.GetFactManager()->IdIsIrrelevant(50));
    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(100, {1}), MakeDataDescriptor(48, {}));
    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(101, {1}), MakeDataDescriptor(49, {}));

    // Good: The following transformation should be applicable.
    TransformationWrapVectorSynonym wrap_add_int64(50, 100, 101, 102, 1);
    ASSERT_TRUE(
        wrap_add_int64.IsApplicable(context.get(), transformation_context));
    // Insert an arithmetic instruction of the same type to add two vectors.
    ApplyAndCheckFreshIds(wrap_add_int64, context.get(),
                          &transformation_context);

    // |instruction_id| and id at |scalar_position of the result vector should
    // be synonyms.
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(102, {1}), MakeDataDescriptor(50, {})));
  }

  // Test support for 64-bit unsigned int.
  {
    ASSERT_FALSE(transformation_context.GetFactManager()->IdIsIrrelevant(70));
    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(103, {2}), MakeDataDescriptor(68, {}));
    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(104, {2}), MakeDataDescriptor(69, {}));

    // Good: The following transformation should be applicable.
    TransformationWrapVectorSynonym wrap_sub_uint64(70, 103, 104, 105, 2);
    ASSERT_TRUE(
        wrap_sub_uint64.IsApplicable(context.get(), transformation_context));

    ApplyAndCheckFreshIds(wrap_sub_uint64, context.get(),
                          &transformation_context);

    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(105, {2}), MakeDataDescriptor(70, {})));
  }

  // Test support for 64-bit float.
  {
    ASSERT_FALSE(transformation_context.GetFactManager()->IdIsIrrelevant(90));
    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(106, {3}), MakeDataDescriptor(88, {}));
    transformation_context.GetFactManager()->AddFactDataSynonym(
        MakeDataDescriptor(107, {3}), MakeDataDescriptor(89, {}));

    // Good: The following transformation should be applicable.
    TransformationWrapVectorSynonym wrap_mul_float64(90, 106, 107, 108, 3);
    ASSERT_TRUE(
        wrap_mul_float64.IsApplicable(context.get(), transformation_context));

    ApplyAndCheckFreshIds(wrap_mul_float64, context.get(),
                          &transformation_context);

    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(108, {3}), MakeDataDescriptor(90, {})));
  }

  std::string after_transformation = R"(
               OpCapability Shader
               OpCapability Int64
               OpCapability Float64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %97
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 64 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 10
         %11 = OpConstant %6 -5
         %12 = OpTypeVector %6 2
         %13 = OpTypePointer Function %12
         %18 = OpTypeInt 64 0
         %19 = OpTypePointer Function %18
         %21 = OpConstant %18 8
         %23 = OpConstant %18 2
         %24 = OpTypeVector %18 3
         %25 = OpTypePointer Function %24
         %31 = OpTypeFloat 64
         %32 = OpTypePointer Function %31
         %34 = OpConstant %31 3.29999995
         %36 = OpConstant %31 1.10000002
         %37 = OpTypeVector %31 4
         %38 = OpTypePointer Function %37
         %96 = OpTypePointer Input %31
         %97 = OpVariable %96 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %20 = OpVariable %19 Function
         %22 = OpVariable %19 Function
         %26 = OpVariable %25 Function
         %33 = OpVariable %32 Function
         %35 = OpVariable %32 Function
         %39 = OpVariable %38 Function
         %47 = OpVariable %7 Function
         %51 = OpVariable %7 Function
         %55 = OpVariable %7 Function
         %59 = OpVariable %7 Function
         %63 = OpVariable %19 Function
         %67 = OpVariable %19 Function
         %71 = OpVariable %19 Function
         %75 = OpVariable %19 Function
         %79 = OpVariable %32 Function
         %83 = OpVariable %32 Function
         %87 = OpVariable %32 Function
         %91 = OpVariable %32 Function
               OpStore %8 %9
               OpStore %10 %11
         %15 = OpLoad %6 %8
         %16 = OpLoad %6 %10
         %17 = OpCompositeConstruct %12 %15 %16
               OpStore %14 %17
               OpStore %20 %21
               OpStore %22 %23
         %27 = OpLoad %18 %20
         %28 = OpLoad %18 %20
         %29 = OpLoad %18 %22
         %30 = OpCompositeConstruct %24 %27 %28 %29
               OpStore %26 %30
               OpStore %33 %34
               OpStore %35 %36
         %40 = OpLoad %31 %33
         %41 = OpLoad %31 %33
         %42 = OpLoad %31 %35
         %43 = OpLoad %31 %35
         %44 = OpCompositeConstruct %37 %40 %41 %42 %43
         %45 = OpLoad %37 %39
         %46 = OpVectorShuffle %37 %45 %44 5 6 7 4
               OpStore %39 %46
         %48 = OpLoad %6 %8
         %49 = OpLoad %6 %10
        %100 = OpCompositeConstruct %12 %48 %48
        %101 = OpCompositeConstruct %12 %49 %49
        %102 = OpIAdd %12 %100 %101
         %50 = OpIAdd %6 %48 %49
               OpStore %47 %50
         %52 = OpLoad %6 %8
         %53 = OpLoad %6 %10
         %54 = OpISub %6 %52 %53
               OpStore %51 %54
         %56 = OpLoad %6 %8
         %57 = OpLoad %6 %10
         %58 = OpIMul %6 %56 %57
               OpStore %55 %58
         %60 = OpLoad %6 %8
         %61 = OpLoad %6 %10
         %62 = OpSDiv %6 %60 %61
               OpStore %59 %62
         %64 = OpLoad %18 %20
         %65 = OpLoad %18 %22
         %66 = OpIAdd %18 %64 %65
               OpStore %63 %66
         %68 = OpLoad %18 %20
         %69 = OpLoad %18 %22
        %103 = OpCompositeConstruct %24 %68 %68 %68
        %104 = OpCompositeConstruct %24 %69 %69 %69
        %105 = OpISub %24 %103 %104
         %70 = OpISub %18 %68 %69
               OpStore %67 %70
         %72 = OpLoad %18 %20
         %73 = OpLoad %18 %22
         %74 = OpIMul %18 %72 %73
               OpStore %71 %74
         %76 = OpLoad %18 %20
         %77 = OpLoad %18 %22
         %78 = OpUDiv %18 %76 %77
               OpStore %75 %78
         %80 = OpLoad %31 %33
         %81 = OpLoad %31 %35
         %82 = OpFAdd %31 %80 %81
               OpStore %79 %82
         %84 = OpLoad %31 %33
         %85 = OpLoad %31 %35
         %86 = OpFSub %31 %84 %85
               OpStore %83 %86
         %88 = OpLoad %31 %33
         %89 = OpLoad %31 %35
        %106 = OpCompositeConstruct %37 %88 %88 %88 %88
        %107 = OpCompositeConstruct %37 %89 %89 %89 %89
        %108 = OpFMul %37 %106 %107
         %90 = OpFMul %31 %88 %89
               OpStore %87 %90
         %92 = OpLoad %31 %33
         %93 = OpLoad %31 %35
         %94 = OpFDiv %31 %92 %93
               OpStore %91 %94
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationWrapVectorSynonym, DifferentVectorSignedness) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeVector %6 2
          %8 = OpTypePointer Function %7
         %10 = OpConstant %6 1
         %11 = OpConstant %6 0
         %12 = OpConstantComposite %7 %10 %11
         %14 = OpTypeInt 32 0
         %15 = OpTypeVector %14 2
         %18 = OpConstant %14 3
         %19 = OpConstant %14 0
         %20 = OpConstantComposite %15 %18 %19
         %21 = OpConstantComposite %15 %19 %18
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %100 = OpIAdd %14 %10 %18
        %101 = OpIAdd %6 %10 %18
        %102 = OpIAdd %6 %18 %19
               OpReturn
               OpFunctionEnd
  )";
  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;

  // Check context validity.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(10, {}), MakeDataDescriptor(12, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(18, {}), MakeDataDescriptor(20, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(19, {}), MakeDataDescriptor(21, {0}));

  {
    TransformationWrapVectorSynonym transformation1(100, 12, 20, 200, 0);
    ASSERT_TRUE(
        transformation1.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation1, context.get(),
                          &transformation_context);
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(200, {0}), MakeDataDescriptor(100, {})));
  }

  {
    TransformationWrapVectorSynonym transformation2(101, 12, 20, 201, 0);
    ASSERT_TRUE(
        transformation2.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation2, context.get(),
                          &transformation_context);
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(201, {0}), MakeDataDescriptor(101, {})));
  }

  {
    TransformationWrapVectorSynonym transformation3(102, 20, 21, 202, 0);
    ASSERT_TRUE(
        transformation3.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation3, context.get(),
                          &transformation_context);
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(202, {0}), MakeDataDescriptor(102, {})));
  }

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeVector %6 2
          %8 = OpTypePointer Function %7
         %10 = OpConstant %6 1
         %11 = OpConstant %6 0
         %12 = OpConstantComposite %7 %10 %11
         %14 = OpTypeInt 32 0
         %15 = OpTypeVector %14 2
         %18 = OpConstant %14 3
         %19 = OpConstant %14 0
         %20 = OpConstantComposite %15 %18 %19
         %21 = OpConstantComposite %15 %19 %18
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %200 = OpIAdd %15 %12 %20
        %100 = OpIAdd %14 %10 %18
        %201 = OpIAdd %7 %12 %20
        %101 = OpIAdd %6 %10 %18
        %202 = OpIAdd %7 %20 %21
        %102 = OpIAdd %6 %18 %19
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationWrapVectorSynonym, SignednessDoesNotMatchResultType) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeVector %6 2
          %8 = OpTypePointer Function %7
         %10 = OpConstant %6 1
         %11 = OpConstant %6 0
         %12 = OpConstantComposite %7 %10 %11
         %13 = OpConstantComposite %7 %11 %10
         %14 = OpTypeInt 32 0
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %100 = OpIAdd %14 %10 %11
               OpReturn
               OpFunctionEnd
  )";
  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;

  // Check context validity.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(10, {}), MakeDataDescriptor(12, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(11, {}), MakeDataDescriptor(13, {0}));

  ASSERT_FALSE(TransformationWrapVectorSynonym(100, 12, 13, 200, 0)
                   .IsApplicable(context.get(), transformation_context));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
