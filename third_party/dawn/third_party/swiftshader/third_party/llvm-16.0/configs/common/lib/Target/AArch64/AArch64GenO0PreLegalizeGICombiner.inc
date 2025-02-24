/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Global Combiner                                                            *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef AARCH64O0PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_DEPS
#include "llvm/ADT/SparseBitVector.h"
namespace llvm {
extern cl::OptionCategory GICombinerOptionCategory;
} // end namespace llvm
#endif // ifdef AARCH64O0PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_DEPS

#ifdef AARCH64O0PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_H
class AArch64GenO0PreLegalizerCombinerHelperRuleConfig {
  SparseBitVector<> DisabledRules;

public:
  bool parseCommandLineOption();
  bool isRuleDisabled(unsigned ID) const;
  bool setRuleEnabled(StringRef RuleIdentifier);
  bool setRuleDisabled(StringRef RuleIdentifier);
};

class AArch64GenO0PreLegalizerCombinerHelper : public AArch64O0PreLegalizerCombinerHelperState {
  const AArch64GenO0PreLegalizerCombinerHelperRuleConfig *RuleConfig;

public:
  template <typename... Args>AArch64GenO0PreLegalizerCombinerHelper(const AArch64GenO0PreLegalizerCombinerHelperRuleConfig &RuleConfig, Args &&... args) : AArch64O0PreLegalizerCombinerHelperState(std::forward<Args>(args)...), RuleConfig(&RuleConfig) {}

  bool tryCombineAll(
    GISelChangeObserver &Observer,
    MachineInstr &MI,
    MachineIRBuilder &B) const;
};

static std::optional<uint64_t> getRuleIdxForIdentifier(StringRef RuleIdentifier) {
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
  case 10:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "mul_to_shl", 10) != 0)
      break;
    return 1;	 // "mul_to_shl"
  case 12:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "not_cmp_fold", 12) != 0)
      break;
    return 8;	 // "not_cmp_fold"
  case 13:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "load_and_mask", 13) != 0)
      break;
    return 7;	 // "load_and_mask"
  case 14:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "mul_by_neg_one", 14) != 0)
      break;
    return 3;	 // "mul_by_neg_one"
  case 15:	 // 2 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'e':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "xtending_loads", 14) != 0)
        break;
      return 6;	 // "extending_loads"
    case 'i':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "dempotent_prop", 14) != 0)
        break;
      return 4;	 // "idempotent_prop"
    }
    break;
  case 17:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "add_p2i_to_ptradd", 17) != 0)
      break;
    return 2;	 // "add_p2i_to_ptradd"
  case 19:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "ptr_add_immed_chain", 19) != 0)
      break;
    return 5;	 // "ptr_add_immed_chain"
  case 28:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "opt_brcond_by_inverting_cond", 28) != 0)
      break;
    return 9;	 // "opt_brcond_by_inverting_cond"
  }
#endif // ifndef NDEBUG

  return std::nullopt;
}
static std::optional<std::pair<uint64_t, uint64_t>> getRuleRangeForIdentifier(StringRef RuleIdentifier) {
  std::pair<StringRef, StringRef> RangePair = RuleIdentifier.split('-');
  if (!RangePair.second.empty()) {
    const auto First = getRuleIdxForIdentifier(RangePair.first);
    const auto Last = getRuleIdxForIdentifier(RangePair.second);
    if (!First || !Last)
      return std::nullopt;
    if (First >= Last)
      report_fatal_error("Beginning of range should be before end of range");
    return {{*First, *Last + 1}};
  }
  if (RangePair.first == "*") {
    return {{0, 10}};
  }
  const auto I = getRuleIdxForIdentifier(RangePair.first);
  if (!I)
    return std::nullopt;
  return {{*I, *I + 1}};
}

bool AArch64GenO0PreLegalizerCombinerHelperRuleConfig::setRuleEnabled(StringRef RuleIdentifier) {
  auto MaybeRange = getRuleRangeForIdentifier(RuleIdentifier);
  if (!MaybeRange)
    return false;
  for (auto I = MaybeRange->first; I < MaybeRange->second; ++I)
    DisabledRules.reset(I);
  return true;
}

bool AArch64GenO0PreLegalizerCombinerHelperRuleConfig::setRuleDisabled(StringRef RuleIdentifier) {
  auto MaybeRange = getRuleRangeForIdentifier(RuleIdentifier);
  if (!MaybeRange)
    return false;
  for (auto I = MaybeRange->first; I < MaybeRange->second; ++I)
    DisabledRules.set(I);
  return true;
}

bool AArch64GenO0PreLegalizerCombinerHelperRuleConfig::isRuleDisabled(unsigned RuleID) const {
  return DisabledRules.test(RuleID);
}
#endif // ifdef AARCH64O0PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_H

#ifdef AARCH64O0PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_CPP

