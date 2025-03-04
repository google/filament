//===--- SemaExprMember.cpp - Semantic Analysis for Expressions -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements semantic analysis member access expressions.
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/Overload.h"
#include "clang/AST/ASTLambda.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/ExprObjC.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/ScopeInfo.h"
#include "clang/Sema/SemaInternal.h"
#include "clang/Sema/SemaHLSL.h" // HLSL Change
using namespace clang;
using namespace sema;

typedef llvm::SmallPtrSet<const CXXRecordDecl*, 4> BaseSet;
static bool BaseIsNotInSet(const CXXRecordDecl *Base, void *BasesPtr) {
  const BaseSet &Bases = *reinterpret_cast<const BaseSet*>(BasesPtr);
  return !Bases.count(Base->getCanonicalDecl());
}

/// Determines if the given class is provably not derived from all of
/// the prospective base classes.
static bool isProvablyNotDerivedFrom(Sema &SemaRef, CXXRecordDecl *Record,
                                     const BaseSet &Bases) {
  void *BasesPtr = const_cast<void*>(reinterpret_cast<const void*>(&Bases));
  return BaseIsNotInSet(Record, BasesPtr) &&
         Record->forallBases(BaseIsNotInSet, BasesPtr);
}

enum IMAKind {
  /// The reference is definitely not an instance member access.
  IMA_Static,

  /// The reference may be an implicit instance member access.
  IMA_Mixed,

  /// The reference may be to an instance member, but it might be invalid if
  /// so, because the context is not an instance method.
  IMA_Mixed_StaticContext,

  /// The reference may be to an instance member, but it is invalid if
  /// so, because the context is from an unrelated class.
  IMA_Mixed_Unrelated,

  /// The reference is definitely an implicit instance member access.
  IMA_Instance,

  /// The reference may be to an unresolved using declaration.
  IMA_Unresolved,

  /// The reference is a contextually-permitted abstract member reference.
  IMA_Abstract,

  /// The reference may be to an unresolved using declaration and the
  /// context is not an instance method.
  IMA_Unresolved_StaticContext,

  // The reference refers to a field which is not a member of the containing
  // class, which is allowed because we're in C++11 mode and the context is
  // unevaluated.
  IMA_Field_Uneval_Context,

  /// All possible referrents are instance members and the current
  /// context is not an instance method.
  IMA_Error_StaticContext,

  /// All possible referrents are instance members of an unrelated
  /// class.
  IMA_Error_Unrelated
};

/// The given lookup names class member(s) and is not being used for
/// an address-of-member expression.  Classify the type of access
/// according to whether it's possible that this reference names an
/// instance member.  This is best-effort in dependent contexts; it is okay to
/// conservatively answer "yes", in which case some errors will simply
/// not be caught until template-instantiation.
static IMAKind ClassifyImplicitMemberAccess(Sema &SemaRef,
                                            const LookupResult &R) {
  assert(!R.empty() && (*R.begin())->isCXXClassMember());

  DeclContext *DC = SemaRef.getFunctionLevelDeclContext();

  bool isStaticContext = SemaRef.CXXThisTypeOverride.isNull() &&
    (!isa<CXXMethodDecl>(DC) || cast<CXXMethodDecl>(DC)->isStatic());

  if (R.isUnresolvableResult())
    return isStaticContext ? IMA_Unresolved_StaticContext : IMA_Unresolved;

  // Collect all the declaring classes of instance members we find.
  bool hasNonInstance = false;
  bool isField = false;
  BaseSet Classes;
  for (LookupResult::iterator I = R.begin(), E = R.end(); I != E; ++I) {
    NamedDecl *D = *I;

    if (D->isCXXInstanceMember()) {
      isField |= isa<FieldDecl>(D) || isa<MSPropertyDecl>(D) ||
                 isa<IndirectFieldDecl>(D);

      CXXRecordDecl *R = cast<CXXRecordDecl>(D->getDeclContext());
      Classes.insert(R->getCanonicalDecl());
    }
    else
      hasNonInstance = true;
  }

  // If we didn't find any instance members, it can't be an implicit
  // member reference.
  if (Classes.empty())
    return IMA_Static;
  
  // C++11 [expr.prim.general]p12:
  //   An id-expression that denotes a non-static data member or non-static
  //   member function of a class can only be used:
  //   (...)
  //   - if that id-expression denotes a non-static data member and it
  //     appears in an unevaluated operand.
  //
  // This rule is specific to C++11.  However, we also permit this form
  // in unevaluated inline assembly operands, like the operand to a SIZE.
  IMAKind AbstractInstanceResult = IMA_Static; // happens to be 'false'
  assert(!AbstractInstanceResult);
  switch (SemaRef.ExprEvalContexts.back().Context) {
  case Sema::Unevaluated:
    if (isField && SemaRef.getLangOpts().CPlusPlus11)
      AbstractInstanceResult = IMA_Field_Uneval_Context;
    break;

  case Sema::UnevaluatedAbstract:
    AbstractInstanceResult = IMA_Abstract;
    break;

  case Sema::ConstantEvaluated:
  case Sema::PotentiallyEvaluated:
  case Sema::PotentiallyEvaluatedIfUsed:
    break;
  }

  // If the current context is not an instance method, it can't be
  // an implicit member reference.
  if (isStaticContext) {
    if (hasNonInstance)
      return IMA_Mixed_StaticContext;

    return AbstractInstanceResult ? AbstractInstanceResult
                                  : IMA_Error_StaticContext;
  }

  CXXRecordDecl *contextClass;
  if (CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(DC))
    contextClass = MD->getParent()->getCanonicalDecl();
  else
    contextClass = cast<CXXRecordDecl>(DC);

  // [class.mfct.non-static]p3: 
  // ...is used in the body of a non-static member function of class X,
  // if name lookup (3.4.1) resolves the name in the id-expression to a
  // non-static non-type member of some class C [...]
  // ...if C is not X or a base class of X, the class member access expression
  // is ill-formed.
  if (R.getNamingClass() &&
      contextClass->getCanonicalDecl() !=
        R.getNamingClass()->getCanonicalDecl()) {
    // If the naming class is not the current context, this was a qualified
    // member name lookup, and it's sufficient to check that we have the naming
    // class as a base class.
    Classes.clear();
    Classes.insert(R.getNamingClass()->getCanonicalDecl());
  }

  // If we can prove that the current context is unrelated to all the
  // declaring classes, it can't be an implicit member reference (in
  // which case it's an error if any of those members are selected).
  if (isProvablyNotDerivedFrom(SemaRef, contextClass, Classes))
    return hasNonInstance ? IMA_Mixed_Unrelated :
           AbstractInstanceResult ? AbstractInstanceResult :
                                    IMA_Error_Unrelated;

  return (hasNonInstance ? IMA_Mixed : IMA_Instance);
}

/// Diagnose a reference to a field with no object available.
static void diagnoseInstanceReference(Sema &SemaRef,
                                      const CXXScopeSpec &SS,
                                      NamedDecl *Rep,
                                      const DeclarationNameInfo &nameInfo) {
  SourceLocation Loc = nameInfo.getLoc();
  SourceRange Range(Loc);
  if (SS.isSet()) Range.setBegin(SS.getRange().getBegin());

  // Look through using shadow decls and aliases.
  Rep = Rep->getUnderlyingDecl();

  DeclContext *FunctionLevelDC = SemaRef.getFunctionLevelDeclContext();
  CXXMethodDecl *Method = dyn_cast<CXXMethodDecl>(FunctionLevelDC);
  CXXRecordDecl *ContextClass = Method ? Method->getParent() : nullptr;
  CXXRecordDecl *RepClass = dyn_cast<CXXRecordDecl>(Rep->getDeclContext());

  bool InStaticMethod = Method && Method->isStatic();
  bool IsField = isa<FieldDecl>(Rep) || isa<IndirectFieldDecl>(Rep);

  if (IsField && InStaticMethod)
    // "invalid use of member 'x' in static member function"
    SemaRef.Diag(Loc, diag::err_invalid_member_use_in_static_method)
        << Range << nameInfo.getName();
  else if (ContextClass && RepClass && SS.isEmpty() && !InStaticMethod &&
           !RepClass->Equals(ContextClass) && RepClass->Encloses(ContextClass))
    // Unqualified lookup in a non-static member function found a member of an
    // enclosing class.
    SemaRef.Diag(Loc, diag::err_nested_non_static_member_use)
      << IsField << RepClass << nameInfo.getName() << ContextClass << Range;
  else if (IsField)
    SemaRef.Diag(Loc, diag::err_invalid_non_static_member_use)
      << nameInfo.getName() << Range;
  else
    SemaRef.Diag(Loc, diag::err_member_call_without_object)
      << Range;
}

