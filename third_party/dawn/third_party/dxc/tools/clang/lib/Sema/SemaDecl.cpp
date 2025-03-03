//===--- SemaDecl.cpp - Semantic Analysis for Declarations ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements semantic analysis for declarations.
//
//===----------------------------------------------------------------------===//

#include "TypeLocBuilder.h"
#include "dxc/DXIL/DxilSemantic.h" // HLSL Change
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTLambda.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/CharUnits.h"
#include "clang/AST/CommentDiagnostic.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/EvaluatedExprVisitor.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/StmtCXX.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/PartialDiagnostic.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/HeaderSearch.h" // TODO: Sema shouldn't depend on Lex
#include "clang/Lex/Lexer.h" // TODO: Extract static functions to fix layering.
#include "clang/Lex/ModuleLoader.h" // TODO: Sema shouldn't depend on Lex
#include "clang/Lex/Preprocessor.h" // Included for isCodeCompletionEnabled()
#include "clang/Parse/ParseDiagnostic.h"
#include "clang/Sema/CXXFieldCollector.h"
#include "clang/Sema/DeclSpec.h"
#include "clang/Sema/DelayedDiagnostic.h"
#include "clang/Sema/Initialization.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/ParsedTemplate.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/ScopeInfo.h"
#include "clang/Sema/SemaHLSL.h" // HLSL Change
#include "clang/Sema/SemaInternal.h"
#include "clang/Sema/Template.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Triple.h"
#include <algorithm>
#include <cstring>
#include <functional>
using namespace clang;
using namespace sema;

Sema::DeclGroupPtrTy Sema::ConvertDeclToDeclGroup(Decl *Ptr, Decl *OwnedType) {
  if (OwnedType) {
    Decl *Group[2] = { OwnedType, Ptr };
    return DeclGroupPtrTy::make(DeclGroupRef::Create(Context, Group, 2));
  }

  return DeclGroupPtrTy::make(DeclGroupRef(Ptr));
}

namespace {

class TypeNameValidatorCCC : public CorrectionCandidateCallback {
 public:
  TypeNameValidatorCCC(bool AllowInvalid, bool WantClass=false,
                       bool AllowTemplates=false)
      : AllowInvalidDecl(AllowInvalid), WantClassName(WantClass),
        AllowClassTemplates(AllowTemplates) {
    WantExpressionKeywords = false;
    WantCXXNamedCasts = false;
    WantRemainingKeywords = false;
  }

  bool ValidateCandidate(const TypoCorrection &candidate) override {
    if (NamedDecl *ND = candidate.getCorrectionDecl()) {
      bool IsType = isa<TypeDecl>(ND) || isa<ObjCInterfaceDecl>(ND);
      bool AllowedTemplate = AllowClassTemplates && isa<ClassTemplateDecl>(ND);
      return (IsType || AllowedTemplate) &&
             (AllowInvalidDecl || !ND->isInvalidDecl());
    }
    return !WantClassName && candidate.isKeyword();
  }

 private:
  bool AllowInvalidDecl;
  bool WantClassName;
  bool AllowClassTemplates;
};

}

/// \brief Determine whether the token kind starts a simple-type-specifier.
bool Sema::isSimpleTypeSpecifier(tok::TokenKind Kind) const {
  switch (Kind) {
  // FIXME: Take into account the current language when deciding whether a
  // token kind is a valid type specifier
  case tok::kw_short:
  case tok::kw_long:
  case tok::kw___int64:
  case tok::kw___int128:
  case tok::kw_signed:
  case tok::kw_unsigned:
  case tok::kw_void:
  case tok::kw_char:
  case tok::kw_int:
  case tok::kw_half:
  case tok::kw_float:
  case tok::kw_double:
  case tok::kw_wchar_t:
  case tok::kw_bool:
  case tok::kw___underlying_type:
    return true;

  case tok::annot_typename:
  case tok::kw_char16_t:
  case tok::kw_char32_t:
  case tok::kw_typeof:
  case tok::annot_decltype:
  case tok::kw_decltype:
    return getLangOpts().CPlusPlus;

  default:
    break;
  }

  return false;
}

namespace {
enum class UnqualifiedTypeNameLookupResult {
  NotFound,
  FoundNonType,
  FoundType
};
} // namespace

/// \brief Tries to perform unqualified lookup of the type decls in bases for
/// dependent class.
/// \return \a NotFound if no any decls is found, \a FoundNotType if found not a
/// type decl, \a FoundType if only type decls are found.
static UnqualifiedTypeNameLookupResult
lookupUnqualifiedTypeNameInBase(Sema &S, const IdentifierInfo &II,
                                SourceLocation NameLoc,
                                const CXXRecordDecl *RD) {
  if (!RD->hasDefinition())
    return UnqualifiedTypeNameLookupResult::NotFound;
  // Look for type decls in base classes.
  UnqualifiedTypeNameLookupResult FoundTypeDecl =
      UnqualifiedTypeNameLookupResult::NotFound;
  for (const auto &Base : RD->bases()) {
    const CXXRecordDecl *BaseRD = nullptr;
    if (auto *BaseTT = Base.getType()->getAs<TagType>())
      BaseRD = BaseTT->getAsCXXRecordDecl();
    else if (auto *TST = Base.getType()->getAs<TemplateSpecializationType>()) {
      // Look for type decls in dependent base classes that have known primary
      // templates.
      if (!TST || !TST->isDependentType())
        continue;
      auto *TD = TST->getTemplateName().getAsTemplateDecl();
      if (!TD)
        continue;
      auto *BasePrimaryTemplate =
          dyn_cast_or_null<CXXRecordDecl>(TD->getTemplatedDecl());
      if (!BasePrimaryTemplate)
        continue;
      BaseRD = BasePrimaryTemplate;
    }
    if (BaseRD) {
      for (NamedDecl *ND : BaseRD->lookup(&II)) {
        if (!isa<TypeDecl>(ND))
          return UnqualifiedTypeNameLookupResult::FoundNonType;
        FoundTypeDecl = UnqualifiedTypeNameLookupResult::FoundType;
      }
      if (FoundTypeDecl == UnqualifiedTypeNameLookupResult::NotFound) {
        switch (lookupUnqualifiedTypeNameInBase(S, II, NameLoc, BaseRD)) {
        case UnqualifiedTypeNameLookupResult::FoundNonType:
          return UnqualifiedTypeNameLookupResult::FoundNonType;
        case UnqualifiedTypeNameLookupResult::FoundType:
          FoundTypeDecl = UnqualifiedTypeNameLookupResult::FoundType;
          break;
        case UnqualifiedTypeNameLookupResult::NotFound:
          break;
        }
      }
    }
  }

  return FoundTypeDecl;
}

static ParsedType recoverFromTypeInKnownDependentBase(Sema &S,
                                                      const IdentifierInfo &II,
                                                      SourceLocation NameLoc) {
  // Lookup in the parent class template context, if any.
  const CXXRecordDecl *RD = nullptr;
  UnqualifiedTypeNameLookupResult FoundTypeDecl =
      UnqualifiedTypeNameLookupResult::NotFound;
  for (DeclContext *DC = S.CurContext;
       DC && FoundTypeDecl == UnqualifiedTypeNameLookupResult::NotFound;
       DC = DC->getParent()) {
    // Look for type decls in dependent base classes that have known primary
    // templates.
    RD = dyn_cast<CXXRecordDecl>(DC);
    if (RD && RD->getDescribedClassTemplate())
      FoundTypeDecl = lookupUnqualifiedTypeNameInBase(S, II, NameLoc, RD);
  }
  if (FoundTypeDecl != UnqualifiedTypeNameLookupResult::FoundType)
    return ParsedType();

  // We found some types in dependent base classes.  Recover as if the user
  // wrote 'typename MyClass::II' instead of 'II'.  We'll fully resolve the
  // lookup during template instantiation.
  S.Diag(NameLoc, diag::ext_found_via_dependent_bases_lookup) << &II;

  ASTContext &Context = S.Context;
  auto *NNS = NestedNameSpecifier::Create(Context, nullptr, false,
                                          cast<Type>(Context.getRecordType(RD)));
  QualType T = Context.getDependentNameType(ETK_Typename, NNS, &II);

  CXXScopeSpec SS;
  SS.MakeTrivial(Context, NNS, SourceRange(NameLoc));

  TypeLocBuilder Builder;
  DependentNameTypeLoc DepTL = Builder.push<DependentNameTypeLoc>(T);
  DepTL.setNameLoc(NameLoc);
  DepTL.setElaboratedKeywordLoc(SourceLocation());
  DepTL.setQualifierLoc(SS.getWithLocInContext(Context));
  return S.CreateParsedType(T, Builder.getTypeSourceInfo(Context, T));
}

/// \brief If the identifier refers to a type name within this scope,
/// return the declaration of that type.
///
/// This routine performs ordinary name lookup of the identifier II
/// within the given scope, with optional C++ scope specifier SS, to
/// determine whether the name refers to a type. If so, returns an
/// opaque pointer (actually a QualType) corresponding to that
/// type. Otherwise, returns NULL.
ParsedType Sema::getTypeName(const IdentifierInfo &II, SourceLocation NameLoc,
                             Scope *S, CXXScopeSpec *SS,
                             bool isClassName, bool HasTrailingDot,
                             ParsedType ObjectTypePtr,
                             bool IsCtorOrDtorName,
                             bool WantNontrivialTypeSourceInfo,
                             IdentifierInfo **CorrectedII) {
  // Determine where we will perform name lookup.
  DeclContext *LookupCtx = nullptr;
  if (ObjectTypePtr) {
    QualType ObjectType = ObjectTypePtr.get();
    if (ObjectType->isRecordType())
      LookupCtx = computeDeclContext(ObjectType);
  } else if (SS && SS->isNotEmpty()) {
    LookupCtx = computeDeclContext(*SS, false);

    if (!LookupCtx) {
      if (isDependentScopeSpecifier(*SS)) {
        // C++ [temp.res]p3:
        //   A qualified-id that refers to a type and in which the
        //   nested-name-specifier depends on a template-parameter (14.6.2)
        //   shall be prefixed by the keyword typename to indicate that the
        //   qualified-id denotes a type, forming an
        //   elaborated-type-specifier (7.1.5.3).
        //
        // We therefore do not perform any name lookup if the result would
        // refer to a member of an unknown specialization.
        if (!isClassName && !IsCtorOrDtorName)
          return ParsedType();
        
        // We know from the grammar that this name refers to a type,
        // so build a dependent node to describe the type.
        if (WantNontrivialTypeSourceInfo)
          return ActOnTypenameType(S, SourceLocation(), *SS, II, NameLoc).get();
        
        NestedNameSpecifierLoc QualifierLoc = SS->getWithLocInContext(Context);
        QualType T = CheckTypenameType(ETK_None, SourceLocation(), QualifierLoc,
                                       II, NameLoc);
        return ParsedType::make(T);
      }
      
      return ParsedType();
    }
    
    if (!LookupCtx->isDependentContext() &&
        RequireCompleteDeclContext(*SS, LookupCtx))
      return ParsedType();
  }

  // FIXME: LookupNestedNameSpecifierName isn't the right kind of
  // lookup for class-names.
  LookupNameKind Kind = isClassName ? LookupNestedNameSpecifierName :
                                      LookupOrdinaryName;
  LookupResult Result(*this, &II, NameLoc, Kind);
  if (LookupCtx) {
    // Perform "qualified" name lookup into the declaration context we
    // computed, which is either the type of the base of a member access
    // expression or the declaration context associated with a prior
    // nested-name-specifier.
    LookupQualifiedName(Result, LookupCtx);

    if (ObjectTypePtr && Result.empty()) {
      // C++ [basic.lookup.classref]p3:
      //   If the unqualified-id is ~type-name, the type-name is looked up
      //   in the context of the entire postfix-expression. If the type T of 
      //   the object expression is of a class type C, the type-name is also
      //   looked up in the scope of class C. At least one of the lookups shall
      //   find a name that refers to (possibly cv-qualified) T.
      LookupName(Result, S);
    }
  } else {
    // Perform unqualified name lookup.
    LookupName(Result, S);

    // For unqualified lookup in a class template in MSVC mode, look into
    // dependent base classes where the primary class template is known.
    if (Result.empty() && getLangOpts().MSVCCompat && (!SS || SS->isEmpty())) {
      if (ParsedType TypeInBase =
              recoverFromTypeInKnownDependentBase(*this, II, NameLoc))
        return TypeInBase;
    }
  }

  NamedDecl *IIDecl = nullptr;
  switch (Result.getResultKind()) {
  case LookupResult::NotFound:
  case LookupResult::NotFoundInCurrentInstantiation:
    if (CorrectedII) {
      TypoCorrection Correction = CorrectTypo(
          Result.getLookupNameInfo(), Kind, S, SS,
          llvm::make_unique<TypeNameValidatorCCC>(true, isClassName),
          CTK_ErrorRecovery);
      IdentifierInfo *NewII = Correction.getCorrectionAsIdentifierInfo();
      TemplateTy Template;
      bool MemberOfUnknownSpecialization;
      UnqualifiedId TemplateName;
      TemplateName.setIdentifier(NewII, NameLoc);
      NestedNameSpecifier *NNS = Correction.getCorrectionSpecifier();
      CXXScopeSpec NewSS, *NewSSPtr = SS;
      if (SS && NNS) {
        NewSS.MakeTrivial(Context, NNS, SourceRange(NameLoc));
        NewSSPtr = &NewSS;
      }
      if (Correction && (NNS || NewII != &II) &&
          // Ignore a correction to a template type as the to-be-corrected
          // identifier is not a template (typo correction for template names
          // is handled elsewhere).
          !(getLangOpts().CPlusPlus && NewSSPtr &&
            isTemplateName(S, *NewSSPtr, false, TemplateName, ParsedType(),
                           false, Template, MemberOfUnknownSpecialization))) {
        ParsedType Ty = getTypeName(*NewII, NameLoc, S, NewSSPtr,
                                    isClassName, HasTrailingDot, ObjectTypePtr,
                                    IsCtorOrDtorName,
                                    WantNontrivialTypeSourceInfo);
        if (Ty) {
          diagnoseTypo(Correction,
                       PDiag(diag::err_unknown_type_or_class_name_suggest)
                         << Result.getLookupName() << isClassName);
          if (SS && NNS)
            SS->MakeTrivial(Context, NNS, SourceRange(NameLoc));
          *CorrectedII = NewII;
          return Ty;
        }
      }
    }
    // If typo correction failed or was not performed, fall through
    LLVM_FALLTHROUGH; // HLSL Change
  case LookupResult::FoundOverloaded:
  case LookupResult::FoundUnresolvedValue:
    Result.suppressDiagnostics();
    return ParsedType();

  case LookupResult::Ambiguous:
    // Recover from type-hiding ambiguities by hiding the type.  We'll
    // do the lookup again when looking for an object, and we can
    // diagnose the error then.  If we don't do this, then the error
    // about hiding the type will be immediately followed by an error
    // that only makes sense if the identifier was treated like a type.
    if (Result.getAmbiguityKind() == LookupResult::AmbiguousTagHiding) {
      Result.suppressDiagnostics();
      return ParsedType();
    }

    // Look to see if we have a type anywhere in the list of results.
    for (LookupResult::iterator Res = Result.begin(), ResEnd = Result.end();
         Res != ResEnd; ++Res) {
      if (isa<TypeDecl>(*Res) || isa<ObjCInterfaceDecl>(*Res)) {
        if (!IIDecl ||
            (*Res)->getLocation().getRawEncoding() <
              IIDecl->getLocation().getRawEncoding())
          IIDecl = *Res;
      }
    }

    if (!IIDecl) {
      // None of the entities we found is a type, so there is no way
      // to even assume that the result is a type. In this case, don't
      // complain about the ambiguity. The parser will either try to
      // perform this lookup again (e.g., as an object name), which
      // will produce the ambiguity, or will complain that it expected
      // a type name.
      Result.suppressDiagnostics();
      return ParsedType();
    }

    // We found a type within the ambiguous lookup; diagnose the
    // ambiguity and then return that type. This might be the right
    // answer, or it might not be, but it suppresses any attempt to
    // perform the name lookup again.
    break;

  case LookupResult::Found:
    IIDecl = Result.getFoundDecl();
    break;
  }

  assert(IIDecl && "Didn't find decl");

  QualType T;
  if (TypeDecl *TD = dyn_cast<TypeDecl>(IIDecl)) {
    DiagnoseUseOfDecl(IIDecl, NameLoc);

    T = Context.getTypeDeclType(TD);
    MarkAnyDeclReferenced(TD->getLocation(), TD, /*OdrUse=*/false);

    // NOTE: avoid constructing an ElaboratedType(Loc) if this is a
    // constructor or destructor name (in such a case, the scope specifier
    // will be attached to the enclosing Expr or Decl node).
    if (SS && SS->isNotEmpty() && !IsCtorOrDtorName) {
      if (WantNontrivialTypeSourceInfo) {
        // Construct a type with type-source information.
        TypeLocBuilder Builder;
        Builder.pushTypeSpec(T).setNameLoc(NameLoc);
        
        T = getElaboratedType(ETK_None, *SS, T);
        ElaboratedTypeLoc ElabTL = Builder.push<ElaboratedTypeLoc>(T);
        ElabTL.setElaboratedKeywordLoc(SourceLocation());
        ElabTL.setQualifierLoc(SS->getWithLocInContext(Context));
        return CreateParsedType(T, Builder.getTypeSourceInfo(Context, T));
      } else {
        T = getElaboratedType(ETK_None, *SS, T);
      }
    }
  } else if (ObjCInterfaceDecl *IDecl = dyn_cast<ObjCInterfaceDecl>(IIDecl)) {
    (void)DiagnoseUseOfDecl(IDecl, NameLoc);
    if (!HasTrailingDot)
      T = Context.getObjCInterfaceType(IDecl);
  } else if (getLangOpts().HLSL) { // HLSL - omit empty template argument lists
    if (TemplateDecl *TD = dyn_cast<TemplateDecl>(IIDecl)) {
      QualType DefaultTy = getHLSLDefaultSpecialization(TD);
      if (!DefaultTy.isNull())
        T = DefaultTy;
    } // HLSL Change end
  }

  if (T.isNull()) {
    // If it's not plausibly a type, suppress diagnostics.
    Result.suppressDiagnostics();
    return ParsedType();
  }
  return ParsedType::make(T);
}

// Builds a fake NNS for the given decl context.
static NestedNameSpecifier *
synthesizeCurrentNestedNameSpecifier(ASTContext &Context, DeclContext *DC) {
  for (;; DC = DC->getLookupParent()) {
    DC = DC->getPrimaryContext();
    auto *ND = dyn_cast<NamespaceDecl>(DC);
    if (ND && !ND->isInline() && !ND->isAnonymousNamespace())
      return NestedNameSpecifier::Create(Context, nullptr, ND);
    else if (auto *RD = dyn_cast<CXXRecordDecl>(DC))
      return NestedNameSpecifier::Create(Context, nullptr, RD->isTemplateDecl(),
                                         RD->getTypeForDecl());
    else if (isa<TranslationUnitDecl>(DC))
      return NestedNameSpecifier::GlobalSpecifier(Context);
  }
  llvm_unreachable("something isn't in TU scope?");
}

ParsedType Sema::ActOnDelayedDefaultTemplateArg(const IdentifierInfo &II,
                                                SourceLocation NameLoc) {
  // Accepting an undeclared identifier as a default argument for a template
  // type parameter is a Microsoft extension.
  Diag(NameLoc, diag::ext_ms_delayed_template_argument) << &II;

  // Build a fake DependentNameType that will perform lookup into CurContext at
  // instantiation time.  The name specifier isn't dependent, so template
  // instantiation won't transform it.  It will retry the lookup, however.
  NestedNameSpecifier *NNS =
      synthesizeCurrentNestedNameSpecifier(Context, CurContext);
  QualType T = Context.getDependentNameType(ETK_None, NNS, &II);

  // Build type location information.  We synthesized the qualifier, so we have
  // to build a fake NestedNameSpecifierLoc.
  NestedNameSpecifierLocBuilder NNSLocBuilder;
  NNSLocBuilder.MakeTrivial(Context, NNS, SourceRange(NameLoc));
  NestedNameSpecifierLoc QualifierLoc = NNSLocBuilder.getWithLocInContext(Context);

  TypeLocBuilder Builder;
  DependentNameTypeLoc DepTL = Builder.push<DependentNameTypeLoc>(T);
  DepTL.setNameLoc(NameLoc);
  DepTL.setElaboratedKeywordLoc(SourceLocation());
  DepTL.setQualifierLoc(QualifierLoc);
  return CreateParsedType(T, Builder.getTypeSourceInfo(Context, T));
}

/// isTagName() - This method is called *for error recovery purposes only*
/// to determine if the specified name is a valid tag name ("struct foo").  If
/// so, this returns the TST for the tag corresponding to it (TST_enum,
/// TST_union, TST_struct, TST_interface, TST_class).  This is used to diagnose
/// cases in C where the user forgot to specify the tag.
DeclSpec::TST Sema::isTagName(IdentifierInfo &II, Scope *S) {
  // Do a tag name lookup in this scope.
  LookupResult R(*this, &II, SourceLocation(), LookupTagName);
  LookupName(R, S, false);
  R.suppressDiagnostics();
  if (R.getResultKind() == LookupResult::Found)
    if (const TagDecl *TD = R.getAsSingle<TagDecl>()) {
      switch (TD->getTagKind()) {
      case TTK_Struct: return DeclSpec::TST_struct;
      case TTK_Interface: return DeclSpec::TST_interface;
      case TTK_Union:  return DeclSpec::TST_union;
      case TTK_Class:  return DeclSpec::TST_class;
      case TTK_Enum:   return DeclSpec::TST_enum;
      }
    }

  return DeclSpec::TST_unspecified;
}

/// isMicrosoftMissingTypename - In Microsoft mode, within class scope,
/// if a CXXScopeSpec's type is equal to the type of one of the base classes
/// then downgrade the missing typename error to a warning.
/// This is needed for MSVC compatibility; Example:
/// @code
/// template<class T> class A {
/// public:
///   typedef int TYPE;
/// };
/// template<class T> class B : public A<T> {
/// public:
///   A<T>::TYPE a; // no typename required because A<T> is a base class.
/// };
/// @endcode
bool Sema::isMicrosoftMissingTypename(const CXXScopeSpec *SS, Scope *S) {
  if (CurContext->isRecord()) {
    if (SS->getScopeRep()->getKind() == NestedNameSpecifier::Super)
      return true;

    const Type *Ty = SS->getScopeRep()->getAsType();

    CXXRecordDecl *RD = cast<CXXRecordDecl>(CurContext);
    for (const auto &Base : RD->bases())
      if (Context.hasSameUnqualifiedType(QualType(Ty, 1), Base.getType()))
        return true;
    return S->isFunctionPrototypeScope();
  } 
  return CurContext->isFunctionOrMethod() || S->isFunctionPrototypeScope();
}

void Sema::DiagnoseUnknownTypeName(IdentifierInfo *&II,
                                   SourceLocation IILoc,
                                   Scope *S,
                                   CXXScopeSpec *SS,
                                   ParsedType &SuggestedType,
                                   bool AllowClassTemplates) {
  // We don't have anything to suggest (yet).
  SuggestedType = ParsedType();
  
  // There may have been a typo in the name of the type. Look up typo
  // results, in case we have something that we can suggest.
  if (TypoCorrection Corrected =
          CorrectTypo(DeclarationNameInfo(II, IILoc), LookupOrdinaryName, S, SS,
                      llvm::make_unique<TypeNameValidatorCCC>(
                          false, false, AllowClassTemplates),
                      CTK_ErrorRecovery)) {
    if (Corrected.isKeyword()) {
      // We corrected to a keyword.
      diagnoseTypo(Corrected, PDiag(diag::err_unknown_typename_suggest) << II);
      II = Corrected.getCorrectionAsIdentifierInfo();
    } else {
      // We found a similarly-named type or interface; suggest that.
      if (!SS || !SS->isSet()) {
        diagnoseTypo(Corrected,
                     PDiag(diag::err_unknown_typename_suggest) << II);
      } else if (DeclContext *DC = computeDeclContext(*SS, false)) {
        std::string CorrectedStr(Corrected.getAsString(getLangOpts()));
        bool DroppedSpecifier = Corrected.WillReplaceSpecifier() &&
                                II->getName().equals(CorrectedStr);
        diagnoseTypo(Corrected,
                     PDiag(diag::err_unknown_nested_typename_suggest)
                       << II << DC << DroppedSpecifier << SS->getRange());
      } else {
        llvm_unreachable("could not have corrected a typo here");
      }

      CXXScopeSpec tmpSS;
      if (Corrected.getCorrectionSpecifier())
        tmpSS.MakeTrivial(Context, Corrected.getCorrectionSpecifier(),
                          SourceRange(IILoc));
      SuggestedType = getTypeName(*Corrected.getCorrectionAsIdentifierInfo(),
                                  IILoc, S, tmpSS.isSet() ? &tmpSS : SS, false,
                                  false, ParsedType(),
                                  /*IsCtorOrDtorName=*/false,
                                  /*NonTrivialTypeSourceInfo=*/true);
    }
    return;
  }

  if (getLangOpts().CPlusPlus) {
    // See if II is a class template that the user forgot to pass arguments to.
    UnqualifiedId Name;
    Name.setIdentifier(II, IILoc);
    CXXScopeSpec EmptySS;
    TemplateTy TemplateResult;
    bool MemberOfUnknownSpecialization;
    if (isTemplateName(S, SS ? *SS : EmptySS, /*hasTemplateKeyword=*/false,
                       Name, ParsedType(), true, TemplateResult,
                       MemberOfUnknownSpecialization) == TNK_Type_template) {
      TemplateName TplName = TemplateResult.get();
      Diag(IILoc, diag::err_template_missing_args) << TplName;
      if (TemplateDecl *TplDecl = TplName.getAsTemplateDecl()) {
        if (TplDecl->getLocation().isValid()) { // HLSL Change - ellide location notes for built-ins
          Diag(TplDecl->getLocation(), diag::note_template_decl_here)
            << TplDecl->getTemplateParameters()->getSourceRange();
        }
      }
      return;
    }
  }

  // FIXME: Should we move the logic that tries to recover from a missing tag
  // (struct, union, enum) from Parser::ParseImplicitInt here, instead?
  
  if (!SS || (!SS->isSet() && !SS->isInvalid()))
    Diag(IILoc, diag::err_unknown_typename) << II;
  else if (DeclContext *DC = computeDeclContext(*SS, false))
    Diag(IILoc, diag::err_typename_nested_not_found) 
      << II << DC << SS->getRange();
  else if (isDependentScopeSpecifier(*SS)) {
    unsigned DiagID = diag::err_typename_missing;
    if (getLangOpts().MSVCCompat && isMicrosoftMissingTypename(SS, S))
      DiagID = diag::ext_typename_missing;

    Diag(SS->getRange().getBegin(), DiagID)
      << SS->getScopeRep() << II->getName()
      << SourceRange(SS->getRange().getBegin(), IILoc)
      << FixItHint::CreateInsertion(SS->getRange().getBegin(), "typename ");
    SuggestedType = ActOnTypenameType(S, SourceLocation(),
                                      *SS, *II, IILoc).get();
  } else {
    assert(SS && SS->isInvalid() && 
           "Invalid scope specifier has already been diagnosed");
  }
}

/// \brief Determine whether the given result set contains either a type name
/// or 
static bool isResultTypeOrTemplate(LookupResult &R, const Token &NextToken) {
  bool CheckTemplate = R.getSema().getLangOpts().CPlusPlus &&
                       NextToken.is(tok::less);
  
  for (LookupResult::iterator I = R.begin(), IEnd = R.end(); I != IEnd; ++I) {
    if (isa<TypeDecl>(*I) || isa<ObjCInterfaceDecl>(*I))
      return true;
    
    if (CheckTemplate && isa<TemplateDecl>(*I))
      return true;
  }
  
  return false;
}

static bool isTagTypeWithMissingTag(Sema &SemaRef, LookupResult &Result,
                                    Scope *S, CXXScopeSpec &SS,
                                    IdentifierInfo *&Name,
                                    SourceLocation NameLoc) {
  LookupResult R(SemaRef, Name, NameLoc, Sema::LookupTagName);
  SemaRef.LookupParsedName(R, S, &SS);
  if (TagDecl *Tag = R.getAsSingle<TagDecl>()) {
    StringRef FixItTagName;
    switch (Tag->getTagKind()) {
      case TTK_Class:
        FixItTagName = "class ";
        break;

      case TTK_Enum:
        FixItTagName = "enum ";
        break;

      case TTK_Struct:
        FixItTagName = "struct ";
        break;

      case TTK_Interface:
        FixItTagName = "__interface ";
        break;

      case TTK_Union:
        FixItTagName = "union ";
        break;
    }

    StringRef TagName = FixItTagName.drop_back();
    SemaRef.Diag(NameLoc, diag::err_use_of_tag_name_without_tag)
      << Name << TagName << SemaRef.getLangOpts().CPlusPlus
      << FixItHint::CreateInsertion(NameLoc, FixItTagName);

    for (LookupResult::iterator I = Result.begin(), IEnd = Result.end();
         I != IEnd; ++I)
      SemaRef.Diag((*I)->getLocation(), diag::note_decl_hiding_tag_type)
        << Name << TagName;

    // Replace lookup results with just the tag decl.
    Result.clear(Sema::LookupTagName);
    SemaRef.LookupParsedName(Result, S, &SS);
    return true;
  }

  return false;
}

/// Build a ParsedType for a simple-type-specifier with a nested-name-specifier.
static ParsedType buildNestedType(Sema &S, CXXScopeSpec &SS,
                                  QualType T, SourceLocation NameLoc) {
  ASTContext &Context = S.Context;

  TypeLocBuilder Builder;
  Builder.pushTypeSpec(T).setNameLoc(NameLoc);

  T = S.getElaboratedType(ETK_None, SS, T);
  ElaboratedTypeLoc ElabTL = Builder.push<ElaboratedTypeLoc>(T);
  ElabTL.setElaboratedKeywordLoc(SourceLocation());
  ElabTL.setQualifierLoc(SS.getWithLocInContext(Context));
  return S.CreateParsedType(T, Builder.getTypeSourceInfo(Context, T));
}

Sema::NameClassification
Sema::ClassifyName(Scope *S, CXXScopeSpec &SS, IdentifierInfo *&Name,
                   SourceLocation NameLoc, const Token &NextToken,
                   bool IsAddressOfOperand,
                   std::unique_ptr<CorrectionCandidateCallback> CCC) {
  DeclarationNameInfo NameInfo(Name, NameLoc);
  ObjCMethodDecl *CurMethod = getCurMethodDecl();

  if (NextToken.is(tok::coloncolon)) {
    BuildCXXNestedNameSpecifier(S, *Name, NameLoc, NextToken.getLocation(),
                                QualType(), false, SS, nullptr, false);
  }

  LookupResult Result(*this, Name, NameLoc, LookupOrdinaryName);
  LookupParsedName(Result, S, &SS, !CurMethod);

  // For unqualified lookup in a class template in MSVC mode, look into
  // dependent base classes where the primary class template is known.
  if (Result.empty() && SS.isEmpty() && getLangOpts().MSVCCompat) {
    if (ParsedType TypeInBase =
            recoverFromTypeInKnownDependentBase(*this, *Name, NameLoc))
      return TypeInBase;
  }

  // Perform lookup for Objective-C instance variables (including automatically 
  // synthesized instance variables), if we're in an Objective-C method.
  // FIXME: This lookup really, really needs to be folded in to the normal
  // unqualified lookup mechanism.
  if (!SS.isSet() && CurMethod && !isResultTypeOrTemplate(Result, NextToken)) {
    ExprResult E = LookupInObjCMethod(Result, S, Name, true);
    if (E.get() || E.isInvalid())
      return E;
  }
  
  bool SecondTry = false;
  bool IsFilteredTemplateName = false;
  
Corrected:
  switch (Result.getResultKind()) {
  case LookupResult::NotFound:
    // If an unqualified-id is followed by a '(', then we have a function
    // call.
    if (!SS.isSet() && NextToken.is(tok::l_paren)) {
      // In C++, this is an ADL-only call.
      // FIXME: Reference?
      if (getLangOpts().CPlusPlus)
        return BuildDeclarationNameExpr(SS, Result, /*ADL=*/true);
      
      // C90 6.3.2.2:
      //   If the expression that precedes the parenthesized argument list in a 
      //   function call consists solely of an identifier, and if no 
      //   declaration is visible for this identifier, the identifier is 
      //   implicitly declared exactly as if, in the innermost block containing
      //   the function call, the declaration
      //
      //     extern int identifier (); 
      //
      //   appeared. 
      // 
      // We also allow this in C99 as an extension.
      if (NamedDecl *D = ImplicitlyDefineFunction(NameLoc, *Name, S)) {
        Result.addDecl(D);
        Result.resolveKind();
        return BuildDeclarationNameExpr(SS, Result, /*ADL=*/false);
      }
    }
    
    // In C, we first see whether there is a tag type by the same name, in 
    // which case it's likely that the user just forget to write "enum", 
    // "struct", or "union".
    if (!getLangOpts().CPlusPlus && !SecondTry &&
        isTagTypeWithMissingTag(*this, Result, S, SS, Name, NameLoc)) {
      break;
    }

    // Perform typo correction to determine if there is another name that is
    // close to this name.
    if (!SecondTry && CCC) {
      SecondTry = true;
      if (TypoCorrection Corrected = CorrectTypo(Result.getLookupNameInfo(),
                                                 Result.getLookupKind(), S, 
                                                 &SS, std::move(CCC),
                                                 CTK_ErrorRecovery)) {
        unsigned UnqualifiedDiag = diag::err_undeclared_var_use_suggest;
        unsigned QualifiedDiag = diag::err_no_member_suggest;

        NamedDecl *FirstDecl = Corrected.getCorrectionDecl();
        NamedDecl *UnderlyingFirstDecl
          = FirstDecl? FirstDecl->getUnderlyingDecl() : nullptr;
        if (getLangOpts().CPlusPlus && NextToken.is(tok::less) &&
            UnderlyingFirstDecl && isa<TemplateDecl>(UnderlyingFirstDecl)) {
          UnqualifiedDiag = diag::err_no_template_suggest;
          QualifiedDiag = diag::err_no_member_template_suggest;
        } else if (UnderlyingFirstDecl && 
                   (isa<TypeDecl>(UnderlyingFirstDecl) || 
                    isa<ObjCInterfaceDecl>(UnderlyingFirstDecl) ||
                    isa<ObjCCompatibleAliasDecl>(UnderlyingFirstDecl))) {
          UnqualifiedDiag = diag::err_unknown_typename_suggest;
          QualifiedDiag = diag::err_unknown_nested_typename_suggest;
        }

        if (SS.isEmpty()) {
          diagnoseTypo(Corrected, PDiag(UnqualifiedDiag) << Name);
        } else {// FIXME: is this even reachable? Test it.
          std::string CorrectedStr(Corrected.getAsString(getLangOpts()));
          bool DroppedSpecifier = Corrected.WillReplaceSpecifier() &&
                                  Name->getName().equals(CorrectedStr);
          diagnoseTypo(Corrected, PDiag(QualifiedDiag)
                                    << Name << computeDeclContext(SS, false)
                                    << DroppedSpecifier << SS.getRange());
        }

        // Update the name, so that the caller has the new name.
        Name = Corrected.getCorrectionAsIdentifierInfo();

        // Typo correction corrected to a keyword.
        if (Corrected.isKeyword())
          return Name;

        // Also update the LookupResult...
        // FIXME: This should probably go away at some point
        Result.clear();
        Result.setLookupName(Corrected.getCorrection());
        if (FirstDecl)
          Result.addDecl(FirstDecl);

        // If we found an Objective-C instance variable, let
        // LookupInObjCMethod build the appropriate expression to
        // reference the ivar.
        // FIXME: This is a gross hack.
        if (ObjCIvarDecl *Ivar = Result.getAsSingle<ObjCIvarDecl>()) {
          Result.clear();
          ExprResult E(LookupInObjCMethod(Result, S, Ivar->getIdentifier()));
          return E;
        }
        
        goto Corrected;
      }
    }
      
    // We failed to correct; just fall through and let the parser deal with it.
    Result.suppressDiagnostics();
    return NameClassification::Unknown();
      
  case LookupResult::NotFoundInCurrentInstantiation: {
    // We performed name lookup into the current instantiation, and there were 
    // dependent bases, so we treat this result the same way as any other
    // dependent nested-name-specifier.
      
    // C++ [temp.res]p2:
    //   A name used in a template declaration or definition and that is 
    //   dependent on a template-parameter is assumed not to name a type 
    //   unless the applicable name lookup finds a type name or the name is 
    //   qualified by the keyword typename.
    //
    // FIXME: If the next token is '<', we might want to ask the parser to
    // perform some heroics to see if we actually have a 
    // template-argument-list, which would indicate a missing 'template'
    // keyword here.
    return ActOnDependentIdExpression(SS, /*TemplateKWLoc=*/SourceLocation(),
                                      NameInfo, IsAddressOfOperand,
                                      /*TemplateArgs=*/nullptr);
  }

  case LookupResult::Found:
  case LookupResult::FoundOverloaded:
  case LookupResult::FoundUnresolvedValue:
    break;
      
  case LookupResult::Ambiguous:
    if (getLangOpts().CPlusPlus && NextToken.is(tok::less) &&
        hasAnyAcceptableTemplateNames(Result)) {
      // C++ [temp.local]p3:
      //   A lookup that finds an injected-class-name (10.2) can result in an
      //   ambiguity in certain cases (for example, if it is found in more than
      //   one base class). If all of the injected-class-names that are found
      //   refer to specializations of the same class template, and if the name
      //   is followed by a template-argument-list, the reference refers to the
      //   class template itself and not a specialization thereof, and is not
      //   ambiguous.
      //
      // This filtering can make an ambiguous result into an unambiguous one,
      // so try again after filtering out template names.
      FilterAcceptableTemplateNames(Result);
      if (!Result.isAmbiguous()) {
        IsFilteredTemplateName = true;
        break;
      }
    }
      
    // Diagnose the ambiguity and return an error.
    return NameClassification::Error();
  }
  
  if (getLangOpts().CPlusPlus && NextToken.is(tok::less) &&
      (IsFilteredTemplateName || hasAnyAcceptableTemplateNames(Result))) {
    // C++ [temp.names]p3:
    //   After name lookup (3.4) finds that a name is a template-name or that
    //   an operator-function-id or a literal- operator-id refers to a set of
    //   overloaded functions any member of which is a function template if 
    //   this is followed by a <, the < is always taken as the delimiter of a
    //   template-argument-list and never as the less-than operator.
    if (!IsFilteredTemplateName)
      FilterAcceptableTemplateNames(Result);
    
    if (!Result.empty()) {
      bool IsFunctionTemplate;
      bool IsVarTemplate;
      TemplateName Template;
      if (Result.end() - Result.begin() > 1) {
        IsFunctionTemplate = true;
        Template = Context.getOverloadedTemplateName(Result.begin(), 
                                                     Result.end());
      } else {
        TemplateDecl *TD
          = cast<TemplateDecl>((*Result.begin())->getUnderlyingDecl());
        IsFunctionTemplate = isa<FunctionTemplateDecl>(TD);
        IsVarTemplate = isa<VarTemplateDecl>(TD);

        if (SS.isSet() && !SS.isInvalid())
          Template = Context.getQualifiedTemplateName(SS.getScopeRep(), 
                                                    /*TemplateKeyword=*/false,
                                                      TD);
        else
          Template = TemplateName(TD);
      }
      
      if (IsFunctionTemplate) {
        // Function templates always go through overload resolution, at which
        // point we'll perform the various checks (e.g., accessibility) we need
        // to based on which function we selected.
        Result.suppressDiagnostics();
        
        return NameClassification::FunctionTemplate(Template);
      }

      return IsVarTemplate ? NameClassification::VarTemplate(Template)
                           : NameClassification::TypeTemplate(Template);
    }
  }

  NamedDecl *FirstDecl = (*Result.begin())->getUnderlyingDecl();
  if (TypeDecl *Type = dyn_cast<TypeDecl>(FirstDecl)) {
    DiagnoseUseOfDecl(Type, NameLoc);
    MarkAnyDeclReferenced(Type->getLocation(), Type, /*OdrUse=*/false);
    QualType T = Context.getTypeDeclType(Type);
    if (SS.isNotEmpty())
      return buildNestedType(*this, SS, T, NameLoc);
    return ParsedType::make(T);
  }

  ObjCInterfaceDecl *Class = dyn_cast<ObjCInterfaceDecl>(FirstDecl);
  if (!Class) {
    // FIXME: It's unfortunate that we don't have a Type node for handling this.
    if (ObjCCompatibleAliasDecl *Alias =
            dyn_cast<ObjCCompatibleAliasDecl>(FirstDecl))
      Class = Alias->getClassInterface();
  }
  
  if (Class) {
    DiagnoseUseOfDecl(Class, NameLoc);
    
    if (NextToken.is(tok::period)) {
      // Interface. <something> is parsed as a property reference expression.
      // Just return "unknown" as a fall-through for now.
      Result.suppressDiagnostics();
      return NameClassification::Unknown();
    }
    
    QualType T = Context.getObjCInterfaceType(Class);
    return ParsedType::make(T);
  }

  // We can have a type template here if we're classifying a template argument.
  if (isa<TemplateDecl>(FirstDecl) && !isa<FunctionTemplateDecl>(FirstDecl))
    return NameClassification::TypeTemplate(
        TemplateName(cast<TemplateDecl>(FirstDecl)));

  // Check for a tag type hidden by a non-type decl in a few cases where it
  // seems likely a type is wanted instead of the non-type that was found.
  bool NextIsOp = NextToken.isOneOf(tok::amp, tok::star);
  if ((NextToken.is(tok::identifier) ||
       (NextIsOp &&
        FirstDecl->getUnderlyingDecl()->isFunctionOrFunctionTemplate())) &&
      isTagTypeWithMissingTag(*this, Result, S, SS, Name, NameLoc)) {
    TypeDecl *Type = Result.getAsSingle<TypeDecl>();
    DiagnoseUseOfDecl(Type, NameLoc);
    QualType T = Context.getTypeDeclType(Type);
    if (SS.isNotEmpty())
      return buildNestedType(*this, SS, T, NameLoc);
    return ParsedType::make(T);
  }
  
  if (FirstDecl->isCXXClassMember())
    return BuildPossibleImplicitMemberExpr(SS, SourceLocation(), Result,
                                           nullptr);

  bool ADL = UseArgumentDependentLookup(SS, Result, NextToken.is(tok::l_paren));
  return BuildDeclarationNameExpr(SS, Result, ADL);
}

// Determines the context to return to after temporarily entering a
// context.  This depends in an unnecessarily complicated way on the
// exact ordering of callbacks from the parser.
DeclContext *Sema::getContainingDC(DeclContext *DC) {

  // Functions defined inline within classes aren't parsed until we've
  // finished parsing the top-level class, so the top-level class is
  // the context we'll need to return to.
  // A Lambda call operator whose parent is a class must not be treated 
  // as an inline member function.  A Lambda can be used legally
  // either as an in-class member initializer or a default argument.  These
  // are parsed once the class has been marked complete and so the containing
  // context would be the nested class (when the lambda is defined in one);
  // If the class is not complete, then the lambda is being used in an 
  // ill-formed fashion (such as to specify the width of a bit-field, or
  // in an array-bound) - in which case we still want to return the 
  // lexically containing DC (which could be a nested class). 
  if (isa<FunctionDecl>(DC) && !isLambdaCallOperator(DC)) {
    DC = DC->getLexicalParent();

    // A function not defined within a class will always return to its
    // lexical context.
    if (!isa<CXXRecordDecl>(DC))
      return DC;

    // A C++ inline method/friend is parsed *after* the topmost class
    // it was declared in is fully parsed ("complete");  the topmost
    // class is the context we need to return to.
    while (CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(DC->getLexicalParent()))
      DC = RD;

    // Return the declaration context of the topmost class the inline method is
    // declared in.
    return DC;
  }

  return DC->getLexicalParent();
}

void Sema::PushDeclContext(Scope *S, DeclContext *DC) {
  assert(getContainingDC(DC) == CurContext &&
      "The next DeclContext should be lexically contained in the current one.");
  CurContext = DC;
  S->setEntity(DC);
}

void Sema::PopDeclContext() {
  assert(CurContext && "DeclContext imbalance!");

  CurContext = getContainingDC(CurContext);
  assert(CurContext && "Popped translation unit!");
}

Sema::SkippedDefinitionContext Sema::ActOnTagStartSkippedDefinition(Scope *S,
                                                                    Decl *D) {
  // Unlike PushDeclContext, the context to which we return is not necessarily
  // the containing DC of TD, because the new context will be some pre-existing
  // TagDecl definition instead of a fresh one.
  auto Result = static_cast<SkippedDefinitionContext>(CurContext);
  CurContext = cast<TagDecl>(D)->getDefinition();
  assert(CurContext && "skipping definition of undefined tag");
  S->setEntity(CurContext);
  return Result;
}

void Sema::ActOnTagFinishSkippedDefinition(SkippedDefinitionContext Context) {
  CurContext = static_cast<decltype(CurContext)>(Context);
}

/// EnterDeclaratorContext - Used when we must lookup names in the context
/// of a declarator's nested name specifier.
///
void Sema::EnterDeclaratorContext(Scope *S, DeclContext *DC) {
  // C++0x [basic.lookup.unqual]p13:
  //   A name used in the definition of a static data member of class
  //   X (after the qualified-id of the static member) is looked up as
  //   if the name was used in a member function of X.
  // C++0x [basic.lookup.unqual]p14:
  //   If a variable member of a namespace is defined outside of the
  //   scope of its namespace then any name used in the definition of
  //   the variable member (after the declarator-id) is looked up as
  //   if the definition of the variable member occurred in its
  //   namespace.
  // Both of these imply that we should push a scope whose context
  // is the semantic context of the declaration.  We can't use
  // PushDeclContext here because that context is not necessarily
  // lexically contained in the current context.  Fortunately,
  // the containing scope should have the appropriate information.

  assert(!S->getEntity() && "scope already has entity");

#ifndef NDEBUG
  Scope *Ancestor = S->getParent();
  while (!Ancestor->getEntity()) Ancestor = Ancestor->getParent();
  assert(Ancestor->getEntity() == CurContext && "ancestor context mismatch");
#endif

  CurContext = DC;
  S->setEntity(DC);
}

void Sema::ExitDeclaratorContext(Scope *S) {
  assert(S->getEntity() == CurContext && "Context imbalance!");

  // Switch back to the lexical context.  The safety of this is
  // enforced by an assert in EnterDeclaratorContext.
  Scope *Ancestor = S->getParent();
  while (!Ancestor->getEntity()) Ancestor = Ancestor->getParent();
  CurContext = Ancestor->getEntity();

  // We don't need to do anything with the scope, which is going to
  // disappear.
}


void Sema::ActOnReenterFunctionContext(Scope* S, Decl *D) {
  // We assume that the caller has already called
  // ActOnReenterTemplateScope so getTemplatedDecl() works.
  FunctionDecl *FD = D->getAsFunction();
  if (!FD)
    return;

  // Same implementation as PushDeclContext, but enters the context
  // from the lexical parent, rather than the top-level class.
  assert(CurContext == FD->getLexicalParent() &&
    "The next DeclContext should be lexically contained in the current one.");
  CurContext = FD;
  S->setEntity(CurContext);

  for (unsigned P = 0, NumParams = FD->getNumParams(); P < NumParams; ++P) {
    ParmVarDecl *Param = FD->getParamDecl(P);
    // If the parameter has an identifier, then add it to the scope
    if (Param->getIdentifier()) {
      S->AddDecl(Param);
      IdResolver.AddDecl(Param);
    }
  }
}


void Sema::ActOnExitFunctionContext() {
  // Same implementation as PopDeclContext, but returns to the lexical parent,
  // rather than the top-level class.
  assert(CurContext && "DeclContext imbalance!");
  CurContext = CurContext->getLexicalParent();
  assert(CurContext && "Popped translation unit!");
}


/// \brief Determine whether we allow overloading of the function
/// PrevDecl with another declaration.
///
/// This routine determines whether overloading is possible, not
/// whether some new function is actually an overload. It will return
/// true in C++ (where we can always provide overloads) or, as an
/// extension, in C when the previous function is already an
/// overloaded function declaration or has the "overloadable"
/// attribute.
static bool AllowOverloadingOfFunction(LookupResult &Previous,
                                       ASTContext &Context) {
  if (Context.getLangOpts().CPlusPlus)
    return true;

  if (Previous.getResultKind() == LookupResult::FoundOverloaded)
    return true;

  return (Previous.getResultKind() == LookupResult::Found
          && Previous.getFoundDecl()->hasAttr<OverloadableAttr>());
}

/// Add this decl to the scope shadowed decl chains.
void Sema::PushOnScopeChains(NamedDecl *D, Scope *S, bool AddToContext) {
  // Move up the scope chain until we find the nearest enclosing
  // non-transparent context. The declaration will be introduced into this
  // scope.
  while (S->getEntity() && S->getEntity()->isTransparentContext())
    S = S->getParent();

  // Add scoped declarations into their context, so that they can be
  // found later. Declarations without a context won't be inserted
  // into any context.
  if (AddToContext)
    CurContext->addDecl(D);

  // Out-of-line definitions shouldn't be pushed into scope in C++, unless they
  // are function-local declarations.
  if (getLangOpts().CPlusPlus && D->isOutOfLine() &&
      !D->getDeclContext()->getRedeclContext()->Equals(
        D->getLexicalDeclContext()->getRedeclContext()) &&
      !D->getLexicalDeclContext()->isFunctionOrMethod())
    return;

  // Template instantiations should also not be pushed into scope.
  if (isa<FunctionDecl>(D) &&
      cast<FunctionDecl>(D)->isFunctionTemplateSpecialization())
    return;

  // If this replaces anything in the current scope, 
  IdentifierResolver::iterator I = IdResolver.begin(D->getDeclName()),
                               IEnd = IdResolver.end();
  for (; I != IEnd; ++I) {
    if (S->isDeclScope(*I) && D->declarationReplaces(*I)) {
      S->RemoveDecl(*I);
      IdResolver.RemoveDecl(*I);

      // Should only need to replace one decl.
      break;
    }
  }

  S->AddDecl(D);
  
  if (isa<LabelDecl>(D) && !cast<LabelDecl>(D)->isGnuLocal()) {
    // Implicitly-generated labels may end up getting generated in an order that
    // isn't strictly lexical, which breaks name lookup. Be careful to insert
    // the label at the appropriate place in the identifier chain.
    for (I = IdResolver.begin(D->getDeclName()); I != IEnd; ++I) {
      DeclContext *IDC = (*I)->getLexicalDeclContext()->getRedeclContext();
      if (IDC == CurContext) {
        if (!S->isDeclScope(*I))
          continue;
      } else if (IDC->Encloses(CurContext))
        break;
    }
    
    IdResolver.InsertDeclAfter(I, D);
  } else {
    IdResolver.AddDecl(D);
  }
}

void Sema::pushExternalDeclIntoScope(NamedDecl *D, DeclarationName Name) {
  if (IdResolver.tryAddTopLevelDecl(D, Name) && TUScope)
    TUScope->AddDecl(D);
}

bool Sema::isDeclInScope(NamedDecl *D, DeclContext *Ctx, Scope *S,
                         bool AllowInlineNamespace) {
  return IdResolver.isDeclInScope(D, Ctx, S, AllowInlineNamespace);
}

Scope *Sema::getScopeForDeclContext(Scope *S, DeclContext *DC) {
  DeclContext *TargetDC = DC->getPrimaryContext();
  do {
    if (DeclContext *ScopeDC = S->getEntity())
      if (ScopeDC->getPrimaryContext() == TargetDC)
        return S;
  } while ((S = S->getParent()));

  return nullptr;
}

static bool isOutOfScopePreviousDeclaration(NamedDecl *,
                                            DeclContext*,
                                            ASTContext&);

/// Filters out lookup results that don't fall within the given scope
/// as determined by isDeclInScope.
void Sema::FilterLookupForScope(LookupResult &R, DeclContext *Ctx, Scope *S,
                                bool ConsiderLinkage,
                                bool AllowInlineNamespace) {
  LookupResult::Filter F = R.makeFilter();
  while (F.hasNext()) {
    NamedDecl *D = F.next();

    if (isDeclInScope(D, Ctx, S, AllowInlineNamespace))
      continue;

    if (ConsiderLinkage && isOutOfScopePreviousDeclaration(D, Ctx, Context))
      continue;

    F.erase();
  }

  F.done();
}

static bool isUsingDecl(NamedDecl *D) {
  return isa<UsingShadowDecl>(D) ||
         isa<UnresolvedUsingTypenameDecl>(D) ||
         isa<UnresolvedUsingValueDecl>(D);
}

/// Removes using shadow declarations from the lookup results.
static void RemoveUsingDecls(LookupResult &R) {
  LookupResult::Filter F = R.makeFilter();
  while (F.hasNext())
    if (isUsingDecl(F.next()))
      F.erase();

  F.done();
}

/// \brief Check for this common pattern:
/// @code
/// class S {
///   S(const S&); // DO NOT IMPLEMENT
///   void operator=(const S&); // DO NOT IMPLEMENT
/// };
/// @endcode
static bool IsDisallowedCopyOrAssign(const CXXMethodDecl *D) {
  // FIXME: Should check for private access too but access is set after we get
  // the decl here.
  if (D->doesThisDeclarationHaveABody())
    return false;

  if (const CXXConstructorDecl *CD = dyn_cast<CXXConstructorDecl>(D))
    return CD->isCopyConstructor();
  if (const CXXMethodDecl *Method = dyn_cast<CXXMethodDecl>(D))
    return Method->isCopyAssignmentOperator();
  return false;
}

// We need this to handle
//
// typedef struct {
//   void *foo() { return 0; }
// } A;
//
// When we see foo we don't know if after the typedef we will get 'A' or '*A'
// for example. If 'A', foo will have external linkage. If we have '*A',
// foo will have no linkage. Since we can't know until we get to the end
// of the typedef, this function finds out if D might have non-external linkage.
// Callers should verify at the end of the TU if it D has external linkage or
// not.
bool Sema::mightHaveNonExternalLinkage(const DeclaratorDecl *D) {
  const DeclContext *DC = D->getDeclContext();
  while (!DC->isTranslationUnit()) {
    if (const RecordDecl *RD = dyn_cast<RecordDecl>(DC)){
      if (!RD->hasNameForLinkage())
        return true;
    }
    DC = DC->getParent();
  }

  return !D->isExternallyVisible();
}

// FIXME: This needs to be refactored; some other isInMainFile users want
// these semantics.
static bool isMainFileLoc(const Sema &S, SourceLocation Loc) {
  if (S.TUKind != TU_Complete)
    return false;
  return S.SourceMgr.isInMainFile(Loc);
}

bool Sema::ShouldWarnIfUnusedFileScopedDecl(const DeclaratorDecl *D) const {
  assert(D);

  if (D->isInvalidDecl() || D->isUsed() || D->hasAttr<UnusedAttr>())
    return false;

  // Ignore all entities declared within templates, and out-of-line definitions
  // of members of class templates.
  if (D->getDeclContext()->isDependentContext() ||
      D->getLexicalDeclContext()->isDependentContext())
    return false;

  if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(D)) {
    if (FD->getTemplateSpecializationKind() == TSK_ImplicitInstantiation)
      return false;

    if (const CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(FD)) {
      if (MD->isVirtual() || IsDisallowedCopyOrAssign(MD))
        return false;
    } else {
      // 'static inline' functions are defined in headers; don't warn.
      if (FD->isInlined() && !isMainFileLoc(*this, FD->getLocation()))
        return false;
    }

    if (FD->doesThisDeclarationHaveABody() &&
        Context.DeclMustBeEmitted(FD))
      return false;
  } else if (const VarDecl *VD = dyn_cast<VarDecl>(D)) {
    // Constants and utility variables are defined in headers with internal
    // linkage; don't warn.  (Unlike functions, there isn't a convenient marker
    // like "inline".)
    if (!isMainFileLoc(*this, VD->getLocation()))
      return false;

    if (Context.DeclMustBeEmitted(VD))
      return false;

    if (VD->isStaticDataMember() &&
        VD->getTemplateSpecializationKind() == TSK_ImplicitInstantiation)
      return false;
  } else {
    return false;
  }

  // Only warn for unused decls internal to the translation unit.
  // FIXME: This seems like a bogus check; it suppresses -Wunused-function
  // for inline functions defined in the main source file, for instance.
  return mightHaveNonExternalLinkage(D);
}

void Sema::MarkUnusedFileScopedDecl(const DeclaratorDecl *D) {
  if (!D)
    return;

  if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(D)) {
    const FunctionDecl *First = FD->getFirstDecl();
    if (FD != First && ShouldWarnIfUnusedFileScopedDecl(First))
      return; // First should already be in the vector.
  }

  if (const VarDecl *VD = dyn_cast<VarDecl>(D)) {
    const VarDecl *First = VD->getFirstDecl();
    if (VD != First && ShouldWarnIfUnusedFileScopedDecl(First))
      return; // First should already be in the vector.
  }

  if (ShouldWarnIfUnusedFileScopedDecl(D))
    UnusedFileScopedDecls.push_back(D);
}

static bool ShouldDiagnoseUnusedDecl(const NamedDecl *D) {
  if (D->isInvalidDecl())
    return false;

  if (D->isReferenced() || D->isUsed() || D->hasAttr<UnusedAttr>() ||
      D->hasAttr<ObjCPreciseLifetimeAttr>())
    return false;

  if (isa<LabelDecl>(D))
    return true;

  // Except for labels, we only care about unused decls that are local to
  // functions.
  bool WithinFunction = D->getDeclContext()->isFunctionOrMethod();
  if (const auto *R = dyn_cast<CXXRecordDecl>(D->getDeclContext()))
    // For dependent types, the diagnostic is deferred.
    WithinFunction =
        WithinFunction || (R->isLocalClass() && !R->isDependentType());
  if (!WithinFunction)
    return false;

  if (isa<TypedefNameDecl>(D))
    return true;
  
  // White-list anything that isn't a local variable.
  if (!isa<VarDecl>(D) || isa<ParmVarDecl>(D) || isa<ImplicitParamDecl>(D))
    return false;

  // Types of valid local variables should be complete, so this should succeed.
  if (const VarDecl *VD = dyn_cast<VarDecl>(D)) {

    // White-list anything with an __attribute__((unused)) type.
    QualType Ty = VD->getType();

    // Only look at the outermost level of typedef.
    if (const TypedefType *TT = Ty->getAs<TypedefType>()) {
      if (TT->getDecl()->hasAttr<UnusedAttr>())
        return false;
    }

    // If we failed to complete the type for some reason, or if the type is
    // dependent, don't diagnose the variable. 
    if (Ty->isIncompleteType() || Ty->isDependentType())
      return false;

    if (const TagType *TT = Ty->getAs<TagType>()) {
      const TagDecl *Tag = TT->getDecl();
      if (Tag->hasAttr<UnusedAttr>())
        return false;

      if (const CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(Tag)) {
        if (!RD->hasTrivialDestructor() && !RD->hasAttr<WarnUnusedAttr>())
          return false;

        if (const Expr *Init = VD->getInit()) {
          if (const ExprWithCleanups *Cleanups =
                  dyn_cast<ExprWithCleanups>(Init))
            Init = Cleanups->getSubExpr();
          const CXXConstructExpr *Construct =
            dyn_cast<CXXConstructExpr>(Init);
          if (Construct && !Construct->isElidable()) {
            CXXConstructorDecl *CD = Construct->getConstructor();
            if (!CD->isTrivial() && !RD->hasAttr<WarnUnusedAttr>())
              return false;
          }
        }
      }
    }

    // TODO: __attribute__((unused)) templates?
  }
  
  return true;
}

static void GenerateFixForUnusedDecl(const NamedDecl *D, ASTContext &Ctx,
                                     FixItHint &Hint) {
  if (isa<LabelDecl>(D)) {
    SourceLocation AfterColon = Lexer::findLocationAfterToken(D->getLocEnd(),
                tok::colon, Ctx.getSourceManager(), Ctx.getLangOpts(), true);
    if (AfterColon.isInvalid())
      return;
    Hint = FixItHint::CreateRemoval(CharSourceRange::
                                    getCharRange(D->getLocStart(), AfterColon));
  }
  return;
}

void Sema::DiagnoseUnusedNestedTypedefs(const RecordDecl *D) {
  if (D->getTypeForDecl()->isDependentType())
    return;

  for (auto *TmpD : D->decls()) {
    if (const auto *T = dyn_cast<TypedefNameDecl>(TmpD))
      DiagnoseUnusedDecl(T);
    else if(const auto *R = dyn_cast<RecordDecl>(TmpD))
      DiagnoseUnusedNestedTypedefs(R);
  }
}

/// DiagnoseUnusedDecl - Emit warnings about declarations that are not used
/// unless they are marked attr(unused).
void Sema::DiagnoseUnusedDecl(const NamedDecl *D) {
  if (!ShouldDiagnoseUnusedDecl(D))
    return;

  if (auto *TD = dyn_cast<TypedefNameDecl>(D)) {
    // typedefs can be referenced later on, so the diagnostics are emitted
    // at end-of-translation-unit.
    UnusedLocalTypedefNameCandidates.insert(TD);
    return;
  }
  
  FixItHint Hint;
  GenerateFixForUnusedDecl(D, Context, Hint);

  unsigned DiagID;
  if (isa<VarDecl>(D) && cast<VarDecl>(D)->isExceptionVariable())
    DiagID = diag::warn_unused_exception_param;
  else if (isa<LabelDecl>(D))
    DiagID = diag::warn_unused_label;
  else
    DiagID = diag::warn_unused_variable;

  Diag(D->getLocation(), DiagID) << D->getDeclName() << Hint;
}

static void CheckPoppedLabel(LabelDecl *L, Sema &S) {
  // Verify that we have no forward references left.  If so, there was a goto
  // or address of a label taken, but no definition of it.  Label fwd
  // definitions are indicated with a null substmt which is also not a resolved
  // MS inline assembly label name.
  bool Diagnose = false;
  if (L->isMSAsmLabel())
    Diagnose = !L->isResolvedMSAsmLabel();
  else
    Diagnose = L->getStmt() == nullptr;
  if (Diagnose)
    S.Diag(L->getLocation(), diag::err_undeclared_label_use) <<L->getDeclName();
}

void Sema::ActOnPopScope(SourceLocation Loc, Scope *S) {
  S->mergeNRVOIntoParent();

  if (S->decl_empty()) return;
  assert((S->getFlags() & (Scope::DeclScope | Scope::TemplateParamScope)) &&
         "Scope shouldn't contain decls!");

  for (auto *TmpD : S->decls()) {
    assert(TmpD && "This decl didn't get pushed??");

    assert(isa<NamedDecl>(TmpD) && "Decl isn't NamedDecl?");
    NamedDecl *D = cast<NamedDecl>(TmpD);

    if (!D->getDeclName()) continue;

    // Diagnose unused variables in this scope.
    if (!S->hasUnrecoverableErrorOccurred()) {
      DiagnoseUnusedDecl(D);
      if (const auto *RD = dyn_cast<RecordDecl>(D))
        DiagnoseUnusedNestedTypedefs(RD);
    }
    
    // If this was a forward reference to a label, verify it was defined.
    if (LabelDecl *LD = dyn_cast<LabelDecl>(D))
      CheckPoppedLabel(LD, *this);
    
    // Remove this name from our lexical scope.
    IdResolver.RemoveDecl(D);
  }
}

/// \brief Look for an Objective-C class in the translation unit.
///
/// \param Id The name of the Objective-C class we're looking for. If
/// typo-correction fixes this name, the Id will be updated
/// to the fixed name.
///
/// \param IdLoc The location of the name in the translation unit.
///
/// \param DoTypoCorrection If true, this routine will attempt typo correction
/// if there is no class with the given name.
///
/// \returns The declaration of the named Objective-C class, or NULL if the
/// class could not be found.
ObjCInterfaceDecl *Sema::getObjCInterfaceDecl(IdentifierInfo *&Id,
                                              SourceLocation IdLoc,
                                              bool DoTypoCorrection) {
  // The third "scope" argument is 0 since we aren't enabling lazy built-in
  // creation from this context.
  NamedDecl *IDecl = LookupSingleName(TUScope, Id, IdLoc, LookupOrdinaryName);

  if (!IDecl && DoTypoCorrection) {
    // Perform typo correction at the given location, but only if we
    // find an Objective-C class name.
    if (TypoCorrection C = CorrectTypo(
            DeclarationNameInfo(Id, IdLoc), LookupOrdinaryName, TUScope, nullptr,
            llvm::make_unique<DeclFilterCCC<ObjCInterfaceDecl>>(),
            CTK_ErrorRecovery)) {
      diagnoseTypo(C, PDiag(diag::err_undef_interface_suggest) << Id);
      IDecl = C.getCorrectionDeclAs<ObjCInterfaceDecl>();
      Id = IDecl->getIdentifier();
    }
  }
  ObjCInterfaceDecl *Def = dyn_cast_or_null<ObjCInterfaceDecl>(IDecl);
  // This routine must always return a class definition, if any.
  if (Def && Def->getDefinition())
      Def = Def->getDefinition();
  return Def;
}

/// getNonFieldDeclScope - Retrieves the innermost scope, starting
/// from S, where a non-field would be declared. This routine copes
/// with the difference between C and C++ scoping rules in structs and
/// unions. For example, the following code is well-formed in C but
/// ill-formed in C++:
/// @code
/// struct S6 {
///   enum { BAR } e;
/// };
///
/// void test_S6() {
///   struct S6 a;
///   a.e = BAR;
/// }
/// @endcode
/// For the declaration of BAR, this routine will return a different
/// scope. The scope S will be the scope of the unnamed enumeration
/// within S6. In C++, this routine will return the scope associated
/// with S6, because the enumeration's scope is a transparent
/// context but structures can contain non-field names. In C, this
/// routine will return the translation unit scope, since the
/// enumeration's scope is a transparent context and structures cannot
/// contain non-field names.
Scope *Sema::getNonFieldDeclScope(Scope *S) {
  while (((S->getFlags() & Scope::DeclScope) == 0) ||
         (S->getEntity() && S->getEntity()->isTransparentContext()) ||
         (S->isClassScope() && !getLangOpts().CPlusPlus))
    S = S->getParent();
  return S;
}

/// \brief Looks up the declaration of "struct objc_super" and
/// saves it for later use in building builtin declaration of
/// objc_msgSendSuper and objc_msgSendSuper_stret. If no such
/// pre-existing declaration exists no action takes place.
static void LookupPredefedObjCSuperType(Sema &ThisSema, Scope *S,
                                        IdentifierInfo *II) {
  if (!II->isStr("objc_msgSendSuper"))
    return;
  ASTContext &Context = ThisSema.Context;
    
  LookupResult Result(ThisSema, &Context.Idents.get("objc_super"),
                      SourceLocation(), Sema::LookupTagName);
  ThisSema.LookupName(Result, S);
  if (Result.getResultKind() == LookupResult::Found)
    if (const TagDecl *TD = Result.getAsSingle<TagDecl>())
      Context.setObjCSuperType(Context.getTagDeclType(TD));
}

static StringRef getHeaderName(ASTContext::GetBuiltinTypeError Error) {
  switch (Error) {
  case ASTContext::GE_None:
    return "";
  case ASTContext::GE_Missing_stdio:
    return "stdio.h";
  case ASTContext::GE_Missing_setjmp:
    return "setjmp.h";
  case ASTContext::GE_Missing_ucontext:
    return "ucontext.h";
  }
  llvm_unreachable("unhandled error kind");
}

/// LazilyCreateBuiltin - The specified Builtin-ID was first used at
/// file scope.  lazily create a decl for it. ForRedeclaration is true
/// if we're creating this built-in in anticipation of redeclaring the
/// built-in.
NamedDecl *Sema::LazilyCreateBuiltin(IdentifierInfo *II, unsigned ID,
                                     Scope *S, bool ForRedeclaration,
                                     SourceLocation Loc) {
  LookupPredefedObjCSuperType(*this, S, II);

  ASTContext::GetBuiltinTypeError Error;
  QualType R = Context.GetBuiltinType(ID, Error);
  if (Error) {
    if (ForRedeclaration)
      Diag(Loc, diag::warn_implicit_decl_requires_sysheader)
          << getHeaderName(Error)
          << Context.BuiltinInfo.GetName(ID);
    return nullptr;
  }

  if (!ForRedeclaration && Context.BuiltinInfo.isPredefinedLibFunction(ID)) {
    Diag(Loc, diag::ext_implicit_lib_function_decl)
      << Context.BuiltinInfo.GetName(ID)
      << R;
    if (Context.BuiltinInfo.getHeaderName(ID) &&
        !Diags.isIgnored(diag::ext_implicit_lib_function_decl, Loc))
      Diag(Loc, diag::note_include_header_or_declare)
          << Context.BuiltinInfo.getHeaderName(ID)
          << Context.BuiltinInfo.GetName(ID);
  }

  DeclContext *Parent = Context.getTranslationUnitDecl();
  if (getLangOpts().CPlusPlus) {
    LinkageSpecDecl *CLinkageDecl =
        LinkageSpecDecl::Create(Context, Parent, Loc, Loc,
                                LinkageSpecDecl::lang_c, false);
    CLinkageDecl->setImplicit();
    Parent->addDecl(CLinkageDecl);
    Parent = CLinkageDecl;
  }

  FunctionDecl *New = FunctionDecl::Create(Context,
                                           Parent,
                                           Loc, Loc, II, R, /*TInfo=*/nullptr,
                                           SC_Extern,
                                           false,
                                           R->isFunctionProtoType());
  New->setImplicit();

  // Create Decl objects for each parameter, adding them to the
  // FunctionDecl.
  if (const FunctionProtoType *FT = dyn_cast<FunctionProtoType>(R)) {
    SmallVector<ParmVarDecl*, 16> Params;
    for (unsigned i = 0, e = FT->getNumParams(); i != e; ++i) {
      ParmVarDecl *parm =
          ParmVarDecl::Create(Context, New, SourceLocation(), SourceLocation(),
                              nullptr, FT->getParamType(i), /*TInfo=*/nullptr,
                              SC_None, nullptr);
      parm->setScopeInfo(0, i);
      Params.push_back(parm);
    }
    New->setParams(Params);
  }

  AddKnownFunctionAttributes(New);
  RegisterLocallyScopedExternCDecl(New, S);

  // TUScope is the translation-unit scope to insert this function into.
  // FIXME: This is hideous. We need to teach PushOnScopeChains to
  // relate Scopes to DeclContexts, and probably eliminate CurContext
  // entirely, but we're not there yet.
  DeclContext *SavedContext = CurContext;
  CurContext = Parent;
  PushOnScopeChains(New, TUScope);
  CurContext = SavedContext;
  return New;
}

/// \brief Filter out any previous declarations that the given declaration
/// should not consider because they are not permitted to conflict, e.g.,
/// because they come from hidden sub-modules and do not refer to the same
/// entity.
static void filterNonConflictingPreviousDecls(Sema &S,
                                              NamedDecl *decl,
                                              LookupResult &previous){
  // This is only interesting when modules are enabled.
  if ((!S.getLangOpts().Modules && !S.getLangOpts().ModulesLocalVisibility) ||
      !S.getLangOpts().ModulesHideInternalLinkage)
    return;

  // Empty sets are uninteresting.
  if (previous.empty())
    return;

  LookupResult::Filter filter = previous.makeFilter();
  while (filter.hasNext()) {
    NamedDecl *old = filter.next();

    // Non-hidden declarations are never ignored.
    if (S.isVisible(old))
      continue;

    if (!old->isExternallyVisible())
      filter.erase();
  }

  filter.done();
}

/// Typedef declarations don't have linkage, but they still denote the same
/// entity if their types are the same.
/// FIXME: This is notionally doing the same thing as ASTReaderDecl's
/// isSameEntity.
static void filterNonConflictingPreviousTypedefDecls(Sema &S,
                                                     TypedefNameDecl *Decl,
                                                     LookupResult &Previous) {
  // This is only interesting when modules are enabled.
  if (!S.getLangOpts().Modules && !S.getLangOpts().ModulesLocalVisibility)
    return;

  // Empty sets are uninteresting.
  if (Previous.empty())
    return;

  LookupResult::Filter Filter = Previous.makeFilter();
  while (Filter.hasNext()) {
    NamedDecl *Old = Filter.next();

    // Non-hidden declarations are never ignored.
    if (S.isVisible(Old))
      continue;

    // Declarations of the same entity are not ignored, even if they have
    // different linkages.
    if (auto *OldTD = dyn_cast<TypedefNameDecl>(Old)) {
      if (S.Context.hasSameType(OldTD->getUnderlyingType(),
                                Decl->getUnderlyingType()))
        continue;

      // If both declarations give a tag declaration a typedef name for linkage
      // purposes, then they declare the same entity.
      if (OldTD->getAnonDeclWithTypedefName(/*AnyRedecl*/true) &&
          Decl->getAnonDeclWithTypedefName())
        continue;
    }

    if (!Old->isExternallyVisible())
      Filter.erase();
  }

  Filter.done();
}

bool Sema::isIncompatibleTypedef(TypeDecl *Old, TypedefNameDecl *New) {
  QualType OldType;
  if (TypedefNameDecl *OldTypedef = dyn_cast<TypedefNameDecl>(Old))
    OldType = OldTypedef->getUnderlyingType();
  else
    OldType = Context.getTypeDeclType(Old);
  QualType NewType = New->getUnderlyingType();

  if (NewType->isVariablyModifiedType()) {
    // Must not redefine a typedef with a variably-modified type.
    int Kind = isa<TypeAliasDecl>(Old) ? 1 : 0;
    Diag(New->getLocation(), diag::err_redefinition_variably_modified_typedef)
      << Kind << NewType;
    if (Old->getLocation().isValid())
      Diag(Old->getLocation(), diag::note_previous_definition);
    New->setInvalidDecl();
    return true;    
  }
  
  if (OldType != NewType &&
      !OldType->isDependentType() &&
      !NewType->isDependentType() &&
      !Context.hasSameType(OldType, NewType)) { 
    int Kind = isa<TypeAliasDecl>(Old) ? 1 : 0;
    Diag(New->getLocation(), diag::err_redefinition_different_typedef)
      << Kind << NewType << OldType;
    if (Old->getLocation().isValid())
      Diag(Old->getLocation(), diag::note_previous_definition);
    New->setInvalidDecl();
    return true;
  }
  return false;
}

/// MergeTypedefNameDecl - We just parsed a typedef 'New' which has the
/// same name and scope as a previous declaration 'Old'.  Figure out
/// how to resolve this situation, merging decls or emitting
/// diagnostics as appropriate. If there was an error, set New to be invalid.
///
void Sema::MergeTypedefNameDecl(TypedefNameDecl *New, LookupResult &OldDecls) {
  // If the new decl is known invalid already, don't bother doing any
  // merging checks.
  if (New->isInvalidDecl()) return;

  // Allow multiple definitions for ObjC built-in typedefs.
  // FIXME: Verify the underlying types are equivalent!
  if (getLangOpts().ObjC1) {
    const IdentifierInfo *TypeID = New->getIdentifier();
    switch (TypeID->getLength()) {
    default: break;
    case 2:
      {
        if (!TypeID->isStr("id"))
          break;
        QualType T = New->getUnderlyingType();
        if (!T->isPointerType())
          break;
        if (!T->isVoidPointerType()) {
          QualType PT = T->getAs<PointerType>()->getPointeeType();
          if (!PT->isStructureType())
            break;
        }
        Context.setObjCIdRedefinitionType(T);
        // Install the built-in type for 'id', ignoring the current definition.
        New->setTypeForDecl(Context.getObjCIdType().getTypePtr());
        return;
      }
    case 5:
      if (!TypeID->isStr("Class"))
        break;
      Context.setObjCClassRedefinitionType(New->getUnderlyingType());
      // Install the built-in type for 'Class', ignoring the current definition.
      New->setTypeForDecl(Context.getObjCClassType().getTypePtr());
      return;
    case 3:
      if (!TypeID->isStr("SEL"))
        break;
      Context.setObjCSelRedefinitionType(New->getUnderlyingType());
      // Install the built-in type for 'SEL', ignoring the current definition.
      New->setTypeForDecl(Context.getObjCSelType().getTypePtr());
      return;
    }
    // Fall through - the typedef name was not a builtin type.
  }

  // Verify the old decl was also a type.
  TypeDecl *Old = OldDecls.getAsSingle<TypeDecl>();
  if (!Old) {
    Diag(New->getLocation(), diag::err_redefinition_different_kind)
      << New->getDeclName();

    NamedDecl *OldD = OldDecls.getRepresentativeDecl();
    if (OldD->getLocation().isValid())
      Diag(OldD->getLocation(), diag::note_previous_definition);

    return New->setInvalidDecl();
  }

  // If the old declaration is invalid, just give up here.
  if (Old->isInvalidDecl())
    return New->setInvalidDecl();

  if (auto *OldTD = dyn_cast<TypedefNameDecl>(Old)) {
    auto *OldTag = OldTD->getAnonDeclWithTypedefName(/*AnyRedecl*/true);
    auto *NewTag = New->getAnonDeclWithTypedefName();
    NamedDecl *Hidden = nullptr;
    if (getLangOpts().CPlusPlus && OldTag && NewTag &&
        OldTag->getCanonicalDecl() != NewTag->getCanonicalDecl() &&
        !hasVisibleDefinition(OldTag, &Hidden)) {
      // There is a definition of this tag, but it is not visible. Use it
      // instead of our tag.
      New->setTypeForDecl(OldTD->getTypeForDecl());
      if (OldTD->isModed())
        New->setModedTypeSourceInfo(OldTD->getTypeSourceInfo(),
                                    OldTD->getUnderlyingType());
      else
        New->setTypeSourceInfo(OldTD->getTypeSourceInfo());

      // Make the old tag definition visible.
      makeMergedDefinitionVisible(Hidden, NewTag->getLocation());
    }
  }

  // If the typedef types are not identical, reject them in all languages and
  // with any extensions enabled.
  if (isIncompatibleTypedef(Old, New))
    return;

  // The types match.  Link up the redeclaration chain and merge attributes if
  // the old declaration was a typedef.
  if (TypedefNameDecl *Typedef = dyn_cast<TypedefNameDecl>(Old)) {
    New->setPreviousDecl(Typedef);
    mergeDeclAttributes(New, Old);
  }

  if (getLangOpts().MicrosoftExt)
    return;

  if (getLangOpts().CPlusPlus) {
    // C++ [dcl.typedef]p2:
    //   In a given non-class scope, a typedef specifier can be used to
    //   redefine the name of any type declared in that scope to refer
    //   to the type to which it already refers.
    if (!isa<CXXRecordDecl>(CurContext))
      return;

    // C++0x [dcl.typedef]p4:
    //   In a given class scope, a typedef specifier can be used to redefine 
    //   any class-name declared in that scope that is not also a typedef-name
    //   to refer to the type to which it already refers.
    //
    // This wording came in via DR424, which was a correction to the
    // wording in DR56, which accidentally banned code like:
    //
    //   struct S {
    //     typedef struct A { } A;
    //   };
    //
    // in the C++03 standard. We implement the C++0x semantics, which
    // allow the above but disallow
    //
    //   struct S {
    //     typedef int I;
    //     typedef int I;
    //   };
    //
    // since that was the intent of DR56.
    if (!isa<TypedefNameDecl>(Old))
      return;

    Diag(New->getLocation(), diag::err_redefinition)
      << New->getDeclName();
    Diag(Old->getLocation(), diag::note_previous_definition);
    return New->setInvalidDecl();
  }

  // Modules always permit redefinition of typedefs, as does C11.
  if (getLangOpts().Modules || getLangOpts().C11)
    return;
  
  // If we have a redefinition of a typedef in C, emit a warning.  This warning
  // is normally mapped to an error, but can be controlled with
  // -Wtypedef-redefinition.  If either the original or the redefinition is
  // in a system header, don't emit this for compatibility with GCC.
  if (getDiagnostics().getSuppressSystemWarnings() &&
      (Context.getSourceManager().isInSystemHeader(Old->getLocation()) ||
       Context.getSourceManager().isInSystemHeader(New->getLocation())))
    return;

  Diag(New->getLocation(), diag::ext_redefinition_of_typedef)
    << New->getDeclName();
  Diag(Old->getLocation(), diag::note_previous_definition);
}

/// DeclhasAttr - returns true if decl Declaration already has the target
/// attribute.
static bool DeclHasAttr(const Decl *D, const Attr *A) {
  const OwnershipAttr *OA = dyn_cast<OwnershipAttr>(A);
  const AnnotateAttr *Ann = dyn_cast<AnnotateAttr>(A);
  for (const auto *i : D->attrs())
    if (i->getKind() == A->getKind()) {
      if (Ann) {
        if (Ann->getAnnotation() == cast<AnnotateAttr>(i)->getAnnotation())
          return true;
        continue;
      }
      // FIXME: Don't hardcode this check
      if (OA && isa<OwnershipAttr>(i))
        return OA->getOwnKind() == cast<OwnershipAttr>(i)->getOwnKind();
      return true;
    }

  return false;
}

static bool isAttributeTargetADefinition(Decl *D) {
  if (VarDecl *VD = dyn_cast<VarDecl>(D))
    return VD->isThisDeclarationADefinition();
  if (TagDecl *TD = dyn_cast<TagDecl>(D))
    return TD->isCompleteDefinition() || TD->isBeingDefined();
  return true;
}

/// Merge alignment attributes from \p Old to \p New, taking into account the
/// special semantics of C11's _Alignas specifier and C++11's alignas attribute.
///
/// \return \c true if any attributes were added to \p New.
static bool mergeAlignedAttrs(Sema &S, NamedDecl *New, Decl *Old) {
  // Look for alignas attributes on Old, and pick out whichever attribute
  // specifies the strictest alignment requirement.
  AlignedAttr *OldAlignasAttr = nullptr;
  AlignedAttr *OldStrictestAlignAttr = nullptr;
  unsigned OldAlign = 0;
  for (auto *I : Old->specific_attrs<AlignedAttr>()) {
    // FIXME: We have no way of representing inherited dependent alignments
    // in a case like:
    //   template<int A, int B> struct alignas(A) X;
    //   template<int A, int B> struct alignas(B) X {};
    // For now, we just ignore any alignas attributes which are not on the
    // definition in such a case.
    if (I->isAlignmentDependent())
      return false;

    if (I->isAlignas())
      OldAlignasAttr = I;

    unsigned Align = I->getAlignment(S.Context);
    if (Align > OldAlign) {
      OldAlign = Align;
      OldStrictestAlignAttr = I;
    }
  }

  // Look for alignas attributes on New.
  AlignedAttr *NewAlignasAttr = nullptr;
  unsigned NewAlign = 0;
  for (auto *I : New->specific_attrs<AlignedAttr>()) {
    if (I->isAlignmentDependent())
      return false;

    if (I->isAlignas())
      NewAlignasAttr = I;

    unsigned Align = I->getAlignment(S.Context);
    if (Align > NewAlign)
      NewAlign = Align;
  }

  if (OldAlignasAttr && NewAlignasAttr && OldAlign != NewAlign) {
    // Both declarations have 'alignas' attributes. We require them to match.
    // C++11 [dcl.align]p6 and C11 6.7.5/7 both come close to saying this, but
    // fall short. (If two declarations both have alignas, they must both match
    // every definition, and so must match each other if there is a definition.)

    // If either declaration only contains 'alignas(0)' specifiers, then it
    // specifies the natural alignment for the type.
    if (OldAlign == 0 || NewAlign == 0) {
      QualType Ty;
      if (ValueDecl *VD = dyn_cast<ValueDecl>(New))
        Ty = VD->getType();
      else
        Ty = S.Context.getTagDeclType(cast<TagDecl>(New));

      if (OldAlign == 0)
        OldAlign = S.Context.getTypeAlign(Ty);
      if (NewAlign == 0)
        NewAlign = S.Context.getTypeAlign(Ty);
    }

    if (OldAlign != NewAlign) {
      S.Diag(NewAlignasAttr->getLocation(), diag::err_alignas_mismatch)
        << (unsigned)S.Context.toCharUnitsFromBits(OldAlign).getQuantity()
        << (unsigned)S.Context.toCharUnitsFromBits(NewAlign).getQuantity();
      S.Diag(OldAlignasAttr->getLocation(), diag::note_previous_declaration);
    }
  }

  if (OldAlignasAttr && !NewAlignasAttr && isAttributeTargetADefinition(New)) {
    // C++11 [dcl.align]p6:
    //   if any declaration of an entity has an alignment-specifier,
    //   every defining declaration of that entity shall specify an
    //   equivalent alignment.
    // C11 6.7.5/7:
    //   If the definition of an object does not have an alignment
    //   specifier, any other declaration of that object shall also
    //   have no alignment specifier.
    S.Diag(New->getLocation(), diag::err_alignas_missing_on_definition)
      << OldAlignasAttr;
    S.Diag(OldAlignasAttr->getLocation(), diag::note_alignas_on_declaration)
      << OldAlignasAttr;
  }

  bool AnyAdded = false;

  // Ensure we have an attribute representing the strictest alignment.
  if (OldAlign > NewAlign) {
    AlignedAttr *Clone = OldStrictestAlignAttr->clone(S.Context);
    Clone->setInherited(true);
    New->addAttr(Clone);
    AnyAdded = true;
  }

  // Ensure we have an alignas attribute if the old declaration had one.
  if (OldAlignasAttr && !NewAlignasAttr &&
      !(AnyAdded && OldStrictestAlignAttr->isAlignas())) {
    AlignedAttr *Clone = OldAlignasAttr->clone(S.Context);
    Clone->setInherited(true);
    New->addAttr(Clone);
    AnyAdded = true;
  }

  return AnyAdded;
}

static bool mergeDeclAttribute(Sema &S, NamedDecl *D,
                               const InheritableAttr *Attr, bool Override) {
  InheritableAttr *NewAttr = nullptr;
  unsigned AttrSpellingListIndex = Attr->getSpellingListIndex();
  if (const auto *AA = dyn_cast<AvailabilityAttr>(Attr))
    NewAttr = S.mergeAvailabilityAttr(D, AA->getRange(), AA->getPlatform(),
                                      AA->getIntroduced(), AA->getDeprecated(),
                                      AA->getObsoleted(), AA->getUnavailable(),
                                      AA->getMessage(), Override,
                                      AttrSpellingListIndex);
  else if (const auto *VA = dyn_cast<VisibilityAttr>(Attr))
    NewAttr = S.mergeVisibilityAttr(D, VA->getRange(), VA->getVisibility(),
                                    AttrSpellingListIndex);
  else if (const auto *VA = dyn_cast<TypeVisibilityAttr>(Attr))
    NewAttr = S.mergeTypeVisibilityAttr(D, VA->getRange(), VA->getVisibility(),
                                        AttrSpellingListIndex);
  else if (const auto *ImportA = dyn_cast<DLLImportAttr>(Attr))
    NewAttr = S.mergeDLLImportAttr(D, ImportA->getRange(),
                                   AttrSpellingListIndex);
  else if (const auto *ExportA = dyn_cast<DLLExportAttr>(Attr))
    NewAttr = S.mergeDLLExportAttr(D, ExportA->getRange(),
                                   AttrSpellingListIndex);
  else if (const auto *FA = dyn_cast<FormatAttr>(Attr))
    NewAttr = S.mergeFormatAttr(D, FA->getRange(), FA->getType(),
                                FA->getFormatIdx(), FA->getFirstArg(),
                                AttrSpellingListIndex);
  else if (const auto *SA = dyn_cast<SectionAttr>(Attr))
    NewAttr = S.mergeSectionAttr(D, SA->getRange(), SA->getName(),
                                 AttrSpellingListIndex);
  else if (const auto *IA = dyn_cast<MSInheritanceAttr>(Attr))
    NewAttr = S.mergeMSInheritanceAttr(D, IA->getRange(), IA->getBestCase(),
                                       AttrSpellingListIndex,
                                       IA->getSemanticSpelling());
  else if (const auto *AA = dyn_cast<AlwaysInlineAttr>(Attr))
    NewAttr = S.mergeAlwaysInlineAttr(D, AA->getRange(),
                                      &S.Context.Idents.get(AA->getSpelling()),
                                      AttrSpellingListIndex);
  else if (const auto *MA = dyn_cast<MinSizeAttr>(Attr))
    NewAttr = S.mergeMinSizeAttr(D, MA->getRange(), AttrSpellingListIndex);
  else if (const auto *OA = dyn_cast<OptimizeNoneAttr>(Attr))
    NewAttr = S.mergeOptimizeNoneAttr(D, OA->getRange(), AttrSpellingListIndex);
  else if (isa<AlignedAttr>(Attr))
    // AlignedAttrs are handled separately, because we need to handle all
    // such attributes on a declaration at the same time.
    NewAttr = nullptr;
  else if (isa<DeprecatedAttr>(Attr) && Override)
    NewAttr = nullptr;
  else if (Attr->duplicatesAllowed() || !DeclHasAttr(D, Attr))
    NewAttr = cast<InheritableAttr>(Attr->clone(S.Context));

  if (NewAttr) {
    NewAttr->setInherited(true);
    D->addAttr(NewAttr);
    return true;
  }

  return false;
}

static const Decl *getDefinition(const Decl *D) {
  if (const TagDecl *TD = dyn_cast<TagDecl>(D))
    return TD->getDefinition();
  if (const VarDecl *VD = dyn_cast<VarDecl>(D)) {
    const VarDecl *Def = VD->getDefinition();
    if (Def)
      return Def;
    return VD->getActingDefinition();
  }
  if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(D)) {
    const FunctionDecl* Def;
    if (FD->isDefined(Def))
      return Def;
  }
  return nullptr;
}

static bool hasAttribute(const Decl *D, attr::Kind Kind) {
  for (const auto *Attribute : D->attrs())
    if (Attribute->getKind() == Kind)
      return true;
  return false;
}

/// checkNewAttributesAfterDef - If we already have a definition, check that
/// there are no new attributes in this declaration.
static void checkNewAttributesAfterDef(Sema &S, Decl *New, const Decl *Old) {
  if (!New->hasAttrs())
    return;

  const Decl *Def = getDefinition(Old);
  if (!Def || Def == New)
    return;

  AttrVec &NewAttributes = New->getAttrs();
  for (unsigned I = 0, E = NewAttributes.size(); I != E;) {
    const Attr *NewAttribute = NewAttributes[I];

    if (isa<AliasAttr>(NewAttribute)) {
      if (FunctionDecl *FD = dyn_cast<FunctionDecl>(New))
        S.CheckForFunctionRedefinition(FD, cast<FunctionDecl>(Def));
      else {
        VarDecl *VD = cast<VarDecl>(New);
        unsigned Diag = cast<VarDecl>(Def)->isThisDeclarationADefinition() ==
                                VarDecl::TentativeDefinition
                            ? diag::err_alias_after_tentative
                            : diag::err_redefinition;
        S.Diag(VD->getLocation(), Diag) << VD->getDeclName();
        S.Diag(Def->getLocation(), diag::note_previous_definition);
        VD->setInvalidDecl();
      }
      ++I;
      continue;
    }

    if (const VarDecl *VD = dyn_cast<VarDecl>(Def)) {
      // Tentative definitions are only interesting for the alias check above.
      if (VD->isThisDeclarationADefinition() != VarDecl::Definition) {
        ++I;
        continue;
      }
    }

    if (hasAttribute(Def, NewAttribute->getKind())) {
      ++I;
      continue; // regular attr merging will take care of validating this.
    }

    if (isa<C11NoReturnAttr>(NewAttribute)) {
      // C's _Noreturn is allowed to be added to a function after it is defined.
      ++I;
      continue;
    } else if (const AlignedAttr *AA = dyn_cast<AlignedAttr>(NewAttribute)) {
      if (AA->isAlignas()) { 
        // C++11 [dcl.align]p6:
        //   if any declaration of an entity has an alignment-specifier,
        //   every defining declaration of that entity shall specify an
        //   equivalent alignment.
        // C11 6.7.5/7:
        //   If the definition of an object does not have an alignment
        //   specifier, any other declaration of that object shall also
        //   have no alignment specifier.
        S.Diag(Def->getLocation(), diag::err_alignas_missing_on_definition)
          << AA;
        S.Diag(NewAttribute->getLocation(), diag::note_alignas_on_declaration)
          << AA;
        NewAttributes.erase(NewAttributes.begin() + I);
        --E;
        continue;
      }
    }

    S.Diag(NewAttribute->getLocation(),
           diag::warn_attribute_precede_definition);
    S.Diag(Def->getLocation(), diag::note_previous_definition);
    NewAttributes.erase(NewAttributes.begin() + I);
    --E;
  }
}

/// mergeDeclAttributes - Copy attributes from the Old decl to the New one.
void Sema::mergeDeclAttributes(NamedDecl *New, Decl *Old,
                               AvailabilityMergeKind AMK) {
  if (UsedAttr *OldAttr = Old->getMostRecentDecl()->getAttr<UsedAttr>()) {
    UsedAttr *NewAttr = OldAttr->clone(Context);
    NewAttr->setInherited(true);
    New->addAttr(NewAttr);
  }

  if (!Old->hasAttrs() && !New->hasAttrs())
    return;

  // attributes declared post-definition are currently ignored
  checkNewAttributesAfterDef(*this, New, Old);

  if (!Old->hasAttrs())
    return;

  bool foundAny = New->hasAttrs();

  // Ensure that any moving of objects within the allocated map is done before
  // we process them.
  if (!foundAny) New->setAttrs(AttrVec());

  for (auto *I : Old->specific_attrs<InheritableAttr>()) {
    bool Override = false;
    // Ignore deprecated/unavailable/availability attributes if requested.
    if (isa<DeprecatedAttr>(I) ||
        isa<UnavailableAttr>(I) ||
        isa<AvailabilityAttr>(I)) {
      switch (AMK) {
      case AMK_None:
        continue;

      case AMK_Redeclaration:
        break;

      case AMK_Override:
        Override = true;
        break;
      }
    }

    // Already handled.
    if (isa<UsedAttr>(I))
      continue;

    if (mergeDeclAttribute(*this, New, I, Override))
      foundAny = true;
  }

  if (mergeAlignedAttrs(*this, New, Old))
    foundAny = true;

  if (!foundAny) New->dropAttrs();
}

/// mergeParamDeclAttributes - Copy attributes from the old parameter
/// to the new one.
static void mergeParamDeclAttributes(ParmVarDecl *newDecl,
                                     const ParmVarDecl *oldDecl,
                                     Sema &S) {
  // C++11 [dcl.attr.depend]p2:
  //   The first declaration of a function shall specify the
  //   carries_dependency attribute for its declarator-id if any declaration
  //   of the function specifies the carries_dependency attribute.
  const CarriesDependencyAttr *CDA = newDecl->getAttr<CarriesDependencyAttr>();
  if (CDA && !oldDecl->hasAttr<CarriesDependencyAttr>()) {
    S.Diag(CDA->getLocation(),
           diag::err_carries_dependency_missing_on_first_decl) << 1/*Param*/;
    // Find the first declaration of the parameter.
    // FIXME: Should we build redeclaration chains for function parameters?
    const FunctionDecl *FirstFD =
      cast<FunctionDecl>(oldDecl->getDeclContext())->getFirstDecl();
    const ParmVarDecl *FirstVD =
      FirstFD->getParamDecl(oldDecl->getFunctionScopeIndex());
    S.Diag(FirstVD->getLocation(),
           diag::note_carries_dependency_missing_first_decl) << 1/*Param*/;
  }

  if (!oldDecl->hasAttrs())
    return;

  bool foundAny = newDecl->hasAttrs();

  // Ensure that any moving of objects within the allocated map is
  // done before we process them.
  if (!foundAny) newDecl->setAttrs(AttrVec());

  for (const auto *I : oldDecl->specific_attrs<InheritableParamAttr>()) {
    if (!DeclHasAttr(newDecl, I)) {
      InheritableAttr *newAttr =
        cast<InheritableParamAttr>(I->clone(S.Context));
      newAttr->setInherited(true);
      newDecl->addAttr(newAttr);
      foundAny = true;
    }
  }

  if (!foundAny) newDecl->dropAttrs();
}

static void mergeParamDeclTypes(ParmVarDecl *NewParam,
                                const ParmVarDecl *OldParam,
                                Sema &S) {
  if (auto Oldnullability = OldParam->getType()->getNullability(S.Context)) {
    if (auto Newnullability = NewParam->getType()->getNullability(S.Context)) {
      if (*Oldnullability != *Newnullability) {
        S.Diag(NewParam->getLocation(), diag::warn_mismatched_nullability_attr)
          << DiagNullabilityKind(
               *Newnullability,
               ((NewParam->getObjCDeclQualifier() & Decl::OBJC_TQ_CSNullability)
                != 0))
          << DiagNullabilityKind(
               *Oldnullability,
               ((OldParam->getObjCDeclQualifier() & Decl::OBJC_TQ_CSNullability)
                != 0));
        S.Diag(OldParam->getLocation(), diag::note_previous_declaration);
      }
    } else {
      QualType NewT = NewParam->getType();
      NewT = S.Context.getAttributedType(
                         AttributedType::getNullabilityAttrKind(*Oldnullability),
                         NewT, NewT);
      NewParam->setType(NewT);
    }
  }
}

namespace {

/// Used in MergeFunctionDecl to keep track of function parameters in
/// C.
struct GNUCompatibleParamWarning {
  ParmVarDecl *OldParm;
  ParmVarDecl *NewParm;
  QualType PromotedType;
};

}

/// getSpecialMember - get the special member enum for a method.
Sema::CXXSpecialMember Sema::getSpecialMember(const CXXMethodDecl *MD) {
  if (const CXXConstructorDecl *Ctor = dyn_cast<CXXConstructorDecl>(MD)) {
    if (Ctor->isDefaultConstructor())
      return Sema::CXXDefaultConstructor;

    if (Ctor->isCopyConstructor())
      return Sema::CXXCopyConstructor;

    if (Ctor->isMoveConstructor())
      return Sema::CXXMoveConstructor;
  } else if (isa<CXXDestructorDecl>(MD)) {
    return Sema::CXXDestructor;
  } else if (MD->isCopyAssignmentOperator()) {
    return Sema::CXXCopyAssignment;
  } else if (MD->isMoveAssignmentOperator()) {
    return Sema::CXXMoveAssignment;
  }

  return Sema::CXXInvalid;
}

// Determine whether the previous declaration was a definition, implicit
// declaration, or a declaration.
template <typename T>
static std::pair<diag::kind, SourceLocation>
getNoteDiagForInvalidRedeclaration(const T *Old, const T *New) {
  diag::kind PrevDiag;
  SourceLocation OldLocation = Old->getLocation();
  if (Old->isThisDeclarationADefinition())
    PrevDiag = diag::note_previous_definition;
  else if (Old->isImplicit()) {
    PrevDiag = diag::note_previous_implicit_declaration;
    if (OldLocation.isInvalid())
      OldLocation = New->getLocation();
  } else
    PrevDiag = diag::note_previous_declaration;
  return std::make_pair(PrevDiag, OldLocation);
}

/// canRedefineFunction - checks if a function can be redefined. Currently,
/// only extern inline functions can be redefined, and even then only in
/// GNU89 mode.
static bool canRedefineFunction(const FunctionDecl *FD,
                                const LangOptions& LangOpts) {
  return ((FD->hasAttr<GNUInlineAttr>() || LangOpts.GNUInline) &&
          !LangOpts.CPlusPlus &&
          FD->isInlineSpecified() &&
          FD->getStorageClass() == SC_Extern);
}

const AttributedType *Sema::getCallingConvAttributedType(QualType T) const {
  const AttributedType *AT = T->getAs<AttributedType>();
  while (AT && !AT->isCallingConv())
    AT = AT->getModifiedType()->getAs<AttributedType>();
  return AT;
}

template <typename T>
static bool haveIncompatibleLanguageLinkages(const T *Old, const T *New) {
  const DeclContext *DC = Old->getDeclContext();
  if (DC->isRecord())
    return false;

  LanguageLinkage OldLinkage = Old->getLanguageLinkage();
  if (OldLinkage == CXXLanguageLinkage && New->isInExternCContext())
    return true;
  if (OldLinkage == CLanguageLinkage && New->isInExternCXXContext())
    return true;
  return false;
}

template<typename T> static bool isExternC(T *D) { return D->isExternC(); }
static bool isExternC(VarTemplateDecl *) { return false; }

/// \brief Check whether a redeclaration of an entity introduced by a
/// using-declaration is valid, given that we know it's not an overload
/// (nor a hidden tag declaration).
template<typename ExpectedDecl>
static bool checkUsingShadowRedecl(Sema &S, UsingShadowDecl *OldS,
                                   ExpectedDecl *New) {
  // C++11 [basic.scope.declarative]p4:
  //   Given a set of declarations in a single declarative region, each of
  //   which specifies the same unqualified name,
  //   -- they shall all refer to the same entity, or all refer to functions
  //      and function templates; or
  //   -- exactly one declaration shall declare a class name or enumeration
  //      name that is not a typedef name and the other declarations shall all
  //      refer to the same variable or enumerator, or all refer to functions
  //      and function templates; in this case the class name or enumeration
  //      name is hidden (3.3.10).

  // C++11 [namespace.udecl]p14:
  //   If a function declaration in namespace scope or block scope has the
  //   same name and the same parameter-type-list as a function introduced
  //   by a using-declaration, and the declarations do not declare the same
  //   function, the program is ill-formed.

  auto *Old = dyn_cast<ExpectedDecl>(OldS->getTargetDecl());
  if (Old &&
      !Old->getDeclContext()->getRedeclContext()->Equals(
          New->getDeclContext()->getRedeclContext()) &&
      !(isExternC(Old) && isExternC(New)))
    Old = nullptr;

  if (!Old) {
    S.Diag(New->getLocation(), diag::err_using_decl_conflict_reverse);
    S.Diag(OldS->getTargetDecl()->getLocation(), diag::note_using_decl_target);
    S.Diag(OldS->getUsingDecl()->getLocation(), diag::note_using_decl) << 0;
    return true;
  }
  return false;
}

/// MergeFunctionDecl - We just parsed a function 'New' from
/// declarator D which has the same name and scope as a previous
/// declaration 'Old'.  Figure out how to resolve this situation,
/// merging decls or emitting diagnostics as appropriate.
///
/// In C++, New and Old must be declarations that are not
/// overloaded. Use IsOverload to determine whether New and Old are
/// overloaded, and to select the Old declaration that New should be
/// merged with.
///
/// Returns true if there was an error, false otherwise.
bool Sema::MergeFunctionDecl(FunctionDecl *New, NamedDecl *&OldD,
                             Scope *S, bool MergeTypeWithOld) {
  // Verify the old decl was also a function.
  FunctionDecl *Old = OldD->getAsFunction();
  if (!Old) {
    if (UsingShadowDecl *Shadow = dyn_cast<UsingShadowDecl>(OldD)) {
      if (New->getFriendObjectKind()) {
        Diag(New->getLocation(), diag::err_using_decl_friend);
        Diag(Shadow->getTargetDecl()->getLocation(),
             diag::note_using_decl_target);
        Diag(Shadow->getUsingDecl()->getLocation(),
             diag::note_using_decl) << 0;
        return true;
      }

      // Check whether the two declarations might declare the same function.
      if (checkUsingShadowRedecl<FunctionDecl>(*this, Shadow, New))
        return true;
      OldD = Old = cast<FunctionDecl>(Shadow->getTargetDecl());
    } else {
      Diag(New->getLocation(), diag::err_redefinition_different_kind)
        << New->getDeclName();
      Diag(OldD->getLocation(), diag::note_previous_definition);
      return true;
    }
  }

  // If the old declaration is invalid, just give up here.
  if (Old->isInvalidDecl())
    return true;

  diag::kind PrevDiag;
  SourceLocation OldLocation;
  std::tie(PrevDiag, OldLocation) =
      getNoteDiagForInvalidRedeclaration(Old, New);

  // Don't complain about this if we're in GNU89 mode and the old function
  // is an extern inline function.
  // Don't complain about specializations. They are not supposed to have
  // storage classes.
  if (!isa<CXXMethodDecl>(New) && !isa<CXXMethodDecl>(Old) &&
      New->getStorageClass() == SC_Static &&
      Old->hasExternalFormalLinkage() &&
      !New->getTemplateSpecializationInfo() &&
      !canRedefineFunction(Old, getLangOpts())) {
    if (getLangOpts().MicrosoftExt) {
      Diag(New->getLocation(), diag::ext_static_non_static) << New;
      Diag(OldLocation, PrevDiag);
    } else {
      Diag(New->getLocation(), diag::err_static_non_static) << New;
      Diag(OldLocation, PrevDiag);
      return true;
    }
  }


  // If a function is first declared with a calling convention, but is later
  // declared or defined without one, all following decls assume the calling
  // convention of the first.
  //
  // It's OK if a function is first declared without a calling convention,
  // but is later declared or defined with the default calling convention.
  //
  // To test if either decl has an explicit calling convention, we look for
  // AttributedType sugar nodes on the type as written.  If they are missing or
  // were canonicalized away, we assume the calling convention was implicit.
  //
  // Note also that we DO NOT return at this point, because we still have
  // other tests to run.
  QualType OldQType = Context.getCanonicalType(Old->getType());
  QualType NewQType = Context.getCanonicalType(New->getType());
  const FunctionType *OldType = cast<FunctionType>(OldQType);
  const FunctionType *NewType = cast<FunctionType>(NewQType);
  FunctionType::ExtInfo OldTypeInfo = OldType->getExtInfo();
  FunctionType::ExtInfo NewTypeInfo = NewType->getExtInfo();
  bool RequiresAdjustment = false;

  if (OldTypeInfo.getCC() != NewTypeInfo.getCC()) {
    FunctionDecl *First = Old->getFirstDecl();
    const FunctionType *FT =
        First->getType().getCanonicalType()->castAs<FunctionType>();
    FunctionType::ExtInfo FI = FT->getExtInfo();
    bool NewCCExplicit = getCallingConvAttributedType(New->getType());
    if (!NewCCExplicit) {
      // Inherit the CC from the previous declaration if it was specified
      // there but not here.
      NewTypeInfo = NewTypeInfo.withCallingConv(OldTypeInfo.getCC());
      RequiresAdjustment = true;
    } else {
      // Calling conventions aren't compatible, so complain.
      bool FirstCCExplicit = getCallingConvAttributedType(First->getType());
      Diag(New->getLocation(), diag::err_cconv_change)
        << FunctionType::getNameForCallConv(NewTypeInfo.getCC())
        << !FirstCCExplicit
        << (!FirstCCExplicit ? "" :
            FunctionType::getNameForCallConv(FI.getCC()));

      // Put the note on the first decl, since it is the one that matters.
      Diag(First->getLocation(), diag::note_previous_declaration);
      return true;
    }
  }

  // FIXME: diagnose the other way around?
  if (OldTypeInfo.getNoReturn() && !NewTypeInfo.getNoReturn()) {
    NewTypeInfo = NewTypeInfo.withNoReturn(true);
    RequiresAdjustment = true;
  }

  // Merge regparm attribute.
  if (OldTypeInfo.getHasRegParm() != NewTypeInfo.getHasRegParm() ||
      OldTypeInfo.getRegParm() != NewTypeInfo.getRegParm()) {
    if (NewTypeInfo.getHasRegParm()) {
      Diag(New->getLocation(), diag::err_regparm_mismatch)
        << NewType->getRegParmType()
        << OldType->getRegParmType();
      Diag(OldLocation, diag::note_previous_declaration);
      return true;
    }

    NewTypeInfo = NewTypeInfo.withRegParm(OldTypeInfo.getRegParm());
    RequiresAdjustment = true;
  }

  // Merge ns_returns_retained attribute.
  if (OldTypeInfo.getProducesResult() != NewTypeInfo.getProducesResult()) {
    if (NewTypeInfo.getProducesResult()) {
      Diag(New->getLocation(), diag::err_returns_retained_mismatch);
      Diag(OldLocation, diag::note_previous_declaration);
      return true;
    }
    
    NewTypeInfo = NewTypeInfo.withProducesResult(true);
    RequiresAdjustment = true;
  }
  
  if (RequiresAdjustment) {
    const FunctionType *AdjustedType = New->getType()->getAs<FunctionType>();
    AdjustedType = Context.adjustFunctionType(AdjustedType, NewTypeInfo);
    New->setType(QualType(AdjustedType, 0));
    NewQType = Context.getCanonicalType(New->getType());
    NewType = cast<FunctionType>(NewQType);
  }

  // If this redeclaration makes the function inline, we may need to add it to
  // UndefinedButUsed.
  if (!Old->isInlined() && New->isInlined() &&
      !New->hasAttr<GNUInlineAttr>() &&
      !getLangOpts().GNUInline &&
      Old->isUsed(false) &&
      !Old->isDefined() && !New->isThisDeclarationADefinition())
    UndefinedButUsed.insert(std::make_pair(Old->getCanonicalDecl(),
                                           SourceLocation()));

  // If this redeclaration makes it newly gnu_inline, we don't want to warn
  // about it.
  if (New->hasAttr<GNUInlineAttr>() &&
      Old->isInlined() && !Old->hasAttr<GNUInlineAttr>()) {
    UndefinedButUsed.erase(Old->getCanonicalDecl());
  }
  
  if (getLangOpts().CPlusPlus) {
    // (C++98 13.1p2):
    //   Certain function declarations cannot be overloaded:
    //     -- Function declarations that differ only in the return type
    //        cannot be overloaded.

    // Go back to the type source info to compare the declared return types,
    // per C++1y [dcl.type.auto]p13:
    //   Redeclarations or specializations of a function or function template
    //   with a declared return type that uses a placeholder type shall also
    //   use that placeholder, not a deduced type.
    QualType OldDeclaredReturnType =
        (Old->getTypeSourceInfo()
             ? Old->getTypeSourceInfo()->getType()->castAs<FunctionType>()
             : OldType)->getReturnType();
    QualType NewDeclaredReturnType =
        (New->getTypeSourceInfo()
             ? New->getTypeSourceInfo()->getType()->castAs<FunctionType>()
             : NewType)->getReturnType();
    QualType ResQT;
    if (!Context.hasSameType(OldDeclaredReturnType, NewDeclaredReturnType) &&
        !((NewQType->isDependentType() || OldQType->isDependentType()) &&
          New->isLocalExternDecl())) {
      if (NewDeclaredReturnType->isObjCObjectPointerType() &&
          OldDeclaredReturnType->isObjCObjectPointerType())
        ResQT = Context.mergeObjCGCQualifiers(NewQType, OldQType);
      if (ResQT.isNull()) {
        if (New->isCXXClassMember() && New->isOutOfLine())
          Diag(New->getLocation(), diag::err_member_def_does_not_match_ret_type)
              << New << New->getReturnTypeSourceRange();
        else
          Diag(New->getLocation(), diag::err_ovl_diff_return_type)
              << New->getReturnTypeSourceRange();
        Diag(OldLocation, PrevDiag) << Old << Old->getType()
                                    << Old->getReturnTypeSourceRange();
        return true;
      }
      else
        NewQType = ResQT;
    }

    QualType OldReturnType = OldType->getReturnType();
    QualType NewReturnType = cast<FunctionType>(NewQType)->getReturnType();
    if (OldReturnType != NewReturnType) {
      // If this function has a deduced return type and has already been
      // defined, copy the deduced value from the old declaration.
      AutoType *OldAT = Old->getReturnType()->getContainedAutoType();
      if (OldAT && OldAT->isDeduced()) {
        New->setType(
            SubstAutoType(New->getType(),
                          OldAT->isDependentType() ? Context.DependentTy
                                                   : OldAT->getDeducedType()));
        NewQType = Context.getCanonicalType(
            SubstAutoType(NewQType,
                          OldAT->isDependentType() ? Context.DependentTy
                                                   : OldAT->getDeducedType()));
      }
    }

    const CXXMethodDecl *OldMethod = dyn_cast<CXXMethodDecl>(Old);
    CXXMethodDecl *NewMethod = dyn_cast<CXXMethodDecl>(New);
    if (OldMethod && NewMethod) {
      // Preserve triviality.
      NewMethod->setTrivial(OldMethod->isTrivial());

      // MSVC allows explicit template specialization at class scope:
      // 2 CXXMethodDecls referring to the same function will be injected.
      // We don't want a redeclaration error.
      bool IsClassScopeExplicitSpecialization =
                              OldMethod->isFunctionTemplateSpecialization() &&
                              NewMethod->isFunctionTemplateSpecialization();
      bool isFriend = NewMethod->getFriendObjectKind();

      if (!isFriend && NewMethod->getLexicalDeclContext()->isRecord() &&
          !IsClassScopeExplicitSpecialization) {
        //    -- Member function declarations with the same name and the
        //       same parameter types cannot be overloaded if any of them
        //       is a static member function declaration.
        if (OldMethod->isStatic() != NewMethod->isStatic()) {
          Diag(New->getLocation(), diag::err_ovl_static_nonstatic_member);
          Diag(OldLocation, PrevDiag) << Old << Old->getType();
          return true;
        }

        // C++ [class.mem]p1:
        //   [...] A member shall not be declared twice in the
        //   member-specification, except that a nested class or member
        //   class template can be declared and then later defined.
        if (ActiveTemplateInstantiations.empty()) {
          unsigned NewDiag;
          if (isa<CXXConstructorDecl>(OldMethod))
            NewDiag = diag::err_constructor_redeclared;
          else if (isa<CXXDestructorDecl>(NewMethod))
            NewDiag = diag::err_destructor_redeclared;
          else if (isa<CXXConversionDecl>(NewMethod))
            NewDiag = diag::err_conv_function_redeclared;
          else
            NewDiag = diag::err_member_redeclared;

          Diag(New->getLocation(), NewDiag);
        } else {
          Diag(New->getLocation(), diag::err_member_redeclared_in_instantiation)
            << New << New->getType();
        }
        Diag(OldLocation, PrevDiag) << Old << Old->getType();
        return true;

      // Complain if this is an explicit declaration of a special
      // member that was initially declared implicitly.
      //
      // As an exception, it's okay to befriend such methods in order
      // to permit the implicit constructor/destructor/operator calls.
      } else if (OldMethod->isImplicit()) {
        if (isFriend) {
          NewMethod->setImplicit();
        } else {
          Diag(NewMethod->getLocation(),
               diag::err_definition_of_implicitly_declared_member) 
            << New << getSpecialMember(OldMethod);
          return true;
        }
      } else if (OldMethod->isExplicitlyDefaulted() && !isFriend) {
        Diag(NewMethod->getLocation(),
             diag::err_definition_of_explicitly_defaulted_member)
          << getSpecialMember(OldMethod);
        return true;
      }
    }

    // C++11 [dcl.attr.noreturn]p1:
    //   The first declaration of a function shall specify the noreturn
    //   attribute if any declaration of that function specifies the noreturn
    //   attribute.
    const CXX11NoReturnAttr *NRA = New->getAttr<CXX11NoReturnAttr>();
    if (NRA && !Old->hasAttr<CXX11NoReturnAttr>()) {
      Diag(NRA->getLocation(), diag::err_noreturn_missing_on_first_decl);
      Diag(Old->getFirstDecl()->getLocation(),
           diag::note_noreturn_missing_first_decl);
    }

    // C++11 [dcl.attr.depend]p2:
    //   The first declaration of a function shall specify the
    //   carries_dependency attribute for its declarator-id if any declaration
    //   of the function specifies the carries_dependency attribute.
    const CarriesDependencyAttr *CDA = New->getAttr<CarriesDependencyAttr>();
    if (CDA && !Old->hasAttr<CarriesDependencyAttr>()) {
      Diag(CDA->getLocation(),
           diag::err_carries_dependency_missing_on_first_decl) << 0/*Function*/;
      Diag(Old->getFirstDecl()->getLocation(),
           diag::note_carries_dependency_missing_first_decl) << 0/*Function*/;
    }

    // (C++98 8.3.5p3):
    //   All declarations for a function shall agree exactly in both the
    //   return type and the parameter-type-list.
    // We also want to respect all the extended bits except noreturn.

    // noreturn should now match unless the old type info didn't have it.
    QualType OldQTypeForComparison = OldQType;
    if (!OldTypeInfo.getNoReturn() && NewTypeInfo.getNoReturn()) {
      assert(OldQType == QualType(OldType, 0));
      const FunctionType *OldTypeForComparison
        = Context.adjustFunctionType(OldType, OldTypeInfo.withNoReturn(true));
      OldQTypeForComparison = QualType(OldTypeForComparison, 0);
      assert(OldQTypeForComparison.isCanonical());
    }

    if (haveIncompatibleLanguageLinkages(Old, New)) {
      // As a special case, retain the language linkage from previous
      // declarations of a friend function as an extension.
      //
      // This liberal interpretation of C++ [class.friend]p3 matches GCC/MSVC
      // and is useful because there's otherwise no way to specify language
      // linkage within class scope.
      //
      // Check cautiously as the friend object kind isn't yet complete.
      if (New->getFriendObjectKind() != Decl::FOK_None) {
        Diag(New->getLocation(), diag::ext_retained_language_linkage) << New;
        Diag(OldLocation, PrevDiag);
      } else {
        Diag(New->getLocation(), diag::err_different_language_linkage) << New;
        Diag(OldLocation, PrevDiag);
        return true;
      }
    }

    if (OldQTypeForComparison == NewQType)
      return MergeCompatibleFunctionDecls(New, Old, S, MergeTypeWithOld);

    if ((NewQType->isDependentType() || OldQType->isDependentType()) &&
        New->isLocalExternDecl()) {
      // It's OK if we couldn't merge types for a local function declaraton
      // if either the old or new type is dependent. We'll merge the types
      // when we instantiate the function.
      return false;
    }

    // Fall through for conflicting redeclarations and redefinitions.
  }

  // C: Function types need to be compatible, not identical. This handles
  // duplicate function decls like "void f(int); void f(enum X);" properly.
  if (!getLangOpts().CPlusPlus &&
      Context.typesAreCompatible(OldQType, NewQType)) {
    const FunctionType *OldFuncType = OldQType->getAs<FunctionType>();
    const FunctionType *NewFuncType = NewQType->getAs<FunctionType>();
    const FunctionProtoType *OldProto = nullptr;
#if 1 // HLSL Change Starts - commenting this out rather than fixing for inout semantics - N/A for HLSL
    assert(!(MergeTypeWithOld && isa<FunctionNoProtoType>(NewFuncType) &&
             (OldProto = dyn_cast<FunctionProtoType>(OldFuncType))) &&
           "else fn with no prototype found");
    (void)OldFuncType;
    (void)NewFuncType;
    (void)OldProto;
#else
    if (MergeTypeWithOld && isa<FunctionNoProtoType>(NewFuncType) &&
        (OldProto = dyn_cast<FunctionProtoType>(OldFuncType))) {
      // The old declaration provided a function prototype, but the
      // new declaration does not. Merge in the prototype.
      assert(!OldProto->hasExceptionSpec() && "Exception spec in C");
      SmallVector<QualType, 16> ParamTypes(OldProto->param_types());
      NewQType =
          Context.getFunctionType(NewFuncType->getReturnType(), ParamTypes,
                                  OldProto->getExtProtoInfo());
      New->setType(NewQType);
      New->setHasInheritedPrototype();

      // Synthesize parameters with the same types.
      SmallVector<ParmVarDecl*, 16> Params;
      for (const auto &ParamType : OldProto->param_types()) {
        ParmVarDecl *Param = ParmVarDecl::Create(Context, New, SourceLocation(),
                                                 SourceLocation(), nullptr,
                                                 ParamType, /*TInfo=*/nullptr,
                                                 SC_None, nullptr);
        Param->setScopeInfo(0, Params.size());
        Param->setImplicit();
        Params.push_back(Param);
      }

      New->setParams(Params);
    }
#endif // HLSL Change Ends

    return MergeCompatibleFunctionDecls(New, Old, S, MergeTypeWithOld);
  }

#if 0 // HLSL Change Starts - commenting this out rather than fixing for inout semantics - N/A for HLSL
  // GNU C permits a K&R definition to follow a prototype declaration
  // if the declared types of the parameters in the K&R definition
  // match the types in the prototype declaration, even when the
  // promoted types of the parameters from the K&R definition differ
  // from the types in the prototype. GCC then keeps the types from
  // the prototype.
  //
  // If a variadic prototype is followed by a non-variadic K&R definition,
  // the K&R definition becomes variadic.  This is sort of an edge case, but
  // it's legal per the standard depending on how you read C99 6.7.5.3p15 and
  // C99 6.9.1p8.
  if (!getLangOpts().CPlusPlus &&
      Old->hasPrototype() && !New->hasPrototype() &&
      New->getType()->getAs<FunctionProtoType>() &&
      Old->getNumParams() == New->getNumParams()) {
    SmallVector<QualType, 16> ArgTypes;
    SmallVector<GNUCompatibleParamWarning, 16> Warnings;
    const FunctionProtoType *OldProto
      = Old->getType()->getAs<FunctionProtoType>();
    const FunctionProtoType *NewProto
      = New->getType()->getAs<FunctionProtoType>();

    // Determine whether this is the GNU C extension.
    QualType MergedReturn = Context.mergeTypes(OldProto->getReturnType(),
                                               NewProto->getReturnType());
    bool LooseCompatible = !MergedReturn.isNull();
    for (unsigned Idx = 0, End = Old->getNumParams();
         LooseCompatible && Idx != End; ++Idx) {
      ParmVarDecl *OldParm = Old->getParamDecl(Idx);
      ParmVarDecl *NewParm = New->getParamDecl(Idx);
      if (Context.typesAreCompatible(OldParm->getType(),
                                     NewProto->getParamType(Idx))) {
        ArgTypes.push_back(NewParm->getType());
      } else if (Context.typesAreCompatible(OldParm->getType(),
                                            NewParm->getType(),
                                            /*CompareUnqualified=*/true)) {
        GNUCompatibleParamWarning Warn = { OldParm, NewParm,
                                           NewProto->getParamType(Idx) };
        Warnings.push_back(Warn);
        ArgTypes.push_back(NewParm->getType());
      } else
        LooseCompatible = false;
    }

    if (LooseCompatible) {
      for (unsigned Warn = 0; Warn < Warnings.size(); ++Warn) {
        Diag(Warnings[Warn].NewParm->getLocation(),
             diag::ext_param_promoted_not_compatible_with_prototype)
          << Warnings[Warn].PromotedType
          << Warnings[Warn].OldParm->getType();
        if (Warnings[Warn].OldParm->getLocation().isValid())
          Diag(Warnings[Warn].OldParm->getLocation(),
               diag::note_previous_declaration);
      }

      if (MergeTypeWithOld)
        New->setType(Context.getFunctionType(MergedReturn, ArgTypes,
                                             OldProto->getExtProtoInfo()));
      return MergeCompatibleFunctionDecls(New, Old, S, MergeTypeWithOld);
    }

    // Fall through to diagnose conflicting types.
  }
#endif // HLSL Change Ends

  // A function that has already been declared has been redeclared or
  // defined with a different type; show an appropriate diagnostic.

  // If the previous declaration was an implicitly-generated builtin
  // declaration, then at the very least we should use a specialized note.
  unsigned BuiltinID;
  if (Old->isImplicit() && (BuiltinID = Old->getBuiltinID())) {
    // If it's actually a library-defined builtin function like 'malloc'
    // or 'printf', just warn about the incompatible redeclaration.
    if (Context.BuiltinInfo.isPredefinedLibFunction(BuiltinID)) {
      Diag(New->getLocation(), diag::warn_redecl_library_builtin) << New;
      Diag(OldLocation, diag::note_previous_builtin_declaration)
        << Old << Old->getType();

      // If this is a global redeclaration, just forget hereafter
      // about the "builtin-ness" of the function.
      //
      // Doing this for local extern declarations is problematic.  If
      // the builtin declaration remains visible, a second invalid
      // local declaration will produce a hard error; if it doesn't
      // remain visible, a single bogus local redeclaration (which is
      // actually only a warning) could break all the downstream code.
      if (!New->getLexicalDeclContext()->isFunctionOrMethod())
        New->getIdentifier()->setBuiltinID(Builtin::NotBuiltin);

      return false;
    }

    PrevDiag = diag::note_previous_builtin_declaration;
  }

  Diag(New->getLocation(), diag::err_conflicting_types) << New->getDeclName();
  Diag(OldLocation, PrevDiag) << Old << Old->getType();
  return true;
}

/// \brief Completes the merge of two function declarations that are
/// known to be compatible.
///
/// This routine handles the merging of attributes and other
/// properties of function declarations from the old declaration to
/// the new declaration, once we know that New is in fact a
/// redeclaration of Old.
///
/// \returns false
bool Sema::MergeCompatibleFunctionDecls(FunctionDecl *New, FunctionDecl *Old,
                                        Scope *S, bool MergeTypeWithOld) {
  // Merge the attributes
  mergeDeclAttributes(New, Old);

  // Merge "pure" flag.
  if (Old->isPure())
    New->setPure();

  // Merge "used" flag.
  if (Old->getMostRecentDecl()->isUsed(false))
    New->setIsUsed();

  // Merge attributes from the parameters.  These can mismatch with K&R
  // declarations.
  if (New->getNumParams() == Old->getNumParams())
      for (unsigned i = 0, e = New->getNumParams(); i != e; ++i) {
        ParmVarDecl *NewParam = New->getParamDecl(i);
        ParmVarDecl *OldParam = Old->getParamDecl(i);
        mergeParamDeclAttributes(NewParam, OldParam, *this);
        mergeParamDeclTypes(NewParam, OldParam, *this);
      }

  if (getLangOpts().CPlusPlus)
    return MergeCXXFunctionDecl(New, Old, S);

  // Merge the function types so the we get the composite types for the return
  // and argument types. Per C11 6.2.7/4, only update the type if the old decl
  // was visible.
  QualType Merged = Context.mergeTypes(Old->getType(), New->getType());
  if (!Merged.isNull() && MergeTypeWithOld)
    New->setType(Merged);

  return false;
}


void Sema::mergeObjCMethodDecls(ObjCMethodDecl *newMethod,
                                ObjCMethodDecl *oldMethod) {

  // Merge the attributes, including deprecated/unavailable
  AvailabilityMergeKind MergeKind =
    isa<ObjCImplDecl>(newMethod->getDeclContext()) ? AMK_Redeclaration
                                                   : AMK_Override;
  mergeDeclAttributes(newMethod, oldMethod, MergeKind);

  // Merge attributes from the parameters.
  ObjCMethodDecl::param_const_iterator oi = oldMethod->param_begin(),
                                       oe = oldMethod->param_end();
  for (ObjCMethodDecl::param_iterator
         ni = newMethod->param_begin(), ne = newMethod->param_end();
       ni != ne && oi != oe; ++ni, ++oi)
    mergeParamDeclAttributes(*ni, *oi, *this);

  CheckObjCMethodOverride(newMethod, oldMethod);
}

/// MergeVarDeclTypes - We parsed a variable 'New' which has the same name and
/// scope as a previous declaration 'Old'.  Figure out how to merge their types,
/// emitting diagnostics as appropriate.
///
/// Declarations using the auto type specifier (C++ [decl.spec.auto]) call back
/// to here in AddInitializerToDecl. We can't check them before the initializer
/// is attached.
void Sema::MergeVarDeclTypes(VarDecl *New, VarDecl *Old,
                             bool MergeTypeWithOld, ShadowMergeState& MergeState) { // HLSL Change - add merge state
  if (New->isInvalidDecl() || Old->isInvalidDecl())
    return;

  QualType MergedT;
  if (getLangOpts().CPlusPlus) {
    if (New->getType()->isUndeducedType()) {
      // We don't know what the new type is until the initializer is attached.
      return;
    } else if (Context.hasSameType(New->getType(), Old->getType())) {
      // These could still be something that needs exception specs checked.
      return MergeVarDeclExceptionSpecs(New, Old);
    }
    // C++ [basic.link]p10:
    //   [...] the types specified by all declarations referring to a given
    //   object or function shall be identical, except that declarations for an
    //   array object can specify array types that differ by the presence or
    //   absence of a major array bound (8.3.4).
    else if (Old->getType()->isIncompleteArrayType() &&
             New->getType()->isArrayType()) {
      const ArrayType *OldArray = Context.getAsArrayType(Old->getType());
      const ArrayType *NewArray = Context.getAsArrayType(New->getType());
      if (Context.hasSameType(OldArray->getElementType(),
                              NewArray->getElementType()))
        MergedT = New->getType();
    } else if (Old->getType()->isArrayType() &&
               New->getType()->isIncompleteArrayType()) {
      const ArrayType *OldArray = Context.getAsArrayType(Old->getType());
      const ArrayType *NewArray = Context.getAsArrayType(New->getType());
      if (Context.hasSameType(OldArray->getElementType(),
                              NewArray->getElementType()))
        MergedT = Old->getType();
    } else if (New->getType()->isObjCObjectPointerType() &&
               Old->getType()->isObjCObjectPointerType()) {
      MergedT = Context.mergeObjCGCQualifiers(New->getType(),
                                              Old->getType());
    }
  } else {
    // C 6.2.7p2:
    //   All declarations that refer to the same object or function shall have
    //   compatible type.
    MergedT = Context.mergeTypes(New->getType(), Old->getType());
  }
  if (MergedT.isNull()) {
    // It's OK if we couldn't merge types if either type is dependent, for a
    // block-scope variable. In other cases (static data members of class
    // templates, variable templates, ...), we require the types to be
    // equivalent.
    // FIXME: The C++ standard doesn't say anything about this.
    if ((New->getType()->isDependentType() ||
         Old->getType()->isDependentType()) && New->isLocalVarDecl()) {
      // If the old type was dependent, we can't merge with it, so the new type
      // becomes dependent for now. We'll reproduce the original type when we
      // instantiate the TypeSourceInfo for the variable.
      if (!New->getType()->isDependentType() && MergeTypeWithOld)
        New->setType(Context.DependentTy);
      return;
    }

    // FIXME: Even if this merging succeeds, some other non-visible declaration
    // of this variable might have an incompatible type. For instance:
    //
    //   extern int arr[];
    //   void f() { extern int arr[2]; }
    //   void g() { extern int arr[3]; }
    //
    // Neither C nor C++ requires a diagnostic for this, but we should still try
    // to diagnose it.
    // HLSL Change Starts
    if (MergeState == ShadowMergeState_Disallowed) {
      Diag(New->getLocation(), New->isThisDeclarationADefinition()
                                 ? diag::err_redefinition_different_type
                                 : diag::err_redeclaration_different_type)
        << New->getDeclName() << New->getType() << Old->getType();

    diag::kind PrevDiag;
    SourceLocation OldLocation;
    std::tie(PrevDiag, OldLocation) =
        getNoteDiagForInvalidRedeclaration(Old, New);
    Diag(OldLocation, PrevDiag);
    return New->setInvalidDecl();
  }
    else if (MergeState == ShadowMergeState_Possible) {
      Diag(New->getLocation(), diag::warn_hlsl_for_redefinition_different_type)
        << New->getDeclName() << New->getType() << Old->getType();
      Diag(Old->getLocation(), diag::note_previous_definition);
      MergeState = ShadowMergeState_Effective;
      MergeTypeWithOld = false;
    }
    // HLSL Change Ends
  }

  // Don't actually update the type on the new declaration if the old
  // declaration was an extern declaration in a different scope.
  if (MergeTypeWithOld)
    New->setType(MergedT);
}

static bool mergeTypeWithPrevious(Sema &S, VarDecl *NewVD, VarDecl *OldVD,
                                  LookupResult &Previous) {
  // C11 6.2.7p4:
  //   For an identifier with internal or external linkage declared
  //   in a scope in which a prior declaration of that identifier is
  //   visible, if the prior declaration specifies internal or
  //   external linkage, the type of the identifier at the later
  //   declaration becomes the composite type.
  //
  // If the variable isn't visible, we do not merge with its type.
  if (Previous.isShadowed())
    return false;

  if (S.getLangOpts().CPlusPlus) {
    // C++11 [dcl.array]p3:
    //   If there is a preceding declaration of the entity in the same
    //   scope in which the bound was specified, an omitted array bound
    //   is taken to be the same as in that earlier declaration.
    return NewVD->isPreviousDeclInSameBlockScope() ||
           (!OldVD->getLexicalDeclContext()->isFunctionOrMethod() &&
            !NewVD->getLexicalDeclContext()->isFunctionOrMethod());
  } else {
    // If the old declaration was function-local, don't merge with its
    // type unless we're in the same function.
    return !OldVD->getLexicalDeclContext()->isFunctionOrMethod() ||
           OldVD->getLexicalDeclContext() == NewVD->getLexicalDeclContext();
  }
}

/// MergeVarDecl - We just parsed a variable 'New' which has the same name
/// and scope as a previous declaration 'Old'.  Figure out how to resolve this
/// situation, merging decls or emitting diagnostics as appropriate.
///
/// Tentative definition rules (C99 6.9.2p2) are checked by
/// FinalizeDeclaratorGroup. Unfortunately, we can't analyze tentative
/// definitions here, since the initializer hasn't been attached.
///
void Sema::MergeVarDecl(VarDecl *New, LookupResult &Previous, ShadowMergeState& MergeState) { // HLSL Change - add merge state
  // If the new decl is already invalid, don't do any other checking.
  if (New->isInvalidDecl())
    return;

  VarTemplateDecl *NewTemplate = New->getDescribedVarTemplate();

  // Verify the old decl was also a variable or variable template.
  VarDecl *Old = nullptr;
  VarTemplateDecl *OldTemplate = nullptr;
  if (Previous.isSingleResult()) {
    if (NewTemplate) {
      OldTemplate = dyn_cast<VarTemplateDecl>(Previous.getFoundDecl());
      Old = OldTemplate ? OldTemplate->getTemplatedDecl() : nullptr;

      if (auto *Shadow =
              dyn_cast<UsingShadowDecl>(Previous.getRepresentativeDecl()))
        if (checkUsingShadowRedecl<VarTemplateDecl>(*this, Shadow, NewTemplate))
          return New->setInvalidDecl();
    } else {
      Old = dyn_cast<VarDecl>(Previous.getFoundDecl());

      if (auto *Shadow =
              dyn_cast<UsingShadowDecl>(Previous.getRepresentativeDecl()))
        if (checkUsingShadowRedecl<VarDecl>(*this, Shadow, New))
          return New->setInvalidDecl();
    }
  }
  if (!Old) {
    Diag(New->getLocation(), diag::err_redefinition_different_kind)
      << New->getDeclName();
    Diag(Previous.getRepresentativeDecl()->getLocation(),
         diag::note_previous_definition);
    return New->setInvalidDecl();
  }

  if (!shouldLinkPossiblyHiddenDecl(Old, New))
    return;

  // Ensure the template parameters are compatible.
  if (NewTemplate &&
      !TemplateParameterListsAreEqual(NewTemplate->getTemplateParameters(),
                                      OldTemplate->getTemplateParameters(),
                                      /*Complain=*/true, TPL_TemplateMatch))
    return;

  // C++ [class.mem]p1:
  //   A member shall not be declared twice in the member-specification [...]
  // 
  // Here, we need only consider static data members.
  if (Old->isStaticDataMember() && !New->isOutOfLine()) {
    Diag(New->getLocation(), diag::err_duplicate_member) 
      << New->getIdentifier();
    Diag(Old->getLocation(), diag::note_previous_declaration);
    New->setInvalidDecl();
  }
  
  mergeDeclAttributes(New, Old);
  // Warn if an already-declared variable is made a weak_import in a subsequent 
  // declaration
  if (New->hasAttr<WeakImportAttr>() &&
      Old->getStorageClass() == SC_None &&
      !Old->hasAttr<WeakImportAttr>()) {
    Diag(New->getLocation(), diag::warn_weak_import) << New->getDeclName();
    Diag(Old->getLocation(), diag::note_previous_definition);
    // Remove weak_import attribute on new declaration.
    New->dropAttr<WeakImportAttr>();
  }

  // Merge the types.
  VarDecl *MostRecent = Old->getMostRecentDecl();
  if (MostRecent != Old) {
    MergeVarDeclTypes(New, MostRecent,
                      mergeTypeWithPrevious(*this, New, MostRecent, Previous), MergeState); // HLSL Change - add MergeState
    if (New->isInvalidDecl())
      return;
  }

  MergeVarDeclTypes(New, Old, mergeTypeWithPrevious(*this, New, Old, Previous), MergeState); // HLSL Change - add MergeState
  if (New->isInvalidDecl())
    return;

  diag::kind PrevDiag;
  SourceLocation OldLocation;
  std::tie(PrevDiag, OldLocation) =
      getNoteDiagForInvalidRedeclaration(Old, New);

  // [dcl.stc]p8: Check if we have a non-static decl followed by a static.
  if (New->getStorageClass() == SC_Static &&
      !New->isStaticDataMember() &&
      Old->hasExternalFormalLinkage()) {
    if (getLangOpts().MicrosoftExt) {
      Diag(New->getLocation(), diag::ext_static_non_static)
          << New->getDeclName();
      Diag(OldLocation, PrevDiag);
    } else {
      Diag(New->getLocation(), diag::err_static_non_static)
          << New->getDeclName();
      Diag(OldLocation, PrevDiag);
      return New->setInvalidDecl();
    }
  }
  // C99 6.2.2p4:
  //   For an identifier declared with the storage-class specifier
  //   extern in a scope in which a prior declaration of that
  //   identifier is visible,23) if the prior declaration specifies
  //   internal or external linkage, the linkage of the identifier at
  //   the later declaration is the same as the linkage specified at
  //   the prior declaration. If no prior declaration is visible, or
  //   if the prior declaration specifies no linkage, then the
  //   identifier has external linkage.
  if (New->hasExternalStorage() && Old->hasLinkage())
    /* Okay */;
  else if (New->getCanonicalDecl()->getStorageClass() != SC_Static &&
           !New->isStaticDataMember() &&
           Old->getCanonicalDecl()->getStorageClass() == SC_Static) {
    Diag(New->getLocation(), diag::err_non_static_static) << New->getDeclName();
    Diag(OldLocation, PrevDiag);
    return New->setInvalidDecl();
  }

  // Check if extern is followed by non-extern and vice-versa.
  if (New->hasExternalStorage() &&
      !Old->hasLinkage() && Old->isLocalVarDeclOrParm()) {
    Diag(New->getLocation(), diag::err_extern_non_extern) << New->getDeclName();
    Diag(OldLocation, PrevDiag);
    return New->setInvalidDecl();
  }
  if (Old->hasLinkage() && New->isLocalVarDeclOrParm() &&
      !New->hasExternalStorage()) {
    Diag(New->getLocation(), diag::err_non_extern_extern) << New->getDeclName();
    Diag(OldLocation, PrevDiag);
    return New->setInvalidDecl();
  }

  // Variables with external linkage are analyzed in FinalizeDeclaratorGroup.

  // FIXME: The test for external storage here seems wrong? We still
  // need to check for mismatches.
  if (!New->hasExternalStorage() && !New->isFileVarDecl() &&
      // Don't complain about out-of-line definitions of static members.
      !(Old->getLexicalDeclContext()->isRecord() &&
        !New->getLexicalDeclContext()->isRecord())) {
    // HLSL Change Starts
    if (MergeState == ShadowMergeState_Disallowed) {
      Diag(New->getLocation(), diag::err_redefinition) << New->getDeclName();
      Diag(OldLocation, PrevDiag);
      return New->setInvalidDecl();
    } else if (MergeState == ShadowMergeState_Possible) {
      Diag(New->getLocation(), diag::warn_hlsl_for_redefinition)
          << New->getDeclName();
      Diag(Old->getLocation(), diag::note_previous_definition);
      MergeState = ShadowMergeState_Effective;
    }
    // HLSL Change Ends
  }

  if (New->getTLSKind() != Old->getTLSKind()) {
    if (!Old->getTLSKind()) {
      Diag(New->getLocation(), diag::err_thread_non_thread) << New->getDeclName();
      Diag(OldLocation, PrevDiag);
    } else if (!New->getTLSKind()) {
      Diag(New->getLocation(), diag::err_non_thread_thread) << New->getDeclName();
      Diag(OldLocation, PrevDiag);
    } else {
      // Do not allow redeclaration to change the variable between requiring
      // static and dynamic initialization.
      // FIXME: GCC allows this, but uses the TLS keyword on the first
      // declaration to determine the kind. Do we need to be compatible here?
      Diag(New->getLocation(), diag::err_thread_thread_different_kind)
        << New->getDeclName() << (New->getTLSKind() == VarDecl::TLS_Dynamic);
      Diag(OldLocation, PrevDiag);
    }
  }

  // C++ doesn't have tentative definitions, so go right ahead and check here.
  VarDecl *Def;
  if (getLangOpts().CPlusPlus &&
      New->isThisDeclarationADefinition() == VarDecl::Definition &&
      (Def = Old->getDefinition())) {
    NamedDecl *Hidden = nullptr;
    if (!hasVisibleDefinition(Def, &Hidden) &&
        (New->getFormalLinkage() == InternalLinkage ||
         New->getDescribedVarTemplate() ||
         New->getNumTemplateParameterLists() ||
         New->getDeclContext()->isDependentContext())) {
      // The previous definition is hidden, and multiple definitions are
      // permitted (in separate TUs). Form another definition of it.
    } else {
      // HLSL Change Starts
      if (MergeState == ShadowMergeState_Disallowed) {
        Diag(New->getLocation(), diag::err_redefinition) << New;
        Diag(Def->getLocation(), diag::note_previous_definition);
        New->setInvalidDecl();
        return;
      }
      else if (MergeState == ShadowMergeState_Possible) {
        Diag(New->getLocation(), diag::warn_hlsl_for_redefinition) << New->getDeclName();
        Diag(Old->getLocation(), diag::note_previous_definition);
        MergeState = ShadowMergeState_Effective;
      }
      // HLSL Change Ends
    }
  }

  if (haveIncompatibleLanguageLinkages(Old, New)) {
    Diag(New->getLocation(), diag::err_different_language_linkage) << New;
    Diag(OldLocation, PrevDiag);
    New->setInvalidDecl();
    return;
  }

  if (MergeState != ShadowMergeState_Effective) { // HLSL Change - avoid merging when shadowing
  // Merge "used" flag.
  if (Old->getMostRecentDecl()->isUsed(false))
    New->setIsUsed();

  // Keep a chain of previous declarations.
  New->setPreviousDecl(Old);
  if (NewTemplate)
    NewTemplate->setPreviousDecl(OldTemplate);

  // Inherit access appropriately.
  New->setAccess(Old->getAccess());
  if (NewTemplate)
    NewTemplate->setAccess(New->getAccess());
  } // HLSL Change - close block conditional on shadow merge state
}

/// ParsedFreeStandingDeclSpec - This method is invoked when a declspec with
/// no declarator (e.g. "struct foo;") is parsed.
Decl *Sema::ParsedFreeStandingDeclSpec(Scope *S, AccessSpecifier AS,
                                       DeclSpec &DS) {
  return ParsedFreeStandingDeclSpec(S, AS, DS, MultiTemplateParamsArg());
}

// The MS ABI changed between VS2013 and VS2015 with regard to numbers used to
// disambiguate entities defined in different scopes.
// While the VS2015 ABI fixes potential miscompiles, it is also breaks
// compatibility.
// We will pick our mangling number depending on which version of MSVC is being
// targeted.
static unsigned getMSManglingNumber(const LangOptions &LO, Scope *S) {
  return LO.isCompatibleWithMSVC(LangOptions::MSVC2015)
             ? S->getMSCurManglingNumber()
             : S->getMSLastManglingNumber();
}

void Sema::handleTagNumbering(const TagDecl *Tag, Scope *TagScope) {
  if (!Context.getLangOpts().CPlusPlus)
    return;

  if (isa<CXXRecordDecl>(Tag->getParent())) {
    // If this tag is the direct child of a class, number it if
    // it is anonymous.
    if (!Tag->getName().empty() || Tag->getTypedefNameForAnonDecl())
      return;
    MangleNumberingContext &MCtx =
        Context.getManglingNumberContext(Tag->getParent());
    Context.setManglingNumber(
        Tag, MCtx.getManglingNumber(
                 Tag, getMSManglingNumber(getLangOpts(), TagScope)));
    return;
  }

  // If this tag isn't a direct child of a class, number it if it is local.
  Decl *ManglingContextDecl;
  if (MangleNumberingContext *MCtx = getCurrentMangleNumberContext(
          Tag->getDeclContext(), ManglingContextDecl)) {
    Context.setManglingNumber(
        Tag, MCtx->getManglingNumber(
                 Tag, getMSManglingNumber(getLangOpts(), TagScope)));
  }
}

void Sema::setTagNameForLinkagePurposes(TagDecl *TagFromDeclSpec,
                                        TypedefNameDecl *NewTD) {
  // Do nothing if the tag is not anonymous or already has an
  // associated typedef (from an earlier typedef in this decl group).
  if (TagFromDeclSpec->getIdentifier())
    return;
  if (TagFromDeclSpec->getTypedefNameForAnonDecl())
    return;

  // A well-formed anonymous tag must always be a TUK_Definition.
  assert(TagFromDeclSpec->isThisDeclarationADefinition());

  // The type must match the tag exactly;  no qualifiers allowed.
  if (!Context.hasSameType(NewTD->getUnderlyingType(),
                           Context.getTagDeclType(TagFromDeclSpec)))
    return;

  // If we've already computed linkage for the anonymous tag, then
  // adding a typedef name for the anonymous decl can change that
  // linkage, which might be a serious problem.  Diagnose this as
  // unsupported and ignore the typedef name.  TODO: we should
  // pursue this as a language defect and establish a formal rule
  // for how to handle it.
  if (TagFromDeclSpec->hasLinkageBeenComputed()) {
    Diag(NewTD->getLocation(), diag::err_typedef_changes_linkage);

    SourceLocation tagLoc = TagFromDeclSpec->getInnerLocStart();
    tagLoc = getLocForEndOfToken(tagLoc);

    llvm::SmallString<40> textToInsert;
    textToInsert += ' ';
    textToInsert += NewTD->getIdentifier()->getName();
    Diag(tagLoc, diag::note_typedef_changes_linkage)
        << FixItHint::CreateInsertion(tagLoc, textToInsert);
    return;
  }

  // Otherwise, set this is the anon-decl typedef for the tag.
  TagFromDeclSpec->setTypedefNameForAnonDecl(NewTD);
}

static unsigned GetDiagnosticTypeSpecifierID(DeclSpec::TST T) {
  switch (T) {
  case DeclSpec::TST_class:
    return 0;
  case DeclSpec::TST_struct:
    return 1;
  case DeclSpec::TST_interface:
    return 2;
  case DeclSpec::TST_union:
    return 3;
  case DeclSpec::TST_enum:
    return 4;
  default:
    llvm_unreachable("unexpected type specifier");
  }
}

/// ParsedFreeStandingDeclSpec - This method is invoked when a declspec with
/// no declarator (e.g. "struct foo;") is parsed. It also accepts template
/// parameters to cope with template friend declarations.
Decl *Sema::ParsedFreeStandingDeclSpec(Scope *S, AccessSpecifier AS,
                                       DeclSpec &DS,
                                       MultiTemplateParamsArg TemplateParams,
                                       bool IsExplicitInstantiation) {
  Decl *TagD = nullptr;
  TagDecl *Tag = nullptr;
  if (DS.getTypeSpecType() == DeclSpec::TST_class ||
      DS.getTypeSpecType() == DeclSpec::TST_struct ||
      DS.getTypeSpecType() == DeclSpec::TST_interface ||
      DS.getTypeSpecType() == DeclSpec::TST_union ||
      DS.getTypeSpecType() == DeclSpec::TST_enum) {
    TagD = DS.getRepAsDecl();

    if (!TagD) // We probably had an error
      return nullptr;

    // Note that the above type specs guarantee that the
    // type rep is a Decl, whereas in many of the others
    // it's a Type.
    if (isa<TagDecl>(TagD))
      Tag = cast<TagDecl>(TagD);
    else if (ClassTemplateDecl *CTD = dyn_cast<ClassTemplateDecl>(TagD))
      Tag = CTD->getTemplatedDecl();
  }

  if (Tag) {
    handleTagNumbering(Tag, S);
    Tag->setFreeStanding();
    if (Tag->isInvalidDecl())
      return Tag;
  }

  if (unsigned TypeQuals = DS.getTypeQualifiers()) {
    // Enforce C99 6.7.3p2: "Types other than pointer types derived from object
    // or incomplete types shall not be restrict-qualified."
    if (TypeQuals & DeclSpec::TQ_restrict)
      Diag(DS.getRestrictSpecLoc(),
           diag::err_typecheck_invalid_restrict_not_pointer_noarg)
           << DS.getSourceRange();
  }

  if (DS.isConstexprSpecified()) {
    // C++0x [dcl.constexpr]p1: constexpr can only be applied to declarations
    // and definitions of functions and variables.
    if (Tag)
      Diag(DS.getConstexprSpecLoc(), diag::err_constexpr_tag)
          << GetDiagnosticTypeSpecifierID(DS.getTypeSpecType());
    else
      Diag(DS.getConstexprSpecLoc(), diag::err_constexpr_no_declarators);
    // Don't emit warnings after this error.
    return TagD;
  }

  DiagnoseFunctionSpecifiers(DS);

  if (DS.isFriendSpecified()) {
    // If we're dealing with a decl but not a TagDecl, assume that
    // whatever routines created it handled the friendship aspect.
    if (TagD && !Tag)
      return nullptr;
    return ActOnFriendTypeDecl(S, DS, TemplateParams);
  }

  const CXXScopeSpec &SS = DS.getTypeSpecScope();
  bool IsExplicitSpecialization =
    !TemplateParams.empty() && TemplateParams.back()->size() == 0;
  if (Tag && SS.isNotEmpty() && !Tag->isCompleteDefinition() &&
      !IsExplicitInstantiation && !IsExplicitSpecialization) {
    // Per C++ [dcl.type.elab]p1, a class declaration cannot have a
    // nested-name-specifier unless it is an explicit instantiation
    // or an explicit specialization.
    // Per C++ [dcl.enum]p1, an opaque-enum-declaration can't either.
    Diag(SS.getBeginLoc(), diag::err_standalone_class_nested_name_specifier)
        << GetDiagnosticTypeSpecifierID(DS.getTypeSpecType()) << SS.getRange();
    return nullptr;
  }

  // Track whether this decl-specifier declares anything.
  bool DeclaresAnything = true;

  // Handle anonymous struct definitions.
  if (RecordDecl *Record = dyn_cast_or_null<RecordDecl>(Tag)) {
    if (!Record->getDeclName() && Record->isCompleteDefinition() &&
        DS.getStorageClassSpec() != DeclSpec::SCS_typedef) {
      if (getLangOpts().CPlusPlus ||
          Record->getDeclContext()->isRecord())
        return BuildAnonymousStructOrUnion(S, DS, AS, Record,
                                           Context.getPrintingPolicy());

      DeclaresAnything = false;
    }
  }

  // C11 6.7.2.1p2:
  //   A struct-declaration that does not declare an anonymous structure or
  //   anonymous union shall contain a struct-declarator-list.
  //
  // This rule also existed in C89 and C99; the grammar for struct-declaration
  // did not permit a struct-declaration without a struct-declarator-list.
  if (!getLangOpts().CPlusPlus && CurContext->isRecord() &&
      DS.getStorageClassSpec() == DeclSpec::SCS_unspecified) {
    // Check for Microsoft C extension: anonymous struct/union member.
    // Handle 2 kinds of anonymous struct/union:
    //   struct STRUCT;
    //   union UNION;
    // and
    //   STRUCT_TYPE;  <- where STRUCT_TYPE is a typedef struct.
    //   UNION_TYPE;   <- where UNION_TYPE is a typedef union.
    if ((Tag && Tag->getDeclName()) ||
        DS.getTypeSpecType() == DeclSpec::TST_typename) {
      RecordDecl *Record = nullptr;
      if (Tag)
        Record = dyn_cast<RecordDecl>(Tag);
      else if (const RecordType *RT =
                   DS.getRepAsType().get()->getAsStructureType())
        Record = RT->getDecl();
      else if (const RecordType *UT = DS.getRepAsType().get()->getAsUnionType())
        Record = UT->getDecl();

      if (Record && getLangOpts().MicrosoftExt) {
        Diag(DS.getLocStart(), diag::ext_ms_anonymous_record)
          << Record->isUnion() << DS.getSourceRange();
        return BuildMicrosoftCAnonymousStruct(S, DS, Record);
      }

      DeclaresAnything = false;
    }
  }

  // Skip all the checks below if we have a type error.
  if (DS.getTypeSpecType() == DeclSpec::TST_error ||
      (TagD && TagD->isInvalidDecl()))
    return TagD;

  if (getLangOpts().CPlusPlus &&
      DS.getStorageClassSpec() != DeclSpec::SCS_typedef)
    if (EnumDecl *Enum = dyn_cast_or_null<EnumDecl>(Tag))
      if (Enum->enumerator_begin() == Enum->enumerator_end() &&
          !Enum->getIdentifier() && !Enum->isInvalidDecl())
        DeclaresAnything = false;

  if (!DS.isMissingDeclaratorOk()) {
    // Customize diagnostic for a typedef missing a name.
    if (DS.getStorageClassSpec() == DeclSpec::SCS_typedef)
      Diag(DS.getLocStart(), diag::ext_typedef_without_a_name)
        << DS.getSourceRange();
    else
      DeclaresAnything = false;
  }

  if (DS.isModulePrivateSpecified() &&
      Tag && Tag->getDeclContext()->isFunctionOrMethod())
    Diag(DS.getModulePrivateSpecLoc(), diag::err_module_private_local_class)
      << Tag->getTagKind()
      << FixItHint::CreateRemoval(DS.getModulePrivateSpecLoc());

  ActOnDocumentableDecl(TagD);

  // C 6.7/2:
  //   A declaration [...] shall declare at least a declarator [...], a tag,
  //   or the members of an enumeration.
  // C++ [dcl.dcl]p3:
  //   [If there are no declarators], and except for the declaration of an
  //   unnamed bit-field, the decl-specifier-seq shall introduce one or more
  //   names into the program, or shall redeclare a name introduced by a
  //   previous declaration.
  if (!DeclaresAnything) {
    // In C, we allow this as a (popular) extension / bug. Don't bother
    // producing further diagnostics for redundant qualifiers after this.
    Diag(DS.getLocStart(), diag::ext_no_declarators) << DS.getSourceRange();
    return TagD;
  }

  // C++ [dcl.stc]p1:
  //   If a storage-class-specifier appears in a decl-specifier-seq, [...] the
  //   init-declarator-list of the declaration shall not be empty.
  // C++ [dcl.fct.spec]p1:
  //   If a cv-qualifier appears in a decl-specifier-seq, the
  //   init-declarator-list of the declaration shall not be empty.
  //
  // Spurious qualifiers here appear to be valid in C.
  unsigned DiagID = diag::warn_standalone_specifier;
  if (getLangOpts().CPlusPlus)
    DiagID = diag::ext_standalone_specifier;

  // Note that a linkage-specification sets a storage class, but
  // 'extern "C" struct foo;' is actually valid and not theoretically
  // useless.
  if (DeclSpec::SCS SCS = DS.getStorageClassSpec()) {
    if (SCS == DeclSpec::SCS_mutable)
      // Since mutable is not a viable storage class specifier in C, there is
      // no reason to treat it as an extension. Instead, diagnose as an error.
      Diag(DS.getStorageClassSpecLoc(), diag::err_mutable_nonmember);
    else if (!DS.isExternInLinkageSpec() && SCS != DeclSpec::SCS_typedef)
      Diag(DS.getStorageClassSpecLoc(), DiagID)
        << DeclSpec::getSpecifierName(SCS);
  }

  if (DeclSpec::TSCS TSCS = DS.getThreadStorageClassSpec())
    Diag(DS.getThreadStorageClassSpecLoc(), DiagID)
      << DeclSpec::getSpecifierName(TSCS);
  if (DS.getTypeQualifiers()) {
    if (DS.getTypeQualifiers() & DeclSpec::TQ_const)
      Diag(DS.getConstSpecLoc(), DiagID) << "const";
    if (DS.getTypeQualifiers() & DeclSpec::TQ_volatile)
      Diag(DS.getConstSpecLoc(), DiagID) << "volatile";
    // Restrict is covered above.
    if (DS.getTypeQualifiers() & DeclSpec::TQ_atomic)
      Diag(DS.getAtomicSpecLoc(), DiagID) << "_Atomic";
  }

  // Warn about ignored type attributes, for example:
  // __attribute__((aligned)) struct A;
  // Attributes should be placed after tag to apply to type declaration.
  if (!DS.getAttributes().empty()) {
    DeclSpec::TST TypeSpecType = DS.getTypeSpecType();
    if (TypeSpecType == DeclSpec::TST_class ||
        TypeSpecType == DeclSpec::TST_struct ||
        TypeSpecType == DeclSpec::TST_interface ||
        TypeSpecType == DeclSpec::TST_union ||
        TypeSpecType == DeclSpec::TST_enum) {
      for (AttributeList* attrs = DS.getAttributes().getList(); attrs;
           attrs = attrs->getNext())
        Diag(attrs->getLoc(), diag::warn_declspec_attribute_ignored)
            << attrs->getName() << GetDiagnosticTypeSpecifierID(TypeSpecType);
    }
  }

  return TagD;
}

/// We are trying to inject an anonymous member into the given scope;
/// check if there's an existing declaration that can't be overloaded.
///
/// \return true if this is a forbidden redeclaration
static bool CheckAnonMemberRedeclaration(Sema &SemaRef,
                                         Scope *S,
                                         DeclContext *Owner,
                                         DeclarationName Name,
                                         SourceLocation NameLoc,
                                         unsigned diagnostic) {
  LookupResult R(SemaRef, Name, NameLoc, Sema::LookupMemberName,
                 Sema::ForRedeclaration);
  if (!SemaRef.LookupName(R, S)) return false;

  if (R.getAsSingle<TagDecl>())
    return false;

  // Pick a representative declaration.
  NamedDecl *PrevDecl = R.getRepresentativeDecl()->getUnderlyingDecl();
  assert(PrevDecl && "Expected a non-null Decl");

  if (!SemaRef.isDeclInScope(PrevDecl, Owner, S))
    return false;

  SemaRef.Diag(NameLoc, diagnostic) << Name;
  SemaRef.Diag(PrevDecl->getLocation(), diag::note_previous_declaration);

  return true;
}

/// InjectAnonymousStructOrUnionMembers - Inject the members of the
/// anonymous struct or union AnonRecord into the owning context Owner
/// and scope S. This routine will be invoked just after we realize
/// that an unnamed union or struct is actually an anonymous union or
/// struct, e.g.,
///
/// @code
/// union {
///   int i;
///   float f;
/// }; // InjectAnonymousStructOrUnionMembers called here to inject i and
///    // f into the surrounding scope.x
/// @endcode
///
/// This routine is recursive, injecting the names of nested anonymous
/// structs/unions into the owning context and scope as well.
static bool InjectAnonymousStructOrUnionMembers(Sema &SemaRef, Scope *S,
                                         DeclContext *Owner,
                                         RecordDecl *AnonRecord,
                                         AccessSpecifier AS,
                                         SmallVectorImpl<NamedDecl *> &Chaining,
                                         bool MSAnonStruct) {
  unsigned diagKind
    = AnonRecord->isUnion() ? diag::err_anonymous_union_member_redecl
                            : diag::err_anonymous_struct_member_redecl;

  bool Invalid = false;

  // Look every FieldDecl and IndirectFieldDecl with a name.
  for (auto *D : AnonRecord->decls()) {
    if ((isa<FieldDecl>(D) || isa<IndirectFieldDecl>(D)) &&
        cast<NamedDecl>(D)->getDeclName()) {
      ValueDecl *VD = cast<ValueDecl>(D);
      if (CheckAnonMemberRedeclaration(SemaRef, S, Owner, VD->getDeclName(),
                                       VD->getLocation(), diagKind)) {
        // C++ [class.union]p2:
        //   The names of the members of an anonymous union shall be
        //   distinct from the names of any other entity in the
        //   scope in which the anonymous union is declared.
        Invalid = true;
      } else {
        // C++ [class.union]p2:
        //   For the purpose of name lookup, after the anonymous union
        //   definition, the members of the anonymous union are
        //   considered to have been defined in the scope in which the
        //   anonymous union is declared.
        unsigned OldChainingSize = Chaining.size();
        if (IndirectFieldDecl *IF = dyn_cast<IndirectFieldDecl>(VD))
          Chaining.append(IF->chain_begin(), IF->chain_end());
        else
          Chaining.push_back(VD);

        assert(Chaining.size() >= 2);
        NamedDecl **NamedChain =
          new (SemaRef.Context)NamedDecl*[Chaining.size()];
        for (unsigned i = 0; i < Chaining.size(); i++)
          NamedChain[i] = Chaining[i];

        IndirectFieldDecl *IndirectField = IndirectFieldDecl::Create(
            SemaRef.Context, Owner, VD->getLocation(), VD->getIdentifier(),
            VD->getType(), NamedChain, Chaining.size());

        for (const auto *Attr : VD->attrs())
          IndirectField->addAttr(Attr->clone(SemaRef.Context));

        IndirectField->setAccess(AS);
        IndirectField->setImplicit();
        SemaRef.PushOnScopeChains(IndirectField, S);

        // That includes picking up the appropriate access specifier.
        if (AS != AS_none) IndirectField->setAccess(AS);

        Chaining.resize(OldChainingSize);
      }
    }
  }

  return Invalid;
}

/// StorageClassSpecToVarDeclStorageClass - Maps a DeclSpec::SCS to
/// a VarDecl::StorageClass. Any error reporting is up to the caller:
/// illegal input values are mapped to SC_None.
static StorageClass
StorageClassSpecToVarDeclStorageClass(const DeclSpec &DS) {
  DeclSpec::SCS StorageClassSpec = DS.getStorageClassSpec();
  assert(StorageClassSpec != DeclSpec::SCS_typedef &&
         "Parser allowed 'typedef' as storage class VarDecl.");
  switch (StorageClassSpec) {
  case DeclSpec::SCS_unspecified:    return SC_None;
  case DeclSpec::SCS_extern:
    if (DS.isExternInLinkageSpec())
      return SC_None;
    return SC_Extern;
  case DeclSpec::SCS_static:         return SC_Static;
  case DeclSpec::SCS_auto:           return SC_Auto;
  case DeclSpec::SCS_register:       return SC_Register;
  case DeclSpec::SCS_private_extern: return SC_PrivateExtern;
    // Illegal SCSs map to None: error reporting is up to the caller.
  case DeclSpec::SCS_mutable:        // Fall through.
  case DeclSpec::SCS_typedef:        return SC_None;
  }
  llvm_unreachable("unknown storage class specifier");
}

static SourceLocation findDefaultInitializer(const CXXRecordDecl *Record) {
  assert(Record->hasInClassInitializer());

  for (const auto *I : Record->decls()) {
    const auto *FD = dyn_cast<FieldDecl>(I);
    if (const auto *IFD = dyn_cast<IndirectFieldDecl>(I))
      FD = IFD->getAnonField();
    if (FD && FD->hasInClassInitializer())
      return FD->getLocation();
  }

  llvm_unreachable("couldn't find in-class initializer");
}

static void checkDuplicateDefaultInit(Sema &S, CXXRecordDecl *Parent,
                                      SourceLocation DefaultInitLoc) {
  if (!Parent->isUnion() || !Parent->hasInClassInitializer())
    return;

  S.Diag(DefaultInitLoc, diag::err_multiple_mem_union_initialization);
  S.Diag(findDefaultInitializer(Parent), diag::note_previous_initializer) << 0;
}

static void checkDuplicateDefaultInit(Sema &S, CXXRecordDecl *Parent,
                                      CXXRecordDecl *AnonUnion) {
  if (!Parent->isUnion() || !Parent->hasInClassInitializer())
    return;

  checkDuplicateDefaultInit(S, Parent, findDefaultInitializer(AnonUnion));
}

/// BuildAnonymousStructOrUnion - Handle the declaration of an
/// anonymous structure or union. Anonymous unions are a C++ feature
/// (C++ [class.union]) and a C11 feature; anonymous structures
/// are a C11 feature and GNU C++ extension.
Decl *Sema::BuildAnonymousStructOrUnion(Scope *S, DeclSpec &DS,
                                        AccessSpecifier AS,
                                        RecordDecl *Record,
                                        const PrintingPolicy &Policy) {
  DeclContext *Owner = Record->getDeclContext();

  // Diagnose whether this anonymous struct/union is an extension.
  if (Record->isUnion() && !getLangOpts().CPlusPlus && !getLangOpts().C11)
    Diag(Record->getLocation(), diag::ext_anonymous_union);
  else if (!Record->isUnion() && getLangOpts().CPlusPlus)
    Diag(Record->getLocation(), diag::ext_gnu_anonymous_struct);
  else if (!Record->isUnion() && !getLangOpts().C11)
    Diag(Record->getLocation(), diag::ext_c11_anonymous_struct);

  // C and C++ require different kinds of checks for anonymous
  // structs/unions.
  bool Invalid = false;
  if (getLangOpts().CPlusPlus) {
    const char *PrevSpec = nullptr;
    unsigned DiagID;
    if (Record->isUnion()) {
      // C++ [class.union]p6:
      //   Anonymous unions declared in a named namespace or in the
      //   global namespace shall be declared static.
      if (DS.getStorageClassSpec() != DeclSpec::SCS_static &&
          (isa<TranslationUnitDecl>(Owner) ||
           (isa<NamespaceDecl>(Owner) &&
            cast<NamespaceDecl>(Owner)->getDeclName()))) {
        Diag(Record->getLocation(), diag::err_anonymous_union_not_static)
          << FixItHint::CreateInsertion(Record->getLocation(), "static ");
  
        // Recover by adding 'static'.
        DS.SetStorageClassSpec(*this, DeclSpec::SCS_static, SourceLocation(),
                               PrevSpec, DiagID, Policy);
      }
      // C++ [class.union]p6:
      //   A storage class is not allowed in a declaration of an
      //   anonymous union in a class scope.
      else if (DS.getStorageClassSpec() != DeclSpec::SCS_unspecified &&
               isa<RecordDecl>(Owner)) {
        Diag(DS.getStorageClassSpecLoc(),
             diag::err_anonymous_union_with_storage_spec)
          << FixItHint::CreateRemoval(DS.getStorageClassSpecLoc());
  
        // Recover by removing the storage specifier.
        DS.SetStorageClassSpec(*this, DeclSpec::SCS_unspecified, 
                               SourceLocation(),
                               PrevSpec, DiagID, Context.getPrintingPolicy());
      }
    }

    // Ignore const/volatile/restrict qualifiers.
    if (DS.getTypeQualifiers()) {
      if (DS.getTypeQualifiers() & DeclSpec::TQ_const)
        Diag(DS.getConstSpecLoc(), diag::ext_anonymous_struct_union_qualified)
          << Record->isUnion() << "const"
          << FixItHint::CreateRemoval(DS.getConstSpecLoc());
      if (DS.getTypeQualifiers() & DeclSpec::TQ_volatile)
        Diag(DS.getVolatileSpecLoc(),
             diag::ext_anonymous_struct_union_qualified)
          << Record->isUnion() << "volatile"
          << FixItHint::CreateRemoval(DS.getVolatileSpecLoc());
      if (DS.getTypeQualifiers() & DeclSpec::TQ_restrict)
        Diag(DS.getRestrictSpecLoc(),
             diag::ext_anonymous_struct_union_qualified)
          << Record->isUnion() << "restrict"
          << FixItHint::CreateRemoval(DS.getRestrictSpecLoc());
      if (DS.getTypeQualifiers() & DeclSpec::TQ_atomic)
        Diag(DS.getAtomicSpecLoc(),
             diag::ext_anonymous_struct_union_qualified)
          << Record->isUnion() << "_Atomic"
          << FixItHint::CreateRemoval(DS.getAtomicSpecLoc());

      DS.ClearTypeQualifiers();
    }

    // C++ [class.union]p2:
    //   The member-specification of an anonymous union shall only
    //   define non-static data members. [Note: nested types and
    //   functions cannot be declared within an anonymous union. ]
    for (auto *Mem : Record->decls()) {
      if (auto *FD = dyn_cast<FieldDecl>(Mem)) {
        // C++ [class.union]p3:
        //   An anonymous union shall not have private or protected
        //   members (clause 11).
        assert(FD->getAccess() != AS_none);
        if (FD->getAccess() != AS_public) {
          Diag(FD->getLocation(), diag::err_anonymous_record_nonpublic_member)
            << (int)Record->isUnion() << (int)(FD->getAccess() == AS_protected);
          Invalid = true;
        }

        // C++ [class.union]p1
        //   An object of a class with a non-trivial constructor, a non-trivial
        //   copy constructor, a non-trivial destructor, or a non-trivial copy
        //   assignment operator cannot be a member of a union, nor can an
        //   array of such objects.
        if (CheckNontrivialField(FD))
          Invalid = true;
      } else if (Mem->isImplicit()) {
        // Any implicit members are fine.
      } else if (isa<TagDecl>(Mem) && Mem->getDeclContext() != Record) {
        // This is a type that showed up in an
        // elaborated-type-specifier inside the anonymous struct or
        // union, but which actually declares a type outside of the
        // anonymous struct or union. It's okay.
      } else if (auto *MemRecord = dyn_cast<RecordDecl>(Mem)) {
        if (!MemRecord->isAnonymousStructOrUnion() &&
            MemRecord->getDeclName()) {
          // Visual C++ allows type definition in anonymous struct or union.
          if (getLangOpts().MicrosoftExt)
            Diag(MemRecord->getLocation(), diag::ext_anonymous_record_with_type)
              << (int)Record->isUnion();
          else {
            // This is a nested type declaration.
            Diag(MemRecord->getLocation(), diag::err_anonymous_record_with_type)
              << (int)Record->isUnion();
            Invalid = true;
          }
        } else {
          // This is an anonymous type definition within another anonymous type.
          // This is a popular extension, provided by Plan9, MSVC and GCC, but
          // not part of standard C++.
          Diag(MemRecord->getLocation(),
               diag::ext_anonymous_record_with_anonymous_type)
            << (int)Record->isUnion();
        }
      } else if (isa<AccessSpecDecl>(Mem)) {
        // Any access specifier is fine.
      } else if (isa<StaticAssertDecl>(Mem)) {
        // In C++1z, static_assert declarations are also fine.
      } else {
        // We have something that isn't a non-static data
        // member. Complain about it.
        unsigned DK = diag::err_anonymous_record_bad_member;
        if (isa<TypeDecl>(Mem))
          DK = diag::err_anonymous_record_with_type;
        else if (isa<FunctionDecl>(Mem))
          DK = diag::err_anonymous_record_with_function;
        else if (isa<VarDecl>(Mem))
          DK = diag::err_anonymous_record_with_static;
        
        // Visual C++ allows type definition in anonymous struct or union.
        if (getLangOpts().MicrosoftExt &&
            DK == diag::err_anonymous_record_with_type)
          Diag(Mem->getLocation(), diag::ext_anonymous_record_with_type)
            << (int)Record->isUnion();
        else {
          Diag(Mem->getLocation(), DK)
              << (int)Record->isUnion();
          Invalid = true;
        }
      }
    }

    // C++11 [class.union]p8 (DR1460):
    //   At most one variant member of a union may have a
    //   brace-or-equal-initializer.
    if (cast<CXXRecordDecl>(Record)->hasInClassInitializer() &&
        Owner->isRecord())
      checkDuplicateDefaultInit(*this, cast<CXXRecordDecl>(Owner),
                                cast<CXXRecordDecl>(Record));
  }

  if (!Record->isUnion() && !Owner->isRecord()) {
    Diag(Record->getLocation(), diag::err_anonymous_struct_not_member)
      << (int)getLangOpts().CPlusPlus;
    Invalid = true;
  }

  // Mock up a declarator.
  Declarator Dc(DS, Declarator::MemberContext);
  TypeSourceInfo *TInfo = GetTypeForDeclarator(Dc, S);
  assert(TInfo && "couldn't build declarator info for anonymous struct/union");

  // Create a declaration for this anonymous struct/union.
  NamedDecl *Anon = nullptr;
  if (RecordDecl *OwningClass = dyn_cast<RecordDecl>(Owner)) {
    Anon = FieldDecl::Create(Context, OwningClass,
                             DS.getLocStart(),
                             Record->getLocation(),
                             /*IdentifierInfo=*/nullptr,
                             Context.getTypeDeclType(Record),
                             TInfo,
                             /*BitWidth=*/nullptr, /*Mutable=*/false,
                             /*InitStyle=*/ICIS_NoInit);
    Anon->setAccess(AS);
    if (getLangOpts().CPlusPlus)
      FieldCollector->Add(cast<FieldDecl>(Anon));
  } else {
    DeclSpec::SCS SCSpec = DS.getStorageClassSpec();
    StorageClass SC = StorageClassSpecToVarDeclStorageClass(DS);
    if (SCSpec == DeclSpec::SCS_mutable) {
      // mutable can only appear on non-static class members, so it's always
      // an error here
      Diag(Record->getLocation(), diag::err_mutable_nonmember);
      Invalid = true;
      SC = SC_None;
    }

    Anon = VarDecl::Create(Context, Owner,
                           DS.getLocStart(),
                           Record->getLocation(), /*IdentifierInfo=*/nullptr,
                           Context.getTypeDeclType(Record),
                           TInfo, SC);

    // Default-initialize the implicit variable. This initialization will be
    // trivial in almost all cases, except if a union member has an in-class
    // initializer:
    //   union { int n = 0; };
    ActOnUninitializedDecl(Anon, /*TypeMayContainAuto=*/false);
  }
  Anon->setImplicit();

  // Mark this as an anonymous struct/union type.
  Record->setAnonymousStructOrUnion(true);

  // Add the anonymous struct/union object to the current
  // context. We'll be referencing this object when we refer to one of
  // its members.
  Owner->addDecl(Anon);

  // Inject the members of the anonymous struct/union into the owning
  // context and into the identifier resolver chain for name lookup
  // purposes.
  SmallVector<NamedDecl*, 2> Chain;
  Chain.push_back(Anon);

  if (InjectAnonymousStructOrUnionMembers(*this, S, Owner, Record, AS,
                                          Chain, false))
    Invalid = true;

  if (VarDecl *NewVD = dyn_cast<VarDecl>(Anon)) {
    if (getLangOpts().CPlusPlus && NewVD->isStaticLocal()) {
      Decl *ManglingContextDecl;
      if (MangleNumberingContext *MCtx = getCurrentMangleNumberContext(
              NewVD->getDeclContext(), ManglingContextDecl)) {
        Context.setManglingNumber(
            NewVD, MCtx->getManglingNumber(
                       NewVD, getMSManglingNumber(getLangOpts(), S)));
        Context.setStaticLocalNumber(NewVD, MCtx->getStaticLocalNumber(NewVD));
      }
    }
  }

  if (Invalid)
    Anon->setInvalidDecl();

  return Anon;
}

/// BuildMicrosoftCAnonymousStruct - Handle the declaration of an
/// Microsoft C anonymous structure.
/// Ref: http://msdn.microsoft.com/en-us/library/z2cx9y4f.aspx
/// Example:
///
/// struct A { int a; };
/// struct B { struct A; int b; };
///
/// void foo() {
///   B var;
///   var.a = 3;
/// }
///
Decl *Sema::BuildMicrosoftCAnonymousStruct(Scope *S, DeclSpec &DS,
                                           RecordDecl *Record) {
  assert(Record && "expected a record!");

  // Mock up a declarator.
  Declarator Dc(DS, Declarator::TypeNameContext);
  TypeSourceInfo *TInfo = GetTypeForDeclarator(Dc, S);
  assert(TInfo && "couldn't build declarator info for anonymous struct");

  auto *ParentDecl = cast<RecordDecl>(CurContext);
  QualType RecTy = Context.getTypeDeclType(Record);

  // Create a declaration for this anonymous struct.
  NamedDecl *Anon = FieldDecl::Create(Context,
                             ParentDecl,
                             DS.getLocStart(),
                             DS.getLocStart(),
                             /*IdentifierInfo=*/nullptr,
                             RecTy,
                             TInfo,
                             /*BitWidth=*/nullptr, /*Mutable=*/false,
                             /*InitStyle=*/ICIS_NoInit);
  Anon->setImplicit();

  // Add the anonymous struct object to the current context.
  CurContext->addDecl(Anon);

  // Inject the members of the anonymous struct into the current
  // context and into the identifier resolver chain for name lookup
  // purposes.
  SmallVector<NamedDecl*, 2> Chain;
  Chain.push_back(Anon);

  RecordDecl *RecordDef = Record->getDefinition();
  if (RequireCompleteType(Anon->getLocation(), RecTy,
                          diag::err_field_incomplete) ||
      InjectAnonymousStructOrUnionMembers(*this, S, CurContext, RecordDef,
                                          AS_none, Chain, true)) {
    Anon->setInvalidDecl();
    ParentDecl->setInvalidDecl();
  }

  return Anon;
}

/// GetNameForDeclarator - Determine the full declaration name for the
/// given Declarator.
DeclarationNameInfo Sema::GetNameForDeclarator(Declarator &D) {
  return GetNameFromUnqualifiedId(D.getName());
}

/// \brief Retrieves the declaration name from a parsed unqualified-id.
DeclarationNameInfo
Sema::GetNameFromUnqualifiedId(const UnqualifiedId &Name) {
  DeclarationNameInfo NameInfo;
  NameInfo.setLoc(Name.StartLocation);

  switch (Name.getKind()) {

  case UnqualifiedId::IK_ImplicitSelfParam:
  case UnqualifiedId::IK_Identifier:
    NameInfo.setName(Name.Identifier);
    NameInfo.setLoc(Name.StartLocation);
    return NameInfo;

  case UnqualifiedId::IK_OperatorFunctionId:
    NameInfo.setName(Context.DeclarationNames.getCXXOperatorName(
                                           Name.OperatorFunctionId.Operator));
    NameInfo.setLoc(Name.StartLocation);
    NameInfo.getInfo().CXXOperatorName.BeginOpNameLoc
      = Name.OperatorFunctionId.SymbolLocations[0];
    NameInfo.getInfo().CXXOperatorName.EndOpNameLoc
      = Name.EndLocation.getRawEncoding();
    return NameInfo;

  case UnqualifiedId::IK_LiteralOperatorId:
    NameInfo.setName(Context.DeclarationNames.getCXXLiteralOperatorName(
                                                           Name.Identifier));
    NameInfo.setLoc(Name.StartLocation);
    NameInfo.setCXXLiteralOperatorNameLoc(Name.EndLocation);
    return NameInfo;

  case UnqualifiedId::IK_ConversionFunctionId: {
    TypeSourceInfo *TInfo;
    QualType Ty = GetTypeFromParser(Name.ConversionFunctionId, &TInfo);
    if (Ty.isNull())
      return DeclarationNameInfo();
    NameInfo.setName(Context.DeclarationNames.getCXXConversionFunctionName(
                                               Context.getCanonicalType(Ty)));
    NameInfo.setLoc(Name.StartLocation);
    NameInfo.setNamedTypeInfo(TInfo);
    return NameInfo;
  }

  case UnqualifiedId::IK_ConstructorName: {
    TypeSourceInfo *TInfo;
    QualType Ty = GetTypeFromParser(Name.ConstructorName, &TInfo);
    if (Ty.isNull())
      return DeclarationNameInfo();
    NameInfo.setName(Context.DeclarationNames.getCXXConstructorName(
                                              Context.getCanonicalType(Ty)));
    NameInfo.setLoc(Name.StartLocation);
    NameInfo.setNamedTypeInfo(TInfo);
    return NameInfo;
  }

  case UnqualifiedId::IK_ConstructorTemplateId: {
    // In well-formed code, we can only have a constructor
    // template-id that refers to the current context, so go there
    // to find the actual type being constructed.
    CXXRecordDecl *CurClass = dyn_cast<CXXRecordDecl>(CurContext);
    if (!CurClass || CurClass->getIdentifier() != Name.TemplateId->Name)
      return DeclarationNameInfo();

    // Determine the type of the class being constructed.
    QualType CurClassType = Context.getTypeDeclType(CurClass);

    // FIXME: Check two things: that the template-id names the same type as
    // CurClassType, and that the template-id does not occur when the name
    // was qualified.

    NameInfo.setName(Context.DeclarationNames.getCXXConstructorName(
                                    Context.getCanonicalType(CurClassType)));
    NameInfo.setLoc(Name.StartLocation);
    // FIXME: should we retrieve TypeSourceInfo?
    NameInfo.setNamedTypeInfo(nullptr);
    return NameInfo;
  }

  case UnqualifiedId::IK_DestructorName: {
    TypeSourceInfo *TInfo;
    QualType Ty = GetTypeFromParser(Name.DestructorName, &TInfo);
    if (Ty.isNull())
      return DeclarationNameInfo();
    NameInfo.setName(Context.DeclarationNames.getCXXDestructorName(
                                              Context.getCanonicalType(Ty)));
    NameInfo.setLoc(Name.StartLocation);
    NameInfo.setNamedTypeInfo(TInfo);
    return NameInfo;
  }

  case UnqualifiedId::IK_TemplateId: {
    TemplateName TName = Name.TemplateId->Template.get();
    SourceLocation TNameLoc = Name.TemplateId->TemplateNameLoc;
    return Context.getNameForTemplate(TName, TNameLoc);
  }

  } // switch (Name.getKind())

  llvm_unreachable("Unknown name kind");
}

static QualType getCoreType(QualType Ty) {
  do {
    if (Ty->isPointerType() || Ty->isReferenceType())
      Ty = Ty->getPointeeType();
    else if (Ty->isArrayType())
      Ty = Ty->castAsArrayTypeUnsafe()->getElementType();
    else
      return Ty.withoutLocalFastQualifiers();
  } while (true);
}

/// hasSimilarParameters - Determine whether the C++ functions Declaration
/// and Definition have "nearly" matching parameters. This heuristic is
/// used to improve diagnostics in the case where an out-of-line function
/// definition doesn't match any declaration within the class or namespace.
/// Also sets Params to the list of indices to the parameters that differ
/// between the declaration and the definition. If hasSimilarParameters
/// returns true and Params is empty, then all of the parameters match.
static bool hasSimilarParameters(ASTContext &Context,
                                     FunctionDecl *Declaration,
                                     FunctionDecl *Definition,
                                     SmallVectorImpl<unsigned> &Params) {
  Params.clear();
  if (Declaration->param_size() != Definition->param_size())
    return false;
  for (unsigned Idx = 0; Idx < Declaration->param_size(); ++Idx) {
    QualType DeclParamTy = Declaration->getParamDecl(Idx)->getType();
    QualType DefParamTy = Definition->getParamDecl(Idx)->getType();

    // The parameter types are identical
    if (Context.hasSameType(DefParamTy, DeclParamTy))
      continue;

    QualType DeclParamBaseTy = getCoreType(DeclParamTy);
    QualType DefParamBaseTy = getCoreType(DefParamTy);
    const IdentifierInfo *DeclTyName = DeclParamBaseTy.getBaseTypeIdentifier();
    const IdentifierInfo *DefTyName = DefParamBaseTy.getBaseTypeIdentifier();

    if (Context.hasSameUnqualifiedType(DeclParamBaseTy, DefParamBaseTy) ||
        (DeclTyName && DeclTyName == DefTyName))
      Params.push_back(Idx);
    else  // The two parameters aren't even close
      return false;
  }

  return true;
}

/// NeedsRebuildingInCurrentInstantiation - Checks whether the given
/// declarator needs to be rebuilt in the current instantiation.
/// Any bits of declarator which appear before the name are valid for
/// consideration here.  That's specifically the type in the decl spec
/// and the base type in any member-pointer chunks.
static bool RebuildDeclaratorInCurrentInstantiation(Sema &S, Declarator &D,
                                                    DeclarationName Name) {
  // The types we specifically need to rebuild are:
  //   - typenames, typeofs, and decltypes
  //   - types which will become injected class names
  // Of course, we also need to rebuild any type referencing such a
  // type.  It's safest to just say "dependent", but we call out a
  // few cases here.

  DeclSpec &DS = D.getMutableDeclSpec();
  switch (DS.getTypeSpecType()) {
  case DeclSpec::TST_typename:
  case DeclSpec::TST_typeofType:
  case DeclSpec::TST_underlyingType:
  case DeclSpec::TST_atomic: {
    // Grab the type from the parser.
    TypeSourceInfo *TSI = nullptr;
    QualType T = S.GetTypeFromParser(DS.getRepAsType(), &TSI);
    if (T.isNull() || !T->isDependentType()) break;

    // Make sure there's a type source info.  This isn't really much
    // of a waste; most dependent types should have type source info
    // attached already.
    if (!TSI)
      TSI = S.Context.getTrivialTypeSourceInfo(T, DS.getTypeSpecTypeLoc());

    // Rebuild the type in the current instantiation.
    TSI = S.RebuildTypeInCurrentInstantiation(TSI, D.getIdentifierLoc(), Name);
    if (!TSI) return true;

    // Store the new type back in the decl spec.
    ParsedType LocType = S.CreateParsedType(TSI->getType(), TSI);
    DS.UpdateTypeRep(LocType);
    break;
  }

  case DeclSpec::TST_decltype:
  case DeclSpec::TST_typeofExpr: {
    Expr *E = DS.getRepAsExpr();
    ExprResult Result = S.RebuildExprInCurrentInstantiation(E);
    if (Result.isInvalid()) return true;
    DS.UpdateExprRep(Result.get());
    break;
  }

  default:
    // Nothing to do for these decl specs.
    break;
  }

  // It doesn't matter what order we do this in.
  for (unsigned I = 0, E = D.getNumTypeObjects(); I != E; ++I) {
    DeclaratorChunk &Chunk = D.getTypeObject(I);

    // The only type information in the declarator which can come
    // before the declaration name is the base type of a member
    // pointer.
    if (Chunk.Kind != DeclaratorChunk::MemberPointer)
      continue;

    // Rebuild the scope specifier in-place.
    CXXScopeSpec &SS = Chunk.Mem.Scope();
    if (S.RebuildNestedNameSpecifierInCurrentInstantiation(SS))
      return true;
  }

  return false;
}

Decl *Sema::ActOnDeclarator(Scope *S, Declarator &D) {
  D.setFunctionDefinitionKind(FDK_Declaration);
  Decl *Dcl = HandleDeclarator(S, D, MultiTemplateParamsArg());

  if (OriginalLexicalContext && OriginalLexicalContext->isObjCContainer() &&
      Dcl && Dcl->getDeclContext()->isFileContext())
    Dcl->setTopLevelDeclInObjCContainer();

  return Dcl;
}

/// DiagnoseClassNameShadow - Implement C++ [class.mem]p13:
///   If T is the name of a class, then each of the following shall have a 
///   name different from T:
///     - every static data member of class T;
///     - every member function of class T
///     - every member of class T that is itself a type;
/// \returns true if the declaration name violates these rules.
bool Sema::DiagnoseClassNameShadow(DeclContext *DC,
                                   DeclarationNameInfo NameInfo) {
  DeclarationName Name = NameInfo.getName();

  if (CXXRecordDecl *Record = dyn_cast<CXXRecordDecl>(DC)) 
    if (Record->getIdentifier() && Record->getDeclName() == Name) {
      Diag(NameInfo.getLoc(), diag::err_member_name_of_class) << Name;
      return true;
    }

  return false;
}

/// \brief Diagnose a declaration whose declarator-id has the given 
/// nested-name-specifier.
///
/// \param SS The nested-name-specifier of the declarator-id.
///
/// \param DC The declaration context to which the nested-name-specifier 
/// resolves.
///
/// \param Name The name of the entity being declared.
///
/// \param Loc The location of the name of the entity being declared.
///
/// \returns true if we cannot safely recover from this error, false otherwise.
bool Sema::diagnoseQualifiedDeclaration(CXXScopeSpec &SS, DeclContext *DC,
                                        DeclarationName Name,
                                        SourceLocation Loc) {
  DeclContext *Cur = CurContext;
  while (isa<LinkageSpecDecl>(Cur) || isa<CapturedDecl>(Cur))
    Cur = Cur->getParent();

  // If the user provided a superfluous scope specifier that refers back to the
  // class in which the entity is already declared, diagnose and ignore it.
  //
  // class X {
  //   void X::f();
  // };
  //
  // Note, it was once ill-formed to give redundant qualification in all
  // contexts, but that rule was removed by DR482.
  if (Cur->Equals(DC)) {
    if (Cur->isRecord()) {
      Diag(Loc, LangOpts.MicrosoftExt ? diag::warn_member_extra_qualification
                                      : diag::err_member_extra_qualification)
        << Name << FixItHint::CreateRemoval(SS.getRange());
      SS.clear();
    } else {
      Diag(Loc, diag::warn_namespace_member_extra_qualification) << Name;
    }
    return false;
  }

  // Check whether the qualifying scope encloses the scope of the original
  // declaration.
  if (!Cur->Encloses(DC)) {
    if (Cur->isRecord())
      Diag(Loc, diag::err_member_qualification)
        << Name << SS.getRange();
    else if (isa<TranslationUnitDecl>(DC))
      Diag(Loc, diag::err_invalid_declarator_global_scope)
        << Name << SS.getRange();
    else if (isa<FunctionDecl>(Cur))
      Diag(Loc, diag::err_invalid_declarator_in_function) 
        << Name << SS.getRange();
    else if (isa<BlockDecl>(Cur))
      Diag(Loc, diag::err_invalid_declarator_in_block) 
        << Name << SS.getRange();
    else
      Diag(Loc, diag::err_invalid_declarator_scope)
      << Name << cast<NamedDecl>(Cur) << cast<NamedDecl>(DC) << SS.getRange();
    
    return true;
  }

  if (Cur->isRecord()) {
    // Cannot qualify members within a class.
    Diag(Loc, diag::err_member_qualification)
      << Name << SS.getRange();
    SS.clear();
    
    // C++ constructors and destructors with incorrect scopes can break
    // our AST invariants by having the wrong underlying types. If
    // that's the case, then drop this declaration entirely.
    if ((Name.getNameKind() == DeclarationName::CXXConstructorName ||
         Name.getNameKind() == DeclarationName::CXXDestructorName) &&
        !Context.hasSameType(Name.getCXXNameType(),
                             Context.getTypeDeclType(cast<CXXRecordDecl>(Cur))))
      return true;
    
    return false;
  }
  
  // C++11 [dcl.meaning]p1:
  //   [...] "The nested-name-specifier of the qualified declarator-id shall
  //   not begin with a decltype-specifer"
  NestedNameSpecifierLoc SpecLoc(SS.getScopeRep(), SS.location_data());
  while (SpecLoc.getPrefix())
    SpecLoc = SpecLoc.getPrefix();
  if (dyn_cast_or_null<DecltypeType>(
        SpecLoc.getNestedNameSpecifier()->getAsType()))
    Diag(Loc, diag::err_decltype_in_declarator)
      << SpecLoc.getTypeLoc().getSourceRange();

  return false;
}

NamedDecl *Sema::HandleDeclarator(Scope *S, Declarator &D,
                                  MultiTemplateParamsArg TemplateParamLists) {
  // TODO: consider using NameInfo for diagnostic.
  DeclarationNameInfo NameInfo = GetNameForDeclarator(D);
  DeclarationName Name = NameInfo.getName();

  // All of these full declarators require an identifier.  If it doesn't have
  // one, the ParsedFreeStandingDeclSpec action should be used.
  if (!Name) {
    if (!D.isInvalidType())  // Reject this if we think it is valid.
      Diag(D.getDeclSpec().getLocStart(),
           diag::err_declarator_need_ident)
        << D.getDeclSpec().getSourceRange() << D.getSourceRange();
    return nullptr;
  } else if (DiagnoseUnexpandedParameterPack(NameInfo, UPPC_DeclarationType))
    return nullptr;

  // The scope passed in may not be a decl scope.  Zip up the scope tree until
  // we find one that is.
  ShadowMergeState MergeState = S->isForDeclScope() ?
    ShadowMergeState_Possible : ShadowMergeState_Disallowed; // HLSL Change - check on original scope
  while ((S->getFlags() & Scope::DeclScope) == 0 ||
         (S->getFlags() & Scope::TemplateParamScope) != 0)
    S = S->getParent();

  DeclContext *DC = CurContext;
  if (D.getCXXScopeSpec().isInvalid())
    D.setInvalidType();
  else if (D.getCXXScopeSpec().isSet()) {
    if (DiagnoseUnexpandedParameterPack(D.getCXXScopeSpec(), 
                                        UPPC_DeclarationQualifier))
      return nullptr;

    bool EnteringContext = !D.getDeclSpec().isFriendSpecified();
    DC = computeDeclContext(D.getCXXScopeSpec(), EnteringContext);
    if (!DC || isa<EnumDecl>(DC)) {
      // If we could not compute the declaration context, it's because the
      // declaration context is dependent but does not refer to a class,
      // class template, or class template partial specialization. Complain
      // and return early, to avoid the coming semantic disaster.
      Diag(D.getIdentifierLoc(),
           diag::err_template_qualified_declarator_no_match)
        << D.getCXXScopeSpec().getScopeRep()
        << D.getCXXScopeSpec().getRange();
      return nullptr;
    }
    bool IsDependentContext = DC->isDependentContext();

    if (!IsDependentContext && 
        RequireCompleteDeclContext(D.getCXXScopeSpec(), DC))
      return nullptr;

    // If a class is incomplete, do not parse entities inside it.
    if (isa<CXXRecordDecl>(DC) && !cast<CXXRecordDecl>(DC)->hasDefinition()) {
      Diag(D.getIdentifierLoc(),
           diag::err_member_def_undefined_record)
        << Name << DC << D.getCXXScopeSpec().getRange();
      return nullptr;
    }
    if (!D.getDeclSpec().isFriendSpecified()) {
      if (diagnoseQualifiedDeclaration(D.getCXXScopeSpec(), DC,
                                      Name, D.getIdentifierLoc())) {
        if (DC->isRecord())
          return nullptr;

        D.setInvalidType();
      }
    }

    // Check whether we need to rebuild the type of the given
    // declaration in the current instantiation.
    if (EnteringContext && IsDependentContext &&
        TemplateParamLists.size() != 0) {
      ContextRAII SavedContext(*this, DC);
      if (RebuildDeclaratorInCurrentInstantiation(*this, D, Name))
        D.setInvalidType();
    }
  }

  TypeSourceInfo *TInfo = GetTypeForDeclarator(D, S);
  QualType R = TInfo->getType();

  if (!R->isFunctionType() && DiagnoseClassNameShadow(DC, NameInfo))
    // If this is a typedef, we'll end up spewing multiple diagnostics.
    // Just return early; it's safer. If this is a function, let the
    // "constructor cannot have a return type" diagnostic handle it.
    if (D.getDeclSpec().getStorageClassSpec() == DeclSpec::SCS_typedef)
      return nullptr;

  if (DiagnoseUnexpandedParameterPack(D.getIdentifierLoc(), TInfo,
                                      UPPC_DeclarationType))
    D.setInvalidType();

  LookupResult Previous(*this, NameInfo, LookupOrdinaryName,
                        ForRedeclaration);

  // See if this is a redefinition of a variable in the same scope.
  if (!D.getCXXScopeSpec().isSet()) {
    bool IsLinkageLookup = false;
    bool CreateBuiltins = false;

    // If the declaration we're planning to build will be a function
    // or object with linkage, then look for another declaration with
    // linkage (C99 6.2.2p4-5 and C++ [basic.link]p6).
    //
    // If the declaration we're planning to build will be declared with
    // external linkage in the translation unit, create any builtin with
    // the same name.
    if (D.getDeclSpec().getStorageClassSpec() == DeclSpec::SCS_typedef)
      /* Do nothing*/;
    else if (CurContext->isFunctionOrMethod() &&
             (D.getDeclSpec().getStorageClassSpec() == DeclSpec::SCS_extern ||
              R->isFunctionType())) {
      IsLinkageLookup = true;
      CreateBuiltins =
          CurContext->getEnclosingNamespaceContext()->isTranslationUnit();
    } else if (CurContext->getRedeclContext()->isTranslationUnit() &&
               D.getDeclSpec().getStorageClassSpec() != DeclSpec::SCS_static)
      CreateBuiltins = true;

    if (IsLinkageLookup)
      Previous.clear(LookupRedeclarationWithLinkage);

    LookupName(Previous, S, CreateBuiltins);
  } else { // Something like "int foo::x;"
    LookupQualifiedName(Previous, DC);

    // C++ [dcl.meaning]p1:
    //   When the declarator-id is qualified, the declaration shall refer to a 
    //  previously declared member of the class or namespace to which the 
    //  qualifier refers (or, in the case of a namespace, of an element of the
    //  inline namespace set of that namespace (7.3.1)) or to a specialization
    //  thereof; [...] 
    //
    // Note that we already checked the context above, and that we do not have
    // enough information to make sure that Previous contains the declaration
    // we want to match. For example, given:
    //
    //   class X {
    //     void f();
    //     void f(float);
    //   };
    //
    //   void X::f(int) { } // ill-formed
    //
    // In this case, Previous will point to the overload set
    // containing the two f's declared in X, but neither of them
    // matches.
    
    // C++ [dcl.meaning]p1:
    //   [...] the member shall not merely have been introduced by a 
    //   using-declaration in the scope of the class or namespace nominated by 
    //   the nested-name-specifier of the declarator-id.
    RemoveUsingDecls(Previous);
  }

  if (Previous.isSingleResult() &&
      Previous.getFoundDecl()->isTemplateParameter()) {
    // Maybe we will complain about the shadowed template parameter.
    if (!D.isInvalidType())
      DiagnoseTemplateParameterShadow(D.getIdentifierLoc(),
                                      Previous.getFoundDecl());

    // Just pretend that we didn't see the previous declaration.
    Previous.clear();
  }

  // In C++, the previous declaration we find might be a tag type
  // (class or enum). In this case, the new declaration will hide the
  // tag type. Note that this does does not apply if we're declaring a
  // typedef (C++ [dcl.typedef]p4).
  if (Previous.isSingleTagDecl() &&
      D.getDeclSpec().getStorageClassSpec() != DeclSpec::SCS_typedef)
    Previous.clear();

  // Check that there are no default arguments other than in the parameters
  // of a function declaration (C++ only).
  if (getLangOpts().CPlusPlus)
    CheckExtraCXXDefaultArguments(D);

  // HLSL Change Starts
  if (getLangOpts().HLSL) {
    const bool IsParameterFalse = false;
    if (!DiagnoseHLSLDecl(D, DC, /*BitWidth*/ nullptr, TInfo, IsParameterFalse)) {
      assert(D.isInvalidType() && "otherwise DiagnoseHLSLDecl failed but "
                                  "didn't invalidate declaration");
      return 0;
    }
  }
  // HLSL Change Ends

  NamedDecl *New;

  bool AddToScope = true;
  if (D.getDeclSpec().getStorageClassSpec() == DeclSpec::SCS_typedef) {
    if (TemplateParamLists.size()) {
      Diag(D.getIdentifierLoc(), diag::err_template_typedef);
      return nullptr;
    }

    New = ActOnTypedefDeclarator(S, D, DC, TInfo, Previous);
  } else if (R->isFunctionType()) {
    New = ActOnFunctionDeclarator(S, D, DC, TInfo, Previous,
                                  TemplateParamLists,
                                  AddToScope);
  } else {
    New = ActOnVariableDeclarator(S, D, DC, TInfo, Previous, TemplateParamLists,
                                  AddToScope, MergeState); // HLSL Change - add merge state
  }

  if (!New)
    return nullptr;

  // If this has an identifier and is not an invalid redeclaration or 
  // function template specialization, add it to the scope stack.
  if (New->getDeclName() && AddToScope &&
       !(D.isRedeclaration() && New->isInvalidDecl())) {
    // Only make a locally-scoped extern declaration visible if it is the first
    // declaration of this entity. Qualified lookup for such an entity should
    // only find this declaration if there is no visible declaration of it.
    bool AddToContext = !D.isRedeclaration() || !New->isLocalExternDecl();
    PushOnScopeChains(New, S, AddToContext);
    if (!AddToContext)
      CurContext->addHiddenDecl(New);
  }

  return New;
}

/// Helper method to turn variable array types into constant array
/// types in certain situations which would otherwise be errors (for
/// GCC compatibility).
static QualType TryToFixInvalidVariablyModifiedType(QualType T,
                                                    ASTContext &Context,
                                                    bool &SizeIsNegative,
                                                    llvm::APSInt &Oversized) {
  // This method tries to turn a variable array into a constant
  // array even when the size isn't an ICE.  This is necessary
  // for compatibility with code that depends on gcc's buggy
  // constant expression folding, like struct {char x[(int)(char*)2];}
  SizeIsNegative = false;
  Oversized = 0;
  
  if (T->isDependentType())
    return QualType();
  
  QualifierCollector Qs;
  const Type *Ty = Qs.strip(T);

  if (const PointerType* PTy = dyn_cast<PointerType>(Ty)) {
    QualType Pointee = PTy->getPointeeType();
    QualType FixedType =
        TryToFixInvalidVariablyModifiedType(Pointee, Context, SizeIsNegative,
                                            Oversized);
    if (FixedType.isNull()) return FixedType;
    FixedType = Context.getPointerType(FixedType);
    return Qs.apply(Context, FixedType);
  }
  if (const ParenType* PTy = dyn_cast<ParenType>(Ty)) {
    QualType Inner = PTy->getInnerType();
    QualType FixedType =
        TryToFixInvalidVariablyModifiedType(Inner, Context, SizeIsNegative,
                                            Oversized);
    if (FixedType.isNull()) return FixedType;
    FixedType = Context.getParenType(FixedType);
    return Qs.apply(Context, FixedType);
  }

  const VariableArrayType* VLATy = dyn_cast<VariableArrayType>(T);
  if (!VLATy)
    return QualType();
  // FIXME: We should probably handle this case
  if (VLATy->getElementType()->isVariablyModifiedType())
    return QualType();

  llvm::APSInt Res;
  if (!VLATy->getSizeExpr() ||
      !VLATy->getSizeExpr()->EvaluateAsInt(Res, Context))
    return QualType();

  // Check whether the array size is negative.
  if (Res.isSigned() && Res.isNegative()) {
    SizeIsNegative = true;
    return QualType();
  }

  // Check whether the array is too large to be addressed.
  unsigned ActiveSizeBits
    = ConstantArrayType::getNumAddressingBits(Context, VLATy->getElementType(),
                                              Res);
  if (ActiveSizeBits > ConstantArrayType::getMaxSizeBits(Context)) {
    Oversized = Res;
    return QualType();
  }
  
  return Context.getConstantArrayType(VLATy->getElementType(),
                                      Res, ArrayType::Normal, 0);
}

static void
FixInvalidVariablyModifiedTypeLoc(TypeLoc SrcTL, TypeLoc DstTL) {
  SrcTL = SrcTL.getUnqualifiedLoc();
  DstTL = DstTL.getUnqualifiedLoc();
  if (PointerTypeLoc SrcPTL = SrcTL.getAs<PointerTypeLoc>()) {
    PointerTypeLoc DstPTL = DstTL.castAs<PointerTypeLoc>();
    FixInvalidVariablyModifiedTypeLoc(SrcPTL.getPointeeLoc(),
                                      DstPTL.getPointeeLoc());
    DstPTL.setStarLoc(SrcPTL.getStarLoc());
    return;
  }
  if (ParenTypeLoc SrcPTL = SrcTL.getAs<ParenTypeLoc>()) {
    ParenTypeLoc DstPTL = DstTL.castAs<ParenTypeLoc>();
    FixInvalidVariablyModifiedTypeLoc(SrcPTL.getInnerLoc(),
                                      DstPTL.getInnerLoc());
    DstPTL.setLParenLoc(SrcPTL.getLParenLoc());
    DstPTL.setRParenLoc(SrcPTL.getRParenLoc());
    return;
  }
  ArrayTypeLoc SrcATL = SrcTL.castAs<ArrayTypeLoc>();
  ArrayTypeLoc DstATL = DstTL.castAs<ArrayTypeLoc>();
  TypeLoc SrcElemTL = SrcATL.getElementLoc();
  TypeLoc DstElemTL = DstATL.getElementLoc();
  DstElemTL.initializeFullCopy(SrcElemTL);
  DstATL.setLBracketLoc(SrcATL.getLBracketLoc());
  DstATL.setSizeExpr(SrcATL.getSizeExpr());
  DstATL.setRBracketLoc(SrcATL.getRBracketLoc());
}

/// Helper method to turn variable array types into constant array
/// types in certain situations which would otherwise be errors (for
/// GCC compatibility).
static TypeSourceInfo*
TryToFixInvalidVariablyModifiedTypeSourceInfo(TypeSourceInfo *TInfo,
                                              ASTContext &Context,
                                              bool &SizeIsNegative,
                                              llvm::APSInt &Oversized) {
  QualType FixedTy
    = TryToFixInvalidVariablyModifiedType(TInfo->getType(), Context,
                                          SizeIsNegative, Oversized);
  if (FixedTy.isNull())
    return nullptr;
  TypeSourceInfo *FixedTInfo = Context.getTrivialTypeSourceInfo(FixedTy);
  FixInvalidVariablyModifiedTypeLoc(TInfo->getTypeLoc(),
                                    FixedTInfo->getTypeLoc());
  return FixedTInfo;
}

/// \brief Register the given locally-scoped extern "C" declaration so
/// that it can be found later for redeclarations. We include any extern "C"
/// declaration that is not visible in the translation unit here, not just
/// function-scope declarations.
void
Sema::RegisterLocallyScopedExternCDecl(NamedDecl *ND, Scope *S) {
  if (!getLangOpts().CPlusPlus &&
      ND->getLexicalDeclContext()->getRedeclContext()->isTranslationUnit())
    // Don't need to track declarations in the TU in C.
    return;

  // Note that we have a locally-scoped external with this name.
  Context.getExternCContextDecl()->makeDeclVisibleInContext(ND);
}

NamedDecl *Sema::findLocallyScopedExternCDecl(DeclarationName Name) {
  // FIXME: We can have multiple results via __attribute__((overloadable)).
  auto Result = Context.getExternCContextDecl()->lookup(Name);
  return Result.empty() ? nullptr : *Result.begin();
}

/// \brief Diagnose function specifiers on a declaration of an identifier that
/// does not identify a function.
void Sema::DiagnoseFunctionSpecifiers(const DeclSpec &DS) {
  // FIXME: We should probably indicate the identifier in question to avoid
  // confusion for constructs like "inline int a(), b;"
  if (DS.isInlineSpecified())
    Diag(DS.getInlineSpecLoc(),
         diag::err_inline_non_function);

  if (DS.isVirtualSpecified())
    Diag(DS.getVirtualSpecLoc(),
         diag::err_virtual_non_function);

  if (DS.isExplicitSpecified())
    Diag(DS.getExplicitSpecLoc(),
         diag::err_explicit_non_function);

  if (DS.isNoreturnSpecified())
    Diag(DS.getNoreturnSpecLoc(),
         diag::err_noreturn_non_function);
}

NamedDecl*
Sema::ActOnTypedefDeclarator(Scope* S, Declarator& D, DeclContext* DC,
                             TypeSourceInfo *TInfo, LookupResult &Previous) {
  // Typedef declarators cannot be qualified (C++ [dcl.meaning]p1).
  if (D.getCXXScopeSpec().isSet()) {
    Diag(D.getIdentifierLoc(), diag::err_qualified_typedef_declarator)
      << D.getCXXScopeSpec().getRange();
    D.setInvalidType();
    // Pretend we didn't see the scope specifier.
    DC = CurContext;
    Previous.clear();
  }

  DiagnoseFunctionSpecifiers(D.getDeclSpec());

  if (D.getDeclSpec().isConstexprSpecified())
    Diag(D.getDeclSpec().getConstexprSpecLoc(), diag::err_invalid_constexpr)
      << 1;

  if (D.getName().Kind != UnqualifiedId::IK_Identifier) {
    Diag(D.getName().StartLocation, diag::err_typedef_not_identifier)
      << D.getName().getSourceRange();
    return nullptr;
  }

  TypedefDecl *NewTD = ParseTypedefDecl(S, D, TInfo->getType(), TInfo);
  if (!NewTD) return nullptr;

  // Handle attributes prior to checking for duplicates in MergeVarDecl
  ProcessDeclAttributes(S, NewTD, D);

  CheckTypedefForVariablyModifiedType(S, NewTD);

  bool Redeclaration = D.isRedeclaration();
  NamedDecl *ND = ActOnTypedefNameDecl(S, DC, NewTD, Previous, Redeclaration);
  D.setRedeclaration(Redeclaration);

  TransferUnusualAttributes(D, ND); // HLSL Change
  return ND;
}

void
Sema::CheckTypedefForVariablyModifiedType(Scope *S, TypedefNameDecl *NewTD) {
  // C99 6.7.7p2: If a typedef name specifies a variably modified type
  // then it shall have block scope.
  // Note that variably modified types must be fixed before merging the decl so
  // that redeclarations will match.
  TypeSourceInfo *TInfo = NewTD->getTypeSourceInfo();
  QualType T = TInfo->getType();
  if (T->isVariablyModifiedType()) {
    getCurFunction()->setHasBranchProtectedScope();

    if (S->getFnParent() == nullptr) {
      bool SizeIsNegative;
      llvm::APSInt Oversized;
      TypeSourceInfo *FixedTInfo =
        TryToFixInvalidVariablyModifiedTypeSourceInfo(TInfo, Context,
                                                      SizeIsNegative,
                                                      Oversized);
      if (FixedTInfo) {
        Diag(NewTD->getLocation(), diag::warn_illegal_constant_array_size);
        NewTD->setTypeSourceInfo(FixedTInfo);
      } else {
        if (SizeIsNegative)
          Diag(NewTD->getLocation(), diag::err_typecheck_negative_array_size);
        else if (T->isVariableArrayType())
          Diag(NewTD->getLocation(), diag::err_vla_decl_in_file_scope);
        else if (Oversized.getBoolValue())
          Diag(NewTD->getLocation(), diag::err_array_too_large) 
            << Oversized.toString(10);
        else
          Diag(NewTD->getLocation(), diag::err_vm_decl_in_file_scope);
        NewTD->setInvalidDecl();
      }
    }
  }
}


/// ActOnTypedefNameDecl - Perform semantic checking for a declaration which
/// declares a typedef-name, either using the 'typedef' type specifier or via
/// a C++0x [dcl.typedef]p2 alias-declaration: 'using T = A;'.
NamedDecl*
Sema::ActOnTypedefNameDecl(Scope *S, DeclContext *DC, TypedefNameDecl *NewTD,
                           LookupResult &Previous, bool &Redeclaration) {
  // Merge the decl with the existing one if appropriate. If the decl is
  // in an outer scope, it isn't the same thing.
  FilterLookupForScope(Previous, DC, S, /*ConsiderLinkage*/false,
                       /*AllowInlineNamespace*/false);
  filterNonConflictingPreviousTypedefDecls(*this, NewTD, Previous);
  if (!Previous.empty()) {
    Redeclaration = true;
    MergeTypedefNameDecl(NewTD, Previous);
  }

  // If this is the C FILE type, notify the AST context.
  if (IdentifierInfo *II = NewTD->getIdentifier())
    if (!NewTD->isInvalidDecl() &&
        NewTD->getDeclContext()->getRedeclContext()->isTranslationUnit()) {
      if (II->isStr("FILE"))
        Context.setFILEDecl(NewTD);
      else if (II->isStr("jmp_buf"))
        Context.setjmp_bufDecl(NewTD);
      else if (II->isStr("sigjmp_buf"))
        Context.setsigjmp_bufDecl(NewTD);
      else if (II->isStr("ucontext_t"))
        Context.setucontext_tDecl(NewTD);
    }

  return NewTD;
}

/// \brief Determines whether the given declaration is an out-of-scope
/// previous declaration.
///
/// This routine should be invoked when name lookup has found a
/// previous declaration (PrevDecl) that is not in the scope where a
/// new declaration by the same name is being introduced. If the new
/// declaration occurs in a local scope, previous declarations with
/// linkage may still be considered previous declarations (C99
/// 6.2.2p4-5, C++ [basic.link]p6).
///
/// \param PrevDecl the previous declaration found by name
/// lookup
///
/// \param DC the context in which the new declaration is being
/// declared.
///
/// \returns true if PrevDecl is an out-of-scope previous declaration
/// for a new delcaration with the same name.
static bool
isOutOfScopePreviousDeclaration(NamedDecl *PrevDecl, DeclContext *DC,
                                ASTContext &Context) {
  if (!PrevDecl)
    return false;

  if (!PrevDecl->hasLinkage())
    return false;

  if (Context.getLangOpts().CPlusPlus) {
    // C++ [basic.link]p6:
    //   If there is a visible declaration of an entity with linkage
    //   having the same name and type, ignoring entities declared
    //   outside the innermost enclosing namespace scope, the block
    //   scope declaration declares that same entity and receives the
    //   linkage of the previous declaration.
    DeclContext *OuterContext = DC->getRedeclContext();
    if (!OuterContext->isFunctionOrMethod())
      // This rule only applies to block-scope declarations.
      return false;
    
    DeclContext *PrevOuterContext = PrevDecl->getDeclContext();
    if (PrevOuterContext->isRecord())
      // We found a member function: ignore it.
      return false;
    
    // Find the innermost enclosing namespace for the new and
    // previous declarations.
    OuterContext = OuterContext->getEnclosingNamespaceContext();
    PrevOuterContext = PrevOuterContext->getEnclosingNamespaceContext();

    // The previous declaration is in a different namespace, so it
    // isn't the same function.
    if (!OuterContext->Equals(PrevOuterContext))
      return false;
  }

  return true;
}

static void SetNestedNameSpecifier(DeclaratorDecl *DD, Declarator &D) {
  CXXScopeSpec &SS = D.getCXXScopeSpec();
  if (!SS.isSet()) return;
  DD->setQualifierInfo(SS.getWithLocInContext(DD->getASTContext()));
}

bool Sema::inferObjCARCLifetime(ValueDecl *decl) {
  QualType type = decl->getType();
  Qualifiers::ObjCLifetime lifetime = type.getObjCLifetime();
  if (lifetime == Qualifiers::OCL_Autoreleasing) {
    // Various kinds of declaration aren't allowed to be __autoreleasing.
    unsigned kind = -1U;
    if (VarDecl *var = dyn_cast<VarDecl>(decl)) {
      if (var->hasAttr<BlocksAttr>())
        kind = 0; // __block
      else if (!var->hasLocalStorage())
        kind = 1; // global
    } else if (isa<ObjCIvarDecl>(decl)) {
      kind = 3; // ivar
    } else if (isa<FieldDecl>(decl)) {
      kind = 2; // field
    }

    if (kind != -1U) {
      Diag(decl->getLocation(), diag::err_arc_autoreleasing_var)
        << kind;
    }
  } else if (lifetime == Qualifiers::OCL_None) {
    // Try to infer lifetime.
    if (!type->isObjCLifetimeType())
      return false;

    lifetime = type->getObjCARCImplicitLifetime();
    type = Context.getLifetimeQualifiedType(type, lifetime);
    decl->setType(type);
  }
  
  if (VarDecl *var = dyn_cast<VarDecl>(decl)) {
    // Thread-local variables cannot have lifetime.
    if (lifetime && lifetime != Qualifiers::OCL_ExplicitNone &&
        var->getTLSKind()) {
      Diag(var->getLocation(), diag::err_arc_thread_ownership)
        << var->getType();
      return true;
    }
  }
  
  return false;
}

static void checkAttributesAfterMerging(Sema &S, NamedDecl &ND) {
  // Ensure that an auto decl is deduced otherwise the checks below might cache
  // the wrong linkage.
  assert(S.ParsingInitForAutoVars.count(&ND) == 0);

  // 'weak' only applies to declarations with external linkage.
  if (WeakAttr *Attr = ND.getAttr<WeakAttr>()) {
    if (!ND.isExternallyVisible()) {
      S.Diag(Attr->getLocation(), diag::err_attribute_weak_static);
      ND.dropAttr<WeakAttr>();
    }
  }
  if (WeakRefAttr *Attr = ND.getAttr<WeakRefAttr>()) {
    if (ND.isExternallyVisible()) {
      S.Diag(Attr->getLocation(), diag::err_attribute_weakref_not_static);
      ND.dropAttr<WeakRefAttr>();
      ND.dropAttr<AliasAttr>();
    }
  }

  if (auto *VD = dyn_cast<VarDecl>(&ND)) {
    if (VD->hasInit()) {
      if (const auto *Attr = VD->getAttr<AliasAttr>()) {
        assert(VD->isThisDeclarationADefinition() &&
               !VD->isExternallyVisible() && "Broken AliasAttr handled late!");
        S.Diag(Attr->getLocation(), diag::err_alias_is_definition) << VD;
        VD->dropAttr<AliasAttr>();
      }
    }
  }

  // 'selectany' only applies to externally visible variable declarations.
  // It does not apply to functions.
  if (SelectAnyAttr *Attr = ND.getAttr<SelectAnyAttr>()) {
    if (isa<FunctionDecl>(ND) || !ND.isExternallyVisible()) {
      S.Diag(Attr->getLocation(),
             diag::err_attribute_selectany_non_extern_data);
      ND.dropAttr<SelectAnyAttr>();
    }
  }

  // dll attributes require external linkage.
  if (const InheritableAttr *Attr = getDLLAttr(&ND)) {
    if (!ND.isExternallyVisible()) {
      S.Diag(ND.getLocation(), diag::err_attribute_dll_not_extern)
        << &ND << Attr;
      ND.setInvalidDecl();
    }
  }
}

static void checkDLLAttributeRedeclaration(Sema &S, NamedDecl *OldDecl,
                                           NamedDecl *NewDecl,
                                           bool IsSpecialization) {
  if (TemplateDecl *OldTD = dyn_cast<TemplateDecl>(OldDecl))
    OldDecl = OldTD->getTemplatedDecl();
  if (TemplateDecl *NewTD = dyn_cast<TemplateDecl>(NewDecl))
    NewDecl = NewTD->getTemplatedDecl();

  if (!OldDecl || !NewDecl)
    return;

  const DLLImportAttr *OldImportAttr = OldDecl->getAttr<DLLImportAttr>();
  const DLLExportAttr *OldExportAttr = OldDecl->getAttr<DLLExportAttr>();
  const DLLImportAttr *NewImportAttr = NewDecl->getAttr<DLLImportAttr>();
  const DLLExportAttr *NewExportAttr = NewDecl->getAttr<DLLExportAttr>();

  // dllimport and dllexport are inheritable attributes so we have to exclude
  // inherited attribute instances.
  bool HasNewAttr = (NewImportAttr && !NewImportAttr->isInherited()) ||
                    (NewExportAttr && !NewExportAttr->isInherited());

  // A redeclaration is not allowed to add a dllimport or dllexport attribute,
  // the only exception being explicit specializations.
  // Implicitly generated declarations are also excluded for now because there
  // is no other way to switch these to use dllimport or dllexport.
  bool AddsAttr = !(OldImportAttr || OldExportAttr) && HasNewAttr;

  if (AddsAttr && !IsSpecialization && !OldDecl->isImplicit()) {
    // Allow with a warning for free functions and global variables.
    bool JustWarn = false;
    if (!OldDecl->isCXXClassMember()) {
      auto *VD = dyn_cast<VarDecl>(OldDecl);
      if (VD && !VD->getDescribedVarTemplate())
        JustWarn = true;
      auto *FD = dyn_cast<FunctionDecl>(OldDecl);
      if (FD && FD->getTemplatedKind() == FunctionDecl::TK_NonTemplate)
        JustWarn = true;
    }

    // We cannot change a declaration that's been used because IR has already
    // been emitted. Dllimported functions will still work though (modulo
    // address equality) as they can use the thunk.
    if (OldDecl->isUsed())
      if (!isa<FunctionDecl>(OldDecl) || !NewImportAttr)
        JustWarn = false;

    unsigned DiagID = JustWarn ? diag::warn_attribute_dll_redeclaration
                               : diag::err_attribute_dll_redeclaration;
    S.Diag(NewDecl->getLocation(), DiagID)
        << NewDecl
        << (NewImportAttr ? (const Attr *)NewImportAttr : NewExportAttr);
    S.Diag(OldDecl->getLocation(), diag::note_previous_declaration);
    if (!JustWarn) {
      NewDecl->setInvalidDecl();
      return;
    }
  }

  // A redeclaration is not allowed to drop a dllimport attribute, the only
  // exceptions being inline function definitions, local extern declarations,
  // and qualified friend declarations.
  // NB: MSVC converts such a declaration to dllexport.
  bool IsInline = false, IsStaticDataMember = false, IsQualifiedFriend = false;
  if (const auto *VD = dyn_cast<VarDecl>(NewDecl))
    // Ignore static data because out-of-line definitions are diagnosed
    // separately.
    IsStaticDataMember = VD->isStaticDataMember();
  else if (const auto *FD = dyn_cast<FunctionDecl>(NewDecl)) {
    IsInline = FD->isInlined();
    IsQualifiedFriend = FD->getQualifier() &&
                        FD->getFriendObjectKind() == Decl::FOK_Declared;
  }

  if (OldImportAttr && !HasNewAttr && !IsInline && !IsStaticDataMember &&
      !NewDecl->isLocalExternDecl() && !IsQualifiedFriend) {
    S.Diag(NewDecl->getLocation(),
           diag::warn_redeclaration_without_attribute_prev_attribute_ignored)
      << NewDecl << OldImportAttr;
    S.Diag(OldDecl->getLocation(), diag::note_previous_declaration);
    S.Diag(OldImportAttr->getLocation(), diag::note_previous_attribute);
    OldDecl->dropAttr<DLLImportAttr>();
    NewDecl->dropAttr<DLLImportAttr>();
  } else if (IsInline && OldImportAttr &&
             !S.Context.getTargetInfo().getCXXABI().isMicrosoft()) {
    // In MinGW, seeing a function declared inline drops the dllimport attribute.
    OldDecl->dropAttr<DLLImportAttr>();
    NewDecl->dropAttr<DLLImportAttr>();
    S.Diag(NewDecl->getLocation(),
           diag::warn_dllimport_dropped_from_inline_function)
        << NewDecl << OldImportAttr;
  }
}

/// Given that we are within the definition of the given function,
/// will that definition behave like C99's 'inline', where the
/// definition is discarded except for optimization purposes?
static bool isFunctionDefinitionDiscarded(Sema &S, FunctionDecl *FD) {
  // Try to avoid calling GetGVALinkageForFunction.

  // All cases of this require the 'inline' keyword.
  if (!FD->isInlined()) return false;

  // This is only possible in C++ with the gnu_inline attribute.
  if (S.getLangOpts().CPlusPlus && !FD->hasAttr<GNUInlineAttr>())
    return false;

  // Okay, go ahead and call the relatively-more-expensive function.

#ifndef NDEBUG
  // AST quite reasonably asserts that it's working on a function
  // definition.  We don't really have a way to tell it that we're
  // currently defining the function, so just lie to it in +Asserts
  // builds.  This is an awful hack.
  FD->setLazyBody(1);
#endif

  bool isC99Inline =
      S.Context.GetGVALinkageForFunction(FD) == GVA_AvailableExternally;

#ifndef NDEBUG
  FD->setLazyBody(0);
#endif

  return isC99Inline;
}

/// Determine whether a variable is extern "C" prior to attaching
/// an initializer. We can't just call isExternC() here, because that
/// will also compute and cache whether the declaration is externally
/// visible, which might change when we attach the initializer.
///
/// This can only be used if the declaration is known to not be a
/// redeclaration of an internal linkage declaration.
///
/// For instance:
///
///   auto x = []{};
///
/// Attaching the initializer here makes this declaration not externally
/// visible, because its type has internal linkage.
///
/// FIXME: This is a hack.
template<typename T>
static bool isIncompleteDeclExternC(Sema &S, const T *D) {
  if (S.getLangOpts().CPlusPlus) {
    // In C++, the overloadable attribute negates the effects of extern "C".
    if (!D->isInExternCContext() || D->template hasAttr<OverloadableAttr>())
      return false;
  }
  return D->isExternC();
}

static bool shouldConsiderLinkage(const VarDecl *VD) {
  const DeclContext *DC = VD->getDeclContext()->getRedeclContext();
  if (DC->isFunctionOrMethod())
    return VD->hasExternalStorage();
  if (DC->isFileContext())
    return true;
  if (DC->isRecord())
    return false;
  llvm_unreachable("Unexpected context");
}

static bool shouldConsiderLinkage(const FunctionDecl *FD) {
  const DeclContext *DC = FD->getDeclContext()->getRedeclContext();
  if (DC->isFileContext() || DC->isFunctionOrMethod())
    return true;
  if (DC->isRecord())
    return false;
  llvm_unreachable("Unexpected context");
}

static bool hasParsedAttr(Scope *S, const AttributeList *AttrList,
                          AttributeList::Kind Kind) {
  for (const AttributeList *L = AttrList; L; L = L->getNext())
    if (L->getKind() == Kind)
      return true;
  return false;
}

static bool hasParsedAttr(Scope *S, const Declarator &PD,
                          AttributeList::Kind Kind) {
  // Check decl attributes on the DeclSpec.
  if (hasParsedAttr(S, PD.getDeclSpec().getAttributes().getList(), Kind))
    return true;

  // Walk the declarator structure, checking decl attributes that were in a type
  // position to the decl itself.
  for (unsigned I = 0, E = PD.getNumTypeObjects(); I != E; ++I) {
    if (hasParsedAttr(S, PD.getTypeObject(I).getAttrs(), Kind))
      return true;
  }

  // Finally, check attributes on the decl itself.
  return hasParsedAttr(S, PD.getAttributes(), Kind);
}

/// Adjust the \c DeclContext for a function or variable that might be a
/// function-local external declaration.
bool Sema::adjustContextForLocalExternDecl(DeclContext *&DC) {
  if (!DC->isFunctionOrMethod())
    return false;

  // If this is a local extern function or variable declared within a function
  // template, don't add it into the enclosing namespace scope until it is
  // instantiated; it might have a dependent type right now.
  if (DC->isDependentContext())
    return true;

  // C++11 [basic.link]p7:
  //   When a block scope declaration of an entity with linkage is not found to
  //   refer to some other declaration, then that entity is a member of the
  //   innermost enclosing namespace.
  //
  // Per C++11 [namespace.def]p6, the innermost enclosing namespace is a
  // semantically-enclosing namespace, not a lexically-enclosing one.
  while (!DC->isFileContext() && !isa<LinkageSpecDecl>(DC))
    DC = DC->getParent();
  return true;
}

/// \brief Returns true if given declaration is TU-scoped and externally
/// visible.
static bool isDeclTUScopedExternallyVisible(const Decl *D) {
  if (auto *FD = dyn_cast<FunctionDecl>(D))
    return (FD->getDeclContext()->isTranslationUnit() || FD->isExternC()) &&
           FD->hasExternalFormalLinkage();
  else if (auto *VD = dyn_cast<VarDecl>(D))
    return (VD->getDeclContext()->isTranslationUnit() || VD->isExternC()) &&
           VD->hasExternalFormalLinkage();

  llvm_unreachable("Unknown type of decl!");
}

NamedDecl *
Sema::ActOnVariableDeclarator(Scope *S, Declarator &D, DeclContext *DC,
                              TypeSourceInfo *TInfo, LookupResult &Previous,
                              MultiTemplateParamsArg TemplateParamLists,
                              bool &AddToScope, ShadowMergeState MergeState) { // HLSL Change - add merge state
  QualType R = TInfo->getType();
  DeclarationName Name = GetNameForDeclarator(D).getName();

  DeclSpec::SCS SCSpec = D.getDeclSpec().getStorageClassSpec();
  StorageClass SC = StorageClassSpecToVarDeclStorageClass(D.getDeclSpec());

  // dllimport globals without explicit storage class are treated as extern. We
  // have to change the storage class this early to get the right DeclContext.
  if (SC == SC_None && !DC->isRecord() &&
      hasParsedAttr(S, D, AttributeList::AT_DLLImport) &&
      !hasParsedAttr(S, D, AttributeList::AT_DLLExport))
    SC = SC_Extern;

  DeclContext *OriginalDC = DC;
  bool IsLocalExternDecl = SC == SC_Extern &&
                           adjustContextForLocalExternDecl(DC);

  if (getLangOpts().OpenCL) {
    // OpenCL v1.0 s6.8.a.3: Pointers to functions are not allowed.
    QualType NR = R;
    while (NR->isPointerType()) {
      if (NR->isFunctionPointerType()) {
        Diag(D.getIdentifierLoc(), diag::err_opencl_function_pointer_variable);
        D.setInvalidType();
        break;
      }
      NR = NR->getPointeeType();
    }

    if (!getOpenCLOptions().cl_khr_fp16) {
      // OpenCL v1.2 s6.1.1.1: reject declaring variables of the half and
      // half array type (unless the cl_khr_fp16 extension is enabled).
      if (Context.getBaseElementType(R)->isHalfType()) {
        Diag(D.getIdentifierLoc(), diag::err_opencl_half_declaration) << R;
        D.setInvalidType();
      }
    }
  }

  if (SCSpec == DeclSpec::SCS_mutable) {
    // mutable can only appear on non-static class members, so it's always
    // an error here
    Diag(D.getIdentifierLoc(), diag::err_mutable_nonmember);
    D.setInvalidType();
    SC = SC_None;
  }

  if (getLangOpts().CPlusPlus11 && SCSpec == DeclSpec::SCS_register &&
      !D.getAsmLabel() && !getSourceManager().isInSystemMacro(
                              D.getDeclSpec().getStorageClassSpecLoc())) {
    // In C++11, the 'register' storage class specifier is deprecated.
    // Suppress the warning in system macros, it's used in macros in some
    // popular C system headers, such as in glibc's htonl() macro.
    Diag(D.getDeclSpec().getStorageClassSpecLoc(),
         diag::warn_deprecated_register)
      << FixItHint::CreateRemoval(D.getDeclSpec().getStorageClassSpecLoc());
  }

  IdentifierInfo *II = Name.getAsIdentifierInfo();
  if (!II) {
    Diag(D.getIdentifierLoc(), diag::err_bad_variable_name)
      << Name;
    return nullptr;
  }

  DiagnoseFunctionSpecifiers(D.getDeclSpec());

  if (!DC->isRecord() && S->getFnParent() == nullptr) {
    // C99 6.9p2: The storage-class specifiers auto and register shall not
    // appear in the declaration specifiers in an external declaration.
    // Global Register+Asm is a GNU extension we support.
    if (SC == SC_Auto || (SC == SC_Register && !D.getAsmLabel())) {
      Diag(D.getIdentifierLoc(), diag::err_typecheck_sclass_fscope);
      D.setInvalidType();
    }
  }

  if (getLangOpts().OpenCL) {
    // Set up the special work-group-local storage class for variables in the
    // OpenCL __local address space.
    if (R.getAddressSpace() == LangAS::opencl_local) {
      SC = SC_OpenCLWorkGroupLocal;
    }

    // OpenCL v1.2 s6.9.b p4:
    // The sampler type cannot be used with the __local and __global address
    // space qualifiers.
    if (R->isSamplerT() && (R.getAddressSpace() == LangAS::opencl_local ||
      R.getAddressSpace() == LangAS::opencl_global)) {
      Diag(D.getIdentifierLoc(), diag::err_wrong_sampler_addressspace);
    }

    // OpenCL 1.2 spec, p6.9 r:
    // The event type cannot be used to declare a program scope variable.
    // The event type cannot be used with the __local, __constant and __global
    // address space qualifiers.
    if (R->isEventT()) {
      if (S->getParent() == nullptr) {
        Diag(D.getLocStart(), diag::err_event_t_global_var);
        D.setInvalidType();
      }

      if (R.getAddressSpace()) {
        Diag(D.getLocStart(), diag::err_event_t_addr_space_qual);
        D.setInvalidType();
      }
    }
  }

  bool IsExplicitSpecialization = false;
  bool IsVariableTemplateSpecialization = false;
  bool IsPartialSpecialization = false;
  bool IsVariableTemplate = false;
  VarDecl *NewVD = nullptr;
  VarTemplateDecl *NewTemplate = nullptr;
  TemplateParameterList *TemplateParams = nullptr;
  if (!getLangOpts().CPlusPlus) {
    NewVD = VarDecl::Create(Context, DC, D.getLocStart(),
                            D.getIdentifierLoc(), II,
                            R, TInfo, SC);
  
    if (D.isInvalidType())
      NewVD->setInvalidDecl();
  } else {
    bool Invalid = false;

    if (DC->isRecord() && !CurContext->isRecord()) {
      // This is an out-of-line definition of a static data member.
      switch (SC) {
      case SC_None:
        break;
      case SC_Static:
        if (!getLangOpts().HLSL) // HLSL Change: We had to set SC_Static for
                                 // static data member to distinguish it from
                                 // global constant variable.
                                 // So we ignore this warning.
          Diag(D.getDeclSpec().getStorageClassSpecLoc(),
             diag::err_static_out_of_line)
          << FixItHint::CreateRemoval(D.getDeclSpec().getStorageClassSpecLoc());
        break;
      case SC_Auto:
      case SC_Register:
      case SC_Extern:
        // [dcl.stc] p2: The auto or register specifiers shall be applied only
        // to names of variables declared in a block or to function parameters.
        // [dcl.stc] p6: The extern specifier cannot be used in the declaration
        // of class members

        Diag(D.getDeclSpec().getStorageClassSpecLoc(),
             diag::err_storage_class_for_static_member)
          << FixItHint::CreateRemoval(D.getDeclSpec().getStorageClassSpecLoc());
        break;
      case SC_PrivateExtern:
        llvm_unreachable("C storage class in c++!");
      case SC_OpenCLWorkGroupLocal:
        llvm_unreachable("OpenCL storage class in c++!");
      }
    }    

    if (SC == SC_Static && CurContext->isRecord()) {
      if (const CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(DC)) {
        if (RD->isLocalClass())
          Diag(D.getIdentifierLoc(),
               diag::err_static_data_member_not_allowed_in_local_class)
            << Name << RD->getDeclName();

        // C++98 [class.union]p1: If a union contains a static data member,
        // the program is ill-formed. C++11 drops this restriction.
        if (RD->isUnion())
          Diag(D.getIdentifierLoc(),
               getLangOpts().CPlusPlus11
                 ? diag::warn_cxx98_compat_static_data_member_in_union
                 : diag::ext_static_data_member_in_union) << Name;
        // We conservatively disallow static data members in anonymous structs.
        else if (!RD->getDeclName())
          Diag(D.getIdentifierLoc(),
               diag::err_static_data_member_not_allowed_in_anon_struct)
            << Name << RD->isUnion();
      }
    }

    // Match up the template parameter lists with the scope specifier, then
    // determine whether we have a template or a template specialization.
    TemplateParams = MatchTemplateParametersToScopeSpecifier(
        D.getDeclSpec().getLocStart(), D.getIdentifierLoc(),
        D.getCXXScopeSpec(),
        D.getName().getKind() == UnqualifiedId::IK_TemplateId
            ? D.getName().TemplateId
            : nullptr,
        TemplateParamLists,
        /*never a friend*/ false, IsExplicitSpecialization, Invalid);

    if (TemplateParams) {
      if (!TemplateParams->size() &&
          D.getName().getKind() != UnqualifiedId::IK_TemplateId) {
        // There is an extraneous 'template<>' for this variable. Complain
        // about it, but allow the declaration of the variable.
        Diag(TemplateParams->getTemplateLoc(),
             diag::err_template_variable_noparams)
          << II
          << SourceRange(TemplateParams->getTemplateLoc(),
                         TemplateParams->getRAngleLoc());
        TemplateParams = nullptr;
      } else {
        if (D.getName().getKind() == UnqualifiedId::IK_TemplateId) {
          // This is an explicit specialization or a partial specialization.
          // FIXME: Check that we can declare a specialization here.
          IsVariableTemplateSpecialization = true;
          IsPartialSpecialization = TemplateParams->size() > 0;
        } else { // if (TemplateParams->size() > 0)
          // This is a template declaration.
          IsVariableTemplate = true;

          // Check that we can declare a template here.
          if (CheckTemplateDeclScope(S, TemplateParams))
            return nullptr;

          // Only C++1y supports variable templates (N3651).
          Diag(D.getIdentifierLoc(),
               getLangOpts().CPlusPlus14
                   ? diag::warn_cxx11_compat_variable_template
                   : diag::ext_variable_template);
        }
      }
    } else {
      assert(
          (Invalid || D.getName().getKind() != UnqualifiedId::IK_TemplateId) &&
          "should have a 'template<>' for this decl");
    }

    if (IsVariableTemplateSpecialization) {
      SourceLocation TemplateKWLoc =
          TemplateParamLists.size() > 0
              ? TemplateParamLists[0]->getTemplateLoc()
              : SourceLocation();
      DeclResult Res = ActOnVarTemplateSpecialization(
          S, D, TInfo, TemplateKWLoc, TemplateParams, SC,
          IsPartialSpecialization);
      if (Res.isInvalid())
        return nullptr;
      NewVD = cast<VarDecl>(Res.get());
      AddToScope = false;
    } else
      NewVD = VarDecl::Create(Context, DC, D.getLocStart(),
                              D.getIdentifierLoc(), II, R, TInfo, SC);

    // If this is supposed to be a variable template, create it as such.
    if (IsVariableTemplate) {
      NewTemplate =
          VarTemplateDecl::Create(Context, DC, D.getIdentifierLoc(), Name,
                                  TemplateParams, NewVD);
      NewVD->setDescribedVarTemplate(NewTemplate);
    }

    // If this decl has an auto type in need of deduction, make a note of the
    // Decl so we can diagnose uses of it in its own initializer.
    if (D.getDeclSpec().containsPlaceholderType() && R->getContainedAutoType())
      ParsingInitForAutoVars.insert(NewVD);

    if (D.isInvalidType() || Invalid) {
      NewVD->setInvalidDecl();
      if (NewTemplate)
        NewTemplate->setInvalidDecl();
    }

    SetNestedNameSpecifier(NewVD, D);

    // If we have any template parameter lists that don't directly belong to
    // the variable (matching the scope specifier), store them.
    unsigned VDTemplateParamLists = TemplateParams ? 1 : 0;
    if (TemplateParamLists.size() > VDTemplateParamLists)
      NewVD->setTemplateParameterListsInfo(
          Context, TemplateParamLists.size() - VDTemplateParamLists,
          TemplateParamLists.data());

    if (D.getDeclSpec().isConstexprSpecified())
      NewVD->setConstexpr(true);
  }

  // Set the lexical context. If the declarator has a C++ scope specifier, the
  // lexical context will be different from the semantic context.
  NewVD->setLexicalDeclContext(CurContext);
  if (NewTemplate)
    NewTemplate->setLexicalDeclContext(CurContext);

  if (IsLocalExternDecl)
    NewVD->setLocalExternDecl();

  bool EmitTLSUnsupportedError = false;
  if (DeclSpec::TSCS TSCS = D.getDeclSpec().getThreadStorageClassSpec()) {
    // C++11 [dcl.stc]p4:
    //   When thread_local is applied to a variable of block scope the
    //   storage-class-specifier static is implied if it does not appear
    //   explicitly.
    // Core issue: 'static' is not implied if the variable is declared
    //   'extern'.
    if (NewVD->hasLocalStorage() &&
        (SCSpec != DeclSpec::SCS_unspecified ||
         TSCS != DeclSpec::TSCS_thread_local ||
         !DC->isFunctionOrMethod()))
      Diag(D.getDeclSpec().getThreadStorageClassSpecLoc(),
           diag::err_thread_non_global)
        << DeclSpec::getSpecifierName(TSCS);
    else if (!Context.getTargetInfo().isTLSSupported()) {
      if (getLangOpts().CUDA) {
        // Postpone error emission until we've collected attributes required to
        // figure out whether it's a host or device variable and whether the
        // error should be ignored.
        EmitTLSUnsupportedError = true;
        // We still need to mark the variable as TLS so it shows up in AST with
        // proper storage class for other tools to use even if we're not going
        // to emit any code for it.
        NewVD->setTSCSpec(TSCS);
      } else
        Diag(D.getDeclSpec().getThreadStorageClassSpecLoc(),
             diag::err_thread_unsupported);
    } else
      NewVD->setTSCSpec(TSCS);
  }

  // C99 6.7.4p3
  //   An inline definition of a function with external linkage shall
  //   not contain a definition of a modifiable object with static or
  //   thread storage duration...
  // We only apply this when the function is required to be defined
  // elsewhere, i.e. when the function is not 'extern inline'.  Note
  // that a local variable with thread storage duration still has to
  // be marked 'static'.  Also note that it's possible to get these
  // semantics in C++ using __attribute__((gnu_inline)).
  if (SC == SC_Static && S->getFnParent() != nullptr &&
      !NewVD->getType().isConstQualified()) {
    FunctionDecl *CurFD = getCurFunctionDecl();
    if (CurFD && isFunctionDefinitionDiscarded(*this, CurFD)) {
      Diag(D.getDeclSpec().getStorageClassSpecLoc(),
           diag::warn_static_local_in_extern_inline);
      MaybeSuggestAddingStaticToDecl(CurFD);
    }
  }

  if (D.getDeclSpec().isModulePrivateSpecified()) {
    if (IsVariableTemplateSpecialization)
      Diag(NewVD->getLocation(), diag::err_module_private_specialization)
          << (IsPartialSpecialization ? 1 : 0)
          << FixItHint::CreateRemoval(
                 D.getDeclSpec().getModulePrivateSpecLoc());
    else if (IsExplicitSpecialization)
      Diag(NewVD->getLocation(), diag::err_module_private_specialization)
        << 2
        << FixItHint::CreateRemoval(D.getDeclSpec().getModulePrivateSpecLoc());
    else if (NewVD->hasLocalStorage())
      Diag(NewVD->getLocation(), diag::err_module_private_local)
        << 0 << NewVD->getDeclName()
        << SourceRange(D.getDeclSpec().getModulePrivateSpecLoc())
        << FixItHint::CreateRemoval(D.getDeclSpec().getModulePrivateSpecLoc());
    else {
      NewVD->setModulePrivate();
      if (NewTemplate)
        NewTemplate->setModulePrivate();
    }
  }

  // Handle attributes prior to checking for duplicates in MergeVarDecl
  ProcessDeclAttributes(S, NewVD, D);

  if (getLangOpts().CUDA) {
    if (EmitTLSUnsupportedError && DeclAttrsMatchCUDAMode(getLangOpts(), NewVD))
      Diag(D.getDeclSpec().getThreadStorageClassSpecLoc(),
           diag::err_thread_unsupported);
    // CUDA B.2.5: "__shared__ and __constant__ variables have implied static
    // storage [duration]."
    if (SC == SC_None && S->getFnParent() != nullptr &&
        (NewVD->hasAttr<CUDASharedAttr>() ||
         NewVD->hasAttr<CUDAConstantAttr>())) {
      NewVD->setStorageClass(SC_Static);
    }
  }

  // Ensure that dllimport globals without explicit storage class are treated as
  // extern. The storage class is set above using parsed attributes. Now we can
  // check the VarDecl itself.
  assert(!NewVD->hasAttr<DLLImportAttr>() ||
         NewVD->getAttr<DLLImportAttr>()->isInherited() ||
         NewVD->isStaticDataMember() || NewVD->getStorageClass() != SC_None);

  // In auto-retain/release, infer strong retension for variables of
  // retainable type.
  if (getLangOpts().ObjCAutoRefCount && inferObjCARCLifetime(NewVD))
    NewVD->setInvalidDecl();

  // Handle GNU asm-label extension (encoded as an attribute).
  if (Expr *E = (Expr*)D.getAsmLabel()) {
    // The parser guarantees this is a string.
    StringLiteral *SE = cast<StringLiteral>(E);
    StringRef Label = SE->getString();
    if (S->getFnParent() != nullptr) {
      switch (SC) {
      case SC_None:
      case SC_Auto:
        Diag(E->getExprLoc(), diag::warn_asm_label_on_auto_decl) << Label;
        break;
      case SC_Register:
        // Local Named register
        if (!Context.getTargetInfo().isValidGCCRegisterName(Label))
          Diag(E->getExprLoc(), diag::err_asm_unknown_register_name) << Label;
        break;
      case SC_Static:
      case SC_Extern:
      case SC_PrivateExtern:
      case SC_OpenCLWorkGroupLocal:
        break;
      }
    } else if (SC == SC_Register) {
      // Global Named register
      if (!Context.getTargetInfo().isValidGCCRegisterName(Label))
        Diag(E->getExprLoc(), diag::err_asm_unknown_register_name) << Label;
      if (!R->isIntegralType(Context) && !R->isPointerType()) {
        Diag(D.getLocStart(), diag::err_asm_bad_register_type);
        NewVD->setInvalidDecl(true);
      }
    }

    NewVD->addAttr(::new (Context) AsmLabelAttr(SE->getStrTokenLoc(0),
                                                Context, Label, 0));
  } else if (!ExtnameUndeclaredIdentifiers.empty() &&
             isDeclTUScopedExternallyVisible(NewVD)) {
    llvm::DenseMap<IdentifierInfo*,AsmLabelAttr*>::iterator I =
      ExtnameUndeclaredIdentifiers.find(NewVD->getIdentifier());
    if (I != ExtnameUndeclaredIdentifiers.end()) {
      NewVD->addAttr(I->second);
      ExtnameUndeclaredIdentifiers.erase(I);
    }
  }

  // Diagnose shadowed variables before filtering for scope.
  if (D.getCXXScopeSpec().isEmpty())
    CheckShadow(S, NewVD, Previous);

  // Don't consider existing declarations that are in a different
  // scope and are out-of-semantic-context declarations (if the new
  // declaration has linkage).
  FilterLookupForScope(Previous, OriginalDC, S, shouldConsiderLinkage(NewVD),
                       D.getCXXScopeSpec().isNotEmpty() ||
                       IsExplicitSpecialization ||
                       IsVariableTemplateSpecialization);

  // Check whether the previous declaration is in the same block scope. This
  // affects whether we merge types with it, per C++11 [dcl.array]p3.
  if (getLangOpts().CPlusPlus &&
      NewVD->isLocalVarDecl() && NewVD->hasExternalStorage())
    NewVD->setPreviousDeclInSameBlockScope(
        Previous.isSingleResult() && !Previous.isShadowed() &&
        isDeclInScope(Previous.getFoundDecl(), OriginalDC, S, false));

  if (!getLangOpts().CPlusPlus) {
    D.setRedeclaration(CheckVariableDeclaration(NewVD, Previous));
  } else {
    // If this is an explicit specialization of a static data member, check it.
    if (IsExplicitSpecialization && !NewVD->isInvalidDecl() &&
        CheckMemberSpecialization(NewVD, Previous))
      NewVD->setInvalidDecl();

    // Merge the decl with the existing one if appropriate.
    if (!Previous.empty()) {
      if (Previous.isSingleResult() &&
          isa<FieldDecl>(Previous.getFoundDecl()) &&
          D.getCXXScopeSpec().isSet()) {
        // The user tried to define a non-static data member
        // out-of-line (C++ [dcl.meaning]p1).
        Diag(NewVD->getLocation(), diag::err_nonstatic_member_out_of_line)
          << D.getCXXScopeSpec().getRange();
        Previous.clear();
        NewVD->setInvalidDecl();
      }
    } else if (D.getCXXScopeSpec().isSet()) {
      // No previous declaration in the qualifying scope.
      Diag(D.getIdentifierLoc(), diag::err_no_member)
        << Name << computeDeclContext(D.getCXXScopeSpec(), true)
        << D.getCXXScopeSpec().getRange();
      NewVD->setInvalidDecl();
    }

    if (!IsVariableTemplateSpecialization)
        D.setRedeclaration(CheckVariableDeclaration(NewVD, Previous, MergeState)); // HLSL Change - add merge state

    if (NewTemplate) {
      VarTemplateDecl *PrevVarTemplate =
          NewVD->getPreviousDecl()
              ? NewVD->getPreviousDecl()->getDescribedVarTemplate()
              : nullptr;

      // Check the template parameter list of this declaration, possibly
      // merging in the template parameter list from the previous variable
      // template declaration.
      if (CheckTemplateParameterList(
              TemplateParams,
              PrevVarTemplate ? PrevVarTemplate->getTemplateParameters()
                              : nullptr,
              (D.getCXXScopeSpec().isSet() && DC && DC->isRecord() &&
               DC->isDependentContext())
                  ? TPC_ClassTemplateMember
                  : TPC_VarTemplate))
        NewVD->setInvalidDecl();

      // If we are providing an explicit specialization of a static variable
      // template, make a note of that.
      if (PrevVarTemplate &&
          PrevVarTemplate->getInstantiatedFromMemberTemplate())
        PrevVarTemplate->setMemberSpecialization();
    }
  }

  ProcessPragmaWeak(S, NewVD);

  // If this is the first declaration of an extern C variable, update
  // the map of such variables.
  if (NewVD->isFirstDecl() && !NewVD->isInvalidDecl() &&
      isIncompleteDeclExternC(*this, NewVD))
    RegisterLocallyScopedExternCDecl(NewVD, S);

  if (getLangOpts().CPlusPlus && NewVD->isStaticLocal()) {
    Decl *ManglingContextDecl;
    if (MangleNumberingContext *MCtx = getCurrentMangleNumberContext(
            NewVD->getDeclContext(), ManglingContextDecl)) {
      Context.setManglingNumber(
          NewVD, MCtx->getManglingNumber(
                     NewVD, getMSManglingNumber(getLangOpts(), S)));
      Context.setStaticLocalNumber(NewVD, MCtx->getStaticLocalNumber(NewVD));
    }
  }

  if (D.isRedeclaration() && !Previous.empty()) {
    checkDLLAttributeRedeclaration(
        *this, dyn_cast<NamedDecl>(Previous.getRepresentativeDecl()), NewVD,
        IsExplicitSpecialization);
  }

  if (NewTemplate) {
    if (NewVD->isInvalidDecl())
      NewTemplate->setInvalidDecl();
    ActOnDocumentableDecl(NewTemplate);
    return NewTemplate;
  }

  TransferUnusualAttributes(D, NewVD); // HLSL Change
  return NewVD;
}

/// \brief Diagnose variable or built-in function shadowing.  Implements
/// -Wshadow.
///
/// This method is called whenever a VarDecl is added to a "useful"
/// scope.
///
/// \param S the scope in which the shadowing name is being declared
/// \param R the lookup of the name
///
void Sema::CheckShadow(Scope *S, VarDecl *D, const LookupResult& R) {
  // Return if warning is ignored.
  if (Diags.isIgnored(diag::warn_decl_shadow, R.getNameLoc()))
    return;

  // Don't diagnose declarations at file scope.
  if (D->hasGlobalStorage())
    return;

  DeclContext *NewDC = D->getDeclContext();

  // Only diagnose if we're shadowing an unambiguous field or variable.
  if (R.getResultKind() != LookupResult::Found)
    return;

  NamedDecl* ShadowedDecl = R.getFoundDecl();
  if (!isa<VarDecl>(ShadowedDecl) && !isa<FieldDecl>(ShadowedDecl))
    return;

  // Fields are not shadowed by variables in C++ static methods.
  if (isa<FieldDecl>(ShadowedDecl))
    if (CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(NewDC))
      if (MD->isStatic())
        return;

  if (VarDecl *shadowedVar = dyn_cast<VarDecl>(ShadowedDecl))
    if (shadowedVar->isExternC()) {
      // For shadowing external vars, make sure that we point to the global
      // declaration, not a locally scoped extern declaration.
      for (auto I : shadowedVar->redecls())
        if (I->isFileVarDecl()) {
          ShadowedDecl = I;
          break;
        }
    }

  DeclContext *OldDC = ShadowedDecl->getDeclContext();

  // Only warn about certain kinds of shadowing for class members.
  if (NewDC && NewDC->isRecord()) {
    // In particular, don't warn about shadowing non-class members.
    if (!OldDC->isRecord())
      return;

    // TODO: should we warn about static data members shadowing
    // static data members from base classes?
    
    // TODO: don't diagnose for inaccessible shadowed members.
    // This is hard to do perfectly because we might friend the
    // shadowing context, but that's just a false negative.
  }

  // Determine what kind of declaration we're shadowing.
  unsigned Kind;
  if (isa<RecordDecl>(OldDC)) {
    if (isa<FieldDecl>(ShadowedDecl))
      Kind = 3; // field
    else
      Kind = 2; // static data member
  } else if (OldDC->isFileContext())
    Kind = 1; // global
  else
    Kind = 0; // local

  DeclarationName Name = R.getLookupName();

  // Emit warning and note.
  if (getSourceManager().isInSystemMacro(R.getNameLoc()))
    return;
  Diag(R.getNameLoc(), diag::warn_decl_shadow) << Name << Kind << OldDC;
  Diag(ShadowedDecl->getLocation(), diag::note_previous_declaration);
}

/// \brief Check -Wshadow without the advantage of a previous lookup.
void Sema::CheckShadow(Scope *S, VarDecl *D) {
  if (Diags.isIgnored(diag::warn_decl_shadow, D->getLocation()))
    return;

  LookupResult R(*this, D->getDeclName(), D->getLocation(),
                 Sema::LookupOrdinaryName, Sema::ForRedeclaration);
  LookupName(R, S);
  CheckShadow(S, D, R);
}

/// Check for conflict between this global or extern "C" declaration and
/// previous global or extern "C" declarations. This is only used in C++.
template<typename T>
static bool checkGlobalOrExternCConflict(
    Sema &S, const T *ND, bool IsGlobal, LookupResult &Previous) {
  assert(S.getLangOpts().CPlusPlus && "only C++ has extern \"C\"");
  NamedDecl *Prev = S.findLocallyScopedExternCDecl(ND->getDeclName());

  if (!Prev && IsGlobal && !isIncompleteDeclExternC(S, ND)) {
    // The common case: this global doesn't conflict with any extern "C"
    // declaration.
    return false;
  }

  if (Prev) {
    if (!IsGlobal || isIncompleteDeclExternC(S, ND)) {
      // Both the old and new declarations have C language linkage. This is a
      // redeclaration.
      Previous.clear();
      Previous.addDecl(Prev);
      return true;
    }

    // This is a global, non-extern "C" declaration, and there is a previous
    // non-global extern "C" declaration. Diagnose if this is a variable
    // declaration.
    if (!isa<VarDecl>(ND))
      return false;
  } else {
    // The declaration is extern "C". Check for any declaration in the
    // translation unit which might conflict.
    if (IsGlobal) {
      // We have already performed the lookup into the translation unit.
      IsGlobal = false;
      for (LookupResult::iterator I = Previous.begin(), E = Previous.end();
           I != E; ++I) {
        if (isa<VarDecl>(*I)) {
          Prev = *I;
          break;
        }
      }
    } else {
      DeclContext::lookup_result R =
          S.Context.getTranslationUnitDecl()->lookup(ND->getDeclName());
      for (DeclContext::lookup_result::iterator I = R.begin(), E = R.end();
           I != E; ++I) {
        if (isa<VarDecl>(*I)) {
          Prev = *I;
          break;
        }
        // FIXME: If we have any other entity with this name in global scope,
        // the declaration is ill-formed, but that is a defect: it breaks the
        // 'stat' hack, for instance. Only variables can have mangled name
        // clashes with extern "C" declarations, so only they deserve a
        // diagnostic.
      }
    }

    if (!Prev)
      return false;
  }

  // Use the first declaration's location to ensure we point at something which
  // is lexically inside an extern "C" linkage-spec.
  assert(Prev && "should have found a previous declaration to diagnose");
  if (FunctionDecl *FD = dyn_cast<FunctionDecl>(Prev))
    Prev = FD->getFirstDecl();
  else
    Prev = cast<VarDecl>(Prev)->getFirstDecl();

  S.Diag(ND->getLocation(), diag::err_extern_c_global_conflict)
    << IsGlobal << ND;
  S.Diag(Prev->getLocation(), diag::note_extern_c_global_conflict)
    << IsGlobal;
  return false;
}

/// Apply special rules for handling extern "C" declarations. Returns \c true
/// if we have found that this is a redeclaration of some prior entity.
///
/// Per C++ [dcl.link]p6:
///   Two declarations [for a function or variable] with C language linkage
///   with the same name that appear in different scopes refer to the same
///   [entity]. An entity with C language linkage shall not be declared with
///   the same name as an entity in global scope.
template<typename T>
static bool checkForConflictWithNonVisibleExternC(Sema &S, const T *ND,
                                                  LookupResult &Previous) {
  if (!S.getLangOpts().CPlusPlus) {
    // In C, when declaring a global variable, look for a corresponding 'extern'
    // variable declared in function scope. We don't need this in C++, because
    // we find local extern decls in the surrounding file-scope DeclContext.
    if (ND->getDeclContext()->getRedeclContext()->isTranslationUnit()) {
      if (NamedDecl *Prev = S.findLocallyScopedExternCDecl(ND->getDeclName())) {
        Previous.clear();
        Previous.addDecl(Prev);
        return true;
      }
    }
    return false;
  }

  // A declaration in the translation unit can conflict with an extern "C"
  // declaration.
  if (ND->getDeclContext()->getRedeclContext()->isTranslationUnit())
    return checkGlobalOrExternCConflict(S, ND, /*IsGlobal*/true, Previous);

  // An extern "C" declaration can conflict with a declaration in the
  // translation unit or can be a redeclaration of an extern "C" declaration
  // in another scope.
  if (isIncompleteDeclExternC(S,ND))
    return checkGlobalOrExternCConflict(S, ND, /*IsGlobal*/false, Previous);

  // Neither global nor extern "C": nothing to do.
  return false;
}

void Sema::CheckVariableDeclarationType(VarDecl *NewVD) {
  // If the decl is already known invalid, don't check it.
  if (NewVD->isInvalidDecl())
    return;

  TypeSourceInfo *TInfo = NewVD->getTypeSourceInfo();
  QualType T = TInfo->getType();

  // Defer checking an 'auto' type until its initializer is attached.
  if (T->isUndeducedType())
    return;

  if (NewVD->hasAttrs())
    CheckAlignasUnderalignment(NewVD);

  if (T->isObjCObjectType()) {
    Diag(NewVD->getLocation(), diag::err_statically_allocated_object)
      << FixItHint::CreateInsertion(NewVD->getLocation(), "*");
    T = Context.getObjCObjectPointerType(T);
    NewVD->setType(T);
  }

  // Emit an error if an address space was applied to decl with local storage.
  // This includes arrays of objects with address space qualifiers, but not
  // automatic variables that point to other address spaces.
  // ISO/IEC TR 18037 S5.1.2
  if (NewVD->hasLocalStorage() && T.getAddressSpace() != 0) {
    Diag(NewVD->getLocation(), diag::err_as_qualified_auto_decl);
    NewVD->setInvalidDecl();
    return;
  }

  // OpenCL v1.2 s6.5 - All program scope variables must be declared in the
  // __constant address space.
  if (getLangOpts().OpenCL && NewVD->isFileVarDecl()
      && T.getAddressSpace() != LangAS::opencl_constant
      && !T->isSamplerT()){
    Diag(NewVD->getLocation(), diag::err_opencl_global_invalid_addr_space);
    NewVD->setInvalidDecl();
    return;
  }
  
  // OpenCL v1.2 s6.8 -- The static qualifier is valid only in program
  // scope.
  if ((getLangOpts().OpenCLVersion >= 120)
      && NewVD->isStaticLocal()) {
    Diag(NewVD->getLocation(), diag::err_static_function_scope);
    NewVD->setInvalidDecl();
    return;
  }

  if (NewVD->hasLocalStorage() && T.isObjCGCWeak()
      && !NewVD->hasAttr<BlocksAttr>()) {
    if (getLangOpts().getGC() != LangOptions::NonGC)
      Diag(NewVD->getLocation(), diag::warn_gc_attribute_weak_on_local);
    else {
      assert(!getLangOpts().ObjCAutoRefCount);
      Diag(NewVD->getLocation(), diag::warn_attribute_weak_on_local);
    }
  }
  
  bool isVM = T->isVariablyModifiedType();
  if (isVM || NewVD->hasAttr<CleanupAttr>() ||
      NewVD->hasAttr<BlocksAttr>())
    getCurFunction()->setHasBranchProtectedScope();

  if ((isVM && NewVD->hasLinkage()) ||
      (T->isVariableArrayType() && NewVD->hasGlobalStorage())) {
    bool SizeIsNegative;
    llvm::APSInt Oversized;
    TypeSourceInfo *FixedTInfo =
      TryToFixInvalidVariablyModifiedTypeSourceInfo(TInfo, Context,
                                                    SizeIsNegative, Oversized);
    if (!FixedTInfo && T->isVariableArrayType()) {
      const VariableArrayType *VAT = Context.getAsVariableArrayType(T);
      // FIXME: This won't give the correct result for
      // int a[10][n];
      SourceRange SizeRange = VAT->getSizeExpr()->getSourceRange();

      if (NewVD->isFileVarDecl())
        Diag(NewVD->getLocation(), diag::err_vla_decl_in_file_scope)
        << SizeRange;
      else if (NewVD->isStaticLocal())
        Diag(NewVD->getLocation(), diag::err_vla_decl_has_static_storage)
        << SizeRange;
      else
        Diag(NewVD->getLocation(), diag::err_vla_decl_has_extern_linkage)
        << SizeRange;
      NewVD->setInvalidDecl();
      return;
    }

    if (!FixedTInfo) {
      if (NewVD->isFileVarDecl())
        Diag(NewVD->getLocation(), diag::err_vm_decl_in_file_scope);
      else
        Diag(NewVD->getLocation(), diag::err_vm_decl_has_extern_linkage);
      NewVD->setInvalidDecl();
      return;
    }

    Diag(NewVD->getLocation(), diag::warn_illegal_constant_array_size);
    NewVD->setType(FixedTInfo->getType());
    NewVD->setTypeSourceInfo(FixedTInfo);
  }

  if (T->isVoidType()) {
    // C++98 [dcl.stc]p5: The extern specifier can be applied only to the names
    //                    of objects and functions.
    if (NewVD->isThisDeclarationADefinition() || getLangOpts().CPlusPlus) {
      Diag(NewVD->getLocation(), diag::err_typecheck_decl_incomplete_type)
        << T;
      NewVD->setInvalidDecl();
      return;
    }
  }

  if (!NewVD->hasLocalStorage() && NewVD->hasAttr<BlocksAttr>()) {
    Diag(NewVD->getLocation(), diag::err_block_on_nonlocal);
    NewVD->setInvalidDecl();
    return;
  }

  if (isVM && NewVD->hasAttr<BlocksAttr>()) {
    Diag(NewVD->getLocation(), diag::err_block_on_vm);
    NewVD->setInvalidDecl();
    return;
  }

  if (NewVD->isConstexpr() && !T->isDependentType() &&
      RequireLiteralType(NewVD->getLocation(), T,
                         diag::err_constexpr_var_non_literal)) {
    NewVD->setInvalidDecl();
    return;
  }
}

/// \brief Perform semantic checking on a newly-created variable
/// declaration.
///
/// This routine performs all of the type-checking required for a
/// variable declaration once it has been built. It is used both to
/// check variables after they have been parsed and their declarators
/// have been translated into a declaration, and to check variables
/// that have been instantiated from a template.
///
/// Sets NewVD->isInvalidDecl() if an error was encountered.
///
/// Returns true if the variable declaration is a redeclaration.
bool Sema::CheckVariableDeclaration(VarDecl *NewVD, LookupResult &Previous, ShadowMergeState MergeState) { // HLSL Change - add merge state
  CheckVariableDeclarationType(NewVD);

  // If the decl is already known invalid, don't check it.
  if (NewVD->isInvalidDecl())
    return false;

  // If we did not find anything by this name, look for a non-visible
  // extern "C" declaration with the same name.
  if (Previous.empty() &&
      checkForConflictWithNonVisibleExternC(*this, NewVD, Previous))
    Previous.setShadowed();

  // Filter out any non-conflicting previous declarations.
  filterNonConflictingPreviousDecls(*this, NewVD, Previous);

  if (!Previous.empty()) {
    MergeVarDecl(NewVD, Previous, MergeState);    // HLSL Change - add merge state
    return true;
  }
  return false;
}

/// \brief Data used with FindOverriddenMethod
struct FindOverriddenMethodData {
  Sema *S;
  CXXMethodDecl *Method;
};

/// \brief Member lookup function that determines whether a given C++
/// method overrides a method in a base class, to be used with
/// CXXRecordDecl::lookupInBases().
static bool FindOverriddenMethod(const CXXBaseSpecifier *Specifier,
                                 CXXBasePath &Path,
                                 void *UserData) {
  RecordDecl *BaseRecord = Specifier->getType()->getAs<RecordType>()->getDecl();

  FindOverriddenMethodData *Data 
    = reinterpret_cast<FindOverriddenMethodData*>(UserData);
  
  DeclarationName Name = Data->Method->getDeclName();
  
  // FIXME: Do we care about other names here too?
  if (Name.getNameKind() == DeclarationName::CXXDestructorName) {
    // We really want to find the base class destructor here.
    QualType T = Data->S->Context.getTypeDeclType(BaseRecord);
    CanQualType CT = Data->S->Context.getCanonicalType(T);
    
    Name = Data->S->Context.DeclarationNames.getCXXDestructorName(CT);
  }    
  
  for (Path.Decls = BaseRecord->lookup(Name);
       !Path.Decls.empty();
       Path.Decls = Path.Decls.slice(1)) {
    NamedDecl *D = Path.Decls.front();
    if (CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(D)) {
      if (MD->isVirtual() && !Data->S->IsOverload(Data->Method, MD, false))
        return true;
    }
  }
  
  return false;
}

namespace {
  enum OverrideErrorKind { OEK_All, OEK_NonDeleted, OEK_Deleted };
}
/// \brief Report an error regarding overriding, along with any relevant
/// overriden methods.
///
/// \param DiagID the primary error to report.
/// \param MD the overriding method.
/// \param OEK which overrides to include as notes.
static void ReportOverrides(Sema& S, unsigned DiagID, const CXXMethodDecl *MD,
                            OverrideErrorKind OEK = OEK_All) {
  S.Diag(MD->getLocation(), DiagID) << MD->getDeclName();
  for (CXXMethodDecl::method_iterator I = MD->begin_overridden_methods(),
                                      E = MD->end_overridden_methods();
       I != E; ++I) {
    // This check (& the OEK parameter) could be replaced by a predicate, but
    // without lambdas that would be overkill. This is still nicer than writing
    // out the diag loop 3 times.
    if ((OEK == OEK_All) ||
        (OEK == OEK_NonDeleted && !(*I)->isDeleted()) ||
        (OEK == OEK_Deleted && (*I)->isDeleted()))
      S.Diag((*I)->getLocation(), diag::note_overridden_virtual_function);
  }
}

/// AddOverriddenMethods - See if a method overrides any in the base classes,
/// and if so, check that it's a valid override and remember it.
bool Sema::AddOverriddenMethods(CXXRecordDecl *DC, CXXMethodDecl *MD) {
  // Look for methods in base classes that this method might override.
  CXXBasePaths Paths;
  FindOverriddenMethodData Data;
  Data.Method = MD;
  Data.S = this;
  bool hasDeletedOverridenMethods = false;
  bool hasNonDeletedOverridenMethods = false;
  bool AddedAny = false;
  if (DC->lookupInBases(&FindOverriddenMethod, &Data, Paths)) {
    for (auto *I : Paths.found_decls()) {
      if (CXXMethodDecl *OldMD = dyn_cast<CXXMethodDecl>(I)) {
        MD->addOverriddenMethod(OldMD->getCanonicalDecl());
        if (!CheckOverridingFunctionReturnType(MD, OldMD) &&
            !CheckOverridingFunctionAttributes(MD, OldMD) &&
            !CheckOverridingFunctionExceptionSpec(MD, OldMD) &&
            !CheckIfOverriddenFunctionIsMarkedFinal(MD, OldMD)) {
          hasDeletedOverridenMethods |= OldMD->isDeleted();
          hasNonDeletedOverridenMethods |= !OldMD->isDeleted();
          AddedAny = true;
        }
      }
    }
  }

  if (hasDeletedOverridenMethods && !MD->isDeleted()) {
    ReportOverrides(*this, diag::err_non_deleted_override, MD, OEK_Deleted);
  }
  if (hasNonDeletedOverridenMethods && MD->isDeleted()) {
    ReportOverrides(*this, diag::err_deleted_override, MD, OEK_NonDeleted);
  }

  return AddedAny;
}

namespace {
  // Struct for holding all of the extra arguments needed by
  // DiagnoseInvalidRedeclaration to call Sema::ActOnFunctionDeclarator.
  struct ActOnFDArgs {
    Scope *S;
    Declarator &D;
    MultiTemplateParamsArg TemplateParamLists;
    bool AddToScope;
  };
}

namespace {

// Callback to only accept typo corrections that have a non-zero edit distance.
// Also only accept corrections that have the same parent decl.
class DifferentNameValidatorCCC : public CorrectionCandidateCallback {
 public:
  DifferentNameValidatorCCC(ASTContext &Context, FunctionDecl *TypoFD,
                            CXXRecordDecl *Parent)
      : Context(Context), OriginalFD(TypoFD),
        ExpectedParent(Parent ? Parent->getCanonicalDecl() : nullptr) {}

  bool ValidateCandidate(const TypoCorrection &candidate) override {
    if (candidate.getEditDistance() == 0)
      return false;

    SmallVector<unsigned, 1> MismatchedParams;
    for (TypoCorrection::const_decl_iterator CDecl = candidate.begin(),
                                          CDeclEnd = candidate.end();
         CDecl != CDeclEnd; ++CDecl) {
      FunctionDecl *FD = dyn_cast<FunctionDecl>(*CDecl);

      if (FD && !FD->hasBody() &&
          hasSimilarParameters(Context, FD, OriginalFD, MismatchedParams)) {
        if (CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(FD)) {
          CXXRecordDecl *Parent = MD->getParent();
          if (Parent && Parent->getCanonicalDecl() == ExpectedParent)
            return true;
        } else if (!ExpectedParent) {
          return true;
        }
      }
    }

    return false;
  }

 private:
  ASTContext &Context;
  FunctionDecl *OriginalFD;
  CXXRecordDecl *ExpectedParent;
};

}

/// \brief Generate diagnostics for an invalid function redeclaration.
///
/// This routine handles generating the diagnostic messages for an invalid
/// function redeclaration, including finding possible similar declarations
/// or performing typo correction if there are no previous declarations with
/// the same name.
///
/// Returns a NamedDecl iff typo correction was performed and substituting in
/// the new declaration name does not cause new errors.
static NamedDecl *DiagnoseInvalidRedeclaration(
    Sema &SemaRef, LookupResult &Previous, FunctionDecl *NewFD,
    ActOnFDArgs &ExtraArgs, bool IsLocalFriend, Scope *S) {
  DeclarationName Name = NewFD->getDeclName();
  DeclContext *NewDC = NewFD->getDeclContext();
  SmallVector<unsigned, 1> MismatchedParams;
  SmallVector<std::pair<FunctionDecl *, unsigned>, 1> NearMatches;
  TypoCorrection Correction;
  bool IsDefinition = ExtraArgs.D.isFunctionDefinition();
  unsigned DiagMsg = IsLocalFriend ? diag::err_no_matching_local_friend
                                   : diag::err_member_decl_does_not_match;
  LookupResult Prev(SemaRef, Name, NewFD->getLocation(),
                    IsLocalFriend ? Sema::LookupLocalFriendName
                                  : Sema::LookupOrdinaryName,
                    Sema::ForRedeclaration);

  NewFD->setInvalidDecl();
  if (IsLocalFriend)
    SemaRef.LookupName(Prev, S);
  else
    SemaRef.LookupQualifiedName(Prev, NewDC);
  assert(!Prev.isAmbiguous() &&
         "Cannot have an ambiguity in previous-declaration lookup");
  CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(NewFD);
  if (!Prev.empty()) {
    for (LookupResult::iterator Func = Prev.begin(), FuncEnd = Prev.end();
         Func != FuncEnd; ++Func) {
      FunctionDecl *FD = dyn_cast<FunctionDecl>(*Func);
      if (FD &&
          hasSimilarParameters(SemaRef.Context, FD, NewFD, MismatchedParams)) {
        // Add 1 to the index so that 0 can mean the mismatch didn't
        // involve a parameter
        unsigned ParamNum =
            MismatchedParams.empty() ? 0 : MismatchedParams.front() + 1;
        NearMatches.push_back(std::make_pair(FD, ParamNum));
      }
    }
  // If the qualified name lookup yielded nothing, try typo correction
  } else if ((Correction = SemaRef.CorrectTypo(
                  Prev.getLookupNameInfo(), Prev.getLookupKind(), S,
                  &ExtraArgs.D.getCXXScopeSpec(),
                  llvm::make_unique<DifferentNameValidatorCCC>(
                      SemaRef.Context, NewFD, MD ? MD->getParent() : nullptr),
                  Sema::CTK_ErrorRecovery, IsLocalFriend ? nullptr : NewDC))) {
    // Set up everything for the call to ActOnFunctionDeclarator
    ExtraArgs.D.SetIdentifier(Correction.getCorrectionAsIdentifierInfo(),
                              ExtraArgs.D.getIdentifierLoc());
    Previous.clear();
    Previous.setLookupName(Correction.getCorrection());
    for (TypoCorrection::decl_iterator CDecl = Correction.begin(),
                                    CDeclEnd = Correction.end();
         CDecl != CDeclEnd; ++CDecl) {
      FunctionDecl *FD = dyn_cast<FunctionDecl>(*CDecl);
      if (FD && !FD->hasBody() &&
          hasSimilarParameters(SemaRef.Context, FD, NewFD, MismatchedParams)) {
        Previous.addDecl(FD);
      }
    }
    bool wasRedeclaration = ExtraArgs.D.isRedeclaration();

    NamedDecl *Result;
    // Retry building the function declaration with the new previous
    // declarations, and with errors suppressed.
    {
      // Trap errors.
      Sema::SFINAETrap Trap(SemaRef);

      // TODO: Refactor ActOnFunctionDeclarator so that we can call only the
      // pieces need to verify the typo-corrected C++ declaration and hopefully
      // eliminate the need for the parameter pack ExtraArgs.
      Result = SemaRef.ActOnFunctionDeclarator(
          ExtraArgs.S, ExtraArgs.D,
          Correction.getCorrectionDecl()->getDeclContext(),
          NewFD->getTypeSourceInfo(), Previous, ExtraArgs.TemplateParamLists,
          ExtraArgs.AddToScope);

      if (Trap.hasErrorOccurred())
        Result = nullptr;
    }

    if (Result) {
      // Determine which correction we picked.
      Decl *Canonical = Result->getCanonicalDecl();
      for (LookupResult::iterator I = Previous.begin(), E = Previous.end();
           I != E; ++I)
        if ((*I)->getCanonicalDecl() == Canonical)
          Correction.setCorrectionDecl(*I);

      SemaRef.diagnoseTypo(
          Correction,
          SemaRef.PDiag(IsLocalFriend
                          ? diag::err_no_matching_local_friend_suggest
                          : diag::err_member_decl_does_not_match_suggest)
            << Name << NewDC << IsDefinition);
      return Result;
    }

    // Pretend the typo correction never occurred
    ExtraArgs.D.SetIdentifier(Name.getAsIdentifierInfo(),
                              ExtraArgs.D.getIdentifierLoc());
    ExtraArgs.D.setRedeclaration(wasRedeclaration);
    Previous.clear();
    Previous.setLookupName(Name);
  }

  SemaRef.Diag(NewFD->getLocation(), DiagMsg)
      << Name << NewDC << IsDefinition << NewFD->getLocation();

  bool NewFDisConst = false;
  if (CXXMethodDecl *NewMD = dyn_cast<CXXMethodDecl>(NewFD))
    NewFDisConst = NewMD->isConst();

  for (SmallVectorImpl<std::pair<FunctionDecl *, unsigned> >::iterator
       NearMatch = NearMatches.begin(), NearMatchEnd = NearMatches.end();
       NearMatch != NearMatchEnd; ++NearMatch) {
    FunctionDecl *FD = NearMatch->first;
    CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(FD);
    bool FDisConst = MD && MD->isConst();
    bool IsMember = MD || !IsLocalFriend;

    // FIXME: These notes are poorly worded for the local friend case.
    if (unsigned Idx = NearMatch->second) {
      ParmVarDecl *FDParam = FD->getParamDecl(Idx-1);
      SourceLocation Loc = FDParam->getTypeSpecStartLoc();
      if (Loc.isInvalid()) Loc = FD->getLocation();
      SemaRef.Diag(Loc, IsMember ? diag::note_member_def_close_param_match
                                 : diag::note_local_decl_close_param_match)
        << Idx << FDParam->getType()
        << NewFD->getParamDecl(Idx - 1)->getType();
    } else if (FDisConst != NewFDisConst) {
      SemaRef.Diag(FD->getLocation(), diag::note_member_def_close_const_match)
          << NewFDisConst << FD->getSourceRange().getEnd();
    } else
      SemaRef.Diag(FD->getLocation(),
                   IsMember ? diag::note_member_def_close_match
                            : diag::note_local_decl_close_match);
  }
  return nullptr;
}

static StorageClass getFunctionStorageClass(Sema &SemaRef, Declarator &D) {
  switch (D.getDeclSpec().getStorageClassSpec()) {
  default: llvm_unreachable("Unknown storage class!");
  case DeclSpec::SCS_auto:
  case DeclSpec::SCS_register:
  case DeclSpec::SCS_mutable:
    SemaRef.Diag(D.getDeclSpec().getStorageClassSpecLoc(),
                 diag::err_typecheck_sclass_func);
    D.setInvalidType();
    break;
  case DeclSpec::SCS_unspecified: break;
  case DeclSpec::SCS_extern:
    if (D.getDeclSpec().isExternInLinkageSpec())
      return SC_None;
    return SC_Extern;
  case DeclSpec::SCS_static: {
    if (SemaRef.CurContext->getRedeclContext()->isFunctionOrMethod()) {
      // C99 6.7.1p5:
      //   The declaration of an identifier for a function that has
      //   block scope shall have no explicit storage-class specifier
      //   other than extern
      // See also (C++ [dcl.stc]p4).
      SemaRef.Diag(D.getDeclSpec().getStorageClassSpecLoc(),
                   diag::err_static_block_func);
      break;
    } else
      return SC_Static;
  }
  case DeclSpec::SCS_private_extern: return SC_PrivateExtern;
  }

  // No explicit storage class has already been returned
  return SC_None;
}

static FunctionDecl* CreateNewFunctionDecl(Sema &SemaRef, Declarator &D,
                                           DeclContext *DC, QualType &R,
                                           TypeSourceInfo *TInfo,
                                           StorageClass SC,
                                           bool &IsVirtualOkay) {
  DeclarationNameInfo NameInfo = SemaRef.GetNameForDeclarator(D);
  DeclarationName Name = NameInfo.getName();

  FunctionDecl *NewFD = nullptr;
  bool isInline = D.getDeclSpec().isInlineSpecified();

  if (!SemaRef.getLangOpts().CPlusPlus) {
    // Determine whether the function was written with a
    // prototype. This true when:
    //   - there is a prototype in the declarator, or
    //   - the type R of the function is some kind of typedef or other reference
    //     to a type name (which eventually refers to a function type).
    bool HasPrototype =
      (D.isFunctionDeclarator() && D.getFunctionTypeInfo().hasPrototype) ||
      (!isa<FunctionType>(R.getTypePtr()) && R->isFunctionProtoType());

    NewFD = FunctionDecl::Create(SemaRef.Context, DC, 
                                 D.getLocStart(), NameInfo, R, 
                                 TInfo, SC, isInline, 
                                 HasPrototype, false);
    if (D.isInvalidType())
      NewFD->setInvalidDecl();

    return NewFD;
  }

  bool isExplicit = D.getDeclSpec().isExplicitSpecified();
  bool isConstexpr = D.getDeclSpec().isConstexprSpecified();

  // Check that the return type is not an abstract class type.
  // For record types, this is done by the AbstractClassUsageDiagnoser once
  // the class has been completely parsed.
  if (!DC->isRecord() &&
      SemaRef.RequireNonAbstractType(
          D.getIdentifierLoc(), R->getAs<FunctionType>()->getReturnType(),
          diag::err_abstract_type_in_decl, SemaRef.AbstractReturnType))
    D.setInvalidType();

  if (Name.getNameKind() == DeclarationName::CXXConstructorName) {
    // This is a C++ constructor declaration.
    assert(DC->isRecord() &&
           "Constructors can only be declared in a member context");

    R = SemaRef.CheckConstructorDeclarator(D, R, SC);
    return CXXConstructorDecl::Create(SemaRef.Context, cast<CXXRecordDecl>(DC),
                                      D.getLocStart(), NameInfo,
                                      R, TInfo, isExplicit, isInline,
                                      /*isImplicitlyDeclared=*/false,
                                      isConstexpr);

  } else if (Name.getNameKind() == DeclarationName::CXXDestructorName) {
    // This is a C++ destructor declaration.
    if (DC->isRecord()) {
      R = SemaRef.CheckDestructorDeclarator(D, R, SC);
      CXXRecordDecl *Record = cast<CXXRecordDecl>(DC);
      CXXDestructorDecl *NewDD = CXXDestructorDecl::Create(
                                        SemaRef.Context, Record,
                                        D.getLocStart(),
                                        NameInfo, R, TInfo, isInline,
                                        /*isImplicitlyDeclared=*/false);

      // If the class is complete, then we now create the implicit exception
      // specification. If the class is incomplete or dependent, we can't do
      // it yet.
      if (SemaRef.getLangOpts().CPlusPlus11 && !Record->isDependentType() &&
          Record->getDefinition() && !Record->isBeingDefined() &&
          R->getAs<FunctionProtoType>()->getExceptionSpecType() == EST_None) {
        SemaRef.AdjustDestructorExceptionSpec(Record, NewDD);
      }

      IsVirtualOkay = true;
      return NewDD;

    } else {
      SemaRef.Diag(D.getIdentifierLoc(), diag::err_destructor_not_member);
      D.setInvalidType();

      // Create a FunctionDecl to satisfy the function definition parsing
      // code path.
      return FunctionDecl::Create(SemaRef.Context, DC,
                                  D.getLocStart(),
                                  D.getIdentifierLoc(), Name, R, TInfo,
                                  SC, isInline,
                                  /*hasPrototype=*/true, isConstexpr);
    }

  } else if (Name.getNameKind() == DeclarationName::CXXConversionFunctionName) {
    if (!DC->isRecord()) {
      SemaRef.Diag(D.getIdentifierLoc(),
           diag::err_conv_function_not_member);
      return nullptr;
    }

    SemaRef.CheckConversionDeclarator(D, R, SC);
    IsVirtualOkay = true;
    return CXXConversionDecl::Create(SemaRef.Context, cast<CXXRecordDecl>(DC),
                                     D.getLocStart(), NameInfo,
                                     R, TInfo, isInline, isExplicit,
                                     isConstexpr, SourceLocation());

  } else if (DC->isRecord()) {
    // If the name of the function is the same as the name of the record,
    // then this must be an invalid constructor that has a return type.
    // (The parser checks for a return type and makes the declarator a
    // constructor if it has no return type).
    if (Name.getAsIdentifierInfo() &&
        Name.getAsIdentifierInfo() == cast<CXXRecordDecl>(DC)->getIdentifier()){
      SemaRef.Diag(D.getIdentifierLoc(), diag::err_constructor_return_type)
        << SourceRange(D.getDeclSpec().getTypeSpecTypeLoc())
        << SourceRange(D.getIdentifierLoc());
      return nullptr;
    }

    // This is a C++ method declaration.
    CXXMethodDecl *Ret = CXXMethodDecl::Create(SemaRef.Context,
                                               cast<CXXRecordDecl>(DC),
                                               D.getLocStart(), NameInfo, R,
                                               TInfo, SC, isInline,
                                               isConstexpr, SourceLocation());
    IsVirtualOkay = !Ret->isStatic();
    return Ret;
  } else {
    bool isFriend =
        SemaRef.getLangOpts().CPlusPlus && D.getDeclSpec().isFriendSpecified();
    if (!isFriend && SemaRef.CurContext->isRecord())
      return nullptr;

    // Determine whether the function was written with a
    // prototype. This true when:
    //   - we're in C++ (where every function has a prototype),
    return FunctionDecl::Create(SemaRef.Context, DC,
                                D.getLocStart(),
                                NameInfo, R, TInfo, SC, isInline,
                                true/*HasPrototype*/, isConstexpr);
  }
}

enum OpenCLParamType {
  ValidKernelParam,
  PtrPtrKernelParam,
  PtrKernelParam,
  PrivatePtrKernelParam,
  InvalidKernelParam,
  RecordKernelParam
};

static OpenCLParamType getOpenCLKernelParameterType(QualType PT) {
  if (PT->isPointerType()) {
    QualType PointeeType = PT->getPointeeType();
    if (PointeeType->isPointerType())
      return PtrPtrKernelParam;
    return PointeeType.getAddressSpace() == 0 ? PrivatePtrKernelParam
                                              : PtrKernelParam;
  }

  // TODO: Forbid the other integer types (size_t, ptrdiff_t...) when they can
  // be used as builtin types.

  if (PT->isImageType())
    return PtrKernelParam;

  if (PT->isBooleanType())
    return InvalidKernelParam;

  if (PT->isEventT())
    return InvalidKernelParam;

  if (PT->isHalfType())
    return InvalidKernelParam;

  if (PT->isRecordType())
    return RecordKernelParam;

  return ValidKernelParam;
}

static void checkIsValidOpenCLKernelParameter(
  Sema &S,
  Declarator &D,
  ParmVarDecl *Param,
  llvm::SmallPtrSetImpl<const Type *> &ValidTypes) {
  QualType PT = Param->getType();

  // Cache the valid types we encounter to avoid rechecking structs that are
  // used again
  if (ValidTypes.count(PT.getTypePtr()))
    return;

  switch (getOpenCLKernelParameterType(PT)) {
  case PtrPtrKernelParam:
    // OpenCL v1.2 s6.9.a:
    // A kernel function argument cannot be declared as a
    // pointer to a pointer type.
    S.Diag(Param->getLocation(), diag::err_opencl_ptrptr_kernel_param);
    D.setInvalidType();
    return;

  case PrivatePtrKernelParam:
    // OpenCL v1.2 s6.9.a:
    // A kernel function argument cannot be declared as a
    // pointer to the private address space.
    S.Diag(Param->getLocation(), diag::err_opencl_private_ptr_kernel_param);
    D.setInvalidType();
    return;

    // OpenCL v1.2 s6.9.k:
    // Arguments to kernel functions in a program cannot be declared with the
    // built-in scalar types bool, half, size_t, ptrdiff_t, intptr_t, and
    // uintptr_t or a struct and/or union that contain fields declared to be
    // one of these built-in scalar types.

  case InvalidKernelParam:
    // OpenCL v1.2 s6.8 n:
    // A kernel function argument cannot be declared
    // of event_t type.
    S.Diag(Param->getLocation(), diag::err_bad_kernel_param_type) << PT;
    D.setInvalidType();
    return;

  case PtrKernelParam:
  case ValidKernelParam:
    ValidTypes.insert(PT.getTypePtr());
    return;

  case RecordKernelParam:
    break;
  }

  // Track nested structs we will inspect
  SmallVector<const Decl *, 4> VisitStack;

  // Track where we are in the nested structs. Items will migrate from
  // VisitStack to HistoryStack as we do the DFS for bad field.
  SmallVector<const FieldDecl *, 4> HistoryStack;
  HistoryStack.push_back(nullptr);

  const RecordDecl *PD = PT->castAs<RecordType>()->getDecl();
  VisitStack.push_back(PD);

  assert(VisitStack.back() && "First decl null?");

  do {
    const Decl *Next = VisitStack.pop_back_val();
    if (!Next) {
      assert(!HistoryStack.empty());
      // Found a marker, we have gone up a level
      if (const FieldDecl *Hist = HistoryStack.pop_back_val())
        ValidTypes.insert(Hist->getType().getTypePtr());

      continue;
    }

    // Adds everything except the original parameter declaration (which is not a
    // field itself) to the history stack.
    const RecordDecl *RD;
    if (const FieldDecl *Field = dyn_cast<FieldDecl>(Next)) {
      HistoryStack.push_back(Field);
      RD = Field->getType()->castAs<RecordType>()->getDecl();
    } else {
      RD = cast<RecordDecl>(Next);
    }

    // Add a null marker so we know when we've gone back up a level
    VisitStack.push_back(nullptr);

    for (const auto *FD : RD->fields()) {
      QualType QT = FD->getType();

      if (ValidTypes.count(QT.getTypePtr()))
        continue;

      OpenCLParamType ParamType = getOpenCLKernelParameterType(QT);
      if (ParamType == ValidKernelParam)
        continue;

      if (ParamType == RecordKernelParam) {
        VisitStack.push_back(FD);
        continue;
      }

      // OpenCL v1.2 s6.9.p:
      // Arguments to kernel functions that are declared to be a struct or union
      // do not allow OpenCL objects to be passed as elements of the struct or
      // union.
      if (ParamType == PtrKernelParam || ParamType == PtrPtrKernelParam ||
          ParamType == PrivatePtrKernelParam) {
        S.Diag(Param->getLocation(),
               diag::err_record_with_pointers_kernel_param)
          << PT->isUnionType()
          << PT;
      } else {
        S.Diag(Param->getLocation(), diag::err_bad_kernel_param_type) << PT;
      }

      S.Diag(PD->getLocation(), diag::note_within_field_of_type)
        << PD->getDeclName();

      // We have an error, now let's go back up through history and show where
      // the offending field came from
      for (ArrayRef<const FieldDecl *>::const_iterator
               I = HistoryStack.begin() + 1,
               E = HistoryStack.end();
           I != E; ++I) {
        const FieldDecl *OuterField = *I;
        S.Diag(OuterField->getLocation(), diag::note_within_field_of_type)
          << OuterField->getType();
      }

      S.Diag(FD->getLocation(), diag::note_illegal_field_declared_here)
        << QT->isPointerType()
        << QT;
      D.setInvalidType();
      return;
    }
  } while (!VisitStack.empty());
}

NamedDecl*
Sema::ActOnFunctionDeclarator(Scope *S, Declarator &D, DeclContext *DC,
                              TypeSourceInfo *TInfo, LookupResult &Previous,
                              MultiTemplateParamsArg TemplateParamLists,
                              bool &AddToScope) {
  QualType R = TInfo->getType();

  assert(R.getTypePtr()->isFunctionType());

  // TODO: consider using NameInfo for diagnostic.
  DeclarationNameInfo NameInfo = GetNameForDeclarator(D);
  DeclarationName Name = NameInfo.getName();
  StorageClass SC = getFunctionStorageClass(*this, D);

  // HLSL Change - reject 'extern' functions
  if (getLangOpts().HLSL && SC == SC_Extern) {
    Diag(D.getLocStart(), diag::err_hlsl_varmodifierna) << "'extern'" << "function";
  }

  if (DeclSpec::TSCS TSCS = D.getDeclSpec().getThreadStorageClassSpec())
    Diag(D.getDeclSpec().getThreadStorageClassSpecLoc(),
         diag::err_invalid_thread)
      << DeclSpec::getSpecifierName(TSCS);

  if (D.isFirstDeclarationOfMember())
    adjustMemberFunctionCC(R, D.isStaticMember());

  bool isFriend = false;
  FunctionTemplateDecl *FunctionTemplate = nullptr;
  bool isExplicitSpecialization = false;
  bool isFunctionTemplateSpecialization = false;

  bool isDependentClassScopeExplicitSpecialization = false;
  bool HasExplicitTemplateArgs = false;
  TemplateArgumentListInfo TemplateArgs;

  bool isVirtualOkay = false;

  DeclContext *OriginalDC = DC;
  bool IsLocalExternDecl = adjustContextForLocalExternDecl(DC);

  FunctionDecl *NewFD = CreateNewFunctionDecl(*this, D, DC, R, TInfo, SC,
                                              isVirtualOkay);
  if (!NewFD) return nullptr;

  if (OriginalLexicalContext && OriginalLexicalContext->isObjCContainer())
    NewFD->setTopLevelDeclInObjCContainer();

  // Set the lexical context. If this is a function-scope declaration, or has a
  // C++ scope specifier, or is the object of a friend declaration, the lexical
  // context will be different from the semantic context.
  NewFD->setLexicalDeclContext(CurContext);

  if (IsLocalExternDecl)
    NewFD->setLocalExternDecl();

  if (getLangOpts().CPlusPlus) {
    bool isInline = D.getDeclSpec().isInlineSpecified();
    bool isVirtual = D.getDeclSpec().isVirtualSpecified();
    bool isExplicit = D.getDeclSpec().isExplicitSpecified();
    bool isConstexpr = D.getDeclSpec().isConstexprSpecified();
    isFriend = D.getDeclSpec().isFriendSpecified();
    if (isFriend && !isInline && D.isFunctionDefinition()) {
      // C++ [class.friend]p5
      //   A function can be defined in a friend declaration of a
      //   class . . . . Such a function is implicitly inline.
      NewFD->setImplicitlyInline();
    }

    // If this is a method defined in an __interface, and is not a constructor
    // or an overloaded operator, then set the pure flag (isVirtual will already
    // return true).
    if (const CXXRecordDecl *Parent =
          dyn_cast<CXXRecordDecl>(NewFD->getDeclContext())) {
      if (Parent->isInterface() && cast<CXXMethodDecl>(NewFD)->isUserProvided())
        NewFD->setPure(true);

      // C++ [class.union]p2
      //   A union can have member functions, but not virtual functions.
      if (isVirtual && Parent->isUnion())
        Diag(D.getDeclSpec().getVirtualSpecLoc(), diag::err_virtual_in_union);
    }

    SetNestedNameSpecifier(NewFD, D);
    isExplicitSpecialization = false;
    isFunctionTemplateSpecialization = false;
    if (D.isInvalidType())
      NewFD->setInvalidDecl();

    // Match up the template parameter lists with the scope specifier, then
    // determine whether we have a template or a template specialization.
    bool Invalid = false;
    if (TemplateParameterList *TemplateParams =
            MatchTemplateParametersToScopeSpecifier(
                D.getDeclSpec().getLocStart(), D.getIdentifierLoc(),
                D.getCXXScopeSpec(),
                D.getName().getKind() == UnqualifiedId::IK_TemplateId
                    ? D.getName().TemplateId
                    : nullptr,
                TemplateParamLists, isFriend, isExplicitSpecialization,
                Invalid)) {
      if (TemplateParams->size() > 0) {
        // This is a function template

        // Check that we can declare a template here.
        if (CheckTemplateDeclScope(S, TemplateParams))
          NewFD->setInvalidDecl();

        // A destructor cannot be a template.
        if (Name.getNameKind() == DeclarationName::CXXDestructorName) {
          Diag(NewFD->getLocation(), diag::err_destructor_template);
          NewFD->setInvalidDecl();
        }
        
        // If we're adding a template to a dependent context, we may need to 
        // rebuilding some of the types used within the template parameter list,
        // now that we know what the current instantiation is.
        if (DC->isDependentContext()) {
          ContextRAII SavedContext(*this, DC);
          if (RebuildTemplateParamsInCurrentInstantiation(TemplateParams))
            Invalid = true;
        }
        

        FunctionTemplate = FunctionTemplateDecl::Create(Context, DC,
                                                        NewFD->getLocation(),
                                                        Name, TemplateParams,
                                                        NewFD);
        FunctionTemplate->setLexicalDeclContext(CurContext);
        NewFD->setDescribedFunctionTemplate(FunctionTemplate);

        // For source fidelity, store the other template param lists.
        if (TemplateParamLists.size() > 1) {
          NewFD->setTemplateParameterListsInfo(Context,
                                               TemplateParamLists.size() - 1,
                                               TemplateParamLists.data());
        }
      } else {
        // This is a function template specialization.
        isFunctionTemplateSpecialization = true;
        // For source fidelity, store all the template param lists.
        if (TemplateParamLists.size() > 0)
          NewFD->setTemplateParameterListsInfo(Context,
                                               TemplateParamLists.size(),
                                               TemplateParamLists.data());

        // C++0x [temp.expl.spec]p20 forbids "template<> friend void foo(int);".
        if (isFriend) {
          // We want to remove the "template<>", found here.
          SourceRange RemoveRange = TemplateParams->getSourceRange();

          // If we remove the template<> and the name is not a
          // template-id, we're actually silently creating a problem:
          // the friend declaration will refer to an untemplated decl,
          // and clearly the user wants a template specialization.  So
          // we need to insert '<>' after the name.
          SourceLocation InsertLoc;
          if (D.getName().getKind() != UnqualifiedId::IK_TemplateId) {
            InsertLoc = D.getName().getSourceRange().getEnd();
            InsertLoc = getLocForEndOfToken(InsertLoc);
          }

          Diag(D.getIdentifierLoc(), diag::err_template_spec_decl_friend)
            << Name << RemoveRange
            << FixItHint::CreateRemoval(RemoveRange)
            << FixItHint::CreateInsertion(InsertLoc, "<>");
        }
      }
    }
    else {
      // All template param lists were matched against the scope specifier:
      // this is NOT (an explicit specialization of) a template.
      if (TemplateParamLists.size() > 0)
        // For source fidelity, store all the template param lists.
        NewFD->setTemplateParameterListsInfo(Context,
                                             TemplateParamLists.size(),
                                             TemplateParamLists.data());
    }

    if (Invalid) {
      NewFD->setInvalidDecl();
      if (FunctionTemplate)
        FunctionTemplate->setInvalidDecl();
    }

    // C++ [dcl.fct.spec]p5:
    //   The virtual specifier shall only be used in declarations of
    //   nonstatic class member functions that appear within a
    //   member-specification of a class declaration; see 10.3.
    //
    if (isVirtual && !NewFD->isInvalidDecl()) {
      if (!isVirtualOkay) {
        Diag(D.getDeclSpec().getVirtualSpecLoc(),
             diag::err_virtual_non_function);
      } else if (!CurContext->isRecord()) {
        // 'virtual' was specified outside of the class.
        Diag(D.getDeclSpec().getVirtualSpecLoc(), 
             diag::err_virtual_out_of_class)
          << FixItHint::CreateRemoval(D.getDeclSpec().getVirtualSpecLoc());
      } else if (NewFD->getDescribedFunctionTemplate()) {
        // C++ [temp.mem]p3:
        //  A member function template shall not be virtual.
        Diag(D.getDeclSpec().getVirtualSpecLoc(),
             diag::err_virtual_member_function_template)
          << FixItHint::CreateRemoval(D.getDeclSpec().getVirtualSpecLoc());
      } else {
        // Okay: Add virtual to the method.
        NewFD->setVirtualAsWritten(true);
      }

      if (getLangOpts().CPlusPlus14 &&
          NewFD->getReturnType()->isUndeducedType())
        Diag(D.getDeclSpec().getVirtualSpecLoc(), diag::err_auto_fn_virtual);
    }

    if (getLangOpts().CPlusPlus14 &&
        (NewFD->isDependentContext() ||
         (isFriend && CurContext->isDependentContext())) &&
        NewFD->getReturnType()->isUndeducedType()) {
      // If the function template is referenced directly (for instance, as a
      // member of the current instantiation), pretend it has a dependent type.
      // This is not really justified by the standard, but is the only sane
      // thing to do.
      // FIXME: For a friend function, we have not marked the function as being
      // a friend yet, so 'isDependentContext' on the FD doesn't work.
      const FunctionProtoType *FPT =
          NewFD->getType()->castAs<FunctionProtoType>();
      QualType Result =
          SubstAutoType(FPT->getReturnType(), Context.DependentTy);
      NewFD->setType(Context.getFunctionType(Result, FPT->getParamTypes(),
                                             FPT->getExtProtoInfo(), FPT->getParamMods())); // HLSL Change - add param mods
    }

    // C++ [dcl.fct.spec]p3:
    //  The inline specifier shall not appear on a block scope function 
    //  declaration.
    if (isInline && !NewFD->isInvalidDecl()) {
      if (CurContext->isFunctionOrMethod()) {
        // 'inline' is not allowed on block scope function declaration.
        Diag(D.getDeclSpec().getInlineSpecLoc(), 
             diag::err_inline_declaration_block_scope) << Name
          << FixItHint::CreateRemoval(D.getDeclSpec().getInlineSpecLoc());
      }
    }

    // C++ [dcl.fct.spec]p6:
    //  The explicit specifier shall be used only in the declaration of a
    //  constructor or conversion function within its class definition; 
    //  see 12.3.1 and 12.3.2.
    if (isExplicit && !NewFD->isInvalidDecl()) {
      if (!CurContext->isRecord()) {
        // 'explicit' was specified outside of the class.
        Diag(D.getDeclSpec().getExplicitSpecLoc(), 
             diag::err_explicit_out_of_class)
          << FixItHint::CreateRemoval(D.getDeclSpec().getExplicitSpecLoc());
      } else if (!isa<CXXConstructorDecl>(NewFD) && 
                 !isa<CXXConversionDecl>(NewFD)) {
        // 'explicit' was specified on a function that wasn't a constructor
        // or conversion function.
        Diag(D.getDeclSpec().getExplicitSpecLoc(),
             diag::err_explicit_non_ctor_or_conv_function)
          << FixItHint::CreateRemoval(D.getDeclSpec().getExplicitSpecLoc());
      }      
    }

    if (isConstexpr) {
      // C++11 [dcl.constexpr]p2: constexpr functions and constexpr constructors
      // are implicitly inline.
      NewFD->setImplicitlyInline();

      // C++11 [dcl.constexpr]p3: functions declared constexpr are required to
      // be either constructors or to return a literal type. Therefore,
      // destructors cannot be declared constexpr.
      if (isa<CXXDestructorDecl>(NewFD))
        Diag(D.getDeclSpec().getConstexprSpecLoc(), diag::err_constexpr_dtor);
    }

    // If __module_private__ was specified, mark the function accordingly.
    if (D.getDeclSpec().isModulePrivateSpecified()) {
      if (isFunctionTemplateSpecialization) {
        SourceLocation ModulePrivateLoc
          = D.getDeclSpec().getModulePrivateSpecLoc();
        Diag(ModulePrivateLoc, diag::err_module_private_specialization)
          << 0
          << FixItHint::CreateRemoval(ModulePrivateLoc);
      } else {
        NewFD->setModulePrivate();
        if (FunctionTemplate)
          FunctionTemplate->setModulePrivate();
      }
    }

    if (isFriend) {
      if (FunctionTemplate) {
        FunctionTemplate->setObjectOfFriendDecl();
        FunctionTemplate->setAccess(AS_public);
      }
      NewFD->setObjectOfFriendDecl();
      NewFD->setAccess(AS_public);
    }

    // If a function is defined as defaulted or deleted, mark it as such now.
    // FIXME: Does this ever happen? ActOnStartOfFunctionDef forces the function
    // definition kind to FDK_Definition.
    switch (D.getFunctionDefinitionKind()) {
      case FDK_Declaration:
      case FDK_Definition:
        break;
        
      case FDK_Defaulted:
        NewFD->setDefaulted();
        break;
        
      case FDK_Deleted:
        NewFD->setDeletedAsWritten();
        break;
    }

    if (isa<CXXMethodDecl>(NewFD) && DC == CurContext &&
        D.isFunctionDefinition()) {
      // C++ [class.mfct]p2:
      //   A member function may be defined (8.4) in its class definition, in 
      //   which case it is an inline member function (7.1.2)
      NewFD->setImplicitlyInline();
    }

    // HLSL Change: We had to set SC_Static for static data member to
    // distinguish it from global constant variable.
    if (SC == SC_Static && isa<CXXMethodDecl>(NewFD) &&
        !CurContext->isRecord() && !getLangOpts().HLSL) {
      // C++ [class.static]p1:
      //   A data or function member of a class may be declared static
      //   in a class definition, in which case it is a static member of
      //   the class.

      // Complain about the 'static' specifier if it's on an out-of-line
      // member function definition.
      Diag(D.getDeclSpec().getStorageClassSpecLoc(),
           diag::err_static_out_of_line)
        << FixItHint::CreateRemoval(D.getDeclSpec().getStorageClassSpecLoc());
    }

    // C++11 [except.spec]p15:
    //   A deallocation function with no exception-specification is treated
    //   as if it were specified with noexcept(true).
    const FunctionProtoType *FPT = R->getAs<FunctionProtoType>();
    if ((Name.getCXXOverloadedOperator() == OO_Delete ||
         Name.getCXXOverloadedOperator() == OO_Array_Delete) &&
        getLangOpts().CPlusPlus11 && FPT && !FPT->hasExceptionSpec())
      NewFD->setType(Context.getFunctionType(
          FPT->getReturnType(), FPT->getParamTypes(),
          FPT->getExtProtoInfo().withExceptionSpec(EST_BasicNoexcept),
          FPT->getParamMods())); // HLSL Change - get param mods
  }

  // Filter out previous declarations that don't match the scope.
  FilterLookupForScope(Previous, OriginalDC, S, shouldConsiderLinkage(NewFD),
                       D.getCXXScopeSpec().isNotEmpty() ||
                       isExplicitSpecialization ||
                       isFunctionTemplateSpecialization);

  // Handle GNU asm-label extension (encoded as an attribute).
  if (Expr *E = (Expr*) D.getAsmLabel()) {
    // The parser guarantees this is a string.
    StringLiteral *SE = cast<StringLiteral>(E);
    NewFD->addAttr(::new (Context) AsmLabelAttr(SE->getStrTokenLoc(0), Context,
                                                SE->getString(), 0));
  } else if (!ExtnameUndeclaredIdentifiers.empty() &&
             isDeclTUScopedExternallyVisible(NewFD)) {
    llvm::DenseMap<IdentifierInfo*,AsmLabelAttr*>::iterator I =
      ExtnameUndeclaredIdentifiers.find(NewFD->getIdentifier());
    if (I != ExtnameUndeclaredIdentifiers.end()) {
      NewFD->addAttr(I->second);
      ExtnameUndeclaredIdentifiers.erase(I);
    }
  }

  // Copy the parameter declarations from the declarator D to the function
  // declaration NewFD, if they are available.  First scavenge them into Params.
  SmallVector<ParmVarDecl*, 16> Params;
  if (D.isFunctionDeclarator()) {
    DeclaratorChunk::FunctionTypeInfo &FTI = D.getFunctionTypeInfo();

    // Check for C99 6.7.5.3p10 - foo(void) is a non-varargs
    // function that takes no arguments, not a function that takes a
    // single void argument.
    // We let through "const void" here because Sema::GetTypeForDeclarator
    // already checks for that case.
    if (FTIHasNonVoidParameters(FTI) && FTI.Params[0].Param) {
      for (unsigned i = 0, e = FTI.NumParams; i != e; ++i) {
        ParmVarDecl *Param = cast<ParmVarDecl>(FTI.Params[i].Param);
        assert(Param->getDeclContext() != NewFD && "Was set before ?");
        Param->setDeclContext(NewFD);
        Params.push_back(Param);

        if (Param->isInvalidDecl())
          NewFD->setInvalidDecl();
      }
    }
  } else if (const FunctionProtoType *FT = R->getAs<FunctionProtoType>()) {
    // When we're declaring a function with a typedef, typeof, etc as in the
    // following example, we'll need to synthesize (unnamed)
    // parameters for use in the declaration.
    //
    // @code
    // typedef void fn(int);
    // fn f;
    // @endcode

    // Synthesize a parameter for each argument type.
    for (const auto &AI : FT->param_types()) {
      ParmVarDecl *Param =
          BuildParmVarDeclForTypedef(NewFD, D.getIdentifierLoc(), AI);
      Param->setScopeInfo(0, Params.size());
      Params.push_back(Param);
    }
  } else {
    assert(R->isFunctionNoProtoType() && NewFD->getNumParams() == 0 &&
           "Should not need args for typedef of non-prototype fn");
  }

  // Finally, we know we have the right number of parameters, install them.
  NewFD->setParams(Params);

  // Find all anonymous symbols defined during the declaration of this function
  // and add to NewFD. This lets us track decls such 'enum Y' in:
  //
  //   void f(enum Y {AA} x) {}
  //
  // which would otherwise incorrectly end up in the translation unit scope.
  NewFD->setDeclsInPrototypeScope(DeclsInPrototypeScope);
  DeclsInPrototypeScope.clear();

  if (D.getDeclSpec().isNoreturnSpecified())
    NewFD->addAttr(
        ::new(Context) C11NoReturnAttr(D.getDeclSpec().getNoreturnSpecLoc(),
                                       Context, 0));

  // Functions returning a variably modified type violate C99 6.7.5.2p2
  // because all functions have linkage.
  if (!NewFD->isInvalidDecl() &&
      NewFD->getReturnType()->isVariablyModifiedType()) {
    Diag(NewFD->getLocation(), diag::err_vm_func_decl);
    NewFD->setInvalidDecl();
  }

  // Apply an implicit SectionAttr if #pragma code_seg is active.
  if (CodeSegStack.CurrentValue && D.isFunctionDefinition() &&
      !NewFD->hasAttr<SectionAttr>()) {
    NewFD->addAttr(
        SectionAttr::CreateImplicit(Context, SectionAttr::Declspec_allocate,
                                    CodeSegStack.CurrentValue->getString(),
                                    CodeSegStack.CurrentPragmaLocation));
    if (UnifySection(CodeSegStack.CurrentValue->getString(),
                     ASTContext::PSF_Implicit | ASTContext::PSF_Execute |
                         ASTContext::PSF_Read,
                     NewFD))
      NewFD->dropAttr<SectionAttr>();
  }

  // Handle attributes.
  ProcessDeclAttributes(S, NewFD, D);

  if (getLangOpts().OpenCL) {
    // OpenCL v1.1 s6.5: Using an address space qualifier in a function return
    // type declaration will generate a compilation error.
    unsigned AddressSpace = NewFD->getReturnType().getAddressSpace();
    if (AddressSpace == LangAS::opencl_local ||
        AddressSpace == LangAS::opencl_global ||
        AddressSpace == LangAS::opencl_constant) {
      Diag(NewFD->getLocation(),
           diag::err_opencl_return_value_with_address_space);
      NewFD->setInvalidDecl();
    }
  }

  if (!getLangOpts().CPlusPlus) {
    // Perform semantic checking on the function declaration.
    bool isExplicitSpecialization=false;
    if (!NewFD->isInvalidDecl() && NewFD->isMain())
      CheckMain(NewFD, D.getDeclSpec());

    if (!NewFD->isInvalidDecl() && NewFD->isMSVCRTEntryPoint())
      CheckMSVCRTEntryPoint(NewFD);

    if (!NewFD->isInvalidDecl())
      D.setRedeclaration(CheckFunctionDeclaration(S, NewFD, Previous,
                                                  isExplicitSpecialization));
    else if (!Previous.empty())
      // Recover gracefully from an invalid redeclaration.
      D.setRedeclaration(true);
    assert((NewFD->isInvalidDecl() || !D.isRedeclaration() ||
            Previous.getResultKind() != LookupResult::FoundOverloaded) &&
           "previous declaration set still overloaded");

    // Diagnose no-prototype function declarations with calling conventions that
    // don't support variadic calls. Only do this in C and do it after merging
    // possibly prototyped redeclarations.
    const FunctionType *FT = NewFD->getType()->castAs<FunctionType>();
    if (isa<FunctionNoProtoType>(FT) && !D.isFunctionDefinition()) {
      CallingConv CC = FT->getExtInfo().getCC();
      if (!supportsVariadicCall(CC)) {
        // Windows system headers sometimes accidentally use stdcall without
        // (void) parameters, so we relax this to a warning.
        int DiagID =
            CC == CC_X86StdCall ? diag::warn_cconv_knr : diag::err_cconv_knr;
        Diag(NewFD->getLocation(), DiagID)
            << FunctionType::getNameForCallConv(CC);
      }
    }
  } else {
    // C++11 [replacement.functions]p3:
    //  The program's definitions shall not be specified as inline.
    //
    // N.B. We diagnose declarations instead of definitions per LWG issue 2340.
    //
    // Suppress the diagnostic if the function is __attribute__((used)), since
    // that forces an external definition to be emitted.
    if (D.getDeclSpec().isInlineSpecified() &&
        NewFD->isReplaceableGlobalAllocationFunction() &&
        !NewFD->hasAttr<UsedAttr>())
      Diag(D.getDeclSpec().getInlineSpecLoc(),
           diag::ext_operator_new_delete_declared_inline)
        << NewFD->getDeclName();

    // If the declarator is a template-id, translate the parser's template 
    // argument list into our AST format.
    if (D.getName().getKind() == UnqualifiedId::IK_TemplateId) {
      TemplateIdAnnotation *TemplateId = D.getName().TemplateId;
      TemplateArgs.setLAngleLoc(TemplateId->LAngleLoc);
      TemplateArgs.setRAngleLoc(TemplateId->RAngleLoc);
      ASTTemplateArgsPtr TemplateArgsPtr(TemplateId->getTemplateArgs(),
                                         TemplateId->NumArgs);
      translateTemplateArguments(TemplateArgsPtr,
                                 TemplateArgs);
    
      HasExplicitTemplateArgs = true;
    
      if (NewFD->isInvalidDecl()) {
        HasExplicitTemplateArgs = false;
      } else if (FunctionTemplate) {
        // Function template with explicit template arguments.
        Diag(D.getIdentifierLoc(), diag::err_function_template_partial_spec)
          << SourceRange(TemplateId->LAngleLoc, TemplateId->RAngleLoc);

        HasExplicitTemplateArgs = false;
      } else {
        assert((isFunctionTemplateSpecialization ||
                D.getDeclSpec().isFriendSpecified()) &&
               "should have a 'template<>' for this decl");
        // "friend void foo<>(int);" is an implicit specialization decl.
        isFunctionTemplateSpecialization = true;
      }
    } else if (isFriend && isFunctionTemplateSpecialization) {
      // This combination is only possible in a recovery case;  the user
      // wrote something like:
      //   template <> friend void foo(int);
      // which we're recovering from as if the user had written:
      //   friend void foo<>(int);
      // Go ahead and fake up a template id.
      HasExplicitTemplateArgs = true;
      TemplateArgs.setLAngleLoc(D.getIdentifierLoc());
      TemplateArgs.setRAngleLoc(D.getIdentifierLoc());
    }

    // If it's a friend (and only if it's a friend), it's possible
    // that either the specialized function type or the specialized
    // template is dependent, and therefore matching will fail.  In
    // this case, don't check the specialization yet.
    bool InstantiationDependent = false;
    if (isFunctionTemplateSpecialization && isFriend &&
        (NewFD->getType()->isDependentType() || DC->isDependentContext() ||
         TemplateSpecializationType::anyDependentTemplateArguments(
            TemplateArgs.getArgumentArray(), TemplateArgs.size(),
            InstantiationDependent))) {
      assert(HasExplicitTemplateArgs &&
             "friend function specialization without template args");
      if (CheckDependentFunctionTemplateSpecialization(NewFD, TemplateArgs,
                                                       Previous))
        NewFD->setInvalidDecl();
    } else if (isFunctionTemplateSpecialization) {
      if (CurContext->isDependentContext() && CurContext->isRecord() 
          && !isFriend) {
        isDependentClassScopeExplicitSpecialization = true;
        Diag(NewFD->getLocation(), getLangOpts().MicrosoftExt ? 
          diag::ext_function_specialization_in_class :
          diag::err_function_specialization_in_class)
          << NewFD->getDeclName();
      } else if (CheckFunctionTemplateSpecialization(NewFD,
                                  (HasExplicitTemplateArgs ? &TemplateArgs
                                                           : nullptr),
                                                     Previous))
        NewFD->setInvalidDecl();
      
      // C++ [dcl.stc]p1:
      //   A storage-class-specifier shall not be specified in an explicit
      //   specialization (14.7.3)
      FunctionTemplateSpecializationInfo *Info =
          NewFD->getTemplateSpecializationInfo();
      if (Info && SC != SC_None) {
        if (SC != Info->getTemplate()->getTemplatedDecl()->getStorageClass())
          Diag(NewFD->getLocation(),
               diag::err_explicit_specialization_inconsistent_storage_class)
            << SC
            << FixItHint::CreateRemoval(
                                      D.getDeclSpec().getStorageClassSpecLoc());
            
        else
          Diag(NewFD->getLocation(), 
               diag::ext_explicit_specialization_storage_class)
            << FixItHint::CreateRemoval(
                                      D.getDeclSpec().getStorageClassSpecLoc());
      }
      
    } else if (isExplicitSpecialization && isa<CXXMethodDecl>(NewFD)) {
      if (CheckMemberSpecialization(NewFD, Previous))
          NewFD->setInvalidDecl();
    }

    // Perform semantic checking on the function declaration.
    if (!isDependentClassScopeExplicitSpecialization) {
      if (!getLangOpts().HLSL && !NewFD->isInvalidDecl() && NewFD->isMain()) // HLSL Change: no main() checks in HLSL
        CheckMain(NewFD, D.getDeclSpec());

      if (!getLangOpts().HLSL && !NewFD->isInvalidDecl() && NewFD->isMSVCRTEntryPoint()) // HLSL Change: no entry point checks in HLSL
        CheckMSVCRTEntryPoint(NewFD);

      if (!NewFD->isInvalidDecl())
        D.setRedeclaration(CheckFunctionDeclaration(S, NewFD, Previous,
                                                    isExplicitSpecialization));
      else if (!Previous.empty())
        // Recover gracefully from an invalid redeclaration.
        D.setRedeclaration(true);
    }

    assert((NewFD->isInvalidDecl() || !D.isRedeclaration() ||
            Previous.getResultKind() != LookupResult::FoundOverloaded) &&
           "previous declaration set still overloaded");

    NamedDecl *PrincipalDecl = (FunctionTemplate
                                ? cast<NamedDecl>(FunctionTemplate)
                                : NewFD);

    if (isFriend && D.isRedeclaration()) {
      AccessSpecifier Access = AS_public;
      if (!NewFD->isInvalidDecl())
        Access = NewFD->getPreviousDecl()->getAccess();

      NewFD->setAccess(Access);
      if (FunctionTemplate) FunctionTemplate->setAccess(Access);
    }

    if (NewFD->isOverloadedOperator() && !DC->isRecord() &&
        PrincipalDecl->isInIdentifierNamespace(Decl::IDNS_Ordinary))
      PrincipalDecl->setNonMemberOperator();

    // If we have a function template, check the template parameter
    // list. This will check and merge default template arguments.
    if (FunctionTemplate) {
      FunctionTemplateDecl *PrevTemplate = 
                                     FunctionTemplate->getPreviousDecl();
      CheckTemplateParameterList(FunctionTemplate->getTemplateParameters(),
                       PrevTemplate ? PrevTemplate->getTemplateParameters()
                                    : nullptr,
                            D.getDeclSpec().isFriendSpecified()
                              ? (D.isFunctionDefinition()
                                   ? TPC_FriendFunctionTemplateDefinition
                                   : TPC_FriendFunctionTemplate)
                              : (D.getCXXScopeSpec().isSet() && 
                                 DC && DC->isRecord() && 
                                 DC->isDependentContext())
                                  ? TPC_ClassTemplateMember
                                  : TPC_FunctionTemplate);
    }

    if (NewFD->isInvalidDecl()) {
      // Ignore all the rest of this.
    } else if (!D.isRedeclaration()) {
      struct ActOnFDArgs ExtraArgs = { S, D, TemplateParamLists,
                                       AddToScope };
      // Fake up an access specifier if it's supposed to be a class member.
      if (isa<CXXRecordDecl>(NewFD->getDeclContext()))
        NewFD->setAccess(AS_public);

      // Qualified decls generally require a previous declaration.
      if (D.getCXXScopeSpec().isSet()) {
        // ...with the major exception of templated-scope or
        // dependent-scope friend declarations.

        // TODO: we currently also suppress this check in dependent
        // contexts because (1) the parameter depth will be off when
        // matching friend templates and (2) we might actually be
        // selecting a friend based on a dependent factor.  But there
        // are situations where these conditions don't apply and we
        // can actually do this check immediately.
        if (isFriend &&
            (TemplateParamLists.size() ||
             D.getCXXScopeSpec().getScopeRep()->isDependent() ||
             CurContext->isDependentContext())) {
          // ignore these
        } else {
          // The user tried to provide an out-of-line definition for a
          // function that is a member of a class or namespace, but there
          // was no such member function declared (C++ [class.mfct]p2,
          // C++ [namespace.memdef]p2). For example:
          //
          // class X {
          //   void f() const;
          // };
          //
          // void X::f() { } // ill-formed
          //
          // Complain about this problem, and attempt to suggest close
          // matches (e.g., those that differ only in cv-qualifiers and
          // whether the parameter types are references).

          if (NamedDecl *Result = DiagnoseInvalidRedeclaration(
                  *this, Previous, NewFD, ExtraArgs, false, nullptr)) {
            AddToScope = ExtraArgs.AddToScope;
            return Result;
          }
        }

        // Unqualified local friend declarations are required to resolve
        // to something.
      } else if (isFriend && cast<CXXRecordDecl>(CurContext)->isLocalClass()) {
        if (NamedDecl *Result = DiagnoseInvalidRedeclaration(
                *this, Previous, NewFD, ExtraArgs, true, S)) {
          AddToScope = ExtraArgs.AddToScope;
          return Result;
        }
      }

    } else if (!D.isFunctionDefinition() &&
               isa<CXXMethodDecl>(NewFD) && NewFD->isOutOfLine() &&
               !isFriend && !isFunctionTemplateSpecialization &&
               !isExplicitSpecialization) {
      // An out-of-line member function declaration must also be a
      // definition (C++ [class.mfct]p2).
      // Note that this is not the case for explicit specializations of
      // function templates or member functions of class templates, per
      // C++ [temp.expl.spec]p2. We also allow these declarations as an 
      // extension for compatibility with old SWIG code which likes to 
      // generate them.
      Diag(NewFD->getLocation(), diag::ext_out_of_line_declaration)
        << D.getCXXScopeSpec().getRange();
    }
  }

  ProcessPragmaWeak(S, NewFD);
  checkAttributesAfterMerging(*this, *NewFD);

  AddKnownFunctionAttributes(NewFD);

  if (NewFD->hasAttr<OverloadableAttr>() && 
      !NewFD->getType()->getAs<FunctionProtoType>()) {
    Diag(NewFD->getLocation(),
         diag::err_attribute_overloadable_no_prototype)
      << NewFD;

    // Turn this into a variadic function with no parameters.
    const FunctionType *FT = NewFD->getType()->getAs<FunctionType>();
    FunctionProtoType::ExtProtoInfo EPI(
        Context.getDefaultCallingConvention(true, false));
    EPI.Variadic = true;
    EPI.ExtInfo = FT->getExtInfo();

    QualType R = Context.getFunctionType(FT->getReturnType(), None, EPI, None); // HLSL Change - all defaults are in
    NewFD->setType(R);
  }

  // If there's a #pragma GCC visibility in scope, and this isn't a class
  // member, set the visibility of this function.
  if (!DC->isRecord() && NewFD->isExternallyVisible())
    AddPushedVisibilityAttribute(NewFD);

  // If there's a #pragma clang arc_cf_code_audited in scope, consider
  // marking the function.
  AddCFAuditedAttribute(NewFD);

  // If this is a function definition, check if we have to apply optnone due to
  // a pragma.
  if(D.isFunctionDefinition())
    AddRangeBasedOptnone(NewFD);

  // If this is the first declaration of an extern C variable, update
  // the map of such variables.
  if (NewFD->isFirstDecl() && !NewFD->isInvalidDecl() &&
      isIncompleteDeclExternC(*this, NewFD))
    RegisterLocallyScopedExternCDecl(NewFD, S);

  // Set this FunctionDecl's range up to the right paren.
  NewFD->setRangeEnd(D.getSourceRange().getEnd());

  if (D.isRedeclaration() && !Previous.empty()) {
    checkDLLAttributeRedeclaration(
        *this, dyn_cast<NamedDecl>(Previous.getRepresentativeDecl()), NewFD,
        isExplicitSpecialization || isFunctionTemplateSpecialization);
  }

  if (getLangOpts().CPlusPlus) {
    if (FunctionTemplate) {
      if (NewFD->isInvalidDecl())
        FunctionTemplate->setInvalidDecl();
      return FunctionTemplate;
    }
  }

  if (NewFD->hasAttr<OpenCLKernelAttr>()) {
    // OpenCL v1.2 s6.8 static is invalid for kernel functions.
    if ((getLangOpts().OpenCLVersion >= 120)
        && (SC == SC_Static)) {
      Diag(D.getIdentifierLoc(), diag::err_static_kernel);
      D.setInvalidType();
    }
    
    // OpenCL v1.2, s6.9 -- Kernels can only have return type void.
    if (!NewFD->getReturnType()->isVoidType()) {
      SourceRange RTRange = NewFD->getReturnTypeSourceRange();
      Diag(D.getIdentifierLoc(), diag::err_expected_kernel_void_return_type)
          << (RTRange.isValid() ? FixItHint::CreateReplacement(RTRange, "void")
                                : FixItHint());
      D.setInvalidType();
    }

    llvm::SmallPtrSet<const Type *, 16> ValidTypes;
    for (auto Param : NewFD->params())
      checkIsValidOpenCLKernelParameter(*this, D, Param, ValidTypes);
  }

  MarkUnusedFileScopedDecl(NewFD);

  if (getLangOpts().CUDA)
    if (IdentifierInfo *II = NewFD->getIdentifier())
      if (!NewFD->isInvalidDecl() &&
          NewFD->getDeclContext()->getRedeclContext()->isTranslationUnit()) {
        if (II->isStr("cudaConfigureCall")) {
          if (!R->getAs<FunctionType>()->getReturnType()->isScalarType())
            Diag(NewFD->getLocation(), diag::err_config_scalar_return);

          Context.setcudaConfigureCallDecl(NewFD);
        }
      }
  
  // Here we have an function template explicit specialization at class scope.
  // The actually specialization will be postponed to template instatiation
  // time via the ClassScopeFunctionSpecializationDecl node.
  if (isDependentClassScopeExplicitSpecialization) {
    ClassScopeFunctionSpecializationDecl *NewSpec =
                         ClassScopeFunctionSpecializationDecl::Create(
                                Context, CurContext, SourceLocation(), 
                                cast<CXXMethodDecl>(NewFD),
                                HasExplicitTemplateArgs, TemplateArgs);
    CurContext->addDecl(NewSpec);
    AddToScope = false;
  }

  // HLSL Change Starts
  TransferUnusualAttributes(D, NewFD);

  if (getLangOpts().HLSL && D.isFunctionDefinition() && D.hasName() &&
      NewFD->getDeclContext()->getRedeclContext()->isTranslationUnit()) {
    hlsl::DiagnoseEntry(*this, NewFD);
  }
  // HLSL Change Ends

  return NewFD;
}

/// \brief Perform semantic checking of a new function declaration.
///
/// Performs semantic analysis of the new function declaration
/// NewFD. This routine performs all semantic checking that does not
/// require the actual declarator involved in the declaration, and is
/// used both for the declaration of functions as they are parsed
/// (called via ActOnDeclarator) and for the declaration of functions
/// that have been instantiated via C++ template instantiation (called
/// via InstantiateDecl).
///
/// \param IsExplicitSpecialization whether this new function declaration is
/// an explicit specialization of the previous declaration.
///
/// This sets NewFD->isInvalidDecl() to true if there was an error.
///
/// \returns true if the function declaration is a redeclaration.
bool Sema::CheckFunctionDeclaration(Scope *S, FunctionDecl *NewFD,
                                    LookupResult &Previous,
                                    bool IsExplicitSpecialization) {
  assert(!NewFD->getReturnType()->isVariablyModifiedType() &&
         "Variably modified return types are not handled here");

  // Determine whether the type of this function should be merged with
  // a previous visible declaration. This never happens for functions in C++,
  // and always happens in C if the previous declaration was visible.
  bool MergeTypeWithPrevious = !getLangOpts().CPlusPlus &&
                               !Previous.isShadowed();

  // Filter out any non-conflicting previous declarations.
  filterNonConflictingPreviousDecls(*this, NewFD, Previous);

  bool Redeclaration = false;
  NamedDecl *OldDecl = nullptr;

  // Merge or overload the declaration with an existing declaration of
  // the same name, if appropriate.
  if (!Previous.empty()) {
    // Determine whether NewFD is an overload of PrevDecl or
    // a declaration that requires merging. If it's an overload,
    // there's no more work to do here; we'll just add the new
    // function to the scope.
    if (!AllowOverloadingOfFunction(Previous, Context)) {
      NamedDecl *Candidate = Previous.getFoundDecl();
      if (shouldLinkPossiblyHiddenDecl(Candidate, NewFD)) {
        Redeclaration = true;
        OldDecl = Candidate;
      }
    } else {
      switch (CheckOverload(S, NewFD, Previous, OldDecl,
                            /*NewIsUsingDecl*/ false)) {
      case Ovl_Match:
        Redeclaration = true;
        break;

      case Ovl_NonFunction:
        Redeclaration = true;
        break;

      case Ovl_Overload:
        Redeclaration = false;
        break;
      }

      if (!getLangOpts().CPlusPlus && !NewFD->hasAttr<OverloadableAttr>()) {
        // If a function name is overloadable in C, then every function
        // with that name must be marked "overloadable".
        Diag(NewFD->getLocation(), diag::err_attribute_overloadable_missing)
          << Redeclaration << NewFD;
        NamedDecl *OverloadedDecl = nullptr;
        if (Redeclaration)
          OverloadedDecl = OldDecl;
        else if (!Previous.empty())
          OverloadedDecl = Previous.getRepresentativeDecl();
        if (OverloadedDecl)
          Diag(OverloadedDecl->getLocation(),
               diag::note_attribute_overloadable_prev_overload);
        NewFD->addAttr(OverloadableAttr::CreateImplicit(Context));
      }
    }
  }

  // Check for a previous extern "C" declaration with this name.
  if (!Redeclaration &&
      checkForConflictWithNonVisibleExternC(*this, NewFD, Previous)) {
    filterNonConflictingPreviousDecls(*this, NewFD, Previous);
    if (!Previous.empty()) {
      // This is an extern "C" declaration with the same name as a previous
      // declaration, and thus redeclares that entity...
      Redeclaration = true;
      OldDecl = Previous.getFoundDecl();
      MergeTypeWithPrevious = false;

      // ... except in the presence of __attribute__((overloadable)).
      if (OldDecl->hasAttr<OverloadableAttr>()) {
        if (!getLangOpts().CPlusPlus && !NewFD->hasAttr<OverloadableAttr>()) {
          Diag(NewFD->getLocation(), diag::err_attribute_overloadable_missing)
            << Redeclaration << NewFD;
          Diag(Previous.getFoundDecl()->getLocation(),
               diag::note_attribute_overloadable_prev_overload);
          NewFD->addAttr(OverloadableAttr::CreateImplicit(Context));
        }
        if (IsOverload(NewFD, cast<FunctionDecl>(OldDecl), false)) {
          Redeclaration = false;
          OldDecl = nullptr;
        }
      }
    }
  }

  // HLSL Change Starts
  // Rather than fix for inout parameters, comment out - this is N/A for HLSL
#if 0
  // HLSL Change Ends
  // C++11 [dcl.constexpr]p8:
  //   A constexpr specifier for a non-static member function that is not
  //   a constructor declares that member function to be const.
  //
  // This needs to be delayed until we know whether this is an out-of-line
  // definition of a static member function.
  //
  // This rule is not present in C++1y, so we produce a backwards
  // compatibility warning whenever it happens in C++11.
  CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(NewFD);
  if (!getLangOpts().CPlusPlus14 && MD && MD->isConstexpr() &&
      !MD->isStatic() && !isa<CXXConstructorDecl>(MD) &&
      (MD->getTypeQualifiers() & Qualifiers::Const) == 0) {
    CXXMethodDecl *OldMD = nullptr;
    if (OldDecl)
      OldMD = dyn_cast_or_null<CXXMethodDecl>(OldDecl->getAsFunction());
    if (!OldMD || !OldMD->isStatic()) {
      const FunctionProtoType *FPT =
        MD->getType()->castAs<FunctionProtoType>();
      FunctionProtoType::ExtProtoInfo EPI = FPT->getExtProtoInfo();
      EPI.TypeQuals |= Qualifiers::Const;
      MD->setType(Context.getFunctionType(FPT->getReturnType(),
                                          FPT->getParamTypes(), EPI));

      // Warn that we did this, if we're not performing template instantiation.
      // In that case, we'll have warned already when the template was defined.
      if (ActiveTemplateInstantiations.empty()) {
        SourceLocation AddConstLoc;
        if (FunctionTypeLoc FTL = MD->getTypeSourceInfo()->getTypeLoc()
                .IgnoreParens().getAs<FunctionTypeLoc>())
          AddConstLoc = getLocForEndOfToken(FTL.getRParenLoc());

        Diag(MD->getLocation(), diag::warn_cxx14_compat_constexpr_not_const)
          << FixItHint::CreateInsertion(AddConstLoc, " const");
      }
    }
  }
#endif // HLSL Change Ends

  if (Redeclaration) {
    // NewFD and OldDecl represent declarations that need to be
    // merged.
    if (MergeFunctionDecl(NewFD, OldDecl, S, MergeTypeWithPrevious)) {
      NewFD->setInvalidDecl();
      return Redeclaration;
    }

    Previous.clear();
    Previous.addDecl(OldDecl);

    if (FunctionTemplateDecl *OldTemplateDecl
                                  = dyn_cast<FunctionTemplateDecl>(OldDecl)) {
      NewFD->setPreviousDeclaration(OldTemplateDecl->getTemplatedDecl());
      FunctionTemplateDecl *NewTemplateDecl
        = NewFD->getDescribedFunctionTemplate();
      assert(NewTemplateDecl && "Template/non-template mismatch");
      if (CXXMethodDecl *Method 
            = dyn_cast<CXXMethodDecl>(NewTemplateDecl->getTemplatedDecl())) {
        Method->setAccess(OldTemplateDecl->getAccess());
        NewTemplateDecl->setAccess(OldTemplateDecl->getAccess());
      }
      
      // If this is an explicit specialization of a member that is a function
      // template, mark it as a member specialization.
      if (IsExplicitSpecialization && 
          NewTemplateDecl->getInstantiatedFromMemberTemplate()) {
        NewTemplateDecl->setMemberSpecialization();
        assert(OldTemplateDecl->isMemberSpecialization());
      }
      
    } else {
      // This needs to happen first so that 'inline' propagates.
      NewFD->setPreviousDeclaration(cast<FunctionDecl>(OldDecl));

      if (isa<CXXMethodDecl>(NewFD))
        NewFD->setAccess(OldDecl->getAccess());
    }
  }

  // Semantic checking for this function declaration (in isolation).

  if (getLangOpts().CPlusPlus) {
    // C++-specific checks.
    if (CXXConstructorDecl *Constructor = dyn_cast<CXXConstructorDecl>(NewFD)) {
      CheckConstructor(Constructor);
    } else if (CXXDestructorDecl *Destructor = 
                dyn_cast<CXXDestructorDecl>(NewFD)) {
      CXXRecordDecl *Record = Destructor->getParent();
      QualType ClassType = Context.getTypeDeclType(Record);
      
      // FIXME: Shouldn't we be able to perform this check even when the class
      // type is dependent? Both gcc and edg can handle that.
      if (!ClassType->isDependentType()) {
        DeclarationName Name
          = Context.DeclarationNames.getCXXDestructorName(
                                        Context.getCanonicalType(ClassType));
        if (NewFD->getDeclName() != Name) {
          Diag(NewFD->getLocation(), diag::err_destructor_name);
          NewFD->setInvalidDecl();
          return Redeclaration;
        }
      }
    } else if (CXXConversionDecl *Conversion
               = dyn_cast<CXXConversionDecl>(NewFD)) {
      ActOnConversionDeclarator(Conversion);
    }

    // Find any virtual functions that this function overrides.
    if (CXXMethodDecl *Method = dyn_cast<CXXMethodDecl>(NewFD)) {
      if (!Method->isFunctionTemplateSpecialization() && 
          !Method->getDescribedFunctionTemplate() &&
          Method->isCanonicalDecl()) {
        if (AddOverriddenMethods(Method->getParent(), Method)) {
          // If the function was marked as "static", we have a problem.
          if (NewFD->getStorageClass() == SC_Static) {
            ReportOverrides(*this, diag::err_static_overrides_virtual, Method);
          }
        }
      }
      
      if (Method->isStatic())
        checkThisInStaticMemberFunctionType(Method);
    }

    // Extra checking for C++ overloaded operators (C++ [over.oper]).
    if (NewFD->isOverloadedOperator() &&
        CheckOverloadedOperatorDeclaration(NewFD)) {
      NewFD->setInvalidDecl();
      return Redeclaration;
    }

    // Extra checking for C++0x literal operators (C++0x [over.literal]).
    if (NewFD->getLiteralIdentifier() &&
        CheckLiteralOperatorDeclaration(NewFD)) {
      NewFD->setInvalidDecl();
      return Redeclaration;
    }

    // In C++, check default arguments now that we have merged decls. Unless
    // the lexical context is the class, because in this case this is done
    // during delayed parsing anyway.
    if (!CurContext->isRecord())
      CheckCXXDefaultArguments(NewFD);

    // If this function declares a builtin function, check the type of this
    // declaration against the expected type for the builtin. 
    if (unsigned BuiltinID = NewFD->getBuiltinID()) {
      ASTContext::GetBuiltinTypeError Error;
      LookupPredefedObjCSuperType(*this, S, NewFD->getIdentifier());
      QualType T = Context.GetBuiltinType(BuiltinID, Error);
      if (!T.isNull() && !Context.hasSameType(T, NewFD->getType())) {
        // The type of this function differs from the type of the builtin,
        // so forget about the builtin entirely.
        Context.BuiltinInfo.ForgetBuiltin(BuiltinID, Context.Idents);
      }
    }

    // If this function is declared as being extern "C", then check to see if 
    // the function returns a UDT (class, struct, or union type) that is not C
    // compatible, and if it does, warn the user.
    // But, issue any diagnostic on the first declaration only.
    if (Previous.empty() && NewFD->isExternC()) {
      QualType R = NewFD->getReturnType();
      if (R->isIncompleteType() && !R->isVoidType())
        Diag(NewFD->getLocation(), diag::warn_return_value_udt_incomplete)
            << NewFD << R;
      else if (!R.isPODType(Context) && !R->isVoidType() &&
               !R->isObjCObjectPointerType())
        Diag(NewFD->getLocation(), diag::warn_return_value_udt) << NewFD << R;
    }
  }
  return Redeclaration;
}

void Sema::CheckMain(FunctionDecl* FD, const DeclSpec& DS) {
  // C++11 [basic.start.main]p3:
  //   A program that [...] declares main to be inline, static or
  //   constexpr is ill-formed.
  // C11 6.7.4p4:  In a hosted environment, no function specifier(s) shall
  //   appear in a declaration of main.
  // static main is not an error under C99, but we should warn about it.
  // We accept _Noreturn main as an extension.
  if (FD->getStorageClass() == SC_Static)
    Diag(DS.getStorageClassSpecLoc(), getLangOpts().CPlusPlus 
         ? diag::err_static_main : diag::warn_static_main) 
      << FixItHint::CreateRemoval(DS.getStorageClassSpecLoc());
  if (FD->isInlineSpecified())
    Diag(DS.getInlineSpecLoc(), diag::err_inline_main) 
      << FixItHint::CreateRemoval(DS.getInlineSpecLoc());
  if (DS.isNoreturnSpecified()) {
    SourceLocation NoreturnLoc = DS.getNoreturnSpecLoc();
    SourceRange NoreturnRange(NoreturnLoc, getLocForEndOfToken(NoreturnLoc));
    Diag(NoreturnLoc, diag::ext_noreturn_main);
    Diag(NoreturnLoc, diag::note_main_remove_noreturn)
      << FixItHint::CreateRemoval(NoreturnRange);
  }
  if (FD->isConstexpr()) {
    Diag(DS.getConstexprSpecLoc(), diag::err_constexpr_main)
      << FixItHint::CreateRemoval(DS.getConstexprSpecLoc());
    FD->setConstexpr(false);
  }

  if (getLangOpts().OpenCL) {
    Diag(FD->getLocation(), diag::err_opencl_no_main)
        << FD->hasAttr<OpenCLKernelAttr>();
    FD->setInvalidDecl();
    return;
  }

  QualType T = FD->getType();
  assert(T->isFunctionType() && "function decl is not of function type");
  const FunctionType* FT = T->castAs<FunctionType>();

  if (getLangOpts().GNUMode && !getLangOpts().CPlusPlus) {
    // In C with GNU extensions we allow main() to have non-integer return
    // type, but we should warn about the extension, and we disable the
    // implicit-return-zero rule.

    // GCC in C mode accepts qualified 'int'.
    if (Context.hasSameUnqualifiedType(FT->getReturnType(), Context.IntTy))
      FD->setHasImplicitReturnZero(true);
    else {
      Diag(FD->getTypeSpecStartLoc(), diag::ext_main_returns_nonint);
      SourceRange RTRange = FD->getReturnTypeSourceRange();
      if (RTRange.isValid())
        Diag(RTRange.getBegin(), diag::note_main_change_return_type)
            << FixItHint::CreateReplacement(RTRange, "int");
    }
  } else {
    // In C and C++, main magically returns 0 if you fall off the end;
    // set the flag which tells us that.
    // This is C++ [basic.start.main]p5 and C99 5.1.2.2.3.

    // All the standards say that main() should return 'int'.
    if (Context.hasSameType(FT->getReturnType(), Context.IntTy))
      FD->setHasImplicitReturnZero(true);
    else {
      // Otherwise, this is just a flat-out error.
      SourceRange RTRange = FD->getReturnTypeSourceRange();
      Diag(FD->getTypeSpecStartLoc(), diag::err_main_returns_nonint)
          << (RTRange.isValid() ? FixItHint::CreateReplacement(RTRange, "int")
                                : FixItHint());
      FD->setInvalidDecl(true);
    }
  }

  // Treat protoless main() as nullary.
  if (isa<FunctionNoProtoType>(FT)) return;

  const FunctionProtoType* FTP = cast<const FunctionProtoType>(FT);
  unsigned nparams = FTP->getNumParams();
  assert(FD->getNumParams() == nparams);

  bool HasExtraParameters = (nparams > 3);

  if (FTP->isVariadic()) {
    Diag(FD->getLocation(), diag::ext_variadic_main);
    // FIXME: if we had information about the location of the ellipsis, we
    // could add a FixIt hint to remove it as a parameter.
  }

  // Darwin passes an undocumented fourth argument of type char**.  If
  // other platforms start sprouting these, the logic below will start
  // getting shifty.
  if (nparams == 4 && Context.getTargetInfo().getTriple().isOSDarwin())
    HasExtraParameters = false;

  if (HasExtraParameters) {
    Diag(FD->getLocation(), diag::err_main_surplus_args) << nparams;
    FD->setInvalidDecl(true);
    nparams = 3;
  }

  // FIXME: a lot of the following diagnostics would be improved
  // if we had some location information about types.

  QualType CharPP =
    Context.getPointerType(Context.getPointerType(Context.CharTy));
  QualType Expected[] = { Context.IntTy, CharPP, CharPP, CharPP };

  for (unsigned i = 0; i < nparams; ++i) {
    QualType AT = FTP->getParamType(i);

    bool mismatch = true;

    if (Context.hasSameUnqualifiedType(AT, Expected[i]))
      mismatch = false;
    else if (Expected[i] == CharPP) {
      // As an extension, the following forms are okay:
      //   char const **
      //   char const * const *
      //   char * const *

      QualifierCollector qs;
      const PointerType* PT;
      if ((PT = qs.strip(AT)->getAs<PointerType>()) &&
          (PT = qs.strip(PT->getPointeeType())->getAs<PointerType>()) &&
          Context.hasSameType(QualType(qs.strip(PT->getPointeeType()), 0),
                              Context.CharTy)) {
        qs.removeConst();
        mismatch = !qs.empty();
      }
    }

    if (mismatch) {
      Diag(FD->getLocation(), diag::err_main_arg_wrong) << i << Expected[i];
      // TODO: suggest replacing given type with expected type
      FD->setInvalidDecl(true);
    }
  }

  if (nparams == 1 && !FD->isInvalidDecl()) {
    Diag(FD->getLocation(), diag::warn_main_one_arg);
  }
  
  if (!FD->isInvalidDecl() && FD->getDescribedFunctionTemplate()) {
    Diag(FD->getLocation(), diag::err_mainlike_template_decl) << FD;
    FD->setInvalidDecl();
  }
}

void Sema::CheckMSVCRTEntryPoint(FunctionDecl *FD) {
  QualType T = FD->getType();
  assert(T->isFunctionType() && "function decl is not of function type");
  const FunctionType *FT = T->castAs<FunctionType>();

  // Set an implicit return of 'zero' if the function can return some integral,
  // enumeration, pointer or nullptr type.
  if (FT->getReturnType()->isIntegralOrEnumerationType() ||
      FT->getReturnType()->isAnyPointerType() ||
      FT->getReturnType()->isNullPtrType())
    // DllMain is exempt because a return value of zero means it failed.
    if (FD->getName() != "DllMain")
      FD->setHasImplicitReturnZero(true);

  if (!FD->isInvalidDecl() && FD->getDescribedFunctionTemplate()) {
    Diag(FD->getLocation(), diag::err_mainlike_template_decl) << FD;
    FD->setInvalidDecl();
  }
}

bool Sema::CheckForConstantInitializer(Expr *Init, QualType DclT) {
  // FIXME: Need strict checking.  In C89, we need to check for
  // any assignment, increment, decrement, function-calls, or
  // commas outside of a sizeof.  In C99, it's the same list,
  // except that the aforementioned are allowed in unevaluated
  // expressions.  Everything else falls under the
  // "may accept other forms of constant expressions" exception.
  // (We never end up here for C++, so the constant expression
  // rules there don't matter.)
  const Expr *Culprit;
  if (Init->isConstantInitializer(Context, false, &Culprit))
    return false;
  Diag(Culprit->getExprLoc(), diag::err_init_element_not_constant)
    << Culprit->getSourceRange();
  return true;
}

namespace {
  // Visits an initialization expression to see if OrigDecl is evaluated in
  // its own initialization and throws a warning if it does.
  class SelfReferenceChecker
      : public EvaluatedExprVisitor<SelfReferenceChecker> {
    Sema &S;
    Decl *OrigDecl;
    bool isRecordType;
    bool isPODType;
    bool isReferenceType;

    bool isInitList;
    llvm::SmallVector<unsigned, 4> InitFieldIndex;
  public:
    typedef EvaluatedExprVisitor<SelfReferenceChecker> Inherited;

    SelfReferenceChecker(Sema &S, Decl *OrigDecl) : Inherited(S.Context),
                                                    S(S), OrigDecl(OrigDecl) {
      isPODType = false;
      isRecordType = false;
      isReferenceType = false;
      isInitList = false;
      if (ValueDecl *VD = dyn_cast<ValueDecl>(OrigDecl)) {
        isPODType = VD->getType().isPODType(S.Context);
        isRecordType = VD->getType()->isRecordType();
        isReferenceType = VD->getType()->isReferenceType();
      }
    }

    // For most expressions, just call the visitor.  For initializer lists,
    // track the index of the field being initialized since fields are
    // initialized in order allowing use of previously initialized fields.
    void CheckExpr(Expr *E) {
      InitListExpr *InitList = dyn_cast<InitListExpr>(E);
      if (!InitList) {
        Visit(E);
        return;
      }

      // Track and increment the index here.
      isInitList = true;
      InitFieldIndex.push_back(0);
      for (auto Child : InitList->children()) {
        CheckExpr(cast<Expr>(Child));
        ++InitFieldIndex.back();
      }
      InitFieldIndex.pop_back();
    }

    // Returns true if MemberExpr is checked and no futher checking is needed.
    // Returns false if additional checking is required.
    bool CheckInitListMemberExpr(MemberExpr *E, bool CheckReference) {
      llvm::SmallVector<FieldDecl*, 4> Fields;
      Expr *Base = E;
      bool ReferenceField = false;

      // Get the field memebers used.
      while (MemberExpr *ME = dyn_cast<MemberExpr>(Base)) {
        FieldDecl *FD = dyn_cast<FieldDecl>(ME->getMemberDecl());
        if (!FD)
          return false;
        Fields.push_back(FD);
        if (FD->getType()->isReferenceType())
          ReferenceField = true;
        Base = ME->getBase()->IgnoreParenImpCasts();
      }

      // Keep checking only if the base Decl is the same.
      DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(Base);
      if (!DRE || DRE->getDecl() != OrigDecl)
        return false;

      // A reference field can be bound to an unininitialized field.
      if (CheckReference && !ReferenceField)
        return true;

      // Convert FieldDecls to their index number.
      llvm::SmallVector<unsigned, 4> UsedFieldIndex;
      for (auto I = Fields.rbegin(), E = Fields.rend(); I != E; ++I) {
        UsedFieldIndex.push_back((*I)->getFieldIndex());
      }

      // See if a warning is needed by checking the first difference in index
      // numbers.  If field being used has index less than the field being
      // initialized, then the use is safe.
      for (auto UsedIter = UsedFieldIndex.begin(),
                UsedEnd = UsedFieldIndex.end(),
                OrigIter = InitFieldIndex.begin(),
                OrigEnd = InitFieldIndex.end();
           UsedIter != UsedEnd && OrigIter != OrigEnd; ++UsedIter, ++OrigIter) {
        if (*UsedIter < *OrigIter)
          return true;
        if (*UsedIter > *OrigIter)
          break;
      }

      // TODO: Add a different warning which will print the field names.
      HandleDeclRefExpr(DRE);
      return true;
    }

    // For most expressions, the cast is directly above the DeclRefExpr.
    // For conditional operators, the cast can be outside the conditional
    // operator if both expressions are DeclRefExpr's.
    void HandleValue(Expr *E) {
      E = E->IgnoreParens();
      if (DeclRefExpr* DRE = dyn_cast<DeclRefExpr>(E)) {
        HandleDeclRefExpr(DRE);
        return;
      }

      if (ConditionalOperator *CO = dyn_cast<ConditionalOperator>(E)) {
        Visit(CO->getCond());
        HandleValue(CO->getTrueExpr());
        HandleValue(CO->getFalseExpr());
        return;
      }

      if (BinaryConditionalOperator *BCO =
              dyn_cast<BinaryConditionalOperator>(E)) {
        Visit(BCO->getCond());
        HandleValue(BCO->getFalseExpr());
        return;
      }

      if (OpaqueValueExpr *OVE = dyn_cast<OpaqueValueExpr>(E)) {
        HandleValue(OVE->getSourceExpr());
        return;
      }

      if (BinaryOperator *BO = dyn_cast<BinaryOperator>(E)) {
        if (BO->getOpcode() == BO_Comma) {
          Visit(BO->getLHS());
          HandleValue(BO->getRHS());
          return;
        }
      }

      if (isa<MemberExpr>(E)) {
        if (isInitList) {
          if (CheckInitListMemberExpr(cast<MemberExpr>(E),
                                      false /*CheckReference*/))
            return;
        }

        Expr *Base = E->IgnoreParenImpCasts();
        while (MemberExpr *ME = dyn_cast<MemberExpr>(Base)) {
          // Check for static member variables and don't warn on them.
          if (!isa<FieldDecl>(ME->getMemberDecl()))
            return;
          Base = ME->getBase()->IgnoreParenImpCasts();
        }
        if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(Base))
          HandleDeclRefExpr(DRE);
        return;
      }

      Visit(E);
    }

    // Reference types not handled in HandleValue are handled here since all
    // uses of references are bad, not just r-value uses.
    void VisitDeclRefExpr(DeclRefExpr *E) {
      if (isReferenceType)
        HandleDeclRefExpr(E);
    }

    void VisitImplicitCastExpr(ImplicitCastExpr *E) {
      if (E->getCastKind() == CK_LValueToRValue) {
        HandleValue(E->getSubExpr());
        return;
      }

      Inherited::VisitImplicitCastExpr(E);
    }

    void VisitMemberExpr(MemberExpr *E) {
      if (isInitList) {
        if (CheckInitListMemberExpr(E, true /*CheckReference*/))
          return;
      }

      // Don't warn on arrays since they can be treated as pointers.
      if (E->getType()->canDecayToPointerType()) return;

      // Warn when a non-static method call is followed by non-static member
      // field accesses, which is followed by a DeclRefExpr.
      CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(E->getMemberDecl());
      bool Warn = (MD && !MD->isStatic());
      Expr *Base = E->getBase()->IgnoreParenImpCasts();
      while (MemberExpr *ME = dyn_cast<MemberExpr>(Base)) {
        if (!isa<FieldDecl>(ME->getMemberDecl()))
          Warn = false;
        Base = ME->getBase()->IgnoreParenImpCasts();
      }

      if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(Base)) {
        if (Warn)
          HandleDeclRefExpr(DRE);
        return;
      }

      // The base of a MemberExpr is not a MemberExpr or a DeclRefExpr.
      // Visit that expression.
      Visit(Base);
    }

    void VisitCXXOperatorCallExpr(CXXOperatorCallExpr *E) {
      Expr *Callee = E->getCallee();

      if (isa<UnresolvedLookupExpr>(Callee))
        return Inherited::VisitCXXOperatorCallExpr(E);

      Visit(Callee);
      for (auto Arg: E->arguments())
        HandleValue(Arg->IgnoreParenImpCasts());
    }

    void VisitUnaryOperator(UnaryOperator *E) {
      // For POD record types, addresses of its own members are well-defined.
      if (E->getOpcode() == UO_AddrOf && isRecordType &&
          isa<MemberExpr>(E->getSubExpr()->IgnoreParens())) {
        if (!isPODType)
          HandleValue(E->getSubExpr());
        return;
      }

      if (E->isIncrementDecrementOp()) {
        HandleValue(E->getSubExpr());
        return;
      }

      Inherited::VisitUnaryOperator(E);
    }

    void VisitObjCMessageExpr(ObjCMessageExpr *E) { return; }

    void VisitCXXConstructExpr(CXXConstructExpr *E) {
      if (E->getConstructor()->isCopyConstructor()) {
        Expr *ArgExpr = E->getArg(0);
        if (InitListExpr *ILE = dyn_cast<InitListExpr>(ArgExpr))
          if (ILE->getNumInits() == 1)
            ArgExpr = ILE->getInit(0);
        if (ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(ArgExpr))
          if (ICE->getCastKind() == CK_NoOp)
            ArgExpr = ICE->getSubExpr();
        HandleValue(ArgExpr);
        return;
      }
      Inherited::VisitCXXConstructExpr(E);
    }

    void VisitCallExpr(CallExpr *E) {
      // Treat std::move as a use.
      if (E->getNumArgs() == 1) {
        if (FunctionDecl *FD = E->getDirectCallee()) {
          if (FD->isInStdNamespace() && FD->getIdentifier() &&
              FD->getIdentifier()->isStr("move")) {
            HandleValue(E->getArg(0));
            return;
          }
        }
      }

      Inherited::VisitCallExpr(E);
    }

    void VisitBinaryOperator(BinaryOperator *E) {
      if (E->isCompoundAssignmentOp()) {
        HandleValue(E->getLHS());
        Visit(E->getRHS());
        return;
      }

      Inherited::VisitBinaryOperator(E);
    }

    // A custom visitor for BinaryConditionalOperator is needed because the
    // regular visitor would check the condition and true expression separately
    // but both point to the same place giving duplicate diagnostics.
    void VisitBinaryConditionalOperator(BinaryConditionalOperator *E) {
      Visit(E->getCond());
      Visit(E->getFalseExpr());
    }

    void HandleDeclRefExpr(DeclRefExpr *DRE) {
      Decl* ReferenceDecl = DRE->getDecl();
      if (OrigDecl != ReferenceDecl) return;
      unsigned diag;
      if (isReferenceType) {
        diag = diag::warn_uninit_self_reference_in_reference_init;
      } else if (cast<VarDecl>(OrigDecl)->isStaticLocal()) {
        diag = diag::warn_static_self_reference_in_init;
      } else if (isa<TranslationUnitDecl>(OrigDecl->getDeclContext()) ||
                 isa<NamespaceDecl>(OrigDecl->getDeclContext()) ||
                 DRE->getDecl()->getType()->isRecordType()) {
        diag = diag::warn_uninit_self_reference_in_init;
      } else {
        // Local variables will be handled by the CFG analysis.
        return;
      }

      S.DiagRuntimeBehavior(DRE->getLocStart(), DRE,
                            S.PDiag(diag)
                              << DRE->getNameInfo().getName()
                              << OrigDecl->getLocation()
                              << DRE->getSourceRange());
    }
  };

  /// CheckSelfReference - Warns if OrigDecl is used in expression E.
  static void CheckSelfReference(Sema &S, Decl* OrigDecl, Expr *E,
                                 bool DirectInit) {
    // Parameters arguments are occassionially constructed with itself,
    // for instance, in recursive functions.  Skip them.
    if (isa<ParmVarDecl>(OrigDecl))
      return;

    E = E->IgnoreParens();

    // Skip checking T a = a where T is not a record or reference type.
    // Doing so is a way to silence uninitialized warnings.
    if (!DirectInit && !cast<VarDecl>(OrigDecl)->getType()->isRecordType())
      if (ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(E))
        if (ICE->getCastKind() == CK_LValueToRValue)
          if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(ICE->getSubExpr()))
            if (DRE->getDecl() == OrigDecl)
              return;

    SelfReferenceChecker(S, OrigDecl).CheckExpr(E);
  }
}

/// AddInitializerToDecl - Adds the initializer Init to the
/// declaration dcl. If DirectInit is true, this is C++ direct
/// initialization rather than copy initialization.
void Sema::AddInitializerToDecl(Decl *RealDecl, Expr *Init,
                                bool DirectInit, bool TypeMayContainAuto) {
  // If there is no declaration, there was an error parsing it.  Just ignore
  // the initializer.
  if (!RealDecl || RealDecl->isInvalidDecl()) {
    CorrectDelayedTyposInExpr(Init, dyn_cast_or_null<VarDecl>(RealDecl));
    return;
  }

  if (CXXMethodDecl *Method = dyn_cast<CXXMethodDecl>(RealDecl)) {
    // Pure-specifiers are handled in ActOnPureSpecifier.
    Diag(Method->getLocation(), diag::err_member_function_initialization)
      << Method->getDeclName() << Init->getSourceRange();
    Method->setInvalidDecl();
    return;
  }

  VarDecl *VDecl = dyn_cast<VarDecl>(RealDecl);
  if (!VDecl) {
    assert(!isa<FieldDecl>(RealDecl) && "field init shouldn't get here");
    Diag(RealDecl->getLocation(), diag::err_illegal_initializer);
    RealDecl->setInvalidDecl();
    return;
  }
  ParenListExpr *CXXDirectInit = dyn_cast<ParenListExpr>(Init);

  // C++11 [decl.spec.auto]p6. Deduce the type which 'auto' stands in for.
  if (TypeMayContainAuto && VDecl->getType()->isUndeducedType()) {
    // Attempt typo correction early so that the type of the init expression can
    // be deduced based on the chosen correction:if the original init contains a
    // TypoExpr.
    ExprResult Res = CorrectDelayedTyposInExpr(Init, VDecl);
    if (!Res.isUsable()) {
      RealDecl->setInvalidDecl();
      return;
    }

    if (Res.get() != Init) {
      Init = Res.get();
      if (CXXDirectInit)
        CXXDirectInit = dyn_cast<ParenListExpr>(Init);
    }

    Expr *DeduceInit = Init;
    // Initializer could be a C++ direct-initializer. Deduction only works if it
    // contains exactly one expression.
    if (CXXDirectInit) {
      if (CXXDirectInit->getNumExprs() == 0) {
        // It isn't possible to write this directly, but it is possible to
        // end up in this situation with "auto x(some_pack...);"
        Diag(CXXDirectInit->getLocStart(),
             VDecl->isInitCapture() ? diag::err_init_capture_no_expression
                                    : diag::err_auto_var_init_no_expression)
          << VDecl->getDeclName() << VDecl->getType()
          << VDecl->getSourceRange();
        RealDecl->setInvalidDecl();
        return;
      } else if (CXXDirectInit->getNumExprs() > 1) {
        Diag(CXXDirectInit->getExpr(1)->getLocStart(),
             VDecl->isInitCapture()
                 ? diag::err_init_capture_multiple_expressions
                 : diag::err_auto_var_init_multiple_expressions)
          << VDecl->getDeclName() << VDecl->getType()
          << VDecl->getSourceRange();
        RealDecl->setInvalidDecl();
        return;
      } else {
        DeduceInit = CXXDirectInit->getExpr(0);
        if (isa<InitListExpr>(DeduceInit))
          Diag(CXXDirectInit->getLocStart(),
               diag::err_auto_var_init_paren_braces)
            << VDecl->getDeclName() << VDecl->getType()
            << VDecl->getSourceRange();
      }
    }

    // Expressions default to 'id' when we're in a debugger.
    bool DefaultedToAuto = false;
    if (getLangOpts().DebuggerCastResultToId &&
        Init->getType() == Context.UnknownAnyTy) {
      ExprResult Result = forceUnknownAnyToType(Init, Context.getObjCIdType());
      if (Result.isInvalid()) {
        VDecl->setInvalidDecl();
        return;
      }
      Init = Result.get();
      DefaultedToAuto = true;
    }

    QualType DeducedType;
    if (DeduceAutoType(VDecl->getTypeSourceInfo(), DeduceInit, DeducedType) ==
            DAR_Failed)
      DiagnoseAutoDeductionFailure(VDecl, DeduceInit);
    if (DeducedType.isNull()) {
      RealDecl->setInvalidDecl();
      return;
    }
    VDecl->setType(DeducedType);
    assert(VDecl->isLinkageValid());

    // In ARC, infer lifetime.
    if (getLangOpts().ObjCAutoRefCount && inferObjCARCLifetime(VDecl))
      VDecl->setInvalidDecl();

    // Warn if we deduced 'id'. 'auto' usually implies type-safety, but using
    // 'id' instead of a specific object type prevents most of our usual checks.
    // We only want to warn outside of template instantiations, though:
    // inside a template, the 'id' could have come from a parameter.
    if (ActiveTemplateInstantiations.empty() && !DefaultedToAuto &&
        DeducedType->isObjCIdType()) {
      SourceLocation Loc =
          VDecl->getTypeSourceInfo()->getTypeLoc().getBeginLoc();
      Diag(Loc, diag::warn_auto_var_is_id)
        << VDecl->getDeclName() << DeduceInit->getSourceRange();
    }

    // If this is a redeclaration, check that the type we just deduced matches
    // the previously declared type.
    assert(!getLangOpts().HLSL && "auto types are not supported - merge type below is inconsequential"); // HLSL Change
    if (VarDecl *Old = VDecl->getPreviousDecl()) {
      // We never need to merge the type, because we cannot form an incomplete
      // array of auto, nor deduce such a type.
      ShadowMergeState MergeState = ShadowMergeState_Disallowed; // HLSL Change - add merge state
      MergeVarDeclTypes(VDecl, Old, /*MergeTypeWithPrevious*/false, MergeState); // HLSL Change - add merge state
    }

    // Check the deduced type is valid for a variable declaration.
    CheckVariableDeclarationType(VDecl);
    if (VDecl->isInvalidDecl())
      return;

    // If all looks well, warn if this is a case that will change meaning when
    // we implement N3922.
    if (DirectInit && !CXXDirectInit && isa<InitListExpr>(Init)) {
      Diag(Init->getLocStart(),
           diag::warn_auto_var_direct_list_init)
        << FixItHint::CreateInsertion(Init->getLocStart(), "=");
    }
  }

  // dllimport cannot be used on variable definitions.
  if (VDecl->hasAttr<DLLImportAttr>() && !VDecl->isStaticDataMember()) {
    Diag(VDecl->getLocation(), diag::err_attribute_dllimport_data_definition);
    VDecl->setInvalidDecl();
    return;
  }

  if (VDecl->isLocalVarDecl() && VDecl->hasExternalStorage()) {
    // C99 6.7.8p5. C++ has no such restriction, but that is a defect.
    Diag(VDecl->getLocation(), diag::err_block_extern_cant_init);
    VDecl->setInvalidDecl();
    return;
  }

  if (!VDecl->getType()->isDependentType()) {
    // A definition must end up with a complete type, which means it must be
    // complete with the restriction that an array type might be completed by
    // the initializer; note that later code assumes this restriction.
    QualType BaseDeclType = VDecl->getType();
    if (const ArrayType *Array = Context.getAsIncompleteArrayType(BaseDeclType))
      BaseDeclType = Array->getElementType();
    if (RequireCompleteType(VDecl->getLocation(), BaseDeclType,
                            diag::err_typecheck_decl_incomplete_type)) {
      RealDecl->setInvalidDecl();
      return;
    }

    // The variable can not have an abstract class type.
    if (RequireNonAbstractType(VDecl->getLocation(), VDecl->getType(),
                               diag::err_abstract_type_in_decl,
                               AbstractVariableType))
      VDecl->setInvalidDecl();
  }

  VarDecl *Def;
  if ((Def = VDecl->getDefinition()) && Def != VDecl) {
    NamedDecl *Hidden = nullptr;
    if (!hasVisibleDefinition(Def, &Hidden) && 
        (VDecl->getFormalLinkage() == InternalLinkage ||
         VDecl->getDescribedVarTemplate() ||
         VDecl->getNumTemplateParameterLists() ||
         VDecl->getDeclContext()->isDependentContext())) {
      // The previous definition is hidden, and multiple definitions are
      // permitted (in separate TUs). Form another definition of it.
    } else {
      Diag(VDecl->getLocation(), diag::err_redefinition)
        << VDecl->getDeclName();
      Diag(Def->getLocation(), diag::note_previous_definition);
      VDecl->setInvalidDecl();
      return;
    }
  }

  if (getLangOpts().CPlusPlus) {
    // C++ [class.static.data]p4
    //   If a static data member is of const integral or const
    //   enumeration type, its declaration in the class definition can
    //   specify a constant-initializer which shall be an integral
    //   constant expression (5.19). In that case, the member can appear
    //   in integral constant expressions. The member shall still be
    //   defined in a namespace scope if it is used in the program and the
    //   namespace scope definition shall not contain an initializer.
    //
    // We already performed a redefinition check above, but for static
    // data members we also need to check whether there was an in-class
    // declaration with an initializer.
    if (VDecl->isStaticDataMember() && VDecl->getCanonicalDecl()->hasInit()) {
      Diag(Init->getExprLoc(), diag::err_static_data_member_reinitialization)
          << VDecl->getDeclName();
      Diag(VDecl->getCanonicalDecl()->getInit()->getExprLoc(),
           diag::note_previous_initializer)
          << 0;
      return;
    }  

    if (VDecl->hasLocalStorage())
      getCurFunction()->setHasBranchProtectedScope();

    if (DiagnoseUnexpandedParameterPack(Init, UPPC_Initializer)) {
      VDecl->setInvalidDecl();
      return;
    }
  }

  // OpenCL 1.1 6.5.2: "Variables allocated in the __local address space inside
  // a kernel function cannot be initialized."
  if (VDecl->getStorageClass() == SC_OpenCLWorkGroupLocal) {
    Diag(VDecl->getLocation(), diag::err_local_cant_init);
    VDecl->setInvalidDecl();
    return;
  }

  // Get the decls type and save a reference for later, since
  // CheckInitializerTypes may change it.
  QualType DclT = VDecl->getType(), SavT = DclT;

  // HLSL Change begin
  // When initializing an HLSL resource type we should diagnose mismatches in
  // globally coherent annotations _unless_ the source is a dynamic resource
  // placeholder type where we safely infer the globallycoherent annotaiton.
  DiagnoseGloballyCoherentMismatch(Init, DclT, Init->getExprLoc());
  // HLSL Change end
  
  // Expressions default to 'id' when we're in a debugger
  // and we are assigning it to a variable of Objective-C pointer type.
  if (getLangOpts().DebuggerCastResultToId && DclT->isObjCObjectPointerType() &&
      Init->getType() == Context.UnknownAnyTy) {
    ExprResult Result = forceUnknownAnyToType(Init, Context.getObjCIdType());
    if (Result.isInvalid()) {
      VDecl->setInvalidDecl();
      return;
    }
    Init = Result.get();
  }

  // Perform the initialization.
  if (!VDecl->isInvalidDecl()) {
    InitializedEntity Entity = InitializedEntity::InitializeVariable(VDecl);
    InitializationKind Kind
      = DirectInit ?
          CXXDirectInit ? InitializationKind::CreateDirect(VDecl->getLocation(),
                                                           Init->getLocStart(),
                                                           Init->getLocEnd())
                        : InitializationKind::CreateDirectList(
                                                          VDecl->getLocation())
                   : InitializationKind::CreateCopy(VDecl->getLocation(),
                                                    Init->getLocStart());

    MultiExprArg Args = Init;
    if (CXXDirectInit)
      Args = MultiExprArg(CXXDirectInit->getExprs(),
                          CXXDirectInit->getNumExprs());

    // Try to correct any TypoExprs in the initialization arguments.
    for (size_t Idx = 0; Idx < Args.size(); ++Idx) {
      ExprResult Res = CorrectDelayedTyposInExpr(
          Args[Idx], VDecl, [this, Entity, Kind](Expr *E) {
            InitializationSequence Init(*this, Entity, Kind, MultiExprArg(E));
            return Init.Failed() ? ExprError() : E;
          });
      if (Res.isInvalid()) {
        VDecl->setInvalidDecl();
      } else if (Res.get() != Args[Idx]) {
        Args[Idx] = Res.get();
      }
    }
    if (VDecl->isInvalidDecl())
      return;

    InitializationSequence InitSeq(*this, Entity, Kind, Args);
    ExprResult Result = InitSeq.Perform(*this, Entity, Kind, Args, &DclT);
    if (Result.isInvalid()) {
      VDecl->setInvalidDecl();
      return;
    }

    Init = Result.getAs<Expr>();
    assert(Init != nullptr && "otherwise result should be invalid"); // HLSL Change
  }

  // Check for self-references within variable initializers.
  // Variables declared within a function/method body (except for references)
  // are handled by a dataflow analysis.
  if (!VDecl->hasLocalStorage() || VDecl->getType()->isRecordType() ||
      VDecl->getType()->isReferenceType()) {
    CheckSelfReference(*this, RealDecl, Init, DirectInit);
  }

  // If the type changed, it means we had an incomplete type that was
  // completed by the initializer. For example:
  //   int ary[] = { 1, 3, 5 };
  // "ary" transitions from an IncompleteArrayType to a ConstantArrayType.
  if (!VDecl->isInvalidDecl() && (DclT != SavT))
    VDecl->setType(DclT);

#if 0 // HLSL Change Starts - no ObjC support
  if (!VDecl->isInvalidDecl()) {
    checkUnsafeAssigns(VDecl->getLocation(), VDecl->getType(), Init);

    if (VDecl->hasAttr<BlocksAttr>())
      checkRetainCycles(VDecl, Init);

    // It is safe to assign a weak reference into a strong variable.
    // Although this code can still have problems:
    //   id x = self.weakProp;
    //   id y = self.weakProp;
    // we do not warn to warn spuriously when 'x' and 'y' are on separate
    // paths through the function. This should be revisited if
    // -Wrepeated-use-of-weak is made flow-sensitive.
    if (VDecl->getType().getObjCLifetime() == Qualifiers::OCL_Strong &&
        !Diags.isIgnored(diag::warn_arc_repeated_use_of_weak,
                         Init->getLocStart()))
        getCurFunction()->markSafeWeakUse(Init);
  }
#endif // HLSL Change Ends - no ObjC support

  // The initialization is usually a full-expression.
  //
  // FIXME: If this is a braced initialization of an aggregate, it is not
  // an expression, and each individual field initializer is a separate
  // full-expression. For instance, in:
  //
  //   struct Temp { ~Temp(); };
  //   struct S { S(Temp); };
  //   struct T { S a, b; } t = { Temp(), Temp() }
  //
  // we should destroy the first Temp before constructing the second.
  ExprResult Result = ActOnFinishFullExpr(Init, VDecl->getLocation(),
                                          false,
                                          VDecl->isConstexpr());
  if (Result.isInvalid()) {
    VDecl->setInvalidDecl();
    return;
  }
  Init = Result.get();

  // Attach the initializer to the decl.
  VDecl->setInit(Init);

  if (VDecl->isLocalVarDecl()) {
    // C99 6.7.8p4: All the expressions in an initializer for an object that has
    // static storage duration shall be constant expressions or string literals.
    // C++ does not have this restriction.
    if (!getLangOpts().CPlusPlus && !VDecl->isInvalidDecl()) {
      const Expr *Culprit;
      if (VDecl->getStorageClass() == SC_Static)
        CheckForConstantInitializer(Init, DclT);
      // C89 is stricter than C99 for non-static aggregate types.
      // C89 6.5.7p3: All the expressions [...] in an initializer list
      // for an object that has aggregate or union type shall be
      // constant expressions.
      else if (!getLangOpts().C99 && VDecl->getType()->isAggregateType() &&
               isa<InitListExpr>(Init) &&
               !Init->isConstantInitializer(Context, false, &Culprit))
        Diag(Culprit->getExprLoc(),
             diag::ext_aggregate_init_not_constant)
          << Culprit->getSourceRange();
    }
  } else if (VDecl->isStaticDataMember() &&
             VDecl->getLexicalDeclContext()->isRecord()) {
    // This is an in-class initialization for a static data member, e.g.,
    //
    // struct S {
    //   static const int value = 17;
    // };

    // C++ [class.mem]p4:
    //   A member-declarator can contain a constant-initializer only
    //   if it declares a static member (9.4) of const integral or
    //   const enumeration type, see 9.4.2.
    //
    // C++11 [class.static.data]p3:
    //   If a non-volatile const static data member is of integral or
    //   enumeration type, its declaration in the class definition can
    //   specify a brace-or-equal-initializer in which every initalizer-clause
    //   that is an assignment-expression is a constant expression. A static
    //   data member of literal type can be declared in the class definition
    //   with the constexpr specifier; if so, its declaration shall specify a
    //   brace-or-equal-initializer in which every initializer-clause that is
    //   an assignment-expression is a constant expression.

    // Do nothing on dependent types.
    if (DclT->isDependentType()) {

    // Allow any 'static constexpr' members, whether or not they are of literal
    // type. We separately check that every constexpr variable is of literal
    // type.
    } else if (VDecl->isConstexpr()) {

    // Require constness.
    } else if (!DclT.isConstQualified()) {
      Diag(VDecl->getLocation(), diag::err_in_class_initializer_non_const)
        << Init->getSourceRange();
      VDecl->setInvalidDecl();

    // We allow integer constant expressions in all cases.
    } else if (DclT->isIntegralOrEnumerationType()) {
      // Check whether the expression is a constant expression.
      SourceLocation Loc;
      if (getLangOpts().CPlusPlus11 && DclT.isVolatileQualified())
        // In C++11, a non-constexpr const static data member with an
        // in-class initializer cannot be volatile.
        Diag(VDecl->getLocation(), diag::err_in_class_initializer_volatile);
      else if (Init->isValueDependent())
        ; // Nothing to check.
      else if (Init->isIntegerConstantExpr(Context, &Loc))
        ; // Ok, it's an ICE!
      else if (Init->isEvaluatable(Context)) {
        // If we can constant fold the initializer through heroics, accept it,
        // but report this as a use of an extension for -pedantic.
        Diag(Loc, diag::ext_in_class_initializer_non_constant)
          << Init->getSourceRange();
      } else {
        // Otherwise, this is some crazy unknown case.  Report the issue at the
        // location provided by the isIntegerConstantExpr failed check.
        Diag(Loc, diag::err_in_class_initializer_non_constant)
          << Init->getSourceRange();
        VDecl->setInvalidDecl();
      }

    // We allow foldable floating-point constants as an extension.
    } else if (DclT->isFloatingType()) { // also permits complex, which is ok
      // In C++98, this is a GNU extension. In C++11, it is not, but we support
      // it anyway and provide a fixit to add the 'constexpr'.
      if (getLangOpts().CPlusPlus11) {
        Diag(VDecl->getLocation(),
             diag::ext_in_class_initializer_float_type_cxx11)
            << DclT << Init->getSourceRange();
        Diag(VDecl->getLocStart(),
             diag::note_in_class_initializer_float_type_cxx11)
            << FixItHint::CreateInsertion(VDecl->getLocStart(), "constexpr ");
      } else {
        Diag(VDecl->getLocation(), diag::ext_in_class_initializer_float_type)
          << DclT << Init->getSourceRange();

        if (!Init->isValueDependent() && !Init->isEvaluatable(Context)) {
          Diag(Init->getExprLoc(), diag::err_in_class_initializer_non_constant)
            << Init->getSourceRange();
          VDecl->setInvalidDecl();
        }
      }

    // Suggest adding 'constexpr' in C++11 for literal types.
    } else if (getLangOpts().CPlusPlus11 && DclT->isLiteralType(Context)) {
      Diag(VDecl->getLocation(), diag::err_in_class_initializer_literal_type)
        << DclT << Init->getSourceRange()
        << FixItHint::CreateInsertion(VDecl->getLocStart(), "constexpr ");
      VDecl->setConstexpr(true);

    } else {
      Diag(VDecl->getLocation(), diag::err_in_class_initializer_bad_type)
        << DclT << Init->getSourceRange();
      VDecl->setInvalidDecl();
    }
  } else if (VDecl->isFileVarDecl()) {
    if (VDecl->getStorageClass() == SC_Extern &&
        (!getLangOpts().CPlusPlus ||
          !VDecl->isExternC()) &&
        !isTemplateInstantiation(VDecl->getTemplateSpecializationKind()))
      Diag(VDecl->getLocation(), diag::warn_extern_init);

    // C99 6.7.8p4. All file scoped initializers need to be constant.
    if (!getLangOpts().CPlusPlus && !VDecl->isInvalidDecl())
      CheckForConstantInitializer(Init, DclT);
  }

  // We will represent direct-initialization similarly to copy-initialization:
  //    int x(1);  -as-> int x = 1;
  //    ClassType x(a,b,c); -as-> ClassType x = ClassType(a,b,c);
  //
  // Clients that want to distinguish between the two forms, can check for
  // direct initializer using VarDecl::getInitStyle().
  // A major benefit is that clients that don't particularly care about which
  // exactly form was it (like the CodeGen) can handle both cases without
  // special case code.

  // C++ 8.5p11:
  // The form of initialization (using parentheses or '=') is generally
  // insignificant, but does matter when the entity being initialized has a
  // class type.
  if (CXXDirectInit) {
    assert(DirectInit && "Call-style initializer must be direct init.");
    VDecl->setInitStyle(VarDecl::CallInit);
  } else if (DirectInit) {
    // This must be list-initialization. No other way is direct-initialization.
    VDecl->setInitStyle(VarDecl::ListInit);
  }

  CheckCompleteVariableDeclaration(VDecl);
}

/// ActOnInitializerError - Given that there was an error parsing an
/// initializer for the given declaration, try to return to some form
/// of sanity.
void Sema::ActOnInitializerError(Decl *D) {
  // Our main concern here is re-establishing invariants like "a
  // variable's type is either dependent or complete".
  if (!D || D->isInvalidDecl()) return;

  VarDecl *VD = dyn_cast<VarDecl>(D);
  if (!VD) return;

  // Auto types are meaningless if we can't make sense of the initializer.
  if (ParsingInitForAutoVars.count(D)) {
    D->setInvalidDecl();
    return;
  }

  QualType Ty = VD->getType();
  if (Ty->isDependentType()) return;

  // Require a complete type.
  if (RequireCompleteType(VD->getLocation(), 
                          Context.getBaseElementType(Ty),
                          diag::err_typecheck_decl_incomplete_type)) {
    VD->setInvalidDecl();
    return;
  }

  // Require a non-abstract type.
  if (RequireNonAbstractType(VD->getLocation(), Ty,
                             diag::err_abstract_type_in_decl,
                             AbstractVariableType)) {
    VD->setInvalidDecl();
    return;
  }

  // Don't bother complaining about constructors or destructors,
  // though.
}

void Sema::ActOnUninitializedDecl(Decl *RealDecl,
                                  bool TypeMayContainAuto) {
  // If there is no declaration, there was an error parsing it. Just ignore it.
  if (!RealDecl)
    return;

  if (VarDecl *Var = dyn_cast<VarDecl>(RealDecl)) {
    QualType Type = Var->getType();

    // C++11 [dcl.spec.auto]p3
    if (TypeMayContainAuto && Type->getContainedAutoType()) {
      Diag(Var->getLocation(), diag::err_auto_var_requires_init)
        << Var->getDeclName() << Type;
      Var->setInvalidDecl();
      return;
    }

    // C++11 [class.static.data]p3: A static data member can be declared with
    // the constexpr specifier; if so, its declaration shall specify
    // a brace-or-equal-initializer.
    // C++11 [dcl.constexpr]p1: The constexpr specifier shall be applied only to
    // the definition of a variable [...] or the declaration of a static data
    // member.
    if (Var->isConstexpr() && !Var->isThisDeclarationADefinition()) {
      if (Var->isStaticDataMember())
        Diag(Var->getLocation(),
             diag::err_constexpr_static_mem_var_requires_init)
          << Var->getDeclName();
      else
        Diag(Var->getLocation(), diag::err_invalid_constexpr_var_decl);
      Var->setInvalidDecl();
      return;
    }

    // OpenCL v1.1 s6.5.3: variables declared in the constant address space must
    // be initialized.
    if (!Var->isInvalidDecl() &&
        Var->getType().getAddressSpace() == LangAS::opencl_constant &&
        Var->getStorageClass() != SC_Extern && !Var->getInit()) {
      Diag(Var->getLocation(), diag::err_opencl_constant_no_init);
      Var->setInvalidDecl();
      return;
    }

    switch (Var->isThisDeclarationADefinition()) {
    case VarDecl::Definition:
      if (!Var->isStaticDataMember() || !Var->getAnyInitializer())
        break;

      // We have an out-of-line definition of a static data member
      // that has an in-class initializer, so we type-check this like
      // a declaration. 
      //
      LLVM_FALLTHROUGH; // HLSL Change
      
    case VarDecl::DeclarationOnly:
      // It's only a declaration. 

      // Block scope. C99 6.7p7: If an identifier for an object is
      // declared with no linkage (C99 6.2.2p6), the type for the
      // object shall be complete.
      if (!Type->isDependentType() && Var->isLocalVarDecl() && 
          !Var->hasLinkage() && !Var->isInvalidDecl() &&
          RequireCompleteType(Var->getLocation(), Type,
                              diag::err_typecheck_decl_incomplete_type))
        Var->setInvalidDecl();

      // Make sure that the type is not abstract.
      if (!Type->isDependentType() && !Var->isInvalidDecl() &&
          RequireNonAbstractType(Var->getLocation(), Type,
                                 diag::err_abstract_type_in_decl,
                                 AbstractVariableType))
        Var->setInvalidDecl();
      if (!Type->isDependentType() && !Var->isInvalidDecl() &&
          Var->getStorageClass() == SC_PrivateExtern) {
        Diag(Var->getLocation(), diag::warn_private_extern);
        Diag(Var->getLocation(), diag::note_private_extern);
      }
        
      return;

    case VarDecl::TentativeDefinition:
      // File scope. C99 6.9.2p2: A declaration of an identifier for an
      // object that has file scope without an initializer, and without a
      // storage-class specifier or with the storage-class specifier "static",
      // constitutes a tentative definition. Note: A tentative definition with
      // external linkage is valid (C99 6.2.2p5).
      if (!Var->isInvalidDecl()) {
        if (const IncompleteArrayType *ArrayT
                                    = Context.getAsIncompleteArrayType(Type)) {
          if (RequireCompleteType(Var->getLocation(),
                                  ArrayT->getElementType(),
                                  diag::err_illegal_decl_array_incomplete_type))
            Var->setInvalidDecl();
        } else if (Var->getStorageClass() == SC_Static) {
          // C99 6.9.2p3: If the declaration of an identifier for an object is
          // a tentative definition and has internal linkage (C99 6.2.2p3), the
          // declared type shall not be an incomplete type.
          // NOTE: code such as the following
          //     static struct s;
          //     struct s { int a; };
          // is accepted by gcc. Hence here we issue a warning instead of
          // an error and we do not invalidate the static declaration.
          // NOTE: to avoid multiple warnings, only check the first declaration.
          if (Var->isFirstDecl())
            RequireCompleteType(Var->getLocation(), Type,
                                diag::ext_typecheck_decl_incomplete_type);
        }
      }

      // Record the tentative definition; we're done.
      if (!Var->isInvalidDecl())
        TentativeDefinitions.push_back(Var);
      return;
    }
    
    // Provide a specific diagnostic for uninitialized variable
    // definitions with incomplete array type.
    if (Type->isIncompleteArrayType()
        // HLSL Change Starts
        // Allow incomplete resource array.
        && !hlsl::IsIncompleteHLSLResourceArrayType(getASTContext(), Type)
        // Allow incomplete ConstantBufferView array.
        && !(IsOnHLSLBufferView())
        // HLSL Change Ends
        ) {
      Diag(Var->getLocation(),
           diag::err_typecheck_incomplete_array_needs_initializer);
      Var->setInvalidDecl();
      return;
    }

    // Provide a specific diagnostic for uninitialized variable
    // definitions with reference type.
    if (Type->isReferenceType()) {
      Diag(Var->getLocation(), diag::err_reference_var_requires_init)
        << Var->getDeclName()
        << SourceRange(Var->getLocation(), Var->getLocation());
      Var->setInvalidDecl();
      return;
    }

    // Do not attempt to type-check the default initializer for a
    // variable with dependent type.
    if (Type->isDependentType())
      return;

    if (Var->isInvalidDecl())
      return;

    if (!Var->hasAttr<AliasAttr>()) {
      if (RequireCompleteType(Var->getLocation(),
                              Context.getBaseElementType(Type),
                              diag::err_typecheck_decl_incomplete_type)) {
        Var->setInvalidDecl();
        return;
      }
    } else {
      return;
    }

    // The variable can not have an abstract class type.
    if (RequireNonAbstractType(Var->getLocation(), Type,
                               diag::err_abstract_type_in_decl,
                               AbstractVariableType)) {
      Var->setInvalidDecl();
      return;
    }

    // Check for jumps past the implicit initializer.  C++0x
    // clarifies that this applies to a "variable with automatic
    // storage duration", not a "local variable".
    // C++11 [stmt.dcl]p3
    //   A program that jumps from a point where a variable with automatic
    //   storage duration is not in scope to a point where it is in scope is
    //   ill-formed unless the variable has scalar type, class type with a
    //   trivial default constructor and a trivial destructor, a cv-qualified
    //   version of one of these types, or an array of one of the preceding
    //   types and is declared without an initializer.
    if (getLangOpts().CPlusPlus && Var->hasLocalStorage()) {
      if (const RecordType *Record
            = Context.getBaseElementType(Type)->getAs<RecordType>()) {
        CXXRecordDecl *CXXRecord = cast<CXXRecordDecl>(Record->getDecl());
        // Mark the function for further checking even if the looser rules of
        // C++11 do not require such checks, so that we can diagnose
        // incompatibilities with C++98.
        if (!CXXRecord->isPOD())
          getCurFunction()->setHasBranchProtectedScope();
      }
    }
    
    // C++03 [dcl.init]p9:
    //   If no initializer is specified for an object, and the
    //   object is of (possibly cv-qualified) non-POD class type (or
    //   array thereof), the object shall be default-initialized; if
    //   the object is of const-qualified type, the underlying class
    //   type shall have a user-declared default
    //   constructor. Otherwise, if no initializer is specified for
    //   a non- static object, the object and its subobjects, if
    //   any, have an indeterminate initial value); if the object
    //   or any of its subobjects are of const-qualified type, the
    //   program is ill-formed.
    // C++0x [dcl.init]p11:
    //   If no initializer is specified for an object, the object is
    //   default-initialized; [...].
    InitializedEntity Entity = InitializedEntity::InitializeVariable(Var);
    InitializationKind Kind
      = InitializationKind::CreateDefault(Var->getLocation());

    InitializationSequence InitSeq(*this, Entity, Kind, None);
    ExprResult Init = InitSeq.Perform(*this, Entity, Kind, None);
    if (Init.isInvalid())
      Var->setInvalidDecl();
    else if (Init.get()) {
      Var->setInit(MaybeCreateExprWithCleanups(Init.get()));
      // This is important for template substitution.
      Var->setInitStyle(VarDecl::CallInit);
    }

    CheckCompleteVariableDeclaration(Var);
  }
}

void Sema::ActOnCXXForRangeDecl(Decl *D) {
  VarDecl *VD = dyn_cast<VarDecl>(D);
  if (!VD) {
    Diag(D->getLocation(), diag::err_for_range_decl_must_be_var);
    D->setInvalidDecl();
    return;
  }

  VD->setCXXForRangeDecl(true);

  // for-range-declaration cannot be given a storage class specifier.
  int Error = -1;
  switch (VD->getStorageClass()) {
  case SC_None:
    break;
  case SC_Extern:
    Error = 0;
    break;
  case SC_Static:
    Error = 1;
    break;
  case SC_PrivateExtern:
    Error = 2;
    break;
  case SC_Auto:
    Error = 3;
    break;
  case SC_Register:
    Error = 4;
    break;
  case SC_OpenCLWorkGroupLocal:
    llvm_unreachable("Unexpected storage class");
  }
  if (Error != -1) {
    Diag(VD->getOuterLocStart(), diag::err_for_range_storage_class)
      << VD->getDeclName() << Error;
    D->setInvalidDecl();
  }
}

StmtResult
Sema::ActOnCXXForRangeIdentifier(Scope *S, SourceLocation IdentLoc,
                                 IdentifierInfo *Ident,
                                 ParsedAttributes &Attrs,
                                 SourceLocation AttrEnd) {
  // C++1y [stmt.iter]p1:
  //   A range-based for statement of the form
  //      for ( for-range-identifier : for-range-initializer ) statement
  //   is equivalent to
  //      for ( auto&& for-range-identifier : for-range-initializer ) statement
  DeclSpec DS(Attrs.getPool().getFactory());

  const char *PrevSpec;
  unsigned DiagID;
  DS.SetTypeSpecType(DeclSpec::TST_auto, IdentLoc, PrevSpec, DiagID,
                     getPrintingPolicy());

  Declarator D(DS, Declarator::ForContext);
  D.SetIdentifier(Ident, IdentLoc);
  D.takeAttributes(Attrs, AttrEnd);

  ParsedAttributes EmptyAttrs(Attrs.getPool().getFactory());
  D.AddTypeInfo(DeclaratorChunk::getReference(0, IdentLoc, /*lvalue*/false),
                EmptyAttrs, IdentLoc);
  Decl *Var = ActOnDeclarator(S, D);
  cast<VarDecl>(Var)->setCXXForRangeDecl(true);
  FinalizeDeclaration(Var);
  return ActOnDeclStmt(FinalizeDeclaratorGroup(S, DS, Var), IdentLoc,
                       AttrEnd.isValid() ? AttrEnd : IdentLoc);
}

void Sema::CheckCompleteVariableDeclaration(VarDecl *var) {
  if (var->isInvalidDecl()) return;

  // In ARC, don't allow jumps past the implicit initialization of a
  // local retaining variable.
  if (getLangOpts().ObjCAutoRefCount &&
      var->hasLocalStorage()) {
    switch (var->getType().getObjCLifetime()) {
    case Qualifiers::OCL_None:
    case Qualifiers::OCL_ExplicitNone:
    case Qualifiers::OCL_Autoreleasing:
      break;

    case Qualifiers::OCL_Weak:
    case Qualifiers::OCL_Strong:
      getCurFunction()->setHasBranchProtectedScope();
      break;
    }
  }

  // Warn about externally-visible variables being defined without a
  // prior declaration.  We only want to do this for global
  // declarations, but we also specifically need to avoid doing it for
  // class members because the linkage of an anonymous class can
  // change if it's later given a typedef name.
  if (var->isThisDeclarationADefinition() &&
      var->getDeclContext()->getRedeclContext()->isFileContext() &&
      var->isExternallyVisible() && var->hasLinkage() &&
      !getDiagnostics().isIgnored(diag::warn_missing_variable_declarations,
                                  var->getLocation())) {
    // Find a previous declaration that's not a definition.
    VarDecl *prev = var->getPreviousDecl();
    while (prev && prev->isThisDeclarationADefinition())
      prev = prev->getPreviousDecl();

    if (!prev)
      Diag(var->getLocation(), diag::warn_missing_variable_declarations) << var;
  }

  if (var->getTLSKind() == VarDecl::TLS_Static) {
    const Expr *Culprit;
    if (var->getType().isDestructedType()) {
      // GNU C++98 edits for __thread, [basic.start.term]p3:
      //   The type of an object with thread storage duration shall not
      //   have a non-trivial destructor.
      Diag(var->getLocation(), diag::err_thread_nontrivial_dtor);
      if (getLangOpts().CPlusPlus11)
        Diag(var->getLocation(), diag::note_use_thread_local);
    } else if (getLangOpts().CPlusPlus && var->hasInit() &&
               !var->getInit()->isConstantInitializer(
                   Context, var->getType()->isReferenceType(), &Culprit)) {
      // GNU C++98 edits for __thread, [basic.start.init]p4:
      //   An object of thread storage duration shall not require dynamic
      //   initialization.
      // FIXME: Need strict checking here.
      Diag(Culprit->getExprLoc(), diag::err_thread_dynamic_init)
        << Culprit->getSourceRange();
      if (getLangOpts().CPlusPlus11)
        Diag(var->getLocation(), diag::note_use_thread_local);
    }

  }

  // Apply section attributes and pragmas to global variables.
  bool GlobalStorage = var->hasGlobalStorage();
  if (GlobalStorage && var->isThisDeclarationADefinition() &&
      ActiveTemplateInstantiations.empty()) {
    PragmaStack<StringLiteral *> *Stack = nullptr;
    int SectionFlags = ASTContext::PSF_Implicit | ASTContext::PSF_Read;
    if (var->getType().isConstQualified()
      // HLSL Change: for HLSL, const initialized isn't the same, unless static
        && (!getLangOpts().HLSL ||
            var->getTLSKind() == VarDecl::TLS_Static)
        )
      Stack = &ConstSegStack;
    else if (!var->getInit()) {
      Stack = &BSSSegStack;
      SectionFlags |= ASTContext::PSF_Write;
    } else {
      Stack = &DataSegStack;
      SectionFlags |= ASTContext::PSF_Write;
    }
    if (Stack->CurrentValue && !var->hasAttr<SectionAttr>()) {
      var->addAttr(SectionAttr::CreateImplicit(
          Context, SectionAttr::Declspec_allocate,
          Stack->CurrentValue->getString(), Stack->CurrentPragmaLocation));
    }
    if (const SectionAttr *SA = var->getAttr<SectionAttr>())
      if (UnifySection(SA->getName(), SectionFlags, var))
        var->dropAttr<SectionAttr>();

    // Apply the init_seg attribute if this has an initializer.  If the
    // initializer turns out to not be dynamic, we'll end up ignoring this
    // attribute.
    if (CurInitSeg && var->getInit())
      var->addAttr(InitSegAttr::CreateImplicit(Context, CurInitSeg->getString(),
                                               CurInitSegLoc));
  }

  // All the following checks are C++ only.
  if (!getLangOpts().CPlusPlus) return;

  QualType type = var->getType();
  if (type->isDependentType()) return;

  // __block variables might require us to capture a copy-initializer.
  if (var->hasAttr<BlocksAttr>()) {
    // It's currently invalid to ever have a __block variable with an
    // array type; should we diagnose that here?

    // Regardless, we don't want to ignore array nesting when
    // constructing this copy.
    if (type->isStructureOrClassType()) {
      EnterExpressionEvaluationContext scope(*this, PotentiallyEvaluated);
      SourceLocation poi = var->getLocation();
      Expr *varRef =new (Context) DeclRefExpr(var, false, type, VK_LValue, poi);
      ExprResult result
        = PerformMoveOrCopyInitialization(
            InitializedEntity::InitializeBlock(poi, type, false),
            var, var->getType(), varRef, /*AllowNRVO=*/true);
      if (!result.isInvalid()) {
        result = MaybeCreateExprWithCleanups(result);
        Expr *init = result.getAs<Expr>();
        Context.setBlockVarCopyInits(var, init);
      }
    }
  }

  Expr *Init = var->getInit();
  bool IsGlobal = GlobalStorage && !var->isStaticLocal();
  QualType baseType = Context.getBaseElementType(type);

  if (!var->getDeclContext()->isDependentContext() &&
      Init && !Init->isValueDependent()) {
    if (IsGlobal && !var->isConstexpr() &&
        !getDiagnostics().isIgnored(diag::warn_global_constructor,
                                    var->getLocation())) {
      // Warn about globals which don't have a constant initializer.  Don't
      // warn about globals with a non-trivial destructor because we already
      // warned about them.
      CXXRecordDecl *RD = baseType->getAsCXXRecordDecl();
      if (!(RD && !RD->hasTrivialDestructor()) &&
          !Init->isConstantInitializer(Context, baseType->isReferenceType()))
        Diag(var->getLocation(), diag::warn_global_constructor)
          << Init->getSourceRange();
    }

    if (var->isConstexpr()) {
      SmallVector<PartialDiagnosticAt, 8> Notes;
      if (!var->evaluateValue(Notes) || !var->isInitICE()) {
        SourceLocation DiagLoc = var->getLocation();
        // If the note doesn't add any useful information other than a source
        // location, fold it into the primary diagnostic.
        if (Notes.size() == 1 && Notes[0].second.getDiagID() ==
              diag::note_invalid_subexpr_in_const_expr) {
          DiagLoc = Notes[0].first;
          Notes.clear();
        }
        Diag(DiagLoc, diag::err_constexpr_var_requires_const_init)
          << var << Init->getSourceRange();
        for (unsigned I = 0, N = Notes.size(); I != N; ++I)
          Diag(Notes[I].first, Notes[I].second);
      }
    } else if (var->isUsableInConstantExpressions(Context)) {
      // Check whether the initializer of a const variable of integral or
      // enumeration type is an ICE now, since we can't tell whether it was
      // initialized by a constant expression if we check later.
      var->checkInitIsICE();
    }
  }

  // Require the destructor.
  if (const RecordType *recordType = baseType->getAs<RecordType>())
    FinalizeVarWithDestructor(var, recordType);
}

/// \brief Determines if a variable's alignment is dependent.
static bool hasDependentAlignment(VarDecl *VD) {
  if (VD->getType()->isDependentType())
    return true;
  for (auto *I : VD->specific_attrs<AlignedAttr>())
    if (I->isAlignmentDependent())
      return true;
  return false;
}

/// FinalizeDeclaration - called by ParseDeclarationAfterDeclarator to perform
/// any semantic actions necessary after any initializer has been attached.
void
Sema::FinalizeDeclaration(Decl *ThisDecl) {
  // Note that we are no longer parsing the initializer for this declaration.
  ParsingInitForAutoVars.erase(ThisDecl);

  VarDecl *VD = dyn_cast_or_null<VarDecl>(ThisDecl);
  if (!VD)
    return;

  checkAttributesAfterMerging(*this, *VD);

  // Perform TLS alignment check here after attributes attached to the variable
  // which may affect the alignment have been processed. Only perform the check
  // if the target has a maximum TLS alignment (zero means no constraints).
  if (unsigned MaxAlign = Context.getTargetInfo().getMaxTLSAlign()) {
    // Protect the check so that it's not performed on dependent types and
    // dependent alignments (we can't determine the alignment in that case).
    if (VD->getTLSKind() && !hasDependentAlignment(VD)) {
      CharUnits MaxAlignChars = Context.toCharUnitsFromBits(MaxAlign);
      if (Context.getDeclAlign(VD) > MaxAlignChars) {
        Diag(VD->getLocation(), diag::err_tls_var_aligned_over_maximum)
          << (unsigned)Context.getDeclAlign(VD).getQuantity() << VD
          << (unsigned)MaxAlignChars.getQuantity();
      }
    }
  }

  // Static locals inherit dll attributes from their function.
  if (VD->isStaticLocal()) {
    if (FunctionDecl *FD =
            dyn_cast_or_null<FunctionDecl>(VD->getParentFunctionOrMethod())) {
      if (Attr *A = getDLLAttr(FD)) {
        auto *NewAttr = cast<InheritableAttr>(A->clone(getASTContext()));
        NewAttr->setInherited(true);
        VD->addAttr(NewAttr);
      }
    }
  }

  // Grab the dllimport or dllexport attribute off of the VarDecl.
  const InheritableAttr *DLLAttr = getDLLAttr(VD);

  // Imported static data members cannot be defined out-of-line.
  if (const auto *IA = dyn_cast_or_null<DLLImportAttr>(DLLAttr)) {
    if (VD->isStaticDataMember() && VD->isOutOfLine() &&
        VD->isThisDeclarationADefinition()) {
      // We allow definitions of dllimport class template static data members
      // with a warning.
      CXXRecordDecl *Context =
        cast<CXXRecordDecl>(VD->getFirstDecl()->getDeclContext());
      bool IsClassTemplateMember =
          isa<ClassTemplatePartialSpecializationDecl>(Context) ||
          Context->getDescribedClassTemplate();

      Diag(VD->getLocation(),
           IsClassTemplateMember
               ? diag::warn_attribute_dllimport_static_field_definition
               : diag::err_attribute_dllimport_static_field_definition);
      Diag(IA->getLocation(), diag::note_attribute);
      if (!IsClassTemplateMember)
        VD->setInvalidDecl();
    }
  }

  // dllimport/dllexport variables cannot be thread local, their TLS index
  // isn't exported with the variable.
  if (DLLAttr && VD->getTLSKind()) {
    Diag(VD->getLocation(), diag::err_attribute_dll_thread_local) << VD
                                                                  << DLLAttr;
    VD->setInvalidDecl();
  }

  if (UsedAttr *Attr = VD->getAttr<UsedAttr>()) {
    if (!Attr->isInherited() && !VD->isThisDeclarationADefinition()) {
      Diag(Attr->getLocation(), diag::warn_attribute_ignored) << Attr;
      VD->dropAttr<UsedAttr>();
    }
  }

  const DeclContext *DC = VD->getDeclContext();
  // If there's a #pragma GCC visibility in scope, and this isn't a class
  // member, set the visibility of this variable.
  if (DC->getRedeclContext()->isFileContext() && VD->isExternallyVisible())
    AddPushedVisibilityAttribute(VD);

  // FIXME: Warn on unused templates.
  if (VD->isFileVarDecl() && !VD->getDescribedVarTemplate() &&
      !isa<VarTemplatePartialSpecializationDecl>(VD))
    MarkUnusedFileScopedDecl(VD);

  // Now we have parsed the initializer and can update the table of magic
  // tag values.
  if (!VD->hasAttr<TypeTagForDatatypeAttr>() ||
      !VD->getType()->isIntegralOrEnumerationType())
    return;

  for (const auto *I : ThisDecl->specific_attrs<TypeTagForDatatypeAttr>()) {
    const Expr *MagicValueExpr = VD->getInit();
    if (!MagicValueExpr) {
      continue;
    }
    llvm::APSInt MagicValueInt;
    if (!MagicValueExpr->isIntegerConstantExpr(MagicValueInt, Context)) {
      Diag(I->getRange().getBegin(),
           diag::err_type_tag_for_datatype_not_ice)
        << LangOpts.CPlusPlus << MagicValueExpr->getSourceRange();
      continue;
    }
    if (MagicValueInt.getActiveBits() > 64) {
      Diag(I->getRange().getBegin(),
           diag::err_type_tag_for_datatype_too_large)
        << LangOpts.CPlusPlus << MagicValueExpr->getSourceRange();
      continue;
    }
    uint64_t MagicValue = MagicValueInt.getZExtValue();
    RegisterTypeTagForDatatype(I->getArgumentKind(),
                               MagicValue,
                               I->getMatchingCType(),
                               I->getLayoutCompatible(),
                               I->getMustBeNull());
  }
}

Sema::DeclGroupPtrTy Sema::FinalizeDeclaratorGroup(Scope *S, const DeclSpec &DS,
                                                   ArrayRef<Decl *> Group) {
  SmallVector<Decl*, 8> Decls;

  if (DS.isTypeSpecOwned())
    Decls.push_back(DS.getRepAsDecl());

  DeclaratorDecl *FirstDeclaratorInGroup = nullptr;
  for (unsigned i = 0, e = Group.size(); i != e; ++i)
    if (Decl *D = Group[i]) {
      if (DeclaratorDecl *DD = dyn_cast<DeclaratorDecl>(D))
        if (!FirstDeclaratorInGroup)
          FirstDeclaratorInGroup = DD;
      Decls.push_back(D);
    }

  if (DeclSpec::isDeclRep(DS.getTypeSpecType())) {
    if (TagDecl *Tag = dyn_cast_or_null<TagDecl>(DS.getRepAsDecl())) {
      handleTagNumbering(Tag, S);
      if (!Tag->hasNameForLinkage() && !Tag->hasDeclaratorForAnonDecl())
        Tag->setDeclaratorForAnonDecl(FirstDeclaratorInGroup);
    }
  }

  return BuildDeclaratorGroup(Decls, DS.containsPlaceholderType());
}

/// BuildDeclaratorGroup - convert a list of declarations into a declaration
/// group, performing any necessary semantic checking.
Sema::DeclGroupPtrTy
Sema::BuildDeclaratorGroup(MutableArrayRef<Decl *> Group,
                           bool TypeMayContainAuto) {
  // C++0x [dcl.spec.auto]p7:
  //   If the type deduced for the template parameter U is not the same in each
  //   deduction, the program is ill-formed.
  // FIXME: When initializer-list support is added, a distinction is needed
  // between the deduced type U and the deduced type which 'auto' stands for.
  //   auto a = 0, b = { 1, 2, 3 };
  // is legal because the deduced type U is 'int' in both cases.
  if (TypeMayContainAuto && Group.size() > 1) {
    QualType Deduced;
    CanQualType DeducedCanon;
    VarDecl *DeducedDecl = nullptr;
    for (unsigned i = 0, e = Group.size(); i != e; ++i) {
      if (VarDecl *D = dyn_cast<VarDecl>(Group[i])) {
        AutoType *AT = D->getType()->getContainedAutoType();
        // Don't reissue diagnostics when instantiating a template.
        if (AT && D->isInvalidDecl())
          break;
        QualType U = AT ? AT->getDeducedType() : QualType();
        if (!U.isNull()) {
          CanQualType UCanon = Context.getCanonicalType(U);
          if (Deduced.isNull()) {
            Deduced = U;
            DeducedCanon = UCanon;
            DeducedDecl = D;
          } else if (DeducedCanon != UCanon) {
            Diag(D->getTypeSourceInfo()->getTypeLoc().getBeginLoc(),
                 diag::err_auto_different_deductions)
              << (AT->isDecltypeAuto() ? 1 : 0)
              << Deduced << DeducedDecl->getDeclName()
              << U << D->getDeclName()
              << DeducedDecl->getInit()->getSourceRange()
              << D->getInit()->getSourceRange();
            D->setInvalidDecl();
            break;
          }
        }
      }
    }
  }

  ActOnDocumentableDecls(Group);

  return DeclGroupPtrTy::make(
      DeclGroupRef::Create(Context, Group.data(), Group.size()));
}

void Sema::ActOnDocumentableDecl(Decl *D) {
  ActOnDocumentableDecls(D);
}

void Sema::ActOnDocumentableDecls(ArrayRef<Decl *> Group) {
  // Don't parse the comment if Doxygen diagnostics are ignored.
  if (Group.empty() || !Group[0])
    return;

  if (Diags.isIgnored(diag::warn_doc_param_not_found,
                      Group[0]->getLocation()) &&
      Diags.isIgnored(diag::warn_unknown_comment_command_name,
                      Group[0]->getLocation()))
    return;

  if (Group.size() >= 2) {
    // This is a decl group.  Normally it will contain only declarations
    // produced from declarator list.  But in case we have any definitions or
    // additional declaration references:
    //   'typedef struct S {} S;'
    //   'typedef struct S *S;'
    //   'struct S *pS;'
    // FinalizeDeclaratorGroup adds these as separate declarations.
    Decl *MaybeTagDecl = Group[0];
    if (MaybeTagDecl && isa<TagDecl>(MaybeTagDecl)) {
      Group = Group.slice(1);
    }
  }

  // See if there are any new comments that are not attached to a decl.
  ArrayRef<RawComment *> Comments = Context.getRawCommentList().getComments();
  if (!Comments.empty() &&
      !Comments.back()->isAttached()) {
    // There is at least one comment that not attached to a decl.
    // Maybe it should be attached to one of these decls?
    //
    // Note that this way we pick up not only comments that precede the
    // declaration, but also comments that *follow* the declaration -- thanks to
    // the lookahead in the lexer: we've consumed the semicolon and looked
    // ahead through comments.
    for (unsigned i = 0, e = Group.size(); i != e; ++i)
      Context.getCommentForDecl(Group[i], &PP);
  }
}

/// ActOnParamDeclarator - Called from Parser::ParseFunctionDeclarator()
/// to introduce parameters into function prototype scope.
Decl *Sema::ActOnParamDeclarator(Scope *S, Declarator &D) {
  const DeclSpec &DS = D.getDeclSpec();

  // Verify C99 6.7.5.3p2: The only SCS allowed is 'register'.

  // C++03 [dcl.stc]p2 also permits 'auto'.
  StorageClass SC = SC_None;
  if (DS.getStorageClassSpec() == DeclSpec::SCS_register) {
    SC = SC_Register;
  } else if (getLangOpts().CPlusPlus &&
             DS.getStorageClassSpec() == DeclSpec::SCS_auto) {
    SC = SC_Auto;
  } else if (DS.getStorageClassSpec() != DeclSpec::SCS_unspecified) {
    Diag(DS.getStorageClassSpecLoc(),
         diag::err_invalid_storage_class_in_func_decl);
    D.getMutableDeclSpec().ClearStorageClassSpecs();
  }

  if (DeclSpec::TSCS TSCS = DS.getThreadStorageClassSpec())
    Diag(DS.getThreadStorageClassSpecLoc(), diag::err_invalid_thread)
      << DeclSpec::getSpecifierName(TSCS);
  if (DS.isConstexprSpecified())
    Diag(DS.getConstexprSpecLoc(), diag::err_invalid_constexpr)
      << 0;

  DiagnoseFunctionSpecifiers(DS);

  TypeSourceInfo *TInfo = GetTypeForDeclarator(D, S);
  QualType parmDeclType = TInfo->getType();

  if (getLangOpts().CPlusPlus) {
    // Check that there are no default arguments inside the type of this
    // parameter.
    CheckExtraCXXDefaultArguments(D);
    
    // Parameter declarators cannot be qualified (C++ [dcl.meaning]p1).
    if (D.getCXXScopeSpec().isSet()) {
      Diag(D.getIdentifierLoc(), diag::err_qualified_param_declarator)
        << D.getCXXScopeSpec().getRange();
      D.getCXXScopeSpec().clear();
    }
  }

  // Ensure we have a valid name
  IdentifierInfo *II = nullptr;
  if (D.hasName()) {
    II = D.getIdentifier();
    if (!II) {
      Diag(D.getIdentifierLoc(), diag::err_bad_parameter_name)
        << GetNameForDeclarator(D).getName();
      D.setInvalidType(true);
    }
  }

  // Check for redeclaration of parameters, e.g. int foo(int x, int x);
  if (II) {
    LookupResult R(*this, II, D.getIdentifierLoc(), LookupOrdinaryName,
                   ForRedeclaration);
    LookupName(R, S);
    if (R.isSingleResult()) {
      NamedDecl *PrevDecl = R.getFoundDecl();
      if (PrevDecl->isTemplateParameter()) {
        // Maybe we will complain about the shadowed template parameter.
        DiagnoseTemplateParameterShadow(D.getIdentifierLoc(), PrevDecl);
        // Just pretend that we didn't see the previous declaration.
        PrevDecl = nullptr;
      } else if (S->isDeclScope(PrevDecl)) {
        Diag(D.getIdentifierLoc(), diag::err_param_redefinition) << II;
        Diag(PrevDecl->getLocation(), diag::note_previous_declaration);

        // Recover by removing the name
        II = nullptr;
        D.SetIdentifier(nullptr, D.getIdentifierLoc());
        D.setInvalidType(true);
      }
    }
  }

  // HLSL Change Starts
  if (getLangOpts().HLSL) {
    const bool IsParm = true;
    if (hlsl::IsStringType(parmDeclType)) {
      static const unsigned selectParamIdx = 1;
      Diag(D.getLocStart(), diag::err_hlsl_unsupported_string_decl) << selectParamIdx;
      D.setInvalidType();
    }
    if (!DiagnoseHLSLDecl(D, Context.getTranslationUnitDecl(), /*BitWidth*/ nullptr, TInfo, IsParm)) {
      assert(D.isInvalidType() && "otherwise DiagnoseHLSLDecl failed but "
                                  "didn't invalidate declaration");
    }
  }
  // HLSL Change Ends

  // Temporarily put parameter variables in the translation unit, not
  // the enclosing context.  This prevents them from accidentally
  // looking like class members in C++.
  ParmVarDecl *New = CheckParameter(Context.getTranslationUnitDecl(),
                                    D.getLocStart(),
                                    D.getIdentifierLoc(), II,
                                    parmDeclType, TInfo,
                                    SC, hlsl::ParamModFromAttributeList(DS.getAttributes().getList())); // HLSL Change

  if (D.isInvalidType())
    New->setInvalidDecl();

  TransferUnusualAttributes(D, New); // HLSL Change

  assert(S->isFunctionPrototypeScope());
  assert(S->getFunctionPrototypeDepth() >= 1);
  New->setScopeInfo(S->getFunctionPrototypeDepth() - 1,
                    S->getNextFunctionPrototypeIndex());
  
  // Add the parameter declaration into this scope.
  S->AddDecl(New);
  if (II)
    IdResolver.AddDecl(New);

  ProcessDeclAttributes(S, New, D);

  if (D.getDeclSpec().isModulePrivateSpecified())
    Diag(New->getLocation(), diag::err_module_private_local)
      << 1 << New->getDeclName()
      << SourceRange(D.getDeclSpec().getModulePrivateSpecLoc())
      << FixItHint::CreateRemoval(D.getDeclSpec().getModulePrivateSpecLoc());

  if (New->hasAttr<BlocksAttr>()) {
    Diag(New->getLocation(), diag::err_block_on_nonlocal);
  }
  return New;
}

/// \brief Synthesizes a variable for a parameter arising from a
/// typedef.
ParmVarDecl *Sema::BuildParmVarDeclForTypedef(DeclContext *DC,
                                              SourceLocation Loc,
                                              QualType T) {
  /* FIXME: setting StartLoc == Loc.
     Would it be worth to modify callers so as to provide proper source
     location for the unnamed parameters, embedding the parameter's type? */
  ParmVarDecl *Param = ParmVarDecl::Create(Context, DC, Loc, Loc, nullptr,
                                T, Context.getTrivialTypeSourceInfo(T, Loc),
                                           SC_None, nullptr);
  Param->setImplicit();
  return Param;
}

void Sema::DiagnoseUnusedParameters(ParmVarDecl * const *Param,
                                    ParmVarDecl * const *ParamEnd) {
  // Don't diagnose unused-parameter errors in template instantiations; we
  // will already have done so in the template itself.
  if (!ActiveTemplateInstantiations.empty())
    return;

  for (; Param != ParamEnd; ++Param) {
    if (!(*Param)->isReferenced() && (*Param)->getDeclName() &&
        !(*Param)->hasAttr<UnusedAttr>()) {
      Diag((*Param)->getLocation(), diag::warn_unused_parameter)
        << (*Param)->getDeclName();
    }
  }
}

void Sema::DiagnoseSizeOfParametersAndReturnValue(ParmVarDecl * const *Param,
                                                  ParmVarDecl * const *ParamEnd,
                                                  QualType ReturnTy,
                                                  NamedDecl *D) {
  if (LangOpts.NumLargeByValueCopy == 0) // No check.
    return;

  // Warn if the return value is pass-by-value and larger than the specified
  // threshold.
  if (!ReturnTy->isDependentType() && ReturnTy.isPODType(Context)) {
    unsigned Size = Context.getTypeSizeInChars(ReturnTy).getQuantity();
    if (Size > LangOpts.NumLargeByValueCopy)
      Diag(D->getLocation(), diag::warn_return_value_size)
          << D->getDeclName() << Size;
  }

  // Warn if any parameter is pass-by-value and larger than the specified
  // threshold.
  for (; Param != ParamEnd; ++Param) {
    QualType T = (*Param)->getType();
    if (T->isDependentType() || !T.isPODType(Context))
      continue;
    unsigned Size = Context.getTypeSizeInChars(T).getQuantity();
    if (Size > LangOpts.NumLargeByValueCopy)
      Diag((*Param)->getLocation(), diag::warn_parameter_size)
          << (*Param)->getDeclName() << Size;
  }
}

ParmVarDecl *Sema::CheckParameter(DeclContext *DC, SourceLocation StartLoc,
                                  SourceLocation NameLoc, IdentifierInfo *Name,
                                  QualType T, TypeSourceInfo *TSInfo,
                                  StorageClass SC, hlsl::ParameterModifier ParamMod) { // HLSL Change - add param mod
  // In ARC, infer a lifetime qualifier for appropriate parameter types.
  if (getLangOpts().ObjCAutoRefCount &&
      T.getObjCLifetime() == Qualifiers::OCL_None &&
      T->isObjCLifetimeType()) {

    Qualifiers::ObjCLifetime lifetime;

    // Special cases for arrays:
    //   - if it's const, use __unsafe_unretained
    //   - otherwise, it's an error
    if (T->isArrayType()) {
      if (!T.isConstQualified()) {
        DelayedDiagnostics.add(
            sema::DelayedDiagnostic::makeForbiddenType(
            NameLoc, diag::err_arc_array_param_no_ownership, T, false));
      }
      lifetime = Qualifiers::OCL_ExplicitNone;
    } else {
      lifetime = T->getObjCARCImplicitLifetime();
    }
    T = Context.getLifetimeQualifiedType(T, lifetime);
  }

  ParmVarDecl *New = ParmVarDecl::Create(Context, DC, StartLoc, NameLoc, Name,
                                         Context.getAdjustedParameterType(T), 
                                         TSInfo, SC, nullptr, ParamMod);

  // Parameters can not be abstract class types.
  // For record types, this is done by the AbstractClassUsageDiagnoser once
  // the class has been completely parsed.
  if (!CurContext->isRecord() &&
      RequireNonAbstractType(NameLoc, T, diag::err_abstract_type_in_decl,
                             AbstractParamType))
    New->setInvalidDecl();

  // Parameter declarators cannot be interface types. All ObjC objects are
  // passed by reference.
  if (T->isObjCObjectType()) {
    SourceLocation TypeEndLoc = TSInfo->getTypeLoc().getLocEnd();
    Diag(NameLoc,
         diag::err_object_cannot_be_passed_returned_by_value) << 1 << T
      << FixItHint::CreateInsertion(TypeEndLoc, "*");
    T = Context.getObjCObjectPointerType(T);
    New->setType(T);
  }

  // ISO/IEC TR 18037 S6.7.3: "The type of an object with automatic storage 
  // duration shall not be qualified by an address-space qualifier."
  // Since all parameters have automatic store duration, they can not have
  // an address space.
  if (T.getAddressSpace() != 0) {
    // OpenCL allows function arguments declared to be an array of a type
    // to be qualified with an address space.
    if (!(getLangOpts().OpenCL && T->isArrayType())) {
      Diag(NameLoc, diag::err_arg_with_address_space);
      New->setInvalidDecl();
    }
  }   

  return New;
}

void Sema::ActOnFinishKNRParamDeclarations(Scope *S, Declarator &D,
                                           SourceLocation LocAfterDecls) {
  DeclaratorChunk::FunctionTypeInfo &FTI = D.getFunctionTypeInfo();

  // Verify 6.9.1p6: 'every identifier in the identifier list shall be declared'
  // for a K&R function.
  if (!FTI.hasPrototype) {
    for (int i = FTI.NumParams; i != 0; /* decrement in loop */) {
      --i;
      if (FTI.Params[i].Param == nullptr) {
        SmallString<256> Code;
        llvm::raw_svector_ostream(Code)
            << "  int " << FTI.Params[i].Ident->getName() << ";\n";
        Diag(FTI.Params[i].IdentLoc, diag::ext_param_not_declared)
            << FTI.Params[i].Ident
            << FixItHint::CreateInsertion(LocAfterDecls, Code);

        // Implicitly declare the argument as type 'int' for lack of a better
        // type.
        AttributeFactory attrs;
        DeclSpec DS(attrs);
        const char* PrevSpec; // unused
        unsigned DiagID; // unused
        DS.SetTypeSpecType(DeclSpec::TST_int, FTI.Params[i].IdentLoc, PrevSpec,
                           DiagID, Context.getPrintingPolicy());
        // Use the identifier location for the type source range.
        DS.SetRangeStart(FTI.Params[i].IdentLoc);
        DS.SetRangeEnd(FTI.Params[i].IdentLoc);
        Declarator ParamD(DS, Declarator::KNRTypeListContext);
        ParamD.SetIdentifier(FTI.Params[i].Ident, FTI.Params[i].IdentLoc);
        FTI.Params[i].Param = ActOnParamDeclarator(S, ParamD);
      }
    }
  }
}

Decl *Sema::ActOnStartOfFunctionDef(Scope *FnBodyScope, Declarator &D) {
  assert(getCurFunctionDecl() == nullptr && "Function parsing confused");
  assert(D.isFunctionDeclarator() && "Not a function declarator!");
  Scope *ParentScope = FnBodyScope->getParent();

  D.setFunctionDefinitionKind(FDK_Definition);
  Decl *DP = HandleDeclarator(ParentScope, D, MultiTemplateParamsArg());
  return ActOnStartOfFunctionDef(FnBodyScope, DP);
}

void Sema::ActOnFinishInlineMethodDef(CXXMethodDecl *D) {
  Consumer.HandleInlineMethodDefinition(D);
}

static bool ShouldWarnAboutMissingPrototype(const FunctionDecl *FD, 
                             const FunctionDecl*& PossibleZeroParamPrototype) {
  // Don't warn about invalid declarations.
  if (FD->isInvalidDecl())
    return false;

  // Or declarations that aren't global.
  if (!FD->isGlobal())
    return false;

  // Don't warn about C++ member functions.
  if (isa<CXXMethodDecl>(FD))
    return false;

  // Don't warn about 'main'.
  if (FD->isMain())
    return false;

  // Don't warn about inline functions.
  if (FD->isInlined())
    return false;

  // Don't warn about function templates.
  if (FD->getDescribedFunctionTemplate())
    return false;

  // Don't warn about function template specializations.
  if (FD->isFunctionTemplateSpecialization())
    return false;

  // Don't warn for OpenCL kernels.
  if (FD->hasAttr<OpenCLKernelAttr>())
    return false;

  // Don't warn on explicitly deleted functions.
  if (FD->isDeleted())
    return false;

  bool MissingPrototype = true;
  for (const FunctionDecl *Prev = FD->getPreviousDecl();
       Prev; Prev = Prev->getPreviousDecl()) {
    // Ignore any declarations that occur in function or method
    // scope, because they aren't visible from the header.
    if (Prev->getLexicalDeclContext()->isFunctionOrMethod())
      continue;

    MissingPrototype = !Prev->getType()->isFunctionProtoType();
    if (FD->getNumParams() == 0)
      PossibleZeroParamPrototype = Prev;
    break;
  }

  return MissingPrototype;
}

void
Sema::CheckForFunctionRedefinition(FunctionDecl *FD,
                                   const FunctionDecl *EffectiveDefinition) {
  // Don't complain if we're in GNU89 mode and the previous definition
  // was an extern inline function.
  const FunctionDecl *Definition = EffectiveDefinition;
  if (!Definition)
    if (!FD->isDefined(Definition))
      return;

  if (canRedefineFunction(Definition, getLangOpts()))
    return;

  // If we don't have a visible definition of the function, and it's inline or
  // a template, it's OK to form another definition of it.
  //
  // FIXME: Should we skip the body of the function and use the old definition
  // in this case? That may be necessary for functions that return local types
  // through a deduced return type, or instantiate templates with local types.
  if (!hasVisibleDefinition(Definition) &&
      (Definition->getFormalLinkage() == InternalLinkage ||
       Definition->isInlined() ||
       Definition->getDescribedFunctionTemplate() ||
       Definition->getNumTemplateParameterLists()))
    return;

  if (getLangOpts().GNUMode && Definition->isInlineSpecified() &&
      Definition->getStorageClass() == SC_Extern)
    Diag(FD->getLocation(), diag::err_redefinition_extern_inline)
        << FD->getDeclName() << getLangOpts().CPlusPlus;
  else
    Diag(FD->getLocation(), diag::err_redefinition) << FD->getDeclName();

  Diag(Definition->getLocation(), diag::note_previous_definition);
  FD->setInvalidDecl();
}


static void RebuildLambdaScopeInfo(CXXMethodDecl *CallOperator, 
                                   Sema &S) {
  CXXRecordDecl *const LambdaClass = CallOperator->getParent();
  
  LambdaScopeInfo *LSI = S.PushLambdaScope();
  LSI->CallOperator = CallOperator;
  LSI->Lambda = LambdaClass;
  LSI->ReturnType = CallOperator->getReturnType();
  const LambdaCaptureDefault LCD = LambdaClass->getLambdaCaptureDefault();

  if (LCD == LCD_None)
    LSI->ImpCaptureStyle = CapturingScopeInfo::ImpCap_None;
  else if (LCD == LCD_ByCopy)
    LSI->ImpCaptureStyle = CapturingScopeInfo::ImpCap_LambdaByval;
  else if (LCD == LCD_ByRef)
    LSI->ImpCaptureStyle = CapturingScopeInfo::ImpCap_LambdaByref;
  DeclarationNameInfo DNI = CallOperator->getNameInfo();
    
  LSI->IntroducerRange = DNI.getCXXOperatorNameRange(); 
  LSI->Mutable = !CallOperator->isConst();

  // Add the captures to the LSI so they can be noted as already
  // captured within tryCaptureVar. 
  auto I = LambdaClass->field_begin();
  for (const auto &C : LambdaClass->captures()) {
    if (C.capturesVariable()) {
      VarDecl *VD = C.getCapturedVar();
      if (VD->isInitCapture())
        S.CurrentInstantiationScope->InstantiatedLocal(VD, VD);
      QualType CaptureType = VD->getType();
      const bool ByRef = C.getCaptureKind() == LCK_ByRef;
      LSI->addCapture(VD, /*IsBlock*/false, ByRef, 
          /*RefersToEnclosingVariableOrCapture*/true, C.getLocation(),
          /*EllipsisLoc*/C.isPackExpansion() 
                         ? C.getEllipsisLoc() : SourceLocation(),
          CaptureType, /*Expr*/ nullptr);

    } else if (C.capturesThis()) {
      LSI->addThisCapture(/*Nested*/ false, C.getLocation(), 
                              S.getCurrentThisType(), /*Expr*/ nullptr);
    } else {
      LSI->addVLATypeCapture(C.getLocation(), I->getType());
    }
    ++I;
  }
}

Decl *Sema::ActOnStartOfFunctionDef(Scope *FnBodyScope, Decl *D) {
  // Clear the last template instantiation error context.
  LastTemplateInstantiationErrorContext = ActiveTemplateInstantiation();
  
  if (!D)
    return D;
  FunctionDecl *FD = nullptr;

  if (FunctionTemplateDecl *FunTmpl = dyn_cast<FunctionTemplateDecl>(D))
    FD = FunTmpl->getTemplatedDecl();
  else
    FD = cast<FunctionDecl>(D);
  // If we are instantiating a generic lambda call operator, push
  // a LambdaScopeInfo onto the function stack.  But use the information
  // that's already been calculated (ActOnLambdaExpr) to prime the current 
  // LambdaScopeInfo.  
  // When the template operator is being specialized, the LambdaScopeInfo,
  // has to be properly restored so that tryCaptureVariable doesn't try
  // and capture any new variables. In addition when calculating potential
  // captures during transformation of nested lambdas, it is necessary to 
  // have the LSI properly restored. 
  if (isGenericLambdaCallOperatorSpecialization(FD)) {
    assert(ActiveTemplateInstantiations.size() &&
      "There should be an active template instantiation on the stack " 
      "when instantiating a generic lambda!");
    RebuildLambdaScopeInfo(cast<CXXMethodDecl>(D), *this);
  }
  else
    // Enter a new function scope
    PushFunctionScope();

  // See if this is a redefinition.
  if (!FD->isLateTemplateParsed())
    CheckForFunctionRedefinition(FD);

  // Builtin functions cannot be defined.
  if (unsigned BuiltinID = FD->getBuiltinID()) {
    if (!Context.BuiltinInfo.isPredefinedLibFunction(BuiltinID) &&
        !Context.BuiltinInfo.isPredefinedRuntimeFunction(BuiltinID)) {
      Diag(FD->getLocation(), diag::err_builtin_definition) << FD;
      FD->setInvalidDecl();
    }
  }

  // The return type of a function definition must be complete
  // (C99 6.9.1p3, C++ [dcl.fct]p6).
  QualType ResultType = FD->getReturnType();
  if (!ResultType->isDependentType() && !ResultType->isVoidType() &&
      !FD->isInvalidDecl() &&
      RequireCompleteType(FD->getLocation(), ResultType,
                          diag::err_func_def_incomplete_result))
    FD->setInvalidDecl();

  if (FnBodyScope)
    PushDeclContext(FnBodyScope, FD);

  // Check the validity of our function parameters
  CheckParmsForFunctionDef(FD->param_begin(), FD->param_end(),
                           /*CheckParameterNames=*/true);

  // Introduce our parameters into the function scope
  for (auto Param : FD->params()) {
    Param->setOwningFunction(FD);

    // If this has an identifier, add it to the scope stack.
    if (Param->getIdentifier() && FnBodyScope) {
      CheckShadow(FnBodyScope, Param);

      PushOnScopeChains(Param, FnBodyScope);
    }
  }

  // If we had any tags defined in the function prototype,
  // introduce them into the function scope.
  if (FnBodyScope) {
    for (ArrayRef<NamedDecl *>::iterator
             I = FD->getDeclsInPrototypeScope().begin(),
             E = FD->getDeclsInPrototypeScope().end();
         I != E; ++I) {
      NamedDecl *D = *I;

      // Some of these decls (like enums) may have been pinned to the
      // translation unit for lack of a real context earlier. If so, remove
      // from the translation unit and reattach to the current context.
      if (D->getLexicalDeclContext() == Context.getTranslationUnitDecl()) {
        // Is the decl actually in the context?
        for (const auto *DI : Context.getTranslationUnitDecl()->decls()) {
          if (DI == D) {  
            Context.getTranslationUnitDecl()->removeDecl(D);
            break;
          }
        }
        // Either way, reassign the lexical decl context to our FunctionDecl.
        D->setLexicalDeclContext(CurContext);
      }

      // If the decl has a non-null name, make accessible in the current scope.
      if (!D->getName().empty())
        PushOnScopeChains(D, FnBodyScope, /*AddToContext=*/false);

      // Similarly, dive into enums and fish their constants out, making them
      // accessible in this scope.
      if (auto *ED = dyn_cast<EnumDecl>(D)) {
        for (auto *EI : ED->enumerators())
          PushOnScopeChains(EI, FnBodyScope, /*AddToContext=*/false);
      }
    }
  }

  // Ensure that the function's exception specification is instantiated.
  if (const FunctionProtoType *FPT = FD->getType()->getAs<FunctionProtoType>())
    ResolveExceptionSpec(D->getLocation(), FPT);

  // dllimport cannot be applied to non-inline function definitions.
  if (FD->hasAttr<DLLImportAttr>() && !FD->isInlined() &&
      !FD->isTemplateInstantiation()) {
    assert(!FD->hasAttr<DLLExportAttr>());
    Diag(FD->getLocation(), diag::err_attribute_dllimport_function_definition);
    FD->setInvalidDecl();
    return D;
  }
  // We want to attach documentation to original Decl (which might be
  // a function template).
  ActOnDocumentableDecl(D);
  if (getCurLexicalContext()->isObjCContainer() &&
      getCurLexicalContext()->getDeclKind() != Decl::ObjCCategoryImpl &&
      getCurLexicalContext()->getDeclKind() != Decl::ObjCImplementation)
    Diag(FD->getLocation(), diag::warn_function_def_in_objc_container);
    
  return D;
}

/// \brief Given the set of return statements within a function body,
/// compute the variables that are subject to the named return value 
/// optimization.
///
/// Each of the variables that is subject to the named return value 
/// optimization will be marked as NRVO variables in the AST, and any
/// return statement that has a marked NRVO variable as its NRVO candidate can
/// use the named return value optimization.
///
/// This function applies a very simplistic algorithm for NRVO: if every return
/// statement in the scope of a variable has the same NRVO candidate, that
/// candidate is an NRVO variable.
void Sema::computeNRVO(Stmt *Body, FunctionScopeInfo *Scope) {
  ReturnStmt **Returns = Scope->Returns.data();

  for (unsigned I = 0, E = Scope->Returns.size(); I != E; ++I) {
    if (const VarDecl *NRVOCandidate = Returns[I]->getNRVOCandidate()) {
      if (!NRVOCandidate->isNRVOVariable())
        Returns[I]->setNRVOCandidate(nullptr);
    }
  }
}

bool Sema::canDelayFunctionBody(const Declarator &D) {
  // We can't delay parsing the body of a constexpr function template (yet).
  if (D.getDeclSpec().isConstexprSpecified())
    return false;

  // We can't delay parsing the body of a function template with a deduced
  // return type (yet).
  if (D.getDeclSpec().containsPlaceholderType()) {
    // If the placeholder introduces a non-deduced trailing return type,
    // we can still delay parsing it.
    if (D.getNumTypeObjects()) {
      const auto &Outer = D.getTypeObject(D.getNumTypeObjects() - 1);
      if (Outer.Kind == DeclaratorChunk::Function &&
          Outer.Fun.hasTrailingReturnType()) {
        QualType Ty = GetTypeFromParser(Outer.Fun.getTrailingReturnType());
        return Ty.isNull() || !Ty->isUndeducedType();
      }
    }
    return false;
  }

  return true;
}

bool Sema::canSkipFunctionBody(Decl *D) {
  // We cannot skip the body of a function (or function template) which is
  // constexpr, since we may need to evaluate its body in order to parse the
  // rest of the file.
  // We cannot skip the body of a function with an undeduced return type,
  // because any callers of that function need to know the type.
  if (const FunctionDecl *FD = D->getAsFunction())
    if (FD->isConstexpr() || FD->getReturnType()->isUndeducedType())
      return false;
  return Consumer.shouldSkipFunctionBody(D);
}

Decl *Sema::ActOnSkippedFunctionBody(Decl *Decl) {
  if (FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(Decl))
    FD->setHasSkippedBody();
  else if (ObjCMethodDecl *MD = dyn_cast_or_null<ObjCMethodDecl>(Decl))
    MD->setHasSkippedBody();
  return ActOnFinishFunctionBody(Decl, nullptr);
}

Decl *Sema::ActOnFinishFunctionBody(Decl *D, Stmt *BodyArg) {
  return ActOnFinishFunctionBody(D, BodyArg, false);
}

Decl *Sema::ActOnFinishFunctionBody(Decl *dcl, Stmt *Body,
                                    bool IsInstantiation) {
  FunctionDecl *FD = dcl ? dcl->getAsFunction() : nullptr;

  sema::AnalysisBasedWarnings::Policy WP = AnalysisWarnings.getDefaultPolicy();
  sema::AnalysisBasedWarnings::Policy *ActivePolicy = nullptr;

  if (FD) {
    FD->setBody(Body);

    if (getLangOpts().CPlusPlus14 && !FD->isInvalidDecl() && Body &&
        !FD->isDependentContext() && FD->getReturnType()->isUndeducedType()) {
      // If the function has a deduced result type but contains no 'return'
      // statements, the result type as written must be exactly 'auto', and
      // the deduced result type is 'void'.
      if (!FD->getReturnType()->getAs<AutoType>()) {
        Diag(dcl->getLocation(), diag::err_auto_fn_no_return_but_not_auto)
            << FD->getReturnType();
        FD->setInvalidDecl();
      } else {
        // Substitute 'void' for the 'auto' in the type.
        TypeLoc ResultType = getReturnTypeLoc(FD);
        Context.adjustDeducedFunctionResultType(
            FD, SubstAutoType(ResultType.getType(), Context.VoidTy));
      }
    }
#if 0 // HLSL Change Start - no lambda support
    else if (getLangOpts().CPlusPlus11 && isLambdaCallOperator(FD)) {
      auto *LSI = getCurLambda();
      if (LSI->HasImplicitReturnType) {
        deduceClosureReturnType(*LSI);

        // C++11 [expr.prim.lambda]p4:
        //   [...] if there are no return statements in the compound-statement
        //   [the deduced type is] the type void
        QualType RetType =
            LSI->ReturnType.isNull() ? Context.VoidTy : LSI->ReturnType;

        // Update the return type to the deduced type.
        const FunctionProtoType *Proto =
            FD->getType()->getAs<FunctionProtoType>();
        FD->setType(Context.getFunctionType(RetType, Proto->getParamTypes(),
                                            Proto->getExtProtoInfo()));
      }
    }
#endif // HLSL Change Start - no lambda support

    // The only way to be included in UndefinedButUsed is if there is an
    // ODR use before the definition. Avoid the expensive map lookup if this
    // is the first declaration.
    if (!FD->isFirstDecl() && FD->getPreviousDecl()->isUsed()) {
      if (!FD->isExternallyVisible())
        UndefinedButUsed.erase(FD);
      else if (FD->isInlined() &&
               !LangOpts.GNUInline &&
               (!FD->getPreviousDecl()->hasAttr<GNUInlineAttr>()))
        UndefinedButUsed.erase(FD);
    }

    // If the function implicitly returns zero (like 'main') or is naked,
    // don't complain about missing return statements.
    if (FD->hasImplicitReturnZero() || FD->hasAttr<NakedAttr>())
      WP.disableCheckFallThrough();

    // MSVC permits the use of pure specifier (=0) on function definition,
    // defined at class scope, warn about this non-standard construct.
    if (getLangOpts().MicrosoftExt && FD->isPure() && FD->isCanonicalDecl())
      Diag(FD->getLocation(), diag::ext_pure_function_definition);

    if (!FD->isInvalidDecl()) {
      // Don't diagnose unused parameters of defaulted or deleted functions.
      if (!FD->isDeleted() && !FD->isDefaulted())
        DiagnoseUnusedParameters(FD->param_begin(), FD->param_end());
      DiagnoseSizeOfParametersAndReturnValue(FD->param_begin(), FD->param_end(),
                                             FD->getReturnType(), FD);

      // If this is a structor, we need a vtable.
      if (CXXConstructorDecl *Constructor = dyn_cast<CXXConstructorDecl>(FD))
        MarkVTableUsed(FD->getLocation(), Constructor->getParent());
      else if (CXXDestructorDecl *Destructor = dyn_cast<CXXDestructorDecl>(FD))
        MarkVTableUsed(FD->getLocation(), Destructor->getParent());
      
      // Try to apply the named return value optimization. We have to check
      // if we can do this here because lambdas keep return statements around
      // to deduce an implicit return type.
      if (getLangOpts().CPlusPlus && FD->getReturnType()->isRecordType() &&
          !FD->isDependentContext())
        computeNRVO(Body, getCurFunction());
    }

    // GNU warning -Wmissing-prototypes:
    //   Warn if a global function is defined without a previous
    //   prototype declaration. This warning is issued even if the
    //   definition itself provides a prototype. The aim is to detect
    //   global functions that fail to be declared in header files.
    const FunctionDecl *PossibleZeroParamPrototype = nullptr;
    if (ShouldWarnAboutMissingPrototype(FD, PossibleZeroParamPrototype)) {
      Diag(FD->getLocation(), diag::warn_missing_prototype) << FD;

      if (PossibleZeroParamPrototype) {
        // We found a declaration that is not a prototype,
        // but that could be a zero-parameter prototype
        if (TypeSourceInfo *TI =
                PossibleZeroParamPrototype->getTypeSourceInfo()) {
          TypeLoc TL = TI->getTypeLoc();
          if (FunctionNoProtoTypeLoc FTL = TL.getAs<FunctionNoProtoTypeLoc>())
            Diag(PossibleZeroParamPrototype->getLocation(),
                 diag::note_declaration_not_a_prototype)
                << PossibleZeroParamPrototype
                << FixItHint::CreateInsertion(FTL.getRParenLoc(), "void");
        }
      }
    }

    if (auto *MD = dyn_cast<CXXMethodDecl>(FD)) {
      const CXXMethodDecl *KeyFunction;
      if (MD->isOutOfLine() && (MD = MD->getCanonicalDecl()) &&
          MD->isVirtual() &&
          (KeyFunction = Context.getCurrentKeyFunction(MD->getParent())) &&
          MD == KeyFunction->getCanonicalDecl()) {
        // Update the key-function state if necessary for this ABI.
        if (FD->isInlined() &&
            !Context.getTargetInfo().getCXXABI().canKeyFunctionBeInline()) {
          Context.setNonKeyFunction(MD);

          // If the newly-chosen key function is already defined, then we
          // need to mark the vtable as used retroactively.
          KeyFunction = Context.getCurrentKeyFunction(MD->getParent());
          const FunctionDecl *Definition;
          if (KeyFunction && KeyFunction->isDefined(Definition))
            MarkVTableUsed(Definition->getLocation(), MD->getParent(), true);
        } else {
          // We just defined they key function; mark the vtable as used.
          MarkVTableUsed(FD->getLocation(), MD->getParent(), true);
        }
      }
    }

    assert((FD == getCurFunctionDecl() || getCurLambda()->CallOperator == FD) &&
           "Function parsing confused");
#if 0 // HLSL Change Starts
  } else if (ObjCMethodDecl *MD = dyn_cast_or_null<ObjCMethodDecl>(dcl)) {
    assert(MD == getCurMethodDecl() && "Method parsing confused");
    MD->setBody(Body);
    if (!MD->isInvalidDecl()) {
      DiagnoseUnusedParameters(MD->param_begin(), MD->param_end());
      DiagnoseSizeOfParametersAndReturnValue(MD->param_begin(), MD->param_end(),
                                             MD->getReturnType(), MD);

      if (Body)
        computeNRVO(Body, getCurFunction());
    }
    if (getCurFunction()->ObjCShouldCallSuper) {
      Diag(MD->getLocEnd(), diag::warn_objc_missing_super_call)
        << MD->getSelector().getAsString();
      getCurFunction()->ObjCShouldCallSuper = false;
    }
    if (getCurFunction()->ObjCWarnForNoDesignatedInitChain) {
      const ObjCMethodDecl *InitMethod = nullptr;
      bool isDesignated =
          MD->isDesignatedInitializerForTheInterface(&InitMethod);
      assert(isDesignated && InitMethod);
      (void)isDesignated;

      auto superIsNSObject = [&](const ObjCMethodDecl *MD) {
        auto IFace = MD->getClassInterface();
        if (!IFace)
          return false;
        auto SuperD = IFace->getSuperClass();
        if (!SuperD)
          return false;
        return SuperD->getIdentifier() ==
            NSAPIObj->getNSClassId(NSAPI::ClassId_NSObject);
      };
      // Don't issue this warning for unavailable inits or direct subclasses
      // of NSObject.
      if (!MD->isUnavailable() && !superIsNSObject(MD)) {
        Diag(MD->getLocation(),
             diag::warn_objc_designated_init_missing_super_call);
        Diag(InitMethod->getLocation(),
             diag::note_objc_designated_init_marked_here);
      }
      getCurFunction()->ObjCWarnForNoDesignatedInitChain = false;
    }
    if (getCurFunction()->ObjCWarnForNoInitDelegation) {
      // Don't issue this warning for unavaialable inits.
      if (!MD->isUnavailable())
        Diag(MD->getLocation(),
             diag::warn_objc_secondary_init_missing_init_call);
      getCurFunction()->ObjCWarnForNoInitDelegation = false;
    }
#endif // HLSL Change Ends
  } else {
    return nullptr;
  }

  assert(!getCurFunction()->ObjCShouldCallSuper &&
         "This should only be set for ObjC methods, which should have been "
         "handled in the block above.");

  // Verify and clean out per-function state.
  if (Body && (!FD || !FD->isDefaulted())) {
    // C++ constructors that have function-try-blocks can't have return
    // statements in the handlers of that block. (C++ [except.handle]p14)
    // Verify this.
    if (FD && isa<CXXConstructorDecl>(FD) && isa<CXXTryStmt>(Body))
      DiagnoseReturnInConstructorExceptionHandler(cast<CXXTryStmt>(Body));
    
    // Verify that gotos and switch cases don't jump into scopes illegally.
    if (getCurFunction()->NeedsScopeChecking() &&
        !PP.isCodeCompletionEnabled())
      DiagnoseInvalidJumps(Body);

    if (CXXDestructorDecl *Destructor = dyn_cast<CXXDestructorDecl>(dcl)) {
      if (!Destructor->getParent()->isDependentType())
        CheckDestructor(Destructor);

      MarkBaseAndMemberDestructorsReferenced(Destructor->getLocation(),
                                             Destructor->getParent());
    }
    
    // If any errors have occurred, clear out any temporaries that may have
    // been leftover. This ensures that these temporaries won't be picked up for
    // deletion in some later function.
    if (getDiagnostics().hasErrorOccurred() ||
        getDiagnostics().getSuppressAllDiagnostics()) {
      DiscardCleanupsInEvaluationContext();
    }
    if (!getDiagnostics().hasUncompilableErrorOccurred() &&
        !isa<FunctionTemplateDecl>(dcl)) {
      // Since the body is valid, issue any analysis-based warnings that are
      // enabled.
      ActivePolicy = &WP;
    }

    if (!IsInstantiation && FD && FD->isConstexpr() && !FD->isInvalidDecl() &&
        (!CheckConstexprFunctionDecl(FD) ||
         !CheckConstexprFunctionBody(FD, Body)))
      FD->setInvalidDecl();

    if (FD && FD->hasAttr<NakedAttr>()) {
      for (const Stmt *S : Body->children()) {
        if (!isa<AsmStmt>(S) && !isa<NullStmt>(S)) {
          Diag(S->getLocStart(), diag::err_non_asm_stmt_in_naked_function);
          Diag(FD->getAttr<NakedAttr>()->getLocation(), diag::note_attribute);
          FD->setInvalidDecl();
          break;
        }
      }
    }

    assert(ExprCleanupObjects.size() ==
               ExprEvalContexts.back().NumCleanupObjects &&
           "Leftover temporaries in function");
    assert(!ExprNeedsCleanups && "Unaccounted cleanups in function");
    assert(MaybeODRUseExprs.empty() &&
           "Leftover expressions for odr-use checking");
  }
  
  if (!IsInstantiation)
    PopDeclContext();

  PopFunctionScopeInfo(ActivePolicy, dcl);
  // If any errors have occurred, clear out any temporaries that may have
  // been leftover. This ensures that these temporaries won't be picked up for
  // deletion in some later function.
  if (getDiagnostics().hasErrorOccurred()) {
    DiscardCleanupsInEvaluationContext();
  }

  return dcl;
}


/// When we finish delayed parsing of an attribute, we must attach it to the
/// relevant Decl.
void Sema::ActOnFinishDelayedAttribute(Scope *S, Decl *D,
                                       ParsedAttributes &Attrs) {
  // Always attach attributes to the underlying decl.
  if (TemplateDecl *TD = dyn_cast<TemplateDecl>(D))
    D = TD->getTemplatedDecl();
  ProcessDeclAttributeList(S, D, Attrs.getList());  
  
  if (CXXMethodDecl *Method = dyn_cast_or_null<CXXMethodDecl>(D))
    if (Method->isStatic())
      checkThisInStaticMemberFunctionAttributes(Method);
}


/// ImplicitlyDefineFunction - An undeclared identifier was used in a function
/// call, forming a call to an implicitly defined function (per C99 6.5.1p2).
NamedDecl *Sema::ImplicitlyDefineFunction(SourceLocation Loc,
                                          IdentifierInfo &II, Scope *S) {
  // Before we produce a declaration for an implicitly defined
  // function, see whether there was a locally-scoped declaration of
  // this name as a function or variable. If so, use that
  // (non-visible) declaration, and complain about it.
  if (NamedDecl *ExternCPrev = findLocallyScopedExternCDecl(&II)) {
    Diag(Loc, diag::warn_use_out_of_scope_declaration) << ExternCPrev;
    Diag(ExternCPrev->getLocation(), diag::note_previous_declaration);
    return ExternCPrev;
  }

  // Extension in C99.  Legal in C90, but warn about it.
  unsigned diag_id;
  if (II.getName().startswith("__builtin_"))
    diag_id = diag::warn_builtin_unknown;
  else if (getLangOpts().C99)
    diag_id = diag::ext_implicit_function_decl;
  else
    diag_id = diag::warn_implicit_function_decl;
  Diag(Loc, diag_id) << &II;

  // Because typo correction is expensive, only do it if the implicit
  // function declaration is going to be treated as an error.
  if (Diags.getDiagnosticLevel(diag_id, Loc) >= DiagnosticsEngine::Error) {
    TypoCorrection Corrected;
    if (S &&
        (Corrected = CorrectTypo(
             DeclarationNameInfo(&II, Loc), LookupOrdinaryName, S, nullptr,
             llvm::make_unique<DeclFilterCCC<FunctionDecl>>(), CTK_NonError)))
      diagnoseTypo(Corrected, PDiag(diag::note_function_suggestion),
                   /*ErrorRecovery*/false);
  }

  // Set a Declarator for the implicit definition: int foo();
  const char *Dummy;
  AttributeFactory attrFactory;
  DeclSpec DS(attrFactory);
  unsigned DiagID;
  bool Error = DS.SetTypeSpecType(DeclSpec::TST_int, Loc, Dummy, DiagID,
                                  Context.getPrintingPolicy());
  (void)Error; // Silence warning.
  assert(!Error && "Error setting up implicit decl!");
  SourceLocation NoLoc;
  Declarator D(DS, Declarator::BlockContext);
  D.AddTypeInfo(DeclaratorChunk::getFunction(/*HasProto=*/false,
                                             /*IsAmbiguous=*/false,
                                             /*LParenLoc=*/NoLoc,
                                             /*Params=*/nullptr,
                                             /*NumParams=*/0,
                                             /*EllipsisLoc=*/NoLoc,
                                             /*RParenLoc=*/NoLoc,
                                             /*TypeQuals=*/0,
                                             /*RefQualifierIsLvalueRef=*/true,
                                             /*RefQualifierLoc=*/NoLoc,
                                             /*ConstQualifierLoc=*/NoLoc,
                                             /*VolatileQualifierLoc=*/NoLoc,
                                             /*RestrictQualifierLoc=*/NoLoc,
                                             /*MutableLoc=*/NoLoc,
                                             EST_None,
                                             /*ESpecLoc=*/NoLoc,
                                             /*Exceptions=*/nullptr,
                                             /*ExceptionRanges=*/nullptr,
                                             /*NumExceptions=*/0,
                                             /*NoexceptExpr=*/nullptr,
                                             /*ExceptionSpecTokens=*/nullptr,
                                             Loc, Loc, D),
                DS.getAttributes(),
                SourceLocation());
  D.SetIdentifier(&II, Loc);

  // Insert this function into translation-unit scope.

  DeclContext *PrevDC = CurContext;
  CurContext = Context.getTranslationUnitDecl();

  FunctionDecl *FD = cast<FunctionDecl>(ActOnDeclarator(TUScope, D));
  FD->setImplicit();

  CurContext = PrevDC;

  AddKnownFunctionAttributes(FD);

  return FD;
}

/// \brief Adds any function attributes that we know a priori based on
/// the declaration of this function.
///
/// These attributes can apply both to implicitly-declared builtins
/// (like __builtin___printf_chk) or to library-declared functions
/// like NSLog or printf.
///
/// We need to check for duplicate attributes both here and where user-written
/// attributes are applied to declarations.
void Sema::AddKnownFunctionAttributes(FunctionDecl *FD) {
  if (FD->isInvalidDecl())
    return;

  // If this is a built-in function, map its builtin attributes to
  // actual attributes.
  if (unsigned BuiltinID = FD->getBuiltinID()) {
    // Handle printf-formatting attributes.
    unsigned FormatIdx;
    bool HasVAListArg;
    if (Context.BuiltinInfo.isPrintfLike(BuiltinID, FormatIdx, HasVAListArg)) {
      if (!FD->hasAttr<FormatAttr>()) {
        const char *fmt = "printf";
        unsigned int NumParams = FD->getNumParams();
        if (FormatIdx < NumParams && // NumParams may be 0 (e.g. vfprintf)
            FD->getParamDecl(FormatIdx)->getType()->isObjCObjectPointerType())
          fmt = "NSString";
        FD->addAttr(FormatAttr::CreateImplicit(Context,
                                               &Context.Idents.get(fmt),
                                               FormatIdx+1,
                                               HasVAListArg ? 0 : FormatIdx+2,
                                               FD->getLocation()));
      }
    }
    if (Context.BuiltinInfo.isScanfLike(BuiltinID, FormatIdx,
                                             HasVAListArg)) {
     if (!FD->hasAttr<FormatAttr>())
       FD->addAttr(FormatAttr::CreateImplicit(Context,
                                              &Context.Idents.get("scanf"),
                                              FormatIdx+1,
                                              HasVAListArg ? 0 : FormatIdx+2,
                                              FD->getLocation()));
    }

    // Mark const if we don't care about errno and that is the only
    // thing preventing the function from being const. This allows
    // IRgen to use LLVM intrinsics for such functions.
    if (!getLangOpts().MathErrno &&
        Context.BuiltinInfo.isConstWithoutErrno(BuiltinID)) {
      if (!FD->hasAttr<ConstAttr>())
        FD->addAttr(ConstAttr::CreateImplicit(Context, FD->getLocation()));
    }

    if (Context.BuiltinInfo.isReturnsTwice(BuiltinID) &&
        !FD->hasAttr<ReturnsTwiceAttr>())
      FD->addAttr(ReturnsTwiceAttr::CreateImplicit(Context,
                                         FD->getLocation()));
    if (Context.BuiltinInfo.isNoThrow(BuiltinID) && !FD->hasAttr<NoThrowAttr>())
      FD->addAttr(NoThrowAttr::CreateImplicit(Context, FD->getLocation()));
    if (Context.BuiltinInfo.isConst(BuiltinID) && !FD->hasAttr<ConstAttr>())
      FD->addAttr(ConstAttr::CreateImplicit(Context, FD->getLocation()));
  }

  IdentifierInfo *Name = FD->getIdentifier();
  if (!Name)
    return;
  if ((!getLangOpts().CPlusPlus &&
       FD->getDeclContext()->isTranslationUnit()) ||
      (isa<LinkageSpecDecl>(FD->getDeclContext()) &&
       cast<LinkageSpecDecl>(FD->getDeclContext())->getLanguage() ==
       LinkageSpecDecl::lang_c)) {
    // Okay: this could be a libc/libm/Objective-C function we know
    // about.
  } else
    return;

  if (Name->isStr("asprintf") || Name->isStr("vasprintf")) {
    // FIXME: asprintf and vasprintf aren't C99 functions. Should they be
    // target-specific builtins, perhaps?
    if (!FD->hasAttr<FormatAttr>())
      FD->addAttr(FormatAttr::CreateImplicit(Context,
                                             &Context.Idents.get("printf"), 2,
                                             Name->isStr("vasprintf") ? 0 : 3,
                                             FD->getLocation()));
  }

  if (Name->isStr("__CFStringMakeConstantString")) {
    // We already have a __builtin___CFStringMakeConstantString,
    // but builds that use -fno-constant-cfstrings don't go through that.
    if (!FD->hasAttr<FormatArgAttr>())
      FD->addAttr(FormatArgAttr::CreateImplicit(Context, 1,
                                                FD->getLocation()));
  }
}

TypedefDecl *Sema::ParseTypedefDecl(Scope *S, Declarator &D, QualType T,
                                    TypeSourceInfo *TInfo) {
  assert(D.getIdentifier() && "Wrong callback for declspec without declarator");
  assert(!T.isNull() && "GetTypeForDeclarator() returned null type");

  if (!TInfo) {
    assert(D.isInvalidType() && "no declarator info for valid type");
    TInfo = Context.getTrivialTypeSourceInfo(T);
  }

  // Scope manipulation handled by caller.
  TypedefDecl *NewTD = TypedefDecl::Create(Context, CurContext,
                                           D.getLocStart(),
                                           D.getIdentifierLoc(),
                                           D.getIdentifier(),
                                           TInfo);

  // Bail out immediately if we have an invalid declaration.
  if (D.isInvalidType()) {
    NewTD->setInvalidDecl();
    return NewTD;
  }
  
  if (D.getDeclSpec().isModulePrivateSpecified()) {
    if (CurContext->isFunctionOrMethod())
      Diag(NewTD->getLocation(), diag::err_module_private_local)
        << 2 << NewTD->getDeclName()
        << SourceRange(D.getDeclSpec().getModulePrivateSpecLoc())
        << FixItHint::CreateRemoval(D.getDeclSpec().getModulePrivateSpecLoc());
    else
      NewTD->setModulePrivate();
  }
  
  // C++ [dcl.typedef]p8:
  //   If the typedef declaration defines an unnamed class (or
  //   enum), the first typedef-name declared by the declaration
  //   to be that class type (or enum type) is used to denote the
  //   class type (or enum type) for linkage purposes only.
  // We need to check whether the type was declared in the declaration.
  switch (D.getDeclSpec().getTypeSpecType()) {
  case TST_enum:
  case TST_struct:
  case TST_interface:
  case TST_union:
  case TST_class: {
    TagDecl *tagFromDeclSpec = cast<TagDecl>(D.getDeclSpec().getRepAsDecl());
    setTagNameForLinkagePurposes(tagFromDeclSpec, NewTD);
    break;
  }

  default:
    break;
  }

  return NewTD;
}


/// \brief Check that this is a valid underlying type for an enum declaration.
bool Sema::CheckEnumUnderlyingType(TypeSourceInfo *TI) {
  SourceLocation UnderlyingLoc = TI->getTypeLoc().getBeginLoc();
  QualType T = TI->getType();

  if (T->isDependentType())
    return false;

  if (const BuiltinType *BT = T->getAs<BuiltinType>())
    if (BT->isInteger())
      return false;

  Diag(UnderlyingLoc, diag::err_enum_invalid_underlying) << T;
  return true;
}

/// Check whether this is a valid redeclaration of a previous enumeration.
/// \return true if the redeclaration was invalid.
bool Sema::CheckEnumRedeclaration(SourceLocation EnumLoc, bool IsScoped,
                                  QualType EnumUnderlyingTy,
                                  const EnumDecl *Prev) {
  bool IsFixed = !EnumUnderlyingTy.isNull();

  if (IsScoped != Prev->isScoped()) {
    Diag(EnumLoc, diag::err_enum_redeclare_scoped_mismatch)
      << Prev->isScoped();
    Diag(Prev->getLocation(), diag::note_previous_declaration);
    return true;
  }

  if (IsFixed && Prev->isFixed()) {
    if (!EnumUnderlyingTy->isDependentType() &&
        !Prev->getIntegerType()->isDependentType() &&
        !Context.hasSameUnqualifiedType(EnumUnderlyingTy,
                                        Prev->getIntegerType())) {
      // TODO: Highlight the underlying type of the redeclaration.
      Diag(EnumLoc, diag::err_enum_redeclare_type_mismatch)
        << EnumUnderlyingTy << Prev->getIntegerType();
      Diag(Prev->getLocation(), diag::note_previous_declaration)
          << Prev->getIntegerTypeRange();
      return true;
    }
  } else if (IsFixed != Prev->isFixed()) {
    Diag(EnumLoc, diag::err_enum_redeclare_fixed_mismatch)
      << Prev->isFixed();
    Diag(Prev->getLocation(), diag::note_previous_declaration);
    return true;
  }

  return false;
}

/// \brief Get diagnostic %select index for tag kind for
/// redeclaration diagnostic message.
/// WARNING: Indexes apply to particular diagnostics only!
///
/// \returns diagnostic %select index.
static unsigned getRedeclDiagFromTagKind(TagTypeKind Tag) {
  switch (Tag) {
  case TTK_Struct: return 0;
  case TTK_Interface: return 1;
  case TTK_Class:  return 2;
  default: llvm_unreachable("Invalid tag kind for redecl diagnostic!");
  }
}

/// \brief Determine if tag kind is a class-key compatible with
/// class for redeclaration (class, struct, or __interface).
///
/// \returns true iff the tag kind is compatible.
static bool isClassCompatTagKind(TagTypeKind Tag)
{
  return Tag == TTK_Struct || Tag == TTK_Class || Tag == TTK_Interface;
}

/// \brief Determine whether a tag with a given kind is acceptable
/// as a redeclaration of the given tag declaration.
///
/// \returns true if the new tag kind is acceptable, false otherwise.
bool Sema::isAcceptableTagRedeclaration(const TagDecl *Previous,
                                        TagTypeKind NewTag, bool isDefinition,
                                        SourceLocation NewTagLoc,
                                        const IdentifierInfo *Name) {
  // C++ [dcl.type.elab]p3:
  //   The class-key or enum keyword present in the
  //   elaborated-type-specifier shall agree in kind with the
  //   declaration to which the name in the elaborated-type-specifier
  //   refers. This rule also applies to the form of
  //   elaborated-type-specifier that declares a class-name or
  //   friend class since it can be construed as referring to the
  //   definition of the class. Thus, in any
  //   elaborated-type-specifier, the enum keyword shall be used to
  //   refer to an enumeration (7.2), the union class-key shall be
  //   used to refer to a union (clause 9), and either the class or
  //   struct class-key shall be used to refer to a class (clause 9)
  //   declared using the class or struct class-key.
  TagTypeKind OldTag = Previous->getTagKind();
  if (!isDefinition || !isClassCompatTagKind(NewTag))
    if (OldTag == NewTag)
      return true;

  if (isClassCompatTagKind(OldTag) && isClassCompatTagKind(NewTag)) {
    // Warn about the struct/class tag mismatch.
    bool isTemplate = false;
    if (const CXXRecordDecl *Record = dyn_cast<CXXRecordDecl>(Previous))
      isTemplate = Record->getDescribedClassTemplate();

    if (!ActiveTemplateInstantiations.empty()) {
      // In a template instantiation, do not offer fix-its for tag mismatches
      // since they usually mess up the template instead of fixing the problem.
      Diag(NewTagLoc, diag::warn_struct_class_tag_mismatch)
        << getRedeclDiagFromTagKind(NewTag) << isTemplate << Name
        << getRedeclDiagFromTagKind(OldTag);
      return true;
    }

    if (isDefinition) {
      // On definitions, check previous tags and issue a fix-it for each
      // one that doesn't match the current tag.
      if (Previous->getDefinition()) {
        // Don't suggest fix-its for redefinitions.
        return true;
      }

      bool previousMismatch = false;
      for (auto I : Previous->redecls()) {
        if (I->getTagKind() != NewTag) {
          if (!previousMismatch) {
            previousMismatch = true;
            Diag(NewTagLoc, diag::warn_struct_class_previous_tag_mismatch)
              << getRedeclDiagFromTagKind(NewTag) << isTemplate << Name
              << getRedeclDiagFromTagKind(I->getTagKind());
          }
          Diag(I->getInnerLocStart(), diag::note_struct_class_suggestion)
            << getRedeclDiagFromTagKind(NewTag)
            << FixItHint::CreateReplacement(I->getInnerLocStart(),
                 TypeWithKeyword::getTagTypeKindName(NewTag));
        }
      }
      return true;
    }

    // Check for a previous definition.  If current tag and definition
    // are same type, do nothing.  If no definition, but disagree with
    // with previous tag type, give a warning, but no fix-it.
    const TagDecl *Redecl = Previous->getDefinition() ?
                            Previous->getDefinition() : Previous;
    if (Redecl->getTagKind() == NewTag) {
      return true;
    }

    Diag(NewTagLoc, diag::warn_struct_class_tag_mismatch)
      << getRedeclDiagFromTagKind(NewTag) << isTemplate << Name
      << getRedeclDiagFromTagKind(OldTag);
    Diag(Redecl->getLocation(), diag::note_previous_use);

    // If there is a previous definition, suggest a fix-it.
    if (Previous->getDefinition()) {
        Diag(NewTagLoc, diag::note_struct_class_suggestion)
          << getRedeclDiagFromTagKind(Redecl->getTagKind())
          << FixItHint::CreateReplacement(SourceRange(NewTagLoc),
               TypeWithKeyword::getTagTypeKindName(Redecl->getTagKind()));
    }

    return true;
  }
  return false;
}

/// Add a minimal nested name specifier fixit hint to allow lookup of a tag name
/// from an outer enclosing namespace or file scope inside a friend declaration.
/// This should provide the commented out code in the following snippet:
///   namespace N {
///     struct X;
///     namespace M {
///       struct Y { friend struct /*N::*/ X; };
///     }
///   }
static FixItHint createFriendTagNNSFixIt(Sema &SemaRef, NamedDecl *ND, Scope *S,
                                         SourceLocation NameLoc) {
  // While the decl is in a namespace, do repeated lookup of that name and see
  // if we get the same namespace back.  If we do not, continue until
  // translation unit scope, at which point we have a fully qualified NNS.
  SmallVector<IdentifierInfo *, 4> Namespaces;
  DeclContext *DC = ND->getDeclContext()->getRedeclContext();
  for (; !DC->isTranslationUnit(); DC = DC->getParent()) {
    // This tag should be declared in a namespace, which can only be enclosed by
    // other namespaces.  Bail if there's an anonymous namespace in the chain.
    NamespaceDecl *Namespace = dyn_cast<NamespaceDecl>(DC);
    if (!Namespace || Namespace->isAnonymousNamespace())
      return FixItHint();
    IdentifierInfo *II = Namespace->getIdentifier();
    Namespaces.push_back(II);
    NamedDecl *Lookup = SemaRef.LookupSingleName(
        S, II, NameLoc, Sema::LookupNestedNameSpecifierName);
    if (Lookup == Namespace)
      break;
  }

  // Once we have all the namespaces, reverse them to go outermost first, and
  // build an NNS.
  SmallString<64> Insertion;
  llvm::raw_svector_ostream OS(Insertion);
  if (DC->isTranslationUnit())
    OS << "::";
  std::reverse(Namespaces.begin(), Namespaces.end());
  for (auto *II : Namespaces)
    OS << II->getName() << "::";
  OS.flush();
  return FixItHint::CreateInsertion(NameLoc, Insertion);
}

/// \brief Determine whether a tag originally declared in context \p OldDC can
/// be redeclared with an unqualfied name in \p NewDC (assuming name lookup
/// found a declaration in \p OldDC as a previous decl, perhaps through a
/// using-declaration).
static bool isAcceptableTagRedeclContext(Sema &S, DeclContext *OldDC,
                                         DeclContext *NewDC) {
  OldDC = OldDC->getRedeclContext();
  NewDC = NewDC->getRedeclContext();

  if (OldDC->Equals(NewDC))
    return true;

  // In MSVC mode, we allow a redeclaration if the contexts are related (either
  // encloses the other).
  if (S.getLangOpts().MSVCCompat &&
      (OldDC->Encloses(NewDC) || NewDC->Encloses(OldDC)))
    return true;

  return false;
}

/// \brief This is invoked when we see 'struct foo' or 'struct {'.  In the
/// former case, Name will be non-null.  In the later case, Name will be null.
/// TagSpec indicates what kind of tag this is. TUK indicates whether this is a
/// reference/declaration/definition of a tag.
///
/// \param IsTypeSpecifier \c true if this is a type-specifier (or
/// trailing-type-specifier) other than one in an alias-declaration.
///
/// \param SkipBody If non-null, will be set to indicate if the caller should
/// skip the definition of this tag and treat it as if it were a declaration.
Decl *Sema::ActOnTag(Scope *S, unsigned TagSpec, TagUseKind TUK,
                     SourceLocation KWLoc, CXXScopeSpec &SS,
                     IdentifierInfo *Name, SourceLocation NameLoc,
                     AttributeList *Attr, AccessSpecifier AS,
                     SourceLocation ModulePrivateLoc,
                     MultiTemplateParamsArg TemplateParameterLists,
                     bool &OwnedDecl, bool &IsDependent,
                     SourceLocation ScopedEnumKWLoc,
                     bool ScopedEnumUsesClassTag,
                     TypeResult UnderlyingType,
                     bool IsTypeSpecifier, SkipBodyInfo *SkipBody) {
  // If this is not a definition, it must have a name.
  IdentifierInfo *OrigName = Name;
  assert((Name != nullptr || TUK == TUK_Definition) &&
         "Nameless record must be a definition!");
  assert(TemplateParameterLists.size() == 0 || TUK != TUK_Reference);

  OwnedDecl = false;
  TagTypeKind Kind = TypeWithKeyword::getTagTypeKindForTypeSpec(TagSpec);
  bool ScopedEnum = ScopedEnumKWLoc.isValid();

  // FIXME: Check explicit specializations more carefully.
  bool isExplicitSpecialization = false;
  bool Invalid = false;

  // We only need to do this matching if we have template parameters
  // or a scope specifier, which also conveniently avoids this work
  // for non-C++ cases.
  if (TemplateParameterLists.size() > 0 ||
      (SS.isNotEmpty() && TUK != TUK_Reference)) {
    if (TemplateParameterList *TemplateParams =
            MatchTemplateParametersToScopeSpecifier(
                KWLoc, NameLoc, SS, nullptr, TemplateParameterLists,
                TUK == TUK_Friend, isExplicitSpecialization, Invalid)) {
      if (Kind == TTK_Enum) {
        Diag(KWLoc, diag::err_enum_template);
        return nullptr;
      }

      if (TemplateParams->size() > 0) {
        // This is a declaration or definition of a class template (which may
        // be a member of another template).

        if (Invalid)
          return nullptr;

        OwnedDecl = false;
        DeclResult Result = CheckClassTemplate(S, TagSpec, TUK, KWLoc,
                                               SS, Name, NameLoc, Attr,
                                               TemplateParams, AS,
                                               ModulePrivateLoc,
                                               /*FriendLoc*/SourceLocation(),
                                               TemplateParameterLists.size()-1,
                                               TemplateParameterLists.data(),
                                               SkipBody);
        return Result.get();
      } else {
        // The "template<>" header is extraneous.
        Diag(TemplateParams->getTemplateLoc(), diag::err_template_tag_noparams)
          << TypeWithKeyword::getTagTypeKindName(Kind) << Name;
        isExplicitSpecialization = true;
      }
    }
  }

  // Figure out the underlying type if this a enum declaration. We need to do
  // this early, because it's needed to detect if this is an incompatible
  // redeclaration.
  llvm::PointerUnion<const Type*, TypeSourceInfo*> EnumUnderlying;

  if (Kind == TTK_Enum) {
    if (UnderlyingType.isInvalid() || (!UnderlyingType.get() && ScopedEnum))
      // No underlying type explicitly specified, or we failed to parse the
      // type, default to int.
      EnumUnderlying = Context.IntTy.getTypePtr();
    else if (UnderlyingType.get()) {
      // C++0x 7.2p2: The type-specifier-seq of an enum-base shall name an
      // integral type; any cv-qualification is ignored.
      TypeSourceInfo *TI = nullptr;
      GetTypeFromParser(UnderlyingType.get(), &TI);
      EnumUnderlying = TI;

      if (CheckEnumUnderlyingType(TI))
        // Recover by falling back to int.
        EnumUnderlying = Context.IntTy.getTypePtr();

      if (DiagnoseUnexpandedParameterPack(TI->getTypeLoc().getBeginLoc(), TI,
                                          UPPC_FixedUnderlyingType))
        EnumUnderlying = Context.IntTy.getTypePtr();

    } else if (getLangOpts().MSVCCompat)
      // Microsoft enums are always of int type.
      EnumUnderlying = Context.IntTy.getTypePtr();
  }

  // HLSL Change Starts
  if (getLangOpts().HLSLVersion == hlsl::LangStd::v2015 && TUK == TUK_Declaration) {
    Diag(NameLoc, diag::err_hlsl_unsupported_construct)
        << "struct declaration without definition";
  }
  // HLSL Change Ends

  DeclContext *SearchDC = CurContext;
  DeclContext *DC = CurContext;
  bool isStdBadAlloc = false;

  RedeclarationKind Redecl = ForRedeclaration;
  if (TUK == TUK_Friend || TUK == TUK_Reference)
    Redecl = NotForRedeclaration;

  LookupResult Previous(*this, Name, NameLoc, LookupTagName, Redecl);
  if (Name && SS.isNotEmpty()) {
    // We have a nested-name tag ('struct foo::bar').

    // Check for invalid 'foo::'.
    if (SS.isInvalid()) {
      Name = nullptr;
      goto CreateNewDecl;
    }

    // If this is a friend or a reference to a class in a dependent
    // context, don't try to make a decl for it.
    if (TUK == TUK_Friend || TUK == TUK_Reference) {
      DC = computeDeclContext(SS, false);
      if (!DC) {
        IsDependent = true;
        return nullptr;
      }
    } else {
      DC = computeDeclContext(SS, true);
      if (!DC) {
        Diag(SS.getRange().getBegin(), diag::err_dependent_nested_name_spec)
          << SS.getRange();
        return nullptr;
      }
    }

    if (RequireCompleteDeclContext(SS, DC))
      return nullptr;

    SearchDC = DC;
    // Look-up name inside 'foo::'.
    LookupQualifiedName(Previous, DC);

    if (Previous.isAmbiguous())
      return nullptr;

    if (Previous.empty()) {
      // Name lookup did not find anything. However, if the
      // nested-name-specifier refers to the current instantiation,
      // and that current instantiation has any dependent base
      // classes, we might find something at instantiation time: treat
      // this as a dependent elaborated-type-specifier.
      // But this only makes any sense for reference-like lookups.
      if (Previous.wasNotFoundInCurrentInstantiation() &&
          (TUK == TUK_Reference || TUK == TUK_Friend)) {
        IsDependent = true;
        return nullptr;
      }

      // A tag 'foo::bar' must already exist.
      Diag(NameLoc, diag::err_not_tag_in_scope) 
        << Kind << Name << DC << SS.getRange();
      Name = nullptr;
      Invalid = true;
      goto CreateNewDecl;
    }
  } else if (Name) {
    // C++14 [class.mem]p14:
    //   If T is the name of a class, then each of the following shall have a
    //   name different from T:
    //    -- every member of class T that is itself a type
    if (TUK != TUK_Reference && TUK != TUK_Friend &&
        DiagnoseClassNameShadow(SearchDC, DeclarationNameInfo(Name, NameLoc)))
      return nullptr;

    // If this is a named struct, check to see if there was a previous forward
    // declaration or definition.
    // FIXME: We're looking into outer scopes here, even when we
    // shouldn't be. Doing so can result in ambiguities that we
    // shouldn't be diagnosing.
    LookupName(Previous, S);

    // When declaring or defining a tag, ignore ambiguities introduced
    // by types using'ed into this scope.
    if (Previous.isAmbiguous() && 
        (TUK == TUK_Definition || TUK == TUK_Declaration)) {
      LookupResult::Filter F = Previous.makeFilter();
      while (F.hasNext()) {
        NamedDecl *ND = F.next();
        if (ND->getDeclContext()->getRedeclContext() != SearchDC)
          F.erase();
      }
      F.done();
    }

    // C++11 [namespace.memdef]p3:
    //   If the name in a friend declaration is neither qualified nor
    //   a template-id and the declaration is a function or an
    //   elaborated-type-specifier, the lookup to determine whether
    //   the entity has been previously declared shall not consider
    //   any scopes outside the innermost enclosing namespace.
    //
    // MSVC doesn't implement the above rule for types, so a friend tag
    // declaration may be a redeclaration of a type declared in an enclosing
    // scope.  They do implement this rule for friend functions.
    //
    // Does it matter that this should be by scope instead of by
    // semantic context?
    if (!Previous.empty() && TUK == TUK_Friend) {
      DeclContext *EnclosingNS = SearchDC->getEnclosingNamespaceContext();
      LookupResult::Filter F = Previous.makeFilter();
      bool FriendSawTagOutsideEnclosingNamespace = false;
      while (F.hasNext()) {
        NamedDecl *ND = F.next();
        DeclContext *DC = ND->getDeclContext()->getRedeclContext();
        if (DC->isFileContext() &&
            !EnclosingNS->Encloses(ND->getDeclContext())) {
          if (getLangOpts().MSVCCompat)
            FriendSawTagOutsideEnclosingNamespace = true;
          else
            F.erase();
        }
      }
      F.done();

      // Diagnose this MSVC extension in the easy case where lookup would have
      // unambiguously found something outside the enclosing namespace.
      if (Previous.isSingleResult() && FriendSawTagOutsideEnclosingNamespace) {
        NamedDecl *ND = Previous.getFoundDecl();
        Diag(NameLoc, diag::ext_friend_tag_redecl_outside_namespace)
            << createFriendTagNNSFixIt(*this, ND, S, NameLoc);
      }
    }

    // Note:  there used to be some attempt at recovery here.
    if (Previous.isAmbiguous())
      return nullptr;

    if (!getLangOpts().CPlusPlus && TUK != TUK_Reference) {
      // FIXME: This makes sure that we ignore the contexts associated
      // with C structs, unions, and enums when looking for a matching
      // tag declaration or definition. See the similar lookup tweak
      // in Sema::LookupName; is there a better way to deal with this?
      while (isa<RecordDecl>(SearchDC) || isa<EnumDecl>(SearchDC))
        SearchDC = SearchDC->getParent();
    }
  }

  if (Previous.isSingleResult() &&
      Previous.getFoundDecl()->isTemplateParameter()) {
    // Maybe we will complain about the shadowed template parameter.
    DiagnoseTemplateParameterShadow(NameLoc, Previous.getFoundDecl());
    // Just pretend that we didn't see the previous declaration.
    Previous.clear();
  }

  if (getLangOpts().CPlusPlus && Name && DC && StdNamespace &&
      DC->Equals(getStdNamespace()) && Name->isStr("bad_alloc")) {
    // This is a declaration of or a reference to "std::bad_alloc".
    isStdBadAlloc = true;
    
    if (Previous.empty() && StdBadAlloc) {
      // std::bad_alloc has been implicitly declared (but made invisible to
      // name lookup). Fill in this implicit declaration as the previous 
      // declaration, so that the declarations get chained appropriately.
      Previous.addDecl(getStdBadAlloc());
    }
  }

  // If we didn't find a previous declaration, and this is a reference
  // (or friend reference), move to the correct scope.  In C++, we
  // also need to do a redeclaration lookup there, just in case
  // there's a shadow friend decl.
  if (Name && Previous.empty() &&
      (TUK == TUK_Reference || TUK == TUK_Friend)) {
    if (Invalid) goto CreateNewDecl;
    assert(SS.isEmpty());

    if (TUK == TUK_Reference) {
      // C++ [basic.scope.pdecl]p5:
      //   -- for an elaborated-type-specifier of the form
      //
      //          class-key identifier
      //
      //      if the elaborated-type-specifier is used in the
      //      decl-specifier-seq or parameter-declaration-clause of a
      //      function defined in namespace scope, the identifier is
      //      declared as a class-name in the namespace that contains
      //      the declaration; otherwise, except as a friend
      //      declaration, the identifier is declared in the smallest
      //      non-class, non-function-prototype scope that contains the
      //      declaration.
      //
      // C99 6.7.2.3p8 has a similar (but not identical!) provision for
      // C structs and unions.
      //
      // It is an error in C++ to declare (rather than define) an enum
      // type, including via an elaborated type specifier.  We'll
      // diagnose that later; for now, declare the enum in the same
      // scope as we would have picked for any other tag type.
      //
      // GNU C also supports this behavior as part of its incomplete
      // enum types extension, while GNU C++ does not.
      //
      // Find the context where we'll be declaring the tag.
      // FIXME: We would like to maintain the current DeclContext as the
      // lexical context,
      while (!SearchDC->isFileContext() && !SearchDC->isFunctionOrMethod())
        SearchDC = SearchDC->getParent();

      // Find the scope where we'll be declaring the tag.
      while (S->isClassScope() ||
             (getLangOpts().CPlusPlus &&
              S->isFunctionPrototypeScope()) ||
             ((S->getFlags() & Scope::DeclScope) == 0) ||
             (S->getEntity() && S->getEntity()->isTransparentContext()))
        S = S->getParent();
    } else {
      assert(TUK == TUK_Friend);
      // C++ [namespace.memdef]p3:
      //   If a friend declaration in a non-local class first declares a
      //   class or function, the friend class or function is a member of
      //   the innermost enclosing namespace.
      SearchDC = SearchDC->getEnclosingNamespaceContext();
    }

    // In C++, we need to do a redeclaration lookup to properly
    // diagnose some problems.
    if (getLangOpts().CPlusPlus) {
      Previous.setRedeclarationKind(ForRedeclaration);
      LookupQualifiedName(Previous, SearchDC);
    }
  }

  // If we have a known previous declaration to use, then use it.
  if (Previous.empty() && SkipBody && SkipBody->Previous)
    Previous.addDecl(SkipBody->Previous);

  if (!Previous.empty()) {
    NamedDecl *PrevDecl = Previous.getFoundDecl();
    NamedDecl *DirectPrevDecl = Previous.getRepresentativeDecl();

    // It's okay to have a tag decl in the same scope as a typedef
    // which hides a tag decl in the same scope.  Finding this
    // insanity with a redeclaration lookup can only actually happen
    // in C++.
    //
    // This is also okay for elaborated-type-specifiers, which is
    // technically forbidden by the current standard but which is
    // okay according to the likely resolution of an open issue;
    // see http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_active.html#407
    if (getLangOpts().CPlusPlus) {
      if (TypedefNameDecl *TD = dyn_cast<TypedefNameDecl>(PrevDecl)) {
        if (const TagType *TT = TD->getUnderlyingType()->getAs<TagType>()) {
          TagDecl *Tag = TT->getDecl();
          if (Tag->getDeclName() == Name &&
              Tag->getDeclContext()->getRedeclContext()
                          ->Equals(TD->getDeclContext()->getRedeclContext())) {
            PrevDecl = Tag;
            Previous.clear();
            Previous.addDecl(Tag);
            Previous.resolveKind();
          }
        }
      }
    }

    // If this is a redeclaration of a using shadow declaration, it must
    // declare a tag in the same context. In MSVC mode, we allow a
    // redefinition if either context is within the other.
    if (auto *Shadow = dyn_cast<UsingShadowDecl>(DirectPrevDecl)) {
      auto *OldTag = dyn_cast<TagDecl>(PrevDecl);
      if (SS.isEmpty() && TUK != TUK_Reference && TUK != TUK_Friend &&
          isDeclInScope(Shadow, SearchDC, S, isExplicitSpecialization) &&
          !(OldTag && isAcceptableTagRedeclContext(
                          *this, OldTag->getDeclContext(), SearchDC))) {
        Diag(KWLoc, diag::err_using_decl_conflict_reverse);
        Diag(Shadow->getTargetDecl()->getLocation(),
             diag::note_using_decl_target);
        Diag(Shadow->getUsingDecl()->getLocation(), diag::note_using_decl)
            << 0;
        // Recover by ignoring the old declaration.
        Previous.clear();
        goto CreateNewDecl;
      }
    }

    if (TagDecl *PrevTagDecl = dyn_cast<TagDecl>(PrevDecl)) {
      // If this is a use of a previous tag, or if the tag is already declared
      // in the same scope (so that the definition/declaration completes or
      // rementions the tag), reuse the decl.
      if (TUK == TUK_Reference || TUK == TUK_Friend ||
          isDeclInScope(DirectPrevDecl, SearchDC, S,
                        SS.isNotEmpty() || isExplicitSpecialization)) {
        // Make sure that this wasn't declared as an enum and now used as a
        // struct or something similar.
        if (!isAcceptableTagRedeclaration(PrevTagDecl, Kind,
                                          TUK == TUK_Definition, KWLoc,
                                          Name)) {
          bool SafeToContinue
            = (PrevTagDecl->getTagKind() != TTK_Enum &&
               Kind != TTK_Enum);
          if (SafeToContinue)
            Diag(KWLoc, diag::err_use_with_wrong_tag)
              << Name
              << FixItHint::CreateReplacement(SourceRange(KWLoc),
                                              PrevTagDecl->getKindName());
          else
            Diag(KWLoc, diag::err_use_with_wrong_tag) << Name;
          Diag(PrevTagDecl->getLocation(), diag::note_previous_use);

          if (SafeToContinue)
            Kind = PrevTagDecl->getTagKind();
          else {
            // Recover by making this an anonymous redefinition.
            Name = nullptr;
            Previous.clear();
            Invalid = true;
          }
        }

        if (Kind == TTK_Enum && PrevTagDecl->getTagKind() == TTK_Enum) {
          const EnumDecl *PrevEnum = cast<EnumDecl>(PrevTagDecl);

          // If this is an elaborated-type-specifier for a scoped enumeration,
          // the 'class' keyword is not necessary and not permitted.
          if (TUK == TUK_Reference || TUK == TUK_Friend) {
            if (ScopedEnum)
              Diag(ScopedEnumKWLoc, diag::err_enum_class_reference)
                << PrevEnum->isScoped()
                << FixItHint::CreateRemoval(ScopedEnumKWLoc);
            return PrevTagDecl;
          }

          QualType EnumUnderlyingTy;
          if (TypeSourceInfo *TI = EnumUnderlying.dyn_cast<TypeSourceInfo*>())
            EnumUnderlyingTy = TI->getType().getUnqualifiedType();
          else if (const Type *T = EnumUnderlying.dyn_cast<const Type*>())
            EnumUnderlyingTy = QualType(T, 0);

          // All conflicts with previous declarations are recovered by
          // returning the previous declaration, unless this is a definition,
          // in which case we want the caller to bail out.
          if (CheckEnumRedeclaration(NameLoc.isValid() ? NameLoc : KWLoc,
                                     ScopedEnum, EnumUnderlyingTy, PrevEnum))
            return TUK == TUK_Declaration ? PrevTagDecl : nullptr;
        }

        // C++11 [class.mem]p1:
        //   A member shall not be declared twice in the member-specification,
        //   except that a nested class or member class template can be declared
        //   and then later defined.
        if (TUK == TUK_Declaration && PrevDecl->isCXXClassMember() &&
            S->isDeclScope(PrevDecl)) {
          Diag(NameLoc, diag::ext_member_redeclared);
          Diag(PrevTagDecl->getLocation(), diag::note_previous_declaration);
        }

        if (!Invalid) {
          // If this is a use, just return the declaration we found, unless
          // we have attributes.

          // FIXME: In the future, return a variant or some other clue
          // for the consumer of this Decl to know it doesn't own it.
          // For our current ASTs this shouldn't be a problem, but will
          // need to be changed with DeclGroups.
          if (!Attr &&
              ((TUK == TUK_Reference &&
                (!PrevTagDecl->getFriendObjectKind() || getLangOpts().MicrosoftExt))
               || TUK == TUK_Friend))
            return PrevTagDecl;

          // Diagnose attempts to redefine a tag.
          if (TUK == TUK_Definition) {
            if (NamedDecl *Def = PrevTagDecl->getDefinition()) {
              // If we're defining a specialization and the previous definition
              // is from an implicit instantiation, don't emit an error
              // here; we'll catch this in the general case below.
              bool IsExplicitSpecializationAfterInstantiation = false;
              if (isExplicitSpecialization) {
                if (CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(Def))
                  IsExplicitSpecializationAfterInstantiation =
                    RD->getTemplateSpecializationKind() !=
                    TSK_ExplicitSpecialization;
                else if (EnumDecl *ED = dyn_cast<EnumDecl>(Def))
                  IsExplicitSpecializationAfterInstantiation =
                    ED->getTemplateSpecializationKind() !=
                    TSK_ExplicitSpecialization;
              }

              NamedDecl *Hidden = nullptr;
              if (SkipBody && getLangOpts().CPlusPlus &&
                  !hasVisibleDefinition(Def, &Hidden)) {
                // There is a definition of this tag, but it is not visible. We
                // explicitly make use of C++'s one definition rule here, and
                // assume that this definition is identical to the hidden one
                // we already have. Make the existing definition visible and
                // use it in place of this one.
                SkipBody->ShouldSkip = true;
                makeMergedDefinitionVisible(Hidden, KWLoc);
                return Def;
              } else if (!IsExplicitSpecializationAfterInstantiation) {
                // A redeclaration in function prototype scope in C isn't
                // visible elsewhere, so merely issue a warning.
                if (!getLangOpts().CPlusPlus && S->containedInPrototypeScope())
                  Diag(NameLoc, diag::warn_redefinition_in_param_list) << Name;
                else
                  Diag(NameLoc, diag::err_redefinition) << Name;
                Diag(Def->getLocation(), diag::note_previous_definition);
                // If this is a redefinition, recover by making this
                // struct be anonymous, which will make any later
                // references get the previous definition.
                Name = nullptr;
                Previous.clear();
                Invalid = true;
              }
            } else {
              // If the type is currently being defined, complain
              // about a nested redefinition.
              auto *TD = Context.getTagDeclType(PrevTagDecl)->getAsTagDecl();
              if (TD->isBeingDefined()) {
                Diag(NameLoc, diag::err_nested_redefinition) << Name;
                Diag(PrevTagDecl->getLocation(),
                     diag::note_previous_definition);
                Name = nullptr;
                Previous.clear();
                Invalid = true;
              }
            }

            // Okay, this is definition of a previously declared or referenced
            // tag. We're going to create a new Decl for it.
          }

          // Okay, we're going to make a redeclaration.  If this is some kind
          // of reference, make sure we build the redeclaration in the same DC
          // as the original, and ignore the current access specifier.
          if (TUK == TUK_Friend || TUK == TUK_Reference) {
            SearchDC = PrevTagDecl->getDeclContext();
            AS = AS_none;
          }
        }
        // If we get here we have (another) forward declaration or we
        // have a definition.  Just create a new decl.

      } else {
        // If we get here, this is a definition of a new tag type in a nested
        // scope, e.g. "struct foo; void bar() { struct foo; }", just create a
        // new decl/type.  We set PrevDecl to NULL so that the entities
        // have distinct types.
        Previous.clear();
      }
      // If we get here, we're going to create a new Decl. If PrevDecl
      // is non-NULL, it's a definition of the tag declared by
      // PrevDecl. If it's NULL, we have a new definition.


    // Otherwise, PrevDecl is not a tag, but was found with tag
    // lookup.  This is only actually possible in C++, where a few
    // things like templates still live in the tag namespace.
    } else {
      // Use a better diagnostic if an elaborated-type-specifier
      // found the wrong kind of type on the first
      // (non-redeclaration) lookup.
      if ((TUK == TUK_Reference || TUK == TUK_Friend) &&
          !Previous.isForRedeclaration()) {
        unsigned Kind = 0;
        if (isa<TypedefDecl>(PrevDecl)) Kind = 1;
        else if (isa<TypeAliasDecl>(PrevDecl)) Kind = 2;
        else if (isa<ClassTemplateDecl>(PrevDecl)) Kind = 3;
        Diag(NameLoc, diag::err_tag_reference_non_tag) << Kind;
        Diag(PrevDecl->getLocation(), diag::note_declared_at);
        Invalid = true;

      // Otherwise, only diagnose if the declaration is in scope.
      } else if (!isDeclInScope(DirectPrevDecl, SearchDC, S,
                                SS.isNotEmpty() || isExplicitSpecialization)) {
        // do nothing

      // Diagnose implicit declarations introduced by elaborated types.
      } else if (TUK == TUK_Reference || TUK == TUK_Friend) {
        unsigned Kind = 0;
        if (isa<TypedefDecl>(PrevDecl)) Kind = 1;
        else if (isa<TypeAliasDecl>(PrevDecl)) Kind = 2;
        else if (isa<ClassTemplateDecl>(PrevDecl)) Kind = 3;
        Diag(NameLoc, diag::err_tag_reference_conflict) << Kind;
        Diag(PrevDecl->getLocation(), diag::note_previous_decl) << PrevDecl;
        Invalid = true;

      // Otherwise it's a declaration.  Call out a particularly common
      // case here.
      } else if (TypedefNameDecl *TND = dyn_cast<TypedefNameDecl>(PrevDecl)) {
        unsigned Kind = 0;
        if (isa<TypeAliasDecl>(PrevDecl)) Kind = 1;
        Diag(NameLoc, diag::err_tag_definition_of_typedef)
          << Name << Kind << TND->getUnderlyingType();
        Diag(PrevDecl->getLocation(), diag::note_previous_decl) << PrevDecl;
        Invalid = true;

      // Otherwise, diagnose.
      } else {
        // The tag name clashes with something else in the target scope,
        // issue an error and recover by making this tag be anonymous.
        Diag(NameLoc, diag::err_redefinition_different_kind) << Name;
        Diag(PrevDecl->getLocation(), diag::note_previous_definition);
        Name = nullptr;
        Invalid = true;
      }

      // The existing declaration isn't relevant to us; we're in a
      // new scope, so clear out the previous declaration.
      Previous.clear();
    }
  }

CreateNewDecl:

  TagDecl *PrevDecl = nullptr;
  if (Previous.isSingleResult())
    PrevDecl = cast<TagDecl>(Previous.getFoundDecl());

  // If there is an identifier, use the location of the identifier as the
  // location of the decl, otherwise use the location of the struct/union
  // keyword.
  SourceLocation Loc = NameLoc.isValid() ? NameLoc : KWLoc;

  // Otherwise, create a new declaration. If there is a previous
  // declaration of the same entity, the two will be linked via
  // PrevDecl.
  TagDecl *New;

  bool IsForwardReference = false;
  if (Kind == TTK_Enum) {
    // FIXME: Tag decls should be chained to any simultaneous vardecls, e.g.:
    // enum X { A, B, C } D;    D should chain to X.
    New = EnumDecl::Create(Context, SearchDC, KWLoc, Loc, Name,
                           cast_or_null<EnumDecl>(PrevDecl), ScopedEnum,
                           ScopedEnumUsesClassTag, !EnumUnderlying.isNull());
    // If this is an undefined enum, warn.
    if (TUK != TUK_Definition && !Invalid) {
      TagDecl *Def;
      if ((getLangOpts().CPlusPlus11 || getLangOpts().ObjC2) &&
          cast<EnumDecl>(New)->isFixed()) {
        // C++0x: 7.2p2: opaque-enum-declaration.
        // Conflicts are diagnosed above. Do nothing.
      }
      else if (PrevDecl && (Def = cast<EnumDecl>(PrevDecl)->getDefinition())) {
        Diag(Loc, diag::ext_forward_ref_enum_def)
          << New;
        Diag(Def->getLocation(), diag::note_previous_definition);
      } else {
        unsigned DiagID = diag::ext_forward_ref_enum;
        if (getLangOpts().MSVCCompat)
          DiagID = diag::ext_ms_forward_ref_enum;
        else if (getLangOpts().CPlusPlus)
          DiagID = diag::err_forward_ref_enum;
        Diag(Loc, DiagID);
        
        // If this is a forward-declared reference to an enumeration, make a 
        // note of it; we won't actually be introducing the declaration into
        // the declaration context.
        if (TUK == TUK_Reference)
          IsForwardReference = true;
      }
    }

    if (EnumUnderlying) {
      EnumDecl *ED = cast<EnumDecl>(New);
      if (TypeSourceInfo *TI = EnumUnderlying.dyn_cast<TypeSourceInfo*>())
        ED->setIntegerTypeSourceInfo(TI);
      else
        ED->setIntegerType(QualType(EnumUnderlying.get<const Type*>(), 0));
      ED->setPromotionType(ED->getIntegerType());
    }

  } else {
    // struct/union/class

    // FIXME: Tag decls should be chained to any simultaneous vardecls, e.g.:
    // struct X { int A; } D;    D should chain to X.
    if (getLangOpts().CPlusPlus) {
      // FIXME: Look for a way to use RecordDecl for simple structs.
      New = CXXRecordDecl::Create(Context, Kind, SearchDC, KWLoc, Loc, Name,
                                  cast_or_null<CXXRecordDecl>(PrevDecl));

      if (isStdBadAlloc && (!StdBadAlloc || getStdBadAlloc()->isImplicit()))
        StdBadAlloc = cast<CXXRecordDecl>(New);
    } else
      New = RecordDecl::Create(Context, Kind, SearchDC, KWLoc, Loc, Name,
                               cast_or_null<RecordDecl>(PrevDecl));
  }

  // C++11 [dcl.type]p3:
  //   A type-specifier-seq shall not define a class or enumeration [...].
  if (getLangOpts().CPlusPlus && IsTypeSpecifier && TUK == TUK_Definition) {
    Diag(New->getLocation(), diag::err_type_defined_in_type_specifier)
      << Context.getTagDeclType(New);
    Invalid = true;
  }

  // Maybe add qualifier info.
  if (SS.isNotEmpty()) {
    if (SS.isSet()) {
      // If this is either a declaration or a definition, check the 
      // nested-name-specifier against the current context. We don't do this
      // for explicit specializations, because they have similar checking
      // (with more specific diagnostics) in the call to 
      // CheckMemberSpecialization, below.
      if (!isExplicitSpecialization &&
          (TUK == TUK_Definition || TUK == TUK_Declaration) &&
          diagnoseQualifiedDeclaration(SS, DC, OrigName, Loc))
        Invalid = true;

      New->setQualifierInfo(SS.getWithLocInContext(Context));
      if (TemplateParameterLists.size() > 0) {
        New->setTemplateParameterListsInfo(Context,
                                           TemplateParameterLists.size(),
                                           TemplateParameterLists.data());
      }
    }
    else
      Invalid = true;
  }

  if (RecordDecl *RD = dyn_cast<RecordDecl>(New)) {
    // Add alignment attributes if necessary; these attributes are checked when
    // the ASTContext lays out the structure.
    //
    // It is important for implementing the correct semantics that this
    // happen here (in act on tag decl). The #pragma pack stack is
    // maintained as a result of parser callbacks which can occur at
    // many points during the parsing of a struct declaration (because
    // the #pragma tokens are effectively skipped over during the
    // parsing of the struct).
    if (TUK == TUK_Definition) {
      AddAlignmentAttributesForRecord(RD);
      AddMsStructLayoutForRecord(RD);
    }
  }

  if (ModulePrivateLoc.isValid()) {
    if (isExplicitSpecialization)
      Diag(New->getLocation(), diag::err_module_private_specialization)
        << 2
        << FixItHint::CreateRemoval(ModulePrivateLoc);
    // __module_private__ does not apply to local classes. However, we only
    // diagnose this as an error when the declaration specifiers are
    // freestanding. Here, we just ignore the __module_private__.
    else if (!SearchDC->isFunctionOrMethod())
      New->setModulePrivate();
  }

  // If this is a specialization of a member class (of a class template),
  // check the specialization.
  if (isExplicitSpecialization && CheckMemberSpecialization(New, Previous))
    Invalid = true;

  // If we're declaring or defining a tag in function prototype scope in C,
  // note that this type can only be used within the function and add it to
  // the list of decls to inject into the function definition scope.
  if ((Name || Kind == TTK_Enum) &&
      getNonFieldDeclScope(S)->isFunctionPrototypeScope()) {
    if (getLangOpts().CPlusPlus) {
      // C++ [dcl.fct]p6:
      //   Types shall not be defined in return or parameter types.
      if (TUK == TUK_Definition && !IsTypeSpecifier) {
        Diag(Loc, diag::err_type_defined_in_param_type)
            << Name;
        Invalid = true;
      }
    } else {
      Diag(Loc, diag::warn_decl_in_param_list) << Context.getTagDeclType(New);
    }
    DeclsInPrototypeScope.push_back(New);
  }

  if (Invalid)
    New->setInvalidDecl();

  if (Attr)
    ProcessDeclAttributeList(S, New, Attr);

  // Set the lexical context. If the tag has a C++ scope specifier, the
  // lexical context will be different from the semantic context.
  New->setLexicalDeclContext(CurContext);

  // Mark this as a friend decl if applicable.
  // In Microsoft mode, a friend declaration also acts as a forward
  // declaration so we always pass true to setObjectOfFriendDecl to make
  // the tag name visible.
  if (TUK == TUK_Friend)
    New->setObjectOfFriendDecl(getLangOpts().MSVCCompat);

  // Set the access specifier.
  if (!Invalid && SearchDC->isRecord())
    SetMemberAccessSpecifier(New, PrevDecl, AS);

  if (TUK == TUK_Definition)
    New->startDefinition();

  // If this has an identifier, add it to the scope stack.
  if (TUK == TUK_Friend) {
    // We might be replacing an existing declaration in the lookup tables;
    // if so, borrow its access specifier.
    if (PrevDecl)
      New->setAccess(PrevDecl->getAccess());

    DeclContext *DC = New->getDeclContext()->getRedeclContext();
    DC->makeDeclVisibleInContext(New);
    if (Name) // can be null along some error paths
      if (Scope *EnclosingScope = getScopeForDeclContext(S, DC))
        PushOnScopeChains(New, EnclosingScope, /* AddToContext = */ false);
  } else if (Name) {
    S = getNonFieldDeclScope(S);
    PushOnScopeChains(New, S, !IsForwardReference);
    if (IsForwardReference)
      SearchDC->makeDeclVisibleInContext(New);

  } else {
    CurContext->addDecl(New);
  }

  // If this is the C FILE type, notify the AST context.
  if (IdentifierInfo *II = New->getIdentifier())
    if (!New->isInvalidDecl() &&
        New->getDeclContext()->getRedeclContext()->isTranslationUnit() &&
        II->isStr("FILE"))
      Context.setFILEDecl(New);

  if (PrevDecl)
    mergeDeclAttributes(New, PrevDecl);

  // If there's a #pragma GCC visibility in scope, set the visibility of this
  // record.
  AddPushedVisibilityAttribute(New);

  OwnedDecl = true;
  // In C++, don't return an invalid declaration. We can't recover well from
  // the cases where we make the type anonymous.
  return (Invalid && getLangOpts().CPlusPlus) ? nullptr : New;
}

void Sema::ActOnTagStartDefinition(Scope *S, Decl *TagD) {
  AdjustDeclIfTemplate(TagD);
  TagDecl *Tag = cast<TagDecl>(TagD);
  
  // Enter the tag context.
  PushDeclContext(S, Tag);

  ActOnDocumentableDecl(TagD);

  // If there's a #pragma GCC visibility in scope, set the visibility of this
  // record.
  AddPushedVisibilityAttribute(Tag);
}

Decl *Sema::ActOnObjCContainerStartDefinition(Decl *IDecl) {
  assert(isa<ObjCContainerDecl>(IDecl) && 
         "ActOnObjCContainerStartDefinition - Not ObjCContainerDecl");
  DeclContext *OCD = cast<DeclContext>(IDecl);
  assert(getContainingDC(OCD) == CurContext &&
      "The next DeclContext should be lexically contained in the current one.");
  CurContext = OCD;
  return IDecl;
}

void Sema::ActOnStartCXXMemberDeclarations(Scope *S, Decl *TagD,
                                           SourceLocation FinalLoc,
                                           bool IsFinalSpelledSealed,
                                           SourceLocation LBraceLoc) {
  AdjustDeclIfTemplate(TagD);
  CXXRecordDecl *Record = cast<CXXRecordDecl>(TagD);

  FieldCollector->StartClass();

  if (!Record->getIdentifier())
    return;

  if (FinalLoc.isValid())
    Record->addAttr(new (Context)
                    FinalAttr(FinalLoc, Context, IsFinalSpelledSealed));

  // C++ [class]p2:
  //   [...] The class-name is also inserted into the scope of the
  //   class itself; this is known as the injected-class-name. For
  //   purposes of access checking, the injected-class-name is treated
  //   as if it were a public member name.
  CXXRecordDecl *InjectedClassName
    = CXXRecordDecl::Create(Context, Record->getTagKind(), CurContext,
                            Record->getLocStart(), Record->getLocation(),
                            Record->getIdentifier(),
                            /*PrevDecl=*/nullptr,
                            /*DelayTypeCreation=*/true);
  Context.getTypeDeclType(InjectedClassName, Record);
  InjectedClassName->setImplicit();
  InjectedClassName->setAccess(AS_public);
  if (ClassTemplateDecl *Template = Record->getDescribedClassTemplate())
      InjectedClassName->setDescribedClassTemplate(Template);
  PushOnScopeChains(InjectedClassName, S);
  assert(InjectedClassName->isInjectedClassName() &&
         "Broken injected-class-name");
}

void Sema::ActOnTagFinishDefinition(Scope *S, Decl *TagD,
                                    SourceLocation RBraceLoc) {
  AdjustDeclIfTemplate(TagD);
  TagDecl *Tag = cast<TagDecl>(TagD);
  Tag->setRBraceLoc(RBraceLoc);

  // Make sure we "complete" the definition even it is invalid.
  if (Tag->isBeingDefined()) {
    assert(Tag->isInvalidDecl() && "We should already have completed it");
    if (RecordDecl *RD = dyn_cast<RecordDecl>(Tag))
      RD->completeDefinition();
  }

  if (isa<CXXRecordDecl>(Tag))
    FieldCollector->FinishClass();

  // Exit this scope of this tag's definition.
  PopDeclContext();

  if (getCurLexicalContext()->isObjCContainer() &&
      Tag->getDeclContext()->isFileContext())
    Tag->setTopLevelDeclInObjCContainer();

  // Notify the consumer that we've defined a tag.
  if (!Tag->isInvalidDecl())
    Consumer.HandleTagDeclDefinition(Tag);
}

void Sema::ActOnObjCContainerFinishDefinition() {
  // Exit this scope of this interface definition.
  PopDeclContext();
}

void Sema::ActOnObjCTemporaryExitContainerContext(DeclContext *DC) {
  assert(DC == CurContext && "Mismatch of container contexts");
  OriginalLexicalContext = DC;
  ActOnObjCContainerFinishDefinition();
}

void Sema::ActOnObjCReenterContainerContext(DeclContext *DC) {
  ActOnObjCContainerStartDefinition(cast<Decl>(DC));
  OriginalLexicalContext = nullptr;
}

void Sema::ActOnTagDefinitionError(Scope *S, Decl *TagD) {
  AdjustDeclIfTemplate(TagD);
  TagDecl *Tag = cast<TagDecl>(TagD);
  Tag->setInvalidDecl();

  // Make sure we "complete" the definition even it is invalid.
  if (Tag->isBeingDefined()) {
    if (RecordDecl *RD = dyn_cast<RecordDecl>(Tag))
      RD->completeDefinition();
  }

  // We're undoing ActOnTagStartDefinition here, not
  // ActOnStartCXXMemberDeclarations, so we don't have to mess with
  // the FieldCollector.

  PopDeclContext();  
}

// Note that FieldName may be null for anonymous bitfields.
ExprResult Sema::VerifyBitField(SourceLocation FieldLoc,
                                IdentifierInfo *FieldName,
                                QualType FieldTy, bool IsMsStruct,
                                Expr *BitWidth, bool *ZeroWidth) {
  // Default to true; that shouldn't confuse checks for emptiness
  if (ZeroWidth)
    *ZeroWidth = true;

  // C99 6.7.2.1p4 - verify the field type.
  // C++ 9.6p3: A bit-field shall have integral or enumeration type.
  if (!FieldTy->isDependentType() && !FieldTy->isIntegralOrEnumerationType()) {
    // Handle incomplete types with specific error.
    if (RequireCompleteType(FieldLoc, FieldTy, diag::err_field_incomplete))
      return ExprError();
    if (FieldName)
      return Diag(FieldLoc, diag::err_not_integral_type_bitfield)
        << FieldName << FieldTy << BitWidth->getSourceRange();
    return Diag(FieldLoc, diag::err_not_integral_type_anon_bitfield)
      << FieldTy << BitWidth->getSourceRange();
  } else if (DiagnoseUnexpandedParameterPack(const_cast<Expr *>(BitWidth),
                                             UPPC_BitFieldWidth))
    return ExprError();

  // If the bit-width is type- or value-dependent, don't try to check
  // it now.
  if (BitWidth->isValueDependent() || BitWidth->isTypeDependent())
    return BitWidth;

  llvm::APSInt Value;
  ExprResult ICE = VerifyIntegerConstantExpression(BitWidth, &Value);
  if (ICE.isInvalid())
    return ICE;
  BitWidth = ICE.get();

  if (Value != 0 && ZeroWidth)
    *ZeroWidth = false;

  // Zero-width bitfield is ok for anonymous field.
  if (Value == 0 && FieldName)
    return Diag(FieldLoc, diag::err_bitfield_has_zero_width) << FieldName;

  if (Value.isSigned() && Value.isNegative()) {
    if (FieldName)
      return Diag(FieldLoc, diag::err_bitfield_has_negative_width)
               << FieldName << Value.toString(10);
    return Diag(FieldLoc, diag::err_anon_bitfield_has_negative_width)
      << Value.toString(10);
  }

  if (!FieldTy->isDependentType()) {
    uint64_t TypeSize = Context.getTypeSize(FieldTy);
    if (Value.getZExtValue() > TypeSize) {
      if (!getLangOpts().CPlusPlus || IsMsStruct ||
          Context.getTargetInfo().getCXXABI().isMicrosoft()) {
        if (FieldName) 
          return Diag(FieldLoc, diag::err_bitfield_width_exceeds_type_size)
            << FieldName << (unsigned)Value.getZExtValue() 
            << (unsigned)TypeSize;
        
        return Diag(FieldLoc, diag::err_anon_bitfield_width_exceeds_type_size)
          << (unsigned)Value.getZExtValue() << (unsigned)TypeSize;
      }
      
      if (FieldName)
        Diag(FieldLoc, diag::warn_bitfield_width_exceeds_type_size)
          << FieldName << (unsigned)Value.getZExtValue() 
          << (unsigned)TypeSize;
      else
        Diag(FieldLoc, diag::warn_anon_bitfield_width_exceeds_type_size)
          << (unsigned)Value.getZExtValue() << (unsigned)TypeSize;        
    }
  }

  return BitWidth;
}

/// ActOnField - Each field of a C struct/union is passed into this in order
/// to create a FieldDecl object for it.
Decl *Sema::ActOnField(Scope *S, Decl *TagD, SourceLocation DeclStart,
                       Declarator &D, Expr *BitfieldWidth) {
  FieldDecl *Res = HandleField(S, cast_or_null<RecordDecl>(TagD),
                               DeclStart, D, static_cast<Expr*>(BitfieldWidth),
                               /*InitStyle=*/ICIS_NoInit, AS_public);
  return Res;
}

/// HandleField - Analyze a field of a C struct or a C++ data member.
///
FieldDecl *Sema::HandleField(Scope *S, RecordDecl *Record,
                             SourceLocation DeclStart,
                             Declarator &D, Expr *BitWidth,
                             InClassInitStyle InitStyle,
                             AccessSpecifier AS) {
  IdentifierInfo *II = D.getIdentifier();
  SourceLocation Loc = DeclStart;
  if (II) Loc = D.getIdentifierLoc();

  TypeSourceInfo *TInfo = GetTypeForDeclarator(D, S);
  QualType T = TInfo->getType();
  if (getLangOpts().CPlusPlus) {
    CheckExtraCXXDefaultArguments(D);

    if (DiagnoseUnexpandedParameterPack(D.getIdentifierLoc(), TInfo,
                                        UPPC_DataMemberType)) {
      D.setInvalidType();
      T = Context.IntTy;
      TInfo = Context.getTrivialTypeSourceInfo(T, Loc);
    }
  }

  // HLSL Changes Start
  if (getLangOpts().HLSL) {
    const bool IsParameterFalse = false;
    if (!DiagnoseHLSLDecl(D, CurContext, BitWidth, TInfo, IsParameterFalse)) {
      // Let the diagnostic provide errors, don't actually return nullptr here;
      // compilation will recover, which is helpful because HLSL diagnostics
      // need not interrupt the declaration processing.
    }
  }
  // HLSL Changes End

  // TR 18037 does not allow fields to be declared with address spaces.
  if (T.getQualifiers().hasAddressSpace()) {
    Diag(Loc, diag::err_field_with_address_space);
    D.setInvalidType();
  }

  // OpenCL 1.2 spec, s6.9 r:
  // The event type cannot be used to declare a structure or union field.
  if (LangOpts.OpenCL && T->isEventT()) {
    Diag(Loc, diag::err_event_t_struct_field);
    D.setInvalidType();
  }

  DiagnoseFunctionSpecifiers(D.getDeclSpec());

  if (DeclSpec::TSCS TSCS = D.getDeclSpec().getThreadStorageClassSpec())
    Diag(D.getDeclSpec().getThreadStorageClassSpecLoc(),
         diag::err_invalid_thread)
      << DeclSpec::getSpecifierName(TSCS);

  // Check to see if this name was declared as a member previously
  NamedDecl *PrevDecl = nullptr;
  LookupResult Previous(*this, II, Loc, LookupMemberName, ForRedeclaration);
  LookupName(Previous, S);
  switch (Previous.getResultKind()) {
    case LookupResult::Found:
    case LookupResult::FoundUnresolvedValue:
      PrevDecl = Previous.getAsSingle<NamedDecl>();
      break;
      
    case LookupResult::FoundOverloaded:
      PrevDecl = Previous.getRepresentativeDecl();
      break;
      
    case LookupResult::NotFound:
    case LookupResult::NotFoundInCurrentInstantiation:
    case LookupResult::Ambiguous:
      break;
  }
  Previous.suppressDiagnostics();

  if (PrevDecl && PrevDecl->isTemplateParameter()) {
    // Maybe we will complain about the shadowed template parameter.
    DiagnoseTemplateParameterShadow(D.getIdentifierLoc(), PrevDecl);
    // Just pretend that we didn't see the previous declaration.
    PrevDecl = nullptr;
  }

  if (PrevDecl && !isDeclInScope(PrevDecl, Record, S))
    PrevDecl = nullptr;

  bool Mutable
    = (D.getDeclSpec().getStorageClassSpec() == DeclSpec::SCS_mutable);
  SourceLocation TSSL = D.getLocStart();
  FieldDecl *NewFD
    = CheckFieldDecl(II, T, TInfo, Record, Loc, Mutable, BitWidth, InitStyle,
                     TSSL, AS, PrevDecl, &D);

  if (NewFD->isInvalidDecl())
    Record->setInvalidDecl();

  TransferUnusualAttributes(D, NewFD); // HLSL Change

  if (D.getDeclSpec().isModulePrivateSpecified())
    NewFD->setModulePrivate();
  
  if (NewFD->isInvalidDecl() && PrevDecl) {
    // Don't introduce NewFD into scope; there's already something
    // with the same name in the same scope.
  } else if (II) {
    PushOnScopeChains(NewFD, S);
  } else
    Record->addDecl(NewFD);

  return NewFD;
}

/// \brief Build a new FieldDecl and check its well-formedness.
///
/// This routine builds a new FieldDecl given the fields name, type,
/// record, etc. \p PrevDecl should refer to any previous declaration
/// with the same name and in the same scope as the field to be
/// created.
///
/// \returns a new FieldDecl.
///
/// \todo The Declarator argument is a hack. It will be removed once
FieldDecl *Sema::CheckFieldDecl(DeclarationName Name, QualType T,
                                TypeSourceInfo *TInfo,
                                RecordDecl *Record, SourceLocation Loc,
                                bool Mutable, Expr *BitWidth,
                                InClassInitStyle InitStyle,
                                SourceLocation TSSL,
                                AccessSpecifier AS, NamedDecl *PrevDecl,
                                Declarator *D) {
  IdentifierInfo *II = Name.getAsIdentifierInfo();
  bool InvalidDecl = false;
  if (D) InvalidDecl = D->isInvalidType();

  // If we receive a broken type, recover by assuming 'int' and
  // marking this declaration as invalid.
  if (T.isNull()) {
    InvalidDecl = true;
    T = Context.IntTy;
  }

  QualType EltTy = Context.getBaseElementType(T);
  if (!EltTy->isDependentType()) {
    if (RequireCompleteType(Loc, EltTy, diag::err_field_incomplete)) {
      // Fields of incomplete type force their record to be invalid.
      Record->setInvalidDecl();
      InvalidDecl = true;
    } else {
      NamedDecl *Def;
      EltTy->isIncompleteType(&Def);
      if (Def && Def->isInvalidDecl()) {
        Record->setInvalidDecl();
        InvalidDecl = true;
      }
    }
  }

  // OpenCL v1.2 s6.9.c: bitfields are not supported.
  if (BitWidth && getLangOpts().OpenCL) {
    Diag(Loc, diag::err_opencl_bitfields);
    InvalidDecl = true;
  }

  // C99 6.7.2.1p8: A member of a structure or union may have any type other
  // than a variably modified type.
  if (!InvalidDecl && T->isVariablyModifiedType()) {
    bool SizeIsNegative;
    llvm::APSInt Oversized;

    TypeSourceInfo *FixedTInfo =
      TryToFixInvalidVariablyModifiedTypeSourceInfo(TInfo, Context,
                                                    SizeIsNegative,
                                                    Oversized);
    if (FixedTInfo) {
      Diag(Loc, diag::warn_illegal_constant_array_size);
      TInfo = FixedTInfo;
      T = FixedTInfo->getType();
    } else {
      if (SizeIsNegative)
        Diag(Loc, diag::err_typecheck_negative_array_size);
      else if (Oversized.getBoolValue())
        Diag(Loc, diag::err_array_too_large)
          << Oversized.toString(10);
      else
        Diag(Loc, diag::err_typecheck_field_variable_size);
      InvalidDecl = true;
    }
  }

  // Fields can not have abstract class types
  if (!InvalidDecl && RequireNonAbstractType(Loc, T,
                                             diag::err_abstract_type_in_decl,
                                             AbstractFieldType))
    InvalidDecl = true;

  bool ZeroWidth = false;
  if (InvalidDecl)
    BitWidth = nullptr;
  // If this is declared as a bit-field, check the bit-field.
  if (BitWidth) {
    BitWidth = VerifyBitField(Loc, II, T, Record->isMsStruct(Context), BitWidth,
                              &ZeroWidth).get();
    if (!BitWidth) {
      InvalidDecl = true;
      BitWidth = nullptr;
      ZeroWidth = false;
    }
  }

  // Check that 'mutable' is consistent with the type of the declaration.
  if (!InvalidDecl && Mutable) {
    unsigned DiagID = 0;
    if (T->isReferenceType())
      DiagID = getLangOpts().MSVCCompat ? diag::ext_mutable_reference
                                        : diag::err_mutable_reference;
    else if (T.isConstQualified())
      DiagID = diag::err_mutable_const;

    if (DiagID) {
      SourceLocation ErrLoc = Loc;
      if (D && D->getDeclSpec().getStorageClassSpecLoc().isValid())
        ErrLoc = D->getDeclSpec().getStorageClassSpecLoc();
      Diag(ErrLoc, DiagID);
      if (DiagID != diag::ext_mutable_reference) {
        Mutable = false;
        InvalidDecl = true;
      }
    }
  }

  // C++11 [class.union]p8 (DR1460):
  //   At most one variant member of a union may have a
  //   brace-or-equal-initializer.
  if (InitStyle != ICIS_NoInit)
    checkDuplicateDefaultInit(*this, cast<CXXRecordDecl>(Record), Loc);

  FieldDecl *NewFD = FieldDecl::Create(Context, Record, TSSL, Loc, II, T, TInfo,
                                       BitWidth, Mutable, InitStyle);
  if (InvalidDecl)
    NewFD->setInvalidDecl();

  if (PrevDecl && !isa<TagDecl>(PrevDecl)) {
    Diag(Loc, diag::err_duplicate_member) << II;
    Diag(PrevDecl->getLocation(), diag::note_previous_declaration);
    NewFD->setInvalidDecl();
  }

  if (!InvalidDecl && getLangOpts().CPlusPlus) {
    if (Record->isUnion()) {
      if (const RecordType *RT = EltTy->getAs<RecordType>()) {
        CXXRecordDecl* RDecl = cast<CXXRecordDecl>(RT->getDecl());
        if (RDecl->getDefinition()) {
          // C++ [class.union]p1: An object of a class with a non-trivial
          // constructor, a non-trivial copy constructor, a non-trivial
          // destructor, or a non-trivial copy assignment operator
          // cannot be a member of a union, nor can an array of such
          // objects.
          if (CheckNontrivialField(NewFD))
            NewFD->setInvalidDecl();
        }
      }

      // C++ [class.union]p1: If a union contains a member of reference type,
      // the program is ill-formed, except when compiling with MSVC extensions
      // enabled.
      if (EltTy->isReferenceType()) {
        Diag(NewFD->getLocation(), getLangOpts().MicrosoftExt ?
                                    diag::ext_union_member_of_reference_type :
                                    diag::err_union_member_of_reference_type)
          << NewFD->getDeclName() << EltTy;
        if (!getLangOpts().MicrosoftExt)
          NewFD->setInvalidDecl();
      }
    }
  }

  // FIXME: We need to pass in the attributes given an AST
  // representation, not a parser representation.
  if (D) {
    // FIXME: The current scope is almost... but not entirely... correct here.
    ProcessDeclAttributes(getCurScope(), NewFD, *D);

    if (NewFD->hasAttrs())
      CheckAlignasUnderalignment(NewFD);
  }

  // In auto-retain/release, infer strong retension for fields of
  // retainable type.
  if (getLangOpts().ObjCAutoRefCount && inferObjCARCLifetime(NewFD))
    NewFD->setInvalidDecl();

  if (T.isObjCGCWeak())
    Diag(Loc, diag::warn_attribute_weak_on_field);

  NewFD->setAccess(AS);
  return NewFD;
}

bool Sema::CheckNontrivialField(FieldDecl *FD) {
  assert(FD);
  assert(getLangOpts().CPlusPlus && "valid check only for C++");

  if (FD->isInvalidDecl() || FD->getType()->isDependentType())
    return false;

  QualType EltTy = Context.getBaseElementType(FD->getType());
  if (const RecordType *RT = EltTy->getAs<RecordType>()) {
    CXXRecordDecl *RDecl = cast<CXXRecordDecl>(RT->getDecl());
    if (RDecl->getDefinition()) {
      // We check for copy constructors before constructors
      // because otherwise we'll never get complaints about
      // copy constructors.

      CXXSpecialMember member = CXXInvalid;
      // We're required to check for any non-trivial constructors. Since the
      // implicit default constructor is suppressed if there are any
      // user-declared constructors, we just need to check that there is a
      // trivial default constructor and a trivial copy constructor. (We don't
      // worry about move constructors here, since this is a C++98 check.)
      if (RDecl->hasNonTrivialCopyConstructor())
        member = CXXCopyConstructor;
      else if (!RDecl->hasTrivialDefaultConstructor())
        member = CXXDefaultConstructor;
      else if (RDecl->hasNonTrivialCopyAssignment())
        member = CXXCopyAssignment;
      else if (RDecl->hasNonTrivialDestructor())
        member = CXXDestructor;

      if (member != CXXInvalid) {
        if (!getLangOpts().CPlusPlus11 &&
            getLangOpts().ObjCAutoRefCount && RDecl->hasObjectMember()) {
          // Objective-C++ ARC: it is an error to have a non-trivial field of
          // a union. However, system headers in Objective-C programs 
          // occasionally have Objective-C lifetime objects within unions,
          // and rather than cause the program to fail, we make those 
          // members unavailable.
          SourceLocation Loc = FD->getLocation();
          if (getSourceManager().isInSystemHeader(Loc)) {
            if (!FD->hasAttr<UnavailableAttr>())
              FD->addAttr(UnavailableAttr::CreateImplicit(Context,
                                  "this system field has retaining ownership",
                                  Loc));
            return false;
          }
        }

        Diag(FD->getLocation(), getLangOpts().CPlusPlus11 ?
               diag::warn_cxx98_compat_nontrivial_union_or_anon_struct_member :
               diag::err_illegal_union_or_anon_struct_member)
          << (int)FD->getParent()->isUnion() << FD->getDeclName() << member;
        DiagnoseNontrivial(RDecl, member);
        return !getLangOpts().CPlusPlus11;
      }
    }
  }

  return false;
}

/// TranslateIvarVisibility - Translate visibility from a token ID to an
///  AST enum value.
static ObjCIvarDecl::AccessControl
TranslateIvarVisibility(tok::ObjCKeywordKind ivarVisibility) {
  switch (ivarVisibility) {
  default: llvm_unreachable("Unknown visitibility kind");
  case tok::objc_private: return ObjCIvarDecl::Private;
  case tok::objc_public: return ObjCIvarDecl::Public;
  case tok::objc_protected: return ObjCIvarDecl::Protected;
  case tok::objc_package: return ObjCIvarDecl::Package;
  }
}

/// ActOnIvar - Each ivar field of an objective-c class is passed into this
/// in order to create an IvarDecl object for it.
Decl *Sema::ActOnIvar(Scope *S,
                                SourceLocation DeclStart,
                                Declarator &D, Expr *BitfieldWidth,
                                tok::ObjCKeywordKind Visibility) {

  IdentifierInfo *II = D.getIdentifier();
  Expr *BitWidth = (Expr*)BitfieldWidth;
  SourceLocation Loc = DeclStart;
  if (II) Loc = D.getIdentifierLoc();

  // FIXME: Unnamed fields can be handled in various different ways, for
  // example, unnamed unions inject all members into the struct namespace!

  TypeSourceInfo *TInfo = GetTypeForDeclarator(D, S);
  QualType T = TInfo->getType();

  if (BitWidth) {
    // 6.7.2.1p3, 6.7.2.1p4
    BitWidth = VerifyBitField(Loc, II, T, /*IsMsStruct*/false, BitWidth).get();
    if (!BitWidth)
      D.setInvalidType();
  } else {
    // Not a bitfield.

    // validate II.

  }
  if (T->isReferenceType()) {
    Diag(Loc, diag::err_ivar_reference_type);
    D.setInvalidType();
  }
  // C99 6.7.2.1p8: A member of a structure or union may have any type other
  // than a variably modified type.
  else if (T->isVariablyModifiedType()) {
    Diag(Loc, diag::err_typecheck_ivar_variable_size);
    D.setInvalidType();
  }

  // Get the visibility (access control) for this ivar.
  ObjCIvarDecl::AccessControl ac =
    Visibility != tok::objc_not_keyword ? TranslateIvarVisibility(Visibility)
                                        : ObjCIvarDecl::None;
  // Must set ivar's DeclContext to its enclosing interface.
  ObjCContainerDecl *EnclosingDecl = cast<ObjCContainerDecl>(CurContext);
  if (!EnclosingDecl || EnclosingDecl->isInvalidDecl())
    return nullptr;
  ObjCContainerDecl *EnclosingContext;
  if (ObjCImplementationDecl *IMPDecl =
      dyn_cast<ObjCImplementationDecl>(EnclosingDecl)) {
    if (LangOpts.ObjCRuntime.isFragile()) {
    // Case of ivar declared in an implementation. Context is that of its class.
      EnclosingContext = IMPDecl->getClassInterface();
      assert(EnclosingContext && "Implementation has no class interface!");
    }
    else
      EnclosingContext = EnclosingDecl;
  } else {
    if (ObjCCategoryDecl *CDecl = 
        dyn_cast<ObjCCategoryDecl>(EnclosingDecl)) {
      if (LangOpts.ObjCRuntime.isFragile() || !CDecl->IsClassExtension()) {
        Diag(Loc, diag::err_misplaced_ivar) << CDecl->IsClassExtension();
        return nullptr;
      }
    }
    EnclosingContext = EnclosingDecl;
  }

  // Construct the decl.
  ObjCIvarDecl *NewID = ObjCIvarDecl::Create(Context, EnclosingContext,
                                             DeclStart, Loc, II, T,
                                             TInfo, ac, (Expr *)BitfieldWidth);

  if (II) {
    NamedDecl *PrevDecl = LookupSingleName(S, II, Loc, LookupMemberName,
                                           ForRedeclaration);
    if (PrevDecl && isDeclInScope(PrevDecl, EnclosingContext, S)
        && !isa<TagDecl>(PrevDecl)) {
      Diag(Loc, diag::err_duplicate_member) << II;
      Diag(PrevDecl->getLocation(), diag::note_previous_declaration);
      NewID->setInvalidDecl();
    }
  }

  // Process attributes attached to the ivar.
  ProcessDeclAttributes(S, NewID, D);

  if (D.isInvalidType())
    NewID->setInvalidDecl();

  // In ARC, infer 'retaining' for ivars of retainable type.
  if (getLangOpts().ObjCAutoRefCount && inferObjCARCLifetime(NewID))
    NewID->setInvalidDecl();

  if (D.getDeclSpec().isModulePrivateSpecified())
    NewID->setModulePrivate();
  
  if (II) {
    // FIXME: When interfaces are DeclContexts, we'll need to add
    // these to the interface.
    S->AddDecl(NewID);
    IdResolver.AddDecl(NewID);
  }
  
  if (LangOpts.ObjCRuntime.isNonFragile() &&
      !NewID->isInvalidDecl() && isa<ObjCInterfaceDecl>(EnclosingDecl))
    Diag(Loc, diag::warn_ivars_in_interface);
  
  return NewID;
}

/// ActOnLastBitfield - This routine handles synthesized bitfields rules for 
/// class and class extensions. For every class \@interface and class 
/// extension \@interface, if the last ivar is a bitfield of any type, 
/// then add an implicit `char :0` ivar to the end of that interface.
void Sema::ActOnLastBitfield(SourceLocation DeclLoc,
                             SmallVectorImpl<Decl *> &AllIvarDecls) {
  if (LangOpts.ObjCRuntime.isFragile() || AllIvarDecls.empty())
    return;
  
  Decl *ivarDecl = AllIvarDecls[AllIvarDecls.size()-1];
  ObjCIvarDecl *Ivar = cast<ObjCIvarDecl>(ivarDecl);
  
  if (!Ivar->isBitField() || Ivar->getBitWidthValue(Context) == 0)
    return;
  ObjCInterfaceDecl *ID = dyn_cast<ObjCInterfaceDecl>(CurContext);
  if (!ID) {
    if (ObjCCategoryDecl *CD = dyn_cast<ObjCCategoryDecl>(CurContext)) {
      if (!CD->IsClassExtension())
        return;
    }
    // No need to add this to end of @implementation.
    else
      return;
  }
  // All conditions are met. Add a new bitfield to the tail end of ivars.
  llvm::APInt Zero(Context.getTypeSize(Context.IntTy), 0);
  Expr * BW = IntegerLiteral::Create(Context, Zero, Context.IntTy, DeclLoc);

  Ivar = ObjCIvarDecl::Create(Context, cast<ObjCContainerDecl>(CurContext),
                              DeclLoc, DeclLoc, nullptr,
                              Context.CharTy, 
                              Context.getTrivialTypeSourceInfo(Context.CharTy,
                                                               DeclLoc),
                              ObjCIvarDecl::Private, BW,
                              true);
  AllIvarDecls.push_back(Ivar);
}

void Sema::ActOnFields(Scope *S, SourceLocation RecLoc, Decl *EnclosingDecl,
                       ArrayRef<Decl *> Fields, SourceLocation LBrac,
                       SourceLocation RBrac, AttributeList *Attr) {
  assert(EnclosingDecl && "missing record or interface decl");

  // If this is an Objective-C @implementation or category and we have
  // new fields here we should reset the layout of the interface since
  // it will now change.
  if (!Fields.empty() && isa<ObjCContainerDecl>(EnclosingDecl)) {
    ObjCContainerDecl *DC = cast<ObjCContainerDecl>(EnclosingDecl);
    switch (DC->getKind()) {
    default: break;
    case Decl::ObjCCategory:
      Context.ResetObjCLayout(cast<ObjCCategoryDecl>(DC)->getClassInterface());
      break;
    case Decl::ObjCImplementation:
      Context.
        ResetObjCLayout(cast<ObjCImplementationDecl>(DC)->getClassInterface());
      break;
    }
  }
  
  RecordDecl *Record = dyn_cast<RecordDecl>(EnclosingDecl);

  // Start counting up the number of named members; make sure to include
  // members of anonymous structs and unions in the total.
  unsigned NumNamedMembers = 0;
  if (Record) {
    for (const auto *I : Record->decls()) {
      if (const auto *IFD = dyn_cast<IndirectFieldDecl>(I))
        if (IFD->getDeclName())
          ++NumNamedMembers;
    }
  }

  // Verify that all the fields are okay.
  SmallVector<FieldDecl*, 32> RecFields;

  bool ARCErrReported = false;
  for (ArrayRef<Decl *>::iterator i = Fields.begin(), end = Fields.end();
       i != end; ++i) {
    FieldDecl *FD = cast<FieldDecl>(*i);

    // Get the type for the field.
    const Type *FDTy = FD->getType().getTypePtr();

    if (!FD->isAnonymousStructOrUnion()) {
      // Remember all fields written by the user.
      RecFields.push_back(FD);
    }

    // If the field is already invalid for some reason, don't emit more
    // diagnostics about it.
    if (FD->isInvalidDecl()) {
      EnclosingDecl->setInvalidDecl();
      continue;
    }

    // C99 6.7.2.1p2:
    //   A structure or union shall not contain a member with
    //   incomplete or function type (hence, a structure shall not
    //   contain an instance of itself, but may contain a pointer to
    //   an instance of itself), except that the last member of a
    //   structure with more than one named member may have incomplete
    //   array type; such a structure (and any union containing,
    //   possibly recursively, a member that is such a structure)
    //   shall not be a member of a structure or an element of an
    //   array.
    if (FDTy->isFunctionType()) {
      // Field declared as a function.
      Diag(FD->getLocation(), diag::err_field_declared_as_function)
        << FD->getDeclName();
      FD->setInvalidDecl();
      EnclosingDecl->setInvalidDecl();
      continue;
    } else if (FDTy->isIncompleteArrayType() && Record && 
               ((i + 1 == Fields.end() && !Record->isUnion()) ||
                ((getLangOpts().MicrosoftExt ||
                  getLangOpts().CPlusPlus) &&
                 (i + 1 == Fields.end() || Record->isUnion())))) {
      // Flexible array member.
      // Microsoft and g++ is more permissive regarding flexible array.
      // It will accept flexible array in union and also
      // as the sole element of a struct/class.
      unsigned DiagID = 0;
      if (Record->isUnion())
        DiagID = getLangOpts().MicrosoftExt
                     ? diag::ext_flexible_array_union_ms
                     : getLangOpts().CPlusPlus
                           ? diag::ext_flexible_array_union_gnu
                           : diag::err_flexible_array_union;
      else if (Fields.size() == 1)
        DiagID = getLangOpts().MicrosoftExt
                     ? diag::ext_flexible_array_empty_aggregate_ms
                     : getLangOpts().CPlusPlus
                           ? diag::ext_flexible_array_empty_aggregate_gnu
                           : NumNamedMembers < 1
                                 ? diag::err_flexible_array_empty_aggregate
                                 : 0;

      if (DiagID)
        Diag(FD->getLocation(), DiagID) << FD->getDeclName()
                                        << Record->getTagKind();
      // While the layout of types that contain virtual bases is not specified
      // by the C++ standard, both the Itanium and Microsoft C++ ABIs place
      // virtual bases after the derived members.  This would make a flexible
      // array member declared at the end of an object not adjacent to the end
      // of the type.
      if (CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(Record))
        if (RD->getNumVBases() != 0)
          Diag(FD->getLocation(), diag::err_flexible_array_virtual_base)
            << FD->getDeclName() << Record->getTagKind();
      if (!getLangOpts().C99)
        Diag(FD->getLocation(), diag::ext_c99_flexible_array_member)
          << FD->getDeclName() << Record->getTagKind();

      // If the element type has a non-trivial destructor, we would not
      // implicitly destroy the elements, so disallow it for now.
      //
      // FIXME: GCC allows this. We should probably either implicitly delete
      // the destructor of the containing class, or just allow this.
      QualType BaseElem = Context.getBaseElementType(FD->getType());
      if (!BaseElem->isDependentType() && BaseElem.isDestructedType()) {
        Diag(FD->getLocation(), diag::err_flexible_array_has_nontrivial_dtor)
          << FD->getDeclName() << FD->getType();
        FD->setInvalidDecl();
        EnclosingDecl->setInvalidDecl();
        continue;
      }
      // Okay, we have a legal flexible array member at the end of the struct.
      Record->setHasFlexibleArrayMember(true);
    } else if (!FDTy->isDependentType() &&
               RequireCompleteType(FD->getLocation(), FD->getType(),
                                   diag::err_field_incomplete)) {
      // Incomplete type
      FD->setInvalidDecl();
      EnclosingDecl->setInvalidDecl();
      continue;
    } else if (const RecordType *FDTTy = FDTy->getAs<RecordType>()) {
      if (Record && FDTTy->getDecl()->hasFlexibleArrayMember()) {
        // A type which contains a flexible array member is considered to be a
        // flexible array member.
        Record->setHasFlexibleArrayMember(true);
        if (!Record->isUnion()) {
          // If this is a struct/class and this is not the last element, reject
          // it.  Note that GCC supports variable sized arrays in the middle of
          // structures.
          if (i + 1 != Fields.end())
            Diag(FD->getLocation(), diag::ext_variable_sized_type_in_struct)
              << FD->getDeclName() << FD->getType();
          else {
            // We support flexible arrays at the end of structs in
            // other structs as an extension.
            Diag(FD->getLocation(), diag::ext_flexible_array_in_struct)
              << FD->getDeclName();
          }
        }
      }
      if (isa<ObjCContainerDecl>(EnclosingDecl) &&
          RequireNonAbstractType(FD->getLocation(), FD->getType(),
                                 diag::err_abstract_type_in_decl,
                                 AbstractIvarType)) {
        // Ivars can not have abstract class types
        FD->setInvalidDecl();
      }
      if (Record && FDTTy->getDecl()->hasObjectMember())
        Record->setHasObjectMember(true);
      if (Record && FDTTy->getDecl()->hasVolatileMember())
        Record->setHasVolatileMember(true);
    } else if (FDTy->isObjCObjectType()) {
      /// A field cannot be an Objective-c object
      Diag(FD->getLocation(), diag::err_statically_allocated_object)
        << FixItHint::CreateInsertion(FD->getLocation(), "*");
      QualType T = Context.getObjCObjectPointerType(FD->getType());
      FD->setType(T);
    } else if (getLangOpts().ObjCAutoRefCount && Record && !ARCErrReported &&
               (!getLangOpts().CPlusPlus || Record->isUnion())) {
      // It's an error in ARC if a field has lifetime.
      // We don't want to report this in a system header, though,
      // so we just make the field unavailable.
      // FIXME: that's really not sufficient; we need to make the type
      // itself invalid to, say, initialize or copy.
      QualType T = FD->getType();
      Qualifiers::ObjCLifetime lifetime = T.getObjCLifetime();
      if (lifetime && lifetime != Qualifiers::OCL_ExplicitNone) {
        SourceLocation loc = FD->getLocation();
        if (getSourceManager().isInSystemHeader(loc)) {
          if (!FD->hasAttr<UnavailableAttr>()) {
            FD->addAttr(UnavailableAttr::CreateImplicit(Context,
                              "this system field has retaining ownership",
                              loc));
          }
        } else {
          Diag(FD->getLocation(), diag::err_arc_objc_object_in_tag) 
            << T->isBlockPointerType() << Record->getTagKind();
        }
        ARCErrReported = true;
      }
    } else if (getLangOpts().ObjC1 &&
               getLangOpts().getGC() != LangOptions::NonGC &&
               Record && !Record->hasObjectMember()) {
      if (FD->getType()->isObjCObjectPointerType() ||
          FD->getType().isObjCGCStrong())
        Record->setHasObjectMember(true);
      else if (Context.getAsArrayType(FD->getType())) {
        QualType BaseType = Context.getBaseElementType(FD->getType());
        if (BaseType->isRecordType() && 
            BaseType->getAs<RecordType>()->getDecl()->hasObjectMember())
          Record->setHasObjectMember(true);
        else if (BaseType->isObjCObjectPointerType() ||
                 BaseType.isObjCGCStrong())
               Record->setHasObjectMember(true);
      }
    }
    if (Record && FD->getType().isVolatileQualified())
      Record->setHasVolatileMember(true);
    // Keep track of the number of named members.
    if (FD->getIdentifier())
      ++NumNamedMembers;
  }

  // Okay, we successfully defined 'Record'.
  if (Record) {
    bool Completed = false;
    if (CXXRecordDecl *CXXRecord = dyn_cast<CXXRecordDecl>(Record)) {
      if (!CXXRecord->isInvalidDecl()) {
        // Set access bits correctly on the directly-declared conversions.
        for (CXXRecordDecl::conversion_iterator
               I = CXXRecord->conversion_begin(),
               E = CXXRecord->conversion_end(); I != E; ++I)
          I.setAccess((*I)->getAccess());
        
        if (!CXXRecord->isDependentType()) {
          if (CXXRecord->hasUserDeclaredDestructor()) {
            // Adjust user-defined destructor exception spec.
            if (getLangOpts().CPlusPlus11)
              AdjustDestructorExceptionSpec(CXXRecord,
                                            CXXRecord->getDestructor());
          }

          // Add any implicitly-declared members to this class.
          AddImplicitlyDeclaredMembersToClass(CXXRecord);

          // If we have virtual base classes, we may end up finding multiple 
          // final overriders for a given virtual function. Check for this 
          // problem now.
          if (CXXRecord->getNumVBases()) {
            CXXFinalOverriderMap FinalOverriders;
            CXXRecord->getFinalOverriders(FinalOverriders);
            
            for (CXXFinalOverriderMap::iterator M = FinalOverriders.begin(), 
                                             MEnd = FinalOverriders.end();
                 M != MEnd; ++M) {
              for (OverridingMethods::iterator SO = M->second.begin(), 
                                            SOEnd = M->second.end();
                   SO != SOEnd; ++SO) {
                assert(SO->second.size() > 0 && 
                       "Virtual function without overridding functions?");
                if (SO->second.size() == 1)
                  continue;
                
                // C++ [class.virtual]p2:
                //   In a derived class, if a virtual member function of a base
                //   class subobject has more than one final overrider the
                //   program is ill-formed.
                Diag(Record->getLocation(), diag::err_multiple_final_overriders)
                  << (const NamedDecl *)M->first << Record;
                Diag(M->first->getLocation(), 
                     diag::note_overridden_virtual_function);
                for (OverridingMethods::overriding_iterator 
                          OM = SO->second.begin(), 
                       OMEnd = SO->second.end();
                     OM != OMEnd; ++OM)
                  Diag(OM->Method->getLocation(), diag::note_final_overrider)
                    << (const NamedDecl *)M->first << OM->Method->getParent();
                
                Record->setInvalidDecl();
              }
            }
            CXXRecord->completeDefinition(&FinalOverriders);
            Completed = true;
          }
        }
      }
    }
    
    if (!Completed)
      Record->completeDefinition();

    if (Record->hasAttrs()) {
      CheckAlignasUnderalignment(Record);

      if (const MSInheritanceAttr *IA = Record->getAttr<MSInheritanceAttr>())
        checkMSInheritanceAttrOnDefinition(cast<CXXRecordDecl>(Record),
                                           IA->getRange(), IA->getBestCase(),
                                           IA->getSemanticSpelling());
    }

    // Check if the structure/union declaration is a type that can have zero
    // size in C. For C this is a language extension, for C++ it may cause
    // compatibility problems.
    bool CheckForZeroSize;
    if (!getLangOpts().CPlusPlus) {
      CheckForZeroSize = true;
    } else {
      // For C++ filter out types that cannot be referenced in C code.
      CXXRecordDecl *CXXRecord = cast<CXXRecordDecl>(Record);
      CheckForZeroSize =
          CXXRecord->getLexicalDeclContext()->isExternCContext() &&
          !CXXRecord->isDependentType() &&
          CXXRecord->isCLike();
    }
    if (CheckForZeroSize) {
      bool ZeroSize = true;
      bool IsEmpty = true;
      unsigned NonBitFields = 0;
      for (RecordDecl::field_iterator I = Record->field_begin(),
                                      E = Record->field_end();
           (NonBitFields == 0 || ZeroSize) && I != E; ++I) {
        IsEmpty = false;
        if (I->isUnnamedBitfield()) {
          if (I->getBitWidthValue(Context) > 0)
            ZeroSize = false;
        } else {
          ++NonBitFields;
          QualType FieldType = I->getType();
          if (FieldType->isIncompleteType() ||
              !Context.getTypeSizeInChars(FieldType).isZero())
            ZeroSize = false;
        }
      }

      // Empty structs are an extension in C (C99 6.7.2.1p7). They are
      // allowed in C++, but warn if its declaration is inside
      // extern "C" block.
      if (ZeroSize) {
        Diag(RecLoc, getLangOpts().CPlusPlus ?
                         diag::warn_zero_size_struct_union_in_extern_c :
                         diag::warn_zero_size_struct_union_compat)
          << IsEmpty << Record->isUnion() << (NonBitFields > 1);
      }

      // Structs without named members are extension in C (C99 6.7.2.1p7),
      // but are accepted by GCC.
      if (NonBitFields == 0 && !getLangOpts().CPlusPlus) {
        Diag(RecLoc, IsEmpty ? diag::ext_empty_struct_union :
                               diag::ext_no_named_members_in_struct_union)
          << Record->isUnion();
      }
    }
  } else {
    ObjCIvarDecl **ClsFields =
      reinterpret_cast<ObjCIvarDecl**>(RecFields.data());
    if (ObjCInterfaceDecl *ID = dyn_cast<ObjCInterfaceDecl>(EnclosingDecl)) {
      ID->setEndOfDefinitionLoc(RBrac);
      // Add ivar's to class's DeclContext.
      for (unsigned i = 0, e = RecFields.size(); i != e; ++i) {
        ClsFields[i]->setLexicalDeclContext(ID);
        ID->addDecl(ClsFields[i]);
      }
      // Must enforce the rule that ivars in the base classes may not be
      // duplicates.
      if (ID->getSuperClass())
        DiagnoseDuplicateIvars(ID, ID->getSuperClass());
    } else if (ObjCImplementationDecl *IMPDecl =
                  dyn_cast<ObjCImplementationDecl>(EnclosingDecl)) {
      assert(IMPDecl && "ActOnFields - missing ObjCImplementationDecl");
      for (unsigned I = 0, N = RecFields.size(); I != N; ++I)
        // Ivar declared in @implementation never belongs to the implementation.
        // Only it is in implementation's lexical context.
        ClsFields[I]->setLexicalDeclContext(IMPDecl);
      CheckImplementationIvars(IMPDecl, ClsFields, RecFields.size(), RBrac);
      IMPDecl->setIvarLBraceLoc(LBrac);
      IMPDecl->setIvarRBraceLoc(RBrac);
    } else if (ObjCCategoryDecl *CDecl = 
                dyn_cast<ObjCCategoryDecl>(EnclosingDecl)) {
      // case of ivars in class extension; all other cases have been
      // reported as errors elsewhere.
      // FIXME. Class extension does not have a LocEnd field.
      // CDecl->setLocEnd(RBrac);
      // Add ivar's to class extension's DeclContext.
      // Diagnose redeclaration of private ivars.
      ObjCInterfaceDecl *IDecl = CDecl->getClassInterface();
      for (unsigned i = 0, e = RecFields.size(); i != e; ++i) {
        if (IDecl) {
          if (const ObjCIvarDecl *ClsIvar = 
              IDecl->getIvarDecl(ClsFields[i]->getIdentifier())) {
            Diag(ClsFields[i]->getLocation(), 
                 diag::err_duplicate_ivar_declaration); 
            Diag(ClsIvar->getLocation(), diag::note_previous_definition);
            continue;
          }
          for (const auto *Ext : IDecl->known_extensions()) {
            if (const ObjCIvarDecl *ClsExtIvar
                  = Ext->getIvarDecl(ClsFields[i]->getIdentifier())) {
              Diag(ClsFields[i]->getLocation(), 
                   diag::err_duplicate_ivar_declaration); 
              Diag(ClsExtIvar->getLocation(), diag::note_previous_definition);
              continue;
            }
          }
        }
        ClsFields[i]->setLexicalDeclContext(CDecl);
        CDecl->addDecl(ClsFields[i]);
      }
      CDecl->setIvarLBraceLoc(LBrac);
      CDecl->setIvarRBraceLoc(RBrac);
    }
  }

  if (Attr)
    ProcessDeclAttributeList(S, Record, Attr);
}

/// \brief Determine whether the given integral value is representable within
/// the given type T.
static bool isRepresentableIntegerValue(ASTContext &Context,
                                        llvm::APSInt &Value,
                                        QualType T) {
  assert(T->isIntegralType(Context) && "Integral type required!");
  unsigned BitWidth = Context.getIntWidth(T);
  
  if (Value.isUnsigned() || Value.isNonNegative()) {
    if (T->isSignedIntegerOrEnumerationType()) 
      --BitWidth;
    return Value.getActiveBits() <= BitWidth;
  }  
  return Value.getMinSignedBits() <= BitWidth;
}

// \brief Given an integral type, return the next larger integral type
// (or a NULL type of no such type exists).
static QualType getNextLargerIntegralType(ASTContext &Context, QualType T) {
  // FIXME: Int128/UInt128 support, which also needs to be introduced into 
  // enum checking below.
  assert(T->isIntegralType(Context) && "Integral type required!");
  const unsigned NumTypes = 4;
  QualType SignedIntegralTypes[NumTypes] = { 
    Context.ShortTy, Context.IntTy, Context.LongTy, Context.LongLongTy
  };
  QualType UnsignedIntegralTypes[NumTypes] = { 
    Context.UnsignedShortTy, Context.UnsignedIntTy, Context.UnsignedLongTy, 
    Context.UnsignedLongLongTy
  };
  
  unsigned BitWidth = Context.getTypeSize(T);
  QualType *Types = T->isSignedIntegerOrEnumerationType()? SignedIntegralTypes
                                                        : UnsignedIntegralTypes;
  for (unsigned I = 0; I != NumTypes; ++I)
    if (Context.getTypeSize(Types[I]) > BitWidth)
      return Types[I];
  
  return QualType();
}

EnumConstantDecl *Sema::CheckEnumConstant(EnumDecl *Enum,
                                          EnumConstantDecl *LastEnumConst,
                                          SourceLocation IdLoc,
                                          IdentifierInfo *Id,
                                          Expr *Val) {
  unsigned IntWidth = Context.getTargetInfo().getIntWidth();
  llvm::APSInt EnumVal(IntWidth);
  QualType EltTy;

  if (Val && DiagnoseUnexpandedParameterPack(Val, UPPC_EnumeratorValue))
    Val = nullptr;

  if (Val)
    Val = DefaultLvalueConversion(Val).get();

  if (Val) {
    if (Enum->isDependentType() || Val->isTypeDependent())
      EltTy = Context.DependentTy;
    else {
      SourceLocation ExpLoc;
      // HLSL Change - check constant expression for enum
      if ((getLangOpts().HLSLVersion >= hlsl::LangStd::v2017 ||
           getLangOpts().CPlusPlus11) &&
          Enum->isFixed() && !getLangOpts().MSVCCompat) {
        // C++11 [dcl.enum]p5: If the underlying type is fixed, [...] the
        // constant-expression in the enumerator-definition shall be a converted
        // constant expression of the underlying type.
        EltTy = Enum->getIntegerType();
        ExprResult Converted =
          CheckConvertedConstantExpression(Val, EltTy, EnumVal,
                                           CCEK_Enumerator);
        if (Converted.isInvalid())
          Val = nullptr;
        else
          Val = Converted.get();
      } else if (!Val->isValueDependent() &&
                 !(Val =
                       VerifyIntegerConstantExpression(Val, &EnumVal).get())) {
        // C99 6.7.2.2p2: Make sure we have an integer constant expression.
      } else {
        if (Enum->isFixed()) {
          EltTy = Enum->getIntegerType();

          // In Obj-C and Microsoft mode, require the enumeration value to be
          // representable in the underlying type of the enumeration. In C++11,
          // we perform a non-narrowing conversion as part of converted constant
          // expression checking.
          if (!isRepresentableIntegerValue(Context, EnumVal, EltTy)) {
            if (getLangOpts().MSVCCompat) {
              Diag(IdLoc, diag::ext_enumerator_too_large) << EltTy;
              Val = ImpCastExprToType(Val, EltTy, CK_IntegralCast).get();
            } else
              Diag(IdLoc, diag::err_enumerator_too_large) << EltTy;
          } else
            Val = ImpCastExprToType(Val, EltTy, CK_IntegralCast).get();
        } else if (getLangOpts().CPlusPlus) {
          // C++11 [dcl.enum]p5:
          //   If the underlying type is not fixed, the type of each enumerator
          //   is the type of its initializing value:
          //     - If an initializer is specified for an enumerator, the 
          //       initializing value has the same type as the expression.
          EltTy = Val->getType();
        } else {
          // C99 6.7.2.2p2:
          //   The expression that defines the value of an enumeration constant
          //   shall be an integer constant expression that has a value
          //   representable as an int.

          // Complain if the value is not representable in an int.
          if (!isRepresentableIntegerValue(Context, EnumVal, Context.IntTy))
            Diag(IdLoc, diag::ext_enum_value_not_int)
              << EnumVal.toString(10) << Val->getSourceRange()
              << (EnumVal.isUnsigned() || EnumVal.isNonNegative());
          else if (!Context.hasSameType(Val->getType(), Context.IntTy)) {
            // Force the type of the expression to 'int'.
            Val = ImpCastExprToType(Val, Context.IntTy, CK_IntegralCast).get();
          }
          EltTy = Val->getType();
        }
      }
    }
  }

  if (!Val) {
    if (Enum->isDependentType())
      EltTy = Context.DependentTy;
    else if (!LastEnumConst) {
      // C++0x [dcl.enum]p5:
      //   If the underlying type is not fixed, the type of each enumerator
      //   is the type of its initializing value:
      //     - If no initializer is specified for the first enumerator, the 
      //       initializing value has an unspecified integral type.
      //
      // GCC uses 'int' for its unspecified integral type, as does 
      // C99 6.7.2.2p3.
      if (Enum->isFixed()) {
        EltTy = Enum->getIntegerType();
      }
      else {
        EltTy = Context.IntTy;
      }
    } else {
      // Assign the last value + 1.
      EnumVal = LastEnumConst->getInitVal();
      ++EnumVal;
      EltTy = LastEnumConst->getType();

      // Check for overflow on increment.
      if (EnumVal < LastEnumConst->getInitVal()) {
        // C++0x [dcl.enum]p5:
        //   If the underlying type is not fixed, the type of each enumerator
        //   is the type of its initializing value:
        //
        //     - Otherwise the type of the initializing value is the same as
        //       the type of the initializing value of the preceding enumerator
        //       unless the incremented value is not representable in that type,
        //       in which case the type is an unspecified integral type 
        //       sufficient to contain the incremented value. If no such type
        //       exists, the program is ill-formed.
        QualType T = getNextLargerIntegralType(Context, EltTy);
        if (T.isNull() || Enum->isFixed()) {
          // There is no integral type larger enough to represent this 
          // value. Complain, then allow the value to wrap around.
          EnumVal = LastEnumConst->getInitVal();
          EnumVal = EnumVal.zext(EnumVal.getBitWidth() * 2);
          ++EnumVal;
          if (Enum->isFixed())
            // When the underlying type is fixed, this is ill-formed.
            Diag(IdLoc, diag::err_enumerator_wrapped)
              << EnumVal.toString(10)
              << EltTy;
          else
            Diag(IdLoc, diag::ext_enumerator_increment_too_large)
              << EnumVal.toString(10);
        } else {
          EltTy = T;
        }
        
        // Retrieve the last enumerator's value, extent that type to the
        // type that is supposed to be large enough to represent the incremented
        // value, then increment.
        EnumVal = LastEnumConst->getInitVal();
        EnumVal.setIsSigned(EltTy->isSignedIntegerOrEnumerationType());
        EnumVal = EnumVal.zextOrTrunc(Context.getIntWidth(EltTy));
        ++EnumVal;        
        
        // If we're not in C++, diagnose the overflow of enumerator values,
        // which in C99 means that the enumerator value is not representable in
        // an int (C99 6.7.2.2p2). However, we support GCC's extension that
        // permits enumerator values that are representable in some larger
        // integral type.
        if (!getLangOpts().CPlusPlus && !T.isNull())
          Diag(IdLoc, diag::warn_enum_value_overflow);
      } else if (!getLangOpts().CPlusPlus &&
                 !isRepresentableIntegerValue(Context, EnumVal, EltTy)) {
        // Enforce C99 6.7.2.2p2 even when we compute the next value.
        Diag(IdLoc, diag::ext_enum_value_not_int)
          << EnumVal.toString(10) << 1;
      }
    }
  }

  if (!EltTy->isDependentType()) {
    // Make the enumerator value match the signedness and size of the 
    // enumerator's type.
    EnumVal = EnumVal.extOrTrunc(Context.getIntWidth(EltTy));
    EnumVal.setIsSigned(EltTy->isSignedIntegerOrEnumerationType());
  }
  
  return EnumConstantDecl::Create(Context, Enum, IdLoc, Id, EltTy,
                                  Val, EnumVal);
}

Sema::SkipBodyInfo Sema::shouldSkipAnonEnumBody(Scope *S, IdentifierInfo *II,
                                                SourceLocation IILoc) {
  if (!(getLangOpts().Modules || getLangOpts().ModulesLocalVisibility) ||
      !getLangOpts().CPlusPlus)
    return SkipBodyInfo();

  // We have an anonymous enum definition. Look up the first enumerator to
  // determine if we should merge the definition with an existing one and
  // skip the body.
  NamedDecl *PrevDecl = LookupSingleName(S, II, IILoc, LookupOrdinaryName,
                                         ForRedeclaration);
  auto *PrevECD = dyn_cast_or_null<EnumConstantDecl>(PrevDecl);
  NamedDecl *Hidden;
  if (PrevECD &&
      !hasVisibleDefinition(cast<NamedDecl>(PrevECD->getDeclContext()),
                            &Hidden)) {
    SkipBodyInfo Skip;
    Skip.Previous = Hidden;
    return Skip;
  }

  return SkipBodyInfo();
}

Decl *Sema::ActOnEnumConstant(Scope *S, Decl *theEnumDecl, Decl *lastEnumConst,
                              SourceLocation IdLoc, IdentifierInfo *Id,
                              AttributeList *Attr,
                              SourceLocation EqualLoc, Expr *Val) {
  EnumDecl *TheEnumDecl = cast<EnumDecl>(theEnumDecl);
  EnumConstantDecl *LastEnumConst =
    cast_or_null<EnumConstantDecl>(lastEnumConst);

  // The scope passed in may not be a decl scope.  Zip up the scope tree until
  // we find one that is.
  S = getNonFieldDeclScope(S);

  // Verify that there isn't already something declared with this name in this
  // scope.
  NamedDecl *PrevDecl = LookupSingleName(S, Id, IdLoc, LookupOrdinaryName,
                                         ForRedeclaration);
  if (PrevDecl && PrevDecl->isTemplateParameter()) {
    // Maybe we will complain about the shadowed template parameter.
    DiagnoseTemplateParameterShadow(IdLoc, PrevDecl);
    // Just pretend that we didn't see the previous declaration.
    PrevDecl = nullptr;
  }

  if (PrevDecl) {
    // When in C++, we may get a TagDecl with the same name; in this case the
    // enum constant will 'hide' the tag.
    assert((getLangOpts().CPlusPlus || !isa<TagDecl>(PrevDecl)) &&
           "Received TagDecl when not in C++!");
    if (!isa<TagDecl>(PrevDecl) && isDeclInScope(PrevDecl, CurContext, S)) {
      if (isa<EnumConstantDecl>(PrevDecl))
        Diag(IdLoc, diag::err_redefinition_of_enumerator) << Id;
      else
        Diag(IdLoc, diag::err_redefinition) << Id;
      Diag(PrevDecl->getLocation(), diag::note_previous_definition);
      return nullptr;
    }
  }

  // C++ [class.mem]p15:
  // If T is the name of a class, then each of the following shall have a name 
  // different from T:
  // - every enumerator of every member of class T that is an unscoped 
  // enumerated type
  if (!TheEnumDecl->isScoped())
    DiagnoseClassNameShadow(TheEnumDecl->getDeclContext(),
                            DeclarationNameInfo(Id, IdLoc));
  
  EnumConstantDecl *New =
    CheckEnumConstant(TheEnumDecl, LastEnumConst, IdLoc, Id, Val);

  if (New) {
    // Process attributes.
    if (Attr) ProcessDeclAttributeList(S, New, Attr);

    // Register this decl in the current scope stack.
    New->setAccess(TheEnumDecl->getAccess());
    PushOnScopeChains(New, S);
  }

  ActOnDocumentableDecl(New);

  return New;
}

// Returns true when the enum initial expression does not trigger the
// duplicate enum warning.  A few common cases are exempted as follows:
// Element2 = Element1
// Element2 = Element1 + 1
// Element2 = Element1 - 1
// Where Element2 and Element1 are from the same enum.
static bool ValidDuplicateEnum(EnumConstantDecl *ECD, EnumDecl *Enum) {
  Expr *InitExpr = ECD->getInitExpr();
  if (!InitExpr)
    return true;
  InitExpr = InitExpr->IgnoreImpCasts();

  if (BinaryOperator *BO = dyn_cast<BinaryOperator>(InitExpr)) {
    if (!BO->isAdditiveOp())
      return true;
    IntegerLiteral *IL = dyn_cast<IntegerLiteral>(BO->getRHS());
    if (!IL)
      return true;
    if (IL->getValue() != 1)
      return true;

    InitExpr = BO->getLHS();
  }

  // This checks if the elements are from the same enum.
  DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(InitExpr);
  if (!DRE)
    return true;

  EnumConstantDecl *EnumConstant = dyn_cast<EnumConstantDecl>(DRE->getDecl());
  if (!EnumConstant)
    return true;

  if (cast<EnumDecl>(TagDecl::castFromDeclContext(ECD->getDeclContext())) !=
      Enum)
    return true;

  return false;
}

struct DupKey {
  int64_t val;
  bool isTombstoneOrEmptyKey;
  DupKey(int64_t val, bool isTombstoneOrEmptyKey)
    : val(val), isTombstoneOrEmptyKey(isTombstoneOrEmptyKey) {}
};

static DupKey GetDupKey(const llvm::APSInt& Val) {
  return DupKey(Val.isSigned() ? Val.getSExtValue() : Val.getZExtValue(),
                false);
}

struct DenseMapInfoDupKey {
  static DupKey getEmptyKey() { return DupKey(0, true); }
  static DupKey getTombstoneKey() { return DupKey(1, true); }
  static unsigned getHashValue(const DupKey Key) {
    return (unsigned)(Key.val * 37);
  }
  static bool isEqual(const DupKey& LHS, const DupKey& RHS) {
    return LHS.isTombstoneOrEmptyKey == RHS.isTombstoneOrEmptyKey &&
           LHS.val == RHS.val;
  }
};

// Emits a warning when an element is implicitly set a value that
// a previous element has already been set to.
static void CheckForDuplicateEnumValues(Sema &S, ArrayRef<Decl *> Elements,
                                        EnumDecl *Enum,
                                        QualType EnumType) {
  if (S.Diags.isIgnored(diag::warn_duplicate_enum_values, Enum->getLocation()))
    return;
  // Avoid anonymous enums
  if (!Enum->getIdentifier())
    return;

  // Only check for small enums.
  if (Enum->getNumPositiveBits() > 63 || Enum->getNumNegativeBits() > 64)
    return;

  typedef SmallVector<EnumConstantDecl *, 3> ECDVector;
  typedef SmallVector<ECDVector *, 3> DuplicatesVector;

  typedef llvm::PointerUnion<EnumConstantDecl*, ECDVector*> DeclOrVector;
  typedef llvm::DenseMap<DupKey, DeclOrVector, DenseMapInfoDupKey>
          ValueToVectorMap;

  DuplicatesVector DupVector;
  ValueToVectorMap EnumMap;

  // Populate the EnumMap with all values represented by enum constants without
  // an initialier.
  for (unsigned i = 0, e = Elements.size(); i != e; ++i) {
    EnumConstantDecl *ECD = cast_or_null<EnumConstantDecl>(Elements[i]);

    // Null EnumConstantDecl means a previous diagnostic has been emitted for
    // this constant.  Skip this enum since it may be ill-formed.
    if (!ECD) {
      return;
    }

    if (ECD->getInitExpr())
      continue;

    DupKey Key = GetDupKey(ECD->getInitVal());
    DeclOrVector &Entry = EnumMap[Key];

    // First time encountering this value.
    if (Entry.isNull())
      Entry = ECD;
  }

  // Create vectors for any values that has duplicates.
  for (unsigned i = 0, e = Elements.size(); i != e; ++i) {
    EnumConstantDecl *ECD = cast<EnumConstantDecl>(Elements[i]);
    if (!ValidDuplicateEnum(ECD, Enum))
      continue;

    DupKey Key = GetDupKey(ECD->getInitVal());

    DeclOrVector& Entry = EnumMap[Key];
    if (Entry.isNull())
      continue;

    if (EnumConstantDecl *D = Entry.dyn_cast<EnumConstantDecl*>()) {
      // Ensure constants are different.
      if (D == ECD)
        continue;

      // Create new vector and push values onto it.
      ECDVector *Vec = new ECDVector();
      Vec->push_back(D);
      Vec->push_back(ECD);

      // Update entry to point to the duplicates vector.
      Entry = Vec;

      // Store the vector somewhere we can consult later for quick emission of
      // diagnostics.
      DupVector.push_back(Vec);
      continue;
    }

    ECDVector *Vec = Entry.get<ECDVector*>();
    // Make sure constants are not added more than once.
    if (*Vec->begin() == ECD)
      continue;

    Vec->push_back(ECD);
  }

  // Emit diagnostics.
  for (DuplicatesVector::iterator DupVectorIter = DupVector.begin(),
                                  DupVectorEnd = DupVector.end();
       DupVectorIter != DupVectorEnd; ++DupVectorIter) {
    ECDVector *Vec = *DupVectorIter;
    assert(Vec->size() > 1 && "ECDVector should have at least 2 elements.");

    // Emit warning for one enum constant.
    ECDVector::iterator I = Vec->begin();
    S.Diag((*I)->getLocation(), diag::warn_duplicate_enum_values)
      << (*I)->getName() << (*I)->getInitVal().toString(10)
      << (*I)->getSourceRange();
    ++I;

    // Emit one note for each of the remaining enum constants with
    // the same value.
    for (ECDVector::iterator E = Vec->end(); I != E; ++I)
      S.Diag((*I)->getLocation(), diag::note_duplicate_element)
        << (*I)->getName() << (*I)->getInitVal().toString(10)
        << (*I)->getSourceRange();
    delete Vec;
  }
}

bool
Sema::IsValueInFlagEnum(const EnumDecl *ED, const llvm::APInt &Val,
                        bool AllowMask) const {
  FlagEnumAttr *FEAttr = ED->getAttr<FlagEnumAttr>();
  assert(FEAttr && "looking for value in non-flag enum");

  llvm::APInt FlagMask = ~FEAttr->getFlagBits();
  unsigned Width = FlagMask.getBitWidth();

  // We will try a zero-extended value for the regular check first.
  llvm::APInt ExtVal = Val.zextOrSelf(Width);

  // A value is in a flag enum if either its bits are a subset of the enum's
  // flag bits (the first condition) or we are allowing masks and the same is
  // true of its complement (the second condition). When masks are allowed, we
  // allow the common idiom of ~(enum1 | enum2) to be a valid enum value.
  //
  // While it's true that any value could be used as a mask, the assumption is
  // that a mask will have all of the insignificant bits set. Anything else is
  // likely a logic error.
  if (!(FlagMask & ExtVal))
    return true;

  if (AllowMask) {
    // Try a one-extended value instead. This can happen if the enum is wider
    // than the constant used, in C with extensions to allow for wider enums.
    // The mask will still have the correct behaviour, so we give the user the
    // benefit of the doubt.
    //
    // FIXME: This heuristic can cause weird results if the enum was extended
    // to a larger type and is signed, because then bit-masks of smaller types
    // that get extended will fall out of range (e.g. ~0x1u). We currently don't
    // detect that case and will get a false positive for it. In most cases,
    // though, it can be fixed by making it a signed type (e.g. ~0x1), so it may
    // be fine just to accept this as a warning.
    ExtVal |= llvm::APInt::getHighBitsSet(Width, Width - Val.getBitWidth());
    if (!(FlagMask & ~ExtVal))
      return true;
  }

  return false;
}

void Sema::ActOnEnumBody(SourceLocation EnumLoc, SourceLocation LBraceLoc,
                         SourceLocation RBraceLoc, Decl *EnumDeclX,
                         ArrayRef<Decl *> Elements,
                         Scope *S, AttributeList *Attr) {
  EnumDecl *Enum = cast<EnumDecl>(EnumDeclX);
  QualType EnumType = Context.getTypeDeclType(Enum);

  if (Attr)
    ProcessDeclAttributeList(S, Enum, Attr);

  if (Enum->isDependentType()) {
    for (unsigned i = 0, e = Elements.size(); i != e; ++i) {
      EnumConstantDecl *ECD =
        cast_or_null<EnumConstantDecl>(Elements[i]);
      if (!ECD) continue;

      ECD->setType(EnumType);
    }

    Enum->completeDefinition(Context.DependentTy, Context.DependentTy, 0, 0);
    return;
  }

  // TODO: If the result value doesn't fit in an int, it must be a long or long
  // long value.  ISO C does not support this, but GCC does as an extension,
  // emit a warning.
  unsigned IntWidth = Context.getTargetInfo().getIntWidth();
  unsigned CharWidth = Context.getTargetInfo().getCharWidth();
  unsigned ShortWidth = Context.getTargetInfo().getShortWidth();

  // Verify that all the values are okay, compute the size of the values, and
  // reverse the list.
  unsigned NumNegativeBits = 0;
  unsigned NumPositiveBits = 0;

  // Keep track of whether all elements have type int.
  bool AllElementsInt = true;

  for (unsigned i = 0, e = Elements.size(); i != e; ++i) {
    EnumConstantDecl *ECD =
      cast_or_null<EnumConstantDecl>(Elements[i]);
    if (!ECD) continue;  // Already issued a diagnostic.

    const llvm::APSInt &InitVal = ECD->getInitVal();

    // Keep track of the size of positive and negative values.
    if (InitVal.isUnsigned() || InitVal.isNonNegative())
      NumPositiveBits = std::max(NumPositiveBits,
                                 (unsigned)InitVal.getActiveBits());
    else
      NumNegativeBits = std::max(NumNegativeBits,
                                 (unsigned)InitVal.getMinSignedBits());

    // Keep track of whether every enum element has type int (very commmon).
    if (AllElementsInt)
      AllElementsInt = ECD->getType() == Context.IntTy;
  }

  // Figure out the type that should be used for this enum.
  QualType BestType;
  unsigned BestWidth;

  // C++0x N3000 [conv.prom]p3:
  //   An rvalue of an unscoped enumeration type whose underlying
  //   type is not fixed can be converted to an rvalue of the first
  //   of the following types that can represent all the values of
  //   the enumeration: int, unsigned int, long int, unsigned long
  //   int, long long int, or unsigned long long int.
  // C99 6.4.4.3p2:
  //   An identifier declared as an enumeration constant has type int.
  // The C99 rule is modified by a gcc extension 
  QualType BestPromotionType;

  bool Packed = Enum->hasAttr<PackedAttr>();
  // -fshort-enums is the equivalent to specifying the packed attribute on all
  // enum definitions.
  if (LangOpts.ShortEnums)
    Packed = true;

  if (Enum->isFixed()) {
    BestType = Enum->getIntegerType();
    if (BestType->isPromotableIntegerType())
      BestPromotionType = Context.getPromotedIntegerType(BestType);
    else
      BestPromotionType = BestType;

    BestWidth = Context.getIntWidth(BestType);
  }
  else if (NumNegativeBits) {
    // If there is a negative value, figure out the smallest integer type (of
    // int/long/longlong) that fits.
    // If it's packed, check also if it fits a char or a short.
    if (Packed && NumNegativeBits <= CharWidth && NumPositiveBits < CharWidth) {
      BestType = Context.SignedCharTy;
      BestWidth = CharWidth;
    } else if (Packed && NumNegativeBits <= ShortWidth &&
               NumPositiveBits < ShortWidth) {
      BestType = Context.ShortTy;
      BestWidth = ShortWidth;
    } else if (NumNegativeBits <= IntWidth && NumPositiveBits < IntWidth) {
      BestType = Context.IntTy;
      BestWidth = IntWidth;
    } else {
      BestWidth = Context.getTargetInfo().getLongWidth();

      if (NumNegativeBits <= BestWidth && NumPositiveBits < BestWidth) {
        BestType = Context.LongTy;
      } else {
        BestWidth = Context.getTargetInfo().getLongLongWidth();

        if (NumNegativeBits > BestWidth || NumPositiveBits >= BestWidth)
          Diag(Enum->getLocation(), diag::ext_enum_too_large);
        BestType = Context.LongLongTy;
      }
    }
    BestPromotionType = (BestWidth <= IntWidth ? Context.IntTy : BestType);
  } else {
    // If there is no negative value, figure out the smallest type that fits
    // all of the enumerator values.
    // If it's packed, check also if it fits a char or a short.
    if (Packed && NumPositiveBits <= CharWidth) {
      BestType = Context.UnsignedCharTy;
      BestPromotionType = Context.IntTy;
      BestWidth = CharWidth;
    } else if (Packed && NumPositiveBits <= ShortWidth) {
      BestType = Context.UnsignedShortTy;
      BestPromotionType = Context.IntTy;
      BestWidth = ShortWidth;
    } else if (NumPositiveBits <= IntWidth) {
      BestType = Context.UnsignedIntTy;
      BestWidth = IntWidth;
      BestPromotionType
        = (NumPositiveBits == BestWidth || !getLangOpts().CPlusPlus)
                           ? Context.UnsignedIntTy : Context.IntTy;
    } else if (NumPositiveBits <=
               (BestWidth = Context.getTargetInfo().getLongWidth())) {
      BestType = Context.UnsignedLongTy;
      BestPromotionType
        = (NumPositiveBits == BestWidth || !getLangOpts().CPlusPlus)
                           ? Context.UnsignedLongTy : Context.LongTy;
    } else {
      BestWidth = Context.getTargetInfo().getLongLongWidth();
      assert(NumPositiveBits <= BestWidth &&
             "How could an initializer get larger than ULL?");
      BestType = Context.UnsignedLongLongTy;
      BestPromotionType
        = (NumPositiveBits == BestWidth || !getLangOpts().CPlusPlus)
                           ? Context.UnsignedLongLongTy : Context.LongLongTy;
    }
  }

  FlagEnumAttr *FEAttr = Enum->getAttr<FlagEnumAttr>();
  if (FEAttr)
    FEAttr->getFlagBits() = llvm::APInt(BestWidth, 0);

  // Loop over all of the enumerator constants, changing their types to match
  // the type of the enum if needed. If we have a flag type, we also prepare the
  // FlagBits cache.
  for (auto *D : Elements) {
    auto *ECD = cast_or_null<EnumConstantDecl>(D);
    if (!ECD) continue;  // Already issued a diagnostic.

    // Standard C says the enumerators have int type, but we allow, as an
    // extension, the enumerators to be larger than int size.  If each
    // enumerator value fits in an int, type it as an int, otherwise type it the
    // same as the enumerator decl itself.  This means that in "enum { X = 1U }"
    // that X has type 'int', not 'unsigned'.

    // Determine whether the value fits into an int.
    llvm::APSInt InitVal = ECD->getInitVal();

    // If it fits into an integer type, force it.  Otherwise force it to match
    // the enum decl type.
    QualType NewTy;
    unsigned NewWidth;
    bool NewSign;
    if (!getLangOpts().CPlusPlus &&
        !Enum->isFixed() &&
        isRepresentableIntegerValue(Context, InitVal, Context.IntTy)) {
      NewTy = Context.IntTy;
      NewWidth = IntWidth;
      NewSign = true;
    } else if (ECD->getType() == BestType) {
      // Already the right type!
      if (getLangOpts().CPlusPlus)
        // C++ [dcl.enum]p4: Following the closing brace of an
        // enum-specifier, each enumerator has the type of its
        // enumeration.
        ECD->setType(EnumType);
      goto flagbits;
    } else {
      NewTy = BestType;
      NewWidth = BestWidth;
      NewSign = BestType->isSignedIntegerOrEnumerationType();
    }

    // Adjust the APSInt value.
    InitVal = InitVal.extOrTrunc(NewWidth);
    InitVal.setIsSigned(NewSign);
    ECD->setInitVal(InitVal);

    // Adjust the Expr initializer and type.
    if (ECD->getInitExpr() &&
        !Context.hasSameType(NewTy, ECD->getInitExpr()->getType()))
      ECD->setInitExpr(ImplicitCastExpr::Create(Context, NewTy,
                                                CK_IntegralCast,
                                                ECD->getInitExpr(),
                                                /*base paths*/ nullptr,
                                                VK_RValue));
    if (getLangOpts().CPlusPlus)
      // C++ [dcl.enum]p4: Following the closing brace of an
      // enum-specifier, each enumerator has the type of its
      // enumeration.
      ECD->setType(EnumType);
    else
      ECD->setType(NewTy);

flagbits:
    // Check to see if we have a constant with exactly one bit set. Note that x
    // & (x - 1) will be nonzero if and only if x has more than one bit set.
    if (FEAttr) {
      llvm::APInt ExtVal = InitVal.zextOrSelf(BestWidth);
      if (ExtVal != 0 && !(ExtVal & (ExtVal - 1))) {
        FEAttr->getFlagBits() |= ExtVal;
      }
    }
  }

  if (FEAttr) {
    for (Decl *D : Elements) {
      EnumConstantDecl *ECD = cast_or_null<EnumConstantDecl>(D);
      if (!ECD) continue;  // Already issued a diagnostic.

      llvm::APSInt InitVal = ECD->getInitVal();
      if (InitVal != 0 && !IsValueInFlagEnum(Enum, InitVal, true))
        Diag(ECD->getLocation(), diag::warn_flag_enum_constant_out_of_range)
          << ECD << Enum;
    }
  }



  Enum->completeDefinition(BestType, BestPromotionType,
                           NumPositiveBits, NumNegativeBits);

  CheckForDuplicateEnumValues(*this, Elements, Enum, EnumType);

  // Now that the enum type is defined, ensure it's not been underaligned.
  if (Enum->hasAttrs())
    CheckAlignasUnderalignment(Enum);
}

Decl *Sema::ActOnFileScopeAsmDecl(Expr *expr,
                                  SourceLocation StartLoc,
                                  SourceLocation EndLoc) {
#if 1 // HLSL Change
  llvm_unreachable("HLSL parser does not produce file scope asm decl");
#else
  StringLiteral *AsmString = cast<StringLiteral>(expr);

  FileScopeAsmDecl *New = FileScopeAsmDecl::Create(Context, CurContext,
                                                   AsmString, StartLoc,
                                                   EndLoc);
  CurContext->addDecl(New);
  return New;
#endif // HLSL Change
}

static void checkModuleImportContext(Sema &S, Module *M,
                                     SourceLocation ImportLoc,
                                     DeclContext *DC) {
  if (auto *LSD = dyn_cast<LinkageSpecDecl>(DC)) {
    switch (LSD->getLanguage()) {
    case LinkageSpecDecl::lang_c:
      if (!M->IsExternC) {
        S.Diag(ImportLoc, diag::err_module_import_in_extern_c)
          << M->getFullModuleName();
        S.Diag(LSD->getLocStart(), diag::note_module_import_in_extern_c);
        return;
      }
      break;
    case LinkageSpecDecl::lang_cxx:
      break;
    }
    DC = LSD->getParent();
  }

  while (isa<LinkageSpecDecl>(DC))
    DC = DC->getParent();
  if (!isa<TranslationUnitDecl>(DC)) {
    S.Diag(ImportLoc, diag::err_module_import_not_at_top_level)
      << M->getFullModuleName() << DC;
    S.Diag(cast<Decl>(DC)->getLocStart(),
           diag::note_module_import_not_at_top_level)
      << DC;
  }
}

DeclResult Sema::ActOnModuleImport(SourceLocation AtLoc, 
                                   SourceLocation ImportLoc, 
                                   ModuleIdPath Path) {
  Module *Mod =
      getModuleLoader().loadModule(ImportLoc, Path, Module::AllVisible,
                                   /*IsIncludeDirective=*/false);
  if (!Mod)
    return true;

  VisibleModules.setVisible(Mod, ImportLoc);

  checkModuleImportContext(*this, Mod, ImportLoc, CurContext);

  // FIXME: we should support importing a submodule within a different submodule
  // of the same top-level module. Until we do, make it an error rather than
  // silently ignoring the import.
  if (Mod->getTopLevelModuleName() == getLangOpts().CurrentModule)
    Diag(ImportLoc, diag::err_module_self_import)
        << Mod->getFullModuleName() << getLangOpts().CurrentModule;
  else if (Mod->getTopLevelModuleName() == getLangOpts().ImplementationOfModule)
    Diag(ImportLoc, diag::err_module_import_in_implementation)
        << Mod->getFullModuleName() << getLangOpts().ImplementationOfModule;

  SmallVector<SourceLocation, 2> IdentifierLocs;
  Module *ModCheck = Mod;
  for (unsigned I = 0, N = Path.size(); I != N; ++I) {
    // If we've run out of module parents, just drop the remaining identifiers.
    // We need the length to be consistent.
    if (!ModCheck)
      break;
    ModCheck = ModCheck->Parent;
    
    IdentifierLocs.push_back(Path[I].second);
  }

  ImportDecl *Import = ImportDecl::Create(Context, 
                                          Context.getTranslationUnitDecl(),
                                          AtLoc.isValid()? AtLoc : ImportLoc, 
                                          Mod, IdentifierLocs);
  Context.getTranslationUnitDecl()->addDecl(Import);
  return Import;
}

void Sema::ActOnModuleInclude(SourceLocation DirectiveLoc, Module *Mod) {
  checkModuleImportContext(*this, Mod, DirectiveLoc, CurContext);

  // Determine whether we're in the #include buffer for a module. The #includes
  // in that buffer do not qualify as module imports; they're just an
  // implementation detail of us building the module.
  //
  // FIXME: Should we even get ActOnModuleInclude calls for those?
  bool IsInModuleIncludes =
      TUKind == TU_Module &&
      getSourceManager().isWrittenInMainFile(DirectiveLoc);

  // If this module import was due to an inclusion directive, create an 
  // implicit import declaration to capture it in the AST.
  if (!IsInModuleIncludes) {
    TranslationUnitDecl *TU = getASTContext().getTranslationUnitDecl();
    ImportDecl *ImportD = ImportDecl::CreateImplicit(getASTContext(), TU,
                                                     DirectiveLoc, Mod,
                                                     DirectiveLoc);
    TU->addDecl(ImportD);
    Consumer.HandleImplicitImportDecl(ImportD);
  }
  
  getModuleLoader().makeModuleVisible(Mod, Module::AllVisible, DirectiveLoc);
  VisibleModules.setVisible(Mod, DirectiveLoc);
}

void Sema::ActOnModuleBegin(SourceLocation DirectiveLoc, Module *Mod) {
  checkModuleImportContext(*this, Mod, DirectiveLoc, CurContext);

  if (getLangOpts().ModulesLocalVisibility)
    VisibleModulesStack.push_back(std::move(VisibleModules));
  VisibleModules.setVisible(Mod, DirectiveLoc);
}

void Sema::ActOnModuleEnd(SourceLocation DirectiveLoc, Module *Mod) {
  checkModuleImportContext(*this, Mod, DirectiveLoc, CurContext);

  if (getLangOpts().ModulesLocalVisibility) {
    VisibleModules = std::move(VisibleModulesStack.back());
    VisibleModulesStack.pop_back();
    VisibleModules.setVisible(Mod, DirectiveLoc);
  }
}

void Sema::createImplicitModuleImportForErrorRecovery(SourceLocation Loc,
                                                      Module *Mod) {
  // Bail if we're not allowed to implicitly import a module here.
  if (isSFINAEContext() || !getLangOpts().ModulesErrorRecovery)
    return;

  // Create the implicit import declaration.
  TranslationUnitDecl *TU = getASTContext().getTranslationUnitDecl();
  ImportDecl *ImportD = ImportDecl::CreateImplicit(getASTContext(), TU,
                                                   Loc, Mod, Loc);
  TU->addDecl(ImportD);
  Consumer.HandleImplicitImportDecl(ImportD);

  // Make the module visible.
  getModuleLoader().makeModuleVisible(Mod, Module::AllVisible, Loc);
  VisibleModules.setVisible(Mod, Loc);
}

void Sema::ActOnPragmaRedefineExtname(IdentifierInfo* Name,
                                      IdentifierInfo* AliasName,
                                      SourceLocation PragmaLoc,
                                      SourceLocation NameLoc,
                                      SourceLocation AliasNameLoc) {
  NamedDecl *PrevDecl = LookupSingleName(TUScope, Name, NameLoc,
                                         LookupOrdinaryName);
  AsmLabelAttr *Attr =
      AsmLabelAttr::CreateImplicit(Context, AliasName->getName(), AliasNameLoc);

  // If a declaration that:
  // 1) declares a function or a variable
  // 2) has external linkage
  // already exists, add a label attribute to it.
  if (PrevDecl &&
      (isa<FunctionDecl>(PrevDecl) || isa<VarDecl>(PrevDecl)) &&
      PrevDecl->hasExternalFormalLinkage())
    PrevDecl->addAttr(Attr);
  // Otherwise, add a label atttibute to ExtnameUndeclaredIdentifiers.
  else
    (void)ExtnameUndeclaredIdentifiers.insert(std::make_pair(Name, Attr));
}

void Sema::ActOnPragmaWeakID(IdentifierInfo* Name,
                             SourceLocation PragmaLoc,
                             SourceLocation NameLoc) {
  Decl *PrevDecl = LookupSingleName(TUScope, Name, NameLoc, LookupOrdinaryName);

  if (PrevDecl) {
    PrevDecl->addAttr(WeakAttr::CreateImplicit(Context, PragmaLoc));
  } else {
    (void)WeakUndeclaredIdentifiers.insert(
      std::pair<IdentifierInfo*,WeakInfo>
        (Name, WeakInfo((IdentifierInfo*)nullptr, NameLoc)));
  }
}

void Sema::ActOnPragmaWeakAlias(IdentifierInfo* Name,
                                IdentifierInfo* AliasName,
                                SourceLocation PragmaLoc,
                                SourceLocation NameLoc,
                                SourceLocation AliasNameLoc) {
  Decl *PrevDecl = LookupSingleName(TUScope, AliasName, AliasNameLoc,
                                    LookupOrdinaryName);
  WeakInfo W = WeakInfo(Name, NameLoc);

  if (PrevDecl) {
    if (!PrevDecl->hasAttr<AliasAttr>())
      if (NamedDecl *ND = dyn_cast<NamedDecl>(PrevDecl))
        DeclApplyPragmaWeak(TUScope, ND, W);
  } else {
    (void)WeakUndeclaredIdentifiers.insert(
      std::pair<IdentifierInfo*,WeakInfo>(AliasName, W));
  }
}

Decl *Sema::getObjCDeclContext() const {
  return (dyn_cast_or_null<ObjCContainerDecl>(CurContext));
}

AvailabilityResult Sema::getCurContextAvailability() const {
  const Decl *D = cast_or_null<Decl>(getCurObjCLexicalContext());
  if (!D)
    return AR_Available;

  // If we are within an Objective-C method, we should consult
  // both the availability of the method as well as the
  // enclosing class.  If the class is (say) deprecated,
  // the entire method is considered deprecated from the
  // purpose of checking if the current context is deprecated.
  if (const ObjCMethodDecl *MD = dyn_cast<ObjCMethodDecl>(D)) {
    AvailabilityResult R = MD->getAvailability();
    if (R != AR_Available)
      return R;
    D = MD->getClassInterface();
  }
  // If we are within an Objective-c @implementation, it
  // gets the same availability context as the @interface.
  else if (const ObjCImplementationDecl *ID =
            dyn_cast<ObjCImplementationDecl>(D)) {
    D = ID->getClassInterface();
  }
  // Recover from user error.
  return D ? D->getAvailability() : AR_Available;
}

// HLSL Change Begin
void Sema::DiagnoseSemanticDecl(hlsl::SemanticDecl *Decl) {
  StringRef SemName = Decl->SemanticName;

  StringRef BaseSemName; // The 'FOO' in 'FOO1'
  uint32_t SemIndex;     // The '1' in 'FOO1'

  // Split name and index.
  hlsl::Semantic::DecomposeNameAndIndex(SemName, &BaseSemName, &SemIndex);

  // The valid semantic indices for SV_Target[n] are 0 <= n <= 7.
  if (BaseSemName.equals("SV_Target") && SemIndex > 7) {
    Diag(Decl->Loc, diag::err_hlsl_unsupported_semantic_index)
        << SemName << SemIndex << "7";
  }
}
// HLSL Change Ends
