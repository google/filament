// Copyright (c) 2017 Google Inc.
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

#include "source/opt/cfg.h"

#include <memory>
#include <utility>

#include "source/cfa.h"
#include "source/opt/ir_builder.h"
#include "source/opt/ir_context.h"
#include "source/opt/module.h"

namespace spvtools {
namespace opt {
namespace {

using cbb_ptr = const opt::BasicBlock*;

// Universal Limit of ResultID + 1
const int kMaxResultId = 0x400000;

}  // namespace

CFG::CFG(Module* module)
    : module_(module),
      pseudo_entry_block_(std::unique_ptr<Instruction>(
          new Instruction(module->context(), SpvOpLabel, 0, 0, {}))),
      pseudo_exit_block_(std::unique_ptr<Instruction>(new Instruction(
          module->context(), SpvOpLabel, 0, kMaxResultId, {}))) {
  for (auto& fn : *module) {
    for (auto& blk : fn) {
      RegisterBlock(&blk);
    }
  }
}

void CFG::AddEdges(BasicBlock* blk) {
  uint32_t blk_id = blk->id();
  // Force the creation of an entry, not all basic block have predecessors
  // (such as the entry blocks and some unreachables).
  label2preds_[blk_id];
  const auto* const_blk = blk;
  const_blk->ForEachSuccessorLabel(
      [blk_id, this](const uint32_t succ_id) { AddEdge(blk_id, succ_id); });
}

void CFG::RemoveNonExistingEdges(uint32_t blk_id) {
  std::vector<uint32_t> updated_pred_list;
  for (uint32_t id : preds(blk_id)) {
    const BasicBlock* pred_blk = block(id);
    bool has_branch = false;
    pred_blk->ForEachSuccessorLabel([&has_branch, blk_id](uint32_t succ) {
      if (succ == blk_id) {
        has_branch = true;
      }
    });
    if (has_branch) updated_pred_list.push_back(id);
  }

  label2preds_.at(blk_id) = std::move(updated_pred_list);
}

void CFG::ComputeStructuredOrder(Function* func, BasicBlock* root,
                                 std::list<BasicBlock*>* order) {
  assert(module_->context()->get_feature_mgr()->HasCapability(
             SpvCapabilityShader) &&
         "This only works on structured control flow");

  // Compute structured successors and do DFS.
  ComputeStructuredSuccessors(func);
  auto ignore_block = [](cbb_ptr) {};
  auto ignore_edge = [](cbb_ptr, cbb_ptr) {};
  auto get_structured_successors = [this](const BasicBlock* b) {
    return &(block2structured_succs_[b]);
  };

  // TODO(greg-lunarg): Get rid of const_cast by making moving const
  // out of the cfa.h prototypes and into the invoking code.
  auto post_order = [&](cbb_ptr b) {
    order->push_front(const_cast<BasicBlock*>(b));
  };
  CFA<BasicBlock>::DepthFirstTraversal(root, get_structured_successors,
                                       ignore_block, post_order, ignore_edge);
}

void CFG::ForEachBlockInPostOrder(BasicBlock* bb,
                                  const std::function<void(BasicBlock*)>& f) {
  std::vector<BasicBlock*> po;
  std::unordered_set<BasicBlock*> seen;
  ComputePostOrderTraversal(bb, &po, &seen);

  for (BasicBlock* current_bb : po) {
    if (!IsPseudoExitBlock(current_bb) && !IsPseudoEntryBlock(current_bb)) {
      f(current_bb);
    }
  }
}

void CFG::ForEachBlockInReversePostOrder(
    BasicBlock* bb, const std::function<void(BasicBlock*)>& f) {
  std::vector<BasicBlock*> po;
  std::unordered_set<BasicBlock*> seen;
  ComputePostOrderTraversal(bb, &po, &seen);

  for (auto current_bb = po.rbegin(); current_bb != po.rend(); ++current_bb) {
    if (!IsPseudoExitBlock(*current_bb) && !IsPseudoEntryBlock(*current_bb)) {
      f(*current_bb);
    }
  }
}

void CFG::ComputeStructuredSuccessors(Function* func) {
  block2structured_succs_.clear();
  for (auto& blk : *func) {
    // If no predecessors in function, make successor to pseudo entry.
    if (label2preds_[blk.id()].size() == 0)
      block2structured_succs_[&pseudo_entry_block_].push_back(&blk);

    // If header, make merge block first successor and continue block second
    // successor if there is one.
    uint32_t mbid = blk.MergeBlockIdIfAny();
    if (mbid != 0) {
      block2structured_succs_[&blk].push_back(block(mbid));
      uint32_t cbid = blk.ContinueBlockIdIfAny();
      if (cbid != 0) {
        block2structured_succs_[&blk].push_back(block(cbid));
      }
    }

    // Add true successors.
    const auto& const_blk = blk;
    const_blk.ForEachSuccessorLabel([&blk, this](const uint32_t sbid) {
      block2structured_succs_[&blk].push_back(block(sbid));
    });
  }
}

void CFG::ComputePostOrderTraversal(BasicBlock* bb,
                                    std::vector<BasicBlock*>* order,
                                    std::unordered_set<BasicBlock*>* seen) {
  seen->insert(bb);
  static_cast<const BasicBlock*>(bb)->ForEachSuccessorLabel(
      [&order, &seen, this](const uint32_t sbid) {
        BasicBlock* succ_bb = id2block_[sbid];
        if (!seen->count(succ_bb)) {
          ComputePostOrderTraversal(succ_bb, order, seen);
        }
      });
  order->push_back(bb);
}

BasicBlock* CFG::SplitLoopHeader(BasicBlock* bb) {
  assert(bb->GetLoopMergeInst() && "Expecting bb to be the header of a loop.");

  Function* fn = bb->GetParent();
  IRContext* context = module_->context();

  // Find the insertion point for the new bb.
  Function::iterator header_it = std::find_if(
      fn->begin(), fn->end(),
      [bb](BasicBlock& block_in_func) { return &block_in_func == bb; });
  assert(header_it != fn->end());

  const std::vector<uint32_t>& pred = preds(bb->id());
  // Find the back edge
  BasicBlock* latch_block = nullptr;
  Function::iterator latch_block_iter = header_it;
  while (++latch_block_iter != fn->end()) {
    // If blocks are in the proper order, then the only branch that appears
    // after the header is the latch.
    if (std::find(pred.begin(), pred.end(), latch_block_iter->id()) !=
        pred.end()) {
      break;
    }
  }
  assert(latch_block_iter != fn->end() && "Could not find the latch.");
  latch_block = &*latch_block_iter;

  RemoveSuccessorEdges(bb);

  // Create the new header bb basic bb.
  // Leave the phi instructions behind.
  auto iter = bb->begin();
  while (iter->opcode() == SpvOpPhi) {
    ++iter;
  }

  std::unique_ptr<BasicBlock> newBlock(
      bb->SplitBasicBlock(context, context->TakeNextId(), iter));

  // Insert the new bb in the correct position
  auto insert_pos = header_it;
  ++insert_pos;
  BasicBlock* new_header = &*insert_pos.InsertBefore(std::move(newBlock));
  new_header->SetParent(fn);
  uint32_t new_header_id = new_header->id();
  context->AnalyzeDefUse(new_header->GetLabelInst());

  // Update cfg
  RegisterBlock(new_header);

  // Update bb mappings.
  context->set_instr_block(new_header->GetLabelInst(), new_header);
  new_header->ForEachInst([new_header, context](Instruction* inst) {
    context->set_instr_block(inst, new_header);
  });

  // Adjust the OpPhi instructions as needed.
  bb->ForEachPhiInst([latch_block, bb, new_header, context](Instruction* phi) {
    std::vector<uint32_t> preheader_phi_ops;
    std::vector<Operand> header_phi_ops;

    // Identify where the original inputs to original OpPhi belong: header or
    // preheader.
    for (uint32_t i = 0; i < phi->NumInOperands(); i += 2) {
      uint32_t def_id = phi->GetSingleWordInOperand(i);
      uint32_t branch_id = phi->GetSingleWordInOperand(i + 1);
      if (branch_id == latch_block->id()) {
        header_phi_ops.push_back({SPV_OPERAND_TYPE_ID, {def_id}});
        header_phi_ops.push_back({SPV_OPERAND_TYPE_ID, {branch_id}});
      } else {
        preheader_phi_ops.push_back(def_id);
        preheader_phi_ops.push_back(branch_id);
      }
    }

    // Create a phi instruction if and only if the preheader_phi_ops has more
    // than one pair.
    if (preheader_phi_ops.size() > 2) {
      InstructionBuilder builder(
          context, &*bb->begin(),
          IRContext::kAnalysisDefUse | IRContext::kAnalysisInstrToBlockMapping);

      Instruction* new_phi = builder.AddPhi(phi->type_id(), preheader_phi_ops);

      // Add the OpPhi to the header bb.
      header_phi_ops.push_back({SPV_OPERAND_TYPE_ID, {new_phi->result_id()}});
      header_phi_ops.push_back({SPV_OPERAND_TYPE_ID, {bb->id()}});
    } else {
      // An OpPhi with a single entry is just a copy.  In this case use the same
      // instruction in the new header.
      header_phi_ops.push_back({SPV_OPERAND_TYPE_ID, {preheader_phi_ops[0]}});
      header_phi_ops.push_back({SPV_OPERAND_TYPE_ID, {bb->id()}});
    }

    phi->RemoveFromList();
    std::unique_ptr<Instruction> phi_owner(phi);
    phi->SetInOperands(std::move(header_phi_ops));
    new_header->begin()->InsertBefore(std::move(phi_owner));
    context->set_instr_block(phi, new_header);
    context->AnalyzeUses(phi);
  });

  // Add a branch to the new header.
  InstructionBuilder branch_builder(
      context, bb,
      IRContext::kAnalysisDefUse | IRContext::kAnalysisInstrToBlockMapping);
  bb->AddInstruction(
      MakeUnique<Instruction>(context, SpvOpBranch, 0, 0,
                              std::initializer_list<Operand>{
                                  {SPV_OPERAND_TYPE_ID, {new_header->id()}}}));
  context->AnalyzeUses(bb->terminator());
  context->set_instr_block(bb->terminator(), bb);
  label2preds_[new_header->id()].push_back(bb->id());

  // Update the latch to branch to the new header.
  latch_block->ForEachSuccessorLabel([bb, new_header_id](uint32_t* id) {
    if (*id == bb->id()) {
      *id = new_header_id;
    }
  });
  Instruction* latch_branch = latch_block->terminator();
  context->AnalyzeUses(latch_branch);
  label2preds_[new_header->id()].push_back(latch_block->id());

  auto& block_preds = label2preds_[bb->id()];
  auto latch_pos =
      std::find(block_preds.begin(), block_preds.end(), latch_block->id());
  assert(latch_pos != block_preds.end() && "The cfg was invalid.");
  block_preds.erase(latch_pos);

  // Update the loop descriptors
  if (context->AreAnalysesValid(IRContext::kAnalysisLoopAnalysis)) {
    LoopDescriptor* loop_desc = context->GetLoopDescriptor(bb->GetParent());
    Loop* loop = (*loop_desc)[bb->id()];

    loop->AddBasicBlock(new_header_id);
    loop->SetHeaderBlock(new_header);
    loop_desc->SetBasicBlockToLoop(new_header_id, loop);

    loop->RemoveBasicBlock(bb->id());
    loop->SetPreHeaderBlock(bb);

    Loop* parent_loop = loop->GetParent();
    if (parent_loop != nullptr) {
      parent_loop->AddBasicBlock(bb->id());
      loop_desc->SetBasicBlockToLoop(bb->id(), parent_loop);
    } else {
      loop_desc->SetBasicBlockToLoop(bb->id(), nullptr);
    }
  }
  return new_header;
}

}  // namespace opt
}  // namespace spvtools
