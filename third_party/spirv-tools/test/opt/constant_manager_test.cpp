// Copyright (c) 2018 Google LLC
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

#include <memory>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "source/opt/build_module.h"
#include "source/opt/constants.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace opt {
namespace analysis {
namespace {

using ConstantManagerTest = ::testing::Test;

TEST_F(ConstantManagerTest, GetDefiningInstruction) {
  const std::string text = R"(
%int = OpTypeInt 32 0
%1 = OpTypeStruct %int
%2 = OpTypeStruct %int
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(context, nullptr);

  Type* struct_type_1 = context->get_type_mgr()->GetType(1);
  StructConstant struct_const_1(struct_type_1->AsStruct());
  Instruction* const_inst_1 =
      context->get_constant_mgr()->GetDefiningInstruction(&struct_const_1, 1);
  EXPECT_EQ(const_inst_1->type_id(), 1);

  Type* struct_type_2 = context->get_type_mgr()->GetType(2);
  StructConstant struct_const_2(struct_type_2->AsStruct());
  Instruction* const_inst_2 =
      context->get_constant_mgr()->GetDefiningInstruction(&struct_const_2, 2);
  EXPECT_EQ(const_inst_2->type_id(), 2);
}

TEST_F(ConstantManagerTest, GetDefiningInstruction2) {
  const std::string text = R"(
%int = OpTypeInt 32 0
%1 = OpTypeStruct %int
%2 = OpTypeStruct %int
%3 = OpConstantNull %1
%4 = OpConstantNull %2
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(context, nullptr);

  Type* struct_type_1 = context->get_type_mgr()->GetType(1);
  NullConstant struct_const_1(struct_type_1->AsStruct());
  Instruction* const_inst_1 =
      context->get_constant_mgr()->GetDefiningInstruction(&struct_const_1, 1);
  EXPECT_EQ(const_inst_1->type_id(), 1);
  EXPECT_EQ(const_inst_1->result_id(), 3);

  Type* struct_type_2 = context->get_type_mgr()->GetType(2);
  NullConstant struct_const_2(struct_type_2->AsStruct());
  Instruction* const_inst_2 =
      context->get_constant_mgr()->GetDefiningInstruction(&struct_const_2, 2);
  EXPECT_EQ(const_inst_2->type_id(), 2);
  EXPECT_EQ(const_inst_2->result_id(), 4);
}

TEST_F(ConstantManagerTest, GetDefiningInstructionIdOverflow) {
  const std::string text = R"(
%1 = OpTypeInt 32 0
%3 = OpConstant %1 1
%4 = OpConstant %1 2
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(context, nullptr);

  // Set the id bound to the max, so the new constant cannot be generated.
  context->module()->SetIdBound(context->max_id_bound());

  Type* int_type = context->get_type_mgr()->GetType(1);
  IntConstant int_constant(int_type->AsInteger(), {3});
  Instruction* inst =
      context->get_constant_mgr()->GetDefiningInstruction(&int_constant, 1);
  EXPECT_EQ(inst, nullptr);
}

TEST_F(ConstantManagerTest, ConstantCompositeReplicateExtMapping) {
  const std::string text = R"(
OpCapability Shader
OpCapability ReplicatedCompositesEXT
OpExtension "SPV_EXT_replicated_composites"
OpMemoryModel Logical Simple
%1 = OpTypeInt 32 1
%2 = OpTypeVector %1 4
%3 = OpConstant %1 0
%4 = OpConstantCompositeReplicateEXT %2 %3
%5 = OpSpecConstantCompositeReplicateEXT %2 %3
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_5, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(context, nullptr);

  ConstantManager* const_mgr = context->get_constant_mgr();
  const Constant* base_constant = const_mgr->FindDeclaredConstant(3);
  ASSERT_NE(base_constant, nullptr);

  const Constant* composite_constant = const_mgr->FindDeclaredConstant(4);
  ASSERT_NE(composite_constant, nullptr);
  const Vector* vector_type = composite_constant->type()->AsVector();
  ASSERT_NE(vector_type, nullptr);

  const CompositeConstant* composite =
      composite_constant->AsCompositeConstant();
  ASSERT_NE(composite, nullptr);
  ASSERT_EQ(composite->GetComponents().size(),
            static_cast<size_t>(vector_type->element_count()));
  ASSERT_FALSE(composite->GetComponents().empty());
  for (const Constant* component : composite->GetComponents()) {
    EXPECT_EQ(component, base_constant);
  }

  const Constant* spec_composite_constant = const_mgr->FindDeclaredConstant(5);
  ASSERT_NE(spec_composite_constant, nullptr);
  const Vector* spec_vector_type = spec_composite_constant->type()->AsVector();
  ASSERT_NE(spec_vector_type, nullptr);

  const CompositeConstant* spec_composite =
      spec_composite_constant->AsCompositeConstant();
  ASSERT_NE(spec_composite, nullptr);
  ASSERT_EQ(spec_composite->GetComponents().size(),
            static_cast<size_t>(spec_vector_type->element_count()));
  ASSERT_FALSE(spec_composite->GetComponents().empty());
  for (const Constant* component : spec_composite->GetComponents()) {
    EXPECT_EQ(component, base_constant);
  }
}

TEST_F(ConstantManagerTest, NullTensor) {
  const std::string text = R"(
%uint = OpTypeInt 32 0
%1 = OpConstant %uint 1
%2 = OpConstant %uint 2
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%arr_uint_1 = OpTypeArray %uint %1
%arr_uint_2 = OpTypeArray %uint %2
%10 = OpConstantComposite %arr_uint_1 %uint_7
%11 = OpConstantComposite %arr_uint_2 %uint_5 %uint_7
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(context, nullptr);
  const auto cstmgr = context->get_constant_mgr();

  Integer ty_uint(32, 0);

  // rank-1 with 7 elements
  TensorARM ty_tensor_r1_7(&ty_uint, 1, 10);
  auto null_tensor_r1_7 =
      cstmgr->GetNullCompositeConstant(&ty_tensor_r1_7)->AsTensorConstant();
  ASSERT_NE(null_tensor_r1_7, nullptr);
  ASSERT_EQ(null_tensor_r1_7->GetComponents().size(), 7);
  ASSERT_TRUE(null_tensor_r1_7->IsZero());

  // rank-2 with 5 elements of rank-1 type with 7 elements
  TensorARM ty_tensor_r2_5_7(&ty_uint, 2, 11);
  auto null_tensor_r2_5_7 =
      cstmgr->GetNullCompositeConstant(&ty_tensor_r2_5_7)->AsTensorConstant();
  ASSERT_NE(null_tensor_r2_5_7, nullptr);
  ASSERT_EQ(null_tensor_r2_5_7->GetComponents().size(), 5);
  ASSERT_TRUE(null_tensor_r2_5_7->IsZero());
  ASSERT_NE(null_tensor_r2_5_7->GetComponents()[0]->AsTensorConstant(),
            nullptr);
  ASSERT_TRUE(null_tensor_r2_5_7->GetComponents()[0]->type()->IsSameImpl(
      &ty_tensor_r1_7, nullptr));
}

TEST_F(ConstantManagerTest, TensorConstantFromInstruction) {
  const std::string text = R"(
OpCapability TensorsARM
OpExtension "SPV_ARM_tensors"
%1 = OpTypeInt 32 0
%2 = OpConstant %1 1
%3 = OpConstant %1 2
%4 = OpConstant %1 3
%5 = OpTypeArray %1 %2
%6 = OpConstantComposite %5 %4
%7 = OpTypeTensorARM %1 %2 %6
%8 = OpConstantComposite %7 %2 %3 %4
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(context, nullptr);

  const Constant* constant =
      context->get_constant_mgr()->FindDeclaredConstant(8);
  ASSERT_NE(constant, nullptr);
  const auto* tensor_const = constant->AsTensorConstant();
  ASSERT_NE(tensor_const, nullptr);
  ASSERT_EQ(tensor_const->GetComponents().size(), 3);
  EXPECT_EQ(tensor_const->GetComponents()[0]->GetZeroExtendedValue(), 1u);
  EXPECT_EQ(tensor_const->GetComponents()[1]->GetZeroExtendedValue(), 2u);
  EXPECT_EQ(tensor_const->GetComponents()[2]->GetZeroExtendedValue(), 3u);
  EXPECT_FALSE(tensor_const->IsZero());
}

struct TensorConstantCompositeReplicateExtCase {
  std::string opcode;
  std::string name;
};

class TensorConstantCompositeReplicateExtTest
    : public ::testing::TestWithParam<TensorConstantCompositeReplicateExtCase> {
};

std::string TensorConstantCompositeReplicateExtCaseName(
    const ::testing::TestParamInfo<TensorConstantCompositeReplicateExtCase>&
        info) {
  return info.param.name;
}

TEST_P(TensorConstantCompositeReplicateExtTest, FromInstruction) {
  const std::string text = R"(
OpCapability TensorsARM
OpCapability ReplicatedCompositesEXT
OpExtension "SPV_ARM_tensors"
OpExtension "SPV_EXT_replicated_composites"
%1 = OpTypeInt 32 0
%2 = OpConstant %1 1
%3 = OpConstant %1 2
%4 = OpTypeArray %1 %2
%5 = OpConstantComposite %4 %3
%6 = OpTypeTensorARM %1 %2 %5
%7 = )" + GetParam().opcode +
                           R"( %6 %3
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_5, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(context, nullptr);

