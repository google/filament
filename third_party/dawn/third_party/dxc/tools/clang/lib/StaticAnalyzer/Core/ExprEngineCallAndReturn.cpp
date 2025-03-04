//=-- ExprEngineCallAndReturn.cpp - Support for call/return -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines ExprEngine's support for calls and returns.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"
#include "PrettyStackTraceLocationContext.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/ParentMap.h"
#include "clang/Analysis/Analyses/LiveVariables.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/SaveAndRestore.h"

using namespace clang;
using namespace ento;

#define DEBUG_TYPE "ExprEngine"

STATISTIC(NumOfDynamicDispatchPathSplits,
  "The # of times we split the path due to imprecise dynamic dispatch info");

STATISTIC(NumInlinedCalls,
  "The # of times we inlined a call");

STATISTIC(NumReachedInlineCountMax,
  "The # of times we reached inline count maximum");

void ExprEngine::processCallEnter(CallEnter CE, ExplodedNode *Pred) {
  // Get the entry block in the CFG of the callee.
  const StackFrameContext *calleeCtx = CE.getCalleeContext();
  PrettyStackTraceLocationContext CrashInfo(calleeCtx);

  const CFG *CalleeCFG = calleeCtx->getCFG();
  const CFGBlock *Entry = &(CalleeCFG->getEntry());
  
  // Validate the CFG.
  assert(Entry->empty());
  assert(Entry->succ_size() == 1);
  
  // Get the solitary successor.
  const CFGBlock *Succ = *(Entry->succ_begin());
  
  // Construct an edge representing the starting location in the callee.
  BlockEdge Loc(Entry, Succ, calleeCtx);

  ProgramStateRef state = Pred->getState();
  
  // Construct a new node and add it to the worklist.
  bool isNew;
  ExplodedNode *Node = G.getNode(Loc, state, false, &isNew);
  Node->addPredecessor(Pred, G);
  if (isNew)
    Engine.getWorkList()->enqueue(Node);
}

// Find the last statement on the path to the exploded node and the
// corresponding Block.
static std::pair<const Stmt*,
                 const CFGBlock*> getLastStmt(const ExplodedNode *Node) {
  const Stmt *S = nullptr;
  const CFGBlock *Blk = nullptr;
  const StackFrameContext *SF =
          Node->getLocation().getLocationContext()->getCurrentStackFrame();

  // Back up through the ExplodedGraph until we reach a statement node in this
  // stack frame.
  while (Node) {
    const ProgramPoint &PP = Node->getLocation();

    if (PP.getLocationContext()->getCurrentStackFrame() == SF) {
      if (Optional<StmtPoint> SP = PP.getAs<StmtPoint>()) {
        S = SP->getStmt();
        break;
      } else if (Optional<CallExitEnd> CEE = PP.getAs<CallExitEnd>()) {
        S = CEE->getCalleeContext()->getCallSite();
        if (S)
          break;

        // If there is no statement, this is an implicitly-generated call.
        // We'll walk backwards over it and then continue the loop to find
        // an actual statement.
        Optional<CallEnter> CE;
        do {
          Node = Node->getFirstPred();
          CE = Node->getLocationAs<CallEnter>();
        } while (!CE || CE->getCalleeContext() != CEE->getCalleeContext());

        // Continue searching the graph.
      } else if (Optional<BlockEdge> BE = PP.getAs<BlockEdge>()) {
        Blk = BE->getSrc();
      }
    } else if (Optional<CallEnter> CE = PP.getAs<CallEnter>()) {
      // If we reached the CallEnter for this function, it has no statements.
      if (CE->getCalleeContext() == SF)
        break;
    }

    if (Node->pred_empty())
      return std::make_pair(nullptr, nullptr);

    Node = *Node->pred_begin();
  }

  return std::make_pair(S, Blk);
}

/// Adjusts a return value when the called function's return type does not
/// match the caller's expression type. This can happen when a dynamic call
/// is devirtualized, and the overridding method has a covariant (more specific)
/// return type than the parent's method. For C++ objects, this means we need
/// to add base casts.
static SVal adjustReturnValue(SVal V, QualType ExpectedTy, QualType ActualTy,
                              StoreManager &StoreMgr) {
  // For now, the only adjustments we handle apply only to locations.
  if (!V.getAs<Loc>())
    return V;

  // If the types already match, don't do any unnecessary work.
  ExpectedTy = ExpectedTy.getCanonicalType();
  ActualTy = ActualTy.getCanonicalType();
  if (ExpectedTy == ActualTy)
    return V;

  // No adjustment is needed between Objective-C pointer types.
  if (ExpectedTy->isObjCObjectPointerType() &&
      ActualTy->isObjCObjectPointerType())
    return V;

  // C++ object pointers may need "derived-to-base" casts.
  const CXXRecordDecl *ExpectedClass = ExpectedTy->getPointeeCXXRecordDecl();
  const CXXRecordDecl *ActualClass = ActualTy->getPointeeCXXRecordDecl();
  if (ExpectedClass && ActualClass) {
    CXXBasePaths Paths(/*FindAmbiguities=*/true, /*RecordPaths=*/true,
                       /*DetectVirtual=*/false);
    if (ActualClass->isDerivedFrom(ExpectedClass, Paths) &&
        !Paths.isAmbiguous(ActualTy->getCanonicalTypeUnqualified())) {
      return StoreMgr.evalDerivedToBase(V, Paths.front());
    }
  }

  // Unfortunately, Objective-C does not enforce that overridden methods have
  // covariant return types, so we can't assert that that never happens.
  // Be safe and return UnknownVal().
  return UnknownVal();
}

