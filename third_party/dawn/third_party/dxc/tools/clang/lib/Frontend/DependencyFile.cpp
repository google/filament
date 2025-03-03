//===--- DependencyFile.cpp - Generate dependency file --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This code generates dependency files.
//
//===----------------------------------------------------------------------===//

#include "clang/Frontend/Utils.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/DependencyOutputOptions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Lex/DirectoryLookup.h"
#include "clang/Lex/LexDiagnostic.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Serialization/ASTReader.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {
struct DepCollectorPPCallbacks : public PPCallbacks {
  DependencyCollector &DepCollector;
  SourceManager &SM;
  DepCollectorPPCallbacks(DependencyCollector &L, SourceManager &SM)
      : DepCollector(L), SM(SM) { }

  void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                   SrcMgr::CharacteristicKind FileType,
                   FileID PrevFID) override {
    if (Reason != PPCallbacks::EnterFile)
      return;

    // Dependency generation really does want to go all the way to the
    // file entry for a source location to find out what is depended on.
    // We do not want #line markers to affect dependency generation!
    const FileEntry *FE =
        SM.getFileEntryForID(SM.getFileID(SM.getExpansionLoc(Loc)));
    if (!FE)
      return;

    StringRef Filename = FE->getName();

    // Remove leading "./" (or ".//" or "././" etc.)
    while (Filename.size() > 2 && Filename[0] == '.' &&
           llvm::sys::path::is_separator(Filename[1])) {
      Filename = Filename.substr(1);
      while (llvm::sys::path::is_separator(Filename[0]))
        Filename = Filename.substr(1);
    }

    DepCollector.maybeAddDependency(Filename, /*FromModule*/false,
                                   FileType != SrcMgr::C_User,
                                   /*IsModuleFile*/false, /*IsMissing*/false);
  }

  void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                          StringRef FileName, bool IsAngled,
                          CharSourceRange FilenameRange, const FileEntry *File,
                          StringRef SearchPath, StringRef RelativePath,
                          const Module *Imported) override {
    if (!File)
      DepCollector.maybeAddDependency(FileName, /*FromModule*/false,
                                     /*IsSystem*/false, /*IsModuleFile*/false,
                                     /*IsMissing*/true);
    // Files that actually exist are handled by FileChanged.
  }

  void EndOfMainFile() override {
    DepCollector.finishedMainFile();
  }
};

#if 0 // HLSL Change Starts - no support for AST serialization
struct DepCollectorASTListener : public ASTReaderListener {
  DependencyCollector &DepCollector;
  DepCollectorASTListener(DependencyCollector &L) : DepCollector(L) { }
  bool needsInputFileVisitation() override { return true; }
  bool needsSystemInputFileVisitation() override {
    return DepCollector.needSystemDependencies();
  }
  void visitModuleFile(StringRef Filename) override {
    DepCollector.maybeAddDependency(Filename, /*FromModule*/true,
                                   /*IsSystem*/false, /*IsModuleFile*/true,
                                   /*IsMissing*/false);
  }
  bool visitInputFile(StringRef Filename, bool IsSystem,
                      bool IsOverridden) override {
    if (IsOverridden)
      return true;

    DepCollector.maybeAddDependency(Filename, /*FromModule*/true, IsSystem,
                                   /*IsModuleFile*/false, /*IsMissing*/false);
    return true;
  }
};

#endif // HLSL Change Ends - no support for AST serialization
} // end anonymous namespace

void DependencyCollector::maybeAddDependency(StringRef Filename, bool FromModule,
                                            bool IsSystem, bool IsModuleFile,
                                            bool IsMissing) {
  if (Seen.insert(Filename).second &&
      sawDependency(Filename, FromModule, IsSystem, IsModuleFile, IsMissing))
    Dependencies.push_back(Filename);
}

static bool isSpecialFilename(StringRef Filename) {
  return llvm::StringSwitch<bool>(Filename)
      .Case("<built-in>", true)
      .Case("<stdin>", true)
      .Default(false);
}

bool DependencyCollector::sawDependency(StringRef Filename, bool FromModule,
                                       bool IsSystem, bool IsModuleFile,
                                       bool IsMissing) {
  return !isSpecialFilename(Filename) &&
         (needSystemDependencies() || !IsSystem);
}

