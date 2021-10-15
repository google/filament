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

#include "source/fuzz/transformation_propagate_instruction_up.h"

#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"

namespace spvtools {
namespace fuzz {
namespace {

uint32_t GetResultIdFromLabelId(const opt::Instruction& phi_inst,
                                uint32_t label_id) {
  assert(phi_inst.opcode() == SpvOpPhi && "|phi_inst| is not an OpPhi");

  for (uint32_t i = 1; i < phi_inst.NumInOperands(); i += 2) {
    if (phi_inst.GetSingleWordInOperand(i) == label_id) {
      return phi_inst.GetSingleWordInOperand(i - 1);
    }
  }

  return 0;
}

bool ContainsPointers(const opt::analysis::Type& type) {
  switch (type.kind()) {
    case opt::analysis::Type::kPointer:
      return true;
    case opt::analysis::Type::kStruct:
      return std::any_of(type.AsStruct()->element_types().begin(),
                         type.AsStruct()->element_types().end(),
                         [](const opt::analysis::Type* element_type) {
                           return ContainsPointers(*element_type);
                         });
    default:
      return false;
  }
}

bool HasValidDependencies(opt::IRContext* ir_context, opt::Instruction* inst) {
  const auto* inst_block = ir_context->get_instr_block(inst);
  assert(inst_block &&
         "This function shouldn't be applied to global instructions or function"
         "parameters");

  for (uint32_t i = 0; i < inst->NumInOperands(); ++i) {
    const auto& operand = inst->GetInOperand(i);
    if (operand.type != SPV_OPERAND_TYPE_ID) {
      // Consider only <id> operands.
      continue;
    }

    auto* dependency = ir_context->get_def_use_mgr()->GetDef(operand.words[0]);
    assert(dependency && "Operand has invalid id");

    if (ir_context->get_instr_block(dependency) == inst_block &&
        dependency->opcode() != SpvOpPhi) {
      // |dependency| is "valid" if it's an OpPhi from the same basic block or
      // an instruction from a different basic block.
      return false;
    }
  }

  return true;
}

}  // namespace

TransformationPropagateInstructionUp::TransformationPropagateInstructionUp(
    protobufs::TransformationPropagateInstructionUp message)
    : message_(std::move(message)) {}

TransformationPropagateInstructionUp::TransformationPropagateInstructionUp(
    uint32_t block_id,
    const std::map<uint32_t, uint32_t>& predecessor_id_to_fresh_id) {
  message_.set_block_id(block_id);
  *message_.mutable_predecessor_id_to_fresh_id() =
      fuzzerutil::MapToRepeatedUInt32Pair(predecessor_id_to_fresh_id);
}

bool TransformationPropagateInstructionUp::IsApplicable(
    opt::IRContext* ir_context, const TransformationContext& /*unused*/) const {
  // Check that we can apply this transformation to the |block_id|.
  if (!IsApplicableToBlock(ir_context, message_.block_id())) {
    return false;
  }

  const auto predecessor_id_to_fresh_id = fuzzerutil::RepeatedUInt32PairToMap(
      message_.predecessor_id_to_fresh_id());
  for (auto id : ir_context->cfg()->preds(message_.block_id())) {
    // Each predecessor must have a fresh id in the |predecessor_id_to_fresh_id|
    // map.
    if (!predecessor_id_to_fresh_id.count(id)) {
      return false;
    }
  }

  std::vector<uint32_t> maybe_fresh_ids;
  maybe_fresh_ids.reserve(predecessor_id_to_fresh_id.size());
  for (const auto& entry : predecessor_id_to_fresh_id) {
    maybe_fresh_ids.push_back(entry.second);
  }

  // All ids must be unique and fresh.
  return !fuzzerutil::HasDuplicates(maybe_fresh_ids) &&
         std::all_of(maybe_fresh_ids.begin(), maybe_fresh_ids.end(),
                     [ir_context](uint32_t id) {
                       return fuzzerutil::IsFreshId(ir_context, id);
                     });
}

void TransformationPropagateInstructionUp::Apply(
    opt::IRContext* ir_context, TransformationContext* /*unused*/) const {
  auto* inst = GetInstructionToPropagate(ir_context, message_.block_id());
  assert(inst &&
         "The block must have at least one supported instruction to propagate");
  assert(inst->result_id() && inst->type_id() &&
         "|inst| must have a result id and a type id");

  opt::Instruction::OperandList op_phi_operands;
  const auto predecessor_id_to_fresh_id = fuzzerutil::RepeatedUInt32PairToMap(
      message_.predecessor_id_to_fresh_id());
  std::unordered_set<uint32_t> visited_predecessors;
  for (auto predecessor_id : ir_context->cfg()->preds(message_.block_id())) {
    // A block can have multiple identical predecessors.
    if (visited_predecessors.count(predecessor_id)) {
      continue;
    }

    visited_predecessors.insert(predecessor_id);

    auto new_result_id = predecessor_id_to_fresh_id.at(predecessor_id);

    // Compute InOperands for the OpPhi instruction to be inserted later.
    op_phi_operands.push_back({SPV_OPERAND_TYPE_ID, {new_result_id}});
    op_phi_operands.push_back({SPV_OPERAND_TYPE_ID, {predecessor_id}});

    // Create a clone of the |inst| to be inserted into the |predecessor_id|.
    std::unique_ptr<opt::Instruction> clone(inst->Clone(ir_context));
    clone->SetResultId(new_result_id);

    fuzzerutil::UpdateModuleIdBound(ir_context, new_result_id);

    // Adjust |clone|'s operands to account for possible dependencies on OpPhi
    // instructions from the same basic block.
    for (uint32_t i = 0; i < clone->NumInOperands(); ++i) {
      auto& operand = clone->GetInOperand(i);
      if (operand.type != SPV_OPERAND_TYPE_ID) {
        // Consider only ids.
        continue;
      }

      const auto* dependency_inst =
          ir_context->get_def_use_mgr()->GetDef(operand.words[0]);
      assert(dependency_inst && "|clone| depends on an invalid id");

      if (ir_context->get_instr_block(dependency_inst->result_id()) !=
          ir_context->cfg()->block(message_.block_id())) {
        // We don't need to adjust anything if |dependency_inst| is from a
        // different block, a global instruction or a function parameter.
        continue;
      }

      assert(dependency_inst->opcode() == SpvOpPhi &&
             "Propagated instruction can depend only on OpPhis from the same "
             "basic block or instructions from different basic blocks");

      auto new_id = GetResultIdFromLabelId(*dependency_inst, predecessor_id);
      assert(new_id && "OpPhi instruction is missing a predecessor");
      operand.words[0] = new_id;
    }

    auto* insert_before_inst = fuzzerutil::GetLastInsertBeforeInstruction(
        ir_context, predecessor_id, clone->opcode());
    assert(insert_before_inst && "Can't insert |clone| into |predecessor_id");

    insert_before_inst->InsertBefore(std::move(clone));
  }

  // Insert an OpPhi instruction into the basic block of |inst|.
  ir_context->get_instr_block(inst)->begin()->InsertBefore(
      MakeUnique<opt::Instruction>(ir_context, SpvOpPhi, inst->type_id(),
                                   inst->result_id(),
                                   std::move(op_phi_operands)));

  // Remove |inst| from the basic block.
  ir_context->KillInst(inst);

  // We have changed the module so most analyzes are now invalid.
  ir_context->InvalidateAnalysesExceptFor(
      opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationPropagateInstructionUp::ToMessage()
    const {
  protobufs::Transformation result;
  *result.mutable_propagate_instruction_up() = message_;
  return result;
}

bool TransformationPropagateInstructionUp::IsOpcodeSupported(SpvOp opcode) {
  // TODO(https://github.com/KhronosGroup/SPIRV-Tools/issues/3605):
  //  We only support "simple" instructions that don't work with memory.
  //  We should extend this so that we support the ones that modify the memory
  //  too.
  switch (opcode) {
    case SpvOpUndef:
    case SpvOpAccessChain:
    case SpvOpInBoundsAccessChain:
    case SpvOpArrayLength:
    case SpvOpVectorExtractDynamic:
    case SpvOpVectorInsertDynamic:
    case SpvOpVectorShuffle:
    case SpvOpCompositeConstruct:
    case SpvOpCompositeExtract:
    case SpvOpCompositeInsert:
    case SpvOpCopyObject:
    case SpvOpTranspose:
    case SpvOpConvertFToU:
    case SpvOpConvertFToS:
    case SpvOpConvertSToF:
    case SpvOpConvertUToF:
    case SpvOpUConvert:
    case SpvOpSConvert:
    case SpvOpFConvert:
    case SpvOpQuantizeToF16:
    case SpvOpSatConvertSToU:
    case SpvOpSatConvertUToS:
    case SpvOpBitcast:
    case SpvOpSNegate:
    case SpvOpFNegate:
    case SpvOpIAdd:
    case SpvOpFAdd:
    case SpvOpISub:
    case SpvOpFSub:
    case SpvOpIMul:
    case SpvOpFMul:
    case SpvOpUDiv:
    case SpvOpSDiv:
    case SpvOpFDiv:
    case SpvOpUMod:
    case SpvOpSRem:
    case SpvOpSMod:
    case SpvOpFRem:
    case SpvOpFMod:
    case SpvOpVectorTimesScalar:
    case SpvOpMatrixTimesScalar:
    case SpvOpVectorTimesMatrix:
    case SpvOpMatrixTimesVector:
    case SpvOpMatrixTimesMatrix:
    case SpvOpOuterProduct:
    case SpvOpDot:
    case SpvOpIAddCarry:
    case SpvOpISubBorrow:
    case SpvOpUMulExtended:
    case SpvOpSMulExtended:
    case SpvOpAny:
    case SpvOpAll:
    case SpvOpIsNan:
    case SpvOpIsInf:
    case SpvOpIsFinite:
    case SpvOpIsNormal:
    case SpvOpSignBitSet:
    case SpvOpLessOrGreater:
    case SpvOpOrdered:
    case SpvOpUnordered:
    case SpvOpLogicalEqual:
    case SpvOpLogicalNotEqual:
    case SpvOpLogicalOr:
    case SpvOpLogicalAnd:
    case SpvOpLogicalNot:
    case SpvOpSelect:
    case SpvOpIEqual:
    case SpvOpINotEqual:
    case SpvOpUGreaterThan:
    case SpvOpSGreaterThan:
    case SpvOpUGreaterThanEqual:
    case SpvOpSGreaterThanEqual:
    case SpvOpULessThan:
    case SpvOpSLessThan:
    case SpvOpULessThanEqual:
    case SpvOpSLessThanEqual:
    case SpvOpFOrdEqual:
    case SpvOpFUnordEqual:
    case SpvOpFOrdNotEqual:
    case SpvOpFUnordNotEqual:
    case SpvOpFOrdLessThan:
    case SpvOpFUnordLessThan:
    case SpvOpFOrdGreaterThan:
    case SpvOpFUnordGreaterThan:
    case SpvOpFOrdLessThanEqual:
    case SpvOpFUnordLessThanEqual:
    case SpvOpFOrdGreaterThanEqual:
    case SpvOpFUnordGreaterThanEqual:
    case SpvOpShiftRightLogical:
    case SpvOpShiftRightArithmetic:
    case SpvOpShiftLeftLogical:
    case SpvOpBitwiseOr:
    case SpvOpBitwiseXor:
    case SpvOpBitwiseAnd:
    case SpvOpNot:
    case SpvOpBitFieldInsert:
    case SpvOpBitFieldSExtract:
    case SpvOpBitFieldUExtract:
    case SpvOpBitReverse:
    case SpvOpBitCount:
    case SpvOpCopyLogical:
    case SpvOpPtrEqual:
    case SpvOpPtrNotEqual:
      return true;
    default:
      return false;
  }
}

opt::Instruction*
TransformationPropagateInstructionUp::GetInstructionToPropagate(
    opt::IRContext* ir_context, uint32_t block_id) {
  auto* block = ir_context->cfg()->block(block_id);
  assert(block && "|block_id| is invalid");

  for (auto& inst : *block) {
    // We look for the first instruction in the block that satisfies the
    // following rules:
    // - it's not an OpPhi
    // - it must be supported by this transformation
    // - it may depend only on instructions from different basic blocks or on
    //   OpPhi instructions from the same basic block.
    if (inst.opcode() == SpvOpPhi || !IsOpcodeSupported(inst.opcode()) ||
        !inst.type_id() || !inst.result_id()) {
      continue;
    }

    const auto* inst_type = ir_context->get_type_mgr()->GetType(inst.type_id());
    assert(inst_type && "|inst| has invalid type");

    if (inst_type->AsSampledImage()) {
      // OpTypeSampledImage cannot be used as an argument to OpPhi instructions,
      // thus we cannot support this type.
      continue;
    }

    if (!ir_context->get_feature_mgr()->HasCapability(
            SpvCapabilityVariablePointersStorageBuffer) &&
        ContainsPointers(*inst_type)) {
      // OpPhi supports pointer operands only with VariablePointers or
      // VariablePointersStorageBuffer capabilities.
      //
      // Note that VariablePointers capability implicitly declares
      // VariablePointersStorageBuffer capability.
      continue;
    }

    if (!HasValidDependencies(ir_context, &inst)) {
      continue;
    }

    return &inst;
  }

  return nullptr;
}

bool TransformationPropagateInstructionUp::IsApplicableToBlock(
    opt::IRContext* ir_context, uint32_t block_id) {
  // Check that |block_id| is valid.
  const auto* label_inst = ir_context->get_def_use_mgr()->GetDef(block_id);
  if (!label_inst || label_inst->opcode() != SpvOpLabel) {
    return false;
  }

  // Check that |block| has predecessors.
  const auto& predecessors = ir_context->cfg()->preds(block_id);
  if (predecessors.empty()) {
    return false;
  }

  // The block must contain an instruction to propagate.
  const auto* inst_to_propagate =
      GetInstructionToPropagate(ir_context, block_id);
  if (!inst_to_propagate) {
    return false;
  }

  // We should be able to insert |inst_to_propagate| into every predecessor of
  // |block|.
  return std::all_of(predecessors.begin(), predecessors.end(),
                     [ir_context, inst_to_propagate](uint32_t predecessor_id) {
                       return fuzzerutil::GetLastInsertBeforeInstruction(
                                  ir_context, predecessor_id,
                                  inst_to_propagate->opcode()) != nullptr;
                     });
}

std::unordered_set<uint32_t> TransformationPropagateInstructionUp::GetFreshIds()
    const {
  std::unordered_set<uint32_t> result;
  for (auto& pair : message_.predecessor_id_to_fresh_id()) {
    result.insert(pair.second());
  }
  return result;
}

}  // namespace fuzz
}  // namespace spvtools
