//===- DxilRemoveUnstructuredLoopExits.cpp - Make unrolled loops structured
//---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//
// Loops that look like the following when unrolled becomes unstructured:
//
//      for(;;) {
//        if (a) {
//          if (b) {
//            exit_code_0;
//            break;       // Unstructured loop exit
//          }
//
//          code_0;
//
//          if (c) {
//            if (d) {
//              exit_code_1;
//              break;    // Unstructured loop exit
//            }
//            code_1;
//          }
//
//          code_2;
//
//          ...
//        }
//
//        code_3;
//
//        if (exit)
//          break;
//      }
//
//
// This pass transforms the loop into the following form:
//
//      bool broke_0 = false;
//      bool broke_1 = false;
//
//      for(;;) {
//        if (a) {
//          if (b) {
//            broke_0 = true;       // Break flag
//          }
//
//          if (!broke_0) {
//            code_0;
//          }
//
//          if (!broke_0) {
//            if (c) {
//              if (d) {
//                broke_1 = true;   // Break flag
//              }
//              if (!broke_1) {
//                code_1;
//              }
//            }
//
//            if (!broke_1) {
//              code_2;
//            }
//          }
//
//          ...
//        }
//
//        if (broke_0) {
//          break;
//        }
//
//        if (broke_1) {
//          break;
//        }
//
//        code_3;
//
//        if (exit)
//          break;
//      }
//
//      if (broke_0) {
//        exit_code_0;
//      }
//
//      if (broke_1) {
//        exit_code_1;
//      }
//
// Essentially it hoists the exit branch out of the loop:
//   - That is, any exiting block must dominate the latch block.
//   - All exits go through a single latch-exit block.
//   - The action of the exiting blocks are deferred and conditionally
//     executed after reaching the latch-exit block.
//
// This function should be called any time before a function is unrolled to
// avoid generating unstructured code.
//
// There are several limitations at the moment:
//
//   - if code_0, code_1, etc has any loops in there, this transform
//     does not take place. Since the values that flow out of the conditions
//     are phi of undef, I do not want to risk the loops not exiting.
//
//   - code_0, code_1, etc, become conditional only when there are
//     side effects in there. This doesn't impact code correctness,
//     but the code will execute for one iteration even if the exit condition
//     is met.
//
//   - If any exiting block uses a switch statement to conditionally exit the
//     loop, we currently do not handle that case.
//
// These limitations can be fixed in the future as needed.
//
//===----------------------------------------------------------------------===//

#include "dxc/HLSL/DxilNoops.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

#include <unordered_map>
#include <unordered_set>

#include "DxilRemoveUnstructuredLoopExits.h"

using namespace llvm;

#define DEBUG_TYPE "dxil-remove-unstructured-loop-exits"

namespace {

bool IsNoop(Instruction *inst) {
  if (CallInst *ci = dyn_cast<CallInst>(inst)) {
    if (Function *f = ci->getCalledFunction()) {
      return f->getName() == hlsl::kNoopName;
    }
  }
  return false;
}

bool HasSideEffects(BasicBlock *bb) {
  for (Instruction &I : *bb) {
    if (I.mayReadOrWriteMemory() && !IsNoop(&I)) {
      return true;
    }
  }
  return false;
}

// Captures information about a value which is propagated from the exiting block
// to the exit block.  A special 'exiting-condition' case occurs when the
// value is the condition on the exiting branch; by prior arrangement the
// exit path is taken when the value is 'true'.
struct Value_Info {
  // The value from the exiting block.
  Value *val;
  // The False value, if 'val' is the exiting-condition value.
  // Otherwise, a default value for the 'val's type.
  Value *false_val;
  // nullptr if 'val' is the exiting-condition and not otherwise propagated into
  // the exit block. Otherwise, this is the single-input phi that carries 'val'
  // into the exit block.  The LCSSA form guarantees this exits for any value
  // carried out of the loop via this exit path.
  PHINode *exit_phi;
};

// A Propagator does the following:
//  - Let EB be an exiting block for a loop.
//  - Let exit_values be the values that EB sends out of the loop to its
//    corresponding exit block.
//  - The Run method:
//     - Traverses blocks all the loop reachable from EB, stopping at
//       a block that dominates the loop latch.
//       (The stopping block is unique, by a contradiction argument.)
//     - Modifies traversed blocks to add phi nodes to propagate values
//       from exit_values
//     - Remembers which phi node is used to propagate each exit value
//       to each traversed block.
//  - The Get method is used to look up those phi nodes.
//
// The Run method can fail, in which case it cleans up after itself by
// removing the phi nodes it may have added in the meantime.
struct Propagator {
  // Maps a {block B, value V} to the phi that is used to get V in B.
  DenseMap<std::pair<BasicBlock *, Value *>, PHINode *> cached_phis;

