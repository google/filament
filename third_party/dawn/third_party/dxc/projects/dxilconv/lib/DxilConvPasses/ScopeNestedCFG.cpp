///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ScopeNestedCFG.cpp                                                        //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements the ScopeNested CFG Transformation.                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "DxilConvPasses/ScopeNestedCFG.h"
#include "dxc/Support/Global.h"
#include "llvm/Analysis/ReducibilityAnalysis.h"

#include "llvm/ADT/BitVector.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <algorithm>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace llvm;
using std::pair;
using std::set;
using std::shared_ptr;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_set;
using std::vector;

#define SNCFG_DBG 0

//===----------------------------------------------------------------------===//
//                    ScopeNested CFG Transformation
//===----------------------------------------------------------------------===//
//
// The transformation requires the following LLVM passes:
// -simplifycfg -loop-simplify -reg2mem_hlsl to be run on each function.
// This is to rely on LLVM standard loop analysis info and to be able to clone
// basic blocks, if necessary.
//
// The core of the algorithm is the transformation of an acyclic CFG region into
// a region that corresponds to control-flow with structured nested scopes.
// Scoping information is conveyed by inserting helper basic blocks (BBs) and
// annotating their terminators with the corresponding "dx.BranchKind" metadata
// (see BranchKind enum in ScopeNestedCFG.h) to make it possible for clients
// to recover the structure after the pass.
//
// To handle loops, the algorithm transforms each loop nest from the deepest
// nested loop upwards. Each transformed loop is conceptually treated as a
// single loop node, defined by LoopEntry and LoopExit (if there is an exit) BB
// pair. A loop is made acyclic region by "removing" its backedge. The process
// finishes with transforming function body starting from the entry basic block
// (BB).
//
// Tranforming an acyclic region.
// 1. Topological ordering is done by DFS graph traversal.
//    - Each BB is assigned an ID
//    - For each BB, a set of all reachable BBs is computed.
// 2. Using topological block order, reachable merge points are propagated along
// predecessors,
//    and for each split point (if, switch), the closest merge point is
//    determined, by intersecting reachable merge point sets of the successor
//    BBs. A switch uses a heuristic that picks the closest merge point
//    reachable via majority of successors.
// 3. The CFG is tranformed to have scope-nested structure. Here are some
// interesting details:
//    - A custom scope-stack is used to recover scopes.
//    - The tranformation operates on the original CFG, with its original
//    structure preserved
//      during transformation until the very last moment.
//      Cloned BBs are inserted into the CFG and their terminators temporarily
//      form self-loops. The implementation maintains a set of edges to
//      instantiate as the final, which destroys the original CFG.
//    - Loops are treated as a single loop node identified via two BBs:
//    LoopBegin->LoopEnd.
//      There is a subroutine to clone an entire loop, if there is a need.
//    - The branches are annotated with dx.BranchKind.
//    - For a switch scope, the tranformation identifies switch breaks, and then
//    recomputes merge points
//      for scopes nested inside the switch scope.
//

namespace ScopeNestedCFGNS {

class ScopeNestedCFG : public FunctionPass {
public:
  static char ID;

  explicit ScopeNestedCFG() : FunctionPass(ID), m_HelperExitCondIndex(0) {
    Clear();
    initializeScopeNestedCFGPass(*PassRegistry::getPassRegistry());
  }

  virtual bool runOnFunction(Function &F);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequiredID(ReducibilityAnalysisID);
    AU.addRequired<LoopInfoWrapperPass>();
  }

private:
  struct LoopItem {
    BasicBlock *pLB; // Loop begin
    BasicBlock *pLE; // Loop end
    BasicBlock *pLP; // Loop preheader
    BasicBlock *pLL; // Loop latch

    LoopItem() { pLB = pLE = pLP = pLL = nullptr; }
  };

  LLVMContext *m_pCtx;
  Module *m_pModule;
  Function *m_pFunc;
  LoopInfo *m_pLI;
  unsigned m_HelperExitCondIndex;
  BasicBlock *m_pLoopHeader;
  vector<Loop *> m_Loops;
  unordered_map<BasicBlock *, LoopItem> m_LoopMap;
  unordered_map<BasicBlock *, BasicBlock *> m_LE2LBMap;

  void Clear();

  //
  // Preliminary CFG transformations and related utilities.
  //
  void SanitizeBranches();
  void SanitizeBranchesRec(BasicBlock *pBB,
                           unordered_set<BasicBlock *> &VisitedBB);
  void CollectUniqueSuccessors(const BasicBlock *pBB,
                               const BasicBlock *pSuccessorToExclude,
                               vector<BasicBlock *> &Successors);

  //
  // Loop region transformations.
  //
  void CollectLoopsRec(Loop *pLoop);

  void AnnotateBranch(BasicBlock *pBB, BranchKind Kind);
  BranchKind GetBranchAnnotation(const BasicBlock *pBB);
  void RemoveBranchAnnotation(BasicBlock *pBB);

  void GetUniqueExitBlocks(const SmallVectorImpl<Loop::Edge> &ExitEdges,
                           SmallVectorImpl<BasicBlock *> &ExitBlocks);
  bool IsLoopBackedge(BasicBlock *pNode);
  bool IsAcyclicRegionTerminator(const BasicBlock *pBB);

  BasicBlock *GetEffectiveNodeToFollowSuccessor(BasicBlock *pBB);
  bool IsMergePoint(BasicBlock *pBB);

  BasicBlock *SplitEdge(BasicBlock *pBB, unsigned SuccIdx, const Twine &Name,
                        Loop *pLoop, BasicBlock *pToInsertBB);
  BasicBlock *SplitEdge(BasicBlock *pBB, BasicBlock *pSucc, const Twine &Name,
                        Loop *pLoop, BasicBlock *pToInsertBB);
  /// Ensure that the latch node terminates by an unconditional branch. Return
  /// the latch node.
  BasicBlock *SanitizeLoopLatch(Loop *pLoop);
  unsigned GetHelperExitCondIndex() { return m_HelperExitCondIndex++; }
  /// Ensure that loop has either single exit or no exits. Return the exit node
  /// or nullptr.
  BasicBlock *SanitizeLoopExits(Loop *pLoop);
  void SanitizeLoopContinues(Loop *pLoop);
  void AnnotateLoopBranches(Loop *pLoop, LoopItem *pLI);

  //
  // BasicBlock topological order and reachability sets for acyclic region.
  //
  class BlockTopologicalOrderAndReachability {
  public:
    void AppendBlock(BasicBlock *pBB, unique_ptr<BitVector> ReachableBBs);
    unsigned GetNumBlocks() const;
    BasicBlock *GetBlock(unsigned Id) const;
    unsigned GetBlockId(BasicBlock *pBB) const;
    BitVector *GetReachableBBs(BasicBlock *pBB) const;
    BitVector *GetReachableBBs(unsigned Id) const;

    void dump(raw_ostream &OS) const;

  private:
    struct BasicBlockState {
      BasicBlock *pBB;
      unique_ptr<BitVector> ReachableBBs;
      BasicBlockState(BasicBlock *p, unique_ptr<BitVector> bv)
          : pBB(p), ReachableBBs(std::move(bv)) {}
    };
    vector<BasicBlockState> m_BlockState;
    unordered_map<BasicBlock *, unsigned> m_BlockIdMap;
  };
  void ComputeBlockTopologicalOrderAndReachability(
      BasicBlock *pEntry, BlockTopologicalOrderAndReachability &BTO);
  void ComputeBlockTopologicalOrderAndReachabilityRec(
      BasicBlock *pNode, BlockTopologicalOrderAndReachability &BTO,
      unordered_map<BasicBlock *, unsigned> &Marks);

  //
  // Recovery of scope end points.
  //
  struct MergePointInfo {
    unsigned MP; // Index of the merge point, if known.
    set<unsigned> CandidateSet;
  };
  using MergePointsMap =
      unordered_map<BasicBlock *, unique_ptr<MergePointInfo>>;
  using ScopeEndPointsMap = unordered_map<BasicBlock *, BasicBlock *>;
  using SwitchBreaksMap = unordered_map<BasicBlock *, BasicBlock *>;

  void DetermineScopeEndPoints(BasicBlock *pEntry, bool bRecomputeSwitchScope,
                               const BlockTopologicalOrderAndReachability &BTO,
                               const SwitchBreaksMap &SwitchBreaks,
                               ScopeEndPointsMap &ScopeEndPoints,
                               ScopeEndPointsMap &DeltaScopeEndPoints);
  void DetermineReachableMergePoints(
      BasicBlock *pEntry, BasicBlock *pExit, bool bRecomputeSwitchScope,
      const BitVector *pReachableBBs,
      const BlockTopologicalOrderAndReachability &BTO,
      const SwitchBreaksMap &SwitchBreaks,
      const ScopeEndPointsMap &OldScopeEndPoints, MergePointsMap &MergePoints);
  void DetermineSwitchBreaks(BasicBlock *pSwitchBegin,
                             const ScopeEndPointsMap &ScopeEndPoints,
                             const BlockTopologicalOrderAndReachability &BTO,
                             SwitchBreaksMap &SwitchBreaks);

  //
  // Transformation of acyclic region.
  //
  void TransformAcyclicRegion(BasicBlock *pEntry);

  // Scope stack.
  struct ScopeStackItem {
    enum class Kind {
      Invalid = 0,
      Return,
      Fallthrough,
      If,
      Switch,
    };

    Kind ScopeKind;

    BasicBlock *pScopeBeginBB;
    BasicBlock *pClonedScopeBeginBB;
    BasicBlock *pScopeEndBB;
    BasicBlock *pClonedScopeEndBB;

    unsigned SuccIdx;
    BasicBlock *pPrevSuccBB;
    BasicBlock *pClonedPrevSuccBB;

    shared_ptr<ScopeEndPointsMap> ScopeEndPoints;
    bool bRestoreIfScopeEndPoint;
    shared_ptr<ScopeEndPointsMap> DeltaScopeEndPoints;
    shared_ptr<SwitchBreaksMap> SwitchBreaks;

    ScopeStackItem()
        : ScopeKind(Kind::Invalid), pScopeBeginBB(nullptr),
          pClonedScopeBeginBB(nullptr), pScopeEndBB(nullptr),
          pClonedScopeEndBB(nullptr), SuccIdx(0), pPrevSuccBB(nullptr),
          pClonedPrevSuccBB(nullptr), bRestoreIfScopeEndPoint(false) {}
  };
  vector<ScopeStackItem> m_ScopeStack;

