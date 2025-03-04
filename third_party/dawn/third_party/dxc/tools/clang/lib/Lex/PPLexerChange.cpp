//===--- PPLexerChange.cpp - Handle changing lexers in the preprocessor ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements pieces of the Preprocessor interface that manage the
// current lexer stack.
//
//===----------------------------------------------------------------------===//

#include "clang/Lex/Preprocessor.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/LexDiagnostic.h"
#include "clang/Lex/MacroInfo.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
using namespace clang;

PPCallbacks::~PPCallbacks() {}

//===----------------------------------------------------------------------===//
// Miscellaneous Methods.
//===----------------------------------------------------------------------===//

/// isInPrimaryFile - Return true if we're in the top-level file, not in a
/// \#include.  This looks through macro expansions and active _Pragma lexers.
bool Preprocessor::isInPrimaryFile() const {
  if (IsFileLexer())
    return IncludeMacroStack.empty();

  // If there are any stacked lexers, we're in a #include.
  assert(IsFileLexer(IncludeMacroStack[0]) &&
         "Top level include stack isn't our primary lexer?");
  for (unsigned i = 1, e = IncludeMacroStack.size(); i != e; ++i)
    if (IsFileLexer(IncludeMacroStack[i]))
      return false;
  return true;
}

/// getCurrentLexer - Return the current file lexer being lexed from.  Note
/// that this ignores any potentially active macro expansions and _Pragma
/// expansions going on at the time.
PreprocessorLexer *Preprocessor::getCurrentFileLexer() const {
  if (IsFileLexer())
    return CurPPLexer;

  // Look for a stacked lexer.
  for (unsigned i = IncludeMacroStack.size(); i != 0; --i) {
    const IncludeStackInfo& ISI = IncludeMacroStack[i-1];
    if (IsFileLexer(ISI))
      return ISI.ThePPLexer;
  }
  return nullptr;
}


//===----------------------------------------------------------------------===//
// Methods for Entering and Callbacks for leaving various contexts
//===----------------------------------------------------------------------===//

/// EnterSourceFile - Add a source file to the top of the include stack and
/// start lexing tokens from it instead of the current buffer.
bool Preprocessor::EnterSourceFile(FileID FID, const DirectoryLookup *CurDir,
                                   SourceLocation Loc) {
  assert(!CurTokenLexer && "Cannot #include a file inside a macro!");
  ++NumEnteredSourceFiles;

  if (MaxIncludeStackDepth < IncludeMacroStack.size())
    MaxIncludeStackDepth = IncludeMacroStack.size();

  if (PTH) {
    if (PTHLexer *PL = PTH->CreateLexer(FID)) {
      EnterSourceFileWithPTH(PL, CurDir);
      return false;
    }
  }
  
  // Get the MemoryBuffer for this FID, if it fails, we fail.
  bool Invalid = false;
  const llvm::MemoryBuffer *InputFile = 
    getSourceManager().getBuffer(FID, Loc, &Invalid);
  if (Invalid) {
    SourceLocation FileStart = SourceMgr.getLocForStartOfFile(FID);
    Diag(Loc, diag::err_pp_error_opening_file)
      << std::string(SourceMgr.getBufferName(FileStart)) << "";
    return true;
  }

  if (isCodeCompletionEnabled() &&
      SourceMgr.getFileEntryForID(FID) == CodeCompletionFile) {
    CodeCompletionFileLoc = SourceMgr.getLocForStartOfFile(FID);
    CodeCompletionLoc =
        CodeCompletionFileLoc.getLocWithOffset(CodeCompletionOffset);
  }

  EnterSourceFileWithLexer(new Lexer(FID, InputFile, *this), CurDir);
  return false;
}