  // The set of blocks visited. Traversals start at an exiting block, then
  // follow successors that are in the same loop. Stop when we reach a block
  // that dominates the latch block. That block is unique:  otherwise there
  // would be different such blocks X and Y that dominate the latch, but not
  // each other. That's a contradiction.)
  // The algorithm stops early (and fails) if any traversed blocks are part
  // of an *inner* loop differnt from L.
  std::unordered_set<BasicBlock *> seen;

  // Get propagated value for val. It's guaranteed to be safe to use in bb.
  Value *Get(Value *val, BasicBlock *bb) {
    auto it = cached_phis.find({bb, val});
    if (it == cached_phis.end())
      return nullptr;

    return it->second;
  }

  // Erase any phis that may have been created, and forget them.
  void DeleteAllNewValues() {
    for (auto &pair : cached_phis) {
      pair.second->dropAllReferences();
    }
    for (auto &pair : cached_phis) {
      pair.second->eraseFromParent();
    }
    cached_phis.clear();
  }

  // Given loop L, and exiting block EB, take all exit values from EB
  // and try to propagate them into other blocks in L reachable from EB,
  // stopping at a block that dominates the loop latch.  (Only one
  // such block dominates the loop latch; otherwise there would be
  // different such blocks X and Y that dominate the latch, but not
  // each other. That's a contradiction.)
  // Assumes EB does not dominate the latch.
  // Exit values are propagated using phis.
  // Also collect the traversed blocks that have side effects, other
  // than the initial exiting block.
  // Fail if the traversal finds a block in L that is also in an (inner)
  // loop contained inside L.  Failure is signaled by returning null.
  // On success return the found block that dominates the latch.
  BasicBlock *Run(const SmallVector<Value_Info, 8> &exit_values,
                  BasicBlock *exiting_block, BasicBlock *latch,
                  DominatorTree *DT, Loop *L, LoopInfo *LI,
                  std::vector<BasicBlock *> &blocks_with_side_effect) {
    BasicBlock *ret = RunImpl(exit_values, exiting_block, latch, DT, L, LI,
                              blocks_with_side_effect);
    // If we failed, remove all the values we added.
    if (!ret) {
      DeleteAllNewValues();
    }
    return ret;
  }

  BasicBlock *RunImpl(const SmallVector<Value_Info, 8> &exit_values,
                      BasicBlock *exiting_block, BasicBlock *latch,
                      DominatorTree *DT, Loop *L, LoopInfo *LI,
                      std::vector<BasicBlock *> &blocks_with_side_effect) {

    struct Edge {
      BasicBlock *prev;
      BasicBlock *bb;
    };

    BasicBlock *new_exiting_block = nullptr;
    SmallVector<Edge, 4> work_list;
    work_list.push_back({nullptr, exiting_block});
    seen.insert(exiting_block);

    for (unsigned i = 0; i < work_list.size(); i++) {
      auto &edge = work_list[i];
      BasicBlock *prev = edge.prev;
      BasicBlock *bb = edge.bb;

      // Don't continue to propagate when we hit the latch or dominate it.
      if (DT->dominates(bb, latch)) {
        new_exiting_block = bb;
        continue;
      }

      // Do not include the exiting block itself in this calculation
      if (prev != nullptr) {
        // If this block is part of an inner loop... Give up for now.
        if (LI->getLoopFor(bb) != L) {
          return nullptr;
        }
        // Otherwise remember the blocks with side effects (including the
        // latch)
        if (HasSideEffects(bb)) {
          blocks_with_side_effect.push_back(bb);
        }
      }

      for (BasicBlock *succ : llvm::successors(bb)) {
        // Don't propagate if block is not part of this loop.
        if (!L->contains(succ))
          continue;

        for (const auto &ev : exit_values) {
          // Find or create phi for the value in the successor block
          PHINode *phi = cached_phis[{succ, ev.val}];
          if (!phi) {
            // Create a phi node with all dummy values for now.
            phi = PHINode::Create(ev.false_val->getType(), 0,
                                  "dx.struct_exit.prop", &*succ->begin());
            for (BasicBlock *pred : llvm::predecessors(succ)) {
              phi->addIncoming(ev.false_val, pred);
            }
            cached_phis[{succ, ev.val}] = phi;
          }

          // Find the incoming value for successor block
          Value *incoming = nullptr;
          if (!prev) {
            incoming = ev.val;
          } else {
            incoming = cached_phis[{bb, ev.val}];
          }

          // Set incoming value for our phi
          for (unsigned i = 0; i < phi->getNumIncomingValues(); i++) {
            if (phi->getIncomingBlock(i) == bb) {
              phi->setIncomingValue(i, incoming);
            }
          }

          // Add to worklist
          if (!seen.count(succ)) {
            work_list.push_back({bb, succ});
            seen.insert(succ);
          }
        }
      } // for each succ
    }   // for each in worklist

    if (new_exiting_block == exiting_block) {
      return nullptr;
    }

    return new_exiting_block;
  }
}; // struct Propagator

} // Unnamed namespace