void ExprEngine::removeDeadOnEndOfFunction(NodeBuilderContext& BC,
                                           ExplodedNode *Pred,
                                           ExplodedNodeSet &Dst) {
  // Find the last statement in the function and the corresponding basic block.
  const Stmt *LastSt = nullptr;
  const CFGBlock *Blk = nullptr;
  std::tie(LastSt, Blk) = getLastStmt(Pred);
  if (!Blk || !LastSt) {
    Dst.Add(Pred);
    return;
  }

  // Here, we destroy the current location context. We use the current
  // function's entire body as a diagnostic statement, with which the program
  // point will be associated. However, we only want to use LastStmt as a
  // reference for what to clean up if it's a ReturnStmt; otherwise, everything
  // is dead.
  SaveAndRestore<const NodeBuilderContext *> NodeContextRAII(currBldrCtx, &BC);
  const LocationContext *LCtx = Pred->getLocationContext();
  removeDead(Pred, Dst, dyn_cast<ReturnStmt>(LastSt), LCtx,
             LCtx->getAnalysisDeclContext()->getBody(),
             ProgramPoint::PostStmtPurgeDeadSymbolsKind);
}

static bool wasDifferentDeclUsedForInlining(CallEventRef<> Call,
    const StackFrameContext *calleeCtx) {
  const Decl *RuntimeCallee = calleeCtx->getDecl();
  const Decl *StaticDecl = Call->getDecl();
  assert(RuntimeCallee);
  if (!StaticDecl)
    return true;
  return RuntimeCallee->getCanonicalDecl() != StaticDecl->getCanonicalDecl();
}

/// Returns true if the CXXConstructExpr \p E was intended to construct a
/// prvalue for the region in \p V.
///
/// Note that we can't just test for rvalue vs. glvalue because
/// CXXConstructExprs embedded in DeclStmts and initializers are considered
/// rvalues by the AST, and the analyzer would like to treat them as lvalues.
static bool isTemporaryPRValue(const CXXConstructExpr *E, SVal V) {
  if (E->isGLValue())
    return false;

  const MemRegion *MR = V.getAsRegion();
  if (!MR)
    return false;

  return isa<CXXTempObjectRegion>(MR);
}

