//===- llvm-pdbdump.cpp - Dump debug info from a PDB file -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Dumps debug information present in PDB files.  This utility makes use of
// the Microsoft Windows SDK, so will not compile or run on non-Windows
// platforms.
//
//===----------------------------------------------------------------------===//

#include "llvm-pdbdump.h"
#include "CompilandDumper.h"
#include "ExternalSymbolDumper.h"
#include "FunctionDumper.h"
#include "LinePrinter.h"
#include "TypeDumper.h"
#include "VariableDumper.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Config/config.h"
#include "llvm/DebugInfo/PDB/IPDBEnumChildren.h"
#include "llvm/DebugInfo/PDB/IPDBRawSymbol.h"
#include "llvm/DebugInfo/PDB/IPDBSession.h"
#include "llvm/DebugInfo/PDB/PDB.h"
#include "llvm/DebugInfo/PDB/PDBSymbolCompiland.h"
#include "llvm/DebugInfo/PDB/PDBSymbolData.h"
#include "llvm/DebugInfo/PDB/PDBSymbolExe.h"
#include "llvm/DebugInfo/PDB/PDBSymbolFunc.h"
#include "llvm/DebugInfo/PDB/PDBSymbolThunk.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Signals.h"

#if defined(HAVE_DIA_SDK)
#include <windows.h>
#endif

using namespace llvm;

namespace opts {

enum class PDB_DumpType { ByType, ByObjFile, Both };

cl::list<std::string> InputFilenames(cl::Positional,
                                     cl::desc("<input PDB files>"),
                                     cl::OneOrMore);

cl::OptionCategory TypeCategory("Symbol Type Options");
cl::OptionCategory FilterCategory("Filtering Options");
cl::OptionCategory OtherOptions("Other Options");

cl::opt<bool> Compilands("compilands", cl::desc("Display compilands"),
                         cl::cat(TypeCategory));
cl::opt<bool> Symbols("symbols", cl::desc("Display symbols for each compiland"),
                      cl::cat(TypeCategory));
cl::opt<bool> Globals("globals", cl::desc("Dump global symbols"),
                      cl::cat(TypeCategory));
cl::opt<bool> Externals("externals", cl::desc("Dump external symbols"),
                        cl::cat(TypeCategory));
cl::opt<bool> Types("types", cl::desc("Display types"), cl::cat(TypeCategory));
cl::opt<bool>
    All("all", cl::desc("Implies all other options in 'Symbol Types' category"),
        cl::cat(TypeCategory));

cl::opt<uint64_t> LoadAddress(
    "load-address",
    cl::desc("Assume the module is loaded at the specified address"),
    cl::cat(OtherOptions));

cl::list<std::string>
    ExcludeTypes("exclude-types",
                 cl::desc("Exclude types by regular expression"),
                 cl::ZeroOrMore, cl::cat(FilterCategory));
cl::list<std::string>
    ExcludeSymbols("exclude-symbols",
                   cl::desc("Exclude symbols by regular expression"),
                   cl::ZeroOrMore, cl::cat(FilterCategory));
cl::list<std::string>
    ExcludeCompilands("exclude-compilands",
                      cl::desc("Exclude compilands by regular expression"),
                      cl::ZeroOrMore, cl::cat(FilterCategory));
cl::opt<bool> ExcludeCompilerGenerated(
    "no-compiler-generated",
    cl::desc("Don't show compiler generated types and symbols"),
    cl::cat(FilterCategory));
cl::opt<bool>
    ExcludeSystemLibraries("no-system-libs",
                           cl::desc("Don't show symbols from system libraries"),
                           cl::cat(FilterCategory));
cl::opt<bool> NoClassDefs("no-class-definitions",
                          cl::desc("Don't display full class definitions"),
                          cl::cat(FilterCategory));
cl::opt<bool> NoEnumDefs("no-enum-definitions",
                         cl::desc("Don't display full enum definitions"),
                         cl::cat(FilterCategory));
}

