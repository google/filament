//===--- SemaDeclAttr.cpp - Declaration Attribute Handling ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements decl-related attribute processing.
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/SemaInternal.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Mangle.h"
#include "clang/AST/ASTMutationListener.h"
#include "clang/Basic/CharInfo.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/DeclSpec.h"
#include "clang/Sema/DelayedDiagnostic.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Scope.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/MathExtras.h"
#include "clang/Sema/SemaHLSL.h" // HLSL Change
using namespace clang;
using namespace sema;

namespace AttributeLangSupport {
  enum LANG {
    C,
    Cpp,
    ObjC
  };
}

//===----------------------------------------------------------------------===//
//  Helper functions
//===----------------------------------------------------------------------===//

/// isFunctionOrMethod - Return true if the given decl has function
/// type (function or function-typed variable) or an Objective-C
/// method.
static bool isFunctionOrMethod(const Decl *D) {
  return (D->getFunctionType() != nullptr) || isa<ObjCMethodDecl>(D);
}
/// \brief Return true if the given decl has function type (function or
/// function-typed variable) or an Objective-C method or a block.
static bool isFunctionOrMethodOrBlock(const Decl *D) {
  return isFunctionOrMethod(D) || isa<BlockDecl>(D);
}

/// Return true if the given decl has a declarator that should have
/// been processed by Sema::GetTypeForDeclarator.
static bool hasDeclarator(const Decl *D) {
  // In some sense, TypedefDecl really *ought* to be a DeclaratorDecl.
  return isa<DeclaratorDecl>(D) || isa<BlockDecl>(D) || isa<TypedefNameDecl>(D) ||
         isa<ObjCPropertyDecl>(D);
}

/// hasFunctionProto - Return true if the given decl has a argument
/// information. This decl should have already passed
/// isFunctionOrMethod or isFunctionOrMethodOrBlock.
static bool hasFunctionProto(const Decl *D) {
  if (const FunctionType *FnTy = D->getFunctionType())
    return isa<FunctionProtoType>(FnTy);
  return isa<ObjCMethodDecl>(D) || isa<BlockDecl>(D);
}

/// getFunctionOrMethodNumParams - Return number of function or method
/// parameters. It is an error to call this on a K&R function (use
/// hasFunctionProto first).
static unsigned getFunctionOrMethodNumParams(const Decl *D) {
  if (const FunctionType *FnTy = D->getFunctionType())
    return cast<FunctionProtoType>(FnTy)->getNumParams();
  if (const BlockDecl *BD = dyn_cast<BlockDecl>(D))
    return BD->getNumParams();
  return cast<ObjCMethodDecl>(D)->param_size();
}

static QualType getFunctionOrMethodParamType(const Decl *D, unsigned Idx) {
  if (const FunctionType *FnTy = D->getFunctionType())
    return cast<FunctionProtoType>(FnTy)->getParamType(Idx);
  if (const BlockDecl *BD = dyn_cast<BlockDecl>(D))
    return BD->getParamDecl(Idx)->getType();

  return cast<ObjCMethodDecl>(D)->parameters()[Idx]->getType();
}

static SourceRange getFunctionOrMethodParamRange(const Decl *D, unsigned Idx) {
  if (const auto *FD = dyn_cast<FunctionDecl>(D))
    return FD->getParamDecl(Idx)->getSourceRange();
  if (const auto *MD = dyn_cast<ObjCMethodDecl>(D))
    return MD->parameters()[Idx]->getSourceRange();
  if (const auto *BD = dyn_cast<BlockDecl>(D))
    return BD->getParamDecl(Idx)->getSourceRange();
  return SourceRange();
}

static QualType getFunctionOrMethodResultType(const Decl *D) {
  if (const FunctionType *FnTy = D->getFunctionType())
    return cast<FunctionType>(FnTy)->getReturnType();
  return cast<ObjCMethodDecl>(D)->getReturnType();
}

static SourceRange getFunctionOrMethodResultSourceRange(const Decl *D) {
  if (const auto *FD = dyn_cast<FunctionDecl>(D))
    return FD->getReturnTypeSourceRange();
  if (const auto *MD = dyn_cast<ObjCMethodDecl>(D))
    return MD->getReturnTypeSourceRange();
  return SourceRange();
}

static bool isFunctionOrMethodVariadic(const Decl *D) {
  if (const FunctionType *FnTy = D->getFunctionType()) {
    const FunctionProtoType *proto = cast<FunctionProtoType>(FnTy);
    return proto->isVariadic();
  }
  if (const BlockDecl *BD = dyn_cast<BlockDecl>(D))
    return BD->isVariadic();

  return cast<ObjCMethodDecl>(D)->isVariadic();
}

static bool isInstanceMethod(const Decl *D) {
  if (const CXXMethodDecl *MethodDecl = dyn_cast<CXXMethodDecl>(D))
    return MethodDecl->isInstance();
  return false;
}

static inline bool isNSStringType(QualType T, ASTContext &Ctx) {
  const ObjCObjectPointerType *PT = T->getAs<ObjCObjectPointerType>();
  if (!PT)
    return false;

  ObjCInterfaceDecl *Cls = PT->getObjectType()->getInterface();
  if (!Cls)
    return false;

  IdentifierInfo* ClsName = Cls->getIdentifier();

  // FIXME: Should we walk the chain of classes?
  return ClsName == &Ctx.Idents.get("NSString") ||
         ClsName == &Ctx.Idents.get("NSMutableString");
}

static inline bool isCFStringType(QualType T, ASTContext &Ctx) {
  const PointerType *PT = T->getAs<PointerType>();
  if (!PT)
    return false;

  const RecordType *RT = PT->getPointeeType()->getAs<RecordType>();
  if (!RT)
    return false;

  const RecordDecl *RD = RT->getDecl();
  if (RD->getTagKind() != TTK_Struct)
    return false;

  return RD->getIdentifier() == &Ctx.Idents.get("__CFString");
}

static unsigned getNumAttributeArgs(const AttributeList &Attr) {
  // FIXME: Include the type in the argument list.
  return Attr.getNumArgs() + Attr.hasParsedType();
}

template <typename Compare>
static bool checkAttributeNumArgsImpl(Sema &S, const AttributeList &Attr,
                                      unsigned Num, unsigned Diag,
                                      Compare Comp) {
  if (Comp(getNumAttributeArgs(Attr), Num)) {
    S.Diag(Attr.getLoc(), Diag) << Attr.getName() << Num;
    return false;
  }

  return true;
}

/// \brief Check if the attribute has exactly as many args as Num. May
/// output an error.
static bool checkAttributeNumArgs(Sema &S, const AttributeList &Attr,
                                  unsigned Num) {
  return checkAttributeNumArgsImpl(S, Attr, Num,
                                   diag::err_attribute_wrong_number_arguments,
                                   std::not_equal_to<unsigned>());
}

/// \brief Check if the attribute has at least as many args as Num. May
/// output an error.
static bool checkAttributeAtLeastNumArgs(Sema &S, const AttributeList &Attr,
                                         unsigned Num) {
  return checkAttributeNumArgsImpl(S, Attr, Num,
                                   diag::err_attribute_too_few_arguments,
                                   std::less<unsigned>());
}

/// \brief Check if the attribute has at most as many args as Num. May
/// output an error.
static bool checkAttributeAtMostNumArgs(Sema &S, const AttributeList &Attr,
                                         unsigned Num) {
  return checkAttributeNumArgsImpl(S, Attr, Num,
                                   diag::err_attribute_too_many_arguments,
                                   std::greater<unsigned>());
}

/// \brief If Expr is a valid integer constant, get the value of the integer
/// expression and return success or failure. May output an error.
static bool checkUInt32Argument(Sema &S, const AttributeList &Attr,
                                const Expr *Expr, uint32_t &Val,
                                unsigned Idx = UINT_MAX) {
  llvm::APSInt I(32);
  if (Expr->isTypeDependent() || Expr->isValueDependent() ||
      !Expr->isIntegerConstantExpr(I, S.Context)) {
    if (Idx != UINT_MAX)
      S.Diag(Attr.getLoc(), diag::err_attribute_argument_n_type)
        << Attr.getName() << Idx << AANT_ArgumentIntegerConstant
        << Expr->getSourceRange();
    else
      S.Diag(Attr.getLoc(), diag::err_attribute_argument_type)
        << Attr.getName() << AANT_ArgumentIntegerConstant
        << Expr->getSourceRange();
    return false;
  }

  if (!I.isIntN(32)) {
    S.Diag(Expr->getExprLoc(), diag::err_ice_too_large)
        << I.toString(10, false) << 32 << /* Unsigned */ 1;
    return false;
  }

  Val = (uint32_t)I.getZExtValue();
  return true;
}

/// \brief Diagnose mutually exclusive attributes when present on a given
/// declaration. Returns true if diagnosed.
template <typename AttrTy>
static bool checkAttrMutualExclusion(Sema &S, Decl *D,
                                     const AttributeList &Attr) {
  if (AttrTy *A = D->getAttr<AttrTy>()) {
    S.Diag(Attr.getLoc(), diag::err_attributes_are_not_compatible)
      << Attr.getName() << A;
    return true;
  }
  return false;
}

/// \brief Check if IdxExpr is a valid parameter index for a function or
/// instance method D.  May output an error.
///
/// \returns true if IdxExpr is a valid index.
static bool checkFunctionOrMethodParameterIndex(Sema &S, const Decl *D,
                                                const AttributeList &Attr,
                                                unsigned AttrArgNum,
                                                const Expr *IdxExpr,
                                                uint64_t &Idx) {
  assert(isFunctionOrMethodOrBlock(D));

  // In C++ the implicit 'this' function parameter also counts.
  // Parameters are counted from one.
  bool HP = hasFunctionProto(D);
  bool HasImplicitThisParam = isInstanceMethod(D);
  bool IV = HP && isFunctionOrMethodVariadic(D);
  unsigned NumParams =
      (HP ? getFunctionOrMethodNumParams(D) : 0) + HasImplicitThisParam;

  llvm::APSInt IdxInt;
  if (IdxExpr->isTypeDependent() || IdxExpr->isValueDependent() ||
      !IdxExpr->isIntegerConstantExpr(IdxInt, S.Context)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_n_type)
      << Attr.getName() << AttrArgNum << AANT_ArgumentIntegerConstant
      << IdxExpr->getSourceRange();
    return false;
  }

  Idx = IdxInt.getLimitedValue();
  if (Idx < 1 || (!IV && Idx > NumParams)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_out_of_bounds)
      << Attr.getName() << AttrArgNum << IdxExpr->getSourceRange();
    return false;
  }
  Idx--; // Convert to zero-based.
  if (HasImplicitThisParam) {
    if (Idx == 0) {
      S.Diag(Attr.getLoc(),
             diag::err_attribute_invalid_implicit_this_argument)
        << Attr.getName() << IdxExpr->getSourceRange();
      return false;
    }
    --Idx;
  }

  return true;
}

/// \brief Check if the argument \p ArgNum of \p Attr is a ASCII string literal.
/// If not emit an error and return false. If the argument is an identifier it
/// will emit an error with a fixit hint and treat it as if it was a string
/// literal.
bool Sema::checkStringLiteralArgumentAttr(const AttributeList &Attr,
                                          unsigned ArgNum, StringRef &Str,
                                          SourceLocation *ArgLocation) {
  // Look for identifiers. If we have one emit a hint to fix it to a literal.
  if (Attr.isArgIdent(ArgNum)) {
    IdentifierLoc *Loc = Attr.getArgAsIdent(ArgNum);
    Diag(Loc->Loc, diag::err_attribute_argument_type)
        << Attr.getName() << AANT_ArgumentString
        << FixItHint::CreateInsertion(Loc->Loc, "\"")
        << FixItHint::CreateInsertion(PP.getLocForEndOfToken(Loc->Loc), "\"");
    Str = Loc->Ident->getName();
    if (ArgLocation)
      *ArgLocation = Loc->Loc;
    return true;
  }

  // Now check for an actual string literal.
  Expr *ArgExpr = Attr.getArgAsExpr(ArgNum);
  StringLiteral *Literal = dyn_cast<StringLiteral>(ArgExpr->IgnoreParenCasts());
  if (ArgLocation)
    *ArgLocation = ArgExpr->getLocStart();

  if (!Literal || !Literal->isAscii()) {
    Diag(ArgExpr->getLocStart(), diag::err_attribute_argument_type)
        << Attr.getName() << AANT_ArgumentString;
    return false;
  }

  Str = Literal->getString();
  return true;
}

/// \brief Applies the given attribute to the Decl without performing any
/// additional semantic checking.
template <typename AttrType>
static void handleSimpleAttribute(Sema &S, Decl *D,
                                  const AttributeList &Attr) {
  D->addAttr(::new (S.Context) AttrType(Attr.getRange(), S.Context,
                                        Attr.getAttributeSpellingListIndex()));
}

/// \brief Check if the passed-in expression is of type int or bool.
static bool isIntOrBool(Expr *Exp) {
  QualType QT = Exp->getType();
  return QT->isBooleanType() || QT->isIntegerType();
}


// Check to see if the type is a smart pointer of some kind.  We assume
// it's a smart pointer if it defines both operator-> and operator*.
static bool threadSafetyCheckIsSmartPointer(Sema &S, const RecordType* RT) {
  DeclContextLookupResult Res1 = RT->getDecl()->lookup(
      S.Context.DeclarationNames.getCXXOperatorName(OO_Star));
  if (Res1.empty())
    return false;

  DeclContextLookupResult Res2 = RT->getDecl()->lookup(
      S.Context.DeclarationNames.getCXXOperatorName(OO_Arrow));
  if (Res2.empty())
    return false;

  return true;
}

/// \brief Check if passed in Decl is a pointer type.
/// Note that this function may produce an error message.
/// \return true if the Decl is a pointer type; false otherwise
static bool threadSafetyCheckIsPointer(Sema &S, const Decl *D,
                                       const AttributeList &Attr) {
  const ValueDecl *vd = cast<ValueDecl>(D);
  QualType QT = vd->getType();
  if (QT->isAnyPointerType())
    return true;

  if (const RecordType *RT = QT->getAs<RecordType>()) {
    // If it's an incomplete type, it could be a smart pointer; skip it.
    // (We don't want to force template instantiation if we can avoid it,
    // since that would alter the order in which templates are instantiated.)
    if (RT->isIncompleteType())
      return true;

    if (threadSafetyCheckIsSmartPointer(S, RT))
      return true;
  }

  S.Diag(Attr.getLoc(), diag::warn_thread_attribute_decl_not_pointer)
    << Attr.getName() << QT;
  return false;
}

/// \brief Checks that the passed in QualType either is of RecordType or points
/// to RecordType. Returns the relevant RecordType, null if it does not exit.
static const RecordType *getRecordType(QualType QT) {
  if (const RecordType *RT = QT->getAs<RecordType>())
    return RT;

  // Now check if we point to record type.
  if (const PointerType *PT = QT->getAs<PointerType>())
    return PT->getPointeeType()->getAs<RecordType>();

  return nullptr;
}

static bool checkRecordTypeForCapability(Sema &S, QualType Ty) {
  const RecordType *RT = getRecordType(Ty);

  if (!RT)
    return false;

  // Don't check for the capability if the class hasn't been defined yet.
  if (RT->isIncompleteType())
    return true;

  // Allow smart pointers to be used as capability objects.
  // FIXME -- Check the type that the smart pointer points to.
  if (threadSafetyCheckIsSmartPointer(S, RT))
    return true;

  // Check if the record itself has a capability.
  RecordDecl *RD = RT->getDecl();
  if (RD->hasAttr<CapabilityAttr>())
    return true;

  // Else check if any base classes have a capability.
  if (CXXRecordDecl *CRD = dyn_cast<CXXRecordDecl>(RD)) {
    CXXBasePaths BPaths(false, false);
    if (CRD->lookupInBases([](const CXXBaseSpecifier *BS, CXXBasePath &P,
      void *) {
      return BS->getType()->getAs<RecordType>()
        ->getDecl()->hasAttr<CapabilityAttr>();
    }, nullptr, BPaths))
      return true;
  }
  return false;
}

static bool checkTypedefTypeForCapability(QualType Ty) {
  const auto *TD = Ty->getAs<TypedefType>();
  if (!TD)
    return false;

  TypedefNameDecl *TN = TD->getDecl();
  if (!TN)
    return false;

  return TN->hasAttr<CapabilityAttr>();
}

static bool typeHasCapability(Sema &S, QualType Ty) {
  if (checkTypedefTypeForCapability(Ty))
    return true;

  if (checkRecordTypeForCapability(S, Ty))
    return true;

  return false;
}

static bool isCapabilityExpr(Sema &S, const Expr *Ex) {
  // Capability expressions are simple expressions involving the boolean logic
  // operators &&, || or !, a simple DeclRefExpr, CastExpr or a ParenExpr. Once
  // a DeclRefExpr is found, its type should be checked to determine whether it
  // is a capability or not.

  if (const auto *E = dyn_cast<DeclRefExpr>(Ex))
    return typeHasCapability(S, E->getType());
  else if (const auto *E = dyn_cast<CastExpr>(Ex))
    return isCapabilityExpr(S, E->getSubExpr());
  else if (const auto *E = dyn_cast<ParenExpr>(Ex))
    return isCapabilityExpr(S, E->getSubExpr());
  else if (const auto *E = dyn_cast<UnaryOperator>(Ex)) {
    if (E->getOpcode() == UO_LNot)
      return isCapabilityExpr(S, E->getSubExpr());
    return false;
  } else if (const auto *E = dyn_cast<BinaryOperator>(Ex)) {
    if (E->getOpcode() == BO_LAnd || E->getOpcode() == BO_LOr)
      return isCapabilityExpr(S, E->getLHS()) &&
             isCapabilityExpr(S, E->getRHS());
    return false;
  }

  return false;
}

/// \brief Checks that all attribute arguments, starting from Sidx, resolve to
/// a capability object.
/// \param Sidx The attribute argument index to start checking with.
/// \param ParamIdxOk Whether an argument can be indexing into a function
/// parameter list.
static void checkAttrArgsAreCapabilityObjs(Sema &S, Decl *D,
                                           const AttributeList &Attr,
                                           SmallVectorImpl<Expr *> &Args,
                                           int Sidx = 0,
                                           bool ParamIdxOk = false) {
  for (unsigned Idx = Sidx; Idx < Attr.getNumArgs(); ++Idx) {
    Expr *ArgExp = Attr.getArgAsExpr(Idx);

    if (ArgExp->isTypeDependent()) {
      // FIXME -- need to check this again on template instantiation
      Args.push_back(ArgExp);
      continue;
    }

    if (StringLiteral *StrLit = dyn_cast<StringLiteral>(ArgExp)) {
      if (StrLit->getLength() == 0 ||
          (StrLit->isAscii() && StrLit->getString() == StringRef("*"))) {
        // Pass empty strings to the analyzer without warnings.
        // Treat "*" as the universal lock.
        Args.push_back(ArgExp);
        continue;
      }

      // We allow constant strings to be used as a placeholder for expressions
      // that are not valid C++ syntax, but warn that they are ignored.
      S.Diag(Attr.getLoc(), diag::warn_thread_attribute_ignored) <<
        Attr.getName();
      Args.push_back(ArgExp);
      continue;
    }

    QualType ArgTy = ArgExp->getType();

    // A pointer to member expression of the form  &MyClass::mu is treated
    // specially -- we need to look at the type of the member.
    if (UnaryOperator *UOp = dyn_cast<UnaryOperator>(ArgExp))
      if (UOp->getOpcode() == UO_AddrOf)
        if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(UOp->getSubExpr()))
          if (DRE->getDecl()->isCXXInstanceMember())
            ArgTy = DRE->getDecl()->getType();

    // First see if we can just cast to record type, or pointer to record type.
    const RecordType *RT = getRecordType(ArgTy);

    // Now check if we index into a record type function param.
    if(!RT && ParamIdxOk) {
      FunctionDecl *FD = dyn_cast<FunctionDecl>(D);
      IntegerLiteral *IL = dyn_cast<IntegerLiteral>(ArgExp);
      if(FD && IL) {
        unsigned int NumParams = FD->getNumParams();
        llvm::APInt ArgValue = IL->getValue();
        uint64_t ParamIdxFromOne = ArgValue.getZExtValue();
        uint64_t ParamIdxFromZero = ParamIdxFromOne - 1;
        if(!ArgValue.isStrictlyPositive() || ParamIdxFromOne > NumParams) {
          S.Diag(Attr.getLoc(), diag::err_attribute_argument_out_of_range)
            << Attr.getName() << Idx + 1 << NumParams;
          continue;
        }
        ArgTy = FD->getParamDecl(ParamIdxFromZero)->getType();
      }
    }

    // If the type does not have a capability, see if the components of the
    // expression have capabilities. This allows for writing C code where the
    // capability may be on the type, and the expression is a capability
    // boolean logic expression. Eg) requires_capability(A || B && !C)
    if (!typeHasCapability(S, ArgTy) && !isCapabilityExpr(S, ArgExp))
      S.Diag(Attr.getLoc(), diag::warn_thread_attribute_argument_not_lockable)
          << Attr.getName() << ArgTy;

    Args.push_back(ArgExp);
  }
}

//===----------------------------------------------------------------------===//
// Attribute Implementations
//===----------------------------------------------------------------------===//

static void handlePtGuardedVarAttr(Sema &S, Decl *D,
                                   const AttributeList &Attr) {
  if (!threadSafetyCheckIsPointer(S, D, Attr))
    return;

  D->addAttr(::new (S.Context)
             PtGuardedVarAttr(Attr.getRange(), S.Context,
                              Attr.getAttributeSpellingListIndex()));
}

static bool checkGuardedByAttrCommon(Sema &S, Decl *D,
                                     const AttributeList &Attr,
                                     Expr* &Arg) {
  SmallVector<Expr*, 1> Args;
  // check that all arguments are lockable objects
  checkAttrArgsAreCapabilityObjs(S, D, Attr, Args);
  unsigned Size = Args.size();
  if (Size != 1)
    return false;

  Arg = Args[0];

  return true;
}

static void handleGuardedByAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  Expr *Arg = nullptr;
  if (!checkGuardedByAttrCommon(S, D, Attr, Arg))
    return;

  D->addAttr(::new (S.Context) GuardedByAttr(Attr.getRange(), S.Context, Arg,
                                        Attr.getAttributeSpellingListIndex()));
}

static void handlePtGuardedByAttr(Sema &S, Decl *D,
                                  const AttributeList &Attr) {
  Expr *Arg = nullptr;
  if (!checkGuardedByAttrCommon(S, D, Attr, Arg))
    return;

  if (!threadSafetyCheckIsPointer(S, D, Attr))
    return;

  D->addAttr(::new (S.Context) PtGuardedByAttr(Attr.getRange(),
                                               S.Context, Arg,
                                        Attr.getAttributeSpellingListIndex()));
}

static bool checkAcquireOrderAttrCommon(Sema &S, Decl *D,
                                        const AttributeList &Attr,
                                        SmallVectorImpl<Expr *> &Args) {
  if (!checkAttributeAtLeastNumArgs(S, Attr, 1))
    return false;

  // Check that this attribute only applies to lockable types.
  QualType QT = cast<ValueDecl>(D)->getType();
  if (!QT->isDependentType()) {
    const RecordType *RT = getRecordType(QT);
    if (!RT || !RT->getDecl()->hasAttr<CapabilityAttr>()) {
      S.Diag(Attr.getLoc(), diag::warn_thread_attribute_decl_not_lockable)
        << Attr.getName();
      return false;
    }
  }

  // Check that all arguments are lockable objects.
  checkAttrArgsAreCapabilityObjs(S, D, Attr, Args);
  if (Args.empty())
    return false;

  return true;
}

static void handleAcquiredAfterAttr(Sema &S, Decl *D,
                                    const AttributeList &Attr) {
  SmallVector<Expr*, 1> Args;
  if (!checkAcquireOrderAttrCommon(S, D, Attr, Args))
    return;

  Expr **StartArg = &Args[0];
  D->addAttr(::new (S.Context)
             AcquiredAfterAttr(Attr.getRange(), S.Context,
                               StartArg, Args.size(),
                               Attr.getAttributeSpellingListIndex()));
}

static void handleAcquiredBeforeAttr(Sema &S, Decl *D,
                                     const AttributeList &Attr) {
  SmallVector<Expr*, 1> Args;
  if (!checkAcquireOrderAttrCommon(S, D, Attr, Args))
    return;

  Expr **StartArg = &Args[0];
  D->addAttr(::new (S.Context)
             AcquiredBeforeAttr(Attr.getRange(), S.Context,
                                StartArg, Args.size(),
                                Attr.getAttributeSpellingListIndex()));
}

static bool checkLockFunAttrCommon(Sema &S, Decl *D,
                                   const AttributeList &Attr,
                                   SmallVectorImpl<Expr *> &Args) {
  // zero or more arguments ok
  // check that all arguments are lockable objects
  checkAttrArgsAreCapabilityObjs(S, D, Attr, Args, 0, /*ParamIdxOk=*/true);

  return true;
}

static void handleAssertSharedLockAttr(Sema &S, Decl *D,
                                       const AttributeList &Attr) {
  SmallVector<Expr*, 1> Args;
  if (!checkLockFunAttrCommon(S, D, Attr, Args))
    return;

  unsigned Size = Args.size();
  Expr **StartArg = Size == 0 ? nullptr : &Args[0];
  D->addAttr(::new (S.Context)
             AssertSharedLockAttr(Attr.getRange(), S.Context, StartArg, Size,
                                  Attr.getAttributeSpellingListIndex()));
}

static void handleAssertExclusiveLockAttr(Sema &S, Decl *D,
                                          const AttributeList &Attr) {
  SmallVector<Expr*, 1> Args;
  if (!checkLockFunAttrCommon(S, D, Attr, Args))
    return;

  unsigned Size = Args.size();
  Expr **StartArg = Size == 0 ? nullptr : &Args[0];
  D->addAttr(::new (S.Context)
             AssertExclusiveLockAttr(Attr.getRange(), S.Context,
                                     StartArg, Size,
                                     Attr.getAttributeSpellingListIndex()));
}


static bool checkTryLockFunAttrCommon(Sema &S, Decl *D,
                                      const AttributeList &Attr,
                                      SmallVectorImpl<Expr *> &Args) {
  if (!checkAttributeAtLeastNumArgs(S, Attr, 1))
    return false;

  if (!isIntOrBool(Attr.getArgAsExpr(0))) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_n_type)
      << Attr.getName() << 1 << AANT_ArgumentIntOrBool;
    return false;
  }

  // check that all arguments are lockable objects
  checkAttrArgsAreCapabilityObjs(S, D, Attr, Args, 1);

  return true;
}

