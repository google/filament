//===--- PPDirectives.cpp - Directive Handling for Preprocessor -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements # directive processing for the Preprocessor.
///
//===----------------------------------------------------------------------===//

#include "clang/Lex/Preprocessor.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/CodeCompletionHandler.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Lex/LexDiagnostic.h"
#include "clang/Lex/LiteralSupport.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/ModuleLoader.h"
#include "clang/Lex/Pragma.h"
#include "llvm/ADT/APInt.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/SaveAndRestore.h"
#include "clang/Lex/PreprocessorOptions.h" // HLSL Change - ignore line directives.
using namespace clang;

//===----------------------------------------------------------------------===//
// Utility Methods for Preprocessor Directive Handling.
//===----------------------------------------------------------------------===//

MacroInfo *Preprocessor::AllocateMacroInfo() {
  MacroInfoChain *MIChain = BP.Allocate<MacroInfoChain>();
  MIChain->Next = MIChainHead;
  MIChainHead = MIChain;
  return &MIChain->MI;
}

MacroInfo *Preprocessor::AllocateMacroInfo(SourceLocation L) {
  MacroInfo *MI = AllocateMacroInfo();
  new (MI) MacroInfo(L);
  return MI;
}

MacroInfo *Preprocessor::AllocateDeserializedMacroInfo(SourceLocation L,
                                                       unsigned SubModuleID) {
  static_assert(llvm::AlignOf<MacroInfo>::Alignment >= sizeof(SubModuleID),
                "alignment for MacroInfo is less than the ID");
  DeserializedMacroInfoChain *MIChain =
      BP.Allocate<DeserializedMacroInfoChain>();
  MIChain->Next = DeserialMIChainHead;
  DeserialMIChainHead = MIChain;

  MacroInfo *MI = &MIChain->MI;
  new (MI) MacroInfo(L);
  MI->FromASTFile = true;
  MI->setOwningModuleID(SubModuleID);
  return MI;
}

DefMacroDirective *Preprocessor::AllocateDefMacroDirective(MacroInfo *MI,
                                                           SourceLocation Loc) {
  return new (BP) DefMacroDirective(MI, Loc);
}

UndefMacroDirective *
Preprocessor::AllocateUndefMacroDirective(SourceLocation UndefLoc) {
  return new (BP) UndefMacroDirective(UndefLoc);
}

VisibilityMacroDirective *
Preprocessor::AllocateVisibilityMacroDirective(SourceLocation Loc,
                                               bool isPublic) {
  return new (BP) VisibilityMacroDirective(Loc, isPublic);
}

/// \brief Read and discard all tokens remaining on the current line until
/// the tok::eod token is found.
void Preprocessor::DiscardUntilEndOfDirective() {
  Token Tmp;
  do {
    LexUnexpandedToken(Tmp);
    assert(Tmp.isNot(tok::eof) && "EOF seen while discarding directive tokens");
  } while (Tmp.isNot(tok::eod));
}

/// \brief Enumerates possible cases of #define/#undef a reserved identifier.
enum MacroDiag {
  MD_NoWarn,        //> Not a reserved identifier
  MD_KeywordDef,    //> Macro hides keyword, enabled by default
  MD_ReservedMacro  //> #define of #undef reserved id, disabled by default
};

/// \brief Checks if the specified identifier is reserved in the specified
/// language.
/// This function does not check if the identifier is a keyword.
static bool isReservedId(StringRef Text, const LangOptions &Lang) {
  // C++ [macro.names], C11 7.1.3:
  // All identifiers that begin with an underscore and either an uppercase
  // letter or another underscore are always reserved for any use.
  if (Text.size() >= 2 && Text[0] == '_' &&
      (isUppercase(Text[1]) || Text[1] == '_'))
      return true;
  // C++ [global.names]
  // Each name that contains a double underscore ... is reserved to the
  // implementation for any use.
  if (Lang.CPlusPlus) {
    if (Text.find("__") != StringRef::npos)
      return true;
  }
  return false;
}

static MacroDiag shouldWarnOnMacroDef(Preprocessor &PP, IdentifierInfo *II) {
  const LangOptions &Lang = PP.getLangOpts();
  StringRef Text = II->getName();
  if (isReservedId(Text, Lang))
    return MD_ReservedMacro;
  if (II->isKeyword(Lang))
    return MD_KeywordDef;
  if (Lang.CPlusPlus11 && (Text.equals("override") || Text.equals("final")))
    return MD_KeywordDef;
  return MD_NoWarn;
}

static MacroDiag shouldWarnOnMacroUndef(Preprocessor &PP, IdentifierInfo *II) {
  const LangOptions &Lang = PP.getLangOpts();
  StringRef Text = II->getName();
  // Do not warn on keyword undef.  It is generally harmless and widely used.
  if (isReservedId(Text, Lang))
    return MD_ReservedMacro;
  return MD_NoWarn;
}

bool Preprocessor::CheckMacroName(Token &MacroNameTok, MacroUse isDefineUndef,
                                  bool *ShadowFlag) {
  // Missing macro name?
  if (MacroNameTok.is(tok::eod))
    return Diag(MacroNameTok, diag::err_pp_missing_macro_name);

  IdentifierInfo *II = MacroNameTok.getIdentifierInfo();
  if (!II) {
    bool Invalid = false;
    std::string Spelling = getSpelling(MacroNameTok, &Invalid);
    if (Invalid)
      return Diag(MacroNameTok, diag::err_pp_macro_not_identifier);
    II = getIdentifierInfo(Spelling);

    if (!II->isCPlusPlusOperatorKeyword() || getLangOpts().HLSL) // HLSL Change: guard against HLSL
      return Diag(MacroNameTok, diag::err_pp_macro_not_identifier);

    // C++ 2.5p2: Alternative tokens behave the same as its primary token
    // except for their spellings.
    Diag(MacroNameTok, getLangOpts().MicrosoftExt
                           ? diag::ext_pp_operator_used_as_macro_name
                           : diag::err_pp_operator_used_as_macro_name)
        << II << MacroNameTok.getKind();

    // Allow #defining |and| and friends for Microsoft compatibility or
    // recovery when legacy C headers are included in C++.
    MacroNameTok.setIdentifierInfo(II);
  }

  if ((isDefineUndef != MU_Other) && II->getPPKeywordID() == tok::pp_defined) {
    // Error if defining "defined": C99 6.10.8/4, C++ [cpp.predefined]p4.
    return Diag(MacroNameTok, diag::err_defined_macro_name);
  }

  if (isDefineUndef == MU_Undef) {
    auto *MI = getMacroInfo(II);
    if (MI && MI->isBuiltinMacro()) {
      // Warn if undefining "__LINE__" and other builtins, per C99 6.10.8/4
      // and C++ [cpp.predefined]p4], but allow it as an extension.
      Diag(MacroNameTok, diag::ext_pp_undef_builtin_macro);
    }
  }

  // If defining/undefining reserved identifier or a keyword, we need to issue
  // a warning.
  SourceLocation MacroNameLoc = MacroNameTok.getLocation();
  if (ShadowFlag)
    *ShadowFlag = false;
  if (!SourceMgr.isInSystemHeader(MacroNameLoc) &&
      (strcmp(SourceMgr.getBufferName(MacroNameLoc), "<built-in>") != 0)) {
    MacroDiag D = MD_NoWarn;
    if (isDefineUndef == MU_Define) {
      D = shouldWarnOnMacroDef(*this, II);
    }
    else if (isDefineUndef == MU_Undef)
      D = shouldWarnOnMacroUndef(*this, II);
    if (D == MD_KeywordDef) {
      // We do not want to warn on some patterns widely used in configuration
      // scripts.  This requires analyzing next tokens, so do not issue warnings
      // now, only inform caller.
      if (ShadowFlag)
        *ShadowFlag = true;
    }
    if (D == MD_ReservedMacro)
      Diag(MacroNameTok, diag::warn_pp_macro_is_reserved_id);
  }

  // Okay, we got a good identifier.
  return false;
}

/// \brief Lex and validate a macro name, which occurs after a
/// \#define or \#undef.
///
/// This sets the token kind to eod and discards the rest of the macro line if
/// the macro name is invalid.
///
/// \param MacroNameTok Token that is expected to be a macro name.
/// \param isDefineUndef Context in which macro is used.
/// \param ShadowFlag Points to a flag that is set if macro shadows a keyword.
void Preprocessor::ReadMacroName(Token &MacroNameTok, MacroUse isDefineUndef,
                                 bool *ShadowFlag) {
  // Read the token, don't allow macro expansion on it.
  LexUnexpandedToken(MacroNameTok);

  if (MacroNameTok.is(tok::code_completion)) {
    if (CodeComplete)
      CodeComplete->CodeCompleteMacroName(isDefineUndef == MU_Define);
    setCodeCompletionReached();
    LexUnexpandedToken(MacroNameTok);
  }

  if (!CheckMacroName(MacroNameTok, isDefineUndef, ShadowFlag))
    return;

  // Invalid macro name, read and discard the rest of the line and set the
  // token kind to tok::eod if necessary.
  if (MacroNameTok.isNot(tok::eod)) {
    MacroNameTok.setKind(tok::eod);
    DiscardUntilEndOfDirective();
  }
}

/// \brief Ensure that the next token is a tok::eod token.
///
/// If not, emit a diagnostic and consume up until the eod.  If EnableMacros is
/// true, then we consider macros that expand to zero tokens as being ok.
void Preprocessor::CheckEndOfDirective(const char *DirType, bool EnableMacros) {
  Token Tmp;
  // Lex unexpanded tokens for most directives: macros might expand to zero
  // tokens, causing us to miss diagnosing invalid lines.  Some directives (like
  // #line) allow empty macros.
  if (EnableMacros)
    Lex(Tmp);
  else
    LexUnexpandedToken(Tmp);

  // There should be no tokens after the directive, but we allow them as an
  // extension.
  while (Tmp.is(tok::comment))  // Skip comments in -C mode.
    LexUnexpandedToken(Tmp);

  if (Tmp.isNot(tok::eod)) {
    // Add a fixit in GNU/C99/C++ mode.  Don't offer a fixit for strict-C89,
    // or if this is a macro-style preprocessing directive, because it is more
    // trouble than it is worth to insert /**/ and check that there is no /**/
    // in the range also.
    FixItHint Hint;
    if ((LangOpts.GNUMode || LangOpts.C99 || LangOpts.CPlusPlus) &&
        !CurTokenLexer)
      Hint = FixItHint::CreateInsertion(Tmp.getLocation(),"//");
    Diag(Tmp, diag::ext_pp_extra_tokens_at_eol) << DirType << Hint;
    DiscardUntilEndOfDirective();
  }
}



