//===--- PPCallbacks.h - Callbacks for Preprocessor actions -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the PPCallbacks interface.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LEX_PPCALLBACKS_H
#define LLVM_CLANG_LEX_PPCALLBACKS_H

#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/DirectoryLookup.h"
#include "clang/Lex/ModuleLoader.h"
#include "clang/Lex/Pragma.h"
#include "llvm/ADT/StringRef.h"
#include <string>

namespace clang {
  class SourceLocation;
  class Token;
  class IdentifierInfo;
  class MacroDefinition;
  class MacroDirective;
  class MacroArgs;

/// \brief This interface provides a way to observe the actions of the
/// preprocessor as it does its thing.
///
/// Clients can define their hooks here to implement preprocessor level tools.
class PPCallbacks {
public:
  virtual ~PPCallbacks();

  enum FileChangeReason {
    EnterFile, ExitFile, SystemHeaderPragma, RenameFile
  };

  /// \brief Callback invoked whenever a source file is entered or exited.
  ///
  /// \param Loc Indicates the new location.
  /// \param PrevFID the file that was exited if \p Reason is ExitFile.
  virtual void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                           SrcMgr::CharacteristicKind FileType,
                           FileID PrevFID = FileID()) {
  }

  /// \brief Callback invoked whenever a source file is skipped as the result
  /// of header guard optimization.
  ///
  /// \param SkippedFile The file that is skipped instead of entering \#include
  ///
  /// \param FilenameTok The file name token in \#include "FileName" directive
  /// or macro expanded file name token from \#include MACRO(PARAMS) directive.
  /// Note that FilenameTok contains corresponding quotes/angles symbols.
  virtual void FileSkipped(const FileEntry &SkippedFile,
                           const Token &FilenameTok,
                           SrcMgr::CharacteristicKind FileType) {
  }

  /// \brief Callback invoked whenever an inclusion directive results in a
  /// file-not-found error.
  ///
  /// \param FileName The name of the file being included, as written in the 
  /// source code.
  ///
  /// \param RecoveryPath If this client indicates that it can recover from 
  /// this missing file, the client should set this as an additional header
  /// search patch.
  ///
  /// \returns true to indicate that the preprocessor should attempt to recover
  /// by adding \p RecoveryPath as a header search path.
  virtual bool FileNotFound(StringRef FileName,
                            SmallVectorImpl<char> &RecoveryPath) {
    return false;
  }

  /// \brief Callback invoked whenever an inclusion directive of
  /// any kind (\c \#include, \c \#import, etc.) has been processed, regardless
  /// of whether the inclusion will actually result in an inclusion.
  ///
  /// \param HashLoc The location of the '#' that starts the inclusion 
  /// directive.
  ///
  /// \param IncludeTok The token that indicates the kind of inclusion 
  /// directive, e.g., 'include' or 'import'.
  ///
  /// \param FileName The name of the file being included, as written in the 
  /// source code.
  ///
  /// \param IsAngled Whether the file name was enclosed in angle brackets;
  /// otherwise, it was enclosed in quotes.
  ///
  /// \param FilenameRange The character range of the quotes or angle brackets
  /// for the written file name.
  ///
  /// \param File The actual file that may be included by this inclusion 
  /// directive.
  ///
  /// \param SearchPath Contains the search path which was used to find the file
  /// in the file system. If the file was found via an absolute include path,
  /// SearchPath will be empty. For framework includes, the SearchPath and
  /// RelativePath will be split up. For example, if an include of "Some/Some.h"
  /// is found via the framework path
  /// "path/to/Frameworks/Some.framework/Headers/Some.h", SearchPath will be
  /// "path/to/Frameworks/Some.framework/Headers" and RelativePath will be
  /// "Some.h".
  ///
  /// \param RelativePath The path relative to SearchPath, at which the include
  /// file was found. This is equal to FileName except for framework includes.
  ///
  /// \param Imported The module, whenever an inclusion directive was
  /// automatically turned into a module import or null otherwise.
  ///
  virtual void InclusionDirective(SourceLocation HashLoc,
                                  const Token &IncludeTok,
                                  StringRef FileName,
                                  bool IsAngled,
                                  CharSourceRange FilenameRange,
                                  const FileEntry *File,
                                  StringRef SearchPath,
                                  StringRef RelativePath,
                                  const Module *Imported) {
  }