static void handleSharedTrylockFunctionAttr(Sema &S, Decl *D,
                                            const AttributeList &Attr) {
  SmallVector<Expr*, 2> Args;
  if (!checkTryLockFunAttrCommon(S, D, Attr, Args))
    return;

  D->addAttr(::new (S.Context)
             SharedTrylockFunctionAttr(Attr.getRange(), S.Context,
                                       Attr.getArgAsExpr(0),
                                       Args.data(), Args.size(),
                                       Attr.getAttributeSpellingListIndex()));
}

static void handleExclusiveTrylockFunctionAttr(Sema &S, Decl *D,
                                               const AttributeList &Attr) {
  SmallVector<Expr*, 2> Args;
  if (!checkTryLockFunAttrCommon(S, D, Attr, Args))
    return;

  D->addAttr(::new (S.Context) ExclusiveTrylockFunctionAttr(
      Attr.getRange(), S.Context, Attr.getArgAsExpr(0), Args.data(),
      Args.size(), Attr.getAttributeSpellingListIndex()));
}

static void handleLockReturnedAttr(Sema &S, Decl *D,
                                   const AttributeList &Attr) {
  // check that the argument is lockable object
  SmallVector<Expr*, 1> Args;
  checkAttrArgsAreCapabilityObjs(S, D, Attr, Args);
  unsigned Size = Args.size();
  if (Size == 0)
    return;

  D->addAttr(::new (S.Context)
             LockReturnedAttr(Attr.getRange(), S.Context, Args[0],
                              Attr.getAttributeSpellingListIndex()));
}

static void handleLocksExcludedAttr(Sema &S, Decl *D,
                                    const AttributeList &Attr) {
  if (!checkAttributeAtLeastNumArgs(S, Attr, 1))
    return;

  // check that all arguments are lockable objects
  SmallVector<Expr*, 1> Args;
  checkAttrArgsAreCapabilityObjs(S, D, Attr, Args);
  unsigned Size = Args.size();
  if (Size == 0)
    return;
  Expr **StartArg = &Args[0];

  D->addAttr(::new (S.Context)
             LocksExcludedAttr(Attr.getRange(), S.Context, StartArg, Size,
                               Attr.getAttributeSpellingListIndex()));
}

static void handleEnableIfAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  Expr *Cond = Attr.getArgAsExpr(0);
  if (!Cond->isTypeDependent()) {
    ExprResult Converted = S.PerformContextuallyConvertToBool(Cond);
    if (Converted.isInvalid())
      return;
    Cond = Converted.get();
  }

  StringRef Msg;
  if (!S.checkStringLiteralArgumentAttr(Attr, 1, Msg))
    return;

  SmallVector<PartialDiagnosticAt, 8> Diags;
  if (!Cond->isValueDependent() &&
      !Expr::isPotentialConstantExprUnevaluated(Cond, cast<FunctionDecl>(D),
                                                Diags)) {
    S.Diag(Attr.getLoc(), diag::err_enable_if_never_constant_expr);
    for (int I = 0, N = Diags.size(); I != N; ++I)
      S.Diag(Diags[I].first, Diags[I].second);
    return;
  }

  D->addAttr(::new (S.Context)
             EnableIfAttr(Attr.getRange(), S.Context, Cond, Msg,
                          Attr.getAttributeSpellingListIndex()));
}

static void handleConsumableAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  ConsumableAttr::ConsumedState DefaultState;

  if (Attr.isArgIdent(0)) {
    IdentifierLoc *IL = Attr.getArgAsIdent(0);
    if (!ConsumableAttr::ConvertStrToConsumedState(IL->Ident->getName(),
                                                   DefaultState)) {
      S.Diag(IL->Loc, diag::warn_attribute_type_not_supported)
        << Attr.getName() << IL->Ident;
      return;
    }
  } else {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_type)
        << Attr.getName() << AANT_ArgumentIdentifier;
    return;
  }
  
  D->addAttr(::new (S.Context)
             ConsumableAttr(Attr.getRange(), S.Context, DefaultState,
                            Attr.getAttributeSpellingListIndex()));
}


static bool checkForConsumableClass(Sema &S, const CXXMethodDecl *MD,
                                        const AttributeList &Attr) {
  ASTContext &CurrContext = S.getASTContext();
  QualType ThisType = MD->getThisType(CurrContext)->getPointeeType();
  
  if (const CXXRecordDecl *RD = ThisType->getAsCXXRecordDecl()) {
    if (!RD->hasAttr<ConsumableAttr>()) {
      S.Diag(Attr.getLoc(), diag::warn_attr_on_unconsumable_class) <<
        RD->getNameAsString();
      
      return false;
    }
  }
  
  return true;
}


static void handleCallableWhenAttr(Sema &S, Decl *D,
                                   const AttributeList &Attr) {
  if (!checkAttributeAtLeastNumArgs(S, Attr, 1))
    return;
  
  if (!checkForConsumableClass(S, cast<CXXMethodDecl>(D), Attr))
    return;
  
  SmallVector<CallableWhenAttr::ConsumedState, 3> States;
  for (unsigned ArgIndex = 0; ArgIndex < Attr.getNumArgs(); ++ArgIndex) {
    CallableWhenAttr::ConsumedState CallableState;
    
    StringRef StateString;
    SourceLocation Loc;
    if (Attr.isArgIdent(ArgIndex)) {
      IdentifierLoc *Ident = Attr.getArgAsIdent(ArgIndex);
      StateString = Ident->Ident->getName();
      Loc = Ident->Loc;
    } else {
      if (!S.checkStringLiteralArgumentAttr(Attr, ArgIndex, StateString, &Loc))
        return;
    }

    if (!CallableWhenAttr::ConvertStrToConsumedState(StateString,
                                                     CallableState)) {
      S.Diag(Loc, diag::warn_attribute_type_not_supported)
        << Attr.getName() << StateString;
      return;
    }
      
    States.push_back(CallableState);
  }
  
  D->addAttr(::new (S.Context)
             CallableWhenAttr(Attr.getRange(), S.Context, States.data(),
               States.size(), Attr.getAttributeSpellingListIndex()));
}


static void handleParamTypestateAttr(Sema &S, Decl *D,
                                    const AttributeList &Attr) {
  ParamTypestateAttr::ConsumedState ParamState;
  
  if (Attr.isArgIdent(0)) {
    IdentifierLoc *Ident = Attr.getArgAsIdent(0);
    StringRef StateString = Ident->Ident->getName();

    if (!ParamTypestateAttr::ConvertStrToConsumedState(StateString,
                                                       ParamState)) {
      S.Diag(Ident->Loc, diag::warn_attribute_type_not_supported)
        << Attr.getName() << StateString;
      return;
    }
  } else {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_type) <<
      Attr.getName() << AANT_ArgumentIdentifier;
    return;
  }
  
  // FIXME: This check is currently being done in the analysis.  It can be
  //        enabled here only after the parser propagates attributes at
  //        template specialization definition, not declaration.
  //QualType ReturnType = cast<ParmVarDecl>(D)->getType();
  //const CXXRecordDecl *RD = ReturnType->getAsCXXRecordDecl();
  //
  //if (!RD || !RD->hasAttr<ConsumableAttr>()) {
  //    S.Diag(Attr.getLoc(), diag::warn_return_state_for_unconsumable_type) <<
  //      ReturnType.getAsString();
  //    return;
  //}
  
  D->addAttr(::new (S.Context)
             ParamTypestateAttr(Attr.getRange(), S.Context, ParamState,
                                Attr.getAttributeSpellingListIndex()));
}


static void handleReturnTypestateAttr(Sema &S, Decl *D,
                                      const AttributeList &Attr) {
  ReturnTypestateAttr::ConsumedState ReturnState;
  
  if (Attr.isArgIdent(0)) {
    IdentifierLoc *IL = Attr.getArgAsIdent(0);
    if (!ReturnTypestateAttr::ConvertStrToConsumedState(IL->Ident->getName(),
                                                        ReturnState)) {
      S.Diag(IL->Loc, diag::warn_attribute_type_not_supported)
        << Attr.getName() << IL->Ident;
      return;
    }
  } else {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_type) <<
      Attr.getName() << AANT_ArgumentIdentifier;
    return;
  }
  
  // FIXME: This check is currently being done in the analysis.  It can be
  //        enabled here only after the parser propagates attributes at
  //        template specialization definition, not declaration.
  //QualType ReturnType;
  //
  //if (const ParmVarDecl *Param = dyn_cast<ParmVarDecl>(D)) {
  //  ReturnType = Param->getType();
  //
  //} else if (const CXXConstructorDecl *Constructor =
  //             dyn_cast<CXXConstructorDecl>(D)) {
  //  ReturnType = Constructor->getThisType(S.getASTContext())->getPointeeType();
  //  
  //} else {
  //  
  //  ReturnType = cast<FunctionDecl>(D)->getCallResultType();
  //}
  //
  //const CXXRecordDecl *RD = ReturnType->getAsCXXRecordDecl();
  //
  //if (!RD || !RD->hasAttr<ConsumableAttr>()) {
  //    S.Diag(Attr.getLoc(), diag::warn_return_state_for_unconsumable_type) <<
  //      ReturnType.getAsString();
  //    return;
  //}
  
  D->addAttr(::new (S.Context)
             ReturnTypestateAttr(Attr.getRange(), S.Context, ReturnState,
                                 Attr.getAttributeSpellingListIndex()));
}


static void handleSetTypestateAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  if (!checkForConsumableClass(S, cast<CXXMethodDecl>(D), Attr))
    return;
  
  SetTypestateAttr::ConsumedState NewState;
  if (Attr.isArgIdent(0)) {
    IdentifierLoc *Ident = Attr.getArgAsIdent(0);
    StringRef Param = Ident->Ident->getName();
    if (!SetTypestateAttr::ConvertStrToConsumedState(Param, NewState)) {
      S.Diag(Ident->Loc, diag::warn_attribute_type_not_supported)
        << Attr.getName() << Param;
      return;
    }
  } else {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_type) <<
      Attr.getName() << AANT_ArgumentIdentifier;
    return;
  }
  
  D->addAttr(::new (S.Context)
             SetTypestateAttr(Attr.getRange(), S.Context, NewState,
                              Attr.getAttributeSpellingListIndex()));
}

static void handleTestTypestateAttr(Sema &S, Decl *D,
                                    const AttributeList &Attr) {
  if (!checkForConsumableClass(S, cast<CXXMethodDecl>(D), Attr))
    return;
  
  TestTypestateAttr::ConsumedState TestState;  
  if (Attr.isArgIdent(0)) {
    IdentifierLoc *Ident = Attr.getArgAsIdent(0);
    StringRef Param = Ident->Ident->getName();
    if (!TestTypestateAttr::ConvertStrToConsumedState(Param, TestState)) {
      S.Diag(Ident->Loc, diag::warn_attribute_type_not_supported)
        << Attr.getName() << Param;
      return;
    }
  } else {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_type) <<
      Attr.getName() << AANT_ArgumentIdentifier;
    return;
  }
  
  D->addAttr(::new (S.Context)
             TestTypestateAttr(Attr.getRange(), S.Context, TestState,
                                Attr.getAttributeSpellingListIndex()));
}

static void handleExtVectorTypeAttr(Sema &S, Scope *scope, Decl *D,
                                    const AttributeList &Attr) {
  // Remember this typedef decl, we will need it later for diagnostics.
  S.ExtVectorDecls.push_back(cast<TypedefNameDecl>(D));
}

static void handlePackedAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  if (TagDecl *TD = dyn_cast<TagDecl>(D))
    TD->addAttr(::new (S.Context) PackedAttr(Attr.getRange(), S.Context,
                                        Attr.getAttributeSpellingListIndex()));
  else if (FieldDecl *FD = dyn_cast<FieldDecl>(D)) {
    // If the alignment is less than or equal to 8 bits, the packed attribute
    // has no effect.
    if (!FD->getType()->isDependentType() &&
        !FD->getType()->isIncompleteType() &&
        S.Context.getTypeAlign(FD->getType()) <= 8)
      S.Diag(Attr.getLoc(), diag::warn_attribute_ignored_for_field_of_type)
        << Attr.getName() << FD->getType();
    else
      FD->addAttr(::new (S.Context)
                  PackedAttr(Attr.getRange(), S.Context,
                             Attr.getAttributeSpellingListIndex()));
  } else
    S.Diag(Attr.getLoc(), diag::warn_attribute_ignored) << Attr.getName();
}

static bool checkIBOutletCommon(Sema &S, Decl *D, const AttributeList &Attr) {
  // The IBOutlet/IBOutletCollection attributes only apply to instance
  // variables or properties of Objective-C classes.  The outlet must also
  // have an object reference type.
  if (const ObjCIvarDecl *VD = dyn_cast<ObjCIvarDecl>(D)) {
    if (!VD->getType()->getAs<ObjCObjectPointerType>()) {
      S.Diag(Attr.getLoc(), diag::warn_iboutlet_object_type)
        << Attr.getName() << VD->getType() << 0;
      return false;
    }
  }
  else if (const ObjCPropertyDecl *PD = dyn_cast<ObjCPropertyDecl>(D)) {
    if (!PD->getType()->getAs<ObjCObjectPointerType>()) {
      S.Diag(Attr.getLoc(), diag::warn_iboutlet_object_type)
        << Attr.getName() << PD->getType() << 1;
      return false;
    }
  }
  else {
    S.Diag(Attr.getLoc(), diag::warn_attribute_iboutlet) << Attr.getName();
    return false;
  }

  return true;
}

static void handleIBOutlet(Sema &S, Decl *D, const AttributeList &Attr) {
  if (!checkIBOutletCommon(S, D, Attr))
    return;

  D->addAttr(::new (S.Context)
             IBOutletAttr(Attr.getRange(), S.Context,
                          Attr.getAttributeSpellingListIndex()));
}

static void handleIBOutletCollection(Sema &S, Decl *D,
                                     const AttributeList &Attr) {

  // The iboutletcollection attribute can have zero or one arguments.
  if (Attr.getNumArgs() > 1) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_number_arguments)
      << Attr.getName() << 1;
    return;
  }

  if (!checkIBOutletCommon(S, D, Attr))
    return;

  ParsedType PT;

  if (Attr.hasParsedType())
    PT = Attr.getTypeArg();
  else {
    PT = S.getTypeName(S.Context.Idents.get("NSObject"), Attr.getLoc(),
                       S.getScopeForContext(D->getDeclContext()->getParent()));
    if (!PT) {
      S.Diag(Attr.getLoc(), diag::err_iboutletcollection_type) << "NSObject";
      return;
    }
  }

  TypeSourceInfo *QTLoc = nullptr;
  QualType QT = S.GetTypeFromParser(PT, &QTLoc);
  if (!QTLoc)
    QTLoc = S.Context.getTrivialTypeSourceInfo(QT, Attr.getLoc());

  // Diagnose use of non-object type in iboutletcollection attribute.
  // FIXME. Gnu attribute extension ignores use of builtin types in
  // attributes. So, __attribute__((iboutletcollection(char))) will be
  // treated as __attribute__((iboutletcollection())).
  if (!QT->isObjCIdType() && !QT->isObjCObjectType()) {
    S.Diag(Attr.getLoc(),
           QT->isBuiltinType() ? diag::err_iboutletcollection_builtintype
                               : diag::err_iboutletcollection_type) << QT;
    return;
  }

  D->addAttr(::new (S.Context)
             IBOutletCollectionAttr(Attr.getRange(), S.Context, QTLoc,
                                    Attr.getAttributeSpellingListIndex()));
}

bool Sema::isValidPointerAttrType(QualType T, bool RefOkay) {
  if (RefOkay) {
    if (T->isReferenceType())
      return true;
  } else {
    T = T.getNonReferenceType();
  }

  // The nonnull attribute, and other similar attributes, can be applied to a
  // transparent union that contains a pointer type.
  if (const RecordType *UT = T->getAsUnionType()) {
    if (UT && UT->getDecl()->hasAttr<TransparentUnionAttr>()) {
      RecordDecl *UD = UT->getDecl();
      for (const auto *I : UD->fields()) {
        QualType QT = I->getType();
        if (QT->isAnyPointerType() || QT->isBlockPointerType())
          return true;
      }
    }
  }

  return T->isAnyPointerType() || T->isBlockPointerType();
}

static bool attrNonNullArgCheck(Sema &S, QualType T, const AttributeList &Attr,
                                SourceRange AttrParmRange,
                                SourceRange TypeRange,
                                bool isReturnValue = false) {
  if (!S.isValidPointerAttrType(T)) {
    S.Diag(Attr.getLoc(), isReturnValue
                              ? diag::warn_attribute_return_pointers_only
                              : diag::warn_attribute_pointers_only)
        << Attr.getName() << AttrParmRange << TypeRange;
    return false;
  }
  return true;
}

static void handleNonNullAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  SmallVector<unsigned, 8> NonNullArgs;
  for (unsigned I = 0; I < Attr.getNumArgs(); ++I) {
    Expr *Ex = Attr.getArgAsExpr(I);
    uint64_t Idx;
    if (!checkFunctionOrMethodParameterIndex(S, D, Attr, I + 1, Ex, Idx))
      return;

    // Is the function argument a pointer type?
    if (Idx < getFunctionOrMethodNumParams(D) &&
        !attrNonNullArgCheck(S, getFunctionOrMethodParamType(D, Idx), Attr,
                             Ex->getSourceRange(),
                             getFunctionOrMethodParamRange(D, Idx)))
      continue;

    NonNullArgs.push_back(Idx);
  }

  // If no arguments were specified to __attribute__((nonnull)) then all pointer
  // arguments have a nonnull attribute; warn if there aren't any. Skip this
  // check if the attribute came from a macro expansion or a template
  // instantiation.
  if (NonNullArgs.empty() && Attr.getLoc().isFileID() &&
      S.ActiveTemplateInstantiations.empty()) {
    bool AnyPointers = isFunctionOrMethodVariadic(D);
    for (unsigned I = 0, E = getFunctionOrMethodNumParams(D);
         I != E && !AnyPointers; ++I) {
      QualType T = getFunctionOrMethodParamType(D, I);
      if (T->isDependentType() || S.isValidPointerAttrType(T))
        AnyPointers = true;
    }

    if (!AnyPointers)
      S.Diag(Attr.getLoc(), diag::warn_attribute_nonnull_no_pointers);
  }

  unsigned *Start = NonNullArgs.data();
  unsigned Size = NonNullArgs.size();
  llvm::array_pod_sort(Start, Start + Size);
  D->addAttr(::new (S.Context)
             NonNullAttr(Attr.getRange(), S.Context, Start, Size,
                         Attr.getAttributeSpellingListIndex()));
}

static void handleNonNullAttrParameter(Sema &S, ParmVarDecl *D,
                                       const AttributeList &Attr) {
  if (Attr.getNumArgs() > 0) {
    if (D->getFunctionType()) {
      handleNonNullAttr(S, D, Attr);
    } else {
      S.Diag(Attr.getLoc(), diag::warn_attribute_nonnull_parm_no_args)
        << D->getSourceRange();
    }
    return;
  }

  // Is the argument a pointer type?
  if (!attrNonNullArgCheck(S, D->getType(), Attr, SourceRange(),
                           D->getSourceRange()))
    return;

  D->addAttr(::new (S.Context)
             NonNullAttr(Attr.getRange(), S.Context, nullptr, 0,
                         Attr.getAttributeSpellingListIndex()));
}

static void handleReturnsNonNullAttr(Sema &S, Decl *D,
                                     const AttributeList &Attr) {
  QualType ResultType = getFunctionOrMethodResultType(D);
  SourceRange SR = getFunctionOrMethodResultSourceRange(D);
  if (!attrNonNullArgCheck(S, ResultType, Attr, SourceRange(), SR,
                           /* isReturnValue */ true))
    return;

  D->addAttr(::new (S.Context)
            ReturnsNonNullAttr(Attr.getRange(), S.Context,
                               Attr.getAttributeSpellingListIndex()));
}

static void handleAssumeAlignedAttr(Sema &S, Decl *D,
                                    const AttributeList &Attr) {
  Expr *E = Attr.getArgAsExpr(0),
       *OE = Attr.getNumArgs() > 1 ? Attr.getArgAsExpr(1) : nullptr;
  S.AddAssumeAlignedAttr(Attr.getRange(), D, E, OE,
                         Attr.getAttributeSpellingListIndex());
}

void Sema::AddAssumeAlignedAttr(SourceRange AttrRange, Decl *D, Expr *E,
                                Expr *OE, unsigned SpellingListIndex) {
  QualType ResultType = getFunctionOrMethodResultType(D);
  SourceRange SR = getFunctionOrMethodResultSourceRange(D);

  AssumeAlignedAttr TmpAttr(AttrRange, Context, E, OE, SpellingListIndex);
  SourceLocation AttrLoc = AttrRange.getBegin();

  if (!isValidPointerAttrType(ResultType, /* RefOkay */ true)) {
    Diag(AttrLoc, diag::warn_attribute_return_pointers_refs_only)
      << &TmpAttr << AttrRange << SR;
    return;
  }

  if (!E->isValueDependent()) {
    llvm::APSInt I(64);
    if (!E->isIntegerConstantExpr(I, Context)) {
      if (OE)
        Diag(AttrLoc, diag::err_attribute_argument_n_type)
          << &TmpAttr << 1 << AANT_ArgumentIntegerConstant
          << E->getSourceRange();
      else
        Diag(AttrLoc, diag::err_attribute_argument_type)
          << &TmpAttr << AANT_ArgumentIntegerConstant
          << E->getSourceRange();
      return;
    }

    if (!I.isPowerOf2()) {
      Diag(AttrLoc, diag::err_alignment_not_power_of_two)
        << E->getSourceRange();
      return;
    }
  }

  if (OE) {
    if (!OE->isValueDependent()) {
      llvm::APSInt I(64);
      if (!OE->isIntegerConstantExpr(I, Context)) {
        Diag(AttrLoc, diag::err_attribute_argument_n_type)
          << &TmpAttr << 2 << AANT_ArgumentIntegerConstant
          << OE->getSourceRange();
        return;
      }
    }
  }

  D->addAttr(::new (Context)
            AssumeAlignedAttr(AttrRange, Context, E, OE, SpellingListIndex));
}

static void handleOwnershipAttr(Sema &S, Decl *D, const AttributeList &AL) {
  // This attribute must be applied to a function declaration. The first
  // argument to the attribute must be an identifier, the name of the resource,
  // for example: malloc. The following arguments must be argument indexes, the
  // arguments must be of integer type for Returns, otherwise of pointer type.
  // The difference between Holds and Takes is that a pointer may still be used
  // after being held. free() should be __attribute((ownership_takes)), whereas
  // a list append function may well be __attribute((ownership_holds)).

  if (!AL.isArgIdent(0)) {
    S.Diag(AL.getLoc(), diag::err_attribute_argument_n_type)
      << AL.getName() << 1 << AANT_ArgumentIdentifier;
    return;
  }

  // Figure out our Kind.
  OwnershipAttr::OwnershipKind K =
      OwnershipAttr(AL.getLoc(), S.Context, nullptr, nullptr, 0,
                    AL.getAttributeSpellingListIndex()).getOwnKind();

  // Check arguments.
  switch (K) {
  case OwnershipAttr::Takes:
  case OwnershipAttr::Holds:
    if (AL.getNumArgs() < 2) {
      S.Diag(AL.getLoc(), diag::err_attribute_too_few_arguments)
        << AL.getName() << 2;
      return;
    }
    break;
  case OwnershipAttr::Returns:
    if (AL.getNumArgs() > 2) {
      S.Diag(AL.getLoc(), diag::err_attribute_too_many_arguments)
        << AL.getName() << 1;
      return;
    }
    break;
  }

  IdentifierInfo *Module = AL.getArgAsIdent(0)->Ident;

  // Normalize the argument, __foo__ becomes foo.
  StringRef ModuleName = Module->getName();
  if (ModuleName.startswith("__") && ModuleName.endswith("__") &&
      ModuleName.size() > 4) {
    ModuleName = ModuleName.drop_front(2).drop_back(2);
    Module = &S.PP.getIdentifierTable().get(ModuleName);
  }

  SmallVector<unsigned, 8> OwnershipArgs;
  for (unsigned i = 1; i < AL.getNumArgs(); ++i) {
    Expr *Ex = AL.getArgAsExpr(i);
    uint64_t Idx;
    if (!checkFunctionOrMethodParameterIndex(S, D, AL, i, Ex, Idx))
      return;

    // Is the function argument a pointer type?
    QualType T = getFunctionOrMethodParamType(D, Idx);
    int Err = -1;  // No error
    switch (K) {
      case OwnershipAttr::Takes:
      case OwnershipAttr::Holds:
        if (!T->isAnyPointerType() && !T->isBlockPointerType())
          Err = 0;
        break;
      case OwnershipAttr::Returns:
        if (!T->isIntegerType())
          Err = 1;
        break;
    }
    if (-1 != Err) {
      S.Diag(AL.getLoc(), diag::err_ownership_type) << AL.getName() << Err
        << Ex->getSourceRange();
      return;
    }

    // Check we don't have a conflict with another ownership attribute.
    for (const auto *I : D->specific_attrs<OwnershipAttr>()) {
      // Cannot have two ownership attributes of different kinds for the same
      // index.
      if (I->getOwnKind() != K && I->args_end() !=
          std::find(I->args_begin(), I->args_end(), Idx)) {
        S.Diag(AL.getLoc(), diag::err_attributes_are_not_compatible)
          << AL.getName() << I;
        return;
      } else if (K == OwnershipAttr::Returns &&
                 I->getOwnKind() == OwnershipAttr::Returns) {
        // A returns attribute conflicts with any other returns attribute using
        // a different index. Note, diagnostic reporting is 1-based, but stored
        // argument indexes are 0-based.
        if (std::find(I->args_begin(), I->args_end(), Idx) == I->args_end()) {
          S.Diag(I->getLocation(), diag::err_ownership_returns_index_mismatch)
              << *(I->args_begin()) + 1;
          if (I->args_size())
            S.Diag(AL.getLoc(), diag::note_ownership_returns_index_mismatch)
                << (unsigned)Idx + 1 << Ex->getSourceRange();
          return;
        }
      }
    }
    OwnershipArgs.push_back(Idx);
  }

  unsigned* start = OwnershipArgs.data();
  unsigned size = OwnershipArgs.size();
  llvm::array_pod_sort(start, start + size);

  D->addAttr(::new (S.Context)
             OwnershipAttr(AL.getLoc(), S.Context, Module, start, size,
                           AL.getAttributeSpellingListIndex()));
}

