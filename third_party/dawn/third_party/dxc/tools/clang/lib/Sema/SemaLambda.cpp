//===--- SemaLambda.cpp - Semantic Analysis for C++11 Lambdas -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements semantic analysis for C++ lambda expressions.
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/DeclSpec.h"
#include "TypeLocBuilder.h"
#include "clang/AST/ASTLambda.h"
#include "clang/AST/ExprCXX.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Sema/Initialization.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/ScopeInfo.h"
#include "clang/Sema/SemaInternal.h"
#include "clang/Sema/SemaLambda.h"
using namespace clang;
using namespace sema;

// HLSL Change Starts
// No lambda support, so simply skip all of this compilation.
// Here are enough stubs to link the current targets.
#if 1

/// \brief Determine whether the given context is or is enclosed in an inline
/// function.
static bool isInInlineFunction(const DeclContext *DC) {
  while (!DC->isFileContext()) {
    if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(DC))
      if (FD->isInlined())
        return true;

    DC = DC->getLexicalParent();
  }

  return false;
}

Optional<unsigned> clang::getStackIndexOfNearestEnclosingCaptureCapableLambda(
  ArrayRef<const sema::FunctionScopeInfo *> FunctionScopes,
  VarDecl *VarToCapture, Sema &S) {
  llvm_unreachable("HLSL does not support lambdas");
}

CXXRecordDecl *Sema::createLambdaClosureType(SourceRange IntroducerRange,
  TypeSourceInfo *Info,
  bool KnownDependent,
  LambdaCaptureDefault CaptureDefault) {
  llvm_unreachable("HLSL does not support lambdas");
}

MangleNumberingContext *
Sema::getCurrentMangleNumberContext(const DeclContext *DC,
  Decl *&ManglingContextDecl) {
  // Compute the context for allocating mangling numbers in the current
  // expression, if the ABI requires them.
  ManglingContextDecl = ExprEvalContexts.back().ManglingContextDecl;

  enum ContextKind {
    Normal,
    DefaultArgument,
    DataMember,
    StaticDataMember
  } Kind = Normal;

  // Default arguments of member function parameters that appear in a class
  // definition, as well as the initializers of data members, receive special
  // treatment. Identify them.
  if (ManglingContextDecl) {
    if (ParmVarDecl *Param = dyn_cast<ParmVarDecl>(ManglingContextDecl)) {
      if (const DeclContext *LexicalDC
        = Param->getDeclContext()->getLexicalParent())
        if (LexicalDC->isRecord())
          Kind = DefaultArgument;
    }
    else if (VarDecl *Var = dyn_cast<VarDecl>(ManglingContextDecl)) {
      if (Var->getDeclContext()->isRecord())
        Kind = StaticDataMember;
    }
    else if (isa<FieldDecl>(ManglingContextDecl)) {
      Kind = DataMember;
    }
  }

  // Itanium ABI [5.1.7]:
  //   In the following contexts [...] the one-definition rule requires closure
  //   types in different translation units to "correspond":
  bool IsInNonspecializedTemplate =
    !ActiveTemplateInstantiations.empty() || CurContext->isDependentContext();
  switch (Kind) {
  case Normal:
    //  -- the bodies of non-exported nonspecialized template functions
    //  -- the bodies of inline functions
    if ((IsInNonspecializedTemplate &&
      !(ManglingContextDecl && isa<ParmVarDecl>(ManglingContextDecl))) ||
      isInInlineFunction(CurContext)) {
      ManglingContextDecl = nullptr;
      return &Context.getManglingNumberContext(DC);
    }

    ManglingContextDecl = nullptr;
    return nullptr;

  case StaticDataMember:
    //  -- the initializers of nonspecialized static members of template classes
    if (!IsInNonspecializedTemplate) {
      ManglingContextDecl = nullptr;
      return nullptr;
    }
    // Fall through to get the current context.
    LLVM_FALLTHROUGH; // HLSL Change

  case DataMember:
    //  -- the in-class initializers of class members
  case DefaultArgument:
    //  -- default arguments appearing in class definitions
    return &ExprEvalContexts.back().getMangleNumberingContext(Context);
  }

  llvm_unreachable("unexpected context");
}

MangleNumberingContext &
Sema::ExpressionEvaluationContextRecord::getMangleNumberingContext(
  ASTContext &Ctx) {
  assert(ManglingContextDecl && "Need to have a context declaration");
  if (!MangleNumbering)
    MangleNumbering = Ctx.createMangleNumberingContext();
  return *MangleNumbering;
}

CXXMethodDecl *Sema::startLambdaDefinition(CXXRecordDecl *Class,
  SourceRange IntroducerRange,
  TypeSourceInfo *MethodTypeInfo,
  SourceLocation EndLoc,
  ArrayRef<ParmVarDecl *> Params) {
  llvm_unreachable("HLSL does not support lambdas");
}

void Sema::buildLambdaScope(LambdaScopeInfo *LSI,
  CXXMethodDecl *CallOperator,
  SourceRange IntroducerRange,
  LambdaCaptureDefault CaptureDefault,
  SourceLocation CaptureDefaultLoc,
  bool ExplicitParams,
  bool ExplicitResultType,
  bool Mutable) {
  llvm_unreachable("HLSL does not support lambdas");
}

void Sema::finishLambdaExplicitCaptures(LambdaScopeInfo *LSI) {
  llvm_unreachable("HLSL does not support lambdas");
}

void Sema::addLambdaParameters(CXXMethodDecl *CallOperator, Scope *CurScope) {
  llvm_unreachable("HLSL does not support lambdas");
}

void Sema::deduceClosureReturnType(CapturingScopeInfo &CSI) {
  llvm_unreachable("HLSL does not support lambdas");
}

QualType Sema::performLambdaInitCaptureInitialization(SourceLocation Loc,
  bool ByRef,
  IdentifierInfo *Id,
  Expr *&Init) {
  llvm_unreachable("HLSL does not support lambdas");
}

VarDecl *Sema::createLambdaInitCaptureVarDecl(SourceLocation Loc,
  QualType InitCaptureType, IdentifierInfo *Id, Expr *Init) {
  llvm_unreachable("HLSL does not support lambdas");
}

FieldDecl *Sema::buildInitCaptureField(LambdaScopeInfo *LSI, VarDecl *Var) {
  llvm_unreachable("HLSL does not support lambdas");
}

void Sema::ActOnStartOfLambdaDefinition(LambdaIntroducer &Intro,
  Declarator &ParamInfo,
  Scope *CurScope) {
  llvm_unreachable("HLSL does not support lambdas");
}

void Sema::ActOnLambdaError(SourceLocation StartLoc, Scope *CurScope,
  bool IsInstantiation) {
  llvm_unreachable("HLSL does not support lambdas");
}

ExprResult Sema::ActOnLambdaExpr(SourceLocation StartLoc, Stmt *Body,
  Scope *CurScope) {
  llvm_unreachable("HLSL does not support lambdas");
  return ExprError();
}

ExprResult Sema::BuildLambdaExpr(SourceLocation StartLoc, SourceLocation EndLoc,
  LambdaScopeInfo *LSI) {
  llvm_unreachable("HLSL does not support lambdas");
}

ExprResult Sema::BuildBlockForLambdaConversion(SourceLocation CurrentLocation,
  SourceLocation ConvLocation,
  CXXConversionDecl *Conv,
  Expr *Src) {
  llvm_unreachable("HLSL does not support lambdas");
}

#else
// HLSL Change Ends

/// \brief Examines the FunctionScopeInfo stack to determine the nearest
/// enclosing lambda (to the current lambda) that is 'capture-ready' for 
/// the variable referenced in the current lambda (i.e. \p VarToCapture).
/// If successful, returns the index into Sema's FunctionScopeInfo stack
/// of the capture-ready lambda's LambdaScopeInfo.
///  
/// Climbs down the stack of lambdas (deepest nested lambda - i.e. current 
/// lambda - is on top) to determine the index of the nearest enclosing/outer
/// lambda that is ready to capture the \p VarToCapture being referenced in 
/// the current lambda. 
/// As we climb down the stack, we want the index of the first such lambda -
/// that is the lambda with the highest index that is 'capture-ready'. 
/// 
/// A lambda 'L' is capture-ready for 'V' (var or this) if:
///  - its enclosing context is non-dependent
///  - and if the chain of lambdas between L and the lambda in which
///    V is potentially used (i.e. the lambda at the top of the scope info 
///    stack), can all capture or have already captured V.
/// If \p VarToCapture is 'null' then we are trying to capture 'this'.
/// 
/// Note that a lambda that is deemed 'capture-ready' still needs to be checked
/// for whether it is 'capture-capable' (see
/// getStackIndexOfNearestEnclosingCaptureCapableLambda), before it can truly 
/// capture.
///
/// \param FunctionScopes - Sema's stack of nested FunctionScopeInfo's (which a
///  LambdaScopeInfo inherits from).  The current/deepest/innermost lambda
///  is at the top of the stack and has the highest index.
/// \param VarToCapture - the variable to capture.  If NULL, capture 'this'.
///
/// \returns An Optional<unsigned> Index that if evaluates to 'true' contains
/// the index (into Sema's FunctionScopeInfo stack) of the innermost lambda
/// which is capture-ready.  If the return value evaluates to 'false' then
/// no lambda is capture-ready for \p VarToCapture.