  const Constant* base_constant =
      context->get_constant_mgr()->FindDeclaredConstant(3);
  ASSERT_NE(base_constant, nullptr);

  const Constant* constant =
      context->get_constant_mgr()->FindDeclaredConstant(7);
  ASSERT_NE(constant, nullptr);
  const auto* tensor_const = constant->AsTensorConstant();
  ASSERT_NE(tensor_const, nullptr);
  ASSERT_EQ(tensor_const->GetComponents().size(), 2u);
  EXPECT_EQ(tensor_const->GetComponents()[0], base_constant);
  EXPECT_EQ(tensor_const->GetComponents()[1], base_constant);
  EXPECT_FALSE(tensor_const->IsZero());
}

TEST_P(TensorConstantCompositeReplicateExtTest, Rank2FromInstruction) {
  const std::string text = R"(
OpCapability TensorsARM
OpCapability ReplicatedCompositesEXT
OpExtension "SPV_ARM_tensors"
OpExtension "SPV_EXT_replicated_composites"
%1 = OpTypeInt 32 0
%2 = OpConstant %1 1
%3 = OpConstant %1 2
%4 = OpConstant %1 3
%5 = OpTypeArray %1 %2
%6 = OpTypeArray %1 %3
%7 = OpConstantComposite %5 %4
%8 = OpConstantComposite %6 %3 %4
%9 = OpTypeTensorARM %1 %2 %7
%10 = OpTypeTensorARM %1 %3 %8
%11 = OpConstantComposite %9 %2 %3 %4
%12 = )" + GetParam().opcode +
                           R"( %10 %11
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_5, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(context, nullptr);

