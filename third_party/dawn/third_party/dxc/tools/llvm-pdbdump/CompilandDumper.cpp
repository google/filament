//===- CompilandDumper.cpp - llvm-pdbdump compiland symbol dumper *- C++ *-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "CompilandDumper.h"
#include "LinePrinter.h"
#include "llvm-pdbdump.h"

#include "llvm/DebugInfo/PDB/IPDBEnumChildren.h"
#include "llvm/DebugInfo/PDB/IPDBSession.h"
#include "llvm/DebugInfo/PDB/PDBExtras.h"
#include "llvm/DebugInfo/PDB/PDBSymbol.h"
#include "llvm/DebugInfo/PDB/PDBSymbolCompiland.h"
#include "llvm/DebugInfo/PDB/PDBSymbolData.h"
#include "llvm/DebugInfo/PDB/PDBSymbolFunc.h"
#include "llvm/DebugInfo/PDB/PDBSymbolFuncDebugEnd.h"
#include "llvm/DebugInfo/PDB/PDBSymbolFuncDebugStart.h"
#include "llvm/DebugInfo/PDB/PDBSymbolLabel.h"
#include "llvm/DebugInfo/PDB/PDBSymbolThunk.h"
#include "llvm/DebugInfo/PDB/PDBSymbolTypeFunctionSig.h"
#include "llvm/DebugInfo/PDB/PDBSymbolUnknown.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

#include "FunctionDumper.h"

#include <utility>
#include <vector>

using namespace llvm;

CompilandDumper::CompilandDumper(LinePrinter &P)
    : PDBSymDumper(true), Printer(P) {}

void CompilandDumper::dump(const PDBSymbolCompilandDetails &Symbol) {}

void CompilandDumper::dump(const PDBSymbolCompilandEnv &Symbol) {}

void CompilandDumper::start(const PDBSymbolCompiland &Symbol, bool Children) {
  std::string FullName = Symbol.getName();
  if (Printer.IsCompilandExcluded(FullName))
    return;

  Printer.NewLine();
  WithColor(Printer, PDB_ColorItem::Path).get() << FullName;
  if (!Children)
    return;

  auto ChildrenEnum = Symbol.findAllChildren();
  Printer.Indent();
  while (auto Child = ChildrenEnum->getNext())
    Child->dump(*this);
  Printer.Unindent();
}

void CompilandDumper::dump(const PDBSymbolData &Symbol) {
  if (Printer.IsSymbolExcluded(Symbol.getName()))
    return;

  Printer.NewLine();

  switch (auto LocType = Symbol.getLocationType()) {
  case PDB_LocType::Static:
    Printer << "data: ";
    WithColor(Printer, PDB_ColorItem::Address).get()
        << "[" << format_hex(Symbol.getVirtualAddress(), 10) << "]";
    break;
  case PDB_LocType::Constant:
    Printer << "constant: ";
    WithColor(Printer, PDB_ColorItem::LiteralValue).get()
        << "[" << Symbol.getValue() << "]";
    break;
  default:
    Printer << "data(unexpected type=" << LocType << ")";
  }

  Printer << " ";
  WithColor(Printer, PDB_ColorItem::Identifier).get() << Symbol.getName();
}

void CompilandDumper::dump(const PDBSymbolFunc &Symbol) {
  if (Symbol.getLength() == 0)
    return;
  if (Printer.IsSymbolExcluded(Symbol.getName()))
    return;

  Printer.NewLine();
  FunctionDumper Dumper(Printer);
  Dumper.start(Symbol, FunctionDumper::PointerType::None);
}

void CompilandDumper::dump(const PDBSymbolLabel &Symbol) {
  if (Printer.IsSymbolExcluded(Symbol.getName()))
    return;

  Printer.NewLine();
  Printer << "label ";
  WithColor(Printer, PDB_ColorItem::Address).get()
      << "[" << format_hex(Symbol.getVirtualAddress(), 10) << "] ";
  WithColor(Printer, PDB_ColorItem::Identifier).get() << Symbol.getName();
}

void CompilandDumper::dump(const PDBSymbolThunk &Symbol) {
  if (Printer.IsSymbolExcluded(Symbol.getName()))
    return;

  Printer.NewLine();
  Printer << "thunk ";
  PDB_ThunkOrdinal Ordinal = Symbol.getThunkOrdinal();
  uint64_t VA = Symbol.getVirtualAddress();
  if (Ordinal == PDB_ThunkOrdinal::TrampIncremental) {
    uint64_t Target = Symbol.getTargetVirtualAddress();
    WithColor(Printer, PDB_ColorItem::Address).get() << format_hex(VA, 10);
    Printer << " -> ";
    WithColor(Printer, PDB_ColorItem::Address).get() << format_hex(Target, 10);
  } else {
    WithColor(Printer, PDB_ColorItem::Address).get()
        << "[" << format_hex(VA, 10) << " - "
        << format_hex(VA + Symbol.getLength(), 10) << "]";
  }
  Printer << " (";
  WithColor(Printer, PDB_ColorItem::Register).get() << Ordinal;
  Printer << ") ";
  std::string Name = Symbol.getName();
  if (!Name.empty())
    WithColor(Printer, PDB_ColorItem::Identifier).get() << Name;
}

void CompilandDumper::dump(const PDBSymbolTypeTypedef &Symbol) {}

void CompilandDumper::dump(const PDBSymbolUnknown &Symbol) {
  Printer.NewLine();
  Printer << "unknown (" << Symbol.getSymTag() << ")";
}