/// EnterSourceFileWithLexer - Add a source file to the top of the include stack
///  and start lexing tokens from it instead of the current buffer.
void Preprocessor::EnterSourceFileWithLexer(Lexer *TheLexer,
                                            const DirectoryLookup *CurDir) {
  std::unique_ptr<Lexer> LexerGuard(TheLexer); // HLSL Change - guard
  // Add the current lexer to the include stack.
  if (CurPPLexer || CurTokenLexer)
    PushIncludeMacroStack();
  LexerGuard.release(); // HLSL Change
  CurLexer.reset(TheLexer);
  CurPPLexer = TheLexer;
  CurDirLookup = CurDir;
  CurSubmodule = nullptr;
  if (CurLexerKind != CLK_LexAfterModuleImport)
    CurLexerKind = CLK_Lexer;
  
  // Notify the client, if desired, that we are in a new source file.
  if (Callbacks && !CurLexer->Is_PragmaLexer) {
    SrcMgr::CharacteristicKind FileType =
       SourceMgr.getFileCharacteristic(CurLexer->getFileLoc());

    Callbacks->FileChanged(CurLexer->getFileLoc(),
                           PPCallbacks::EnterFile, FileType);
  }
}

/// EnterSourceFileWithPTH - Add a source file to the top of the include stack
/// and start getting tokens from it using the PTH cache.
void Preprocessor::EnterSourceFileWithPTH(PTHLexer *PL,
                                          const DirectoryLookup *CurDir) {

  if (CurPPLexer || CurTokenLexer)
    PushIncludeMacroStack();

  CurDirLookup = CurDir;
  CurPTHLexer.reset(PL);
  CurPPLexer = CurPTHLexer.get();
  CurSubmodule = nullptr;
  if (CurLexerKind != CLK_LexAfterModuleImport)
    CurLexerKind = CLK_PTHLexer;
  
  // Notify the client, if desired, that we are in a new source file.
  if (Callbacks) {
    FileID FID = CurPPLexer->getFileID();
    SourceLocation EnterLoc = SourceMgr.getLocForStartOfFile(FID);
    SrcMgr::CharacteristicKind FileType =
      SourceMgr.getFileCharacteristic(EnterLoc);
    Callbacks->FileChanged(EnterLoc, PPCallbacks::EnterFile, FileType);
  }
}

/// EnterMacro - Add a Macro to the top of the include stack and start lexing
/// tokens from it instead of the current buffer.
void Preprocessor::EnterMacro(Token &Tok, SourceLocation ILEnd,
                              MacroInfo *Macro, MacroArgs *Args) {
  std::unique_ptr<TokenLexer> TokLexer;
  if (NumCachedTokenLexers == 0) {
    TokLexer = llvm::make_unique<TokenLexer>(Tok, ILEnd, Macro, Args, *this);
  } else {
    TokLexer = std::move(TokenLexerCache[--NumCachedTokenLexers]);
    TokLexer->Init(Tok, ILEnd, Macro, Args);
  }

  PushIncludeMacroStack();
  CurDirLookup = nullptr;
  CurTokenLexer = std::move(TokLexer);
  if (CurLexerKind != CLK_LexAfterModuleImport)
    CurLexerKind = CLK_TokenLexer;
}

