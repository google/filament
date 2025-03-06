//==- WorkList.h - Worklist class used by CoreEngine ---------------*- C++ -*-//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines WorkList, a pure virtual class that represents an opaque
//  worklist used by CoreEngine to explore the reachability state space.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_STATICANALYZER_CORE_PATHSENSITIVE_WORKLIST_H
#define LLVM_CLANG_STATICANALYZER_CORE_PATHSENSITIVE_WORKLIST_H

#include "clang/StaticAnalyzer/Core/PathSensitive/BlockCounter.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ExplodedGraph.h"
#include <cassert>

namespace clang {
  
class CFGBlock;

namespace ento {

class WorkListUnit {
  ExplodedNode *node;
  BlockCounter counter;
  const CFGBlock *block;
  unsigned blockIdx; // This is the index of the next statement.

public:
  WorkListUnit(ExplodedNode *N, BlockCounter C,
               const CFGBlock *B, unsigned idx)
  : node(N),
    counter(C),
    block(B),
    blockIdx(idx) {}

  explicit WorkListUnit(ExplodedNode *N, BlockCounter C)
  : node(N),
    counter(C),
    block(nullptr),
    blockIdx(0) {}

  /// Returns the node associated with the worklist unit.
  ExplodedNode *getNode() const { return node; }
  
  /// Returns the block counter map associated with the worklist unit.
  BlockCounter getBlockCounter() const { return counter; }

  /// Returns the CFGblock associated with the worklist unit.
  const CFGBlock *getBlock() const { return block; }
  
  /// Return the index within the CFGBlock for the worklist unit.
  unsigned getIndex() const { return blockIdx; }
};

class WorkList {
  BlockCounter CurrentCounter;
public:
  virtual ~WorkList();
  virtual bool hasWork() const = 0;

  virtual void enqueue(const WorkListUnit& U) = 0;

  void enqueue(ExplodedNode *N, const CFGBlock *B, unsigned idx) {
    enqueue(WorkListUnit(N, CurrentCounter, B, idx));
  }

  void enqueue(ExplodedNode *N) {
    assert(N->getLocation().getKind() != ProgramPoint::PostStmtKind);
    enqueue(WorkListUnit(N, CurrentCounter));
  }

  virtual WorkListUnit dequeue() = 0;

  void setBlockCounter(BlockCounter C) { CurrentCounter = C; }
  BlockCounter getBlockCounter() const { return CurrentCounter; }

  class Visitor {
  public:
    Visitor() {}
    virtual ~Visitor();
    virtual bool visit(const WorkListUnit &U) = 0;
  };
  virtual bool visitItemsInWorkList(Visitor &V) = 0;
  
  static WorkList *makeDFS();
  static WorkList *makeBFS();
  static WorkList *makeBFSBlockDFSContents();
};

} // end GR namespace

} // end clang namespace

#endif
