/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Global Combiner                                                            *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef AARCH64POSTLEGALIZERLOWERINGHELPER_GENCOMBINERHELPER_DEPS
#include "llvm/ADT/SparseBitVector.h"
namespace llvm {
extern cl::OptionCategory GICombinerOptionCategory;
} // end namespace llvm
#endif // ifdef AARCH64POSTLEGALIZERLOWERINGHELPER_GENCOMBINERHELPER_DEPS

#ifdef AARCH64POSTLEGALIZERLOWERINGHELPER_GENCOMBINERHELPER_H
class AArch64GenPostLegalizerLoweringHelperRuleConfig {
  SparseBitVector<> DisabledRules;

public:
  bool parseCommandLineOption();
  bool isRuleDisabled(unsigned ID) const;
  bool setRuleEnabled(StringRef RuleIdentifier);
  bool setRuleDisabled(StringRef RuleIdentifier);
};

class AArch64GenPostLegalizerLoweringHelper {
  const AArch64GenPostLegalizerLoweringHelperRuleConfig *RuleConfig;

public:
  template <typename... Args>AArch64GenPostLegalizerLoweringHelper(const AArch64GenPostLegalizerLoweringHelperRuleConfig &RuleConfig, Args &&... args) : RuleConfig(&RuleConfig) {}

  bool tryCombineAll(
    GISelChangeObserver &Observer,
    MachineInstr &MI,
    MachineIRBuilder &B,
    CombinerHelper & Helper) const;
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
  case 3:	 // 6 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'd':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "up", 2) != 0)
        break;
      return 0;	 // "dup"
    case 'e':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "xt", 2) != 0)
        break;
      return 2;	 // "ext"
    case 'r':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ev", 2) != 0)
        break;
      return 1;	 // "rev"
    case 't':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "rn", 2) != 0)
        break;
      return 5;	 // "trn"
    case 'u':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "zp", 2) != 0)
        break;
      return 4;	 // "uzp"
    case 'z':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ip", 2) != 0)
        break;
      return 3;	 // "zip"
    }
    break;
  case 11:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "shuf_to_ins", 11) != 0)
      break;
    return 7;	 // "shuf_to_ins"
  case 12:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "form_duplane", 12) != 0)
      break;
    return 6;	 // "form_duplane"
  case 15:	 // 3 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "djust_icmp_imm", 14) != 0)
        break;
      return 9;	 // "adjust_icmp_imm"
    case 'f':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "orm_truncstore", 14) != 0)
        break;
      return 13;	 // "form_truncstore"
    case 'v':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ashr_vlshr_imm", 14) != 0)
        break;
      return 8;	 // "vashr_vlshr_imm"
    }
    break;
  case 17:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "lower_vector_fcmp", 17) != 0)
      break;
    return 12;	 // "lower_vector_fcmp"
  case 18:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "swap_icmp_operands", 18) != 0)
      break;
    return 10;	 // "swap_icmp_operands"
  case 19:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "build_vector_to_dup", 19) != 0)
      break;
    return 11;	 // "build_vector_to_dup"
  case 26:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "vector_sext_inreg_to_shift", 26) != 0)
      break;
    return 14;	 // "vector_sext_inreg_to_shift"
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
    return {{0, 15}};
  }
  const auto I = getRuleIdxForIdentifier(RangePair.first);
  if (!I)
    return std::nullopt;
  return {{*I, *I + 1}};
}

bool AArch64GenPostLegalizerLoweringHelperRuleConfig::setRuleEnabled(StringRef RuleIdentifier) {
  auto MaybeRange = getRuleRangeForIdentifier(RuleIdentifier);
  if (!MaybeRange)
    return false;
  for (auto I = MaybeRange->first; I < MaybeRange->second; ++I)
    DisabledRules.reset(I);
  return true;
}

bool AArch64GenPostLegalizerLoweringHelperRuleConfig::setRuleDisabled(StringRef RuleIdentifier) {
  auto MaybeRange = getRuleRangeForIdentifier(RuleIdentifier);
  if (!MaybeRange)
    return false;
  for (auto I = MaybeRange->first; I < MaybeRange->second; ++I)
    DisabledRules.set(I);
  return true;
}

bool AArch64GenPostLegalizerLoweringHelperRuleConfig::isRuleDisabled(unsigned RuleID) const {
  return DisabledRules.test(RuleID);
}
#endif // ifdef AARCH64POSTLEGALIZERLOWERINGHELPER_GENCOMBINERHELPER_H

#ifdef AARCH64POSTLEGALIZERLOWERINGHELPER_GENCOMBINERHELPER_CPP

