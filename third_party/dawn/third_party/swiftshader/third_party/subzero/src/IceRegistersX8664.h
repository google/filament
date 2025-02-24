//===- subzero/src/IceRegistersX8664.h - Register information ---*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the registers and their encodings for x86-64.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEREGISTERSX8664_H
#define SUBZERO_SRC_ICEREGISTERSX8664_H

#include "IceBitVector.h"
#include "IceDefs.h"
#include "IceInstX8664.def"
#include "IceTargetLowering.h"
#include "IceTargetLoweringX86RegClass.h"
#include "IceTypes.h"

#include <initializer_list>

namespace Ice {
using namespace ::Ice::X86;

class RegX8664 {
public:
  /// An enum of every register. The enum value may not match the encoding used
  /// to binary encode register operands in instructions.
  enum AllRegisters {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  val,
    REGX8664_TABLE
#undef X
        Reg_NUM
  };

  /// An enum of GPR Registers. The enum value does match the encoding used to
  /// binary encode register operands in instructions.
  enum GPRRegister {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  Encoded_##val = encode,
    REGX8664_GPR_TABLE
#undef X
        Encoded_Not_GPR = -1
  };

  /// An enum of XMM Registers. The enum value does match the encoding used to
  /// binary encode register operands in instructions.
  enum XmmRegister {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  Encoded_##val = encode,
    REGX8664_XMM_TABLE
#undef X
        Encoded_Not_Xmm = -1
  };

  /// An enum of Byte Registers. The enum value does match the encoding used to
  /// binary encode register operands in instructions.
  enum ByteRegister {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  Encoded_8_##val = encode,
    REGX8664_BYTEREG_TABLE
#undef X
        Encoded_Not_ByteReg = -1
  };

  static inline const char *getRegName(RegNumT RegNum) {
    static const char *const RegNames[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  name,
        REGX8664_TABLE
#undef X
    };
    RegNum.assertIsValid();
    return RegNames[RegNum];
  }

  static inline GPRRegister getEncodedGPR(RegNumT RegNum) {
    static const GPRRegister GPRRegs[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  GPRRegister(isGPR ? encode : GPRRegister::Encoded_Not_GPR),
        REGX8664_TABLE
#undef X
    };
    RegNum.assertIsValid();
    assert(GPRRegs[RegNum] != GPRRegister::Encoded_Not_GPR);
    return GPRRegs[RegNum];
  }

  static inline ByteRegister getEncodedByteReg(RegNumT RegNum) {
    static const ByteRegister ByteRegs[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  ByteRegister(is8 ? encode : ByteRegister::Encoded_Not_ByteReg),
        REGX8664_TABLE
#undef X
    };
    RegNum.assertIsValid();
    assert(ByteRegs[RegNum] != ByteRegister::Encoded_Not_ByteReg);
    return ByteRegs[RegNum];
  }

  static inline bool isXmm(RegNumT RegNum) {
    static const bool IsXmm[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  isXmm,
        REGX8664_TABLE
#undef X
    };
    return IsXmm[RegNum];
  }

  static inline XmmRegister getEncodedXmm(RegNumT RegNum) {
    static const XmmRegister XmmRegs[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  XmmRegister(isXmm ? encode : XmmRegister::Encoded_Not_Xmm),
        REGX8664_TABLE
#undef X
    };
    RegNum.assertIsValid();
    assert(XmmRegs[RegNum] != XmmRegister::Encoded_Not_Xmm);
    return XmmRegs[RegNum];
  }

  static inline uint32_t getEncoding(RegNumT RegNum) {
    static const uint32_t Encoding[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  encode,
        REGX8664_TABLE
#undef X
    };
    RegNum.assertIsValid();
    return Encoding[RegNum];
  }

