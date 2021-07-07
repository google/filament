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

#include "source/fuzz/transformation_set_memory_operands_mask.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationSetMemoryOperandsMaskTest, PreSpirv14) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %7 "Point3D"
               OpMemberName %7 0 "x"
               OpMemberName %7 1 "y"
               OpMemberName %7 2 "z"
               OpName %12 "global_points"
               OpName %15 "block"
               OpMemberName %15 0 "in_points"
               OpMemberName %15 1 "in_point"
               OpName %17 ""
               OpName %133 "local_points"
               OpMemberDecorate %7 0 Offset 0
               OpMemberDecorate %7 1 Offset 4
               OpMemberDecorate %7 2 Offset 8
               OpDecorate %10 ArrayStride 16
               OpMemberDecorate %15 0 Offset 0
               OpMemberDecorate %15 1 Offset 192
               OpDecorate %15 Block
               OpDecorate %17 DescriptorSet 0
               OpDecorate %17 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeStruct %6 %6 %6
          %8 = OpTypeInt 32 0
          %9 = OpConstant %8 12
         %10 = OpTypeArray %7 %9
         %11 = OpTypePointer Private %10
         %12 = OpVariable %11 Private
         %15 = OpTypeStruct %10 %7
         %16 = OpTypePointer Uniform %15
         %17 = OpVariable %16 Uniform
         %18 = OpTypeInt 32 1
         %19 = OpConstant %18 0
         %20 = OpTypePointer Uniform %10
         %24 = OpTypePointer Private %7
         %27 = OpTypePointer Private %6
         %30 = OpConstant %18 1
        %132 = OpTypePointer Function %10
        %135 = OpTypePointer Uniform %7
        %145 = OpTypePointer Function %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %133 = OpVariable %132 Function
         %21 = OpAccessChain %20 %17 %19
               OpCopyMemory %12 %21 Aligned 16
               OpCopyMemory %133 %12 Volatile
               OpCopyMemory %133 %12
        %136 = OpAccessChain %135 %17 %30
        %138 = OpAccessChain %24 %12 %19
               OpCopyMemory %138 %136 None
        %146 = OpAccessChain %145 %133 %30
        %147 = OpLoad %7 %146 Volatile|Nontemporal|Aligned 16
        %148 = OpAccessChain %24 %12 %19
               OpStore %148 %147 Nontemporal
               OpReturn
               OpFunctionEnd
  )";

  for (auto env :
       {SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1, SPV_ENV_UNIVERSAL_1_2,
        SPV_ENV_UNIVERSAL_1_3, SPV_ENV_VULKAN_1_0, SPV_ENV_VULKAN_1_1}) {
    const auto consumer = nullptr;
    const auto context =
        BuildModule(env, consumer, shader, kFuzzAssembleOption);
    spvtools::ValidatorOptions validator_options;
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    TransformationContext transformation_context(
        MakeUnique<FactManager>(context.get()), validator_options);

#ifndef NDEBUG
    {
      // Not OK: multiple operands are not supported pre SPIR-V 1.4.
      TransformationSetMemoryOperandsMask transformation(
          MakeInstructionDescriptor(21, SpvOpCopyMemory, 3),
          SpvMemoryAccessNontemporalMask | SpvMemoryAccessVolatileMask, 1);
      ASSERT_DEATH(
          transformation.IsApplicable(context.get(), transformation_context),
          "Multiple memory operand masks are not supported");
    }
#endif

    // Not OK: the instruction is not a memory access.
    ASSERT_FALSE(TransformationSetMemoryOperandsMask(
                     MakeInstructionDescriptor(21, SpvOpAccessChain, 0),
                     SpvMemoryAccessMaskNone, 0)
                     .IsApplicable(context.get(), transformation_context));

    // Not OK to remove Aligned
    ASSERT_FALSE(
        TransformationSetMemoryOperandsMask(
            MakeInstructionDescriptor(147, SpvOpLoad, 0),
            SpvMemoryAccessVolatileMask | SpvMemoryAccessNontemporalMask, 0)
            .IsApplicable(context.get(), transformation_context));

    {
      TransformationSetMemoryOperandsMask transformation(
          MakeInstructionDescriptor(147, SpvOpLoad, 0),
          SpvMemoryAccessAlignedMask | SpvMemoryAccessVolatileMask, 0);
      ASSERT_TRUE(
          transformation.IsApplicable(context.get(), transformation_context));
      ApplyAndCheckFreshIds(transformation, context.get(),
                            &transformation_context);
    }

    // Not OK to remove Aligned
    ASSERT_FALSE(TransformationSetMemoryOperandsMask(
                     MakeInstructionDescriptor(21, SpvOpCopyMemory, 0),
                     SpvMemoryAccessMaskNone, 0)
                     .IsApplicable(context.get(), transformation_context));

    // OK: leaves the mask as is
    ASSERT_TRUE(TransformationSetMemoryOperandsMask(
                    MakeInstructionDescriptor(21, SpvOpCopyMemory, 0),
                    SpvMemoryAccessAlignedMask, 0)
                    .IsApplicable(context.get(), transformation_context));

    {
      // OK: adds Nontemporal and Volatile
      TransformationSetMemoryOperandsMask transformation(
          MakeInstructionDescriptor(21, SpvOpCopyMemory, 0),
          SpvMemoryAccessAlignedMask | SpvMemoryAccessNontemporalMask |
              SpvMemoryAccessVolatileMask,
          0);
      ASSERT_TRUE(
          transformation.IsApplicable(context.get(), transformation_context));
      ApplyAndCheckFreshIds(transformation, context.get(),
                            &transformation_context);
    }

    // Not OK to remove Volatile
    ASSERT_FALSE(TransformationSetMemoryOperandsMask(
                     MakeInstructionDescriptor(21, SpvOpCopyMemory, 1),
                     SpvMemoryAccessNontemporalMask, 0)
                     .IsApplicable(context.get(), transformation_context));

    // Not OK to add Aligned
    ASSERT_FALSE(TransformationSetMemoryOperandsMask(
                     MakeInstructionDescriptor(21, SpvOpCopyMemory, 1),
                     SpvMemoryAccessAlignedMask | SpvMemoryAccessVolatileMask,
                     0)
                     .IsApplicable(context.get(), transformation_context));

    {
      // OK: adds Nontemporal
      TransformationSetMemoryOperandsMask transformation(
          MakeInstructionDescriptor(21, SpvOpCopyMemory, 1),
          SpvMemoryAccessNontemporalMask | SpvMemoryAccessVolatileMask, 0);
      ASSERT_TRUE(
          transformation.IsApplicable(context.get(), transformation_context));
      ApplyAndCheckFreshIds(transformation, context.get(),
                            &transformation_context);
    }

    {
      // OK: adds Nontemporal (creates new operand)
      TransformationSetMemoryOperandsMask transformation(
          MakeInstructionDescriptor(21, SpvOpCopyMemory, 2),
          SpvMemoryAccessNontemporalMask | SpvMemoryAccessVolatileMask, 0);
      ASSERT_TRUE(
          transformation.IsApplicable(context.get(), transformation_context));
      ApplyAndCheckFreshIds(transformation, context.get(),
                            &transformation_context);
    }

    {
      // OK: adds Nontemporal and Volatile
      TransformationSetMemoryOperandsMask transformation(
          MakeInstructionDescriptor(138, SpvOpCopyMemory, 0),
          SpvMemoryAccessNontemporalMask | SpvMemoryAccessVolatileMask, 0);
      ASSERT_TRUE(
          transformation.IsApplicable(context.get(), transformation_context));
      ApplyAndCheckFreshIds(transformation, context.get(),
                            &transformation_context);
    }

    {
      // OK: removes Nontemporal, adds Volatile
      TransformationSetMemoryOperandsMask transformation(
          MakeInstructionDescriptor(148, SpvOpStore, 0),
          SpvMemoryAccessVolatileMask, 0);
      ASSERT_TRUE(
          transformation.IsApplicable(context.get(), transformation_context));
      ApplyAndCheckFreshIds(transformation, context.get(),
                            &transformation_context);
    }

    std::string after_transformation = R"(
                 OpCapability Shader
            %1 = OpExtInstImport "GLSL.std.450"
                 OpMemoryModel Logical GLSL450
                 OpEntryPoint Fragment %4 "main"
                 OpExecutionMode %4 OriginUpperLeft
                 OpSource ESSL 310
                 OpName %4 "main"
                 OpName %7 "Point3D"
                 OpMemberName %7 0 "x"
                 OpMemberName %7 1 "y"
                 OpMemberName %7 2 "z"
                 OpName %12 "global_points"
                 OpName %15 "block"
                 OpMemberName %15 0 "in_points"
                 OpMemberName %15 1 "in_point"
                 OpName %17 ""
                 OpName %133 "local_points"
                 OpMemberDecorate %7 0 Offset 0
                 OpMemberDecorate %7 1 Offset 4
                 OpMemberDecorate %7 2 Offset 8
                 OpDecorate %10 ArrayStride 16
                 OpMemberDecorate %15 0 Offset 0
                 OpMemberDecorate %15 1 Offset 192
                 OpDecorate %15 Block
                 OpDecorate %17 DescriptorSet 0
                 OpDecorate %17 Binding 0
            %2 = OpTypeVoid
            %3 = OpTypeFunction %2
            %6 = OpTypeFloat 32
            %7 = OpTypeStruct %6 %6 %6
            %8 = OpTypeInt 32 0
            %9 = OpConstant %8 12
           %10 = OpTypeArray %7 %9
           %11 = OpTypePointer Private %10
           %12 = OpVariable %11 Private
           %15 = OpTypeStruct %10 %7
           %16 = OpTypePointer Uniform %15
           %17 = OpVariable %16 Uniform
           %18 = OpTypeInt 32 1
           %19 = OpConstant %18 0
           %20 = OpTypePointer Uniform %10
           %24 = OpTypePointer Private %7
           %27 = OpTypePointer Private %6
           %30 = OpConstant %18 1
          %132 = OpTypePointer Function %10
          %135 = OpTypePointer Uniform %7
          %145 = OpTypePointer Function %7
            %4 = OpFunction %2 None %3
            %5 = OpLabel
          %133 = OpVariable %132 Function
           %21 = OpAccessChain %20 %17 %19
                 OpCopyMemory %12 %21 Aligned|Nontemporal|Volatile 16
                 OpCopyMemory %133 %12 Nontemporal|Volatile
                 OpCopyMemory %133 %12 Nontemporal|Volatile
          %136 = OpAccessChain %135 %17 %30
          %138 = OpAccessChain %24 %12 %19
                 OpCopyMemory %138 %136 Nontemporal|Volatile
          %146 = OpAccessChain %145 %133 %30
          %147 = OpLoad %7 %146 Aligned|Volatile 16
          %148 = OpAccessChain %24 %12 %19
                 OpStore %148 %147 Volatile
                 OpReturn
                 OpFunctionEnd
    )";
    ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
  }
}

