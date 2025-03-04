//===--- SemaStmt.cpp - Semantic Analysis for Statements ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements semantic analysis for statements.
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/SemaInternal.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTDiagnostic.h"
#include "clang/AST/CharUnits.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/EvaluatedExprVisitor.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/ExprObjC.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/StmtCXX.h"
#include "clang/AST/StmtObjC.h"
#include "clang/AST/TypeLoc.h"
#include "clang/AST/TypeOrdering.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/Initialization.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/ScopeInfo.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "clang/Sema/SemaHLSL.h" // HLSL Change
using namespace clang;
using namespace sema;

StmtResult Sema::ActOnExprStmt(ExprResult FE) {
  if (FE.isInvalid())
    return StmtError();

  FE = ActOnFinishFullExpr(FE.get(), FE.get()->getExprLoc(),
                           /*DiscardedValue*/ true);
  if (FE.isInvalid())
    return StmtError();

  // C99 6.8.3p2: The expression in an expression statement is evaluated as a
  // void expression for its side effects.  Conversion to void allows any
  // operand, even incomplete types.

  // Same thing in for stmt first clause (when expr) and third clause.
  return StmtResult(FE.getAs<Stmt>());
}



StmtResult Sema::ActOnExprStmtError() {
  DiscardCleanupsInEvaluationContext();
  return StmtError();
}

StmtResult Sema::ActOnNullStmt(SourceLocation SemiLoc,
                               bool HasLeadingEmptyMacro) {
  return new (Context) NullStmt(SemiLoc, HasLeadingEmptyMacro);
}

StmtResult Sema::ActOnDeclStmt(DeclGroupPtrTy dg, SourceLocation StartLoc,
                               SourceLocation EndLoc) {
  DeclGroupRef DG = dg.get();

  // If we have an invalid decl, just return an error.
  if (DG.isNull()) return StmtError();

  return new (Context) DeclStmt(DG, StartLoc, EndLoc);
}

void Sema::ActOnForEachDeclStmt(DeclGroupPtrTy dg) {
  DeclGroupRef DG = dg.get();

  // If we don't have a declaration, or we have an invalid declaration,
  // just return.
  if (DG.isNull() || !DG.isSingleDecl())
    return;

  Decl *decl = DG.getSingleDecl();
  if (!decl || decl->isInvalidDecl())
    return;

  // Only variable declarations are permitted.
  VarDecl *var = dyn_cast<VarDecl>(decl);
  if (!var) {
    Diag(decl->getLocation(), diag::err_non_variable_decl_in_for);
    decl->setInvalidDecl();
    return;
  }

  // foreach variables are never actually initialized in the way that
  // the parser came up with.
  var->setInit(nullptr);

  // In ARC, we don't need to retain the iteration variable of a fast
  // enumeration loop.  Rather than actually trying to catch that
  // during declaration processing, we remove the consequences here.
  if (getLangOpts().ObjCAutoRefCount) {
    QualType type = var->getType();

    // Only do this if we inferred the lifetime.  Inferred lifetime
    // will show up as a local qualifier because explicit lifetime
    // should have shown up as an AttributedType instead.
    if (type.getLocalQualifiers().getObjCLifetime() == Qualifiers::OCL_Strong) {
      // Add 'const' and mark the variable as pseudo-strong.
      var->setType(type.withConst());
      var->setARCPseudoStrong(true);
    }
  }
}

/// \brief Diagnose unused comparisons, both builtin and overloaded operators.
/// For '==' and '!=', suggest fixits for '=' or '|='.
///
/// Adding a cast to void (or other expression wrappers) will prevent the
/// warning from firing.
static bool DiagnoseUnusedComparison(Sema &S, const Expr *E) {
  SourceLocation Loc;
  bool IsNotEqual, CanAssign, IsRelational;

  if (const BinaryOperator *Op = dyn_cast<BinaryOperator>(E)) {
    if (!Op->isComparisonOp())
      return false;

    IsRelational = Op->isRelationalOp();
    Loc = Op->getOperatorLoc();
    IsNotEqual = Op->getOpcode() == BO_NE;
    CanAssign = Op->getLHS()->IgnoreParenImpCasts()->isLValue();
  } else if (const CXXOperatorCallExpr *Op = dyn_cast<CXXOperatorCallExpr>(E)) {
    switch (Op->getOperator()) {
    default:
      return false;
    case OO_EqualEqual:
    case OO_ExclaimEqual:
      IsRelational = false;
      break;
    case OO_Less:
    case OO_Greater:
    case OO_GreaterEqual:
    case OO_LessEqual:
      IsRelational = true;
      break;
    }

    Loc = Op->getOperatorLoc();
    IsNotEqual = Op->getOperator() == OO_ExclaimEqual;
    CanAssign = Op->getArg(0)->IgnoreParenImpCasts()->isLValue();
  } else {
    // Not a typo-prone comparison.
    return false;
  }

  // Suppress warnings when the operator, suspicious as it may be, comes from
  // a macro expansion.
  if (S.SourceMgr.isMacroBodyExpansion(Loc))
    return false;

  S.Diag(Loc, diag::warn_unused_comparison)
    << (unsigned)IsRelational << (unsigned)IsNotEqual << E->getSourceRange();

  // If the LHS is a plausible entity to assign to, provide a fixit hint to
  // correct common typos.
  if (!IsRelational && CanAssign) {
    if (IsNotEqual)
      S.Diag(Loc, diag::note_inequality_comparison_to_or_assign)
        << FixItHint::CreateReplacement(Loc, "|=");
    else
      S.Diag(Loc, diag::note_equality_comparison_to_assign)
        << FixItHint::CreateReplacement(Loc, "=");
  }

  return true;
}

void Sema::DiagnoseUnusedExprResult(const Stmt *S) {
  if (const LabelStmt *Label = dyn_cast_or_null<LabelStmt>(S))
    return DiagnoseUnusedExprResult(Label->getSubStmt());

  const Expr *E = dyn_cast_or_null<Expr>(S);
  if (!E)
    return;

  // If we are in an unevaluated expression context, then there can be no unused
  // results because the results aren't expected to be used in the first place.
  if (isUnevaluatedContext())
    return;

  SourceLocation ExprLoc = E->IgnoreParens()->getExprLoc();
  // In most cases, we don't want to warn if the expression is written in a
  // macro body, or if the macro comes from a system header. If the offending
  // expression is a call to a function with the warn_unused_result attribute,
  // we warn no matter the location. Because of the order in which the various
  // checks need to happen, we factor out the macro-related test here.
  bool ShouldSuppress = 
      SourceMgr.isMacroBodyExpansion(ExprLoc) ||
      SourceMgr.isInSystemMacro(ExprLoc);

  const Expr *WarnExpr;
  SourceLocation Loc;
  SourceRange R1, R2;
  if (!E->isUnusedResultAWarning(WarnExpr, Loc, R1, R2, Context))
    return;

  // If this is a GNU statement expression expanded from a macro, it is probably
  // unused because it is a function-like macro that can be used as either an
  // expression or statement.  Don't warn, because it is almost certainly a
  // false positive.
  if (isa<StmtExpr>(E) && Loc.isMacroID())
    return;

  // Okay, we have an unused result.  Depending on what the base expression is,
  // we might want to make a more specific diagnostic.  Check for one of these
  // cases now.
  unsigned DiagID = diag::warn_unused_expr;
  if (const ExprWithCleanups *Temps = dyn_cast<ExprWithCleanups>(E))
    E = Temps->getSubExpr();
  if (const CXXBindTemporaryExpr *TempExpr = dyn_cast<CXXBindTemporaryExpr>(E))
    E = TempExpr->getSubExpr();

  if (DiagnoseUnusedComparison(*this, E))
    return;

  E = WarnExpr;
  if (const CallExpr *CE = dyn_cast<CallExpr>(E)) {
    if (E->getType()->isVoidType())
      return;

    // If the callee has attribute pure, const, or warn_unused_result, warn with
    // a more specific message to make it clear what is happening. If the call
    // is written in a macro body, only warn if it has the warn_unused_result
    // attribute.
    if (const Decl *FD = CE->getCalleeDecl()) {
      const FunctionDecl *Func = dyn_cast<FunctionDecl>(FD);
      if (Func ? Func->hasUnusedResultAttr()
               : FD->hasAttr<WarnUnusedResultAttr>()) {
        Diag(Loc, diag::warn_unused_result) << R1 << R2;
        return;
      }
      if (ShouldSuppress)
        return;
      if (FD->hasAttr<PureAttr>()) {
        Diag(Loc, diag::warn_hlsl_unused_call) << R1 << R2 << "pure"; // HLSL Change: rather than warn_unused_call
        return;
      }
      if (FD->hasAttr<ConstAttr>()) {
        Diag(Loc, diag::warn_unused_call) << R1 << R2 << "const";
        return;
      }
    }
  } else if (ShouldSuppress)
    return;

  if (const ObjCMessageExpr *ME = dyn_cast<ObjCMessageExpr>(E)) {
    if (getLangOpts().ObjCAutoRefCount && ME->isDelegateInitCall()) {
      Diag(Loc, diag::err_arc_unused_init_message) << R1;
      return;
    }
    const ObjCMethodDecl *MD = ME->getMethodDecl();
    if (MD) {
      if (MD->hasAttr<WarnUnusedResultAttr>()) {
        Diag(Loc, diag::warn_unused_result) << R1 << R2;
        return;
      }
    }
  } else if (const PseudoObjectExpr *POE = dyn_cast<PseudoObjectExpr>(E)) {
    const Expr *Source = POE->getSyntacticForm();
    if (isa<ObjCSubscriptRefExpr>(Source))
      DiagID = diag::warn_unused_container_subscript_expr;
    else
      DiagID = diag::warn_unused_property_expr;
  } else if (const CXXFunctionalCastExpr *FC
                                       = dyn_cast<CXXFunctionalCastExpr>(E)) {
    if (isa<CXXConstructExpr>(FC->getSubExpr()) ||
        isa<CXXTemporaryObjectExpr>(FC->getSubExpr()))
      return;
  }
  // Diagnose "(void*) blah" as a typo for "(void) blah".
  else if (const CStyleCastExpr *CE = dyn_cast<CStyleCastExpr>(E)) {
    TypeSourceInfo *TI = CE->getTypeInfoAsWritten();
    QualType T = TI->getType();

    // We really do want to use the non-canonical type here.
    if (T == Context.VoidPtrTy) {
      PointerTypeLoc TL = TI->getTypeLoc().castAs<PointerTypeLoc>();

      Diag(Loc, diag::warn_unused_voidptr)
        << FixItHint::CreateRemoval(TL.getStarLoc());
      return;
    }
  }

  if (E->isGLValue() && E->getType().isVolatileQualified()) {
    Diag(Loc, diag::warn_unused_volatile) << R1 << R2;
    return;
  }

  DiagRuntimeBehavior(Loc, nullptr, PDiag(DiagID) << R1 << R2);
}

void Sema::ActOnStartOfCompoundStmt() {
  PushCompoundScope();
}

void Sema::ActOnFinishOfCompoundStmt() {
  PopCompoundScope();
}

sema::CompoundScopeInfo &Sema::getCurCompoundScope() const {
  return getCurFunction()->CompoundScopes.back();
}

StmtResult Sema::ActOnCompoundStmt(SourceLocation L, SourceLocation R,
                                   ArrayRef<Stmt *> Elts, bool isStmtExpr) {
  const unsigned NumElts = Elts.size();

  // If we're in C89 mode, check that we don't have any decls after stmts.  If
  // so, emit an extension diagnostic.
  if (!getLangOpts().C99 && !getLangOpts().CPlusPlus) {
    // Note that __extension__ can be around a decl.
    unsigned i = 0;
    // Skip over all declarations.
    for (; i != NumElts && isa<DeclStmt>(Elts[i]); ++i)
      /*empty*/;

    // We found the end of the list or a statement.  Scan for another declstmt.
    for (; i != NumElts && !isa<DeclStmt>(Elts[i]); ++i)
      /*empty*/;

    if (i != NumElts) {
      Decl *D = *cast<DeclStmt>(Elts[i])->decl_begin();
      Diag(D->getLocation(), diag::ext_mixed_decls_code);
    }
  }
  // Warn about unused expressions in statements.
  for (unsigned i = 0; i != NumElts; ++i) {
    // Ignore statements that are last in a statement expression.
    if (isStmtExpr && i == NumElts - 1)
      continue;

    DiagnoseUnusedExprResult(Elts[i]);
  }

  // Check for suspicious empty body (null statement) in `for' and `while'
  // statements.  Don't do anything for template instantiations, this just adds
  // noise.
  if (NumElts != 0 && !CurrentInstantiationScope &&
      getCurCompoundScope().HasEmptyLoopBodies) {
    for (unsigned i = 0; i != NumElts - 1; ++i)
      DiagnoseEmptyLoopBody(Elts[i], Elts[i + 1]);
  }

  return new (Context) CompoundStmt(Context, Elts, L, R);
}

StmtResult
Sema::ActOnCaseStmt(SourceLocation CaseLoc, Expr *LHSVal,
                    SourceLocation DotDotDotLoc, Expr *RHSVal,
                    SourceLocation ColonLoc) {
  assert(LHSVal && "missing expression in case statement");

  if (getCurFunction()->SwitchStack.empty()) {
    Diag(CaseLoc, diag::err_case_not_in_switch);
    return StmtError();
  }

  ExprResult LHS =
      CorrectDelayedTyposInExpr(LHSVal, [this](class Expr *E) {
        if (!getLangOpts().CPlusPlus11)
          return VerifyIntegerConstantExpression(E);
        if (Expr *CondExpr =
                getCurFunction()->SwitchStack.back()->getCond()) {
          QualType CondType = CondExpr->getType();
          llvm::APSInt TempVal;
          return CheckConvertedConstantExpression(E, CondType, TempVal,
                                                        CCEK_CaseValue);
        }
        return ExprError();
      });
  if (LHS.isInvalid())
    return StmtError();
  LHSVal = LHS.get();

  if (!getLangOpts().CPlusPlus11 &&
      getLangOpts().HLSLVersion < hlsl::LangStd::v2017) {
    // C99 6.8.4.2p3: The expression shall be an integer constant.
    // However, GCC allows any evaluatable integer expression.
    if (!LHSVal->isTypeDependent() && !LHSVal->isValueDependent()) {
      LHSVal = VerifyIntegerConstantExpression(LHSVal).get();
      if (!LHSVal)
        return StmtError();
    }

    // GCC extension: The expression shall be an integer constant.

    if (RHSVal && !RHSVal->isTypeDependent() && !RHSVal->isValueDependent()) {
      RHSVal = VerifyIntegerConstantExpression(RHSVal).get();
      // Recover from an error by just forgetting about it.
    }
  }

  LHS = ActOnFinishFullExpr(LHSVal, LHSVal->getExprLoc(), false,
                                 getLangOpts().CPlusPlus11);
  if (LHS.isInvalid())
    return StmtError();

  auto RHS = RHSVal ? ActOnFinishFullExpr(RHSVal, RHSVal->getExprLoc(), false,
                                          getLangOpts().CPlusPlus11)
                    : ExprResult();
  if (RHS.isInvalid())
    return StmtError();

  CaseStmt *CS = new (Context)
      CaseStmt(LHS.get(), RHS.get(), CaseLoc, DotDotDotLoc, ColonLoc);
  getCurFunction()->SwitchStack.back()->addSwitchCase(CS);
  return CS;
}

/// ActOnCaseStmtBody - This installs a statement as the body of a case.
void Sema::ActOnCaseStmtBody(Stmt *caseStmt, Stmt *SubStmt) {
  DiagnoseUnusedExprResult(SubStmt);

  CaseStmt *CS = static_cast<CaseStmt*>(caseStmt);
  CS->setSubStmt(SubStmt);
}

StmtResult
Sema::ActOnDefaultStmt(SourceLocation DefaultLoc, SourceLocation ColonLoc,
                       Stmt *SubStmt, Scope *CurScope) {
  DiagnoseUnusedExprResult(SubStmt);

  if (getCurFunction()->SwitchStack.empty()) {
    Diag(DefaultLoc, diag::err_default_not_in_switch);
    return SubStmt;
  }

  DefaultStmt *DS = new (Context) DefaultStmt(DefaultLoc, ColonLoc, SubStmt);
  getCurFunction()->SwitchStack.back()->addSwitchCase(DS);
  return DS;
}

StmtResult
Sema::ActOnLabelStmt(SourceLocation IdentLoc, LabelDecl *TheDecl,
                     SourceLocation ColonLoc, Stmt *SubStmt) {
  // If the label was multiply defined, reject it now.
  if (TheDecl->getStmt()) {
    Diag(IdentLoc, diag::err_redefinition_of_label) << TheDecl->getDeclName();
    Diag(TheDecl->getLocation(), diag::note_previous_definition);
    return SubStmt;
  }

  // Otherwise, things are good.  Fill in the declaration and return it.
  LabelStmt *LS = new (Context) LabelStmt(IdentLoc, TheDecl, SubStmt);
  TheDecl->setStmt(LS);
  if (!TheDecl->isGnuLocal()) {
    TheDecl->setLocStart(IdentLoc);
    if (!TheDecl->isMSAsmLabel()) {
      // Don't update the location of MS ASM labels.  These will result in
      // a diagnostic, and changing the location here will mess that up.
      TheDecl->setLocation(IdentLoc);
    }
  }
  return LS;
}

StmtResult Sema::ActOnAttributedStmt(SourceLocation AttrLoc,
                                     ArrayRef<const Attr*> Attrs,
                                     Stmt *SubStmt) {
  // HLSL Change - Validate inconsistent attributes being set on statemetns.
  if (getLangOpts().HLSL) {
    bool loopFound = false;
    bool unrollFound = false;
    bool fastOptFound = false;
    for (const Attr* a : Attrs) {
      unrollFound = unrollFound || (a->getKind() == attr::HLSLUnroll);
      fastOptFound = fastOptFound || (a->getKind() == attr::HLSLFastOpt);
      loopFound = loopFound || (a->getKind() == attr::HLSLLoop);
    }
    if (loopFound && unrollFound) {
      Diag(AttrLoc, diag::err_attributes_are_not_compatible) << "loop" << "unroll";
    }
    if (fastOptFound && unrollFound) {
      Diag(AttrLoc, diag::err_attributes_are_not_compatible) << "fastopt" << "unroll";
    }
  }
  // HLSL Change Ends

  // Fill in the declaration and return it.
  AttributedStmt *LS = AttributedStmt::Create(Context, AttrLoc, Attrs, SubStmt);
  return LS;
}

StmtResult
Sema::ActOnIfStmt(SourceLocation IfLoc, FullExprArg CondVal, Decl *CondVar,
                  Stmt *thenStmt, SourceLocation ElseLoc,
                  Stmt *elseStmt) {
  // If the condition was invalid, discard the if statement.  We could recover
  // better by replacing it with a valid expr, but don't do that yet.
  if (!CondVal.get() && !CondVar) {
    getCurFunction()->setHasDroppedStmt();
    return StmtError();
  }

  ExprResult CondResult(CondVal.release());

  VarDecl *ConditionVar = nullptr;
  if (CondVar) {
    ConditionVar = cast<VarDecl>(CondVar);
    CondResult = CheckConditionVariable(ConditionVar, IfLoc, true);
    CondResult = ActOnFinishFullExpr(CondResult.get(), IfLoc);
    if (CondResult.isInvalid())
      return StmtError();
  }
  Expr *ConditionExpr = CondResult.getAs<Expr>();
  if (!ConditionExpr)
    return StmtError();
  // HLSL Change Begin.
  hlsl::DiagnoseControlFlowConditionForHLSL(this, ConditionExpr, "if");
  // HLSL Change End.
  DiagnoseUnusedExprResult(thenStmt);

  if (!elseStmt) {
    DiagnoseEmptyStmtBody(ConditionExpr->getLocEnd(), thenStmt,
                          diag::warn_empty_if_body);
  }

  DiagnoseUnusedExprResult(elseStmt);

  return new (Context) IfStmt(Context, IfLoc, ConditionVar, ConditionExpr,
                              thenStmt, ElseLoc, elseStmt);
}

