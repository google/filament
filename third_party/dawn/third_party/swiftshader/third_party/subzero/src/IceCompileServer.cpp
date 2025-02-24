//===- subzero/src/IceCompileServer.cpp - Compile server ------------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the basic commandline-based compile server.
///
//===----------------------------------------------------------------------===//

#include "IceCompileServer.h"

#include "IceASanInstrumentation.h"
#include "IceClFlags.h"
#include "IceELFStreamer.h"
#include "IceGlobalContext.h"
#include "IceRevision.h"
#include "LinuxMallocProfiling.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif // __clang__

#ifdef PNACL_LLVM
#include "llvm/Bitcode/NaCl/NaClBitcodeMungeUtils.h"
#endif // PNACL_LLVM
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/StreamingMemoryObject.h"
#include "llvm/Support/raw_os_ostream.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__

#include <cstdio>
#include <fstream>
#include <iostream>
#include <thread>

namespace Ice {

namespace {

// Define a SmallVector backed buffer as a data stream, so that it can hold the
// generated binary version of the textual bitcode in the input file.
class TextDataStreamer : public llvm::DataStreamer {
public:
  TextDataStreamer() = default;
  ~TextDataStreamer() final = default;
#ifdef PNACL_LLVM
  using CreateType = TextDataStreamer *;
#else  // !PNACL_LLVM
  using CreateType = std::unique_ptr<TextDataStreamer>;
#endif // !PNACL_LLVM
  static CreateType create(const std::string &Filename, std::string *Err);
  size_t GetBytes(unsigned char *Buf, size_t Len) final;

private:
  llvm::SmallVector<char, 1024> BitcodeBuffer;
  size_t Cursor = 0;
};

TextDataStreamer::CreateType
TextDataStreamer::create(const std::string &Filename, std::string *Err) {
#ifdef PNACL_LLVM
  TextDataStreamer *Streamer = new TextDataStreamer();
  llvm::raw_string_ostream ErrStrm(*Err);
  if (std::error_code EC = llvm::readNaClRecordTextAndBuildBitcode(
          Filename, Streamer->BitcodeBuffer, &ErrStrm)) {
    ErrStrm << EC.message();
    ErrStrm.flush();
    delete Streamer;
    return nullptr;
  }
  ErrStrm.flush();
  return Streamer;
#else  // !PNACL_LLVM
  return CreateType();
#endif // !PNACL_LLVM
}

size_t TextDataStreamer::GetBytes(unsigned char *Buf, size_t Len) {
  if (Cursor >= BitcodeBuffer.size())
    return 0;
  size_t Remaining = BitcodeBuffer.size();
  Len = std::min(Len, Remaining);
  for (size_t i = 0; i < Len; ++i)
    Buf[i] = BitcodeBuffer[Cursor + i];
  Cursor += Len;
  return Len;
}

std::unique_ptr<Ostream> makeStream(const std::string &Filename,
                                    std::error_code &EC) {
  if (Filename == "-") {
    return std::unique_ptr<Ostream>(new llvm::raw_os_ostream(std::cout));
  } else if (Filename == "/dev/stderr") {
    return std::unique_ptr<Ostream>(new llvm::raw_os_ostream(std::cerr));
  } else {
    return std::unique_ptr<Ostream>(
        new llvm::raw_fd_ostream(Filename, EC, llvm::sys::fs::F_None));
  }
}

ErrorCodes getReturnValue(ErrorCodes Val) {
  if (getFlags().getAlwaysExitSuccess())
    return EC_None;
  return Val;
}

// Reports fatal error message, and then exits with success status 0.
void reportFatalErrorThenExitSuccess(void *UserData, const std::string &Reason,
                                     bool GenCrashDag) {
  (void)UserData;
  (void)GenCrashDag;

  // Note: This code is (mostly) copied from llvm/lib/Support/ErrorHandling.cpp

  // Blast the result out to stderr.  We don't try hard to make sure this
  // succeeds (e.g. handling EINTR) and we can't use errs() here because
  // raw ostreams can call report_fatal_error.
  llvm::SmallVector<char, 64> Buffer;
  llvm::raw_svector_ostream OS(Buffer);
  OS << "LLVM ERROR: " << Reason << "\n";
  llvm::StringRef MessageStr = OS.str();
  ssize_t Written =
      std::fwrite(MessageStr.data(), sizeof(char), MessageStr.size(), stderr);
  (void)Written; // If something went wrong, we deliberately just give up.

  // If we reached here, we are failing ungracefully. Run the interrupt handlers
  // to make sure any special cleanups get done, in particular that we remove
  // files registered with RemoveFileOnSignal.
  llvm::sys::RunInterruptHandlers();

  exit(0);
}

struct {
  const char *FlagName;
  bool FlagValue;
} ConditionalBuildAttributes[] = {
    {"dump", BuildDefs::dump()},
    {"llvm_cl", BuildDefs::llvmCl()},
    {"llvm_ir", BuildDefs::llvmIr()},
    {"llvm_ir_as_input", BuildDefs::llvmIrAsInput()},
    {"minimal_build", BuildDefs::minimal()},
    {"browser_mode", BuildDefs::browser()}};

/// Dumps values of build attributes to Stream if Stream is non-null.
void dumpBuildAttributes(Ostream &Str) {
// List the supported targets.
#define SUBZERO_TARGET(TARGET) Str << "target_" XSTRINGIFY(TARGET) "\n";
#include "SZTargets.def"
  const char *Prefix[2] = {"no", "allow"};
  for (size_t i = 0; i < llvm::array_lengthof(ConditionalBuildAttributes);
       ++i) {
    const auto &A = ConditionalBuildAttributes[i];
    Str << Prefix[A.FlagValue] << "_" << A.FlagName << "\n";
  }
  Str << "revision_" << getSubzeroRevision() << "\n";
}

} // end of anonymous namespace

void CLCompileServer::run() {
  if (BuildDefs::dump()) {
#ifdef PNACL_LLVM
    llvm::sys::PrintStackTraceOnErrorSignal();
#else  // !PNACL_LLVM
    llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);
#endif // !PNACL_LLVM
  }
  ClFlags::parseFlags(argc, argv);
  ClFlags &Flags = ClFlags::Flags;
  ClFlags::getParsedClFlags(Flags);

