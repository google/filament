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

#include "source/fuzz/fuzzer_util.h"

#include "source/opt/build_module.h"

namespace spvtools {
namespace fuzz {

namespace fuzzerutil {

bool IsFreshId(opt::IRContext* context, uint32_t id) {
  return !context->get_def_use_mgr()->GetDef(id);
}

void UpdateModuleIdBound(opt::IRContext* context, uint32_t id) {
  // TODO(https://github.com/KhronosGroup/SPIRV-Tools/issues/2541) consider the
  //  case where the maximum id bound is reached.
  context->module()->SetIdBound(
      std::max(context->module()->id_bound(), id + 1));
}

opt::BasicBlock* MaybeFindBlock(opt::IRContext* context,
                                uint32_t maybe_block_id) {
  auto inst = context->get_def_use_mgr()->GetDef(maybe_block_id);
  if (inst == nullptr) {
    // No instruction defining this id was found.
    return nullptr;
  }
  if (inst->opcode() != SpvOpLabel) {
    // The instruction defining the id is not a label, so it cannot be a block
    // id.
    return nullptr;
  }
  return context->cfg()->block(maybe_block_id);
}

bool PhiIdsOkForNewEdge(
    opt::IRContext* context, opt::BasicBlock* bb_from, opt::BasicBlock* bb_to,
    const google::protobuf::RepeatedField<google::protobuf::uint32>& phi_ids) {
  if (bb_from->IsSuccessor(bb_to)) {
    // There is already an edge from |from_block| to |to_block|, so there is
    // no need to extend OpPhi instructions.  Do not allow phi ids to be
    // present. This might turn out to be too strict; perhaps it would be OK
    // just to ignore the ids in this case.
    return phi_ids.empty();
  }
  // The edge would add a previously non-existent edge from |from_block| to
  // |to_block|, so we go through the given phi ids and check that they exactly
  // match the OpPhi instructions in |to_block|.
  uint32_t phi_index = 0;
  // An explicit loop, rather than applying a lambda to each OpPhi in |bb_to|,
  // makes sense here because we need to increment |phi_index| for each OpPhi
  // instruction.
  for (auto& inst : *bb_to) {
    if (inst.opcode() != SpvOpPhi) {
      // The OpPhi instructions all occur at the start of the block; if we find
      // a non-OpPhi then we have seen them all.
      break;
    }
    if (phi_index == static_cast<uint32_t>(phi_ids.size())) {
      // Not enough phi ids have been provided to account for the OpPhi
      // instructions.
      return false;
    }
    // Look for an instruction defining the next phi id.
    opt::Instruction* phi_extension =
        context->get_def_use_mgr()->GetDef(phi_ids[phi_index]);
    if (!phi_extension) {
      // The id given to extend this OpPhi does not exist.
      return false;
    }
    if (phi_extension->type_id() != inst.type_id()) {
      // The instruction given to extend this OpPhi either does not have a type
      // or its type does not match that of the OpPhi.
      return false;
    }

    if (context->get_instr_block(phi_extension)) {
      // The instruction defining the phi id has an associated block (i.e., it
      // is not a global value).  Check whether its definition dominates the
      // exit of |from_block|.
      auto dominator_analysis =
          context->GetDominatorAnalysis(bb_from->GetParent());
      if (!dominator_analysis->Dominates(phi_extension,
                                         bb_from->terminator())) {
        // The given id is no good as its definition does not dominate the exit
        // of |from_block|
        return false;
      }
    }
    phi_index++;
  }
  // We allow some of the ids provided for extending OpPhi instructions to be
  // unused.  Their presence does no harm, and requiring a perfect match may
  // make transformations less likely to cleanly apply.
  return true;
}

uint32_t MaybeGetBoolConstantId(opt::IRContext* context, bool value) {
  opt::analysis::Bool bool_type;
  auto registered_bool_type =
      context->get_type_mgr()->GetRegisteredType(&bool_type);
  if (!registered_bool_type) {
    return 0;
  }
  opt::analysis::BoolConstant bool_constant(registered_bool_type->AsBool(),
                                            value);
  return context->get_constant_mgr()->FindDeclaredConstant(
      &bool_constant, context->get_type_mgr()->GetId(&bool_type));
}

void AddUnreachableEdgeAndUpdateOpPhis(
    opt::IRContext* context, opt::BasicBlock* bb_from, opt::BasicBlock* bb_to,
    bool condition_value,
    const google::protobuf::RepeatedField<google::protobuf::uint32>& phi_ids) {
  assert(PhiIdsOkForNewEdge(context, bb_from, bb_to, phi_ids) &&
         "Precondition on phi_ids is not satisfied");
  assert(bb_from->terminator()->opcode() == SpvOpBranch &&
         "Precondition on terminator of bb_from is not satisfied");

  // Get the id of the boolean constant to be used as the condition.
  uint32_t bool_id = MaybeGetBoolConstantId(context, condition_value);
  assert(
      bool_id &&
      "Precondition that condition value must be available is not satisfied");

  const bool from_to_edge_already_exists = bb_from->IsSuccessor(bb_to);
  auto successor = bb_from->terminator()->GetSingleWordInOperand(0);

  // Add the dead branch, by turning OpBranch into OpBranchConditional, and
  // ordering the targets depending on whether the given boolean corresponds to
  // true or false.
  bb_from->terminator()->SetOpcode(SpvOpBranchConditional);
  bb_from->terminator()->SetInOperands(
      {{SPV_OPERAND_TYPE_ID, {bool_id}},
       {SPV_OPERAND_TYPE_ID, {condition_value ? successor : bb_to->id()}},
       {SPV_OPERAND_TYPE_ID, {condition_value ? bb_to->id() : successor}}});

  // Update OpPhi instructions in the target block if this branch adds a
  // previously non-existent edge from source to target.
  if (!from_to_edge_already_exists) {
    uint32_t phi_index = 0;
    for (auto& inst : *bb_to) {
      if (inst.opcode() != SpvOpPhi) {
        break;
      }
      assert(phi_index < static_cast<uint32_t>(phi_ids.size()) &&
             "There should be at least one phi id per OpPhi instruction.");
      inst.AddOperand({SPV_OPERAND_TYPE_ID, {phi_ids[phi_index]}});
      inst.AddOperand({SPV_OPERAND_TYPE_ID, {bb_from->id()}});
      phi_index++;
    }
  }
}

bool BlockIsInLoopContinueConstruct(opt::IRContext* context, uint32_t block_id,
                                    uint32_t maybe_loop_header_id) {
  // We deem a block to be part of a loop's continue construct if the loop's
  // continue target dominates the block.
  auto containing_construct_block = context->cfg()->block(maybe_loop_header_id);
  if (containing_construct_block->IsLoopHeader()) {
    auto continue_target = containing_construct_block->ContinueBlockId();
    if (context->GetDominatorAnalysis(containing_construct_block->GetParent())
            ->Dominates(continue_target, block_id)) {
      return true;
    }
  }
  return false;
}

opt::BasicBlock::iterator GetIteratorForInstruction(
    opt::BasicBlock* block, const opt::Instruction* inst) {
  for (auto inst_it = block->begin(); inst_it != block->end(); ++inst_it) {
    if (inst == &*inst_it) {
      return inst_it;
    }
  }
  return block->end();
}

bool BlockIsReachableInItsFunction(opt::IRContext* context,
                                   opt::BasicBlock* bb) {
  auto enclosing_function = bb->GetParent();
  return context->GetDominatorAnalysis(enclosing_function)
      ->Dominates(enclosing_function->entry().get(), bb);
}

bool CanInsertOpcodeBeforeInstruction(
    SpvOp opcode, const opt::BasicBlock::iterator& instruction_in_block) {
  if (instruction_in_block->PreviousNode() &&
      (instruction_in_block->PreviousNode()->opcode() == SpvOpLoopMerge ||
       instruction_in_block->PreviousNode()->opcode() == SpvOpSelectionMerge)) {
    // We cannot insert directly after a merge instruction.
    return false;
  }
  if (opcode != SpvOpVariable &&
      instruction_in_block->opcode() == SpvOpVariable) {
    // We cannot insert a non-OpVariable instruction directly before a
    // variable; variables in a function must be contiguous in the entry block.
    return false;
  }
  // We cannot insert a non-OpPhi instruction directly before an OpPhi, because
  // OpPhi instructions need to be contiguous at the start of a block.
  return opcode == SpvOpPhi || instruction_in_block->opcode() != SpvOpPhi;
}

bool CanMakeSynonymOf(opt::IRContext* ir_context, opt::Instruction* inst) {
  if (!inst->HasResultId()) {
    // We can only make a synonym of an instruction that generates an id.
    return false;
  }
  if (!inst->type_id()) {
    // We can only make a synonym of an instruction that has a type.
    return false;
  }
  auto type_inst = ir_context->get_def_use_mgr()->GetDef(inst->type_id());
  if (type_inst->opcode() == SpvOpTypePointer) {
    switch (inst->opcode()) {
      case SpvOpConstantNull:
      case SpvOpUndef:
        // We disallow making synonyms of null or undefined pointers.  This is
        // to provide the property that if the original shader exhibited no bad
        // pointer accesses, the transformed shader will not either.
        return false;
      default:
        break;
    }
  }

  // We do not make synonyms of objects that have decorations: if the synonym is
  // not decorated analogously, using the original object vs. its synonymous
  // form may not be equivalent.
  return ir_context->get_decoration_mgr()
      ->GetDecorationsFor(inst->result_id(), true)
      .empty();
}

bool IsCompositeType(const opt::analysis::Type* type) {
  return type && (type->AsArray() || type->AsMatrix() || type->AsStruct() ||
                  type->AsVector());
}

std::vector<uint32_t> RepeatedFieldToVector(
    const google::protobuf::RepeatedField<uint32_t>& repeated_field) {
  std::vector<uint32_t> result;
  for (auto i : repeated_field) {
    result.push_back(i);
  }
  return result;
}

uint32_t WalkCompositeTypeIndices(
    opt::IRContext* context, uint32_t base_object_type_id,
    const google::protobuf::RepeatedField<google::protobuf::uint32>& indices) {
  uint32_t sub_object_type_id = base_object_type_id;
  for (auto index : indices) {
    auto should_be_composite_type =
        context->get_def_use_mgr()->GetDef(sub_object_type_id);
    assert(should_be_composite_type && "The type should exist.");
    switch (should_be_composite_type->opcode()) {
      case SpvOpTypeArray: {
        auto array_length = GetArraySize(*should_be_composite_type, context);
        if (array_length == 0 || index >= array_length) {
          return 0;
        }
        sub_object_type_id =
            should_be_composite_type->GetSingleWordInOperand(0);
        break;
      }
      case SpvOpTypeMatrix:
      case SpvOpTypeVector: {
        auto count = should_be_composite_type->GetSingleWordInOperand(1);
        if (index >= count) {
          return 0;
        }
        sub_object_type_id =
            should_be_composite_type->GetSingleWordInOperand(0);
        break;
      }
      case SpvOpTypeStruct: {
        if (index >= GetNumberOfStructMembers(*should_be_composite_type)) {
          return 0;
        }
        sub_object_type_id =
            should_be_composite_type->GetSingleWordInOperand(index);
        break;
      }
      default:
        return 0;
    }
  }
  return sub_object_type_id;
}

uint32_t GetNumberOfStructMembers(
    const opt::Instruction& struct_type_instruction) {
  assert(struct_type_instruction.opcode() == SpvOpTypeStruct &&
         "An OpTypeStruct instruction is required here.");
  return struct_type_instruction.NumInOperands();
}

uint32_t GetArraySize(const opt::Instruction& array_type_instruction,
                      opt::IRContext* context) {
  auto array_length_constant =
      context->get_constant_mgr()
          ->GetConstantFromInst(context->get_def_use_mgr()->GetDef(
              array_type_instruction.GetSingleWordInOperand(1)))
          ->AsIntConstant();
  if (array_length_constant->words().size() != 1) {
    return 0;
  }
  return array_length_constant->GetU32();
}

bool IsValid(opt::IRContext* context) {
  std::vector<uint32_t> binary;
  context->module()->ToBinary(&binary, false);
  SpirvTools tools(context->grammar().target_env());
  return tools.Validate(binary);
}

std::unique_ptr<opt::IRContext> CloneIRContext(opt::IRContext* context) {
  std::vector<uint32_t> binary;
  context->module()->ToBinary(&binary, false);
  return BuildModule(context->grammar().target_env(), nullptr, binary.data(),
                     binary.size());
}

bool IsNonFunctionTypeId(opt::IRContext* ir_context, uint32_t id) {
  auto type = ir_context->get_type_mgr()->GetType(id);
  return type && !type->AsFunction();
}

bool IsMergeOrContinue(opt::IRContext* ir_context, uint32_t block_id) {
  bool result = false;
  ir_context->get_def_use_mgr()->WhileEachUse(
      block_id,
      [&result](const opt::Instruction* use_instruction,
                uint32_t /*unused*/) -> bool {
        switch (use_instruction->opcode()) {
          case SpvOpLoopMerge:
          case SpvOpSelectionMerge:
            result = true;
            return false;
          default:
            return true;
        }
      });
  return result;
}

uint32_t FindFunctionType(opt::IRContext* ir_context,
                          const std::vector<uint32_t>& type_ids) {
  // Look through the existing types for a match.
  for (auto& type_or_value : ir_context->types_values()) {
    if (type_or_value.opcode() != SpvOpTypeFunction) {
      // We are only interested in function types.
      continue;
    }
    if (type_or_value.NumInOperands() != type_ids.size()) {
      // Not a match: different numbers of arguments.
      continue;
    }
    // Check whether the return type and argument types match.
    bool input_operands_match = true;
    for (uint32_t i = 0; i < type_or_value.NumInOperands(); i++) {
      if (type_ids[i] != type_or_value.GetSingleWordInOperand(i)) {
        input_operands_match = false;
        break;
      }
    }
    if (input_operands_match) {
      // Everything matches.
      return type_or_value.result_id();
    }
  }
  // No match was found.
  return 0;
}

}  // namespace fuzzerutil

}  // namespace fuzz
}  // namespace spvtools
