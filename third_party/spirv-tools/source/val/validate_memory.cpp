// Copyright (c) 2018 Google LLC.
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

#include "source/val/validate.h"

#include <algorithm>
#include <string>
#include <vector>

#include "source/opcode.h"
#include "source/val/instruction.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

bool AreLayoutCompatibleStructs(ValidationState_t&, const Instruction*,
                                const Instruction*);
bool HaveLayoutCompatibleMembers(ValidationState_t&, const Instruction*,
                                 const Instruction*);
bool HaveSameLayoutDecorations(ValidationState_t&, const Instruction*,
                               const Instruction*);
bool HasConflictingMemberOffsets(const std::vector<Decoration>&,
                                 const std::vector<Decoration>&);

// Returns true if the two instructions represent structs that, as far as the
// validator can tell, have the exact same data layout.
bool AreLayoutCompatibleStructs(ValidationState_t& _, const Instruction* type1,
                                const Instruction* type2) {
  if (type1->opcode() != SpvOpTypeStruct) {
    return false;
  }
  if (type2->opcode() != SpvOpTypeStruct) {
    return false;
  }

  if (!HaveLayoutCompatibleMembers(_, type1, type2)) return false;

  return HaveSameLayoutDecorations(_, type1, type2);
}

// Returns true if the operands to the OpTypeStruct instruction defining the
// types are the same or are layout compatible types. |type1| and |type2| must
// be OpTypeStruct instructions.
bool HaveLayoutCompatibleMembers(ValidationState_t& _, const Instruction* type1,
                                 const Instruction* type2) {
  assert(type1->opcode() == SpvOpTypeStruct &&
         "type1 must be and OpTypeStruct instruction.");
  assert(type2->opcode() == SpvOpTypeStruct &&
         "type2 must be and OpTypeStruct instruction.");
  const auto& type1_operands = type1->operands();
  const auto& type2_operands = type2->operands();
  if (type1_operands.size() != type2_operands.size()) {
    return false;
  }

  for (size_t operand = 2; operand < type1_operands.size(); ++operand) {
    if (type1->word(operand) != type2->word(operand)) {
      auto def1 = _.FindDef(type1->word(operand));
      auto def2 = _.FindDef(type2->word(operand));
      if (!AreLayoutCompatibleStructs(_, def1, def2)) {
        return false;
      }
    }
  }
  return true;
}

// Returns true if all decorations that affect the data layout of the struct
// (like Offset), are the same for the two types. |type1| and |type2| must be
// OpTypeStruct instructions.
bool HaveSameLayoutDecorations(ValidationState_t& _, const Instruction* type1,
                               const Instruction* type2) {
  assert(type1->opcode() == SpvOpTypeStruct &&
         "type1 must be and OpTypeStruct instruction.");
  assert(type2->opcode() == SpvOpTypeStruct &&
         "type2 must be and OpTypeStruct instruction.");
  const std::vector<Decoration>& type1_decorations =
      _.id_decorations(type1->id());
  const std::vector<Decoration>& type2_decorations =
      _.id_decorations(type2->id());

  // TODO: Will have to add other check for arrays an matricies if we want to
  // handle them.
  if (HasConflictingMemberOffsets(type1_decorations, type2_decorations)) {
    return false;
  }

  return true;
}

bool HasConflictingMemberOffsets(
    const std::vector<Decoration>& type1_decorations,
    const std::vector<Decoration>& type2_decorations) {
  {
    // We are interested in conflicting decoration.  If a decoration is in one
    // list but not the other, then we will assume the code is correct.  We are
    // looking for things we know to be wrong.
    //
    // We do not have to traverse type2_decoration because, after traversing
    // type1_decorations, anything new will not be found in
    // type1_decoration.  Therefore, it cannot lead to a conflict.
    for (const Decoration& decoration : type1_decorations) {
      switch (decoration.dec_type()) {
        case SpvDecorationOffset: {
          // Since these affect the layout of the struct, they must be present
          // in both structs.
          auto compare = [&decoration](const Decoration& rhs) {
            if (rhs.dec_type() != SpvDecorationOffset) return false;
            return decoration.struct_member_index() ==
                   rhs.struct_member_index();
          };
          auto i = std::find_if(type2_decorations.begin(),
                                type2_decorations.end(), compare);
          if (i != type2_decorations.end() &&
              decoration.params().front() != i->params().front()) {
            return true;
          }
        } break;
        default:
          // This decoration does not affect the layout of the structure, so
          // just moving on.
          break;
      }
    }
  }
  return false;
}

spv_result_t ValidateVariable(ValidationState_t& _, const Instruction& inst) {
  auto result_type = _.FindDef(inst.type_id());
  if (!result_type || result_type->opcode() != SpvOpTypePointer) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "OpVariable Result Type <id> '" << _.getIdName(inst.type_id())
           << "' is not a pointer type.";
  }

  const auto initializer_index = 3;
  if (initializer_index < inst.operands().size()) {
    const auto initializer_id = inst.GetOperandAs<uint32_t>(initializer_index);
    const auto initializer = _.FindDef(initializer_id);
    const auto storage_class_index = 2;
    const auto is_module_scope_var =
        initializer && (initializer->opcode() == SpvOpVariable) &&
        (initializer->GetOperandAs<uint32_t>(storage_class_index) !=
         SpvStorageClassFunction);
    const auto is_constant =
        initializer && spvOpcodeIsConstant(initializer->opcode());
    if (!initializer || !(is_constant || is_module_scope_var)) {
      return _.diag(SPV_ERROR_INVALID_ID, &inst)
             << "OpVariable Initializer <id> '" << _.getIdName(initializer_id)
             << "' is not a constant or module-scope variable.";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateLoad(ValidationState_t& _, const Instruction& inst) {
  const auto result_type = _.FindDef(inst.type_id());
  if (!result_type) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "OpLoad Result Type <id> '" << _.getIdName(inst.type_id())
           << "' is not defined.";
  }

  const bool uses_variable_pointers =
      _.features().variable_pointers ||
      _.features().variable_pointers_storage_buffer;
  const auto pointer_index = 2;
  const auto pointer_id = inst.GetOperandAs<uint32_t>(pointer_index);
  const auto pointer = _.FindDef(pointer_id);
  if (!pointer ||
      ((_.addressing_model() == SpvAddressingModelLogical) &&
       ((!uses_variable_pointers &&
         !spvOpcodeReturnsLogicalPointer(pointer->opcode())) ||
        (uses_variable_pointers &&
         !spvOpcodeReturnsLogicalVariablePointer(pointer->opcode()))))) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "OpLoad Pointer <id> '" << _.getIdName(pointer_id)
           << "' is not a logical pointer.";
  }

  const auto pointer_type = _.FindDef(pointer->type_id());
  if (!pointer_type || pointer_type->opcode() != SpvOpTypePointer) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "OpLoad type for pointer <id> '" << _.getIdName(pointer_id)
           << "' is not a pointer type.";
  }

  const auto pointee_type = _.FindDef(pointer_type->GetOperandAs<uint32_t>(2));
  if (!pointee_type || result_type->id() != pointee_type->id()) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "OpLoad Result Type <id> '" << _.getIdName(inst.type_id())
           << "' does not match Pointer <id> '" << _.getIdName(pointer->id())
           << "'s type.";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateStore(ValidationState_t& _, const Instruction& inst) {
  const bool uses_variable_pointer =
      _.features().variable_pointers ||
      _.features().variable_pointers_storage_buffer;
  const auto pointer_index = 0;
  const auto pointer_id = inst.GetOperandAs<uint32_t>(pointer_index);
  const auto pointer = _.FindDef(pointer_id);
  if (!pointer ||
      (_.addressing_model() == SpvAddressingModelLogical &&
       ((!uses_variable_pointer &&
         !spvOpcodeReturnsLogicalPointer(pointer->opcode())) ||
        (uses_variable_pointer &&
         !spvOpcodeReturnsLogicalVariablePointer(pointer->opcode()))))) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "OpStore Pointer <id> '" << _.getIdName(pointer_id)
           << "' is not a logical pointer.";
  }
  const auto pointer_type = _.FindDef(pointer->type_id());
  if (!pointer_type || pointer_type->opcode() != SpvOpTypePointer) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "OpStore type for pointer <id> '" << _.getIdName(pointer_id)
           << "' is not a pointer type.";
  }
  const auto type_id = pointer_type->GetOperandAs<uint32_t>(2);
  const auto type = _.FindDef(type_id);
  if (!type || SpvOpTypeVoid == type->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "OpStore Pointer <id> '" << _.getIdName(pointer_id)
           << "'s type is void.";
  }

  // validate storage class
  {
    uint32_t data_type;
    uint32_t storage_class;
    if (!_.GetPointerTypeInfo(pointer_type->id(), &data_type, &storage_class)) {
      return _.diag(SPV_ERROR_INVALID_ID, &inst)
             << "OpStore Pointer <id> '" << _.getIdName(pointer_id)
             << "' is not pointer type";
    }

    if (storage_class == SpvStorageClassUniformConstant ||
        storage_class == SpvStorageClassInput ||
        storage_class == SpvStorageClassPushConstant) {
      return _.diag(SPV_ERROR_INVALID_ID, &inst)
             << "OpStore Pointer <id> '" << _.getIdName(pointer_id)
             << "' storage class is read-only";
    }
  }

  const auto object_index = 1;
  const auto object_id = inst.GetOperandAs<uint32_t>(object_index);
  const auto object = _.FindDef(object_id);
  if (!object || !object->type_id()) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "OpStore Object <id> '" << _.getIdName(object_id)
           << "' is not an object.";
  }
  const auto object_type = _.FindDef(object->type_id());
  if (!object_type || SpvOpTypeVoid == object_type->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "OpStore Object <id> '" << _.getIdName(object_id)
           << "'s type is void.";
  }

  if (type->id() != object_type->id()) {
    if (!_.options()->relax_struct_store || type->opcode() != SpvOpTypeStruct ||
        object_type->opcode() != SpvOpTypeStruct) {
      return _.diag(SPV_ERROR_INVALID_ID, &inst)
             << "OpStore Pointer <id> '" << _.getIdName(pointer_id)
             << "'s type does not match Object <id> '"
             << _.getIdName(object->id()) << "'s type.";
    }

    // TODO: Check for layout compatible matricies and arrays as well.
    if (!AreLayoutCompatibleStructs(_, type, object_type)) {
      return _.diag(SPV_ERROR_INVALID_ID, &inst)
             << "OpStore Pointer <id> '" << _.getIdName(pointer_id)
             << "'s layout does not match Object <id> '"
             << _.getIdName(object->id()) << "'s layout.";
    }
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateCopyMemory(ValidationState_t& _, const Instruction& inst) {
  const auto target_index = 0;
  const auto target_id = inst.GetOperandAs<uint32_t>(target_index);
  const auto target = _.FindDef(target_id);
  if (!target) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "Target operand <id> '" << _.getIdName(target_id)
           << "' is not defined.";
  }

  const auto source_index = 1;
  const auto source_id = inst.GetOperandAs<uint32_t>(source_index);
  const auto source = _.FindDef(source_id);
  if (!source) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "Source operand <id> '" << _.getIdName(source_id)
           << "' is not defined.";
  }

  const auto target_pointer_type = _.FindDef(target->type_id());
  if (!target_pointer_type ||
      target_pointer_type->opcode() != SpvOpTypePointer) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "Target operand <id> '" << _.getIdName(target_id)
           << "' is not a pointer.";
  }

  const auto source_pointer_type = _.FindDef(source->type_id());
  if (!source_pointer_type ||
      source_pointer_type->opcode() != SpvOpTypePointer) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "Source operand <id> '" << _.getIdName(source_id)
           << "' is not a pointer.";
  }

  if (inst.opcode() == SpvOpCopyMemory) {
    const auto target_type =
        _.FindDef(target_pointer_type->GetOperandAs<uint32_t>(2));
    if (!target_type || target_type->opcode() == SpvOpTypeVoid) {
      return _.diag(SPV_ERROR_INVALID_ID, &inst)
             << "Target operand <id> '" << _.getIdName(target_id)
             << "' cannot be a void pointer.";
    }

    const auto source_type =
        _.FindDef(source_pointer_type->GetOperandAs<uint32_t>(2));
    if (!source_type || source_type->opcode() == SpvOpTypeVoid) {
      return _.diag(SPV_ERROR_INVALID_ID, &inst)
             << "Source operand <id> '" << _.getIdName(source_id)
             << "' cannot be a void pointer.";
    }

    if (target_type->id() != source_type->id()) {
      return _.diag(SPV_ERROR_INVALID_ID, &inst)
             << "Target <id> '" << _.getIdName(source_id)
             << "'s type does not match Source <id> '"
             << _.getIdName(source_type->id()) << "'s type.";
    }
  } else {
    const auto size_id = inst.GetOperandAs<uint32_t>(2);
    const auto size = _.FindDef(size_id);
    if (!size) {
      return _.diag(SPV_ERROR_INVALID_ID, &inst)
             << "Size operand <id> '" << _.getIdName(size_id)
             << "' is not defined.";
    }

    const auto size_type = _.FindDef(size->type_id());
    if (!_.IsIntScalarType(size_type->id())) {
      return _.diag(SPV_ERROR_INVALID_ID, &inst)
             << "Size operand <id> '" << _.getIdName(size_id)
             << "' must be a scalar integer type.";
    }

    bool is_zero = true;
    switch (size->opcode()) {
      case SpvOpConstantNull:
        return _.diag(SPV_ERROR_INVALID_ID, &inst)
               << "Size operand <id> '" << _.getIdName(size_id)
               << "' cannot be a constant zero.";
      case SpvOpConstant:
        if (size_type->word(3) == 1 &&
            size->word(size->words().size() - 1) & 0x80000000) {
          return _.diag(SPV_ERROR_INVALID_ID, &inst)
                 << "Size operand <id> '" << _.getIdName(size_id)
                 << "' cannot have the sign bit set to 1.";
        }
        for (size_t i = 3; is_zero && i < size->words().size(); ++i) {
          is_zero &= (size->word(i) == 0);
        }
        if (is_zero) {
          return _.diag(SPV_ERROR_INVALID_ID, &inst)
                 << "Size operand <id> '" << _.getIdName(size_id)
                 << "' cannot be a constant zero.";
        }
        break;
      default:
        // Cannot infer any other opcodes.
        break;
    }
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateAccessChain(ValidationState_t& _,
                                 const Instruction& inst) {
  std::string instr_name =
      "Op" + std::string(spvOpcodeString(static_cast<SpvOp>(inst.opcode())));

  // The result type must be OpTypePointer.
  auto result_type = _.FindDef(inst.type_id());
  if (SpvOpTypePointer != result_type->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "The Result Type of " << instr_name << " <id> '"
           << _.getIdName(inst.id()) << "' must be OpTypePointer. Found Op"
           << spvOpcodeString(static_cast<SpvOp>(result_type->opcode())) << ".";
  }

  // Result type is a pointer. Find out what it's pointing to.
  // This will be used to make sure the indexing results in the same type.
  // OpTypePointer word 3 is the type being pointed to.
  const auto result_type_pointee = _.FindDef(result_type->word(3));

  // Base must be a pointer, pointing to the base of a composite object.
  const auto base_index = 2;
  const auto base_id = inst.GetOperandAs<uint32_t>(base_index);
  const auto base = _.FindDef(base_id);
  const auto base_type = _.FindDef(base->type_id());
  if (!base_type || SpvOpTypePointer != base_type->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "The Base <id> '" << _.getIdName(base_id) << "' in " << instr_name
           << " instruction must be a pointer.";
  }

  // The result pointer storage class and base pointer storage class must match.
  // Word 2 of OpTypePointer is the Storage Class.
  auto result_type_storage_class = result_type->word(2);
  auto base_type_storage_class = base_type->word(2);
  if (result_type_storage_class != base_type_storage_class) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "The result pointer storage class and base "
              "pointer storage class in "
           << instr_name << " do not match.";
  }

  // The type pointed to by OpTypePointer (word 3) must be a composite type.
  auto type_pointee = _.FindDef(base_type->word(3));

  // Check Universal Limit (SPIR-V Spec. Section 2.17).
  // The number of indexes passed to OpAccessChain may not exceed 255
  // The instruction includes 4 words + N words (for N indexes)
  size_t num_indexes = inst.words().size() - 4;
  if (inst.opcode() == SpvOpPtrAccessChain ||
      inst.opcode() == SpvOpInBoundsPtrAccessChain) {
    // In pointer access chains, the element operand is required, but not
    // counted as an index.
    --num_indexes;
  }
  const size_t num_indexes_limit =
      _.options()->universal_limits_.max_access_chain_indexes;
  if (num_indexes > num_indexes_limit) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << "The number of indexes in " << instr_name << " may not exceed "
           << num_indexes_limit << ". Found " << num_indexes << " indexes.";
  }
  // Indexes walk the type hierarchy to the desired depth, potentially down to
  // scalar granularity. The first index in Indexes will select the top-level
  // member/element/component/element of the base composite. All composite
  // constituents use zero-based numbering, as described by their OpType...
  // instruction. The second index will apply similarly to that result, and so
  // on. Once any non-composite type is reached, there must be no remaining
  // (unused) indexes.
  auto starting_index = 4;
  if (inst.opcode() == SpvOpPtrAccessChain ||
      inst.opcode() == SpvOpInBoundsPtrAccessChain) {
    ++starting_index;
  }
  for (size_t i = starting_index; i < inst.words().size(); ++i) {
    const uint32_t cur_word = inst.words()[i];
    // Earlier ID checks ensure that cur_word definition exists.
    auto cur_word_instr = _.FindDef(cur_word);
    // The index must be a scalar integer type (See OpAccessChain in the Spec.)
    auto index_type = _.FindDef(cur_word_instr->type_id());
    if (!index_type || SpvOpTypeInt != index_type->opcode()) {
      return _.diag(SPV_ERROR_INVALID_ID, &inst)
             << "Indexes passed to " << instr_name
             << " must be of type integer.";
    }
    switch (type_pointee->opcode()) {
      case SpvOpTypeMatrix:
      case SpvOpTypeVector:
      case SpvOpTypeArray:
      case SpvOpTypeRuntimeArray: {
        // In OpTypeMatrix, OpTypeVector, OpTypeArray, and OpTypeRuntimeArray,
        // word 2 is the Element Type.
        type_pointee = _.FindDef(type_pointee->word(2));
        break;
      }
      case SpvOpTypeStruct: {
        // In case of structures, there is an additional constraint on the
        // index: the index must be an OpConstant.
        if (SpvOpConstant != cur_word_instr->opcode()) {
          return _.diag(SPV_ERROR_INVALID_ID, cur_word_instr)
                 << "The <id> passed to " << instr_name
                 << " to index into a "
                    "structure must be an OpConstant.";
        }
        // Get the index value from the OpConstant (word 3 of OpConstant).
        // OpConstant could be a signed integer. But it's okay to treat it as
        // unsigned because a negative constant int would never be seen as
        // correct as a struct offset, since structs can't have more than 2
        // billion members.
        const uint32_t cur_index = cur_word_instr->word(3);
        // The index points to the struct member we want, therefore, the index
        // should be less than the number of struct members.
        const uint32_t num_struct_members =
            static_cast<uint32_t>(type_pointee->words().size() - 2);
        if (cur_index >= num_struct_members) {
          return _.diag(SPV_ERROR_INVALID_ID, cur_word_instr)
                 << "Index is out of bounds: " << instr_name
                 << " can not find index " << cur_index
                 << " into the structure <id> '"
                 << _.getIdName(type_pointee->id()) << "'. This structure has "
                 << num_struct_members << " members. Largest valid index is "
                 << num_struct_members - 1 << ".";
        }
        // Struct members IDs start at word 2 of OpTypeStruct.
        auto structMemberId = type_pointee->word(cur_index + 2);
        type_pointee = _.FindDef(structMemberId);
        break;
      }
      default: {
        // Give an error. reached non-composite type while indexes still remain.
        return _.diag(SPV_ERROR_INVALID_ID, cur_word_instr)
               << instr_name
               << " reached non-composite type while indexes "
                  "still remain to be traversed.";
      }
    }
  }
  // At this point, we have fully walked down from the base using the indeces.
  // The type being pointed to should be the same as the result type.
  if (type_pointee->id() != result_type_pointee->id()) {
    return _.diag(SPV_ERROR_INVALID_ID, &inst)
           << instr_name << " result type (Op"
           << spvOpcodeString(static_cast<SpvOp>(result_type_pointee->opcode()))
           << ") does not match the type that results from indexing into the "
              "base "
              "<id> (Op"
           << spvOpcodeString(static_cast<SpvOp>(type_pointee->opcode()))
           << ").";
  }

  return SPV_SUCCESS;
}

}  // namespace

spv_result_t ValidateMemoryInstructions(ValidationState_t& _,
                                        const Instruction* inst) {
  switch (inst->opcode()) {
    case SpvOpVariable:
      if (auto error = ValidateVariable(_, *inst)) return error;
      break;
    case SpvOpLoad:
      if (auto error = ValidateLoad(_, *inst)) return error;
      break;
    case SpvOpStore:
      if (auto error = ValidateStore(_, *inst)) return error;
      break;
    case SpvOpCopyMemory:
    case SpvOpCopyMemorySized:
      if (auto error = ValidateCopyMemory(_, *inst)) return error;
      break;
    case SpvOpAccessChain:
    case SpvOpInBoundsAccessChain:
    case SpvOpPtrAccessChain:
    case SpvOpInBoundsPtrAccessChain:
      if (auto error = ValidateAccessChain(_, *inst)) return error;
      break;
    case SpvOpImageTexelPointer:
    case SpvOpArrayLength:
    case SpvOpGenericPtrMemSemantics:
    default:
      break;
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
