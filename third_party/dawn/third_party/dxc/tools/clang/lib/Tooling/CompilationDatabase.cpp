//===--- CompilationDatabase.cpp - ----------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file contains implementations of the CompilationDatabase base class
//  and the FixedCompilationDatabase.
//
//===----------------------------------------------------------------------===//

#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Driver/Action.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Job.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Tooling/CompilationDatabasePluginRegistry.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Option/Arg.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/Path.h"
#include <sstream>
#include <system_error>
using namespace clang;
using namespace tooling;

CompilationDatabase::~CompilationDatabase() {}

std::unique_ptr<CompilationDatabase>
CompilationDatabase::loadFromDirectory(StringRef BuildDirectory,
                                       std::string &ErrorMessage) {
  std::stringstream ErrorStream;
  for (CompilationDatabasePluginRegistry::iterator
       It = CompilationDatabasePluginRegistry::begin(),
       Ie = CompilationDatabasePluginRegistry::end();
       It != Ie; ++It) {
    std::string DatabaseErrorMessage;
    std::unique_ptr<CompilationDatabasePlugin> Plugin(It->instantiate());
    if (std::unique_ptr<CompilationDatabase> DB =
            Plugin->loadFromDirectory(BuildDirectory, DatabaseErrorMessage))
      return DB;
    ErrorStream << It->getName() << ": " << DatabaseErrorMessage << "\n";
  }
  ErrorMessage = ErrorStream.str();
  return nullptr;
}

static std::unique_ptr<CompilationDatabase>
findCompilationDatabaseFromDirectory(StringRef Directory,
                                     std::string &ErrorMessage) {
  std::stringstream ErrorStream;
  bool HasErrorMessage = false;
  while (!Directory.empty()) {
    std::string LoadErrorMessage;

    if (std::unique_ptr<CompilationDatabase> DB =
            CompilationDatabase::loadFromDirectory(Directory, LoadErrorMessage))
      return DB;

    if (!HasErrorMessage) {
      ErrorStream << "No compilation database found in " << Directory.str()
                  << " or any parent directory\n" << LoadErrorMessage;
      HasErrorMessage = true;
    }

    Directory = llvm::sys::path::parent_path(Directory);
  }
  ErrorMessage = ErrorStream.str();
  return nullptr;
}

std::unique_ptr<CompilationDatabase>
CompilationDatabase::autoDetectFromSource(StringRef SourceFile,
                                          std::string &ErrorMessage) {
  SmallString<1024> AbsolutePath(getAbsolutePath(SourceFile));
  StringRef Directory = llvm::sys::path::parent_path(AbsolutePath);

  std::unique_ptr<CompilationDatabase> DB =
      findCompilationDatabaseFromDirectory(Directory, ErrorMessage);

  if (!DB)
    ErrorMessage = ("Could not auto-detect compilation database for file \"" +
                   SourceFile + "\"\n" + ErrorMessage).str();
  return DB;
}

std::unique_ptr<CompilationDatabase>
CompilationDatabase::autoDetectFromDirectory(StringRef SourceDir,
                                             std::string &ErrorMessage) {
  SmallString<1024> AbsolutePath(getAbsolutePath(SourceDir));

  std::unique_ptr<CompilationDatabase> DB =
      findCompilationDatabaseFromDirectory(AbsolutePath, ErrorMessage);

  if (!DB)
    ErrorMessage = ("Could not auto-detect compilation database from directory \"" +
                   SourceDir + "\"\n" + ErrorMessage).str();
  return DB;
}

CompilationDatabasePlugin::~CompilationDatabasePlugin() {}

namespace {
// Helper for recursively searching through a chain of actions and collecting
// all inputs, direct and indirect, of compile jobs.
struct CompileJobAnalyzer {
  void run(const driver::Action *A) {
    runImpl(A, false);
  }

  SmallVector<std::string, 2> Inputs;

private:

