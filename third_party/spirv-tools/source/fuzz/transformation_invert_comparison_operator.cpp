// Copyright (c) 2020 Vasyl Teliman
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

#include "source/fuzz/transformation_invert_comparison_operator.h"

#include <utility>

#include "source/fuzz/fuzzer_util.h"

namespace spvtools {
namespace fuzz {

TransformationInvertComparisonOperator::TransformationInvertComparisonOperator(
    protobufs::TransformationInvertComparisonOperator message)
    : message_(std::move(message)) {}

TransformationInvertComparisonOperator::TransformationInvertComparisonOperator(
    uint32_t operator_id, uint32_t fresh_id) {
  message_.set_operator_id(operator_id);
  message_.set_fresh_id(fresh_id);
}

bool TransformationInvertComparisonOperator::IsApplicable(
    opt::IRContext* ir_context, const TransformationContext& /*unused*/) const {
  // |message_.operator_id| must be valid and inversion must be supported for
  // it.
  auto* inst = ir_context->get_def_use_mgr()->GetDef(message_.operator_id());
  if (!inst || !IsInversionSupported(inst->opcode())) {
    return false;
  }

  // Check that we can insert negation instruction.
  auto* block = ir_context->get_instr_block(inst);
  assert(block && "Instruction must have a basic block");

  auto iter = fuzzerutil::GetIteratorForInstruction(block, inst);
  ++iter;
  assert(iter != block->end() && "Instruction can't be the last in the block");
  assert(fuzzerutil::CanInsertOpcodeBeforeInstruction(SpvOpLogicalNot, iter) &&
         "Can't insert negation after comparison operator");

  // |message_.fresh_id| must be fresh.
  return fuzzerutil::IsFreshId(ir_context, message_.fresh_id());
}

void TransformationInvertComparisonOperator::Apply(
    opt::IRContext* ir_context, TransformationContext* /*unused*/) const {
  auto* inst = ir_context->get_def_use_mgr()->GetDef(message_.operator_id());
  assert(inst && "Result id of an operator is invalid");

  // Insert negation after |inst|.
  auto iter = fuzzerutil::GetIteratorForInstruction(
      ir_context->get_instr_block(inst), inst);
  ++iter;

  iter.InsertBefore(MakeUnique<opt::Instruction>(
      ir_context, SpvOpLogicalNot, inst->type_id(), inst->result_id(),
      opt::Instruction::OperandList{
          {SPV_OPERAND_TYPE_ID, {message_.fresh_id()}}}));

  // Change the result id of the original operator to |fresh_id|.
  inst->SetResultId(message_.fresh_id());

  // Invert the operator.
  inst->SetOpcode(InvertOpcode(inst->opcode()));

  fuzzerutil::UpdateModuleIdBound(ir_context, message_.fresh_id());

  ir_context->InvalidateAnalysesExceptFor(
      opt::IRContext::Analysis::kAnalysisNone);
}

bool TransformationInvertComparisonOperator::IsInversionSupported(
    SpvOp opcode) {
  switch (opcode) {
    case SpvOpSGreaterThan:
    case SpvOpSGreaterThanEqual:
    case SpvOpSLessThan:
    case SpvOpSLessThanEqual:
    case SpvOpUGreaterThan:
    case SpvOpUGreaterThanEqual:
    case SpvOpULessThan:
    case SpvOpULessThanEqual:
    case SpvOpIEqual:
    case SpvOpINotEqual:
    case SpvOpFOrdEqual:
    case SpvOpFUnordEqual:
    case SpvOpFOrdNotEqual:
    case SpvOpFUnordNotEqual:
    case SpvOpFOrdLessThan:
    case SpvOpFUnordLessThan:
    case SpvOpFOrdLessThanEqual:
    case SpvOpFUnordLessThanEqual:
    case SpvOpFOrdGreaterThan:
    case SpvOpFUnordGreaterThan:
    case SpvOpFOrdGreaterThanEqual:
    case SpvOpFUnordGreaterThanEqual:
      return true;
    default:
      return false;
  }
}

SpvOp TransformationInvertComparisonOperator::InvertOpcode(SpvOp opcode) {
  assert(IsInversionSupported(opcode) && "Inversion must be supported");

  switch (opcode) {
    case SpvOpSGreaterThan:
      return SpvOpSLessThanEqual;
    case SpvOpSGreaterThanEqual:
      return SpvOpSLessThan;
    case SpvOpSLessThan:
      return SpvOpSGreaterThanEqual;
    case SpvOpSLessThanEqual:
      return SpvOpSGreaterThan;
    case SpvOpUGreaterThan:
      return SpvOpULessThanEqual;
    case SpvOpUGreaterThanEqual:
      return SpvOpULessThan;
    case SpvOpULessThan:
      return SpvOpUGreaterThanEqual;
    case SpvOpULessThanEqual:
      return SpvOpUGreaterThan;
    case SpvOpIEqual:
      return SpvOpINotEqual;
    case SpvOpINotEqual:
      return SpvOpIEqual;
    case SpvOpFOrdEqual:
      return SpvOpFUnordNotEqual;
    case SpvOpFUnordEqual:
      return SpvOpFOrdNotEqual;
    case SpvOpFOrdNotEqual:
      return SpvOpFUnordEqual;
    case SpvOpFUnordNotEqual:
      return SpvOpFOrdEqual;
    case SpvOpFOrdLessThan:
      return SpvOpFUnordGreaterThanEqual;
    case SpvOpFUnordLessThan:
      return SpvOpFOrdGreaterThanEqual;
    case SpvOpFOrdLessThanEqual:
      return SpvOpFUnordGreaterThan;
    case SpvOpFUnordLessThanEqual:
      return SpvOpFOrdGreaterThan;
    case SpvOpFOrdGreaterThan:
      return SpvOpFUnordLessThanEqual;
    case SpvOpFUnordGreaterThan:
      return SpvOpFOrdLessThanEqual;
    case SpvOpFOrdGreaterThanEqual:
      return SpvOpFUnordLessThan;
    case SpvOpFUnordGreaterThanEqual:
      return SpvOpFOrdLessThan;
    default:
      // The program will fail in the debug mode because of the assertion
      // at the beginning of the function.
      return SpvOpNop;
  }
}

protobufs::Transformation TransformationInvertComparisonOperator::ToMessage()
    const {
  protobufs::Transformation result;
  *result.mutable_invert_comparison_operator() = message_;
  return result;
}

std::unordered_set<uint32_t>
TransformationInvertComparisonOperator::GetFreshIds() const {
  return {message_.fresh_id()};
}

}  // namespace fuzz
}  // namespace spvtools
