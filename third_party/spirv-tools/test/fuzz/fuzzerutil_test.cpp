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

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(FuzzerUtilMaybeFindBlockTest, BasicTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %8 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %10 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpBranch %11
         %11 = OpLabel
               OpStore %8 %9
               OpBranch %12
         %12 = OpLabel
               OpStore %8 %10
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const std::unique_ptr<opt::IRContext> context =
      BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Only blocks with id 11 and 12 can be found.
  // Should return nullptr when id is not a label or id was not found.
  uint32_t block_id1 = 11;
  uint32_t block_id2 = 12;
  uint32_t block_id3 = 13;
  uint32_t block_id4 = 8;

  opt::IRContext* ir_context = context.get();
  // Block with id 11 should be found.
  ASSERT_TRUE(fuzzerutil::MaybeFindBlock(ir_context, block_id1) != nullptr);
  // Block with id 12 should be found.
  ASSERT_TRUE(fuzzerutil::MaybeFindBlock(ir_context, block_id2) != nullptr);
  // Block with id 13 cannot be found.
  ASSERT_FALSE(fuzzerutil::MaybeFindBlock(ir_context, block_id3) != nullptr);
  // Block with id 8 exisits but don't not of type OpLabel.
  ASSERT_FALSE(fuzzerutil::MaybeFindBlock(ir_context, block_id4) != nullptr);
}

TEST(FuzzerutilTest, FuzzerUtilMaybeGetBoolConstantTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %36
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "b1"
               OpName %10 "b2"
               OpName %12 "b3"
               OpName %13 "b4"
               OpName %16 "f1"
               OpName %18 "f2"
               OpName %20 "cf1"
               OpName %22 "cf2"
               OpName %26 "i1"
               OpName %28 "i2"
               OpName %30 "ci1"
               OpName %32 "ci2"
               OpName %36 "value"
               OpDecorate %26 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %36 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpTypePointer Function %6
          %9 = OpConstantTrue %6
         %11 = OpConstantFalse %6
         %14 = OpTypeFloat 32
         %15 = OpTypePointer Function %14
         %17 = OpConstant %14 1.23000002
         %19 = OpConstant %14 1.11000001
         %21 = OpConstant %14 2
         %23 = OpConstant %14 3.29999995
         %24 = OpTypeInt 32 1
         %25 = OpTypePointer Function %24
         %27 = OpConstant %24 1
         %29 = OpConstant %24 100
         %31 = OpConstant %24 123
         %33 = OpConstant %24 1111
         %35 = OpTypePointer Input %14
         %36 = OpVariable %35 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %12 = OpVariable %7 Function
         %13 = OpVariable %7 Function
         %16 = OpVariable %15 Function
         %18 = OpVariable %15 Function
         %20 = OpVariable %15 Function
         %22 = OpVariable %15 Function
         %26 = OpVariable %25 Function
         %28 = OpVariable %25 Function
         %30 = OpVariable %25 Function
         %32 = OpVariable %25 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %12 %9
               OpStore %13 %11
               OpStore %16 %17
               OpStore %18 %19
               OpStore %20 %21
               OpStore %22 %23
               OpStore %26 %27
               OpStore %28 %29
               OpStore %30 %31
               OpStore %32 %33
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const std::unique_ptr<opt::IRContext> context =
      BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  opt::IRContext* ir_context = context.get();
  // A bool constant with value false exists and the id is 11.
  ASSERT_EQ(11, fuzzerutil::MaybeGetBoolConstant(
                    ir_context, transformation_context, false, false));
  // A bool constant with value true exists and the id is 9.
  ASSERT_EQ(9, fuzzerutil::MaybeGetBoolConstant(
                   ir_context, transformation_context, true, false));
}

TEST(FuzzerutilTest, FuzzerUtilMaybeGetBoolTypeTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %92 %52 %53
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %92 BuiltIn FragCoord
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFloat 32
          %8 = OpTypeStruct %6 %7
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %6 %9
         %14 = OpConstant %6 0
         %15 = OpTypePointer Function %6
         %51 = OpTypePointer Private %6
         %21 = OpConstant %6 2
         %23 = OpConstant %6 1
         %24 = OpConstant %7 1
         %25 = OpTypePointer Function %7
         %50 = OpTypePointer Private %7
         %34 = OpTypeBool
         %35 = OpConstantFalse %34
         %60 = OpConstantNull %50
         %61 = OpUndef %51
         %52 = OpVariable %50 Private
         %53 = OpVariable %51 Private
         %80 = OpConstantComposite %8 %21 %24
         %90 = OpTypeVector %7 4
         %91 = OpTypePointer Input %90
         %92 = OpVariable %91 Input
         %93 = OpConstantComposite %90 %24 %24 %24 %24
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpVariable %9 Function
         %27 = OpVariable %9 Function
         %22 = OpAccessChain %15 %20 %14
         %44 = OpCopyObject %9 %20
         %26 = OpAccessChain %25 %20 %23
         %29 = OpFunctionCall %6 %12 %27
         %30 = OpAccessChain %15 %20 %14
         %45 = OpCopyObject %15 %30
         %81 = OpCopyObject %9 %27
         %33 = OpAccessChain %15 %20 %14
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %38 = OpAccessChain %15 %20 %14
         %40 = OpAccessChain %15 %20 %14
         %43 = OpAccessChain %15 %20 %14
         %82 = OpCopyObject %9 %27
               OpBranch %37
         %37 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %6 None %10
         %11 = OpFunctionParameter %9
         %13 = OpLabel
         %46 = OpCopyObject %9 %11
         %16 = OpAccessChain %15 %11 %14
         %95 = OpCopyObject %8 %80
               OpReturnValue %21
        %100 = OpLabel
               OpUnreachable
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const std::unique_ptr<opt::IRContext> context =
      BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  opt::IRContext* ir_context = context.get();
  // A bool type with result id of 34 exists.
  ASSERT_TRUE(fuzzerutil::MaybeGetBoolType(ir_context));
}

