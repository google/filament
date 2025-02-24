//===- subzero/src/IceCfg.cpp - Control flow graph implementation ---------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the Cfg class.
///
//===----------------------------------------------------------------------===//

#include "IceCfg.h"

#include "IceAssembler.h"
#include "IceBitVector.h"
#include "IceCfgNode.h"
#include "IceClFlags.h"
#include "IceDefs.h"
#include "IceELFObjectWriter.h"
#include "IceGlobalInits.h"
#include "IceInst.h"
#include "IceInstVarIter.h"
#include "IceInstrumentation.h"
#include "IceLiveness.h"
#include "IceLoopAnalyzer.h"
#include "IceOperand.h"
#include "IceTargetLowering.h"

#include <memory>
#include <utility>

namespace Ice {

Cfg::Cfg(GlobalContext *Ctx, uint32_t SequenceNumber)
    : Allocator(createAllocator()), Ctx(Ctx), SequenceNumber(SequenceNumber),
      VMask(getFlags().getVerbose()), FunctionName(),
      NextInstNumber(Inst::NumberInitial), Live(nullptr) {
  NodeStrings.reset(new StringPool);
  VarStrings.reset(new StringPool);
  CfgLocalAllocatorScope _(this);
  Target = TargetLowering::createLowering(getFlags().getTargetArch(), this);
  VMetadata.reset(new VariablesMetadata(this));
  TargetAssembler = Target->createAssembler();
}

Cfg::~Cfg() {
  assert(CfgAllocatorTraits::current() == nullptr);
  if (getFlags().getDumpStrings()) {
    OstreamLocker _(Ctx);
    Ostream &Str = Ctx->getStrDump();
    getNodeStrings()->dump(Str);
    getVarStrings()->dump(Str);
  }
}

// Called in the initalizer list of Cfg's constructor to create the Allocator
// and set it as TLS before any other member fields are constructed, since they
// may depend on it.
ArenaAllocator *Cfg::createAllocator() {
  ArenaAllocator *Allocator = new ArenaAllocator();
  CfgAllocatorTraits::set_current(Allocator);
  return Allocator;
}

/// Create a string like "foo(i=123:b=9)" indicating the function name, number
/// of high-level instructions, and number of basic blocks.  This string is only
/// used for dumping and other diagnostics, and the idea is that given a set of
/// functions to debug a problem on, it's easy to find the smallest or simplest
/// function to attack.  Note that the counts may change somewhat depending on
/// what point it is called during the translation passes.
std::string Cfg::getFunctionNameAndSize() const {
  if (!BuildDefs::dump())
    return getFunctionName().toString();
  SizeT NodeCount = 0;
  SizeT InstCount = 0;
  for (CfgNode *Node : getNodes()) {
    ++NodeCount;
    // Note: deleted instructions are *not* ignored.
    InstCount += Node->getPhis().size();
    for (Inst &I : Node->getInsts()) {
      if (!llvm::isa<InstTarget>(&I))
        ++InstCount;
    }
  }
  return getFunctionName() + "(i=" + std::to_string(InstCount) +
         ":b=" + std::to_string(NodeCount) + ")";
}

void Cfg::setError(const std::string &Message) {
  HasError = true;
  ErrorMessage = Message;
}

CfgNode *Cfg::makeNode() {
  SizeT LabelIndex = Nodes.size();
  auto *Node = CfgNode::create(this, LabelIndex);
  Nodes.push_back(Node);
  return Node;
}

void Cfg::swapNodes(NodeList &NewNodes) {
  assert(Nodes.size() == NewNodes.size());
  Nodes.swap(NewNodes);
  for (SizeT I = 0, NumNodes = getNumNodes(); I < NumNodes; ++I)
    Nodes[I]->resetIndex(I);
}

template <> Variable *Cfg::makeVariable<Variable>(Type Ty) {
  SizeT Index = Variables.size();
  Variable *Var;
  if (Target->shouldSplitToVariableVecOn32(Ty)) {
    Var = VariableVecOn32::create(this, Ty, Index);
  } else if (Target->shouldSplitToVariable64On32(Ty)) {
    Var = Variable64On32::create(this, Ty, Index);
  } else {
    Var = Variable::create(this, Ty, Index);
  }
  Variables.push_back(Var);
  return Var;
}

void Cfg::addArg(Variable *Arg) {
  Arg->setIsArg();
  Args.push_back(Arg);
}

void Cfg::addImplicitArg(Variable *Arg) {
  Arg->setIsImplicitArg();
  ImplicitArgs.push_back(Arg);
}

// Returns whether the stack frame layout has been computed yet. This is used
// for dumping the stack frame location of Variables.
bool Cfg::hasComputedFrame() const { return getTarget()->hasComputedFrame(); }

namespace {
constexpr char BlockNameGlobalPrefix[] = ".L$profiler$block_name$";
constexpr char BlockStatsGlobalPrefix[] = ".L$profiler$block_info$";
} // end of anonymous namespace

void Cfg::createNodeNameDeclaration(const std::string &NodeAsmName) {
  auto *Var = VariableDeclaration::create(GlobalInits.get());
  Var->setName(Ctx, BlockNameGlobalPrefix + NodeAsmName);
  Var->setIsConstant(true);
  Var->addInitializer(VariableDeclaration::DataInitializer::create(
      GlobalInits.get(), NodeAsmName.data(), NodeAsmName.size() + 1));
  const SizeT Int64ByteSize = typeWidthInBytes(IceType_i64);
  Var->setAlignment(Int64ByteSize); // Wasteful, 32-bit could use 4 bytes.
  GlobalInits->push_back(Var);
}

void Cfg::createBlockProfilingInfoDeclaration(
    const std::string &NodeAsmName, VariableDeclaration *NodeNameDeclaration) {
  auto *Var = VariableDeclaration::create(GlobalInits.get());
  Var->setName(Ctx, BlockStatsGlobalPrefix + NodeAsmName);
  const SizeT Int64ByteSize = typeWidthInBytes(IceType_i64);
  Var->addInitializer(VariableDeclaration::ZeroInitializer::create(
      GlobalInits.get(), Int64ByteSize));

  const RelocOffsetT NodeNameDeclarationOffset = 0;
  Var->addInitializer(VariableDeclaration::RelocInitializer::create(
      GlobalInits.get(), NodeNameDeclaration,
      {RelocOffset::create(Ctx, NodeNameDeclarationOffset)}));
  Var->setAlignment(Int64ByteSize);
  GlobalInits->push_back(Var);
}

void Cfg::translate() {
  if (hasError())
    return;
  // Cache the possibly-overridden optimization level once translation begins.
  // It would be nicer to do this in the constructor, but we need to wait until
  // after setFunctionName() has a chance to be called.
  OptimizationLevel =
      getFlags().matchForceO2(getFunctionName(), getSequenceNumber())
          ? Opt_2
          : getFlags().getOptLevel();
  if (BuildDefs::timers()) {
    if (getFlags().matchTimingFocus(getFunctionName(), getSequenceNumber())) {
      setFocusedTiming();
      getContext()->resetTimer(GlobalContext::TSK_Default);
    }
  }
  if (BuildDefs::dump()) {
    if (isVerbose(IceV_Status) &&
        getFlags().matchTestStatus(getFunctionName(), getSequenceNumber())) {
      getContext()->getStrDump()
          << ">>>Translating " << getFunctionNameAndSize()
          << " seq=" << getSequenceNumber() << "\n";
    }
  }
  TimerMarker T_func(getContext(), getFunctionName().toStringOrEmpty());
  TimerMarker T(TimerStack::TT_translate, this);

  dump("Initial CFG");

  // Create the Hi and Lo variables where a split was needed
  for (Variable *Var : Variables) {
    if (auto *Var64On32 = llvm::dyn_cast<Variable64On32>(Var)) {
      Var64On32->initHiLo(this);
    } else if (auto *VarVecOn32 = llvm::dyn_cast<VariableVecOn32>(Var)) {
      VarVecOn32->initVecElement(this);
    }
  }

  // Instrument the Cfg, e.g. with AddressSanitizer
  if (!BuildDefs::minimal() && getFlags().getSanitizeAddresses()) {
    getContext()->instrumentFunc(this);
    dump("Instrumented CFG");
  }

  // The set of translation passes and their order are determined by the
  // target.
  getTarget()->translate();

  dump("Final output");
  if (getFocusedTiming()) {
    getContext()->dumpLocalTimers(getFunctionName().toString());
  }
}

void Cfg::fixPhiNodes() {
  for (auto *Node : Nodes) {
    // Fix all the phi edges since WASM can't tell how to make them correctly at
    // the beginning.
    assert(Node);
    const auto &InEdges = Node->getInEdges();
    for (auto &Instr : Node->getPhis()) {
      auto *Phi = llvm::cast<InstPhi>(&Instr);
      assert(Phi);
      for (SizeT i = 0; i < InEdges.size(); ++i) {
        Phi->setLabel(i, InEdges[i]);
      }
    }
  }
}

void Cfg::computeInOutEdges() {
  // Compute the out-edges.
  for (CfgNode *Node : Nodes) {
    Node->computeSuccessors();
  }

  // Prune any unreachable nodes before computing in-edges.
  SizeT NumNodes = getNumNodes();
  BitVector Reachable(NumNodes);
  BitVector Pending(NumNodes);
  Pending.set(getEntryNode()->getIndex());
  while (true) {
    int Index = Pending.find_first();
    if (Index == -1)
      break;
    Pending.reset(Index);
    Reachable.set(Index);
    CfgNode *Node = Nodes[Index];
    assert(Node->getIndex() == (SizeT)Index);
    for (CfgNode *Succ : Node->getOutEdges()) {
      SizeT SuccIndex = Succ->getIndex();
      if (!Reachable.test(SuccIndex))
        Pending.set(SuccIndex);
    }
  }
  SizeT Dest = 0;
  for (SizeT Source = 0; Source < NumNodes; ++Source) {
    if (Reachable.test(Source)) {
      Nodes[Dest] = Nodes[Source];
      Nodes[Dest]->resetIndex(Dest);
      // Compute the in-edges.
      Nodes[Dest]->computePredecessors();
      ++Dest;
    }
  }
  Nodes.resize(Dest);

  TimerMarker T(TimerStack::TT_phiValidation, this);
  for (CfgNode *Node : Nodes)
    Node->enforcePhiConsistency();
}

void Cfg::renumberInstructions() {
  TimerMarker T(TimerStack::TT_renumberInstructions, this);
  NextInstNumber = Inst::NumberInitial;
  for (CfgNode *Node : Nodes)
    Node->renumberInstructions();
  // Make sure the entry node is the first node and therefore got the lowest
  // instruction numbers, to facilitate live range computation of function
  // arguments.  We want to model function arguments as being live on entry to
  // the function, otherwise an argument whose only use is in the first
  // instruction will be assigned a trivial live range and the register
  // allocator will not recognize its live range as overlapping another
  // variable's live range.
  assert(Nodes.empty() || (*Nodes.begin() == getEntryNode()));
}

// placePhiLoads() must be called before placePhiStores().
void Cfg::placePhiLoads() {
  TimerMarker T(TimerStack::TT_placePhiLoads, this);
  for (CfgNode *Node : Nodes)
    Node->placePhiLoads();
}

// placePhiStores() must be called after placePhiLoads().
void Cfg::placePhiStores() {
  TimerMarker T(TimerStack::TT_placePhiStores, this);
  for (CfgNode *Node : Nodes)
    Node->placePhiStores();
}

void Cfg::deletePhis() {
  TimerMarker T(TimerStack::TT_deletePhis, this);
  for (CfgNode *Node : Nodes)
    Node->deletePhis();
}

void Cfg::advancedPhiLowering() {
  TimerMarker T(TimerStack::TT_advancedPhiLowering, this);
  // Clear all previously computed live ranges (but not live-in/live-out bit
  // vectors or last-use markers), because the follow-on register allocation is
  // only concerned with live ranges across the newly created blocks.
  for (Variable *Var : Variables) {
    Var->getLiveRange().reset();
  }
  // This splits edges and appends new nodes to the end of the node list. This
  // can invalidate iterators, so don't use an iterator.
  SizeT NumNodes = getNumNodes();
  SizeT NumVars = getNumVariables();
  for (SizeT I = 0; I < NumNodes; ++I)
    Nodes[I]->advancedPhiLowering();

  TimerMarker TT(TimerStack::TT_lowerPhiAssignments, this);
  if (true) {
    // The following code does an in-place update of liveness and live ranges
    // as a result of adding the new phi edge split nodes.
    getLiveness()->initPhiEdgeSplits(Nodes.begin() + NumNodes,
                                     Variables.begin() + NumVars);
    TimerMarker TTT(TimerStack::TT_liveness, this);
    // Iterate over the newly added nodes to add their liveness info.
    for (auto I = Nodes.begin() + NumNodes, E = Nodes.end(); I != E; ++I) {
      InstNumberT FirstInstNum = getNextInstNumber();
      (*I)->renumberInstructions();
      InstNumberT LastInstNum = getNextInstNumber() - 1;
      (*I)->liveness(getLiveness());
      (*I)->livenessAddIntervals(getLiveness(), FirstInstNum, LastInstNum);
    }
  } else {
    // The following code does a brute-force recalculation of live ranges as a
    // result of adding the new phi edge split nodes. The liveness calculation
    // is particularly expensive because the new nodes are not yet in a proper
    // topological order and so convergence is slower.
    //
    // This code is kept here for reference and can be temporarily enabled in
    // case the incremental code is under suspicion.
    renumberInstructions();
    liveness(Liveness_Intervals);
    getVMetadata()->init(VMK_All);
  }
  Target->regAlloc(RAK_Phi);
}

// Find a reasonable placement for nodes that have not yet been placed, while
// maintaining the same relative ordering among already placed nodes.
void Cfg::reorderNodes() {
  // TODO(ascull): it would be nice if the switch tests were always followed by
  // the default case to allow for fall through.
  using PlacedList = CfgList<CfgNode *>;
  PlacedList Placed;      // Nodes with relative placement locked down
  PlacedList Unreachable; // Unreachable nodes
  PlacedList::iterator NoPlace = Placed.end();
  // Keep track of where each node has been tentatively placed so that we can
  // manage insertions into the middle.
  CfgVector<PlacedList::iterator> PlaceIndex(Nodes.size(), NoPlace);
  for (CfgNode *Node : Nodes) {
    // The "do ... while(0);" construct is to factor out the --PlaceIndex and
    // assert() statements before moving to the next node.
    do {
      if (Node != getEntryNode() && Node->getInEdges().empty()) {
        // The node has essentially been deleted since it is not a successor of
        // any other node.
        Unreachable.push_back(Node);
        PlaceIndex[Node->getIndex()] = Unreachable.end();
        Node->setNeedsPlacement(false);
        continue;
      }
      if (!Node->needsPlacement()) {
        // Add to the end of the Placed list.
        Placed.push_back(Node);
        PlaceIndex[Node->getIndex()] = Placed.end();
        continue;
      }
      Node->setNeedsPlacement(false);
      // Assume for now that the unplaced node is from edge-splitting and
      // therefore has 1 in-edge and 1 out-edge (actually, possibly more than 1
      // in-edge if the predecessor node was contracted). If this changes in
      // the future, rethink the strategy.
      assert(Node->getInEdges().size() >= 1);
      assert(Node->hasSingleOutEdge());

      // If it's a (non-critical) edge where the successor has a single
      // in-edge, then place it before the successor.
      CfgNode *Succ = Node->getOutEdges().front();
      if (Succ->getInEdges().size() == 1 &&
          PlaceIndex[Succ->getIndex()] != NoPlace) {
        Placed.insert(PlaceIndex[Succ->getIndex()], Node);
        PlaceIndex[Node->getIndex()] = PlaceIndex[Succ->getIndex()];
        continue;
      }

      // Otherwise, place it after the (first) predecessor.
      CfgNode *Pred = Node->getInEdges().front();
      auto PredPosition = PlaceIndex[Pred->getIndex()];
      // It shouldn't be the case that PredPosition==NoPlace, but if that
      // somehow turns out to be true, we just insert Node before
      // PredPosition=NoPlace=Placed.end() .
      if (PredPosition != NoPlace)
        ++PredPosition;
      Placed.insert(PredPosition, Node);
      PlaceIndex[Node->getIndex()] = PredPosition;
    } while (0);

    --PlaceIndex[Node->getIndex()];
    assert(*PlaceIndex[Node->getIndex()] == Node);
  }

  // Reorder Nodes according to the built-up lists.
  NodeList Reordered;
  Reordered.reserve(Placed.size() + Unreachable.size());
  for (CfgNode *Node : Placed)
    Reordered.push_back(Node);
  for (CfgNode *Node : Unreachable)
    Reordered.push_back(Node);
  assert(getNumNodes() == Reordered.size());
  swapNodes(Reordered);
}

void Cfg::localCSE(bool AssumeSSA) {
  // Performs basic-block local common-subexpression elimination
  // If we have
  // t1 = op b c
  // t2 = op b c
  // This pass will replace future references to t2 in a basic block by t1
  // Points to note:
  // 1. Assumes SSA by default. To change this, use -lcse=no-ssa
  //      This is needed if this pass is moved to a point later in the pipeline.
  //      If variables have a single definition (in the node), CSE can work just
  //      on the basis of an equality compare on instructions (sans Dest). When
  //      variables can be updated (hence, non-SSA) the result of a previous
  //      instruction which used that variable as an operand can not be reused.
  // 2. Leaves removal of instructions to DCE.
  // 3. Only enabled on arithmetic instructions. pnacl-clang (-O2) is expected
  //    to take care of cases not arising from GEP simplification.
  // 4. By default, a single pass is made over each basic block. Control this
  //    with -lcse-max-iters=N

  TimerMarker T(TimerStack::TT_localCse, this);
  struct VariableHash {
    size_t operator()(const Variable *Var) const { return Var->hashValue(); }
  };

  struct InstHash {
    size_t operator()(const Inst *Instr) const {
      auto Kind = Instr->getKind();
      auto Result =
          std::hash<typename std::underlying_type<Inst::InstKind>::type>()(
              Kind);
      for (SizeT i = 0; i < Instr->getSrcSize(); ++i) {
        Result ^= Instr->getSrc(i)->hashValue();
      }
      return Result;
    }
  };
  struct InstEq {
    bool srcEq(const Operand *A, const Operand *B) const {
      if (llvm::isa<Variable>(A) || llvm::isa<Constant>(A))
        return (A == B);
      return false;
    }
    bool operator()(const Inst *InstrA, const Inst *InstrB) const {
      if ((InstrA->getKind() != InstrB->getKind()) ||
          (InstrA->getSrcSize() != InstrB->getSrcSize()))
        return false;

      if (auto *A = llvm::dyn_cast<InstArithmetic>(InstrA)) {
        auto *B = llvm::cast<InstArithmetic>(InstrB);
        // A, B are guaranteed to be of the same 'kind' at this point
        // So, dyn_cast is not needed
        if (A->getOp() != B->getOp())
          return false;
      }
      // Does not enter loop if different kind or number of operands
      for (SizeT i = 0; i < InstrA->getSrcSize(); ++i) {
        if (!srcEq(InstrA->getSrc(i), InstrB->getSrc(i)))
          return false;
      }
      return true;
    }
  };

  for (CfgNode *Node : getNodes()) {
    CfgUnorderedSet<Inst *, InstHash, InstEq> Seen;
    // Stores currently available instructions.

    CfgUnorderedMap<Variable *, Variable *, VariableHash> Replacements;
    // Combining the above two into a single data structure might consume less
    // memory but will be slower i.e map of Instruction -> Set of Variables

    CfgUnorderedMap<Variable *, std::vector<Inst *>, VariableHash> Dependency;
    // Maps a variable to the Instructions that depend on it.
    // a = op1 b c
    // x = op2 c d
    // Will result in the map : b -> {a}, c -> {a, x}, d -> {x}
    // Not necessary for SSA as dependencies will never be invalidated, and the
    // container will use minimal memory when left unused.

    auto IterCount = getFlags().getLocalCseMaxIterations();

    for (uint32_t i = 0; i < IterCount; ++i) {
      // TODO(manasijm): Stats on IterCount -> performance
      for (Inst &Instr : Node->getInsts()) {
        if (Instr.isDeleted() || !llvm::isa<InstArithmetic>(&Instr))
          continue;
        if (!AssumeSSA) {
          // Invalidate replacements
          auto Iter = Replacements.find(Instr.getDest());
          if (Iter != Replacements.end()) {
            Replacements.erase(Iter);
          }

          // Invalidate 'seen' instructions whose operands were just updated.
          auto DepIter = Dependency.find(Instr.getDest());
          if (DepIter != Dependency.end()) {
            for (auto *DepInst : DepIter->second) {
              Seen.erase(DepInst);
            }
          }
        }

        // Replace - doing this before checking for repetitions might enable
        // more optimizations
        for (SizeT i = 0; i < Instr.getSrcSize(); ++i) {
          auto *Opnd = Instr.getSrc(i);
          if (auto *Var = llvm::dyn_cast<Variable>(Opnd)) {
            if (Replacements.find(Var) != Replacements.end()) {
              Instr.replaceSource(i, Replacements[Var]);
            }
          }
        }

        // Check for repetitions
        auto SeenIter = Seen.find(&Instr);
        if (SeenIter != Seen.end()) { // seen before
          const Inst *Found = *SeenIter;
          Replacements[Instr.getDest()] = Found->getDest();
        } else { // new
          Seen.insert(&Instr);

          if (!AssumeSSA) {
            // Update dependencies
            for (SizeT i = 0; i < Instr.getSrcSize(); ++i) {
              auto *Opnd = Instr.getSrc(i);
              if (auto *Var = llvm::dyn_cast<Variable>(Opnd)) {
                Dependency[Var].push_back(&Instr);
              }
            }
          }
        }
      }
    }
  }
}

void Cfg::loopInvariantCodeMotion() {
  TimerMarker T(TimerStack::TT_loopInvariantCodeMotion, this);
  // Does not introduce new nodes as of now.
  for (auto &Loop : LoopInfo) {
    CfgNode *Header = Loop.Header;
    assert(Header);
    if (Header->getLoopNestDepth() < 1)
      return;
    CfgNode *PreHeader = Loop.PreHeader;
    if (PreHeader == nullptr || PreHeader->getInsts().size() == 0) {
      return; // try next loop
    }

    auto &Insts = PreHeader->getInsts();
    auto &LastInst = Insts.back();
    Insts.pop_back();

    for (auto *Inst : findLoopInvariantInstructions(Loop.Body)) {
      PreHeader->appendInst(Inst);
    }
    PreHeader->appendInst(&LastInst);
  }
}

CfgVector<Inst *>
Cfg::findLoopInvariantInstructions(const CfgUnorderedSet<SizeT> &Body) {
  CfgUnorderedSet<Inst *> InvariantInsts;
  CfgUnorderedSet<Variable *> InvariantVars;
  for (auto *Var : getArgs()) {
    InvariantVars.insert(Var);
  }
  bool Changed = false;
  do {
    Changed = false;
    for (auto NodeIndex : Body) {
      auto *Node = Nodes[NodeIndex];
      CfgVector<std::reference_wrapper<Inst>> Insts(Node->getInsts().begin(),
                                                    Node->getInsts().end());

      for (auto &InstRef : Insts) {
        auto &Inst = InstRef.get();
        if (Inst.isDeleted() ||
            InvariantInsts.find(&Inst) != InvariantInsts.end())
          continue;
        switch (Inst.getKind()) {
        case Inst::InstKind::Alloca:
        case Inst::InstKind::Br:
        case Inst::InstKind::Ret:
        case Inst::InstKind::Phi:
        case Inst::InstKind::Call:
        case Inst::InstKind::Intrinsic:
        case Inst::InstKind::Load:
        case Inst::InstKind::Store:
        case Inst::InstKind::Switch:
          continue;
        default:
          break;
        }

        bool IsInvariant = true;
        for (SizeT i = 0; i < Inst.getSrcSize(); ++i) {
          if (auto *Var = llvm::dyn_cast<Variable>(Inst.getSrc(i))) {
            if (InvariantVars.find(Var) == InvariantVars.end()) {
              IsInvariant = false;
            }
          }
        }
        if (IsInvariant) {
          Changed = true;
          InvariantInsts.insert(&Inst);
          Node->getInsts().remove(Inst);
          if (Inst.getDest() != nullptr) {
            InvariantVars.insert(Inst.getDest());
          }
        }
      }
    }
  } while (Changed);

  CfgVector<Inst *> InstVector(InvariantInsts.begin(), InvariantInsts.end());
  std::sort(InstVector.begin(), InstVector.end(),
            [](Inst *A, Inst *B) { return A->getNumber() < B->getNumber(); });
  return InstVector;
}

void Cfg::shortCircuitJumps() {
  // Split Nodes whenever an early jump is possible.
  // __N :
  //   a = <something>
  //   Instruction 1 without side effect
  //   ... b = <something> ...
  //   Instruction N without side effect
  //   t1 = or a b
  //   br t1 __X __Y
  //
  // is transformed into:
  // __N :
  //   a = <something>
  //   br a __X __N_ext
  //
  // __N_ext :
  //   Instruction 1 without side effect
  //   ... b = <something> ...
  //   Instruction N without side effect
  //   br b __X __Y
  // (Similar logic for AND, jump to false instead of true target.)

  TimerMarker T(TimerStack::TT_shortCircuit, this);
  getVMetadata()->init(VMK_Uses);
  auto NodeStack = this->getNodes();
  CfgUnorderedMap<SizeT, CfgVector<CfgNode *>> Splits;
  while (!NodeStack.empty()) {
    auto *Node = NodeStack.back();
    NodeStack.pop_back();
    auto NewNode = Node->shortCircuit();
    if (NewNode) {
      NodeStack.push_back(NewNode);
      NodeStack.push_back(Node);
      Splits[Node->getIndex()].push_back(NewNode);
    }
  }

  // Insert nodes in the right place
  NodeList NewList;
  NewList.reserve(Nodes.size());
  CfgUnorderedSet<SizeT> Inserted;
  for (auto *Node : Nodes) {
    if (Inserted.find(Node->getIndex()) != Inserted.end())
      continue; // already inserted
    NodeList Stack{Node};
    while (!Stack.empty()) {
      auto *Current = Stack.back();
      Stack.pop_back();
      Inserted.insert(Current->getIndex());
      NewList.push_back(Current);
      for (auto *Next : Splits[Current->getIndex()]) {
        Stack.push_back(Next);
      }
    }
  }

  SizeT NodeIndex = 0;
  for (auto *Node : NewList) {
    Node->resetIndex(NodeIndex++);
  }
  Nodes = NewList;
}

void Cfg::floatConstantCSE() {
  // Load multiple uses of a floating point constant (between two call
  // instructions or block start/end) into a variable before its first use.
  //   t1 = b + 1.0
  //   t2 = c + 1.0
  // Gets transformed to:
  //   t0 = 1.0
  //   t0_1 = t0
  //   t1 = b + t0_1
  //   t2 = c + t0_1
  // Call instructions reset the procedure, but use the same variable, just in
  // case it got a register. We are assuming floating point registers are not
  // callee saved in general. Example, continuing from before:
  //   result = call <some function>
  //   t3 = d + 1.0
  // Gets transformed to:
  //   result = call <some function>
  //   t0_2 = t0
  //   t3 = d + t0_2
  // TODO(manasijm, stichnot): Figure out how to 'link' t0 to the stack slot of
  // 1.0. When t0 does not get a register, introducing an extra assignment
  // statement does not make sense. The relevant portion is marked below.

  TimerMarker _(TimerStack::TT_floatConstantCse, this);
  for (CfgNode *Node : getNodes()) {

    CfgUnorderedMap<Constant *, Variable *> ConstCache;
    auto Current = Node->getInsts().begin();
    auto End = Node->getInsts().end();
    while (Current != End) {
      CfgUnorderedMap<Constant *, CfgVector<InstList::iterator>> FloatUses;
      if (llvm::isa<InstCall>(iteratorToInst(Current))) {
        ++Current;
        assert(Current != End);
        // Block should not end with a call
      }
      while (Current != End && !llvm::isa<InstCall>(iteratorToInst(Current))) {
        if (!Current->isDeleted()) {
          for (SizeT i = 0; i < Current->getSrcSize(); ++i) {
            if (auto *Const = llvm::dyn_cast<Constant>(Current->getSrc(i))) {
              if (Const->getType() == IceType_f32 ||
                  Const->getType() == IceType_f64) {
                FloatUses[Const].push_back(Current);
              }
            }
          }
        }
        ++Current;
      }
      for (auto &Pair : FloatUses) {
        static constexpr SizeT MinUseThreshold = 3;
        if (Pair.second.size() < MinUseThreshold)
          continue;
        // Only consider constants with at least `MinUseThreshold` uses
        auto &Insts = Node->getInsts();

        if (ConstCache.find(Pair.first) == ConstCache.end()) {
          // Saw a constant (which is used at least twice) for the first time
          auto *NewVar = makeVariable(Pair.first->getType());
          // NewVar->setLinkedTo(Pair.first);
          // TODO(manasijm): Plumbing for linking to an Operand.
          auto *Assign = InstAssign::create(Node->getCfg(), NewVar, Pair.first);
          Insts.insert(Pair.second[0], Assign);
          ConstCache[Pair.first] = NewVar;
        }

        auto *NewVar = makeVariable(Pair.first->getType());
        NewVar->setLinkedTo(ConstCache[Pair.first]);
        auto *Assign =
            InstAssign::create(Node->getCfg(), NewVar, ConstCache[Pair.first]);

        Insts.insert(Pair.second[0], Assign);
        for (auto InstUse : Pair.second) {
          for (SizeT i = 0; i < InstUse->getSrcSize(); ++i) {
            if (auto *Const = llvm::dyn_cast<Constant>(InstUse->getSrc(i))) {
              if (Const == Pair.first) {
                InstUse->replaceSource(i, NewVar);
              }
            }
          }
        }
      }
    }
  }
}

void Cfg::doArgLowering() {
  TimerMarker T(TimerStack::TT_doArgLowering, this);
  getTarget()->lowerArguments();
}

void Cfg::sortAndCombineAllocas(CfgVector<InstAlloca *> &Allocas,
                                uint32_t CombinedAlignment, InstList &Insts,
                                AllocaBaseVariableType BaseVariableType) {
  if (Allocas.empty())
    return;
  // Sort by decreasing alignment.
  std::sort(Allocas.begin(), Allocas.end(), [](InstAlloca *A1, InstAlloca *A2) {
    uint32_t Align1 = A1->getAlignInBytes();
    uint32_t Align2 = A2->getAlignInBytes();
    if (Align1 == Align2)
      return A1->getNumber() < A2->getNumber();
    else
      return Align1 > Align2;
  });
  // Process the allocas in order of decreasing stack alignment.  This allows
  // us to pack less-aligned pieces after more-aligned ones, resulting in less
  // stack growth.  It also allows there to be at most one stack alignment "and"
  // instruction for a whole list of allocas.
  uint32_t CurrentOffset = 0;
  CfgVector<int32_t> Offsets;
  for (Inst *Instr : Allocas) {
    auto *Alloca = llvm::cast<InstAlloca>(Instr);
    // Adjust the size of the allocation up to the next multiple of the
    // object's alignment.
    uint32_t Alignment = std::max(Alloca->getAlignInBytes(), 1u);
    auto *ConstSize =
        llvm::dyn_cast<ConstantInteger32>(Alloca->getSizeInBytes());
    const uint32_t Size =
        Utils::applyAlignment(ConstSize->getValue(), Alignment);

    // Ensure that the Size does not exceed StackSizeLimit which can lead to
    // undefined behavior below.
    if (Size > StackSizeLimit) {
      llvm::report_fatal_error("Local variable exceeds stack size limit");
      return; // NOTREACHED
    }

    if (BaseVariableType == BVT_FramePointer) {
      // Addressing is relative to the frame pointer.  Subtract the offset after
      // adding the size of the alloca, because it grows downwards from the
      // frame pointer.
      Offsets.push_back(Target->getFramePointerOffset(CurrentOffset, Size));
    } else {
      // Addressing is relative to the stack pointer or to a user pointer.  Add
      // the offset before adding the size of the object, because it grows
      // upwards from the stack pointer. In addition, if the addressing is
      // relative to the stack pointer, we need to add the pre-computed max out
      // args size bytes.
      const uint32_t OutArgsOffsetOrZero =
          (BaseVariableType == BVT_StackPointer)
              ? getTarget()->maxOutArgsSizeBytes()
              : 0;
      Offsets.push_back(CurrentOffset + OutArgsOffsetOrZero);
    }

    // Ensure that the addition below does not overflow or exceed
    // StackSizeLimit as this leads to undefined behavior.
    if (CurrentOffset + Size > StackSizeLimit) {
      llvm::report_fatal_error("Local variable exceeds stack size limit");
      return; // NOTREACHED
    }

    // Update the running offset of the fused alloca region.
    CurrentOffset += Size;
  }
  // Round the offset up to the alignment granularity to use as the size.
  uint32_t TotalSize = Utils::applyAlignment(CurrentOffset, CombinedAlignment);
  // Ensure every alloca was assigned an offset.
  assert(Allocas.size() == Offsets.size());

  switch (BaseVariableType) {
  case BVT_UserPointer: {
    Variable *BaseVariable = makeVariable(IceType_i32);
    for (SizeT i = 0; i < Allocas.size(); ++i) {
      auto *Alloca = llvm::cast<InstAlloca>(Allocas[i]);
      // Emit a new addition operation to replace the alloca.
      Operand *AllocaOffset = Ctx->getConstantInt32(Offsets[i]);
      InstArithmetic *Add =
          InstArithmetic::create(this, InstArithmetic::Add, Alloca->getDest(),
                                 BaseVariable, AllocaOffset);
      Insts.push_front(Add);
      Alloca->setDeleted();
    }
    Operand *AllocaSize = Ctx->getConstantInt32(TotalSize);
    InstAlloca *CombinedAlloca =
        InstAlloca::create(this, BaseVariable, AllocaSize, CombinedAlignment);
    CombinedAlloca->setKnownFrameOffset();
    Insts.push_front(CombinedAlloca);
  } break;
  case BVT_StackPointer:
  case BVT_FramePointer: {
    for (SizeT i = 0; i < Allocas.size(); ++i) {
      auto *Alloca = llvm::cast<InstAlloca>(Allocas[i]);
      // Emit a fake definition of the rematerializable variable.
      Variable *Dest = Alloca->getDest();
      auto *Def = InstFakeDef::create(this, Dest);
      if (BaseVariableType == BVT_StackPointer)
        Dest->setRematerializable(getTarget()->getStackReg(), Offsets[i]);
      else
        Dest->setRematerializable(getTarget()->getFrameReg(), Offsets[i]);
      Insts.push_front(Def);
      Alloca->setDeleted();
    }
    // Allocate the fixed area in the function prolog.
    getTarget()->reserveFixedAllocaArea(TotalSize, CombinedAlignment);
  } break;
  }
}

void Cfg::processAllocas(bool SortAndCombine) {
  TimerMarker _(TimerStack::TT_alloca, this);
  const uint32_t StackAlignment = getTarget()->getStackAlignment();
  CfgNode *EntryNode = getEntryNode();
  assert(EntryNode);
  // LLVM enforces power of 2 alignment.
  assert(llvm::isPowerOf2_32(StackAlignment));
  // If the ABI's stack alignment is smaller than the vector size (16 bytes),
  // conservatively use a frame pointer to allow for explicit alignment of the
  // stack pointer. This needs to happen before register allocation so the frame
  // pointer can be reserved.
  if (getTarget()->needsStackPointerAlignment()) {
    getTarget()->setHasFramePointer();
  }
  // Determine if there are large alignment allocations in the entry block or
  // dynamic allocations (variable size in the entry block).
  bool HasLargeAlignment = false;
  bool HasDynamicAllocation = false;
  for (Inst &Instr : EntryNode->getInsts()) {
    if (Instr.isDeleted())
      continue;
    if (auto *Alloca = llvm::dyn_cast<InstAlloca>(&Instr)) {
      uint32_t AlignmentParam = Alloca->getAlignInBytes();
      if (AlignmentParam > StackAlignment)
        HasLargeAlignment = true;
      if (llvm::isa<Constant>(Alloca->getSizeInBytes()))
        Alloca->setKnownFrameOffset();
      else {
        HasDynamicAllocation = true;
        // If Allocas are not sorted, the first dynamic allocation causes
        // later Allocas to be at unknown offsets relative to the stack/frame.
        if (!SortAndCombine)
          break;
      }
    }
  }
  // Don't do the heavyweight sorting and layout for low optimization levels.
  if (!SortAndCombine)
    return;
  // Any alloca outside the entry block is a dynamic allocation.
  for (CfgNode *Node : Nodes) {
    if (Node == EntryNode)
      continue;
    for (Inst &Instr : Node->getInsts()) {
      if (Instr.isDeleted())
        continue;
      if (llvm::isa<InstAlloca>(&Instr)) {
        // Allocations outside the entry block require a frame pointer.
        HasDynamicAllocation = true;
        break;
      }
    }
    if (HasDynamicAllocation)
      break;
  }
  // Mark the target as requiring a frame pointer.
  if (HasLargeAlignment || HasDynamicAllocation)
    getTarget()->setHasFramePointer();
  // Collect the Allocas into the two vectors.
  // Allocas in the entry block that have constant size and alignment less
  // than or equal to the function's stack alignment.
  CfgVector<InstAlloca *> FixedAllocas;
  // Allocas in the entry block that have constant size and alignment greater
  // than the function's stack alignment.
  CfgVector<InstAlloca *> AlignedAllocas;
  // Maximum alignment used by any alloca.
  uint32_t MaxAlignment = StackAlignment;
  for (Inst &Instr : EntryNode->getInsts()) {
    if (Instr.isDeleted())
      continue;
    if (auto *Alloca = llvm::dyn_cast<InstAlloca>(&Instr)) {
      if (!llvm::isa<Constant>(Alloca->getSizeInBytes()))
        continue;
      uint32_t AlignmentParam = Alloca->getAlignInBytes();
      // For default align=0, set it to the real value 1, to avoid any
      // bit-manipulation problems below.
      AlignmentParam = std::max(AlignmentParam, 1u);
      assert(llvm::isPowerOf2_32(AlignmentParam));
      if (HasDynamicAllocation && AlignmentParam > StackAlignment) {
        // If we have both dynamic allocations and large stack alignments,
        // high-alignment allocations are pulled out with their own base.
        AlignedAllocas.push_back(Alloca);
      } else {
        FixedAllocas.push_back(Alloca);
      }
      MaxAlignment = std::max(AlignmentParam, MaxAlignment);
    }
  }
  // Add instructions to the head of the entry block in reverse order.
  InstList &Insts = getEntryNode()->getInsts();
  if (HasDynamicAllocation && HasLargeAlignment) {
    // We are using a frame pointer, but fixed large-alignment alloca addresses
    // do not have a known offset from either the stack or frame pointer.
    // They grow up from a user pointer from an alloca.
    sortAndCombineAllocas(AlignedAllocas, MaxAlignment, Insts, BVT_UserPointer);
    // Fixed size allocas are addressed relative to the frame pointer.
    sortAndCombineAllocas(FixedAllocas, StackAlignment, Insts,
                          BVT_FramePointer);
  } else {
    // Otherwise, fixed size allocas are addressed relative to the stack unless
    // there are dynamic allocas.
    const AllocaBaseVariableType BasePointerType =
        (HasDynamicAllocation ? BVT_FramePointer : BVT_StackPointer);
    sortAndCombineAllocas(FixedAllocas, MaxAlignment, Insts, BasePointerType);
  }
  if (!FixedAllocas.empty() || !AlignedAllocas.empty())
    // No use calling findRematerializable() unless there is some
    // rematerializable alloca instruction to seed it.
    findRematerializable();
}

namespace {

// Helpers for findRematerializable().  For each of them, if a suitable
// rematerialization is found, the instruction's Dest variable is set to be
// rematerializable and it returns true, otherwise it returns false.

bool rematerializeArithmetic(const Inst *Instr) {
  // Check that it's an Arithmetic instruction with an Add operation.
  auto *Arith = llvm::dyn_cast<InstArithmetic>(Instr);
  if (Arith == nullptr || Arith->getOp() != InstArithmetic::Add)
    return false;
  // Check that Src(0) is rematerializable.
  auto *Src0Var = llvm::dyn_cast<Variable>(Arith->getSrc(0));
  if (Src0Var == nullptr || !Src0Var->isRematerializable())
    return false;
  // Check that Src(1) is an immediate.
  auto *Src1Imm = llvm::dyn_cast<ConstantInteger32>(Arith->getSrc(1));
  if (Src1Imm == nullptr)
    return false;
  Arith->getDest()->setRematerializable(
      Src0Var->getRegNum(), Src0Var->getStackOffset() + Src1Imm->getValue());
  return true;
}

bool rematerializeAssign(const Inst *Instr) {
  // An InstAssign only originates from an inttoptr or ptrtoint instruction,
  // which never occurs in a MINIMAL build.
  if (BuildDefs::minimal())
    return false;
  // Check that it's an Assign instruction.
  if (!llvm::isa<InstAssign>(Instr))
    return false;
  // Check that Src(0) is rematerializable.
  auto *Src0Var = llvm::dyn_cast<Variable>(Instr->getSrc(0));
  if (Src0Var == nullptr || !Src0Var->isRematerializable())
    return false;
  Instr->getDest()->setRematerializable(Src0Var->getRegNum(),
                                        Src0Var->getStackOffset());
  return true;
}

bool rematerializeCast(const Inst *Instr) {
  // An pointer-type bitcast never occurs in a MINIMAL build.
  if (BuildDefs::minimal())
    return false;
  // Check that it's a Cast instruction with a Bitcast operation.
  auto *Cast = llvm::dyn_cast<InstCast>(Instr);
  if (Cast == nullptr || Cast->getCastKind() != InstCast::Bitcast)
    return false;
  // Check that Src(0) is rematerializable.
  auto *Src0Var = llvm::dyn_cast<Variable>(Cast->getSrc(0));
  if (Src0Var == nullptr || !Src0Var->isRematerializable())
    return false;
  // Check that Dest and Src(0) have the same type.
  Variable *Dest = Cast->getDest();
  if (Dest->getType() != Src0Var->getType())
    return false;
  Dest->setRematerializable(Src0Var->getRegNum(), Src0Var->getStackOffset());
  return true;
}

} // end of anonymous namespace

/// Scan the function to find additional rematerializable variables.  This is
/// possible when the source operand of an InstAssignment is a rematerializable
/// variable, or the same for a pointer-type InstCast::Bitcast, or when an
/// InstArithmetic is an add of a rematerializable variable and an immediate.
/// Note that InstAssignment instructions and pointer-type InstCast::Bitcast
/// instructions generally only come about from the IceConverter's treatment of
/// inttoptr, ptrtoint, and bitcast instructions.  TODO(stichnot): Consider
/// other possibilities, however unlikely, such as InstArithmetic::Sub, or
/// commutativity.
void Cfg::findRematerializable() {
  // Scan the instructions in order, and repeat until no new opportunities are
  // found.  It may take more than one iteration because a variable's defining
  // block may happen to come after a block where it is used, depending on the
  // CfgNode linearization order.
  bool FoundNewAssignment;
  do {
    FoundNewAssignment = false;
    for (CfgNode *Node : getNodes()) {
      // No need to process Phi instructions.
      for (Inst &Instr : Node->getInsts()) {
        if (Instr.isDeleted())
          continue;
        Variable *Dest = Instr.getDest();
        if (Dest == nullptr || Dest->isRematerializable())
          continue;
        if (rematerializeArithmetic(&Instr) || rematerializeAssign(&Instr) ||
            rematerializeCast(&Instr)) {
          FoundNewAssignment = true;
        }
      }
    }
  } while (FoundNewAssignment);
}

void Cfg::doAddressOpt() {
  TimerMarker T(TimerStack::TT_doAddressOpt, this);
  for (CfgNode *Node : Nodes)
    Node->doAddressOpt();
}

namespace {
// ShuffleVectorUtils implements helper functions for rematerializing
// shufflevector instructions from a sequence of extractelement/insertelement
// instructions. It looks for the following pattern:
//
// %t0 = extractelement A, %n0
// %t1 = extractelement B, %n1
// %t2 = extractelement C, %n2
// ...
// %tN = extractelement N, %nN
// %d0 = insertelement undef, %t0, 0
// %d1 = insertelement %d0, %t1, 1
// %d2 = insertelement %d1, %t2, 2
// ...
// %dest = insertelement %d_N-1, %tN, N
//
// where N is num_element(typeof(%dest)), and A, B, C, ... N are at most two
// distinct variables.
namespace ShuffleVectorUtils {
// findAllInserts is used when searching for all the insertelements that are
// used in a shufflevector operation. This function works recursively, when
// invoked with I = i, the function assumes Insts[i] is the last found
// insertelement in the chain. The next insertelement insertruction is saved in
// Insts[i+1].
bool findAllInserts(Cfg *Func, GlobalContext *Ctx, VariablesMetadata *VM,
                    CfgVector<const Inst *> *Insts, SizeT I = 0) {
  const bool Verbose = BuildDefs::dump() && Func->isVerbose(IceV_ShufMat);

  if (I > Insts->size()) {
    if (Verbose) {
      Ctx->getStrDump() << "\tToo many inserts.\n";
    }
    return false;
  }

  const auto *LastInsert = Insts->at(I);
  assert(llvm::isa<InstInsertElement>(LastInsert));

  if (I == Insts->size() - 1) {
    // Matching against undef is not really needed because the value in Src(0)
    // will be totally overwritten. We still enforce it anyways because the
    // PNaCl toolchain generates the bitcode with it.
    if (!llvm::isa<ConstantUndef>(LastInsert->getSrc(0))) {
      if (Verbose) {
        Ctx->getStrDump() << "\tSrc0 is not undef: " << I << " "
                          << Insts->size();
        LastInsert->dump(Func);
        Ctx->getStrDump() << "\n";
      }
      return false;
    }

    // The following loop ensures that the insertelements are sorted. In theory,
    // we could relax this restriction and allow any order. As long as each
    // index appears exactly once, this chain is still a candidate for becoming
    // a shufflevector. The Insts vector is traversed backwards because the
    // instructions are "enqueued" in reverse order.
    int32_t ExpectedElement = 0;
    for (const auto *I : reverse_range(*Insts)) {
      if (llvm::cast<ConstantInteger32>(I->getSrc(2))->getValue() !=
          ExpectedElement) {
        return false;
      }
      ++ExpectedElement;
    }
    return true;
  }

  const auto *Src0V = llvm::cast<Variable>(LastInsert->getSrc(0));
  const auto *Def = VM->getSingleDefinition(Src0V);

  // Only optimize if the first operand in
  //
  //   Dest = insertelement A, B, 10
  //
  // is singly-def'ed.
  if (Def == nullptr) {
    if (Verbose) {
      Ctx->getStrDump() << "\tmulti-def: ";
      (*Insts)[I]->dump(Func);
      Ctx->getStrDump() << "\n";
    }
    return false;
  }

  // We also require the (single) definition to come from an insertelement
  // instruction.
  if (!llvm::isa<InstInsertElement>(Def)) {
    if (Verbose) {
      Ctx->getStrDump() << "\tnot insert element: ";
      Def->dump(Func);
      Ctx->getStrDump() << "\n";
    }
    return false;
  }

  // Everything seems fine, so we save Def in Insts, and delegate the decision
  // to findAllInserts.
  (*Insts)[I + 1] = Def;

  return findAllInserts(Func, Ctx, VM, Insts, I + 1);
}

// insertsLastElement returns true if Insert is inserting an element in the last
// position of a vector.
bool insertsLastElement(const Inst &Insert) {
  const Type DestTy = Insert.getDest()->getType();
  assert(isVectorType(DestTy));
  const SizeT Elem =
      llvm::cast<ConstantInteger32>(Insert.getSrc(2))->getValue();
  return Elem == typeNumElements(DestTy) - 1;
}

// findAllExtracts goes over all the insertelement instructions that are
// candidates to be replaced by a shufflevector, and searches for all the
// definitions of the elements being inserted. If all of the elements are the
// result of an extractelement instruction, and all of the extractelements
// operate on at most two different sources, than the instructions can be
// replaced by a shufflevector.
bool findAllExtracts(Cfg *Func, GlobalContext *Ctx, VariablesMetadata *VM,
                     const CfgVector<const Inst *> &Insts, Variable **Src0,
                     Variable **Src1, CfgVector<const Inst *> *Extracts) {
  const bool Verbose = BuildDefs::dump() && Func->isVerbose(IceV_ShufMat);

  *Src0 = nullptr;
  *Src1 = nullptr;
  assert(Insts.size() > 0);
  for (SizeT I = 0; I < Insts.size(); ++I) {
    const auto *Insert = Insts.at(I);
    const auto *Src1V = llvm::dyn_cast<Variable>(Insert->getSrc(1));
    if (Src1V == nullptr) {
      if (Verbose) {
        Ctx->getStrDump() << "src(1) is not a variable: ";
        Insert->dump(Func);
        Ctx->getStrDump() << "\n";
      }
      return false;
    }

    const auto *Def = VM->getSingleDefinition(Src1V);
    if (Def == nullptr) {
      if (Verbose) {
        Ctx->getStrDump() << "multi-def src(1): ";
        Insert->dump(Func);
        Ctx->getStrDump() << "\n";
      }
      return false;
    }

    if (!llvm::isa<InstExtractElement>(Def)) {
      if (Verbose) {
        Ctx->getStrDump() << "not extractelement: ";
        Def->dump(Func);
        Ctx->getStrDump() << "\n";
      }
      return false;
    }

    auto *Src = llvm::cast<Variable>(Def->getSrc(0));
    if (*Src0 == nullptr) {
      // No sources yet. Save Src to Src0.
      *Src0 = Src;
    } else if (*Src1 == nullptr) {
      // We already have a source, so we might save Src in Src1 -- but only if
      // Src0 is not Src.
      if (*Src0 != Src) {
        *Src1 = Src;
      }
    } else if (Src != *Src0 && Src != *Src1) {
      // More than two sources, so we can't rematerialize the shufflevector
      // instruction.
      if (Verbose) {
        Ctx->getStrDump() << "Can't shuffle more than two sources.\n";
      }
      return false;
    }

    (*Extracts)[I] = Def;
  }

  // We should have seen at least one source operand.
  assert(*Src0 != nullptr);

  // If a second source was not seen, then we just make Src1 = Src0 to simplify
  // things down stream. This should not matter, as all of the indexes in the
  // shufflevector instruction will point to Src0.
  if (*Src1 == nullptr) {
    *Src1 = *Src0;
  }

  return true;
}

} // end of namespace ShuffleVectorUtils
} // end of anonymous namespace

void Cfg::materializeVectorShuffles() {
  const bool Verbose = BuildDefs::dump() && isVerbose(IceV_ShufMat);

  std::unique_ptr<OstreamLocker> L;
  if (Verbose) {
    L.reset(new OstreamLocker(getContext()));
    getContext()->getStrDump() << "\nShuffle materialization:\n";
  }

  // MaxVectorElements is the maximum number of elements in the vector types
  // handled by Subzero. We use it to create the Inserts and Extracts vectors
  // with the appropriate size, thus avoiding resize() calls.
  const SizeT MaxVectorElements = typeNumElements(IceType_v16i8);
  CfgVector<const Inst *> Inserts(MaxVectorElements);
  CfgVector<const Inst *> Extracts(MaxVectorElements);

  TimerMarker T(TimerStack::TT_materializeVectorShuffles, this);
  for (CfgNode *Node : Nodes) {
    for (auto &Instr : Node->getInsts()) {
      if (!llvm::isa<InstInsertElement>(Instr)) {
        continue;
      }
      if (!ShuffleVectorUtils::insertsLastElement(Instr)) {
        // To avoid wasting time, we only start the pattern match at the last
        // insertelement instruction -- and go backwards from there.
        continue;
      }
      if (Verbose) {
        getContext()->getStrDump() << "\tCandidate: ";
        Instr.dump(this);
        getContext()->getStrDump() << "\n";
      }
      Inserts.resize(typeNumElements(Instr.getDest()->getType()));
      Inserts[0] = &Instr;
      if (!ShuffleVectorUtils::findAllInserts(this, getContext(),
                                              VMetadata.get(), &Inserts)) {
        // If we fail to find a sequence of insertelements, we stop the
        // optimization.
        if (Verbose) {
          getContext()->getStrDump() << "\tFalse alarm.\n";
        }
        continue;
      }
      if (Verbose) {
        getContext()->getStrDump() << "\tFound the following insertelement: \n";
        for (auto *I : reverse_range(Inserts)) {
          getContext()->getStrDump() << "\t\t";
          I->dump(this);
          getContext()->getStrDump() << "\n";
        }
      }
      Extracts.resize(Inserts.size());
      Variable *Src0;
      Variable *Src1;
      if (!ShuffleVectorUtils::findAllExtracts(this, getContext(),
                                               VMetadata.get(), Inserts, &Src0,
                                               &Src1, &Extracts)) {
        // If we fail to match the definitions of the insertelements' sources
        // with extractelement instructions -- or if those instructions operate
        // on more than two different variables -- we stop the optimization.
        if (Verbose) {
          getContext()->getStrDump() << "\tFailed to match extractelements.\n";
        }
        continue;
      }
      if (Verbose) {
        getContext()->getStrDump()
            << "\tFound the following insert/extract element pairs: \n";
        for (SizeT I = 0; I < Inserts.size(); ++I) {
          const SizeT Pos = Inserts.size() - I - 1;
          getContext()->getStrDump() << "\t\tInsert : ";
          Inserts[Pos]->dump(this);
          getContext()->getStrDump() << "\n\t\tExtract: ";
          Extracts[Pos]->dump(this);
          getContext()->getStrDump() << "\n";
        }
      }

      assert(Src0 != nullptr);
      assert(Src1 != nullptr);

      auto *ShuffleVector =
          InstShuffleVector::create(this, Instr.getDest(), Src0, Src1);
      assert(ShuffleVector->getSrc(0) == Src0);
      assert(ShuffleVector->getSrc(1) == Src1);
      for (SizeT I = 0; I < Extracts.size(); ++I) {
        const SizeT Pos = Extracts.size() - I - 1;
        auto *Index = llvm::cast<ConstantInteger32>(Extracts[Pos]->getSrc(1));
        if (Src0 == Extracts[Pos]->getSrc(0)) {
          ShuffleVector->addIndex(Index);
        } else {
          ShuffleVector->addIndex(llvm::cast<ConstantInteger32>(
              Ctx->getConstantInt32(Index->getValue() + Extracts.size())));
        }
      }

      if (Verbose) {
        getContext()->getStrDump() << "Created: ";
        ShuffleVector->dump(this);
        getContext()->getStrDump() << "\n";
      }

      Instr.setDeleted();
      auto &LoweringContext = getTarget()->getContext();
      LoweringContext.setInsertPoint(instToIterator(&Instr));
      LoweringContext.insert(ShuffleVector);
    }
  }
}

void Cfg::genCode() {
  TimerMarker T(TimerStack::TT_genCode, this);
  for (CfgNode *Node : Nodes)
    Node->genCode();
}

// Compute the stack frame layout.
void Cfg::genFrame() {
  TimerMarker T(TimerStack::TT_genFrame, this);
  getTarget()->addProlog(Entry);
  for (CfgNode *Node : Nodes)
    if (Node->getHasReturn())
      getTarget()->addEpilog(Node);
}

void Cfg::generateLoopInfo() {
  TimerMarker T(TimerStack::TT_computeLoopNestDepth, this);
  LoopInfo = ComputeLoopInfo(this);
}

// This is a lightweight version of live-range-end calculation. Marks the last
// use of only those variables whose definition and uses are completely with a
// single block. It is a quick single pass and doesn't need to iterate until
// convergence.
void Cfg::livenessLightweight() {
  TimerMarker T(TimerStack::TT_livenessLightweight, this);
  getVMetadata()->init(VMK_Uses);
  for (CfgNode *Node : Nodes)
    Node->livenessLightweight();
}

void Cfg::liveness(LivenessMode Mode) {
  TimerMarker T(TimerStack::TT_liveness, this);
  // Destroying the previous (if any) Liveness information clears the Liveness
  // allocator TLS pointer.
  Live = nullptr;
  Live = Liveness::create(this, Mode);

  getVMetadata()->init(VMK_Uses);
  Live->init();

  // Initialize with all nodes needing to be processed.
  BitVector NeedToProcess(Nodes.size(), true);
  while (NeedToProcess.any()) {
    // Iterate in reverse topological order to speed up convergence.
    for (CfgNode *Node : reverse_range(Nodes)) {
      if (NeedToProcess[Node->getIndex()]) {
        NeedToProcess[Node->getIndex()] = false;
        bool Changed = Node->liveness(getLiveness());
        if (Changed) {
          // If the beginning-of-block liveness changed since the last
          // iteration, mark all in-edges as needing to be processed.
          for (CfgNode *Pred : Node->getInEdges())
            NeedToProcess[Pred->getIndex()] = true;
        }
      }
    }
  }
  if (Mode == Liveness_Intervals) {
    // Reset each variable's live range.
    for (Variable *Var : Variables)
      Var->resetLiveRange();
  }
  // Make a final pass over each node to delete dead instructions, collect the
  // first and last instruction numbers, and add live range segments for that
  // node.
  for (CfgNode *Node : Nodes) {
    InstNumberT FirstInstNum = Inst::NumberSentinel;
    InstNumberT LastInstNum = Inst::NumberSentinel;
    for (Inst &I : Node->getPhis()) {
      I.deleteIfDead();
      if (Mode == Liveness_Intervals && !I.isDeleted()) {
        if (FirstInstNum == Inst::NumberSentinel)
          FirstInstNum = I.getNumber();
        assert(I.getNumber() > LastInstNum);
        LastInstNum = I.getNumber();
      }
    }
    for (Inst &I : Node->getInsts()) {
      I.deleteIfDead();
      if (Mode == Liveness_Intervals && !I.isDeleted()) {
        if (FirstInstNum == Inst::NumberSentinel)
          FirstInstNum = I.getNumber();
        assert(I.getNumber() > LastInstNum);
        LastInstNum = I.getNumber();
      }
    }
    if (Mode == Liveness_Intervals) {
      // Special treatment for live in-args. Their liveness needs to extend
      // beyond the beginning of the function, otherwise an arg whose only use
      // is in the first instruction will end up having the trivial live range
      // [2,2) and will *not* interfere with other arguments. So if the first
      // instruction of the method is "r=arg1+arg2", both args may be assigned
      // the same register. This is accomplished by extending the entry block's
      // instruction range from [2,n) to [1,n) which will transform the
      // problematic [2,2) live ranges into [1,2).  This extension works because
      // the entry node is guaranteed to have the lowest instruction numbers.
      if (Node == getEntryNode()) {
        FirstInstNum = Inst::NumberExtended;
        // Just in case the entry node somehow contains no instructions...
        if (LastInstNum == Inst::NumberSentinel)
          LastInstNum = FirstInstNum;
      }
      // If this node somehow contains no instructions, don't bother trying to
      // add liveness intervals for it, because variables that are live-in and
      // live-out will have a bogus interval added.
      if (FirstInstNum != Inst::NumberSentinel)
        Node->livenessAddIntervals(getLiveness(), FirstInstNum, LastInstNum);
    }
  }
}

// Traverse every Variable of every Inst and verify that it appears within the
// Variable's computed live range.
bool Cfg::validateLiveness() const {
  TimerMarker T(TimerStack::TT_validateLiveness, this);
  bool Valid = true;
  OstreamLocker L(Ctx);
  Ostream &Str = Ctx->getStrDump();
  for (CfgNode *Node : Nodes) {
    Inst *FirstInst = nullptr;
    for (Inst &Instr : Node->getInsts()) {
      if (Instr.isDeleted())
        continue;
      if (FirstInst == nullptr)
        FirstInst = &Instr;
      InstNumberT InstNumber = Instr.getNumber();
      if (Variable *Dest = Instr.getDest()) {
        if (!Dest->getIgnoreLiveness()) {
          bool Invalid = false;
          constexpr bool IsDest = true;
          if (!Dest->getLiveRange().containsValue(InstNumber, IsDest))
            Invalid = true;
          // Check that this instruction actually *begins* Dest's live range,
          // by checking that Dest is not live in the previous instruction. As
          // a special exception, we don't check this for the first instruction
          // of the block, because a Phi temporary may be live at the end of
          // the previous block, and if it is also assigned in the first
          // instruction of this block, the adjacent live ranges get merged.
          if (&Instr != FirstInst && !Instr.isDestRedefined() &&
              Dest->getLiveRange().containsValue(InstNumber - 1, IsDest))
            Invalid = true;
          if (Invalid) {
            Valid = false;
            Str << "Liveness error: inst " << Instr.getNumber() << " dest ";
            Dest->dump(this);
            Str << " live range " << Dest->getLiveRange() << "\n";
          }
        }
      }
      FOREACH_VAR_IN_INST(Var, Instr) {
        static constexpr bool IsDest = false;
        if (!Var->getIgnoreLiveness() &&
            !Var->getLiveRange().containsValue(InstNumber, IsDest)) {
          Valid = false;
          Str << "Liveness error: inst " << Instr.getNumber() << " var ";
          Var->dump(this);
          Str << " live range " << Var->getLiveRange() << "\n";
        }
      }
    }
  }
  return Valid;
}

void Cfg::contractEmptyNodes() {
  // If we're decorating the asm output with register liveness info, this
  // information may become corrupted or incorrect after contracting nodes that
  // contain only redundant assignments. As such, we disable this pass when
  // DecorateAsm is specified. This may make the resulting code look more
  // branchy, but it should have no effect on the register assignments.
  if (getFlags().getDecorateAsm())
    return;
  for (CfgNode *Node : Nodes) {
    Node->contractIfEmpty();
  }
}

void Cfg::doBranchOpt() {
  TimerMarker T(TimerStack::TT_doBranchOpt, this);
  for (auto I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    auto NextNode = I + 1;
    (*I)->doBranchOpt(NextNode == E ? nullptr : *NextNode);
  }
}

// ======================== Dump routines ======================== //

// emitTextHeader() is not target-specific (apart from what is abstracted by
// the Assembler), so it is defined here rather than in the target lowering
// class.
void Cfg::emitTextHeader(GlobalString Name, GlobalContext *Ctx,
                         const Assembler *Asm) {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Ctx->getStrEmit();
  Str << "\t.text\n";
  if (getFlags().getFunctionSections())
    Str << "\t.section\t.text." << Name << ",\"ax\",%progbits\n";
  if (!Asm->getInternal() || getFlags().getDisableInternal()) {
    Str << "\t.globl\t" << Name << "\n";
    Str << "\t.type\t" << Name << ",%function\n";
  }
  Str << "\t" << Asm->getAlignDirective() << " "
      << Asm->getBundleAlignLog2Bytes() << ",0x";
  for (uint8_t I : Asm->getNonExecBundlePadding())
    Str.write_hex(I);
  Str << "\n";
  Str << Name << ":\n";
}

void Cfg::emitJumpTables() {
  switch (getFlags().getOutFileType()) {
  case FT_Elf:
  case FT_Iasm: {
    // The emission needs to be delayed until the after the text section so
    // save the offsets in the global context.
    for (const InstJumpTable *JumpTable : JumpTables) {
      Ctx->addJumpTableData(JumpTable->toJumpTableData(getAssembler()));
    }
  } break;
  case FT_Asm: {
    // Emit the assembly directly so we don't need to hang on to all the names
    for (const InstJumpTable *JumpTable : JumpTables)
      getTarget()->emitJumpTable(this, JumpTable);
  } break;
  }
}

void Cfg::emit() {
  if (!BuildDefs::dump())
    return;
  TimerMarker T(TimerStack::TT_emitAsm, this);
  if (getFlags().getDecorateAsm()) {
    renumberInstructions();
    getVMetadata()->init(VMK_Uses);
    liveness(Liveness_Basic);
    dump("After recomputing liveness for -decorate-asm");
  }
  OstreamLocker L(Ctx);
  Ostream &Str = Ctx->getStrEmit();
  const Assembler *Asm = getAssembler<>();

  emitTextHeader(FunctionName, Ctx, Asm);
  if (getFlags().getDecorateAsm()) {
    for (Variable *Var : getVariables()) {
      if (Var->hasKnownStackOffset() && !Var->isRematerializable()) {
        Str << "\t" << Var->getSymbolicStackOffset() << " = "
            << Var->getStackOffset() << "\n";
      }
    }
  }
  for (CfgNode *Node : Nodes) {
    Node->emit(this);
  }
  emitJumpTables();
  Str << "\n";
}

void Cfg::emitIAS() {
  TimerMarker T(TimerStack::TT_emitAsm, this);
  // The emitIAS() routines emit into the internal assembler buffer, so there's
  // no need to lock the streams.
  for (CfgNode *Node : Nodes) {
    Node->emitIAS(this);
  }
  emitJumpTables();
}

size_t Cfg::getTotalMemoryMB() const {
  constexpr size_t _1MB = 1024 * 1024;
  assert(Allocator != nullptr);
  assert(CfgAllocatorTraits::current() == Allocator.get());
  return Allocator->getTotalMemory() / _1MB;
}

size_t Cfg::getLivenessMemoryMB() const {
  constexpr size_t _1MB = 1024 * 1024;
  if (Live == nullptr) {
    return 0;
  }
  return Live->getAllocator()->getTotalMemory() / _1MB;
}

// Dumps the IR with an optional introductory message.
void Cfg::dump(const char *Message) {
  if (!BuildDefs::dump())
    return;
  if (!isVerbose())
    return;
  OstreamLocker L(Ctx);
  Ostream &Str = Ctx->getStrDump();
  if (Message[0])
    Str << "================ " << Message << " ================\n";
  if (isVerbose(IceV_Mem)) {
    Str << "Memory size = " << getTotalMemoryMB() << " MB\n";
  }
  setCurrentNode(getEntryNode());
  // Print function name+args
  if (isVerbose(IceV_Instructions)) {
    Str << "define ";
    if (getInternal() && !getFlags().getDisableInternal())
      Str << "internal ";
    Str << ReturnType << " @" << getFunctionName() << "(";
    for (SizeT i = 0; i < Args.size(); ++i) {
      if (i > 0)
        Str << ", ";
      Str << Args[i]->getType() << " ";
      Args[i]->dump(this);
    }
    // Append an extra copy of the function name here, in order to print its
    // size stats but not mess up lit tests.
    Str << ") { # " << getFunctionNameAndSize() << "\n";
  }
  resetCurrentNode();
  if (isVerbose(IceV_Liveness)) {
    // Print summary info about variables
    for (Variable *Var : Variables) {
      Str << "// multiblock=";
      if (getVMetadata()->isTracked(Var))
        Str << getVMetadata()->isMultiBlock(Var);
      else
        Str << "?";
      Str << " defs=";
      bool FirstPrint = true;
      if (VMetadata->getKind() != VMK_Uses) {
        if (const Inst *FirstDef = VMetadata->getFirstDefinition(Var)) {
          Str << FirstDef->getNumber();
          FirstPrint = false;
        }
      }
      if (VMetadata->getKind() == VMK_All) {
        for (const Inst *Instr : VMetadata->getLatterDefinitions(Var)) {
          if (!FirstPrint)
            Str << ",";
          Str << Instr->getNumber();
          FirstPrint = false;
        }
      }
      Str << " weight=" << Var->getWeight(this) << " ";
      Var->dump(this);
      Str << " LIVE=" << Var->getLiveRange() << "\n";
    }
  }
  // Print each basic block
  for (CfgNode *Node : Nodes)
    Node->dump(this);
  if (isVerbose(IceV_Instructions))
    Str << "}\n";
}

} // end of namespace Ice