/// The call exit is simulated with a sequence of nodes, which occur between 
/// CallExitBegin and CallExitEnd. The following operations occur between the 
/// two program points:
/// 1. CallExitBegin (triggers the start of call exit sequence)
/// 2. Bind the return value
/// 3. Run Remove dead bindings to clean up the dead symbols from the callee.
/// 4. CallExitEnd (switch to the caller context)
/// 5. PostStmt<CallExpr>
void ExprEngine::processCallExit(ExplodedNode *CEBNode) {
  // Step 1 CEBNode was generated before the call.
  PrettyStackTraceLocationContext CrashInfo(CEBNode->getLocationContext());
  const StackFrameContext *calleeCtx =
      CEBNode->getLocationContext()->getCurrentStackFrame();
  
  // The parent context might not be a stack frame, so make sure we
  // look up the first enclosing stack frame.
  const StackFrameContext *callerCtx =
    calleeCtx->getParent()->getCurrentStackFrame();
  
  const Stmt *CE = calleeCtx->getCallSite();
  ProgramStateRef state = CEBNode->getState();
  // Find the last statement in the function and the corresponding basic block.
  const Stmt *LastSt = nullptr;
  const CFGBlock *Blk = nullptr;
  std::tie(LastSt, Blk) = getLastStmt(CEBNode);

  // Generate a CallEvent /before/ cleaning the state, so that we can get the
  // correct value for 'this' (if necessary).
  CallEventManager &CEMgr = getStateManager().getCallEventManager();
  CallEventRef<> Call = CEMgr.getCaller(calleeCtx, state);

  // Step 2: generate node with bound return value: CEBNode -> BindedRetNode.

  // If the callee returns an expression, bind its value to CallExpr.
  if (CE) {
    if (const ReturnStmt *RS = dyn_cast_or_null<ReturnStmt>(LastSt)) {
      const LocationContext *LCtx = CEBNode->getLocationContext();
      SVal V = state->getSVal(RS, LCtx);

      // Ensure that the return type matches the type of the returned Expr.
      if (wasDifferentDeclUsedForInlining(Call, calleeCtx)) {
        QualType ReturnedTy =
          CallEvent::getDeclaredResultType(calleeCtx->getDecl());
        if (!ReturnedTy.isNull()) {
          if (const Expr *Ex = dyn_cast<Expr>(CE)) {
            V = adjustReturnValue(V, Ex->getType(), ReturnedTy,
                                  getStoreManager());
          }
        }
      }

      state = state->BindExpr(CE, callerCtx, V);
    }

    // Bind the constructed object value to CXXConstructExpr.
    if (const CXXConstructExpr *CCE = dyn_cast<CXXConstructExpr>(CE)) {
      loc::MemRegionVal This =
        svalBuilder.getCXXThis(CCE->getConstructor()->getParent(), calleeCtx);
      SVal ThisV = state->getSVal(This);

      // If the constructed object is a temporary prvalue, get its bindings.
      if (isTemporaryPRValue(CCE, ThisV))
        ThisV = state->getSVal(ThisV.castAs<Loc>());

      state = state->BindExpr(CCE, callerCtx, ThisV);
    }
  }

  // Step 3: BindedRetNode -> CleanedNodes
  // If we can find a statement and a block in the inlined function, run remove
  // dead bindings before returning from the call. This is important to ensure
  // that we report the issues such as leaks in the stack contexts in which
  // they occurred.
  ExplodedNodeSet CleanedNodes;
  if (LastSt && Blk && AMgr.options.AnalysisPurgeOpt != PurgeNone) {
    static SimpleProgramPointTag retValBind("ExprEngine", "Bind Return Value");
    PostStmt Loc(LastSt, calleeCtx, &retValBind);
    bool isNew;
    ExplodedNode *BindedRetNode = G.getNode(Loc, state, false, &isNew);
    BindedRetNode->addPredecessor(CEBNode, G);
    if (!isNew)
      return;

    NodeBuilderContext Ctx(getCoreEngine(), Blk, BindedRetNode);
    currBldrCtx = &Ctx;
    // Here, we call the Symbol Reaper with 0 statement and callee location
    // context, telling it to clean up everything in the callee's context
    // (and its children). We use the callee's function body as a diagnostic
    // statement, with which the program point will be associated.
    removeDead(BindedRetNode, CleanedNodes, nullptr, calleeCtx,
               calleeCtx->getAnalysisDeclContext()->getBody(),
               ProgramPoint::PostStmtPurgeDeadSymbolsKind);
    currBldrCtx = nullptr;
  } else {
    CleanedNodes.Add(CEBNode);
  }

  for (ExplodedNodeSet::iterator I = CleanedNodes.begin(),
                                 E = CleanedNodes.end(); I != E; ++I) {

    // Step 4: Generate the CallExit and leave the callee's context.
    // CleanedNodes -> CEENode
    CallExitEnd Loc(calleeCtx, callerCtx);
    bool isNew;
    ProgramStateRef CEEState = (*I == CEBNode) ? state : (*I)->getState();
    ExplodedNode *CEENode = G.getNode(Loc, CEEState, false, &isNew);
    CEENode->addPredecessor(*I, G);
    if (!isNew)
      return;

    // Step 5: Perform the post-condition check of the CallExpr and enqueue the
    // result onto the work list.
    // CEENode -> Dst -> WorkList
    NodeBuilderContext Ctx(Engine, calleeCtx->getCallSiteBlock(), CEENode);
    SaveAndRestore<const NodeBuilderContext*> NBCSave(currBldrCtx,
        &Ctx);
    SaveAndRestore<unsigned> CBISave(currStmtIdx, calleeCtx->getIndex());

    CallEventRef<> UpdatedCall = Call.cloneWithState(CEEState);

    ExplodedNodeSet DstPostCall;
    getCheckerManager().runCheckersForPostCall(DstPostCall, CEENode,
                                               *UpdatedCall, *this,
                                               /*WasInlined=*/true);

    ExplodedNodeSet Dst;
    if (const ObjCMethodCall *Msg = dyn_cast<ObjCMethodCall>(Call)) {
      getCheckerManager().runCheckersForPostObjCMessage(Dst, DstPostCall, *Msg,
                                                        *this,
                                                        /*WasInlined=*/true);
    } else if (CE) {
      getCheckerManager().runCheckersForPostStmt(Dst, DstPostCall, CE,
                                                 *this, /*WasInlined=*/true);
    } else {
      Dst.insert(DstPostCall);
    }

    // Enqueue the next element in the block.
    for (ExplodedNodeSet::iterator PSI = Dst.begin(), PSE = Dst.end();
                                   PSI != PSE; ++PSI) {
      Engine.getWorkList()->enqueue(*PSI, calleeCtx->getCallSiteBlock(),
                                    calleeCtx->getIndex()+1);
    }
  }
}

