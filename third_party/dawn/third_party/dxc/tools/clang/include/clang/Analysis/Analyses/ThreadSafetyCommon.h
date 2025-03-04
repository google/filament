//===- ThreadSafetyCommon.h ------------------------------------*- C++ --*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Parts of thread safety analysis that are not specific to thread safety
// itself have been factored into classes here, where they can be potentially
// used by other analyses.  Currently these include:
//
// * Generalize clang CFG visitors.
// * Conversion of the clang CFG to SSA form.
// * Translation of clang Exprs to TIL SExprs
//
// UNDER CONSTRUCTION.  USE AT YOUR OWN RISK.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_ANALYSIS_ANALYSES_THREADSAFETYCOMMON_H
#define LLVM_CLANG_ANALYSIS_ANALYSES_THREADSAFETYCOMMON_H

#include "clang/Analysis/Analyses/PostOrderCFGView.h"
#include "clang/Analysis/Analyses/ThreadSafetyTIL.h"
#include "clang/Analysis/Analyses/ThreadSafetyTraverse.h"
#include "clang/Analysis/AnalysisContext.h"
#include "clang/Basic/OperatorKinds.h"
#include <memory>
#include <ostream>
#include <sstream>
#include <vector>


namespace clang {
namespace threadSafety {


// Various helper functions on til::SExpr
namespace sx {

inline bool equals(const til::SExpr *E1, const til::SExpr *E2) {
  return til::EqualsComparator::compareExprs(E1, E2);
}

inline bool matches(const til::SExpr *E1, const til::SExpr *E2) {
  // We treat a top-level wildcard as the "univsersal" lock.
  // It matches everything for the purpose of checking locks, but not
  // for unlocking them.
  if (isa<til::Wildcard>(E1))
    return isa<til::Wildcard>(E2);
  if (isa<til::Wildcard>(E2))
    return isa<til::Wildcard>(E1);

  return til::MatchComparator::compareExprs(E1, E2);
}

inline bool partiallyMatches(const til::SExpr *E1, const til::SExpr *E2) {
  const auto *PE1 = dyn_cast_or_null<til::Project>(E1);
  if (!PE1)
    return false;
  const auto *PE2 = dyn_cast_or_null<til::Project>(E2);
  if (!PE2)
    return false;
  return PE1->clangDecl() == PE2->clangDecl();
}

inline std::string toString(const til::SExpr *E) {
  std::stringstream ss;
  til::StdPrinter::print(E, ss);
  return ss.str();
}

}  // end namespace sx



// This class defines the interface of a clang CFG Visitor.
// CFGWalker will invoke the following methods.
// Note that methods are not virtual; the visitor is templatized.
class CFGVisitor {
  // Enter the CFG for Decl D, and perform any initial setup operations.
  void enterCFG(CFG *Cfg, const NamedDecl *D, const CFGBlock *First) {}

  // Enter a CFGBlock.
  void enterCFGBlock(const CFGBlock *B) {}

  // Returns true if this visitor implements handlePredecessor
  bool visitPredecessors() { return true; }

  // Process a predecessor edge.
  void handlePredecessor(const CFGBlock *Pred) {}

  // Process a successor back edge to a previously visited block.
  void handlePredecessorBackEdge(const CFGBlock *Pred) {}

  // Called just before processing statements.
  void enterCFGBlockBody(const CFGBlock *B) {}

  // Process an ordinary statement.
  void handleStatement(const Stmt *S) {}

  // Process a destructor call
  void handleDestructorCall(const VarDecl *VD, const CXXDestructorDecl *DD) {}

  // Called after all statements have been handled.
  void exitCFGBlockBody(const CFGBlock *B) {}

  // Return true
  bool visitSuccessors() { return true; }

  // Process a successor edge.
  void handleSuccessor(const CFGBlock *Succ) {}

  // Process a successor back edge to a previously visited block.
  void handleSuccessorBackEdge(const CFGBlock *Succ) {}

  // Leave a CFGBlock.
  void exitCFGBlock(const CFGBlock *B) {}

  // Leave the CFG, and perform any final cleanup operations.
  void exitCFG(const CFGBlock *Last) {}
};


// Walks the clang CFG, and invokes methods on a given CFGVisitor.
class CFGWalker {
public:
  CFGWalker() : CFGraph(nullptr), ACtx(nullptr), SortedGraph(nullptr) {}

  // Initialize the CFGWalker.  This setup only needs to be done once, even
  // if there are multiple passes over the CFG.
  bool init(AnalysisDeclContext &AC) {
    ACtx = &AC;
    CFGraph = AC.getCFG();
    if (!CFGraph)
      return false;

    // Ignore anonymous functions.
    if (!dyn_cast_or_null<NamedDecl>(AC.getDecl()))
      return false;

    SortedGraph = AC.getAnalysis<PostOrderCFGView>();
    if (!SortedGraph)
      return false;

    return true;
  }

