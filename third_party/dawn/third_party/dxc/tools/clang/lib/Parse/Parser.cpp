//===--- Parser.cpp - C Language Family Parser ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements the Parser interfaces.
//
//===----------------------------------------------------------------------===//

#include "clang/Parse/Parser.h"
#include "RAIIObjectsForParser.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/Parse/ParseDiagnostic.h"
#include "clang/Sema/DeclSpec.h"
#include "clang/Sema/ParsedTemplate.h"
#include "clang/Sema/Scope.h"
#include "llvm/Support/raw_ostream.h"
using namespace clang;


namespace {
/// \brief A comment handler that passes comments found by the preprocessor
/// to the parser action.
class ActionCommentHandler : public CommentHandler {
  Sema &S;

public:
  explicit ActionCommentHandler(Sema &S) : S(S) { }

  bool HandleComment(Preprocessor &PP, SourceRange Comment) override {
    S.ActOnComment(Comment);
    return false;
  }
};

/// \brief RAIIObject to destroy the contents of a SmallVector of
/// TemplateIdAnnotation pointers and clear the vector.
class DestroyTemplateIdAnnotationsRAIIObj {
  SmallVectorImpl<TemplateIdAnnotation *> &Container;

public:
  DestroyTemplateIdAnnotationsRAIIObj(
      SmallVectorImpl<TemplateIdAnnotation *> &Container)
      : Container(Container) {}

  ~DestroyTemplateIdAnnotationsRAIIObj() {
    for (SmallVectorImpl<TemplateIdAnnotation *>::iterator I =
             Container.begin(),
                                                           E = Container.end();
         I != E; ++I)
      (*I)->Destroy();
    Container.clear();
  }
};
} // end anonymous namespace

IdentifierInfo *Parser::getSEHExceptKeyword() {
  // __except is accepted as a (contextual) keyword 
  if (!Ident__except && (getLangOpts().MicrosoftExt || getLangOpts().Borland))
    Ident__except = PP.getIdentifierInfo("__except");

  return Ident__except;
}

Parser::Parser(Preprocessor &pp, Sema &actions, bool skipFunctionBodies)
  : PP(pp), Actions(actions), Diags(PP.getDiagnostics()),
    GreaterThanIsOperator(true), ColonIsSacred(false), 
    InMessageExpression(false), TemplateParameterDepth(0),
    ParsingInObjCContainer(false) {
  SkipFunctionBodies = pp.isCodeCompletionEnabled() || skipFunctionBodies;
  Tok.startToken();
  Tok.setKind(tok::eof);
  Actions.CurScope = nullptr;
  NumCachedScopes = 0;
  ParenCount = BracketCount = BraceCount = 0;
  CurParsedObjCImpl = nullptr;

  // Add #pragma handlers. These are removed and destroyed in the
  // destructor.
  initializePragmaHandlers();

  CommentSemaHandler.reset(new ActionCommentHandler(actions));
  PP.addCommentHandler(CommentSemaHandler.get());

  PP.setCodeCompletionHandler(*this);
}

DiagnosticBuilder Parser::Diag(SourceLocation Loc, unsigned DiagID) {
  return Diags.Report(Loc, DiagID);
}

DiagnosticBuilder Parser::Diag(const Token &Tok, unsigned DiagID) {
  return Diag(Tok.getLocation(), DiagID);
}

/// \brief Emits a diagnostic suggesting parentheses surrounding a
/// given range.
///
/// \param Loc The location where we'll emit the diagnostic.
/// \param DK The kind of diagnostic to emit.
/// \param ParenRange Source range enclosing code that should be parenthesized.
void Parser::SuggestParentheses(SourceLocation Loc, unsigned DK,
                                SourceRange ParenRange) {
  SourceLocation EndLoc = PP.getLocForEndOfToken(ParenRange.getEnd());
  if (!ParenRange.getEnd().isFileID() || EndLoc.isInvalid()) {
    // We can't display the parentheses, so just dig the
    // warning/error and return.
    Diag(Loc, DK);
    return;
  }

  Diag(Loc, DK)
    << FixItHint::CreateInsertion(ParenRange.getBegin(), "(")
    << FixItHint::CreateInsertion(EndLoc, ")");
}

static bool IsCommonTypo(tok::TokenKind ExpectedTok, const Token &Tok) {
  switch (ExpectedTok) {
  case tok::semi:
    return Tok.is(tok::colon) || Tok.is(tok::comma); // : or , for ;
  default: return false;
  }
}

bool Parser::ExpectAndConsume(tok::TokenKind ExpectedTok, unsigned DiagID,
                              StringRef Msg) {
  if (Tok.is(ExpectedTok) || Tok.is(tok::code_completion)) {
    ConsumeAnyToken();
    return false;
  }

  // Detect common single-character typos and resume.
  if (IsCommonTypo(ExpectedTok, Tok)) {
    SourceLocation Loc = Tok.getLocation();
    {
      DiagnosticBuilder DB = Diag(Loc, DiagID);
      DB << FixItHint::CreateReplacement(
                SourceRange(Loc), tok::getPunctuatorSpelling(ExpectedTok));
      if (DiagID == diag::err_expected)
        DB << ExpectedTok;
      else if (DiagID == diag::err_expected_after)
        DB << Msg << ExpectedTok;
      else
        DB << Msg;
    }

    // Pretend there wasn't a problem.
    ConsumeAnyToken();
    return false;
  }

  SourceLocation EndLoc = PP.getLocForEndOfToken(PrevTokLocation);
  const char *Spelling = nullptr;
  if (EndLoc.isValid())
    Spelling = tok::getPunctuatorSpelling(ExpectedTok);

  DiagnosticBuilder DB =
      Spelling
          ? Diag(EndLoc, DiagID) << FixItHint::CreateInsertion(EndLoc, Spelling)
          : Diag(Tok, DiagID);
  if (DiagID == diag::err_expected)
    DB << ExpectedTok;
  else if (DiagID == diag::err_expected_after)
    DB << Msg << ExpectedTok;
  else
    DB << Msg;

  return true;
}

bool Parser::ExpectAndConsumeSemi(unsigned DiagID) {
  if (TryConsumeToken(tok::semi))
    return false;

  if (Tok.is(tok::code_completion)) {
    handleUnexpectedCodeCompletionToken();
    return false;
  }
  
  if ((Tok.is(tok::r_paren) || Tok.is(tok::r_square)) && 
      NextToken().is(tok::semi)) {
    Diag(Tok, diag::err_extraneous_token_before_semi)
      << PP.getSpelling(Tok)
      << FixItHint::CreateRemoval(Tok.getLocation());
    ConsumeAnyToken(); // The ')' or ']'.
    ConsumeToken(); // The ';'.
    return false;
  }
  
  return ExpectAndConsume(tok::semi, DiagID);
}

void Parser::ConsumeExtraSemi(ExtraSemiKind Kind, unsigned TST) {
  if (!Tok.is(tok::semi)) return;

  bool HadMultipleSemis = false;
  SourceLocation StartLoc = Tok.getLocation();
  SourceLocation EndLoc = Tok.getLocation();
  ConsumeToken();

  while ((Tok.is(tok::semi) && !Tok.isAtStartOfLine())) {
    HadMultipleSemis = true;
    EndLoc = Tok.getLocation();
    ConsumeToken();
  }

  // C++11 allows extra semicolons at namespace scope, but not in any of the
  // other contexts.
  if (!getLangOpts().HLSL && (Kind == OutsideFunction && getLangOpts().CPlusPlus)) { // HLSL Change
    if (getLangOpts().CPlusPlus11)
      Diag(StartLoc, diag::warn_cxx98_compat_top_level_semi)
          << FixItHint::CreateRemoval(SourceRange(StartLoc, EndLoc));
    else
      Diag(StartLoc, diag::ext_extra_semi_cxx11)
          << FixItHint::CreateRemoval(SourceRange(StartLoc, EndLoc));
    return;
  }

  if (Kind != AfterMemberFunctionDefinition || HadMultipleSemis)
    Diag(StartLoc, diag::ext_extra_semi)
        << Kind << DeclSpec::getSpecifierName((DeclSpec::TST)TST,
                                    Actions.getASTContext().getPrintingPolicy())
        << FixItHint::CreateRemoval(SourceRange(StartLoc, EndLoc));
  else
    // A single semicolon is valid after a member function definition.
    Diag(StartLoc, diag::warn_extra_semi_after_mem_fn_def)
      << FixItHint::CreateRemoval(SourceRange(StartLoc, EndLoc));
}

//===----------------------------------------------------------------------===//
// Error recovery.
//===----------------------------------------------------------------------===//

static bool HasFlagsSet(Parser::SkipUntilFlags L, Parser::SkipUntilFlags R) {
  return (static_cast<unsigned>(L) & static_cast<unsigned>(R)) != 0;
}