/// SkipExcludedConditionalBlock - We just read a \#if or related directive and
/// decided that the subsequent tokens are in the \#if'd out portion of the
/// file.  Lex the rest of the file, until we see an \#endif.  If
/// FoundNonSkipPortion is true, then we have already emitted code for part of
/// this \#if directive, so \#else/\#elif blocks should never be entered.
/// If ElseOk is true, then \#else directives are ok, if not, then we have
/// already seen one so a \#else directive is a duplicate.  When this returns,
/// the caller can lex the first valid token.
void Preprocessor::SkipExcludedConditionalBlock(SourceLocation IfTokenLoc,
                                                bool FoundNonSkipPortion,
                                                bool FoundElse,
                                                SourceLocation ElseLoc) {
  ++NumSkipped;
  assert(!CurTokenLexer && CurPPLexer && "Lexing a macro, not a file?");

  CurPPLexer->pushConditionalLevel(IfTokenLoc, /*isSkipping*/false,
                                 FoundNonSkipPortion, FoundElse);

  if (CurPTHLexer) {
    PTHSkipExcludedConditionalBlock();
    return;
  }

  // Enter raw mode to disable identifier lookup (and thus macro expansion),
  // disabling warnings, etc.
  CurPPLexer->LexingRawMode = true;
  Token Tok;
  while (1) {
    CurLexer->Lex(Tok);

    if (Tok.is(tok::code_completion)) {
      if (CodeComplete)
        CodeComplete->CodeCompleteInConditionalExclusion();
      setCodeCompletionReached();
      continue;
    }
    
    // If this is the end of the buffer, we have an error.
    if (Tok.is(tok::eof)) {
      // Emit errors for each unterminated conditional on the stack, including
      // the current one.
      while (!CurPPLexer->ConditionalStack.empty()) {
        if (CurLexer->getFileLoc() != CodeCompletionFileLoc)
          Diag(CurPPLexer->ConditionalStack.back().IfLoc,
               diag::err_pp_unterminated_conditional);
        CurPPLexer->ConditionalStack.pop_back();
      }

      // Just return and let the caller lex after this #include.
      break;
    }

    // If this token is not a preprocessor directive, just skip it.
    if (Tok.isNot(tok::hash) || !Tok.isAtStartOfLine())
      continue;

    // We just parsed a # character at the start of a line, so we're in
    // directive mode.  Tell the lexer this so any newlines we see will be
    // converted into an EOD token (this terminates the macro).
    CurPPLexer->ParsingPreprocessorDirective = true;
    if (CurLexer) CurLexer->SetKeepWhitespaceMode(false);


    // Read the next token, the directive flavor.
    LexUnexpandedToken(Tok);

    // If this isn't an identifier directive (e.g. is "# 1\n" or "#\n", or
    // something bogus), skip it.
    if (Tok.isNot(tok::raw_identifier)) {
      CurPPLexer->ParsingPreprocessorDirective = false;
      // Restore comment saving mode.
      if (CurLexer) CurLexer->resetExtendedTokenMode();
      continue;
    }

    // If the first letter isn't i or e, it isn't intesting to us.  We know that
    // this is safe in the face of spelling differences, because there is no way
    // to spell an i/e in a strange way that is another letter.  Skipping this
    // allows us to avoid looking up the identifier info for #define/#undef and
    // other common directives.
    StringRef RI = Tok.getRawIdentifier();

    char FirstChar = RI[0];
    if (FirstChar >= 'a' && FirstChar <= 'z' &&
        FirstChar != 'i' && FirstChar != 'e') {
      CurPPLexer->ParsingPreprocessorDirective = false;
      // Restore comment saving mode.
      if (CurLexer) CurLexer->resetExtendedTokenMode();
      continue;
    }

    // Get the identifier name without trigraphs or embedded newlines.  Note
    // that we can't use Tok.getIdentifierInfo() because its lookup is disabled
    // when skipping.
    char DirectiveBuf[20];
    StringRef Directive;
    if (!Tok.needsCleaning() && RI.size() < 20) {
      Directive = RI;
    } else {
      std::string DirectiveStr = getSpelling(Tok);
      unsigned IdLen = DirectiveStr.size();
      if (IdLen >= 20) {
        CurPPLexer->ParsingPreprocessorDirective = false;
        // Restore comment saving mode.
        if (CurLexer) CurLexer->resetExtendedTokenMode();
        continue;
      }
      memcpy(DirectiveBuf, &DirectiveStr[0], IdLen);
      Directive = StringRef(DirectiveBuf, IdLen);
    }

    if (Directive.startswith("if")) {
      StringRef Sub = Directive.substr(2);
      if (Sub.empty() ||   // "if"
          Sub == "def" ||   // "ifdef"
          Sub == "ndef") {  // "ifndef"
        // We know the entire #if/#ifdef/#ifndef block will be skipped, don't
        // bother parsing the condition.
        DiscardUntilEndOfDirective();
        CurPPLexer->pushConditionalLevel(Tok.getLocation(), /*wasskipping*/true,
                                       /*foundnonskip*/false,
                                       /*foundelse*/false);
      }
    } else if (Directive[0] == 'e') {
      StringRef Sub = Directive.substr(1);
      if (Sub == "ndif") {  // "endif"
        PPConditionalInfo CondInfo;
        CondInfo.WasSkipping = true; // Silence bogus warning.
        bool InCond = CurPPLexer->popConditionalLevel(CondInfo);
        (void)InCond;  // Silence warning in no-asserts mode.
        assert(!InCond && "Can't be skipping if not in a conditional!");

        // If we popped the outermost skipping block, we're done skipping!
        if (!CondInfo.WasSkipping) {
          // Restore the value of LexingRawMode so that trailing comments
          // are handled correctly, if we've reached the outermost block.
          CurPPLexer->LexingRawMode = false;
          CheckEndOfDirective("endif");
          CurPPLexer->LexingRawMode = true;
          if (Callbacks)
            Callbacks->Endif(Tok.getLocation(), CondInfo.IfLoc);
          break;
        } else {
          DiscardUntilEndOfDirective();
        }
      } else if (Sub == "lse") { // "else".
        // #else directive in a skipping conditional.  If not in some other
        // skipping conditional, and if #else hasn't already been seen, enter it
        // as a non-skipping conditional.
        PPConditionalInfo &CondInfo = CurPPLexer->peekConditionalLevel();

        // If this is a #else with a #else before it, report the error.
        if (CondInfo.FoundElse) Diag(Tok, diag::pp_err_else_after_else);

        // Note that we've seen a #else in this conditional.
        CondInfo.FoundElse = true;

        // If the conditional is at the top level, and the #if block wasn't
        // entered, enter the #else block now.
        if (!CondInfo.WasSkipping && !CondInfo.FoundNonSkip) {
          CondInfo.FoundNonSkip = true;
          // Restore the value of LexingRawMode so that trailing comments
          // are handled correctly.
          CurPPLexer->LexingRawMode = false;
          CheckEndOfDirective("else");
          CurPPLexer->LexingRawMode = true;
          if (Callbacks)
            Callbacks->Else(Tok.getLocation(), CondInfo.IfLoc);
          break;
        } else {
          DiscardUntilEndOfDirective();  // C99 6.10p4.
        }
      } else if (Sub == "lif") {  // "elif".
        PPConditionalInfo &CondInfo = CurPPLexer->peekConditionalLevel();

        // If this is a #elif with a #else before it, report the error.
        if (CondInfo.FoundElse) Diag(Tok, diag::pp_err_elif_after_else);

        // If this is in a skipping block or if we're already handled this #if
        // block, don't bother parsing the condition.
        if (CondInfo.WasSkipping || CondInfo.FoundNonSkip) {
          DiscardUntilEndOfDirective();
        } else {
          const SourceLocation CondBegin = CurPPLexer->getSourceLocation();
          // Restore the value of LexingRawMode so that identifiers are
          // looked up, etc, inside the #elif expression.
          assert(CurPPLexer->LexingRawMode && "We have to be skipping here!");
          CurPPLexer->LexingRawMode = false;
          IdentifierInfo *IfNDefMacro = nullptr;
          const bool CondValue = EvaluateDirectiveExpression(IfNDefMacro);
          CurPPLexer->LexingRawMode = true;
          if (Callbacks) {
            const SourceLocation CondEnd = CurPPLexer->getSourceLocation();
            Callbacks->Elif(Tok.getLocation(),
                            SourceRange(CondBegin, CondEnd),
                            (CondValue ? PPCallbacks::CVK_True : PPCallbacks::CVK_False), CondInfo.IfLoc);
          }
          // If this condition is true, enter it!
          if (CondValue) {
            CondInfo.FoundNonSkip = true;
            break;
          }
        }
      }
    }

    CurPPLexer->ParsingPreprocessorDirective = false;
    // Restore comment saving mode.
    if (CurLexer) CurLexer->resetExtendedTokenMode();
  }

  // Finally, if we are out of the conditional (saw an #endif or ran off the end
  // of the file, just stop skipping and return to lexing whatever came after
  // the #if block.
  CurPPLexer->LexingRawMode = false;

  if (Callbacks) {
    SourceLocation BeginLoc = ElseLoc.isValid() ? ElseLoc : IfTokenLoc;
    Callbacks->SourceRangeSkipped(SourceRange(BeginLoc, Tok.getLocation()));
  }
}

void Preprocessor::PTHSkipExcludedConditionalBlock() {

  while (1) {
    assert(CurPTHLexer);
    assert(CurPTHLexer->LexingRawMode == false);

    // Skip to the next '#else', '#elif', or #endif.
    if (CurPTHLexer->SkipBlock()) {
      // We have reached an #endif.  Both the '#' and 'endif' tokens
      // have been consumed by the PTHLexer.  Just pop off the condition level.
      PPConditionalInfo CondInfo;
      bool InCond = CurPTHLexer->popConditionalLevel(CondInfo);
      (void)InCond;  // Silence warning in no-asserts mode.
      assert(!InCond && "Can't be skipping if not in a conditional!");
      break;
    }

    // We have reached a '#else' or '#elif'.  Lex the next token to get
    // the directive flavor.
    Token Tok;
    LexUnexpandedToken(Tok);

    // We can actually look up the IdentifierInfo here since we aren't in
    // raw mode.
    tok::PPKeywordKind K = Tok.getIdentifierInfo()->getPPKeywordID();

    if (K == tok::pp_else) {
      // #else: Enter the else condition.  We aren't in a nested condition
      //  since we skip those. We're always in the one matching the last
      //  blocked we skipped.
      PPConditionalInfo &CondInfo = CurPTHLexer->peekConditionalLevel();
      // Note that we've seen a #else in this conditional.
      CondInfo.FoundElse = true;

      // If the #if block wasn't entered then enter the #else block now.
      if (!CondInfo.FoundNonSkip) {
        CondInfo.FoundNonSkip = true;

        // Scan until the eod token.
        CurPTHLexer->ParsingPreprocessorDirective = true;
        DiscardUntilEndOfDirective();
        CurPTHLexer->ParsingPreprocessorDirective = false;

        break;
      }

      // Otherwise skip this block.
      continue;
    }

    assert(K == tok::pp_elif);
    PPConditionalInfo &CondInfo = CurPTHLexer->peekConditionalLevel();

    // If this is a #elif with a #else before it, report the error.
    if (CondInfo.FoundElse)
      Diag(Tok, diag::pp_err_elif_after_else);

    // If this is in a skipping block or if we're already handled this #if
    // block, don't bother parsing the condition.  We just skip this block.
    if (CondInfo.FoundNonSkip)
      continue;

    // Evaluate the condition of the #elif.
    IdentifierInfo *IfNDefMacro = nullptr;
    CurPTHLexer->ParsingPreprocessorDirective = true;
    bool ShouldEnter = EvaluateDirectiveExpression(IfNDefMacro);
    CurPTHLexer->ParsingPreprocessorDirective = false;

    // If this condition is true, enter it!
    if (ShouldEnter) {
      CondInfo.FoundNonSkip = true;
      break;
    }

    // Otherwise, skip this block and go to the next one.
    continue;
  }
}

Module *Preprocessor::getModuleForLocation(SourceLocation Loc) {
  ModuleMap &ModMap = HeaderInfo.getModuleMap();
  if (SourceMgr.isInMainFile(Loc)) {
    if (Module *CurMod = getCurrentModule())
      return CurMod;                               // Compiling a module.
    return HeaderInfo.getModuleMap().SourceModule; // Compiling a source.
  }
  // Try to determine the module of the include directive.
  // FIXME: Look into directly passing the FileEntry from LookupFile instead.
  FileID IDOfIncl = SourceMgr.getFileID(SourceMgr.getExpansionLoc(Loc));
  if (const FileEntry *EntryOfIncl = SourceMgr.getFileEntryForID(IDOfIncl)) {
    // The include comes from a file.
    return ModMap.findModuleForHeader(EntryOfIncl).getModule();
  } else {
    // The include does not come from a file,
    // so it is probably a module compilation.
    return getCurrentModule();
  }
}

Module *Preprocessor::getModuleContainingLocation(SourceLocation Loc) {
  return HeaderInfo.getModuleMap().inferModuleFromLocation(
      FullSourceLoc(Loc, SourceMgr));
}