static void handleWeakRefAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  // Check the attribute arguments.
  if (Attr.getNumArgs() > 1) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_number_arguments)
      << Attr.getName() << 1;
    return;
  }

  NamedDecl *nd = cast<NamedDecl>(D);

  // gcc rejects
  // class c {
  //   static int a __attribute__((weakref ("v2")));
  //   static int b() __attribute__((weakref ("f3")));
  // };
  // and ignores the attributes of
  // void f(void) {
  //   static int a __attribute__((weakref ("v2")));
  // }
  // we reject them
  const DeclContext *Ctx = D->getDeclContext()->getRedeclContext();
  if (!Ctx->isFileContext()) {
    S.Diag(Attr.getLoc(), diag::err_attribute_weakref_not_global_context)
      << nd;
    return;
  }

  // The GCC manual says
  //
  // At present, a declaration to which `weakref' is attached can only
  // be `static'.
  //
  // It also says
  //
  // Without a TARGET,
  // given as an argument to `weakref' or to `alias', `weakref' is
  // equivalent to `weak'.
  //
  // gcc 4.4.1 will accept
  // int a7 __attribute__((weakref));
  // as
  // int a7 __attribute__((weak));
  // This looks like a bug in gcc. We reject that for now. We should revisit
  // it if this behaviour is actually used.

  // GCC rejects
  // static ((alias ("y"), weakref)).
  // Should we? How to check that weakref is before or after alias?

  // FIXME: it would be good for us to keep the WeakRefAttr as-written instead
  // of transforming it into an AliasAttr.  The WeakRefAttr never uses the
  // StringRef parameter it was given anyway.
  StringRef Str;
  if (Attr.getNumArgs() && S.checkStringLiteralArgumentAttr(Attr, 0, Str))
    // GCC will accept anything as the argument of weakref. Should we
    // check for an existing decl?
    D->addAttr(::new (S.Context) AliasAttr(Attr.getRange(), S.Context, Str,
                                        Attr.getAttributeSpellingListIndex()));

  D->addAttr(::new (S.Context)
             WeakRefAttr(Attr.getRange(), S.Context,
                         Attr.getAttributeSpellingListIndex()));
}

static void handleAliasAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  StringRef Str;
  if (!S.checkStringLiteralArgumentAttr(Attr, 0, Str))
    return;

  if (S.Context.getTargetInfo().getTriple().isOSDarwin()) {
    S.Diag(Attr.getLoc(), diag::err_alias_not_supported_on_darwin);
    return;
  }

  // Aliases should be on declarations, not definitions.
  if (const auto *FD = dyn_cast<FunctionDecl>(D)) {
    if (FD->isThisDeclarationADefinition()) {
      S.Diag(Attr.getLoc(), diag::err_alias_is_definition) << FD;
      return;
    }
  } else {
    const auto *VD = cast<VarDecl>(D);
    if (VD->isThisDeclarationADefinition() && VD->isExternallyVisible()) {
      S.Diag(Attr.getLoc(), diag::err_alias_is_definition) << VD;
      return;
    }
  }

  // FIXME: check if target symbol exists in current file

  D->addAttr(::new (S.Context) AliasAttr(Attr.getRange(), S.Context, Str,
                                         Attr.getAttributeSpellingListIndex()));
}

static void handleColdAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  if (checkAttrMutualExclusion<HotAttr>(S, D, Attr))
    return;

  D->addAttr(::new (S.Context) ColdAttr(Attr.getRange(), S.Context,
                                        Attr.getAttributeSpellingListIndex()));
}

static void handleHotAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  if (checkAttrMutualExclusion<ColdAttr>(S, D, Attr))
    return;

  D->addAttr(::new (S.Context) HotAttr(Attr.getRange(), S.Context,
                                       Attr.getAttributeSpellingListIndex()));
}

static void handleTLSModelAttr(Sema &S, Decl *D,
                               const AttributeList &Attr) {
  StringRef Model;
  SourceLocation LiteralLoc;
  // Check that it is a string.
  if (!S.checkStringLiteralArgumentAttr(Attr, 0, Model, &LiteralLoc))
    return;

  // Check that the value.
  if (Model != "global-dynamic" && Model != "local-dynamic"
      && Model != "initial-exec" && Model != "local-exec") {
    S.Diag(LiteralLoc, diag::err_attr_tlsmodel_arg);
    return;
  }

  D->addAttr(::new (S.Context)
             TLSModelAttr(Attr.getRange(), S.Context, Model,
                          Attr.getAttributeSpellingListIndex()));
}

static void handleRestrictAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  QualType ResultType = getFunctionOrMethodResultType(D);
  if (ResultType->isAnyPointerType() || ResultType->isBlockPointerType()) {
    D->addAttr(::new (S.Context) RestrictAttr(
        Attr.getRange(), S.Context, Attr.getAttributeSpellingListIndex()));
    return;
  }

  S.Diag(Attr.getLoc(), diag::warn_attribute_return_pointers_only)
      << Attr.getName() << getFunctionOrMethodResultSourceRange(D);
}

static void handleCommonAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  if (S.LangOpts.CPlusPlus) {
    S.Diag(Attr.getLoc(), diag::err_attribute_not_supported_in_lang)
      << Attr.getName() << AttributeLangSupport::Cpp;
    return;
  }

  D->addAttr(::new (S.Context) CommonAttr(Attr.getRange(), S.Context,
                                        Attr.getAttributeSpellingListIndex()));
}

static void handleNoReturnAttr(Sema &S, Decl *D, const AttributeList &attr) {
  if (hasDeclarator(D)) return;

  if (S.CheckNoReturnAttr(attr)) return;

  if (!isa<ObjCMethodDecl>(D)) {
    S.Diag(attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << attr.getName() << ExpectedFunctionOrMethod;
    return;
  }

  D->addAttr(::new (S.Context)
             NoReturnAttr(attr.getRange(), S.Context,
                          attr.getAttributeSpellingListIndex()));
}

bool Sema::CheckNoReturnAttr(const AttributeList &attr) {
  if (!checkAttributeNumArgs(*this, attr, 0)) {
    attr.setInvalid();
    return true;
  }

  return false;
}

static void handleAnalyzerNoReturnAttr(Sema &S, Decl *D,
                                       const AttributeList &Attr) {
  
  // The checking path for 'noreturn' and 'analyzer_noreturn' are different
  // because 'analyzer_noreturn' does not impact the type.
  if (!isFunctionOrMethodOrBlock(D)) {
    ValueDecl *VD = dyn_cast<ValueDecl>(D);
    if (!VD || (!VD->getType()->isBlockPointerType() &&
                !VD->getType()->isFunctionPointerType())) {
      S.Diag(Attr.getLoc(),
             Attr.isCXX11Attribute() ? diag::err_attribute_wrong_decl_type
             : diag::warn_attribute_wrong_decl_type)
        << Attr.getName() << ExpectedFunctionMethodOrBlock;
      return;
    }
  }
  
  D->addAttr(::new (S.Context)
             AnalyzerNoReturnAttr(Attr.getRange(), S.Context,
                                  Attr.getAttributeSpellingListIndex()));
}

// PS3 PPU-specific.
static void handleVecReturnAttr(Sema &S, Decl *D, const AttributeList &Attr) {
/*
  Returning a Vector Class in Registers
  
  According to the PPU ABI specifications, a class with a single member of 
  vector type is returned in memory when used as the return value of a function.
  This results in inefficient code when implementing vector classes. To return
  the value in a single vector register, add the vecreturn attribute to the
  class definition. This attribute is also applicable to struct types.
  
  Example:
  
  struct Vector
  {
    __vector float xyzw;
  } __attribute__((vecreturn));
  
  Vector Add(Vector lhs, Vector rhs)
  {
    Vector result;
    result.xyzw = vec_add(lhs.xyzw, rhs.xyzw);
    return result; // This will be returned in a register
  }
*/
  if (VecReturnAttr *A = D->getAttr<VecReturnAttr>()) {
    S.Diag(Attr.getLoc(), diag::err_repeat_attribute) << A;
    return;
  }

  RecordDecl *record = cast<RecordDecl>(D);
  int count = 0;

  if (!isa<CXXRecordDecl>(record)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_vecreturn_only_vector_member);
    return;
  }

  if (!cast<CXXRecordDecl>(record)->isPOD()) {
    S.Diag(Attr.getLoc(), diag::err_attribute_vecreturn_only_pod_record);
    return;
  }

  for (const auto *I : record->fields()) {
    if ((count == 1) || !I->getType()->isVectorType()) {
      S.Diag(Attr.getLoc(), diag::err_attribute_vecreturn_only_vector_member);
      return;
    }
    count++;
  }

  D->addAttr(::new (S.Context)
             VecReturnAttr(Attr.getRange(), S.Context,
                           Attr.getAttributeSpellingListIndex()));
}

static void handleDependencyAttr(Sema &S, Scope *Scope, Decl *D,
                                 const AttributeList &Attr) {
  if (isa<ParmVarDecl>(D)) {
    // [[carries_dependency]] can only be applied to a parameter if it is a
    // parameter of a function declaration or lambda.
    if (!(Scope->getFlags() & clang::Scope::FunctionDeclarationScope)) {
      S.Diag(Attr.getLoc(),
             diag::err_carries_dependency_param_not_function_decl);
      return;
    }
  }

  D->addAttr(::new (S.Context) CarriesDependencyAttr(
                                   Attr.getRange(), S.Context,
                                   Attr.getAttributeSpellingListIndex()));
}

static void handleUsedAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  if (const VarDecl *VD = dyn_cast<VarDecl>(D)) {
    if (VD->hasLocalStorage()) {
      S.Diag(Attr.getLoc(), diag::warn_attribute_ignored) << Attr.getName();
      return;
    }
  } else if (!isFunctionOrMethod(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedVariableOrFunction;
    return;
  }

  D->addAttr(::new (S.Context)
             UsedAttr(Attr.getRange(), S.Context,
                      Attr.getAttributeSpellingListIndex()));
}

static void handleConstructorAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  uint32_t priority = ConstructorAttr::DefaultPriority;
  if (Attr.getNumArgs() &&
      !checkUInt32Argument(S, Attr, Attr.getArgAsExpr(0), priority))
    return;

  D->addAttr(::new (S.Context)
             ConstructorAttr(Attr.getRange(), S.Context, priority,
                             Attr.getAttributeSpellingListIndex()));
}

static void handleDestructorAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  uint32_t priority = DestructorAttr::DefaultPriority;
  if (Attr.getNumArgs() &&
      !checkUInt32Argument(S, Attr, Attr.getArgAsExpr(0), priority))
    return;

  D->addAttr(::new (S.Context)
             DestructorAttr(Attr.getRange(), S.Context, priority,
                            Attr.getAttributeSpellingListIndex()));
}

template <typename AttrTy>
static void handleAttrWithMessage(Sema &S, Decl *D,
                                  const AttributeList &Attr) {
  // Handle the case where the attribute has a text message.
  StringRef Str;
  if (Attr.getNumArgs() == 1 && !S.checkStringLiteralArgumentAttr(Attr, 0, Str))
    return;

  D->addAttr(::new (S.Context) AttrTy(Attr.getRange(), S.Context, Str,
                                      Attr.getAttributeSpellingListIndex()));
}

static void handleObjCSuppresProtocolAttr(Sema &S, Decl *D,
                                          const AttributeList &Attr) {
  if (!cast<ObjCProtocolDecl>(D)->isThisDeclarationADefinition()) {
    S.Diag(Attr.getLoc(), diag::err_objc_attr_protocol_requires_definition)
      << Attr.getName() << Attr.getRange();
    return;
  }

  D->addAttr(::new (S.Context)
          ObjCExplicitProtocolImplAttr(Attr.getRange(), S.Context,
                                       Attr.getAttributeSpellingListIndex()));
}

static bool checkAvailabilityAttr(Sema &S, SourceRange Range,
                                  IdentifierInfo *Platform,
                                  VersionTuple Introduced,
                                  VersionTuple Deprecated,
                                  VersionTuple Obsoleted) {
  StringRef PlatformName
    = AvailabilityAttr::getPrettyPlatformName(Platform->getName());
  if (PlatformName.empty())
    PlatformName = Platform->getName();

  // Ensure that Introduced <= Deprecated <= Obsoleted (although not all
  // of these steps are needed).
  if (!Introduced.empty() && !Deprecated.empty() &&
      !(Introduced <= Deprecated)) {
    S.Diag(Range.getBegin(), diag::warn_availability_version_ordering)
      << 1 << PlatformName << Deprecated.getAsString()
      << 0 << Introduced.getAsString();
    return true;
  }

  if (!Introduced.empty() && !Obsoleted.empty() &&
      !(Introduced <= Obsoleted)) {
    S.Diag(Range.getBegin(), diag::warn_availability_version_ordering)
      << 2 << PlatformName << Obsoleted.getAsString()
      << 0 << Introduced.getAsString();
    return true;
  }

  if (!Deprecated.empty() && !Obsoleted.empty() &&
      !(Deprecated <= Obsoleted)) {
    S.Diag(Range.getBegin(), diag::warn_availability_version_ordering)
      << 2 << PlatformName << Obsoleted.getAsString()
      << 1 << Deprecated.getAsString();
    return true;
  }

  return false;
}

/// \brief Check whether the two versions match.
///
/// If either version tuple is empty, then they are assumed to match. If
/// \p BeforeIsOkay is true, then \p X can be less than or equal to \p Y.
static bool versionsMatch(const VersionTuple &X, const VersionTuple &Y,
                          bool BeforeIsOkay) {
  if (X.empty() || Y.empty())
    return true;

  if (X == Y)
    return true;

  if (BeforeIsOkay && X < Y)
    return true;

  return false;
}

AvailabilityAttr *Sema::mergeAvailabilityAttr(NamedDecl *D, SourceRange Range,
                                              IdentifierInfo *Platform,
                                              VersionTuple Introduced,
                                              VersionTuple Deprecated,
                                              VersionTuple Obsoleted,
                                              bool IsUnavailable,
                                              StringRef Message,
                                              bool Override,
                                              unsigned AttrSpellingListIndex) {
  VersionTuple MergedIntroduced = Introduced;
  VersionTuple MergedDeprecated = Deprecated;
  VersionTuple MergedObsoleted = Obsoleted;
  bool FoundAny = false;

  if (D->hasAttrs()) {
    AttrVec &Attrs = D->getAttrs();
    for (unsigned i = 0, e = Attrs.size(); i != e;) {
      const AvailabilityAttr *OldAA = dyn_cast<AvailabilityAttr>(Attrs[i]);
      if (!OldAA) {
        ++i;
        continue;
      }

      IdentifierInfo *OldPlatform = OldAA->getPlatform();
      if (OldPlatform != Platform) {
        ++i;
        continue;
      }

      FoundAny = true;
      VersionTuple OldIntroduced = OldAA->getIntroduced();
      VersionTuple OldDeprecated = OldAA->getDeprecated();
      VersionTuple OldObsoleted = OldAA->getObsoleted();
      bool OldIsUnavailable = OldAA->getUnavailable();

      if (!versionsMatch(OldIntroduced, Introduced, Override) ||
          !versionsMatch(Deprecated, OldDeprecated, Override) ||
          !versionsMatch(Obsoleted, OldObsoleted, Override) ||
          !(OldIsUnavailable == IsUnavailable ||
            (Override && !OldIsUnavailable && IsUnavailable))) {
        if (Override) {
          int Which = -1;
          VersionTuple FirstVersion;
          VersionTuple SecondVersion;
          if (!versionsMatch(OldIntroduced, Introduced, Override)) {
            Which = 0;
            FirstVersion = OldIntroduced;
            SecondVersion = Introduced;
          } else if (!versionsMatch(Deprecated, OldDeprecated, Override)) {
            Which = 1;
            FirstVersion = Deprecated;
            SecondVersion = OldDeprecated;
          } else if (!versionsMatch(Obsoleted, OldObsoleted, Override)) {
            Which = 2;
            FirstVersion = Obsoleted;
            SecondVersion = OldObsoleted;
          }

          if (Which == -1) {
            Diag(OldAA->getLocation(),
                 diag::warn_mismatched_availability_override_unavail)
              << AvailabilityAttr::getPrettyPlatformName(Platform->getName());
          } else {
            Diag(OldAA->getLocation(),
                 diag::warn_mismatched_availability_override)
              << Which
              << AvailabilityAttr::getPrettyPlatformName(Platform->getName())
              << FirstVersion.getAsString() << SecondVersion.getAsString();
          }
          Diag(Range.getBegin(), diag::note_overridden_method);
        } else {
          Diag(OldAA->getLocation(), diag::warn_mismatched_availability);
          Diag(Range.getBegin(), diag::note_previous_attribute);
        }

        Attrs.erase(Attrs.begin() + i);
        --e;
        continue;
      }

      VersionTuple MergedIntroduced2 = MergedIntroduced;
      VersionTuple MergedDeprecated2 = MergedDeprecated;
      VersionTuple MergedObsoleted2 = MergedObsoleted;

      if (MergedIntroduced2.empty())
        MergedIntroduced2 = OldIntroduced;
      if (MergedDeprecated2.empty())
        MergedDeprecated2 = OldDeprecated;
      if (MergedObsoleted2.empty())
        MergedObsoleted2 = OldObsoleted;

      if (checkAvailabilityAttr(*this, OldAA->getRange(), Platform,
                                MergedIntroduced2, MergedDeprecated2,
                                MergedObsoleted2)) {
        Attrs.erase(Attrs.begin() + i);
        --e;
        continue;
      }

      MergedIntroduced = MergedIntroduced2;
      MergedDeprecated = MergedDeprecated2;
      MergedObsoleted = MergedObsoleted2;
      ++i;
    }
  }

  if (FoundAny &&
      MergedIntroduced == Introduced &&
      MergedDeprecated == Deprecated &&
      MergedObsoleted == Obsoleted)
    return nullptr;

  // Only create a new attribute if !Override, but we want to do
  // the checking.
  if (!checkAvailabilityAttr(*this, Range, Platform, MergedIntroduced,
                             MergedDeprecated, MergedObsoleted) &&
      !Override) {
    return ::new (Context) AvailabilityAttr(Range, Context, Platform,
                                            Introduced, Deprecated,
                                            Obsoleted, IsUnavailable, Message,
                                            AttrSpellingListIndex);
  }
  return nullptr;
}

static void handleAvailabilityAttr(Sema &S, Decl *D,
                                   const AttributeList &Attr) {
  if (!checkAttributeNumArgs(S, Attr, 1))
    return;
  IdentifierLoc *Platform = Attr.getArgAsIdent(0);
  unsigned Index = Attr.getAttributeSpellingListIndex();
  
  IdentifierInfo *II = Platform->Ident;
  if (AvailabilityAttr::getPrettyPlatformName(II->getName()).empty())
    S.Diag(Platform->Loc, diag::warn_availability_unknown_platform)
      << Platform->Ident;

  NamedDecl *ND = dyn_cast<NamedDecl>(D);
  if (!ND) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_ignored) << Attr.getName();
    return;
  }

  AvailabilityChange Introduced = Attr.getAvailabilityIntroduced();
  AvailabilityChange Deprecated = Attr.getAvailabilityDeprecated();
  AvailabilityChange Obsoleted = Attr.getAvailabilityObsoleted();
  bool IsUnavailable = Attr.getUnavailableLoc().isValid();
  StringRef Str;
  if (const StringLiteral *SE =
          dyn_cast_or_null<StringLiteral>(Attr.getMessageExpr()))
    Str = SE->getString();

  AvailabilityAttr *NewAttr = S.mergeAvailabilityAttr(ND, Attr.getRange(), II,
                                                      Introduced.Version,
                                                      Deprecated.Version,
                                                      Obsoleted.Version,
                                                      IsUnavailable, Str,
                                                      /*Override=*/false,
                                                      Index);
  if (NewAttr)
    D->addAttr(NewAttr);
}

template <class T>
static T *mergeVisibilityAttr(Sema &S, Decl *D, SourceRange range,
                              typename T::VisibilityType value,
                              unsigned attrSpellingListIndex) {
  T *existingAttr = D->getAttr<T>();
  if (existingAttr) {
    typename T::VisibilityType existingValue = existingAttr->getVisibility();
    if (existingValue == value)
      return nullptr;
    S.Diag(existingAttr->getLocation(), diag::err_mismatched_visibility);
    S.Diag(range.getBegin(), diag::note_previous_attribute);
    D->dropAttr<T>();
  }
  return ::new (S.Context) T(range, S.Context, value, attrSpellingListIndex);
}

VisibilityAttr *Sema::mergeVisibilityAttr(Decl *D, SourceRange Range,
                                          VisibilityAttr::VisibilityType Vis,
                                          unsigned AttrSpellingListIndex) {
  return ::mergeVisibilityAttr<VisibilityAttr>(*this, D, Range, Vis,
                                               AttrSpellingListIndex);
}

TypeVisibilityAttr *Sema::mergeTypeVisibilityAttr(Decl *D, SourceRange Range,
                                      TypeVisibilityAttr::VisibilityType Vis,
                                      unsigned AttrSpellingListIndex) {
  return ::mergeVisibilityAttr<TypeVisibilityAttr>(*this, D, Range, Vis,
                                                   AttrSpellingListIndex);
}

static void handleVisibilityAttr(Sema &S, Decl *D, const AttributeList &Attr,
                                 bool isTypeVisibility) {
  // Visibility attributes don't mean anything on a typedef.
  if (isa<TypedefNameDecl>(D)) {
    S.Diag(Attr.getRange().getBegin(), diag::warn_attribute_ignored)
      << Attr.getName();
    return;
  }

  // 'type_visibility' can only go on a type or namespace.
  if (isTypeVisibility &&
      !(isa<TagDecl>(D) ||
        isa<ObjCInterfaceDecl>(D) ||
        isa<NamespaceDecl>(D))) {
    S.Diag(Attr.getRange().getBegin(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedTypeOrNamespace;
    return;
  }

  // Check that the argument is a string literal.
  StringRef TypeStr;
  SourceLocation LiteralLoc;
  if (!S.checkStringLiteralArgumentAttr(Attr, 0, TypeStr, &LiteralLoc))
    return;

  VisibilityAttr::VisibilityType type;
  if (!VisibilityAttr::ConvertStrToVisibilityType(TypeStr, type)) {
    S.Diag(LiteralLoc, diag::warn_attribute_type_not_supported)
      << Attr.getName() << TypeStr;
    return;
  }
  
  // Complain about attempts to use protected visibility on targets
  // (like Darwin) that don't support it.
  if (type == VisibilityAttr::Protected &&
      !S.Context.getTargetInfo().hasProtectedVisibility()) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_protected_visibility);
    type = VisibilityAttr::Default;
  }

  unsigned Index = Attr.getAttributeSpellingListIndex();
  clang::Attr *newAttr;
  if (isTypeVisibility) {
    newAttr = S.mergeTypeVisibilityAttr(D, Attr.getRange(),
                                    (TypeVisibilityAttr::VisibilityType) type,
                                        Index);
  } else {
    newAttr = S.mergeVisibilityAttr(D, Attr.getRange(), type, Index);
  }
  if (newAttr)
    D->addAttr(newAttr);
}

static void handleObjCMethodFamilyAttr(Sema &S, Decl *decl,
                                       const AttributeList &Attr) {
  ObjCMethodDecl *method = cast<ObjCMethodDecl>(decl);
  if (!Attr.isArgIdent(0)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_n_type)
      << Attr.getName() << 1 << AANT_ArgumentIdentifier;
    return;
  }

  IdentifierLoc *IL = Attr.getArgAsIdent(0);
  ObjCMethodFamilyAttr::FamilyKind F;
  if (!ObjCMethodFamilyAttr::ConvertStrToFamilyKind(IL->Ident->getName(), F)) {
    S.Diag(IL->Loc, diag::warn_attribute_type_not_supported) << Attr.getName()
      << IL->Ident;
    return;
  }

  if (F == ObjCMethodFamilyAttr::OMF_init &&
      !method->getReturnType()->isObjCObjectPointerType()) {
    S.Diag(method->getLocation(), diag::err_init_method_bad_return_type)
        << method->getReturnType();
    // Ignore the attribute.
    return;
  }

  method->addAttr(new (S.Context) ObjCMethodFamilyAttr(Attr.getRange(),
                                                       S.Context, F,
                                        Attr.getAttributeSpellingListIndex()));
}

static void handleObjCNSObject(Sema &S, Decl *D, const AttributeList &Attr) {
  if (TypedefNameDecl *TD = dyn_cast<TypedefNameDecl>(D)) {
    QualType T = TD->getUnderlyingType();
    if (!T->isCARCBridgableType()) {
      S.Diag(TD->getLocation(), diag::err_nsobject_attribute);
      return;
    }
  }
  else if (ObjCPropertyDecl *PD = dyn_cast<ObjCPropertyDecl>(D)) {
    QualType T = PD->getType();
    if (!T->isCARCBridgableType()) {
      S.Diag(PD->getLocation(), diag::err_nsobject_attribute);
      return;
    }
  }
  else {
    // It is okay to include this attribute on properties, e.g.:
    //
    //  @property (retain, nonatomic) struct Bork *Q __attribute__((NSObject));
    //
    // In this case it follows tradition and suppresses an error in the above
    // case.    
    S.Diag(D->getLocation(), diag::warn_nsobject_attribute);
  }
  D->addAttr(::new (S.Context)
             ObjCNSObjectAttr(Attr.getRange(), S.Context,
                              Attr.getAttributeSpellingListIndex()));
}

static void handleObjCIndependentClass(Sema &S, Decl *D, const AttributeList &Attr) {
  if (TypedefNameDecl *TD = dyn_cast<TypedefNameDecl>(D)) {
    QualType T = TD->getUnderlyingType();
    if (!T->isObjCObjectPointerType()) {
      S.Diag(TD->getLocation(), diag::warn_ptr_independentclass_attribute);
      return;
    }
  } else {
    S.Diag(D->getLocation(), diag::warn_independentclass_attribute);
    return;
  }
  D->addAttr(::new (S.Context)
             ObjCIndependentClassAttr(Attr.getRange(), S.Context,
                              Attr.getAttributeSpellingListIndex()));
}

static void handleBlocksAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  if (!Attr.isArgIdent(0)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_n_type)
      << Attr.getName() << 1 << AANT_ArgumentIdentifier;
    return;
  }

  IdentifierInfo *II = Attr.getArgAsIdent(0)->Ident;
  BlocksAttr::BlockType type;
  if (!BlocksAttr::ConvertStrToBlockType(II->getName(), type)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_type_not_supported)
      << Attr.getName() << II;
    return;
  }

  D->addAttr(::new (S.Context)
             BlocksAttr(Attr.getRange(), S.Context, type,
                        Attr.getAttributeSpellingListIndex()));
}

