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
class AArch64GenPreLegalizerCombinerHelperRuleConfig {
  SparseBitVector<> DisabledRules;

public:
  bool parseCommandLineOption();
  bool isRuleDisabled(unsigned ID) const;
  bool setRuleEnabled(StringRef RuleIdentifier);
  bool setRuleDisabled(StringRef RuleIdentifier);
};

class AArch64GenPreLegalizerCombinerHelper : public AArch64PreLegalizerCombinerHelperState {
  const AArch64GenPreLegalizerCombinerHelperRuleConfig *RuleConfig;

public:
  template <typename... Args>AArch64GenPreLegalizerCombinerHelper(const AArch64GenPreLegalizerCombinerHelperRuleConfig &RuleConfig, Args &&... args) : AArch64PreLegalizerCombinerHelperState(std::forward<Args>(args)...), RuleConfig(&RuleConfig) {}

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
  case 9:	 // 4 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ddo_by_0", 8) != 0)
        break;
      return 73;	 // "addo_by_0"
    case 'c':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "opy_prop", 8) != 0)
        break;
      return 0;	 // "copy_prop"
    case 'm':	 // 2 strings to match.
      if (memcmp(RuleIdentifier.data()+1, "ulo_by_", 7) != 0)
        break;
      switch (RuleIdentifier[8]) {
      default: break;
      case '0':	 // 1 string to match.
        return 72;	 // "mulo_by_0"
      case '2':	 // 1 string to match.
        return 71;	 // "mulo_by_2"
      }
      break;
    }
    break;
  case 10:	 // 3 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'i':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "2p_to_p2i", 9) != 0)
        break;
      return 29;	 // "i2p_to_p2i"
    case 'm':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ul_to_shl", 9) != 0)
        break;
      return 1;	 // "mul_to_shl"
    case 'p':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "2i_to_i2p", 9) != 0)
        break;
      return 28;	 // "p2i_to_i2p"
    }
    break;
  case 11:	 // 4 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "dd_sub_reg", 10) != 0)
        break;
      return 33;	 // "add_sub_reg"
    case 's':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ub_add_reg", 10) != 0)
        break;
      return 105;	 // "sub_add_reg"
    case 't':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "runc_shift", 10) != 0)
        break;
      return 67;	 // "trunc_shift"
    case 'u':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "nmerge_cst", 10) != 0)
        break;
      return 62;	 // "unmerge_cst"
    }
    break;
  case 12:	 // 6 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "dde_to_addo", 11) != 0)
        break;
      return 74;	 // "adde_to_addo"
    case 'e':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "xt_ext_fold", 11) != 0)
        break;
      return 58;	 // "ext_ext_fold"
    case 'f':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "sub_to_fneg", 11) != 0)
        break;
      return 108;	 // "fsub_to_fneg"
    case 'm':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ulh_to_lshr", 11) != 0)
        break;
      return 93;	 // "mulh_to_lshr"
    case 'n':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ot_cmp_fold", 11) != 0)
        break;
      return 59;	 // "not_cmp_fold"
    case 'r':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "edundant_or", 11) != 0)
        break;
      return 52;	 // "redundant_or"
    }
    break;
  case 13:	 // 8 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'c':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "onstant_fold", 12) != 0)
        break;
      return 89;	 // "constant_fold"
    case 'l':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "oad_and_mask", 12) != 0)
        break;
      return 9;	 // "load_and_mask"
    case 'm':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "erge_unmerge", 12) != 0)
        break;
      return 65;	 // "merge_unmerge"
    case 'r':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "edundant_and", 12) != 0)
        break;
      return 50;	 // "redundant_and"
    case 's':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "div_by_const", 12) != 0)
        break;
      return 92;	 // "sdiv_by_const"
    case 'u':	 // 3 strings to match.
      switch (RuleIdentifier[1]) {
      default: break;
      case 'd':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+2, "iv_by_const", 11) != 0)
          break;
        return 91;	 // "udiv_by_const"
      case 'n':	 // 2 strings to match.
        if (memcmp(RuleIdentifier.data()+2, "merge_", 6) != 0)
          break;
        switch (RuleIdentifier[8]) {
        default: break;
        case 'm':	 // 1 string to match.
          if (memcmp(RuleIdentifier.data()+9, "erge", 4) != 0)
            break;
          return 61;	 // "unmerge_merge"
        case 'u':	 // 1 string to match.
          if (memcmp(RuleIdentifier.data()+9, "ndef", 4) != 0)
            break;
          return 21;	 // "unmerge_undef"
        }
        break;
      }
      break;
    }
    break;
  case 14:	 // 7 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'b':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "inop_same_val", 13) != 0)
        break;
      return 25;	 // "binop_same_val"
    case 'c':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "onstant_fp_op", 13) != 0)
        break;
      return 68;	 // "constant_fp_op"
    case 'f':	 // 2 strings to match.
      switch (RuleIdentifier[1]) {
      default: break;
      case 'a':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+2, "bs_fneg_fold", 12) != 0)
          break;
        return 90;	 // "fabs_fneg_fold"
      case 'n':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+2, "eg_fneg_fold", 12) != 0)
          break;
        return 31;	 // "fneg_fneg_fold"
      }
      break;
    case 'm':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ul_by_neg_one", 13) != 0)
        break;
      return 3;	 // "mul_by_neg_one"
    case 'r':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "eassoc_ptradd", 13) != 0)
        break;
      return 41;	 // "reassoc_ptradd"
    case 't':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "runc_ext_fold", 13) != 0)
        break;
      return 66;	 // "trunc_ext_fold"
    }
    break;
  case 15:	 // 6 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'e':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "xtending_loads", 14) != 0)
        break;
      return 8;	 // "extending_loads"
    case 'i':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "dempotent_prop", 14) != 0)
        break;
      return 4;	 // "idempotent_prop"
    case 'l':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "oad_or_combine", 14) != 0)
        break;
      return 80;	 // "load_or_combine"
    case 'o':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "verlapping_and", 14) != 0)
        break;
      return 70;	 // "overlapping_and"
    case 's':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "elect_same_val", 14) != 0)
        break;
      return 23;	 // "select_same_val"
    case 'z':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ext_trunc_fold", 14) != 0)
        break;
      return 54;	 // "zext_trunc_fold"
    }
    break;
  case 16:	 // 4 strings to match.
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
        return 106;	 // "select_to_minmax"
      case 'u':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+8, "ndef_cmp", 8) != 0)
          break;
        return 47;	 // "select_undef_cmp"
      }
      break;
    case 't':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "runcstore_merge", 15) != 0)
        break;
      return 81;	 // "truncstore_merge"
    case 'u':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ndef_to_fp_zero", 15) != 0)
        break;
      return 11;	 // "undef_to_fp_zero"
    }
    break;
  case 17:	 // 9 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'a':	 // 2 strings to match.
      switch (RuleIdentifier[1]) {
      default: break;
      case 'd':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+2, "d_p2i_to_ptradd", 15) != 0)
          break;
        return 2;	 // "add_p2i_to_ptradd"
      case 'n':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+2, "yext_trunc_fold", 15) != 0)
          break;
        return 30;	 // "anyext_trunc_fold"
      }
      break;
    case 'd':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "iv_rem_to_divrem", 16) != 0)
        break;
      return 82;	 // "div_rem_to_divrem"
    case 'e':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "rase_undef_store", 16) != 0)
        break;
      return 20;	 // "erase_undef_store"
    case 'p':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "tr_add_with_zero", 16) != 0)
        break;
      return 77;	 // "ptr_add_with_zero"
    case 's':	 // 2 strings to match.
      switch (RuleIdentifier[1]) {
      default: break;
      case 'e':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+2, "lect_to_logical", 15) != 0)
          break;
        return 49;	 // "select_to_logical"
      case 'h':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+2, "ift_immed_chain", 15) != 0)
          break;
        return 78;	 // "shift_immed_chain"
      }
      break;
    case 'u':	 // 2 strings to match.
      switch (RuleIdentifier[1]) {
      default: break;
      case 'n':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+2, "def_to_int_zero", 15) != 0)
          break;
        return 12;	 // "undef_to_int_zero"
      case 'r':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+2, "em_pow2_to_mask", 15) != 0)
          break;
        return 53;	 // "urem_pow2_to_mask"
      }
      break;
    }
    break;
  case 18:	 // 5 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'b':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "inop_left_to_zero", 17) != 0)
        break;
      return 26;	 // "binop_left_to_zero"
    case 'c':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ombine_minmax_nan", 17) != 0)
        break;
      return 75;	 // "combine_minmax_nan"
    case 'f':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "old_global_offset", 17) != 0)
        break;
      return 112;	 // "fold_global_offset"
    case 'r':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ight_identity_one", 17) != 0)
        break;
      return 32;	 // "right_identity_one"
    case 's':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ext_inreg_of_load", 17) != 0)
        break;
      return 44;	 // "sext_inreg_of_load"
    }
    break;
  case 19:	 // 8 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'b':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "inop_right_to_zero", 18) != 0)
        break;
      return 27;	 // "binop_right_to_zero"
    case 'c':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "onst_ptradd_to_i2p", 18) != 0)
        break;
      return 69;	 // "const_ptradd_to_i2p"
    case 'e':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "xtend_through_phis", 18) != 0)
        break;
      return 38;	 // "extend_through_phis"
    case 'p':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "tr_add_immed_chain", 18) != 0)
        break;
      return 42;	 // "ptr_add_immed_chain"
    case 'r':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ight_identity_zero", 18) != 0)
        break;
      return 24;	 // "right_identity_zero"
    case 's':	 // 2 strings to match.
      switch (RuleIdentifier[1]) {
      default: break;
      case 'e':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+2, "lect_constant_cmp", 17) != 0)
          break;
        return 48;	 // "select_constant_cmp"
      case 'i':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+2, "mplify_add_to_sub", 17) != 0)
          break;
        return 39;	 // "simplify_add_to_sub"
      }
      break;
    case 'u':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "nary_undef_to_zero", 18) != 0)
        break;
      return 16;	 // "unary_undef_to_zero"
    }
    break;
  case 20:	 // 6 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "nd_or_disjoint_mask", 19) != 0)
        break;
      return 95;	 // "and_or_disjoint_mask"
    case 'b':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "itcast_bitcast_fold", 19) != 0)
        break;
      return 37;	 // "bitcast_bitcast_fold"
    case 'i':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "cmp_redundant_trunc", 19) != 0)
        break;
      return 111;	 // "icmp_redundant_trunc"
    case 'r':	 // 2 strings to match.
      if (memcmp(RuleIdentifier.data()+1, "edu", 3) != 0)
        break;
      switch (RuleIdentifier[4]) {
      default: break;
      case 'c':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+5, "e_shl_of_extend", 15) != 0)
          break;
        return 45;	 // "reduce_shl_of_extend"
      case 'n':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+5, "dant_sext_inreg", 15) != 0)
          break;
        return 51;	 // "redundant_sext_inreg"
      }
      break;
    case 'u':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "nmerge_zext_to_zext", 19) != 0)
        break;
      return 64;	 // "unmerge_zext_to_zext"
    }
    break;
  case 21:	 // 3 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'f':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "constant_to_constant", 20) != 0)
        break;
      return 110;	 // "fconstant_to_constant"
    case 'u':	 // 2 strings to match.
      if (RuleIdentifier[1] != 'n')
        break;
      switch (RuleIdentifier[2]) {
      default: break;
      case 'd':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+3, "ef_to_negative_one", 18) != 0)
          break;
        return 13;	 // "undef_to_negative_one"
      case 'm':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+3, "erge_dead_to_trunc", 18) != 0)
          break;
        return 63;	 // "unmerge_dead_to_trunc"
      }
      break;
    }
    break;
  case 22:	 // 7 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'f':	 // 2 strings to match.
      switch (RuleIdentifier[1]) {
      default: break;
      case 'o':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+2, "ld_binop_into_select", 20) != 0)
          break;
        return 104;	 // "fold_binop_into_select"
      case 'u':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+2, "nnel_shift_to_rotate", 20) != 0)
          break;
        return 84;	 // "funnel_shift_to_rotate"
      }
      break;
    case 'i':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "cmp_to_lhs_known_bits", 21) != 0)
        break;
      return 56;	 // "icmp_to_lhs_known_bits"
    case 'p':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ropagate_undef_any_op", 21) != 0)
        break;
      return 17;	 // "propagate_undef_any_op"
    case 'r':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "edundant_neg_operands", 21) != 0)
        break;
      return 94;	 // "redundant_neg_operands"
    case 's':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "hl_ashr_to_sext_inreg", 21) != 0)
        break;
      return 43;	 // "shl_ashr_to_sext_inreg"
    case 't':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "runc_buildvector_fold", 21) != 0)
        break;
      return 35;	 // "trunc_buildvector_fold"
    }
    break;
  case 23:	 // 2 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'c':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ommute_constant_to_rhs", 22) != 0)
        break;
      return 109;	 // "commute_constant_to_rhs"
    case 'p':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ropagate_undef_all_ops", 22) != 0)
        break;
      return 18;	 // "propagate_undef_all_ops"
    }
    break;
  case 24:	 // 4 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'b':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "inop_left_undef_to_zero", 23) != 0)
        break;
      return 14;	 // "binop_left_undef_to_zero"
    case 'n':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "arrow_binop_feeding_and", 23) != 0)
        break;
      return 46;	 // "narrow_binop_feeding_and"
    case 's':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ext_inreg_to_zext_inreg", 23) != 0)
        break;
      return 57;	 // "sext_inreg_to_zext_inreg"
    case 'x':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "or_of_and_with_same_reg", 23) != 0)
        break;
      return 76;	 // "xor_of_and_with_same_reg"
    }
    break;
  case 25:	 // 4 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'b':	 // 3 strings to match.
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
          return 86;	 // "bitfield_extract_from_and"
        case 's':	 // 1 string to match.
          if (memcmp(RuleIdentifier.data()+23, "hr", 2) != 0)
            break;
          return 87;	 // "bitfield_extract_from_shr"
        }
        break;
      case 'u':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+2, "ildvector_identity_fold", 23) != 0)
          break;
        return 34;	 // "buildvector_identity_fold"
      }
      break;
    case 'e':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "xtract_vec_elt_build_vec", 24) != 0)
        break;
      return 6;	 // "extract_vec_elt_build_vec"
    }
    break;
  case 26:	 // 3 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'b':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "inop_right_undef_to_undef", 25) != 0)
        break;
      return 15;	 // "binop_right_undef_to_undef"
    case 'c':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ombine_indexed_load_store", 25) != 0)
        break;
      return 10;	 // "combine_indexed_load_store"
    case 'f':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "unnel_shift_from_or_shift", 25) != 0)
        break;
      return 83;	 // "funnel_shift_from_or_shift"
    }
    break;
  case 27:	 // 2 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'r':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "edundant_binop_in_equality", 26) != 0)
        break;
      return 107;	 // "redundant_binop_in_equality"
    case 't':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "runc_lshr_buildvector_fold", 26) != 0)
        break;
      return 36;	 // "trunc_lshr_buildvector_fold"
    }
    break;
  case 28:	 // 3 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'o':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "pt_brcond_by_inverting_cond", 27) != 0)
        break;
      return 60;	 // "opt_brcond_by_inverting_cond"
    case 'p':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ropagate_undef_shuffle_mask", 27) != 0)
        break;
      return 19;	 // "propagate_undef_shuffle_mask"
    case 's':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "hift_of_shifted_logic_chain", 27) != 0)
        break;
      return 79;	 // "shift_of_shifted_logic_chain"
    }
    break;
  case 29:	 // 2 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'b':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "itfield_extract_from_shr_and", 28) != 0)
        break;
      return 88;	 // "bitfield_extract_from_shr_and"
    case 'i':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "cmp_to_true_false_known_bits", 28) != 0)
        break;
      return 55;	 // "icmp_to_true_false_known_bits"
    }
    break;
  case 32:	 // 3 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'b':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "itfield_extract_from_sext_inreg", 31) != 0)
        break;
      return 85;	 // "bitfield_extract_from_sext_inreg"
    case 'c':	 // 2 strings to match.
      if (memcmp(RuleIdentifier.data()+1, "ombine_f", 8) != 0)
        break;
      switch (RuleIdentifier[9]) {
      default: break;
      case 'a':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+10, "dd_fmul_to_fmad_or_fma", 22) != 0)
          break;
        return 96;	 // "combine_fadd_fmul_to_fmad_or_fma"
      case 's':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+10, "ub_fmul_to_fmad_or_fma", 22) != 0)
          break;
        return 100;	 // "combine_fsub_fmul_to_fmad_or_fma"
      }
      break;
    }
    break;
  case 34:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "extract_all_elts_from_build_vector", 34) != 0)
      break;
    return 7;	 // "extract_all_elts_from_build_vector"
  case 36:	 // 3 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'c':	 // 2 strings to match.
      if (memcmp(RuleIdentifier.data()+1, "ombine_", 7) != 0)
        break;
      switch (RuleIdentifier[8]) {
      default: break;
      case 'f':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+9, "add_fma_fmul_to_fmad_or_fma", 27) != 0)
          break;
        return 98;	 // "combine_fadd_fma_fmul_to_fmad_or_fma"
      case 'i':	 // 1 string to match.
        if (memcmp(RuleIdentifier.data()+9, "nsert_vec_elts_build_vector", 27) != 0)
          break;
        return 5;	 // "combine_insert_vec_elts_build_vector"
      }
      break;
    case 'i':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "nsert_extract_vec_elt_out_of_bounds", 35) != 0)
        break;
      return 22;	 // "insert_extract_vec_elt_out_of_bounds"
    }
    break;
  case 37:	 // 2 strings to match.
    switch (RuleIdentifier[0]) {
    default: break;
    case 'c':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "ombine_fsub_fneg_fmul_to_fmad_or_fma", 36) != 0)
        break;
      return 101;	 // "combine_fsub_fneg_fmul_to_fmad_or_fma"
    case 'h':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+1, "oist_logic_op_with_same_opcode_hands", 36) != 0)
        break;
      return 40;	 // "hoist_logic_op_with_same_opcode_hands"
    }
    break;
  case 38:	 // 2 strings to match.
    if (memcmp(RuleIdentifier.data()+0, "combine_f", 9) != 0)
      break;
    switch (RuleIdentifier[9]) {
    default: break;
    case 'a':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+10, "dd_fpext_fmul_to_fmad_or_fma", 28) != 0)
        break;
      return 97;	 // "combine_fadd_fpext_fmul_to_fmad_or_fma"
    case 's':	 // 1 string to match.
      if (memcmp(RuleIdentifier.data()+10, "ub_fpext_fmul_to_fmad_or_fma", 28) != 0)
        break;
      return 102;	 // "combine_fsub_fpext_fmul_to_fmad_or_fma"
    }
    break;
  case 42:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "combine_fadd_fpext_fma_fmul_to_fmad_or_fma", 42) != 0)
      break;
    return 99;	 // "combine_fadd_fpext_fma_fmul_to_fmad_or_fma"
  case 43:	 // 1 string to match.
    if (memcmp(RuleIdentifier.data()+0, "combine_fsub_fpext_fneg_fmul_to_fmad_or_fma", 43) != 0)
      break;
    return 103;	 // "combine_fsub_fpext_fneg_fmul_to_fmad_or_fma"
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
    return {{0, 113}};
  }
  const auto I = getRuleIdxForIdentifier(RangePair.first);
  if (!I)
    return std::nullopt;
  return {{*I, *I + 1}};
}