/// EnterTokenStream - Add a "macro" context to the top of the include stack,
/// which will cause the lexer to start returning the specified tokens.
///
/// If DisableMacroExpansion is true, tokens lexed from the token stream will
/// not be subject to further macro expansion.  Otherwise, these tokens will
/// be re-macro-expanded when/if expansion is enabled.
///
/// If OwnsTokens is false, this method assumes that the specified stream of
/// tokens has a permanent owner somewhere, so they do not need to be copied.
/// If it is true, it assumes the array of tokens is allocated with new[] and
/// must be freed.
///
void Preprocessor::EnterTokenStream(const Token *Toks, unsigned NumToks,
                                    bool DisableMacroExpansion,
                                    bool OwnsTokens) {
  if (CurLexerKind == CLK_CachingLexer) {
    if (CachedLexPos < CachedTokens.size()) {
      // We're entering tokens into the middle of our cached token stream. We
      // can't represent that, so just insert the tokens into the buffer.
      CachedTokens.insert(CachedTokens.begin() + CachedLexPos,
                          Toks, Toks + NumToks);
      if (OwnsTokens)
        delete [] Toks;
      return;
    }

    // New tokens are at the end of the cached token sequnece; insert the
    // token stream underneath the caching lexer.
    ExitCachingLexMode();
    EnterTokenStream(Toks, NumToks, DisableMacroExpansion, OwnsTokens);
    EnterCachingLexMode();
    return;
  }

  // Create a macro expander to expand from the specified token stream.
  std::unique_ptr<TokenLexer> TokLexer;
  if (NumCachedTokenLexers == 0) {
    TokLexer = llvm::make_unique<TokenLexer>(
        Toks, NumToks, DisableMacroExpansion, OwnsTokens, *this);
  } else {
    TokLexer = std::move(TokenLexerCache[--NumCachedTokenLexers]);
    TokLexer->Init(Toks, NumToks, DisableMacroExpansion, OwnsTokens);
  }

  // Save our current state.
  PushIncludeMacroStack();
  CurDirLookup = nullptr;
  CurTokenLexer = std::move(TokLexer);
  if (CurLexerKind != CLK_LexAfterModuleImport)
    CurLexerKind = CLK_TokenLexer;
}

/// \brief Compute the relative path that names the given file relative to
/// the given directory.
static void computeRelativePath(FileManager &FM, const DirectoryEntry *Dir,
                                const FileEntry *File,
                                SmallString<128> &Result) {
  Result.clear();

  StringRef FilePath = File->getDir()->getName();
  StringRef Path = FilePath;
  while (!Path.empty()) {
    if (const DirectoryEntry *CurDir = FM.getDirectory(Path)) {
      if (CurDir == Dir) {
        Result = FilePath.substr(Path.size());
        llvm::sys::path::append(Result, 
                                llvm::sys::path::filename(File->getName()));
        return;
      }
    }
    
    Path = llvm::sys::path::parent_path(Path);
  }
  
  Result = File->getName();
}

void Preprocessor::PropagateLineStartLeadingSpaceInfo(Token &Result) {
  if (CurTokenLexer) {
    CurTokenLexer->PropagateLineStartLeadingSpaceInfo(Result);
    return;
  }
  if (CurLexer) {
    CurLexer->PropagateLineStartLeadingSpaceInfo(Result);
    return;
  }
  // FIXME: Handle other kinds of lexers?  It generally shouldn't matter,
  // but it might if they're empty?
}

/// \brief Determine the location to use as the end of the buffer for a lexer.
///
/// If the file ends with a newline, form the EOF token on the newline itself,
/// rather than "on the line following it", which doesn't exist.  This makes
/// diagnostics relating to the end of file include the last file that the user
/// actually typed, which is goodness.
const char *Preprocessor::getCurLexerEndPos() {
  const char *EndPos = CurLexer->BufferEnd;
  if (EndPos != CurLexer->BufferStart &&
      (EndPos[-1] == '\n' || EndPos[-1] == '\r')) {
    --EndPos;

    // Handle \n\r and \r\n:
    if (EndPos != CurLexer->BufferStart &&
        (EndPos[-1] == '\n' || EndPos[-1] == '\r') &&
        EndPos[-1] != EndPos[0])
      --EndPos;
  }

  return EndPos;
}