  // Traverse the CFG, calling methods on V as appropriate.
  template <class Visitor>
  void walk(Visitor &V) {
    PostOrderCFGView::CFGBlockSet VisitedBlocks(CFGraph);

    V.enterCFG(CFGraph, getDecl(), &CFGraph->getEntry());

    for (const auto *CurrBlock : *SortedGraph) {
      VisitedBlocks.insert(CurrBlock);

      V.enterCFGBlock(CurrBlock);

      // Process predecessors, handling back edges last
      if (V.visitPredecessors()) {
        SmallVector<CFGBlock*, 4> BackEdges;
        // Process successors
        for (CFGBlock::const_pred_iterator SI = CurrBlock->pred_begin(),
                                           SE = CurrBlock->pred_end();
             SI != SE; ++SI) {
          if (*SI == nullptr)
            continue;

          if (!VisitedBlocks.alreadySet(*SI)) {
            BackEdges.push_back(*SI);
            continue;
          }
          V.handlePredecessor(*SI);
        }

        for (auto *Blk : BackEdges)
          V.handlePredecessorBackEdge(Blk);
      }

      V.enterCFGBlockBody(CurrBlock);

      // Process statements
      for (const auto &BI : *CurrBlock) {
        switch (BI.getKind()) {
        case CFGElement::Statement: {
          V.handleStatement(BI.castAs<CFGStmt>().getStmt());
          break;
        }
        case CFGElement::AutomaticObjectDtor: {
          CFGAutomaticObjDtor AD = BI.castAs<CFGAutomaticObjDtor>();
          CXXDestructorDecl *DD = const_cast<CXXDestructorDecl*>(
              AD.getDestructorDecl(ACtx->getASTContext()));
          VarDecl *VD = const_cast<VarDecl*>(AD.getVarDecl());
          V.handleDestructorCall(VD, DD);
          break;
        }
        default:
          break;
        }
      }

      V.exitCFGBlockBody(CurrBlock);

      // Process successors, handling back edges first.
      if (V.visitSuccessors()) {
        SmallVector<CFGBlock*, 8> ForwardEdges;

        // Process successors
        for (CFGBlock::const_succ_iterator SI = CurrBlock->succ_begin(),
                                           SE = CurrBlock->succ_end();
             SI != SE; ++SI) {
          if (*SI == nullptr)
            continue;

          if (!VisitedBlocks.alreadySet(*SI)) {
            ForwardEdges.push_back(*SI);
            continue;
          }
          V.handleSuccessorBackEdge(*SI);
        }

        for (auto *Blk : ForwardEdges)
          V.handleSuccessor(Blk);
      }

      V.exitCFGBlock(CurrBlock);
    }
    V.exitCFG(&CFGraph->getExit());
  }

  const CFG *getGraph() const { return CFGraph; }
  CFG *getGraph() { return CFGraph; }

  const NamedDecl *getDecl() const {
    return dyn_cast<NamedDecl>(ACtx->getDecl());
  }

  const PostOrderCFGView *getSortedGraph() const { return SortedGraph; }

private:
  CFG *CFGraph;
  AnalysisDeclContext *ACtx;
  PostOrderCFGView *SortedGraph;
};




class CapabilityExpr {
  // TODO: move this back into ThreadSafety.cpp
  // This is specific to thread safety.  It is here because
  // translateAttrExpr needs it, but that should be moved too.

private:
  const til::SExpr* CapExpr;   ///< The capability expression.
  bool Negated;                ///< True if this is a negative capability

public:
  CapabilityExpr(const til::SExpr *E, bool Neg) : CapExpr(E), Negated(Neg) {}

  const til::SExpr* sexpr()    const { return CapExpr; }
  bool              negative() const { return Negated; }

  CapabilityExpr operator!() const {
    return CapabilityExpr(CapExpr, !Negated);
  }

  bool equals(const CapabilityExpr &other) const {
    return (Negated == other.Negated) && sx::equals(CapExpr, other.CapExpr);
  }

  bool matches(const CapabilityExpr &other) const {
    return (Negated == other.Negated) && sx::matches(CapExpr, other.CapExpr);
  }

  bool matchesUniv(const CapabilityExpr &CapE) const {
    return isUniversal() || matches(CapE);
  }

  bool partiallyMatches(const CapabilityExpr &other) const {
    return (Negated == other.Negated) &&
            sx::partiallyMatches(CapExpr, other.CapExpr);
  }

  const ValueDecl* valueDecl() const {
    if (Negated)
      return nullptr;
    if (auto *P = dyn_cast<til::Project>(CapExpr))
      return P->clangDecl();
    return nullptr;
  }

