//===- subzero/src/IceCfgNode.cpp - Basic block (node) implementation -----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the CfgNode class, including the complexities of
/// instruction insertion and in-edge calculation.
///
//===----------------------------------------------------------------------===//

#include "IceCfgNode.h"

#include "IceAssembler.h"
#include "IceCfg.h"
#include "IceGlobalInits.h"
#include "IceInst.h"
#include "IceInstVarIter.h"
#include "IceLiveness.h"
#include "IceOperand.h"
#include "IceTargetLowering.h"

namespace Ice {

// Adds an instruction to either the Phi list or the regular instruction list.
// Validates that all Phis are added before all regular instructions.
void CfgNode::appendInst(Inst *Instr) {
  ++InstCountEstimate;

  if (BuildDefs::wasm()) {
    if (llvm::isa<InstSwitch>(Instr) || llvm::isa<InstBr>(Instr)) {
      for (auto *N : Instr->getTerminatorEdges()) {
        N->addInEdge(this);
        addOutEdge(N);
      }
    }
  }

  if (auto *Phi = llvm::dyn_cast<InstPhi>(Instr)) {
    if (!Insts.empty()) {
      Func->setError("Phi instruction added to the middle of a block");
      return;
    }
    Phis.push_back(Phi);
  } else {
    Insts.push_back(Instr);
  }
}

void CfgNode::replaceInEdge(CfgNode *Old, CfgNode *New) {
  for (SizeT i = 0; i < InEdges.size(); ++i) {
    if (InEdges[i] == Old) {
      InEdges[i] = New;
    }
  }
  for (auto &Inst : getPhis()) {
    auto &Phi = llvm::cast<InstPhi>(Inst);
    for (SizeT i = 0; i < Phi.getSrcSize(); ++i) {
      if (Phi.getLabel(i) == Old) {
        Phi.setLabel(i, New);
      }
    }
  }
}

namespace {
template <typename List> void removeDeletedAndRenumber(List *L, Cfg *Func) {
  const bool DoDelete =
      BuildDefs::minimal() || !getFlags().getKeepDeletedInsts();
  auto I = L->begin(), E = L->end(), Next = I;
  for (++Next; I != E; I = Next++) {
    if (DoDelete && I->isDeleted()) {
      L->remove(I);
    } else {
      I->renumber(Func);
    }
  }
}
} // end of anonymous namespace

void CfgNode::renumberInstructions() {
  InstNumberT FirstNumber = Func->getNextInstNumber();
  removeDeletedAndRenumber(&Phis, Func);
  removeDeletedAndRenumber(&Insts, Func);
  InstCountEstimate = Func->getNextInstNumber() - FirstNumber;
}

// When a node is created, the OutEdges are immediately known, but the InEdges
// have to be built up incrementally. After the CFG has been constructed, the
// computePredecessors() pass finalizes it by creating the InEdges list.
void CfgNode::computePredecessors() {
  for (CfgNode *Succ : OutEdges)
    Succ->InEdges.push_back(this);
}

void CfgNode::computeSuccessors() {
  OutEdges.clear();
  InEdges.clear();
  assert(!Insts.empty());
  OutEdges = Insts.rbegin()->getTerminatorEdges();
}

// Ensure each Phi instruction in the node is consistent with respect to control
// flow.  For each predecessor, there must be a phi argument with that label.
// If a phi argument's label doesn't appear in the predecessor list (which can
// happen as a result of e.g. unreachable node elimination), its value is
// modified to be zero, to maintain consistency in liveness analysis.  This
// allows us to remove some dead control flow without a major rework of the phi
// instructions.  We don't check that phi arguments with the same label have the
// same value.
void CfgNode::enforcePhiConsistency() {
  for (Inst &Instr : Phis) {
    auto *Phi = llvm::cast<InstPhi>(&Instr);
    // We do a simple O(N^2) algorithm to check for consistency. Even so, it
    // shows up as only about 0.2% of the total translation time. But if
    // necessary, we could improve the complexity by using a hash table to
    // count how many times each node is referenced in the Phi instruction, and
    // how many times each node is referenced in the incoming edge list, and
    // compare the two for equality.
    for (SizeT i = 0; i < Phi->getSrcSize(); ++i) {
      CfgNode *Label = Phi->getLabel(i);
      bool Found = false;
      for (CfgNode *InNode : getInEdges()) {
        if (InNode == Label) {
          Found = true;
          break;
        }
      }
      if (!Found) {
        // Predecessor was unreachable, so if (impossibly) the control flow
        // enters from that predecessor, the value should be zero.
        Phi->clearOperandForTarget(Label);
      }
    }
    for (CfgNode *InNode : getInEdges()) {
      bool Found = false;
      for (SizeT i = 0; i < Phi->getSrcSize(); ++i) {
        CfgNode *Label = Phi->getLabel(i);
        if (InNode == Label) {
          Found = true;
          break;
        }
      }
      if (!Found)
        llvm::report_fatal_error("Phi error: missing label for incoming edge");
    }
  }
}

// This does part 1 of Phi lowering, by creating a new dest variable for each
// Phi instruction, replacing the Phi instruction's dest with that variable,
// and adding an explicit assignment of the old dest to the new dest. For
// example,
//   a=phi(...)
// changes to
//   "a_phi=phi(...); a=a_phi".
//
// This is in preparation for part 2 which deletes the Phi instructions and
// appends assignment instructions to predecessor blocks. Note that this
// transformation preserves SSA form.
void CfgNode::placePhiLoads() {
  for (Inst &I : Phis) {
    auto *Phi = llvm::dyn_cast<InstPhi>(&I);
    Insts.insert(Insts.begin(), Phi->lower(Func));
  }
}

// This does part 2 of Phi lowering. For each Phi instruction at each out-edge,
// create a corresponding assignment instruction, and add all the assignments
// near the end of this block. They need to be added before any branch
// instruction, and also if the block ends with a compare instruction followed
// by a branch instruction that we may want to fuse, it's better to insert the
// new assignments before the compare instruction. The
// tryOptimizedCmpxchgCmpBr() method assumes this ordering of instructions.
//
// Note that this transformation takes the Phi dest variables out of SSA form,
// as there may be assignments to the dest variable in multiple blocks.
void CfgNode::placePhiStores() {
  // Find the insertion point.
  InstList::iterator InsertionPoint = Insts.end();
  // Every block must end in a terminator instruction, and therefore must have
  // at least one instruction, so it's valid to decrement InsertionPoint (but
  // assert just in case).
  assert(InsertionPoint != Insts.begin());
  --InsertionPoint;
  // Confirm that InsertionPoint is a terminator instruction. Calling
  // getTerminatorEdges() on a non-terminator instruction will cause an
  // llvm_unreachable().
  (void)InsertionPoint->getTerminatorEdges();
  // SafeInsertionPoint is always immediately before the terminator
  // instruction. If the block ends in a compare and conditional branch, it's
  // better to place the Phi store before the compare so as not to interfere
  // with compare/branch fusing. However, if the compare instruction's dest
  // operand is the same as the new assignment statement's source operand, this
  // can't be done due to data dependences, so we need to fall back to the
  // SafeInsertionPoint. To illustrate:
  //   ; <label>:95
  //   %97 = load i8* %96, align 1
  //   %98 = icmp ne i8 %97, 0
  //   br i1 %98, label %99, label %2132
  //   ; <label>:99
  //   %100 = phi i8 [ %97, %95 ], [ %110, %108 ]
  //   %101 = phi i1 [ %98, %95 ], [ %111, %108 ]
  // would be Phi-lowered as:
  //   ; <label>:95
  //   %97 = load i8* %96, align 1
  //   %100_phi = %97 ; can be at InsertionPoint
  //   %98 = icmp ne i8 %97, 0
  //   %101_phi = %98 ; must be at SafeInsertionPoint
  //   br i1 %98, label %99, label %2132
  //   ; <label>:99
  //   %100 = %100_phi
  //   %101 = %101_phi
  //
  // TODO(stichnot): It may be possible to bypass this whole SafeInsertionPoint
  // mechanism. If a source basic block ends in a conditional branch:
  //   labelSource:
  //   ...
  //   br i1 %foo, label %labelTrue, label %labelFalse
  // and a branch target has a Phi involving the branch operand:
  //   labelTrue:
  //   %bar = phi i1 [ %foo, %labelSource ], ...
  // then we actually know the constant i1 value of the Phi operand:
  //   labelTrue:
  //   %bar = phi i1 [ true, %labelSource ], ...
  // It seems that this optimization should be done by clang or opt, but we
  // could also do it here.
  InstList::iterator SafeInsertionPoint = InsertionPoint;
  // Keep track of the dest variable of a compare instruction, so that we
  // insert the new instruction at the SafeInsertionPoint if the compare's dest
  // matches the Phi-lowered assignment's source.
  Variable *CmpInstDest = nullptr;
  // If the current insertion point is at a conditional branch instruction, and
  // the previous instruction is a compare instruction, then we move the
  // insertion point before the compare instruction so as not to interfere with
  // compare/branch fusing.
  if (auto *Branch = llvm::dyn_cast<InstBr>(InsertionPoint)) {
    if (!Branch->isUnconditional()) {
      if (InsertionPoint != Insts.begin()) {
        --InsertionPoint;
        if (llvm::isa<InstIcmp>(InsertionPoint) ||
            llvm::isa<InstFcmp>(InsertionPoint)) {
          CmpInstDest = InsertionPoint->getDest();
        } else {
          ++InsertionPoint;
        }
      }
    }
  }

  // Consider every out-edge.
  for (CfgNode *Succ : OutEdges) {
    // Consider every Phi instruction at the out-edge.
    for (Inst &I : Succ->Phis) {
      auto *Phi = llvm::dyn_cast<InstPhi>(&I);
      Operand *Operand = Phi->getOperandForTarget(this);
      assert(Operand);
      Variable *Dest = I.getDest();
      assert(Dest);
      auto *NewInst = InstAssign::create(Func, Dest, Operand);
      if (CmpInstDest == Operand)
        Insts.insert(SafeInsertionPoint, NewInst);
      else
        Insts.insert(InsertionPoint, NewInst);
    }
  }
}

// Deletes the phi instructions after the loads and stores are placed.
void CfgNode::deletePhis() {
  for (Inst &I : Phis)
    I.setDeleted();
}

// Splits the edge from Pred to this node by creating a new node and hooking up
// the in and out edges appropriately. (The EdgeIndex parameter is only used to
// make the new node's name unique when there are multiple edges between the
// same pair of nodes.) The new node's instruction list is initialized to the
// empty list, with no terminator instruction. There must not be multiple edges
// from Pred to this node so all Inst::getTerminatorEdges implementations must
// not contain duplicates.
CfgNode *CfgNode::splitIncomingEdge(CfgNode *Pred, SizeT EdgeIndex) {
  CfgNode *NewNode = Func->makeNode();
  // Depth is the minimum as it works if both are the same, but if one is
  // outside the loop and the other is inside, the new node should be placed
  // outside and not be executed multiple times within the loop.
  NewNode->setLoopNestDepth(
      std::min(getLoopNestDepth(), Pred->getLoopNestDepth()));
  if (BuildDefs::dump())
    NewNode->setName("split_" + Pred->getName() + "_" + getName() + "_" +
                     std::to_string(EdgeIndex));
  // The new node is added to the end of the node list, and will later need to
  // be sorted into a reasonable topological order.
  NewNode->setNeedsPlacement(true);
  // Repoint Pred's out-edge.
  bool Found = false;
  for (CfgNode *&I : Pred->OutEdges) {
    if (I == this) {
      I = NewNode;
      NewNode->InEdges.push_back(Pred);
      Found = true;
      break;
    }
  }
  assert(Found);
  (void)Found;
  // Repoint this node's in-edge.
  Found = false;
  for (CfgNode *&I : InEdges) {
    if (I == Pred) {
      I = NewNode;
      NewNode->OutEdges.push_back(this);
      Found = true;
      break;
    }
  }
  assert(Found);
  (void)Found;
  // Repoint all suitable branch instructions' target and return.
  Found = false;
  for (Inst &I : Pred->getInsts())
    if (!I.isDeleted() && I.repointEdges(this, NewNode))
      Found = true;
  assert(Found);
  (void)Found;
  return NewNode;
}

namespace {

// Helpers for advancedPhiLowering().

class PhiDesc {
  PhiDesc() = delete;
  PhiDesc(const PhiDesc &) = delete;
  PhiDesc &operator=(const PhiDesc &) = delete;

public:
  PhiDesc(InstPhi *Phi, Variable *Dest) : Phi(Phi), Dest(Dest) {}
  PhiDesc(PhiDesc &&) = default;
  InstPhi *Phi = nullptr;
  Variable *Dest = nullptr;
  Operand *Src = nullptr;
  bool Processed = false;
  size_t NumPred = 0; // number of entries whose Src is this Dest
  int32_t Weight = 0; // preference for topological order
};
using PhiDescList = llvm::SmallVector<PhiDesc, 32>;

// Always pick NumPred=0 over NumPred>0.
constexpr int32_t WeightNoPreds = 8;
// Prefer Src as a register because the register might free up.
constexpr int32_t WeightSrcIsReg = 4;
// Prefer Dest not as a register because the register stays free longer.
constexpr int32_t WeightDestNotReg = 2;
// Prefer NumPred=1 over NumPred>1.  This is used as a tiebreaker when a
// dependency cycle must be broken so that hopefully only one temporary
// assignment has to be added to break the cycle.
constexpr int32_t WeightOnePred = 1;

bool sameVarOrReg(TargetLowering *Target, const Variable *Var1,
                  const Operand *Opnd) {
  if (Var1 == Opnd)
    return true;
  const auto *Var2 = llvm::dyn_cast<Variable>(Opnd);
  if (Var2 == nullptr)
    return false;

  // If either operand lacks a register, they cannot be the same.
  if (!Var1->hasReg())
    return false;
  if (!Var2->hasReg())
    return false;

  const auto RegNum1 = Var1->getRegNum();
  const auto RegNum2 = Var2->getRegNum();
  // Quick common-case check.
  if (RegNum1 == RegNum2)
    return true;

  assert(Target->getAliasesForRegister(RegNum1)[RegNum2] ==
         Target->getAliasesForRegister(RegNum2)[RegNum1]);
  return Target->getAliasesForRegister(RegNum1)[RegNum2];
}

// Update NumPred for all Phi assignments using Var as their Dest variable.
// Also update Weight if NumPred dropped from 2 to 1, or 1 to 0.
void updatePreds(PhiDescList &Desc, TargetLowering *Target, Variable *Var) {
  for (PhiDesc &Item : Desc) {
    if (!Item.Processed && sameVarOrReg(Target, Var, Item.Dest)) {
      --Item.NumPred;
      if (Item.NumPred == 1) {
        // If NumPred changed from 2 to 1, add in WeightOnePred.
        Item.Weight += WeightOnePred;
      } else if (Item.NumPred == 0) {
        // If NumPred changed from 1 to 0, subtract WeightOnePred and add in
        // WeightNoPreds.
        Item.Weight += (WeightNoPreds - WeightOnePred);
      }
    }
  }
}

} // end of anonymous namespace

// This the "advanced" version of Phi lowering for a basic block, in contrast
// to the simple version that lowers through assignments involving temporaries.
//
// All Phi instructions in a basic block are conceptually executed in parallel.
// However, if we lower Phis early and commit to a sequential ordering, we may
// end up creating unnecessary interferences which lead to worse register
// allocation. Delaying Phi scheduling until after register allocation can help
// unless there are no free registers for shuffling registers or stack slots
// and spilling becomes necessary.
//
// The advanced Phi lowering starts by finding a topological sort of the Phi
// instructions, where "A=B" comes before "B=C" due to the anti-dependence on
// B. Preexisting register assignments are considered in the topological sort.
// If a topological sort is not possible due to a cycle, the cycle is broken by
// introducing a non-parallel temporary. For example, a cycle arising from a
// permutation like "A=B;B=C;C=A" can become "T=A;A=B;B=C;C=T". All else being
// equal, prefer to schedule assignments with register-allocated Src operands
// earlier, in case that register becomes free afterwards, and prefer to
// schedule assignments with register-allocated Dest variables later, to keep
// that register free for longer.
//
// Once the ordering is determined, the Cfg edge is split and the assignment
// list is lowered by the target lowering layer. Since the assignment lowering
// may create new infinite-weight temporaries, a follow-on register allocation
// pass will be needed. To prepare for this, liveness (including live range
// calculation) of the split nodes needs to be calculated, and liveness of the
// original node need to be updated to "undo" the effects of the phi
// assignments.

// The specific placement of the new node within the Cfg node list is deferred
// until later, including after empty node contraction.
//
// After phi assignments are lowered across all blocks, another register
// allocation pass is run, focusing only on pre-colored and infinite-weight
// variables, similar to Om1 register allocation (except without the need to
// specially compute these variables' live ranges, since they have already been
// precisely calculated). The register allocator in this mode needs the ability
// to forcibly spill and reload registers in case none are naturally available.
void CfgNode::advancedPhiLowering() {
  if (getPhis().empty())
    return;

  PhiDescList Desc;

  for (Inst &I : Phis) {
    auto *Phi = llvm::dyn_cast<InstPhi>(&I);
    if (!Phi->isDeleted()) {
      Variable *Dest = Phi->getDest();
      Desc.emplace_back(Phi, Dest);
      // Undo the effect of the phi instruction on this node's live-in set by
      // marking the phi dest variable as live on entry.
      SizeT VarNum = Func->getLiveness()->getLiveIndex(Dest->getIndex());
      auto &LiveIn = Func->getLiveness()->getLiveIn(this);
      if (VarNum < LiveIn.size()) {
        assert(!LiveIn[VarNum]);
        LiveIn[VarNum] = true;
      }
      Phi->setDeleted();
    }
  }
  if (Desc.empty())
    return;

  TargetLowering *Target = Func->getTarget();
  SizeT InEdgeIndex = 0;
  for (CfgNode *Pred : InEdges) {
    CfgNode *Split = splitIncomingEdge(Pred, InEdgeIndex++);
    SizeT Remaining = Desc.size();

    // First pass computes Src and initializes NumPred.
    for (PhiDesc &Item : Desc) {
      Variable *Dest = Item.Dest;
      Operand *Src = Item.Phi->getOperandForTarget(Pred);
      Item.Src = Src;
      Item.Processed = false;
      Item.NumPred = 0;
      // Cherry-pick any trivial assignments, so that they don't contribute to
      // the running complexity of the topological sort.
      if (sameVarOrReg(Target, Dest, Src)) {
        Item.Processed = true;
        --Remaining;
        if (Dest != Src)
          // If Dest and Src are syntactically the same, don't bother adding
          // the assignment, because in all respects it would be redundant, and
          // if Dest/Src are on the stack, the target lowering may naively
          // decide to lower it using a temporary register.
          Split->appendInst(InstAssign::create(Func, Dest, Src));
      }
    }
    // Second pass computes NumPred by comparing every pair of Phi instructions.
    for (PhiDesc &Item : Desc) {
      if (Item.Processed)
        continue;
      const Variable *Dest = Item.Dest;
      for (PhiDesc &Item2 : Desc) {
        if (Item2.Processed)
          continue;
        // There shouldn't be two different Phis with the same Dest variable or
        // register.
        assert((&Item == &Item2) || !sameVarOrReg(Target, Dest, Item2.Dest));
        if (sameVarOrReg(Target, Dest, Item2.Src))
          ++Item.NumPred;
      }
    }

    // Another pass to compute initial Weight values.
    for (PhiDesc &Item : Desc) {
      if (Item.Processed)
        continue;
      int32_t Weight = 0;
      if (Item.NumPred == 0)
        Weight += WeightNoPreds;
      if (Item.NumPred == 1)
        Weight += WeightOnePred;
      if (auto *Var = llvm::dyn_cast<Variable>(Item.Src))
        if (Var->hasReg())
          Weight += WeightSrcIsReg;
      if (!Item.Dest->hasReg())
        Weight += WeightDestNotReg;
      Item.Weight = Weight;
    }

    // Repeatedly choose and process the best candidate in the topological sort,
    // until no candidates remain. This implementation is O(N^2) where N is the
    // number of Phi instructions, but with a small constant factor compared to
    // a likely implementation of O(N) topological sort.
    for (; Remaining; --Remaining) {
      int32_t BestWeight = -1;
      PhiDesc *BestItem = nullptr;
      // Find the best candidate.
      for (PhiDesc &Item : Desc) {
        if (Item.Processed)
          continue;
        const int32_t Weight = Item.Weight;
        if (Weight > BestWeight) {
          BestItem = &Item;
          BestWeight = Weight;
        }
      }
      assert(BestWeight >= 0);
      Variable *Dest = BestItem->Dest;
      Operand *Src = BestItem->Src;
      assert(!sameVarOrReg(Target, Dest, Src));
      // Break a cycle by introducing a temporary.
      while (BestItem->NumPred > 0) {
        bool Found = false;
        // If the target instruction "A=B" is part of a cycle, find the "X=A"
        // assignment in the cycle because it will have to be rewritten as
        // "X=tmp".
        for (PhiDesc &Item : Desc) {
          if (Item.Processed)
            continue;
          Operand *OtherSrc = Item.Src;
          if (Item.NumPred && sameVarOrReg(Target, Dest, OtherSrc)) {
            SizeT VarNum = Func->getNumVariables();
            Variable *Tmp = Func->makeVariable(OtherSrc->getType());
            if (BuildDefs::dump())
              Tmp->setName(Func, "__split_" + std::to_string(VarNum));
            Split->appendInst(InstAssign::create(Func, Tmp, OtherSrc));
            Item.Src = Tmp;
            updatePreds(Desc, Target, llvm::cast<Variable>(OtherSrc));
            Found = true;
            break;
          }
        }
        assert(Found);
        (void)Found;
      }
      // Now that a cycle (if any) has been broken, create the actual
      // assignment.
      Split->appendInst(InstAssign::create(Func, Dest, Src));
      if (auto *Var = llvm::dyn_cast<Variable>(Src))
        updatePreds(Desc, Target, Var);
      BestItem->Processed = true;
    }
    Split->appendInst(InstBr::create(Func, this));

    Split->genCode();
    Func->getVMetadata()->addNode(Split);
    // Validate to be safe.  All items should be marked as processed, and have
    // no predecessors.
    if (BuildDefs::asserts()) {
      for (PhiDesc &Item : Desc) {
        (void)Item;
        assert(Item.Processed);
        assert(Item.NumPred == 0);
      }
    }
  }
}

// Does address mode optimization. Pass each instruction to the TargetLowering
// object. If it returns a new instruction (representing the optimized address
// mode), then insert the new instruction and delete the old.
void CfgNode::doAddressOpt() {
  TargetLowering *Target = Func->getTarget();
  LoweringContext &Context = Target->getContext();
  Context.init(this);
  while (!Context.atEnd()) {
    Target->doAddressOpt();
  }
}

// Drives the target lowering. Passes the current instruction and the next
// non-deleted instruction for target lowering.
void CfgNode::genCode() {
  TargetLowering *Target = Func->getTarget();
  LoweringContext &Context = Target->getContext();
  // Lower the regular instructions.
  Context.init(this);
  Target->initNodeForLowering(this);
  while (!Context.atEnd()) {
    InstList::iterator Orig = Context.getCur();
    if (llvm::isa<InstRet>(*Orig))
      setHasReturn();
    Target->lower();
    // Ensure target lowering actually moved the cursor.
    assert(Context.getCur() != Orig);
  }
  Context.availabilityReset();
  // Do preliminary lowering of the Phi instructions.
  Target->prelowerPhis();
}

void CfgNode::livenessLightweight() {
  SizeT NumVars = Func->getNumVariables();
  LivenessBV Live(NumVars);
  // Process regular instructions in reverse order.
  for (Inst &I : reverse_range(Insts)) {
    if (I.isDeleted())
      continue;
    I.livenessLightweight(Func, Live);
  }
  for (Inst &I : Phis) {
    if (I.isDeleted())
      continue;
    I.livenessLightweight(Func, Live);
  }
}

// Performs liveness analysis on the block. Returns true if the incoming
// liveness changed from before, false if it stayed the same. (If it changes,
// the node's predecessors need to be processed again.)
bool CfgNode::liveness(Liveness *Liveness) {
  const SizeT NumVars = Liveness->getNumVarsInNode(this);
  const SizeT NumGlobalVars = Liveness->getNumGlobalVars();
  LivenessBV &Live = Liveness->getScratchBV();
  Live.clear();

  LiveBeginEndMap *LiveBegin = nullptr;
  LiveBeginEndMap *LiveEnd = nullptr;
  // Mark the beginning and ending of each variable's live range with the
  // sentinel instruction number 0.
  if (Liveness->getMode() == Liveness_Intervals) {
    LiveBegin = Liveness->getLiveBegin(this);
    LiveEnd = Liveness->getLiveEnd(this);
    LiveBegin->clear();
    LiveEnd->clear();
    // Guess that the number of live ranges beginning is roughly the number of
    // instructions, and same for live ranges ending.
    LiveBegin->reserve(getInstCountEstimate());
    LiveEnd->reserve(getInstCountEstimate());
  }

  // Initialize Live to be the union of all successors' LiveIn.
  for (CfgNode *Succ : OutEdges) {
    const LivenessBV &LiveIn = Liveness->getLiveIn(Succ);
    assert(LiveIn.empty() || LiveIn.size() == NumGlobalVars);
    Live |= LiveIn;
    // Mark corresponding argument of phis in successor as live.
    for (Inst &I : Succ->Phis) {
      if (I.isDeleted())
        continue;
      auto *Phi = llvm::cast<InstPhi>(&I);
      Phi->livenessPhiOperand(Live, this, Liveness);
    }
  }
  assert(Live.empty() || Live.size() == NumGlobalVars);
  Liveness->getLiveOut(this) = Live;

  // Expand Live so it can hold locals in addition to globals.
  Live.resize(NumVars);
  // Process regular instructions in reverse order.
  for (Inst &I : reverse_range(Insts)) {
    if (I.isDeleted())
      continue;
    I.liveness(I.getNumber(), Live, Liveness, LiveBegin, LiveEnd);
  }
  // Process phis in forward order so that we can override the instruction
  // number to be that of the earliest phi instruction in the block.
  SizeT NumNonDeadPhis = 0;
  InstNumberT FirstPhiNumber = Inst::NumberSentinel;
  for (Inst &I : Phis) {
    if (I.isDeleted())
      continue;
    if (FirstPhiNumber == Inst::NumberSentinel)
      FirstPhiNumber = I.getNumber();
    if (I.liveness(FirstPhiNumber, Live, Liveness, LiveBegin, LiveEnd))
      ++NumNonDeadPhis;
  }

  // When using the sparse representation, after traversing the instructions in
  // the block, the Live bitvector should only contain set bits for global
  // variables upon block entry.  We validate this by testing the upper bits of
  // the Live bitvector.
  if (Live.find_next(NumGlobalVars) != -1) {
    if (BuildDefs::dump()) {
      // This is a fatal liveness consistency error. Print some diagnostics and
      // abort.
      Ostream &Str = Func->getContext()->getStrDump();
      Func->resetCurrentNode();
      Str << "Invalid Live =";
      for (SizeT i = NumGlobalVars; i < Live.size(); ++i) {
        if (Live.test(i)) {
          Str << " ";
          Liveness->getVariable(i, this)->dump(Func);
        }
      }
      Str << "\n";
    }
    llvm::report_fatal_error("Fatal inconsistency in liveness analysis");
  }
  // Now truncate Live to prevent LiveIn from growing.
  Live.resize(NumGlobalVars);

  bool Changed = false;
  LivenessBV &LiveIn = Liveness->getLiveIn(this);
  assert(LiveIn.empty() || LiveIn.size() == NumGlobalVars);
  // Add in current LiveIn
  Live |= LiveIn;
  // Check result, set LiveIn=Live
  SizeT &PrevNumNonDeadPhis = Liveness->getNumNonDeadPhis(this);
  bool LiveInChanged = (Live != LiveIn);
  Changed = (NumNonDeadPhis != PrevNumNonDeadPhis || LiveInChanged);
  if (LiveInChanged)
    LiveIn = Live;
  PrevNumNonDeadPhis = NumNonDeadPhis;
  return Changed;
}

// Validate the integrity of the live ranges in this block.  If there are any
// errors, it prints details and returns false.  On success, it returns true.
bool CfgNode::livenessValidateIntervals(Liveness *Liveness) const {
  if (!BuildDefs::asserts())
    return true;

  // Verify there are no duplicates.
  auto ComparePair = [](const LiveBeginEndMapEntry &A,
                        const LiveBeginEndMapEntry &B) {
    return A.first == B.first;
  };
  LiveBeginEndMap &MapBegin = *Liveness->getLiveBegin(this);
  LiveBeginEndMap &MapEnd = *Liveness->getLiveEnd(this);
  if (std::adjacent_find(MapBegin.begin(), MapBegin.end(), ComparePair) ==
          MapBegin.end() &&
      std::adjacent_find(MapEnd.begin(), MapEnd.end(), ComparePair) ==
          MapEnd.end())
    return true;

  // There is definitely a liveness error.  All paths from here return false.
  if (!BuildDefs::dump())
    return false;

  // Print all the errors.
  if (BuildDefs::dump()) {
    GlobalContext *Ctx = Func->getContext();
    OstreamLocker L(Ctx);
    Ostream &Str = Ctx->getStrDump();
    if (Func->isVerbose()) {
      Str << "Live range errors in the following block:\n";
      dump(Func);
    }
    for (auto Start = MapBegin.begin();
         (Start = std::adjacent_find(Start, MapBegin.end(), ComparePair)) !=
         MapBegin.end();
         ++Start) {
      auto Next = Start + 1;
      Str << "Duplicate LR begin, block " << getName() << ", instructions "
          << Start->second << " & " << Next->second << ", variable "
          << Liveness->getVariable(Start->first, this)->getName() << "\n";
    }
    for (auto Start = MapEnd.begin();
         (Start = std::adjacent_find(Start, MapEnd.end(), ComparePair)) !=
         MapEnd.end();
         ++Start) {
      auto Next = Start + 1;
      Str << "Duplicate LR end,   block " << getName() << ", instructions "
          << Start->second << " & " << Next->second << ", variable "
          << Liveness->getVariable(Start->first, this)->getName() << "\n";
    }
  }

  return false;
}

// Once basic liveness is complete, compute actual live ranges. It is assumed
// that within a single basic block, a live range begins at most once and ends
// at most once. This is certainly true for pure SSA form. It is also true once
// phis are lowered, since each assignment to the phi-based temporary is in a
// different basic block, and there is a single read that ends the live in the
// basic block that contained the actual phi instruction.
void CfgNode::livenessAddIntervals(Liveness *Liveness, InstNumberT FirstInstNum,
                                   InstNumberT LastInstNum) {
  TimerMarker T1(TimerStack::TT_liveRange, Func);

  const SizeT NumVars = Liveness->getNumVarsInNode(this);
  const LivenessBV &LiveIn = Liveness->getLiveIn(this);
  const LivenessBV &LiveOut = Liveness->getLiveOut(this);
  LiveBeginEndMap &MapBegin = *Liveness->getLiveBegin(this);
  LiveBeginEndMap &MapEnd = *Liveness->getLiveEnd(this);
  std::sort(MapBegin.begin(), MapBegin.end());
  std::sort(MapEnd.begin(), MapEnd.end());

  if (!livenessValidateIntervals(Liveness)) {
    llvm::report_fatal_error("livenessAddIntervals: Liveness error");
    return;
  }

  LivenessBV &LiveInAndOut = Liveness->getScratchBV();
  LiveInAndOut = LiveIn;
  LiveInAndOut &= LiveOut;

  // Iterate in parallel across the sorted MapBegin[] and MapEnd[].
  auto IBB = MapBegin.begin(), IEB = MapEnd.begin();
  auto IBE = MapBegin.end(), IEE = MapEnd.end();
  while (IBB != IBE || IEB != IEE) {
    SizeT i1 = IBB == IBE ? NumVars : IBB->first;
    SizeT i2 = IEB == IEE ? NumVars : IEB->first;
    SizeT i = std::min(i1, i2);
    // i1 is the Variable number of the next MapBegin entry, and i2 is the
    // Variable number of the next MapEnd entry. If i1==i2, then the Variable's
    // live range begins and ends in this block. If i1<i2, then i1's live range
    // begins at instruction IBB->second and extends through the end of the
    // block. If i1>i2, then i2's live range begins at the first instruction of
    // the block and ends at IEB->second. In any case, we choose the lesser of
    // i1 and i2 and proceed accordingly.
    InstNumberT LB = i == i1 ? IBB->second : FirstInstNum;
    InstNumberT LE = i == i2 ? IEB->second : LastInstNum + 1;

    Variable *Var = Liveness->getVariable(i, this);
    if (LB > LE) {
      Var->addLiveRange(FirstInstNum, LE, this);
      Var->addLiveRange(LB, LastInstNum + 1, this);
      // Assert that Var is a global variable by checking that its liveness
      // index is less than the number of globals. This ensures that the
      // LiveInAndOut[] access is valid.
      assert(i < Liveness->getNumGlobalVars());
      LiveInAndOut[i] = false;
    } else {
      Var->addLiveRange(LB, LE, this);
    }
    if (i == i1)
      ++IBB;
    if (i == i2)
      ++IEB;
  }
  // Process the variables that are live across the entire block.
  for (int i = LiveInAndOut.find_first(); i != -1;
       i = LiveInAndOut.find_next(i)) {
    Variable *Var = Liveness->getVariable(i, this);
    if (Liveness->getRangeMask(Var->getIndex()))
      Var->addLiveRange(FirstInstNum, LastInstNum + 1, this);
  }
}

// If this node contains only deleted instructions, and ends in an
// unconditional branch, contract the node by repointing all its in-edges to
// its successor.
void CfgNode::contractIfEmpty() {
  if (InEdges.empty())
    return;
  Inst *Branch = nullptr;
  for (Inst &I : Insts) {
    if (I.isDeleted())
      continue;
    if (I.isUnconditionalBranch())
      Branch = &I;
    else if (!I.isRedundantAssign())
      return;
  }
  // Make sure there is actually a successor to repoint in-edges to.
  if (OutEdges.empty())
    return;
  assert(hasSingleOutEdge());
  // Don't try to delete a self-loop.
  if (OutEdges[0] == this)
    return;
  // Make sure the node actually contains (ends with) an unconditional branch.
  if (Branch == nullptr)
    return;

  Branch->setDeleted();
  CfgNode *Successor = OutEdges.front();
  // Repoint all this node's in-edges to this node's successor, unless this
  // node's successor is actually itself (in which case the statement
  // "OutEdges.front()->InEdges.push_back(Pred)" could invalidate the iterator
  // over this->InEdges).
  if (Successor != this) {
    for (CfgNode *Pred : InEdges) {
      for (CfgNode *&I : Pred->OutEdges) {
        if (I == this) {
          I = Successor;
          Successor->InEdges.push_back(Pred);
        }
      }
      for (Inst &I : Pred->getInsts()) {
        if (!I.isDeleted())
          I.repointEdges(this, Successor);
      }
    }

    // Remove the in-edge to the successor to allow node reordering to make
    // better decisions. For example it's more helpful to place a node after a
    // reachable predecessor than an unreachable one (like the one we just
    // contracted).
    Successor->InEdges.erase(
        std::find(Successor->InEdges.begin(), Successor->InEdges.end(), this));
  }
  InEdges.clear();
}

void CfgNode::doBranchOpt(const CfgNode *NextNode) {
  TargetLowering *Target = Func->getTarget();
  // Find the first opportunity for branch optimization (which will be the last
  // instruction in the block) and stop. This is sufficient unless there is
  // some target lowering where we have the possibility of multiple
  // optimizations per block. Take care with switch lowering as there are
  // multiple unconditional branches and only the last can be deleted.
  for (Inst &I : reverse_range(Insts)) {
    if (!I.isDeleted()) {
      Target->doBranchOpt(&I, NextNode);
      return;
    }
  }
}

// ======================== Dump routines ======================== //

namespace {

// Helper functions for emit().

void emitRegisterUsage(Ostream &Str, const Cfg *Func, const CfgNode *Node,
                       bool IsLiveIn, CfgVector<SizeT> &LiveRegCount) {
  if (!BuildDefs::dump())
    return;
  Liveness *Liveness = Func->getLiveness();
  const LivenessBV *Live;
  const auto StackReg = Func->getTarget()->getStackReg();
  const auto FrameOrStackReg = Func->getTarget()->getFrameOrStackReg();
  if (IsLiveIn) {
    Live = &Liveness->getLiveIn(Node);
    Str << "\t\t\t\t/* LiveIn=";
  } else {
    Live = &Liveness->getLiveOut(Node);
    Str << "\t\t\t\t/* LiveOut=";
  }
  if (!Live->empty()) {
    CfgVector<Variable *> LiveRegs;
    for (SizeT i = 0; i < Live->size(); ++i) {
      if (!(*Live)[i])
        continue;
      Variable *Var = Liveness->getVariable(i, Node);
      if (!Var->hasReg())
        continue;
      const auto RegNum = Var->getRegNum();
      if (RegNum == StackReg || RegNum == FrameOrStackReg)
        continue;
      if (IsLiveIn)
        ++LiveRegCount[RegNum];
      LiveRegs.push_back(Var);
    }
    // Sort the variables by regnum so they are always printed in a familiar
    // order.
    std::sort(LiveRegs.begin(), LiveRegs.end(),
              [](const Variable *V1, const Variable *V2) {
                return unsigned(V1->getRegNum()) < unsigned(V2->getRegNum());
              });
    bool First = true;
    for (Variable *Var : LiveRegs) {
      if (!First)
        Str << ",";
      First = false;
      Var->emit(Func);
    }
  }
  Str << " */\n";
}

/// Returns true if some text was emitted - in which case the caller definitely
/// needs to emit a newline character.
bool emitLiveRangesEnded(Ostream &Str, const Cfg *Func, const Inst *Instr,
                         CfgVector<SizeT> &LiveRegCount) {
  bool Printed = false;
  if (!BuildDefs::dump())
    return Printed;
  Variable *Dest = Instr->getDest();
  // Normally we increment the live count for the dest register. But we
  // shouldn't if the instruction's IsDestRedefined flag is set, because this
  // means that the target lowering created this instruction as a non-SSA
  // assignment; i.e., a different, previous instruction started the dest
  // variable's live range.
  if (!Instr->isDestRedefined() && Dest && Dest->hasReg())
    ++LiveRegCount[Dest->getRegNum()];
  FOREACH_VAR_IN_INST(Var, *Instr) {
    bool ShouldReport = Instr->isLastUse(Var);
    if (ShouldReport && Var->hasReg()) {
      // Don't report end of live range until the live count reaches 0.
      SizeT NewCount = --LiveRegCount[Var->getRegNum()];
      if (NewCount)
        ShouldReport = false;
    }
    if (ShouldReport) {
      if (Printed)
        Str << ",";
      else
        Str << " \t/* END=";
      Var->emit(Func);
      Printed = true;
    }
  }
  if (Printed)
    Str << " */";
  return Printed;
}

void updateStats(Cfg *Func, const Inst *I) {
  if (!BuildDefs::dump())
    return;
  // Update emitted instruction count, plus fill/spill count for Variable
  // operands without a physical register.
  if (uint32_t Count = I->getEmitInstCount()) {
    Func->getContext()->statsUpdateEmitted(Count);
    if (Variable *Dest = I->getDest()) {
      if (!Dest->hasReg())
        Func->getContext()->statsUpdateFills();
    }
    for (SizeT S = 0; S < I->getSrcSize(); ++S) {
      if (auto *Src = llvm::dyn_cast<Variable>(I->getSrc(S))) {
        if (!Src->hasReg())
          Func->getContext()->statsUpdateSpills();
      }
    }
  }
}

} // end of anonymous namespace

void CfgNode::emit(Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Func->setCurrentNode(this);
  Ostream &Str = Func->getContext()->getStrEmit();
  Liveness *Liveness = Func->getLiveness();
  const bool DecorateAsm = Liveness && getFlags().getDecorateAsm();
  Str << getAsmName() << ":\n";
  // LiveRegCount keeps track of the number of currently live variables that
  // each register is assigned to. Normally that would be only 0 or 1, but the
  // register allocator's AllowOverlap inference allows it to be greater than 1
  // for short periods.
  CfgVector<SizeT> LiveRegCount(Func->getTarget()->getNumRegisters());
  if (DecorateAsm) {
    constexpr bool IsLiveIn = true;
    emitRegisterUsage(Str, Func, this, IsLiveIn, LiveRegCount);
    if (getInEdges().size()) {
      Str << "\t\t\t\t/* preds=";
      bool First = true;
      for (CfgNode *I : getInEdges()) {
        if (!First)
          Str << ",";
        First = false;
        Str << "$" << I->getName();
      }
      Str << " */\n";
    }
    if (getLoopNestDepth()) {
      Str << "\t\t\t\t/* loop depth=" << getLoopNestDepth() << " */\n";
    }
  }

  for (const Inst &I : Phis) {
    if (I.isDeleted())
      continue;
    // Emitting a Phi instruction should cause an error.
    I.emit(Func);
  }
  for (const Inst &I : Insts) {
    if (I.isDeleted())
      continue;
    if (I.isRedundantAssign()) {
      // Usually, redundant assignments end the live range of the src variable
      // and begin the live range of the dest variable, with no net effect on
      // the liveness of their register. However, if the register allocator
      // infers the AllowOverlap condition, then this may be a redundant
      // assignment that does not end the src variable's live range, in which
      // case the active variable count for that register needs to be bumped.
      // That normally would have happened as part of emitLiveRangesEnded(),
      // but that isn't called for redundant assignments.
      Variable *Dest = I.getDest();
      if (DecorateAsm && Dest->hasReg()) {
        ++LiveRegCount[Dest->getRegNum()];
        if (I.isLastUse(I.getSrc(0)))
          --LiveRegCount[llvm::cast<Variable>(I.getSrc(0))->getRegNum()];
      }
      continue;
    }
    I.emit(Func);
    bool Printed = false;
    if (DecorateAsm)
      Printed = emitLiveRangesEnded(Str, Func, &I, LiveRegCount);
    if (Printed || llvm::isa<InstTarget>(&I))
      Str << "\n";
    updateStats(Func, &I);
  }
  if (DecorateAsm) {
    constexpr bool IsLiveIn = false;
    emitRegisterUsage(Str, Func, this, IsLiveIn, LiveRegCount);
  }
}

void CfgNode::emitIAS(Cfg *Func) const {
  Func->setCurrentNode(this);
  Assembler *Asm = Func->getAssembler<>();
  Asm->bindCfgNodeLabel(this);
  for (const Inst &I : Phis) {
    if (I.isDeleted())
      continue;
    // Emitting a Phi instruction should cause an error.
    I.emitIAS(Func);
  }

  for (const Inst &I : Insts) {
    if (!I.isDeleted() && !I.isRedundantAssign()) {
      I.emitIAS(Func);
      updateStats(Func, &I);
    }
  }
}

void CfgNode::dump(Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Func->setCurrentNode(this);
  Ostream &Str = Func->getContext()->getStrDump();
  Liveness *Liveness = Func->getLiveness();
  if (Func->isVerbose(IceV_Instructions) || Func->isVerbose(IceV_Loop))
    Str << getName() << ":\n";
  // Dump the loop nest depth
  if (Func->isVerbose(IceV_Loop))
    Str << "    // LoopNestDepth = " << getLoopNestDepth() << "\n";
  // Dump list of predecessor nodes.
  if (Func->isVerbose(IceV_Preds) && !InEdges.empty()) {
    Str << "    // preds = ";
    bool First = true;
    for (CfgNode *I : InEdges) {
      if (!First)
        Str << ", ";
      First = false;
      Str << "%" << I->getName();
    }
    Str << "\n";
  }
  // Dump the live-in variables.
  if (Func->isVerbose(IceV_Liveness)) {
    if (Liveness != nullptr && !Liveness->getLiveIn(this).empty()) {
      const LivenessBV &LiveIn = Liveness->getLiveIn(this);
      Str << "    // LiveIn:";
      for (SizeT i = 0; i < LiveIn.size(); ++i) {
        if (LiveIn[i]) {
          Variable *Var = Liveness->getVariable(i, this);
          Str << " %" << Var->getName();
          if (Func->isVerbose(IceV_RegOrigins) && Var->hasReg()) {
            Str << ":"
                << Func->getTarget()->getRegName(Var->getRegNum(),
                                                 Var->getType());
          }
        }
      }
      Str << "\n";
    }
  }
  // Dump each instruction.
  if (Func->isVerbose(IceV_Instructions)) {
    for (const Inst &I : Phis)
      I.dumpDecorated(Func);
    for (const Inst &I : Insts)
      I.dumpDecorated(Func);
  }
  // Dump the live-out variables.
  if (Func->isVerbose(IceV_Liveness)) {
    if (Liveness != nullptr && !Liveness->getLiveOut(this).empty()) {
      const LivenessBV &LiveOut = Liveness->getLiveOut(this);
      Str << "    // LiveOut:";
      for (SizeT i = 0; i < LiveOut.size(); ++i) {
        if (LiveOut[i]) {
          Variable *Var = Liveness->getVariable(i, this);
          Str << " %" << Var->getName();
          if (Func->isVerbose(IceV_RegOrigins) && Var->hasReg()) {
            Str << ":"
                << Func->getTarget()->getRegName(Var->getRegNum(),
                                                 Var->getType());
          }
        }
      }
      Str << "\n";
    }
  }
  // Dump list of successor nodes.
  if (Func->isVerbose(IceV_Succs)) {
    Str << "    // succs = ";
    bool First = true;
    for (CfgNode *I : OutEdges) {
      if (!First)
        Str << ", ";
      First = false;
      Str << "%" << I->getName();
    }
    Str << "\n";
  }
}

void CfgNode::removeInEdge(CfgNode *In) {
  InEdges.erase(std::find(InEdges.begin(), InEdges.end(), In));
}

CfgNode *CfgNode::shortCircuit() {
  auto *Func = getCfg();
  auto *Last = &getInsts().back();
  Variable *Condition = nullptr;
  InstBr *Br = nullptr;
  if ((Br = llvm::dyn_cast<InstBr>(Last))) {
    if (!Br->isUnconditional()) {
      Condition = llvm::dyn_cast<Variable>(Br->getCondition());
    }
  }
  if (Condition == nullptr)
    return nullptr;

  auto *JumpOnTrue = Br->getTargetTrue();
  auto *JumpOnFalse = Br->getTargetFalse();

  bool FoundOr = false;
  bool FoundAnd = false;

  InstArithmetic *TopLevelBoolOp = nullptr;

  for (auto &Inst : reverse_range(getInsts())) {
    if (Inst.isDeleted())
      continue;
    if (Inst.getDest() == Condition) {
      if (auto *Arith = llvm::dyn_cast<InstArithmetic>(&Inst)) {

        FoundOr = (Arith->getOp() == InstArithmetic::OpKind::Or);
        FoundAnd = (Arith->getOp() == InstArithmetic::OpKind::And);

        if (FoundOr || FoundAnd) {
          TopLevelBoolOp = Arith;
          break;
        }
      }
    }
  }

  if (!TopLevelBoolOp)
    return nullptr;

  auto IsOperand = [](Inst *Instr, Operand *Opr) -> bool {
    for (SizeT i = 0; i < Instr->getSrcSize(); ++i) {
      if (Instr->getSrc(i) == Opr)
        return true;
    }
    return false;
  };
  Inst *FirstOperandDef = nullptr;
  for (auto &Inst : getInsts()) {
    if (IsOperand(TopLevelBoolOp, Inst.getDest())) {
      FirstOperandDef = &Inst;
      break;
    }
  }

  if (FirstOperandDef == nullptr) {
    return nullptr;
  }

  // Check for side effects
  auto It = Ice::instToIterator(FirstOperandDef);
  while (It != getInsts().end()) {
    if (It->isDeleted()) {
      ++It;
      continue;
    }
    if (llvm::isa<InstBr>(It) || llvm::isa<InstRet>(It)) {
      break;
    }
    auto *Dest = It->getDest();
    if (It->getDest() == nullptr || It->hasSideEffects() ||
        !Func->getVMetadata()->isSingleBlock(Dest)) {
      // Relying on short cicuit eval here.
      // getVMetadata()->isSingleBlock(Dest)
      // will segfault if It->getDest() == nullptr
      return nullptr;
    }
    It++;
  }

  auto *NewNode = Func->makeNode();
  NewNode->setLoopNestDepth(getLoopNestDepth());
  It = Ice::instToIterator(FirstOperandDef);
  It++; // Have to split after the def

  NewNode->getInsts().splice(NewNode->getInsts().begin(), getInsts(), It,
                             getInsts().end());

  if (BuildDefs::dump()) {
    NewNode->setName(getName().append("_2"));
    setName(getName().append("_1"));
  }

  // Point edges properly
  NewNode->addInEdge(this);
  for (auto *Out : getOutEdges()) {
    NewNode->addOutEdge(Out);
    Out->addInEdge(NewNode);
  }
  removeAllOutEdges();
  addOutEdge(NewNode);

  // Manage Phi instructions of successors
  for (auto *Succ : NewNode->getOutEdges()) {
    for (auto &Inst : Succ->getPhis()) {
      auto *Phi = llvm::cast<InstPhi>(&Inst);
      for (SizeT i = 0; i < Phi->getSrcSize(); ++i) {
        if (Phi->getLabel(i) == this) {
          Phi->addArgument(Phi->getSrc(i), NewNode);
        }
      }
    }
  }

  // Create new Br instruction
  InstBr *NewInst = nullptr;
  if (FoundOr) {
    addOutEdge(JumpOnTrue);
    JumpOnFalse->removeInEdge(this);
    NewInst =
        InstBr::create(Func, FirstOperandDef->getDest(), JumpOnTrue, NewNode);
  } else if (FoundAnd) {
    addOutEdge(JumpOnFalse);
    JumpOnTrue->removeInEdge(this);
    NewInst =
        InstBr::create(Func, FirstOperandDef->getDest(), NewNode, JumpOnFalse);
  } else {
    return nullptr;
  }

  assert(NewInst != nullptr);
  appendInst(NewInst);

  Operand *UnusedOperand = nullptr;
  assert(TopLevelBoolOp->getSrcSize() == 2);
  if (TopLevelBoolOp->getSrc(0) == FirstOperandDef->getDest())
    UnusedOperand = TopLevelBoolOp->getSrc(1);
  else if (TopLevelBoolOp->getSrc(1) == FirstOperandDef->getDest())
    UnusedOperand = TopLevelBoolOp->getSrc(0);
  assert(UnusedOperand);

  Br->replaceSource(0, UnusedOperand); // Index 0 has the condition of the Br

  TopLevelBoolOp->setDeleted();
  return NewNode;
}

} // end of namespace Ice