  ScopeStackItem &PushScope(BasicBlock *pBB);
  ScopeStackItem &RePushScope(const ScopeStackItem &Scope);
  ScopeStackItem *GetScope(unsigned Idx = 0);
  ScopeStackItem *FindParentScope(ScopeStackItem::Kind ScopeKind);
  void PopScope();

  // Cloning.
  void AddEdge(BasicBlock *pClonedSrcBB, unsigned SuccSlotIdx,
               BasicBlock *pDstBB,
               unordered_map<BasicBlock *, vector<BasicBlock *>> &Edges);
  BasicBlock *CloneBasicBlockAndFixupValues(const BasicBlock *pBB,
                                            ValueToValueMapTy &RegionValueRemap,
                                            const Twine &NameSuffix = "");
  BasicBlock *
  CloneNode(BasicBlock *pBB,
            unordered_map<BasicBlock *, vector<BasicBlock *>> &BlockClones,
            ValueToValueMapTy &RegionValueRemap);
  BasicBlock *
  CloneLoop(BasicBlock *pHeaderBB, BasicBlock *pClonedPreHeaderBB,
            unordered_map<BasicBlock *, vector<BasicBlock *>> &BlockClones,
            unordered_map<BasicBlock *, vector<BasicBlock *>> &Edges,
            ValueToValueMapTy &RegionValueRemap);
  BasicBlock *
  CloneLoopRec(BasicBlock *pBB, BasicBlock *pClonedPredBB,
               unsigned ClonedPredIdx,
               unordered_map<BasicBlock *, vector<BasicBlock *>> &BlockClones,
               unordered_map<BasicBlock *, vector<BasicBlock *>> &Edges,
               unordered_set<BasicBlock *> &VisitedBlocks, const LoopItem &LI,
               LoopItem &ClonedLI, ValueToValueMapTy &RegionValueRemap);

  //
  // Utility functions.
  //
  bool IsIf(BasicBlock *pBB);
  bool IsIf(TerminatorInst *pTI);
  bool IsSwitch(BasicBlock *pBB);
  bool IsSwitch(TerminatorInst *pTI);
  Value *GetFalse();
  Value *GetTrue();
  ConstantInt *GetI32Const(int v);
  void DumpIntSet(raw_ostream &s, set<unsigned> Set);
};

char ScopeNestedCFG::ID = 0;

bool ScopeNestedCFG::runOnFunction(Function &F) {
#if SNCFG_DBG
  dbgs() << "ScopeNestedCFG: processing function " << F.getName();
#endif
  Clear();

  m_pCtx = &F.getContext();
  m_pModule = F.getParent();
  m_pFunc = &F;
  m_pLI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  // Sanitize branches.
  SanitizeBranches();

  // Collect loops innermost to outermost.
  for (auto itLoop = m_pLI->begin(), endLoop = m_pLI->end(); itLoop != endLoop;
       ++itLoop) {
    Loop *pLoop = *itLoop;
    CollectLoopsRec(pLoop);
  }

  //
  // Phase 1:
  //   - verify, analyze and prepare loop shape
  //   - record loop information
  //   - classify and annotate loop branches
  //
  for (size_t iLoop = 0; iLoop < m_Loops.size(); iLoop++) {
    Loop *pLoop = m_Loops[iLoop];
    BasicBlock *pPreHeader = pLoop->getLoopPreheader();
    BasicBlock *pHeader = pLoop->getHeader();
    BasicBlock *pLatch = pLoop->getLoopLatch();
    BasicBlock *pExit = nullptr;

    // Make sure there is preheader.
    IFTBOOL(pPreHeader != nullptr, DXC_E_SCOPE_NESTED_FAILED);

    // Make sure there is a single backedge.
    IFTBOOL(pLoop->getNumBackEdges() == 1, DXC_E_SCOPE_NESTED_FAILED);

    // Prepare loop latch.
    pLatch = SanitizeLoopLatch(pLoop);

    // Prepare exits and breaks.
    pExit = SanitizeLoopExits(pLoop);

    // Prepare continues.
    SanitizeLoopContinues(pLoop);

    // Record essential loop information.
    LoopItem LI;
    LI.pLB = pHeader;
    LI.pLE = pExit;
    LI.pLL = pLatch;
    LI.pLP = pPreHeader;
    DXASSERT_NOMSG(m_LoopMap.find(LI.pLB) == m_LoopMap.end());
    m_LoopMap[LI.pLB] = LI;
    if (LI.pLE != nullptr) {
      DXASSERT_NOMSG(m_LE2LBMap.find(LI.pLE) == m_LE2LBMap.end());
      m_LE2LBMap[LI.pLE] = LI.pLB;
    }

    // Annotate known branches for the loop.
    AnnotateLoopBranches(pLoop, &LI);
  }

  //
  // Phase 2:
  //   - for each loop from most inner:
  //     + "remove" backedge
  //     + transform acyclic region
  //   - transform entry region
  //
  for (size_t iLoop = 0; iLoop < m_Loops.size(); iLoop++) {
    Loop *pLoop = m_Loops[iLoop];
    BasicBlock *pHeader = pLoop->getHeader();
    LoopItem LI = m_LoopMap[pHeader];
    BasicBlock *pLatch = LI.pLL;
    DXASSERT_LOCALVAR_NOMSG(
        pLatch, pLatch->getTerminator()->getNumSuccessors() == 1 &&
                    pLatch->getTerminator()->getSuccessor(0) == pHeader);

    m_pLoopHeader = pHeader;

    TransformAcyclicRegion(pHeader);
  }

  m_pLoopHeader = nullptr;
  TransformAcyclicRegion(F.begin());

  return true;
}

void ScopeNestedCFG::Clear() {
  m_pCtx = nullptr;
  m_pModule = nullptr;
  m_pFunc = nullptr;
  m_pLI = nullptr;
  m_HelperExitCondIndex = 0;
  m_pLoopHeader = nullptr;
  m_Loops.clear();
  m_LoopMap.clear();
  m_LE2LBMap.clear();
}

//-----------------------------------------------------------------------------
// Preliminary CFG transformations and related utilities.
//-----------------------------------------------------------------------------
void ScopeNestedCFG::SanitizeBranches() {
  unordered_set<BasicBlock *> VisitedBB;
  SanitizeBranchesRec(m_pFunc->begin(), VisitedBB);
}

void ScopeNestedCFG::SanitizeBranchesRec(
    BasicBlock *pBB, unordered_set<BasicBlock *> &VisitedBB) {
  // Mark pBB as visited, and return if pBB already has been visited.
  if (!VisitedBB.emplace(pBB).second)
    return;

  // Sanitize branch.
  if (BranchInst *I = dyn_cast<BranchInst>(pBB->getTerminator())) {
    // a. Convert a conditional branch to unconditional, if successors are the
    // same.
    if (I->isConditional()) {
      BasicBlock *pSucc1 = I->getSuccessor(0);
      BasicBlock *pSucc2 = I->getSuccessor(1);
      if (pSucc1 == pSucc2) {
        BranchInst::Create(pSucc1, I);
        I->eraseFromParent();
      }
    }
  } else if (SwitchInst *I = dyn_cast<SwitchInst>(pBB->getTerminator())) {
    // b. Group switch successors.
    struct SwitchCaseGroup {
      BasicBlock *pSuccBB;
      vector<ConstantInt *> CaseValue;
    };
    vector<SwitchCaseGroup> SwitchCaseGroups;
    unordered_map<BasicBlock *, unsigned> BB2GroupIdMap;
    BasicBlock *pDefaultBB = I->getDefaultDest();

    for (SwitchInst::CaseIt itCase = I->case_begin(), endCase = I->case_end();
         itCase != endCase; ++itCase) {
      BasicBlock *pSuccBB = itCase.getCaseSuccessor();
      ConstantInt *pCaseValue = itCase.getCaseValue();

      if (pSuccBB == pDefaultBB) {
        // Assimilate this case label into default label.
        continue;
      }

      auto itGroup = BB2GroupIdMap.insert({pSuccBB, SwitchCaseGroups.size()});
      if (itGroup.second) {
        SwitchCaseGroups.emplace_back(SwitchCaseGroup{});
      }

      SwitchCaseGroup &G = SwitchCaseGroups[itGroup.first->second];
      G.pSuccBB = pSuccBB;
      G.CaseValue.emplace_back(pCaseValue);
    }

    if (SwitchCaseGroups.size() == 0) {
      // All case labels were assimilated into the default label.
      // Replace switch with an unconditional branch.
      BranchInst::Create(pDefaultBB, I);
      I->eraseFromParent();
    } else {
      // Rewrite switch instruction such that case labels are grouped by the
      // successor.
      unsigned CaseIdx = 0;
      for (const SwitchCaseGroup &G : SwitchCaseGroups) {
        for (ConstantInt *pCaseValue : G.CaseValue) {
          SwitchInst::CaseIt itCase(I, CaseIdx++);
          itCase.setSuccessor(G.pSuccBB);
          itCase.setValue(pCaseValue);
        }
      }
      // Remove unused case labels.
      for (unsigned NumCases = I->getNumCases(); CaseIdx < NumCases;
           NumCases--) {
        I->removeCase(SwitchInst::CaseIt{I, NumCases - 1});
      }
    }
  }

  // Recurse, visiting each successor group once.
  TerminatorInst *pTI = pBB->getTerminator();
  BasicBlock *pPrevSuccBB = nullptr;
  for (unsigned i = 0; i < pTI->getNumSuccessors(); i++) {
    BasicBlock *pSuccBB = pTI->getSuccessor(i);
    if (pSuccBB != pPrevSuccBB) {
      SanitizeBranchesRec(pSuccBB, VisitedBB);
    }
    pPrevSuccBB = pSuccBB;
  }
}

void ScopeNestedCFG::CollectUniqueSuccessors(
    const BasicBlock *pBB, const BasicBlock *pSuccessorToExclude,
    vector<BasicBlock *> &Successors) {
  DXASSERT_NOMSG(Successors.empty());
  const TerminatorInst *pTI = pBB->getTerminator();
  BasicBlock *pPrevSuccBB = nullptr;
  for (unsigned i = 0; i < pTI->getNumSuccessors(); i++) {
    BasicBlock *pSuccBB = pTI->getSuccessor(i);

    if (pSuccBB != pPrevSuccBB) {
      pPrevSuccBB = pSuccBB;

      if (pSuccBB != pSuccessorToExclude)
        Successors.emplace_back(pSuccBB);
    }
  }
}