namespace {
  struct CaseCompareFunctor {
    bool operator()(const std::pair<llvm::APSInt, CaseStmt*> &LHS,
                    const llvm::APSInt &RHS) {
      return LHS.first < RHS;
    }
    bool operator()(const std::pair<llvm::APSInt, CaseStmt*> &LHS,
                    const std::pair<llvm::APSInt, CaseStmt*> &RHS) {
      return LHS.first < RHS.first;
    }
    bool operator()(const llvm::APSInt &LHS,
                    const std::pair<llvm::APSInt, CaseStmt*> &RHS) {
      return LHS < RHS.first;
    }
  };
}

/// CmpCaseVals - Comparison predicate for sorting case values.
///
static bool CmpCaseVals(const std::pair<llvm::APSInt, CaseStmt*>& lhs,
                        const std::pair<llvm::APSInt, CaseStmt*>& rhs) {
  if (lhs.first < rhs.first)
    return true;

  if (lhs.first == rhs.first &&
      lhs.second->getCaseLoc().getRawEncoding()
       < rhs.second->getCaseLoc().getRawEncoding())
    return true;
  return false;
}

/// CmpEnumVals - Comparison predicate for sorting enumeration values.
///
static bool CmpEnumVals(const std::pair<llvm::APSInt, EnumConstantDecl*>& lhs,
                        const std::pair<llvm::APSInt, EnumConstantDecl*>& rhs)
{
  return lhs.first < rhs.first;
}

/// EqEnumVals - Comparison preficate for uniqing enumeration values.
///
static bool EqEnumVals(const std::pair<llvm::APSInt, EnumConstantDecl*>& lhs,
                       const std::pair<llvm::APSInt, EnumConstantDecl*>& rhs)
{
  return lhs.first == rhs.first;
}

/// GetTypeBeforeIntegralPromotion - Returns the pre-promotion type of
/// potentially integral-promoted expression @p expr.
static QualType GetTypeBeforeIntegralPromotion(Expr *&expr) {
  if (ExprWithCleanups *cleanups = dyn_cast<ExprWithCleanups>(expr))
    expr = cleanups->getSubExpr();
  while (ImplicitCastExpr *impcast = dyn_cast<ImplicitCastExpr>(expr)) {
    if (impcast->getCastKind() != CK_IntegralCast) break;
    expr = impcast->getSubExpr();
  }
  return expr->getType();
}

StmtResult
Sema::ActOnStartOfSwitchStmt(SourceLocation SwitchLoc, Expr *Cond,
                             Decl *CondVar) {
  ExprResult CondResult;

  VarDecl *ConditionVar = nullptr;
  if (CondVar) {
    ConditionVar = cast<VarDecl>(CondVar);
    CondResult = CheckConditionVariable(ConditionVar, SourceLocation(), false);
    if (CondResult.isInvalid())
      return StmtError();

    Cond = CondResult.get();
  }

  if (!Cond)
    return StmtError();

  // HLSL Change Begins.
  if (Cond->getType()->isFloatingType()) {
    // Cast float to int for switch.
    QualType intTy = Context.getIntTypeForBitwidth(32, /*isSigned*/true);

    ExprResult Res = ImplicitCastExpr::Create(Context, intTy, CK_FloatingToIntegral, Cond,
                                            nullptr, Cond->getValueKind());
    Cond = Res.get();
  }
  // HLSL Change Ends.

  class SwitchConvertDiagnoser : public ICEConvertDiagnoser {
    Expr *Cond;

  public:
    SwitchConvertDiagnoser(Expr *Cond)
        : ICEConvertDiagnoser(/*AllowScopedEnumerations*/true, false, true),
          Cond(Cond) {}

    SemaDiagnosticBuilder diagnoseNotInt(Sema &S, SourceLocation Loc,
                                         QualType T) override {
      return S.Diag(Loc, diag::err_typecheck_statement_requires_integer) << T;
    }

    SemaDiagnosticBuilder diagnoseIncomplete(
        Sema &S, SourceLocation Loc, QualType T) override {
      return S.Diag(Loc, diag::err_switch_incomplete_class_type)
               << T << Cond->getSourceRange();
    }

    SemaDiagnosticBuilder diagnoseExplicitConv(
        Sema &S, SourceLocation Loc, QualType T, QualType ConvTy) override {
      return S.Diag(Loc, diag::err_switch_explicit_conversion) << T << ConvTy;
    }

    SemaDiagnosticBuilder noteExplicitConv(
        Sema &S, CXXConversionDecl *Conv, QualType ConvTy) override {
      return S.Diag(Conv->getLocation(), diag::note_switch_conversion)
        << ConvTy->isEnumeralType() << ConvTy;
    }

    SemaDiagnosticBuilder diagnoseAmbiguous(Sema &S, SourceLocation Loc,
                                            QualType T) override {
      return S.Diag(Loc, diag::err_switch_multiple_conversions) << T;
    }

    SemaDiagnosticBuilder noteAmbiguous(
        Sema &S, CXXConversionDecl *Conv, QualType ConvTy) override {
      return S.Diag(Conv->getLocation(), diag::note_switch_conversion)
      << ConvTy->isEnumeralType() << ConvTy;
    }

    SemaDiagnosticBuilder diagnoseConversion(
        Sema &S, SourceLocation Loc, QualType T, QualType ConvTy) override {
      llvm_unreachable("conversion functions are permitted");
    }
  } SwitchDiagnoser(Cond);

  CondResult =
      PerformContextualImplicitConversion(SwitchLoc, Cond, SwitchDiagnoser);
  if (CondResult.isInvalid()) return StmtError();
  Cond = CondResult.get();

  // C99 6.8.4.2p5 - Integer promotions are performed on the controlling expr.
  CondResult = UsualUnaryConversions(Cond);
  if (CondResult.isInvalid()) return StmtError();
  Cond = CondResult.get();

  CondResult = ActOnFinishFullExpr(Cond, SwitchLoc);
  if (CondResult.isInvalid())
    return StmtError();
  Cond = CondResult.get();

  getCurFunction()->setHasBranchIntoScope();

  SwitchStmt *SS = new (Context) SwitchStmt(Context, ConditionVar, Cond);
  getCurFunction()->SwitchStack.push_back(SS);
  return SS;
}

static void AdjustAPSInt(llvm::APSInt &Val, unsigned BitWidth, bool IsSigned) {
  Val = Val.extOrTrunc(BitWidth);
  Val.setIsSigned(IsSigned);
}

/// Check the specified case value is in range for the given unpromoted switch
/// type.
static void checkCaseValue(Sema &S, SourceLocation Loc, const llvm::APSInt &Val,
                           unsigned UnpromotedWidth, bool UnpromotedSign) {
  // If the case value was signed and negative and the switch expression is
  // unsigned, don't bother to warn: this is implementation-defined behavior.
  // FIXME: Introduce a second, default-ignored warning for this case?
  if (UnpromotedWidth < Val.getBitWidth()) {
    llvm::APSInt ConvVal(Val);
    AdjustAPSInt(ConvVal, UnpromotedWidth, UnpromotedSign);
    AdjustAPSInt(ConvVal, Val.getBitWidth(), Val.isSigned());
    // FIXME: Use different diagnostics for overflow  in conversion to promoted
    // type versus "switch expression cannot have this value". Use proper
    // IntRange checking rather than just looking at the unpromoted type here.
    if (ConvVal != Val)
      S.Diag(Loc, diag::warn_case_value_overflow) << Val.toString(10)
                                                  << ConvVal.toString(10);
  }
}

typedef SmallVector<std::pair<llvm::APSInt, EnumConstantDecl*>, 64> EnumValsTy;

/// Returns true if we should emit a diagnostic about this case expression not
/// being a part of the enum used in the switch controlling expression.
static bool ShouldDiagnoseSwitchCaseNotInEnum(const Sema &S,
                                              const EnumDecl *ED,
                                              const Expr *CaseExpr,
                                              EnumValsTy::iterator &EI,
                                              EnumValsTy::iterator &EIEnd,
                                              const llvm::APSInt &Val) {
  bool FlagType = ED->hasAttr<FlagEnumAttr>();

  if (const DeclRefExpr *DRE =
          dyn_cast<DeclRefExpr>(CaseExpr->IgnoreParenImpCasts())) {
    if (const VarDecl *VD = dyn_cast<VarDecl>(DRE->getDecl())) {
      QualType VarType = VD->getType();
      QualType EnumType = S.Context.getTypeDeclType(ED);
      if (VD->hasGlobalStorage() && VarType.isConstQualified() &&
          S.Context.hasSameUnqualifiedType(EnumType, VarType))
        return false;
    }
  }

  if (FlagType) {
    return !S.IsValueInFlagEnum(ED, Val, false);
  } else {
    while (EI != EIEnd && EI->first < Val)
      EI++;

    if (EI != EIEnd && EI->first == Val)
      return false;
  }

  return true;
}

StmtResult
Sema::ActOnFinishSwitchStmt(SourceLocation SwitchLoc, Stmt *Switch,
                            Stmt *BodyStmt) {
  SwitchStmt *SS = cast<SwitchStmt>(Switch);
  assert(SS == getCurFunction()->SwitchStack.back() &&
         "switch stack missing push/pop!");

  getCurFunction()->SwitchStack.pop_back();

  if (!BodyStmt) return StmtError();
  SS->setBody(BodyStmt, SwitchLoc);

  Expr *CondExpr = SS->getCond();
  if (!CondExpr) return StmtError();

  QualType CondType = CondExpr->getType();

  Expr *CondExprBeforePromotion = CondExpr;
  QualType CondTypeBeforePromotion =
      GetTypeBeforeIntegralPromotion(CondExprBeforePromotion);

  // C++ 6.4.2.p2:
  // Integral promotions are performed (on the switch condition).
  //
  // A case value unrepresentable by the original switch condition
  // type (before the promotion) doesn't make sense, even when it can
  // be represented by the promoted type.  Therefore we need to find
  // the pre-promotion type of the switch condition.
  if (!CondExpr->isTypeDependent()) {
    // We have already converted the expression to an integral or enumeration
    // type, when we started the switch statement. If we don't have an
    // appropriate type now, just return an error.
    if (!CondType->isIntegralOrEnumerationType())
      return StmtError();

    if (CondExpr->isKnownToHaveBooleanValue()) {
      // switch(bool_expr) {...} is often a programmer error, e.g.
      //   switch(n && mask) { ... }  // Doh - should be "n & mask".
      // One can always use an if statement instead of switch(bool_expr).
      Diag(SwitchLoc, diag::warn_bool_switch_condition)
          << CondExpr->getSourceRange();
    }
  }

  // Get the bitwidth of the switched-on value after promotions. We must
  // convert the integer case values to this width before comparison.
  bool HasDependentValue
    = CondExpr->isTypeDependent() || CondExpr->isValueDependent();
  unsigned CondWidth = HasDependentValue ? 0 : Context.getIntWidth(CondType);
  bool CondIsSigned = CondType->isSignedIntegerOrEnumerationType();

  // Get the width and signedness that the condition might actually have, for
  // warning purposes.
  // FIXME: Grab an IntRange for the condition rather than using the unpromoted
  // type.
  unsigned CondWidthBeforePromotion
    = HasDependentValue ? 0 : Context.getIntWidth(CondTypeBeforePromotion);
  bool CondIsSignedBeforePromotion
    = CondTypeBeforePromotion->isSignedIntegerOrEnumerationType();

  // Accumulate all of the case values in a vector so that we can sort them
  // and detect duplicates.  This vector contains the APInt for the case after
  // it has been converted to the condition type.
  typedef SmallVector<std::pair<llvm::APSInt, CaseStmt*>, 64> CaseValsTy;
  CaseValsTy CaseVals;

  // Keep track of any GNU case ranges we see.  The APSInt is the low value.
  typedef std::vector<std::pair<llvm::APSInt, CaseStmt*> > CaseRangesTy;
  CaseRangesTy CaseRanges;

  DefaultStmt *TheDefaultStmt = nullptr;

  bool CaseListIsErroneous = false;

  for (SwitchCase *SC = SS->getSwitchCaseList(); SC && !HasDependentValue;
       SC = SC->getNextSwitchCase()) {

    if (DefaultStmt *DS = dyn_cast<DefaultStmt>(SC)) {
      if (TheDefaultStmt) {
        Diag(DS->getDefaultLoc(), diag::err_multiple_default_labels_defined);
        Diag(TheDefaultStmt->getDefaultLoc(), diag::note_duplicate_case_prev);

        // FIXME: Remove the default statement from the switch block so that
        // we'll return a valid AST.  This requires recursing down the AST and
        // finding it, not something we are set up to do right now.  For now,
        // just lop the entire switch stmt out of the AST.
        CaseListIsErroneous = true;
      }
      TheDefaultStmt = DS;

    } else {
      CaseStmt *CS = cast<CaseStmt>(SC);

      Expr *Lo = CS->getLHS();

      if (Lo->isTypeDependent() || Lo->isValueDependent()) {
        HasDependentValue = true;
        break;
      }

      llvm::APSInt LoVal;

      if (getLangOpts().CPlusPlus11 ||
          getLangOpts().HLSLVersion >= hlsl::LangStd::v2017) {
        // C++11 [stmt.switch]p2: the constant-expression shall be a converted
        // constant expression of the promoted type of the switch condition.
        ExprResult ConvLo =
          CheckConvertedConstantExpression(Lo, CondType, LoVal, CCEK_CaseValue);
        if (ConvLo.isInvalid()) {
          CaseListIsErroneous = true;
          continue;
        }
        Lo = ConvLo.get();
      } else {
        // We already verified that the expression has a i-c-e value (C99
        // 6.8.4.2p3) - get that value now.
        LoVal = Lo->EvaluateKnownConstInt(Context);

        // If the LHS is not the same type as the condition, insert an implicit
        // cast.
        Lo = DefaultLvalueConversion(Lo).get();
        Lo = ImpCastExprToType(Lo, CondType, CK_IntegralCast).get();
      }

      // Check the unconverted value is within the range of possible values of
      // the switch expression.
      checkCaseValue(*this, Lo->getLocStart(), LoVal,
                     CondWidthBeforePromotion, CondIsSignedBeforePromotion);

      // Convert the value to the same width/sign as the condition.
      AdjustAPSInt(LoVal, CondWidth, CondIsSigned);

      CS->setLHS(Lo);

      // If this is a case range, remember it in CaseRanges, otherwise CaseVals.
      if (CS->getRHS()) {
        if (CS->getRHS()->isTypeDependent() ||
            CS->getRHS()->isValueDependent()) {
          HasDependentValue = true;
          break;
        }
        CaseRanges.push_back(std::make_pair(LoVal, CS));
      } else
        CaseVals.push_back(std::make_pair(LoVal, CS));
    }
  }

  if (!HasDependentValue) {
    // If we don't have a default statement, check whether the
    // condition is constant.
    llvm::APSInt ConstantCondValue;
    bool HasConstantCond = false;
    if (!HasDependentValue && !TheDefaultStmt) {
      HasConstantCond = CondExpr->EvaluateAsInt(ConstantCondValue, Context,
                                                Expr::SE_AllowSideEffects);
      assert(!HasConstantCond ||
             (ConstantCondValue.getBitWidth() == CondWidth &&
              ConstantCondValue.isSigned() == CondIsSigned));
    }
    bool ShouldCheckConstantCond = HasConstantCond;

    // Sort all the scalar case values so we can easily detect duplicates.
    std::stable_sort(CaseVals.begin(), CaseVals.end(), CmpCaseVals);

    if (!CaseVals.empty()) {
      for (unsigned i = 0, e = CaseVals.size(); i != e; ++i) {
        if (ShouldCheckConstantCond &&
            CaseVals[i].first == ConstantCondValue)
          ShouldCheckConstantCond = false;

        if (i != 0 && CaseVals[i].first == CaseVals[i-1].first) {
          // If we have a duplicate, report it.
          // First, determine if either case value has a name
          StringRef PrevString, CurrString;
          Expr *PrevCase = CaseVals[i-1].second->getLHS()->IgnoreParenCasts();
          Expr *CurrCase = CaseVals[i].second->getLHS()->IgnoreParenCasts();
          if (DeclRefExpr *DeclRef = dyn_cast<DeclRefExpr>(PrevCase)) {
            PrevString = DeclRef->getDecl()->getName();
          }
          if (DeclRefExpr *DeclRef = dyn_cast<DeclRefExpr>(CurrCase)) {
            CurrString = DeclRef->getDecl()->getName();
          }
          SmallString<16> CaseValStr;
          CaseVals[i-1].first.toString(CaseValStr);

          if (PrevString == CurrString)
            Diag(CaseVals[i].second->getLHS()->getLocStart(),
                 diag::err_duplicate_case) <<
                 (PrevString.empty() ? StringRef(CaseValStr) : PrevString);
          else
            Diag(CaseVals[i].second->getLHS()->getLocStart(),
                 diag::err_duplicate_case_differing_expr) <<
                 (PrevString.empty() ? StringRef(CaseValStr) : PrevString) <<
                 (CurrString.empty() ? StringRef(CaseValStr) : CurrString) <<
                 CaseValStr;

          Diag(CaseVals[i-1].second->getLHS()->getLocStart(),
               diag::note_duplicate_case_prev);
          // FIXME: We really want to remove the bogus case stmt from the
          // substmt, but we have no way to do this right now.
          CaseListIsErroneous = true;
        }
      }
    }

    // Detect duplicate case ranges, which usually don't exist at all in
    // the first place.
    if (!CaseRanges.empty()) {
      // Sort all the case ranges by their low value so we can easily detect
      // overlaps between ranges.
      std::stable_sort(CaseRanges.begin(), CaseRanges.end());

      // Scan the ranges, computing the high values and removing empty ranges.
      std::vector<llvm::APSInt> HiVals;
      for (unsigned i = 0, e = CaseRanges.size(); i != e; ++i) {
        llvm::APSInt &LoVal = CaseRanges[i].first;
        CaseStmt *CR = CaseRanges[i].second;
        Expr *Hi = CR->getRHS();
        llvm::APSInt HiVal;

        if (getLangOpts().CPlusPlus11) {
          // C++11 [stmt.switch]p2: the constant-expression shall be a converted
          // constant expression of the promoted type of the switch condition.
          ExprResult ConvHi =
            CheckConvertedConstantExpression(Hi, CondType, HiVal,
                                             CCEK_CaseValue);
          if (ConvHi.isInvalid()) {
            CaseListIsErroneous = true;
            continue;
          }
          Hi = ConvHi.get();
        } else {
          HiVal = Hi->EvaluateKnownConstInt(Context);

          // If the RHS is not the same type as the condition, insert an
          // implicit cast.
          Hi = DefaultLvalueConversion(Hi).get();
          Hi = ImpCastExprToType(Hi, CondType, CK_IntegralCast).get();
        }

        // Check the unconverted value is within the range of possible values of
        // the switch expression.
        checkCaseValue(*this, Hi->getLocStart(), HiVal,
                       CondWidthBeforePromotion, CondIsSignedBeforePromotion);

        // Convert the value to the same width/sign as the condition.
        AdjustAPSInt(HiVal, CondWidth, CondIsSigned);

        CR->setRHS(Hi);

        // If the low value is bigger than the high value, the case is empty.
        if (LoVal > HiVal) {
          Diag(CR->getLHS()->getLocStart(), diag::warn_case_empty_range)
            << SourceRange(CR->getLHS()->getLocStart(),
                           Hi->getLocEnd());
          CaseRanges.erase(CaseRanges.begin()+i);
          --i, --e;
          continue;
        }

        if (ShouldCheckConstantCond &&
            LoVal <= ConstantCondValue &&
            ConstantCondValue <= HiVal)
          ShouldCheckConstantCond = false;

        HiVals.push_back(HiVal);
      }

      // Rescan the ranges, looking for overlap with singleton values and other
      // ranges.  Since the range list is sorted, we only need to compare case
      // ranges with their neighbors.
      for (unsigned i = 0, e = CaseRanges.size(); i != e; ++i) {
        llvm::APSInt &CRLo = CaseRanges[i].first;
        llvm::APSInt &CRHi = HiVals[i];
        CaseStmt *CR = CaseRanges[i].second;

        // Check to see whether the case range overlaps with any
        // singleton cases.
        CaseStmt *OverlapStmt = nullptr;
        llvm::APSInt OverlapVal(32);

        // Find the smallest value >= the lower bound.  If I is in the
        // case range, then we have overlap.
        CaseValsTy::iterator I = std::lower_bound(CaseVals.begin(),
                                                  CaseVals.end(), CRLo,
                                                  CaseCompareFunctor());
        if (I != CaseVals.end() && I->first < CRHi) {
          OverlapVal  = I->first;   // Found overlap with scalar.
          OverlapStmt = I->second;
        }

        // Find the smallest value bigger than the upper bound.
        I = std::upper_bound(I, CaseVals.end(), CRHi, CaseCompareFunctor());
        if (I != CaseVals.begin() && (I-1)->first >= CRLo) {
          OverlapVal  = (I-1)->first;      // Found overlap with scalar.
          OverlapStmt = (I-1)->second;
        }

        // Check to see if this case stmt overlaps with the subsequent
        // case range.
        if (i && CRLo <= HiVals[i-1]) {
          OverlapVal  = HiVals[i-1];       // Found overlap with range.
          OverlapStmt = CaseRanges[i-1].second;
        }

        if (OverlapStmt) {
          // If we have a duplicate, report it.
          Diag(CR->getLHS()->getLocStart(), diag::err_duplicate_case)
            << OverlapVal.toString(10);
          Diag(OverlapStmt->getLHS()->getLocStart(),
               diag::note_duplicate_case_prev);
          // FIXME: We really want to remove the bogus case stmt from the
          // substmt, but we have no way to do this right now.
          CaseListIsErroneous = true;
        }
      }
    }

    // Complain if we have a constant condition and we didn't find a match.
    if (!CaseListIsErroneous && ShouldCheckConstantCond) {
      // TODO: it would be nice if we printed enums as enums, chars as
      // chars, etc.
      Diag(CondExpr->getExprLoc(), diag::warn_missing_case_for_condition)
        << ConstantCondValue.toString(10)
        << CondExpr->getSourceRange();
    }

    // Check to see if switch is over an Enum and handles all of its
    // values.  We only issue a warning if there is not 'default:', but
    // we still do the analysis to preserve this information in the AST
    // (which can be used by flow-based analyes).
    //
    const EnumType *ET = CondTypeBeforePromotion->getAs<EnumType>();

    // If switch has default case, then ignore it.
    if (!CaseListIsErroneous  && !HasConstantCond && ET) {
      const EnumDecl *ED = ET->getDecl();
      EnumValsTy EnumVals;

      // Gather all enum values, set their type and sort them,
      // allowing easier comparison with CaseVals.
      for (auto *EDI : ED->enumerators()) {
        llvm::APSInt Val = EDI->getInitVal();
        AdjustAPSInt(Val, CondWidth, CondIsSigned);
        EnumVals.push_back(std::make_pair(Val, EDI));
      }
      std::stable_sort(EnumVals.begin(), EnumVals.end(), CmpEnumVals);
      auto EI = EnumVals.begin(), EIEnd =
        std::unique(EnumVals.begin(), EnumVals.end(), EqEnumVals);

      // See which case values aren't in enum.
      for (CaseValsTy::const_iterator CI = CaseVals.begin();
          CI != CaseVals.end(); CI++) {
        Expr *CaseExpr = CI->second->getLHS();
        if (ShouldDiagnoseSwitchCaseNotInEnum(*this, ED, CaseExpr, EI, EIEnd,
                                              CI->first))
          Diag(CaseExpr->getExprLoc(), diag::warn_not_in_enum)
            << CondTypeBeforePromotion;
      }

      // See which of case ranges aren't in enum
      EI = EnumVals.begin();
      for (CaseRangesTy::const_iterator RI = CaseRanges.begin();
          RI != CaseRanges.end(); RI++) {
        Expr *CaseExpr = RI->second->getLHS();
        if (ShouldDiagnoseSwitchCaseNotInEnum(*this, ED, CaseExpr, EI, EIEnd,
                                              RI->first))
          Diag(CaseExpr->getExprLoc(), diag::warn_not_in_enum)
            << CondTypeBeforePromotion;

        llvm::APSInt Hi =
          RI->second->getRHS()->EvaluateKnownConstInt(Context);
        AdjustAPSInt(Hi, CondWidth, CondIsSigned);

        CaseExpr = RI->second->getRHS();
        if (ShouldDiagnoseSwitchCaseNotInEnum(*this, ED, CaseExpr, EI, EIEnd,
                                              Hi))
          Diag(CaseExpr->getExprLoc(), diag::warn_not_in_enum)
            << CondTypeBeforePromotion;
      }

      // Check which enum vals aren't in switch
      auto CI = CaseVals.begin();
      auto RI = CaseRanges.begin();
      bool hasCasesNotInSwitch = false;

      SmallVector<DeclarationName,8> UnhandledNames;

      for (EI = EnumVals.begin(); EI != EIEnd; EI++){
        // Drop unneeded case values
        while (CI != CaseVals.end() && CI->first < EI->first)
          CI++;

        if (CI != CaseVals.end() && CI->first == EI->first)
          continue;

        // Drop unneeded case ranges
        for (; RI != CaseRanges.end(); RI++) {
          llvm::APSInt Hi =
            RI->second->getRHS()->EvaluateKnownConstInt(Context);
          AdjustAPSInt(Hi, CondWidth, CondIsSigned);
          if (EI->first <= Hi)
            break;
        }

        if (RI == CaseRanges.end() || EI->first < RI->first) {
          hasCasesNotInSwitch = true;
          UnhandledNames.push_back(EI->second->getDeclName());
        }
      }

      if (TheDefaultStmt && UnhandledNames.empty())
        Diag(TheDefaultStmt->getDefaultLoc(), diag::warn_unreachable_default);

      // Produce a nice diagnostic if multiple values aren't handled.
      if (!UnhandledNames.empty()) {
        DiagnosticBuilder DB = Diag(CondExpr->getExprLoc(),
                                    TheDefaultStmt ? diag::warn_def_missing_case
                                                   : diag::warn_missing_case)
                               << (int)UnhandledNames.size();

        for (size_t I = 0, E = std::min(UnhandledNames.size(), (size_t)3);
             I != E; ++I)
          DB << UnhandledNames[I];
      }

      if (!hasCasesNotInSwitch)
        SS->setAllEnumCasesCovered();
    }
  }

  if (BodyStmt)
    DiagnoseEmptyStmtBody(CondExpr->getLocEnd(), BodyStmt,
                          diag::warn_empty_switch_body);

  // FIXME: If the case list was broken is some way, we don't have a good system
  // to patch it up.  Instead, just return the whole substmt as broken.
  if (CaseListIsErroneous)
    return StmtError();

  return SS;
}