  // Override report_fatal_error if we want to exit with 0 status.
  if (Flags.getAlwaysExitSuccess())
    llvm::install_fatal_error_handler(reportFatalErrorThenExitSuccess, this);

  std::error_code EC;
  std::unique_ptr<Ostream> Ls = makeStream(Flags.getLogFilename(), EC);
  if (EC) {
    llvm::report_fatal_error("Unable to open log file");
  }
  Ls->SetUnbuffered();
  Ice::LinuxMallocProfiling _(Flags.getNumTranslationThreads(), Ls.get());

  std::unique_ptr<Ostream> Os;
  std::unique_ptr<ELFStreamer> ELFStr;
  switch (Flags.getOutFileType()) {
  case FT_Elf: {
    if (Flags.getOutputFilename() == "-" && !Flags.getGenerateBuildAtts()) {
      *Ls << "Error: writing binary ELF to stdout is unsupported\n";
      return transferErrorCode(getReturnValue(Ice::EC_Args));
    }
    std::unique_ptr<llvm::raw_fd_ostream> FdOs(new llvm::raw_fd_ostream(
        Flags.getOutputFilename(), EC, llvm::sys::fs::F_None));
    if (EC) {
      *Ls << "Failed to open output file: " << Flags.getOutputFilename()
          << ":\n"
          << EC.message() << "\n";
      return transferErrorCode(getReturnValue(Ice::EC_Args));
    }
    ELFStr.reset(new ELFFileStreamer(*FdOs.get()));
    Os.reset(FdOs.release());
    // NaCl sets st_blksize to 0, and LLVM uses that to pick the default
    // preferred buffer size. Set to something non-zero.
    Os->SetBufferSize(1 << 14);
  } break;
  case FT_Asm:
  case FT_Iasm: {
    Os = makeStream(Flags.getOutputFilename(), EC);
    if (EC) {
      *Ls << "Failed to open output file: " << Flags.getOutputFilename()
          << ":\n"
          << EC.message() << "\n";
      return transferErrorCode(getReturnValue(Ice::EC_Args));
    }
    Os->SetUnbuffered();
  } break;
  }

  if (BuildDefs::minimal() && Flags.getBitcodeAsText())
    llvm::report_fatal_error("Can't specify 'bitcode-as-text' flag in "
                             "minimal build");

  std::string StrError;
  std::unique_ptr<llvm::DataStreamer> InputStream(
      (!BuildDefs::minimal() && Flags.getBitcodeAsText())
          ? TextDataStreamer::create(Flags.getIRFilename(), &StrError)
          : llvm::getDataFileStreamer(Flags.getIRFilename(), &StrError));
  if (!StrError.empty() || !InputStream) {
    llvm::SMDiagnostic Err(Flags.getIRFilename(), llvm::SourceMgr::DK_Error,
                           StrError);
    Err.print(Flags.getAppName().c_str(), *Ls);
    return transferErrorCode(getReturnValue(Ice::EC_Bitcode));
  }

  if (Flags.getGenerateBuildAtts()) {
    dumpBuildAttributes(*Os.get());
    return transferErrorCode(getReturnValue(Ice::EC_None));
  }

  Ctx.reset(new GlobalContext(Ls.get(), Os.get(), Ls.get(), ELFStr.get()));

  if (!BuildDefs::minimal() && getFlags().getSanitizeAddresses()) {
    std::unique_ptr<Instrumentation> Instr(new ASanInstrumentation(Ctx.get()));
    Ctx->setInstrumentation(std::move(Instr));
  }

  if (getFlags().getNumTranslationThreads() != 0) {
    std::thread CompileThread([this, &InputStream]() {
      Ctx->initParserThread();
      getCompiler().run(Flags, *Ctx.get(), std::move(InputStream));
    });
    CompileThread.join();
  } else {
    getCompiler().run(Flags, *Ctx.get(), std::move(InputStream));
  }
  transferErrorCode(
      getReturnValue(static_cast<ErrorCodes>(Ctx->getErrorStatus()->value())));
  Ctx->dumpConstantLookupCounts();
  Ctx->dumpStrings();
}

} // end of namespace Ice