/// SkipUntil - Read tokens until we get to the specified token, then consume
/// it (unless no flag StopBeforeMatch).  Because we cannot guarantee that the
/// token will ever occur, this skips to the next token, or to some likely
/// good stopping point.  If StopAtSemi is true, skipping will stop at a ';'
/// character.
///
/// If SkipUntil finds the specified token, it returns true, otherwise it
/// returns false.
bool Parser::SkipUntil(ArrayRef<tok::TokenKind> Toks, SkipUntilFlags Flags) {
  // We always want this function to skip at least one token if the first token
  // isn't T and if not at EOF.
  bool isFirstTokenSkipped = true;
  while (1) {
    // If we found one of the tokens, stop and return true.
    for (unsigned i = 0, NumToks = Toks.size(); i != NumToks; ++i) {
      if (Tok.is(Toks[i])) {
        if (HasFlagsSet(Flags, StopBeforeMatch)) {
          // Noop, don't consume the token.
        } else {
          ConsumeAnyToken();
        }
        return true;
      }
    }

    // Important special case: The caller has given up and just wants us to
    // skip the rest of the file. Do this without recursing, since we can
    // get here precisely because the caller detected too much recursion.
    if (Toks.size() == 1 && Toks[0] == tok::eof &&
        !HasFlagsSet(Flags, StopAtSemi) &&
        !HasFlagsSet(Flags, StopAtCodeCompletion)) {
      while (Tok.isNot(tok::eof))
        ConsumeAnyToken();
      return true;
    }

    switch (Tok.getKind()) {
    case tok::eof:
      // Ran out of tokens.
      return false;

    case tok::annot_pragma_openmp_end:
      // Stop before an OpenMP pragma boundary.
    case tok::annot_module_begin:
    case tok::annot_module_end:
    case tok::annot_module_include:
      // Stop before we change submodules. They generally indicate a "good"
      // place to pick up parsing again (except in the special case where
      // we're trying to skip to EOF).
      return false;

    case tok::code_completion:
      if (!HasFlagsSet(Flags, StopAtCodeCompletion))
        handleUnexpectedCodeCompletionToken();
      return false;
        
    case tok::l_paren:
      // Recursively skip properly-nested parens.
      ConsumeParen();
      if (HasFlagsSet(Flags, StopAtCodeCompletion))
        SkipUntil(tok::r_paren, StopAtCodeCompletion);
      else
        SkipUntil(tok::r_paren);
      break;
    case tok::l_square:
      // Recursively skip properly-nested square brackets.
      ConsumeBracket();
      if (HasFlagsSet(Flags, StopAtCodeCompletion))
        SkipUntil(tok::r_square, StopAtCodeCompletion);
      else
        SkipUntil(tok::r_square);
      break;
    case tok::l_brace:
      // Recursively skip properly-nested braces.
      ConsumeBrace();
      if (HasFlagsSet(Flags, StopAtCodeCompletion))
        SkipUntil(tok::r_brace, StopAtCodeCompletion);
      else
        SkipUntil(tok::r_brace);
      break;

    // Okay, we found a ']' or '}' or ')', which we think should be balanced.
    // Since the user wasn't looking for this token (if they were, it would
    // already be handled), this isn't balanced.  If there is a LHS token at a
    // higher level, we will assume that this matches the unbalanced token
    // and return it.  Otherwise, this is a spurious RHS token, which we skip.
    case tok::r_paren:
      if (ParenCount && !isFirstTokenSkipped)
        return false;  // Matches something.
      ConsumeParen();
      break;
    case tok::r_square:
      if (BracketCount && !isFirstTokenSkipped)
        return false;  // Matches something.
      ConsumeBracket();
      break;
    case tok::r_brace:
      if (BraceCount && !isFirstTokenSkipped)
        return false;  // Matches something.
      ConsumeBrace();
      break;

    case tok::string_literal:
    case tok::wide_string_literal:
    case tok::utf8_string_literal:
    case tok::utf16_string_literal:
    case tok::utf32_string_literal:
      ConsumeStringToken();
      break;
        
    case tok::semi:
      if (HasFlagsSet(Flags, StopAtSemi))
        return false;
      LLVM_FALLTHROUGH; // HLSL Change
    default:
      // Skip this token.
      ConsumeToken();
      break;
    }
    isFirstTokenSkipped = false;
  }
}

//===----------------------------------------------------------------------===//
// Scope manipulation
//===----------------------------------------------------------------------===//

/// EnterScope - Start a new scope.
void Parser::EnterScope(unsigned ScopeFlags) {
  if (NumCachedScopes) {
    Scope *N = ScopeCache[--NumCachedScopes];
    N->Init(getCurScope(), ScopeFlags);
    Actions.CurScope = N;
  } else {
    Actions.CurScope = new Scope(getCurScope(), ScopeFlags, Diags);
  }
}

/// ExitScope - Pop a scope off the scope stack.
void Parser::ExitScope() {
  assert(getCurScope() && "Scope imbalance!");

  // Inform the actions module that this scope is going away if there are any
  // decls in it.
  Actions.ActOnPopScope(Tok.getLocation(), getCurScope());

  Scope *OldScope = getCurScope();
  Actions.CurScope = OldScope->getParent();

  if (NumCachedScopes == ScopeCacheSize)
    delete OldScope;
  else
    ScopeCache[NumCachedScopes++] = OldScope;
}

/// Set the flags for the current scope to ScopeFlags. If ManageFlags is false,
/// this object does nothing.
Parser::ParseScopeFlags::ParseScopeFlags(Parser *Self, unsigned ScopeFlags,
                                 bool ManageFlags)
  : CurScope(ManageFlags ? Self->getCurScope() : nullptr) {
  if (CurScope) {
    OldFlags = CurScope->getFlags();
    CurScope->setFlags(ScopeFlags);
  }
}

/// Restore the flags for the current scope to what they were before this
/// object overrode them.
Parser::ParseScopeFlags::~ParseScopeFlags() {
  if (CurScope)
    CurScope->setFlags(OldFlags);
}


//===----------------------------------------------------------------------===//
// C99 6.9: External Definitions.
//===----------------------------------------------------------------------===//

Parser::~Parser() {
  // If we still have scopes active, delete the scope tree.
  delete getCurScope();
  Actions.CurScope = nullptr;

  // Free the scope cache.
  for (unsigned i = 0, e = NumCachedScopes; i != e; ++i)
    delete ScopeCache[i];

  resetPragmaHandlers();

  PP.removeCommentHandler(CommentSemaHandler.get());

  PP.clearCodeCompletionHandler();

  if (getLangOpts().DelayedTemplateParsing &&
      !PP.isIncrementalProcessingEnabled() && !TemplateIds.empty()) {
    // If an ASTConsumer parsed delay-parsed templates in their
    // HandleTranslationUnit() method, TemplateIds created there were not
    // guarded by a DestroyTemplateIdAnnotationsRAIIObj object in
    // ParseTopLevelDecl(). Destroy them here.
    DestroyTemplateIdAnnotationsRAIIObj CleanupRAII(TemplateIds);
  }

  assert(TemplateIds.empty() && "Still alive TemplateIdAnnotations around?");
}

/// Initialize - Warm up the parser.
///
void Parser::Initialize() {
  // Create the translation unit scope.  Install it as the current scope.
  assert(getCurScope() == nullptr && "A scope is already active?");
  EnterScope(Scope::DeclScope);
  Actions.ActOnTranslationUnitScope(getCurScope());

  // Initialization for Objective-C context sensitive keywords recognition.
  // Referenced in Parser::ParseObjCTypeQualifierList.
  if (getLangOpts().ObjC1) {
    ObjCTypeQuals[objc_in] = &PP.getIdentifierTable().get("in");
    ObjCTypeQuals[objc_out] = &PP.getIdentifierTable().get("out");
    ObjCTypeQuals[objc_inout] = &PP.getIdentifierTable().get("inout");
    ObjCTypeQuals[objc_oneway] = &PP.getIdentifierTable().get("oneway");
    ObjCTypeQuals[objc_bycopy] = &PP.getIdentifierTable().get("bycopy");
    ObjCTypeQuals[objc_byref] = &PP.getIdentifierTable().get("byref");
    ObjCTypeQuals[objc_nonnull] = &PP.getIdentifierTable().get("nonnull");
    ObjCTypeQuals[objc_nullable] = &PP.getIdentifierTable().get("nullable");
    ObjCTypeQuals[objc_null_unspecified]
      = &PP.getIdentifierTable().get("null_unspecified");
  }

  Ident_instancetype = nullptr;
  Ident_final = nullptr;
  Ident_sealed = nullptr;
  Ident_override = nullptr;

  Ident_super = &PP.getIdentifierTable().get("super");

  Ident_vector = nullptr;
  Ident_bool = nullptr;
  Ident_pixel = nullptr;
  if (getLangOpts().AltiVec || getLangOpts().ZVector) {
    Ident_vector = &PP.getIdentifierTable().get("vector");
    Ident_bool = &PP.getIdentifierTable().get("bool");
  }
  if (getLangOpts().AltiVec)
    Ident_pixel = &PP.getIdentifierTable().get("pixel");

  Ident_introduced = nullptr;
  Ident_deprecated = nullptr;
  Ident_obsoleted = nullptr;
  Ident_unavailable = nullptr;

  Ident__except = nullptr;

  Ident__exception_code = Ident__exception_info = nullptr;
  Ident__abnormal_termination = Ident___exception_code = nullptr;
  Ident___exception_info = Ident___abnormal_termination = nullptr;
  Ident_GetExceptionCode = Ident_GetExceptionInfo = nullptr;
  Ident_AbnormalTermination = nullptr;

  if(getLangOpts().Borland) {
    Ident__exception_info        = PP.getIdentifierInfo("_exception_info");
    Ident___exception_info       = PP.getIdentifierInfo("__exception_info");
    Ident_GetExceptionInfo       = PP.getIdentifierInfo("GetExceptionInformation");
    Ident__exception_code        = PP.getIdentifierInfo("_exception_code");
    Ident___exception_code       = PP.getIdentifierInfo("__exception_code");
    Ident_GetExceptionCode       = PP.getIdentifierInfo("GetExceptionCode");
    Ident__abnormal_termination  = PP.getIdentifierInfo("_abnormal_termination");
    Ident___abnormal_termination = PP.getIdentifierInfo("__abnormal_termination");
    Ident_AbnormalTermination    = PP.getIdentifierInfo("AbnormalTermination");

    PP.SetPoisonReason(Ident__exception_code,diag::err_seh___except_block);
    PP.SetPoisonReason(Ident___exception_code,diag::err_seh___except_block);
    PP.SetPoisonReason(Ident_GetExceptionCode,diag::err_seh___except_block);
    PP.SetPoisonReason(Ident__exception_info,diag::err_seh___except_filter);
    PP.SetPoisonReason(Ident___exception_info,diag::err_seh___except_filter);
    PP.SetPoisonReason(Ident_GetExceptionInfo,diag::err_seh___except_filter);
    PP.SetPoisonReason(Ident__abnormal_termination,diag::err_seh___finally_block);
    PP.SetPoisonReason(Ident___abnormal_termination,diag::err_seh___finally_block);
    PP.SetPoisonReason(Ident_AbnormalTermination,diag::err_seh___finally_block);
  }

  Actions.Initialize();

  // Prime the lexer look-ahead.
  ConsumeToken();
}

void Parser::LateTemplateParserCleanupCallback(void *P) {
  // While this RAII helper doesn't bracket any actual work, the destructor will
  // clean up annotations that were created during ActOnEndOfTranslationUnit
  // when incremental processing is enabled.
  DestroyTemplateIdAnnotationsRAIIObj CleanupRAII(((Parser *)P)->TemplateIds);
}

