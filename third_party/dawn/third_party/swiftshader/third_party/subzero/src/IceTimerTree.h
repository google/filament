//===- subzero/src/IceTimerTree.h - Pass timer defs -------------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the TimerTree class, which allows flat and cumulative
/// execution time collection of call chains.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICETIMERTREE_H
#define SUBZERO_SRC_ICETIMERTREE_H

// TODO(jpp): Refactor IceDefs.
#include "IceDefs.h"
#include "IceTimerTree.def"

namespace Ice {

class TimerStack {
  TimerStack() = delete;
  TimerStack &operator=(const TimerStack &) = delete;

  /// Timer tree index type. A variable of this type is used to access an
  /// interior, not-necessarily-leaf node of the tree.
  using TTindex = std::vector<class TimerTreeNode>::size_type;
  /// Representation of a path of leaf values leading to a particular node. The
  /// representation happens to be in "reverse" order, i.e. from leaf/interior
  /// to root, for implementation efficiency.
  using PathType = llvm::SmallVector<TTindex, 8>;
  /// Representation of a mapping of leaf node indexes from one timer stack to
  /// another.
  using TranslationType = std::vector<TimerIdT>;

  /// TimerTreeNode represents an interior or leaf node in the call tree. It
  /// contains a list of children, a pointer to its parent, and the timer ID for
  /// the node. It also holds the cumulative time spent at this node and below.
  /// The children are always at a higher index in the TimerTreeNode::Nodes
  /// array, and the parent is always at a lower index.
  class TimerTreeNode {
    TimerTreeNode &operator=(const TimerTreeNode &) = delete;

  public:
    TimerTreeNode() = default;
    TimerTreeNode(const TimerTreeNode &) = default;
    std::vector<TTindex> Children; // indexed by TimerIdT
    TTindex Parent = 0;
    TimerIdT Interior = 0;
    double Time = 0;
    size_t UpdateCount = 0;
  };

public:
  enum TimerTag {
#define X(tag) TT_##tag,
    TIMERTREE_TABLE
#undef X
        TT__num
  };
  explicit TimerStack(const std::string &Name);
  TimerStack(const TimerStack &) = default;
  TimerIdT getTimerID(const std::string &Name);
  void mergeFrom(const TimerStack &Src);
  void setName(const std::string &NewName) { Name = NewName; }
  const std::string &getName() const { return Name; }
  void push(TimerIdT ID);
  void pop(TimerIdT ID);
  void reset();
  void dump(Ostream &Str, bool DumpCumulative);

private:
  void update(bool UpdateCounts);
  static double timestamp();
  TranslationType translateIDsFrom(const TimerStack &Src);
  PathType getPath(TTindex Index, const TranslationType &Mapping) const;
  TTindex getChildIndex(TTindex Parent, TimerIdT ID);
  TTindex findPath(const PathType &Path);
  std::string Name;
  double FirstTimestamp;
  double LastTimestamp;
  uint64_t StateChangeCount = 0;
  /// IDsIndex maps a symbolic timer name to its integer ID.
  std::map<std::string, TimerIdT> IDsIndex;
  std::vector<std::string> IDs;     /// indexed by TimerIdT
  std::vector<TimerTreeNode> Nodes; /// indexed by TTindex
  std::vector<double> LeafTimes;    /// indexed by TimerIdT
  std::vector<size_t> LeafCounts;   /// indexed by TimerIdT
  TTindex StackTop = 0;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICETIMERTREE_H
