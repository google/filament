//===- subzero/src/IceRegistersX8632.h - Register information ---*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the registers and their encodings for x86-32.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEREGISTERSX8632_H
#define SUBZERO_SRC_ICEREGISTERSX8632_H

#include "IceBitVector.h"
#include "IceDefs.h"
#include "IceInstX8632.def"
#include "IceTargetLowering.h"
#include "IceTargetLoweringX86RegClass.h"
#include "IceTypes.h"

#include <initializer_list>

namespace Ice {
using namespace ::Ice::X86;

class RegX8632 {
public:
  /// An enum of every register. The enum value may not match the encoding used
  /// to binary encode register operands in instructions.
  enum AllRegisters {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8, is16To8,      \
          isTrunc8Rcvr, isAhRcvr, aliases)                                     \
  val,
    REGX8632_TABLE
#undef X
        Reg_NUM
  };

  /// An enum of GPR Registers. The enum value does match the encoding used to
  /// binary encode register operands in instructions.
  enum GPRRegister {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8, is16To8,      \
          isTrunc8Rcvr, isAhRcvr, aliases)                                     \
  Encoded_##val = encode,
    REGX8632_GPR_TABLE
#undef X
        Encoded_Not_GPR = -1
  };

  /// An enum of XMM Registers. The enum value does match the encoding used to
  /// binary encode register operands in instructions.
  enum XmmRegister {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8, is16To8,      \
          isTrunc8Rcvr, isAhRcvr, aliases)                                     \
  Encoded_##val = encode,
    REGX8632_XMM_TABLE
#undef X
        Encoded_Not_Xmm = -1
  };

  /// An enum of Byte Registers. The enum value does match the encoding used to
  /// binary encode register operands in instructions.
  enum ByteRegister {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8, is16To8,      \
          isTrunc8Rcvr, isAhRcvr, aliases)                                     \
  Encoded_8_##val = encode,
    REGX8632_BYTEREG_TABLE
#undef X
        Encoded_Not_ByteReg = -1
  };

  /// An enum of X87 Stack Registers. The enum value does match the encoding
  /// used to binary encode register operands in instructions.
  enum X87STRegister {
#define X(val, encode, name) Encoded_##val = encode,
    X87ST_REGX8632_TABLE
#undef X
        Encoded_Not_X87STReg = -1
  };

  static inline X87STRegister getEncodedSTReg(uint32_t X87RegNum) {
    assert(int(Encoded_X87ST_First) <= int(X87RegNum));
    assert(X87RegNum <= Encoded_X87ST_Last);
    return X87STRegister(X87RegNum);
  }

  static inline const char *getRegName(RegNumT RegNum) {
    static const char *const RegNames[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8, is16To8,      \
          isTrunc8Rcvr, isAhRcvr, aliases)                                     \
  name,
        REGX8632_TABLE
#undef X
    };
    RegNum.assertIsValid();
    return RegNames[RegNum];
  }

  static inline GPRRegister getEncodedGPR(RegNumT RegNum) {
    static const GPRRegister GPRRegs[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8, is16To8,      \
          isTrunc8Rcvr, isAhRcvr, aliases)                                     \
  GPRRegister(isGPR ? encode : GPRRegister::Encoded_Not_GPR),
        REGX8632_TABLE
#undef X
    };
    RegNum.assertIsValid();
    assert(GPRRegs[RegNum] != GPRRegister::Encoded_Not_GPR);
    return GPRRegs[RegNum];
  }

  static inline ByteRegister getEncodedByteReg(RegNumT RegNum) {
    static const ByteRegister ByteRegs[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8, is16To8,      \
          isTrunc8Rcvr, isAhRcvr, aliases)                                     \
  ByteRegister(is8 ? encode : ByteRegister::Encoded_Not_ByteReg),
        REGX8632_TABLE
#undef X
    };
    RegNum.assertIsValid();
    assert(ByteRegs[RegNum] != ByteRegister::Encoded_Not_ByteReg);
    return ByteRegs[RegNum];
  }

  static inline bool isXmm(RegNumT RegNum) {
    static const bool IsXmm[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8, is16To8,      \
          isTrunc8Rcvr, isAhRcvr, aliases)                                     \
  isXmm,
        REGX8632_TABLE
#undef X
    };
    return IsXmm[RegNum];
  }

  static inline XmmRegister getEncodedXmm(RegNumT RegNum) {
    static const XmmRegister XmmRegs[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8, is16To8,      \
          isTrunc8Rcvr, isAhRcvr, aliases)                                     \
  XmmRegister(isXmm ? encode : XmmRegister::Encoded_Not_Xmm),
        REGX8632_TABLE
#undef X
    };
    RegNum.assertIsValid();
    assert(XmmRegs[RegNum] != XmmRegister::Encoded_Not_Xmm);
    return XmmRegs[RegNum];
  }

  static inline uint32_t getEncoding(RegNumT RegNum) {
    static const uint32_t Encoding[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8, is16To8,      \
          isTrunc8Rcvr, isAhRcvr, aliases)                                     \
  encode,
        REGX8632_TABLE
#undef X
    };
    RegNum.assertIsValid();
    return Encoding[RegNum];
  }

  static inline RegNumT getBaseReg(RegNumT RegNum) {
    static const RegNumT BaseRegs[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8, is16To8,      \
          isTrunc8Rcvr, isAhRcvr, aliases)                                     \
  base,
        REGX8632_TABLE
#undef X
    };
    RegNum.assertIsValid();
    return BaseRegs[RegNum];
  }

