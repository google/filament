//===- LinePrinter.cpp ------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "LinePrinter.h"

#include "llvm-pdbdump.h"

#include "llvm/Support/Regex.h"

#include <algorithm>

using namespace llvm;

LinePrinter::LinePrinter(int Indent, llvm::raw_ostream &Stream)
    : OS(Stream), IndentSpaces(Indent), CurrentIndent(0) {
  SetFilters(TypeFilters, opts::ExcludeTypes.begin(), opts::ExcludeTypes.end());
  SetFilters(SymbolFilters, opts::ExcludeSymbols.begin(),
             opts::ExcludeSymbols.end());
  SetFilters(CompilandFilters, opts::ExcludeCompilands.begin(),
             opts::ExcludeCompilands.end());
}

void LinePrinter::Indent() { CurrentIndent += IndentSpaces; }

void LinePrinter::Unindent() {
  CurrentIndent = std::max(0, CurrentIndent - IndentSpaces);
}

void LinePrinter::NewLine() {
  OS << "\n";
  OS.indent(CurrentIndent);
}

bool LinePrinter::IsTypeExcluded(llvm::StringRef TypeName) {
  if (TypeName.empty())
    return false;

  for (auto &Expr : TypeFilters) {
    if (Expr.match(TypeName))
      return true;
  }
  return false;
}

bool LinePrinter::IsSymbolExcluded(llvm::StringRef SymbolName) {
  if (SymbolName.empty())
    return false;

  for (auto &Expr : SymbolFilters) {
    if (Expr.match(SymbolName))
      return true;
  }
  return false;
}

bool LinePrinter::IsCompilandExcluded(llvm::StringRef CompilandName) {
  if (CompilandName.empty())
    return false;

  for (auto &Expr : CompilandFilters) {
    if (Expr.match(CompilandName))
      return true;
  }
  return false;
}

WithColor::WithColor(LinePrinter &P, PDB_ColorItem C) : OS(P.OS) {
  if (C == PDB_ColorItem::None)
    OS.resetColor();
  else {
    raw_ostream::Colors Color;
    bool Bold;
    translateColor(C, Color, Bold);
    OS.changeColor(Color, Bold);
  }
}

WithColor::~WithColor() { OS.resetColor(); }

void WithColor::translateColor(PDB_ColorItem C, raw_ostream::Colors &Color,
                               bool &Bold) const {
  switch (C) {
  case PDB_ColorItem::Address:
    Color = raw_ostream::YELLOW;
    Bold = true;
    return;
  case PDB_ColorItem::Keyword:
    Color = raw_ostream::MAGENTA;
    Bold = true;
    return;
  case PDB_ColorItem::Register:
  case PDB_ColorItem::Offset:
    Color = raw_ostream::YELLOW;
    Bold = false;
    return;
  case PDB_ColorItem::Type:
    Color = raw_ostream::CYAN;
    Bold = true;
    return;
  case PDB_ColorItem::Identifier:
    Color = raw_ostream::CYAN;
    Bold = false;
    return;
  case PDB_ColorItem::Path:
    Color = raw_ostream::CYAN;
    Bold = false;
    return;
  case PDB_ColorItem::SectionHeader:
    Color = raw_ostream::RED;
    Bold = true;
    return;
  case PDB_ColorItem::LiteralValue:
    Color = raw_ostream::GREEN;
    Bold = true;
  default:
    return;
  }
}
