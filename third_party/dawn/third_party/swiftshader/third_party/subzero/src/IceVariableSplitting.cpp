//===- subzero/src/IceVariableSplitting.cpp - Local variable splitting ----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Aggressive block-local variable splitting to improve linear-scan
/// register allocation.
///
//===----------------------------------------------------------------------===//

#include "IceVariableSplitting.h"

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceClFlags.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceTargetLowering.h"

namespace Ice {

namespace {

/// A Variable is "allocable" if it is a register allocation candidate but
/// doesn't already have a register.
bool isAllocable(const Variable *Var) {
  if (Var == nullptr)
    return false;
  return !Var->hasReg() && Var->mayHaveReg();
}

/// A Variable is "inf" if it already has a register or is infinite-weight.
bool isInf(const Variable *Var) {
  if (Var == nullptr)
    return false;
  return Var->hasReg() || Var->mustHaveReg();
}

/// VariableMap is a simple helper class that keeps track of the latest split
/// version of the original Variables, as well as the instruction containing the
/// last use of the Variable within the current block.  For each entry, the
/// Variable is tagged with the CfgNode that it is valid in, so that we don't
/// need to clear the entire Map[] vector for each block.
class VariableMap {
private:
  VariableMap() = delete;
  VariableMap(const VariableMap &) = delete;
  VariableMap &operator=(const VariableMap &) = delete;

  struct VarInfo {
    /// MappedVar is the latest mapped/split version of the Variable.
    Variable *MappedVar = nullptr;
    /// MappedVarNode is the block in which MappedVar is valid.
    const CfgNode *MappedVarNode = nullptr;
    /// LastUseInst is the last instruction in the block that uses the Variable
    /// as a source operand.
    const Inst *LastUseInst = nullptr;
    /// LastUseNode is the block in which LastUseInst is valid.
    const CfgNode *LastUseNode = nullptr;
    VarInfo() = default;