/// Builds an expression which might be an implicit member expression.
ExprResult
Sema::BuildPossibleImplicitMemberExpr(const CXXScopeSpec &SS,
                                      SourceLocation TemplateKWLoc,
                                      LookupResult &R,
                                const TemplateArgumentListInfo *TemplateArgs) {
  switch (ClassifyImplicitMemberAccess(*this, R)) {
  case IMA_Instance:
    return BuildImplicitMemberExpr(SS, TemplateKWLoc, R, TemplateArgs, true);

  case IMA_Mixed:
  case IMA_Mixed_Unrelated:
  case IMA_Unresolved:
    return BuildImplicitMemberExpr(SS, TemplateKWLoc, R, TemplateArgs, false);

  case IMA_Field_Uneval_Context:
    Diag(R.getNameLoc(), diag::warn_cxx98_compat_non_static_member_use)
      << R.getLookupNameInfo().getName();
    LLVM_FALLTHROUGH; // HLSL Change
  case IMA_Static:
  case IMA_Abstract:
  case IMA_Mixed_StaticContext:
  case IMA_Unresolved_StaticContext:
    if (TemplateArgs || TemplateKWLoc.isValid())
      return BuildTemplateIdExpr(SS, TemplateKWLoc, R, false, TemplateArgs);
    return BuildDeclarationNameExpr(SS, R, false);

  case IMA_Error_StaticContext:
  case IMA_Error_Unrelated:
    diagnoseInstanceReference(*this, SS, R.getRepresentativeDecl(),
                              R.getLookupNameInfo());
    return ExprError();
  }

  llvm_unreachable("unexpected instance member access kind");
}

/// Check an ext-vector component access expression.
///
/// VK should be set in advance to the value kind of the base
/// expression.
static QualType
CheckExtVectorComponent(Sema &S, QualType baseType, ExprValueKind &VK,
                        SourceLocation OpLoc, const IdentifierInfo *CompName,
                        SourceLocation CompLoc) {
  // FIXME: Share logic with ExtVectorElementExpr::containsDuplicateElements,
  // see FIXME there.
  //
  // FIXME: This logic can be greatly simplified by splitting it along
  // halving/not halving and reworking the component checking.
  const ExtVectorType *vecType = baseType->getAs<ExtVectorType>();

  // The vector accessor can't exceed the number of elements.
  const char *compStr = CompName->getNameStart();

  // This flag determines whether or not the component is one of the four
  // special names that indicate a subset of exactly half the elements are
  // to be selected.
  bool HalvingSwizzle = false;

  // This flag determines whether or not CompName has an 's' char prefix,
  // indicating that it is a string of hex values to be used as vector indices.
  bool HexSwizzle = (*compStr == 's' || *compStr == 'S') && compStr[1];

  bool HasRepeated = false;
  bool HasIndex[16] = {};

  int Idx;

  // Check that we've found one of the special components, or that the component
  // names must come from the same set.
  if (!strcmp(compStr, "hi") || !strcmp(compStr, "lo") ||
      !strcmp(compStr, "even") || !strcmp(compStr, "odd")) {
    HalvingSwizzle = true;
  } else if (!HexSwizzle &&
             (Idx = vecType->getPointAccessorIdx(*compStr)) != -1) {
    ExtVectorType::AccessorSet currentSet = vecType->getPointAccessorSet(*compStr);  // HLSL Change
    do {
      if (HasIndex[Idx]) HasRepeated = true;
      // HLSL Change Starts
      if (currentSet != vecType->getPointAccessorSet(*compStr)) {
        S.Diag(OpLoc, diag::err_ext_vector_component_name_mixedsets)
          << SourceRange(CompLoc);
        return QualType();
      }
      // HLSL Change Ends
      HasIndex[Idx] = true;
      compStr++;
    } while (*compStr && (Idx = vecType->getPointAccessorIdx(*compStr)) != -1);
  } else {
    // HLSL Note - is this parsing digits?
    if (HexSwizzle) compStr++;
    while ((Idx = vecType->getNumericAccessorIdx(*compStr)) != -1) {
      if (HasIndex[Idx]) HasRepeated = true;
      HasIndex[Idx] = true;
      compStr++;
    }
  }

  if (!HalvingSwizzle && *compStr) {
    // We didn't get to the end of the string. This means the component names
    // didn't come from the same set *or* we encountered an illegal name.
    S.Diag(OpLoc, diag::err_ext_vector_component_name_illegal)
      << StringRef(compStr, 1) << SourceRange(CompLoc);
    return QualType();
  }

  // Ensure no component accessor exceeds the width of the vector type it
  // operates on.
  if (!HalvingSwizzle) {
    compStr = CompName->getNameStart();

    if (HexSwizzle)
      compStr++;

    while (*compStr) {
      if (!vecType->isAccessorWithinNumElements(*compStr++)) {
        S.Diag(OpLoc, diag::err_ext_vector_component_exceeds_length)
          << baseType << SourceRange(CompLoc);
        return QualType();
      }
    }
  }

  // The component accessor looks fine - now we need to compute the actual type.
  // The vector type is implied by the component accessor. For example,
  // vec4.b is a float, vec4.xy is a vec2, vec4.rgb is a vec3, etc.
  // vec4.s0 is a float, vec4.s23 is a vec3, etc.
  // vec4.hi, vec4.lo, vec4.e, and vec4.o all return vec2.
  unsigned CompSize = HalvingSwizzle ? (vecType->getNumElements() + 1) / 2
                                     : CompName->getLength();
  if (HexSwizzle)
    CompSize--;

  if (CompSize == 1)
    return vecType->getElementType();

  if (HasRepeated) VK = VK_RValue;

  QualType VT = S.Context.getExtVectorType(vecType->getElementType(), CompSize);
  // Now look up the TypeDefDecl from the vector type. Without this,
  // diagostics look bad. We want extended vector types to appear built-in.
  for (Sema::ExtVectorDeclsType::iterator 
         I = S.ExtVectorDecls.begin(S.getExternalSource()),
         E = S.ExtVectorDecls.end(); 
       I != E; ++I) {
    if ((*I)->getUnderlyingType() == VT)
      return S.Context.getTypedefType(*I);
  }
  
  return VT; // should never get here (a typedef type should always be found).
}

static Decl *FindGetterSetterNameDeclFromProtocolList(const ObjCProtocolDecl*PDecl,
                                                IdentifierInfo *Member,
                                                const Selector &Sel,
                                                ASTContext &Context) {
  if (Member)
    if (ObjCPropertyDecl *PD = PDecl->FindPropertyDeclaration(Member))
      return PD;
  if (ObjCMethodDecl *OMD = PDecl->getInstanceMethod(Sel))
    return OMD;

  for (const auto *I : PDecl->protocols()) {
    if (Decl *D = FindGetterSetterNameDeclFromProtocolList(I, Member, Sel,
                                                           Context))
      return D;
  }
  return nullptr;
}

static Decl *FindGetterSetterNameDecl(const ObjCObjectPointerType *QIdTy,
                                      IdentifierInfo *Member,
                                      const Selector &Sel,
                                      ASTContext &Context) {
  // Check protocols on qualified interfaces.
  Decl *GDecl = nullptr;
  for (const auto *I : QIdTy->quals()) {
    if (Member)
      if (ObjCPropertyDecl *PD = I->FindPropertyDeclaration(Member)) {
        GDecl = PD;
        break;
      }
    // Also must look for a getter or setter name which uses property syntax.
    if (ObjCMethodDecl *OMD = I->getInstanceMethod(Sel)) {
      GDecl = OMD;
      break;
    }
  }
  if (!GDecl) {
    for (const auto *I : QIdTy->quals()) {
      // Search in the protocol-qualifier list of current protocol.
      GDecl = FindGetterSetterNameDeclFromProtocolList(I, Member, Sel, Context);
      if (GDecl)
        return GDecl;
    }
  }
  return GDecl;
}

ExprResult
Sema::ActOnDependentMemberExpr(Expr *BaseExpr, QualType BaseType,
                               bool IsArrow, SourceLocation OpLoc,
                               const CXXScopeSpec &SS,
                               SourceLocation TemplateKWLoc,
                               NamedDecl *FirstQualifierInScope,
                               const DeclarationNameInfo &NameInfo,
                               const TemplateArgumentListInfo *TemplateArgs) {
  // Even in dependent contexts, try to diagnose base expressions with
  // obviously wrong types, e.g.:
  //
  // T* t;
  // t.f;
  //
  // In Obj-C++, however, the above expression is valid, since it could be
  // accessing the 'f' property if T is an Obj-C interface. The extra check
  // allows this, while still reporting an error if T is a struct pointer.
  if (!IsArrow) {
    const PointerType *PT = BaseType->getAs<PointerType>();
    if (PT && (!getLangOpts().ObjC1 ||
               PT->getPointeeType()->isRecordType())) {
      assert(BaseExpr && "cannot happen with implicit member accesses");
      Diag(OpLoc, diag::err_typecheck_member_reference_struct_union)
        << BaseType << BaseExpr->getSourceRange() << NameInfo.getSourceRange();
      return ExprError();
    }
  }

  assert(BaseType->isDependentType() ||
         NameInfo.getName().isDependentName() ||
         isDependentScopeSpecifier(SS));

  // Get the type being accessed in BaseType.  If this is an arrow, the BaseExpr
  // must have pointer type, and the accessed type is the pointee.
  return CXXDependentScopeMemberExpr::Create(
      Context, BaseExpr, BaseType, IsArrow, OpLoc,
      SS.getWithLocInContext(Context), TemplateKWLoc, FirstQualifierInScope,
      NameInfo, TemplateArgs);
}