  const Constant* row_constant =
      context->get_constant_mgr()->FindDeclaredConstant(11);
  ASSERT_NE(row_constant, nullptr);
  const auto* row_tensor = row_constant->AsTensorConstant();
  ASSERT_NE(row_tensor, nullptr);
  ASSERT_EQ(row_tensor->GetComponents().size(), 3u);
  EXPECT_EQ(row_tensor->GetComponents()[0]->GetZeroExtendedValue(), 1u);
  EXPECT_EQ(row_tensor->GetComponents()[1]->GetZeroExtendedValue(), 2u);
  EXPECT_EQ(row_tensor->GetComponents()[2]->GetZeroExtendedValue(), 3u);

  const Constant* constant =
      context->get_constant_mgr()->FindDeclaredConstant(12);
  ASSERT_NE(constant, nullptr);
  const auto* tensor_const = constant->AsTensorConstant();
  ASSERT_NE(tensor_const, nullptr);
  ASSERT_EQ(tensor_const->GetComponents().size(), 2u);
  EXPECT_EQ(tensor_const->GetComponents()[0], row_constant);
  EXPECT_EQ(tensor_const->GetComponents()[1], row_constant);
  EXPECT_FALSE(tensor_const->IsZero());
}

TEST_P(TensorConstantCompositeReplicateExtTest,
       WithReplicatedShapeFromInstruction) {
  const std::string text = R"(
OpCapability TensorsARM
OpCapability ReplicatedCompositesEXT
OpExtension "SPV_ARM_tensors"
OpExtension "SPV_EXT_replicated_composites"
%1 = OpTypeInt 32 0
%2 = OpConstant %1 1
%3 = OpConstant %1 2
%4 = OpConstant %1 7
%5 = OpTypeArray %1 %2
%6 = OpTypeArray %1 %3
%7 = OpConstantCompositeReplicateEXT %5 %3
%8 = OpConstantCompositeReplicateEXT %6 %3
%9 = OpTypeTensorARM %1 %2 %7
%10 = OpTypeTensorARM %1 %3 %8
%11 = OpConstantCompositeReplicateEXT %9 %4
%12 = )" + GetParam().opcode +
                           R"( %10 %11
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_5, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(context, nullptr);

  const Constant* shape_constant =
      context->get_constant_mgr()->FindDeclaredConstant(8);
  ASSERT_NE(shape_constant, nullptr);
  const auto* shape_array = shape_constant->AsArrayConstant();
  ASSERT_NE(shape_array, nullptr);
  ASSERT_EQ(shape_array->GetComponents().size(), 2u);
  EXPECT_EQ(shape_array->GetComponents()[0]->GetZeroExtendedValue(), 2u);
  EXPECT_EQ(shape_array->GetComponents()[1]->GetZeroExtendedValue(), 2u);

  const Constant* row_constant =
      context->get_constant_mgr()->FindDeclaredConstant(11);
  ASSERT_NE(row_constant, nullptr);
  const auto* row_tensor = row_constant->AsTensorConstant();
  ASSERT_NE(row_tensor, nullptr);
  ASSERT_EQ(row_tensor->GetComponents().size(), 2u);
  EXPECT_EQ(row_tensor->GetComponents()[0]->GetZeroExtendedValue(), 7u);
  EXPECT_EQ(row_tensor->GetComponents()[1]->GetZeroExtendedValue(), 7u);

  const Constant* constant =
      context->get_constant_mgr()->FindDeclaredConstant(12);
  ASSERT_NE(constant, nullptr);
  const auto* tensor_const = constant->AsTensorConstant();
  ASSERT_NE(tensor_const, nullptr);
  ASSERT_EQ(tensor_const->GetComponents().size(), 2u);
  EXPECT_EQ(tensor_const->GetComponents()[0], row_constant);
  EXPECT_EQ(tensor_const->GetComponents()[1], row_constant);
  EXPECT_FALSE(tensor_const->IsZero());
}