TEST(FuzzerutilTest, FuzzerUtilMaybeGetCompositeConstantTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %54
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "b1"
               OpName %10 "b2"
               OpName %12 "b3"
               OpName %13 "b4"
               OpName %16 "f1"
               OpName %18 "f2"
               OpName %22 "zc"
               OpName %24 "i1"
               OpName %28 "i2"
               OpName %30 "i3"
               OpName %32 "i4"
               OpName %37 "f_arr"
               OpName %47 "i_arr"
               OpName %54 "value"
               OpDecorate %22 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %54 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpTypePointer Function %6
          %9 = OpConstantTrue %6
         %11 = OpConstantFalse %6
         %14 = OpTypeFloat 32
         %15 = OpTypePointer Function %14
         %17 = OpConstant %14 1.23000002
         %19 = OpConstant %14 1.11000001
         %20 = OpTypeInt 32 1
         %21 = OpTypePointer Function %20
         %23 = OpConstant %20 0
         %25 = OpConstant %20 1
         %26 = OpTypeInt 32 0
         %27 = OpTypePointer Function %26
         %29 = OpConstant %26 100
         %31 = OpConstant %20 -1
         %33 = OpConstant %20 -99
         %34 = OpConstant %26 5
         %35 = OpTypeArray %14 %34
         %36 = OpTypePointer Function %35
         %38 = OpConstant %14 5.5
         %39 = OpConstant %14 4.4000001
         %40 = OpConstant %14 3.29999995
         %41 = OpConstant %14 2.20000005
         %42 = OpConstant %14 1.10000002
         %43 = OpConstantComposite %35 %38 %39 %40 %41 %42
         %44 = OpConstant %26 3
         %45 = OpTypeArray %20 %44
         %46 = OpTypePointer Function %45
         %48 = OpConstant %20 3
         %49 = OpConstant %20 7
         %50 = OpConstant %20 9
         %51 = OpConstantComposite %45 %48 %49 %50
         %53 = OpTypePointer Input %14
         %54 = OpVariable %53 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %12 = OpVariable %7 Function
         %13 = OpVariable %7 Function
         %16 = OpVariable %15 Function
         %18 = OpVariable %15 Function
         %22 = OpVariable %21 Function
         %24 = OpVariable %21 Function
         %28 = OpVariable %27 Function
         %30 = OpVariable %21 Function
         %32 = OpVariable %21 Function
         %37 = OpVariable %36 Function
         %47 = OpVariable %46 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %12 %9
               OpStore %13 %11
               OpStore %16 %17
               OpStore %18 %19
               OpStore %22 %23
               OpStore %24 %25
               OpStore %28 %29
               OpStore %30 %31
               OpStore %32 %33
               OpStore %37 %43
               OpStore %47 %51
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const std::unique_ptr<opt::IRContext> context =
      BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  opt::IRContext* ir_context = context.get();

  //      %43 = OpConstantComposite %35 %38 %39 %40 %41 %42
  //    %51 = OpConstantComposite %45 %48 %49 %50
  // This should pass as a float array with 5 elements exist and its id is 43.
  ASSERT_EQ(43, fuzzerutil::MaybeGetCompositeConstant(
                    ir_context, transformation_context, {38, 39, 40, 41, 42},
                    35, false));
  // This should pass as an int array with 3 elements exist and its id is 51.
  ASSERT_EQ(51,
            fuzzerutil::MaybeGetCompositeConstant(
                ir_context, transformation_context, {48, 49, 50}, 45, false));
  // An int array with 2 elements does not exist.
  ASSERT_EQ(0, fuzzerutil::MaybeGetCompositeConstant(
                   ir_context, transformation_context, {48, 49}, 45, false));
}

TEST(FuzzerutilTest, FuzzerUtilMaybeGetFloatConstantTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %36
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "b1"
               OpName %10 "b2"
               OpName %12 "b3"
               OpName %13 "b4"
               OpName %16 "f1"
               OpName %18 "f2"
               OpName %20 "cf1"
               OpName %22 "cf2"
               OpName %26 "i1"
               OpName %28 "i2"
               OpName %30 "ci1"
               OpName %32 "ci2"
               OpName %36 "value"
               OpDecorate %26 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %36 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpTypePointer Function %6
          %9 = OpConstantTrue %6
         %11 = OpConstantFalse %6
         %14 = OpTypeFloat 32
         %15 = OpTypePointer Function %14
         %17 = OpConstant %14 1.23000002
         %19 = OpConstant %14 1.11000001
         %21 = OpConstant %14 2
         %23 = OpConstant %14 3.29999995
         %24 = OpTypeInt 32 1
         %25 = OpTypePointer Function %24
         %27 = OpConstant %24 1
         %29 = OpConstant %24 100
         %31 = OpConstant %24 123
         %33 = OpConstant %24 1111
         %35 = OpTypePointer Input %14
         %36 = OpVariable %35 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %12 = OpVariable %7 Function
         %13 = OpVariable %7 Function
         %16 = OpVariable %15 Function
         %18 = OpVariable %15 Function
         %20 = OpVariable %15 Function
         %22 = OpVariable %15 Function
         %26 = OpVariable %25 Function
         %28 = OpVariable %25 Function
         %30 = OpVariable %25 Function
         %32 = OpVariable %25 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %12 %9
               OpStore %13 %11
               OpStore %16 %17
               OpStore %18 %19
               OpStore %20 %21
               OpStore %22 %23
               OpStore %26 %27
               OpStore %28 %29
               OpStore %30 %31
               OpStore %32 %33
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const std::unique_ptr<opt::IRContext> context =
      BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  opt::IRContext* ir_context = context.get();

  uint32_t word1 = fuzzerutil::FloatToWord(2);
  uint32_t word2 = fuzzerutil::FloatToWord(1.23f);

  // A 32 bit float constant of value 2 exists and its id is 21.
  ASSERT_EQ(21, fuzzerutil::MaybeGetFloatConstant(
                    ir_context, transformation_context,
                    std::vector<uint32_t>{word1}, 32, false));
  // A 32 bit float constant of value 1.23 exists and its id is 17.
  ASSERT_EQ(17, fuzzerutil::MaybeGetFloatConstant(
                    ir_context, transformation_context,
                    std::vector<uint32_t>{word2}, 32, false));
}