DependencyCollector::~DependencyCollector() { }
void DependencyCollector::attachToPreprocessor(Preprocessor &PP) {
  PP.addPPCallbacks(
      llvm::make_unique<DepCollectorPPCallbacks>(*this, PP.getSourceManager()));
}
#if 0 // HLSL Change Starts - no support for AST serialization
void DependencyCollector::attachToASTReader(ASTReader &R) {
  R.addListener(llvm::make_unique<DepCollectorASTListener>(*this));
}
#endif // HLSL Change Ends - no support for AST serialization

namespace {
/// Private implementation for DependencyFileGenerator
class DFGImpl : public PPCallbacks {
  std::vector<std::string> Files;
  llvm::StringSet<> FilesSet;
  const Preprocessor *PP;
  std::string OutputFile;
  std::vector<std::string> Targets;
  bool IncludeSystemHeaders;
  bool PhonyTarget;
  bool AddMissingHeaderDeps;
  bool SeenMissingHeader;
  bool IncludeModuleFiles;
  DependencyOutputFormat OutputFormat;

private:
  bool FileMatchesDepCriteria(const char *Filename,
                              SrcMgr::CharacteristicKind FileType);
  void OutputDependencyFile();

public:
  DFGImpl(const Preprocessor *_PP, const DependencyOutputOptions &Opts)
    : PP(_PP), OutputFile(Opts.OutputFile), Targets(Opts.Targets),
      IncludeSystemHeaders(Opts.IncludeSystemHeaders),
      PhonyTarget(Opts.UsePhonyTargets),
      AddMissingHeaderDeps(Opts.AddMissingHeaderDeps),
      SeenMissingHeader(false),
      IncludeModuleFiles(Opts.IncludeModuleFiles),
      OutputFormat(Opts.OutputFormat) {}

  void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                   SrcMgr::CharacteristicKind FileType,
                   FileID PrevFID) override;
  void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                          StringRef FileName, bool IsAngled,
                          CharSourceRange FilenameRange, const FileEntry *File,
                          StringRef SearchPath, StringRef RelativePath,
                          const Module *Imported) override;

  void EndOfMainFile() override {
    OutputDependencyFile();
  }

  void AddFilename(StringRef Filename);
  bool includeSystemHeaders() const { return IncludeSystemHeaders; }
  bool includeModuleFiles() const { return IncludeModuleFiles; }
};

#if 0 // HLSL Change Starts - no support for AST serialization
class DFGASTReaderListener : public ASTReaderListener {
  DFGImpl &Parent;
public:
  DFGASTReaderListener(DFGImpl &Parent)
  : Parent(Parent) { }
  bool needsInputFileVisitation() override { return true; }
  bool needsSystemInputFileVisitation() override {
    return Parent.includeSystemHeaders();
  }
  void visitModuleFile(StringRef Filename) override;
  bool visitInputFile(StringRef Filename, bool isSystem,
                      bool isOverridden) override;
};
#endif // HLSL Change Ends - no support for AST serialization
}

DependencyFileGenerator::DependencyFileGenerator(void *Impl)
: Impl(Impl) { }

DependencyFileGenerator *DependencyFileGenerator::CreateAndAttachToPreprocessor(
    clang::Preprocessor &PP, const clang::DependencyOutputOptions &Opts) {

  if (Opts.Targets.empty()) {
    PP.getDiagnostics().Report(diag::err_fe_dependency_file_requires_MT);
    return nullptr;
  }

  // Disable the "file not found" diagnostic if the -MG option was given.
  if (Opts.AddMissingHeaderDeps)
    PP.SetSuppressIncludeNotFoundError(true);

  DFGImpl *Callback = new DFGImpl(&PP, Opts);
  PP.addPPCallbacks(std::unique_ptr<PPCallbacks>(Callback));
  return new DependencyFileGenerator(Callback);
}

#if 0 // HLSL Change Starts - no support for AST serialization
void DependencyFileGenerator::AttachToASTReader(ASTReader &R) {
  DFGImpl *I = reinterpret_cast<DFGImpl *>(Impl);
  assert(I && "missing implementation");
  R.addListener(llvm::make_unique<DFGASTReaderListener>(*I));
}
#endif // HLSL Change Starts - no support for AST serialization

