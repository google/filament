//===- subzero/src/IceLoopAnalyzer.cpp - Loop Analysis --------------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the loop analysis on the CFG.
///
//===----------------------------------------------------------------------===//
#include "IceLoopAnalyzer.h"

#include "IceCfg.h"
#include "IceCfgNode.h"

#include <algorithm>

namespace Ice {
class LoopAnalyzer {
public:
  explicit LoopAnalyzer(Cfg *Func);

  /// Use Tarjan's strongly connected components algorithm to identify outermost
  /// to innermost loops. By deleting the head of the loop from the graph, inner
  /// loops can be found. This assumes that the head node is not shared between
  /// loops but instead all paths to the head come from 'continue' constructs.
  ///
  /// This only computes the loop nest depth within the function and does not
  /// take into account whether the function was called from within a loop.
  // TODO(ascull): this currently uses a extension of Tarjan's algorithm with
  // is bounded linear. ncbray suggests another algorithm which is linear in
  // practice but not bounded linear. I think it also finds dominators.
  // http://lenx.100871.net/papers/loop-SAS.pdf

  CfgVector<CfgUnorderedSet<SizeT>> getLoopBodies() { return Loops; }

private:
  LoopAnalyzer() = delete;
  LoopAnalyzer(const LoopAnalyzer &) = delete;
  LoopAnalyzer &operator=(const LoopAnalyzer &) = delete;
  void computeLoopNestDepth();

  using IndexT = uint32_t;
  static constexpr IndexT UndefinedIndex = 0;
  static constexpr IndexT FirstDefinedIndex = 1;

  // TODO(ascull): classify the other fields
  class LoopNode {
    LoopNode() = delete;
    LoopNode operator=(const LoopNode &) = delete;

  public:
    explicit LoopNode(CfgNode *BB) : BB(BB) { reset(); }
    LoopNode(const LoopNode &) = default;

    void reset();

    NodeList::const_iterator successorsEnd() const;
    NodeList::const_iterator currentSuccessor() const { return Succ; }
    void nextSuccessor() { ++Succ; }

    void visit(IndexT VisitIndex) { Index = LowLink = VisitIndex; }
    bool isVisited() const { return Index != UndefinedIndex; }
    IndexT getIndex() const { return Index; }

    void tryLink(IndexT NewLink) {
      if (NewLink < LowLink)
        LowLink = NewLink;
    }
    IndexT getLowLink() const { return LowLink; }

    void setOnStack(bool NewValue = true) { OnStack = NewValue; }
    bool isOnStack() const { return OnStack; }

    void setDeleted() { Deleted = true; }
    bool isDeleted() const { return Deleted; }

    void incrementLoopNestDepth();
    bool hasSelfEdge() const;

    CfgNode *getNode() { return BB; }

  private:
    CfgNode *BB;
    NodeList::const_iterator Succ;
    IndexT Index;
    IndexT LowLink;
    bool OnStack;
    bool Deleted = false;
  };

  using LoopNodeList = CfgVector<LoopNode>;
  using LoopNodePtrList = CfgVector<LoopNode *>;

  /// Process the node as part as part of Tarjan's algorithm and return either a
  /// node to recurse into or nullptr when the node has been fully processed.
  LoopNode *processNode(LoopNode &Node);

  /// The function to analyze for loops.
  Cfg *const Func;
  /// A list of decorated nodes in the same order as Func->getNodes() which
  /// means the node's index will also be valid in this list.
  LoopNodeList AllNodes;
  /// This is used as a replacement for the call stack.
  LoopNodePtrList WorkStack;
  /// Track which loop a node belongs to.
  LoopNodePtrList LoopStack;
  /// The index to assign to the next visited node.
  IndexT NextIndex = FirstDefinedIndex;
  /// The number of nodes which have been marked deleted. This is used to track
  /// when the iteration should end.
  LoopNodePtrList::size_type NumDeletedNodes = 0;

