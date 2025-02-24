//===- subzero/src/IceTargetLoweringX86RegClass.h - x86 reg class -*- C++ -*-=//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the X86 register class extensions.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICETARGETLOWERINGX86REGCLASS_H
#define SUBZERO_SRC_ICETARGETLOWERINGX86REGCLASS_H

#include "IceOperand.h" // RC_Target

namespace Ice {
namespace X86 {

// Extend enum RegClass with x86-specific register classes.
enum RegClassX86 : uint8_t {
  RCX86_Is64To8 = RC_Target, // 64-bit GPR trivially truncable to 8-bit
  RCX86_Is32To8,             // 32-bit GPR trivially truncable to 8-bit
  RCX86_Is16To8,             // 16-bit GPR trivially truncable to 8-bit
  RCX86_IsTrunc8Rcvr,        // 8-bit GPR that can receive a trunc operation
  RCX86_IsAhRcvr,            // 8-bit GPR that can be a mov dest from %ah
  RCX86_NUM
};

} // end of namespace X86
} // end of namespace Ice

#endif // SUBZERO_SRC_ICETARGETLOWERINGX86REGCLASS_H
