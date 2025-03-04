//===-- llvm-dis.cpp - The low-level LLVM disassembler --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This utility may be invoked in the following manner:
//  llvm-dis [options]      - Read LLVM bitcode from stdin, write asm to stdout
//  llvm-dis [options] x.bc - Read LLVM bitcode from the x.bc file, write asm
//                            to the x.ll file.
//  Options:
//      --help   - Output information about command line switches
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/LLVMContext.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DataStream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/ToolOutputFile.h"
#include "dxc/DxilContainer/DxilContainer.h" // HLSL Change
#include <system_error>
using namespace llvm;

static cl::opt<std::string>
InputFilename(cl::Positional, cl::desc("<input bitcode>"), cl::init("-"));

static cl::opt<std::string>
OutputFilename("o", cl::desc("Override output filename"),
               cl::value_desc("filename"));

static cl::opt<bool>
Force("f", cl::desc("Enable binary output on terminals"));

static cl::opt<bool>
DontPrint("disable-output", cl::desc("Don't output the .ll file"), cl::Hidden);

static cl::opt<bool>
ShowAnnotations("show-annotations",
                cl::desc("Add informational comments to the .ll file"));

static cl::opt<bool> PreserveAssemblyUseListOrder(
    "preserve-ll-uselistorder",
    cl::desc("Preserve use-list order when writing LLVM assembly."),
    cl::init(false), cl::Hidden);

namespace {

static void printDebugLoc(const DebugLoc &DL, formatted_raw_ostream &OS) {
  OS << DL.getLine() << ":" << DL.getCol();
  if (DILocation *IDL = DL.getInlinedAt()) {
    OS << "@";
    printDebugLoc(IDL, OS);
  }
}
class CommentWriter : public AssemblyAnnotationWriter {
public:
  void emitFunctionAnnot(const Function *F,
                         formatted_raw_ostream &OS) override {
    OS << "; [#uses=" << F->getNumUses() << ']';  // Output # uses
    OS << '\n';
  }
  void printInfoComment(const Value &V, formatted_raw_ostream &OS) override {
    bool Padded = false;
    if (!V.getType()->isVoidTy()) {
      OS.PadToColumn(50);
      Padded = true;
      // Output # uses and type
      OS << "; [#uses=" << V.getNumUses() << " type=" << *V.getType() << "]";
    }
    if (const Instruction *I = dyn_cast<Instruction>(&V)) {
      if (const DebugLoc &DL = I->getDebugLoc()) {
        if (!Padded) {
          OS.PadToColumn(50);
          Padded = true;
          OS << ";";
        }
        OS << " [debug line = ";
        printDebugLoc(DL,OS);
        OS << "]";
      }
      if (const DbgDeclareInst *DDI = dyn_cast<DbgDeclareInst>(I)) {
        if (!Padded) {
          OS.PadToColumn(50);
          OS << ";";
        }
        OS << " [debug variable = " << DDI->getVariable()->getName() << "]";
      }
      else if (const DbgValueInst *DVI = dyn_cast<DbgValueInst>(I)) {
        if (!Padded) {
          OS.PadToColumn(50);
          OS << ";";
        }
        OS << " [debug variable = " << DVI->getVariable()->getName() << "]";
      }
    }
  }
};

} // end anon namespace

static void diagnosticHandler(const DiagnosticInfo &DI, void *Context) {
  raw_ostream &OS = errs();
  OS << (char *)Context << ": ";
  switch (DI.getSeverity()) {
  case DS_Error: OS << "error: "; break;
  case DS_Warning: OS << "warning: "; break;
  case DS_Remark: OS << "remark: "; break;
  case DS_Note: OS << "note: "; break;
  }

  DiagnosticPrinterRawOStream DP(OS);
  DI.print(DP);
  OS << '\n';

  if (DI.getSeverity() == DS_Error)
    exit(1);
}

// HLSL Change Starts
#include "dxc/Support/WinIncludes.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MSFileSystem.h"
// HLSL Change Ends