  std::string toString() const {
    if (Negated)
      return "!" + sx::toString(CapExpr);
    return sx::toString(CapExpr);
  }

  bool shouldIgnore() const { return CapExpr == nullptr; }

  bool isInvalid() const { return sexpr() && isa<til::Undefined>(sexpr()); }

  bool isUniversal() const { return sexpr() && isa<til::Wildcard>(sexpr()); }
};



// Translate clang::Expr to til::SExpr.
class SExprBuilder {
public:
  /// \brief Encapsulates the lexical context of a function call.  The lexical
  /// context includes the arguments to the call, including the implicit object
  /// argument.  When an attribute containing a mutex expression is attached to
  /// a method, the expression may refer to formal parameters of the method.
  /// Actual arguments must be substituted for formal parameters to derive
  /// the appropriate mutex expression in the lexical context where the function
  /// is called.  PrevCtx holds the context in which the arguments themselves
  /// should be evaluated; multiple calling contexts can be chained together
  /// by the lock_returned attribute.
  struct CallingContext {
    CallingContext  *Prev;      // The previous context; or 0 if none.
    const NamedDecl *AttrDecl;  // The decl to which the attr is attached.
    const Expr *SelfArg;        // Implicit object argument -- e.g. 'this'
    unsigned NumArgs;           // Number of funArgs
    const Expr *const *FunArgs; // Function arguments
    bool SelfArrow;             // is Self referred to with -> or .?

    CallingContext(CallingContext *P, const NamedDecl *D = nullptr)
        : Prev(P), AttrDecl(D), SelfArg(nullptr),
          NumArgs(0), FunArgs(nullptr), SelfArrow(false)
    {}
  };

  SExprBuilder(til::MemRegionRef A)
      : Arena(A), SelfVar(nullptr), Scfg(nullptr), CurrentBB(nullptr),
        CurrentBlockInfo(nullptr) {
    // FIXME: we don't always have a self-variable.
    SelfVar = new (Arena) til::Variable(nullptr);
    SelfVar->setKind(til::Variable::VK_SFun);
  }

  // Translate a clang expression in an attribute to a til::SExpr.
  // Constructs the context from D, DeclExp, and SelfDecl.
  CapabilityExpr translateAttrExpr(const Expr *AttrExp, const NamedDecl *D,
                                   const Expr *DeclExp, VarDecl *SelfD=nullptr);

  CapabilityExpr translateAttrExpr(const Expr *AttrExp, CallingContext *Ctx);

  // Translate a clang statement or expression to a TIL expression.
  // Also performs substitution of variables; Ctx provides the context.
  // Dispatches on the type of S.
  til::SExpr *translate(const Stmt *S, CallingContext *Ctx);
  til::SCFG  *buildCFG(CFGWalker &Walker);

  til::SExpr *lookupStmt(const Stmt *S);

  til::BasicBlock *lookupBlock(const CFGBlock *B) {
    return BlockMap[B->getBlockID()];
  }

  const til::SCFG *getCFG() const { return Scfg; }
  til::SCFG *getCFG() { return Scfg; }

private:
  til::SExpr *translateDeclRefExpr(const DeclRefExpr *DRE,
                                   CallingContext *Ctx) ;
  til::SExpr *translateCXXThisExpr(const CXXThisExpr *TE, CallingContext *Ctx);
  til::SExpr *translateMemberExpr(const MemberExpr *ME, CallingContext *Ctx);
  til::SExpr *translateCallExpr(const CallExpr *CE, CallingContext *Ctx,
                                const Expr *SelfE = nullptr);
  til::SExpr *translateCXXMemberCallExpr(const CXXMemberCallExpr *ME,
                                         CallingContext *Ctx);
  til::SExpr *translateCXXOperatorCallExpr(const CXXOperatorCallExpr *OCE,
                                           CallingContext *Ctx);
  til::SExpr *translateUnaryOperator(const UnaryOperator *UO,
                                     CallingContext *Ctx);
  til::SExpr *translateBinOp(til::TIL_BinaryOpcode Op,
                             const BinaryOperator *BO,
                             CallingContext *Ctx, bool Reverse = false);
  til::SExpr *translateBinAssign(til::TIL_BinaryOpcode Op,
                                 const BinaryOperator *BO,
                                 CallingContext *Ctx, bool Assign = false);
  til::SExpr *translateBinaryOperator(const BinaryOperator *BO,
                                      CallingContext *Ctx);
  til::SExpr *translateCastExpr(const CastExpr *CE, CallingContext *Ctx);
  til::SExpr *translateArraySubscriptExpr(const ArraySubscriptExpr *E,
                                          CallingContext *Ctx);
  til::SExpr *translateAbstractConditionalOperator(
      const AbstractConditionalOperator *C, CallingContext *Ctx);

  til::SExpr *translateDeclStmt(const DeclStmt *S, CallingContext *Ctx);