void
Sema::DiagnoseAssignmentEnum(QualType DstType, QualType SrcType,
                             Expr *SrcExpr) {
  if (Diags.isIgnored(diag::warn_not_in_enum_assignment, SrcExpr->getExprLoc()))
    return;

  if (const EnumType *ET = DstType->getAs<EnumType>())
    if (!Context.hasSameUnqualifiedType(SrcType, DstType) &&
        SrcType->isIntegerType()) {
      if (!SrcExpr->isTypeDependent() && !SrcExpr->isValueDependent() &&
          SrcExpr->isIntegerConstantExpr(Context)) {
        // Get the bitwidth of the enum value before promotions.
        unsigned DstWidth = Context.getIntWidth(DstType);
        bool DstIsSigned = DstType->isSignedIntegerOrEnumerationType();

        llvm::APSInt RhsVal = SrcExpr->EvaluateKnownConstInt(Context);
        AdjustAPSInt(RhsVal, DstWidth, DstIsSigned);
        const EnumDecl *ED = ET->getDecl();

        if (ED->hasAttr<FlagEnumAttr>()) {
          if (!IsValueInFlagEnum(ED, RhsVal, true))
            Diag(SrcExpr->getExprLoc(), diag::warn_not_in_enum_assignment)
              << DstType.getUnqualifiedType();
        } else {
          typedef SmallVector<std::pair<llvm::APSInt, EnumConstantDecl *>, 64>
              EnumValsTy;
          EnumValsTy EnumVals;

          // Gather all enum values, set their type and sort them,
          // allowing easier comparison with rhs constant.
          for (auto *EDI : ED->enumerators()) {
            llvm::APSInt Val = EDI->getInitVal();
            AdjustAPSInt(Val, DstWidth, DstIsSigned);
            EnumVals.push_back(std::make_pair(Val, EDI));
          }
          if (EnumVals.empty())
            return;
          std::stable_sort(EnumVals.begin(), EnumVals.end(), CmpEnumVals);
          EnumValsTy::iterator EIend =
              std::unique(EnumVals.begin(), EnumVals.end(), EqEnumVals);

          // See which values aren't in the enum.
          EnumValsTy::const_iterator EI = EnumVals.begin();
          while (EI != EIend && EI->first < RhsVal)
            EI++;
          if (EI == EIend || EI->first != RhsVal) {
            Diag(SrcExpr->getExprLoc(), diag::warn_not_in_enum_assignment)
                << DstType.getUnqualifiedType();
          }
        }
      }
    }
}

StmtResult
Sema::ActOnWhileStmt(SourceLocation WhileLoc, FullExprArg Cond,
                     Decl *CondVar, Stmt *Body) {
  ExprResult CondResult(Cond.release());

  VarDecl *ConditionVar = nullptr;
  if (CondVar) {
    ConditionVar = cast<VarDecl>(CondVar);
    CondResult = CheckConditionVariable(ConditionVar, WhileLoc, true);
    CondResult = ActOnFinishFullExpr(CondResult.get(), WhileLoc);
    if (CondResult.isInvalid())
      return StmtError();
  }
  Expr *ConditionExpr = CondResult.get();
  if (!ConditionExpr)
    return StmtError();

  // HLSL Change Begin.
  hlsl::DiagnoseControlFlowConditionForHLSL(this, ConditionExpr, "while");
  // HLSL Change End.

  CheckBreakContinueBinding(ConditionExpr);

  DiagnoseUnusedExprResult(Body);

  if (isa<NullStmt>(Body))
    getCurCompoundScope().setHasEmptyLoopBodies();

  return new (Context)
      WhileStmt(Context, ConditionVar, ConditionExpr, Body, WhileLoc);
}

StmtResult
Sema::ActOnDoStmt(SourceLocation DoLoc, Stmt *Body,
                  SourceLocation WhileLoc, SourceLocation CondLParen,
                  Expr *Cond, SourceLocation CondRParen) {
  assert(Cond && "ActOnDoStmt(): missing expression");

  CheckBreakContinueBinding(Cond);
  ExprResult CondResult = CheckBooleanCondition(Cond, DoLoc);
  if (CondResult.isInvalid())
    return StmtError();
  Cond = CondResult.get();

  CondResult = ActOnFinishFullExpr(Cond, DoLoc);
  if (CondResult.isInvalid())
    return StmtError();
  Cond = CondResult.get();
  // HLSL Change Begin.
  if (Cond) {
    hlsl::DiagnoseControlFlowConditionForHLSL(this, Cond, "do-while");
  }
  // HLSL Change End.

  DiagnoseUnusedExprResult(Body);

  return new (Context) DoStmt(Body, Cond, DoLoc, WhileLoc, CondRParen);
}

namespace {
  // This visitor will traverse a conditional statement and store all
  // the evaluated decls into a vector.  Simple is set to true if none
  // of the excluded constructs are used.
  class DeclExtractor : public EvaluatedExprVisitor<DeclExtractor> {
    llvm::SmallPtrSetImpl<VarDecl*> &Decls;
    SmallVectorImpl<SourceRange> &Ranges;
    bool Simple;
  public:
    typedef EvaluatedExprVisitor<DeclExtractor> Inherited;

    DeclExtractor(Sema &S, llvm::SmallPtrSetImpl<VarDecl*> &Decls,
                  SmallVectorImpl<SourceRange> &Ranges) :
        Inherited(S.Context),
        Decls(Decls),
        Ranges(Ranges),
        Simple(true) {}

    bool isSimple() { return Simple; }

    // Replaces the method in EvaluatedExprVisitor.
    void VisitMemberExpr(MemberExpr* E) {
      Simple = false;
    }

    // Any Stmt not whitelisted will cause the condition to be marked complex.
    void VisitStmt(Stmt *S) {
      Simple = false;
    }

    void VisitBinaryOperator(BinaryOperator *E) {
      Visit(E->getLHS());
      Visit(E->getRHS());
    }

    void VisitCastExpr(CastExpr *E) {
      Visit(E->getSubExpr());
    }

    void VisitUnaryOperator(UnaryOperator *E) {
      // Skip checking conditionals with derefernces.
      if (E->getOpcode() == UO_Deref)
        Simple = false;
      else
        Visit(E->getSubExpr());
    }

    void VisitConditionalOperator(ConditionalOperator *E) {
      Visit(E->getCond());
      Visit(E->getTrueExpr());
      Visit(E->getFalseExpr());
    }

    void VisitParenExpr(ParenExpr *E) {
      Visit(E->getSubExpr());
    }

    void VisitBinaryConditionalOperator(BinaryConditionalOperator *E) {
      Visit(E->getOpaqueValue()->getSourceExpr());
      Visit(E->getFalseExpr());
    }

    void VisitIntegerLiteral(IntegerLiteral *E) { }
    void VisitFloatingLiteral(FloatingLiteral *E) { }
    void VisitCXXBoolLiteralExpr(CXXBoolLiteralExpr *E) { }
    void VisitCharacterLiteral(CharacterLiteral *E) { }
    void VisitGNUNullExpr(GNUNullExpr *E) { }
    void VisitImaginaryLiteral(ImaginaryLiteral *E) { }

    void VisitDeclRefExpr(DeclRefExpr *E) {
      VarDecl *VD = dyn_cast<VarDecl>(E->getDecl());
      if (!VD) return;

      Ranges.push_back(E->getSourceRange());

      Decls.insert(VD);
    }

  }; // end class DeclExtractor

  // DeclMatcher checks to see if the decls are used in a non-evauluated
  // context.
  class DeclMatcher : public EvaluatedExprVisitor<DeclMatcher> {
    llvm::SmallPtrSetImpl<VarDecl*> &Decls;
    bool FoundDecl;

  public:
    typedef EvaluatedExprVisitor<DeclMatcher> Inherited;

    DeclMatcher(Sema &S, llvm::SmallPtrSetImpl<VarDecl*> &Decls,
                Stmt *Statement) :
        Inherited(S.Context), Decls(Decls), FoundDecl(false) {
      if (!Statement) return;

      Visit(Statement);
    }

    void VisitReturnStmt(ReturnStmt *S) {
      FoundDecl = true;
    }

    void VisitBreakStmt(BreakStmt *S) {
      FoundDecl = true;
    }

    void VisitGotoStmt(GotoStmt *S) {
      FoundDecl = true;
    }

    void VisitCastExpr(CastExpr *E) {
      if (E->getCastKind() == CK_LValueToRValue)
        CheckLValueToRValueCast(E->getSubExpr());
      else
        Visit(E->getSubExpr());
    }

    void CheckLValueToRValueCast(Expr *E) {
      E = E->IgnoreParenImpCasts();

      if (isa<DeclRefExpr>(E)) {
        return;
      }

      if (ConditionalOperator *CO = dyn_cast<ConditionalOperator>(E)) {
        Visit(CO->getCond());
        CheckLValueToRValueCast(CO->getTrueExpr());
        CheckLValueToRValueCast(CO->getFalseExpr());
        return;
      }

      if (BinaryConditionalOperator *BCO =
              dyn_cast<BinaryConditionalOperator>(E)) {
        CheckLValueToRValueCast(BCO->getOpaqueValue()->getSourceExpr());
        CheckLValueToRValueCast(BCO->getFalseExpr());
        return;
      }

      Visit(E);
    }

    void VisitDeclRefExpr(DeclRefExpr *E) {
      if (VarDecl *VD = dyn_cast<VarDecl>(E->getDecl()))
        if (Decls.count(VD))
          FoundDecl = true;
    }

    bool FoundDeclInUse() { return FoundDecl; }

  };  // end class DeclMatcher

  void CheckForLoopConditionalStatement(Sema &S, Expr *Second,
                                        Expr *Third, Stmt *Body) {
    // Condition is empty
    if (!Second) return;

    if (S.Diags.isIgnored(diag::warn_variables_not_in_loop_body,
                          Second->getLocStart()))
      return;

    PartialDiagnostic PDiag = S.PDiag(diag::warn_variables_not_in_loop_body);
    llvm::SmallPtrSet<VarDecl*, 8> Decls;
    SmallVector<SourceRange, 10> Ranges;
    DeclExtractor DE(S, Decls, Ranges);
    DE.Visit(Second);

    // Don't analyze complex conditionals.
    if (!DE.isSimple()) return;

    // No decls found.
    if (Decls.size() == 0) return;

    // Don't warn on volatile, static, or global variables.
    for (llvm::SmallPtrSetImpl<VarDecl*>::iterator I = Decls.begin(),
                                                   E = Decls.end();
         I != E; ++I)
      if ((*I)->getType().isVolatileQualified() ||
          (*I)->hasGlobalStorage()) return;

    if (DeclMatcher(S, Decls, Second).FoundDeclInUse() ||
        DeclMatcher(S, Decls, Third).FoundDeclInUse() ||
        DeclMatcher(S, Decls, Body).FoundDeclInUse())
      return;

    // Load decl names into diagnostic.
    if (Decls.size() > 4)
      PDiag << 0;
    else {
      PDiag << Decls.size();
      for (llvm::SmallPtrSetImpl<VarDecl*>::iterator I = Decls.begin(),
                                                     E = Decls.end();
           I != E; ++I)
        PDiag << (*I)->getDeclName();
    }

    // Load SourceRanges into diagnostic if there is room.
    // Otherwise, load the SourceRange of the conditional expression.
    if (Ranges.size() <= PartialDiagnostic::MaxArguments)
      for (SmallVectorImpl<SourceRange>::iterator I = Ranges.begin(),
                                                  E = Ranges.end();
           I != E; ++I)
        PDiag << *I;
    else
      PDiag << Second->getSourceRange();

    S.Diag(Ranges.begin()->getBegin(), PDiag);
  }

  // If Statement is an incemement or decrement, return true and sets the
  // variables Increment and DRE.
  bool ProcessIterationStmt(Sema &S, Stmt* Statement, bool &Increment,
                            DeclRefExpr *&DRE) {
    if (UnaryOperator *UO = dyn_cast<UnaryOperator>(Statement)) {
      switch (UO->getOpcode()) {
        default: return false;
        case UO_PostInc:
        case UO_PreInc:
          Increment = true;
          break;
        case UO_PostDec:
        case UO_PreDec:
          Increment = false;
          break;
      }
      DRE = dyn_cast<DeclRefExpr>(UO->getSubExpr());
      return DRE;
    }

    if (CXXOperatorCallExpr *Call = dyn_cast<CXXOperatorCallExpr>(Statement)) {
      FunctionDecl *FD = Call->getDirectCallee();
      if (!FD || !FD->isOverloadedOperator()) return false;
      switch (FD->getOverloadedOperator()) {
        default: return false;
        case OO_PlusPlus:
          Increment = true;
          break;
        case OO_MinusMinus:
          Increment = false;
          break;
      }
      DRE = dyn_cast<DeclRefExpr>(Call->getArg(0));
      return DRE;
    }

    return false;
  }