static inline Optional<unsigned>
getStackIndexOfNearestEnclosingCaptureReadyLambda(
    ArrayRef<const clang::sema::FunctionScopeInfo *> FunctionScopes,
    VarDecl *VarToCapture) {
  // Label failure to capture.
  const Optional<unsigned> NoLambdaIsCaptureReady;

  assert(
      isa<clang::sema::LambdaScopeInfo>(
          FunctionScopes[FunctionScopes.size() - 1]) &&
      "The function on the top of sema's function-info stack must be a lambda");
  
  // If VarToCapture is null, we are attempting to capture 'this'.
  const bool IsCapturingThis = !VarToCapture;
  const bool IsCapturingVariable = !IsCapturingThis;

  // Start with the current lambda at the top of the stack (highest index).
  unsigned CurScopeIndex = FunctionScopes.size() - 1;
  DeclContext *EnclosingDC =
      cast<sema::LambdaScopeInfo>(FunctionScopes[CurScopeIndex])->CallOperator;

  do {
    const clang::sema::LambdaScopeInfo *LSI =
        cast<sema::LambdaScopeInfo>(FunctionScopes[CurScopeIndex]);
    // IF we have climbed down to an intervening enclosing lambda that contains
    // the variable declaration - it obviously can/must not capture the
    // variable.
    // Since its enclosing DC is dependent, all the lambdas between it and the
    // innermost nested lambda are dependent (otherwise we wouldn't have
    // arrived here) - so we don't yet have a lambda that can capture the
    // variable.
    if (IsCapturingVariable &&
        VarToCapture->getDeclContext()->Equals(EnclosingDC))
      return NoLambdaIsCaptureReady;

    // For an enclosing lambda to be capture ready for an entity, all
    // intervening lambda's have to be able to capture that entity. If even
    // one of the intervening lambda's is not capable of capturing the entity
    // then no enclosing lambda can ever capture that entity.
    // For e.g.
    // const int x = 10;
    // [=](auto a) {    #1
    //   [](auto b) {   #2 <-- an intervening lambda that can never capture 'x'
    //    [=](auto c) { #3
    //       f(x, c);  <-- can not lead to x's speculative capture by #1 or #2
    //    }; }; };
    // If they do not have a default implicit capture, check to see
    // if the entity has already been explicitly captured.
    // If even a single dependent enclosing lambda lacks the capability
    // to ever capture this variable, there is no further enclosing
    // non-dependent lambda that can capture this variable.
    if (LSI->ImpCaptureStyle == sema::LambdaScopeInfo::ImpCap_None) {
      if (IsCapturingVariable && !LSI->isCaptured(VarToCapture))
        return NoLambdaIsCaptureReady;
      if (IsCapturingThis && !LSI->isCXXThisCaptured())
        return NoLambdaIsCaptureReady;
    }
    EnclosingDC = getLambdaAwareParentOfDeclContext(EnclosingDC);
    
    assert(CurScopeIndex);
    --CurScopeIndex;
  } while (!EnclosingDC->isTranslationUnit() &&
           EnclosingDC->isDependentContext() &&
           isLambdaCallOperator(EnclosingDC));

  assert(CurScopeIndex < (FunctionScopes.size() - 1));
  // If the enclosingDC is not dependent, then the immediately nested lambda
  // (one index above) is capture-ready.
  if (!EnclosingDC->isDependentContext())
    return CurScopeIndex + 1;
  return NoLambdaIsCaptureReady;
}

/// \brief Examines the FunctionScopeInfo stack to determine the nearest
/// enclosing lambda (to the current lambda) that is 'capture-capable' for 
/// the variable referenced in the current lambda (i.e. \p VarToCapture).
/// If successful, returns the index into Sema's FunctionScopeInfo stack
/// of the capture-capable lambda's LambdaScopeInfo.
///
/// Given the current stack of lambdas being processed by Sema and
/// the variable of interest, to identify the nearest enclosing lambda (to the 
/// current lambda at the top of the stack) that can truly capture
/// a variable, it has to have the following two properties:
///  a) 'capture-ready' - be the innermost lambda that is 'capture-ready':
///     - climb down the stack (i.e. starting from the innermost and examining
///       each outer lambda step by step) checking if each enclosing
///       lambda can either implicitly or explicitly capture the variable.
///       Record the first such lambda that is enclosed in a non-dependent
///       context. If no such lambda currently exists return failure.
///  b) 'capture-capable' - make sure the 'capture-ready' lambda can truly
///  capture the variable by checking all its enclosing lambdas:
///     - check if all outer lambdas enclosing the 'capture-ready' lambda
///       identified above in 'a' can also capture the variable (this is done
///       via tryCaptureVariable for variables and CheckCXXThisCapture for
///       'this' by passing in the index of the Lambda identified in step 'a')
///
/// \param FunctionScopes - Sema's stack of nested FunctionScopeInfo's (which a
/// LambdaScopeInfo inherits from).  The current/deepest/innermost lambda
/// is at the top of the stack.
///
/// \param VarToCapture - the variable to capture.  If NULL, capture 'this'.
///
///
/// \returns An Optional<unsigned> Index that if evaluates to 'true' contains
/// the index (into Sema's FunctionScopeInfo stack) of the innermost lambda
/// which is capture-capable.  If the return value evaluates to 'false' then
/// no lambda is capture-capable for \p VarToCapture.

Optional<unsigned> clang::getStackIndexOfNearestEnclosingCaptureCapableLambda(
    ArrayRef<const sema::FunctionScopeInfo *> FunctionScopes,
    VarDecl *VarToCapture, Sema &S) {

  const Optional<unsigned> NoLambdaIsCaptureCapable;
  
  const Optional<unsigned> OptionalStackIndex =
      getStackIndexOfNearestEnclosingCaptureReadyLambda(FunctionScopes,
                                                        VarToCapture);
  if (!OptionalStackIndex)
    return NoLambdaIsCaptureCapable;

  const unsigned IndexOfCaptureReadyLambda = OptionalStackIndex.getValue();
  assert(((IndexOfCaptureReadyLambda != (FunctionScopes.size() - 1)) ||
          S.getCurGenericLambda()) &&
         "The capture ready lambda for a potential capture can only be the "
         "current lambda if it is a generic lambda");

  const sema::LambdaScopeInfo *const CaptureReadyLambdaLSI =
      cast<sema::LambdaScopeInfo>(FunctionScopes[IndexOfCaptureReadyLambda]);
  
  // If VarToCapture is null, we are attempting to capture 'this'
  const bool IsCapturingThis = !VarToCapture;
  const bool IsCapturingVariable = !IsCapturingThis;

  if (IsCapturingVariable) {
    // Check if the capture-ready lambda can truly capture the variable, by
    // checking whether all enclosing lambdas of the capture-ready lambda allow
    // the capture - i.e. make sure it is capture-capable.
    QualType CaptureType, DeclRefType;
    const bool CanCaptureVariable =
        !S.tryCaptureVariable(VarToCapture,
                              /*ExprVarIsUsedInLoc*/ SourceLocation(),
                              clang::Sema::TryCapture_Implicit,
                              /*EllipsisLoc*/ SourceLocation(),
                              /*BuildAndDiagnose*/ false, CaptureType,
                              DeclRefType, &IndexOfCaptureReadyLambda);
    if (!CanCaptureVariable)
      return NoLambdaIsCaptureCapable;
  } else {
    // Check if the capture-ready lambda can truly capture 'this' by checking
    // whether all enclosing lambdas of the capture-ready lambda can capture
    // 'this'.
    const bool CanCaptureThis =
        !S.CheckCXXThisCapture(
             CaptureReadyLambdaLSI->PotentialThisCaptureLocation,
             /*Explicit*/ false, /*BuildAndDiagnose*/ false,
             &IndexOfCaptureReadyLambda);
    if (!CanCaptureThis)
      return NoLambdaIsCaptureCapable;
  } 
  return IndexOfCaptureReadyLambda;
}

static inline TemplateParameterList *
getGenericLambdaTemplateParameterList(LambdaScopeInfo *LSI, Sema &SemaRef) {
  if (LSI->GLTemplateParameterList)
    return LSI->GLTemplateParameterList;

  if (LSI->AutoTemplateParams.size()) {
    SourceRange IntroRange = LSI->IntroducerRange;
    SourceLocation LAngleLoc = IntroRange.getBegin();
    SourceLocation RAngleLoc = IntroRange.getEnd();
    LSI->GLTemplateParameterList = TemplateParameterList::Create(
        SemaRef.Context,
        /*Template kw loc*/ SourceLocation(), LAngleLoc,
        (NamedDecl **)LSI->AutoTemplateParams.data(),
        LSI->AutoTemplateParams.size(), RAngleLoc);
  }
  return LSI->GLTemplateParameterList;
}

CXXRecordDecl *Sema::createLambdaClosureType(SourceRange IntroducerRange,
                                             TypeSourceInfo *Info,
                                             bool KnownDependent, 
                                             LambdaCaptureDefault CaptureDefault) {
  DeclContext *DC = CurContext;
  while (!(DC->isFunctionOrMethod() || DC->isRecord() || DC->isFileContext()))
    DC = DC->getParent();
  bool IsGenericLambda = getGenericLambdaTemplateParameterList(getCurLambda(),
                                                               *this);  
  // Start constructing the lambda class.
  CXXRecordDecl *Class = CXXRecordDecl::CreateLambda(Context, DC, Info,
                                                     IntroducerRange.getBegin(),
                                                     KnownDependent, 
                                                     IsGenericLambda, 
                                                     CaptureDefault);
  DC->addDecl(Class);

  return Class;
}

/// \brief Determine whether the given context is or is enclosed in an inline
/// function.
static bool isInInlineFunction(const DeclContext *DC) {
  while (!DC->isFileContext()) {
    if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(DC))
      if (FD->isInlined())
        return true;
    
    DC = DC->getLexicalParent();
  }
  
  return false;
}

MangleNumberingContext *
Sema::getCurrentMangleNumberContext(const DeclContext *DC,
                                    Decl *&ManglingContextDecl) {
  // Compute the context for allocating mangling numbers in the current
  // expression, if the ABI requires them.
  ManglingContextDecl = ExprEvalContexts.back().ManglingContextDecl;

  enum ContextKind {
    Normal,
    DefaultArgument,
    DataMember,
    StaticDataMember
  } Kind = Normal;

  // Default arguments of member function parameters that appear in a class
  // definition, as well as the initializers of data members, receive special
  // treatment. Identify them.
  if (ManglingContextDecl) {
    if (ParmVarDecl *Param = dyn_cast<ParmVarDecl>(ManglingContextDecl)) {
      if (const DeclContext *LexicalDC
          = Param->getDeclContext()->getLexicalParent())
        if (LexicalDC->isRecord())
          Kind = DefaultArgument;
    } else if (VarDecl *Var = dyn_cast<VarDecl>(ManglingContextDecl)) {
      if (Var->getDeclContext()->isRecord())
        Kind = StaticDataMember;
    } else if (isa<FieldDecl>(ManglingContextDecl)) {
      Kind = DataMember;
    }
  }

  // Itanium ABI [5.1.7]:
  //   In the following contexts [...] the one-definition rule requires closure
  //   types in different translation units to "correspond":
  bool IsInNonspecializedTemplate =
    !ActiveTemplateInstantiations.empty() || CurContext->isDependentContext();
  switch (Kind) {
  case Normal:
    //  -- the bodies of non-exported nonspecialized template functions
    //  -- the bodies of inline functions
    if ((IsInNonspecializedTemplate &&
         !(ManglingContextDecl && isa<ParmVarDecl>(ManglingContextDecl))) ||
        isInInlineFunction(CurContext)) {
      ManglingContextDecl = nullptr;
      return &Context.getManglingNumberContext(DC);
    }

    ManglingContextDecl = nullptr;
    return nullptr;

  case StaticDataMember:
    //  -- the initializers of nonspecialized static members of template classes
    if (!IsInNonspecializedTemplate) {
      ManglingContextDecl = nullptr;
      return nullptr;
    }
    // Fall through to get the current context.

  case DataMember:
    //  -- the in-class initializers of class members
  case DefaultArgument:
    //  -- default arguments appearing in class definitions
    return &ExprEvalContexts.back().getMangleNumberingContext(Context);
  }

  llvm_unreachable("unexpected context");
}