TEST(FuzzerutilTest, FuzzerUtilMaybeGetFloatTypeTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %92 %52 %53
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %92 BuiltIn FragCoord
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFloat 32
          %8 = OpTypeStruct %6 %7
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %6 %9
         %14 = OpConstant %6 0
         %15 = OpTypePointer Function %6
         %51 = OpTypePointer Private %6
         %21 = OpConstant %6 2
         %23 = OpConstant %6 1
         %24 = OpConstant %7 1
         %25 = OpTypePointer Function %7
         %50 = OpTypePointer Private %7
         %34 = OpTypeBool
         %35 = OpConstantFalse %34
         %60 = OpConstantNull %50
         %61 = OpUndef %51
         %52 = OpVariable %50 Private
         %53 = OpVariable %51 Private
         %80 = OpConstantComposite %8 %21 %24
         %90 = OpTypeVector %7 4
         %91 = OpTypePointer Input %90
         %92 = OpVariable %91 Input
         %93 = OpConstantComposite %90 %24 %24 %24 %24
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpVariable %9 Function
         %27 = OpVariable %9 Function
         %22 = OpAccessChain %15 %20 %14
         %44 = OpCopyObject %9 %20
         %26 = OpAccessChain %25 %20 %23
         %29 = OpFunctionCall %6 %12 %27
         %30 = OpAccessChain %15 %20 %14
         %45 = OpCopyObject %15 %30
         %81 = OpCopyObject %9 %27
         %33 = OpAccessChain %15 %20 %14
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %38 = OpAccessChain %15 %20 %14
         %40 = OpAccessChain %15 %20 %14
         %43 = OpAccessChain %15 %20 %14
         %82 = OpCopyObject %9 %27
               OpBranch %37
         %37 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %6 None %10
         %11 = OpFunctionParameter %9
         %13 = OpLabel
         %46 = OpCopyObject %9 %11
         %16 = OpAccessChain %15 %11 %14
         %95 = OpCopyObject %8 %80
               OpReturnValue %21
        %100 = OpLabel
               OpUnreachable
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const std::unique_ptr<opt::IRContext> context =
      BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  opt::IRContext* ir_context = context.get();
  // A float type with width = 32 and result id of 7 exists.
  ASSERT_EQ(7, fuzzerutil::MaybeGetFloatType(ir_context, 32));

  // A float int type with width = 32 exists, but the id should be 7.
  ASSERT_NE(5, fuzzerutil::MaybeGetFloatType(ir_context, 32));

  // A float type with width 30 does not exist.
  ASSERT_EQ(0, fuzzerutil::MaybeGetFloatType(ir_context, 30));
}

TEST(FuzzerutilTest, FuzzerUtilMaybeGetIntegerConstantFromValueAndTypeTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %36
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "b1"
               OpName %10 "b2"
               OpName %12 "b3"
               OpName %13 "b4"
               OpName %16 "f1"
               OpName %18 "f2"
               OpName %22 "zc"
               OpName %24 "i1"
               OpName %28 "i2"
               OpName %30 "i3"
               OpName %32 "i4"
               OpName %36 "value"
               OpDecorate %22 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %36 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpTypePointer Function %6
          %9 = OpConstantTrue %6
         %11 = OpConstantFalse %6
         %14 = OpTypeFloat 32
         %15 = OpTypePointer Function %14
         %17 = OpConstant %14 1.23000002
         %19 = OpConstant %14 1.11000001
         %20 = OpTypeInt 32 1
         %21 = OpTypePointer Function %20
         %23 = OpConstant %20 0
         %25 = OpConstant %20 1
         %26 = OpTypeInt 32 0
         %27 = OpTypePointer Function %26
         %29 = OpConstant %26 100
         %31 = OpConstant %20 -1
         %33 = OpConstant %20 -99
         %35 = OpTypePointer Input %14
         %36 = OpVariable %35 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %12 = OpVariable %7 Function
         %13 = OpVariable %7 Function
         %16 = OpVariable %15 Function
         %18 = OpVariable %15 Function
         %22 = OpVariable %21 Function
         %24 = OpVariable %21 Function
         %28 = OpVariable %27 Function
         %30 = OpVariable %21 Function
         %32 = OpVariable %21 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %12 %9
               OpStore %13 %11
               OpStore %16 %17
               OpStore %18 %19
               OpStore %22 %23
               OpStore %24 %25
               OpStore %28 %29
               OpStore %30 %31
               OpStore %32 %33
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const std::unique_ptr<opt::IRContext> context =
      BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  opt::IRContext* ir_context = context.get();

  // A 32 bit signed int constant (with int type id 20) with value 1 exists and
  // the id is 25.
  ASSERT_EQ(25, fuzzerutil::MaybeGetIntegerConstantFromValueAndType(ir_context,
                                                                    1, 20));
  // A 32 bit unsigned int constant (with int type id 0) with value 100 exists
  // and the id is 29.
  ASSERT_EQ(29, fuzzerutil::MaybeGetIntegerConstantFromValueAndType(ir_context,
                                                                    100, 26));
  // A 32 bit unsigned int constant with value 50 does not exist.
  ASSERT_EQ(0, fuzzerutil::MaybeGetIntegerConstantFromValueAndType(ir_context,
                                                                   50, 26));
}