static void handleSentinelAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  unsigned sentinel = (unsigned)SentinelAttr::DefaultSentinel;
  if (Attr.getNumArgs() > 0) {
    Expr *E = Attr.getArgAsExpr(0);
    llvm::APSInt Idx(32);
    if (E->isTypeDependent() || E->isValueDependent() ||
        !E->isIntegerConstantExpr(Idx, S.Context)) {
      S.Diag(Attr.getLoc(), diag::err_attribute_argument_n_type)
        << Attr.getName() << 1 << AANT_ArgumentIntegerConstant
        << E->getSourceRange();
      return;
    }

    if (Idx.isSigned() && Idx.isNegative()) {
      S.Diag(Attr.getLoc(), diag::err_attribute_sentinel_less_than_zero)
        << E->getSourceRange();
      return;
    }

    sentinel = Idx.getZExtValue();
  }

  unsigned nullPos = (unsigned)SentinelAttr::DefaultNullPos;
  if (Attr.getNumArgs() > 1) {
    Expr *E = Attr.getArgAsExpr(1);
    llvm::APSInt Idx(32);
    if (E->isTypeDependent() || E->isValueDependent() ||
        !E->isIntegerConstantExpr(Idx, S.Context)) {
      S.Diag(Attr.getLoc(), diag::err_attribute_argument_n_type)
        << Attr.getName() << 2 << AANT_ArgumentIntegerConstant
        << E->getSourceRange();
      return;
    }
    nullPos = Idx.getZExtValue();

    if ((Idx.isSigned() && Idx.isNegative()) || nullPos > 1) {
      // FIXME: This error message could be improved, it would be nice
      // to say what the bounds actually are.
      S.Diag(Attr.getLoc(), diag::err_attribute_sentinel_not_zero_or_one)
        << E->getSourceRange();
      return;
    }
  }

  if (FunctionDecl *FD = dyn_cast<FunctionDecl>(D)) {
    const FunctionType *FT = FD->getType()->castAs<FunctionType>();
    if (isa<FunctionNoProtoType>(FT)) {
      S.Diag(Attr.getLoc(), diag::warn_attribute_sentinel_named_arguments);
      return;
    }

    if (!cast<FunctionProtoType>(FT)->isVariadic()) {
      S.Diag(Attr.getLoc(), diag::warn_attribute_sentinel_not_variadic) << 0;
      return;
    }
  } else if (ObjCMethodDecl *MD = dyn_cast<ObjCMethodDecl>(D)) {
    if (!MD->isVariadic()) {
      S.Diag(Attr.getLoc(), diag::warn_attribute_sentinel_not_variadic) << 0;
      return;
    }
  } else if (BlockDecl *BD = dyn_cast<BlockDecl>(D)) {
    if (!BD->isVariadic()) {
      S.Diag(Attr.getLoc(), diag::warn_attribute_sentinel_not_variadic) << 1;
      return;
    }
  } else if (const VarDecl *V = dyn_cast<VarDecl>(D)) {
    QualType Ty = V->getType();
    if (Ty->isBlockPointerType() || Ty->isFunctionPointerType()) {
      const FunctionType *FT = Ty->isFunctionPointerType()
       ? D->getFunctionType()
       : Ty->getAs<BlockPointerType>()->getPointeeType()->getAs<FunctionType>();
      if (!cast<FunctionProtoType>(FT)->isVariadic()) {
        int m = Ty->isFunctionPointerType() ? 0 : 1;
        S.Diag(Attr.getLoc(), diag::warn_attribute_sentinel_not_variadic) << m;
        return;
      }
    } else {
      S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
        << Attr.getName() << ExpectedFunctionMethodOrBlock;
      return;
    }
  } else {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunctionMethodOrBlock;
    return;
  }
  D->addAttr(::new (S.Context)
             SentinelAttr(Attr.getRange(), S.Context, sentinel, nullPos,
                          Attr.getAttributeSpellingListIndex()));
}

static void handleWarnUnusedResult(Sema &S, Decl *D, const AttributeList &Attr) {
  if (D->getFunctionType() &&
      D->getFunctionType()->getReturnType()->isVoidType()) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_void_function_method)
      << Attr.getName() << 0;
    return;
  }
  if (const ObjCMethodDecl *MD = dyn_cast<ObjCMethodDecl>(D))
    if (MD->getReturnType()->isVoidType()) {
      S.Diag(Attr.getLoc(), diag::warn_attribute_void_function_method)
      << Attr.getName() << 1;
      return;
    }
  
  D->addAttr(::new (S.Context) 
             WarnUnusedResultAttr(Attr.getRange(), S.Context,
                                  Attr.getAttributeSpellingListIndex()));
}

static void handleWeakImportAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  // weak_import only applies to variable & function declarations.
  bool isDef = false;
  if (!D->canBeWeakImported(isDef)) {
    if (isDef)
      S.Diag(Attr.getLoc(), diag::warn_attribute_invalid_on_definition)
        << "weak_import";
    else if (isa<ObjCPropertyDecl>(D) || isa<ObjCMethodDecl>(D) ||
             (S.Context.getTargetInfo().getTriple().isOSDarwin() &&
              (isa<ObjCInterfaceDecl>(D) || isa<EnumDecl>(D)))) {
      // Nothing to warn about here.
    } else
      S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
        << Attr.getName() << ExpectedVariableOrFunction;

    return;
  }

  D->addAttr(::new (S.Context)
             WeakImportAttr(Attr.getRange(), S.Context,
                            Attr.getAttributeSpellingListIndex()));
}

// Handles reqd_work_group_size and work_group_size_hint.
template <typename WorkGroupAttr>
static void handleWorkGroupSize(Sema &S, Decl *D,
                                const AttributeList &Attr) {
  uint32_t WGSize[3];
  for (unsigned i = 0; i < 3; ++i) {
    const Expr *E = Attr.getArgAsExpr(i);
    if (!checkUInt32Argument(S, Attr, E, WGSize[i], i))
      return;
    if (WGSize[i] == 0) {
      S.Diag(Attr.getLoc(), diag::err_attribute_argument_is_zero)
        << Attr.getName() << E->getSourceRange();
      return;
    }
  }

  WorkGroupAttr *Existing = D->getAttr<WorkGroupAttr>();
  if (Existing && !(Existing->getXDim() == WGSize[0] &&
                    Existing->getYDim() == WGSize[1] &&
                    Existing->getZDim() == WGSize[2]))
    S.Diag(Attr.getLoc(), diag::warn_duplicate_attribute) << Attr.getName();

  D->addAttr(::new (S.Context) WorkGroupAttr(Attr.getRange(), S.Context,
                                             WGSize[0], WGSize[1], WGSize[2],
                                       Attr.getAttributeSpellingListIndex()));
}

static void handleVecTypeHint(Sema &S, Decl *D, const AttributeList &Attr) {
  if (!Attr.hasParsedType()) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_number_arguments)
      << Attr.getName() << 1;
    return;
  }

  TypeSourceInfo *ParmTSI = nullptr;
  QualType ParmType = S.GetTypeFromParser(Attr.getTypeArg(), &ParmTSI);
  assert(ParmTSI && "no type source info for attribute argument");

  if (!ParmType->isExtVectorType() && !ParmType->isFloatingType() &&
      (ParmType->isBooleanType() ||
       !ParmType->isIntegralType(S.getASTContext()))) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_vec_type_hint)
        << ParmType;
    return;
  }

  if (VecTypeHintAttr *A = D->getAttr<VecTypeHintAttr>()) {
    if (!S.Context.hasSameType(A->getTypeHint(), ParmType)) {
      S.Diag(Attr.getLoc(), diag::warn_duplicate_attribute) << Attr.getName();
      return;
    }
  }

  D->addAttr(::new (S.Context) VecTypeHintAttr(Attr.getLoc(), S.Context,
                                               ParmTSI,
                                        Attr.getAttributeSpellingListIndex()));
}

SectionAttr *Sema::mergeSectionAttr(Decl *D, SourceRange Range,
                                    StringRef Name,
                                    unsigned AttrSpellingListIndex) {
  if (SectionAttr *ExistingAttr = D->getAttr<SectionAttr>()) {
    if (ExistingAttr->getName() == Name)
      return nullptr;
    Diag(ExistingAttr->getLocation(), diag::warn_mismatched_section);
    Diag(Range.getBegin(), diag::note_previous_attribute);
    return nullptr;
  }
  return ::new (Context) SectionAttr(Range, Context, Name,
                                     AttrSpellingListIndex);
}

bool Sema::checkSectionName(SourceLocation LiteralLoc, StringRef SecName) {
  std::string Error = Context.getTargetInfo().isValidSectionSpecifier(SecName);
  if (!Error.empty()) {
    Diag(LiteralLoc, diag::err_attribute_section_invalid_for_target) << Error;
    return false;
  }
  return true;
}

static void handleSectionAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  // Make sure that there is a string literal as the sections's single
  // argument.
  StringRef Str;
  SourceLocation LiteralLoc;
  if (!S.checkStringLiteralArgumentAttr(Attr, 0, Str, &LiteralLoc))
    return;

  if (!S.checkSectionName(LiteralLoc, Str))
    return;

  // If the target wants to validate the section specifier, make it happen.
  std::string Error = S.Context.getTargetInfo().isValidSectionSpecifier(Str);
  if (!Error.empty()) {
    S.Diag(LiteralLoc, diag::err_attribute_section_invalid_for_target)
    << Error;
    return;
  }

  unsigned Index = Attr.getAttributeSpellingListIndex();
  SectionAttr *NewAttr = S.mergeSectionAttr(D, Attr.getRange(), Str, Index);
  if (NewAttr)
    D->addAttr(NewAttr);
}

// Check for things we'd like to warn about, no errors or validation for now.
// TODO: Validation should use a backend target library that specifies
// the allowable subtarget features and cpus. We could use something like a
// TargetCodeGenInfo hook here to do validation.
void Sema::checkTargetAttr(SourceLocation LiteralLoc, StringRef AttrStr) {
  for (auto Str : {"tune=", "fpmath="})
    if (AttrStr.find(Str) != StringRef::npos)
      Diag(LiteralLoc, diag::warn_unsupported_target_attribute) << Str;
}

static void handleTargetAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  StringRef Str;
  SourceLocation LiteralLoc;
  if (!S.checkStringLiteralArgumentAttr(Attr, 0, Str, &LiteralLoc))
    return;
  S.checkTargetAttr(LiteralLoc, Str);
  unsigned Index = Attr.getAttributeSpellingListIndex();
  TargetAttr *NewAttr =
      ::new (S.Context) TargetAttr(Attr.getRange(), S.Context, Str, Index);
  D->addAttr(NewAttr);
}


static void handleCleanupAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  VarDecl *VD = cast<VarDecl>(D);
  if (!VD->hasLocalStorage()) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_ignored) << Attr.getName();
    return;
  }

  Expr *E = Attr.getArgAsExpr(0);
  SourceLocation Loc = E->getExprLoc();
  FunctionDecl *FD = nullptr;
  DeclarationNameInfo NI;

  // gcc only allows for simple identifiers. Since we support more than gcc, we
  // will warn the user.
  if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(E)) {
    if (DRE->hasQualifier())
      S.Diag(Loc, diag::warn_cleanup_ext);
    FD = dyn_cast<FunctionDecl>(DRE->getDecl());
    NI = DRE->getNameInfo();
    if (!FD) {
      S.Diag(Loc, diag::err_attribute_cleanup_arg_not_function) << 1
        << NI.getName();
      return;
    }
  } else if (UnresolvedLookupExpr *ULE = dyn_cast<UnresolvedLookupExpr>(E)) {
    if (ULE->hasExplicitTemplateArgs())
      S.Diag(Loc, diag::warn_cleanup_ext);
    FD = S.ResolveSingleFunctionTemplateSpecialization(ULE, true);
    NI = ULE->getNameInfo();
    if (!FD) {
      S.Diag(Loc, diag::err_attribute_cleanup_arg_not_function) << 2
        << NI.getName();
      if (ULE->getType() == S.Context.OverloadTy)
        S.NoteAllOverloadCandidates(ULE);
      return;
    }
  } else {
    S.Diag(Loc, diag::err_attribute_cleanup_arg_not_function) << 0;
    return;
  }

  if (FD->getNumParams() != 1) {
    S.Diag(Loc, diag::err_attribute_cleanup_func_must_take_one_arg)
      << NI.getName();
    return;
  }

  // We're currently more strict than GCC about what function types we accept.
  // If this ever proves to be a problem it should be easy to fix.
  QualType Ty = S.Context.getPointerType(VD->getType());
  QualType ParamTy = FD->getParamDecl(0)->getType();
  if (S.CheckAssignmentConstraints(FD->getParamDecl(0)->getLocation(),
                                   ParamTy, Ty) != Sema::Compatible) {
    S.Diag(Loc, diag::err_attribute_cleanup_func_arg_incompatible_type)
      << NI.getName() << ParamTy << Ty;
    return;
  }

  D->addAttr(::new (S.Context)
             CleanupAttr(Attr.getRange(), S.Context, FD,
                         Attr.getAttributeSpellingListIndex()));
}

/// Handle __attribute__((format_arg((idx)))) attribute based on
/// http://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html
static void handleFormatArgAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  Expr *IdxExpr = Attr.getArgAsExpr(0);
  uint64_t Idx;
  if (!checkFunctionOrMethodParameterIndex(S, D, Attr, 1, IdxExpr, Idx))
    return;

  // make sure the format string is really a string
  QualType Ty = getFunctionOrMethodParamType(D, Idx);

  bool not_nsstring_type = !isNSStringType(Ty, S.Context);
  if (not_nsstring_type &&
      !isCFStringType(Ty, S.Context) &&
      (!Ty->isPointerType() ||
       !Ty->getAs<PointerType>()->getPointeeType()->isCharType())) {
    S.Diag(Attr.getLoc(), diag::err_format_attribute_not)
        << (not_nsstring_type ? "a string type" : "an NSString")
        << IdxExpr->getSourceRange() << getFunctionOrMethodParamRange(D, 0);
    return;
  }
  Ty = getFunctionOrMethodResultType(D);
  if (!isNSStringType(Ty, S.Context) &&
      !isCFStringType(Ty, S.Context) &&
      (!Ty->isPointerType() ||
       !Ty->getAs<PointerType>()->getPointeeType()->isCharType())) {
    S.Diag(Attr.getLoc(), diag::err_format_attribute_result_not)
        << (not_nsstring_type ? "string type" : "NSString")
        << IdxExpr->getSourceRange() << getFunctionOrMethodParamRange(D, 0);
    return;
  }

  // We cannot use the Idx returned from checkFunctionOrMethodParameterIndex
  // because that has corrected for the implicit this parameter, and is zero-
  // based.  The attribute expects what the user wrote explicitly.
  llvm::APSInt Val;
  IdxExpr->EvaluateAsInt(Val, S.Context);

  D->addAttr(::new (S.Context)
             FormatArgAttr(Attr.getRange(), S.Context, Val.getZExtValue(),
                           Attr.getAttributeSpellingListIndex()));
}

enum FormatAttrKind {
  CFStringFormat,
  NSStringFormat,
  StrftimeFormat,
  SupportedFormat,
  IgnoredFormat,
  InvalidFormat
};

/// getFormatAttrKind - Map from format attribute names to supported format
/// types.
static FormatAttrKind getFormatAttrKind(StringRef Format) {
  return llvm::StringSwitch<FormatAttrKind>(Format)
    // Check for formats that get handled specially.
    .Case("NSString", NSStringFormat)
    .Case("CFString", CFStringFormat)
    .Case("strftime", StrftimeFormat)

    // Otherwise, check for supported formats.
    .Cases("scanf", "printf", "printf0", "strfmon", SupportedFormat)
    .Cases("cmn_err", "vcmn_err", "zcmn_err", SupportedFormat)
    .Case("kprintf", SupportedFormat) // OpenBSD.
    .Case("freebsd_kprintf", SupportedFormat) // FreeBSD.
    .Case("os_trace", SupportedFormat)

    .Cases("gcc_diag", "gcc_cdiag", "gcc_cxxdiag", "gcc_tdiag", IgnoredFormat)
    .Default(InvalidFormat);
}

/// Handle __attribute__((init_priority(priority))) attributes based on
/// http://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Attributes.html
static void handleInitPriorityAttr(Sema &S, Decl *D,
                                   const AttributeList &Attr) {
  if (!S.getLangOpts().CPlusPlus) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_ignored) << Attr.getName();
    return;
  }
  
  if (S.getCurFunctionOrMethodDecl()) {
    S.Diag(Attr.getLoc(), diag::err_init_priority_object_attr);
    Attr.setInvalid();
    return;
  }
  QualType T = cast<VarDecl>(D)->getType();
  if (S.Context.getAsArrayType(T))
    T = S.Context.getBaseElementType(T);
  if (!T->getAs<RecordType>()) {
    S.Diag(Attr.getLoc(), diag::err_init_priority_object_attr);
    Attr.setInvalid();
    return;
  }

  Expr *E = Attr.getArgAsExpr(0);
  uint32_t prioritynum;
  if (!checkUInt32Argument(S, Attr, E, prioritynum)) {
    Attr.setInvalid();
    return;
  }

  if (prioritynum < 101 || prioritynum > 65535) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_outof_range)
      << E->getSourceRange();
    Attr.setInvalid();
    return;
  }
  D->addAttr(::new (S.Context)
             InitPriorityAttr(Attr.getRange(), S.Context, prioritynum,
                              Attr.getAttributeSpellingListIndex()));
}

FormatAttr *Sema::mergeFormatAttr(Decl *D, SourceRange Range,
                                  IdentifierInfo *Format, int FormatIdx,
                                  int FirstArg,
                                  unsigned AttrSpellingListIndex) {
  // Check whether we already have an equivalent format attribute.
  for (auto *F : D->specific_attrs<FormatAttr>()) {
    if (F->getType() == Format &&
        F->getFormatIdx() == FormatIdx &&
        F->getFirstArg() == FirstArg) {
      // If we don't have a valid location for this attribute, adopt the
      // location.
      if (F->getLocation().isInvalid())
        F->setRange(Range);
      return nullptr;
    }
  }

  return ::new (Context) FormatAttr(Range, Context, Format, FormatIdx,
                                    FirstArg, AttrSpellingListIndex);
}

/// Handle __attribute__((format(type,idx,firstarg))) attributes based on
/// http://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html
static void handleFormatAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  if (!Attr.isArgIdent(0)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_n_type)
      << Attr.getName() << 1 << AANT_ArgumentIdentifier;
    return;
  }

  // In C++ the implicit 'this' function parameter also counts, and they are
  // counted from one.
  bool HasImplicitThisParam = isInstanceMethod(D);
  unsigned NumArgs = getFunctionOrMethodNumParams(D) + HasImplicitThisParam;

  IdentifierInfo *II = Attr.getArgAsIdent(0)->Ident;
  StringRef Format = II->getName();

  // Normalize the argument, __foo__ becomes foo.
  if (Format.startswith("__") && Format.endswith("__")) {
    Format = Format.substr(2, Format.size() - 4);
    // If we've modified the string name, we need a new identifier for it.
    II = &S.Context.Idents.get(Format);
  }

  // Check for supported formats.
  FormatAttrKind Kind = getFormatAttrKind(Format);
  
  if (Kind == IgnoredFormat)
    return;
  
  if (Kind == InvalidFormat) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_type_not_supported)
      << Attr.getName() << II->getName();
    return;
  }

  // checks for the 2nd argument
  Expr *IdxExpr = Attr.getArgAsExpr(1);
  uint32_t Idx;
  if (!checkUInt32Argument(S, Attr, IdxExpr, Idx, 2))
    return;

  if (Idx < 1 || Idx > NumArgs) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_out_of_bounds)
      << Attr.getName() << 2 << IdxExpr->getSourceRange();
    return;
  }

  // FIXME: Do we need to bounds check?
  unsigned ArgIdx = Idx - 1;

  if (HasImplicitThisParam) {
    if (ArgIdx == 0) {
      S.Diag(Attr.getLoc(),
             diag::err_format_attribute_implicit_this_format_string)
        << IdxExpr->getSourceRange();
      return;
    }
    ArgIdx--;
  }

  // make sure the format string is really a string
  QualType Ty = getFunctionOrMethodParamType(D, ArgIdx);

  if (Kind == CFStringFormat) {
    if (!isCFStringType(Ty, S.Context)) {
      S.Diag(Attr.getLoc(), diag::err_format_attribute_not)
        << "a CFString" << IdxExpr->getSourceRange()
        << getFunctionOrMethodParamRange(D, ArgIdx);
      return;
    }
  } else if (Kind == NSStringFormat) {
    // FIXME: do we need to check if the type is NSString*?  What are the
    // semantics?
    if (!isNSStringType(Ty, S.Context)) {
      S.Diag(Attr.getLoc(), diag::err_format_attribute_not)
        << "an NSString" << IdxExpr->getSourceRange()
        << getFunctionOrMethodParamRange(D, ArgIdx);
      return;
    }
  } else if (!Ty->isPointerType() ||
             !Ty->getAs<PointerType>()->getPointeeType()->isCharType()) {
    S.Diag(Attr.getLoc(), diag::err_format_attribute_not)
      << "a string type" << IdxExpr->getSourceRange()
      << getFunctionOrMethodParamRange(D, ArgIdx);
    return;
  }

  // check the 3rd argument
  Expr *FirstArgExpr = Attr.getArgAsExpr(2);
  uint32_t FirstArg;
  if (!checkUInt32Argument(S, Attr, FirstArgExpr, FirstArg, 3))
    return;

  // check if the function is variadic if the 3rd argument non-zero
  if (FirstArg != 0) {
    if (isFunctionOrMethodVariadic(D)) {
      ++NumArgs; // +1 for ...
    } else {
      S.Diag(D->getLocation(), diag::err_format_attribute_requires_variadic);
      return;
    }
  }

  // strftime requires FirstArg to be 0 because it doesn't read from any
  // variable the input is just the current time + the format string.
  if (Kind == StrftimeFormat) {
    if (FirstArg != 0) {
      S.Diag(Attr.getLoc(), diag::err_format_strftime_third_parameter)
        << FirstArgExpr->getSourceRange();
      return;
    }
  // if 0 it disables parameter checking (to use with e.g. va_list)
  } else if (FirstArg != 0 && FirstArg != NumArgs) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_out_of_bounds)
      << Attr.getName() << 3 << FirstArgExpr->getSourceRange();
    return;
  }

  FormatAttr *NewAttr = S.mergeFormatAttr(D, Attr.getRange(), II,
                                          Idx, FirstArg,
                                          Attr.getAttributeSpellingListIndex());
  if (NewAttr)
    D->addAttr(NewAttr);
}

static void handleTransparentUnionAttr(Sema &S, Decl *D,
                                       const AttributeList &Attr) {
  // Try to find the underlying union declaration.
  RecordDecl *RD = nullptr;
  TypedefNameDecl *TD = dyn_cast<TypedefNameDecl>(D);
  if (TD && TD->getUnderlyingType()->isUnionType())
    RD = TD->getUnderlyingType()->getAsUnionType()->getDecl();
  else
    RD = dyn_cast<RecordDecl>(D);

  if (!RD || !RD->isUnion()) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedUnion;
    return;
  }

  if (!RD->isCompleteDefinition()) {
    S.Diag(Attr.getLoc(),
        diag::warn_transparent_union_attribute_not_definition);
    return;
  }

  RecordDecl::field_iterator Field = RD->field_begin(),
                          FieldEnd = RD->field_end();
  if (Field == FieldEnd) {
    S.Diag(Attr.getLoc(), diag::warn_transparent_union_attribute_zero_fields);
    return;
  }

  FieldDecl *FirstField = *Field;
  QualType FirstType = FirstField->getType();
  if (FirstType->hasFloatingRepresentation() || FirstType->isVectorType()) {
    S.Diag(FirstField->getLocation(),
           diag::warn_transparent_union_attribute_floating)
      << FirstType->isVectorType() << FirstType;
    return;
  }

  uint64_t FirstSize = S.Context.getTypeSize(FirstType);
  uint64_t FirstAlign = S.Context.getTypeAlign(FirstType);
  for (; Field != FieldEnd; ++Field) {
    QualType FieldType = Field->getType();
    // FIXME: this isn't fully correct; we also need to test whether the
    // members of the union would all have the same calling convention as the
    // first member of the union. Checking just the size and alignment isn't
    // sufficient (consider structs passed on the stack instead of in registers
    // as an example).
    if (S.Context.getTypeSize(FieldType) != FirstSize ||
        S.Context.getTypeAlign(FieldType) > FirstAlign) {
      // Warn if we drop the attribute.
      bool isSize = S.Context.getTypeSize(FieldType) != FirstSize;
      unsigned FieldBits = isSize? S.Context.getTypeSize(FieldType)
                                 : S.Context.getTypeAlign(FieldType);
      S.Diag(Field->getLocation(),
          diag::warn_transparent_union_attribute_field_size_align)
        << isSize << Field->getDeclName() << FieldBits;
      unsigned FirstBits = isSize? FirstSize : FirstAlign;
      S.Diag(FirstField->getLocation(),
             diag::note_transparent_union_first_field_size_align)
        << isSize << FirstBits;
      return;
    }
  }

  RD->addAttr(::new (S.Context)
              TransparentUnionAttr(Attr.getRange(), S.Context,
                                   Attr.getAttributeSpellingListIndex()));
}

static void handleAnnotateAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  // Make sure that there is a string literal as the annotation's single
  // argument.
  StringRef Str;
  if (!S.checkStringLiteralArgumentAttr(Attr, 0, Str))
    return;

  // Don't duplicate annotations that are already set.
  for (const auto *I : D->specific_attrs<AnnotateAttr>()) {
    if (I->getAnnotation() == Str)
      return;
  }
  
  D->addAttr(::new (S.Context)
             AnnotateAttr(Attr.getRange(), S.Context, Str,
                          Attr.getAttributeSpellingListIndex()));
}

static void handleAlignValueAttr(Sema &S, Decl *D,
                                 const AttributeList &Attr) {
  S.AddAlignValueAttr(Attr.getRange(), D, Attr.getArgAsExpr(0),
                      Attr.getAttributeSpellingListIndex());
}

