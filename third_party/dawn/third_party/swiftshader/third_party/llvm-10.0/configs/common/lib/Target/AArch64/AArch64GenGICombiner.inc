/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Global Combiner                                                            *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef AARCH64PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_DEPS
#include "llvm/ADT/SparseBitVector.h"
namespace llvm {
extern cl::OptionCategory GICombinerOptionCategory;
} // end namespace llvm
#endif // ifdef AARCH64PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_DEPS

#ifdef AARCH64PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_H
class AArch64GenPreLegalizerCombinerHelper {
  SparseBitVector<> DisabledRules;

public:
  bool parseCommandLineOption();
  bool isRuleDisabled(unsigned ID) const;
  bool setRuleDisabled(StringRef RuleIdentifier);

  bool tryCombineAll(
    GISelChangeObserver &Observer,
    MachineInstr &MI,
    MachineIRBuilder &B,
    CombinerHelper &Helper) const;
};

static Optional<uint64_t> getRuleIdxForIdentifier(StringRef RuleIdentifier) {
  uint64_t I;
  // getAtInteger(...) returns false on success
  bool Parsed = !RuleIdentifier.getAsInteger(0, I);
  if (Parsed)
    return I;

#ifndef NDEBUG
  switch (RuleIdentifier.size()) {
  default: break;
  case 9:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "copy_prop", 9) != 0)
      break;
    return 0;	 // "copy_prop"
  case 15:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "extending_loads", 15) != 0)
      break;
    return 2;	 // "extending_loads"
  case 19:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "ptr_add_immed_chain", 19) != 0)
      break;
    return 1;	 // "ptr_add_immed_chain"
  case 26:	 // 2 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'c':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ombine_indexed_load_store", 25) != 0)
        break;
      return 3;	 // "combine_indexed_load_store"
    case 'e':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "lide_br_by_inverting_cond", 25) != 0)
        break;
      return 4;	 // "elide_br_by_inverting_cond"
    }
    break;
  }
#endif // ifndef NDEBUG

  return None;
}
bool AArch64GenPreLegalizerCombinerHelper::setRuleDisabled(StringRef RuleIdentifier) {
  std::pair<StringRef, StringRef> RangePair = RuleIdentifier.split('-');
  if (!RangePair.second.empty()) {
    const auto First = getRuleIdxForIdentifier(RangePair.first);
    const auto Last = getRuleIdxForIdentifier(RangePair.second);
    if (!First.hasValue() || !Last.hasValue())
      return false;
    if (First >= Last)
      report_fatal_error("Beginning of range should be before end of range");
    for (auto I = First.getValue(); I < Last.getValue(); ++I)
      DisabledRules.set(I);
    return true;
  } else {
    const auto I = getRuleIdxForIdentifier(RangePair.first);
    if (!I.hasValue())
      return false;
    DisabledRules.set(I.getValue());
    return true;
  }
  return false;
}
bool AArch64GenPreLegalizerCombinerHelper::isRuleDisabled(unsigned RuleID) const {
  return DisabledRules.test(RuleID);
}
#endif // ifdef AARCH64PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_H

#ifdef AARCH64PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_CPP

cl::list<std::string> AArch64PreLegalizerCombinerHelperOption(
    "aarch64prelegalizercombinerhelper-disable-rule",
    cl::desc("Disable one or more combiner rules temporarily in the AArch64PreLegalizerCombinerHelper pass"),
    cl::CommaSeparated,
    cl::Hidden,
    cl::cat(GICombinerOptionCategory));

bool AArch64GenPreLegalizerCombinerHelper::parseCommandLineOption() {
  for (const auto &Identifier : AArch64PreLegalizerCombinerHelperOption)
    if (!setRuleDisabled(Identifier))
      return false;
  return true;
}