const FileEntry *Preprocessor::LookupFile(
    SourceLocation FilenameLoc,
    StringRef Filename,
    bool isAngled,
    const DirectoryLookup *FromDir,
    const FileEntry *FromFile,
    const DirectoryLookup *&CurDir,
    SmallVectorImpl<char> *SearchPath,
    SmallVectorImpl<char> *RelativePath,
    ModuleMap::KnownHeader *SuggestedModule,
    bool SkipCache) {
  // If the header lookup mechanism may be relative to the current inclusion
  // stack, record the parent #includes.
  SmallVector<std::pair<const FileEntry *, const DirectoryEntry *>, 16>
      Includers;
  if (!FromDir && !FromFile) {
    FileID FID = getCurrentFileLexer()->getFileID();
    const FileEntry *FileEnt = SourceMgr.getFileEntryForID(FID);

    // If there is no file entry associated with this file, it must be the
    // predefines buffer or the module includes buffer. Any other file is not
    // lexed with a normal lexer, so it won't be scanned for preprocessor
    // directives.
    //
    // If we have the predefines buffer, resolve #include references (which come
    // from the -include command line argument) from the current working
    // directory instead of relative to the main file.
    //
    // If we have the module includes buffer, resolve #include references (which
    // come from header declarations in the module map) relative to the module
    // map file.
    if (!FileEnt) {
      if (FID == SourceMgr.getMainFileID() && MainFileDir)
        Includers.push_back(std::make_pair(nullptr, MainFileDir));
      else if ((FileEnt =
                    SourceMgr.getFileEntryForID(SourceMgr.getMainFileID())))
        Includers.push_back(std::make_pair(FileEnt, FileMgr.getDirectory(".")));
    } else {
      Includers.push_back(std::make_pair(FileEnt, FileEnt->getDir()));
    }

    // MSVC searches the current include stack from top to bottom for
    // headers included by quoted include directives.
    // See: http://msdn.microsoft.com/en-us/library/36k2cdd4.aspx
    if ((LangOpts.MSVCCompat || LangOpts.HLSL) && !isAngled) { // HLSL Change - use MSVC compat behavior
      for (unsigned i = 0, e = IncludeMacroStack.size(); i != e; ++i) {
        IncludeStackInfo &ISEntry = IncludeMacroStack[e - i - 1];
        if (IsFileLexer(ISEntry))
          if ((FileEnt = SourceMgr.getFileEntryForID(
                   ISEntry.ThePPLexer->getFileID())))
            Includers.push_back(std::make_pair(FileEnt, FileEnt->getDir()));
      }
    }
  }

  CurDir = CurDirLookup;

  if (FromFile) {
    // We're supposed to start looking from after a particular file. Search
    // the include path until we find that file or run out of files.
    const DirectoryLookup *TmpCurDir = CurDir;
    const DirectoryLookup *TmpFromDir = nullptr;
    while (const FileEntry *FE = HeaderInfo.LookupFile(
               Filename, FilenameLoc, isAngled, TmpFromDir, TmpCurDir,
               Includers, SearchPath, RelativePath, SuggestedModule,
               SkipCache)) {
      // Keep looking as if this file did a #include_next.
      TmpFromDir = TmpCurDir;
      ++TmpFromDir;
      if (FE == FromFile) {
        // Found it.
        FromDir = TmpFromDir;
        CurDir = TmpCurDir;
        break;
      }
    }
  }

  // Do a standard file entry lookup.
  const FileEntry *FE = HeaderInfo.LookupFile(
      Filename, FilenameLoc, isAngled, FromDir, CurDir, Includers, SearchPath,
      RelativePath, SuggestedModule, SkipCache);
  if (FE) {
    if (SuggestedModule && !LangOpts.AsmPreprocessor)
      HeaderInfo.getModuleMap().diagnoseHeaderInclusion(
          getModuleForLocation(FilenameLoc), FilenameLoc, Filename, FE);
    return FE;
  }

  const FileEntry *CurFileEnt;
  // Otherwise, see if this is a subframework header.  If so, this is relative
  // to one of the headers on the #include stack.  Walk the list of the current
  // headers on the #include stack and pass them to HeaderInfo.
  if (IsFileLexer()) {
    if ((CurFileEnt = SourceMgr.getFileEntryForID(CurPPLexer->getFileID()))) {
      if ((FE = HeaderInfo.LookupSubframeworkHeader(Filename, CurFileEnt,
                                                    SearchPath, RelativePath,
                                                    SuggestedModule))) {
        if (SuggestedModule && !LangOpts.AsmPreprocessor)
          HeaderInfo.getModuleMap().diagnoseHeaderInclusion(
              getModuleForLocation(FilenameLoc), FilenameLoc, Filename, FE);
        return FE;
      }
    }
  }

  for (unsigned i = 0, e = IncludeMacroStack.size(); i != e; ++i) {
    IncludeStackInfo &ISEntry = IncludeMacroStack[e-i-1];
    if (IsFileLexer(ISEntry)) {
      if ((CurFileEnt =
           SourceMgr.getFileEntryForID(ISEntry.ThePPLexer->getFileID()))) {
        if ((FE = HeaderInfo.LookupSubframeworkHeader(
                Filename, CurFileEnt, SearchPath, RelativePath,
                SuggestedModule))) {
          if (SuggestedModule && !LangOpts.AsmPreprocessor)
            HeaderInfo.getModuleMap().diagnoseHeaderInclusion(
                getModuleForLocation(FilenameLoc), FilenameLoc, Filename, FE);
          return FE;
        }
      }
    }
  }

  // Otherwise, we really couldn't find the file.
  return nullptr;
}


//===----------------------------------------------------------------------===//
// Preprocessor Directive Handling.
//===----------------------------------------------------------------------===//

class Preprocessor::ResetMacroExpansionHelper {
public:
  ResetMacroExpansionHelper(Preprocessor *pp)
    : PP(pp), save(pp->DisableMacroExpansion) {
    if (pp->MacroExpansionInDirectivesOverride)
      pp->DisableMacroExpansion = false;
  }
  ~ResetMacroExpansionHelper() {
    PP->DisableMacroExpansion = save;
  }
private:
  Preprocessor *PP;
  bool save;
};

/// HandleDirective - This callback is invoked when the lexer sees a # token
/// at the start of a line.  This consumes the directive, modifies the
/// lexer/preprocessor state, and advances the lexer(s) so that the next token
/// read is the correct one.
void Preprocessor::HandleDirective(Token &Result) {
  // FIXME: Traditional: # with whitespace before it not recognized by K&R?

  // We just parsed a # character at the start of a line, so we're in directive
  // mode.  Tell the lexer this so any newlines we see will be converted into an
  // EOD token (which terminates the directive).
  CurPPLexer->ParsingPreprocessorDirective = true;
  if (CurLexer) CurLexer->SetKeepWhitespaceMode(false);

  bool ImmediatelyAfterTopLevelIfndef =
      CurPPLexer->MIOpt.getImmediatelyAfterTopLevelIfndef();
  CurPPLexer->MIOpt.resetImmediatelyAfterTopLevelIfndef();

  ++NumDirectives;

  // We are about to read a token.  For the multiple-include optimization FA to
  // work, we have to remember if we had read any tokens *before* this
  // pp-directive.
  bool ReadAnyTokensBeforeDirective =CurPPLexer->MIOpt.getHasReadAnyTokensVal();

  // Save the '#' token in case we need to return it later.
  Token SavedHash = Result;

  // Read the next token, the directive flavor.  This isn't expanded due to
  // C99 6.10.3p8.
  LexUnexpandedToken(Result);

  // C99 6.10.3p11: Is this preprocessor directive in macro invocation?  e.g.:
  //   #define A(x) #x
  //   A(abc
  //     #warning blah
  //   def)
  // If so, the user is relying on undefined behavior, emit a diagnostic. Do
  // not support this for #include-like directives, since that can result in
  // terrible diagnostics, and does not work in GCC.
  if (InMacroArgs) {
    if (IdentifierInfo *II = Result.getIdentifierInfo()) {
      switch (II->getPPKeywordID()) {
      case tok::pp_include:
      case tok::pp_import:
      case tok::pp_include_next:
      case tok::pp___include_macros:
      case tok::pp_pragma:
        Diag(Result, diag::err_embedded_directive) << II->getName();
        DiscardUntilEndOfDirective();
        return;
      default:
        break;
      }
    }
    Diag(Result, diag::ext_embedded_directive);
  }

  // Temporarily enable macro expansion if set so
  // and reset to previous state when returning from this function.
  ResetMacroExpansionHelper helper(this);

  switch (Result.getKind()) {
  case tok::eod:
    return;   // null directive.
  case tok::code_completion:
    if (CodeComplete)
      CodeComplete->CodeCompleteDirective(
                                    CurPPLexer->getConditionalStackDepth() > 0);
    setCodeCompletionReached();
    return;
  case tok::numeric_constant:  // # 7  GNU line marker directive.
    if (getLangOpts().AsmPreprocessor)
      break;  // # 4 is not a preprocessor directive in .S files.
    return HandleDigitDirective(Result);
  default:
    IdentifierInfo *II = Result.getIdentifierInfo();
    if (!II) break; // Not an identifier.

    // Ask what the preprocessor keyword ID is.
    switch (II->getPPKeywordID()) {
    default: break;
    // C99 6.10.1 - Conditional Inclusion.
    case tok::pp_if:
      return HandleIfDirective(Result, ReadAnyTokensBeforeDirective);
    case tok::pp_ifdef:
      return HandleIfdefDirective(Result, false, true/*not valid for miopt*/);
    case tok::pp_ifndef:
      return HandleIfdefDirective(Result, true, ReadAnyTokensBeforeDirective);
    case tok::pp_elif:
      return HandleElifDirective(Result);
    case tok::pp_else:
      return HandleElseDirective(Result);
    case tok::pp_endif:
      return HandleEndifDirective(Result);

    // C99 6.10.2 - Source File Inclusion.
    case tok::pp_include:
      // Handle #include.
      return HandleIncludeDirective(SavedHash.getLocation(), Result);
    case tok::pp___include_macros:
      // Handle -imacros.
      return HandleIncludeMacrosDirective(SavedHash.getLocation(), Result); 

    // C99 6.10.3 - Macro Replacement.
    case tok::pp_define:
      return HandleDefineDirective(Result, ImmediatelyAfterTopLevelIfndef);
    case tok::pp_undef:
      return HandleUndefDirective(Result);

    // C99 6.10.4 - Line Control.
    case tok::pp_line:
      return HandleLineDirective(Result);

    // C99 6.10.5 - Error Directive.
    case tok::pp_error:
      return HandleUserDiagnosticDirective(Result, false);

    // C99 6.10.6 - Pragma Directive.
    case tok::pp_pragma:
      return HandlePragmaDirective(SavedHash.getLocation(), PIK_HashPragma);

    // GNU Extensions.
    case tok::pp_import:
      if (getLangOpts().HLSL) break; // HLSL Change - disable #import
      return HandleImportDirective(SavedHash.getLocation(), Result);
    case tok::pp_include_next:
      if (getLangOpts().HLSL) break; // HLSL Change - disable #include_next
      return HandleIncludeNextDirective(SavedHash.getLocation(), Result);

    case tok::pp_warning:
      Diag(Result, diag::ext_pp_warning_directive);
      return HandleUserDiagnosticDirective(Result, true);
    case tok::pp_ident:
      if (getLangOpts().HLSL) break; // HLSL Change - disable #ident
      return HandleIdentSCCSDirective(Result);
    case tok::pp_sccs:
      if (getLangOpts().HLSL) break; // HLSL Change - disable #sccs
      return HandleIdentSCCSDirective(Result);
    case tok::pp_assert:
      //isExtension = true;  // FIXME: implement #assert
      break;
    case tok::pp_unassert:
      //isExtension = true;  // FIXME: implement #unassert
      break;
        
    case tok::pp___public_macro:
      if (getLangOpts().Modules)
        return HandleMacroPublicDirective(Result);
      break;
        
    case tok::pp___private_macro:
      if (getLangOpts().Modules)
        return HandleMacroPrivateDirective(Result);
      break;
    }
    break;
  }

  // If this is a .S file, treat unknown # directives as non-preprocessor
  // directives.  This is important because # may be a comment or introduce
  // various pseudo-ops.  Just return the # token and push back the following
  // token to be lexed next time.
  if (getLangOpts().AsmPreprocessor) {
    Token *Toks = new Token[2];
    // Return the # and the token after it.
    Toks[0] = SavedHash;
    Toks[1] = Result;
    
    // If the second token is a hashhash token, then we need to translate it to
    // unknown so the token lexer doesn't try to perform token pasting.
    if (Result.is(tok::hashhash))
      Toks[1].setKind(tok::unknown);
    
    // Enter this token stream so that we re-lex the tokens.  Make sure to
    // enable macro expansion, in case the token after the # is an identifier
    // that is expanded.
    EnterTokenStream(Toks, 2, false, true);
    return;
  }

  // If we reached here, the preprocessing token is not valid!
  Diag(Result, diag::err_pp_invalid_directive);

  // Read the rest of the PP line.
  DiscardUntilEndOfDirective();

  // Okay, we're done parsing the directive.
}

