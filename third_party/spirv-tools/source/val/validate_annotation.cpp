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

#include "source/opcode.h"
#include "source/val/instruction.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

spv_result_t ValidateDecorate(ValidationState_t& _, const Instruction* inst) {
  const auto decoration = inst->GetOperandAs<uint32_t>(1);
  if (decoration == SpvDecorationSpecId) {
    const auto target_id = inst->GetOperandAs<uint32_t>(0);
    const auto target = _.FindDef(target_id);
    if (!target || !spvOpcodeIsScalarSpecConstant(target->opcode())) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpDecorate SpecId decoration target <id> '"
             << _.getIdName(decoration)
             << "' is not a scalar specialization constant.";
    }
  }
  // TODO: Add validations for all decorations.
  return SPV_SUCCESS;
}

spv_result_t ValidateMemberDecorate(ValidationState_t& _,
                                    const Instruction* inst) {
  const auto struct_type_id = inst->GetOperandAs<uint32_t>(0);
  const auto struct_type = _.FindDef(struct_type_id);
  if (!struct_type || SpvOpTypeStruct != struct_type->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpMemberDecorate Structure type <id> '"
           << _.getIdName(struct_type_id) << "' is not a struct type.";
  }
  const auto member = inst->GetOperandAs<uint32_t>(1);
  const auto member_count =
      static_cast<uint32_t>(struct_type->words().size() - 2);
  if (member_count < member) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Index " << member
           << " provided in OpMemberDecorate for struct <id> "
           << _.getIdName(struct_type_id)
           << " is out of bounds. The structure has " << member_count
           << " members. Largest valid index is " << member_count - 1 << ".";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateDecorationGroup(ValidationState_t& _,
                                     const Instruction* inst) {
  const auto decoration_group_id = inst->GetOperandAs<uint32_t>(0);
  const auto decoration_group = _.FindDef(decoration_group_id);
  for (auto pair : decoration_group->uses()) {
    auto use = pair.first;
    if (use->opcode() != SpvOpDecorate && use->opcode() != SpvOpGroupDecorate &&
        use->opcode() != SpvOpGroupMemberDecorate &&
        use->opcode() != SpvOpName) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Result id of OpDecorationGroup can only "
             << "be targeted by OpName, OpGroupDecorate, "
             << "OpDecorate, and OpGroupMemberDecorate";
    }
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateGroupDecorate(ValidationState_t& _,
                                   const Instruction* inst) {
  const auto decoration_group_id = inst->GetOperandAs<uint32_t>(0);
  auto decoration_group = _.FindDef(decoration_group_id);
  if (!decoration_group || SpvOpDecorationGroup != decoration_group->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpGroupDecorate Decoration group <id> '"
           << _.getIdName(decoration_group_id)
           << "' is not a decoration group.";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateGroupMemberDecorate(ValidationState_t& _,
                                         const Instruction* inst) {
  const auto decoration_group_id = inst->GetOperandAs<uint32_t>(0);
  const auto decoration_group = _.FindDef(decoration_group_id);
  if (!decoration_group || SpvOpDecorationGroup != decoration_group->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpGroupMemberDecorate Decoration group <id> '"
           << _.getIdName(decoration_group_id)
           << "' is not a decoration group.";
  }
  // Grammar checks ensures that the number of arguments to this instruction
  // is an odd number: 1 decoration group + (id,literal) pairs.
  for (size_t i = 1; i + 1 < inst->operands().size(); i += 2) {
    const uint32_t struct_id = inst->GetOperandAs<uint32_t>(i);
    const uint32_t index = inst->GetOperandAs<uint32_t>(i + 1);
    auto struct_instr = _.FindDef(struct_id);
    if (!struct_instr || SpvOpTypeStruct != struct_instr->opcode()) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpGroupMemberDecorate Structure type <id> '"
             << _.getIdName(struct_id) << "' is not a struct type.";
    }
    const uint32_t num_struct_members =
        static_cast<uint32_t>(struct_instr->words().size() - 2);
    if (index >= num_struct_members) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Index " << index
             << " provided in OpGroupMemberDecorate for struct <id> "
             << _.getIdName(struct_id)
             << " is out of bounds. The structure has " << num_struct_members
             << " members. Largest valid index is " << num_struct_members - 1
             << ".";
    }
  }
  return SPV_SUCCESS;
}

}  // namespace

spv_result_t AnnotationPass(ValidationState_t& _, const Instruction* inst) {
  switch (inst->opcode()) {
    case SpvOpDecorate:
      if (auto error = ValidateDecorate(_, inst)) return error;
      break;
    case SpvOpMemberDecorate:
      if (auto error = ValidateMemberDecorate(_, inst)) return error;
      break;
    case SpvOpDecorationGroup:
      if (auto error = ValidateDecorationGroup(_, inst)) return error;
      break;
    case SpvOpGroupDecorate:
      if (auto error = ValidateGroupDecorate(_, inst)) return error;
      break;
    case SpvOpGroupMemberDecorate:
      if (auto error = ValidateGroupMemberDecorate(_, inst)) return error;
      break;
    default:
      break;
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
