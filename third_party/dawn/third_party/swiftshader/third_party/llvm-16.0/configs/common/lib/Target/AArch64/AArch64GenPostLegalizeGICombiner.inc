/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Global Combiner                                                            *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef AARCH64POSTLEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_DEPS
#include "llvm/ADT/SparseBitVector.h"
namespace llvm {
extern cl::OptionCategory GICombinerOptionCategory;
} // end namespace llvm
#endif // ifdef AARCH64POSTLEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_DEPS

#ifdef AARCH64POSTLEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_H
class AArch64GenPostLegalizerCombinerHelperRuleConfig {
  SparseBitVector<> DisabledRules;

public:
  bool parseCommandLineOption();
  bool isRuleDisabled(unsigned ID) const;
  bool setRuleEnabled(StringRef RuleIdentifier);
  bool setRuleDisabled(StringRef RuleIdentifier);
};

class AArch64GenPostLegalizerCombinerHelper {
  const AArch64GenPostLegalizerCombinerHelperRuleConfig *RuleConfig;

public:
  template <typename... Args>AArch64GenPostLegalizerCombinerHelper(const AArch64GenPostLegalizerCombinerHelperRuleConfig &RuleConfig, Args &&... args) : RuleConfig(&RuleConfig) {}

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
  case 9:	 // 2 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'c':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "opy_prop", 8) != 0)
        break;
      return 0;	 // "copy_prop"
    case 'm':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ul_const", 8) != 0)
        break;
      return 10;	 // "mul_const"
    }
    break;
  case 10:	 // 2 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'i':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "2p_to_p2i", 9) != 0)
        break;
      return 30;	 // "i2p_to_p2i"
    case 'p':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "2i_to_i2p", 9) != 0)
        break;
      return 29;	 // "p2i_to_i2p"
    }
    break;
  case 11:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "add_sub_reg", 11) != 0)
      break;
    return 34;	 // "add_sub_reg"
  case 12:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "redundant_or", 12) != 0)
      break;
    return 9;	 // "redundant_or"
  case 13:	 // 5 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'c':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "onstant_fold", 12) != 0)
        break;
      return 23;	 // "constant_fold"
    case 'l':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "oad_and_mask", 12) != 0)
        break;
      return 2;	 // "load_and_mask"
    case 'm':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "erge_unmerge", 12) != 0)
        break;
      return 18;	 // "merge_unmerge"
    case 'r':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "edundant_and", 12) != 0)
        break;
      return 6;	 // "redundant_and"
    case 'u':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "nmerge_undef", 12) != 0)
        break;
      return 52;	 // "unmerge_undef"
    }
    break;
  case 14:	 // 2 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'b':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "inop_same_val", 13) != 0)
        break;
      return 26;	 // "binop_same_val"
    case 'f':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "neg_fneg_fold", 13) != 0)
        break;
      return 32;	 // "fneg_fneg_fold"
    }
    break;
  case 15:	 // 3 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'e':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "xtending_loads", 14) != 0)
        break;
      return 1;	 // "extending_loads"
    case 'o':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "verlapping_and", 14) != 0)
        break;
      return 40;	 // "overlapping_and"
    case 's':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "elect_same_val", 14) != 0)
        break;
      return 24;	 // "select_same_val"
    }
    break;
  case 16:	 // 3 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 's':	 // 2 strings to match.
      if (memcmp(RuleIdentifier.data()+1, "elect_", 6) != 0)
        break;
      switch (RuleIdentifier[7]) {
      default: break;
      case 't':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+8, "o_minmax", 8) != 0)
          break;
        return 54;	 // "select_to_minmax"
      case 'u':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+8, "ndef_cmp", 8) != 0)
          break;
        return 19;	 // "select_undef_cmp"
      }
      break;
    case 'u':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ndef_to_fp_zero", 15) != 0)
        break;
      return 42;	 // "undef_to_fp_zero"
    }
    break;
  case 17:	 // 4 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "nyext_trunc_fold", 16) != 0)
        break;
      return 31;	 // "anyext_trunc_fold"
    case 'e':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "rase_undef_store", 16) != 0)
        break;
      return 51;	 // "erase_undef_store"
    case 's':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "elect_to_logical", 16) != 0)
        break;
      return 21;	 // "select_to_logical"
    case 'u':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ndef_to_int_zero", 16) != 0)
        break;
      return 43;	 // "undef_to_int_zero"
    }
    break;
  case 18:	 // 3 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'b':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "inop_left_to_zero", 17) != 0)
        break;
      return 27;	 // "binop_left_to_zero"
    case 'f':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "old_merge_to_zext", 17) != 0)
        break;
      return 22;	 // "fold_merge_to_zext"
    case 'r':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ight_identity_one", 17) != 0)
        break;
      return 33;	 // "right_identity_one"
    }
    break;
  case 19:	 // 7 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'b':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "inop_right_to_zero", 18) != 0)
        break;
      return 28;	 // "binop_right_to_zero"
    case 'p':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "tr_add_immed_chain", 18) != 0)
        break;
      return 39;	 // "ptr_add_immed_chain"
    case 'r':	 // 2 strings to match.
      switch (RuleIdentifier[1]) {
      default: break;
      case 'i':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+2, "ght_identity_zero", 17) != 0)
          break;
        return 25;	 // "right_identity_zero"
      case 'o':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+2, "tate_out_of_range", 17) != 0)
          break;
        return 16;	 // "rotate_out_of_range"
      }
      break;
    case 's':	 // 2 strings to match.
      if (RuleIdentifier[1] != 'e')
        break;
      switch (RuleIdentifier[2]) {
      default: break;
      case 'l':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+3, "ect_constant_cmp", 16) != 0)
          break;
        return 20;	 // "select_constant_cmp"
      case 'x':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+3, "t_trunc_sextload", 16) != 0)
          break;
        return 3;	 // "sext_trunc_sextload"
      }
      break;
    case 'u':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "nary_undef_to_zero", 18) != 0)
        break;
      return 47;	 // "unary_undef_to_zero"
    }
    break;
  case 20:	 // 3 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'b':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "itcast_bitcast_fold", 19) != 0)
        break;
      return 38;	 // "bitcast_bitcast_fold"
    case 'r':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "edundant_sext_inreg", 19) != 0)
        break;
      return 11;	 // "redundant_sext_inreg"
    case 's':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "plit_store_zero_128", 19) != 0)
        break;
      return 41;	 // "split_store_zero_128"
    }
    break;
  case 21:	 // 2 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'm':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "utate_anyext_to_zext", 20) != 0)
        break;
      return 4;	 // "mutate_anyext_to_zext"
    case 'u':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ndef_to_negative_one", 20) != 0)
        break;
      return 44;	 // "undef_to_negative_one"
    }
    break;
  case 22:	 // 2 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'p':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ropagate_undef_any_op", 21) != 0)
        break;
      return 48;	 // "propagate_undef_any_op"
    case 't':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "runc_buildvector_fold", 21) != 0)
        break;
      return 36;	 // "trunc_buildvector_fold"
    }
    break;
  case 23:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "propagate_undef_all_ops", 23) != 0)
      break;
    return 49;	 // "propagate_undef_all_ops"
  case 24:	 // 2 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'b':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "inop_left_undef_to_zero", 23) != 0)
        break;
      return 45;	 // "binop_left_undef_to_zero"
    case 'x':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "or_of_and_with_same_reg", 23) != 0)
        break;
      return 7;	 // "xor_of_and_with_same_reg"
    }
    break;
  case 25:	 // 3 strings to match.
    if (RuleIdentifier[0] != 'b')
      break;
    switch (RuleIdentifier[1]) {
    default: break;
    case 'i':	 // 2 strings to match.
      if (memcmp(RuleIdentifier.data()+2, "tfield_extract_from_", 20) != 0)
        break;
      switch (RuleIdentifier[22]) {
      default: break;
      case 'a':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+23, "nd", 2) != 0)
          break;
        return 13;	 // "bitfield_extract_from_and"
      case 's':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+23, "hr", 2) != 0)
          break;
        return 14;	 // "bitfield_extract_from_shr"
      }
      break;
    case 'u':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+2, "ildvector_identity_fold", 23) != 0)
        break;
      return 35;	 // "buildvector_identity_fold"
    }
    break;
  case 26:	 // 2 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'b':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "inop_right_undef_to_undef", 25) != 0)
        break;
      return 46;	 // "binop_right_undef_to_undef"
    case 'e':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "xtractvecelt_pairwise_add", 25) != 0)
        break;
      return 8;	 // "extractvecelt_pairwise_add"
    }
    break;
  case 27:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "trunc_lshr_buildvector_fold", 27) != 0)
      break;
    return 37;	 // "trunc_lshr_buildvector_fold"
  case 28:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "propagate_undef_shuffle_mask", 28) != 0)
      break;
    return 50;	 // "propagate_undef_shuffle_mask"
  case 29:	 // 2 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'b':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "itfield_extract_from_shr_and", 28) != 0)
        break;
      return 15;	 // "bitfield_extract_from_shr_and"
    case 'i':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "cmp_to_true_false_known_bits", 28) != 0)
        break;
      return 17;	 // "icmp_to_true_false_known_bits"
    }
    break;
  case 32:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "bitfield_extract_from_sext_inreg", 32) != 0)
      break;
    return 12;	 // "bitfield_extract_from_sext_inreg"
  case 36:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "insert_extract_vec_elt_out_of_bounds", 36) != 0)
      break;
    return 53;	 // "insert_extract_vec_elt_out_of_bounds"
  case 37:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "hoist_logic_op_with_same_opcode_hands", 37) != 0)
      break;
    return 5;	 // "hoist_logic_op_with_same_opcode_hands"
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
    return {{0, 55}};
  }
  const auto I = getRuleIdxForIdentifier(RangePair.first);
  if (!I)
    return std::nullopt;
  return {{*I, *I + 1}};
}