/// GetLineValue - Convert a numeric token into an unsigned value, emitting
/// Diagnostic DiagID if it is invalid, and returning the value in Val.
static bool GetLineValue(Token &DigitTok, unsigned &Val,
                         unsigned DiagID, Preprocessor &PP,
                         bool IsGNULineDirective=false) {
  if (DigitTok.isNot(tok::numeric_constant)) {
    PP.Diag(DigitTok, DiagID);

    if (DigitTok.isNot(tok::eod))
      PP.DiscardUntilEndOfDirective();
    return true;
  }

  SmallString<64> IntegerBuffer;
  IntegerBuffer.resize(DigitTok.getLength());
  const char *DigitTokBegin = &IntegerBuffer[0];
  bool Invalid = false;
  unsigned ActualLength = PP.getSpelling(DigitTok, DigitTokBegin, &Invalid);
  if (Invalid)
    return true;
  
  // Verify that we have a simple digit-sequence, and compute the value.  This
  // is always a simple digit string computed in decimal, so we do this manually
  // here.
  Val = 0;
  for (unsigned i = 0; i != ActualLength; ++i) {
    // C++1y [lex.fcon]p1:
    //   Optional separating single quotes in a digit-sequence are ignored
    if (DigitTokBegin[i] == '\'')
      continue;

    if (!isDigit(DigitTokBegin[i])) {
      PP.Diag(PP.AdvanceToTokenCharacter(DigitTok.getLocation(), i),
              diag::err_pp_line_digit_sequence) << IsGNULineDirective;
      PP.DiscardUntilEndOfDirective();
      return true;
    }

    unsigned NextVal = Val*10+(DigitTokBegin[i]-'0');
    if (NextVal < Val) { // overflow.
      PP.Diag(DigitTok, DiagID);
      PP.DiscardUntilEndOfDirective();
      return true;
    }
    Val = NextVal;
  }

  if (DigitTokBegin[0] == '0' && Val)
    PP.Diag(DigitTok.getLocation(), diag::warn_pp_line_decimal)
      << IsGNULineDirective;

  return false;
}

/// \brief Handle a \#line directive: C99 6.10.4.
///
/// The two acceptable forms are:
/// \verbatim
///   # line digit-sequence
///   # line digit-sequence "s-char-sequence"
/// \endverbatim
void Preprocessor::HandleLineDirective(Token &Tok) {
  // Read the line # and string argument.  Per C99 6.10.4p5, these tokens are
  // expanded.
  Token DigitTok;
  Lex(DigitTok);

  // Validate the number and convert it to an unsigned.
  unsigned LineNo;
  if (GetLineValue(DigitTok, LineNo, diag::err_pp_line_requires_integer,*this))
    return;
  
  if (LineNo == 0)
    Diag(DigitTok, diag::ext_pp_line_zero);

  // Enforce C99 6.10.4p3: "The digit sequence shall not specify ... a
  // number greater than 2147483647".  C90 requires that the line # be <= 32767.
  unsigned LineLimit = 32768U;
  if (LangOpts.C99 || LangOpts.CPlusPlus11)
    LineLimit = 2147483648U;
  if (LineNo >= LineLimit)
    Diag(DigitTok, diag::ext_pp_line_too_big) << LineLimit;
  else if (LangOpts.CPlusPlus11 && LineNo >= 32768U)
    Diag(DigitTok, diag::warn_cxx98_compat_pp_line_too_big);

  int FilenameID = -1;
  Token StrTok;
  Lex(StrTok);

  // If the StrTok is "eod", then it wasn't present.  Otherwise, it must be a
  // string followed by eod.
  if (StrTok.is(tok::eod))
    ; // ok
  else if (StrTok.isNot(tok::string_literal)) {
    Diag(StrTok, diag::err_pp_line_invalid_filename);
    return DiscardUntilEndOfDirective();
  } else if (StrTok.hasUDSuffix()) {
    Diag(StrTok, diag::err_invalid_string_udl);
    return DiscardUntilEndOfDirective();
  } else {
    // Parse and validate the string, converting it into a unique ID.
    StringLiteralParser Literal(StrTok, *this);
    assert(Literal.isAscii() && "Didn't allow wide strings in");
    if (Literal.hadError)
      return DiscardUntilEndOfDirective();
    if (Literal.Pascal) {
      Diag(StrTok, diag::err_pp_linemarker_invalid_filename);
      return DiscardUntilEndOfDirective();
    }
    FilenameID = SourceMgr.getLineTableFilenameID(Literal.GetString());

    // Verify that there is nothing after the string, other than EOD.  Because
    // of C99 6.10.4p5, macros that expand to empty tokens are ok.
    CheckEndOfDirective("line", true);
  }

  // HLSL Change Begin - ignore line directives.
  if (PPOpts->IgnoreLineDirectives)
    return;
  // HLSL Change End

  SourceMgr.AddLineNote(DigitTok.getLocation(), LineNo, FilenameID);

  if (Callbacks)
    Callbacks->FileChanged(CurPPLexer->getSourceLocation(),
                           PPCallbacks::RenameFile,
                           SrcMgr::C_User);
}

/// ReadLineMarkerFlags - Parse and validate any flags at the end of a GNU line
/// marker directive.
static bool ReadLineMarkerFlags(bool &IsFileEntry, bool &IsFileExit,
                                bool &IsSystemHeader, bool &IsExternCHeader,
                                Preprocessor &PP) {
  unsigned FlagVal;
  Token FlagTok;
  PP.Lex(FlagTok);
  if (FlagTok.is(tok::eod)) return false;
  if (GetLineValue(FlagTok, FlagVal, diag::err_pp_linemarker_invalid_flag, PP))
    return true;

  if (FlagVal == 1) {
    IsFileEntry = true;

    PP.Lex(FlagTok);
    if (FlagTok.is(tok::eod)) return false;
    if (GetLineValue(FlagTok, FlagVal, diag::err_pp_linemarker_invalid_flag,PP))
      return true;
  } else if (FlagVal == 2) {
    IsFileExit = true;

    SourceManager &SM = PP.getSourceManager();
    // If we are leaving the current presumed file, check to make sure the
    // presumed include stack isn't empty!
    FileID CurFileID =
      SM.getDecomposedExpansionLoc(FlagTok.getLocation()).first;
    PresumedLoc PLoc = SM.getPresumedLoc(FlagTok.getLocation());
    if (PLoc.isInvalid())
      return true;
    
    // If there is no include loc (main file) or if the include loc is in a
    // different physical file, then we aren't in a "1" line marker flag region.
    SourceLocation IncLoc = PLoc.getIncludeLoc();
    if (IncLoc.isInvalid() ||
        SM.getDecomposedExpansionLoc(IncLoc).first != CurFileID) {
      PP.Diag(FlagTok, diag::err_pp_linemarker_invalid_pop);
      PP.DiscardUntilEndOfDirective();
      return true;
    }

    PP.Lex(FlagTok);
    if (FlagTok.is(tok::eod)) return false;
    if (GetLineValue(FlagTok, FlagVal, diag::err_pp_linemarker_invalid_flag,PP))
      return true;
  }

  // We must have 3 if there are still flags.
  if (FlagVal != 3) {
    PP.Diag(FlagTok, diag::err_pp_linemarker_invalid_flag);
    PP.DiscardUntilEndOfDirective();
    return true;
  }

  IsSystemHeader = true;

  PP.Lex(FlagTok);
  if (FlagTok.is(tok::eod)) return false;
  if (GetLineValue(FlagTok, FlagVal, diag::err_pp_linemarker_invalid_flag, PP))
    return true;

  // We must have 4 if there is yet another flag.
  if (FlagVal != 4) {
    PP.Diag(FlagTok, diag::err_pp_linemarker_invalid_flag);
    PP.DiscardUntilEndOfDirective();
    return true;
  }

  IsExternCHeader = true;

  PP.Lex(FlagTok);
  if (FlagTok.is(tok::eod)) return false;

  // There are no more valid flags here.
  PP.Diag(FlagTok, diag::err_pp_linemarker_invalid_flag);
  PP.DiscardUntilEndOfDirective();
  return true;
}

/// HandleDigitDirective - Handle a GNU line marker directive, whose syntax is
/// one of the following forms:
///
///     # 42
///     # 42 "file" ('1' | '2')?
///     # 42 "file" ('1' | '2')? '3' '4'?
///
void Preprocessor::HandleDigitDirective(Token &DigitTok) {
  // Validate the number and convert it to an unsigned.  GNU does not have a
  // line # limit other than it fit in 32-bits.
  unsigned LineNo;
  if (GetLineValue(DigitTok, LineNo, diag::err_pp_linemarker_requires_integer,
                   *this, true))
    return;

  Token StrTok;
  Lex(StrTok);

  bool IsFileEntry = false, IsFileExit = false;
  bool IsSystemHeader = false, IsExternCHeader = false;
  int FilenameID = -1;

  // If the StrTok is "eod", then it wasn't present.  Otherwise, it must be a
  // string followed by eod.
  if (StrTok.is(tok::eod))
    ; // ok
  else if (StrTok.isNot(tok::string_literal)) {
    Diag(StrTok, diag::err_pp_linemarker_invalid_filename);
    return DiscardUntilEndOfDirective();
  } else if (StrTok.hasUDSuffix()) {
    Diag(StrTok, diag::err_invalid_string_udl);
    return DiscardUntilEndOfDirective();
  } else {
    // Parse and validate the string, converting it into a unique ID.
    StringLiteralParser Literal(StrTok, *this);
    assert(Literal.isAscii() && "Didn't allow wide strings in");
    if (Literal.hadError)
      return DiscardUntilEndOfDirective();
    if (Literal.Pascal) {
      Diag(StrTok, diag::err_pp_linemarker_invalid_filename);
      return DiscardUntilEndOfDirective();
    }
    FilenameID = SourceMgr.getLineTableFilenameID(Literal.GetString());

    // If a filename was present, read any flags that are present.
    if (ReadLineMarkerFlags(IsFileEntry, IsFileExit,
                            IsSystemHeader, IsExternCHeader, *this))
      return;
  }

  // Create a line note with this information.
  SourceMgr.AddLineNote(DigitTok.getLocation(), LineNo, FilenameID,
                        IsFileEntry, IsFileExit,
                        IsSystemHeader, IsExternCHeader);

  // If the preprocessor has callbacks installed, notify them of the #line
  // change.  This is used so that the line marker comes out in -E mode for
  // example.
  if (Callbacks) {
    PPCallbacks::FileChangeReason Reason = PPCallbacks::RenameFile;
    if (IsFileEntry)
      Reason = PPCallbacks::EnterFile;
    else if (IsFileExit)
      Reason = PPCallbacks::ExitFile;
    SrcMgr::CharacteristicKind FileKind = SrcMgr::C_User;
    if (IsExternCHeader)
      FileKind = SrcMgr::C_ExternCSystem;
    else if (IsSystemHeader)
      FileKind = SrcMgr::C_System;

    Callbacks->FileChanged(CurPPLexer->getSourceLocation(), Reason, FileKind);
  }
}


/// HandleUserDiagnosticDirective - Handle a #warning or #error directive.
///
void Preprocessor::HandleUserDiagnosticDirective(Token &Tok,
                                                 bool isWarning) {
  // PTH doesn't emit #warning or #error directives.
  if (CurPTHLexer)
    return CurPTHLexer->DiscardToEndOfLine();

  // Read the rest of the line raw.  We do this because we don't want macros
  // to be expanded and we don't require that the tokens be valid preprocessing
  // tokens.  For example, this is allowed: "#warning `   'foo".  GCC does
  // collapse multiple consequtive white space between tokens, but this isn't
  // specified by the standard.
  SmallString<128> Message;
  CurLexer->ReadToEndOfLine(&Message);

  // Find the first non-whitespace character, so that we can make the
  // diagnostic more succinct.
  StringRef Msg = StringRef(Message).ltrim(" ");

  if (isWarning)
    Diag(Tok, diag::pp_hash_warning) << Msg;
  else
    Diag(Tok, diag::err_pp_hash_error) << Msg;
}

/// HandleIdentSCCSDirective - Handle a #ident/#sccs directive.
///
void Preprocessor::HandleIdentSCCSDirective(Token &Tok) {
  // Yes, this directive is an extension.
  Diag(Tok, diag::ext_pp_ident_directive);

  // Read the string argument.
  Token StrTok;
  Lex(StrTok);

  // If the token kind isn't a string, it's a malformed directive.
  if (StrTok.isNot(tok::string_literal) &&
      StrTok.isNot(tok::wide_string_literal)) {
    Diag(StrTok, diag::err_pp_malformed_ident);
    if (StrTok.isNot(tok::eod))
      DiscardUntilEndOfDirective();
    return;
  }

  if (StrTok.hasUDSuffix()) {
    Diag(StrTok, diag::err_invalid_string_udl);
    return DiscardUntilEndOfDirective();
  }

  // Verify that there is nothing after the string, other than EOD.
  CheckEndOfDirective("ident");

  if (Callbacks) {
    bool Invalid = false;
    std::string Str = getSpelling(StrTok, &Invalid);
    if (!Invalid)
      Callbacks->Ident(Tok.getLocation(), Str);
  }
}