  // A visitor to determine if a continue or break statement is a
  // subexpression.
  class BreakContinueFinder : public EvaluatedExprVisitor<BreakContinueFinder> {
    SourceLocation BreakLoc;
    SourceLocation ContinueLoc;
  public:
    BreakContinueFinder(Sema &S, Stmt* Body) :
        Inherited(S.Context) {
      Visit(Body);
    }

    typedef EvaluatedExprVisitor<BreakContinueFinder> Inherited;

    void VisitContinueStmt(ContinueStmt* E) {
      ContinueLoc = E->getContinueLoc();
    }

    void VisitBreakStmt(BreakStmt* E) {
      BreakLoc = E->getBreakLoc();
    }

    bool ContinueFound() { return ContinueLoc.isValid(); }
    bool BreakFound() { return BreakLoc.isValid(); }
    SourceLocation GetContinueLoc() { return ContinueLoc; }
    SourceLocation GetBreakLoc() { return BreakLoc; }

  };  // end class BreakContinueFinder

  // Emit a warning when a loop increment/decrement appears twice per loop
  // iteration.  The conditions which trigger this warning are:
  // 1) The last statement in the loop body and the third expression in the
  //    for loop are both increment or both decrement of the same variable
  // 2) No continue statements in the loop body.
  void CheckForRedundantIteration(Sema &S, Expr *Third, Stmt *Body) {
    // Return when there is nothing to check.
    if (!Body || !Third) return;

    if (S.Diags.isIgnored(diag::warn_redundant_loop_iteration,
                          Third->getLocStart()))
      return;

    // Get the last statement from the loop body.
    CompoundStmt *CS = dyn_cast<CompoundStmt>(Body);
    if (!CS || CS->body_empty()) return;
    Stmt *LastStmt = CS->body_back();
    if (!LastStmt) return;

    bool LoopIncrement, LastIncrement;
    DeclRefExpr *LoopDRE, *LastDRE;

    if (!ProcessIterationStmt(S, Third, LoopIncrement, LoopDRE)) return;
    if (!ProcessIterationStmt(S, LastStmt, LastIncrement, LastDRE)) return;

    // Check that the two statements are both increments or both decrements
    // on the same variable.
    if (LoopIncrement != LastIncrement ||
        LoopDRE->getDecl() != LastDRE->getDecl()) return;

    if (BreakContinueFinder(S, Body).ContinueFound()) return;

    S.Diag(LastDRE->getLocation(), diag::warn_redundant_loop_iteration)
         << LastDRE->getDecl() << LastIncrement;
    S.Diag(LoopDRE->getLocation(), diag::note_loop_iteration_here)
         << LoopIncrement;
  }

} // end namespace


void Sema::CheckBreakContinueBinding(Expr *E) {
  if (!E || getLangOpts().CPlusPlus)
    return;
  BreakContinueFinder BCFinder(*this, E);
  Scope *BreakParent = CurScope->getBreakParent();
  if (BCFinder.BreakFound() && BreakParent) {
    if (BreakParent->getFlags() & Scope::SwitchScope) {
      Diag(BCFinder.GetBreakLoc(), diag::warn_break_binds_to_switch);
    } else {
      Diag(BCFinder.GetBreakLoc(), diag::warn_loop_ctrl_binds_to_inner)
          << "break";
    }
  } else if (BCFinder.ContinueFound() && CurScope->getContinueParent()) {
    Diag(BCFinder.GetContinueLoc(), diag::warn_loop_ctrl_binds_to_inner)
        << "continue";
  }
}

StmtResult
Sema::ActOnForStmt(SourceLocation ForLoc, SourceLocation LParenLoc,
                   Stmt *First, FullExprArg second, Decl *secondVar,
                   FullExprArg third,
                   SourceLocation RParenLoc, Stmt *Body) {
  if (!getLangOpts().CPlusPlus) {
    if (DeclStmt *DS = dyn_cast_or_null<DeclStmt>(First)) {
      // C99 6.8.5p3: The declaration part of a 'for' statement shall only
      // declare identifiers for objects having storage class 'auto' or
      // 'register'.
      for (auto *DI : DS->decls()) {
        VarDecl *VD = dyn_cast<VarDecl>(DI);
        if (VD && VD->isLocalVarDecl() && !VD->hasLocalStorage())
          VD = nullptr;
        if (!VD) {
          Diag(DI->getLocation(), diag::err_non_local_variable_decl_in_for);
          DI->setInvalidDecl();
        }
      }
    }
  }

  CheckBreakContinueBinding(second.get());
  CheckBreakContinueBinding(third.get());

  CheckForLoopConditionalStatement(*this, second.get(), third.get(), Body);
  CheckForRedundantIteration(*this, third.get(), Body);

  ExprResult SecondResult(second.release());
  VarDecl *ConditionVar = nullptr;
  if (secondVar) {
    ConditionVar = cast<VarDecl>(secondVar);
    SecondResult = CheckConditionVariable(ConditionVar, ForLoc, true);
    SecondResult = ActOnFinishFullExpr(SecondResult.get(), ForLoc);
    if (SecondResult.isInvalid())
      return StmtError();
  }
  // HLSL Change Begin.
  Expr *Cond = SecondResult.get();
  if (Cond) {
    hlsl::DiagnoseControlFlowConditionForHLSL(this, Cond, "for");
  }
  // HLSL Change End.
  Expr *Third  = third.release().getAs<Expr>();

  DiagnoseUnusedExprResult(First);
  DiagnoseUnusedExprResult(Third);
  DiagnoseUnusedExprResult(Body);

  if (isa<NullStmt>(Body))
    getCurCompoundScope().setHasEmptyLoopBodies();

  return new (Context) ForStmt(Context, First, Cond, ConditionVar,
                               Third, Body, ForLoc, LParenLoc, RParenLoc);
}

/// In an Objective C collection iteration statement:
///   for (x in y)
/// x can be an arbitrary l-value expression.  Bind it up as a
/// full-expression.
StmtResult Sema::ActOnForEachLValueExpr(Expr *E) {
  // Reduce placeholder expressions here.  Note that this rejects the
  // use of pseudo-object l-values in this position.
  ExprResult result = CheckPlaceholderExpr(E);
  if (result.isInvalid()) return StmtError();
  E = result.get();

  ExprResult FullExpr = ActOnFinishFullExpr(E);
  if (FullExpr.isInvalid())
    return StmtError();
  return StmtResult(static_cast<Stmt*>(FullExpr.get()));
}

ExprResult
Sema::CheckObjCForCollectionOperand(SourceLocation forLoc, Expr *collection) {
  if (!collection)
    return ExprError();

  ExprResult result = CorrectDelayedTyposInExpr(collection);
  if (!result.isUsable())
    return ExprError();
  collection = result.get();

  // Bail out early if we've got a type-dependent expression.
  if (collection->isTypeDependent()) return collection;

  // Perform normal l-value conversion.
  result = DefaultFunctionArrayLvalueConversion(collection);
  if (result.isInvalid())
    return ExprError();
  collection = result.get();

  // The operand needs to have object-pointer type.
  // TODO: should we do a contextual conversion?
  const ObjCObjectPointerType *pointerType =
    collection->getType()->getAs<ObjCObjectPointerType>();
  if (!pointerType)
    return Diag(forLoc, diag::err_collection_expr_type)
             << collection->getType() << collection->getSourceRange();

  // Check that the operand provides
  //   - countByEnumeratingWithState:objects:count:
  const ObjCObjectType *objectType = pointerType->getObjectType();
  ObjCInterfaceDecl *iface = objectType->getInterface();

  // If we have a forward-declared type, we can't do this check.
  // Under ARC, it is an error not to have a forward-declared class.
  if (iface &&
      RequireCompleteType(forLoc, QualType(objectType, 0),
                          getLangOpts().ObjCAutoRefCount
                            ? diag::err_arc_collection_forward
                            : 0,
                          collection)) {
    // Otherwise, if we have any useful type information, check that
    // the type declares the appropriate method.
  } else if (iface || !objectType->qual_empty()) {
    IdentifierInfo *selectorIdents[] = {
      &Context.Idents.get("countByEnumeratingWithState"),
      &Context.Idents.get("objects"),
      &Context.Idents.get("count")
    };
    Selector selector = Context.Selectors.getSelector(3, &selectorIdents[0]);

    ObjCMethodDecl *method = nullptr;

    // If there's an interface, look in both the public and private APIs.
    if (iface) {
      method = iface->lookupInstanceMethod(selector);
      if (!method) method = iface->lookupPrivateMethod(selector);
    }

    // Also check protocol qualifiers.
    if (!method)
      method = LookupMethodInQualifiedType(selector, pointerType,
                                           /*instance*/ true);

    // If we didn't find it anywhere, give up.
    if (!method) {
      Diag(forLoc, diag::warn_collection_expr_type)
        << collection->getType() << selector << collection->getSourceRange();
    }

    // TODO: check for an incompatible signature?
  }

  // Wrap up any cleanups in the expression.
  return collection;
}

StmtResult
Sema::ActOnObjCForCollectionStmt(SourceLocation ForLoc,
                                 Stmt *First, Expr *collection,
                                 SourceLocation RParenLoc) {

  ExprResult CollectionExprResult =
    CheckObjCForCollectionOperand(ForLoc, collection);

  if (First) {
    QualType FirstType;
    if (DeclStmt *DS = dyn_cast<DeclStmt>(First)) {
      if (!DS->isSingleDecl())
        return StmtError(Diag((*DS->decl_begin())->getLocation(),
                         diag::err_toomany_element_decls));

      VarDecl *D = dyn_cast<VarDecl>(DS->getSingleDecl());
      if (!D || D->isInvalidDecl())
        return StmtError();
      
      FirstType = D->getType();
      // C99 6.8.5p3: The declaration part of a 'for' statement shall only
      // declare identifiers for objects having storage class 'auto' or
      // 'register'.
      if (!D->hasLocalStorage())
        return StmtError(Diag(D->getLocation(),
                              diag::err_non_local_variable_decl_in_for));

      // If the type contained 'auto', deduce the 'auto' to 'id'.
      if (FirstType->getContainedAutoType()) {
        OpaqueValueExpr OpaqueId(D->getLocation(), Context.getObjCIdType(),
                                 VK_RValue);
        Expr *DeducedInit = &OpaqueId;
        if (DeduceAutoType(D->getTypeSourceInfo(), DeducedInit, FirstType) ==
                DAR_Failed)
          DiagnoseAutoDeductionFailure(D, DeducedInit);
        if (FirstType.isNull()) {
          D->setInvalidDecl();
          return StmtError();
        }

        D->setType(FirstType);

        if (ActiveTemplateInstantiations.empty()) {
          SourceLocation Loc =
              D->getTypeSourceInfo()->getTypeLoc().getBeginLoc();
          Diag(Loc, diag::warn_auto_var_is_id)
            << D->getDeclName();
        }
      }

    } else {
      Expr *FirstE = cast<Expr>(First);
      if (!FirstE->isTypeDependent() && !FirstE->isLValue())
        return StmtError(Diag(First->getLocStart(),
                   diag::err_selector_element_not_lvalue)
          << First->getSourceRange());

      FirstType = static_cast<Expr*>(First)->getType();
      if (FirstType.isConstQualified())
        Diag(ForLoc, diag::err_selector_element_const_type)
          << FirstType << First->getSourceRange();
    }
    if (!FirstType->isDependentType() &&
        !FirstType->isObjCObjectPointerType() &&
        !FirstType->isBlockPointerType())
        return StmtError(Diag(ForLoc, diag::err_selector_element_type)
                           << FirstType << First->getSourceRange());
  }

  if (CollectionExprResult.isInvalid())
    return StmtError();

  CollectionExprResult = ActOnFinishFullExpr(CollectionExprResult.get());
  if (CollectionExprResult.isInvalid())
    return StmtError();

  return new (Context) ObjCForCollectionStmt(First, CollectionExprResult.get(),
                                             nullptr, ForLoc, RParenLoc);
}

/// Finish building a variable declaration for a for-range statement.
/// \return true if an error occurs.
static bool FinishForRangeVarDecl(Sema &SemaRef, VarDecl *Decl, Expr *Init,
                                  SourceLocation Loc, int DiagID) {
  if (Decl->getType()->isUndeducedType()) {
    ExprResult Res = SemaRef.CorrectDelayedTyposInExpr(Init);
    if (!Res.isUsable()) {
      Decl->setInvalidDecl();
      return true;
    }
    Init = Res.get();
  }

  // Deduce the type for the iterator variable now rather than leaving it to
  // AddInitializerToDecl, so we can produce a more suitable diagnostic.
  QualType InitType;
  if ((!isa<InitListExpr>(Init) && Init->getType()->isVoidType()) ||
      SemaRef.DeduceAutoType(Decl->getTypeSourceInfo(), Init, InitType) ==
          Sema::DAR_Failed)
    SemaRef.Diag(Loc, DiagID) << Init->getType();
  if (InitType.isNull()) {
    Decl->setInvalidDecl();
    return true;
  }
  Decl->setType(InitType);

  // In ARC, infer lifetime.
  // FIXME: ARC may want to turn this into 'const __unsafe_unretained' if
  // we're doing the equivalent of fast iteration.
  if (SemaRef.getLangOpts().ObjCAutoRefCount &&
      SemaRef.inferObjCARCLifetime(Decl))
    Decl->setInvalidDecl();

  SemaRef.AddInitializerToDecl(Decl, Init, /*DirectInit=*/false,
                               /*TypeMayContainAuto=*/false);
  SemaRef.FinalizeDeclaration(Decl);
  SemaRef.CurContext->addHiddenDecl(Decl);
  return false;
}

namespace {

/// Produce a note indicating which begin/end function was implicitly called
/// by a C++11 for-range statement. This is often not obvious from the code,
/// nor from the diagnostics produced when analysing the implicit expressions
/// required in a for-range statement.
void NoteForRangeBeginEndFunction(Sema &SemaRef, Expr *E,
                                  Sema::BeginEndFunction BEF) {
  CallExpr *CE = dyn_cast<CallExpr>(E);
  if (!CE)
    return;
  FunctionDecl *D = dyn_cast<FunctionDecl>(CE->getCalleeDecl());
  if (!D)
    return;
  SourceLocation Loc = D->getLocation();

  std::string Description;
  bool IsTemplate = false;
  if (FunctionTemplateDecl *FunTmpl = D->getPrimaryTemplate()) {
    Description = SemaRef.getTemplateArgumentBindingsText(
      FunTmpl->getTemplateParameters(), *D->getTemplateSpecializationArgs());
    IsTemplate = true;
  }

  SemaRef.Diag(Loc, diag::note_for_range_begin_end)
    << BEF << IsTemplate << Description << E->getType();
}

/// Build a variable declaration for a for-range statement.
VarDecl *BuildForRangeVarDecl(Sema &SemaRef, SourceLocation Loc,
                              QualType Type, const char *Name) {
  DeclContext *DC = SemaRef.CurContext;
  IdentifierInfo *II = &SemaRef.PP.getIdentifierTable().get(Name);
  TypeSourceInfo *TInfo = SemaRef.Context.getTrivialTypeSourceInfo(Type, Loc);
  VarDecl *Decl = VarDecl::Create(SemaRef.Context, DC, Loc, Loc, II, Type,
                                  TInfo, SC_None);
  Decl->setImplicit();
  return Decl;
}

}

static bool ObjCEnumerationCollection(Expr *Collection) {
  return !Collection->isTypeDependent()
          && Collection->getType()->getAs<ObjCObjectPointerType>() != nullptr;
}

/// ActOnCXXForRangeStmt - Check and build a C++11 for-range statement.
///
/// C++11 [stmt.ranged]:
///   A range-based for statement is equivalent to
///
///   {
///     auto && __range = range-init;
///     for ( auto __begin = begin-expr,
///           __end = end-expr;
///           __begin != __end;
///           ++__begin ) {
///       for-range-declaration = *__begin;
///       statement
///     }
///   }
///
/// The body of the loop is not available yet, since it cannot be analysed until
/// we have determined the type of the for-range-declaration.
StmtResult
Sema::ActOnCXXForRangeStmt(SourceLocation ForLoc,
                           Stmt *First, SourceLocation ColonLoc, Expr *Range,
                           SourceLocation RParenLoc, BuildForRangeKind Kind) {
  if (!First)
    return StmtError();

  if (Range && ObjCEnumerationCollection(Range))
    return ActOnObjCForCollectionStmt(ForLoc, First, Range, RParenLoc);

  DeclStmt *DS = dyn_cast<DeclStmt>(First);
  assert(DS && "first part of for range not a decl stmt");

  if (!DS->isSingleDecl()) {
    Diag(DS->getStartLoc(), diag::err_type_defined_in_for_range);
    return StmtError();
  }

  Decl *LoopVar = DS->getSingleDecl();
  if (LoopVar->isInvalidDecl() || !Range ||
      DiagnoseUnexpandedParameterPack(Range, UPPC_Expression)) {
    LoopVar->setInvalidDecl();
    return StmtError();
  }

  // Build  auto && __range = range-init
  SourceLocation RangeLoc = Range->getLocStart();
  VarDecl *RangeVar = BuildForRangeVarDecl(*this, RangeLoc,
                                           Context.getAutoRRefDeductType(),
                                           "__range");
  if (FinishForRangeVarDecl(*this, RangeVar, Range, RangeLoc,
                            diag::err_for_range_deduction_failure)) {
    LoopVar->setInvalidDecl();
    return StmtError();
  }

  // Claim the type doesn't contain auto: we've already done the checking.
  DeclGroupPtrTy RangeGroup =
      BuildDeclaratorGroup(MutableArrayRef<Decl *>((Decl **)&RangeVar, 1),
                           /*TypeMayContainAuto=*/ false);
  StmtResult RangeDecl = ActOnDeclStmt(RangeGroup, RangeLoc, RangeLoc);
  if (RangeDecl.isInvalid()) {
    LoopVar->setInvalidDecl();
    return StmtError();
  }

  return BuildCXXForRangeStmt(ForLoc, ColonLoc, RangeDecl.get(),
                              /*BeginEndDecl=*/nullptr, /*Cond=*/nullptr,
                              /*Inc=*/nullptr, DS, RParenLoc, Kind);
}

