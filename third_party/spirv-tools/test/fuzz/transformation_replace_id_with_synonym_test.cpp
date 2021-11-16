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

#include "source/fuzz/transformation_replace_id_with_synonym.h"

#include "gtest/gtest.h"
#include "source/fuzz/data_descriptor.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/id_use_descriptor.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

// The following shader was obtained from this GLSL, which was then optimized
// with spirv-opt -O and manually edited to include some uses of OpCopyObject
// (to introduce id synonyms).
//
// #version 310 es
//
// precision highp int;
// precision highp float;
//
// layout(set = 0, binding = 0) uniform buf {
//   int a;
//   int b;
//   int c;
// };
//
// layout(location = 0) out vec4 color;
//
// void main() {
//   int x = a;
//   float f = 0.0;
//   while (x < b) {
//     switch(x % 4) {
//       case 0:
//         color[0] = f;
//         break;
//       case 1:
//         color[1] = f;
//         break;
//       case 2:
//         color[2] = f;
//         break;
//       case 3:
//         color[3] = f;
//         break;
//       default:
//         break;
//     }
//     if (x > c) {
//       x++;
//     } else {
//       x += 2;
//     }
//   }
//   color[0] += color[1] + float(x);
// }
const std::string kComplexShader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %42
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %9 "buf"
               OpMemberName %9 0 "a"
               OpMemberName %9 1 "b"
               OpMemberName %9 2 "c"
               OpName %11 ""
               OpName %42 "color"
               OpMemberDecorate %9 0 Offset 0
               OpMemberDecorate %9 1 Offset 4
               OpMemberDecorate %9 2 Offset 8
               OpDecorate %9 Block
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 0
               OpDecorate %42 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %9 = OpTypeStruct %6 %6 %6
         %10 = OpTypePointer Uniform %9
         %11 = OpVariable %10 Uniform
         %12 = OpConstant %6 0
         %13 = OpTypePointer Uniform %6
         %16 = OpTypeFloat 32
         %19 = OpConstant %16 0
         %26 = OpConstant %6 1
         %29 = OpTypeBool
         %32 = OpConstant %6 4
         %40 = OpTypeVector %16 4
         %41 = OpTypePointer Output %40
         %42 = OpVariable %41 Output
         %44 = OpTypeInt 32 0
         %45 = OpConstant %44 0
         %46 = OpTypePointer Output %16
         %50 = OpConstant %44 1
         %54 = OpConstant %44 2
         %58 = OpConstant %44 3
         %64 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %209 = OpCopyObject %6 %12
         %14 = OpAccessChain %13 %11 %12
         %15 = OpLoad %6 %14
        %200 = OpCopyObject %6 %15
               OpBranch %20
         %20 = OpLabel
         %84 = OpPhi %6 %15 %5 %86 %69
         %27 = OpAccessChain %13 %11 %26
         %28 = OpLoad %6 %27
        %207 = OpCopyObject %6 %84
        %201 = OpCopyObject %6 %15
         %30 = OpSLessThan %29 %84 %28
               OpLoopMerge %22 %69 None
               OpBranchConditional %30 %21 %22
         %21 = OpLabel
         %33 = OpSMod %6 %84 %32
        %208 = OpCopyObject %6 %33
               OpSelectionMerge %39 None
               OpSwitch %33 %38 0 %34 1 %35 2 %36 3 %37
         %38 = OpLabel
        %202 = OpCopyObject %6 %15
               OpBranch %39
         %34 = OpLabel
        %210 = OpCopyObject %16 %19
         %47 = OpAccessChain %46 %42 %45
               OpStore %47 %19
               OpBranch %39
         %35 = OpLabel
         %51 = OpAccessChain %46 %42 %50
               OpStore %51 %19
               OpBranch %39
         %36 = OpLabel
        %204 = OpCopyObject %44 %54
         %55 = OpAccessChain %46 %42 %54
        %203 = OpCopyObject %46 %55
               OpStore %55 %19
               OpBranch %39
         %37 = OpLabel
         %59 = OpAccessChain %46 %42 %58
               OpStore %59 %19
               OpBranch %39
         %39 = OpLabel
        %300 = OpIAdd %6 %15 %15
         %65 = OpAccessChain %13 %11 %64
         %66 = OpLoad %6 %65
         %67 = OpSGreaterThan %29 %84 %66
               OpSelectionMerge %1000 None
               OpBranchConditional %67 %68 %72
         %68 = OpLabel
         %71 = OpIAdd %6 %84 %26
               OpBranch %1000
         %72 = OpLabel
         %74 = OpIAdd %6 %84 %64
        %205 = OpCopyObject %6 %74
               OpBranch %1000
       %1000 = OpLabel
         %86 = OpPhi %6 %71 %68 %74 %72
        %301 = OpPhi %6 %71 %68 %15 %72
               OpBranch %69
         %69 = OpLabel
               OpBranch %20
         %22 = OpLabel
         %75 = OpAccessChain %46 %42 %50
         %76 = OpLoad %16 %75
         %78 = OpConvertSToF %16 %84
         %80 = OpAccessChain %46 %42 %45
        %206 = OpCopyObject %16 %78
         %81 = OpLoad %16 %80
         %79 = OpFAdd %16 %76 %78
         %82 = OpFAdd %16 %81 %79
               OpStore %80 %82
               OpReturn
               OpFunctionEnd
)";

protobufs::Fact MakeSynonymFact(uint32_t first, uint32_t second) {
  protobufs::FactDataSynonym data_synonym_fact;
  *data_synonym_fact.mutable_data1() = MakeDataDescriptor(first, {});
  *data_synonym_fact.mutable_data2() = MakeDataDescriptor(second, {});
  protobufs::Fact result;
  *result.mutable_data_synonym_fact() = data_synonym_fact;
  return result;
}

// Equips the fact manager with synonym facts for the above shader.
void SetUpIdSynonyms(FactManager* fact_manager) {
  fact_manager->MaybeAddFact(MakeSynonymFact(15, 200));
  fact_manager->MaybeAddFact(MakeSynonymFact(15, 201));
  fact_manager->MaybeAddFact(MakeSynonymFact(15, 202));
  fact_manager->MaybeAddFact(MakeSynonymFact(55, 203));
  fact_manager->MaybeAddFact(MakeSynonymFact(54, 204));
  fact_manager->MaybeAddFact(MakeSynonymFact(74, 205));
  fact_manager->MaybeAddFact(MakeSynonymFact(78, 206));
  fact_manager->MaybeAddFact(MakeSynonymFact(84, 207));
  fact_manager->MaybeAddFact(MakeSynonymFact(33, 208));
  fact_manager->MaybeAddFact(MakeSynonymFact(12, 209));
  fact_manager->MaybeAddFact(MakeSynonymFact(19, 210));
}