/// \brief Handle a #public directive.
void Preprocessor::HandleMacroPublicDirective(Token &Tok) {
  Token MacroNameTok;
  ReadMacroName(MacroNameTok, MU_Undef);
  
  // Error reading macro name?  If so, diagnostic already issued.
  if (MacroNameTok.is(tok::eod))
    return;

  // Check to see if this is the last token on the #__public_macro line.
  CheckEndOfDirective("__public_macro");

  IdentifierInfo *II = MacroNameTok.getIdentifierInfo();
  // Okay, we finally have a valid identifier to undef.
  MacroDirective *MD = getLocalMacroDirective(II);
  
  // If the macro is not defined, this is an error.
  if (!MD) {
    Diag(MacroNameTok, diag::err_pp_visibility_non_macro) << II;
    return;
  }
  
  // Note that this macro has now been exported.
  appendMacroDirective(II, AllocateVisibilityMacroDirective(
                                MacroNameTok.getLocation(), /*IsPublic=*/true));
}

/// \brief Handle a #private directive.
void Preprocessor::HandleMacroPrivateDirective(Token &Tok) {
  Token MacroNameTok;
  ReadMacroName(MacroNameTok, MU_Undef);
  
  // Error reading macro name?  If so, diagnostic already issued.
  if (MacroNameTok.is(tok::eod))
    return;
  
  // Check to see if this is the last token on the #__private_macro line.
  CheckEndOfDirective("__private_macro");
  
  IdentifierInfo *II = MacroNameTok.getIdentifierInfo();
  // Okay, we finally have a valid identifier to undef.
  MacroDirective *MD = getLocalMacroDirective(II);
  
  // If the macro is not defined, this is an error.
  if (!MD) {
    Diag(MacroNameTok, diag::err_pp_visibility_non_macro) << II;
    return;
  }
  
  // Note that this macro has now been marked private.
  appendMacroDirective(II, AllocateVisibilityMacroDirective(
                               MacroNameTok.getLocation(), /*IsPublic=*/false));
}

//===----------------------------------------------------------------------===//
// Preprocessor Include Directive Handling.
//===----------------------------------------------------------------------===//

/// GetIncludeFilenameSpelling - Turn the specified lexer token into a fully
/// checked and spelled filename, e.g. as an operand of \#include. This returns
/// true if the input filename was in <>'s or false if it were in ""'s.  The
/// caller is expected to provide a buffer that is large enough to hold the
/// spelling of the filename, but is also expected to handle the case when
/// this method decides to use a different buffer.
bool Preprocessor::GetIncludeFilenameSpelling(SourceLocation Loc,
                                              StringRef &Buffer) {
  // Get the text form of the filename.
  assert(!Buffer.empty() && "Can't have tokens with empty spellings!");

  // Make sure the filename is <x> or "x".
  bool isAngled;
  if (Buffer[0] == '<') {
    if (Buffer.back() != '>') {
      Diag(Loc, diag::err_pp_expects_filename);
      Buffer = StringRef();
      return true;
    }
    isAngled = true;
  } else if (Buffer[0] == '"') {
    if (Buffer.back() != '"') {
      Diag(Loc, diag::err_pp_expects_filename);
      Buffer = StringRef();
      return true;
    }
    isAngled = false;
  } else {
    Diag(Loc, diag::err_pp_expects_filename);
    Buffer = StringRef();
    return true;
  }

  // Diagnose #include "" as invalid.
  if (Buffer.size() <= 2) {
    Diag(Loc, diag::err_pp_empty_filename);
    Buffer = StringRef();
    return true;
  }

  // Skip the brackets.
  Buffer = Buffer.substr(1, Buffer.size()-2);
  return isAngled;
}

// \brief Handle cases where the \#include name is expanded from a macro
// as multiple tokens, which need to be glued together.
//
// This occurs for code like:
// \code
//    \#define FOO <a/b.h>
//    \#include FOO
// \endcode
// because in this case, "<a/b.h>" is returned as 7 tokens, not one.
//
// This code concatenates and consumes tokens up to the '>' token.  It returns
// false if the > was found, otherwise it returns true if it finds and consumes
// the EOD marker.
bool Preprocessor::ConcatenateIncludeName(SmallString<128> &FilenameBuffer,
                                          SourceLocation &End) {
  Token CurTok;

  Lex(CurTok);
  while (CurTok.isNot(tok::eod)) {
    End = CurTok.getLocation();
    
    // FIXME: Provide code completion for #includes.
    if (CurTok.is(tok::code_completion)) {
      setCodeCompletionReached();
      Lex(CurTok);
      continue;
    }

    // Append the spelling of this token to the buffer. If there was a space
    // before it, add it now.
    if (CurTok.hasLeadingSpace())
      FilenameBuffer.push_back(' ');

    // Get the spelling of the token, directly into FilenameBuffer if possible.
    unsigned PreAppendSize = FilenameBuffer.size();
    FilenameBuffer.resize(PreAppendSize+CurTok.getLength());

    const char *BufPtr = &FilenameBuffer[PreAppendSize];
    unsigned ActualLen = getSpelling(CurTok, BufPtr);

    // If the token was spelled somewhere else, copy it into FilenameBuffer.
    if (BufPtr != &FilenameBuffer[PreAppendSize])
      memcpy(&FilenameBuffer[PreAppendSize], BufPtr, ActualLen);

    // Resize FilenameBuffer to the correct size.
    if (CurTok.getLength() != ActualLen)
      FilenameBuffer.resize(PreAppendSize+ActualLen);

    // If we found the '>' marker, return success.
    if (CurTok.is(tok::greater))
      return false;

    Lex(CurTok);
  }

  // If we hit the eod marker, emit an error and return true so that the caller
  // knows the EOD has been read.
  Diag(CurTok.getLocation(), diag::err_pp_expects_filename);
  return true;
}

/// \brief Push a token onto the token stream containing an annotation.
static void EnterAnnotationToken(Preprocessor &PP,
                                 SourceLocation Begin, SourceLocation End,
                                 tok::TokenKind Kind, void *AnnotationVal) {
  // FIXME: Produce this as the current token directly, rather than
  // allocating a new token for it.
  Token *Tok = new Token[1];
  Tok[0].startToken();
  Tok[0].setKind(Kind);
  Tok[0].setLocation(Begin);
  Tok[0].setAnnotationEndLoc(End);
  Tok[0].setAnnotationValue(AnnotationVal);
  PP.EnterTokenStream(Tok, 1, true, true);
}

/// \brief Produce a diagnostic informing the user that a #include or similar
/// was implicitly treated as a module import.
static void diagnoseAutoModuleImport(
    Preprocessor &PP, SourceLocation HashLoc, Token &IncludeTok,
    ArrayRef<std::pair<IdentifierInfo *, SourceLocation>> Path,
    SourceLocation PathEnd) {
  assert(PP.getLangOpts().ObjC2 && "no import syntax available");

  SmallString<128> PathString;
  for (unsigned I = 0, N = Path.size(); I != N; ++I) {
    if (I)
      PathString += '.';
    PathString += Path[I].first->getName();
  }
  int IncludeKind = 0;
  
  switch (IncludeTok.getIdentifierInfo()->getPPKeywordID()) {
  case tok::pp_include:
    IncludeKind = 0;
    break;
    
  case tok::pp_import:
    IncludeKind = 1;
    break;        
      
  case tok::pp_include_next:
    IncludeKind = 2;
    break;
      
  case tok::pp___include_macros:
    IncludeKind = 3;
    break;
      
  default:
    llvm_unreachable("unknown include directive kind");
  }

  CharSourceRange ReplaceRange(SourceRange(HashLoc, PathEnd),
                               /*IsTokenRange=*/false);
  PP.Diag(HashLoc, diag::warn_auto_module_import)
      << IncludeKind << PathString
      << FixItHint::CreateReplacement(ReplaceRange,
                                      ("@import " + PathString + ";").str());
}