  /// All the Loops, in descending order of size
  CfgVector<CfgUnorderedSet<SizeT>> Loops;
};
void LoopAnalyzer::LoopNode::reset() {
  if (Deleted)
    return;
  Succ = BB->getOutEdges().begin();
  Index = LowLink = UndefinedIndex;
  OnStack = false;
}

NodeList::const_iterator LoopAnalyzer::LoopNode::successorsEnd() const {
  return BB->getOutEdges().end();
}

void LoopAnalyzer::LoopNode::incrementLoopNestDepth() {
  BB->incrementLoopNestDepth();
}

bool LoopAnalyzer::LoopNode::hasSelfEdge() const {
  for (CfgNode *Succ : BB->getOutEdges()) {
    if (Succ == BB)
      return true;
  }
  return false;
}

LoopAnalyzer::LoopAnalyzer(Cfg *Fn) : Func(Fn) {
  const NodeList &Nodes = Func->getNodes();

  // Allocate memory ahead of time. This is why a vector is used instead of a
  // stack which doesn't support reserving (or bulk erasure used below).
  AllNodes.reserve(Nodes.size());
  WorkStack.reserve(Nodes.size());
  LoopStack.reserve(Nodes.size());

  // Create the LoopNodes from the function's CFG
  for (CfgNode *Node : Nodes)
    AllNodes.emplace_back(Node);
  computeLoopNestDepth();
}

void LoopAnalyzer::computeLoopNestDepth() {
  assert(AllNodes.size() == Func->getNodes().size());
  assert(NextIndex == FirstDefinedIndex);
  assert(NumDeletedNodes == 0);

  while (NumDeletedNodes < AllNodes.size()) {
    // Prepare to run Tarjan's
    for (LoopNode &Node : AllNodes)
      Node.reset();

    assert(WorkStack.empty());
    assert(LoopStack.empty());

    for (LoopNode &Node : AllNodes) {
      if (Node.isDeleted() || Node.isVisited())
        continue;

      WorkStack.push_back(&Node);

      while (!WorkStack.empty()) {
        LoopNode &WorkNode = *WorkStack.back();
        if (LoopNode *Succ = processNode(WorkNode))
          WorkStack.push_back(Succ);
        else
          WorkStack.pop_back();
      }
    }
  }
}

LoopAnalyzer::LoopNode *
LoopAnalyzer::processNode(LoopAnalyzer::LoopNode &Node) {
  if (!Node.isVisited()) {
    Node.visit(NextIndex++);
    LoopStack.push_back(&Node);
    Node.setOnStack();
  } else {
    // Returning to a node after having recursed into Succ so continue
    // iterating through successors after using the Succ.LowLink value that was
    // computed in the recursion.
    LoopNode &Succ = AllNodes[(*Node.currentSuccessor())->getIndex()];
    Node.tryLink(Succ.getLowLink());
    Node.nextSuccessor();
  }

  // Visit the successors and recurse into unvisited nodes. The recursion could
  // cause the iteration to be suspended but it will resume as the stack is
  // unwound.
  auto SuccEnd = Node.successorsEnd();
  for (; Node.currentSuccessor() != SuccEnd; Node.nextSuccessor()) {
    LoopNode &Succ = AllNodes[(*Node.currentSuccessor())->getIndex()];

    if (Succ.isDeleted())
      continue;

    if (!Succ.isVisited())
      return &Succ;
    else if (Succ.isOnStack())
      Node.tryLink(Succ.getIndex());
  }

  if (Node.getLowLink() != Node.getIndex())
    return nullptr;

  // Single node means no loop in the CFG
  if (LoopStack.back() == &Node) {
    LoopStack.back()->setOnStack(false);
    if (Node.hasSelfEdge())
      LoopStack.back()->incrementLoopNestDepth();
    LoopStack.back()->setDeleted();
    ++NumDeletedNodes;
    LoopStack.pop_back();
    return nullptr;
  }

  // Reaching here means a loop has been found! It consists of the nodes on the
  // top of the stack, down until the current node being processed, Node, is
  // found.
  for (auto It = LoopStack.rbegin(); It != LoopStack.rend(); ++It) {
    (*It)->setOnStack(false);
    (*It)->incrementLoopNestDepth();
    // Remove the loop from the stack and delete the head node
    if (*It == &Node) {
      (*It)->setDeleted();
      ++NumDeletedNodes;
      CfgUnorderedSet<SizeT> LoopNodes;
      for (auto LoopIter = It.base() - 1; LoopIter != LoopStack.end();
           ++LoopIter) {
        LoopNodes.insert((*LoopIter)->getNode()->getIndex());
      }
      Loops.push_back(LoopNodes);
      LoopStack.erase(It.base() - 1, LoopStack.end());
      break;
    }
  }

  return nullptr;
}
CfgVector<Loop> ComputeLoopInfo(Cfg *Func) {
  auto LoopBodies = LoopAnalyzer(Func).getLoopBodies();

  CfgVector<Loop> Loops;
  Loops.reserve(LoopBodies.size());
  std::sort(
      LoopBodies.begin(), LoopBodies.end(),
      [](const CfgUnorderedSet<SizeT> &A, const CfgUnorderedSet<SizeT> &B) {
        return A.size() > B.size();
      });
  for (auto &LoopBody : LoopBodies) {
    CfgNode *Header = nullptr;
    bool IsSimpleLoop = true;
    for (auto NodeIndex : LoopBody) {
      CfgNode *Cur = Func->getNodes()[NodeIndex];
      for (auto *Prev : Cur->getInEdges()) {
        if (LoopBody.find(Prev->getIndex()) ==
            LoopBody.end()) { // coming from outside
          if (Header == nullptr) {
            Header = Cur;
          } else {
            Header = nullptr;
            IsSimpleLoop = false;
            break;
          }
        }
      }
      if (!IsSimpleLoop) {
        break;
      }
    }
    if (!IsSimpleLoop)
      continue; // To next potential loop

    CfgNode *PreHeader = nullptr;
    for (auto *Prev : Header->getInEdges()) {
      if (LoopBody.find(Prev->getIndex()) == LoopBody.end()) {
        if (PreHeader == nullptr) {
          PreHeader = Prev;
        } else {
          PreHeader = nullptr;
          break;
        }
      }
    }

    Loops.emplace_back(Header, PreHeader, LoopBody);
  }
  return Loops;
}

} // end of namespace Ice