/// We know that the given qualified member reference points only to
/// declarations which do not belong to the static type of the base
/// expression.  Diagnose the problem.
static void DiagnoseQualifiedMemberReference(Sema &SemaRef,
                                             Expr *BaseExpr,
                                             QualType BaseType,
                                             const CXXScopeSpec &SS,
                                             NamedDecl *rep,
                                       const DeclarationNameInfo &nameInfo) {
  // If this is an implicit member access, use a different set of
  // diagnostics.
  if (!BaseExpr)
    return diagnoseInstanceReference(SemaRef, SS, rep, nameInfo);

  SemaRef.Diag(nameInfo.getLoc(), diag::err_qualified_member_of_unrelated)
    << SS.getRange() << rep << BaseType;
}

// Check whether the declarations we found through a nested-name
// specifier in a member expression are actually members of the base
// type.  The restriction here is:
//
//   C++ [expr.ref]p2:
//     ... In these cases, the id-expression shall name a
//     member of the class or of one of its base classes.
//
// So it's perfectly legitimate for the nested-name specifier to name
// an unrelated class, and for us to find an overload set including
// decls from classes which are not superclasses, as long as the decl
// we actually pick through overload resolution is from a superclass.
bool Sema::CheckQualifiedMemberReference(Expr *BaseExpr,
                                         QualType BaseType,
                                         const CXXScopeSpec &SS,
                                         const LookupResult &R) {
  BaseType = BaseType.getNonReferenceType(); // HLSL Change
  CXXRecordDecl *BaseRecord =
    cast_or_null<CXXRecordDecl>(computeDeclContext(BaseType));
  if (!BaseRecord) {
    // We can't check this yet because the base type is still
    // dependent.
    assert(BaseType->isDependentType());
    return false;
  }

  for (LookupResult::iterator I = R.begin(), E = R.end(); I != E; ++I) {
    // If this is an implicit member reference and we find a
    // non-instance member, it's not an error.
    if (!BaseExpr && !(*I)->isCXXInstanceMember())
      return false;

    // Note that we use the DC of the decl, not the underlying decl.
    DeclContext *DC = (*I)->getDeclContext();
    while (DC->isTransparentContext())
      DC = DC->getParent();

    if (!DC->isRecord())
      continue;

    CXXRecordDecl *MemberRecord = cast<CXXRecordDecl>(DC)->getCanonicalDecl();
    if (BaseRecord->getCanonicalDecl() == MemberRecord ||
        !BaseRecord->isProvablyNotDerivedFrom(MemberRecord))
      return false;
  }

  DiagnoseQualifiedMemberReference(*this, BaseExpr, BaseType, SS,
                                   R.getRepresentativeDecl(),
                                   R.getLookupNameInfo());
  return true;
}

namespace {

// Callback to only accept typo corrections that are either a ValueDecl or a
// FunctionTemplateDecl and are declared in the current record or, for a C++
// classes, one of its base classes.
class RecordMemberExprValidatorCCC : public CorrectionCandidateCallback {
public:
  explicit RecordMemberExprValidatorCCC(const RecordType *RTy)
      : Record(RTy->getDecl()) {
    // Don't add bare keywords to the consumer since they will always fail
    // validation by virtue of not being associated with any decls.
    WantTypeSpecifiers = false;
    WantExpressionKeywords = false;
    WantCXXNamedCasts = false;
    WantFunctionLikeCasts = false;
    WantRemainingKeywords = false;
  }

  bool ValidateCandidate(const TypoCorrection &candidate) override {
    NamedDecl *ND = candidate.getCorrectionDecl();
    // Don't accept candidates that cannot be member functions, constants,
    // variables, or templates.
    if (!ND || !(isa<ValueDecl>(ND) || isa<FunctionTemplateDecl>(ND)))
      return false;

    // Accept candidates that occur in the current record.
    if (Record->containsDecl(ND))
      return true;

    if (const CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(Record)) {
      // Accept candidates that occur in any of the current class' base classes.
      for (const auto &BS : RD->bases()) {
        if (const RecordType *BSTy =
                dyn_cast_or_null<RecordType>(BS.getType().getTypePtrOrNull())) {
          if (BSTy->getDecl()->containsDecl(ND))
            return true;
        }
      }
    }

    return false;
  }

private:
  const RecordDecl *const Record;
};

}

static bool LookupMemberExprInRecord(Sema &SemaRef, LookupResult &R,
                                     Expr *BaseExpr,
                                     const RecordType *RTy,
                                     SourceLocation OpLoc, bool IsArrow,
                                     CXXScopeSpec &SS, bool HasTemplateArgs,
                                     TypoExpr *&TE) {
  SourceRange BaseRange = BaseExpr ? BaseExpr->getSourceRange() : SourceRange();
  RecordDecl *RDecl = RTy->getDecl();
  if (!SemaRef.isThisOutsideMemberFunctionBody(QualType(RTy, 0)) &&
      SemaRef.RequireCompleteType(OpLoc, QualType(RTy, 0),
                                  diag::err_typecheck_incomplete_tag,
                                  BaseRange))
    return true;

  if (HasTemplateArgs) {
    // LookupTemplateName doesn't expect these both to exist simultaneously.
    QualType ObjectType = SS.isSet() ? QualType() : QualType(RTy, 0);

    bool MOUS;
    SemaRef.LookupTemplateName(R, nullptr, SS, ObjectType, false, MOUS);
    return false;
  }

  DeclContext *DC = RDecl;
  if (SS.isSet()) {
    // If the member name was a qualified-id, look into the
    // nested-name-specifier.
    DC = SemaRef.computeDeclContext(SS, false);

    if (SemaRef.RequireCompleteDeclContext(SS, DC)) {
      SemaRef.Diag(SS.getRange().getEnd(), diag::err_typecheck_incomplete_tag)
          << SS.getRange() << DC;
      return true;
    }

    assert(DC && "Cannot handle non-computable dependent contexts in lookup");

    if (!isa<TypeDecl>(DC)) {
      SemaRef.Diag(R.getNameLoc(), diag::err_qualified_member_nonclass)
          << DC << SS.getRange();
      return true;
    }
  }

  // The record definition is complete, now look up the member.
  SemaRef.LookupQualifiedName(R, DC, SS);

  if (!R.empty())
    return false;

  DeclarationName Typo = R.getLookupName();
  SourceLocation TypoLoc = R.getNameLoc();
  TE = SemaRef.CorrectTypoDelayed(
      R.getLookupNameInfo(), R.getLookupKind(), nullptr, &SS,
      llvm::make_unique<RecordMemberExprValidatorCCC>(RTy),
      [=, &SemaRef](const TypoCorrection &TC) {
        if (TC) {
          assert(!TC.isKeyword() &&
                 "Got a keyword as a correction for a member!");
          bool DroppedSpecifier =
              TC.WillReplaceSpecifier() &&
              Typo.getAsString() == TC.getAsString(SemaRef.getLangOpts());
          SemaRef.diagnoseTypo(TC, SemaRef.PDiag(diag::err_no_member_suggest)
                                       << Typo << DC << DroppedSpecifier
                                       << SS.getRange());
        } else {
          SemaRef.Diag(TypoLoc, diag::err_no_member) << Typo << DC << BaseRange;
        }
      },
      [=](Sema &SemaRef, TypoExpr *TE, TypoCorrection TC) mutable {
        R.clear(); // Ensure there's no decls lingering in the shared state.
        R.suppressDiagnostics();
        R.setLookupName(TC.getCorrection());
        for (NamedDecl *ND : TC)
          R.addDecl(ND);
        R.resolveKind();
        return SemaRef.BuildMemberReferenceExpr(
            BaseExpr, BaseExpr->getType(), OpLoc, IsArrow, SS, SourceLocation(),
            nullptr, R, nullptr);
      },
      Sema::CTK_ErrorRecovery, DC);

  return false;
}

static ExprResult LookupMemberExpr(Sema &S, LookupResult &R,
                                   ExprResult &BaseExpr, bool &IsArrow,
                                   SourceLocation OpLoc, CXXScopeSpec &SS,
                                   Decl *ObjCImpDecl, bool HasTemplateArgs);

ExprResult
Sema::BuildMemberReferenceExpr(Expr *Base, QualType BaseType,
                               SourceLocation OpLoc, bool IsArrow,
                               CXXScopeSpec &SS,
                               SourceLocation TemplateKWLoc,
                               NamedDecl *FirstQualifierInScope,
                               const DeclarationNameInfo &NameInfo,
                               const TemplateArgumentListInfo *TemplateArgs,
                               ActOnMemberAccessExtraArgs *ExtraArgs) {
  if (BaseType->isDependentType() ||
      (SS.isSet() && isDependentScopeSpecifier(SS)))
    return ActOnDependentMemberExpr(Base, BaseType,
                                    IsArrow, OpLoc,
                                    SS, TemplateKWLoc, FirstQualifierInScope,
                                    NameInfo, TemplateArgs);

  LookupResult R(*this, NameInfo, LookupMemberName);

  // Implicit member accesses.
  if (!Base) {
    TypoExpr *TE = nullptr;
    QualType RecordTy = BaseType;
    if (IsArrow) RecordTy = RecordTy->getAs<PointerType>()->getPointeeType();
    RecordTy = RecordTy.getNonReferenceType(); // HLSL Change - implicit this is a reference.
    if (LookupMemberExprInRecord(*this, R, nullptr,
                                 RecordTy->getAs<RecordType>(), OpLoc, IsArrow,
                                 SS, TemplateArgs != nullptr, TE))
      return ExprError();
    if (TE)
      return TE;

  // Explicit member accesses.
  } else {
    ExprResult BaseResult = Base;
    ExprResult Result = LookupMemberExpr(
        *this, R, BaseResult, IsArrow, OpLoc, SS,
        ExtraArgs ? ExtraArgs->ObjCImpDecl : nullptr,
        TemplateArgs != nullptr);

    if (BaseResult.isInvalid())
      return ExprError();
    Base = BaseResult.get();

    if (Result.isInvalid())
      return ExprError();

    if (Result.get())
      return Result;

    // LookupMemberExpr can modify Base, and thus change BaseType
    BaseType = Base->getType();
  }

  return BuildMemberReferenceExpr(Base, BaseType,
                                  OpLoc, IsArrow, SS, TemplateKWLoc,
                                  FirstQualifierInScope, R, TemplateArgs,
                                  false, ExtraArgs);
}

