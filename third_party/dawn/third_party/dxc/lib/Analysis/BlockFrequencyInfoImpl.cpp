//===- BlockFrequencyImplInfo.cpp - Block Frequency Info Implementation ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Loops should be simplified before this analysis.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/BlockFrequencyInfoImpl.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/Support/raw_ostream.h"
#include <numeric>

using namespace llvm;
using namespace llvm::bfi_detail;

#define DEBUG_TYPE "block-freq"

ScaledNumber<uint64_t> BlockMass::toScaled() const {
  if (isFull())
    return ScaledNumber<uint64_t>(1, 0);
  return ScaledNumber<uint64_t>(getMass() + 1, -64);
}

void BlockMass::dump() const { print(dbgs()); }

static char getHexDigit(int N) {
  assert(N < 16);
  if (N < 10)
    return '0' + N;
  return 'a' + N - 10;
}
raw_ostream &BlockMass::print(raw_ostream &OS) const {
  for (int Digits = 0; Digits < 16; ++Digits)
    OS << getHexDigit(Mass >> (60 - Digits * 4) & 0xf);
  return OS;
}

namespace {

typedef BlockFrequencyInfoImplBase::BlockNode BlockNode;
typedef BlockFrequencyInfoImplBase::Distribution Distribution;
typedef BlockFrequencyInfoImplBase::Distribution::WeightList WeightList;
typedef BlockFrequencyInfoImplBase::Scaled64 Scaled64;
typedef BlockFrequencyInfoImplBase::LoopData LoopData;
typedef BlockFrequencyInfoImplBase::Weight Weight;
typedef BlockFrequencyInfoImplBase::FrequencyData FrequencyData;

/// \brief Dithering mass distributer.
///
/// This class splits up a single mass into portions by weight, dithering to
/// spread out error.  No mass is lost.  The dithering precision depends on the
/// precision of the product of \a BlockMass and \a BranchProbability.
///
/// The distribution algorithm follows.
///
///  1. Initialize by saving the sum of the weights in \a RemWeight and the
///     mass to distribute in \a RemMass.
///
///  2. For each portion:
///
///      1. Construct a branch probability, P, as the portion's weight divided
///         by the current value of \a RemWeight.
///      2. Calculate the portion's mass as \a RemMass times P.
///      3. Update \a RemWeight and \a RemMass at each portion by subtracting
///         the current portion's weight and mass.
struct DitheringDistributer {
  uint32_t RemWeight;
  BlockMass RemMass;

  DitheringDistributer(Distribution &Dist, const BlockMass &Mass);

  BlockMass takeMass(uint32_t Weight);
};

} // end namespace

DitheringDistributer::DitheringDistributer(Distribution &Dist,
                                           const BlockMass &Mass) {
  Dist.normalize();
  RemWeight = Dist.Total;
  RemMass = Mass;
}

BlockMass DitheringDistributer::takeMass(uint32_t Weight) {
  assert(Weight && "invalid weight");
  assert(Weight <= RemWeight);
  BlockMass Mass = RemMass * BranchProbability(Weight, RemWeight);

  // Decrement totals (dither).
  RemWeight -= Weight;
  RemMass -= Mass;
  return Mass;
}

void Distribution::add(const BlockNode &Node, uint64_t Amount,
                       Weight::DistType Type) {
  assert(Amount && "invalid weight of 0");
  uint64_t NewTotal = Total + Amount;

  // Check for overflow.  It should be impossible to overflow twice.
  bool IsOverflow = NewTotal < Total;
  assert(!(DidOverflow && IsOverflow) && "unexpected repeated overflow");
  DidOverflow |= IsOverflow;

  // Update the total.
  Total = NewTotal;

  // Save the weight.
  Weights.push_back(Weight(Type, Node, Amount));
}