/// FileMatchesDepCriteria - Determine whether the given Filename should be
/// considered as a dependency.
bool DFGImpl::FileMatchesDepCriteria(const char *Filename,
                                     SrcMgr::CharacteristicKind FileType) {
  if (isSpecialFilename(Filename))
    return false;

  if (IncludeSystemHeaders)
    return true;

  return FileType == SrcMgr::C_User;
}

void DFGImpl::FileChanged(SourceLocation Loc,
                          FileChangeReason Reason,
                          SrcMgr::CharacteristicKind FileType,
                          FileID PrevFID) {
  if (Reason != PPCallbacks::EnterFile)
    return;

  // Dependency generation really does want to go all the way to the
  // file entry for a source location to find out what is depended on.
  // We do not want #line markers to affect dependency generation!
  SourceManager &SM = PP->getSourceManager();

  const FileEntry *FE =
    SM.getFileEntryForID(SM.getFileID(SM.getExpansionLoc(Loc)));
  if (!FE) return;

  StringRef Filename = FE->getName();
  if (!FileMatchesDepCriteria(Filename.data(), FileType))
    return;

  // Remove leading "./" (or ".//" or "././" etc.)
  while (Filename.size() > 2 && Filename[0] == '.' &&
         llvm::sys::path::is_separator(Filename[1])) {
    Filename = Filename.substr(1);
    while (llvm::sys::path::is_separator(Filename[0]))
      Filename = Filename.substr(1);
  }
    
  AddFilename(Filename);
}

void DFGImpl::InclusionDirective(SourceLocation HashLoc,
                                 const Token &IncludeTok,
                                 StringRef FileName,
                                 bool IsAngled,
                                 CharSourceRange FilenameRange,
                                 const FileEntry *File,
                                 StringRef SearchPath,
                                 StringRef RelativePath,
                                 const Module *Imported) {
  if (!File) {
    if (AddMissingHeaderDeps)
      AddFilename(FileName);
    else
      SeenMissingHeader = true;
  }
}

void DFGImpl::AddFilename(StringRef Filename) {
  if (FilesSet.insert(Filename).second)
    Files.push_back(Filename);
}

/// Print the filename, with escaping or quoting that accommodates the three
/// most likely tools that use dependency files: GNU Make, BSD Make, and
/// NMake/Jom.
///
/// BSD Make is the simplest case: It does no escaping at all.  This means
/// characters that are normally delimiters, i.e. space and # (the comment
/// character) simply aren't supported in filenames.
///
/// GNU Make does allow space and # in filenames, but to avoid being treated
/// as a delimiter or comment, these must be escaped with a backslash. Because
/// backslash is itself the escape character, if a backslash appears in a
/// filename, it should be escaped as well.  (As a special case, $ is escaped
/// as $$, which is the normal Make way to handle the $ character.)
/// For compatibility with BSD Make and historical practice, if GNU Make
/// un-escapes characters in a filename but doesn't find a match, it will
/// retry with the unmodified original string.
///
/// GCC tries to accommodate both Make formats by escaping any space or #
/// characters in the original filename, but not escaping backslashes.  The
/// apparent intent is so that filenames with backslashes will be handled
/// correctly by BSD Make, and by GNU Make in its fallback mode of using the
/// unmodified original string; filenames with # or space characters aren't
/// supported by BSD Make at all, but will be handled correctly by GNU Make
/// due to the escaping.
///
/// A corner case that GCC gets only partly right is when the original filename
/// has a backslash immediately followed by space or #.  GNU Make would expect
/// this backslash to be escaped; however GCC escapes the original backslash
/// only when followed by space, not #.  It will therefore take a dependency
/// from a directive such as
///     #include "a\ b\#c.h"
/// and emit it as
///     a\\\ b\\#c.h
/// which GNU Make will interpret as
///     a\ b\
/// followed by a comment. Failing to find this file, it will fall back to the
/// original string, which probably doesn't exist either; in any case it won't
/// find
///     a\ b\#c.h
/// which is the actual filename specified by the include directive.
///
/// Clang does what GCC does, rather than what GNU Make expects.
///
/// NMake/Jom has a different set of scary characters, but wraps filespecs in
/// double-quotes to avoid misinterpreting them; see
/// https://msdn.microsoft.com/en-us/library/dd9y37ha.aspx for NMake info,
/// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx
/// for Windows file-naming info.
static void PrintFilename(raw_ostream &OS, StringRef Filename,
                          DependencyOutputFormat OutputFormat) {
  if (OutputFormat == DependencyOutputFormat::NMake) {
    // Add quotes if needed. These are the characters listed as "special" to
    // NMake, that are legal in a Windows filespec, and that could cause
    // misinterpretation of the dependency string.
    if (Filename.find_first_of(" #${}^!") != StringRef::npos)
      OS << '\"' << Filename << '\"';
    else
      OS << Filename;
    return;
  }
  assert(OutputFormat == DependencyOutputFormat::Make);
  for (unsigned i = 0, e = Filename.size(); i != e; ++i) {
    if (Filename[i] == '#') // Handle '#' the broken gcc way.
      OS << '\\';
    else if (Filename[i] == ' ') { // Handle space correctly.
      OS << '\\';
      unsigned j = i;
      while (j > 0 && Filename[--j] == '\\')
        OS << '\\';
    } else if (Filename[i] == '$') // $ is escaped by $$.
      OS << '$';
    OS << Filename[i];
  }
}

