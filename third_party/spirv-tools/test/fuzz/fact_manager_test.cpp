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

#include <limits>

#include "source/fuzz/fact_manager.h"
#include "source/fuzz/uniform_buffer_element_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

using opt::analysis::BoolConstant;
using opt::analysis::FloatConstant;
using opt::analysis::IntConstant;
using opt::analysis::ScalarConstant;

using opt::analysis::Bool;
using opt::analysis::Float;
using opt::analysis::Integer;
using opt::analysis::Type;

bool AddFactHelper(
    FactManager* fact_manager, opt::IRContext* context,
    std::vector<uint32_t>&& words,
    const protobufs::UniformBufferElementDescriptor& descriptor) {
  protobufs::FactConstantUniform constant_uniform_fact;
  for (auto word : words) {
    constant_uniform_fact.add_constant_word(word);
  }
  *constant_uniform_fact.mutable_uniform_buffer_element_descriptor() =
      descriptor;
  protobufs::Fact fact;
  *fact.mutable_constant_uniform_fact() = constant_uniform_fact;
  return fact_manager->AddFact(fact, context);
}

TEST(FactManagerTest, ConstantsAvailableViaUniforms) {
  std::string shader = R"(
               OpCapability Shader
               OpCapability Int64
               OpCapability Float64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource GLSL 450
               OpName %4 "main"
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0
               OpDecorate %200 DescriptorSet 0
               OpDecorate %200 Binding 1
               OpDecorate %300 DescriptorSet 0
               OpDecorate %300 Binding 2
               OpDecorate %400 DescriptorSet 0
               OpDecorate %400 Binding 3
               OpDecorate %500 DescriptorSet 0
               OpDecorate %500 Binding 4
               OpDecorate %600 DescriptorSet 0
               OpDecorate %600 Binding 5
               OpDecorate %700 DescriptorSet 0
               OpDecorate %700 Binding 6
               OpDecorate %800 DescriptorSet 1
               OpDecorate %800 Binding 0
               OpDecorate %900 DescriptorSet 1
               OpDecorate %900 Binding 1
               OpDecorate %1000 DescriptorSet 1
               OpDecorate %1000 Binding 2
               OpDecorate %1100 DescriptorSet 1
               OpDecorate %1100 Binding 3
               OpDecorate %1200 DescriptorSet 1
               OpDecorate %1200 Binding 4
               OpDecorate %1300 DescriptorSet 1
               OpDecorate %1300 Binding 5
               OpDecorate %1400 DescriptorSet 1
               OpDecorate %1400 Binding 6
               OpDecorate %1500 DescriptorSet 2
               OpDecorate %1500 Binding 0
               OpDecorate %1600 DescriptorSet 2
               OpDecorate %1600 Binding 1
               OpDecorate %1700 DescriptorSet 2
               OpDecorate %1700 Binding 2
               OpDecorate %1800 DescriptorSet 2
               OpDecorate %1800 Binding 3
               OpDecorate %1900 DescriptorSet 2
               OpDecorate %1900 Binding 4
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %10 = OpTypeInt 32 0
         %11 = OpTypeInt 32 1
         %12 = OpTypeInt 64 0
         %13 = OpTypeInt 64 1
         %15 = OpTypeFloat 32
         %16 = OpTypeFloat 64
         %17 = OpConstant %11 5
         %18 = OpConstant %11 20
         %19 = OpTypeVector %10 4
         %20 = OpConstant %11 6
         %21 = OpTypeVector %12 4
         %22 = OpConstant %11 10
         %23 = OpTypeVector %11 4

        %102 = OpTypeStruct %10 %10 %23
        %101 = OpTypePointer Uniform %102
        %100 = OpVariable %101 Uniform

        %203 = OpTypeArray %23 %17
        %202 = OpTypeArray %203 %18
        %201 = OpTypePointer Uniform %202
        %200 = OpVariable %201 Uniform

        %305 = OpTypeStruct %16 %16 %16 %11 %16
        %304 = OpTypeStruct %16 %16 %305
        %303 = OpTypeStruct %304
        %302 = OpTypeStruct %10 %303
        %301 = OpTypePointer Uniform %302
        %300 = OpVariable %301 Uniform

        %400 = OpVariable %101 Uniform

        %500 = OpVariable %201 Uniform

        %604 = OpTypeArray %13 %20
        %603 = OpTypeArray %604 %20
        %602 = OpTypeArray %603 %20
        %601 = OpTypePointer Uniform %602
        %600 = OpVariable %601 Uniform

        %703 = OpTypeArray %13 %20
        %702 = OpTypeArray %703 %20
        %701 = OpTypePointer Uniform %702
        %700 = OpVariable %701 Uniform

        %802 = OpTypeStruct %702 %602 %19 %202 %302
        %801 = OpTypePointer Uniform %802
        %800 = OpVariable %801 Uniform

        %902 = OpTypeStruct %702 %802 %19 %202 %302
        %901 = OpTypePointer Uniform %902
        %900 = OpVariable %901 Uniform

       %1003 = OpTypeStruct %802
       %1002 = OpTypeArray %1003 %20
       %1001 = OpTypePointer Uniform %1002
       %1000 = OpVariable %1001 Uniform

       %1101 = OpTypePointer Uniform %21
       %1100 = OpVariable %1101 Uniform

       %1202 = OpTypeArray %21 %20
       %1201 = OpTypePointer Uniform %1202
       %1200 = OpVariable %1201 Uniform

       %1302 = OpTypeArray %21 %20
       %1301 = OpTypePointer Uniform %1302
       %1300 = OpVariable %1301 Uniform

       %1402 = OpTypeArray %15 %22
       %1401 = OpTypePointer Uniform %1402
       %1400 = OpVariable %1401 Uniform

       %1501 = OpTypePointer Uniform %1402
       %1500 = OpVariable %1501 Uniform

       %1602 = OpTypeArray %1402 %22
       %1601 = OpTypePointer Uniform %1602
       %1600 = OpVariable %1601 Uniform

       %1704 = OpTypeStruct %16 %16 %16
       %1703 = OpTypeArray %1704 %22
       %1702 = OpTypeArray %1703 %22
       %1701 = OpTypePointer Uniform %1702
       %1700 = OpVariable %1701 Uniform

       %1800 = OpVariable %1701 Uniform

       %1906 = OpTypeStruct %16
       %1905 = OpTypeStruct %1906
       %1904 = OpTypeStruct %1905
       %1903 = OpTypeStruct %1904
       %1902 = OpTypeStruct %1903
       %1901 = OpTypePointer Uniform %1902
       %1900 = OpVariable %1901 Uniform

          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  uint32_t buffer_int32_min[1];
  uint32_t buffer_int64_1[2];
  uint32_t buffer_int64_max[2];
  uint32_t buffer_uint64_1[2];
  uint32_t buffer_uint64_max[2];
  uint32_t buffer_float_10[1];
  uint32_t buffer_double_10[2];
  uint32_t buffer_double_20[2];

  {
    int32_t temp = std::numeric_limits<int32_t>::min();
    std::memcpy(&buffer_int32_min, &temp, sizeof(temp));
  }

  {
    int64_t temp = 1;
    std::memcpy(&buffer_int64_1, &temp, sizeof(temp));
  }

  {
    int64_t temp = std::numeric_limits<int64_t>::max();
    std::memcpy(&buffer_int64_max, &temp, sizeof(temp));
  }

  {
    uint64_t temp = 1;
    std::memcpy(&buffer_uint64_1, &temp, sizeof(temp));
  }

  {
    uint64_t temp = std::numeric_limits<uint64_t>::max();
    std::memcpy(&buffer_uint64_max, &temp, sizeof(temp));
  }

  {
    float temp = 10.0f;
    std::memcpy(&buffer_float_10, &temp, sizeof(float));
  }

  {
    double temp = 10.0;
    std::memcpy(&buffer_double_10, &temp, sizeof(temp));
  }

  {
    double temp = 20.0;
    std::memcpy(&buffer_double_20, &temp, sizeof(temp));
  }

  FactManager fact_manager;

  uint32_t type_int32_id = 11;
  uint32_t type_int64_id = 13;
  uint32_t type_uint32_id = 10;
  uint32_t type_uint64_id = 12;
  uint32_t type_float_id = 15;
  uint32_t type_double_id = 16;

  // Initially there should be no facts about uniforms.
  ASSERT_TRUE(fact_manager
                  .GetConstantsAvailableFromUniformsForType(context.get(),
                                                            type_uint32_id)
                  .empty());

  // In the comments that follow we write v[...][...] to refer to uniform
  // variable v indexed with some given indices, when in practice v is
  // identified via a (descriptor set, binding) pair.

  // 100[2][3] == int(1)
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(), {1},
                            MakeUniformBufferElementDescriptor(0, 0, {2, 3})));

  // 200[1][2][3] == int(1)
  ASSERT_TRUE(
      AddFactHelper(&fact_manager, context.get(), {1},
                    MakeUniformBufferElementDescriptor(0, 1, {1, 2, 3})));

  // 300[1][0][2][3] == int(1)
  ASSERT_TRUE(
      AddFactHelper(&fact_manager, context.get(), {1},
                    MakeUniformBufferElementDescriptor(0, 2, {1, 0, 2, 3})));

  // 400[2][3] = int32_min
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(), {buffer_int32_min[0]},
                            MakeUniformBufferElementDescriptor(0, 3, {2, 3})));

  // 500[1][2][3] = int32_min
  ASSERT_TRUE(
      AddFactHelper(&fact_manager, context.get(), {buffer_int32_min[0]},
                    MakeUniformBufferElementDescriptor(0, 4, {1, 2, 3})));

  // 600[1][2][3] = int64_max
  ASSERT_TRUE(AddFactHelper(
      &fact_manager, context.get(), {buffer_int64_max[0], buffer_int64_max[1]},
      MakeUniformBufferElementDescriptor(0, 5, {1, 2, 3})));

  // 700[1][1] = int64_max
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(),
                            {buffer_int64_max[0], buffer_int64_max[1]},
                            MakeUniformBufferElementDescriptor(0, 6, {1, 1})));

  // 800[2][3] = uint(1)
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(), {1},
                            MakeUniformBufferElementDescriptor(1, 0, {2, 3})));

  // 900[1][2][3] = uint(1)
  ASSERT_TRUE(
      AddFactHelper(&fact_manager, context.get(), {1},
                    MakeUniformBufferElementDescriptor(1, 1, {1, 2, 3})));

  // 1000[1][0][2][3] = uint(1)
  ASSERT_TRUE(
      AddFactHelper(&fact_manager, context.get(), {1},
                    MakeUniformBufferElementDescriptor(1, 2, {1, 0, 2, 3})));

  // 1100[0] = uint64(1)
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(),
                            {buffer_uint64_1[0], buffer_uint64_1[1]},
                            MakeUniformBufferElementDescriptor(1, 3, {0})));

  // 1200[0][0] = uint64_max
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(),
                            {buffer_uint64_max[0], buffer_uint64_max[1]},
                            MakeUniformBufferElementDescriptor(1, 4, {0, 0})));

  // 1300[1][0] = uint64_max
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(),
                            {buffer_uint64_max[0], buffer_uint64_max[1]},
                            MakeUniformBufferElementDescriptor(1, 5, {1, 0})));

  // 1400[6] = float(10.0)
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(), {buffer_float_10[0]},
                            MakeUniformBufferElementDescriptor(1, 6, {6})));

  // 1500[7] = float(10.0)
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(), {buffer_float_10[0]},
                            MakeUniformBufferElementDescriptor(2, 0, {7})));

  // 1600[9][9] = float(10.0)
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(), {buffer_float_10[0]},
                            MakeUniformBufferElementDescriptor(2, 1, {9, 9})));

  // 1700[9][9][1] = double(10.0)
  ASSERT_TRUE(AddFactHelper(
      &fact_manager, context.get(), {buffer_double_10[0], buffer_double_10[1]},
      MakeUniformBufferElementDescriptor(2, 2, {9, 9, 1})));

  // 1800[9][9][2] = double(10.0)
  ASSERT_TRUE(AddFactHelper(
      &fact_manager, context.get(), {buffer_double_10[0], buffer_double_10[1]},
      MakeUniformBufferElementDescriptor(2, 3, {9, 9, 2})));

  // 1900[0][0][0][0][0] = double(20.0)
  ASSERT_TRUE(AddFactHelper(
      &fact_manager, context.get(), {buffer_double_20[0], buffer_double_20[1]},
      MakeUniformBufferElementDescriptor(2, 4, {0, 0, 0, 0, 0})));

  opt::Instruction::OperandList operands = {
      {SPV_OPERAND_TYPE_LITERAL_INTEGER, {1}}};
  context->module()->AddGlobalValue(MakeUnique<opt::Instruction>(
      context.get(), SpvOpConstant, type_int32_id, 50, operands));
  operands = {{SPV_OPERAND_TYPE_LITERAL_INTEGER, {buffer_int32_min[0]}}};
  context->module()->AddGlobalValue(MakeUnique<opt::Instruction>(
      context.get(), SpvOpConstant, type_int32_id, 51, operands));
  operands = {{SPV_OPERAND_TYPE_LITERAL_INTEGER, {buffer_int64_max[0]}},
              {SPV_OPERAND_TYPE_LITERAL_INTEGER, {buffer_int64_max[1]}}};
  context->module()->AddGlobalValue(MakeUnique<opt::Instruction>(
      context.get(), SpvOpConstant, type_int64_id, 52, operands));
  operands = {{SPV_OPERAND_TYPE_LITERAL_INTEGER, {1}}};
  context->module()->AddGlobalValue(MakeUnique<opt::Instruction>(
      context.get(), SpvOpConstant, type_uint32_id, 53, operands));
  operands = {{SPV_OPERAND_TYPE_LITERAL_INTEGER, {buffer_uint64_1[0]}},
              {SPV_OPERAND_TYPE_LITERAL_INTEGER, {buffer_uint64_1[1]}}};
  context->module()->AddGlobalValue(MakeUnique<opt::Instruction>(
      context.get(), SpvOpConstant, type_uint64_id, 54, operands));
  operands = {{SPV_OPERAND_TYPE_LITERAL_INTEGER, {buffer_uint64_max[0]}},
              {SPV_OPERAND_TYPE_LITERAL_INTEGER, {buffer_uint64_max[1]}}};
  context->module()->AddGlobalValue(MakeUnique<opt::Instruction>(
      context.get(), SpvOpConstant, type_uint64_id, 55, operands));
  operands = {{SPV_OPERAND_TYPE_LITERAL_INTEGER, {buffer_float_10[0]}}};
  context->module()->AddGlobalValue(MakeUnique<opt::Instruction>(
      context.get(), SpvOpConstant, type_float_id, 56, operands));
  operands = {{SPV_OPERAND_TYPE_LITERAL_INTEGER, {buffer_double_10[0]}},
              {SPV_OPERAND_TYPE_LITERAL_INTEGER, {buffer_double_10[1]}}};
  context->module()->AddGlobalValue(MakeUnique<opt::Instruction>(
      context.get(), SpvOpConstant, type_double_id, 57, operands));
  operands = {{SPV_OPERAND_TYPE_LITERAL_INTEGER, {buffer_double_20[0]}},
              {SPV_OPERAND_TYPE_LITERAL_INTEGER, {buffer_double_20[1]}}};
  context->module()->AddGlobalValue(MakeUnique<opt::Instruction>(
      context.get(), SpvOpConstant, type_double_id, 58, operands));

  // A duplicate of the constant with id 59.
  operands = {{SPV_OPERAND_TYPE_LITERAL_INTEGER, {1}}};
  context->module()->AddGlobalValue(MakeUnique<opt::Instruction>(
      context.get(), SpvOpConstant, type_int32_id, 59, operands));

  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);

  // Constants 1 and int32_min are available.
  ASSERT_EQ(2, fact_manager
                   .GetConstantsAvailableFromUniformsForType(context.get(),
                                                             type_int32_id)
                   .size());
  // Constant int64_max is available.
  ASSERT_EQ(1, fact_manager
                   .GetConstantsAvailableFromUniformsForType(context.get(),
                                                             type_int64_id)
                   .size());
  // Constant 1u is available.
  ASSERT_EQ(1, fact_manager
                   .GetConstantsAvailableFromUniformsForType(context.get(),
                                                             type_uint32_id)
                   .size());
  // Constants 1u and uint64_max are available.
  ASSERT_EQ(2, fact_manager
                   .GetConstantsAvailableFromUniformsForType(context.get(),
                                                             type_uint64_id)
                   .size());
  // Constant 10.0 is available.
  ASSERT_EQ(1, fact_manager
                   .GetConstantsAvailableFromUniformsForType(context.get(),
                                                             type_float_id)
                   .size());
  // Constants 10.0 and 20.0 are available.
  ASSERT_EQ(2, fact_manager
                   .GetConstantsAvailableFromUniformsForType(context.get(),
                                                             type_double_id)
                   .size());

  ASSERT_EQ(std::numeric_limits<int64_t>::max(),
            context->get_constant_mgr()
                ->FindDeclaredConstant(
                    fact_manager.GetConstantsAvailableFromUniformsForType(
                        context.get(), type_int64_id)[0])
                ->AsIntConstant()
                ->GetS64());
  ASSERT_EQ(1, context->get_constant_mgr()
                   ->FindDeclaredConstant(
                       fact_manager.GetConstantsAvailableFromUniformsForType(
                           context.get(), type_uint32_id)[0])
                   ->AsIntConstant()
                   ->GetU32());
  ASSERT_EQ(10.0f,
            context->get_constant_mgr()
                ->FindDeclaredConstant(
                    fact_manager.GetConstantsAvailableFromUniformsForType(
                        context.get(), type_float_id)[0])
                ->AsFloatConstant()
                ->GetFloat());
  const std::vector<uint32_t>& double_constant_ids =
      fact_manager.GetConstantsAvailableFromUniformsForType(context.get(),
                                                            type_double_id);
  ASSERT_EQ(10.0, context->get_constant_mgr()
                      ->FindDeclaredConstant(double_constant_ids[0])
                      ->AsFloatConstant()
                      ->GetDouble());
  ASSERT_EQ(20.0, context->get_constant_mgr()
                      ->FindDeclaredConstant(double_constant_ids[1])
                      ->AsFloatConstant()
                      ->GetDouble());

  const std::vector<protobufs::UniformBufferElementDescriptor>
      descriptors_for_double_10 = fact_manager.GetUniformDescriptorsForConstant(
          context.get(), double_constant_ids[0]);
  ASSERT_EQ(2, descriptors_for_double_10.size());
  {
    auto temp = MakeUniformBufferElementDescriptor(2, 2, {9, 9, 1});
    ASSERT_TRUE(UniformBufferElementDescriptorEquals()(
        &temp, &descriptors_for_double_10[0]));
  }
  {
    auto temp = MakeUniformBufferElementDescriptor(2, 3, {9, 9, 2});
    ASSERT_TRUE(UniformBufferElementDescriptorEquals()(
        &temp, &descriptors_for_double_10[1]));
  }
  const std::vector<protobufs::UniformBufferElementDescriptor>
      descriptors_for_double_20 = fact_manager.GetUniformDescriptorsForConstant(
          context.get(), double_constant_ids[1]);
  ASSERT_EQ(1, descriptors_for_double_20.size());
  {
    auto temp = MakeUniformBufferElementDescriptor(2, 4, {0, 0, 0, 0, 0});
    ASSERT_TRUE(UniformBufferElementDescriptorEquals()(
        &temp, &descriptors_for_double_20[0]));
  }

  auto constant_1_id = fact_manager.GetConstantFromUniformDescriptor(
      context.get(), MakeUniformBufferElementDescriptor(2, 3, {9, 9, 2}));
  ASSERT_TRUE(constant_1_id);

  auto constant_2_id = fact_manager.GetConstantFromUniformDescriptor(
      context.get(), MakeUniformBufferElementDescriptor(2, 4, {0, 0, 0, 0, 0}));
  ASSERT_TRUE(constant_2_id);

  ASSERT_EQ(double_constant_ids[0], constant_1_id);

  ASSERT_EQ(double_constant_ids[1], constant_2_id);
}