static Value *GetDefaultValue(Type *type) {
  if (type->isIntegerTy()) {
    return ConstantInt::get(type, 0);
  } else if (type->isFloatingPointTy()) {
    return ConstantFP::get(type, 0);
  }
  return UndefValue::get(type);
}

static BasicBlock *GetExitBlockForExitingBlock(Loop *L,
                                               BasicBlock *exiting_block) {
  BranchInst *br = dyn_cast<BranchInst>(exiting_block->getTerminator());
  assert(L->contains(exiting_block));
  assert(br->isConditional());
  BasicBlock *result = L->contains(br->getSuccessor(0)) ? br->getSuccessor(1)
                                                        : br->getSuccessor(0);
  assert(!L->contains(result));
  return result;
}

// Branch over the block's content when skip_cond is true.
// All values used outside the block are replaced by a phi.
static void SkipBlockWithBranch(BasicBlock *bb, Value *skip_cond, Loop *L,
                                LoopInfo *LI) {
  BasicBlock *body = bb->splitBasicBlock(bb->getFirstNonPHI());
  body->setName("dx.struct_exit.cond_body");
  BasicBlock *end = body->splitBasicBlock(body->getTerminator());
  end->setName("dx.struct_exit.cond_end");

  bb->getTerminator()->eraseFromParent();
  BranchInst::Create(end, body, skip_cond, bb);

  for (Instruction &inst : *body) {

    // For each user that's outside of 'body', replace its use of 'inst' with a
    // phi created in 'end'
    SmallPtrSet<Instruction *, 8> users_in_other_blocks;
    for (auto *user : inst.users()) {
      Instruction *user_inst = cast<Instruction>(user);
      if (user_inst->getParent() != body) {
        users_in_other_blocks.insert(user_inst);
      }
    }
    if (users_in_other_blocks.size() > 0) {
      auto *phi = PHINode::Create(inst.getType(), 2, "", &*end->begin());
      phi->addIncoming(GetDefaultValue(inst.getType()), bb);
      phi->addIncoming(&inst, body);

      for (auto *user_inst : users_in_other_blocks) {
        user_inst->replaceUsesOfWith(&inst, phi);
      }
    }
  } // For each inst in body

  L->addBasicBlockToLoop(body, *LI);
  L->addBasicBlockToLoop(end, *LI);
}

static unsigned GetNumPredecessors(BasicBlock *bb) {
  unsigned ret = 0;
  for (BasicBlock *pred : llvm::predecessors(bb)) {
    (void)pred;
    ret++;
  }
  return ret;
}

// Returns a vector of Value_Info:
//  - one for each value carried from the loop into the exit block via the
//  exiting block.
//  - one for the new exit condition (the one that will be used to exit the
//    loop from a block later in the loop body)
static SmallVector<Value_Info, 8> CollectExitValues(Value *new_exit_cond,
                                                    BasicBlock *exiting_block,
                                                    BasicBlock *exit_block) {

  SmallVector<Value_Info, 8> exit_values;

  // Look at the lcssa phi's in the exit block.
  bool exit_cond_has_phi = false;
  for (Instruction &I : *exit_block) {
    if (PHINode *phi = dyn_cast<PHINode>(&I)) {
      // If there are values flowing out of the loop into the exit_block,
      // add them to the list to be propagated
      Value *value = phi->getIncomingValueForBlock(exiting_block);
      Value *false_value = nullptr;
      if (value == new_exit_cond) {
        false_value = Constant::getNullValue(value->getType());
        exit_cond_has_phi = true;
      } else {
        false_value = GetDefaultValue(value->getType());
      }
      exit_values.push_back({value, false_value, phi});
    } else {
      break;
    }
  }

  // If the new exit condition is not among the exit phi's, add it.
  if (!exit_cond_has_phi) {
    exit_values.push_back({new_exit_cond,
                           Constant::getNullValue(new_exit_cond->getType()),
                           nullptr});
  }
  return exit_values;
}