void Sema::AddAlignValueAttr(SourceRange AttrRange, Decl *D, Expr *E,
                             unsigned SpellingListIndex) {
  AlignValueAttr TmpAttr(AttrRange, Context, E, SpellingListIndex);
  SourceLocation AttrLoc = AttrRange.getBegin();

  QualType T;
  if (TypedefNameDecl *TD = dyn_cast<TypedefNameDecl>(D))
    T = TD->getUnderlyingType();
  else if (ValueDecl *VD = dyn_cast<ValueDecl>(D))
    T = VD->getType();
  else
    llvm_unreachable("Unknown decl type for align_value");

  if (!T->isDependentType() && !T->isAnyPointerType() &&
      !T->isReferenceType() && !T->isMemberPointerType()) {
    Diag(AttrLoc, diag::warn_attribute_pointer_or_reference_only)
      << &TmpAttr /*TmpAttr.getName()*/ << T << D->getSourceRange();
    return;
  }

  if (!E->isValueDependent()) {
    llvm::APSInt Alignment(32);
    ExprResult ICE
      = VerifyIntegerConstantExpression(E, &Alignment,
          diag::err_align_value_attribute_argument_not_int,
            /*AllowFold*/ false);
    if (ICE.isInvalid())
      return;

    if (!Alignment.isPowerOf2()) {
      Diag(AttrLoc, diag::err_alignment_not_power_of_two)
        << E->getSourceRange();
      return;
    }

    D->addAttr(::new (Context)
               AlignValueAttr(AttrRange, Context, ICE.get(),
               SpellingListIndex));
    return;
  }

  // Save dependent expressions in the AST to be instantiated.
  D->addAttr(::new (Context) AlignValueAttr(TmpAttr));
  return;
}

static void handleAlignedAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  // check the attribute arguments.
  if (Attr.getNumArgs() > 1) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_number_arguments)
      << Attr.getName() << 1;
    return;
  }

  if (Attr.getNumArgs() == 0) {
    D->addAttr(::new (S.Context) AlignedAttr(Attr.getRange(), S.Context,
               true, nullptr, Attr.getAttributeSpellingListIndex()));
    return;
  }

  Expr *E = Attr.getArgAsExpr(0);
  if (Attr.isPackExpansion() && !E->containsUnexpandedParameterPack()) {
    S.Diag(Attr.getEllipsisLoc(),
           diag::err_pack_expansion_without_parameter_packs);
    return;
  }

  if (!Attr.isPackExpansion() && S.DiagnoseUnexpandedParameterPack(E))
    return;

  if (E->isValueDependent()) {
    if (const auto *TND = dyn_cast<TypedefNameDecl>(D)) {
      if (!TND->getUnderlyingType()->isDependentType()) {
        S.Diag(Attr.getLoc(), diag::err_alignment_dependent_typedef_name)
            << E->getSourceRange();
        return;
      }
    }
  }

  S.AddAlignedAttr(Attr.getRange(), D, E, Attr.getAttributeSpellingListIndex(),
                   Attr.isPackExpansion());
}

void Sema::AddAlignedAttr(SourceRange AttrRange, Decl *D, Expr *E,
                          unsigned SpellingListIndex, bool IsPackExpansion) {
  AlignedAttr TmpAttr(AttrRange, Context, true, E, SpellingListIndex);
  SourceLocation AttrLoc = AttrRange.getBegin();

  // C++11 alignas(...) and C11 _Alignas(...) have additional requirements.
  if (TmpAttr.isAlignas()) {
    // C++11 [dcl.align]p1:
    //   An alignment-specifier may be applied to a variable or to a class
    //   data member, but it shall not be applied to a bit-field, a function
    //   parameter, the formal parameter of a catch clause, or a variable
    //   declared with the register storage class specifier. An
    //   alignment-specifier may also be applied to the declaration of a class
    //   or enumeration type.
    // C11 6.7.5/2:
    //   An alignment attribute shall not be specified in a declaration of
    //   a typedef, or a bit-field, or a function, or a parameter, or an
    //   object declared with the register storage-class specifier.
    int DiagKind = -1;
    if (isa<ParmVarDecl>(D)) {
      DiagKind = 0;
    } else if (VarDecl *VD = dyn_cast<VarDecl>(D)) {
      if (VD->getStorageClass() == SC_Register)
        DiagKind = 1;
      if (VD->isExceptionVariable())
        DiagKind = 2;
    } else if (FieldDecl *FD = dyn_cast<FieldDecl>(D)) {
      if (FD->isBitField())
        DiagKind = 3;
    } else if (!isa<TagDecl>(D)) {
      Diag(AttrLoc, diag::err_attribute_wrong_decl_type) << &TmpAttr
        << (TmpAttr.isC11() ? ExpectedVariableOrField
                            : ExpectedVariableFieldOrTag);
      return;
    }
    if (DiagKind != -1) {
      Diag(AttrLoc, diag::err_alignas_attribute_wrong_decl_type)
        << &TmpAttr << DiagKind;
      return;
    }
  }

  if (E->isTypeDependent() || E->isValueDependent()) {
    // Save dependent expressions in the AST to be instantiated.
    AlignedAttr *AA = ::new (Context) AlignedAttr(TmpAttr);
    AA->setPackExpansion(IsPackExpansion);
    D->addAttr(AA);
    return;
  }

  // FIXME: Cache the number on the Attr object?
  llvm::APSInt Alignment(32);
  ExprResult ICE
    = VerifyIntegerConstantExpression(E, &Alignment,
        diag::err_aligned_attribute_argument_not_int,
        /*AllowFold*/ false);
  if (ICE.isInvalid())
    return;

  // C++11 [dcl.align]p2:
  //   -- if the constant expression evaluates to zero, the alignment
  //      specifier shall have no effect
  // C11 6.7.5p6:
  //   An alignment specification of zero has no effect.
  if (!(TmpAttr.isAlignas() && !Alignment)) {
    if(!llvm::isPowerOf2_64(Alignment.getZExtValue())) {
      Diag(AttrLoc, diag::err_alignment_not_power_of_two)
        << E->getSourceRange();
      return;
    }
    if (Context.getTargetInfo().isTLSSupported()) {
      if (unsigned MaxAlign = Context.getTargetInfo().getMaxTLSAlign()) {
        if (VarDecl *VD = dyn_cast<VarDecl>(D)) {
          if (VD->getTLSKind()) {
            CharUnits MaxAlignChars = Context.toCharUnitsFromBits(MaxAlign);
            if (Alignment.getSExtValue() > MaxAlignChars.getQuantity()) {
              Diag(VD->getLocation(), diag::err_tls_var_aligned_over_maximum)
                << (unsigned)Alignment.getZExtValue() << VD
                << (unsigned)MaxAlignChars.getQuantity();
              return;
            }
          }
        }
      }
    }
  }

  // Alignment calculations can wrap around if it's greater than 2**28.
  unsigned MaxValidAlignment = TmpAttr.isDeclspec() ? 8192 : 268435456;
  if (Alignment.getZExtValue() > MaxValidAlignment) {
    Diag(AttrLoc, diag::err_attribute_aligned_too_great) << MaxValidAlignment
                                                         << E->getSourceRange();
    return;
  }

  AlignedAttr *AA = ::new (Context) AlignedAttr(AttrRange, Context, true,
                                                ICE.get(), SpellingListIndex);
  AA->setPackExpansion(IsPackExpansion);
  D->addAttr(AA);
}

void Sema::AddAlignedAttr(SourceRange AttrRange, Decl *D, TypeSourceInfo *TS,
                          unsigned SpellingListIndex, bool IsPackExpansion) {
  // FIXME: Cache the number on the Attr object if non-dependent?
  // FIXME: Perform checking of type validity
  AlignedAttr *AA = ::new (Context) AlignedAttr(AttrRange, Context, false, TS,
                                                SpellingListIndex);
  AA->setPackExpansion(IsPackExpansion);
  D->addAttr(AA);
}

void Sema::CheckAlignasUnderalignment(Decl *D) {
  assert(D->hasAttrs() && "no attributes on decl");

  QualType UnderlyingTy, DiagTy;
  if (ValueDecl *VD = dyn_cast<ValueDecl>(D)) {
    UnderlyingTy = DiagTy = VD->getType();
  } else {
    UnderlyingTy = DiagTy = Context.getTagDeclType(cast<TagDecl>(D));
    if (EnumDecl *ED = dyn_cast<EnumDecl>(D))
      UnderlyingTy = ED->getIntegerType();
  }
  if (DiagTy->isDependentType() || DiagTy->isIncompleteType())
    return;

  // C++11 [dcl.align]p5, C11 6.7.5/4:
  //   The combined effect of all alignment attributes in a declaration shall
  //   not specify an alignment that is less strict than the alignment that
  //   would otherwise be required for the entity being declared.
  AlignedAttr *AlignasAttr = nullptr;
  unsigned Align = 0;
  for (auto *I : D->specific_attrs<AlignedAttr>()) {
    if (I->isAlignmentDependent())
      return;
    if (I->isAlignas())
      AlignasAttr = I;
    Align = std::max(Align, I->getAlignment(Context));
  }

  if (AlignasAttr && Align) {
    CharUnits RequestedAlign = Context.toCharUnitsFromBits(Align);
    CharUnits NaturalAlign = Context.getTypeAlignInChars(UnderlyingTy);
    if (NaturalAlign > RequestedAlign)
      Diag(AlignasAttr->getLocation(), diag::err_alignas_underaligned)
        << DiagTy << (unsigned)NaturalAlign.getQuantity();
  }
}

bool Sema::checkMSInheritanceAttrOnDefinition(
    CXXRecordDecl *RD, SourceRange Range, bool BestCase,
    MSInheritanceAttr::Spelling SemanticSpelling) {
  assert(RD->hasDefinition() && "RD has no definition!");

  // We may not have seen base specifiers or any virtual methods yet.  We will
  // have to wait until the record is defined to catch any mismatches.
  if (!RD->getDefinition()->isCompleteDefinition())
    return false;

  // The unspecified model never matches what a definition could need.
  if (SemanticSpelling == MSInheritanceAttr::Keyword_unspecified_inheritance)
    return false;

  if (BestCase) {
    if (RD->calculateInheritanceModel() == SemanticSpelling)
      return false;
  } else {
    if (RD->calculateInheritanceModel() <= SemanticSpelling)
      return false;
  }

  Diag(Range.getBegin(), diag::err_mismatched_ms_inheritance)
      << 0 /*definition*/;
  Diag(RD->getDefinition()->getLocation(), diag::note_defined_here)
      << RD->getNameAsString();
  return true;
}

/// handleModeAttr - This attribute modifies the width of a decl with primitive
/// type.
///
/// Despite what would be logical, the mode attribute is a decl attribute, not a
/// type attribute: 'int ** __attribute((mode(HI))) *G;' tries to make 'G' be
/// HImode, not an intermediate pointer.
static void handleModeAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  // This attribute isn't documented, but glibc uses it.  It changes
  // the width of an int or unsigned int to the specified size.
  if (!Attr.isArgIdent(0)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_type) << Attr.getName()
      << AANT_ArgumentIdentifier;
    return;
  }
  
  IdentifierInfo *Name = Attr.getArgAsIdent(0)->Ident;
  StringRef Str = Name->getName();

  // Normalize the attribute name, __foo__ becomes foo.
  if (Str.startswith("__") && Str.endswith("__"))
    Str = Str.substr(2, Str.size() - 4);

  unsigned DestWidth = 0;
  bool IntegerMode = true;
  bool ComplexMode = false;
  switch (Str.size()) {
  case 2:
    switch (Str[0]) {
    case 'Q': DestWidth = 8; break;
    case 'H': DestWidth = 16; break;
    case 'S': DestWidth = 32; break;
    case 'D': DestWidth = 64; break;
    case 'X': DestWidth = 96; break;
    case 'T': DestWidth = 128; break;
    }
    if (Str[1] == 'F') {
      IntegerMode = false;
    } else if (Str[1] == 'C') {
      IntegerMode = false;
      ComplexMode = true;
    } else if (Str[1] != 'I') {
      DestWidth = 0;
    }
    break;
  case 4:
    // FIXME: glibc uses 'word' to define register_t; this is narrower than a
    // pointer on PIC16 and other embedded platforms.
    if (Str == "word")
      DestWidth = S.Context.getTargetInfo().getPointerWidth(0);
    else if (Str == "byte")
      DestWidth = S.Context.getTargetInfo().getCharWidth();
    break;
  case 7:
    if (Str == "pointer")
      DestWidth = S.Context.getTargetInfo().getPointerWidth(0);
    break;
  case 11:
    if (Str == "unwind_word")
      DestWidth = S.Context.getTargetInfo().getUnwindWordWidth();
    break;
  }

  QualType OldTy;
  if (TypedefNameDecl *TD = dyn_cast<TypedefNameDecl>(D))
    OldTy = TD->getUnderlyingType();
  else if (ValueDecl *VD = dyn_cast<ValueDecl>(D))
    OldTy = VD->getType();
  else {
    S.Diag(D->getLocation(), diag::err_attr_wrong_decl)
      << Attr.getName() << Attr.getRange();
    return;
  }

  // Base type can also be a vector type (see PR17453).
  // Distinguish between base type and base element type.
  QualType OldElemTy = OldTy;
  if (const VectorType *VT = OldTy->getAs<VectorType>())
    OldElemTy = VT->getElementType();

  if (!OldElemTy->getAs<BuiltinType>() && !OldElemTy->isComplexType())
    S.Diag(Attr.getLoc(), diag::err_mode_not_primitive);
  else if (IntegerMode) {
    if (!OldElemTy->isIntegralOrEnumerationType())
      S.Diag(Attr.getLoc(), diag::err_mode_wrong_type);
  } else if (ComplexMode) {
    if (!OldElemTy->isComplexType())
      S.Diag(Attr.getLoc(), diag::err_mode_wrong_type);
  } else {
    if (!OldElemTy->isFloatingType())
      S.Diag(Attr.getLoc(), diag::err_mode_wrong_type);
  }

  // FIXME: Sync this with InitializePredefinedMacros; we need to match int8_t
  // and friends, at least with glibc.
  // FIXME: Make sure floating-point mappings are accurate
  // FIXME: Support XF and TF types
  if (!DestWidth) {
    S.Diag(Attr.getLoc(), diag::err_machine_mode) << 0 /*Unknown*/ << Name;
    return;
  }

  QualType NewElemTy;

  if (IntegerMode)
    NewElemTy = S.Context.getIntTypeForBitwidth(
        DestWidth, OldElemTy->isSignedIntegerType());
  else
    NewElemTy = S.Context.getRealTypeForBitwidth(DestWidth);

  if (NewElemTy.isNull()) {
    S.Diag(Attr.getLoc(), diag::err_machine_mode) << 1 /*Unsupported*/ << Name;
    return;
  }

  if (ComplexMode) {
    NewElemTy = S.Context.getComplexType(NewElemTy);
  }

  QualType NewTy = NewElemTy;
  if (const VectorType *OldVT = OldTy->getAs<VectorType>()) {
    // Complex machine mode does not support base vector types.
    if (ComplexMode) {
      S.Diag(Attr.getLoc(), diag::err_complex_mode_vector_type);
      return;
    }
    unsigned NumElements = S.Context.getTypeSize(OldElemTy) *
                           OldVT->getNumElements() /
                           S.Context.getTypeSize(NewElemTy);
    NewTy =
        S.Context.getVectorType(NewElemTy, NumElements, OldVT->getVectorKind());
  }

  if (NewTy.isNull()) {
    S.Diag(Attr.getLoc(), diag::err_mode_wrong_type);
    return;
  }

  // Install the new type.
  if (TypedefNameDecl *TD = dyn_cast<TypedefNameDecl>(D))
    TD->setModedTypeSourceInfo(TD->getTypeSourceInfo(), NewTy);
  else
    cast<ValueDecl>(D)->setType(NewTy);

  D->addAttr(::new (S.Context)
             ModeAttr(Attr.getRange(), S.Context, Name,
                      Attr.getAttributeSpellingListIndex()));
}

static void handleNoDebugAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  if (const VarDecl *VD = dyn_cast<VarDecl>(D)) {
    if (!VD->hasGlobalStorage())
      S.Diag(Attr.getLoc(),
             diag::warn_attribute_requires_functions_or_static_globals)
        << Attr.getName();
  } else if (!isFunctionOrMethod(D)) {
    S.Diag(Attr.getLoc(),
           diag::warn_attribute_requires_functions_or_static_globals)
      << Attr.getName();
    return;
  }

  D->addAttr(::new (S.Context)
             NoDebugAttr(Attr.getRange(), S.Context,
                         Attr.getAttributeSpellingListIndex()));
}

AlwaysInlineAttr *Sema::mergeAlwaysInlineAttr(Decl *D, SourceRange Range,
                                              IdentifierInfo *Ident,
                                              unsigned AttrSpellingListIndex) {
  if (OptimizeNoneAttr *Optnone = D->getAttr<OptimizeNoneAttr>()) {
    Diag(Range.getBegin(), diag::warn_attribute_ignored) << Ident;
    Diag(Optnone->getLocation(), diag::note_conflicting_attribute);
    return nullptr;
  }

  if (D->hasAttr<AlwaysInlineAttr>())
    return nullptr;

  return ::new (Context) AlwaysInlineAttr(Range, Context,
                                          AttrSpellingListIndex);
}

MinSizeAttr *Sema::mergeMinSizeAttr(Decl *D, SourceRange Range,
                                    unsigned AttrSpellingListIndex) {
  if (OptimizeNoneAttr *Optnone = D->getAttr<OptimizeNoneAttr>()) {
    Diag(Range.getBegin(), diag::warn_attribute_ignored) << "'minsize'";
    Diag(Optnone->getLocation(), diag::note_conflicting_attribute);
    return nullptr;
  }

  if (D->hasAttr<MinSizeAttr>())
    return nullptr;

  return ::new (Context) MinSizeAttr(Range, Context, AttrSpellingListIndex);
}

OptimizeNoneAttr *Sema::mergeOptimizeNoneAttr(Decl *D, SourceRange Range,
                                              unsigned AttrSpellingListIndex) {
  if (AlwaysInlineAttr *Inline = D->getAttr<AlwaysInlineAttr>()) {
    Diag(Inline->getLocation(), diag::warn_attribute_ignored) << Inline;
    Diag(Range.getBegin(), diag::note_conflicting_attribute);
    D->dropAttr<AlwaysInlineAttr>();
  }
  if (MinSizeAttr *MinSize = D->getAttr<MinSizeAttr>()) {
    Diag(MinSize->getLocation(), diag::warn_attribute_ignored) << MinSize;
    Diag(Range.getBegin(), diag::note_conflicting_attribute);
    D->dropAttr<MinSizeAttr>();
  }

  if (D->hasAttr<OptimizeNoneAttr>())
    return nullptr;

  return ::new (Context) OptimizeNoneAttr(Range, Context,
                                          AttrSpellingListIndex);
}

static void handleAlwaysInlineAttr(Sema &S, Decl *D,
                                   const AttributeList &Attr) {
  if (AlwaysInlineAttr *Inline = S.mergeAlwaysInlineAttr(
          D, Attr.getRange(), Attr.getName(),
          Attr.getAttributeSpellingListIndex()))
    D->addAttr(Inline);
}

static void handleMinSizeAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  if (MinSizeAttr *MinSize = S.mergeMinSizeAttr(
          D, Attr.getRange(), Attr.getAttributeSpellingListIndex()))
    D->addAttr(MinSize);
}

static void handleOptimizeNoneAttr(Sema &S, Decl *D,
                                   const AttributeList &Attr) {
  if (OptimizeNoneAttr *Optnone = S.mergeOptimizeNoneAttr(
          D, Attr.getRange(), Attr.getAttributeSpellingListIndex()))
    D->addAttr(Optnone);
}

static void handleGlobalAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  FunctionDecl *FD = cast<FunctionDecl>(D);
  if (!FD->getReturnType()->isVoidType()) {
    SourceRange RTRange = FD->getReturnTypeSourceRange();
    S.Diag(FD->getTypeSpecStartLoc(), diag::err_kern_type_not_void_return)
        << FD->getType()
        << (RTRange.isValid() ? FixItHint::CreateReplacement(RTRange, "void")
                              : FixItHint());
    return;
  }

  D->addAttr(::new (S.Context)
              CUDAGlobalAttr(Attr.getRange(), S.Context,
                             Attr.getAttributeSpellingListIndex()));
}

static void handleGNUInlineAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  FunctionDecl *Fn = cast<FunctionDecl>(D);
  if (!Fn->isInlineSpecified()) {
    S.Diag(Attr.getLoc(), diag::warn_gnu_inline_attribute_requires_inline);
    return;
  }

  D->addAttr(::new (S.Context)
             GNUInlineAttr(Attr.getRange(), S.Context,
                           Attr.getAttributeSpellingListIndex()));
}

static void handleCallConvAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  if (hasDeclarator(D)) return;

  // Diagnostic is emitted elsewhere: here we store the (valid) Attr
  // in the Decl node for syntactic reasoning, e.g., pretty-printing.
  CallingConv CC;
  if (S.CheckCallingConvAttr(Attr, CC, /*FD*/nullptr))
    return;

  if (!isa<ObjCMethodDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunctionOrMethod;
    return;
  }

  switch (Attr.getKind()) {
  case AttributeList::AT_FastCall:
    D->addAttr(::new (S.Context)
               FastCallAttr(Attr.getRange(), S.Context,
                            Attr.getAttributeSpellingListIndex()));
    return;
  case AttributeList::AT_StdCall:
    D->addAttr(::new (S.Context)
               StdCallAttr(Attr.getRange(), S.Context,
                           Attr.getAttributeSpellingListIndex()));
    return;
  case AttributeList::AT_ThisCall:
    D->addAttr(::new (S.Context)
               ThisCallAttr(Attr.getRange(), S.Context,
                            Attr.getAttributeSpellingListIndex()));
    return;
  case AttributeList::AT_CDecl:
    D->addAttr(::new (S.Context)
               CDeclAttr(Attr.getRange(), S.Context,
                         Attr.getAttributeSpellingListIndex()));
    return;
  case AttributeList::AT_Pascal:
    D->addAttr(::new (S.Context)
               PascalAttr(Attr.getRange(), S.Context,
                          Attr.getAttributeSpellingListIndex()));
    return;
  case AttributeList::AT_VectorCall:
    D->addAttr(::new (S.Context)
               VectorCallAttr(Attr.getRange(), S.Context,
                              Attr.getAttributeSpellingListIndex()));
    return;
  case AttributeList::AT_MSABI:
    D->addAttr(::new (S.Context)
               MSABIAttr(Attr.getRange(), S.Context,
                         Attr.getAttributeSpellingListIndex()));
    return;
  case AttributeList::AT_SysVABI:
    D->addAttr(::new (S.Context)
               SysVABIAttr(Attr.getRange(), S.Context,
                           Attr.getAttributeSpellingListIndex()));
    return;
  case AttributeList::AT_Pcs: {
    PcsAttr::PCSType PCS;
    switch (CC) {
    case CC_AAPCS:
      PCS = PcsAttr::AAPCS;
      break;
    case CC_AAPCS_VFP:
      PCS = PcsAttr::AAPCS_VFP;
      break;
    default:
      llvm_unreachable("unexpected calling convention in pcs attribute");
    }

    D->addAttr(::new (S.Context)
               PcsAttr(Attr.getRange(), S.Context, PCS,
                       Attr.getAttributeSpellingListIndex()));
    return;
  }
  case AttributeList::AT_IntelOclBicc:
    D->addAttr(::new (S.Context)
               IntelOclBiccAttr(Attr.getRange(), S.Context,
                                Attr.getAttributeSpellingListIndex()));
    return;

  default:
    llvm_unreachable("unexpected attribute kind");
  }
}

bool Sema::CheckCallingConvAttr(const AttributeList &attr, CallingConv &CC, 
                                const FunctionDecl *FD) {
  if (attr.isInvalid())
    return true;

  unsigned ReqArgs = attr.getKind() == AttributeList::AT_Pcs ? 1 : 0;
  if (!checkAttributeNumArgs(*this, attr, ReqArgs)) {
    attr.setInvalid();
    return true;
  }

  // TODO: diagnose uses of these conventions on the wrong target.
  switch (attr.getKind()) {
  case AttributeList::AT_CDecl: CC = CC_C; break;
  case AttributeList::AT_FastCall: CC = CC_X86FastCall; break;
  case AttributeList::AT_StdCall: CC = CC_X86StdCall; break;
  case AttributeList::AT_ThisCall: CC = CC_X86ThisCall; break;
  case AttributeList::AT_Pascal: CC = CC_X86Pascal; break;
  case AttributeList::AT_VectorCall: CC = CC_X86VectorCall; break;
  case AttributeList::AT_MSABI:
    CC = Context.getTargetInfo().getTriple().isOSWindows() ? CC_C :
                                                             CC_X86_64Win64;
    break;
  case AttributeList::AT_SysVABI:
    CC = Context.getTargetInfo().getTriple().isOSWindows() ? CC_X86_64SysV :
                                                             CC_C;
    break;
  case AttributeList::AT_Pcs: {
    StringRef StrRef;
    if (!checkStringLiteralArgumentAttr(attr, 0, StrRef)) {
      attr.setInvalid();
      return true;
    }
    if (StrRef == "aapcs") {
      CC = CC_AAPCS;
      break;
    } else if (StrRef == "aapcs-vfp") {
      CC = CC_AAPCS_VFP;
      break;
    }

    attr.setInvalid();
    Diag(attr.getLoc(), diag::err_invalid_pcs);
    return true;
  }
  case AttributeList::AT_IntelOclBicc: CC = CC_IntelOclBicc; break;
  default: llvm_unreachable("unexpected attribute kind");
  }

  const TargetInfo &TI = Context.getTargetInfo();
  TargetInfo::CallingConvCheckResult A = TI.checkCallingConvention(CC);
  if (A != TargetInfo::CCCR_OK) {
    if (A == TargetInfo::CCCR_Warning)
      Diag(attr.getLoc(), diag::warn_cconv_ignored) << attr.getName();

    // This convention is not valid for the target. Use the default function or
    // method calling convention.
    TargetInfo::CallingConvMethodType MT = TargetInfo::CCMT_Unknown;
    if (FD)
      MT = FD->isCXXInstanceMember() ? TargetInfo::CCMT_Member : 
                                    TargetInfo::CCMT_NonMember;
    CC = TI.getDefaultCallingConv(MT);
  }

  return false;
}

/// Checks a regparm attribute, returning true if it is ill-formed and
/// otherwise setting numParams to the appropriate value.
bool Sema::CheckRegparmAttr(const AttributeList &Attr, unsigned &numParams) {
  if (Attr.isInvalid())
    return true;

  if (!checkAttributeNumArgs(*this, Attr, 1)) {
    Attr.setInvalid();
    return true;
  }

  uint32_t NP;
  Expr *NumParamsExpr = Attr.getArgAsExpr(0);
  if (!checkUInt32Argument(*this, Attr, NumParamsExpr, NP)) {
    Attr.setInvalid();
    return true;
  }

  if (Context.getTargetInfo().getRegParmMax() == 0) {
    Diag(Attr.getLoc(), diag::err_attribute_regparm_wrong_platform)
      << NumParamsExpr->getSourceRange();
    Attr.setInvalid();
    return true;
  }

  numParams = NP;
  if (numParams > Context.getTargetInfo().getRegParmMax()) {
    Diag(Attr.getLoc(), diag::err_attribute_regparm_invalid_number)
      << Context.getTargetInfo().getRegParmMax() << NumParamsExpr->getSourceRange();
    Attr.setInvalid();
    return true;
  }

  return false;
}

