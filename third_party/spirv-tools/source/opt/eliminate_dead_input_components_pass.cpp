// Copyright (c) 2022 The Khronos Group Inc.
// Copyright (c) 2022 LunarG Inc.
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

#include "source/opt/eliminate_dead_input_components_pass.h"

#include <set>
#include <vector>

#include "source/opt/instruction.h"
#include "source/opt/ir_builder.h"
#include "source/opt/ir_context.h"
#include "source/util/bit_vector.h"

namespace {

const uint32_t kAccessChainBaseInIdx = 0;
const uint32_t kAccessChainIndex0InIdx = 1;
const uint32_t kConstantValueInIdx = 0;
const uint32_t kVariableStorageClassInIdx = 0;

}  // namespace

namespace spvtools {
namespace opt {

Pass::Status EliminateDeadInputComponentsPass::Process() {
  // Current functionality assumes shader capability
  if (!context()->get_feature_mgr()->HasCapability(SpvCapabilityShader))
    return Status::SuccessWithoutChange;
  analysis::DefUseManager* def_use_mgr = context()->get_def_use_mgr();
  analysis::TypeManager* type_mgr = context()->get_type_mgr();
  bool modified = false;
  std::vector<std::pair<Instruction*, unsigned>> arrays_to_change;
  for (auto& var : context()->types_values()) {
    if (var.opcode() != SpvOpVariable) {
      continue;
    }
    analysis::Type* var_type = type_mgr->GetType(var.type_id());
    analysis::Pointer* ptr_type = var_type->AsPointer();
    if (ptr_type == nullptr) {
      continue;
    }
    if (ptr_type->storage_class() != SpvStorageClassInput) {
      continue;
    }
    const analysis::Array* arr_type = ptr_type->pointee_type()->AsArray();
    if (arr_type == nullptr) {
      continue;
    }
    unsigned arr_len_id = arr_type->LengthId();
    Instruction* arr_len_inst = def_use_mgr->GetDef(arr_len_id);
    if (arr_len_inst->opcode() != SpvOpConstant) {
      continue;
    }
    // SPIR-V requires array size is >= 1, so this works for signed or
    // unsigned size
    unsigned original_max =
        arr_len_inst->GetSingleWordInOperand(kConstantValueInIdx) - 1;
    unsigned max_idx = FindMaxIndex(var, original_max);
    if (max_idx != original_max) {
      ChangeArrayLength(var, max_idx + 1);
      modified = true;
    }
  }

  return modified ? Status::SuccessWithChange : Status::SuccessWithoutChange;
}

unsigned EliminateDeadInputComponentsPass::FindMaxIndex(Instruction& var,
                                                        unsigned original_max) {
  unsigned max = 0;
  bool seen_non_const_ac = false;
  assert(var.opcode() == SpvOpVariable && "must be variable");
  context()->get_def_use_mgr()->WhileEachUser(
      var.result_id(), [&max, &seen_non_const_ac, var, this](Instruction* use) {
        auto use_opcode = use->opcode();
        if (use_opcode == SpvOpLoad || use_opcode == SpvOpCopyMemory ||
            use_opcode == SpvOpCopyMemorySized ||
            use_opcode == SpvOpCopyObject) {
          seen_non_const_ac = true;
          return false;
        }
        if (use->opcode() != SpvOpAccessChain &&
            use->opcode() != SpvOpInBoundsAccessChain) {
          return true;
        }
        // OpAccessChain with no indices currently not optimized
        if (use->NumInOperands() == 1) {
          seen_non_const_ac = true;
          return false;
        }
        unsigned base_id = use->GetSingleWordInOperand(kAccessChainBaseInIdx);
        USE_ASSERT(base_id == var.result_id() && "unexpected base");
        unsigned idx_id = use->GetSingleWordInOperand(kAccessChainIndex0InIdx);
        Instruction* idx_inst = context()->get_def_use_mgr()->GetDef(idx_id);
        if (idx_inst->opcode() != SpvOpConstant) {
          seen_non_const_ac = true;
          return false;
        }
        unsigned value = idx_inst->GetSingleWordInOperand(kConstantValueInIdx);
        if (value > max) max = value;
        return true;
      });
  return seen_non_const_ac ? original_max : max;
}

void EliminateDeadInputComponentsPass::ChangeArrayLength(Instruction& arr,
                                                         unsigned length) {
  analysis::TypeManager* type_mgr = context()->get_type_mgr();
  analysis::ConstantManager* const_mgr = context()->get_constant_mgr();
  analysis::DefUseManager* def_use_mgr = context()->get_def_use_mgr();
  analysis::Pointer* ptr_type = type_mgr->GetType(arr.type_id())->AsPointer();
  const analysis::Array* arr_ty = ptr_type->pointee_type()->AsArray();
  assert(arr_ty && "expecting array type");
  uint32_t length_id = const_mgr->GetUIntConst(length);
  analysis::Array new_arr_ty(arr_ty->element_type(),
                             arr_ty->GetConstantLengthInfo(length_id, length));
  analysis::Type* reg_new_arr_ty = type_mgr->GetRegisteredType(&new_arr_ty);
  analysis::Pointer new_ptr_ty(reg_new_arr_ty, SpvStorageClassInput);
  analysis::Type* reg_new_ptr_ty = type_mgr->GetRegisteredType(&new_ptr_ty);
  uint32_t new_ptr_ty_id = type_mgr->GetTypeInstruction(reg_new_ptr_ty);
  arr.SetResultType(new_ptr_ty_id);
  def_use_mgr->AnalyzeInstUse(&arr);
  // Move array OpVariable instruction after its new type to preserve order
  USE_ASSERT(arr.GetSingleWordInOperand(kVariableStorageClassInIdx) !=
                 SpvStorageClassFunction &&
             "cannot move Function variable");
  Instruction* new_ptr_ty_inst = def_use_mgr->GetDef(new_ptr_ty_id);
  arr.RemoveFromList();
  arr.InsertAfter(new_ptr_ty_inst);
}

}  // namespace opt
}  // namespace spvtools