static ExprResult
BuildFieldReferenceExpr(Sema &S, Expr *BaseExpr, bool IsArrow,
                        SourceLocation OpLoc, const CXXScopeSpec &SS,
                        FieldDecl *Field, DeclAccessPair FoundDecl,
                        const DeclarationNameInfo &MemberNameInfo);

ExprResult
Sema::BuildAnonymousStructUnionMemberReference(const CXXScopeSpec &SS,
                                               SourceLocation loc,
                                               IndirectFieldDecl *indirectField,
                                               DeclAccessPair foundDecl,
                                               Expr *baseObjectExpr,
                                               SourceLocation opLoc) {
  // First, build the expression that refers to the base object.
  
  bool baseObjectIsPointer = false;
  Qualifiers baseQuals;
  
  // Case 1:  the base of the indirect field is not a field.
  VarDecl *baseVariable = indirectField->getVarDecl();
  CXXScopeSpec EmptySS;
  if (baseVariable) {
    assert(baseVariable->getType()->isRecordType());
    
    // In principle we could have a member access expression that
    // accesses an anonymous struct/union that's a static member of
    // the base object's class.  However, under the current standard,
    // static data members cannot be anonymous structs or unions.
    // Supporting this is as easy as building a MemberExpr here.
    assert(!baseObjectExpr && "anonymous struct/union is static data member?");
    
    DeclarationNameInfo baseNameInfo(DeclarationName(), loc);
    
    ExprResult result 
      = BuildDeclarationNameExpr(EmptySS, baseNameInfo, baseVariable);
    if (result.isInvalid()) return ExprError();
    
    baseObjectExpr = result.get();    
    baseObjectIsPointer = false;
    baseQuals = baseObjectExpr->getType().getQualifiers();
    
    // Case 2: the base of the indirect field is a field and the user
    // wrote a member expression.
  } else if (baseObjectExpr) {
    // The caller provided the base object expression. Determine
    // whether its a pointer and whether it adds any qualifiers to the
    // anonymous struct/union fields we're looking into.
    QualType objectType = baseObjectExpr->getType();
    
    if (const PointerType *ptr = objectType->getAs<PointerType>()) {
      baseObjectIsPointer = true;
      objectType = ptr->getPointeeType();
    } else {
      baseObjectIsPointer = false;
    }
    baseQuals = objectType.getQualifiers();
    
    // Case 3: the base of the indirect field is a field and we should
    // build an implicit member access.
  } else {
    // We've found a member of an anonymous struct/union that is
    // inside a non-anonymous struct/union, so in a well-formed
    // program our base object expression is "this".
    QualType ThisTy = getCurrentThisType();
    if (ThisTy.isNull()) {
      Diag(loc, diag::err_invalid_member_use_in_static_method)
        << indirectField->getDeclName();
      return ExprError();
    }
    
    // Our base object expression is "this".
    CheckCXXThisCapture(loc);
    baseObjectExpr 
      = new (Context) CXXThisExpr(loc, ThisTy, /*isImplicit=*/ true);
    baseObjectIsPointer = true;
    baseQuals = ThisTy->castAs<PointerType>()->getPointeeType().getQualifiers();
  }
  
  // Build the implicit member references to the field of the
  // anonymous struct/union.
  Expr *result = baseObjectExpr;
  IndirectFieldDecl::chain_iterator
  FI = indirectField->chain_begin(), FEnd = indirectField->chain_end();
  
  // Build the first member access in the chain with full information.
  if (!baseVariable) {
    FieldDecl *field = cast<FieldDecl>(*FI);
    
    // Make a nameInfo that properly uses the anonymous name.
    DeclarationNameInfo memberNameInfo(field->getDeclName(), loc);

    result = BuildFieldReferenceExpr(*this, result, baseObjectIsPointer,
                                     SourceLocation(), EmptySS, field,
                                     foundDecl, memberNameInfo).get();
    if (!result)
      return ExprError();

    // FIXME: check qualified member access
  }
  
  // In all cases, we should now skip the first declaration in the chain.
  ++FI;
  
  while (FI != FEnd) {
    FieldDecl *field = cast<FieldDecl>(*FI++);

    // FIXME: these are somewhat meaningless
    DeclarationNameInfo memberNameInfo(field->getDeclName(), loc);
    DeclAccessPair fakeFoundDecl =
        DeclAccessPair::make(field, field->getAccess());

    result =
        BuildFieldReferenceExpr(*this, result, /*isarrow*/ false,
                                SourceLocation(), (FI == FEnd ? SS : EmptySS),
                                field, fakeFoundDecl, memberNameInfo).get();
  }
  
  return result;
}

static ExprResult
BuildMSPropertyRefExpr(Sema &S, Expr *BaseExpr, bool IsArrow,
                       const CXXScopeSpec &SS,
                       MSPropertyDecl *PD,
                       const DeclarationNameInfo &NameInfo) {
  // Property names are always simple identifiers and therefore never
  // require any interesting additional storage.
  return new (S.Context) MSPropertyRefExpr(BaseExpr, PD, IsArrow,
                                           S.Context.PseudoObjectTy, VK_LValue,
                                           SS.getWithLocInContext(S.Context),
                                           NameInfo.getLoc());
}

/// \brief Build a MemberExpr AST node.
static MemberExpr *BuildMemberExpr(
    Sema &SemaRef, ASTContext &C, Expr *Base, bool isArrow,
    SourceLocation OpLoc, const CXXScopeSpec &SS, SourceLocation TemplateKWLoc,
    ValueDecl *Member, DeclAccessPair FoundDecl,
    const DeclarationNameInfo &MemberNameInfo, QualType Ty, ExprValueKind VK,
    ExprObjectKind OK, const TemplateArgumentListInfo *TemplateArgs = nullptr) {
  assert((!isArrow || Base->isRValue()) && "-> base must be a pointer rvalue");
  MemberExpr *E = MemberExpr::Create(
      C, Base, isArrow, OpLoc, SS.getWithLocInContext(C), TemplateKWLoc, Member,
      FoundDecl, MemberNameInfo, TemplateArgs, Ty, VK, OK);
  SemaRef.MarkMemberReferenced(E);
  return E;
}