/// HandleEndOfFile - This callback is invoked when the lexer hits the end of
/// the current file.  This either returns the EOF token or pops a level off
/// the include stack and keeps going.
bool Preprocessor::HandleEndOfFile(Token &Result, bool isEndOfMacro) {
  assert(!CurTokenLexer &&
         "Ending a file when currently in a macro!");

  // See if this file had a controlling macro.
  if (CurPPLexer) {  // Not ending a macro, ignore it.
    if (const IdentifierInfo *ControllingMacro =
          CurPPLexer->MIOpt.GetControllingMacroAtEndOfFile()) {
      // Okay, this has a controlling macro, remember in HeaderFileInfo.
      if (const FileEntry *FE =
            SourceMgr.getFileEntryForID(CurPPLexer->getFileID())) {
        HeaderInfo.SetFileControllingMacro(FE, ControllingMacro);
        if (MacroInfo *MI =
              getMacroInfo(const_cast<IdentifierInfo*>(ControllingMacro))) {
          MI->UsedForHeaderGuard = true;
        }
        if (const IdentifierInfo *DefinedMacro =
              CurPPLexer->MIOpt.GetDefinedMacro()) {
          if (!isMacroDefined(ControllingMacro) &&
              DefinedMacro != ControllingMacro &&
              HeaderInfo.FirstTimeLexingFile(FE)) {

            // If the edit distance between the two macros is more than 50%,
            // DefinedMacro may not be header guard, or can be header guard of
            // another header file. Therefore, it maybe defining something
            // completely different. This can be observed in the wild when
            // handling feature macros or header guards in different files.

            const StringRef ControllingMacroName = ControllingMacro->getName();
            const StringRef DefinedMacroName = DefinedMacro->getName();
            const size_t MaxHalfLength = std::max(ControllingMacroName.size(),
                                                  DefinedMacroName.size()) / 2;
            const unsigned ED = ControllingMacroName.edit_distance(
                DefinedMacroName, true, MaxHalfLength);
            if (ED <= MaxHalfLength) {
              // Emit a warning for a bad header guard.
              Diag(CurPPLexer->MIOpt.GetMacroLocation(),
                   diag::warn_header_guard)
                  << CurPPLexer->MIOpt.GetMacroLocation() << ControllingMacro;
              Diag(CurPPLexer->MIOpt.GetDefinedLocation(),
                   diag::note_header_guard)
                  << CurPPLexer->MIOpt.GetDefinedLocation() << DefinedMacro
                  << ControllingMacro
                  << FixItHint::CreateReplacement(
                         CurPPLexer->MIOpt.GetDefinedLocation(),
                         ControllingMacro->getName());
            }
          }
        }
      }
    }
  }

  // Complain about reaching a true EOF within arc_cf_code_audited.
  // We don't want to complain about reaching the end of a macro
  // instantiation or a _Pragma.
  if (PragmaARCCFCodeAuditedLoc.isValid() &&
      !isEndOfMacro && !(CurLexer && CurLexer->Is_PragmaLexer)) {
    Diag(PragmaARCCFCodeAuditedLoc, diag::err_pp_eof_in_arc_cf_code_audited);

    // Recover by leaving immediately.
    PragmaARCCFCodeAuditedLoc = SourceLocation();
  }

  // Complain about reaching a true EOF within assume_nonnull.
  // We don't want to complain about reaching the end of a macro
  // instantiation or a _Pragma.
  if (PragmaAssumeNonNullLoc.isValid() &&
      !isEndOfMacro && !(CurLexer && CurLexer->Is_PragmaLexer)) {
    Diag(PragmaAssumeNonNullLoc, diag::err_pp_eof_in_assume_nonnull);

    // Recover by leaving immediately.
    PragmaAssumeNonNullLoc = SourceLocation();
  }

  // If this is a #include'd file, pop it off the include stack and continue
  // lexing the #includer file.
  if (!IncludeMacroStack.empty()) {

    // If we lexed the code-completion file, act as if we reached EOF.
    if (isCodeCompletionEnabled() && CurPPLexer &&
        SourceMgr.getLocForStartOfFile(CurPPLexer->getFileID()) ==
            CodeCompletionFileLoc) {
      if (CurLexer) {
        Result.startToken();
        CurLexer->FormTokenWithChars(Result, CurLexer->BufferEnd, tok::eof);
        CurLexer.reset();
      } else {
        assert(CurPTHLexer && "Got EOF but no current lexer set!");
        CurPTHLexer->getEOF(Result);
        CurPTHLexer.reset();
      }

      CurPPLexer = nullptr;
      return true;
    }

    if (!isEndOfMacro && CurPPLexer &&
        SourceMgr.getIncludeLoc(CurPPLexer->getFileID()).isValid()) {
      // Notify SourceManager to record the number of FileIDs that were created
      // during lexing of the #include'd file.
      unsigned NumFIDs =
          SourceMgr.local_sloc_entry_size() -
          CurPPLexer->getInitialNumSLocEntries() + 1/*#include'd file*/;
      SourceMgr.setNumCreatedFIDsForFileID(CurPPLexer->getFileID(), NumFIDs);
    }

    FileID ExitedFID;
    if (Callbacks && !isEndOfMacro && CurPPLexer)
      ExitedFID = CurPPLexer->getFileID();

    bool LeavingSubmodule = CurSubmodule && CurLexer;
    if (LeavingSubmodule) {
      // Notify the parser that we've left the module.
      const char *EndPos = getCurLexerEndPos();
      Result.startToken();
      CurLexer->BufferPtr = EndPos;
      CurLexer->FormTokenWithChars(Result, EndPos, tok::annot_module_end);
      Result.setAnnotationEndLoc(Result.getLocation());
      Result.setAnnotationValue(CurSubmodule);

      // We're done with this submodule.
      LeaveSubmodule();
    }

    // We're done with the #included file.
    RemoveTopOfLexerStack();

    // Propagate info about start-of-line/leading white-space/etc.
    PropagateLineStartLeadingSpaceInfo(Result);

    // Notify the client, if desired, that we are in a new source file.
    if (Callbacks && !isEndOfMacro && CurPPLexer) {
      SrcMgr::CharacteristicKind FileType =
        SourceMgr.getFileCharacteristic(CurPPLexer->getSourceLocation());
      Callbacks->FileChanged(CurPPLexer->getSourceLocation(),
                             PPCallbacks::ExitFile, FileType, ExitedFID);
    }

    // Client should lex another token unless we generated an EOM.
    return LeavingSubmodule;
  }

  // If this is the end of the main file, form an EOF token.
  if (CurLexer) {
    const char *EndPos = getCurLexerEndPos();
    Result.startToken();
    CurLexer->BufferPtr = EndPos;
    CurLexer->FormTokenWithChars(Result, EndPos, tok::eof);

    if (isCodeCompletionEnabled()) {
      // Inserting the code-completion point increases the source buffer by 1,
      // but the main FileID was created before inserting the point.
      // Compensate by reducing the EOF location by 1, otherwise the location
      // will point to the next FileID.
      // FIXME: This is hacky, the code-completion point should probably be
      // inserted before the main FileID is created.
      if (CurLexer->getFileLoc() == CodeCompletionFileLoc)
        Result.setLocation(Result.getLocation().getLocWithOffset(-1));
    }

    if (!isIncrementalProcessingEnabled())
      // We're done with lexing.
      CurLexer.reset();
  } else {
    assert(CurPTHLexer && "Got EOF but no current lexer set!");
    CurPTHLexer->getEOF(Result);
    CurPTHLexer.reset();
  }
  
  if (!isIncrementalProcessingEnabled())
    CurPPLexer = nullptr;

  if (TUKind == TU_Complete) {
    // This is the end of the top-level file. 'WarnUnusedMacroLocs' has
    // collected all macro locations that we need to warn because they are not
    // used.
    for (WarnUnusedMacroLocsTy::iterator
           I=WarnUnusedMacroLocs.begin(), E=WarnUnusedMacroLocs.end();
           I!=E; ++I)
      Diag(*I, diag::pp_macro_not_used);
  }

  // If we are building a module that has an umbrella header, make sure that
  // each of the headers within the directory covered by the umbrella header
  // was actually included by the umbrella header.
  if (Module *Mod = getCurrentModule()) {
    if (Mod->getUmbrellaHeader()) {
      SourceLocation StartLoc
        = SourceMgr.getLocForStartOfFile(SourceMgr.getMainFileID());

      if (!getDiagnostics().isIgnored(diag::warn_uncovered_module_header,
                                      StartLoc)) {
        ModuleMap &ModMap = getHeaderSearchInfo().getModuleMap();
        const DirectoryEntry *Dir = Mod->getUmbrellaDir().Entry;
        vfs::FileSystem &FS = *FileMgr.getVirtualFileSystem();
        std::error_code EC;
        for (vfs::recursive_directory_iterator Entry(FS, Dir->getName(), EC), End;
             Entry != End && !EC; Entry.increment(EC)) {
          using llvm::StringSwitch;
          
          // Check whether this entry has an extension typically associated with
          // headers.
          if (!StringSwitch<bool>(llvm::sys::path::extension(Entry->getName()))
                 .Cases(".h", ".H", ".hh", ".hpp", true)
                 .Default(false))
            continue;

          if (const FileEntry *Header =
                  getFileManager().getFile(Entry->getName()))
            if (!getSourceManager().hasFileInfo(Header)) {
              if (!ModMap.isHeaderInUnavailableModule(Header)) {
                // Find the relative path that would access this header.
                SmallString<128> RelativePath;
                computeRelativePath(FileMgr, Dir, Header, RelativePath);              
                Diag(StartLoc, diag::warn_uncovered_module_header)
                  << Mod->getFullModuleName() << RelativePath;
              }
            }
        }
      }
    }
  }

  return true;
}

