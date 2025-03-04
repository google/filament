//===- BuiltinDumper.h ---------------------------------------- *- C++ --*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TOOLS_LLVMPDBDUMP_BUILTINDUMPER_H
#define LLVM_TOOLS_LLVMPDBDUMP_BUILTINDUMPER_H

#include "llvm/DebugInfo/PDB/PDBSymDumper.h"

namespace llvm {

class LinePrinter;

class BuiltinDumper : public PDBSymDumper {
public:
  BuiltinDumper(LinePrinter &P);

  void start(const PDBSymbolTypeBuiltin &Symbol);

private:
  LinePrinter &Printer;
};
}

#endif