ExprResult
Sema::BuildMemberReferenceExpr(Expr *BaseExpr, QualType BaseExprType,
                               SourceLocation OpLoc, bool IsArrow,
                               const CXXScopeSpec &SS,
                               SourceLocation TemplateKWLoc,
                               NamedDecl *FirstQualifierInScope,
                               LookupResult &R,
                               const TemplateArgumentListInfo *TemplateArgs,
                               bool SuppressQualifierCheck,
                               ActOnMemberAccessExtraArgs *ExtraArgs) {
  QualType BaseType = BaseExprType;
  if (IsArrow) {
    assert(BaseType->isPointerType());
    BaseType = BaseType->castAs<PointerType>()->getPointeeType();
  }
  R.setBaseObjectType(BaseType);
  
  LambdaScopeInfo *const CurLSI = getCurLambda();
  // If this is an implicit member reference and the overloaded
  // name refers to both static and non-static member functions
  // (i.e. BaseExpr is null) and if we are currently processing a lambda, 
  // check if we should/can capture 'this'...
  // Keep this example in mind:
  //  struct X {
  //   void f(int) { }
  //   static void f(double) { }
  // 
  //   int g() {
  //     auto L = [=](auto a) { 
  //       return [](int i) {
  //         return [=](auto b) {
  //           f(b); 
  //           //f(decltype(a){});
  //         };
  //       };
  //     };
  //     auto M = L(0.0); 
  //     auto N = M(3);
  //     N(5.32); // OK, must not error. 
  //     return 0;
  //   }
  //  };
  //
  if (!BaseExpr && CurLSI) {
    SourceLocation Loc = R.getNameLoc();
    if (SS.getRange().isValid())
      Loc = SS.getRange().getBegin();    
    DeclContext *EnclosingFunctionCtx = CurContext->getParent()->getParent();
    // If the enclosing function is not dependent, then this lambda is 
    // capture ready, so if we can capture this, do so.
    if (!EnclosingFunctionCtx->isDependentContext()) {
      // If the current lambda and all enclosing lambdas can capture 'this' -
      // then go ahead and capture 'this' (since our unresolved overload set 
      // contains both static and non-static member functions). 
      if (!CheckCXXThisCapture(Loc, /*Explcit*/false, /*Diagnose*/false))
        CheckCXXThisCapture(Loc);
    } else if (CurContext->isDependentContext()) { 
      // ... since this is an implicit member reference, that might potentially
      // involve a 'this' capture, mark 'this' for potential capture in 
      // enclosing lambdas.
      if (CurLSI->ImpCaptureStyle != CurLSI->ImpCap_None)
        CurLSI->addPotentialThisCapture(Loc);
    }
  }
  const DeclarationNameInfo &MemberNameInfo = R.getLookupNameInfo();
  DeclarationName MemberName = MemberNameInfo.getName();
  SourceLocation MemberLoc = MemberNameInfo.getLoc();

  if (R.isAmbiguous())
    return ExprError();

  if (R.empty()) {
    // Rederive where we looked up.
    DeclContext *DC = (SS.isSet()
                       ? computeDeclContext(SS, false)
                       : BaseType->getAs<RecordType>()->getDecl());

    if (ExtraArgs) {
      ExprResult RetryExpr;
      if (!IsArrow && BaseExpr) {
        SFINAETrap Trap(*this, true);
        ParsedType ObjectType;
        bool MayBePseudoDestructor = false;
        RetryExpr = ActOnStartCXXMemberReference(getCurScope(), BaseExpr,
                                                 OpLoc, tok::arrow, ObjectType,
                                                 MayBePseudoDestructor);
        if (RetryExpr.isUsable() && !Trap.hasErrorOccurred()) {
          CXXScopeSpec TempSS(SS);
          RetryExpr = ActOnMemberAccessExpr(
              ExtraArgs->S, RetryExpr.get(), OpLoc, tok::arrow, TempSS,
              TemplateKWLoc, ExtraArgs->Id, ExtraArgs->ObjCImpDecl);
        }
        if (Trap.hasErrorOccurred())
          RetryExpr = ExprError();
      }
      if (RetryExpr.isUsable()) {
        Diag(OpLoc, diag::err_no_member_overloaded_arrow)
          << MemberName << DC << FixItHint::CreateReplacement(OpLoc, "->");
        return RetryExpr;
      }
    }

    Diag(R.getNameLoc(), diag::err_no_member)
      << MemberName << DC
      << (BaseExpr ? BaseExpr->getSourceRange() : SourceRange());
    return ExprError();
  }

  // Diagnose lookups that find only declarations from a non-base
  // type.  This is possible for either qualified lookups (which may
  // have been qualified with an unrelated type) or implicit member
  // expressions (which were found with unqualified lookup and thus
  // may have come from an enclosing scope).  Note that it's okay for
  // lookup to find declarations from a non-base type as long as those
  // aren't the ones picked by overload resolution.
  if ((SS.isSet() || !BaseExpr ||
       (isa<CXXThisExpr>(BaseExpr) &&
        cast<CXXThisExpr>(BaseExpr)->isImplicit())) &&
      !SuppressQualifierCheck &&
      CheckQualifiedMemberReference(BaseExpr, BaseType, SS, R))
    return ExprError();
  
  // Construct an unresolved result if we in fact got an unresolved
  // result.
  if (R.isOverloadedResult() || R.isUnresolvableResult()) {
    // Suppress any lookup-related diagnostics; we'll do these when we
    // pick a member.
    R.suppressDiagnostics();

    UnresolvedMemberExpr *MemExpr
      = UnresolvedMemberExpr::Create(Context, R.isUnresolvableResult(),
                                     BaseExpr, BaseExprType,
                                     IsArrow, OpLoc,
                                     SS.getWithLocInContext(Context),
                                     TemplateKWLoc, MemberNameInfo,
                                     TemplateArgs, R.begin(), R.end());

    return MemExpr;
  }

  assert(R.isSingleResult());
  DeclAccessPair FoundDecl = R.begin().getPair();
  NamedDecl *MemberDecl = R.getFoundDecl();

  // FIXME: diagnose the presence of template arguments now.

  // If the decl being referenced had an error, return an error for this
  // sub-expr without emitting another error, in order to avoid cascading
  // error cases.
  if (MemberDecl->isInvalidDecl())
    return ExprError();

  // Handle the implicit-member-access case.
  if (!BaseExpr) {
    // If this is not an instance member, convert to a non-member access.
    if (!MemberDecl->isCXXInstanceMember())
      return BuildDeclarationNameExpr(SS, R.getLookupNameInfo(), MemberDecl);

    SourceLocation Loc = R.getNameLoc();
    if (SS.getRange().isValid())
      Loc = SS.getRange().getBegin();
    CheckCXXThisCapture(Loc);

    // HLSL Change Starts - adjust this from T* to T&-like
    if (getLangOpts().HLSL)
      BaseExpr = genereateHLSLThis(Loc, BaseExprType, /*isImplicit=*/true);
    else
      BaseExpr = new (Context) CXXThisExpr(Loc, BaseExprType,/*isImplicit=*/true);
    // HLSL Change Ends
  }

  bool ShouldCheckUse = true;
  if (CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(MemberDecl)) {
    // Don't diagnose the use of a virtual member function unless it's
    // explicitly qualified.
    if (MD->isVirtual() && !SS.isSet())
      ShouldCheckUse = false;
  }

  // Check the use of this member.
  if (ShouldCheckUse && DiagnoseUseOfDecl(MemberDecl, MemberLoc))
    return ExprError();

  if (FieldDecl *FD = dyn_cast<FieldDecl>(MemberDecl))
    return BuildFieldReferenceExpr(*this, BaseExpr, IsArrow, OpLoc, SS, FD,
                                   FoundDecl, MemberNameInfo);

  if (MSPropertyDecl *PD = dyn_cast<MSPropertyDecl>(MemberDecl))
    return BuildMSPropertyRefExpr(*this, BaseExpr, IsArrow, SS, PD,
                                  MemberNameInfo);

  if (IndirectFieldDecl *FD = dyn_cast<IndirectFieldDecl>(MemberDecl))
    // We may have found a field within an anonymous union or struct
    // (C++ [class.union]).
    return BuildAnonymousStructUnionMemberReference(SS, MemberLoc, FD,
                                                    FoundDecl, BaseExpr,
                                                    OpLoc);

  if (VarDecl *Var = dyn_cast<VarDecl>(MemberDecl)) {
    return BuildMemberExpr(*this, Context, BaseExpr, IsArrow, OpLoc, SS,
                           TemplateKWLoc, Var, FoundDecl, MemberNameInfo,
                           Var->getType().getNonReferenceType(), VK_LValue,
                           OK_Ordinary);
  }

  if (CXXMethodDecl *MemberFn = dyn_cast<CXXMethodDecl>(MemberDecl)) {
    ExprValueKind valueKind;
    QualType type;
    if (MemberFn->isInstance()) {
      valueKind = VK_RValue;
      type = Context.BoundMemberTy;
    } else {
      valueKind = VK_LValue;
      type = MemberFn->getType();
    }

    return BuildMemberExpr(*this, Context, BaseExpr, IsArrow, OpLoc, SS,
                           TemplateKWLoc, MemberFn, FoundDecl, MemberNameInfo,
                           type, valueKind, OK_Ordinary);
  }
  assert(!isa<FunctionDecl>(MemberDecl) && "member function not C++ method?");

  if (EnumConstantDecl *Enum = dyn_cast<EnumConstantDecl>(MemberDecl)) {
    return BuildMemberExpr(*this, Context, BaseExpr, IsArrow, OpLoc, SS,
                           TemplateKWLoc, Enum, FoundDecl, MemberNameInfo,
                           Enum->getType(), VK_RValue, OK_Ordinary);
  }

  // We found something that we didn't expect. Complain.
  if (isa<TypeDecl>(MemberDecl))
    Diag(MemberLoc, diag::err_typecheck_member_reference_type)
      << MemberName << BaseType << int(IsArrow);
  else
    Diag(MemberLoc, diag::err_typecheck_member_reference_unknown)
      << MemberName << BaseType << int(IsArrow);

  Diag(MemberDecl->getLocation(), diag::note_member_declared_here)
    << MemberName;
  R.suppressDiagnostics();
  return ExprError();
}

/// Given that normal member access failed on the given expression,
/// and given that the expression's type involves builtin-id or
/// builtin-Class, decide whether substituting in the redefinition
/// types would be profitable.  The redefinition type is whatever
/// this translation unit tried to typedef to id/Class;  we store
/// it to the side and then re-use it in places like this.
static bool ShouldTryAgainWithRedefinitionType(Sema &S, ExprResult &base) {
  const ObjCObjectPointerType *opty
    = base.get()->getType()->getAs<ObjCObjectPointerType>();
  if (!opty) return false;

  const ObjCObjectType *ty = opty->getObjectType();

  QualType redef;
  if (ty->isObjCId()) {
    redef = S.Context.getObjCIdRedefinitionType();
  } else if (ty->isObjCClass()) {
    redef = S.Context.getObjCClassRedefinitionType();
  } else {
    return false;
  }

  // Do the substitution as long as the redefinition type isn't just a
  // possibly-qualified pointer to builtin-id or builtin-Class again.
  opty = redef->getAs<ObjCObjectPointerType>();
  if (opty && !opty->getObjectType()->getInterface())
    return false;

  base = S.ImpCastExprToType(base.get(), redef, CK_BitCast);
  return true;
}