/// \brief Create the initialization, compare, and increment steps for
/// the range-based for loop expression.
/// This function does not handle array-based for loops,
/// which are created in Sema::BuildCXXForRangeStmt.
///
/// \returns a ForRangeStatus indicating success or what kind of error occurred.
/// BeginExpr and EndExpr are set and FRS_Success is returned on success;
/// CandidateSet and BEF are set and some non-success value is returned on
/// failure.
static Sema::ForRangeStatus BuildNonArrayForRange(Sema &SemaRef, Scope *S,
                                            Expr *BeginRange, Expr *EndRange,
                                            QualType RangeType,
                                            VarDecl *BeginVar,
                                            VarDecl *EndVar,
                                            SourceLocation ColonLoc,
                                            OverloadCandidateSet *CandidateSet,
                                            ExprResult *BeginExpr,
                                            ExprResult *EndExpr,
                                            Sema::BeginEndFunction *BEF) {
  DeclarationNameInfo BeginNameInfo(
      &SemaRef.PP.getIdentifierTable().get("begin"), ColonLoc);
  DeclarationNameInfo EndNameInfo(&SemaRef.PP.getIdentifierTable().get("end"),
                                  ColonLoc);

  LookupResult BeginMemberLookup(SemaRef, BeginNameInfo,
                                 Sema::LookupMemberName);
  LookupResult EndMemberLookup(SemaRef, EndNameInfo, Sema::LookupMemberName);

  if (CXXRecordDecl *D = RangeType->getAsCXXRecordDecl()) {
    // - if _RangeT is a class type, the unqualified-ids begin and end are
    //   looked up in the scope of class _RangeT as if by class member access
    //   lookup (3.4.5), and if either (or both) finds at least one
    //   declaration, begin-expr and end-expr are __range.begin() and
    //   __range.end(), respectively;
    SemaRef.LookupQualifiedName(BeginMemberLookup, D);
    SemaRef.LookupQualifiedName(EndMemberLookup, D);

    if (BeginMemberLookup.empty() != EndMemberLookup.empty()) {
      SourceLocation RangeLoc = BeginVar->getLocation();
      *BEF = BeginMemberLookup.empty() ? Sema::BEF_end : Sema::BEF_begin;

      SemaRef.Diag(RangeLoc, diag::err_for_range_member_begin_end_mismatch)
          << RangeLoc << BeginRange->getType() << *BEF;
      return Sema::FRS_DiagnosticIssued;
    }
  } else {
    // - otherwise, begin-expr and end-expr are begin(__range) and
    //   end(__range), respectively, where begin and end are looked up with
    //   argument-dependent lookup (3.4.2). For the purposes of this name
    //   lookup, namespace std is an associated namespace.

  }

  *BEF = Sema::BEF_begin;
  Sema::ForRangeStatus RangeStatus =
      SemaRef.BuildForRangeBeginEndCall(S, ColonLoc, ColonLoc, BeginVar,
                                        Sema::BEF_begin, BeginNameInfo,
                                        BeginMemberLookup, CandidateSet,
                                        BeginRange, BeginExpr);

  if (RangeStatus != Sema::FRS_Success)
    return RangeStatus;
  if (FinishForRangeVarDecl(SemaRef, BeginVar, BeginExpr->get(), ColonLoc,
                            diag::err_for_range_iter_deduction_failure)) {
    NoteForRangeBeginEndFunction(SemaRef, BeginExpr->get(), *BEF);
    return Sema::FRS_DiagnosticIssued;
  }

  *BEF = Sema::BEF_end;
  RangeStatus =
      SemaRef.BuildForRangeBeginEndCall(S, ColonLoc, ColonLoc, EndVar,
                                        Sema::BEF_end, EndNameInfo,
                                        EndMemberLookup, CandidateSet,
                                        EndRange, EndExpr);
  if (RangeStatus != Sema::FRS_Success)
    return RangeStatus;
  if (FinishForRangeVarDecl(SemaRef, EndVar, EndExpr->get(), ColonLoc,
                            diag::err_for_range_iter_deduction_failure)) {
    NoteForRangeBeginEndFunction(SemaRef, EndExpr->get(), *BEF);
    return Sema::FRS_DiagnosticIssued;
  }
  return Sema::FRS_Success;
}

/// Speculatively attempt to dereference an invalid range expression.
/// If the attempt fails, this function will return a valid, null StmtResult
/// and emit no diagnostics.
static StmtResult RebuildForRangeWithDereference(Sema &SemaRef, Scope *S,
                                                 SourceLocation ForLoc,
                                                 Stmt *LoopVarDecl,
                                                 SourceLocation ColonLoc,
                                                 Expr *Range,
                                                 SourceLocation RangeLoc,
                                                 SourceLocation RParenLoc) {
  // Determine whether we can rebuild the for-range statement with a
  // dereferenced range expression.
  ExprResult AdjustedRange;
  {
    Sema::SFINAETrap Trap(SemaRef);

    AdjustedRange = SemaRef.BuildUnaryOp(S, RangeLoc, UO_Deref, Range);
    if (AdjustedRange.isInvalid())
      return StmtResult();

    StmtResult SR =
      SemaRef.ActOnCXXForRangeStmt(ForLoc, LoopVarDecl, ColonLoc,
                                   AdjustedRange.get(), RParenLoc,
                                   Sema::BFRK_Check);
    if (SR.isInvalid())
      return StmtResult();
  }

  // The attempt to dereference worked well enough that it could produce a valid
  // loop. Produce a fixit, and rebuild the loop with diagnostics enabled, in
  // case there are any other (non-fatal) problems with it.
  SemaRef.Diag(RangeLoc, diag::err_for_range_dereference)
    << Range->getType() << FixItHint::CreateInsertion(RangeLoc, "*");
  return SemaRef.ActOnCXXForRangeStmt(ForLoc, LoopVarDecl, ColonLoc,
                                      AdjustedRange.get(), RParenLoc,
                                      Sema::BFRK_Rebuild);
}

namespace {
/// RAII object to automatically invalidate a declaration if an error occurs.
struct InvalidateOnErrorScope {
  InvalidateOnErrorScope(Sema &SemaRef, Decl *D, bool Enabled)
      : Trap(SemaRef.Diags), D(D), Enabled(Enabled) {}
  ~InvalidateOnErrorScope() {
    if (Enabled && Trap.hasErrorOccurred())
      D->setInvalidDecl();
  }

  DiagnosticErrorTrap Trap;
  Decl *D;
  bool Enabled;
};
}

/// BuildCXXForRangeStmt - Build or instantiate a C++11 for-range statement.
StmtResult
Sema::BuildCXXForRangeStmt(SourceLocation ForLoc, SourceLocation ColonLoc,
                           Stmt *RangeDecl, Stmt *BeginEnd, Expr *Cond,
                           Expr *Inc, Stmt *LoopVarDecl,
                           SourceLocation RParenLoc, BuildForRangeKind Kind) {
  Scope *S = getCurScope();

  DeclStmt *RangeDS = cast<DeclStmt>(RangeDecl);
  VarDecl *RangeVar = cast<VarDecl>(RangeDS->getSingleDecl());
  QualType RangeVarType = RangeVar->getType();

  DeclStmt *LoopVarDS = cast<DeclStmt>(LoopVarDecl);
  VarDecl *LoopVar = cast<VarDecl>(LoopVarDS->getSingleDecl());

  // If we hit any errors, mark the loop variable as invalid if its type
  // contains 'auto'.
  InvalidateOnErrorScope Invalidate(*this, LoopVar,
                                    LoopVar->getType()->isUndeducedType());

  StmtResult BeginEndDecl = BeginEnd;
  ExprResult NotEqExpr = Cond, IncrExpr = Inc;

  if (RangeVarType->isDependentType()) {
    // The range is implicitly used as a placeholder when it is dependent.
    RangeVar->markUsed(Context);

    // Deduce any 'auto's in the loop variable as 'DependentTy'. We'll fill
    // them in properly when we instantiate the loop.
    if (!LoopVar->isInvalidDecl() && Kind != BFRK_Check)
      LoopVar->setType(SubstAutoType(LoopVar->getType(), Context.DependentTy));
  } else if (!BeginEndDecl.get()) {
    SourceLocation RangeLoc = RangeVar->getLocation();

    const QualType RangeVarNonRefType = RangeVarType.getNonReferenceType();

    ExprResult BeginRangeRef = BuildDeclRefExpr(RangeVar, RangeVarNonRefType,
                                                VK_LValue, ColonLoc);
    if (BeginRangeRef.isInvalid())
      return StmtError();

    ExprResult EndRangeRef = BuildDeclRefExpr(RangeVar, RangeVarNonRefType,
                                              VK_LValue, ColonLoc);
    if (EndRangeRef.isInvalid())
      return StmtError();

    QualType AutoType = Context.getAutoDeductType();
    Expr *Range = RangeVar->getInit();
    if (!Range)
      return StmtError();
    QualType RangeType = Range->getType();

    if (RequireCompleteType(RangeLoc, RangeType,
                            diag::err_for_range_incomplete_type))
      return StmtError();

    // Build auto __begin = begin-expr, __end = end-expr.
    VarDecl *BeginVar = BuildForRangeVarDecl(*this, ColonLoc, AutoType,
                                             "__begin");
    VarDecl *EndVar = BuildForRangeVarDecl(*this, ColonLoc, AutoType,
                                           "__end");

    // Build begin-expr and end-expr and attach to __begin and __end variables.
    ExprResult BeginExpr, EndExpr;
    if (const ArrayType *UnqAT = RangeType->getAsArrayTypeUnsafe()) {
      // - if _RangeT is an array type, begin-expr and end-expr are __range and
      //   __range + __bound, respectively, where __bound is the array bound. If
      //   _RangeT is an array of unknown size or an array of incomplete type,
      //   the program is ill-formed;

      // begin-expr is __range.
      BeginExpr = BeginRangeRef;
      if (FinishForRangeVarDecl(*this, BeginVar, BeginRangeRef.get(), ColonLoc,
                                diag::err_for_range_iter_deduction_failure)) {
        NoteForRangeBeginEndFunction(*this, BeginExpr.get(), BEF_begin);
        return StmtError();
      }

      // Find the array bound.
      ExprResult BoundExpr;
      if (const ConstantArrayType *CAT = dyn_cast<ConstantArrayType>(UnqAT))
        BoundExpr = IntegerLiteral::Create(
            Context, CAT->getSize(), Context.getPointerDiffType(), RangeLoc);
      else if (const VariableArrayType *VAT =
               dyn_cast<VariableArrayType>(UnqAT))
        BoundExpr = VAT->getSizeExpr();
      else {
        // Can't be a DependentSizedArrayType or an IncompleteArrayType since
        // UnqAT is not incomplete and Range is not type-dependent.
        llvm_unreachable("Unexpected array type in for-range");
      }

      // end-expr is __range + __bound.
      EndExpr = ActOnBinOp(S, ColonLoc, tok::plus, EndRangeRef.get(),
                           BoundExpr.get());
      if (EndExpr.isInvalid())
        return StmtError();
      if (FinishForRangeVarDecl(*this, EndVar, EndExpr.get(), ColonLoc,
                                diag::err_for_range_iter_deduction_failure)) {
        NoteForRangeBeginEndFunction(*this, EndExpr.get(), BEF_end);
        return StmtError();
      }
    } else {
      OverloadCandidateSet CandidateSet(RangeLoc,
                                        OverloadCandidateSet::CSK_Normal);
      Sema::BeginEndFunction BEFFailure;
      ForRangeStatus RangeStatus =
          BuildNonArrayForRange(*this, S, BeginRangeRef.get(),
                                EndRangeRef.get(), RangeType,
                                BeginVar, EndVar, ColonLoc, &CandidateSet,
                                &BeginExpr, &EndExpr, &BEFFailure);

      if (Kind == BFRK_Build && RangeStatus == FRS_NoViableFunction &&
          BEFFailure == BEF_begin) {
        // If the range is being built from an array parameter, emit a
        // a diagnostic that it is being treated as a pointer.
        if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(Range)) {
          if (ParmVarDecl *PVD = dyn_cast<ParmVarDecl>(DRE->getDecl())) {
            QualType ArrayTy = PVD->getOriginalType();
            QualType PointerTy = PVD->getType();
            if (PointerTy->isPointerType() && ArrayTy->isArrayType()) {
              Diag(Range->getLocStart(), diag::err_range_on_array_parameter)
                << RangeLoc << PVD << ArrayTy << PointerTy;
              Diag(PVD->getLocation(), diag::note_declared_at);
              return StmtError();
            }
          }
        }

        // If building the range failed, try dereferencing the range expression
        // unless a diagnostic was issued or the end function is problematic.
        StmtResult SR = RebuildForRangeWithDereference(*this, S, ForLoc,
                                                       LoopVarDecl, ColonLoc,
                                                       Range, RangeLoc,
                                                       RParenLoc);
        if (SR.isInvalid() || SR.isUsable())
          return SR;
      }

      // Otherwise, emit diagnostics if we haven't already.
      if (RangeStatus == FRS_NoViableFunction) {
        Expr *Range = BEFFailure ? EndRangeRef.get() : BeginRangeRef.get();
        Diag(Range->getLocStart(), diag::err_for_range_invalid)
            << RangeLoc << Range->getType() << BEFFailure;
        CandidateSet.NoteCandidates(*this, OCD_AllCandidates, Range);
      }
      // Return an error if no fix was discovered.
      if (RangeStatus != FRS_Success)
        return StmtError();
    }

    assert(!BeginExpr.isInvalid() && !EndExpr.isInvalid() &&
           "invalid range expression in for loop");

    // C++11 [dcl.spec.auto]p7: BeginType and EndType must be the same.
    QualType BeginType = BeginVar->getType(), EndType = EndVar->getType();
    if (!Context.hasSameType(BeginType, EndType)) {
      Diag(RangeLoc, diag::err_for_range_begin_end_types_differ)
        << BeginType << EndType;
      NoteForRangeBeginEndFunction(*this, BeginExpr.get(), BEF_begin);
      NoteForRangeBeginEndFunction(*this, EndExpr.get(), BEF_end);
    }

    Decl *BeginEndDecls[] = { BeginVar, EndVar };
    // Claim the type doesn't contain auto: we've already done the checking.
    DeclGroupPtrTy BeginEndGroup =
        BuildDeclaratorGroup(MutableArrayRef<Decl *>(BeginEndDecls, 2),
                             /*TypeMayContainAuto=*/ false);
    BeginEndDecl = ActOnDeclStmt(BeginEndGroup, ColonLoc, ColonLoc);

    const QualType BeginRefNonRefType = BeginType.getNonReferenceType();
    ExprResult BeginRef = BuildDeclRefExpr(BeginVar, BeginRefNonRefType,
                                           VK_LValue, ColonLoc);
    if (BeginRef.isInvalid())
      return StmtError();

    ExprResult EndRef = BuildDeclRefExpr(EndVar, EndType.getNonReferenceType(),
                                         VK_LValue, ColonLoc);
    if (EndRef.isInvalid())
      return StmtError();

    // Build and check __begin != __end expression.
    NotEqExpr = ActOnBinOp(S, ColonLoc, tok::exclaimequal,
                           BeginRef.get(), EndRef.get());
    NotEqExpr = ActOnBooleanCondition(S, ColonLoc, NotEqExpr.get());
    NotEqExpr = ActOnFinishFullExpr(NotEqExpr.get());
    if (NotEqExpr.isInvalid()) {
      Diag(RangeLoc, diag::note_for_range_invalid_iterator)
        << RangeLoc << 0 << BeginRangeRef.get()->getType();
      NoteForRangeBeginEndFunction(*this, BeginExpr.get(), BEF_begin);
      if (!Context.hasSameType(BeginType, EndType))
        NoteForRangeBeginEndFunction(*this, EndExpr.get(), BEF_end);
      return StmtError();
    }

    // Build and check ++__begin expression.
    BeginRef = BuildDeclRefExpr(BeginVar, BeginRefNonRefType,
                                VK_LValue, ColonLoc);
    if (BeginRef.isInvalid())
      return StmtError();

    IncrExpr = ActOnUnaryOp(S, ColonLoc, tok::plusplus, BeginRef.get());
    IncrExpr = ActOnFinishFullExpr(IncrExpr.get());
    if (IncrExpr.isInvalid()) {
      Diag(RangeLoc, diag::note_for_range_invalid_iterator)
        << RangeLoc << 2 << BeginRangeRef.get()->getType() ;
      NoteForRangeBeginEndFunction(*this, BeginExpr.get(), BEF_begin);
      return StmtError();
    }

    // Build and check *__begin  expression.
    BeginRef = BuildDeclRefExpr(BeginVar, BeginRefNonRefType,
                                VK_LValue, ColonLoc);
    if (BeginRef.isInvalid())
      return StmtError();

    ExprResult DerefExpr = ActOnUnaryOp(S, ColonLoc, tok::star, BeginRef.get());
    if (DerefExpr.isInvalid()) {
      Diag(RangeLoc, diag::note_for_range_invalid_iterator)
        << RangeLoc << 1 << BeginRangeRef.get()->getType();
      NoteForRangeBeginEndFunction(*this, BeginExpr.get(), BEF_begin);
      return StmtError();
    }

    // Attach  *__begin  as initializer for VD. Don't touch it if we're just
    // trying to determine whether this would be a valid range.
    if (!LoopVar->isInvalidDecl() && Kind != BFRK_Check) {
      AddInitializerToDecl(LoopVar, DerefExpr.get(), /*DirectInit=*/false,
                           /*TypeMayContainAuto=*/true);
      if (LoopVar->isInvalidDecl())
        NoteForRangeBeginEndFunction(*this, BeginExpr.get(), BEF_begin);
    }
  }

  // Don't bother to actually allocate the result if we're just trying to
  // determine whether it would be valid.
  if (Kind == BFRK_Check)
    return StmtResult();

  return new (Context) CXXForRangeStmt(
      RangeDS, cast_or_null<DeclStmt>(BeginEndDecl.get()), NotEqExpr.get(),
      IncrExpr.get(), LoopVarDS, /*Body=*/nullptr, ForLoc, ColonLoc, RParenLoc);
}

/// FinishObjCForCollectionStmt - Attach the body to a objective-C foreach
/// statement.
StmtResult Sema::FinishObjCForCollectionStmt(Stmt *S, Stmt *B) {
  if (!S || !B)
    return StmtError();
  ObjCForCollectionStmt * ForStmt = cast<ObjCForCollectionStmt>(S);

  ForStmt->setBody(B);
  return S;
}

// Warn when the loop variable is a const reference that creates a copy.
// Suggest using the non-reference type for copies.  If a copy can be prevented
// suggest the const reference type that would do so.
// For instance, given "for (const &Foo : Range)", suggest
// "for (const Foo : Range)" to denote a copy is made for the loop.  If
// possible, also suggest "for (const &Bar : Range)" if this type prevents
// the copy altogether.
static void DiagnoseForRangeReferenceVariableCopies(Sema &SemaRef,
                                                    const VarDecl *VD,
                                                    QualType RangeInitType) {
  const Expr *InitExpr = VD->getInit();
  if (!InitExpr)
    return;

  QualType VariableType = VD->getType();

  const MaterializeTemporaryExpr *MTE =
      dyn_cast<MaterializeTemporaryExpr>(InitExpr);

  // No copy made.
  if (!MTE)
    return;

  const Expr *E = MTE->GetTemporaryExpr()->IgnoreImpCasts();

  // Searching for either UnaryOperator for dereference of a pointer or
  // CXXOperatorCallExpr for handling iterators.
  while (!isa<CXXOperatorCallExpr>(E) && !isa<UnaryOperator>(E)) {
    if (const CXXConstructExpr *CCE = dyn_cast<CXXConstructExpr>(E)) {
      E = CCE->getArg(0);
    } else if (const CXXMemberCallExpr *Call = dyn_cast<CXXMemberCallExpr>(E)) {
      const MemberExpr *ME = cast<MemberExpr>(Call->getCallee());
      E = ME->getBase();
    } else {
      const MaterializeTemporaryExpr *MTE = cast<MaterializeTemporaryExpr>(E);
      E = MTE->GetTemporaryExpr();
    }
    E = E->IgnoreImpCasts();
  }

  bool ReturnsReference = false;
  if (isa<UnaryOperator>(E)) {
    ReturnsReference = true;
  } else {
    const CXXOperatorCallExpr *Call = cast<CXXOperatorCallExpr>(E);
    const FunctionDecl *FD = Call->getDirectCallee();
    QualType ReturnType = FD->getReturnType();
    ReturnsReference = ReturnType->isReferenceType();
  }

  if (ReturnsReference) {
    // Loop variable creates a temporary.  Suggest either to go with
    // non-reference loop variable to indiciate a copy is made, or
    // the correct time to bind a const reference.
    SemaRef.Diag(VD->getLocation(), diag::warn_for_range_const_reference_copy)
        << VD << VariableType << E->getType();
    QualType NonReferenceType = VariableType.getNonReferenceType();
    NonReferenceType.removeLocalConst();
    QualType NewReferenceType =
        SemaRef.Context.getLValueReferenceType(E->getType().withConst());
    SemaRef.Diag(VD->getLocStart(), diag::note_use_type_or_non_reference)
        << NonReferenceType << NewReferenceType << VD->getSourceRange();
  } else {
    // The range always returns a copy, so a temporary is always created.
    // Suggest removing the reference from the loop variable.
    SemaRef.Diag(VD->getLocation(), diag::warn_for_range_variable_always_copy)
        << VD << RangeInitType;
    QualType NonReferenceType = VariableType.getNonReferenceType();
    NonReferenceType.removeLocalConst();
    SemaRef.Diag(VD->getLocStart(), diag::note_use_non_reference_type)
        << NonReferenceType << VD->getSourceRange();
  }
}