TEST(TransformationReplaceIdWithSynonymTest, IllegalTransformations) {
  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, kComplexShader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  SetUpIdSynonyms(transformation_context.GetFactManager());

  // %202 cannot replace %15 as in-operand 0 of %300, since %202 does not
  // dominate %300.
  auto synonym_does_not_dominate_use = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(15, MakeInstructionDescriptor(300, SpvOpIAdd, 0), 0),
      202);
  ASSERT_FALSE(synonym_does_not_dominate_use.IsApplicable(
      context.get(), transformation_context));

  // %202 cannot replace %15 as in-operand 2 of %301, since this is the OpPhi's
  // incoming value for block %72, and %202 does not dominate %72.
  auto synonym_does_not_dominate_use_op_phi =
      TransformationReplaceIdWithSynonym(
          MakeIdUseDescriptor(15, MakeInstructionDescriptor(301, SpvOpPhi, 0),
                              2),
          202);
  ASSERT_FALSE(synonym_does_not_dominate_use_op_phi.IsApplicable(
      context.get(), transformation_context));

  // %200 is not a synonym for %84
  auto id_in_use_is_not_synonymous = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          84, MakeInstructionDescriptor(67, SpvOpSGreaterThan, 0), 0),
      200);
  ASSERT_FALSE(id_in_use_is_not_synonymous.IsApplicable(
      context.get(), transformation_context));

  // %86 is not a synonym for anything (and in particular not for %74)
  auto id_has_no_synonyms = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(86, MakeInstructionDescriptor(84, SpvOpPhi, 0), 2),
      74);
  ASSERT_FALSE(
      id_has_no_synonyms.IsApplicable(context.get(), transformation_context));

  // This would lead to %207 = 'OpCopyObject %type %207' if it were allowed
  auto synonym_use_is_in_synonym_definition =
      TransformationReplaceIdWithSynonym(
          MakeIdUseDescriptor(
              84, MakeInstructionDescriptor(207, SpvOpCopyObject, 0), 0),
          207);
  ASSERT_FALSE(synonym_use_is_in_synonym_definition.IsApplicable(
      context.get(), transformation_context));

  // The id use descriptor does not lead to a use (%84 is not used in the
  // definition of %207)
  auto bad_id_use_descriptor = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          84, MakeInstructionDescriptor(200, SpvOpCopyObject, 0), 0),
      207);
  ASSERT_FALSE(bad_id_use_descriptor.IsApplicable(context.get(),
                                                  transformation_context));

  // This replacement would lead to an access chain into a struct using a
  // non-constant index.
  auto bad_access_chain = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          12, MakeInstructionDescriptor(14, SpvOpAccessChain, 0), 1),
      209);
  ASSERT_FALSE(
      bad_access_chain.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationReplaceIdWithSynonymTest, LegalTransformations) {
  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, kComplexShader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  SetUpIdSynonyms(transformation_context.GetFactManager());

  auto global_constant_synonym = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(19, MakeInstructionDescriptor(47, SpvOpStore, 0), 1),
      210);
  uint32_t num_uses_of_original_id_before_replacement =
      context->get_def_use_mgr()->NumUses(19);
  uint32_t num_uses_of_synonym_before_replacement =
      context->get_def_use_mgr()->NumUses(210);
  ASSERT_TRUE(global_constant_synonym.IsApplicable(context.get(),
                                                   transformation_context));
  ApplyAndCheckFreshIds(global_constant_synonym, context.get(),
                        &transformation_context);
  ASSERT_EQ(num_uses_of_original_id_before_replacement - 1,
            context->get_def_use_mgr()->NumUses(19));
  ASSERT_EQ(num_uses_of_synonym_before_replacement + 1,
            context->get_def_use_mgr()->NumUses(210));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto replace_vector_access_chain_index = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          54, MakeInstructionDescriptor(55, SpvOpAccessChain, 0), 1),
      204);
  ASSERT_TRUE(replace_vector_access_chain_index.IsApplicable(
      context.get(), transformation_context));
  ApplyAndCheckFreshIds(replace_vector_access_chain_index, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // This is an interesting case because it replaces something that is being
  // copied with something that is already a synonym.
  auto regular_replacement = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          15, MakeInstructionDescriptor(202, SpvOpCopyObject, 0), 0),
      201);
  ASSERT_TRUE(
      regular_replacement.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(regular_replacement, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto regular_replacement2 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(55, MakeInstructionDescriptor(203, SpvOpStore, 0), 0),
      203);
  ASSERT_TRUE(
      regular_replacement2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(regular_replacement2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto good_op_phi = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(74, MakeInstructionDescriptor(86, SpvOpPhi, 0), 2),
      205);
  ASSERT_TRUE(good_op_phi.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(good_op_phi, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %42
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %9 "buf"
               OpMemberName %9 0 "a"
               OpMemberName %9 1 "b"
               OpMemberName %9 2 "c"
               OpName %11 ""
               OpName %42 "color"
               OpMemberDecorate %9 0 Offset 0
               OpMemberDecorate %9 1 Offset 4
               OpMemberDecorate %9 2 Offset 8
               OpDecorate %9 Block
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 0
               OpDecorate %42 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %9 = OpTypeStruct %6 %6 %6
         %10 = OpTypePointer Uniform %9
         %11 = OpVariable %10 Uniform
         %12 = OpConstant %6 0
         %13 = OpTypePointer Uniform %6
         %16 = OpTypeFloat 32
         %19 = OpConstant %16 0
         %26 = OpConstant %6 1
         %29 = OpTypeBool
         %32 = OpConstant %6 4
         %40 = OpTypeVector %16 4
         %41 = OpTypePointer Output %40
         %42 = OpVariable %41 Output
         %44 = OpTypeInt 32 0
         %45 = OpConstant %44 0
         %46 = OpTypePointer Output %16
         %50 = OpConstant %44 1
         %54 = OpConstant %44 2
         %58 = OpConstant %44 3
         %64 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %209 = OpCopyObject %6 %12
         %14 = OpAccessChain %13 %11 %12
         %15 = OpLoad %6 %14
        %200 = OpCopyObject %6 %15
               OpBranch %20
         %20 = OpLabel
         %84 = OpPhi %6 %15 %5 %86 %69
         %27 = OpAccessChain %13 %11 %26
         %28 = OpLoad %6 %27
        %207 = OpCopyObject %6 %84
        %201 = OpCopyObject %6 %15
         %30 = OpSLessThan %29 %84 %28
               OpLoopMerge %22 %69 None
               OpBranchConditional %30 %21 %22
         %21 = OpLabel
         %33 = OpSMod %6 %84 %32
        %208 = OpCopyObject %6 %33
               OpSelectionMerge %39 None
               OpSwitch %33 %38 0 %34 1 %35 2 %36 3 %37
         %38 = OpLabel
        %202 = OpCopyObject %6 %201
               OpBranch %39
         %34 = OpLabel
        %210 = OpCopyObject %16 %19
         %47 = OpAccessChain %46 %42 %45
               OpStore %47 %210
               OpBranch %39
         %35 = OpLabel
         %51 = OpAccessChain %46 %42 %50
               OpStore %51 %19
               OpBranch %39
         %36 = OpLabel
        %204 = OpCopyObject %44 %54
         %55 = OpAccessChain %46 %42 %204
        %203 = OpCopyObject %46 %55
               OpStore %203 %19
               OpBranch %39
         %37 = OpLabel
         %59 = OpAccessChain %46 %42 %58
               OpStore %59 %19
               OpBranch %39
         %39 = OpLabel
        %300 = OpIAdd %6 %15 %15
         %65 = OpAccessChain %13 %11 %64
         %66 = OpLoad %6 %65
         %67 = OpSGreaterThan %29 %84 %66
               OpSelectionMerge %1000 None
               OpBranchConditional %67 %68 %72
         %68 = OpLabel
         %71 = OpIAdd %6 %84 %26
               OpBranch %1000
         %72 = OpLabel
         %74 = OpIAdd %6 %84 %64
        %205 = OpCopyObject %6 %74
               OpBranch %1000
       %1000 = OpLabel
         %86 = OpPhi %6 %71 %68 %205 %72
        %301 = OpPhi %6 %71 %68 %15 %72
               OpBranch %69
         %69 = OpLabel
               OpBranch %20
         %22 = OpLabel
         %75 = OpAccessChain %46 %42 %50
         %76 = OpLoad %16 %75
         %78 = OpConvertSToF %16 %84
         %80 = OpAccessChain %46 %42 %45
        %206 = OpCopyObject %16 %78
         %81 = OpLoad %16 %80
         %79 = OpFAdd %16 %76 %78
         %82 = OpFAdd %16 %81 %79
               OpStore %80 %82
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceIdWithSynonymTest, SynonymsOfVariables) {
  // The following SPIR-V comes from this GLSL, with object copies added:
  //
  // #version 310 es
  //
  // precision highp int;
  //
  // int g;
  //
  // void main() {
  //   int l;
  //   l = g;
  //   g = l;
  // }
  const std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "l"
               OpName %10 "g"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpTypePointer Private %6
         %10 = OpVariable %9 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
        %100 = OpCopyObject %9 %10
        %101 = OpCopyObject %7 %8
         %11 = OpLoad %6 %10
               OpStore %8 %11
         %12 = OpLoad %6 %8
               OpStore %10 %12
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
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(10, 100));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(8, 101));

  // Replace %10 with %100 in:
  // %11 = OpLoad %6 %10
  auto replacement1 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(10, MakeInstructionDescriptor(11, SpvOpLoad, 0), 0),
      100);
  ASSERT_TRUE(replacement1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement1, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %8 with %101 in:
  // OpStore %8 %11
  auto replacement2 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(8, MakeInstructionDescriptor(11, SpvOpStore, 0), 0),
      101);
  ASSERT_TRUE(replacement2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement2, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %8 with %101 in:
  // %12 = OpLoad %6 %8
  auto replacement3 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(8, MakeInstructionDescriptor(12, SpvOpLoad, 0), 0),
      101);
  ASSERT_TRUE(replacement3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement3, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %10 with %100 in:
  // OpStore %10 %12
  auto replacement4 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(10, MakeInstructionDescriptor(12, SpvOpStore, 0), 0),
      100);
  ASSERT_TRUE(replacement4.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement4, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "l"
               OpName %10 "g"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpTypePointer Private %6
         %10 = OpVariable %9 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
        %100 = OpCopyObject %9 %10
        %101 = OpCopyObject %7 %8
         %11 = OpLoad %6 %100
               OpStore %101 %11
         %12 = OpLoad %6 %101
               OpStore %100 %12
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceIdWithSynonymTest,
     SynonymOfVariableNoGoodInFunctionCall) {
  // The following SPIR-V comes from this GLSL, with an object copy added:
  //
  // #version 310 es
  //
  // precision highp int;
  //
  // void foo(int x) { }
  //
  // void main() {
  //   int a;
  //   a = 2;
  //   foo(a);
  // }
  const std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "foo(i1;"
               OpName %9 "x"
               OpName %12 "a"
               OpName %14 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %13 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %12 = OpVariable %7 Function
         %14 = OpVariable %7 Function
               OpStore %12 %13
         %15 = OpLoad %6 %12
               OpStore %14 %15
        %100 = OpCopyObject %7 %14
         %16 = OpFunctionCall %2 %10 %14
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
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
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(14, 100));

  // Replace %14 with %100 in:
  // %16 = OpFunctionCall %2 %10 %14
  auto replacement = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          14, MakeInstructionDescriptor(16, SpvOpFunctionCall, 0), 1),
      100);
  ASSERT_FALSE(replacement.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationReplaceIdWithSynonymTest, SynonymsOfAccessChainIndices) {
  // The following SPIR-V comes from this GLSL, with object copies added:
  //
  // #version 310 es
  //
  // precision highp float;
  // precision highp int;
  //
  // struct S {
  //   int[3] a;
  //   vec4 b;
  //   bool c;
  // } d;
  //
  // float[20] e;
  //
  // struct T {
  //   float f;
  //   S g;
  // } h;
  //
  // T[4] i;
  //
  // void main() {
  //   d.a[2] = 10;
  //   d.b[3] = 11.0;
  //   d.c = false;
  //   e[17] = 12.0;
  //   h.f = 13.0;
  //   h.g.a[1] = 14;
  //   h.g.b[0] = 15.0;
  //   h.g.c = true;
  //   i[0].f = 16.0;
  //   i[1].g.a[0] = 17;
  //   i[2].g.b[1] = 18.0;
  //   i[3].g.c = true;
  // }
  const std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %13 "S"
               OpMemberName %13 0 "a"
               OpMemberName %13 1 "b"
               OpMemberName %13 2 "c"
               OpName %15 "d"
               OpName %31 "e"
               OpName %35 "T"
               OpMemberName %35 0 "f"
               OpMemberName %35 1 "g"
               OpName %37 "h"
               OpName %50 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 3
          %9 = OpTypeArray %6 %8
         %10 = OpTypeFloat 32
         %11 = OpTypeVector %10 4
         %12 = OpTypeBool
         %13 = OpTypeStruct %9 %11 %12
         %14 = OpTypePointer Private %13
         %15 = OpVariable %14 Private
         %16 = OpConstant %6 0
         %17 = OpConstant %6 2
         %18 = OpConstant %6 10
         %19 = OpTypePointer Private %6
         %21 = OpConstant %6 1
         %22 = OpConstant %10 11
         %23 = OpTypePointer Private %10
         %25 = OpConstantFalse %12
         %26 = OpTypePointer Private %12
         %28 = OpConstant %7 20
         %29 = OpTypeArray %10 %28
         %30 = OpTypePointer Private %29
         %31 = OpVariable %30 Private
         %32 = OpConstant %6 17
         %33 = OpConstant %10 12
         %35 = OpTypeStruct %10 %13
         %36 = OpTypePointer Private %35
         %37 = OpVariable %36 Private
         %38 = OpConstant %10 13
         %40 = OpConstant %6 14
         %42 = OpConstant %10 15
         %43 = OpConstant %7 0
         %45 = OpConstantTrue %12
         %47 = OpConstant %7 4
         %48 = OpTypeArray %35 %47
         %49 = OpTypePointer Private %48
         %50 = OpVariable %49 Private
         %51 = OpConstant %10 16
         %54 = OpConstant %10 18
         %55 = OpConstant %7 1
         %57 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel

         %100 = OpCopyObject %6 %16 ; 0
         %101 = OpCopyObject %6 %21 ; 1
         %102 = OpCopyObject %6 %17 ; 2
         %103 = OpCopyObject %6 %57 ; 3
         %104 = OpCopyObject %6 %18 ; 10
         %105 = OpCopyObject %6 %40 ; 14
         %106 = OpCopyObject %6 %32 ; 17
         %107 = OpCopyObject %7 %43 ; 0
         %108 = OpCopyObject %7 %55 ; 1
         %109 = OpCopyObject %7  %8 ; 3
         %110 = OpCopyObject %7 %47 ; 4
         %111 = OpCopyObject %7 %28 ; 20
         %112 = OpCopyObject %12 %45 ; true

         %20 = OpAccessChain %19 %15 %16 %17
               OpStore %20 %18
         %24 = OpAccessChain %23 %15 %21 %8
               OpStore %24 %22
         %27 = OpAccessChain %26 %15 %17
               OpStore %27 %25
         %34 = OpAccessChain %23 %31 %32
               OpStore %34 %33
         %39 = OpAccessChain %23 %37 %16
               OpStore %39 %38
         %41 = OpAccessChain %19 %37 %21 %16 %21
               OpStore %41 %40
         %44 = OpAccessChain %23 %37 %21 %21 %43
               OpStore %44 %42
         %46 = OpAccessChain %26 %37 %21 %17
               OpStore %46 %45
         %52 = OpAccessChain %23 %50 %16 %16
               OpStore %52 %51
         %53 = OpAccessChain %19 %50 %21 %21 %16 %16
               OpStore %53 %32
         %56 = OpAccessChain %23 %50 %17 %21 %21 %55
               OpStore %56 %54
         %58 = OpInBoundsAccessChain %26 %50 %57 %21 %17
               OpStore %58 %45
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
  // Add synonym facts corresponding to the OpCopyObject operations that have
  // been applied to all constants in the module.
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(16, 100));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(21, 101));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(17, 102));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(57, 103));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(18, 104));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(40, 105));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(32, 106));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(43, 107));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(55, 108));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(8, 109));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(47, 110));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(28, 111));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(45, 112));

  // Replacements of the form %16 -> %100

  // %20 = OpAccessChain %19 %15 *%16* %17
  // Corresponds to d.*a*[2]
  // The index %16 used for a cannot be replaced
  auto replacement1 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          16, MakeInstructionDescriptor(20, SpvOpAccessChain, 0), 1),
      100);
  ASSERT_FALSE(
      replacement1.IsApplicable(context.get(), transformation_context));

  // %39 = OpAccessChain %23 %37 *%16*
  // Corresponds to h.*f*
  // The index %16 used for f cannot be replaced
  auto replacement2 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          16, MakeInstructionDescriptor(39, SpvOpAccessChain, 0), 1),
      100);
  ASSERT_FALSE(
      replacement2.IsApplicable(context.get(), transformation_context));

  // %41 = OpAccessChain %19 %37 %21 *%16* %21
  // Corresponds to h.g.*a*[1]
  // The index %16 used for a cannot be replaced
  auto replacement3 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          16, MakeInstructionDescriptor(41, SpvOpAccessChain, 0), 2),
      100);
  ASSERT_FALSE(
      replacement3.IsApplicable(context.get(), transformation_context));

  // %52 = OpAccessChain %23 %50 *%16* %16
  // Corresponds to i[*0*].f
  // The index %16 used for 0 *can* be replaced
  auto replacement4 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          16, MakeInstructionDescriptor(52, SpvOpAccessChain, 0), 1),
      100);
  ASSERT_TRUE(replacement4.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement4, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %52 = OpAccessChain %23 %50 %16 *%16*
  // Corresponds to i[0].*f*
  // The index %16 used for f cannot be replaced
  auto replacement5 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          16, MakeInstructionDescriptor(52, SpvOpAccessChain, 0), 2),
      100);
  ASSERT_FALSE(
      replacement5.IsApplicable(context.get(), transformation_context));

  // %53 = OpAccessChain %19 %50 %21 %21 *%16* %16
  // Corresponds to i[1].g.*a*[0]
  // The index %16 used for a cannot be replaced
  auto replacement6 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          16, MakeInstructionDescriptor(53, SpvOpAccessChain, 0), 3),
      100);
  ASSERT_FALSE(
      replacement6.IsApplicable(context.get(), transformation_context));

  // %53 = OpAccessChain %19 %50 %21 %21 %16 *%16*
  // Corresponds to i[1].g.a[*0*]
  // The index %16 used for 0 *can* be replaced
  auto replacement7 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          16, MakeInstructionDescriptor(53, SpvOpAccessChain, 0), 4),
      100);
  ASSERT_TRUE(replacement7.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement7, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replacements of the form %21 -> %101

  // %24 = OpAccessChain %23 %15 *%21* %8
  // Corresponds to d.*b*[3]
  // The index %24 used for b cannot be replaced
  auto replacement8 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(24, SpvOpAccessChain, 0), 1),
      101);
  ASSERT_FALSE(
      replacement8.IsApplicable(context.get(), transformation_context));

  // %41 = OpAccessChain %19 %37 *%21* %16 %21
  // Corresponds to h.*g*.a[1]
  // The index %24 used for g cannot be replaced
  auto replacement9 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(41, SpvOpAccessChain, 0), 1),
      101);
  ASSERT_FALSE(
      replacement9.IsApplicable(context.get(), transformation_context));

  // %41 = OpAccessChain %19 %37 %21 %16 *%21*
  // Corresponds to h.g.a[*1*]
  // The index %24 used for 1 *can* be replaced
  auto replacement10 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(41, SpvOpAccessChain, 0), 3),
      101);
  ASSERT_TRUE(
      replacement10.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement10, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %44 = OpAccessChain %23 %37 *%21* %21 %43
  // Corresponds to h.*g*.b[0]
  // The index %24 used for g cannot be replaced
  auto replacement11 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(44, SpvOpAccessChain, 0), 1),
      101);
  ASSERT_FALSE(
      replacement11.IsApplicable(context.get(), transformation_context));

  // %44 = OpAccessChain %23 %37 %21 *%21* %43
  // Corresponds to h.g.*b*[0]
  // The index %24 used for b cannot be replaced
  auto replacement12 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(44, SpvOpAccessChain, 0), 2),
      101);
  ASSERT_FALSE(
      replacement12.IsApplicable(context.get(), transformation_context));

  // %46 = OpAccessChain %26 %37 *%21* %17
  // Corresponds to h.*g*.c
  // The index %24 used for g cannot be replaced
  auto replacement13 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(46, SpvOpAccessChain, 0), 1),
      101);
  ASSERT_FALSE(
      replacement13.IsApplicable(context.get(), transformation_context));

  // %53 = OpAccessChain %19 %50 *%21* %21 %16 %16
  // Corresponds to i[*1*].g.a[0]
  // The index %24 used for 1 *can* be replaced
  auto replacement14 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(53, SpvOpAccessChain, 0), 1),
      101);
  ASSERT_TRUE(
      replacement14.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement14, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %53 = OpAccessChain %19 %50 %21 *%21* %16 %16
  // Corresponds to i[1].*g*.a[0]
  // The index %24 used for g cannot be replaced
  auto replacement15 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(53, SpvOpAccessChain, 0), 2),
      101);
  ASSERT_FALSE(
      replacement15.IsApplicable(context.get(), transformation_context));

  // %56 = OpAccessChain %23 %50 %17 *%21* %21 %55
  // Corresponds to i[2].*g*.b[1]
  // The index %24 used for g cannot be replaced
  auto replacement16 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(56, SpvOpAccessChain, 0), 2),
      101);
  ASSERT_FALSE(
      replacement16.IsApplicable(context.get(), transformation_context));

  // %56 = OpAccessChain %23 %50 %17 %21 *%21* %55
  // Corresponds to i[2].g.*b*[1]
  // The index %24 used for b cannot be replaced
  auto replacement17 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(56, SpvOpAccessChain, 0), 3),
      101);
  ASSERT_FALSE(
      replacement17.IsApplicable(context.get(), transformation_context));

  // %58 = OpInBoundsAccessChain %26 %50 %57 *%21* %17
  // Corresponds to i[3].*g*.c
  // The index %24 used for g cannot be replaced
  auto replacement18 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(58, SpvOpInBoundsAccessChain, 0), 2),
      101);
  ASSERT_FALSE(
      replacement18.IsApplicable(context.get(), transformation_context));

  // Replacements of the form %17 -> %102

  // %20 = OpAccessChain %19 %15 %16 %17
  // Corresponds to d.a[*2*]
  // The index %17 used for 2 *can* be replaced
  auto replacement19 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          17, MakeInstructionDescriptor(20, SpvOpAccessChain, 0), 2),
      102);
  ASSERT_TRUE(
      replacement19.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement19, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %27 = OpAccessChain %26 %15 %17
  // Corresponds to d.c
  // The index %17 used for c cannot be replaced
  auto replacement20 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          17, MakeInstructionDescriptor(27, SpvOpAccessChain, 0), 1),
      102);
  ASSERT_FALSE(
      replacement20.IsApplicable(context.get(), transformation_context));

  // %46 = OpAccessChain %26 %37 %21 %17
  // Corresponds to h.g.*c*
  // The index %17 used for c cannot be replaced
  auto replacement21 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          17, MakeInstructionDescriptor(46, SpvOpAccessChain, 0), 2),
      102);
  ASSERT_FALSE(
      replacement21.IsApplicable(context.get(), transformation_context));

  // %56 = OpAccessChain %23 %50 %17 %21 %21 %55
  // Corresponds to i[*2*].g.b[1]
  // The index %17 used for 2 *can* be replaced
  auto replacement22 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          17, MakeInstructionDescriptor(56, SpvOpAccessChain, 0), 1),
      102);
  ASSERT_TRUE(
      replacement22.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement22, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %58 = OpInBoundsAccessChain %26 %50 %57 %21 %17
  // Corresponds to i[3].g.*c*
  // The index %17 used for c cannot be replaced
  auto replacement23 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          17, MakeInstructionDescriptor(58, SpvOpInBoundsAccessChain, 0), 3),
      102);
  ASSERT_FALSE(
      replacement23.IsApplicable(context.get(), transformation_context));

  // Replacements of the form %57 -> %103

  // %58 = OpInBoundsAccessChain %26 %50 *%57* %21 %17
  // Corresponds to i[*3*].g.c
  // The index %57 used for 3 *can* be replaced
  auto replacement24 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          57, MakeInstructionDescriptor(58, SpvOpInBoundsAccessChain, 0), 1),
      103);
  ASSERT_TRUE(
      replacement24.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement24, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replacements of the form %32 -> %106

  // %34 = OpAccessChain %23 %31 *%32*
  // Corresponds to e[*17*]
  // The index %32 used for 17 *can* be replaced
  auto replacement25 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          32, MakeInstructionDescriptor(34, SpvOpAccessChain, 0), 1),
      106);
  ASSERT_TRUE(
      replacement25.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement25, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replacements of the form %43 -> %107

  // %44 = OpAccessChain %23 %37 %21 %21 *%43*
  // Corresponds to h.g.b[*0*]
  // The index %43 used for 0 *can* be replaced
  auto replacement26 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          43, MakeInstructionDescriptor(44, SpvOpAccessChain, 0), 3),
      107);
  ASSERT_TRUE(
      replacement26.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement26, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replacements of the form %55 -> %108

  // %56 = OpAccessChain %23 %50 %17 %21 %21 *%55*
  // Corresponds to i[2].g.b[*1*]
  // The index %55 used for 1 *can* be replaced
  auto replacement27 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          55, MakeInstructionDescriptor(56, SpvOpAccessChain, 0), 4),
      108);
  ASSERT_TRUE(
      replacement27.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement27, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replacements of the form %8 -> %109

  // %24 = OpAccessChain %23 %15 %21 *%8*
  // Corresponds to d.b[*3*]
  // The index %8 used for 3 *can* be replaced
  auto replacement28 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(8, MakeInstructionDescriptor(24, SpvOpAccessChain, 0),
                          2),
      109);
  ASSERT_TRUE(
      replacement28.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement28, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %13 "S"
               OpMemberName %13 0 "a"
               OpMemberName %13 1 "b"
               OpMemberName %13 2 "c"
               OpName %15 "d"
               OpName %31 "e"
               OpName %35 "T"
               OpMemberName %35 0 "f"
               OpMemberName %35 1 "g"
               OpName %37 "h"
               OpName %50 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 3
          %9 = OpTypeArray %6 %8
         %10 = OpTypeFloat 32
         %11 = OpTypeVector %10 4
         %12 = OpTypeBool
         %13 = OpTypeStruct %9 %11 %12
         %14 = OpTypePointer Private %13
         %15 = OpVariable %14 Private
         %16 = OpConstant %6 0
         %17 = OpConstant %6 2
         %18 = OpConstant %6 10
         %19 = OpTypePointer Private %6
         %21 = OpConstant %6 1
         %22 = OpConstant %10 11
         %23 = OpTypePointer Private %10
         %25 = OpConstantFalse %12
         %26 = OpTypePointer Private %12
         %28 = OpConstant %7 20
         %29 = OpTypeArray %10 %28
         %30 = OpTypePointer Private %29
         %31 = OpVariable %30 Private
         %32 = OpConstant %6 17
         %33 = OpConstant %10 12
         %35 = OpTypeStruct %10 %13
         %36 = OpTypePointer Private %35
         %37 = OpVariable %36 Private
         %38 = OpConstant %10 13
         %40 = OpConstant %6 14
         %42 = OpConstant %10 15
         %43 = OpConstant %7 0
         %45 = OpConstantTrue %12
         %47 = OpConstant %7 4
         %48 = OpTypeArray %35 %47
         %49 = OpTypePointer Private %48
         %50 = OpVariable %49 Private
         %51 = OpConstant %10 16
         %54 = OpConstant %10 18
         %55 = OpConstant %7 1
         %57 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel

         %100 = OpCopyObject %6 %16 ; 0
         %101 = OpCopyObject %6 %21 ; 1
         %102 = OpCopyObject %6 %17 ; 2
         %103 = OpCopyObject %6 %57 ; 3
         %104 = OpCopyObject %6 %18 ; 10
         %105 = OpCopyObject %6 %40 ; 14
         %106 = OpCopyObject %6 %32 ; 17
         %107 = OpCopyObject %7 %43 ; 0
         %108 = OpCopyObject %7 %55 ; 1
         %109 = OpCopyObject %7  %8 ; 3
         %110 = OpCopyObject %7 %47 ; 4
         %111 = OpCopyObject %7 %28 ; 20
         %112 = OpCopyObject %12 %45 ; true

         %20 = OpAccessChain %19 %15 %16 %102
               OpStore %20 %18
         %24 = OpAccessChain %23 %15 %21 %109
               OpStore %24 %22
         %27 = OpAccessChain %26 %15 %17
               OpStore %27 %25
         %34 = OpAccessChain %23 %31 %106
               OpStore %34 %33
         %39 = OpAccessChain %23 %37 %16
               OpStore %39 %38
         %41 = OpAccessChain %19 %37 %21 %16 %101
               OpStore %41 %40
         %44 = OpAccessChain %23 %37 %21 %21 %107
               OpStore %44 %42
         %46 = OpAccessChain %26 %37 %21 %17
               OpStore %46 %45
         %52 = OpAccessChain %23 %50 %100 %16
               OpStore %52 %51
         %53 = OpAccessChain %19 %50 %101 %21 %16 %100
               OpStore %53 %32
         %56 = OpAccessChain %23 %50 %102 %21 %21 %108
               OpStore %56 %54
         %58 = OpInBoundsAccessChain %26 %50 %103 %21 %17
               OpStore %58 %45
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceIdWithSynonymTest, RuntimeArrayTest) {
  // This checks that OpRuntimeArray is correctly handled.
  const std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %8 ArrayStride 8
               OpMemberDecorate %9 0 Offset 0
               OpDecorate %9 BufferBlock
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeVector %6 2
          %8 = OpTypeRuntimeArray %7
          %9 = OpTypeStruct %8
         %10 = OpTypePointer Uniform %9
         %11 = OpVariable %10 Uniform
         %12 = OpConstant %6 0
         %13 = OpTypeInt 32 0
         %14 = OpConstant %13 0
         %15 = OpTypePointer Uniform %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %50 = OpCopyObject %6 %12
         %51 = OpCopyObject %13 %14
         %16 = OpAccessChain %15 %11 %12 %12 %14
               OpStore %16 %12
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
  // Add synonym fact relating %50 and %12.
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(50, 12));
  // Add synonym fact relating %51 and %14.
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(51, 14));

  // Not legal because the index being replaced is a struct index.
  ASSERT_FALSE(
      TransformationReplaceIdWithSynonym(
          MakeIdUseDescriptor(
              12, MakeInstructionDescriptor(16, SpvOpAccessChain, 0), 1),
          50)
          .IsApplicable(context.get(), transformation_context));

  // Fine to replace an index into a runtime array.
  auto replacement1 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          12, MakeInstructionDescriptor(16, SpvOpAccessChain, 0), 2),
      50);
  ASSERT_TRUE(replacement1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement1, context.get(), &transformation_context);

  // Fine to replace an index into a vector inside the runtime array.
  auto replacement2 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          14, MakeInstructionDescriptor(16, SpvOpAccessChain, 0), 3),
      51);
  ASSERT_TRUE(replacement2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement2, context.get(), &transformation_context);

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %8 ArrayStride 8
               OpMemberDecorate %9 0 Offset 0
               OpDecorate %9 BufferBlock
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeVector %6 2
          %8 = OpTypeRuntimeArray %7
          %9 = OpTypeStruct %8
         %10 = OpTypePointer Uniform %9
         %11 = OpVariable %10 Uniform
         %12 = OpConstant %6 0
         %13 = OpTypeInt 32 0
         %14 = OpConstant %13 0
         %15 = OpTypePointer Uniform %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %50 = OpCopyObject %6 %12
         %51 = OpCopyObject %13 %14
         %16 = OpAccessChain %15 %11 %12 %50 %51
               OpStore %16 %12
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceIdWithSynonymTest,
     DoNotReplaceSampleParameterOfOpImageTexelPointer) {
  const std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main" %3
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %4 = OpTypeVoid
          %5 = OpTypeFunction %4
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpConstant %6 2
          %9 = OpConstant %6 0
         %10 = OpConstant %6 10
         %11 = OpTypeBool
         %12 = OpConstant %6 1
         %13 = OpTypeFloat 32
         %14 = OpTypePointer Image %13
         %15 = OpTypeImage %13 2D 0 0 0 0 Rgba8
         %16 = OpTypePointer Private %15
          %3 = OpVariable %16 Private
         %17 = OpTypeVector %6 2
         %18 = OpConstantComposite %17 %9 %9
          %2 = OpFunction %4 None %5
         %19 = OpLabel
        %100 = OpCopyObject %6 %9
         %20 = OpImageTexelPointer %14 %3 %18 %9
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Add synonym fact relating %100 and %9.
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(100, 9));

  // Not legal the Sample argument of OpImageTexelPointer needs to be a zero
  // constant.
  ASSERT_FALSE(
      TransformationReplaceIdWithSynonym(
          MakeIdUseDescriptor(
              9, MakeInstructionDescriptor(20, SpvOpImageTexelPointer, 0), 2),
          100)
          .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationReplaceIdWithSynonymTest, EquivalentIntegerConstants) {
  // This checks that replacing an integer constant with an equivalent one with
  // different signedness is allowed only when valid.
  const std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
               OpName %3 "a"
               OpDecorate %3 RelaxedPrecision
          %4 = OpTypeVoid
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpTypeFunction %4
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %10 = OpConstant %8 1
         %11 = OpTypeInt 32 0
         %12 = OpTypePointer Function %11
         %13 = OpConstant %11 1
          %2 = OpFunction %4 None %7
         %14 = OpLabel
          %3 = OpVariable %9 Function
         %15 = OpSNegate %8 %10
         %16 = OpIAdd %8 %10 %10
         %17 = OpSDiv %8 %10 %10
         %18 = OpUDiv %11 %13 %13
         %19 = OpBitwiseAnd %8 %10 %10
         %20 = OpSelect %8 %6 %10 %17
         %21 = OpIEqual %5 %10 %10
               OpStore %3 %10
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
  // Add synonym fact relating %10 and %13 (equivalent integer constant with
  // different signedness).
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(10, 13));

  // Legal because OpSNegate always considers the integer as signed
  auto replacement1 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(10, MakeInstructionDescriptor(15, SpvOpSNegate, 0),
                          0),
      13);
  ASSERT_TRUE(replacement1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement1, context.get(), &transformation_context);

  // Legal because OpIAdd does not care about the signedness of the operands
  auto replacement2 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(10, MakeInstructionDescriptor(16, SpvOpIAdd, 0), 0),
      13);
  ASSERT_TRUE(replacement2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement2, context.get(), &transformation_context);

  // Legal because OpSDiv does not care about the signedness of the operands
  auto replacement3 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(10, MakeInstructionDescriptor(17, SpvOpSDiv, 0), 0),
      13);
  ASSERT_TRUE(replacement3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement3, context.get(), &transformation_context);

  // Not legal because OpUDiv requires unsigned integers
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(
                   MakeIdUseDescriptor(
                       13, MakeInstructionDescriptor(18, SpvOpUDiv, 0), 0),
                   10)
                   .IsApplicable(context.get(), transformation_context));

  // Legal because OpSDiv does not care about the signedness of the operands
  auto replacement4 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(10, MakeInstructionDescriptor(19, SpvOpBitwiseAnd, 0),
                          0),
      13);
  ASSERT_TRUE(replacement4.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement4, context.get(), &transformation_context);

  // Not legal because OpSelect requires both operands to have the same type as
  // the result type
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(
                   MakeIdUseDescriptor(
                       10, MakeInstructionDescriptor(20, SpvOpUDiv, 0), 1),
                   13)
                   .IsApplicable(context.get(), transformation_context));

  // Not legal because OpStore requires the object to match the type pointed
  // to by the pointer.
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(
                   MakeIdUseDescriptor(
                       10, MakeInstructionDescriptor(21, SpvOpStore, 0), 1),
                   13)
                   .IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
               OpName %3 "a"
               OpDecorate %3 RelaxedPrecision
          %4 = OpTypeVoid
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpTypeFunction %4
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %10 = OpConstant %8 1
         %11 = OpTypeInt 32 0
         %12 = OpTypePointer Function %11
         %13 = OpConstant %11 1
          %2 = OpFunction %4 None %7
         %14 = OpLabel
          %3 = OpVariable %9 Function
         %15 = OpSNegate %8 %13
         %16 = OpIAdd %8 %13 %10
         %17 = OpSDiv %8 %13 %10
         %18 = OpUDiv %11 %13 %13
         %19 = OpBitwiseAnd %8 %13 %10
         %20 = OpSelect %8 %6 %10 %17
         %21 = OpIEqual %5 %10 %10
               OpStore %3 %10
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceIdWithSynonymTest, EquivalentIntegerVectorConstants) {
  // This checks that replacing an integer constant with an equivalent one with
  // different signedness is allowed only when valid.
  const std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
               OpName %3 "a"
               OpDecorate %3 RelaxedPrecision
               OpDecorate %4 RelaxedPrecision
          %5 = OpTypeVoid
          %6 = OpTypeFunction %5
          %7 = OpTypeInt 32 1
          %8 = OpTypeInt 32 0
          %9 = OpTypeVector %7 4
         %10 = OpTypeVector %8 4
         %11 = OpTypePointer Function %9
         %12 = OpConstant %7 1
         %13 = OpConstant %8 1
         %14 = OpConstantComposite %9 %12 %12 %12 %12
         %15 = OpConstantComposite %10 %13 %13 %13 %13
         %16 = OpTypePointer Function %7
          %2 = OpFunction %5 None %6
         %17 = OpLabel
          %3 = OpVariable %11 Function
         %18 = OpIAdd %9 %14 %14
               OpStore %3 %14
         %19 = OpAccessChain %16 %3 %13
          %4 = OpLoad %7 %19
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
  // Add synonym fact relating %10 and %13 (equivalent integer vectors with
  // different signedness).
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(14, 15));

  // Legal because OpIAdd does not consider the signedness of the operands
  auto replacement1 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(14, MakeInstructionDescriptor(18, SpvOpIAdd, 0), 0),
      15);
  ASSERT_TRUE(replacement1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement1, context.get(), &transformation_context);

  // Not legal because OpStore requires the object to match the type pointed
  // to by the pointer.
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(
                   MakeIdUseDescriptor(
                       14, MakeInstructionDescriptor(18, SpvOpStore, 0), 1),
                   15)
                   .IsApplicable(context.get(), transformation_context));

  // Add synonym fact relating %12 and %13 (equivalent integer constants with
  // different signedness).
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(12, 13));

  // Legal because the indices of OpAccessChain are always treated as signed
  auto replacement2 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          13, MakeInstructionDescriptor(19, SpvOpAccessChain, 0), 1),
      12);
  ASSERT_TRUE(replacement2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement2, context.get(), &transformation_context);

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
               OpName %3 "a"
               OpDecorate %3 RelaxedPrecision
               OpDecorate %4 RelaxedPrecision
          %5 = OpTypeVoid
          %6 = OpTypeFunction %5
          %7 = OpTypeInt 32 1
          %8 = OpTypeInt 32 0
          %9 = OpTypeVector %7 4
         %10 = OpTypeVector %8 4
         %11 = OpTypePointer Function %9
         %12 = OpConstant %7 1
         %13 = OpConstant %8 1
         %14 = OpConstantComposite %9 %12 %12 %12 %12
         %15 = OpConstantComposite %10 %13 %13 %13 %13
         %16 = OpTypePointer Function %7
          %2 = OpFunction %5 None %6
         %17 = OpLabel
          %3 = OpVariable %11 Function
         %18 = OpIAdd %9 %15 %14
               OpStore %3 %14
         %19 = OpAccessChain %16 %3 %12
          %4 = OpLoad %7 %19
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceIdWithSynonymTest, IncompatibleTypes) {
  const std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %5 = OpTypeVoid
          %6 = OpTypeFunction %5
          %7 = OpTypeInt 32 1
          %8 = OpTypeInt 32 0
          %9 = OpTypeFloat 32
         %12 = OpConstant %7 1
         %13 = OpConstant %8 1
         %10 = OpConstant %9 1
          %2 = OpFunction %5 None %6
         %17 = OpLabel
         %18 = OpIAdd %7 %12 %13
         %19 = OpFAdd %9 %10 %10
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
  auto* op_i_add = context->get_def_use_mgr()->GetDef(18);
  ASSERT_TRUE(op_i_add);

  auto* op_f_add = context->get_def_use_mgr()->GetDef(19);
  ASSERT_TRUE(op_f_add);

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(12, {}), MakeDataDescriptor(13, {}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(12, {}), MakeDataDescriptor(10, {}));

  // Synonym differs only in signedness for OpIAdd.
  ASSERT_TRUE(TransformationReplaceIdWithSynonym(
                  MakeIdUseDescriptorFromUse(context.get(), op_i_add, 0), 13)
                  .IsApplicable(context.get(), transformation_context));

  // Synonym has wrong type for OpIAdd.
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(
                   MakeIdUseDescriptorFromUse(context.get(), op_i_add, 0), 10)
                   .IsApplicable(context.get(), transformation_context));

  // Synonym has wrong type for OpFAdd.
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(
                   MakeIdUseDescriptorFromUse(context.get(), op_f_add, 0), 12)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(
                   MakeIdUseDescriptorFromUse(context.get(), op_f_add, 0), 13)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationReplaceIdWithSynonymTest,
     AtomicScopeAndMemorySemanticsMustBeConstant) {
  const std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource ESSL 320
               OpSourceExtension "GL_KHR_memory_scope_semantics"
               OpMemberDecorate %9 0 Offset 0
               OpDecorate %9 Block
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %17 = OpTypeInt 32 0
          %7 = OpTypePointer Function %6
          %9 = OpTypeStruct %6
         %10 = OpTypePointer StorageBuffer %9
         %11 = OpVariable %10 StorageBuffer
         %86 = OpTypeStruct %17
         %87 = OpTypePointer Workgroup %86         
         %88 = OpVariable %87 Workgroup
         %12 = OpConstant %6 0
         %13 = OpTypePointer StorageBuffer %6
         %15 = OpConstant %6 2
         %16 = OpConstant %6 64
         %89 = OpTypePointer Workgroup %17
         %18 = OpConstant %17 1
         %19 = OpConstant %17 0
         %20 = OpConstant %17 64
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
        %100 = OpCopyObject %6 %15 ; A non-constant version of %15
        %101 = OpCopyObject %17 %20 ; A non-constant version of %20
         %14 = OpAccessChain %13 %11 %12
         %90 = OpAccessChain %89 %88 %19
         %21 = OpAtomicLoad %6 %14 %15 %20
         %22 = OpAtomicExchange %6 %14 %15 %20 %16
         %23 = OpAtomicCompareExchange %6 %14 %15 %20 %12 %16 %15
         %24 = OpAtomicIIncrement %6 %14 %15 %20
         %25 = OpAtomicIDecrement %6 %14 %15 %20
         %26 = OpAtomicIAdd %6  %14 %15 %20 %16
         %27 = OpAtomicISub %6  %14 %15 %20 %16
         %28 = OpAtomicSMin %6  %14 %15 %20 %16
         %29 = OpAtomicUMin %17 %90 %15 %20 %18
         %30 = OpAtomicSMax %6  %14 %15 %20 %15
         %31 = OpAtomicUMax %17 %90 %15 %20 %18
         %32 = OpAtomicAnd  %6  %14 %15 %20 %16
         %33 = OpAtomicOr   %6  %14 %15 %20 %16
         %34 = OpAtomicXor  %6  %14 %15 %20 %16
               OpStore %8 %21
               OpAtomicStore %14 %15 %20 %12
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

  // Tell the fact manager that %100 and %15 are synonymous
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(100, {}), MakeDataDescriptor(15, {}));

  // Tell the fact manager that %101 and %20 are synonymous
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(101, {}), MakeDataDescriptor(20, {}));
  // OpAtomicLoad
  const auto& scope_operand = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(21), 1);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(scope_operand, 100)
                   .IsApplicable(context.get(), transformation_context));

  const auto& semantics_operand = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(21), 2);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(semantics_operand, 101)
                   .IsApplicable(context.get(), transformation_context));
  // OpAtomicExchange.
  const auto& scope_operand2 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(22), 1);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(scope_operand2, 100)
                   .IsApplicable(context.get(), transformation_context));

  const auto& semantics_operand2 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(22), 2);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(semantics_operand2, 101)
                   .IsApplicable(context.get(), transformation_context));
  // OpAtomicCompareExchange.
  const auto& scope_operand3 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(23), 1);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(scope_operand3, 100)
                   .IsApplicable(context.get(), transformation_context));

  const auto& semantics_equal_operand3 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(23), 2);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(semantics_equal_operand3, 101)
                   .IsApplicable(context.get(), transformation_context));
  const auto& semantics_unequal_operand3 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(23), 3);
  ASSERT_FALSE(
      TransformationReplaceIdWithSynonym(semantics_unequal_operand3, 101)
          .IsApplicable(context.get(), transformation_context));
  // OpAtomicIIncrement.
  const auto& scope_operand4 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(24), 1);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(scope_operand4, 100)
                   .IsApplicable(context.get(), transformation_context));

  const auto& semantics_operand4 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(24), 2);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(semantics_operand4, 101)
                   .IsApplicable(context.get(), transformation_context));

  // OpAtomicIDecrement.
  const auto& scope_operand5 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(25), 1);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(scope_operand5, 100)
                   .IsApplicable(context.get(), transformation_context));

  const auto& semantics_operand5 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(25), 2);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(semantics_operand5, 101)
                   .IsApplicable(context.get(), transformation_context));

  // OpAtomicIAdd.
  const auto& scope_operand6 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(26), 1);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(scope_operand6, 100)
                   .IsApplicable(context.get(), transformation_context));

  const auto& semantics_operand6 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(26), 2);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(semantics_operand6, 101)
                   .IsApplicable(context.get(), transformation_context));
  // OpAtomicISub
  const auto& scope_operand8 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(27), 1);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(scope_operand8, 100)
                   .IsApplicable(context.get(), transformation_context));

  const auto& semantics_operand8 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(27), 2);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(semantics_operand8, 101)
                   .IsApplicable(context.get(), transformation_context));

  // OpAtomicSMin
  const auto& scope_operand9 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(28), 1);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(scope_operand9, 100)
                   .IsApplicable(context.get(), transformation_context));

  const auto& semantics_operand9 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(28), 2);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(semantics_operand9, 101)
                   .IsApplicable(context.get(), transformation_context));
  // OpAtomicUMin
  const auto& scope_operand10 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(29), 1);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(scope_operand10, 100)
                   .IsApplicable(context.get(), transformation_context));

  const auto& semantics_operand10 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(29), 2);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(semantics_operand10, 101)
                   .IsApplicable(context.get(), transformation_context));

  // OpAtomicSMax
  const auto& scope_operand11 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(30), 1);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(scope_operand11, 100)
                   .IsApplicable(context.get(), transformation_context));

  const auto& semantics_operand11 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(30), 2);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(semantics_operand11, 101)
                   .IsApplicable(context.get(), transformation_context));
  // OpAtomicUMax
  const auto& scope_operand12 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(31), 1);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(scope_operand12, 100)
                   .IsApplicable(context.get(), transformation_context));

  const auto& semantics_operand12 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(31), 2);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(semantics_operand12, 101)
                   .IsApplicable(context.get(), transformation_context));

  // OpAtomicAnd
  const auto& scope_operand13 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(32), 1);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(scope_operand13, 100)
                   .IsApplicable(context.get(), transformation_context));

  const auto& semantics_operand13 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(32), 2);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(semantics_operand13, 101)
                   .IsApplicable(context.get(), transformation_context));

  // OpAtomicOr
  const auto& scope_operand14 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(33), 1);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(scope_operand14, 100)
                   .IsApplicable(context.get(), transformation_context));

  const auto& semantics_operand14 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(33), 2);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(semantics_operand14, 101)
                   .IsApplicable(context.get(), transformation_context));

  // OpAtomicXor
  const auto& scope_operand15 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(34), 1);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(scope_operand15, 100)
                   .IsApplicable(context.get(), transformation_context));

  const auto& semantics_operand15 = MakeIdUseDescriptorFromUse(
      context.get(), context->get_def_use_mgr()->GetDef(34), 2);
  ASSERT_FALSE(TransformationReplaceIdWithSynonym(semantics_operand15, 101)
                   .IsApplicable(context.get(), transformation_context));
}