void ExprEngine::examineStackFrames(const Decl *D, const LocationContext *LCtx,
                               bool &IsRecursive, unsigned &StackDepth) {
  IsRecursive = false;
  StackDepth = 0;

  while (LCtx) {
    if (const StackFrameContext *SFC = dyn_cast<StackFrameContext>(LCtx)) {
      const Decl *DI = SFC->getDecl();

      // Mark recursive (and mutually recursive) functions and always count
      // them when measuring the stack depth.
      if (DI == D) {
        IsRecursive = true;
        ++StackDepth;
        LCtx = LCtx->getParent();
        continue;
      }

      // Do not count the small functions when determining the stack depth.
      AnalysisDeclContext *CalleeADC = AMgr.getAnalysisDeclContext(DI);
      const CFG *CalleeCFG = CalleeADC->getCFG();
      if (CalleeCFG->getNumBlockIDs() > AMgr.options.getAlwaysInlineSize())
        ++StackDepth;
    }
    LCtx = LCtx->getParent();
  }

}

static bool IsInStdNamespace(const FunctionDecl *FD) {
  const DeclContext *DC = FD->getEnclosingNamespaceContext();
  const NamespaceDecl *ND = dyn_cast<NamespaceDecl>(DC);
  if (!ND)
    return false;

  while (const DeclContext *Parent = ND->getParent()) {
    if (!isa<NamespaceDecl>(Parent))
      break;
    ND = cast<NamespaceDecl>(Parent);
  }

  return ND->isStdNamespace();
}

// The GDM component containing the dynamic dispatch bifurcation info. When
// the exact type of the receiver is not known, we want to explore both paths -
// one on which we do inline it and the other one on which we don't. This is
// done to ensure we do not drop coverage.
// This is the map from the receiver region to a bool, specifying either we
// consider this region's information precise or not along the given path.
namespace {
  enum DynamicDispatchMode {
    DynamicDispatchModeInlined = 1,
    DynamicDispatchModeConservative
  };
}
REGISTER_TRAIT_WITH_PROGRAMSTATE(DynamicDispatchBifurcationMap,
                                 CLANG_ENTO_PROGRAMSTATE_MAP(const MemRegion *,
                                                             unsigned))

bool ExprEngine::inlineCall(const CallEvent &Call, const Decl *D,
                            NodeBuilder &Bldr, ExplodedNode *Pred,
                            ProgramStateRef State) {
  assert(D);

  const LocationContext *CurLC = Pred->getLocationContext();
  const StackFrameContext *CallerSFC = CurLC->getCurrentStackFrame();
  const LocationContext *ParentOfCallee = CallerSFC;
  if (Call.getKind() == CE_Block) {
    const BlockDataRegion *BR = cast<BlockCall>(Call).getBlockRegion();
    assert(BR && "If we have the block definition we should have its region");
    AnalysisDeclContext *BlockCtx = AMgr.getAnalysisDeclContext(D);
    ParentOfCallee = BlockCtx->getBlockInvocationContext(CallerSFC,
                                                         cast<BlockDecl>(D),
                                                         BR);
  }
  
  // This may be NULL, but that's fine.
  const Expr *CallE = Call.getOriginExpr();

  // Construct a new stack frame for the callee.
  AnalysisDeclContext *CalleeADC = AMgr.getAnalysisDeclContext(D);
  const StackFrameContext *CalleeSFC =
    CalleeADC->getStackFrame(ParentOfCallee, CallE,
                             currBldrCtx->getBlock(),
                             currStmtIdx);
  
    
  CallEnter Loc(CallE, CalleeSFC, CurLC);

  // Construct a new state which contains the mapping from actual to
  // formal arguments.
  State = State->enterStackFrame(Call, CalleeSFC);

  bool isNew;
  if (ExplodedNode *N = G.getNode(Loc, State, false, &isNew)) {
    N->addPredecessor(Pred, G);
    if (isNew)
      Engine.getWorkList()->enqueue(N);
  }

  // If we decided to inline the call, the successor has been manually
  // added onto the work list so remove it from the node builder.
  Bldr.takeNodes(Pred);

  NumInlinedCalls++;

  // Mark the decl as visited.
  if (VisitedCallees)
    VisitedCallees->insert(D);

  return true;
}

static ProgramStateRef getInlineFailedState(ProgramStateRef State,
                                            const Stmt *CallE) {
  const void *ReplayState = State->get<ReplayWithoutInlining>();
  if (!ReplayState)
    return nullptr;

  assert(ReplayState == CallE && "Backtracked to the wrong call.");
  (void)CallE;

  return State->remove<ReplayWithoutInlining>();
}

void ExprEngine::VisitCallExpr(const CallExpr *CE, ExplodedNode *Pred,
                               ExplodedNodeSet &dst) {
  // Perform the previsit of the CallExpr.
  ExplodedNodeSet dstPreVisit;
  getCheckerManager().runCheckersForPreStmt(dstPreVisit, Pred, CE, *this);

  // Get the call in its initial state. We use this as a template to perform
  // all the checks.
  CallEventManager &CEMgr = getStateManager().getCallEventManager();
  CallEventRef<> CallTemplate
    = CEMgr.getSimpleCall(CE, Pred->getState(), Pred->getLocationContext());

  // Evaluate the function call.  We try each of the checkers
  // to see if the can evaluate the function call.
  ExplodedNodeSet dstCallEvaluated;
  for (ExplodedNodeSet::iterator I = dstPreVisit.begin(), E = dstPreVisit.end();
       I != E; ++I) {
    evalCall(dstCallEvaluated, *I, *CallTemplate);
  }

  // Finally, perform the post-condition check of the CallExpr and store
  // the created nodes in 'Dst'.
  // Note that if the call was inlined, dstCallEvaluated will be empty.
  // The post-CallExpr check will occur in processCallExit.
  getCheckerManager().runCheckersForPostStmt(dst, dstCallEvaluated, CE,
                                             *this);
}

