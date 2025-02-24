//===- subzero/src/IceLiveness.cpp - Liveness analysis implementation -----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Provides some of the support for the Liveness class.

/// In particular, it handles the sparsity representation of the mapping
/// between Variables and CfgNodes. The idea is that since most variables are
/// used only within a single basic block, we can partition the variables into
/// "local" and "global" sets. Instead of sizing and indexing vectors according
/// to Variable::Number, we create a mapping such that global variables are
/// mapped to low indexes that are common across nodes, and local variables are
/// mapped to a higher index space that is shared across nodes.
///
//===----------------------------------------------------------------------===//

#include "IceLiveness.h"

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceDefs.h"
#include "IceInst.h"
#include "IceOperand.h"

namespace Ice {

// Initializes the basic liveness-related data structures for full liveness
// analysis (IsFullInit=true), or for incremental update after phi lowering
// (IsFullInit=false). In the latter case, FirstNode points to the first node
// added since starting phi lowering, and FirstVar points to the first Variable
// added since starting phi lowering.
void Liveness::initInternal(NodeList::const_iterator FirstNode,
                            VarList::const_iterator FirstVar, bool IsFullInit) {
  // Initialize most of the container sizes.
  SizeT NumVars = Func->getVariables().size();
  SizeT NumNodes = Func->getNumNodes();
  VariablesMetadata *VMetadata = Func->getVMetadata();
  Nodes.resize(NumNodes);
  VarToLiveMap.resize(NumVars);

  // Count the number of globals, and the number of locals for each block.
  SizeT TmpNumGlobals = 0;
  for (auto I = FirstVar, E = Func->getVariables().end(); I != E; ++I) {
    Variable *Var = *I;
    if (VMetadata->isMultiBlock(Var)) {
      ++TmpNumGlobals;
    } else if (VMetadata->isSingleBlock(Var)) {
      SizeT Index = VMetadata->getLocalUseNode(Var)->getIndex();
      ++Nodes[Index].NumLocals;
    }
  }
  if (IsFullInit)
    NumGlobals = TmpNumGlobals;
  else
    assert(TmpNumGlobals == 0);

  // Resize each LivenessNode::LiveToVarMap, and the global LiveToVarMap. Reset
  // the counts to 0.
  for (auto I = FirstNode, E = Func->getNodes().end(); I != E; ++I) {
    LivenessNode &N = Nodes[(*I)->getIndex()];
    N.LiveToVarMap.assign(N.NumLocals, nullptr);
    N.NumLocals = 0;
    N.NumNonDeadPhis = 0;
  }
  if (IsFullInit)
    LiveToVarMap.assign(NumGlobals, nullptr);

  // Initialize the bitmask of which variables to track.
  RangeMask.resize(NumVars);
  RangeMask.set(0, NumVars); // Track all variables by default.

  // Sort each variable into the appropriate LiveToVarMap. Set VarToLiveMap.
  // Set RangeMask correctly for each variable.
  TmpNumGlobals = 0;
  for (auto I = FirstVar, E = Func->getVariables().end(); I != E; ++I) {
    Variable *Var = *I;
    SizeT VarIndex = Var->getIndex();
    SizeT LiveIndex = InvalidLiveIndex;
    if (VMetadata->isMultiBlock(Var)) {
      LiveIndex = TmpNumGlobals++;
      LiveToVarMap[LiveIndex] = Var;
    } else if (VMetadata->isSingleBlock(Var)) {
      SizeT NodeIndex = VMetadata->getLocalUseNode(Var)->getIndex();
      LiveIndex = Nodes[NodeIndex].NumLocals++;
      Nodes[NodeIndex].LiveToVarMap[LiveIndex] = Var;
      LiveIndex += NumGlobals;
    }
    VarToLiveMap[VarIndex] = LiveIndex;
    if (LiveIndex == InvalidLiveIndex || Var->getIgnoreLiveness())
      RangeMask[VarIndex] = false;
  }
  assert(TmpNumGlobals == (IsFullInit ? NumGlobals : 0));

  // Fix up RangeMask for variables before FirstVar.
  for (auto I = Func->getVariables().begin(); I != FirstVar; ++I) {
    Variable *Var = *I;
    SizeT VarIndex = Var->getIndex();
    if (Var->getIgnoreLiveness() ||
        (!IsFullInit && !Var->hasReg() && !Var->mustHaveReg()))
      RangeMask[VarIndex] = false;
  }

  // Process each node.
  MaxLocals = 0;
  for (auto I = FirstNode, E = Func->getNodes().end(); I != E; ++I) {
    LivenessNode &Node = Nodes[(*I)->getIndex()];
    // NumLocals, LiveToVarMap already initialized
    Node.LiveIn.resize(NumGlobals);
    Node.LiveOut.resize(NumGlobals);
    // LiveBegin and LiveEnd are reinitialized before each pass over the block.
    MaxLocals = std::max(MaxLocals, Node.NumLocals);
  }
  ScratchBV.reserve(NumGlobals + MaxLocals);
}

void Liveness::init() {
  constexpr bool IsFullInit = true;
  NodeList::const_iterator FirstNode = Func->getNodes().begin();
  VarList::const_iterator FirstVar = Func->getVariables().begin();
  initInternal(FirstNode, FirstVar, IsFullInit);
}

void Liveness::initPhiEdgeSplits(NodeList::const_iterator FirstNode,
                                 VarList::const_iterator FirstVar) {
  constexpr bool IsFullInit = false;
  initInternal(FirstNode, FirstVar, IsFullInit);
}

Variable *Liveness::getVariable(SizeT LiveIndex, const CfgNode *Node) const {
  if (LiveIndex < NumGlobals)
    return LiveToVarMap[LiveIndex];
  SizeT NodeIndex = Node->getIndex();
  return Nodes[NodeIndex].LiveToVarMap[LiveIndex - NumGlobals];
}

} // end of namespace Ice
