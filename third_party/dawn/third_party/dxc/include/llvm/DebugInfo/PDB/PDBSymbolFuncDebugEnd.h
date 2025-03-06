//===- PDBSymbolFuncDebugEnd.h - function end bounds info -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_DEBUGINFO_PDB_PDBSYMBOLFUNCDEBUGEND_H
#define LLVM_DEBUGINFO_PDB_PDBSYMBOLFUNCDEBUGEND_H

#include "PDBSymbol.h"
#include "PDBTypes.h"

namespace llvm {

class raw_ostream;

class PDBSymbolFuncDebugEnd : public PDBSymbol {
public:
  PDBSymbolFuncDebugEnd(const IPDBSession &PDBSession,
                        std::unique_ptr<IPDBRawSymbol> FuncDebugEndSymbol);

  DECLARE_PDB_SYMBOL_CONCRETE_TYPE(PDB_SymType::FuncDebugEnd)

  void dump(PDBSymDumper &Dumper) const override;

  FORWARD_SYMBOL_METHOD(getAddressOffset)
  FORWARD_SYMBOL_METHOD(getAddressSection)
  FORWARD_SYMBOL_METHOD(hasCustomCallingConvention)
  FORWARD_SYMBOL_METHOD(hasFarReturn)
  FORWARD_SYMBOL_METHOD(hasInterruptReturn)
  FORWARD_SYMBOL_METHOD(isStatic)
  FORWARD_SYMBOL_METHOD(getLexicalParentId)
  FORWARD_SYMBOL_METHOD(getLocationType)
  FORWARD_SYMBOL_METHOD(hasNoInlineAttribute)
  FORWARD_SYMBOL_METHOD(hasNoReturnAttribute)
  FORWARD_SYMBOL_METHOD(isUnreached)
  FORWARD_SYMBOL_METHOD(getOffset)
  FORWARD_SYMBOL_METHOD(hasOptimizedCodeDebugInfo)
  FORWARD_SYMBOL_METHOD(getRelativeVirtualAddress)
  FORWARD_SYMBOL_METHOD(getSymIndexId)
  FORWARD_SYMBOL_METHOD(getVirtualAddress)
};

} // namespace llvm

#endif // LLVM_DEBUGINFO_PDB_PDBSYMBOLFUNCDEBUGEND_H