TEST(TransformationSetMemoryOperandsMaskTest, Spirv14OrHigher) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %12 %17
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %7 "Point3D"
               OpMemberName %7 0 "x"
               OpMemberName %7 1 "y"
               OpMemberName %7 2 "z"
               OpName %12 "global_points"
               OpName %15 "block"
               OpMemberName %15 0 "in_points"
               OpMemberName %15 1 "in_point"
               OpName %17 ""
               OpName %133 "local_points"
               OpMemberDecorate %7 0 Offset 0
               OpMemberDecorate %7 1 Offset 4
               OpMemberDecorate %7 2 Offset 8
               OpDecorate %10 ArrayStride 16
               OpMemberDecorate %15 0 Offset 0
               OpMemberDecorate %15 1 Offset 192
               OpDecorate %15 Block
               OpDecorate %17 DescriptorSet 0
               OpDecorate %17 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeStruct %6 %6 %6
          %8 = OpTypeInt 32 0
          %9 = OpConstant %8 12
         %10 = OpTypeArray %7 %9
         %11 = OpTypePointer Private %10
         %12 = OpVariable %11 Private
         %15 = OpTypeStruct %10 %7
         %16 = OpTypePointer Uniform %15
         %17 = OpVariable %16 Uniform
         %18 = OpTypeInt 32 1
         %19 = OpConstant %18 0
         %20 = OpTypePointer Uniform %10
         %24 = OpTypePointer Private %7
         %27 = OpTypePointer Private %6
         %30 = OpConstant %18 1
        %132 = OpTypePointer Function %10
        %135 = OpTypePointer Uniform %7
        %145 = OpTypePointer Function %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %133 = OpVariable %132 Function
         %21 = OpAccessChain %20 %17 %19
               OpCopyMemory %12 %21 Aligned 16 Nontemporal|Aligned 16
               OpCopyMemory %133 %12 Volatile
               OpCopyMemory %133 %12
               OpCopyMemory %133 %12
        %136 = OpAccessChain %135 %17 %30
        %138 = OpAccessChain %24 %12 %19
               OpCopyMemory %138 %136 None Aligned 16
               OpCopyMemory %138 %136 Aligned 16
        %146 = OpAccessChain %145 %133 %30
        %147 = OpLoad %7 %146 Volatile|Nontemporal|Aligned 16
        %148 = OpAccessChain %24 %12 %19
               OpStore %148 %147 Nontemporal
               OpReturn
               OpFunctionEnd
  )";

  for (auto env : {SPV_ENV_UNIVERSAL_1_4, SPV_ENV_UNIVERSAL_1_5,
                   SPV_ENV_VULKAN_1_1_SPIRV_1_4, SPV_ENV_VULKAN_1_2}) {
    const auto consumer = nullptr;
    const auto context =
        BuildModule(env, consumer, shader, kFuzzAssembleOption);
    spvtools::ValidatorOptions validator_options;
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    TransformationContext transformation_context(
        MakeUnique<FactManager>(context.get()), validator_options);
    {
      TransformationSetMemoryOperandsMask transformation(
          MakeInstructionDescriptor(21, SpvOpCopyMemory, 0),
          SpvMemoryAccessAlignedMask | SpvMemoryAccessVolatileMask, 1);
      // Bad: cannot remove aligned
      ASSERT_FALSE(TransformationSetMemoryOperandsMask(
                       MakeInstructionDescriptor(21, SpvOpCopyMemory, 0),
                       SpvMemoryAccessVolatileMask, 1)
                       .IsApplicable(context.get(), transformation_context));
      ASSERT_TRUE(
          transformation.IsApplicable(context.get(), transformation_context));
      ApplyAndCheckFreshIds(transformation, context.get(),
                            &transformation_context);
    }

    {
      TransformationSetMemoryOperandsMask transformation(
          MakeInstructionDescriptor(21, SpvOpCopyMemory, 1),
          SpvMemoryAccessNontemporalMask | SpvMemoryAccessVolatileMask, 1);
      // Bad: cannot remove volatile
      ASSERT_FALSE(TransformationSetMemoryOperandsMask(
                       MakeInstructionDescriptor(21, SpvOpCopyMemory, 1),
                       SpvMemoryAccessNontemporalMask, 0)
                       .IsApplicable(context.get(), transformation_context));
      ASSERT_TRUE(
          transformation.IsApplicable(context.get(), transformation_context));
      ApplyAndCheckFreshIds(transformation, context.get(),
                            &transformation_context);
    }

    {
      // Creates the first operand.
      TransformationSetMemoryOperandsMask transformation(
          MakeInstructionDescriptor(21, SpvOpCopyMemory, 2),
          SpvMemoryAccessNontemporalMask | SpvMemoryAccessVolatileMask, 0);
      ASSERT_TRUE(
          transformation.IsApplicable(context.get(), transformation_context));
      ApplyAndCheckFreshIds(transformation, context.get(),
                            &transformation_context);
    }

    {
      // Creates both operands.
      TransformationSetMemoryOperandsMask transformation(
          MakeInstructionDescriptor(21, SpvOpCopyMemory, 3),
          SpvMemoryAccessNontemporalMask | SpvMemoryAccessVolatileMask, 1);
      ASSERT_TRUE(
          transformation.IsApplicable(context.get(), transformation_context));
      ApplyAndCheckFreshIds(transformation, context.get(),
                            &transformation_context);
    }

    {
      TransformationSetMemoryOperandsMask transformation(
          MakeInstructionDescriptor(138, SpvOpCopyMemory, 0),
          SpvMemoryAccessAlignedMask | SpvMemoryAccessNontemporalMask, 1);
      // Bad: the first mask is None, so Aligned cannot be added to it.
      ASSERT_FALSE(
          TransformationSetMemoryOperandsMask(
              MakeInstructionDescriptor(138, SpvOpCopyMemory, 0),
              SpvMemoryAccessAlignedMask | SpvMemoryAccessNontemporalMask, 0)
              .IsApplicable(context.get(), transformation_context));
      ASSERT_TRUE(
          transformation.IsApplicable(context.get(), transformation_context));
      ApplyAndCheckFreshIds(transformation, context.get(),
                            &transformation_context);
    }

    {
      TransformationSetMemoryOperandsMask transformation(
          MakeInstructionDescriptor(138, SpvOpCopyMemory, 1),
          SpvMemoryAccessVolatileMask, 1);
      ASSERT_TRUE(
          transformation.IsApplicable(context.get(), transformation_context));
      ApplyAndCheckFreshIds(transformation, context.get(),
                            &transformation_context);
    }

    {
      TransformationSetMemoryOperandsMask transformation(
          MakeInstructionDescriptor(147, SpvOpLoad, 0),
          SpvMemoryAccessVolatileMask | SpvMemoryAccessAlignedMask, 0);
      ASSERT_TRUE(
          transformation.IsApplicable(context.get(), transformation_context));
      ApplyAndCheckFreshIds(transformation, context.get(),
                            &transformation_context);
    }

    {
      TransformationSetMemoryOperandsMask transformation(
          MakeInstructionDescriptor(148, SpvOpStore, 0),
          SpvMemoryAccessMaskNone, 0);
      ASSERT_TRUE(
          transformation.IsApplicable(context.get(), transformation_context));
      ApplyAndCheckFreshIds(transformation, context.get(),
                            &transformation_context);
    }

    std::string after_transformation = R"(
                 OpCapability Shader
            %1 = OpExtInstImport "GLSL.std.450"
                 OpMemoryModel Logical GLSL450
                 OpEntryPoint Fragment %4 "main" %12 %17
                 OpExecutionMode %4 OriginUpperLeft
                 OpSource ESSL 310
                 OpName %4 "main"
                 OpName %7 "Point3D"
                 OpMemberName %7 0 "x"
                 OpMemberName %7 1 "y"
                 OpMemberName %7 2 "z"
                 OpName %12 "global_points"
                 OpName %15 "block"
                 OpMemberName %15 0 "in_points"
                 OpMemberName %15 1 "in_point"
                 OpName %17 ""
                 OpName %133 "local_points"
                 OpMemberDecorate %7 0 Offset 0
                 OpMemberDecorate %7 1 Offset 4
                 OpMemberDecorate %7 2 Offset 8
                 OpDecorate %10 ArrayStride 16
                 OpMemberDecorate %15 0 Offset 0
                 OpMemberDecorate %15 1 Offset 192
                 OpDecorate %15 Block
                 OpDecorate %17 DescriptorSet 0
                 OpDecorate %17 Binding 0
            %2 = OpTypeVoid
            %3 = OpTypeFunction %2
            %6 = OpTypeFloat 32
            %7 = OpTypeStruct %6 %6 %6
            %8 = OpTypeInt 32 0
            %9 = OpConstant %8 12
           %10 = OpTypeArray %7 %9
           %11 = OpTypePointer Private %10
           %12 = OpVariable %11 Private
           %15 = OpTypeStruct %10 %7
           %16 = OpTypePointer Uniform %15
           %17 = OpVariable %16 Uniform
           %18 = OpTypeInt 32 1
           %19 = OpConstant %18 0
           %20 = OpTypePointer Uniform %10
           %24 = OpTypePointer Private %7
           %27 = OpTypePointer Private %6
           %30 = OpConstant %18 1
          %132 = OpTypePointer Function %10
          %135 = OpTypePointer Uniform %7
          %145 = OpTypePointer Function %7
            %4 = OpFunction %2 None %3
            %5 = OpLabel
          %133 = OpVariable %132 Function
           %21 = OpAccessChain %20 %17 %19
                 OpCopyMemory %12 %21 Aligned 16 Aligned|Volatile 16
                 OpCopyMemory %133 %12 Volatile Nontemporal|Volatile
                 OpCopyMemory %133 %12 Nontemporal|Volatile
                 OpCopyMemory %133 %12 None Nontemporal|Volatile
          %136 = OpAccessChain %135 %17 %30
          %138 = OpAccessChain %24 %12 %19
                 OpCopyMemory %138 %136 None Aligned|Nontemporal 16
                 OpCopyMemory %138 %136 Aligned 16 Volatile
          %146 = OpAccessChain %145 %133 %30
          %147 = OpLoad %7 %146 Volatile|Aligned 16
          %148 = OpAccessChain %24 %12 %19
                 OpStore %148 %147 None
                 OpReturn
                 OpFunctionEnd
    )";
    ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
  }
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