/// ParseTopLevelDecl - Parse one top-level declaration, return whatever the
/// action tells us to.  This returns true if the EOF was encountered.
bool Parser::ParseTopLevelDecl(DeclGroupPtrTy &Result) {
  DestroyTemplateIdAnnotationsRAIIObj CleanupRAII(TemplateIds);

  // Skip over the EOF token, flagging end of previous input for incremental
  // processing
  if (PP.isIncrementalProcessingEnabled() && Tok.is(tok::eof))
    ConsumeToken();

  Result = DeclGroupPtrTy();
  switch (Tok.getKind()) {
  case tok::annot_pragma_unused:
    if (getLangOpts().HLSL) break; // HLSL Change - this should merge with 'default' and avoid unused pragma handling
    HandlePragmaUnused();
    return false;

  case tok::annot_module_include:
    if (getLangOpts().HLSL) break; // HLSL Change - HLSL does not support modules and cannot get to this codepath
    Actions.ActOnModuleInclude(Tok.getLocation(),
                               reinterpret_cast<Module *>(
                                   Tok.getAnnotationValue()));
    ConsumeToken();
    return false;

  case tok::annot_module_begin:
    if (getLangOpts().HLSL) break; // HLSL Change - HLSL does not support modules and cannot get to this codepath
    Actions.ActOnModuleBegin(Tok.getLocation(), reinterpret_cast<Module *>(
                                                    Tok.getAnnotationValue()));
    ConsumeToken();
    return false;

  case tok::annot_module_end:
    if (getLangOpts().HLSL) break; // HLSL Change - HLSL does not support modules and cannot get to this codepath
    Actions.ActOnModuleEnd(Tok.getLocation(), reinterpret_cast<Module *>(
                                                  Tok.getAnnotationValue()));
    ConsumeToken();
    return false;

  case tok::eof:
    // Late template parsing can begin.
    if (getLangOpts().DelayedTemplateParsing)
      Actions.SetLateTemplateParser(LateTemplateParserCallback,
                                    PP.isIncrementalProcessingEnabled() ?
                                    LateTemplateParserCleanupCallback : nullptr,
                                    this);
    if (!PP.isIncrementalProcessingEnabled())
      Actions.ActOnEndOfTranslationUnit();
    //else don't tell Sema that we ended parsing: more input might come.
    return true;

  // HLSL Change Starts - skip legacy effects technique syntax
  case tok::kw_technique:
    Diag(Tok.getLocation(), diag::warn_hlsl_effect_technique);
    SkipUntil(tok::l_brace);    // skip through {
    SkipUntil(tok::r_brace);    // skip through matching }
    Result = DeclGroupPtrTy();
    return false;
  // HLSL Change Ends

  default:
    break;
  }

  ParsedAttributesWithRange attrs(AttrFactory);
  MaybeParseCXX11Attributes(attrs);
  MaybeParseHLSLAttributes(attrs);      // HLSL Change
  MaybeParseMicrosoftAttributes(attrs);

  Result = ParseExternalDeclaration(attrs);
  return false;
}

/// ParseExternalDeclaration:
///
///       external-declaration: [C99 6.9], declaration: [C++ dcl.dcl]
///         function-definition
///         declaration
/// [GNU]   asm-definition
/// [GNU]   __extension__ external-declaration
/// [OBJC]  objc-class-definition
/// [OBJC]  objc-class-declaration
/// [OBJC]  objc-alias-declaration
/// [OBJC]  objc-protocol-definition
/// [OBJC]  objc-method-definition
/// [OBJC]  @end
/// [C++]   linkage-specification
/// [GNU] asm-definition:
///         simple-asm-expr ';'
/// [C++11] empty-declaration
/// [C++11] attribute-declaration
///
/// [C++11] empty-declaration:
///           ';'
///
/// [C++0x/GNU] 'extern' 'template' declaration
Parser::DeclGroupPtrTy
Parser::ParseExternalDeclaration(ParsedAttributesWithRange &attrs,
                                 ParsingDeclSpec *DS) {
  DestroyTemplateIdAnnotationsRAIIObj CleanupRAII(TemplateIds);
  ParenBraceBracketBalancer BalancerRAIIObj(*this);

  if (PP.isCodeCompletionReached()) {
    cutOffParsing();
    return DeclGroupPtrTy();
  }

  Decl *SingleDecl = nullptr;
  switch (Tok.getKind()) {
  case tok::annot_pragma_vis:
    if (getLangOpts().HLSL) goto dont_know; // HLSL Change - skip unsupported processing
    HandlePragmaVisibility();
    return DeclGroupPtrTy();
  case tok::annot_pragma_pack:
    if (getLangOpts().HLSL) goto dont_know; // HLSL Change - skip unsupported processing
    HandlePragmaPack();
    return DeclGroupPtrTy();
  case tok::annot_pragma_msstruct:
    if (getLangOpts().HLSL) goto dont_know; // HLSL Change - skip unsupported processing
    HandlePragmaMSStruct();
    return DeclGroupPtrTy();
  case tok::annot_pragma_align:
    if (getLangOpts().HLSL) goto dont_know; // HLSL Change - skip unsupported processing
    HandlePragmaAlign();
    return DeclGroupPtrTy();
  case tok::annot_pragma_weak:
    if (getLangOpts().HLSL) goto dont_know; // HLSL Change - skip unsupported processing
    HandlePragmaWeak();
    return DeclGroupPtrTy();
  case tok::annot_pragma_weakalias:
    if (getLangOpts().HLSL) goto dont_know; // HLSL Change - skip unsupported processing
    HandlePragmaWeakAlias();
    return DeclGroupPtrTy();
  case tok::annot_pragma_redefine_extname:
    if (getLangOpts().HLSL) goto dont_know; // HLSL Change - skip unsupported processing
    HandlePragmaRedefineExtname();
    return DeclGroupPtrTy();
  case tok::annot_pragma_fp_contract:
    if (getLangOpts().HLSL) goto dont_know; // HLSL Change - skip unsupported processing
    HandlePragmaFPContract();
    return DeclGroupPtrTy();
  case tok::annot_pragma_opencl_extension:
    if (getLangOpts().HLSL) goto dont_know; // HLSL Change - skip unsupported processing
    HandlePragmaOpenCLExtension();
    return DeclGroupPtrTy();
  case tok::annot_pragma_openmp:
    if (getLangOpts().HLSL) goto dont_know; // HLSL Change - skip unsupported processing
    return ParseOpenMPDeclarativeDirective();
  case tok::annot_pragma_ms_pointers_to_members:
    if (getLangOpts().HLSL) goto dont_know; // HLSL Change - skip unsupported processing
    HandlePragmaMSPointersToMembers();
    return DeclGroupPtrTy();
  case tok::annot_pragma_ms_vtordisp:
    if (getLangOpts().HLSL) goto dont_know; // HLSL Change - skip unsupported processing
    HandlePragmaMSVtorDisp();
    return DeclGroupPtrTy();
  case tok::annot_pragma_ms_pragma:
    if (getLangOpts().HLSL) goto dont_know; // HLSL Change - skip unsupported processing
    HandlePragmaMSPragma();
    return DeclGroupPtrTy();
  case tok::semi:
    // Either a C++11 empty-declaration or attribute-declaration.
    SingleDecl = Actions.ActOnEmptyDeclaration(getCurScope(),
                                               attrs.getList(),
                                               Tok.getLocation());
    ConsumeExtraSemi(OutsideFunction);
    break;
  case tok::r_brace:
    Diag(Tok, diag::err_extraneous_closing_brace);
    ConsumeBrace();
    return DeclGroupPtrTy();
  case tok::eof:
    Diag(Tok, diag::err_expected_external_declaration);
    return DeclGroupPtrTy();
  case tok::kw___extension__: {
    // __extension__ silences extension warnings in the subexpression.
    ExtensionRAIIObject O(Diags);  // Use RAII to do this.
    ConsumeToken();
    return ParseExternalDeclaration(attrs);
  }
  case tok::kw_asm: {
    if (getLangOpts().HLSL) goto dont_know; // HLSL Change - skip unsupported processing
    ProhibitAttributes(attrs);

    SourceLocation StartLoc = Tok.getLocation();
    SourceLocation EndLoc;

    ExprResult Result(ParseSimpleAsm(&EndLoc));

    // Check if GNU-style InlineAsm is disabled.
    // Empty asm string is allowed because it will not introduce
    // any assembly code.
    if (!(getLangOpts().GNUAsm || Result.isInvalid())) {
      const auto *SL = cast<StringLiteral>(Result.get());
      if (!SL->getString().trim().empty())
        Diag(StartLoc, diag::err_gnu_inline_asm_disabled);
    }

    ExpectAndConsume(tok::semi, diag::err_expected_after,
                     "top-level asm block");

    if (Result.isInvalid())
      return DeclGroupPtrTy();
    SingleDecl = Actions.ActOnFileScopeAsmDecl(Result.get(), StartLoc, EndLoc);
    break;
  }
  case tok::at:
    if (getLangOpts().HLSL) goto dont_know; // HLSL Change - skip unsupported processing
    return ParseObjCAtDirectives();
  case tok::minus:
  case tok::plus:
    if (!getLangOpts().ObjC1) {
      Diag(Tok, diag::err_expected_external_declaration);
      ConsumeToken();
      return DeclGroupPtrTy();
    }
    SingleDecl = ParseObjCMethodDefinition();
    break;
  case tok::code_completion:
      Actions.CodeCompleteOrdinaryName(getCurScope(), 
                             CurParsedObjCImpl? Sema::PCC_ObjCImplementation
                                              : Sema::PCC_Namespace);
    cutOffParsing();
    return DeclGroupPtrTy();
  // HLSL Change Starts: Ignore shared keyword for now
  case tok::kw_shared:
      ConsumeToken();
      return ParseExternalDeclaration(attrs);
  // HLSL Change Ends
  // HLSL Change Starts: Start parsing declaration of cbuffer and tbuffers
  case tok::kw_cbuffer:
  case tok::kw_tbuffer:
    // HLSL Change Ends
  case tok::kw_using:
  case tok::kw_namespace:
  case tok::kw_typedef:
  case tok::kw_template:
  case tok::kw_export:    // As in 'export template'
  case tok::kw_static_assert:
  case tok::kw__Static_assert:
    // A function definition cannot start with any of these keywords.
    {
      SourceLocation DeclEnd;
      return ParseDeclaration(Declarator::FileContext, DeclEnd, attrs);
    }

  case tok::kw_static:
    // Parse (then ignore) 'static' prior to a template instantiation. This is
    // a GCC extension that we intentionally do not support.
    if (getLangOpts().CPlusPlus && NextToken().is(tok::kw_template)) {
      Diag(ConsumeToken(), diag::warn_static_inline_explicit_inst_ignored)
        << 0;
      SourceLocation DeclEnd;
      return ParseDeclaration(Declarator::FileContext, DeclEnd, attrs);
    }
    goto dont_know;
      
  case tok::kw_inline:
    if (getLangOpts().CPlusPlus) {
      tok::TokenKind NextKind = NextToken().getKind();
      
      // Inline namespaces. Allowed as an extension even in C++03.
      if (NextKind == tok::kw_namespace) {
        SourceLocation DeclEnd;
        return ParseDeclaration(Declarator::FileContext, DeclEnd, attrs);
      }
      
      // Parse (then ignore) 'inline' prior to a template instantiation. This is
      // a GCC extension that we intentionally do not support.
      if (NextKind == tok::kw_template) {
        Diag(ConsumeToken(), diag::warn_static_inline_explicit_inst_ignored)
          << 1;
        SourceLocation DeclEnd;
        return ParseDeclaration(Declarator::FileContext, DeclEnd, attrs);
      }
    }
    goto dont_know;

  case tok::kw_extern:
    if (getLangOpts().CPlusPlus && NextToken().is(tok::kw_template) && !getLangOpts().HLSL) { // HLSL Change
      // Extern templates
      SourceLocation ExternLoc = ConsumeToken();
      SourceLocation TemplateLoc = ConsumeToken();
      Diag(ExternLoc, getLangOpts().CPlusPlus11 ?
             diag::warn_cxx98_compat_extern_template :
             diag::ext_extern_template) << SourceRange(ExternLoc, TemplateLoc);
      SourceLocation DeclEnd;
      return Actions.ConvertDeclToDeclGroup(
                  ParseExplicitInstantiation(Declarator::FileContext,
                                             ExternLoc, TemplateLoc, DeclEnd));
    }
    goto dont_know;

  case tok::kw___if_exists:
  case tok::kw___if_not_exists:
    if (getLangOpts().HLSL) goto dont_know; // HLSL Change - skip unsupported processing
    ParseMicrosoftIfExistsExternalDeclaration();
    return DeclGroupPtrTy();

  default:
  dont_know:
    // We can't tell whether this is a function-definition or declaration yet.
    return ParseDeclarationOrFunctionDefinition(attrs, DS);
  }

  // This routine returns a DeclGroup, if the thing we parsed only contains a
  // single decl, convert it now.
  return Actions.ConvertDeclToDeclGroup(SingleDecl);
}

