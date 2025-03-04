//===- TableGen.cpp - Top-Level TableGen implementation for LLVM ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the main function for LLVM's TableGen.
//
//===----------------------------------------------------------------------===//

#include "TableGenBackends.h" // Declares all backends.
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Main.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/SetTheory.h"

// HLSL Change Starts
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MSFileSystem.h"
// HLSL Change Ends

using namespace llvm;

enum ActionType {
  PrintRecords,
  GenEmitter,
  GenRegisterInfo,
  GenInstrInfo,
  GenAsmWriter,
  GenAsmMatcher,
  GenDisassembler,
  GenPseudoLowering,
  GenCallingConv,
  GenDAGISel,
  GenDFAPacketizer,
  GenFastISel,
  GenSubtarget,
  GenIntrinsic,
  GenTgtIntrinsic,
  PrintEnums,
  PrintSets,
  GenOptParserDefs,
  GenCTags
};

namespace {
  cl::opt<ActionType>
  Action(cl::desc("Action to perform:"),
         cl::values(clEnumValN(PrintRecords, "print-records",
                               "Print all records to stdout (default)"),
                    clEnumValN(GenEmitter, "gen-emitter",
                               "Generate machine code emitter"),
                    clEnumValN(GenRegisterInfo, "gen-register-info",
                               "Generate registers and register classes info"),
                    clEnumValN(GenInstrInfo, "gen-instr-info",
                               "Generate instruction descriptions"),
                    clEnumValN(GenCallingConv, "gen-callingconv",
                               "Generate calling convention descriptions"),
                    clEnumValN(GenAsmWriter, "gen-asm-writer",
                               "Generate assembly writer"),
                    clEnumValN(GenDisassembler, "gen-disassembler",
                               "Generate disassembler"),
                    clEnumValN(GenPseudoLowering, "gen-pseudo-lowering",
                               "Generate pseudo instruction lowering"),
                    clEnumValN(GenAsmMatcher, "gen-asm-matcher",
                               "Generate assembly instruction matcher"),
                    clEnumValN(GenDAGISel, "gen-dag-isel",
                               "Generate a DAG instruction selector"),
                    clEnumValN(GenDFAPacketizer, "gen-dfa-packetizer",
                               "Generate DFA Packetizer for VLIW targets"),
                    clEnumValN(GenFastISel, "gen-fast-isel",
                               "Generate a \"fast\" instruction selector"),
                    clEnumValN(GenSubtarget, "gen-subtarget",
                               "Generate subtarget enumerations"),
                    clEnumValN(GenIntrinsic, "gen-intrinsic",
                               "Generate intrinsic information"),
                    clEnumValN(GenTgtIntrinsic, "gen-tgt-intrinsic",
                               "Generate target intrinsic information"),
                    clEnumValN(PrintEnums, "print-enums",
                               "Print enum values for a class"),
                    clEnumValN(PrintSets, "print-sets",
                               "Print expanded sets for testing DAG exprs"),
                    clEnumValN(GenOptParserDefs, "gen-opt-parser-defs",
                               "Generate option definitions"),
                    clEnumValN(GenCTags, "gen-ctags",
                               "Generate ctags-compatible index"),
                    clEnumValEnd));

  cl::opt<std::string>
  Class("class", cl::desc("Print Enum list for this class"),
          cl::value_desc("class name"));

bool LLVMTableGenMain(raw_ostream &OS, RecordKeeper &Records) {
  switch (Action) {
  case PrintRecords:
    OS << Records;           // No argument, dump all contents
    break;
  case GenEmitter:
    EmitCodeEmitter(Records, OS);
    break;
  case GenRegisterInfo:
    EmitRegisterInfo(Records, OS);
    break;
  case GenInstrInfo:
    EmitInstrInfo(Records, OS);
    break;
  case GenCallingConv:
    EmitCallingConv(Records, OS);
    break;
  case GenAsmWriter:
    EmitAsmWriter(Records, OS);
    break;
  case GenAsmMatcher:
    EmitAsmMatcher(Records, OS);
    break;
  case GenDisassembler:
    EmitDisassembler(Records, OS);
    break;
  case GenPseudoLowering:
    EmitPseudoLowering(Records, OS);
    break;
  case GenDAGISel:
    EmitDAGISel(Records, OS);
    break;
  case GenDFAPacketizer:
    EmitDFAPacketizer(Records, OS);
    break;
  case GenFastISel:
    EmitFastISel(Records, OS);
    break;
  case GenSubtarget:
    EmitSubtarget(Records, OS);
    break;
  case GenIntrinsic:
    EmitIntrinsics(Records, OS);
    break;
  case GenTgtIntrinsic:
    EmitIntrinsics(Records, OS, true);
    break;
  case GenOptParserDefs:
    EmitOptParser(Records, OS);
    break;
  case PrintEnums:
  {
    for (Record *Rec : Records.getAllDerivedDefinitions(Class))
      OS << Rec->getName() << ", ";
    OS << "\n";
    break;
  }
  case PrintSets:
  {
    SetTheory Sets;
    Sets.addFieldExpander("Set", "Elements");
    for (Record *Rec : Records.getAllDerivedDefinitions("Set")) {
      OS << Rec->getName() << " = [";
      const std::vector<Record*> *Elts = Sets.expand(Rec);
      assert(Elts && "Couldn't expand Set instance");
      for (Record *Elt : *Elts)
        OS << ' ' << Elt->getName();
      OS << " ]\n";
    }
    break;
  }
  case GenCTags:
    EmitCTags(Records, OS);
    break;
  }

  return false;
}
}

int main(int argc, char **argv) {
  // HLSL Change Starts
  if (std::error_code ec = llvm::sys::fs::SetupPerThreadFileSystem())
      return 1;
  llvm::sys::fs::AutoCleanupPerThreadFileSystem auto_cleanup_fs;
  llvm::sys::fs::MSFileSystem* msfPtr;
  HRESULT hr;
  if (!SUCCEEDED(hr = CreateMSFileSystemForDisk(&msfPtr)))
    return 1;
  std::unique_ptr<llvm::sys::fs::MSFileSystem> msf(msfPtr);
  llvm::sys::fs::AutoPerThreadSystem pts(msf.get());
  // HLSL Change Ends

  // sys::PrintStackTraceOnErrorSignal(); // HLSL Change
  // PrettyStackTraceProgram X(argc, argv); // HLSL Change
  cl::ParseCommandLineOptions(argc, argv);

  return TableGenMain(argv[0], &LLVMTableGenMain);
}

#ifdef __has_feature
#if __has_feature(address_sanitizer)
#include <sanitizer/lsan_interface.h>
// Disable LeakSanitizer for this binary as it has too many leaks that are not
// very interesting to fix. See compiler-rt/include/sanitizer/lsan_interface.h .
int __lsan_is_turned_off() { return 1; }
#endif  // __has_feature(address_sanitizer)
#endif  // defined(__has_feature)
