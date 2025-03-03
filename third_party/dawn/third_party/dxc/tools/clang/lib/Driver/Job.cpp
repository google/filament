//===--- Job.cpp - Command to Execute -------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Job.h"
#include "clang/Driver/Tool.h"
#include "clang/Driver/ToolChain.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
using namespace clang::driver;
using llvm::raw_ostream;
using llvm::StringRef;
using llvm::ArrayRef;

Command::Command(const Action &Source, const Tool &Creator,
                 const char *Executable, const ArgStringList &Arguments)
    : Source(Source), Creator(Creator), Executable(Executable),
      Arguments(Arguments), ResponseFile(nullptr) {}

static int skipArgs(const char *Flag, bool HaveCrashVFS) {
  // These flags are all of the form -Flag <Arg> and are treated as two
  // arguments.  Therefore, we need to skip the flag and the next argument.
  bool Res = llvm::StringSwitch<bool>(Flag)
    .Cases("-I", "-MF", "-MT", "-MQ", true)
    .Cases("-o", "-coverage-file", "-dependency-file", true)
    .Cases("-fdebug-compilation-dir", "-idirafter", true)
    .Cases("-include", "-include-pch", "-internal-isystem", true)
    .Cases("-internal-externc-isystem", "-iprefix", "-iwithprefix", true)
    .Cases("-iwithprefixbefore", "-isystem", "-iquote", true)
    .Cases("-resource-dir", "-serialize-diagnostic-file", true)
    .Cases("-dwarf-debug-flags", "-ivfsoverlay", true)
    // Some include flags shouldn't be skipped if we have a crash VFS
    .Case("-isysroot", !HaveCrashVFS)
    .Default(false);

  // Match found.
  if (Res)
    return 2;

  // The remaining flags are treated as a single argument.

  // These flags are all of the form -Flag and have no second argument.
  Res = llvm::StringSwitch<bool>(Flag)
    .Cases("-M", "-MM", "-MG", "-MP", "-MD", true)
    .Case("-MMD", true)
    .Default(false);

  // Match found.
  if (Res)
    return 1;

  // These flags are treated as a single argument (e.g., -F<Dir>).
  StringRef FlagRef(Flag);
  if (FlagRef.startswith("-F") || FlagRef.startswith("-I") ||
      FlagRef.startswith("-fmodules-cache-path="))
    return 1;

  return 0;
}