MangleNumberingContext &
Sema::ExpressionEvaluationContextRecord::getMangleNumberingContext(
    ASTContext &Ctx) {
  assert(ManglingContextDecl && "Need to have a context declaration");
  if (!MangleNumbering)
    MangleNumbering = Ctx.createMangleNumberingContext();
  return *MangleNumbering;
}

CXXMethodDecl *Sema::startLambdaDefinition(CXXRecordDecl *Class,
                                           SourceRange IntroducerRange,
                                           TypeSourceInfo *MethodTypeInfo,
                                           SourceLocation EndLoc,
                                           ArrayRef<ParmVarDecl *> Params) {
  QualType MethodType = MethodTypeInfo->getType();
  TemplateParameterList *TemplateParams = 
            getGenericLambdaTemplateParameterList(getCurLambda(), *this);
  // If a lambda appears in a dependent context or is a generic lambda (has
  // template parameters) and has an 'auto' return type, deduce it to a 
  // dependent type.
  if (Class->isDependentContext() || TemplateParams) {
    const FunctionProtoType *FPT = MethodType->castAs<FunctionProtoType>();
    QualType Result = FPT->getReturnType();
    if (Result->isUndeducedType()) {
      Result = SubstAutoType(Result, Context.DependentTy);
      MethodType = Context.getFunctionType(Result, FPT->getParamTypes(),
                                           FPT->getExtProtoInfo());
    }
  }

  // C++11 [expr.prim.lambda]p5:
  //   The closure type for a lambda-expression has a public inline function 
  //   call operator (13.5.4) whose parameters and return type are described by
  //   the lambda-expression's parameter-declaration-clause and 
  //   trailing-return-type respectively.
  DeclarationName MethodName
    = Context.DeclarationNames.getCXXOperatorName(OO_Call);
  DeclarationNameLoc MethodNameLoc;
  MethodNameLoc.CXXOperatorName.BeginOpNameLoc
    = IntroducerRange.getBegin().getRawEncoding();
  MethodNameLoc.CXXOperatorName.EndOpNameLoc
    = IntroducerRange.getEnd().getRawEncoding();
  CXXMethodDecl *Method
    = CXXMethodDecl::Create(Context, Class, EndLoc,
                            DeclarationNameInfo(MethodName, 
                                                IntroducerRange.getBegin(),
                                                MethodNameLoc),
                            MethodType, MethodTypeInfo,
                            SC_None,
                            /*isInline=*/true,
                            /*isConstExpr=*/false,
                            EndLoc);
  Method->setAccess(AS_public);
  
  // Temporarily set the lexical declaration context to the current
  // context, so that the Scope stack matches the lexical nesting.
  Method->setLexicalDeclContext(CurContext);  
  // Create a function template if we have a template parameter list
  FunctionTemplateDecl *const TemplateMethod = TemplateParams ?
            FunctionTemplateDecl::Create(Context, Class,
                                         Method->getLocation(), MethodName, 
                                         TemplateParams,
                                         Method) : nullptr;
  if (TemplateMethod) {
    TemplateMethod->setLexicalDeclContext(CurContext);
    TemplateMethod->setAccess(AS_public);
    Method->setDescribedFunctionTemplate(TemplateMethod);
  }
  
  // Add parameters.
  if (!Params.empty()) {
    Method->setParams(Params);
    CheckParmsForFunctionDef(const_cast<ParmVarDecl **>(Params.begin()),
                             const_cast<ParmVarDecl **>(Params.end()),
                             /*CheckParameterNames=*/false);
    
    for (auto P : Method->params())
      P->setOwningFunction(Method);
  }

  Decl *ManglingContextDecl;
  if (MangleNumberingContext *MCtx =
          getCurrentMangleNumberContext(Class->getDeclContext(),
                                        ManglingContextDecl)) {
    unsigned ManglingNumber = MCtx->getManglingNumber(Method);
    Class->setLambdaMangling(ManglingNumber, ManglingContextDecl);
  }

  return Method;
}

void Sema::buildLambdaScope(LambdaScopeInfo *LSI,
                                        CXXMethodDecl *CallOperator,
                                        SourceRange IntroducerRange,
                                        LambdaCaptureDefault CaptureDefault,
                                        SourceLocation CaptureDefaultLoc,
                                        bool ExplicitParams,
                                        bool ExplicitResultType,
                                        bool Mutable) {
  LSI->CallOperator = CallOperator;
  CXXRecordDecl *LambdaClass = CallOperator->getParent();
  LSI->Lambda = LambdaClass;
  if (CaptureDefault == LCD_ByCopy)
    LSI->ImpCaptureStyle = LambdaScopeInfo::ImpCap_LambdaByval;
  else if (CaptureDefault == LCD_ByRef)
    LSI->ImpCaptureStyle = LambdaScopeInfo::ImpCap_LambdaByref;
  LSI->CaptureDefaultLoc = CaptureDefaultLoc;
  LSI->IntroducerRange = IntroducerRange;
  LSI->ExplicitParams = ExplicitParams;
  LSI->Mutable = Mutable;

  if (ExplicitResultType) {
    LSI->ReturnType = CallOperator->getReturnType();

    if (!LSI->ReturnType->isDependentType() &&
        !LSI->ReturnType->isVoidType()) {
      if (RequireCompleteType(CallOperator->getLocStart(), LSI->ReturnType,
                              diag::err_lambda_incomplete_result)) {
        // Do nothing.
      }
    }
  } else {
    LSI->HasImplicitReturnType = true;
  }
}

void Sema::finishLambdaExplicitCaptures(LambdaScopeInfo *LSI) {
  LSI->finishedExplicitCaptures();
}

void Sema::addLambdaParameters(CXXMethodDecl *CallOperator, Scope *CurScope) {  
  // Introduce our parameters into the function scope
  for (unsigned p = 0, NumParams = CallOperator->getNumParams(); 
       p < NumParams; ++p) {
    ParmVarDecl *Param = CallOperator->getParamDecl(p);
    
    // If this has an identifier, add it to the scope stack.
    if (CurScope && Param->getIdentifier()) {
      CheckShadow(CurScope, Param);
      
      PushOnScopeChains(Param, CurScope);
    }
  }
}

/// If this expression is an enumerator-like expression of some type
/// T, return the type T; otherwise, return null.
///
/// Pointer comparisons on the result here should always work because
/// it's derived from either the parent of an EnumConstantDecl
/// (i.e. the definition) or the declaration returned by
/// EnumType::getDecl() (i.e. the definition).
static EnumDecl *findEnumForBlockReturn(Expr *E) {
  // An expression is an enumerator-like expression of type T if,
  // ignoring parens and parens-like expressions:
  E = E->IgnoreParens();

  //  - it is an enumerator whose enum type is T or
  if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(E)) {
    if (EnumConstantDecl *D
          = dyn_cast<EnumConstantDecl>(DRE->getDecl())) {
      return cast<EnumDecl>(D->getDeclContext());
    }
    return nullptr;
  }

  //  - it is a comma expression whose RHS is an enumerator-like
  //    expression of type T or
  if (BinaryOperator *BO = dyn_cast<BinaryOperator>(E)) {
    if (BO->getOpcode() == BO_Comma)
      return findEnumForBlockReturn(BO->getRHS());
    return nullptr;
  }

  //  - it is a statement-expression whose value expression is an
  //    enumerator-like expression of type T or
  if (StmtExpr *SE = dyn_cast<StmtExpr>(E)) {
    if (Expr *last = dyn_cast_or_null<Expr>(SE->getSubStmt()->body_back()))
      return findEnumForBlockReturn(last);
    return nullptr;
  }

  //   - it is a ternary conditional operator (not the GNU ?:
  //     extension) whose second and third operands are
  //     enumerator-like expressions of type T or
  if (ConditionalOperator *CO = dyn_cast<ConditionalOperator>(E)) {
    if (EnumDecl *ED = findEnumForBlockReturn(CO->getTrueExpr()))
      if (ED == findEnumForBlockReturn(CO->getFalseExpr()))
        return ED;
    return nullptr;
  }

  // (implicitly:)
  //   - it is an implicit integral conversion applied to an
  //     enumerator-like expression of type T or
  if (ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(E)) {
    // We can sometimes see integral conversions in valid
    // enumerator-like expressions.
    if (ICE->getCastKind() == CK_IntegralCast)
      return findEnumForBlockReturn(ICE->getSubExpr());

    // Otherwise, just rely on the type.
  }

  //   - it is an expression of that formal enum type.
  if (const EnumType *ET = E->getType()->getAs<EnumType>()) {
    return ET->getDecl();
  }

  // Otherwise, nope.
  return nullptr;
}

/// Attempt to find a type T for which the returned expression of the
/// given statement is an enumerator-like expression of that type.
static EnumDecl *findEnumForBlockReturn(ReturnStmt *ret) {
  if (Expr *retValue = ret->getRetValue())
    return findEnumForBlockReturn(retValue);
  return nullptr;
}