// Checks whether an argument of launch_bounds attribute is acceptable
// May output an error.
static bool checkLaunchBoundsArgument(Sema &S, Expr *E,
                                      const CUDALaunchBoundsAttr &Attr,
                                      const unsigned Idx) {

  if (S.DiagnoseUnexpandedParameterPack(E))
    return false;

  // Accept template arguments for now as they depend on something else.
  // We'll get to check them when they eventually get instantiated.
  if (E->isValueDependent())
    return true;

  llvm::APSInt I(64);
  if (!E->isIntegerConstantExpr(I, S.Context)) {
    S.Diag(E->getExprLoc(), diag::err_attribute_argument_n_type)
        << &Attr << Idx << AANT_ArgumentIntegerConstant << E->getSourceRange();
    return false;
  }
  // Make sure we can fit it in 32 bits.
  if (!I.isIntN(32)) {
    S.Diag(E->getExprLoc(), diag::err_ice_too_large) << I.toString(10, false)
                                                     << 32 << /* Unsigned */ 1;
    return false;
  }
  if (I < 0)
    S.Diag(E->getExprLoc(), diag::warn_attribute_argument_n_negative)
        << &Attr << Idx << E->getSourceRange();

  return true;
}

void Sema::AddLaunchBoundsAttr(SourceRange AttrRange, Decl *D, Expr *MaxThreads,
                               Expr *MinBlocks, unsigned SpellingListIndex) {
  CUDALaunchBoundsAttr TmpAttr(AttrRange, Context, MaxThreads, MinBlocks,
                               SpellingListIndex);

  if (!checkLaunchBoundsArgument(*this, MaxThreads, TmpAttr, 0))
    return;

  if (MinBlocks && !checkLaunchBoundsArgument(*this, MinBlocks, TmpAttr, 1))
    return;

  D->addAttr(::new (Context) CUDALaunchBoundsAttr(
      AttrRange, Context, MaxThreads, MinBlocks, SpellingListIndex));
}

static void handleLaunchBoundsAttr(Sema &S, Decl *D,
                                   const AttributeList &Attr) {
  if (!checkAttributeAtLeastNumArgs(S, Attr, 1) ||
      !checkAttributeAtMostNumArgs(S, Attr, 2))
    return;

  S.AddLaunchBoundsAttr(Attr.getRange(), D, Attr.getArgAsExpr(0),
                        Attr.getNumArgs() > 1 ? Attr.getArgAsExpr(1) : nullptr,
                        Attr.getAttributeSpellingListIndex());
}

static void handleArgumentWithTypeTagAttr(Sema &S, Decl *D,
                                          const AttributeList &Attr) {
  if (!Attr.isArgIdent(0)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_n_type)
      << Attr.getName() << /* arg num = */ 1 << AANT_ArgumentIdentifier;
    return;
  }
  
  if (!checkAttributeNumArgs(S, Attr, 3))
    return;

  IdentifierInfo *ArgumentKind = Attr.getArgAsIdent(0)->Ident;

  if (!isFunctionOrMethod(D) || !hasFunctionProto(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedFunctionOrMethod;
    return;
  }

  uint64_t ArgumentIdx;
  if (!checkFunctionOrMethodParameterIndex(S, D, Attr, 2, Attr.getArgAsExpr(1),
                                           ArgumentIdx))
    return;

  uint64_t TypeTagIdx;
  if (!checkFunctionOrMethodParameterIndex(S, D, Attr, 3, Attr.getArgAsExpr(2),
                                           TypeTagIdx))
    return;

  bool IsPointer = (Attr.getName()->getName() == "pointer_with_type_tag");
  if (IsPointer) {
    // Ensure that buffer has a pointer type.
    QualType BufferTy = getFunctionOrMethodParamType(D, ArgumentIdx);
    if (!BufferTy->isPointerType()) {
      S.Diag(Attr.getLoc(), diag::err_attribute_pointers_only)
        << Attr.getName();
    }
  }

  D->addAttr(::new (S.Context)
             ArgumentWithTypeTagAttr(Attr.getRange(), S.Context, ArgumentKind,
                                     ArgumentIdx, TypeTagIdx, IsPointer,
                                     Attr.getAttributeSpellingListIndex()));
}

static void handleTypeTagForDatatypeAttr(Sema &S, Decl *D,
                                         const AttributeList &Attr) {
  if (!Attr.isArgIdent(0)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_n_type)
      << Attr.getName() << 1 << AANT_ArgumentIdentifier;
    return;
  }
  
  if (!checkAttributeNumArgs(S, Attr, 1))
    return;

  if (!isa<VarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedVariable;
    return;
  }

  IdentifierInfo *PointerKind = Attr.getArgAsIdent(0)->Ident;
  TypeSourceInfo *MatchingCTypeLoc = nullptr;
  S.GetTypeFromParser(Attr.getMatchingCType(), &MatchingCTypeLoc);
  assert(MatchingCTypeLoc && "no type source info for attribute argument");

  D->addAttr(::new (S.Context)
             TypeTagForDatatypeAttr(Attr.getRange(), S.Context, PointerKind,
                                    MatchingCTypeLoc,
                                    Attr.getLayoutCompatible(),
                                    Attr.getMustBeNull(),
                                    Attr.getAttributeSpellingListIndex()));
}

//===----------------------------------------------------------------------===//
// Checker-specific attribute handlers.
//===----------------------------------------------------------------------===//

static bool isValidSubjectOfNSReturnsRetainedAttribute(QualType type) {
  return type->isDependentType() ||
         type->isObjCRetainableType();
}

static bool isValidSubjectOfNSAttribute(Sema &S, QualType type) {
  return type->isDependentType() || 
         type->isObjCObjectPointerType() || 
         S.Context.isObjCNSObjectType(type);
}
static bool isValidSubjectOfCFAttribute(Sema &S, QualType type) {
  return type->isDependentType() || 
         type->isPointerType() || 
         isValidSubjectOfNSAttribute(S, type);
}

static void handleNSConsumedAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  ParmVarDecl *param = cast<ParmVarDecl>(D);
  bool typeOK, cf;

  if (Attr.getKind() == AttributeList::AT_NSConsumed) {
    typeOK = isValidSubjectOfNSAttribute(S, param->getType());
    cf = false;
  } else {
    typeOK = isValidSubjectOfCFAttribute(S, param->getType());
    cf = true;
  }

  if (!typeOK) {
    S.Diag(D->getLocStart(), diag::warn_ns_attribute_wrong_parameter_type)
      << Attr.getRange() << Attr.getName() << cf;
    return;
  }

  if (cf)
    param->addAttr(::new (S.Context)
                   CFConsumedAttr(Attr.getRange(), S.Context,
                                  Attr.getAttributeSpellingListIndex()));
  else
    param->addAttr(::new (S.Context)
                   NSConsumedAttr(Attr.getRange(), S.Context,
                                  Attr.getAttributeSpellingListIndex()));
}

static void handleNSReturnsRetainedAttr(Sema &S, Decl *D,
                                        const AttributeList &Attr) {

  QualType returnType;

  if (ObjCMethodDecl *MD = dyn_cast<ObjCMethodDecl>(D))
    returnType = MD->getReturnType();
  else if (S.getLangOpts().ObjCAutoRefCount && hasDeclarator(D) &&
           (Attr.getKind() == AttributeList::AT_NSReturnsRetained))
    return; // ignore: was handled as a type attribute
  else if (ObjCPropertyDecl *PD = dyn_cast<ObjCPropertyDecl>(D))
    returnType = PD->getType();
  else if (FunctionDecl *FD = dyn_cast<FunctionDecl>(D))
    returnType = FD->getReturnType();
  else if (auto *Param = dyn_cast<ParmVarDecl>(D)) {
    returnType = Param->getType()->getPointeeType();
    if (returnType.isNull()) {
      S.Diag(D->getLocStart(), diag::warn_ns_attribute_wrong_parameter_type)
          << Attr.getName() << /*pointer-to-CF*/2
          << Attr.getRange();
      return;
    }
  } else {
    AttributeDeclKind ExpectedDeclKind;
    switch (Attr.getKind()) {
    default: llvm_unreachable("invalid ownership attribute");
    case AttributeList::AT_NSReturnsRetained:
    case AttributeList::AT_NSReturnsAutoreleased:
    case AttributeList::AT_NSReturnsNotRetained:
      ExpectedDeclKind = ExpectedFunctionOrMethod;
      break;

    case AttributeList::AT_CFReturnsRetained:
    case AttributeList::AT_CFReturnsNotRetained:
      ExpectedDeclKind = ExpectedFunctionMethodOrParameter;
      break;
    }
    S.Diag(D->getLocStart(), diag::warn_attribute_wrong_decl_type)
        << Attr.getRange() << Attr.getName() << ExpectedDeclKind;
    return;
  }

  bool typeOK;
  bool cf;
  switch (Attr.getKind()) {
  default: llvm_unreachable("invalid ownership attribute");
  case AttributeList::AT_NSReturnsRetained:
    typeOK = isValidSubjectOfNSReturnsRetainedAttribute(returnType);
    cf = false;
    break;
      
  case AttributeList::AT_NSReturnsAutoreleased:
  case AttributeList::AT_NSReturnsNotRetained:
    typeOK = isValidSubjectOfNSAttribute(S, returnType);
    cf = false;
    break;

  case AttributeList::AT_CFReturnsRetained:
  case AttributeList::AT_CFReturnsNotRetained:
    typeOK = isValidSubjectOfCFAttribute(S, returnType);
    cf = true;
    break;
  }

  if (!typeOK) {
    if (isa<ParmVarDecl>(D)) {
      S.Diag(D->getLocStart(), diag::warn_ns_attribute_wrong_parameter_type)
          << Attr.getName() << /*pointer-to-CF*/2
          << Attr.getRange();
    } else {
      // Needs to be kept in sync with warn_ns_attribute_wrong_return_type.
      enum : unsigned {
        Function,
        Method,
        Property
      } SubjectKind = Function;
      if (isa<ObjCMethodDecl>(D))
        SubjectKind = Method;
      else if (isa<ObjCPropertyDecl>(D))
        SubjectKind = Property;
      S.Diag(D->getLocStart(), diag::warn_ns_attribute_wrong_return_type)
          << Attr.getName() << SubjectKind << cf
          << Attr.getRange();
    }
    return;
  }

  switch (Attr.getKind()) {
    default:
      llvm_unreachable("invalid ownership attribute");
    case AttributeList::AT_NSReturnsAutoreleased:
      D->addAttr(::new (S.Context) NSReturnsAutoreleasedAttr(
          Attr.getRange(), S.Context, Attr.getAttributeSpellingListIndex()));
      return;
    case AttributeList::AT_CFReturnsNotRetained:
      D->addAttr(::new (S.Context) CFReturnsNotRetainedAttr(
          Attr.getRange(), S.Context, Attr.getAttributeSpellingListIndex()));
      return;
    case AttributeList::AT_NSReturnsNotRetained:
      D->addAttr(::new (S.Context) NSReturnsNotRetainedAttr(
          Attr.getRange(), S.Context, Attr.getAttributeSpellingListIndex()));
      return;
    case AttributeList::AT_CFReturnsRetained:
      D->addAttr(::new (S.Context) CFReturnsRetainedAttr(
          Attr.getRange(), S.Context, Attr.getAttributeSpellingListIndex()));
      return;
    case AttributeList::AT_NSReturnsRetained:
      D->addAttr(::new (S.Context) NSReturnsRetainedAttr(
          Attr.getRange(), S.Context, Attr.getAttributeSpellingListIndex()));
      return;
  };
}

static void handleObjCReturnsInnerPointerAttr(Sema &S, Decl *D,
                                              const AttributeList &attr) {
  const int EP_ObjCMethod = 1;
  const int EP_ObjCProperty = 2;
  
  SourceLocation loc = attr.getLoc();
  QualType resultType;
  if (isa<ObjCMethodDecl>(D))
    resultType = cast<ObjCMethodDecl>(D)->getReturnType();
  else
    resultType = cast<ObjCPropertyDecl>(D)->getType();

  if (!resultType->isReferenceType() &&
      (!resultType->isPointerType() || resultType->isObjCRetainableType())) {
    S.Diag(D->getLocStart(), diag::warn_ns_attribute_wrong_return_type)
      << SourceRange(loc)
    << attr.getName()
    << (isa<ObjCMethodDecl>(D) ? EP_ObjCMethod : EP_ObjCProperty)
    << /*non-retainable pointer*/ 2;

    // Drop the attribute.
    return;
  }

  D->addAttr(::new (S.Context) ObjCReturnsInnerPointerAttr(
      attr.getRange(), S.Context, attr.getAttributeSpellingListIndex()));
}

static void handleObjCRequiresSuperAttr(Sema &S, Decl *D,
                                        const AttributeList &attr) {
  ObjCMethodDecl *method = cast<ObjCMethodDecl>(D);
  
  DeclContext *DC = method->getDeclContext();
  if (const ObjCProtocolDecl *PDecl = dyn_cast_or_null<ObjCProtocolDecl>(DC)) {
    S.Diag(D->getLocStart(), diag::warn_objc_requires_super_protocol)
    << attr.getName() << 0;
    S.Diag(PDecl->getLocation(), diag::note_protocol_decl);
    return;
  }
  if (method->getMethodFamily() == OMF_dealloc) {
    S.Diag(D->getLocStart(), diag::warn_objc_requires_super_protocol)
    << attr.getName() << 1;
    return;
  }
  
  method->addAttr(::new (S.Context)
                  ObjCRequiresSuperAttr(attr.getRange(), S.Context,
                                        attr.getAttributeSpellingListIndex()));
}

static void handleCFAuditedTransferAttr(Sema &S, Decl *D,
                                        const AttributeList &Attr) {
  if (checkAttrMutualExclusion<CFUnknownTransferAttr>(S, D, Attr))
    return;

  D->addAttr(::new (S.Context)
             CFAuditedTransferAttr(Attr.getRange(), S.Context,
                                   Attr.getAttributeSpellingListIndex()));
}

static void handleCFUnknownTransferAttr(Sema &S, Decl *D,
                                        const AttributeList &Attr) {
  if (checkAttrMutualExclusion<CFAuditedTransferAttr>(S, D, Attr))
    return;

  D->addAttr(::new (S.Context)
             CFUnknownTransferAttr(Attr.getRange(), S.Context,
             Attr.getAttributeSpellingListIndex()));
}

static void handleObjCBridgeAttr(Sema &S, Scope *Sc, Decl *D,
                                const AttributeList &Attr) {
  IdentifierLoc * Parm = Attr.isArgIdent(0) ? Attr.getArgAsIdent(0) : nullptr;

  if (!Parm) {
    S.Diag(D->getLocStart(), diag::err_objc_attr_not_id) << Attr.getName() << 0;
    return;
  }

  // Typedefs only allow objc_bridge(id) and have some additional checking.
  if (auto TD = dyn_cast<TypedefNameDecl>(D)) {
    if (!Parm->Ident->isStr("id")) {
      S.Diag(Attr.getLoc(), diag::err_objc_attr_typedef_not_id)
        << Attr.getName();
      return;
    }

    // Only allow 'cv void *'.
    QualType T = TD->getUnderlyingType();
    if (!T->isVoidPointerType()) {
      S.Diag(Attr.getLoc(), diag::err_objc_attr_typedef_not_void_pointer);
      return;
    }
  }
  
  D->addAttr(::new (S.Context)
             ObjCBridgeAttr(Attr.getRange(), S.Context, Parm->Ident,
                           Attr.getAttributeSpellingListIndex()));
}

static void handleObjCBridgeMutableAttr(Sema &S, Scope *Sc, Decl *D,
                                        const AttributeList &Attr) {
  IdentifierLoc * Parm = Attr.isArgIdent(0) ? Attr.getArgAsIdent(0) : nullptr;

  if (!Parm) {
    S.Diag(D->getLocStart(), diag::err_objc_attr_not_id) << Attr.getName() << 0;
    return;
  }
  
  D->addAttr(::new (S.Context)
             ObjCBridgeMutableAttr(Attr.getRange(), S.Context, Parm->Ident,
                            Attr.getAttributeSpellingListIndex()));
}

static void handleObjCBridgeRelatedAttr(Sema &S, Scope *Sc, Decl *D,
                                 const AttributeList &Attr) {
  IdentifierInfo *RelatedClass =
    Attr.isArgIdent(0) ? Attr.getArgAsIdent(0)->Ident : nullptr;
  if (!RelatedClass) {
    S.Diag(D->getLocStart(), diag::err_objc_attr_not_id) << Attr.getName() << 0;
    return;
  }
  IdentifierInfo *ClassMethod =
    Attr.getArgAsIdent(1) ? Attr.getArgAsIdent(1)->Ident : nullptr;
  IdentifierInfo *InstanceMethod =
    Attr.getArgAsIdent(2) ? Attr.getArgAsIdent(2)->Ident : nullptr;
  D->addAttr(::new (S.Context)
             ObjCBridgeRelatedAttr(Attr.getRange(), S.Context, RelatedClass,
                                   ClassMethod, InstanceMethod,
                                   Attr.getAttributeSpellingListIndex()));
}

static void handleObjCDesignatedInitializer(Sema &S, Decl *D,
                                            const AttributeList &Attr) {
  ObjCInterfaceDecl *IFace;
  if (ObjCCategoryDecl *CatDecl =
          dyn_cast<ObjCCategoryDecl>(D->getDeclContext()))
    IFace = CatDecl->getClassInterface();
  else
    IFace = cast<ObjCInterfaceDecl>(D->getDeclContext());

  if (!IFace)
    return;

  IFace->setHasDesignatedInitializers();
  D->addAttr(::new (S.Context)
                  ObjCDesignatedInitializerAttr(Attr.getRange(), S.Context,
                                         Attr.getAttributeSpellingListIndex()));
}

static void handleObjCRuntimeName(Sema &S, Decl *D,
                                  const AttributeList &Attr) {
  StringRef MetaDataName;
  if (!S.checkStringLiteralArgumentAttr(Attr, 0, MetaDataName))
    return;
  D->addAttr(::new (S.Context)
             ObjCRuntimeNameAttr(Attr.getRange(), S.Context,
                                 MetaDataName,
                                 Attr.getAttributeSpellingListIndex()));
}

// when a user wants to use objc_boxable with a union or struct
// but she doesn't have access to the declaration (legacy/third-party code)
// then she can 'enable' this feature via trick with a typedef
// e.g.:
// typedef struct __attribute((objc_boxable)) legacy_struct legacy_struct;
static void handleObjCBoxable(Sema &S, Decl *D, const AttributeList &Attr) {
  bool notify = false;

  RecordDecl *RD = dyn_cast<RecordDecl>(D);
  if (RD && RD->getDefinition()) {
    RD = RD->getDefinition();
    notify = true;
  }

  if (RD) {
    ObjCBoxableAttr *BoxableAttr = ::new (S.Context)
                          ObjCBoxableAttr(Attr.getRange(), S.Context,
                                          Attr.getAttributeSpellingListIndex());
    RD->addAttr(BoxableAttr);
    if (notify) {
      // we need to notify ASTReader/ASTWriter about
      // modification of existing declaration
      if (ASTMutationListener *L = S.getASTMutationListener())
        L->AddedAttributeToRecord(BoxableAttr, RD);
    }
  }
}

static void handleObjCOwnershipAttr(Sema &S, Decl *D,
                                    const AttributeList &Attr) {
  if (hasDeclarator(D)) return;

  S.Diag(D->getLocStart(), diag::err_attribute_wrong_decl_type)
    << Attr.getRange() << Attr.getName() << ExpectedVariable;
}

static void handleObjCPreciseLifetimeAttr(Sema &S, Decl *D,
                                          const AttributeList &Attr) {
  ValueDecl *vd = cast<ValueDecl>(D);
  QualType type = vd->getType();

  if (!type->isDependentType() &&
      !type->isObjCLifetimeType()) {
    S.Diag(Attr.getLoc(), diag::err_objc_precise_lifetime_bad_type)
      << type;
    return;
  }

  Qualifiers::ObjCLifetime lifetime = type.getObjCLifetime();

  // If we have no lifetime yet, check the lifetime we're presumably
  // going to infer.
  if (lifetime == Qualifiers::OCL_None && !type->isDependentType())
    lifetime = type->getObjCARCImplicitLifetime();

  switch (lifetime) {
  case Qualifiers::OCL_None:
    assert(type->isDependentType() &&
           "didn't infer lifetime for non-dependent type?");
    break;

  case Qualifiers::OCL_Weak:   // meaningful
  case Qualifiers::OCL_Strong: // meaningful
    break;

  case Qualifiers::OCL_ExplicitNone:
  case Qualifiers::OCL_Autoreleasing:
    S.Diag(Attr.getLoc(), diag::warn_objc_precise_lifetime_meaningless)
      << (lifetime == Qualifiers::OCL_Autoreleasing);
    break;
  }

  D->addAttr(::new (S.Context)
             ObjCPreciseLifetimeAttr(Attr.getRange(), S.Context,
                                     Attr.getAttributeSpellingListIndex()));
}

//===----------------------------------------------------------------------===//
// Microsoft specific attribute handlers.
//===----------------------------------------------------------------------===//

static void handleUuidAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  if (!S.LangOpts.CPlusPlus) {
    S.Diag(Attr.getLoc(), diag::err_attribute_not_supported_in_lang)
      << Attr.getName() << AttributeLangSupport::C;
    return;
  }

  if (!isa<CXXRecordDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << ExpectedClass;
    return;
  }

  StringRef StrRef;
  SourceLocation LiteralLoc;
  if (!S.checkStringLiteralArgumentAttr(Attr, 0, StrRef, &LiteralLoc))
    return;

  // GUID format is "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX" or
  // "{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}", normalize to the former.
  if (StrRef.size() == 38 && StrRef.front() == '{' && StrRef.back() == '}')
    StrRef = StrRef.drop_front().drop_back();

  // Validate GUID length.
  if (StrRef.size() != 36) {
    S.Diag(LiteralLoc, diag::err_attribute_uuid_malformed_guid);
    return;
  }

  for (unsigned i = 0; i < 36; ++i) {
    if (i == 8 || i == 13 || i == 18 || i == 23) {
      if (StrRef[i] != '-') {
        S.Diag(LiteralLoc, diag::err_attribute_uuid_malformed_guid);
        return;
      }
    } else if (!isHexDigit(StrRef[i])) {
      S.Diag(LiteralLoc, diag::err_attribute_uuid_malformed_guid);
      return;
    }
  }

  D->addAttr(::new (S.Context) UuidAttr(Attr.getRange(), S.Context, StrRef,
                                        Attr.getAttributeSpellingListIndex()));
}

static void handleMSInheritanceAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  if (!S.LangOpts.CPlusPlus) {
    S.Diag(Attr.getLoc(), diag::err_attribute_not_supported_in_lang)
      << Attr.getName() << AttributeLangSupport::C;
    return;
  }
  MSInheritanceAttr *IA = S.mergeMSInheritanceAttr(
      D, Attr.getRange(), /*BestCase=*/true,
      Attr.getAttributeSpellingListIndex(),
      (MSInheritanceAttr::Spelling)Attr.getSemanticSpelling());
  if (IA)
    D->addAttr(IA);
}

static void handleDeclspecThreadAttr(Sema &S, Decl *D,
                                     const AttributeList &Attr) {
  VarDecl *VD = cast<VarDecl>(D);
  if (!S.Context.getTargetInfo().isTLSSupported()) {
    S.Diag(Attr.getLoc(), diag::err_thread_unsupported);
    return;
  }
  if (VD->getTSCSpec() != TSCS_unspecified) {
    S.Diag(Attr.getLoc(), diag::err_declspec_thread_on_thread_variable);
    return;
  }
  if (VD->hasLocalStorage()) {
    S.Diag(Attr.getLoc(), diag::err_thread_non_global) << "__declspec(thread)";
    return;
  }
  VD->addAttr(::new (S.Context) ThreadAttr(
      Attr.getRange(), S.Context, Attr.getAttributeSpellingListIndex()));
}

static void handleARMInterruptAttr(Sema &S, Decl *D,
                                   const AttributeList &Attr) {
  // Check the attribute arguments.
  if (Attr.getNumArgs() > 1) {
    S.Diag(Attr.getLoc(), diag::err_attribute_too_many_arguments)
      << Attr.getName() << 1;
    return;
  }

  StringRef Str;
  SourceLocation ArgLoc;

  if (Attr.getNumArgs() == 0)
    Str = "";
  else if (!S.checkStringLiteralArgumentAttr(Attr, 0, Str, &ArgLoc))
    return;

  ARMInterruptAttr::InterruptType Kind;
  if (!ARMInterruptAttr::ConvertStrToInterruptType(Str, Kind)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_type_not_supported)
      << Attr.getName() << Str << ArgLoc;
    return;
  }

  unsigned Index = Attr.getAttributeSpellingListIndex();
  D->addAttr(::new (S.Context)
             ARMInterruptAttr(Attr.getLoc(), S.Context, Kind, Index));
}

static void handleMSP430InterruptAttr(Sema &S, Decl *D,
                                      const AttributeList &Attr) {
  if (!checkAttributeNumArgs(S, Attr, 1))
    return;

  if (!Attr.isArgExpr(0)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_type) << Attr.getName()
      << AANT_ArgumentIntegerConstant;
    return;    
  }

  // FIXME: Check for decl - it should be void ()(void).

  Expr *NumParamsExpr = static_cast<Expr *>(Attr.getArgAsExpr(0));
  llvm::APSInt NumParams(32);
  if (!NumParamsExpr->isIntegerConstantExpr(NumParams, S.Context)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_type)
      << Attr.getName() << AANT_ArgumentIntegerConstant
      << NumParamsExpr->getSourceRange();
    return;
  }

  unsigned Num = NumParams.getLimitedValue(255);
  if ((Num & 1) || Num > 30) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_out_of_bounds)
      << Attr.getName() << (int)NumParams.getSExtValue()
      << NumParamsExpr->getSourceRange();
    return;
  }

  D->addAttr(::new (S.Context)
              MSP430InterruptAttr(Attr.getLoc(), S.Context, Num,
                                  Attr.getAttributeSpellingListIndex()));
  D->addAttr(UsedAttr::CreateImplicit(S.Context));
}

static void handleInterruptAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  // Dispatch the interrupt attribute based on the current target.
  if (S.Context.getTargetInfo().getTriple().getArch() == llvm::Triple::msp430)
    handleMSP430InterruptAttr(S, D, Attr);
  else
    handleARMInterruptAttr(S, D, Attr);
}

