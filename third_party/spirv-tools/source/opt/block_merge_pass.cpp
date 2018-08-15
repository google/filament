// Copyright (c) 2017 The Khronos Group Inc.
// Copyright (c) 2017 Valve Corporation
// Copyright (c) 2017 LunarG Inc.
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

#include "source/opt/block_merge_pass.h"

#include <vector>

#include "source/opt/ir_context.h"
#include "source/opt/iterator.h"

namespace spvtools {
namespace opt {

void BlockMergePass::KillInstAndName(Instruction* inst) {
  std::vector<Instruction*> to_kill;
  get_def_use_mgr()->ForEachUser(inst, [&to_kill](Instruction* user) {
    if (user->opcode() == SpvOpName) {
      to_kill.push_back(user);
    }
  });
  for (auto i : to_kill) {
    context()->KillInst(i);
  }
  context()->KillInst(inst);
}

bool BlockMergePass::MergeBlocks(Function* func) {
  bool modified = false;
  for (auto bi = func->begin(); bi != func->end();) {
    // Find block with single successor which has no other predecessors.
    auto ii = bi->end();
    --ii;
    Instruction* br = &*ii;
    if (br->opcode() != SpvOpBranch) {
      ++bi;
      continue;
    }

    const uint32_t lab_id = br->GetSingleWordInOperand(0);
    if (cfg()->preds(lab_id).size() != 1) {
      ++bi;
      continue;
    }

    bool pred_is_header = IsHeader(&*bi);
    bool succ_is_header = IsHeader(lab_id);
    if (pred_is_header && succ_is_header) {
      // Cannot merge two headers together.
      ++bi;
      continue;
    }

    bool pred_is_merge = IsMerge(&*bi);
    bool succ_is_merge = IsMerge(lab_id);
    if (pred_is_merge && succ_is_merge) {
      // Cannot merge two merges together.
      ++bi;
      continue;
    }

    Instruction* merge_inst = bi->GetMergeInst();
    if (pred_is_header && lab_id != merge_inst->GetSingleWordInOperand(0u)) {
      // If this is a header block and the successor is not its merge, we must
      // be careful about which blocks we are willing to merge together.
      // OpLoopMerge must be followed by a conditional or unconditional branch.
      // The merge must be a loop merge because a selection merge cannot be
      // followed by an unconditional branch.
      BasicBlock* succ_block = context()->get_instr_block(lab_id);
      SpvOp succ_term_op = succ_block->terminator()->opcode();
      assert(merge_inst->opcode() == SpvOpLoopMerge);
      if (succ_term_op != SpvOpBranch &&
          succ_term_op != SpvOpBranchConditional) {
        ++bi;
        continue;
      }
    }

    // Merge blocks.
    context()->KillInst(br);
    auto sbi = bi;
    for (; sbi != func->end(); ++sbi)
      if (sbi->id() == lab_id) break;
    // If bi is sbi's only predecessor, it dominates sbi and thus
    // sbi must follow bi in func's ordering.
    assert(sbi != func->end());

    // Update the inst-to-block mapping for the instructions in sbi.
    for (auto& inst : *sbi) {
      context()->set_instr_block(&inst, &*bi);
    }

    // Now actually move the instructions.
    bi->AddInstructions(&*sbi);

    if (merge_inst) {
      if (pred_is_header && lab_id == merge_inst->GetSingleWordInOperand(0u)) {
        // Merging the header and merge blocks, so remove the structured control
        // flow declaration.
        context()->KillInst(merge_inst);
      } else {
        // Move the merge instruction to just before the terminator.
        merge_inst->InsertBefore(bi->terminator());
      }
    }
    context()->ReplaceAllUsesWith(lab_id, bi->id());
    KillInstAndName(sbi->GetLabelInst());
    (void)sbi.Erase();
    // Reprocess block.
    modified = true;
  }
  return modified;
}

bool BlockMergePass::IsHeader(BasicBlock* block) {
  return block->GetMergeInst() != nullptr;
}

bool BlockMergePass::IsHeader(uint32_t id) {
  return IsHeader(context()->get_instr_block(get_def_use_mgr()->GetDef(id)));
}

bool BlockMergePass::IsMerge(uint32_t id) {
  return !get_def_use_mgr()->WhileEachUse(id, [](Instruction* user,
                                                 uint32_t index) {
    SpvOp op = user->opcode();
    if ((op == SpvOpLoopMerge || op == SpvOpSelectionMerge) && index == 0u) {
      return false;
    }
    return true;
  });
}

bool BlockMergePass::IsMerge(BasicBlock* block) { return IsMerge(block->id()); }

Pass::Status BlockMergePass::Process() {
  // Process all entry point functions.
  ProcessFunction pfn = [this](Function* fp) { return MergeBlocks(fp); };
  bool modified = ProcessEntryPointCallTree(pfn, get_module());
  return modified ? Status::SuccessWithChange : Status::SuccessWithoutChange;
}

BlockMergePass::BlockMergePass() = default;

}  // namespace opt
}  // namespace spvtools