  private:
    VarInfo(const VarInfo &) = delete;
    VarInfo &operator=(const VarInfo &) = delete;
  };

public:
  explicit VariableMap(Cfg *Func)
      : Func(Func), NumVars(Func->getNumVariables()), Map(NumVars) {}
  /// Reset the mappings at the start of a block.
  void reset(const CfgNode *CurNode) {
    Node = CurNode;
    // Do a prepass through all the instructions, marking which instruction is
    // the last use of each Variable within the block.
    for (const Inst &Instr : Node->getInsts()) {
      if (Instr.isDeleted())
        continue;
      for (SizeT i = 0; i < Instr.getSrcSize(); ++i) {
        if (auto *SrcVar = llvm::dyn_cast<Variable>(Instr.getSrc(i))) {
          const SizeT VarNum = getVarNum(SrcVar);
          Map[VarNum].LastUseInst = &Instr;
          Map[VarNum].LastUseNode = Node;
        }
      }
    }
  }
  /// Get Var's current mapping (or Var itself if it has no mapping yet).
  Variable *get(Variable *Var) const {
    const SizeT VarNum = getVarNum(Var);
    Variable *MappedVar = Map[VarNum].MappedVar;
    if (MappedVar == nullptr)
      return Var;
    if (Map[VarNum].MappedVarNode != Node)
      return Var;
    return MappedVar;
  }
  /// Create a new linked Variable in the LinkedTo chain, and set it as Var's
  /// latest mapping.
  Variable *makeLinked(Variable *Var) {
    Variable *NewVar = Func->makeVariable(Var->getType());
    NewVar->setRegClass(Var->getRegClass());
    NewVar->setLinkedTo(get(Var));
    const SizeT VarNum = getVarNum(Var);
    Map[VarNum].MappedVar = NewVar;
    Map[VarNum].MappedVarNode = Node;
    return NewVar;
  }
  /// Given Var that is LinkedTo some other variable, re-splice it into the
  /// LinkedTo chain so that the chain is ordered by Variable::getIndex().
  void spliceBlockLocalLinkedToChain(Variable *Var) {
    Variable *LinkedTo = Var->getLinkedTo();
    assert(LinkedTo != nullptr);
    assert(Var->getIndex() > LinkedTo->getIndex());
    const SizeT VarNum = getVarNum(LinkedTo);
    Variable *Link = Map[VarNum].MappedVar;
    if (Link == nullptr || Map[VarNum].MappedVarNode != Node)
      return;
    Variable *LinkParent = Link->getLinkedTo();
    while (LinkParent != nullptr && LinkParent->getIndex() >= Var->getIndex()) {
      Link = LinkParent;
      LinkParent = Link->getLinkedTo();
    }
    Var->setLinkedTo(LinkParent);
    Link->setLinkedTo(Var);
  }
  /// Return whether the given Variable has any uses as a source operand within
  /// the current block.  If it has no source operand uses, but is assigned as a
  /// dest variable in some instruction in the block, then we needn't bother
  /// splitting it.
  bool isDestUsedInBlock(const Variable *Dest) const {
    return Map[getVarNum(Dest)].LastUseNode == Node;
  }
  /// Return whether the given instruction is the last use of the given Variable
  /// within the current block.  If it is, then we needn't bother splitting the
  /// Variable at this instruction.
  bool isInstLastUseOfVar(const Variable *Var, const Inst *Instr) {
    return Map[getVarNum(Var)].LastUseInst == Instr;
  }

private:
  Cfg *const Func;
  // NumVars is for the size of the Map array.  It can be const because any new
  // Variables created during the splitting pass don't need to be mapped.
  const SizeT NumVars;
  CfgVector<VarInfo> Map;
  const CfgNode *Node = nullptr;
  /// Get Var's VarNum, and do some validation.
  SizeT getVarNum(const Variable *Var) const {
    const SizeT VarNum = Var->getIndex();
    assert(VarNum < NumVars);
    return VarNum;
  }
};

/// LocalVariableSplitter tracks the necessary splitting state across
/// instructions.
class LocalVariableSplitter {
  LocalVariableSplitter() = delete;
  LocalVariableSplitter(const LocalVariableSplitter &) = delete;
  LocalVariableSplitter &operator=(const LocalVariableSplitter &) = delete;

public:
  explicit LocalVariableSplitter(Cfg *Func)
      : Target(Func->getTarget()), VarMap(Func) {}
  /// setNode() is called before processing the instructions of a block.
  void setNode(CfgNode *CurNode) {
    Node = CurNode;
    VarMap.reset(Node);
    LinkedToFixups.clear();
  }
  /// finalizeNode() is called after all instructions in the block are
  /// processed.
  void finalizeNode() {
    // Splice in any preexisting LinkedTo links into the single chain.  These
    // are the ones that were recorded during setInst().
    for (Variable *Var : LinkedToFixups) {
      VarMap.spliceBlockLocalLinkedToChain(Var);
    }
  }
  /// setInst() is called before processing the next instruction.  The iterators
  /// are the insertion points for a new instructions, depending on whether the
  /// new instruction should be inserted before or after the current
  /// instruction.
  void setInst(Inst *CurInst, InstList::iterator Cur, InstList::iterator Next) {
    Instr = CurInst;
    Dest = Instr->getDest();
    IterCur = Cur;
    IterNext = Next;
    ShouldSkipRemainingInstructions = false;
    // Note any preexisting LinkedTo relationships that were created during
    // target lowering.  Record them in LinkedToFixups which is then processed
    // in finalizeNode().
    if (Dest != nullptr && Dest->getLinkedTo() != nullptr) {
      LinkedToFixups.emplace_back(Dest);
    }
  }
  bool shouldSkipRemainingInstructions() const {
    return ShouldSkipRemainingInstructions;
  }
  bool isUnconditionallyExecuted() const { return WaitingForLabel == nullptr; }