void DFGImpl::OutputDependencyFile() {
  if (SeenMissingHeader) {
    llvm::sys::fs::remove(OutputFile);
    return;
  }

  std::error_code EC;
  llvm::raw_fd_ostream OS(OutputFile, EC, llvm::sys::fs::F_Text);
  if (EC) {
    PP->getDiagnostics().Report(diag::err_fe_error_opening) << OutputFile
                                                            << EC.message();
    return;
  }

  // Write out the dependency targets, trying to avoid overly long
  // lines when possible. We try our best to emit exactly the same
  // dependency file as GCC (4.2), assuming the included files are the
  // same.
  const unsigned MaxColumns = 75;
  unsigned Columns = 0;

  for (std::vector<std::string>::iterator
         I = Targets.begin(), E = Targets.end(); I != E; ++I) {
    unsigned N = I->length();
    if (Columns == 0) {
      Columns += N;
    } else if (Columns + N + 2 > MaxColumns) {
      Columns = N + 2;
      OS << " \\\n  ";
    } else {
      Columns += N + 1;
      OS << ' ';
    }
    // Targets already quoted as needed.
    OS << *I;
  }

  OS << ':';
  Columns += 1;

  // Now add each dependency in the order it was seen, but avoiding
  // duplicates.
  for (std::vector<std::string>::iterator I = Files.begin(),
         E = Files.end(); I != E; ++I) {
    // Start a new line if this would exceed the column limit. Make
    // sure to leave space for a trailing " \" in case we need to
    // break the line on the next iteration.
    unsigned N = I->length();
    if (Columns + (N + 1) + 2 > MaxColumns) {
      OS << " \\\n ";
      Columns = 2;
    }
    OS << ' ';
    PrintFilename(OS, *I, OutputFormat);
    Columns += N + 1;
  }
  OS << '\n';

  // Create phony targets if requested.
  if (PhonyTarget && !Files.empty()) {
    // Skip the first entry, this is always the input file itself.
    for (std::vector<std::string>::iterator I = Files.begin() + 1,
           E = Files.end(); I != E; ++I) {
      OS << '\n';
      PrintFilename(OS, *I, OutputFormat);
      OS << ":\n";
    }
  }
}

#if 0 // HLSL Change Starts - no support for AST serialization
bool DFGASTReaderListener::visitInputFile(llvm::StringRef Filename,
                                          bool IsSystem, bool IsOverridden) {
  assert(!IsSystem || needsSystemInputFileVisitation());
  if (IsOverridden)
    return true;

  Parent.AddFilename(Filename);
  return true;
}

void DFGASTReaderListener::visitModuleFile(llvm::StringRef Filename) {
  if (Parent.includeModuleFiles())
    Parent.AddFilename(Filename);
}
#endif // HLSL Change Ends - no support for AST serialization