// Restructures exiting_block so its work, including its exit branch, is moved
// to a block B that dominates the latch block. Let's call B the
// newly-exiting-block.
// Assumes the loop has a single latch block, and the terminator on that
// latch block is a conditional branch.
static bool RemoveUnstructuredLoopExitsIteration(BasicBlock *exiting_block,
                                                 Loop *L, LoopInfo *LI,
                                                 DominatorTree *DT) {
  BasicBlock *latch = L->getLoopLatch();
  BasicBlock *latch_exit = GetExitBlockForExitingBlock(L, latch);

  // Ensure the latch-exit is "dedicated": no block outside the loop
  // branches to it.
  //
  // Suppose this iteration successfully moves an exit block X until
  // after the latch block.  It will do so by rewiring the CFG so
  // the latch *exit* block will branch to X.  If the latch exit
  // block is already reachable from X, then the rewiring will
  // create an unwanted loop.
  // So prevent this from happening by ensuring the latch exit is
  // "dedicated": the only branches to it come from inside the
  // loop, and hence not from X.
  //
  // The latch_exit block could have *multiple* branches to it from
  // outside the loop.
  //
  // When the edge from latch to latch_exit is split, the local picture is:
  //
  //    latch --> middle --> tail
  //
  // where:
  //  - Branches that used to go to latch_exit, from outside the loop, now
  //    point to 'tail'.
  //  - 'middle' is now an exit block for the loop, and its only incoming
  //    edge is from latch.
  for (auto *pred : predecessors(latch_exit)) {
    if (!L->contains(pred)) {
      SplitEdge(latch, latch_exit, DT, LI);
      // Quit early and recalculate exit blocks.
      return true;
    }
  }

  BasicBlock *exit_block = GetExitBlockForExitingBlock(L, exiting_block);

  // If exiting block already dominates latch, then no need to do anything.
  if (DT->dominates(exiting_block, latch)) {
    return false;
  }

  Propagator prop;

  // The newly-exiting-block B will end in a conditional branch, with
  // the true branch exiting the loop, and the false branch falling through
  // (staying in the loop).
  // Compute the exit condition for B.
  BranchInst *exiting_br = cast<BranchInst>(exiting_block->getTerminator());
  Value *exit_if_true = exiting_br->getCondition();
  // When the original exit_block is the false block, use the negate the
  // condition.
  if (exiting_br->getSuccessor(1) == exit_block) {
    IRBuilder<> B(exiting_br);
    exit_if_true = B.CreateNot(exit_if_true);
  }

  // Collect relevant information about values that flow from this loop
  // into the exit block.
  const auto exit_values =
      CollectExitValues(exit_if_true, exiting_block, exit_block);

  //
  // Propagate those values we just found to a block that dominates the latch,
  // and return that final block.
  // Also, remember the blocks along the traversal that have side effects.
  // This can fail, signaled by returning null.
  std::vector<BasicBlock *> blocks_with_side_effect;
  BasicBlock *new_exiting_block = prop.Run(exit_values, exiting_block, latch,
                                           DT, L, LI, blocks_with_side_effect);

  // Stop now if we failed.
  if (!new_exiting_block)
    return false;

  // If any blocks on the traversal have side effects, skip them when the loop
  // should be exiting.
  for (BasicBlock *bb : blocks_with_side_effect) {
    Value *exit_cond_for_block = prop.Get(exit_if_true, bb);
    SkipBlockWithBranch(bb, exit_cond_for_block, L, LI);
  }

  // Make the exiting block not exit.
  {
    BasicBlock *non_exiting_block = exiting_br->getSuccessor(
        exiting_br->getSuccessor(0) == exit_block ? 1 : 0);
    BranchInst::Create(non_exiting_block, exiting_block);
    exiting_br->eraseFromParent();
    exiting_br = nullptr;
  }

  Value *new_exit_cond = prop.Get(exit_if_true, new_exiting_block);
  assert(new_exit_cond);

  // Split the block where we're now exiting from, and branch to latch exit
  std::string old_name = new_exiting_block->getName().str();
  BasicBlock *new_not_exiting_block =
      new_exiting_block->splitBasicBlock(new_exiting_block->getFirstNonPHI());
  new_exiting_block->setName("dx.struct_exit.new_exiting");
  new_not_exiting_block->setName(old_name);
  // Query for new_exiting_block's own loop to add new_not_exiting_block to.
  // It's possible that new_exiting_block is part of another inner loop
  // separate from L. If added directly to L, the inner loop(s) will not
  // contain new_not_exiting_block, making them malformed.
  Loop *inner_loop_of_exiting_block = LI->getLoopFor(new_exiting_block);
  inner_loop_of_exiting_block->addBasicBlockToLoop(new_not_exiting_block, *LI);

  // Branch to latch_exit
  new_exiting_block->getTerminator()->eraseFromParent();
  BranchInst::Create(latch_exit, new_not_exiting_block, new_exit_cond,
                     new_exiting_block);

  // If the exit block and the latch exit are the same, then we're already good.
  // just update the phi nodes in the exit block. Use the values that were
  // propagated down to the newly exiting node.
  // This can't happen if the loop is in LoopSimplifyForm, because that requires
  // 'dedicated exits', and we already know that exiting_block is not the same
  // as the latch block.
  if (latch_exit == exit_block) {
    for (const Value_Info &info : exit_values) {
      // Take the phi node in the exit block and reset incoming block and value
      // from latch_exit
      PHINode *exit_phi = info.exit_phi;
      if (exit_phi) {
        for (unsigned i = 0; i < exit_phi->getNumIncomingValues(); i++) {
          if (exit_phi->getIncomingBlock(i) == exiting_block) {
            exit_phi->setIncomingBlock(i, new_exiting_block);
            exit_phi->setIncomingValue(i,
                                       prop.Get(info.val, new_exiting_block));
          }
        }
      }
    }
  }
  // Otherwise...
  else {

    // 1. Split the latch exit, since it's going to branch to the real exit
    // block
    BasicBlock *post_exit_location =
        latch_exit->splitBasicBlock(latch_exit->getFirstNonPHI());

    {
      // If latch exit is part of an outer loop, add its split in there too.
      if (Loop *outer_loop = LI->getLoopFor(latch_exit)) {
        outer_loop->addBasicBlockToLoop(post_exit_location, *LI);
      }
      // If the original exit block is part of an outer loop, then latch exit
      // (which is the new exit block) must be part of it, since all blocks that
      // branch to within a loop must be part of that loop structure.
      else if (Loop *outer_loop = LI->getLoopFor(exit_block)) {
        outer_loop->addBasicBlockToLoop(latch_exit, *LI);
      }
    }

    // 2. Add incoming values to latch_exit's phi nodes.
    // Since now new exiting block is branching to latch exit, its phis need to
    // be updated.
    for (Instruction &inst : *latch_exit) {
      PHINode *phi = dyn_cast<PHINode>(&inst);
      if (!phi)
        break;
      // We don't care about the values for these old phis when taking the
      // newly constructed exit path.
      phi->addIncoming(GetDefaultValue(phi->getType()), new_exiting_block);
    }

    unsigned latch_exit_num_predecessors = GetNumPredecessors(latch_exit);
    PHINode *exit_cond_lcssa = nullptr;
    for (const Value_Info &info : exit_values) {

      // 3. Create lcssa phi's for all the propagated values at latch_exit.
      // Make exit values visible in the latch_exit
      PHINode *val_lcssa =
          PHINode::Create(info.val->getType(), latch_exit_num_predecessors,
                          "dx.struct_exit.val_lcssa", latch_exit->begin());

      if (info.val == exit_if_true) {
        // Record the phi for the exit condition
        exit_cond_lcssa = val_lcssa;
        exit_cond_lcssa->setName("dx.struct_exit.exit_cond_lcssa");
      }

      for (BasicBlock *pred : llvm::predecessors(latch_exit)) {
        if (pred == new_exiting_block) {
          Value *incoming = prop.Get(info.val, new_exiting_block);
          assert(incoming);
          val_lcssa->addIncoming(incoming, pred);
        } else {
          val_lcssa->addIncoming(info.false_val, pred);
        }
      }

      // 4. Update the phis in the exit_block to use the lcssa phi's we just
      // created.
      PHINode *exit_phi = info.exit_phi;
      if (exit_phi) {
        for (unsigned i = 0; i < exit_phi->getNumIncomingValues(); i++) {
          if (exit_phi->getIncomingBlock(i) == exiting_block) {
            exit_phi->setIncomingBlock(i, latch_exit);
            exit_phi->setIncomingValue(i, val_lcssa);
          }
        }
      }
    }

    // 5. Take the first half of latch_exit and branch it to the exit_block
    // based on the propagated exit condition.
    // (Currently the latch_exit unconditionally branches to
    // post_exit_location.)
    latch_exit->getTerminator()->eraseFromParent();
    BranchInst::Create(exit_block, post_exit_location, exit_cond_lcssa,
                       latch_exit);
  }

  DT->recalculate(*L->getHeader()->getParent());
  assert(L->isLCSSAForm(*DT));

  return true;
}