// TODO(https://github.com/KhronosGroup/SPIRV-Tools/issues/4345): Improve this
//  test so that it covers more atomic operations, and enable the test once the
//  issue is fixed.
TEST(TransformationReplaceIdWithSynonymTest,
     DISABLED_SignOfAtomicScopeAndMemorySemanticsDoesNotMatter) {
  // TODO(https://github.com/KhronosGroup/SPIRV-Tools/issues/4345): both the
  //  GLSL comment and the corresponding SPIR-V should be updated to cover a
  //  larger number of atomic operations.
  // The following SPIR-V came from this GLSL, edited to add some synonyms:
  //
  // #version 320 es
  //
  // #extension GL_KHR_memory_scope_semantics : enable
  //
  // layout(set = 0, binding = 0) buffer Buf {
  //   int x;
  // };
  //
  // void main() {
  //   int tmp = atomicLoad(x,
  //                        gl_ScopeWorkgroup,
  //                        gl_StorageSemanticsBuffer,
  //                        gl_SemanticsRelaxed);
  // }
  const std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource ESSL 320
               OpSourceExtension "GL_KHR_memory_scope_semantics"
               OpMemberDecorate %9 0 Offset 0
               OpDecorate %9 Block
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpTypeStruct %6
         %10 = OpTypePointer StorageBuffer %9
         %11 = OpVariable %10 StorageBuffer
         %12 = OpConstant %6 0
         %13 = OpTypePointer StorageBuffer %6
         %15 = OpConstant %6 2
         %16 = OpConstant %6 64
         %17 = OpTypeInt 32 0
        %100 = OpConstant %17 2 ; The same as %15, but with unsigned int type
         %18 = OpConstant %17 1
         %19 = OpConstant %17 0
         %20 = OpConstant %17 64
        %101 = OpConstant %6 64 ; The same as %20, but with signed int type
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %14 = OpAccessChain %13 %11 %12
         %21 = OpAtomicLoad %6 %14 %15 %20
               OpStore %8 %21
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

  // Tell the fact manager that %100 and %15 are synonymous
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(100, {}), MakeDataDescriptor(15, {}));

  // Tell the fact manager that %101 and %20 are synonymous
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(101, {}), MakeDataDescriptor(20, {}));

  {
    const auto& scope_operand = MakeIdUseDescriptorFromUse(
        context.get(), context->get_def_use_mgr()->GetDef(21), 1);
    TransformationReplaceIdWithSynonym replace_scope(scope_operand, 100);
    ASSERT_TRUE(
        replace_scope.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(replace_scope, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  {
    const auto& semantics_operand = MakeIdUseDescriptorFromUse(
        context.get(), context->get_def_use_mgr()->GetDef(21), 2);
    TransformationReplaceIdWithSynonym replace_semantics(semantics_operand,
                                                         101);
    ASSERT_TRUE(
        replace_semantics.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(replace_semantics, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource ESSL 320
               OpSourceExtension "GL_KHR_memory_scope_semantics"
               OpMemberDecorate %9 0 Offset 0
               OpDecorate %9 Block
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpTypeStruct %6
         %10 = OpTypePointer StorageBuffer %9
         %11 = OpVariable %10 StorageBuffer
         %12 = OpConstant %6 0
         %13 = OpTypePointer StorageBuffer %6
         %15 = OpConstant %6 2
         %16 = OpConstant %6 64
         %17 = OpTypeInt 32 0
        %100 = OpConstant %17 2
         %18 = OpConstant %17 1
         %19 = OpConstant %17 0
         %20 = OpConstant %17 64
        %101 = OpConstant %6 64
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %14 = OpAccessChain %13 %11 %12
         %21 = OpAtomicLoad %6 %14 %100 %101
               OpStore %8 %21
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