void ExprEngine::evalCall(ExplodedNodeSet &Dst, ExplodedNode *Pred,
                          const CallEvent &Call) {
  // WARNING: At this time, the state attached to 'Call' may be older than the
  // state in 'Pred'. This is a minor optimization since CheckerManager will
  // use an updated CallEvent instance when calling checkers, but if 'Call' is
  // ever used directly in this function all callers should be updated to pass
  // the most recent state. (It is probably not worth doing the work here since
  // for some callers this will not be necessary.)

  // Run any pre-call checks using the generic call interface.
  ExplodedNodeSet dstPreVisit;
  getCheckerManager().runCheckersForPreCall(dstPreVisit, Pred, Call, *this);

  // Actually evaluate the function call.  We try each of the checkers
  // to see if the can evaluate the function call, and get a callback at
  // defaultEvalCall if all of them fail.
  ExplodedNodeSet dstCallEvaluated;
  getCheckerManager().runCheckersForEvalCall(dstCallEvaluated, dstPreVisit,
                                             Call, *this);

  // Finally, run any post-call checks.
  getCheckerManager().runCheckersForPostCall(Dst, dstCallEvaluated,
                                             Call, *this);
}

ProgramStateRef ExprEngine::bindReturnValue(const CallEvent &Call,
                                            const LocationContext *LCtx,
                                            ProgramStateRef State) {
  const Expr *E = Call.getOriginExpr();
  if (!E)
    return State;

  // Some method families have known return values.
  if (const ObjCMethodCall *Msg = dyn_cast<ObjCMethodCall>(&Call)) {
    switch (Msg->getMethodFamily()) {
    default:
      break;
    case OMF_autorelease:
    case OMF_retain:
    case OMF_self: {
      // These methods return their receivers.
      return State->BindExpr(E, LCtx, Msg->getReceiverSVal());
    }
    }
  } else if (const CXXConstructorCall *C = dyn_cast<CXXConstructorCall>(&Call)){
    SVal ThisV = C->getCXXThisVal();

    // If the constructed object is a temporary prvalue, get its bindings.
    if (isTemporaryPRValue(cast<CXXConstructExpr>(E), ThisV))
      ThisV = State->getSVal(ThisV.castAs<Loc>());

    return State->BindExpr(E, LCtx, ThisV);
  }

  // Conjure a symbol if the return value is unknown.
  QualType ResultTy = Call.getResultType();
  SValBuilder &SVB = getSValBuilder();
  unsigned Count = currBldrCtx->blockCount();
  SVal R = SVB.conjureSymbolVal(nullptr, E, LCtx, ResultTy, Count);
  return State->BindExpr(E, LCtx, R);
}

// Conservatively evaluate call by invalidating regions and binding
// a conjured return value.
void ExprEngine::conservativeEvalCall(const CallEvent &Call, NodeBuilder &Bldr,
                                      ExplodedNode *Pred,
                                      ProgramStateRef State) {
  State = Call.invalidateRegions(currBldrCtx->blockCount(), State);
  State = bindReturnValue(Call, Pred->getLocationContext(), State);

  // And make the result node.
  Bldr.generateNode(Call.getProgramPoint(), State, Pred);
}

enum CallInlinePolicy {
  CIP_Allowed,
  CIP_DisallowedOnce,
  CIP_DisallowedAlways
};