std::vector<std::string> AArch64O0PreLegalizerCombinerHelperOption;
cl::list<std::string> AArch64O0PreLegalizerCombinerHelperDisableOption(
    "aarch64o0prelegalizercombinerhelper-disable-rule",
    cl::desc("Disable one or more combiner rules temporarily in the AArch64O0PreLegalizerCombinerHelper pass"),
    cl::CommaSeparated,
    cl::Hidden,
    cl::cat(GICombinerOptionCategory),
    cl::callback([](const std::string &Str) {
      AArch64O0PreLegalizerCombinerHelperOption.push_back(Str);
    }));
cl::list<std::string> AArch64O0PreLegalizerCombinerHelperOnlyEnableOption(
    "aarch64o0prelegalizercombinerhelper-only-enable-rule",
    cl::desc("Disable all rules in the AArch64O0PreLegalizerCombinerHelper pass then re-enable the specified ones"),
    cl::Hidden,
    cl::cat(GICombinerOptionCategory),
    cl::callback([](const std::string &CommaSeparatedArg) {
      StringRef Str = CommaSeparatedArg;
      AArch64O0PreLegalizerCombinerHelperOption.push_back("*");
      do {
        auto X = Str.split(",");
        AArch64O0PreLegalizerCombinerHelperOption.push_back(("!" + X.first).str());
        Str = X.second;
      } while (!Str.empty());
    }));

bool AArch64GenO0PreLegalizerCombinerHelperRuleConfig::parseCommandLineOption() {
  for (StringRef Identifier : AArch64O0PreLegalizerCombinerHelperOption) {
    bool Enabled = Identifier.consume_front("!");
    if (Enabled && !setRuleEnabled(Identifier))
      return false;
    if (!Enabled && !setRuleDisabled(Identifier))
      return false;
  }
  return true;
}