/// \brief Determine whether the current token, if it occurs after a
/// declarator, continues a declaration or declaration list.
bool Parser::isDeclarationAfterDeclarator() {
  // Check for '= delete' or '= default'
  if (!getLangOpts().HLSL && (getLangOpts().CPlusPlus && Tok.is(tok::equal))) { // HLSL Change - remove support
    const Token &KW = NextToken();
    if (KW.is(tok::kw_default) || KW.is(tok::kw_delete))
      return false;
  }
  
  return Tok.is(tok::equal) ||      // int X()=  -> not a function def
    Tok.is(tok::comma) ||           // int X(),  -> not a function def
    Tok.is(tok::semi)  ||           // int X();  -> not a function def
    Tok.is(tok::kw_asm) ||          // int X() __asm__ -> not a function def
    Tok.is(tok::kw___attribute) ||  // int X() __attr__ -> not a function def
    (getLangOpts().CPlusPlus &&
     Tok.is(tok::l_paren));         // int X(0) -> not a function def [C++]
}

/// \brief Determine whether the current token, if it occurs after a
/// declarator, indicates the start of a function definition.
bool Parser::isStartOfFunctionDefinition(const ParsingDeclarator &Declarator) {
  assert(Declarator.isFunctionDeclarator() && "Isn't a function declarator");
  if (Tok.is(tok::l_brace))   // int X() {}
    return true;
  
  // Handle K&R C argument lists: int X(f) int f; {}
  if (!getLangOpts().CPlusPlus &&
      Declarator.getFunctionTypeInfo().isKNRPrototype()) 
    return isDeclarationSpecifier();

  if (!getLangOpts().HLSL && getLangOpts().CPlusPlus && Tok.is(tok::equal)) { // HLSL Change - remove support
    const Token &KW = NextToken();
    return KW.is(tok::kw_default) || KW.is(tok::kw_delete);
  }
  
  return Tok.is(tok::colon) ||         // X() : Base() {} (used for ctors)
         Tok.is(tok::kw_try);          // X() try { ... }
}

/// ParseDeclarationOrFunctionDefinition - Parse either a function-definition or
/// a declaration.  We can't tell which we have until we read up to the
/// compound-statement in function-definition. TemplateParams, if
/// non-NULL, provides the template parameters when we're parsing a
/// C++ template-declaration.
///
///       function-definition: [C99 6.9.1]
///         decl-specs      declarator declaration-list[opt] compound-statement
/// [C90] function-definition: [C99 6.7.1] - implicit int result
/// [C90]   decl-specs[opt] declarator declaration-list[opt] compound-statement
///
///       declaration: [C99 6.7]
///         declaration-specifiers init-declarator-list[opt] ';'
/// [!C99]  init-declarator-list ';'                   [TODO: warn in c99 mode]
/// [OMP]   threadprivate-directive                              [TODO]
///
Parser::DeclGroupPtrTy
Parser::ParseDeclOrFunctionDefInternal(ParsedAttributesWithRange &attrs,
                                       ParsingDeclSpec &DS,
                                       AccessSpecifier AS) {
  // Parse the common declaration-specifiers piece.
  ParseDeclarationSpecifiers(DS, ParsedTemplateInfo(), AS, DSC_top_level);

  // If we had a free-standing type definition with a missing semicolon, we
  // may get this far before the problem becomes obvious.
  if (DS.hasTagDefinition() &&
      DiagnoseMissingSemiAfterTagDefinition(DS, AS, DSC_top_level))
    return DeclGroupPtrTy();

  // C99 6.7.2.3p6: Handle "struct-or-union identifier;", "enum { X };"
  // declaration-specifiers init-declarator-list[opt] ';'
  if (Tok.is(tok::semi)) {
    ProhibitAttributes(attrs);
    ConsumeToken();
    Decl *TheDecl = Actions.ParsedFreeStandingDeclSpec(getCurScope(), AS, DS);
    DS.complete(TheDecl);
    return Actions.ConvertDeclToDeclGroup(TheDecl);
  }

  DS.takeAttributesFrom(attrs);

  // ObjC2 allows prefix attributes on class interfaces and protocols.
  // FIXME: This still needs better diagnostics. We should only accept
  // attributes here, no types, etc.
  if (getLangOpts().ObjC2 && Tok.is(tok::at)) {
    SourceLocation AtLoc = ConsumeToken(); // the "@"
    if (!Tok.isObjCAtKeyword(tok::objc_interface) &&
        !Tok.isObjCAtKeyword(tok::objc_protocol)) {
      Diag(Tok, diag::err_objc_unexpected_attr);
      SkipUntil(tok::semi); // FIXME: better skip?
      return DeclGroupPtrTy();
    }

    DS.abort();

    const char *PrevSpec = nullptr;
    unsigned DiagID;
    if (DS.SetTypeSpecType(DeclSpec::TST_unspecified, AtLoc, PrevSpec, DiagID,
                           Actions.getASTContext().getPrintingPolicy()))
      Diag(AtLoc, DiagID) << PrevSpec;

    if (Tok.isObjCAtKeyword(tok::objc_protocol))
      return ParseObjCAtProtocolDeclaration(AtLoc, DS.getAttributes());

    return Actions.ConvertDeclToDeclGroup(
            ParseObjCAtInterfaceDeclaration(AtLoc, DS.getAttributes()));
  }

  // If the declspec consisted only of 'extern' and we have a string
  // literal following it, this must be a C++ linkage specifier like
  // 'extern "C"'.
  if (!getLangOpts().HLSL && getLangOpts().CPlusPlus && isTokenStringLiteral() && // HLSL Change - disallow extern "C"
      DS.getStorageClassSpec() == DeclSpec::SCS_extern &&
      DS.getParsedSpecifiers() == DeclSpec::PQ_StorageClassSpecifier) {
    Decl *TheDecl = ParseLinkage(DS, Declarator::FileContext);
    return Actions.ConvertDeclToDeclGroup(TheDecl);
  }

  return ParseDeclGroup(DS, Declarator::FileContext);
}

Parser::DeclGroupPtrTy
Parser::ParseDeclarationOrFunctionDefinition(ParsedAttributesWithRange &attrs,
                                             ParsingDeclSpec *DS,
                                             AccessSpecifier AS) {
  if (DS) {
    return ParseDeclOrFunctionDefInternal(attrs, *DS, AS);
  } else {
    ParsingDeclSpec PDS(*this);
    // Must temporarily exit the objective-c container scope for
    // parsing c constructs and re-enter objc container scope
    // afterwards.
    ObjCDeclContextSwitch ObjCDC(*this);
      
    return ParseDeclOrFunctionDefInternal(attrs, PDS, AS);
  }
}