static CallInlinePolicy mayInlineCallKind(const CallEvent &Call,
                                          const ExplodedNode *Pred,
                                          AnalyzerOptions &Opts) {
  const LocationContext *CurLC = Pred->getLocationContext();
  const StackFrameContext *CallerSFC = CurLC->getCurrentStackFrame();
  switch (Call.getKind()) {
  case CE_Function:
  case CE_Block:
    break;
  case CE_CXXMember:
  case CE_CXXMemberOperator:
    if (!Opts.mayInlineCXXMemberFunction(CIMK_MemberFunctions))
      return CIP_DisallowedAlways;
    break;
  case CE_CXXConstructor: {
    if (!Opts.mayInlineCXXMemberFunction(CIMK_Constructors))
      return CIP_DisallowedAlways;

    const CXXConstructorCall &Ctor = cast<CXXConstructorCall>(Call);

    // FIXME: We don't handle constructors or destructors for arrays properly.
    // Even once we do, we still need to be careful about implicitly-generated
    // initializers for array fields in default move/copy constructors.
    const MemRegion *Target = Ctor.getCXXThisVal().getAsRegion();
    if (Target && isa<ElementRegion>(Target))
      return CIP_DisallowedOnce;

    // FIXME: This is a hack. We don't use the correct region for a new
    // expression, so if we inline the constructor its result will just be
    // thrown away. This short-term hack is tracked in <rdar://problem/12180598>
    // and the longer-term possible fix is discussed in PR12014.
    const CXXConstructExpr *CtorExpr = Ctor.getOriginExpr();
    if (const Stmt *Parent = CurLC->getParentMap().getParent(CtorExpr))
      if (isa<CXXNewExpr>(Parent))
        return CIP_DisallowedOnce;

    // Inlining constructors requires including initializers in the CFG.
    const AnalysisDeclContext *ADC = CallerSFC->getAnalysisDeclContext();
    assert(ADC->getCFGBuildOptions().AddInitializers && "No CFG initializers");
    (void)ADC;

    // If the destructor is trivial, it's always safe to inline the constructor.
    if (Ctor.getDecl()->getParent()->hasTrivialDestructor())
      break;

    // For other types, only inline constructors if destructor inlining is
    // also enabled.
    if (!Opts.mayInlineCXXMemberFunction(CIMK_Destructors))
      return CIP_DisallowedAlways;

    // FIXME: This is a hack. We don't handle temporary destructors
    // right now, so we shouldn't inline their constructors.
    if (CtorExpr->getConstructionKind() == CXXConstructExpr::CK_Complete)
      if (!Target || !isa<DeclRegion>(Target))
        return CIP_DisallowedOnce;

    break;
  }
  case CE_CXXDestructor: {
    if (!Opts.mayInlineCXXMemberFunction(CIMK_Destructors))
      return CIP_DisallowedAlways;

    // Inlining destructors requires building the CFG correctly.
    const AnalysisDeclContext *ADC = CallerSFC->getAnalysisDeclContext();
    assert(ADC->getCFGBuildOptions().AddImplicitDtors && "No CFG destructors");
    (void)ADC;

    const CXXDestructorCall &Dtor = cast<CXXDestructorCall>(Call);

    // FIXME: We don't handle constructors or destructors for arrays properly.
    const MemRegion *Target = Dtor.getCXXThisVal().getAsRegion();
    if (Target && isa<ElementRegion>(Target))
      return CIP_DisallowedOnce;

    break;
  }
  case CE_CXXAllocator:
    if (Opts.mayInlineCXXAllocator())
      break;
    // Do not inline allocators until we model deallocators.
    // This is unfortunate, but basically necessary for smart pointers and such.
    return CIP_DisallowedAlways;
  case CE_ObjCMessage:
    if (!Opts.mayInlineObjCMethod())
      return CIP_DisallowedAlways;
    if (!(Opts.getIPAMode() == IPAK_DynamicDispatch ||
          Opts.getIPAMode() == IPAK_DynamicDispatchBifurcate))
      return CIP_DisallowedAlways;
    break;
  }

  return CIP_Allowed;
}

/// Returns true if the given C++ class contains a member with the given name.
static bool hasMember(const ASTContext &Ctx, const CXXRecordDecl *RD,
                      StringRef Name) {
  const IdentifierInfo &II = Ctx.Idents.get(Name);
  DeclarationName DeclName = Ctx.DeclarationNames.getIdentifier(&II);
  if (!RD->lookup(DeclName).empty())
    return true;

  CXXBasePaths Paths(false, false, false);
  if (RD->lookupInBases(&CXXRecordDecl::FindOrdinaryMember,
                        DeclName.getAsOpaquePtr(),
                        Paths))
    return true;

  return false;
}

/// Returns true if the given C++ class is a container or iterator.
///
/// Our heuristic for this is whether it contains a method named 'begin()' or a
/// nested type named 'iterator' or 'iterator_category'.
static bool isContainerClass(const ASTContext &Ctx, const CXXRecordDecl *RD) {
  return hasMember(Ctx, RD, "begin") ||
         hasMember(Ctx, RD, "iterator") ||
         hasMember(Ctx, RD, "iterator_category");
}

/// Returns true if the given function refers to a method of a C++ container
/// or iterator.
///
/// We generally do a poor job modeling most containers right now, and might
/// prefer not to inline their methods.
static bool isContainerMethod(const ASTContext &Ctx,
                              const FunctionDecl *FD) {
  if (const CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(FD))
    return isContainerClass(Ctx, MD->getParent());
  return false;
}

/// Returns true if the given function is the destructor of a class named
/// "shared_ptr".
static bool isCXXSharedPtrDtor(const FunctionDecl *FD) {
  const CXXDestructorDecl *Dtor = dyn_cast<CXXDestructorDecl>(FD);
  if (!Dtor)
    return false;

  const CXXRecordDecl *RD = Dtor->getParent();
  if (const IdentifierInfo *II = RD->getDeclName().getAsIdentifierInfo())
    if (II->isStr("shared_ptr"))
        return true;

  return false;
}

