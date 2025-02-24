//===- subzero/src/IceCompiler.cpp - Driver for bitcode translation -------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines a driver for translating PNaCl bitcode into native code.
///
/// The driver can either directly parse the binary bitcode file, or use LLVM
/// routines to parse a textual bitcode file into LLVM IR and then convert LLVM
/// IR into ICE. In either case, the high-level ICE is then compiled down to
/// native code, as either an ELF object file or a textual asm file.
///
//===----------------------------------------------------------------------===//

#include "IceCompiler.h"

#include "IceBuildDefs.h"
#include "IceCfg.h"
#include "IceClFlags.h"
#include "IceConverter.h"
#include "IceELFObjectWriter.h"
#include "PNaClTranslator.h"
#include "WasmTranslator.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif // __clang__

#include "llvm/ADT/STLExtras.h"
#include "llvm/Bitcode/NaCl/NaClReaderWriter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/StreamingMemoryObject.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__

#include <regex>

namespace Ice {

namespace {

bool llvmIRInput(const std::string &Filename) {
  return BuildDefs::llvmIrAsInput() &&
         std::regex_match(Filename, std::regex(".*\\.ll"));
}

bool wasmInput(const std::string &Filename) {
  return BuildDefs::wasm() &&
         std::regex_match(Filename, std::regex(".*\\.wasm"));
}

} // end of anonymous namespace

void Compiler::run(const Ice::ClFlags &Flags, GlobalContext &Ctx,
                   std::unique_ptr<llvm::DataStreamer> &&InputStream) {
  // The Minimal build (specifically, when dump()/emit() are not implemented)
  // allows only --filetype=obj. Check here to avoid cryptic error messages
  // downstream.
  if (!BuildDefs::dump() && getFlags().getOutFileType() != FT_Elf) {
    Ctx.getStrError()
        << "Error: only --filetype=obj is supported in this build.\n";
    Ctx.getErrorStatus()->assign(EC_Args);
    return;
  }

  TimerMarker T(Ice::TimerStack::TT_szmain, &Ctx);

  Ctx.emitFileHeader();
  Ctx.startWorkerThreads();

  std::unique_ptr<Translator> Translator;
  const std::string IRFilename = Flags.getIRFilename();
  const bool BuildOnRead = Flags.getBuildOnRead() && !llvmIRInput(IRFilename) &&
                           !wasmInput(IRFilename);
  const bool WasmBuildOnRead = Flags.getBuildOnRead() && wasmInput(IRFilename);
  if (BuildOnRead) {
    std::unique_ptr<PNaClTranslator> PTranslator(new PNaClTranslator(&Ctx));
#ifdef PNACL_LLVM
    std::unique_ptr<llvm::StreamingMemoryObject> MemObj(
        new llvm::StreamingMemoryObjectImpl(InputStream.release()));
#else  // !PNACL_LLVM
    std::unique_ptr<llvm::StreamingMemoryObject> MemObj(
        new llvm::StreamingMemoryObject(std::move(InputStream)));
#endif // !PNACL_LLVM
    PTranslator->translate(IRFilename, std::move(MemObj));
    Translator.reset(PTranslator.release());
  } else if (WasmBuildOnRead) {
    if (BuildDefs::wasm()) {
#if !ALLOW_WASM
      assert(false && "wasm not allowed");
#else
      std::unique_ptr<WasmTranslator> WTranslator(new WasmTranslator(&Ctx));

      WTranslator->translate(IRFilename, std::move(InputStream));

      Translator.reset(WTranslator.release());
#endif // !ALLOW_WASM
    } else {
      Ctx.getStrError() << "WASM support not enabled\n";
      Ctx.getErrorStatus()->assign(EC_Args);
      return;
    }
  } else if (BuildDefs::llvmIr()) {
    if (BuildDefs::browser()) {
      Ctx.getStrError()
          << "non BuildOnRead is not supported w/ PNACL_BROWSER_TRANSLATOR\n";
      Ctx.getErrorStatus()->assign(EC_Args);
      Ctx.waitForWorkerThreads();
      return;
    }
    // Globals must be kept alive after lowering when converting from LLVM to
    // Ice.
    Ctx.setDisposeGlobalVariablesAfterLowering(false);
    // Parse the input LLVM IR file into a module.
    llvm::SMDiagnostic Err;
    TimerMarker T1(Ice::TimerStack::TT_parse, &Ctx);
#ifdef PNACL_LLVM
    llvm::DiagnosticHandlerFunction DiagnosticHandler =
        Flags.getLLVMVerboseErrors()
            ? redirectNaClDiagnosticToStream(llvm::errs())
            : nullptr;
    std::unique_ptr<llvm::Module> Mod =
        NaClParseIRFile(IRFilename, Flags.getInputFileFormat(), Err,
                        llvm::getGlobalContext(), DiagnosticHandler);
#else  // !PNACL_LLVM
    llvm::DiagnosticHandlerFunction DiagnosticHandler = nullptr;
    llvm::LLVMContext Context;
    std::unique_ptr<llvm::Module> Mod = parseIRFile(IRFilename, Err, Context);
#endif // !PNACL_LLVM
    if (!Mod) {
      Err.print(Flags.getAppName().c_str(), llvm::errs());
      Ctx.getErrorStatus()->assign(EC_Bitcode);
      Ctx.waitForWorkerThreads();
      return;
    }

    std::unique_ptr<Converter> Converter(new class Converter(Mod.get(), &Ctx));
    Converter->convertToIce();
    Translator.reset(Converter.release());
  } else {
    Ctx.getStrError() << "Error: Build doesn't allow LLVM IR, "
                      << "--build-on-read=0 not allowed\n";
    Ctx.getErrorStatus()->assign(EC_Args);
    Ctx.waitForWorkerThreads();
    return;
  }

  Ctx.waitForWorkerThreads();
  if (Translator->getErrorStatus()) {
    Ctx.getErrorStatus()->assign(Translator->getErrorStatus().value());
  } else {
    Ctx.lowerGlobals("last");
    Ctx.lowerProfileData();
    Ctx.lowerConstants();
    Ctx.lowerJumpTables();

    if (getFlags().getOutFileType() == FT_Elf) {
      TimerMarker T1(Ice::TimerStack::TT_emitAsm, &Ctx);
      Ctx.getObjectWriter()->setUndefinedSyms(Ctx.getConstantExternSyms());
      Ctx.emitTargetRODataSections();
      Ctx.getObjectWriter()->writeNonUserSections();
    }
  }

  if (getFlags().getSubzeroTimingEnabled())
    Ctx.dumpTimers();

  if (getFlags().getTimeEachFunction()) {
    constexpr bool NoDumpCumulative = false;
    Ctx.dumpTimers(GlobalContext::TSK_Funcs, NoDumpCumulative);
  }
  Ctx.dumpStats();
}

} // end of namespace Ice