TEST(FactManagerTest, TwoConstantsWithSameValue) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %10 "buf"
               OpMemberName %10 0 "a"
               OpName %12 ""
               OpDecorate %8 RelaxedPrecision
               OpMemberDecorate %10 0 RelaxedPrecision
               OpMemberDecorate %10 0 Offset 0
               OpDecorate %10 Block
               OpDecorate %12 DescriptorSet 0
               OpDecorate %12 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %20 = OpConstant %6 1
         %10 = OpTypeStruct %6
         %11 = OpTypePointer Uniform %10
         %12 = OpVariable %11 Uniform
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpStore %8 %9
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  auto uniform_buffer_element_descriptor =
      MakeUniformBufferElementDescriptor(0, 0, {0});

  // (0, 0, [0]) = int(1)
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(), {1},
                            uniform_buffer_element_descriptor));
  auto constants =
      fact_manager.GetConstantsAvailableFromUniformsForType(context.get(), 6);
  ASSERT_EQ(1, constants.size());
  ASSERT_TRUE(constants[0] == 9 || constants[0] == 20);

  auto constant = fact_manager.GetConstantFromUniformDescriptor(
      context.get(), uniform_buffer_element_descriptor);
  ASSERT_TRUE(constant == 9 || constant == 20);

  // Because the constants with ids 9 and 20 are equal, we should get the same
  // single uniform buffer element descriptor when we look up the descriptors
  // for either one of them.
  for (auto constant_id : {9u, 20u}) {
    auto descriptors = fact_manager.GetUniformDescriptorsForConstant(
        context.get(), constant_id);
    ASSERT_EQ(1, descriptors.size());
    ASSERT_TRUE(UniformBufferElementDescriptorEquals()(
        &uniform_buffer_element_descriptor, &descriptors[0]));
  }
}