static void combineWeight(Weight &W, const Weight &OtherW) {
  assert(OtherW.TargetNode.isValid());
  if (!W.Amount) {
    W = OtherW;
    return;
  }
  assert(W.Type == OtherW.Type);
  assert(W.TargetNode == OtherW.TargetNode);
  assert(OtherW.Amount && "Expected non-zero weight");
  if (W.Amount > W.Amount + OtherW.Amount)
    // Saturate on overflow.
    W.Amount = UINT64_MAX;
  else
    W.Amount += OtherW.Amount;
}
static void combineWeightsBySorting(WeightList &Weights) {
  // Sort so edges to the same node are adjacent.
  std::sort(Weights.begin(), Weights.end(),
            [](const Weight &L,
               const Weight &R) { return L.TargetNode < R.TargetNode; });

  // Combine adjacent edges.
  WeightList::iterator O = Weights.begin();
  for (WeightList::const_iterator I = O, L = O, E = Weights.end(); I != E;
       ++O, (I = L)) {
    *O = *I;

    // Find the adjacent weights to the same node.
    for (++L; L != E && I->TargetNode == L->TargetNode; ++L)
      combineWeight(*O, *L);
  }

  // Erase extra entries.
  Weights.erase(O, Weights.end());
  return;
}
static void combineWeightsByHashing(WeightList &Weights) {
  // Collect weights into a DenseMap.
  typedef DenseMap<BlockNode::IndexType, Weight> HashTable;
  HashTable Combined(NextPowerOf2(2 * Weights.size()));
  for (const Weight &W : Weights)
    combineWeight(Combined[W.TargetNode.Index], W);

  // Check whether anything changed.
  if (Weights.size() == Combined.size())
    return;

  // Fill in the new weights.
  Weights.clear();
  Weights.reserve(Combined.size());
  for (const auto &I : Combined)
    Weights.push_back(I.second);
}
static void combineWeights(WeightList &Weights) {
  // Use a hash table for many successors to keep this linear.
  if (Weights.size() > 128) {
    combineWeightsByHashing(Weights);
    return;
  }

  combineWeightsBySorting(Weights);
}
static uint64_t shiftRightAndRound(uint64_t N, int Shift) {
  assert(Shift >= 0);
  assert(Shift < 64);
  if (!Shift)
    return N;
  return (N >> Shift) + (UINT64_C(1) & N >> (Shift - 1));
}
void Distribution::normalize() {
  // Early exit for termination nodes.
  if (Weights.empty())
    return;

  // Only bother if there are multiple successors.
  if (Weights.size() > 1)
    combineWeights(Weights);

  // Early exit when combined into a single successor.
  if (Weights.size() == 1) {
    Total = 1;
    Weights.front().Amount = 1;
    return;
  }

  // Determine how much to shift right so that the total fits into 32-bits.
  //
  // If we shift at all, shift by 1 extra.  Otherwise, the lower limit of 1
  // for each weight can cause a 32-bit overflow.
  int Shift = 0;
  if (DidOverflow)
    Shift = 33;
  else if (Total > UINT32_MAX)
    Shift = 33 - countLeadingZeros(Total);

  // Early exit if nothing needs to be scaled.
  if (!Shift) {
    // If we didn't overflow then combineWeights() shouldn't have changed the
    // sum of the weights, but let's double-check.
    assert(Total == std::accumulate(Weights.begin(), Weights.end(), UINT64_C(0),
                                    [](uint64_t Sum, const Weight &W) {
                      return Sum + W.Amount;
                    }) &&
           "Expected total to be correct");
    return;
  }

  // Recompute the total through accumulation (rather than shifting it) so that
  // it's accurate after shifting and any changes combineWeights() made above.
  Total = 0;

  // Sum the weights to each node and shift right if necessary.
  for (Weight &W : Weights) {
    // Scale down below UINT32_MAX.  Since Shift is larger than necessary, we
    // can round here without concern about overflow.
    assert(W.TargetNode.isValid());
    W.Amount = std::max(UINT64_C(1), shiftRightAndRound(W.Amount, Shift));
    assert(W.Amount <= UINT32_MAX);

    // Update the total.
    Total += W.Amount;
  }
  assert(Total <= UINT32_MAX);
}

void BlockFrequencyInfoImplBase::clear() {
  // Swap with a default-constructed std::vector, since std::vector<>::clear()
  // does not actually clear heap storage.
  std::vector<FrequencyData>().swap(Freqs);
  std::vector<WorkingData>().swap(Working);
  Loops.clear();
}

