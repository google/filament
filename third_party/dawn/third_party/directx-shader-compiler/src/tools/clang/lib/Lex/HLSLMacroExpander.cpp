//===--- HLSLMacroExpander.cpp - Standalone Macro expansion -----*- C++ -*-===//
//                                                                            //
// HLSLMacroExpander.cpp                                                      //
// Copyright (C) Microsoft Corporation. All rights reserved.                  //
// This file is distributed under the University of Illinois Open Source      //
// License. See LICENSE.TXT for details.                                      //
//===----------------------------------------------------------------------===//
//
// This file implements the MacroExpander class.
//
//===----------------------------------------------------------------------===//
#include "clang/Lex/HLSLMacroExpander.h"

#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/ModuleMap.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/PTHLexer.h"
#include "clang/Lex/PTHManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Token.h"
#include "clang/Lex/TokenLexer.h"
#include "llvm/ADT/StringRef.h"

#include "dxc/Support/Global.h"
using namespace clang;
using namespace llvm;
using namespace hlsl;

MacroExpander::MacroExpander(Preprocessor &PP_, unsigned options)
    : PP(PP_), m_expansionFileId(), m_stripQuotes(false) {
  if (options & STRIP_QUOTES)
    m_stripQuotes = true;

  // The preprocess requires a file to be on the lexing stack when we
  // call ExpandMacro. We add an empty in-memory buffer that we use
  // just for expanding macros.
  std::unique_ptr<llvm::MemoryBuffer> SB =
      llvm::MemoryBuffer::getMemBuffer("", "<hlsl-semantic-defines>");
  if (!SB) {
    DXASSERT(false, "Cannot create macro expansion source buffer");
    throw hlsl::Exception(DXC_E_MACRO_EXPANSION_FAILURE);
  }

  // Unfortunately, there is no api in the SourceManager to lookup a
  // previously added file, so we have to add the empty file every time
  // we expand macros. We could modify source manager to get/set the
  // macro file id similar to the one we have for getPreambleFileID.
  // Macros should only be expanded once (if needed for a root signature)
  // or twice (for semantic defines) so adding an empty file every time
  // is probably not a big deal.
  m_expansionFileId = PP.getSourceManager().createFileID(std::move(SB));
  if (m_expansionFileId.isInvalid()) {
    DXASSERT(false, "Could not create FileID for macro expnasion?");
    throw hlsl::Exception(DXC_E_MACRO_EXPANSION_FAILURE);
  }
}

// Simple struct to hold a data/length pair.
struct LiteralData {
  const char *Data;
  unsigned Length;
};

// Get the literal data from a literal token.
// If stripQuotes flag is true the quotes (and string literal type) will
// be removed from the data and only the raw string literal value will be
// returned.
static LiteralData GetLiteralData(const Token &Tok, bool stripQuotes) {
  if (!tok::isStringLiteral(Tok.getKind()))
    return LiteralData{Tok.getLiteralData(), Tok.getLength()};

  unsigned start_offset = 0;
  unsigned end_offset = 0;
  switch (Tok.getKind()) {
  case tok::string_literal:
    start_offset = 1;
    end_offset = 1;
    break; // "foo"
  case tok::wide_string_literal:
    start_offset = 2;
    end_offset = 1;
    break; // L"foo"
  case tok::utf8_string_literal:
    start_offset = 3;
    end_offset = 1;
    break; // u8"foo"
  case tok::utf16_string_literal:
    start_offset = 2;
    end_offset = 1;
    break; // u"foo"
  case tok::utf32_string_literal:
    start_offset = 2;
    end_offset = 1;
    break; // U"foo"
  default:
    break;
  }

  unsigned length = Tok.getLength() - (start_offset + end_offset);
  if (length > Tok.getLength()) { // Check for unsigned underflow.
    DXASSERT(false, "string literal quote count is wrong?");
    start_offset = 0;
    length = Tok.getLength();
  }

  return LiteralData{Tok.getLiteralData() + start_offset, length};
}

// Print leading spaces if needed by the token.
// Take care when stripping string literal quoates that we do not add extra
// spaces to the output.
static bool ShouldPrintLeadingSpace(const Token &Tok, const Token &PrevTok,
                                    bool stripQuotes) {
  if (!Tok.hasLeadingSpace())
    return false;

  // Token has leading spaces, but the previous token was a sting literal
  // and we are stripping quotes to paste the strings together so do not
  // add a space between the string literal values.
  if (tok::isStringLiteral(PrevTok.getKind()) && stripQuotes)
    return false;

  return true;
}

// Macro expansion implementation.
// We re-lex the macro using the preprocessors lexer.
bool MacroExpander::ExpandMacro(MacroInfo *pMacro, std::string *out) {
  if (!pMacro || !out)
    return false;
  MacroInfo &macro = *pMacro;

  // Initialize the token from the macro definition location.
  Token Tok;
  bool failed = PP.getRawToken(macro.getDefinitionLoc(), Tok);
  if (failed)
    return false;

  // Start the lexing process. Use an outer file to make the preprocessor happy.
  PP.EnterSourceFile(
      m_expansionFileId, nullptr,
      PP.getSourceManager().getLocForStartOfFile(m_expansionFileId));
  PP.EnterMacro(Tok, macro.getDefinitionEndLoc(), &macro, nullptr);
  PP.Lex(Tok);
  llvm::raw_string_ostream OS(*out);

  // Keep track of previous token to print spaces correctly.
  Token PrevTok;
  PrevTok.startToken();

  // Lex all the tokens from the macro and add them to the output.
  while (!Tok.is(tok::eof)) {
    if (ShouldPrintLeadingSpace(Tok, PrevTok, m_stripQuotes)) {
      OS << ' ';
    }
    if (IdentifierInfo *II = Tok.getIdentifierInfo()) {
      OS << II->getName();
    } else if (Tok.isLiteral() && !Tok.needsCleaning() &&
               Tok.getLiteralData()) {
      LiteralData literalData = GetLiteralData(Tok, m_stripQuotes);
      OS.write(literalData.Data, literalData.Length);
    } else {
      std::string S = PP.getSpelling(Tok);
      OS.write(&S[0], S.size());
    }
    PrevTok = Tok;
    PP.Lex(Tok);
  }

  return true;
}

// Search for the macro info by the given name.
MacroInfo *MacroExpander::FindMacroInfo(clang::Preprocessor &PP,
                                        StringRef macroName) {
  // Lookup macro identifier.
  IdentifierInfo *ii = PP.getIdentifierInfo(macroName);
  if (!ii)
    return nullptr;

  // Lookup macro info.
  return PP.getMacroInfo(ii);
}