/// HandleIncludeDirective - The "\#include" tokens have just been read, read
/// the file to be included from the lexer, then include it!  This is a common
/// routine with functionality shared between \#include, \#include_next and
/// \#import.  LookupFrom is set when this is a \#include_next directive, it
/// specifies the file to start searching from.
void Preprocessor::HandleIncludeDirective(SourceLocation HashLoc, 
                                          Token &IncludeTok,
                                          const DirectoryLookup *LookupFrom,
                                          const FileEntry *LookupFromFile,
                                          bool isImport) {

  Token FilenameTok;
  CurPPLexer->LexIncludeFilename(FilenameTok);

  // Reserve a buffer to get the spelling.
  SmallString<128> FilenameBuffer;
  StringRef Filename;
  SourceLocation End;
  SourceLocation CharEnd; // the end of this directive, in characters
  
  switch (FilenameTok.getKind()) {
  case tok::eod:
    // If the token kind is EOD, the error has already been diagnosed.
    return;

  case tok::angle_string_literal:
  case tok::string_literal:
    Filename = getSpelling(FilenameTok, FilenameBuffer);
    End = FilenameTok.getLocation();
    CharEnd = End.getLocWithOffset(FilenameTok.getLength());
    break;

  case tok::less:
    // This could be a <foo/bar.h> file coming from a macro expansion.  In this
    // case, glue the tokens together into FilenameBuffer and interpret those.
    FilenameBuffer.push_back('<');
    if (ConcatenateIncludeName(FilenameBuffer, End))
      return;   // Found <eod> but no ">"?  Diagnostic already emitted.
    Filename = FilenameBuffer;
    CharEnd = End.getLocWithOffset(1);
    break;
  default:
    Diag(FilenameTok.getLocation(), diag::err_pp_expects_filename);
    DiscardUntilEndOfDirective();
    return;
  }

  CharSourceRange FilenameRange
    = CharSourceRange::getCharRange(FilenameTok.getLocation(), CharEnd);
  StringRef OriginalFilename = Filename;
  bool isAngled =
    GetIncludeFilenameSpelling(FilenameTok.getLocation(), Filename);
  // If GetIncludeFilenameSpelling set the start ptr to null, there was an
  // error.
  if (Filename.empty()) {
    DiscardUntilEndOfDirective();
    return;
  }

  // Verify that there is nothing after the filename, other than EOD.  Note that
  // we allow macros that expand to nothing after the filename, because this
  // falls into the category of "#include pp-tokens new-line" specified in
  // C99 6.10.2p4.
  CheckEndOfDirective(IncludeTok.getIdentifierInfo()->getNameStart(), true);

  // Check that we don't have infinite #include recursion.
  if (IncludeMacroStack.size() == MaxAllowedIncludeStackDepth-1) {
    Diag(FilenameTok, diag::err_pp_include_too_deep);
    return;
  }

  // Complain about attempts to #include files in an audit pragma.
  if (PragmaARCCFCodeAuditedLoc.isValid()) {
    Diag(HashLoc, diag::err_pp_include_in_arc_cf_code_audited);
    Diag(PragmaARCCFCodeAuditedLoc, diag::note_pragma_entered_here);

    // Immediately leave the pragma.
    PragmaARCCFCodeAuditedLoc = SourceLocation();
  }

  // Complain about attempts to #include files in an assume-nonnull pragma.
  if (PragmaAssumeNonNullLoc.isValid()) {
    Diag(HashLoc, diag::err_pp_include_in_assume_nonnull);
    Diag(PragmaAssumeNonNullLoc, diag::note_pragma_entered_here);

    // Immediately leave the pragma.
    PragmaAssumeNonNullLoc = SourceLocation();
  }

  if (HeaderInfo.HasIncludeAliasMap()) {
    // Map the filename with the brackets still attached.  If the name doesn't 
    // map to anything, fall back on the filename we've already gotten the 
    // spelling for.
    StringRef NewName = HeaderInfo.MapHeaderToIncludeAlias(OriginalFilename);
    if (!NewName.empty())
      Filename = NewName;
  }

  // Search include directories.
  const DirectoryLookup *CurDir;
  SmallString<1024> SearchPath;
  SmallString<1024> RelativePath;
  // We get the raw path only if we have 'Callbacks' to which we later pass
  // the path.
  ModuleMap::KnownHeader SuggestedModule;
  SourceLocation FilenameLoc = FilenameTok.getLocation();
  SmallString<128> NormalizedPath;
  if (LangOpts.MSVCCompat || LangOpts.HLSL) { // HLSL Change: use MSVC compat behavior
    NormalizedPath = Filename.str();
#ifndef LLVM_ON_WIN32
    llvm::sys::path::native(NormalizedPath);
#endif
  }
  const FileEntry *File = LookupFile(
      FilenameLoc, (LangOpts.MSVCCompat || LangOpts.HLSL) ? NormalizedPath.c_str() : Filename, // HLSL Change: use MSVC compat behavior
      isAngled, LookupFrom, LookupFromFile, CurDir,
      Callbacks ? &SearchPath : nullptr, Callbacks ? &RelativePath : nullptr,
      &SuggestedModule);

  if (!File) {
    if (Callbacks) {
      // Give the clients a chance to recover.
      SmallString<128> RecoveryPath;
      if (Callbacks->FileNotFound(Filename, RecoveryPath)) {
        if (const DirectoryEntry *DE = FileMgr.getDirectory(RecoveryPath)) {
          // Add the recovery path to the list of search paths.
          DirectoryLookup DL(DE, SrcMgr::C_User, false);
          HeaderInfo.AddSearchPath(DL, isAngled);
          
          // Try the lookup again, skipping the cache.
          File = LookupFile(
              FilenameLoc,
             (LangOpts.MSVCCompat || LangOpts.HLSL) ? NormalizedPath.c_str() : Filename, isAngled, // HLSL Change: use MSVC compat behavior
              LookupFrom, LookupFromFile, CurDir, nullptr, nullptr,
              &SuggestedModule, /*SkipCache*/ true);
        }
      }
    }

    if (!SuppressIncludeNotFoundError) {
      // If the file could not be located and it was included via angle 
      // brackets, we can attempt a lookup as though it were a quoted path to
      // provide the user with a possible fixit.
      if (isAngled) {
        File = LookupFile(
            FilenameLoc,
            (LangOpts.MSVCCompat || LangOpts.HLSL) ? NormalizedPath.c_str() : Filename, false, // HLSL Change: use MSVC compat behavior
            LookupFrom, LookupFromFile, CurDir,
            Callbacks ? &SearchPath : nullptr,
            Callbacks ? &RelativePath : nullptr,
            &SuggestedModule);
        if (File) {
          SourceRange Range(FilenameTok.getLocation(), CharEnd);
          Diag(FilenameTok, diag::err_pp_file_not_found_not_fatal) << 
            Filename << 
            FixItHint::CreateReplacement(Range, "\"" + Filename.str() + "\"");
        }
      }

      // If the file is still not found, just go with the vanilla diagnostic
      if (!File)
        Diag(FilenameTok, diag::err_pp_file_not_found) << Filename;
    }
  }

  // Should we enter the source file? Set to false if either the source file is
  // known to have no effect beyond its effect on module visibility -- that is,
  // if it's got an include guard that is already defined or is a modular header
  // we've imported or already built.
  bool ShouldEnter = true;

  // Determine whether we should try to import the module for this #include, if
  // there is one. Don't do so if precompiled module support is disabled or we
  // are processing this module textually (because we're building the module).
  if (File && SuggestedModule && getLangOpts().Modules &&
      SuggestedModule.getModule()->getTopLevelModuleName() !=
          getLangOpts().CurrentModule &&
      SuggestedModule.getModule()->getTopLevelModuleName() !=
          getLangOpts().ImplementationOfModule) {
    // Compute the module access path corresponding to this module.
    // FIXME: Should we have a second loadModule() overload to avoid this
    // extra lookup step?
    SmallVector<std::pair<IdentifierInfo *, SourceLocation>, 2> Path;
    for (Module *Mod = SuggestedModule.getModule(); Mod; Mod = Mod->Parent)
      Path.push_back(std::make_pair(getIdentifierInfo(Mod->Name),
                                    FilenameTok.getLocation()));
    std::reverse(Path.begin(), Path.end());

    // Warn that we're replacing the include/import with a module import.
    // We only do this in Objective-C, where we have a module-import syntax.
    if (getLangOpts().ObjC2)
      diagnoseAutoModuleImport(*this, HashLoc, IncludeTok, Path, CharEnd);
    
    // Load the module to import its macros. We'll make the declarations
    // visible when the parser gets here.
    // FIXME: Pass SuggestedModule in here rather than converting it to a path
    // and making the module loader convert it back again.
    ModuleLoadResult Imported = TheModuleLoader.loadModule(
        IncludeTok.getLocation(), Path, Module::Hidden,
        /*IsIncludeDirective=*/true);
    assert((Imported == nullptr || Imported == SuggestedModule.getModule()) &&
           "the imported module is different than the suggested one");

    if (Imported)
      ShouldEnter = false;
    else if (Imported.isMissingExpected()) {
      // We failed to find a submodule that we assumed would exist (because it
      // was in the directory of an umbrella header, for instance), but no
      // actual module exists for it (because the umbrella header is
      // incomplete).  Treat this as a textual inclusion.
      SuggestedModule = ModuleMap::KnownHeader();
    } else {
      // We hit an error processing the import. Bail out.
      if (hadModuleLoaderFatalFailure()) {
        // With a fatal failure in the module loader, we abort parsing.
        Token &Result = IncludeTok;
        if (CurLexer) {
          Result.startToken();
          CurLexer->FormTokenWithChars(Result, CurLexer->BufferEnd, tok::eof);
          CurLexer->cutOffLexing();
        } else {
          assert(CurPTHLexer && "#include but no current lexer set!");
          CurPTHLexer->getEOF(Result);
        }
      }
      return;
    }
  }

  if (Callbacks) {
    // Notify the callback object that we've seen an inclusion directive.
    Callbacks->InclusionDirective(
        HashLoc, IncludeTok,
        (LangOpts.MSVCCompat || LangOpts.HLSL) ? NormalizedPath.c_str() : Filename, isAngled, // HLSL Change: use MSVC compat behavior
        FilenameRange, File, SearchPath, RelativePath,
        ShouldEnter ? nullptr : SuggestedModule.getModule());
  }

  if (!File)
    return;
  
  // The #included file will be considered to be a system header if either it is
  // in a system include directory, or if the #includer is a system include
  // header.
  SrcMgr::CharacteristicKind FileCharacter =
    std::max(HeaderInfo.getFileDirFlavor(File),
             SourceMgr.getFileCharacteristic(FilenameTok.getLocation()));

  // FIXME: If we have a suggested module, and we've already visited this file,
  // don't bother entering it again. We know it has no further effect.

  // Ask HeaderInfo if we should enter this #include file.  If not, #including
  // this file will have no effect.
  if (ShouldEnter &&
      !HeaderInfo.ShouldEnterIncludeFile(*this, File, isImport,
                                         SuggestedModule.getModule())) {
    ShouldEnter = false;
    if (Callbacks)
      Callbacks->FileSkipped(*File, FilenameTok, FileCharacter);
  }

  // If we don't need to enter the file, stop now.
  if (!ShouldEnter) {
    // If this is a module import, make it visible if needed.
    if (auto *M = SuggestedModule.getModule()) {
      makeModuleVisible(M, HashLoc);

      if (IncludeTok.getIdentifierInfo()->getPPKeywordID() !=
          tok::pp___include_macros)
        EnterAnnotationToken(*this, HashLoc, End, tok::annot_module_include, M);
    }
    return;
  }

  // Look up the file, create a File ID for it.
  SourceLocation IncludePos = End;
  // If the filename string was the result of macro expansions, set the include
  // position on the file where it will be included and after the expansions.
  if (IncludePos.isMacroID())
    IncludePos = SourceMgr.getExpansionRange(IncludePos).second;
  FileID FID = SourceMgr.createFileID(File, IncludePos, FileCharacter);
  assert(!FID.isInvalid() && "Expected valid file ID");

  // If all is good, enter the new file!
  if (EnterSourceFile(FID, CurDir, FilenameTok.getLocation()))
    return;

  // Determine if we're switching to building a new submodule, and which one.
  if (auto *M = SuggestedModule.getModule()) {
    assert(!CurSubmodule && "should not have marked this as a module yet");
    CurSubmodule = M;

    // Let the macro handling code know that any future macros are within
    // the new submodule.
    EnterSubmodule(M, HashLoc);

    // Let the parser know that any future declarations are within the new
    // submodule.
    // FIXME: There's no point doing this if we're handling a #__include_macros
    // directive.
    EnterAnnotationToken(*this, HashLoc, End, tok::annot_module_begin, M);
  }
}

/// HandleIncludeNextDirective - Implements \#include_next.
///
void Preprocessor::HandleIncludeNextDirective(SourceLocation HashLoc,
                                              Token &IncludeNextTok) {
  Diag(IncludeNextTok, diag::ext_pp_include_next_directive);

  // #include_next is like #include, except that we start searching after
  // the current found directory.  If we can't do this, issue a
  // diagnostic.
  const DirectoryLookup *Lookup = CurDirLookup;
  const FileEntry *LookupFromFile = nullptr;
  if (isInPrimaryFile()) {
    Lookup = nullptr;
    Diag(IncludeNextTok, diag::pp_include_next_in_primary);
  } else if (CurSubmodule) {
    // Start looking up in the directory *after* the one in which the current
    // file would be found, if any.
    assert(CurPPLexer && "#include_next directive in macro?");
    LookupFromFile = CurPPLexer->getFileEntry();
    Lookup = nullptr;
  } else if (!Lookup) {
    Diag(IncludeNextTok, diag::pp_include_next_absolute_path);
  } else {
    // Start looking up in the next directory.
    ++Lookup;
  }

  return HandleIncludeDirective(HashLoc, IncludeNextTok, Lookup,
                                LookupFromFile);
}

/// HandleMicrosoftImportDirective - Implements \#import for Microsoft Mode
void Preprocessor::HandleMicrosoftImportDirective(Token &Tok) {
  // The Microsoft #import directive takes a type library and generates header
  // files from it, and includes those.  This is beyond the scope of what clang
  // does, so we ignore it and error out.  However, #import can optionally have
  // trailing attributes that span multiple lines.  We're going to eat those
  // so we can continue processing from there.
  Diag(Tok, diag::err_pp_import_directive_ms );

  // Read tokens until we get to the end of the directive.  Note that the 
  // directive can be split over multiple lines using the backslash character.
  DiscardUntilEndOfDirective();
}

/// HandleImportDirective - Implements \#import.
///
void Preprocessor::HandleImportDirective(SourceLocation HashLoc,
                                         Token &ImportTok) {
  if (!LangOpts.ObjC1) {  // #import is standard for ObjC.
    if (LangOpts.MSVCCompat)
      return HandleMicrosoftImportDirective(ImportTok);
    Diag(ImportTok, diag::ext_pp_import_directive);
  }
  return HandleIncludeDirective(HashLoc, ImportTok, nullptr, nullptr, true);
}

/// HandleIncludeMacrosDirective - The -imacros command line option turns into a
/// pseudo directive in the predefines buffer.  This handles it by sucking all
/// tokens through the preprocessor and discarding them (only keeping the side
/// effects on the preprocessor).
void Preprocessor::HandleIncludeMacrosDirective(SourceLocation HashLoc,
                                                Token &IncludeMacrosTok) {
  // This directive should only occur in the predefines buffer.  If not, emit an
  // error and reject it.
  SourceLocation Loc = IncludeMacrosTok.getLocation();
  if (strcmp(SourceMgr.getBufferName(Loc), "<built-in>") != 0) {
    Diag(IncludeMacrosTok.getLocation(),
         diag::pp_include_macros_out_of_predefines);
    DiscardUntilEndOfDirective();
    return;
  }

  // Treat this as a normal #include for checking purposes.  If this is
  // successful, it will push a new lexer onto the include stack.
  HandleIncludeDirective(HashLoc, IncludeMacrosTok);

  Token TmpTok;
  do {
    Lex(TmpTok);
    assert(TmpTok.isNot(tok::eof) && "Didn't find end of -imacros!");
  } while (TmpTok.isNot(tok::hashhash));
}

//===----------------------------------------------------------------------===//
// Preprocessor Macro Directive Handling.
//===----------------------------------------------------------------------===//

