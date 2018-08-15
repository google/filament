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

#ifndef SOURCE_OPT_INLINE_PASS_H_
#define SOURCE_OPT_INLINE_PASS_H_

#include <algorithm>
#include <list>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

#include "source/opt/decoration_manager.h"
#include "source/opt/module.h"
#include "source/opt/pass.h"

namespace spvtools {
namespace opt {

// See optimizer.hpp for documentation.
class InlinePass : public Pass {
  using cbb_ptr = const BasicBlock*;

 public:
  using GetBlocksFunction =
      std::function<std::vector<BasicBlock*>*(const BasicBlock*)>;

  virtual ~InlinePass() = default;

 protected:
  InlinePass();

  // Add pointer to type to module and return resultId.
  uint32_t AddPointerToType(uint32_t type_id, SpvStorageClass storage_class);

  // Add unconditional branch to labelId to end of block block_ptr.
  void AddBranch(uint32_t labelId, std::unique_ptr<BasicBlock>* block_ptr);

  // Add conditional branch to end of block |block_ptr|.
  void AddBranchCond(uint32_t cond_id, uint32_t true_id, uint32_t false_id,
                     std::unique_ptr<BasicBlock>* block_ptr);

  // Add unconditional branch to labelId to end of block block_ptr.
  void AddLoopMerge(uint32_t merge_id, uint32_t continue_id,
                    std::unique_ptr<BasicBlock>* block_ptr);

  // Add store of valId to ptrId to end of block block_ptr.
  void AddStore(uint32_t ptrId, uint32_t valId,
                std::unique_ptr<BasicBlock>* block_ptr);

  // Add load of ptrId into resultId to end of block block_ptr.
  void AddLoad(uint32_t typeId, uint32_t resultId, uint32_t ptrId,
               std::unique_ptr<BasicBlock>* block_ptr);

  // Return new label.
  std::unique_ptr<Instruction> NewLabel(uint32_t label_id);

  // Returns the id for the boolean false value. Looks in the module first
  // and creates it if not found. Remembers it for future calls.
  uint32_t GetFalseId();

  // Map callee params to caller args
  void MapParams(Function* calleeFn, BasicBlock::iterator call_inst_itr,
                 std::unordered_map<uint32_t, uint32_t>* callee2caller);

  // Clone and map callee locals
  void CloneAndMapLocals(Function* calleeFn,
                         std::vector<std::unique_ptr<Instruction>>* new_vars,
                         std::unordered_map<uint32_t, uint32_t>* callee2caller);

  // Create return variable for callee clone code if needed. Return id
  // if created, otherwise 0.
  uint32_t CreateReturnVar(Function* calleeFn,
                           std::vector<std::unique_ptr<Instruction>>* new_vars);

  // Return true if instruction must be in the same block that its result
  // is used.
  bool IsSameBlockOp(const Instruction* inst) const;

  // Clone operands which must be in same block as consumer instructions.
  // Look in preCallSB for instructions that need cloning. Look in
  // postCallSB for instructions already cloned. Add cloned instruction
  // to postCallSB.
  void CloneSameBlockOps(std::unique_ptr<Instruction>* inst,
                         std::unordered_map<uint32_t, uint32_t>* postCallSB,
                         std::unordered_map<uint32_t, Instruction*>* preCallSB,
                         std::unique_ptr<BasicBlock>* block_ptr);

  // Return in new_blocks the result of inlining the call at call_inst_itr
  // within its block at call_block_itr. The block at call_block_itr can
  // just be replaced with the blocks in new_blocks. Any additional branches
  // are avoided. Debug instructions are cloned along with their callee
  // instructions. Early returns are replaced by a store to a local return
  // variable and a branch to a (created) exit block where the local variable
  // is returned. Formal parameters are trivially mapped to their actual
  // parameters. Note that the first block in new_blocks retains the label
  // of the original calling block. Also note that if an exit block is
  // created, it is the last block of new_blocks.
  //
  // Also return in new_vars additional OpVariable instructions required by
  // and to be inserted into the caller function after the block at
  // call_block_itr is replaced with new_blocks.
  void GenInlineCode(std::vector<std::unique_ptr<BasicBlock>>* new_blocks,
                     std::vector<std::unique_ptr<Instruction>>* new_vars,
                     BasicBlock::iterator call_inst_itr,
                     UptrVectorIterator<BasicBlock> call_block_itr);

  // Return true if |inst| is a function call that can be inlined.
  bool IsInlinableFunctionCall(const Instruction* inst);

  // Compute structured successors for function |func|.
  // A block's structured successors are the blocks it branches to
  // together with its declared merge block if it has one.
  // When order matters, the merge block always appears first.
  // This assures correct depth first search in the presence of early
  // returns and kills. If the successor vector contain duplicates
  // if the merge block, they are safely ignored by DFS.
  void ComputeStructuredSuccessors(Function* func);

  // Return function to return ordered structure successors for a given block
  // Assumes ComputeStructuredSuccessors() has been called.
  GetBlocksFunction StructuredSuccessorsFunction();

  // Return true if |func| has multiple returns
  bool HasMultipleReturns(Function* func);

  // Return true if |func| has no return in a loop. The current analysis
  // requires structured control flow, so return false if control flow not
  // structured ie. module is not a shader.
  bool HasNoReturnInLoop(Function* func);

  // Find all functions with multiple returns and no returns in loops
  void AnalyzeReturns(Function* func);

  // Return true if |func| is a function that can be inlined.
  bool IsInlinableFunction(Function* func);

  // Update phis in succeeding blocks to point to new last block
  void UpdateSucceedingPhis(
      std::vector<std::unique_ptr<BasicBlock>>& new_blocks);

  // Initialize state for optimization of |module|
  void InitializeInline();

  // Map from function's result id to function.
  std::unordered_map<uint32_t, Function*> id2function_;

  // Map from block's label id to block. TODO(dnovillo): This is superfluous wrt
  // CFG. It has functionality not present in CFG. Consolidate.
  std::unordered_map<uint32_t, BasicBlock*> id2block_;

  // Set of ids of functions with multiple returns.
  std::set<uint32_t> multi_return_funcs_;

  // Set of ids of functions with no returns in loop
  std::set<uint32_t> no_return_in_loop_;

  // Set of ids of inlinable functions
  std::set<uint32_t> inlinable_;

  // result id for OpConstantFalse
  uint32_t false_id_;

  // Map from block to its structured successor blocks. See
  // ComputeStructuredSuccessors() for definition. TODO(dnovillo): This is
  // superfluous wrt CFG, but it seems to be computed in a slightly
  // different way in the inliner. Can these be consolidated?
  std::unordered_map<const BasicBlock*, std::vector<BasicBlock*>>
      block2structured_succs_;
};

}  // namespace opt
}  // namespace spvtools

#endif  // SOURCE_OPT_INLINE_PASS_H_
