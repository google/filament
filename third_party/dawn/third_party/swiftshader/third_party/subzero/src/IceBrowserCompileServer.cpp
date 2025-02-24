//===- subzero/src/IceBrowserCompileServer.cpp - Browser compile server ---===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the browser-based compile server.
///
//===----------------------------------------------------------------------===//

// Can only compile this with the NaCl compiler (needs irt.h, and the
// unsandboxed LLVM build using the trusted compiler does not have irt.h).
#include "IceBrowserCompileServer.h"
#include "IceRangeSpec.h"

#if PNACL_BROWSER_TRANSLATOR

// Headers which are not properly part of the SDK are included by their path in
// the NaCl tree.
#ifdef __pnacl__
#include "native_client/src/untrusted/nacl/pnacl.h"
#endif // __pnacl__

#include "llvm/Support/QueueStreamer.h"

#include <cstring>
#include <fstream>
#include <irt.h>
#include <irt_dev.h>
#include <pthread.h>
#include <thread>

namespace Ice {

// Create C wrappers around callback handlers for the IRT interface.
namespace {

BrowserCompileServer *gCompileServer;
struct nacl_irt_private_pnacl_translator_compile gIRTFuncs;

void getIRTInterfaces() {
  size_t QueryResult =
      nacl_interface_query(NACL_IRT_PRIVATE_PNACL_TRANSLATOR_COMPILE_v0_1,
                           &gIRTFuncs, sizeof(gIRTFuncs));
  if (QueryResult != sizeof(gIRTFuncs))
    llvm::report_fatal_error("Failed to get translator compile IRT interface");
}

// Allow pnacl-sz arguments to be supplied externally, instead of coming from
// the browser.  This is meant to be used for debugging.
//
// NOTE: This functionality is only enabled in non-MINIMAL Subzero builds, for
// security/safety reasons.
//
// If the SZARGFILE environment variable is set to a file name, arguments are
// read from that file, one argument per line.  This requires setting 3
// environment variables before starting the browser:
//
// NACL_ENV_PASSTHROUGH=NACL_DANGEROUS_ENABLE_FILE_ACCESS,NACLENV_SZARGFILE
// NACL_DANGEROUS_ENABLE_FILE_ACCESS=1
// NACLENV_SZARGFILE=/path/to/myargs.txt
//
// In addition, Chrome needs to be launched with the "--no-sandbox" argument.
//
// If the SZARGLIST environment variable is set, arguments are extracted from
// that variable's value, separated by the '|' character (being careful to
// escape/quote special shell characters).  This requires setting 2 environment
// variables before starting the browser:
//
// NACL_ENV_PASSTHROUGH=NACLENV_SZARGLIST
// NACLENV_SZARGLIST=arg
//
// This does not require the "--no-sandbox" argument, and is therefore much
// safer, but does require restarting the browser to change the arguments.
//
// If external arguments are supplied, the browser's NumThreads specification is
// ignored, to allow -threads to be specified as an external argument.  Note
// that the browser normally supplies the "-O2" argument, so externally supplied
// arguments might want to provide an explicit -O argument.
//
// See Chrome's src/components/nacl/zygote/nacl_fork_delegate_linux.cc for the
// NACL_ENV_PASSTHROUGH mechanism.
//
// See NaCl's src/trusted/service_runtime/env_cleanser.c for the NACLENV_
// mechanism.
std::vector<std::string> getExternalArgs() {
  std::vector<std::string> ExternalArgs;
  if (BuildDefs::minimal())
    return ExternalArgs;
  char ArgsFileVar[] = "SZARGFILE";
  char ArgsListVar[] = "SZARGLIST";
  if (const char *ArgsFilename = getenv(ArgsFileVar)) {
    std::ifstream ArgsStream(ArgsFilename);
    std::string Arg;
    while (ArgsStream >> std::ws, std::getline(ArgsStream, Arg)) {
      if (!Arg.empty() && Arg[0] == '#')
        continue;
      ExternalArgs.emplace_back(Arg);
    }
    if (ExternalArgs.empty()) {
      llvm::report_fatal_error("Failed to read arguments from file '" +
                               std::string(ArgsFilename) + "'");
    }
  } else if (const char *ArgsList = getenv(ArgsListVar)) {
    // Leverage the RangeSpec tokenizer.
    auto Args = RangeSpec::tokenize(ArgsList, '|');
    ExternalArgs.insert(ExternalArgs.end(), Args.begin(), Args.end());
  }
  return ExternalArgs;
}

char *onInitCallback(uint32_t NumThreads, int *ObjFileFDs,
                     size_t ObjFileFDCount, char **CLArgs, size_t CLArgsLen) {
  if (ObjFileFDCount < 1) {
    std::string Buffer;
    llvm::raw_string_ostream StrBuf(Buffer);
    StrBuf << "Invalid number of FDs for onInitCallback " << ObjFileFDCount
           << "\n";
    return strdup(StrBuf.str().c_str());
  }
  int ObjFileFD = ObjFileFDs[0];
  if (ObjFileFD < 0) {
    std::string Buffer;
    llvm::raw_string_ostream StrBuf(Buffer);
    StrBuf << "Invalid FD given for onInitCallback " << ObjFileFD << "\n";
    return strdup(StrBuf.str().c_str());
  }
  // CLArgs is almost an "argv", but is missing the argv[0] program name.
  std::vector<const char *> Argv;
  constexpr static char ProgramName[] = "pnacl-sz.nexe";
  Argv.reserve(CLArgsLen + 1);
  Argv.push_back(ProgramName);

  bool UseNumThreadsFromBrowser = true;
  auto ExternalArgs = getExternalArgs();
  if (ExternalArgs.empty()) {
    for (size_t i = 0; i < CLArgsLen; ++i) {
      Argv.push_back(CLArgs[i]);
    }
  } else {
    for (auto &Arg : ExternalArgs) {
      Argv.emplace_back(Arg.c_str());
    }
    UseNumThreadsFromBrowser = false;
  }
  // NOTE: strings pointed to by argv are owned by the caller, but we parse
  // here before returning and don't store them.
  gCompileServer->getParsedFlags(UseNumThreadsFromBrowser, NumThreads,
                                 Argv.size(), Argv.data());
  gCompileServer->startCompileThread(ObjFileFD);
  return nullptr;
}

int onDataCallback(const void *Data, size_t NumBytes) {
  return gCompileServer->pushInputBytes(Data, NumBytes) ? 1 : 0;
}

char *onEndCallback() {
  gCompileServer->endInputStream();
  gCompileServer->waitForCompileThread();
  // TODO(jvoung): Also return UMA data.
  if (gCompileServer->getErrorCode().value()) {
    const std::string Error = gCompileServer->getErrorStream().getContents();
    return strdup(Error.empty() ? "Some error occurred" : Error.c_str());
  }
  return nullptr;
}

struct nacl_irt_pnacl_compile_funcs SubzeroCallbacks {
  &onInitCallback, &onDataCallback, &onEndCallback
};

std::unique_ptr<llvm::raw_fd_ostream> getOutputStream(int FD) {
  if (FD <= 0)
    llvm::report_fatal_error("Invalid output FD");
  constexpr bool CloseOnDtor = true;
  constexpr bool Unbuffered = false;
  return std::unique_ptr<llvm::raw_fd_ostream>(
      new llvm::raw_fd_ostream(FD, CloseOnDtor, Unbuffered));
}

void fatalErrorHandler(void *UserData, const std::string &Reason,
                       bool GenCrashDialog) {
  (void)GenCrashDialog;
  BrowserCompileServer *Server =
      reinterpret_cast<BrowserCompileServer *>(UserData);
  Server->setFatalError(Reason);
  // Only kill the current thread instead of the whole process. We need the
  // server thread to remain alive in order to respond with the error message.
  // We could also try to pthread_kill all other worker threads, but
  // pthread_kill / raising signals is not supported by NaCl. We'll have to
  // assume that the worker/emitter threads will be well behaved after a fatal
  // error in other threads, and either get stuck waiting on input from a
  // previous stage, or also call report_fatal_error.
  pthread_exit(0);
}

/// Adapted from pnacl-llc's AddDefaultCPU() in srpc_main.cpp.
TargetArch getTargetArch() {
#if defined(__pnacl__)
  switch (__builtin_nacl_target_arch()) {
  case PnaclTargetArchitectureX86_32:
  case PnaclTargetArchitectureX86_32_NonSFI:
    return Target_X8632;
  case PnaclTargetArchitectureX86_64:
    return Target_X8664;
  case PnaclTargetArchitectureARM_32:
  case PnaclTargetArchitectureARM_32_NonSFI:
    return Target_ARM32;
  case PnaclTargetArchitectureMips_32:
    return Target_MIPS32;
  default:
    llvm::report_fatal_error("no target architecture match.");
  }
#elif defined(__i386__)
  return Target_X8632;
#elif defined(__x86_64__)
  return Target_X8664;
#elif defined(__arm__)
  return Target_ARM32;
#else
// TODO(stichnot): Add mips.
#error "Unknown architecture"
#endif
}

} // end of anonymous namespace

BrowserCompileServer::~BrowserCompileServer() = default;

void BrowserCompileServer::run() {
  gCompileServer = this;
  getIRTInterfaces();
  gIRTFuncs.serve_translate_request(&SubzeroCallbacks);
}

void BrowserCompileServer::getParsedFlags(bool UseNumThreadsFromBrowser,
                                          uint32_t NumThreads, int argc,
                                          const char *const *argv) {
  ClFlags::parseFlags(argc, argv);
  ClFlags::getParsedClFlags(ClFlags::Flags);
  // Set some defaults which aren't specified via the argv string.
  if (UseNumThreadsFromBrowser)
    ClFlags::Flags.setNumTranslationThreads(NumThreads);
  ClFlags::Flags.setUseSandboxing(true);
  ClFlags::Flags.setOutFileType(FT_Elf);
  ClFlags::Flags.setTargetArch(getTargetArch());
  ClFlags::Flags.setInputFileFormat(llvm::PNaClFormat);
}

bool BrowserCompileServer::pushInputBytes(const void *Data, size_t NumBytes) {
  // If there was an earlier error, do not attempt to push bytes to the
  // QueueStreamer. Otherwise the thread could become blocked.
  if (HadError.load())
    return true;
  return InputStream->PutBytes(
             const_cast<unsigned char *>(
                 reinterpret_cast<const unsigned char *>(Data)),
             NumBytes) != NumBytes;
}

void BrowserCompileServer::setFatalError(const std::string &Reason) {
  HadError.store(true);
  Ctx->getStrError() << Reason;
  // Make sure that the QueueStreamer is not stuck by signaling an early end.
  InputStream->SetDone();
}

ErrorCode &BrowserCompileServer::getErrorCode() {
  if (HadError.load()) {
    // HadError means report_fatal_error is called. Make sure that the
    // LastError is not EC_None. We don't know the type of error so just pick
    // some error category.
    LastError.assign(EC_Translation);
  }
  return LastError;
}

void BrowserCompileServer::endInputStream() { InputStream->SetDone(); }

void BrowserCompileServer::startCompileThread(int ObjFD) {
  InputStream = new llvm::QueueStreamer();
  bool LogStreamFailure = false;
  int LogFD = STDOUT_FILENO;
  if (getFlags().getLogFilename() == "-") {
    // Common case, do nothing.
  } else if (getFlags().getLogFilename() == "/dev/stderr") {
    LogFD = STDERR_FILENO;
  } else {
    LogStreamFailure = true;
  }
  LogStream = getOutputStream(LogFD);
  LogStream->SetUnbuffered();
  if (LogStreamFailure) {
    *LogStream
        << "Warning: Log file name must be either '-' or '/dev/stderr'\n";
  }
  EmitStream = getOutputStream(ObjFD);
  EmitStream->SetBufferSize(1 << 14);
  std::unique_ptr<StringStream> ErrStrm(new StringStream());
  ErrorStream = std::move(ErrStrm);
  ELFStream.reset(new ELFFileStreamer(*EmitStream.get()));
  Ctx.reset(new GlobalContext(LogStream.get(), EmitStream.get(),
                              &ErrorStream->getStream(), ELFStream.get()));
  CompileThread = std::thread([this]() {
    llvm::install_fatal_error_handler(fatalErrorHandler, this);
    Ctx->initParserThread();
    this->getCompiler().run(ClFlags::Flags, *Ctx.get(),
                            // Retain original reference, but the compiler
                            // (LLVM's MemoryObject) wants to handle deletion.
                            std::unique_ptr<llvm::DataStreamer>(InputStream));
  });
}

} // end of namespace Ice

#else // !PNACL_BROWSER_TRANSLATOR

#include "llvm/Support/ErrorHandling.h"

namespace Ice {

BrowserCompileServer::~BrowserCompileServer() {}

void BrowserCompileServer::run() {
  llvm::report_fatal_error("no browser hookups");
}

ErrorCode &BrowserCompileServer::getErrorCode() {
  llvm::report_fatal_error("no browser hookups");
}

} // end of namespace Ice

#endif // PNACL_BROWSER_TRANSLATOR