/// HandleEndOfTokenLexer - This callback is invoked when the current TokenLexer
/// hits the end of its token stream.
bool Preprocessor::HandleEndOfTokenLexer(Token &Result) {
  assert(CurTokenLexer && !CurPPLexer &&
         "Ending a macro when currently in a #include file!");

  if (!MacroExpandingLexersStack.empty() &&
      MacroExpandingLexersStack.back().first == CurTokenLexer.get())
    removeCachedMacroExpandedTokensOfLastLexer();

  // Delete or cache the now-dead macro expander.
  if (NumCachedTokenLexers == TokenLexerCacheSize)
    CurTokenLexer.reset();
  else
    TokenLexerCache[NumCachedTokenLexers++] = std::move(CurTokenLexer);

  // Handle this like a #include file being popped off the stack.
  return HandleEndOfFile(Result, true);
}

/// RemoveTopOfLexerStack - Pop the current lexer/macro exp off the top of the
/// lexer stack.  This should only be used in situations where the current
/// state of the top-of-stack lexer is unknown.
void Preprocessor::RemoveTopOfLexerStack() {
  assert(!IncludeMacroStack.empty() && "Ran out of stack entries to load");

  if (CurTokenLexer) {
    // Delete or cache the now-dead macro expander.
    if (NumCachedTokenLexers == TokenLexerCacheSize)
      CurTokenLexer.reset();
    else
      TokenLexerCache[NumCachedTokenLexers++] = std::move(CurTokenLexer);
  }

  PopIncludeMacroStack();
}

