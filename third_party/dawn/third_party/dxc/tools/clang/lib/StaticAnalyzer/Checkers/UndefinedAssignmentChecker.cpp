//===--- UndefinedAssignmentChecker.h ---------------------------*- C++ -*--==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This defines UndefinedAssignmentChecker, a builtin check in ExprEngine that
// checks for assigning undefined values.
//
//===----------------------------------------------------------------------===//

#include "ClangSACheckers.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;

namespace {
class UndefinedAssignmentChecker
  : public Checker<check::Bind> {
  mutable std::unique_ptr<BugType> BT;

public:
  void checkBind(SVal location, SVal val, const Stmt *S,
                 CheckerContext &C) const;
};
}

void UndefinedAssignmentChecker::checkBind(SVal location, SVal val,
                                           const Stmt *StoreE,
                                           CheckerContext &C) const {
  if (!val.isUndef())
    return;

  // Do not report assignments of uninitialized values inside swap functions.
  // This should allow to swap partially uninitialized structs
  // (radar://14129997)
  if (const FunctionDecl *EnclosingFunctionDecl =
      dyn_cast<FunctionDecl>(C.getStackFrame()->getDecl()))
    if (C.getCalleeName(EnclosingFunctionDecl) == "swap")
      return;

  ExplodedNode *N = C.generateSink();

  if (!N)
    return;

  const char *str = "Assigned value is garbage or undefined";

  if (!BT)
    BT.reset(new BuiltinBug(this, str));

  // Generate a report for this bug.
  const Expr *ex = nullptr;

  while (StoreE) {
    if (const BinaryOperator *B = dyn_cast<BinaryOperator>(StoreE)) {
      if (B->isCompoundAssignmentOp()) {
        ProgramStateRef state = C.getState();
        if (state->getSVal(B->getLHS(), C.getLocationContext()).isUndef()) {
          str = "The left expression of the compound assignment is an "
                "uninitialized value. The computed value will also be garbage";
          ex = B->getLHS();
          break;
        }
      }

      ex = B->getRHS();
      break;
    }

    if (const DeclStmt *DS = dyn_cast<DeclStmt>(StoreE)) {
      const VarDecl *VD = dyn_cast<VarDecl>(DS->getSingleDecl());
      ex = VD->getInit();
    }

    break;
  }

  auto R = llvm::make_unique<BugReport>(*BT, str, N);
  if (ex) {
    R->addRange(ex->getSourceRange());
    bugreporter::trackNullOrUndefValue(N, ex, *R);
  }
  C.emitReport(std::move(R));
}

void ento::registerUndefinedAssignmentChecker(CheckerManager &mgr) {
  mgr.registerChecker<UndefinedAssignmentChecker>();
}