int __cdecl main(int argc, char **argv) { // HLSL Change - __cdecl
  // Print a stack trace if we signal out.
  // sys::PrintStackTraceOnErrorSignal(); // HLSL Change - disable this
  // PrettyStackTraceProgram X(argc, argv); // HLSL Change - disable this
  // HLSL Change Starts
  if (llvm::sys::fs::SetupPerThreadFileSystem())
    return 1;
  llvm::sys::fs::MSFileSystem* msfPtr;
  HRESULT hr;
  if (!SUCCEEDED(hr = CreateMSFileSystemForDisk(&msfPtr)))
    return 1;
  std::unique_ptr<llvm::sys::fs::MSFileSystem> msf(msfPtr);
  llvm::sys::fs::AutoPerThreadSystem pts(msf.get());
  // HLSL Change Ends

  LLVMContext &Context = getGlobalContext();
  llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.

  Context.setDiagnosticHandler(diagnosticHandler, argv[0]);

  cl::ParseCommandLineOptions(argc, argv, "llvm .bc -> .ll disassembler\n");

  std::string ErrorMessage;
  std::unique_ptr<Module> M;

  ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
      MemoryBuffer::getFileOrSTDIN(InputFilename);
  if (std::error_code EC = FileOrErr.getError()) {
    errs() << argv[0] << ": "
           << "Could not open file '" << InputFilename << "': " << EC.message()
           << '\n';
    return 1;
  }

  std::unique_ptr<MemoryBuffer> &Buf = FileOrErr.get();
  MemoryBufferRef BitcodeData = Buf->getMemBufferRef();
  if (Buf->getBuffer().startswith("DXBC")) {
    // move along until I get to the bitcode
    const hlsl::DxilContainerHeader *Header =
        reinterpret_cast<const hlsl::DxilContainerHeader *>(Buf->getBufferStart());
    const hlsl::DxilProgramHeader *DXILHeader =
        hlsl::GetDxilProgramHeader(Header, hlsl::DFCC_DXIL);
    if (!DXILHeader) {
      errs() << argv[0] << ": DXBC file '" << InputFilename
             << "': Does not contain DXIL part\n";
      return 1;
    }
    StringRef DXILData = StringRef(hlsl::GetDxilBitcodeData(DXILHeader),
                                   hlsl::GetDxilBitcodeSize(DXILHeader));
    BitcodeData = MemoryBufferRef(DXILData, "");
  }

  ErrorOr<std::unique_ptr<Module>> MOrErr =
      parseBitcodeFile(BitcodeData, Context);
  if (std::error_code EC = MOrErr.getError()) {
    errs() << argv[0] << ": "
           << "Could not load bitcode from file '" << InputFilename
           << "': " << EC.message() << '\n';
    return 1;
  }
  M = std::move(*MOrErr);
  M->materializeAllPermanently();

  // Just use stdout.  We won't actually print anything on it.
  if (DontPrint)
    OutputFilename = "-";

  if (OutputFilename.empty()) { // Unspecified output, infer it.
    if (InputFilename == "-") {
      OutputFilename = "-";
    } else {
      StringRef IFN = InputFilename;
      OutputFilename = (IFN.endswith(".bc") ? IFN.drop_back(3) : IFN).str();
      OutputFilename += ".ll";
    }
  }

  std::error_code EC;
  std::unique_ptr<tool_output_file> Out(
      new tool_output_file(OutputFilename, EC, sys::fs::F_None));
  if (EC) {
    errs() << EC.message() << '\n';
    return 1;
  }

  std::unique_ptr<AssemblyAnnotationWriter> Annotator;
  if (ShowAnnotations)
    Annotator.reset(new CommentWriter());

  // All that llvm-dis does is write the assembly to a file.
  if (!DontPrint)
    M->print(Out->os(), Annotator.get(), PreserveAssemblyUseListOrder);

  // Declare success.
  Out->keep();

  return 0;
}