static void handleAMDGPUNumVGPRAttr(Sema &S, Decl *D,
                                    const AttributeList &Attr) {
  uint32_t NumRegs;
  Expr *NumRegsExpr = static_cast<Expr *>(Attr.getArgAsExpr(0));
  if (!checkUInt32Argument(S, Attr, NumRegsExpr, NumRegs))
    return;

  D->addAttr(::new (S.Context)
             AMDGPUNumVGPRAttr(Attr.getLoc(), S.Context,
                               NumRegs,
                               Attr.getAttributeSpellingListIndex()));
}

static void handleAMDGPUNumSGPRAttr(Sema &S, Decl *D,
                                    const AttributeList &Attr) {
  uint32_t NumRegs;
  Expr *NumRegsExpr = static_cast<Expr *>(Attr.getArgAsExpr(0));
  if (!checkUInt32Argument(S, Attr, NumRegsExpr, NumRegs))
    return;

  D->addAttr(::new (S.Context)
             AMDGPUNumSGPRAttr(Attr.getLoc(), S.Context,
                               NumRegs,
                               Attr.getAttributeSpellingListIndex()));
}

static void handleX86ForceAlignArgPointerAttr(Sema &S, Decl *D,
                                              const AttributeList& Attr) {
  // If we try to apply it to a function pointer, don't warn, but don't
  // do anything, either. It doesn't matter anyway, because there's nothing
  // special about calling a force_align_arg_pointer function.
  ValueDecl *VD = dyn_cast<ValueDecl>(D);
  if (VD && VD->getType()->isFunctionPointerType())
    return;
  // Also don't warn on function pointer typedefs.
  TypedefNameDecl *TD = dyn_cast<TypedefNameDecl>(D);
  if (TD && (TD->getUnderlyingType()->isFunctionPointerType() ||
    TD->getUnderlyingType()->isFunctionType()))
    return;
  // Attribute can only be applied to function types.
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << /* function */0;
    return;
  }

  D->addAttr(::new (S.Context)
              X86ForceAlignArgPointerAttr(Attr.getRange(), S.Context,
                                        Attr.getAttributeSpellingListIndex()));
}

DLLImportAttr *Sema::mergeDLLImportAttr(Decl *D, SourceRange Range,
                                        unsigned AttrSpellingListIndex) {
  if (D->hasAttr<DLLExportAttr>()) {
    Diag(Range.getBegin(), diag::warn_attribute_ignored) << "'dllimport'";
    return nullptr;
  }

  if (D->hasAttr<DLLImportAttr>())
    return nullptr;

  return ::new (Context) DLLImportAttr(Range, Context, AttrSpellingListIndex);
}

DLLExportAttr *Sema::mergeDLLExportAttr(Decl *D, SourceRange Range,
                                        unsigned AttrSpellingListIndex) {
  if (DLLImportAttr *Import = D->getAttr<DLLImportAttr>()) {
    Diag(Import->getLocation(), diag::warn_attribute_ignored) << Import;
    D->dropAttr<DLLImportAttr>();
  }

  if (D->hasAttr<DLLExportAttr>())
    return nullptr;

  return ::new (Context) DLLExportAttr(Range, Context, AttrSpellingListIndex);
}

static void handleDLLAttr(Sema &S, Decl *D, const AttributeList &A) {
  if (isa<ClassTemplatePartialSpecializationDecl>(D) &&
      S.Context.getTargetInfo().getCXXABI().isMicrosoft()) {
    S.Diag(A.getRange().getBegin(), diag::warn_attribute_ignored)
        << A.getName();
    return;
  }

  if (FunctionDecl *FD = dyn_cast<FunctionDecl>(D)) {
    if (FD->isInlined() && A.getKind() == AttributeList::AT_DLLImport &&
        !S.Context.getTargetInfo().getCXXABI().isMicrosoft()) {
      // MinGW doesn't allow dllimport on inline functions.
      S.Diag(A.getRange().getBegin(), diag::warn_attribute_ignored_on_inline)
          << A.getName();
      return;
    }
  }

  unsigned Index = A.getAttributeSpellingListIndex();
  Attr *NewAttr = A.getKind() == AttributeList::AT_DLLExport
                      ? (Attr *)S.mergeDLLExportAttr(D, A.getRange(), Index)
                      : (Attr *)S.mergeDLLImportAttr(D, A.getRange(), Index);
  if (NewAttr)
    D->addAttr(NewAttr);
}

MSInheritanceAttr *
Sema::mergeMSInheritanceAttr(Decl *D, SourceRange Range, bool BestCase,
                             unsigned AttrSpellingListIndex,
                             MSInheritanceAttr::Spelling SemanticSpelling) {
  if (MSInheritanceAttr *IA = D->getAttr<MSInheritanceAttr>()) {
    if (IA->getSemanticSpelling() == SemanticSpelling)
      return nullptr;
    Diag(IA->getLocation(), diag::err_mismatched_ms_inheritance)
        << 1 /*previous declaration*/;
    Diag(Range.getBegin(), diag::note_previous_ms_inheritance);
    D->dropAttr<MSInheritanceAttr>();
  }

  CXXRecordDecl *RD = cast<CXXRecordDecl>(D);
  if (RD->hasDefinition()) {
    if (checkMSInheritanceAttrOnDefinition(RD, Range, BestCase,
                                           SemanticSpelling)) {
      return nullptr;
    }
  } else {
    if (isa<ClassTemplatePartialSpecializationDecl>(RD)) {
      Diag(Range.getBegin(), diag::warn_ignored_ms_inheritance)
          << 1 /*partial specialization*/;
      return nullptr;
    }
    if (RD->getDescribedClassTemplate()) {
      Diag(Range.getBegin(), diag::warn_ignored_ms_inheritance)
          << 0 /*primary template*/;
      return nullptr;
    }
  }

  return ::new (Context)
      MSInheritanceAttr(Range, Context, BestCase, AttrSpellingListIndex);
}

static void handleCapabilityAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  // The capability attributes take a single string parameter for the name of
  // the capability they represent. The lockable attribute does not take any
  // parameters. However, semantically, both attributes represent the same
  // concept, and so they use the same semantic attribute. Eventually, the
  // lockable attribute will be removed.
  //
  // For backward compatibility, any capability which has no specified string
  // literal will be considered a "mutex."
  StringRef N("mutex");
  SourceLocation LiteralLoc;
  if (Attr.getKind() == AttributeList::AT_Capability &&
      !S.checkStringLiteralArgumentAttr(Attr, 0, N, &LiteralLoc))
    return;

  // Currently, there are only two names allowed for a capability: role and
  // mutex (case insensitive). Diagnose other capability names.
  if (!N.equals_lower("mutex") && !N.equals_lower("role"))
    S.Diag(LiteralLoc, diag::warn_invalid_capability_name) << N;

  D->addAttr(::new (S.Context) CapabilityAttr(Attr.getRange(), S.Context, N,
                                        Attr.getAttributeSpellingListIndex()));
}

static void handleAssertCapabilityAttr(Sema &S, Decl *D,
                                       const AttributeList &Attr) {
  D->addAttr(::new (S.Context) AssertCapabilityAttr(Attr.getRange(), S.Context,
                                                    Attr.getArgAsExpr(0),
                                        Attr.getAttributeSpellingListIndex()));
}

static void handleAcquireCapabilityAttr(Sema &S, Decl *D,
                                        const AttributeList &Attr) {
  SmallVector<Expr*, 1> Args;
  if (!checkLockFunAttrCommon(S, D, Attr, Args))
    return;

  D->addAttr(::new (S.Context) AcquireCapabilityAttr(Attr.getRange(),
                                                     S.Context,
                                                     Args.data(), Args.size(),
                                        Attr.getAttributeSpellingListIndex()));
}

static void handleTryAcquireCapabilityAttr(Sema &S, Decl *D,
                                           const AttributeList &Attr) {
  SmallVector<Expr*, 2> Args;
  if (!checkTryLockFunAttrCommon(S, D, Attr, Args))
    return;

  D->addAttr(::new (S.Context) TryAcquireCapabilityAttr(Attr.getRange(),
                                                        S.Context,
                                                        Attr.getArgAsExpr(0),
                                                        Args.data(),
                                                        Args.size(),
                                        Attr.getAttributeSpellingListIndex()));
}

static void handleReleaseCapabilityAttr(Sema &S, Decl *D,
                                        const AttributeList &Attr) {
  // Check that all arguments are lockable objects.
  SmallVector<Expr *, 1> Args;
  checkAttrArgsAreCapabilityObjs(S, D, Attr, Args, 0, true);

  D->addAttr(::new (S.Context) ReleaseCapabilityAttr(
      Attr.getRange(), S.Context, Args.data(), Args.size(),
      Attr.getAttributeSpellingListIndex()));
}

static void handleRequiresCapabilityAttr(Sema &S, Decl *D,
                                         const AttributeList &Attr) {
  if (!checkAttributeAtLeastNumArgs(S, Attr, 1))
    return;

  // check that all arguments are lockable objects
  SmallVector<Expr*, 1> Args;
  checkAttrArgsAreCapabilityObjs(S, D, Attr, Args);
  if (Args.empty())
    return;

  RequiresCapabilityAttr *RCA = ::new (S.Context)
    RequiresCapabilityAttr(Attr.getRange(), S.Context, Args.data(),
                           Args.size(), Attr.getAttributeSpellingListIndex());

  D->addAttr(RCA);
}

static void handleDeprecatedAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  if (auto *NSD = dyn_cast<NamespaceDecl>(D)) {
    if (NSD->isAnonymousNamespace()) {
      S.Diag(Attr.getLoc(), diag::warn_deprecated_anonymous_namespace);
      // Do not want to attach the attribute to the namespace because that will
      // cause confusing diagnostic reports for uses of declarations within the
      // namespace.
      return;
    }
  }

  if (!S.getLangOpts().CPlusPlus14)
    if (Attr.isCXX11Attribute() &&
        !(Attr.hasScope() && Attr.getScopeName()->isStr("gnu")))
      S.Diag(Attr.getLoc(), diag::ext_deprecated_attr_is_a_cxx14_extension);

  handleAttrWithMessage<DeprecatedAttr>(S, D, Attr);
}

static void handleNoSanitizeAttr(Sema &S, Decl *D, const AttributeList &Attr) {
  if (!checkAttributeAtLeastNumArgs(S, Attr, 1))
    return;

  std::vector<std::string> Sanitizers;

  for (unsigned I = 0, E = Attr.getNumArgs(); I != E; ++I) {
    StringRef SanitizerName;
    SourceLocation LiteralLoc;

    if (!S.checkStringLiteralArgumentAttr(Attr, I, SanitizerName, &LiteralLoc))
      return;

    if (parseSanitizerValue(SanitizerName, /*AllowGroups=*/true) == 0)
      S.Diag(LiteralLoc, diag::warn_unknown_sanitizer_ignored) << SanitizerName;

    Sanitizers.push_back(SanitizerName);
  }

  D->addAttr(::new (S.Context) NoSanitizeAttr(
      Attr.getRange(), S.Context, Sanitizers.data(), Sanitizers.size(),
      Attr.getAttributeSpellingListIndex()));
}

static void handleNoSanitizeSpecificAttr(Sema &S, Decl *D,
                                         const AttributeList &Attr) {
  std::string SanitizerName =
      llvm::StringSwitch<std::string>(Attr.getName()->getName())
          .Case("no_address_safety_analysis", "address")
          .Case("no_sanitize_address", "address")
          .Case("no_sanitize_thread", "thread")
          .Case("no_sanitize_memory", "memory");
  D->addAttr(::new (S.Context)
                 NoSanitizeAttr(Attr.getRange(), S.Context, &SanitizerName, 1,
                                Attr.getAttributeSpellingListIndex()));
}

/// Handles semantic checking for features that are common to all attributes,
/// such as checking whether a parameter was properly specified, or the correct
/// number of arguments were passed, etc.
static bool handleCommonAttributeFeatures(Sema &S, Scope *scope, Decl *D,
                                          const AttributeList &Attr) {
  // Several attributes carry different semantics than the parsing requires, so
  // those are opted out of the common handling.
  //
  // We also bail on unknown and ignored attributes because those are handled
  // as part of the target-specific handling logic.
  if (Attr.hasCustomParsing() ||
      Attr.getKind() == AttributeList::UnknownAttribute)
    return false;

  // Check whether the attribute requires specific language extensions to be
  // enabled.
  if (!Attr.diagnoseLangOpts(S))
    return true;

  if (Attr.getMinArgs() == Attr.getMaxArgs()) {
    // If there are no optional arguments, then checking for the argument count
    // is trivial.
    if (!checkAttributeNumArgs(S, Attr, Attr.getMinArgs()))
      return true;
  } else {
    // There are optional arguments, so checking is slightly more involved.
    if (Attr.getMinArgs() &&
        !checkAttributeAtLeastNumArgs(S, Attr, Attr.getMinArgs()))
      return true;
    else if (!Attr.hasVariadicArg() && Attr.getMaxArgs() &&
             !checkAttributeAtMostNumArgs(S, Attr, Attr.getMaxArgs()))
      return true;
  }

  // Check whether the attribute appertains to the given subject.
  if (!Attr.diagnoseAppertainsTo(S, D))
    return true;

  return false;
}

//===----------------------------------------------------------------------===//
// Top Level Sema Entry Points
//===----------------------------------------------------------------------===//

/// ProcessDeclAttribute - Apply the specific attribute to the specified decl if
/// the attribute applies to decls.  If the attribute is a type attribute, just
/// silently ignore it if a GNU attribute.
static void ProcessDeclAttribute(Sema &S, Scope *scope, Decl *D,
                                 const AttributeList &Attr,
                                 bool IncludeCXX11Attributes) {
  if (Attr.isInvalid() || Attr.getKind() == AttributeList::IgnoredAttribute)
    return;

  // Ignore C++11 attributes on declarator chunks: they appertain to the type
  // instead.
  if (Attr.isCXX11Attribute() && !IncludeCXX11Attributes)
    return;

  // Unknown attributes are automatically warned on. Target-specific attributes
  // which do not apply to the current target architecture are treated as
  // though they were unknown attributes.
  if (Attr.getKind() == AttributeList::UnknownAttribute ||
      !Attr.existsInTarget(S.Context.getTargetInfo().getTriple())) {
    S.Diag(Attr.getLoc(), Attr.isDeclspecAttribute()
                              ? diag::warn_unhandled_ms_attribute_ignored
                              : diag::warn_unknown_attribute_ignored)
        << Attr.getName();
    return;
  }

  if (handleCommonAttributeFeatures(S, scope, D, Attr))
    return;

  switch (Attr.getKind()) {
  default:
    // HLSL Change Starts
    if (S.LangOpts.HLSL)
    {
      bool Handled = false;
      hlsl::HandleDeclAttributeForHLSL(S, D, Attr, Handled);
      if (Handled) break;
    }
    // HLSL Change Ends

    // Type attributes are handled elsewhere; silently move on.
    assert(Attr.isTypeAttr() && "Non-type attribute not handled");
    break;
  case AttributeList::AT_Interrupt:
    handleInterruptAttr(S, D, Attr);
    break;
  case AttributeList::AT_X86ForceAlignArgPointer:
    handleX86ForceAlignArgPointerAttr(S, D, Attr);
    break;
  case AttributeList::AT_DLLExport:
  case AttributeList::AT_DLLImport:
    handleDLLAttr(S, D, Attr);
    break;
  case AttributeList::AT_Mips16:
    handleSimpleAttribute<Mips16Attr>(S, D, Attr);
    break;
  case AttributeList::AT_NoMips16:
    handleSimpleAttribute<NoMips16Attr>(S, D, Attr);
    break;
  case AttributeList::AT_AMDGPUNumVGPR:
    handleAMDGPUNumVGPRAttr(S, D, Attr);
    break;
  case AttributeList::AT_AMDGPUNumSGPR:
    handleAMDGPUNumSGPRAttr(S, D, Attr);
    break;
  case AttributeList::AT_IBAction:
    handleSimpleAttribute<IBActionAttr>(S, D, Attr);
    break;
  case AttributeList::AT_IBOutlet:
    handleIBOutlet(S, D, Attr);
    break;
  case AttributeList::AT_IBOutletCollection:
    handleIBOutletCollection(S, D, Attr);
    break;
  case AttributeList::AT_Alias:
    handleAliasAttr(S, D, Attr);
    break;
  case AttributeList::AT_Aligned:
    handleAlignedAttr(S, D, Attr);
    break;
  case AttributeList::AT_AlignValue:
    handleAlignValueAttr(S, D, Attr);
    break;
  case AttributeList::AT_AlwaysInline:
    handleAlwaysInlineAttr(S, D, Attr);
    break;
  case AttributeList::AT_AnalyzerNoReturn:
    handleAnalyzerNoReturnAttr(S, D, Attr);
    break;
  case AttributeList::AT_TLSModel:
    handleTLSModelAttr(S, D, Attr);
    break;
  case AttributeList::AT_Annotate:
    handleAnnotateAttr(S, D, Attr);
    break;
  case AttributeList::AT_Availability:
    handleAvailabilityAttr(S, D, Attr);
    break;
  case AttributeList::AT_CarriesDependency:
    handleDependencyAttr(S, scope, D, Attr);
    break;
  case AttributeList::AT_Common:
    handleCommonAttr(S, D, Attr);
    break;
  case AttributeList::AT_CUDAConstant:
    handleSimpleAttribute<CUDAConstantAttr>(S, D, Attr);
    break;
  case AttributeList::AT_Constructor:
    handleConstructorAttr(S, D, Attr);
    break;
  case AttributeList::AT_CXX11NoReturn:
    handleSimpleAttribute<CXX11NoReturnAttr>(S, D, Attr);
    break;
  case AttributeList::AT_Deprecated:
    handleDeprecatedAttr(S, D, Attr);
    break;
  case AttributeList::AT_Destructor:
    handleDestructorAttr(S, D, Attr);
    break;
  case AttributeList::AT_EnableIf:
    handleEnableIfAttr(S, D, Attr);
    break;
  case AttributeList::AT_ExtVectorType:
    handleExtVectorTypeAttr(S, scope, D, Attr);
    break;
  case AttributeList::AT_MinSize:
    handleMinSizeAttr(S, D, Attr);
    break;
  case AttributeList::AT_OptimizeNone:
    handleOptimizeNoneAttr(S, D, Attr);
    break;
  case AttributeList::AT_FlagEnum:
    handleSimpleAttribute<FlagEnumAttr>(S, D, Attr);
    break;
  case AttributeList::AT_Flatten:
    handleSimpleAttribute<FlattenAttr>(S, D, Attr);
    break;
  case AttributeList::AT_Format:
    handleFormatAttr(S, D, Attr);
    break;
  case AttributeList::AT_FormatArg:
    handleFormatArgAttr(S, D, Attr);
    break;
  case AttributeList::AT_CUDAGlobal:
    handleGlobalAttr(S, D, Attr);
    break;
  case AttributeList::AT_CUDADevice:
    handleSimpleAttribute<CUDADeviceAttr>(S, D, Attr);
    break;
  case AttributeList::AT_CUDAHost:
    handleSimpleAttribute<CUDAHostAttr>(S, D, Attr);
    break;
  case AttributeList::AT_GNUInline:
    handleGNUInlineAttr(S, D, Attr);
    break;
  case AttributeList::AT_CUDALaunchBounds:
    handleLaunchBoundsAttr(S, D, Attr);
    break;
  case AttributeList::AT_Restrict:
    handleRestrictAttr(S, D, Attr);
    break;
  case AttributeList::AT_MayAlias:
    handleSimpleAttribute<MayAliasAttr>(S, D, Attr);
    break;
  case AttributeList::AT_Mode:
    handleModeAttr(S, D, Attr);
    break;
  case AttributeList::AT_NoCommon:
    handleSimpleAttribute<NoCommonAttr>(S, D, Attr);
    break;
  case AttributeList::AT_NoSplitStack:
    handleSimpleAttribute<NoSplitStackAttr>(S, D, Attr);
    break;
  case AttributeList::AT_NonNull:
    if (ParmVarDecl *PVD = dyn_cast<ParmVarDecl>(D))
      handleNonNullAttrParameter(S, PVD, Attr);
    else
      handleNonNullAttr(S, D, Attr);
    break;
  case AttributeList::AT_ReturnsNonNull:
    handleReturnsNonNullAttr(S, D, Attr);
    break;
  case AttributeList::AT_AssumeAligned:
    handleAssumeAlignedAttr(S, D, Attr);
    break;
  case AttributeList::AT_Overloadable:
    handleSimpleAttribute<OverloadableAttr>(S, D, Attr);
    break;
  case AttributeList::AT_Ownership:
    handleOwnershipAttr(S, D, Attr);
    break;
  case AttributeList::AT_Cold:
    handleColdAttr(S, D, Attr);
    break;
  case AttributeList::AT_Hot:
    handleHotAttr(S, D, Attr);
    break;
  case AttributeList::AT_Naked:
    handleSimpleAttribute<NakedAttr>(S, D, Attr);
    break;
  case AttributeList::AT_NoReturn:
    handleNoReturnAttr(S, D, Attr);
    break;
  case AttributeList::AT_NoThrow:
    handleSimpleAttribute<NoThrowAttr>(S, D, Attr);
    break;
  case AttributeList::AT_CUDAShared:
    handleSimpleAttribute<CUDASharedAttr>(S, D, Attr);
    break;
  case AttributeList::AT_VecReturn:
    handleVecReturnAttr(S, D, Attr);
    break;

  case AttributeList::AT_ObjCOwnership:
    handleObjCOwnershipAttr(S, D, Attr);
    break;
  case AttributeList::AT_ObjCPreciseLifetime:
    handleObjCPreciseLifetimeAttr(S, D, Attr);
    break;

  case AttributeList::AT_ObjCReturnsInnerPointer:
    handleObjCReturnsInnerPointerAttr(S, D, Attr);
    break;

  case AttributeList::AT_ObjCRequiresSuper:
    handleObjCRequiresSuperAttr(S, D, Attr);
    break;

  case AttributeList::AT_ObjCBridge:
    handleObjCBridgeAttr(S, scope, D, Attr);
    break;

  case AttributeList::AT_ObjCBridgeMutable:
    handleObjCBridgeMutableAttr(S, scope, D, Attr);
    break;

  case AttributeList::AT_ObjCBridgeRelated:
    handleObjCBridgeRelatedAttr(S, scope, D, Attr);
    break;

  case AttributeList::AT_ObjCDesignatedInitializer:
    handleObjCDesignatedInitializer(S, D, Attr);
    break;

  case AttributeList::AT_ObjCRuntimeName:
    handleObjCRuntimeName(S, D, Attr);
    break;

  case AttributeList::AT_ObjCBoxable:
    handleObjCBoxable(S, D, Attr);
    break;
          
  case AttributeList::AT_CFAuditedTransfer:
    handleCFAuditedTransferAttr(S, D, Attr);
    break;
  case AttributeList::AT_CFUnknownTransfer:
    handleCFUnknownTransferAttr(S, D, Attr);
    break;

  case AttributeList::AT_CFConsumed:
  case AttributeList::AT_NSConsumed:
    handleNSConsumedAttr(S, D, Attr);
    break;
  case AttributeList::AT_NSConsumesSelf:
    handleSimpleAttribute<NSConsumesSelfAttr>(S, D, Attr);
    break;

  case AttributeList::AT_NSReturnsAutoreleased:
  case AttributeList::AT_NSReturnsNotRetained:
  case AttributeList::AT_CFReturnsNotRetained:
  case AttributeList::AT_NSReturnsRetained:
  case AttributeList::AT_CFReturnsRetained:
    handleNSReturnsRetainedAttr(S, D, Attr);
    break;
  case AttributeList::AT_WorkGroupSizeHint:
    handleWorkGroupSize<WorkGroupSizeHintAttr>(S, D, Attr);
    break;
  case AttributeList::AT_ReqdWorkGroupSize:
    handleWorkGroupSize<ReqdWorkGroupSizeAttr>(S, D, Attr);
    break;
  case AttributeList::AT_VecTypeHint:
    handleVecTypeHint(S, D, Attr);
    break;

  case AttributeList::AT_InitPriority:
    handleInitPriorityAttr(S, D, Attr);
    break;

  case AttributeList::AT_Packed:
    handlePackedAttr(S, D, Attr);
    break;
  case AttributeList::AT_Section:
    handleSectionAttr(S, D, Attr);
    break;
  case AttributeList::AT_Target:
    handleTargetAttr(S, D, Attr);
    break;
  case AttributeList::AT_Unavailable:
    handleAttrWithMessage<UnavailableAttr>(S, D, Attr);
    break;
  case AttributeList::AT_ArcWeakrefUnavailable:
    handleSimpleAttribute<ArcWeakrefUnavailableAttr>(S, D, Attr);
    break;
  case AttributeList::AT_ObjCRootClass:
    handleSimpleAttribute<ObjCRootClassAttr>(S, D, Attr);
    break;
  case AttributeList::AT_ObjCExplicitProtocolImpl:
    handleObjCSuppresProtocolAttr(S, D, Attr);
    break;
  case AttributeList::AT_ObjCRequiresPropertyDefs:
    handleSimpleAttribute<ObjCRequiresPropertyDefsAttr>(S, D, Attr);
    break;
  case AttributeList::AT_Unused:
    handleSimpleAttribute<UnusedAttr>(S, D, Attr);
    break;
  case AttributeList::AT_ReturnsTwice:
    handleSimpleAttribute<ReturnsTwiceAttr>(S, D, Attr);
    break;
  case AttributeList::AT_Used:
    handleUsedAttr(S, D, Attr);
    break;
  case AttributeList::AT_Visibility:
    handleVisibilityAttr(S, D, Attr, false);
    break;
  case AttributeList::AT_TypeVisibility:
    handleVisibilityAttr(S, D, Attr, true);
    break;
  case AttributeList::AT_WarnUnused:
    handleSimpleAttribute<WarnUnusedAttr>(S, D, Attr);
    break;
  case AttributeList::AT_WarnUnusedResult:
    handleWarnUnusedResult(S, D, Attr);
    break;
  case AttributeList::AT_Weak:
    handleSimpleAttribute<WeakAttr>(S, D, Attr);
    break;
  case AttributeList::AT_WeakRef:
    handleWeakRefAttr(S, D, Attr);
    break;
  case AttributeList::AT_WeakImport:
    handleWeakImportAttr(S, D, Attr);
    break;
  case AttributeList::AT_TransparentUnion:
    handleTransparentUnionAttr(S, D, Attr);
    break;
  case AttributeList::AT_ObjCException:
    handleSimpleAttribute<ObjCExceptionAttr>(S, D, Attr);
    break;
  case AttributeList::AT_ObjCMethodFamily:
    handleObjCMethodFamilyAttr(S, D, Attr);
    break;
  case AttributeList::AT_ObjCNSObject:
    handleObjCNSObject(S, D, Attr);
    break;
  case AttributeList::AT_ObjCIndependentClass:
    handleObjCIndependentClass(S, D, Attr);
    break;
  case AttributeList::AT_Blocks:
    handleBlocksAttr(S, D, Attr);
    break;
  case AttributeList::AT_Sentinel:
    handleSentinelAttr(S, D, Attr);
    break;
  case AttributeList::AT_Const:
    handleSimpleAttribute<ConstAttr>(S, D, Attr);
    break;
  case AttributeList::AT_Pure:
    handleSimpleAttribute<PureAttr>(S, D, Attr);
    break;
  case AttributeList::AT_Cleanup:
    handleCleanupAttr(S, D, Attr);
    break;
  case AttributeList::AT_NoDebug:
    handleNoDebugAttr(S, D, Attr);
    break;
  case AttributeList::AT_NoDuplicate:
    handleSimpleAttribute<NoDuplicateAttr>(S, D, Attr);
    break;
  case AttributeList::AT_NoInline:
    handleSimpleAttribute<NoInlineAttr>(S, D, Attr);
    break;
  case AttributeList::AT_NoInstrumentFunction: // Interacts with -pg.
    handleSimpleAttribute<NoInstrumentFunctionAttr>(S, D, Attr);
    break;
  case AttributeList::AT_StdCall:
  case AttributeList::AT_CDecl:
  case AttributeList::AT_FastCall:
  case AttributeList::AT_ThisCall:
  case AttributeList::AT_Pascal:
  case AttributeList::AT_VectorCall:
  case AttributeList::AT_MSABI:
  case AttributeList::AT_SysVABI:
  case AttributeList::AT_Pcs:
  case AttributeList::AT_IntelOclBicc:
    handleCallConvAttr(S, D, Attr);
    break;
  case AttributeList::AT_OpenCLKernel:
    handleSimpleAttribute<OpenCLKernelAttr>(S, D, Attr);
    break;
  case AttributeList::AT_OpenCLImageAccess:
    handleSimpleAttribute<OpenCLImageAccessAttr>(S, D, Attr);
    break;

  // Microsoft attributes:
  case AttributeList::AT_MSNoVTable:
    handleSimpleAttribute<MSNoVTableAttr>(S, D, Attr);
    break;
  case AttributeList::AT_MSStruct:
    handleSimpleAttribute<MSStructAttr>(S, D, Attr);
    break;
  case AttributeList::AT_Uuid:
    handleUuidAttr(S, D, Attr);
    break;
  case AttributeList::AT_MSInheritance:
    handleMSInheritanceAttr(S, D, Attr);
    break;
  case AttributeList::AT_SelectAny:
    handleSimpleAttribute<SelectAnyAttr>(S, D, Attr);
    break;
  case AttributeList::AT_Thread:
    handleDeclspecThreadAttr(S, D, Attr);
    break;

  // Thread safety attributes:
  case AttributeList::AT_AssertExclusiveLock:
    handleAssertExclusiveLockAttr(S, D, Attr);
    break;
  case AttributeList::AT_AssertSharedLock:
    handleAssertSharedLockAttr(S, D, Attr);
    break;
  case AttributeList::AT_GuardedVar:
    handleSimpleAttribute<GuardedVarAttr>(S, D, Attr);
    break;
  case AttributeList::AT_PtGuardedVar:
    handlePtGuardedVarAttr(S, D, Attr);
    break;
  case AttributeList::AT_ScopedLockable:
    handleSimpleAttribute<ScopedLockableAttr>(S, D, Attr);
    break;
  case AttributeList::AT_NoSanitize:
    handleNoSanitizeAttr(S, D, Attr);
    break;
  case AttributeList::AT_NoSanitizeSpecific:
    handleNoSanitizeSpecificAttr(S, D, Attr);
    break;
  case AttributeList::AT_NoThreadSafetyAnalysis:
    handleSimpleAttribute<NoThreadSafetyAnalysisAttr>(S, D, Attr);
    break;
  case AttributeList::AT_GuardedBy:
    handleGuardedByAttr(S, D, Attr);
    break;
  case AttributeList::AT_PtGuardedBy:
    handlePtGuardedByAttr(S, D, Attr);
    break;
  case AttributeList::AT_ExclusiveTrylockFunction:
    handleExclusiveTrylockFunctionAttr(S, D, Attr);
    break;
  case AttributeList::AT_LockReturned:
    handleLockReturnedAttr(S, D, Attr);
    break;
  case AttributeList::AT_LocksExcluded:
    handleLocksExcludedAttr(S, D, Attr);
    break;
  case AttributeList::AT_SharedTrylockFunction:
    handleSharedTrylockFunctionAttr(S, D, Attr);
    break;
  case AttributeList::AT_AcquiredBefore:
    handleAcquiredBeforeAttr(S, D, Attr);
    break;
  case AttributeList::AT_AcquiredAfter:
    handleAcquiredAfterAttr(S, D, Attr);
    break;

  // Capability analysis attributes.
  case AttributeList::AT_Capability:
  case AttributeList::AT_Lockable:
    handleCapabilityAttr(S, D, Attr);
    break;
  case AttributeList::AT_RequiresCapability:
    handleRequiresCapabilityAttr(S, D, Attr);
    break;

  case AttributeList::AT_AssertCapability:
    handleAssertCapabilityAttr(S, D, Attr);
    break;
  case AttributeList::AT_AcquireCapability:
    handleAcquireCapabilityAttr(S, D, Attr);
    break;
  case AttributeList::AT_ReleaseCapability:
    handleReleaseCapabilityAttr(S, D, Attr);
    break;
  case AttributeList::AT_TryAcquireCapability:
    handleTryAcquireCapabilityAttr(S, D, Attr);
    break;

  // Consumed analysis attributes.
  case AttributeList::AT_Consumable:
    handleConsumableAttr(S, D, Attr);
    break;
  case AttributeList::AT_ConsumableAutoCast:
    handleSimpleAttribute<ConsumableAutoCastAttr>(S, D, Attr);
    break;
  case AttributeList::AT_ConsumableSetOnRead:
    handleSimpleAttribute<ConsumableSetOnReadAttr>(S, D, Attr);
    break;
  case AttributeList::AT_CallableWhen:
    handleCallableWhenAttr(S, D, Attr);
    break;
  case AttributeList::AT_ParamTypestate:
    handleParamTypestateAttr(S, D, Attr);
    break;
  case AttributeList::AT_ReturnTypestate:
    handleReturnTypestateAttr(S, D, Attr);
    break;
  case AttributeList::AT_SetTypestate:
    handleSetTypestateAttr(S, D, Attr);
    break;
  case AttributeList::AT_TestTypestate:
    handleTestTypestateAttr(S, D, Attr);
    break;

  // Type safety attributes.
  case AttributeList::AT_ArgumentWithTypeTag:
    handleArgumentWithTypeTagAttr(S, D, Attr);
    break;
  case AttributeList::AT_TypeTagForDatatype:
    handleTypeTagForDatatypeAttr(S, D, Attr);
    break;
  }
}

