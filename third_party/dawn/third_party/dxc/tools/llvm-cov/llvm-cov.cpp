//===- llvm-cov.cpp - LLVM coverage tool ----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// llvm-cov is a command line tools to analyze and report coverage information.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/raw_ostream.h"
#include <string>

using namespace llvm;

/// \brief The main entry point for the 'show' subcommand.
int showMain(int argc, const char *argv[]);

/// \brief The main entry point for the 'report' subcommand.
int reportMain(int argc, const char *argv[]);

/// \brief The main entry point for the 'convert-for-testing' subcommand.
int convertForTestingMain(int argc, const char *argv[]);

/// \brief The main entry point for the gcov compatible coverage tool.
int gcovMain(int argc, const char *argv[]);

/// \brief Top level help.
static int helpMain(int argc, const char *argv[]) {
  errs() << "Usage: llvm-cov {gcov|report|show} [OPTION]...\n\n"
         << "Shows code coverage information.\n\n"
         << "Subcommands:\n"
         << "  gcov:   Work with the gcov format.\n"
         << "  show:   Annotate source files using instrprof style coverage.\n"
         << "  report: Summarize instrprof style coverage information.\n";
  return 0;
}

/// \brief Top level version information.
static int versionMain(int argc, const char *argv[]) {
  cl::PrintVersionMessage();
  return 0;
}

int __cdecl main(int argc, const char **argv) { // HLSL Change - __cdecl
  // If argv[0] is or ends with 'gcov', always be gcov compatible
  if (sys::path::stem(argv[0]).endswith_lower("gcov"))
    return gcovMain(argc, argv);

  // Check if we are invoking a specific tool command.
  if (argc > 1) {
    typedef int (*MainFunction)(int, const char *[]);
    MainFunction Func = StringSwitch<MainFunction>(argv[1])
                            .Case("convert-for-testing", convertForTestingMain)
                            .Case("gcov", gcovMain)
                            .Case("report", reportMain)
                            .Case("show", showMain)
                            .Cases("-h", "-help", "--help", helpMain)
                            .Cases("-version", "--version", versionMain)
                            .Default(nullptr);

    if (Func) {
      std::string Invocation = std::string(argv[0]) + " " + argv[1];
      argv[1] = Invocation.c_str();
      return Func(argc - 1, argv + 1);
    }
  }

  if (argc > 1) {
    if (sys::Process::StandardErrHasColors())
      errs().changeColor(raw_ostream::RED);
    errs() << "Unrecognized command: " << argv[1] << ".\n\n";
    if (sys::Process::StandardErrHasColors())
      errs().resetColor();
  }
  helpMain(argc, argv);
  return 1;
}