TEST(FuzzerutilTest, FuzzerUtilMaybeGetIntegerConstantTest) {
  std::string shader = R"(
OpCapability Shader
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %36
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "b1"
               OpName %10 "b2"
               OpName %12 "b3"
               OpName %13 "b4"
               OpName %16 "f1"
               OpName %18 "f2"
               OpName %22 "zc"
               OpName %24 "i1"
               OpName %28 "i2"
               OpName %30 "i3"
               OpName %32 "i4"
               OpName %36 "value"
               OpDecorate %22 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %36 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpTypePointer Function %6
          %9 = OpConstantTrue %6
         %11 = OpConstantFalse %6
         %14 = OpTypeFloat 32
         %15 = OpTypePointer Function %14
         %17 = OpConstant %14 1.23000002
         %19 = OpConstant %14 1.11000001
         %20 = OpTypeInt 32 1
         %21 = OpTypePointer Function %20
         %23 = OpConstant %20 0
         %25 = OpConstant %20 1
         %26 = OpTypeInt 32 0
         %27 = OpTypePointer Function %26
         %29 = OpConstant %26 100
         %31 = OpConstant %20 -1
         %33 = OpConstant %20 -99
         %35 = OpTypePointer Input %14
         %36 = OpVariable %35 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %12 = OpVariable %7 Function
         %13 = OpVariable %7 Function
         %16 = OpVariable %15 Function
         %18 = OpVariable %15 Function
         %22 = OpVariable %21 Function
         %24 = OpVariable %21 Function
         %28 = OpVariable %27 Function
         %30 = OpVariable %21 Function
         %32 = OpVariable %21 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %12 %9
               OpStore %13 %11
               OpStore %16 %17
               OpStore %18 %19
               OpStore %22 %23
               OpStore %24 %25
               OpStore %28 %29
               OpStore %30 %31
               OpStore %32 %33
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const std::unique_ptr<opt::IRContext> context =
      BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  opt::IRContext* ir_context = context.get();

  // A 32 bit unsigned int constant with value 1 exists and the id is 25.
  ASSERT_EQ(25, fuzzerutil::MaybeGetIntegerConstant(
                    ir_context, transformation_context,
                    std::vector<uint32_t>{1}, 32, true, false));
  // A 32 bit unsigned int constant with value 100 exists and the id is 29.
  ASSERT_EQ(29, fuzzerutil::MaybeGetIntegerConstant(
                    ir_context, transformation_context,
                    std::vector<uint32_t>{100}, 32, false, false));
  // A 32 bit signed int constant with value 99 doesn't not exist and should
  // return 0.
  ASSERT_EQ(0, fuzzerutil::MaybeGetIntegerConstant(
                   ir_context, transformation_context,
                   std::vector<uint32_t>{99}, 32, true, false));
}

TEST(FuzzerutilTest, FuzzerUtilMaybeGetIntegerTypeTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %92 %52 %53
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %92 BuiltIn FragCoord
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFloat 32
          %8 = OpTypeStruct %6 %7
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %6 %9
         %14 = OpConstant %6 0
         %15 = OpTypePointer Function %6
         %51 = OpTypePointer Private %6
         %21 = OpConstant %6 2
         %23 = OpConstant %6 1
         %24 = OpConstant %7 1
         %25 = OpTypePointer Function %7
         %50 = OpTypePointer Private %7
         %34 = OpTypeBool
         %35 = OpConstantFalse %34
         %60 = OpConstantNull %50
         %61 = OpUndef %51
         %52 = OpVariable %50 Private
         %53 = OpVariable %51 Private
         %80 = OpConstantComposite %8 %21 %24
         %90 = OpTypeVector %7 4
         %91 = OpTypePointer Input %90
         %92 = OpVariable %91 Input
         %93 = OpConstantComposite %90 %24 %24 %24 %24
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpVariable %9 Function
         %27 = OpVariable %9 Function
         %22 = OpAccessChain %15 %20 %14
         %44 = OpCopyObject %9 %20
         %26 = OpAccessChain %25 %20 %23
         %29 = OpFunctionCall %6 %12 %27
         %30 = OpAccessChain %15 %20 %14
         %45 = OpCopyObject %15 %30
         %81 = OpCopyObject %9 %27
         %33 = OpAccessChain %15 %20 %14
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %38 = OpAccessChain %15 %20 %14
         %40 = OpAccessChain %15 %20 %14
         %43 = OpAccessChain %15 %20 %14
         %82 = OpCopyObject %9 %27
               OpBranch %37
         %37 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %6 None %10
         %11 = OpFunctionParameter %9
         %13 = OpLabel
         %46 = OpCopyObject %9 %11
         %16 = OpAccessChain %15 %11 %14
         %95 = OpCopyObject %8 %80
               OpReturnValue %21
        %100 = OpLabel
               OpUnreachable
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const std::unique_ptr<opt::IRContext> context =
      BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  opt::IRContext* ir_context = context.get();

  // A signed int type with width = 32 and result id of 6 exists.
  ASSERT_EQ(6, fuzzerutil::MaybeGetIntegerType(ir_context, 32, true));

  // A signed int type with width = 32 exists, but the id should be 6.
  ASSERT_FALSE(fuzzerutil::MaybeGetIntegerType(ir_context, 32, true) == 5);

  // A int type with width = 32 and result id of 6 exists, but it should be a
  // signed int.
  ASSERT_EQ(0, fuzzerutil::MaybeGetIntegerType(ir_context, 32, false));
  // A signed int type with width 30 does not exist.
  ASSERT_EQ(0, fuzzerutil::MaybeGetIntegerType(ir_context, 30, true));
  // An unsigned int type with width 22 does not exist.
  ASSERT_EQ(0, fuzzerutil::MaybeGetIntegerType(ir_context, 22, false));
}