/// ReadMacroDefinitionArgList - The ( starting an argument list of a macro
/// definition has just been read.  Lex the rest of the arguments and the
/// closing ), updating MI with what we learn.  Return true if an error occurs
/// parsing the arg list.
bool Preprocessor::ReadMacroDefinitionArgList(MacroInfo *MI, Token &Tok) {
  SmallVector<IdentifierInfo*, 32> Arguments;

  while (1) {
    LexUnexpandedToken(Tok);
    switch (Tok.getKind()) {
    case tok::r_paren:
      // Found the end of the argument list.
      if (Arguments.empty())  // #define FOO()
        return false;
      // Otherwise we have #define FOO(A,)
      Diag(Tok, diag::err_pp_expected_ident_in_arg_list);
      return true;
    case tok::ellipsis:  // #define X(... -> C99 varargs
      if (!LangOpts.C99)
        Diag(Tok, LangOpts.CPlusPlus11 ? 
             diag::warn_cxx98_compat_variadic_macro :
             diag::ext_variadic_macro);

      // OpenCL v1.2 s6.9.e: variadic macros are not supported.
      if (LangOpts.OpenCL) {
        Diag(Tok, diag::err_pp_opencl_variadic_macros);
        return true;
      }

      // Lex the token after the identifier.
      LexUnexpandedToken(Tok);
      if (Tok.isNot(tok::r_paren)) {
        Diag(Tok, diag::err_pp_missing_rparen_in_macro_def);
        return true;
      }
      // Add the __VA_ARGS__ identifier as an argument.
      Arguments.push_back(Ident__VA_ARGS__);
      MI->setIsC99Varargs();
      MI->setArgumentList(&Arguments[0], Arguments.size(), BP);
      return false;
    case tok::eod:  // #define X(
      Diag(Tok, diag::err_pp_missing_rparen_in_macro_def);
      return true;
    default:
      // Handle keywords and identifiers here to accept things like
      // #define Foo(for) for.
      IdentifierInfo *II = Tok.getIdentifierInfo();
      if (!II) {
        // #define X(1
        Diag(Tok, diag::err_pp_invalid_tok_in_arg_list);
        return true;
      }

      // If this is already used as an argument, it is used multiple times (e.g.
      // #define X(A,A.
      if (std::find(Arguments.begin(), Arguments.end(), II) !=
          Arguments.end()) {  // C99 6.10.3p6
        Diag(Tok, diag::err_pp_duplicate_name_in_arg_list) << II;
        return true;
      }

      // Add the argument to the macro info.
      Arguments.push_back(II);

      // Lex the token after the identifier.
      LexUnexpandedToken(Tok);

      switch (Tok.getKind()) {
      default:          // #define X(A B
        Diag(Tok, diag::err_pp_expected_comma_in_arg_list);
        return true;
      case tok::r_paren: // #define X(A)
        MI->setArgumentList(&Arguments[0], Arguments.size(), BP);
        return false;
      case tok::comma:  // #define X(A,
        break;
      case tok::ellipsis:  // #define X(A... -> GCC extension
        // Diagnose extension.
        Diag(Tok, diag::ext_named_variadic_macro);

        // Lex the token after the identifier.
        LexUnexpandedToken(Tok);
        if (Tok.isNot(tok::r_paren)) {
          Diag(Tok, diag::err_pp_missing_rparen_in_macro_def);
          return true;
        }

        MI->setIsGNUVarargs();
        MI->setArgumentList(&Arguments[0], Arguments.size(), BP);
        return false;
      }
    }
  }
}

static bool isConfigurationPattern(Token &MacroName, MacroInfo *MI,
                                   const LangOptions &LOptions) {
  if (MI->getNumTokens() == 1) {
    const Token &Value = MI->getReplacementToken(0);

    // Macro that is identity, like '#define inline inline' is a valid pattern.
    if (MacroName.getKind() == Value.getKind())
      return true;

    // Macro that maps a keyword to the same keyword decorated with leading/
    // trailing underscores is a valid pattern:
    //    #define inline __inline
    //    #define inline __inline__
    //    #define inline _inline (in MS compatibility mode)
    StringRef MacroText = MacroName.getIdentifierInfo()->getName();
    if (IdentifierInfo *II = Value.getIdentifierInfo()) {
      if (!II->isKeyword(LOptions))
        return false;
      StringRef ValueText = II->getName();
      StringRef TrimmedValue = ValueText;
      if (!ValueText.startswith("__")) {
        if (ValueText.startswith("_"))
          TrimmedValue = TrimmedValue.drop_front(1);
        else
          return false;
      } else {
        TrimmedValue = TrimmedValue.drop_front(2);
        if (TrimmedValue.endswith("__"))
          TrimmedValue = TrimmedValue.drop_back(2);
      }
      return TrimmedValue.equals(MacroText);
    } else {
      return false;
    }
  }

  // #define inline
  if (MacroName.isOneOf(tok::kw_extern, tok::kw_inline, tok::kw_static,
                        tok::kw_const) &&
      MI->getNumTokens() == 0) {
    return true;
  }

  return false;
}

/// HandleDefineDirective - Implements \#define.  This consumes the entire macro
/// line then lets the caller lex the next real token.
void Preprocessor::HandleDefineDirective(Token &DefineTok,
                                         bool ImmediatelyAfterHeaderGuard) {
  ++NumDefined;

  Token MacroNameTok;
  bool MacroShadowsKeyword;
  ReadMacroName(MacroNameTok, MU_Define, &MacroShadowsKeyword);

  // Error reading macro name?  If so, diagnostic already issued.
  if (MacroNameTok.is(tok::eod))
    return;

  Token LastTok = MacroNameTok;

  // If we are supposed to keep comments in #defines, reenable comment saving
  // mode.
  if (CurLexer) CurLexer->SetCommentRetentionState(KeepMacroComments);

  // Create the new macro.
  MacroInfo *MI = AllocateMacroInfo(MacroNameTok.getLocation());

  Token Tok;
  LexUnexpandedToken(Tok);

  // If this is a function-like macro definition, parse the argument list,
  // marking each of the identifiers as being used as macro arguments.  Also,
  // check other constraints on the first token of the macro body.
  if (Tok.is(tok::eod)) {
    if (ImmediatelyAfterHeaderGuard) {
      // Save this macro information since it may part of a header guard.
      CurPPLexer->MIOpt.SetDefinedMacro(MacroNameTok.getIdentifierInfo(),
                                        MacroNameTok.getLocation());
    }
    // If there is no body to this macro, we have no special handling here.
  } else if (Tok.hasLeadingSpace()) {
    // This is a normal token with leading space.  Clear the leading space
    // marker on the first token to get proper expansion.
    Tok.clearFlag(Token::LeadingSpace);
  } else if (Tok.is(tok::l_paren)) {
    // This is a function-like macro definition.  Read the argument list.
    MI->setIsFunctionLike();
    if (ReadMacroDefinitionArgList(MI, LastTok)) {
      // Throw away the rest of the line.
      if (CurPPLexer->ParsingPreprocessorDirective)
        DiscardUntilEndOfDirective();
      return;
    }

    // If this is a definition of a variadic C99 function-like macro, not using
    // the GNU named varargs extension, enabled __VA_ARGS__.

    // "Poison" __VA_ARGS__, which can only appear in the expansion of a macro.
    // This gets unpoisoned where it is allowed.
    assert(Ident__VA_ARGS__->isPoisoned() && "__VA_ARGS__ should be poisoned!");
    if (MI->isC99Varargs())
      Ident__VA_ARGS__->setIsPoisoned(false);

    // Read the first token after the arg list for down below.
    LexUnexpandedToken(Tok);
  } else if (LangOpts.C99 || LangOpts.CPlusPlus11) {
    // C99 requires whitespace between the macro definition and the body.  Emit
    // a diagnostic for something like "#define X+".
    Diag(Tok, diag::ext_c99_whitespace_required_after_macro_name);
  } else {
    // C90 6.8 TC1 says: "In the definition of an object-like macro, if the
    // first character of a replacement list is not a character required by
    // subclause 5.2.1, then there shall be white-space separation between the
    // identifier and the replacement list.".  5.2.1 lists this set:
    //   "A-Za-z0-9!"#%&'()*+,_./:;<=>?[\]^_{|}~" as well as whitespace, which
    // is irrelevant here.
    bool isInvalid = false;
    if (Tok.is(tok::at)) // @ is not in the list above.
      isInvalid = true;
    else if (Tok.is(tok::unknown)) {
      // If we have an unknown token, it is something strange like "`".  Since
      // all of valid characters would have lexed into a single character
      // token of some sort, we know this is not a valid case.
      isInvalid = true;
    }
    if (isInvalid)
      Diag(Tok, diag::ext_missing_whitespace_after_macro_name);
    else
      Diag(Tok, diag::warn_missing_whitespace_after_macro_name);
  }

  if (!Tok.is(tok::eod))
    LastTok = Tok;

  // Read the rest of the macro body.
  if (MI->isObjectLike()) {
    // Object-like macros are very simple, just read their body.
    while (Tok.isNot(tok::eod)) {
      LastTok = Tok;
      MI->AddTokenToBody(Tok);
      // Get the next token of the macro.
      LexUnexpandedToken(Tok);
    }

  } else {
    // Otherwise, read the body of a function-like macro.  While we are at it,
    // check C99 6.10.3.2p1: ensure that # operators are followed by macro
    // parameters in function-like macro expansions.
    while (Tok.isNot(tok::eod)) {
      LastTok = Tok;

      if (Tok.isNot(tok::hash) && Tok.isNot(tok::hashhash)) {
        MI->AddTokenToBody(Tok);

        // Get the next token of the macro.
        LexUnexpandedToken(Tok);
        continue;
      }

      // If we're in -traditional mode, then we should ignore stringification
      // and token pasting. Mark the tokens as unknown so as not to confuse
      // things.
      if (getLangOpts().TraditionalCPP) {
        Tok.setKind(tok::unknown);
        MI->AddTokenToBody(Tok);

        // Get the next token of the macro.
        LexUnexpandedToken(Tok);
        continue;
      }

      if (Tok.is(tok::hashhash)) {
        
        // If we see token pasting, check if it looks like the gcc comma
        // pasting extension.  We'll use this information to suppress
        // diagnostics later on.
        
        // Get the next token of the macro.
        LexUnexpandedToken(Tok);

        if (Tok.is(tok::eod)) {
          MI->AddTokenToBody(LastTok);
          break;
        }

        unsigned NumTokens = MI->getNumTokens();
        if (NumTokens && Tok.getIdentifierInfo() == Ident__VA_ARGS__ &&
            MI->getReplacementToken(NumTokens-1).is(tok::comma))
          MI->setHasCommaPasting();

        // Things look ok, add the '##' token to the macro.
        MI->AddTokenToBody(LastTok);
        continue;
      }

      // Get the next token of the macro.
      LexUnexpandedToken(Tok);

      // Check for a valid macro arg identifier.
      if (Tok.getIdentifierInfo() == nullptr ||
          MI->getArgumentNum(Tok.getIdentifierInfo()) == -1) {

        // If this is assembler-with-cpp mode, we accept random gibberish after
        // the '#' because '#' is often a comment character.  However, change
        // the kind of the token to tok::unknown so that the preprocessor isn't
        // confused.
        if (getLangOpts().AsmPreprocessor && Tok.isNot(tok::eod)) {
          LastTok.setKind(tok::unknown);
          MI->AddTokenToBody(LastTok);
          continue;
        } else {
          Diag(Tok, diag::err_pp_stringize_not_parameter);

          // Disable __VA_ARGS__ again.
          Ident__VA_ARGS__->setIsPoisoned(true);
          return;
        }
      }

      // Things look ok, add the '#' and param name tokens to the macro.
      MI->AddTokenToBody(LastTok);
      MI->AddTokenToBody(Tok);
      LastTok = Tok;

      // Get the next token of the macro.
      LexUnexpandedToken(Tok);
    }
  }

  if (MacroShadowsKeyword &&
      !isConfigurationPattern(MacroNameTok, MI, getLangOpts())) {
    Diag(MacroNameTok, diag::warn_pp_macro_hides_keyword);
  }

  // Disable __VA_ARGS__ again.
  Ident__VA_ARGS__->setIsPoisoned(true);

  // Check that there is no paste (##) operator at the beginning or end of the
  // replacement list.
  unsigned NumTokens = MI->getNumTokens();
  if (NumTokens != 0) {
    if (MI->getReplacementToken(0).is(tok::hashhash)) {
      Diag(MI->getReplacementToken(0), diag::err_paste_at_start);
      return;
    }
    if (MI->getReplacementToken(NumTokens-1).is(tok::hashhash)) {
      Diag(MI->getReplacementToken(NumTokens-1), diag::err_paste_at_end);
      return;
    }
  }

  MI->setDefinitionEndLoc(LastTok.getLocation());

  // Finally, if this identifier already had a macro defined for it, verify that
  // the macro bodies are identical, and issue diagnostics if they are not.
  if (const MacroInfo *OtherMI=getMacroInfo(MacroNameTok.getIdentifierInfo())) {
    // It is very common for system headers to have tons of macro redefinitions
    // and for warnings to be disabled in system headers.  If this is the case,
    // then don't bother calling MacroInfo::isIdenticalTo.
    if (!getDiagnostics().getSuppressSystemWarnings() ||
        !SourceMgr.isInSystemHeader(DefineTok.getLocation())) {
      if (!OtherMI->isUsed() && OtherMI->isWarnIfUnused())
        Diag(OtherMI->getDefinitionLoc(), diag::pp_macro_not_used);

      // Warn if defining "__LINE__" and other builtins, per C99 6.10.8/4 and 
      // C++ [cpp.predefined]p4, but allow it as an extension.
      if (OtherMI->isBuiltinMacro())
        Diag(MacroNameTok, diag::ext_pp_redef_builtin_macro);
      // Macros must be identical.  This means all tokens and whitespace
      // separation must be the same.  C99 6.10.3p2.
      else if (!OtherMI->isAllowRedefinitionsWithoutWarning() &&
               !MI->isIdenticalTo(*OtherMI, *this, /*Syntactic=*/LangOpts.MicrosoftExt)) {
        Diag(MI->getDefinitionLoc(), diag::ext_pp_macro_redef)
          << MacroNameTok.getIdentifierInfo();
        Diag(OtherMI->getDefinitionLoc(), diag::note_previous_definition);
      }
    }
    if (OtherMI->isWarnIfUnused())
      WarnUnusedMacroLocs.erase(OtherMI->getDefinitionLoc());
  }

  DefMacroDirective *MD =
      appendDefMacroDirective(MacroNameTok.getIdentifierInfo(), MI);

  assert(!MI->isUsed());
  // If we need warning for not using the macro, add its location in the
  // warn-because-unused-macro set. If it gets used it will be removed from set.
  if (getSourceManager().isInMainFile(MI->getDefinitionLoc()) &&
      !Diags->isIgnored(diag::pp_macro_not_used, MI->getDefinitionLoc())) {
    MI->setIsWarnIfUnused(true);
    WarnUnusedMacroLocs.insert(MI->getDefinitionLoc());
  }

  // If the callbacks want to know, tell them about the macro definition.
  if (Callbacks)
    Callbacks->MacroDefined(MacroNameTok, MD);
}