TEST(FactManagerTest, NonFiniteFactsAreNotValid) {
  std::string shader = R"(
               OpCapability Shader
               OpCapability Float64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %7 "buf"
               OpMemberName %7 0 "f"
               OpMemberName %7 1 "d"
               OpName %9 ""
               OpMemberDecorate %7 0 Offset 0
               OpMemberDecorate %7 1 Offset 8
               OpDecorate %7 Block
               OpDecorate %9 DescriptorSet 0
               OpDecorate %9 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
         %10 = OpTypeFloat 64
          %7 = OpTypeStruct %6 %10
          %8 = OpTypePointer Uniform %7
          %9 = OpVariable %8 Uniform
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
  auto uniform_buffer_element_descriptor_f =
      MakeUniformBufferElementDescriptor(0, 0, {0});

  auto uniform_buffer_element_descriptor_d =
      MakeUniformBufferElementDescriptor(0, 0, {1});

  if (std::numeric_limits<float>::has_infinity) {
    // f == +inf
    float positive_infinity_float = std::numeric_limits<float>::infinity();
    uint32_t words[1];
    memcpy(words, &positive_infinity_float, sizeof(float));
    ASSERT_FALSE(AddFactHelper(&fact_manager, context.get(), {words[0]},
                               uniform_buffer_element_descriptor_f));
    // f == -inf
    float negative_infinity_float = std::numeric_limits<float>::infinity();
    memcpy(words, &negative_infinity_float, sizeof(float));
    ASSERT_FALSE(AddFactHelper(&fact_manager, context.get(), {words[0]},
                               uniform_buffer_element_descriptor_f));
  }

  if (std::numeric_limits<float>::has_quiet_NaN) {
    // f == NaN
    float quiet_nan_float = std::numeric_limits<float>::quiet_NaN();
    uint32_t words[1];
    memcpy(words, &quiet_nan_float, sizeof(float));
    ASSERT_FALSE(AddFactHelper(&fact_manager, context.get(), {words[0]},
                               uniform_buffer_element_descriptor_f));
  }

  if (std::numeric_limits<double>::has_infinity) {
    // d == +inf
    double positive_infinity_double = std::numeric_limits<double>::infinity();
    uint32_t words[2];
    memcpy(words, &positive_infinity_double, sizeof(double));
    ASSERT_FALSE(AddFactHelper(&fact_manager, context.get(),
                               {words[0], words[1]},
                               uniform_buffer_element_descriptor_d));
    // d == -inf
    double negative_infinity_double = -std::numeric_limits<double>::infinity();
    memcpy(words, &negative_infinity_double, sizeof(double));
    ASSERT_FALSE(AddFactHelper(&fact_manager, context.get(),
                               {words[0], words[1]},
                               uniform_buffer_element_descriptor_d));
  }

  if (std::numeric_limits<double>::has_quiet_NaN) {
    // d == NaN
    double quiet_nan_double = std::numeric_limits<double>::quiet_NaN();
    uint32_t words[2];
    memcpy(words, &quiet_nan_double, sizeof(double));
    ASSERT_FALSE(AddFactHelper(&fact_manager, context.get(),
                               {words[0], words[1]},
                               uniform_buffer_element_descriptor_d));
  }
}