TEST(FuzzerutilTest, FuzzerUtilMaybeGetPointerTypeTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %92 %52 %53
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %92 BuiltIn FragCoord
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFloat 32
          %8 = OpTypeStruct %6 %7
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %6 %9
         %14 = OpConstant %6 0
         %15 = OpTypePointer Function %6
         %51 = OpTypePointer Private %6
         %21 = OpConstant %6 2
         %23 = OpConstant %6 1
         %24 = OpConstant %7 1
         %25 = OpTypePointer Function %7
         %50 = OpTypePointer Private %7
         %34 = OpTypeBool
         %35 = OpConstantFalse %34
         %60 = OpConstantNull %50
         %61 = OpUndef %51
         %52 = OpVariable %50 Private
         %53 = OpVariable %51 Private
         %80 = OpConstantComposite %8 %21 %24
         %90 = OpTypeVector %7 4
         %91 = OpTypePointer Input %90
         %92 = OpVariable %91 Input
         %93 = OpConstantComposite %90 %24 %24 %24 %24
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpVariable %9 Function
         %27 = OpVariable %9 Function
         %22 = OpAccessChain %15 %20 %14
         %44 = OpCopyObject %9 %20
         %26 = OpAccessChain %25 %20 %23
         %29 = OpFunctionCall %6 %12 %27
         %30 = OpAccessChain %15 %20 %14
         %45 = OpCopyObject %15 %30
         %81 = OpCopyObject %9 %27
         %33 = OpAccessChain %15 %20 %14
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %38 = OpAccessChain %15 %20 %14
         %40 = OpAccessChain %15 %20 %14
         %43 = OpAccessChain %15 %20 %14
         %82 = OpCopyObject %9 %27
               OpBranch %37
         %37 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %6 None %10
         %11 = OpFunctionParameter %9
         %13 = OpLabel
         %46 = OpCopyObject %9 %11
         %16 = OpAccessChain %15 %11 %14
         %95 = OpCopyObject %8 %80
               OpReturnValue %21
        %100 = OpLabel
               OpUnreachable
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const std::unique_ptr<opt::IRContext> context =
      BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  opt::IRContext* ir_context = context.get();
  auto private_storage_class = SpvStorageClassPrivate;
  auto function_storage_class = SpvStorageClassFunction;
  auto input_storage_class = SpvStorageClassInput;

  // A valid pointer must have the correct |pointee_type_id| and |storageClass|.
  // A function type pointer with id = 9 and pointee type id 8 should be found.
  ASSERT_EQ(9, fuzzerutil::MaybeGetPointerType(ir_context, 8,
                                               function_storage_class));
  // A function type pointer with id = 15 and pointee type id 6 should be found.
  ASSERT_EQ(15, fuzzerutil::MaybeGetPointerType(ir_context, 6,
                                                function_storage_class));
  // A function type pointer with id = 25 and pointee type id 7 should be found.
  ASSERT_EQ(25, fuzzerutil::MaybeGetPointerType(ir_context, 7,
                                                function_storage_class));

  // A private type pointer with id=51 and pointee type id 6 should be found.
  ASSERT_EQ(51, fuzzerutil::MaybeGetPointerType(ir_context, 6,
                                                private_storage_class));
  // A function pointer with id=50 and pointee type id 7 should be found.
  ASSERT_EQ(50, fuzzerutil::MaybeGetPointerType(ir_context, 7,
                                                private_storage_class));

  // A input type pointer with id=91 and pointee type id 90 should be found.
  ASSERT_EQ(
      91, fuzzerutil::MaybeGetPointerType(ir_context, 90, input_storage_class));

  // A pointer with id=91 and pointee type 90 exisits, but the type should be
  // input.
  ASSERT_EQ(0, fuzzerutil::MaybeGetPointerType(ir_context, 90,
                                               function_storage_class));
  // A input type pointer with id=91 exists but the pointee id should be 90.
  ASSERT_EQ(
      0, fuzzerutil::MaybeGetPointerType(ir_context, 89, input_storage_class));
  // A input type pointer with pointee id 90 exists but result id of the pointer
  // should be 91.
  ASSERT_NE(
      58, fuzzerutil::MaybeGetPointerType(ir_context, 90, input_storage_class));
}

