//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TOOLS_LLVM_OBJDUMP_LLVM_OBJDUMP_H
#define LLVM_TOOLS_LLVM_OBJDUMP_LLVM_OBJDUMP_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {
namespace object {
  class COFFObjectFile;
  class MachOObjectFile;
  class ObjectFile;
  class RelocationRef;
}

extern cl::opt<std::string> TripleName;
extern cl::opt<std::string> ArchName;
extern cl::opt<std::string> MCPU;
extern cl::list<std::string> MAttrs;
extern cl::list<std::string> DumpSections;
extern cl::opt<bool> Disassemble;
extern cl::opt<bool> NoShowRawInsn;
extern cl::opt<bool> PrivateHeaders;
extern cl::opt<bool> ExportsTrie;
extern cl::opt<bool> Rebase;
extern cl::opt<bool> Bind;
extern cl::opt<bool> LazyBind;
extern cl::opt<bool> WeakBind;
extern cl::opt<bool> RawClangAST;
extern cl::opt<bool> UniversalHeaders;
extern cl::opt<bool> ArchiveHeaders;
extern cl::opt<bool> IndirectSymbols;
extern cl::opt<bool> DataInCode;
extern cl::opt<bool> LinkOptHints;
extern cl::opt<bool> InfoPlist;
extern cl::opt<bool> DylibsUsed;
extern cl::opt<bool> DylibId;
extern cl::opt<bool> ObjcMetaData;
extern cl::opt<std::string> DisSymName;
extern cl::opt<bool> NonVerbose;
extern cl::opt<bool> Relocations;
extern cl::opt<bool> SectionHeaders;
extern cl::opt<bool> SectionContents;
extern cl::opt<bool> SymbolTable;
extern cl::opt<bool> UnwindInfo;
extern cl::opt<bool> PrintImmHex;

// Various helper functions.
bool error(std::error_code ec);
bool RelocAddressLess(object::RelocationRef a, object::RelocationRef b);
void ParseInputMachO(StringRef Filename);
void printCOFFUnwindInfo(const object::COFFObjectFile* o);
void printMachOUnwindInfo(const object::MachOObjectFile* o);
void printMachOExportsTrie(const object::MachOObjectFile* o);
void printMachORebaseTable(const object::MachOObjectFile* o);
void printMachOBindTable(const object::MachOObjectFile* o);
void printMachOLazyBindTable(const object::MachOObjectFile* o);
void printMachOWeakBindTable(const object::MachOObjectFile* o);
void printELFFileHeader(const object::ObjectFile *o);
void printCOFFFileHeader(const object::ObjectFile *o);
void printMachOFileHeader(const object::ObjectFile *o);
void printExportsTrie(const object::ObjectFile *o);
void printRebaseTable(const object::ObjectFile *o);
void printBindTable(const object::ObjectFile *o);
void printLazyBindTable(const object::ObjectFile *o);
void printWeakBindTable(const object::ObjectFile *o);
void printRawClangAST(const object::ObjectFile *o);
void PrintRelocations(const object::ObjectFile *o);
void PrintSectionHeaders(const object::ObjectFile *o);
void PrintSectionContents(const object::ObjectFile *o);
void PrintSymbolTable(const object::ObjectFile *o);

} // end namespace llvm

#endif