/// \brief Clear all memory not needed downstream.
///
/// Releases all memory not used downstream.  In particular, saves Freqs.
static void cleanup(BlockFrequencyInfoImplBase &BFI) {
  std::vector<FrequencyData> SavedFreqs(std::move(BFI.Freqs));
  BFI.clear();
  BFI.Freqs = std::move(SavedFreqs);
}

bool BlockFrequencyInfoImplBase::addToDist(Distribution &Dist,
                                           const LoopData *OuterLoop,
                                           const BlockNode &Pred,
                                           const BlockNode &Succ,
                                           uint64_t Weight) {
  if (!Weight)
    Weight = 1;

  auto isLoopHeader = [&OuterLoop](const BlockNode &Node) {
    return OuterLoop && OuterLoop->isHeader(Node);
  };

  BlockNode Resolved = Working[Succ.Index].getResolvedNode();

#ifndef NDEBUG
  auto debugSuccessor = [&](const char *Type) {
    dbgs() << "  =>"
           << " [" << Type << "] weight = " << Weight;
    if (!isLoopHeader(Resolved))
      dbgs() << ", succ = " << getBlockName(Succ);
    if (Resolved != Succ)
      dbgs() << ", resolved = " << getBlockName(Resolved);
    dbgs() << "\n";
  };
  (void)debugSuccessor;
#endif

  if (isLoopHeader(Resolved)) {
    DEBUG(debugSuccessor("backedge"));
    Dist.addBackedge(Resolved, Weight);
    return true;
  }

  if (Working[Resolved.Index].getContainingLoop() != OuterLoop) {
    DEBUG(debugSuccessor("  exit  "));
    Dist.addExit(Resolved, Weight);
    return true;
  }

  if (Resolved < Pred) {
    if (!isLoopHeader(Pred)) {
      // If OuterLoop is an irreducible loop, we can't actually handle this.
      assert((!OuterLoop || !OuterLoop->isIrreducible()) &&
             "unhandled irreducible control flow");

      // Irreducible backedge.  Abort.
      DEBUG(debugSuccessor("abort!!!"));
      return false;
    }

    // If "Pred" is a loop header, then this isn't really a backedge; rather,
    // OuterLoop must be irreducible.  These false backedges can come only from
    // secondary loop headers.
    assert(OuterLoop && OuterLoop->isIrreducible() && !isLoopHeader(Resolved) &&
           "unhandled irreducible control flow");
  }

  DEBUG(debugSuccessor(" local  "));
  Dist.addLocal(Resolved, Weight);
  return true;
}

bool BlockFrequencyInfoImplBase::addLoopSuccessorsToDist(
    const LoopData *OuterLoop, LoopData &Loop, Distribution &Dist) {
  // Copy the exit map into Dist.
  for (const auto &I : Loop.Exits)
    if (!addToDist(Dist, OuterLoop, Loop.getHeader(), I.first,
                   I.second.getMass()))
      // Irreducible backedge.
      return false;

  return true;
}

/// \brief Compute the loop scale for a loop.
void BlockFrequencyInfoImplBase::computeLoopScale(LoopData &Loop) {
  // Compute loop scale.
  DEBUG(dbgs() << "compute-loop-scale: " << getLoopName(Loop) << "\n");

  // Infinite loops need special handling. If we give the back edge an infinite
  // mass, they may saturate all the other scales in the function down to 1,
  // making all the other region temperatures look exactly the same. Choose an
  // arbitrary scale to avoid these issues.
  //
  // FIXME: An alternate way would be to select a symbolic scale which is later
  // replaced to be the maximum of all computed scales plus 1. This would
  // appropriately describe the loop as having a large scale, without skewing
  // the final frequency computation.
  const Scaled64 InifiniteLoopScale(1, 12);

  // LoopScale == 1 / ExitMass
  // ExitMass == HeadMass - BackedgeMass
  BlockMass TotalBackedgeMass;
  for (auto &Mass : Loop.BackedgeMass)
    TotalBackedgeMass += Mass;
  BlockMass ExitMass = BlockMass::getFull() - TotalBackedgeMass;

  // Block scale stores the inverse of the scale. If this is an infinite loop,
  // its exit mass will be zero. In this case, use an arbitrary scale for the
  // loop scale.
  Loop.Scale =
      ExitMass.isEmpty() ? InifiniteLoopScale : ExitMass.toScaled().inverse();

  DEBUG(dbgs() << " - exit-mass = " << ExitMass << " (" << BlockMass::getFull()
               << " - " << TotalBackedgeMass << ")\n"
               << " - scale = " << Loop.Scale << "\n");
}

