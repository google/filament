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
#include "source/fuzz/data_descriptor.h"
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
void SetUpIdSynonyms(FactManager* fact_manager, opt::IRContext* context) {
  fact_manager->AddFact(MakeSynonymFact(15, 200), context);
  fact_manager->AddFact(MakeSynonymFact(15, 201), context);
  fact_manager->AddFact(MakeSynonymFact(15, 202), context);
  fact_manager->AddFact(MakeSynonymFact(55, 203), context);
  fact_manager->AddFact(MakeSynonymFact(54, 204), context);
  fact_manager->AddFact(MakeSynonymFact(74, 205), context);
  fact_manager->AddFact(MakeSynonymFact(78, 206), context);
  fact_manager->AddFact(MakeSynonymFact(84, 207), context);
  fact_manager->AddFact(MakeSynonymFact(33, 208), context);
  fact_manager->AddFact(MakeSynonymFact(12, 209), context);
  fact_manager->AddFact(MakeSynonymFact(19, 210), context);
}

TEST(TransformationReplaceIdWithSynonymTest, IllegalTransformations) {
  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, kComplexShader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;
  SetUpIdSynonyms(&fact_manager, context.get());

  // %202 cannot replace %15 as in-operand 0 of %300, since %202 does not
  // dominate %300.
  auto synonym_does_not_dominate_use = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(15, MakeInstructionDescriptor(300, SpvOpIAdd, 0), 0),
      202);
  ASSERT_FALSE(
      synonym_does_not_dominate_use.IsApplicable(context.get(), fact_manager));

  // %202 cannot replace %15 as in-operand 2 of %301, since this is the OpPhi's
  // incoming value for block %72, and %202 does not dominate %72.
  auto synonym_does_not_dominate_use_op_phi =
      TransformationReplaceIdWithSynonym(
          MakeIdUseDescriptor(15, MakeInstructionDescriptor(301, SpvOpPhi, 0),
                              2),
          202);
  ASSERT_FALSE(synonym_does_not_dominate_use_op_phi.IsApplicable(context.get(),
                                                                 fact_manager));

  // %200 is not a synonym for %84
  auto id_in_use_is_not_synonymous = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          84, MakeInstructionDescriptor(67, SpvOpSGreaterThan, 0), 0),
      200);
  ASSERT_FALSE(
      id_in_use_is_not_synonymous.IsApplicable(context.get(), fact_manager));

  // %86 is not a synonym for anything (and in particular not for %74)
  auto id_has_no_synonyms = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(86, MakeInstructionDescriptor(84, SpvOpPhi, 0), 2),
      74);
  ASSERT_FALSE(id_has_no_synonyms.IsApplicable(context.get(), fact_manager));

  // This would lead to %207 = 'OpCopyObject %type %207' if it were allowed
  auto synonym_use_is_in_synonym_definition =
      TransformationReplaceIdWithSynonym(
          MakeIdUseDescriptor(
              84, MakeInstructionDescriptor(207, SpvOpCopyObject, 0), 0),
          207);
  ASSERT_FALSE(synonym_use_is_in_synonym_definition.IsApplicable(context.get(),
                                                                 fact_manager));

  // The id use descriptor does not lead to a use (%84 is not used in the
  // definition of %207)
  auto bad_id_use_descriptor = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          84, MakeInstructionDescriptor(200, SpvOpCopyObject, 0), 0),
      207);
  ASSERT_FALSE(bad_id_use_descriptor.IsApplicable(context.get(), fact_manager));

  // This replacement would lead to an access chain into a struct using a
  // non-constant index.
  auto bad_access_chain = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          12, MakeInstructionDescriptor(14, SpvOpAccessChain, 0), 1),
      209);
  ASSERT_FALSE(bad_access_chain.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationReplaceIdWithSynonymTest, LegalTransformations) {
  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, kComplexShader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;
  SetUpIdSynonyms(&fact_manager, context.get());

  auto global_constant_synonym = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(19, MakeInstructionDescriptor(47, SpvOpStore, 0), 1),
      210);
  ASSERT_TRUE(
      global_constant_synonym.IsApplicable(context.get(), fact_manager));
  global_constant_synonym.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  auto replace_vector_access_chain_index = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          54, MakeInstructionDescriptor(55, SpvOpAccessChain, 0), 1),
      204);
  ASSERT_TRUE(replace_vector_access_chain_index.IsApplicable(context.get(),
                                                             fact_manager));
  replace_vector_access_chain_index.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // This is an interesting case because it replaces something that is being
  // copied with something that is already a synonym.
  auto regular_replacement = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          15, MakeInstructionDescriptor(202, SpvOpCopyObject, 0), 0),
      201);
  ASSERT_TRUE(regular_replacement.IsApplicable(context.get(), fact_manager));
  regular_replacement.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  auto regular_replacement2 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(55, MakeInstructionDescriptor(203, SpvOpStore, 0), 0),
      203);
  ASSERT_TRUE(regular_replacement2.IsApplicable(context.get(), fact_manager));
  regular_replacement2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  auto good_op_phi = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(74, MakeInstructionDescriptor(86, SpvOpPhi, 0), 2),
      205);
  ASSERT_TRUE(good_op_phi.IsApplicable(context.get(), fact_manager));
  good_op_phi.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

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
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  fact_manager.AddFact(MakeSynonymFact(10, 100), context.get());
  fact_manager.AddFact(MakeSynonymFact(8, 101), context.get());

  // Replace %10 with %100 in:
  // %11 = OpLoad %6 %10
  auto replacement1 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(10, MakeInstructionDescriptor(11, SpvOpLoad, 0), 0),
      100);
  ASSERT_TRUE(replacement1.IsApplicable(context.get(), fact_manager));
  replacement1.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %8 with %101 in:
  // OpStore %8 %11
  auto replacement2 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(8, MakeInstructionDescriptor(11, SpvOpStore, 0), 0),
      101);
  ASSERT_TRUE(replacement2.IsApplicable(context.get(), fact_manager));
  replacement2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %8 with %101 in:
  // %12 = OpLoad %6 %8
  auto replacement3 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(8, MakeInstructionDescriptor(12, SpvOpLoad, 0), 0),
      101);
  ASSERT_TRUE(replacement3.IsApplicable(context.get(), fact_manager));
  replacement3.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %10 with %100 in:
  // OpStore %10 %12
  auto replacement4 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(10, MakeInstructionDescriptor(12, SpvOpStore, 0), 0),
      100);
  ASSERT_TRUE(replacement4.IsApplicable(context.get(), fact_manager));
  replacement4.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

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
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  fact_manager.AddFact(MakeSynonymFact(14, 100), context.get());

  // Replace %14 with %100 in:
  // %16 = OpFunctionCall %2 %10 %14
  auto replacement = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          14, MakeInstructionDescriptor(16, SpvOpFunctionCall, 0), 1),
      100);
  ASSERT_FALSE(replacement.IsApplicable(context.get(), fact_manager));
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
         %58 = OpAccessChain %26 %50 %57 %21 %17
               OpStore %58 %45
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  // Add synonym facts corresponding to the OpCopyObject operations that have
  // been applied to all constants in the module.
  fact_manager.AddFact(MakeSynonymFact(16, 100), context.get());
  fact_manager.AddFact(MakeSynonymFact(21, 101), context.get());
  fact_manager.AddFact(MakeSynonymFact(17, 102), context.get());
  fact_manager.AddFact(MakeSynonymFact(57, 103), context.get());
  fact_manager.AddFact(MakeSynonymFact(18, 104), context.get());
  fact_manager.AddFact(MakeSynonymFact(40, 105), context.get());
  fact_manager.AddFact(MakeSynonymFact(32, 106), context.get());
  fact_manager.AddFact(MakeSynonymFact(43, 107), context.get());
  fact_manager.AddFact(MakeSynonymFact(55, 108), context.get());
  fact_manager.AddFact(MakeSynonymFact(8, 109), context.get());
  fact_manager.AddFact(MakeSynonymFact(47, 110), context.get());
  fact_manager.AddFact(MakeSynonymFact(28, 111), context.get());
  fact_manager.AddFact(MakeSynonymFact(45, 112), context.get());

  // Replacements of the form %16 -> %100

  // %20 = OpAccessChain %19 %15 *%16* %17
  // Corresponds to d.*a*[2]
  // The index %16 used for a cannot be replaced
  auto replacement1 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          16, MakeInstructionDescriptor(20, SpvOpAccessChain, 0), 1),
      100);
  ASSERT_FALSE(replacement1.IsApplicable(context.get(), fact_manager));

  // %39 = OpAccessChain %23 %37 *%16*
  // Corresponds to h.*f*
  // The index %16 used for f cannot be replaced
  auto replacement2 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          16, MakeInstructionDescriptor(39, SpvOpAccessChain, 0), 1),
      100);
  ASSERT_FALSE(replacement2.IsApplicable(context.get(), fact_manager));

  // %41 = OpAccessChain %19 %37 %21 *%16* %21
  // Corresponds to h.g.*a*[1]
  // The index %16 used for a cannot be replaced
  auto replacement3 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          16, MakeInstructionDescriptor(41, SpvOpAccessChain, 0), 2),
      100);
  ASSERT_FALSE(replacement3.IsApplicable(context.get(), fact_manager));

  // %52 = OpAccessChain %23 %50 *%16* %16
  // Corresponds to i[*0*].f
  // The index %16 used for 0 *can* be replaced
  auto replacement4 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          16, MakeInstructionDescriptor(52, SpvOpAccessChain, 0), 1),
      100);
  ASSERT_TRUE(replacement4.IsApplicable(context.get(), fact_manager));
  replacement4.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // %52 = OpAccessChain %23 %50 %16 *%16*
  // Corresponds to i[0].*f*
  // The index %16 used for f cannot be replaced
  auto replacement5 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          16, MakeInstructionDescriptor(52, SpvOpAccessChain, 0), 2),
      100);
  ASSERT_FALSE(replacement5.IsApplicable(context.get(), fact_manager));

  // %53 = OpAccessChain %19 %50 %21 %21 *%16* %16
  // Corresponds to i[1].g.*a*[0]
  // The index %16 used for a cannot be replaced
  auto replacement6 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          16, MakeInstructionDescriptor(53, SpvOpAccessChain, 0), 3),
      100);
  ASSERT_FALSE(replacement6.IsApplicable(context.get(), fact_manager));

  // %53 = OpAccessChain %19 %50 %21 %21 %16 *%16*
  // Corresponds to i[1].g.a[*0*]
  // The index %16 used for 0 *can* be replaced
  auto replacement7 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          16, MakeInstructionDescriptor(53, SpvOpAccessChain, 0), 4),
      100);
  ASSERT_TRUE(replacement7.IsApplicable(context.get(), fact_manager));
  replacement7.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replacements of the form %21 -> %101

  // %24 = OpAccessChain %23 %15 *%21* %8
  // Corresponds to d.*b*[3]
  // The index %24 used for b cannot be replaced
  auto replacement8 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(24, SpvOpAccessChain, 0), 1),
      101);
  ASSERT_FALSE(replacement8.IsApplicable(context.get(), fact_manager));

  // %41 = OpAccessChain %19 %37 *%21* %16 %21
  // Corresponds to h.*g*.a[1]
  // The index %24 used for g cannot be replaced
  auto replacement9 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(41, SpvOpAccessChain, 0), 1),
      101);
  ASSERT_FALSE(replacement9.IsApplicable(context.get(), fact_manager));

  // %41 = OpAccessChain %19 %37 %21 %16 *%21*
  // Corresponds to h.g.a[*1*]
  // The index %24 used for 1 *can* be replaced
  auto replacement10 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(41, SpvOpAccessChain, 0), 3),
      101);
  ASSERT_TRUE(replacement10.IsApplicable(context.get(), fact_manager));
  replacement10.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // %44 = OpAccessChain %23 %37 *%21* %21 %43
  // Corresponds to h.*g*.b[0]
  // The index %24 used for g cannot be replaced
  auto replacement11 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(44, SpvOpAccessChain, 0), 1),
      101);
  ASSERT_FALSE(replacement11.IsApplicable(context.get(), fact_manager));

  // %44 = OpAccessChain %23 %37 %21 *%21* %43
  // Corresponds to h.g.*b*[0]
  // The index %24 used for b cannot be replaced
  auto replacement12 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(44, SpvOpAccessChain, 0), 2),
      101);
  ASSERT_FALSE(replacement12.IsApplicable(context.get(), fact_manager));

  // %46 = OpAccessChain %26 %37 *%21* %17
  // Corresponds to h.*g*.c
  // The index %24 used for g cannot be replaced
  auto replacement13 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(46, SpvOpAccessChain, 0), 1),
      101);
  ASSERT_FALSE(replacement13.IsApplicable(context.get(), fact_manager));

  // %53 = OpAccessChain %19 %50 *%21* %21 %16 %16
  // Corresponds to i[*1*].g.a[0]
  // The index %24 used for 1 *can* be replaced
  auto replacement14 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(53, SpvOpAccessChain, 0), 1),
      101);
  ASSERT_TRUE(replacement14.IsApplicable(context.get(), fact_manager));
  replacement14.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // %53 = OpAccessChain %19 %50 %21 *%21* %16 %16
  // Corresponds to i[1].*g*.a[0]
  // The index %24 used for g cannot be replaced
  auto replacement15 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(53, SpvOpAccessChain, 0), 2),
      101);
  ASSERT_FALSE(replacement15.IsApplicable(context.get(), fact_manager));

  // %56 = OpAccessChain %23 %50 %17 *%21* %21 %55
  // Corresponds to i[2].*g*.b[1]
  // The index %24 used for g cannot be replaced
  auto replacement16 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(56, SpvOpAccessChain, 0), 2),
      101);
  ASSERT_FALSE(replacement16.IsApplicable(context.get(), fact_manager));

  // %56 = OpAccessChain %23 %50 %17 %21 *%21* %55
  // Corresponds to i[2].g.*b*[1]
  // The index %24 used for b cannot be replaced
  auto replacement17 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(56, SpvOpAccessChain, 0), 3),
      101);
  ASSERT_FALSE(replacement17.IsApplicable(context.get(), fact_manager));

  // %58 = OpAccessChain %26 %50 %57 *%21* %17
  // Corresponds to i[3].*g*.c
  // The index %24 used for g cannot be replaced
  auto replacement18 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          21, MakeInstructionDescriptor(58, SpvOpAccessChain, 0), 2),
      101);
  ASSERT_FALSE(replacement18.IsApplicable(context.get(), fact_manager));

  // Replacements of the form %17 -> %102

  // %20 = OpAccessChain %19 %15 %16 %17
  // Corresponds to d.a[*2*]
  // The index %17 used for 2 *can* be replaced
  auto replacement19 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          17, MakeInstructionDescriptor(20, SpvOpAccessChain, 0), 2),
      102);
  ASSERT_TRUE(replacement19.IsApplicable(context.get(), fact_manager));
  replacement19.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // %27 = OpAccessChain %26 %15 %17
  // Corresponds to d.c
  // The index %17 used for c cannot be replaced
  auto replacement20 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          17, MakeInstructionDescriptor(27, SpvOpAccessChain, 0), 1),
      102);
  ASSERT_FALSE(replacement20.IsApplicable(context.get(), fact_manager));

  // %46 = OpAccessChain %26 %37 %21 %17
  // Corresponds to h.g.*c*
  // The index %17 used for c cannot be replaced
  auto replacement21 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          17, MakeInstructionDescriptor(46, SpvOpAccessChain, 0), 2),
      102);
  ASSERT_FALSE(replacement21.IsApplicable(context.get(), fact_manager));

  // %56 = OpAccessChain %23 %50 %17 %21 %21 %55
  // Corresponds to i[*2*].g.b[1]
  // The index %17 used for 2 *can* be replaced
  auto replacement22 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          17, MakeInstructionDescriptor(56, SpvOpAccessChain, 0), 1),
      102);
  ASSERT_TRUE(replacement22.IsApplicable(context.get(), fact_manager));
  replacement22.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // %58 = OpAccessChain %26 %50 %57 %21 %17
  // Corresponds to i[3].g.*c*
  // The index %17 used for c cannot be replaced
  auto replacement23 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          17, MakeInstructionDescriptor(58, SpvOpAccessChain, 0), 3),
      102);
  ASSERT_FALSE(replacement23.IsApplicable(context.get(), fact_manager));

  // Replacements of the form %57 -> %103

  // %58 = OpAccessChain %26 %50 *%57* %21 %17
  // Corresponds to i[*3*].g.c
  // The index %57 used for 3 *can* be replaced
  auto replacement24 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          57, MakeInstructionDescriptor(58, SpvOpAccessChain, 0), 1),
      103);
  ASSERT_TRUE(replacement24.IsApplicable(context.get(), fact_manager));
  replacement24.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replacements of the form %32 -> %106

  // %34 = OpAccessChain %23 %31 *%32*
  // Corresponds to e[*17*]
  // The index %32 used for 17 *can* be replaced
  auto replacement25 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          32, MakeInstructionDescriptor(34, SpvOpAccessChain, 0), 1),
      106);
  ASSERT_TRUE(replacement25.IsApplicable(context.get(), fact_manager));
  replacement25.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replacements of the form %43 -> %107

  // %44 = OpAccessChain %23 %37 %21 %21 *%43*
  // Corresponds to h.g.b[*0*]
  // The index %43 used for 0 *can* be replaced
  auto replacement26 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          43, MakeInstructionDescriptor(44, SpvOpAccessChain, 0), 3),
      107);
  ASSERT_TRUE(replacement26.IsApplicable(context.get(), fact_manager));
  replacement26.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replacements of the form %55 -> %108

  // %56 = OpAccessChain %23 %50 %17 %21 %21 *%55*
  // Corresponds to i[2].g.b[*1*]
  // The index %55 used for 1 *can* be replaced
  auto replacement27 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(
          55, MakeInstructionDescriptor(56, SpvOpAccessChain, 0), 4),
      108);
  ASSERT_TRUE(replacement27.IsApplicable(context.get(), fact_manager));
  replacement27.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replacements of the form %8 -> %109

  // %24 = OpAccessChain %23 %15 %21 *%8*
  // Corresponds to d.b[*3*]
  // The index %8 used for 3 *can* be replaced
  auto replacement28 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(8, MakeInstructionDescriptor(24, SpvOpAccessChain, 0),
                          2),
      109);
  ASSERT_TRUE(replacement28.IsApplicable(context.get(), fact_manager));
  replacement28.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

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
         %58 = OpAccessChain %26 %50 %103 %21 %17
               OpStore %58 %45
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