/// ParseFunctionDefinition - We parsed and verified that the specified
/// Declarator is well formed.  If this is a K&R-style function, read the
/// parameters declaration-list, then start the compound-statement.
///
///       function-definition: [C99 6.9.1]
///         decl-specs      declarator declaration-list[opt] compound-statement
/// [C90] function-definition: [C99 6.7.1] - implicit int result
/// [C90]   decl-specs[opt] declarator declaration-list[opt] compound-statement
/// [C++] function-definition: [C++ 8.4]
///         decl-specifier-seq[opt] declarator ctor-initializer[opt]
///         function-body
/// [C++] function-definition: [C++ 8.4]
///         decl-specifier-seq[opt] declarator function-try-block
///
Decl *Parser::ParseFunctionDefinition(ParsingDeclarator &D,
                                      const ParsedTemplateInfo &TemplateInfo,
                                      LateParsedAttrList *LateParsedAttrs) {
  // Poison SEH identifiers so they are flagged as illegal in function bodies.
  PoisonSEHIdentifiersRAIIObject PoisonSEHIdentifiers(*this, true);
  const DeclaratorChunk::FunctionTypeInfo &FTI = D.getFunctionTypeInfo();

  // If this is C90 and the declspecs were completely missing, fudge in an
  // implicit int.  We do this here because this is the only place where
  // declaration-specifiers are completely optional in the grammar.
  if (getLangOpts().ImplicitInt && D.getDeclSpec().isEmpty()) {
    const char *PrevSpec;
    unsigned DiagID;
    const PrintingPolicy &Policy = Actions.getASTContext().getPrintingPolicy();
    D.getMutableDeclSpec().SetTypeSpecType(DeclSpec::TST_int,
                                           D.getIdentifierLoc(),
                                           PrevSpec, DiagID,
                                           Policy);
    D.SetRangeBegin(D.getDeclSpec().getSourceRange().getBegin());
  }

  // If this declaration was formed with a K&R-style identifier list for the
  // arguments, parse declarations for all of the args next.
  // int foo(a,b) int a; float b; {}
  if (!getLangOpts().HLSL && FTI.isKNRPrototype()) // HLSL Change - there is no support for K&R-style functions
    ParseKNRParamDeclarations(D);

  // We should have either an opening brace or, in a C++ constructor,
  // we may have a colon.
  if (Tok.isNot(tok::l_brace) && 
      ((getLangOpts().HLSL && Tok.isNot(tok::colon)) || // HLSL Change - for HLSL, only allow ':', not try or =
      (!getLangOpts().CPlusPlus ||
       (Tok.isNot(tok::colon) && Tok.isNot(tok::kw_try) &&
        Tok.isNot(tok::equal))))) {
    Diag(Tok, diag::err_expected_fn_body);

    // Skip over garbage, until we get to '{'.  Don't eat the '{'.
    SkipUntil(tok::l_brace, StopAtSemi | StopBeforeMatch);

    // If we didn't find the '{', bail out.
    if (Tok.isNot(tok::l_brace))
      return nullptr;
  }

  // Check to make sure that any normal attributes are allowed to be on
  // a definition.  Late parsed attributes are checked at the end.
  if (Tok.isNot(tok::equal)) {
    AttributeList *DtorAttrs = D.getAttributes();
    while (DtorAttrs) {
      if (DtorAttrs->isKnownToGCC() &&
          !DtorAttrs->isCXX11Attribute()) {
        Diag(DtorAttrs->getLoc(), diag::warn_attribute_on_function_definition)
          << DtorAttrs->getName();
      }
      DtorAttrs = DtorAttrs->getNext();
    }
  }

  // In delayed template parsing mode, for function template we consume the
  // tokens and store them for late parsing at the end of the translation unit.
  if (getLangOpts().DelayedTemplateParsing && Tok.isNot(tok::equal) &&
      TemplateInfo.Kind == ParsedTemplateInfo::Template &&
      Actions.canDelayFunctionBody(D)) {
    MultiTemplateParamsArg TemplateParameterLists(*TemplateInfo.TemplateParams);
    
    ParseScope BodyScope(this, Scope::FnScope|Scope::DeclScope);
    Scope *ParentScope = getCurScope()->getParent();

    D.setFunctionDefinitionKind(FDK_Definition);
    Decl *DP = Actions.HandleDeclarator(ParentScope, D,
                                        TemplateParameterLists);
    D.complete(DP);
    D.getMutableDeclSpec().abort();

    CachedTokens Toks;
    LexTemplateFunctionForLateParsing(Toks);

    if (DP) {
      FunctionDecl *FnD = DP->getAsFunction();
      Actions.CheckForFunctionRedefinition(FnD);
      Actions.MarkAsLateParsedTemplate(FnD, DP, Toks);
    }
    return DP;
  }
  else if (CurParsedObjCImpl && !getLangOpts().HLSL && // HLSL Change - remove support for Obj-C parsing 
           !TemplateInfo.TemplateParams &&
           (Tok.is(tok::l_brace) || Tok.is(tok::kw_try) ||
            Tok.is(tok::colon)) && 
      Actions.CurContext->isTranslationUnit()) {
    ParseScope BodyScope(this, Scope::FnScope|Scope::DeclScope);
    Scope *ParentScope = getCurScope()->getParent();

    D.setFunctionDefinitionKind(FDK_Definition);
    Decl *FuncDecl = Actions.HandleDeclarator(ParentScope, D,
                                              MultiTemplateParamsArg());
    D.complete(FuncDecl);
    D.getMutableDeclSpec().abort();
    if (FuncDecl) {
      // Consume the tokens and store them for later parsing.
      StashAwayMethodOrFunctionBodyTokens(FuncDecl);
      CurParsedObjCImpl->HasCFunction = true;
      return FuncDecl;
    }
    // FIXME: Should we really fall through here?
  }

  // Enter a scope for the function body.
  ParseScope BodyScope(this, Scope::FnScope|Scope::DeclScope);

  // Tell the actions module that we have entered a function definition with the
  // specified Declarator for the function.
  Decl *Res = TemplateInfo.TemplateParams?
      Actions.ActOnStartOfFunctionTemplateDef(getCurScope(),
                                              *TemplateInfo.TemplateParams, D)
    : Actions.ActOnStartOfFunctionDef(getCurScope(), D);

  // Break out of the ParsingDeclarator context before we parse the body.
  D.complete(Res);
  
  // Break out of the ParsingDeclSpec context, too.  This const_cast is
  // safe because we're always the sole owner.
  D.getMutableDeclSpec().abort();

  if (!getLangOpts().HLSL && TryConsumeToken(tok::equal)) { // HLSL Change: this has already been rejected in this function
    assert(getLangOpts().CPlusPlus && "Only C++ function definitions have '='");

    bool Delete = false;
    SourceLocation KWLoc;
    if (TryConsumeToken(tok::kw_delete, KWLoc)) {
      Diag(KWLoc, getLangOpts().CPlusPlus11
                      ? diag::warn_cxx98_compat_deleted_function
                      : diag::ext_deleted_function);
      Actions.SetDeclDeleted(Res, KWLoc);
      Delete = true;
    } else if (TryConsumeToken(tok::kw_default, KWLoc)) {
      Diag(KWLoc, getLangOpts().CPlusPlus11
                      ? diag::warn_cxx98_compat_defaulted_function
                      : diag::ext_defaulted_function);
      Actions.SetDeclDefaulted(Res, KWLoc);
    } else {
      llvm_unreachable("function definition after = not 'delete' or 'default'");
    }

    if (Tok.is(tok::comma)) {
      Diag(KWLoc, diag::err_default_delete_in_multiple_declaration)
        << Delete;
      SkipUntil(tok::semi);
    } else if (ExpectAndConsume(tok::semi, diag::err_expected_after,
                                Delete ? "delete" : "default")) {
      SkipUntil(tok::semi);
    }

    Stmt *GeneratedBody = Res ? Res->getBody() : nullptr;
    Actions.ActOnFinishFunctionBody(Res, GeneratedBody, false);
    return Res;
  }

  if (Tok.is(tok::kw_try) && !getLangOpts().HLSL)  // HLSL Change: this has already been rejected in this function
    return ParseFunctionTryBlock(Res, BodyScope);

  // If we have a colon, then we're probably parsing a C++
  // ctor-initializer.
  if (Tok.is(tok::colon)) {
    ParseConstructorInitializer(Res);

    // Recover from error.
    if (!Tok.is(tok::l_brace)) {
      BodyScope.Exit();
      Actions.ActOnFinishFunctionBody(Res, nullptr);
      return Res;
    }
  } else
    Actions.ActOnDefaultCtorInitializers(Res);

  // Late attributes are parsed in the same scope as the function body.
  if (LateParsedAttrs)
    ParseLexedAttributeList(*LateParsedAttrs, Res, false, true);

  return ParseFunctionStatementBody(Res, BodyScope);
}

/// ParseKNRParamDeclarations - Parse 'declaration-list[opt]' which provides
/// types for a function with a K&R-style identifier list for arguments.
void Parser::ParseKNRParamDeclarations(Declarator &D) {
  // We know that the top-level of this declarator is a function.
  DeclaratorChunk::FunctionTypeInfo &FTI = D.getFunctionTypeInfo();

  // Enter function-declaration scope, limiting any declarators to the
  // function prototype scope, including parameter declarators.
  ParseScope PrototypeScope(this, Scope::FunctionPrototypeScope |
                            Scope::FunctionDeclarationScope | Scope::DeclScope);

  // Read all the argument declarations.
  while (isDeclarationSpecifier()) {
    SourceLocation DSStart = Tok.getLocation();

    // Parse the common declaration-specifiers piece.
    DeclSpec DS(AttrFactory);
    ParseDeclarationSpecifiers(DS);

    // C99 6.9.1p6: 'each declaration in the declaration list shall have at
    // least one declarator'.
    // NOTE: GCC just makes this an ext-warn.  It's not clear what it does with
    // the declarations though.  It's trivial to ignore them, really hard to do
    // anything else with them.
    if (TryConsumeToken(tok::semi)) {
      Diag(DSStart, diag::err_declaration_does_not_declare_param);
      continue;
    }

    // C99 6.9.1p6: Declarations shall contain no storage-class specifiers other
    // than register.
    if (DS.getStorageClassSpec() != DeclSpec::SCS_unspecified &&
        DS.getStorageClassSpec() != DeclSpec::SCS_register) {
      Diag(DS.getStorageClassSpecLoc(),
           diag::err_invalid_storage_class_in_func_decl);
      DS.ClearStorageClassSpecs();
    }
    if (DS.getThreadStorageClassSpec() != DeclSpec::TSCS_unspecified) {
      Diag(DS.getThreadStorageClassSpecLoc(),
           diag::err_invalid_storage_class_in_func_decl);
      DS.ClearStorageClassSpecs();
    }

    // Parse the first declarator attached to this declspec.
    Declarator ParmDeclarator(DS, Declarator::KNRTypeListContext);
    ParseDeclarator(ParmDeclarator);

    // Handle the full declarator list.
    while (1) {
      // If attributes are present, parse them.
      MaybeParseGNUAttributes(ParmDeclarator);

      // Ask the actions module to compute the type for this declarator.
      Decl *Param =
        Actions.ActOnParamDeclarator(getCurScope(), ParmDeclarator);

      if (Param &&
          // A missing identifier has already been diagnosed.
          ParmDeclarator.getIdentifier()) {

        // Scan the argument list looking for the correct param to apply this
        // type.
        for (unsigned i = 0; ; ++i) {
          // C99 6.9.1p6: those declarators shall declare only identifiers from
          // the identifier list.
          if (i == FTI.NumParams) {
            Diag(ParmDeclarator.getIdentifierLoc(), diag::err_no_matching_param)
              << ParmDeclarator.getIdentifier();
            break;
          }

          if (FTI.Params[i].Ident == ParmDeclarator.getIdentifier()) {
            // Reject redefinitions of parameters.
            if (FTI.Params[i].Param) {
              Diag(ParmDeclarator.getIdentifierLoc(),
                   diag::err_param_redefinition)
                 << ParmDeclarator.getIdentifier();
            } else {
              FTI.Params[i].Param = Param;
            }
            break;
          }
        }
      }

      // If we don't have a comma, it is either the end of the list (a ';') or
      // an error, bail out.
      if (Tok.isNot(tok::comma))
        break;

      ParmDeclarator.clear();

      // Consume the comma.
      ParmDeclarator.setCommaLoc(ConsumeToken());

      // Parse the next declarator.
      ParseDeclarator(ParmDeclarator);
    }

    // Consume ';' and continue parsing.
    if (!ExpectAndConsumeSemi(diag::err_expected_semi_declaration))
      continue;

    // Otherwise recover by skipping to next semi or mandatory function body.
    if (SkipUntil(tok::l_brace, StopAtSemi | StopBeforeMatch))
      break;
    TryConsumeToken(tok::semi);
  }

  // The actions module must verify that all arguments were declared.
  Actions.ActOnFinishKNRParamDeclarations(getCurScope(), D, Tok.getLocation());
}


