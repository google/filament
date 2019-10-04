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

#include "source/fuzz/transformation_copy_object.h"
#include "source/fuzz/data_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationCopyObjectTest, CopyBooleanConstants) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
          %8 = OpConstantFalse %6
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  ASSERT_EQ(0, fact_manager.GetIdsForWhichSynonymsAreKnown().size());

  TransformationCopyObject copy_true(7, 5, 1, 100);
  ASSERT_TRUE(copy_true.IsApplicable(context.get(), fact_manager));
  copy_true.Apply(context.get(), &fact_manager);

  const std::set<uint32_t>& ids_for_which_synonyms_are_known =
      fact_manager.GetIdsForWhichSynonymsAreKnown();
  ASSERT_EQ(1, ids_for_which_synonyms_are_known.size());
  ASSERT_TRUE(ids_for_which_synonyms_are_known.find(7) !=
              ids_for_which_synonyms_are_known.end());
  ASSERT_EQ(1, fact_manager.GetSynonymsForId(7).size());
  protobufs::DataDescriptor descriptor_100 = MakeDataDescriptor(100, {});
  ASSERT_TRUE(DataDescriptorEquals()(&descriptor_100,
                                     &fact_manager.GetSynonymsForId(7)[0]));

  TransformationCopyObject copy_false(8, 100, 1, 101);
  ASSERT_TRUE(copy_false.IsApplicable(context.get(), fact_manager));
  copy_false.Apply(context.get(), &fact_manager);
  ASSERT_EQ(2, ids_for_which_synonyms_are_known.size());
  ASSERT_TRUE(ids_for_which_synonyms_are_known.find(8) !=
              ids_for_which_synonyms_are_known.end());
  ASSERT_EQ(1, fact_manager.GetSynonymsForId(8).size());
  protobufs::DataDescriptor descriptor_101 = MakeDataDescriptor(101, {});
  ASSERT_TRUE(DataDescriptorEquals()(&descriptor_101,
                                     &fact_manager.GetSynonymsForId(8)[0]));

  TransformationCopyObject copy_false_again(101, 5, 3, 102);
  ASSERT_TRUE(copy_false_again.IsApplicable(context.get(), fact_manager));
  copy_false_again.Apply(context.get(), &fact_manager);
  ASSERT_EQ(3, ids_for_which_synonyms_are_known.size());
  ASSERT_TRUE(ids_for_which_synonyms_are_known.find(101) !=
              ids_for_which_synonyms_are_known.end());
  ASSERT_EQ(1, fact_manager.GetSynonymsForId(101).size());
  protobufs::DataDescriptor descriptor_102 = MakeDataDescriptor(102, {});
  ASSERT_TRUE(DataDescriptorEquals()(&descriptor_102,
                                     &fact_manager.GetSynonymsForId(101)[0]));

  TransformationCopyObject copy_true_again(7, 102, 1, 103);
  ASSERT_TRUE(copy_true_again.IsApplicable(context.get(), fact_manager));
  copy_true_again.Apply(context.get(), &fact_manager);
  // This does re-uses an id for which synonyms are already known, so the count
  // of such ids does not change.
  ASSERT_EQ(3, ids_for_which_synonyms_are_known.size());
  ASSERT_TRUE(ids_for_which_synonyms_are_known.find(7) !=
              ids_for_which_synonyms_are_known.end());
  ASSERT_EQ(2, fact_manager.GetSynonymsForId(7).size());
  protobufs::DataDescriptor descriptor_103 = MakeDataDescriptor(103, {});
  ASSERT_TRUE(DataDescriptorEquals()(&descriptor_103,
                                     &fact_manager.GetSynonymsForId(7)[0]) ||
              DataDescriptorEquals()(&descriptor_103,
                                     &fact_manager.GetSynonymsForId(7)[1]));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
          %8 = OpConstantFalse %6
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %100 = OpCopyObject %6 %7
        %101 = OpCopyObject %6 %8
        %102 = OpCopyObject %6 %101
        %103 = OpCopyObject %6 %7
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationCopyObjectTest, CheckIllegalCases) {
  // The following SPIR-V comes from this GLSL, pushed through spirv-opt
  // and then doctored a bit.
  //
  // #version 310 es
  //
  // precision highp float;
  //
  // struct S {
  //   int a;
  //   float b;
  // };
  //
  // layout(set = 0, binding = 2) uniform block {
  //   S s;
  //   lowp float f;
  //   int ii;
  // } ubuf;
  //
  // layout(location = 0) out vec4 color;
  //
  // void main() {
  //   float c = 0.0;
  //   lowp float d = 0.0;
  //   S localS = ubuf.s;
  //   for (int i = 0; i < ubuf.s.a; i++) {
  //     switch (ubuf.ii) {
  //       case 0:
  //         c += 0.1;
  //         d += 0.2;
  //       case 1:
  //         c += 0.1;
  //         if (c > d) {
  //           d += 0.2;
  //         } else {
  //           d += c;
  //         }
  //         break;
  //       default:
  //         i += 1;
  //         localS.b += d;
  //     }
  //   }
  //   color = vec4(c, d, localS.b, 1.0);
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %80
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %12 "S"
               OpMemberName %12 0 "a"
               OpMemberName %12 1 "b"
               OpName %15 "S"
               OpMemberName %15 0 "a"
               OpMemberName %15 1 "b"
               OpName %16 "block"
               OpMemberName %16 0 "s"
               OpMemberName %16 1 "f"
               OpMemberName %16 2 "ii"
               OpName %18 "ubuf"
               OpName %80 "color"
               OpMemberDecorate %12 0 RelaxedPrecision
               OpMemberDecorate %15 0 RelaxedPrecision
               OpMemberDecorate %15 0 Offset 0
               OpMemberDecorate %15 1 Offset 4
               OpMemberDecorate %16 0 Offset 0
               OpMemberDecorate %16 1 RelaxedPrecision
               OpMemberDecorate %16 1 Offset 16
               OpMemberDecorate %16 2 RelaxedPrecision
               OpMemberDecorate %16 2 Offset 20
               OpDecorate %16 Block
               OpDecorate %18 DescriptorSet 0
               OpDecorate %18 Binding 2
               OpDecorate %38 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %80 Location 0
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %9 = OpConstant %6 0
         %11 = OpTypeInt 32 1
         %12 = OpTypeStruct %11 %6
         %15 = OpTypeStruct %11 %6
         %16 = OpTypeStruct %15 %6 %11
         %17 = OpTypePointer Uniform %16
         %18 = OpVariable %17 Uniform
         %19 = OpConstant %11 0
         %20 = OpTypePointer Uniform %15
         %27 = OpConstant %11 1
         %36 = OpTypePointer Uniform %11
         %39 = OpTypeBool
         %41 = OpConstant %11 2
         %48 = OpConstant %6 0.100000001
         %51 = OpConstant %6 0.200000003
         %78 = OpTypeVector %6 4
         %79 = OpTypePointer Output %78
         %80 = OpVariable %79 Output
         %85 = OpConstant %6 1
         %95 = OpUndef %12
        %112 = OpTypePointer Uniform %6
        %113 = OpTypeInt 32 0
        %114 = OpConstant %113 1
        %179 = OpTypePointer Function %39
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %180 = OpVariable %179 Function
        %181 = OpVariable %179 Function
        %182 = OpVariable %179 Function
         %21 = OpAccessChain %20 %18 %19
        %115 = OpAccessChain %112 %21 %114
        %116 = OpLoad %6 %115
         %90 = OpCompositeInsert %12 %116 %95 1
               OpBranch %30
         %30 = OpLabel
         %99 = OpPhi %12 %90 %5 %109 %47
         %98 = OpPhi %6 %9 %5 %107 %47
         %97 = OpPhi %6 %9 %5 %105 %47
         %96 = OpPhi %11 %19 %5 %77 %47
         %37 = OpAccessChain %36 %18 %19 %19
         %38 = OpLoad %11 %37
         %40 = OpSLessThan %39 %96 %38
               OpLoopMerge %32 %47 None
               OpBranchConditional %40 %31 %32
         %31 = OpLabel
         %42 = OpAccessChain %36 %18 %41
         %43 = OpLoad %11 %42
               OpSelectionMerge %47 None
               OpSwitch %43 %46 0 %44 1 %45
         %46 = OpLabel
         %69 = OpIAdd %11 %96 %27
         %72 = OpCompositeExtract %6 %99 1
         %73 = OpFAdd %6 %72 %98
         %93 = OpCompositeInsert %12 %73 %99 1
               OpBranch %47
         %44 = OpLabel
         %50 = OpFAdd %6 %97 %48
         %53 = OpFAdd %6 %98 %51
               OpBranch %45
         %45 = OpLabel
        %101 = OpPhi %6 %98 %31 %53 %44
        %100 = OpPhi %6 %97 %31 %50 %44
         %55 = OpFAdd %6 %100 %48
         %58 = OpFOrdGreaterThan %39 %55 %101
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %63
         %59 = OpLabel
         %62 = OpFAdd %6 %101 %51
               OpBranch %60
         %63 = OpLabel
         %66 = OpFAdd %6 %101 %55
               OpBranch %60
         %60 = OpLabel
        %108 = OpPhi %6 %62 %59 %66 %63
               OpBranch %47
         %47 = OpLabel
        %109 = OpPhi %12 %93 %46 %99 %60
        %107 = OpPhi %6 %98 %46 %108 %60
        %105 = OpPhi %6 %97 %46 %55 %60
        %102 = OpPhi %11 %69 %46 %96 %60
         %77 = OpIAdd %11 %102 %27
               OpBranch %30
         %32 = OpLabel
         %84 = OpCompositeExtract %6 %99 1
         %86 = OpCompositeConstruct %78 %97 %98 %84 %85
               OpStore %80 %86
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  // Inapplicable because %18 is decorated.
  ASSERT_FALSE(TransformationCopyObject(18, 21, 0, 200)
                   .IsApplicable(context.get(), fact_manager));

  // Inapplicable because %77 is decorated.
  ASSERT_FALSE(TransformationCopyObject(17, 17, 1, 200)
                   .IsApplicable(context.get(), fact_manager));

  // Inapplicable because %80 is decorated.
  ASSERT_FALSE(TransformationCopyObject(80, 77, 0, 200)
                   .IsApplicable(context.get(), fact_manager));

  // Inapplicable because %84 is not available at the requested point
  ASSERT_FALSE(TransformationCopyObject(84, 32, 1, 200)
                   .IsApplicable(context.get(), fact_manager));

  // Fine because %84 is available at the requested point
  ASSERT_TRUE(TransformationCopyObject(84, 32, 2, 200)
                  .IsApplicable(context.get(), fact_manager));

  // Inapplicable because id %9 is already in use
  ASSERT_FALSE(TransformationCopyObject(84, 32, 2, 9)
                   .IsApplicable(context.get(), fact_manager));

  // Inapplicable because the requested point is not in a block
  ASSERT_FALSE(TransformationCopyObject(84, 86, 3, 200)
                   .IsApplicable(context.get(), fact_manager));

  // Inapplicable because %9 is not in a function
  ASSERT_FALSE(TransformationCopyObject(9, 9, 1, 200)
                   .IsApplicable(context.get(), fact_manager));

  // Inapplicable because %9 is not in a function
  ASSERT_FALSE(TransformationCopyObject(9, 9, 1, 200)
                   .IsApplicable(context.get(), fact_manager));

  // Inapplicable because the insert point is right before, or inside, a chunk
  // of OpPhis
  ASSERT_FALSE(TransformationCopyObject(9, 30, 1, 200)
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationCopyObject(9, 99, 1, 200)
                   .IsApplicable(context.get(), fact_manager));

  // OK, because the insert point is just after a chunk of OpPhis.
  ASSERT_TRUE(TransformationCopyObject(9, 96, 1, 200)
                  .IsApplicable(context.get(), fact_manager));

  // Inapplicable because the insert point is right after an OpSelectionMerge
  ASSERT_FALSE(TransformationCopyObject(9, 58, 2, 200)
                   .IsApplicable(context.get(), fact_manager));

  // OK, because the insert point is right before the OpSelectionMerge
  ASSERT_TRUE(TransformationCopyObject(9, 58, 1, 200)
                  .IsApplicable(context.get(), fact_manager));

  // Inapplicable because the insert point is right after an OpSelectionMerge
  ASSERT_FALSE(TransformationCopyObject(9, 43, 2, 200)
                   .IsApplicable(context.get(), fact_manager));

  // OK, because the insert point is right before the OpSelectionMerge
  ASSERT_TRUE(TransformationCopyObject(9, 43, 1, 200)
                  .IsApplicable(context.get(), fact_manager));

  // Inapplicable because the insert point is right after an OpLoopMerge
  ASSERT_FALSE(TransformationCopyObject(9, 40, 2, 200)
                   .IsApplicable(context.get(), fact_manager));

  // OK, because the insert point is right before the OpLoopMerge
  ASSERT_TRUE(TransformationCopyObject(9, 40, 1, 200)
                  .IsApplicable(context.get(), fact_manager));

  // Inapplicable because id %300 does not exist
  ASSERT_FALSE(TransformationCopyObject(300, 40, 1, 200)
                   .IsApplicable(context.get(), fact_manager));

  // Inapplicable because the following instruction is OpVariable
  ASSERT_FALSE(TransformationCopyObject(9, 180, 0, 200)
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationCopyObject(9, 181, 0, 200)
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationCopyObject(9, 182, 0, 200)
                   .IsApplicable(context.get(), fact_manager));

  // OK, because this is just past the group of OpVariable instructions.
  ASSERT_TRUE(TransformationCopyObject(9, 182, 1, 200)
                  .IsApplicable(context.get(), fact_manager));
}