/// Returns true if the function in \p CalleeADC may be inlined in general.
///
/// This checks static properties of the function, such as its signature and
/// CFG, to determine whether the analyzer should ever consider inlining it,
/// in any context.
static bool mayInlineDecl(AnalysisDeclContext *CalleeADC,
                          AnalyzerOptions &Opts) {
  // FIXME: Do not inline variadic calls.
  if (CallEvent::isVariadic(CalleeADC->getDecl()))
    return false;

  // Check certain C++-related inlining policies.
  ASTContext &Ctx = CalleeADC->getASTContext();
  if (Ctx.getLangOpts().CPlusPlus) {
    if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(CalleeADC->getDecl())) {
      // Conditionally control the inlining of template functions.
      if (!Opts.mayInlineTemplateFunctions())
        if (FD->getTemplatedKind() != FunctionDecl::TK_NonTemplate)
          return false;

      // Conditionally control the inlining of C++ standard library functions.
      if (!Opts.mayInlineCXXStandardLibrary())
        if (Ctx.getSourceManager().isInSystemHeader(FD->getLocation()))
          if (IsInStdNamespace(FD))
            return false;

      // Conditionally control the inlining of methods on objects that look
      // like C++ containers.
      if (!Opts.mayInlineCXXContainerMethods())
        if (!Ctx.getSourceManager().isInMainFile(FD->getLocation()))
          if (isContainerMethod(Ctx, FD))
            return false;
            
      // Conditionally control the inlining of the destructor of C++ shared_ptr.
      // We don't currently do a good job modeling shared_ptr because we can't
      // see the reference count, so treating as opaque is probably the best
      // idea.
      if (!Opts.mayInlineCXXSharedPtrDtor())
        if (isCXXSharedPtrDtor(FD))
          return false;

    }
  }

  // It is possible that the CFG cannot be constructed.
  // Be safe, and check if the CalleeCFG is valid.
  const CFG *CalleeCFG = CalleeADC->getCFG();
  if (!CalleeCFG)
    return false;

  // Do not inline large functions.
  if (CalleeCFG->getNumBlockIDs() > Opts.getMaxInlinableSize())
    return false;

  // It is possible that the live variables analysis cannot be
  // run.  If so, bail out.
  if (!CalleeADC->getAnalysis<RelaxedLiveVariables>())
    return false;

  return true;
}

bool ExprEngine::shouldInlineCall(const CallEvent &Call, const Decl *D,
                                  const ExplodedNode *Pred) {
  if (!D)
    return false;

  AnalysisManager &AMgr = getAnalysisManager();
  AnalyzerOptions &Opts = AMgr.options;
  AnalysisDeclContextManager &ADCMgr = AMgr.getAnalysisDeclContextManager();
  AnalysisDeclContext *CalleeADC = ADCMgr.getContext(D);

  // Temporary object destructor processing is currently broken, so we never
  // inline them.
  // FIXME: Remove this once temp destructors are working.
  if (isa<CXXDestructorCall>(Call)) {
    if ((*currBldrCtx->getBlock())[currStmtIdx].getAs<CFGTemporaryDtor>())
      return false;
  }

  // The auto-synthesized bodies are essential to inline as they are
  // usually small and commonly used. Note: we should do this check early on to
  // ensure we always inline these calls.
  if (CalleeADC->isBodyAutosynthesized())
    return true;

  if (!AMgr.shouldInlineCall())
    return false;

  // Check if this function has been marked as non-inlinable.
  Optional<bool> MayInline = Engine.FunctionSummaries->mayInline(D);
  if (MayInline.hasValue()) {
    if (!MayInline.getValue())
      return false;

  } else {
    // We haven't actually checked the static properties of this function yet.
    // Do that now, and record our decision in the function summaries.
    if (mayInlineDecl(CalleeADC, Opts)) {
      Engine.FunctionSummaries->markMayInline(D);
    } else {
      Engine.FunctionSummaries->markShouldNotInline(D);
      return false;
    }
  }

  // Check if we should inline a call based on its kind.
  // FIXME: this checks both static and dynamic properties of the call, which
  // means we're redoing a bit of work that could be cached in the function
  // summary.
  CallInlinePolicy CIP = mayInlineCallKind(Call, Pred, Opts);
  if (CIP != CIP_Allowed) {
    if (CIP == CIP_DisallowedAlways) {
      assert(!MayInline.hasValue() || MayInline.getValue());
      Engine.FunctionSummaries->markShouldNotInline(D);
    }
    return false;
  }

  const CFG *CalleeCFG = CalleeADC->getCFG();

  // Do not inline if recursive or we've reached max stack frame count.
  bool IsRecursive = false;
  unsigned StackDepth = 0;
  examineStackFrames(D, Pred->getLocationContext(), IsRecursive, StackDepth);
  if ((StackDepth >= Opts.InlineMaxStackDepth) &&
      ((CalleeCFG->getNumBlockIDs() > Opts.getAlwaysInlineSize())
       || IsRecursive))
    return false;

  // Do not inline large functions too many times.
  if ((Engine.FunctionSummaries->getNumTimesInlined(D) >
       Opts.getMaxTimesInlineLarge()) &&
      CalleeCFG->getNumBlockIDs() > 13) {
    NumReachedInlineCountMax++;
    return false;
  }

  if (HowToInline == Inline_Minimal &&
      (CalleeCFG->getNumBlockIDs() > Opts.getAlwaysInlineSize()
      || IsRecursive))
    return false;

  Engine.FunctionSummaries->bumpNumTimesInlined(D);

  return true;
}

