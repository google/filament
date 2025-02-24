//===- subzero/src/IceRegistersMIPS32.h - Register information --*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the registers and their encodings for MIPS32.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEREGISTERSMIPS32_H
#define SUBZERO_SRC_ICEREGISTERSMIPS32_H

#include "IceDefs.h"
#include "IceInstMIPS32.def"
#include "IceOperand.h" // RC_Target
#include "IceTypes.h"

namespace Ice {
namespace MIPS32 {
namespace RegMIPS32 {

/// An enum of every register. The enum value may not match the encoding used to
/// binary encode register operands in instructions.
enum AllRegisters {
#define X(val, encode, name, scratch, preserved, stackptr, frameptr, isInt,    \
          isI64Pair, isFP32, isFP64, isVec128, alias_init)                     \
  val,
  REGMIPS32_TABLE
#undef X
      Reg_NUM,
#define X(val, init) val init,
  REGMIPS32_TABLE_BOUNDS
#undef X
};

/// An enum of GPR Registers. The enum value does match the encoding used to
/// binary encode register operands in instructions.
enum GPRRegister {
#define X(val, encode, name, scratch, preserved, stackptr, frameptr, isInt,    \
          isI64Pair, isFP32, isFP64, isVec128, alias_init)                     \
                                                                               \
  Encoded_##val = encode,
  REGMIPS32_GPR_TABLE
#undef X
      Encoded_Not_GPR = -1
};

/// An enum of FPR Registers. The enum value does match the encoding used to
/// binary encode register operands in instructions.
enum FPRRegister {
#define X(val, encode, name, scratch, preserved, stackptr, frameptr, isInt,    \
          isI64Pair, isFP32, isFP64, isVec128, alias_init)                     \
                                                                               \
  Encoded_##val = encode,
  REGMIPS32_FPR_TABLE
#undef X
      Encoded_Not_FPR = -1
};

// TODO(jvoung): Floating point and vector registers...
// Need to model overlap and difference in encoding too.

static inline GPRRegister getEncodedGPR(RegNumT RegNum) {
  assert(int(Reg_GPR_First) <= int(RegNum));
  assert(unsigned(RegNum) <= Reg_GPR_Last);
  return GPRRegister(RegNum - Reg_GPR_First);
}

static inline bool isGPRReg(RegNumT RegNum) {
  bool IsGPR = ((int(Reg_GPR_First) <= int(RegNum)) &&
                (unsigned(RegNum) <= Reg_GPR_Last)) ||
               ((int(Reg_I64PAIR_First) <= int(RegNum)) &&
                (unsigned(RegNum) <= Reg_I64PAIR_Last));
  return IsGPR;
}

static inline FPRRegister getEncodedFPR(RegNumT RegNum) {
  assert(int(Reg_FPR_First) <= int(RegNum));
  assert(unsigned(RegNum) <= Reg_FPR_Last);
  return FPRRegister(RegNum - Reg_FPR_First);
}

static inline bool isFPRReg(RegNumT RegNum) {
  return ((int(Reg_FPR_First) <= int(RegNum)) &&
          (unsigned(RegNum) <= Reg_FPR_Last));
}

static inline FPRRegister getEncodedFPR64(RegNumT RegNum) {
  assert(int(Reg_F64PAIR_First) <= int(RegNum));
  assert(unsigned(RegNum) <= Reg_F64PAIR_Last);
  return FPRRegister((RegNum - Reg_F64PAIR_First) * 2);
}

static inline bool isFPR64Reg(RegNumT RegNum) {
  return (int(Reg_F64PAIR_First) <= int(RegNum)) &&
         (unsigned(RegNum) <= Reg_F64PAIR_Last);
}

const char *getRegName(RegNumT RegNum);

static inline RegNumT get64PairFirstRegNum(RegNumT RegNum) {
  assert(unsigned(RegNum) >= Reg_I64PAIR_First);
  assert(unsigned(RegNum) <= Reg_F64PAIR_Last);
  if (unsigned(RegNum) >= Reg_F64PAIR_First &&
      unsigned(RegNum) <= Reg_F64PAIR_Last)
    return RegNumT::fixme(((RegNum - Reg_F64PAIR_First) * 2) +
                          unsigned(Reg_FPR_First));
  if (unsigned(RegNum) >= Reg_I64PAIR_First && unsigned(RegNum) <= Reg_T8T9)
    return RegNumT::fixme(((RegNum - Reg_I64PAIR_First) * 2) +
                          unsigned(Reg_V0));
  return RegMIPS32::Reg_LO;
}

static inline RegNumT get64PairSecondRegNum(RegNumT RegNum) {
  assert(unsigned(RegNum) >= Reg_I64PAIR_First);
  assert(unsigned(RegNum) <= Reg_F64PAIR_Last);
  if (unsigned(RegNum) >= Reg_F64PAIR_First &&
      unsigned(RegNum) <= Reg_F64PAIR_Last)
    return RegNumT::fixme(((RegNum - Reg_F64PAIR_First) * 2) +
                          unsigned(Reg_FPR_First) + 1);
  if (unsigned(RegNum) >= Reg_I64PAIR_First && unsigned(RegNum) <= Reg_T8T9)
    return RegNumT::fixme(((RegNum - Reg_I64PAIR_First) * 2) +
                          unsigned(Reg_V1));
  return RegMIPS32::Reg_HI;
}

} // end of namespace RegMIPS32

// Extend enum RegClass with MIPS32-specific register classes (if any).
enum RegClassMIPS32 : uint8_t { RCMIPS32_NUM = RC_Target };

} // end of namespace MIPS32
} // end of namespace Ice

#endif // SUBZERO_SRC_ICEREGISTERSMIPS32_H