TEST(FactManagerTest, AmbiguousFact) {
  //  This test came from the following GLSL:
  //
  // #version 310 es
  //
  // precision highp float;
  //
  // layout(set = 0, binding = 0) uniform buf {
  //   float f;
  // };
  //
  // layout(set = 0, binding = 0) uniform buf2 {
  //   float g;
  // };
  //
  // void main() {
  //
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %7 "buf"
               OpMemberName %7 0 "f"
               OpName %9 ""
               OpName %10 "buf2"
               OpMemberName %10 0 "g"
               OpName %12 ""
               OpMemberDecorate %7 0 Offset 0
               OpDecorate %7 Block
               OpDecorate %9 DescriptorSet 0
               OpDecorate %9 Binding 0
               OpMemberDecorate %10 0 Offset 0
               OpDecorate %10 Block
               OpDecorate %12 DescriptorSet 0
               OpDecorate %12 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeStruct %6
          %8 = OpTypePointer Uniform %7
          %9 = OpVariable %8 Uniform
         %10 = OpTypeStruct %6
         %11 = OpTypePointer Uniform %10
         %12 = OpVariable %11 Uniform
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
  auto uniform_buffer_element_descriptor =
      MakeUniformBufferElementDescriptor(0, 0, {0});

  // The fact cannot be added because it is ambiguous: there are two uniforms
  // with descriptor set 0 and binding 0.
  ASSERT_FALSE(AddFactHelper(&fact_manager, context.get(), {1},
                             uniform_buffer_element_descriptor));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