  static inline RegNumT getBaseReg(RegNumT RegNum) {
    static const RegNumT BaseRegs[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  base,
        REGX8664_TABLE
#undef X
    };
    RegNum.assertIsValid();
    return BaseRegs[RegNum];
  }

private:
  static inline RegNumT getFirstGprForType(Type Ty) {
    switch (Ty) {
    default:
      llvm_unreachable("Invalid type for GPR.");
    case IceType_i1:
    case IceType_i8:
      return Reg_al;
    case IceType_i16:
      return Reg_ax;
    case IceType_i32:
      return Reg_eax;
    case IceType_i64:
      return Reg_rax;
    }
  }

public:
  static inline RegNumT getGprForType(Type Ty, RegNumT RegNum) {
    assert(RegNum.hasValue());

    if (!isScalarIntegerType(Ty)) {
      return RegNum;
    }

    assert(Ty == IceType_i1 || Ty == IceType_i8 || Ty == IceType_i16 ||
           Ty == IceType_i32 || Ty == IceType_i64);

    if (RegNum == Reg_ah) {
      assert(Ty == IceType_i8);
      return RegNum;
    }

    assert(RegNum != Reg_bh);
    assert(RegNum != Reg_ch);
    assert(RegNum != Reg_dh);

    const RegNumT FirstGprForType = getFirstGprForType(Ty);

    switch (RegNum) {
    default:
      llvm::report_fatal_error("Unknown register.");
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  case val: {                                                                  \
    if (!isGPR)                                                                \
      return val;                                                              \
    assert((is64) || (is32) || (is16) || (is8) || getBaseReg(val) == Reg_rsp); \
    constexpr AllRegisters FirstGprWithRegNumSize =                            \
        ((is64) || val == Reg_rsp)                                             \
            ? Reg_rax                                                          \
            : (((is32) || val == Reg_esp)                                      \
                   ? Reg_eax                                                   \
                   : (((is16) || val == Reg_sp) ? Reg_ax : Reg_al));           \
    const auto NewRegNum =                                                     \
        RegNumT::fixme(RegNum - FirstGprWithRegNumSize + FirstGprForType);     \
    assert(getBaseReg(RegNum) == getBaseReg(NewRegNum) &&                      \
           "Error involving " #val);                                           \
    return NewRegNum;                                                          \
  }
      REGX8664_TABLE
#undef X
    }
  }

  static inline void
  initRegisterSet(const ::Ice::ClFlags &Flags,
                  std::array<SmallBitVector, RCX86_NUM> *TypeToRegisterSet,
                  std::array<SmallBitVector, Reg_NUM> *RegisterAliases) {
    SmallBitVector IntegerRegistersI64(Reg_NUM);
    SmallBitVector IntegerRegistersI32(Reg_NUM);
    SmallBitVector IntegerRegistersI16(Reg_NUM);
    SmallBitVector IntegerRegistersI8(Reg_NUM);
    SmallBitVector FloatRegisters(Reg_NUM);
    SmallBitVector VectorRegisters(Reg_NUM);
    SmallBitVector Trunc64To8Registers(Reg_NUM);
    SmallBitVector Trunc32To8Registers(Reg_NUM);
    SmallBitVector Trunc16To8Registers(Reg_NUM);
    SmallBitVector Trunc8RcvrRegisters(Reg_NUM);
    SmallBitVector AhRcvrRegisters(Reg_NUM);
    SmallBitVector InvalidRegisters(Reg_NUM);

    static constexpr struct {
      uint16_t Val;
      unsigned IsReservedWhenSandboxing : 1;
      unsigned Is64 : 1;
      unsigned Is32 : 1;
      unsigned Is16 : 1;
      unsigned Is8 : 1;
      unsigned IsXmm : 1;
      unsigned Is64To8 : 1;
      unsigned Is32To8 : 1;
      unsigned Is16To8 : 1;
      unsigned IsTrunc8Rcvr : 1;
      unsigned IsAhRcvr : 1;
#define NUM_ALIASES_BITS 2
      SizeT NumAliases : (NUM_ALIASES_BITS + 1);
      uint16_t Aliases[1 << NUM_ALIASES_BITS];
#undef NUM_ALIASES_BITS
    } X8664RegTable[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  {                                                                            \
      val,                                                                     \
      sboxres,                                                                 \
      is64,                                                                    \
      is32,                                                                    \
      is16,                                                                    \
      is8,                                                                     \
      isXmm,                                                                   \
      is64To8,                                                                 \
      is32To8,                                                                 \
      is16To8,                                                                 \
      isTrunc8Rcvr,                                                            \
      isAhRcvr,                                                                \
      (std::initializer_list<uint16_t> aliases).size(),                        \
      aliases,                                                                 \
  },
        REGX8664_TABLE
#undef X
    };

    for (SizeT ii = 0; ii < llvm::array_lengthof(X8664RegTable); ++ii) {
      const auto &Entry = X8664RegTable[ii];
      // Even though the register is disabled for register allocation, it might
      // still be used by the Target Lowering (e.g., base pointer), so the
      // register alias table still needs to be defined.
      (*RegisterAliases)[Entry.Val].resize(Reg_NUM);
      for (Ice::SizeT J = 0; J < Entry.NumAliases; ++J) {
        SizeT Alias = Entry.Aliases[J];
        assert(!(*RegisterAliases)[Entry.Val][Alias] && "Duplicate alias");
        (*RegisterAliases)[Entry.Val].set(Alias);
      }

      (*RegisterAliases)[Entry.Val].set(Entry.Val);

      (IntegerRegistersI64)[Entry.Val] = Entry.Is64;
      (IntegerRegistersI32)[Entry.Val] = Entry.Is32;
      (IntegerRegistersI16)[Entry.Val] = Entry.Is16;
      (IntegerRegistersI8)[Entry.Val] = Entry.Is8;
      (FloatRegisters)[Entry.Val] = Entry.IsXmm;
      (VectorRegisters)[Entry.Val] = Entry.IsXmm;
      (Trunc64To8Registers)[Entry.Val] = Entry.Is64To8;
      (Trunc32To8Registers)[Entry.Val] = Entry.Is32To8;
      (Trunc16To8Registers)[Entry.Val] = Entry.Is16To8;
      (Trunc8RcvrRegisters)[Entry.Val] = Entry.IsTrunc8Rcvr;
      (AhRcvrRegisters)[Entry.Val] = Entry.IsAhRcvr;
    }

    (*TypeToRegisterSet)[RC_void] = InvalidRegisters;
    (*TypeToRegisterSet)[RC_i1] = IntegerRegistersI8;
    (*TypeToRegisterSet)[RC_i8] = IntegerRegistersI8;
    (*TypeToRegisterSet)[RC_i16] = IntegerRegistersI16;
    (*TypeToRegisterSet)[RC_i32] = IntegerRegistersI32;
    (*TypeToRegisterSet)[RC_i64] = IntegerRegistersI64;
    (*TypeToRegisterSet)[RC_f32] = FloatRegisters;
    (*TypeToRegisterSet)[RC_f64] = FloatRegisters;
    (*TypeToRegisterSet)[RC_v4i1] = VectorRegisters;
    (*TypeToRegisterSet)[RC_v8i1] = VectorRegisters;
    (*TypeToRegisterSet)[RC_v16i1] = VectorRegisters;
    (*TypeToRegisterSet)[RC_v16i8] = VectorRegisters;
    (*TypeToRegisterSet)[RC_v8i16] = VectorRegisters;
    (*TypeToRegisterSet)[RC_v4i32] = VectorRegisters;
    (*TypeToRegisterSet)[RC_v4f32] = VectorRegisters;
    (*TypeToRegisterSet)[RCX86_Is64To8] = Trunc64To8Registers;
    (*TypeToRegisterSet)[RCX86_Is32To8] = Trunc32To8Registers;
    (*TypeToRegisterSet)[RCX86_Is16To8] = Trunc16To8Registers;
    (*TypeToRegisterSet)[RCX86_IsTrunc8Rcvr] = Trunc8RcvrRegisters;
    (*TypeToRegisterSet)[RCX86_IsAhRcvr] = AhRcvrRegisters;
  }

  static inline SmallBitVector
  getRegisterSet(const ::Ice::ClFlags &Flags,
                 TargetLowering::RegSetMask Include,
                 TargetLowering::RegSetMask Exclude) {
    SmallBitVector Registers(Reg_NUM);

#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          sboxres, isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8,      \
          is16To8, isTrunc8Rcvr, isAhRcvr, aliases)                            \
  if (scratch && (Include & TargetLowering::RegSet_CallerSave))                \
    Registers[val] = true;                                                     \
  if (preserved && (Include & TargetLowering::RegSet_CalleeSave))              \
    Registers[val] = true;                                                     \
  if (stackptr && (Include & TargetLowering::RegSet_StackPointer))             \
    Registers[val] = true;                                                     \
  if (frameptr && (Include & TargetLowering::RegSet_FramePointer))             \
    Registers[val] = true;                                                     \
  if (scratch && (Exclude & TargetLowering::RegSet_CallerSave))                \
    Registers[val] = false;                                                    \
  if (preserved && (Exclude & TargetLowering::RegSet_CalleeSave))              \
    Registers[val] = false;                                                    \
  if (stackptr && (Exclude & TargetLowering::RegSet_StackPointer))             \
    Registers[val] = false;                                                    \
  if (frameptr && (Exclude & TargetLowering::RegSet_FramePointer))             \
    Registers[val] = false;

    REGX8664_TABLE

#undef X

    return Registers;
  }

#if defined(_WIN64)
  // Microsoft x86-64 calling convention:
  //
  // * The first four arguments of vector/fp type, regardless of their
  // position relative to the other arguments in the argument list, are placed
  // in registers %xmm0 - %xmm3.
  //
  // * The first four arguments of integer types, regardless of their position
  // relative to the other arguments in the argument list, are placed in
  // registers %rcx, %rdx, %r8, and %r9.