  /// Note: the handle*() functions return true to indicate that the instruction
  /// has now been handled and that the instruction loop should continue to the
  /// next instruction in the block (and return false otherwise).  In addition,
  /// they set the ShouldSkipRemainingInstructions flag to indicate that no more
  /// instructions in the block should be processed.

  /// Handle an "unwanted" instruction by returning true;
  bool handleUnwantedInstruction() {
    // We can limit the splitting to an arbitrary subset of the instructions,
    // and still expect correct code.  As such, we can do instruction-subset
    // bisection to help debug any problems in this pass.
    static constexpr char AnInstructionHasNoName[] = "";
    if (!BuildDefs::minimal() &&
        !getFlags().matchSplitInsts(AnInstructionHasNoName,
                                    Instr->getNumber())) {
      return true;
    }
    if (!llvm::isa<InstTarget>(Instr)) {
      // Ignore non-lowered instructions like FakeDef/FakeUse.
      return true;
    }
    return false;
  }

  /// Process a potential label instruction.
  bool handleLabel() {
    if (!Instr->isLabel())
      return false;
    // A Label instruction shouldn't have any operands, so it can be handled
    // right here and then move on.
    assert(Dest == nullptr);
    assert(Instr->getSrcSize() == 0);
    if (Instr == WaitingForLabel) {
      // If we found the forward-branch-target Label instruction we're waiting
      // for, then clear the WaitingForLabel state.
      WaitingForLabel = nullptr;
    } else if (WaitingForLabel == nullptr && WaitingForBranchTo == nullptr) {
      // If we found a new Label instruction while the WaitingFor* state is
      // clear, then set things up for this being a backward branch target.
      WaitingForBranchTo = Instr;
    } else {
      // We see something we don't understand, so skip to the next block.
      ShouldSkipRemainingInstructions = true;
    }
    return true;
  }

  /// Process a potential intra-block branch instruction.
  bool handleIntraBlockBranch() {
    const Inst *Label = Instr->getIntraBlockBranchTarget();
    if (Label == nullptr)
      return false;
    // An intra-block branch instruction shouldn't have any operands, so it can
    // be handled right here and then move on.
    assert(Dest == nullptr);
    assert(Instr->getSrcSize() == 0);
    if (WaitingForBranchTo == Label && WaitingForLabel == nullptr) {
      WaitingForBranchTo = nullptr;
    } else if (WaitingForBranchTo == nullptr &&
               (WaitingForLabel == nullptr || WaitingForLabel == Label)) {
      WaitingForLabel = Label;
    } else {
      // We see something we don't understand, so skip to the next block.
      ShouldSkipRemainingInstructions = true;
    }
    return true;
  }

