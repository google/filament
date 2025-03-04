//===--- TemplateName.cpp - C++ Template Name Representation---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the TemplateName interface and subclasses.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/TemplateName.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/NestedNameSpecifier.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/TemplateBase.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/LangOptions.h"
#include "llvm/Support/raw_ostream.h"
using namespace clang;
using namespace llvm;

TemplateArgument 
SubstTemplateTemplateParmPackStorage::getArgumentPack() const {
  return TemplateArgument(Arguments, size());
}

void SubstTemplateTemplateParmStorage::Profile(llvm::FoldingSetNodeID &ID) {
  Profile(ID, Parameter, Replacement);
}

void SubstTemplateTemplateParmStorage::Profile(llvm::FoldingSetNodeID &ID, 
                                           TemplateTemplateParmDecl *parameter,
                                               TemplateName replacement) {
  ID.AddPointer(parameter);
  ID.AddPointer(replacement.getAsVoidPointer());
}

void SubstTemplateTemplateParmPackStorage::Profile(llvm::FoldingSetNodeID &ID,
                                                   ASTContext &Context) {
  Profile(ID, Context, Parameter, TemplateArgument(Arguments, size()));
}

void SubstTemplateTemplateParmPackStorage::Profile(llvm::FoldingSetNodeID &ID, 
                                                   ASTContext &Context,
                                           TemplateTemplateParmDecl *Parameter,
                                             const TemplateArgument &ArgPack) {
  ID.AddPointer(Parameter);
  ArgPack.Profile(ID, Context);
}

TemplateName::NameKind TemplateName::getKind() const {
  if (Storage.is<TemplateDecl *>())
    return Template;
  if (Storage.is<DependentTemplateName *>())
    return DependentTemplate;
  if (Storage.is<QualifiedTemplateName *>())
    return QualifiedTemplate;

  UncommonTemplateNameStorage *uncommon
    = Storage.get<UncommonTemplateNameStorage*>();
  if (uncommon->getAsOverloadedStorage())
    return OverloadedTemplate;
  if (uncommon->getAsSubstTemplateTemplateParm())
    return SubstTemplateTemplateParm;
  return SubstTemplateTemplateParmPack;
}

TemplateDecl *TemplateName::getAsTemplateDecl() const {
  if (TemplateDecl *Template = Storage.dyn_cast<TemplateDecl *>())
    return Template;

  if (QualifiedTemplateName *QTN = getAsQualifiedTemplateName())
    return QTN->getTemplateDecl();

  if (SubstTemplateTemplateParmStorage *sub = getAsSubstTemplateTemplateParm())
    return sub->getReplacement().getAsTemplateDecl();

  return nullptr;
}

bool TemplateName::isDependent() const {
  if (TemplateDecl *Template = getAsTemplateDecl()) {
    if (isa<TemplateTemplateParmDecl>(Template))
      return true;
    // FIXME: Hack, getDeclContext() can be null if Template is still
    // initializing due to PCH reading, so we check it before using it.
    // Should probably modify TemplateSpecializationType to allow constructing
    // it without the isDependent() checking.
    return Template->getDeclContext() &&
           Template->getDeclContext()->isDependentContext();
  }

  assert(!getAsOverloadedTemplate() &&
         "overloaded templates shouldn't survive to here");

  return true;
}

bool TemplateName::isInstantiationDependent() const {
  if (QualifiedTemplateName *QTN = getAsQualifiedTemplateName()) {
    if (QTN->getQualifier()->isInstantiationDependent())
      return true;
  }
  
  return isDependent();
}

bool TemplateName::containsUnexpandedParameterPack() const {
  if (TemplateDecl *Template = getAsTemplateDecl()) {
    if (TemplateTemplateParmDecl *TTP 
                                  = dyn_cast<TemplateTemplateParmDecl>(Template))
      return TTP->isParameterPack();

    return false;
  }

  if (DependentTemplateName *DTN = getAsDependentTemplateName())
    return DTN->getQualifier() && 
      DTN->getQualifier()->containsUnexpandedParameterPack();

  return getAsSubstTemplateTemplateParmPack() != nullptr;
}

void
TemplateName::print(raw_ostream &OS, const PrintingPolicy &Policy,
                    bool SuppressNNS) const {
  if (TemplateDecl *Template = Storage.dyn_cast<TemplateDecl *>())
    OS << *Template;
  else if (QualifiedTemplateName *QTN = getAsQualifiedTemplateName()) {
    if (!SuppressNNS)
      QTN->getQualifier()->print(OS, Policy);
    if (QTN->hasTemplateKeyword())
      OS << "template ";
    OS << *QTN->getDecl();
  } else if (DependentTemplateName *DTN = getAsDependentTemplateName()) {
    if (!SuppressNNS && DTN->getQualifier())
      DTN->getQualifier()->print(OS, Policy);
    OS << "template ";
    
    if (DTN->isIdentifier())
      OS << DTN->getIdentifier()->getName();
    else
      OS << "operator " << getOperatorSpelling(DTN->getOperator());
  } else if (SubstTemplateTemplateParmStorage *subst
               = getAsSubstTemplateTemplateParm()) {
    subst->getReplacement().print(OS, Policy, SuppressNNS);
  } else if (SubstTemplateTemplateParmPackStorage *SubstPack
                                        = getAsSubstTemplateTemplateParmPack())
    OS << *SubstPack->getParameterPack();
  else {
    OverloadedTemplateStorage *OTS = getAsOverloadedTemplate();
    (*OTS->begin())->printName(OS);
  }
}

const DiagnosticBuilder &clang::operator<<(const DiagnosticBuilder &DB,
                                           TemplateName N) {
  std::string NameStr;
  raw_string_ostream OS(NameStr);
  LangOptions LO;
#ifdef MS_SUPPORT_VARIABLE_LANGOPTS
  LO.CPlusPlus = true;
  LO.Bool = true;
#endif
  OS << '\'';
  N.print(OS, PrintingPolicy(LO));
  OS << '\'';
  OS.flush();
  return DB << NameStr;
}

void TemplateName::dump(raw_ostream &OS) const {
  LangOptions LO;  // FIXME!
#ifdef MS_SUPPORT_VARIABLE_LANGOPTS
  LO.CPlusPlus = true;
  LO.Bool = true;
#endif
  print(OS, PrintingPolicy(LO));
}

void TemplateName::dump() const {
  dump(llvm::errs());
}
