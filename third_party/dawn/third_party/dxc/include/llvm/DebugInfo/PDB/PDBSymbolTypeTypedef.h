//===- PDBSymbolTypeTypedef.h - typedef type info ---------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_DEBUGINFO_PDB_PDBSYMBOLTYPETYPEDEF_H
#define LLVM_DEBUGINFO_PDB_PDBSYMBOLTYPETYPEDEF_H

#include "PDBSymbol.h"
#include "PDBTypes.h"

namespace llvm {

class raw_ostream;

class PDBSymbolTypeTypedef : public PDBSymbol {
public:
  PDBSymbolTypeTypedef(const IPDBSession &PDBSession,
                       std::unique_ptr<IPDBRawSymbol> Symbol);

  DECLARE_PDB_SYMBOL_CONCRETE_TYPE(PDB_SymType::Typedef)

  void dump(PDBSymDumper &Dumper) const override;

  FORWARD_SYMBOL_METHOD(getBuiltinType)
  FORWARD_SYMBOL_METHOD(getClassParentId)
  FORWARD_SYMBOL_METHOD(hasConstructor)
  FORWARD_SYMBOL_METHOD(isConstType)
  FORWARD_SYMBOL_METHOD(hasAssignmentOperator)
  FORWARD_SYMBOL_METHOD(hasCastOperator)
  FORWARD_SYMBOL_METHOD(hasNestedTypes)
  FORWARD_SYMBOL_METHOD(getLength)
  FORWARD_SYMBOL_METHOD(getLexicalParentId)
  FORWARD_SYMBOL_METHOD(getName)
  FORWARD_SYMBOL_METHOD(isNested)
  FORWARD_SYMBOL_METHOD(hasOverloadedOperator)
  FORWARD_SYMBOL_METHOD(isPacked)
  FORWARD_SYMBOL_METHOD(isReference)
  FORWARD_SYMBOL_METHOD(isScoped)
  FORWARD_SYMBOL_METHOD(getSymIndexId)
  FORWARD_SYMBOL_METHOD(getTypeId)
  FORWARD_SYMBOL_METHOD(getUdtKind)
  FORWARD_SYMBOL_METHOD(isUnalignedType)
  FORWARD_SYMBOL_METHOD(getVirtualTableShapeId)
  FORWARD_SYMBOL_METHOD(isVolatileType)
};

} // namespace llvm

#endif // LLVM_DEBUGINFO_PDB_PDBSYMBOLTYPETYPEDEF_H
