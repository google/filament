//===- ShowEnabledWarnings - diagtool tool for printing enabled flags -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "DiagTool.h"
#include "DiagnosticNames.h"
#include "clang/Basic/LLVM.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/Utils.h"
#include "llvm/Support/TargetSelect.h"

DEF_DIAGTOOL("show-enabled",
             "Show which warnings are enabled for a given command line",
             ShowEnabledWarnings)

using namespace clang;
using namespace diagtool;

namespace {
  struct PrettyDiag {
    StringRef Name;
    StringRef Flag;
    DiagnosticsEngine::Level Level;

    PrettyDiag(StringRef name, StringRef flag, DiagnosticsEngine::Level level)
    : Name(name), Flag(flag), Level(level) {}

    bool operator<(const PrettyDiag &x) const { return Name < x.Name; }
  };
}

static void printUsage() {
  llvm::errs() << "Usage: diagtool show-enabled [<flags>] <single-input.c>\n";
}

static char getCharForLevel(DiagnosticsEngine::Level Level) {
  switch (Level) {
  case DiagnosticsEngine::Ignored: return ' ';
  case DiagnosticsEngine::Note:    return '-';
  case DiagnosticsEngine::Remark:  return 'R';
  case DiagnosticsEngine::Warning: return 'W';
  case DiagnosticsEngine::Error:   return 'E';
  case DiagnosticsEngine::Fatal:   return 'F';
  }

  llvm_unreachable("Unknown diagnostic level");
}

static IntrusiveRefCntPtr<DiagnosticsEngine>
createDiagnostics(unsigned int argc, char **argv) {
  IntrusiveRefCntPtr<DiagnosticIDs> DiagIDs(new DiagnosticIDs());

  // Buffer diagnostics from argument parsing so that we can output them using a
  // well formed diagnostic object.
  TextDiagnosticBuffer *DiagsBuffer = new TextDiagnosticBuffer;
  IntrusiveRefCntPtr<DiagnosticsEngine> InterimDiags(
    new DiagnosticsEngine(DiagIDs, new DiagnosticOptions(), DiagsBuffer));

  // Try to build a CompilerInvocation.
  std::unique_ptr<CompilerInvocation> Invocation(
      createInvocationFromCommandLine(llvm::makeArrayRef(argv, argc),
                                      InterimDiags));
  if (!Invocation)
    return nullptr;

  // Build the diagnostics parser
  IntrusiveRefCntPtr<DiagnosticsEngine> FinalDiags =
    CompilerInstance::createDiagnostics(&Invocation->getDiagnosticOpts());
  if (!FinalDiags)
    return nullptr;

  // Flush any errors created when initializing everything. This could happen
  // for invalid command lines, which will probably give non-sensical results.
  DiagsBuffer->FlushDiagnostics(*FinalDiags);

  return FinalDiags;
}

int ShowEnabledWarnings::run(unsigned int argc, char **argv, raw_ostream &Out) {
  // First check our one flag (--levels).
  bool ShouldShowLevels = true;
  if (argc > 0) {
    StringRef FirstArg(*argv);
    if (FirstArg.equals("--no-levels")) {
      ShouldShowLevels = false;
      --argc;
      ++argv;
    } else if (FirstArg.equals("--levels")) {
      ShouldShowLevels = true;
      --argc;
      ++argv;
    }
  }

  // Create the diagnostic engine.
  IntrusiveRefCntPtr<DiagnosticsEngine> Diags = createDiagnostics(argc, argv);
  if (!Diags) {
    printUsage();
    return EXIT_FAILURE;
  }

  // Now we have our diagnostics. Iterate through EVERY diagnostic and see
  // which ones are turned on.
  // FIXME: It would be very nice to print which flags are turning on which
  // diagnostics, but this can be done with a diff.
  ArrayRef<DiagnosticRecord> AllDiagnostics = getBuiltinDiagnosticsByName();
  std::vector<PrettyDiag> Active;

  for (ArrayRef<DiagnosticRecord>::iterator I = AllDiagnostics.begin(),
                                            E = AllDiagnostics.end();
       I != E; ++I) {
    unsigned DiagID = I->DiagID;
    
    if (DiagnosticIDs::isBuiltinNote(DiagID))
      continue;
    
    if (!DiagnosticIDs::isBuiltinWarningOrExtension(DiagID))
      continue;

    DiagnosticsEngine::Level DiagLevel =
      Diags->getDiagnosticLevel(DiagID, SourceLocation());
    if (DiagLevel == DiagnosticsEngine::Ignored)
      continue;

    StringRef WarningOpt = DiagnosticIDs::getWarningOptionForDiag(DiagID);
    Active.push_back(PrettyDiag(I->getName(), WarningOpt, DiagLevel));
  }

  // Print them all out.
  for (std::vector<PrettyDiag>::const_iterator I = Active.begin(),
       E = Active.end(); I != E; ++I) {
    if (ShouldShowLevels)
      Out << getCharForLevel(I->Level) << "  ";
    Out << I->Name;
    if (!I->Flag.empty())
      Out << " [-W" << I->Flag << "]";
    Out << '\n';
  }

  return EXIT_SUCCESS;
}