/// \brief Package up a loop.
void BlockFrequencyInfoImplBase::packageLoop(LoopData &Loop) {
  DEBUG(dbgs() << "packaging-loop: " << getLoopName(Loop) << "\n");

  // Clear the subloop exits to prevent quadratic memory usage.
  for (const BlockNode &M : Loop.Nodes) {
    if (auto *Loop = Working[M.Index].getPackagedLoop())
      Loop->Exits.clear();
    DEBUG(dbgs() << " - node: " << getBlockName(M.Index) << "\n");
  }
  Loop.IsPackaged = true;
}

#ifndef NDEBUG
static void debugAssign(const BlockFrequencyInfoImplBase &BFI,
                        const DitheringDistributer &D, const BlockNode &T,
                        const BlockMass &M, const char *Desc) {
  dbgs() << "  => assign " << M << " (" << D.RemMass << ")";
  if (Desc)
    dbgs() << " [" << Desc << "]";
  if (T.isValid())
    dbgs() << " to " << BFI.getBlockName(T);
  dbgs() << "\n";
}
#endif

void BlockFrequencyInfoImplBase::distributeMass(const BlockNode &Source,
                                                LoopData *OuterLoop,
                                                Distribution &Dist) {
  BlockMass Mass = Working[Source.Index].getMass();
  DEBUG(dbgs() << "  => mass:  " << Mass << "\n");

  // Distribute mass to successors as laid out in Dist.
  DitheringDistributer D(Dist, Mass);

  for (const Weight &W : Dist.Weights) {
    // Check for a local edge (non-backedge and non-exit).
    BlockMass Taken = D.takeMass(W.Amount);
    if (W.Type == Weight::Local) {
      Working[W.TargetNode.Index].getMass() += Taken;
      DEBUG(debugAssign(*this, D, W.TargetNode, Taken, nullptr));
      continue;
    }

    // Backedges and exits only make sense if we're processing a loop.
    assert(OuterLoop && "backedge or exit outside of loop");

    // Check for a backedge.
    if (W.Type == Weight::Backedge) {
      OuterLoop->BackedgeMass[OuterLoop->getHeaderIndex(W.TargetNode)] += Taken;
      DEBUG(debugAssign(*this, D, W.TargetNode, Taken, "back"));
      continue;
    }

    // This must be an exit.
    assert(W.Type == Weight::Exit);
    OuterLoop->Exits.push_back(std::make_pair(W.TargetNode, Taken));
    DEBUG(debugAssign(*this, D, W.TargetNode, Taken, "exit"));
  }
}

static void convertFloatingToInteger(BlockFrequencyInfoImplBase &BFI,
                                     const Scaled64 &Min, const Scaled64 &Max) {
  // Scale the Factor to a size that creates integers.  Ideally, integers would
  // be scaled so that Max == UINT64_MAX so that they can be best
  // differentiated.  However, in the presence of large frequency values, small
  // frequencies are scaled down to 1, making it impossible to differentiate
  // small, unequal numbers. When the spread between Min and Max frequencies
  // fits well within MaxBits, we make the scale be at least 8.
  const unsigned MaxBits = 64;
  const unsigned SpreadBits = (Max / Min).lg();
  Scaled64 ScalingFactor;
  if (SpreadBits <= MaxBits - 3) {
    // If the values are small enough, make the scaling factor at least 8 to
    // allow distinguishing small values.
    ScalingFactor = Min.inverse();
    ScalingFactor <<= 3;
  } else {
    // If the values need more than MaxBits to be represented, saturate small
    // frequency values down to 1 by using a scaling factor that benefits large
    // frequency values.
    ScalingFactor = Scaled64(1, MaxBits) / Max;
  }

  // Translate the floats to integers.
  DEBUG(dbgs() << "float-to-int: min = " << Min << ", max = " << Max
               << ", factor = " << ScalingFactor << "\n");
  for (size_t Index = 0; Index < BFI.Freqs.size(); ++Index) {
    Scaled64 Scaled = BFI.Freqs[Index].Scaled * ScalingFactor;
    BFI.Freqs[Index].Integer = std::max(UINT64_C(1), Scaled.toInt<uint64_t>());
    DEBUG(dbgs() << " - " << BFI.getBlockName(Index) << ": float = "
                 << BFI.Freqs[Index].Scaled << ", scaled = " << Scaled
                 << ", int = " << BFI.Freqs[Index].Integer << "\n");
  }
}