//-----------------------------------------------------------------------------
// Loop region transformations.
//-----------------------------------------------------------------------------
void ScopeNestedCFG::CollectLoopsRec(Loop *pLoop) {
  for (auto itLoop = pLoop->begin(), endLoop = pLoop->end(); itLoop != endLoop;
       ++itLoop) {
    Loop *pNestedLoop = *itLoop;
    CollectLoopsRec(pNestedLoop);
  }

  m_Loops.emplace_back(pLoop);
}

void ScopeNestedCFG::AnnotateBranch(BasicBlock *pBB, BranchKind Kind) {
  TerminatorInst *pTI = pBB->getTerminator();
  DXASSERT(dyn_cast<BranchInst>(pTI) != nullptr ||
               dyn_cast<SwitchInst>(pTI) != nullptr,
           "annotate only branch and switch terminators");

  // Check that we are not changing the annotation.
  MDNode *pMD = pTI->getMetadata("dx.BranchKind");
  if (pMD != nullptr) {
    ConstantAsMetadata *p1 = dyn_cast<ConstantAsMetadata>(pMD->getOperand(0));
    ConstantInt *pVal = dyn_cast<ConstantInt>(p1->getValue());
    BranchKind OldKind = (BranchKind)pVal->getZExtValue();
    DXASSERT_LOCALVAR(
        OldKind,
        OldKind == Kind ||
            (OldKind == BranchKind::IfBegin && Kind == BranchKind::IfNoEnd) ||
            (OldKind == BranchKind::IfNoEnd && Kind == BranchKind::IfBegin),
        "the algorithm should not be changing branch types implicitly (unless "
        "it is an if)");
  }

  pTI->setMetadata(
      "dx.BranchKind",
      MDNode::get(*m_pCtx, ConstantAsMetadata::get(GetI32Const((int)Kind))));
}

BranchKind ScopeNestedCFG::GetBranchAnnotation(const BasicBlock *pBB) {
  const TerminatorInst *pTI = pBB->getTerminator();
  MDNode *pMD = pTI->getMetadata("dx.BranchKind");
  if (pMD != nullptr) {
    ConstantAsMetadata *p1 = dyn_cast<ConstantAsMetadata>(pMD->getOperand(0));
    ConstantInt *pVal = dyn_cast<ConstantInt>(p1->getValue());
    return (BranchKind)pVal->getZExtValue();
  }
  return BranchKind::Invalid;
}

void ScopeNestedCFG::RemoveBranchAnnotation(BasicBlock *pBB) {
  TerminatorInst *pTI = pBB->getTerminator();
  pTI->setMetadata("dx.BranchKind", nullptr);
}

void ScopeNestedCFG::GetUniqueExitBlocks(
    const SmallVectorImpl<Loop::Edge> &ExitEdges,
    SmallVectorImpl<BasicBlock *> &ExitBlocks) {
  DXASSERT_NOMSG(ExitBlocks.empty());
  unordered_set<BasicBlock *> S;
  for (size_t i = 0; i < ExitEdges.size(); i++) {
    const Loop::Edge &E = ExitEdges[i];
    BasicBlock *B = const_cast<BasicBlock *>(E.second);
    auto itp = S.insert(B);
    if (itp.second) {
      ExitBlocks.push_back(B);
    }
  }
}

bool ScopeNestedCFG::IsLoopBackedge(BasicBlock *pNode) {
  BranchKind BK = GetBranchAnnotation(pNode);

  if (BK == BranchKind::LoopBackEdge) {
    DXASSERT_NOMSG(pNode->getTerminator()->getNumSuccessors() == 1);
    DXASSERT_NOMSG(pNode->getTerminator()->getSuccessor(0) == m_pLoopHeader);
    return true;
  }

  return false;
}

BasicBlock *ScopeNestedCFG::GetEffectiveNodeToFollowSuccessor(BasicBlock *pBB) {
  BasicBlock *pEffectiveSuccessor = nullptr;
  BranchKind BK = GetBranchAnnotation(pBB);

  switch (BK) {
  case BranchKind::LoopBegin: {
    TerminatorInst *pTI = pBB->getTerminator();
    DXASSERT_NOMSG(pTI->getNumSuccessors() == 1);
    BasicBlock *pLoopHead = pTI->getSuccessor(0);
    auto itLoop = m_LoopMap.find(pLoopHead);
    DXASSERT_NOMSG(itLoop != m_LoopMap.end());
    const LoopItem &LI = itLoop->second;
    DXASSERT_NOMSG(LI.pLB == pLoopHead && LI.pLP == pBB);
    DXASSERT_NOMSG(LI.pLE->getTerminator()->getNumSuccessors() == 1);
    pEffectiveSuccessor = LI.pLE;
    break;
  }

  case BranchKind::LoopNoEnd:
    pEffectiveSuccessor = nullptr;
    break;

  default:
    pEffectiveSuccessor = pBB;
    break;
  }

  return pEffectiveSuccessor;
}

bool ScopeNestedCFG::IsMergePoint(BasicBlock *pBB) {
  unordered_set<BasicBlock *> UniquePredecessors;
  for (auto itPred = pred_begin(pBB), endPred = pred_end(pBB);
       itPred != endPred; ++itPred) {
    BasicBlock *pPredBB = *itPred;
    if (IsLoopBackedge(pPredBB))
      continue;

    UniquePredecessors.insert(pPredBB);
  }

  return UniquePredecessors.size() >= 2;
}

bool ScopeNestedCFG::IsAcyclicRegionTerminator(const BasicBlock *pNode) {
  // Return.
  if (dyn_cast<ReturnInst>(pNode->getTerminator()))
    return true;

  BranchKind BK = GetBranchAnnotation(pNode);
  switch (BK) {
  case BranchKind::LoopBreak:
  case BranchKind::LoopContinue:
  case BranchKind::LoopBackEdge:
    return true;
  }

  return false;
}

BasicBlock *ScopeNestedCFG::SplitEdge(BasicBlock *pBB, BasicBlock *pSucc,
                                      const Twine &Name, Loop *pLoop,
                                      BasicBlock *pToInsertBB) {
  unsigned SuccIdx = GetSuccessorNumber(pBB, pSucc);
  return SplitEdge(pBB, SuccIdx, Name, pLoop, pToInsertBB);
}

BasicBlock *ScopeNestedCFG::SplitEdge(BasicBlock *pBB, unsigned SuccIdx,
                                      const Twine &Name, Loop *pLoop,
                                      BasicBlock *pToInsertBB) {
  BasicBlock *pNewBB = pToInsertBB;
  if (pToInsertBB == nullptr) {
    pNewBB = BasicBlock::Create(*m_pCtx, Name, m_pFunc, pBB->getNextNode());
  }

  if (pLoop != nullptr) {
    pLoop->addBasicBlockToLoop(pNewBB, *m_pLI);
  }

  BasicBlock *pSucc = pBB->getTerminator()->getSuccessor(SuccIdx);
  pBB->getTerminator()->setSuccessor(SuccIdx, pNewBB);

  if (pToInsertBB == nullptr) {
    BranchInst::Create(pSucc, pNewBB);
  } else {
    TerminatorInst *pTI = pNewBB->getTerminator();
    DXASSERT_NOMSG(dyn_cast<BranchInst>(pTI) != nullptr &&
                   pTI->getNumSuccessors() == 1);
    pTI->setSuccessor(0, pSucc);
  }

  return pNewBB;
}

BasicBlock *ScopeNestedCFG::SanitizeLoopLatch(Loop *pLoop) {
  BasicBlock *pHeader = pLoop->getHeader();
  BasicBlock *pLatch = pLoop->getLoopLatch();

  TerminatorInst *pTI = pLatch->getTerminator();
  DXASSERT_NOMSG(pTI->getNumSuccessors() != 0 &&
                 dyn_cast<ReturnInst>(pTI) == nullptr);

  BasicBlock *pNewLatch = pLatch;
  // Make sure that latch node is empty and terminates with a 'br'.
  if (dyn_cast<BranchInst>(pTI) == nullptr || (&*pLatch->begin()) != pTI ||
      pTI->getNumSuccessors() > 1) {
    pNewLatch = SplitEdge(pLatch, pHeader, "dx.LoopLatch", pLoop, nullptr);
  }

  return pNewLatch;
}