bool AArch64GenPreLegalizerCombinerHelperRuleConfig::setRuleEnabled(StringRef RuleIdentifier) {
  auto MaybeRange = getRuleRangeForIdentifier(RuleIdentifier);
  if (!MaybeRange)
    return false;
  for (auto I = MaybeRange->first; I < MaybeRange->second; ++I)
    DisabledRules.reset(I);
  return true;
}

bool AArch64GenPreLegalizerCombinerHelperRuleConfig::setRuleDisabled(StringRef RuleIdentifier) {
  auto MaybeRange = getRuleRangeForIdentifier(RuleIdentifier);
  if (!MaybeRange)
    return false;
  for (auto I = MaybeRange->first; I < MaybeRange->second; ++I)
    DisabledRules.set(I);
  return true;
}

bool AArch64GenPreLegalizerCombinerHelperRuleConfig::isRuleDisabled(unsigned RuleID) const {
  return DisabledRules.test(RuleID);
}
#endif // ifdef AARCH64PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_H

#ifdef AARCH64PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_CPP

std::vector<std::string> AArch64PreLegalizerCombinerHelperOption;
cl::list<std::string> AArch64PreLegalizerCombinerHelperDisableOption(
    "aarch64prelegalizercombinerhelper-disable-rule",
    cl::desc("Disable one or more combiner rules temporarily in the AArch64PreLegalizerCombinerHelper pass"),
    cl::CommaSeparated,
    cl::Hidden,
    cl::cat(GICombinerOptionCategory),
    cl::callback([](const std::string &Str) {
      AArch64PreLegalizerCombinerHelperOption.push_back(Str);
    }));
cl::list<std::string> AArch64PreLegalizerCombinerHelperOnlyEnableOption(
    "aarch64prelegalizercombinerhelper-only-enable-rule",
    cl::desc("Disable all rules in the AArch64PreLegalizerCombinerHelper pass then re-enable the specified ones"),
    cl::Hidden,
    cl::cat(GICombinerOptionCategory),
    cl::callback([](const std::string &CommaSeparatedArg) {
      StringRef Str = CommaSeparatedArg;
      AArch64PreLegalizerCombinerHelperOption.push_back("*");
      do {
        auto X = Str.split(",");
        AArch64PreLegalizerCombinerHelperOption.push_back(("!" + X.first).str());
        Str = X.second;
      } while (!Str.empty());
    }));

bool AArch64GenPreLegalizerCombinerHelperRuleConfig::parseCommandLineOption() {
  for (StringRef Identifier : AArch64PreLegalizerCombinerHelperOption) {
    bool Enabled = Identifier.consume_front("!");
    if (Enabled && !setRuleEnabled(Identifier))
      return false;
    if (!Enabled && !setRuleDisabled(Identifier))
      return false;
  }
  return true;
}

