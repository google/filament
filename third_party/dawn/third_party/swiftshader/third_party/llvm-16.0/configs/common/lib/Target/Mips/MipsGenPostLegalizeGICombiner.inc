/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Global Combiner                                                            *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef MIPSPOSTLEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_DEPS
#include "llvm/ADT/SparseBitVector.h"
namespace llvm {
extern cl::OptionCategory GICombinerOptionCategory;
} // end namespace llvm
#endif // ifdef MIPSPOSTLEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_DEPS

#ifdef MIPSPOSTLEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_H
class MipsGenPostLegalizerCombinerHelperRuleConfig {
  SparseBitVector<> DisabledRules;

public:
  bool parseCommandLineOption();
  bool isRuleDisabled(unsigned ID) const;
  bool setRuleEnabled(StringRef RuleIdentifier);
  bool setRuleDisabled(StringRef RuleIdentifier);
};

class MipsGenPostLegalizerCombinerHelper {
  const MipsGenPostLegalizerCombinerHelperRuleConfig *RuleConfig;

public:
  template <typename... Args>MipsGenPostLegalizerCombinerHelper(const MipsGenPostLegalizerCombinerHelperRuleConfig &RuleConfig, Args &&... args) : RuleConfig(&RuleConfig) {}

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
    return {{0, 0}};
  }
  const auto I = getRuleIdxForIdentifier(RangePair.first);
  if (!I)
    return std::nullopt;
  return {{*I, *I + 1}};
}

bool MipsGenPostLegalizerCombinerHelperRuleConfig::setRuleEnabled(StringRef RuleIdentifier) {
  auto MaybeRange = getRuleRangeForIdentifier(RuleIdentifier);
  if (!MaybeRange)
    return false;
  for (auto I = MaybeRange->first; I < MaybeRange->second; ++I)
    DisabledRules.reset(I);
  return true;
}

bool MipsGenPostLegalizerCombinerHelperRuleConfig::setRuleDisabled(StringRef RuleIdentifier) {
  auto MaybeRange = getRuleRangeForIdentifier(RuleIdentifier);
  if (!MaybeRange)
    return false;
  for (auto I = MaybeRange->first; I < MaybeRange->second; ++I)
    DisabledRules.set(I);
  return true;
}

bool MipsGenPostLegalizerCombinerHelperRuleConfig::isRuleDisabled(unsigned RuleID) const {
  return DisabledRules.test(RuleID);
}
#endif // ifdef MIPSPOSTLEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_H

#ifdef MIPSPOSTLEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_CPP

std::vector<std::string> MipsPostLegalizerCombinerHelperOption;
cl::list<std::string> MipsPostLegalizerCombinerHelperDisableOption(
    "mipspostlegalizercombinerhelper-disable-rule",
    cl::desc("Disable one or more combiner rules temporarily in the MipsPostLegalizerCombinerHelper pass"),
    cl::CommaSeparated,
    cl::Hidden,
    cl::cat(GICombinerOptionCategory),
    cl::callback([](const std::string &Str) {
      MipsPostLegalizerCombinerHelperOption.push_back(Str);
    }));
cl::list<std::string> MipsPostLegalizerCombinerHelperOnlyEnableOption(
    "mipspostlegalizercombinerhelper-only-enable-rule",
    cl::desc("Disable all rules in the MipsPostLegalizerCombinerHelper pass then re-enable the specified ones"),
    cl::Hidden,
    cl::cat(GICombinerOptionCategory),
    cl::callback([](const std::string &CommaSeparatedArg) {
      StringRef Str = CommaSeparatedArg;
      MipsPostLegalizerCombinerHelperOption.push_back("*");
      do {
        auto X = Str.split(",");
        MipsPostLegalizerCombinerHelperOption.push_back(("!" + X.first).str());
        Str = X.second;
      } while (!Str.empty());
    }));

bool MipsGenPostLegalizerCombinerHelperRuleConfig::parseCommandLineOption() {
  for (StringRef Identifier : MipsPostLegalizerCombinerHelperOption) {
    bool Enabled = Identifier.consume_front("!");
    if (Enabled && !setRuleEnabled(Identifier))
      return false;
    if (!Enabled && !setRuleDisabled(Identifier))
      return false;
  }
  return true;
}

bool MipsGenPostLegalizerCombinerHelper::tryCombineAll(
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

  int Partition = -1;
  Partition = 0;
  // Default case but without conflicting with potential default case in selection.
  if (Partition == -1) return false;
  if (Partition == 0 /* * or nullptr */) {
    return false;
  }

  return false;
}
#endif // ifdef MIPSPOSTLEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_CPP