bool hlsl::RemoveUnstructuredLoopExits(
    llvm::Loop *L, llvm::LoopInfo *LI, llvm::DominatorTree *DT,
    std::unordered_set<llvm::BasicBlock *> *exclude_set) {

  bool changed = false;

  if (!L->isLCSSAForm(*DT))
    return false;

  // Check that the exiting blocks in the loop have BranchInst terminators (as
  // opposed to SwitchInst). At the moment we only handle BranchInst case.
  {
    llvm::SmallVector<BasicBlock *, 4> exiting_blocks;
    L->getExitingBlocks(exiting_blocks);
    for (BasicBlock *BB : exiting_blocks) {
      if (!isa<BranchInst>(BB->getTerminator()))
        return false;
    }
  }

  // Give up if loop is not rotated somehow.
  // This condition is ensured by DxilLoopUnrollPass.
  if (BasicBlock *latch = L->getLoopLatch()) {
    if (!cast<BranchInst>(latch->getTerminator())->isConditional())
      return false;
  }
  // Give up if there's not a single latch
  else {
    return false;
  }

  // The loop might not be in LoopSimplifyForm.
  // Therefore exit blocks might not be dominated by the exiting block.

  for (;;) {
    // Recompute exiting block every time, since they could change between
    // iterations
    llvm::SmallVector<BasicBlock *, 4> exiting_blocks;
    L->getExitingBlocks(exiting_blocks);

    bool local_changed = false;
    for (BasicBlock *exiting_block : exiting_blocks) {

      if (exclude_set &&
          exclude_set->count(GetExitBlockForExitingBlock(L, exiting_block)))
        continue;

      // As soon as we got a success, break and start a new iteration, since
      // exiting blocks could have changed.
      local_changed =
          RemoveUnstructuredLoopExitsIteration(exiting_block, L, LI, DT);
      if (local_changed) {
        break;
      }
    }

    changed |= local_changed;
    if (!local_changed) {
      break;
    }
  }

  return changed;
}