  /// Specially process a potential "Variable=Variable" assignment instruction,
  /// when it conforms to certain patterns.
  bool handleSimpleVarAssign() {
    if (!Instr->isVarAssign())
      return false;
    const bool DestIsInf = isInf(Dest);
    const bool DestIsAllocable = isAllocable(Dest);
    auto *SrcVar = llvm::cast<Variable>(Instr->getSrc(0));
    const bool SrcIsInf = isInf(SrcVar);
    const bool SrcIsAllocable = isAllocable(SrcVar);
    if (DestIsInf && SrcIsInf) {
      // The instruction:
      //   t:inf = u:inf
      // No transformation is needed.
      return true;
    }
    if (DestIsInf && SrcIsAllocable && Dest->getType() == SrcVar->getType()) {
      // The instruction:
      //   t:inf = v
      // gets transformed to:
      //   t:inf = v1
      //   v2 = t:inf
      // where:
      //   v1 := map[v]
      //   v2 := linkTo(v)
      //   map[v] := v2
      //
      // If both v2 and its linkedToStackRoot get a stack slot, then "v2=t:inf"
      // is recognized as a redundant assignment and elided.
      //
      // Note that if the dest and src types are different, then this is
      // actually a truncation operation, which would make "v2=t:inf" an invalid
      // instruction.  In this case, the type test will make it fall through to
      // the general case below.
      Variable *OldMapped = VarMap.get(SrcVar);
      Instr->replaceSource(0, OldMapped);
      if (isUnconditionallyExecuted()) {
        // Only create new mapping state if the instruction is unconditionally
        // executed.
        if (!VarMap.isInstLastUseOfVar(SrcVar, Instr)) {
          Variable *NewMapped = VarMap.makeLinked(SrcVar);
          Inst *Mov = Target->createLoweredMove(NewMapped, Dest);
          Node->getInsts().insert(IterNext, Mov);
        }
      }
      return true;
    }
    if (DestIsAllocable && SrcIsInf) {
      if (!VarMap.isDestUsedInBlock(Dest)) {
        return true;
      }
      // The instruction:
      //   v = t:inf
      // gets transformed to:
      //   v = t:inf
      //   v2 = t:inf
      // where:
      //   v2 := linkTo(v)
      //   map[v] := v2
      //
      // If both v2 and v get a stack slot, then "v2=t:inf" is recognized as a
      // redundant assignment and elided.
      if (isUnconditionallyExecuted()) {
        // Only create new mapping state if the instruction is unconditionally
        // executed.
        Variable *NewMapped = VarMap.makeLinked(Dest);
        Inst *Mov = Target->createLoweredMove(NewMapped, SrcVar);
        Node->getInsts().insert(IterNext, Mov);
      } else {
        // For a conditionally executed instruction, add a redefinition of the
        // original Dest mapping, without creating a new linked variable.
        Variable *OldMapped = VarMap.get(Dest);
        Inst *Mov = Target->createLoweredMove(OldMapped, SrcVar);
        Mov->setDestRedefined();
        Node->getInsts().insert(IterNext, Mov);
      }
      return true;
    }
    assert(!ShouldSkipRemainingInstructions);
    return false;
  }

  /// Process the dest Variable of a Phi instruction.
  bool handlePhi() {
    assert(llvm::isa<InstPhi>(Instr));
    const bool DestIsAllocable = isAllocable(Dest);
    if (!DestIsAllocable)
      return true;
    if (!VarMap.isDestUsedInBlock(Dest))
      return true;
    Variable *NewMapped = VarMap.makeLinked(Dest);
    Inst *Mov = Target->createLoweredMove(NewMapped, Dest);
    Node->getInsts().insert(IterCur, Mov);
    return true;
  }

