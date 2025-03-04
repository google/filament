//===- PostOrderCFGView.h - Post order view of CFG blocks ---------*- C++ --*-//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements post order view of the blocks in a CFG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_ANALYSIS_ANALYSES_POSTORDERCFGVIEW_H
#define LLVM_CLANG_ANALYSIS_ANALYSES_POSTORDERCFGVIEW_H

#include <vector>
//#include <algorithm>

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/BitVector.h"

#include "clang/Analysis/AnalysisContext.h"
#include "clang/Analysis/CFG.h"

namespace clang {

class PostOrderCFGView : public ManagedAnalysis {
  virtual void anchor();
public:
  /// \brief Implements a set of CFGBlocks using a BitVector.
  ///
  /// This class contains a minimal interface, primarily dictated by the SetType
  /// template parameter of the llvm::po_iterator template, as used with
  /// external storage. We also use this set to keep track of which CFGBlocks we
  /// visit during the analysis.
  class CFGBlockSet {
    llvm::BitVector VisitedBlockIDs;
  public:
    // po_iterator requires this iterator, but the only interface needed is the
    // value_type typedef.
    struct iterator { typedef const CFGBlock *value_type; };

    CFGBlockSet() {}
    CFGBlockSet(const CFG *G) : VisitedBlockIDs(G->getNumBlockIDs(), false) {}

    /// \brief Set the bit associated with a particular CFGBlock.
    /// This is the important method for the SetType template parameter.
    std::pair<llvm::NoneType, bool> insert(const CFGBlock *Block) {
      // Note that insert() is called by po_iterator, which doesn't check to
      // make sure that Block is non-null.  Moreover, the CFGBlock iterator will
      // occasionally hand out null pointers for pruned edges, so we catch those
      // here.
      if (!Block)
        return std::make_pair(None, false); // if an edge is trivially false.
      if (VisitedBlockIDs.test(Block->getBlockID()))
        return std::make_pair(None, false);
      VisitedBlockIDs.set(Block->getBlockID());
      return std::make_pair(None, true);
    }

    /// \brief Check if the bit for a CFGBlock has been already set.
    /// This method is for tracking visited blocks in the main threadsafety
    /// loop. Block must not be null.
    bool alreadySet(const CFGBlock *Block) {
      return VisitedBlockIDs.test(Block->getBlockID());
    }
  };

private:
  typedef llvm::po_iterator<const CFG*, CFGBlockSet, true>  po_iterator;
  std::vector<const CFGBlock*> Blocks;

  typedef llvm::DenseMap<const CFGBlock *, unsigned> BlockOrderTy;
  BlockOrderTy BlockOrder;

public:
  typedef std::vector<const CFGBlock *>::reverse_iterator iterator;
  typedef std::vector<const CFGBlock *>::const_reverse_iterator const_iterator;

  PostOrderCFGView(const CFG *cfg);

  iterator begin() { return Blocks.rbegin(); }
  iterator end()   { return Blocks.rend(); }

  const_iterator begin() const { return Blocks.rbegin(); }
  const_iterator end() const { return Blocks.rend(); }

  bool empty() const { return begin() == end(); }

  struct BlockOrderCompare;
  friend struct BlockOrderCompare;

  struct BlockOrderCompare {
    const PostOrderCFGView &POV;
  public:
    BlockOrderCompare(const PostOrderCFGView &pov) : POV(pov) {}
    bool operator()(const CFGBlock *b1, const CFGBlock *b2) const;
  };

  BlockOrderCompare getComparator() const {
    return BlockOrderCompare(*this);
  }

  // Used by AnalyisContext to construct this object.
  static const void *getTag();

  static PostOrderCFGView *create(AnalysisDeclContext &analysisContext);
};

} // end clang namespace

#endif