/// Attempt to find a common type T for which all of the returned
/// expressions in a block are enumerator-like expressions of that
/// type.
static EnumDecl *findCommonEnumForBlockReturns(ArrayRef<ReturnStmt*> returns) {
  ArrayRef<ReturnStmt*>::iterator i = returns.begin(), e = returns.end();

  // Try to find one for the first return.
  EnumDecl *ED = findEnumForBlockReturn(*i);
  if (!ED) return nullptr;

  // Check that the rest of the returns have the same enum.
  for (++i; i != e; ++i) {
    if (findEnumForBlockReturn(*i) != ED)
      return nullptr;
  }

  // Never infer an anonymous enum type.
  if (!ED->hasNameForLinkage()) return nullptr;

  return ED;
}

/// Adjust the given return statements so that they formally return
/// the given type.  It should require, at most, an IntegralCast.
static void adjustBlockReturnsToEnum(Sema &S, ArrayRef<ReturnStmt*> returns,
                                     QualType returnType) {
  for (ArrayRef<ReturnStmt*>::iterator
         i = returns.begin(), e = returns.end(); i != e; ++i) {
    ReturnStmt *ret = *i;
    Expr *retValue = ret->getRetValue();
    if (S.Context.hasSameType(retValue->getType(), returnType))
      continue;

    // Right now we only support integral fixup casts.
    assert(returnType->isIntegralOrUnscopedEnumerationType());
    assert(retValue->getType()->isIntegralOrUnscopedEnumerationType());

    ExprWithCleanups *cleanups = dyn_cast<ExprWithCleanups>(retValue);

    Expr *E = (cleanups ? cleanups->getSubExpr() : retValue);
    E = ImplicitCastExpr::Create(S.Context, returnType, CK_IntegralCast,
                                 E, /*base path*/ nullptr, VK_RValue);
    if (cleanups) {
      cleanups->setSubExpr(E);
    } else {
      ret->setRetValue(E);
    }
  }
}

void Sema::deduceClosureReturnType(CapturingScopeInfo &CSI) {
  assert(CSI.HasImplicitReturnType);
  // If it was ever a placeholder, it had to been deduced to DependentTy.
  assert(CSI.ReturnType.isNull() || !CSI.ReturnType->isUndeducedType()); 

  // C++ core issue 975:
  //   If a lambda-expression does not include a trailing-return-type,
  //   it is as if the trailing-return-type denotes the following type:
  //     - if there are no return statements in the compound-statement,
  //       or all return statements return either an expression of type
  //       void or no expression or braced-init-list, the type void;
  //     - otherwise, if all return statements return an expression
  //       and the types of the returned expressions after
  //       lvalue-to-rvalue conversion (4.1 [conv.lval]),
  //       array-to-pointer conversion (4.2 [conv.array]), and
  //       function-to-pointer conversion (4.3 [conv.func]) are the
  //       same, that common type;
  //     - otherwise, the program is ill-formed.
  //
  // C++ core issue 1048 additionally removes top-level cv-qualifiers
  // from the types of returned expressions to match the C++14 auto
  // deduction rules.
  //
  // In addition, in blocks in non-C++ modes, if all of the return
  // statements are enumerator-like expressions of some type T, where
  // T has a name for linkage, then we infer the return type of the
  // block to be that type.

  // First case: no return statements, implicit void return type.
  ASTContext &Ctx = getASTContext();
  if (CSI.Returns.empty()) {
    // It's possible there were simply no /valid/ return statements.
    // In this case, the first one we found may have at least given us a type.
    if (CSI.ReturnType.isNull())
      CSI.ReturnType = Ctx.VoidTy;
    return;
  }

  // Second case: at least one return statement has dependent type.
  // Delay type checking until instantiation.
  assert(!CSI.ReturnType.isNull() && "We should have a tentative return type.");
  if (CSI.ReturnType->isDependentType())
    return;

  // Try to apply the enum-fuzz rule.
  if (!getLangOpts().CPlusPlus) {
    assert(isa<BlockScopeInfo>(CSI));
    const EnumDecl *ED = findCommonEnumForBlockReturns(CSI.Returns);
    if (ED) {
      CSI.ReturnType = Context.getTypeDeclType(ED);
      adjustBlockReturnsToEnum(*this, CSI.Returns, CSI.ReturnType);
      return;
    }
  }

  // Third case: only one return statement. Don't bother doing extra work!
  SmallVectorImpl<ReturnStmt*>::iterator I = CSI.Returns.begin(),
                                         E = CSI.Returns.end();
  if (I+1 == E)
    return;

  // General case: many return statements.
  // Check that they all have compatible return types.

  // We require the return types to strictly match here.
  // Note that we've already done the required promotions as part of
  // processing the return statement.
  for (; I != E; ++I) {
    const ReturnStmt *RS = *I;
    const Expr *RetE = RS->getRetValue();

    QualType ReturnType =
        (RetE ? RetE->getType() : Context.VoidTy).getUnqualifiedType();
    if (Context.hasSameType(ReturnType, CSI.ReturnType))
      continue;

    // FIXME: This is a poor diagnostic for ReturnStmts without expressions.
    // TODO: It's possible that the *first* return is the divergent one.
    Diag(RS->getLocStart(),
         diag::err_typecheck_missing_return_type_incompatible)
      << ReturnType << CSI.ReturnType
      << isa<LambdaScopeInfo>(CSI);
    // Continue iterating so that we keep emitting diagnostics.
  }
}

QualType Sema::performLambdaInitCaptureInitialization(SourceLocation Loc,
                                                      bool ByRef,
                                                      IdentifierInfo *Id,
                                                      Expr *&Init) {

  // We do not need to distinguish between direct-list-initialization
  // and copy-list-initialization here, because we will always deduce
  // std::initializer_list<T>, and direct- and copy-list-initialization
  // always behave the same for such a type.
  // FIXME: We should model whether an '=' was present.
  const bool IsDirectInit = isa<ParenListExpr>(Init) || isa<InitListExpr>(Init);

  // Create an 'auto' or 'auto&' TypeSourceInfo that we can use to
  // deduce against.
  QualType DeductType = Context.getAutoDeductType();
  TypeLocBuilder TLB;
  TLB.pushTypeSpec(DeductType).setNameLoc(Loc);
  if (ByRef) {
    DeductType = BuildReferenceType(DeductType, true, Loc, Id);
    assert(!DeductType.isNull() && "can't build reference to auto");
    TLB.push<ReferenceTypeLoc>(DeductType).setSigilLoc(Loc);
  }
  TypeSourceInfo *TSI = TLB.getTypeSourceInfo(Context, DeductType);

  // Are we a non-list direct initialization?
  ParenListExpr *CXXDirectInit = dyn_cast<ParenListExpr>(Init);

  Expr *DeduceInit = Init;
  // Initializer could be a C++ direct-initializer. Deduction only works if it
  // contains exactly one expression.
  if (CXXDirectInit) {
    if (CXXDirectInit->getNumExprs() == 0) {
      Diag(CXXDirectInit->getLocStart(), diag::err_init_capture_no_expression)
          << DeclarationName(Id) << TSI->getType() << Loc;
      return QualType();
    } else if (CXXDirectInit->getNumExprs() > 1) {
      Diag(CXXDirectInit->getExpr(1)->getLocStart(),
           diag::err_init_capture_multiple_expressions)
          << DeclarationName(Id) << TSI->getType() << Loc;
      return QualType();
    } else {
      DeduceInit = CXXDirectInit->getExpr(0);
      if (isa<InitListExpr>(DeduceInit))
        Diag(CXXDirectInit->getLocStart(), diag::err_init_capture_paren_braces)
          << DeclarationName(Id) << Loc;
    }
  }

  // Now deduce against the initialization expression and store the deduced
  // type below.
  QualType DeducedType;
  if (DeduceAutoType(TSI, DeduceInit, DeducedType) == DAR_Failed) {
    if (isa<InitListExpr>(Init))
      Diag(Loc, diag::err_init_capture_deduction_failure_from_init_list)
          << DeclarationName(Id)
          << (DeduceInit->getType().isNull() ? TSI->getType()
                                             : DeduceInit->getType())
          << DeduceInit->getSourceRange();
    else
      Diag(Loc, diag::err_init_capture_deduction_failure)
          << DeclarationName(Id) << TSI->getType()
          << (DeduceInit->getType().isNull() ? TSI->getType()
                                             : DeduceInit->getType())
          << DeduceInit->getSourceRange();
  }
  if (DeducedType.isNull())
    return QualType();

  // Perform initialization analysis and ensure any implicit conversions
  // (such as lvalue-to-rvalue) are enforced.
  InitializedEntity Entity =
      InitializedEntity::InitializeLambdaCapture(Id, DeducedType, Loc);
  InitializationKind Kind =
      IsDirectInit
          ? (CXXDirectInit ? InitializationKind::CreateDirect(
                                 Loc, Init->getLocStart(), Init->getLocEnd())
                           : InitializationKind::CreateDirectList(Loc))
          : InitializationKind::CreateCopy(Loc, Init->getLocStart());

  MultiExprArg Args = Init;
  if (CXXDirectInit)
    Args =
        MultiExprArg(CXXDirectInit->getExprs(), CXXDirectInit->getNumExprs());
  QualType DclT;
  InitializationSequence InitSeq(*this, Entity, Kind, Args);
  ExprResult Result = InitSeq.Perform(*this, Entity, Kind, Args, &DclT);

  if (Result.isInvalid())
    return QualType();
  Init = Result.getAs<Expr>();

  // The init-capture initialization is a full-expression that must be
  // processed as one before we enter the declcontext of the lambda's
  // call-operator.
  Result = ActOnFinishFullExpr(Init, Loc, /*DiscardedValue*/ false,
                               /*IsConstexpr*/ false,
                               /*IsLambdaInitCaptureInitalizer*/ true);
  if (Result.isInvalid())
    return QualType();

  Init = Result.getAs<Expr>();
  return DeducedType;
}

VarDecl *Sema::createLambdaInitCaptureVarDecl(SourceLocation Loc, 
    QualType InitCaptureType, IdentifierInfo *Id, Expr *Init) {

  TypeSourceInfo *TSI = Context.getTrivialTypeSourceInfo(InitCaptureType,
      Loc);
  // Create a dummy variable representing the init-capture. This is not actually
  // used as a variable, and only exists as a way to name and refer to the
  // init-capture.
  // FIXME: Pass in separate source locations for '&' and identifier.
  VarDecl *NewVD = VarDecl::Create(Context, CurContext, Loc,
                                   Loc, Id, InitCaptureType, TSI, SC_Auto);
  NewVD->setInitCapture(true);
  NewVD->setReferenced(true);
  NewVD->markUsed(Context);
  NewVD->setInit(Init);
  return NewVD;
}

