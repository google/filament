//===- subzero/src/IceRegAlloc.cpp - Linear-scan implementation -----------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the LinearScan class, which performs the linear-scan
/// register allocation after liveness analysis has been performed.
///
//===----------------------------------------------------------------------===//

#include "IceRegAlloc.h"

#include "IceBitVector.h"
#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceInstVarIter.h"
#include "IceOperand.h"
#include "IceTargetLowering.h"

#include "llvm/Support/Format.h"

namespace Ice {

namespace {

// Returns true if Var has any definitions within Item's live range.
// TODO(stichnot): Consider trimming the Definitions list similar to how the
// live ranges are trimmed, since all the overlapsDefs() tests are whether some
// variable's definitions overlap Cur, and trimming is with respect Cur.start.
// Initial tests show no measurable performance difference, so we'll keep the
// code simple for now.
bool overlapsDefs(const Cfg *Func, const Variable *Item, const Variable *Var) {
  constexpr bool UseTrimmed = true;
  VariablesMetadata *VMetadata = Func->getVMetadata();
  if (const Inst *FirstDef = VMetadata->getFirstDefinition(Var))
    if (Item->getLiveRange().overlapsInst(FirstDef->getNumber(), UseTrimmed))
      return true;
  for (const Inst *Def : VMetadata->getLatterDefinitions(Var)) {
    if (Item->getLiveRange().overlapsInst(Def->getNumber(), UseTrimmed))
      return true;
  }
  return false;
}

void dumpDisableOverlap(const Cfg *Func, const Variable *Var,
                        const char *Reason) {
  if (!BuildDefs::dump())
    return;
  if (!Func->isVerbose(IceV_LinearScan))
    return;

  VariablesMetadata *VMetadata = Func->getVMetadata();
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "Disabling Overlap due to " << Reason << " " << *Var
      << " LIVE=" << Var->getLiveRange() << " Defs=";
  if (const Inst *FirstDef = VMetadata->getFirstDefinition(Var))
    Str << FirstDef->getNumber();
  const InstDefList &Defs = VMetadata->getLatterDefinitions(Var);
  for (size_t i = 0; i < Defs.size(); ++i) {
    Str << "," << Defs[i]->getNumber();
  }
  Str << "\n";
}

void dumpLiveRange(const Variable *Var, const Cfg *Func) {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "R=";
  if (Var->hasRegTmp()) {
    Str << llvm::format("%2d", int(Var->getRegNumTmp()));
  } else {
    Str << "NA";
  }
  Str << "  V=";
  Var->dump(Func);
  Str << "  Range=" << Var->getLiveRange();
}

int32_t findMinWeightIndex(
    const SmallBitVector &RegMask,
    const llvm::SmallVector<RegWeight, LinearScan::REGS_SIZE> &Weights) {
  int MinWeightIndex = -1;
  for (RegNumT i : RegNumBVIter(RegMask)) {
    if (MinWeightIndex < 0 || Weights[i] < Weights[MinWeightIndex])
      MinWeightIndex = i;
  }
  assert(getFlags().getRegAllocReserve() || MinWeightIndex >= 0);
  return MinWeightIndex;
}

} // end of anonymous namespace

LinearScan::LinearScan(Cfg *Func)
    : Func(Func), Ctx(Func->getContext()), Target(Func->getTarget()),
      Verbose(BuildDefs::dump() && Func->isVerbose(IceV_LinearScan)),
      UseReserve(getFlags().getRegAllocReserve()) {}

// Prepare for full register allocation of all variables. We depend on liveness
// analysis to have calculated live ranges.
void LinearScan::initForGlobal() {
  TimerMarker T(TimerStack::TT_initUnhandled, Func);
  FindPreference = true;
  // For full register allocation, normally we want to enable FindOverlap
  // (meaning we look for opportunities for two overlapping live ranges to
  // safely share the same register). However, we disable it for phi-lowering
  // register allocation since no overlap opportunities should be available and
  // it's more expensive to look for opportunities.
  FindOverlap = (Kind != RAK_Phi);
  Unhandled.reserve(Vars.size());
  UnhandledPrecolored.reserve(Vars.size());
  // Gather the live ranges of all variables and add them to the Unhandled set.
  for (Variable *Var : Vars) {
    // Don't consider rematerializable variables.
    if (Var->isRematerializable())
      continue;
    // Explicitly don't consider zero-weight variables, which are meant to be
    // spill slots.
    if (Var->mustNotHaveReg())
      continue;
    // Don't bother if the variable has a null live range, which means it was
    // never referenced.
    if (Var->getLiveRange().isEmpty())
      continue;
    Var->untrimLiveRange();
    Unhandled.push_back(Var);
    if (Var->hasReg()) {
      Var->setRegNumTmp(Var->getRegNum());
      Var->setMustHaveReg();
      UnhandledPrecolored.push_back(Var);
    }
  }

  // Build the (ordered) list of FakeKill instruction numbers.
  Kills.clear();
  // Phi lowering should not be creating new call instructions, so there should
  // be no infinite-weight not-yet-colored live ranges that span a call
  // instruction, hence no need to construct the Kills list.
  if (Kind == RAK_Phi)
    return;
  for (CfgNode *Node : Func->getNodes()) {
    for (Inst &I : Node->getInsts()) {
      if (auto *Kill = llvm::dyn_cast<InstFakeKill>(&I)) {
        if (!Kill->isDeleted() && !Kill->getLinked()->isDeleted())
          Kills.push_back(I.getNumber());
      }
    }
  }
}

// Validate the integrity of the live ranges.  If there are any errors, it
// prints details and returns false.  On success, it returns true.
bool LinearScan::livenessValidateIntervals(
    const DefUseErrorList &DefsWithoutUses,
    const DefUseErrorList &UsesBeforeDefs,
    const CfgVector<InstNumberT> &LRBegin,
    const CfgVector<InstNumberT> &LREnd) const {
  if (DefsWithoutUses.empty() && UsesBeforeDefs.empty())
    return true;

  if (!BuildDefs::dump())
    return false;

  OstreamLocker L(Ctx);
  Ostream &Str = Ctx->getStrDump();
  for (SizeT VarNum : DefsWithoutUses) {
    Variable *Var = Vars[VarNum];
    Str << "LR def without use, instruction " << LRBegin[VarNum]
        << ", variable " << Var->getName() << "\n";
  }
  for (SizeT VarNum : UsesBeforeDefs) {
    Variable *Var = Vars[VarNum];
    Str << "LR use before def, instruction " << LREnd[VarNum] << ", variable "
        << Var->getName() << "\n";
  }
  return false;
}

// Prepare for very simple register allocation of only infinite-weight Variables
// while respecting pre-colored Variables. Some properties we take advantage of:
//
// * Live ranges of interest consist of a single segment.
//
// * Live ranges of interest never span a call instruction.
//
// * Phi instructions are not considered because either phis have already been
//   lowered, or they don't contain any pre-colored or infinite-weight
//   Variables.
//
// * We don't need to renumber instructions before computing live ranges because
//   all the high-level ICE instructions are deleted prior to lowering, and the
//   low-level instructions are added in monotonically increasing order.
//
// * There are no opportunities for register preference or allowing overlap.
//
// Some properties we aren't (yet) taking advantage of:
//
// * Because live ranges are a single segment, the Inactive set will always be
//   empty, and the live range trimming operation is unnecessary.
//
// * Calculating overlap of single-segment live ranges could be optimized a bit.
void LinearScan::initForInfOnly() {
  TimerMarker T(TimerStack::TT_initUnhandled, Func);
  FindPreference = false;
  FindOverlap = false;
  SizeT NumVars = 0;

  // Iterate across all instructions and record the begin and end of the live
  // range for each variable that is pre-colored or infinite weight.
  CfgVector<InstNumberT> LRBegin(Vars.size(), Inst::NumberSentinel);
  CfgVector<InstNumberT> LREnd(Vars.size(), Inst::NumberSentinel);
  DefUseErrorList DefsWithoutUses, UsesBeforeDefs;
  for (CfgNode *Node : Func->getNodes()) {
    for (Inst &Instr : Node->getInsts()) {
      if (Instr.isDeleted())
        continue;
      FOREACH_VAR_IN_INST(Var, Instr) {
        if (Var->getIgnoreLiveness())
          continue;
        if (Var->hasReg() || Var->mustHaveReg()) {
          SizeT VarNum = Var->getIndex();
          LREnd[VarNum] = Instr.getNumber();
          if (!Var->getIsArg() && LRBegin[VarNum] == Inst::NumberSentinel)
            UsesBeforeDefs.push_back(VarNum);
        }
      }
      if (const Variable *Var = Instr.getDest()) {
        if (!Var->getIgnoreLiveness() &&
            (Var->hasReg() || Var->mustHaveReg())) {
          if (LRBegin[Var->getIndex()] == Inst::NumberSentinel) {
            LRBegin[Var->getIndex()] = Instr.getNumber();
            ++NumVars;
          }
        }
      }
    }
  }

  Unhandled.reserve(NumVars);
  UnhandledPrecolored.reserve(NumVars);
  for (SizeT i = 0; i < Vars.size(); ++i) {
    Variable *Var = Vars[i];
    if (Var->isRematerializable())
      continue;
    if (LRBegin[i] != Inst::NumberSentinel) {
      if (LREnd[i] == Inst::NumberSentinel) {
        DefsWithoutUses.push_back(i);
        continue;
      }
      Unhandled.push_back(Var);
      Var->resetLiveRange();
      Var->addLiveRange(LRBegin[i], LREnd[i]);
      Var->untrimLiveRange();
      if (Var->hasReg()) {
        Var->setRegNumTmp(Var->getRegNum());
        Var->setMustHaveReg();
        UnhandledPrecolored.push_back(Var);
      }
      --NumVars;
    }
  }

  if (!livenessValidateIntervals(DefsWithoutUses, UsesBeforeDefs, LRBegin,
                                 LREnd)) {
    llvm::report_fatal_error("initForInfOnly: Liveness error");
    return;
  }

  if (!DefsWithoutUses.empty() || !UsesBeforeDefs.empty()) {
    if (BuildDefs::dump()) {
      OstreamLocker L(Ctx);
      Ostream &Str = Ctx->getStrDump();
      for (SizeT VarNum : DefsWithoutUses) {
        Variable *Var = Vars[VarNum];
        Str << "LR def without use, instruction " << LRBegin[VarNum]
            << ", variable " << Var->getName() << "\n";
      }
      for (SizeT VarNum : UsesBeforeDefs) {
        Variable *Var = Vars[VarNum];
        Str << "LR use before def, instruction " << LREnd[VarNum]
            << ", variable " << Var->getName() << "\n";
      }
    }
    llvm::report_fatal_error("initForInfOnly: Liveness error");
  }
  // This isn't actually a fatal condition, but it would be nice to know if we
  // somehow pre-calculated Unhandled's size wrong.
  assert(NumVars == 0);

  // Don't build up the list of Kills because we know that no infinite-weight
  // Variable has a live range spanning a call.
  Kills.clear();
}

void LinearScan::initForSecondChance() {
  TimerMarker T(TimerStack::TT_initUnhandled, Func);
  FindPreference = true;
  FindOverlap = true;
  const VarList &Vars = Func->getVariables();
  Unhandled.reserve(Vars.size());
  UnhandledPrecolored.reserve(Vars.size());
  for (Variable *Var : Vars) {
    if (Var->isRematerializable())
      continue;
    if (Var->hasReg()) {
      Var->untrimLiveRange();
      Var->setRegNumTmp(Var->getRegNum());
      Var->setMustHaveReg();
      UnhandledPrecolored.push_back(Var);
      Unhandled.push_back(Var);
    }
  }
  for (Variable *Var : Evicted) {
    Var->untrimLiveRange();
    Unhandled.push_back(Var);
  }
}

void LinearScan::init(RegAllocKind Kind, CfgSet<Variable *> ExcludeVars) {
  this->Kind = Kind;
  Unhandled.clear();
  UnhandledPrecolored.clear();
  Handled.clear();
  Inactive.clear();
  Active.clear();
  Vars.clear();
  Vars.reserve(Func->getVariables().size() - ExcludeVars.size());
  for (auto *Var : Func->getVariables()) {
    if (ExcludeVars.find(Var) == ExcludeVars.end())
      Vars.emplace_back(Var);
  }

  SizeT NumRegs = Target->getNumRegisters();
  RegAliases.resize(NumRegs);
  for (SizeT Reg = 0; Reg < NumRegs; ++Reg) {
    RegAliases[Reg] = &Target->getAliasesForRegister(RegNumT::fromInt(Reg));
  }

  switch (Kind) {
  case RAK_Unknown:
    llvm::report_fatal_error("Invalid RAK_Unknown");
    break;
  case RAK_Global:
  case RAK_Phi:
    initForGlobal();
    break;
  case RAK_InfOnly:
    initForInfOnly();
    break;
  case RAK_SecondChance:
    initForSecondChance();
    break;
  }

  Evicted.clear();

  auto CompareRanges = [](const Variable *L, const Variable *R) {
    InstNumberT Lstart = L->getLiveRange().getStart();
    InstNumberT Rstart = R->getLiveRange().getStart();
    if (Lstart == Rstart)
      return L->getIndex() < R->getIndex();
    return Lstart < Rstart;
  };
  // Do a reverse sort so that erasing elements (from the end) is fast.
  std::sort(Unhandled.rbegin(), Unhandled.rend(), CompareRanges);
  std::sort(UnhandledPrecolored.rbegin(), UnhandledPrecolored.rend(),
            CompareRanges);

  Handled.reserve(Unhandled.size());
  Inactive.reserve(Unhandled.size());
  Active.reserve(Unhandled.size());
  Evicted.reserve(Unhandled.size());
}

// This is called when Cur must be allocated a register but no registers are
// available across Cur's live range. To handle this, we find a register that is
// not explicitly used during Cur's live range, spill that register to a stack
// location right before Cur's live range begins, and fill (reload) the register
// from the stack location right after Cur's live range ends.
void LinearScan::addSpillFill(IterationState &Iter) {
  // Identify the actual instructions that begin and end Iter.Cur's live range.
  // Iterate through Iter.Cur's node's instruction list until we find the actual
  // instructions with instruction numbers corresponding to Iter.Cur's recorded
  // live range endpoints.  This sounds inefficient but shouldn't be a problem
  // in practice because:
  // (1) This function is almost never called in practice.
  // (2) Since this register over-subscription problem happens only for
  //     phi-lowered instructions, the number of instructions in the node is
  //     proportional to the number of phi instructions in the original node,
  //     which is never very large in practice.
  // (3) We still have to iterate through all instructions of Iter.Cur's live
  //     range to find all explicitly used registers (though the live range is
  //     usually only 2-3 instructions), so the main cost that could be avoided
  //     would be finding the instruction that begin's Iter.Cur's live range.
  assert(!Iter.Cur->getLiveRange().isEmpty());
  InstNumberT Start = Iter.Cur->getLiveRange().getStart();
  InstNumberT End = Iter.Cur->getLiveRange().getEnd();
  CfgNode *Node = Func->getVMetadata()->getLocalUseNode(Iter.Cur);
  assert(Node);
  InstList &Insts = Node->getInsts();
  InstList::iterator SpillPoint = Insts.end();
  InstList::iterator FillPoint = Insts.end();
  // Stop searching after we have found both the SpillPoint and the FillPoint.
  for (auto I = Insts.begin(), E = Insts.end();
       I != E && (SpillPoint == E || FillPoint == E); ++I) {
    if (I->getNumber() == Start)
      SpillPoint = I;
    if (I->getNumber() == End)
      FillPoint = I;
    if (SpillPoint != E) {
      // Remove from RegMask any physical registers referenced during Cur's live
      // range. Start looking after SpillPoint gets set, i.e. once Cur's live
      // range begins.
      FOREACH_VAR_IN_INST(Var, *I) {
        if (!Var->hasRegTmp())
          continue;
        const auto &Aliases = *RegAliases[Var->getRegNumTmp()];
        for (RegNumT RegAlias : RegNumBVIter(Aliases)) {
          Iter.RegMask[RegAlias] = false;
        }
      }
    }
  }
  assert(SpillPoint != Insts.end());
  assert(FillPoint != Insts.end());
  ++FillPoint;
  // TODO(stichnot): Randomize instead of *.begin() which maps to find_first().
  const RegNumT RegNum = *RegNumBVIter(Iter.RegMask).begin();
  Iter.Cur->setRegNumTmp(RegNum);
  Variable *Preg = Target->getPhysicalRegister(RegNum, Iter.Cur->getType());
  // TODO(stichnot): Add SpillLoc to VariablesMetadata tracking so that SpillLoc
  // is correctly identified as !isMultiBlock(), reducing stack frame size.
  Variable *SpillLoc = Func->makeVariable(Iter.Cur->getType());
  // Add "reg=FakeDef;spill=reg" before SpillPoint
  Target->lowerInst(Node, SpillPoint, InstFakeDef::create(Func, Preg));
  Target->lowerInst(Node, SpillPoint, InstAssign::create(Func, SpillLoc, Preg));
  // add "reg=spill;FakeUse(reg)" before FillPoint
  Target->lowerInst(Node, FillPoint, InstAssign::create(Func, Preg, SpillLoc));
  Target->lowerInst(Node, FillPoint, InstFakeUse::create(Func, Preg));
}

void LinearScan::handleActiveRangeExpiredOrInactive(const Variable *Cur) {
  for (SizeT I = Active.size(); I > 0; --I) {
    const SizeT Index = I - 1;
    Variable *Item = Active[Index];
    Item->trimLiveRange(Cur->getLiveRange().getStart());
    bool Moved = false;
    if (Item->rangeEndsBefore(Cur)) {
      // Move Item from Active to Handled list.
      dumpLiveRangeTrace("Expiring     ", Item);
      moveItem(Active, Index, Handled);
      Moved = true;
    } else if (!Item->rangeOverlapsStart(Cur)) {
      // Move Item from Active to Inactive list.
      dumpLiveRangeTrace("Inactivating ", Item);
      moveItem(Active, Index, Inactive);
      Moved = true;
    }
    if (Moved) {
      // Decrement Item from RegUses[].
      assert(Item->hasRegTmp());
      const auto &Aliases = *RegAliases[Item->getRegNumTmp()];
      for (RegNumT RegAlias : RegNumBVIter(Aliases)) {
        --RegUses[RegAlias];
        assert(RegUses[RegAlias] >= 0);
      }
    }
  }
}

void LinearScan::handleInactiveRangeExpiredOrReactivated(const Variable *Cur) {
  for (SizeT I = Inactive.size(); I > 0; --I) {
    const SizeT Index = I - 1;
    Variable *Item = Inactive[Index];
    Item->trimLiveRange(Cur->getLiveRange().getStart());
    if (Item->rangeEndsBefore(Cur)) {
      // Move Item from Inactive to Handled list.
      dumpLiveRangeTrace("Expiring     ", Item);
      moveItem(Inactive, Index, Handled);
    } else if (Item->rangeOverlapsStart(Cur)) {
      // Move Item from Inactive to Active list.
      dumpLiveRangeTrace("Reactivating ", Item);
      moveItem(Inactive, Index, Active);
      // Increment Item in RegUses[].
      assert(Item->hasRegTmp());
      const auto &Aliases = *RegAliases[Item->getRegNumTmp()];
      for (RegNumT RegAlias : RegNumBVIter(Aliases)) {
        assert(RegUses[RegAlias] >= 0);
        ++RegUses[RegAlias];
      }
    }
  }
}

// Infer register preference and allowable overlap. Only form a preference when
// the current Variable has an unambiguous "first" definition. The preference is
// some source Variable of the defining instruction that either is assigned a
// register that is currently free, or that is assigned a register that is not
// free but overlap is allowed. Overlap is allowed when the Variable under
// consideration is single-definition, and its definition is a simple assignment
// - i.e., the register gets copied/aliased but is never modified.  Furthermore,
// overlap is only allowed when preferred Variable definition instructions do
// not appear within the current Variable's live range.
void LinearScan::findRegisterPreference(IterationState &Iter) {
  Iter.Prefer = nullptr;
  Iter.PreferReg = RegNumT();
  Iter.AllowOverlap = false;

  if (!FindPreference)
    return;

  VariablesMetadata *VMetadata = Func->getVMetadata();
  const Inst *DefInst = VMetadata->getFirstDefinitionSingleBlock(Iter.Cur);
  if (DefInst == nullptr)
    return;

  assert(DefInst->getDest() == Iter.Cur);
  const bool IsSingleDefAssign =
      DefInst->isVarAssign() && !VMetadata->isMultiDef(Iter.Cur);
  FOREACH_VAR_IN_INST(SrcVar, *DefInst) {
    // Only consider source variables that have (so far) been assigned a
    // register.
    if (!SrcVar->hasRegTmp())
      continue;

    // That register must be one in the RegMask set, e.g. don't try to prefer
    // the stack pointer as a result of the stacksave intrinsic.
    const auto &Aliases = *RegAliases[SrcVar->getRegNumTmp()];
    const int SrcReg = (Iter.RegMask & Aliases).find_first();
    if (SrcReg == -1)
      continue;

    if (FindOverlap && IsSingleDefAssign && !Iter.Free[SrcReg]) {
      // Don't bother trying to enable AllowOverlap if the register is already
      // free (hence the test on Iter.Free[SrcReg]).
      Iter.AllowOverlap = !overlapsDefs(Func, Iter.Cur, SrcVar);
    }
    if (Iter.AllowOverlap || Iter.Free[SrcReg]) {
      Iter.Prefer = SrcVar;
      Iter.PreferReg = RegNumT::fromInt(SrcReg);
      // Stop looking for a preference after the first valid preference is
      // found.  One might think that we should look at all instruction
      // variables to find the best <Prefer,AllowOverlap> combination, but note
      // that AllowOverlap can only be true for a simple assignment statement
      // which can have only one source operand, so it's not possible for
      // AllowOverlap to be true beyond the first source operand.
      FOREACH_VAR_IN_INST_BREAK;
    }
  }
  if (BuildDefs::dump() && Verbose && Iter.Prefer) {
    Ostream &Str = Ctx->getStrDump();
    Str << "Initial Iter.Prefer=";
    Iter.Prefer->dump(Func);
    Str << " R=" << Iter.PreferReg << " LIVE=" << Iter.Prefer->getLiveRange()
        << " Overlap=" << Iter.AllowOverlap << "\n";
  }
}

// Remove registers from the Iter.Free[] list where an Inactive range overlaps
// with the current range.
void LinearScan::filterFreeWithInactiveRanges(IterationState &Iter) {
  for (const Variable *Item : Inactive) {
    if (!Item->rangeOverlaps(Iter.Cur))
      continue;
    const auto &Aliases = *RegAliases[Item->getRegNumTmp()];
    for (RegNumT RegAlias : RegNumBVIter(Aliases)) {
      // Don't assert(Iter.Free[RegAlias]) because in theory (though probably
      // never in practice) there could be two inactive variables that were
      // marked with AllowOverlap.
      Iter.Free[RegAlias] = false;
      Iter.FreeUnfiltered[RegAlias] = false;
      // Disable AllowOverlap if an Inactive variable, which is not Prefer,
      // shares Prefer's register, and has a definition within Cur's live range.
      if (Iter.AllowOverlap && Item != Iter.Prefer &&
          RegAlias == Iter.PreferReg && overlapsDefs(Func, Iter.Cur, Item)) {
        Iter.AllowOverlap = false;
        dumpDisableOverlap(Func, Item, "Inactive");
      }
    }
  }
}

// Remove registers from the Iter.Free[] list where an Unhandled pre-colored
// range overlaps with the current range, and set those registers to infinite
// weight so that they aren't candidates for eviction.
// Cur->rangeEndsBefore(Item) is an early exit check that turns a guaranteed
// O(N^2) algorithm into expected linear complexity.
void LinearScan::filterFreeWithPrecoloredRanges(IterationState &Iter) {
  // TODO(stichnot): Partition UnhandledPrecolored according to register class,
  // to restrict the number of overlap comparisons needed.
  for (Variable *Item : reverse_range(UnhandledPrecolored)) {
    assert(Item->hasReg());
    if (Iter.Cur->rangeEndsBefore(Item))
      break;
    if (!Item->rangeOverlaps(Iter.Cur))
      continue;
    const auto &Aliases =
        *RegAliases[Item->getRegNum()]; // Note: not getRegNumTmp()
    for (RegNumT RegAlias : RegNumBVIter(Aliases)) {
      Iter.Weights[RegAlias].setWeight(RegWeight::Inf);
      Iter.Free[RegAlias] = false;
      Iter.FreeUnfiltered[RegAlias] = false;
      Iter.PrecoloredUnhandledMask[RegAlias] = true;
      // Disable Iter.AllowOverlap if the preferred register is one of these
      // pre-colored unhandled overlapping ranges.
      if (Iter.AllowOverlap && RegAlias == Iter.PreferReg) {
        Iter.AllowOverlap = false;
        dumpDisableOverlap(Func, Item, "PrecoloredUnhandled");
      }
    }
  }
}

void LinearScan::allocatePrecoloredRegister(Variable *Cur) {
  const auto RegNum = Cur->getRegNum();
  // RegNumTmp should have already been set above.
  assert(Cur->getRegNumTmp() == RegNum);
  dumpLiveRangeTrace("Precoloring  ", Cur);
  Active.push_back(Cur);
  const auto &Aliases = *RegAliases[RegNum];
  for (RegNumT RegAlias : RegNumBVIter(Aliases)) {
    assert(RegUses[RegAlias] >= 0);
    ++RegUses[RegAlias];
  }
  assert(!UnhandledPrecolored.empty());
  assert(UnhandledPrecolored.back() == Cur);
  UnhandledPrecolored.pop_back();
}

void LinearScan::allocatePreferredRegister(IterationState &Iter) {
  Iter.Cur->setRegNumTmp(Iter.PreferReg);
  dumpLiveRangeTrace("Preferring   ", Iter.Cur);
  const auto &Aliases = *RegAliases[Iter.PreferReg];
  for (RegNumT RegAlias : RegNumBVIter(Aliases)) {
    assert(RegUses[RegAlias] >= 0);
    ++RegUses[RegAlias];
  }
  Active.push_back(Iter.Cur);
}

void LinearScan::allocateFreeRegister(IterationState &Iter, bool Filtered) {
  const RegNumT RegNum =
      *RegNumBVIter(Filtered ? Iter.Free : Iter.FreeUnfiltered).begin();
  Iter.Cur->setRegNumTmp(RegNum);
  if (Filtered)
    dumpLiveRangeTrace("Allocating Y ", Iter.Cur);
  else
    dumpLiveRangeTrace("Allocating X ", Iter.Cur);
  const auto &Aliases = *RegAliases[RegNum];
  for (RegNumT RegAlias : RegNumBVIter(Aliases)) {
    assert(RegUses[RegAlias] >= 0);
    ++RegUses[RegAlias];
  }
  Active.push_back(Iter.Cur);
}

void LinearScan::handleNoFreeRegisters(IterationState &Iter) {
  // Check Active ranges.
  for (const Variable *Item : Active) {
    assert(Item->rangeOverlaps(Iter.Cur));
    assert(Item->hasRegTmp());
    const auto &Aliases = *RegAliases[Item->getRegNumTmp()];
    // We add the Item's weight to each alias/subregister to represent that,
    // should we decide to pick any of them, then we would incur that many
    // memory accesses.
    RegWeight W = Item->getWeight(Func);
    for (RegNumT RegAlias : RegNumBVIter(Aliases)) {
      Iter.Weights[RegAlias].addWeight(W);
    }
  }
  // Same as above, but check Inactive ranges instead of Active.
  for (const Variable *Item : Inactive) {
    if (!Item->rangeOverlaps(Iter.Cur))
      continue;
    assert(Item->hasRegTmp());
    const auto &Aliases = *RegAliases[Item->getRegNumTmp()];
    RegWeight W = Item->getWeight(Func);
    for (RegNumT RegAlias : RegNumBVIter(Aliases)) {
      Iter.Weights[RegAlias].addWeight(W);
    }
  }

  // All the weights are now calculated. Find the register with smallest weight.
  int32_t MinWeightIndex = findMinWeightIndex(Iter.RegMask, Iter.Weights);

  if (MinWeightIndex < 0 ||
      Iter.Cur->getWeight(Func) <= Iter.Weights[MinWeightIndex]) {
    if (!Iter.Cur->mustHaveReg()) {
      // Iter.Cur doesn't have priority over any other live ranges, so don't
      // allocate any register to it, and move it to the Handled state.
      Handled.push_back(Iter.Cur);
      return;
    }
    if (Kind == RAK_Phi) {
      // Iter.Cur is infinite-weight but all physical registers are already
      // taken, so we need to force one to be temporarily available.
      addSpillFill(Iter);
      Handled.push_back(Iter.Cur);
      return;
    }
    // The remaining portion of the enclosing "if" block should only be
    // reachable if we are manually limiting physical registers for testing.
    if (UseReserve) {
      if (Iter.FreeUnfiltered.any()) {
        // There is some available physical register held in reserve, so use it.
        constexpr bool NotFiltered = false;
        allocateFreeRegister(Iter, NotFiltered);
        // Iter.Cur is now on the Active list.
        return;
      }
      // At this point, we need to find some reserve register that is already
      // assigned to a non-infinite-weight variable.  This could happen if some
      // variable was previously assigned an alias of such a register.
      MinWeightIndex = findMinWeightIndex(Iter.RegMaskUnfiltered, Iter.Weights);
    }
    if (Iter.Cur->getWeight(Func) <= Iter.Weights[MinWeightIndex]) {
      dumpLiveRangeTrace("Failing      ", Iter.Cur);
      Func->setError("Unable to find a physical register for an "
                     "infinite-weight live range "
                     "(consider using -reg-reserve): " +
                     Iter.Cur->getName());
      Handled.push_back(Iter.Cur);
      return;
    }
    // At this point, MinWeightIndex points to a valid reserve register to
    // reallocate to Iter.Cur, so drop into the eviction code.
  }

  // Evict all live ranges in Active that register number MinWeightIndex is
  // assigned to.
  const auto &Aliases = *RegAliases[MinWeightIndex];
  for (SizeT I = Active.size(); I > 0; --I) {
    const SizeT Index = I - 1;
    Variable *Item = Active[Index];
    const auto RegNum = Item->getRegNumTmp();
    if (Aliases[RegNum]) {
      dumpLiveRangeTrace("Evicting A   ", Item);
      const auto &Aliases = *RegAliases[RegNum];
      for (RegNumT RegAlias : RegNumBVIter(Aliases)) {
        --RegUses[RegAlias];
        assert(RegUses[RegAlias] >= 0);
      }
      Item->setRegNumTmp(RegNumT());
      moveItem(Active, Index, Handled);
      Evicted.push_back(Item);
    }
  }
  // Do the same for Inactive.
  for (SizeT I = Inactive.size(); I > 0; --I) {
    const SizeT Index = I - 1;
    Variable *Item = Inactive[Index];
    // Note: The Item->rangeOverlaps(Cur) clause is not part of the description
    // of AssignMemLoc() in the original paper. But there doesn't seem to be any
    // need to evict an inactive live range that doesn't overlap with the live
    // range currently being considered. It's especially bad if we would end up
    // evicting an infinite-weight but currently-inactive live range. The most
    // common situation for this would be a scratch register kill set for call
    // instructions.
    if (Aliases[Item->getRegNumTmp()] && Item->rangeOverlaps(Iter.Cur)) {
      dumpLiveRangeTrace("Evicting I   ", Item);
      Item->setRegNumTmp(RegNumT());
      moveItem(Inactive, Index, Handled);
      Evicted.push_back(Item);
    }
  }
  // Assign the register to Cur.
  Iter.Cur->setRegNumTmp(RegNumT::fromInt(MinWeightIndex));
  for (RegNumT RegAlias : RegNumBVIter(Aliases)) {
    assert(RegUses[RegAlias] >= 0);
    ++RegUses[RegAlias];
  }
  Active.push_back(Iter.Cur);
  dumpLiveRangeTrace("Allocating Z ", Iter.Cur);
}

void LinearScan::assignFinalRegisters(const SmallBitVector &RegMaskFull) {
  // Finish up by setting RegNum = RegNumTmp (or a random permutation thereof)
  // for each Variable.
  for (Variable *Item : Handled) {
    const auto RegNum = Item->getRegNumTmp();
    auto AssignedRegNum = RegNum;

    if (BuildDefs::dump() && Verbose) {
      Ostream &Str = Ctx->getStrDump();
      if (!Item->hasRegTmp()) {
        Str << "Not assigning ";
        Item->dump(Func);
        Str << "\n";
      } else {
        Str << (AssignedRegNum == Item->getRegNum() ? "Reassigning "
                                                    : "Assigning ")
            << Target->getRegName(AssignedRegNum, Item->getType()) << "(r"
            << AssignedRegNum << ") to ";
        Item->dump(Func);
        Str << "\n";
      }
    }
    Item->setRegNum(AssignedRegNum);
  }
}

// Implements the linear-scan algorithm. Based on "Linear Scan Register
// Allocation in the Context of SSA Form and Register Constraints" by Hanspeter
// Mössenböck and Michael Pfeiffer,
// ftp://ftp.ssw.uni-linz.ac.at/pub/Papers/Moe02.PDF. This implementation is
// modified to take affinity into account and allow two interfering variables to
// share the same register in certain cases.
//
// Requires running Cfg::liveness(Liveness_Intervals) in preparation. Results
// are assigned to Variable::RegNum for each Variable.
void LinearScan::scan(const SmallBitVector &RegMaskFull) {
  TimerMarker T(TimerStack::TT_linearScan, Func);
  assert(RegMaskFull.any()); // Sanity check
  if (Verbose)
    Ctx->lockStr();
  Func->resetCurrentNode();
  const size_t NumRegisters = RegMaskFull.size();

  // Build a LiveRange representing the Kills list.
  LiveRange KillsRange(Kills);
  KillsRange.untrim();

  // Reset the register use count.
  RegUses.resize(NumRegisters);
  std::fill(RegUses.begin(), RegUses.end(), 0);

  // Unhandled is already set to all ranges in increasing order of start points.
  assert(Active.empty());
  assert(Inactive.empty());
  assert(Handled.empty());
  const TargetLowering::RegSetMask RegsInclude =
      TargetLowering::RegSet_CallerSave;
  const TargetLowering::RegSetMask RegsExclude = TargetLowering::RegSet_None;
  const SmallBitVector KillsMask =
      Target->getRegisterSet(RegsInclude, RegsExclude);

  // Allocate memory once outside the loop.
  IterationState Iter;
  Iter.Weights.reserve(NumRegisters);
  Iter.PrecoloredUnhandledMask.reserve(NumRegisters);

  while (!Unhandled.empty()) {
    Iter.Cur = Unhandled.back();
    Unhandled.pop_back();
    dumpLiveRangeTrace("\nConsidering  ", Iter.Cur);
    if (UseReserve)
      assert(Target->getAllRegistersForVariable(Iter.Cur).any());
    else
      assert(Target->getRegistersForVariable(Iter.Cur).any());
    Iter.RegMask = RegMaskFull & Target->getRegistersForVariable(Iter.Cur);
    Iter.RegMaskUnfiltered =
        RegMaskFull & Target->getAllRegistersForVariable(Iter.Cur);
    KillsRange.trim(Iter.Cur->getLiveRange().getStart());

    // Check for pre-colored ranges. If Cur is pre-colored, it definitely gets
    // that register. Previously processed live ranges would have avoided that
    // register due to it being pre-colored. Future processed live ranges won't
    // evict that register because the live range has infinite weight.
    if (Iter.Cur->hasReg()) {
      allocatePrecoloredRegister(Iter.Cur);
      continue;
    }

    handleActiveRangeExpiredOrInactive(Iter.Cur);
    handleInactiveRangeExpiredOrReactivated(Iter.Cur);

    // Calculate available registers into Iter.Free[] and Iter.FreeUnfiltered[].
    Iter.Free = Iter.RegMask;
    Iter.FreeUnfiltered = Iter.RegMaskUnfiltered;
    for (SizeT i = 0; i < Iter.RegMask.size(); ++i) {
      if (RegUses[i] > 0) {
        Iter.Free[i] = false;
        Iter.FreeUnfiltered[i] = false;
      }
    }

    findRegisterPreference(Iter);
    filterFreeWithInactiveRanges(Iter);

    // Disable AllowOverlap if an Active variable, which is not Prefer, shares
    // Prefer's register, and has a definition within Cur's live range.
    if (Iter.AllowOverlap) {
      const auto &Aliases = *RegAliases[Iter.PreferReg];
      for (const Variable *Item : Active) {
        const RegNumT RegNum = Item->getRegNumTmp();
        if (Item != Iter.Prefer && Aliases[RegNum] &&
            overlapsDefs(Func, Iter.Cur, Item)) {
          Iter.AllowOverlap = false;
          dumpDisableOverlap(Func, Item, "Active");
        }
      }
    }

    Iter.Weights.resize(Iter.RegMask.size());
    std::fill(Iter.Weights.begin(), Iter.Weights.end(), RegWeight());

    Iter.PrecoloredUnhandledMask.resize(Iter.RegMask.size());
    Iter.PrecoloredUnhandledMask.reset();

    filterFreeWithPrecoloredRanges(Iter);

    // Remove scratch registers from the Iter.Free[] list, and mark their
    // Iter.Weights[] as infinite, if KillsRange overlaps Cur's live range.
    constexpr bool UseTrimmed = true;
    if (Iter.Cur->getLiveRange().overlaps(KillsRange, UseTrimmed)) {
      Iter.Free.reset(KillsMask);
      Iter.FreeUnfiltered.reset(KillsMask);
      for (RegNumT i : RegNumBVIter(KillsMask)) {
        Iter.Weights[i].setWeight(RegWeight::Inf);
        if (Iter.PreferReg == i)
          Iter.AllowOverlap = false;
      }
    }

    // Print info about physical register availability.
    if (BuildDefs::dump() && Verbose) {
      Ostream &Str = Ctx->getStrDump();
      for (RegNumT i : RegNumBVIter(Iter.RegMaskUnfiltered)) {
        Str << Target->getRegName(i, Iter.Cur->getType()) << "(U=" << RegUses[i]
            << ",F=" << Iter.Free[i] << ",P=" << Iter.PrecoloredUnhandledMask[i]
            << ") ";
      }
      Str << "\n";
    }

    if (Iter.Prefer && (Iter.AllowOverlap || Iter.Free[Iter.PreferReg])) {
      // First choice: a preferred register that is either free or is allowed to
      // overlap with its linked variable.
      allocatePreferredRegister(Iter);
    } else if (Iter.Free.any()) {
      // Second choice: any free register.
      constexpr bool Filtered = true;
      allocateFreeRegister(Iter, Filtered);
    } else {
      // Fallback: there are no free registers, so we look for the lowest-weight
      // register and see if Cur has higher weight.
      handleNoFreeRegisters(Iter);
    }
    dump(Func);
  }

  // Move anything Active or Inactive to Handled for easier handling.
  Handled.insert(Handled.end(), Active.begin(), Active.end());
  Active.clear();
  Handled.insert(Handled.end(), Inactive.begin(), Inactive.end());
  Inactive.clear();
  dump(Func);

  assignFinalRegisters(RegMaskFull);

  if (Verbose)
    Ctx->unlockStr();
}

// ======================== Dump routines ======================== //

void LinearScan::dumpLiveRangeTrace(const char *Label, const Variable *Item) {
  if (!BuildDefs::dump())
    return;

  if (Verbose) {
    Ostream &Str = Ctx->getStrDump();
    Str << Label;
    dumpLiveRange(Item, Func);
    Str << "\n";
  }
}

void LinearScan::dump(Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  if (!Verbose)
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Func->resetCurrentNode();
  Str << "**** Current regalloc state:\n";
  Str << "++++++ Handled:\n";
  for (const Variable *Item : Handled) {
    dumpLiveRange(Item, Func);
    Str << "\n";
  }
  Str << "++++++ Unhandled:\n";
  for (const Variable *Item : reverse_range(Unhandled)) {
    dumpLiveRange(Item, Func);
    Str << "\n";
  }
  Str << "++++++ Active:\n";
  for (const Variable *Item : Active) {
    dumpLiveRange(Item, Func);
    Str << "\n";
  }
  Str << "++++++ Inactive:\n";
  for (const Variable *Item : Inactive) {
    dumpLiveRange(Item, Func);
    Str << "\n";
  }
}

} // end of namespace Ice