bool AArch64GenO0PreLegalizerCombinerHelper::tryCombineAll(
    GISelChangeObserver &Observer,
    MachineInstr &MI,
    MachineIRBuilder &B) const {
  MachineBasicBlock *MBB = MI.getParent();
  MachineFunction *MF = MBB->getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  SmallVector<MachineInstr *, 8> MIs = {&MI};

  (void)MBB; (void)MF; (void)MRI; (void)RuleConfig;

  // Match data
  unsigned MatchData1;
  std::pair<Register, bool> MatchData2;
  PtrAddChain MatchData5;
  PreferredTuple MatchData6;
  std::function<void(MachineIRBuilder &)> MatchData7;
  SmallVector<Register, 4> MatchData8;
  MachineInstr * MatchData9;

  int Partition = -1;
  Partition = -1;
  switch (MIs[0]->getOpcode()) {
  case TargetOpcode::COPY: Partition = 0; break;
  case TargetOpcode::G_MUL: Partition = 1; break;
  case TargetOpcode::G_ADD: Partition = 2; break;
  case TargetOpcode::G_FREEZE: Partition = 3; break;
  case TargetOpcode::G_FABS: Partition = 4; break;
  case TargetOpcode::G_FCANONICALIZE: Partition = 5; break;
  case TargetOpcode::G_PTR_ADD: Partition = 6; break;
  case TargetOpcode::G_LOAD: Partition = 7; break;
  case TargetOpcode::G_SEXTLOAD: Partition = 8; break;
  case TargetOpcode::G_ZEXTLOAD: Partition = 9; break;
  case TargetOpcode::G_AND: Partition = 10; break;
  case TargetOpcode::G_XOR: Partition = 11; break;
  case TargetOpcode::G_BR: Partition = 12; break;
  }
  // Default case but without conflicting with potential default case in selection.
  if (Partition == -1) return false;
  if (Partition == 0 /* TargetOpcode::COPY */) {
    // Leaf name: copy_prop
    // Rule: copy_prop
    if (!RuleConfig->isRuleDisabled(0)) {
      if (1
          && [&]() {
           return Helper.matchCombineCopy(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'copy_prop'\n");
 Helper.applyCombineCopy(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 1 /* TargetOpcode::G_MUL */) {
    // Leaf name: mul_to_shl
    // Rule: mul_to_shl
    if (!RuleConfig->isRuleDisabled(1)) {
      if (1
          && [&]() {
           return Helper.matchCombineMulToShl(*MIs[0], MatchData1); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'mul_to_shl'\n");
 Helper.applyCombineMulToShl(*MIs[0], MatchData1); 
        return true;
      }
    }
    // Leaf name: mul_by_neg_one
    // Rule: mul_by_neg_one
    if (!RuleConfig->isRuleDisabled(3)) {
      if (1
          && [&]() {
           return Helper.matchConstantOp(MIs[0]->getOperand(2), -1); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'mul_by_neg_one'\n");
 Helper.applyCombineMulByNegativeOne(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 2 /* TargetOpcode::G_ADD */) {
    // Leaf name: add_p2i_to_ptradd
    // Rule: add_p2i_to_ptradd
    if (!RuleConfig->isRuleDisabled(2)) {
      if (1
          && [&]() {
           return Helper.matchCombineAddP2IToPtrAdd(*MIs[0], MatchData2); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'add_p2i_to_ptradd'\n");
 Helper.applyCombineAddP2IToPtrAdd(*MIs[0], MatchData2); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 3 /* TargetOpcode::G_FREEZE */) {
    // Leaf name: idempotent_prop
    // Rule: idempotent_prop
    if (!RuleConfig->isRuleDisabled(4)) {
      if (1
          && [&]() {
           return MRI.getVRegDef(MIs[0]->getOperand(1).getReg())->getOpcode() ==
                        MIs[0]->getOpcode(); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'idempotent_prop'\n");
 Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 4 /* TargetOpcode::G_FABS */) {
    // Leaf name: idempotent_prop
    // Rule: idempotent_prop
    if (!RuleConfig->isRuleDisabled(4)) {
      if (1
          && [&]() {
           return MRI.getVRegDef(MIs[0]->getOperand(1).getReg())->getOpcode() ==
                        MIs[0]->getOpcode(); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'idempotent_prop'\n");
 Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 5 /* TargetOpcode::G_FCANONICALIZE */) {
    // Leaf name: idempotent_prop
    // Rule: idempotent_prop
    if (!RuleConfig->isRuleDisabled(4)) {
      if (1
          && [&]() {
           return MRI.getVRegDef(MIs[0]->getOperand(1).getReg())->getOpcode() ==
                        MIs[0]->getOpcode(); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'idempotent_prop'\n");
 Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 6 /* TargetOpcode::G_PTR_ADD */) {
    // Leaf name: ptr_add_immed_chain
    // Rule: ptr_add_immed_chain
    if (!RuleConfig->isRuleDisabled(5)) {
      if (1
          && [&]() {
           return Helper.matchPtrAddImmedChain(*MIs[0], MatchData5); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'ptr_add_immed_chain'\n");
 Helper.applyPtrAddImmedChain(*MIs[0], MatchData5); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 7 /* TargetOpcode::G_LOAD */) {
    // Leaf name: extending_loads
    // Rule: extending_loads
    if (!RuleConfig->isRuleDisabled(6)) {
      if (1
          && [&]() {
           return Helper.matchCombineExtendingLoads(*MIs[0], MatchData6); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'extending_loads'\n");
 Helper.applyCombineExtendingLoads(*MIs[0], MatchData6); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 8 /* TargetOpcode::G_SEXTLOAD */) {
    // Leaf name: extending_loads
    // Rule: extending_loads
    if (!RuleConfig->isRuleDisabled(6)) {
      if (1
          && [&]() {
           return Helper.matchCombineExtendingLoads(*MIs[0], MatchData6); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'extending_loads'\n");
 Helper.applyCombineExtendingLoads(*MIs[0], MatchData6); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 9 /* TargetOpcode::G_ZEXTLOAD */) {
    // Leaf name: extending_loads
    // Rule: extending_loads
    if (!RuleConfig->isRuleDisabled(6)) {
      if (1
          && [&]() {
           return Helper.matchCombineExtendingLoads(*MIs[0], MatchData6); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'extending_loads'\n");
 Helper.applyCombineExtendingLoads(*MIs[0], MatchData6); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 10 /* TargetOpcode::G_AND */) {
    // Leaf name: load_and_mask
    // Rule: load_and_mask
    if (!RuleConfig->isRuleDisabled(7)) {
      if (1
          && [&]() {
           return Helper.matchCombineLoadWithAndMask(*MIs[0], MatchData7); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'load_and_mask'\n");
 Helper.applyBuildFn(*MIs[0], MatchData7); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 11 /* TargetOpcode::G_XOR */) {
    // Leaf name: not_cmp_fold
    // Rule: not_cmp_fold
    if (!RuleConfig->isRuleDisabled(8)) {
      if (1
          && [&]() {
           return Helper.matchNotCmp(*MIs[0], MatchData8); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'not_cmp_fold'\n");
 Helper.applyNotCmp(*MIs[0], MatchData8); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 12 /* TargetOpcode::G_BR */) {
    // Leaf name: opt_brcond_by_inverting_cond
    // Rule: opt_brcond_by_inverting_cond
    if (!RuleConfig->isRuleDisabled(9)) {
      if (1
          && [&]() {
           return Helper.matchOptBrCondByInvertingCond(*MIs[0], MatchData9); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'opt_brcond_by_inverting_cond'\n");
 Helper.applyOptBrCondByInvertingCond(*MIs[0], MatchData9); 
        return true;
      }
    }
    return false;
  }

  return false;
}
#endif // ifdef AARCH64O0PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_CPP