BasicBlock *ScopeNestedCFG::SanitizeLoopExits(Loop *pLoop) {
  Loop *pOuterLoop = pLoop->getParentLoop();
  BasicBlock *pPreHeader = pLoop->getLoopPreheader();
  BasicBlock *pLatch = pLoop->getLoopLatch();

  SmallVector<Loop::Edge, 8> ExitEdges;
  pLoop->getExitEdges(ExitEdges);
  SmallVector<BasicBlock *, 8> OldExitBBs;
  GetUniqueExitBlocks(ExitEdges, OldExitBBs);

  if (OldExitBBs.empty()) {
    // A loop without breaks.
    return nullptr;
  }

  // Create the loop exit BB.
  BasicBlock *pExit = BasicBlock::Create(*m_pCtx, "dx.LoopExit", m_pFunc,
                                         pLatch->getNextNode());
  if (pOuterLoop != nullptr) {
    pOuterLoop->addBasicBlockToLoop(pExit, *m_pLI);
  }

  // Create helper exit blocks.
  SmallVector<BasicBlock *, 8> HelperExitBBs;
  for (size_t iExitBB = 0; iExitBB < OldExitBBs.size(); iExitBB++) {
    BasicBlock *pOldExit = OldExitBBs[iExitBB];
    BasicBlock *pNewExit = BasicBlock::Create(*m_pCtx, "dx.LoopExitHelper",
                                              m_pFunc, pLatch->getNextNode());
    HelperExitBBs.push_back(pNewExit);

    if (pOuterLoop != nullptr) {
      pOuterLoop->addBasicBlockToLoop(pNewExit, *m_pLI);
    }

    // Adjust exit edges.
    SmallVector<BasicBlock *, 8> OldExitPredBBs;
    for (auto itPred = pred_begin(pOldExit), endPred = pred_end(pOldExit);
         itPred != endPred; ++itPred) {
      OldExitPredBBs.push_back(*itPred);
    }
    for (size_t PredIdx = 0; PredIdx < OldExitPredBBs.size(); PredIdx++) {
      BasicBlock *pOldExitPred = OldExitPredBBs[PredIdx];

      if (pLoop->contains(pOldExitPred)) {
        unsigned PredSuccIdx = GetSuccessorNumber(pOldExitPred, pOldExit);
        pOldExitPred->getTerminator()->setSuccessor(PredSuccIdx, pNewExit);
      }
    }
    DXASSERT_NOMSG(pred_begin(pNewExit) != pred_end(pNewExit));

    // Connect helper exit to the loop exit node.
    BranchInst::Create(pExit, pNewExit);
  }
  DXASSERT_NOMSG(HelperExitBBs.size() == OldExitBBs.size());

  // Fix up conditions for the rest of execution.
  unsigned NumExits = HelperExitBBs.size();
  BasicBlock *pRestOfExecutionBB = OldExitBBs.back();
  BranchInst::Create(pRestOfExecutionBB, pExit);
  for (unsigned i = 0; i < NumExits - 1; i++) {
    unsigned ExitIdx = NumExits - 2 - i;
    BasicBlock *pExitHelper = HelperExitBBs[ExitIdx];
    BasicBlock *pOldExit = OldExitBBs[ExitIdx];

    // Declare helper-exit guard variable.
    AllocaInst *pAI =
        new AllocaInst(Type::getInt1Ty(*m_pCtx), Twine("dx.LoopExitHelperCond"),
                       m_pFunc->begin()->begin());

    // Initialize the guard to 'false' before the loop.
    new StoreInst(GetFalse(), pAI, pPreHeader->getTerminator());

    // Assing the guard to 'true' in exit helper.
    new StoreInst(GetTrue(), pAI, pExitHelper->begin());

    // Insert an 'if' to conditionally guard exit execution.
    BasicBlock *pIfBB = BasicBlock::Create(*m_pCtx, "dx.LoopExitHelperIf",
                                           m_pFunc, pExit->getNextNode());
    if (pOuterLoop != nullptr) {
      pOuterLoop->addBasicBlockToLoop(pIfBB, *m_pLI);
    }
    LoadInst *pLoadCondI = new LoadInst(pAI);
    (void)BranchInst::Create(pOldExit, pRestOfExecutionBB, pLoadCondI, pIfBB);
    pIfBB->getInstList().insert(pIfBB->begin(), pLoadCondI);

    // Adjust rest-of-computation point.
    pExit->getTerminator()->setSuccessor(0, pIfBB);
    pRestOfExecutionBB = pIfBB;
  }

  // Duplicate helper exit nodes such that each has unique predecessor.
  for (size_t iHelperBB = 0; iHelperBB < HelperExitBBs.size(); iHelperBB++) {
    BasicBlock *pHelperBB = HelperExitBBs[iHelperBB];
    // Collect unique predecessors.
    SmallVector<BasicBlock *, 8> PredBBs;
    unordered_set<BasicBlock *> UniquePredBBs;
    for (auto itPred = pred_begin(pHelperBB), endPred = pred_end(pHelperBB);
         itPred != endPred; ++itPred) {
      BasicBlock *pPredBB = *itPred;
      auto P = UniquePredBBs.insert(pPredBB);
      if (P.second) {
        PredBBs.push_back(pPredBB);
      }
    }
    // Duplicate helper node.
    BasicBlock *pInsertionBB = PredBBs[0];
    for (size_t iSrc = 1; iSrc < PredBBs.size(); iSrc++) {
      BasicBlock *pPredBB = PredBBs[iSrc];
      ValueToValueMapTy EmptyRemap;
      BasicBlock *pClone = CloneBasicBlockAndFixupValues(pHelperBB, EmptyRemap);
      if (pOuterLoop != nullptr) {
        pOuterLoop->addBasicBlockToLoop(pClone, *m_pLI);
      }
      // Redirect predecessor successors.
      for (unsigned PredSuccIdx = 0;
           PredSuccIdx < pPredBB->getTerminator()->getNumSuccessors();
           PredSuccIdx++) {
        if (pPredBB->getTerminator()->getSuccessor(PredSuccIdx) != pHelperBB)
          continue;

        pPredBB->getTerminator()->setSuccessor(PredSuccIdx, pClone);
        // Update LoopInfo.
        if (pOuterLoop != nullptr && !pOuterLoop->contains(pExit)) {
          pOuterLoop->addBasicBlockToLoop(pExit, *m_pLI);
        }
        // Insert into function.
        m_pFunc->getBasicBlockList().insertAfter(pInsertionBB, pClone);
        pInsertionBB = pClone;
      }
    }
  }

  return pExit;
}

void ScopeNestedCFG::SanitizeLoopContinues(Loop *pLoop) {
  BasicBlock *pLatch = pLoop->getLoopLatch();
  TerminatorInst *pLatchTI = pLatch->getTerminator();
  DXASSERT_LOCALVAR_NOMSG(pLatchTI, dyn_cast<BranchInst>(pLatchTI) != nullptr &&
                                        pLatchTI->getNumSuccessors() == 1 &&
                                        (&*pLatch->begin()) == pLatchTI);

  // Collect continue BBs.
  SmallVector<BasicBlock *, 8> LatchPredBBs;
  for (auto itPred = pred_begin(pLatch), endPred = pred_end(pLatch);
       itPred != endPred; ++itPred) {
    BasicBlock *pPredBB = *itPred;
    LatchPredBBs.push_back(pPredBB);
  }
  DXASSERT_NOMSG(LatchPredBBs.size() >= 1);

  // Insert continue helpers.
  for (size_t i = 0; i < LatchPredBBs.size(); i++) {
    BasicBlock *pPredBB = LatchPredBBs[i];

    BasicBlock *pContinue =
        SplitEdge(pPredBB, pLatch, "dx.LoopContinue", pLoop, nullptr);
    DXASSERT_LOCALVAR_NOMSG(
        pContinue, pContinue->getTerminator()->getNumSuccessors() == 1);
    DXASSERT_NOMSG((++pred_begin(pContinue)) == pred_end(pContinue));
  }
}

void ScopeNestedCFG::AnnotateLoopBranches(Loop *pLoop, LoopItem *pLI) {
  // Annotate LB & LE.
  if (pLI->pLE != nullptr) {
    AnnotateBranch(pLI->pLP, BranchKind::LoopBegin);
    AnnotateBranch(pLI->pLE, BranchKind::LoopExit);
    DXASSERT_NOMSG(pLI->pLE->getTerminator()->getNumSuccessors() == 1);

    // Record and annotate loop breaks.
    for (auto itPred = pred_begin(pLI->pLE), endPred = pred_end(pLI->pLE);
         itPred != endPred; ++itPred) {
      BasicBlock *pPredBB = *itPred;
      DXASSERT_NOMSG(pPredBB->getTerminator()->getNumSuccessors() == 1);
      AnnotateBranch(pPredBB, BranchKind::LoopBreak);
    }
  } else {
    AnnotateBranch(pLI->pLP, BranchKind::LoopNoEnd);
  }

  // Record and annotate loop continues.
  for (auto itPred = pred_begin(pLI->pLL), endPred = pred_end(pLI->pLL);
       itPred != endPred; ++itPred) {
    BasicBlock *pPredBB = *itPred;
    DXASSERT_NOMSG(pPredBB->getTerminator()->getNumSuccessors() == 1);
    DXASSERT_NOMSG((++pred_begin(pPredBB)) == pred_end(pPredBB));
    AnnotateBranch(pPredBB, BranchKind::LoopContinue);
  }

  // Annotate loop backedge.
  AnnotateBranch(pLI->pLL, BranchKind::LoopBackEdge);
}

//-----------------------------------------------------------------------------
// BasicBlock topological order for acyclic region.
//-----------------------------------------------------------------------------
void ScopeNestedCFG::BlockTopologicalOrderAndReachability::AppendBlock(
    BasicBlock *pBB, unique_ptr<BitVector> ReachableBBs) {
  unsigned Id = (unsigned)m_BlockState.size();
  auto itp = m_BlockIdMap.insert({pBB, Id});
  DXASSERT_NOMSG(itp.second);
  ReachableBBs->set(Id);
  m_BlockState.emplace_back(BasicBlockState(pBB, std::move(ReachableBBs)));
}

unsigned
ScopeNestedCFG::BlockTopologicalOrderAndReachability::GetNumBlocks() const {
  DXASSERT_NOMSG(m_BlockState.size() < UINT32_MAX);
  return (unsigned)m_BlockState.size();
}

BasicBlock *ScopeNestedCFG::BlockTopologicalOrderAndReachability::GetBlock(
    unsigned Id) const {
  return m_BlockState[Id].pBB;
}

unsigned ScopeNestedCFG::BlockTopologicalOrderAndReachability::GetBlockId(
    BasicBlock *pBB) const {
  const auto it = m_BlockIdMap.find(pBB);
  if (it != m_BlockIdMap.cend())
    return it->second;
  else
    return UINT32_MAX;
}

BitVector *
ScopeNestedCFG::BlockTopologicalOrderAndReachability::GetReachableBBs(
    BasicBlock *pBB) const {
  return GetReachableBBs(GetBlockId(pBB));
}

BitVector *
ScopeNestedCFG::BlockTopologicalOrderAndReachability::GetReachableBBs(
    unsigned Id) const {
  return m_BlockState[Id].ReachableBBs.get();
}

void ScopeNestedCFG::BlockTopologicalOrderAndReachability::dump(
    raw_ostream &OS) const {
  for (unsigned i = 0; i < GetNumBlocks(); i++) {
    BasicBlock *pBB = GetBlock(i);
    DXASSERT_NOMSG(GetBlockId(pBB) == i);
    OS << i << ": " << pBB->getName() << ",  ReachableBBs = { ";
    BitVector *pReachableBBs = GetReachableBBs(i);
    bool bFirst = true;
    for (unsigned j = 0; j < GetNumBlocks(); j++) {
      if (pReachableBBs->test(j)) {
        if (!bFirst)
          OS << ", ";
        OS << j;
        bFirst = false;
      }
    }
    OS << " }\n";
  }
}

void ScopeNestedCFG::ComputeBlockTopologicalOrderAndReachability(
    BasicBlock *pEntry, BlockTopologicalOrderAndReachability &BTO) {
  unordered_map<BasicBlock *, unsigned> WaterMarks;
  ComputeBlockTopologicalOrderAndReachabilityRec(pEntry, BTO, WaterMarks);

#if SNCFG_DBG
  dbgs() << "\nBB topological order and reachable BBs rooted at "
         << pEntry->getName() << ":\n";
  BTO.dump(dbgs());
#endif
}