static bool isRecordType(QualType T) {
  return T->isRecordType();
}
static bool isPointerToRecordType(QualType T) {
  if (const PointerType *PT = T->getAs<PointerType>())
    return PT->getPointeeType()->isRecordType();
  return false;
}

/// Perform conversions on the LHS of a member access expression.
ExprResult
Sema::PerformMemberExprBaseConversion(Expr *Base, bool IsArrow) {
  if (IsArrow && !Base->getType()->isFunctionType())
    return DefaultFunctionArrayLvalueConversion(Base);

  return CheckPlaceholderExpr(Base);
}

/// Look up the given member of the given non-type-dependent
/// expression.  This can return in one of two ways:
///  * If it returns a sentinel null-but-valid result, the caller will
///    assume that lookup was performed and the results written into
///    the provided structure.  It will take over from there.
///  * Otherwise, the returned expression will be produced in place of
///    an ordinary member expression.
///
/// The ObjCImpDecl bit is a gross hack that will need to be properly
/// fixed for ObjC++.
static ExprResult LookupMemberExpr(Sema &S, LookupResult &R,
                                   ExprResult &BaseExpr, bool &IsArrow,
                                   SourceLocation OpLoc, CXXScopeSpec &SS,
                                   Decl *ObjCImpDecl, bool HasTemplateArgs) {
  assert(BaseExpr.get() && "no base expression");

  // Perform default conversions.
  BaseExpr = S.PerformMemberExprBaseConversion(BaseExpr.get(), IsArrow);
  if (BaseExpr.isInvalid())
    return ExprError();

  QualType BaseType = BaseExpr.get()->getType();
  assert(!BaseType->isDependentType());

  DeclarationName MemberName = R.getLookupName();
  SourceLocation MemberLoc = R.getNameLoc();

  // For later type-checking purposes, turn arrow accesses into dot
  // accesses.  The only access type we support that doesn't follow
  // the C equivalence "a->b === (*a).b" is ObjC property accesses,
  // and those never use arrows, so this is unaffected.
  if (IsArrow) {
    if (const PointerType *Ptr = BaseType->getAs<PointerType>())
      BaseType = Ptr->getPointeeType();
    else if (const ObjCObjectPointerType *Ptr
               = BaseType->getAs<ObjCObjectPointerType>())
      BaseType = Ptr->getPointeeType();
    else if (BaseType->isRecordType()) {
      // Recover from arrow accesses to records, e.g.:
      //   struct MyRecord foo;
      //   foo->bar
      // This is actually well-formed in C++ if MyRecord has an
      // overloaded operator->, but that should have been dealt with
      // by now--or a diagnostic message already issued if a problem
      // was encountered while looking for the overloaded operator->.
      if (!S.getLangOpts().CPlusPlus) {
        S.Diag(OpLoc, diag::err_typecheck_member_reference_suggestion)
          << BaseType << int(IsArrow) << BaseExpr.get()->getSourceRange()
          << FixItHint::CreateReplacement(OpLoc, ".");
      }
      IsArrow = false;
    } else if (BaseType->isFunctionType()) {
      goto fail;
    } else {
      S.Diag(MemberLoc, diag::err_typecheck_member_reference_arrow)
        << BaseType << BaseExpr.get()->getSourceRange();
      return ExprError();
    }
  }

  // HLSL Change Starts
  // Look up HLSL specialty records: Matrix, Vector, Array
  if (S.getLangOpts().HLSL) {
    ExprResult res;
    if (hlsl::LookupRecordMemberExprForHLSL(&S, *BaseExpr.get(), MemberName, IsArrow, OpLoc, MemberLoc, res))
      return res;
  }
  // HLSL Change Ends

  // Handle field access to simple records.
  if (const RecordType *RTy = BaseType->getAs<RecordType>()) {
    TypoExpr *TE = nullptr;
    if (LookupMemberExprInRecord(S, R, BaseExpr.get(), RTy,
                                 OpLoc, IsArrow, SS, HasTemplateArgs, TE))
      return ExprError();

    // Returning valid-but-null is how we indicate to the caller that
    // the lookup result was filled in. If typo correction was attempted and
    // failed, the lookup result will have been cleared--that combined with the
    // valid-but-null ExprResult will trigger the appropriate diagnostics.
    return ExprResult(TE);
  }

  // Handle ivar access to Objective-C objects.
  if (const ObjCObjectType *OTy = BaseType->getAs<ObjCObjectType>()) {
    if (!SS.isEmpty() && !SS.isInvalid()) {
      S.Diag(SS.getRange().getBegin(), diag::err_qualified_objc_access)
        << 1 << SS.getScopeRep()
        << FixItHint::CreateRemoval(SS.getRange());
      SS.clear();
    }

    IdentifierInfo *Member = MemberName.getAsIdentifierInfo();

    // There are three cases for the base type:
    //   - builtin id (qualified or unqualified)
    //   - builtin Class (qualified or unqualified)
    //   - an interface
    ObjCInterfaceDecl *IDecl = OTy->getInterface();
    if (!IDecl) {
      if (S.getLangOpts().ObjCAutoRefCount &&
          (OTy->isObjCId() || OTy->isObjCClass()))
        goto fail;
      // There's an implicit 'isa' ivar on all objects.
      // But we only actually find it this way on objects of type 'id',
      // apparently.
      if (OTy->isObjCId() && Member->isStr("isa"))
        return new (S.Context) ObjCIsaExpr(BaseExpr.get(), IsArrow, MemberLoc,
                                           OpLoc, S.Context.getObjCClassType());
      if (ShouldTryAgainWithRedefinitionType(S, BaseExpr))
        return LookupMemberExpr(S, R, BaseExpr, IsArrow, OpLoc, SS,
                                ObjCImpDecl, HasTemplateArgs);
      goto fail;
    }

    if (S.RequireCompleteType(OpLoc, BaseType,
                              diag::err_typecheck_incomplete_tag,
                              BaseExpr.get()))
      return ExprError();

    ObjCInterfaceDecl *ClassDeclared = nullptr;
    ObjCIvarDecl *IV = IDecl->lookupInstanceVariable(Member, ClassDeclared);

    if (!IV) {
      // Attempt to correct for typos in ivar names.
      auto Validator = llvm::make_unique<DeclFilterCCC<ObjCIvarDecl>>();
      Validator->IsObjCIvarLookup = IsArrow;
      if (TypoCorrection Corrected = S.CorrectTypo(
              R.getLookupNameInfo(), Sema::LookupMemberName, nullptr, nullptr,
              std::move(Validator), Sema::CTK_ErrorRecovery, IDecl)) {
        IV = Corrected.getCorrectionDeclAs<ObjCIvarDecl>();
        S.diagnoseTypo(
            Corrected,
            S.PDiag(diag::err_typecheck_member_reference_ivar_suggest)
                << IDecl->getDeclName() << MemberName);

        // Figure out the class that declares the ivar.
        assert(!ClassDeclared);
        Decl *D = cast<Decl>(IV->getDeclContext());
        if (ObjCCategoryDecl *CAT = dyn_cast<ObjCCategoryDecl>(D))
          D = CAT->getClassInterface();
        ClassDeclared = cast<ObjCInterfaceDecl>(D);
      } else {
        if (IsArrow && IDecl->FindPropertyDeclaration(Member)) {
          S.Diag(MemberLoc, diag::err_property_found_suggest)
              << Member << BaseExpr.get()->getType()
              << FixItHint::CreateReplacement(OpLoc, ".");
          return ExprError();
        }

        S.Diag(MemberLoc, diag::err_typecheck_member_reference_ivar)
            << IDecl->getDeclName() << MemberName
            << BaseExpr.get()->getSourceRange();
        return ExprError();
      }
    }

    assert(ClassDeclared);

    // If the decl being referenced had an error, return an error for this
    // sub-expr without emitting another error, in order to avoid cascading
    // error cases.
    if (IV->isInvalidDecl())
      return ExprError();

    // Check whether we can reference this field.
    if (S.DiagnoseUseOfDecl(IV, MemberLoc))
      return ExprError();
    if (IV->getAccessControl() != ObjCIvarDecl::Public &&
        IV->getAccessControl() != ObjCIvarDecl::Package) {
      ObjCInterfaceDecl *ClassOfMethodDecl = nullptr;
      if (ObjCMethodDecl *MD = S.getCurMethodDecl())
        ClassOfMethodDecl =  MD->getClassInterface();
      else if (ObjCImpDecl && S.getCurFunctionDecl()) {
        // Case of a c-function declared inside an objc implementation.
        // FIXME: For a c-style function nested inside an objc implementation
        // class, there is no implementation context available, so we pass
        // down the context as argument to this routine. Ideally, this context
        // need be passed down in the AST node and somehow calculated from the
        // AST for a function decl.
        if (ObjCImplementationDecl *IMPD =
              dyn_cast<ObjCImplementationDecl>(ObjCImpDecl))
          ClassOfMethodDecl = IMPD->getClassInterface();
        else if (ObjCCategoryImplDecl* CatImplClass =
                   dyn_cast<ObjCCategoryImplDecl>(ObjCImpDecl))
          ClassOfMethodDecl = CatImplClass->getClassInterface();
      }
      if (!S.getLangOpts().DebuggerSupport) {
        if (IV->getAccessControl() == ObjCIvarDecl::Private) {
          if (!declaresSameEntity(ClassDeclared, IDecl) ||
              !declaresSameEntity(ClassOfMethodDecl, ClassDeclared))
            S.Diag(MemberLoc, diag::error_private_ivar_access)
              << IV->getDeclName();
        } else if (!IDecl->isSuperClassOf(ClassOfMethodDecl))
          // @protected
          S.Diag(MemberLoc, diag::error_protected_ivar_access)
              << IV->getDeclName();
      }
    }
    bool warn = true;
    if (S.getLangOpts().ObjCAutoRefCount) {
      Expr *BaseExp = BaseExpr.get()->IgnoreParenImpCasts();
      if (UnaryOperator *UO = dyn_cast<UnaryOperator>(BaseExp))
        if (UO->getOpcode() == UO_Deref)
          BaseExp = UO->getSubExpr()->IgnoreParenCasts();
      
      if (DeclRefExpr *DE = dyn_cast<DeclRefExpr>(BaseExp))
        if (DE->getType().getObjCLifetime() == Qualifiers::OCL_Weak) {
          S.Diag(DE->getLocation(), diag::error_arc_weak_ivar_access);
          warn = false;
        }
    }
    if (warn) {
      if (ObjCMethodDecl *MD = S.getCurMethodDecl()) {
        ObjCMethodFamily MF = MD->getMethodFamily();
        warn = (MF != OMF_init && MF != OMF_dealloc && 
                MF != OMF_finalize &&
                !S.IvarBacksCurrentMethodAccessor(IDecl, MD, IV));
      }
      if (warn)
        S.Diag(MemberLoc, diag::warn_direct_ivar_access) << IV->getDeclName();
    }

    ObjCIvarRefExpr *Result = new (S.Context) ObjCIvarRefExpr(
        IV, IV->getUsageType(BaseType), MemberLoc, OpLoc, BaseExpr.get(),
        IsArrow);

    if (S.getLangOpts().ObjCAutoRefCount) {
      if (IV->getType().getObjCLifetime() == Qualifiers::OCL_Weak) {
        if (!S.Diags.isIgnored(diag::warn_arc_repeated_use_of_weak, MemberLoc))
          S.recordUseOfEvaluatedWeak(Result);
      }
    }

    return Result;
  }

  // Objective-C property access.
  const ObjCObjectPointerType *OPT;
  if (!IsArrow && (OPT = BaseType->getAs<ObjCObjectPointerType>())) {
    if (!SS.isEmpty() && !SS.isInvalid()) {
      S.Diag(SS.getRange().getBegin(), diag::err_qualified_objc_access)
          << 0 << SS.getScopeRep() << FixItHint::CreateRemoval(SS.getRange());
      SS.clear();
    }

    // This actually uses the base as an r-value.
    BaseExpr = S.DefaultLvalueConversion(BaseExpr.get());
    if (BaseExpr.isInvalid())
      return ExprError();

    assert(S.Context.hasSameUnqualifiedType(BaseType,
                                            BaseExpr.get()->getType()));

    IdentifierInfo *Member = MemberName.getAsIdentifierInfo();

    const ObjCObjectType *OT = OPT->getObjectType();

    // id, with and without qualifiers.
    if (OT->isObjCId()) {
      // Check protocols on qualified interfaces.
      Selector Sel = S.PP.getSelectorTable().getNullarySelector(Member);
      if (Decl *PMDecl =
              FindGetterSetterNameDecl(OPT, Member, Sel, S.Context)) {
        if (ObjCPropertyDecl *PD = dyn_cast<ObjCPropertyDecl>(PMDecl)) {
          // Check the use of this declaration
          if (S.DiagnoseUseOfDecl(PD, MemberLoc))
            return ExprError();

          return new (S.Context)
              ObjCPropertyRefExpr(PD, S.Context.PseudoObjectTy, VK_LValue,
                                  OK_ObjCProperty, MemberLoc, BaseExpr.get());
        }

        if (ObjCMethodDecl *OMD = dyn_cast<ObjCMethodDecl>(PMDecl)) {
          // Check the use of this method.
          if (S.DiagnoseUseOfDecl(OMD, MemberLoc))
            return ExprError();
          Selector SetterSel =
            SelectorTable::constructSetterSelector(S.PP.getIdentifierTable(),
                                                   S.PP.getSelectorTable(),
                                                   Member);
          ObjCMethodDecl *SMD = nullptr;
          if (Decl *SDecl = FindGetterSetterNameDecl(OPT,
                                                     /*Property id*/ nullptr,
                                                     SetterSel, S.Context))
            SMD = dyn_cast<ObjCMethodDecl>(SDecl);

          return new (S.Context)
              ObjCPropertyRefExpr(OMD, SMD, S.Context.PseudoObjectTy, VK_LValue,
                                  OK_ObjCProperty, MemberLoc, BaseExpr.get());
        }
      }
      // Use of id.member can only be for a property reference. Do not
      // use the 'id' redefinition in this case.
      if (IsArrow && ShouldTryAgainWithRedefinitionType(S, BaseExpr))
        return LookupMemberExpr(S, R, BaseExpr, IsArrow, OpLoc, SS,
                                ObjCImpDecl, HasTemplateArgs);

      return ExprError(S.Diag(MemberLoc, diag::err_property_not_found)
                         << MemberName << BaseType);
    }

    // 'Class', unqualified only.
    if (OT->isObjCClass()) {
      // Only works in a method declaration (??!).
      ObjCMethodDecl *MD = S.getCurMethodDecl();
      if (!MD) {
        if (ShouldTryAgainWithRedefinitionType(S, BaseExpr))
          return LookupMemberExpr(S, R, BaseExpr, IsArrow, OpLoc, SS,
                                  ObjCImpDecl, HasTemplateArgs);

        goto fail;
      }

      // Also must look for a getter name which uses property syntax.
      Selector Sel = S.PP.getSelectorTable().getNullarySelector(Member);
      ObjCInterfaceDecl *IFace = MD->getClassInterface();
      ObjCMethodDecl *Getter;
      if ((Getter = IFace->lookupClassMethod(Sel))) {
        // Check the use of this method.
        if (S.DiagnoseUseOfDecl(Getter, MemberLoc))
          return ExprError();
      } else
        Getter = IFace->lookupPrivateMethod(Sel, false);
      // If we found a getter then this may be a valid dot-reference, we
      // will look for the matching setter, in case it is needed.
      Selector SetterSel =
        SelectorTable::constructSetterSelector(S.PP.getIdentifierTable(),
                                               S.PP.getSelectorTable(),
                                               Member);
      ObjCMethodDecl *Setter = IFace->lookupClassMethod(SetterSel);
      if (!Setter) {
        // If this reference is in an @implementation, also check for 'private'
        // methods.
        Setter = IFace->lookupPrivateMethod(SetterSel, false);
      }

      if (Setter && S.DiagnoseUseOfDecl(Setter, MemberLoc))
        return ExprError();

      if (Getter || Setter) {
        return new (S.Context) ObjCPropertyRefExpr(
            Getter, Setter, S.Context.PseudoObjectTy, VK_LValue,
            OK_ObjCProperty, MemberLoc, BaseExpr.get());
      }

      if (ShouldTryAgainWithRedefinitionType(S, BaseExpr))
        return LookupMemberExpr(S, R, BaseExpr, IsArrow, OpLoc, SS,
                                ObjCImpDecl, HasTemplateArgs);

      return ExprError(S.Diag(MemberLoc, diag::err_property_not_found)
                         << MemberName << BaseType);
    }

    // Normal property access.
    return S.HandleExprPropertyRefExpr(OPT, BaseExpr.get(), OpLoc, MemberName,
                                       MemberLoc, SourceLocation(), QualType(),
                                       false);
  }

  // Handle 'field access' to vectors, such as 'V.xx'.
  if (BaseType->isExtVectorType()) {
    // FIXME: this expr should store IsArrow.
    IdentifierInfo *Member = MemberName.getAsIdentifierInfo();
    ExprValueKind VK;
    if (IsArrow)
      VK = VK_LValue;
    else {
      if (PseudoObjectExpr *POE = dyn_cast<PseudoObjectExpr>(BaseExpr.get()))
        VK = POE->getSyntacticForm()->getValueKind();
      else
        VK = BaseExpr.get()->getValueKind();
    }
    QualType ret = CheckExtVectorComponent(S, BaseType, VK, OpLoc,
                                           Member, MemberLoc);
    if (ret.isNull())
      return ExprError();

    return new (S.Context)
        ExtVectorElementExpr(ret, VK, BaseExpr.get(), *Member, MemberLoc);
  }

  // Adjust builtin-sel to the appropriate redefinition type if that's
  // not just a pointer to builtin-sel again.
  if (IsArrow && BaseType->isSpecificBuiltinType(BuiltinType::ObjCSel) &&
      !S.Context.getObjCSelRedefinitionType()->isObjCSelType()) {
    BaseExpr = S.ImpCastExprToType(
        BaseExpr.get(), S.Context.getObjCSelRedefinitionType(), CK_BitCast);
    return LookupMemberExpr(S, R, BaseExpr, IsArrow, OpLoc, SS,
                            ObjCImpDecl, HasTemplateArgs);
  }

  // Failure cases.
 fail:

  // Recover from dot accesses to pointers, e.g.:
  //   type *foo;
  //   foo.bar
  // This is actually well-formed in two cases:
  //   - 'type' is an Objective C type
  //   - 'bar' is a pseudo-destructor name which happens to refer to
  //     the appropriate pointer type
  if (const PointerType *Ptr = BaseType->getAs<PointerType>()) {
    if (!IsArrow && Ptr->getPointeeType()->isRecordType() &&
        MemberName.getNameKind() != DeclarationName::CXXDestructorName) {
      S.Diag(OpLoc, diag::err_typecheck_member_reference_suggestion)
          << BaseType << int(IsArrow) << BaseExpr.get()->getSourceRange()
          << FixItHint::CreateReplacement(OpLoc, "->");

      // Recurse as an -> access.
      IsArrow = true;
      return LookupMemberExpr(S, R, BaseExpr, IsArrow, OpLoc, SS,
                              ObjCImpDecl, HasTemplateArgs);
    }
  }

  // If the user is trying to apply -> or . to a function name, it's probably
  // because they forgot parentheses to call that function.
  if (S.tryToRecoverWithCall(
          BaseExpr, S.PDiag(diag::err_member_reference_needs_call),
          /*complain*/ false,
          IsArrow ? &isPointerToRecordType : &isRecordType)) {
    if (BaseExpr.isInvalid())
      return ExprError();
    BaseExpr = S.DefaultFunctionArrayConversion(BaseExpr.get());
    return LookupMemberExpr(S, R, BaseExpr, IsArrow, OpLoc, SS,
                            ObjCImpDecl, HasTemplateArgs);
  }

  S.Diag(OpLoc, diag::err_typecheck_member_reference_struct_union)
    << BaseType << BaseExpr.get()->getSourceRange() << MemberLoc;

  return ExprError();
}