  /// \brief Callback invoked whenever there was an explicit module-import
  /// syntax.
  ///
  /// \param ImportLoc The location of import directive token.
  ///
  /// \param Path The identifiers (and their locations) of the module
  /// "path", e.g., "std.vector" would be split into "std" and "vector".
  ///
  /// \param Imported The imported module; can be null if importing failed.
  ///
  virtual void moduleImport(SourceLocation ImportLoc,
                            ModuleIdPath Path,
                            const Module *Imported) {
  }

  /// \brief Callback invoked when the end of the main file is reached.
  ///
  /// No subsequent callbacks will be made.
  virtual void EndOfMainFile() {
  }

  /// \brief Callback invoked when a \#ident or \#sccs directive is read.
  /// \param Loc The location of the directive.
  /// \param str The text of the directive.
  ///
  virtual void Ident(SourceLocation Loc, StringRef str) {
  }

  /// \brief Callback invoked when start reading any pragma directive.
  virtual void PragmaDirective(SourceLocation Loc,
                               PragmaIntroducerKind Introducer) {
  }

  /// \brief Callback invoked when a \#pragma comment directive is read.
  virtual void PragmaComment(SourceLocation Loc, const IdentifierInfo *Kind,
                             StringRef Str) {
  }

  /// \brief Callback invoked when a \#pragma detect_mismatch directive is
  /// read.
  virtual void PragmaDetectMismatch(SourceLocation Loc, StringRef Name,
                                    StringRef Value) {
  }

  /// \brief Callback invoked when a \#pragma clang __debug directive is read.
  /// \param Loc The location of the debug directive.
  /// \param DebugType The identifier following __debug.
  virtual void PragmaDebug(SourceLocation Loc, StringRef DebugType) {
  }

  /// \brief Determines the kind of \#pragma invoking a call to PragmaMessage.
  enum PragmaMessageKind {
    /// \brief \#pragma message has been invoked.
    PMK_Message,

    /// \brief \#pragma GCC warning has been invoked.
    PMK_Warning,

    /// \brief \#pragma GCC error has been invoked.
    PMK_Error
  };

  /// \brief Callback invoked when a \#pragma message directive is read.
  /// \param Loc The location of the message directive.
  /// \param Namespace The namespace of the message directive.
  /// \param Kind The type of the message directive.
  /// \param Str The text of the message directive.
  virtual void PragmaMessage(SourceLocation Loc, StringRef Namespace,
                             PragmaMessageKind Kind, StringRef Str) {
  }

  /// \brief Callback invoked when a \#pragma gcc dianostic push directive
  /// is read.
  virtual void PragmaDiagnosticPush(SourceLocation Loc,
                                    StringRef Namespace) {
  }

  /// \brief Callback invoked when a \#pragma gcc dianostic pop directive
  /// is read.
  virtual void PragmaDiagnosticPop(SourceLocation Loc,
                                   StringRef Namespace) {
  }

  /// \brief Callback invoked when a \#pragma gcc dianostic directive is read.
  virtual void PragmaDiagnostic(SourceLocation Loc, StringRef Namespace,
                                diag::Severity mapping, StringRef Str) {}

  /// \brief Called when an OpenCL extension is either disabled or
  /// enabled with a pragma.
  virtual void PragmaOpenCLExtension(SourceLocation NameLoc, 
                                     const IdentifierInfo *Name,
                                     SourceLocation StateLoc, unsigned State) {
  }

