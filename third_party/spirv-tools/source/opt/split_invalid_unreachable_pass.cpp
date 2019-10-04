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

#include "source/opt/split_invalid_unreachable_pass.h"

#include "source/opt/ir_builder.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace opt {

Pass::Status SplitInvalidUnreachablePass::Process() {
  bool changed = false;
  std::unordered_set<uint32_t> entry_points;
  for (auto entry_point : context()->module()->entry_points()) {
    entry_points.insert(entry_point.GetSingleWordOperand(1));
  }

  for (auto func = context()->module()->begin();
       func != context()->module()->end(); ++func) {
    if (entry_points.find(func->result_id()) == entry_points.end()) continue;
    std::unordered_set<uint32_t> continue_targets;
    std::unordered_set<uint32_t> merge_blocks;
    std::unordered_set<BasicBlock*> unreachable_blocks;
    for (auto block = func->begin(); block != func->end(); ++block) {
      unreachable_blocks.insert(&*block);
      uint32_t continue_target = block->ContinueBlockIdIfAny();
      if (continue_target != 0) continue_targets.insert(continue_target);
      uint32_t merge_block = block->MergeBlockIdIfAny();
      if (merge_block != 0) merge_blocks.insert(merge_block);
    }

    cfg()->ForEachBlockInPostOrder(
        func->entry().get(), [&unreachable_blocks](BasicBlock* inner_block) {
          unreachable_blocks.erase(inner_block);
        });

    for (auto unreachable : unreachable_blocks) {
      uint32_t block_id = unreachable->id();
      if (continue_targets.find(block_id) == continue_targets.end() ||
          merge_blocks.find(block_id) == merge_blocks.end()) {
        continue;
      }

      std::vector<std::tuple<Instruction*, uint32_t>> usages;
      context()->get_def_use_mgr()->ForEachUse(
          unreachable->GetLabelInst(),
          [&usages](Instruction* use, uint32_t idx) {
            if ((use->opcode() == SpvOpLoopMerge && idx == 0) ||
                use->opcode() == SpvOpSelectionMerge) {
              usages.push_back(std::make_pair(use, idx));
            }
          });

      for (auto usage : usages) {
        Instruction* use;
        uint32_t idx;
        std::tie(use, idx) = usage;
        uint32_t new_id = context()->TakeNextId();
        std::unique_ptr<Instruction> new_label(
            new Instruction(context(), SpvOpLabel, 0, new_id, {}));
        get_def_use_mgr()->AnalyzeInstDefUse(new_label.get());
        std::unique_ptr<BasicBlock> new_block(
            new BasicBlock(std::move(new_label)));
        auto* block_ptr = new_block.get();
        InstructionBuilder builder(context(), new_block.get(),
                                   IRContext::kAnalysisDefUse |
                                       IRContext::kAnalysisInstrToBlockMapping);
        builder.AddUnreachable();
        cfg()->RegisterBlock(block_ptr);
        (&*func)->InsertBasicBlockBefore(std::move(new_block), unreachable);
        use->SetInOperand(0, {new_id});
        get_def_use_mgr()->UpdateDefUse(use);
        cfg()->AddEdges(block_ptr);
        changed = true;
      }
    }
  }

  return changed ? Status::SuccessWithChange : Status::SuccessWithoutChange;
}

}  // namespace opt
}  // namespace spvtools