/// ProcessDeclAttributeList - Apply all the decl attributes in the specified
/// attribute list to the specified decl, ignoring any type attributes.
void Sema::ProcessDeclAttributeList(Scope *S, Decl *D,
                                    const AttributeList *AttrList,
                                    bool IncludeCXX11Attributes) {
  for (const AttributeList* l = AttrList; l; l = l->getNext())
    ProcessDeclAttribute(*this, S, D, *l, IncludeCXX11Attributes);

  // FIXME: We should be able to handle these cases in TableGen.
  // GCC accepts
  // static int a9 __attribute__((weakref));
  // but that looks really pointless. We reject it.
  if (D->hasAttr<WeakRefAttr>() && !D->hasAttr<AliasAttr>()) {
    Diag(AttrList->getLoc(), diag::err_attribute_weakref_without_alias)
      << cast<NamedDecl>(D);
    D->dropAttr<WeakRefAttr>();
    return;
  }

  // FIXME: We should be able to handle this in TableGen as well. It would be
  // good to have a way to specify "these attributes must appear as a group",
  // for these. Additionally, it would be good to have a way to specify "these
  // attribute must never appear as a group" for attributes like cold and hot.
  if (!D->hasAttr<OpenCLKernelAttr>()) {
    // These attributes cannot be applied to a non-kernel function.
    if (Attr *A = D->getAttr<ReqdWorkGroupSizeAttr>()) {
      // FIXME: This emits a different error message than
      // diag::err_attribute_wrong_decl_type + ExpectedKernelFunction.
      Diag(D->getLocation(), diag::err_opencl_kernel_attr) << A;
      D->setInvalidDecl();
    } else if (Attr *A = D->getAttr<WorkGroupSizeHintAttr>()) {
      Diag(D->getLocation(), diag::err_opencl_kernel_attr) << A;
      D->setInvalidDecl();
    } else if (Attr *A = D->getAttr<VecTypeHintAttr>()) {
      Diag(D->getLocation(), diag::err_opencl_kernel_attr) << A;
      D->setInvalidDecl();
    } else if (Attr *A = D->getAttr<AMDGPUNumVGPRAttr>()) {
      Diag(D->getLocation(), diag::err_attribute_wrong_decl_type)
        << A << ExpectedKernelFunction;
      D->setInvalidDecl();
    } else if (Attr *A = D->getAttr<AMDGPUNumSGPRAttr>()) {
      Diag(D->getLocation(), diag::err_attribute_wrong_decl_type)
        << A << ExpectedKernelFunction;
      D->setInvalidDecl();
    }
  }
}

// Annotation attributes are the only attributes allowed after an access
// specifier.
bool Sema::ProcessAccessDeclAttributeList(AccessSpecDecl *ASDecl,
                                          const AttributeList *AttrList) {
  for (const AttributeList* l = AttrList; l; l = l->getNext()) {
    if (l->getKind() == AttributeList::AT_Annotate) {
      ProcessDeclAttribute(*this, nullptr, ASDecl, *l, l->isCXX11Attribute());
    } else {
      Diag(l->getLoc(), diag::err_only_annotate_after_access_spec);
      return true;
    }
  }

  return false;
}

/// checkUnusedDeclAttributes - Check a list of attributes to see if it
/// contains any decl attributes that we should warn about.
static void checkUnusedDeclAttributes(Sema &S, const AttributeList *A) {
  for ( ; A; A = A->getNext()) {
    // Only warn if the attribute is an unignored, non-type attribute.
    if (A->isUsedAsTypeAttr() || A->isInvalid()) continue;
    if (A->getKind() == AttributeList::IgnoredAttribute) continue;

    if (A->getKind() == AttributeList::UnknownAttribute) {
      S.Diag(A->getLoc(), diag::warn_unknown_attribute_ignored)
        << A->getName() << A->getRange();
    } else {
      S.Diag(A->getLoc(), diag::warn_attribute_not_on_decl)
        << A->getName() << A->getRange();
    }
  }
}

/// checkUnusedDeclAttributes - Given a declarator which is not being
/// used to build a declaration, complain about any decl attributes
/// which might be lying around on it.
void Sema::checkUnusedDeclAttributes(Declarator &D) {
  ::checkUnusedDeclAttributes(*this, D.getDeclSpec().getAttributes().getList());
  ::checkUnusedDeclAttributes(*this, D.getAttributes());
  for (unsigned i = 0, e = D.getNumTypeObjects(); i != e; ++i)
    ::checkUnusedDeclAttributes(*this, D.getTypeObject(i).getAttrs());
}

/// DeclClonePragmaWeak - clone existing decl (maybe definition),
/// \#pragma weak needs a non-definition decl and source may not have one.
NamedDecl * Sema::DeclClonePragmaWeak(NamedDecl *ND, IdentifierInfo *II,
                                      SourceLocation Loc) {
  assert(isa<FunctionDecl>(ND) || isa<VarDecl>(ND));
  NamedDecl *NewD = nullptr;
  if (FunctionDecl *FD = dyn_cast<FunctionDecl>(ND)) {
    FunctionDecl *NewFD;
    // FIXME: Missing call to CheckFunctionDeclaration().
    // FIXME: Mangling?
    // FIXME: Is the qualifier info correct?
    // FIXME: Is the DeclContext correct?
    NewFD = FunctionDecl::Create(FD->getASTContext(), FD->getDeclContext(),
                                 Loc, Loc, DeclarationName(II),
                                 FD->getType(), FD->getTypeSourceInfo(),
                                 SC_None, false/*isInlineSpecified*/,
                                 FD->hasPrototype(),
                                 false/*isConstexprSpecified*/);
    NewD = NewFD;

    if (FD->getQualifier())
      NewFD->setQualifierInfo(FD->getQualifierLoc());

    // Fake up parameter variables; they are declared as if this were
    // a typedef.
    QualType FDTy = FD->getType();
    if (const FunctionProtoType *FT = FDTy->getAs<FunctionProtoType>()) {
      SmallVector<ParmVarDecl*, 16> Params;
      for (const auto &AI : FT->param_types()) {
        ParmVarDecl *Param = BuildParmVarDeclForTypedef(NewFD, Loc, AI);
        Param->setScopeInfo(0, Params.size());
        Params.push_back(Param);
      }
      NewFD->setParams(Params);
    }
  } else if (VarDecl *VD = dyn_cast<VarDecl>(ND)) {
    NewD = VarDecl::Create(VD->getASTContext(), VD->getDeclContext(),
                           VD->getInnerLocStart(), VD->getLocation(), II,
                           VD->getType(), VD->getTypeSourceInfo(),
                           VD->getStorageClass());
    if (VD->getQualifier()) {
      VarDecl *NewVD = cast<VarDecl>(NewD);
      NewVD->setQualifierInfo(VD->getQualifierLoc());
    }
  }
  return NewD;
}

/// DeclApplyPragmaWeak - A declaration (maybe definition) needs \#pragma weak
/// applied to it, possibly with an alias.
void Sema::DeclApplyPragmaWeak(Scope *S, NamedDecl *ND, WeakInfo &W) {
  if (W.getUsed()) return; // only do this once
  W.setUsed(true);
  if (W.getAlias()) { // clone decl, impersonate __attribute(weak,alias(...))
    IdentifierInfo *NDId = ND->getIdentifier();
    NamedDecl *NewD = DeclClonePragmaWeak(ND, W.getAlias(), W.getLocation());
    NewD->addAttr(AliasAttr::CreateImplicit(Context, NDId->getName(),
                                            W.getLocation()));
    NewD->addAttr(WeakAttr::CreateImplicit(Context, W.getLocation()));
    WeakTopLevelDecl.push_back(NewD);
    // FIXME: "hideous" code from Sema::LazilyCreateBuiltin
    // to insert Decl at TU scope, sorry.
    DeclContext *SavedContext = CurContext;
    CurContext = Context.getTranslationUnitDecl();
    NewD->setDeclContext(CurContext);
    NewD->setLexicalDeclContext(CurContext);
    PushOnScopeChains(NewD, S);
    CurContext = SavedContext;
  } else { // just add weak to existing
    ND->addAttr(WeakAttr::CreateImplicit(Context, W.getLocation()));
  }
}

void Sema::ProcessPragmaWeak(Scope *S, Decl *D) {
  // It's valid to "forward-declare" #pragma weak, in which case we
  // have to do this.
  LoadExternalWeakUndeclaredIdentifiers();
  if (!WeakUndeclaredIdentifiers.empty()) {
    NamedDecl *ND = nullptr;
    if (VarDecl *VD = dyn_cast<VarDecl>(D))
      if (VD->isExternC())
        ND = VD;
    if (FunctionDecl *FD = dyn_cast<FunctionDecl>(D))
      if (FD->isExternC())
        ND = FD;
    if (ND) {
      if (IdentifierInfo *Id = ND->getIdentifier()) {
        auto I = WeakUndeclaredIdentifiers.find(Id);
        if (I != WeakUndeclaredIdentifiers.end()) {
          WeakInfo W = I->second;
          DeclApplyPragmaWeak(S, ND, W);
          WeakUndeclaredIdentifiers[Id] = W;
        }
      }
    }
  }
}

/// ProcessDeclAttributes - Given a declarator (PD) with attributes indicated in
/// it, apply them to D.  This is a bit tricky because PD can have attributes
/// specified in many different places, and we need to find and apply them all.
void Sema::ProcessDeclAttributes(Scope *S, Decl *D, const Declarator &PD) {
  // Apply decl attributes from the DeclSpec if present.
  if (const AttributeList *Attrs = PD.getDeclSpec().getAttributes().getList())
    ProcessDeclAttributeList(S, D, Attrs);

  // Walk the declarator structure, applying decl attributes that were in a type
  // position to the decl itself.  This handles cases like:
  //   int *__attr__(x)** D;
  // when X is a decl attribute.
  for (unsigned i = 0, e = PD.getNumTypeObjects(); i != e; ++i)
    if (const AttributeList *Attrs = PD.getTypeObject(i).getAttrs())
      ProcessDeclAttributeList(S, D, Attrs, /*IncludeCXX11Attributes=*/false);

  // Finally, apply any attributes on the decl itself.
  if (const AttributeList *Attrs = PD.getAttributes())
    ProcessDeclAttributeList(S, D, Attrs);

  // HLSL Change Starts - Get custom attributes from our declarator object and append to AST Decl object
  for (std::vector<InheritableAttr*>::const_iterator
    i = PD.customAttributesList.begin(),
    e = PD.customAttributesList.end();
    i != e; ++i) {
    D->addAttr(*i);
  }
  // HLSL Change Ends
}

/// Is the given declaration allowed to use a forbidden type?
static bool isForbiddenTypeAllowed(Sema &S, Decl *decl) {
  // Private ivars are always okay.  Unfortunately, people don't
  // always properly make their ivars private, even in system headers.
  // Plus we need to make fields okay, too.
  // Function declarations in sys headers will be marked unavailable.
  if (!isa<FieldDecl>(decl) && !isa<ObjCPropertyDecl>(decl) &&
      !isa<FunctionDecl>(decl))
    return false;

  // Require it to be declared in a system header.
  return S.Context.getSourceManager().isInSystemHeader(decl->getLocation());
}

/// Handle a delayed forbidden-type diagnostic.
static void handleDelayedForbiddenType(Sema &S, DelayedDiagnostic &diag,
                                       Decl *decl) {
  if (decl && isForbiddenTypeAllowed(S, decl)) {
    decl->addAttr(UnavailableAttr::CreateImplicit(S.Context,
                        "this system declaration uses an unsupported type",
                        diag.Loc));
    return;
  }
  if (S.getLangOpts().ObjCAutoRefCount)
    if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(decl)) {
      // FIXME: we may want to suppress diagnostics for all
      // kind of forbidden type messages on unavailable functions. 
      if (FD->hasAttr<UnavailableAttr>() &&
          diag.getForbiddenTypeDiagnostic() == 
          diag::err_arc_array_param_no_ownership) {
        diag.Triggered = true;
        return;
      }
    }

  S.Diag(diag.Loc, diag.getForbiddenTypeDiagnostic())
    << diag.getForbiddenTypeOperand() << diag.getForbiddenTypeArgument();
  diag.Triggered = true;
}


static bool isDeclDeprecated(Decl *D) {
  do {
    if (D->isDeprecated())
      return true;
    // A category implicitly has the availability of the interface.
    if (const ObjCCategoryDecl *CatD = dyn_cast<ObjCCategoryDecl>(D))
      if (const ObjCInterfaceDecl *Interface = CatD->getClassInterface())
        return Interface->isDeprecated();
  } while ((D = cast_or_null<Decl>(D->getDeclContext())));
  return false;
}

static bool isDeclUnavailable(Decl *D) {
  do {
    if (D->isUnavailable())
      return true;
    // A category implicitly has the availability of the interface.
    if (const ObjCCategoryDecl *CatD = dyn_cast<ObjCCategoryDecl>(D))
      if (const ObjCInterfaceDecl *Interface = CatD->getClassInterface())
        return Interface->isUnavailable();
  } while ((D = cast_or_null<Decl>(D->getDeclContext())));
  return false;
}

static void DoEmitAvailabilityWarning(Sema &S, Sema::AvailabilityDiagnostic K,
                                      Decl *Ctx, const NamedDecl *D,
                                      StringRef Message, SourceLocation Loc,
                                      const ObjCInterfaceDecl *UnknownObjCClass,
                                      const ObjCPropertyDecl *ObjCProperty,
                                      bool ObjCPropertyAccess) {
  // Diagnostics for deprecated or unavailable.
  unsigned diag, diag_message, diag_fwdclass_message;

  // Matches 'diag::note_property_attribute' options.
  unsigned property_note_select;

  // Matches diag::note_availability_specified_here.
  unsigned available_here_select_kind;

  // Don't warn if our current context is deprecated or unavailable.
  switch (K) {
  case Sema::AD_Deprecation:
    if (isDeclDeprecated(Ctx) || isDeclUnavailable(Ctx))
      return;
    diag = !ObjCPropertyAccess ? diag::warn_deprecated
                               : diag::warn_property_method_deprecated;
    diag_message = diag::warn_deprecated_message;
    diag_fwdclass_message = diag::warn_deprecated_fwdclass_message;
    property_note_select = /* deprecated */ 0;
    available_here_select_kind = /* deprecated */ 2;
    break;

  case Sema::AD_Unavailable:
    if (isDeclUnavailable(Ctx))
      return;
    diag = !ObjCPropertyAccess ? diag::err_unavailable
                               : diag::err_property_method_unavailable;
    diag_message = diag::err_unavailable_message;
    diag_fwdclass_message = diag::warn_unavailable_fwdclass_message;
    property_note_select = /* unavailable */ 1;
    available_here_select_kind = /* unavailable */ 0;
    break;

  case Sema::AD_Partial:
    diag = diag::warn_partial_availability;
    diag_message = diag::warn_partial_message;
    diag_fwdclass_message = diag::warn_partial_fwdclass_message;
    property_note_select = /* partial */ 2;
    available_here_select_kind = /* partial */ 3;
    break;
  }

  if (!Message.empty()) {
    S.Diag(Loc, diag_message) << D << Message;
    if (ObjCProperty)
      S.Diag(ObjCProperty->getLocation(), diag::note_property_attribute)
          << ObjCProperty->getDeclName() << property_note_select;
  } else if (!UnknownObjCClass) {
    S.Diag(Loc, diag) << D;
    if (ObjCProperty)
      S.Diag(ObjCProperty->getLocation(), diag::note_property_attribute)
          << ObjCProperty->getDeclName() << property_note_select;
  } else {
    S.Diag(Loc, diag_fwdclass_message) << D;
    S.Diag(UnknownObjCClass->getLocation(), diag::note_forward_class);
  }

  S.Diag(D->getLocation(), diag::note_availability_specified_here)
      << D << available_here_select_kind;
  if (K == Sema::AD_Partial)
    S.Diag(Loc, diag::note_partial_availability_silence) << D;
}

static void handleDelayedAvailabilityCheck(Sema &S, DelayedDiagnostic &DD,
                                           Decl *Ctx) {
  assert(DD.Kind == DelayedDiagnostic::Deprecation ||
         DD.Kind == DelayedDiagnostic::Unavailable);
  Sema::AvailabilityDiagnostic AD = DD.Kind == DelayedDiagnostic::Deprecation
                                        ? Sema::AD_Deprecation
                                        : Sema::AD_Unavailable;
  DD.Triggered = true;
  DoEmitAvailabilityWarning(
      S, AD, Ctx, DD.getDeprecationDecl(), DD.getDeprecationMessage(), DD.Loc,
      DD.getUnknownObjCClass(), DD.getObjCProperty(), false);
}

void Sema::PopParsingDeclaration(ParsingDeclState state, Decl *decl) {
  assert(DelayedDiagnostics.getCurrentPool());
  DelayedDiagnosticPool &poppedPool = *DelayedDiagnostics.getCurrentPool();
  DelayedDiagnostics.popWithoutEmitting(state);

  // When delaying diagnostics to run in the context of a parsed
  // declaration, we only want to actually emit anything if parsing
  // succeeds.
  if (!decl) return;

  // We emit all the active diagnostics in this pool or any of its
  // parents.  In general, we'll get one pool for the decl spec
  // and a child pool for each declarator; in a decl group like:
  //   deprecated_typedef foo, *bar, baz();
  // only the declarator pops will be passed decls.  This is correct;
  // we really do need to consider delayed diagnostics from the decl spec
  // for each of the different declarations.
  const DelayedDiagnosticPool *pool = &poppedPool;
  do {
    for (DelayedDiagnosticPool::pool_iterator
           i = pool->pool_begin(), e = pool->pool_end(); i != e; ++i) {
      // This const_cast is a bit lame.  Really, Triggered should be mutable.
      DelayedDiagnostic &diag = const_cast<DelayedDiagnostic&>(*i);
      if (diag.Triggered)
        continue;

      switch (diag.Kind) {
      case DelayedDiagnostic::Deprecation:
      case DelayedDiagnostic::Unavailable:
        // Don't bother giving deprecation/unavailable diagnostics if
        // the decl is invalid.
        if (!decl->isInvalidDecl())
          handleDelayedAvailabilityCheck(*this, diag, decl);
        break;

      case DelayedDiagnostic::Access:
        HandleDelayedAccessCheck(diag, decl);
        break;

      case DelayedDiagnostic::ForbiddenType:
        handleDelayedForbiddenType(*this, diag, decl);
        break;
      }
    }
  } while ((pool = pool->getParent()));
}

/// Given a set of delayed diagnostics, re-emit them as if they had
/// been delayed in the current context instead of in the given pool.
/// Essentially, this just moves them to the current pool.
void Sema::redelayDiagnostics(DelayedDiagnosticPool &pool) {
  DelayedDiagnosticPool *curPool = DelayedDiagnostics.getCurrentPool();
  assert(curPool && "re-emitting in undelayed context not supported");
  curPool->steal(pool);
}

void Sema::EmitAvailabilityWarning(AvailabilityDiagnostic AD,
                                   NamedDecl *D, StringRef Message,
                                   SourceLocation Loc,
                                   const ObjCInterfaceDecl *UnknownObjCClass,
                                   const ObjCPropertyDecl  *ObjCProperty,
                                   bool ObjCPropertyAccess) {
  // Delay if we're currently parsing a declaration.
  if (DelayedDiagnostics.shouldDelayDiagnostics() && AD != AD_Partial) {
    DelayedDiagnostics.add(DelayedDiagnostic::makeAvailability(
        AD, Loc, D, UnknownObjCClass, ObjCProperty, Message,
        ObjCPropertyAccess));
    return;
  }

  Decl *Ctx = cast<Decl>(getCurLexicalContext());
  DoEmitAvailabilityWarning(*this, AD, Ctx, D, Message, Loc, UnknownObjCClass,
                            ObjCProperty, ObjCPropertyAccess);
}