FieldDecl *Sema::buildInitCaptureField(LambdaScopeInfo *LSI, VarDecl *Var) {
  FieldDecl *Field = FieldDecl::Create(
      Context, LSI->Lambda, Var->getLocation(), Var->getLocation(),
      nullptr, Var->getType(), Var->getTypeSourceInfo(), nullptr, false,
      ICIS_NoInit);
  Field->setImplicit(true);
  Field->setAccess(AS_private);
  LSI->Lambda->addDecl(Field);

  LSI->addCapture(Var, /*isBlock*/false, Var->getType()->isReferenceType(),
                  /*isNested*/false, Var->getLocation(), SourceLocation(),
                  Var->getType(), Var->getInit());
  return Field;
}

void Sema::ActOnStartOfLambdaDefinition(LambdaIntroducer &Intro,
                                        Declarator &ParamInfo,
                                        Scope *CurScope) {
  // Determine if we're within a context where we know that the lambda will
  // be dependent, because there are template parameters in scope.
  bool KnownDependent = false;
  LambdaScopeInfo *const LSI = getCurLambda();
  assert(LSI && "LambdaScopeInfo should be on stack!");
  TemplateParameterList *TemplateParams = 
            getGenericLambdaTemplateParameterList(LSI, *this);

  if (Scope *TmplScope = CurScope->getTemplateParamParent()) {
    // Since we have our own TemplateParams, so check if an outer scope
    // has template params, only then are we in a dependent scope.
    if (TemplateParams)  {
      TmplScope = TmplScope->getParent();
      TmplScope = TmplScope ? TmplScope->getTemplateParamParent() : nullptr;
    }
    if (TmplScope && !TmplScope->decl_empty())
      KnownDependent = true;
  }
  // Determine the signature of the call operator.
  TypeSourceInfo *MethodTyInfo;
  bool ExplicitParams = true;
  bool ExplicitResultType = true;
  bool ContainsUnexpandedParameterPack = false;
  SourceLocation EndLoc;
  SmallVector<ParmVarDecl *, 8> Params;
  if (ParamInfo.getNumTypeObjects() == 0) {
    // C++11 [expr.prim.lambda]p4:
    //   If a lambda-expression does not include a lambda-declarator, it is as 
    //   if the lambda-declarator were ().
    FunctionProtoType::ExtProtoInfo EPI(Context.getDefaultCallingConvention(
        /*IsVariadic=*/false, /*IsCXXMethod=*/true));
    EPI.HasTrailingReturn = true;
    EPI.TypeQuals |= DeclSpec::TQ_const;
    // C++1y [expr.prim.lambda]:
    //   The lambda return type is 'auto', which is replaced by the
    //   trailing-return type if provided and/or deduced from 'return'
    //   statements
    // We don't do this before C++1y, because we don't support deduced return
    // types there.
    QualType DefaultTypeForNoTrailingReturn =
        getLangOpts().CPlusPlus14 ? Context.getAutoDeductType()
                                  : Context.DependentTy;
    QualType MethodTy =
        Context.getFunctionType(DefaultTypeForNoTrailingReturn, None, EPI);
    MethodTyInfo = Context.getTrivialTypeSourceInfo(MethodTy);
    ExplicitParams = false;
    ExplicitResultType = false;
    EndLoc = Intro.Range.getEnd();
  } else {
    assert(ParamInfo.isFunctionDeclarator() &&
           "lambda-declarator is a function");
    DeclaratorChunk::FunctionTypeInfo &FTI = ParamInfo.getFunctionTypeInfo();

    // C++11 [expr.prim.lambda]p5:
    //   This function call operator is declared const (9.3.1) if and only if 
    //   the lambda-expression's parameter-declaration-clause is not followed 
    //   by mutable. It is neither virtual nor declared volatile. [...]
    if (!FTI.hasMutableQualifier())
      FTI.TypeQuals |= DeclSpec::TQ_const;

    MethodTyInfo = GetTypeForDeclarator(ParamInfo, CurScope);
    assert(MethodTyInfo && "no type from lambda-declarator");
    EndLoc = ParamInfo.getSourceRange().getEnd();

    ExplicitResultType = FTI.hasTrailingReturnType();

    if (FTIHasNonVoidParameters(FTI)) {
      Params.reserve(FTI.NumParams);
      for (unsigned i = 0, e = FTI.NumParams; i != e; ++i)
        Params.push_back(cast<ParmVarDecl>(FTI.Params[i].Param));
    }

    // Check for unexpanded parameter packs in the method type.
    if (MethodTyInfo->getType()->containsUnexpandedParameterPack())
      ContainsUnexpandedParameterPack = true;
  }

  CXXRecordDecl *Class = createLambdaClosureType(Intro.Range, MethodTyInfo,
                                                 KnownDependent, Intro.Default);

  CXXMethodDecl *Method = startLambdaDefinition(Class, Intro.Range,
                                                MethodTyInfo, EndLoc, Params);
  if (ExplicitParams)
    CheckCXXDefaultArguments(Method);
  
  // Attributes on the lambda apply to the method.  
  ProcessDeclAttributes(CurScope, Method, ParamInfo);
  
  // Introduce the function call operator as the current declaration context.
  PushDeclContext(CurScope, Method);
    
  // Build the lambda scope.
  buildLambdaScope(LSI, Method, Intro.Range, Intro.Default, Intro.DefaultLoc,
                   ExplicitParams, ExplicitResultType, !Method->isConst());

  // C++11 [expr.prim.lambda]p9:
  //   A lambda-expression whose smallest enclosing scope is a block scope is a
  //   local lambda expression; any other lambda expression shall not have a
  //   capture-default or simple-capture in its lambda-introducer.
  //
  // For simple-captures, this is covered by the check below that any named
  // entity is a variable that can be captured.
  //
  // For DR1632, we also allow a capture-default in any context where we can
  // odr-use 'this' (in particular, in a default initializer for a non-static
  // data member).
  if (Intro.Default != LCD_None && !Class->getParent()->isFunctionOrMethod() &&
      (getCurrentThisType().isNull() ||
       CheckCXXThisCapture(SourceLocation(), /*Explicit*/true,
                           /*BuildAndDiagnose*/false)))
    Diag(Intro.DefaultLoc, diag::err_capture_default_non_local);

  // Distinct capture names, for diagnostics.
  llvm::SmallSet<IdentifierInfo*, 8> CaptureNames;

  // Handle explicit captures.
  SourceLocation PrevCaptureLoc
    = Intro.Default == LCD_None? Intro.Range.getBegin() : Intro.DefaultLoc;
  for (auto C = Intro.Captures.begin(), E = Intro.Captures.end(); C != E;
       PrevCaptureLoc = C->Loc, ++C) {
    if (C->Kind == LCK_This) {
      // C++11 [expr.prim.lambda]p8:
      //   An identifier or this shall not appear more than once in a 
      //   lambda-capture.
      if (LSI->isCXXThisCaptured()) {
        Diag(C->Loc, diag::err_capture_more_than_once)
            << "'this'" << SourceRange(LSI->getCXXThisCapture().getLocation())
            << FixItHint::CreateRemoval(
                   SourceRange(getLocForEndOfToken(PrevCaptureLoc), C->Loc));
        continue;
      }

      // C++11 [expr.prim.lambda]p8:
      //   If a lambda-capture includes a capture-default that is =, the 
      //   lambda-capture shall not contain this [...].
      if (Intro.Default == LCD_ByCopy) {
        Diag(C->Loc, diag::err_this_capture_with_copy_default)
            << FixItHint::CreateRemoval(
                SourceRange(getLocForEndOfToken(PrevCaptureLoc), C->Loc));
        continue;
      }

      // C++11 [expr.prim.lambda]p12:
      //   If this is captured by a local lambda expression, its nearest
      //   enclosing function shall be a non-static member function.
      QualType ThisCaptureType = getCurrentThisType();
      if (ThisCaptureType.isNull()) {
        Diag(C->Loc, diag::err_this_capture) << true;
        continue;
      }
      
      CheckCXXThisCapture(C->Loc, /*Explicit=*/true);
      continue;
    }

    assert(C->Id && "missing identifier for capture");

    if (C->Init.isInvalid())
      continue;

    VarDecl *Var = nullptr;
    if (C->Init.isUsable()) {
      Diag(C->Loc, getLangOpts().CPlusPlus14
                       ? diag::warn_cxx11_compat_init_capture
                       : diag::ext_init_capture);

      if (C->Init.get()->containsUnexpandedParameterPack())
        ContainsUnexpandedParameterPack = true;
      // If the initializer expression is usable, but the InitCaptureType
      // is not, then an error has occurred - so ignore the capture for now.
      // for e.g., [n{0}] { }; <-- if no <initializer_list> is included.
      // FIXME: we should create the init capture variable and mark it invalid 
      // in this case.
      if (C->InitCaptureType.get().isNull()) 
        continue;
      Var = createLambdaInitCaptureVarDecl(C->Loc, C->InitCaptureType.get(), 
            C->Id, C->Init.get());
      // C++1y [expr.prim.lambda]p11:
      //   An init-capture behaves as if it declares and explicitly
      //   captures a variable [...] whose declarative region is the
      //   lambda-expression's compound-statement
      if (Var)
        PushOnScopeChains(Var, CurScope, false);
    } else {
      // C++11 [expr.prim.lambda]p8:
      //   If a lambda-capture includes a capture-default that is &, the 
      //   identifiers in the lambda-capture shall not be preceded by &.
      //   If a lambda-capture includes a capture-default that is =, [...]
      //   each identifier it contains shall be preceded by &.
      if (C->Kind == LCK_ByRef && Intro.Default == LCD_ByRef) {
        Diag(C->Loc, diag::err_reference_capture_with_reference_default)
            << FixItHint::CreateRemoval(
                SourceRange(getLocForEndOfToken(PrevCaptureLoc), C->Loc));
        continue;
      } else if (C->Kind == LCK_ByCopy && Intro.Default == LCD_ByCopy) {
        Diag(C->Loc, diag::err_copy_capture_with_copy_default)
            << FixItHint::CreateRemoval(
                SourceRange(getLocForEndOfToken(PrevCaptureLoc), C->Loc));
        continue;
      }

      // C++11 [expr.prim.lambda]p10:
      //   The identifiers in a capture-list are looked up using the usual
      //   rules for unqualified name lookup (3.4.1)
      DeclarationNameInfo Name(C->Id, C->Loc);
      LookupResult R(*this, Name, LookupOrdinaryName);
      LookupName(R, CurScope);
      if (R.isAmbiguous())
        continue;
      if (R.empty()) {
        // FIXME: Disable corrections that would add qualification?
        CXXScopeSpec ScopeSpec;
        if (DiagnoseEmptyLookup(CurScope, ScopeSpec, R,
                                llvm::make_unique<DeclFilterCCC<VarDecl>>()))
          continue;
      }

      Var = R.getAsSingle<VarDecl>();
      if (Var && DiagnoseUseOfDecl(Var, C->Loc))
        continue;
    }

    // C++11 [expr.prim.lambda]p8:
    //   An identifier or this shall not appear more than once in a
    //   lambda-capture.
    if (!CaptureNames.insert(C->Id).second) {
      if (Var && LSI->isCaptured(Var)) {
        Diag(C->Loc, diag::err_capture_more_than_once)
            << C->Id << SourceRange(LSI->getCapture(Var).getLocation())
            << FixItHint::CreateRemoval(
                   SourceRange(getLocForEndOfToken(PrevCaptureLoc), C->Loc));
      } else
        // Previous capture captured something different (one or both was
        // an init-cpature): no fixit.
        Diag(C->Loc, diag::err_capture_more_than_once) << C->Id;
      continue;
    }

    // C++11 [expr.prim.lambda]p10:
    //   [...] each such lookup shall find a variable with automatic storage
    //   duration declared in the reaching scope of the local lambda expression.
    // Note that the 'reaching scope' check happens in tryCaptureVariable().
    if (!Var) {
      Diag(C->Loc, diag::err_capture_does_not_name_variable) << C->Id;
      continue;
    }

    // Ignore invalid decls; they'll just confuse the code later.
    if (Var->isInvalidDecl())
      continue;

    if (!Var->hasLocalStorage()) {
      Diag(C->Loc, diag::err_capture_non_automatic_variable) << C->Id;
      Diag(Var->getLocation(), diag::note_previous_decl) << C->Id;
      continue;
    }

    // C++11 [expr.prim.lambda]p23:
    //   A capture followed by an ellipsis is a pack expansion (14.5.3).
    SourceLocation EllipsisLoc;
    if (C->EllipsisLoc.isValid()) {
      if (Var->isParameterPack()) {
        EllipsisLoc = C->EllipsisLoc;
      } else {
        Diag(C->EllipsisLoc, diag::err_pack_expansion_without_parameter_packs)
          << SourceRange(C->Loc);
        
        // Just ignore the ellipsis.
      }
    } else if (Var->isParameterPack()) {
      ContainsUnexpandedParameterPack = true;
    }

    if (C->Init.isUsable()) {
      buildInitCaptureField(LSI, Var);
    } else {
      TryCaptureKind Kind = C->Kind == LCK_ByRef ? TryCapture_ExplicitByRef :
                                                   TryCapture_ExplicitByVal;
      tryCaptureVariable(Var, C->Loc, Kind, EllipsisLoc);
    }
  }
  finishLambdaExplicitCaptures(LSI);

  LSI->ContainsUnexpandedParameterPack = ContainsUnexpandedParameterPack;

  // Add lambda parameters into scope.
  addLambdaParameters(Method, CurScope);

  // Enter a new evaluation context to insulate the lambda from any
  // cleanups from the enclosing full-expression.
  PushExpressionEvaluationContext(PotentiallyEvaluated);  
}

