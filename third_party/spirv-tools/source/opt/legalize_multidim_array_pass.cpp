// Copyright (c) 2026 Google LLC
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

#include "source/opt/legalize_multidim_array_pass.h"

#include "source/opt/constants.h"
#include "source/opt/desc_sroa_util.h"
#include "source/opt/ir_builder.h"
#include "source/opt/ir_context.h"
#include "source/opt/type_manager.h"

namespace spvtools {
namespace opt {

Pass::Status LegalizeMultidimArrayPass::Process() {
  std::vector<Instruction*> vars_to_legalize;

  for (auto& var : context()->types_values()) {
    if (var.opcode() != spv::Op::OpVariable) continue;
    if (!IsMultidimArrayOfResources(&var)) continue;
    if (!CanLegalize(&var)) {
      context()->EmitErrorMessage("Unable to legalize multidimensional array: ",
                                  &var);
      return Status::Failure;
    }
    vars_to_legalize.push_back(&var);
  }

  if (vars_to_legalize.empty()) return Status::SuccessWithoutChange;

  for (auto* var : vars_to_legalize) {
    uint32_t old_ptr_type_id = var->type_id();
    uint32_t new_ptr_type_id = FlattenArrayType(var);
    if (new_ptr_type_id == 0) return Status::Failure;
    if (!RewriteAccessChains(var, old_ptr_type_id)) return Status::Failure;
  }

  return Status::SuccessWithChange;
}

bool LegalizeMultidimArrayPass::IsMultidimArrayOfResources(Instruction* var) {
  if (!descsroautil::IsDescriptorArray(context(), var)) return false;

  uint32_t type_id = var->type_id();
  Instruction* type_inst = context()->get_def_use_mgr()->GetDef(type_id);
  uint32_t pointee_type_id = type_inst->GetSingleWordInOperand(1);
  std::vector<uint32_t> dims;
  uint32_t element_type_id = 0;
  GetArrayDimensions(pointee_type_id, &dims, &element_type_id);

  return dims.size() > 1;
}

void LegalizeMultidimArrayPass::GetArrayDimensions(uint32_t type_id,
                                                   std::vector<uint32_t>* dims,
                                                   uint32_t* element_type_id) {
  assert(dims != nullptr && "dims cannot be null.");
  dims->clear();

  Instruction* type_inst = context()->get_def_use_mgr()->GetDef(type_id);
  while (type_inst->opcode() == spv::Op::OpTypeArray) {
    uint32_t length_id = type_inst->GetSingleWordInOperand(1);
    Instruction* length_inst = context()->get_def_use_mgr()->GetDef(length_id);
    // Assume OpConstant. According to the spec the length could also be an
    // OpSpecConstantOp. However, DXC will not generate that type of code. The
    // code to handle spec constants will be much more complicated.
    assert(length_inst->opcode() == spv::Op::OpConstant);
    uint32_t length = length_inst->GetSingleWordInOperand(0);
    dims->push_back(length);
    type_id = type_inst->GetSingleWordInOperand(0);
    type_inst = context()->get_def_use_mgr()->GetDef(type_id);
  }
  *element_type_id = type_id;
}

uint32_t LegalizeMultidimArrayPass::FlattenArrayType(Instruction* var) {
  analysis::TypeManager* type_mgr = context()->get_type_mgr();
  analysis::ConstantManager* constant_mgr = context()->get_constant_mgr();

  uint32_t ptr_type_id = var->type_id();
  Instruction* ptr_type_inst =
      context()->get_def_use_mgr()->GetDef(ptr_type_id);
  uint32_t pointee_type_id = ptr_type_inst->GetSingleWordInOperand(1);

  std::vector<uint32_t> dims;
  uint32_t element_type_id = 0;
  GetArrayDimensions(pointee_type_id, &dims, &element_type_id);

  uint32_t total_elements = 1;
  for (uint32_t dim : dims) {
    total_elements *= dim;
  }

  const analysis::Constant* total_elements_const =
      constant_mgr->GetIntConst(total_elements, 32, false);

  Instruction* total_elements_inst =
      constant_mgr->GetDefiningInstruction(total_elements_const);
  uint32_t total_elements_id = total_elements_inst->result_id();

  // Create new OpTypeArray.
  analysis::Type* element_type = type_mgr->GetType(element_type_id);
  analysis::Array::LengthInfo length_info = {
      total_elements_id,
      {analysis::Array::LengthInfo::kConstant, total_elements}};
  analysis::Array new_array_type(element_type, length_info);
  uint32_t new_array_type_id = type_mgr->GetTypeInstruction(&new_array_type);

  // Create new OpTypePointer.
  spv::StorageClass sc =
      static_cast<spv::StorageClass>(ptr_type_inst->GetSingleWordInOperand(0));
  analysis::Pointer new_ptr_type(type_mgr->GetType(new_array_type_id), sc);
  uint32_t new_ptr_type_id = type_mgr->GetTypeInstruction(&new_ptr_type);

  var->SetResultType(new_ptr_type_id);
  context()->UpdateDefUse(var);

  // Move the var after the new pointer type to avoid a def-before-use.
  var->InsertAfter(get_def_use_mgr()->GetDef(new_ptr_type_id));

  return new_ptr_type_id;
}

bool LegalizeMultidimArrayPass::RewriteAccessChains(Instruction* var,
                                                    uint32_t old_ptr_type_id) {
  uint32_t var_id = var->result_id();
  std::vector<Instruction*> users;
  // Use a worklist to handle transitive uses (e.g. through OpCopyObject)
  std::vector<Instruction*> worklist;

  context()->get_def_use_mgr()->ForEachUser(
      var_id, [&worklist](Instruction* user) { worklist.push_back(user); });

  Instruction* old_ptr_type_inst =
      context()->get_def_use_mgr()->GetDef(old_ptr_type_id);
  uint32_t old_pointee_type_id = old_ptr_type_inst->GetSingleWordInOperand(1);
  std::vector<uint32_t> dims;
  uint32_t element_type_id = 0;
  GetArrayDimensions(old_pointee_type_id, &dims, &element_type_id);
  assert(dims.size() != 0 &&
         "This variable should have been rejected earlier.");

  // Calculate strides once
  std::vector<uint32_t> strides(dims.size());
  strides[dims.size() - 1] = 1;
  for (int i = static_cast<int>(dims.size()) - 2; i >= 0; --i) {
    strides[i] = strides[i + 1] * dims[i + 1];
  }

  // Pre-calculate uint type id
  uint32_t uint_type_id = context()->get_type_mgr()->GetUIntTypeId();
  if (uint_type_id == 0) return false;

  while (!worklist.empty()) {
    Instruction* user = worklist.back();
    worklist.pop_back();

    if (user->opcode() == spv::Op::OpAccessChain ||
        user->opcode() == spv::Op::OpInBoundsAccessChain) {
      uint32_t num_indices = user->NumInOperands() - 1;
      assert(num_indices >= dims.size());

      InstructionBuilder builder(context(), user, IRContext::kAnalysisDefUse);

      uint32_t linearized_idx_id = 0;
      for (uint32_t i = 0; i < dims.size(); ++i) {
        uint32_t idx_id = user->GetSingleWordInOperand(i + 1);

        uint32_t term_id = idx_id;
        if (strides[i] != 1) {
          const analysis::Constant* stride_const =
              context()->get_constant_mgr()->GetConstant(
                  context()->get_type_mgr()->GetType(uint_type_id),
                  {strides[i]});
          Instruction* stride_inst =
              context()->get_constant_mgr()->GetDefiningInstruction(
                  stride_const);

          Instruction* mul_inst = builder.AddBinaryOp(
              uint_type_id, spv::Op::OpIMul, idx_id, stride_inst->result_id());
          if (mul_inst == nullptr) return false;
          term_id = mul_inst->result_id();
        }

        if (linearized_idx_id == 0) {
          linearized_idx_id = term_id;
        } else {
          Instruction* add_inst = builder.AddBinaryOp(
              uint_type_id, spv::Op::OpIAdd, linearized_idx_id, term_id);
          if (add_inst == nullptr) return false;
          linearized_idx_id = add_inst->result_id();
        }
      }

      // Create new AccessChain.
      Instruction::OperandList new_operands;
      new_operands.push_back(user->GetInOperand(0));
      new_operands.push_back({SPV_OPERAND_TYPE_ID, {linearized_idx_id}});
      for (uint32_t i = static_cast<uint32_t>(dims.size()); i < num_indices;
           ++i) {
        new_operands.push_back(user->GetInOperand(i + 1));
      }
      user->SetInOperands(std::move(new_operands));
      context()->UpdateDefUse(user);
    } else if (user->opcode() == spv::Op::OpCopyObject) {
      // The type of the variable has changed so the result type of the
      // OpCopyObject will change as well.

      uint32_t operand_id = user->GetSingleWordInOperand(0);
      Instruction* operand_inst =
          context()->get_def_use_mgr()->GetDef(operand_id);
      user->SetResultType(operand_inst->type_id());
      context()->UpdateDefUse(user);

      // Add users of this copy to worklist
      context()->get_def_use_mgr()->ForEachUser(
          user->result_id(),
          [&worklist](Instruction* u) { worklist.push_back(u); });
    }
  }
  return true;
}

bool LegalizeMultidimArrayPass::CheckUse(Instruction* inst,
                                         uint32_t max_depth) {
  if (inst->opcode() == spv::Op::OpAccessChain ||
      inst->opcode() == spv::Op::OpInBoundsAccessChain) {
    uint32_t num_indices = inst->NumInOperands() - 1;
    return num_indices >= max_depth;
  } else if (inst->opcode() == spv::Op::OpCopyObject) {
    bool ok = true;
    return !context()->get_def_use_mgr()->WhileEachUser(
        inst->result_id(),
        [&](Instruction* u) { return !CheckUse(u, max_depth); });
    return ok;
  } else if (inst->IsDecoration() || inst->opcode() == spv::Op::OpName ||
             inst->opcode() == spv::Op::OpMemberName) {
    // Metadata is fine.
    return true;
  }

  // Direct use of array or partial array without AccessChain is not allowed.
  return false;
}

bool LegalizeMultidimArrayPass::CanLegalize(Instruction* var) {
  bool ok = true;
  uint32_t ptr_type_id = var->type_id();
  Instruction* ptr_type_inst =
      context()->get_def_use_mgr()->GetDef(ptr_type_id);
  uint32_t pointee_type_id = ptr_type_inst->GetSingleWordInOperand(1);
  std::vector<uint32_t> dims;
  uint32_t element_type_id = 0;
  GetArrayDimensions(pointee_type_id, &dims, &element_type_id);

  context()->get_def_use_mgr()->ForEachUser(
      var->result_id(), [&](Instruction* u) {
        if (!CheckUse(u, static_cast<uint32_t>(dims.size()))) ok = false;
      });
  return ok;
}

}  // namespace opt
}  // namespace spvtools
