// Copyright (c) 2019 Google LLC.
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

#include "source/opt/decompose_initialized_variables_pass.h"

#include "source/opt/ir_context.h"

namespace spvtools {
namespace opt {

using inst_iterator = InstructionList::iterator;

namespace {

bool HasInitializer(Instruction* inst) {
  if (inst->opcode() != SpvOpVariable) return false;
  if (inst->NumOperands() < 4) return false;

  return true;
}

}  // namespace

Pass::Status DecomposeInitializedVariablesPass::Process() {
  auto* module = context()->module();
  std::unordered_set<Instruction*> changed;

  std::vector<std::tuple<uint32_t, uint32_t>> global_stores;
  for (auto iter = module->types_values_begin();
       iter != module->types_values_end(); ++iter) {
    Instruction* inst = &(*iter);
    if (!HasInitializer(inst)) continue;

    auto var_id = inst->result_id();
    auto val_id = inst->GetOperand(3).words[0];
    global_stores.push_back(std::make_tuple(var_id, val_id));
    iter->RemoveOperand(3);
    changed.insert(&*iter);
  }

  std::unordered_set<uint32_t> entry_ids;
  for (auto entry = module->entry_points().begin();
       entry != module->entry_points().end(); ++entry) {
    entry_ids.insert(entry->GetSingleWordInOperand(1));
  }

  for (auto func = module->begin(); func != module->end(); ++func) {
    std::vector<Instruction*> function_stores;
    auto first_block = func->entry().get();
    inst_iterator insert_point = first_block->begin();
    for (auto iter = first_block->begin();
         iter != first_block->end() && iter->opcode() == SpvOpVariable;
         ++iter) {
      // For valid SPIRV-V, there is guaranteed to be at least one instruction
      // after the OpVariable instructions.
      insert_point = (*iter).NextNode();
      Instruction* inst = &(*iter);
      if (!HasInitializer(inst)) continue;

      auto var_id = inst->result_id();
      auto val_id = inst->GetOperand(3).words[0];
      Instruction* store_inst = new Instruction(
          context(), SpvOpStore, 0, 0,
          {{SPV_OPERAND_TYPE_ID, {var_id}}, {SPV_OPERAND_TYPE_ID, {val_id}}});
      function_stores.push_back(store_inst);
      iter->RemoveOperand(3);
      changed.insert(&*iter);
    }

    if (entry_ids.find(func->result_id()) != entry_ids.end()) {
      for (auto store_ids : global_stores) {
        uint32_t var_id;
        uint32_t val_id;
        std::tie(var_id, val_id) = store_ids;
        auto* store_inst = new Instruction(
            context(), SpvOpStore, 0, 0,
            {{SPV_OPERAND_TYPE_ID, {var_id}}, {SPV_OPERAND_TYPE_ID, {val_id}}});
        context()->set_instr_block(store_inst, &*first_block);
        first_block->AddInstruction(std::unique_ptr<Instruction>(store_inst));
        store_inst->InsertBefore(&*insert_point);
        changed.insert(store_inst);
      }
    }

    for (auto store = function_stores.begin(); store != function_stores.end();
         ++store) {
      context()->set_instr_block(*store, first_block);
      (*store)->InsertBefore(&*insert_point);
      changed.insert(*store);
    }
  }

  auto* def_use_mgr = get_def_use_mgr();
  for (auto* inst : changed) def_use_mgr->UpdateDefUse(inst);

  return !changed.empty() ? Pass::Status::SuccessWithChange
                          : Pass::Status::SuccessWithoutChange;
}

}  // namespace opt
}  // namespace spvtools