/// HandleMicrosoftCommentPaste - When the macro expander pastes together a
/// comment (/##/) in microsoft mode, this method handles updating the current
/// state, returning the token on the next source line.
void Preprocessor::HandleMicrosoftCommentPaste(Token &Tok) {
  assert(CurTokenLexer && !CurPPLexer &&
         "Pasted comment can only be formed from macro");

  // We handle this by scanning for the closest real lexer, switching it to
  // raw mode and preprocessor mode.  This will cause it to return \n as an
  // explicit EOD token.
  PreprocessorLexer *FoundLexer = nullptr;
  bool LexerWasInPPMode = false;
  for (unsigned i = 0, e = IncludeMacroStack.size(); i != e; ++i) {
    IncludeStackInfo &ISI = *(IncludeMacroStack.end()-i-1);
    if (ISI.ThePPLexer == nullptr) continue;  // Scan for a real lexer.

    // Once we find a real lexer, mark it as raw mode (disabling macro
    // expansions) and preprocessor mode (return EOD).  We know that the lexer
    // was *not* in raw mode before, because the macro that the comment came
    // from was expanded.  However, it could have already been in preprocessor
    // mode (#if COMMENT) in which case we have to return it to that mode and
    // return EOD.
    FoundLexer = ISI.ThePPLexer;
    FoundLexer->LexingRawMode = true;
    LexerWasInPPMode = FoundLexer->ParsingPreprocessorDirective;
    FoundLexer->ParsingPreprocessorDirective = true;
    break;
  }

  // Okay, we either found and switched over the lexer, or we didn't find a
  // lexer.  In either case, finish off the macro the comment came from, getting
  // the next token.
  if (!HandleEndOfTokenLexer(Tok)) Lex(Tok);

  // Discarding comments as long as we don't have EOF or EOD.  This 'comments
  // out' the rest of the line, including any tokens that came from other macros
  // that were active, as in:
  //  #define submacro a COMMENT b
  //    submacro c
  // which should lex to 'a' only: 'b' and 'c' should be removed.
  while (Tok.isNot(tok::eod) && Tok.isNot(tok::eof))
    Lex(Tok);

  // If we got an eod token, then we successfully found the end of the line.
  if (Tok.is(tok::eod)) {
    assert(FoundLexer && "Can't get end of line without an active lexer");
    // Restore the lexer back to normal mode instead of raw mode.
    FoundLexer->LexingRawMode = false;

    // If the lexer was already in preprocessor mode, just return the EOD token
    // to finish the preprocessor line.
    if (LexerWasInPPMode) return;

    // Otherwise, switch out of PP mode and return the next lexed token.
    FoundLexer->ParsingPreprocessorDirective = false;
    return Lex(Tok);
  }

  // If we got an EOF token, then we reached the end of the token stream but
  // didn't find an explicit \n.  This can only happen if there was no lexer
  // active (an active lexer would return EOD at EOF if there was no \n in
  // preprocessor directive mode), so just return EOF as our token.
  assert(!FoundLexer && "Lexer should return EOD before EOF in PP mode");
}

