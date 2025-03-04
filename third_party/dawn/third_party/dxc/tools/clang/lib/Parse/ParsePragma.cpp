//===--- ParsePragma.cpp - Language specific pragma parsing ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the language specific #pragma handlers.
//
//===----------------------------------------------------------------------===//

#include "RAIIObjectsForParser.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseDiagnostic.h"
#include "clang/Parse/Parser.h"
#include "clang/Sema/LoopHint.h"
#include "clang/Sema/Scope.h"
#include "llvm/ADT/StringSwitch.h"
using namespace clang;

namespace {

struct PragmaAlignHandler : public PragmaHandler {
  explicit PragmaAlignHandler() : PragmaHandler("align") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};

struct PragmaGCCVisibilityHandler : public PragmaHandler {
  explicit PragmaGCCVisibilityHandler() : PragmaHandler("visibility") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};

struct PragmaOptionsHandler : public PragmaHandler {
  explicit PragmaOptionsHandler() : PragmaHandler("options") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};

struct PragmaPackHandler : public PragmaHandler {
  explicit PragmaPackHandler() : PragmaHandler("pack") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};

struct PragmaMSStructHandler : public PragmaHandler {
  explicit PragmaMSStructHandler() : PragmaHandler("ms_struct") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};

struct PragmaUnusedHandler : public PragmaHandler {
  PragmaUnusedHandler() : PragmaHandler("unused") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};

struct PragmaWeakHandler : public PragmaHandler {
  explicit PragmaWeakHandler() : PragmaHandler("weak") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};

struct PragmaRedefineExtnameHandler : public PragmaHandler {
  explicit PragmaRedefineExtnameHandler() : PragmaHandler("redefine_extname") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};

struct PragmaOpenCLExtensionHandler : public PragmaHandler {
  PragmaOpenCLExtensionHandler() : PragmaHandler("EXTENSION") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};


struct PragmaFPContractHandler : public PragmaHandler {
  PragmaFPContractHandler() : PragmaHandler("FP_CONTRACT") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};

struct PragmaNoOpenMPHandler : public PragmaHandler {
  PragmaNoOpenMPHandler() : PragmaHandler("omp") { }
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};

struct PragmaOpenMPHandler : public PragmaHandler {
  PragmaOpenMPHandler() : PragmaHandler("omp") { }
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};

/// PragmaCommentHandler - "\#pragma comment ...".
struct PragmaCommentHandler : public PragmaHandler {
  PragmaCommentHandler(Sema &Actions)
    : PragmaHandler("comment"), Actions(Actions) {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
private:
  Sema &Actions;
};

struct PragmaDetectMismatchHandler : public PragmaHandler {
  PragmaDetectMismatchHandler(Sema &Actions)
    : PragmaHandler("detect_mismatch"), Actions(Actions) {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
private:
  Sema &Actions;
};

struct PragmaMSPointersToMembers : public PragmaHandler {
  explicit PragmaMSPointersToMembers() : PragmaHandler("pointers_to_members") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};

struct PragmaMSVtorDisp : public PragmaHandler {
  explicit PragmaMSVtorDisp() : PragmaHandler("vtordisp") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};

struct PragmaMSPragma : public PragmaHandler {
  explicit PragmaMSPragma(const char *name) : PragmaHandler(name) {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};

/// PragmaOptimizeHandler - "\#pragma clang optimize on/off".
struct PragmaOptimizeHandler : public PragmaHandler {
  PragmaOptimizeHandler(Sema &S)
    : PragmaHandler("optimize"), Actions(S) {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
private:
  Sema &Actions;
};

struct PragmaLoopHintHandler : public PragmaHandler {
  PragmaLoopHintHandler() : PragmaHandler("loop") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};

struct PragmaUnrollHintHandler : public PragmaHandler {
  PragmaUnrollHintHandler(const char *name) : PragmaHandler(name) {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;
};

struct PragmaPackMatrixHandler : public PragmaHandler {
  PragmaPackMatrixHandler(Sema &S) : PragmaHandler("pack_matrix"), Actions(S) {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &FirstToken) override;

private:
  Sema &Actions;
};

}  // end namespace

void Parser::initializePragmaHandlers() {
  if (!getLangOpts().HLSL) { // HLSL Change
  // HLSL Note - Considering adding alignment support
  AlignHandler.reset(new PragmaAlignHandler());
  PP.AddPragmaHandler(AlignHandler.get());

  GCCVisibilityHandler.reset(new PragmaGCCVisibilityHandler());
  PP.AddPragmaHandler("GCC", GCCVisibilityHandler.get());

  OptionsHandler.reset(new PragmaOptionsHandler());
  PP.AddPragmaHandler(OptionsHandler.get());

  PackHandler.reset(new PragmaPackHandler());
  PP.AddPragmaHandler(PackHandler.get());

  MSStructHandler.reset(new PragmaMSStructHandler());
  PP.AddPragmaHandler(MSStructHandler.get());

  UnusedHandler.reset(new PragmaUnusedHandler());
  PP.AddPragmaHandler(UnusedHandler.get());

  WeakHandler.reset(new PragmaWeakHandler());
  PP.AddPragmaHandler(WeakHandler.get());

  RedefineExtnameHandler.reset(new PragmaRedefineExtnameHandler());
  PP.AddPragmaHandler(RedefineExtnameHandler.get());

  FPContractHandler.reset(new PragmaFPContractHandler());
  PP.AddPragmaHandler("STDC", FPContractHandler.get());

  if (getLangOpts().OpenCL) {
    OpenCLExtensionHandler.reset(new PragmaOpenCLExtensionHandler());
    PP.AddPragmaHandler("OPENCL", OpenCLExtensionHandler.get());

    PP.AddPragmaHandler("OPENCL", FPContractHandler.get());
  }
  if (getLangOpts().OpenMP)
    OpenMPHandler.reset(new PragmaOpenMPHandler());
  else
    OpenMPHandler.reset(new PragmaNoOpenMPHandler());
  PP.AddPragmaHandler(OpenMPHandler.get());

  if (getLangOpts().MicrosoftExt || getTargetInfo().getTriple().isPS4()) {
    MSCommentHandler.reset(new PragmaCommentHandler(Actions));
    PP.AddPragmaHandler(MSCommentHandler.get());
  }

  if (getLangOpts().MicrosoftExt) {
    MSDetectMismatchHandler.reset(new PragmaDetectMismatchHandler(Actions));
    PP.AddPragmaHandler(MSDetectMismatchHandler.get());
    MSPointersToMembers.reset(new PragmaMSPointersToMembers());
    PP.AddPragmaHandler(MSPointersToMembers.get());
    MSVtorDisp.reset(new PragmaMSVtorDisp());
    PP.AddPragmaHandler(MSVtorDisp.get());
    MSInitSeg.reset(new PragmaMSPragma("init_seg"));
    PP.AddPragmaHandler(MSInitSeg.get());
    MSDataSeg.reset(new PragmaMSPragma("data_seg"));
    PP.AddPragmaHandler(MSDataSeg.get());
    MSBSSSeg.reset(new PragmaMSPragma("bss_seg"));
    PP.AddPragmaHandler(MSBSSSeg.get());
    MSConstSeg.reset(new PragmaMSPragma("const_seg"));
    PP.AddPragmaHandler(MSConstSeg.get());
    MSCodeSeg.reset(new PragmaMSPragma("code_seg"));
    PP.AddPragmaHandler(MSCodeSeg.get());
    MSSection.reset(new PragmaMSPragma("section"));
    PP.AddPragmaHandler(MSSection.get());
  }

  // HLSL Note - Considering adding alignment support
  OptimizeHandler.reset(new PragmaOptimizeHandler(Actions));
  PP.AddPragmaHandler("clang", OptimizeHandler.get());

  // HLSL Note - Considering adding alignment support
  LoopHintHandler.reset(new PragmaLoopHintHandler());
  PP.AddPragmaHandler("clang", LoopHintHandler.get());

  UnrollHintHandler.reset(new PragmaUnrollHintHandler("unroll"));
  PP.AddPragmaHandler(UnrollHintHandler.get());

  NoUnrollHintHandler.reset(new PragmaUnrollHintHandler("nounroll"));
  PP.AddPragmaHandler(NoUnrollHintHandler.get());
  } // HLSL Change, matching HLSL check to remove pragma processing
  else {
    // HLSL Change Begin - packmatrix.
    // The pointer ownership goes to PP as soon as we do the call,
    // which deletes it in its destructor unless it is removed & deleted via resetPragmaHandlers
    pPackMatrixHandler = new PragmaPackMatrixHandler(Actions);
    PP.AddPragmaHandler(pPackMatrixHandler);
    // HLSL Change End.
  }
}

void Parser::resetPragmaHandlers() {
  if (!getLangOpts().HLSL) { // HLSL Change - open conditional for skipping pragmas
  // Remove the pragma handlers we installed.
  PP.RemovePragmaHandler(AlignHandler.get());
  AlignHandler.reset();
  PP.RemovePragmaHandler("GCC", GCCVisibilityHandler.get());
  GCCVisibilityHandler.reset();
  PP.RemovePragmaHandler(OptionsHandler.get());
  OptionsHandler.reset();
  PP.RemovePragmaHandler(PackHandler.get());
  PackHandler.reset();
  PP.RemovePragmaHandler(MSStructHandler.get());
  MSStructHandler.reset();
  PP.RemovePragmaHandler(UnusedHandler.get());
  UnusedHandler.reset();
  PP.RemovePragmaHandler(WeakHandler.get());
  WeakHandler.reset();
  PP.RemovePragmaHandler(RedefineExtnameHandler.get());
  RedefineExtnameHandler.reset();

  if (getLangOpts().OpenCL) {
    PP.RemovePragmaHandler("OPENCL", OpenCLExtensionHandler.get());
    OpenCLExtensionHandler.reset();
    PP.RemovePragmaHandler("OPENCL", FPContractHandler.get());
  }
  PP.RemovePragmaHandler(OpenMPHandler.get());
  OpenMPHandler.reset();

  if (getLangOpts().MicrosoftExt || getTargetInfo().getTriple().isPS4()) {
    PP.RemovePragmaHandler(MSCommentHandler.get());
    MSCommentHandler.reset();
  }

  if (getLangOpts().MicrosoftExt) {
    PP.RemovePragmaHandler(MSDetectMismatchHandler.get());
    MSDetectMismatchHandler.reset();
    PP.RemovePragmaHandler(MSPointersToMembers.get());
    MSPointersToMembers.reset();
    PP.RemovePragmaHandler(MSVtorDisp.get());
    MSVtorDisp.reset();
    PP.RemovePragmaHandler(MSInitSeg.get());
    MSInitSeg.reset();
    PP.RemovePragmaHandler(MSDataSeg.get());
    MSDataSeg.reset();
    PP.RemovePragmaHandler(MSBSSSeg.get());
    MSBSSSeg.reset();
    PP.RemovePragmaHandler(MSConstSeg.get());
    MSConstSeg.reset();
    PP.RemovePragmaHandler(MSCodeSeg.get());
    MSCodeSeg.reset();
    PP.RemovePragmaHandler(MSSection.get());
    MSSection.reset();
  }

  PP.RemovePragmaHandler("STDC", FPContractHandler.get());
  FPContractHandler.reset();

  PP.RemovePragmaHandler("clang", OptimizeHandler.get());
  OptimizeHandler.reset();

  PP.RemovePragmaHandler("clang", LoopHintHandler.get());
  LoopHintHandler.reset();

  PP.RemovePragmaHandler(UnrollHintHandler.get());
  UnrollHintHandler.reset();

  PP.RemovePragmaHandler(NoUnrollHintHandler.get());
  NoUnrollHintHandler.reset();
  } // HLSL Change - close conditional for skipping pragmas
  else {
    // HLSL Change Begin - packmatrix.
    PP.RemovePragmaHandler(pPackMatrixHandler);
    delete pPackMatrixHandler;
    pPackMatrixHandler = nullptr;
    // HLSL Change End.
  }
}

/// \brief Handle the annotation token produced for #pragma unused(...)
///
/// Each annot_pragma_unused is followed by the argument token so e.g.
/// "#pragma unused(x,y)" becomes:
/// annot_pragma_unused 'x' annot_pragma_unused 'y'
void Parser::HandlePragmaUnused() {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  assert(Tok.is(tok::annot_pragma_unused));
  SourceLocation UnusedLoc = ConsumeToken();
  Actions.ActOnPragmaUnused(Tok, getCurScope(), UnusedLoc);
  ConsumeToken(); // The argument token.
}

void Parser::HandlePragmaVisibility() {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  assert(Tok.is(tok::annot_pragma_vis));
  const IdentifierInfo *VisType =
    static_cast<IdentifierInfo *>(Tok.getAnnotationValue());
  SourceLocation VisLoc = ConsumeToken();
  Actions.ActOnPragmaVisibility(VisType, VisLoc);
}

struct PragmaPackInfo {
  Sema::PragmaPackKind Kind;
  IdentifierInfo *Name;
  Token Alignment;
  SourceLocation LParenLoc;
  SourceLocation RParenLoc;
};

void Parser::HandlePragmaPack() {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  assert(Tok.is(tok::annot_pragma_pack));
  PragmaPackInfo *Info =
    static_cast<PragmaPackInfo *>(Tok.getAnnotationValue());
  SourceLocation PragmaLoc = ConsumeToken();
  ExprResult Alignment;
  if (Info->Alignment.is(tok::numeric_constant)) {
    Alignment = Actions.ActOnNumericConstant(Info->Alignment);
    if (Alignment.isInvalid())
      return;
  }
  Actions.ActOnPragmaPack(Info->Kind, Info->Name, Alignment.get(), PragmaLoc,
                          Info->LParenLoc, Info->RParenLoc);
}

void Parser::HandlePragmaMSStruct() {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  assert(Tok.is(tok::annot_pragma_msstruct));
  Sema::PragmaMSStructKind Kind =
    static_cast<Sema::PragmaMSStructKind>(
    reinterpret_cast<uintptr_t>(Tok.getAnnotationValue()));
  Actions.ActOnPragmaMSStruct(Kind);
  ConsumeToken(); // The annotation token.
}

void Parser::HandlePragmaAlign() {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  assert(Tok.is(tok::annot_pragma_align));
  Sema::PragmaOptionsAlignKind Kind =
    static_cast<Sema::PragmaOptionsAlignKind>(
    reinterpret_cast<uintptr_t>(Tok.getAnnotationValue()));
  SourceLocation PragmaLoc = ConsumeToken();
  Actions.ActOnPragmaOptionsAlign(Kind, PragmaLoc);
}

void Parser::HandlePragmaWeak() {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  assert(Tok.is(tok::annot_pragma_weak));
  SourceLocation PragmaLoc = ConsumeToken();
  Actions.ActOnPragmaWeakID(Tok.getIdentifierInfo(), PragmaLoc,
                            Tok.getLocation());
  ConsumeToken(); // The weak name.
}

void Parser::HandlePragmaWeakAlias() {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  assert(Tok.is(tok::annot_pragma_weakalias));
  SourceLocation PragmaLoc = ConsumeToken();
  IdentifierInfo *WeakName = Tok.getIdentifierInfo();
  SourceLocation WeakNameLoc = Tok.getLocation();
  ConsumeToken();
  IdentifierInfo *AliasName = Tok.getIdentifierInfo();
  SourceLocation AliasNameLoc = Tok.getLocation();
  ConsumeToken();
  Actions.ActOnPragmaWeakAlias(WeakName, AliasName, PragmaLoc,
                               WeakNameLoc, AliasNameLoc);

}

void Parser::HandlePragmaRedefineExtname() {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  assert(Tok.is(tok::annot_pragma_redefine_extname));
  SourceLocation RedefLoc = ConsumeToken();
  IdentifierInfo *RedefName = Tok.getIdentifierInfo();
  SourceLocation RedefNameLoc = Tok.getLocation();
  ConsumeToken();
  IdentifierInfo *AliasName = Tok.getIdentifierInfo();
  SourceLocation AliasNameLoc = Tok.getLocation();
  ConsumeToken();
  Actions.ActOnPragmaRedefineExtname(RedefName, AliasName, RedefLoc,
                                     RedefNameLoc, AliasNameLoc);
}

void Parser::HandlePragmaFPContract() {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  assert(Tok.is(tok::annot_pragma_fp_contract));
  tok::OnOffSwitch OOS =
    static_cast<tok::OnOffSwitch>(
    reinterpret_cast<uintptr_t>(Tok.getAnnotationValue()));
  Actions.ActOnPragmaFPContract(OOS);
  ConsumeToken(); // The annotation token.
}

StmtResult Parser::HandlePragmaCaptured()
{
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  assert(Tok.is(tok::annot_pragma_captured));
  ConsumeToken();

  if (Tok.isNot(tok::l_brace)) {
    PP.Diag(Tok, diag::err_expected) << tok::l_brace;
    return StmtError();
  }

  SourceLocation Loc = Tok.getLocation();

  ParseScope CapturedRegionScope(this, Scope::FnScope | Scope::DeclScope);
  Actions.ActOnCapturedRegionStart(Loc, getCurScope(), CR_Default,
                                   /*NumParams=*/1);

  StmtResult R = ParseCompoundStatement();
  CapturedRegionScope.Exit();

  if (R.isInvalid()) {
    Actions.ActOnCapturedRegionError();
    return StmtError();
  }

  return Actions.ActOnCapturedRegionEnd(R.get());
}

namespace {
  typedef llvm::PointerIntPair<IdentifierInfo *, 1, bool> OpenCLExtData;
}

void Parser::HandlePragmaOpenCLExtension() {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  assert(Tok.is(tok::annot_pragma_opencl_extension));
  OpenCLExtData data =
      OpenCLExtData::getFromOpaqueValue(Tok.getAnnotationValue());
  unsigned state = data.getInt();
  IdentifierInfo *ename = data.getPointer();
  SourceLocation NameLoc = Tok.getLocation();
  ConsumeToken(); // The annotation token.

  OpenCLOptions &f = Actions.getOpenCLOptions();
  // OpenCL 1.1 9.1: "The all variant sets the behavior for all extensions,
  // overriding all previously issued extension directives, but only if the
  // behavior is set to disable."
  if (state == 0 && ename->isStr("all")) {
#define OPENCLEXT(nm)   f.nm = 0;
#include "clang/Basic/OpenCLExtensions.def"
  }
#define OPENCLEXT(nm) else if (ename->isStr(#nm)) { f.nm = state; }
#include "clang/Basic/OpenCLExtensions.def"
  else {
    PP.Diag(NameLoc, diag::warn_pragma_unknown_extension) << ename;
    return;
  }
}

void Parser::HandlePragmaMSPointersToMembers() {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  assert(Tok.is(tok::annot_pragma_ms_pointers_to_members));
  LangOptions::PragmaMSPointersToMembersKind RepresentationMethod =
      static_cast<LangOptions::PragmaMSPointersToMembersKind>(
          reinterpret_cast<uintptr_t>(Tok.getAnnotationValue()));
  SourceLocation PragmaLoc = ConsumeToken(); // The annotation token.
  Actions.ActOnPragmaMSPointersToMembers(RepresentationMethod, PragmaLoc);
}

void Parser::HandlePragmaMSVtorDisp() {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  assert(Tok.is(tok::annot_pragma_ms_vtordisp));
  uintptr_t Value = reinterpret_cast<uintptr_t>(Tok.getAnnotationValue());
  Sema::PragmaVtorDispKind Kind =
      static_cast<Sema::PragmaVtorDispKind>((Value >> 16) & 0xFFFF);
  MSVtorDispAttr::Mode Mode = MSVtorDispAttr::Mode(Value & 0xFFFF);
  SourceLocation PragmaLoc = ConsumeToken(); // The annotation token.
  Actions.ActOnPragmaMSVtorDisp(Kind, PragmaLoc, Mode);
}

void Parser::HandlePragmaMSPragma() {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  assert(Tok.is(tok::annot_pragma_ms_pragma));
  // Grab the tokens out of the annotation and enter them into the stream.
  auto TheTokens = (std::pair<Token*, size_t> *)Tok.getAnnotationValue();
  PP.EnterTokenStream(TheTokens->first, TheTokens->second, true, true);
  SourceLocation PragmaLocation = ConsumeToken(); // The annotation token.
  assert(Tok.isAnyIdentifier());
  StringRef PragmaName = Tok.getIdentifierInfo()->getName();
  PP.Lex(Tok); // pragma kind

  // Figure out which #pragma we're dealing with.  The switch has no default
  // because lex shouldn't emit the annotation token for unrecognized pragmas.
  typedef bool (Parser::*PragmaHandler)(StringRef, SourceLocation);
  PragmaHandler Handler = llvm::StringSwitch<PragmaHandler>(PragmaName)
    .Case("data_seg", &Parser::HandlePragmaMSSegment)
    .Case("bss_seg", &Parser::HandlePragmaMSSegment)
    .Case("const_seg", &Parser::HandlePragmaMSSegment)
    .Case("code_seg", &Parser::HandlePragmaMSSegment)
    .Case("section", &Parser::HandlePragmaMSSection)
    .Case("init_seg", &Parser::HandlePragmaMSInitSeg);

  if (!(this->*Handler)(PragmaName, PragmaLocation)) {
    // Pragma handling failed, and has been diagnosed.  Slurp up the tokens
    // until eof (really end of line) to prevent follow-on errors.
    while (Tok.isNot(tok::eof))
      PP.Lex(Tok);
    PP.Lex(Tok);
  }
}

bool Parser::HandlePragmaMSSection(StringRef PragmaName,
                                   SourceLocation PragmaLocation) {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  if (Tok.isNot(tok::l_paren)) {
    PP.Diag(PragmaLocation, diag::warn_pragma_expected_lparen) << PragmaName;
    return false;
  }
  PP.Lex(Tok); // (
  // Parsing code for pragma section
  if (Tok.isNot(tok::string_literal)) {
    PP.Diag(PragmaLocation, diag::warn_pragma_expected_section_name)
        << PragmaName;
    return false;
  }
  ExprResult StringResult = ParseStringLiteralExpression();
  if (StringResult.isInvalid())
    return false; // Already diagnosed.
  StringLiteral *SegmentName = cast<StringLiteral>(StringResult.get());
  if (SegmentName->getCharByteWidth() != 1) {
    PP.Diag(PragmaLocation, diag::warn_pragma_expected_non_wide_string)
        << PragmaName;
    return false;
  }
  int SectionFlags = ASTContext::PSF_Read;
  bool SectionFlagsAreDefault = true;
  while (Tok.is(tok::comma)) {
    PP.Lex(Tok); // ,
    // Ignore "long" and "short".
    // They are undocumented, but widely used, section attributes which appear
    // to do nothing.
    if (Tok.is(tok::kw_long) || Tok.is(tok::kw_short)) {
      PP.Lex(Tok); // long/short
      continue;
    }

    if (!Tok.isAnyIdentifier()) {
      PP.Diag(PragmaLocation, diag::warn_pragma_expected_action_or_r_paren)
          << PragmaName;
      return false;
    }
    ASTContext::PragmaSectionFlag Flag =
      llvm::StringSwitch<ASTContext::PragmaSectionFlag>(
      Tok.getIdentifierInfo()->getName())
      .Case("read", ASTContext::PSF_Read)
      .Case("write", ASTContext::PSF_Write)
      .Case("execute", ASTContext::PSF_Execute)
      .Case("shared", ASTContext::PSF_Invalid)
      .Case("nopage", ASTContext::PSF_Invalid)
      .Case("nocache", ASTContext::PSF_Invalid)
      .Case("discard", ASTContext::PSF_Invalid)
      .Case("remove", ASTContext::PSF_Invalid)
      .Default(ASTContext::PSF_None);
    if (Flag == ASTContext::PSF_None || Flag == ASTContext::PSF_Invalid) {
      PP.Diag(PragmaLocation, Flag == ASTContext::PSF_None
                                  ? diag::warn_pragma_invalid_specific_action
                                  : diag::warn_pragma_unsupported_action)
          << PragmaName << Tok.getIdentifierInfo()->getName();
      return false;
    }
    SectionFlags |= Flag;
    SectionFlagsAreDefault = false;
    PP.Lex(Tok); // Identifier
  }
  // If no section attributes are specified, the section will be marked as
  // read/write.
  if (SectionFlagsAreDefault)
    SectionFlags |= ASTContext::PSF_Write;
  if (Tok.isNot(tok::r_paren)) {
    PP.Diag(PragmaLocation, diag::warn_pragma_expected_rparen) << PragmaName;
    return false;
  }
  PP.Lex(Tok); // )
  if (Tok.isNot(tok::eof)) {
    PP.Diag(PragmaLocation, diag::warn_pragma_extra_tokens_at_eol)
        << PragmaName;
    return false;
  }
  PP.Lex(Tok); // eof
  Actions.ActOnPragmaMSSection(PragmaLocation, SectionFlags, SegmentName);
  return true;
}

bool Parser::HandlePragmaMSSegment(StringRef PragmaName,
                                   SourceLocation PragmaLocation) {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  if (Tok.isNot(tok::l_paren)) {
    PP.Diag(PragmaLocation, diag::warn_pragma_expected_lparen) << PragmaName;
    return false;
  }
  PP.Lex(Tok); // (
  Sema::PragmaMsStackAction Action = Sema::PSK_Reset;
  StringRef SlotLabel;
  if (Tok.isAnyIdentifier()) {
    StringRef PushPop = Tok.getIdentifierInfo()->getName();
    if (PushPop == "push")
      Action = Sema::PSK_Push;
    else if (PushPop == "pop")
      Action = Sema::PSK_Pop;
    else {
      PP.Diag(PragmaLocation,
              diag::warn_pragma_expected_section_push_pop_or_name)
          << PragmaName;
      return false;
    }
    if (Action != Sema::PSK_Reset) {
      PP.Lex(Tok); // push | pop
      if (Tok.is(tok::comma)) {
        PP.Lex(Tok); // ,
        // If we've got a comma, we either need a label or a string.
        if (Tok.isAnyIdentifier()) {
          SlotLabel = Tok.getIdentifierInfo()->getName();
          PP.Lex(Tok); // identifier
          if (Tok.is(tok::comma))
            PP.Lex(Tok);
          else if (Tok.isNot(tok::r_paren)) {
            PP.Diag(PragmaLocation, diag::warn_pragma_expected_punc)
                << PragmaName;
            return false;
          }
        }
      } else if (Tok.isNot(tok::r_paren)) {
        PP.Diag(PragmaLocation, diag::warn_pragma_expected_punc) << PragmaName;
        return false;
      }
    }
  }
  // Grab the string literal for our section name.
  StringLiteral *SegmentName = nullptr;
  if (Tok.isNot(tok::r_paren)) {
    if (Tok.isNot(tok::string_literal)) {
      unsigned DiagID = Action != Sema::PSK_Reset ? !SlotLabel.empty() ?
          diag::warn_pragma_expected_section_name :
          diag::warn_pragma_expected_section_label_or_name :
          diag::warn_pragma_expected_section_push_pop_or_name;
      PP.Diag(PragmaLocation, DiagID) << PragmaName;
      return false;
    }
    ExprResult StringResult = ParseStringLiteralExpression();
    if (StringResult.isInvalid())
      return false; // Already diagnosed.
    SegmentName = cast<StringLiteral>(StringResult.get());
    if (SegmentName->getCharByteWidth() != 1) {
      PP.Diag(PragmaLocation, diag::warn_pragma_expected_non_wide_string)
          << PragmaName;
      return false;
    }
    // Setting section "" has no effect
    if (SegmentName->getLength())
      Action = (Sema::PragmaMsStackAction)(Action | Sema::PSK_Set);
  }
  if (Tok.isNot(tok::r_paren)) {
    PP.Diag(PragmaLocation, diag::warn_pragma_expected_rparen) << PragmaName;
    return false;
  }
  PP.Lex(Tok); // )
  if (Tok.isNot(tok::eof)) {
    PP.Diag(PragmaLocation, diag::warn_pragma_extra_tokens_at_eol)
        << PragmaName;
    return false;
  }
  PP.Lex(Tok); // eof
  Actions.ActOnPragmaMSSeg(PragmaLocation, Action, SlotLabel,
                           SegmentName, PragmaName);
  return true;
}

// #pragma init_seg({ compiler | lib | user | "section-name" [, func-name]} )
bool Parser::HandlePragmaMSInitSeg(StringRef PragmaName,
                                   SourceLocation PragmaLocation) {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  if (getTargetInfo().getTriple().getEnvironment() != llvm::Triple::MSVC) {
    PP.Diag(PragmaLocation, diag::warn_pragma_init_seg_unsupported_target);
    return false;
  }

  if (ExpectAndConsume(tok::l_paren, diag::warn_pragma_expected_lparen,
                       PragmaName))
    return false;

  // Parse either the known section names or the string section name.
  StringLiteral *SegmentName = nullptr;
  if (Tok.isAnyIdentifier()) {
    auto *II = Tok.getIdentifierInfo();
    StringRef Section = llvm::StringSwitch<StringRef>(II->getName())
                            .Case("compiler", "\".CRT$XCC\"")
                            .Case("lib", "\".CRT$XCL\"")
                            .Case("user", "\".CRT$XCU\"")
                            .Default("");

    if (!Section.empty()) {
      // Pretend the user wrote the appropriate string literal here.
      Token Toks[1];
      Toks[0].startToken();
      Toks[0].setKind(tok::string_literal);
      Toks[0].setLocation(Tok.getLocation());
      Toks[0].setLiteralData(Section.data());
      Toks[0].setLength(Section.size());
      SegmentName =
          cast<StringLiteral>(Actions.ActOnStringLiteral(Toks, nullptr).get());
      PP.Lex(Tok);
    }
  } else if (Tok.is(tok::string_literal)) {
    ExprResult StringResult = ParseStringLiteralExpression();
    if (StringResult.isInvalid())
      return false;
    SegmentName = cast<StringLiteral>(StringResult.get());
    if (SegmentName->getCharByteWidth() != 1) {
      PP.Diag(PragmaLocation, diag::warn_pragma_expected_non_wide_string)
          << PragmaName;
      return false;
    }
    // FIXME: Add support for the '[, func-name]' part of the pragma.
  }

  if (!SegmentName) {
    PP.Diag(PragmaLocation, diag::warn_pragma_expected_init_seg) << PragmaName;
    return false;
  }

  if (ExpectAndConsume(tok::r_paren, diag::warn_pragma_expected_rparen,
                       PragmaName) ||
      ExpectAndConsume(tok::eof, diag::warn_pragma_extra_tokens_at_eol,
                       PragmaName))
    return false;

  Actions.ActOnPragmaMSInitSeg(PragmaLocation, SegmentName);
  return true;
}

struct PragmaLoopHintInfo {
  Token PragmaName;
  Token Option;
  Token *Toks;
  size_t TokSize;
  PragmaLoopHintInfo() : Toks(nullptr), TokSize(0) {}
};

static std::string PragmaLoopHintString(Token PragmaName, Token Option) {
  std::string PragmaString;
  if (PragmaName.getIdentifierInfo()->getName() == "loop") {
    PragmaString = "clang loop ";
    PragmaString += Option.getIdentifierInfo()->getName();
  } else {
    assert(PragmaName.getIdentifierInfo()->getName() == "unroll" &&
           "Unexpected pragma name");
    PragmaString = "unroll";
  }
  return PragmaString;
}

bool Parser::HandlePragmaLoopHint(LoopHint &Hint) {
  assert(!getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  assert(Tok.is(tok::annot_pragma_loop_hint));
  PragmaLoopHintInfo *Info =
      static_cast<PragmaLoopHintInfo *>(Tok.getAnnotationValue());

  IdentifierInfo *PragmaNameInfo = Info->PragmaName.getIdentifierInfo();
  Hint.PragmaNameLoc = IdentifierLoc::create(
      Actions.Context, Info->PragmaName.getLocation(), PragmaNameInfo);

  // It is possible that the loop hint has no option identifier, such as
  // #pragma unroll(4).
  IdentifierInfo *OptionInfo = Info->Option.is(tok::identifier)
                                   ? Info->Option.getIdentifierInfo()
                                   : nullptr;
  Hint.OptionLoc = IdentifierLoc::create(
      Actions.Context, Info->Option.getLocation(), OptionInfo);

  Token *Toks = Info->Toks;
  size_t TokSize = Info->TokSize;

  // Return a valid hint if pragma unroll or nounroll were specified
  // without an argument.
  bool PragmaUnroll = PragmaNameInfo->getName() == "unroll";
  bool PragmaNoUnroll = PragmaNameInfo->getName() == "nounroll";
  if (TokSize == 0 && (PragmaUnroll || PragmaNoUnroll)) {
    ConsumeToken(); // The annotation token.
    Hint.Range = Info->PragmaName.getLocation();
    return true;
  }

  // The constant expression is always followed by an eof token, which increases
  // the TokSize by 1.
  assert(TokSize > 0 &&
         "PragmaLoopHintInfo::Toks must contain at least one token.");

  // If no option is specified the argument is assumed to be a constant expr.
  bool OptionUnroll = false;
  bool StateOption = false;
  if (OptionInfo) { // Pragma Unroll does not specify an option.
    OptionUnroll = OptionInfo->isStr("unroll");
    StateOption = llvm::StringSwitch<bool>(OptionInfo->getName())
                      .Case("vectorize", true)
                      .Case("interleave", true)
                      .Case("unroll", true)
                      .Default(false);
  }

  // Verify loop hint has an argument.
  if (Toks[0].is(tok::eof)) {
    ConsumeToken(); // The annotation token.
    Diag(Toks[0].getLocation(), diag::err_pragma_loop_missing_argument)
        << /*StateArgument=*/StateOption << /*FullKeyword=*/OptionUnroll;
    return false;
  }

  // Validate the argument.
  if (StateOption) {
    ConsumeToken(); // The annotation token.
    SourceLocation StateLoc = Toks[0].getLocation();
    IdentifierInfo *StateInfo = Toks[0].getIdentifierInfo();
    if (!StateInfo ||
        ((OptionUnroll ? !StateInfo->isStr("full")
                       : !StateInfo->isStr("enable") &&
                             !StateInfo->isStr("assume_safety")) &&
         !StateInfo->isStr("disable"))) {
      Diag(Toks[0].getLocation(), diag::err_pragma_invalid_keyword)
          << /*FullKeyword=*/OptionUnroll;
      return false;
    }
    if (TokSize > 2)
      Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol)
          << PragmaLoopHintString(Info->PragmaName, Info->Option);
    Hint.StateLoc = IdentifierLoc::create(Actions.Context, StateLoc, StateInfo);
  } else {
    // Enter constant expression including eof terminator into token stream.
    PP.EnterTokenStream(Toks, TokSize, /*DisableMacroExpansion=*/false,
                        /*OwnsTokens=*/false);
    ConsumeToken(); // The annotation token.

    ExprResult R = ParseConstantExpression();

    // Tokens following an error in an ill-formed constant expression will
    // remain in the token stream and must be removed.
    if (Tok.isNot(tok::eof)) {
      Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol)
          << PragmaLoopHintString(Info->PragmaName, Info->Option);
      while (Tok.isNot(tok::eof))
        ConsumeAnyToken();
    }

    ConsumeToken(); // Consume the constant expression eof terminator.

    if (R.isInvalid() ||
        Actions.CheckLoopHintExpr(R.get(), Toks[0].getLocation()))
      return false;

    // Argument is a constant expression with an integer type.
    Hint.ValueExpr = R.get();
  }

