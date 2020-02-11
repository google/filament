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

#include "source/opt/desc_sroa.h"

#include "source/util/string_utils.h"

namespace spvtools {
namespace opt {

Pass::Status DescriptorScalarReplacement::Process() {
  bool modified = false;

  std::vector<Instruction*> vars_to_kill;

  for (Instruction& var : context()->types_values()) {
    if (IsCandidate(&var)) {
      modified = true;
      if (!ReplaceCandidate(&var)) {
        return Status::Failure;
      }
      vars_to_kill.push_back(&var);
    }
  }

  for (Instruction* var : vars_to_kill) {
    context()->KillInst(var);
  }

  return (modified ? Status::SuccessWithChange : Status::SuccessWithoutChange);
}

bool DescriptorScalarReplacement::IsCandidate(Instruction* var) {
  if (var->opcode() != SpvOpVariable) {
    return false;
  }

  uint32_t ptr_type_id = var->type_id();
  Instruction* ptr_type_inst =
      context()->get_def_use_mgr()->GetDef(ptr_type_id);
  if (ptr_type_inst->opcode() != SpvOpTypePointer) {
    return false;
  }

  uint32_t var_type_id = ptr_type_inst->GetSingleWordInOperand(1);
  Instruction* var_type_inst =
      context()->get_def_use_mgr()->GetDef(var_type_id);
  if (var_type_inst->opcode() != SpvOpTypeArray) {
    return false;
  }

  bool has_desc_set_decoration = false;
  context()->get_decoration_mgr()->ForEachDecoration(
      var->result_id(), SpvDecorationDescriptorSet,
      [&has_desc_set_decoration](const Instruction&) {
        has_desc_set_decoration = true;
      });
  if (!has_desc_set_decoration) {
    return false;
  }

  bool has_binding_decoration = false;
  context()->get_decoration_mgr()->ForEachDecoration(
      var->result_id(), SpvDecorationBinding,
      [&has_binding_decoration](const Instruction&) {
        has_binding_decoration = true;
      });
  if (!has_binding_decoration) {
    return false;
  }

  return true;
}

bool DescriptorScalarReplacement::ReplaceCandidate(Instruction* var) {
  std::vector<Instruction*> work_list;
  bool failed = !get_def_use_mgr()->WhileEachUser(
      var->result_id(), [this, &work_list](Instruction* use) {
        if (use->opcode() == SpvOpName) {
          return true;
        }

        if (use->IsDecoration()) {
          return true;
        }

        switch (use->opcode()) {
          case SpvOpAccessChain:
          case SpvOpInBoundsAccessChain:
            work_list.push_back(use);
            return true;
          default:
            context()->EmitErrorMessage(
                "Variable cannot be replaced: invalid instruction", use);
            return false;
        }
        return true;
      });

  if (failed) {
    return false;
  }

  for (Instruction* use : work_list) {
    if (!ReplaceAccessChain(var, use)) {
      return false;
    }
  }
  return true;
}

bool DescriptorScalarReplacement::ReplaceAccessChain(Instruction* var,
                                                     Instruction* use) {
  if (use->NumInOperands() <= 1) {
    context()->EmitErrorMessage(
        "Variable cannot be replaced: invalid instruction", use);
    return false;
  }

  uint32_t idx_id = use->GetSingleWordInOperand(1);
  const analysis::Constant* idx_const =
      context()->get_constant_mgr()->FindDeclaredConstant(idx_id);
  if (idx_const == nullptr) {
    context()->EmitErrorMessage("Variable cannot be replaced: invalid index",
                                use);
    return false;
  }

  uint32_t idx = idx_const->GetU32();
  uint32_t replacement_var = GetReplacementVariable(var, idx);

  if (use->NumInOperands() == 2) {
    // We are not indexing into the replacement variable.  We can replaces the
    // access chain with the replacement varibale itself.
    context()->ReplaceAllUsesWith(use->result_id(), replacement_var);
    context()->KillInst(use);
    return true;
  }

  // We need to build a new access chain with the replacement variable as the
  // base address.
  Instruction::OperandList new_operands;

  // Same result id and result type.
  new_operands.emplace_back(use->GetOperand(0));
  new_operands.emplace_back(use->GetOperand(1));

  // Use the replacement variable as the base address.
  new_operands.push_back({SPV_OPERAND_TYPE_ID, {replacement_var}});

  // Drop the first index because it is consumed by the replacment, and copy the
  // rest.
  for (uint32_t i = 4; i < use->NumOperands(); i++) {
    new_operands.emplace_back(use->GetOperand(i));
  }

  use->ReplaceOperands(new_operands);
  context()->UpdateDefUse(use);
  return true;
}

uint32_t DescriptorScalarReplacement::GetReplacementVariable(Instruction* var,
                                                             uint32_t idx) {
  auto replacement_vars = replacement_variables_.find(var);
  if (replacement_vars == replacement_variables_.end()) {
    uint32_t ptr_type_id = var->type_id();
    Instruction* ptr_type_inst = get_def_use_mgr()->GetDef(ptr_type_id);
    assert(ptr_type_inst->opcode() == SpvOpTypePointer &&
           "Variable should be a pointer to an array.");
    uint32_t arr_type_id = ptr_type_inst->GetSingleWordInOperand(1);
    Instruction* arr_type_inst = get_def_use_mgr()->GetDef(arr_type_id);
    assert(arr_type_inst->opcode() == SpvOpTypeArray &&
           "Variable should be a pointer to an array.");

    uint32_t array_len_id = arr_type_inst->GetSingleWordInOperand(1);
    const analysis::Constant* array_len_const =
        context()->get_constant_mgr()->FindDeclaredConstant(array_len_id);
    assert(array_len_const != nullptr && "Array length must be a constant.");
    uint32_t array_len = array_len_const->GetU32();

    replacement_vars = replacement_variables_
                           .insert({var, std::vector<uint32_t>(array_len, 0)})
                           .first;
  }

  if (replacement_vars->second[idx] == 0) {
    replacement_vars->second[idx] = CreateReplacementVariable(var, idx);
  }

  return replacement_vars->second[idx];
}

uint32_t DescriptorScalarReplacement::CreateReplacementVariable(
    Instruction* var, uint32_t idx) {
  // The storage class for the new variable is the same as the original.
  SpvStorageClass storage_class =
      static_cast<SpvStorageClass>(var->GetSingleWordInOperand(0));

  // The type for the new variable will be a pointer to type of the elements of
  // the array.
  uint32_t ptr_type_id = var->type_id();
  Instruction* ptr_type_inst = get_def_use_mgr()->GetDef(ptr_type_id);
  assert(ptr_type_inst->opcode() == SpvOpTypePointer &&
         "Variable should be a pointer to an array.");
  uint32_t arr_type_id = ptr_type_inst->GetSingleWordInOperand(1);
  Instruction* arr_type_inst = get_def_use_mgr()->GetDef(arr_type_id);
  assert(arr_type_inst->opcode() == SpvOpTypeArray &&
         "Variable should be a pointer to an array.");
  uint32_t element_type_id = arr_type_inst->GetSingleWordInOperand(0);

  uint32_t ptr_element_type_id = context()->get_type_mgr()->FindPointerToType(
      element_type_id, storage_class);

  // Create the variable.
  uint32_t id = TakeNextId();
  std::unique_ptr<Instruction> variable(
      new Instruction(context(), SpvOpVariable, ptr_element_type_id, id,
                      std::initializer_list<Operand>{
                          {SPV_OPERAND_TYPE_STORAGE_CLASS,
                           {static_cast<uint32_t>(storage_class)}}}));
  context()->AddGlobalValue(std::move(variable));

  // Copy all of the decorations to the new variable.  The only difference is
  // the Binding decoration needs to be adjusted.
  for (auto old_decoration :
       get_decoration_mgr()->GetDecorationsFor(var->result_id(), true)) {
    assert(old_decoration->opcode() == SpvOpDecorate);
    std::unique_ptr<Instruction> new_decoration(
        old_decoration->Clone(context()));
    new_decoration->SetInOperand(0, {id});

    uint32_t decoration = new_decoration->GetSingleWordInOperand(1u);
    if (decoration == SpvDecorationBinding) {
      uint32_t new_binding = new_decoration->GetSingleWordInOperand(2) + idx;
      new_decoration->SetInOperand(2, {new_binding});
    }
    context()->AddAnnotationInst(std::move(new_decoration));
  }

  // Create a new OpName for the replacement variable.
  for (auto p : context()->GetNames(var->result_id())) {
    Instruction* name_inst = p.second;
    std::string name_str = utils::MakeString(name_inst->GetOperand(1).words);
    name_str += "[";
    name_str += utils::ToString(idx);
    name_str += "]";

    std::unique_ptr<Instruction> new_name(new Instruction(
        context(), SpvOpName, 0, 0,
        std::initializer_list<Operand>{
            {SPV_OPERAND_TYPE_ID, {id}},
            {SPV_OPERAND_TYPE_LITERAL_STRING, utils::MakeVector(name_str)}}));
    Instruction* new_name_inst = new_name.get();
    context()->AddDebug2Inst(std::move(new_name));
    get_def_use_mgr()->AnalyzeInstDefUse(new_name_inst);
  }

  return id;
}

}  // namespace opt
}  // namespace spvtools