bool AArch64GenPostLegalizerCombinerHelperRuleConfig::setRuleEnabled(StringRef RuleIdentifier) {
  auto MaybeRange = getRuleRangeForIdentifier(RuleIdentifier);
  if (!MaybeRange)
    return false;
  for (auto I = MaybeRange->first; I < MaybeRange->second; ++I)
    DisabledRules.reset(I);
  return true;
}

bool AArch64GenPostLegalizerCombinerHelperRuleConfig::setRuleDisabled(StringRef RuleIdentifier) {
  auto MaybeRange = getRuleRangeForIdentifier(RuleIdentifier);
  if (!MaybeRange)
    return false;
  for (auto I = MaybeRange->first; I < MaybeRange->second; ++I)
    DisabledRules.set(I);
  return true;
}

bool AArch64GenPostLegalizerCombinerHelperRuleConfig::isRuleDisabled(unsigned RuleID) const {
  return DisabledRules.test(RuleID);
}
#endif // ifdef AARCH64POSTLEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_H

#ifdef AARCH64POSTLEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_CPP

std::vector<std::string> AArch64PostLegalizerCombinerHelperOption;
cl::list<std::string> AArch64PostLegalizerCombinerHelperDisableOption(
    "aarch64postlegalizercombinerhelper-disable-rule",
    cl::desc("Disable one or more combiner rules temporarily in the AArch64PostLegalizerCombinerHelper pass"),
    cl::CommaSeparated,
    cl::Hidden,
    cl::cat(GICombinerOptionCategory),
    cl::callback([](const std::string &Str) {
      AArch64PostLegalizerCombinerHelperOption.push_back(Str);
    }));
cl::list<std::string> AArch64PostLegalizerCombinerHelperOnlyEnableOption(
    "aarch64postlegalizercombinerhelper-only-enable-rule",
    cl::desc("Disable all rules in the AArch64PostLegalizerCombinerHelper pass then re-enable the specified ones"),
    cl::Hidden,
    cl::cat(GICombinerOptionCategory),
    cl::callback([](const std::string &CommaSeparatedArg) {
      StringRef Str = CommaSeparatedArg;
      AArch64PostLegalizerCombinerHelperOption.push_back("*");
      do {
        auto X = Str.split(",");
        AArch64PostLegalizerCombinerHelperOption.push_back(("!" + X.first).str());
        Str = X.second;
      } while (!Str.empty());
    }));

bool AArch64GenPostLegalizerCombinerHelperRuleConfig::parseCommandLineOption() {
  for (StringRef Identifier : AArch64PostLegalizerCombinerHelperOption) {
    bool Enabled = Identifier.consume_front("!");
    if (Enabled && !setRuleEnabled(Identifier))
      return false;
    if (!Enabled && !setRuleDisabled(Identifier))
      return false;
  }
  return true;
}