void Sema::ActOnLambdaError(SourceLocation StartLoc, Scope *CurScope,
                            bool IsInstantiation) {
  LambdaScopeInfo *LSI = cast<LambdaScopeInfo>(FunctionScopes.back());

  // Leave the expression-evaluation context.
  DiscardCleanupsInEvaluationContext();
  PopExpressionEvaluationContext();

  // Leave the context of the lambda.
  if (!IsInstantiation)
    PopDeclContext();

  // Finalize the lambda.
  CXXRecordDecl *Class = LSI->Lambda;
  Class->setInvalidDecl();
  SmallVector<Decl*, 4> Fields(Class->fields());
  ActOnFields(nullptr, Class->getLocation(), Class, Fields, SourceLocation(),
              SourceLocation(), nullptr);
  CheckCompletedCXXClass(Class);

  PopFunctionScopeInfo();
}

/// \brief Add a lambda's conversion to function pointer, as described in
/// C++11 [expr.prim.lambda]p6.
static void addFunctionPointerConversion(Sema &S, 
                                         SourceRange IntroducerRange,
                                         CXXRecordDecl *Class,
                                         CXXMethodDecl *CallOperator) {
  // Add the conversion to function pointer.
  const FunctionProtoType *CallOpProto = 
      CallOperator->getType()->getAs<FunctionProtoType>();
  const FunctionProtoType::ExtProtoInfo CallOpExtInfo = 
      CallOpProto->getExtProtoInfo();   
  QualType PtrToFunctionTy;
  QualType InvokerFunctionTy;
  {
    FunctionProtoType::ExtProtoInfo InvokerExtInfo = CallOpExtInfo;
    CallingConv CC = S.Context.getDefaultCallingConvention(
        CallOpProto->isVariadic(), /*IsCXXMethod=*/false);
    InvokerExtInfo.ExtInfo = InvokerExtInfo.ExtInfo.withCallingConv(CC);
    InvokerExtInfo.TypeQuals = 0;
    assert(InvokerExtInfo.RefQualifier == RQ_None && 
        "Lambda's call operator should not have a reference qualifier");
    InvokerFunctionTy =
        S.Context.getFunctionType(CallOpProto->getReturnType(),
                                  CallOpProto->getParamTypes(), InvokerExtInfo);
    PtrToFunctionTy = S.Context.getPointerType(InvokerFunctionTy);
  }

  // Create the type of the conversion function.
  FunctionProtoType::ExtProtoInfo ConvExtInfo(
      S.Context.getDefaultCallingConvention(
      /*IsVariadic=*/false, /*IsCXXMethod=*/true));
  // The conversion function is always const.
  ConvExtInfo.TypeQuals = Qualifiers::Const;
  QualType ConvTy = 
      S.Context.getFunctionType(PtrToFunctionTy, None, ConvExtInfo);

  SourceLocation Loc = IntroducerRange.getBegin();
  DeclarationName ConversionName
    = S.Context.DeclarationNames.getCXXConversionFunctionName(
        S.Context.getCanonicalType(PtrToFunctionTy));
  DeclarationNameLoc ConvNameLoc;
  // Construct a TypeSourceInfo for the conversion function, and wire
  // all the parameters appropriately for the FunctionProtoTypeLoc 
  // so that everything works during transformation/instantiation of 
  // generic lambdas.
  // The main reason for wiring up the parameters of the conversion
  // function with that of the call operator is so that constructs
  // like the following work:
  // auto L = [](auto b) {                <-- 1
  //   return [](auto a) -> decltype(a) { <-- 2
  //      return a;
  //   };
  // };
  // int (*fp)(int) = L(5);  
  // Because the trailing return type can contain DeclRefExprs that refer
  // to the original call operator's variables, we hijack the call 
  // operators ParmVarDecls below.
  TypeSourceInfo *ConvNamePtrToFunctionTSI = 
      S.Context.getTrivialTypeSourceInfo(PtrToFunctionTy, Loc);
  ConvNameLoc.NamedType.TInfo = ConvNamePtrToFunctionTSI;

  // The conversion function is a conversion to a pointer-to-function.
  TypeSourceInfo *ConvTSI = S.Context.getTrivialTypeSourceInfo(ConvTy, Loc);
  FunctionProtoTypeLoc ConvTL = 
      ConvTSI->getTypeLoc().getAs<FunctionProtoTypeLoc>();
  // Get the result of the conversion function which is a pointer-to-function.
  PointerTypeLoc PtrToFunctionTL = 
      ConvTL.getReturnLoc().getAs<PointerTypeLoc>();
  // Do the same for the TypeSourceInfo that is used to name the conversion
  // operator.
  PointerTypeLoc ConvNamePtrToFunctionTL = 
      ConvNamePtrToFunctionTSI->getTypeLoc().getAs<PointerTypeLoc>();
  
  // Get the underlying function types that the conversion function will
  // be converting to (should match the type of the call operator).
  FunctionProtoTypeLoc CallOpConvTL = 
      PtrToFunctionTL.getPointeeLoc().getAs<FunctionProtoTypeLoc>();
  FunctionProtoTypeLoc CallOpConvNameTL = 
    ConvNamePtrToFunctionTL.getPointeeLoc().getAs<FunctionProtoTypeLoc>();
  
  // Wire up the FunctionProtoTypeLocs with the call operator's parameters.
  // These parameter's are essentially used to transform the name and
  // the type of the conversion operator.  By using the same parameters
  // as the call operator's we don't have to fix any back references that
  // the trailing return type of the call operator's uses (such as 
  // decltype(some_type<decltype(a)>::type{} + decltype(a){}) etc.)
  // - we can simply use the return type of the call operator, and 
  // everything should work. 
  SmallVector<ParmVarDecl *, 4> InvokerParams;
  for (unsigned I = 0, N = CallOperator->getNumParams(); I != N; ++I) {
    ParmVarDecl *From = CallOperator->getParamDecl(I);

    InvokerParams.push_back(ParmVarDecl::Create(S.Context, 
           // Temporarily add to the TU. This is set to the invoker below.
                                             S.Context.getTranslationUnitDecl(),
                                             From->getLocStart(),
                                             From->getLocation(),
                                             From->getIdentifier(),
                                             From->getType(),
                                             From->getTypeSourceInfo(),
                                             From->getStorageClass(),
                                             /*DefaultArg=*/nullptr));
    CallOpConvTL.setParam(I, From);
    CallOpConvNameTL.setParam(I, From);
  }

  CXXConversionDecl *Conversion 
    = CXXConversionDecl::Create(S.Context, Class, Loc, 
                                DeclarationNameInfo(ConversionName, 
                                  Loc, ConvNameLoc),
                                ConvTy, 
                                ConvTSI,
                                /*isInline=*/true, /*isExplicit=*/false,
                                /*isConstexpr=*/false, 
                                CallOperator->getBody()->getLocEnd());
  Conversion->setAccess(AS_public);
  Conversion->setImplicit(true);

  if (Class->isGenericLambda()) {
    // Create a template version of the conversion operator, using the template
    // parameter list of the function call operator.
    FunctionTemplateDecl *TemplateCallOperator = 
            CallOperator->getDescribedFunctionTemplate();
    FunctionTemplateDecl *ConversionTemplate =
                  FunctionTemplateDecl::Create(S.Context, Class,
                                      Loc, ConversionName,
                                      TemplateCallOperator->getTemplateParameters(),
                                      Conversion);
    ConversionTemplate->setAccess(AS_public);
    ConversionTemplate->setImplicit(true);
    Conversion->setDescribedFunctionTemplate(ConversionTemplate);
    Class->addDecl(ConversionTemplate);
  } else
    Class->addDecl(Conversion);
  // Add a non-static member function that will be the result of
  // the conversion with a certain unique ID.
  DeclarationName InvokerName = &S.Context.Idents.get(
                                                 getLambdaStaticInvokerName());
  // FIXME: Instead of passing in the CallOperator->getTypeSourceInfo()
  // we should get a prebuilt TrivialTypeSourceInfo from Context
  // using FunctionTy & Loc and get its TypeLoc as a FunctionProtoTypeLoc
  // then rewire the parameters accordingly, by hoisting up the InvokeParams
  // loop below and then use its Params to set Invoke->setParams(...) below.
  // This would avoid the 'const' qualifier of the calloperator from 
  // contaminating the type of the invoker, which is currently adjusted 
  // in SemaTemplateDeduction.cpp:DeduceTemplateArguments.  Fixing the
  // trailing return type of the invoker would require a visitor to rebuild
  // the trailing return type and adjusting all back DeclRefExpr's to refer
  // to the new static invoker parameters - not the call operator's.
  CXXMethodDecl *Invoke
    = CXXMethodDecl::Create(S.Context, Class, Loc, 
                            DeclarationNameInfo(InvokerName, Loc), 
                            InvokerFunctionTy,
                            CallOperator->getTypeSourceInfo(), 
                            SC_Static, /*IsInline=*/true,
                            /*IsConstexpr=*/false, 
                            CallOperator->getBody()->getLocEnd());
  for (unsigned I = 0, N = CallOperator->getNumParams(); I != N; ++I)
    InvokerParams[I]->setOwningFunction(Invoke);
  Invoke->setParams(InvokerParams);
  Invoke->setAccess(AS_private);
  Invoke->setImplicit(true);
  if (Class->isGenericLambda()) {
    FunctionTemplateDecl *TemplateCallOperator = 
            CallOperator->getDescribedFunctionTemplate();
    FunctionTemplateDecl *StaticInvokerTemplate = FunctionTemplateDecl::Create(
                          S.Context, Class, Loc, InvokerName,
                          TemplateCallOperator->getTemplateParameters(),
                          Invoke);
    StaticInvokerTemplate->setAccess(AS_private);
    StaticInvokerTemplate->setImplicit(true);
    Invoke->setDescribedFunctionTemplate(StaticInvokerTemplate);
    Class->addDecl(StaticInvokerTemplate);
  } else
    Class->addDecl(Invoke);
}