// Warns when the loop variable can be changed to a reference type to
// prevent a copy.  For instance, if given "for (const Foo x : Range)" suggest
// "for (const Foo &x : Range)" if this form does not make a copy.
static void DiagnoseForRangeConstVariableCopies(Sema &SemaRef,
                                                const VarDecl *VD) {
  const Expr *InitExpr = VD->getInit();
  if (!InitExpr)
    return;

  QualType VariableType = VD->getType();

  if (const CXXConstructExpr *CE = dyn_cast<CXXConstructExpr>(InitExpr)) {
    if (!CE->getConstructor()->isCopyConstructor())
      return;
  } else if (const CastExpr *CE = dyn_cast<CastExpr>(InitExpr)) {
    if (CE->getCastKind() != CK_LValueToRValue)
      return;
  } else {
    return;
  }

  // TODO: Determine a maximum size that a POD type can be before a diagnostic
  // should be emitted.  Also, only ignore POD types with trivial copy
  // constructors.
  if (VariableType.isPODType(SemaRef.Context))
    return;

  // Suggest changing from a const variable to a const reference variable
  // if doing so will prevent a copy.
  SemaRef.Diag(VD->getLocation(), diag::warn_for_range_copy)
      << VD << VariableType << InitExpr->getType();
  SemaRef.Diag(VD->getLocStart(), diag::note_use_reference_type)
      << SemaRef.Context.getLValueReferenceType(VariableType)
      << VD->getSourceRange();
}

/// DiagnoseForRangeVariableCopies - Diagnose three cases and fixes for them.
/// 1) for (const foo &x : foos) where foos only returns a copy.  Suggest
///    using "const foo x" to show that a copy is made
/// 2) for (const bar &x : foos) where bar is a temporary intialized by bar.
///    Suggest either "const bar x" to keep the copying or "const foo& x" to
///    prevent the copy.
/// 3) for (const foo x : foos) where x is constructed from a reference foo.
///    Suggest "const foo &x" to prevent the copy.
static void DiagnoseForRangeVariableCopies(Sema &SemaRef,
                                           const CXXForRangeStmt *ForStmt) {
  if (SemaRef.Diags.isIgnored(diag::warn_for_range_const_reference_copy,
                              ForStmt->getLocStart()) &&
      SemaRef.Diags.isIgnored(diag::warn_for_range_variable_always_copy,
                              ForStmt->getLocStart()) &&
      SemaRef.Diags.isIgnored(diag::warn_for_range_copy,
                              ForStmt->getLocStart())) {
    return;
  }

  const VarDecl *VD = ForStmt->getLoopVariable();
  if (!VD)
    return;

  QualType VariableType = VD->getType();

  if (VariableType->isIncompleteType())
    return;

  const Expr *InitExpr = VD->getInit();
  if (!InitExpr)
    return;

  if (VariableType->isReferenceType()) {
    DiagnoseForRangeReferenceVariableCopies(SemaRef, VD,
                                            ForStmt->getRangeInit()->getType());
  } else if (VariableType.isConstQualified()) {
    DiagnoseForRangeConstVariableCopies(SemaRef, VD);
  }
}

/// FinishCXXForRangeStmt - Attach the body to a C++0x for-range statement.
/// This is a separate step from ActOnCXXForRangeStmt because analysis of the
/// body cannot be performed until after the type of the range variable is
/// determined.
StmtResult Sema::FinishCXXForRangeStmt(Stmt *S, Stmt *B) {
  if (!S || !B)
    return StmtError();

  if (isa<ObjCForCollectionStmt>(S))
    return FinishObjCForCollectionStmt(S, B);

  CXXForRangeStmt *ForStmt = cast<CXXForRangeStmt>(S);
  ForStmt->setBody(B);

  DiagnoseEmptyStmtBody(ForStmt->getRParenLoc(), B,
                        diag::warn_empty_range_based_for_body);

  DiagnoseForRangeVariableCopies(*this, ForStmt);

  return S;
}

StmtResult Sema::ActOnGotoStmt(SourceLocation GotoLoc,
                               SourceLocation LabelLoc,
                               LabelDecl *TheDecl) {
  getCurFunction()->setHasBranchIntoScope();
  TheDecl->markUsed(Context);
  return new (Context) GotoStmt(TheDecl, GotoLoc, LabelLoc);
}

StmtResult
Sema::ActOnIndirectGotoStmt(SourceLocation GotoLoc, SourceLocation StarLoc,
                            Expr *E) {
  // Convert operand to void*
  if (!E->isTypeDependent()) {
    QualType ETy = E->getType();
    QualType DestTy = Context.getPointerType(Context.VoidTy.withConst());
    ExprResult ExprRes = E;
    AssignConvertType ConvTy =
      CheckSingleAssignmentConstraints(DestTy, ExprRes);
    if (ExprRes.isInvalid())
      return StmtError();
    E = ExprRes.get();
    if (DiagnoseAssignmentResult(ConvTy, StarLoc, DestTy, ETy, E, AA_Passing))
      return StmtError();
  }

  ExprResult ExprRes = ActOnFinishFullExpr(E);
  if (ExprRes.isInvalid())
    return StmtError();
  E = ExprRes.get();

  getCurFunction()->setHasIndirectGoto();

  return new (Context) IndirectGotoStmt(GotoLoc, StarLoc, E);
}

static void CheckJumpOutOfSEHFinally(Sema &S, SourceLocation Loc,
                                     const Scope &DestScope) {
  if (!S.CurrentSEHFinally.empty() &&
      DestScope.Contains(*S.CurrentSEHFinally.back())) {
    S.Diag(Loc, diag::warn_jump_out_of_seh_finally);
  }
}

StmtResult
Sema::ActOnContinueStmt(SourceLocation ContinueLoc, Scope *CurScope) {
  Scope *S = CurScope->getContinueParent();
  if (!S) {
    // C99 6.8.6.2p1: A break shall appear only in or as a loop body.
    return StmtError(Diag(ContinueLoc, diag::err_continue_not_in_loop));
  }
  CheckJumpOutOfSEHFinally(*this, ContinueLoc, *S);

  return new (Context) ContinueStmt(ContinueLoc);
}

StmtResult
Sema::ActOnBreakStmt(SourceLocation BreakLoc, Scope *CurScope) {
  Scope *S = CurScope->getBreakParent();
  if (!S) {
    // C99 6.8.6.3p1: A break shall appear only in or as a switch/loop body.
    return StmtError(Diag(BreakLoc, diag::err_break_not_in_loop_or_switch));
  }
  if (S->isOpenMPLoopScope())
    return StmtError(Diag(BreakLoc, diag::err_omp_loop_cannot_use_stmt)
                     << "break");
  CheckJumpOutOfSEHFinally(*this, BreakLoc, *S);

  return new (Context) BreakStmt(BreakLoc);
}

/// \brief Determine whether the given expression is a candidate for
/// copy elision in either a return statement or a throw expression.
///
/// \param ReturnType If we're determining the copy elision candidate for
/// a return statement, this is the return type of the function. If we're
/// determining the copy elision candidate for a throw expression, this will
/// be a NULL type.
///
/// \param E The expression being returned from the function or block, or
/// being thrown.
///
/// \param AllowFunctionParameter Whether we allow function parameters to
/// be considered NRVO candidates. C++ prohibits this for NRVO itself, but
/// we re-use this logic to determine whether we should try to move as part of
/// a return or throw (which does allow function parameters).
///
/// \returns The NRVO candidate variable, if the return statement may use the
/// NRVO, or NULL if there is no such candidate.
VarDecl *Sema::getCopyElisionCandidate(QualType ReturnType,
                                       Expr *E,
                                       bool AllowFunctionParameter) {
  if (!getLangOpts().CPlusPlus)
    return nullptr;

  // - in a return statement in a function [where] ...
  // ... the expression is the name of a non-volatile automatic object ...
  DeclRefExpr *DR = dyn_cast<DeclRefExpr>(E->IgnoreParens());
  if (!DR || DR->refersToEnclosingVariableOrCapture())
    return nullptr;
  VarDecl *VD = dyn_cast<VarDecl>(DR->getDecl());
  if (!VD)
    return nullptr;

  // HLSL Change Begins: NRVO unsafe for a variety of cases in HLSL
  if (getLangOpts().HLSL && hlsl::ShouldSkipNRVO(*this, ReturnType, VD, getCurFunctionDecl()))
    return nullptr;
  // HLSL Change Ends

  if (isCopyElisionCandidate(ReturnType, VD, AllowFunctionParameter))
    return VD;
  return nullptr;
}

bool Sema::isCopyElisionCandidate(QualType ReturnType, const VarDecl *VD,
                                  bool AllowFunctionParameter) {
  QualType VDType = VD->getType();
  // - in a return statement in a function with ...
  // ... a class return type ...
  if (!ReturnType.isNull() && !ReturnType->isDependentType()) {
    if (!ReturnType->isRecordType())
      return false;
    // ... the same cv-unqualified type as the function return type ...
    if (!VDType->isDependentType() &&
        !Context.hasSameUnqualifiedType(ReturnType, VDType))
      return false;
  }

  // ...object (other than a function or catch-clause parameter)...
  if (VD->getKind() != Decl::Var &&
      !(AllowFunctionParameter && VD->getKind() == Decl::ParmVar))
    return false;
  if (VD->isExceptionVariable()) return false;

  // ...automatic...
  if (!VD->hasLocalStorage()) return false;

  // ...non-volatile...
  if (VD->getType().isVolatileQualified()) return false;

  // __block variables can't be allocated in a way that permits NRVO.
  if (VD->hasAttr<BlocksAttr>()) return false;

  // Variables with higher required alignment than their type's ABI
  // alignment cannot use NRVO.
  if (!VD->getType()->isDependentType() && VD->hasAttr<AlignedAttr>() &&
      Context.getDeclAlign(VD) > Context.getTypeAlignInChars(VD->getType()))
    return false;

  return true;
}

/// \brief Perform the initialization of a potentially-movable value, which
/// is the result of return value.
///
/// This routine implements C++0x [class.copy]p33, which attempts to treat
/// returned lvalues as rvalues in certain cases (to prefer move construction),
/// then falls back to treating them as lvalues if that failed.
ExprResult
Sema::PerformMoveOrCopyInitialization(const InitializedEntity &Entity,
                                      const VarDecl *NRVOCandidate,
                                      QualType ResultType,
                                      Expr *Value,
                                      bool AllowNRVO) {
  // C++0x [class.copy]p33:
  //   When the criteria for elision of a copy operation are met or would
  //   be met save for the fact that the source object is a function
  //   parameter, and the object to be copied is designated by an lvalue,
  //   overload resolution to select the constructor for the copy is first
  //   performed as if the object were designated by an rvalue.
  ExprResult Res = ExprError();
  if (AllowNRVO &&
      (NRVOCandidate || getCopyElisionCandidate(ResultType, Value, true))) {
    ImplicitCastExpr AsRvalue(ImplicitCastExpr::OnStack,
                              Value->getType(), CK_NoOp, Value, VK_XValue);

    Expr *InitExpr = &AsRvalue;
    InitializationKind Kind
      = InitializationKind::CreateCopy(Value->getLocStart(),
                                       Value->getLocStart());
    InitializationSequence Seq(*this, Entity, Kind, InitExpr);

    //   [...] If overload resolution fails, or if the type of the first
    //   parameter of the selected constructor is not an rvalue reference
    //   to the object's type (possibly cv-qualified), overload resolution
    //   is performed again, considering the object as an lvalue.
    if (Seq) {
      for (InitializationSequence::step_iterator Step = Seq.step_begin(),
           StepEnd = Seq.step_end();
           Step != StepEnd; ++Step) {
        if (Step->Kind != InitializationSequence::SK_ConstructorInitialization)
          continue;

        CXXConstructorDecl *Constructor
        = cast<CXXConstructorDecl>(Step->Function.Function);

        const RValueReferenceType *RRefType
          = Constructor->getParamDecl(0)->getType()
                                                 ->getAs<RValueReferenceType>();

        // If we don't meet the criteria, break out now.
        if (!RRefType ||
            !Context.hasSameUnqualifiedType(RRefType->getPointeeType(),
                            Context.getTypeDeclType(Constructor->getParent())))
          break;

        // Promote "AsRvalue" to the heap, since we now need this
        // expression node to persist.
        Value = ImplicitCastExpr::Create(Context, Value->getType(),
                                         CK_NoOp, Value, nullptr, VK_XValue);

        // Complete type-checking the initialization of the return type
        // using the constructor we found.
        Res = Seq.Perform(*this, Entity, Kind, Value);
      }
    }
  }

  // Either we didn't meet the criteria for treating an lvalue as an rvalue,
  // above, or overload resolution failed. Either way, we need to try
  // (again) now with the return value expression as written.
  if (Res.isInvalid())
    Res = PerformCopyInitialization(Entity, SourceLocation(), Value);

  return Res;
}

/// \brief Determine whether the declared return type of the specified function
/// contains 'auto'.
static bool hasDeducedReturnType(FunctionDecl *FD) {
  const FunctionProtoType *FPT =
      FD->getTypeSourceInfo()->getType()->castAs<FunctionProtoType>();
  return FPT->getReturnType()->isUndeducedType();
}

/// ActOnCapScopeReturnStmt - Utility routine to type-check return statements
/// for capturing scopes.
///
StmtResult
Sema::ActOnCapScopeReturnStmt(SourceLocation ReturnLoc, Expr *RetValExp) {
  // If this is the first return we've seen, infer the return type.
  // [expr.prim.lambda]p4 in C++11; block literals follow the same rules.
  CapturingScopeInfo *CurCap = cast<CapturingScopeInfo>(getCurFunction());
  QualType FnRetType = CurCap->ReturnType;
  LambdaScopeInfo *CurLambda = dyn_cast<LambdaScopeInfo>(CurCap);

  if (CurLambda && hasDeducedReturnType(CurLambda->CallOperator)) {
    // In C++1y, the return type may involve 'auto'.
    // FIXME: Blocks might have a return type of 'auto' explicitly specified.
    FunctionDecl *FD = CurLambda->CallOperator;
    if (CurCap->ReturnType.isNull())
      CurCap->ReturnType = FD->getReturnType();

    AutoType *AT = CurCap->ReturnType->getContainedAutoType();
    assert(AT && "lost auto type from lambda return type");
    if (DeduceFunctionTypeFromReturnExpr(FD, ReturnLoc, RetValExp, AT)) {
      FD->setInvalidDecl();
      return StmtError();
    }
    CurCap->ReturnType = FnRetType = FD->getReturnType();
  } else if (CurCap->HasImplicitReturnType) {
    // For blocks/lambdas with implicit return types, we check each return
    // statement individually, and deduce the common return type when the block
    // or lambda is completed.
    // FIXME: Fold this into the 'auto' codepath above.
    if (RetValExp && !isa<InitListExpr>(RetValExp)) {
      ExprResult Result = DefaultFunctionArrayLvalueConversion(RetValExp);
      if (Result.isInvalid())
        return StmtError();
      RetValExp = Result.get();

      // DR1048: even prior to C++14, we should use the 'auto' deduction rules
      // when deducing a return type for a lambda-expression (or by extension
      // for a block). These rules differ from the stated C++11 rules only in
      // that they remove top-level cv-qualifiers.
      if (!CurContext->isDependentContext())
        FnRetType = RetValExp->getType().getUnqualifiedType();
      else
        FnRetType = CurCap->ReturnType = Context.DependentTy;
    } else {
      if (RetValExp) {
        // C++11 [expr.lambda.prim]p4 bans inferring the result from an
        // initializer list, because it is not an expression (even
        // though we represent it as one). We still deduce 'void'.
        Diag(ReturnLoc, diag::err_lambda_return_init_list)
          << RetValExp->getSourceRange();
      }

      FnRetType = Context.VoidTy;
    }

    // Although we'll properly infer the type of the block once it's completed,
    // make sure we provide a return type now for better error recovery.
    if (CurCap->ReturnType.isNull())
      CurCap->ReturnType = FnRetType;
  }
  assert(!FnRetType.isNull());

  if (BlockScopeInfo *CurBlock = dyn_cast<BlockScopeInfo>(CurCap)) {
    if (CurBlock->FunctionType->getAs<FunctionType>()->getNoReturnAttr()) {
      Diag(ReturnLoc, diag::err_noreturn_block_has_return_expr);
      return StmtError();
    }
  } else if (CapturedRegionScopeInfo *CurRegion =
                 dyn_cast<CapturedRegionScopeInfo>(CurCap)) {
    Diag(ReturnLoc, diag::err_return_in_captured_stmt) << CurRegion->getRegionName();
    return StmtError();
  } else {
    assert(CurLambda && "unknown kind of captured scope");
    if (CurLambda->CallOperator->getType()->getAs<FunctionType>()
            ->getNoReturnAttr()) {
      Diag(ReturnLoc, diag::err_noreturn_lambda_has_return_expr);
      return StmtError();
    }
  }

  // Otherwise, verify that this result type matches the previous one.  We are
  // pickier with blocks than for normal functions because we don't have GCC
  // compatibility to worry about here.
  const VarDecl *NRVOCandidate = nullptr;
  if (FnRetType->isDependentType()) {
    // Delay processing for now.  TODO: there are lots of dependent
    // types we can conclusively prove aren't void.
  } else if (FnRetType->isVoidType()) {
    if (RetValExp && !isa<InitListExpr>(RetValExp) &&
        !(getLangOpts().CPlusPlus &&
          (RetValExp->isTypeDependent() ||
           RetValExp->getType()->isVoidType()))) {
      if (!getLangOpts().CPlusPlus &&
          RetValExp->getType()->isVoidType())
        Diag(ReturnLoc, diag::ext_return_has_void_expr) << "literal" << 2;
      else {
        Diag(ReturnLoc, diag::err_return_block_has_expr);
        RetValExp = nullptr;
      }
    }
  } else if (!RetValExp) {
    return StmtError(Diag(ReturnLoc, diag::err_block_return_missing_expr));
  } else if (!RetValExp->isTypeDependent()) {
    // we have a non-void block with an expression, continue checking

    // C99 6.8.6.4p3(136): The return statement is not an assignment. The
    // overlap restriction of subclause 6.5.16.1 does not apply to the case of
    // function return.

    // In C++ the return statement is handled via a copy initialization.
    // the C version of which boils down to CheckSingleAssignmentConstraints.
    NRVOCandidate = getCopyElisionCandidate(FnRetType, RetValExp, false);
    InitializedEntity Entity = InitializedEntity::InitializeResult(ReturnLoc,
                                                                   FnRetType,
                                                      NRVOCandidate != nullptr);
    ExprResult Res = PerformMoveOrCopyInitialization(Entity, NRVOCandidate,
                                                     FnRetType, RetValExp);
    if (Res.isInvalid()) {
      // FIXME: Cleanup temporaries here, anyway?
      return StmtError();
    }
    RetValExp = Res.get();
    CheckReturnValExpr(RetValExp, FnRetType, ReturnLoc);
  } else {
    NRVOCandidate = getCopyElisionCandidate(FnRetType, RetValExp, false);
  }

  if (RetValExp) {
    ExprResult ER = ActOnFinishFullExpr(RetValExp, ReturnLoc);
    if (ER.isInvalid())
      return StmtError();
    RetValExp = ER.get();
  }
  ReturnStmt *Result = new (Context) ReturnStmt(ReturnLoc, RetValExp,
                                                NRVOCandidate);

  // If we need to check for the named return value optimization,
  // or if we need to infer the return type,
  // save the return statement in our scope for later processing.
  if (CurCap->HasImplicitReturnType || NRVOCandidate)
    FunctionScopes.back()->Returns.push_back(Result);

  return Result;
}