  /// \brief Callback invoked when a \#pragma warning directive is read.
  virtual void PragmaWarning(SourceLocation Loc, StringRef WarningSpec,
                             ArrayRef<int> Ids) {
  }

  /// \brief Callback invoked when a \#pragma warning(push) directive is read.
  virtual void PragmaWarningPush(SourceLocation Loc, int Level) {
  }

  /// \brief Callback invoked when a \#pragma warning(pop) directive is read.
  virtual void PragmaWarningPop(SourceLocation Loc) {
  }

  /// \brief Called by Preprocessor::HandleMacroExpandedIdentifier when a
  /// macro invocation is found.
  virtual void MacroExpands(const Token &MacroNameTok,
                            const MacroDefinition &MD, SourceRange Range,
                            const MacroArgs *Args) {}

  /// \brief Hook called whenever a macro definition is seen.
  virtual void MacroDefined(const Token &MacroNameTok,
                            const MacroDirective *MD) {
  }

  /// \brief Hook called whenever a macro \#undef is seen.
  ///
  /// MD is released immediately following this callback.
  virtual void MacroUndefined(const Token &MacroNameTok,
                              const MacroDefinition &MD) {
  }
  
  /// \brief Hook called whenever the 'defined' operator is seen.
  /// \param MD The MacroDirective if the name was a macro, null otherwise.
  virtual void Defined(const Token &MacroNameTok, const MacroDefinition &MD,
                       SourceRange Range) {
  }
  
  /// \brief Hook called when a source range is skipped.
  /// \param Range The SourceRange that was skipped. The range begins at the
  /// \#if/\#else directive and ends after the \#endif/\#else directive.
  virtual void SourceRangeSkipped(SourceRange Range) {
  }

  enum ConditionValueKind {
    CVK_NotEvaluated, CVK_False, CVK_True
  };

  /// \brief Hook called whenever an \#if is seen.
  /// \param Loc the source location of the directive.
  /// \param ConditionRange The SourceRange of the expression being tested.
  /// \param ConditionValue The evaluated value of the condition.
  ///
  // FIXME: better to pass in a list (or tree!) of Tokens.
  virtual void If(SourceLocation Loc, SourceRange ConditionRange,
                  ConditionValueKind ConditionValue) {
  }

  /// \brief Hook called whenever an \#elif is seen.
  /// \param Loc the source location of the directive.
  /// \param ConditionRange The SourceRange of the expression being tested.
  /// \param ConditionValue The evaluated value of the condition.
  /// \param IfLoc the source location of the \#if/\#ifdef/\#ifndef directive.
  // FIXME: better to pass in a list (or tree!) of Tokens.
  virtual void Elif(SourceLocation Loc, SourceRange ConditionRange,
                    ConditionValueKind ConditionValue, SourceLocation IfLoc) {
  }

  /// \brief Hook called whenever an \#ifdef is seen.
  /// \param Loc the source location of the directive.
  /// \param MacroNameTok Information on the token being tested.
  /// \param MD The MacroDefinition if the name was a macro, null otherwise.
  virtual void Ifdef(SourceLocation Loc, const Token &MacroNameTok,
                     const MacroDefinition &MD) {
  }

  /// \brief Hook called whenever an \#ifndef is seen.
  /// \param Loc the source location of the directive.
  /// \param MacroNameTok Information on the token being tested.
  /// \param MD The MacroDefiniton if the name was a macro, null otherwise.
  virtual void Ifndef(SourceLocation Loc, const Token &MacroNameTok,
                      const MacroDefinition &MD) {
  }

  /// \brief Hook called whenever an \#else is seen.
  /// \param Loc the source location of the directive.
  /// \param IfLoc the source location of the \#if/\#ifdef/\#ifndef directive.
  virtual void Else(SourceLocation Loc, SourceLocation IfLoc) {
  }

  /// \brief Hook called whenever an \#endif is seen.
  /// \param Loc the source location of the directive.
  /// \param IfLoc the source location of the \#if/\#ifdef/\#ifndef directive.
  virtual void Endif(SourceLocation Loc, SourceLocation IfLoc) {
  }
};

