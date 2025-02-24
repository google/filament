//===- subzero/src/IceLiveness.h - Liveness analysis ------------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the Liveness and LivenessNode classes, which are used for
/// liveness analysis.
///
/// The node-specific information tracked for each Variable includes whether it
/// is live on entry, whether it is live on exit, the instruction number that
/// starts its live range, and the instruction number that ends its live range.
/// At the Cfg level, the actual live intervals are recorded.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICELIVENESS_H
#define SUBZERO_SRC_ICELIVENESS_H

#include "IceBitVector.h"
#include "IceCfgNode.h"
#include "IceDefs.h"
#include "IceTLS.h"
#include "IceTypes.h"

#include <memory>
#include <utility>

namespace Ice {

class Liveness {
  Liveness() = delete;
  Liveness(const Liveness &) = delete;
  Liveness &operator=(const Liveness &) = delete;

  class LivenessNode {
    LivenessNode &operator=(const LivenessNode &) = delete;

  public:
    LivenessNode() = default;
    LivenessNode(const LivenessNode &) = default;
    /// NumLocals is the number of Variables local to this block.
    SizeT NumLocals = 0;
    /// NumNonDeadPhis tracks the number of Phi instructions that
    /// Inst::liveness() identified as tentatively live. If NumNonDeadPhis
    /// changes from the last liveness pass, then liveness has not yet
    /// converged.
    SizeT NumNonDeadPhis = 0;
    // LiveToVarMap maps a liveness bitvector index to a Variable. This is
    // generally just for printing/dumping. The index should be less than
    // NumLocals + Liveness::NumGlobals.
    LivenessVector<Variable *> LiveToVarMap;
    // LiveIn and LiveOut track the in- and out-liveness of the global
    // variables. The size of each vector is LivenessNode::NumGlobals.
    LivenessBV LiveIn, LiveOut;
    // LiveBegin and LiveEnd track the instruction numbers of the start and end
    // of each variable's live range within this block. The index/key of each
    // element is less than NumLocals + Liveness::NumGlobals.
    LiveBeginEndMap LiveBegin, LiveEnd;
  };

public:
  void init();
  void initPhiEdgeSplits(NodeList::const_iterator FirstNode,
                         VarList::const_iterator FirstVar);
  Cfg *getFunc() const { return Func; }
  LivenessMode getMode() const { return Mode; }
  Variable *getVariable(SizeT LiveIndex, const CfgNode *Node) const;
  SizeT getLiveIndex(SizeT VarIndex) const {
    const SizeT LiveIndex = VarToLiveMap[VarIndex];
    assert(LiveIndex != InvalidLiveIndex);
    return LiveIndex;
  }
  SizeT getNumGlobalVars() const { return NumGlobals; }
  SizeT getNumVarsInNode(const CfgNode *Node) const {
    return NumGlobals + Nodes[Node->getIndex()].NumLocals;
  }
  SizeT &getNumNonDeadPhis(const CfgNode *Node) {
    return Nodes[Node->getIndex()].NumNonDeadPhis;
  }
  LivenessBV &getLiveIn(const CfgNode *Node) {
    SizeT Index = Node->getIndex();
    resize(Index);
    return Nodes[Index].LiveIn;
  }
  LivenessBV &getLiveOut(const CfgNode *Node) {
    SizeT Index = Node->getIndex();
    resize(Index);
    return Nodes[Index].LiveOut;
  }
  LivenessBV &getScratchBV() { return ScratchBV; }
  LiveBeginEndMap *getLiveBegin(const CfgNode *Node) {
    SizeT Index = Node->getIndex();
    resize(Index);
    return &Nodes[Index].LiveBegin;
  }
  LiveBeginEndMap *getLiveEnd(const CfgNode *Node) {
    SizeT Index = Node->getIndex();
    resize(Index);
    return &Nodes[Index].LiveEnd;
  }
  bool getRangeMask(SizeT Index) const { return RangeMask[Index]; }

  ArenaAllocator *getAllocator() const { return Alloc.get(); }

  static std::unique_ptr<Liveness> create(Cfg *Func, LivenessMode Mode) {
    return std::unique_ptr<Liveness>(new Liveness(Func, Mode));
  }

  static void TlsInit() { LivenessAllocatorTraits::init(); }

  std::string dumpStr() const {
    return "MaxLocals(" + std::to_string(MaxLocals) +
           "), "
           "NumGlobals(" +
           std::to_string(NumGlobals) + ")";
  }

private:
  Liveness(Cfg *Func, LivenessMode Mode)
      : Alloc(new ArenaAllocator()), AllocScope(this), Func(Func), Mode(Mode) {}

  void initInternal(NodeList::const_iterator FirstNode,
                    VarList::const_iterator FirstVar, bool IsFullInit);
  /// Resize Nodes so that Nodes[Index] is valid.
  void resize(SizeT Index) {
    if (Index >= Nodes.size()) {
      assert(false && "The Nodes array is not expected to be resized.");
      Nodes.resize(Index + 1);
    }
  }
  std::unique_ptr<ArenaAllocator> Alloc;
  LivenessAllocatorScope AllocScope; // Must be declared after Alloc.
  static constexpr SizeT InvalidLiveIndex = -1;
  Cfg *Func;
  LivenessMode Mode;
  /// Size of Nodes is Cfg::Nodes.size().
  LivenessVector<LivenessNode> Nodes;
  /// VarToLiveMap maps a Variable's Variable::Number to its live index within
  /// its basic block.
  LivenessVector<SizeT> VarToLiveMap;
  /// LiveToVarMap is analogous to LivenessNode::LiveToVarMap, but for non-local
  /// variables.
  LivenessVector<Variable *> LiveToVarMap;
  /// RangeMask[Variable::Number] indicates whether we want to track that
  /// Variable's live range.
  LivenessBV RangeMask;
  /// ScratchBV is a bitvector that can be reused across CfgNode passes, to
  /// avoid having to allocate/deallocate memory so frequently.
  LivenessBV ScratchBV;
  /// MaxLocals indicates what is the maximum number of local variables in a
  /// single basic block, across all blocks in a function.
  SizeT MaxLocals = 0;
  /// NumGlobals indicates how many global variables (i.e., Multi Block) exist
  /// for a function.
  SizeT NumGlobals = 0;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICELIVENESS_H