void ScopeNestedCFG::ComputeBlockTopologicalOrderAndReachabilityRec(
    BasicBlock *pNode, BlockTopologicalOrderAndReachability &BTO,
    unordered_map<BasicBlock *, unsigned> &Marks) {
  auto itMarkBB = Marks.find(pNode);
  if (Marks.find(pNode) != Marks.end()) {
    DXASSERT(itMarkBB->second == 2, "acyclic component has a cycle");
    return;
  }

  unsigned NumBBs = (unsigned)pNode->getParent()->getBasicBlockList().size();

  // Region terminator.
  if (IsAcyclicRegionTerminator(pNode)) {
    Marks[pNode] = 2; // late watermark
    BTO.AppendBlock(pNode, std::make_unique<BitVector>(NumBBs, false));
    return;
  }

  BasicBlock *pNodeToFollowSuccessors =
      GetEffectiveNodeToFollowSuccessor(pNode);

  // Loop with no exit.
  if (pNodeToFollowSuccessors == nullptr) {
    Marks[pNode] = 2; // late watermark
    BTO.AppendBlock(pNode, std::make_unique<BitVector>(NumBBs, false));
    return;
  }

  Marks[pNode] = 1; // early watermark

  auto ReachableBBs = std::make_unique<BitVector>(NumBBs, false);
  for (auto itSucc = succ_begin(pNodeToFollowSuccessors),
            endSucc = succ_end(pNodeToFollowSuccessors);
       itSucc != endSucc; ++itSucc) {
    BasicBlock *pSuccBB = *itSucc;

    ComputeBlockTopologicalOrderAndReachabilityRec(pSuccBB, BTO, Marks);
    // Union reachable BBs.
    (*ReachableBBs) |= (*BTO.GetReachableBBs(pSuccBB));
  }

  Marks[pNode] = 2; // late watermark

  BTO.AppendBlock(pNode, std::move(ReachableBBs));
}

//-----------------------------------------------------------------------------
// Recovery of scope end points.
//-----------------------------------------------------------------------------
void ScopeNestedCFG::DetermineScopeEndPoints(
    BasicBlock *pEntry, bool bRecomputeSwitchScope,
    const BlockTopologicalOrderAndReachability &BTO,
    const SwitchBreaksMap &SwitchBreaks, ScopeEndPointsMap &ScopeEndPoints,
    ScopeEndPointsMap &DeltaScopeEndPoints) {
  DXASSERT_NOMSG(DeltaScopeEndPoints.empty());

  // 1. Determine sets of reachable merge points and identifiable scope end
  // points.
  MergePointsMap MergePoints;
  BasicBlock *pExit = nullptr;
  BitVector *pReachableBBs = nullptr;
  if (bRecomputeSwitchScope) {
    auto it = ScopeEndPoints.find(pEntry);
    if (it != ScopeEndPoints.end()) {
      pExit = it->second;
    }
    pReachableBBs = BTO.GetReachableBBs(pEntry);
  }
  DetermineReachableMergePoints(pEntry, pExit, bRecomputeSwitchScope,
                                pReachableBBs, BTO, SwitchBreaks,
                                ScopeEndPoints, MergePoints);

  // 2. Construct partial scope end points map.
  for (auto &itMPI : MergePoints) {
    BasicBlock *pBB = itMPI.first;
    MergePointInfo &MPI = *itMPI.second;

    BasicBlock *pEndBB = nullptr;
    if (MPI.MP != UINT32_MAX) {
      pEndBB = BTO.GetBlock(MPI.MP);
    }

    auto itOldEndPointBB = ScopeEndPoints.find(pBB);
    if (itOldEndPointBB != ScopeEndPoints.end() &&
        itOldEndPointBB->second != pEndBB) {
      DeltaScopeEndPoints[pBB] = itOldEndPointBB->second;
      itOldEndPointBB->second = pEndBB;
    } else {
      ScopeEndPoints[pBB] = pEndBB;
    }
  }

#if SNCFG_DBG
  dbgs() << "\nScope ends:\n";
  for (auto it = ScopeEndPoints.begin(); it != ScopeEndPoints.end(); ++it) {
    BasicBlock *pBegin = it->first;
    BasicBlock *pEnd = it->second;
    dbgs() << pBegin->getName() << ", ID=" << BTO.GetBlockId(pBegin) << " -> ";
    if (pEnd) {
      dbgs() << pEnd->getName() << ", ID=" << BTO.GetBlockId(pEnd) << "\n";
    } else {
      dbgs() << "unreachable\n";
    }
  }
#endif
}

void ScopeNestedCFG::DetermineReachableMergePoints(
    BasicBlock *pEntry, BasicBlock *pExit, bool bRecomputeSwitchScope,
    const BitVector *pReachableBBs,
    const BlockTopologicalOrderAndReachability &BTO,
    const SwitchBreaksMap &SwitchBreaks,
    const ScopeEndPointsMap &OldScopeEndPoints, MergePointsMap &MergePoints) {
  DXASSERT_NOMSG(MergePoints.empty());
  unsigned MinBBIdx = 0;
  unsigned MaxBBIdx = BTO.GetNumBlocks() - 1;
  if (bRecomputeSwitchScope) {
    MinBBIdx = BTO.GetBlockId(pExit);
    MaxBBIdx = BTO.GetBlockId(pEntry);
  }

  for (unsigned iBB = MinBBIdx; iBB <= MaxBBIdx; iBB++) {
    if (bRecomputeSwitchScope && !pReachableBBs->test(iBB)) {
      // The block does not belong to the current switch region.
      continue;
    }

    BasicBlock *pBB = BTO.GetBlock(iBB);
    MergePoints[pBB] = unique_ptr<MergePointInfo>(new MergePointInfo);
    MergePointInfo &MPI = *MergePoints[pBB];
    BasicBlock *pNodeToFollowSuccessors =
        GetEffectiveNodeToFollowSuccessor(pBB);

    MPI.MP = UINT32_MAX;

    if (!IsAcyclicRegionTerminator(pBB) && pNodeToFollowSuccessors != nullptr &&
        !IsLoopBackedge(pNodeToFollowSuccessors) &&
        !(bRecomputeSwitchScope && pBB == pExit)) {
      // a. Collect unique successors, excluding switch break.
      const auto itSwitchBreaks = SwitchBreaks.find(pBB);
      const BasicBlock *pSwitchBreak = (itSwitchBreaks == SwitchBreaks.cend())
                                           ? nullptr
                                           : itSwitchBreaks->second;
      vector<BasicBlock *> Successors;
      CollectUniqueSuccessors(pNodeToFollowSuccessors, pSwitchBreak,
                              Successors);

      // b. Partition successors.
      struct Partition {
        set<unsigned> MPIndices;
        unordered_set<BasicBlock *> Blocks;
      };
      vector<Partition> Partitions;

      for (auto pSuccBB : Successors) {
        if (MergePoints.find(pSuccBB) == MergePoints.end()) {
          DXASSERT_NOMSG(bRecomputeSwitchScope &&
                         BTO.GetBlockId(pSuccBB) < MinBBIdx);
          MergePoints[pSuccBB] = std::make_unique<MergePointInfo>();
        }
        MergePointInfo &SuccMPI = *MergePoints[pSuccBB];

        // Find a partition for this successor.
        bool bFound = false;
        for (auto &P : Partitions) {
          set<unsigned> Intersection;
          std::set_intersection(
              P.MPIndices.begin(), P.MPIndices.end(),
              SuccMPI.CandidateSet.begin(), SuccMPI.CandidateSet.end(),
              std::inserter(Intersection, Intersection.end()));
          if (!Intersection.empty()) {
            swap(P.MPIndices, Intersection);
            P.Blocks.insert(pSuccBB);
            bFound = true;
            break;
          }
        }

        if (!bFound) {
          // Create a new partition.
          Partition P;
          P.MPIndices = SuccMPI.CandidateSet;
          P.Blocks.insert(pSuccBB);
          Partitions.emplace_back(P);
        }
      }

      // c. Analyze successors.
      if (Partitions.size() == 1) {
        auto &Intersection = Partitions[0].MPIndices;
        if (!Intersection.empty()) {
          MPI.MP = *Intersection.crbegin();
          swap(MPI.CandidateSet, Intersection); // discard partition set, as we
                                                // do not need it anymore.
        } else {
          MPI.MP = UINT32_MAX;
        }
      } else {
        // We do not [yet] know the merge point.
        MPI.MP = UINT32_MAX;

        // For switch, select the largest partition with at least two elements.
        if (SwitchInst *pSI = dyn_cast<SwitchInst>(
                pNodeToFollowSuccessors->getTerminator())) {
          size_t MaxPartSize = 0;
          size_t MaxPartIdx = 0;
          for (size_t i = 0; i < Partitions.size(); i++) {
            auto s = Partitions[i].Blocks.size();
            if (s > MaxPartSize) {
              MaxPartSize = s;
              MaxPartIdx = i;
            }
          }

          if (MaxPartSize >= 2) {
            MPI.MP = *Partitions[MaxPartIdx].MPIndices.crbegin();
            swap(
                MPI.CandidateSet,
                Partitions[MaxPartIdx].MPIndices); // discard partition set, as
                                                   // we do not need it anymore.
          }

          // TODO: during final testing consider to remove.
          if (MPI.MP == UINT32_MAX) {
            auto itOldMP = OldScopeEndPoints.find(pBB);
            if (itOldMP != OldScopeEndPoints.end()) {
              MPI.MP = BTO.GetBlockId(itOldMP->second);
              MPI.CandidateSet.insert(MPI.MP);
            }
          }
        }

        if (MPI.MP == UINT32_MAX) {
          // Compute MP union for upcoming propagation upwards.
          set<unsigned> Union;
          for (auto pSuccBB : Successors) {
            MergePointInfo &SuccMPI = *MergePoints[pSuccBB];

            set<unsigned> TmpSet;
            std::set_union(Union.begin(), Union.end(),
                           SuccMPI.CandidateSet.begin(),
                           SuccMPI.CandidateSet.end(),
                           std::inserter(TmpSet, TmpSet.end()));
            swap(Union, TmpSet);
          }

          swap(MPI.CandidateSet, Union);
        }
      }
    }

    // Add a merge point to the candidate set.
    if (IsMergePoint(pBB)) {
      DXASSERT_NOMSG(m_LoopMap.find(pBB) == m_LoopMap.cend());
      DXASSERT_NOMSG(m_LE2LBMap.find(pBB) == m_LE2LBMap.cend());
      MPI.CandidateSet.insert(iBB);
    }
  }

  // TODO: during final testing consider to remove.
  //  Compensate switch end point.
  if (SwitchInst *pSI = dyn_cast<SwitchInst>(pEntry->getTerminator())) {
    auto itOldEP = OldScopeEndPoints.find(pEntry);
    auto itMP = MergePoints.find(pEntry);
    if (itOldEP != OldScopeEndPoints.end()) {
      unsigned OldMP = BTO.GetBlockId(itOldEP->second);
      MergePointInfo &MPI = *itMP->second;
      if (MPI.MP != OldMP) {
        MPI.MP = OldMP;
        MPI.CandidateSet.clear();
        if (MPI.MP != UINT32_MAX) {
          MPI.CandidateSet.insert(MPI.MP);
        }
      }
    }
  }

#if SNCFG_DBG
  dbgs() << "\nScope ends:\n";
  for (auto it = MergePoints.begin(); it != MergePoints.end(); ++it) {
    BasicBlock *pBB = it->first;
    MergePointInfo &MPI = *it->second;
    dbgs() << it->first->getName() << ":  ID = " << BTO.GetBlockId(pBB)
           << ", MP = " << (int)MPI.MP << "\n";
    dbgs() << "  CandidateSet = ";
    DumpIntSet(dbgs(), MPI.CandidateSet);
    dbgs() << "\n";
  }
#endif
}