/// \brief Add a lambda's conversion to block pointer.
static void addBlockPointerConversion(Sema &S, 
                                      SourceRange IntroducerRange,
                                      CXXRecordDecl *Class,
                                      CXXMethodDecl *CallOperator) {
  const FunctionProtoType *Proto =
      CallOperator->getType()->getAs<FunctionProtoType>();

  // The function type inside the block pointer type is the same as the call
  // operator with some tweaks. The calling convention is the default free
  // function convention, and the type qualifications are lost.
  FunctionProtoType::ExtProtoInfo BlockEPI = Proto->getExtProtoInfo();
  BlockEPI.ExtInfo =
      BlockEPI.ExtInfo.withCallingConv(S.Context.getDefaultCallingConvention(
          Proto->isVariadic(), /*IsCXXMethod=*/false));
  BlockEPI.TypeQuals = 0;
  QualType FunctionTy = S.Context.getFunctionType(
      Proto->getReturnType(), Proto->getParamTypes(), BlockEPI);
  QualType BlockPtrTy = S.Context.getBlockPointerType(FunctionTy);

  FunctionProtoType::ExtProtoInfo ConversionEPI(
      S.Context.getDefaultCallingConvention(
          /*IsVariadic=*/false, /*IsCXXMethod=*/true));
  ConversionEPI.TypeQuals = Qualifiers::Const;
  QualType ConvTy = S.Context.getFunctionType(BlockPtrTy, None, ConversionEPI);

  SourceLocation Loc = IntroducerRange.getBegin();
  DeclarationName Name
    = S.Context.DeclarationNames.getCXXConversionFunctionName(
        S.Context.getCanonicalType(BlockPtrTy));
  DeclarationNameLoc NameLoc;
  NameLoc.NamedType.TInfo = S.Context.getTrivialTypeSourceInfo(BlockPtrTy, Loc);
  CXXConversionDecl *Conversion 
    = CXXConversionDecl::Create(S.Context, Class, Loc, 
                                DeclarationNameInfo(Name, Loc, NameLoc),
                                ConvTy, 
                                S.Context.getTrivialTypeSourceInfo(ConvTy, Loc),
                                /*isInline=*/true, /*isExplicit=*/false,
                                /*isConstexpr=*/false, 
                                CallOperator->getBody()->getLocEnd());
  Conversion->setAccess(AS_public);
  Conversion->setImplicit(true);
  Class->addDecl(Conversion);
}

static ExprResult performLambdaVarCaptureInitialization(
    Sema &S, LambdaScopeInfo::Capture &Capture,
    FieldDecl *Field,
    SmallVectorImpl<VarDecl *> &ArrayIndexVars,
    SmallVectorImpl<unsigned> &ArrayIndexStarts) {
  assert(Capture.isVariableCapture() && "not a variable capture");

  auto *Var = Capture.getVariable();
  SourceLocation Loc = Capture.getLocation();

  // C++11 [expr.prim.lambda]p21:
  //   When the lambda-expression is evaluated, the entities that
  //   are captured by copy are used to direct-initialize each
  //   corresponding non-static data member of the resulting closure
  //   object. (For array members, the array elements are
  //   direct-initialized in increasing subscript order.) These
  //   initializations are performed in the (unspecified) order in
  //   which the non-static data members are declared.
      
  // C++ [expr.prim.lambda]p12:
  //   An entity captured by a lambda-expression is odr-used (3.2) in
  //   the scope containing the lambda-expression.
  ExprResult RefResult = S.BuildDeclarationNameExpr(
      CXXScopeSpec(), DeclarationNameInfo(Var->getDeclName(), Loc), Var);
  if (RefResult.isInvalid())
    return ExprError();
  Expr *Ref = RefResult.get();

  QualType FieldType = Field->getType();

  // When the variable has array type, create index variables for each
  // dimension of the array. We use these index variables to subscript
  // the source array, and other clients (e.g., CodeGen) will perform
  // the necessary iteration with these index variables.
  //
  // FIXME: This is dumb. Add a proper AST representation for array
  // copy-construction and use it here.
  SmallVector<VarDecl *, 4> IndexVariables;
  QualType BaseType = FieldType;
  QualType SizeType = S.Context.getSizeType();
  ArrayIndexStarts.push_back(ArrayIndexVars.size());
  while (const ConstantArrayType *Array
                        = S.Context.getAsConstantArrayType(BaseType)) {
    // Create the iteration variable for this array index.
    IdentifierInfo *IterationVarName = nullptr;
    {
      SmallString<8> Str;
      llvm::raw_svector_ostream OS(Str);
      OS << "__i" << IndexVariables.size();
      IterationVarName = &S.Context.Idents.get(OS.str());
    }
    VarDecl *IterationVar = VarDecl::Create(
        S.Context, S.CurContext, Loc, Loc, IterationVarName, SizeType,
        S.Context.getTrivialTypeSourceInfo(SizeType, Loc), SC_None);
    IterationVar->setImplicit();
    IndexVariables.push_back(IterationVar);
    ArrayIndexVars.push_back(IterationVar);
    
    // Create a reference to the iteration variable.
    ExprResult IterationVarRef =
        S.BuildDeclRefExpr(IterationVar, SizeType, VK_LValue, Loc);
    assert(!IterationVarRef.isInvalid() &&
           "Reference to invented variable cannot fail!");
    IterationVarRef = S.DefaultLvalueConversion(IterationVarRef.get());
    assert(!IterationVarRef.isInvalid() &&
           "Conversion of invented variable cannot fail!");
    
    // Subscript the array with this iteration variable.
    ExprResult Subscript =
        S.CreateBuiltinArraySubscriptExpr(Ref, Loc, IterationVarRef.get(), Loc);
    if (Subscript.isInvalid())
      return ExprError();

    Ref = Subscript.get();
    BaseType = Array->getElementType();
  }

  // Construct the entity that we will be initializing. For an array, this
  // will be first element in the array, which may require several levels
  // of array-subscript entities. 
  SmallVector<InitializedEntity, 4> Entities;
  Entities.reserve(1 + IndexVariables.size());
  Entities.push_back(InitializedEntity::InitializeLambdaCapture(
      Var->getIdentifier(), FieldType, Loc));
  for (unsigned I = 0, N = IndexVariables.size(); I != N; ++I)
    Entities.push_back(
        InitializedEntity::InitializeElement(S.Context, 0, Entities.back()));

  InitializationKind InitKind = InitializationKind::CreateDirect(Loc, Loc, Loc);
  InitializationSequence Init(S, Entities.back(), InitKind, Ref);
  return Init.Perform(S, Entities.back(), InitKind, Ref);
}
         
ExprResult Sema::ActOnLambdaExpr(SourceLocation StartLoc, Stmt *Body, 
                                 Scope *CurScope) {
  LambdaScopeInfo LSI = *cast<LambdaScopeInfo>(FunctionScopes.back());
  ActOnFinishFunctionBody(LSI.CallOperator, Body);
  return BuildLambdaExpr(StartLoc, Body->getLocEnd(), &LSI);
}