/// \brief Unwrap a loop package.
///
/// Visits all the members of a loop, adjusting their BlockData according to
/// the loop's pseudo-node.
static void unwrapLoop(BlockFrequencyInfoImplBase &BFI, LoopData &Loop) {
  DEBUG(dbgs() << "unwrap-loop-package: " << BFI.getLoopName(Loop)
               << ": mass = " << Loop.Mass << ", scale = " << Loop.Scale
               << "\n");
  Loop.Scale *= Loop.Mass.toScaled();
  Loop.IsPackaged = false;
  DEBUG(dbgs() << "  => combined-scale = " << Loop.Scale << "\n");

  // Propagate the head scale through the loop.  Since members are visited in
  // RPO, the head scale will be updated by the loop scale first, and then the
  // final head scale will be used for updated the rest of the members.
  for (const BlockNode &N : Loop.Nodes) {
    const auto &Working = BFI.Working[N.Index];
    Scaled64 &F = Working.isAPackage() ? Working.getPackagedLoop()->Scale
                                       : BFI.Freqs[N.Index].Scaled;
    Scaled64 New = Loop.Scale * F;
    DEBUG(dbgs() << " - " << BFI.getBlockName(N) << ": " << F << " => " << New
                 << "\n");
    F = New;
  }
}

void BlockFrequencyInfoImplBase::unwrapLoops() {
  // Set initial frequencies from loop-local masses.
  for (size_t Index = 0; Index < Working.size(); ++Index)
    Freqs[Index].Scaled = Working[Index].Mass.toScaled();

  for (LoopData &Loop : Loops)
    unwrapLoop(*this, Loop);
}

void BlockFrequencyInfoImplBase::finalizeMetrics() {
  // Unwrap loop packages in reverse post-order, tracking min and max
  // frequencies.
  auto Min = Scaled64::getLargest();
  auto Max = Scaled64::getZero();
  for (size_t Index = 0; Index < Working.size(); ++Index) {
    // Update min/max scale.
    Min = std::min(Min, Freqs[Index].Scaled);
    Max = std::max(Max, Freqs[Index].Scaled);
  }

  // Convert to integers.
  convertFloatingToInteger(*this, Min, Max);

  // Clean up data structures.
  cleanup(*this);

  // Print out the final stats.
  DEBUG(dump());
}

BlockFrequency
BlockFrequencyInfoImplBase::getBlockFreq(const BlockNode &Node) const {
  if (!Node.isValid())
    return 0;
  return Freqs[Node.Index].Integer;
}
Scaled64
BlockFrequencyInfoImplBase::getFloatingBlockFreq(const BlockNode &Node) const {
  if (!Node.isValid())
    return Scaled64::getZero();
  return Freqs[Node.Index].Scaled;
}

std::string
BlockFrequencyInfoImplBase::getBlockName(const BlockNode &Node) const {
  return std::string();
}
std::string
BlockFrequencyInfoImplBase::getLoopName(const LoopData &Loop) const {
  return getBlockName(Loop.getHeader()) + (Loop.isIrreducible() ? "**" : "*");
}

raw_ostream &
BlockFrequencyInfoImplBase::printBlockFreq(raw_ostream &OS,
                                           const BlockNode &Node) const {
  return OS << getFloatingBlockFreq(Node);
}

raw_ostream &
BlockFrequencyInfoImplBase::printBlockFreq(raw_ostream &OS,
                                           const BlockFrequency &Freq) const {
  Scaled64 Block(Freq.getFrequency(), 0);
  Scaled64 Entry(getEntryFreq(), 0);

  return OS << Block / Entry;
}