void ScopeNestedCFG::DetermineSwitchBreaks(
    BasicBlock *pSwitchBegin, const ScopeEndPointsMap &ScopeEndPoints,
    const BlockTopologicalOrderAndReachability &BTO,
    SwitchBreaksMap &SwitchBreaks) {
  DXASSERT_NOMSG(SwitchBreaks.empty());
  TerminatorInst *pTI = pSwitchBegin->getTerminator();
  DXASSERT_LOCALVAR_NOMSG(pTI, dyn_cast<SwitchInst>(pTI) != nullptr);

  auto it = ScopeEndPoints.find(pSwitchBegin);
  if (it == ScopeEndPoints.end())
    return;

  BasicBlock *pSwitchEnd = it->second;
  if (pSwitchEnd == nullptr)
    return;

  BitVector *pReachableFromSwitchBegin = BTO.GetReachableBBs(pSwitchBegin);
  for (auto itPred = pred_begin(pSwitchEnd), endPred = pred_end(pSwitchEnd);
       itPred != endPred; ++itPred) {
    BasicBlock *pPredBB = *itPred;
    unsigned PredId = BTO.GetBlockId(pPredBB);

    // An alternative entry into the acyclic component.
    if (PredId == UINT32_MAX)
      continue;

    // Record this switch break.
    if (pReachableFromSwitchBegin->test(PredId)) {
      SwitchBreaks.insert({pPredBB, pSwitchEnd});
    }
  }

#if SNCFG_DBG
  if (!SwitchBreaks.empty()) {
    dbgs() << "\nSwitch breaks:\n";
    for (auto it = SwitchBreaks.begin(); it != SwitchBreaks.end(); ++it) {
      BasicBlock *pSrcBB = it->first;
      BasicBlock *pDstBB = it->second;
      dbgs() << pSrcBB->getName() << " -> " << pDstBB->getName() << "\n";
    }
  }
#endif
}

