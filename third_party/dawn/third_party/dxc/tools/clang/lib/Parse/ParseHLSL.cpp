//===--- ParseHLSL.cpp - HLSL Parsing -------------------------------------===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ParseHLSL.cpp                                                             //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//  This file implements the HLSLportions of the Parser interfaces.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "RAIIObjectsForParser.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/Basic/CharInfo.h"
#include "clang/Basic/OperatorKinds.h"
#include "clang/Parse/ParseDiagnostic.h"
#include "clang/Parse/Parser.h"
#include "clang/Sema/DeclSpec.h"
#include "clang/Sema/ParsedTemplate.h"
#include "clang/Sema/PrettyDeclStackTrace.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/SemaDiagnostic.h"
#include "llvm/ADT/SmallString.h"

using namespace clang;

Decl *Parser::ParseCTBuffer(unsigned Context, SourceLocation &DeclEnd,
                            ParsedAttributesWithRange &CTBAttrs,
                            SourceLocation InlineLoc) {
  assert((Tok.is(tok::kw_cbuffer) || Tok.is(tok::kw_tbuffer)) &&
         "Not a cbuffer or tbuffer!");
  bool isCBuffer = Tok.is(tok::kw_cbuffer);
  SourceLocation BufferLoc = ConsumeToken(); // eat the 'cbuffer or tbuffer'.

  if (!Tok.is(tok::identifier)) {
    Diag(Tok, diag::err_expected) << tok::identifier;
    return nullptr;
  }

  IdentifierInfo *identifier = Tok.getIdentifierInfo();
  SourceLocation identifierLoc = ConsumeToken(); // consume identifier
  std::vector<hlsl::UnusualAnnotation *> hlslAttrs;
  MaybeParseHLSLAttributes(hlslAttrs);

  ParseScope BufferScope(this, Scope::DeclScope);
  BalancedDelimiterTracker T(*this, tok::l_brace);
  if (T.consumeOpen()) {
    Diag(Tok, diag::err_expected) << tok::l_brace;
    return nullptr;
  }

  Decl *decl = Actions.ActOnStartHLSLBuffer(getCurScope(), isCBuffer, BufferLoc,
                                            identifier, identifierLoc,
                                            hlslAttrs, T.getOpenLocation());

  // Process potential C++11 attribute specifiers
  Actions.ProcessDeclAttributeList(getCurScope(), decl, CTBAttrs.getList());

  while (Tok.isNot(tok::r_brace) && Tok.isNot(tok::eof)) {
    ParsedAttributesWithRange attrs(AttrFactory);
    MaybeParseCXX11Attributes(attrs);
    MaybeParseHLSLAttributes(attrs);
    MaybeParseMicrosoftAttributes(attrs);
    ParseExternalDeclaration(attrs);
  }

  T.consumeClose();
  DeclEnd = T.getCloseLocation();
  BufferScope.Exit();
  Actions.ActOnFinishHLSLBuffer(decl, DeclEnd);

  return decl;
}

/// ParseHLSLAttributeSpecifier - Parse an HLSL attribute-specifier.
///
/// [HLSL] attribute-specifier:
///        '[' attribute[opt] ']'
///
/// [HLSL] attribute:
///        attribute-token attribute-argument-clause[opt]
///
/// [HLSL] attribute-token:
///        identifier
///
/// [HLSL] attribute-argument-clause:
///        '(' attribute-params ')'
///
/// [HLSL] attribute-params:
///        constant-expr
///        attribute-params ',' constant-expr
///
void Parser::ParseHLSLAttributeSpecifier(ParsedAttributes &attrs,
                                         SourceLocation *endLoc) {
  assert(getLangOpts().HLSL);
  assert(Tok.is(tok::l_square) && "Not an HLSL attribute list");

  ConsumeBracket();

  llvm::SmallDenseMap<IdentifierInfo *, SourceLocation, 4> SeenAttrs;

  // '[]' is valid.
  if (Tok.is(tok::r_square)) {
    *endLoc = ConsumeBracket();
    return;
  }

  if (!Tok.isAnyIdentifier()) {
    Diag(Tok, diag::err_expected) << tok::identifier;
    SkipUntil(tok::r_square);
    return;
  }

  SourceLocation AttrLoc;
  IdentifierInfo *AttrName = 0;

  AttrName = TryParseCXX11AttributeIdentifier(AttrLoc);
  assert(AttrName != nullptr && "already called isAnyIdenfier before");

  // Parse attribute arguments
  if (Tok.is(tok::l_paren)) {
    ParseGNUAttributeArgs(AttrName, AttrLoc, attrs, endLoc, nullptr,
                          SourceLocation(), AttributeList::AS_CXX11, nullptr);
  } else {
    attrs.addNew(AttrName, AttrLoc, nullptr, SourceLocation(), 0, 0,
                 AttributeList::AS_CXX11);
  }

  if (endLoc)
    *endLoc = Tok.getLocation();
  if (ExpectAndConsume(tok::r_square, diag::err_expected))
    SkipUntil(tok::r_square);
}

/// ParseHLSLAttributes - Parse an HLSL attribute-specifier-seq.
///
/// attribute-specifier-seq:
///       attribute-specifier-seq[opt] attribute-specifier
void Parser::ParseHLSLAttributes(ParsedAttributesWithRange &attrs,
                                 SourceLocation *endLoc) {
  assert(getLangOpts().HLSL);
  SourceLocation StartLoc = Tok.getLocation(), Loc;
  if (!endLoc)
    endLoc = &Loc;

  do {
    ParseHLSLAttributeSpecifier(attrs, endLoc);
  } while (Tok.is(tok::l_square));

  attrs.Range = SourceRange(StartLoc, *endLoc);
}
