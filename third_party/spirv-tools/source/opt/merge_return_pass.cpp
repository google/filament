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

#include "source/opt/merge_return_pass.h"

#include <list>
#include <memory>
#include <utility>

#include "source/opt/instruction.h"
#include "source/opt/ir_builder.h"
#include "source/opt/ir_context.h"
#include "source/opt/reflect.h"
#include "source/util/make_unique.h"

namespace spvtools {
namespace opt {

Pass::Status MergeReturnPass::Process() {
  bool modified = false;
  bool is_shader =
      context()->get_feature_mgr()->HasCapability(SpvCapabilityShader);
  for (auto& function : *get_module()) {
    std::vector<BasicBlock*> return_blocks = CollectReturnBlocks(&function);
    if (return_blocks.size() <= 1) continue;

    function_ = &function;
    return_flag_ = nullptr;
    return_value_ = nullptr;
    final_return_block_ = nullptr;

    modified = true;
    if (is_shader) {
      ProcessStructured(&function, return_blocks);
    } else {
      MergeReturnBlocks(&function, return_blocks);
    }
  }

  return modified ? Status::SuccessWithChange : Status::SuccessWithoutChange;
}

void MergeReturnPass::ProcessStructured(
    Function* function, const std::vector<BasicBlock*>& return_blocks) {
  std::list<BasicBlock*> order;
  cfg()->ComputeStructuredOrder(function, &*function->begin(), &order);

  // Create the new return block
  CreateReturnBlock();

  // Create the return
  CreateReturn(final_return_block_);

  cfg()->RegisterBlock(final_return_block_);

  state_.clear();
  state_.emplace_back(nullptr, nullptr);
  for (auto block : order) {
    if (cfg()->IsPseudoEntryBlock(block) || cfg()->IsPseudoExitBlock(block)) {
      continue;
    }

    auto blockId = block->GetLabelInst()->result_id();
    if (blockId == CurrentState().CurrentMergeId()) {
      // Pop the current state as we've hit the merge
      state_.pop_back();
    }

    ProcessStructuredBlock(block);

    // Generate state for next block
    if (Instruction* mergeInst = block->GetMergeInst()) {
      Instruction* loopMergeInst = block->GetLoopMergeInst();
      if (!loopMergeInst) loopMergeInst = state_.back().LoopMergeInst();
      state_.emplace_back(loopMergeInst, mergeInst);
    }
  }

  // Predicate successors of the original return blocks as necessary.
  PredicateBlocks(return_blocks);

  // We have not kept the dominator tree up-to-date.
  // Invalidate it at this point to make sure it will be rebuilt.
  context()->RemoveDominatorAnalysis(function);
  AddNewPhiNodes();
}

void MergeReturnPass::CreateReturnBlock() {
  // Create a label for the new return block
  std::unique_ptr<Instruction> return_label(
      new Instruction(context(), SpvOpLabel, 0u, TakeNextId(), {}));

  // Create the new basic block
  std::unique_ptr<BasicBlock> return_block(
      new BasicBlock(std::move(return_label)));
  function_->AddBasicBlock(std::move(return_block));
  final_return_block_ = &*(--function_->end());
  context()->AnalyzeDefUse(final_return_block_->GetLabelInst());
  context()->set_instr_block(final_return_block_->GetLabelInst(),
                             final_return_block_);
  final_return_block_->SetParent(function_);
}

void MergeReturnPass::CreateReturn(BasicBlock* block) {
  AddReturnValue();

  if (return_value_) {
    // Load and return the final return value
    uint32_t loadId = TakeNextId();
    block->AddInstruction(MakeUnique<Instruction>(
        context(), SpvOpLoad, function_->type_id(), loadId,
        std::initializer_list<Operand>{
            {SPV_OPERAND_TYPE_ID, {return_value_->result_id()}}}));
    Instruction* var_inst = block->terminator();
    context()->AnalyzeDefUse(var_inst);
    context()->set_instr_block(var_inst, block);

    block->AddInstruction(MakeUnique<Instruction>(
        context(), SpvOpReturnValue, 0, 0,
        std::initializer_list<Operand>{{SPV_OPERAND_TYPE_ID, {loadId}}}));
    context()->AnalyzeDefUse(block->terminator());
    context()->set_instr_block(block->terminator(), block);
  } else {
    block->AddInstruction(MakeUnique<Instruction>(context(), SpvOpReturn));
    context()->AnalyzeDefUse(block->terminator());
    context()->set_instr_block(block->terminator(), block);
  }
}

void MergeReturnPass::ProcessStructuredBlock(BasicBlock* block) {
  SpvOp tail_opcode = block->tail()->opcode();
  if (tail_opcode == SpvOpReturn || tail_opcode == SpvOpReturnValue) {
    if (!return_flag_) {
      AddReturnFlag();
    }
  }

  if (tail_opcode == SpvOpReturn || tail_opcode == SpvOpReturnValue ||
      tail_opcode == SpvOpUnreachable) {
    if (CurrentState().InLoop()) {
      // Can always break out of innermost loop
      BranchToBlock(block, CurrentState().LoopMergeId());
    } else if (CurrentState().InStructuredFlow()) {
      BranchToBlock(block, CurrentState().CurrentMergeId());
    } else {
      BranchToBlock(block, final_return_block_->id());
    }
  }
}

void MergeReturnPass::BranchToBlock(BasicBlock* block, uint32_t target) {
  if (block->tail()->opcode() == SpvOpReturn ||
      block->tail()->opcode() == SpvOpReturnValue) {
    RecordReturned(block);
    RecordReturnValue(block);
  }

  // Fix up existing phi nodes.
  //
  // A new edge is being added from |block| to |target|, so go through
  // |target|'s phi nodes add an undef incoming value for |block|.
  BasicBlock* target_block = context()->get_instr_block(target);
  target_block->ForEachPhiInst([this, block](Instruction* inst) {
    uint32_t undefId = Type2Undef(inst->type_id());
    inst->AddOperand({SPV_OPERAND_TYPE_ID, {undefId}});
    inst->AddOperand({SPV_OPERAND_TYPE_ID, {block->id()}});
    context()->UpdateDefUse(inst);
  });

  const auto& target_pred = cfg()->preds(target);
  if (target_pred.size() == 1) {
    MarkForNewPhiNodes(target_block,
                       context()->get_instr_block(target_pred[0]));
  }

  Instruction* return_inst = block->terminator();
  return_inst->SetOpcode(SpvOpBranch);
  return_inst->ReplaceOperands({{SPV_OPERAND_TYPE_ID, {target}}});
  context()->get_def_use_mgr()->AnalyzeInstDefUse(return_inst);
  cfg()->AddEdge(block->id(), target);
}

void MergeReturnPass::CreatePhiNodesForInst(BasicBlock* merge_block,
                                            uint32_t predecessor,
                                            Instruction& inst) {
  DominatorAnalysis* dom_tree =
      context()->GetDominatorAnalysis(merge_block->GetParent());
  BasicBlock* inst_bb = context()->get_instr_block(&inst);

  if (inst.result_id() != 0) {
    std::vector<Instruction*> users_to_update;
    context()->get_def_use_mgr()->ForEachUser(
        &inst, [&users_to_update, &dom_tree, inst_bb, this](Instruction* user) {
          BasicBlock* user_bb = context()->get_instr_block(user);
          // If |user_bb| is nullptr, then |user| is not in the function.  It is
          // something like an OpName or decoration, which should not be
          // replaced with the result of the OpPhi.
          if (user_bb && !dom_tree->Dominates(inst_bb, user_bb)) {
            users_to_update.push_back(user);
          }
        });

    if (users_to_update.empty()) {
      return;
    }

    // There is at least one values that needs to be replaced.
    // First create the OpPhi instruction.
    InstructionBuilder builder(context(), &*merge_block->begin(),
                               IRContext::kAnalysisDefUse);
    uint32_t undef_id = Type2Undef(inst.type_id());
    std::vector<uint32_t> phi_operands;

    // Add the operands for the defining instructions.
    phi_operands.push_back(inst.result_id());
    phi_operands.push_back(predecessor);

    // Add undef from all other blocks.
    std::vector<uint32_t> preds = cfg()->preds(merge_block->id());
    for (uint32_t pred_id : preds) {
      if (pred_id != predecessor) {
        phi_operands.push_back(undef_id);
        phi_operands.push_back(pred_id);
      }
    }

    Instruction* new_phi = builder.AddPhi(inst.type_id(), phi_operands);
    uint32_t result_of_phi = new_phi->result_id();

    // Update all of the users to use the result of the new OpPhi.
    for (Instruction* user : users_to_update) {
      user->ForEachInId([&inst, result_of_phi](uint32_t* id) {
        if (*id == inst.result_id()) {
          *id = result_of_phi;
        }
      });
      context()->AnalyzeUses(user);
    }
  }
}

void MergeReturnPass::PredicateBlocks(
    const std::vector<BasicBlock*>& return_blocks) {
  // The CFG is being modified as the function proceeds so avoid caching
  // successors.
  std::vector<BasicBlock*> stack;
  auto add_successors = [this, &stack](BasicBlock* block) {
    const BasicBlock* const_block = const_cast<const BasicBlock*>(block);
    const_block->ForEachSuccessorLabel([this, &stack](const uint32_t idx) {
      stack.push_back(context()->get_instr_block(idx));
    });
  };

  std::unordered_set<BasicBlock*> seen;
  std::unordered_set<BasicBlock*> predicated;
  for (auto b : return_blocks) {
    seen.clear();
    add_successors(b);

    while (!stack.empty()) {
      BasicBlock* block = stack.back();
      assert(block);
      stack.pop_back();

      if (block == b) continue;
      if (block == final_return_block_) continue;
      if (!seen.insert(block).second) continue;
      if (!predicated.insert(block).second) continue;

      // Skip structured subgraphs.
      BasicBlock* next = block;
      while (next->GetMergeInst()) {
        next = context()->get_instr_block(next->MergeBlockIdIfAny());
      }
      add_successors(next);
      PredicateBlock(block, next, &predicated);
    }
  }
}

bool MergeReturnPass::RequiresPredication(const BasicBlock* block,
                                          const BasicBlock* tail_block) const {
  // This is intentionally conservative.
  // TODO(alanbaker): re-visit this when more performance data is available.
  if (block != tail_block) return true;

  bool requires_predicate = false;
  block->ForEachInst([&requires_predicate](const Instruction* inst) {
    if (inst->opcode() != SpvOpPhi && inst->opcode() != SpvOpLabel &&
        !IsTerminatorInst(inst->opcode())) {
      requires_predicate = true;
    }
  });
  return requires_predicate;
}

void MergeReturnPass::PredicateBlock(
    BasicBlock* block, BasicBlock* tail_block,
    std::unordered_set<BasicBlock*>* predicated) {
  if (!RequiresPredication(block, tail_block)) {
    return;
  }

  // Make sure the cfg is build here.  If we don't then it becomes very hard to
  // know which new blocks need to be updated.
  context()->BuildInvalidAnalyses(IRContext::kAnalysisCFG);

  // When predicating, be aware of whether this block is a header block, a merge
  // block or both.
  //
  // If this block is a merge block, ensure the appropriate header stays
  // up-to-date with any changes (i.e. points to the pre-header).
  //
  // If this block is a header block, predicate the entire structured subgraph.
  // This can act recursively.

  // If |block| is a loop header, then the back edge must jump to the original
  // code, not the new header.
  if (block->GetLoopMergeInst()) {
    cfg()->SplitLoopHeader(block);
  }

  // Leave the phi instructions behind.
  auto iter = block->begin();
  while (iter->opcode() == SpvOpPhi) {
    ++iter;
  }

  // Forget about the edges leaving block.  They will be removed.
  cfg()->RemoveSuccessorEdges(block);

  std::unique_ptr<BasicBlock> new_block(
      block->SplitBasicBlock(context(), TakeNextId(), iter));
  BasicBlock* old_body =
      function_->InsertBasicBlockAfter(std::move(new_block), block);
  predicated->insert(old_body);

  if (tail_block == block) {
    tail_block = old_body;
  }

  const BasicBlock* const_old_body = static_cast<const BasicBlock*>(old_body);
  const_old_body->ForEachSuccessorLabel(
      [old_body, block, this](const uint32_t label) {
        BasicBlock* target_bb = context()->get_instr_block(label);
        if (MarkedSinglePred(target_bb) == block) {
          MarkForNewPhiNodes(target_bb, old_body);
        }
      });

  std::unique_ptr<BasicBlock> new_merge_block(new BasicBlock(
      MakeUnique<Instruction>(context(), SpvOpLabel, 0, TakeNextId(),
                              std::initializer_list<Operand>{})));

  BasicBlock* new_merge =
      function_->InsertBasicBlockAfter(std::move(new_merge_block), tail_block);
  predicated->insert(new_merge);
  new_merge->SetParent(function_);

  // Register the new label.
  get_def_use_mgr()->AnalyzeInstDef(new_merge->GetLabelInst());
  context()->set_instr_block(new_merge->GetLabelInst(), new_merge);

  // Move the tail branch into the new merge and fix the mapping. If a single
  // block is being predicated then its branch was moved to the old body
  // previously.
  std::unique_ptr<Instruction> inst;
  Instruction* i = tail_block->terminator();
  cfg()->RemoveSuccessorEdges(tail_block);
  get_def_use_mgr()->ClearInst(i);
  inst.reset(std::move(i));
  inst->RemoveFromList();
  new_merge->end().InsertBefore(std::move(inst));
  get_def_use_mgr()->AnalyzeInstUse(new_merge->terminator());
  context()->set_instr_block(new_merge->terminator(), new_merge);

  // Add a branch to the new merge. If we jumped multiple blocks, the branch is
  // added to tail_block, otherwise the branch belongs in old_body.
  tail_block->AddInstruction(
      MakeUnique<Instruction>(context(), SpvOpBranch, 0, 0,
                              std::initializer_list<Operand>{
                                  {SPV_OPERAND_TYPE_ID, {new_merge->id()}}}));
  get_def_use_mgr()->AnalyzeInstUse(tail_block->terminator());
  context()->set_instr_block(tail_block->terminator(), tail_block);

  // Within the new header we need the following:
  // 1. Load of the return status flag
  // 2. Declare the merge block
  // 3. Branch to new merge (true) or old body (false)

  // 1. Load of the return status flag
  analysis::Bool bool_type;
  uint32_t bool_id = context()->get_type_mgr()->GetId(&bool_type);
  assert(bool_id != 0);
  uint32_t load_id = TakeNextId();
  block->AddInstruction(MakeUnique<Instruction>(
      context(), SpvOpLoad, bool_id, load_id,
      std::initializer_list<Operand>{
          {SPV_OPERAND_TYPE_ID, {return_flag_->result_id()}}}));
  get_def_use_mgr()->AnalyzeInstDefUse(block->terminator());
  context()->set_instr_block(block->terminator(), block);

  // 2. Declare the merge block
  block->AddInstruction(MakeUnique<Instruction>(
      context(), SpvOpSelectionMerge, 0, 0,
      std::initializer_list<Operand>{{SPV_OPERAND_TYPE_ID, {new_merge->id()}},
                                     {SPV_OPERAND_TYPE_SELECTION_CONTROL,
                                      {SpvSelectionControlMaskNone}}}));
  get_def_use_mgr()->AnalyzeInstUse(block->terminator());
  context()->set_instr_block(block->terminator(), block);

  // 3. Branch to new merge (true) or old body (false)
  block->AddInstruction(MakeUnique<Instruction>(
      context(), SpvOpBranchConditional, 0, 0,
      std::initializer_list<Operand>{{SPV_OPERAND_TYPE_ID, {load_id}},
                                     {SPV_OPERAND_TYPE_ID, {new_merge->id()}},
                                     {SPV_OPERAND_TYPE_ID, {old_body->id()}}}));
  get_def_use_mgr()->AnalyzeInstUse(block->terminator());
  context()->set_instr_block(block->terminator(), block);

  assert(old_body->begin() != old_body->end());
  assert(block->begin() != block->end());
  assert(new_merge->begin() != new_merge->end());

  // Update the cfg
  cfg()->AddEdges(block);
  cfg()->RegisterBlock(old_body);
  if (old_body != tail_block) {
    cfg()->AddEdges(tail_block);
  }
  cfg()->RegisterBlock(new_merge);
  MarkForNewPhiNodes(new_merge, tail_block);
}

void MergeReturnPass::RecordReturned(BasicBlock* block) {
  if (block->tail()->opcode() != SpvOpReturn &&
      block->tail()->opcode() != SpvOpReturnValue)
    return;

  assert(return_flag_ && "Did not generate the return flag variable.");

  if (!constant_true_) {
    analysis::Bool temp;
    const analysis::Bool* bool_type =
        context()->get_type_mgr()->GetRegisteredType(&temp)->AsBool();

    analysis::ConstantManager* const_mgr = context()->get_constant_mgr();
    const analysis::Constant* true_const =
        const_mgr->GetConstant(bool_type, {true});
    constant_true_ = const_mgr->GetDefiningInstruction(true_const);
    context()->UpdateDefUse(constant_true_);
  }

  std::unique_ptr<Instruction> return_store(new Instruction(
      context(), SpvOpStore, 0, 0,
      std::initializer_list<Operand>{
          {SPV_OPERAND_TYPE_ID, {return_flag_->result_id()}},
          {SPV_OPERAND_TYPE_ID, {constant_true_->result_id()}}}));

  Instruction* store_inst =
      &*block->tail().InsertBefore(std::move(return_store));
  context()->set_instr_block(store_inst, block);
  context()->AnalyzeDefUse(store_inst);
}

void MergeReturnPass::RecordReturnValue(BasicBlock* block) {
  auto terminator = *block->tail();
  if (terminator.opcode() != SpvOpReturnValue) {
    return;
  }

  assert(return_value_ &&
         "Did not generate the variable to hold the return value.");

  std::unique_ptr<Instruction> value_store(new Instruction(
      context(), SpvOpStore, 0, 0,
      std::initializer_list<Operand>{
          {SPV_OPERAND_TYPE_ID, {return_value_->result_id()}},
          {SPV_OPERAND_TYPE_ID, {terminator.GetSingleWordInOperand(0u)}}}));

  Instruction* store_inst =
      &*block->tail().InsertBefore(std::move(value_store));
  context()->set_instr_block(store_inst, block);
  context()->AnalyzeDefUse(store_inst);
}

void MergeReturnPass::AddReturnValue() {
  if (return_value_) return;

  uint32_t return_type_id = function_->type_id();
  if (get_def_use_mgr()->GetDef(return_type_id)->opcode() == SpvOpTypeVoid)
    return;

  uint32_t return_ptr_type = context()->get_type_mgr()->FindPointerToType(
      return_type_id, SpvStorageClassFunction);

  uint32_t var_id = TakeNextId();
  std::unique_ptr<Instruction> returnValue(new Instruction(
      context(), SpvOpVariable, return_ptr_type, var_id,
      std::initializer_list<Operand>{
          {SPV_OPERAND_TYPE_STORAGE_CLASS, {SpvStorageClassFunction}}}));

  auto insert_iter = function_->begin()->begin();
  insert_iter.InsertBefore(std::move(returnValue));
  BasicBlock* entry_block = &*function_->begin();
  return_value_ = &*entry_block->begin();
  context()->AnalyzeDefUse(return_value_);
  context()->set_instr_block(return_value_, entry_block);
}

void MergeReturnPass::AddReturnFlag() {
  if (return_flag_) return;

  analysis::TypeManager* type_mgr = context()->get_type_mgr();
  analysis::ConstantManager* const_mgr = context()->get_constant_mgr();

  analysis::Bool temp;
  uint32_t bool_id = type_mgr->GetTypeInstruction(&temp);
  analysis::Bool* bool_type = type_mgr->GetType(bool_id)->AsBool();

  const analysis::Constant* false_const =
      const_mgr->GetConstant(bool_type, {false});
  uint32_t const_false_id =
      const_mgr->GetDefiningInstruction(false_const)->result_id();

  uint32_t bool_ptr_id =
      type_mgr->FindPointerToType(bool_id, SpvStorageClassFunction);

  uint32_t var_id = TakeNextId();
  std::unique_ptr<Instruction> returnFlag(new Instruction(
      context(), SpvOpVariable, bool_ptr_id, var_id,
      std::initializer_list<Operand>{
          {SPV_OPERAND_TYPE_STORAGE_CLASS, {SpvStorageClassFunction}},
          {SPV_OPERAND_TYPE_ID, {const_false_id}}}));

  auto insert_iter = function_->begin()->begin();

  insert_iter.InsertBefore(std::move(returnFlag));
  BasicBlock* entry_block = &*function_->begin();
  return_flag_ = &*entry_block->begin();
  context()->AnalyzeDefUse(return_flag_);
  context()->set_instr_block(return_flag_, entry_block);
}

std::vector<BasicBlock*> MergeReturnPass::CollectReturnBlocks(
    Function* function) {
  std::vector<BasicBlock*> return_blocks;
  for (auto& block : *function) {
    Instruction& terminator = *block.tail();
    if (terminator.opcode() == SpvOpReturn ||
        terminator.opcode() == SpvOpReturnValue) {
      return_blocks.push_back(&block);
    }
  }
  return return_blocks;
}

void MergeReturnPass::MergeReturnBlocks(
    Function* function, const std::vector<BasicBlock*>& return_blocks) {
  if (return_blocks.size() <= 1) {
    // No work to do.
    return;
  }

  CreateReturnBlock();
  uint32_t return_id = final_return_block_->id();
  auto ret_block_iter = --function->end();
  // Create the PHI for the merged block (if necessary).
  // Create new return.
  std::vector<Operand> phi_ops;
  for (auto block : return_blocks) {
    if (block->tail()->opcode() == SpvOpReturnValue) {
      phi_ops.push_back(
          {SPV_OPERAND_TYPE_ID, {block->tail()->GetSingleWordInOperand(0u)}});
      phi_ops.push_back({SPV_OPERAND_TYPE_ID, {block->id()}});
    }
  }

  if (!phi_ops.empty()) {
    // Need a PHI node to select the correct return value.
    uint32_t phi_result_id = TakeNextId();
    uint32_t phi_type_id = function->type_id();
    std::unique_ptr<Instruction> phi_inst(new Instruction(
        context(), SpvOpPhi, phi_type_id, phi_result_id, phi_ops));
    ret_block_iter->AddInstruction(std::move(phi_inst));
    BasicBlock::iterator phiIter = ret_block_iter->tail();

    std::unique_ptr<Instruction> return_inst(
        new Instruction(context(), SpvOpReturnValue, 0u, 0u,
                        {{SPV_OPERAND_TYPE_ID, {phi_result_id}}}));
    ret_block_iter->AddInstruction(std::move(return_inst));
    BasicBlock::iterator ret = ret_block_iter->tail();

    // Register the phi def and mark instructions for use updates.
    get_def_use_mgr()->AnalyzeInstDefUse(&*phiIter);
    get_def_use_mgr()->AnalyzeInstDef(&*ret);
  } else {
    std::unique_ptr<Instruction> return_inst(
        new Instruction(context(), SpvOpReturn));
    ret_block_iter->AddInstruction(std::move(return_inst));
  }

  // Replace returns with branches
  for (auto block : return_blocks) {
    context()->ForgetUses(block->terminator());
    block->tail()->SetOpcode(SpvOpBranch);
    block->tail()->ReplaceOperands({{SPV_OPERAND_TYPE_ID, {return_id}}});
    get_def_use_mgr()->AnalyzeInstUse(block->terminator());
    get_def_use_mgr()->AnalyzeInstUse(block->GetLabelInst());
  }

  get_def_use_mgr()->AnalyzeInstDefUse(ret_block_iter->GetLabelInst());
}

void MergeReturnPass::AddNewPhiNodes() {
  DominatorAnalysis* dom_tree = context()->GetDominatorAnalysis(function_);
  std::list<BasicBlock*> order;
  cfg()->ComputeStructuredOrder(function_, &*function_->begin(), &order);

  for (BasicBlock* bb : order) {
    AddNewPhiNodes(bb, new_merge_nodes_[bb],
                   dom_tree->ImmediateDominator(bb)->id());
  }
}

void MergeReturnPass::AddNewPhiNodes(BasicBlock* bb, BasicBlock* pred,
                                     uint32_t header_id) {
  DominatorAnalysis* dom_tree = context()->GetDominatorAnalysis(function_);
  // Insert as a stopping point.  We do not have to add anything in the block or
  // above because the header dominates |bb|.

  BasicBlock* current_bb = pred;
  while (current_bb != nullptr && current_bb->id() != header_id) {
    for (Instruction& inst : *current_bb) {
      CreatePhiNodesForInst(bb, pred->id(), inst);
    }
    current_bb = dom_tree->ImmediateDominator(current_bb);
  }
}

void MergeReturnPass::MarkForNewPhiNodes(BasicBlock* block,
                                         BasicBlock* single_original_pred) {
  new_merge_nodes_[block] = single_original_pred;
}

}  // namespace opt
}  // namespace spvtools
