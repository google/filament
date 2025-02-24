//===- subzero/src/IceConditionCodesARM32.h - Condition Codes ---*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the condition codes for ARM32.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICECONDITIONCODESARM32_H
#define SUBZERO_SRC_ICECONDITIONCODESARM32_H

#include "IceDefs.h"
#include "IceInstARM32.def"

namespace Ice {

class CondARM32 {
  CondARM32() = delete;
  CondARM32(const CondARM32 &) = delete;
  CondARM32 &operator=(const CondARM32 &) = delete;

public:
  /// An enum of codes used for conditional instructions. The enum value should
  /// match the value used to encode operands in binary instructions.
  enum Cond {
#define X(tag, encode, opp, emit) tag = encode,
    ICEINSTARM32COND_TABLE
#undef X
  };

  static bool isDefined(Cond C) { return C != kNone; }

  static bool isUnconditional(Cond C) { return !isDefined(C) || C == AL; }
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICECONDITIONCODESARM32_H