//-----------------------------------------------------------------------------
// Transformation of acyclic region.
//-----------------------------------------------------------------------------
void ScopeNestedCFG::TransformAcyclicRegion(BasicBlock *pEntry) {
  unordered_map<BasicBlock *, vector<BasicBlock *>> BlockClones;
  unordered_map<BasicBlock *, vector<BasicBlock *>> Edges;
  ValueToValueMapTy RegionValueRemap;

  BlockTopologicalOrderAndReachability BTO;
  ComputeBlockTopologicalOrderAndReachability(pEntry, BTO);

  // Set up entry scope.
  ScopeStackItem &EntryScope = PushScope(pEntry);
  DXASSERT_NOMSG(EntryScope.pScopeBeginBB == pEntry);
  EntryScope.pClonedScopeBeginBB = pEntry;
  EntryScope.pScopeEndBB = nullptr;
  EntryScope.pClonedScopeEndBB = nullptr;
  DXASSERT_NOMSG(EntryScope.SuccIdx == 0);
  EntryScope.ScopeEndPoints = std::make_shared<ScopeEndPointsMap>();
  EntryScope.DeltaScopeEndPoints = std::make_shared<ScopeEndPointsMap>();
  EntryScope.SwitchBreaks = std::make_shared<SwitchBreaksMap>();
  DetermineScopeEndPoints(pEntry, false, BTO, SwitchBreaksMap{},
                          *EntryScope.ScopeEndPoints.get(),
                          *EntryScope.DeltaScopeEndPoints.get());

  while (!m_ScopeStack.empty()) {
    ScopeStackItem Scope = *GetScope();
    PopScope();
    // Assume: (1) current node is already cloned (if needed),
    //         (2) current node is already properly connected to its predecessor

    TerminatorInst *pScopeBeginTI = Scope.pScopeBeginBB->getTerminator();
    BranchKind BeginScopeBranchKind = GetBranchAnnotation(Scope.pScopeBeginBB);

    //
    // I. Process the node.
    //

    // 1. The node is a scope terminator.

    // 1a. Return.
    if (dyn_cast<ReturnInst>(pScopeBeginTI)) {
      continue;
    }

    DXASSERT_NOMSG(pScopeBeginTI->getNumSuccessors() > 0);
    // 1b. Break and continue.
    switch (BeginScopeBranchKind) {
    case BranchKind::LoopBreak: {
      // Connect to loop exit.
      TerminatorInst *pClonedScopeBeginTI =
          Scope.pClonedScopeBeginBB->getTerminator();
      DXASSERT_LOCALVAR_NOMSG(pClonedScopeBeginTI,
                              pClonedScopeBeginTI->getNumSuccessors() == 1);
      DXASSERT_NOMSG(m_LoopMap.find(pEntry) != m_LoopMap.end());
      LoopItem &LI = m_LoopMap[pEntry];
      AddEdge(Scope.pClonedScopeBeginBB, 0, LI.pLE, Edges);
      continue;
    }

    case BranchKind::LoopContinue: {
      // Connect to loop latch.
      TerminatorInst *pClonedScopeBeginTI =
          Scope.pClonedScopeBeginBB->getTerminator();
      DXASSERT_LOCALVAR_NOMSG(pClonedScopeBeginTI,
                              pClonedScopeBeginTI->getNumSuccessors() == 1);
      DXASSERT_NOMSG(m_LoopMap.find(pEntry) != m_LoopMap.end());
      LoopItem &LI = m_LoopMap[pEntry];
      AddEdge(Scope.pClonedScopeBeginBB, 0, LI.pLL, Edges);
      continue;
    }

    default:; // Process further.
    }

    // 1c. Loop latch node.
    if (IsLoopBackedge(Scope.pScopeBeginBB)) {
      continue;
    }

    // 2. Clone a nested loop and proceed after the loop.
    if (BeginScopeBranchKind == BranchKind::LoopBegin ||
        BeginScopeBranchKind == BranchKind::LoopNoEnd) {
      // The node is a loop preheader, which has been already cloned, if
      // necessary.

      // Original loop.
      BasicBlock *pPreheader = Scope.pScopeBeginBB;
      DXASSERT_NOMSG(pPreheader->getTerminator()->getNumSuccessors() == 1);
      BasicBlock *pHeader = pPreheader->getTerminator()->getSuccessor(0);
      LoopItem &Loop = m_LoopMap[pHeader];

      // Clone loop.
      BasicBlock *pClonedHeader =
          CloneLoop(pHeader, Scope.pClonedScopeBeginBB, BlockClones, Edges,
                    RegionValueRemap);

      // Connect cloned preheader to cloned loop.
      AddEdge(Scope.pClonedScopeBeginBB, 0, pClonedHeader, Edges);

      // Push loop-end node onto the stack.
      LoopItem &ClonedLoop = m_LoopMap[pClonedHeader];

      if (Loop.pLE != nullptr) {
        // Loop with loop exit node.
        DXASSERT_NOMSG(Loop.pLE->getTerminator()->getNumSuccessors() == 1);
        ScopeStackItem &AfterEndLoopScope = PushScope(Loop.pLE);
        AfterEndLoopScope.pClonedScopeBeginBB = ClonedLoop.pLE;
        AfterEndLoopScope.ScopeEndPoints = Scope.ScopeEndPoints;
        AfterEndLoopScope.DeltaScopeEndPoints = Scope.DeltaScopeEndPoints;
        AfterEndLoopScope.SwitchBreaks = Scope.SwitchBreaks;
      } else {
        // Loop without loop exit node.
        DXASSERT_NOMSG(ClonedLoop.pLE == nullptr);
      }

      continue;
    }

    // 3. Classify scope.
    bool bSwitchScope = IsSwitch(pScopeBeginTI);
    bool bIfScope = IsIf(pScopeBeginTI);

    // 4. Open scope.
    if (Scope.SuccIdx == 0 && (bIfScope || bSwitchScope)) {
      if (bSwitchScope) {
        // Detect switch breaks for switch scope.
        SwitchBreaksMap SwitchBreaks;
        DetermineSwitchBreaks(Scope.pScopeBeginBB, *Scope.ScopeEndPoints.get(),
                              BTO, SwitchBreaks);

        if (!SwitchBreaks.empty()) {
          // After switch breaks are known, recompute scope end points more
          // precisely.
          Scope.DeltaScopeEndPoints = std::make_shared<ScopeEndPointsMap>();
          Scope.SwitchBreaks = std::make_shared<SwitchBreaksMap>(SwitchBreaks);
          DetermineScopeEndPoints(
              Scope.pScopeBeginBB, true, BTO, *Scope.SwitchBreaks.get(),
              *Scope.ScopeEndPoints.get(), *Scope.DeltaScopeEndPoints.get());
        }
      }

      if (bIfScope) {
        // Refine if-scope end point.
        auto itEndIfScope = Scope.ScopeEndPoints->find(Scope.pScopeBeginBB);
        DXASSERT_NOMSG(itEndIfScope != Scope.ScopeEndPoints->cend());
        if (itEndIfScope->second == nullptr) {
          ScopeStackItem *pParentScope = GetScope();
          BasicBlock *pCandidateEndScopeBB = nullptr;
          if (pParentScope != nullptr && pParentScope->pScopeEndBB != nullptr) {
            // Determine which branch has parent's end scope node.
            unsigned ParentScopeEndId =
                BTO.GetBlockId(pParentScope->pScopeEndBB);
            for (unsigned i = 0; i < pScopeBeginTI->getNumSuccessors(); i++) {
              BasicBlock *pSucc = pScopeBeginTI->getSuccessor(i);
              // Skip a switch break.
              auto itSwBreak = Scope.SwitchBreaks->find(pSucc);
              if (itSwBreak != Scope.SwitchBreaks->end()) {
                continue;
              }

              BitVector *pReachableBBs = BTO.GetReachableBBs(pSucc);
              if (pReachableBBs->test(ParentScopeEndId)) {
                if (!pCandidateEndScopeBB) {
                  // Case1: one of IF's branches terminates only by region
                  // terminators.
                  pCandidateEndScopeBB = pSucc;
                } else {
                  // Case2: both branches terminate only by region terminators
                  // (e.g., SWITCH breaks).
                  pCandidateEndScopeBB = nullptr;
                }
              }
            }

            if (pCandidateEndScopeBB) {
              Scope.bRestoreIfScopeEndPoint = true;
              itEndIfScope->second = pCandidateEndScopeBB;
#if SNCFG_DBG
              BasicBlock *pBegin = Scope.pScopeBeginBB;
              BasicBlock *pEnd = pCandidateEndScopeBB;
              dbgs() << "\nAdjusted IF's end: ";
              dbgs() << pBegin->getName() << ", ID=" << BTO.GetBlockId(pBegin)
                     << " -> ";
              dbgs() << pEnd->getName() << ", ID=" << BTO.GetBlockId(pEnd)
                     << "\n";
#endif
            }
          }
        }
      }

      // Determine scope end and set up helper nodes, if necessary.
      BranchKind ScopeBeginBranchKind = BranchKind::Invalid;
      BranchKind ScopeEndBranchKind = BranchKind::Invalid;
      auto itEndScope = Scope.ScopeEndPoints->find(Scope.pScopeBeginBB);
      if (itEndScope != Scope.ScopeEndPoints->cend() &&
          itEndScope->second != nullptr) {
        Scope.pScopeEndBB = itEndScope->second;
        Scope.pClonedScopeEndBB = BasicBlock::Create(
            *m_pCtx, bIfScope ? "dx.EndIfScope" : "dx.EndSwitchScope", m_pFunc,
            Scope.pScopeEndBB);
        BranchInst::Create(Scope.pClonedScopeEndBB, Scope.pClonedScopeEndBB);
        ScopeBeginBranchKind =
            bIfScope ? BranchKind::IfBegin : BranchKind::SwitchBegin;
        ScopeEndBranchKind =
            bIfScope ? BranchKind::IfEnd : BranchKind::SwitchEnd;
      } else {
        Scope.pScopeEndBB = nullptr;
        Scope.pClonedScopeEndBB = nullptr;
        ScopeBeginBranchKind =
            bIfScope ? BranchKind::IfNoEnd : BranchKind::SwitchNoEnd;
      }

      // Annotate scope-begin and scope-end branches.
      DXASSERT_NOMSG(ScopeBeginBranchKind != BranchKind::Invalid);
      AnnotateBranch(Scope.pClonedScopeBeginBB, ScopeBeginBranchKind);
      if (Scope.pClonedScopeEndBB != nullptr) {
        DXASSERT_NOMSG(ScopeEndBranchKind != BranchKind::Invalid);
        AnnotateBranch(Scope.pClonedScopeEndBB, ScopeEndBranchKind);
      }
    }

    // 5. Push unfinished if and switch scopes onto the stack.
    if ((bIfScope || bSwitchScope) &&
        Scope.SuccIdx < pScopeBeginTI->getNumSuccessors()) {
      ScopeStackItem &UnfinishedScope = RePushScope(Scope);

      // Advance successor.
      UnfinishedScope.SuccIdx++;
    }

    // 6. Finalize scope.
    if ((bIfScope || bSwitchScope) &&
        (Scope.SuccIdx == pScopeBeginTI->getNumSuccessors())) {
      if (Scope.pScopeEndBB != nullptr) {
        bool bEndScopeSharedWithParent = false;

        ScopeStackItem *pParentScope = GetScope();
        if (pParentScope != nullptr) {
          if (Scope.pScopeEndBB == pParentScope->pScopeEndBB) {
            bEndScopeSharedWithParent = true;
            if (Scope.pClonedScopeEndBB != nullptr) {
              AddEdge(Scope.pClonedScopeEndBB, 0,
                      pParentScope->pClonedScopeEndBB, Edges);
            }
          }
        }

        if (!bEndScopeSharedWithParent) {
          // Clone original end-of-scope BB.
          ScopeStackItem &AfterEndOfScopeScope = PushScope(Scope.pScopeEndBB);
          AfterEndOfScopeScope.pClonedScopeBeginBB =
              CloneNode(Scope.pScopeEndBB, BlockClones, RegionValueRemap);
          AfterEndOfScopeScope.ScopeEndPoints = Scope.ScopeEndPoints;
          AfterEndOfScopeScope.DeltaScopeEndPoints = Scope.DeltaScopeEndPoints;
          AfterEndOfScopeScope.SwitchBreaks = Scope.SwitchBreaks;
          AddEdge(Scope.pClonedScopeEndBB, 0,
                  AfterEndOfScopeScope.pClonedScopeBeginBB, Edges);
        }
      }

      // Restore original (parent scope) ScopeEndPoints.
      if (bSwitchScope) {
        for (auto &it : *Scope.DeltaScopeEndPoints) {
          BasicBlock *pBB = it.first;
          BasicBlock *pOldMP = it.second;
          (*Scope.ScopeEndPoints)[pBB] = pOldMP;
        }
      }
      if (Scope.bRestoreIfScopeEndPoint) {
        DXASSERT_NOMSG(bIfScope);
        auto itEndIfScope = Scope.ScopeEndPoints->find(Scope.pScopeBeginBB);
        DXASSERT_NOMSG(itEndIfScope != Scope.ScopeEndPoints->cend());
        DXASSERT_NOMSG(itEndIfScope->second != nullptr);
        itEndIfScope->second = nullptr;
      }

      continue;
    }

    //
    // II. Process successors.
    //
    BasicBlock *pSuccBB = pScopeBeginTI->getSuccessor(Scope.SuccIdx);

    // 7. Already processed successor.
    if (bIfScope || bSwitchScope) {
      if (pSuccBB == Scope.pPrevSuccBB) {
        DXASSERT_NOMSG(Scope.pClonedPrevSuccBB != nullptr);
        AddEdge(Scope.pClonedScopeBeginBB, Scope.SuccIdx,
                Scope.pClonedPrevSuccBB, Edges);
        continue;
      }
    }

    // 8. Successor meets end-of-scope.
    bool bEndOfScope = false;
    if (pSuccBB == Scope.pScopeEndBB) {
      // 8a. Successor is end of current scope.
      bEndOfScope = true;
      AddEdge(Scope.pClonedScopeBeginBB, Scope.SuccIdx, Scope.pClonedScopeEndBB,
              Edges);
    } else {
      // 8b. Successor is end of parent scope.
      ScopeStackItem *pParentScope = GetScope();
      if (pParentScope != nullptr) {
        auto it = Scope.SwitchBreaks->find(Scope.pScopeBeginBB);
        bool bSwitchBreak = it != Scope.SwitchBreaks->cend();
        if (pSuccBB == pParentScope->pScopeEndBB) {
          bEndOfScope = true;
          if (!bSwitchBreak) {
            AddEdge(Scope.pClonedScopeBeginBB, Scope.SuccIdx,
                    pParentScope->pClonedScopeEndBB, Edges);
          }
        }
        if (bSwitchBreak) {
          if (pSuccBB == it->second) {
            // Switch break.
            bEndOfScope = true;
            ScopeStackItem *pSwitchScope =
                FindParentScope(ScopeStackItem::Kind::Switch);
            DXASSERT_NOMSG(pSuccBB == pSwitchScope->pScopeEndBB);
            BasicBlock *pSwitchBreakHelper =
                BasicBlock::Create(*m_pCtx, "dx.SwitchBreak", m_pFunc, pSuccBB);
            BranchInst::Create(pSwitchBreakHelper, pSwitchBreakHelper);
            AnnotateBranch(pSwitchBreakHelper, BranchKind::SwitchBreak);
            AddEdge(Scope.pClonedScopeBeginBB, Scope.SuccIdx,
                    pSwitchBreakHelper, Edges);
            AddEdge(pSwitchBreakHelper, 0, pSwitchScope->pClonedScopeEndBB,
                    Edges);
          }
        }
      }
    }

    // 9. Clone successor & push its record onto the stack.
    if (!bEndOfScope) {
      BasicBlock *pClonedSucc =
          CloneNode(pSuccBB, BlockClones, RegionValueRemap);

      if (bIfScope || bSwitchScope) {
        ScopeStackItem *pParentScope = GetScope();
        pParentScope->pPrevSuccBB = pSuccBB;
        pParentScope->pClonedPrevSuccBB = pClonedSucc;
      }

      // Create new scope to process the successor.
      ScopeStackItem &SuccScope = PushScope(pSuccBB);
      SuccScope.pPrevSuccBB = nullptr;
      SuccScope.pClonedPrevSuccBB = nullptr;
      SuccScope.pClonedScopeBeginBB = pClonedSucc;
      SuccScope.ScopeEndPoints = Scope.ScopeEndPoints;
      SuccScope.DeltaScopeEndPoints = Scope.DeltaScopeEndPoints;
      SuccScope.SwitchBreaks = Scope.SwitchBreaks;
      AddEdge(Scope.pClonedScopeBeginBB, Scope.SuccIdx,
              SuccScope.pClonedScopeBeginBB, Edges);
    }
  }

  // Fixup edges.
  for (auto itEdge = Edges.begin(), endEdge = Edges.end(); itEdge != endEdge;
       ++itEdge) {
    BasicBlock *pBB = itEdge->first;
    vector<BasicBlock *> &Successors = itEdge->second;
    TerminatorInst *pTI = pBB->getTerminator();
    DXASSERT_NOMSG(Successors.size() == pTI->getNumSuccessors());

    for (unsigned i = 0; i < pTI->getNumSuccessors(); ++i) {
      pTI->setSuccessor(i, Successors[i]);
    }
  }
}

