//===- subzero/src/IceRegistersARM32.h - Register information ---*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the registers and their encodings for ARM32.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEREGISTERSARM32_H
#define SUBZERO_SRC_ICEREGISTERSARM32_H

#include "IceDefs.h"
#include "IceInstARM32.def"
#include "IceOperand.h" // RC_Target
#include "IceTypes.h"

namespace Ice {
namespace ARM32 {
namespace RegARM32 {

/// An enum of every register. The enum value may not match the encoding used
/// to binary encode register operands in instructions.
enum AllRegisters {
#define X(val, encode, name, cc_arg, scratch, preserved, stackptr, frameptr,   \
          isGPR, isInt, isI64Pair, isFP32, isFP64, isVec128, alias_init)       \
  val,
  REGARM32_TABLE
#undef X
      Reg_NUM,
#define X(val, init) val init,
  REGARM32_TABLE_BOUNDS
#undef X
};

/// An enum of GPR Registers. The enum value does match the encoding used to
/// binary encode register operands in instructions.
enum GPRRegister {
#define X(val, encode, name, cc_arg, scratch, preserved, stackptr, frameptr,   \
          isGPR, isInt, isI64Pair, isFP32, isFP64, isVec128, alias_init)       \
  Encoded_##val = encode,
  REGARM32_GPR_TABLE
#undef X
      Encoded_Not_GPR = -1
};

/// An enum of FP32 S-Registers. The enum value does match the encoding used
/// to binary encode register operands in instructions.
enum SRegister {
#define X(val, encode, name, cc_arg, scratch, preserved, stackptr, frameptr,   \
          isGPR, isInt, isI64Pair, isFP32, isFP64, isVec128, alias_init)       \
  Encoded_##val = encode,
  REGARM32_FP32_TABLE
#undef X
      Encoded_Not_SReg = -1
};

/// An enum of FP64 D-Registers. The enum value does match the encoding used
/// to binary encode register operands in instructions.
enum DRegister {
#define X(val, encode, name, cc_arg, scratch, preserved, stackptr, frameptr,   \
          isGPR, isInt, isI64Pair, isFP32, isFP64, isVec128, alias_init)       \
  Encoded_##val = encode,
  REGARM32_FP64_TABLE
#undef X
      Encoded_Not_DReg = -1
};

/// An enum of 128-bit Q-Registers. The enum value does match the encoding
/// used to binary encode register operands in instructions.
enum QRegister {
#define X(val, encode, name, cc_arg, scratch, preserved, stackptr, frameptr,   \
          isGPR, isInt, isI64Pair, isFP32, isFP64, isVec128, alias_init)       \
  Encoded_##val = encode,
  REGARM32_VEC128_TABLE
#undef X
      Encoded_Not_QReg = -1
};

extern struct RegTableType {
  const char *Name;
  unsigned Encoding : 10;
  unsigned CCArg : 6;
  unsigned Scratch : 1;
  unsigned Preserved : 1;
  unsigned StackPtr : 1;
  unsigned FramePtr : 1;
  unsigned IsGPR : 1;
  unsigned IsInt : 1;
  unsigned IsI64Pair : 1;
  unsigned IsFP32 : 1;
  unsigned IsFP64 : 1;
  unsigned IsVec128 : 1;
#define NUM_ALIASES_BITS 3
  SizeT NumAliases : (NUM_ALIASES_BITS + 1);
  uint16_t Aliases[1 << NUM_ALIASES_BITS];
#undef NUM_ALIASES_BITS
} RegTable[Reg_NUM];

static inline void assertValidRegNum(RegNumT RegNum) {
  (void)RegNum;
  assert(RegNum.hasValue());
}

static inline bool isGPRegister(RegNumT RegNum) {
  RegNum.assertIsValid();
  return RegTable[RegNum].IsGPR;
}

static constexpr inline SizeT getNumGPRegs() {
  return 0
#define X(val, encode, name, cc_arg, scratch, preserved, stackptr, frameptr,   \
          isGPR, isInt, isI64Pair, isFP32, isFP64, isVec128, alias_init)       \
  +(isGPR)
      REGARM32_TABLE
#undef X
      ;
}

static inline GPRRegister getEncodedGPR(RegNumT RegNum) {
  RegNum.assertIsValid();
  return GPRRegister(RegTable[RegNum].Encoding);
}

static constexpr inline SizeT getNumGPRs() {
  return 0
#define X(val, encode, name, cc_arg, scratch, preserved, stackptr, frameptr,   \
          isGPR, isInt, isI64Pair, isFP32, isFP64, isVec128, alias_init)       \
  +(isGPR)
      REGARM32_TABLE
#undef X
      ;
}

static inline bool isGPR(RegNumT RegNum) {
  RegNum.assertIsValid();
  return RegTable[RegNum].IsGPR;
}

static inline GPRRegister getI64PairFirstGPRNum(RegNumT RegNum) {
  RegNum.assertIsValid();
  return GPRRegister(RegTable[RegNum].Encoding);
}

static inline GPRRegister getI64PairSecondGPRNum(RegNumT RegNum) {
  RegNum.assertIsValid();
  return GPRRegister(RegTable[RegNum].Encoding + 1);
}

static inline bool isI64RegisterPair(RegNumT RegNum) {
  RegNum.assertIsValid();
  return RegTable[RegNum].IsI64Pair;
}

static inline bool isEncodedSReg(RegNumT RegNum) {
  RegNum.assertIsValid();
  return RegTable[RegNum].IsFP32;
}

static constexpr inline SizeT getNumSRegs() {
  return 0
#define X(val, encode, name, cc_arg, scratch, preserved, stackptr, frameptr,   \
          isGPR, isInt, isI64Pair, isFP32, isFP64, isVec128, alias_init)       \
  +(isFP32)
      REGARM32_TABLE
#undef X
      ;
}

static inline SRegister getEncodedSReg(RegNumT RegNum) {
  RegNum.assertIsValid();
  return SRegister(RegTable[RegNum].Encoding);
}

static inline bool isEncodedDReg(RegNumT RegNum) {
  RegNum.assertIsValid();
  return RegTable[RegNum].IsFP64;
}

static constexpr inline SizeT getNumDRegs() {
  return 0
#define X(val, encode, name, cc_arg, scratch, preserved, stackptr, frameptr,   \
          isGPR, isInt, isI64Pair, isFP32, isFP64, isVec128, alias_init)       \
  +(isFP64)
      REGARM32_TABLE
#undef X
      ;
}

static inline DRegister getEncodedDReg(RegNumT RegNum) {
  RegNum.assertIsValid();
  return DRegister(RegTable[RegNum].Encoding);
}

static inline bool isEncodedQReg(RegNumT RegNum) {
  RegNum.assertIsValid();
  return RegTable[RegNum].IsVec128;
}

static inline QRegister getEncodedQReg(RegNumT RegNum) {
  assert(isEncodedQReg(RegNum));
  return QRegister(RegTable[RegNum].Encoding);
}

static inline const char *getRegName(RegNumT RegNum) {
  RegNum.assertIsValid();
  return RegTable[RegNum].Name;
}

// Extend enum RegClass with ARM32-specific register classes.
enum RegClassARM32 : uint8_t {
  RCARM32_QtoS = RC_Target, // Denotes Q registers that are aliased by S
                            // registers.
  RCARM32_NUM
};

} // end of namespace RegARM32
} // end of namespace ARM32
} // end of namespace Ice

#endif // SUBZERO_SRC_ICEREGISTERSARM32_H