  // Map from statements in the clang CFG to SExprs in the til::SCFG.
  typedef llvm::DenseMap<const Stmt*, til::SExpr*> StatementMap;

  // Map from clang local variables to indices in a LVarDefinitionMap.
  typedef llvm::DenseMap<const ValueDecl *, unsigned> LVarIndexMap;

  // Map from local variable indices to SSA variables (or constants).
  typedef std::pair<const ValueDecl *, til::SExpr *> NameVarPair;
  typedef CopyOnWriteVector<NameVarPair> LVarDefinitionMap;

  struct BlockInfo {
    LVarDefinitionMap ExitMap;
    bool HasBackEdges;
    unsigned UnprocessedSuccessors;   // Successors yet to be processed
    unsigned ProcessedPredecessors;   // Predecessors already processed

    BlockInfo()
        : HasBackEdges(false), UnprocessedSuccessors(0),
          ProcessedPredecessors(0) {}
    BlockInfo(BlockInfo &&RHS)
        : ExitMap(std::move(RHS.ExitMap)),
          HasBackEdges(RHS.HasBackEdges),
          UnprocessedSuccessors(RHS.UnprocessedSuccessors),
          ProcessedPredecessors(RHS.ProcessedPredecessors) {}

    BlockInfo &operator=(BlockInfo &&RHS) {
      if (this != &RHS) {
        ExitMap = std::move(RHS.ExitMap);
        HasBackEdges = RHS.HasBackEdges;
        UnprocessedSuccessors = RHS.UnprocessedSuccessors;
        ProcessedPredecessors = RHS.ProcessedPredecessors;
      }
      return *this;
    }

  private:
    BlockInfo(const BlockInfo &) = delete;
    void operator=(const BlockInfo &) = delete;
  };

  // We implement the CFGVisitor API
  friend class CFGWalker;

  void enterCFG(CFG *Cfg, const NamedDecl *D, const CFGBlock *First);
  void enterCFGBlock(const CFGBlock *B);
  bool visitPredecessors() { return true; }
  void handlePredecessor(const CFGBlock *Pred);
  void handlePredecessorBackEdge(const CFGBlock *Pred);
  void enterCFGBlockBody(const CFGBlock *B);
  void handleStatement(const Stmt *S);
  void handleDestructorCall(const VarDecl *VD, const CXXDestructorDecl *DD);
  void exitCFGBlockBody(const CFGBlock *B);
  bool visitSuccessors() { return true; }
  void handleSuccessor(const CFGBlock *Succ);
  void handleSuccessorBackEdge(const CFGBlock *Succ);
  void exitCFGBlock(const CFGBlock *B);
  void exitCFG(const CFGBlock *Last);

  void insertStmt(const Stmt *S, til::SExpr *E) {
    SMap.insert(std::make_pair(S, E));
  }
  til::SExpr *getCurrentLVarDefinition(const ValueDecl *VD);

  til::SExpr *addStatement(til::SExpr *E, const Stmt *S,
                           const ValueDecl *VD = nullptr);
  til::SExpr *lookupVarDecl(const ValueDecl *VD);
  til::SExpr *addVarDecl(const ValueDecl *VD, til::SExpr *E);
  til::SExpr *updateVarDecl(const ValueDecl *VD, til::SExpr *E);

  void makePhiNodeVar(unsigned i, unsigned NPreds, til::SExpr *E);
  void mergeEntryMap(LVarDefinitionMap Map);
  void mergeEntryMapBackEdge();
  void mergePhiNodesBackEdge(const CFGBlock *Blk);

private:
  // Set to true when parsing capability expressions, which get translated
  // inaccurately in order to hack around smart pointers etc.
  static const bool CapabilityExprMode = true;

  til::MemRegionRef Arena;
  til::Variable *SelfVar;       // Variable to use for 'this'.  May be null.

  til::SCFG *Scfg;
  StatementMap SMap;                       // Map from Stmt to TIL Variables
  LVarIndexMap LVarIdxMap;                 // Indices of clang local vars.
  std::vector<til::BasicBlock *> BlockMap; // Map from clang to til BBs.
  std::vector<BlockInfo> BBInfo;           // Extra information per BB.
                                           // Indexed by clang BlockID.

  LVarDefinitionMap CurrentLVarMap;
  std::vector<til::Phi*>   CurrentArguments;
  std::vector<til::SExpr*> CurrentInstructions;
  std::vector<til::Phi*>   IncompleteArgs;
  til::BasicBlock *CurrentBB;
  BlockInfo *CurrentBlockInfo;
};


// Dump an SCFG to llvm::errs().
void printSCFG(CFGWalker &Walker);


} // end namespace threadSafety

} // end namespace clang

#endif  // LLVM_CLANG_THREAD_SAFETY_COMMON_H