  Hint.Range = SourceRange(Info->PragmaName.getLocation(),
                           Info->Toks[TokSize - 1].getLocation());
  return true;
}

// #pragma GCC visibility comes in two variants:
//   'push' '(' [visibility] ')'
//   'pop'
void PragmaGCCVisibilityHandler::HandlePragma(Preprocessor &PP, 
                                              PragmaIntroducerKind Introducer,
                                              Token &VisTok) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  SourceLocation VisLoc = VisTok.getLocation();

  Token Tok;
  PP.LexUnexpandedToken(Tok);

  const IdentifierInfo *PushPop = Tok.getIdentifierInfo();

  const IdentifierInfo *VisType;
  if (PushPop && PushPop->isStr("pop")) {
    VisType = nullptr;
  } else if (PushPop && PushPop->isStr("push")) {
    PP.LexUnexpandedToken(Tok);
    if (Tok.isNot(tok::l_paren)) {
      PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_lparen)
        << "visibility";
      return;
    }
    PP.LexUnexpandedToken(Tok);
    VisType = Tok.getIdentifierInfo();
    if (!VisType) {
      PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_identifier)
        << "visibility";
      return;
    }
    PP.LexUnexpandedToken(Tok);
    if (Tok.isNot(tok::r_paren)) {
      PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_rparen)
        << "visibility";
      return;
    }
  } else {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_identifier)
      << "visibility";
    return;
  }
  SourceLocation EndLoc = Tok.getLocation();
  PP.LexUnexpandedToken(Tok);
  if (Tok.isNot(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol)
      << "visibility";
    return;
  }

  Token *Toks = new Token[1];
  Toks[0].startToken();
  Toks[0].setKind(tok::annot_pragma_vis);
  Toks[0].setLocation(VisLoc);
  Toks[0].setAnnotationEndLoc(EndLoc);
  Toks[0].setAnnotationValue(
                          const_cast<void*>(static_cast<const void*>(VisType)));
  PP.EnterTokenStream(Toks, 1, /*DisableMacroExpansion=*/true,
                      /*OwnsTokens=*/true);
}