static void dumpInput(StringRef Path) {
  std::unique_ptr<IPDBSession> Session;
  PDB_ErrorCode Error =
      llvm::loadDataForPDB(PDB_ReaderType::DIA, Path, Session);
  switch (Error) {
  case PDB_ErrorCode::Success:
    break;
  case PDB_ErrorCode::NoPdbImpl:
    outs() << "Reading PDBs is not supported on this platform.\n";
    return;
  case PDB_ErrorCode::InvalidPath:
    outs() << "Unable to load PDB at '" << Path
           << "'.  Check that the file exists and is readable.\n";
    return;
  case PDB_ErrorCode::InvalidFileFormat:
    outs() << "Unable to load PDB at '" << Path
           << "'.  The file has an unrecognized format.\n";
    return;
  default:
    outs() << "Unable to load PDB at '" << Path
           << "'.  An unknown error occured.\n";
    return;
  }
  if (opts::LoadAddress)
    Session->setLoadAddress(opts::LoadAddress);

  LinePrinter Printer(2, outs());

  auto GlobalScope(Session->getGlobalScope());
  std::string FileName(GlobalScope->getSymbolsFileName());

  WithColor(Printer, PDB_ColorItem::None).get() << "Summary for ";
  WithColor(Printer, PDB_ColorItem::Path).get() << FileName;
  Printer.Indent();
  uint64_t FileSize = 0;

  Printer.NewLine();
  WithColor(Printer, PDB_ColorItem::Identifier).get() << "Size";
  if (!llvm::sys::fs::file_size(FileName, FileSize)) {
    Printer << ": " << FileSize << " bytes";
  } else {
    Printer << ": (Unable to obtain file size)";
  }

  Printer.NewLine();
  WithColor(Printer, PDB_ColorItem::Identifier).get() << "Guid";
  Printer << ": " << GlobalScope->getGuid();

  Printer.NewLine();
  WithColor(Printer, PDB_ColorItem::Identifier).get() << "Age";
  Printer << ": " << GlobalScope->getAge();

  Printer.NewLine();
  WithColor(Printer, PDB_ColorItem::Identifier).get() << "Attributes";
  Printer << ": ";
  if (GlobalScope->hasCTypes())
    outs() << "HasCTypes ";
  if (GlobalScope->hasPrivateSymbols())
    outs() << "HasPrivateSymbols ";
  Printer.Unindent();

  if (opts::Compilands) {
    Printer.NewLine();
    WithColor(Printer, PDB_ColorItem::SectionHeader).get()
        << "---COMPILANDS---";
    Printer.Indent();
    auto Compilands = GlobalScope->findAllChildren<PDBSymbolCompiland>();
    CompilandDumper Dumper(Printer);
    while (auto Compiland = Compilands->getNext())
      Dumper.start(*Compiland, false);
    Printer.Unindent();
  }

  if (opts::Types) {
    Printer.NewLine();
    WithColor(Printer, PDB_ColorItem::SectionHeader).get() << "---TYPES---";
    Printer.Indent();
    TypeDumper Dumper(Printer);
    Dumper.start(*GlobalScope);
    Printer.Unindent();
  }

  if (opts::Symbols) {
    Printer.NewLine();
    WithColor(Printer, PDB_ColorItem::SectionHeader).get() << "---SYMBOLS---";
    Printer.Indent();
    auto Compilands = GlobalScope->findAllChildren<PDBSymbolCompiland>();
    CompilandDumper Dumper(Printer);
    while (auto Compiland = Compilands->getNext())
      Dumper.start(*Compiland, true);
    Printer.Unindent();
  }

  if (opts::Globals) {
    Printer.NewLine();
    WithColor(Printer, PDB_ColorItem::SectionHeader).get() << "---GLOBALS---";
    Printer.Indent();
    {
      FunctionDumper Dumper(Printer);
      auto Functions = GlobalScope->findAllChildren<PDBSymbolFunc>();
      while (auto Function = Functions->getNext()) {
        Printer.NewLine();
        Dumper.start(*Function, FunctionDumper::PointerType::None);
      }
    }
    {
      auto Vars = GlobalScope->findAllChildren<PDBSymbolData>();
      VariableDumper Dumper(Printer);
      while (auto Var = Vars->getNext())
        Dumper.start(*Var);
    }
    {
      auto Thunks = GlobalScope->findAllChildren<PDBSymbolThunk>();
      CompilandDumper Dumper(Printer);
      while (auto Thunk = Thunks->getNext())
        Dumper.dump(*Thunk);
    }
    Printer.Unindent();
  }
  if (opts::Externals) {
    Printer.NewLine();
    WithColor(Printer, PDB_ColorItem::SectionHeader).get() << "---EXTERNALS---";
    Printer.Indent();
    ExternalSymbolDumper Dumper(Printer);
    Dumper.start(*GlobalScope);
  }
  outs().flush();
}

int main(int argc_, const char *argv_[]) {
  // Print a stack trace if we signal out.
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(argc_, argv_);

  SmallVector<const char *, 256> argv;
  llvm::SpecificBumpPtrAllocator<char> ArgAllocator;
  std::error_code EC = llvm::sys::Process::GetArgumentVector(
      argv, llvm::makeArrayRef(argv_, argc_), ArgAllocator);
  if (EC) {
    llvm::errs() << "error: couldn't get arguments: " << EC.message() << '\n';
    return 1;
  }

  llvm_shutdown_obj Y; // Call llvm_shutdown() on exit.

  cl::ParseCommandLineOptions(argv.size(), argv.data(), "LLVM PDB Dumper\n");
  if (opts::All) {
    opts::Compilands = true;
    opts::Symbols = true;
    opts::Globals = true;
    opts::Types = true;
    opts::Externals = true;
  }
  if (opts::ExcludeCompilerGenerated) {
    opts::ExcludeTypes.push_back("__vc_attributes");
    opts::ExcludeCompilands.push_back("* Linker *");
  }
  if (opts::ExcludeSystemLibraries) {
    opts::ExcludeCompilands.push_back(
        "f:\\binaries\\Intermediate\\vctools\\crt_bld");
  }

#if defined(HAVE_DIA_SDK)
  CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif

  std::for_each(opts::InputFilenames.begin(), opts::InputFilenames.end(),
                dumpInput);

#if defined(HAVE_DIA_SDK)
  CoUninitialize();
#endif

  return 0;
}