static bool isTrivialObjectAssignment(const CallEvent &Call) {
  const CXXInstanceCall *ICall = dyn_cast<CXXInstanceCall>(&Call);
  if (!ICall)
    return false;

  const CXXMethodDecl *MD = dyn_cast_or_null<CXXMethodDecl>(ICall->getDecl());
  if (!MD)
    return false;
  if (!(MD->isCopyAssignmentOperator() || MD->isMoveAssignmentOperator()))
    return false;

  return MD->isTrivial();
}

void ExprEngine::defaultEvalCall(NodeBuilder &Bldr, ExplodedNode *Pred,
                                 const CallEvent &CallTemplate) {
  // Make sure we have the most recent state attached to the call.
  ProgramStateRef State = Pred->getState();
  CallEventRef<> Call = CallTemplate.cloneWithState(State);

  // Special-case trivial assignment operators.
  if (isTrivialObjectAssignment(*Call)) {
    performTrivialCopy(Bldr, Pred, *Call);
    return;
  }

  // Try to inline the call.
  // The origin expression here is just used as a kind of checksum;
  // this should still be safe even for CallEvents that don't come from exprs.
  const Expr *E = Call->getOriginExpr();

  ProgramStateRef InlinedFailedState = getInlineFailedState(State, E);
  if (InlinedFailedState) {
    // If we already tried once and failed, make sure we don't retry later.
    State = InlinedFailedState;
  } else {
    RuntimeDefinition RD = Call->getRuntimeDefinition();
    const Decl *D = RD.getDecl();
    if (shouldInlineCall(*Call, D, Pred)) {
      if (RD.mayHaveOtherDefinitions()) {
        AnalyzerOptions &Options = getAnalysisManager().options;

        // Explore with and without inlining the call.
        if (Options.getIPAMode() == IPAK_DynamicDispatchBifurcate) {
          BifurcateCall(RD.getDispatchRegion(), *Call, D, Bldr, Pred);
          return;
        }

        // Don't inline if we're not in any dynamic dispatch mode.
        if (Options.getIPAMode() != IPAK_DynamicDispatch) {
          conservativeEvalCall(*Call, Bldr, Pred, State);
          return;
        }
      }

      // We are not bifurcating and we do have a Decl, so just inline.
      if (inlineCall(*Call, D, Bldr, Pred, State))
        return;
    }
  }

  // If we can't inline it, handle the return value and invalidate the regions.
  conservativeEvalCall(*Call, Bldr, Pred, State);
}

void ExprEngine::BifurcateCall(const MemRegion *BifurReg,
                               const CallEvent &Call, const Decl *D,
                               NodeBuilder &Bldr, ExplodedNode *Pred) {
  assert(BifurReg);
  BifurReg = BifurReg->StripCasts();

  // Check if we've performed the split already - note, we only want
  // to split the path once per memory region.
  ProgramStateRef State = Pred->getState();
  const unsigned *BState =
                        State->get<DynamicDispatchBifurcationMap>(BifurReg);
  if (BState) {
    // If we are on "inline path", keep inlining if possible.
    if (*BState == DynamicDispatchModeInlined)
      if (inlineCall(Call, D, Bldr, Pred, State))
        return;
    // If inline failed, or we are on the path where we assume we
    // don't have enough info about the receiver to inline, conjure the
    // return value and invalidate the regions.
    conservativeEvalCall(Call, Bldr, Pred, State);
    return;
  }

  // If we got here, this is the first time we process a message to this
  // region, so split the path.
  ProgramStateRef IState =
      State->set<DynamicDispatchBifurcationMap>(BifurReg,
                                               DynamicDispatchModeInlined);
  inlineCall(Call, D, Bldr, Pred, IState);

  ProgramStateRef NoIState =
      State->set<DynamicDispatchBifurcationMap>(BifurReg,
                                               DynamicDispatchModeConservative);
  conservativeEvalCall(Call, Bldr, Pred, NoIState);

  NumOfDynamicDispatchPathSplits++;
  return;
}


void ExprEngine::VisitReturnStmt(const ReturnStmt *RS, ExplodedNode *Pred,
                                 ExplodedNodeSet &Dst) {
  
  ExplodedNodeSet dstPreVisit;
  getCheckerManager().runCheckersForPreStmt(dstPreVisit, Pred, RS, *this);

  StmtNodeBuilder B(dstPreVisit, Dst, *currBldrCtx);
  
  if (RS->getRetValue()) {
    for (ExplodedNodeSet::iterator it = dstPreVisit.begin(),
                                  ei = dstPreVisit.end(); it != ei; ++it) {
      B.generateNode(RS, *it, (*it)->getState());
    }
  }
}