std::vector<std::string> AArch64PostLegalizerLoweringHelperOption;
cl::list<std::string> AArch64PostLegalizerLoweringHelperDisableOption(
    "aarch64postlegalizerloweringhelper-disable-rule",
    cl::desc("Disable one or more combiner rules temporarily in the AArch64PostLegalizerLoweringHelper pass"),
    cl::CommaSeparated,
    cl::Hidden,
    cl::cat(GICombinerOptionCategory),
    cl::callback([](const std::string &Str) {
      AArch64PostLegalizerLoweringHelperOption.push_back(Str);
    }));
cl::list<std::string> AArch64PostLegalizerLoweringHelperOnlyEnableOption(
    "aarch64postlegalizerloweringhelper-only-enable-rule",
    cl::desc("Disable all rules in the AArch64PostLegalizerLoweringHelper pass then re-enable the specified ones"),
    cl::Hidden,
    cl::cat(GICombinerOptionCategory),
    cl::callback([](const std::string &CommaSeparatedArg) {
      StringRef Str = CommaSeparatedArg;
      AArch64PostLegalizerLoweringHelperOption.push_back("*");
      do {
        auto X = Str.split(",");
        AArch64PostLegalizerLoweringHelperOption.push_back(("!" + X.first).str());
        Str = X.second;
      } while (!Str.empty());
    }));

bool AArch64GenPostLegalizerLoweringHelperRuleConfig::parseCommandLineOption() {
  for (StringRef Identifier : AArch64PostLegalizerLoweringHelperOption) {
    bool Enabled = Identifier.consume_front("!");
    if (Enabled && !setRuleEnabled(Identifier))
      return false;
    if (!Enabled && !setRuleDisabled(Identifier))
      return false;
  }
  return true;
}