TEST(FuzzerutilTest, FuzzerUtilMaybeGetScalarConstantTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %56
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "b1"
               OpName %10 "b2"
               OpName %12 "b3"
               OpName %13 "b4"
               OpName %16 "f1"
               OpName %18 "f2"
               OpName %22 "zc"
               OpName %24 "i1"
               OpName %28 "i2"
               OpName %30 "i"
               OpName %32 "i3"
               OpName %34 "i4"
               OpName %39 "f_arr"
               OpName %49 "i_arr"
               OpName %56 "value"
               OpDecorate %22 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %56 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpTypePointer Function %6
          %9 = OpConstantTrue %6
         %11 = OpConstantFalse %6
         %14 = OpTypeFloat 32
         %15 = OpTypePointer Function %14
         %17 = OpConstant %14 1.23000002
         %19 = OpConstant %14 1.11000001
         %20 = OpTypeInt 32 1
         %21 = OpTypePointer Function %20
         %23 = OpConstant %20 0
         %25 = OpConstant %20 1
         %26 = OpTypeInt 32 0
         %27 = OpTypePointer Function %26
         %29 = OpConstant %26 100
         %31 = OpConstant %26 0
         %33 = OpConstant %20 -1
         %35 = OpConstant %20 -99
         %36 = OpConstant %26 5
         %37 = OpTypeArray %14 %36
         %38 = OpTypePointer Function %37
         %40 = OpConstant %14 5.5
         %41 = OpConstant %14 4.4000001
         %42 = OpConstant %14 3.29999995
         %43 = OpConstant %14 2.20000005
         %44 = OpConstant %14 1.10000002
         %45 = OpConstantComposite %37 %40 %41 %42 %43 %44
         %46 = OpConstant %26 3
         %47 = OpTypeArray %20 %46
         %48 = OpTypePointer Function %47
         %50 = OpConstant %20 3
         %51 = OpConstant %20 7
         %52 = OpConstant %20 9
         %53 = OpConstantComposite %47 %50 %51 %52
         %55 = OpTypePointer Input %14
         %56 = OpVariable %55 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %12 = OpVariable %7 Function
         %13 = OpVariable %7 Function
         %16 = OpVariable %15 Function
         %18 = OpVariable %15 Function
         %22 = OpVariable %21 Function
         %24 = OpVariable %21 Function
         %28 = OpVariable %27 Function
         %30 = OpVariable %27 Function
         %32 = OpVariable %21 Function
         %34 = OpVariable %21 Function
         %39 = OpVariable %38 Function
         %49 = OpVariable %48 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %12 %9
               OpStore %13 %11
               OpStore %16 %17
               OpStore %18 %19
               OpStore %22 %23
               OpStore %24 %25
               OpStore %28 %29
               OpStore %30 %31
               OpStore %32 %33
               OpStore %34 %35
               OpStore %39 %45
               OpStore %49 %53
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const std::unique_ptr<opt::IRContext> context =
      BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  opt::IRContext* ir_context = context.get();

  std::vector<uint32_t> uint_words1 = fuzzerutil::IntToWords(100, 32, false);
  std::vector<uint32_t> uint_words2 = fuzzerutil::IntToWords(0, 32, false);
  std::vector<uint32_t> int_words1 = fuzzerutil::IntToWords(-99, 32, true);
  std::vector<uint32_t> int_words2 = fuzzerutil::IntToWords(1, 32, true);
  uint32_t float_word1 = fuzzerutil::FloatToWord(1.11f);
  uint32_t float_word2 = fuzzerutil::FloatToWord(4.4f);

  // A unsigned int of value 100 that has a scalar type id of 26 exists and its
  // id is 29.
  ASSERT_EQ(
      29, fuzzerutil::MaybeGetScalarConstant(ir_context, transformation_context,
                                             uint_words1, 26, false));
  // A unsigned int of value 0 that has a scalar type id of 26 exists and its id
  // is 29.
  ASSERT_EQ(
      31, fuzzerutil::MaybeGetScalarConstant(ir_context, transformation_context,
                                             uint_words2, 26, false));
  // A signed int of value -99 that has a scalar type id of 20 exists and its id
  // is 35.
  ASSERT_EQ(35, fuzzerutil::MaybeGetScalarConstant(
                    ir_context, transformation_context, int_words1, 20, false));
  // A signed int of value 1 that has a scalar type id of 20 exists and its id
  // is 25.
  ASSERT_EQ(25, fuzzerutil::MaybeGetScalarConstant(
                    ir_context, transformation_context, int_words2, 20, false));
  // A float of value 1.11 that has a scalar type id of 14 exists and its id
  // is 19.
  ASSERT_EQ(19, fuzzerutil::MaybeGetScalarConstant(
                    ir_context, transformation_context,
                    std::vector<uint32_t>{float_word1}, 14, false));
  // A signed int of value 1 that has a scalar type id of 20 exists and its id
  // is 25.
  ASSERT_EQ(41, fuzzerutil::MaybeGetScalarConstant(
                    ir_context, transformation_context,
                    std::vector<uint32_t>{float_word2}, 14, false));
}

TEST(FuzzerutilTest, FuzzerUtilMaybeGetStructTypeTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %92 %52 %53
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %92 BuiltIn FragCoord
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFloat 32
          %8 = OpTypeStruct %6 %7
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %6 %9
         %14 = OpConstant %6 0
         %15 = OpTypePointer Function %6
         %51 = OpTypePointer Private %6
         %21 = OpConstant %6 2
         %23 = OpConstant %6 1
         %24 = OpConstant %7 1
         %25 = OpTypePointer Function %7
         %50 = OpTypePointer Private %7
         %34 = OpTypeBool
         %35 = OpConstantFalse %34
         %60 = OpConstantNull %50
         %61 = OpUndef %51
         %52 = OpVariable %50 Private
         %53 = OpVariable %51 Private
         %80 = OpConstantComposite %8 %21 %24
         %90 = OpTypeVector %7 4
         %91 = OpTypePointer Input %90
         %92 = OpVariable %91 Input
         %93 = OpConstantComposite %90 %24 %24 %24 %24
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpVariable %9 Function
         %27 = OpVariable %9 Function
         %22 = OpAccessChain %15 %20 %14
         %44 = OpCopyObject %9 %20
         %26 = OpAccessChain %25 %20 %23
         %29 = OpFunctionCall %6 %12 %27
         %30 = OpAccessChain %15 %20 %14
         %45 = OpCopyObject %15 %30
         %81 = OpCopyObject %9 %27
         %33 = OpAccessChain %15 %20 %14
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %38 = OpAccessChain %15 %20 %14
         %40 = OpAccessChain %15 %20 %14
         %43 = OpAccessChain %15 %20 %14
         %82 = OpCopyObject %9 %27
               OpBranch %37
         %37 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %6 None %10
         %11 = OpFunctionParameter %9
         %13 = OpLabel
         %46 = OpCopyObject %9 %11
         %16 = OpAccessChain %15 %11 %14
         %95 = OpCopyObject %8 %80
               OpReturnValue %21
        %100 = OpLabel
               OpUnreachable
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const std::unique_ptr<opt::IRContext> context =
      BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  opt::IRContext* ir_context = context.get();

  // 6 and 7 are all valid ids from OpTypeInt and OpTypeFloat
  // so the result id of 8 should be found.
  ASSERT_EQ(8, fuzzerutil::MaybeGetStructType(ir_context,
                                              std::vector<uint32_t>{6, 7}));

  // |component_type_id| of 16 does not exist in the module, so such a struct
  // type cannot be found.
  ASSERT_EQ(0, fuzzerutil::MaybeGetStructType(ir_context,
                                              std::vector<uint32_t>(6, 16)));

  // |component_type_id| of 10 is of OpTypeFunction type and thus the struct
  // cannot be found.
  ASSERT_EQ(0, fuzzerutil::MaybeGetStructType(ir_context,
                                              std::vector<uint32_t>(6, 10)));
}