static LambdaCaptureDefault
mapImplicitCaptureStyle(CapturingScopeInfo::ImplicitCaptureStyle ICS) {
  switch (ICS) {
  case CapturingScopeInfo::ImpCap_None:
    return LCD_None;
  case CapturingScopeInfo::ImpCap_LambdaByval:
    return LCD_ByCopy;
  case CapturingScopeInfo::ImpCap_CapturedRegion:
  case CapturingScopeInfo::ImpCap_LambdaByref:
    return LCD_ByRef;
  case CapturingScopeInfo::ImpCap_Block:
    llvm_unreachable("block capture in lambda");
  }
  llvm_unreachable("Unknown implicit capture style");
}

ExprResult Sema::BuildLambdaExpr(SourceLocation StartLoc, SourceLocation EndLoc,
                                 LambdaScopeInfo *LSI) {
  // Collect information from the lambda scope.
  SmallVector<LambdaCapture, 4> Captures;
  SmallVector<Expr *, 4> CaptureInits;
  SourceLocation CaptureDefaultLoc = LSI->CaptureDefaultLoc;
  LambdaCaptureDefault CaptureDefault =
      mapImplicitCaptureStyle(LSI->ImpCaptureStyle);
  CXXRecordDecl *Class;
  CXXMethodDecl *CallOperator;
  SourceRange IntroducerRange;
  bool ExplicitParams;
  bool ExplicitResultType;
  bool LambdaExprNeedsCleanups;
  bool ContainsUnexpandedParameterPack;
  SmallVector<VarDecl *, 4> ArrayIndexVars;
  SmallVector<unsigned, 4> ArrayIndexStarts;
  {
    CallOperator = LSI->CallOperator;
    Class = LSI->Lambda;
    IntroducerRange = LSI->IntroducerRange;
    ExplicitParams = LSI->ExplicitParams;
    ExplicitResultType = !LSI->HasImplicitReturnType;
    LambdaExprNeedsCleanups = LSI->ExprNeedsCleanups;
    ContainsUnexpandedParameterPack = LSI->ContainsUnexpandedParameterPack;
    
    CallOperator->setLexicalDeclContext(Class);
    Decl *TemplateOrNonTemplateCallOperatorDecl = 
        CallOperator->getDescribedFunctionTemplate()  
        ? CallOperator->getDescribedFunctionTemplate() 
        : cast<Decl>(CallOperator);

    TemplateOrNonTemplateCallOperatorDecl->setLexicalDeclContext(Class);
    Class->addDecl(TemplateOrNonTemplateCallOperatorDecl);

    PopExpressionEvaluationContext();

    // Translate captures.
    auto CurField = Class->field_begin();
    for (unsigned I = 0, N = LSI->Captures.size(); I != N; ++I, ++CurField) {
      LambdaScopeInfo::Capture From = LSI->Captures[I];
      assert(!From.isBlockCapture() && "Cannot capture __block variables");
      bool IsImplicit = I >= LSI->NumExplicitCaptures;

      // Handle 'this' capture.
      if (From.isThisCapture()) {
        Captures.push_back(
            LambdaCapture(From.getLocation(), IsImplicit, LCK_This));
        CaptureInits.push_back(new (Context) CXXThisExpr(From.getLocation(),
                                                         getCurrentThisType(),
                                                         /*isImplicit=*/true));
        ArrayIndexStarts.push_back(ArrayIndexVars.size());
        continue;
      }
      if (From.isVLATypeCapture()) {
        Captures.push_back(
            LambdaCapture(From.getLocation(), IsImplicit, LCK_VLAType));
        CaptureInits.push_back(nullptr);
        ArrayIndexStarts.push_back(ArrayIndexVars.size());
        continue;
      }

      VarDecl *Var = From.getVariable();
      LambdaCaptureKind Kind = From.isCopyCapture() ? LCK_ByCopy : LCK_ByRef;
      Captures.push_back(LambdaCapture(From.getLocation(), IsImplicit, Kind,
                                       Var, From.getEllipsisLoc()));
      Expr *Init = From.getInitExpr();
      if (!Init) {
        auto InitResult = performLambdaVarCaptureInitialization(
            *this, From, *CurField, ArrayIndexVars, ArrayIndexStarts);
        if (InitResult.isInvalid())
          return ExprError();
        Init = InitResult.get();
      } else {
        ArrayIndexStarts.push_back(ArrayIndexVars.size());
      }
      CaptureInits.push_back(Init);
    }

    // C++11 [expr.prim.lambda]p6:
    //   The closure type for a lambda-expression with no lambda-capture
    //   has a public non-virtual non-explicit const conversion function
    //   to pointer to function having the same parameter and return
    //   types as the closure type's function call operator.
    if (Captures.empty() && CaptureDefault == LCD_None)
      addFunctionPointerConversion(*this, IntroducerRange, Class,
                                   CallOperator);

    // Objective-C++:
    //   The closure type for a lambda-expression has a public non-virtual
    //   non-explicit const conversion function to a block pointer having the
    //   same parameter and return types as the closure type's function call
    //   operator.
    // FIXME: Fix generic lambda to block conversions.
    if (getLangOpts().Blocks && getLangOpts().ObjC1 && 
                                              !Class->isGenericLambda())
      addBlockPointerConversion(*this, IntroducerRange, Class, CallOperator);
    
    // Finalize the lambda class.
    SmallVector<Decl*, 4> Fields(Class->fields());
    ActOnFields(nullptr, Class->getLocation(), Class, Fields, SourceLocation(),
                SourceLocation(), nullptr);
    CheckCompletedCXXClass(Class);
  }

  if (LambdaExprNeedsCleanups)
    ExprNeedsCleanups = true;
  
  LambdaExpr *Lambda = LambdaExpr::Create(Context, Class, IntroducerRange, 
                                          CaptureDefault, CaptureDefaultLoc,
                                          Captures, 
                                          ExplicitParams, ExplicitResultType,
                                          CaptureInits, ArrayIndexVars, 
                                          ArrayIndexStarts, EndLoc,
                                          ContainsUnexpandedParameterPack);

  if (!CurContext->isDependentContext()) {
    switch (ExprEvalContexts.back().Context) {
    // C++11 [expr.prim.lambda]p2:
    //   A lambda-expression shall not appear in an unevaluated operand
    //   (Clause 5).
    case Unevaluated:
    case UnevaluatedAbstract:
    // C++1y [expr.const]p2:
    //   A conditional-expression e is a core constant expression unless the
    //   evaluation of e, following the rules of the abstract machine, would
    //   evaluate [...] a lambda-expression.
    //
    // This is technically incorrect, there are some constant evaluated contexts
    // where this should be allowed.  We should probably fix this when DR1607 is
    // ratified, it lays out the exact set of conditions where we shouldn't
    // allow a lambda-expression.
    case ConstantEvaluated:
      // We don't actually diagnose this case immediately, because we
      // could be within a context where we might find out later that
      // the expression is potentially evaluated (e.g., for typeid).
      ExprEvalContexts.back().Lambdas.push_back(Lambda);
      break;

    case PotentiallyEvaluated:
    case PotentiallyEvaluatedIfUsed:
      break;
    }
  }

  return MaybeBindToTemporary(Lambda);
}

ExprResult Sema::BuildBlockForLambdaConversion(SourceLocation CurrentLocation,
                                               SourceLocation ConvLocation,
                                               CXXConversionDecl *Conv,
                                               Expr *Src) {
  // Make sure that the lambda call operator is marked used.
  CXXRecordDecl *Lambda = Conv->getParent();
  CXXMethodDecl *CallOperator 
    = cast<CXXMethodDecl>(
        Lambda->lookup(
          Context.DeclarationNames.getCXXOperatorName(OO_Call)).front());
  CallOperator->setReferenced();
  CallOperator->markUsed(Context);

  ExprResult Init = PerformCopyInitialization(
                      InitializedEntity::InitializeBlock(ConvLocation, 
                                                         Src->getType(), 
                                                         /*NRVO=*/false),
                      CurrentLocation, Src);
  if (!Init.isInvalid())
    Init = ActOnFinishFullExpr(Init.get());
  
  if (Init.isInvalid())
    return ExprError();
  
  // Create the new block to be returned.
  BlockDecl *Block = BlockDecl::Create(Context, CurContext, ConvLocation);

  // Set the type information.
  Block->setSignatureAsWritten(CallOperator->getTypeSourceInfo());
  Block->setIsVariadic(CallOperator->isVariadic());
  Block->setBlockMissingReturnType(false);

  // Add parameters.
  SmallVector<ParmVarDecl *, 4> BlockParams;
  for (unsigned I = 0, N = CallOperator->getNumParams(); I != N; ++I) {
    ParmVarDecl *From = CallOperator->getParamDecl(I);
    BlockParams.push_back(ParmVarDecl::Create(Context, Block,
                                              From->getLocStart(),
                                              From->getLocation(),
                                              From->getIdentifier(),
                                              From->getType(),
                                              From->getTypeSourceInfo(),
                                              From->getStorageClass(),
                                              /*DefaultArg=*/nullptr));
  }
  Block->setParams(BlockParams);

  Block->setIsConversionFromLambda(true);

  // Add capture. The capture uses a fake variable, which doesn't correspond
  // to any actual memory location. However, the initializer copy-initializes
  // the lambda object.
  TypeSourceInfo *CapVarTSI =
      Context.getTrivialTypeSourceInfo(Src->getType());
  VarDecl *CapVar = VarDecl::Create(Context, Block, ConvLocation,
                                    ConvLocation, nullptr,
                                    Src->getType(), CapVarTSI,
                                    SC_None);
  BlockDecl::Capture Capture(/*Variable=*/CapVar, /*ByRef=*/false,
                             /*Nested=*/false, /*Copy=*/Init.get());
  Block->setCaptures(Context, &Capture, &Capture + 1, 
                     /*CapturesCXXThis=*/false);

  // Add a fake function body to the block. IR generation is responsible
  // for filling in the actual body, which cannot be expressed as an AST.
  Block->setBody(new (Context) CompoundStmt(ConvLocation));

  // Create the block literal expression.
  Expr *BuildBlock = new (Context) BlockExpr(Block, Conv->getConversionType());
  ExprCleanupObjects.push_back(Block);
  ExprNeedsCleanups = true;

  return BuildBlock;
}

#endif // HLSL Change
