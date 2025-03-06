//==- Dominators.h - Implementation of dominators tree for Clang CFG C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the dominators tree functionality for Clang CFGs.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_ANALYSIS_ANALYSES_DOMINATORS_H
#define LLVM_CLANG_ANALYSIS_ANALYSES_DOMINATORS_H

#include "clang/Analysis/AnalysisContext.h"
#include "clang/Analysis/CFG.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/Support/GenericDomTree.h"
#include "llvm/Support/GenericDomTreeConstruction.h"

// FIXME: There is no good reason for the domtree to require a print method
// which accepts an LLVM Module, so remove this (and the method's argument that
// needs it) when that is fixed.
namespace llvm {
class Module;
}

namespace clang {

class CFGBlock;
typedef llvm::DomTreeNodeBase<CFGBlock> DomTreeNode;

/// \brief Concrete subclass of DominatorTreeBase for Clang
/// This class implements the dominators tree functionality given a Clang CFG.
///
class DominatorTree : public ManagedAnalysis {
  virtual void anchor();
public:
  llvm::DominatorTreeBase<CFGBlock>* DT;

  DominatorTree() {
    DT = new llvm::DominatorTreeBase<CFGBlock>(false);
  }

  ~DominatorTree() override { delete DT; }

  llvm::DominatorTreeBase<CFGBlock>& getBase() { return *DT; }

  /// \brief This method returns the root CFGBlock of the dominators tree.
  ///
  inline CFGBlock *getRoot() const {
    return DT->getRoot();
  }

  /// \brief This method returns the root DomTreeNode, which is the wrapper
  /// for CFGBlock.
  inline DomTreeNode *getRootNode() const {
    return DT->getRootNode();
  }

  /// \brief This method compares two dominator trees.
  /// The method returns false if the other dominator tree matches this
  /// dominator tree, otherwise returns true.
  ///
  inline bool compare(DominatorTree &Other) const {
    DomTreeNode *R = getRootNode();
    DomTreeNode *OtherR = Other.getRootNode();

    if (!R || !OtherR || R->getBlock() != OtherR->getBlock())
      return true;

    if (DT->compare(Other.getBase()))
      return true;

    return false;
  }

  /// \brief This method builds the dominator tree for a given CFG
  /// The CFG information is passed via AnalysisDeclContext
  ///
  void buildDominatorTree(AnalysisDeclContext &AC) {
    cfg = AC.getCFG();
    DT->recalculate(*cfg);
  }

  /// \brief This method dumps immediate dominators for each block,
  /// mainly used for debug purposes.
  ///
  void dump() {
    llvm::errs() << "Immediate dominance tree (Node#,IDom#):\n";
    for (CFG::const_iterator I = cfg->begin(),
        E = cfg->end(); I != E; ++I) {
      if(DT->getNode(*I)->getIDom())
        llvm::errs() << "(" << (*I)->getBlockID()
                     << ","
                     << DT->getNode(*I)->getIDom()->getBlock()->getBlockID()
                     << ")\n";
      else llvm::errs() << "(" << (*I)->getBlockID()
                        << "," << (*I)->getBlockID() << ")\n";
    }
  }

  /// \brief This method tests if one CFGBlock dominates the other.
  /// The method return true if A dominates B, false otherwise.
  /// Note a block always dominates itself.
  ///
  inline bool dominates(const CFGBlock* A, const CFGBlock* B) const {
    return DT->dominates(A, B);
  }

  /// \brief This method tests if one CFGBlock properly dominates the other.
  /// The method return true if A properly dominates B, false otherwise.
  ///
  bool properlyDominates(const CFGBlock*A, const CFGBlock*B) const {
    return DT->properlyDominates(A, B);
  }

  /// \brief This method finds the nearest common dominator CFG block
  /// for CFG block A and B. If there is no such block then return NULL.
  ///
  inline CFGBlock *findNearestCommonDominator(CFGBlock *A, CFGBlock *B) {
    return DT->findNearestCommonDominator(A, B);
  }

  inline const CFGBlock *findNearestCommonDominator(const CFGBlock *A,
                                                      const CFGBlock *B) {
    return DT->findNearestCommonDominator(A, B);
  }

  /// \brief This method is used to update the dominator
  /// tree information when a node's immediate dominator changes.
  ///
  inline void changeImmediateDominator(CFGBlock *N, CFGBlock *NewIDom) {
    DT->changeImmediateDominator(N, NewIDom);
  }

  /// \brief This method tests if the given CFGBlock can be reachable from root.
  /// Returns true if reachable, false otherwise.
  ///
  bool isReachableFromEntry(const CFGBlock *A) {
    return DT->isReachableFromEntry(A);
  }

  /// \brief This method releases the memory held by the dominator tree.
  ///
  virtual void releaseMemory() {
    DT->releaseMemory();
  }

  /// \brief This method converts the dominator tree to human readable form.
  ///
  virtual void print(raw_ostream &OS, const llvm::Module* M= nullptr) const {
    DT->print(OS);
  }

private:
  CFG *cfg;
};

} // end namespace clang

//===-------------------------------------
/// DominatorTree GraphTraits specialization so the DominatorTree can be
/// iterable by generic graph iterators.
///
namespace llvm {
template <> struct GraphTraits< ::clang::DomTreeNode* > {
  typedef ::clang::DomTreeNode NodeType;
  typedef NodeType::iterator  ChildIteratorType;

  static NodeType *getEntryNode(NodeType *N) {
    return N;
  }
  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->end();
  }

  typedef df_iterator< ::clang::DomTreeNode* > nodes_iterator;

  static nodes_iterator nodes_begin(::clang::DomTreeNode *N) {
    return df_begin(getEntryNode(N));
  }

  static nodes_iterator nodes_end(::clang::DomTreeNode *N) {
    return df_end(getEntryNode(N));
  }
};

template <> struct GraphTraits< ::clang::DominatorTree* >
  : public GraphTraits< ::clang::DomTreeNode* > {
  static NodeType *getEntryNode(::clang::DominatorTree *DT) {
    return DT->getRootNode();
  }

  static nodes_iterator nodes_begin(::clang::DominatorTree *N) {
    return df_begin(getEntryNode(N));
  }

  static nodes_iterator nodes_end(::clang::DominatorTree *N) {
    return df_end(getEntryNode(N));
  }
};
} // end namespace llvm

#endif