void Preprocessor::EnterSubmodule(Module *M, SourceLocation ImportLoc) {
  if (!getLangOpts().ModulesLocalVisibility) {
    // Just track that we entered this submodule.
    BuildingSubmoduleStack.push_back(
        BuildingSubmoduleInfo(M, ImportLoc, CurSubmoduleState));
    return;
  }

  // Resolve as much of the module definition as we can now, before we enter
  // one of its headers.
  // FIXME: Can we enable Complain here?
  // FIXME: Can we do this when local visibility is disabled?
  ModuleMap &ModMap = getHeaderSearchInfo().getModuleMap();
  ModMap.resolveExports(M, /*Complain=*/false);
  ModMap.resolveUses(M, /*Complain=*/false);
  ModMap.resolveConflicts(M, /*Complain=*/false);

  // If this is the first time we've entered this module, set up its state.
  auto R = Submodules.insert(std::make_pair(M, SubmoduleState()));
  auto &State = R.first->second;
  bool FirstTime = R.second;
  if (FirstTime) {
    // Determine the set of starting macros for this submodule; take these
    // from the "null" module (the predefines buffer).
    //
    // FIXME: If we have local visibility but not modules enabled, the
    // NullSubmoduleState is polluted by #defines in the top-level source
    // file.
    auto &StartingMacros = NullSubmoduleState.Macros;

    // Restore to the starting state.
    // FIXME: Do this lazily, when each macro name is first referenced.
    for (auto &Macro : StartingMacros) {
      // Skip uninteresting macros.
      if (!Macro.second.getLatest() &&
          Macro.second.getOverriddenMacros().empty())
        continue;

      MacroState MS(Macro.second.getLatest());
      MS.setOverriddenMacros(*this, Macro.second.getOverriddenMacros());
      State.Macros.insert(std::make_pair(Macro.first, std::move(MS)));
    }
  }

  // Track that we entered this module.
  BuildingSubmoduleStack.push_back(
      BuildingSubmoduleInfo(M, ImportLoc, CurSubmoduleState));

  // Switch to this submodule as the current submodule.
  CurSubmoduleState = &State;

  // This module is visible to itself.
  if (FirstTime)
    makeModuleVisible(M, ImportLoc);
}