// #pragma pack(...) comes in the following delicious flavors:
//   pack '(' [integer] ')'
//   pack '(' 'show' ')'
//   pack '(' ('push' | 'pop') [',' identifier] [, integer] ')'
void PragmaPackHandler::HandlePragma(Preprocessor &PP, 
                                     PragmaIntroducerKind Introducer,
                                     Token &PackTok) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  SourceLocation PackLoc = PackTok.getLocation();

  Token Tok;
  PP.Lex(Tok);
  if (Tok.isNot(tok::l_paren)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_lparen) << "pack";
    return;
  }

  Sema::PragmaPackKind Kind = Sema::PPK_Default;
  IdentifierInfo *Name = nullptr;
  Token Alignment;
  Alignment.startToken();
  SourceLocation LParenLoc = Tok.getLocation();
  PP.Lex(Tok);
  if (Tok.is(tok::numeric_constant)) {
    Alignment = Tok;

    PP.Lex(Tok);

    // In MSVC/gcc, #pragma pack(4) sets the alignment without affecting
    // the push/pop stack.
    // In Apple gcc, #pragma pack(4) is equivalent to #pragma pack(push, 4)
    if (PP.getLangOpts().ApplePragmaPack)
      Kind = Sema::PPK_Push;
  } else if (Tok.is(tok::identifier)) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if (II->isStr("show")) {
      Kind = Sema::PPK_Show;
      PP.Lex(Tok);
    } else {
      if (II->isStr("push")) {
        Kind = Sema::PPK_Push;
      } else if (II->isStr("pop")) {
        Kind = Sema::PPK_Pop;
      } else {
        PP.Diag(Tok.getLocation(), diag::warn_pragma_invalid_action) << "pack";
        return;
      }
      PP.Lex(Tok);

      if (Tok.is(tok::comma)) {
        PP.Lex(Tok);

        if (Tok.is(tok::numeric_constant)) {
          Alignment = Tok;

          PP.Lex(Tok);
        } else if (Tok.is(tok::identifier)) {
          Name = Tok.getIdentifierInfo();
          PP.Lex(Tok);

          if (Tok.is(tok::comma)) {
            PP.Lex(Tok);

            if (Tok.isNot(tok::numeric_constant)) {
              PP.Diag(Tok.getLocation(), diag::warn_pragma_pack_malformed);
              return;
            }

            Alignment = Tok;

            PP.Lex(Tok);
          }
        } else {
          PP.Diag(Tok.getLocation(), diag::warn_pragma_pack_malformed);
          return;
        }
      }
    }
  } else if (PP.getLangOpts().ApplePragmaPack) {
    // In MSVC/gcc, #pragma pack() resets the alignment without affecting
    // the push/pop stack.
    // In Apple gcc #pragma pack() is equivalent to #pragma pack(pop).
    Kind = Sema::PPK_Pop;
  }

  if (Tok.isNot(tok::r_paren)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_rparen) << "pack";
    return;
  }

  SourceLocation RParenLoc = Tok.getLocation();
  PP.Lex(Tok);
  if (Tok.isNot(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol) << "pack";
    return;
  }

  PragmaPackInfo *Info = 
    (PragmaPackInfo*) PP.getPreprocessorAllocator().Allocate(
      sizeof(PragmaPackInfo), llvm::alignOf<PragmaPackInfo>());
  new (Info) PragmaPackInfo();
  Info->Kind = Kind;
  Info->Name = Name;
  Info->Alignment = Alignment;
  Info->LParenLoc = LParenLoc;
  Info->RParenLoc = RParenLoc;

  Token *Toks = 
    (Token*) PP.getPreprocessorAllocator().Allocate(
      sizeof(Token) * 1, llvm::alignOf<Token>());
  new (Toks) Token();
  Toks[0].startToken();
  Toks[0].setKind(tok::annot_pragma_pack);
  Toks[0].setLocation(PackLoc);
  Toks[0].setAnnotationEndLoc(RParenLoc);
  Toks[0].setAnnotationValue(static_cast<void*>(Info));
  PP.EnterTokenStream(Toks, 1, /*DisableMacroExpansion=*/true,
                      /*OwnsTokens=*/false);
}

// #pragma ms_struct on
// #pragma ms_struct off
void PragmaMSStructHandler::HandlePragma(Preprocessor &PP, 
                                         PragmaIntroducerKind Introducer,
                                         Token &MSStructTok) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  Sema::PragmaMSStructKind Kind = Sema::PMSST_OFF;
  
  Token Tok;
  PP.Lex(Tok);
  if (Tok.isNot(tok::identifier)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_ms_struct);
    return;
  }
  SourceLocation EndLoc = Tok.getLocation();
  const IdentifierInfo *II = Tok.getIdentifierInfo();
  if (II->isStr("on")) {
    Kind = Sema::PMSST_ON;
    PP.Lex(Tok);
  }
  else if (II->isStr("off") || II->isStr("reset"))
    PP.Lex(Tok);
  else {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_ms_struct);
    return;
  }
  
  if (Tok.isNot(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol)
      << "ms_struct";
    return;
  }

  Token *Toks =
    (Token*) PP.getPreprocessorAllocator().Allocate(
      sizeof(Token) * 1, llvm::alignOf<Token>());
  new (Toks) Token();
  Toks[0].startToken();
  Toks[0].setKind(tok::annot_pragma_msstruct);
  Toks[0].setLocation(MSStructTok.getLocation());
  Toks[0].setAnnotationEndLoc(EndLoc);
  Toks[0].setAnnotationValue(reinterpret_cast<void*>(
                             static_cast<uintptr_t>(Kind)));
  PP.EnterTokenStream(Toks, 1, /*DisableMacroExpansion=*/true,
                      /*OwnsTokens=*/false);
}

// #pragma 'align' '=' {'native','natural','mac68k','power','reset'}
// #pragma 'options 'align' '=' {'native','natural','mac68k','power','reset'}
static void ParseAlignPragma(Preprocessor &PP, Token &FirstTok,
                             bool IsOptions) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  Token Tok;

  if (IsOptions) {
    PP.Lex(Tok);
    if (Tok.isNot(tok::identifier) ||
        !Tok.getIdentifierInfo()->isStr("align")) {
      PP.Diag(Tok.getLocation(), diag::warn_pragma_options_expected_align);
      return;
    }
  }

  PP.Lex(Tok);
  if (Tok.isNot(tok::equal)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_align_expected_equal)
      << IsOptions;
    return;
  }

  PP.Lex(Tok);
  if (Tok.isNot(tok::identifier)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_identifier)
      << (IsOptions ? "options" : "align");
    return;
  }

  Sema::PragmaOptionsAlignKind Kind = Sema::POAK_Natural;
  const IdentifierInfo *II = Tok.getIdentifierInfo();
  if (II->isStr("native"))
    Kind = Sema::POAK_Native;
  else if (II->isStr("natural"))
    Kind = Sema::POAK_Natural;
  else if (II->isStr("packed"))
    Kind = Sema::POAK_Packed;
  else if (II->isStr("power"))
    Kind = Sema::POAK_Power;
  else if (II->isStr("mac68k"))
    Kind = Sema::POAK_Mac68k;
  else if (II->isStr("reset"))
    Kind = Sema::POAK_Reset;
  else {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_align_invalid_option)
      << IsOptions;
    return;
  }

  SourceLocation EndLoc = Tok.getLocation();
  PP.Lex(Tok);
  if (Tok.isNot(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol)
      << (IsOptions ? "options" : "align");
    return;
  }

  Token *Toks =
    (Token*) PP.getPreprocessorAllocator().Allocate(
      sizeof(Token) * 1, llvm::alignOf<Token>());
  new (Toks) Token();
  Toks[0].startToken();
  Toks[0].setKind(tok::annot_pragma_align);
  Toks[0].setLocation(FirstTok.getLocation());
  Toks[0].setAnnotationEndLoc(EndLoc);
  Toks[0].setAnnotationValue(reinterpret_cast<void*>(
                             static_cast<uintptr_t>(Kind)));
  PP.EnterTokenStream(Toks, 1, /*DisableMacroExpansion=*/true,
                      /*OwnsTokens=*/false);
}

void PragmaAlignHandler::HandlePragma(Preprocessor &PP, 
                                      PragmaIntroducerKind Introducer,
                                      Token &AlignTok) {
  ParseAlignPragma(PP, AlignTok, /*IsOptions=*/false);
}

void PragmaOptionsHandler::HandlePragma(Preprocessor &PP, 
                                        PragmaIntroducerKind Introducer,
                                        Token &OptionsTok) {
  ParseAlignPragma(PP, OptionsTok, /*IsOptions=*/true);
}

// #pragma unused(identifier)
void PragmaUnusedHandler::HandlePragma(Preprocessor &PP, 
                                       PragmaIntroducerKind Introducer,
                                       Token &UnusedTok) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  // FIXME: Should we be expanding macros here? My guess is no.
  SourceLocation UnusedLoc = UnusedTok.getLocation();

  // Lex the left '('.
  Token Tok;
  PP.Lex(Tok);
  if (Tok.isNot(tok::l_paren)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_lparen) << "unused";
    return;
  }

  // Lex the declaration reference(s).
  SmallVector<Token, 5> Identifiers;
  SourceLocation RParenLoc;
  bool LexID = true;

  while (true) {
    PP.Lex(Tok);

    if (LexID) {
      if (Tok.is(tok::identifier)) {
        Identifiers.push_back(Tok);
        LexID = false;
        continue;
      }

      // Illegal token!
      PP.Diag(Tok.getLocation(), diag::warn_pragma_unused_expected_var);
      return;
    }

    // We are execting a ')' or a ','.
    if (Tok.is(tok::comma)) {
      LexID = true;
      continue;
    }

    if (Tok.is(tok::r_paren)) {
      RParenLoc = Tok.getLocation();
      break;
    }

    // Illegal token!
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_punc) << "unused";
    return;
  }

  PP.Lex(Tok);
  if (Tok.isNot(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol) <<
        "unused";
    return;
  }

  // Verify that we have a location for the right parenthesis.
  assert(RParenLoc.isValid() && "Valid '#pragma unused' must have ')'");
  assert(!Identifiers.empty() && "Valid '#pragma unused' must have arguments");

  // For each identifier token, insert into the token stream a
  // annot_pragma_unused token followed by the identifier token.
  // This allows us to cache a "#pragma unused" that occurs inside an inline
  // C++ member function.

  Token *Toks = 
    (Token*) PP.getPreprocessorAllocator().Allocate(
      sizeof(Token) * 2 * Identifiers.size(), llvm::alignOf<Token>());
  for (unsigned i=0; i != Identifiers.size(); i++) {
    Token &pragmaUnusedTok = Toks[2*i], &idTok = Toks[2*i+1];
    pragmaUnusedTok.startToken();
    pragmaUnusedTok.setKind(tok::annot_pragma_unused);
    pragmaUnusedTok.setLocation(UnusedLoc);
    idTok = Identifiers[i];
  }
  PP.EnterTokenStream(Toks, 2*Identifiers.size(),
                      /*DisableMacroExpansion=*/true, /*OwnsTokens=*/false);
}

// #pragma weak identifier
// #pragma weak identifier '=' identifier
void PragmaWeakHandler::HandlePragma(Preprocessor &PP, 
                                     PragmaIntroducerKind Introducer,
                                     Token &WeakTok) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  SourceLocation WeakLoc = WeakTok.getLocation();

  Token Tok;
  PP.Lex(Tok);
  if (Tok.isNot(tok::identifier)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_identifier) << "weak";
    return;
  }

  Token WeakName = Tok;
  bool HasAlias = false;
  Token AliasName;

  PP.Lex(Tok);
  if (Tok.is(tok::equal)) {
    HasAlias = true;
    PP.Lex(Tok);
    if (Tok.isNot(tok::identifier)) {
      PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_identifier)
          << "weak";
      return;
    }
    AliasName = Tok;
    PP.Lex(Tok);
  }

  if (Tok.isNot(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol) << "weak";
    return;
  }

  if (HasAlias) {
    Token *Toks = 
      (Token*) PP.getPreprocessorAllocator().Allocate(
        sizeof(Token) * 3, llvm::alignOf<Token>());
    Token &pragmaUnusedTok = Toks[0];
    pragmaUnusedTok.startToken();
    pragmaUnusedTok.setKind(tok::annot_pragma_weakalias);
    pragmaUnusedTok.setLocation(WeakLoc);
    pragmaUnusedTok.setAnnotationEndLoc(AliasName.getLocation());
    Toks[1] = WeakName;
    Toks[2] = AliasName;
    PP.EnterTokenStream(Toks, 3,
                        /*DisableMacroExpansion=*/true, /*OwnsTokens=*/false);
  } else {
    Token *Toks = 
      (Token*) PP.getPreprocessorAllocator().Allocate(
        sizeof(Token) * 2, llvm::alignOf<Token>());
    Token &pragmaUnusedTok = Toks[0];
    pragmaUnusedTok.startToken();
    pragmaUnusedTok.setKind(tok::annot_pragma_weak);
    pragmaUnusedTok.setLocation(WeakLoc);
    pragmaUnusedTok.setAnnotationEndLoc(WeakLoc);
    Toks[1] = WeakName;
    PP.EnterTokenStream(Toks, 2,
                        /*DisableMacroExpansion=*/true, /*OwnsTokens=*/false);
  }
}

// #pragma redefine_extname identifier identifier
void PragmaRedefineExtnameHandler::HandlePragma(Preprocessor &PP, 
                                               PragmaIntroducerKind Introducer,
                                                Token &RedefToken) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  SourceLocation RedefLoc = RedefToken.getLocation();

  Token Tok;
  PP.Lex(Tok);
  if (Tok.isNot(tok::identifier)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_identifier) <<
      "redefine_extname";
    return;
  }

  Token RedefName = Tok;
  PP.Lex(Tok);

  if (Tok.isNot(tok::identifier)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_identifier)
        << "redefine_extname";
    return;
  }

  Token AliasName = Tok;
  PP.Lex(Tok);

  if (Tok.isNot(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol) <<
      "redefine_extname";
    return;
  }

  Token *Toks = 
    (Token*) PP.getPreprocessorAllocator().Allocate(
      sizeof(Token) * 3, llvm::alignOf<Token>());
  Token &pragmaRedefTok = Toks[0];
  pragmaRedefTok.startToken();
  pragmaRedefTok.setKind(tok::annot_pragma_redefine_extname);
  pragmaRedefTok.setLocation(RedefLoc);
  pragmaRedefTok.setAnnotationEndLoc(AliasName.getLocation());
  Toks[1] = RedefName;
  Toks[2] = AliasName;
  PP.EnterTokenStream(Toks, 3,
                      /*DisableMacroExpansion=*/true, /*OwnsTokens=*/false);
}


void
PragmaFPContractHandler::HandlePragma(Preprocessor &PP, 
                                      PragmaIntroducerKind Introducer,
                                      Token &Tok) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  tok::OnOffSwitch OOS;
  if (PP.LexOnOffSwitch(OOS))
    return;

  Token *Toks =
    (Token*) PP.getPreprocessorAllocator().Allocate(
      sizeof(Token) * 1, llvm::alignOf<Token>());
  new (Toks) Token();
  Toks[0].startToken();
  Toks[0].setKind(tok::annot_pragma_fp_contract);
  Toks[0].setLocation(Tok.getLocation());
  Toks[0].setAnnotationEndLoc(Tok.getLocation());
  Toks[0].setAnnotationValue(reinterpret_cast<void*>(
                             static_cast<uintptr_t>(OOS)));
  PP.EnterTokenStream(Toks, 1, /*DisableMacroExpansion=*/true,
                      /*OwnsTokens=*/false);
}

void 
PragmaOpenCLExtensionHandler::HandlePragma(Preprocessor &PP, 
                                           PragmaIntroducerKind Introducer,
                                           Token &Tok) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  PP.LexUnexpandedToken(Tok);
  if (Tok.isNot(tok::identifier)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_identifier) <<
      "OPENCL";
    return;
  }
  IdentifierInfo *ename = Tok.getIdentifierInfo();
  SourceLocation NameLoc = Tok.getLocation();

  PP.Lex(Tok);
  if (Tok.isNot(tok::colon)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_colon) << ename;
    return;
  }

  PP.Lex(Tok);
  if (Tok.isNot(tok::identifier)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_enable_disable);
    return;
  }
  IdentifierInfo *op = Tok.getIdentifierInfo();

  unsigned state;
  if (op->isStr("enable")) {
    state = 1;
  } else if (op->isStr("disable")) {
    state = 0;
  } else {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_enable_disable);
    return;
  }
  SourceLocation StateLoc = Tok.getLocation();

  PP.Lex(Tok);
  if (Tok.isNot(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol) <<
      "OPENCL EXTENSION";
    return;
  }

  OpenCLExtData data(ename, state);
  Token *Toks =
    (Token*) PP.getPreprocessorAllocator().Allocate(
      sizeof(Token) * 1, llvm::alignOf<Token>());
  new (Toks) Token();
  Toks[0].startToken();
  Toks[0].setKind(tok::annot_pragma_opencl_extension);
  Toks[0].setLocation(NameLoc);
  Toks[0].setAnnotationValue(data.getOpaqueValue());
  Toks[0].setAnnotationEndLoc(StateLoc);
  PP.EnterTokenStream(Toks, 1, /*DisableMacroExpansion=*/true,
                      /*OwnsTokens=*/false);

  if (PP.getPPCallbacks())
    PP.getPPCallbacks()->PragmaOpenCLExtension(NameLoc, ename, 
                                               StateLoc, state);
}

/// \brief Handle '#pragma omp ...' when OpenMP is disabled.
///
void
PragmaNoOpenMPHandler::HandlePragma(Preprocessor &PP,
                                    PragmaIntroducerKind Introducer,
                                    Token &FirstTok) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  if (!PP.getDiagnostics().isIgnored(diag::warn_pragma_omp_ignored,
                                     FirstTok.getLocation())) {
    PP.Diag(FirstTok, diag::warn_pragma_omp_ignored);
    PP.getDiagnostics().setSeverity(diag::warn_pragma_omp_ignored,
                                    diag::Severity::Ignored, SourceLocation());
  }
  PP.DiscardUntilEndOfDirective();
}

/// \brief Handle '#pragma omp ...' when OpenMP is enabled.
///
void
PragmaOpenMPHandler::HandlePragma(Preprocessor &PP,
                                  PragmaIntroducerKind Introducer,
                                  Token &FirstTok) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  SmallVector<Token, 16> Pragma;
  Token Tok;
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_openmp);
  Tok.setLocation(FirstTok.getLocation());

  while (Tok.isNot(tok::eod)) {
    Pragma.push_back(Tok);
    PP.Lex(Tok);
  }
  SourceLocation EodLoc = Tok.getLocation();
  Tok.startToken();
  Tok.setKind(tok::annot_pragma_openmp_end);
  Tok.setLocation(EodLoc);
  Pragma.push_back(Tok);

  Token *Toks = new Token[Pragma.size()];
  std::copy(Pragma.begin(), Pragma.end(), Toks);
  PP.EnterTokenStream(Toks, Pragma.size(),
                      /*DisableMacroExpansion=*/false, /*OwnsTokens=*/true);
}

/// \brief Handle '#pragma pointers_to_members'
// The grammar for this pragma is as follows:
//
// <inheritance model> ::= ('single' | 'multiple' | 'virtual') '_inheritance'
//
// #pragma pointers_to_members '(' 'best_case' ')'
// #pragma pointers_to_members '(' 'full_generality' [',' inheritance-model] ')'
// #pragma pointers_to_members '(' inheritance-model ')'
void PragmaMSPointersToMembers::HandlePragma(Preprocessor &PP,
                                             PragmaIntroducerKind Introducer,
                                             Token &Tok) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  SourceLocation PointersToMembersLoc = Tok.getLocation();
  PP.Lex(Tok);
  if (Tok.isNot(tok::l_paren)) {
    PP.Diag(PointersToMembersLoc, diag::warn_pragma_expected_lparen)
      << "pointers_to_members";
    return;
  }
  PP.Lex(Tok);
  const IdentifierInfo *Arg = Tok.getIdentifierInfo();
  if (!Arg) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_identifier)
      << "pointers_to_members";
    return;
  }
  PP.Lex(Tok);

  LangOptions::PragmaMSPointersToMembersKind RepresentationMethod;
  if (Arg->isStr("best_case")) {
    RepresentationMethod = LangOptions::PPTMK_BestCase;
  } else {
    if (Arg->isStr("full_generality")) {
      if (Tok.is(tok::comma)) {
        PP.Lex(Tok);

        Arg = Tok.getIdentifierInfo();
        if (!Arg) {
          PP.Diag(Tok.getLocation(),
                  diag::err_pragma_pointers_to_members_unknown_kind)
              << Tok.getKind() << /*OnlyInheritanceModels*/ 0;
          return;
        }
        PP.Lex(Tok);
      } else if (Tok.is(tok::r_paren)) {
        // #pragma pointers_to_members(full_generality) implicitly specifies
        // virtual_inheritance.
        Arg = nullptr;
        RepresentationMethod = LangOptions::PPTMK_FullGeneralityVirtualInheritance;
      } else {
        PP.Diag(Tok.getLocation(), diag::err_expected_punc)
            << "full_generality";
        return;
      }
    }

    if (Arg) {
      if (Arg->isStr("single_inheritance")) {
        RepresentationMethod =
            LangOptions::PPTMK_FullGeneralitySingleInheritance;
      } else if (Arg->isStr("multiple_inheritance")) {
        RepresentationMethod =
            LangOptions::PPTMK_FullGeneralityMultipleInheritance;
      } else if (Arg->isStr("virtual_inheritance")) {
        RepresentationMethod =
            LangOptions::PPTMK_FullGeneralityVirtualInheritance;
      } else {
        PP.Diag(Tok.getLocation(),
                diag::err_pragma_pointers_to_members_unknown_kind)
            << Arg << /*HasPointerDeclaration*/ 1;
        return;
      }
    }
  }

  if (Tok.isNot(tok::r_paren)) {
    PP.Diag(Tok.getLocation(), diag::err_expected_rparen_after)
        << (Arg ? Arg->getName() : "full_generality");
    return;
  }

  SourceLocation EndLoc = Tok.getLocation();
  PP.Lex(Tok);
  if (Tok.isNot(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol)
      << "pointers_to_members";
    return;
  }

  Token AnnotTok;
  AnnotTok.startToken();
  AnnotTok.setKind(tok::annot_pragma_ms_pointers_to_members);
  AnnotTok.setLocation(PointersToMembersLoc);
  AnnotTok.setAnnotationEndLoc(EndLoc);
  AnnotTok.setAnnotationValue(
      reinterpret_cast<void *>(static_cast<uintptr_t>(RepresentationMethod)));
  PP.EnterToken(AnnotTok);
}

/// \brief Handle '#pragma vtordisp'
// The grammar for this pragma is as follows:
//
// <vtordisp-mode> ::= ('off' | 'on' | '0' | '1' | '2' )
//
// #pragma vtordisp '(' ['push' ','] vtordisp-mode ')'
// #pragma vtordisp '(' 'pop' ')'
// #pragma vtordisp '(' ')'
void PragmaMSVtorDisp::HandlePragma(Preprocessor &PP,
                                    PragmaIntroducerKind Introducer,
                                    Token &Tok) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  SourceLocation VtorDispLoc = Tok.getLocation();
  PP.Lex(Tok);
  if (Tok.isNot(tok::l_paren)) {
    PP.Diag(VtorDispLoc, diag::warn_pragma_expected_lparen) << "vtordisp";
    return;
  }
  PP.Lex(Tok);

  Sema::PragmaVtorDispKind Kind = Sema::PVDK_Set;
  const IdentifierInfo *II = Tok.getIdentifierInfo();
  if (II) {
    if (II->isStr("push")) {
      // #pragma vtordisp(push, mode)
      PP.Lex(Tok);
      if (Tok.isNot(tok::comma)) {
        PP.Diag(VtorDispLoc, diag::warn_pragma_expected_punc) << "vtordisp";
        return;
      }
      PP.Lex(Tok);
      Kind = Sema::PVDK_Push;
      // not push, could be on/off
    } else if (II->isStr("pop")) {
      // #pragma vtordisp(pop)
      PP.Lex(Tok);
      Kind = Sema::PVDK_Pop;
    }
    // not push or pop, could be on/off
  } else {
    if (Tok.is(tok::r_paren)) {
      // #pragma vtordisp()
      Kind = Sema::PVDK_Reset;
    }
  }


  uint64_t Value = 0;
  if (Kind == Sema::PVDK_Push || Kind == Sema::PVDK_Set) {
    const IdentifierInfo *II = Tok.getIdentifierInfo();
    if (II && II->isStr("off")) {
      PP.Lex(Tok);
      Value = 0;
    } else if (II && II->isStr("on")) {
      PP.Lex(Tok);
      Value = 1;
    } else if (Tok.is(tok::numeric_constant) &&
               PP.parseSimpleIntegerLiteral(Tok, Value)) {
      if (Value > 2) {
        PP.Diag(Tok.getLocation(), diag::warn_pragma_expected_integer)
            << 0 << 2 << "vtordisp";
        return;
      }
    } else {
      PP.Diag(Tok.getLocation(), diag::warn_pragma_invalid_action)
          << "vtordisp";
      return;
    }
  }

  // Finish the pragma: ')' $
  if (Tok.isNot(tok::r_paren)) {
    PP.Diag(VtorDispLoc, diag::warn_pragma_expected_rparen) << "vtordisp";
    return;
  }
  SourceLocation EndLoc = Tok.getLocation();
  PP.Lex(Tok);
  if (Tok.isNot(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol)
        << "vtordisp";
    return;
  }

  // Enter the annotation.
  Token AnnotTok;
  AnnotTok.startToken();
  AnnotTok.setKind(tok::annot_pragma_ms_vtordisp);
  AnnotTok.setLocation(VtorDispLoc);
  AnnotTok.setAnnotationEndLoc(EndLoc);
// OACR error 6297
#pragma prefast(disable: __WARNING_RESULTOFSHIFTCASTTOLARGERSIZE, "valid Kind values will not overflow")
  AnnotTok.setAnnotationValue(reinterpret_cast<void *>(
      static_cast<uintptr_t>((Kind << 16) | (Value & 0xFFFF))));
  PP.EnterToken(AnnotTok);
}

/// \brief Handle all MS pragmas.  Simply forwards the tokens after inserting
/// an annotation token.
void PragmaMSPragma::HandlePragma(Preprocessor &PP,
                                  PragmaIntroducerKind Introducer,
                                  Token &Tok) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  Token EoF, AnnotTok;
  EoF.startToken();
  EoF.setKind(tok::eof);
  AnnotTok.startToken();
  AnnotTok.setKind(tok::annot_pragma_ms_pragma);
  AnnotTok.setLocation(Tok.getLocation());
  AnnotTok.setAnnotationEndLoc(Tok.getLocation());
  SmallVector<Token, 8> TokenVector;
  // Suck up all of the tokens before the eod.
  for (; Tok.isNot(tok::eod); PP.Lex(Tok)) {
    TokenVector.push_back(Tok);
    AnnotTok.setAnnotationEndLoc(Tok.getLocation());
  }
  // Add a sentinal EoF token to the end of the list.
  TokenVector.push_back(EoF);
  // We must allocate this array with new because EnterTokenStream is going to
  // delete it later.
  Token *TokenArray = new Token[TokenVector.size()];
  std::copy(TokenVector.begin(), TokenVector.end(), TokenArray);
  auto Value = new (PP.getPreprocessorAllocator())
      std::pair<Token*, size_t>(std::make_pair(TokenArray, TokenVector.size()));
  AnnotTok.setAnnotationValue(Value);
  PP.EnterToken(AnnotTok);
}

/// \brief Handle the Microsoft \#pragma detect_mismatch extension.
///
/// The syntax is:
/// \code
///   #pragma detect_mismatch("name", "value")
/// \endcode
/// Where 'name' and 'value' are quoted strings.  The values are embedded in
/// the object file and passed along to the linker.  If the linker detects a
/// mismatch in the object file's values for the given name, a LNK2038 error
/// is emitted.  See MSDN for more details.
void PragmaDetectMismatchHandler::HandlePragma(Preprocessor &PP,
                                               PragmaIntroducerKind Introducer,
                                               Token &Tok) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  SourceLocation CommentLoc = Tok.getLocation();
  PP.Lex(Tok);
  if (Tok.isNot(tok::l_paren)) {
    PP.Diag(CommentLoc, diag::err_expected) << tok::l_paren;
    return;
  }

  // Read the name to embed, which must be a string literal.
  std::string NameString;
  if (!PP.LexStringLiteral(Tok, NameString,
                           "pragma detect_mismatch",
                           /*MacroExpansion=*/true))
    return;

  // Read the comma followed by a second string literal.
  std::string ValueString;
  if (Tok.isNot(tok::comma)) {
    PP.Diag(Tok.getLocation(), diag::err_pragma_detect_mismatch_malformed);
    return;
  }

  if (!PP.LexStringLiteral(Tok, ValueString, "pragma detect_mismatch",
                           /*MacroExpansion=*/true))
    return;

  if (Tok.isNot(tok::r_paren)) {
    PP.Diag(Tok.getLocation(), diag::err_expected) << tok::r_paren;
    return;
  }
  PP.Lex(Tok);  // Eat the r_paren.

  if (Tok.isNot(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::err_pragma_detect_mismatch_malformed);
    return;
  }

  // If the pragma is lexically sound, notify any interested PPCallbacks.
  if (PP.getPPCallbacks())
    PP.getPPCallbacks()->PragmaDetectMismatch(CommentLoc, NameString,
                                              ValueString);

  Actions.ActOnPragmaDetectMismatch(NameString, ValueString);
}

/// \brief Handle the microsoft \#pragma comment extension.
///
/// The syntax is:
/// \code
///   #pragma comment(linker, "foo")
/// \endcode
/// 'linker' is one of five identifiers: compiler, exestr, lib, linker, user.
/// "foo" is a string, which is fully macro expanded, and permits string
/// concatenation, embedded escape characters etc.  See MSDN for more details.
void PragmaCommentHandler::HandlePragma(Preprocessor &PP,
                                        PragmaIntroducerKind Introducer,
                                        Token &Tok) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  SourceLocation CommentLoc = Tok.getLocation();
  PP.Lex(Tok);
  if (Tok.isNot(tok::l_paren)) {
    PP.Diag(CommentLoc, diag::err_pragma_comment_malformed);
    return;
  }

  // Read the identifier.
  PP.Lex(Tok);
  if (Tok.isNot(tok::identifier)) {
    PP.Diag(CommentLoc, diag::err_pragma_comment_malformed);
    return;
  }

  // Verify that this is one of the 5 whitelisted options.
  IdentifierInfo *II = Tok.getIdentifierInfo();
  Sema::PragmaMSCommentKind Kind =
    llvm::StringSwitch<Sema::PragmaMSCommentKind>(II->getName())
    .Case("linker",   Sema::PCK_Linker)
    .Case("lib",      Sema::PCK_Lib)
    .Case("compiler", Sema::PCK_Compiler)
    .Case("exestr",   Sema::PCK_ExeStr)
    .Case("user",     Sema::PCK_User)
    .Default(Sema::PCK_Unknown);
  if (Kind == Sema::PCK_Unknown) {
    PP.Diag(Tok.getLocation(), diag::err_pragma_comment_unknown_kind);
    return;
  }

  // On PS4, issue a warning about any pragma comments other than
  // #pragma comment lib.
  if (PP.getTargetInfo().getTriple().isPS4() && Kind != Sema::PCK_Lib) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_comment_ignored)
      << II->getName();
    return;
  }

  // Read the optional string if present.
  PP.Lex(Tok);
  std::string ArgumentString;
  if (Tok.is(tok::comma) && !PP.LexStringLiteral(Tok, ArgumentString,
                                                 "pragma comment",
                                                 /*MacroExpansion=*/true))
    return;

  // FIXME: warn that 'exestr' is deprecated.
  // FIXME: If the kind is "compiler" warn if the string is present (it is
  // ignored).
  // The MSDN docs say that "lib" and "linker" require a string and have a short
  // whitelist of linker options they support, but in practice MSVC doesn't
  // issue a diagnostic.  Therefore neither does clang.

  if (Tok.isNot(tok::r_paren)) {
    PP.Diag(Tok.getLocation(), diag::err_pragma_comment_malformed);
    return;
  }
  PP.Lex(Tok);  // eat the r_paren.

  if (Tok.isNot(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::err_pragma_comment_malformed);
    return;
  }

  // If the pragma is lexically sound, notify any interested PPCallbacks.
  if (PP.getPPCallbacks())
    PP.getPPCallbacks()->PragmaComment(CommentLoc, II, ArgumentString);

  Actions.ActOnPragmaMSComment(Kind, ArgumentString);
}

// #pragma clang optimize off
// #pragma clang optimize on
void PragmaOptimizeHandler::HandlePragma(Preprocessor &PP, 
                                        PragmaIntroducerKind Introducer,
                                        Token &FirstToken) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  Token Tok;
  PP.Lex(Tok);
  if (Tok.is(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::err_pragma_missing_argument)
        << "clang optimize" << /*Expected=*/true << "'on' or 'off'";
    return;
  }
  if (Tok.isNot(tok::identifier)) {
    PP.Diag(Tok.getLocation(), diag::err_pragma_optimize_invalid_argument)
      << PP.getSpelling(Tok);
    return;
  }
  const IdentifierInfo *II = Tok.getIdentifierInfo();
  // The only accepted values are 'on' or 'off'.
  bool IsOn = false;
  if (II->isStr("on")) {
    IsOn = true;
  } else if (!II->isStr("off")) {
    PP.Diag(Tok.getLocation(), diag::err_pragma_optimize_invalid_argument)
      << PP.getSpelling(Tok);
    return;
  }
  PP.Lex(Tok);
  
  if (Tok.isNot(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::err_pragma_optimize_extra_argument)
      << PP.getSpelling(Tok);
    return;
  }

  Actions.ActOnPragmaOptimize(IsOn, FirstToken.getLocation());
}

/// \brief Parses loop or unroll pragma hint value and fills in Info.
static bool ParseLoopHintValue(Preprocessor &PP, Token &Tok, Token PragmaName,
                               Token Option, bool ValueInParens,
                               PragmaLoopHintInfo &Info) {
  SmallVector<Token, 1> ValueList;
  int OpenParens = ValueInParens ? 1 : 0;
  // Read constant expression.
  while (Tok.isNot(tok::eod)) {
    if (Tok.is(tok::l_paren))
      OpenParens++;
    else if (Tok.is(tok::r_paren)) {
      OpenParens--;
      if (OpenParens == 0 && ValueInParens)
        break;
    }

    ValueList.push_back(Tok);
    PP.Lex(Tok);
  }

  if (ValueInParens) {
    // Read ')'
    if (Tok.isNot(tok::r_paren)) {
      PP.Diag(Tok.getLocation(), diag::err_expected) << tok::r_paren;
      return true;
    }
    PP.Lex(Tok);
  }

  Token EOFTok;
  EOFTok.startToken();
  EOFTok.setKind(tok::eof);
  EOFTok.setLocation(Tok.getLocation());
  ValueList.push_back(EOFTok); // Terminates expression for parsing.

  Token *TokenArray = (Token *)PP.getPreprocessorAllocator().Allocate(
      ValueList.size() * sizeof(Token), llvm::alignOf<Token>());
  std::copy(ValueList.begin(), ValueList.end(), TokenArray);
  Info.Toks = TokenArray;
  Info.TokSize = ValueList.size();

  Info.PragmaName = PragmaName;
  Info.Option = Option;
  return false;
}

/// \brief Handle the \#pragma clang loop directive.
///  #pragma clang 'loop' loop-hints
///
///  loop-hints:
///    loop-hint loop-hints[opt]
///
///  loop-hint:
///    'vectorize' '(' loop-hint-keyword ')'
///    'interleave' '(' loop-hint-keyword ')'
///    'unroll' '(' unroll-hint-keyword ')'
///    'vectorize_width' '(' loop-hint-value ')'
///    'interleave_count' '(' loop-hint-value ')'
///    'unroll_count' '(' loop-hint-value ')'
///
///  loop-hint-keyword:
///    'enable'
///    'disable'
///    'assume_safety'
///
///  unroll-hint-keyword:
///    'full'
///    'disable'
///
///  loop-hint-value:
///    constant-expression
///
/// Specifying vectorize(enable) or vectorize_width(_value_) instructs llvm to
/// try vectorizing the instructions of the loop it precedes. Specifying
/// interleave(enable) or interleave_count(_value_) instructs llvm to try
/// interleaving multiple iterations of the loop it precedes. The width of the
/// vector instructions is specified by vectorize_width() and the number of
/// interleaved loop iterations is specified by interleave_count(). Specifying a
/// value of 1 effectively disables vectorization/interleaving, even if it is
/// possible and profitable, and 0 is invalid. The loop vectorizer currently
/// only works on inner loops.
///
/// The unroll and unroll_count directives control the concatenation
/// unroller. Specifying unroll(full) instructs llvm to try to
/// unroll the loop completely, and unroll(disable) disables unrolling
/// for the loop. Specifying unroll_count(_value_) instructs llvm to
/// try to unroll the loop the number of times indicated by the value.
void PragmaLoopHintHandler::HandlePragma(Preprocessor &PP,
                                         PragmaIntroducerKind Introducer,
                                         Token &Tok) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  // Incoming token is "loop" from "#pragma clang loop".
  Token PragmaName = Tok;
  SmallVector<Token, 1> TokenList;

  // Lex the optimization option and verify it is an identifier.
  PP.Lex(Tok);
  if (Tok.isNot(tok::identifier)) {
    PP.Diag(Tok.getLocation(), diag::err_pragma_loop_invalid_option)
        << /*MissingOption=*/true << "";
    return;
  }

  while (Tok.is(tok::identifier)) {
    Token Option = Tok;
    IdentifierInfo *OptionInfo = Tok.getIdentifierInfo();

    bool OptionValid = llvm::StringSwitch<bool>(OptionInfo->getName())
                           .Case("vectorize", true)
                           .Case("interleave", true)
                           .Case("unroll", true)
                           .Case("vectorize_width", true)
                           .Case("interleave_count", true)
                           .Case("unroll_count", true)
                           .Default(false);
    if (!OptionValid) {
      PP.Diag(Tok.getLocation(), diag::err_pragma_loop_invalid_option)
          << /*MissingOption=*/false << OptionInfo;
      return;
    }
    PP.Lex(Tok);

    // Read '('
    if (Tok.isNot(tok::l_paren)) {
      PP.Diag(Tok.getLocation(), diag::err_expected) << tok::l_paren;
      return;
    }
    PP.Lex(Tok);

    auto *Info = new (PP.getPreprocessorAllocator()) PragmaLoopHintInfo;
    if (ParseLoopHintValue(PP, Tok, PragmaName, Option, /*ValueInParens=*/true,
                           *Info))
      return;

    // Generate the loop hint token.
    Token LoopHintTok;
    LoopHintTok.startToken();
    LoopHintTok.setKind(tok::annot_pragma_loop_hint);
    LoopHintTok.setLocation(PragmaName.getLocation());
    LoopHintTok.setAnnotationEndLoc(PragmaName.getLocation());
    LoopHintTok.setAnnotationValue(static_cast<void *>(Info));
    TokenList.push_back(LoopHintTok);
  }

  if (Tok.isNot(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol)
        << "clang loop";
    return;
  }

  Token *TokenArray = new Token[TokenList.size()];
  std::copy(TokenList.begin(), TokenList.end(), TokenArray);

  PP.EnterTokenStream(TokenArray, TokenList.size(),
                      /*DisableMacroExpansion=*/false,
                      /*OwnsTokens=*/true);
}

/// \brief Handle the loop unroll optimization pragmas.
///  #pragma unroll
///  #pragma unroll unroll-hint-value
///  #pragma unroll '(' unroll-hint-value ')'
///  #pragma nounroll
///
///  unroll-hint-value:
///    constant-expression
///
/// Loop unrolling hints can be specified with '#pragma unroll' or
/// '#pragma nounroll'. '#pragma unroll' can take a numeric argument optionally
/// contained in parentheses. With no argument the directive instructs llvm to
/// try to unroll the loop completely. A positive integer argument can be
/// specified to indicate the number of times the loop should be unrolled.  To
/// maximize compatibility with other compilers the unroll count argument can be
/// specified with or without parentheses.  Specifying, '#pragma nounroll'
/// disables unrolling of the loop.
void PragmaUnrollHintHandler::HandlePragma(Preprocessor &PP,
                                           PragmaIntroducerKind Introducer,
                                           Token &Tok) {
  assert(!PP.getLangOpts().HLSL && "not supported in HLSL - unreachable"); // HLSL Change
  // Incoming token is "unroll" for "#pragma unroll", or "nounroll" for
  // "#pragma nounroll".
  Token PragmaName = Tok;
  PP.Lex(Tok);
  auto *Info = new (PP.getPreprocessorAllocator()) PragmaLoopHintInfo;
  if (Tok.is(tok::eod)) {
    // nounroll or unroll pragma without an argument.
    Info->PragmaName = PragmaName;
    Info->Option.startToken();
  } else if (PragmaName.getIdentifierInfo()->getName() == "nounroll") {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol)
        << "nounroll";
    return;
  } else {
    // Unroll pragma with an argument: "#pragma unroll N" or
    // "#pragma unroll(N)".
    // Read '(' if it exists.
    bool ValueInParens = Tok.is(tok::l_paren);
    if (ValueInParens)
      PP.Lex(Tok);

    Token Option;
    Option.startToken();
    if (ParseLoopHintValue(PP, Tok, PragmaName, Option, ValueInParens, *Info))
      return;

    // In CUDA, the argument to '#pragma unroll' should not be contained in
    // parentheses.
    if (PP.getLangOpts().CUDA && ValueInParens)
      PP.Diag(Info->Toks[0].getLocation(),
              diag::warn_pragma_unroll_cuda_value_in_parens);

    if (Tok.isNot(tok::eod)) {
      PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol)
          << "unroll";
      return;
    }
  }

  // Generate the hint token.
  Token *TokenArray = new Token[1];
  TokenArray[0].startToken();
  TokenArray[0].setKind(tok::annot_pragma_loop_hint);
  TokenArray[0].setLocation(PragmaName.getLocation());
  TokenArray[0].setAnnotationEndLoc(PragmaName.getLocation());
  TokenArray[0].setAnnotationValue(static_cast<void *>(Info));
  PP.EnterTokenStream(TokenArray, 1, /*DisableMacroExpansion=*/false,
                      /*OwnsTokens=*/true);
}

// HLSL Change Begin - pack_matrix
/// \brief Handle the pack_matrix pragmas.
///  #pragma pack_matrix(row_major)
///  #pragma pack_matrix(column_major)
///
void PragmaPackMatrixHandler::HandlePragma(Preprocessor &PP,
                                           PragmaIntroducerKind Introducer,
                                           Token &Tok) {
  assert(PP.getLangOpts().HLSL && "only supported in HLSL");
  PP.Lex(Tok);
  if (!Tok.is(tok::l_paren)) {
    PP.Diag(Tok, diag::err_expected) << tok::l_brace;
    return;
  }

  PP.Lex(Tok);
  Token PragmaArg = Tok;
  bool bRowMajor = false;
  if (Tok.is(tok::kw_row_major)) {
    bRowMajor = true;
  }
  else if (Tok.isNot(tok::kw_column_major)) {
    PP.Diag(Tok.getLocation(), diag::err_pragma_invalid_keyword);
    return;
  }
  // Make sure pragma finish correctly.
  PP.Lex(Tok);
  if (Tok.isNot(tok::r_paren)) {
    PP.Diag(Tok, diag::err_expected) << tok::r_brace;
    return;
  }
  PP.Lex(Tok);
  if (Tok.isNot(tok::eod)) {
    PP.Diag(Tok.getLocation(), diag::warn_pragma_extra_tokens_at_eol);
    return;
  }
  // Note: to make things easy, pack_matrix will modify ast type directly in
  // Sema::TransferUnusualAttributes.
  // Another solution is create ast node for pack_matrix, and take care it at
  // clang codegen.
  Actions.ActOnPragmaPackMatrix(bRowMajor, PragmaArg.getLocation());
}
// HLSL Change End.