namespace {
/// \brief Marks all typedefs in all local classes in a type referenced.
///
/// In a function like
/// auto f() {
///   struct S { typedef int a; };
///   return S();
/// }
///
/// the local type escapes and could be referenced in some TUs but not in
/// others. Pretend that all local typedefs are always referenced, to not warn
/// on this. This isn't necessary if f has internal linkage, or the typedef
/// is private.
class LocalTypedefNameReferencer
    : public RecursiveASTVisitor<LocalTypedefNameReferencer> {
public:
  LocalTypedefNameReferencer(Sema &S) : S(S) {}
  bool VisitRecordType(const RecordType *RT);
private:
  Sema &S;
};
bool LocalTypedefNameReferencer::VisitRecordType(const RecordType *RT) {
  auto *R = dyn_cast<CXXRecordDecl>(RT->getDecl());
  if (!R || !R->isLocalClass() || !R->isLocalClass()->isExternallyVisible() ||
      R->isDependentType())
    return true;
  for (auto *TmpD : R->decls())
    if (auto *T = dyn_cast<TypedefNameDecl>(TmpD))
      if (T->getAccess() != AS_private || R->hasFriends())
        S.MarkAnyDeclReferenced(T->getLocation(), T, /*OdrUse=*/false);
  return true;
}
}

TypeLoc Sema::getReturnTypeLoc(FunctionDecl *FD) const {
  TypeLoc TL = FD->getTypeSourceInfo()->getTypeLoc().IgnoreParens();
  while (auto ATL = TL.getAs<AttributedTypeLoc>())
    TL = ATL.getModifiedLoc().IgnoreParens();
  return TL.castAs<FunctionProtoTypeLoc>().getReturnLoc();
}

/// Deduce the return type for a function from a returned expression, per
/// C++1y [dcl.spec.auto]p6.
bool Sema::DeduceFunctionTypeFromReturnExpr(FunctionDecl *FD,
                                            SourceLocation ReturnLoc,
                                            Expr *&RetExpr,
                                            AutoType *AT) {
  TypeLoc OrigResultType = getReturnTypeLoc(FD);
  QualType Deduced;

  if (RetExpr && isa<InitListExpr>(RetExpr)) {
    //  If the deduction is for a return statement and the initializer is
    //  a braced-init-list, the program is ill-formed.
    Diag(RetExpr->getExprLoc(),
         getCurLambda() ? diag::err_lambda_return_init_list
                        : diag::err_auto_fn_return_init_list)
        << RetExpr->getSourceRange();
    return true;
  }

  if (FD->isDependentContext()) {
    // C++1y [dcl.spec.auto]p12:
    //   Return type deduction [...] occurs when the definition is
    //   instantiated even if the function body contains a return
    //   statement with a non-type-dependent operand.
    assert(AT->isDeduced() && "should have deduced to dependent type");
    return false;
  } else if (RetExpr) {
    //  If the deduction is for a return statement and the initializer is
    //  a braced-init-list, the program is ill-formed.
    if (isa<InitListExpr>(RetExpr)) {
      Diag(RetExpr->getExprLoc(), diag::err_auto_fn_return_init_list);
      return true;
    }

    //  Otherwise, [...] deduce a value for U using the rules of template
    //  argument deduction.
    DeduceAutoResult DAR = DeduceAutoType(OrigResultType, RetExpr, Deduced);

    if (DAR == DAR_Failed && !FD->isInvalidDecl())
      Diag(RetExpr->getExprLoc(), diag::err_auto_fn_deduction_failure)
        << OrigResultType.getType() << RetExpr->getType();

    if (DAR != DAR_Succeeded)
      return true;

    // If a local type is part of the returned type, mark its fields as
    // referenced.
    LocalTypedefNameReferencer Referencer(*this);
    Referencer.TraverseType(RetExpr->getType());
  } else {
    //  In the case of a return with no operand, the initializer is considered
    //  to be void().
    //
    // Deduction here can only succeed if the return type is exactly 'cv auto'
    // or 'decltype(auto)', so just check for that case directly.
    if (!OrigResultType.getType()->getAs<AutoType>()) {
      Diag(ReturnLoc, diag::err_auto_fn_return_void_but_not_auto)
        << OrigResultType.getType();
      return true;
    }
    // We always deduce U = void in this case.
    Deduced = SubstAutoType(OrigResultType.getType(), Context.VoidTy);
    if (Deduced.isNull())
      return true;
  }

  //  If a function with a declared return type that contains a placeholder type
  //  has multiple return statements, the return type is deduced for each return
  //  statement. [...] if the type deduced is not the same in each deduction,
  //  the program is ill-formed.
  if (AT->isDeduced() && !FD->isInvalidDecl()) {
    AutoType *NewAT = Deduced->getContainedAutoType();
    if (!FD->isDependentContext() &&
        !Context.hasSameType(AT->getDeducedType(), NewAT->getDeducedType())) {
      const LambdaScopeInfo *LambdaSI = getCurLambda();
      if (LambdaSI && LambdaSI->HasImplicitReturnType) {
        Diag(ReturnLoc, diag::err_typecheck_missing_return_type_incompatible)
          << NewAT->getDeducedType() << AT->getDeducedType()
          << true /*IsLambda*/;
      } else {
        Diag(ReturnLoc, diag::err_auto_fn_different_deductions)
          << (AT->isDecltypeAuto() ? 1 : 0)
          << NewAT->getDeducedType() << AT->getDeducedType();
      }
      return true;
    }
  } else if (!FD->isInvalidDecl()) {
    // Update all declarations of the function to have the deduced return type.
    Context.adjustDeducedFunctionResultType(FD, Deduced);
  }

  return false;
}

StmtResult
Sema::ActOnReturnStmt(SourceLocation ReturnLoc, Expr *RetValExp,
                      Scope *CurScope) {
  StmtResult R = BuildReturnStmt(ReturnLoc, RetValExp);
  if (R.isInvalid()) {
    return R;
  }

  if (VarDecl *VD =
      const_cast<VarDecl*>(cast<ReturnStmt>(R.get())->getNRVOCandidate())) {
    CurScope->addNRVOCandidate(VD);
  } else {
    CurScope->setNoNRVO();
  }

  CheckJumpOutOfSEHFinally(*this, ReturnLoc, *CurScope->getFnParent());

  return R;
}

StmtResult Sema::BuildReturnStmt(SourceLocation ReturnLoc, Expr *RetValExp) {
  // Check for unexpanded parameter packs.
  if (RetValExp && DiagnoseUnexpandedParameterPack(RetValExp))
    return StmtError();

  if (isa<CapturingScopeInfo>(getCurFunction()))
    return ActOnCapScopeReturnStmt(ReturnLoc, RetValExp);

  QualType FnRetType;
  QualType RelatedRetType;
  const AttrVec *Attrs = nullptr;
  bool isObjCMethod = false;

  if (const FunctionDecl *FD = getCurFunctionDecl()) {
    FnRetType = FD->getReturnType();
    if (FD->hasAttrs())
      Attrs = &FD->getAttrs();
    if (FD->isNoReturn())
      Diag(ReturnLoc, diag::warn_noreturn_function_has_return_expr)
        << FD->getDeclName();
  } else if (ObjCMethodDecl *MD = getCurMethodDecl()) {
    FnRetType = MD->getReturnType();
    isObjCMethod = true;
    if (MD->hasAttrs())
      Attrs = &MD->getAttrs();
    if (MD->hasRelatedResultType() && MD->getClassInterface()) {
      // In the implementation of a method with a related return type, the
      // type used to type-check the validity of return statements within the
      // method body is a pointer to the type of the class being implemented.
      RelatedRetType = Context.getObjCInterfaceType(MD->getClassInterface());
      RelatedRetType = Context.getObjCObjectPointerType(RelatedRetType);
    }
  } else // If we don't have a function/method context, bail.
    return StmtError();

  // FIXME: Add a flag to the ScopeInfo to indicate whether we're performing
  // deduction.
  if (getLangOpts().CPlusPlus14) {
    if (AutoType *AT = FnRetType->getContainedAutoType()) {
      FunctionDecl *FD = cast<FunctionDecl>(CurContext);
      if (DeduceFunctionTypeFromReturnExpr(FD, ReturnLoc, RetValExp, AT)) {
        FD->setInvalidDecl();
        return StmtError();
      } else {
        FnRetType = FD->getReturnType();
      }
    }
  }

  // HLSL Change begin - Diagnose mismatched globallycoherent attrs on return.
  if (RetValExp)
    DiagnoseGloballyCoherentMismatch(RetValExp, FnRetType, ReturnLoc);
  // HLSL Change end

  bool HasDependentReturnType = FnRetType->isDependentType();

  ReturnStmt *Result = nullptr;
  if (FnRetType->isVoidType()) {
    if (RetValExp) {
      if (isa<InitListExpr>(RetValExp)) {
        // We simply never allow init lists as the return value of void
        // functions. This is compatible because this was never allowed before,
        // so there's no legacy code to deal with.
        NamedDecl *CurDecl = getCurFunctionOrMethodDecl();
        int FunctionKind = 0;
        if (isa<ObjCMethodDecl>(CurDecl))
          FunctionKind = 1;
        else if (isa<CXXConstructorDecl>(CurDecl))
          FunctionKind = 2;
        else if (isa<CXXDestructorDecl>(CurDecl))
          FunctionKind = 3;

        Diag(ReturnLoc, diag::err_return_init_list)
          << CurDecl->getDeclName() << FunctionKind
          << RetValExp->getSourceRange();

        // Drop the expression.
        RetValExp = nullptr;
      } else if (!RetValExp->isTypeDependent()) {
        // C99 6.8.6.4p1 (ext_ since GCC warns)
        unsigned D = diag::ext_return_has_expr;
        if (RetValExp->getType()->isVoidType()) {
          NamedDecl *CurDecl = getCurFunctionOrMethodDecl();
          if (isa<CXXConstructorDecl>(CurDecl) ||
              isa<CXXDestructorDecl>(CurDecl))
            D = diag::err_ctor_dtor_returns_void;
          else
            D = diag::ext_return_has_void_expr;
        }
        else {
          ExprResult Result = RetValExp;
          Result = IgnoredValueConversions(Result.get());
          if (Result.isInvalid())
            return StmtError();
          RetValExp = Result.get();
          RetValExp = ImpCastExprToType(RetValExp,
                                        Context.VoidTy, CK_ToVoid).get();
        }
        // return of void in constructor/destructor is illegal in C++.
        if (D == diag::err_ctor_dtor_returns_void) {
          NamedDecl *CurDecl = getCurFunctionOrMethodDecl();
          Diag(ReturnLoc, D)
            << CurDecl->getDeclName() << isa<CXXDestructorDecl>(CurDecl)
            << RetValExp->getSourceRange();
        }
        // return (some void expression); is legal in C++.
        else if (D != diag::ext_return_has_void_expr ||
            !getLangOpts().CPlusPlus) {
          NamedDecl *CurDecl = getCurFunctionOrMethodDecl();

          int FunctionKind = 0;
          if (isa<ObjCMethodDecl>(CurDecl))
            FunctionKind = 1;
          else if (isa<CXXConstructorDecl>(CurDecl))
            FunctionKind = 2;
          else if (isa<CXXDestructorDecl>(CurDecl))
            FunctionKind = 3;

          Diag(ReturnLoc, D)
            << CurDecl->getDeclName() << FunctionKind
            << RetValExp->getSourceRange();
        }
      }

      if (RetValExp) {
        ExprResult ER = ActOnFinishFullExpr(RetValExp, ReturnLoc);
        if (ER.isInvalid())
          return StmtError();
        RetValExp = ER.get();
      }
    }

    Result = new (Context) ReturnStmt(ReturnLoc, RetValExp, nullptr);
  } else if (!RetValExp && !HasDependentReturnType) {
    FunctionDecl *FD = getCurFunctionDecl();

    unsigned DiagID;
    if (getLangOpts().CPlusPlus11 && FD && FD->isConstexpr()) {
      // C++11 [stmt.return]p2
      DiagID = diag::err_constexpr_return_missing_expr;
      FD->setInvalidDecl();
    } else if (getLangOpts().C99) {
      // C99 6.8.6.4p1 (ext_ since GCC warns)
      DiagID = diag::ext_return_missing_expr;
    } else {
      // C90 6.6.6.4p4
      DiagID = diag::warn_return_missing_expr;
    }

    if (FD)
      Diag(ReturnLoc, DiagID) << FD->getIdentifier() << 0/*fn*/;
    else
      Diag(ReturnLoc, DiagID) << getCurMethodDecl()->getDeclName() << 1/*meth*/;

    Result = new (Context) ReturnStmt(ReturnLoc);
  } else {
    assert(RetValExp || HasDependentReturnType);
    const VarDecl *NRVOCandidate = nullptr;

    QualType RetType = RelatedRetType.isNull() ? FnRetType : RelatedRetType;

    // C99 6.8.6.4p3(136): The return statement is not an assignment. The
    // overlap restriction of subclause 6.5.16.1 does not apply to the case of
    // function return.

    // In C++ the return statement is handled via a copy initialization,
    // the C version of which boils down to CheckSingleAssignmentConstraints.
    if (RetValExp)
      NRVOCandidate = getCopyElisionCandidate(FnRetType, RetValExp, false);
    if (!HasDependentReturnType && !RetValExp->isTypeDependent()) {
      // we have a non-void function with an expression, continue checking
      InitializedEntity Entity = InitializedEntity::InitializeResult(ReturnLoc,
                                                                     RetType,
                                                      NRVOCandidate != nullptr);
      ExprResult Res = PerformMoveOrCopyInitialization(Entity, NRVOCandidate,
                                                       RetType, RetValExp);
      if (Res.isInvalid()) {
        // FIXME: Clean up temporaries here anyway?
        return StmtError();
      }
      RetValExp = Res.getAs<Expr>();

      // If we have a related result type, we need to implicitly
      // convert back to the formal result type.  We can't pretend to
      // initialize the result again --- we might end double-retaining
      // --- so instead we initialize a notional temporary.
      if (!RelatedRetType.isNull()) {
        Entity = InitializedEntity::InitializeRelatedResult(getCurMethodDecl(),
                                                            FnRetType);
        Res = PerformCopyInitialization(Entity, ReturnLoc, RetValExp);
        if (Res.isInvalid()) {
          // FIXME: Clean up temporaries here anyway?
          return StmtError();
        }
        RetValExp = Res.getAs<Expr>();
      }

      CheckReturnValExpr(RetValExp, FnRetType, ReturnLoc, isObjCMethod, Attrs,
                         getCurFunctionDecl());
    }

    if (RetValExp) {
      ExprResult ER = ActOnFinishFullExpr(RetValExp, ReturnLoc);
      if (ER.isInvalid())
        return StmtError();
      RetValExp = ER.get();
    }
    Result = new (Context) ReturnStmt(ReturnLoc, RetValExp, NRVOCandidate);
  }

  // If we need to check for the named return value optimization, save the
  // return statement in our scope for later processing.
  if (Result->getNRVOCandidate())
    FunctionScopes.back()->Returns.push_back(Result);

  return Result;
}

StmtResult
Sema::ActOnObjCAtCatchStmt(SourceLocation AtLoc,
                           SourceLocation RParen, Decl *Parm,
                           Stmt *Body) {
  VarDecl *Var = cast_or_null<VarDecl>(Parm);
  if (Var && Var->isInvalidDecl())
    return StmtError();

  return new (Context) ObjCAtCatchStmt(AtLoc, RParen, Var, Body);
}

StmtResult
Sema::ActOnObjCAtFinallyStmt(SourceLocation AtLoc, Stmt *Body) {
  return new (Context) ObjCAtFinallyStmt(AtLoc, Body);
}

StmtResult
Sema::ActOnObjCAtTryStmt(SourceLocation AtLoc, Stmt *Try,
                         MultiStmtArg CatchStmts, Stmt *Finally) {
  if (!getLangOpts().ObjCExceptions)
    Diag(AtLoc, diag::err_objc_exceptions_disabled) << "@try";

  getCurFunction()->setHasBranchProtectedScope();
  unsigned NumCatchStmts = CatchStmts.size();
  return ObjCAtTryStmt::Create(Context, AtLoc, Try, CatchStmts.data(),
                               NumCatchStmts, Finally);
}

StmtResult Sema::BuildObjCAtThrowStmt(SourceLocation AtLoc, Expr *Throw) {
  if (Throw) {
    ExprResult Result = DefaultLvalueConversion(Throw);
    if (Result.isInvalid())
      return StmtError();

    Result = ActOnFinishFullExpr(Result.get());
    if (Result.isInvalid())
      return StmtError();
    Throw = Result.get();

    QualType ThrowType = Throw->getType();
    // Make sure the expression type is an ObjC pointer or "void *".
    if (!ThrowType->isDependentType() &&
        !ThrowType->isObjCObjectPointerType()) {
      const PointerType *PT = ThrowType->getAs<PointerType>();
      if (!PT || !PT->getPointeeType()->isVoidType())
        return StmtError(Diag(AtLoc, diag::error_objc_throw_expects_object)
                         << Throw->getType() << Throw->getSourceRange());
    }
  }

  return new (Context) ObjCAtThrowStmt(AtLoc, Throw);
}

StmtResult
Sema::ActOnObjCAtThrowStmt(SourceLocation AtLoc, Expr *Throw,
                           Scope *CurScope) {
  if (!getLangOpts().ObjCExceptions)
    Diag(AtLoc, diag::err_objc_exceptions_disabled) << "@throw";

  if (!Throw) {
    // @throw without an expression designates a rethrow (which must occur
    // in the context of an @catch clause).
    Scope *AtCatchParent = CurScope;
    while (AtCatchParent && !AtCatchParent->isAtCatchScope())
      AtCatchParent = AtCatchParent->getParent();
    if (!AtCatchParent)
      return StmtError(Diag(AtLoc, diag::error_rethrow_used_outside_catch));
  }
  return BuildObjCAtThrowStmt(AtLoc, Throw);
}

ExprResult
Sema::ActOnObjCAtSynchronizedOperand(SourceLocation atLoc, Expr *operand) {
  ExprResult result = DefaultLvalueConversion(operand);
  if (result.isInvalid())
    return ExprError();
  operand = result.get();

  // Make sure the expression type is an ObjC pointer or "void *".
  QualType type = operand->getType();
  if (!type->isDependentType() &&
      !type->isObjCObjectPointerType()) {
    const PointerType *pointerType = type->getAs<PointerType>();
    if (!pointerType || !pointerType->getPointeeType()->isVoidType()) {
      if (getLangOpts().CPlusPlus) {
        if (RequireCompleteType(atLoc, type,
                                diag::err_incomplete_receiver_type))
          return Diag(atLoc, diag::error_objc_synchronized_expects_object)
                   << type << operand->getSourceRange();

        ExprResult result = PerformContextuallyConvertToObjCPointer(operand);
        if (!result.isUsable())
          return Diag(atLoc, diag::error_objc_synchronized_expects_object)
                   << type << operand->getSourceRange();

        operand = result.get();
      } else {
          return Diag(atLoc, diag::error_objc_synchronized_expects_object)
                   << type << operand->getSourceRange();
      }
    }
  }

  // The operand to @synchronized is a full-expression.
  return ActOnFinishFullExpr(operand);
}

StmtResult
Sema::ActOnObjCAtSynchronizedStmt(SourceLocation AtLoc, Expr *SyncExpr,
                                  Stmt *SyncBody) {
  // We can't jump into or indirect-jump out of a @synchronized block.
  getCurFunction()->setHasBranchProtectedScope();
  return new (Context) ObjCAtSynchronizedStmt(AtLoc, SyncExpr, SyncBody);
}