private:
  static inline AllRegisters getFirstGprForType(Type Ty) {
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
    }
  }

public:
  // Return a register in RegNum's alias set that is suitable for Ty.
  static inline RegNumT getGprForType(Type Ty, RegNumT RegNum) {
    assert(RegNum.hasValue());

    if (!isScalarIntegerType(Ty)) {
      return RegNum;
    }

    // [abcd]h registers are not convertible to their ?l, ?x, and e?x versions.
    switch (RegNum) {
    default:
      break;
    case Reg_ah:
    case Reg_bh:
    case Reg_ch:
    case Reg_dh:
      assert(isByteSizedType(Ty));
      return RegNum;
    }

    const AllRegisters FirstGprForType = getFirstGprForType(Ty);

    switch (RegNum) {
    default:
      llvm::report_fatal_error("Unknown register.");
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8, is16To8,      \
          isTrunc8Rcvr, isAhRcvr, aliases)                                     \
  case val: {                                                                  \
    if (!isGPR)                                                                \
      return val;                                                              \
    assert((is32) || (is16) || (is8) || getBaseReg(val) == Reg_esp);           \
    constexpr AllRegisters FirstGprWithRegNumSize =                            \
        (((is32) || val == Reg_esp)                                            \
             ? Reg_eax                                                         \
             : (((is16) || val == Reg_sp) ? Reg_ax : Reg_al));                 \
    const RegNumT NewRegNum =                                                  \
        RegNumT::fixme(RegNum - FirstGprWithRegNumSize + FirstGprForType);     \
    assert(getBaseReg(RegNum) == getBaseReg(NewRegNum) &&                      \
           "Error involving " #val);                                           \
    return NewRegNum;                                                          \
  }
      REGX8632_TABLE
#undef X
    }
  }

public:
  static inline void
  initRegisterSet(const ::Ice::ClFlags & /*Flags*/,
                  std::array<SmallBitVector, RCX86_NUM> *TypeToRegisterSet,
                  std::array<SmallBitVector, Reg_NUM> *RegisterAliases) {
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
    } X8632RegTable[Reg_NUM] = {
#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,      \
          isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8, is16To8,       \
          isTrunc8Rcvr, isAhRcvr, aliases)                                      \
  {                                                                             \
      val,          is64,     is32,                                             \
      is16,         is8,      isXmm,                                            \
      is64To8,      is32To8,  is16To8,                                          \
      isTrunc8Rcvr, isAhRcvr, (std::initializer_list<uint16_t> aliases).size(), \
      aliases,                                                                  \
  },
        REGX8632_TABLE