bool AArch64GenPreLegalizerCombinerHelper::tryCombineAll(
    GISelChangeObserver &Observer,
    MachineInstr &MI,
    MachineIRBuilder &B,
    CombinerHelper &Helper) const {
  MachineBasicBlock *MBB = MI.getParent();
  MachineFunction *MF = MBB->getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  SmallVector<MachineInstr *, 8> MIs = { &MI };

  (void)MBB; (void)MF; (void)MRI;

  // Match data
  PtrAddChain MatchData1;
  PreferredTuple MatchData2;
  IndexedLoadStoreMatchInfo MatchData3;

  int Partition = -1;
  Partition = -1;
  switch (MIs[0]->getOpcode()) {
  case TargetOpcode::COPY: Partition = 0; break;
  case TargetOpcode::G_PTR_ADD: Partition = 1; break;
  case TargetOpcode::G_LOAD: Partition = 2; break;
  case TargetOpcode::G_SEXTLOAD: Partition = 3; break;
  case TargetOpcode::G_ZEXTLOAD: Partition = 4; break;
  case TargetOpcode::G_STORE: Partition = 5; break;
  case TargetOpcode::G_BR: Partition = 6; break;
  }
  // Default case but without conflicting with potential default case in selection.
  if (Partition == -1) return false;
  if (Partition == 0 /* TargetOpcode::COPY */) {
    // Leaf name: copy_prop
    // Rule: copy_prop
    if (!isRuleDisabled(0)) {
      if (1
          && [&]() {
           return Helper.matchCombineCopy(*MIs[0]); 
          return true;
      }()) {
        Helper.applyCombineCopy(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 1 /* TargetOpcode::G_PTR_ADD */) {
    // Leaf name: ptr_add_immed_chain
    // Rule: ptr_add_immed_chain
    if (!isRuleDisabled(1)) {
      if (1
          && [&]() {
           return Helper.matchPtrAddImmedChain(*MIs[0], MatchData1); 
          return true;
      }()) {
        Helper.applyPtrAddImmedChain(*MIs[0], MatchData1); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 2 /* TargetOpcode::G_LOAD */) {
    // Leaf name: extending_loads
    // Rule: extending_loads
    if (!isRuleDisabled(2)) {
      if (1
          && [&]() {
           return Helper.matchCombineExtendingLoads(*MIs[0], MatchData2); 
          return true;
      }()) {
        Helper.applyCombineExtendingLoads(*MIs[0], MatchData2); 
        return true;
      }
    }
    // Leaf name: combine_indexed_load_store
    // Rule: combine_indexed_load_store
    if (!isRuleDisabled(3)) {
      if (1
          && [&]() {
           return Helper.matchCombineIndexedLoadStore(*MIs[0], MatchData3); 
          return true;
      }()) {
        Helper.applyCombineIndexedLoadStore(*MIs[0], MatchData3); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 3 /* TargetOpcode::G_SEXTLOAD */) {
    // Leaf name: extending_loads
    // Rule: extending_loads
    if (!isRuleDisabled(2)) {
      if (1
          && [&]() {
           return Helper.matchCombineExtendingLoads(*MIs[0], MatchData2); 
          return true;
      }()) {
        Helper.applyCombineExtendingLoads(*MIs[0], MatchData2); 
        return true;
      }
    }
    // Leaf name: combine_indexed_load_store
    // Rule: combine_indexed_load_store
    if (!isRuleDisabled(3)) {
      if (1
          && [&]() {
           return Helper.matchCombineIndexedLoadStore(*MIs[0], MatchData3); 
          return true;
      }()) {
        Helper.applyCombineIndexedLoadStore(*MIs[0], MatchData3); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 4 /* TargetOpcode::G_ZEXTLOAD */) {
    // Leaf name: extending_loads
    // Rule: extending_loads
    if (!isRuleDisabled(2)) {
      if (1
          && [&]() {
           return Helper.matchCombineExtendingLoads(*MIs[0], MatchData2); 
          return true;
      }()) {
        Helper.applyCombineExtendingLoads(*MIs[0], MatchData2); 
        return true;
      }
    }
    // Leaf name: combine_indexed_load_store
    // Rule: combine_indexed_load_store
    if (!isRuleDisabled(3)) {
      if (1
          && [&]() {
           return Helper.matchCombineIndexedLoadStore(*MIs[0], MatchData3); 
          return true;
      }()) {
        Helper.applyCombineIndexedLoadStore(*MIs[0], MatchData3); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 5 /* TargetOpcode::G_STORE */) {
    // Leaf name: combine_indexed_load_store
    // Rule: combine_indexed_load_store
    if (!isRuleDisabled(3)) {
      if (1
          && [&]() {
           return Helper.matchCombineIndexedLoadStore(*MIs[0], MatchData3); 
          return true;
      }()) {
        Helper.applyCombineIndexedLoadStore(*MIs[0], MatchData3); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 6 /* TargetOpcode::G_BR */) {
    // Leaf name: elide_br_by_inverting_cond
    // Rule: elide_br_by_inverting_cond
    if (!isRuleDisabled(4)) {
      if (1
          && [&]() {
           return Helper.matchElideBrByInvertingCond(*MIs[0]); 
          return true;
      }()) {
        Helper.applyElideBrByInvertingCond(*MIs[0]); 
        return true;
      }
    }
    return false;
  }

  return false;
}
#endif // ifdef AARCH64PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_CPP
