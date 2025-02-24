//===- subzero/src/IceConditionCodesMIPS32.h - Condition Codes --*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the condition codes for MIPS32.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICECONDITIONCODESMIPS32_H
#define SUBZERO_SRC_ICECONDITIONCODESMIPS32_H

#include "IceDefs.h"
#include "IceInstMIPS32.def"

namespace Ice {

class CondMIPS32 {
  CondMIPS32() = delete;
  CondMIPS32(const CondMIPS32 &) = delete;
  CondMIPS32 &operator=(const CondMIPS32 &) = delete;

public:
  /// An enum of codes used for conditional instructions. The enum value should
  /// match the value used to encode operands in binary instructions.
  enum Cond {
#define X(tag, opp, emit) tag,
    ICEINSTMIPS32COND_TABLE
#undef X
  };

  static bool isDefined(Cond C) { return C != kNone; }

  static bool isUnconditional(Cond C) { return !isDefined(C) || C == AL; }
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICECONDITIONCODESMIPS32_H
