//===--- MacroInfo.cpp - Information about #defined identifiers -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the MacroInfo interface.
//
//===----------------------------------------------------------------------===//

#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/Preprocessor.h"
using namespace clang;

MacroInfo::MacroInfo(SourceLocation DefLoc)
  : Location(DefLoc),
    ArgumentList(nullptr),
    NumArguments(0),
    IsDefinitionLengthCached(false),
    IsFunctionLike(false),
    IsC99Varargs(false),
    IsGNUVarargs(false),
    IsBuiltinMacro(false),
    HasCommaPasting(false),
    IsDisabled(false),
    IsUsed(false),
    IsAllowRedefinitionsWithoutWarning(false),
    IsWarnIfUnused(false),
    FromASTFile(false),
    UsedForHeaderGuard(false) {
}

unsigned MacroInfo::getDefinitionLengthSlow(SourceManager &SM) const {
  assert(!IsDefinitionLengthCached);
  IsDefinitionLengthCached = true;

  if (ReplacementTokens.empty())
    return (DefinitionLength = 0);

  const Token &firstToken = ReplacementTokens.front();
  const Token &lastToken = ReplacementTokens.back();
  SourceLocation macroStart = firstToken.getLocation();
  SourceLocation macroEnd = lastToken.getLocation();
  assert(macroStart.isValid() && macroEnd.isValid());
  assert((macroStart.isFileID() || firstToken.is(tok::comment)) &&
         "Macro defined in macro?");
  assert((macroEnd.isFileID() || lastToken.is(tok::comment)) &&
         "Macro defined in macro?");
  std::pair<FileID, unsigned>
      startInfo = SM.getDecomposedExpansionLoc(macroStart);
  std::pair<FileID, unsigned>
      endInfo = SM.getDecomposedExpansionLoc(macroEnd);
  assert(startInfo.first == endInfo.first &&
         "Macro definition spanning multiple FileIDs ?");
  assert(startInfo.second <= endInfo.second);
  DefinitionLength = endInfo.second - startInfo.second;
  DefinitionLength += lastToken.getLength();

  return DefinitionLength;
}

/// \brief Return true if the specified macro definition is equal to
/// this macro in spelling, arguments, and whitespace.
///
/// \param Syntactically if true, the macro definitions can be identical even
/// if they use different identifiers for the function macro parameters.
/// Otherwise the comparison is lexical and this implements the rules in
/// C99 6.10.3.
bool MacroInfo::isIdenticalTo(const MacroInfo &Other, Preprocessor &PP,
                              bool Syntactically) const {
  bool Lexically = !Syntactically;

  // Check # tokens in replacement, number of args, and various flags all match.
  if (ReplacementTokens.size() != Other.ReplacementTokens.size() ||
      getNumArgs() != Other.getNumArgs() ||
      isFunctionLike() != Other.isFunctionLike() ||
      isC99Varargs() != Other.isC99Varargs() ||
      isGNUVarargs() != Other.isGNUVarargs())
    return false;

  if (Lexically) {
    // Check arguments.
    for (arg_iterator I = arg_begin(), OI = Other.arg_begin(), E = arg_end();
         I != E; ++I, ++OI)
      if (*I != *OI) return false;
  }

  // Check all the tokens.
  for (unsigned i = 0, e = ReplacementTokens.size(); i != e; ++i) {
    const Token &A = ReplacementTokens[i];
    const Token &B = Other.ReplacementTokens[i];
    if (A.getKind() != B.getKind())
      return false;

    // If this isn't the first first token, check that the whitespace and
    // start-of-line characteristics match.
    if (i != 0 &&
        (A.isAtStartOfLine() != B.isAtStartOfLine() ||
         A.hasLeadingSpace() != B.hasLeadingSpace()))
      return false;

    // If this is an identifier, it is easy.
    if (A.getIdentifierInfo() || B.getIdentifierInfo()) {
      if (A.getIdentifierInfo() == B.getIdentifierInfo())
        continue;
      if (Lexically)
        return false;
      // With syntactic equivalence the parameter names can be different as long
      // as they are used in the same place.
      int AArgNum = getArgumentNum(A.getIdentifierInfo());
      if (AArgNum == -1)
        return false;
      if (AArgNum != Other.getArgumentNum(B.getIdentifierInfo()))
        return false;
      continue;
    }

    // Otherwise, check the spelling.
    if (PP.getSpelling(A) != PP.getSpelling(B))
      return false;
  }

  return true;
}

void MacroInfo::dump() const {
  llvm::raw_ostream &Out = llvm::errs();

  // FIXME: Dump locations.
  Out << "MacroInfo " << this;
  if (IsBuiltinMacro) Out << " builtin";
  if (IsDisabled) Out << " disabled";
  if (IsUsed) Out << " used";
  if (IsAllowRedefinitionsWithoutWarning)
    Out << " allow_redefinitions_without_warning";
  if (IsWarnIfUnused) Out << " warn_if_unused";
  if (FromASTFile) Out << " imported";
  if (UsedForHeaderGuard) Out << " header_guard";

  Out << "\n    #define <macro>";
  if (IsFunctionLike) {
    Out << "(";
    for (unsigned I = 0; I != NumArguments; ++I) {
      if (I) Out << ", ";
      Out << ArgumentList[I]->getName();
    }
    if (IsC99Varargs || IsGNUVarargs) {
      if (NumArguments && IsC99Varargs) Out << ", ";
      Out << "...";
    }
    Out << ")";
  }

  for (const Token &Tok : ReplacementTokens) {
    Out << " ";
    if (const char *Punc = tok::getPunctuatorSpelling(Tok.getKind()))
      Out << Punc;
    else if (const char *Kwd = tok::getKeywordSpelling(Tok.getKind()))
      Out << Kwd;
    else if (Tok.is(tok::identifier))
      Out << Tok.getIdentifierInfo()->getName();
    else if (Tok.isLiteral() && Tok.getLiteralData())
      Out << StringRef(Tok.getLiteralData(), Tok.getLength());
    else
      Out << Tok.getName();
  }
}

MacroDirective::DefInfo MacroDirective::getDefinition() {
  MacroDirective *MD = this;
  SourceLocation UndefLoc;
  Optional<bool> isPublic;
  for (; MD; MD = MD->getPrevious()) {
    if (DefMacroDirective *DefMD = dyn_cast<DefMacroDirective>(MD))
      return DefInfo(DefMD, UndefLoc,
                     !isPublic.hasValue() || isPublic.getValue());

    if (UndefMacroDirective *UndefMD = dyn_cast<UndefMacroDirective>(MD)) {
      UndefLoc = UndefMD->getLocation();
      continue;
    }

    VisibilityMacroDirective *VisMD = cast<VisibilityMacroDirective>(MD);
    if (!isPublic.hasValue())
      isPublic = VisMD->isPublic();
  }

  return DefInfo(nullptr, UndefLoc,
                 !isPublic.hasValue() || isPublic.getValue());
}

const MacroDirective::DefInfo
MacroDirective::findDirectiveAtLoc(SourceLocation L, SourceManager &SM) const {
  assert(L.isValid() && "SourceLocation is invalid.");
  for (DefInfo Def = getDefinition(); Def; Def = Def.getPreviousDefinition()) {
    if (Def.getLocation().isInvalid() ||  // For macros defined on the command line.
        SM.isBeforeInTranslationUnit(Def.getLocation(), L))
      return (!Def.isUndefined() ||
              SM.isBeforeInTranslationUnit(L, Def.getUndefLocation()))
                  ? Def : DefInfo();
  }
  return DefInfo();
}

void MacroDirective::dump() const {
  llvm::raw_ostream &Out = llvm::errs();

  switch (getKind()) {
  case MD_Define: Out << "DefMacroDirective"; break;
  case MD_Undefine: Out << "UndefMacroDirective"; break;
  case MD_Visibility: Out << "VisibilityMacroDirective"; break;
  }
  Out << " " << this;
  // FIXME: Dump SourceLocation.
  if (auto *Prev = getPrevious())
    Out << " prev " << Prev;
  if (IsFromPCH) Out << " from_pch";

  if (isa<VisibilityMacroDirective>(this))
    Out << (IsPublic ? " public" : " private");

  if (auto *DMD = dyn_cast<DefMacroDirective>(this)) {
    if (auto *Info = DMD->getInfo()) {
      Out << "\n  ";
      Info->dump();
    }
  }
  Out << "\n";
}

ModuleMacro *ModuleMacro::create(Preprocessor &PP, Module *OwningModule,
                                 IdentifierInfo *II, MacroInfo *Macro,
                                 ArrayRef<ModuleMacro *> Overrides) {
  void *Mem = PP.getPreprocessorAllocator().Allocate(
      sizeof(ModuleMacro) + sizeof(ModuleMacro *) * Overrides.size(),
      llvm::alignOf<ModuleMacro>());
  return new (Mem) ModuleMacro(OwningModule, II, Macro, Overrides);
}