// This pass runs hlsl::RemoveUnstructuredLoopExits.
// It is used for testing, and can be run from `opt` like this:
//    opt -dxil-remove-unstructured-loop-exits module.ll
namespace {

class DxilRemoveUnstructuredLoopExits : public LoopPass {
public:
  static char ID;
  DxilRemoveUnstructuredLoopExits() : LoopPass(ID) {
    initializeDxilRemoveUnstructuredLoopExitsPass(
        *PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "Dxil Remove Unstructured Loop Exits";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequiredID(&LCSSAID);
    // Don't assume it's in LoopSimplifyForm. That is not guaranteed
    // by the usual callers.
  }

  bool runOnLoop(Loop *L, LPPassManager &LPM) override {
    LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    return hlsl::RemoveUnstructuredLoopExits(L, LI, DT);
  }
};
} // namespace

char DxilRemoveUnstructuredLoopExits::ID;

Pass *llvm::createDxilRemoveUnstructuredLoopExitsPass() {
  return new DxilRemoveUnstructuredLoopExits();
}

INITIALIZE_PASS_BEGIN(DxilRemoveUnstructuredLoopExits,
                      "dxil-remove-unstructured-loop-exits",
                      "DXIL Remove Unstructured Loop Exits", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(DxilRemoveUnstructuredLoopExits,
                    "dxil-remove-unstructured-loop-exits",
                    "DXIL Remove Unstructured Loop Exits", false, false)