/// The main callback when the parser finds something like
///   expression . [nested-name-specifier] identifier
///   expression -> [nested-name-specifier] identifier
/// where 'identifier' encompasses a fairly broad spectrum of
/// possibilities, including destructor and operator references.
///
/// \param OpKind either tok::arrow or tok::period
/// \param ObjCImpDecl the current Objective-C \@implementation
///   decl; this is an ugly hack around the fact that Objective-C
///   \@implementations aren't properly put in the context chain
ExprResult Sema::ActOnMemberAccessExpr(Scope *S, Expr *Base,
                                       SourceLocation OpLoc,
                                       tok::TokenKind OpKind,
                                       CXXScopeSpec &SS,
                                       SourceLocation TemplateKWLoc,
                                       UnqualifiedId &Id,
                                       Decl *ObjCImpDecl) {
  if (SS.isSet() && SS.isInvalid())
    return ExprError();

  // Warn about the explicit constructor calls Microsoft extension.
  if (getLangOpts().MicrosoftExt &&
      Id.getKind() == UnqualifiedId::IK_ConstructorName)
    Diag(Id.getSourceRange().getBegin(),
         diag::ext_ms_explicit_constructor_call);

  TemplateArgumentListInfo TemplateArgsBuffer;

  // Decompose the name into its component parts.
  DeclarationNameInfo NameInfo;
  const TemplateArgumentListInfo *TemplateArgs;
  DecomposeUnqualifiedId(Id, TemplateArgsBuffer,
                         NameInfo, TemplateArgs);

  DeclarationName Name = NameInfo.getName();
  bool IsArrow = (OpKind == tok::arrow);

  NamedDecl *FirstQualifierInScope
    = (!SS.isSet() ? nullptr : FindFirstQualifierInScope(S, SS.getScopeRep()));

  // This is a postfix expression, so get rid of ParenListExprs.
  ExprResult Result = MaybeConvertParenListExprToParenExpr(S, Base);
  if (Result.isInvalid()) return ExprError();
  Base = Result.get();

  // HLSL Changes Start
  if (getLangOpts().HLSL) {
    Result = hlsl::MaybeConvertMemberAccess(this, Base);
    if (Result.isInvalid()) return ExprError();
    Base = Result.get();
  }
  // HLSL Changes End

  if (Base->getType()->isDependentType() || Name.isDependentName() ||
      isDependentScopeSpecifier(SS)) {
    return ActOnDependentMemberExpr(Base, Base->getType(), IsArrow, OpLoc, SS,
                                    TemplateKWLoc, FirstQualifierInScope,
                                    NameInfo, TemplateArgs);
  }

  ActOnMemberAccessExtraArgs ExtraArgs = {S, Id, ObjCImpDecl};
  return BuildMemberReferenceExpr(Base, Base->getType(), OpLoc, IsArrow, SS,
                                  TemplateKWLoc, FirstQualifierInScope,
                                  NameInfo, TemplateArgs, &ExtraArgs);
}