TEST(TransformationCopyObjectTest, MiscellaneousCopies) {
  // The following SPIR-V comes from this GLSL:
  //
  // #version 310 es
  //
  // precision highp float;
  //
  // float g;
  //
  // vec4 h;
  //
  // void main() {
  //   int a;
  //   int b;
  //   b = int(g);
  //   h.x = float(a);
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "b"
               OpName %11 "g"
               OpName %16 "h"
               OpName %17 "a"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpTypeFloat 32
         %10 = OpTypePointer Private %9
         %11 = OpVariable %10 Private
         %14 = OpTypeVector %9 4
         %15 = OpTypePointer Private %14
         %16 = OpVariable %15 Private
         %20 = OpTypeInt 32 0
         %21 = OpConstant %20 0
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %17 = OpVariable %7 Function
         %12 = OpLoad %9 %11
         %13 = OpConvertFToS %6 %12
               OpStore %8 %13
         %18 = OpLoad %6 %17
         %19 = OpConvertSToF %9 %18
         %22 = OpAccessChain %10 %16 %21
               OpStore %22 %19
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  std::vector<TransformationCopyObject> transformations = {
      TransformationCopyObject(19, 22, 1, 100),
      TransformationCopyObject(22, 22, 1, 101),
      TransformationCopyObject(12, 22, 1, 102),
      TransformationCopyObject(11, 22, 1, 103),
      TransformationCopyObject(16, 22, 1, 104),
      TransformationCopyObject(8, 22, 1, 105),
      TransformationCopyObject(17, 22, 1, 106)};

  for (auto& transformation : transformations) {
    ASSERT_TRUE(transformation.IsApplicable(context.get(), fact_manager));
    transformation.Apply(context.get(), &fact_manager);
  }

  ASSERT_TRUE(IsValid(env, context.get()));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "b"
               OpName %11 "g"
               OpName %16 "h"
               OpName %17 "a"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpTypeFloat 32
         %10 = OpTypePointer Private %9
         %11 = OpVariable %10 Private
         %14 = OpTypeVector %9 4
         %15 = OpTypePointer Private %14
         %16 = OpVariable %15 Private
         %20 = OpTypeInt 32 0
         %21 = OpConstant %20 0
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %17 = OpVariable %7 Function
         %12 = OpLoad %9 %11
         %13 = OpConvertFToS %6 %12
               OpStore %8 %13
         %18 = OpLoad %6 %17
         %19 = OpConvertSToF %9 %18
         %22 = OpAccessChain %10 %16 %21
        %106 = OpCopyObject %7 %17
        %105 = OpCopyObject %7 %8
        %104 = OpCopyObject %15 %16
        %103 = OpCopyObject %10 %11
        %102 = OpCopyObject %9 %12
        %101 = OpCopyObject %10 %22
        %100 = OpCopyObject %9 %19
               OpStore %22 %19
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
