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

#include "source/fuzz/transformation_replace_boolean_constant_with_constant_binary.h"

#include <cmath>

#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/id_use_descriptor.h"

namespace spvtools {
namespace fuzz {

namespace {

// Given floating-point values |lhs| and |rhs|, and a floating-point binary
// operator |binop|, returns true if it is certain that 'lhs binop rhs'
// evaluates to |required_value|.
template <typename T>
bool float_binop_evaluates_to(T lhs, T rhs, SpvOp binop, bool required_value) {
  // Infinity and NaN values are conservatively treated as out of scope.
  if (!std::isfinite(lhs) || !std::isfinite(rhs)) {
    return false;
  }
  bool binop_result;
  // The following captures the binary operators that spirv-fuzz can actually
  // generate when turning a boolean constant into a binary expression.
  switch (binop) {
    case SpvOpFOrdGreaterThanEqual:
    case SpvOpFUnordGreaterThanEqual:
      binop_result = (lhs >= rhs);
      break;
    case SpvOpFOrdGreaterThan:
    case SpvOpFUnordGreaterThan:
      binop_result = (lhs > rhs);
      break;
    case SpvOpFOrdLessThanEqual:
    case SpvOpFUnordLessThanEqual:
      binop_result = (lhs <= rhs);
      break;
    case SpvOpFOrdLessThan:
    case SpvOpFUnordLessThan:
      binop_result = (lhs < rhs);
      break;
    default:
      return false;
  }
  return binop_result == required_value;
}

// Analogous to 'float_binop_evaluates_to', but for signed int values.
template <typename T>
bool signed_int_binop_evaluates_to(T lhs, T rhs, SpvOp binop,
                                   bool required_value) {
  bool binop_result;
  switch (binop) {
    case SpvOpSGreaterThanEqual:
      binop_result = (lhs >= rhs);
      break;
    case SpvOpSGreaterThan:
      binop_result = (lhs > rhs);
      break;
    case SpvOpSLessThanEqual:
      binop_result = (lhs <= rhs);
      break;
    case SpvOpSLessThan:
      binop_result = (lhs < rhs);
      break;
    default:
      return false;
  }
  return binop_result == required_value;
}

// Analogous to 'float_binop_evaluates_to', but for unsigned int values.
template <typename T>
bool unsigned_int_binop_evaluates_to(T lhs, T rhs, SpvOp binop,
                                     bool required_value) {
  bool binop_result;
  switch (binop) {
    case SpvOpUGreaterThanEqual:
      binop_result = (lhs >= rhs);
      break;
    case SpvOpUGreaterThan:
      binop_result = (lhs > rhs);
      break;
    case SpvOpULessThanEqual:
      binop_result = (lhs <= rhs);
      break;
    case SpvOpULessThan:
      binop_result = (lhs < rhs);
      break;
    default:
      return false;
  }
  return binop_result == required_value;
}

}  // namespace

TransformationReplaceBooleanConstantWithConstantBinary::
    TransformationReplaceBooleanConstantWithConstantBinary(
        const spvtools::fuzz::protobufs::
            TransformationReplaceBooleanConstantWithConstantBinary& message)
    : message_(message) {}

TransformationReplaceBooleanConstantWithConstantBinary::
    TransformationReplaceBooleanConstantWithConstantBinary(
        const protobufs::IdUseDescriptor& id_use_descriptor, uint32_t lhs_id,
        uint32_t rhs_id, SpvOp comparison_opcode,
        uint32_t fresh_id_for_binary_operation) {
  *message_.mutable_id_use_descriptor() = id_use_descriptor;
  message_.set_lhs_id(lhs_id);
  message_.set_rhs_id(rhs_id);
  message_.set_opcode(comparison_opcode);
  message_.set_fresh_id_for_binary_operation(fresh_id_for_binary_operation);
}

bool TransformationReplaceBooleanConstantWithConstantBinary::IsApplicable(
    opt::IRContext* context, const FactManager& /*unused*/) const {
  // The id for the binary result must be fresh
  if (!fuzzerutil::IsFreshId(context,
                             message_.fresh_id_for_binary_operation())) {
    return false;
  }

  // The used id must be for a boolean constant
  auto boolean_constant = context->get_def_use_mgr()->GetDef(
      message_.id_use_descriptor().id_of_interest());
  if (!boolean_constant) {
    return false;
  }
  if (!(boolean_constant->opcode() == SpvOpConstantFalse ||
        boolean_constant->opcode() == SpvOpConstantTrue)) {
    return false;
  }

  // The left-hand-side id must correspond to a constant instruction.
  auto lhs_constant_inst =
      context->get_def_use_mgr()->GetDef(message_.lhs_id());
  if (!lhs_constant_inst) {
    return false;
  }
  if (lhs_constant_inst->opcode() != SpvOpConstant) {
    return false;
  }

  // The right-hand-side id must correspond to a constant instruction.
  auto rhs_constant_inst =
      context->get_def_use_mgr()->GetDef(message_.rhs_id());
  if (!rhs_constant_inst) {
    return false;
  }
  if (rhs_constant_inst->opcode() != SpvOpConstant) {
    return false;
  }

  // The left- and right-hand side instructions must have the same type.
  if (lhs_constant_inst->type_id() != rhs_constant_inst->type_id()) {
    return false;
  }

  // The expression 'LHS opcode RHS' must evaluate to the boolean constant.
  auto lhs_constant =
      context->get_constant_mgr()->FindDeclaredConstant(message_.lhs_id());
  auto rhs_constant =
      context->get_constant_mgr()->FindDeclaredConstant(message_.rhs_id());
  bool expected_result = (boolean_constant->opcode() == SpvOpConstantTrue);

  const auto binary_opcode = static_cast<SpvOp>(message_.opcode());

  // We consider the floating point, signed and unsigned integer cases
  // separately.  In each case the logic is very similar.
  if (lhs_constant->AsFloatConstant()) {
    assert(rhs_constant->AsFloatConstant() &&
           "Both constants should be of the same type.");
    if (lhs_constant->type()->AsFloat()->width() == 32) {
      if (!float_binop_evaluates_to(lhs_constant->GetFloat(),
                                    rhs_constant->GetFloat(), binary_opcode,
                                    expected_result)) {
        return false;
      }
    } else {
      assert(lhs_constant->type()->AsFloat()->width() == 64);
      if (!float_binop_evaluates_to(lhs_constant->GetDouble(),
                                    rhs_constant->GetDouble(), binary_opcode,
                                    expected_result)) {
        return false;
      }
    }
  } else {
    assert(lhs_constant->AsIntConstant() && "Constants should be in or float.");
    assert(rhs_constant->AsIntConstant() &&
           "Both constants should be of the same type.");
    if (lhs_constant->type()->AsInteger()->IsSigned()) {
      if (lhs_constant->type()->AsInteger()->width() == 32) {
        if (!signed_int_binop_evaluates_to(lhs_constant->GetS32(),
                                           rhs_constant->GetS32(),
                                           binary_opcode, expected_result)) {
          return false;
        }
      } else {
        assert(lhs_constant->type()->AsInteger()->width() == 64);
        if (!signed_int_binop_evaluates_to(lhs_constant->GetS64(),
                                           rhs_constant->GetS64(),
                                           binary_opcode, expected_result)) {
          return false;
        }
      }
    } else {
      if (lhs_constant->type()->AsInteger()->width() == 32) {
        if (!unsigned_int_binop_evaluates_to(lhs_constant->GetU32(),
                                             rhs_constant->GetU32(),
                                             binary_opcode, expected_result)) {
          return false;
        }
      } else {
        assert(lhs_constant->type()->AsInteger()->width() == 64);
        if (!unsigned_int_binop_evaluates_to(lhs_constant->GetU64(),
                                             rhs_constant->GetU64(),
                                             binary_opcode, expected_result)) {
          return false;
        }
      }
    }
  }

  // The id use descriptor must identify some instruction
  auto instruction =
      FindInstructionContainingUse(message_.id_use_descriptor(), context);
  if (instruction == nullptr) {
    return false;
  }

  // The instruction must not be an OpPhi, as we cannot insert a binary
  // operator instruction before an OpPhi.
  // TODO(https://github.com/KhronosGroup/SPIRV-Tools/issues/2902): there is
  //  scope for being less conservative.
  return instruction->opcode() != SpvOpPhi;
}

void TransformationReplaceBooleanConstantWithConstantBinary::Apply(
    opt::IRContext* context, FactManager* fact_manager) const {
  ApplyWithResult(context, fact_manager);
}

opt::Instruction*
TransformationReplaceBooleanConstantWithConstantBinary::ApplyWithResult(
    opt::IRContext* context, FactManager* /*unused*/) const {
  opt::analysis::Bool bool_type;
  opt::Instruction::OperandList operands = {
      {SPV_OPERAND_TYPE_ID, {message_.lhs_id()}},
      {SPV_OPERAND_TYPE_ID, {message_.rhs_id()}}};
  auto binary_instruction = MakeUnique<opt::Instruction>(
      context, static_cast<SpvOp>(message_.opcode()),
      context->get_type_mgr()->GetId(&bool_type),
      message_.fresh_id_for_binary_operation(), operands);
  opt::Instruction* result = binary_instruction.get();
  auto instruction_containing_constant_use =
      FindInstructionContainingUse(message_.id_use_descriptor(), context);

  // We want to insert the new instruction before the instruction that contains
  // the use of the boolean, but we need to go backwards one more instruction if
  // the using instruction is preceded by a merge instruction.
  auto instruction_before_which_to_insert = instruction_containing_constant_use;
  {
    opt::Instruction* previous_node =
        instruction_before_which_to_insert->PreviousNode();
    if (previous_node && (previous_node->opcode() == SpvOpLoopMerge ||
                          previous_node->opcode() == SpvOpSelectionMerge)) {
      instruction_before_which_to_insert = previous_node;
    }
  }
  instruction_before_which_to_insert->InsertBefore(
      std::move(binary_instruction));
  instruction_containing_constant_use->SetInOperand(
      message_.id_use_descriptor().in_operand_index(),
      {message_.fresh_id_for_binary_operation()});
  fuzzerutil::UpdateModuleIdBound(context,
                                  message_.fresh_id_for_binary_operation());
  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);
  return result;
}

protobufs::Transformation
TransformationReplaceBooleanConstantWithConstantBinary::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_replace_boolean_constant_with_constant_binary() = message_;
  return result;
}

}  // namespace fuzz
}  // namespace spvtools
