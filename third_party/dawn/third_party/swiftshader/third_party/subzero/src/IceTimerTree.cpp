//===- subzero/src/IceTimerTree.cpp - Pass timer defs ---------------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the TimerTree class, which tracks flat and cumulative
/// execution time collection of call chains.
///
//===----------------------------------------------------------------------===//

#include "IceTimerTree.h"

#include "IceDefs.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif // __clang__

#include "llvm/Support/Format.h"
#include "llvm/Support/Timer.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__

namespace Ice {

TimerStack::TimerStack(const std::string &Name)
    : Name(Name), FirstTimestamp(timestamp()), LastTimestamp(FirstTimestamp) {
  if (!BuildDefs::timers())
    return;
  Nodes.resize(1); // Reserve Nodes[0] for the root node (sentinel).
  IDs.resize(TT__num);
  LeafTimes.resize(TT__num);
  LeafCounts.resize(TT__num);
#define STR(s) #s
#define X(tag)                                                                 \
  IDs[TT_##tag] = STR(tag);                                                    \
  IDsIndex[STR(tag)] = TT_##tag;
  TIMERTREE_TABLE;
#undef X
#undef STR
}

// Returns the unique timer ID for the given Name, creating a new ID if needed.
TimerIdT TimerStack::getTimerID(const std::string &Name) {
  if (!BuildDefs::timers())
    return 0;
  if (IDsIndex.find(Name) == IDsIndex.end()) {
    IDsIndex[Name] = IDs.size();
    IDs.push_back(Name);
    LeafTimes.push_back(decltype(LeafTimes)::value_type());
    LeafCounts.push_back(decltype(LeafCounts)::value_type());
  }
  return IDsIndex[Name];
}

// Creates a mapping from TimerIdT (leaf) values in the Src timer stack into
// TimerIdT values in this timer stack. Creates new entries in this timer stack
// as needed.
TimerStack::TranslationType
TimerStack::translateIDsFrom(const TimerStack &Src) {
  size_t Size = Src.IDs.size();
  TranslationType Mapping(Size);
  for (TimerIdT i = 0; i < Size; ++i) {
    Mapping[i] = getTimerID(Src.IDs[i]);
  }
  return Mapping;
}

// Merges two timer stacks, by combining and summing corresponding entries.
// This timer stack is updated from Src.
void TimerStack::mergeFrom(const TimerStack &Src) {
  if (!BuildDefs::timers())
    return;
  TranslationType Mapping = translateIDsFrom(Src);
  TTindex SrcIndex = 0;
  for (const TimerTreeNode &SrcNode : Src.Nodes) {
    // The first node is reserved as a sentinel, so avoid it.
    if (SrcIndex > 0) {
      // Find the full path to the Src node, translated to path components
      // corresponding to this timer stack.
      PathType MyPath = Src.getPath(SrcIndex, Mapping);
      // Find a node in this timer stack corresponding to the given path,
      // creating new interior nodes as necessary.
      TTindex MyIndex = findPath(MyPath);
      Nodes[MyIndex].Time += SrcNode.Time;
      Nodes[MyIndex].UpdateCount += SrcNode.UpdateCount;
    }
    ++SrcIndex;
  }
  for (TimerIdT i = 0; i < Src.LeafTimes.size(); ++i) {
    LeafTimes[Mapping[i]] += Src.LeafTimes[i];
    LeafCounts[Mapping[i]] += Src.LeafCounts[i];
  }
  StateChangeCount += Src.StateChangeCount;
}

// Constructs a path consisting of the sequence of leaf values leading to a
// given node, with the Mapping translation applied to the leaf values. The
// path ends up being in "reverse" order, i.e. from leaf to root.
TimerStack::PathType TimerStack::getPath(TTindex Index,
                                         const TranslationType &Mapping) const {
  PathType Path;
  while (Index) {
    Path.push_back(Mapping[Nodes[Index].Interior]);
    assert(Nodes[Index].Parent < Index);
    Index = Nodes[Index].Parent;
  }
  return Path;
}

// Given a parent node and a leaf ID, returns the index of the parent's child
// ID, creating a new node for the child as necessary.
TimerStack::TTindex TimerStack::getChildIndex(TimerStack::TTindex Parent,
                                              TimerIdT ID) {
  if (Nodes[Parent].Children.size() <= ID)
    Nodes[Parent].Children.resize(ID + 1);
  if (Nodes[Parent].Children[ID] == 0) {
    TTindex Size = Nodes.size();
    Nodes[Parent].Children[ID] = Size;
    Nodes.resize(Size + 1);
    Nodes[Size].Parent = Parent;
    Nodes[Size].Interior = ID;
  }
  return Nodes[Parent].Children[ID];
}

// Finds a node in the timer stack corresponding to the given path, creating
// new interior nodes as necessary.
TimerStack::TTindex TimerStack::findPath(const PathType &Path) {
  TTindex CurIndex = 0;
  // The path is in reverse order (leaf to root), so it needs to be followed in
  // reverse.
  for (TTindex Index : reverse_range(Path)) {
    CurIndex = getChildIndex(CurIndex, Index);
  }
  assert(CurIndex); // shouldn't be the sentinel node
  return CurIndex;
}

// Pushes a new marker onto the timer stack.
void TimerStack::push(TimerIdT ID) {
  if (!BuildDefs::timers())
    return;
  constexpr bool UpdateCounts = false;
  update(UpdateCounts);
  StackTop = getChildIndex(StackTop, ID);
  assert(StackTop);
}

// Pops the top marker from the timer stack. Validates via assert() that the
// expected marker is popped.
void TimerStack::pop(TimerIdT ID) {
  if (!BuildDefs::timers())
    return;
  constexpr bool UpdateCounts = true;
  update(UpdateCounts);
  assert(StackTop);
  assert(Nodes[StackTop].Parent < StackTop);
  // Verify that the expected ID is being popped.
  assert(Nodes[StackTop].Interior == ID);
  (void)ID;
  // Verify that the parent's child points to the current stack top.
  assert(Nodes[Nodes[StackTop].Parent].Children[ID] == StackTop);
  StackTop = Nodes[StackTop].Parent;
}

// At a state change (e.g. push or pop), updates the flat and cumulative
// timings for everything on the timer stack.
void TimerStack::update(bool UpdateCounts) {
  if (!BuildDefs::timers())
    return;
  ++StateChangeCount;
  // Whenever the stack is about to change, we grab the time delta since the
  // last change and add it to all active cumulative elements and to the flat
  // element for the top of the stack.
  double Current = timestamp();
  double Delta = Current - LastTimestamp;
  if (StackTop) {
    TimerIdT Leaf = Nodes[StackTop].Interior;
    if (Leaf >= LeafTimes.size()) {
      LeafTimes.resize(Leaf + 1);
      LeafCounts.resize(Leaf + 1);
    }
    LeafTimes[Leaf] += Delta;
    if (UpdateCounts)
      ++LeafCounts[Leaf];
  }
  TTindex Prefix = StackTop;
  while (Prefix) {
    Nodes[Prefix].Time += Delta;
    // Only update a leaf node count, not the internal node counts.
    if (UpdateCounts && Prefix == StackTop)
      ++Nodes[Prefix].UpdateCount;
    TTindex Next = Nodes[Prefix].Parent;
    assert(Next < Prefix);
    Prefix = Next;
  }
  // Capture the next timestamp *after* the updates are finished. This
  // minimizes how much the timer can perturb the reported timing. The numbers
  // may not sum to 100%, and the missing amount is indicative of the overhead
  // of timing.
  LastTimestamp = timestamp();
}

void TimerStack::reset() {
  if (!BuildDefs::timers())
    return;
  StateChangeCount = 0;
  FirstTimestamp = LastTimestamp = timestamp();
  LeafTimes.assign(LeafTimes.size(), 0);
  LeafCounts.assign(LeafCounts.size(), 0);
  for (TimerTreeNode &Node : Nodes) {
    Node.Time = 0;
    Node.UpdateCount = 0;
  }
}

namespace {

using DumpMapType = std::multimap<double, std::string>;

// Dump the Map items in reverse order of their time contribution.  If
// AddPercents is true (i.e. for printing "flat times"), it also prints a
// cumulative percentage column, and recalculates TotalTime as the sum of all
// the individual times so that cumulative percentage adds up to 100%.
void dumpHelper(Ostream &Str, const DumpMapType &Map, double TotalTime,
                bool AddPercents) {
  if (!BuildDefs::timers())
    return;
  if (AddPercents) {
    // Recalculate TotalTime as the sum of the individual times.  This is
    // because the individual times generally add up to less than 100% because
    // of timer overhead.
    TotalTime = 0;
    for (const auto &I : Map) {
      TotalTime += I.first;
    }
  }
  double Sum = 0;
  for (const auto &I : reverse_range(Map)) {
    Sum += I.first;
    if (AddPercents) {
      Str << llvm::format("  %10.6f %4.1f%% %5.1f%% ", I.first,
                          I.first * 100 / TotalTime, Sum * 100 / TotalTime)
          << I.second << "\n";
    } else {
      Str << llvm::format("  %10.6f %4.1f%% ", I.first,
                          I.first * 100 / TotalTime)
          << I.second << "\n";
    }
  }
}

} // end of anonymous namespace

void TimerStack::dump(Ostream &Str, bool DumpCumulative) {
  if (!BuildDefs::timers())
    return;
  constexpr bool UpdateCounts = true;
  update(UpdateCounts);
  double TotalTime = LastTimestamp - FirstTimestamp;
  assert(TotalTime);
  char PrefixStr[30];
  if (DumpCumulative) {
    Str << Name
        << " - Cumulative times:\n"
           "     Seconds   Pct  EventCnt TimerPath\n";
    DumpMapType CumulativeMap;
    for (TTindex i = 1; i < Nodes.size(); ++i) {
      TTindex Prefix = i;
      std::string Suffix = "";
      while (Prefix) {
        if (Suffix.empty())
          Suffix = IDs[Nodes[Prefix].Interior];
        else
          Suffix = IDs[Nodes[Prefix].Interior] + "." + Suffix;
        assert(Nodes[Prefix].Parent < Prefix);
        Prefix = Nodes[Prefix].Parent;
      }
      snprintf(PrefixStr, llvm::array_lengthof(PrefixStr), "%9zu ",
               Nodes[i].UpdateCount);
      CumulativeMap.insert(std::make_pair(Nodes[i].Time, PrefixStr + Suffix));
    }
    constexpr bool NoAddPercents = false;
    dumpHelper(Str, CumulativeMap, TotalTime, NoAddPercents);
  }
  Str << Name
      << " - Flat times:\n"
         "     Seconds   Pct CumPct  EventCnt TimerName\n";
  DumpMapType FlatMap;
  for (TimerIdT i = 0; i < LeafTimes.size(); ++i) {
    if (LeafCounts[i]) {
      snprintf(PrefixStr, llvm::array_lengthof(PrefixStr), "%9zu ",
               LeafCounts[i]);
      FlatMap.insert(std::make_pair(LeafTimes[i], PrefixStr + IDs[i]));
    }
  }
  constexpr bool AddPercents = true;
  dumpHelper(Str, FlatMap, TotalTime, AddPercents);
  Str << "Number of timer updates: " << StateChangeCount << "\n";
}

double TimerStack::timestamp() {
  // TODO: Implement in terms of std::chrono for C++11.
  return llvm::TimeRecord::getCurrentTime(false).getWallTime();
}

} // end of namespace Ice