TEST(FuzzerutilTest, FuzzerUtilMaybeGetVectorTypeTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %92 %52 %53
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %92 BuiltIn FragCoord
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFloat 32
          %8 = OpTypeStruct %6 %7
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %6 %9
         %14 = OpConstant %6 0
         %15 = OpTypePointer Function %6
         %51 = OpTypePointer Private %6
         %21 = OpConstant %6 2
         %23 = OpConstant %6 1
         %24 = OpConstant %7 1
         %25 = OpTypePointer Function %7
         %50 = OpTypePointer Private %7
         %34 = OpTypeBool
         %35 = OpConstantFalse %34
         %60 = OpConstantNull %50
         %61 = OpUndef %51
         %52 = OpVariable %50 Private
         %53 = OpVariable %51 Private
         %80 = OpConstantComposite %8 %21 %24
         %90 = OpTypeVector %7 4
         %91 = OpTypePointer Input %90
         %92 = OpVariable %91 Input
         %93 = OpConstantComposite %90 %24 %24 %24 %24
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpVariable %9 Function
         %27 = OpVariable %9 Function
         %22 = OpAccessChain %15 %20 %14
         %44 = OpCopyObject %9 %20
         %26 = OpAccessChain %25 %20 %23
         %29 = OpFunctionCall %6 %12 %27
         %30 = OpAccessChain %15 %20 %14
         %45 = OpCopyObject %15 %30
         %81 = OpCopyObject %9 %27
         %33 = OpAccessChain %15 %20 %14
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %38 = OpAccessChain %15 %20 %14
         %40 = OpAccessChain %15 %20 %14
         %43 = OpAccessChain %15 %20 %14
         %82 = OpCopyObject %9 %27
               OpBranch %37
         %37 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %6 None %10
         %11 = OpFunctionParameter %9
         %13 = OpLabel
         %46 = OpCopyObject %9 %11
         %16 = OpAccessChain %15 %11 %14
         %95 = OpCopyObject %8 %80
               OpReturnValue %21
        %100 = OpLabel
               OpUnreachable
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const std::unique_ptr<opt::IRContext> context =
      BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  opt::IRContext* ir_context = context.get();
  // The vector type with |element_count| 4 and |component_type_id| 7
  // is present and has a result id of 90.
  ASSERT_EQ(90, fuzzerutil::MaybeGetVectorType(ir_context, 7, 4));

  // The vector type with |element_count| 3 and |component_type_id| 7
  // is not present in the module.
  ASSERT_EQ(0, fuzzerutil::MaybeGetVectorType(ir_context, 7, 3));

#ifndef NDEBUG
  // It should abort with |component_type_id| of 100
  // |component_type_id| must be a valid result id of an OpTypeInt,
  // OpTypeFloat or OpTypeBool instruction in the module.
  ASSERT_DEATH(fuzzerutil::MaybeGetVectorType(ir_context, 100, 4),
               "\\|component_type_id\\| is invalid");

  // It should abort with |element_count| of 5.
  // |element_count| must be in the range [2,4].
  ASSERT_DEATH(fuzzerutil::MaybeGetVectorType(ir_context, 7, 5),
               "Precondition: component count must be in range \\[2, 4\\].");
#endif
}