void IrreducibleGraph::addNodesInLoop(const BFIBase::LoopData &OuterLoop) {
  Start = OuterLoop.getHeader();
  Nodes.reserve(OuterLoop.Nodes.size());
  for (auto N : OuterLoop.Nodes)
    addNode(N);
  indexNodes();
}
void IrreducibleGraph::addNodesInFunction() {
  Start = 0;
  for (uint32_t Index = 0; Index < BFI.Working.size(); ++Index)
    if (!BFI.Working[Index].isPackaged())
      addNode(Index);
  indexNodes();
}
void IrreducibleGraph::indexNodes() {
  for (auto &I : Nodes)
    Lookup[I.Node.Index] = &I;
}
void IrreducibleGraph::addEdge(IrrNode &Irr, const BlockNode &Succ,
                               const BFIBase::LoopData *OuterLoop) {
  if (OuterLoop && OuterLoop->isHeader(Succ))
    return;
  auto L = Lookup.find(Succ.Index);
  if (L == Lookup.end())
    return;
  IrrNode &SuccIrr = *L->second;
  Irr.Edges.push_back(&SuccIrr);
  SuccIrr.Edges.push_front(&Irr);
  ++SuccIrr.NumIn;
}

namespace llvm {
template <> struct GraphTraits<IrreducibleGraph> {
  typedef bfi_detail::IrreducibleGraph GraphT;

  typedef const GraphT::IrrNode NodeType;
  typedef GraphT::IrrNode::iterator ChildIteratorType;

  static const NodeType *getEntryNode(const GraphT &G) {
    return G.StartIrr;
  }
  static ChildIteratorType child_begin(NodeType *N) { return N->succ_begin(); }
  static ChildIteratorType child_end(NodeType *N) { return N->succ_end(); }
};
}

/// \brief Find extra irreducible headers.
///
/// Find entry blocks and other blocks with backedges, which exist when \c G
/// contains irreducible sub-SCCs.
static void findIrreducibleHeaders(
    const BlockFrequencyInfoImplBase &BFI,
    const IrreducibleGraph &G,
    const std::vector<const IrreducibleGraph::IrrNode *> &SCC,
    LoopData::NodeList &Headers, LoopData::NodeList &Others) {
  // Map from nodes in the SCC to whether it's an entry block.
  SmallDenseMap<const IrreducibleGraph::IrrNode *, bool, 8> InSCC;

  // InSCC also acts the set of nodes in the graph.  Seed it.
  for (const auto *I : SCC)
    InSCC[I] = false;

  for (auto I = InSCC.begin(), E = InSCC.end(); I != E; ++I) {
    auto &Irr = *I->first;
    for (const auto *P : make_range(Irr.pred_begin(), Irr.pred_end())) {
      if (InSCC.count(P))
        continue;

      // This is an entry block.
      I->second = true;
      Headers.push_back(Irr.Node);
      DEBUG(dbgs() << "  => entry = " << BFI.getBlockName(Irr.Node) << "\n");
      break;
    }
  }
  assert(Headers.size() >= 2 &&
         "Expected irreducible CFG; -loop-info is likely invalid");
  if (Headers.size() == InSCC.size()) {
    // Every block is a header.
    std::sort(Headers.begin(), Headers.end());
    return;
  }

  // Look for extra headers from irreducible sub-SCCs.
  for (const auto &I : InSCC) {
    // Entry blocks are already headers.
    if (I.second)
      continue;

    auto &Irr = *I.first;
    for (const auto *P : make_range(Irr.pred_begin(), Irr.pred_end())) {
      // Skip forward edges.
      if (P->Node < Irr.Node)
        continue;

      // Skip predecessors from entry blocks.  These can have inverted
      // ordering.
      if (InSCC.lookup(P))
        continue;

      // Store the extra header.
      Headers.push_back(Irr.Node);
      DEBUG(dbgs() << "  => extra = " << BFI.getBlockName(Irr.Node) << "\n");
      break;
    }
    if (Headers.back() == Irr.Node)
      // Added this as a header.
      continue;

    // This is not a header.
    Others.push_back(Irr.Node);
    DEBUG(dbgs() << "  => other = " << BFI.getBlockName(Irr.Node) << "\n");
  }
  std::sort(Headers.begin(), Headers.end());
  std::sort(Others.begin(), Others.end());
}