  void runImpl(const driver::Action *A, bool Collect) {
    bool CollectChildren = Collect;
    switch (A->getKind()) {
    case driver::Action::CompileJobClass:
      CollectChildren = true;
      break;

    case driver::Action::InputClass: {
      if (Collect) {
        const driver::InputAction *IA = cast<driver::InputAction>(A);
        Inputs.push_back(IA->getInputArg().getSpelling());
      }
    } break;

    default:
      // Don't care about others
      ;
    }

    for (driver::ActionList::const_iterator I = A->begin(), E = A->end();
         I != E; ++I)
      runImpl(*I, CollectChildren);
  }
};

// Special DiagnosticConsumer that looks for warn_drv_input_file_unused
// diagnostics from the driver and collects the option strings for those unused
// options.
class UnusedInputDiagConsumer : public DiagnosticConsumer {
public:
  UnusedInputDiagConsumer() : Other(nullptr) {}

  // Useful for debugging, chain diagnostics to another consumer after
  // recording for our own purposes.
  UnusedInputDiagConsumer(DiagnosticConsumer *Other) : Other(Other) {}

  void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                        const Diagnostic &Info) override {
    if (Info.getID() == clang::diag::warn_drv_input_file_unused) {
      // Arg 1 for this diagnostic is the option that didn't get used.
      UnusedInputs.push_back(Info.getArgStdStr(0));
    }
    if (Other)
      Other->HandleDiagnostic(DiagLevel, Info);
  }

  DiagnosticConsumer *Other;
  SmallVector<std::string, 2> UnusedInputs;
};

// Unary functor for asking "Given a StringRef S1, does there exist a string
// S2 in Arr where S1 == S2?"
struct MatchesAny {
  MatchesAny(ArrayRef<std::string> Arr) : Arr(Arr) {}
  bool operator() (StringRef S) {
    for (const std::string *I = Arr.begin(), *E = Arr.end(); I != E; ++I)
      if (*I == S)
        return true;
    return false;
  }
private:
  ArrayRef<std::string> Arr;
};
} // namespace