/// HandleUndefDirective - Implements \#undef.
///
void Preprocessor::HandleUndefDirective(Token &UndefTok) {
  ++NumUndefined;

  Token MacroNameTok;
  ReadMacroName(MacroNameTok, MU_Undef);

  // Error reading macro name?  If so, diagnostic already issued.
  if (MacroNameTok.is(tok::eod))
    return;

  // Check to see if this is the last token on the #undef line.
  CheckEndOfDirective("undef");

  // Okay, we have a valid identifier to undef.
  auto *II = MacroNameTok.getIdentifierInfo();
  auto MD = getMacroDefinition(II);

  // If the callbacks want to know, tell them about the macro #undef.
  // Note: no matter if the macro was defined or not.
  if (Callbacks)
    Callbacks->MacroUndefined(MacroNameTok, MD);

  // If the macro is not defined, this is a noop undef, just return.
  const MacroInfo *MI = MD.getMacroInfo();
  if (!MI)
    return;

  if (!MI->isUsed() && MI->isWarnIfUnused())
    Diag(MI->getDefinitionLoc(), diag::pp_macro_not_used);

  if (MI->isWarnIfUnused())
    WarnUnusedMacroLocs.erase(MI->getDefinitionLoc());

  appendMacroDirective(MacroNameTok.getIdentifierInfo(),
                       AllocateUndefMacroDirective(MacroNameTok.getLocation()));
}


//===----------------------------------------------------------------------===//
// Preprocessor Conditional Directive Handling.
//===----------------------------------------------------------------------===//

/// HandleIfdefDirective - Implements the \#ifdef/\#ifndef directive.  isIfndef
/// is true when this is a \#ifndef directive.  ReadAnyTokensBeforeDirective is
/// true if any tokens have been returned or pp-directives activated before this
/// \#ifndef has been lexed.
///
void Preprocessor::HandleIfdefDirective(Token &Result, bool isIfndef,
                                        bool ReadAnyTokensBeforeDirective) {
  ++NumIf;
  Token DirectiveTok = Result;

  Token MacroNameTok;
  ReadMacroName(MacroNameTok);

  // Error reading macro name?  If so, diagnostic already issued.
  if (MacroNameTok.is(tok::eod)) {
    // Skip code until we get to #endif.  This helps with recovery by not
    // emitting an error when the #endif is reached.
    SkipExcludedConditionalBlock(DirectiveTok.getLocation(),
                                 /*Foundnonskip*/false, /*FoundElse*/false);
    return;
  }

  // Check to see if this is the last token on the #if[n]def line.
  CheckEndOfDirective(isIfndef ? "ifndef" : "ifdef");

  IdentifierInfo *MII = MacroNameTok.getIdentifierInfo();
  auto MD = getMacroDefinition(MII);
  MacroInfo *MI = MD.getMacroInfo();

  if (CurPPLexer->getConditionalStackDepth() == 0) {
    // If the start of a top-level #ifdef and if the macro is not defined,
    // inform MIOpt that this might be the start of a proper include guard.
    // Otherwise it is some other form of unknown conditional which we can't
    // handle.
    if (!ReadAnyTokensBeforeDirective && !MI) {
      assert(isIfndef && "#ifdef shouldn't reach here");
      CurPPLexer->MIOpt.EnterTopLevelIfndef(MII, MacroNameTok.getLocation());
    } else
      CurPPLexer->MIOpt.EnterTopLevelConditional();
  }

  // If there is a macro, process it.
  if (MI)  // Mark it used.
    markMacroAsUsed(MI);

  if (Callbacks) {
    if (isIfndef)
      Callbacks->Ifndef(DirectiveTok.getLocation(), MacroNameTok, MD);
    else
      Callbacks->Ifdef(DirectiveTok.getLocation(), MacroNameTok, MD);
  }

  // Should we include the stuff contained by this directive?
  if (!MI == isIfndef) {
    // Yes, remember that we are inside a conditional, then lex the next token.
    CurPPLexer->pushConditionalLevel(DirectiveTok.getLocation(),
                                     /*wasskip*/false, /*foundnonskip*/true,
                                     /*foundelse*/false);
  } else {
    // No, skip the contents of this block.
    SkipExcludedConditionalBlock(DirectiveTok.getLocation(),
                                 /*Foundnonskip*/false,
                                 /*FoundElse*/false);
  }
}

/// HandleIfDirective - Implements the \#if directive.
///
void Preprocessor::HandleIfDirective(Token &IfToken,
                                     bool ReadAnyTokensBeforeDirective) {
  ++NumIf;

  // Parse and evaluate the conditional expression.
  IdentifierInfo *IfNDefMacro = nullptr;
  const SourceLocation ConditionalBegin = CurPPLexer->getSourceLocation();
  const bool ConditionalTrue = EvaluateDirectiveExpression(IfNDefMacro);
  const SourceLocation ConditionalEnd = CurPPLexer->getSourceLocation();

  // If this condition is equivalent to #ifndef X, and if this is the first
  // directive seen, handle it for the multiple-include optimization.
  if (CurPPLexer->getConditionalStackDepth() == 0) {
    if (!ReadAnyTokensBeforeDirective && IfNDefMacro && ConditionalTrue)
      // FIXME: Pass in the location of the macro name, not the 'if' token.
      CurPPLexer->MIOpt.EnterTopLevelIfndef(IfNDefMacro, IfToken.getLocation());
    else
      CurPPLexer->MIOpt.EnterTopLevelConditional();
  }

  if (Callbacks)
    Callbacks->If(IfToken.getLocation(),
                  SourceRange(ConditionalBegin, ConditionalEnd),
                  (ConditionalTrue ? PPCallbacks::CVK_True : PPCallbacks::CVK_False));

  // Should we include the stuff contained by this directive?
  if (ConditionalTrue) {
    // Yes, remember that we are inside a conditional, then lex the next token.
    CurPPLexer->pushConditionalLevel(IfToken.getLocation(), /*wasskip*/false,
                                   /*foundnonskip*/true, /*foundelse*/false);
  } else {
    // No, skip the contents of this block.
    SkipExcludedConditionalBlock(IfToken.getLocation(), /*Foundnonskip*/false,
                                 /*FoundElse*/false);
  }
}

/// HandleEndifDirective - Implements the \#endif directive.
///
void Preprocessor::HandleEndifDirective(Token &EndifToken) {
  ++NumEndif;

  // Check that this is the whole directive.
  CheckEndOfDirective("endif");

  PPConditionalInfo CondInfo;
  if (CurPPLexer->popConditionalLevel(CondInfo)) {
    // No conditionals on the stack: this is an #endif without an #if.
    Diag(EndifToken, diag::err_pp_endif_without_if);
    return;
  }

  // If this the end of a top-level #endif, inform MIOpt.
  if (CurPPLexer->getConditionalStackDepth() == 0)
    CurPPLexer->MIOpt.ExitTopLevelConditional();

  assert(!CondInfo.WasSkipping && !CurPPLexer->LexingRawMode &&
         "This code should only be reachable in the non-skipping case!");

  if (Callbacks)
    Callbacks->Endif(EndifToken.getLocation(), CondInfo.IfLoc);
}

/// HandleElseDirective - Implements the \#else directive.
///
void Preprocessor::HandleElseDirective(Token &Result) {
  ++NumElse;

  // #else directive in a non-skipping conditional... start skipping.
  CheckEndOfDirective("else");

  PPConditionalInfo CI;
  if (CurPPLexer->popConditionalLevel(CI)) {
    Diag(Result, diag::pp_err_else_without_if);
    return;
  }

  // If this is a top-level #else, inform the MIOpt.
  if (CurPPLexer->getConditionalStackDepth() == 0)
    CurPPLexer->MIOpt.EnterTopLevelConditional();

  // If this is a #else with a #else before it, report the error.
  if (CI.FoundElse) Diag(Result, diag::pp_err_else_after_else);

  if (Callbacks)
    Callbacks->Else(Result.getLocation(), CI.IfLoc);

  // Finally, skip the rest of the contents of this block.
  SkipExcludedConditionalBlock(CI.IfLoc, /*Foundnonskip*/true,
                               /*FoundElse*/true, Result.getLocation());
}

/// HandleElifDirective - Implements the \#elif directive.
///
void Preprocessor::HandleElifDirective(Token &ElifToken) {
  ++NumElse;

  // #elif directive in a non-skipping conditional... start skipping.
  // We don't care what the condition is, because we will always skip it (since
  // the block immediately before it was included).
  const SourceLocation ConditionalBegin = CurPPLexer->getSourceLocation();
  DiscardUntilEndOfDirective();
  const SourceLocation ConditionalEnd = CurPPLexer->getSourceLocation();

  PPConditionalInfo CI;
  if (CurPPLexer->popConditionalLevel(CI)) {
    Diag(ElifToken, diag::pp_err_elif_without_if);
    return;
  }

  // If this is a top-level #elif, inform the MIOpt.
  if (CurPPLexer->getConditionalStackDepth() == 0)
    CurPPLexer->MIOpt.EnterTopLevelConditional();

  // If this is a #elif with a #else before it, report the error.
  if (CI.FoundElse) Diag(ElifToken, diag::pp_err_elif_after_else);
  
  if (Callbacks)
    Callbacks->Elif(ElifToken.getLocation(),
                    SourceRange(ConditionalBegin, ConditionalEnd),
                    PPCallbacks::CVK_NotEvaluated, CI.IfLoc);

  // Finally, skip the rest of the contents of this block.
  SkipExcludedConditionalBlock(CI.IfLoc, /*Foundnonskip*/true,
                               /*FoundElse*/CI.FoundElse,
                               ElifToken.getLocation());
}