  /// Process an arbitrary instruction.
  bool handleGeneralInst() {
    const bool DestIsAllocable = isAllocable(Dest);
    // The (non-variable-assignment) instruction:
    //   ... = F(v)
    // where v is not infinite-weight, gets transformed to:
    //   v2 = v1
    //   ... = F(v1)
    // where:
    //   v1 := map[v]
    //   v2 := linkTo(v)
    //   map[v] := v2
    // After that, if the "..." dest=u is not infinite-weight, append:
    //   u2 = u
    // where:
    //   u2 := linkTo(u)
    //   map[u] := u2
    for (SizeT i = 0; i < Instr->getSrcSize(); ++i) {
      // Iterate over the top-level src vars.  Don't bother to dig into
      // e.g. MemOperands because their vars should all be infinite-weight.
      // (This assumption would need to change if the pass were done
      // pre-lowering.)
      if (auto *SrcVar = llvm::dyn_cast<Variable>(Instr->getSrc(i))) {
        const bool SrcIsAllocable = isAllocable(SrcVar);
        if (SrcIsAllocable) {
          Variable *OldMapped = VarMap.get(SrcVar);
          if (isUnconditionallyExecuted()) {
            if (!VarMap.isInstLastUseOfVar(SrcVar, Instr)) {
              Variable *NewMapped = VarMap.makeLinked(SrcVar);
              Inst *Mov = Target->createLoweredMove(NewMapped, OldMapped);
              Node->getInsts().insert(IterCur, Mov);
            }
          }
          Instr->replaceSource(i, OldMapped);
        }
      }
    }
    // Transformation of Dest is the same as the "v=t:inf" case above.
    if (DestIsAllocable && VarMap.isDestUsedInBlock(Dest)) {
      if (isUnconditionallyExecuted()) {
        Variable *NewMapped = VarMap.makeLinked(Dest);
        Inst *Mov = Target->createLoweredMove(NewMapped, Dest);
        Node->getInsts().insert(IterNext, Mov);
      } else {
        Variable *OldMapped = VarMap.get(Dest);
        Inst *Mov = Target->createLoweredMove(OldMapped, Dest);
        Mov->setDestRedefined();
        Node->getInsts().insert(IterNext, Mov);
      }
    }
    return true;
  }

private:
  TargetLowering *Target;
  CfgNode *Node = nullptr;
  Inst *Instr = nullptr;
  Variable *Dest = nullptr;
  InstList::iterator IterCur;
  InstList::iterator IterNext;
  bool ShouldSkipRemainingInstructions = false;
  VariableMap VarMap;
  CfgVector<Variable *> LinkedToFixups;
  /// WaitingForLabel and WaitingForBranchTo are for tracking intra-block
  /// control flow.
  const Inst *WaitingForLabel = nullptr;
  const Inst *WaitingForBranchTo = nullptr;
};

} // end of anonymous namespace

/// Within each basic block, rewrite Variable references in terms of chained
/// copies of the original Variable.  For example:
///   A = B + C
/// might be rewritten as:
///   B1 = B
///   C1 = C
///   A = B + C
///   A1 = A
/// and then:
///   D = A + B
/// might be rewritten as:
///   A2 = A1
///   B2 = B1
///   D = A1 + B1
///   D1 = D
///
/// The purpose is to present the linear-scan register allocator with smaller
/// live ranges, to help mitigate its "all or nothing" allocation strategy,
/// while counting on its preference mechanism to keep the split versions in the
/// same register when possible.
///
/// When creating new Variables, A2 is linked to A1 which is linked to A, and
/// similar for the other Variable linked-to chains.  Rewrites apply only to
/// Variables where mayHaveReg() is true.
///
/// At code emission time, redundant linked-to stack assignments will be
/// recognized and elided.  To illustrate using the above example, if A1 gets a
/// register but A and A2 are on the stack, the "A2=A1" store instruction is
/// redundant since A and A2 share the same stack slot and A1 originated from A.
///
/// Simple assignment instructions are rewritten slightly differently, to take
/// maximal advantage of Variables known to have registers.
///
/// In general, there may be several valid ways to rewrite an instruction: add
/// the new assignment instruction either before or after the original
/// instruction, and rewrite the original instruction with either the old or the
/// new variable mapping.  We try to pick a strategy most likely to avoid
/// potential performance problems.  For example, try to avoid storing to the
/// stack and then immediately reloading from the same location.  One
/// consequence is that code might be generated that loads a register from a
/// stack location, followed almost immediately by another use of the same stack
/// location, despite its value already being available in a register as a
/// result of the first instruction.  However, the performance impact here is
/// likely to be negligible, and a simple availability peephole optimization
/// could clean it up.
///
/// This pass potentially adds a lot of new instructions and variables, and as
/// such there are compile-time performance concerns, particularly with liveness
/// analysis and register allocation.  Note that for liveness analysis, the new
/// variables have single-block liveness, so they don't increase the size of the
/// liveness bit vectors that need to be merged across blocks.  As a result, the
/// performance impact is likely to be linearly related to the number of new
/// instructions, rather than number of new variables times number of blocks
/// which would be the case if they were multi-block variables.
void splitBlockLocalVariables(Cfg *Func) {
  if (!getFlags().getSplitLocalVars())
    return;
  TimerMarker _(TimerStack::TT_splitLocalVars, Func);
  LocalVariableSplitter Splitter(Func);
  // TODO(stichnot): Fix this mechanism for LinkedTo variables and stack slot
  // assignment.
  //
  // To work around shortcomings with stack frame mapping, we want to arrange
  // LinkedTo structure such that within one block, the LinkedTo structure
  // leading to a root forms a list, not a tree.  A LinkedTo root can have
  // multiple children linking to it, but only one per block.  Furthermore,
  // because stack slot mapping processes variables in numerical order, the
  // LinkedTo chain needs to be ordered such that when A->getLinkedTo() == B,
  // then A->getIndex() > B->getIndex().
  //
  // To effect this, while processing a block we keep track of preexisting
  // LinkedTo relationships via the LinkedToFixups vector, and at the end of the
  // block we splice them in such that the block has a single chain for each
  // root, ordered by getIndex() value.
  CfgVector<Variable *> LinkedToFixups;
  for (CfgNode *Node : Func->getNodes()) {
    // Clear the VarMap and LinkedToFixups at the start of every block.
    LinkedToFixups.clear();
    Splitter.setNode(Node);
    auto &Insts = Node->getInsts();
    auto Iter = Insts.begin();
    auto IterEnd = Insts.end();
    // TODO(stichnot): Figure out why Phi processing usually degrades
    // performance.  Disable for now.
    static constexpr bool ProcessPhis = false;
    if (ProcessPhis) {
      for (Inst &Instr : Node->getPhis()) {
        if (Instr.isDeleted())
          continue;
        Splitter.setInst(&Instr, Iter, Iter);
        Splitter.handlePhi();
      }
    }
    InstList::iterator NextIter;
    for (; Iter != IterEnd && !Splitter.shouldSkipRemainingInstructions();
         Iter = NextIter) {
      NextIter = Iter;
      ++NextIter;
      Inst *Instr = iteratorToInst(Iter);
      if (Instr->isDeleted())
        continue;
      Splitter.setInst(Instr, Iter, NextIter);

      // Before doing any transformations, take care of the bookkeeping for
      // intra-block branching.
      //
      // This is tricky because the transformation for one instruction may
      // depend on a transformation for a previous instruction, but if that
      // previous instruction is not dynamically executed due to intra-block
      // control flow, it may lead to an inconsistent state and incorrect code.
      //
      // We want to handle some simple cases, and reject some others:
      //
      // 1. For something like a select instruction, we could have:
      //   test cond
      //   dest = src_false
      //   branch conditionally to label
      //   dest = src_true
      //   label:
      //
      // Between the conditional branch and the label, we need to treat dest and
      // src variables specially, specifically not creating any new state.
      //
      // 2. Some 64-bit atomic instructions may be lowered to a loop:
      //   label:
      //   ...
      //   branch conditionally to label
      //
      // No special treatment is needed, but it's worth tracking so that case #1
      // above can also be handled.
      //
      // 3. Advanced switch lowering can create really complex intra-block
      // control flow, so when we recognize this, we should just stop splitting
      // for the remainder of the block (which isn't much since a switch
      // instruction is a terminator).
      //
      // 4. Other complex lowering, e.g. an i64 icmp on a 32-bit architecture,
      // can result in an if/then/else like structure with two labels.  One
      // possibility would be to suspect splitting for the remainder of the
      // lowered instruction, and then resume for the remainder of the block,
      // but since we don't have high-level instruction markers, we might as
      // well just stop splitting for the remainder of the block.
      if (Splitter.handleLabel())
        continue;
      if (Splitter.handleIntraBlockBranch())
        continue;
      if (Splitter.handleUnwantedInstruction())
        continue;

      // Intra-block bookkeeping is complete, now do the transformations.

      // Determine the transformation based on the kind of instruction, and
      // whether its Variables are infinite-weight.  New instructions can be
      // inserted before the current instruction via Iter, or after the current
      // instruction via NextIter.
      if (Splitter.handleSimpleVarAssign())
        continue;
      if (Splitter.handleGeneralInst())
        continue;
    }
    Splitter.finalizeNode();
  }

  Func->dump("After splitting local variables");
}

} // end of namespace Ice