static void createIrreducibleLoop(
    BlockFrequencyInfoImplBase &BFI, const IrreducibleGraph &G,
    LoopData *OuterLoop, std::list<LoopData>::iterator Insert,
    const std::vector<const IrreducibleGraph::IrrNode *> &SCC) {
  // Translate the SCC into RPO.
  DEBUG(dbgs() << " - found-scc\n");

  LoopData::NodeList Headers;
  LoopData::NodeList Others;
  findIrreducibleHeaders(BFI, G, SCC, Headers, Others);

  auto Loop = BFI.Loops.emplace(Insert, OuterLoop, Headers.begin(),
                                Headers.end(), Others.begin(), Others.end());

  // Update loop hierarchy.
  for (const auto &N : Loop->Nodes)
    if (BFI.Working[N.Index].isLoopHeader())
      BFI.Working[N.Index].Loop->Parent = &*Loop;
    else
      BFI.Working[N.Index].Loop = &*Loop;
}

iterator_range<std::list<LoopData>::iterator>
BlockFrequencyInfoImplBase::analyzeIrreducible(
    const IrreducibleGraph &G, LoopData *OuterLoop,
    std::list<LoopData>::iterator Insert) {
  assert((OuterLoop == nullptr) == (Insert == Loops.begin()));
  auto Prev = OuterLoop ? std::prev(Insert) : Loops.end();

  for (auto I = scc_begin(G); !I.isAtEnd(); ++I) {
    if (I->size() < 2)
      continue;

    // Translate the SCC into RPO.
    createIrreducibleLoop(*this, G, OuterLoop, Insert, *I);
  }

  if (OuterLoop)
    return make_range(std::next(Prev), Insert);
  return make_range(Loops.begin(), Insert);
}

void
BlockFrequencyInfoImplBase::updateLoopWithIrreducible(LoopData &OuterLoop) {
  OuterLoop.Exits.clear();
  for (auto &Mass : OuterLoop.BackedgeMass)
    Mass = BlockMass::getEmpty();
  auto O = OuterLoop.Nodes.begin() + 1;
  for (auto I = O, E = OuterLoop.Nodes.end(); I != E; ++I)
    if (!Working[I->Index].isPackaged())
      *O++ = *I;
  OuterLoop.Nodes.erase(O, OuterLoop.Nodes.end());
}

void BlockFrequencyInfoImplBase::adjustLoopHeaderMass(LoopData &Loop) {
  assert(Loop.isIrreducible() && "this only makes sense on irreducible loops");

  // Since the loop has more than one header block, the mass flowing back into
  // each header will be different. Adjust the mass in each header loop to
  // reflect the masses flowing through back edges.
  //
  // To do this, we distribute the initial mass using the backedge masses
  // as weights for the distribution.
  BlockMass LoopMass = BlockMass::getFull();
  Distribution Dist;

  DEBUG(dbgs() << "adjust-loop-header-mass:\n");
  for (uint32_t H = 0; H < Loop.NumHeaders; ++H) {
    auto &HeaderNode = Loop.Nodes[H];
    auto &BackedgeMass = Loop.BackedgeMass[Loop.getHeaderIndex(HeaderNode)];
    DEBUG(dbgs() << " - Add back edge mass for node "
                 << getBlockName(HeaderNode) << ": " << BackedgeMass << "\n");
    Dist.addLocal(HeaderNode, BackedgeMass.getMass());
  }

  DitheringDistributer D(Dist, LoopMass);

  DEBUG(dbgs() << " Distribute loop mass " << LoopMass
               << " to headers using above weights\n");
  for (const Weight &W : Dist.Weights) {
    BlockMass Taken = D.takeMass(W.Amount);
    assert(W.Type == Weight::Local && "all weights should be local");
    Working[W.TargetNode.Index].getMass() = Taken;
    DEBUG(debugAssign(*this, D, W.TargetNode, Taken, nullptr));
  }
}