#undef X
    };

    for (SizeT ii = 0; ii < llvm::array_lengthof(X8632RegTable); ++ii) {
      const auto &Entry = X8632RegTable[ii];
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
      (*RegisterAliases)[Entry.Val].resize(Reg_NUM);
      for (SizeT J = 0; J < Entry.NumAliases; J++) {
        SizeT Alias = Entry.Aliases[J];
        assert(!(*RegisterAliases)[Entry.Val][Alias] && "Duplicate alias");
        (*RegisterAliases)[Entry.Val].set(Alias);
      }
      (*RegisterAliases)[Entry.Val].set(Entry.Val);
    }

    (*TypeToRegisterSet)[RC_void] = InvalidRegisters;
    (*TypeToRegisterSet)[RC_i1] = IntegerRegistersI8;
    (*TypeToRegisterSet)[RC_i8] = IntegerRegistersI8;
    (*TypeToRegisterSet)[RC_i16] = IntegerRegistersI16;
    (*TypeToRegisterSet)[RC_i32] = IntegerRegistersI32;
    (*TypeToRegisterSet)[RC_i64] = InvalidRegisters;
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
  getRegisterSet(const ::Ice::ClFlags & /*Flags*/,
                 TargetLowering::RegSetMask Include,
                 TargetLowering::RegSetMask Exclude) {
    SmallBitVector Registers(Reg_NUM);

#define X(val, encode, name, base, scratch, preserved, stackptr, frameptr,     \
          isGPR, is64, is32, is16, is8, isXmm, is64To8, is32To8, is16To8,      \
          isTrunc8Rcvr, isAhRcvr, aliases)                                     \
  if (scratch && (Include & ::Ice::TargetLowering::RegSet_CallerSave))         \
    Registers[val] = true;                                                     \
  if (preserved && (Include & ::Ice::TargetLowering::RegSet_CalleeSave))       \
    Registers[val] = true;                                                     \
  if (stackptr && (Include & ::Ice::TargetLowering::RegSet_StackPointer))      \
    Registers[val] = true;                                                     \
  if (frameptr && (Include & ::Ice::TargetLowering::RegSet_FramePointer))      \
    Registers[val] = true;                                                     \
  if (scratch && (Exclude & ::Ice::TargetLowering::RegSet_CallerSave))         \
    Registers[val] = false;                                                    \
  if (preserved && (Exclude & ::Ice::TargetLowering::RegSet_CalleeSave))       \
    Registers[val] = false;                                                    \
  if (stackptr && (Exclude & ::Ice::TargetLowering::RegSet_StackPointer))      \
    Registers[val] = false;                                                    \
  if (frameptr && (Exclude & ::Ice::TargetLowering::RegSet_FramePointer))      \
    Registers[val] = false;

    REGX8632_TABLE

#undef X

    return Registers;
  }

  // x86-32 calling convention:
  //
  // * The first four arguments of vector type, regardless of their position
  // relative to the other arguments in the argument list, are placed in
  // registers xmm0 - xmm3.
  //
  // This intends to match the section "IA-32 Function Calling Convention" of
  // the document "OS X ABI Function Call Guide" by Apple.

  /// The maximum number of arguments to pass in XMM registers
  static constexpr uint32_t X86_MAX_XMM_ARGS = 4;
  /// The maximum number of arguments to pass in GPR registers
  static constexpr uint32_t X86_MAX_GPR_ARGS = 0;
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
  /// Get the register for a given argument slot in the GPRs.
  static inline RegNumT getRegisterForGprArgNum(Type Ty, uint32_t ArgNum) {
    assert(Ty == IceType_i64 || Ty == IceType_i32);
    (void)Ty;
    (void)ArgNum;
    return RegNumT();
  }
  // Given the absolute argument position and argument position by type, return
  // the register index to assign it to.
  static inline SizeT getArgIndex(SizeT argPos, SizeT argPosByType) {
    (void)argPos;
    return argPosByType;
  };
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEREGISTERSX8632_H