static ExprResult
BuildFieldReferenceExpr(Sema &S, Expr *BaseExpr, bool IsArrow,
                        SourceLocation OpLoc, const CXXScopeSpec &SS,
                        FieldDecl *Field, DeclAccessPair FoundDecl,
                        const DeclarationNameInfo &MemberNameInfo) {
  // x.a is an l-value if 'a' has a reference type. Otherwise:
  // x.a is an l-value/x-value/pr-value if the base is (and note
  //   that *x is always an l-value), except that if the base isn't
  //   an ordinary object then we must have an rvalue.
  ExprValueKind VK = VK_LValue;
  ExprObjectKind OK = OK_Ordinary;
  if (!IsArrow) {
    if (BaseExpr->getObjectKind() == OK_Ordinary)
      VK = BaseExpr->getValueKind();
    else
      VK = VK_RValue;
  }
  if (VK != VK_RValue && Field->isBitField())
    OK = OK_BitField;
  
  // Figure out the type of the member; see C99 6.5.2.3p3, C++ [expr.ref]
  QualType MemberType = Field->getType();
  if (const ReferenceType *Ref = MemberType->getAs<ReferenceType>()) {
    MemberType = Ref->getPointeeType();
    VK = VK_LValue;
  } else {
    QualType BaseType = BaseExpr->getType();
    if (IsArrow) BaseType = BaseType->getAs<PointerType>()->getPointeeType();

    Qualifiers BaseQuals = BaseType.getQualifiers();

    // GC attributes are never picked up by members.
    BaseQuals.removeObjCGCAttr();

    // CVR attributes from the base are picked up by members,
    // except that 'mutable' members don't pick up 'const'.
    if (Field->isMutable()) BaseQuals.removeConst();

    Qualifiers MemberQuals
    = S.Context.getCanonicalType(MemberType).getQualifiers();

    assert(!MemberQuals.hasAddressSpace());


    Qualifiers Combined = BaseQuals + MemberQuals;
    if (Combined != MemberQuals)
      MemberType = S.Context.getQualifiedType(MemberType, Combined);
  }

  S.UnusedPrivateFields.remove(Field);

  ExprResult Base =
  S.PerformObjectMemberConversion(BaseExpr, SS.getScopeRep(),
                                  FoundDecl, Field);
  if (Base.isInvalid())
    return ExprError();
  return BuildMemberExpr(S, S.Context, Base.get(), IsArrow, OpLoc, SS,
                         /*TemplateKWLoc=*/SourceLocation(), Field, FoundDecl,
                         MemberNameInfo, MemberType, VK, OK);
}

/// Builds an implicit member access expression.  The current context
/// is known to be an instance method, and the given unqualified lookup
/// set is known to contain only instance members, at least one of which
/// is from an appropriate type.
ExprResult
Sema::BuildImplicitMemberExpr(const CXXScopeSpec &SS,
                              SourceLocation TemplateKWLoc,
                              LookupResult &R,
                              const TemplateArgumentListInfo *TemplateArgs,
                              bool IsKnownInstance) {
  assert(!R.empty() && !R.isAmbiguous());
  
  SourceLocation loc = R.getNameLoc();

  // If this is known to be an instance access, go ahead and build an
  // implicit 'this' expression now.
  // 'this' expression now.
  QualType ThisTy = getCurrentThisType();
  assert(!ThisTy.isNull() && "didn't correctly pre-flight capture of 'this'");

  Expr *baseExpr = nullptr; // null signifies implicit access
  if (IsKnownInstance) {
    SourceLocation Loc = R.getNameLoc();
    if (SS.getRange().isValid())
      Loc = SS.getRange().getBegin();
    CheckCXXThisCapture(Loc);
    if (getLangOpts().HLSL) {
      baseExpr = genereateHLSLThis(Loc, ThisTy, /*isImplicit=*/true);
      ThisTy = ThisTy->getPointeeType();
    } else
      baseExpr = new (Context) CXXThisExpr(loc, ThisTy, /*isImplicit=*/true);
  }

  return BuildMemberReferenceExpr(baseExpr, ThisTy,
                                  /*OpLoc*/ SourceLocation(),
                                  // HLSL Change - this is a reference
                                  /*IsArrow*/ !getLangOpts().HLSL,
                                  SS, TemplateKWLoc,
                                  /*FirstQualifierInScope*/ nullptr,
                                  R, TemplateArgs);
}