bool AArch64GenPostLegalizerLoweringHelper::tryCombineAll(
    GISelChangeObserver &Observer,
    MachineInstr &MI,
    MachineIRBuilder &B,
    CombinerHelper & Helper) const {
  MachineBasicBlock *MBB = MI.getParent();
  MachineFunction *MF = MBB->getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  SmallVector<MachineInstr *, 8> MIs = {&MI};

  (void)MBB; (void)MF; (void)MRI; (void)RuleConfig;

  // Match data
  ShuffleVectorPseudo MatchData0;
  ShuffleVectorPseudo MatchData1;
  ShuffleVectorPseudo MatchData2;
  ShuffleVectorPseudo MatchData3;
  ShuffleVectorPseudo MatchData4;
  ShuffleVectorPseudo MatchData5;
  std::pair<unsigned, int> MatchData6;
  std::tuple<Register, int, Register, int> MatchData7;
  int64_t MatchData8;
  std::pair<uint64_t, CmpInst::Predicate> MatchData9;
  Register MatchData13;

  int Partition = -1;
  Partition = -1;
  switch (MIs[0]->getOpcode()) {
  case TargetOpcode::G_SHUFFLE_VECTOR: Partition = 0; break;
  case TargetOpcode::G_ASHR: Partition = 1; break;
  case TargetOpcode::G_LSHR: Partition = 2; break;
  case TargetOpcode::G_ICMP: Partition = 3; break;
  case TargetOpcode::G_BUILD_VECTOR: Partition = 4; break;
  case TargetOpcode::G_FCMP: Partition = 5; break;
  case TargetOpcode::G_STORE: Partition = 6; break;
  case TargetOpcode::G_SEXT_INREG: Partition = 7; break;
  }
  // Default case but without conflicting with potential default case in selection.
  if (Partition == -1) return false;
  if (Partition == 0 /* TargetOpcode::G_SHUFFLE_VECTOR */) {
    // Leaf name: dup
    // Rule: dup
    if (!RuleConfig->isRuleDisabled(0)) {
      if (1
          && [&]() {
           return matchDup(*MIs[0], MRI, MatchData0); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'dup'\n");
 applyShuffleVectorPseudo(*MIs[0], MatchData0); 
        return true;
      }
    }
    // Leaf name: rev
    // Rule: rev
    if (!RuleConfig->isRuleDisabled(1)) {
      if (1
          && [&]() {
           return matchREV(*MIs[0], MRI, MatchData1); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'rev'\n");
 applyShuffleVectorPseudo(*MIs[0], MatchData1); 
        return true;
      }
    }
    // Leaf name: ext
    // Rule: ext
    if (!RuleConfig->isRuleDisabled(2)) {
      if (1
          && [&]() {
           return matchEXT(*MIs[0], MRI, MatchData2); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'ext'\n");
 applyEXT(*MIs[0], MatchData2); 
        return true;
      }
    }
    // Leaf name: zip
    // Rule: zip
    if (!RuleConfig->isRuleDisabled(3)) {
      if (1
          && [&]() {
           return matchZip(*MIs[0], MRI, MatchData3); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'zip'\n");
 applyShuffleVectorPseudo(*MIs[0], MatchData3); 
        return true;
      }
    }
    // Leaf name: uzp
    // Rule: uzp
    if (!RuleConfig->isRuleDisabled(4)) {
      if (1
          && [&]() {
           return matchUZP(*MIs[0], MRI, MatchData4); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'uzp'\n");
 applyShuffleVectorPseudo(*MIs[0], MatchData4); 
        return true;
      }
    }
    // Leaf name: trn
    // Rule: trn
    if (!RuleConfig->isRuleDisabled(5)) {
      if (1
          && [&]() {
           return matchTRN(*MIs[0], MRI, MatchData5); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'trn'\n");
 applyShuffleVectorPseudo(*MIs[0], MatchData5); 
        return true;
      }
    }
    // Leaf name: form_duplane
    // Rule: form_duplane
    if (!RuleConfig->isRuleDisabled(6)) {
      if (1
          && [&]() {
           return matchDupLane(*MIs[0], MRI, MatchData6); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'form_duplane'\n");
 applyDupLane(*MIs[0], MRI, B, MatchData6); 
        return true;
      }
    }
    // Leaf name: shuf_to_ins
    // Rule: shuf_to_ins
    if (!RuleConfig->isRuleDisabled(7)) {
      if (1
          && [&]() {
           return matchINS(*MIs[0], MRI, MatchData7); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'shuf_to_ins'\n");
 return applyINS(*MIs[0], MRI, B, MatchData7); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 1 /* TargetOpcode::G_ASHR */) {
    // Leaf name: vashr_vlshr_imm
    // Rule: vashr_vlshr_imm
    if (!RuleConfig->isRuleDisabled(8)) {
      if (1
          && [&]() {
           return matchVAshrLshrImm(*MIs[0], MRI, MatchData8); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'vashr_vlshr_imm'\n");
 applyVAshrLshrImm(*MIs[0], MRI, MatchData8); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 2 /* TargetOpcode::G_LSHR */) {
    // Leaf name: vashr_vlshr_imm
    // Rule: vashr_vlshr_imm
    if (!RuleConfig->isRuleDisabled(8)) {
      if (1
          && [&]() {
           return matchVAshrLshrImm(*MIs[0], MRI, MatchData8); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'vashr_vlshr_imm'\n");
 applyVAshrLshrImm(*MIs[0], MRI, MatchData8); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 3 /* TargetOpcode::G_ICMP */) {
    // Leaf name: adjust_icmp_imm
    // Rule: adjust_icmp_imm
    if (!RuleConfig->isRuleDisabled(9)) {
      if (1
          && [&]() {
           return matchAdjustICmpImmAndPred(*MIs[0], MRI, MatchData9); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'adjust_icmp_imm'\n");
 applyAdjustICmpImmAndPred(*MIs[0], MatchData9, B, Observer); 
        return true;
      }
    }
    // Leaf name: swap_icmp_operands
    // Rule: swap_icmp_operands
    if (!RuleConfig->isRuleDisabled(10)) {
      if (1
          && [&]() {
           return trySwapICmpOperands(*MIs[0], MRI); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'swap_icmp_operands'\n");
 applySwapICmpOperands(*MIs[0], Observer); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 4 /* TargetOpcode::G_BUILD_VECTOR */) {
    // Leaf name: build_vector_to_dup
    // Rule: build_vector_to_dup
    if (!RuleConfig->isRuleDisabled(11)) {
      if (1
          && [&]() {
           return matchBuildVectorToDup(*MIs[0], MRI); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'build_vector_to_dup'\n");
 return applyBuildVectorToDup(*MIs[0], MRI, B); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 5 /* TargetOpcode::G_FCMP */) {
    // Leaf name: lower_vector_fcmp
    // Rule: lower_vector_fcmp
    if (!RuleConfig->isRuleDisabled(12)) {
      if (1
          && [&]() {
           return lowerVectorFCMP(*MIs[0], MRI, B); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'lower_vector_fcmp'\n");

        return true;
      }
    }
    return false;
  }
  if (Partition == 6 /* TargetOpcode::G_STORE */) {
    // Leaf name: form_truncstore
    // Rule: form_truncstore
    if (!RuleConfig->isRuleDisabled(13)) {
      if (1
          && [&]() {
           return matchFormTruncstore(*MIs[0], MRI, MatchData13); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'form_truncstore'\n");
 applyFormTruncstore(*MIs[0], MRI, B, Observer, MatchData13); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 7 /* TargetOpcode::G_SEXT_INREG */) {
    // Leaf name: vector_sext_inreg_to_shift
    // Rule: vector_sext_inreg_to_shift
    if (!RuleConfig->isRuleDisabled(14)) {
      if (1
          && [&]() {
           return matchVectorSextInReg(*MIs[0], MRI); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'vector_sext_inreg_to_shift'\n");
 applyVectorSextInReg(*MIs[0], MRI, B, Observer); 
        return true;
      }
    }
    return false;
  }

  return false;
}
#endif // ifdef AARCH64POSTLEGALIZERLOWERINGHELPER_GENCOMBINERHELPER_CPP