/// \brief Simple wrapper class for chaining callbacks.
class PPChainedCallbacks : public PPCallbacks {
  virtual void anchor();
  std::unique_ptr<PPCallbacks> First, Second;

public:
  PPChainedCallbacks(std::unique_ptr<PPCallbacks> _First,
                     std::unique_ptr<PPCallbacks> _Second)
    : First(std::move(_First)), Second(std::move(_Second)) {}

  void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                   SrcMgr::CharacteristicKind FileType,
                   FileID PrevFID) override {
    First->FileChanged(Loc, Reason, FileType, PrevFID);
    Second->FileChanged(Loc, Reason, FileType, PrevFID);
  }

  void FileSkipped(const FileEntry &SkippedFile,
                   const Token &FilenameTok,
                   SrcMgr::CharacteristicKind FileType) override {
    First->FileSkipped(SkippedFile, FilenameTok, FileType);
    Second->FileSkipped(SkippedFile, FilenameTok, FileType);
  }

  bool FileNotFound(StringRef FileName,
                    SmallVectorImpl<char> &RecoveryPath) override {
    return First->FileNotFound(FileName, RecoveryPath) ||
           Second->FileNotFound(FileName, RecoveryPath);
  }

  void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                          StringRef FileName, bool IsAngled,
                          CharSourceRange FilenameRange, const FileEntry *File,
                          StringRef SearchPath, StringRef RelativePath,
                          const Module *Imported) override {
    First->InclusionDirective(HashLoc, IncludeTok, FileName, IsAngled,
                              FilenameRange, File, SearchPath, RelativePath,
                              Imported);
    Second->InclusionDirective(HashLoc, IncludeTok, FileName, IsAngled,
                               FilenameRange, File, SearchPath, RelativePath,
                               Imported);
  }

  void moduleImport(SourceLocation ImportLoc, ModuleIdPath Path,
                    const Module *Imported) override {
    First->moduleImport(ImportLoc, Path, Imported);
    Second->moduleImport(ImportLoc, Path, Imported);
  }

  void EndOfMainFile() override {
    First->EndOfMainFile();
    Second->EndOfMainFile();
  }

  void Ident(SourceLocation Loc, StringRef str) override {
    First->Ident(Loc, str);
    Second->Ident(Loc, str);
  }

  void PragmaComment(SourceLocation Loc, const IdentifierInfo *Kind,
                     StringRef Str) override {
    First->PragmaComment(Loc, Kind, Str);
    Second->PragmaComment(Loc, Kind, Str);
  }

  void PragmaDetectMismatch(SourceLocation Loc, StringRef Name,
                            StringRef Value) override {
    First->PragmaDetectMismatch(Loc, Name, Value);
    Second->PragmaDetectMismatch(Loc, Name, Value);
  }

  void PragmaMessage(SourceLocation Loc, StringRef Namespace,
                     PragmaMessageKind Kind, StringRef Str) override {
    First->PragmaMessage(Loc, Namespace, Kind, Str);
    Second->PragmaMessage(Loc, Namespace, Kind, Str);
  }

  void PragmaDiagnosticPush(SourceLocation Loc, StringRef Namespace) override {
    First->PragmaDiagnosticPush(Loc, Namespace);
    Second->PragmaDiagnosticPush(Loc, Namespace);
  }

  void PragmaDiagnosticPop(SourceLocation Loc, StringRef Namespace) override {
    First->PragmaDiagnosticPop(Loc, Namespace);
    Second->PragmaDiagnosticPop(Loc, Namespace);
  }

  void PragmaDiagnostic(SourceLocation Loc, StringRef Namespace,
                        diag::Severity mapping, StringRef Str) override {
    First->PragmaDiagnostic(Loc, Namespace, mapping, Str);
    Second->PragmaDiagnostic(Loc, Namespace, mapping, Str);
  }

  void PragmaOpenCLExtension(SourceLocation NameLoc, const IdentifierInfo *Name,
                             SourceLocation StateLoc, unsigned State) override {
    First->PragmaOpenCLExtension(NameLoc, Name, StateLoc, State);
    Second->PragmaOpenCLExtension(NameLoc, Name, StateLoc, State);
  }

  void PragmaWarning(SourceLocation Loc, StringRef WarningSpec,
                     ArrayRef<int> Ids) override {
    First->PragmaWarning(Loc, WarningSpec, Ids);
    Second->PragmaWarning(Loc, WarningSpec, Ids);
  }

  void PragmaWarningPush(SourceLocation Loc, int Level) override {
    First->PragmaWarningPush(Loc, Level);
    Second->PragmaWarningPush(Loc, Level);
  }

  void PragmaWarningPop(SourceLocation Loc) override {
    First->PragmaWarningPop(Loc);
    Second->PragmaWarningPop(Loc);
  }

  void MacroExpands(const Token &MacroNameTok, const MacroDefinition &MD,
                    SourceRange Range, const MacroArgs *Args) override {
    First->MacroExpands(MacroNameTok, MD, Range, Args);
    Second->MacroExpands(MacroNameTok, MD, Range, Args);
  }

  void MacroDefined(const Token &MacroNameTok, const MacroDirective *MD) override {
    First->MacroDefined(MacroNameTok, MD);
    Second->MacroDefined(MacroNameTok, MD);
  }

  void MacroUndefined(const Token &MacroNameTok,
                      const MacroDefinition &MD) override {
    First->MacroUndefined(MacroNameTok, MD);
    Second->MacroUndefined(MacroNameTok, MD);
  }

  void Defined(const Token &MacroNameTok, const MacroDefinition &MD,
               SourceRange Range) override {
    First->Defined(MacroNameTok, MD, Range);
    Second->Defined(MacroNameTok, MD, Range);
  }

  void SourceRangeSkipped(SourceRange Range) override {
    First->SourceRangeSkipped(Range);
    Second->SourceRangeSkipped(Range);
  }

  /// \brief Hook called whenever an \#if is seen.
  void If(SourceLocation Loc, SourceRange ConditionRange,
          ConditionValueKind ConditionValue) override {
    First->If(Loc, ConditionRange, ConditionValue);
    Second->If(Loc, ConditionRange, ConditionValue);
  }

  /// \brief Hook called whenever an \#elif is seen.
  void Elif(SourceLocation Loc, SourceRange ConditionRange,
            ConditionValueKind ConditionValue, SourceLocation IfLoc) override {
    First->Elif(Loc, ConditionRange, ConditionValue, IfLoc);
    Second->Elif(Loc, ConditionRange, ConditionValue, IfLoc);
  }

  /// \brief Hook called whenever an \#ifdef is seen.
  void Ifdef(SourceLocation Loc, const Token &MacroNameTok,
             const MacroDefinition &MD) override {
    First->Ifdef(Loc, MacroNameTok, MD);
    Second->Ifdef(Loc, MacroNameTok, MD);
  }

  /// \brief Hook called whenever an \#ifndef is seen.
  void Ifndef(SourceLocation Loc, const Token &MacroNameTok,
              const MacroDefinition &MD) override {
    First->Ifndef(Loc, MacroNameTok, MD);
    Second->Ifndef(Loc, MacroNameTok, MD);
  }

  /// \brief Hook called whenever an \#else is seen.
  void Else(SourceLocation Loc, SourceLocation IfLoc) override {
    First->Else(Loc, IfLoc);
    Second->Else(Loc, IfLoc);
  }

  /// \brief Hook called whenever an \#endif is seen.
  void Endif(SourceLocation Loc, SourceLocation IfLoc) override {
    First->Endif(Loc, IfLoc);
    Second->Endif(Loc, IfLoc);
  }
};

}  // end namespace clang

#endif