ScopeNestedCFG::ScopeStackItem &ScopeNestedCFG::PushScope(BasicBlock *pBB) {
  ScopeStackItem SSI;
  SSI.pScopeBeginBB = pBB;
  TerminatorInst *pTI = pBB->getTerminator();

  if (dyn_cast<BranchInst>(pTI)) {
    DXASSERT_NOMSG(!IsLoopBackedge(pBB));
    unsigned NumSucc = pBB->getTerminator()->getNumSuccessors();
    switch (NumSucc) {
    case 1:
      SSI.ScopeKind = ScopeStackItem::Kind::Fallthrough;
      break;
    case 2:
      SSI.ScopeKind = ScopeStackItem::Kind::If;
      break;
    default:
      DXASSERT_NOMSG(false);
    }
  } else if (dyn_cast<ReturnInst>(pTI)) {
    SSI.ScopeKind = ScopeStackItem::Kind::Return;
  } else if (dyn_cast<SwitchInst>(pTI)) {
    SSI.ScopeKind = ScopeStackItem::Kind::Switch;
  } else {
    DXASSERT_NOMSG(false);
  }

  m_ScopeStack.emplace_back(SSI);
  return *GetScope();
}

ScopeNestedCFG::ScopeStackItem &
ScopeNestedCFG::RePushScope(const ScopeStackItem &Scope) {
  m_ScopeStack.emplace_back(Scope);
  return *GetScope();
}

ScopeNestedCFG::ScopeStackItem *ScopeNestedCFG::GetScope(unsigned Idx) {
  if (m_ScopeStack.size() > Idx) {
    return &m_ScopeStack[m_ScopeStack.size() - 1 - Idx];
  } else {
    return nullptr;
  }
}

ScopeNestedCFG::ScopeStackItem *
ScopeNestedCFG::FindParentScope(ScopeStackItem::Kind ScopeKind) {
  for (auto it = m_ScopeStack.rbegin(); it != m_ScopeStack.rend(); ++it) {
    ScopeStackItem &SSI = *it;
    if (SSI.ScopeKind == ScopeKind)
      return &SSI;
  }

  IFT(DXC_E_SCOPE_NESTED_FAILED);
  return nullptr;
}

void ScopeNestedCFG::PopScope() { m_ScopeStack.pop_back(); }

void ScopeNestedCFG::AddEdge(
    BasicBlock *pClonedSrcBB, unsigned SuccSlotIdx, BasicBlock *pDstBB,
    unordered_map<BasicBlock *, vector<BasicBlock *>> &Edges) {
  DXASSERT_NOMSG(pDstBB != nullptr);
  TerminatorInst *pTI = pClonedSrcBB->getTerminator();
  vector<BasicBlock *> *pSuccessors;
  auto it = Edges.find(pClonedSrcBB);
  if (it == Edges.end()) {
    Edges[pClonedSrcBB] = vector<BasicBlock *>(pTI->getNumSuccessors());
    pSuccessors = &Edges[pClonedSrcBB];
  } else {
    pSuccessors = &it->second;
  }

  (*pSuccessors)[SuccSlotIdx] = pDstBB;
}

BasicBlock *ScopeNestedCFG::CloneBasicBlockAndFixupValues(
    const BasicBlock *pBB, ValueToValueMapTy &RegionValueRemap,
    const Twine &NameSuffix) {
  // Create a clone.
  ValueToValueMapTy CloneMap;
  BasicBlock *pCloneBB = CloneBasicBlock(pBB, CloneMap, NameSuffix);

  // Update remapped values to the value remap for the acyclic region.
  for (auto it = CloneMap.begin(), endIt = CloneMap.end(); it != endIt; ++it) {
    RegionValueRemap[it->first] = it->second;
  }

  // Fixup values.
  for (auto itInst = pCloneBB->begin(), endInst = pCloneBB->end();
       itInst != endInst; ++itInst) {
    Instruction *pInst = itInst;
    for (unsigned i = 0; i < pInst->getNumOperands(); i++) {
      Value *V = pInst->getOperand(i);
      auto itV = RegionValueRemap.find(V);
      if (itV != RegionValueRemap.end()) {
        // Replace the replicated value.
        pInst->replaceUsesOfWith(V, itV->second);
      }
    }
  }

  return pCloneBB;
}

BasicBlock *ScopeNestedCFG::CloneNode(
    BasicBlock *pBB,
    unordered_map<BasicBlock *, vector<BasicBlock *>> &BlockClones,
    ValueToValueMapTy &RegionValueRemap) {
  auto it = BlockClones.find(pBB);
  if (it == BlockClones.end()) {
    // First time we see this BB.
    vector<BasicBlock *> V;
    V.emplace_back(pBB);
    BlockClones[pBB] = V;
    return pBB;
  }

  // Create a clone.
  BasicBlock *pCloneBB = CloneBasicBlockAndFixupValues(pBB, RegionValueRemap);
  it->second.emplace_back(pCloneBB);
  m_pFunc->getBasicBlockList().insertAfter(pBB, pCloneBB);

  // Temporarily adjust successors.
  for (unsigned i = 0; i < pCloneBB->getTerminator()->getNumSuccessors(); i++) {
    pCloneBB->getTerminator()->setSuccessor(i, pCloneBB);
  }

  return pCloneBB;
}

BasicBlock *ScopeNestedCFG::CloneLoop(
    BasicBlock *pHeaderBB, BasicBlock *pClonedPreHeaderBB,
    unordered_map<BasicBlock *, vector<BasicBlock *>> &BlockClones,
    unordered_map<BasicBlock *, vector<BasicBlock *>> &Edges,
    ValueToValueMapTy &RegionValueRemap) {
  // 1. clone every reachable node from LoopHeader (not! preheader) to LoopExit
  // (if not null).
  // 2. collect cloned edges along the way
  // 3. update loop map [for this loop only] (in case we need to copy a cloned
  // loop in the future).
  DXASSERT_NOMSG(m_LoopMap.find(pHeaderBB) != m_LoopMap.end());
  const LoopItem &LI = m_LoopMap.find(pHeaderBB)->second;
  unordered_set<BasicBlock *> VisitedBlocks;
  LoopItem ClonedLI;
  ClonedLI.pLP = pClonedPreHeaderBB;

  CloneLoopRec(pHeaderBB, nullptr, 0, BlockClones, Edges, VisitedBlocks, LI,
               ClonedLI, RegionValueRemap);

  m_LoopMap[ClonedLI.pLB] = ClonedLI;
  return ClonedLI.pLB;
}

BasicBlock *ScopeNestedCFG::CloneLoopRec(
    BasicBlock *pBB, BasicBlock *pClonePredBB, unsigned ClonedPredIdx,
    unordered_map<BasicBlock *, vector<BasicBlock *>> &BlockClones,
    unordered_map<BasicBlock *, vector<BasicBlock *>> &Edges,
    unordered_set<BasicBlock *> &VisitedBlocks, const LoopItem &LI,
    LoopItem &ClonedLI, ValueToValueMapTy &RegionValueRemap) {
  auto itBB = VisitedBlocks.find(pBB);
  if (itBB != VisitedBlocks.end()) {
    BasicBlock *pClonedBB = *BlockClones[*itBB].rbegin();
    // Clone the edge, but do not follow successors.
    if (pClonePredBB != nullptr) {
      AddEdge(pClonePredBB, ClonedPredIdx, pClonedBB, Edges);
    }
    return pClonedBB;
  }
  VisitedBlocks.insert(pBB);

  // Clone myself.
  BasicBlock *pClonedBB = CloneNode(pBB, BlockClones, RegionValueRemap);

  // Add edge from the predecessor BB to myself.
  if (pClonePredBB != nullptr) {
    AddEdge(pClonePredBB, ClonedPredIdx, pClonedBB, Edges);
  } else {
    ClonedLI.pLB = pClonedBB;
  }

  // Loop exit?
  if (pBB == LI.pLE) {
    ClonedLI.pLE = pClonedBB;
    return pClonedBB;
  }

  // Loop latch?
  if (pBB == LI.pLL) {
    ClonedLI.pLL = pClonedBB;
    AddEdge(ClonedLI.pLL, 0, ClonedLI.pLB, Edges);
  }

  // Process successors.
  TerminatorInst *pTI = pBB->getTerminator();
  BasicBlock *pPrevSuccBB = nullptr;
  BasicBlock *pPrevClonedSuccBB = nullptr;
  for (unsigned SuccIdx = 0; SuccIdx < pTI->getNumSuccessors(); SuccIdx++) {
    BasicBlock *pSuccBB = pTI->getSuccessor(SuccIdx);
    if (pSuccBB != pPrevSuccBB) {
      pPrevClonedSuccBB =
          CloneLoopRec(pSuccBB, pClonedBB, SuccIdx, BlockClones, Edges,
                       VisitedBlocks, LI, ClonedLI, RegionValueRemap);
      pPrevSuccBB = pSuccBB;
    } else {
      AddEdge(pClonedBB, SuccIdx, pPrevClonedSuccBB, Edges);
    }
  }

  return pClonedBB;
}

//-----------------------------------------------------------------------------
// Utility functions.
//-----------------------------------------------------------------------------
bool ScopeNestedCFG::IsIf(BasicBlock *pBB) {
  return IsIf(pBB->getTerminator());
}

bool ScopeNestedCFG::IsIf(TerminatorInst *pTI) {
  return pTI->getNumSuccessors() == 2 && dyn_cast<BranchInst>(pTI) != nullptr;
}

bool ScopeNestedCFG::IsSwitch(BasicBlock *pBB) {
  return IsSwitch(pBB->getTerminator());
}

bool ScopeNestedCFG::IsSwitch(TerminatorInst *pTI) {
  return dyn_cast<SwitchInst>(pTI) != nullptr;
}

Value *ScopeNestedCFG::GetFalse() {
  return Constant::getIntegerValue(IntegerType::get(*m_pCtx, 1), APInt(1, 0));
}

Value *ScopeNestedCFG::GetTrue() {
  return Constant::getIntegerValue(IntegerType::get(*m_pCtx, 1), APInt(1, 1));
}

ConstantInt *ScopeNestedCFG::GetI32Const(int v) {
  return ConstantInt::get(*m_pCtx, APInt(32, v));
}

void ScopeNestedCFG::DumpIntSet(raw_ostream &s, set<unsigned> Set) {
  s << "{ ";
  for (auto it = Set.begin(); it != Set.end(); ++it)
    s << *it << " ";
  s << "}";
}

} // namespace ScopeNestedCFGNS

using namespace ScopeNestedCFGNS;

INITIALIZE_PASS_BEGIN(ScopeNestedCFG, "scopenested",
                      "Scope-nested CFG transformation", false, false)
INITIALIZE_PASS_DEPENDENCY(ReducibilityAnalysis)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(ScopeNestedCFG, "scopenested",
                    "Scope-nested CFG transformation", false, false)

namespace llvm {

FunctionPass *createScopeNestedCFGPass() { return new ScopeNestedCFG(); }

} // namespace llvm