/// ActOnCXXCatchBlock - Takes an exception declaration and a handler block
/// and creates a proper catch handler from them.
StmtResult
Sema::ActOnCXXCatchBlock(SourceLocation CatchLoc, Decl *ExDecl,
                         Stmt *HandlerBlock) {
  // There's nothing to test that ActOnExceptionDecl didn't already test.
  return new (Context)
      CXXCatchStmt(CatchLoc, cast_or_null<VarDecl>(ExDecl), HandlerBlock);
}

StmtResult
Sema::ActOnObjCAutoreleasePoolStmt(SourceLocation AtLoc, Stmt *Body) {
  getCurFunction()->setHasBranchProtectedScope();
  return new (Context) ObjCAutoreleasePoolStmt(AtLoc, Body);
}

namespace {
class CatchHandlerType {
  QualType QT;
  unsigned IsPointer : 1;

  // This is a special constructor to be used only with DenseMapInfo's
  // getEmptyKey() and getTombstoneKey() functions.
  friend struct llvm::DenseMapInfo<CatchHandlerType>;
  enum Unique { ForDenseMap };
  CatchHandlerType(QualType QT, Unique) : QT(QT), IsPointer(false) {}

public:
  /// Used when creating a CatchHandlerType from a handler type; will determine
  /// whether the type is a pointer or reference and will strip off the top
  /// level pointer and cv-qualifiers.
  CatchHandlerType(QualType Q) : QT(Q), IsPointer(false) {
    if (QT->isPointerType())
      IsPointer = true;

    if (IsPointer || QT->isReferenceType())
      QT = QT->getPointeeType();
    QT = QT.getUnqualifiedType();
  }

  /// Used when creating a CatchHandlerType from a base class type; pretends the
  /// type passed in had the pointer qualifier, does not need to get an
  /// unqualified type.
  CatchHandlerType(QualType QT, bool IsPointer)
      : QT(QT), IsPointer(IsPointer) {}

  QualType underlying() const { return QT; }
  bool isPointer() const { return IsPointer; }

  friend bool operator==(const CatchHandlerType &LHS,
                         const CatchHandlerType &RHS) {
    // If the pointer qualification does not match, we can return early.
    if (LHS.IsPointer != RHS.IsPointer)
      return false;
    // Otherwise, check the underlying type without cv-qualifiers.
    return LHS.QT == RHS.QT;
  }
};
} // namespace

namespace llvm {
template <> struct DenseMapInfo<CatchHandlerType> {
  static CatchHandlerType getEmptyKey() {
    return CatchHandlerType(DenseMapInfo<QualType>::getEmptyKey(),
                       CatchHandlerType::ForDenseMap);
  }

  static CatchHandlerType getTombstoneKey() {
    return CatchHandlerType(DenseMapInfo<QualType>::getTombstoneKey(),
                       CatchHandlerType::ForDenseMap);
  }

  static unsigned getHashValue(const CatchHandlerType &Base) {
    return DenseMapInfo<QualType>::getHashValue(Base.underlying());
  }

  static bool isEqual(const CatchHandlerType &LHS,
                      const CatchHandlerType &RHS) {
    return LHS == RHS;
  }
};

// It's OK to treat CatchHandlerType as a POD type.
template <> struct isPodLike<CatchHandlerType> {
  static const bool value = true;
};
}

namespace {
class CatchTypePublicBases {
  ASTContext &Ctx;
  const llvm::DenseMap<CatchHandlerType, CXXCatchStmt *> &TypesToCheck;
  const bool CheckAgainstPointer;

  CXXCatchStmt *FoundHandler;
  CanQualType FoundHandlerType;

public:
  CatchTypePublicBases(
      ASTContext &Ctx,
      const llvm::DenseMap<CatchHandlerType, CXXCatchStmt *> &T, bool C)
      : Ctx(Ctx), TypesToCheck(T), CheckAgainstPointer(C),
        FoundHandler(nullptr) {}

  CXXCatchStmt *getFoundHandler() const { return FoundHandler; }
  CanQualType getFoundHandlerType() const { return FoundHandlerType; }

  static bool FindPublicBasesOfType(const CXXBaseSpecifier *S, CXXBasePath &,
                                    void *User) {
    auto &PBOT = *reinterpret_cast<CatchTypePublicBases *>(User);
    if (S->getAccessSpecifier() == AccessSpecifier::AS_public) {
      CatchHandlerType Check(S->getType(), PBOT.CheckAgainstPointer);
      auto M = PBOT.TypesToCheck;
      auto I = M.find(Check);
      if (I != M.end()) {
        PBOT.FoundHandler = I->second;
        PBOT.FoundHandlerType = PBOT.Ctx.getCanonicalType(S->getType());
        return true;
      }
    }
    return false;
  }
};
}

/// ActOnCXXTryBlock - Takes a try compound-statement and a number of
/// handlers and creates a try statement from them.
StmtResult Sema::ActOnCXXTryBlock(SourceLocation TryLoc, Stmt *TryBlock,
                                  ArrayRef<Stmt *> Handlers) {
  // Don't report an error if 'try' is used in system headers.
  if (!getLangOpts().CXXExceptions &&
      !getSourceManager().isInSystemHeader(TryLoc))
    Diag(TryLoc, diag::err_exceptions_disabled) << "try";

  if (getCurScope() && getCurScope()->isOpenMPSimdDirectiveScope())
    Diag(TryLoc, diag::err_omp_simd_region_cannot_use_stmt) << "try";

  sema::FunctionScopeInfo *FSI = getCurFunction();

  // C++ try is incompatible with SEH __try.
  if (!getLangOpts().Borland && FSI->FirstSEHTryLoc.isValid()) {
    Diag(TryLoc, diag::err_mixing_cxx_try_seh_try);
    Diag(FSI->FirstSEHTryLoc, diag::note_conflicting_try_here) << "'__try'";
  }

  const unsigned NumHandlers = Handlers.size();
  assert(!Handlers.empty() &&
         "The parser shouldn't call this if there are no handlers.");

  llvm::DenseMap<CatchHandlerType, CXXCatchStmt *> HandledTypes;
  for (unsigned i = 0; i < NumHandlers; ++i) {
    CXXCatchStmt *H = cast<CXXCatchStmt>(Handlers[i]);

    // Diagnose when the handler is a catch-all handler, but it isn't the last
    // handler for the try block. [except.handle]p5. Also, skip exception
    // declarations that are invalid, since we can't usefully report on them.
    if (!H->getExceptionDecl()) {
      if (i < NumHandlers - 1)
        return StmtError(Diag(H->getLocStart(), diag::err_early_catch_all));
      continue;
    } else if (H->getExceptionDecl()->isInvalidDecl())
      continue;

    // Walk the type hierarchy to diagnose when this type has already been
    // handled (duplication), or cannot be handled (derivation inversion). We
    // ignore top-level cv-qualifiers, per [except.handle]p3
    CatchHandlerType HandlerCHT =
        (QualType)Context.getCanonicalType(H->getCaughtType());

    // We can ignore whether the type is a reference or a pointer; we need the
    // underlying declaration type in order to get at the underlying record
    // decl, if there is one.
    QualType Underlying = HandlerCHT.underlying();
    if (auto *RD = Underlying->getAsCXXRecordDecl()) {
      if (!RD->hasDefinition())
        continue;
      // Check that none of the public, unambiguous base classes are in the
      // map ([except.handle]p1). Give the base classes the same pointer
      // qualification as the original type we are basing off of. This allows
      // comparison against the handler type using the same top-level pointer
      // as the original type.
      CXXBasePaths Paths;
      Paths.setOrigin(RD);
      CatchTypePublicBases CTPB(Context, HandledTypes, HandlerCHT.isPointer());
      if (RD->lookupInBases(CatchTypePublicBases::FindPublicBasesOfType, &CTPB,
                            Paths)) {
        const CXXCatchStmt *Problem = CTPB.getFoundHandler();
        if (!Paths.isAmbiguous(CTPB.getFoundHandlerType())) {
          Diag(H->getExceptionDecl()->getTypeSpecStartLoc(),
               diag::warn_exception_caught_by_earlier_handler)
              << H->getCaughtType();
          Diag(Problem->getExceptionDecl()->getTypeSpecStartLoc(),
                diag::note_previous_exception_handler)
              << Problem->getCaughtType();
        }
      }
    }

    // Add the type the list of ones we have handled; diagnose if we've already
    // handled it.
    auto R = HandledTypes.insert(std::make_pair(H->getCaughtType(), H));
    if (!R.second) {
      const CXXCatchStmt *Problem = R.first->second;
      Diag(H->getExceptionDecl()->getTypeSpecStartLoc(),
           diag::warn_exception_caught_by_earlier_handler)
          << H->getCaughtType();
      Diag(Problem->getExceptionDecl()->getTypeSpecStartLoc(),
           diag::note_previous_exception_handler)
          << Problem->getCaughtType();
    }
  }

  FSI->setHasCXXTry(TryLoc);

  return CXXTryStmt::Create(Context, TryLoc, TryBlock, Handlers);
}

StmtResult Sema::ActOnSEHTryBlock(bool IsCXXTry, SourceLocation TryLoc,
                                  Stmt *TryBlock, Stmt *Handler) {
  assert(TryBlock && Handler);

  sema::FunctionScopeInfo *FSI = getCurFunction();

  // SEH __try is incompatible with C++ try. Borland appears to support this,
  // however.
  if (!getLangOpts().Borland) {
    if (FSI->FirstCXXTryLoc.isValid()) {
      Diag(TryLoc, diag::err_mixing_cxx_try_seh_try);
      Diag(FSI->FirstCXXTryLoc, diag::note_conflicting_try_here) << "'try'";
    }
  }

  FSI->setHasSEHTry(TryLoc);

  // Reject __try in Obj-C methods, blocks, and captured decls, since we don't
  // track if they use SEH.
  DeclContext *DC = CurContext;
  while (DC && !DC->isFunctionOrMethod())
    DC = DC->getParent();
  FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(DC);
  if (FD)
    FD->setUsesSEHTry(true);
  else
    Diag(TryLoc, diag::err_seh_try_outside_functions);

  // Reject __try on unsupported targets.
  if (!Context.getTargetInfo().isSEHTrySupported())
    Diag(TryLoc, diag::err_seh_try_unsupported);

  return SEHTryStmt::Create(Context, IsCXXTry, TryLoc, TryBlock, Handler);
}

StmtResult
Sema::ActOnSEHExceptBlock(SourceLocation Loc,
                          Expr *FilterExpr,
                          Stmt *Block) {
  assert(FilterExpr && Block);

  if(!FilterExpr->getType()->isIntegerType()) {
    return StmtError(Diag(FilterExpr->getExprLoc(),
                     diag::err_filter_expression_integral)
                     << FilterExpr->getType());
  }

  return SEHExceptStmt::Create(Context,Loc,FilterExpr,Block);
}

void Sema::ActOnStartSEHFinallyBlock() {
  CurrentSEHFinally.push_back(CurScope);
}

void Sema::ActOnAbortSEHFinallyBlock() {
  CurrentSEHFinally.pop_back();
}

StmtResult Sema::ActOnFinishSEHFinallyBlock(SourceLocation Loc, Stmt *Block) {
  assert(Block);
  CurrentSEHFinally.pop_back();
  return SEHFinallyStmt::Create(Context, Loc, Block);
}

StmtResult
Sema::ActOnSEHLeaveStmt(SourceLocation Loc, Scope *CurScope) {
  Scope *SEHTryParent = CurScope;
  while (SEHTryParent && !SEHTryParent->isSEHTryScope())
    SEHTryParent = SEHTryParent->getParent();
  if (!SEHTryParent)
    return StmtError(Diag(Loc, diag::err_ms___leave_not_in___try));
  CheckJumpOutOfSEHFinally(*this, Loc, *SEHTryParent);

  return new (Context) SEHLeaveStmt(Loc);
}

StmtResult Sema::BuildMSDependentExistsStmt(SourceLocation KeywordLoc,
                                            bool IsIfExists,
                                            NestedNameSpecifierLoc QualifierLoc,
                                            DeclarationNameInfo NameInfo,
                                            Stmt *Nested)
{
  return new (Context) MSDependentExistsStmt(KeywordLoc, IsIfExists,
                                             QualifierLoc, NameInfo,
                                             cast<CompoundStmt>(Nested));
}


StmtResult Sema::ActOnMSDependentExistsStmt(SourceLocation KeywordLoc,
                                            bool IsIfExists,
                                            CXXScopeSpec &SS,
                                            UnqualifiedId &Name,
                                            Stmt *Nested) {
  return BuildMSDependentExistsStmt(KeywordLoc, IsIfExists,
                                    SS.getWithLocInContext(Context),
                                    GetNameFromUnqualifiedId(Name),
                                    Nested);
}

RecordDecl*
Sema::CreateCapturedStmtRecordDecl(CapturedDecl *&CD, SourceLocation Loc,
                                   unsigned NumParams) {
  DeclContext *DC = CurContext;
  while (!(DC->isFunctionOrMethod() || DC->isRecord() || DC->isFileContext()))
    DC = DC->getParent();

  RecordDecl *RD = nullptr;
  if (getLangOpts().CPlusPlus)
    RD = CXXRecordDecl::Create(Context, TTK_Struct, DC, Loc, Loc,
                               /*Id=*/nullptr);
  else
    RD = RecordDecl::Create(Context, TTK_Struct, DC, Loc, Loc, /*Id=*/nullptr);

  RD->setCapturedRecord();
  DC->addDecl(RD);
  RD->setImplicit();
  RD->startDefinition();

  assert(NumParams > 0 && "CapturedStmt requires context parameter");
  CD = CapturedDecl::Create(Context, CurContext, NumParams);
  DC->addDecl(CD);
  return RD;
}

static void buildCapturedStmtCaptureList(
    SmallVectorImpl<CapturedStmt::Capture> &Captures,
    SmallVectorImpl<Expr *> &CaptureInits,
    ArrayRef<CapturingScopeInfo::Capture> Candidates) {

  typedef ArrayRef<CapturingScopeInfo::Capture>::const_iterator CaptureIter;
  for (CaptureIter Cap = Candidates.begin(); Cap != Candidates.end(); ++Cap) {

    if (Cap->isThisCapture()) {
      Captures.push_back(CapturedStmt::Capture(Cap->getLocation(),
                                               CapturedStmt::VCK_This));
      CaptureInits.push_back(Cap->getInitExpr());
      continue;
    } else if (Cap->isVLATypeCapture()) {
      Captures.push_back(
          CapturedStmt::Capture(Cap->getLocation(), CapturedStmt::VCK_VLAType));
      CaptureInits.push_back(nullptr);
      continue;
    }

    assert(Cap->isReferenceCapture() &&
           "non-reference capture not yet implemented");

    Captures.push_back(CapturedStmt::Capture(Cap->getLocation(),
                                             CapturedStmt::VCK_ByRef,
                                             Cap->getVariable()));
    CaptureInits.push_back(Cap->getInitExpr());
  }
}

void Sema::ActOnCapturedRegionStart(SourceLocation Loc, Scope *CurScope,
                                    CapturedRegionKind Kind,
                                    unsigned NumParams) {
  CapturedDecl *CD = nullptr;
  RecordDecl *RD = CreateCapturedStmtRecordDecl(CD, Loc, NumParams);

  // Build the context parameter
  DeclContext *DC = CapturedDecl::castToDeclContext(CD);
  IdentifierInfo *ParamName = &Context.Idents.get("__context");
  QualType ParamType = Context.getPointerType(Context.getTagDeclType(RD));
  ImplicitParamDecl *Param
    = ImplicitParamDecl::Create(Context, DC, Loc, ParamName, ParamType);
  DC->addDecl(Param);

  CD->setContextParam(0, Param);

  // Enter the capturing scope for this captured region.
  PushCapturedRegionScope(CurScope, CD, RD, Kind);

  if (CurScope)
    PushDeclContext(CurScope, CD);
  else
    CurContext = CD;

  PushExpressionEvaluationContext(PotentiallyEvaluated);
}

void Sema::ActOnCapturedRegionStart(SourceLocation Loc, Scope *CurScope,
                                    CapturedRegionKind Kind,
                                    ArrayRef<CapturedParamNameType> Params) {
  CapturedDecl *CD = nullptr;
  RecordDecl *RD = CreateCapturedStmtRecordDecl(CD, Loc, Params.size());

  // Build the context parameter
  DeclContext *DC = CapturedDecl::castToDeclContext(CD);
  bool ContextIsFound = false;
  unsigned ParamNum = 0;
  for (ArrayRef<CapturedParamNameType>::iterator I = Params.begin(),
                                                 E = Params.end();
       I != E; ++I, ++ParamNum) {
    if (I->second.isNull()) {
      assert(!ContextIsFound &&
             "null type has been found already for '__context' parameter");
      IdentifierInfo *ParamName = &Context.Idents.get("__context");
      QualType ParamType = Context.getPointerType(Context.getTagDeclType(RD));
      ImplicitParamDecl *Param
        = ImplicitParamDecl::Create(Context, DC, Loc, ParamName, ParamType);
      DC->addDecl(Param);
      CD->setContextParam(ParamNum, Param);
      ContextIsFound = true;
    } else {
      IdentifierInfo *ParamName = &Context.Idents.get(I->first);
      ImplicitParamDecl *Param
        = ImplicitParamDecl::Create(Context, DC, Loc, ParamName, I->second);
      DC->addDecl(Param);
      CD->setParam(ParamNum, Param);
    }
  }
  assert(ContextIsFound && "no null type for '__context' parameter");
  if (!ContextIsFound) {
    // Add __context implicitly if it is not specified.
    IdentifierInfo *ParamName = &Context.Idents.get("__context");
    QualType ParamType = Context.getPointerType(Context.getTagDeclType(RD));
    ImplicitParamDecl *Param =
        ImplicitParamDecl::Create(Context, DC, Loc, ParamName, ParamType);
    DC->addDecl(Param);
    CD->setContextParam(ParamNum, Param);
  }
  // Enter the capturing scope for this captured region.
  PushCapturedRegionScope(CurScope, CD, RD, Kind);

  if (CurScope)
    PushDeclContext(CurScope, CD);
  else
    CurContext = CD;

  PushExpressionEvaluationContext(PotentiallyEvaluated);
}

void Sema::ActOnCapturedRegionError() {
  DiscardCleanupsInEvaluationContext();
  PopExpressionEvaluationContext();

  CapturedRegionScopeInfo *RSI = getCurCapturedRegion();
  RecordDecl *Record = RSI->TheRecordDecl;
  Record->setInvalidDecl();

  SmallVector<Decl*, 4> Fields(Record->fields());
  ActOnFields(/*Scope=*/nullptr, Record->getLocation(), Record, Fields,
              SourceLocation(), SourceLocation(), /*AttributeList=*/nullptr);

  PopDeclContext();
  PopFunctionScopeInfo();
}

StmtResult Sema::ActOnCapturedRegionEnd(Stmt *S) {
  CapturedRegionScopeInfo *RSI = getCurCapturedRegion();

  SmallVector<CapturedStmt::Capture, 4> Captures;
  SmallVector<Expr *, 4> CaptureInits;
  buildCapturedStmtCaptureList(Captures, CaptureInits, RSI->Captures);

  CapturedDecl *CD = RSI->TheCapturedDecl;
  RecordDecl *RD = RSI->TheRecordDecl;

  CapturedStmt *Res = CapturedStmt::Create(getASTContext(), S,
                                           RSI->CapRegionKind, Captures,
                                           CaptureInits, CD, RD);

  CD->setBody(Res->getCapturedStmt());
  RD->completeDefinition();

  DiscardCleanupsInEvaluationContext();
  PopExpressionEvaluationContext();

  PopDeclContext();
  PopFunctionScopeInfo();

  return Res;
}

// HLSL Change:  adding hlsl support
StmtResult Sema::ActOnHlslDiscardStmt(SourceLocation Loc)
{
  return new (Context)DiscardStmt(Loc);
}
