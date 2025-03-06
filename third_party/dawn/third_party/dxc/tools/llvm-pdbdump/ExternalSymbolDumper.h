//===- ExternalSymbolDumper.h --------------------------------- *- C++ --*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TOOLS_LLVMPDBDUMP_EXTERNALSYMBOLDUMPER_H
#define LLVM_TOOLS_LLVMPDBDUMP_EXTERNALSYMBOLDUMPER_H

#include "llvm/DebugInfo/PDB/PDBSymDumper.h"

namespace llvm {

class LinePrinter;

class ExternalSymbolDumper : public PDBSymDumper {
public:
  ExternalSymbolDumper(LinePrinter &P);

  void start(const PDBSymbolExe &Symbol);

  void dump(const PDBSymbolPublicSymbol &Symbol) override;

private:
  LinePrinter &Printer;
};
}

#endif
