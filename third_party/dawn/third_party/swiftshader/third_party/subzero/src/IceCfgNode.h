//===- subzero/src/IceCfgNode.h - Control flow graph node -------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the CfgNode class, which represents a single basic block as
/// its instruction list, in-edge list, and out-edge list.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICECFGNODE_H
#define SUBZERO_SRC_ICECFGNODE_H

#include "IceDefs.h"
#include "IceInst.h" // InstList traits
#include "IceStringPool.h"

namespace Ice {

class CfgNode {
  CfgNode() = delete;
  CfgNode(const CfgNode &) = delete;
  CfgNode &operator=(const CfgNode &) = delete;

public:
  static CfgNode *create(Cfg *Func, SizeT Number) {
    return new (Func->allocate<CfgNode>()) CfgNode(Func, Number);
  }

  Cfg *getCfg() const { return Func; }

  /// Access the label number and name for this node.
  SizeT getIndex() const { return Number; }
  void resetIndex(SizeT NewNumber) { Number = NewNumber; }
  std::string getName() const {
    if (Name.hasStdString())
      return Name.toString();
    return "__" + std::to_string(NumberOrig);
  }
  void setName(const std::string &NewName) {
    if (NewName.empty())
      return;
    Name = NodeString::createWithString(Func, NewName);
  }
  std::string getAsmName() const {
    return ".L" + Func->getFunctionName() + "$" + getName();
  }

  void incrementLoopNestDepth() { ++LoopNestDepth; }
  void setLoopNestDepth(SizeT NewDepth) { LoopNestDepth = NewDepth; }
  SizeT getLoopNestDepth() const { return LoopNestDepth; }

  /// The HasReturn flag indicates that this node contains a return instruction
  /// and therefore needs an epilog.
  void setHasReturn() { HasReturn = true; }
  bool getHasReturn() const { return HasReturn; }

  void setNeedsPlacement(bool Value) { NeedsPlacement = Value; }
  bool needsPlacement() const { return NeedsPlacement; }

  void setNeedsAlignment() { NeedsAlignment = true; }
  bool needsAlignment() const { return NeedsAlignment; }

  /// \name Access predecessor and successor edge lists.
  /// @{
  const NodeList &getInEdges() const { return InEdges; }
  const NodeList &getOutEdges() const { return OutEdges; }
  /// @}

  /// \name Manage the instruction list.
  /// @{
  InstList &getInsts() { return Insts; }
  PhiList &getPhis() { return Phis; }
  const InstList &getInsts() const { return Insts; }
  const PhiList &getPhis() const { return Phis; }
  void appendInst(Inst *Instr);
  void renumberInstructions();
  /// Rough and generally conservative estimate of the number of instructions in
  /// the block. It is updated when an instruction is added, but not when
  /// deleted. It is recomputed during renumberInstructions().
  InstNumberT getInstCountEstimate() const { return InstCountEstimate; }
  /// @}

  /// \name Manage predecessors and successors.
  /// @{

  /// Add a predecessor edge to the InEdges list for each of this node's
  /// successors.
  void computePredecessors();
  void computeSuccessors();
  CfgNode *splitIncomingEdge(CfgNode *Pred, SizeT InEdgeIndex);
  /// @}

  void enforcePhiConsistency();
  void placePhiLoads();
  void placePhiStores();
  void deletePhis();
  void advancedPhiLowering();
  void doAddressOpt();
  void genCode();
  void livenessLightweight();
  bool liveness(Liveness *Liveness);
  void livenessAddIntervals(Liveness *Liveness, InstNumberT FirstInstNum,
                            InstNumberT LastInstNum);
  void contractIfEmpty();
  void doBranchOpt(const CfgNode *NextNode);
  void emit(Cfg *Func) const;
  void emitIAS(Cfg *Func) const;
  void dump(Cfg *Func) const;

  void profileExecutionCount(VariableDeclaration *Var);

  void addOutEdge(CfgNode *Out) { OutEdges.push_back(Out); }
  void addInEdge(CfgNode *In) { InEdges.push_back(In); }
  void replaceInEdge(CfgNode *Old, CfgNode *New);
  void removeAllOutEdges() { OutEdges.clear(); }
  void removeInEdge(CfgNode *In);

  bool hasSingleOutEdge() const {
    return (getOutEdges().size() == 1 || getOutEdges()[0] == getOutEdges()[1]);
  }
  CfgNode *shortCircuit();

  inline void *getExternalData() const { return externalData; }
  inline void setExternalData(void *data) { externalData = data; }

private:
  CfgNode(Cfg *Func, SizeT Number)
      : Func(Func), Number(Number), NumberOrig(Number),
        Name(NodeString::createWithoutString(Func)) {}
  bool livenessValidateIntervals(Liveness *Liveness) const;
  Cfg *const Func;
  SizeT Number;           /// invariant: Func->Nodes[Number]==this
  const SizeT NumberOrig; /// used for name auto-generation
  NodeString Name;
  SizeT LoopNestDepth = 0; /// the loop nest depth of this node
  bool HasReturn = false;  /// does this block need an epilog?
  bool NeedsPlacement = false;
  bool NeedsAlignment = false;       /// is sandboxing required?
  InstNumberT InstCountEstimate = 0; /// rough instruction count estimate
  NodeList InEdges;                  /// in no particular order
  NodeList OutEdges;                 /// in no particular order
  PhiList Phis;                      /// unordered set of phi instructions
  InstList Insts;                    /// ordered list of non-phi instructions

  /// External data can be set by an optimizer to compute and retain any
  /// information related to the current node. All the memory used to
  /// store this information must be managed by the optimizer.
  void *externalData = nullptr;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICECFGNODE_H