bool AArch64GenPostLegalizerCombinerHelper::tryCombineAll(
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
  PreferredTuple MatchData1;
  std::function<void(MachineIRBuilder &)> MatchData2;
  InstructionStepsMatchInfo MatchData5;
  Register MatchData6;
  std::pair<Register, Register> MatchData7;
  std::tuple<unsigned, LLT, Register> MatchData8;
  Register MatchData9;
  std::function<void(MachineIRBuilder&, Register)> MatchData10;
  std::function<void(MachineIRBuilder &)> MatchData12;
  std::function<void(MachineIRBuilder &)> MatchData13;
  std::function<void(MachineIRBuilder &)> MatchData14;
  std::function<void(MachineIRBuilder &)> MatchData15;
  int64_t MatchData17;
  Register MatchData18;
  unsigned MatchData20;
  std::function<void(MachineIRBuilder &)> MatchData21;
  APInt MatchData23;
  Register MatchData29;
  Register MatchData30;
  Register MatchData31;
  Register MatchData32;
  Register MatchData34;
  Register MatchData35;
  Register MatchData36;
  Register MatchData37;
  PtrAddChain MatchData39;
  std::function<void(MachineIRBuilder &)> MatchData40;
  std::function<void(MachineIRBuilder &)> MatchData52;
  std::function<void(MachineIRBuilder &)> MatchData54;

  int Partition = -1;
  Partition = -1;
  switch (MIs[0]->getOpcode()) {
  case TargetOpcode::COPY: Partition = 0; break;
  case TargetOpcode::G_LOAD: Partition = 1; break;
  case TargetOpcode::G_SEXTLOAD: Partition = 2; break;
  case TargetOpcode::G_ZEXTLOAD: Partition = 3; break;
  case TargetOpcode::G_AND: Partition = 4; break;
  case TargetOpcode::G_SEXT_INREG: Partition = 5; break;
  case TargetOpcode::G_ANYEXT: Partition = 6; break;
  case TargetOpcode::G_OR: Partition = 7; break;
  case TargetOpcode::G_XOR: Partition = 8; break;
  case TargetOpcode::G_EXTRACT_VECTOR_ELT: Partition = 9; break;
  case TargetOpcode::G_MUL: Partition = 10; break;
  case TargetOpcode::G_ASHR: Partition = 11; break;
  case TargetOpcode::G_LSHR: Partition = 12; break;
  case TargetOpcode::G_ROTR: Partition = 13; break;
  case TargetOpcode::G_ROTL: Partition = 14; break;
  case TargetOpcode::G_ICMP: Partition = 15; break;
  case TargetOpcode::G_MERGE_VALUES: Partition = 16; break;
  case TargetOpcode::G_SELECT: Partition = 17; break;
  case TargetOpcode::G_ADD: Partition = 18; break;
  case TargetOpcode::G_SUB: Partition = 19; break;
  case TargetOpcode::G_SHL: Partition = 20; break;
  case TargetOpcode::G_PTR_ADD: Partition = 21; break;
  case TargetOpcode::G_SDIV: Partition = 22; break;
  case TargetOpcode::G_UDIV: Partition = 23; break;
  case TargetOpcode::G_SREM: Partition = 24; break;
  case TargetOpcode::G_UREM: Partition = 25; break;
  case TargetOpcode::G_INTTOPTR: Partition = 26; break;
  case TargetOpcode::G_PTRTOINT: Partition = 27; break;
  case TargetOpcode::G_FNEG: Partition = 28; break;
  case TargetOpcode::G_BUILD_VECTOR_TRUNC: Partition = 29; break;
  case TargetOpcode::G_BUILD_VECTOR: Partition = 30; break;
  case TargetOpcode::G_TRUNC: Partition = 31; break;
  case TargetOpcode::G_BITCAST: Partition = 32; break;
  case TargetOpcode::G_STORE: Partition = 33; break;
  case TargetOpcode::G_UITOFP: Partition = 34; break;
  case TargetOpcode::G_SITOFP: Partition = 35; break;
  case TargetOpcode::G_ABS: Partition = 36; break;
  case TargetOpcode::G_FPTOSI: Partition = 37; break;
  case TargetOpcode::G_FPTOUI: Partition = 38; break;
  case TargetOpcode::G_SHUFFLE_VECTOR: Partition = 39; break;
  case TargetOpcode::G_UNMERGE_VALUES: Partition = 40; break;
  case TargetOpcode::G_INSERT_VECTOR_ELT: Partition = 41; break;
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
  if (Partition == 1 /* TargetOpcode::G_LOAD */) {
    // Leaf name: extending_loads
    // Rule: extending_loads
    if (!RuleConfig->isRuleDisabled(1)) {
      if (1
          && [&]() {
           return Helper.matchCombineExtendingLoads(*MIs[0], MatchData1); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'extending_loads'\n");
 Helper.applyCombineExtendingLoads(*MIs[0], MatchData1); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 2 /* TargetOpcode::G_SEXTLOAD */) {
    // Leaf name: extending_loads
    // Rule: extending_loads
    if (!RuleConfig->isRuleDisabled(1)) {
      if (1
          && [&]() {
           return Helper.matchCombineExtendingLoads(*MIs[0], MatchData1); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'extending_loads'\n");
 Helper.applyCombineExtendingLoads(*MIs[0], MatchData1); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 3 /* TargetOpcode::G_ZEXTLOAD */) {
    // Leaf name: extending_loads
    // Rule: extending_loads
    if (!RuleConfig->isRuleDisabled(1)) {
      if (1
          && [&]() {
           return Helper.matchCombineExtendingLoads(*MIs[0], MatchData1); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'extending_loads'\n");
 Helper.applyCombineExtendingLoads(*MIs[0], MatchData1); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 4 /* TargetOpcode::G_AND */) {
    // Leaf name: load_and_mask
    // Rule: load_and_mask
    if (!RuleConfig->isRuleDisabled(2)) {
      if (1
          && [&]() {
           return Helper.matchCombineLoadWithAndMask(*MIs[0], MatchData2); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'load_and_mask'\n");
 Helper.applyBuildFn(*MIs[0], MatchData2); 
        return true;
      }
    }
    // Leaf name: hoist_logic_op_with_same_opcode_hands
    // Rule: hoist_logic_op_with_same_opcode_hands
    if (!RuleConfig->isRuleDisabled(5)) {
      if (1
          && [&]() {
           return Helper.matchHoistLogicOpWithSameOpcodeHands(*MIs[0], MatchData5); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'hoist_logic_op_with_same_opcode_hands'\n");
 Helper.applyBuildInstructionSteps(*MIs[0], MatchData5);
        return true;
      }
    }
    // Leaf name: redundant_and
    // Rule: redundant_and
    if (!RuleConfig->isRuleDisabled(6)) {
      if (1
          && [&]() {
           return Helper.matchRedundantAnd(*MIs[0], MatchData6); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'redundant_and'\n");
 return Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData6); 
        return true;
      }
    }
    // Leaf name: bitfield_extract_from_and
    // Rule: bitfield_extract_from_and
    if (!RuleConfig->isRuleDisabled(13)) {
      if (1
          && [&]() {
           return Helper.matchBitfieldExtractFromAnd(*MIs[0], MatchData13); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'bitfield_extract_from_and'\n");
 Helper.applyBuildFn(*MIs[0], MatchData13); 
        return true;
      }
    }
    // Leaf name: constant_fold
    // Rule: constant_fold
    if (!RuleConfig->isRuleDisabled(23)) {
      if (1
          && [&]() {
           return Helper.matchConstantFold(*MIs[0], MatchData23); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'constant_fold'\n");
 Helper.replaceInstWithConstant(*MIs[0], MatchData23); 
        return true;
      }
    }
    // Leaf name: binop_same_val
    // Rule: binop_same_val
    if (!RuleConfig->isRuleDisabled(26)) {
      if (1
          && [&]() {
           return Helper.matchBinOpSameVal(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'binop_same_val'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    // Leaf name: overlapping_and
    // Rule: overlapping_and
    if (!RuleConfig->isRuleDisabled(40)) {
      if (1
          && [&]() {
           return Helper.matchOverlappingAnd(*MIs[0], MatchData40); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'overlapping_and'\n");
 Helper.applyBuildFn(*MIs[0], MatchData40); 
        return true;
      }
    }
    // Leaf name: undef_to_int_zero
    // Rule: undef_to_int_zero
    if (!RuleConfig->isRuleDisabled(43)) {
      if (1
          && [&]() {
           return Helper.matchAnyExplicitUseIsUndef(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'undef_to_int_zero'\n");
 Helper.replaceInstWithConstant(*MIs[0], 0); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 5 /* TargetOpcode::G_SEXT_INREG */) {
    // Leaf name: sext_trunc_sextload
    // Rule: sext_trunc_sextload
    if (!RuleConfig->isRuleDisabled(3)) {
      if (1
          && [&]() {
           return Helper.matchSextTruncSextLoad(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'sext_trunc_sextload'\n");
 Helper.applySextTruncSextLoad(*MIs[0]); 
        return true;
      }
    }
    // Leaf name: redundant_sext_inreg
    // Rule: redundant_sext_inreg
    if (!RuleConfig->isRuleDisabled(11)) {
      if (1
          && [&]() {
           return Helper.matchRedundantSExtInReg(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'redundant_sext_inreg'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    // Leaf name: bitfield_extract_from_sext_inreg
    // Rule: bitfield_extract_from_sext_inreg
    if (!RuleConfig->isRuleDisabled(12)) {
      if (1
          && [&]() {
           return Helper.matchBitfieldExtractFromSExtInReg(*MIs[0], MatchData12); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'bitfield_extract_from_sext_inreg'\n");
 Helper.applyBuildFn(*MIs[0], MatchData12); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 6 /* TargetOpcode::G_ANYEXT */) {
    // Leaf name: mutate_anyext_to_zext
    // Rule: mutate_anyext_to_zext
    if (!RuleConfig->isRuleDisabled(4)) {
      if (1
          && [&]() {
           return matchMutateAnyExtToZExt(*MIs[0], MRI); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'mutate_anyext_to_zext'\n");
 applyMutateAnyExtToZExt(*MIs[0], MRI, B, Observer); 
        return true;
      }
    }
    // Leaf name: anyext_trunc_fold
    // Rule: anyext_trunc_fold
    if (!RuleConfig->isRuleDisabled(31)) {
      if (1
          && [&]() {
           return Helper.matchCombineAnyExtTrunc(*MIs[0], MatchData31); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'anyext_trunc_fold'\n");
 return Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData31); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 7 /* TargetOpcode::G_OR */) {
    // Leaf name: hoist_logic_op_with_same_opcode_hands
    // Rule: hoist_logic_op_with_same_opcode_hands
    if (!RuleConfig->isRuleDisabled(5)) {
      if (1
          && [&]() {
           return Helper.matchHoistLogicOpWithSameOpcodeHands(*MIs[0], MatchData5); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'hoist_logic_op_with_same_opcode_hands'\n");
 Helper.applyBuildInstructionSteps(*MIs[0], MatchData5);
        return true;
      }
    }
    // Leaf name: redundant_or
    // Rule: redundant_or
    if (!RuleConfig->isRuleDisabled(9)) {
      if (1
          && [&]() {
           return Helper.matchRedundantOr(*MIs[0], MatchData9); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'redundant_or'\n");
 return Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData9); 
        return true;
      }
    }
    // Leaf name: constant_fold
    // Rule: constant_fold
    if (!RuleConfig->isRuleDisabled(23)) {
      if (1
          && [&]() {
           return Helper.matchConstantFold(*MIs[0], MatchData23); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'constant_fold'\n");
 Helper.replaceInstWithConstant(*MIs[0], MatchData23); 
        return true;
      }
    }
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(25)) {
      if (1
          && [&]() {
           return Helper.matchConstantOp(MIs[0]->getOperand(2), 0); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'right_identity_zero'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    // Leaf name: binop_same_val
    // Rule: binop_same_val
    if (!RuleConfig->isRuleDisabled(26)) {
      if (1
          && [&]() {
           return Helper.matchBinOpSameVal(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'binop_same_val'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    // Leaf name: undef_to_negative_one
    // Rule: undef_to_negative_one
    if (!RuleConfig->isRuleDisabled(44)) {
      if (1
          && [&]() {
           return Helper.matchAnyExplicitUseIsUndef(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'undef_to_negative_one'\n");
 Helper.replaceInstWithConstant(*MIs[0], -1); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 8 /* TargetOpcode::G_XOR */) {
    // Leaf name: hoist_logic_op_with_same_opcode_hands
    // Rule: hoist_logic_op_with_same_opcode_hands
    if (!RuleConfig->isRuleDisabled(5)) {
      if (1
          && [&]() {
           return Helper.matchHoistLogicOpWithSameOpcodeHands(*MIs[0], MatchData5); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'hoist_logic_op_with_same_opcode_hands'\n");
 Helper.applyBuildInstructionSteps(*MIs[0], MatchData5);
        return true;
      }
    }
    // Leaf name: xor_of_and_with_same_reg
    // Rule: xor_of_and_with_same_reg
    if (!RuleConfig->isRuleDisabled(7)) {
      if (1
          && [&]() {
           return Helper.matchXorOfAndWithSameReg(*MIs[0], MatchData7); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'xor_of_and_with_same_reg'\n");
 Helper.applyXorOfAndWithSameReg(*MIs[0], MatchData7); 
        return true;
      }
    }
    // Leaf name: constant_fold
    // Rule: constant_fold
    if (!RuleConfig->isRuleDisabled(23)) {
      if (1
          && [&]() {
           return Helper.matchConstantFold(*MIs[0], MatchData23); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'constant_fold'\n");
 Helper.replaceInstWithConstant(*MIs[0], MatchData23); 
        return true;
      }
    }
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(25)) {
      if (1
          && [&]() {
           return Helper.matchConstantOp(MIs[0]->getOperand(2), 0); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'right_identity_zero'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    // Leaf name: propagate_undef_any_op
    // Rule: propagate_undef_any_op
    if (!RuleConfig->isRuleDisabled(48)) {
      if (1
          && [&]() {
           return Helper.matchAnyExplicitUseIsUndef(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'propagate_undef_any_op'\n");
 Helper.replaceInstWithUndef(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 9 /* TargetOpcode::G_EXTRACT_VECTOR_ELT */) {
    // Leaf name: extractvecelt_pairwise_add
    // Rule: extractvecelt_pairwise_add
    if (!RuleConfig->isRuleDisabled(8)) {
      if (1
          && [&]() {
           return matchExtractVecEltPairwiseAdd(*MIs[0], MRI, MatchData8); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'extractvecelt_pairwise_add'\n");
 applyExtractVecEltPairwiseAdd(*MIs[0], MRI, B, MatchData8); 
        return true;
      }
    }
    // Leaf name: insert_extract_vec_elt_out_of_bounds
    // Rule: insert_extract_vec_elt_out_of_bounds
    if (!RuleConfig->isRuleDisabled(53)) {
      if (1
          && [&]() {
           return Helper.matchInsertExtractVecEltOutOfBounds(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'insert_extract_vec_elt_out_of_bounds'\n");
 Helper.replaceInstWithUndef(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 10 /* TargetOpcode::G_MUL */) {
    // Leaf name: mul_const
    // Rule: mul_const
    if (!RuleConfig->isRuleDisabled(10)) {
      if (1
          && [&]() {
           return matchAArch64MulConstCombine(*MIs[0], MRI, MatchData10); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'mul_const'\n");
 applyAArch64MulConstCombine(*MIs[0], MRI, B, MatchData10); 
        return true;
      }
    }
    // Leaf name: constant_fold
    // Rule: constant_fold
    if (!RuleConfig->isRuleDisabled(23)) {
      if (1
          && [&]() {
           return Helper.matchConstantFold(*MIs[0], MatchData23); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'constant_fold'\n");
 Helper.replaceInstWithConstant(*MIs[0], MatchData23); 
        return true;
      }
    }
    // Leaf name: binop_right_to_zero
    // Rule: binop_right_to_zero
    if (!RuleConfig->isRuleDisabled(28)) {
      if (1
          && [&]() {
           return Helper.matchOperandIsZero(*MIs[0], 2); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'binop_right_to_zero'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 2); 
        return true;
      }
    }
    // Leaf name: right_identity_one
    // Rule: right_identity_one
    if (!RuleConfig->isRuleDisabled(33)) {
      if (1
          && [&]() {
           return Helper.matchConstantOp(MIs[0]->getOperand(2), 1); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'right_identity_one'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    // Leaf name: undef_to_int_zero
    // Rule: undef_to_int_zero
    if (!RuleConfig->isRuleDisabled(43)) {
      if (1
          && [&]() {
           return Helper.matchAnyExplicitUseIsUndef(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'undef_to_int_zero'\n");
 Helper.replaceInstWithConstant(*MIs[0], 0); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 11 /* TargetOpcode::G_ASHR */) {
    // Leaf name: bitfield_extract_from_shr
    // Rule: bitfield_extract_from_shr
    if (!RuleConfig->isRuleDisabled(14)) {
      if (1
          && [&]() {
           return Helper.matchBitfieldExtractFromShr(*MIs[0], MatchData14); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'bitfield_extract_from_shr'\n");
 Helper.applyBuildFn(*MIs[0], MatchData14); 
        return true;
      }
    }
    // Leaf name: bitfield_extract_from_shr_and
    // Rule: bitfield_extract_from_shr_and
    if (!RuleConfig->isRuleDisabled(15)) {
      if (1
          && [&]() {
           return Helper.matchBitfieldExtractFromShrAnd(*MIs[0], MatchData15); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'bitfield_extract_from_shr_and'\n");
 Helper.applyBuildFn(*MIs[0], MatchData15); 
        return true;
      }
    }
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(25)) {
      if (1
          && [&]() {
           return Helper.matchConstantOp(MIs[0]->getOperand(2), 0); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'right_identity_zero'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    // Leaf name: binop_right_undef_to_undef
    // Rule: binop_right_undef_to_undef
    if (!RuleConfig->isRuleDisabled(46)) {
      if (1
          && [&]() {
           return Helper.matchOperandIsUndef(*MIs[0], 2); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'binop_right_undef_to_undef'\n");
 Helper.replaceInstWithUndef(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 12 /* TargetOpcode::G_LSHR */) {
    // Leaf name: bitfield_extract_from_shr
    // Rule: bitfield_extract_from_shr
    if (!RuleConfig->isRuleDisabled(14)) {
      if (1
          && [&]() {
           return Helper.matchBitfieldExtractFromShr(*MIs[0], MatchData14); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'bitfield_extract_from_shr'\n");
 Helper.applyBuildFn(*MIs[0], MatchData14); 
        return true;
      }
    }
    // Leaf name: bitfield_extract_from_shr_and
    // Rule: bitfield_extract_from_shr_and
    if (!RuleConfig->isRuleDisabled(15)) {
      if (1
          && [&]() {
           return Helper.matchBitfieldExtractFromShrAnd(*MIs[0], MatchData15); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'bitfield_extract_from_shr_and'\n");
 Helper.applyBuildFn(*MIs[0], MatchData15); 
        return true;
      }
    }
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(25)) {
      if (1
          && [&]() {
           return Helper.matchConstantOp(MIs[0]->getOperand(2), 0); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'right_identity_zero'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    // Leaf name: binop_right_undef_to_undef
    // Rule: binop_right_undef_to_undef
    if (!RuleConfig->isRuleDisabled(46)) {
      if (1
          && [&]() {
           return Helper.matchOperandIsUndef(*MIs[0], 2); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'binop_right_undef_to_undef'\n");
 Helper.replaceInstWithUndef(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 13 /* TargetOpcode::G_ROTR */) {
    // Leaf name: rotate_out_of_range
    // Rule: rotate_out_of_range
    if (!RuleConfig->isRuleDisabled(16)) {
      if (1
          && [&]() {
           return Helper.matchRotateOutOfRange(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'rotate_out_of_range'\n");
 Helper.applyRotateOutOfRange(*MIs[0]); 
        return true;
      }
    }
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(25)) {
      if (1
          && [&]() {
           return Helper.matchConstantOp(MIs[0]->getOperand(2), 0); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'right_identity_zero'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 14 /* TargetOpcode::G_ROTL */) {
    // Leaf name: rotate_out_of_range
    // Rule: rotate_out_of_range
    if (!RuleConfig->isRuleDisabled(16)) {
      if (1
          && [&]() {
           return Helper.matchRotateOutOfRange(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'rotate_out_of_range'\n");
 Helper.applyRotateOutOfRange(*MIs[0]); 
        return true;
      }
    }
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(25)) {
      if (1
          && [&]() {
           return Helper.matchConstantOp(MIs[0]->getOperand(2), 0); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'right_identity_zero'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 15 /* TargetOpcode::G_ICMP */) {
    // Leaf name: icmp_to_true_false_known_bits
    // Rule: icmp_to_true_false_known_bits
    if (!RuleConfig->isRuleDisabled(17)) {
      if (1
          && [&]() {
           return Helper.matchICmpToTrueFalseKnownBits(*MIs[0], MatchData17); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'icmp_to_true_false_known_bits'\n");
 Helper.replaceInstWithConstant(*MIs[0], MatchData17); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 16 /* TargetOpcode::G_MERGE_VALUES */) {
    // Leaf name: merge_unmerge
    // Rule: merge_unmerge
    if (!RuleConfig->isRuleDisabled(18)) {
      if (1
          && [&]() {
           return Helper.matchCombineMergeUnmerge(*MIs[0], MatchData18); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'merge_unmerge'\n");
 Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData18); 
        return true;
      }
    }
    // Leaf name: fold_merge_to_zext
    // Rule: fold_merge_to_zext
    if (!RuleConfig->isRuleDisabled(22)) {
      if (1
          && [&]() {
           return matchFoldMergeToZext(*MIs[0], MRI); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_merge_to_zext'\n");
 applyFoldMergeToZext(*MIs[0], MRI, B, Observer); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 17 /* TargetOpcode::G_SELECT */) {
    // Leaf name: select_undef_cmp
    // Rule: select_undef_cmp
    if (!RuleConfig->isRuleDisabled(19)) {
      if (1
          && [&]() {
           return Helper.matchUndefSelectCmp(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'select_undef_cmp'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 2); 
        return true;
      }
    }
    // Leaf name: select_constant_cmp
    // Rule: select_constant_cmp
    if (!RuleConfig->isRuleDisabled(20)) {
      if (1
          && [&]() {
           return Helper.matchConstantSelectCmp(*MIs[0], MatchData20); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'select_constant_cmp'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], MatchData20); 
        return true;
      }
    }
    // Leaf name: select_to_logical
    // Rule: select_to_logical
    if (!RuleConfig->isRuleDisabled(21)) {
      if (1
          && [&]() {
           return Helper.matchSelectToLogical(*MIs[0], MatchData21); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'select_to_logical'\n");
 Helper.applyBuildFn(*MIs[0], MatchData21); 
        return true;
      }
    }
    // Leaf name: select_same_val
    // Rule: select_same_val
    if (!RuleConfig->isRuleDisabled(24)) {
      if (1
          && [&]() {
           return Helper.matchSelectSameVal(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'select_same_val'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 2); 
        return true;
      }
    }
    // Leaf name: select_to_minmax
    // Rule: select_to_minmax
    if (!RuleConfig->isRuleDisabled(54)) {
      if (1
          && [&]() {
           return Helper.matchSimplifySelectToMinMax(*MIs[0], MatchData54); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'select_to_minmax'\n");
 Helper.applyBuildFn(*MIs[0], MatchData54); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 18 /* TargetOpcode::G_ADD */) {
    // Leaf name: constant_fold
    // Rule: constant_fold
    if (!RuleConfig->isRuleDisabled(23)) {
      if (1
          && [&]() {
           return Helper.matchConstantFold(*MIs[0], MatchData23); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'constant_fold'\n");
 Helper.replaceInstWithConstant(*MIs[0], MatchData23); 
        return true;
      }
    }
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(25)) {
      if (1
          && [&]() {
           return Helper.matchConstantOp(MIs[0]->getOperand(2), 0); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'right_identity_zero'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    // Leaf name: add_sub_reg
    // Rule: add_sub_reg
    if (!RuleConfig->isRuleDisabled(34)) {
      if (1
          && [&]() {
           return Helper.matchAddSubSameReg(*MIs[0], MatchData34); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'add_sub_reg'\n");
 return Helper.replaceSingleDefInstWithReg(*MIs[0],
                                                          MatchData34); 
        return true;
      }
    }
    // Leaf name: propagate_undef_any_op
    // Rule: propagate_undef_any_op
    if (!RuleConfig->isRuleDisabled(48)) {
      if (1
          && [&]() {
           return Helper.matchAnyExplicitUseIsUndef(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'propagate_undef_any_op'\n");
 Helper.replaceInstWithUndef(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 19 /* TargetOpcode::G_SUB */) {
    // Leaf name: constant_fold
    // Rule: constant_fold
    if (!RuleConfig->isRuleDisabled(23)) {
      if (1
          && [&]() {
           return Helper.matchConstantFold(*MIs[0], MatchData23); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'constant_fold'\n");
 Helper.replaceInstWithConstant(*MIs[0], MatchData23); 
        return true;
      }
    }
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(25)) {
      if (1
          && [&]() {
           return Helper.matchConstantOp(MIs[0]->getOperand(2), 0); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'right_identity_zero'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    // Leaf name: propagate_undef_any_op
    // Rule: propagate_undef_any_op
    if (!RuleConfig->isRuleDisabled(48)) {
      if (1
          && [&]() {
           return Helper.matchAnyExplicitUseIsUndef(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'propagate_undef_any_op'\n");
 Helper.replaceInstWithUndef(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 20 /* TargetOpcode::G_SHL */) {
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(25)) {
      if (1
          && [&]() {
           return Helper.matchConstantOp(MIs[0]->getOperand(2), 0); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'right_identity_zero'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    // Leaf name: binop_left_undef_to_zero
    // Rule: binop_left_undef_to_zero
    if (!RuleConfig->isRuleDisabled(45)) {
      if (1
          && [&]() {
           return Helper.matchOperandIsUndef(*MIs[0], 1); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'binop_left_undef_to_zero'\n");
 Helper.replaceInstWithConstant(*MIs[0], 0); 
        return true;
      }
    }
    // Leaf name: binop_right_undef_to_undef
    // Rule: binop_right_undef_to_undef
    if (!RuleConfig->isRuleDisabled(46)) {
      if (1
          && [&]() {
           return Helper.matchOperandIsUndef(*MIs[0], 2); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'binop_right_undef_to_undef'\n");
 Helper.replaceInstWithUndef(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 21 /* TargetOpcode::G_PTR_ADD */) {
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(25)) {
      if (1
          && [&]() {
           return Helper.matchConstantOp(MIs[0]->getOperand(2), 0); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'right_identity_zero'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    // Leaf name: ptr_add_immed_chain
    // Rule: ptr_add_immed_chain
    if (!RuleConfig->isRuleDisabled(39)) {
      if (1
          && [&]() {
           return Helper.matchPtrAddImmedChain(*MIs[0], MatchData39); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'ptr_add_immed_chain'\n");
 Helper.applyPtrAddImmedChain(*MIs[0], MatchData39); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 22 /* TargetOpcode::G_SDIV */) {
    // Leaf name: binop_left_to_zero
    // Rule: binop_left_to_zero
    if (!RuleConfig->isRuleDisabled(27)) {
      if (1
          && [&]() {
           return Helper.matchOperandIsZero(*MIs[0], 1); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'binop_left_to_zero'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 23 /* TargetOpcode::G_UDIV */) {
    // Leaf name: binop_left_to_zero
    // Rule: binop_left_to_zero
    if (!RuleConfig->isRuleDisabled(27)) {
      if (1
          && [&]() {
           return Helper.matchOperandIsZero(*MIs[0], 1); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'binop_left_to_zero'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    // Leaf name: binop_left_undef_to_zero
    // Rule: binop_left_undef_to_zero
    if (!RuleConfig->isRuleDisabled(45)) {
      if (1
          && [&]() {
           return Helper.matchOperandIsUndef(*MIs[0], 1); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'binop_left_undef_to_zero'\n");
 Helper.replaceInstWithConstant(*MIs[0], 0); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 24 /* TargetOpcode::G_SREM */) {
    // Leaf name: binop_left_to_zero
    // Rule: binop_left_to_zero
    if (!RuleConfig->isRuleDisabled(27)) {
      if (1
          && [&]() {
           return Helper.matchOperandIsZero(*MIs[0], 1); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'binop_left_to_zero'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 25 /* TargetOpcode::G_UREM */) {
    // Leaf name: binop_left_to_zero
    // Rule: binop_left_to_zero
    if (!RuleConfig->isRuleDisabled(27)) {
      if (1
          && [&]() {
           return Helper.matchOperandIsZero(*MIs[0], 1); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'binop_left_to_zero'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], 1); 
        return true;
      }
    }
    // Leaf name: binop_left_undef_to_zero
    // Rule: binop_left_undef_to_zero
    if (!RuleConfig->isRuleDisabled(45)) {
      if (1
          && [&]() {
           return Helper.matchOperandIsUndef(*MIs[0], 1); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'binop_left_undef_to_zero'\n");
 Helper.replaceInstWithConstant(*MIs[0], 0); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 26 /* TargetOpcode::G_INTTOPTR */) {
    // Leaf name: p2i_to_i2p
    // Rule: p2i_to_i2p
    if (!RuleConfig->isRuleDisabled(29)) {
      if (1
          && [&]() {
           return Helper.matchCombineI2PToP2I(*MIs[0], MatchData29); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'p2i_to_i2p'\n");
 Helper.applyCombineI2PToP2I(*MIs[0], MatchData29); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 27 /* TargetOpcode::G_PTRTOINT */) {
    Partition = -1;
    if (MIs.size() <= 1) MIs.resize(2);
    MIs[1] = nullptr;
    if (MIs[0]->getOperand(1).isReg())
      MIs[1] = MRI.getVRegDef(MIs[0]->getOperand(1).getReg());
    if (MIs[1] != nullptr) Partition = 0;
    if (Partition == -1) return false;
    if (Partition == 0 /* true */) {
      Partition = -1;
      switch (MIs[1]->getOpcode()) {
      case TargetOpcode::G_INTTOPTR: Partition = 0; break;
      }
      // Default case but without conflicting with potential default case in selection.
      if (Partition == -1) return false;
      if (Partition == 0 /* TargetOpcode::G_INTTOPTR */) {
        // Leaf name: i2p_to_p2i
        // Rule: i2p_to_p2i
        if (!RuleConfig->isRuleDisabled(30)) {
          if (1
              && [&]() {
               MatchData30 = MIs[1]->getOperand(1).getReg(); 
              return true;
          }()             ) {
               LLVM_DEBUG(dbgs() << "Applying rule 'i2p_to_p2i'\n");
 Helper.applyCombineP2IToI2P(*MIs[0], MatchData30); 
            return true;
          }
        }
        return false;
      }
    }
  }
  if (Partition == 28 /* TargetOpcode::G_FNEG */) {
    Partition = -1;
    if (MIs.size() <= 1) MIs.resize(2);
    MIs[1] = nullptr;
    if (MIs[0]->getOperand(1).isReg())
      MIs[1] = MRI.getVRegDef(MIs[0]->getOperand(1).getReg());
    if (MIs[1] != nullptr) Partition = 0;
    if (Partition == -1) return false;
    if (Partition == 0 /* true */) {
      Partition = -1;
      switch (MIs[1]->getOpcode()) {
      case TargetOpcode::G_FNEG: Partition = 0; break;
      }
      // Default case but without conflicting with potential default case in selection.
      if (Partition == -1) return false;
      if (Partition == 0 /* TargetOpcode::G_FNEG */) {
        // Leaf name: fneg_fneg_fold
        // Rule: fneg_fneg_fold
        if (!RuleConfig->isRuleDisabled(32)) {
          if (1
              && [&]() {
               MatchData32 = MIs[1]->getOperand(1).getReg(); 
              return true;
          }()             ) {
               LLVM_DEBUG(dbgs() << "Applying rule 'fneg_fneg_fold'\n");
 return Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData32); 
            return true;
          }
        }
        return false;
      }
    }
  }
  if (Partition == 29 /* TargetOpcode::G_BUILD_VECTOR_TRUNC */) {
    // Leaf name: buildvector_identity_fold
    // Rule: buildvector_identity_fold
    if (!RuleConfig->isRuleDisabled(35)) {
      if (1
          && [&]() {
           return Helper.matchBuildVectorIdentityFold(*MIs[0], MatchData35); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'buildvector_identity_fold'\n");
 Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData35); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 30 /* TargetOpcode::G_BUILD_VECTOR */) {
    // Leaf name: buildvector_identity_fold
    // Rule: buildvector_identity_fold
    if (!RuleConfig->isRuleDisabled(35)) {
      if (1
          && [&]() {
           return Helper.matchBuildVectorIdentityFold(*MIs[0], MatchData35); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'buildvector_identity_fold'\n");
 Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData35); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 31 /* TargetOpcode::G_TRUNC */) {
    // Leaf name: trunc_buildvector_fold
    // Rule: trunc_buildvector_fold
    if (!RuleConfig->isRuleDisabled(36)) {
      if (1
          && [&]() {
           return Helper.matchTruncBuildVectorFold(*MIs[0], MatchData36); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'trunc_buildvector_fold'\n");
 Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData36); 
        return true;
      }
    }
    // Leaf name: trunc_lshr_buildvector_fold
    // Rule: trunc_lshr_buildvector_fold
    if (!RuleConfig->isRuleDisabled(37)) {
      if (1
          && [&]() {
           return Helper.matchTruncLshrBuildVectorFold(*MIs[0], MatchData37); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'trunc_lshr_buildvector_fold'\n");
 Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData37); 
        return true;
      }
    }
    // Leaf name: propagate_undef_any_op
    // Rule: propagate_undef_any_op
    if (!RuleConfig->isRuleDisabled(48)) {
      if (1
          && [&]() {
           return Helper.matchAnyExplicitUseIsUndef(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'propagate_undef_any_op'\n");
 Helper.replaceInstWithUndef(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 32 /* TargetOpcode::G_BITCAST */) {
    Partition = -1;
    if (MIs.size() <= 1) MIs.resize(2);
    MIs[1] = nullptr;
    if (MIs[0]->getOperand(1).isReg())
      MIs[1] = MRI.getVRegDef(MIs[0]->getOperand(1).getReg());
    if (MIs[1] != nullptr) Partition = 0;
    if (Partition == -1) return false;
    if (Partition == 0 /* true */) {
      Partition = -1;
      switch (MIs[1]->getOpcode()) {
      case TargetOpcode::G_BITCAST: Partition = 0; break;
      }
      // Default case but without conflicting with potential default case in selection.
      if (Partition == -1) return false;
      if (Partition == 0 /* TargetOpcode::G_BITCAST */) {
        // Leaf name: bitcast_bitcast_fold
        // Rule: bitcast_bitcast_fold
        if (!RuleConfig->isRuleDisabled(38)) {
          if (1
              && [&]() {
               return MRI.getType(MIs[1]->getOperand(1).getReg()) == MRI.getType(MIs[0]->getOperand(0).getReg()); 
              return true;
          }()             ) {
               LLVM_DEBUG(dbgs() << "Applying rule 'bitcast_bitcast_fold'\n");
 Helper.replaceSingleDefInstWithReg(*MIs[0], MIs[1]->getOperand(1).getReg()); 
            return true;
          }
        }
        return false;
      }
    }
  }
  if (Partition == 33 /* TargetOpcode::G_STORE */) {
    // Leaf name: split_store_zero_128
    // Rule: split_store_zero_128
    if (!RuleConfig->isRuleDisabled(41)) {
      if (1
          && [&]() {
           return matchSplitStoreZero128(*MIs[0], MRI); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'split_store_zero_128'\n");
 applySplitStoreZero128(*MIs[0], MRI, B, Observer); 
        return true;
      }
    }
    // Leaf name: erase_undef_store
    // Rule: erase_undef_store
    if (!RuleConfig->isRuleDisabled(51)) {
      if (1
          && [&]() {
           return Helper.matchUndefStore(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'erase_undef_store'\n");
 return Helper.eraseInst(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 34 /* TargetOpcode::G_UITOFP */) {
    // Leaf name: undef_to_fp_zero
    // Rule: undef_to_fp_zero
    if (!RuleConfig->isRuleDisabled(42)) {
      if (1
          && [&]() {
           return Helper.matchAnyExplicitUseIsUndef(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'undef_to_fp_zero'\n");
 Helper.replaceInstWithFConstant(*MIs[0], 0.0); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 35 /* TargetOpcode::G_SITOFP */) {
    // Leaf name: undef_to_fp_zero
    // Rule: undef_to_fp_zero
    if (!RuleConfig->isRuleDisabled(42)) {
      if (1
          && [&]() {
           return Helper.matchAnyExplicitUseIsUndef(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'undef_to_fp_zero'\n");
 Helper.replaceInstWithFConstant(*MIs[0], 0.0); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 36 /* TargetOpcode::G_ABS */) {
    // Leaf name: unary_undef_to_zero
    // Rule: unary_undef_to_zero
    if (!RuleConfig->isRuleDisabled(47)) {
      if (1
          && [&]() {
           return Helper.matchOperandIsUndef(*MIs[0], 1); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'unary_undef_to_zero'\n");
 Helper.replaceInstWithConstant(*MIs[0], 0); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 37 /* TargetOpcode::G_FPTOSI */) {
    // Leaf name: propagate_undef_any_op
    // Rule: propagate_undef_any_op
    if (!RuleConfig->isRuleDisabled(48)) {
      if (1
          && [&]() {
           return Helper.matchAnyExplicitUseIsUndef(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'propagate_undef_any_op'\n");
 Helper.replaceInstWithUndef(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 38 /* TargetOpcode::G_FPTOUI */) {
    // Leaf name: propagate_undef_any_op
    // Rule: propagate_undef_any_op
    if (!RuleConfig->isRuleDisabled(48)) {
      if (1
          && [&]() {
           return Helper.matchAnyExplicitUseIsUndef(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'propagate_undef_any_op'\n");
 Helper.replaceInstWithUndef(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 39 /* TargetOpcode::G_SHUFFLE_VECTOR */) {
    // Leaf name: propagate_undef_all_ops
    // Rule: propagate_undef_all_ops
    if (!RuleConfig->isRuleDisabled(49)) {
      if (1
          && [&]() {
           return Helper.matchAllExplicitUsesAreUndef(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'propagate_undef_all_ops'\n");
 Helper.replaceInstWithUndef(*MIs[0]); 
        return true;
      }
    }
    // Leaf name: propagate_undef_shuffle_mask
    // Rule: propagate_undef_shuffle_mask
    if (!RuleConfig->isRuleDisabled(50)) {
      if (1
          && [&]() {
           return Helper.matchUndefShuffleVectorMask(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'propagate_undef_shuffle_mask'\n");
 Helper.replaceInstWithUndef(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 40 /* TargetOpcode::G_UNMERGE_VALUES */) {
    // Leaf name: unmerge_undef
    // Rule: unmerge_undef
    if (!RuleConfig->isRuleDisabled(52)) {
      if (1
          && [&]() {
           return Helper.matchCombineUnmergeUndef(*MIs[0], MatchData52); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'unmerge_undef'\n");
 Helper.applyBuildFn(*MIs[0], MatchData52); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 41 /* TargetOpcode::G_INSERT_VECTOR_ELT */) {
    // Leaf name: insert_extract_vec_elt_out_of_bounds
    // Rule: insert_extract_vec_elt_out_of_bounds
    if (!RuleConfig->isRuleDisabled(53)) {
      if (1
          && [&]() {
           return Helper.matchInsertExtractVecEltOutOfBounds(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'insert_extract_vec_elt_out_of_bounds'\n");
 Helper.replaceInstWithUndef(*MIs[0]); 
        return true;
      }
    }
    return false;
  }

  return false;
}
#endif // ifdef AARCH64POSTLEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_CPP
