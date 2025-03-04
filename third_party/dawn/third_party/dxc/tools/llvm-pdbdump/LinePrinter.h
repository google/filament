//===- LinePrinter.h ------------------------------------------ *- C++ --*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TOOLS_LLVMPDBDUMP_LINEPRINTER_H
#define LLVM_TOOLS_LLVMPDBDUMP_LINEPRINTER_H

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Regex.h"

#include <list>

namespace llvm {

class LinePrinter {
  friend class WithColor;

public:
  LinePrinter(int Indent, raw_ostream &Stream);

  void Indent();
  void Unindent();
  void NewLine();

  raw_ostream &getStream() { return OS; }
  int getIndentLevel() const { return CurrentIndent; }

  bool IsTypeExcluded(llvm::StringRef TypeName);
  bool IsSymbolExcluded(llvm::StringRef SymbolName);
  bool IsCompilandExcluded(llvm::StringRef CompilandName);

private:
  template <typename Iter>
  void SetFilters(std::list<Regex> &List, Iter Begin, Iter End) {
    List.clear();
    for (; Begin != End; ++Begin)
      List.emplace_back(StringRef(*Begin));
  }

  raw_ostream &OS;
  int IndentSpaces;
  int CurrentIndent;

  std::list<Regex> CompilandFilters;
  std::list<Regex> TypeFilters;
  std::list<Regex> SymbolFilters;
};

template <class T>
inline raw_ostream &operator<<(LinePrinter &Printer, const T &Item) {
  Printer.getStream() << Item;
  return Printer.getStream();
}

enum class PDB_ColorItem {
  None,
  Address,
  Type,
  Keyword,
  Offset,
  Identifier,
  Path,
  SectionHeader,
  LiteralValue,
  Register,
};

class WithColor {
public:
  WithColor(LinePrinter &P, PDB_ColorItem C);
  ~WithColor();

  raw_ostream &get() { return OS; }

private:
  void translateColor(PDB_ColorItem C, raw_ostream::Colors &Color,
                      bool &Bold) const;
  raw_ostream &OS;
};
}

#endif
