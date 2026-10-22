///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "clang/AST/HlslBuiltinTypeDeclBuilder.h"

#include "dxc/Support/Global.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/TypeLoc.h"

using namespace clang;
using namespace hlsl;

static const SourceLocation NoLoc; // no source location attribution available

BuiltinTypeDeclBuilder::BuiltinTypeDeclBuilder(DeclContext *declContext,
                                               StringRef name,
                                               TagDecl::TagKind tagKind) {
  ASTContext &astContext = declContext->getParentASTContext();
  IdentifierInfo &nameId =
      astContext.Idents.get(name, tok::TokenKind::identifier);
  m_recordDecl =
      CXXRecordDecl::Create(astContext, tagKind, declContext, NoLoc, NoLoc,
                            &nameId, nullptr, /* DelayTypeCreation */ true);
  m_recordDecl->setImplicit(true);
}

TemplateTypeParmDecl *BuiltinTypeDeclBuilder::addTypeTemplateParam(
    StringRef name, TypeSourceInfo *defaultValue, bool parameterPack) {
  DXASSERT_NOMSG(!m_recordDecl->isBeingDefined() &&
                 !m_recordDecl->isCompleteDefinition());

  ASTContext &astContext = m_recordDecl->getASTContext();
  unsigned index = (unsigned)m_templateParams.size();
  TemplateTypeParmDecl *decl = TemplateTypeParmDecl::Create(
      astContext, m_recordDecl->getDeclContext(), NoLoc, NoLoc,
      /* TemplateDepth */ 0, index,
      &astContext.Idents.get(name, tok::TokenKind::identifier),
      /* Typename */ false, parameterPack);
  if (defaultValue != nullptr)
    decl->setDefaultArgument(defaultValue);
  m_templateParams.emplace_back(decl);
  return decl;
}

TemplateTypeParmDecl *
BuiltinTypeDeclBuilder::addTypeTemplateParam(StringRef name,
                                             QualType defaultValue) {
  TypeSourceInfo *defaultValueSourceInfo = nullptr;
  if (!defaultValue.isNull())
    defaultValueSourceInfo =
        m_recordDecl->getASTContext().getTrivialTypeSourceInfo(defaultValue);
  return addTypeTemplateParam(name, defaultValueSourceInfo);
}

NonTypeTemplateParmDecl *BuiltinTypeDeclBuilder::addIntegerTemplateParam(
    StringRef name, QualType type, Optional<int64_t> defaultValue) {
  DXASSERT_NOMSG(!m_recordDecl->isBeingDefined() &&
                 !m_recordDecl->isCompleteDefinition());

  ASTContext &astContext = m_recordDecl->getASTContext();
  unsigned index = (unsigned)m_templateParams.size();
  NonTypeTemplateParmDecl *decl = NonTypeTemplateParmDecl::Create(
      astContext, m_recordDecl->getDeclContext(), NoLoc, NoLoc,
      /* TemplateDepth */ 0, index,
      &astContext.Idents.get(name, tok::TokenKind::identifier), type,
      /* ParameterPack */ false, astContext.getTrivialTypeSourceInfo(type));
  if (defaultValue.hasValue()) {
    Expr *defaultValueLiteral = IntegerLiteral::Create(
        astContext,
        llvm::APInt(astContext.getIntWidth(type), defaultValue.getValue()),
        type, NoLoc);
    decl->setDefaultArgument(defaultValueLiteral);
  }
  m_templateParams.emplace_back(decl);
  return decl;
}

void BuiltinTypeDeclBuilder::startDefinition() {
  DXASSERT_NOMSG(!m_recordDecl->isBeingDefined() &&
                 !m_recordDecl->isCompleteDefinition());

  ASTContext &astContext = m_recordDecl->getASTContext();
  DeclContext *declContext = m_recordDecl->getDeclContext();

  if (!m_templateParams.empty()) {
    TemplateParameterList *templateParameterList =
        TemplateParameterList::Create(astContext, NoLoc, NoLoc,
                                      m_templateParams.data(),
                                      m_templateParams.size(), NoLoc);
    m_templateDecl = ClassTemplateDecl::Create(
        astContext, declContext, NoLoc,
        DeclarationName(m_recordDecl->getIdentifier()), templateParameterList,
        m_recordDecl, nullptr);
    m_recordDecl->setDescribedClassTemplate(m_templateDecl);
    m_templateDecl->setImplicit(true);
    m_templateDecl->setLexicalDeclContext(declContext);
    declContext->addDecl(m_templateDecl);

    // Requesting the class name specialization will fault in required types.
    QualType T = m_templateDecl->getInjectedClassNameSpecialization();
    T = astContext.getInjectedClassNameType(m_recordDecl, T);
    assert(T->isDependentType() && "Class template type is not dependent?");
  } else {
    declContext->addDecl(m_recordDecl);
  }

  m_recordDecl->setLexicalDeclContext(declContext);
  m_recordDecl->addAttr(
      FinalAttr::CreateImplicit(astContext, FinalAttr::Keyword_final));
  m_recordDecl->startDefinition();
}

FieldDecl *BuiltinTypeDeclBuilder::addField(StringRef name, QualType type,
                                            AccessSpecifier access) {
  DXASSERT_NOMSG(m_recordDecl->isBeingDefined());

  ASTContext &astContext = m_recordDecl->getASTContext();

  IdentifierInfo &nameId =
      astContext.Idents.get(name, tok::TokenKind::identifier);
  TypeSourceInfo *fieldTypeSource =
      astContext.getTrivialTypeSourceInfo(type, NoLoc);
  const bool MutableFalse = false;
  const InClassInitStyle initStyle = InClassInitStyle::ICIS_NoInit;
  FieldDecl *fieldDecl =
      FieldDecl::Create(astContext, m_recordDecl, NoLoc, NoLoc, &nameId, type,
                        fieldTypeSource, nullptr, MutableFalse, initStyle);
  fieldDecl->setAccess(access);
  fieldDecl->setImplicit(true);
  m_recordDecl->addDecl(fieldDecl);

#ifndef NDEBUG
  // Verify that we can read the field member from the record.
  DeclContext::lookup_result lookupResult =
      m_recordDecl->lookup(DeclarationName(&nameId));
  DXASSERT(!lookupResult.empty(), "Field cannot be looked up");
#endif

  return fieldDecl;
}

CXXRecordDecl *BuiltinTypeDeclBuilder::completeDefinition() {
  DXASSERT_NOMSG(!m_recordDecl->isCompleteDefinition());
  if (!m_recordDecl->isBeingDefined())
    startDefinition();
  m_recordDecl->completeDefinition();
  return m_recordDecl;
}

ClassTemplateDecl *BuiltinTypeDeclBuilder::getTemplateDecl() const {
  DXASSERT_NOMSG(m_recordDecl->isBeingDefined() ||
                 m_recordDecl->isCompleteDefinition());
  return m_templateDecl;
}