/// ParseAsmStringLiteral - This is just a normal string-literal, but is not
/// allowed to be a wide string, and is not subject to character translation.
///
/// [GNU] asm-string-literal:
///         string-literal
///
ExprResult Parser::ParseAsmStringLiteral() {
  assert(!getLangOpts().HLSL); // HLSL Change - this point should be unreachable
  if (!isTokenStringLiteral()) {
    Diag(Tok, diag::err_expected_string_literal)
      << /*Source='in...'*/0 << "'asm'";
    return ExprError();
  }

  ExprResult AsmString(ParseStringLiteralExpression());
  if (!AsmString.isInvalid()) {
    const auto *SL = cast<StringLiteral>(AsmString.get());
    if (!SL->isAscii()) {
      Diag(Tok, diag::err_asm_operand_wide_string_literal)
        << SL->isWide()
        << SL->getSourceRange();
      return ExprError();
    }
  }
  return AsmString;
}

/// ParseSimpleAsm
///
/// [GNU] simple-asm-expr:
///         'asm' '(' asm-string-literal ')'
///
ExprResult Parser::ParseSimpleAsm(SourceLocation *EndLoc) {
  assert(!getLangOpts().HLSL); // HLSL Change - this point should be unreachable
  assert(Tok.is(tok::kw_asm) && "Not an asm!");
  SourceLocation Loc = ConsumeToken();

  if (Tok.is(tok::kw_volatile)) {
    // Remove from the end of 'asm' to the end of 'volatile'.
    SourceRange RemovalRange(PP.getLocForEndOfToken(Loc),
                             PP.getLocForEndOfToken(Tok.getLocation()));

    Diag(Tok, diag::warn_file_asm_volatile)
      << FixItHint::CreateRemoval(RemovalRange);
    ConsumeToken();
  }

  BalancedDelimiterTracker T(*this, tok::l_paren);
  if (T.consumeOpen()) {
    Diag(Tok, diag::err_expected_lparen_after) << "asm";
    return ExprError();
  }

  ExprResult Result(ParseAsmStringLiteral());

  if (!Result.isInvalid()) {
    // Close the paren and get the location of the end bracket
    T.consumeClose();
    if (EndLoc)
      *EndLoc = T.getCloseLocation();
  } else if (SkipUntil(tok::r_paren, StopAtSemi | StopBeforeMatch)) {
    if (EndLoc)
      *EndLoc = Tok.getLocation();
    ConsumeParen();
  }

  return Result;
}

/// \brief Get the TemplateIdAnnotation from the token and put it in the
/// cleanup pool so that it gets destroyed when parsing the current top level
/// declaration is finished.
TemplateIdAnnotation *Parser::takeTemplateIdAnnotation(const Token &tok) {
  assert(tok.is(tok::annot_template_id) && "Expected template-id token");
  TemplateIdAnnotation *
      Id = static_cast<TemplateIdAnnotation *>(tok.getAnnotationValue());
  return Id;
}

void Parser::AnnotateScopeToken(CXXScopeSpec &SS, bool IsNewAnnotation) {
  // Push the current token back into the token stream (or revert it if it is
  // cached) and use an annotation scope token for current token.
  if (PP.isBacktrackEnabled())
    PP.RevertCachedTokens(1);
  else
    PP.EnterToken(Tok);
  Tok.setKind(tok::annot_cxxscope);
  Tok.setAnnotationValue(Actions.SaveNestedNameSpecifierAnnotation(SS));
  Tok.setAnnotationRange(SS.getRange());

  // In case the tokens were cached, have Preprocessor replace them
  // with the annotation token.  We don't need to do this if we've
  // just reverted back to a prior state.
  if (IsNewAnnotation)
    PP.AnnotateCachedTokens(Tok);
}

/// \brief Attempt to classify the name at the current token position. This may
/// form a type, scope or primary expression annotation, or replace the token
/// with a typo-corrected keyword. This is only appropriate when the current
/// name must refer to an entity which has already been declared.
///
/// \param IsAddressOfOperand Must be \c true if the name is preceded by an '&'
///        and might possibly have a dependent nested name specifier.
/// \param CCC Indicates how to perform typo-correction for this name. If NULL,
///        no typo correction will be performed.
Parser::AnnotatedNameKind
Parser::TryAnnotateName(bool IsAddressOfOperand,
                        std::unique_ptr<CorrectionCandidateCallback> CCC) {
  assert(Tok.is(tok::identifier) || Tok.is(tok::annot_cxxscope));

  const bool EnteringContext = false;
  const bool WasScopeAnnotation = Tok.is(tok::annot_cxxscope);

  CXXScopeSpec SS;
  if (getLangOpts().CPlusPlus &&
      ParseOptionalCXXScopeSpecifier(SS, ParsedType(), EnteringContext))
    return ANK_Error;

  if (Tok.isNot(tok::identifier) || SS.isInvalid()) {
    if (TryAnnotateTypeOrScopeTokenAfterScopeSpec(EnteringContext, false, SS,
                                                  !WasScopeAnnotation))
      return ANK_Error;
    return ANK_Unresolved;
  }

  IdentifierInfo *Name = Tok.getIdentifierInfo();
  SourceLocation NameLoc = Tok.getLocation();

  // FIXME: Move the tentative declaration logic into ClassifyName so we can
  // typo-correct to tentatively-declared identifiers.
  if (isTentativelyDeclared(Name)) {
    // Identifier has been tentatively declared, and thus cannot be resolved as
    // an expression. Fall back to annotating it as a type.
    if (TryAnnotateTypeOrScopeTokenAfterScopeSpec(EnteringContext, false, SS,
                                                  !WasScopeAnnotation))
      return ANK_Error;
    return Tok.is(tok::annot_typename) ? ANK_Success : ANK_TentativeDecl;
  }

  Token Next = NextToken();

  // Look up and classify the identifier. We don't perform any typo-correction
  // after a scope specifier, because in general we can't recover from typos
  // there (eg, after correcting 'A::tempalte B<X>::C' [sic], we would need to
  // jump back into scope specifier parsing).
  Sema::NameClassification Classification = Actions.ClassifyName(
      getCurScope(), SS, Name, NameLoc, Next, IsAddressOfOperand,
      SS.isEmpty() ? std::move(CCC) : nullptr);

  switch (Classification.getKind()) {
  case Sema::NC_Error:
    return ANK_Error;

  case Sema::NC_Keyword:
    // The identifier was typo-corrected to a keyword.
    Tok.setIdentifierInfo(Name);
    Tok.setKind(Name->getTokenID());
    PP.TypoCorrectToken(Tok);
    if (SS.isNotEmpty())
      AnnotateScopeToken(SS, !WasScopeAnnotation);
    // We've "annotated" this as a keyword.
    return ANK_Success;

  case Sema::NC_Unknown:
    // It's not something we know about. Leave it unannotated.
    break;

  case Sema::NC_Type: {
    SourceLocation BeginLoc = NameLoc;
    if (SS.isNotEmpty())
      BeginLoc = SS.getBeginLoc();

    /// An Objective-C object type followed by '<' is a specialization of
    /// a parameterized class type or a protocol-qualified type.
    ParsedType Ty = Classification.getType();
    if (getLangOpts().ObjC1 && NextToken().is(tok::less) &&
        (Ty.get()->isObjCObjectType() ||
         Ty.get()->isObjCObjectPointerType())) {
      // Consume the name.
      SourceLocation IdentifierLoc = ConsumeToken();
      SourceLocation NewEndLoc;
      TypeResult NewType
          = parseObjCTypeArgsAndProtocolQualifiers(IdentifierLoc, Ty,
                                                   /*consumeLastToken=*/false,
                                                   NewEndLoc);
      if (NewType.isUsable())
        Ty = NewType.get();
    }

    Tok.setKind(tok::annot_typename);
    setTypeAnnotation(Tok, Ty);
    Tok.setAnnotationEndLoc(Tok.getLocation());
    Tok.setLocation(BeginLoc);
    PP.AnnotateCachedTokens(Tok);
    return ANK_Success;
  }

  case Sema::NC_Expression:
    Tok.setKind(tok::annot_primary_expr);
    setExprAnnotation(Tok, Classification.getExpression());
    Tok.setAnnotationEndLoc(NameLoc);
    if (SS.isNotEmpty())
      Tok.setLocation(SS.getBeginLoc());
    PP.AnnotateCachedTokens(Tok);
    return ANK_Success;

  case Sema::NC_TypeTemplate:
    if (Next.isNot(tok::less)) {
      // This may be a type template being used as a template template argument.
      if (SS.isNotEmpty())
        AnnotateScopeToken(SS, !WasScopeAnnotation);
      return ANK_TemplateName;
    }
    LLVM_FALLTHROUGH; // HLSL Change
  case Sema::NC_VarTemplate:
  case Sema::NC_FunctionTemplate: {
    // We have a type, variable or function template followed by '<'.
    ConsumeToken();
    UnqualifiedId Id;
    Id.setIdentifier(Name, NameLoc);
    if (AnnotateTemplateIdToken(
            TemplateTy::make(Classification.getTemplateName()),
            Classification.getTemplateNameKind(), SS, SourceLocation(), Id))
      return ANK_Error;
    return ANK_Success;
  }

  case Sema::NC_NestedNameSpecifier:
    llvm_unreachable("already parsed nested name specifier");
  }

  // Unable to classify the name, but maybe we can annotate a scope specifier.
  if (SS.isNotEmpty())
    AnnotateScopeToken(SS, !WasScopeAnnotation);
  return ANK_Unresolved;
}

bool Parser::TryKeywordIdentFallback(bool DisableKeyword) {
  assert(Tok.isNot(tok::identifier));
  Diag(Tok, diag::ext_keyword_as_ident)
    << PP.getSpelling(Tok)
    << DisableKeyword;
  if (DisableKeyword)
    Tok.getIdentifierInfo()->RevertTokenIDToIdentifier();
  Tok.setKind(tok::identifier);
  return true;
}

