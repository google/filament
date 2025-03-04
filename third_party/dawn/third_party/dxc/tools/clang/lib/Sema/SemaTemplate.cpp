//===------- SemaTemplate.cpp - Semantic Analysis for C++ Templates -------===/
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//===----------------------------------------------------------------------===/
//
//  This file implements semantic analysis for C++ templates.
//===----------------------------------------------------------------------===/

#include "TreeTransform.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclFriend.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/TypeVisitor.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/PartialDiagnostic.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Sema/DeclSpec.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/ParsedTemplate.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/SemaInternal.h"
#include "clang/Sema/Template.h"
#include "clang/Sema/TemplateDeduction.h"
#include "clang/Sema/SemaHLSL.h" // HLSL Change
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
using namespace clang;
using namespace sema;

// Exported for use by Parser.
SourceRange
clang::getTemplateParamsRange(TemplateParameterList const * const *Ps,
                              unsigned N) {
  if (!N) return SourceRange();
  return SourceRange(Ps[0]->getTemplateLoc(), Ps[N-1]->getRAngleLoc());
}

/// \brief Determine whether the declaration found is acceptable as the name
/// of a template and, if so, return that template declaration. Otherwise,
/// returns NULL.
static NamedDecl *isAcceptableTemplateName(ASTContext &Context,
                                           NamedDecl *Orig,
                                           bool AllowFunctionTemplates) {
  NamedDecl *D = Orig->getUnderlyingDecl();

  if (isa<TemplateDecl>(D)) {
    if (!AllowFunctionTemplates && isa<FunctionTemplateDecl>(D))
      return nullptr;

    return Orig;
  }

  if (CXXRecordDecl *Record = dyn_cast<CXXRecordDecl>(D)) {
    // C++ [temp.local]p1:
    //   Like normal (non-template) classes, class templates have an
    //   injected-class-name (Clause 9). The injected-class-name
    //   can be used with or without a template-argument-list. When
    //   it is used without a template-argument-list, it is
    //   equivalent to the injected-class-name followed by the
    //   template-parameters of the class template enclosed in
    //   <>. When it is used with a template-argument-list, it
    //   refers to the specified class template specialization,
    //   which could be the current specialization or another
    //   specialization.
    if (Record->isInjectedClassName()) {
      Record = cast<CXXRecordDecl>(Record->getDeclContext());
      if (Record->getDescribedClassTemplate())
        return Record->getDescribedClassTemplate();

      if (ClassTemplateSpecializationDecl *Spec
            = dyn_cast<ClassTemplateSpecializationDecl>(Record))
        return Spec->getSpecializedTemplate();
    }

    return nullptr;
  }

  return nullptr;
}

void Sema::FilterAcceptableTemplateNames(LookupResult &R, 
                                         bool AllowFunctionTemplates) {
  // The set of class templates we've already seen.
  llvm::SmallPtrSet<ClassTemplateDecl *, 8> ClassTemplates;
  LookupResult::Filter filter = R.makeFilter();
  while (filter.hasNext()) {
    NamedDecl *Orig = filter.next();
    NamedDecl *Repl = isAcceptableTemplateName(Context, Orig, 
                                               AllowFunctionTemplates);
    if (!Repl)
      filter.erase();
    else if (Repl != Orig) {

      // C++ [temp.local]p3:
      //   A lookup that finds an injected-class-name (10.2) can result in an
      //   ambiguity in certain cases (for example, if it is found in more than
      //   one base class). If all of the injected-class-names that are found
      //   refer to specializations of the same class template, and if the name
      //   is used as a template-name, the reference refers to the class
      //   template itself and not a specialization thereof, and is not
      //   ambiguous.
      if (ClassTemplateDecl *ClassTmpl = dyn_cast<ClassTemplateDecl>(Repl))
        if (!ClassTemplates.insert(ClassTmpl).second) {
          filter.erase();
          continue;
        }

      // FIXME: we promote access to public here as a workaround to
      // the fact that LookupResult doesn't let us remember that we
      // found this template through a particular injected class name,
      // which means we end up doing nasty things to the invariants.
      // Pretending that access is public is *much* safer.
      filter.replace(Repl, AS_public);
    }
  }
  filter.done();
}

bool Sema::hasAnyAcceptableTemplateNames(LookupResult &R,
                                         bool AllowFunctionTemplates) {
  for (LookupResult::iterator I = R.begin(), IEnd = R.end(); I != IEnd; ++I)
    if (isAcceptableTemplateName(Context, *I, AllowFunctionTemplates))
      return true;
  
  return false;
}

TemplateNameKind Sema::isTemplateName(Scope *S,
                                      CXXScopeSpec &SS,
                                      bool hasTemplateKeyword,
                                      UnqualifiedId &Name,
                                      ParsedType ObjectTypePtr,
                                      bool EnteringContext,
                                      TemplateTy &TemplateResult,
                                      bool &MemberOfUnknownSpecialization) {
  assert(getLangOpts().CPlusPlus && "No template names in C!");

  DeclarationName TName;
  MemberOfUnknownSpecialization = false;

  switch (Name.getKind()) {
  case UnqualifiedId::IK_Identifier:
    TName = DeclarationName(Name.Identifier);
    break;

  case UnqualifiedId::IK_OperatorFunctionId:
    TName = Context.DeclarationNames.getCXXOperatorName(
                                              Name.OperatorFunctionId.Operator);
    break;

  case UnqualifiedId::IK_LiteralOperatorId:
    TName = Context.DeclarationNames.getCXXLiteralOperatorName(Name.Identifier);
    break;

  default:
    return TNK_Non_template;
  }

  QualType ObjectType = ObjectTypePtr.get();

  LookupResult R(*this, TName, Name.getLocStart(), LookupOrdinaryName);
  LookupTemplateName(R, S, SS, ObjectType, EnteringContext,
                     MemberOfUnknownSpecialization);
  if (R.empty()) return TNK_Non_template;
  if (R.isAmbiguous()) {
    // Suppress diagnostics;  we'll redo this lookup later.
    R.suppressDiagnostics();

    // FIXME: we might have ambiguous templates, in which case we
    // should at least parse them properly!
    return TNK_Non_template;
  }

  TemplateName Template;
  TemplateNameKind TemplateKind;

  unsigned ResultCount = R.end() - R.begin();
  if (ResultCount > 1) {
    // We assume that we'll preserve the qualifier from a function
    // template name in other ways.
    Template = Context.getOverloadedTemplateName(R.begin(), R.end());
    TemplateKind = TNK_Function_template;

    // We'll do this lookup again later.
    R.suppressDiagnostics();
  } else {
    TemplateDecl *TD = cast<TemplateDecl>((*R.begin())->getUnderlyingDecl());

    if (SS.isSet() && !SS.isInvalid()) {
      NestedNameSpecifier *Qualifier = SS.getScopeRep();
      Template = Context.getQualifiedTemplateName(Qualifier,
                                                  hasTemplateKeyword, TD);
    } else {
      Template = TemplateName(TD);
    }

    if (isa<FunctionTemplateDecl>(TD)) {
      TemplateKind = TNK_Function_template;

      // We'll do this lookup again later.
      R.suppressDiagnostics();
    } else {
      assert(isa<ClassTemplateDecl>(TD) || isa<TemplateTemplateParmDecl>(TD) ||
             isa<TypeAliasTemplateDecl>(TD) || isa<VarTemplateDecl>(TD));
      TemplateKind =
          isa<VarTemplateDecl>(TD) ? TNK_Var_template : TNK_Type_template;
    }
  }

  TemplateResult = TemplateTy::make(Template);
  return TemplateKind;
}

bool Sema::DiagnoseUnknownTemplateName(const IdentifierInfo &II,
                                       SourceLocation IILoc,
                                       Scope *S,
                                       const CXXScopeSpec *SS,
                                       TemplateTy &SuggestedTemplate,
                                       TemplateNameKind &SuggestedKind) {
  // We can't recover unless there's a dependent scope specifier preceding the
  // template name.
  // FIXME: Typo correction?
  if (!SS || !SS->isSet() || !isDependentScopeSpecifier(*SS) ||
      computeDeclContext(*SS))
    return false;

  // The code is missing a 'template' keyword prior to the dependent template
  // name.
  NestedNameSpecifier *Qualifier = (NestedNameSpecifier*)SS->getScopeRep();
  Diag(IILoc, diag::err_template_kw_missing)
    << Qualifier << II.getName()
    << FixItHint::CreateInsertion(IILoc, "template ");
  SuggestedTemplate
    = TemplateTy::make(Context.getDependentTemplateName(Qualifier, &II));
  SuggestedKind = TNK_Dependent_template_name;
  return true;
}

void Sema::LookupTemplateName(LookupResult &Found,
                              Scope *S, CXXScopeSpec &SS,
                              QualType ObjectType,
                              bool EnteringContext,
                              bool &MemberOfUnknownSpecialization) {
  // Determine where to perform name lookup
  MemberOfUnknownSpecialization = false;
  DeclContext *LookupCtx = nullptr;
  bool isDependent = false;
  if (!ObjectType.isNull()) {
    // This nested-name-specifier occurs in a member access expression, e.g.,
    // x->B::f, and we are looking into the type of the object.
    assert(!SS.isSet() && "ObjectType and scope specifier cannot coexist");
    LookupCtx = computeDeclContext(ObjectType);
    isDependent = ObjectType->isDependentType();
    assert((isDependent || !ObjectType->isIncompleteType() ||
            ObjectType->castAs<TagType>()->isBeingDefined()) &&
           "Caller should have completed object type");
    
    // Template names cannot appear inside an Objective-C class or object type.
    if (ObjectType->isObjCObjectOrInterfaceType()) {
      Found.clear();
      return;
    }
  } else if (SS.isSet()) {
    // This nested-name-specifier occurs after another nested-name-specifier,
    // so long into the context associated with the prior nested-name-specifier.
    LookupCtx = computeDeclContext(SS, EnteringContext);
    isDependent = isDependentScopeSpecifier(SS);

    // The declaration context must be complete.
    if (LookupCtx && RequireCompleteDeclContext(SS, LookupCtx))
      return;
  }

  bool ObjectTypeSearchedInScope = false;
  bool AllowFunctionTemplatesInLookup = true;
  if (LookupCtx) {
    // Perform "qualified" name lookup into the declaration context we
    // computed, which is either the type of the base of a member access
    // expression or the declaration context associated with a prior
    // nested-name-specifier.
    LookupQualifiedName(Found, LookupCtx);
    if (!ObjectType.isNull() && Found.empty()) {
      // C++ [basic.lookup.classref]p1:
      //   In a class member access expression (5.2.5), if the . or -> token is
      //   immediately followed by an identifier followed by a <, the
      //   identifier must be looked up to determine whether the < is the
      //   beginning of a template argument list (14.2) or a less-than operator.
      //   The identifier is first looked up in the class of the object
      //   expression. If the identifier is not found, it is then looked up in
      //   the context of the entire postfix-expression and shall name a class
      //   or function template.
      if (S) LookupName(Found, S);
      ObjectTypeSearchedInScope = true;
      AllowFunctionTemplatesInLookup = false;
    }
  } else if (isDependent && (!S || ObjectType.isNull())) {
    // We cannot look into a dependent object type or nested nme
    // specifier.
    MemberOfUnknownSpecialization = true;
    return;
  } else {
    // Perform unqualified name lookup in the current scope.
    LookupName(Found, S);
    if (!ObjectType.isNull())
      AllowFunctionTemplatesInLookup = false;
  }

  if (Found.empty() && !isDependent) {
    // If we did not find any names, attempt to correct any typos.
    DeclarationName Name = Found.getLookupName();
    Found.clear();
    // Simple filter callback that, for keywords, only accepts the C++ *_cast
    auto FilterCCC = llvm::make_unique<CorrectionCandidateCallback>();
    FilterCCC->WantTypeSpecifiers = false;
    FilterCCC->WantExpressionKeywords = false;
    FilterCCC->WantRemainingKeywords = false;
    FilterCCC->WantCXXNamedCasts = true;
    if (TypoCorrection Corrected = CorrectTypo(
            Found.getLookupNameInfo(), Found.getLookupKind(), S, &SS,
            std::move(FilterCCC), CTK_ErrorRecovery, LookupCtx)) {
      Found.setLookupName(Corrected.getCorrection());
      if (Corrected.getCorrectionDecl())
        Found.addDecl(Corrected.getCorrectionDecl());
      FilterAcceptableTemplateNames(Found);
      if (!Found.empty()) {
        if (LookupCtx) {
          std::string CorrectedStr(Corrected.getAsString(getLangOpts()));
          bool DroppedSpecifier = Corrected.WillReplaceSpecifier() &&
                                  Name.getAsString() == CorrectedStr;
          diagnoseTypo(Corrected, PDiag(diag::err_no_member_template_suggest)
                                    << Name << LookupCtx << DroppedSpecifier
                                    << SS.getRange());
        } else {
          diagnoseTypo(Corrected, PDiag(diag::err_no_template_suggest) << Name);
        }
      }
    } else {
      Found.setLookupName(Name);
    }
  }

  FilterAcceptableTemplateNames(Found, AllowFunctionTemplatesInLookup);
  if (Found.empty()) {
    if (isDependent)
      MemberOfUnknownSpecialization = true;
    return;
  }

  if (S && !ObjectType.isNull() && !ObjectTypeSearchedInScope &&
      !getLangOpts().CPlusPlus11) {
    // C++03 [basic.lookup.classref]p1:
    //   [...] If the lookup in the class of the object expression finds a
    //   template, the name is also looked up in the context of the entire
    //   postfix-expression and [...]
    //
    // Note: C++11 does not perform this second lookup.
    LookupResult FoundOuter(*this, Found.getLookupName(), Found.getNameLoc(),
                            LookupOrdinaryName);
    LookupName(FoundOuter, S);
    FilterAcceptableTemplateNames(FoundOuter, /*AllowFunctionTemplates=*/false);

    if (FoundOuter.empty()) {
      //   - if the name is not found, the name found in the class of the
      //     object expression is used, otherwise
    } else if (!FoundOuter.getAsSingle<ClassTemplateDecl>() ||
               FoundOuter.isAmbiguous()) {
      //   - if the name is found in the context of the entire
      //     postfix-expression and does not name a class template, the name
      //     found in the class of the object expression is used, otherwise
      FoundOuter.clear();
    } else if (!Found.isSuppressingDiagnostics()) {
      //   - if the name found is a class template, it must refer to the same
      //     entity as the one found in the class of the object expression,
      //     otherwise the program is ill-formed.
      if (!Found.isSingleResult() ||
          Found.getFoundDecl()->getCanonicalDecl()
            != FoundOuter.getFoundDecl()->getCanonicalDecl()) {
        Diag(Found.getNameLoc(),
             diag::ext_nested_name_member_ref_lookup_ambiguous)
          << Found.getLookupName()
          << ObjectType;
        Diag(Found.getRepresentativeDecl()->getLocation(),
             diag::note_ambig_member_ref_object_type)
          << ObjectType;
        Diag(FoundOuter.getFoundDecl()->getLocation(),
             diag::note_ambig_member_ref_scope);

        // Recover by taking the template that we found in the object
        // expression's type.
      }
    }
  }
}

/// ActOnDependentIdExpression - Handle a dependent id-expression that
/// was just parsed.  This is only possible with an explicit scope
/// specifier naming a dependent type.
ExprResult
Sema::ActOnDependentIdExpression(const CXXScopeSpec &SS,
                                 SourceLocation TemplateKWLoc,
                                 const DeclarationNameInfo &NameInfo,
                                 bool isAddressOfOperand,
                           const TemplateArgumentListInfo *TemplateArgs) {
  DeclContext *DC = getFunctionLevelDeclContext();

  if (!isAddressOfOperand &&
      isa<CXXMethodDecl>(DC) &&
      cast<CXXMethodDecl>(DC)->isInstance()) {
    QualType ThisType = cast<CXXMethodDecl>(DC)->getThisType(Context);

    // Since the 'this' expression is synthesized, we don't need to
    // perform the double-lookup check.
    NamedDecl *FirstQualifierInScope = nullptr;

    // HLSL Change begin - This is a reference.
    return CXXDependentScopeMemberExpr::Create(
        Context, /*This*/ nullptr, ThisType, /*IsArrow*/ !getLangOpts().HLSL,
        /*Op*/ SourceLocation(), SS.getWithLocInContext(Context), TemplateKWLoc,
        FirstQualifierInScope, NameInfo, TemplateArgs);
    // HLSL Change end - This is a reference.
  }

  return BuildDependentDeclRefExpr(SS, TemplateKWLoc, NameInfo, TemplateArgs);
}

ExprResult
Sema::BuildDependentDeclRefExpr(const CXXScopeSpec &SS,
                                SourceLocation TemplateKWLoc,
                                const DeclarationNameInfo &NameInfo,
                                const TemplateArgumentListInfo *TemplateArgs) {
  return DependentScopeDeclRefExpr::Create(
      Context, SS.getWithLocInContext(Context), TemplateKWLoc, NameInfo,
      TemplateArgs);
}

/// DiagnoseTemplateParameterShadow - Produce a diagnostic complaining
/// that the template parameter 'PrevDecl' is being shadowed by a new
/// declaration at location Loc. Returns true to indicate that this is
/// an error, and false otherwise.
void Sema::DiagnoseTemplateParameterShadow(SourceLocation Loc, Decl *PrevDecl) {
  assert(PrevDecl->isTemplateParameter() && "Not a template parameter");

  // Microsoft Visual C++ permits template parameters to be shadowed.
  if (getLangOpts().MicrosoftExt)
    return;

  // C++ [temp.local]p4:
  //   A template-parameter shall not be redeclared within its
  //   scope (including nested scopes).
  Diag(Loc, diag::err_template_param_shadow)
    << cast<NamedDecl>(PrevDecl)->getDeclName();
  Diag(PrevDecl->getLocation(), diag::note_template_param_here);
  return;
}

/// AdjustDeclIfTemplate - If the given decl happens to be a template, reset
/// the parameter D to reference the templated declaration and return a pointer
/// to the template declaration. Otherwise, do nothing to D and return null.
TemplateDecl *Sema::AdjustDeclIfTemplate(Decl *&D) {
  if (TemplateDecl *Temp = dyn_cast_or_null<TemplateDecl>(D)) {
    D = Temp->getTemplatedDecl();
    return Temp;
  }
  return nullptr;
}

ParsedTemplateArgument ParsedTemplateArgument::getTemplatePackExpansion(
                                             SourceLocation EllipsisLoc) const {
  assert(Kind == Template &&
         "Only template template arguments can be pack expansions here");
  assert(getAsTemplate().get().containsUnexpandedParameterPack() &&
         "Template template argument pack expansion without packs");
  ParsedTemplateArgument Result(*this);
  Result.EllipsisLoc = EllipsisLoc;
  return Result;
}

static TemplateArgumentLoc translateTemplateArgument(Sema &SemaRef,
                                            const ParsedTemplateArgument &Arg) {

  switch (Arg.getKind()) {
  case ParsedTemplateArgument::Type: {
    TypeSourceInfo *DI;
    QualType T = SemaRef.GetTypeFromParser(Arg.getAsType(), &DI);
    if (!DI)
      DI = SemaRef.Context.getTrivialTypeSourceInfo(T, Arg.getLocation());
    return TemplateArgumentLoc(TemplateArgument(T), DI);
  }

  case ParsedTemplateArgument::NonType: {
    Expr *E = static_cast<Expr *>(Arg.getAsExpr());
    return TemplateArgumentLoc(TemplateArgument(E), E);
  }

  case ParsedTemplateArgument::Template: {
    TemplateName Template = Arg.getAsTemplate().get();
    TemplateArgument TArg;
    if (Arg.getEllipsisLoc().isValid())
      TArg = TemplateArgument(Template, Optional<unsigned int>());
    else
      TArg = Template;
    return TemplateArgumentLoc(TArg,
                               Arg.getScopeSpec().getWithLocInContext(
                                                              SemaRef.Context),
                               Arg.getLocation(),
                               Arg.getEllipsisLoc());
  }
  }

  llvm_unreachable("Unhandled parsed template argument");
}

/// \brief Translates template arguments as provided by the parser
/// into template arguments used by semantic analysis.
void Sema::translateTemplateArguments(const ASTTemplateArgsPtr &TemplateArgsIn,
                                      TemplateArgumentListInfo &TemplateArgs) {
 for (unsigned I = 0, Last = TemplateArgsIn.size(); I != Last; ++I)
   TemplateArgs.addArgument(translateTemplateArgument(*this,
                                                      TemplateArgsIn[I]));
}

static void maybeDiagnoseTemplateParameterShadow(Sema &SemaRef, Scope *S,
                                                 SourceLocation Loc,
                                                 IdentifierInfo *Name) {
  NamedDecl *PrevDecl = SemaRef.LookupSingleName(
      S, Name, Loc, Sema::LookupOrdinaryName, Sema::ForRedeclaration);
  if (PrevDecl && PrevDecl->isTemplateParameter())
    SemaRef.DiagnoseTemplateParameterShadow(Loc, PrevDecl);
}

/// ActOnTypeParameter - Called when a C++ template type parameter
/// (e.g., "typename T") has been parsed. Typename specifies whether
/// the keyword "typename" was used to declare the type parameter
/// (otherwise, "class" was used), and KeyLoc is the location of the
/// "class" or "typename" keyword. ParamName is the name of the
/// parameter (NULL indicates an unnamed template parameter) and
/// ParamNameLoc is the location of the parameter name (if any).
/// If the type parameter has a default argument, it will be added
/// later via ActOnTypeParameterDefault.
Decl *Sema::ActOnTypeParameter(Scope *S, bool Typename,
                               SourceLocation EllipsisLoc,
                               SourceLocation KeyLoc,
                               IdentifierInfo *ParamName,
                               SourceLocation ParamNameLoc,
                               unsigned Depth, unsigned Position,
                               SourceLocation EqualLoc,
                               ParsedType DefaultArg) {
  assert(S->isTemplateParamScope() &&
         "Template type parameter not in template parameter scope!");
  bool Invalid = false;

  SourceLocation Loc = ParamNameLoc;
  if (!ParamName)
    Loc = KeyLoc;

  bool IsParameterPack = EllipsisLoc.isValid();
  TemplateTypeParmDecl *Param
    = TemplateTypeParmDecl::Create(Context, Context.getTranslationUnitDecl(),
                                   KeyLoc, Loc, Depth, Position, ParamName,
                                   Typename, IsParameterPack);
  Param->setAccess(AS_public);
  if (Invalid)
    Param->setInvalidDecl();

  if (ParamName) {
    maybeDiagnoseTemplateParameterShadow(*this, S, ParamNameLoc, ParamName);

    // Add the template parameter into the current scope.
    S->AddDecl(Param);
    IdResolver.AddDecl(Param);
  }

  // C++0x [temp.param]p9:
  //   A default template-argument may be specified for any kind of
  //   template-parameter that is not a template parameter pack.
  if (DefaultArg && IsParameterPack) {
    Diag(EqualLoc, diag::err_template_param_pack_default_arg);
    DefaultArg = ParsedType();
  }

  // Handle the default argument, if provided.
  if (DefaultArg) {
    TypeSourceInfo *DefaultTInfo;
    GetTypeFromParser(DefaultArg, &DefaultTInfo);

    assert(DefaultTInfo && "expected source information for type");

    // Check for unexpanded parameter packs.
    if (DiagnoseUnexpandedParameterPack(Loc, DefaultTInfo,
                                        UPPC_DefaultArgument))
      return Param;

    // Check the template argument itself.
    if (CheckTemplateArgument(Param, DefaultTInfo)) {
      Param->setInvalidDecl();
      return Param;
    }

    Param->setDefaultArgument(DefaultTInfo);
  }

  return Param;
}

/// \brief Check that the type of a non-type template parameter is
/// well-formed.
///
/// \returns the (possibly-promoted) parameter type if valid;
/// otherwise, produces a diagnostic and returns a NULL type.
QualType
Sema::CheckNonTypeTemplateParameterType(QualType T, SourceLocation Loc) {
  // We don't allow variably-modified types as the type of non-type template
  // parameters.
  if (T->isVariablyModifiedType()) {
    Diag(Loc, diag::err_variably_modified_nontype_template_param)
      << T;
    return QualType();
  }

  // C++ [temp.param]p4:
  //
  // A non-type template-parameter shall have one of the following
  // (optionally cv-qualified) types:
  //
  //       -- integral or enumeration type,
  if (T->isIntegralOrEnumerationType() ||
      //   -- pointer to object or pointer to function,
      T->isPointerType() ||
      //   -- reference to object or reference to function,
      T->isReferenceType() ||
      //   -- pointer to member,
      T->isMemberPointerType() ||
      //   -- std::nullptr_t.
      T->isNullPtrType() ||
      // If T is a dependent type, we can't do the check now, so we
      // assume that it is well-formed.
      T->isDependentType()) {
    // C++ [temp.param]p5: The top-level cv-qualifiers on the template-parameter
    // are ignored when determining its type.
    return T.getUnqualifiedType();
  }

  // C++ [temp.param]p8:
  //
  //   A non-type template-parameter of type "array of T" or
  //   "function returning T" is adjusted to be of type "pointer to
  //   T" or "pointer to function returning T", respectively.
  else if (T->isArrayType() || T->isFunctionType())
    return Context.getDecayedType(T);

  Diag(Loc, diag::err_template_nontype_parm_bad_type)
    << T;

  return QualType();
}

Decl *Sema::ActOnNonTypeTemplateParameter(Scope *S, Declarator &D,
                                          unsigned Depth,
                                          unsigned Position,
                                          SourceLocation EqualLoc,
                                          Expr *Default) {
  TypeSourceInfo *TInfo = GetTypeForDeclarator(D, S);
  QualType T = TInfo->getType();

  assert(S->isTemplateParamScope() &&
         "Non-type template parameter not in template parameter scope!");
  bool Invalid = false;

  T = CheckNonTypeTemplateParameterType(T, D.getIdentifierLoc());
  if (T.isNull()) {
    T = Context.IntTy; // Recover with an 'int' type.
    Invalid = true;
  }

  IdentifierInfo *ParamName = D.getIdentifier();
  bool IsParameterPack = D.hasEllipsis();
  NonTypeTemplateParmDecl *Param
    = NonTypeTemplateParmDecl::Create(Context, Context.getTranslationUnitDecl(),
                                      D.getLocStart(),
                                      D.getIdentifierLoc(),
                                      Depth, Position, ParamName, T,
                                      IsParameterPack, TInfo);
  Param->setAccess(AS_public);

  if (Invalid)
    Param->setInvalidDecl();

  if (ParamName) {
    maybeDiagnoseTemplateParameterShadow(*this, S, D.getIdentifierLoc(),
                                         ParamName);

    // Add the template parameter into the current scope.
    S->AddDecl(Param);
    IdResolver.AddDecl(Param);
  }

  // C++0x [temp.param]p9:
  //   A default template-argument may be specified for any kind of
  //   template-parameter that is not a template parameter pack.
  if (Default && IsParameterPack) {
    Diag(EqualLoc, diag::err_template_param_pack_default_arg);
    Default = nullptr;
  }

  // Check the well-formedness of the default template argument, if provided.
  if (Default) {
    // Check for unexpanded parameter packs.
    if (DiagnoseUnexpandedParameterPack(Default, UPPC_DefaultArgument))
      return Param;

    TemplateArgument Converted;
    ExprResult DefaultRes =
        CheckTemplateArgument(Param, Param->getType(), Default, Converted);
    if (DefaultRes.isInvalid()) {
      Param->setInvalidDecl();
      return Param;
    }
    Default = DefaultRes.get();

    Param->setDefaultArgument(Default);
  }

  return Param;
}

/// ActOnTemplateTemplateParameter - Called when a C++ template template
/// parameter (e.g. T in template <template \<typename> class T> class array)
/// has been parsed. S is the current scope.
Decl *Sema::ActOnTemplateTemplateParameter(Scope* S,
                                           SourceLocation TmpLoc,
                                           TemplateParameterList *Params,
                                           SourceLocation EllipsisLoc,
                                           IdentifierInfo *Name,
                                           SourceLocation NameLoc,
                                           unsigned Depth,
                                           unsigned Position,
                                           SourceLocation EqualLoc,
                                           ParsedTemplateArgument Default) {
  assert(S->isTemplateParamScope() &&
         "Template template parameter not in template parameter scope!");

  // Construct the parameter object.
  bool IsParameterPack = EllipsisLoc.isValid();
  TemplateTemplateParmDecl *Param =
    TemplateTemplateParmDecl::Create(Context, Context.getTranslationUnitDecl(),
                                     NameLoc.isInvalid()? TmpLoc : NameLoc,
                                     Depth, Position, IsParameterPack,
                                     Name, Params);
  Param->setAccess(AS_public);
  
  // If the template template parameter has a name, then link the identifier
  // into the scope and lookup mechanisms.
  if (Name) {
    maybeDiagnoseTemplateParameterShadow(*this, S, NameLoc, Name);

    S->AddDecl(Param);
    IdResolver.AddDecl(Param);
  }

  if (Params->size() == 0) {
    Diag(Param->getLocation(), diag::err_template_template_parm_no_parms)
    << SourceRange(Params->getLAngleLoc(), Params->getRAngleLoc());
    Param->setInvalidDecl();
  }

  // C++0x [temp.param]p9:
  //   A default template-argument may be specified for any kind of
  //   template-parameter that is not a template parameter pack.
  if (IsParameterPack && !Default.isInvalid()) {
    Diag(EqualLoc, diag::err_template_param_pack_default_arg);
    Default = ParsedTemplateArgument();
  }

  if (!Default.isInvalid()) {
    // Check only that we have a template template argument. We don't want to
    // try to check well-formedness now, because our template template parameter
    // might have dependent types in its template parameters, which we wouldn't
    // be able to match now.
    //
    // If none of the template template parameter's template arguments mention
    // other template parameters, we could actually perform more checking here.
    // However, it isn't worth doing.
    TemplateArgumentLoc DefaultArg = translateTemplateArgument(*this, Default);
    if (DefaultArg.getArgument().getAsTemplate().isNull()) {
      Diag(DefaultArg.getLocation(), diag::err_template_arg_not_class_template)
        << DefaultArg.getSourceRange();
      return Param;
    }

    // Check for unexpanded parameter packs.
    if (DiagnoseUnexpandedParameterPack(DefaultArg.getLocation(),
                                        DefaultArg.getArgument().getAsTemplate(),
                                        UPPC_DefaultArgument))
      return Param;

    Param->setDefaultArgument(Context, DefaultArg);
  }

  return Param;
}

/// ActOnTemplateParameterList - Builds a TemplateParameterList that
/// contains the template parameters in Params/NumParams.
TemplateParameterList *
Sema::ActOnTemplateParameterList(unsigned Depth,
                                 SourceLocation ExportLoc,
                                 SourceLocation TemplateLoc,
                                 SourceLocation LAngleLoc,
                                 Decl **Params, unsigned NumParams,
                                 SourceLocation RAngleLoc) {
  if (ExportLoc.isValid())
    Diag(ExportLoc, diag::warn_template_export_unsupported);

  return TemplateParameterList::Create(Context, TemplateLoc, LAngleLoc,
                                       (NamedDecl**)Params, NumParams,
                                       RAngleLoc);
}

static void SetNestedNameSpecifier(TagDecl *T, const CXXScopeSpec &SS) {
  if (SS.isSet())
    T->setQualifierInfo(SS.getWithLocInContext(T->getASTContext()));
}

DeclResult
Sema::CheckClassTemplate(Scope *S, unsigned TagSpec, TagUseKind TUK,
                         SourceLocation KWLoc, CXXScopeSpec &SS,
                         IdentifierInfo *Name, SourceLocation NameLoc,
                         AttributeList *Attr,
                         TemplateParameterList *TemplateParams,
                         AccessSpecifier AS, SourceLocation ModulePrivateLoc,
                         SourceLocation FriendLoc,
                         unsigned NumOuterTemplateParamLists,
                         TemplateParameterList** OuterTemplateParamLists,
                         SkipBodyInfo *SkipBody) {
  assert(TemplateParams && TemplateParams->size() > 0 &&
         "No template parameters");
  assert(TUK != TUK_Reference && "Can only declare or define class templates");
  bool Invalid = false;

  // Check that we can declare a template here.
  if (CheckTemplateDeclScope(S, TemplateParams))
    return true;

  TagTypeKind Kind = TypeWithKeyword::getTagTypeKindForTypeSpec(TagSpec);
  assert(Kind != TTK_Enum && "can't build template of enumerated type");

  // There is no such thing as an unnamed class template.
  if (!Name) {
    Diag(KWLoc, diag::err_template_unnamed_class);
    return true;
  }

  // Find any previous declaration with this name. For a friend with no
  // scope explicitly specified, we only look for tag declarations (per
  // C++11 [basic.lookup.elab]p2).
  DeclContext *SemanticContext;
  LookupResult Previous(*this, Name, NameLoc,
                        (SS.isEmpty() && TUK == TUK_Friend)
                          ? LookupTagName : LookupOrdinaryName,
                        ForRedeclaration);
  if (SS.isNotEmpty() && !SS.isInvalid()) {
    SemanticContext = computeDeclContext(SS, true);
    if (!SemanticContext) {
      // FIXME: Horrible, horrible hack! We can't currently represent this
      // in the AST, and historically we have just ignored such friend
      // class templates, so don't complain here.
      Diag(NameLoc, TUK == TUK_Friend
                        ? diag::warn_template_qualified_friend_ignored
                        : diag::err_template_qualified_declarator_no_match)
          << SS.getScopeRep() << SS.getRange();
      return TUK != TUK_Friend;
    }

    if (RequireCompleteDeclContext(SS, SemanticContext))
      return true;

    // If we're adding a template to a dependent context, we may need to 
    // rebuilding some of the types used within the template parameter list, 
    // now that we know what the current instantiation is.
    if (SemanticContext->isDependentContext()) {
      ContextRAII SavedContext(*this, SemanticContext);
      if (RebuildTemplateParamsInCurrentInstantiation(TemplateParams))
        Invalid = true;
    } else if (TUK != TUK_Friend && TUK != TUK_Reference)
      diagnoseQualifiedDeclaration(SS, SemanticContext, Name, NameLoc);

    LookupQualifiedName(Previous, SemanticContext);
  } else {
    SemanticContext = CurContext;

    // C++14 [class.mem]p14:
    //   If T is the name of a class, then each of the following shall have a
    //   name different from T:
    //    -- every member template of class T
    if (TUK != TUK_Friend &&
        DiagnoseClassNameShadow(SemanticContext,
                                DeclarationNameInfo(Name, NameLoc)))
      return true;

    LookupName(Previous, S);
  }

  if (Previous.isAmbiguous())
    return true;

  NamedDecl *PrevDecl = nullptr;
  if (Previous.begin() != Previous.end())
    PrevDecl = (*Previous.begin())->getUnderlyingDecl();

  // If there is a previous declaration with the same name, check
  // whether this is a valid redeclaration.
  ClassTemplateDecl *PrevClassTemplate
    = dyn_cast_or_null<ClassTemplateDecl>(PrevDecl);

  // We may have found the injected-class-name of a class template,
  // class template partial specialization, or class template specialization.
  // In these cases, grab the template that is being defined or specialized.
  if (!PrevClassTemplate && PrevDecl && isa<CXXRecordDecl>(PrevDecl) &&
      cast<CXXRecordDecl>(PrevDecl)->isInjectedClassName()) {
    PrevDecl = cast<CXXRecordDecl>(PrevDecl->getDeclContext());
    PrevClassTemplate
      = cast<CXXRecordDecl>(PrevDecl)->getDescribedClassTemplate();
    if (!PrevClassTemplate && isa<ClassTemplateSpecializationDecl>(PrevDecl)) {
      PrevClassTemplate
        = cast<ClassTemplateSpecializationDecl>(PrevDecl)
            ->getSpecializedTemplate();
    }
  }

  if (TUK == TUK_Friend) {
    // C++ [namespace.memdef]p3:
    //   [...] When looking for a prior declaration of a class or a function
    //   declared as a friend, and when the name of the friend class or
    //   function is neither a qualified name nor a template-id, scopes outside
    //   the innermost enclosing namespace scope are not considered.
    if (!SS.isSet()) {
      DeclContext *OutermostContext = CurContext;
      while (!OutermostContext->isFileContext())
        OutermostContext = OutermostContext->getLookupParent();

      if (PrevDecl &&
          (OutermostContext->Equals(PrevDecl->getDeclContext()) ||
           OutermostContext->Encloses(PrevDecl->getDeclContext()))) {
        SemanticContext = PrevDecl->getDeclContext();
      } else {
        // Declarations in outer scopes don't matter. However, the outermost
        // context we computed is the semantic context for our new
        // declaration.
        PrevDecl = PrevClassTemplate = nullptr;
        SemanticContext = OutermostContext;

        // Check that the chosen semantic context doesn't already contain a
        // declaration of this name as a non-tag type.
        Previous.clear(LookupOrdinaryName);
        DeclContext *LookupContext = SemanticContext;
        while (LookupContext->isTransparentContext())
          LookupContext = LookupContext->getLookupParent();
        LookupQualifiedName(Previous, LookupContext);

        if (Previous.isAmbiguous())
          return true;

        if (Previous.begin() != Previous.end())
          PrevDecl = (*Previous.begin())->getUnderlyingDecl();
      }
    }
  } else if (PrevDecl &&
             !isDeclInScope(Previous.getRepresentativeDecl(), SemanticContext,
                            S, SS.isValid()))
    PrevDecl = PrevClassTemplate = nullptr;

  if (auto *Shadow = dyn_cast_or_null<UsingShadowDecl>(
          PrevDecl ? Previous.getRepresentativeDecl() : nullptr)) {
    if (SS.isEmpty() &&
        !(PrevClassTemplate &&
          PrevClassTemplate->getDeclContext()->getRedeclContext()->Equals(
              SemanticContext->getRedeclContext()))) {
      Diag(KWLoc, diag::err_using_decl_conflict_reverse);
      Diag(Shadow->getTargetDecl()->getLocation(),
           diag::note_using_decl_target);
      Diag(Shadow->getUsingDecl()->getLocation(), diag::note_using_decl) << 0;
      // Recover by ignoring the old declaration.
      PrevDecl = PrevClassTemplate = nullptr;
    }
  }

  if (PrevClassTemplate) {
    // Ensure that the template parameter lists are compatible. Skip this check
    // for a friend in a dependent context: the template parameter list itself
    // could be dependent.
    if (!(TUK == TUK_Friend && CurContext->isDependentContext()) &&
        !TemplateParameterListsAreEqual(TemplateParams,
                                   PrevClassTemplate->getTemplateParameters(),
                                        /*Complain=*/true,
                                        TPL_TemplateMatch))
      return true;

    // C++ [temp.class]p4:
    //   In a redeclaration, partial specialization, explicit
    //   specialization or explicit instantiation of a class template,
    //   the class-key shall agree in kind with the original class
    //   template declaration (7.1.5.3).
    RecordDecl *PrevRecordDecl = PrevClassTemplate->getTemplatedDecl();
    if (!isAcceptableTagRedeclaration(PrevRecordDecl, Kind,
                                      TUK == TUK_Definition,  KWLoc, Name)) {
      Diag(KWLoc, diag::err_use_with_wrong_tag)
        << Name
        << FixItHint::CreateReplacement(KWLoc, PrevRecordDecl->getKindName());
      Diag(PrevRecordDecl->getLocation(), diag::note_previous_use);
      Kind = PrevRecordDecl->getTagKind();
    }

    // Check for redefinition of this class template.
    if (TUK == TUK_Definition) {
      if (TagDecl *Def = PrevRecordDecl->getDefinition()) {
        // If we have a prior definition that is not visible, treat this as
        // simply making that previous definition visible.
        NamedDecl *Hidden = nullptr;
        if (SkipBody && !hasVisibleDefinition(Def, &Hidden)) {
          SkipBody->ShouldSkip = true;
          auto *Tmpl = cast<CXXRecordDecl>(Hidden)->getDescribedClassTemplate();
          assert(Tmpl && "original definition of a class template is not a "
                         "class template?");
          makeMergedDefinitionVisible(Hidden, KWLoc);
          makeMergedDefinitionVisible(Tmpl, KWLoc);
          return Def;
        }

        Diag(NameLoc, diag::err_redefinition) << Name;
        Diag(Def->getLocation(), diag::note_previous_definition);
        // FIXME: Would it make sense to try to "forget" the previous
        // definition, as part of error recovery?
        return true;
      }
    }    
  } else if (PrevDecl && PrevDecl->isTemplateParameter()) {
    // Maybe we will complain about the shadowed template parameter.
    DiagnoseTemplateParameterShadow(NameLoc, PrevDecl);
    // Just pretend that we didn't see the previous declaration.
    PrevDecl = nullptr;
  } else if (PrevDecl) {
    // C++ [temp]p5:
    //   A class template shall not have the same name as any other
    //   template, class, function, object, enumeration, enumerator,
    //   namespace, or type in the same scope (3.3), except as specified
    //   in (14.5.4).
    Diag(NameLoc, diag::err_redefinition_different_kind) << Name;
    Diag(PrevDecl->getLocation(), diag::note_previous_definition);
    return true;
  }

  // Check the template parameter list of this declaration, possibly
  // merging in the template parameter list from the previous class
  // template declaration. Skip this check for a friend in a dependent
  // context, because the template parameter list might be dependent.
  if (!(TUK == TUK_Friend && CurContext->isDependentContext()) &&
      CheckTemplateParameterList(
          TemplateParams,
          PrevClassTemplate ? PrevClassTemplate->getTemplateParameters()
                            : nullptr,
          (SS.isSet() && SemanticContext && SemanticContext->isRecord() &&
           SemanticContext->isDependentContext())
              ? TPC_ClassTemplateMember
              : TUK == TUK_Friend ? TPC_FriendClassTemplate
                                  : TPC_ClassTemplate))
    Invalid = true;

  if (SS.isSet()) {
    // If the name of the template was qualified, we must be defining the
    // template out-of-line.
    if (!SS.isInvalid() && !Invalid && !PrevClassTemplate) {
      Diag(NameLoc, TUK == TUK_Friend ? diag::err_friend_decl_does_not_match
                                      : diag::err_member_decl_does_not_match)
        << Name << SemanticContext << /*IsDefinition*/true << SS.getRange();
      Invalid = true;
    }
  }

  CXXRecordDecl *NewClass =
    CXXRecordDecl::Create(Context, Kind, SemanticContext, KWLoc, NameLoc, Name,
                          PrevClassTemplate?
                            PrevClassTemplate->getTemplatedDecl() : nullptr,
                          /*DelayTypeCreation=*/true);
  SetNestedNameSpecifier(NewClass, SS);
  if (NumOuterTemplateParamLists > 0)
    NewClass->setTemplateParameterListsInfo(Context,
                                            NumOuterTemplateParamLists,
                                            OuterTemplateParamLists);

  // Add alignment attributes if necessary; these attributes are checked when
  // the ASTContext lays out the structure.
  if (TUK == TUK_Definition) {
    AddAlignmentAttributesForRecord(NewClass);
    AddMsStructLayoutForRecord(NewClass);
  }

  ClassTemplateDecl *NewTemplate
    = ClassTemplateDecl::Create(Context, SemanticContext, NameLoc,
                                DeclarationName(Name), TemplateParams,
                                NewClass, PrevClassTemplate);
  NewClass->setDescribedClassTemplate(NewTemplate);
  
  if (ModulePrivateLoc.isValid())
    NewTemplate->setModulePrivate();
  
  // Build the type for the class template declaration now.
  QualType T = NewTemplate->getInjectedClassNameSpecialization();
  T = Context.getInjectedClassNameType(NewClass, T);
  assert(T->isDependentType() && "Class template type is not dependent?");
  (void)T;

  // If we are providing an explicit specialization of a member that is a
  // class template, make a note of that.
  if (PrevClassTemplate &&
      PrevClassTemplate->getInstantiatedFromMemberTemplate())
    PrevClassTemplate->setMemberSpecialization();

  // Set the access specifier.
  if (!Invalid && TUK != TUK_Friend && NewTemplate->getDeclContext()->isRecord())
    SetMemberAccessSpecifier(NewTemplate, PrevClassTemplate, AS);

  // Set the lexical context of these templates
  NewClass->setLexicalDeclContext(CurContext);
  NewTemplate->setLexicalDeclContext(CurContext);

  if (TUK == TUK_Definition)
    NewClass->startDefinition();

  if (Attr)
    ProcessDeclAttributeList(S, NewClass, Attr);

  if (PrevClassTemplate)
    mergeDeclAttributes(NewClass, PrevClassTemplate->getTemplatedDecl());

  AddPushedVisibilityAttribute(NewClass);

  if (TUK != TUK_Friend) {
    // Per C++ [basic.scope.temp]p2, skip the template parameter scopes.
    Scope *Outer = S;
    while ((Outer->getFlags() & Scope::TemplateParamScope) != 0)
      Outer = Outer->getParent();
    PushOnScopeChains(NewTemplate, Outer);
  } else {
    if (PrevClassTemplate && PrevClassTemplate->getAccess() != AS_none) {
      NewTemplate->setAccess(PrevClassTemplate->getAccess());
      NewClass->setAccess(PrevClassTemplate->getAccess());
    }

    NewTemplate->setObjectOfFriendDecl();

    // Friend templates are visible in fairly strange ways.
    if (!CurContext->isDependentContext()) {
      DeclContext *DC = SemanticContext->getRedeclContext();
      DC->makeDeclVisibleInContext(NewTemplate);
      if (Scope *EnclosingScope = getScopeForDeclContext(S, DC))
        PushOnScopeChains(NewTemplate, EnclosingScope,
                          /* AddToContext = */ false);
    }

    FriendDecl *Friend = FriendDecl::Create(
        Context, CurContext, NewClass->getLocation(), NewTemplate, FriendLoc);
    Friend->setAccess(AS_public);
    CurContext->addDecl(Friend);
  }

  if (Invalid) {
    NewTemplate->setInvalidDecl();
    NewClass->setInvalidDecl();
  }

  ActOnDocumentableDecl(NewTemplate);

  return NewTemplate;
}

/// \brief Diagnose the presence of a default template argument on a
/// template parameter, which is ill-formed in certain contexts.
///
/// \returns true if the default template argument should be dropped.
static bool DiagnoseDefaultTemplateArgument(Sema &S,
                                            Sema::TemplateParamListContext TPC,
                                            SourceLocation ParamLoc,
                                            SourceRange DefArgRange) {
  switch (TPC) {
  case Sema::TPC_ClassTemplate:
  case Sema::TPC_VarTemplate:
  case Sema::TPC_TypeAliasTemplate:
    return false;

  case Sema::TPC_FunctionTemplate:
  case Sema::TPC_FriendFunctionTemplateDefinition:
    // C++ [temp.param]p9:
    //   A default template-argument shall not be specified in a
    //   function template declaration or a function template
    //   definition [...]
    //   If a friend function template declaration specifies a default 
    //   template-argument, that declaration shall be a definition and shall be
    //   the only declaration of the function template in the translation unit.
    // (C++98/03 doesn't have this wording; see DR226).
    // HLSL Change Begin - Treat HLSL as C++11 here. This hides the C++11
    // extension warning, and the C++98 compat warning is disabled unless
    // explicitly enabled.
    S.Diag(ParamLoc, S.getLangOpts().CPlusPlus11 || S.getLangOpts().HLSL ?
         diag::warn_cxx98_compat_template_parameter_default_in_function_template
           : diag::ext_template_parameter_default_in_function_template)
      << DefArgRange;
    return false;
    // HLSL Change End

  case Sema::TPC_ClassTemplateMember:
    // C++0x [temp.param]p9:
    //   A default template-argument shall not be specified in the
    //   template-parameter-lists of the definition of a member of a
    //   class template that appears outside of the member's class.
    S.Diag(ParamLoc, diag::err_template_parameter_default_template_member)
      << DefArgRange;
    return true;

  case Sema::TPC_FriendClassTemplate:
  case Sema::TPC_FriendFunctionTemplate:
    // C++ [temp.param]p9:
    //   A default template-argument shall not be specified in a
    //   friend template declaration.
    S.Diag(ParamLoc, diag::err_template_parameter_default_friend_template)
      << DefArgRange;
    return true;

    // FIXME: C++0x [temp.param]p9 allows default template-arguments
    // for friend function templates if there is only a single
    // declaration (and it is a definition). Strange!
  }

  llvm_unreachable("Invalid TemplateParamListContext!");
}

/// \brief Check for unexpanded parameter packs within the template parameters
/// of a template template parameter, recursively.
static bool DiagnoseUnexpandedParameterPacks(Sema &S,
                                             TemplateTemplateParmDecl *TTP) {
  // A template template parameter which is a parameter pack is also a pack
  // expansion.
  if (TTP->isParameterPack())
    return false;

  TemplateParameterList *Params = TTP->getTemplateParameters();
  for (unsigned I = 0, N = Params->size(); I != N; ++I) {
    NamedDecl *P = Params->getParam(I);
    if (NonTypeTemplateParmDecl *NTTP = dyn_cast<NonTypeTemplateParmDecl>(P)) {
      if (!NTTP->isParameterPack() &&
          S.DiagnoseUnexpandedParameterPack(NTTP->getLocation(),
                                            NTTP->getTypeSourceInfo(),
                                      Sema::UPPC_NonTypeTemplateParameterType))
        return true;

      continue;
    }

    if (TemplateTemplateParmDecl *InnerTTP
                                        = dyn_cast<TemplateTemplateParmDecl>(P))
      if (DiagnoseUnexpandedParameterPacks(S, InnerTTP))
        return true;
  }

  return false;
}

/// \brief Checks the validity of a template parameter list, possibly
/// considering the template parameter list from a previous
/// declaration.
///
/// If an "old" template parameter list is provided, it must be
/// equivalent (per TemplateParameterListsAreEqual) to the "new"
/// template parameter list.
///
/// \param NewParams Template parameter list for a new template
/// declaration. This template parameter list will be updated with any
/// default arguments that are carried through from the previous
/// template parameter list.
///
/// \param OldParams If provided, template parameter list from a
/// previous declaration of the same template. Default template
/// arguments will be merged from the old template parameter list to
/// the new template parameter list.
///
/// \param TPC Describes the context in which we are checking the given
/// template parameter list.
///
/// \returns true if an error occurred, false otherwise.
bool Sema::CheckTemplateParameterList(TemplateParameterList *NewParams,
                                      TemplateParameterList *OldParams,
                                      TemplateParamListContext TPC) {
  bool Invalid = false;

  // C++ [temp.param]p10:
  //   The set of default template-arguments available for use with a
  //   template declaration or definition is obtained by merging the
  //   default arguments from the definition (if in scope) and all
  //   declarations in scope in the same way default function
  //   arguments are (8.3.6).
  bool SawDefaultArgument = false;
  SourceLocation PreviousDefaultArgLoc;

  // Dummy initialization to avoid warnings.
  TemplateParameterList::iterator OldParam = NewParams->end();
  if (OldParams)
    OldParam = OldParams->begin();

  bool RemoveDefaultArguments = false;
  for (TemplateParameterList::iterator NewParam = NewParams->begin(),
                                    NewParamEnd = NewParams->end();
       NewParam != NewParamEnd; ++NewParam) {
    // Variables used to diagnose redundant default arguments
    bool RedundantDefaultArg = false;
    SourceLocation OldDefaultLoc;
    SourceLocation NewDefaultLoc;

    // Variable used to diagnose missing default arguments
    bool MissingDefaultArg = false;

    // Variable used to diagnose non-final parameter packs
    bool SawParameterPack = false;

    if (TemplateTypeParmDecl *NewTypeParm
          = dyn_cast<TemplateTypeParmDecl>(*NewParam)) {
      // Check the presence of a default argument here.
      if (NewTypeParm->hasDefaultArgument() &&
          DiagnoseDefaultTemplateArgument(*this, TPC,
                                          NewTypeParm->getLocation(),
               NewTypeParm->getDefaultArgumentInfo()->getTypeLoc()
                                                       .getSourceRange()))
        NewTypeParm->removeDefaultArgument();

      // Merge default arguments for template type parameters.
      TemplateTypeParmDecl *OldTypeParm
          = OldParams? cast<TemplateTypeParmDecl>(*OldParam) : nullptr;
      if (NewTypeParm->isParameterPack()) {
        assert(!NewTypeParm->hasDefaultArgument() &&
               "Parameter packs can't have a default argument!");
        SawParameterPack = true;
      } else if (OldTypeParm && hasVisibleDefaultArgument(OldTypeParm) &&
                 NewTypeParm->hasDefaultArgument()) {
        OldDefaultLoc = OldTypeParm->getDefaultArgumentLoc();
        NewDefaultLoc = NewTypeParm->getDefaultArgumentLoc();
        SawDefaultArgument = true;
        RedundantDefaultArg = true;
        PreviousDefaultArgLoc = NewDefaultLoc;
      } else if (OldTypeParm && OldTypeParm->hasDefaultArgument()) {
        // Merge the default argument from the old declaration to the
        // new declaration.
        NewTypeParm->setInheritedDefaultArgument(Context, OldTypeParm);
        PreviousDefaultArgLoc = OldTypeParm->getDefaultArgumentLoc();
      } else if (NewTypeParm->hasDefaultArgument()) {
        SawDefaultArgument = true;
        PreviousDefaultArgLoc = NewTypeParm->getDefaultArgumentLoc();
      } else if (SawDefaultArgument)
        MissingDefaultArg = true;
    } else if (NonTypeTemplateParmDecl *NewNonTypeParm
               = dyn_cast<NonTypeTemplateParmDecl>(*NewParam)) {
      // Check for unexpanded parameter packs.
      if (!NewNonTypeParm->isParameterPack() &&
          DiagnoseUnexpandedParameterPack(NewNonTypeParm->getLocation(),
                                          NewNonTypeParm->getTypeSourceInfo(),
                                          UPPC_NonTypeTemplateParameterType)) {
        Invalid = true;
        continue;
      }

      // Check the presence of a default argument here.
      if (NewNonTypeParm->hasDefaultArgument() &&
          DiagnoseDefaultTemplateArgument(*this, TPC,
                                          NewNonTypeParm->getLocation(),
                    NewNonTypeParm->getDefaultArgument()->getSourceRange())) {
        NewNonTypeParm->removeDefaultArgument();
      }

      // Merge default arguments for non-type template parameters
      NonTypeTemplateParmDecl *OldNonTypeParm
        = OldParams? cast<NonTypeTemplateParmDecl>(*OldParam) : nullptr;
      if (NewNonTypeParm->isParameterPack()) {
        assert(!NewNonTypeParm->hasDefaultArgument() &&
               "Parameter packs can't have a default argument!");
        if (!NewNonTypeParm->isPackExpansion())
          SawParameterPack = true;
      } else if (OldNonTypeParm && hasVisibleDefaultArgument(OldNonTypeParm) &&
                 NewNonTypeParm->hasDefaultArgument()) {
        OldDefaultLoc = OldNonTypeParm->getDefaultArgumentLoc();
        NewDefaultLoc = NewNonTypeParm->getDefaultArgumentLoc();
        SawDefaultArgument = true;
        RedundantDefaultArg = true;
        PreviousDefaultArgLoc = NewDefaultLoc;
      } else if (OldNonTypeParm && OldNonTypeParm->hasDefaultArgument()) {
        // Merge the default argument from the old declaration to the
        // new declaration.
        NewNonTypeParm->setInheritedDefaultArgument(Context, OldNonTypeParm);
        PreviousDefaultArgLoc = OldNonTypeParm->getDefaultArgumentLoc();
      } else if (NewNonTypeParm->hasDefaultArgument()) {
        SawDefaultArgument = true;
        PreviousDefaultArgLoc = NewNonTypeParm->getDefaultArgumentLoc();
      } else if (SawDefaultArgument)
        MissingDefaultArg = true;
    } else {
      TemplateTemplateParmDecl *NewTemplateParm
        = cast<TemplateTemplateParmDecl>(*NewParam);

      // Check for unexpanded parameter packs, recursively.
      if (::DiagnoseUnexpandedParameterPacks(*this, NewTemplateParm)) {
        Invalid = true;
        continue;
      }

      // Check the presence of a default argument here.
      if (NewTemplateParm->hasDefaultArgument() &&
          DiagnoseDefaultTemplateArgument(*this, TPC,
                                          NewTemplateParm->getLocation(),
                     NewTemplateParm->getDefaultArgument().getSourceRange()))
        NewTemplateParm->removeDefaultArgument();

      // Merge default arguments for template template parameters
      TemplateTemplateParmDecl *OldTemplateParm
        = OldParams? cast<TemplateTemplateParmDecl>(*OldParam) : nullptr;
      if (NewTemplateParm->isParameterPack()) {
        assert(!NewTemplateParm->hasDefaultArgument() &&
               "Parameter packs can't have a default argument!");
        if (!NewTemplateParm->isPackExpansion())
          SawParameterPack = true;
      } else if (OldTemplateParm &&
                 hasVisibleDefaultArgument(OldTemplateParm) &&
                 NewTemplateParm->hasDefaultArgument()) {
        OldDefaultLoc = OldTemplateParm->getDefaultArgument().getLocation();
        NewDefaultLoc = NewTemplateParm->getDefaultArgument().getLocation();
        SawDefaultArgument = true;
        RedundantDefaultArg = true;
        PreviousDefaultArgLoc = NewDefaultLoc;
      } else if (OldTemplateParm && OldTemplateParm->hasDefaultArgument()) {
        // Merge the default argument from the old declaration to the
        // new declaration.
        NewTemplateParm->setInheritedDefaultArgument(Context, OldTemplateParm);
        PreviousDefaultArgLoc
          = OldTemplateParm->getDefaultArgument().getLocation();
      } else if (NewTemplateParm->hasDefaultArgument()) {
        SawDefaultArgument = true;
        PreviousDefaultArgLoc
          = NewTemplateParm->getDefaultArgument().getLocation();
      } else if (SawDefaultArgument)
        MissingDefaultArg = true;
    }

    // C++11 [temp.param]p11:
    //   If a template parameter of a primary class template or alias template
    //   is a template parameter pack, it shall be the last template parameter.
    if (SawParameterPack && (NewParam + 1) != NewParamEnd &&
        (TPC == TPC_ClassTemplate || TPC == TPC_VarTemplate ||
         TPC == TPC_TypeAliasTemplate)) {
      Diag((*NewParam)->getLocation(),
           diag::err_template_param_pack_must_be_last_template_parameter);
      Invalid = true;
    }

    if (RedundantDefaultArg) {
      // C++ [temp.param]p12:
      //   A template-parameter shall not be given default arguments
      //   by two different declarations in the same scope.
      Diag(NewDefaultLoc, diag::err_template_param_default_arg_redefinition);
      Diag(OldDefaultLoc, diag::note_template_param_prev_default_arg);
      Invalid = true;
    } else if (MissingDefaultArg && TPC != TPC_FunctionTemplate) {
      // C++ [temp.param]p11:
      //   If a template-parameter of a class template has a default
      //   template-argument, each subsequent template-parameter shall either
      //   have a default template-argument supplied or be a template parameter
      //   pack.
      Diag((*NewParam)->getLocation(),
           diag::err_template_param_default_arg_missing);
      Diag(PreviousDefaultArgLoc, diag::note_template_param_prev_default_arg);
      Invalid = true;
      RemoveDefaultArguments = true;
    }

    // If we have an old template parameter list that we're merging
    // in, move on to the next parameter.
    if (OldParams)
      ++OldParam;
  }

  // We were missing some default arguments at the end of the list, so remove
  // all of the default arguments.
  if (RemoveDefaultArguments) {
    for (TemplateParameterList::iterator NewParam = NewParams->begin(),
                                      NewParamEnd = NewParams->end();
         NewParam != NewParamEnd; ++NewParam) {
      if (TemplateTypeParmDecl *TTP = dyn_cast<TemplateTypeParmDecl>(*NewParam))
        TTP->removeDefaultArgument();
      else if (NonTypeTemplateParmDecl *NTTP
                                = dyn_cast<NonTypeTemplateParmDecl>(*NewParam))
        NTTP->removeDefaultArgument();
      else
        cast<TemplateTemplateParmDecl>(*NewParam)->removeDefaultArgument();
    }
  }

  return Invalid;
}

namespace {

/// A class which looks for a use of a certain level of template
/// parameter.
struct DependencyChecker : RecursiveASTVisitor<DependencyChecker> {
  typedef RecursiveASTVisitor<DependencyChecker> super;

  unsigned Depth;
  bool Match;
  SourceLocation MatchLoc;

  DependencyChecker(unsigned Depth) : Depth(Depth), Match(false) {}

  DependencyChecker(TemplateParameterList *Params) : Match(false) {
    NamedDecl *ND = Params->getParam(0);
    if (TemplateTypeParmDecl *PD = dyn_cast<TemplateTypeParmDecl>(ND)) {
      Depth = PD->getDepth();
    } else if (NonTypeTemplateParmDecl *PD =
                 dyn_cast<NonTypeTemplateParmDecl>(ND)) {
      Depth = PD->getDepth();
    } else {
      Depth = cast<TemplateTemplateParmDecl>(ND)->getDepth();
    }
  }

  bool Matches(unsigned ParmDepth, SourceLocation Loc = SourceLocation()) {
    if (ParmDepth >= Depth) {
      Match = true;
      MatchLoc = Loc;
      return true;
    }
    return false;
  }

  bool VisitTemplateTypeParmTypeLoc(TemplateTypeParmTypeLoc TL) {
    return !Matches(TL.getTypePtr()->getDepth(), TL.getNameLoc());
  }

  bool VisitTemplateTypeParmType(const TemplateTypeParmType *T) {
    return !Matches(T->getDepth());
  }

  bool TraverseTemplateName(TemplateName N) {
    if (TemplateTemplateParmDecl *PD =
          dyn_cast_or_null<TemplateTemplateParmDecl>(N.getAsTemplateDecl()))
      if (Matches(PD->getDepth()))
        return false;
    return super::TraverseTemplateName(N);
  }

  bool VisitDeclRefExpr(DeclRefExpr *E) {
    if (NonTypeTemplateParmDecl *PD =
          dyn_cast<NonTypeTemplateParmDecl>(E->getDecl()))
      if (Matches(PD->getDepth(), E->getExprLoc()))
        return false;
    return super::VisitDeclRefExpr(E);
  }

  bool VisitSubstTemplateTypeParmType(const SubstTemplateTypeParmType *T) {
    return TraverseType(T->getReplacementType());
  }

  bool
  VisitSubstTemplateTypeParmPackType(const SubstTemplateTypeParmPackType *T) {
    return TraverseTemplateArgument(T->getArgumentPack());
  }

  bool TraverseInjectedClassNameType(const InjectedClassNameType *T) {
    return TraverseType(T->getInjectedSpecializationType());
  }
};
}

/// Determines whether a given type depends on the given parameter
/// list.
static bool
DependsOnTemplateParameters(QualType T, TemplateParameterList *Params) {
  DependencyChecker Checker(Params);
  Checker.TraverseType(T);
  return Checker.Match;
}

// Find the source range corresponding to the named type in the given
// nested-name-specifier, if any.
static SourceRange getRangeOfTypeInNestedNameSpecifier(ASTContext &Context,
                                                       QualType T,
                                                       const CXXScopeSpec &SS) {
  NestedNameSpecifierLoc NNSLoc(SS.getScopeRep(), SS.location_data());
  while (NestedNameSpecifier *NNS = NNSLoc.getNestedNameSpecifier()) {
    if (const Type *CurType = NNS->getAsType()) {
      if (Context.hasSameUnqualifiedType(T, QualType(CurType, 0)))
        return NNSLoc.getTypeLoc().getSourceRange();
    } else
      break;
    
    NNSLoc = NNSLoc.getPrefix();
  }
  
  return SourceRange();
}

/// \brief Match the given template parameter lists to the given scope
/// specifier, returning the template parameter list that applies to the
/// name.
///
/// \param DeclStartLoc the start of the declaration that has a scope
/// specifier or a template parameter list.
///
/// \param DeclLoc The location of the declaration itself.
///
/// \param SS the scope specifier that will be matched to the given template
/// parameter lists. This scope specifier precedes a qualified name that is
/// being declared.
///
/// \param TemplateId The template-id following the scope specifier, if there
/// is one. Used to check for a missing 'template<>'.
///
/// \param ParamLists the template parameter lists, from the outermost to the
/// innermost template parameter lists.
///
/// \param IsFriend Whether to apply the slightly different rules for
/// matching template parameters to scope specifiers in friend
/// declarations.
///
/// \param IsExplicitSpecialization will be set true if the entity being
/// declared is an explicit specialization, false otherwise.
///
/// \returns the template parameter list, if any, that corresponds to the
/// name that is preceded by the scope specifier @p SS. This template
/// parameter list may have template parameters (if we're declaring a
/// template) or may have no template parameters (if we're declaring a
/// template specialization), or may be NULL (if what we're declaring isn't
/// itself a template).
TemplateParameterList *Sema::MatchTemplateParametersToScopeSpecifier(
    SourceLocation DeclStartLoc, SourceLocation DeclLoc, const CXXScopeSpec &SS,
    TemplateIdAnnotation *TemplateId,
    ArrayRef<TemplateParameterList *> ParamLists, bool IsFriend,
    bool &IsExplicitSpecialization, bool &Invalid) {
  IsExplicitSpecialization = false;
  Invalid = false;
  
  // The sequence of nested types to which we will match up the template
  // parameter lists. We first build this list by starting with the type named
  // by the nested-name-specifier and walking out until we run out of types.
  SmallVector<QualType, 4> NestedTypes;
  QualType T;
  if (SS.getScopeRep()) {
    if (CXXRecordDecl *Record 
              = dyn_cast_or_null<CXXRecordDecl>(computeDeclContext(SS, true)))
      T = Context.getTypeDeclType(Record);
    else
      T = QualType(SS.getScopeRep()->getAsType(), 0);
  }
  
  // If we found an explicit specialization that prevents us from needing
  // 'template<>' headers, this will be set to the location of that
  // explicit specialization.
  SourceLocation ExplicitSpecLoc;
  
  while (!T.isNull()) {
    NestedTypes.push_back(T);
    
    // Retrieve the parent of a record type.
    if (CXXRecordDecl *Record = T->getAsCXXRecordDecl()) {
      // If this type is an explicit specialization, we're done.
      if (ClassTemplateSpecializationDecl *Spec
          = dyn_cast<ClassTemplateSpecializationDecl>(Record)) {
        if (!isa<ClassTemplatePartialSpecializationDecl>(Spec) && 
            Spec->getSpecializationKind() == TSK_ExplicitSpecialization) {
          ExplicitSpecLoc = Spec->getLocation();
          break;
        }
      } else if (Record->getTemplateSpecializationKind()
                                                == TSK_ExplicitSpecialization) {
        ExplicitSpecLoc = Record->getLocation();
        break;
      }
      
      if (TypeDecl *Parent = dyn_cast<TypeDecl>(Record->getParent()))
        T = Context.getTypeDeclType(Parent);
      else
        T = QualType();
      continue;
    } 
    
    if (const TemplateSpecializationType *TST
                                     = T->getAs<TemplateSpecializationType>()) {
      if (TemplateDecl *Template = TST->getTemplateName().getAsTemplateDecl()) {
        if (TypeDecl *Parent = dyn_cast<TypeDecl>(Template->getDeclContext()))
          T = Context.getTypeDeclType(Parent);
        else
          T = QualType();
        continue;        
      }
    }
    
    // Look one step prior in a dependent template specialization type.
    if (const DependentTemplateSpecializationType *DependentTST
                          = T->getAs<DependentTemplateSpecializationType>()) {
      if (NestedNameSpecifier *NNS = DependentTST->getQualifier())
        T = QualType(NNS->getAsType(), 0);
      else
        T = QualType();
      continue;
    }
    
    // Look one step prior in a dependent name type.
    if (const DependentNameType *DependentName = T->getAs<DependentNameType>()){
      if (NestedNameSpecifier *NNS = DependentName->getQualifier())
        T = QualType(NNS->getAsType(), 0);
      else
        T = QualType();
      continue;
    }
    
    // Retrieve the parent of an enumeration type.
    if (const EnumType *EnumT = T->getAs<EnumType>()) {
      // FIXME: Forward-declared enums require a TSK_ExplicitSpecialization
      // check here.
      EnumDecl *Enum = EnumT->getDecl();
      
      // Get to the parent type.
      if (TypeDecl *Parent = dyn_cast<TypeDecl>(Enum->getParent()))
        T = Context.getTypeDeclType(Parent);
      else
        T = QualType();      
      continue;
    }

    T = QualType();
  }
  // Reverse the nested types list, since we want to traverse from the outermost
  // to the innermost while checking template-parameter-lists.
  std::reverse(NestedTypes.begin(), NestedTypes.end());

  // C++0x [temp.expl.spec]p17:
  //   A member or a member template may be nested within many
  //   enclosing class templates. In an explicit specialization for
  //   such a member, the member declaration shall be preceded by a
  //   template<> for each enclosing class template that is
  //   explicitly specialized.
  bool SawNonEmptyTemplateParameterList = false;

  auto CheckExplicitSpecialization = [&](SourceRange Range, bool Recovery) {
    if (SawNonEmptyTemplateParameterList) {
      Diag(DeclLoc, diag::err_specialize_member_of_template)
        << !Recovery << Range;
      Invalid = true;
      IsExplicitSpecialization = false;
      return true;
    }

    return false;
  };

  auto DiagnoseMissingExplicitSpecialization = [&] (SourceRange Range) {
    // Check that we can have an explicit specialization here.
    if (CheckExplicitSpecialization(Range, true))
      return true;

    // We don't have a template header, but we should.
    SourceLocation ExpectedTemplateLoc;
    if (!ParamLists.empty())
      ExpectedTemplateLoc = ParamLists[0]->getTemplateLoc();
    else
      ExpectedTemplateLoc = DeclStartLoc;

    Diag(DeclLoc, diag::err_template_spec_needs_header)
      << Range
      << FixItHint::CreateInsertion(ExpectedTemplateLoc, "template<> ");
    return false;
  };

  unsigned ParamIdx = 0;
  for (unsigned TypeIdx = 0, NumTypes = NestedTypes.size(); TypeIdx != NumTypes;
       ++TypeIdx) {
    T = NestedTypes[TypeIdx];
    
    // Whether we expect a 'template<>' header.
    bool NeedEmptyTemplateHeader = false;

    // Whether we expect a template header with parameters.
    bool NeedNonemptyTemplateHeader = false;
    
    // For a dependent type, the set of template parameters that we
    // expect to see.
    TemplateParameterList *ExpectedTemplateParams = nullptr;

    // C++0x [temp.expl.spec]p15:
    //   A member or a member template may be nested within many enclosing 
    //   class templates. In an explicit specialization for such a member, the 
    //   member declaration shall be preceded by a template<> for each 
    //   enclosing class template that is explicitly specialized.
    if (CXXRecordDecl *Record = T->getAsCXXRecordDecl()) {
      if (ClassTemplatePartialSpecializationDecl *Partial
            = dyn_cast<ClassTemplatePartialSpecializationDecl>(Record)) {
        ExpectedTemplateParams = Partial->getTemplateParameters();
        NeedNonemptyTemplateHeader = true;
      } else if (Record->isDependentType()) {
        if (Record->getDescribedClassTemplate()) {
          ExpectedTemplateParams = Record->getDescribedClassTemplate()
                                                      ->getTemplateParameters();
          NeedNonemptyTemplateHeader = true;
        }
      } else if (ClassTemplateSpecializationDecl *Spec
                     = dyn_cast<ClassTemplateSpecializationDecl>(Record)) {
        // C++0x [temp.expl.spec]p4:
        //   Members of an explicitly specialized class template are defined
        //   in the same manner as members of normal classes, and not using 
        //   the template<> syntax. 
        if (Spec->getSpecializationKind() != TSK_ExplicitSpecialization)
          NeedEmptyTemplateHeader = true;
        else
          continue;
      } else if (Record->getTemplateSpecializationKind()) {
        if (Record->getTemplateSpecializationKind() 
                                                != TSK_ExplicitSpecialization &&
            TypeIdx == NumTypes - 1)
          IsExplicitSpecialization = true;
        
        continue;
      }
    } else if (const TemplateSpecializationType *TST
                                     = T->getAs<TemplateSpecializationType>()) {
      if (TemplateDecl *Template = TST->getTemplateName().getAsTemplateDecl()) {
        ExpectedTemplateParams = Template->getTemplateParameters();
        NeedNonemptyTemplateHeader = true;        
      }
    } else if (T->getAs<DependentTemplateSpecializationType>()) {
      // FIXME:  We actually could/should check the template arguments here
      // against the corresponding template parameter list.
      NeedNonemptyTemplateHeader = false;
    } 
    
    // C++ [temp.expl.spec]p16:
    //   In an explicit specialization declaration for a member of a class 
    //   template or a member template that ap- pears in namespace scope, the 
    //   member template and some of its enclosing class templates may remain 
    //   unspecialized, except that the declaration shall not explicitly 
    //   specialize a class member template if its en- closing class templates 
    //   are not explicitly specialized as well.
    if (ParamIdx < ParamLists.size()) {
      if (ParamLists[ParamIdx]->size() == 0) {
        if (CheckExplicitSpecialization(ParamLists[ParamIdx]->getSourceRange(),
                                        false))
          return nullptr;
      } else
        SawNonEmptyTemplateParameterList = true;
    }
    
    if (NeedEmptyTemplateHeader) {
      // If we're on the last of the types, and we need a 'template<>' header
      // here, then it's an explicit specialization.
      if (TypeIdx == NumTypes - 1)
        IsExplicitSpecialization = true;

      if (ParamIdx < ParamLists.size()) {
        if (ParamLists[ParamIdx]->size() > 0) {
          // The header has template parameters when it shouldn't. Complain.
          Diag(ParamLists[ParamIdx]->getTemplateLoc(), 
               diag::err_template_param_list_matches_nontemplate)
            << T
            << SourceRange(ParamLists[ParamIdx]->getLAngleLoc(),
                           ParamLists[ParamIdx]->getRAngleLoc())
            << getRangeOfTypeInNestedNameSpecifier(Context, T, SS);
          Invalid = true;
          return nullptr;
        }

        // Consume this template header.
        ++ParamIdx;
        continue;
      }

      if (!IsFriend)
        if (DiagnoseMissingExplicitSpecialization(
                getRangeOfTypeInNestedNameSpecifier(Context, T, SS)))
          return nullptr;

      continue;
    }

    if (NeedNonemptyTemplateHeader) {
      // In friend declarations we can have template-ids which don't
      // depend on the corresponding template parameter lists.  But
      // assume that empty parameter lists are supposed to match this
      // template-id.
      if (IsFriend && T->isDependentType()) {
        if (ParamIdx < ParamLists.size() &&
            DependsOnTemplateParameters(T, ParamLists[ParamIdx]))
          ExpectedTemplateParams = nullptr;
        else 
          continue;
      }

      if (ParamIdx < ParamLists.size()) {
        // Check the template parameter list, if we can.
        if (ExpectedTemplateParams &&
            !TemplateParameterListsAreEqual(ParamLists[ParamIdx],
                                            ExpectedTemplateParams,
                                            true, TPL_TemplateMatch))
          Invalid = true;

        if (!Invalid &&
            CheckTemplateParameterList(ParamLists[ParamIdx], nullptr,
                                       TPC_ClassTemplateMember))
          Invalid = true;
        
        ++ParamIdx;
        continue;
      }
      
      Diag(DeclLoc, diag::err_template_spec_needs_template_parameters)
        << T
        << getRangeOfTypeInNestedNameSpecifier(Context, T, SS);
      Invalid = true;
      continue;
    }
  }

  // If there were at least as many template-ids as there were template
  // parameter lists, then there are no template parameter lists remaining for
  // the declaration itself.
  if (ParamIdx >= ParamLists.size()) {
    if (TemplateId && !IsFriend) {
      // We don't have a template header for the declaration itself, but we
      // should.
      IsExplicitSpecialization = true;
      DiagnoseMissingExplicitSpecialization(SourceRange(TemplateId->LAngleLoc,
                                                        TemplateId->RAngleLoc));

      // Fabricate an empty template parameter list for the invented header.
      return TemplateParameterList::Create(Context, SourceLocation(),
                                           SourceLocation(), nullptr, 0,
                                           SourceLocation());
    }

    return nullptr;
  }

  // If there were too many template parameter lists, complain about that now.
  if (ParamIdx < ParamLists.size() - 1) {
    bool HasAnyExplicitSpecHeader = false;
    bool AllExplicitSpecHeaders = true;
    for (unsigned I = ParamIdx, E = ParamLists.size() - 1; I != E; ++I) {
      if (ParamLists[I]->size() == 0)
        HasAnyExplicitSpecHeader = true;
      else
        AllExplicitSpecHeaders = false;
    }

    Diag(ParamLists[ParamIdx]->getTemplateLoc(),
         AllExplicitSpecHeaders ? diag::warn_template_spec_extra_headers
                                : diag::err_template_spec_extra_headers)
        << SourceRange(ParamLists[ParamIdx]->getTemplateLoc(),
                       ParamLists[ParamLists.size() - 2]->getRAngleLoc());

    // If there was a specialization somewhere, such that 'template<>' is
    // not required, and there were any 'template<>' headers, note where the
    // specialization occurred.
    if (ExplicitSpecLoc.isValid() && HasAnyExplicitSpecHeader)
      Diag(ExplicitSpecLoc, 
           diag::note_explicit_template_spec_does_not_need_header)
        << NestedTypes.back();
    
    // We have a template parameter list with no corresponding scope, which
    // means that the resulting template declaration can't be instantiated
    // properly (we'll end up with dependent nodes when we shouldn't).
    if (!AllExplicitSpecHeaders)
      Invalid = true;
  }

  // C++ [temp.expl.spec]p16:
  //   In an explicit specialization declaration for a member of a class 
  //   template or a member template that ap- pears in namespace scope, the 
  //   member template and some of its enclosing class templates may remain 
  //   unspecialized, except that the declaration shall not explicitly 
  //   specialize a class member template if its en- closing class templates 
  //   are not explicitly specialized as well.
  if (ParamLists.back()->size() == 0 &&
      CheckExplicitSpecialization(ParamLists[ParamIdx]->getSourceRange(),
                                  false))
    return nullptr;

  // Return the last template parameter list, which corresponds to the
  // entity being declared.
  return ParamLists.back();
}

void Sema::NoteAllFoundTemplates(TemplateName Name) {
  if (TemplateDecl *Template = Name.getAsTemplateDecl()) {
    Diag(Template->getLocation(), diag::note_template_declared_here)
        << (isa<FunctionTemplateDecl>(Template)
                ? 0
                : isa<ClassTemplateDecl>(Template)
                      ? 1
                      : isa<VarTemplateDecl>(Template)
                            ? 2
                            : isa<TypeAliasTemplateDecl>(Template) ? 3 : 4)
        << Template->getDeclName();
    return;
  }
  
  if (OverloadedTemplateStorage *OST = Name.getAsOverloadedTemplate()) {
    for (OverloadedTemplateStorage::iterator I = OST->begin(), 
                                          IEnd = OST->end();
         I != IEnd; ++I)
      Diag((*I)->getLocation(), diag::note_template_declared_here)
        << 0 << (*I)->getDeclName();
    
    return;
  }
}

QualType Sema::CheckTemplateIdType(TemplateName Name,
                                   SourceLocation TemplateLoc,
                                   TemplateArgumentListInfo &TemplateArgs) {
  DependentTemplateName *DTN
    = Name.getUnderlying().getAsDependentTemplateName();
  if (DTN && DTN->isIdentifier())
    // When building a template-id where the template-name is dependent,
    // assume the template is a type template. Either our assumption is
    // correct, or the code is ill-formed and will be diagnosed when the
    // dependent name is substituted.
    return Context.getDependentTemplateSpecializationType(ETK_None,
                                                          DTN->getQualifier(),
                                                          DTN->getIdentifier(),
                                                          TemplateArgs);

  TemplateDecl *Template = Name.getAsTemplateDecl();
  if (!Template || isa<FunctionTemplateDecl>(Template) ||
      isa<VarTemplateDecl>(Template)) {
    // We might have a substituted template template parameter pack. If so,
    // build a template specialization type for it.
    if (Name.getAsSubstTemplateTemplateParmPack())
      return Context.getTemplateSpecializationType(Name, TemplateArgs);

    Diag(TemplateLoc, diag::err_template_id_not_a_type)
      << Name;
    NoteAllFoundTemplates(Name);
    return QualType();
  }

  // Check that the template argument list is well-formed for this
  // template.
  SmallVector<TemplateArgument, 4> Converted;
  if (CheckTemplateArgumentList(Template, TemplateLoc, TemplateArgs,
                                false, Converted))
    return QualType();

  // HLSL Change Starts - check template values for HLSL object/matrix/vector signatures
  if (getLangOpts().HLSL && Template->isImplicit() &&
      hlsl::CheckTemplateArgumentListForHLSL(*this, Template, TemplateLoc,
                                             TemplateArgs))
    return QualType();
  // HLSL Change Ends

  QualType CanonType;

  bool InstantiationDependent = false;
  if (TypeAliasTemplateDecl *AliasTemplate =
          dyn_cast<TypeAliasTemplateDecl>(Template)) {
    // Find the canonical type for this type alias template specialization.
    TypeAliasDecl *Pattern = AliasTemplate->getTemplatedDecl();
    if (Pattern->isInvalidDecl())
      return QualType();

    TemplateArgumentList TemplateArgs(TemplateArgumentList::OnStack,
                                      Converted.data(), Converted.size());

    // Only substitute for the innermost template argument list.
    MultiLevelTemplateArgumentList TemplateArgLists;
    TemplateArgLists.addOuterTemplateArguments(&TemplateArgs);
    unsigned Depth = AliasTemplate->getTemplateParameters()->getDepth();
    for (unsigned I = 0; I < Depth; ++I)
      TemplateArgLists.addOuterTemplateArguments(None);

    LocalInstantiationScope Scope(*this);
    InstantiatingTemplate Inst(*this, TemplateLoc, Template);
    if (Inst.isInvalid())
      return QualType();

    CanonType = SubstType(Pattern->getUnderlyingType(),
                          TemplateArgLists, AliasTemplate->getLocation(),
                          AliasTemplate->getDeclName());
    if (CanonType.isNull())
      return QualType();
  } else if (Name.isDependent() ||
             TemplateSpecializationType::anyDependentTemplateArguments(
               TemplateArgs, InstantiationDependent)) {
    // This class template specialization is a dependent
    // type. Therefore, its canonical type is another class template
    // specialization type that contains all of the converted
    // arguments in canonical form. This ensures that, e.g., A<T> and
    // A<T, T> have identical types when A is declared as:
    //
    //   template<typename T, typename U = T> struct A;
    TemplateName CanonName = Context.getCanonicalTemplateName(Name);
    CanonType = Context.getTemplateSpecializationType(CanonName,
                                                      Converted.data(),
                                                      Converted.size());

    // FIXME: CanonType is not actually the canonical type, and unfortunately
    // it is a TemplateSpecializationType that we will never use again.
    // In the future, we need to teach getTemplateSpecializationType to only
    // build the canonical type and return that to us.
    CanonType = Context.getCanonicalType(CanonType);

    // This might work out to be a current instantiation, in which
    // case the canonical type needs to be the InjectedClassNameType.
    //
    // TODO: in theory this could be a simple hashtable lookup; most
    // changes to CurContext don't change the set of current
    // instantiations.
    if (isa<ClassTemplateDecl>(Template)) {
      for (DeclContext *Ctx = CurContext; Ctx; Ctx = Ctx->getLookupParent()) {
        // If we get out to a namespace, we're done.
        if (Ctx->isFileContext()) break;

        // If this isn't a record, keep looking.
        CXXRecordDecl *Record = dyn_cast<CXXRecordDecl>(Ctx);
        if (!Record) continue;

        // Look for one of the two cases with InjectedClassNameTypes
        // and check whether it's the same template.
        if (!isa<ClassTemplatePartialSpecializationDecl>(Record) &&
            !Record->getDescribedClassTemplate())
          continue;

        // Fetch the injected class name type and check whether its
        // injected type is equal to the type we just built.
        QualType ICNT = Context.getTypeDeclType(Record);
        QualType Injected = cast<InjectedClassNameType>(ICNT)
          ->getInjectedSpecializationType();

        if (CanonType != Injected->getCanonicalTypeInternal())
          continue;

        // If so, the canonical type of this TST is the injected
        // class name type of the record we just found.
        assert(ICNT.isCanonical());
        CanonType = ICNT;
        break;
      }
    }
  } else if (ClassTemplateDecl *ClassTemplate
               = dyn_cast<ClassTemplateDecl>(Template)) {
    // Find the class template specialization declaration that
    // corresponds to these arguments.
    void *InsertPos = nullptr;
    ClassTemplateSpecializationDecl *Decl
      = ClassTemplate->findSpecialization(Converted, InsertPos);
    if (!Decl) {
      // This is the first time we have referenced this class template
      // specialization. Create the canonical declaration and add it to
      // the set of specializations.
      Decl = ClassTemplateSpecializationDecl::Create(Context,
                            ClassTemplate->getTemplatedDecl()->getTagKind(),
                                                ClassTemplate->getDeclContext(),
                            ClassTemplate->getTemplatedDecl()->getLocStart(),
                                                ClassTemplate->getLocation(),
                                                     ClassTemplate,
                                                     Converted.data(),
                                                     Converted.size(), nullptr);
      ClassTemplate->AddSpecialization(Decl, InsertPos);
      if (ClassTemplate->isOutOfLine())
        Decl->setLexicalDeclContext(ClassTemplate->getLexicalDeclContext());
    }

    // Diagnose uses of this specialization.
    (void)DiagnoseUseOfDecl(Decl, TemplateLoc);

    CanonType = Context.getTypeDeclType(Decl);
    assert(isa<RecordType>(CanonType) &&
           "type of non-dependent specialization is not a RecordType");
  }

  // Build the fully-sugared type for this class template
  // specialization, which refers back to the class template
  // specialization we created or found.
  return Context.getTemplateSpecializationType(Name, TemplateArgs, CanonType);
}

TypeResult
Sema::ActOnTemplateIdType(CXXScopeSpec &SS, SourceLocation TemplateKWLoc,
                          TemplateTy TemplateD, SourceLocation TemplateLoc,
                          SourceLocation LAngleLoc,
                          ASTTemplateArgsPtr TemplateArgsIn,
                          SourceLocation RAngleLoc,
                          bool IsCtorOrDtorName) {
  if (SS.isInvalid())
    return true;

  TemplateName Template = TemplateD.get();

  // Translate the parser's template argument list in our AST format.
  TemplateArgumentListInfo TemplateArgs(LAngleLoc, RAngleLoc);
  translateTemplateArguments(TemplateArgsIn, TemplateArgs);

  if (DependentTemplateName *DTN = Template.getAsDependentTemplateName()) {
    QualType T
      = Context.getDependentTemplateSpecializationType(ETK_None,
                                                       DTN->getQualifier(),
                                                       DTN->getIdentifier(),
                                                       TemplateArgs);
    // Build type-source information.
    TypeLocBuilder TLB;
    DependentTemplateSpecializationTypeLoc SpecTL
      = TLB.push<DependentTemplateSpecializationTypeLoc>(T);
    SpecTL.setElaboratedKeywordLoc(SourceLocation());
    SpecTL.setQualifierLoc(SS.getWithLocInContext(Context));
    SpecTL.setTemplateKeywordLoc(TemplateKWLoc);
    SpecTL.setTemplateNameLoc(TemplateLoc);
    SpecTL.setLAngleLoc(LAngleLoc);
    SpecTL.setRAngleLoc(RAngleLoc);
    for (unsigned I = 0, N = SpecTL.getNumArgs(); I != N; ++I)
      SpecTL.setArgLocInfo(I, TemplateArgs[I].getLocInfo());
    return CreateParsedType(T, TLB.getTypeSourceInfo(Context, T));
  }
  
  QualType Result = CheckTemplateIdType(Template, TemplateLoc, TemplateArgs);

  if (Result.isNull())
    return true;

  // Build type-source information.
  TypeLocBuilder TLB;
  TemplateSpecializationTypeLoc SpecTL
    = TLB.push<TemplateSpecializationTypeLoc>(Result);
  SpecTL.setTemplateKeywordLoc(TemplateKWLoc);
  SpecTL.setTemplateNameLoc(TemplateLoc);
  SpecTL.setLAngleLoc(LAngleLoc);
  SpecTL.setRAngleLoc(RAngleLoc);
  for (unsigned i = 0, e = SpecTL.getNumArgs(); i != e; ++i)
    SpecTL.setArgLocInfo(i, TemplateArgs[i].getLocInfo());

  // NOTE: avoid constructing an ElaboratedTypeLoc if this is a
  // constructor or destructor name (in such a case, the scope specifier
  // will be attached to the enclosing Decl or Expr node).
  if (SS.isNotEmpty() && !IsCtorOrDtorName) {
    // Create an elaborated-type-specifier containing the nested-name-specifier.
    Result = Context.getElaboratedType(ETK_None, SS.getScopeRep(), Result);
    ElaboratedTypeLoc ElabTL = TLB.push<ElaboratedTypeLoc>(Result);
    ElabTL.setElaboratedKeywordLoc(SourceLocation());
    ElabTL.setQualifierLoc(SS.getWithLocInContext(Context));
  }
  
  return CreateParsedType(Result, TLB.getTypeSourceInfo(Context, Result));
}

TypeResult Sema::ActOnTagTemplateIdType(TagUseKind TUK,
                                        TypeSpecifierType TagSpec,
                                        SourceLocation TagLoc,
                                        CXXScopeSpec &SS,
                                        SourceLocation TemplateKWLoc,
                                        TemplateTy TemplateD,
                                        SourceLocation TemplateLoc,
                                        SourceLocation LAngleLoc,
                                        ASTTemplateArgsPtr TemplateArgsIn,
                                        SourceLocation RAngleLoc) {
  TemplateName Template = TemplateD.get();
  
  // Translate the parser's template argument list in our AST format.
  TemplateArgumentListInfo TemplateArgs(LAngleLoc, RAngleLoc);
  translateTemplateArguments(TemplateArgsIn, TemplateArgs);
  
  // Determine the tag kind
  TagTypeKind TagKind = TypeWithKeyword::getTagTypeKindForTypeSpec(TagSpec);
  ElaboratedTypeKeyword Keyword
    = TypeWithKeyword::getKeywordForTagTypeKind(TagKind);

  if (DependentTemplateName *DTN = Template.getAsDependentTemplateName()) {
    QualType T = Context.getDependentTemplateSpecializationType(Keyword,
                                                          DTN->getQualifier(), 
                                                          DTN->getIdentifier(), 
                                                                TemplateArgs);
    
    // Build type-source information.    
    TypeLocBuilder TLB;
    DependentTemplateSpecializationTypeLoc SpecTL
      = TLB.push<DependentTemplateSpecializationTypeLoc>(T);
    SpecTL.setElaboratedKeywordLoc(TagLoc);
    SpecTL.setQualifierLoc(SS.getWithLocInContext(Context));
    SpecTL.setTemplateKeywordLoc(TemplateKWLoc);
    SpecTL.setTemplateNameLoc(TemplateLoc);
    SpecTL.setLAngleLoc(LAngleLoc);
    SpecTL.setRAngleLoc(RAngleLoc);
    for (unsigned I = 0, N = SpecTL.getNumArgs(); I != N; ++I)
      SpecTL.setArgLocInfo(I, TemplateArgs[I].getLocInfo());
    return CreateParsedType(T, TLB.getTypeSourceInfo(Context, T));
  }

  if (TypeAliasTemplateDecl *TAT =
        dyn_cast_or_null<TypeAliasTemplateDecl>(Template.getAsTemplateDecl())) {
    // C++0x [dcl.type.elab]p2:
    //   If the identifier resolves to a typedef-name or the simple-template-id
    //   resolves to an alias template specialization, the
    //   elaborated-type-specifier is ill-formed.
    Diag(TemplateLoc, diag::err_tag_reference_non_tag) << 4;
    Diag(TAT->getLocation(), diag::note_declared_at);
  }
  
  QualType Result = CheckTemplateIdType(Template, TemplateLoc, TemplateArgs);
  if (Result.isNull())
    return TypeResult(true);
  
  // Check the tag kind
  if (const RecordType *RT = Result->getAs<RecordType>()) {
    RecordDecl *D = RT->getDecl();
    
    IdentifierInfo *Id = D->getIdentifier();
    assert(Id && "templated class must have an identifier");
    
    if (!isAcceptableTagRedeclaration(D, TagKind, TUK == TUK_Definition,
                                      TagLoc, Id)) {
      Diag(TagLoc, diag::err_use_with_wrong_tag)
        << Result
        << FixItHint::CreateReplacement(SourceRange(TagLoc), D->getKindName());
      Diag(D->getLocation(), diag::note_previous_use);
    }
  }

  // Provide source-location information for the template specialization.
  TypeLocBuilder TLB;
  TemplateSpecializationTypeLoc SpecTL
    = TLB.push<TemplateSpecializationTypeLoc>(Result);
  SpecTL.setTemplateKeywordLoc(TemplateKWLoc);
  SpecTL.setTemplateNameLoc(TemplateLoc);
  SpecTL.setLAngleLoc(LAngleLoc);
  SpecTL.setRAngleLoc(RAngleLoc);
  for (unsigned i = 0, e = SpecTL.getNumArgs(); i != e; ++i)
    SpecTL.setArgLocInfo(i, TemplateArgs[i].getLocInfo());

  // Construct an elaborated type containing the nested-name-specifier (if any)
  // and tag keyword.
  Result = Context.getElaboratedType(Keyword, SS.getScopeRep(), Result);
  ElaboratedTypeLoc ElabTL = TLB.push<ElaboratedTypeLoc>(Result);
  ElabTL.setElaboratedKeywordLoc(TagLoc);
  ElabTL.setQualifierLoc(SS.getWithLocInContext(Context));
  return CreateParsedType(Result, TLB.getTypeSourceInfo(Context, Result));
}

static bool CheckTemplatePartialSpecializationArgs(
    Sema &S, SourceLocation NameLoc, TemplateParameterList *TemplateParams,
    unsigned ExplicitArgs, SmallVectorImpl<TemplateArgument> &TemplateArgs);

static bool CheckTemplateSpecializationScope(Sema &S, NamedDecl *Specialized,
                                             NamedDecl *PrevDecl,
                                             SourceLocation Loc,
                                             bool IsPartialSpecialization);

static TemplateSpecializationKind getTemplateSpecializationKind(Decl *D);

static bool isTemplateArgumentTemplateParameter(
    const TemplateArgument &Arg, unsigned Depth, unsigned Index) {
  switch (Arg.getKind()) {
  case TemplateArgument::Null:
  case TemplateArgument::NullPtr:
  case TemplateArgument::Integral:
  case TemplateArgument::Declaration:
  case TemplateArgument::Pack:
  case TemplateArgument::TemplateExpansion:
    return false;

  case TemplateArgument::Type: {
    QualType Type = Arg.getAsType();
    const TemplateTypeParmType *TPT =
        Arg.getAsType()->getAs<TemplateTypeParmType>();
    return TPT && !Type.hasQualifiers() &&
           TPT->getDepth() == Depth && TPT->getIndex() == Index;
  }

  case TemplateArgument::Expression: {
    DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(Arg.getAsExpr());
    if (!DRE || !DRE->getDecl())
      return false;
    const NonTypeTemplateParmDecl *NTTP =
        dyn_cast<NonTypeTemplateParmDecl>(DRE->getDecl());
    return NTTP && NTTP->getDepth() == Depth && NTTP->getIndex() == Index;
  }

  case TemplateArgument::Template:
    const TemplateTemplateParmDecl *TTP =
        dyn_cast_or_null<TemplateTemplateParmDecl>(
            Arg.getAsTemplateOrTemplatePattern().getAsTemplateDecl());
    return TTP && TTP->getDepth() == Depth && TTP->getIndex() == Index;
  }
  llvm_unreachable("unexpected kind of template argument");
}

static bool isSameAsPrimaryTemplate(TemplateParameterList *Params,
                                    ArrayRef<TemplateArgument> Args) {
  if (Params->size() != Args.size())
    return false;

  unsigned Depth = Params->getDepth();

  for (unsigned I = 0, N = Args.size(); I != N; ++I) {
    TemplateArgument Arg = Args[I];

    // If the parameter is a pack expansion, the argument must be a pack
    // whose only element is a pack expansion.
    if (Params->getParam(I)->isParameterPack()) {
      if (Arg.getKind() != TemplateArgument::Pack || Arg.pack_size() != 1 ||
          !Arg.pack_begin()->isPackExpansion())
        return false;
      Arg = Arg.pack_begin()->getPackExpansionPattern();
    }

    if (!isTemplateArgumentTemplateParameter(Arg, Depth, I))
      return false;
  }

  return true;
}

/// Convert the parser's template argument list representation into our form.
static TemplateArgumentListInfo
makeTemplateArgumentListInfo(Sema &S, TemplateIdAnnotation &TemplateId) {
  TemplateArgumentListInfo TemplateArgs(TemplateId.LAngleLoc,
                                        TemplateId.RAngleLoc);
  ASTTemplateArgsPtr TemplateArgsPtr(TemplateId.getTemplateArgs(),
                                     TemplateId.NumArgs);
  S.translateTemplateArguments(TemplateArgsPtr, TemplateArgs);
  return TemplateArgs;
}

DeclResult Sema::ActOnVarTemplateSpecialization(
    Scope *S, Declarator &D, TypeSourceInfo *DI, SourceLocation TemplateKWLoc,
    TemplateParameterList *TemplateParams, StorageClass SC,
    bool IsPartialSpecialization) {
  // D must be variable template id.
  assert(D.getName().getKind() == UnqualifiedId::IK_TemplateId &&
         "Variable template specialization is declared with a template it.");

  TemplateIdAnnotation *TemplateId = D.getName().TemplateId;
  TemplateArgumentListInfo TemplateArgs =
      makeTemplateArgumentListInfo(*this, *TemplateId);
  SourceLocation TemplateNameLoc = D.getIdentifierLoc();
  SourceLocation LAngleLoc = TemplateId->LAngleLoc;
  SourceLocation RAngleLoc = TemplateId->RAngleLoc;

  TemplateName Name = TemplateId->Template.get();

  // The template-id must name a variable template.
  VarTemplateDecl *VarTemplate =
      dyn_cast_or_null<VarTemplateDecl>(Name.getAsTemplateDecl());
  if (!VarTemplate) {
    NamedDecl *FnTemplate;
    if (auto *OTS = Name.getAsOverloadedTemplate())
      FnTemplate = *OTS->begin();
    else
      FnTemplate = dyn_cast_or_null<FunctionTemplateDecl>(Name.getAsTemplateDecl());
    if (FnTemplate)
      return Diag(D.getIdentifierLoc(), diag::err_var_spec_no_template_but_method)
               << FnTemplate->getDeclName();
    return Diag(D.getIdentifierLoc(), diag::err_var_spec_no_template)
             << IsPartialSpecialization;
  }

  // Check for unexpanded parameter packs in any of the template arguments.
  for (unsigned I = 0, N = TemplateArgs.size(); I != N; ++I)
    if (DiagnoseUnexpandedParameterPack(TemplateArgs[I],
                                        UPPC_PartialSpecialization))
      return true;

  // Check that the template argument list is well-formed for this
  // template.
  SmallVector<TemplateArgument, 4> Converted;
  if (CheckTemplateArgumentList(VarTemplate, TemplateNameLoc, TemplateArgs,
                                false, Converted))
    return true;

  // Check that the type of this variable template specialization
  // matches the expected type.
  TypeSourceInfo *ExpectedDI;
  {
    // Do substitution on the type of the declaration
    TemplateArgumentList TemplateArgList(TemplateArgumentList::OnStack,
                                         Converted.data(), Converted.size());
    InstantiatingTemplate Inst(*this, TemplateKWLoc, VarTemplate);
    if (Inst.isInvalid())
      return true;
    VarDecl *Templated = VarTemplate->getTemplatedDecl();
    ExpectedDI =
        SubstType(Templated->getTypeSourceInfo(),
                  MultiLevelTemplateArgumentList(TemplateArgList),
                  Templated->getTypeSpecStartLoc(), Templated->getDeclName());
  }
  if (!ExpectedDI)
    return true;

  // Find the variable template (partial) specialization declaration that
  // corresponds to these arguments.
  if (IsPartialSpecialization) {
    if (CheckTemplatePartialSpecializationArgs(
            *this, TemplateNameLoc, VarTemplate->getTemplateParameters(),
            TemplateArgs.size(), Converted))
      return true;

    bool InstantiationDependent;
    if (!Name.isDependent() &&
        !TemplateSpecializationType::anyDependentTemplateArguments(
            TemplateArgs.getArgumentArray(), TemplateArgs.size(),
            InstantiationDependent)) {
      Diag(TemplateNameLoc, diag::err_partial_spec_fully_specialized)
          << VarTemplate->getDeclName();
      IsPartialSpecialization = false;
    }

    if (isSameAsPrimaryTemplate(VarTemplate->getTemplateParameters(),
                                Converted)) {
      // C++ [temp.class.spec]p9b3:
      //
      //   -- The argument list of the specialization shall not be identical
      //      to the implicit argument list of the primary template.
      Diag(TemplateNameLoc, diag::err_partial_spec_args_match_primary_template)
        << /*variable template*/ 1
        << /*is definition*/(SC != SC_Extern && !CurContext->isRecord())
        << FixItHint::CreateRemoval(SourceRange(LAngleLoc, RAngleLoc));
      // FIXME: Recover from this by treating the declaration as a redeclaration
      // of the primary template.
      return true;
    }
  }

  void *InsertPos = nullptr;
  VarTemplateSpecializationDecl *PrevDecl = nullptr;

  if (IsPartialSpecialization)
    // FIXME: Template parameter list matters too
    PrevDecl = VarTemplate->findPartialSpecialization(Converted, InsertPos);
  else
    PrevDecl = VarTemplate->findSpecialization(Converted, InsertPos);

  VarTemplateSpecializationDecl *Specialization = nullptr;

  // Check whether we can declare a variable template specialization in
  // the current scope.
  if (CheckTemplateSpecializationScope(*this, VarTemplate, PrevDecl,
                                       TemplateNameLoc,
                                       IsPartialSpecialization))
    return true;

  if (PrevDecl && PrevDecl->getSpecializationKind() == TSK_Undeclared) {
    // Since the only prior variable template specialization with these
    // arguments was referenced but not declared,  reuse that
    // declaration node as our own, updating its source location and
    // the list of outer template parameters to reflect our new declaration.
    Specialization = PrevDecl;
    Specialization->setLocation(TemplateNameLoc);
    PrevDecl = nullptr;
  } else if (IsPartialSpecialization) {
    // Create a new class template partial specialization declaration node.
    VarTemplatePartialSpecializationDecl *PrevPartial =
        cast_or_null<VarTemplatePartialSpecializationDecl>(PrevDecl);
    VarTemplatePartialSpecializationDecl *Partial =
        VarTemplatePartialSpecializationDecl::Create(
            Context, VarTemplate->getDeclContext(), TemplateKWLoc,
            TemplateNameLoc, TemplateParams, VarTemplate, DI->getType(), DI, SC,
            Converted.data(), Converted.size(), TemplateArgs);

    if (!PrevPartial)
      VarTemplate->AddPartialSpecialization(Partial, InsertPos);
    Specialization = Partial;

    // If we are providing an explicit specialization of a member variable
    // template specialization, make a note of that.
    if (PrevPartial && PrevPartial->getInstantiatedFromMember())
      PrevPartial->setMemberSpecialization();

    // Check that all of the template parameters of the variable template
    // partial specialization are deducible from the template
    // arguments. If not, this variable template partial specialization
    // will never be used.
    llvm::SmallBitVector DeducibleParams(TemplateParams->size());
    MarkUsedTemplateParameters(Partial->getTemplateArgs(), true,
                               TemplateParams->getDepth(), DeducibleParams);

    if (!DeducibleParams.all()) {
      unsigned NumNonDeducible =
          DeducibleParams.size() - DeducibleParams.count();
      Diag(TemplateNameLoc, diag::warn_partial_specs_not_deducible)
        << /*variable template*/ 1 << (NumNonDeducible > 1)
        << SourceRange(TemplateNameLoc, RAngleLoc);
      for (unsigned I = 0, N = DeducibleParams.size(); I != N; ++I) {
        if (!DeducibleParams[I]) {
          NamedDecl *Param = cast<NamedDecl>(TemplateParams->getParam(I));
          if (Param->getDeclName())
            Diag(Param->getLocation(), diag::note_partial_spec_unused_parameter)
                << Param->getDeclName();
          else
            Diag(Param->getLocation(), diag::note_partial_spec_unused_parameter)
                << "(anonymous)";
        }
      }
    }
  } else {
    // Create a new class template specialization declaration node for
    // this explicit specialization or friend declaration.
    Specialization = VarTemplateSpecializationDecl::Create(
        Context, VarTemplate->getDeclContext(), TemplateKWLoc, TemplateNameLoc,
        VarTemplate, DI->getType(), DI, SC, Converted.data(), Converted.size());
    Specialization->setTemplateArgsInfo(TemplateArgs);

    if (!PrevDecl)
      VarTemplate->AddSpecialization(Specialization, InsertPos);
  }

  // C++ [temp.expl.spec]p6:
  //   If a template, a member template or the member of a class template is
  //   explicitly specialized then that specialization shall be declared
  //   before the first use of that specialization that would cause an implicit
  //   instantiation to take place, in every translation unit in which such a
  //   use occurs; no diagnostic is required.
  if (PrevDecl && PrevDecl->getPointOfInstantiation().isValid()) {
    bool Okay = false;
    for (Decl *Prev = PrevDecl; Prev; Prev = Prev->getPreviousDecl()) {
      // Is there any previous explicit specialization declaration?
      if (getTemplateSpecializationKind(Prev) == TSK_ExplicitSpecialization) {
        Okay = true;
        break;
      }
    }

    if (!Okay) {
      SourceRange Range(TemplateNameLoc, RAngleLoc);
      Diag(TemplateNameLoc, diag::err_specialization_after_instantiation)
          << Name << Range;

      Diag(PrevDecl->getPointOfInstantiation(),
           diag::note_instantiation_required_here)
          << (PrevDecl->getTemplateSpecializationKind() !=
              TSK_ImplicitInstantiation);
      return true;
    }
  }

  Specialization->setTemplateKeywordLoc(TemplateKWLoc);
  Specialization->setLexicalDeclContext(CurContext);

  // Add the specialization into its lexical context, so that it can
  // be seen when iterating through the list of declarations in that
  // context. However, specializations are not found by name lookup.
  CurContext->addDecl(Specialization);

  // Note that this is an explicit specialization.
  Specialization->setSpecializationKind(TSK_ExplicitSpecialization);

  if (PrevDecl) {
    // Check that this isn't a redefinition of this specialization,
    // merging with previous declarations.
    LookupResult PrevSpec(*this, GetNameForDeclarator(D), LookupOrdinaryName,
                          ForRedeclaration);
    PrevSpec.addDecl(PrevDecl);
    D.setRedeclaration(CheckVariableDeclaration(Specialization, PrevSpec));
  } else if (Specialization->isStaticDataMember() &&
             Specialization->isOutOfLine()) {
    Specialization->setAccess(VarTemplate->getAccess());
  }

  // Link instantiations of static data members back to the template from
  // which they were instantiated.
  if (Specialization->isStaticDataMember())
    Specialization->setInstantiationOfStaticDataMember(
        VarTemplate->getTemplatedDecl(),
        Specialization->getSpecializationKind());

  return Specialization;
}

namespace {
/// \brief A partial specialization whose template arguments have matched
/// a given template-id.
struct PartialSpecMatchResult {
  VarTemplatePartialSpecializationDecl *Partial;
  TemplateArgumentList *Args;
};
}

DeclResult
Sema::CheckVarTemplateId(VarTemplateDecl *Template, SourceLocation TemplateLoc,
                         SourceLocation TemplateNameLoc,
                         const TemplateArgumentListInfo &TemplateArgs) {
  assert(Template && "A variable template id without template?");

  // Check that the template argument list is well-formed for this template.
  SmallVector<TemplateArgument, 4> Converted;
  if (CheckTemplateArgumentList(
          Template, TemplateNameLoc,
          const_cast<TemplateArgumentListInfo &>(TemplateArgs), false,
          Converted))
    return true;

  // Find the variable template specialization declaration that
  // corresponds to these arguments.
  void *InsertPos = nullptr;
  if (VarTemplateSpecializationDecl *Spec = Template->findSpecialization(
          Converted, InsertPos))
    // If we already have a variable template specialization, return it.
    return Spec;

  // This is the first time we have referenced this variable template
  // specialization. Create the canonical declaration and add it to
  // the set of specializations, based on the closest partial specialization
  // that it represents. That is,
  VarDecl *InstantiationPattern = Template->getTemplatedDecl();
  TemplateArgumentList TemplateArgList(TemplateArgumentList::OnStack,
                                       Converted.data(), Converted.size());
  TemplateArgumentList *InstantiationArgs = &TemplateArgList;
  bool AmbiguousPartialSpec = false;
  typedef PartialSpecMatchResult MatchResult;
  SmallVector<MatchResult, 4> Matched;
  SourceLocation PointOfInstantiation = TemplateNameLoc;
  TemplateSpecCandidateSet FailedCandidates(PointOfInstantiation);

  // 1. Attempt to find the closest partial specialization that this
  // specializes, if any.
  // If any of the template arguments is dependent, then this is probably
  // a placeholder for an incomplete declarative context; which must be
  // complete by instantiation time. Thus, do not search through the partial
  // specializations yet.
  // TODO: Unify with InstantiateClassTemplateSpecialization()?
  //       Perhaps better after unification of DeduceTemplateArguments() and
  //       getMoreSpecializedPartialSpecialization().
  bool InstantiationDependent = false;
  if (!TemplateSpecializationType::anyDependentTemplateArguments(
          TemplateArgs, InstantiationDependent)) {

    SmallVector<VarTemplatePartialSpecializationDecl *, 4> PartialSpecs;
    Template->getPartialSpecializations(PartialSpecs);

    for (unsigned I = 0, N = PartialSpecs.size(); I != N; ++I) {
      VarTemplatePartialSpecializationDecl *Partial = PartialSpecs[I];
      TemplateDeductionInfo Info(FailedCandidates.getLocation());

      if (TemplateDeductionResult Result =
              DeduceTemplateArguments(Partial, TemplateArgList, Info)) {
        // Store the failed-deduction information for use in diagnostics, later.
        // TODO: Actually use the failed-deduction info?
        FailedCandidates.addCandidate()
            .set(Partial, MakeDeductionFailureInfo(Context, Result, Info));
        (void)Result;
      } else {
        Matched.push_back(PartialSpecMatchResult());
        Matched.back().Partial = Partial;
        Matched.back().Args = Info.take();
      }
    }

    if (Matched.size() >= 1) {
      SmallVector<MatchResult, 4>::iterator Best = Matched.begin();
      if (Matched.size() == 1) {
        //   -- If exactly one matching specialization is found, the
        //      instantiation is generated from that specialization.
        // We don't need to do anything for this.
      } else {
        //   -- If more than one matching specialization is found, the
        //      partial order rules (14.5.4.2) are used to determine
        //      whether one of the specializations is more specialized
        //      than the others. If none of the specializations is more
        //      specialized than all of the other matching
        //      specializations, then the use of the variable template is
        //      ambiguous and the program is ill-formed.
        for (SmallVector<MatchResult, 4>::iterator P = Best + 1,
                                                   PEnd = Matched.end();
             P != PEnd; ++P) {
          if (getMoreSpecializedPartialSpecialization(P->Partial, Best->Partial,
                                                      PointOfInstantiation) ==
              P->Partial)
            Best = P;
        }

        // Determine if the best partial specialization is more specialized than
        // the others.
        for (SmallVector<MatchResult, 4>::iterator P = Matched.begin(),
                                                   PEnd = Matched.end();
             P != PEnd; ++P) {
          if (P != Best && getMoreSpecializedPartialSpecialization(
                               P->Partial, Best->Partial,
                               PointOfInstantiation) != Best->Partial) {
            AmbiguousPartialSpec = true;
            break;
          }
        }
      }

      // Instantiate using the best variable template partial specialization.
      InstantiationPattern = Best->Partial;
      InstantiationArgs = Best->Args;
    } else {
      //   -- If no match is found, the instantiation is generated
      //      from the primary template.
      // InstantiationPattern = Template->getTemplatedDecl();
    }
  }

  // 2. Create the canonical declaration.
  // Note that we do not instantiate the variable just yet, since
  // instantiation is handled in DoMarkVarDeclReferenced().
  // FIXME: LateAttrs et al.?
  VarTemplateSpecializationDecl *Decl = BuildVarTemplateInstantiation(
      Template, InstantiationPattern, *InstantiationArgs, TemplateArgs,
      Converted, TemplateNameLoc, InsertPos /*, LateAttrs, StartingScope*/);
  if (!Decl)
    return true;

  if (AmbiguousPartialSpec) {
    // Partial ordering did not produce a clear winner. Complain.
    Decl->setInvalidDecl();
    Diag(PointOfInstantiation, diag::err_partial_spec_ordering_ambiguous)
        << Decl;

    // Print the matching partial specializations.
    for (SmallVector<MatchResult, 4>::iterator P = Matched.begin(),
                                               PEnd = Matched.end();
         P != PEnd; ++P)
      Diag(P->Partial->getLocation(), diag::note_partial_spec_match)
          << getTemplateArgumentBindingsText(
                 P->Partial->getTemplateParameters(), *P->Args);
    return true;
  }

  if (VarTemplatePartialSpecializationDecl *D =
          dyn_cast<VarTemplatePartialSpecializationDecl>(InstantiationPattern))
    Decl->setInstantiationOf(D, InstantiationArgs);

  assert(Decl && "No variable template specialization?");
  return Decl;
}

ExprResult
Sema::CheckVarTemplateId(const CXXScopeSpec &SS,
                         const DeclarationNameInfo &NameInfo,
                         VarTemplateDecl *Template, SourceLocation TemplateLoc,
                         const TemplateArgumentListInfo *TemplateArgs) {

  DeclResult Decl = CheckVarTemplateId(Template, TemplateLoc, NameInfo.getLoc(),
                                       *TemplateArgs);
  if (Decl.isInvalid())
    return ExprError();

  VarDecl *Var = cast<VarDecl>(Decl.get());
  if (!Var->getTemplateSpecializationKind())
    Var->setTemplateSpecializationKind(TSK_ImplicitInstantiation,
                                       NameInfo.getLoc());

  // Build an ordinary singleton decl ref.
  return BuildDeclarationNameExpr(SS, NameInfo, Var,
                                  /*FoundD=*/nullptr, TemplateArgs);
}

ExprResult Sema::BuildTemplateIdExpr(const CXXScopeSpec &SS,
                                     SourceLocation TemplateKWLoc,
                                     LookupResult &R,
                                     bool RequiresADL,
                                 const TemplateArgumentListInfo *TemplateArgs) {
  // FIXME: Can we do any checking at this point? I guess we could check the
  // template arguments that we have against the template name, if the template
  // name refers to a single template. That's not a terribly common case,
  // though.
  // foo<int> could identify a single function unambiguously
  // This approach does NOT work, since f<int>(1);
  // gets resolved prior to resorting to overload resolution
  // i.e., template<class T> void f(double);
  //       vs template<class T, class U> void f(U);

  // These should be filtered out by our callers.
  assert(!R.empty() && "empty lookup results when building templateid");
  assert(!R.isAmbiguous() && "ambiguous lookup when building templateid");

  // In C++1y, check variable template ids.
  bool InstantiationDependent;
  if (R.getAsSingle<VarTemplateDecl>() &&
      !TemplateSpecializationType::anyDependentTemplateArguments(
           *TemplateArgs, InstantiationDependent)) {
    return CheckVarTemplateId(SS, R.getLookupNameInfo(),
                              R.getAsSingle<VarTemplateDecl>(),
                              TemplateKWLoc, TemplateArgs);
  }

  // We don't want lookup warnings at this point.
  R.suppressDiagnostics();

  UnresolvedLookupExpr *ULE
    = UnresolvedLookupExpr::Create(Context, R.getNamingClass(),
                                   SS.getWithLocInContext(Context),
                                   TemplateKWLoc,
                                   R.getLookupNameInfo(),
                                   RequiresADL, TemplateArgs,
                                   R.begin(), R.end());

  return ULE;
}

// We actually only call this from template instantiation.
ExprResult
Sema::BuildQualifiedTemplateIdExpr(CXXScopeSpec &SS,
                                   SourceLocation TemplateKWLoc,
                                   const DeclarationNameInfo &NameInfo,
                             const TemplateArgumentListInfo *TemplateArgs) {

  assert(TemplateArgs || TemplateKWLoc.isValid());
  DeclContext *DC;
  if (!(DC = computeDeclContext(SS, false)) ||
      DC->isDependentContext() ||
      RequireCompleteDeclContext(SS, DC))
    return BuildDependentDeclRefExpr(SS, TemplateKWLoc, NameInfo, TemplateArgs);

  bool MemberOfUnknownSpecialization;
  LookupResult R(*this, NameInfo, LookupOrdinaryName);
  LookupTemplateName(R, (Scope*)nullptr, SS, QualType(), /*Entering*/ false,
                     MemberOfUnknownSpecialization);

  if (R.isAmbiguous())
    return ExprError();

  if (R.empty()) {
    Diag(NameInfo.getLoc(), diag::err_template_kw_refers_to_non_template)
      << NameInfo.getName() << SS.getRange();
    return ExprError();
  }

  if (ClassTemplateDecl *Temp = R.getAsSingle<ClassTemplateDecl>()) {
    Diag(NameInfo.getLoc(), diag::err_template_kw_refers_to_class_template)
      << SS.getScopeRep()
      << NameInfo.getName().getAsString() << SS.getRange();
    Diag(Temp->getLocation(), diag::note_referenced_class_template);
    return ExprError();
  }

  return BuildTemplateIdExpr(SS, TemplateKWLoc, R, /*ADL*/ false, TemplateArgs);
}

/// \brief Form a dependent template name.
///
/// This action forms a dependent template name given the template
/// name and its (presumably dependent) scope specifier. For
/// example, given "MetaFun::template apply", the scope specifier \p
/// SS will be "MetaFun::", \p TemplateKWLoc contains the location
/// of the "template" keyword, and "apply" is the \p Name.
TemplateNameKind Sema::ActOnDependentTemplateName(Scope *S,
                                                  CXXScopeSpec &SS,
                                                  SourceLocation TemplateKWLoc,
                                                  UnqualifiedId &Name,
                                                  ParsedType ObjectType,
                                                  bool EnteringContext,
                                                  TemplateTy &Result) {
  if (TemplateKWLoc.isValid() && S && !S->getTemplateParamParent())
    Diag(TemplateKWLoc,
         getLangOpts().CPlusPlus11 ?
           diag::warn_cxx98_compat_template_outside_of_template :
           diag::ext_template_outside_of_template)
      << FixItHint::CreateRemoval(TemplateKWLoc);

  DeclContext *LookupCtx = nullptr;
  if (SS.isSet())
    LookupCtx = computeDeclContext(SS, EnteringContext);
  if (!LookupCtx && ObjectType)
    LookupCtx = computeDeclContext(ObjectType.get());
  if (LookupCtx) {
    // C++0x [temp.names]p5:
    //   If a name prefixed by the keyword template is not the name of
    //   a template, the program is ill-formed. [Note: the keyword
    //   template may not be applied to non-template members of class
    //   templates. -end note ] [ Note: as is the case with the
    //   typename prefix, the template prefix is allowed in cases
    //   where it is not strictly necessary; i.e., when the
    //   nested-name-specifier or the expression on the left of the ->
    //   or . is not dependent on a template-parameter, or the use
    //   does not appear in the scope of a template. -end note]
    //
    // Note: C++03 was more strict here, because it banned the use of
    // the "template" keyword prior to a template-name that was not a
    // dependent name. C++ DR468 relaxed this requirement (the
    // "template" keyword is now permitted). We follow the C++0x
    // rules, even in C++03 mode with a warning, retroactively applying the DR.
    bool MemberOfUnknownSpecialization;
    TemplateNameKind TNK = isTemplateName(S, SS, TemplateKWLoc.isValid(), Name,
                                          ObjectType, EnteringContext, Result,
                                          MemberOfUnknownSpecialization);
    if (TNK == TNK_Non_template && LookupCtx->isDependentContext() &&
        isa<CXXRecordDecl>(LookupCtx) &&
        (!cast<CXXRecordDecl>(LookupCtx)->hasDefinition() ||
         cast<CXXRecordDecl>(LookupCtx)->hasAnyDependentBases())) {
      // This is a dependent template. Handle it below.
    } else if (TNK == TNK_Non_template) {
      Diag(Name.getLocStart(),
           diag::err_template_kw_refers_to_non_template)
        << GetNameFromUnqualifiedId(Name).getName()
        << Name.getSourceRange()
        << TemplateKWLoc;
      return TNK_Non_template;
    } else {
      // We found something; return it.
      return TNK;
    }
  }

  NestedNameSpecifier *Qualifier = SS.getScopeRep();

  switch (Name.getKind()) {
  case UnqualifiedId::IK_Identifier:
    Result = TemplateTy::make(Context.getDependentTemplateName(Qualifier,
                                                              Name.Identifier));
    return TNK_Dependent_template_name;

  case UnqualifiedId::IK_OperatorFunctionId:
    Result = TemplateTy::make(Context.getDependentTemplateName(Qualifier,
                                             Name.OperatorFunctionId.Operator));
    return TNK_Function_template;

  case UnqualifiedId::IK_LiteralOperatorId:
    llvm_unreachable("literal operator id cannot have a dependent scope");

  default:
    break;
  }

  Diag(Name.getLocStart(),
       diag::err_template_kw_refers_to_non_template)
    << GetNameFromUnqualifiedId(Name).getName()
    << Name.getSourceRange()
    << TemplateKWLoc;
  return TNK_Non_template;
}

bool Sema::CheckTemplateTypeArgument(TemplateTypeParmDecl *Param,
                                     TemplateArgumentLoc &AL,
                          SmallVectorImpl<TemplateArgument> &Converted) {
  const TemplateArgument &Arg = AL.getArgument();
  QualType ArgType;
  TypeSourceInfo *TSI = nullptr;

  // Check template type parameter.
  switch(Arg.getKind()) {
  case TemplateArgument::Type:
    // C++ [temp.arg.type]p1:
    //   A template-argument for a template-parameter which is a
    //   type shall be a type-id.
    ArgType = Arg.getAsType();
    TSI = AL.getTypeSourceInfo();
    break;
  case TemplateArgument::Template: {
    // We have a template type parameter but the template argument
    // is a template without any arguments.
    SourceRange SR = AL.getSourceRange();
    TemplateName Name = Arg.getAsTemplate();

    // HLSL Change Starts
    // HLSL allows omiting empty template argument lists
    TemplateDecl *Decl = Name.getAsTemplateDecl();
    if (getLangOpts().HLSL && Decl) {
      if (TemplateDecl *TD = dyn_cast<TemplateDecl>(Decl)) {
        ArgType = getHLSLDefaultSpecialization(TD);
        if (!ArgType.isNull()) {
          CXXScopeSpec SS;
          TypeLocBuilder TLB;
          TemplateSpecializationTypeLoc TL =
              TLB.push<TemplateSpecializationTypeLoc>(ArgType);
          TL.setTemplateKeywordLoc(SourceLocation());
          TL.setTemplateNameLoc(SR.getBegin());
          TL.setLAngleLoc(SR.getEnd());
          TL.setRAngleLoc(SR.getEnd());
          TSI = TLB.getTypeSourceInfo(Context, ArgType);
        }
      }
    }
    if (ArgType.isNull()) {
      Diag(SR.getBegin(), diag::err_template_missing_args) << Name << SR;
      // HLSL Change - ellide location notes for built-ins
      if (Decl && Decl->getLocation().isValid()) {
        Diag(Decl->getLocation(), diag::note_template_decl_here);
      }
      return true;
    }
    break;
    // HLSL Change Ends
  }
  case TemplateArgument::Expression: {
    // We have a template type parameter but the template argument is an
    // expression; see if maybe it is missing the "typename" keyword.
    CXXScopeSpec SS;
    DeclarationNameInfo NameInfo;

    if (DeclRefExpr *ArgExpr = dyn_cast<DeclRefExpr>(Arg.getAsExpr())) {
      SS.Adopt(ArgExpr->getQualifierLoc());
      NameInfo = ArgExpr->getNameInfo();
    } else if (DependentScopeDeclRefExpr *ArgExpr =
               dyn_cast<DependentScopeDeclRefExpr>(Arg.getAsExpr())) {
      SS.Adopt(ArgExpr->getQualifierLoc());
      NameInfo = ArgExpr->getNameInfo();
    } else if (CXXDependentScopeMemberExpr *ArgExpr =
               dyn_cast<CXXDependentScopeMemberExpr>(Arg.getAsExpr())) {
      if (ArgExpr->isImplicitAccess()) {
        SS.Adopt(ArgExpr->getQualifierLoc());
        NameInfo = ArgExpr->getMemberNameInfo();
      }
    }

    if (auto *II = NameInfo.getName().getAsIdentifierInfo()) {
      LookupResult Result(*this, NameInfo, LookupOrdinaryName);
      LookupParsedName(Result, CurScope, &SS);

      if (Result.getAsSingle<TypeDecl>() ||
          Result.getResultKind() ==
              LookupResult::NotFoundInCurrentInstantiation) {
        // Suggest that the user add 'typename' before the NNS.
        SourceLocation Loc = AL.getSourceRange().getBegin();
        Diag(Loc, getLangOpts().MSVCCompat
                      ? diag::ext_ms_template_type_arg_missing_typename
                      : diag::err_template_arg_must_be_type_suggest)
            << FixItHint::CreateInsertion(Loc, "typename ");
        Diag(Param->getLocation(), diag::note_template_param_here);

        // Recover by synthesizing a type using the location information that we
        // already have.
        ArgType =
            Context.getDependentNameType(ETK_Typename, SS.getScopeRep(), II);
        TypeLocBuilder TLB;
        DependentNameTypeLoc TL = TLB.push<DependentNameTypeLoc>(ArgType);
        TL.setElaboratedKeywordLoc(SourceLocation(/*synthesized*/));
        TL.setQualifierLoc(SS.getWithLocInContext(Context));
        TL.setNameLoc(NameInfo.getLoc());
        TSI = TLB.getTypeSourceInfo(Context, ArgType);

        // Overwrite our input TemplateArgumentLoc so that we can recover
        // properly.
        AL = TemplateArgumentLoc(TemplateArgument(ArgType),
                                 TemplateArgumentLocInfo(TSI));

        break;
      }
    }
    // fallthrough
    LLVM_FALLTHROUGH; // HLSL Change
  }
  default: {
    // We have a template type parameter but the template argument
    // is not a type.
    SourceRange SR = AL.getSourceRange();
    Diag(SR.getBegin(), diag::err_template_arg_must_be_type) << SR;
    Diag(Param->getLocation(), diag::note_template_param_here);

    return true;
  }
  }

  if (CheckTemplateArgument(Param, TSI))
    return true;

  // Add the converted template type argument.
  ArgType = Context.getCanonicalType(ArgType);
  
  // Objective-C ARC:
  //   If an explicitly-specified template argument type is a lifetime type
  //   with no lifetime qualifier, the __strong lifetime qualifier is inferred.
  if (getLangOpts().ObjCAutoRefCount &&
      ArgType->isObjCLifetimeType() &&
      !ArgType.getObjCLifetime()) {
    Qualifiers Qs;
    Qs.setObjCLifetime(Qualifiers::OCL_Strong);
    ArgType = Context.getQualifiedType(ArgType, Qs);
  }
  
  Converted.push_back(TemplateArgument(ArgType));
  return false;
}

/// \brief Substitute template arguments into the default template argument for
/// the given template type parameter.
///
/// \param SemaRef the semantic analysis object for which we are performing
/// the substitution.
///
/// \param Template the template that we are synthesizing template arguments
/// for.
///
/// \param TemplateLoc the location of the template name that started the
/// template-id we are checking.
///
/// \param RAngleLoc the location of the right angle bracket ('>') that
/// terminates the template-id.
///
/// \param Param the template template parameter whose default we are
/// substituting into.
///
/// \param Converted the list of template arguments provided for template
/// parameters that precede \p Param in the template parameter list.
/// \returns the substituted template argument, or NULL if an error occurred.
static TypeSourceInfo *
SubstDefaultTemplateArgument(Sema &SemaRef,
                             TemplateDecl *Template,
                             SourceLocation TemplateLoc,
                             SourceLocation RAngleLoc,
                             TemplateTypeParmDecl *Param,
                         SmallVectorImpl<TemplateArgument> &Converted) {
  TypeSourceInfo *ArgType = Param->getDefaultArgumentInfo();

  // If the argument type is dependent, instantiate it now based
  // on the previously-computed template arguments.
  if (ArgType->getType()->isDependentType()) {
    Sema::InstantiatingTemplate Inst(SemaRef, TemplateLoc,
                                     Template, Converted,
                                     SourceRange(TemplateLoc, RAngleLoc));
    if (Inst.isInvalid())
      return nullptr;

    TemplateArgumentList TemplateArgs(TemplateArgumentList::OnStack,
                                      Converted.data(), Converted.size());

    // Only substitute for the innermost template argument list.
    MultiLevelTemplateArgumentList TemplateArgLists;
    TemplateArgLists.addOuterTemplateArguments(&TemplateArgs);
    for (unsigned i = 0, e = Param->getDepth(); i != e; ++i)
      TemplateArgLists.addOuterTemplateArguments(None);

    Sema::ContextRAII SavedContext(SemaRef, Template->getDeclContext());
    ArgType =
        SemaRef.SubstType(ArgType, TemplateArgLists,
                          Param->getDefaultArgumentLoc(), Param->getDeclName());
  }

  return ArgType;
}

/// \brief Substitute template arguments into the default template argument for
/// the given non-type template parameter.
///
/// \param SemaRef the semantic analysis object for which we are performing
/// the substitution.
///
/// \param Template the template that we are synthesizing template arguments
/// for.
///
/// \param TemplateLoc the location of the template name that started the
/// template-id we are checking.
///
/// \param RAngleLoc the location of the right angle bracket ('>') that
/// terminates the template-id.
///
/// \param Param the non-type template parameter whose default we are
/// substituting into.
///
/// \param Converted the list of template arguments provided for template
/// parameters that precede \p Param in the template parameter list.
///
/// \returns the substituted template argument, or NULL if an error occurred.
static ExprResult
SubstDefaultTemplateArgument(Sema &SemaRef,
                             TemplateDecl *Template,
                             SourceLocation TemplateLoc,
                             SourceLocation RAngleLoc,
                             NonTypeTemplateParmDecl *Param,
                        SmallVectorImpl<TemplateArgument> &Converted) {
  Sema::InstantiatingTemplate Inst(SemaRef, TemplateLoc,
                                   Template, Converted,
                                   SourceRange(TemplateLoc, RAngleLoc));
  if (Inst.isInvalid())
    return ExprError();

  TemplateArgumentList TemplateArgs(TemplateArgumentList::OnStack,
                                    Converted.data(), Converted.size());

  // Only substitute for the innermost template argument list.
  MultiLevelTemplateArgumentList TemplateArgLists;
  TemplateArgLists.addOuterTemplateArguments(&TemplateArgs);
  for (unsigned i = 0, e = Param->getDepth(); i != e; ++i)
    TemplateArgLists.addOuterTemplateArguments(None);

  Sema::ContextRAII SavedContext(SemaRef, Template->getDeclContext());
  EnterExpressionEvaluationContext Unevaluated(SemaRef, Sema::Unevaluated);
  return SemaRef.SubstExpr(Param->getDefaultArgument(), TemplateArgLists);
}

/// \brief Substitute template arguments into the default template argument for
/// the given template template parameter.
///
/// \param SemaRef the semantic analysis object for which we are performing
/// the substitution.
///
/// \param Template the template that we are synthesizing template arguments
/// for.
///
/// \param TemplateLoc the location of the template name that started the
/// template-id we are checking.
///
/// \param RAngleLoc the location of the right angle bracket ('>') that
/// terminates the template-id.
///
/// \param Param the template template parameter whose default we are
/// substituting into.
///
/// \param Converted the list of template arguments provided for template
/// parameters that precede \p Param in the template parameter list.
///
/// \param QualifierLoc Will be set to the nested-name-specifier (with 
/// source-location information) that precedes the template name.
///
/// \returns the substituted template argument, or NULL if an error occurred.
static TemplateName
SubstDefaultTemplateArgument(Sema &SemaRef,
                             TemplateDecl *Template,
                             SourceLocation TemplateLoc,
                             SourceLocation RAngleLoc,
                             TemplateTemplateParmDecl *Param,
                       SmallVectorImpl<TemplateArgument> &Converted,
                             NestedNameSpecifierLoc &QualifierLoc) {
  Sema::InstantiatingTemplate Inst(SemaRef, TemplateLoc, Template, Converted,
                                   SourceRange(TemplateLoc, RAngleLoc));
  if (Inst.isInvalid())
    return TemplateName();

  TemplateArgumentList TemplateArgs(TemplateArgumentList::OnStack,
                                    Converted.data(), Converted.size());

  // Only substitute for the innermost template argument list.
  MultiLevelTemplateArgumentList TemplateArgLists;
  TemplateArgLists.addOuterTemplateArguments(&TemplateArgs);
  for (unsigned i = 0, e = Param->getDepth(); i != e; ++i)
    TemplateArgLists.addOuterTemplateArguments(None);

  Sema::ContextRAII SavedContext(SemaRef, Template->getDeclContext());
  // Substitute into the nested-name-specifier first,
  QualifierLoc = Param->getDefaultArgument().getTemplateQualifierLoc();
  if (QualifierLoc) {
    QualifierLoc =
        SemaRef.SubstNestedNameSpecifierLoc(QualifierLoc, TemplateArgLists);
    if (!QualifierLoc)
      return TemplateName();
  }

  return SemaRef.SubstTemplateName(
             QualifierLoc,
             Param->getDefaultArgument().getArgument().getAsTemplate(),
             Param->getDefaultArgument().getTemplateNameLoc(),
             TemplateArgLists);
}

/// \brief If the given template parameter has a default template
/// argument, substitute into that default template argument and
/// return the corresponding template argument.
TemplateArgumentLoc
Sema::SubstDefaultTemplateArgumentIfAvailable(TemplateDecl *Template,
                                              SourceLocation TemplateLoc,
                                              SourceLocation RAngleLoc,
                                              Decl *Param,
                                              SmallVectorImpl<TemplateArgument>
                                                &Converted,
                                              bool &HasDefaultArg) {
  HasDefaultArg = false;

  if (TemplateTypeParmDecl *TypeParm = dyn_cast<TemplateTypeParmDecl>(Param)) {
    if (!hasVisibleDefaultArgument(TypeParm))
      return TemplateArgumentLoc();

    HasDefaultArg = true;
    TypeSourceInfo *DI = SubstDefaultTemplateArgument(*this, Template,
                                                      TemplateLoc,
                                                      RAngleLoc,
                                                      TypeParm,
                                                      Converted);
    if (DI)
      return TemplateArgumentLoc(TemplateArgument(DI->getType()), DI);

    return TemplateArgumentLoc();
  }

  if (NonTypeTemplateParmDecl *NonTypeParm
        = dyn_cast<NonTypeTemplateParmDecl>(Param)) {
    if (!hasVisibleDefaultArgument(NonTypeParm))
      return TemplateArgumentLoc();

    HasDefaultArg = true;
    ExprResult Arg = SubstDefaultTemplateArgument(*this, Template,
                                                  TemplateLoc,
                                                  RAngleLoc,
                                                  NonTypeParm,
                                                  Converted);
    if (Arg.isInvalid())
      return TemplateArgumentLoc();

    Expr *ArgE = Arg.getAs<Expr>();
    return TemplateArgumentLoc(TemplateArgument(ArgE), ArgE);
  }

  TemplateTemplateParmDecl *TempTempParm
    = cast<TemplateTemplateParmDecl>(Param);
  if (!hasVisibleDefaultArgument(TempTempParm))
    return TemplateArgumentLoc();

  HasDefaultArg = true;
  NestedNameSpecifierLoc QualifierLoc;
  TemplateName TName = SubstDefaultTemplateArgument(*this, Template,
                                                    TemplateLoc,
                                                    RAngleLoc,
                                                    TempTempParm,
                                                    Converted,
                                                    QualifierLoc);
  if (TName.isNull())
    return TemplateArgumentLoc();

  return TemplateArgumentLoc(TemplateArgument(TName),
                TempTempParm->getDefaultArgument().getTemplateQualifierLoc(),
                TempTempParm->getDefaultArgument().getTemplateNameLoc());
}

/// \brief Check that the given template argument corresponds to the given
/// template parameter.
///
/// \param Param The template parameter against which the argument will be
/// checked.
///
/// \param Arg The template argument, which may be updated due to conversions.
///
/// \param Template The template in which the template argument resides.
///
/// \param TemplateLoc The location of the template name for the template
/// whose argument list we're matching.
///
/// \param RAngleLoc The location of the right angle bracket ('>') that closes
/// the template argument list.
///
/// \param ArgumentPackIndex The index into the argument pack where this
/// argument will be placed. Only valid if the parameter is a parameter pack.
///
/// \param Converted The checked, converted argument will be added to the
/// end of this small vector.
///
/// \param CTAK Describes how we arrived at this particular template argument:
/// explicitly written, deduced, etc.
///
/// \returns true on error, false otherwise.
bool Sema::CheckTemplateArgument(NamedDecl *Param,
                                 TemplateArgumentLoc &Arg,
                                 NamedDecl *Template,
                                 SourceLocation TemplateLoc,
                                 SourceLocation RAngleLoc,
                                 unsigned ArgumentPackIndex,
                            SmallVectorImpl<TemplateArgument> &Converted,
                                 CheckTemplateArgumentKind CTAK) {
  // Check template type parameters.
  if (TemplateTypeParmDecl *TTP = dyn_cast<TemplateTypeParmDecl>(Param))
    return CheckTemplateTypeArgument(TTP, Arg, Converted);

  // Check non-type template parameters.
  if (NonTypeTemplateParmDecl *NTTP =dyn_cast<NonTypeTemplateParmDecl>(Param)) {
    // Do substitution on the type of the non-type template parameter
    // with the template arguments we've seen thus far.  But if the
    // template has a dependent context then we cannot substitute yet.
    QualType NTTPType = NTTP->getType();
    if (NTTP->isParameterPack() && NTTP->isExpandedParameterPack())
      NTTPType = NTTP->getExpansionType(ArgumentPackIndex);

    if (NTTPType->isDependentType() &&
        !isa<TemplateTemplateParmDecl>(Template) &&
        !Template->getDeclContext()->isDependentContext()) {
      // Do substitution on the type of the non-type template parameter.
      InstantiatingTemplate Inst(*this, TemplateLoc, Template,
                                 NTTP, Converted,
                                 SourceRange(TemplateLoc, RAngleLoc));
      if (Inst.isInvalid())
        return true;

      TemplateArgumentList TemplateArgs(TemplateArgumentList::OnStack,
                                        Converted.data(), Converted.size());
      NTTPType = SubstType(NTTPType,
                           MultiLevelTemplateArgumentList(TemplateArgs),
                           NTTP->getLocation(),
                           NTTP->getDeclName());
      // If that worked, check the non-type template parameter type
      // for validity.
      if (!NTTPType.isNull())
        NTTPType = CheckNonTypeTemplateParameterType(NTTPType,
                                                     NTTP->getLocation());
      if (NTTPType.isNull())
        return true;
    }

    switch (Arg.getArgument().getKind()) {
    case TemplateArgument::Null:
      llvm_unreachable("Should never see a NULL template argument here");

    case TemplateArgument::Expression: {
      TemplateArgument Result;
      ExprResult Res =
        CheckTemplateArgument(NTTP, NTTPType, Arg.getArgument().getAsExpr(),
                              Result, CTAK);
      if (Res.isInvalid())
        return true;

      // If the resulting expression is new, then use it in place of the
      // old expression in the template argument.
      if (Res.get() != Arg.getArgument().getAsExpr()) {
        TemplateArgument TA(Res.get());
        Arg = TemplateArgumentLoc(TA, Res.get());
      }

      Converted.push_back(Result);
      break;
    }

    case TemplateArgument::Declaration:
    case TemplateArgument::Integral:
    case TemplateArgument::NullPtr:
      // We've already checked this template argument, so just copy
      // it to the list of converted arguments.
      Converted.push_back(Arg.getArgument());
      break;

    case TemplateArgument::Template:
    case TemplateArgument::TemplateExpansion:
      // We were given a template template argument. It may not be ill-formed;
      // see below.
      if (DependentTemplateName *DTN
            = Arg.getArgument().getAsTemplateOrTemplatePattern()
                                              .getAsDependentTemplateName()) {
        // We have a template argument such as \c T::template X, which we
        // parsed as a template template argument. However, since we now
        // know that we need a non-type template argument, convert this
        // template name into an expression.

        DeclarationNameInfo NameInfo(DTN->getIdentifier(),
                                     Arg.getTemplateNameLoc());

        CXXScopeSpec SS;
        SS.Adopt(Arg.getTemplateQualifierLoc());
        // FIXME: the template-template arg was a DependentTemplateName,
        // so it was provided with a template keyword. However, its source
        // location is not stored in the template argument structure.
        SourceLocation TemplateKWLoc;
        ExprResult E = DependentScopeDeclRefExpr::Create(
            Context, SS.getWithLocInContext(Context), TemplateKWLoc, NameInfo,
            nullptr);

        // If we parsed the template argument as a pack expansion, create a
        // pack expansion expression.
        if (Arg.getArgument().getKind() == TemplateArgument::TemplateExpansion){
          E = ActOnPackExpansion(E.get(), Arg.getTemplateEllipsisLoc());
          if (E.isInvalid())
            return true;
        }

        TemplateArgument Result;
        E = CheckTemplateArgument(NTTP, NTTPType, E.get(), Result);
        if (E.isInvalid())
          return true;

        Converted.push_back(Result);
        break;
      }

      // We have a template argument that actually does refer to a class
      // template, alias template, or template template parameter, and
      // therefore cannot be a non-type template argument.
      Diag(Arg.getLocation(), diag::err_template_arg_must_be_expr)
        << Arg.getSourceRange();

      Diag(Param->getLocation(), diag::note_template_param_here);
      return true;

    case TemplateArgument::Type: {
      // We have a non-type template parameter but the template
      // argument is a type.

      // C++ [temp.arg]p2:
      //   In a template-argument, an ambiguity between a type-id and
      //   an expression is resolved to a type-id, regardless of the
      //   form of the corresponding template-parameter.
      //
      // We warn specifically about this case, since it can be rather
      // confusing for users.
      QualType T = Arg.getArgument().getAsType();
      SourceRange SR = Arg.getSourceRange();
      if (T->isFunctionType())
        Diag(SR.getBegin(), diag::err_template_arg_nontype_ambig) << SR << T;
      else
        Diag(SR.getBegin(), diag::err_template_arg_must_be_expr) << SR;
      Diag(Param->getLocation(), diag::note_template_param_here);
      return true;
    }

    case TemplateArgument::Pack:
      llvm_unreachable("Caller must expand template argument packs");
    }

    return false;
  }


  // Check template template parameters.
  TemplateTemplateParmDecl *TempParm = cast<TemplateTemplateParmDecl>(Param);

  // Substitute into the template parameter list of the template
  // template parameter, since previously-supplied template arguments
  // may appear within the template template parameter.
  {
    // Set up a template instantiation context.
    LocalInstantiationScope Scope(*this);
    InstantiatingTemplate Inst(*this, TemplateLoc, Template,
                               TempParm, Converted,
                               SourceRange(TemplateLoc, RAngleLoc));
    if (Inst.isInvalid())
      return true;

    TemplateArgumentList TemplateArgs(TemplateArgumentList::OnStack,
                                      Converted.data(), Converted.size());
    TempParm = cast_or_null<TemplateTemplateParmDecl>(
                      SubstDecl(TempParm, CurContext,
                                MultiLevelTemplateArgumentList(TemplateArgs)));
    if (!TempParm)
      return true;
  }

  switch (Arg.getArgument().getKind()) {
  case TemplateArgument::Null:
    llvm_unreachable("Should never see a NULL template argument here");

  case TemplateArgument::Template:
  case TemplateArgument::TemplateExpansion:
    if (CheckTemplateArgument(TempParm, Arg, ArgumentPackIndex))
      return true;

    Converted.push_back(Arg.getArgument());
    break;

  case TemplateArgument::Expression:
  case TemplateArgument::Type:
    // We have a template template parameter but the template
    // argument does not refer to a template.
    Diag(Arg.getLocation(), diag::err_template_arg_must_be_template)
      << getLangOpts().CPlusPlus11;
    return true;

  case TemplateArgument::Declaration:
    llvm_unreachable("Declaration argument with template template parameter");
  case TemplateArgument::Integral:
    llvm_unreachable("Integral argument with template template parameter");
  case TemplateArgument::NullPtr:
    llvm_unreachable("Null pointer argument with template template parameter");

  case TemplateArgument::Pack:
    llvm_unreachable("Caller must expand template argument packs");
  }

  return false;
}

/// \brief Diagnose an arity mismatch in the 
static bool diagnoseArityMismatch(Sema &S, TemplateDecl *Template,
                                  SourceLocation TemplateLoc,
                                  TemplateArgumentListInfo &TemplateArgs) {
  TemplateParameterList *Params = Template->getTemplateParameters();
  unsigned NumParams = Params->size();
  unsigned NumArgs = TemplateArgs.size();

  SourceRange Range;
  if (NumArgs > NumParams)
    Range = SourceRange(TemplateArgs[NumParams].getLocation(), 
                        TemplateArgs.getRAngleLoc());
  S.Diag(TemplateLoc, diag::err_template_arg_list_different_arity)
    << (NumArgs > NumParams)
    << (isa<ClassTemplateDecl>(Template)? 0 :
        isa<FunctionTemplateDecl>(Template)? 1 :
        isa<TemplateTemplateParmDecl>(Template)? 2 : 3)
    << Template << Range;
  if (Template->getLocation().isValid()) { // HLSL Change - ellide location notes for built-ins
  S.Diag(Template->getLocation(), diag::note_template_decl_here)
    << Params->getSourceRange();
  }
  return true;
}

/// \brief Check whether the template parameter is a pack expansion, and if so,
/// determine the number of parameters produced by that expansion. For instance:
///
/// \code
/// template<typename ...Ts> struct A {
///   template<Ts ...NTs, template<Ts> class ...TTs, typename ...Us> struct B;
/// };
/// \endcode
///
/// In \c A<int,int>::B, \c NTs and \c TTs have expanded pack size 2, and \c Us
/// is not a pack expansion, so returns an empty Optional.
static Optional<unsigned> getExpandedPackSize(NamedDecl *Param) {
  if (NonTypeTemplateParmDecl *NTTP
        = dyn_cast<NonTypeTemplateParmDecl>(Param)) {
    if (NTTP->isExpandedParameterPack())
      return NTTP->getNumExpansionTypes();
  }

  if (TemplateTemplateParmDecl *TTP
        = dyn_cast<TemplateTemplateParmDecl>(Param)) {
    if (TTP->isExpandedParameterPack())
      return TTP->getNumExpansionTemplateParameters();
  }

  return None;
}

/// Diagnose a missing template argument.
template<typename TemplateParmDecl>
static bool diagnoseMissingArgument(Sema &S, SourceLocation Loc,
                                    TemplateDecl *TD,
                                    const TemplateParmDecl *D,
                                    TemplateArgumentListInfo &Args) {
  // Dig out the most recent declaration of the template parameter; there may be
  // declarations of the template that are more recent than TD.
  D = cast<TemplateParmDecl>(cast<TemplateDecl>(TD->getMostRecentDecl())
                                 ->getTemplateParameters()
                                 ->getParam(D->getIndex()));

  // If there's a default argument that's not visible, diagnose that we're
  // missing a module import.
  llvm::SmallVector<Module*, 8> Modules;
  if (D->hasDefaultArgument() && !S.hasVisibleDefaultArgument(D, &Modules)) {
    S.diagnoseMissingImport(Loc, cast<NamedDecl>(TD),
                            D->getDefaultArgumentLoc(), Modules,
                            Sema::MissingImportKind::DefaultArgument,
                            /*Recover*/ true);
    return true;
  }

  // FIXME: If there's a more recent default argument that *is* visible,
  // diagnose that it was declared too late.

  return diagnoseArityMismatch(S, TD, Loc, Args);
}

/// \brief Check that the given template argument list is well-formed
/// for specializing the given template.
bool Sema::CheckTemplateArgumentList(TemplateDecl *Template,
                                     SourceLocation TemplateLoc,
                                     TemplateArgumentListInfo &TemplateArgs,
                                     bool PartialTemplateArgs,
                          SmallVectorImpl<TemplateArgument> &Converted) {
  // Make a copy of the template arguments for processing.  Only make the
  // changes at the end when successful in matching the arguments to the
  // template.
  TemplateArgumentListInfo NewArgs = TemplateArgs;

  TemplateParameterList *Params = Template->getTemplateParameters();

  SourceLocation RAngleLoc = NewArgs.getRAngleLoc();

  // C++ [temp.arg]p1:
  //   [...] The type and form of each template-argument specified in
  //   a template-id shall match the type and form specified for the
  //   corresponding parameter declared by the template in its
  //   template-parameter-list.
  bool isTemplateTemplateParameter = isa<TemplateTemplateParmDecl>(Template);
  SmallVector<TemplateArgument, 2> ArgumentPack;
  unsigned ArgIdx = 0, NumArgs = NewArgs.size();
  LocalInstantiationScope InstScope(*this, true);
  for (TemplateParameterList::iterator Param = Params->begin(),
                                       ParamEnd = Params->end();
       Param != ParamEnd; /* increment in loop */) {
    // If we have an expanded parameter pack, make sure we don't have too
    // many arguments.
    if (Optional<unsigned> Expansions = getExpandedPackSize(*Param)) {
      if (*Expansions == ArgumentPack.size()) {
        // We're done with this parameter pack. Pack up its arguments and add
        // them to the list.
        Converted.push_back(
          TemplateArgument::CreatePackCopy(Context,
                                           ArgumentPack.data(),
                                           ArgumentPack.size()));
        ArgumentPack.clear();

        // This argument is assigned to the next parameter.
        ++Param;
        continue;
      } else if (ArgIdx == NumArgs && !PartialTemplateArgs) {
        // Not enough arguments for this parameter pack.
        Diag(TemplateLoc, diag::err_template_arg_list_different_arity)
          << false
          << (isa<ClassTemplateDecl>(Template)? 0 :
              isa<FunctionTemplateDecl>(Template)? 1 :
              isa<TemplateTemplateParmDecl>(Template)? 2 : 3)
          << Template;
        if (Template->getLocation().isValid()) { // HLSL Change - ellide location notes for built-ins
        Diag(Template->getLocation(), diag::note_template_decl_here)
          << Params->getSourceRange();
        }
        return true;
      }
    }

    if (ArgIdx < NumArgs) {
      // Check the template argument we were given.
      if (CheckTemplateArgument(*Param, NewArgs[ArgIdx], Template,
                                TemplateLoc, RAngleLoc,
                                ArgumentPack.size(), Converted))
        return true;

      bool PackExpansionIntoNonPack =
          NewArgs[ArgIdx].getArgument().isPackExpansion() &&
          (!(*Param)->isTemplateParameterPack() || getExpandedPackSize(*Param));
      if (PackExpansionIntoNonPack && isa<TypeAliasTemplateDecl>(Template)) {
        // Core issue 1430: we have a pack expansion as an argument to an
        // alias template, and it's not part of a parameter pack. This
        // can't be canonicalized, so reject it now.
        Diag(NewArgs[ArgIdx].getLocation(),
             diag::err_alias_template_expansion_into_fixed_list)
          << NewArgs[ArgIdx].getSourceRange();
        Diag((*Param)->getLocation(), diag::note_template_param_here);
        return true;
      }

      // We're now done with this argument.
      ++ArgIdx;

      if ((*Param)->isTemplateParameterPack()) {
        // The template parameter was a template parameter pack, so take the
        // deduced argument and place it on the argument pack. Note that we
        // stay on the same template parameter so that we can deduce more
        // arguments.
        ArgumentPack.push_back(Converted.pop_back_val());
      } else {
        // Move to the next template parameter.
        ++Param;
      }

      // If we just saw a pack expansion into a non-pack, then directly convert
      // the remaining arguments, because we don't know what parameters they'll
      // match up with.
      if (PackExpansionIntoNonPack) {
        if (!ArgumentPack.empty()) {
          // If we were part way through filling in an expanded parameter pack,
          // fall back to just producing individual arguments.
          Converted.insert(Converted.end(),
                           ArgumentPack.begin(), ArgumentPack.end());
          ArgumentPack.clear();
        }

        while (ArgIdx < NumArgs) {
          Converted.push_back(NewArgs[ArgIdx].getArgument());
          ++ArgIdx;
        }

        return false;
      }

      continue;
    }

    // If we're checking a partial template argument list, we're done.
    if (PartialTemplateArgs) {
      if ((*Param)->isTemplateParameterPack() && !ArgumentPack.empty())
        Converted.push_back(TemplateArgument::CreatePackCopy(Context,
                                                         ArgumentPack.data(),
                                                         ArgumentPack.size()));
        
      return false;
    }

    // If we have a template parameter pack with no more corresponding
    // arguments, just break out now and we'll fill in the argument pack below.
    if ((*Param)->isTemplateParameterPack()) {
      assert(!getExpandedPackSize(*Param) &&
             "Should have dealt with this already");

      // A non-expanded parameter pack before the end of the parameter list
      // only occurs for an ill-formed template parameter list, unless we've
      // got a partial argument list for a function template, so just bail out.
      if (Param + 1 != ParamEnd)
        return true;

      Converted.push_back(TemplateArgument::CreatePackCopy(Context,
                                                       ArgumentPack.data(),
                                                       ArgumentPack.size()));
      ArgumentPack.clear();

      ++Param;
      continue;
    }

    // Check whether we have a default argument.
    TemplateArgumentLoc Arg;

    // Retrieve the default template argument from the template
    // parameter. For each kind of template parameter, we substitute the
    // template arguments provided thus far and any "outer" template arguments
    // (when the template parameter was part of a nested template) into
    // the default argument.
    if (TemplateTypeParmDecl *TTP = dyn_cast<TemplateTypeParmDecl>(*Param)) {
      if (!hasVisibleDefaultArgument(TTP))
        return diagnoseMissingArgument(*this, TemplateLoc, Template, TTP,
                                       NewArgs);

      TypeSourceInfo *ArgType = SubstDefaultTemplateArgument(*this,
                                                             Template,
                                                             TemplateLoc,
                                                             RAngleLoc,
                                                             TTP,
                                                             Converted);
      if (!ArgType)
        return true;

      Arg = TemplateArgumentLoc(TemplateArgument(ArgType->getType()),
                                ArgType);
    } else if (NonTypeTemplateParmDecl *NTTP
                 = dyn_cast<NonTypeTemplateParmDecl>(*Param)) {
      if (!hasVisibleDefaultArgument(NTTP))
        return diagnoseMissingArgument(*this, TemplateLoc, Template, NTTP,
                                       NewArgs);

      ExprResult E = SubstDefaultTemplateArgument(*this, Template,
                                                              TemplateLoc,
                                                              RAngleLoc,
                                                              NTTP,
                                                              Converted);
      if (E.isInvalid())
        return true;

      Expr *Ex = E.getAs<Expr>();
      Arg = TemplateArgumentLoc(TemplateArgument(Ex), Ex);
    } else {
      TemplateTemplateParmDecl *TempParm
        = cast<TemplateTemplateParmDecl>(*Param);

      if (!hasVisibleDefaultArgument(TempParm))
        return diagnoseMissingArgument(*this, TemplateLoc, Template, TempParm,
                                       NewArgs);

      NestedNameSpecifierLoc QualifierLoc;
      TemplateName Name = SubstDefaultTemplateArgument(*this, Template,
                                                       TemplateLoc,
                                                       RAngleLoc,
                                                       TempParm,
                                                       Converted,
                                                       QualifierLoc);
      if (Name.isNull())
        return true;

      Arg = TemplateArgumentLoc(TemplateArgument(Name), QualifierLoc,
                           TempParm->getDefaultArgument().getTemplateNameLoc());
    }

    // Introduce an instantiation record that describes where we are using
    // the default template argument.
    InstantiatingTemplate Inst(*this, RAngleLoc, Template, *Param, Converted,
                               SourceRange(TemplateLoc, RAngleLoc));
    if (Inst.isInvalid())
      return true;

    // Check the default template argument.
    if (CheckTemplateArgument(*Param, Arg, Template, TemplateLoc,
                              RAngleLoc, 0, Converted))
      return true;

    // Core issue 150 (assumed resolution): if this is a template template
    // parameter, keep track of the default template arguments from the
    // template definition.
    if (isTemplateTemplateParameter)
      NewArgs.addArgument(Arg);

    // Move to the next template parameter and argument.
    ++Param;
    ++ArgIdx;
  }

  // If we're performing a partial argument substitution, allow any trailing
  // pack expansions; they might be empty. This can happen even if
  // PartialTemplateArgs is false (the list of arguments is complete but
  // still dependent).
  if (ArgIdx < NumArgs && CurrentInstantiationScope &&
      CurrentInstantiationScope->getPartiallySubstitutedPack()) {
    while (ArgIdx < NumArgs && NewArgs[ArgIdx].getArgument().isPackExpansion())
      Converted.push_back(NewArgs[ArgIdx++].getArgument());
  }

  // If we have any leftover arguments, then there were too many arguments.
  // Complain and fail.
  if (ArgIdx < NumArgs)
    return diagnoseArityMismatch(*this, Template, TemplateLoc, NewArgs);

  // No problems found with the new argument list, propagate changes back
  // to caller.
  TemplateArgs = NewArgs;

  return false;
}

namespace {
  class UnnamedLocalNoLinkageFinder
    : public TypeVisitor<UnnamedLocalNoLinkageFinder, bool>
  {
    Sema &S;
    SourceRange SR;

    typedef TypeVisitor<UnnamedLocalNoLinkageFinder, bool> inherited;

  public:
    UnnamedLocalNoLinkageFinder(Sema &S, SourceRange SR) : S(S), SR(SR) { }

    bool Visit(QualType T) {
      return inherited::Visit(T.getTypePtr());
    }

#define TYPE(Class, Parent) \
    bool Visit##Class##Type(const Class##Type *);
#define ABSTRACT_TYPE(Class, Parent) \
    bool Visit##Class##Type(const Class##Type *) { return false; }
#define NON_CANONICAL_TYPE(Class, Parent) \
    bool Visit##Class##Type(const Class##Type *) { return false; }
#include "clang/AST/TypeNodes.def"

    bool VisitTagDecl(const TagDecl *Tag);
    bool VisitNestedNameSpecifier(NestedNameSpecifier *NNS);
  };
}

bool UnnamedLocalNoLinkageFinder::VisitBuiltinType(const BuiltinType*) {
  return false;
}

bool UnnamedLocalNoLinkageFinder::VisitComplexType(const ComplexType* T) {
  return Visit(T->getElementType());
}

bool UnnamedLocalNoLinkageFinder::VisitPointerType(const PointerType* T) {
  return Visit(T->getPointeeType());
}

bool UnnamedLocalNoLinkageFinder::VisitBlockPointerType(
                                                    const BlockPointerType* T) {
  return Visit(T->getPointeeType());
}

bool UnnamedLocalNoLinkageFinder::VisitLValueReferenceType(
                                                const LValueReferenceType* T) {
  return Visit(T->getPointeeType());
}

bool UnnamedLocalNoLinkageFinder::VisitRValueReferenceType(
                                                const RValueReferenceType* T) {
  return Visit(T->getPointeeType());
}

bool UnnamedLocalNoLinkageFinder::VisitMemberPointerType(
                                                  const MemberPointerType* T) {
  return Visit(T->getPointeeType()) || Visit(QualType(T->getClass(), 0));
}

bool UnnamedLocalNoLinkageFinder::VisitConstantArrayType(
                                                  const ConstantArrayType* T) {
  return Visit(T->getElementType());
}

bool UnnamedLocalNoLinkageFinder::VisitIncompleteArrayType(
                                                 const IncompleteArrayType* T) {
  return Visit(T->getElementType());
}

bool UnnamedLocalNoLinkageFinder::VisitVariableArrayType(
                                                   const VariableArrayType* T) {
  return Visit(T->getElementType());
}

bool UnnamedLocalNoLinkageFinder::VisitDependentSizedArrayType(
                                            const DependentSizedArrayType* T) {
  return Visit(T->getElementType());
}

bool UnnamedLocalNoLinkageFinder::VisitDependentSizedExtVectorType(
                                         const DependentSizedExtVectorType* T) {
  return Visit(T->getElementType());
}

bool UnnamedLocalNoLinkageFinder::VisitVectorType(const VectorType* T) {
  return Visit(T->getElementType());
}

bool UnnamedLocalNoLinkageFinder::VisitExtVectorType(const ExtVectorType* T) {
  return Visit(T->getElementType());
}

bool UnnamedLocalNoLinkageFinder::VisitFunctionProtoType(
                                                  const FunctionProtoType* T) {
  for (const auto &A : T->param_types()) {
    if (Visit(A))
      return true;
  }

  return Visit(T->getReturnType());
}

bool UnnamedLocalNoLinkageFinder::VisitFunctionNoProtoType(
                                               const FunctionNoProtoType* T) {
  return Visit(T->getReturnType());
}

bool UnnamedLocalNoLinkageFinder::VisitUnresolvedUsingType(
                                                  const UnresolvedUsingType*) {
  return false;
}

bool UnnamedLocalNoLinkageFinder::VisitTypeOfExprType(const TypeOfExprType*) {
  return false;
}

bool UnnamedLocalNoLinkageFinder::VisitTypeOfType(const TypeOfType* T) {
  return Visit(T->getUnderlyingType());
}

bool UnnamedLocalNoLinkageFinder::VisitDecltypeType(const DecltypeType*) {
  return false;
}

bool UnnamedLocalNoLinkageFinder::VisitUnaryTransformType(
                                                    const UnaryTransformType*) {
  return false;
}

bool UnnamedLocalNoLinkageFinder::VisitAutoType(const AutoType *T) {
  return Visit(T->getDeducedType());
}

bool UnnamedLocalNoLinkageFinder::VisitRecordType(const RecordType* T) {
  return VisitTagDecl(T->getDecl());
}

bool UnnamedLocalNoLinkageFinder::VisitEnumType(const EnumType* T) {
  return VisitTagDecl(T->getDecl());
}

bool UnnamedLocalNoLinkageFinder::VisitTemplateTypeParmType(
                                                 const TemplateTypeParmType*) {
  return false;
}

bool UnnamedLocalNoLinkageFinder::VisitSubstTemplateTypeParmPackType(
                                        const SubstTemplateTypeParmPackType *) {
  return false;
}

bool UnnamedLocalNoLinkageFinder::VisitTemplateSpecializationType(
                                            const TemplateSpecializationType*) {
  return false;
}

bool UnnamedLocalNoLinkageFinder::VisitInjectedClassNameType(
                                              const InjectedClassNameType* T) {
  return VisitTagDecl(T->getDecl());
}

bool UnnamedLocalNoLinkageFinder::VisitDependentNameType(
                                                   const DependentNameType* T) {
  return VisitNestedNameSpecifier(T->getQualifier());
}

bool UnnamedLocalNoLinkageFinder::VisitDependentTemplateSpecializationType(
                                 const DependentTemplateSpecializationType* T) {
  return VisitNestedNameSpecifier(T->getQualifier());
}

bool UnnamedLocalNoLinkageFinder::VisitPackExpansionType(
                                                   const PackExpansionType* T) {
  return Visit(T->getPattern());
}

bool UnnamedLocalNoLinkageFinder::VisitObjCObjectType(const ObjCObjectType *) {
  return false;
}

bool UnnamedLocalNoLinkageFinder::VisitObjCInterfaceType(
                                                   const ObjCInterfaceType *) {
  return false;
}

bool UnnamedLocalNoLinkageFinder::VisitObjCObjectPointerType(
                                                const ObjCObjectPointerType *) {
  return false;
}

bool UnnamedLocalNoLinkageFinder::VisitAtomicType(const AtomicType* T) {
  return Visit(T->getValueType());
}

bool UnnamedLocalNoLinkageFinder::VisitTagDecl(const TagDecl *Tag) {
  if (Tag->getDeclContext()->isFunctionOrMethod()) {
    S.Diag(SR.getBegin(),
           S.getLangOpts().CPlusPlus11 ?
             diag::warn_cxx98_compat_template_arg_local_type :
             diag::ext_template_arg_local_type)
      << S.Context.getTypeDeclType(Tag) << SR;
    return true;
  }

  if (!Tag->hasNameForLinkage()) {
    S.Diag(SR.getBegin(),
           S.getLangOpts().CPlusPlus11 ?
             diag::warn_cxx98_compat_template_arg_unnamed_type :
             diag::ext_template_arg_unnamed_type) << SR;
    S.Diag(Tag->getLocation(), diag::note_template_unnamed_type_here);
    return true;
  }

  return false;
}

bool UnnamedLocalNoLinkageFinder::VisitNestedNameSpecifier(
                                                    NestedNameSpecifier *NNS) {
  if (NNS->getPrefix() && VisitNestedNameSpecifier(NNS->getPrefix()))
    return true;

  switch (NNS->getKind()) {
  case NestedNameSpecifier::Identifier:
  case NestedNameSpecifier::Namespace:
  case NestedNameSpecifier::NamespaceAlias:
  case NestedNameSpecifier::Global:
  case NestedNameSpecifier::Super:
    return false;

  case NestedNameSpecifier::TypeSpec:
  case NestedNameSpecifier::TypeSpecWithTemplate:
    return Visit(QualType(NNS->getAsType(), 0));
  }
  llvm_unreachable("Invalid NestedNameSpecifier::Kind!");
}


/// \brief Check a template argument against its corresponding
/// template type parameter.
///
/// This routine implements the semantics of C++ [temp.arg.type]. It
/// returns true if an error occurred, and false otherwise.
bool Sema::CheckTemplateArgument(TemplateTypeParmDecl *Param,
                                 TypeSourceInfo *ArgInfo) {
  assert(ArgInfo && "invalid TypeSourceInfo");
  QualType Arg = ArgInfo->getType();
  SourceRange SR = ArgInfo->getTypeLoc().getSourceRange();

  if (Arg->isVariablyModifiedType()) {
    return Diag(SR.getBegin(), diag::err_variably_modified_template_arg) << Arg;
  } else if (Context.hasSameUnqualifiedType(Arg, Context.OverloadTy)) {
    return Diag(SR.getBegin(), diag::err_template_arg_overload_type) << SR;
  }

  // C++03 [temp.arg.type]p2:
  //   A local type, a type with no linkage, an unnamed type or a type
  //   compounded from any of these types shall not be used as a
  //   template-argument for a template type-parameter.
  //
  // C++11 allows these, and even in C++03 we allow them as an extension with
  // a warning.
  bool NeedsCheck;
  if (LangOpts.CPlusPlus11)
    NeedsCheck =
        !Diags.isIgnored(diag::warn_cxx98_compat_template_arg_unnamed_type,
                         SR.getBegin()) ||
        !Diags.isIgnored(diag::warn_cxx98_compat_template_arg_local_type,
                         SR.getBegin());
  else
    NeedsCheck = Arg->hasUnnamedOrLocalType();

  if (NeedsCheck) {
    UnnamedLocalNoLinkageFinder Finder(*this, SR);
    (void)Finder.Visit(Context.getCanonicalType(Arg));
  }

  return false;
}

enum NullPointerValueKind {
  NPV_NotNullPointer,
  NPV_NullPointer,
  NPV_Error
};

/// \brief Determine whether the given template argument is a null pointer
/// value of the appropriate type.
static NullPointerValueKind
isNullPointerValueTemplateArgument(Sema &S, NonTypeTemplateParmDecl *Param,
                                   QualType ParamType, Expr *Arg) {
  if (Arg->isValueDependent() || Arg->isTypeDependent())
    return NPV_NotNullPointer;
  
  if (!S.getLangOpts().CPlusPlus11)
    return NPV_NotNullPointer;
  
  // Determine whether we have a constant expression.
  ExprResult ArgRV = S.DefaultFunctionArrayConversion(Arg);
  if (ArgRV.isInvalid())
    return NPV_Error;
  Arg = ArgRV.get();
  
  Expr::EvalResult EvalResult;
  SmallVector<PartialDiagnosticAt, 8> Notes;
  EvalResult.Diag = &Notes;
  if (!Arg->EvaluateAsRValue(EvalResult, S.Context) ||
      EvalResult.HasSideEffects) {
    SourceLocation DiagLoc = Arg->getExprLoc();
    
    // If our only note is the usual "invalid subexpression" note, just point
    // the caret at its location rather than producing an essentially
    // redundant note.
    if (Notes.size() == 1 && Notes[0].second.getDiagID() ==
        diag::note_invalid_subexpr_in_const_expr) {
      DiagLoc = Notes[0].first;
      Notes.clear();
    }
    
    S.Diag(DiagLoc, diag::err_template_arg_not_address_constant)
      << Arg->getType() << Arg->getSourceRange();
    for (unsigned I = 0, N = Notes.size(); I != N; ++I)
      S.Diag(Notes[I].first, Notes[I].second);
    
    S.Diag(Param->getLocation(), diag::note_template_param_here);
    return NPV_Error;
  }
  
  // C++11 [temp.arg.nontype]p1:
  //   - an address constant expression of type std::nullptr_t
  if (Arg->getType()->isNullPtrType())
    return NPV_NullPointer;
  
  //   - a constant expression that evaluates to a null pointer value (4.10); or
  //   - a constant expression that evaluates to a null member pointer value
  //     (4.11); or
  if ((EvalResult.Val.isLValue() && !EvalResult.Val.getLValueBase()) ||
      (EvalResult.Val.isMemberPointer() &&
       !EvalResult.Val.getMemberPointerDecl())) {
    // If our expression has an appropriate type, we've succeeded.
    bool ObjCLifetimeConversion;
    if (S.Context.hasSameUnqualifiedType(Arg->getType(), ParamType) ||
        S.IsQualificationConversion(Arg->getType(), ParamType, false,
                                     ObjCLifetimeConversion))
      return NPV_NullPointer;
    
    // The types didn't match, but we know we got a null pointer; complain,
    // then recover as if the types were correct.
    S.Diag(Arg->getExprLoc(), diag::err_template_arg_wrongtype_null_constant)
      << Arg->getType() << ParamType << Arg->getSourceRange();
    S.Diag(Param->getLocation(), diag::note_template_param_here);
    return NPV_NullPointer;
  }

  // If we don't have a null pointer value, but we do have a NULL pointer
  // constant, suggest a cast to the appropriate type.
  if (Arg->isNullPointerConstant(S.Context, Expr::NPC_NeverValueDependent)) {
    std::string Code = "static_cast<" + ParamType.getAsString() + ">(";
    S.Diag(Arg->getExprLoc(), diag::err_template_arg_untyped_null_constant)
        << ParamType << FixItHint::CreateInsertion(Arg->getLocStart(), Code)
        << FixItHint::CreateInsertion(S.getLocForEndOfToken(Arg->getLocEnd()),
                                      ")");
    S.Diag(Param->getLocation(), diag::note_template_param_here);
    return NPV_NullPointer;
  }
  
  // FIXME: If we ever want to support general, address-constant expressions
  // as non-type template arguments, we should return the ExprResult here to
  // be interpreted by the caller.
  return NPV_NotNullPointer;
}

/// \brief Checks whether the given template argument is compatible with its
/// template parameter.
static bool CheckTemplateArgumentIsCompatibleWithParameter(
    Sema &S, NonTypeTemplateParmDecl *Param, QualType ParamType, Expr *ArgIn,
    Expr *Arg, QualType ArgType) {
  bool ObjCLifetimeConversion;
  if (ParamType->isPointerType() &&
      !ParamType->getAs<PointerType>()->getPointeeType()->isFunctionType() &&
      S.IsQualificationConversion(ArgType, ParamType, false,
                                  ObjCLifetimeConversion)) {
    // For pointer-to-object types, qualification conversions are
    // permitted.
  } else {
    if (const ReferenceType *ParamRef = ParamType->getAs<ReferenceType>()) {
      if (!ParamRef->getPointeeType()->isFunctionType()) {
        // C++ [temp.arg.nontype]p5b3:
        //   For a non-type template-parameter of type reference to
        //   object, no conversions apply. The type referred to by the
        //   reference may be more cv-qualified than the (otherwise
        //   identical) type of the template- argument. The
        //   template-parameter is bound directly to the
        //   template-argument, which shall be an lvalue.

        // FIXME: Other qualifiers?
        unsigned ParamQuals = ParamRef->getPointeeType().getCVRQualifiers();
        unsigned ArgQuals = ArgType.getCVRQualifiers();

        if ((ParamQuals | ArgQuals) != ParamQuals) {
          S.Diag(Arg->getLocStart(),
                 diag::err_template_arg_ref_bind_ignores_quals)
            << ParamType << Arg->getType() << Arg->getSourceRange();
          S.Diag(Param->getLocation(), diag::note_template_param_here);
          return true;
        }
      }
    }

    // At this point, the template argument refers to an object or
    // function with external linkage. We now need to check whether the
    // argument and parameter types are compatible.
    if (!S.Context.hasSameUnqualifiedType(ArgType,
                                          ParamType.getNonReferenceType())) {
      // We can't perform this conversion or binding.
      if (ParamType->isReferenceType())
        S.Diag(Arg->getLocStart(), diag::err_template_arg_no_ref_bind)
          << ParamType << ArgIn->getType() << Arg->getSourceRange();
      else
        S.Diag(Arg->getLocStart(),  diag::err_template_arg_not_convertible)
          << ArgIn->getType() << ParamType << Arg->getSourceRange();
      S.Diag(Param->getLocation(), diag::note_template_param_here);
      return true;
    }
  }

  return false;
}

/// \brief Checks whether the given template argument is the address
/// of an object or function according to C++ [temp.arg.nontype]p1.
static bool
CheckTemplateArgumentAddressOfObjectOrFunction(Sema &S,
                                               NonTypeTemplateParmDecl *Param,
                                               QualType ParamType,
                                               Expr *ArgIn,
                                               TemplateArgument &Converted) {
  bool Invalid = false;
  Expr *Arg = ArgIn;
  QualType ArgType = Arg->getType();

  bool AddressTaken = false;
  SourceLocation AddrOpLoc;
  if (S.getLangOpts().MicrosoftExt) {
    // Microsoft Visual C++ strips all casts, allows an arbitrary number of
    // dereference and address-of operators.
    Arg = Arg->IgnoreParenCasts();

    bool ExtWarnMSTemplateArg = false;
    UnaryOperatorKind FirstOpKind;
    SourceLocation FirstOpLoc;
    while (UnaryOperator *UnOp = dyn_cast<UnaryOperator>(Arg)) {
      UnaryOperatorKind UnOpKind = UnOp->getOpcode();
      if (UnOpKind == UO_Deref)
        ExtWarnMSTemplateArg = true;
      if (UnOpKind == UO_AddrOf || UnOpKind == UO_Deref) {
        Arg = UnOp->getSubExpr()->IgnoreParenCasts();
        if (!AddrOpLoc.isValid()) {
          FirstOpKind = UnOpKind;
          FirstOpLoc = UnOp->getOperatorLoc();
        }
      } else
        break;
    }
    if (FirstOpLoc.isValid()) {
      if (ExtWarnMSTemplateArg)
        S.Diag(ArgIn->getLocStart(), diag::ext_ms_deref_template_argument)
          << ArgIn->getSourceRange();

      if (FirstOpKind == UO_AddrOf)
        AddressTaken = true;
      else if (Arg->getType()->isPointerType()) {
        // We cannot let pointers get dereferenced here, that is obviously not a
        // constant expression.
        assert(FirstOpKind == UO_Deref);
        S.Diag(Arg->getLocStart(), diag::err_template_arg_not_decl_ref)
          << Arg->getSourceRange();
      }
    }
  } else {
    // See through any implicit casts we added to fix the type.
    Arg = Arg->IgnoreImpCasts();

    // C++ [temp.arg.nontype]p1:
    //
    //   A template-argument for a non-type, non-template
    //   template-parameter shall be one of: [...]
    //
    //     -- the address of an object or function with external
    //        linkage, including function templates and function
    //        template-ids but excluding non-static class members,
    //        expressed as & id-expression where the & is optional if
    //        the name refers to a function or array, or if the
    //        corresponding template-parameter is a reference; or

    // In C++98/03 mode, give an extension warning on any extra parentheses.
    // See http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#773
    bool ExtraParens = false;
    while (ParenExpr *Parens = dyn_cast<ParenExpr>(Arg)) {
      if (!Invalid && !ExtraParens) {
        S.Diag(Arg->getLocStart(),
               S.getLangOpts().CPlusPlus11
                   ? diag::warn_cxx98_compat_template_arg_extra_parens
                   : diag::ext_template_arg_extra_parens)
            << Arg->getSourceRange();
        ExtraParens = true;
      }

      Arg = Parens->getSubExpr();
    }

    while (SubstNonTypeTemplateParmExpr *subst =
               dyn_cast<SubstNonTypeTemplateParmExpr>(Arg))
      Arg = subst->getReplacement()->IgnoreImpCasts();

    if (UnaryOperator *UnOp = dyn_cast<UnaryOperator>(Arg)) {
      if (UnOp->getOpcode() == UO_AddrOf) {
        Arg = UnOp->getSubExpr();
        AddressTaken = true;
        AddrOpLoc = UnOp->getOperatorLoc();
      }
    }

    while (SubstNonTypeTemplateParmExpr *subst =
               dyn_cast<SubstNonTypeTemplateParmExpr>(Arg))
      Arg = subst->getReplacement()->IgnoreImpCasts();
  }

  DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(Arg);
  ValueDecl *Entity = DRE ? DRE->getDecl() : nullptr;

  // If our parameter has pointer type, check for a null template value.
  if (ParamType->isPointerType() || ParamType->isNullPtrType()) {
    NullPointerValueKind NPV;
    // dllimport'd entities aren't constant but are available inside of template
    // arguments.
    if (Entity && Entity->hasAttr<DLLImportAttr>())
      NPV = NPV_NotNullPointer;
    else
      NPV = isNullPointerValueTemplateArgument(S, Param, ParamType, ArgIn);
    switch (NPV) {
    case NPV_NullPointer:
      S.Diag(Arg->getExprLoc(), diag::warn_cxx98_compat_template_arg_null);
      Converted = TemplateArgument(S.Context.getCanonicalType(ParamType),
                                   /*isNullPtr=*/true);
      return false;

    case NPV_Error:
      return true;

    case NPV_NotNullPointer:
      break;
    }
  }

  // Stop checking the precise nature of the argument if it is value dependent,
  // it should be checked when instantiated.
  if (Arg->isValueDependent()) {
    Converted = TemplateArgument(ArgIn);
    return false;
  }

  if (isa<CXXUuidofExpr>(Arg)) {
    if (CheckTemplateArgumentIsCompatibleWithParameter(S, Param, ParamType,
                                                       ArgIn, Arg, ArgType))
      return true;

    Converted = TemplateArgument(ArgIn);
    return false;
  }

  if (!DRE) {
    S.Diag(Arg->getLocStart(), diag::err_template_arg_not_decl_ref)
    << Arg->getSourceRange();
    S.Diag(Param->getLocation(), diag::note_template_param_here);
    return true;
  }

  // Cannot refer to non-static data members
  if (isa<FieldDecl>(Entity) || isa<IndirectFieldDecl>(Entity)) {
    S.Diag(Arg->getLocStart(), diag::err_template_arg_field)
      << Entity << Arg->getSourceRange();
    S.Diag(Param->getLocation(), diag::note_template_param_here);
    return true;
  }

  // Cannot refer to non-static member functions
  if (CXXMethodDecl *Method = dyn_cast<CXXMethodDecl>(Entity)) {
    if (!Method->isStatic()) {
      S.Diag(Arg->getLocStart(), diag::err_template_arg_method)
        << Method << Arg->getSourceRange();
      S.Diag(Param->getLocation(), diag::note_template_param_here);
      return true;
    }
  }

  FunctionDecl *Func = dyn_cast<FunctionDecl>(Entity);
  VarDecl *Var = dyn_cast<VarDecl>(Entity);

  // A non-type template argument must refer to an object or function.
  if (!Func && !Var) {
    // We found something, but we don't know specifically what it is.
    S.Diag(Arg->getLocStart(), diag::err_template_arg_not_object_or_func)
      << Arg->getSourceRange();
    S.Diag(DRE->getDecl()->getLocation(), diag::note_template_arg_refers_here);
    return true;
  }

  // Address / reference template args must have external linkage in C++98.
  if (Entity->getFormalLinkage() == InternalLinkage) {
    S.Diag(Arg->getLocStart(), S.getLangOpts().CPlusPlus11 ?
             diag::warn_cxx98_compat_template_arg_object_internal :
             diag::ext_template_arg_object_internal)
      << !Func << Entity << Arg->getSourceRange();
    S.Diag(Entity->getLocation(), diag::note_template_arg_internal_object)
      << !Func;
  } else if (!Entity->hasLinkage()) {
    S.Diag(Arg->getLocStart(), diag::err_template_arg_object_no_linkage)
      << !Func << Entity << Arg->getSourceRange();
    S.Diag(Entity->getLocation(), diag::note_template_arg_internal_object)
      << !Func;
    return true;
  }

  if (Func) {
    // If the template parameter has pointer type, the function decays.
    if (ParamType->isPointerType() && !AddressTaken)
      ArgType = S.Context.getPointerType(Func->getType());
    else if (AddressTaken && ParamType->isReferenceType()) {
      // If we originally had an address-of operator, but the
      // parameter has reference type, complain and (if things look
      // like they will work) drop the address-of operator.
      if (!S.Context.hasSameUnqualifiedType(Func->getType(),
                                            ParamType.getNonReferenceType())) {
        S.Diag(AddrOpLoc, diag::err_template_arg_address_of_non_pointer)
          << ParamType;
        S.Diag(Param->getLocation(), diag::note_template_param_here);
        return true;
      }

      S.Diag(AddrOpLoc, diag::err_template_arg_address_of_non_pointer)
        << ParamType
        << FixItHint::CreateRemoval(AddrOpLoc);
      S.Diag(Param->getLocation(), diag::note_template_param_here);

      ArgType = Func->getType();
    }
  } else {
    // A value of reference type is not an object.
    if (Var->getType()->isReferenceType()) {
      S.Diag(Arg->getLocStart(),
             diag::err_template_arg_reference_var)
        << Var->getType() << Arg->getSourceRange();
      S.Diag(Param->getLocation(), diag::note_template_param_here);
      return true;
    }

    // A template argument must have static storage duration.
    if (Var->getTLSKind()) {
      S.Diag(Arg->getLocStart(), diag::err_template_arg_thread_local)
        << Arg->getSourceRange();
      S.Diag(Var->getLocation(), diag::note_template_arg_refers_here);
      return true;
    }

    // If the template parameter has pointer type, we must have taken
    // the address of this object.
    if (ParamType->isReferenceType()) {
      if (AddressTaken) {
        // If we originally had an address-of operator, but the
        // parameter has reference type, complain and (if things look
        // like they will work) drop the address-of operator.
        if (!S.Context.hasSameUnqualifiedType(Var->getType(),
                                            ParamType.getNonReferenceType())) {
          S.Diag(AddrOpLoc, diag::err_template_arg_address_of_non_pointer)
            << ParamType;
          S.Diag(Param->getLocation(), diag::note_template_param_here);
          return true;
        }

        S.Diag(AddrOpLoc, diag::err_template_arg_address_of_non_pointer)
          << ParamType
          << FixItHint::CreateRemoval(AddrOpLoc);
        S.Diag(Param->getLocation(), diag::note_template_param_here);

        ArgType = Var->getType();
      }
    } else if (!AddressTaken && ParamType->isPointerType()) {
      if (Var->getType()->isArrayType()) {
        // Array-to-pointer decay.
        ArgType = S.Context.getArrayDecayedType(Var->getType());
      } else {
        // If the template parameter has pointer type but the address of
        // this object was not taken, complain and (possibly) recover by
        // taking the address of the entity.
        ArgType = S.Context.getPointerType(Var->getType());
        if (!S.Context.hasSameUnqualifiedType(ArgType, ParamType)) {
          S.Diag(Arg->getLocStart(), diag::err_template_arg_not_address_of)
            << ParamType;
          S.Diag(Param->getLocation(), diag::note_template_param_here);
          return true;
        }

        S.Diag(Arg->getLocStart(), diag::err_template_arg_not_address_of)
          << ParamType
          << FixItHint::CreateInsertion(Arg->getLocStart(), "&");

        S.Diag(Param->getLocation(), diag::note_template_param_here);
      }
    }
  }

  if (CheckTemplateArgumentIsCompatibleWithParameter(S, Param, ParamType, ArgIn,
                                                     Arg, ArgType))
    return true;

  // Create the template argument.
  Converted =
      TemplateArgument(cast<ValueDecl>(Entity->getCanonicalDecl()), ParamType);
  S.MarkAnyDeclReferenced(Arg->getLocStart(), Entity, false);
  return false;
}

/// \brief Checks whether the given template argument is a pointer to
/// member constant according to C++ [temp.arg.nontype]p1.
static bool CheckTemplateArgumentPointerToMember(Sema &S,
                                                 NonTypeTemplateParmDecl *Param,
                                                 QualType ParamType,
                                                 Expr *&ResultArg,
                                                 TemplateArgument &Converted) {
  bool Invalid = false;

  // Check for a null pointer value.
  Expr *Arg = ResultArg;
  switch (isNullPointerValueTemplateArgument(S, Param, ParamType, Arg)) {
  case NPV_Error:
    return true;
  case NPV_NullPointer:
    S.Diag(Arg->getExprLoc(), diag::warn_cxx98_compat_template_arg_null);
    Converted = TemplateArgument(S.Context.getCanonicalType(ParamType),
                                 /*isNullPtr*/true);
    if (S.Context.getTargetInfo().getCXXABI().isMicrosoft())
      S.RequireCompleteType(Arg->getExprLoc(), ParamType, 0);
    return false;
  case NPV_NotNullPointer:
    break;
  }

  bool ObjCLifetimeConversion;
  if (S.IsQualificationConversion(Arg->getType(),
                                  ParamType.getNonReferenceType(),
                                  false, ObjCLifetimeConversion)) {
    Arg = S.ImpCastExprToType(Arg, ParamType, CK_NoOp,
                              Arg->getValueKind()).get();
    ResultArg = Arg;
  } else if (!S.Context.hasSameUnqualifiedType(Arg->getType(),
                ParamType.getNonReferenceType())) {
    // We can't perform this conversion.
    S.Diag(Arg->getLocStart(), diag::err_template_arg_not_convertible)
      << Arg->getType() << ParamType << Arg->getSourceRange();
    S.Diag(Param->getLocation(), diag::note_template_param_here);
    return true;
  }

  // See through any implicit casts we added to fix the type.
  while (ImplicitCastExpr *Cast = dyn_cast<ImplicitCastExpr>(Arg))
    Arg = Cast->getSubExpr();

  // C++ [temp.arg.nontype]p1:
  //
  //   A template-argument for a non-type, non-template
  //   template-parameter shall be one of: [...]
  //
  //     -- a pointer to member expressed as described in 5.3.1.
  DeclRefExpr *DRE = nullptr;

  // In C++98/03 mode, give an extension warning on any extra parentheses.
  // See http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#773
  bool ExtraParens = false;
  while (ParenExpr *Parens = dyn_cast<ParenExpr>(Arg)) {
    if (!Invalid && !ExtraParens) {
      S.Diag(Arg->getLocStart(),
             S.getLangOpts().CPlusPlus11 ?
               diag::warn_cxx98_compat_template_arg_extra_parens :
               diag::ext_template_arg_extra_parens)
        << Arg->getSourceRange();
      ExtraParens = true;
    }

    Arg = Parens->getSubExpr();
  }

  while (SubstNonTypeTemplateParmExpr *subst =
           dyn_cast<SubstNonTypeTemplateParmExpr>(Arg))
    Arg = subst->getReplacement()->IgnoreImpCasts();

  // A pointer-to-member constant written &Class::member.
  if (UnaryOperator *UnOp = dyn_cast<UnaryOperator>(Arg)) {
    if (UnOp->getOpcode() == UO_AddrOf) {
      DRE = dyn_cast<DeclRefExpr>(UnOp->getSubExpr());
      if (DRE && !DRE->getQualifier())
        DRE = nullptr;
    }
  }
  // A constant of pointer-to-member type.
  else if ((DRE = dyn_cast<DeclRefExpr>(Arg))) {
    if (ValueDecl *VD = dyn_cast<ValueDecl>(DRE->getDecl())) {
      if (VD->getType()->isMemberPointerType()) {
        if (isa<NonTypeTemplateParmDecl>(VD)) {
          if (Arg->isTypeDependent() || Arg->isValueDependent()) {
            Converted = TemplateArgument(Arg);
          } else {
            VD = cast<ValueDecl>(VD->getCanonicalDecl());
            Converted = TemplateArgument(VD, ParamType);
          }
          return Invalid;
        }
      }
    }

    DRE = nullptr;
  }

  if (!DRE)
    return S.Diag(Arg->getLocStart(),
                  diag::err_template_arg_not_pointer_to_member_form)
      << Arg->getSourceRange();

  if (isa<FieldDecl>(DRE->getDecl()) ||
      isa<IndirectFieldDecl>(DRE->getDecl()) ||
      isa<CXXMethodDecl>(DRE->getDecl())) {
    assert((isa<FieldDecl>(DRE->getDecl()) ||
            isa<IndirectFieldDecl>(DRE->getDecl()) ||
            !cast<CXXMethodDecl>(DRE->getDecl())->isStatic()) &&
           "Only non-static member pointers can make it here");

    // Okay: this is the address of a non-static member, and therefore
    // a member pointer constant.
    if (Arg->isTypeDependent() || Arg->isValueDependent()) {
      Converted = TemplateArgument(Arg);
    } else {
      ValueDecl *D = cast<ValueDecl>(DRE->getDecl()->getCanonicalDecl());
      Converted = TemplateArgument(D, ParamType);
    }
    return Invalid;
  }

  // We found something else, but we don't know specifically what it is.
  S.Diag(Arg->getLocStart(),
         diag::err_template_arg_not_pointer_to_member_form)
    << Arg->getSourceRange();
  S.Diag(DRE->getDecl()->getLocation(), diag::note_template_arg_refers_here);
  return true;
}

/// \brief Check a template argument against its corresponding
/// non-type template parameter.
///
/// This routine implements the semantics of C++ [temp.arg.nontype].
/// If an error occurred, it returns ExprError(); otherwise, it
/// returns the converted template argument. \p ParamType is the
/// type of the non-type template parameter after it has been instantiated.
ExprResult Sema::CheckTemplateArgument(NonTypeTemplateParmDecl *Param,
                                       QualType ParamType, Expr *Arg,
                                       TemplateArgument &Converted,
                                       CheckTemplateArgumentKind CTAK) {
  SourceLocation StartLoc = Arg->getLocStart();

  // If either the parameter has a dependent type or the argument is
  // type-dependent, there's nothing we can check now.
  if (ParamType->isDependentType() || Arg->isTypeDependent()) {
    // FIXME: Produce a cloned, canonical expression?
    Converted = TemplateArgument(Arg);
    return Arg;
  }

  // We should have already dropped all cv-qualifiers by now.
  assert(!ParamType.hasQualifiers() &&
         "non-type template parameter type cannot be qualified");

  if (CTAK == CTAK_Deduced &&
      !Context.hasSameUnqualifiedType(ParamType, Arg->getType())) {
    // C++ [temp.deduct.type]p17:
    //   If, in the declaration of a function template with a non-type
    //   template-parameter, the non-type template-parameter is used
    //   in an expression in the function parameter-list and, if the
    //   corresponding template-argument is deduced, the
    //   template-argument type shall match the type of the
    //   template-parameter exactly, except that a template-argument
    //   deduced from an array bound may be of any integral type.
    Diag(StartLoc, diag::err_deduced_non_type_template_arg_type_mismatch)
      << Arg->getType().getUnqualifiedType()
      << ParamType.getUnqualifiedType();
    Diag(Param->getLocation(), diag::note_template_param_here);
    return ExprError();
  }

  if (getLangOpts().CPlusPlus1z) {
    // FIXME: We can do some limited checking for a value-dependent but not
    // type-dependent argument.
    if (Arg->isValueDependent()) {
      Converted = TemplateArgument(Arg);
      return Arg;
    }

    // C++1z [temp.arg.nontype]p1:
    //   A template-argument for a non-type template parameter shall be
    //   a converted constant expression of the type of the template-parameter.
    APValue Value;
    ExprResult ArgResult = CheckConvertedConstantExpression(
        Arg, ParamType, Value, CCEK_TemplateArg);
    if (ArgResult.isInvalid())
      return ExprError();

    QualType CanonParamType = Context.getCanonicalType(ParamType);

    // Convert the APValue to a TemplateArgument.
    switch (Value.getKind()) {
    case APValue::Uninitialized:
      assert(ParamType->isNullPtrType());
      Converted = TemplateArgument(CanonParamType, /*isNullPtr*/true);
      break;
    case APValue::Int:
      assert(ParamType->isIntegralOrEnumerationType());
      Converted = TemplateArgument(Context, Value.getInt(), CanonParamType);
      break;
    case APValue::MemberPointer: {
      assert(ParamType->isMemberPointerType());

      // FIXME: We need TemplateArgument representation and mangling for these.
      if (!Value.getMemberPointerPath().empty()) {
        Diag(Arg->getLocStart(),
             diag::err_template_arg_member_ptr_base_derived_not_supported)
            << Value.getMemberPointerDecl() << ParamType
            << Arg->getSourceRange();
        return ExprError();
      }

      auto *VD = const_cast<ValueDecl*>(Value.getMemberPointerDecl());
      Converted = VD ? TemplateArgument(VD, CanonParamType)
                     : TemplateArgument(CanonParamType, /*isNullPtr*/true);
      break;
    }
    case APValue::LValue: {
      //   For a non-type template-parameter of pointer or reference type,
      //   the value of the constant expression shall not refer to
      assert(ParamType->isPointerType() || ParamType->isReferenceType() ||
             ParamType->isNullPtrType());
      // -- a temporary object
      // -- a string literal
      // -- the result of a typeid expression, or
      // -- a predefind __func__ variable
      if (auto *E = Value.getLValueBase().dyn_cast<const Expr*>()) {
        if (isa<CXXUuidofExpr>(E)) {
          Converted = TemplateArgument(const_cast<Expr*>(E));
          break;
        }
        Diag(Arg->getLocStart(), diag::err_template_arg_not_decl_ref)
          << Arg->getSourceRange();
        return ExprError();
      }
      auto *VD = const_cast<ValueDecl *>(
          Value.getLValueBase().dyn_cast<const ValueDecl *>());
      // -- a subobject
      if (Value.hasLValuePath() && Value.getLValuePath().size() == 1 &&
          VD && VD->getType()->isArrayType() &&
          Value.getLValuePath()[0].ArrayIndex == 0 &&
          !Value.isLValueOnePastTheEnd() && ParamType->isPointerType()) {
        // Per defect report (no number yet):
        //   ... other than a pointer to the first element of a complete array
        //       object.
      } else if (!Value.hasLValuePath() || Value.getLValuePath().size() ||
                 Value.isLValueOnePastTheEnd()) {
        Diag(StartLoc, diag::err_non_type_template_arg_subobject)
          << Value.getAsString(Context, ParamType);
        return ExprError();
      }
      assert((VD || !ParamType->isReferenceType()) &&
             "null reference should not be a constant expression");
      assert((!VD || !ParamType->isNullPtrType()) &&
             "non-null value of type nullptr_t?");
      Converted = VD ? TemplateArgument(VD, CanonParamType)
                     : TemplateArgument(CanonParamType, /*isNullPtr*/true);
      break;
    }
    case APValue::AddrLabelDiff:
      return Diag(StartLoc, diag::err_non_type_template_arg_addr_label_diff);
    case APValue::Float:
    case APValue::ComplexInt:
    case APValue::ComplexFloat:
    case APValue::Vector:
    case APValue::Array:
    case APValue::Struct:
    case APValue::Union:
      llvm_unreachable("invalid kind for template argument");
    }

    return ArgResult.get();
  }

  // C++ [temp.arg.nontype]p5:
  //   The following conversions are performed on each expression used
  //   as a non-type template-argument. If a non-type
  //   template-argument cannot be converted to the type of the
  //   corresponding template-parameter then the program is
  //   ill-formed.
  if (ParamType->isIntegralOrEnumerationType()) {
    // C++11:
    //   -- for a non-type template-parameter of integral or
    //      enumeration type, conversions permitted in a converted
    //      constant expression are applied.
    //
    // C++98:
    //   -- for a non-type template-parameter of integral or
    //      enumeration type, integral promotions (4.5) and integral
    //      conversions (4.7) are applied.

    if (getLangOpts().CPlusPlus11) {
      // We can't check arbitrary value-dependent arguments.
      // FIXME: If there's no viable conversion to the template parameter type,
      // we should be able to diagnose that prior to instantiation.
      if (Arg->isValueDependent()) {
        Converted = TemplateArgument(Arg);
        return Arg;
      }

      // C++ [temp.arg.nontype]p1:
      //   A template-argument for a non-type, non-template template-parameter
      //   shall be one of:
      //
      //     -- for a non-type template-parameter of integral or enumeration
      //        type, a converted constant expression of the type of the
      //        template-parameter; or
      llvm::APSInt Value;
      ExprResult ArgResult =
        CheckConvertedConstantExpression(Arg, ParamType, Value,
                                         CCEK_TemplateArg);
      if (ArgResult.isInvalid())
        return ExprError();

      // Widen the argument value to sizeof(parameter type). This is almost
      // always a no-op, except when the parameter type is bool. In
      // that case, this may extend the argument from 1 bit to 8 bits.
      QualType IntegerType = ParamType;
      if (const EnumType *Enum = IntegerType->getAs<EnumType>())
        IntegerType = Enum->getDecl()->getIntegerType();
      Value = Value.extOrTrunc(Context.getTypeSize(IntegerType));

      Converted = TemplateArgument(Context, Value,
                                   Context.getCanonicalType(ParamType));
      return ArgResult;
    }

    ExprResult ArgResult = DefaultLvalueConversion(Arg);
    if (ArgResult.isInvalid())
      return ExprError();
    Arg = ArgResult.get();

    QualType ArgType = Arg->getType();

    // C++ [temp.arg.nontype]p1:
    //   A template-argument for a non-type, non-template
    //   template-parameter shall be one of:
    //
    //     -- an integral constant-expression of integral or enumeration
    //        type; or
    //     -- the name of a non-type template-parameter; or
    SourceLocation NonConstantLoc;
    llvm::APSInt Value;
    if (!ArgType->isIntegralOrEnumerationType()) {
      Diag(Arg->getLocStart(),
           diag::err_template_arg_not_integral_or_enumeral)
        << ArgType << Arg->getSourceRange();
      Diag(Param->getLocation(), diag::note_template_param_here);
      return ExprError();
    } else if (!Arg->isValueDependent()) {
      class TmplArgICEDiagnoser : public VerifyICEDiagnoser {
        QualType T;
        
      public:
        TmplArgICEDiagnoser(QualType T) : T(T) { }

        void diagnoseNotICE(Sema &S, SourceLocation Loc,
                            SourceRange SR) override {
          S.Diag(Loc, diag::err_template_arg_not_ice) << T << SR;
        }
      } Diagnoser(ArgType);

      Arg = VerifyIntegerConstantExpression(Arg, &Value, Diagnoser,
                                            false).get();
      if (!Arg)
        return ExprError();
    }

    // From here on out, all we care about is the unqualified form
    // of the argument type.
    ArgType = ArgType.getUnqualifiedType();

    // Try to convert the argument to the parameter's type.
    if (Context.hasSameType(ParamType, ArgType)) {
      // Okay: no conversion necessary
    } else if (ParamType->isBooleanType()) {
      // This is an integral-to-boolean conversion.
      Arg = ImpCastExprToType(Arg, ParamType, CK_IntegralToBoolean).get();
    } else if (IsIntegralPromotion(Arg, ArgType, ParamType) ||
               !ParamType->isEnumeralType()) {
      // This is an integral promotion or conversion.
      Arg = ImpCastExprToType(Arg, ParamType, CK_IntegralCast).get();
    } else {
      // We can't perform this conversion.
      Diag(Arg->getLocStart(),
           diag::err_template_arg_not_convertible)
        << Arg->getType() << ParamType << Arg->getSourceRange();
      Diag(Param->getLocation(), diag::note_template_param_here);
      return ExprError();
    }

    // Add the value of this argument to the list of converted
    // arguments. We use the bitwidth and signedness of the template
    // parameter.
    if (Arg->isValueDependent()) {
      // The argument is value-dependent. Create a new
      // TemplateArgument with the converted expression.
      Converted = TemplateArgument(Arg);
      return Arg;
    }

    QualType IntegerType = Context.getCanonicalType(ParamType);
    if (const EnumType *Enum = IntegerType->getAs<EnumType>())
      IntegerType = Context.getCanonicalType(Enum->getDecl()->getIntegerType());

    if (ParamType->isBooleanType()) {
      // Value must be zero or one.
      Value = Value != 0;
      unsigned AllowedBits = Context.getTypeSize(IntegerType);
      if (Value.getBitWidth() != AllowedBits)
        Value = Value.extOrTrunc(AllowedBits);
      Value.setIsSigned(IntegerType->isSignedIntegerOrEnumerationType());
    } else {
      llvm::APSInt OldValue = Value;
      
      // Coerce the template argument's value to the value it will have
      // based on the template parameter's type.
      unsigned AllowedBits = Context.getTypeSize(IntegerType);
      if (Value.getBitWidth() != AllowedBits)
        Value = Value.extOrTrunc(AllowedBits);
      Value.setIsSigned(IntegerType->isSignedIntegerOrEnumerationType());
      
      // Complain if an unsigned parameter received a negative value.
      if (IntegerType->isUnsignedIntegerOrEnumerationType()
               && (OldValue.isSigned() && OldValue.isNegative())) {
        Diag(Arg->getLocStart(), diag::warn_template_arg_negative)
          << OldValue.toString(10) << Value.toString(10) << Param->getType()
          << Arg->getSourceRange();
        Diag(Param->getLocation(), diag::note_template_param_here);
      }
      
      // Complain if we overflowed the template parameter's type.
      unsigned RequiredBits;
      if (IntegerType->isUnsignedIntegerOrEnumerationType())
        RequiredBits = OldValue.getActiveBits();
      else if (OldValue.isUnsigned())
        RequiredBits = OldValue.getActiveBits() + 1;
      else
        RequiredBits = OldValue.getMinSignedBits();
      if (RequiredBits > AllowedBits) {
        Diag(Arg->getLocStart(),
             diag::warn_template_arg_too_large)
          << OldValue.toString(10) << Value.toString(10) << Param->getType()
          << Arg->getSourceRange();
        Diag(Param->getLocation(), diag::note_template_param_here);
      }
    }

    Converted = TemplateArgument(Context, Value,
                                 ParamType->isEnumeralType() 
                                   ? Context.getCanonicalType(ParamType)
                                   : IntegerType);
    return Arg;
  }

  QualType ArgType = Arg->getType();
  DeclAccessPair FoundResult; // temporary for ResolveOverloadedFunction

  // Handle pointer-to-function, reference-to-function, and
  // pointer-to-member-function all in (roughly) the same way.
  if (// -- For a non-type template-parameter of type pointer to
      //    function, only the function-to-pointer conversion (4.3) is
      //    applied. If the template-argument represents a set of
      //    overloaded functions (or a pointer to such), the matching
      //    function is selected from the set (13.4).
      (ParamType->isPointerType() &&
       ParamType->getAs<PointerType>()->getPointeeType()->isFunctionType()) ||
      // -- For a non-type template-parameter of type reference to
      //    function, no conversions apply. If the template-argument
      //    represents a set of overloaded functions, the matching
      //    function is selected from the set (13.4).
      (ParamType->isReferenceType() &&
       ParamType->getAs<ReferenceType>()->getPointeeType()->isFunctionType()) ||
      // -- For a non-type template-parameter of type pointer to
      //    member function, no conversions apply. If the
      //    template-argument represents a set of overloaded member
      //    functions, the matching member function is selected from
      //    the set (13.4).
      (ParamType->isMemberPointerType() &&
       ParamType->getAs<MemberPointerType>()->getPointeeType()
         ->isFunctionType())) {

    if (Arg->getType() == Context.OverloadTy) {
      if (FunctionDecl *Fn = ResolveAddressOfOverloadedFunction(Arg, ParamType,
                                                                true,
                                                                FoundResult)) {
        if (DiagnoseUseOfDecl(Fn, Arg->getLocStart()))
          return ExprError();

        Arg = FixOverloadedFunctionReference(Arg, FoundResult, Fn);
        ArgType = Arg->getType();
      } else
        return ExprError();
    }

    if (!ParamType->isMemberPointerType()) {
      if (CheckTemplateArgumentAddressOfObjectOrFunction(*this, Param,
                                                         ParamType,
                                                         Arg, Converted))
        return ExprError();
      return Arg;
    }

    if (CheckTemplateArgumentPointerToMember(*this, Param, ParamType, Arg,
                                             Converted))
      return ExprError();
    return Arg;
  }

  if (ParamType->isPointerType()) {
    //   -- for a non-type template-parameter of type pointer to
    //      object, qualification conversions (4.4) and the
    //      array-to-pointer conversion (4.2) are applied.
    // C++0x also allows a value of std::nullptr_t.
    assert(ParamType->getPointeeType()->isIncompleteOrObjectType() &&
           "Only object pointers allowed here");

    if (CheckTemplateArgumentAddressOfObjectOrFunction(*this, Param,
                                                       ParamType,
                                                       Arg, Converted))
      return ExprError();
    return Arg;
  }

  if (const ReferenceType *ParamRefType = ParamType->getAs<ReferenceType>()) {
    //   -- For a non-type template-parameter of type reference to
    //      object, no conversions apply. The type referred to by the
    //      reference may be more cv-qualified than the (otherwise
    //      identical) type of the template-argument. The
    //      template-parameter is bound directly to the
    //      template-argument, which must be an lvalue.
    assert(ParamRefType->getPointeeType()->isIncompleteOrObjectType() &&
           "Only object references allowed here");

    if (Arg->getType() == Context.OverloadTy) {
      if (FunctionDecl *Fn = ResolveAddressOfOverloadedFunction(Arg,
                                                 ParamRefType->getPointeeType(),
                                                                true,
                                                                FoundResult)) {
        if (DiagnoseUseOfDecl(Fn, Arg->getLocStart()))
          return ExprError();

        Arg = FixOverloadedFunctionReference(Arg, FoundResult, Fn);
        ArgType = Arg->getType();
      } else
        return ExprError();
    }

    if (CheckTemplateArgumentAddressOfObjectOrFunction(*this, Param,
                                                       ParamType,
                                                       Arg, Converted))
      return ExprError();
    return Arg;
  }

  // Deal with parameters of type std::nullptr_t.
  if (ParamType->isNullPtrType()) {
    if (Arg->isTypeDependent() || Arg->isValueDependent()) {
      Converted = TemplateArgument(Arg);
      return Arg;
    }
    
    switch (isNullPointerValueTemplateArgument(*this, Param, ParamType, Arg)) {
    case NPV_NotNullPointer:
      Diag(Arg->getExprLoc(), diag::err_template_arg_not_convertible)
        << Arg->getType() << ParamType;
      Diag(Param->getLocation(), diag::note_template_param_here);
      return ExprError();
      
    case NPV_Error:
      return ExprError();
      
    case NPV_NullPointer:
      Diag(Arg->getExprLoc(), diag::warn_cxx98_compat_template_arg_null);
      Converted = TemplateArgument(Context.getCanonicalType(ParamType),
                                   /*isNullPtr*/true);
      return Arg;
    }
  }

  //     -- For a non-type template-parameter of type pointer to data
  //        member, qualification conversions (4.4) are applied.
  assert(ParamType->isMemberPointerType() && "Only pointers to members remain");

  if (CheckTemplateArgumentPointerToMember(*this, Param, ParamType, Arg,
                                           Converted))
    return ExprError();
  return Arg;
}

/// \brief Check a template argument against its corresponding
/// template template parameter.
///
/// This routine implements the semantics of C++ [temp.arg.template].
/// It returns true if an error occurred, and false otherwise.
bool Sema::CheckTemplateArgument(TemplateTemplateParmDecl *Param,
                                 TemplateArgumentLoc &Arg,
                                 unsigned ArgumentPackIndex) {
  TemplateName Name = Arg.getArgument().getAsTemplateOrTemplatePattern();
  TemplateDecl *Template = Name.getAsTemplateDecl();
  if (!Template) {
    // Any dependent template name is fine.
    assert(Name.isDependent() && "Non-dependent template isn't a declaration?");
    return false;
  }

  // C++0x [temp.arg.template]p1:
  //   A template-argument for a template template-parameter shall be
  //   the name of a class template or an alias template, expressed as an
  //   id-expression. When the template-argument names a class template, only
  //   primary class templates are considered when matching the
  //   template template argument with the corresponding parameter;
  //   partial specializations are not considered even if their
  //   parameter lists match that of the template template parameter.
  //
  // Note that we also allow template template parameters here, which
  // will happen when we are dealing with, e.g., class template
  // partial specializations.
  if (!isa<ClassTemplateDecl>(Template) &&
      !isa<TemplateTemplateParmDecl>(Template) &&
      !isa<TypeAliasTemplateDecl>(Template)) {
    assert(isa<FunctionTemplateDecl>(Template) &&
           "Only function templates are possible here");
    Diag(Arg.getLocation(), diag::err_template_arg_not_class_template);
    Diag(Template->getLocation(), diag::note_template_arg_refers_here_func)
      << Template;
  }

  TemplateParameterList *Params = Param->getTemplateParameters();
  if (Param->isExpandedParameterPack())
    Params = Param->getExpansionTemplateParameters(ArgumentPackIndex);

  return !TemplateParameterListsAreEqual(Template->getTemplateParameters(),
                                         Params,
                                         true,
                                         TPL_TemplateTemplateArgumentMatch,
                                         Arg.getLocation());
}

/// \brief Given a non-type template argument that refers to a
/// declaration and the type of its corresponding non-type template
/// parameter, produce an expression that properly refers to that
/// declaration.
ExprResult
Sema::BuildExpressionFromDeclTemplateArgument(const TemplateArgument &Arg,
                                              QualType ParamType,
                                              SourceLocation Loc) {
  // C++ [temp.param]p8:
  //
  //   A non-type template-parameter of type "array of T" or
  //   "function returning T" is adjusted to be of type "pointer to
  //   T" or "pointer to function returning T", respectively.
  if (ParamType->isArrayType())
    ParamType = Context.getArrayDecayedType(ParamType);
  else if (ParamType->isFunctionType())
    ParamType = Context.getPointerType(ParamType);

  // For a NULL non-type template argument, return nullptr casted to the
  // parameter's type.
  if (Arg.getKind() == TemplateArgument::NullPtr) {
    return ImpCastExprToType(
             new (Context) CXXNullPtrLiteralExpr(Context.NullPtrTy, Loc),
                             ParamType,
                             ParamType->getAs<MemberPointerType>()
                               ? CK_NullToMemberPointer
                               : CK_NullToPointer);
  }
  assert(Arg.getKind() == TemplateArgument::Declaration &&
         "Only declaration template arguments permitted here");

  ValueDecl *VD = cast<ValueDecl>(Arg.getAsDecl());

  if (VD->getDeclContext()->isRecord() &&
      (isa<CXXMethodDecl>(VD) || isa<FieldDecl>(VD) ||
       isa<IndirectFieldDecl>(VD))) {
    // If the value is a class member, we might have a pointer-to-member.
    // Determine whether the non-type template template parameter is of
    // pointer-to-member type. If so, we need to build an appropriate
    // expression for a pointer-to-member, since a "normal" DeclRefExpr
    // would refer to the member itself.
    if (ParamType->isMemberPointerType()) {
      QualType ClassType
        = Context.getTypeDeclType(cast<RecordDecl>(VD->getDeclContext()));
      NestedNameSpecifier *Qualifier
        = NestedNameSpecifier::Create(Context, nullptr, false,
                                      ClassType.getTypePtr());
      CXXScopeSpec SS;
      SS.MakeTrivial(Context, Qualifier, Loc);

      // The actual value-ness of this is unimportant, but for
      // internal consistency's sake, references to instance methods
      // are r-values.
      ExprValueKind VK = VK_LValue;
      if (isa<CXXMethodDecl>(VD) && cast<CXXMethodDecl>(VD)->isInstance())
        VK = VK_RValue;

      ExprResult RefExpr = BuildDeclRefExpr(VD,
                                            VD->getType().getNonReferenceType(),
                                            VK,
                                            Loc,
                                            &SS);
      if (RefExpr.isInvalid())
        return ExprError();

      RefExpr = CreateBuiltinUnaryOp(Loc, UO_AddrOf, RefExpr.get());

      // We might need to perform a trailing qualification conversion, since
      // the element type on the parameter could be more qualified than the
      // element type in the expression we constructed.
      bool ObjCLifetimeConversion;
      if (IsQualificationConversion(((Expr*) RefExpr.get())->getType(),
                                    ParamType.getUnqualifiedType(), false,
                                    ObjCLifetimeConversion))
        RefExpr = ImpCastExprToType(RefExpr.get(), ParamType.getUnqualifiedType(), CK_NoOp);

      assert(!RefExpr.isInvalid() &&
             Context.hasSameType(((Expr*) RefExpr.get())->getType(),
                                 ParamType.getUnqualifiedType()));
      return RefExpr;
    }
  }

  QualType T = VD->getType().getNonReferenceType();

  if (ParamType->isPointerType()) {
    // When the non-type template parameter is a pointer, take the
    // address of the declaration.
    ExprResult RefExpr = BuildDeclRefExpr(VD, T, VK_LValue, Loc);
    if (RefExpr.isInvalid())
      return ExprError();

    if (T->isFunctionType() || T->isArrayType()) {
      // Decay functions and arrays.
      RefExpr = DefaultFunctionArrayConversion(RefExpr.get());
      if (RefExpr.isInvalid())
        return ExprError();

      return RefExpr;
    }

    // Take the address of everything else
    return CreateBuiltinUnaryOp(Loc, UO_AddrOf, RefExpr.get());
  }

  ExprValueKind VK = VK_RValue;

  // If the non-type template parameter has reference type, qualify the
  // resulting declaration reference with the extra qualifiers on the
  // type that the reference refers to.
  if (const ReferenceType *TargetRef = ParamType->getAs<ReferenceType>()) {
    VK = VK_LValue;
    T = Context.getQualifiedType(T,
                              TargetRef->getPointeeType().getQualifiers());
  } else if (isa<FunctionDecl>(VD)) {
    // References to functions are always lvalues.
    VK = VK_LValue;
  }

  return BuildDeclRefExpr(VD, T, VK, Loc);
}

/// \brief Construct a new expression that refers to the given
/// integral template argument with the given source-location
/// information.
///
/// This routine takes care of the mapping from an integral template
/// argument (which may have any integral type) to the appropriate
/// literal value.
ExprResult
Sema::BuildExpressionFromIntegralTemplateArgument(const TemplateArgument &Arg,
                                                  SourceLocation Loc) {
  assert(Arg.getKind() == TemplateArgument::Integral &&
         "Operation is only valid for integral template arguments");
  QualType OrigT = Arg.getIntegralType();

  // If this is an enum type that we're instantiating, we need to use an integer
  // type the same size as the enumerator.  We don't want to build an
  // IntegerLiteral with enum type.  The integer type of an enum type can be of
  // any integral type with C++11 enum classes, make sure we create the right
  // type of literal for it.
  QualType T = OrigT;
  if (const EnumType *ET = OrigT->getAs<EnumType>())
    T = ET->getDecl()->getIntegerType();

  Expr *E;
  if (T->isAnyCharacterType()) {
    CharacterLiteral::CharacterKind Kind;
    if (T->isWideCharType())
      Kind = CharacterLiteral::Wide;
    else if (T->isChar16Type())
      Kind = CharacterLiteral::UTF16;
    else if (T->isChar32Type())
      Kind = CharacterLiteral::UTF32;
    else
      Kind = CharacterLiteral::Ascii;

    E = new (Context) CharacterLiteral(Arg.getAsIntegral().getZExtValue(),
                                       Kind, T, Loc);
  } else if (T->isBooleanType()) {
    E = new (Context) CXXBoolLiteralExpr(Arg.getAsIntegral().getBoolValue(),
                                         T, Loc);
  } else if (T->isNullPtrType()) {
    E = new (Context) CXXNullPtrLiteralExpr(Context.NullPtrTy, Loc);
  } else {
    E = IntegerLiteral::Create(Context, Arg.getAsIntegral(), T, Loc);
  }

  if (OrigT->isEnumeralType()) {
    // FIXME: This is a hack. We need a better way to handle substituted
    // non-type template parameters.
    E = CStyleCastExpr::Create(Context, OrigT, VK_RValue, CK_IntegralCast, E,
                               nullptr,
                               Context.getTrivialTypeSourceInfo(OrigT, Loc),
                               Loc, Loc);
  }
  
  return E;
}

/// \brief Match two template parameters within template parameter lists.
static bool MatchTemplateParameterKind(Sema &S, NamedDecl *New, NamedDecl *Old,
                                       bool Complain,
                                     Sema::TemplateParameterListEqualKind Kind,
                                       SourceLocation TemplateArgLoc) {
  // Check the actual kind (type, non-type, template).
  if (Old->getKind() != New->getKind()) {
    if (Complain) {
      unsigned NextDiag = diag::err_template_param_different_kind;
      if (TemplateArgLoc.isValid()) {
        S.Diag(TemplateArgLoc, diag::err_template_arg_template_params_mismatch);
        NextDiag = diag::note_template_param_different_kind;
      }
      S.Diag(New->getLocation(), NextDiag)
        << (Kind != Sema::TPL_TemplateMatch);
      S.Diag(Old->getLocation(), diag::note_template_prev_declaration)
        << (Kind != Sema::TPL_TemplateMatch);
    }

    return false;
  }

  // Check that both are parameter packs are neither are parameter packs.
  // However, if we are matching a template template argument to a
  // template template parameter, the template template parameter can have
  // a parameter pack where the template template argument does not.
  if (Old->isTemplateParameterPack() != New->isTemplateParameterPack() &&
      !(Kind == Sema::TPL_TemplateTemplateArgumentMatch &&
        Old->isTemplateParameterPack())) {
    if (Complain) {
      unsigned NextDiag = diag::err_template_parameter_pack_non_pack;
      if (TemplateArgLoc.isValid()) {
        S.Diag(TemplateArgLoc,
             diag::err_template_arg_template_params_mismatch);
        NextDiag = diag::note_template_parameter_pack_non_pack;
      }

      unsigned ParamKind = isa<TemplateTypeParmDecl>(New)? 0
                      : isa<NonTypeTemplateParmDecl>(New)? 1
                      : 2;
      S.Diag(New->getLocation(), NextDiag)
        << ParamKind << New->isParameterPack();
      S.Diag(Old->getLocation(), diag::note_template_parameter_pack_here)
        << ParamKind << Old->isParameterPack();
    }

    return false;
  }

  // For non-type template parameters, check the type of the parameter.
  if (NonTypeTemplateParmDecl *OldNTTP
                                    = dyn_cast<NonTypeTemplateParmDecl>(Old)) {
    NonTypeTemplateParmDecl *NewNTTP = cast<NonTypeTemplateParmDecl>(New);

    // If we are matching a template template argument to a template
    // template parameter and one of the non-type template parameter types
    // is dependent, then we must wait until template instantiation time
    // to actually compare the arguments.
    if (Kind == Sema::TPL_TemplateTemplateArgumentMatch &&
        (OldNTTP->getType()->isDependentType() ||
         NewNTTP->getType()->isDependentType()))
      return true;

    if (!S.Context.hasSameType(OldNTTP->getType(), NewNTTP->getType())) {
      if (Complain) {
        unsigned NextDiag = diag::err_template_nontype_parm_different_type;
        if (TemplateArgLoc.isValid()) {
          S.Diag(TemplateArgLoc,
                 diag::err_template_arg_template_params_mismatch);
          NextDiag = diag::note_template_nontype_parm_different_type;
        }
        S.Diag(NewNTTP->getLocation(), NextDiag)
          << NewNTTP->getType()
          << (Kind != Sema::TPL_TemplateMatch);
        S.Diag(OldNTTP->getLocation(),
               diag::note_template_nontype_parm_prev_declaration)
          << OldNTTP->getType();
      }

      return false;
    }

    return true;
  }

  // For template template parameters, check the template parameter types.
  // The template parameter lists of template template
  // parameters must agree.
  if (TemplateTemplateParmDecl *OldTTP
                                    = dyn_cast<TemplateTemplateParmDecl>(Old)) {
    TemplateTemplateParmDecl *NewTTP = cast<TemplateTemplateParmDecl>(New);
    return S.TemplateParameterListsAreEqual(NewTTP->getTemplateParameters(),
                                            OldTTP->getTemplateParameters(),
                                            Complain,
                                        (Kind == Sema::TPL_TemplateMatch
                                           ? Sema::TPL_TemplateTemplateParmMatch
                                           : Kind),
                                            TemplateArgLoc);
  }

  return true;
}

/// \brief Diagnose a known arity mismatch when comparing template argument
/// lists.
static
void DiagnoseTemplateParameterListArityMismatch(Sema &S,
                                                TemplateParameterList *New,
                                                TemplateParameterList *Old,
                                      Sema::TemplateParameterListEqualKind Kind,
                                                SourceLocation TemplateArgLoc) {
  unsigned NextDiag = diag::err_template_param_list_different_arity;
  if (TemplateArgLoc.isValid()) {
    S.Diag(TemplateArgLoc, diag::err_template_arg_template_params_mismatch);
    NextDiag = diag::note_template_param_list_different_arity;
  }
  S.Diag(New->getTemplateLoc(), NextDiag)
    << (New->size() > Old->size())
    << (Kind != Sema::TPL_TemplateMatch)
    << SourceRange(New->getTemplateLoc(), New->getRAngleLoc());
  S.Diag(Old->getTemplateLoc(), diag::note_template_prev_declaration)
    << (Kind != Sema::TPL_TemplateMatch)
    << SourceRange(Old->getTemplateLoc(), Old->getRAngleLoc());
}

/// \brief Determine whether the given template parameter lists are
/// equivalent.
///
/// \param New  The new template parameter list, typically written in the
/// source code as part of a new template declaration.
///
/// \param Old  The old template parameter list, typically found via
/// name lookup of the template declared with this template parameter
/// list.
///
/// \param Complain  If true, this routine will produce a diagnostic if
/// the template parameter lists are not equivalent.
///
/// \param Kind describes how we are to match the template parameter lists.
///
/// \param TemplateArgLoc If this source location is valid, then we
/// are actually checking the template parameter list of a template
/// argument (New) against the template parameter list of its
/// corresponding template template parameter (Old). We produce
/// slightly different diagnostics in this scenario.
///
/// \returns True if the template parameter lists are equal, false
/// otherwise.
bool
Sema::TemplateParameterListsAreEqual(TemplateParameterList *New,
                                     TemplateParameterList *Old,
                                     bool Complain,
                                     TemplateParameterListEqualKind Kind,
                                     SourceLocation TemplateArgLoc) {
  if (Old->size() != New->size() && Kind != TPL_TemplateTemplateArgumentMatch) {
    if (Complain)
      DiagnoseTemplateParameterListArityMismatch(*this, New, Old, Kind,
                                                 TemplateArgLoc);

    return false;
  }

  // C++0x [temp.arg.template]p3:
  //   A template-argument matches a template template-parameter (call it P)
  //   when each of the template parameters in the template-parameter-list of
  //   the template-argument's corresponding class template or alias template
  //   (call it A) matches the corresponding template parameter in the
  //   template-parameter-list of P. [...]
  TemplateParameterList::iterator NewParm = New->begin();
  TemplateParameterList::iterator NewParmEnd = New->end();
  for (TemplateParameterList::iterator OldParm = Old->begin(),
                                    OldParmEnd = Old->end();
       OldParm != OldParmEnd; ++OldParm) {
    if (Kind != TPL_TemplateTemplateArgumentMatch ||
        !(*OldParm)->isTemplateParameterPack()) {
      if (NewParm == NewParmEnd) {
        if (Complain)
          DiagnoseTemplateParameterListArityMismatch(*this, New, Old, Kind,
                                                     TemplateArgLoc);

        return false;
      }

      if (!MatchTemplateParameterKind(*this, *NewParm, *OldParm, Complain,
                                      Kind, TemplateArgLoc))
        return false;

      ++NewParm;
      continue;
    }

    // C++0x [temp.arg.template]p3:
    //   [...] When P's template- parameter-list contains a template parameter
    //   pack (14.5.3), the template parameter pack will match zero or more
    //   template parameters or template parameter packs in the
    //   template-parameter-list of A with the same type and form as the
    //   template parameter pack in P (ignoring whether those template
    //   parameters are template parameter packs).
    for (; NewParm != NewParmEnd; ++NewParm) {
      if (!MatchTemplateParameterKind(*this, *NewParm, *OldParm, Complain,
                                      Kind, TemplateArgLoc))
        return false;
    }
  }

  // Make sure we exhausted all of the arguments.
  if (NewParm != NewParmEnd) {
    if (Complain)
      DiagnoseTemplateParameterListArityMismatch(*this, New, Old, Kind,
                                                 TemplateArgLoc);

    return false;
  }

  return true;
}

/// \brief Check whether a template can be declared within this scope.
///
/// If the template declaration is valid in this scope, returns
/// false. Otherwise, issues a diagnostic and returns true.
bool
Sema::CheckTemplateDeclScope(Scope *S, TemplateParameterList *TemplateParams) {
  if (!S)
    return false;

  // Find the nearest enclosing declaration scope.
  while ((S->getFlags() & Scope::DeclScope) == 0 ||
         (S->getFlags() & Scope::TemplateParamScope) != 0)
    S = S->getParent();

  // C++ [temp]p4:
  //   A template [...] shall not have C linkage.
  DeclContext *Ctx = S->getEntity();
  if (Ctx && Ctx->isExternCContext())
    return Diag(TemplateParams->getTemplateLoc(), diag::err_template_linkage)
             << TemplateParams->getSourceRange();

  while (Ctx && isa<LinkageSpecDecl>(Ctx))
    Ctx = Ctx->getParent();

  // C++ [temp]p2:
  //   A template-declaration can appear only as a namespace scope or
  //   class scope declaration.
  if (Ctx) {
    if (Ctx->isFileContext())
      return false;
    if (CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(Ctx)) {
      // C++ [temp.mem]p2:
      //   A local class shall not have member templates.
      if (RD->isLocalClass())
        return Diag(TemplateParams->getTemplateLoc(),
                    diag::err_template_inside_local_class)
          << TemplateParams->getSourceRange();
      else
        return false;
    }
  }

  return Diag(TemplateParams->getTemplateLoc(),
              diag::err_template_outside_namespace_or_class_scope)
    << TemplateParams->getSourceRange();
}

/// \brief Determine what kind of template specialization the given declaration
/// is.
static TemplateSpecializationKind getTemplateSpecializationKind(Decl *D) {
  if (!D)
    return TSK_Undeclared;

  if (CXXRecordDecl *Record = dyn_cast<CXXRecordDecl>(D))
    return Record->getTemplateSpecializationKind();
  if (FunctionDecl *Function = dyn_cast<FunctionDecl>(D))
    return Function->getTemplateSpecializationKind();
  if (VarDecl *Var = dyn_cast<VarDecl>(D))
    return Var->getTemplateSpecializationKind();

  return TSK_Undeclared;
}

/// \brief Check whether a specialization is well-formed in the current
/// context.
///
/// This routine determines whether a template specialization can be declared
/// in the current context (C++ [temp.expl.spec]p2).
///
/// \param S the semantic analysis object for which this check is being
/// performed.
///
/// \param Specialized the entity being specialized or instantiated, which
/// may be a kind of template (class template, function template, etc.) or
/// a member of a class template (member function, static data member,
/// member class).
///
/// \param PrevDecl the previous declaration of this entity, if any.
///
/// \param Loc the location of the explicit specialization or instantiation of
/// this entity.
///
/// \param IsPartialSpecialization whether this is a partial specialization of
/// a class template.
///
/// \returns true if there was an error that we cannot recover from, false
/// otherwise.
static bool CheckTemplateSpecializationScope(Sema &S,
                                             NamedDecl *Specialized,
                                             NamedDecl *PrevDecl,
                                             SourceLocation Loc,
                                             bool IsPartialSpecialization) {
  // Keep these "kind" numbers in sync with the %select statements in the
  // various diagnostics emitted by this routine.
  int EntityKind = 0;
  if (isa<ClassTemplateDecl>(Specialized))
    EntityKind = IsPartialSpecialization? 1 : 0;
  else if (isa<VarTemplateDecl>(Specialized))
    EntityKind = IsPartialSpecialization ? 3 : 2;
  else if (isa<FunctionTemplateDecl>(Specialized))
    EntityKind = 4;
  else if (isa<CXXMethodDecl>(Specialized))
    EntityKind = 5;
  else if (isa<VarDecl>(Specialized))
    EntityKind = 6;
  else if (isa<RecordDecl>(Specialized))
    EntityKind = 7;
  else if (isa<EnumDecl>(Specialized) && S.getLangOpts().CPlusPlus11)
    EntityKind = 8;
  else {
    S.Diag(Loc, diag::err_template_spec_unknown_kind)
      << S.getLangOpts().CPlusPlus11;
    S.Diag(Specialized->getLocation(), diag::note_specialized_entity);
    return true;
  }

  // C++ [temp.expl.spec]p2:
  //   An explicit specialization shall be declared in the namespace
  //   of which the template is a member, or, for member templates, in
  //   the namespace of which the enclosing class or enclosing class
  //   template is a member. An explicit specialization of a member
  //   function, member class or static data member of a class
  //   template shall be declared in the namespace of which the class
  //   template is a member. Such a declaration may also be a
  //   definition. If the declaration is not a definition, the
  //   specialization may be defined later in the name- space in which
  //   the explicit specialization was declared, or in a namespace
  //   that encloses the one in which the explicit specialization was
  //   declared.
  if (S.CurContext->getRedeclContext()->isFunctionOrMethod()) {
    S.Diag(Loc, diag::err_template_spec_decl_function_scope)
      << Specialized;
    return true;
  }

  if (S.CurContext->isRecord() && !IsPartialSpecialization) {
    if (S.getLangOpts().MicrosoftExt) {
      // Do not warn for class scope explicit specialization during
      // instantiation, warning was already emitted during pattern
      // semantic analysis.
      if (!S.ActiveTemplateInstantiations.size())
        S.Diag(Loc, diag::ext_function_specialization_in_class)
          << Specialized;
    } else {
      S.Diag(Loc, diag::err_template_spec_decl_class_scope)
        << Specialized;
      return true;
    }
  }

  if (S.CurContext->isRecord() &&
      !S.CurContext->Equals(Specialized->getDeclContext())) {
    // Make sure that we're specializing in the right record context.
    // Otherwise, things can go horribly wrong.
    S.Diag(Loc, diag::err_template_spec_decl_class_scope)
      << Specialized;
    return true;
  }
  
  // C++ [temp.class.spec]p6:
  //   A class template partial specialization may be declared or redeclared
  //   in any namespace scope in which its definition may be defined (14.5.1
  //   and 14.5.2).
  DeclContext *SpecializedContext
    = Specialized->getDeclContext()->getEnclosingNamespaceContext();
  DeclContext *DC = S.CurContext->getEnclosingNamespaceContext();

  // Make sure that this redeclaration (or definition) occurs in an enclosing
  // namespace.
  // Note that HandleDeclarator() performs this check for explicit
  // specializations of function templates, static data members, and member
  // functions, so we skip the check here for those kinds of entities.
  // FIXME: HandleDeclarator's diagnostics aren't quite as good, though.
  // Should we refactor that check, so that it occurs later?
  if (!DC->Encloses(SpecializedContext) &&
      !(isa<FunctionTemplateDecl>(Specialized) ||
        isa<FunctionDecl>(Specialized) ||
        isa<VarTemplateDecl>(Specialized) ||
        isa<VarDecl>(Specialized))) {
    if (isa<TranslationUnitDecl>(SpecializedContext))
      S.Diag(Loc, diag::err_template_spec_redecl_global_scope)
        << EntityKind << Specialized;
    else if (isa<NamespaceDecl>(SpecializedContext)) {
      int Diag = diag::err_template_spec_redecl_out_of_scope;
      if (S.getLangOpts().MicrosoftExt)
        Diag = diag::ext_ms_template_spec_redecl_out_of_scope;
      S.Diag(Loc, Diag) << EntityKind << Specialized
                        << cast<NamedDecl>(SpecializedContext);
    } else
      llvm_unreachable("unexpected namespace context for specialization");

    S.Diag(Specialized->getLocation(), diag::note_specialized_entity);
  } else if ((!PrevDecl ||
              getTemplateSpecializationKind(PrevDecl) == TSK_Undeclared ||
              getTemplateSpecializationKind(PrevDecl) ==
                  TSK_ImplicitInstantiation)) {
    // C++ [temp.exp.spec]p2:
    //   An explicit specialization shall be declared in the namespace of which
    //   the template is a member, or, for member templates, in the namespace
    //   of which the enclosing class or enclosing class template is a member.
    //   An explicit specialization of a member function, member class or
    //   static data member of a class template shall be declared in the
    //   namespace of which the class template is a member.
    //
    // C++11 [temp.expl.spec]p2:
    //   An explicit specialization shall be declared in a namespace enclosing
    //   the specialized template.
    // C++11 [temp.explicit]p3:
    //   An explicit instantiation shall appear in an enclosing namespace of its
    //   template.
    if (!DC->InEnclosingNamespaceSetOf(SpecializedContext)) {
      bool IsCPlusPlus11Extension = DC->Encloses(SpecializedContext);
      if (isa<TranslationUnitDecl>(SpecializedContext)) {
        assert(!IsCPlusPlus11Extension &&
               "DC encloses TU but isn't in enclosing namespace set");
        S.Diag(Loc, diag::err_template_spec_decl_out_of_scope_global)
          << EntityKind << Specialized;
      } else if (isa<NamespaceDecl>(SpecializedContext)) {
        int Diag;
        if (!IsCPlusPlus11Extension)
          Diag = diag::err_template_spec_decl_out_of_scope;
        else if (!S.getLangOpts().CPlusPlus11)
          Diag = diag::ext_template_spec_decl_out_of_scope;
        else
          Diag = diag::warn_cxx98_compat_template_spec_decl_out_of_scope;
        S.Diag(Loc, Diag)
          << EntityKind << Specialized << cast<NamedDecl>(SpecializedContext);
      }

      S.Diag(Specialized->getLocation(), diag::note_specialized_entity);
    }
  }

  return false;
}

static SourceRange findTemplateParameter(unsigned Depth, Expr *E) {
  if (!E->isInstantiationDependent())
    return SourceLocation();
  DependencyChecker Checker(Depth);
  Checker.TraverseStmt(E);
  if (Checker.Match && Checker.MatchLoc.isInvalid())
    return E->getSourceRange();
  return Checker.MatchLoc;
}

static SourceRange findTemplateParameter(unsigned Depth, TypeLoc TL) {
  if (!TL.getType()->isDependentType())
    return SourceLocation();
  DependencyChecker Checker(Depth);
  Checker.TraverseTypeLoc(TL);
  if (Checker.Match && Checker.MatchLoc.isInvalid())
    return TL.getSourceRange();
  return Checker.MatchLoc;
}

/// \brief Subroutine of Sema::CheckTemplatePartialSpecializationArgs
/// that checks non-type template partial specialization arguments.
static bool CheckNonTypeTemplatePartialSpecializationArgs(
    Sema &S, SourceLocation TemplateNameLoc, NonTypeTemplateParmDecl *Param,
    const TemplateArgument *Args, unsigned NumArgs, bool IsDefaultArgument) {
  for (unsigned I = 0; I != NumArgs; ++I) {
    if (Args[I].getKind() == TemplateArgument::Pack) {
      if (CheckNonTypeTemplatePartialSpecializationArgs(
              S, TemplateNameLoc, Param, Args[I].pack_begin(),
              Args[I].pack_size(), IsDefaultArgument))
        return true;

      continue;
    }

    if (Args[I].getKind() != TemplateArgument::Expression)
      continue;

    Expr *ArgExpr = Args[I].getAsExpr();

    // We can have a pack expansion of any of the bullets below.
    if (PackExpansionExpr *Expansion = dyn_cast<PackExpansionExpr>(ArgExpr))
      ArgExpr = Expansion->getPattern();

    // Strip off any implicit casts we added as part of type checking.
    while (ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(ArgExpr))
      ArgExpr = ICE->getSubExpr();

    // C++ [temp.class.spec]p8:
    //   A non-type argument is non-specialized if it is the name of a
    //   non-type parameter. All other non-type arguments are
    //   specialized.
    //
    // Below, we check the two conditions that only apply to
    // specialized non-type arguments, so skip any non-specialized
    // arguments.
    if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(ArgExpr))
      if (isa<NonTypeTemplateParmDecl>(DRE->getDecl()))
        continue;

    // C++ [temp.class.spec]p9:
    //   Within the argument list of a class template partial
    //   specialization, the following restrictions apply:
    //     -- A partially specialized non-type argument expression
    //        shall not involve a template parameter of the partial
    //        specialization except when the argument expression is a
    //        simple identifier.
    SourceRange ParamUseRange =
        findTemplateParameter(Param->getDepth(), ArgExpr);
    if (ParamUseRange.isValid()) {
      if (IsDefaultArgument) {
        S.Diag(TemplateNameLoc,
               diag::err_dependent_non_type_arg_in_partial_spec);
        S.Diag(ParamUseRange.getBegin(),
               diag::note_dependent_non_type_default_arg_in_partial_spec)
          << ParamUseRange;
      } else {
        S.Diag(ParamUseRange.getBegin(),
               diag::err_dependent_non_type_arg_in_partial_spec)
          << ParamUseRange;
      }
      return true;
    }

    //     -- The type of a template parameter corresponding to a
    //        specialized non-type argument shall not be dependent on a
    //        parameter of the specialization.
    //
    // FIXME: We need to delay this check until instantiation in some cases:
    //
    //   template<template<typename> class X> struct A {
    //     template<typename T, X<T> N> struct B;
    //     template<typename T> struct B<T, 0>;
    //   };
    //   template<typename> using X = int;
    //   A<X>::B<int, 0> b;
    ParamUseRange = findTemplateParameter(
            Param->getDepth(), Param->getTypeSourceInfo()->getTypeLoc());
    if (ParamUseRange.isValid()) {
      S.Diag(IsDefaultArgument ? TemplateNameLoc : ArgExpr->getLocStart(),
             diag::err_dependent_typed_non_type_arg_in_partial_spec)
        << Param->getType() << ParamUseRange;
      S.Diag(Param->getLocation(), diag::note_template_param_here)
        << (IsDefaultArgument ? ParamUseRange : SourceRange());
      return true;
    }
  }

  return false;
}

/// \brief Check the non-type template arguments of a class template
/// partial specialization according to C++ [temp.class.spec]p9.
///
/// \param TemplateNameLoc the location of the template name.
/// \param TemplateParams the template parameters of the primary class
///        template.
/// \param NumExplicit the number of explicitly-specified template arguments.
/// \param TemplateArgs the template arguments of the class template
///        partial specialization.
///
/// \returns \c true if there was an error, \c false otherwise.
static bool CheckTemplatePartialSpecializationArgs(
    Sema &S, SourceLocation TemplateNameLoc,
    TemplateParameterList *TemplateParams, unsigned NumExplicit,
    SmallVectorImpl<TemplateArgument> &TemplateArgs) {
  const TemplateArgument *ArgList = TemplateArgs.data();

  for (unsigned I = 0, N = TemplateParams->size(); I != N; ++I) {
    NonTypeTemplateParmDecl *Param
      = dyn_cast<NonTypeTemplateParmDecl>(TemplateParams->getParam(I));
    if (!Param)
      continue;

    if (CheckNonTypeTemplatePartialSpecializationArgs(
            S, TemplateNameLoc, Param, &ArgList[I], 1, I >= NumExplicit))
      return true;
  }

  return false;
}

DeclResult
Sema::ActOnClassTemplateSpecialization(Scope *S, unsigned TagSpec,
                                       TagUseKind TUK,
                                       SourceLocation KWLoc,
                                       SourceLocation ModulePrivateLoc,
                                       TemplateIdAnnotation &TemplateId,
                                       AttributeList *Attr,
                                       MultiTemplateParamsArg
                                           TemplateParameterLists,
                                       SkipBodyInfo *SkipBody) {
  assert(TUK != TUK_Reference && "References are not specializations");

  CXXScopeSpec &SS = TemplateId.SS;

  // NOTE: KWLoc is the location of the tag keyword. This will instead
  // store the location of the outermost template keyword in the declaration.
  SourceLocation TemplateKWLoc = TemplateParameterLists.size() > 0
    ? TemplateParameterLists[0]->getTemplateLoc() : KWLoc;
  SourceLocation TemplateNameLoc = TemplateId.TemplateNameLoc;
  SourceLocation LAngleLoc = TemplateId.LAngleLoc;
  SourceLocation RAngleLoc = TemplateId.RAngleLoc;

  // Find the class template we're specializing
  TemplateName Name = TemplateId.Template.get();
  ClassTemplateDecl *ClassTemplate
    = dyn_cast_or_null<ClassTemplateDecl>(Name.getAsTemplateDecl());

  if (!ClassTemplate) {
    Diag(TemplateNameLoc, diag::err_not_class_template_specialization)
      << (Name.getAsTemplateDecl() &&
          isa<TemplateTemplateParmDecl>(Name.getAsTemplateDecl()));
    return true;
  }

  bool isExplicitSpecialization = false;
  bool isPartialSpecialization = false;

  // Check the validity of the template headers that introduce this
  // template.
  // FIXME: We probably shouldn't complain about these headers for
  // friend declarations.
  bool Invalid = false;
  TemplateParameterList *TemplateParams =
      MatchTemplateParametersToScopeSpecifier(
          KWLoc, TemplateNameLoc, SS, &TemplateId,
          TemplateParameterLists, TUK == TUK_Friend, isExplicitSpecialization,
          Invalid);
  if (Invalid)
    return true;

  if (TemplateParams && TemplateParams->size() > 0) {
    isPartialSpecialization = true;

    if (TUK == TUK_Friend) {
      Diag(KWLoc, diag::err_partial_specialization_friend)
        << SourceRange(LAngleLoc, RAngleLoc);
      return true;
    }

    // C++ [temp.class.spec]p10:
    //   The template parameter list of a specialization shall not
    //   contain default template argument values.
    for (unsigned I = 0, N = TemplateParams->size(); I != N; ++I) {
      Decl *Param = TemplateParams->getParam(I);
      if (TemplateTypeParmDecl *TTP = dyn_cast<TemplateTypeParmDecl>(Param)) {
        if (TTP->hasDefaultArgument()) {
          Diag(TTP->getDefaultArgumentLoc(),
               diag::err_default_arg_in_partial_spec);
          TTP->removeDefaultArgument();
        }
      } else if (NonTypeTemplateParmDecl *NTTP
                   = dyn_cast<NonTypeTemplateParmDecl>(Param)) {
        if (Expr *DefArg = NTTP->getDefaultArgument()) {
          Diag(NTTP->getDefaultArgumentLoc(),
               diag::err_default_arg_in_partial_spec)
            << DefArg->getSourceRange();
          NTTP->removeDefaultArgument();
        }
      } else {
        TemplateTemplateParmDecl *TTP = cast<TemplateTemplateParmDecl>(Param);
        if (TTP->hasDefaultArgument()) {
          Diag(TTP->getDefaultArgument().getLocation(),
               diag::err_default_arg_in_partial_spec)
            << TTP->getDefaultArgument().getSourceRange();
          TTP->removeDefaultArgument();
        }
      }
    }
  } else if (TemplateParams) {
    if (TUK == TUK_Friend)
      Diag(KWLoc, diag::err_template_spec_friend)
        << FixItHint::CreateRemoval(
                                SourceRange(TemplateParams->getTemplateLoc(),
                                            TemplateParams->getRAngleLoc()))
        << SourceRange(LAngleLoc, RAngleLoc);
    else
      isExplicitSpecialization = true;
  } else {
    assert(TUK == TUK_Friend && "should have a 'template<>' for this decl");
  }

  // Check that the specialization uses the same tag kind as the
  // original template.
  TagTypeKind Kind = TypeWithKeyword::getTagTypeKindForTypeSpec(TagSpec);
  assert(Kind != TTK_Enum && "Invalid enum tag in class template spec!");
  if (!isAcceptableTagRedeclaration(ClassTemplate->getTemplatedDecl(),
                                    Kind, TUK == TUK_Definition, KWLoc,
                                    ClassTemplate->getIdentifier())) {
    Diag(KWLoc, diag::err_use_with_wrong_tag)
      << ClassTemplate
      << FixItHint::CreateReplacement(KWLoc,
                            ClassTemplate->getTemplatedDecl()->getKindName());
    Diag(ClassTemplate->getTemplatedDecl()->getLocation(),
         diag::note_previous_use);
    Kind = ClassTemplate->getTemplatedDecl()->getTagKind();
  }

  // Translate the parser's template argument list in our AST format.
  TemplateArgumentListInfo TemplateArgs =
      makeTemplateArgumentListInfo(*this, TemplateId);

  // Check for unexpanded parameter packs in any of the template arguments.
  for (unsigned I = 0, N = TemplateArgs.size(); I != N; ++I)
    if (DiagnoseUnexpandedParameterPack(TemplateArgs[I],
                                        UPPC_PartialSpecialization))
      return true;

  // Check that the template argument list is well-formed for this
  // template.
  SmallVector<TemplateArgument, 4> Converted;
  if (CheckTemplateArgumentList(ClassTemplate, TemplateNameLoc,
                                TemplateArgs, false, Converted))
    return true;

  // Find the class template (partial) specialization declaration that
  // corresponds to these arguments.
  if (isPartialSpecialization) {
    if (CheckTemplatePartialSpecializationArgs(
            *this, TemplateNameLoc, ClassTemplate->getTemplateParameters(),
            TemplateArgs.size(), Converted))
      return true;

    bool InstantiationDependent;
    if (!Name.isDependent() &&
        !TemplateSpecializationType::anyDependentTemplateArguments(
                                             TemplateArgs.getArgumentArray(),
                                                         TemplateArgs.size(),
                                                     InstantiationDependent)) {
      Diag(TemplateNameLoc, diag::err_partial_spec_fully_specialized)
        << ClassTemplate->getDeclName();
      isPartialSpecialization = false;
    }
  }

  void *InsertPos = nullptr;
  ClassTemplateSpecializationDecl *PrevDecl = nullptr;

  if (isPartialSpecialization)
    // FIXME: Template parameter list matters, too
    PrevDecl = ClassTemplate->findPartialSpecialization(Converted, InsertPos);
  else
    PrevDecl = ClassTemplate->findSpecialization(Converted, InsertPos);

  ClassTemplateSpecializationDecl *Specialization = nullptr;

  // Check whether we can declare a class template specialization in
  // the current scope.
  if (TUK != TUK_Friend &&
      CheckTemplateSpecializationScope(*this, ClassTemplate, PrevDecl,
                                       TemplateNameLoc,
                                       isPartialSpecialization))
    return true;

  // The canonical type
  QualType CanonType;
  if (isPartialSpecialization) {
    // Build the canonical type that describes the converted template
    // arguments of the class template partial specialization.
    TemplateName CanonTemplate = Context.getCanonicalTemplateName(Name);
    CanonType = Context.getTemplateSpecializationType(CanonTemplate,
                                                      Converted.data(),
                                                      Converted.size());

    if (Context.hasSameType(CanonType,
                        ClassTemplate->getInjectedClassNameSpecialization())) {
      // C++ [temp.class.spec]p9b3:
      //
      //   -- The argument list of the specialization shall not be identical
      //      to the implicit argument list of the primary template.
      Diag(TemplateNameLoc, diag::err_partial_spec_args_match_primary_template)
        << /*class template*/0 << (TUK == TUK_Definition)
        << FixItHint::CreateRemoval(SourceRange(LAngleLoc, RAngleLoc));
      return CheckClassTemplate(S, TagSpec, TUK, KWLoc, SS,
                                ClassTemplate->getIdentifier(),
                                TemplateNameLoc,
                                Attr,
                                TemplateParams,
                                AS_none, /*ModulePrivateLoc=*/SourceLocation(),
                                /*FriendLoc*/SourceLocation(),
                                TemplateParameterLists.size() - 1,
                                TemplateParameterLists.data());
    }

    // Create a new class template partial specialization declaration node.
    ClassTemplatePartialSpecializationDecl *PrevPartial
      = cast_or_null<ClassTemplatePartialSpecializationDecl>(PrevDecl);
    ClassTemplatePartialSpecializationDecl *Partial
      = ClassTemplatePartialSpecializationDecl::Create(Context, Kind,
                                             ClassTemplate->getDeclContext(),
                                                       KWLoc, TemplateNameLoc,
                                                       TemplateParams,
                                                       ClassTemplate,
                                                       Converted.data(),
                                                       Converted.size(),
                                                       TemplateArgs,
                                                       CanonType,
                                                       PrevPartial);
    SetNestedNameSpecifier(Partial, SS);
    if (TemplateParameterLists.size() > 1 && SS.isSet()) {
      Partial->setTemplateParameterListsInfo(Context,
                                             TemplateParameterLists.size() - 1,
                                             TemplateParameterLists.data());
    }

    if (!PrevPartial)
      ClassTemplate->AddPartialSpecialization(Partial, InsertPos);
    Specialization = Partial;

    // If we are providing an explicit specialization of a member class
    // template specialization, make a note of that.
    if (PrevPartial && PrevPartial->getInstantiatedFromMember())
      PrevPartial->setMemberSpecialization();

    // Check that all of the template parameters of the class template
    // partial specialization are deducible from the template
    // arguments. If not, this class template partial specialization
    // will never be used.
    llvm::SmallBitVector DeducibleParams(TemplateParams->size());
    MarkUsedTemplateParameters(Partial->getTemplateArgs(), true,
                               TemplateParams->getDepth(),
                               DeducibleParams);

    if (!DeducibleParams.all()) {
      unsigned NumNonDeducible = DeducibleParams.size()-DeducibleParams.count();
      Diag(TemplateNameLoc, diag::warn_partial_specs_not_deducible)
        << /*class template*/0 << (NumNonDeducible > 1)
        << SourceRange(TemplateNameLoc, RAngleLoc);
      for (unsigned I = 0, N = DeducibleParams.size(); I != N; ++I) {
        if (!DeducibleParams[I]) {
          NamedDecl *Param = cast<NamedDecl>(TemplateParams->getParam(I));
          if (Param->getDeclName())
            Diag(Param->getLocation(),
                 diag::note_partial_spec_unused_parameter)
              << Param->getDeclName();
          else
            Diag(Param->getLocation(),
                 diag::note_partial_spec_unused_parameter)
              << "(anonymous)";
        }
      }
    }
  } else {
    // Create a new class template specialization declaration node for
    // this explicit specialization or friend declaration.
    Specialization
      = ClassTemplateSpecializationDecl::Create(Context, Kind,
                                             ClassTemplate->getDeclContext(),
                                                KWLoc, TemplateNameLoc,
                                                ClassTemplate,
                                                Converted.data(),
                                                Converted.size(),
                                                PrevDecl);
    SetNestedNameSpecifier(Specialization, SS);
    if (TemplateParameterLists.size() > 0) {
      Specialization->setTemplateParameterListsInfo(Context,
                                              TemplateParameterLists.size(),
                                              TemplateParameterLists.data());
    }

    if (!PrevDecl)
      ClassTemplate->AddSpecialization(Specialization, InsertPos);

    CanonType = Context.getTypeDeclType(Specialization);
  }

  // C++ [temp.expl.spec]p6:
  //   If a template, a member template or the member of a class template is
  //   explicitly specialized then that specialization shall be declared
  //   before the first use of that specialization that would cause an implicit
  //   instantiation to take place, in every translation unit in which such a
  //   use occurs; no diagnostic is required.
  if (PrevDecl && PrevDecl->getPointOfInstantiation().isValid()) {
    bool Okay = false;
    for (Decl *Prev = PrevDecl; Prev; Prev = Prev->getPreviousDecl()) {
      // Is there any previous explicit specialization declaration?
      if (getTemplateSpecializationKind(Prev) == TSK_ExplicitSpecialization) {
        Okay = true;
        break;
      }
    }

    if (!Okay) {
      SourceRange Range(TemplateNameLoc, RAngleLoc);
      Diag(TemplateNameLoc, diag::err_specialization_after_instantiation)
        << Context.getTypeDeclType(Specialization) << Range;

      Diag(PrevDecl->getPointOfInstantiation(),
           diag::note_instantiation_required_here)
        << (PrevDecl->getTemplateSpecializationKind()
                                                != TSK_ImplicitInstantiation);
      return true;
    }
  }

  // If this is not a friend, note that this is an explicit specialization.
  if (TUK != TUK_Friend)
    Specialization->setSpecializationKind(TSK_ExplicitSpecialization);

  // Check that this isn't a redefinition of this specialization.
  if (TUK == TUK_Definition) {
    RecordDecl *Def = Specialization->getDefinition();
    NamedDecl *Hidden = nullptr;
    if (Def && SkipBody && !hasVisibleDefinition(Def, &Hidden)) {
      SkipBody->ShouldSkip = true;
      makeMergedDefinitionVisible(Hidden, KWLoc);
      // From here on out, treat this as just a redeclaration.
      TUK = TUK_Declaration;
    } else if (Def) {
      SourceRange Range(TemplateNameLoc, RAngleLoc);
      Diag(TemplateNameLoc, diag::err_redefinition)
        << Context.getTypeDeclType(Specialization) << Range;
      Diag(Def->getLocation(), diag::note_previous_definition);
      Specialization->setInvalidDecl();
      return true;
    }
  }

  if (Attr)
    ProcessDeclAttributeList(S, Specialization, Attr);

  // Add alignment attributes if necessary; these attributes are checked when
  // the ASTContext lays out the structure.
  if (TUK == TUK_Definition) {
    AddAlignmentAttributesForRecord(Specialization);
    AddMsStructLayoutForRecord(Specialization);
  }

  if (ModulePrivateLoc.isValid())
    Diag(Specialization->getLocation(), diag::err_module_private_specialization)
      << (isPartialSpecialization? 1 : 0)
      << FixItHint::CreateRemoval(ModulePrivateLoc);
  
  // Build the fully-sugared type for this class template
  // specialization as the user wrote in the specialization
  // itself. This means that we'll pretty-print the type retrieved
  // from the specialization's declaration the way that the user
  // actually wrote the specialization, rather than formatting the
  // name based on the "canonical" representation used to store the
  // template arguments in the specialization.
  TypeSourceInfo *WrittenTy
    = Context.getTemplateSpecializationTypeInfo(Name, TemplateNameLoc,
                                                TemplateArgs, CanonType);
  if (TUK != TUK_Friend) {
    Specialization->setTypeAsWritten(WrittenTy);
    Specialization->setTemplateKeywordLoc(TemplateKWLoc);
  }

  // C++ [temp.expl.spec]p9:
  //   A template explicit specialization is in the scope of the
  //   namespace in which the template was defined.
  //
  // We actually implement this paragraph where we set the semantic
  // context (in the creation of the ClassTemplateSpecializationDecl),
  // but we also maintain the lexical context where the actual
  // definition occurs.
  Specialization->setLexicalDeclContext(CurContext);

  // We may be starting the definition of this specialization.
  if (TUK == TUK_Definition)
    Specialization->startDefinition();

  if (TUK == TUK_Friend) {
    FriendDecl *Friend = FriendDecl::Create(Context, CurContext,
                                            TemplateNameLoc,
                                            WrittenTy,
                                            /*FIXME:*/KWLoc);
    Friend->setAccess(AS_public);
    CurContext->addDecl(Friend);
  } else {
    // Add the specialization into its lexical context, so that it can
    // be seen when iterating through the list of declarations in that
    // context. However, specializations are not found by name lookup.
    CurContext->addDecl(Specialization);
  }
  return Specialization;
}

Decl *Sema::ActOnTemplateDeclarator(Scope *S,
                              MultiTemplateParamsArg TemplateParameterLists,
                                    Declarator &D) {
  Decl *NewDecl = HandleDeclarator(S, D, TemplateParameterLists);
  ActOnDocumentableDecl(NewDecl);
  return NewDecl;
}

Decl *Sema::ActOnStartOfFunctionTemplateDef(Scope *FnBodyScope,
                               MultiTemplateParamsArg TemplateParameterLists,
                                            Declarator &D) {
  assert(getCurFunctionDecl() == nullptr && "Function parsing confused");
  DeclaratorChunk::FunctionTypeInfo &FTI = D.getFunctionTypeInfo();

  if (FTI.hasPrototype) {
    // FIXME: Diagnose arguments without names in C.
  }

  Scope *ParentScope = FnBodyScope->getParent();

  D.setFunctionDefinitionKind(FDK_Definition);
  Decl *DP = HandleDeclarator(ParentScope, D,
                              TemplateParameterLists);
  return ActOnStartOfFunctionDef(FnBodyScope, DP);
}

/// \brief Strips various properties off an implicit instantiation
/// that has just been explicitly specialized.
static void StripImplicitInstantiation(NamedDecl *D) {
  D->dropAttr<DLLImportAttr>();
  D->dropAttr<DLLExportAttr>();

  if (FunctionDecl *FD = dyn_cast<FunctionDecl>(D))
    FD->setInlineSpecified(false);
}

/// \brief Compute the diagnostic location for an explicit instantiation
//  declaration or definition.
static SourceLocation DiagLocForExplicitInstantiation(
    NamedDecl* D, SourceLocation PointOfInstantiation) {
  // Explicit instantiations following a specialization have no effect and
  // hence no PointOfInstantiation. In that case, walk decl backwards
  // until a valid name loc is found.
  SourceLocation PrevDiagLoc = PointOfInstantiation;
  for (Decl *Prev = D; Prev && !PrevDiagLoc.isValid();
       Prev = Prev->getPreviousDecl()) {
    PrevDiagLoc = Prev->getLocation();
  }
  assert(PrevDiagLoc.isValid() &&
         "Explicit instantiation without point of instantiation?");
  return PrevDiagLoc;
}

/// \brief Diagnose cases where we have an explicit template specialization
/// before/after an explicit template instantiation, producing diagnostics
/// for those cases where they are required and determining whether the
/// new specialization/instantiation will have any effect.
///
/// \param NewLoc the location of the new explicit specialization or
/// instantiation.
///
/// \param NewTSK the kind of the new explicit specialization or instantiation.
///
/// \param PrevDecl the previous declaration of the entity.
///
/// \param PrevTSK the kind of the old explicit specialization or instantiatin.
///
/// \param PrevPointOfInstantiation if valid, indicates where the previus
/// declaration was instantiated (either implicitly or explicitly).
///
/// \param HasNoEffect will be set to true to indicate that the new
/// specialization or instantiation has no effect and should be ignored.
///
/// \returns true if there was an error that should prevent the introduction of
/// the new declaration into the AST, false otherwise.
bool
Sema::CheckSpecializationInstantiationRedecl(SourceLocation NewLoc,
                                             TemplateSpecializationKind NewTSK,
                                             NamedDecl *PrevDecl,
                                             TemplateSpecializationKind PrevTSK,
                                        SourceLocation PrevPointOfInstantiation,
                                             bool &HasNoEffect) {
  HasNoEffect = false;

  switch (NewTSK) {
  case TSK_Undeclared:
  case TSK_ImplicitInstantiation:
    assert(
        (PrevTSK == TSK_Undeclared || PrevTSK == TSK_ImplicitInstantiation) &&
        "previous declaration must be implicit!");
    return false;

  case TSK_ExplicitSpecialization:
    switch (PrevTSK) {
    case TSK_Undeclared:
    case TSK_ExplicitSpecialization:
      // Okay, we're just specializing something that is either already
      // explicitly specialized or has merely been mentioned without any
      // instantiation.
      return false;

    case TSK_ImplicitInstantiation:
      if (PrevPointOfInstantiation.isInvalid()) {
        // The declaration itself has not actually been instantiated, so it is
        // still okay to specialize it.
        StripImplicitInstantiation(PrevDecl);
        return false;
      }
      LLVM_FALLTHROUGH; // HLSL Change

    case TSK_ExplicitInstantiationDeclaration:
    case TSK_ExplicitInstantiationDefinition:
      assert((PrevTSK == TSK_ImplicitInstantiation ||
              PrevPointOfInstantiation.isValid()) &&
             "Explicit instantiation without point of instantiation?");

      // C++ [temp.expl.spec]p6:
      //   If a template, a member template or the member of a class template
      //   is explicitly specialized then that specialization shall be declared
      //   before the first use of that specialization that would cause an
      //   implicit instantiation to take place, in every translation unit in
      //   which such a use occurs; no diagnostic is required.
      for (Decl *Prev = PrevDecl; Prev; Prev = Prev->getPreviousDecl()) {
        // Is there any previous explicit specialization declaration?
        if (getTemplateSpecializationKind(Prev) == TSK_ExplicitSpecialization)
          return false;
      }

      Diag(NewLoc, diag::err_specialization_after_instantiation)
        << PrevDecl;
      Diag(PrevPointOfInstantiation, diag::note_instantiation_required_here)
        << (PrevTSK != TSK_ImplicitInstantiation);

      return true;
    }
    llvm_unreachable("Unrecognized PrevTSK!");

  case TSK_ExplicitInstantiationDeclaration:
    switch (PrevTSK) {
    case TSK_ExplicitInstantiationDeclaration:
      // This explicit instantiation declaration is redundant (that's okay).
      HasNoEffect = true;
      return false;

    case TSK_Undeclared:
    case TSK_ImplicitInstantiation:
      // We're explicitly instantiating something that may have already been
      // implicitly instantiated; that's fine.
      return false;

    case TSK_ExplicitSpecialization:
      // C++0x [temp.explicit]p4:
      //   For a given set of template parameters, if an explicit instantiation
      //   of a template appears after a declaration of an explicit
      //   specialization for that template, the explicit instantiation has no
      //   effect.
      HasNoEffect = true;
      return false;

    case TSK_ExplicitInstantiationDefinition:
      // C++0x [temp.explicit]p10:
      //   If an entity is the subject of both an explicit instantiation
      //   declaration and an explicit instantiation definition in the same
      //   translation unit, the definition shall follow the declaration.
      Diag(NewLoc,
           diag::err_explicit_instantiation_declaration_after_definition);

      // Explicit instantiations following a specialization have no effect and
      // hence no PrevPointOfInstantiation. In that case, walk decl backwards
      // until a valid name loc is found.
      Diag(DiagLocForExplicitInstantiation(PrevDecl, PrevPointOfInstantiation),
           diag::note_explicit_instantiation_definition_here);
      HasNoEffect = true;
      return false;
    }
    llvm_unreachable("Unrecognized PrevTSK!");

  case TSK_ExplicitInstantiationDefinition:
    switch (PrevTSK) {
    case TSK_Undeclared:
    case TSK_ImplicitInstantiation:
      // We're explicitly instantiating something that may have already been
      // implicitly instantiated; that's fine.
      return false;

    case TSK_ExplicitSpecialization:
      // C++ DR 259, C++0x [temp.explicit]p4:
      //   For a given set of template parameters, if an explicit
      //   instantiation of a template appears after a declaration of
      //   an explicit specialization for that template, the explicit
      //   instantiation has no effect.
      //
      // In C++98/03 mode, we only give an extension warning here, because it
      // is not harmful to try to explicitly instantiate something that
      // has been explicitly specialized.
      Diag(NewLoc, getLangOpts().CPlusPlus11 ?
           diag::warn_cxx98_compat_explicit_instantiation_after_specialization :
           diag::ext_explicit_instantiation_after_specialization)
        << PrevDecl;
      Diag(PrevDecl->getLocation(),
           diag::note_previous_template_specialization);
      HasNoEffect = true;
      return false;

    case TSK_ExplicitInstantiationDeclaration:
      // We're explicity instantiating a definition for something for which we
      // were previously asked to suppress instantiations. That's fine.

      // C++0x [temp.explicit]p4:
      //   For a given set of template parameters, if an explicit instantiation
      //   of a template appears after a declaration of an explicit
      //   specialization for that template, the explicit instantiation has no
      //   effect.
      for (Decl *Prev = PrevDecl; Prev; Prev = Prev->getPreviousDecl()) {
        // Is there any previous explicit specialization declaration?
        if (getTemplateSpecializationKind(Prev) == TSK_ExplicitSpecialization) {
          HasNoEffect = true;
          break;
        }
      }

      return false;

    case TSK_ExplicitInstantiationDefinition:
      // C++0x [temp.spec]p5:
      //   For a given template and a given set of template-arguments,
      //     - an explicit instantiation definition shall appear at most once
      //       in a program,

      // MSVCCompat: MSVC silently ignores duplicate explicit instantiations.
      Diag(NewLoc, (getLangOpts().MSVCCompat)
                       ? diag::ext_explicit_instantiation_duplicate
                       : diag::err_explicit_instantiation_duplicate)
          << PrevDecl;
      Diag(DiagLocForExplicitInstantiation(PrevDecl, PrevPointOfInstantiation),
           diag::note_previous_explicit_instantiation);
      HasNoEffect = true;
      return false;
    }
  }

  llvm_unreachable("Missing specialization/instantiation case?");
}

/// \brief Perform semantic analysis for the given dependent function
/// template specialization.
///
/// The only possible way to get a dependent function template specialization
/// is with a friend declaration, like so:
///
/// \code
///   template \<class T> void foo(T);
///   template \<class T> class A {
///     friend void foo<>(T);
///   };
/// \endcode
///
/// There really isn't any useful analysis we can do here, so we
/// just store the information.
bool
Sema::CheckDependentFunctionTemplateSpecialization(FunctionDecl *FD,
                   const TemplateArgumentListInfo &ExplicitTemplateArgs,
                                                   LookupResult &Previous) {
  // Remove anything from Previous that isn't a function template in
  // the correct context.
  DeclContext *FDLookupContext = FD->getDeclContext()->getRedeclContext();
  LookupResult::Filter F = Previous.makeFilter();
  while (F.hasNext()) {
    NamedDecl *D = F.next()->getUnderlyingDecl();
    if (!isa<FunctionTemplateDecl>(D) ||
        !FDLookupContext->InEnclosingNamespaceSetOf(
                              D->getDeclContext()->getRedeclContext()))
      F.erase();
  }
  F.done();

  // Should this be diagnosed here?
  if (Previous.empty()) return true;

  FD->setDependentTemplateSpecialization(Context, Previous.asUnresolvedSet(),
                                         ExplicitTemplateArgs);
  return false;
}

/// \brief Perform semantic analysis for the given function template
/// specialization.
///
/// This routine performs all of the semantic analysis required for an
/// explicit function template specialization. On successful completion,
/// the function declaration \p FD will become a function template
/// specialization.
///
/// \param FD the function declaration, which will be updated to become a
/// function template specialization.
///
/// \param ExplicitTemplateArgs the explicitly-provided template arguments,
/// if any. Note that this may be valid info even when 0 arguments are
/// explicitly provided as in, e.g., \c void sort<>(char*, char*);
/// as it anyway contains info on the angle brackets locations.
///
/// \param Previous the set of declarations that may be specialized by
/// this function specialization.
bool Sema::CheckFunctionTemplateSpecialization(
    FunctionDecl *FD, TemplateArgumentListInfo *ExplicitTemplateArgs,
    LookupResult &Previous) {
  // The set of function template specializations that could match this
  // explicit function template specialization.
  UnresolvedSet<8> Candidates;
  TemplateSpecCandidateSet FailedCandidates(FD->getLocation());

  DeclContext *FDLookupContext = FD->getDeclContext()->getRedeclContext();
  for (LookupResult::iterator I = Previous.begin(), E = Previous.end();
         I != E; ++I) {
    NamedDecl *Ovl = (*I)->getUnderlyingDecl();
    if (FunctionTemplateDecl *FunTmpl = dyn_cast<FunctionTemplateDecl>(Ovl)) {
      // Only consider templates found within the same semantic lookup scope as
      // FD.
      if (!FDLookupContext->InEnclosingNamespaceSetOf(
                                Ovl->getDeclContext()->getRedeclContext()))
        continue;

      // When matching a constexpr member function template specialization
      // against the primary template, we don't yet know whether the
      // specialization has an implicit 'const' (because we don't know whether
      // it will be a static member function until we know which template it
      // specializes), so adjust it now assuming it specializes this template.
      QualType FT = FD->getType();
      if (FD->isConstexpr()) {
        CXXMethodDecl *OldMD =
          dyn_cast<CXXMethodDecl>(FunTmpl->getTemplatedDecl());
        if (OldMD && OldMD->isConst()) {
          const FunctionProtoType *FPT = FT->castAs<FunctionProtoType>();
          FunctionProtoType::ExtProtoInfo EPI = FPT->getExtProtoInfo();
          EPI.TypeQuals |= Qualifiers::Const;
          FT = Context.getFunctionType(FPT->getReturnType(),
                                       FPT->getParamTypes(), EPI,
                                       None); // HLSL - assume in (although constexpr not enabled yet)
        }
      }

      // C++ [temp.expl.spec]p11:
      //   A trailing template-argument can be left unspecified in the
      //   template-id naming an explicit function template specialization
      //   provided it can be deduced from the function argument type.
      // Perform template argument deduction to determine whether we may be
      // specializing this template.
      // FIXME: It is somewhat wasteful to build
      TemplateDeductionInfo Info(FailedCandidates.getLocation());
      FunctionDecl *Specialization = nullptr;
      if (TemplateDeductionResult TDK = DeduceTemplateArguments(
              cast<FunctionTemplateDecl>(FunTmpl->getFirstDecl()),
              ExplicitTemplateArgs, FT, Specialization, Info)) {
        // Template argument deduction failed; record why it failed, so
        // that we can provide nifty diagnostics.
        FailedCandidates.addCandidate()
            .set(FunTmpl->getTemplatedDecl(),
                 MakeDeductionFailureInfo(Context, TDK, Info));
        (void)TDK;
        continue;
      }

      // Record this candidate.
      Candidates.addDecl(Specialization, I.getAccess());
    }
  }

  // Find the most specialized function template.
  UnresolvedSetIterator Result = getMostSpecialized(
      Candidates.begin(), Candidates.end(), FailedCandidates,
      FD->getLocation(),
      PDiag(diag::err_function_template_spec_no_match) << FD->getDeclName(),
      PDiag(diag::err_function_template_spec_ambiguous)
          << FD->getDeclName() << (ExplicitTemplateArgs != nullptr),
      PDiag(diag::note_function_template_spec_matched));

  if (Result == Candidates.end())
    return true;

  // Ignore access information;  it doesn't figure into redeclaration checking.
  FunctionDecl *Specialization = cast<FunctionDecl>(*Result);

  FunctionTemplateSpecializationInfo *SpecInfo
    = Specialization->getTemplateSpecializationInfo();
  assert(SpecInfo && "Function template specialization info missing?");

  // Note: do not overwrite location info if previous template
  // specialization kind was explicit.
  TemplateSpecializationKind TSK = SpecInfo->getTemplateSpecializationKind();
  if (TSK == TSK_Undeclared || TSK == TSK_ImplicitInstantiation) {
    Specialization->setLocation(FD->getLocation());
    // C++11 [dcl.constexpr]p1: An explicit specialization of a constexpr
    // function can differ from the template declaration with respect to
    // the constexpr specifier.
    Specialization->setConstexpr(FD->isConstexpr());
  }

  // FIXME: Check if the prior specialization has a point of instantiation.
  // If so, we have run afoul of .

  // If this is a friend declaration, then we're not really declaring
  // an explicit specialization.
  bool isFriend = (FD->getFriendObjectKind() != Decl::FOK_None);

  // Check the scope of this explicit specialization.
  if (!isFriend &&
      CheckTemplateSpecializationScope(*this,
                                       Specialization->getPrimaryTemplate(),
                                       Specialization, FD->getLocation(),
                                       false))
    return true;

  // C++ [temp.expl.spec]p6:
  //   If a template, a member template or the member of a class template is
  //   explicitly specialized then that specialization shall be declared
  //   before the first use of that specialization that would cause an implicit
  //   instantiation to take place, in every translation unit in which such a
  //   use occurs; no diagnostic is required.
  bool HasNoEffect = false;
  if (!isFriend &&
      CheckSpecializationInstantiationRedecl(FD->getLocation(),
                                             TSK_ExplicitSpecialization,
                                             Specialization,
                                   SpecInfo->getTemplateSpecializationKind(),
                                         SpecInfo->getPointOfInstantiation(),
                                             HasNoEffect))
    return true;
  
  // Mark the prior declaration as an explicit specialization, so that later
  // clients know that this is an explicit specialization.
  if (!isFriend) {
    SpecInfo->setTemplateSpecializationKind(TSK_ExplicitSpecialization);
    MarkUnusedFileScopedDecl(Specialization);
  }

  // Turn the given function declaration into a function template
  // specialization, with the template arguments from the previous
  // specialization.
  // Take copies of (semantic and syntactic) template argument lists.
  const TemplateArgumentList* TemplArgs = new (Context)
    TemplateArgumentList(Specialization->getTemplateSpecializationArgs());
  FD->setFunctionTemplateSpecialization(Specialization->getPrimaryTemplate(),
                                        TemplArgs, /*InsertPos=*/nullptr,
                                    SpecInfo->getTemplateSpecializationKind(),
                                        ExplicitTemplateArgs);

  // The "previous declaration" for this function template specialization is
  // the prior function template specialization.
  Previous.clear();
  Previous.addDecl(Specialization);
  return false;
}

/// \brief Perform semantic analysis for the given non-template member
/// specialization.
///
/// This routine performs all of the semantic analysis required for an
/// explicit member function specialization. On successful completion,
/// the function declaration \p FD will become a member function
/// specialization.
///
/// \param Member the member declaration, which will be updated to become a
/// specialization.
///
/// \param Previous the set of declarations, one of which may be specialized
/// by this function specialization;  the set will be modified to contain the
/// redeclared member.
bool
Sema::CheckMemberSpecialization(NamedDecl *Member, LookupResult &Previous) {
  assert(!isa<TemplateDecl>(Member) && "Only for non-template members");

  // Try to find the member we are instantiating.
  NamedDecl *Instantiation = nullptr;
  NamedDecl *InstantiatedFrom = nullptr;
  MemberSpecializationInfo *MSInfo = nullptr;

  if (Previous.empty()) {
    // Nowhere to look anyway.
  } else if (FunctionDecl *Function = dyn_cast<FunctionDecl>(Member)) {
    for (LookupResult::iterator I = Previous.begin(), E = Previous.end();
           I != E; ++I) {
      NamedDecl *D = (*I)->getUnderlyingDecl();
      if (CXXMethodDecl *Method = dyn_cast<CXXMethodDecl>(D)) {
        QualType Adjusted = Function->getType();
        if (!hasExplicitCallingConv(Adjusted))
          Adjusted = adjustCCAndNoReturn(Adjusted, Method->getType());
        if (Context.hasSameType(Adjusted, Method->getType())) {
          Instantiation = Method;
          InstantiatedFrom = Method->getInstantiatedFromMemberFunction();
          MSInfo = Method->getMemberSpecializationInfo();
          break;
        }
      }
    }
  } else if (isa<VarDecl>(Member)) {
    VarDecl *PrevVar;
    if (Previous.isSingleResult() &&
        (PrevVar = dyn_cast<VarDecl>(Previous.getFoundDecl())))
      if (PrevVar->isStaticDataMember()) {
        Instantiation = PrevVar;
        InstantiatedFrom = PrevVar->getInstantiatedFromStaticDataMember();
        MSInfo = PrevVar->getMemberSpecializationInfo();
      }
  } else if (isa<RecordDecl>(Member)) {
    CXXRecordDecl *PrevRecord;
    if (Previous.isSingleResult() &&
        (PrevRecord = dyn_cast<CXXRecordDecl>(Previous.getFoundDecl()))) {
      Instantiation = PrevRecord;
      InstantiatedFrom = PrevRecord->getInstantiatedFromMemberClass();
      MSInfo = PrevRecord->getMemberSpecializationInfo();
    }
  } else if (isa<EnumDecl>(Member)) {
    EnumDecl *PrevEnum;
    if (Previous.isSingleResult() &&
        (PrevEnum = dyn_cast<EnumDecl>(Previous.getFoundDecl()))) {
      Instantiation = PrevEnum;
      InstantiatedFrom = PrevEnum->getInstantiatedFromMemberEnum();
      MSInfo = PrevEnum->getMemberSpecializationInfo();
    }
  }

  if (!Instantiation) {
    // There is no previous declaration that matches. Since member
    // specializations are always out-of-line, the caller will complain about
    // this mismatch later.
    return false;
  }

  // If this is a friend, just bail out here before we start turning
  // things into explicit specializations.
  if (Member->getFriendObjectKind() != Decl::FOK_None) {
    // Preserve instantiation information.
    if (InstantiatedFrom && isa<CXXMethodDecl>(Member)) {
      cast<CXXMethodDecl>(Member)->setInstantiationOfMemberFunction(
                                      cast<CXXMethodDecl>(InstantiatedFrom),
        cast<CXXMethodDecl>(Instantiation)->getTemplateSpecializationKind());
    } else if (InstantiatedFrom && isa<CXXRecordDecl>(Member)) {
      cast<CXXRecordDecl>(Member)->setInstantiationOfMemberClass(
                                      cast<CXXRecordDecl>(InstantiatedFrom),
        cast<CXXRecordDecl>(Instantiation)->getTemplateSpecializationKind());
    }

    Previous.clear();
    Previous.addDecl(Instantiation);
    return false;
  }

  // Make sure that this is a specialization of a member.
  if (!InstantiatedFrom) {
    Diag(Member->getLocation(), diag::err_spec_member_not_instantiated)
      << Member;
    Diag(Instantiation->getLocation(), diag::note_specialized_decl);
    return true;
  }

  // C++ [temp.expl.spec]p6:
  //   If a template, a member template or the member of a class template is
  //   explicitly specialized then that specialization shall be declared
  //   before the first use of that specialization that would cause an implicit
  //   instantiation to take place, in every translation unit in which such a
  //   use occurs; no diagnostic is required.
  assert(MSInfo && "Member specialization info missing?");

  bool HasNoEffect = false;
  if (CheckSpecializationInstantiationRedecl(Member->getLocation(),
                                             TSK_ExplicitSpecialization,
                                             Instantiation,
                                     MSInfo->getTemplateSpecializationKind(),
                                           MSInfo->getPointOfInstantiation(),
                                             HasNoEffect))
    return true;

  // Check the scope of this explicit specialization.
  if (CheckTemplateSpecializationScope(*this,
                                       InstantiatedFrom,
                                       Instantiation, Member->getLocation(),
                                       false))
    return true;

  // Note that this is an explicit instantiation of a member.
  // the original declaration to note that it is an explicit specialization
  // (if it was previously an implicit instantiation). This latter step
  // makes bookkeeping easier.
  if (isa<FunctionDecl>(Member)) {
    FunctionDecl *InstantiationFunction = cast<FunctionDecl>(Instantiation);
    if (InstantiationFunction->getTemplateSpecializationKind() ==
          TSK_ImplicitInstantiation) {
      InstantiationFunction->setTemplateSpecializationKind(
                                                  TSK_ExplicitSpecialization);
      InstantiationFunction->setLocation(Member->getLocation());
    }

    cast<FunctionDecl>(Member)->setInstantiationOfMemberFunction(
                                        cast<CXXMethodDecl>(InstantiatedFrom),
                                                  TSK_ExplicitSpecialization);
    MarkUnusedFileScopedDecl(InstantiationFunction);
  } else if (isa<VarDecl>(Member)) {
    VarDecl *InstantiationVar = cast<VarDecl>(Instantiation);
    if (InstantiationVar->getTemplateSpecializationKind() ==
          TSK_ImplicitInstantiation) {
      InstantiationVar->setTemplateSpecializationKind(
                                                  TSK_ExplicitSpecialization);
      InstantiationVar->setLocation(Member->getLocation());
    }

    cast<VarDecl>(Member)->setInstantiationOfStaticDataMember(
        cast<VarDecl>(InstantiatedFrom), TSK_ExplicitSpecialization);
    MarkUnusedFileScopedDecl(InstantiationVar);
  } else if (isa<CXXRecordDecl>(Member)) {
    CXXRecordDecl *InstantiationClass = cast<CXXRecordDecl>(Instantiation);
    if (InstantiationClass->getTemplateSpecializationKind() ==
          TSK_ImplicitInstantiation) {
      InstantiationClass->setTemplateSpecializationKind(
                                                   TSK_ExplicitSpecialization);
      InstantiationClass->setLocation(Member->getLocation());
    }

    cast<CXXRecordDecl>(Member)->setInstantiationOfMemberClass(
                                        cast<CXXRecordDecl>(InstantiatedFrom),
                                                   TSK_ExplicitSpecialization);
  } else {
    assert(isa<EnumDecl>(Member) && "Only member enums remain");
    EnumDecl *InstantiationEnum = cast<EnumDecl>(Instantiation);
    if (InstantiationEnum->getTemplateSpecializationKind() ==
          TSK_ImplicitInstantiation) {
      InstantiationEnum->setTemplateSpecializationKind(
                                                   TSK_ExplicitSpecialization);
      InstantiationEnum->setLocation(Member->getLocation());
    }

    cast<EnumDecl>(Member)->setInstantiationOfMemberEnum(
        cast<EnumDecl>(InstantiatedFrom), TSK_ExplicitSpecialization);
  }

  // Save the caller the trouble of having to figure out which declaration
  // this specialization matches.
  Previous.clear();
  Previous.addDecl(Instantiation);
  return false;
}

/// \brief Check the scope of an explicit instantiation.
///
/// \returns true if a serious error occurs, false otherwise.
static bool CheckExplicitInstantiationScope(Sema &S, NamedDecl *D,
                                            SourceLocation InstLoc,
                                            bool WasQualifiedName) {
  DeclContext *OrigContext= D->getDeclContext()->getEnclosingNamespaceContext();
  DeclContext *CurContext = S.CurContext->getRedeclContext();

  if (CurContext->isRecord()) {
    S.Diag(InstLoc, diag::err_explicit_instantiation_in_class)
      << D;
    return true;
  }

  // C++11 [temp.explicit]p3:
  //   An explicit instantiation shall appear in an enclosing namespace of its
  //   template. If the name declared in the explicit instantiation is an
  //   unqualified name, the explicit instantiation shall appear in the
  //   namespace where its template is declared or, if that namespace is inline
  //   (7.3.1), any namespace from its enclosing namespace set.
  //
  // This is DR275, which we do not retroactively apply to C++98/03.
  if (WasQualifiedName) {
    if (CurContext->Encloses(OrigContext))
      return false;
  } else {
    if (CurContext->InEnclosingNamespaceSetOf(OrigContext))
      return false;
  }

  if (NamespaceDecl *NS = dyn_cast<NamespaceDecl>(OrigContext)) {
    if (WasQualifiedName)
      S.Diag(InstLoc,
             S.getLangOpts().CPlusPlus11?
               diag::err_explicit_instantiation_out_of_scope :
               diag::warn_explicit_instantiation_out_of_scope_0x)
        << D << NS;
    else
      S.Diag(InstLoc,
             S.getLangOpts().CPlusPlus11?
               diag::err_explicit_instantiation_unqualified_wrong_namespace :
               diag::warn_explicit_instantiation_unqualified_wrong_namespace_0x)
        << D << NS;
  } else
    S.Diag(InstLoc,
           S.getLangOpts().CPlusPlus11?
             diag::err_explicit_instantiation_must_be_global :
             diag::warn_explicit_instantiation_must_be_global_0x)
      << D;
  S.Diag(D->getLocation(), diag::note_explicit_instantiation_here);
  return false;
}

/// \brief Determine whether the given scope specifier has a template-id in it.
static bool ScopeSpecifierHasTemplateId(const CXXScopeSpec &SS) {
  if (!SS.isSet())
    return false;

  // C++11 [temp.explicit]p3:
  //   If the explicit instantiation is for a member function, a member class
  //   or a static data member of a class template specialization, the name of
  //   the class template specialization in the qualified-id for the member
  //   name shall be a simple-template-id.
  //
  // C++98 has the same restriction, just worded differently.
  for (NestedNameSpecifier *NNS = SS.getScopeRep(); NNS;
       NNS = NNS->getPrefix())
    if (const Type *T = NNS->getAsType())
      if (isa<TemplateSpecializationType>(T))
        return true;

  return false;
}

// Explicit instantiation of a class template specialization
DeclResult
Sema::ActOnExplicitInstantiation(Scope *S,
                                 SourceLocation ExternLoc,
                                 SourceLocation TemplateLoc,
                                 unsigned TagSpec,
                                 SourceLocation KWLoc,
                                 const CXXScopeSpec &SS,
                                 TemplateTy TemplateD,
                                 SourceLocation TemplateNameLoc,
                                 SourceLocation LAngleLoc,
                                 ASTTemplateArgsPtr TemplateArgsIn,
                                 SourceLocation RAngleLoc,
                                 AttributeList *Attr) {
  // Find the class template we're specializing
  TemplateName Name = TemplateD.get();
  TemplateDecl *TD = Name.getAsTemplateDecl();
  // Check that the specialization uses the same tag kind as the
  // original template.
  TagTypeKind Kind = TypeWithKeyword::getTagTypeKindForTypeSpec(TagSpec);
  assert(Kind != TTK_Enum &&
         "Invalid enum tag in class template explicit instantiation!");

  if (isa<TypeAliasTemplateDecl>(TD)) {
      Diag(KWLoc, diag::err_tag_reference_non_tag) << Kind;
      Diag(TD->getTemplatedDecl()->getLocation(),
           diag::note_previous_use);
    return true;
  }

  ClassTemplateDecl *ClassTemplate = cast<ClassTemplateDecl>(TD);

  if (!isAcceptableTagRedeclaration(ClassTemplate->getTemplatedDecl(),
                                    Kind, /*isDefinition*/false, KWLoc,
                                    ClassTemplate->getIdentifier())) {
    Diag(KWLoc, diag::err_use_with_wrong_tag)
      << ClassTemplate
      << FixItHint::CreateReplacement(KWLoc,
                            ClassTemplate->getTemplatedDecl()->getKindName());
    Diag(ClassTemplate->getTemplatedDecl()->getLocation(),
         diag::note_previous_use);
    Kind = ClassTemplate->getTemplatedDecl()->getTagKind();
  }

  // C++0x [temp.explicit]p2:
  //   There are two forms of explicit instantiation: an explicit instantiation
  //   definition and an explicit instantiation declaration. An explicit
  //   instantiation declaration begins with the extern keyword. [...]
  TemplateSpecializationKind TSK = ExternLoc.isInvalid()
                                       ? TSK_ExplicitInstantiationDefinition
                                       : TSK_ExplicitInstantiationDeclaration;

  if (TSK == TSK_ExplicitInstantiationDeclaration) {
    // Check for dllexport class template instantiation declarations.
    for (AttributeList *A = Attr; A; A = A->getNext()) {
      if (A->getKind() == AttributeList::AT_DLLExport) {
        Diag(ExternLoc,
             diag::warn_attribute_dllexport_explicit_instantiation_decl);
        Diag(A->getLoc(), diag::note_attribute);
        break;
      }
    }

    if (auto *A = ClassTemplate->getTemplatedDecl()->getAttr<DLLExportAttr>()) {
      Diag(ExternLoc,
           diag::warn_attribute_dllexport_explicit_instantiation_decl);
      Diag(A->getLocation(), diag::note_attribute);
    }
  }

  // Translate the parser's template argument list in our AST format.
  TemplateArgumentListInfo TemplateArgs(LAngleLoc, RAngleLoc);
  translateTemplateArguments(TemplateArgsIn, TemplateArgs);

  // Check that the template argument list is well-formed for this
  // template.
  SmallVector<TemplateArgument, 4> Converted;
  if (CheckTemplateArgumentList(ClassTemplate, TemplateNameLoc,
                                TemplateArgs, false, Converted))
    return true;

  // Find the class template specialization declaration that
  // corresponds to these arguments.
  void *InsertPos = nullptr;
  ClassTemplateSpecializationDecl *PrevDecl
    = ClassTemplate->findSpecialization(Converted, InsertPos);

  TemplateSpecializationKind PrevDecl_TSK
    = PrevDecl ? PrevDecl->getTemplateSpecializationKind() : TSK_Undeclared;

  // C++0x [temp.explicit]p2:
  //   [...] An explicit instantiation shall appear in an enclosing
  //   namespace of its template. [...]
  //
  // This is C++ DR 275.
  if (CheckExplicitInstantiationScope(*this, ClassTemplate, TemplateNameLoc,
                                      SS.isSet()))
    return true;

  ClassTemplateSpecializationDecl *Specialization = nullptr;

  bool HasNoEffect = false;
  if (PrevDecl) {
    if (CheckSpecializationInstantiationRedecl(TemplateNameLoc, TSK,
                                               PrevDecl, PrevDecl_TSK,
                                            PrevDecl->getPointOfInstantiation(),
                                               HasNoEffect))
      return PrevDecl;

    // Even though HasNoEffect == true means that this explicit instantiation
    // has no effect on semantics, we go on to put its syntax in the AST.

    if (PrevDecl_TSK == TSK_ImplicitInstantiation ||
        PrevDecl_TSK == TSK_Undeclared) {
      // Since the only prior class template specialization with these
      // arguments was referenced but not declared, reuse that
      // declaration node as our own, updating the source location
      // for the template name to reflect our new declaration.
      // (Other source locations will be updated later.)
      Specialization = PrevDecl;
      Specialization->setLocation(TemplateNameLoc);
      PrevDecl = nullptr;
    }
  }

  if (!Specialization) {
    // Create a new class template specialization declaration node for
    // this explicit specialization.
    Specialization
      = ClassTemplateSpecializationDecl::Create(Context, Kind,
                                             ClassTemplate->getDeclContext(),
                                                KWLoc, TemplateNameLoc,
                                                ClassTemplate,
                                                Converted.data(),
                                                Converted.size(),
                                                PrevDecl);
    SetNestedNameSpecifier(Specialization, SS);

    if (!HasNoEffect && !PrevDecl) {
      // Insert the new specialization.
      ClassTemplate->AddSpecialization(Specialization, InsertPos);
    }
  }

  // Build the fully-sugared type for this explicit instantiation as
  // the user wrote in the explicit instantiation itself. This means
  // that we'll pretty-print the type retrieved from the
  // specialization's declaration the way that the user actually wrote
  // the explicit instantiation, rather than formatting the name based
  // on the "canonical" representation used to store the template
  // arguments in the specialization.
  TypeSourceInfo *WrittenTy
    = Context.getTemplateSpecializationTypeInfo(Name, TemplateNameLoc,
                                                TemplateArgs,
                                  Context.getTypeDeclType(Specialization));
  Specialization->setTypeAsWritten(WrittenTy);

  // Set source locations for keywords.
  Specialization->setExternLoc(ExternLoc);
  Specialization->setTemplateKeywordLoc(TemplateLoc);
  Specialization->setRBraceLoc(SourceLocation());

  if (Attr)
    ProcessDeclAttributeList(S, Specialization, Attr);

  // Add the explicit instantiation into its lexical context. However,
  // since explicit instantiations are never found by name lookup, we
  // just put it into the declaration context directly.
  Specialization->setLexicalDeclContext(CurContext);
  CurContext->addDecl(Specialization);

  // Syntax is now OK, so return if it has no other effect on semantics.
  if (HasNoEffect) {
    // Set the template specialization kind.
    Specialization->setTemplateSpecializationKind(TSK);
    return Specialization;
  }

  // C++ [temp.explicit]p3:
  //   A definition of a class template or class member template
  //   shall be in scope at the point of the explicit instantiation of
  //   the class template or class member template.
  //
  // This check comes when we actually try to perform the
  // instantiation.
  ClassTemplateSpecializationDecl *Def
    = cast_or_null<ClassTemplateSpecializationDecl>(
                                              Specialization->getDefinition());
  if (!Def)
    InstantiateClassTemplateSpecialization(TemplateNameLoc, Specialization, TSK);
  else if (TSK == TSK_ExplicitInstantiationDefinition) {
    MarkVTableUsed(TemplateNameLoc, Specialization, true);
    Specialization->setPointOfInstantiation(Def->getPointOfInstantiation());
  }

  // Instantiate the members of this class template specialization.
  Def = cast_or_null<ClassTemplateSpecializationDecl>(
                                       Specialization->getDefinition());
  if (Def) {
    TemplateSpecializationKind Old_TSK = Def->getTemplateSpecializationKind();

    // Fix a TSK_ExplicitInstantiationDeclaration followed by a
    // TSK_ExplicitInstantiationDefinition
    if (Old_TSK == TSK_ExplicitInstantiationDeclaration &&
        TSK == TSK_ExplicitInstantiationDefinition) {
      // FIXME: Need to notify the ASTMutationListener that we did this.
      Def->setTemplateSpecializationKind(TSK);

      if (!getDLLAttr(Def) && getDLLAttr(Specialization) &&
          Context.getTargetInfo().getCXXABI().isMicrosoft()) {
        // In the MS ABI, an explicit instantiation definition can add a dll
        // attribute to a template with a previous instantiation declaration.
        // MinGW doesn't allow this.
        auto *A = cast<InheritableAttr>(
            getDLLAttr(Specialization)->clone(getASTContext()));
        A->setInherited(true);
        Def->addAttr(A);
        checkClassLevelDLLAttribute(Def);

        // Propagate attribute to base class templates.
        for (auto &B : Def->bases()) {
          if (auto *BT = dyn_cast_or_null<ClassTemplateSpecializationDecl>(
                  B.getType()->getAsCXXRecordDecl()))
            propagateDLLAttrToBaseClassTemplate(Def, A, BT, B.getLocStart());
        }
      }
    }

    InstantiateClassTemplateSpecializationMembers(TemplateNameLoc, Def, TSK);
  }

  // Set the template specialization kind.
  Specialization->setTemplateSpecializationKind(TSK);
  return Specialization;
}

// Explicit instantiation of a member class of a class template.
DeclResult
Sema::ActOnExplicitInstantiation(Scope *S,
                                 SourceLocation ExternLoc,
                                 SourceLocation TemplateLoc,
                                 unsigned TagSpec,
                                 SourceLocation KWLoc,
                                 CXXScopeSpec &SS,
                                 IdentifierInfo *Name,
                                 SourceLocation NameLoc,
                                 AttributeList *Attr) {

  bool Owned = false;
  bool IsDependent = false;
  Decl *TagD = ActOnTag(S, TagSpec, Sema::TUK_Reference,
                        KWLoc, SS, Name, NameLoc, Attr, AS_none,
                        /*ModulePrivateLoc=*/SourceLocation(),
                        MultiTemplateParamsArg(), Owned, IsDependent,
                        SourceLocation(), false, TypeResult(),
                        /*IsTypeSpecifier*/false);
  assert(!IsDependent && "explicit instantiation of dependent name not yet handled");

  if (!TagD)
    return true;

  TagDecl *Tag = cast<TagDecl>(TagD);
  assert(!Tag->isEnum() && "shouldn't see enumerations here");

  if (Tag->isInvalidDecl())
    return true;

  CXXRecordDecl *Record = cast<CXXRecordDecl>(Tag);
  CXXRecordDecl *Pattern = Record->getInstantiatedFromMemberClass();
  if (!Pattern) {
    Diag(TemplateLoc, diag::err_explicit_instantiation_nontemplate_type)
      << Context.getTypeDeclType(Record);
    Diag(Record->getLocation(), diag::note_nontemplate_decl_here);
    return true;
  }

  // C++0x [temp.explicit]p2:
  //   If the explicit instantiation is for a class or member class, the
  //   elaborated-type-specifier in the declaration shall include a
  //   simple-template-id.
  //
  // C++98 has the same restriction, just worded differently.
  if (!ScopeSpecifierHasTemplateId(SS))
    Diag(TemplateLoc, diag::ext_explicit_instantiation_without_qualified_id)
      << Record << SS.getRange();

  // C++0x [temp.explicit]p2:
  //   There are two forms of explicit instantiation: an explicit instantiation
  //   definition and an explicit instantiation declaration. An explicit
  //   instantiation declaration begins with the extern keyword. [...]
  TemplateSpecializationKind TSK
    = ExternLoc.isInvalid()? TSK_ExplicitInstantiationDefinition
                           : TSK_ExplicitInstantiationDeclaration;

  // C++0x [temp.explicit]p2:
  //   [...] An explicit instantiation shall appear in an enclosing
  //   namespace of its template. [...]
  //
  // This is C++ DR 275.
  CheckExplicitInstantiationScope(*this, Record, NameLoc, true);

  // Verify that it is okay to explicitly instantiate here.
  CXXRecordDecl *PrevDecl
    = cast_or_null<CXXRecordDecl>(Record->getPreviousDecl());
  if (!PrevDecl && Record->getDefinition())
    PrevDecl = Record;
  if (PrevDecl) {
    MemberSpecializationInfo *MSInfo = PrevDecl->getMemberSpecializationInfo();
    bool HasNoEffect = false;
    assert(MSInfo && "No member specialization information?");
    if (CheckSpecializationInstantiationRedecl(TemplateLoc, TSK,
                                               PrevDecl,
                                        MSInfo->getTemplateSpecializationKind(),
                                             MSInfo->getPointOfInstantiation(),
                                               HasNoEffect))
      return true;
    if (HasNoEffect)
      return TagD;
  }

  CXXRecordDecl *RecordDef
    = cast_or_null<CXXRecordDecl>(Record->getDefinition());
  if (!RecordDef) {
    // C++ [temp.explicit]p3:
    //   A definition of a member class of a class template shall be in scope
    //   at the point of an explicit instantiation of the member class.
    CXXRecordDecl *Def
      = cast_or_null<CXXRecordDecl>(Pattern->getDefinition());
    if (!Def) {
      Diag(TemplateLoc, diag::err_explicit_instantiation_undefined_member)
        << 0 << Record->getDeclName() << Record->getDeclContext();
      Diag(Pattern->getLocation(), diag::note_forward_declaration)
        << Pattern;
      return true;
    } else {
      if (InstantiateClass(NameLoc, Record, Def,
                           getTemplateInstantiationArgs(Record),
                           TSK))
        return true;

      RecordDef = cast_or_null<CXXRecordDecl>(Record->getDefinition());
      if (!RecordDef)
        return true;
    }
  }

  // Instantiate all of the members of the class.
  InstantiateClassMembers(NameLoc, RecordDef,
                          getTemplateInstantiationArgs(Record), TSK);

  if (TSK == TSK_ExplicitInstantiationDefinition)
    MarkVTableUsed(NameLoc, RecordDef, true);

  // FIXME: We don't have any representation for explicit instantiations of
  // member classes. Such a representation is not needed for compilation, but it
  // should be available for clients that want to see all of the declarations in
  // the source code.
  return TagD;
}

DeclResult Sema::ActOnExplicitInstantiation(Scope *S,
                                            SourceLocation ExternLoc,
                                            SourceLocation TemplateLoc,
                                            Declarator &D) {
  // Explicit instantiations always require a name.
  // TODO: check if/when DNInfo should replace Name.
  DeclarationNameInfo NameInfo = GetNameForDeclarator(D);
  DeclarationName Name = NameInfo.getName();
  if (!Name) {
    if (!D.isInvalidType())
      Diag(D.getDeclSpec().getLocStart(),
           diag::err_explicit_instantiation_requires_name)
        << D.getDeclSpec().getSourceRange()
        << D.getSourceRange();

    return true;
  }

  // The scope passed in may not be a decl scope.  Zip up the scope tree until
  // we find one that is.
  while ((S->getFlags() & Scope::DeclScope) == 0 ||
         (S->getFlags() & Scope::TemplateParamScope) != 0)
    S = S->getParent();

  // Determine the type of the declaration.
  TypeSourceInfo *T = GetTypeForDeclarator(D, S);
  QualType R = T->getType();
  if (R.isNull())
    return true;

  // C++ [dcl.stc]p1:
  //   A storage-class-specifier shall not be specified in [...] an explicit 
  //   instantiation (14.7.2) directive.
  if (D.getDeclSpec().getStorageClassSpec() == DeclSpec::SCS_typedef) {
    Diag(D.getIdentifierLoc(), diag::err_explicit_instantiation_of_typedef)
      << Name;
    return true;
  } else if (D.getDeclSpec().getStorageClassSpec() 
                                                != DeclSpec::SCS_unspecified) {
    // Complain about then remove the storage class specifier.
    Diag(D.getIdentifierLoc(), diag::err_explicit_instantiation_storage_class)
      << FixItHint::CreateRemoval(D.getDeclSpec().getStorageClassSpecLoc());
    
    D.getMutableDeclSpec().ClearStorageClassSpecs();
  }

  // C++0x [temp.explicit]p1:
  //   [...] An explicit instantiation of a function template shall not use the
  //   inline or constexpr specifiers.
  // Presumably, this also applies to member functions of class templates as
  // well.
  if (D.getDeclSpec().isInlineSpecified())
    Diag(D.getDeclSpec().getInlineSpecLoc(),
         getLangOpts().CPlusPlus11 ?
           diag::err_explicit_instantiation_inline :
           diag::warn_explicit_instantiation_inline_0x)
      << FixItHint::CreateRemoval(D.getDeclSpec().getInlineSpecLoc());
  if (D.getDeclSpec().isConstexprSpecified() && R->isFunctionType())
    // FIXME: Add a fix-it to remove the 'constexpr' and add a 'const' if one is
    // not already specified.
    Diag(D.getDeclSpec().getConstexprSpecLoc(),
         diag::err_explicit_instantiation_constexpr);

  // C++0x [temp.explicit]p2:
  //   There are two forms of explicit instantiation: an explicit instantiation
  //   definition and an explicit instantiation declaration. An explicit
  //   instantiation declaration begins with the extern keyword. [...]
  TemplateSpecializationKind TSK
    = ExternLoc.isInvalid()? TSK_ExplicitInstantiationDefinition
                           : TSK_ExplicitInstantiationDeclaration;

  LookupResult Previous(*this, NameInfo, LookupOrdinaryName);
  LookupParsedName(Previous, S, &D.getCXXScopeSpec());

  if (!R->isFunctionType()) {
    // C++ [temp.explicit]p1:
    //   A [...] static data member of a class template can be explicitly
    //   instantiated from the member definition associated with its class
    //   template.
    // C++1y [temp.explicit]p1:
    //   A [...] variable [...] template specialization can be explicitly
    //   instantiated from its template.
    if (Previous.isAmbiguous())
      return true;

    VarDecl *Prev = Previous.getAsSingle<VarDecl>();
    VarTemplateDecl *PrevTemplate = Previous.getAsSingle<VarTemplateDecl>();

    if (!PrevTemplate) {
      if (!Prev || !Prev->isStaticDataMember()) {
        // We expect to see a data data member here.
        Diag(D.getIdentifierLoc(), diag::err_explicit_instantiation_not_known)
            << Name;
        for (LookupResult::iterator P = Previous.begin(), PEnd = Previous.end();
             P != PEnd; ++P)
          Diag((*P)->getLocation(), diag::note_explicit_instantiation_here);
        return true;
      }

      if (!Prev->getInstantiatedFromStaticDataMember()) {
        // FIXME: Check for explicit specialization?
        Diag(D.getIdentifierLoc(),
             diag::err_explicit_instantiation_data_member_not_instantiated)
            << Prev;
        Diag(Prev->getLocation(), diag::note_explicit_instantiation_here);
        // FIXME: Can we provide a note showing where this was declared?
        return true;
      }
    } else {
      // Explicitly instantiate a variable template.

      // C++1y [dcl.spec.auto]p6:
      //   ... A program that uses auto or decltype(auto) in a context not
      //   explicitly allowed in this section is ill-formed.
      //
      // This includes auto-typed variable template instantiations.
      if (R->isUndeducedType()) {
        Diag(T->getTypeLoc().getLocStart(),
             diag::err_auto_not_allowed_var_inst);
        return true;
      }

      if (D.getName().getKind() != UnqualifiedId::IK_TemplateId) {
        // C++1y [temp.explicit]p3:
        //   If the explicit instantiation is for a variable, the unqualified-id
        //   in the declaration shall be a template-id.
        Diag(D.getIdentifierLoc(),
             diag::err_explicit_instantiation_without_template_id)
          << PrevTemplate;
        Diag(PrevTemplate->getLocation(),
             diag::note_explicit_instantiation_here);
        return true;
      }

      // Translate the parser's template argument list into our AST format.
      TemplateArgumentListInfo TemplateArgs =
          makeTemplateArgumentListInfo(*this, *D.getName().TemplateId);

      DeclResult Res = CheckVarTemplateId(PrevTemplate, TemplateLoc,
                                          D.getIdentifierLoc(), TemplateArgs);
      if (Res.isInvalid())
        return true;

      // Ignore access control bits, we don't need them for redeclaration
      // checking.
      Prev = cast<VarDecl>(Res.get());
    }

    // C++0x [temp.explicit]p2:
    //   If the explicit instantiation is for a member function, a member class
    //   or a static data member of a class template specialization, the name of
    //   the class template specialization in the qualified-id for the member
    //   name shall be a simple-template-id.
    //
    // C++98 has the same restriction, just worded differently.
    //
    // This does not apply to variable template specializations, where the
    // template-id is in the unqualified-id instead.
    if (!ScopeSpecifierHasTemplateId(D.getCXXScopeSpec()) && !PrevTemplate)
      Diag(D.getIdentifierLoc(),
           diag::ext_explicit_instantiation_without_qualified_id)
        << Prev << D.getCXXScopeSpec().getRange();

    // Check the scope of this explicit instantiation.
    CheckExplicitInstantiationScope(*this, Prev, D.getIdentifierLoc(), true);

    // Verify that it is okay to explicitly instantiate here.
    TemplateSpecializationKind PrevTSK = Prev->getTemplateSpecializationKind();
    SourceLocation POI = Prev->getPointOfInstantiation();
    bool HasNoEffect = false;
    if (CheckSpecializationInstantiationRedecl(D.getIdentifierLoc(), TSK, Prev,
                                               PrevTSK, POI, HasNoEffect))
      return true;

    if (!HasNoEffect) {
      // Instantiate static data member or variable template.

      Prev->setTemplateSpecializationKind(TSK, D.getIdentifierLoc());
      if (PrevTemplate) {
        // Merge attributes.
        if (AttributeList *Attr = D.getDeclSpec().getAttributes().getList())
          ProcessDeclAttributeList(S, Prev, Attr);
      }
      if (TSK == TSK_ExplicitInstantiationDefinition)
        InstantiateVariableDefinition(D.getIdentifierLoc(), Prev);
    }

    // Check the new variable specialization against the parsed input.
    if (PrevTemplate && Prev && !Context.hasSameType(Prev->getType(), R)) {
      Diag(T->getTypeLoc().getLocStart(),
           diag::err_invalid_var_template_spec_type)
          << 0 << PrevTemplate << R << Prev->getType();
      Diag(PrevTemplate->getLocation(), diag::note_template_declared_here)
          << 2 << PrevTemplate->getDeclName();
      return true;
    }

    // FIXME: Create an ExplicitInstantiation node?
    return (Decl*) nullptr;
  }

  // If the declarator is a template-id, translate the parser's template
  // argument list into our AST format.
  bool HasExplicitTemplateArgs = false;
  TemplateArgumentListInfo TemplateArgs;
  if (D.getName().getKind() == UnqualifiedId::IK_TemplateId) {
    TemplateArgs = makeTemplateArgumentListInfo(*this, *D.getName().TemplateId);
    HasExplicitTemplateArgs = true;
  }

  // C++ [temp.explicit]p1:
  //   A [...] function [...] can be explicitly instantiated from its template.
  //   A member function [...] of a class template can be explicitly
  //  instantiated from the member definition associated with its class
  //  template.
  UnresolvedSet<8> Matches;
  TemplateSpecCandidateSet FailedCandidates(D.getIdentifierLoc());
  for (LookupResult::iterator P = Previous.begin(), PEnd = Previous.end();
       P != PEnd; ++P) {
    NamedDecl *Prev = *P;
    if (!HasExplicitTemplateArgs) {
      if (CXXMethodDecl *Method = dyn_cast<CXXMethodDecl>(Prev)) {
        QualType Adjusted = adjustCCAndNoReturn(R, Method->getType());
        if (Context.hasSameUnqualifiedType(Method->getType(), Adjusted)) {
          Matches.clear();

          Matches.addDecl(Method, P.getAccess());
          if (Method->getTemplateSpecializationKind() == TSK_Undeclared)
            break;
        }
      }
    }

    FunctionTemplateDecl *FunTmpl = dyn_cast<FunctionTemplateDecl>(Prev);
    if (!FunTmpl)
      continue;

    TemplateDeductionInfo Info(FailedCandidates.getLocation());
    FunctionDecl *Specialization = nullptr;
    if (TemplateDeductionResult TDK
          = DeduceTemplateArguments(FunTmpl,
                               (HasExplicitTemplateArgs ? &TemplateArgs
                                                        : nullptr),
                                    R, Specialization, Info)) {
      // Keep track of almost-matches.
      FailedCandidates.addCandidate()
          .set(FunTmpl->getTemplatedDecl(),
               MakeDeductionFailureInfo(Context, TDK, Info));
      (void)TDK;
      continue;
    }

    Matches.addDecl(Specialization, P.getAccess());
  }

  // Find the most specialized function template specialization.
  UnresolvedSetIterator Result = getMostSpecialized(
      Matches.begin(), Matches.end(), FailedCandidates,
      D.getIdentifierLoc(),
      PDiag(diag::err_explicit_instantiation_not_known) << Name,
      PDiag(diag::err_explicit_instantiation_ambiguous) << Name,
      PDiag(diag::note_explicit_instantiation_candidate));

  if (Result == Matches.end())
    return true;

  // Ignore access control bits, we don't need them for redeclaration checking.
  FunctionDecl *Specialization = cast<FunctionDecl>(*Result);

  // C++11 [except.spec]p4
  // In an explicit instantiation an exception-specification may be specified,
  // but is not required.
  // If an exception-specification is specified in an explicit instantiation
  // directive, it shall be compatible with the exception-specifications of
  // other declarations of that function.
  if (auto *FPT = R->getAs<FunctionProtoType>())
    if (FPT->hasExceptionSpec()) {
      unsigned DiagID =
          diag::err_mismatched_exception_spec_explicit_instantiation;
      if (getLangOpts().MicrosoftExt)
        DiagID = diag::ext_mismatched_exception_spec_explicit_instantiation;
      bool Result = CheckEquivalentExceptionSpec(
          PDiag(DiagID) << Specialization->getType(),
          PDiag(diag::note_explicit_instantiation_here),
          Specialization->getType()->getAs<FunctionProtoType>(),
          Specialization->getLocation(), FPT, D.getLocStart());
      // In Microsoft mode, mismatching exception specifications just cause a
      // warning.
      if (!getLangOpts().MicrosoftExt && Result)
        return true;
    }

  if (Specialization->getTemplateSpecializationKind() == TSK_Undeclared) {
    Diag(D.getIdentifierLoc(),
         diag::err_explicit_instantiation_member_function_not_instantiated)
      << Specialization
      << (Specialization->getTemplateSpecializationKind() ==
          TSK_ExplicitSpecialization);
    Diag(Specialization->getLocation(), diag::note_explicit_instantiation_here);
    return true;
  }

  FunctionDecl *PrevDecl = Specialization->getPreviousDecl();
  if (!PrevDecl && Specialization->isThisDeclarationADefinition())
    PrevDecl = Specialization;

  if (PrevDecl) {
    bool HasNoEffect = false;
    if (CheckSpecializationInstantiationRedecl(D.getIdentifierLoc(), TSK,
                                               PrevDecl,
                                     PrevDecl->getTemplateSpecializationKind(),
                                          PrevDecl->getPointOfInstantiation(),
                                               HasNoEffect))
      return true;

    // FIXME: We may still want to build some representation of this
    // explicit specialization.
    if (HasNoEffect)
      return (Decl*) nullptr;
  }

  Specialization->setTemplateSpecializationKind(TSK, D.getIdentifierLoc());
  AttributeList *Attr = D.getDeclSpec().getAttributes().getList();
  if (Attr)
    ProcessDeclAttributeList(S, Specialization, Attr);

  if (Specialization->isDefined()) {
    // Let the ASTConsumer know that this function has been explicitly
    // instantiated now, and its linkage might have changed.
    Consumer.HandleTopLevelDecl(DeclGroupRef(Specialization));
  } else if (TSK == TSK_ExplicitInstantiationDefinition)
    InstantiateFunctionDefinition(D.getIdentifierLoc(), Specialization);

  // C++0x [temp.explicit]p2:
  //   If the explicit instantiation is for a member function, a member class
  //   or a static data member of a class template specialization, the name of
  //   the class template specialization in the qualified-id for the member
  //   name shall be a simple-template-id.
  //
  // C++98 has the same restriction, just worded differently.
  FunctionTemplateDecl *FunTmpl = Specialization->getPrimaryTemplate();
  if (D.getName().getKind() != UnqualifiedId::IK_TemplateId && !FunTmpl &&
      D.getCXXScopeSpec().isSet() &&
      !ScopeSpecifierHasTemplateId(D.getCXXScopeSpec()))
    Diag(D.getIdentifierLoc(),
         diag::ext_explicit_instantiation_without_qualified_id)
    << Specialization << D.getCXXScopeSpec().getRange();

  CheckExplicitInstantiationScope(*this,
                   FunTmpl? (NamedDecl *)FunTmpl
                          : Specialization->getInstantiatedFromMemberFunction(),
                                  D.getIdentifierLoc(),
                                  D.getCXXScopeSpec().isSet());

  // FIXME: Create some kind of ExplicitInstantiationDecl here.
  return (Decl*) nullptr;
}

TypeResult
Sema::ActOnDependentTag(Scope *S, unsigned TagSpec, TagUseKind TUK,
                        const CXXScopeSpec &SS, IdentifierInfo *Name,
                        SourceLocation TagLoc, SourceLocation NameLoc) {
  // This has to hold, because SS is expected to be defined.
  assert(Name && "Expected a name in a dependent tag");

  NestedNameSpecifier *NNS = SS.getScopeRep();
  if (!NNS)
    return true;

  TagTypeKind Kind = TypeWithKeyword::getTagTypeKindForTypeSpec(TagSpec);

  if (TUK == TUK_Declaration || TUK == TUK_Definition) {
    Diag(NameLoc, diag::err_dependent_tag_decl)
      << (TUK == TUK_Definition) << Kind << SS.getRange();
    return true;
  }

  // Create the resulting type.
  ElaboratedTypeKeyword Kwd = TypeWithKeyword::getKeywordForTagTypeKind(Kind);
  QualType Result = Context.getDependentNameType(Kwd, NNS, Name);
  
  // Create type-source location information for this type.
  TypeLocBuilder TLB;
  DependentNameTypeLoc TL = TLB.push<DependentNameTypeLoc>(Result);
  TL.setElaboratedKeywordLoc(TagLoc);
  TL.setQualifierLoc(SS.getWithLocInContext(Context));
  TL.setNameLoc(NameLoc);
  return CreateParsedType(Result, TLB.getTypeSourceInfo(Context, Result));
}

TypeResult
Sema::ActOnTypenameType(Scope *S, SourceLocation TypenameLoc,
                        const CXXScopeSpec &SS, const IdentifierInfo &II,
                        SourceLocation IdLoc) {
  if (SS.isInvalid())
    return true;
  
  if (TypenameLoc.isValid() && S && !S->getTemplateParamParent())
    Diag(TypenameLoc,
         getLangOpts().CPlusPlus11 ?
           diag::warn_cxx98_compat_typename_outside_of_template :
           diag::ext_typename_outside_of_template)
      << FixItHint::CreateRemoval(TypenameLoc);

  NestedNameSpecifierLoc QualifierLoc = SS.getWithLocInContext(Context);
  QualType T = CheckTypenameType(TypenameLoc.isValid()? ETK_Typename : ETK_None,
                                 TypenameLoc, QualifierLoc, II, IdLoc);
  if (T.isNull())
    return true;

  TypeSourceInfo *TSI = Context.CreateTypeSourceInfo(T);
  if (isa<DependentNameType>(T)) {
    DependentNameTypeLoc TL = TSI->getTypeLoc().castAs<DependentNameTypeLoc>();
    TL.setElaboratedKeywordLoc(TypenameLoc);
    TL.setQualifierLoc(QualifierLoc);
    TL.setNameLoc(IdLoc);
  } else {
    ElaboratedTypeLoc TL = TSI->getTypeLoc().castAs<ElaboratedTypeLoc>();
    TL.setElaboratedKeywordLoc(TypenameLoc);
    TL.setQualifierLoc(QualifierLoc);
    TL.getNamedTypeLoc().castAs<TypeSpecTypeLoc>().setNameLoc(IdLoc);
  }

  return CreateParsedType(T, TSI);
}

TypeResult
Sema::ActOnTypenameType(Scope *S,
                        SourceLocation TypenameLoc,
                        const CXXScopeSpec &SS,
                        SourceLocation TemplateKWLoc,
                        TemplateTy TemplateIn,
                        SourceLocation TemplateNameLoc,
                        SourceLocation LAngleLoc,
                        ASTTemplateArgsPtr TemplateArgsIn,
                        SourceLocation RAngleLoc) {
  if (TypenameLoc.isValid() && S && !S->getTemplateParamParent())
    Diag(TypenameLoc,
         getLangOpts().CPlusPlus11 ?
           diag::warn_cxx98_compat_typename_outside_of_template :
           diag::ext_typename_outside_of_template)
      << FixItHint::CreateRemoval(TypenameLoc);
  
  // Translate the parser's template argument list in our AST format.
  TemplateArgumentListInfo TemplateArgs(LAngleLoc, RAngleLoc);
  translateTemplateArguments(TemplateArgsIn, TemplateArgs);
  
  TemplateName Template = TemplateIn.get();
  if (DependentTemplateName *DTN = Template.getAsDependentTemplateName()) {
    // Construct a dependent template specialization type.
    assert(DTN && "dependent template has non-dependent name?");
    assert(DTN->getQualifier() == SS.getScopeRep());
    QualType T = Context.getDependentTemplateSpecializationType(ETK_Typename,
                                                          DTN->getQualifier(),
                                                          DTN->getIdentifier(),
                                                                TemplateArgs);
    
    // Create source-location information for this type.
    TypeLocBuilder Builder;
    DependentTemplateSpecializationTypeLoc SpecTL 
    = Builder.push<DependentTemplateSpecializationTypeLoc>(T);
    SpecTL.setElaboratedKeywordLoc(TypenameLoc);
    SpecTL.setQualifierLoc(SS.getWithLocInContext(Context));
    SpecTL.setTemplateKeywordLoc(TemplateKWLoc);
    SpecTL.setTemplateNameLoc(TemplateNameLoc);
    SpecTL.setLAngleLoc(LAngleLoc);
    SpecTL.setRAngleLoc(RAngleLoc);
    for (unsigned I = 0, N = TemplateArgs.size(); I != N; ++I)
      SpecTL.setArgLocInfo(I, TemplateArgs[I].getLocInfo());
    return CreateParsedType(T, Builder.getTypeSourceInfo(Context, T));
  }
  
  QualType T = CheckTemplateIdType(Template, TemplateNameLoc, TemplateArgs);
  if (T.isNull())
    return true;
  
  // Provide source-location information for the template specialization type.
  TypeLocBuilder Builder;
  TemplateSpecializationTypeLoc SpecTL
    = Builder.push<TemplateSpecializationTypeLoc>(T);
  SpecTL.setTemplateKeywordLoc(TemplateKWLoc);
  SpecTL.setTemplateNameLoc(TemplateNameLoc);
  SpecTL.setLAngleLoc(LAngleLoc);
  SpecTL.setRAngleLoc(RAngleLoc);
  for (unsigned I = 0, N = TemplateArgs.size(); I != N; ++I)
    SpecTL.setArgLocInfo(I, TemplateArgs[I].getLocInfo());
  
  T = Context.getElaboratedType(ETK_Typename, SS.getScopeRep(), T);
  ElaboratedTypeLoc TL = Builder.push<ElaboratedTypeLoc>(T);
  TL.setElaboratedKeywordLoc(TypenameLoc);
  TL.setQualifierLoc(SS.getWithLocInContext(Context));
  
  TypeSourceInfo *TSI = Builder.getTypeSourceInfo(Context, T);
  return CreateParsedType(T, TSI);
}


/// Determine whether this failed name lookup should be treated as being
/// disabled by a usage of std::enable_if.
static bool isEnableIf(NestedNameSpecifierLoc NNS, const IdentifierInfo &II,
                       SourceRange &CondRange) {
  // We must be looking for a ::type...
  if (!II.isStr("type"))
    return false;

  // ... within an explicitly-written template specialization...
  if (!NNS || !NNS.getNestedNameSpecifier()->getAsType())
    return false;
  TypeLoc EnableIfTy = NNS.getTypeLoc();
  TemplateSpecializationTypeLoc EnableIfTSTLoc =
      EnableIfTy.getAs<TemplateSpecializationTypeLoc>();
  if (!EnableIfTSTLoc || EnableIfTSTLoc.getNumArgs() == 0)
    return false;
  const TemplateSpecializationType *EnableIfTST =
    cast<TemplateSpecializationType>(EnableIfTSTLoc.getTypePtr());

  // ... which names a complete class template declaration...
  const TemplateDecl *EnableIfDecl =
    EnableIfTST->getTemplateName().getAsTemplateDecl();
  if (!EnableIfDecl || EnableIfTST->isIncompleteType())
    return false;

  // ... called "enable_if".
  const IdentifierInfo *EnableIfII =
    EnableIfDecl->getDeclName().getAsIdentifierInfo();
  if (!EnableIfII || !EnableIfII->isStr("enable_if"))
    return false;

  // Assume the first template argument is the condition.
  CondRange = EnableIfTSTLoc.getArgLoc(0).getSourceRange();
  return true;
}

/// \brief Build the type that describes a C++ typename specifier,
/// e.g., "typename T::type".
QualType
Sema::CheckTypenameType(ElaboratedTypeKeyword Keyword, 
                        SourceLocation KeywordLoc,
                        NestedNameSpecifierLoc QualifierLoc, 
                        const IdentifierInfo &II,
                        SourceLocation IILoc) {
  CXXScopeSpec SS;
  SS.Adopt(QualifierLoc);

  DeclContext *Ctx = computeDeclContext(SS);
  if (!Ctx) {
    // If the nested-name-specifier is dependent and couldn't be
    // resolved to a type, build a typename type.
    assert(QualifierLoc.getNestedNameSpecifier()->isDependent());
    return Context.getDependentNameType(Keyword, 
                                        QualifierLoc.getNestedNameSpecifier(), 
                                        &II);
  }

  // If the nested-name-specifier refers to the current instantiation,
  // the "typename" keyword itself is superfluous. In C++03, the
  // program is actually ill-formed. However, DR 382 (in C++0x CD1)
  // allows such extraneous "typename" keywords, and we retroactively
  // apply this DR to C++03 code with only a warning. In any case we continue.

  if (RequireCompleteDeclContext(SS, Ctx))
    return QualType();

  DeclarationName Name(&II);
  LookupResult Result(*this, Name, IILoc, LookupOrdinaryName);
  LookupQualifiedName(Result, Ctx, SS);
  unsigned DiagID = 0;
  Decl *Referenced = nullptr;
  switch (Result.getResultKind()) {
  case LookupResult::NotFound: {
    // If we're looking up 'type' within a template named 'enable_if', produce
    // a more specific diagnostic.
    SourceRange CondRange;
    if (isEnableIf(QualifierLoc, II, CondRange)) {
      Diag(CondRange.getBegin(), diag::err_typename_nested_not_found_enable_if)
        << Ctx << CondRange;
      return QualType();
    }

    DiagID = diag::err_typename_nested_not_found;
    break;
  }

  case LookupResult::FoundUnresolvedValue: {
    // We found a using declaration that is a value. Most likely, the using
    // declaration itself is meant to have the 'typename' keyword.
    SourceRange FullRange(KeywordLoc.isValid() ? KeywordLoc : SS.getBeginLoc(),
                          IILoc);
    Diag(IILoc, diag::err_typename_refers_to_using_value_decl)
      << Name << Ctx << FullRange;
    if (UnresolvedUsingValueDecl *Using
          = dyn_cast<UnresolvedUsingValueDecl>(Result.getRepresentativeDecl())){
      SourceLocation Loc = Using->getQualifierLoc().getBeginLoc();
      Diag(Loc, diag::note_using_value_decl_missing_typename)
        << FixItHint::CreateInsertion(Loc, "typename ");
    }
  }
  // Fall through to create a dependent typename type, from which we can recover
  // better.
  LLVM_FALLTHROUGH; // HLSL Change
  case LookupResult::NotFoundInCurrentInstantiation:
    // Okay, it's a member of an unknown instantiation.
    return Context.getDependentNameType(Keyword, 
                                        QualifierLoc.getNestedNameSpecifier(), 
                                        &II);

  case LookupResult::Found:
    if (TypeDecl *Type = dyn_cast<TypeDecl>(Result.getFoundDecl())) {
      // We found a type. Build an ElaboratedType, since the
      // typename-specifier was just sugar.
      MarkAnyDeclReferenced(Type->getLocation(), Type, /*OdrUse=*/false);
      return Context.getElaboratedType(ETK_Typename, 
                                       QualifierLoc.getNestedNameSpecifier(),
                                       Context.getTypeDeclType(Type));
    }

    DiagID = diag::err_typename_nested_not_type;
    Referenced = Result.getFoundDecl();
    break;

  case LookupResult::FoundOverloaded:
    DiagID = diag::err_typename_nested_not_type;
    Referenced = *Result.begin();
    break;

  case LookupResult::Ambiguous:
    return QualType();
  }

  // If we get here, it's because name lookup did not find a
  // type. Emit an appropriate diagnostic and return an error.
  SourceRange FullRange(KeywordLoc.isValid() ? KeywordLoc : SS.getBeginLoc(),
                        IILoc);
  Diag(IILoc, DiagID) << FullRange << Name << Ctx;
  if (Referenced)
    Diag(Referenced->getLocation(), diag::note_typename_refers_here)
      << Name;
  return QualType();
}

namespace {
  // See Sema::RebuildTypeInCurrentInstantiation
  class CurrentInstantiationRebuilder
    : public TreeTransform<CurrentInstantiationRebuilder> {
    SourceLocation Loc;
    DeclarationName Entity;

  public:
    typedef TreeTransform<CurrentInstantiationRebuilder> inherited;

    CurrentInstantiationRebuilder(Sema &SemaRef,
                                  SourceLocation Loc,
                                  DeclarationName Entity)
    : TreeTransform<CurrentInstantiationRebuilder>(SemaRef),
      Loc(Loc), Entity(Entity) { }

    /// \brief Determine whether the given type \p T has already been
    /// transformed.
    ///
    /// For the purposes of type reconstruction, a type has already been
    /// transformed if it is NULL or if it is not dependent.
    bool AlreadyTransformed(QualType T) {
      return T.isNull() || !T->isDependentType();
    }

    /// \brief Returns the location of the entity whose type is being
    /// rebuilt.
    SourceLocation getBaseLocation() { return Loc; }

    /// \brief Returns the name of the entity whose type is being rebuilt.
    DeclarationName getBaseEntity() { return Entity; }

    /// \brief Sets the "base" location and entity when that
    /// information is known based on another transformation.
    void setBase(SourceLocation Loc, DeclarationName Entity) {
      this->Loc = Loc;
      this->Entity = Entity;
    }
      
    ExprResult TransformLambdaExpr(LambdaExpr *E) {
      // Lambdas never need to be transformed.
      return E;
    }
  };
}

/// \brief Rebuilds a type within the context of the current instantiation.
///
/// The type \p T is part of the type of an out-of-line member definition of
/// a class template (or class template partial specialization) that was parsed
/// and constructed before we entered the scope of the class template (or
/// partial specialization thereof). This routine will rebuild that type now
/// that we have entered the declarator's scope, which may produce different
/// canonical types, e.g.,
///
/// \code
/// template<typename T>
/// struct X {
///   typedef T* pointer;
///   pointer data();
/// };
///
/// template<typename T>
/// typename X<T>::pointer X<T>::data() { ... }
/// \endcode
///
/// Here, the type "typename X<T>::pointer" will be created as a DependentNameType,
/// since we do not know that we can look into X<T> when we parsed the type.
/// This function will rebuild the type, performing the lookup of "pointer"
/// in X<T> and returning an ElaboratedType whose canonical type is the same
/// as the canonical type of T*, allowing the return types of the out-of-line
/// definition and the declaration to match.
TypeSourceInfo *Sema::RebuildTypeInCurrentInstantiation(TypeSourceInfo *T,
                                                        SourceLocation Loc,
                                                        DeclarationName Name) {
  if (!T || !T->getType()->isDependentType())
    return T;

  CurrentInstantiationRebuilder Rebuilder(*this, Loc, Name);
  return Rebuilder.TransformType(T);
}

ExprResult Sema::RebuildExprInCurrentInstantiation(Expr *E) {
  CurrentInstantiationRebuilder Rebuilder(*this, E->getExprLoc(),
                                          DeclarationName());
  return Rebuilder.TransformExpr(E);
}

bool Sema::RebuildNestedNameSpecifierInCurrentInstantiation(CXXScopeSpec &SS) {
  if (SS.isInvalid()) 
    return true;

  NestedNameSpecifierLoc QualifierLoc = SS.getWithLocInContext(Context);
  CurrentInstantiationRebuilder Rebuilder(*this, SS.getRange().getBegin(),
                                          DeclarationName());
  NestedNameSpecifierLoc Rebuilt 
    = Rebuilder.TransformNestedNameSpecifierLoc(QualifierLoc);
  if (!Rebuilt) 
    return true;

  SS.Adopt(Rebuilt);
  return false;
}

/// \brief Rebuild the template parameters now that we know we're in a current
/// instantiation.
bool Sema::RebuildTemplateParamsInCurrentInstantiation(
                                               TemplateParameterList *Params) {
  for (unsigned I = 0, N = Params->size(); I != N; ++I) {
    Decl *Param = Params->getParam(I);
    
    // There is nothing to rebuild in a type parameter.
    if (isa<TemplateTypeParmDecl>(Param))
      continue;
    
    // Rebuild the template parameter list of a template template parameter.
    if (TemplateTemplateParmDecl *TTP 
        = dyn_cast<TemplateTemplateParmDecl>(Param)) {
      if (RebuildTemplateParamsInCurrentInstantiation(
            TTP->getTemplateParameters()))
        return true;
      
      continue;
    }
    
    // Rebuild the type of a non-type template parameter.
    NonTypeTemplateParmDecl *NTTP = cast<NonTypeTemplateParmDecl>(Param);
    TypeSourceInfo *NewTSI 
      = RebuildTypeInCurrentInstantiation(NTTP->getTypeSourceInfo(), 
                                          NTTP->getLocation(), 
                                          NTTP->getDeclName());
    if (!NewTSI)
      return true;
    
    if (NewTSI != NTTP->getTypeSourceInfo()) {
      NTTP->setTypeSourceInfo(NewTSI);
      NTTP->setType(NewTSI->getType());
    }
  }
  
  return false;
}

/// \brief Produces a formatted string that describes the binding of
/// template parameters to template arguments.
std::string
Sema::getTemplateArgumentBindingsText(const TemplateParameterList *Params,
                                      const TemplateArgumentList &Args) {
  return getTemplateArgumentBindingsText(Params, Args.data(), Args.size());
}

std::string
Sema::getTemplateArgumentBindingsText(const TemplateParameterList *Params,
                                      const TemplateArgument *Args,
                                      unsigned NumArgs) {
  SmallString<128> Str;
  llvm::raw_svector_ostream Out(Str);

  if (!Params || Params->size() == 0 || NumArgs == 0)
    return std::string();

  for (unsigned I = 0, N = Params->size(); I != N; ++I) {
    if (I >= NumArgs)
      break;

    if (I == 0)
      Out << "[with ";
    else
      Out << ", ";

    if (const IdentifierInfo *Id = Params->getParam(I)->getIdentifier()) {
      Out << Id->getName();
    } else {
      Out << '$' << I;
    }

    Out << " = ";
    Args[I].print(getPrintingPolicy(), Out);
  }

  Out << ']';
  return Out.str();
}

void Sema::MarkAsLateParsedTemplate(FunctionDecl *FD, Decl *FnD,
                                    CachedTokens &Toks) {
  if (!FD)
    return;

  LateParsedTemplate *LPT = new LateParsedTemplate;

  // Take tokens to avoid allocations
  LPT->Toks.swap(Toks);
  LPT->D = FnD;
  LateParsedTemplateMap.insert(std::make_pair(FD, LPT));

  FD->setLateTemplateParsed(true);
}

void Sema::UnmarkAsLateParsedTemplate(FunctionDecl *FD) {
  if (!FD)
    return;
  FD->setLateTemplateParsed(false);
}

bool Sema::IsInsideALocalClassWithinATemplateFunction() {
  DeclContext *DC = CurContext;

  while (DC) {
    if (CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(CurContext)) {
      const FunctionDecl *FD = RD->isLocalClass();
      return (FD && FD->getTemplatedKind() != FunctionDecl::TK_NonTemplate);
    } else if (DC->isTranslationUnit() || DC->isNamespace())
      return false;

    DC = DC->getParent();
  }
  return false;
}