/// \brief Strips any positional args and possible argv[0] from a command-line
/// provided by the user to construct a FixedCompilationDatabase.
///
/// FixedCompilationDatabase requires a command line to be in this format as it
/// constructs the command line for each file by appending the name of the file
/// to be compiled. FixedCompilationDatabase also adds its own argv[0] to the
/// start of the command line although its value is not important as it's just
/// ignored by the Driver invoked by the ClangTool using the
/// FixedCompilationDatabase.
///
/// FIXME: This functionality should probably be made available by
/// clang::driver::Driver although what the interface should look like is not
/// clear.
///
/// \param[in] Args Args as provided by the user.
/// \return Resulting stripped command line.
///          \li true if successful.
///          \li false if \c Args cannot be used for compilation jobs (e.g.
///          contains an option like -E or -version).
static bool stripPositionalArgs(std::vector<const char *> Args,
                                std::vector<std::string> &Result) {
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
  UnusedInputDiagConsumer DiagClient;
  DiagnosticsEngine Diagnostics(
      IntrusiveRefCntPtr<clang::DiagnosticIDs>(new DiagnosticIDs()),
      &*DiagOpts, &DiagClient, false);

  // The clang executable path isn't required since the jobs the driver builds
  // will not be executed.
  std::unique_ptr<driver::Driver> NewDriver(new driver::Driver(
      /* ClangExecutable= */ "", llvm::sys::getDefaultTargetTriple(),
      Diagnostics));
  NewDriver->setCheckInputsExist(false);

  // This becomes the new argv[0]. The value is actually not important as it
  // isn't used for invoking Tools.
  Args.insert(Args.begin(), "clang-tool");

  // By adding -c, we force the driver to treat compilation as the last phase.
  // It will then issue warnings via Diagnostics about un-used options that
  // would have been used for linking. If the user provided a compiler name as
  // the original argv[0], this will be treated as a linker input thanks to
  // insertng a new argv[0] above. All un-used options get collected by
  // UnusedInputdiagConsumer and get stripped out later.
  Args.push_back("-c");

  // Put a dummy C++ file on to ensure there's at least one compile job for the
  // driver to construct. If the user specified some other argument that
  // prevents compilation, e.g. -E or something like -version, we may still end
  // up with no jobs but then this is the user's fault.
  Args.push_back("placeholder.cpp");

  // Remove -no-integrated-as; it's not used for syntax checking,
  // and it confuses targets which don't support this option.
  Args.erase(std::remove_if(Args.begin(), Args.end(),
                            MatchesAny(std::string("-no-integrated-as"))),
             Args.end());

  const std::unique_ptr<driver::Compilation> Compilation(
      NewDriver->BuildCompilation(Args));

  const driver::JobList &Jobs = Compilation->getJobs();

  CompileJobAnalyzer CompileAnalyzer;

  for (const auto &Cmd : Jobs) {
    // Collect only for Assemble jobs. If we do all jobs we get duplicates
    // since Link jobs point to Assemble jobs as inputs.
    if (Cmd.getSource().getKind() == driver::Action::AssembleJobClass)
      CompileAnalyzer.run(&Cmd.getSource());
  }

  if (CompileAnalyzer.Inputs.empty()) {
    // No compile jobs found.
    // FIXME: Emit a warning of some kind?
    return false;
  }

  // Remove all compilation input files from the command line. This is
  // necessary so that getCompileCommands() can construct a command line for
  // each file.
  std::vector<const char *>::iterator End = std::remove_if(
      Args.begin(), Args.end(), MatchesAny(CompileAnalyzer.Inputs));

  // Remove all inputs deemed unused for compilation.
  End = std::remove_if(Args.begin(), End, MatchesAny(DiagClient.UnusedInputs));

  // Remove the -c add above as well. It will be at the end right now.
  assert(strcmp(*(End - 1), "-c") == 0);
  --End;

  Result = std::vector<std::string>(Args.begin() + 1, End);
  return true;
}

FixedCompilationDatabase *FixedCompilationDatabase::loadFromCommandLine(
    int &Argc, const char *const *Argv, Twine Directory) {
  const char *const *DoubleDash = std::find(Argv, Argv + Argc, StringRef("--"));
  if (DoubleDash == Argv + Argc)
    return nullptr;
  std::vector<const char *> CommandLine(DoubleDash + 1, Argv + Argc);
  Argc = DoubleDash - Argv;

  std::vector<std::string> StrippedArgs;
  if (!stripPositionalArgs(CommandLine, StrippedArgs))
    return nullptr;
  return new FixedCompilationDatabase(Directory, StrippedArgs);
}

FixedCompilationDatabase::
FixedCompilationDatabase(Twine Directory, ArrayRef<std::string> CommandLine) {
  std::vector<std::string> ToolCommandLine(1, "clang-tool");
  ToolCommandLine.insert(ToolCommandLine.end(),
                         CommandLine.begin(), CommandLine.end());
  CompileCommands.emplace_back(Directory, std::move(ToolCommandLine));
}

std::vector<CompileCommand>
FixedCompilationDatabase::getCompileCommands(StringRef FilePath) const {
  std::vector<CompileCommand> Result(CompileCommands);
  Result[0].CommandLine.push_back(FilePath);
  return Result;
}

std::vector<std::string>
FixedCompilationDatabase::getAllFiles() const {
  return std::vector<std::string>();
}

std::vector<CompileCommand>
FixedCompilationDatabase::getAllCompileCommands() const {
  return std::vector<CompileCommand>();
}

namespace clang {
namespace tooling {

// This anchor is used to force the linker to link in the generated object file
// and thus register the JSONCompilationDatabasePlugin.
extern volatile int JSONAnchorSource;
static int JSONAnchorDest = JSONAnchorSource;

} // end namespace tooling
} // end namespace clang