  /// The maximum number of arguments to pass in XMM registers
  static constexpr uint32_t X86_MAX_XMM_ARGS = 4;
  /// The maximum number of arguments to pass in GPR registers
  static constexpr uint32_t X86_MAX_GPR_ARGS = 4;
  static inline RegNumT getRegisterForGprArgNum(Type Ty, uint32_t ArgNum) {
    if (ArgNum >= X86_MAX_GPR_ARGS) {
      return RegNumT();
    }
    static const AllRegisters GprForArgNum[] = {
        Reg_rcx,
        Reg_rdx,
        Reg_r8,
        Reg_r9,
    };
    static_assert(llvm::array_lengthof(GprForArgNum) == X86_MAX_GPR_ARGS,
                  "Mismatch between MAX_GPR_ARGS and GprForArgNum.");
    assert(Ty == IceType_i64 || Ty == IceType_i32);
    return RegX8664::getGprForType(Ty, GprForArgNum[ArgNum]);
  }
  // Given the absolute argument position and argument position by type, return
  // the register index to assign it to.
  static inline SizeT getArgIndex(SizeT argPos, SizeT argPosByType) {
    // Microsoft x64 ABI: register is selected by arg position (e.g. 1st int as
    // 2nd param goes into 2nd int reg)
    (void)argPosByType;
    return argPos;
  };

#else
  // System V x86-64 calling convention:
  //
  // * The first eight arguments of vector/fp type, regardless of their
  // position relative to the other arguments in the argument list, are placed
  // in registers %xmm0 - %xmm7.
  //
  // * The first six arguments of integer types, regardless of their position
  // relative to the other arguments in the argument list, are placed in
  // registers %rdi, %rsi, %rdx, %rcx, %r8, and %r9.
  //
  // This intends to match the section "Function Calling Sequence" of the
  // document "System V Application Binary Interface."