bool AArch64GenPreLegalizerCombinerHelper::tryCombineAll(
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
  SmallVector<Register, 4> MatchData5;
  Register MatchData6;
  SmallVector<std::pair<Register, MachineInstr*>> MatchData7;
  PreferredTuple MatchData8;
  std::function<void(MachineIRBuilder &)> MatchData9;
  IndexedLoadStoreMatchInfo MatchData10;
  std::function<void(MachineIRBuilder &)> MatchData21;
  Register MatchData28;
  Register MatchData29;
  Register MatchData30;
  Register MatchData31;
  Register MatchData33;
  Register MatchData34;
  Register MatchData35;
  Register MatchData36;
  MachineInstr* MatchData38;
  std::tuple<Register, Register> MatchData39;
  InstructionStepsMatchInfo MatchData40;
  std::function<void(MachineIRBuilder &)> MatchData41;
  PtrAddChain MatchData42;
  std::tuple<Register, int64_t> MatchData43;
  std::tuple<Register, unsigned> MatchData44;
  RegisterImmPair MatchData45;
  std::function<void(MachineIRBuilder &)> MatchData46;
  unsigned MatchData48;
  std::function<void(MachineIRBuilder &)> MatchData49;
  Register MatchData50;
  Register MatchData52;
  Register MatchData54;
  int64_t MatchData55;
  std::function<void(MachineIRBuilder &)> MatchData56;
  std::tuple<Register, unsigned> MatchData58;
  SmallVector<Register, 4> MatchData59;
  MachineInstr * MatchData60;
  SmallVector<Register, 8> MatchData61;
  SmallVector<APInt, 8> MatchData62;
  Register MatchData65;
  std::pair<Register, unsigned> MatchData66;
  std::pair<MachineInstr*, LLT> MatchData67;
  std::optional<APFloat> MatchData68;
  APInt MatchData69;
  std::function<void(MachineIRBuilder &)> MatchData70;
  std::function<void(MachineIRBuilder &)> MatchData71;
  std::function<void(MachineIRBuilder &)> MatchData72;
  std::function<void(MachineIRBuilder &)> MatchData73;
  std::function<void(MachineIRBuilder &)> MatchData74;
  unsigned MatchData75;
  std::pair<Register, Register> MatchData76;
  RegisterImmPair MatchData78;
  ShiftOfShiftedLogic MatchData79;
  std::function<void(MachineIRBuilder &)> MatchData80;
  MergeTruncStoresInfo MatchData81;
  MachineInstr * MatchData82;
  std::function<void(MachineIRBuilder &)> MatchData83;
  std::function<void(MachineIRBuilder &)> MatchData85;
  std::function<void(MachineIRBuilder &)> MatchData86;
  std::function<void(MachineIRBuilder &)> MatchData87;
  std::function<void(MachineIRBuilder &)> MatchData88;
  APInt MatchData89;
  std::function<void(MachineIRBuilder &)> MatchData90;
  std::function<void(MachineIRBuilder &)> MatchData94;
  std::function<void(MachineIRBuilder &)> MatchData95;
  std::function<void(MachineIRBuilder &)> MatchData96;
  std::function<void(MachineIRBuilder &)> MatchData97;
  std::function<void(MachineIRBuilder &)> MatchData98;
  std::function<void(MachineIRBuilder &)> MatchData99;
  std::function<void(MachineIRBuilder &)> MatchData100;
  std::function<void(MachineIRBuilder &)> MatchData101;
  std::function<void(MachineIRBuilder &)> MatchData102;
  std::function<void(MachineIRBuilder &)> MatchData103;
  unsigned MatchData104;
  std::function<void(MachineIRBuilder &)> MatchData105;
  std::function<void(MachineIRBuilder &)> MatchData106;
  std::function<void(MachineIRBuilder &)> MatchData107;
  Register MatchData108;
  Register MatchData111;
  std::pair<uint64_t, uint64_t> MatchData112;

  int Partition = -1;
  Partition = -1;
  switch (MIs[0]->getOpcode()) {
  case TargetOpcode::COPY: Partition = 0; break;
  case TargetOpcode::G_MUL: Partition = 1; break;
  case TargetOpcode::G_ADD: Partition = 2; break;
  case TargetOpcode::G_FREEZE: Partition = 3; break;
  case TargetOpcode::G_FABS: Partition = 4; break;
  case TargetOpcode::G_FCANONICALIZE: Partition = 5; break;
  case TargetOpcode::G_INSERT_VECTOR_ELT: Partition = 6; break;
  case TargetOpcode::G_EXTRACT_VECTOR_ELT: Partition = 7; break;
  case TargetOpcode::G_BUILD_VECTOR: Partition = 8; break;
  case TargetOpcode::G_LOAD: Partition = 9; break;
  case TargetOpcode::G_SEXTLOAD: Partition = 10; break;
  case TargetOpcode::G_ZEXTLOAD: Partition = 11; break;
  case TargetOpcode::G_AND: Partition = 12; break;
  case TargetOpcode::G_STORE: Partition = 13; break;
  case TargetOpcode::G_UITOFP: Partition = 14; break;
  case TargetOpcode::G_SITOFP: Partition = 15; break;
  case TargetOpcode::G_OR: Partition = 16; break;
  case TargetOpcode::G_SHL: Partition = 17; break;
  case TargetOpcode::G_UDIV: Partition = 18; break;
  case TargetOpcode::G_UREM: Partition = 19; break;
  case TargetOpcode::G_ASHR: Partition = 20; break;
  case TargetOpcode::G_LSHR: Partition = 21; break;
  case TargetOpcode::G_ABS: Partition = 22; break;
  case TargetOpcode::G_FPTOSI: Partition = 23; break;
  case TargetOpcode::G_FPTOUI: Partition = 24; break;
  case TargetOpcode::G_SUB: Partition = 25; break;
  case TargetOpcode::G_XOR: Partition = 26; break;
  case TargetOpcode::G_TRUNC: Partition = 27; break;
  case TargetOpcode::G_SHUFFLE_VECTOR: Partition = 28; break;
  case TargetOpcode::G_UNMERGE_VALUES: Partition = 29; break;
  case TargetOpcode::G_SELECT: Partition = 30; break;
  case TargetOpcode::G_PTR_ADD: Partition = 31; break;
  case TargetOpcode::G_ROTL: Partition = 32; break;
  case TargetOpcode::G_ROTR: Partition = 33; break;
  case TargetOpcode::G_SDIV: Partition = 34; break;
  case TargetOpcode::G_SREM: Partition = 35; break;
  case TargetOpcode::G_INTTOPTR: Partition = 36; break;
  case TargetOpcode::G_PTRTOINT: Partition = 37; break;
  case TargetOpcode::G_ANYEXT: Partition = 38; break;
  case TargetOpcode::G_FNEG: Partition = 39; break;
  case TargetOpcode::G_BUILD_VECTOR_TRUNC: Partition = 40; break;
  case TargetOpcode::G_BITCAST: Partition = 41; break;
  case TargetOpcode::G_PHI: Partition = 42; break;
  case TargetOpcode::G_SEXT_INREG: Partition = 43; break;
  case TargetOpcode::G_ZEXT: Partition = 44; break;
  case TargetOpcode::G_ICMP: Partition = 45; break;
  case TargetOpcode::G_SEXT: Partition = 46; break;
  case TargetOpcode::G_BR: Partition = 47; break;
  case TargetOpcode::G_MERGE_VALUES: Partition = 48; break;
  case TargetOpcode::G_FPTRUNC: Partition = 49; break;
  case TargetOpcode::G_FSQRT: Partition = 50; break;
  case TargetOpcode::G_FLOG2: Partition = 51; break;
  case TargetOpcode::G_UMULO: Partition = 52; break;
  case TargetOpcode::G_SMULO: Partition = 53; break;
  case TargetOpcode::G_UADDO: Partition = 54; break;
  case TargetOpcode::G_SADDO: Partition = 55; break;
  case TargetOpcode::G_UADDE: Partition = 56; break;
  case TargetOpcode::G_SADDE: Partition = 57; break;
  case TargetOpcode::G_USUBE: Partition = 58; break;
  case TargetOpcode::G_SSUBE: Partition = 59; break;
  case TargetOpcode::G_FMINNUM: Partition = 60; break;
  case TargetOpcode::G_FMAXNUM: Partition = 61; break;
  case TargetOpcode::G_FMINIMUM: Partition = 62; break;
  case TargetOpcode::G_FMAXIMUM: Partition = 63; break;
  case TargetOpcode::G_SSHLSAT: Partition = 64; break;
  case TargetOpcode::G_USHLSAT: Partition = 65; break;
  case TargetOpcode::G_FSHL: Partition = 66; break;
  case TargetOpcode::G_FSHR: Partition = 67; break;
  case TargetOpcode::G_UMULH: Partition = 68; break;
  case TargetOpcode::G_FADD: Partition = 69; break;
  case TargetOpcode::G_FSUB: Partition = 70; break;
  case TargetOpcode::G_FMUL: Partition = 71; break;
  case TargetOpcode::G_FDIV: Partition = 72; break;
  case TargetOpcode::G_FMAD: Partition = 73; break;
  case TargetOpcode::G_FMA: Partition = 74; break;
  case TargetOpcode::G_SMIN: Partition = 75; break;
  case TargetOpcode::G_SMAX: Partition = 76; break;
  case TargetOpcode::G_UMIN: Partition = 77; break;
  case TargetOpcode::G_UMAX: Partition = 78; break;
  case TargetOpcode::G_FREM: Partition = 79; break;
  case TargetOpcode::G_FCONSTANT: Partition = 80; break;
  case TargetOpcode::G_GLOBAL_VALUE: Partition = 81; break;
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
    // Leaf name: undef_to_int_zero
    // Rule: undef_to_int_zero
    if (!RuleConfig->isRuleDisabled(12)) {
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
    // Leaf name: binop_right_to_zero
    // Rule: binop_right_to_zero
    if (!RuleConfig->isRuleDisabled(27)) {
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
    if (!RuleConfig->isRuleDisabled(32)) {
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
    // Leaf name: constant_fold
    // Rule: constant_fold
    if (!RuleConfig->isRuleDisabled(89)) {
      if (1
          && [&]() {
           return Helper.matchConstantFold(*MIs[0], MatchData89); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'constant_fold'\n");
 Helper.replaceInstWithConstant(*MIs[0], MatchData89); 
        return true;
      }
    }
    // Leaf name: commute_constant_to_rhs
    // Rule: commute_constant_to_rhs
    if (!RuleConfig->isRuleDisabled(109)) {
      if (1
          && [&]() {
          
        return getIConstantVRegVal(MIs[0]->getOperand(1).getReg(), MRI).has_value();
      
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'commute_constant_to_rhs'\n");

        Observer.changingInstr(*MIs[0]);
        Register LHSReg = MIs[0]->getOperand(1).getReg();
        Register RHSReg = MIs[0]->getOperand(2).getReg();
        MIs[0]->getOperand(1).setReg(RHSReg);
        MIs[0]->getOperand(2).setReg(LHSReg);
        Observer.changedInstr(*MIs[0]);
      
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
    // Leaf name: propagate_undef_any_op
    // Rule: propagate_undef_any_op
    if (!RuleConfig->isRuleDisabled(17)) {
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
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(24)) {
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
    if (!RuleConfig->isRuleDisabled(33)) {
      if (1
          && [&]() {
           return Helper.matchAddSubSameReg(*MIs[0], MatchData33); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'add_sub_reg'\n");
 return Helper.replaceSingleDefInstWithReg(*MIs[0],
                                                          MatchData33); 
        return true;
      }
    }
    // Leaf name: simplify_add_to_sub
    // Rule: simplify_add_to_sub
    if (!RuleConfig->isRuleDisabled(39)) {
      if (1
          && [&]() {
           return Helper.matchSimplifyAddToSub(*MIs[0], MatchData39); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'simplify_add_to_sub'\n");
 Helper.applySimplifyAddToSub(*MIs[0], MatchData39);
        return true;
      }
    }
    // Leaf name: constant_fold
    // Rule: constant_fold
    if (!RuleConfig->isRuleDisabled(89)) {
      if (1
          && [&]() {
           return Helper.matchConstantFold(*MIs[0], MatchData89); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'constant_fold'\n");
 Helper.replaceInstWithConstant(*MIs[0], MatchData89); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    // Leaf name: commute_constant_to_rhs
    // Rule: commute_constant_to_rhs
    if (!RuleConfig->isRuleDisabled(109)) {
      if (1
          && [&]() {
          
        return getIConstantVRegVal(MIs[0]->getOperand(1).getReg(), MRI).has_value();
      
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'commute_constant_to_rhs'\n");

        Observer.changingInstr(*MIs[0]);
        Register LHSReg = MIs[0]->getOperand(1).getReg();
        Register RHSReg = MIs[0]->getOperand(2).getReg();
        MIs[0]->getOperand(1).setReg(RHSReg);
        MIs[0]->getOperand(2).setReg(LHSReg);
        Observer.changedInstr(*MIs[0]);
      
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
    // Leaf name: constant_fp_op
    // Rule: constant_fp_op
    if (!RuleConfig->isRuleDisabled(68)) {
      if (1
          && [&]() {
           return Helper.matchCombineConstantFoldFpUnary(*MIs[0], MatchData68); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'constant_fp_op'\n");
 Helper.applyCombineConstantFoldFpUnary(*MIs[0], MatchData68); 
        return true;
      }
    }
    // Leaf name: fabs_fneg_fold
    // Rule: fabs_fneg_fold
    if (!RuleConfig->isRuleDisabled(90)) {
      if (1
          && [&]() {
           return Helper.matchCombineFAbsOfFNeg(*MIs[0], MatchData90); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fabs_fneg_fold'\n");
 Helper.applyBuildFnNoErase(*MIs[0], MatchData90); 
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
  if (Partition == 6 /* TargetOpcode::G_INSERT_VECTOR_ELT */) {
    // Leaf name: combine_insert_vec_elts_build_vector
    // Rule: combine_insert_vec_elts_build_vector
    if (!RuleConfig->isRuleDisabled(5)) {
      if (1
          && [&]() {
           return Helper.matchCombineInsertVecElts(*MIs[0], MatchData5); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_insert_vec_elts_build_vector'\n");
 Helper.applyCombineInsertVecElts(*MIs[0], MatchData5); 
        return true;
      }
    }
    // Leaf name: insert_extract_vec_elt_out_of_bounds
    // Rule: insert_extract_vec_elt_out_of_bounds
    if (!RuleConfig->isRuleDisabled(22)) {
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
  if (Partition == 7 /* TargetOpcode::G_EXTRACT_VECTOR_ELT */) {
    // Leaf name: extract_vec_elt_build_vec
    // Rule: extract_vec_elt_build_vec
    if (!RuleConfig->isRuleDisabled(6)) {
      if (1
          && [&]() {
           return Helper.matchExtractVecEltBuildVec(*MIs[0], MatchData6); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'extract_vec_elt_build_vec'\n");
 Helper.applyExtractVecEltBuildVec(*MIs[0], MatchData6); 
        return true;
      }
    }
    // Leaf name: insert_extract_vec_elt_out_of_bounds
    // Rule: insert_extract_vec_elt_out_of_bounds
    if (!RuleConfig->isRuleDisabled(22)) {
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
  if (Partition == 8 /* TargetOpcode::G_BUILD_VECTOR */) {
    // Leaf name: extract_all_elts_from_build_vector
    // Rule: extract_all_elts_from_build_vector
    if (!RuleConfig->isRuleDisabled(7)) {
      if (1
          && [&]() {
           return Helper.matchExtractAllEltsFromBuildVector(*MIs[0], MatchData7); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'extract_all_elts_from_build_vector'\n");
 Helper.applyExtractAllEltsFromBuildVector(*MIs[0], MatchData7); 
        return true;
      }
    }
    // Leaf name: buildvector_identity_fold
    // Rule: buildvector_identity_fold
    if (!RuleConfig->isRuleDisabled(34)) {
      if (1
          && [&]() {
           return Helper.matchBuildVectorIdentityFold(*MIs[0], MatchData34); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'buildvector_identity_fold'\n");
 Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData34); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 9 /* TargetOpcode::G_LOAD */) {
    // Leaf name: extending_loads
    // Rule: extending_loads
    if (!RuleConfig->isRuleDisabled(8)) {
      if (1
          && [&]() {
           return Helper.matchCombineExtendingLoads(*MIs[0], MatchData8); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'extending_loads'\n");
 Helper.applyCombineExtendingLoads(*MIs[0], MatchData8); 
        return true;
      }
    }
    // Leaf name: combine_indexed_load_store
    // Rule: combine_indexed_load_store
    if (!RuleConfig->isRuleDisabled(10)) {
      if (1
          && [&]() {
           return Helper.matchCombineIndexedLoadStore(*MIs[0], MatchData10); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_indexed_load_store'\n");
 Helper.applyCombineIndexedLoadStore(*MIs[0], MatchData10); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 10 /* TargetOpcode::G_SEXTLOAD */) {
    // Leaf name: extending_loads
    // Rule: extending_loads
    if (!RuleConfig->isRuleDisabled(8)) {
      if (1
          && [&]() {
           return Helper.matchCombineExtendingLoads(*MIs[0], MatchData8); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'extending_loads'\n");
 Helper.applyCombineExtendingLoads(*MIs[0], MatchData8); 
        return true;
      }
    }
    // Leaf name: combine_indexed_load_store
    // Rule: combine_indexed_load_store
    if (!RuleConfig->isRuleDisabled(10)) {
      if (1
          && [&]() {
           return Helper.matchCombineIndexedLoadStore(*MIs[0], MatchData10); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_indexed_load_store'\n");
 Helper.applyCombineIndexedLoadStore(*MIs[0], MatchData10); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 11 /* TargetOpcode::G_ZEXTLOAD */) {
    // Leaf name: extending_loads
    // Rule: extending_loads
    if (!RuleConfig->isRuleDisabled(8)) {
      if (1
          && [&]() {
           return Helper.matchCombineExtendingLoads(*MIs[0], MatchData8); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'extending_loads'\n");
 Helper.applyCombineExtendingLoads(*MIs[0], MatchData8); 
        return true;
      }
    }
    // Leaf name: combine_indexed_load_store
    // Rule: combine_indexed_load_store
    if (!RuleConfig->isRuleDisabled(10)) {
      if (1
          && [&]() {
           return Helper.matchCombineIndexedLoadStore(*MIs[0], MatchData10); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_indexed_load_store'\n");
 Helper.applyCombineIndexedLoadStore(*MIs[0], MatchData10); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 12 /* TargetOpcode::G_AND */) {
    // Leaf name: load_and_mask
    // Rule: load_and_mask
    if (!RuleConfig->isRuleDisabled(9)) {
      if (1
          && [&]() {
           return Helper.matchCombineLoadWithAndMask(*MIs[0], MatchData9); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'load_and_mask'\n");
 Helper.applyBuildFn(*MIs[0], MatchData9); 
        return true;
      }
    }
    // Leaf name: undef_to_int_zero
    // Rule: undef_to_int_zero
    if (!RuleConfig->isRuleDisabled(12)) {
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
    // Leaf name: binop_same_val
    // Rule: binop_same_val
    if (!RuleConfig->isRuleDisabled(25)) {
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
    // Leaf name: hoist_logic_op_with_same_opcode_hands
    // Rule: hoist_logic_op_with_same_opcode_hands
    if (!RuleConfig->isRuleDisabled(40)) {
      if (1
          && [&]() {
           return Helper.matchHoistLogicOpWithSameOpcodeHands(*MIs[0], MatchData40); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'hoist_logic_op_with_same_opcode_hands'\n");
 Helper.applyBuildInstructionSteps(*MIs[0], MatchData40);
        return true;
      }
    }
    // Leaf name: narrow_binop_feeding_and
    // Rule: narrow_binop_feeding_and
    if (!RuleConfig->isRuleDisabled(46)) {
      if (1
          && [&]() {
           return Helper.matchNarrowBinopFeedingAnd(*MIs[0], MatchData46); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'narrow_binop_feeding_and'\n");
 Helper.applyBuildFnNoErase(*MIs[0], MatchData46); 
        return true;
      }
    }
    // Leaf name: redundant_and
    // Rule: redundant_and
    if (!RuleConfig->isRuleDisabled(50)) {
      if (1
          && [&]() {
           return Helper.matchRedundantAnd(*MIs[0], MatchData50); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'redundant_and'\n");
 return Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData50); 
        return true;
      }
    }
    // Leaf name: overlapping_and
    // Rule: overlapping_and
    if (!RuleConfig->isRuleDisabled(70)) {
      if (1
          && [&]() {
           return Helper.matchOverlappingAnd(*MIs[0], MatchData70); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'overlapping_and'\n");
 Helper.applyBuildFn(*MIs[0], MatchData70); 
        return true;
      }
    }
    // Leaf name: bitfield_extract_from_and
    // Rule: bitfield_extract_from_and
    if (!RuleConfig->isRuleDisabled(86)) {
      if (1
          && [&]() {
           return Helper.matchBitfieldExtractFromAnd(*MIs[0], MatchData86); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'bitfield_extract_from_and'\n");
 Helper.applyBuildFn(*MIs[0], MatchData86); 
        return true;
      }
    }
    // Leaf name: constant_fold
    // Rule: constant_fold
    if (!RuleConfig->isRuleDisabled(89)) {
      if (1
          && [&]() {
           return Helper.matchConstantFold(*MIs[0], MatchData89); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'constant_fold'\n");
 Helper.replaceInstWithConstant(*MIs[0], MatchData89); 
        return true;
      }
    }
    // Leaf name: and_or_disjoint_mask
    // Rule: and_or_disjoint_mask
    if (!RuleConfig->isRuleDisabled(95)) {
      if (1
          && [&]() {
           return Helper.matchAndOrDisjointMask(*MIs[0], MatchData95); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'and_or_disjoint_mask'\n");
 Helper.applyBuildFnNoErase(*MIs[0], MatchData95); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    // Leaf name: commute_constant_to_rhs
    // Rule: commute_constant_to_rhs
    if (!RuleConfig->isRuleDisabled(109)) {
      if (1
          && [&]() {
          
        return getIConstantVRegVal(MIs[0]->getOperand(1).getReg(), MRI).has_value();
      
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'commute_constant_to_rhs'\n");

        Observer.changingInstr(*MIs[0]);
        Register LHSReg = MIs[0]->getOperand(1).getReg();
        Register RHSReg = MIs[0]->getOperand(2).getReg();
        MIs[0]->getOperand(1).setReg(RHSReg);
        MIs[0]->getOperand(2).setReg(LHSReg);
        Observer.changedInstr(*MIs[0]);
      
        return true;
      }
    }
    return false;
  }
  if (Partition == 13 /* TargetOpcode::G_STORE */) {
    // Leaf name: combine_indexed_load_store
    // Rule: combine_indexed_load_store
    if (!RuleConfig->isRuleDisabled(10)) {
      if (1
          && [&]() {
           return Helper.matchCombineIndexedLoadStore(*MIs[0], MatchData10); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_indexed_load_store'\n");
 Helper.applyCombineIndexedLoadStore(*MIs[0], MatchData10); 
        return true;
      }
    }
    // Leaf name: erase_undef_store
    // Rule: erase_undef_store
    if (!RuleConfig->isRuleDisabled(20)) {
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
    // Leaf name: truncstore_merge
    // Rule: truncstore_merge
    if (!RuleConfig->isRuleDisabled(81)) {
      if (1
          && [&]() {
           return Helper.matchTruncStoreMerge(*MIs[0], MatchData81); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'truncstore_merge'\n");
 Helper.applyTruncStoreMerge(*MIs[0], MatchData81); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 14 /* TargetOpcode::G_UITOFP */) {
    // Leaf name: undef_to_fp_zero
    // Rule: undef_to_fp_zero
    if (!RuleConfig->isRuleDisabled(11)) {
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
  if (Partition == 15 /* TargetOpcode::G_SITOFP */) {
    // Leaf name: undef_to_fp_zero
    // Rule: undef_to_fp_zero
    if (!RuleConfig->isRuleDisabled(11)) {
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
  if (Partition == 16 /* TargetOpcode::G_OR */) {
    // Leaf name: undef_to_negative_one
    // Rule: undef_to_negative_one
    if (!RuleConfig->isRuleDisabled(13)) {
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
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(24)) {
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
    if (!RuleConfig->isRuleDisabled(25)) {
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
    // Leaf name: hoist_logic_op_with_same_opcode_hands
    // Rule: hoist_logic_op_with_same_opcode_hands
    if (!RuleConfig->isRuleDisabled(40)) {
      if (1
          && [&]() {
           return Helper.matchHoistLogicOpWithSameOpcodeHands(*MIs[0], MatchData40); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'hoist_logic_op_with_same_opcode_hands'\n");
 Helper.applyBuildInstructionSteps(*MIs[0], MatchData40);
        return true;
      }
    }
    // Leaf name: redundant_or
    // Rule: redundant_or
    if (!RuleConfig->isRuleDisabled(52)) {
      if (1
          && [&]() {
           return Helper.matchRedundantOr(*MIs[0], MatchData52); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'redundant_or'\n");
 return Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData52); 
        return true;
      }
    }
    // Leaf name: load_or_combine
    // Rule: load_or_combine
    if (!RuleConfig->isRuleDisabled(80)) {
      if (1
          && [&]() {
           return Helper.matchLoadOrCombine(*MIs[0], MatchData80); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'load_or_combine'\n");
 Helper.applyBuildFn(*MIs[0], MatchData80); 
        return true;
      }
    }
    // Leaf name: funnel_shift_from_or_shift
    // Rule: funnel_shift_from_or_shift
    if (!RuleConfig->isRuleDisabled(83)) {
      if (1
          && [&]() {
           return Helper.matchOrShiftToFunnelShift(*MIs[0], MatchData83); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'funnel_shift_from_or_shift'\n");
 Helper.applyBuildFn(*MIs[0], MatchData83); 
        return true;
      }
    }
    // Leaf name: constant_fold
    // Rule: constant_fold
    if (!RuleConfig->isRuleDisabled(89)) {
      if (1
          && [&]() {
           return Helper.matchConstantFold(*MIs[0], MatchData89); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'constant_fold'\n");
 Helper.replaceInstWithConstant(*MIs[0], MatchData89); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    // Leaf name: commute_constant_to_rhs
    // Rule: commute_constant_to_rhs
    if (!RuleConfig->isRuleDisabled(109)) {
      if (1
          && [&]() {
          
        return getIConstantVRegVal(MIs[0]->getOperand(1).getReg(), MRI).has_value();
      
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'commute_constant_to_rhs'\n");

        Observer.changingInstr(*MIs[0]);
        Register LHSReg = MIs[0]->getOperand(1).getReg();
        Register RHSReg = MIs[0]->getOperand(2).getReg();
        MIs[0]->getOperand(1).setReg(RHSReg);
        MIs[0]->getOperand(2).setReg(LHSReg);
        Observer.changedInstr(*MIs[0]);
      
        return true;
      }
    }
    return false;
  }
  if (Partition == 17 /* TargetOpcode::G_SHL */) {
    // Leaf name: binop_left_undef_to_zero
    // Rule: binop_left_undef_to_zero
    if (!RuleConfig->isRuleDisabled(14)) {
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
    if (!RuleConfig->isRuleDisabled(15)) {
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
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(24)) {
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
    // Leaf name: reduce_shl_of_extend
    // Rule: reduce_shl_of_extend
    if (!RuleConfig->isRuleDisabled(45)) {
      if (1
          && [&]() {
           return Helper.matchCombineShlOfExtend(*MIs[0], MatchData45); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'reduce_shl_of_extend'\n");
 Helper.applyCombineShlOfExtend(*MIs[0], MatchData45); 
        return true;
      }
    }
    // Leaf name: shift_immed_chain
    // Rule: shift_immed_chain
    if (!RuleConfig->isRuleDisabled(78)) {
      if (1
          && [&]() {
           return Helper.matchShiftImmedChain(*MIs[0], MatchData78); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'shift_immed_chain'\n");
 Helper.applyShiftImmedChain(*MIs[0], MatchData78); 
        return true;
      }
    }
    // Leaf name: shift_of_shifted_logic_chain
    // Rule: shift_of_shifted_logic_chain
    if (!RuleConfig->isRuleDisabled(79)) {
      if (1
          && [&]() {
           return Helper.matchShiftOfShiftedLogic(*MIs[0], MatchData79); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'shift_of_shifted_logic_chain'\n");
 Helper.applyShiftOfShiftedLogic(*MIs[0], MatchData79); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 18 /* TargetOpcode::G_UDIV */) {
    // Leaf name: binop_left_undef_to_zero
    // Rule: binop_left_undef_to_zero
    if (!RuleConfig->isRuleDisabled(14)) {
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
    // Leaf name: binop_left_to_zero
    // Rule: binop_left_to_zero
    if (!RuleConfig->isRuleDisabled(26)) {
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
    // Leaf name: div_rem_to_divrem
    // Rule: div_rem_to_divrem
    if (!RuleConfig->isRuleDisabled(82)) {
      if (1
          && [&]() {
           return Helper.matchCombineDivRem(*MIs[0], MatchData82); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'div_rem_to_divrem'\n");
 Helper.applyCombineDivRem(*MIs[0], MatchData82); 
        return true;
      }
    }
    // Leaf name: udiv_by_const
    // Rule: udiv_by_const
    if (!RuleConfig->isRuleDisabled(91)) {
      if (1
          && [&]() {
           return Helper.matchUDivByConst(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'udiv_by_const'\n");
 Helper.applyUDivByConst(*MIs[0]); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 19 /* TargetOpcode::G_UREM */) {
    // Leaf name: binop_left_undef_to_zero
    // Rule: binop_left_undef_to_zero
    if (!RuleConfig->isRuleDisabled(14)) {
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
    // Leaf name: binop_left_to_zero
    // Rule: binop_left_to_zero
    if (!RuleConfig->isRuleDisabled(26)) {
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
    // Leaf name: urem_pow2_to_mask
    // Rule: urem_pow2_to_mask
    if (!RuleConfig->isRuleDisabled(53)) {
      if (1
          && [&]() {
           return Helper.matchOperandIsKnownToBeAPowerOfTwo(*MIs[0], 2); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'urem_pow2_to_mask'\n");
 Helper.applySimplifyURemByPow2(*MIs[0]); 
        return true;
      }
    }
    // Leaf name: div_rem_to_divrem
    // Rule: div_rem_to_divrem
    if (!RuleConfig->isRuleDisabled(82)) {
      if (1
          && [&]() {
           return Helper.matchCombineDivRem(*MIs[0], MatchData82); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'div_rem_to_divrem'\n");
 Helper.applyCombineDivRem(*MIs[0], MatchData82); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 20 /* TargetOpcode::G_ASHR */) {
    // Leaf name: binop_right_undef_to_undef
    // Rule: binop_right_undef_to_undef
    if (!RuleConfig->isRuleDisabled(15)) {
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
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(24)) {
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
    // Leaf name: shl_ashr_to_sext_inreg
    // Rule: shl_ashr_to_sext_inreg
    if (!RuleConfig->isRuleDisabled(43)) {
      if (1
          && [&]() {
           return Helper.matchAshrShlToSextInreg(*MIs[0], MatchData43); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'shl_ashr_to_sext_inreg'\n");
 Helper.applyAshShlToSextInreg(*MIs[0], MatchData43);
        return true;
      }
    }
    // Leaf name: shift_immed_chain
    // Rule: shift_immed_chain
    if (!RuleConfig->isRuleDisabled(78)) {
      if (1
          && [&]() {
           return Helper.matchShiftImmedChain(*MIs[0], MatchData78); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'shift_immed_chain'\n");
 Helper.applyShiftImmedChain(*MIs[0], MatchData78); 
        return true;
      }
    }
    // Leaf name: shift_of_shifted_logic_chain
    // Rule: shift_of_shifted_logic_chain
    if (!RuleConfig->isRuleDisabled(79)) {
      if (1
          && [&]() {
           return Helper.matchShiftOfShiftedLogic(*MIs[0], MatchData79); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'shift_of_shifted_logic_chain'\n");
 Helper.applyShiftOfShiftedLogic(*MIs[0], MatchData79); 
        return true;
      }
    }
    // Leaf name: bitfield_extract_from_shr
    // Rule: bitfield_extract_from_shr
    if (!RuleConfig->isRuleDisabled(87)) {
      if (1
          && [&]() {
           return Helper.matchBitfieldExtractFromShr(*MIs[0], MatchData87); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'bitfield_extract_from_shr'\n");
 Helper.applyBuildFn(*MIs[0], MatchData87); 
        return true;
      }
    }
    // Leaf name: bitfield_extract_from_shr_and
    // Rule: bitfield_extract_from_shr_and
    if (!RuleConfig->isRuleDisabled(88)) {
      if (1
          && [&]() {
           return Helper.matchBitfieldExtractFromShrAnd(*MIs[0], MatchData88); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'bitfield_extract_from_shr_and'\n");
 Helper.applyBuildFn(*MIs[0], MatchData88); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 21 /* TargetOpcode::G_LSHR */) {
    // Leaf name: binop_right_undef_to_undef
    // Rule: binop_right_undef_to_undef
    if (!RuleConfig->isRuleDisabled(15)) {
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
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(24)) {
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
    // Leaf name: shift_immed_chain
    // Rule: shift_immed_chain
    if (!RuleConfig->isRuleDisabled(78)) {
      if (1
          && [&]() {
           return Helper.matchShiftImmedChain(*MIs[0], MatchData78); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'shift_immed_chain'\n");
 Helper.applyShiftImmedChain(*MIs[0], MatchData78); 
        return true;
      }
    }
    // Leaf name: shift_of_shifted_logic_chain
    // Rule: shift_of_shifted_logic_chain
    if (!RuleConfig->isRuleDisabled(79)) {
      if (1
          && [&]() {
           return Helper.matchShiftOfShiftedLogic(*MIs[0], MatchData79); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'shift_of_shifted_logic_chain'\n");
 Helper.applyShiftOfShiftedLogic(*MIs[0], MatchData79); 
        return true;
      }
    }
    // Leaf name: bitfield_extract_from_shr
    // Rule: bitfield_extract_from_shr
    if (!RuleConfig->isRuleDisabled(87)) {
      if (1
          && [&]() {
           return Helper.matchBitfieldExtractFromShr(*MIs[0], MatchData87); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'bitfield_extract_from_shr'\n");
 Helper.applyBuildFn(*MIs[0], MatchData87); 
        return true;
      }
    }
    // Leaf name: bitfield_extract_from_shr_and
    // Rule: bitfield_extract_from_shr_and
    if (!RuleConfig->isRuleDisabled(88)) {
      if (1
          && [&]() {
           return Helper.matchBitfieldExtractFromShrAnd(*MIs[0], MatchData88); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'bitfield_extract_from_shr_and'\n");
 Helper.applyBuildFn(*MIs[0], MatchData88); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 22 /* TargetOpcode::G_ABS */) {
    // Leaf name: unary_undef_to_zero
    // Rule: unary_undef_to_zero
    if (!RuleConfig->isRuleDisabled(16)) {
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
  if (Partition == 23 /* TargetOpcode::G_FPTOSI */) {
    // Leaf name: propagate_undef_any_op
    // Rule: propagate_undef_any_op
    if (!RuleConfig->isRuleDisabled(17)) {
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
  if (Partition == 24 /* TargetOpcode::G_FPTOUI */) {
    // Leaf name: propagate_undef_any_op
    // Rule: propagate_undef_any_op
    if (!RuleConfig->isRuleDisabled(17)) {
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
  if (Partition == 25 /* TargetOpcode::G_SUB */) {
    // Leaf name: propagate_undef_any_op
    // Rule: propagate_undef_any_op
    if (!RuleConfig->isRuleDisabled(17)) {
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
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(24)) {
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
    // Leaf name: constant_fold
    // Rule: constant_fold
    if (!RuleConfig->isRuleDisabled(89)) {
      if (1
          && [&]() {
           return Helper.matchConstantFold(*MIs[0], MatchData89); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'constant_fold'\n");
 Helper.replaceInstWithConstant(*MIs[0], MatchData89); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    // Leaf name: sub_add_reg
    // Rule: sub_add_reg
    if (!RuleConfig->isRuleDisabled(105)) {
      if (1
          && [&]() {
           return Helper.matchSubAddSameReg(*MIs[0], MatchData105); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'sub_add_reg'\n");
 Helper.applyBuildFn(*MIs[0], MatchData105); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 26 /* TargetOpcode::G_XOR */) {
    // Leaf name: propagate_undef_any_op
    // Rule: propagate_undef_any_op
    if (!RuleConfig->isRuleDisabled(17)) {
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
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(24)) {
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
    // Leaf name: hoist_logic_op_with_same_opcode_hands
    // Rule: hoist_logic_op_with_same_opcode_hands
    if (!RuleConfig->isRuleDisabled(40)) {
      if (1
          && [&]() {
           return Helper.matchHoistLogicOpWithSameOpcodeHands(*MIs[0], MatchData40); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'hoist_logic_op_with_same_opcode_hands'\n");
 Helper.applyBuildInstructionSteps(*MIs[0], MatchData40);
        return true;
      }
    }
    // Leaf name: not_cmp_fold
    // Rule: not_cmp_fold
    if (!RuleConfig->isRuleDisabled(59)) {
      if (1
          && [&]() {
           return Helper.matchNotCmp(*MIs[0], MatchData59); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'not_cmp_fold'\n");
 Helper.applyNotCmp(*MIs[0], MatchData59); 
        return true;
      }
    }
    // Leaf name: xor_of_and_with_same_reg
    // Rule: xor_of_and_with_same_reg
    if (!RuleConfig->isRuleDisabled(76)) {
      if (1
          && [&]() {
           return Helper.matchXorOfAndWithSameReg(*MIs[0], MatchData76); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'xor_of_and_with_same_reg'\n");
 Helper.applyXorOfAndWithSameReg(*MIs[0], MatchData76); 
        return true;
      }
    }
    // Leaf name: constant_fold
    // Rule: constant_fold
    if (!RuleConfig->isRuleDisabled(89)) {
      if (1
          && [&]() {
           return Helper.matchConstantFold(*MIs[0], MatchData89); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'constant_fold'\n");
 Helper.replaceInstWithConstant(*MIs[0], MatchData89); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    // Leaf name: commute_constant_to_rhs
    // Rule: commute_constant_to_rhs
    if (!RuleConfig->isRuleDisabled(109)) {
      if (1
          && [&]() {
          
        return getIConstantVRegVal(MIs[0]->getOperand(1).getReg(), MRI).has_value();
      
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'commute_constant_to_rhs'\n");

        Observer.changingInstr(*MIs[0]);
        Register LHSReg = MIs[0]->getOperand(1).getReg();
        Register RHSReg = MIs[0]->getOperand(2).getReg();
        MIs[0]->getOperand(1).setReg(RHSReg);
        MIs[0]->getOperand(2).setReg(LHSReg);
        Observer.changedInstr(*MIs[0]);
      
        return true;
      }
    }
    return false;
  }
  if (Partition == 27 /* TargetOpcode::G_TRUNC */) {
    // Leaf name: propagate_undef_any_op
    // Rule: propagate_undef_any_op
    if (!RuleConfig->isRuleDisabled(17)) {
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
    // Leaf name: trunc_buildvector_fold
    // Rule: trunc_buildvector_fold
    if (!RuleConfig->isRuleDisabled(35)) {
      if (1
          && [&]() {
           return Helper.matchTruncBuildVectorFold(*MIs[0], MatchData35); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'trunc_buildvector_fold'\n");
 Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData35); 
        return true;
      }
    }
    // Leaf name: trunc_lshr_buildvector_fold
    // Rule: trunc_lshr_buildvector_fold
    if (!RuleConfig->isRuleDisabled(36)) {
      if (1
          && [&]() {
           return Helper.matchTruncLshrBuildVectorFold(*MIs[0], MatchData36); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'trunc_lshr_buildvector_fold'\n");
 Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData36); 
        return true;
      }
    }
    // Leaf name: trunc_ext_fold
    // Rule: trunc_ext_fold
    if (!RuleConfig->isRuleDisabled(66)) {
      if (1
          && [&]() {
           return Helper.matchCombineTruncOfExt(*MIs[0], MatchData66); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'trunc_ext_fold'\n");
 Helper.applyCombineTruncOfExt(*MIs[0], MatchData66); 
        return true;
      }
    }
    // Leaf name: trunc_shift
    // Rule: trunc_shift
    if (!RuleConfig->isRuleDisabled(67)) {
      if (1
          && [&]() {
           return Helper.matchCombineTruncOfShift(*MIs[0], MatchData67); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'trunc_shift'\n");
 Helper.applyCombineTruncOfShift(*MIs[0], MatchData67); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 28 /* TargetOpcode::G_SHUFFLE_VECTOR */) {
    // Leaf name: propagate_undef_all_ops
    // Rule: propagate_undef_all_ops
    if (!RuleConfig->isRuleDisabled(18)) {
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
    if (!RuleConfig->isRuleDisabled(19)) {
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
  if (Partition == 29 /* TargetOpcode::G_UNMERGE_VALUES */) {
    // Leaf name: unmerge_undef
    // Rule: unmerge_undef
    if (!RuleConfig->isRuleDisabled(21)) {
      if (1
          && [&]() {
           return Helper.matchCombineUnmergeUndef(*MIs[0], MatchData21); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'unmerge_undef'\n");
 Helper.applyBuildFn(*MIs[0], MatchData21); 
        return true;
      }
    }
    // Leaf name: unmerge_merge
    // Rule: unmerge_merge
    if (!RuleConfig->isRuleDisabled(61)) {
      if (1
          && [&]() {
           return Helper.matchCombineUnmergeMergeToPlainValues(*MIs[0], MatchData61); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'unmerge_merge'\n");
 Helper.applyCombineUnmergeMergeToPlainValues(*MIs[0], MatchData61); 
        return true;
      }
    }
    // Leaf name: unmerge_cst
    // Rule: unmerge_cst
    if (!RuleConfig->isRuleDisabled(62)) {
      if (1
          && [&]() {
           return Helper.matchCombineUnmergeConstant(*MIs[0], MatchData62); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'unmerge_cst'\n");
 Helper.applyCombineUnmergeConstant(*MIs[0], MatchData62); 
        return true;
      }
    }
    // Leaf name: unmerge_dead_to_trunc
    // Rule: unmerge_dead_to_trunc
    if (!RuleConfig->isRuleDisabled(63)) {
      if (1
          && [&]() {
           return Helper.matchCombineUnmergeWithDeadLanesToTrunc(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'unmerge_dead_to_trunc'\n");
 Helper.applyCombineUnmergeWithDeadLanesToTrunc(*MIs[0]); 
        return true;
      }
    }
    // Leaf name: unmerge_zext_to_zext
    // Rule: unmerge_zext_to_zext
    if (!RuleConfig->isRuleDisabled(64)) {
      if (1
          && [&]() {
           return Helper.matchCombineUnmergeZExtToZExt(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'unmerge_zext_to_zext'\n");
 Helper.applyCombineUnmergeZExtToZExt(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 30 /* TargetOpcode::G_SELECT */) {
    // Leaf name: select_same_val
    // Rule: select_same_val
    if (!RuleConfig->isRuleDisabled(23)) {
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
    // Leaf name: select_undef_cmp
    // Rule: select_undef_cmp
    if (!RuleConfig->isRuleDisabled(47)) {
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
    if (!RuleConfig->isRuleDisabled(48)) {
      if (1
          && [&]() {
           return Helper.matchConstantSelectCmp(*MIs[0], MatchData48); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'select_constant_cmp'\n");
 return Helper.replaceSingleDefInstWithOperand(*MIs[0], MatchData48); 
        return true;
      }
    }
    // Leaf name: select_to_logical
    // Rule: select_to_logical
    if (!RuleConfig->isRuleDisabled(49)) {
      if (1
          && [&]() {
           return Helper.matchSelectToLogical(*MIs[0], MatchData49); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'select_to_logical'\n");
 Helper.applyBuildFn(*MIs[0], MatchData49); 
        return true;
      }
    }
    // Leaf name: select_to_minmax
    // Rule: select_to_minmax
    if (!RuleConfig->isRuleDisabled(106)) {
      if (1
          && [&]() {
           return Helper.matchSimplifySelectToMinMax(*MIs[0], MatchData106); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'select_to_minmax'\n");
 Helper.applyBuildFn(*MIs[0], MatchData106); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 31 /* TargetOpcode::G_PTR_ADD */) {
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(24)) {
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
    // Leaf name: reassoc_ptradd
    // Rule: reassoc_ptradd
    if (!RuleConfig->isRuleDisabled(41)) {
      if (1
          && [&]() {
           return Helper.matchReassocPtrAdd(*MIs[0], MatchData41); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'reassoc_ptradd'\n");
 Helper.applyBuildFnNoErase(*MIs[0], MatchData41); 
        return true;
      }
    }
    // Leaf name: ptr_add_immed_chain
    // Rule: ptr_add_immed_chain
    if (!RuleConfig->isRuleDisabled(42)) {
      if (1
          && [&]() {
           return Helper.matchPtrAddImmedChain(*MIs[0], MatchData42); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'ptr_add_immed_chain'\n");
 Helper.applyPtrAddImmedChain(*MIs[0], MatchData42); 
        return true;
      }
    }
    // Leaf name: const_ptradd_to_i2p
    // Rule: const_ptradd_to_i2p
    if (!RuleConfig->isRuleDisabled(69)) {
      if (1
          && [&]() {
           return Helper.matchCombineConstPtrAddToI2P(*MIs[0], MatchData69); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'const_ptradd_to_i2p'\n");
 Helper.applyCombineConstPtrAddToI2P(*MIs[0], MatchData69); 
        return true;
      }
    }
    // Leaf name: ptr_add_with_zero
    // Rule: ptr_add_with_zero
    if (!RuleConfig->isRuleDisabled(77)) {
      if (1
          && [&]() {
           return Helper.matchPtrAddZero(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'ptr_add_with_zero'\n");
 Helper.applyPtrAddZero(*MIs[0]); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 32 /* TargetOpcode::G_ROTL */) {
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(24)) {
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
  if (Partition == 33 /* TargetOpcode::G_ROTR */) {
    // Leaf name: right_identity_zero
    // Rule: right_identity_zero
    if (!RuleConfig->isRuleDisabled(24)) {
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
  if (Partition == 34 /* TargetOpcode::G_SDIV */) {
    // Leaf name: binop_left_to_zero
    // Rule: binop_left_to_zero
    if (!RuleConfig->isRuleDisabled(26)) {
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
    // Leaf name: div_rem_to_divrem
    // Rule: div_rem_to_divrem
    if (!RuleConfig->isRuleDisabled(82)) {
      if (1
          && [&]() {
           return Helper.matchCombineDivRem(*MIs[0], MatchData82); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'div_rem_to_divrem'\n");
 Helper.applyCombineDivRem(*MIs[0], MatchData82); 
        return true;
      }
    }
    // Leaf name: sdiv_by_const
    // Rule: sdiv_by_const
    if (!RuleConfig->isRuleDisabled(92)) {
      if (1
          && [&]() {
           return Helper.matchSDivByConst(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'sdiv_by_const'\n");
 Helper.applySDivByConst(*MIs[0]); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 35 /* TargetOpcode::G_SREM */) {
    // Leaf name: binop_left_to_zero
    // Rule: binop_left_to_zero
    if (!RuleConfig->isRuleDisabled(26)) {
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
    // Leaf name: div_rem_to_divrem
    // Rule: div_rem_to_divrem
    if (!RuleConfig->isRuleDisabled(82)) {
      if (1
          && [&]() {
           return Helper.matchCombineDivRem(*MIs[0], MatchData82); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'div_rem_to_divrem'\n");
 Helper.applyCombineDivRem(*MIs[0], MatchData82); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 36 /* TargetOpcode::G_INTTOPTR */) {
    // Leaf name: p2i_to_i2p
    // Rule: p2i_to_i2p
    if (!RuleConfig->isRuleDisabled(28)) {
      if (1
          && [&]() {
           return Helper.matchCombineI2PToP2I(*MIs[0], MatchData28); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'p2i_to_i2p'\n");
 Helper.applyCombineI2PToP2I(*MIs[0], MatchData28); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 37 /* TargetOpcode::G_PTRTOINT */) {
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
        if (!RuleConfig->isRuleDisabled(29)) {
          if (1
              && [&]() {
               MatchData29 = MIs[1]->getOperand(1).getReg(); 
              return true;
          }()             ) {
               LLVM_DEBUG(dbgs() << "Applying rule 'i2p_to_p2i'\n");
 Helper.applyCombineP2IToI2P(*MIs[0], MatchData29); 
            return true;
          }
        }
        return false;
      }
    }
  }
  if (Partition == 38 /* TargetOpcode::G_ANYEXT */) {
    // Leaf name: anyext_trunc_fold
    // Rule: anyext_trunc_fold
    if (!RuleConfig->isRuleDisabled(30)) {
      if (1
          && [&]() {
           return Helper.matchCombineAnyExtTrunc(*MIs[0], MatchData30); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'anyext_trunc_fold'\n");
 return Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData30); 
        return true;
      }
    }
    // Leaf name: ext_ext_fold
    // Rule: ext_ext_fold
    if (!RuleConfig->isRuleDisabled(58)) {
      if (1
          && [&]() {
           return Helper.matchCombineExtOfExt(*MIs[0], MatchData58); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'ext_ext_fold'\n");
 Helper.applyCombineExtOfExt(*MIs[0], MatchData58); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 39 /* TargetOpcode::G_FNEG */) {
    Partition = -1;
    if (MIs.size() <= 1) MIs.resize(2);
    MIs[1] = nullptr;
    if (MIs[0]->getOperand(1).isReg())
      MIs[1] = MRI.getVRegDef(MIs[0]->getOperand(1).getReg());
    if (MIs[1] == nullptr) Partition = 1;
    if (MIs[1] != nullptr) Partition = 0;
    if (Partition == -1) return false;
    if (Partition == 0 /* true */) {
      Partition = -1;
      switch (MIs[1]->getOpcode()) {
      case TargetOpcode::G_FNEG: Partition = 0; break;
      default: Partition = 1; break;
      }
      // Default case but without conflicting with potential default case in selection.
      if (Partition == -1) return false;
      if (Partition == 0 /* TargetOpcode::G_FNEG */) {
        // Leaf name: fneg_fneg_fold
        // Rule: fneg_fneg_fold
        if (!RuleConfig->isRuleDisabled(31)) {
          if (1
              && [&]() {
               MatchData31 = MIs[1]->getOperand(1).getReg(); 
              return true;
          }()             ) {
               LLVM_DEBUG(dbgs() << "Applying rule 'fneg_fneg_fold'\n");
 return Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData31); 
            return true;
          }
        }
        // Leaf name: constant_fp_op
        // Rule: constant_fp_op
        if (!RuleConfig->isRuleDisabled(68)) {
          if (1
              && [&]() {
               return Helper.matchCombineConstantFoldFpUnary(*MIs[0], MatchData68); 
              return true;
          }()             ) {
               LLVM_DEBUG(dbgs() << "Applying rule 'constant_fp_op'\n");
 Helper.applyCombineConstantFoldFpUnary(*MIs[0], MatchData68); 
            return true;
          }
        }
        return false;
      }
      if (Partition == 1 /* * or nullptr */) {
        // Leaf name: constant_fp_op
        // Rule: constant_fp_op
        if (!RuleConfig->isRuleDisabled(68)) {
          if (1
              && [&]() {
               return Helper.matchCombineConstantFoldFpUnary(*MIs[0], MatchData68); 
              return true;
          }()             ) {
               LLVM_DEBUG(dbgs() << "Applying rule 'constant_fp_op'\n");
 Helper.applyCombineConstantFoldFpUnary(*MIs[0], MatchData68); 
            return true;
          }
        }
        return false;
      }
    }
    if (Partition == 1 /* false */) {
      // Leaf name: constant_fp_op
      // Rule: constant_fp_op
      if (!RuleConfig->isRuleDisabled(68)) {
        if (1
            && [&]() {
             return Helper.matchCombineConstantFoldFpUnary(*MIs[0], MatchData68); 
            return true;
        }()           ) {
             LLVM_DEBUG(dbgs() << "Applying rule 'constant_fp_op'\n");
 Helper.applyCombineConstantFoldFpUnary(*MIs[0], MatchData68); 
          return true;
        }
      }
      return false;
    }
  }
  if (Partition == 40 /* TargetOpcode::G_BUILD_VECTOR_TRUNC */) {
    // Leaf name: buildvector_identity_fold
    // Rule: buildvector_identity_fold
    if (!RuleConfig->isRuleDisabled(34)) {
      if (1
          && [&]() {
           return Helper.matchBuildVectorIdentityFold(*MIs[0], MatchData34); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'buildvector_identity_fold'\n");
 Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData34); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 41 /* TargetOpcode::G_BITCAST */) {
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
        if (!RuleConfig->isRuleDisabled(37)) {
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
  if (Partition == 42 /* TargetOpcode::G_PHI */) {
    // Leaf name: extend_through_phis
    // Rule: extend_through_phis
    if (!RuleConfig->isRuleDisabled(38)) {
      if (1
          && [&]() {
           return Helper.matchExtendThroughPhis(*MIs[0], MatchData38); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'extend_through_phis'\n");
 Helper.applyExtendThroughPhis(*MIs[0], MatchData38); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 43 /* TargetOpcode::G_SEXT_INREG */) {
    // Leaf name: sext_inreg_of_load
    // Rule: sext_inreg_of_load
    if (!RuleConfig->isRuleDisabled(44)) {
      if (1
          && [&]() {
           return Helper.matchSextInRegOfLoad(*MIs[0], MatchData44); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'sext_inreg_of_load'\n");
 Helper.applySextInRegOfLoad(*MIs[0], MatchData44); 
        return true;
      }
    }
    // Leaf name: redundant_sext_inreg
    // Rule: redundant_sext_inreg
    if (!RuleConfig->isRuleDisabled(51)) {
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
    // Leaf name: sext_inreg_to_zext_inreg
    // Rule: sext_inreg_to_zext_inreg
    if (!RuleConfig->isRuleDisabled(57)) {
      if (1
          && [&]() {
          
            unsigned BitWidth = MRI.getType(MIs[0]->getOperand(1).getReg()).getScalarSizeInBits();
            return Helper.getKnownBits()->maskedValueIsZero(MIs[0]->getOperand(1).getReg(),
                     APInt::getOneBitSet(BitWidth, MIs[0]->getOperand(2).getImm() - 1)); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'sext_inreg_to_zext_inreg'\n");

          Helper.getBuilder().setInstrAndDebugLoc(*MIs[0]);
          Helper.getBuilder().buildZExtInReg(MIs[0]->getOperand(0), MIs[0]->getOperand(1), MIs[0]->getOperand(2).getImm());
          MIs[0]->eraseFromParent();
          return true;
      
        return true;
      }
    }
    // Leaf name: bitfield_extract_from_sext_inreg
    // Rule: bitfield_extract_from_sext_inreg
    if (!RuleConfig->isRuleDisabled(85)) {
      if (1
          && [&]() {
           return Helper.matchBitfieldExtractFromSExtInReg(*MIs[0], MatchData85); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'bitfield_extract_from_sext_inreg'\n");
 Helper.applyBuildFn(*MIs[0], MatchData85); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 44 /* TargetOpcode::G_ZEXT */) {
    // Leaf name: zext_trunc_fold
    // Rule: zext_trunc_fold
    if (!RuleConfig->isRuleDisabled(54)) {
      if (1
          && [&]() {
           return Helper.matchCombineZextTrunc(*MIs[0], MatchData54); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'zext_trunc_fold'\n");
 return Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData54); 
        return true;
      }
    }
    // Leaf name: ext_ext_fold
    // Rule: ext_ext_fold
    if (!RuleConfig->isRuleDisabled(58)) {
      if (1
          && [&]() {
           return Helper.matchCombineExtOfExt(*MIs[0], MatchData58); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'ext_ext_fold'\n");
 Helper.applyCombineExtOfExt(*MIs[0], MatchData58); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 45 /* TargetOpcode::G_ICMP */) {
    // Leaf name: icmp_to_true_false_known_bits
    // Rule: icmp_to_true_false_known_bits
    if (!RuleConfig->isRuleDisabled(55)) {
      if (1
          && [&]() {
           return Helper.matchICmpToTrueFalseKnownBits(*MIs[0], MatchData55); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'icmp_to_true_false_known_bits'\n");
 Helper.replaceInstWithConstant(*MIs[0], MatchData55); 
        return true;
      }
    }
    // Leaf name: icmp_to_lhs_known_bits
    // Rule: icmp_to_lhs_known_bits
    if (!RuleConfig->isRuleDisabled(56)) {
      if (1
          && [&]() {
           return Helper.matchICmpToLHSKnownBits(*MIs[0], MatchData56); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'icmp_to_lhs_known_bits'\n");
 Helper.applyBuildFn(*MIs[0], MatchData56); 
        return true;
      }
    }
    // Leaf name: redundant_binop_in_equality
    // Rule: redundant_binop_in_equality
    if (!RuleConfig->isRuleDisabled(107)) {
      if (1
          && [&]() {
           return Helper.matchRedundantBinOpInEquality(*MIs[0], MatchData107); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'redundant_binop_in_equality'\n");
 Helper.applyBuildFn(*MIs[0], MatchData107); 
        return true;
      }
    }
    // Leaf name: icmp_redundant_trunc
    // Rule: icmp_redundant_trunc
    if (!RuleConfig->isRuleDisabled(111)) {
      if (1
          && [&]() {
           return matchICmpRedundantTrunc(*MIs[0], MRI, Helper.getKnownBits(), MatchData111); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'icmp_redundant_trunc'\n");
 applyICmpRedundantTrunc(*MIs[0], MRI, B, Observer, MatchData111); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 46 /* TargetOpcode::G_SEXT */) {
    // Leaf name: ext_ext_fold
    // Rule: ext_ext_fold
    if (!RuleConfig->isRuleDisabled(58)) {
      if (1
          && [&]() {
           return Helper.matchCombineExtOfExt(*MIs[0], MatchData58); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'ext_ext_fold'\n");
 Helper.applyCombineExtOfExt(*MIs[0], MatchData58); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 47 /* TargetOpcode::G_BR */) {
    // Leaf name: opt_brcond_by_inverting_cond
    // Rule: opt_brcond_by_inverting_cond
    if (!RuleConfig->isRuleDisabled(60)) {
      if (1
          && [&]() {
           return Helper.matchOptBrCondByInvertingCond(*MIs[0], MatchData60); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'opt_brcond_by_inverting_cond'\n");
 Helper.applyOptBrCondByInvertingCond(*MIs[0], MatchData60); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 48 /* TargetOpcode::G_MERGE_VALUES */) {
    // Leaf name: merge_unmerge
    // Rule: merge_unmerge
    if (!RuleConfig->isRuleDisabled(65)) {
      if (1
          && [&]() {
           return Helper.matchCombineMergeUnmerge(*MIs[0], MatchData65); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'merge_unmerge'\n");
 Helper.replaceSingleDefInstWithReg(*MIs[0], MatchData65); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 49 /* TargetOpcode::G_FPTRUNC */) {
    // Leaf name: constant_fp_op
    // Rule: constant_fp_op
    if (!RuleConfig->isRuleDisabled(68)) {
      if (1
          && [&]() {
           return Helper.matchCombineConstantFoldFpUnary(*MIs[0], MatchData68); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'constant_fp_op'\n");
 Helper.applyCombineConstantFoldFpUnary(*MIs[0], MatchData68); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 50 /* TargetOpcode::G_FSQRT */) {
    // Leaf name: constant_fp_op
    // Rule: constant_fp_op
    if (!RuleConfig->isRuleDisabled(68)) {
      if (1
          && [&]() {
           return Helper.matchCombineConstantFoldFpUnary(*MIs[0], MatchData68); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'constant_fp_op'\n");
 Helper.applyCombineConstantFoldFpUnary(*MIs[0], MatchData68); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 51 /* TargetOpcode::G_FLOG2 */) {
    // Leaf name: constant_fp_op
    // Rule: constant_fp_op
    if (!RuleConfig->isRuleDisabled(68)) {
      if (1
          && [&]() {
           return Helper.matchCombineConstantFoldFpUnary(*MIs[0], MatchData68); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'constant_fp_op'\n");
 Helper.applyCombineConstantFoldFpUnary(*MIs[0], MatchData68); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 52 /* TargetOpcode::G_UMULO */) {
    // Leaf name: mulo_by_2
    // Rule: mulo_by_2
    if (!RuleConfig->isRuleDisabled(71)) {
      if (1
          && [&]() {
           return Helper.matchMulOBy2(*MIs[0], MatchData71); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'mulo_by_2'\n");
 Helper.applyBuildFnNoErase(*MIs[0], MatchData71); 
        return true;
      }
    }
    // Leaf name: mulo_by_0
    // Rule: mulo_by_0
    if (!RuleConfig->isRuleDisabled(72)) {
      if (1
          && [&]() {
           return Helper.matchMulOBy0(*MIs[0], MatchData72); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'mulo_by_0'\n");
 Helper.applyBuildFn(*MIs[0], MatchData72); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 53 /* TargetOpcode::G_SMULO */) {
    // Leaf name: mulo_by_2
    // Rule: mulo_by_2
    if (!RuleConfig->isRuleDisabled(71)) {
      if (1
          && [&]() {
           return Helper.matchMulOBy2(*MIs[0], MatchData71); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'mulo_by_2'\n");
 Helper.applyBuildFnNoErase(*MIs[0], MatchData71); 
        return true;
      }
    }
    // Leaf name: mulo_by_0
    // Rule: mulo_by_0
    if (!RuleConfig->isRuleDisabled(72)) {
      if (1
          && [&]() {
           return Helper.matchMulOBy0(*MIs[0], MatchData72); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'mulo_by_0'\n");
 Helper.applyBuildFn(*MIs[0], MatchData72); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 54 /* TargetOpcode::G_UADDO */) {
    // Leaf name: addo_by_0
    // Rule: addo_by_0
    if (!RuleConfig->isRuleDisabled(73)) {
      if (1
          && [&]() {
           return Helper.matchAddOBy0(*MIs[0], MatchData73); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'addo_by_0'\n");
 Helper.applyBuildFn(*MIs[0], MatchData73); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 55 /* TargetOpcode::G_SADDO */) {
    // Leaf name: addo_by_0
    // Rule: addo_by_0
    if (!RuleConfig->isRuleDisabled(73)) {
      if (1
          && [&]() {
           return Helper.matchAddOBy0(*MIs[0], MatchData73); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'addo_by_0'\n");
 Helper.applyBuildFn(*MIs[0], MatchData73); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 56 /* TargetOpcode::G_UADDE */) {
    // Leaf name: adde_to_addo
    // Rule: adde_to_addo
    if (!RuleConfig->isRuleDisabled(74)) {
      if (1
          && [&]() {
           return Helper.matchAddEToAddO(*MIs[0], MatchData74); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'adde_to_addo'\n");
 Helper.applyBuildFnNoErase(*MIs[0], MatchData74); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 57 /* TargetOpcode::G_SADDE */) {
    // Leaf name: adde_to_addo
    // Rule: adde_to_addo
    if (!RuleConfig->isRuleDisabled(74)) {
      if (1
          && [&]() {
           return Helper.matchAddEToAddO(*MIs[0], MatchData74); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'adde_to_addo'\n");
 Helper.applyBuildFnNoErase(*MIs[0], MatchData74); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 58 /* TargetOpcode::G_USUBE */) {
    // Leaf name: adde_to_addo
    // Rule: adde_to_addo
    if (!RuleConfig->isRuleDisabled(74)) {
      if (1
          && [&]() {
           return Helper.matchAddEToAddO(*MIs[0], MatchData74); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'adde_to_addo'\n");
 Helper.applyBuildFnNoErase(*MIs[0], MatchData74); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 59 /* TargetOpcode::G_SSUBE */) {
    // Leaf name: adde_to_addo
    // Rule: adde_to_addo
    if (!RuleConfig->isRuleDisabled(74)) {
      if (1
          && [&]() {
           return Helper.matchAddEToAddO(*MIs[0], MatchData74); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'adde_to_addo'\n");
 Helper.applyBuildFnNoErase(*MIs[0], MatchData74); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 60 /* TargetOpcode::G_FMINNUM */) {
    // Leaf name: combine_minmax_nan
    // Rule: combine_minmax_nan
    if (!RuleConfig->isRuleDisabled(75)) {
      if (1
          && [&]() {
           return Helper.matchCombineFMinMaxNaN(*MIs[0], MatchData75); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_minmax_nan'\n");
 Helper.replaceSingleDefInstWithOperand(*MIs[0], MatchData75); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 61 /* TargetOpcode::G_FMAXNUM */) {
    // Leaf name: combine_minmax_nan
    // Rule: combine_minmax_nan
    if (!RuleConfig->isRuleDisabled(75)) {
      if (1
          && [&]() {
           return Helper.matchCombineFMinMaxNaN(*MIs[0], MatchData75); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_minmax_nan'\n");
 Helper.replaceSingleDefInstWithOperand(*MIs[0], MatchData75); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 62 /* TargetOpcode::G_FMINIMUM */) {
    // Leaf name: combine_minmax_nan
    // Rule: combine_minmax_nan
    if (!RuleConfig->isRuleDisabled(75)) {
      if (1
          && [&]() {
           return Helper.matchCombineFMinMaxNaN(*MIs[0], MatchData75); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_minmax_nan'\n");
 Helper.replaceSingleDefInstWithOperand(*MIs[0], MatchData75); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 63 /* TargetOpcode::G_FMAXIMUM */) {
    // Leaf name: combine_minmax_nan
    // Rule: combine_minmax_nan
    if (!RuleConfig->isRuleDisabled(75)) {
      if (1
          && [&]() {
           return Helper.matchCombineFMinMaxNaN(*MIs[0], MatchData75); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_minmax_nan'\n");
 Helper.replaceSingleDefInstWithOperand(*MIs[0], MatchData75); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 64 /* TargetOpcode::G_SSHLSAT */) {
    // Leaf name: shift_immed_chain
    // Rule: shift_immed_chain
    if (!RuleConfig->isRuleDisabled(78)) {
      if (1
          && [&]() {
           return Helper.matchShiftImmedChain(*MIs[0], MatchData78); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'shift_immed_chain'\n");
 Helper.applyShiftImmedChain(*MIs[0], MatchData78); 
        return true;
      }
    }
    // Leaf name: shift_of_shifted_logic_chain
    // Rule: shift_of_shifted_logic_chain
    if (!RuleConfig->isRuleDisabled(79)) {
      if (1
          && [&]() {
           return Helper.matchShiftOfShiftedLogic(*MIs[0], MatchData79); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'shift_of_shifted_logic_chain'\n");
 Helper.applyShiftOfShiftedLogic(*MIs[0], MatchData79); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 65 /* TargetOpcode::G_USHLSAT */) {
    // Leaf name: shift_immed_chain
    // Rule: shift_immed_chain
    if (!RuleConfig->isRuleDisabled(78)) {
      if (1
          && [&]() {
           return Helper.matchShiftImmedChain(*MIs[0], MatchData78); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'shift_immed_chain'\n");
 Helper.applyShiftImmedChain(*MIs[0], MatchData78); 
        return true;
      }
    }
    // Leaf name: shift_of_shifted_logic_chain
    // Rule: shift_of_shifted_logic_chain
    if (!RuleConfig->isRuleDisabled(79)) {
      if (1
          && [&]() {
           return Helper.matchShiftOfShiftedLogic(*MIs[0], MatchData79); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'shift_of_shifted_logic_chain'\n");
 Helper.applyShiftOfShiftedLogic(*MIs[0], MatchData79); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 66 /* TargetOpcode::G_FSHL */) {
    // Leaf name: funnel_shift_to_rotate
    // Rule: funnel_shift_to_rotate
    if (!RuleConfig->isRuleDisabled(84)) {
      if (1
          && [&]() {
           return Helper.matchFunnelShiftToRotate(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'funnel_shift_to_rotate'\n");
 Helper.applyFunnelShiftToRotate(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 67 /* TargetOpcode::G_FSHR */) {
    // Leaf name: funnel_shift_to_rotate
    // Rule: funnel_shift_to_rotate
    if (!RuleConfig->isRuleDisabled(84)) {
      if (1
          && [&]() {
           return Helper.matchFunnelShiftToRotate(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'funnel_shift_to_rotate'\n");
 Helper.applyFunnelShiftToRotate(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 68 /* TargetOpcode::G_UMULH */) {
    // Leaf name: mulh_to_lshr
    // Rule: mulh_to_lshr
    if (!RuleConfig->isRuleDisabled(93)) {
      if (1
          && [&]() {
           return Helper.matchUMulHToLShr(*MIs[0]); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'mulh_to_lshr'\n");
 Helper.applyUMulHToLShr(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 69 /* TargetOpcode::G_FADD */) {
    // Leaf name: redundant_neg_operands
    // Rule: redundant_neg_operands
    if (!RuleConfig->isRuleDisabled(94)) {
      if (1
          && [&]() {
           return Helper.matchRedundantNegOperands(*MIs[0], MatchData94); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'redundant_neg_operands'\n");
 Helper.applyBuildFnNoErase(*MIs[0], MatchData94); 
        return true;
      }
    }
    // Leaf name: combine_fadd_fmul_to_fmad_or_fma
    // Rule: combine_fadd_fmul_to_fmad_or_fma
    if (!RuleConfig->isRuleDisabled(96)) {
      if (1
          && [&]() {
           return Helper.matchCombineFAddFMulToFMadOrFMA(*MIs[0],
                                                              MatchData96); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_fadd_fmul_to_fmad_or_fma'\n");
 Helper.applyBuildFn(*MIs[0], MatchData96); 
        return true;
      }
    }
    // Leaf name: combine_fadd_fpext_fmul_to_fmad_or_fma
    // Rule: combine_fadd_fpext_fmul_to_fmad_or_fma
    if (!RuleConfig->isRuleDisabled(97)) {
      if (1
          && [&]() {
           return Helper.matchCombineFAddFpExtFMulToFMadOrFMA(*MIs[0],
                                                                   MatchData97); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_fadd_fpext_fmul_to_fmad_or_fma'\n");
 Helper.applyBuildFn(*MIs[0], MatchData97); 
        return true;
      }
    }
    // Leaf name: combine_fadd_fma_fmul_to_fmad_or_fma
    // Rule: combine_fadd_fma_fmul_to_fmad_or_fma
    if (!RuleConfig->isRuleDisabled(98)) {
      if (1
          && [&]() {
           return Helper.matchCombineFAddFMAFMulToFMadOrFMA(*MIs[0],
                                                                 MatchData98); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_fadd_fma_fmul_to_fmad_or_fma'\n");
 Helper.applyBuildFn(*MIs[0], MatchData98); 
        return true;
      }
    }
    // Leaf name: combine_fadd_fpext_fma_fmul_to_fmad_or_fma
    // Rule: combine_fadd_fpext_fma_fmul_to_fmad_or_fma
    if (!RuleConfig->isRuleDisabled(99)) {
      if (1
          && [&]() {
           return Helper.matchCombineFAddFpExtFMulToFMadOrFMAAggressive(
                                                      *MIs[0], MatchData99); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_fadd_fpext_fma_fmul_to_fmad_or_fma'\n");
 Helper.applyBuildFn(*MIs[0], MatchData99); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 70 /* TargetOpcode::G_FSUB */) {
    // Leaf name: redundant_neg_operands
    // Rule: redundant_neg_operands
    if (!RuleConfig->isRuleDisabled(94)) {
      if (1
          && [&]() {
           return Helper.matchRedundantNegOperands(*MIs[0], MatchData94); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'redundant_neg_operands'\n");
 Helper.applyBuildFnNoErase(*MIs[0], MatchData94); 
        return true;
      }
    }
    // Leaf name: combine_fsub_fmul_to_fmad_or_fma
    // Rule: combine_fsub_fmul_to_fmad_or_fma
    if (!RuleConfig->isRuleDisabled(100)) {
      if (1
          && [&]() {
           return Helper.matchCombineFSubFMulToFMadOrFMA(*MIs[0],
                                                              MatchData100); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_fsub_fmul_to_fmad_or_fma'\n");
 Helper.applyBuildFn(*MIs[0], MatchData100); 
        return true;
      }
    }
    // Leaf name: combine_fsub_fneg_fmul_to_fmad_or_fma
    // Rule: combine_fsub_fneg_fmul_to_fmad_or_fma
    if (!RuleConfig->isRuleDisabled(101)) {
      if (1
          && [&]() {
           return Helper.matchCombineFSubFNegFMulToFMadOrFMA(*MIs[0],
                                                                  MatchData101); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_fsub_fneg_fmul_to_fmad_or_fma'\n");
 Helper.applyBuildFn(*MIs[0], MatchData101); 
        return true;
      }
    }
    // Leaf name: combine_fsub_fpext_fmul_to_fmad_or_fma
    // Rule: combine_fsub_fpext_fmul_to_fmad_or_fma
    if (!RuleConfig->isRuleDisabled(102)) {
      if (1
          && [&]() {
           return Helper.matchCombineFSubFpExtFMulToFMadOrFMA(*MIs[0],
                                                                   MatchData102); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_fsub_fpext_fmul_to_fmad_or_fma'\n");
 Helper.applyBuildFn(*MIs[0], MatchData102); 
        return true;
      }
    }
    // Leaf name: combine_fsub_fpext_fneg_fmul_to_fmad_or_fma
    // Rule: combine_fsub_fpext_fneg_fmul_to_fmad_or_fma
    if (!RuleConfig->isRuleDisabled(103)) {
      if (1
          && [&]() {
           return Helper.matchCombineFSubFpExtFNegFMulToFMadOrFMA(
                                                *MIs[0], MatchData103); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'combine_fsub_fpext_fneg_fmul_to_fmad_or_fma'\n");
 Helper.applyBuildFn(*MIs[0], MatchData103); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    // Leaf name: fsub_to_fneg
    // Rule: fsub_to_fneg
    if (!RuleConfig->isRuleDisabled(108)) {
      if (1
          && [&]() {
           return Helper.matchFsubToFneg(*MIs[0], MatchData108); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fsub_to_fneg'\n");
 Helper.applyFsubToFneg(*MIs[0], MatchData108); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 71 /* TargetOpcode::G_FMUL */) {
    // Leaf name: redundant_neg_operands
    // Rule: redundant_neg_operands
    if (!RuleConfig->isRuleDisabled(94)) {
      if (1
          && [&]() {
           return Helper.matchRedundantNegOperands(*MIs[0], MatchData94); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'redundant_neg_operands'\n");
 Helper.applyBuildFnNoErase(*MIs[0], MatchData94); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 72 /* TargetOpcode::G_FDIV */) {
    // Leaf name: redundant_neg_operands
    // Rule: redundant_neg_operands
    if (!RuleConfig->isRuleDisabled(94)) {
      if (1
          && [&]() {
           return Helper.matchRedundantNegOperands(*MIs[0], MatchData94); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'redundant_neg_operands'\n");
 Helper.applyBuildFnNoErase(*MIs[0], MatchData94); 
        return true;
      }
    }
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 73 /* TargetOpcode::G_FMAD */) {
    // Leaf name: redundant_neg_operands
    // Rule: redundant_neg_operands
    if (!RuleConfig->isRuleDisabled(94)) {
      if (1
          && [&]() {
           return Helper.matchRedundantNegOperands(*MIs[0], MatchData94); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'redundant_neg_operands'\n");
 Helper.applyBuildFnNoErase(*MIs[0], MatchData94); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 74 /* TargetOpcode::G_FMA */) {
    // Leaf name: redundant_neg_operands
    // Rule: redundant_neg_operands
    if (!RuleConfig->isRuleDisabled(94)) {
      if (1
          && [&]() {
           return Helper.matchRedundantNegOperands(*MIs[0], MatchData94); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'redundant_neg_operands'\n");
 Helper.applyBuildFnNoErase(*MIs[0], MatchData94); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 75 /* TargetOpcode::G_SMIN */) {
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 76 /* TargetOpcode::G_SMAX */) {
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 77 /* TargetOpcode::G_UMIN */) {
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 78 /* TargetOpcode::G_UMAX */) {
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 79 /* TargetOpcode::G_FREM */) {
    // Leaf name: fold_binop_into_select
    // Rule: fold_binop_into_select
    if (!RuleConfig->isRuleDisabled(104)) {
      if (1
          && [&]() {
           return Helper.matchFoldBinOpIntoSelect(*MIs[0], MatchData104); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_binop_into_select'\n");
 return Helper.applyFoldBinOpIntoSelect(*MIs[0], MatchData104); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 80 /* TargetOpcode::G_FCONSTANT */) {
    // Leaf name: fconstant_to_constant
    // Rule: fconstant_to_constant
    if (!RuleConfig->isRuleDisabled(110)) {
      if (1
          && [&]() {
           return matchFConstantToConstant(*MIs[0], MRI); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fconstant_to_constant'\n");
 applyFConstantToConstant(*MIs[0]); 
        return true;
      }
    }
    return false;
  }
  if (Partition == 81 /* TargetOpcode::G_GLOBAL_VALUE */) {
    // Leaf name: fold_global_offset
    // Rule: fold_global_offset
    if (!RuleConfig->isRuleDisabled(112)) {
      if (1
          && [&]() {
           return matchFoldGlobalOffset(*MIs[0], MRI, MatchData112); 
          return true;
      }()         ) {
           LLVM_DEBUG(dbgs() << "Applying rule 'fold_global_offset'\n");
  return applyFoldGlobalOffset(*MIs[0], MRI, B, Observer, MatchData112);
        return true;
      }
    }
    return false;
  }

  return false;
}
#endif // ifdef AARCH64PRELEGALIZERCOMBINERHELPER_GENCOMBINERHELPER_CPP