/// TryAnnotateTypeOrScopeToken - If the current token position is on a
/// typename (possibly qualified in C++) or a C++ scope specifier not followed
/// by a typename, TryAnnotateTypeOrScopeToken will replace one or more tokens
/// with a single annotation token representing the typename or C++ scope
/// respectively.
/// This simplifies handling of C++ scope specifiers and allows efficient
/// backtracking without the need to re-parse and resolve nested-names and
/// typenames.
/// It will mainly be called when we expect to treat identifiers as typenames
/// (if they are typenames). For example, in C we do not expect identifiers
/// inside expressions to be treated as typenames so it will not be called
/// for expressions in C.
/// The benefit for C/ObjC is that a typename will be annotated and
/// Actions.getTypeName will not be needed to be called again (e.g. getTypeName
/// will not be called twice, once to check whether we have a declaration
/// specifier, and another one to get the actual type inside
/// ParseDeclarationSpecifiers).
///
/// This returns true if an error occurred.
///
/// Note that this routine emits an error if you call it with ::new or ::delete
/// as the current tokens, so only call it in contexts where these are invalid.
bool Parser::TryAnnotateTypeOrScopeToken(bool EnteringContext, bool NeedType) {
  assert((Tok.is(tok::identifier) || Tok.is(tok::coloncolon) ||
          Tok.is(tok::kw_typename) || Tok.is(tok::annot_cxxscope) ||
          Tok.is(tok::kw_decltype) || Tok.is(tok::annot_template_id) ||
          Tok.is(tok::kw___super)) &&
         "Cannot be a type or scope token!");

  if (Tok.is(tok::kw_typename)) {
    // MSVC lets you do stuff like:
    //   typename typedef T_::D D;
    //
    // We will consume the typedef token here and put it back after we have
    // parsed the first identifier, transforming it into something more like:
    //   typename T_::D typedef D;
    if (getLangOpts().MSVCCompat && NextToken().is(tok::kw_typedef)) {
      Token TypedefToken;
      PP.Lex(TypedefToken);
      bool Result = TryAnnotateTypeOrScopeToken(EnteringContext, NeedType);
      PP.EnterToken(Tok);
      Tok = TypedefToken;
      if (!Result)
        Diag(Tok.getLocation(), diag::warn_expected_qualified_after_typename);
      return Result;
    }

    // Parse a C++ typename-specifier, e.g., "typename T::type".
    //
    //   typename-specifier:
    //     'typename' '::' [opt] nested-name-specifier identifier
    //     'typename' '::' [opt] nested-name-specifier template [opt]
    //            simple-template-id
    SourceLocation TypenameLoc = ConsumeToken();
    CXXScopeSpec SS;
    if (ParseOptionalCXXScopeSpecifier(SS, /*ObjectType=*/ParsedType(), 
                                       /*EnteringContext=*/false,
                                       nullptr, /*IsTypename*/ true))
      return true;
    if (!SS.isSet()) {
      if (Tok.is(tok::identifier) || Tok.is(tok::annot_template_id) ||
          Tok.is(tok::annot_decltype)) {
        // Attempt to recover by skipping the invalid 'typename'
        if (Tok.is(tok::annot_decltype) ||
            (!TryAnnotateTypeOrScopeToken(EnteringContext, NeedType) &&
             Tok.isAnnotation())) {
          unsigned DiagID = diag::err_expected_qualified_after_typename;
          // MS compatibility: MSVC permits using known types with typename.
          // e.g. "typedef typename T* pointer_type"
          if (getLangOpts().MicrosoftExt)
            DiagID = diag::warn_expected_qualified_after_typename;
          Diag(Tok.getLocation(), DiagID);
          return false;
        }
      }

      Diag(Tok.getLocation(), diag::err_expected_qualified_after_typename);
      return true;
    }

    TypeResult Ty;
    if (Tok.is(tok::identifier)) {
      // FIXME: check whether the next token is '<', first!
      Ty = Actions.ActOnTypenameType(getCurScope(), TypenameLoc, SS, 
                                     *Tok.getIdentifierInfo(),
                                     Tok.getLocation());
    } else if (Tok.is(tok::annot_template_id)) {
      TemplateIdAnnotation *TemplateId = takeTemplateIdAnnotation(Tok);
      if (TemplateId->Kind != TNK_Type_template &&
          TemplateId->Kind != TNK_Dependent_template_name) {
        Diag(Tok, diag::err_typename_refers_to_non_type_template)
          << Tok.getAnnotationRange();
        return true;
      }

      ASTTemplateArgsPtr TemplateArgsPtr(TemplateId->getTemplateArgs(),
                                         TemplateId->NumArgs);

      Ty = Actions.ActOnTypenameType(getCurScope(), TypenameLoc, SS,
                                     TemplateId->TemplateKWLoc,
                                     TemplateId->Template,
                                     TemplateId->TemplateNameLoc,
                                     TemplateId->LAngleLoc,
                                     TemplateArgsPtr,
                                     TemplateId->RAngleLoc);
    } else {
      Diag(Tok, diag::err_expected_type_name_after_typename)
        << SS.getRange();
      return true;
    }

    SourceLocation EndLoc = Tok.getLastLoc();
    Tok.setKind(tok::annot_typename);
    setTypeAnnotation(Tok, Ty.isInvalid() ? ParsedType() : Ty.get());
    Tok.setAnnotationEndLoc(EndLoc);
    Tok.setLocation(TypenameLoc);
    PP.AnnotateCachedTokens(Tok);
    return false;
  }

  // Remembers whether the token was originally a scope annotation.
  bool WasScopeAnnotation = Tok.is(tok::annot_cxxscope);

  CXXScopeSpec SS;
  if (getLangOpts().CPlusPlus)  // HLSL Note - allowing scope qualification
    if (ParseOptionalCXXScopeSpecifier(SS, ParsedType(), EnteringContext))
      return true;

  return TryAnnotateTypeOrScopeTokenAfterScopeSpec(EnteringContext, NeedType,
                                                   SS, !WasScopeAnnotation);
}

/// \brief Try to annotate a type or scope token, having already parsed an
/// optional scope specifier. \p IsNewScope should be \c true unless the scope
/// specifier was extracted from an existing tok::annot_cxxscope annotation.
bool Parser::TryAnnotateTypeOrScopeTokenAfterScopeSpec(bool EnteringContext,
                                                       bool NeedType,
                                                       CXXScopeSpec &SS,
                                                       bool IsNewScope) {
  if (Tok.is(tok::identifier)) {
    IdentifierInfo *CorrectedII = nullptr;
    // Determine whether the identifier is a type name.
    if (ParsedType Ty = Actions.getTypeName(*Tok.getIdentifierInfo(),
                                            Tok.getLocation(), getCurScope(),
                                            &SS, false, 
                                            NextToken().is(tok::period),
                                            ParsedType(),
                                            /*IsCtorOrDtorName=*/false,
                                            /*NonTrivialTypeSourceInfo*/ true,
                                            NeedType ? &CorrectedII
                                                     : nullptr)) {
      // A FixIt was applied as a result of typo correction
      if (CorrectedII)
        Tok.setIdentifierInfo(CorrectedII);

      SourceLocation BeginLoc = Tok.getLocation();
      if (SS.isNotEmpty()) // it was a C++ qualified type name.
        BeginLoc = SS.getBeginLoc();

      /// An Objective-C object type followed by '<' is a specialization of
      /// a parameterized class type or a protocol-qualified type.
      if (getLangOpts().ObjC1 && NextToken().is(tok::less) &&
          (Ty.get()->isObjCObjectType() ||
           Ty.get()->isObjCObjectPointerType())) {
        // Consume the name.
        SourceLocation IdentifierLoc = ConsumeToken();
        SourceLocation NewEndLoc;
        TypeResult NewType
          = parseObjCTypeArgsAndProtocolQualifiers(IdentifierLoc, Ty,
                                                   /*consumeLastToken=*/false,
                                                   NewEndLoc);
        if (NewType.isUsable())
          Ty = NewType.get();
      }

      // This is a typename. Replace the current token in-place with an
      // annotation type token.
      Tok.setKind(tok::annot_typename);
      setTypeAnnotation(Tok, Ty);
      Tok.setAnnotationEndLoc(Tok.getLocation());
      Tok.setLocation(BeginLoc);

      // In case the tokens were cached, have Preprocessor replace
      // them with the annotation token.
      PP.AnnotateCachedTokens(Tok);
      return false;
    }

    if (!getLangOpts().CPlusPlus) { // HLSL Note - allow '::' qualification
      // If we're in C, we can't have :: tokens at all (the lexer won't return
      // them).  If the identifier is not a type, then it can't be scope either,
      // just early exit.
      return false;
    }

    // If this is a template-id, annotate with a template-id or type token.
    if (NextToken().is(tok::less)) {
      TemplateTy Template;
      UnqualifiedId TemplateName;
      TemplateName.setIdentifier(Tok.getIdentifierInfo(), Tok.getLocation());
      bool MemberOfUnknownSpecialization;
      if (TemplateNameKind TNK
          = Actions.isTemplateName(getCurScope(), SS,
                                   /*hasTemplateKeyword=*/false, TemplateName,
                                   /*ObjectType=*/ ParsedType(),
                                   EnteringContext,
                                   Template, MemberOfUnknownSpecialization)) {
        // Consume the identifier.
        ConsumeToken();
        if (AnnotateTemplateIdToken(Template, TNK, SS, SourceLocation(),
                                    TemplateName)) {
          // If an unrecoverable error occurred, we need to return true here,
          // because the token stream is in a damaged state.  We may not return
          // a valid identifier.
          return true;
        }
      }
    }

    // The current token, which is either an identifier or a
    // template-id, is not part of the annotation. Fall through to
    // push that token back into the stream and complete the C++ scope
    // specifier annotation.
  }

  if (Tok.is(tok::annot_template_id)) {
    TemplateIdAnnotation *TemplateId = takeTemplateIdAnnotation(Tok);
    if (TemplateId->Kind == TNK_Type_template) {
      // A template-id that refers to a type was parsed into a
      // template-id annotation in a context where we weren't allowed
      // to produce a type annotation token. Update the template-id
      // annotation token to a type annotation token now.
      AnnotateTemplateIdTokenAsType();
      return false;
    }
  }

  if (SS.isEmpty())
    return false;

  // A C++ scope specifier that isn't followed by a typename.
  AnnotateScopeToken(SS, IsNewScope);
  return false;
}

