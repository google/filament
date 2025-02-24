//===- subzero/src/IceVariableSplitting.h - Local var splitting -*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Aggressive block-local variable splitting to improve linear-scan
/// register allocation.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEVARIABLESPLITTING_H
#define SUBZERO_SRC_ICEVARIABLESPLITTING_H

namespace Ice {

void splitBlockLocalVariables(class Cfg *Func);

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEVARIABLESPLITTING_H