INSTANTIATE_TEST_SUITE_P(
    ConstantAndSpecConstant, TensorConstantCompositeReplicateExtTest,
    ::testing::ValuesIn(std::vector<TensorConstantCompositeReplicateExtCase>{
        {"OpConstantCompositeReplicateEXT", "Constant"},
        {"OpSpecConstantCompositeReplicateEXT", "SpecConstant"},
    }),
    TensorConstantCompositeReplicateExtCaseName);

TEST_F(ConstantManagerTest, NestedTensorConstantFromInstruction) {
  const std::string text = R"(
OpCapability TensorsARM
OpExtension "SPV_ARM_tensors"
%1 = OpTypeInt 32 0
%2 = OpConstant %1 1
%3 = OpConstant %1 2
%4 = OpConstant %1 3
%5 = OpConstant %1 4
%6 = OpConstant %1 5
%7 = OpConstant %1 6
%8 = OpTypeArray %1 %2
%9 = OpTypeArray %1 %3
%10 = OpConstantComposite %8 %4
%11 = OpConstantComposite %9 %3 %4
%12 = OpTypeTensorARM %1 %2 %10
%13 = OpTypeTensorARM %1 %3 %11
%14 = OpConstantComposite %12 %2 %3 %4
%15 = OpConstantComposite %12 %5 %6 %7
%16 = OpConstantComposite %13 %14 %15
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(context, nullptr);

  const Constant* constant =
      context->get_constant_mgr()->FindDeclaredConstant(16);
  ASSERT_NE(constant, nullptr);
  const auto* tensor_const = constant->AsTensorConstant();
  ASSERT_NE(tensor_const, nullptr);
  ASSERT_EQ(tensor_const->GetComponents().size(), 2);

  const auto* row_0 = tensor_const->GetComponents()[0]->AsTensorConstant();
  const auto* row_1 = tensor_const->GetComponents()[1]->AsTensorConstant();
  ASSERT_NE(row_0, nullptr);
  ASSERT_NE(row_1, nullptr);
  ASSERT_EQ(row_0->GetComponents().size(), 3);
  ASSERT_EQ(row_1->GetComponents().size(), 3);
  EXPECT_EQ(row_0->GetComponents()[0]->GetZeroExtendedValue(), 1u);
  EXPECT_EQ(row_0->GetComponents()[1]->GetZeroExtendedValue(), 2u);
  EXPECT_EQ(row_0->GetComponents()[2]->GetZeroExtendedValue(), 3u);
  EXPECT_EQ(row_1->GetComponents()[0]->GetZeroExtendedValue(), 4u);
  EXPECT_EQ(row_1->GetComponents()[1]->GetZeroExtendedValue(), 5u);
  EXPECT_EQ(row_1->GetComponents()[2]->GetZeroExtendedValue(), 6u);
  EXPECT_FALSE(tensor_const->IsZero());
}

TEST_F(ConstantManagerTest, GetDefiningInstructionForNestedTensorConstant) {
  const std::string text = R"(
OpCapability TensorsARM
OpExtension "SPV_ARM_tensors"
%1 = OpTypeInt 32 0
%2 = OpConstant %1 1
%3 = OpConstant %1 2
%4 = OpConstant %1 3
%5 = OpConstant %1 4
%6 = OpConstant %1 5
%7 = OpConstant %1 6
%8 = OpTypeArray %1 %2
%9 = OpTypeArray %1 %3
%10 = OpConstantComposite %8 %4
%11 = OpConstantComposite %9 %3 %4
%12 = OpTypeTensorARM %1 %2 %10
%13 = OpTypeTensorARM %1 %3 %11
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(context, nullptr);

  ConstantManager* const_mgr = context->get_constant_mgr();
  Type* tensor_r1_type = context->get_type_mgr()->GetType(12);
  Type* tensor_r2_type = context->get_type_mgr()->GetType(13);
  ASSERT_NE(tensor_r1_type, nullptr);
  ASSERT_NE(tensor_r2_type, nullptr);

  const Constant* row_0 = const_mgr->GetConstant(tensor_r1_type, {2, 3, 4});
  const Constant* row_1 = const_mgr->GetConstant(tensor_r1_type, {5, 6, 7});
  ASSERT_NE(row_0, nullptr);
  ASSERT_NE(row_1, nullptr);

  Instruction* row_0_inst = const_mgr->GetDefiningInstruction(row_0, 12);
  Instruction* row_1_inst = const_mgr->GetDefiningInstruction(row_1, 12);
  ASSERT_NE(row_0_inst, nullptr);
  ASSERT_NE(row_1_inst, nullptr);
  EXPECT_EQ(row_0_inst->opcode(), spv::Op::OpConstantComposite);
  EXPECT_EQ(row_0_inst->type_id(), 12u);
  EXPECT_EQ(row_0_inst->NumInOperands(), 3u);
  EXPECT_EQ(row_0_inst->GetSingleWordInOperand(0), 2u);
  EXPECT_EQ(row_0_inst->GetSingleWordInOperand(1), 3u);
  EXPECT_EQ(row_0_inst->GetSingleWordInOperand(2), 4u);

  const Constant* tensor_const = const_mgr->GetConstant(
      tensor_r2_type, {row_0_inst->result_id(), row_1_inst->result_id()});
  ASSERT_NE(tensor_const, nullptr);

  Instruction* tensor_inst =
      const_mgr->GetDefiningInstruction(tensor_const, 13);
  ASSERT_NE(tensor_inst, nullptr);
  EXPECT_EQ(tensor_inst->opcode(), spv::Op::OpConstantComposite);
  EXPECT_EQ(tensor_inst->type_id(), 13u);
  EXPECT_EQ(tensor_inst->NumInOperands(), 2u);
  EXPECT_EQ(tensor_inst->GetSingleWordInOperand(0), row_0_inst->result_id());
  EXPECT_EQ(tensor_inst->GetSingleWordInOperand(1), row_1_inst->result_id());
}

}  // namespace
}  // namespace analysis
}  // namespace opt
}  // namespace spvtools