TEST(FuzzerutilTest, FuzzerUtilMaybeGetVoidTypeTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %92 %52 %53
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %92 BuiltIn FragCoord
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFloat 32
          %8 = OpTypeStruct %6 %7
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %6 %9
         %14 = OpConstant %6 0
         %15 = OpTypePointer Function %6
         %51 = OpTypePointer Private %6
         %21 = OpConstant %6 2
         %23 = OpConstant %6 1
         %24 = OpConstant %7 1
         %25 = OpTypePointer Function %7
         %50 = OpTypePointer Private %7
         %34 = OpTypeBool
         %35 = OpConstantFalse %34
         %60 = OpConstantNull %50
         %61 = OpUndef %51
         %52 = OpVariable %50 Private
         %53 = OpVariable %51 Private
         %80 = OpConstantComposite %8 %21 %24
         %90 = OpTypeVector %7 4
         %91 = OpTypePointer Input %90
         %92 = OpVariable %91 Input
         %93 = OpConstantComposite %90 %24 %24 %24 %24
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpVariable %9 Function
         %27 = OpVariable %9 Function
         %22 = OpAccessChain %15 %20 %14
         %44 = OpCopyObject %9 %20
         %26 = OpAccessChain %25 %20 %23
         %29 = OpFunctionCall %6 %12 %27
         %30 = OpAccessChain %15 %20 %14
         %45 = OpCopyObject %15 %30
         %81 = OpCopyObject %9 %27
         %33 = OpAccessChain %15 %20 %14
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %38 = OpAccessChain %15 %20 %14
         %40 = OpAccessChain %15 %20 %14
         %43 = OpAccessChain %15 %20 %14
         %82 = OpCopyObject %9 %27
               OpBranch %37
         %37 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %6 None %10
         %11 = OpFunctionParameter %9
         %13 = OpLabel
         %46 = OpCopyObject %9 %11
         %16 = OpAccessChain %15 %11 %14
         %95 = OpCopyObject %8 %80
               OpReturnValue %21
        %100 = OpLabel
               OpUnreachable
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const std::unique_ptr<opt::IRContext> context =
      BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  opt::IRContext* ir_context = context.get();
  // A void type with a result id of 2 can be found.
  ASSERT_EQ(2, fuzzerutil::MaybeGetVoidType(ir_context));
}

TEST(FuzzerutilTest, FuzzerUtilMaybeGetZeroConstantTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %56
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "b1"
               OpName %10 "b2"
               OpName %12 "b3"
               OpName %13 "b4"
               OpName %16 "f1"
               OpName %18 "f2"
               OpName %22 "zc"
               OpName %24 "i1"
               OpName %28 "i2"
               OpName %30 "i"
               OpName %32 "i3"
               OpName %34 "i4"
               OpName %39 "f_arr"
               OpName %49 "i_arr"
               OpName %56 "value"
               OpDecorate %22 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %56 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpTypePointer Function %6
          %9 = OpConstantTrue %6
         %11 = OpConstantFalse %6
         %14 = OpTypeFloat 32
         %15 = OpTypePointer Function %14
         %17 = OpConstant %14 1.23000002
         %19 = OpConstant %14 1.11000001
         %20 = OpTypeInt 32 1
         %21 = OpTypePointer Function %20
         %23 = OpConstant %20 0
         %25 = OpConstant %20 1
         %26 = OpTypeInt 32 0
         %27 = OpTypePointer Function %26
         %29 = OpConstant %26 100
         %31 = OpConstant %26 0
         %33 = OpConstant %20 -1
         %35 = OpConstant %20 -99
         %36 = OpConstant %26 5
         %37 = OpTypeArray %14 %36
         %38 = OpTypePointer Function %37
         %40 = OpConstant %14 5.5
         %41 = OpConstant %14 4.4000001
         %42 = OpConstant %14 3.29999995
         %43 = OpConstant %14 2.20000005
         %44 = OpConstant %14 1.10000002
         %45 = OpConstantComposite %37 %40 %41 %42 %43 %44
         %46 = OpConstant %26 3
         %47 = OpTypeArray %20 %46
         %48 = OpTypePointer Function %47
         %50 = OpConstant %20 3
         %51 = OpConstant %20 7
         %52 = OpConstant %20 9
         %53 = OpConstantComposite %47 %50 %51 %52
         %55 = OpTypePointer Input %14
         %56 = OpVariable %55 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %12 = OpVariable %7 Function
         %13 = OpVariable %7 Function
         %16 = OpVariable %15 Function
         %18 = OpVariable %15 Function
         %22 = OpVariable %21 Function
         %24 = OpVariable %21 Function
         %28 = OpVariable %27 Function
         %30 = OpVariable %27 Function
         %32 = OpVariable %21 Function
         %34 = OpVariable %21 Function
         %39 = OpVariable %38 Function
         %49 = OpVariable %48 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %12 %9
               OpStore %13 %11
               OpStore %16 %17
               OpStore %18 %19
               OpStore %22 %23
               OpStore %24 %25
               OpStore %28 %29
               OpStore %30 %31
               OpStore %32 %33
               OpStore %34 %35
               OpStore %39 %45
               OpStore %49 %53
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const std::unique_ptr<opt::IRContext> context =
      BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  opt::IRContext* ir_context = context.get();

  // The id of a boolean constant will be returned give boolean type id 6.
  uint32_t maybe_bool_id = fuzzerutil::MaybeGetZeroConstant(
      ir_context, transformation_context, 6, false);
  // The id of a 32 bit float constant will be returned given the float type
  // id 14.
  uint32_t maybe_float_id = fuzzerutil::MaybeGetZeroConstant(
      ir_context, transformation_context, 14, false);
  uint32_t maybe_signed_int_id = fuzzerutil::MaybeGetZeroConstant(
      ir_context, transformation_context, 20, false);
  uint32_t maybe_unsigned_int_id = fuzzerutil::MaybeGetZeroConstant(
      ir_context, transformation_context, 26, false);

  // Lists of possible ids for float, signed int, unsigned int and array.
  std::vector<uint32_t> float_ids{17, 19};
  std::vector<uint32_t> signed_int_ids{23, 25, 31, 33};

  ASSERT_TRUE(maybe_bool_id == 9 || maybe_bool_id == 11);
  ASSERT_TRUE(std::find(signed_int_ids.begin(), signed_int_ids.end(),
                        maybe_signed_int_id) != signed_int_ids.end());

  // There is a unsigned int typed zero constant and its id is 31.
  ASSERT_EQ(31, maybe_unsigned_int_id);

  // There is no zero float constant.
  ASSERT_TRUE(std::find(float_ids.begin(), float_ids.end(), maybe_float_id) ==
              float_ids.end());
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