  /// The maximum number of arguments to pass in XMM registers
  static constexpr uint32_t X86_MAX_XMM_ARGS = 8;
  /// The maximum number of arguments to pass in GPR registers
  static constexpr uint32_t X86_MAX_GPR_ARGS = 6;
  /// Get the register for a given argument slot in the GPRs.
  static inline RegNumT getRegisterForGprArgNum(Type Ty, uint32_t ArgNum) {
    if (ArgNum >= X86_MAX_GPR_ARGS) {
      return RegNumT();
    }
    static const AllRegisters GprForArgNum[] = {
        Reg_rdi, Reg_rsi, Reg_rdx, Reg_rcx, Reg_r8, Reg_r9,
    };
    static_assert(llvm::array_lengthof(GprForArgNum) == X86_MAX_GPR_ARGS,
                  "Mismatch between MAX_GPR_ARGS and GprForArgNum.");
    assert(Ty == IceType_i64 || Ty == IceType_i32);
    return RegX8664::getGprForType(Ty, GprForArgNum[ArgNum]);
  }
  // Given the absolute argument position and argument position by type, return
  // the register index to assign it to.
  static inline SizeT getArgIndex(SizeT argPos, SizeT argPosByType) {
    (void)argPos;
    return argPosByType;
  }
#endif

  /// Get the register for a given argument slot in the XMM registers.
  static inline RegNumT getRegisterForXmmArgNum(uint32_t ArgNum) {
    // TODO(sehr): Change to use the CCArg technique used in ARM32.
    static_assert(Reg_xmm0 + 1 == Reg_xmm1,
                  "Inconsistency between XMM register numbers and ordinals");
    if (ArgNum >= X86_MAX_XMM_ARGS) {
      return RegNumT();
    }
    return RegNumT::fixme(Reg_xmm0 + ArgNum);
  }
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEREGISTERSX8664_H