void Command::printArg(raw_ostream &OS, const char *Arg, bool Quote) {
  const bool Escape = std::strpbrk(Arg, "\"\\$");

  if (!Quote && !Escape) {
    OS << Arg;
    return;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  while (const char c = *Arg++) {
    if (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

void Command::writeResponseFile(raw_ostream &OS) const {
  // In a file list, we only write the set of inputs to the response file
  if (Creator.getResponseFilesSupport() == Tool::RF_FileList) {
    for (const char *Arg : InputFileList) {
      OS << Arg << '\n';
    }
    return;
  }

  // In regular response files, we send all arguments to the response file
  for (const char *Arg : Arguments) {
    OS << '"';

    for (; *Arg != '\0'; Arg++) {
      if (*Arg == '\"' || *Arg == '\\') {
        OS << '\\';
      }
      OS << *Arg;
    }

    OS << "\" ";
  }
}

void Command::buildArgvForResponseFile(
    llvm::SmallVectorImpl<const char *> &Out) const {
  // When not a file list, all arguments are sent to the response file.
  // This leaves us to set the argv to a single parameter, requesting the tool
  // to read the response file.
  if (Creator.getResponseFilesSupport() != Tool::RF_FileList) {
    Out.push_back(Executable);
    Out.push_back(ResponseFileFlag.c_str());
    return;
  }

  llvm::StringSet<> Inputs;
  for (const char *InputName : InputFileList)
    Inputs.insert(InputName);
  Out.push_back(Executable);
  // In a file list, build args vector ignoring parameters that will go in the
  // response file (elements of the InputFileList vector)
  bool FirstInput = true;
  for (const char *Arg : Arguments) {
    if (Inputs.count(Arg) == 0) {
      Out.push_back(Arg);
    } else if (FirstInput) {
      FirstInput = false;
      Out.push_back(Creator.getResponseFileFlag());
      Out.push_back(ResponseFile);
    }
  }
}

void Command::Print(raw_ostream &OS, const char *Terminator, bool Quote,
                    CrashReportInfo *CrashInfo) const {
  // Always quote the exe.
  OS << ' ';
  printArg(OS, Executable, /*Quote=*/true);

  llvm::ArrayRef<const char *> Args = Arguments;
  llvm::SmallVector<const char *, 128> ArgsRespFile;
  if (ResponseFile != nullptr) {
    buildArgvForResponseFile(ArgsRespFile);
    Args = ArrayRef<const char *>(ArgsRespFile).slice(1); // no executable name
  }

  StringRef MainFilename;
  // We'll need the argument to -main-file-name to find the input file name.
  if (CrashInfo)
    for (size_t I = 0, E = Args.size(); I + 1 < E; ++I)
      if (StringRef(Args[I]).equals("-main-file-name"))
        MainFilename = Args[I + 1];

  bool HaveCrashVFS = CrashInfo && !CrashInfo->VFSPath.empty();
  for (size_t i = 0, e = Args.size(); i < e; ++i) {
    const char *const Arg = Args[i];

    if (CrashInfo) {
      if (int Skip = skipArgs(Arg, HaveCrashVFS)) {
        i += Skip - 1;
        continue;
      } else if (llvm::sys::path::filename(Arg) == MainFilename &&
                 (i == 0 || StringRef(Args[i - 1]) != "-main-file-name")) {
        // Replace the input file name with the crashinfo's file name.
        OS << ' ';
        StringRef ShortName = llvm::sys::path::filename(CrashInfo->Filename);
        printArg(OS, ShortName.str().c_str(), Quote);
        continue;
      }
    }

    OS << ' ';
    printArg(OS, Arg, Quote);
  }

  if (CrashInfo && HaveCrashVFS) {
    OS << ' ';
    printArg(OS, "-ivfsoverlay", Quote);
    OS << ' ';
    printArg(OS, CrashInfo->VFSPath.str().c_str(), Quote);
  }

  if (ResponseFile != nullptr) {
    OS << "\n Arguments passed via response file:\n";
    writeResponseFile(OS);
    // Avoiding duplicated newline terminator, since FileLists are
    // newline-separated.
    if (Creator.getResponseFilesSupport() != Tool::RF_FileList)
      OS << "\n";
    OS << " (end of response file)";
  }

  OS << Terminator;
}

void Command::setResponseFile(const char *FileName) {
  ResponseFile = FileName;
  ResponseFileFlag = Creator.getResponseFileFlag();
  ResponseFileFlag += FileName;
}

int Command::Execute(const StringRef **Redirects, std::string *ErrMsg,
                     bool *ExecutionFailed) const {
#ifdef MSFT_SUPPORTS_CHILD_PROCESSES
  SmallVector<const char*, 128> Argv;

  if (ResponseFile == nullptr) {
    Argv.push_back(Executable);
    Argv.append(Arguments.begin(), Arguments.end());
    Argv.push_back(nullptr);

    return llvm::sys::ExecuteAndWait(Executable, Argv.data(), /*env*/ nullptr,
                                     Redirects, /*secondsToWait*/ 0,
                                     /*memoryLimit*/ 0, ErrMsg,
                                     ExecutionFailed);
  }

  // We need to put arguments in a response file (command is too large)
  // Open stream to store the response file contents
  std::string RespContents;
  llvm::raw_string_ostream SS(RespContents);

  // Write file contents and build the Argv vector
  writeResponseFile(SS);
  buildArgvForResponseFile(Argv);
  Argv.push_back(nullptr);
  SS.flush();

  // Save the response file in the appropriate encoding
  if (std::error_code EC = writeFileWithEncoding(
          ResponseFile, RespContents, Creator.getResponseFileEncoding())) {
    if (ErrMsg)
      *ErrMsg = EC.message();
    if (ExecutionFailed)
      *ExecutionFailed = true;
    return -1;
  }

  return llvm::sys::ExecuteAndWait(Executable, Argv.data(), /*env*/ nullptr,
                                   Redirects, /*secondsToWait*/ 0,
                                   /*memoryLimit*/ 0, ErrMsg, ExecutionFailed);
#else
  assert(false && "support for spawning processes should be removed from public paths");
  return 1;
#endif
}

FallbackCommand::FallbackCommand(const Action &Source_, const Tool &Creator_,
                                 const char *Executable_,
                                 const ArgStringList &Arguments_,
                                 std::unique_ptr<Command> Fallback_)
    : Command(Source_, Creator_, Executable_, Arguments_),
      Fallback(std::move(Fallback_)) {}

void FallbackCommand::Print(raw_ostream &OS, const char *Terminator,
                            bool Quote, CrashReportInfo *CrashInfo) const {
  Command::Print(OS, "", Quote, CrashInfo);
  OS << " ||";
  Fallback->Print(OS, Terminator, Quote, CrashInfo);
}

static bool ShouldFallback(int ExitCode) {
  // FIXME: We really just want to fall back for internal errors, such
  // as when some symbol cannot be mangled, when we should be able to
  // parse something but can't, etc.
  return ExitCode != 0;
}

int FallbackCommand::Execute(const StringRef **Redirects, std::string *ErrMsg,
                             bool *ExecutionFailed) const {
  int PrimaryStatus = Command::Execute(Redirects, ErrMsg, ExecutionFailed);
  if (!ShouldFallback(PrimaryStatus))
    return PrimaryStatus;

  // Clear ExecutionFailed and ErrMsg before falling back.
  if (ErrMsg)
    ErrMsg->clear();
  if (ExecutionFailed)
    *ExecutionFailed = false;

  const Driver &D = getCreator().getToolChain().getDriver();
  D.Diag(diag::warn_drv_invoking_fallback) << Fallback->getExecutable();

  int SecondaryStatus = Fallback->Execute(Redirects, ErrMsg, ExecutionFailed);
  return SecondaryStatus;
}

void JobList::Print(raw_ostream &OS, const char *Terminator, bool Quote,
                    CrashReportInfo *CrashInfo) const {
  for (const auto &Job : *this)
    Job.Print(OS, Terminator, Quote, CrashInfo);
}

void JobList::clear() { Jobs.clear(); }