/// TryAnnotateScopeToken - Like TryAnnotateTypeOrScopeToken but only
/// annotates C++ scope specifiers and template-ids.  This returns
/// true if there was an error that could not be recovered from.
///
/// Note that this routine emits an error if you call it with ::new or ::delete
/// as the current tokens, so only call it in contexts where these are invalid.
bool Parser::TryAnnotateCXXScopeToken(bool EnteringContext) {
  assert(getLangOpts().CPlusPlus &&
         "Call sites of this function should be guarded by checking for C++");
  assert((Tok.is(tok::identifier) || Tok.is(tok::coloncolon) ||
          (Tok.is(tok::annot_template_id) && NextToken().is(tok::coloncolon)) ||
          Tok.is(tok::kw_decltype) || Tok.is(tok::kw___super)) &&
         "Cannot be a type or scope token!");

  CXXScopeSpec SS;
  if (ParseOptionalCXXScopeSpecifier(SS, ParsedType(), EnteringContext))
    return true;
  if (SS.isEmpty())
    return false;

  AnnotateScopeToken(SS, true);
  return false;
}

bool Parser::isTokenEqualOrEqualTypo() {
  tok::TokenKind Kind = Tok.getKind();
  switch (Kind) {
  default:
    return false;
  case tok::ampequal:            // &=
  case tok::starequal:           // *=
  case tok::plusequal:           // +=
  case tok::minusequal:          // -=
  case tok::exclaimequal:        // !=
  case tok::slashequal:          // /=
  case tok::percentequal:        // %=
  case tok::lessequal:           // <=
  case tok::lesslessequal:       // <<=
  case tok::greaterequal:        // >=
  case tok::greatergreaterequal: // >>=
  case tok::caretequal:          // ^=
  case tok::pipeequal:           // |=
  case tok::equalequal:          // ==
    Diag(Tok, diag::err_invalid_token_after_declarator_suggest_equal)
        << Kind
        << FixItHint::CreateReplacement(SourceRange(Tok.getLocation()), "=");
    LLVM_FALLTHROUGH; // HLSL Change
  case tok::equal:
    return true;
  }
}

SourceLocation Parser::handleUnexpectedCodeCompletionToken() {
  assert(Tok.is(tok::code_completion));
  PrevTokLocation = Tok.getLocation();

  for (Scope *S = getCurScope(); S; S = S->getParent()) {
    if (S->getFlags() & Scope::FnScope) {
      Actions.CodeCompleteOrdinaryName(getCurScope(),
                                       Sema::PCC_RecoveryInFunction);
      cutOffParsing();
      return PrevTokLocation;
    }
    
    if (S->getFlags() & Scope::ClassScope) {
      Actions.CodeCompleteOrdinaryName(getCurScope(), Sema::PCC_Class);
      cutOffParsing();
      return PrevTokLocation;
    }
  }
  
  Actions.CodeCompleteOrdinaryName(getCurScope(), Sema::PCC_Namespace);
  cutOffParsing();
  return PrevTokLocation;
}

// Code-completion pass-through functions

void Parser::CodeCompleteDirective(bool InConditional) {
  Actions.CodeCompletePreprocessorDirective(InConditional);
}

void Parser::CodeCompleteInConditionalExclusion() {
  Actions.CodeCompleteInPreprocessorConditionalExclusion(getCurScope());
}

void Parser::CodeCompleteMacroName(bool IsDefinition) {
  Actions.CodeCompletePreprocessorMacroName(IsDefinition);
}

void Parser::CodeCompletePreprocessorExpression() { 
  Actions.CodeCompletePreprocessorExpression();
}

void Parser::CodeCompleteMacroArgument(IdentifierInfo *Macro,
                                       MacroInfo *MacroInfo,
                                       unsigned ArgumentIndex) {
  Actions.CodeCompletePreprocessorMacroArgument(getCurScope(), Macro, MacroInfo,
                                                ArgumentIndex);
}

void Parser::CodeCompleteNaturalLanguage() {
  Actions.CodeCompleteNaturalLanguage();
}

bool Parser::ParseMicrosoftIfExistsCondition(IfExistsCondition& Result) {
  assert((Tok.is(tok::kw___if_exists) || Tok.is(tok::kw___if_not_exists)) &&
         "Expected '__if_exists' or '__if_not_exists'");
  Result.IsIfExists = Tok.is(tok::kw___if_exists);
  Result.KeywordLoc = ConsumeToken();

  BalancedDelimiterTracker T(*this, tok::l_paren);
  if (T.consumeOpen()) {
    Diag(Tok, diag::err_expected_lparen_after) 
      << (Result.IsIfExists? "__if_exists" : "__if_not_exists");
    return true;
  }
  
  // Parse nested-name-specifier.
  if (getLangOpts().CPlusPlus)
    ParseOptionalCXXScopeSpecifier(Result.SS, ParsedType(),
                                   /*EnteringContext=*/false);

  // Check nested-name specifier.
  if (Result.SS.isInvalid()) {
    T.skipToEnd();
    return true;
  }

  // Parse the unqualified-id.
  SourceLocation TemplateKWLoc; // FIXME: parsed, but unused.
  if (ParseUnqualifiedId(Result.SS, false, true, true, ParsedType(),
                         TemplateKWLoc, Result.Name)) {
    T.skipToEnd();
    return true;
  }

  if (T.consumeClose())
    return true;
  
  // Check if the symbol exists.
  switch (Actions.CheckMicrosoftIfExistsSymbol(getCurScope(), Result.KeywordLoc,
                                               Result.IsIfExists, Result.SS,
                                               Result.Name)) {
  case Sema::IER_Exists:
    Result.Behavior = Result.IsIfExists ? IEB_Parse : IEB_Skip;
    break;

  case Sema::IER_DoesNotExist:
    Result.Behavior = !Result.IsIfExists ? IEB_Parse : IEB_Skip;
    break;

  case Sema::IER_Dependent:
    Result.Behavior = IEB_Dependent;
    break;
      
  case Sema::IER_Error:
    return true;
  }

  return false;
}

void Parser::ParseMicrosoftIfExistsExternalDeclaration() {
  IfExistsCondition Result;
  if (ParseMicrosoftIfExistsCondition(Result))
    return;
  
  BalancedDelimiterTracker Braces(*this, tok::l_brace);
  if (Braces.consumeOpen()) {
    Diag(Tok, diag::err_expected) << tok::l_brace;
    return;
  }

  switch (Result.Behavior) {
  case IEB_Parse:
    // Parse declarations below.
    break;
      
  case IEB_Dependent:
    llvm_unreachable("Cannot have a dependent external declaration");
      
  case IEB_Skip:
    Braces.skipToEnd();
    return;
  }

  // Parse the declarations.
  // FIXME: Support module import within __if_exists?
  while (Tok.isNot(tok::r_brace) && !isEofOrEom()) {
    ParsedAttributesWithRange attrs(AttrFactory);
    MaybeParseCXX11Attributes(attrs);
    assert(!getLangOpts().HLSL); // HLSL Change: in lieu of MaybeParseHLSLAttributes - if-exists not allowed
    MaybeParseMicrosoftAttributes(attrs);
    DeclGroupPtrTy Result = ParseExternalDeclaration(attrs);
    if (Result && !getCurScope()->getParent())
      Actions.getASTConsumer().HandleTopLevelDecl(Result.get());
  }
  Braces.consumeClose();
}

Parser::DeclGroupPtrTy Parser::ParseModuleImport(SourceLocation AtLoc) {
  assert(Tok.isObjCAtKeyword(tok::objc_import) && 
         "Improper start to module import");
  SourceLocation ImportLoc = ConsumeToken();
  
  SmallVector<std::pair<IdentifierInfo *, SourceLocation>, 2> Path;
  
  // Parse the module path.
  do {
    if (!Tok.is(tok::identifier)) {
      if (Tok.is(tok::code_completion)) {
        Actions.CodeCompleteModuleImport(ImportLoc, Path);
        cutOffParsing();
        return DeclGroupPtrTy();
      }
      
      Diag(Tok, diag::err_module_expected_ident);
      SkipUntil(tok::semi);
      return DeclGroupPtrTy();
    }
    
    // Record this part of the module path.
    Path.push_back(std::make_pair(Tok.getIdentifierInfo(), Tok.getLocation()));
    ConsumeToken();
    
    if (Tok.is(tok::period)) {
      ConsumeToken();
      continue;
    }
    
    break;
  } while (true);

  if (PP.hadModuleLoaderFatalFailure()) {
    // With a fatal failure in the module loader, we abort parsing.
    cutOffParsing();
    return DeclGroupPtrTy();
  }

  DeclResult Import = Actions.ActOnModuleImport(AtLoc, ImportLoc, Path);
  ExpectAndConsumeSemi(diag::err_module_expected_semi);
  if (Import.isInvalid())
    return DeclGroupPtrTy();
  
  return Actions.ConvertDeclToDeclGroup(Import.get());
}

bool BalancedDelimiterTracker::diagnoseOverflow() {
  P.Diag(P.Tok, diag::err_bracket_depth_exceeded)
    << P.getLangOpts().BracketDepth;
  P.Diag(P.Tok, diag::note_bracket_depth);
  P.cutOffParsing();
  return true;
}

bool BalancedDelimiterTracker::expectAndConsume(unsigned DiagID,
                                                const char *Msg,
                                                tok::TokenKind SkipToTok) {
  LOpen = P.Tok.getLocation();
  if (P.ExpectAndConsume(Kind, DiagID, Msg)) {
    if (SkipToTok != tok::unknown)
      P.SkipUntil(SkipToTok, Parser::StopAtSemi);
    return true;
  }

  if (getDepth() < MaxDepth)
    return false;
    
  return diagnoseOverflow();
}

bool BalancedDelimiterTracker::diagnoseMissingClose() {
  assert(!P.Tok.is(Close) && "Should have consumed closing delimiter");

  P.Diag(P.Tok, diag::err_expected) << Close;
  P.Diag(LOpen, diag::note_matching) << Kind;

  // If we're not already at some kind of closing bracket, skip to our closing
  // token.
  if (P.Tok.isNot(tok::r_paren) && P.Tok.isNot(tok::r_brace) &&
      P.Tok.isNot(tok::r_square) &&
      P.SkipUntil(Close, FinalToken,
                  Parser::StopAtSemi | Parser::StopBeforeMatch) &&
      P.Tok.is(Close))
    LClose = P.ConsumeAnyToken();
  return true;
}

void BalancedDelimiterTracker::skipToEnd() {
  P.SkipUntil(Close, Parser::StopBeforeMatch);
  consumeClose();
}