void Preprocessor::LeaveSubmodule() {
  auto &Info = BuildingSubmoduleStack.back();

  Module *LeavingMod = Info.M;
  SourceLocation ImportLoc = Info.ImportLoc;

  // Create ModuleMacros for any macros defined in this submodule.
  for (auto &Macro : CurSubmoduleState->Macros) {
    auto *II = const_cast<IdentifierInfo*>(Macro.first);

    // Find the starting point for the MacroDirective chain in this submodule.
    MacroDirective *OldMD = nullptr;
    if (getLangOpts().ModulesLocalVisibility) {
      // FIXME: It'd be better to start at the state from when we most recently
      // entered this submodule, but it doesn't really matter.
      auto &PredefMacros = NullSubmoduleState.Macros;
      auto PredefMacroIt = PredefMacros.find(Macro.first);
      if (PredefMacroIt == PredefMacros.end())
        OldMD = nullptr;
      else
        OldMD = PredefMacroIt->second.getLatest();
    }

    // This module may have exported a new macro. If so, create a ModuleMacro
    // representing that fact.
    bool ExplicitlyPublic = false;
    for (auto *MD = Macro.second.getLatest(); MD != OldMD;
         MD = MD->getPrevious()) {
      assert(MD && "broken macro directive chain");

      // Stop on macros defined in other submodules we #included along the way.
      // There's no point doing this if we're tracking local submodule
      // visibility, since there can be no such directives in our list.
      if (!getLangOpts().ModulesLocalVisibility) {
        Module *Mod = getModuleContainingLocation(MD->getLocation());
        if (Mod != LeavingMod)
          break;
      }

      if (auto *VisMD = dyn_cast<VisibilityMacroDirective>(MD)) {
        // The latest visibility directive for a name in a submodule affects
        // all the directives that come before it.
        if (VisMD->isPublic())
          ExplicitlyPublic = true;
        else if (!ExplicitlyPublic)
          // Private with no following public directive: not exported.
          break;
      } else {
        MacroInfo *Def = nullptr;
        if (DefMacroDirective *DefMD = dyn_cast<DefMacroDirective>(MD))
          Def = DefMD->getInfo();

        // FIXME: Issue a warning if multiple headers for the same submodule
        // define a macro, rather than silently ignoring all but the first.
        bool IsNew;
        // Don't bother creating a module macro if it would represent a #undef
        // that doesn't override anything.
        if (Def || !Macro.second.getOverriddenMacros().empty())
          addModuleMacro(LeavingMod, II, Def,
                         Macro.second.getOverriddenMacros(), IsNew);
        break;
      }
    }
  }

  // FIXME: Before we leave this submodule, we should parse all the other
  // headers within it. Otherwise, we're left with an inconsistent state
  // where we've made the module visible but don't yet have its complete
  // contents.

  // Put back the outer module's state, if we're tracking it.
  if (getLangOpts().ModulesLocalVisibility)
    CurSubmoduleState = Info.OuterSubmoduleState;

  BuildingSubmoduleStack.pop_back();

  // A nested #include makes the included submodule visible.
  makeModuleVisible(LeavingMod, ImportLoc);
}
