//===- LoopStrengthReduce.cpp - Strength Reduce IVs in Loops --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This transformation analyzes and transforms the induction variables (and
// computations derived from them) into forms suitable for efficient execution
// on the target.
//
// This pass performs a strength reduction on array references inside loops that
// have as one or more of their components the loop induction variable, it
// rewrites expressions to take advantage of scaled-index addressing modes
// available on the target, and it performs a variety of other optimizations
// related to loop induction variables.
//
// Terminology note: this code has a lot of handling for "post-increment" or
// "post-inc" users. This is not talking about post-increment addressing modes;
// it is instead talking about code like this:
//
//   %i = phi [ 0, %entry ], [ %i.next, %latch ]
//   ...
//   %i.next = add %i, 1
//   %c = icmp eq %i.next, %n
//
// The SCEV for %i is {0,+,1}<%L>. The SCEV for %i.next is {1,+,1}<%L>, however
// it's useful to think about these as the same register, with some uses using
// the value of the register before the add and some using it after. In this
// example, the icmp is a post-increment user, since it uses %i.next, which is
// the value of the induction variable after the increment. The other common
// case of post-increment users is users outside the loop.
//
// TODO: More sophistication in the way Formulae are generated and filtered.
//
// TODO: Handle multiple loops at a time.
//
// TODO: Should the addressing mode BaseGV be changed to a ConstantExpr instead
//       of a GlobalValue?
//
// TODO: When truncation is free, truncate ICmp users' operands to make it a
//       smaller encoding (on x86 at least).
//
// TODO: When a negated register is used by an add (such as in a list of
//       multiple base registers, or as the increment expression in an addrec),
//       we may not actually need both reg and (-1 * reg) in registers; the
//       negation can be implemented by using a sub instead of an add. The
//       lack of support for taking this into consideration when making
//       register pressure decisions is partly worked around by the "Special"
//       use kind.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/Analysis/IVUsers.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include <algorithm>
#include <memory> // HLSL Change
using namespace llvm;

#define DEBUG_TYPE "loop-reduce"

/// MaxIVUsers is an arbitrary threshold that provides an early opportunitiy for
/// bail out. This threshold is far beyond the number of users that LSR can
/// conceivably solve, so it should not affect generated code, but catches the
/// worst cases before LSR burns too much compile time and stack space.
static const unsigned MaxIVUsers = 200;

// Temporary flag to cleanup congruent phis after LSR phi expansion.
// It's currently disabled until we can determine whether it's truly useful or
// not. The flag should be removed after the v3.0 release.
// This is now needed for ivchains.
static cl::opt<bool> EnablePhiElim(
  "enable-lsr-phielim", cl::Hidden, cl::init(true),
  cl::desc("Enable LSR phi elimination"));

#ifndef NDEBUG
// Stress test IV chain generation.
static cl::opt<bool> StressIVChain(
  "stress-ivchain", cl::Hidden, cl::init(false),
  cl::desc("Stress test LSR IV chains"));
#else
static bool StressIVChain = false;
#endif

namespace {

/// RegSortData - This class holds data which is used to order reuse candidates.
class RegSortData {
public:
  /// UsedByIndices - This represents the set of LSRUse indices which reference
  /// a particular register.
  SmallBitVector UsedByIndices;

  void print(raw_ostream &OS) const;
  void dump() const;
};

}

void RegSortData::print(raw_ostream &OS) const {
  OS << "[NumUses=" << UsedByIndices.count() << ']';
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void RegSortData::dump() const {
  print(errs()); errs() << '\n';
}
#endif

namespace {

/// RegUseTracker - Map register candidates to information about how they are
/// used.
class RegUseTracker {
  typedef DenseMap<const SCEV *, RegSortData> RegUsesTy;

  RegUsesTy RegUsesMap;
  SmallVector<const SCEV *, 16> RegSequence;

public:
  void CountRegister(const SCEV *Reg, size_t LUIdx);
  void DropRegister(const SCEV *Reg, size_t LUIdx);
  void SwapAndDropUse(size_t LUIdx, size_t LastLUIdx);

  bool isRegUsedByUsesOtherThan(const SCEV *Reg, size_t LUIdx) const;

  const SmallBitVector &getUsedByIndices(const SCEV *Reg) const;

  void clear();

  typedef SmallVectorImpl<const SCEV *>::iterator iterator;
  typedef SmallVectorImpl<const SCEV *>::const_iterator const_iterator;
  iterator begin() { return RegSequence.begin(); }
  iterator end()   { return RegSequence.end(); }
  const_iterator begin() const { return RegSequence.begin(); }
  const_iterator end() const   { return RegSequence.end(); }
};

}

void
RegUseTracker::CountRegister(const SCEV *Reg, size_t LUIdx) {
  std::pair<RegUsesTy::iterator, bool> Pair =
    RegUsesMap.insert(std::make_pair(Reg, RegSortData()));
  RegSortData &RSD = Pair.first->second;
  if (Pair.second)
    RegSequence.push_back(Reg);
  RSD.UsedByIndices.resize(std::max(RSD.UsedByIndices.size(), LUIdx + 1));
  RSD.UsedByIndices.set(LUIdx);
}

void
RegUseTracker::DropRegister(const SCEV *Reg, size_t LUIdx) {
  RegUsesTy::iterator It = RegUsesMap.find(Reg);
  assert(It != RegUsesMap.end());
  RegSortData &RSD = It->second;
  assert(RSD.UsedByIndices.size() > LUIdx);
  RSD.UsedByIndices.reset(LUIdx);
}

void
RegUseTracker::SwapAndDropUse(size_t LUIdx, size_t LastLUIdx) {
  assert(LUIdx <= LastLUIdx);

  // Update RegUses. The data structure is not optimized for this purpose;
  // we must iterate through it and update each of the bit vectors.
  for (auto &Pair : RegUsesMap) {
    SmallBitVector &UsedByIndices = Pair.second.UsedByIndices;
    if (LUIdx < UsedByIndices.size())
      UsedByIndices[LUIdx] =
        LastLUIdx < UsedByIndices.size() ? UsedByIndices[LastLUIdx] : 0;
    UsedByIndices.resize(std::min(UsedByIndices.size(), LastLUIdx));
  }
}

bool
RegUseTracker::isRegUsedByUsesOtherThan(const SCEV *Reg, size_t LUIdx) const {
  RegUsesTy::const_iterator I = RegUsesMap.find(Reg);
  if (I == RegUsesMap.end())
    return false;
  const SmallBitVector &UsedByIndices = I->second.UsedByIndices;
  int i = UsedByIndices.find_first();
  if (i == -1) return false;
  if ((size_t)i != LUIdx) return true;
  return UsedByIndices.find_next(i) != -1;
}

const SmallBitVector &RegUseTracker::getUsedByIndices(const SCEV *Reg) const {
  RegUsesTy::const_iterator I = RegUsesMap.find(Reg);
  assert(I != RegUsesMap.end() && "Unknown register!");
  return I->second.UsedByIndices;
}

void RegUseTracker::clear() {
  RegUsesMap.clear();
  RegSequence.clear();
}

namespace {

/// Formula - This class holds information that describes a formula for
/// computing satisfying a use. It may include broken-out immediates and scaled
/// registers.
struct Formula {
  /// Global base address used for complex addressing.
  GlobalValue *BaseGV;

  /// Base offset for complex addressing.
  int64_t BaseOffset;

  /// Whether any complex addressing has a base register.
  bool HasBaseReg;

  /// The scale of any complex addressing.
  int64_t Scale;

  /// BaseRegs - The list of "base" registers for this use. When this is
  /// non-empty. The canonical representation of a formula is
  /// 1. BaseRegs.size > 1 implies ScaledReg != NULL and
  /// 2. ScaledReg != NULL implies Scale != 1 || !BaseRegs.empty().
  /// #1 enforces that the scaled register is always used when at least two
  /// registers are needed by the formula: e.g., reg1 + reg2 is reg1 + 1 * reg2.
  /// #2 enforces that 1 * reg is reg.
  /// This invariant can be temporarly broken while building a formula.
  /// However, every formula inserted into the LSRInstance must be in canonical
  /// form.
  SmallVector<const SCEV *, 4> BaseRegs;

  /// ScaledReg - The 'scaled' register for this use. This should be non-null
  /// when Scale is not zero.
  const SCEV *ScaledReg;

  /// UnfoldedOffset - An additional constant offset which added near the
  /// use. This requires a temporary register, but the offset itself can
  /// live in an add immediate field rather than a register.
  int64_t UnfoldedOffset;

  Formula()
      : BaseGV(nullptr), BaseOffset(0), HasBaseReg(false), Scale(0),
        ScaledReg(nullptr), UnfoldedOffset(0) {}

  void InitialMatch(const SCEV *S, Loop *L, ScalarEvolution &SE);

  bool isCanonical() const;

  void Canonicalize();

  bool Unscale();

  size_t getNumRegs() const;
  Type *getType() const;

  void DeleteBaseReg(const SCEV *&S);

  bool referencesReg(const SCEV *S) const;
  bool hasRegsUsedByUsesOtherThan(size_t LUIdx,
                                  const RegUseTracker &RegUses) const;

  void print(raw_ostream &OS) const;
  void dump() const;
};

}

/// DoInitialMatch - Recursion helper for InitialMatch.
static void DoInitialMatch(const SCEV *S, Loop *L,
                           SmallVectorImpl<const SCEV *> &Good,
                           SmallVectorImpl<const SCEV *> &Bad,
                           ScalarEvolution &SE) {
  // Collect expressions which properly dominate the loop header.
  if (SE.properlyDominates(S, L->getHeader())) {
    Good.push_back(S);
    return;
  }

  // Look at add operands.
  if (const SCEVAddExpr *Add = dyn_cast<SCEVAddExpr>(S)) {
    for (const SCEV *S : Add->operands())
      DoInitialMatch(S, L, Good, Bad, SE);
    return;
  }

  // Look at addrec operands.
  if (const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(S))
    if (!AR->getStart()->isZero()) {
      DoInitialMatch(AR->getStart(), L, Good, Bad, SE);
      DoInitialMatch(SE.getAddRecExpr(SE.getConstant(AR->getType(), 0),
                                      AR->getStepRecurrence(SE),
                                      // FIXME: AR->getNoWrapFlags()
                                      AR->getLoop(), SCEV::FlagAnyWrap),
                     L, Good, Bad, SE);
      return;
    }

  // Handle a multiplication by -1 (negation) if it didn't fold.
  if (const SCEVMulExpr *Mul = dyn_cast<SCEVMulExpr>(S))
    if (Mul->getOperand(0)->isAllOnesValue()) {
      SmallVector<const SCEV *, 4> Ops(Mul->op_begin()+1, Mul->op_end());
      const SCEV *NewMul = SE.getMulExpr(Ops);

      SmallVector<const SCEV *, 4> MyGood;
      SmallVector<const SCEV *, 4> MyBad;
      DoInitialMatch(NewMul, L, MyGood, MyBad, SE);
      const SCEV *NegOne = SE.getSCEV(ConstantInt::getAllOnesValue(
        SE.getEffectiveSCEVType(NewMul->getType())));
      for (const SCEV *S : MyGood)
        Good.push_back(SE.getMulExpr(NegOne, S));
      for (const SCEV *S : MyBad)
        Bad.push_back(SE.getMulExpr(NegOne, S));
      return;
    }

  // Ok, we can't do anything interesting. Just stuff the whole thing into a
  // register and hope for the best.
  Bad.push_back(S);
}

/// InitialMatch - Incorporate loop-variant parts of S into this Formula,
/// attempting to keep all loop-invariant and loop-computable values in a
/// single base register.
void Formula::InitialMatch(const SCEV *S, Loop *L, ScalarEvolution &SE) {
  SmallVector<const SCEV *, 4> Good;
  SmallVector<const SCEV *, 4> Bad;
  DoInitialMatch(S, L, Good, Bad, SE);
  if (!Good.empty()) {
    const SCEV *Sum = SE.getAddExpr(Good);
    if (!Sum->isZero())
      BaseRegs.push_back(Sum);
    HasBaseReg = true;
  }
  if (!Bad.empty()) {
    const SCEV *Sum = SE.getAddExpr(Bad);
    if (!Sum->isZero())
      BaseRegs.push_back(Sum);
    HasBaseReg = true;
  }
  Canonicalize();
}

/// \brief Check whether or not this formula statisfies the canonical
/// representation.
/// \see Formula::BaseRegs.
bool Formula::isCanonical() const {
  if (ScaledReg)
    return Scale != 1 || !BaseRegs.empty();
  return BaseRegs.size() <= 1;
}

/// \brief Helper method to morph a formula into its canonical representation.
/// \see Formula::BaseRegs.
/// Every formula having more than one base register, must use the ScaledReg
/// field. Otherwise, we would have to do special cases everywhere in LSR
/// to treat reg1 + reg2 + ... the same way as reg1 + 1*reg2 + ...
/// On the other hand, 1*reg should be canonicalized into reg.
void Formula::Canonicalize() {
  if (isCanonical())
    return;
  // So far we did not need this case. This is easy to implement but it is
  // useless to maintain dead code. Beside it could hurt compile time.
  assert(!BaseRegs.empty() && "1*reg => reg, should not be needed.");
  // Keep the invariant sum in BaseRegs and one of the variant sum in ScaledReg.
  ScaledReg = BaseRegs.back();
  BaseRegs.pop_back();
  Scale = 1;
  size_t BaseRegsSize = BaseRegs.size();
  size_t Try = 0;
  // If ScaledReg is an invariant, try to find a variant expression.
  while (Try < BaseRegsSize && !isa<SCEVAddRecExpr>(ScaledReg))
    std::swap(ScaledReg, BaseRegs[Try++]);
}

/// \brief Get rid of the scale in the formula.
/// In other words, this method morphes reg1 + 1*reg2 into reg1 + reg2.
/// \return true if it was possible to get rid of the scale, false otherwise.
/// \note After this operation the formula may not be in the canonical form.
bool Formula::Unscale() {
  if (Scale != 1)
    return false;
  Scale = 0;
  BaseRegs.push_back(ScaledReg);
  ScaledReg = nullptr;
  return true;
}

/// getNumRegs - Return the total number of register operands used by this
/// formula. This does not include register uses implied by non-constant
/// addrec strides.
size_t Formula::getNumRegs() const {
  return !!ScaledReg + BaseRegs.size();
}

/// getType - Return the type of this formula, if it has one, or null
/// otherwise. This type is meaningless except for the bit size.
Type *Formula::getType() const {
  return !BaseRegs.empty() ? BaseRegs.front()->getType() :
         ScaledReg ? ScaledReg->getType() :
         BaseGV ? BaseGV->getType() :
         nullptr;
}

/// DeleteBaseReg - Delete the given base reg from the BaseRegs list.
void Formula::DeleteBaseReg(const SCEV *&S) {
  if (&S != &BaseRegs.back())
    std::swap(S, BaseRegs.back());
  BaseRegs.pop_back();
}

/// referencesReg - Test if this formula references the given register.
bool Formula::referencesReg(const SCEV *S) const {
  return S == ScaledReg ||
         std::find(BaseRegs.begin(), BaseRegs.end(), S) != BaseRegs.end();
}

/// hasRegsUsedByUsesOtherThan - Test whether this formula uses registers
/// which are used by uses other than the use with the given index.
bool Formula::hasRegsUsedByUsesOtherThan(size_t LUIdx,
                                         const RegUseTracker &RegUses) const {
  if (ScaledReg)
    if (RegUses.isRegUsedByUsesOtherThan(ScaledReg, LUIdx))
      return true;
  for (const SCEV *BaseReg : BaseRegs)
    if (RegUses.isRegUsedByUsesOtherThan(BaseReg, LUIdx))
      return true;
  return false;
}

void Formula::print(raw_ostream &OS) const {
  bool First = true;
  if (BaseGV) {
    if (!First) OS << " + "; else First = false;
    BaseGV->printAsOperand(OS, /*PrintType=*/false);
  }
  if (BaseOffset != 0) {
    if (!First) OS << " + "; else First = false;
    OS << BaseOffset;
  }
  for (const SCEV *BaseReg : BaseRegs) {
    if (!First) OS << " + "; else First = false;
    OS << "reg(" << *BaseReg << ')';
  }
  if (HasBaseReg && BaseRegs.empty()) {
    if (!First) OS << " + "; else First = false;
    OS << "**error: HasBaseReg**";
  } else if (!HasBaseReg && !BaseRegs.empty()) {
    if (!First) OS << " + "; else First = false;
    OS << "**error: !HasBaseReg**";
  }
  if (Scale != 0) {
    if (!First) OS << " + "; else First = false;
    OS << Scale << "*reg(";
    if (ScaledReg)
      OS << *ScaledReg;
    else
      OS << "<unknown>";
    OS << ')';
  }
  if (UnfoldedOffset != 0) {
    if (!First) OS << " + ";
    OS << "imm(" << UnfoldedOffset << ')';
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void Formula::dump() const {
  print(errs()); errs() << '\n';
}
#endif

/// isAddRecSExtable - Return true if the given addrec can be sign-extended
/// without changing its value.
static bool isAddRecSExtable(const SCEVAddRecExpr *AR, ScalarEvolution &SE) {
  Type *WideTy =
    IntegerType::get(SE.getContext(), SE.getTypeSizeInBits(AR->getType()) + 1);
  return isa<SCEVAddRecExpr>(SE.getSignExtendExpr(AR, WideTy));
}

/// isAddSExtable - Return true if the given add can be sign-extended
/// without changing its value.
static bool isAddSExtable(const SCEVAddExpr *A, ScalarEvolution &SE) {
  Type *WideTy =
    IntegerType::get(SE.getContext(), SE.getTypeSizeInBits(A->getType()) + 1);
  return isa<SCEVAddExpr>(SE.getSignExtendExpr(A, WideTy));
}

/// isMulSExtable - Return true if the given mul can be sign-extended
/// without changing its value.
static bool isMulSExtable(const SCEVMulExpr *M, ScalarEvolution &SE) {
  Type *WideTy =
    IntegerType::get(SE.getContext(),
                     SE.getTypeSizeInBits(M->getType()) * M->getNumOperands());
  return isa<SCEVMulExpr>(SE.getSignExtendExpr(M, WideTy));
}

/// getExactSDiv - Return an expression for LHS /s RHS, if it can be determined
/// and if the remainder is known to be zero,  or null otherwise. If
/// IgnoreSignificantBits is true, expressions like (X * Y) /s Y are simplified
/// to Y, ignoring that the multiplication may overflow, which is useful when
/// the result will be used in a context where the most significant bits are
/// ignored.
static const SCEV *getExactSDiv(const SCEV *LHS, const SCEV *RHS,
                                ScalarEvolution &SE,
                                bool IgnoreSignificantBits = false) {
  // Handle the trivial case, which works for any SCEV type.
  if (LHS == RHS)
    return SE.getConstant(LHS->getType(), 1);

  // Handle a few RHS special cases.
  const SCEVConstant *RC = dyn_cast<SCEVConstant>(RHS);
  if (RC) {
    const APInt &RA = RC->getValue()->getValue();
    // Handle x /s -1 as x * -1, to give ScalarEvolution a chance to do
    // some folding.
    if (RA.isAllOnesValue())
      return SE.getMulExpr(LHS, RC);
    // Handle x /s 1 as x.
    if (RA == 1)
      return LHS;
  }

  // Check for a division of a constant by a constant.
  if (const SCEVConstant *C = dyn_cast<SCEVConstant>(LHS)) {
    if (!RC)
      return nullptr;
    const APInt &LA = C->getValue()->getValue();
    const APInt &RA = RC->getValue()->getValue();
    if (LA.srem(RA) != 0)
      return nullptr;
    return SE.getConstant(LA.sdiv(RA));
  }

  // Distribute the sdiv over addrec operands, if the addrec doesn't overflow.
  if (const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(LHS)) {
    if (IgnoreSignificantBits || isAddRecSExtable(AR, SE)) {
      const SCEV *Step = getExactSDiv(AR->getStepRecurrence(SE), RHS, SE,
                                      IgnoreSignificantBits);
      if (!Step) return nullptr;
      const SCEV *Start = getExactSDiv(AR->getStart(), RHS, SE,
                                       IgnoreSignificantBits);
      if (!Start) return nullptr;
      // FlagNW is independent of the start value, step direction, and is
      // preserved with smaller magnitude steps.
      // FIXME: AR->getNoWrapFlags(SCEV::FlagNW)
      return SE.getAddRecExpr(Start, Step, AR->getLoop(), SCEV::FlagAnyWrap);
    }
    return nullptr;
  }

  // Distribute the sdiv over add operands, if the add doesn't overflow.
  if (const SCEVAddExpr *Add = dyn_cast<SCEVAddExpr>(LHS)) {
    if (IgnoreSignificantBits || isAddSExtable(Add, SE)) {
      SmallVector<const SCEV *, 8> Ops;
      for (const SCEV *S : Add->operands()) {
        const SCEV *Op = getExactSDiv(S, RHS, SE, IgnoreSignificantBits);
        if (!Op) return nullptr;
        Ops.push_back(Op);
      }
      return SE.getAddExpr(Ops);
    }
    return nullptr;
  }

  // Check for a multiply operand that we can pull RHS out of.
  if (const SCEVMulExpr *Mul = dyn_cast<SCEVMulExpr>(LHS)) {
    if (IgnoreSignificantBits || isMulSExtable(Mul, SE)) {
      SmallVector<const SCEV *, 4> Ops;
      bool Found = false;
      for (const SCEV *S : Mul->operands()) {
        if (!Found)
          if (const SCEV *Q = getExactSDiv(S, RHS, SE,
                                           IgnoreSignificantBits)) {
            S = Q;
            Found = true;
          }
        Ops.push_back(S);
      }
      return Found ? SE.getMulExpr(Ops) : nullptr;
    }
    return nullptr;
  }

  // Otherwise we don't know.
  return nullptr;
}

/// ExtractImmediate - If S involves the addition of a constant integer value,
/// return that integer value, and mutate S to point to a new SCEV with that
/// value excluded.
static int64_t ExtractImmediate(const SCEV *&S, ScalarEvolution &SE) {
  if (const SCEVConstant *C = dyn_cast<SCEVConstant>(S)) {
    if (C->getValue()->getValue().getMinSignedBits() <= 64) {
      S = SE.getConstant(C->getType(), 0);
      return C->getValue()->getSExtValue();
    }
  } else if (const SCEVAddExpr *Add = dyn_cast<SCEVAddExpr>(S)) {
    SmallVector<const SCEV *, 8> NewOps(Add->op_begin(), Add->op_end());
    int64_t Result = ExtractImmediate(NewOps.front(), SE);
    if (Result != 0)
      S = SE.getAddExpr(NewOps);
    return Result;
  } else if (const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(S)) {
    SmallVector<const SCEV *, 8> NewOps(AR->op_begin(), AR->op_end());
    int64_t Result = ExtractImmediate(NewOps.front(), SE);
    if (Result != 0)
      S = SE.getAddRecExpr(NewOps, AR->getLoop(),
                           // FIXME: AR->getNoWrapFlags(SCEV::FlagNW)
                           SCEV::FlagAnyWrap);
    return Result;
  }
  return 0;
}

/// ExtractSymbol - If S involves the addition of a GlobalValue address,
/// return that symbol, and mutate S to point to a new SCEV with that
/// value excluded.
static GlobalValue *ExtractSymbol(const SCEV *&S, ScalarEvolution &SE) {
  if (const SCEVUnknown *U = dyn_cast<SCEVUnknown>(S)) {
    if (GlobalValue *GV = dyn_cast<GlobalValue>(U->getValue())) {
      S = SE.getConstant(GV->getType(), 0);
      return GV;
    }
  } else if (const SCEVAddExpr *Add = dyn_cast<SCEVAddExpr>(S)) {
    SmallVector<const SCEV *, 8> NewOps(Add->op_begin(), Add->op_end());
    GlobalValue *Result = ExtractSymbol(NewOps.back(), SE);
    if (Result)
      S = SE.getAddExpr(NewOps);
    return Result;
  } else if (const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(S)) {
    SmallVector<const SCEV *, 8> NewOps(AR->op_begin(), AR->op_end());
    GlobalValue *Result = ExtractSymbol(NewOps.front(), SE);
    if (Result)
      S = SE.getAddRecExpr(NewOps, AR->getLoop(),
                           // FIXME: AR->getNoWrapFlags(SCEV::FlagNW)
                           SCEV::FlagAnyWrap);
    return Result;
  }
  return nullptr;
}

/// isAddressUse - Returns true if the specified instruction is using the
/// specified value as an address.
static bool isAddressUse(Instruction *Inst, Value *OperandVal) {
  bool isAddress = isa<LoadInst>(Inst);
  if (StoreInst *SI = dyn_cast<StoreInst>(Inst)) {
    if (SI->getOperand(1) == OperandVal)
      isAddress = true;
  } else if (IntrinsicInst *II = dyn_cast<IntrinsicInst>(Inst)) {
    // Addressing modes can also be folded into prefetches and a variety
    // of intrinsics.
    switch (II->getIntrinsicID()) {
      default: break;
      case Intrinsic::prefetch:
#if 0 // HLSL Change - remove platform intrinsics
      case Intrinsic::x86_sse_storeu_ps:
      case Intrinsic::x86_sse2_storeu_pd:
      case Intrinsic::x86_sse2_storeu_dq:
      case Intrinsic::x86_sse2_storel_dq:
#endif // HLSL Change - remove platform intrinsics
        if (II->getArgOperand(0) == OperandVal)
          isAddress = true;
        break;
    }
  }
  return isAddress;
}

/// getAccessType - Return the type of the memory being accessed.
static Type *getAccessType(const Instruction *Inst) {
  Type *AccessTy = Inst->getType();
  if (const StoreInst *SI = dyn_cast<StoreInst>(Inst)) {
    AccessTy = SI->getOperand(0)->getType();
#if 0 // HLSL Change - remove platform intrinsics
  } else if (const IntrinsicInst *II = dyn_cast<IntrinsicInst>(Inst)) {
    // Addressing modes can also be folded into prefetches and a variety
    // of intrinsics.
    switch (II->getIntrinsicID()) {
    default: break;
    case Intrinsic::x86_sse_storeu_ps:
    case Intrinsic::x86_sse2_storeu_pd:
    case Intrinsic::x86_sse2_storeu_dq:
    case Intrinsic::x86_sse2_storel_dq:
      AccessTy = II->getArgOperand(0)->getType();
      break;
    }
#endif // HLSL Change - remove platform intrinsics
  }

  // All pointers have the same requirements, so canonicalize them to an
  // arbitrary pointer type to minimize variation.
  if (PointerType *PTy = dyn_cast<PointerType>(AccessTy))
    AccessTy = PointerType::get(IntegerType::get(PTy->getContext(), 1),
                                PTy->getAddressSpace());

  return AccessTy;
}

/// isExistingPhi - Return true if this AddRec is already a phi in its loop.
static bool isExistingPhi(const SCEVAddRecExpr *AR, ScalarEvolution &SE) {
  for (BasicBlock::iterator I = AR->getLoop()->getHeader()->begin();
       PHINode *PN = dyn_cast<PHINode>(I); ++I) {
    if (SE.isSCEVable(PN->getType()) &&
        (SE.getEffectiveSCEVType(PN->getType()) ==
         SE.getEffectiveSCEVType(AR->getType())) &&
        SE.getSCEV(PN) == AR)
      return true;
  }
  return false;
}

/// Check if expanding this expression is likely to incur significant cost. This
/// is tricky because SCEV doesn't track which expressions are actually computed
/// by the current IR.
///
/// We currently allow expansion of IV increments that involve adds,
/// multiplication by constants, and AddRecs from existing phis.
///
/// TODO: Allow UDivExpr if we can find an existing IV increment that is an
/// obvious multiple of the UDivExpr.
static bool isHighCostExpansion(const SCEV *S,
                                SmallPtrSetImpl<const SCEV*> &Processed,
                                ScalarEvolution &SE) {
  // Zero/One operand expressions
  switch (S->getSCEVType()) {
  case scUnknown:
  case scConstant:
    return false;
  case scTruncate:
    return isHighCostExpansion(cast<SCEVTruncateExpr>(S)->getOperand(),
                               Processed, SE);
  case scZeroExtend:
    return isHighCostExpansion(cast<SCEVZeroExtendExpr>(S)->getOperand(),
                               Processed, SE);
  case scSignExtend:
    return isHighCostExpansion(cast<SCEVSignExtendExpr>(S)->getOperand(),
                               Processed, SE);
  }

  if (!Processed.insert(S).second)
    return false;

  if (const SCEVAddExpr *Add = dyn_cast<SCEVAddExpr>(S)) {
    for (const SCEV *S : Add->operands()) {
      if (isHighCostExpansion(S, Processed, SE))
        return true;
    }
    return false;
  }

  if (const SCEVMulExpr *Mul = dyn_cast<SCEVMulExpr>(S)) {
    if (Mul->getNumOperands() == 2) {
      // Multiplication by a constant is ok
      if (isa<SCEVConstant>(Mul->getOperand(0)))
        return isHighCostExpansion(Mul->getOperand(1), Processed, SE);

      // If we have the value of one operand, check if an existing
      // multiplication already generates this expression.
      if (const SCEVUnknown *U = dyn_cast<SCEVUnknown>(Mul->getOperand(1))) {
        Value *UVal = U->getValue();
        for (User *UR : UVal->users()) {
          // If U is a constant, it may be used by a ConstantExpr.
          Instruction *UI = dyn_cast<Instruction>(UR);
          if (UI && UI->getOpcode() == Instruction::Mul &&
              SE.isSCEVable(UI->getType())) {
            return SE.getSCEV(UI) == Mul;
          }
        }
      }
    }
  }

  if (const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(S)) {
    if (isExistingPhi(AR, SE))
      return false;
  }

  // Fow now, consider any other type of expression (div/mul/min/max) high cost.
  return true;
}

/// DeleteTriviallyDeadInstructions - If any of the instructions is the
/// specified set are trivially dead, delete them and see if this makes any of
/// their operands subsequently dead.
static bool
DeleteTriviallyDeadInstructions(SmallVectorImpl<WeakTrackingVH> &DeadInsts) {
  bool Changed = false;

  while (!DeadInsts.empty()) {
    Value *V = DeadInsts.pop_back_val();
    Instruction *I = dyn_cast_or_null<Instruction>(V);

    if (!I || !isInstructionTriviallyDead(I))
      continue;

    for (Use &O : I->operands())
      if (Instruction *U = dyn_cast<Instruction>(O)) {
        O = nullptr;
        if (U->use_empty())
          DeadInsts.emplace_back(U);
      }

    I->eraseFromParent();
    Changed = true;
  }

  return Changed;
}

namespace {
class LSRUse;
}

/// \brief Check if the addressing mode defined by \p F is completely
/// folded in \p LU at isel time.
/// This includes address-mode folding and special icmp tricks.
/// This function returns true if \p LU can accommodate what \p F
/// defines and up to 1 base + 1 scaled + offset.
/// In other words, if \p F has several base registers, this function may
/// still return true. Therefore, users still need to account for
/// additional base registers and/or unfolded offsets to derive an
/// accurate cost model.
static bool isAMCompletelyFolded(const TargetTransformInfo &TTI,
                                 const LSRUse &LU, const Formula &F);
// Get the cost of the scaling factor used in F for LU.
static unsigned getScalingFactorCost(const TargetTransformInfo &TTI,
                                     const LSRUse &LU, const Formula &F);

namespace {

/// Cost - This class is used to measure and compare candidate formulae.
class Cost {
  /// TODO: Some of these could be merged. Also, a lexical ordering
  /// isn't always optimal.
  unsigned NumRegs;
  unsigned AddRecCost;
  unsigned NumIVMuls;
  unsigned NumBaseAdds;
  unsigned ImmCost;
  unsigned SetupCost;
  unsigned ScaleCost;

public:
  Cost()
    : NumRegs(0), AddRecCost(0), NumIVMuls(0), NumBaseAdds(0), ImmCost(0),
      SetupCost(0), ScaleCost(0) {}

  bool operator<(const Cost &Other) const;

  void Lose();

#ifndef NDEBUG
  // Once any of the metrics loses, they must all remain losers.
  bool isValid() {
    return ((NumRegs | AddRecCost | NumIVMuls | NumBaseAdds
             | ImmCost | SetupCost | ScaleCost) != ~0u)
      || ((NumRegs & AddRecCost & NumIVMuls & NumBaseAdds
           & ImmCost & SetupCost & ScaleCost) == ~0u);
  }
#endif

  bool isLoser() {
    assert(isValid() && "invalid cost");
    return NumRegs == ~0u;
  }

  void RateFormula(const TargetTransformInfo &TTI,
                   const Formula &F,
                   SmallPtrSetImpl<const SCEV *> &Regs,
                   const DenseSet<const SCEV *> &VisitedRegs,
                   const Loop *L,
                   const SmallVectorImpl<int64_t> &Offsets,
                   ScalarEvolution &SE, DominatorTree &DT,
                   const LSRUse &LU,
                   SmallPtrSetImpl<const SCEV *> *LoserRegs = nullptr);

  void print(raw_ostream &OS) const;
  void dump() const;

private:
  void RateRegister(const SCEV *Reg,
                    SmallPtrSetImpl<const SCEV *> &Regs,
                    const Loop *L,
                    ScalarEvolution &SE, DominatorTree &DT);
  void RatePrimaryRegister(const SCEV *Reg,
                           SmallPtrSetImpl<const SCEV *> &Regs,
                           const Loop *L,
                           ScalarEvolution &SE, DominatorTree &DT,
                           SmallPtrSetImpl<const SCEV *> *LoserRegs);
};

}

/// RateRegister - Tally up interesting quantities from the given register.
void Cost::RateRegister(const SCEV *Reg,
                        SmallPtrSetImpl<const SCEV *> &Regs,
                        const Loop *L,
                        ScalarEvolution &SE, DominatorTree &DT) {
  if (const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(Reg)) {
    // If this is an addrec for another loop, don't second-guess its addrec phi
    // nodes. LSR isn't currently smart enough to reason about more than one
    // loop at a time. LSR has already run on inner loops, will not run on outer
    // loops, and cannot be expected to change sibling loops.
    if (AR->getLoop() != L) {
      // If the AddRec exists, consider it's register free and leave it alone.
      if (isExistingPhi(AR, SE))
        return;

      // Otherwise, do not consider this formula at all.
      Lose();
      return;
    }
    AddRecCost += 1; /// TODO: This should be a function of the stride.

    // Add the step value register, if it needs one.
    // TODO: The non-affine case isn't precisely modeled here.
    if (!AR->isAffine() || !isa<SCEVConstant>(AR->getOperand(1))) {
      if (!Regs.count(AR->getOperand(1))) {
        RateRegister(AR->getOperand(1), Regs, L, SE, DT);
        if (isLoser())
          return;
      }
    }
  }
  ++NumRegs;

  // Rough heuristic; favor registers which don't require extra setup
  // instructions in the preheader.
  if (!isa<SCEVUnknown>(Reg) &&
      !isa<SCEVConstant>(Reg) &&
      !(isa<SCEVAddRecExpr>(Reg) &&
        (isa<SCEVUnknown>(cast<SCEVAddRecExpr>(Reg)->getStart()) ||
         isa<SCEVConstant>(cast<SCEVAddRecExpr>(Reg)->getStart()))))
    ++SetupCost;

  NumIVMuls += isa<SCEVMulExpr>(Reg) &&
               SE.hasComputableLoopEvolution(Reg, L);
}

/// RatePrimaryRegister - Record this register in the set. If we haven't seen it
/// before, rate it. Optional LoserRegs provides a way to declare any formula
/// that refers to one of those regs an instant loser.
void Cost::RatePrimaryRegister(const SCEV *Reg,
                               SmallPtrSetImpl<const SCEV *> &Regs,
                               const Loop *L,
                               ScalarEvolution &SE, DominatorTree &DT,
                               SmallPtrSetImpl<const SCEV *> *LoserRegs) {
  if (LoserRegs && LoserRegs->count(Reg)) {
    Lose();
    return;
  }
  if (Regs.insert(Reg).second) {
    RateRegister(Reg, Regs, L, SE, DT);
    if (LoserRegs && isLoser())
      LoserRegs->insert(Reg);
  }
}

void Cost::RateFormula(const TargetTransformInfo &TTI,
                       const Formula &F,
                       SmallPtrSetImpl<const SCEV *> &Regs,
                       const DenseSet<const SCEV *> &VisitedRegs,
                       const Loop *L,
                       const SmallVectorImpl<int64_t> &Offsets,
                       ScalarEvolution &SE, DominatorTree &DT,
                       const LSRUse &LU,
                       SmallPtrSetImpl<const SCEV *> *LoserRegs) {
  assert(F.isCanonical() && "Cost is accurate only for canonical formula");
  // Tally up the registers.
  if (const SCEV *ScaledReg = F.ScaledReg) {
    if (VisitedRegs.count(ScaledReg)) {
      Lose();
      return;
    }
    RatePrimaryRegister(ScaledReg, Regs, L, SE, DT, LoserRegs);
    if (isLoser())
      return;
  }
  for (const SCEV *BaseReg : F.BaseRegs) {
    if (VisitedRegs.count(BaseReg)) {
      Lose();
      return;
    }
    RatePrimaryRegister(BaseReg, Regs, L, SE, DT, LoserRegs);
    if (isLoser())
      return;
  }

  // Determine how many (unfolded) adds we'll need inside the loop.
  size_t NumBaseParts = F.getNumRegs();
  if (NumBaseParts > 1)
    // Do not count the base and a possible second register if the target
    // allows to fold 2 registers.
    NumBaseAdds +=
        NumBaseParts - (1 + (F.Scale && isAMCompletelyFolded(TTI, LU, F)));
  NumBaseAdds += (F.UnfoldedOffset != 0);

  // Accumulate non-free scaling amounts.
  ScaleCost += getScalingFactorCost(TTI, LU, F);

  // Tally up the non-zero immediates.
  for (int64_t O : Offsets) {
    int64_t Offset = (uint64_t)O + F.BaseOffset;
    if (F.BaseGV)
      ImmCost += 64; // Handle symbolic values conservatively.
                     // TODO: This should probably be the pointer size.
    else if (Offset != 0)
      ImmCost += APInt(64, Offset, true).getMinSignedBits();
  }
  assert(isValid() && "invalid cost");
}

/// Lose - Set this cost to a losing value.
void Cost::Lose() {
  NumRegs = ~0u;
  AddRecCost = ~0u;
  NumIVMuls = ~0u;
  NumBaseAdds = ~0u;
  ImmCost = ~0u;
  SetupCost = ~0u;
  ScaleCost = ~0u;
}

/// operator< - Choose the lower cost.
bool Cost::operator<(const Cost &Other) const {
  return std::tie(NumRegs, AddRecCost, NumIVMuls, NumBaseAdds, ScaleCost,
                  ImmCost, SetupCost) <
         std::tie(Other.NumRegs, Other.AddRecCost, Other.NumIVMuls,
                  Other.NumBaseAdds, Other.ScaleCost, Other.ImmCost,
                  Other.SetupCost);
}

void Cost::print(raw_ostream &OS) const {
  OS << NumRegs << " reg" << (NumRegs == 1 ? "" : "s");
  if (AddRecCost != 0)
    OS << ", with addrec cost " << AddRecCost;
  if (NumIVMuls != 0)
    OS << ", plus " << NumIVMuls << " IV mul" << (NumIVMuls == 1 ? "" : "s");
  if (NumBaseAdds != 0)
    OS << ", plus " << NumBaseAdds << " base add"
       << (NumBaseAdds == 1 ? "" : "s");
  if (ScaleCost != 0)
    OS << ", plus " << ScaleCost << " scale cost";
  if (ImmCost != 0)
    OS << ", plus " << ImmCost << " imm cost";
  if (SetupCost != 0)
    OS << ", plus " << SetupCost << " setup cost";
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void Cost::dump() const {
  print(errs()); errs() << '\n';
}
#endif

namespace {

/// LSRFixup - An operand value in an instruction which is to be replaced
/// with some equivalent, possibly strength-reduced, replacement.
struct LSRFixup {
  /// UserInst - The instruction which will be updated.
  Instruction *UserInst;

  /// OperandValToReplace - The operand of the instruction which will
  /// be replaced. The operand may be used more than once; every instance
  /// will be replaced.
  Value *OperandValToReplace;

  /// PostIncLoops - If this user is to use the post-incremented value of an
  /// induction variable, this variable is non-null and holds the loop
  /// associated with the induction variable.
  PostIncLoopSet PostIncLoops;

  /// LUIdx - The index of the LSRUse describing the expression which
  /// this fixup needs, minus an offset (below).
  size_t LUIdx;

  /// Offset - A constant offset to be added to the LSRUse expression.
  /// This allows multiple fixups to share the same LSRUse with different
  /// offsets, for example in an unrolled loop.
  int64_t Offset;

  bool isUseFullyOutsideLoop(const Loop *L) const;

  LSRFixup();

  void print(raw_ostream &OS) const;
  void dump() const;
};

}

LSRFixup::LSRFixup()
  : UserInst(nullptr), OperandValToReplace(nullptr), LUIdx(~size_t(0)),
    Offset(0) {}

/// isUseFullyOutsideLoop - Test whether this fixup always uses its
/// value outside of the given loop.
bool LSRFixup::isUseFullyOutsideLoop(const Loop *L) const {
  // PHI nodes use their value in their incoming blocks.
  if (const PHINode *PN = dyn_cast<PHINode>(UserInst)) {
    for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i)
      if (PN->getIncomingValue(i) == OperandValToReplace &&
          L->contains(PN->getIncomingBlock(i)))
        return false;
    return true;
  }

  return !L->contains(UserInst);
}

void LSRFixup::print(raw_ostream &OS) const {
  OS << "UserInst=";
  // Store is common and interesting enough to be worth special-casing.
  if (StoreInst *Store = dyn_cast<StoreInst>(UserInst)) {
    OS << "store ";
    Store->getOperand(0)->printAsOperand(OS, /*PrintType=*/false);
  } else if (UserInst->getType()->isVoidTy())
    OS << UserInst->getOpcodeName();
  else
    UserInst->printAsOperand(OS, /*PrintType=*/false);

  OS << ", OperandValToReplace=";
  OperandValToReplace->printAsOperand(OS, /*PrintType=*/false);

  for (const Loop *PIL : PostIncLoops) {
    OS << ", PostIncLoop=";
    PIL->getHeader()->printAsOperand(OS, /*PrintType=*/false);
  }

  if (LUIdx != ~size_t(0))
    OS << ", LUIdx=" << LUIdx;

  if (Offset != 0)
    OS << ", Offset=" << Offset;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void LSRFixup::dump() const {
  print(errs()); errs() << '\n';
}
#endif

namespace {

/// UniquifierDenseMapInfo - A DenseMapInfo implementation for holding
/// DenseMaps and DenseSets of sorted SmallVectors of const SCEV*.
struct UniquifierDenseMapInfo {
  static SmallVector<const SCEV *, 4> getEmptyKey() {
    SmallVector<const SCEV *, 4>  V;
    V.push_back(reinterpret_cast<const SCEV *>(-1));
    return V;
  }

  static SmallVector<const SCEV *, 4> getTombstoneKey() {
    SmallVector<const SCEV *, 4> V;
    V.push_back(reinterpret_cast<const SCEV *>(-2));
    return V;
  }

  static unsigned getHashValue(const SmallVector<const SCEV *, 4> &V) {
    return static_cast<unsigned>(hash_combine_range(V.begin(), V.end()));
  }

  static bool isEqual(const SmallVector<const SCEV *, 4> &LHS,
                      const SmallVector<const SCEV *, 4> &RHS) {
    return LHS == RHS;
  }
};

/// LSRUse - This class holds the state that LSR keeps for each use in
/// IVUsers, as well as uses invented by LSR itself. It includes information
/// about what kinds of things can be folded into the user, information about
/// the user itself, and information about how the use may be satisfied.
/// TODO: Represent multiple users of the same expression in common?
class LSRUse {
  DenseSet<SmallVector<const SCEV *, 4>, UniquifierDenseMapInfo> Uniquifier;

public:
  /// KindType - An enum for a kind of use, indicating what types of
  /// scaled and immediate operands it might support.
  enum KindType {
    Basic,   ///< A normal use, with no folding.
    Special, ///< A special case of basic, allowing -1 scales.
    Address, ///< An address use; folding according to TargetLowering
    ICmpZero ///< An equality icmp with both operands folded into one.
    // TODO: Add a generic icmp too?
  };

  typedef PointerIntPair<const SCEV *, 2, KindType> SCEVUseKindPair;

  KindType Kind;
  Type *AccessTy;

  SmallVector<int64_t, 8> Offsets;
  int64_t MinOffset;
  int64_t MaxOffset;

  /// AllFixupsOutsideLoop - This records whether all of the fixups using this
  /// LSRUse are outside of the loop, in which case some special-case heuristics
  /// may be used.
  bool AllFixupsOutsideLoop;

  /// RigidFormula is set to true to guarantee that this use will be associated
  /// with a single formula--the one that initially matched. Some SCEV
  /// expressions cannot be expanded. This allows LSR to consider the registers
  /// used by those expressions without the need to expand them later after
  /// changing the formula.
  bool RigidFormula;

  /// WidestFixupType - This records the widest use type for any fixup using
  /// this LSRUse. FindUseWithSimilarFormula can't consider uses with different
  /// max fixup widths to be equivalent, because the narrower one may be relying
  /// on the implicit truncation to truncate away bogus bits.
  Type *WidestFixupType;

  /// Formulae - A list of ways to build a value that can satisfy this user.
  /// After the list is populated, one of these is selected heuristically and
  /// used to formulate a replacement for OperandValToReplace in UserInst.
  SmallVector<Formula, 12> Formulae;

  /// Regs - The set of register candidates used by all formulae in this LSRUse.
  SmallPtrSet<const SCEV *, 4> Regs;

  LSRUse(KindType K, Type *T) : Kind(K), AccessTy(T),
                                      MinOffset(INT64_MAX),
                                      MaxOffset(INT64_MIN),
                                      AllFixupsOutsideLoop(true),
                                      RigidFormula(false),
                                      WidestFixupType(nullptr) {}

  bool HasFormulaWithSameRegs(const Formula &F) const;
  bool InsertFormula(const Formula &F);
  void DeleteFormula(Formula &F);
  void RecomputeRegs(size_t LUIdx, RegUseTracker &Reguses);

  void print(raw_ostream &OS) const;
  void dump() const;
};

}

/// HasFormula - Test whether this use as a formula which has the same
/// registers as the given formula.
bool LSRUse::HasFormulaWithSameRegs(const Formula &F) const {
  SmallVector<const SCEV *, 4> Key = F.BaseRegs;
  if (F.ScaledReg) Key.push_back(F.ScaledReg);
  // Unstable sort by host order ok, because this is only used for uniquifying.
  std::sort(Key.begin(), Key.end());
  return Uniquifier.count(Key);
}

/// InsertFormula - If the given formula has not yet been inserted, add it to
/// the list, and return true. Return false otherwise.
/// The formula must be in canonical form.
bool LSRUse::InsertFormula(const Formula &F) {
  assert(F.isCanonical() && "Invalid canonical representation");

  if (!Formulae.empty() && RigidFormula)
    return false;

  SmallVector<const SCEV *, 4> Key = F.BaseRegs;
  if (F.ScaledReg) Key.push_back(F.ScaledReg);
  // Unstable sort by host order ok, because this is only used for uniquifying.
  std::sort(Key.begin(), Key.end());

  if (!Uniquifier.insert(Key).second)
    return false;

  // Using a register to hold the value of 0 is not profitable.
  assert((!F.ScaledReg || !F.ScaledReg->isZero()) &&
         "Zero allocated in a scaled register!");
#ifndef NDEBUG
  for (const SCEV *BaseReg : F.BaseRegs)
    assert(!BaseReg->isZero() && "Zero allocated in a base register!");
#endif

  // Add the formula to the list.
  Formulae.push_back(F);

  // Record registers now being used by this use.
  Regs.insert(F.BaseRegs.begin(), F.BaseRegs.end());
  if (F.ScaledReg)
    Regs.insert(F.ScaledReg);

  return true;
}

/// DeleteFormula - Remove the given formula from this use's list.
void LSRUse::DeleteFormula(Formula &F) {
  if (&F != &Formulae.back())
    std::swap(F, Formulae.back());
  Formulae.pop_back();
}

/// RecomputeRegs - Recompute the Regs field, and update RegUses.
void LSRUse::RecomputeRegs(size_t LUIdx, RegUseTracker &RegUses) {
  // Now that we've filtered out some formulae, recompute the Regs set.
  SmallPtrSet<const SCEV *, 4> OldRegs = std::move(Regs);
  Regs.clear();
  for (const Formula &F : Formulae) {
    if (F.ScaledReg) Regs.insert(F.ScaledReg);
    Regs.insert(F.BaseRegs.begin(), F.BaseRegs.end());
  }

  // Update the RegTracker.
  for (const SCEV *S : OldRegs)
    if (!Regs.count(S))
      RegUses.DropRegister(S, LUIdx);
}

void LSRUse::print(raw_ostream &OS) const {
  OS << "LSR Use: Kind=";
  switch (Kind) {
  case Basic:    OS << "Basic"; break;
  case Special:  OS << "Special"; break;
  case ICmpZero: OS << "ICmpZero"; break;
  case Address:
    OS << "Address of ";
    if (AccessTy->isPointerTy())
      OS << "pointer"; // the full pointer type could be really verbose
    else
      OS << *AccessTy;
  }

  OS << ", Offsets={";
  bool NeedComma = false;
  for (int64_t O : Offsets) {
    if (NeedComma) OS << ',';
    OS << O;
    NeedComma = true;
  }
  OS << '}';

  if (AllFixupsOutsideLoop)
    OS << ", all-fixups-outside-loop";

  if (WidestFixupType)
    OS << ", widest fixup type: " << *WidestFixupType;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void LSRUse::dump() const {
  print(errs()); errs() << '\n';
}
#endif

static bool isAMCompletelyFolded(const TargetTransformInfo &TTI,
                                 LSRUse::KindType Kind, Type *AccessTy,
                                 GlobalValue *BaseGV, int64_t BaseOffset,
                                 bool HasBaseReg, int64_t Scale) {
  switch (Kind) {
  case LSRUse::Address:
    return TTI.isLegalAddressingMode(AccessTy, BaseGV, BaseOffset, HasBaseReg, Scale);

  case LSRUse::ICmpZero:
    // There's not even a target hook for querying whether it would be legal to
    // fold a GV into an ICmp.
    if (BaseGV)
      return false;

    // ICmp only has two operands; don't allow more than two non-trivial parts.
    if (Scale != 0 && HasBaseReg && BaseOffset != 0)
      return false;

    // ICmp only supports no scale or a -1 scale, as we can "fold" a -1 scale by
    // putting the scaled register in the other operand of the icmp.
    if (Scale != 0 && Scale != -1)
      return false;

    // If we have low-level target information, ask the target if it can fold an
    // integer immediate on an icmp.
    if (BaseOffset != 0) {
      // We have one of:
      // ICmpZero     BaseReg + BaseOffset => ICmp BaseReg, -BaseOffset
      // ICmpZero -1*ScaleReg + BaseOffset => ICmp ScaleReg, BaseOffset
      // Offs is the ICmp immediate.
      if (Scale == 0)
        // The cast does the right thing with INT64_MIN.
        BaseOffset = -(uint64_t)BaseOffset;
      return TTI.isLegalICmpImmediate(BaseOffset);
    }

    // ICmpZero BaseReg + -1*ScaleReg => ICmp BaseReg, ScaleReg
    return true;

  case LSRUse::Basic:
    // Only handle single-register values.
    return !BaseGV && Scale == 0 && BaseOffset == 0;

  case LSRUse::Special:
    // Special case Basic to handle -1 scales.
    return !BaseGV && (Scale == 0 || Scale == -1) && BaseOffset == 0;
  }

  llvm_unreachable("Invalid LSRUse Kind!");
}

static bool isAMCompletelyFolded(const TargetTransformInfo &TTI,
                                 int64_t MinOffset, int64_t MaxOffset,
                                 LSRUse::KindType Kind, Type *AccessTy,
                                 GlobalValue *BaseGV, int64_t BaseOffset,
                                 bool HasBaseReg, int64_t Scale) {
  // Check for overflow.
  if (((int64_t)((uint64_t)BaseOffset + MinOffset) > BaseOffset) !=
      (MinOffset > 0))
    return false;
  MinOffset = (uint64_t)BaseOffset + MinOffset;
  if (((int64_t)((uint64_t)BaseOffset + MaxOffset) > BaseOffset) !=
      (MaxOffset > 0))
    return false;
  MaxOffset = (uint64_t)BaseOffset + MaxOffset;

  return isAMCompletelyFolded(TTI, Kind, AccessTy, BaseGV, MinOffset,
                              HasBaseReg, Scale) &&
         isAMCompletelyFolded(TTI, Kind, AccessTy, BaseGV, MaxOffset,
                              HasBaseReg, Scale);
}

static bool isAMCompletelyFolded(const TargetTransformInfo &TTI,
                                 int64_t MinOffset, int64_t MaxOffset,
                                 LSRUse::KindType Kind, Type *AccessTy,
                                 const Formula &F) {
  // For the purpose of isAMCompletelyFolded either having a canonical formula
  // or a scale not equal to zero is correct.
  // Problems may arise from non canonical formulae having a scale == 0.
  // Strictly speaking it would best to just rely on canonical formulae.
  // However, when we generate the scaled formulae, we first check that the
  // scaling factor is profitable before computing the actual ScaledReg for
  // compile time sake.
  assert((F.isCanonical() || F.Scale != 0));
  return isAMCompletelyFolded(TTI, MinOffset, MaxOffset, Kind, AccessTy,
                              F.BaseGV, F.BaseOffset, F.HasBaseReg, F.Scale);
}

/// isLegalUse - Test whether we know how to expand the current formula.
static bool isLegalUse(const TargetTransformInfo &TTI, int64_t MinOffset,
                       int64_t MaxOffset, LSRUse::KindType Kind, Type *AccessTy,
                       GlobalValue *BaseGV, int64_t BaseOffset, bool HasBaseReg,
                       int64_t Scale) {
  // We know how to expand completely foldable formulae.
  return isAMCompletelyFolded(TTI, MinOffset, MaxOffset, Kind, AccessTy, BaseGV,
                              BaseOffset, HasBaseReg, Scale) ||
         // Or formulae that use a base register produced by a sum of base
         // registers.
         (Scale == 1 &&
          isAMCompletelyFolded(TTI, MinOffset, MaxOffset, Kind, AccessTy,
                               BaseGV, BaseOffset, true, 0));
}

static bool isLegalUse(const TargetTransformInfo &TTI, int64_t MinOffset,
                       int64_t MaxOffset, LSRUse::KindType Kind, Type *AccessTy,
                       const Formula &F) {
  return isLegalUse(TTI, MinOffset, MaxOffset, Kind, AccessTy, F.BaseGV,
                    F.BaseOffset, F.HasBaseReg, F.Scale);
}

static bool isAMCompletelyFolded(const TargetTransformInfo &TTI,
                                 const LSRUse &LU, const Formula &F) {
  return isAMCompletelyFolded(TTI, LU.MinOffset, LU.MaxOffset, LU.Kind,
                              LU.AccessTy, F.BaseGV, F.BaseOffset, F.HasBaseReg,
                              F.Scale);
}

static unsigned getScalingFactorCost(const TargetTransformInfo &TTI,
                                     const LSRUse &LU, const Formula &F) {
  if (!F.Scale)
    return 0;

  // If the use is not completely folded in that instruction, we will have to
  // pay an extra cost only for scale != 1.
  if (!isAMCompletelyFolded(TTI, LU.MinOffset, LU.MaxOffset, LU.Kind,
                            LU.AccessTy, F))
    return F.Scale != 1;

  switch (LU.Kind) {
  case LSRUse::Address: {
    // Check the scaling factor cost with both the min and max offsets.
    int ScaleCostMinOffset =
      TTI.getScalingFactorCost(LU.AccessTy, F.BaseGV,
                               F.BaseOffset + LU.MinOffset,
                               F.HasBaseReg, F.Scale);
    int ScaleCostMaxOffset =
      TTI.getScalingFactorCost(LU.AccessTy, F.BaseGV,
                               F.BaseOffset + LU.MaxOffset,
                               F.HasBaseReg, F.Scale);

    assert(ScaleCostMinOffset >= 0 && ScaleCostMaxOffset >= 0 &&
           "Legal addressing mode has an illegal cost!");
    return std::max(ScaleCostMinOffset, ScaleCostMaxOffset);
  }
  case LSRUse::ICmpZero:
  case LSRUse::Basic:
  case LSRUse::Special:
    // The use is completely folded, i.e., everything is folded into the
    // instruction.
    return 0;
  }

  llvm_unreachable("Invalid LSRUse Kind!");
}

static bool isAlwaysFoldable(const TargetTransformInfo &TTI,
                             LSRUse::KindType Kind, Type *AccessTy,
                             GlobalValue *BaseGV, int64_t BaseOffset,
                             bool HasBaseReg) {
  // Fast-path: zero is always foldable.
  if (BaseOffset == 0 && !BaseGV) return true;

  // Conservatively, create an address with an immediate and a
  // base and a scale.
  int64_t Scale = Kind == LSRUse::ICmpZero ? -1 : 1;

  // Canonicalize a scale of 1 to a base register if the formula doesn't
  // already have a base register.
  if (!HasBaseReg && Scale == 1) {
    Scale = 0;
    HasBaseReg = true;
  }

  return isAMCompletelyFolded(TTI, Kind, AccessTy, BaseGV, BaseOffset,
                              HasBaseReg, Scale);
}

static bool isAlwaysFoldable(const TargetTransformInfo &TTI,
                             ScalarEvolution &SE, int64_t MinOffset,
                             int64_t MaxOffset, LSRUse::KindType Kind,
                             Type *AccessTy, const SCEV *S, bool HasBaseReg) {
  // Fast-path: zero is always foldable.
  if (S->isZero()) return true;

  // Conservatively, create an address with an immediate and a
  // base and a scale.
  int64_t BaseOffset = ExtractImmediate(S, SE);
  GlobalValue *BaseGV = ExtractSymbol(S, SE);

  // If there's anything else involved, it's not foldable.
  if (!S->isZero()) return false;

  // Fast-path: zero is always foldable.
  if (BaseOffset == 0 && !BaseGV) return true;

  // Conservatively, create an address with an immediate and a
  // base and a scale.
  int64_t Scale = Kind == LSRUse::ICmpZero ? -1 : 1;

  return isAMCompletelyFolded(TTI, MinOffset, MaxOffset, Kind, AccessTy, BaseGV,
                              BaseOffset, HasBaseReg, Scale);
}

namespace {

/// IVInc - An individual increment in a Chain of IV increments.
/// Relate an IV user to an expression that computes the IV it uses from the IV
/// used by the previous link in the Chain.
///
/// For the head of a chain, IncExpr holds the absolute SCEV expression for the
/// original IVOperand. The head of the chain's IVOperand is only valid during
/// chain collection, before LSR replaces IV users. During chain generation,
/// IncExpr can be used to find the new IVOperand that computes the same
/// expression.
struct IVInc {
  Instruction *UserInst;
  Value* IVOperand;
  const SCEV *IncExpr;

  IVInc(Instruction *U, Value *O, const SCEV *E):
    UserInst(U), IVOperand(O), IncExpr(E) {}
};

// IVChain - The list of IV increments in program order.
// We typically add the head of a chain without finding subsequent links.
struct IVChain {
  SmallVector<IVInc,1> Incs;
  const SCEV *ExprBase;

  IVChain() : ExprBase(nullptr) {}

  IVChain(const IVInc &Head, const SCEV *Base)
    : Incs(1, Head), ExprBase(Base) {}

  typedef SmallVectorImpl<IVInc>::const_iterator const_iterator;

  // begin - return the first increment in the chain.
  const_iterator begin() const {
    assert(!Incs.empty());
    return std::next(Incs.begin());
  }
  const_iterator end() const {
    return Incs.end();
  }

  // hasIncs - Returns true if this chain contains any increments.
  bool hasIncs() const { return Incs.size() >= 2; }

  // add - Add an IVInc to the end of this chain.
  void add(const IVInc &X) { Incs.push_back(X); }

  // tailUserInst - Returns the last UserInst in the chain.
  Instruction *tailUserInst() const { return Incs.back().UserInst; }

  // isProfitableIncrement - Returns true if IncExpr can be profitably added to
  // this chain.
  bool isProfitableIncrement(const SCEV *OperExpr,
                             const SCEV *IncExpr,
                             ScalarEvolution&);
};

/// ChainUsers - Helper for CollectChains to track multiple IV increment uses.
/// Distinguish between FarUsers that definitely cross IV increments and
/// NearUsers that may be used between IV increments.
struct ChainUsers {
  SmallPtrSet<Instruction*, 4> FarUsers;
  SmallPtrSet<Instruction*, 4> NearUsers;
};

/// LSRInstance - This class holds state for the main loop strength reduction
/// logic.
class LSRInstance {
  IVUsers &IU;
  ScalarEvolution &SE;
  DominatorTree &DT;
  LoopInfo &LI;
  const TargetTransformInfo &TTI;
  Loop *const L;
  bool Changed;

  /// IVIncInsertPos - This is the insert position that the current loop's
  /// induction variable increment should be placed. In simple loops, this is
  /// the latch block's terminator. But in more complicated cases, this is a
  /// position which will dominate all the in-loop post-increment users.
  Instruction *IVIncInsertPos;

  /// Factors - Interesting factors between use strides.
  SmallSetVector<int64_t, 8> Factors;

  /// Types - Interesting use types, to facilitate truncation reuse.
  SmallSetVector<Type *, 4> Types;

  /// Fixups - The list of operands which are to be replaced.
  SmallVector<LSRFixup, 16> Fixups;

  /// Uses - The list of interesting uses.
  SmallVector<LSRUse, 16> Uses;

  /// RegUses - Track which uses use which register candidates.
  RegUseTracker RegUses;

  // Limit the number of chains to avoid quadratic behavior. We don't expect to
  // have more than a few IV increment chains in a loop. Missing a Chain falls
  // back to normal LSR behavior for those uses.
  static const unsigned MaxChains = 8;

  /// IVChainVec - IV users can form a chain of IV increments.
  SmallVector<IVChain, MaxChains> IVChainVec;

  /// IVIncSet - IV users that belong to profitable IVChains.
  SmallPtrSet<Use*, MaxChains> IVIncSet;

  void OptimizeShadowIV();
  bool FindIVUserForCond(ICmpInst *Cond, IVStrideUse *&CondUse);
  ICmpInst *OptimizeMax(ICmpInst *Cond, IVStrideUse* &CondUse);
  void OptimizeLoopTermCond();

  void ChainInstruction(Instruction *UserInst, Instruction *IVOper,
                        SmallVectorImpl<ChainUsers> &ChainUsersVec);
  void FinalizeChain(IVChain &Chain);
  void CollectChains();
  void GenerateIVChain(const IVChain &Chain, SCEVExpander &Rewriter,
                       SmallVectorImpl<WeakTrackingVH> &DeadInsts);

  void CollectInterestingTypesAndFactors();
  void CollectFixupsAndInitialFormulae();

  LSRFixup &getNewFixup() {
    Fixups.push_back(LSRFixup());
    return Fixups.back();
  }

  // Support for sharing of LSRUses between LSRFixups.
  typedef DenseMap<LSRUse::SCEVUseKindPair, size_t> UseMapTy;
  UseMapTy UseMap;

  bool reconcileNewOffset(LSRUse &LU, int64_t NewOffset, bool HasBaseReg,
                          LSRUse::KindType Kind, Type *AccessTy);

  std::pair<size_t, int64_t> getUse(const SCEV *&Expr,
                                    LSRUse::KindType Kind,
                                    Type *AccessTy);

  void DeleteUse(LSRUse &LU, size_t LUIdx);

  LSRUse *FindUseWithSimilarFormula(const Formula &F, const LSRUse &OrigLU);

  void InsertInitialFormula(const SCEV *S, LSRUse &LU, size_t LUIdx);
  void InsertSupplementalFormula(const SCEV *S, LSRUse &LU, size_t LUIdx);
  void CountRegisters(const Formula &F, size_t LUIdx);
  bool InsertFormula(LSRUse &LU, unsigned LUIdx, const Formula &F);

  void CollectLoopInvariantFixupsAndFormulae();

  void GenerateReassociations(LSRUse &LU, unsigned LUIdx, Formula Base,
                              unsigned Depth = 0);

  void GenerateReassociationsImpl(LSRUse &LU, unsigned LUIdx,
                                  const Formula &Base, unsigned Depth,
                                  size_t Idx, bool IsScaledReg = false);
  void GenerateCombinations(LSRUse &LU, unsigned LUIdx, Formula Base);
  void GenerateSymbolicOffsetsImpl(LSRUse &LU, unsigned LUIdx,
                                   const Formula &Base, size_t Idx,
                                   bool IsScaledReg = false);
  void GenerateSymbolicOffsets(LSRUse &LU, unsigned LUIdx, Formula Base);
  void GenerateConstantOffsetsImpl(LSRUse &LU, unsigned LUIdx,
                                   const Formula &Base,
                                   const SmallVectorImpl<int64_t> &Worklist,
                                   size_t Idx, bool IsScaledReg = false);
  void GenerateConstantOffsets(LSRUse &LU, unsigned LUIdx, Formula Base);
  void GenerateICmpZeroScales(LSRUse &LU, unsigned LUIdx, Formula Base);
  void GenerateScales(LSRUse &LU, unsigned LUIdx, Formula Base);
  void GenerateTruncates(LSRUse &LU, unsigned LUIdx, Formula Base);
  void GenerateCrossUseConstantOffsets();
  void GenerateAllReuseFormulae();

  void FilterOutUndesirableDedicatedRegisters();

  size_t EstimateSearchSpaceComplexity() const;
  void NarrowSearchSpaceByDetectingSupersets();
  void NarrowSearchSpaceByCollapsingUnrolledCode();
  void NarrowSearchSpaceByRefilteringUndesirableDedicatedRegisters();
  void NarrowSearchSpaceByPickingWinnerRegs();
  void NarrowSearchSpaceUsingHeuristics();

  void SolveRecurse(SmallVectorImpl<const Formula *> &Solution,
                    Cost &SolutionCost,
                    SmallVectorImpl<const Formula *> &Workspace,
                    const Cost &CurCost,
                    const SmallPtrSet<const SCEV *, 16> &CurRegs,
                    DenseSet<const SCEV *> &VisitedRegs) const;
  void Solve(SmallVectorImpl<const Formula *> &Solution) const;

  BasicBlock::iterator
    HoistInsertPosition(BasicBlock::iterator IP,
                        const SmallVectorImpl<Instruction *> &Inputs) const;
  BasicBlock::iterator
    AdjustInsertPositionForExpand(BasicBlock::iterator IP,
                                  const LSRFixup &LF,
                                  const LSRUse &LU,
                                  SCEVExpander &Rewriter) const;

  Value *Expand(const LSRFixup &LF, const Formula &F, BasicBlock::iterator IP,
                SCEVExpander &Rewriter,
                SmallVectorImpl<WeakTrackingVH> &DeadInsts) const;
  void RewriteForPHI(PHINode *PN, const LSRFixup &LF, const Formula &F,
                     SCEVExpander &Rewriter,
                     SmallVectorImpl<WeakTrackingVH> &DeadInsts, Pass *P) const;
  void Rewrite(const LSRFixup &LF, const Formula &F, SCEVExpander &Rewriter,
               SmallVectorImpl<WeakTrackingVH> &DeadInsts, Pass *P) const;
  void ImplementSolution(const SmallVectorImpl<const Formula *> &Solution,
                         Pass *P);

public:
  LSRInstance(Loop *L, Pass *P);

  bool getChanged() const { return Changed; }

  void print_factors_and_types(raw_ostream &OS) const;
  void print_fixups(raw_ostream &OS) const;
  void print_uses(raw_ostream &OS) const;
  void print(raw_ostream &OS) const;
  void dump() const;
};

}

/// OptimizeShadowIV - If IV is used in a int-to-float cast
/// inside the loop then try to eliminate the cast operation.
void LSRInstance::OptimizeShadowIV() {
  const SCEV *BackedgeTakenCount = SE.getBackedgeTakenCount(L);
  if (isa<SCEVCouldNotCompute>(BackedgeTakenCount))
    return;

  for (IVUsers::const_iterator UI = IU.begin(), E = IU.end();
       UI != E; /* empty */) {
    IVUsers::const_iterator CandidateUI = UI;
    ++UI;
    Instruction *ShadowUse = CandidateUI->getUser();
    Type *DestTy = nullptr;
    bool IsSigned = false;

    /* If shadow use is a int->float cast then insert a second IV
       to eliminate this cast.

         for (unsigned i = 0; i < n; ++i)
           foo((double)i);

       is transformed into

         double d = 0.0;
         for (unsigned i = 0; i < n; ++i, ++d)
           foo(d);
    */
    if (UIToFPInst *UCast = dyn_cast<UIToFPInst>(CandidateUI->getUser())) {
      IsSigned = false;
      DestTy = UCast->getDestTy();
    }
    else if (SIToFPInst *SCast = dyn_cast<SIToFPInst>(CandidateUI->getUser())) {
      IsSigned = true;
      DestTy = SCast->getDestTy();
    }
    if (!DestTy) continue;

    // If target does not support DestTy natively then do not apply
    // this transformation.
    if (!TTI.isTypeLegal(DestTy)) continue;

    PHINode *PH = dyn_cast<PHINode>(ShadowUse->getOperand(0));
    if (!PH) continue;
    if (PH->getNumIncomingValues() != 2) continue;

    Type *SrcTy = PH->getType();
    int Mantissa = DestTy->getFPMantissaWidth();
    if (Mantissa == -1) continue;
    if ((int)SE.getTypeSizeInBits(SrcTy) > Mantissa)
      continue;

    unsigned Entry, Latch;
    if (PH->getIncomingBlock(0) == L->getLoopPreheader()) {
      Entry = 0;
      Latch = 1;
    } else {
      Entry = 1;
      Latch = 0;
    }

    ConstantInt *Init = dyn_cast<ConstantInt>(PH->getIncomingValue(Entry));
    if (!Init) continue;
    Constant *NewInit = ConstantFP::get(DestTy, IsSigned ?
                                        (double)Init->getSExtValue() :
                                        (double)Init->getZExtValue());

    BinaryOperator *Incr =
      dyn_cast<BinaryOperator>(PH->getIncomingValue(Latch));
    if (!Incr) continue;
    if (Incr->getOpcode() != Instruction::Add
        && Incr->getOpcode() != Instruction::Sub)
      continue;

    /* Initialize new IV, double d = 0.0 in above example. */
    ConstantInt *C = nullptr;
    if (Incr->getOperand(0) == PH)
      C = dyn_cast<ConstantInt>(Incr->getOperand(1));
    else if (Incr->getOperand(1) == PH)
      C = dyn_cast<ConstantInt>(Incr->getOperand(0));
    else
      continue;

    if (!C) continue;

    // Ignore negative constants, as the code below doesn't handle them
    // correctly. TODO: Remove this restriction.
    if (!C->getValue().isStrictlyPositive()) continue;

    /* Add new PHINode. */
    PHINode *NewPH = PHINode::Create(DestTy, 2, "IV.S.", PH);

    /* create new increment. '++d' in above example. */
    Constant *CFP = ConstantFP::get(DestTy, C->getZExtValue());
    BinaryOperator *NewIncr =
      BinaryOperator::Create(Incr->getOpcode() == Instruction::Add ?
                               Instruction::FAdd : Instruction::FSub,
                             NewPH, CFP, "IV.S.next.", Incr);

    NewPH->addIncoming(NewInit, PH->getIncomingBlock(Entry));
    NewPH->addIncoming(NewIncr, PH->getIncomingBlock(Latch));

    /* Remove cast operation */
    ShadowUse->replaceAllUsesWith(NewPH);
    ShadowUse->eraseFromParent();
    Changed = true;
    break;
  }
}

/// FindIVUserForCond - If Cond has an operand that is an expression of an IV,
/// set the IV user and stride information and return true, otherwise return
/// false.
bool LSRInstance::FindIVUserForCond(ICmpInst *Cond, IVStrideUse *&CondUse) {
  for (IVStrideUse &U : IU)
    if (U.getUser() == Cond) {
      // NOTE: we could handle setcc instructions with multiple uses here, but
      // InstCombine does it as well for simple uses, it's not clear that it
      // occurs enough in real life to handle.
      CondUse = &U;
      return true;
    }
  return false;
}

/// OptimizeMax - Rewrite the loop's terminating condition if it uses
/// a max computation.
///
/// This is a narrow solution to a specific, but acute, problem. For loops
/// like this:
///
///   i = 0;
///   do {
///     p[i] = 0.0;
///   } while (++i < n);
///
/// the trip count isn't just 'n', because 'n' might not be positive. And
/// unfortunately this can come up even for loops where the user didn't use
/// a C do-while loop. For example, seemingly well-behaved top-test loops
/// will commonly be lowered like this:
//
///   if (n > 0) {
///     i = 0;
///     do {
///       p[i] = 0.0;
///     } while (++i < n);
///   }
///
/// and then it's possible for subsequent optimization to obscure the if
/// test in such a way that indvars can't find it.
///
/// When indvars can't find the if test in loops like this, it creates a
/// max expression, which allows it to give the loop a canonical
/// induction variable:
///
///   i = 0;
///   max = n < 1 ? 1 : n;
///   do {
///     p[i] = 0.0;
///   } while (++i != max);
///
/// Canonical induction variables are necessary because the loop passes
/// are designed around them. The most obvious example of this is the
/// LoopInfo analysis, which doesn't remember trip count values. It
/// expects to be able to rediscover the trip count each time it is
/// needed, and it does this using a simple analysis that only succeeds if
/// the loop has a canonical induction variable.
///
/// However, when it comes time to generate code, the maximum operation
/// can be quite costly, especially if it's inside of an outer loop.
///
/// This function solves this problem by detecting this type of loop and
/// rewriting their conditions from ICMP_NE back to ICMP_SLT, and deleting
/// the instructions for the maximum computation.
///
ICmpInst *LSRInstance::OptimizeMax(ICmpInst *Cond, IVStrideUse* &CondUse) {
  // Check that the loop matches the pattern we're looking for.
  if (Cond->getPredicate() != CmpInst::ICMP_EQ &&
      Cond->getPredicate() != CmpInst::ICMP_NE)
    return Cond;

  SelectInst *Sel = dyn_cast<SelectInst>(Cond->getOperand(1));
  if (!Sel || !Sel->hasOneUse()) return Cond;

  const SCEV *BackedgeTakenCount = SE.getBackedgeTakenCount(L);
  if (isa<SCEVCouldNotCompute>(BackedgeTakenCount))
    return Cond;
  const SCEV *One = SE.getConstant(BackedgeTakenCount->getType(), 1);

  // Add one to the backedge-taken count to get the trip count.
  const SCEV *IterationCount = SE.getAddExpr(One, BackedgeTakenCount);
  if (IterationCount != SE.getSCEV(Sel)) return Cond;

  // Check for a max calculation that matches the pattern. There's no check
  // for ICMP_ULE here because the comparison would be with zero, which
  // isn't interesting.
  CmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
  const SCEVNAryExpr *Max = nullptr;
  if (const SCEVSMaxExpr *S = dyn_cast<SCEVSMaxExpr>(BackedgeTakenCount)) {
    Pred = ICmpInst::ICMP_SLE;
    Max = S;
  } else if (const SCEVSMaxExpr *S = dyn_cast<SCEVSMaxExpr>(IterationCount)) {
    Pred = ICmpInst::ICMP_SLT;
    Max = S;
  } else if (const SCEVUMaxExpr *U = dyn_cast<SCEVUMaxExpr>(IterationCount)) {
    Pred = ICmpInst::ICMP_ULT;
    Max = U;
  } else {
    // No match; bail.
    return Cond;
  }

  // To handle a max with more than two operands, this optimization would
  // require additional checking and setup.
  if (Max->getNumOperands() != 2)
    return Cond;

  const SCEV *MaxLHS = Max->getOperand(0);
  const SCEV *MaxRHS = Max->getOperand(1);

  // ScalarEvolution canonicalizes constants to the left. For < and >, look
  // for a comparison with 1. For <= and >=, a comparison with zero.
  if (!MaxLHS ||
      (ICmpInst::isTrueWhenEqual(Pred) ? !MaxLHS->isZero() : (MaxLHS != One)))
    return Cond;

  // Check the relevant induction variable for conformance to
  // the pattern.
  const SCEV *IV = SE.getSCEV(Cond->getOperand(0));
  const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(IV);
  if (!AR || !AR->isAffine() ||
      AR->getStart() != One ||
      AR->getStepRecurrence(SE) != One)
    return Cond;

  assert(AR->getLoop() == L &&
         "Loop condition operand is an addrec in a different loop!");

  // Check the right operand of the select, and remember it, as it will
  // be used in the new comparison instruction.
  Value *NewRHS = nullptr;
  if (ICmpInst::isTrueWhenEqual(Pred)) {
    // Look for n+1, and grab n.
    if (AddOperator *BO = dyn_cast<AddOperator>(Sel->getOperand(1)))
      if (ConstantInt *BO1 = dyn_cast<ConstantInt>(BO->getOperand(1)))
         if (BO1->isOne() && SE.getSCEV(BO->getOperand(0)) == MaxRHS)
           NewRHS = BO->getOperand(0);
    if (AddOperator *BO = dyn_cast<AddOperator>(Sel->getOperand(2)))
      if (ConstantInt *BO1 = dyn_cast<ConstantInt>(BO->getOperand(1)))
        if (BO1->isOne() && SE.getSCEV(BO->getOperand(0)) == MaxRHS)
          NewRHS = BO->getOperand(0);
    if (!NewRHS)
      return Cond;
  } else if (SE.getSCEV(Sel->getOperand(1)) == MaxRHS)
    NewRHS = Sel->getOperand(1);
  else if (SE.getSCEV(Sel->getOperand(2)) == MaxRHS)
    NewRHS = Sel->getOperand(2);
  else if (const SCEVUnknown *SU = dyn_cast<SCEVUnknown>(MaxRHS))
    NewRHS = SU->getValue();
  else
    // Max doesn't match expected pattern.
    return Cond;

  // Determine the new comparison opcode. It may be signed or unsigned,
  // and the original comparison may be either equality or inequality.
  if (Cond->getPredicate() == CmpInst::ICMP_EQ)
    Pred = CmpInst::getInversePredicate(Pred);

  // Ok, everything looks ok to change the condition into an SLT or SGE and
  // delete the max calculation.
  ICmpInst *NewCond =
    new ICmpInst(Cond, Pred, Cond->getOperand(0), NewRHS, "scmp");

  // Delete the max calculation instructions.
  Cond->replaceAllUsesWith(NewCond);
  CondUse->setUser(NewCond);
  Instruction *Cmp = cast<Instruction>(Sel->getOperand(0));
  Cond->eraseFromParent();
  Sel->eraseFromParent();
  if (Cmp->use_empty())
    Cmp->eraseFromParent();
  return NewCond;
}

/// OptimizeLoopTermCond - Change loop terminating condition to use the
/// postinc iv when possible.
void
LSRInstance::OptimizeLoopTermCond() {
  SmallPtrSet<Instruction *, 4> PostIncs;

  BasicBlock *LatchBlock = L->getLoopLatch();
  SmallVector<BasicBlock*, 8> ExitingBlocks;
  L->getExitingBlocks(ExitingBlocks);

  for (BasicBlock *ExitingBlock : ExitingBlocks) {

    // Get the terminating condition for the loop if possible.  If we
    // can, we want to change it to use a post-incremented version of its
    // induction variable, to allow coalescing the live ranges for the IV into
    // one register value.

    BranchInst *TermBr = dyn_cast<BranchInst>(ExitingBlock->getTerminator());
    if (!TermBr)
      continue;
    // FIXME: Overly conservative, termination condition could be an 'or' etc..
    if (TermBr->isUnconditional() || !isa<ICmpInst>(TermBr->getCondition()))
      continue;

    // Search IVUsesByStride to find Cond's IVUse if there is one.
    IVStrideUse *CondUse = nullptr;
    ICmpInst *Cond = cast<ICmpInst>(TermBr->getCondition());
    if (!FindIVUserForCond(Cond, CondUse))
      continue;

    // If the trip count is computed in terms of a max (due to ScalarEvolution
    // being unable to find a sufficient guard, for example), change the loop
    // comparison to use SLT or ULT instead of NE.
    // One consequence of doing this now is that it disrupts the count-down
    // optimization. That's not always a bad thing though, because in such
    // cases it may still be worthwhile to avoid a max.
    Cond = OptimizeMax(Cond, CondUse);

    // If this exiting block dominates the latch block, it may also use
    // the post-inc value if it won't be shared with other uses.
    // Check for dominance.
    if (!DT.dominates(ExitingBlock, LatchBlock))
      continue;

    // Conservatively avoid trying to use the post-inc value in non-latch
    // exits if there may be pre-inc users in intervening blocks.
    if (LatchBlock != ExitingBlock)
      for (IVUsers::const_iterator UI = IU.begin(), E = IU.end(); UI != E; ++UI)
        // Test if the use is reachable from the exiting block. This dominator
        // query is a conservative approximation of reachability.
        if (&*UI != CondUse &&
            !DT.properlyDominates(UI->getUser()->getParent(), ExitingBlock)) {
          // Conservatively assume there may be reuse if the quotient of their
          // strides could be a legal scale.
          const SCEV *A = IU.getStride(*CondUse, L);
          const SCEV *B = IU.getStride(*UI, L);
          if (!A || !B) continue;
          if (SE.getTypeSizeInBits(A->getType()) !=
              SE.getTypeSizeInBits(B->getType())) {
            if (SE.getTypeSizeInBits(A->getType()) >
                SE.getTypeSizeInBits(B->getType()))
              B = SE.getSignExtendExpr(B, A->getType());
            else
              A = SE.getSignExtendExpr(A, B->getType());
          }
          if (const SCEVConstant *D =
                dyn_cast_or_null<SCEVConstant>(getExactSDiv(B, A, SE))) {
            const ConstantInt *C = D->getValue();
            // Stride of one or negative one can have reuse with non-addresses.
            if (C->isOne() || C->isAllOnesValue())
              goto decline_post_inc;
            // Avoid weird situations.
            if (C->getValue().getMinSignedBits() >= 64 ||
                C->getValue().isMinSignedValue())
              goto decline_post_inc;
            // Check for possible scaled-address reuse.
            Type *AccessTy = getAccessType(UI->getUser());
            int64_t Scale = C->getSExtValue();
            if (TTI.isLegalAddressingMode(AccessTy, /*BaseGV=*/ nullptr,
                                          /*BaseOffset=*/ 0,
                                          /*HasBaseReg=*/ false, Scale))
              goto decline_post_inc;
            Scale = -Scale;
            if (TTI.isLegalAddressingMode(AccessTy, /*BaseGV=*/ nullptr,
                                          /*BaseOffset=*/ 0,
                                          /*HasBaseReg=*/ false, Scale))
              goto decline_post_inc;
          }
        }

    DEBUG(dbgs() << "  Change loop exiting icmp to use postinc iv: "
                 << *Cond << '\n');

    // It's possible for the setcc instruction to be anywhere in the loop, and
    // possible for it to have multiple users.  If it is not immediately before
    // the exiting block branch, move it.
    if (&*++BasicBlock::iterator(Cond) != TermBr) {
      if (Cond->hasOneUse()) {
        Cond->moveBefore(TermBr);
      } else {
        // Clone the terminating condition and insert into the loopend.
        ICmpInst *OldCond = Cond;
        Cond = cast<ICmpInst>(Cond->clone());
        Cond->setName(L->getHeader()->getName() + ".termcond");
        ExitingBlock->getInstList().insert(TermBr, Cond);

        // Clone the IVUse, as the old use still exists!
        CondUse = &IU.AddUser(Cond, CondUse->getOperandValToReplace());
        TermBr->replaceUsesOfWith(OldCond, Cond);
      }
    }

    // If we get to here, we know that we can transform the setcc instruction to
    // use the post-incremented version of the IV, allowing us to coalesce the
    // live ranges for the IV correctly.
    CondUse->transformToPostInc(L);
    Changed = true;

    PostIncs.insert(Cond);
  decline_post_inc:;
  }

  // Determine an insertion point for the loop induction variable increment. It
  // must dominate all the post-inc comparisons we just set up, and it must
  // dominate the loop latch edge.
  IVIncInsertPos = L->getLoopLatch()->getTerminator();
  for (Instruction *Inst : PostIncs) {
    BasicBlock *BB =
      DT.findNearestCommonDominator(IVIncInsertPos->getParent(),
                                    Inst->getParent());
    if (BB == Inst->getParent())
      IVIncInsertPos = Inst;
    else if (BB != IVIncInsertPos->getParent())
      IVIncInsertPos = BB->getTerminator();
  }
}

/// reconcileNewOffset - Determine if the given use can accommodate a fixup
/// at the given offset and other details. If so, update the use and
/// return true.
bool
LSRInstance::reconcileNewOffset(LSRUse &LU, int64_t NewOffset, bool HasBaseReg,
                                LSRUse::KindType Kind, Type *AccessTy) {
  int64_t NewMinOffset = LU.MinOffset;
  int64_t NewMaxOffset = LU.MaxOffset;
  Type *NewAccessTy = AccessTy;

  // Check for a mismatched kind. It's tempting to collapse mismatched kinds to
  // something conservative, however this can pessimize in the case that one of
  // the uses will have all its uses outside the loop, for example.
  if (LU.Kind != Kind)
    return false;

  // Check for a mismatched access type, and fall back conservatively as needed.
  // TODO: Be less conservative when the type is similar and can use the same
  // addressing modes.
  if (Kind == LSRUse::Address && AccessTy != LU.AccessTy)
    NewAccessTy = Type::getVoidTy(AccessTy->getContext());

  // Conservatively assume HasBaseReg is true for now.
  if (NewOffset < LU.MinOffset) {
    if (!isAlwaysFoldable(TTI, Kind, NewAccessTy, /*BaseGV=*/nullptr,
                          LU.MaxOffset - NewOffset, HasBaseReg))
      return false;
    NewMinOffset = NewOffset;
  } else if (NewOffset > LU.MaxOffset) {
    if (!isAlwaysFoldable(TTI, Kind, NewAccessTy, /*BaseGV=*/nullptr,
                          NewOffset - LU.MinOffset, HasBaseReg))
      return false;
    NewMaxOffset = NewOffset;
  }

  // Update the use.
  LU.MinOffset = NewMinOffset;
  LU.MaxOffset = NewMaxOffset;
  LU.AccessTy = NewAccessTy;
  if (NewOffset != LU.Offsets.back())
    LU.Offsets.push_back(NewOffset);
  return true;
}

/// getUse - Return an LSRUse index and an offset value for a fixup which
/// needs the given expression, with the given kind and optional access type.
/// Either reuse an existing use or create a new one, as needed.
std::pair<size_t, int64_t>
LSRInstance::getUse(const SCEV *&Expr,
                    LSRUse::KindType Kind, Type *AccessTy) {
  const SCEV *Copy = Expr;
  int64_t Offset = ExtractImmediate(Expr, SE);

  // Basic uses can't accept any offset, for example.
  if (!isAlwaysFoldable(TTI, Kind, AccessTy, /*BaseGV=*/ nullptr,
                        Offset, /*HasBaseReg=*/ true)) {
    Expr = Copy;
    Offset = 0;
  }

  std::pair<UseMapTy::iterator, bool> P =
    UseMap.insert(std::make_pair(LSRUse::SCEVUseKindPair(Expr, Kind), 0));
  if (!P.second) {
    // A use already existed with this base.
    size_t LUIdx = P.first->second;
    LSRUse &LU = Uses[LUIdx];
    if (reconcileNewOffset(LU, Offset, /*HasBaseReg=*/true, Kind, AccessTy))
      // Reuse this use.
      return std::make_pair(LUIdx, Offset);
  }

  // Create a new use.
  size_t LUIdx = Uses.size();
  P.first->second = LUIdx;
  Uses.push_back(LSRUse(Kind, AccessTy));
  LSRUse &LU = Uses[LUIdx];

  // We don't need to track redundant offsets, but we don't need to go out
  // of our way here to avoid them.
  if (LU.Offsets.empty() || Offset != LU.Offsets.back())
    LU.Offsets.push_back(Offset);

  LU.MinOffset = Offset;
  LU.MaxOffset = Offset;
  return std::make_pair(LUIdx, Offset);
}

/// DeleteUse - Delete the given use from the Uses list.
void LSRInstance::DeleteUse(LSRUse &LU, size_t LUIdx) {
  if (&LU != &Uses.back())
    std::swap(LU, Uses.back());
  Uses.pop_back();

  // Update RegUses.
  RegUses.SwapAndDropUse(LUIdx, Uses.size());
}

/// FindUseWithFormula - Look for a use distinct from OrigLU which is has
/// a formula that has the same registers as the given formula.
LSRUse *
LSRInstance::FindUseWithSimilarFormula(const Formula &OrigF,
                                       const LSRUse &OrigLU) {
  // Search all uses for the formula. This could be more clever.
  for (size_t LUIdx = 0, NumUses = Uses.size(); LUIdx != NumUses; ++LUIdx) {
    LSRUse &LU = Uses[LUIdx];
    // Check whether this use is close enough to OrigLU, to see whether it's
    // worthwhile looking through its formulae.
    // Ignore ICmpZero uses because they may contain formulae generated by
    // GenerateICmpZeroScales, in which case adding fixup offsets may
    // be invalid.
    if (&LU != &OrigLU &&
        LU.Kind != LSRUse::ICmpZero &&
        LU.Kind == OrigLU.Kind && OrigLU.AccessTy == LU.AccessTy &&
        LU.WidestFixupType == OrigLU.WidestFixupType &&
        LU.HasFormulaWithSameRegs(OrigF)) {
      // Scan through this use's formulae.
      for (const Formula &F : LU.Formulae) {
        // Check to see if this formula has the same registers and symbols
        // as OrigF.
        if (F.BaseRegs == OrigF.BaseRegs &&
            F.ScaledReg == OrigF.ScaledReg &&
            F.BaseGV == OrigF.BaseGV &&
            F.Scale == OrigF.Scale &&
            F.UnfoldedOffset == OrigF.UnfoldedOffset) {
          if (F.BaseOffset == 0)
            return &LU;
          // This is the formula where all the registers and symbols matched;
          // there aren't going to be any others. Since we declined it, we
          // can skip the rest of the formulae and proceed to the next LSRUse.
          break;
        }
      }
    }
  }

  // Nothing looked good.
  return nullptr;
}

void LSRInstance::CollectInterestingTypesAndFactors() {
  SmallSetVector<const SCEV *, 4> Strides;

  // Collect interesting types and strides.
  SmallVector<const SCEV *, 4> Worklist;
  for (const IVStrideUse &U : IU) {
    const SCEV *Expr = IU.getExpr(U);

    // Collect interesting types.
    Types.insert(SE.getEffectiveSCEVType(Expr->getType()));

    // Add strides for mentioned loops.
    Worklist.push_back(Expr);
    do {
      const SCEV *S = Worklist.pop_back_val();
      if (const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(S)) {
        if (AR->getLoop() == L)
          Strides.insert(AR->getStepRecurrence(SE));
        Worklist.push_back(AR->getStart());
      } else if (const SCEVAddExpr *Add = dyn_cast<SCEVAddExpr>(S)) {
        Worklist.append(Add->op_begin(), Add->op_end());
      }
    } while (!Worklist.empty());
  }

  // Compute interesting factors from the set of interesting strides.
  for (SmallSetVector<const SCEV *, 4>::const_iterator
       I = Strides.begin(), E = Strides.end(); I != E; ++I)
    for (SmallSetVector<const SCEV *, 4>::const_iterator NewStrideIter =
         std::next(I); NewStrideIter != E; ++NewStrideIter) {
      const SCEV *OldStride = *I;
      const SCEV *NewStride = *NewStrideIter;

      if (SE.getTypeSizeInBits(OldStride->getType()) !=
          SE.getTypeSizeInBits(NewStride->getType())) {
        if (SE.getTypeSizeInBits(OldStride->getType()) >
            SE.getTypeSizeInBits(NewStride->getType()))
          NewStride = SE.getSignExtendExpr(NewStride, OldStride->getType());
        else
          OldStride = SE.getSignExtendExpr(OldStride, NewStride->getType());
      }
      if (const SCEVConstant *Factor =
            dyn_cast_or_null<SCEVConstant>(getExactSDiv(NewStride, OldStride,
                                                        SE, true))) {
        if (Factor->getValue()->getValue().getMinSignedBits() <= 64)
          Factors.insert(Factor->getValue()->getValue().getSExtValue());
      } else if (const SCEVConstant *Factor =
                   dyn_cast_or_null<SCEVConstant>(getExactSDiv(OldStride,
                                                               NewStride,
                                                               SE, true))) {
        if (Factor->getValue()->getValue().getMinSignedBits() <= 64)
          Factors.insert(Factor->getValue()->getValue().getSExtValue());
      }
    }

  // If all uses use the same type, don't bother looking for truncation-based
  // reuse.
  if (Types.size() == 1)
    Types.clear();

  DEBUG(print_factors_and_types(dbgs()));
}

/// findIVOperand - Helper for CollectChains that finds an IV operand (computed
/// by an AddRec in this loop) within [OI,OE) or returns OE. If IVUsers mapped
/// Instructions to IVStrideUses, we could partially skip this.
static User::op_iterator
findIVOperand(User::op_iterator OI, User::op_iterator OE,
              Loop *L, ScalarEvolution &SE) {
  for(; OI != OE; ++OI) {
    if (Instruction *Oper = dyn_cast<Instruction>(*OI)) {
      if (!SE.isSCEVable(Oper->getType()))
        continue;

      if (const SCEVAddRecExpr *AR =
          dyn_cast<SCEVAddRecExpr>(SE.getSCEV(Oper))) {
        if (AR->getLoop() == L)
          break;
      }
    }
  }
  return OI;
}

/// getWideOperand - IVChain logic must consistenctly peek base TruncInst
/// operands, so wrap it in a convenient helper.
static Value *getWideOperand(Value *Oper) {
  if (TruncInst *Trunc = dyn_cast<TruncInst>(Oper))
    return Trunc->getOperand(0);
  return Oper;
}

/// isCompatibleIVType - Return true if we allow an IV chain to include both
/// types.
static bool isCompatibleIVType(Value *LVal, Value *RVal) {
  Type *LType = LVal->getType();
  Type *RType = RVal->getType();
  return (LType == RType) || (LType->isPointerTy() && RType->isPointerTy());
}

/// getExprBase - Return an approximation of this SCEV expression's "base", or
/// NULL for any constant. Returning the expression itself is
/// conservative. Returning a deeper subexpression is more precise and valid as
/// long as it isn't less complex than another subexpression. For expressions
/// involving multiple unscaled values, we need to return the pointer-type
/// SCEVUnknown. This avoids forming chains across objects, such as:
/// PrevOper==a[i], IVOper==b[i], IVInc==b-a.
///
/// Since SCEVUnknown is the rightmost type, and pointers are the rightmost
/// SCEVUnknown, we simply return the rightmost SCEV operand.
static const SCEV *getExprBase(const SCEV *S) {
  switch (S->getSCEVType()) {
  default: // uncluding scUnknown.
    return S;
  case scConstant:
    return nullptr;
  case scTruncate:
    return getExprBase(cast<SCEVTruncateExpr>(S)->getOperand());
  case scZeroExtend:
    return getExprBase(cast<SCEVZeroExtendExpr>(S)->getOperand());
  case scSignExtend:
    return getExprBase(cast<SCEVSignExtendExpr>(S)->getOperand());
  case scAddExpr: {
    // Skip over scaled operands (scMulExpr) to follow add operands as long as
    // there's nothing more complex.
    // FIXME: not sure if we want to recognize negation.
    const SCEVAddExpr *Add = cast<SCEVAddExpr>(S);
    for (std::reverse_iterator<SCEVAddExpr::op_iterator> I(Add->op_end()),
           E(Add->op_begin()); I != E; ++I) {
      const SCEV *SubExpr = *I;
      if (SubExpr->getSCEVType() == scAddExpr)
        return getExprBase(SubExpr);

      if (SubExpr->getSCEVType() != scMulExpr)
        return SubExpr;
    }
    return S; // all operands are scaled, be conservative.
  }
  case scAddRecExpr:
    return getExprBase(cast<SCEVAddRecExpr>(S)->getStart());
  }
}

/// Return true if the chain increment is profitable to expand into a loop
/// invariant value, which may require its own register. A profitable chain
/// increment will be an offset relative to the same base. We allow such offsets
/// to potentially be used as chain increment as long as it's not obviously
/// expensive to expand using real instructions.
bool IVChain::isProfitableIncrement(const SCEV *OperExpr,
                                    const SCEV *IncExpr,
                                    ScalarEvolution &SE) {
  // Aggressively form chains when -stress-ivchain.
  if (StressIVChain)
    return true;

  // Do not replace a constant offset from IV head with a nonconstant IV
  // increment.
  if (!isa<SCEVConstant>(IncExpr)) {
    const SCEV *HeadExpr = SE.getSCEV(getWideOperand(Incs[0].IVOperand));
    if (isa<SCEVConstant>(SE.getMinusSCEV(OperExpr, HeadExpr)))
      return 0;
  }

  SmallPtrSet<const SCEV*, 8> Processed;
  return !isHighCostExpansion(IncExpr, Processed, SE);
}

/// Return true if the number of registers needed for the chain is estimated to
/// be less than the number required for the individual IV users. First prohibit
/// any IV users that keep the IV live across increments (the Users set should
/// be empty). Next count the number and type of increments in the chain.
///
/// Chaining IVs can lead to considerable code bloat if ISEL doesn't
/// effectively use postinc addressing modes. Only consider it profitable it the
/// increments can be computed in fewer registers when chained.
///
/// TODO: Consider IVInc free if it's already used in another chains.
static bool
isProfitableChain(IVChain &Chain, SmallPtrSetImpl<Instruction*> &Users,
                  ScalarEvolution &SE, const TargetTransformInfo &TTI) {
  if (StressIVChain)
    return true;

  if (!Chain.hasIncs())
    return false;

  if (!Users.empty()) {
    DEBUG(dbgs() << "Chain: " << *Chain.Incs[0].UserInst << " users:\n";
          for (Instruction *Inst : Users) {
            dbgs() << "  " << *Inst << "\n";
          });
    return false;
  }
  assert(!Chain.Incs.empty() && "empty IV chains are not allowed");

  // The chain itself may require a register, so intialize cost to 1.
  int cost = 1;

  // A complete chain likely eliminates the need for keeping the original IV in
  // a register. LSR does not currently know how to form a complete chain unless
  // the header phi already exists.
  if (isa<PHINode>(Chain.tailUserInst())
      && SE.getSCEV(Chain.tailUserInst()) == Chain.Incs[0].IncExpr) {
    --cost;
  }
  const SCEV *LastIncExpr = nullptr;
  unsigned NumConstIncrements = 0;
  unsigned NumVarIncrements = 0;
  unsigned NumReusedIncrements = 0;
  for (const IVInc &Inc : Chain) {
    if (Inc.IncExpr->isZero())
      continue;

    // Incrementing by zero or some constant is neutral. We assume constants can
    // be folded into an addressing mode or an add's immediate operand.
    if (isa<SCEVConstant>(Inc.IncExpr)) {
      ++NumConstIncrements;
      continue;
    }

    if (Inc.IncExpr == LastIncExpr)
      ++NumReusedIncrements;
    else
      ++NumVarIncrements;

    LastIncExpr = Inc.IncExpr;
  }
  // An IV chain with a single increment is handled by LSR's postinc
  // uses. However, a chain with multiple increments requires keeping the IV's
  // value live longer than it needs to be if chained.
  if (NumConstIncrements > 1)
    --cost;

  // Materializing increment expressions in the preheader that didn't exist in
  // the original code may cost a register. For example, sign-extended array
  // indices can produce ridiculous increments like this:
  // IV + ((sext i32 (2 * %s) to i64) + (-1 * (sext i32 %s to i64)))
  cost += NumVarIncrements;

  // Reusing variable increments likely saves a register to hold the multiple of
  // the stride.
  cost -= NumReusedIncrements;

  DEBUG(dbgs() << "Chain: " << *Chain.Incs[0].UserInst << " Cost: " << cost
               << "\n");

  return cost < 0;
}

/// ChainInstruction - Add this IV user to an existing chain or make it the head
/// of a new chain.
void LSRInstance::ChainInstruction(Instruction *UserInst, Instruction *IVOper,
                                   SmallVectorImpl<ChainUsers> &ChainUsersVec) {
  // When IVs are used as types of varying widths, they are generally converted
  // to a wider type with some uses remaining narrow under a (free) trunc.
  Value *const NextIV = getWideOperand(IVOper);
  const SCEV *const OperExpr = SE.getSCEV(NextIV);
  const SCEV *const OperExprBase = getExprBase(OperExpr);

  // Visit all existing chains. Check if its IVOper can be computed as a
  // profitable loop invariant increment from the last link in the Chain.
  unsigned ChainIdx = 0, NChains = IVChainVec.size();
  const SCEV *LastIncExpr = nullptr;
  for (; ChainIdx < NChains; ++ChainIdx) {
    IVChain &Chain = IVChainVec[ChainIdx];

    // Prune the solution space aggressively by checking that both IV operands
    // are expressions that operate on the same unscaled SCEVUnknown. This
    // "base" will be canceled by the subsequent getMinusSCEV call. Checking
    // first avoids creating extra SCEV expressions.
    if (!StressIVChain && Chain.ExprBase != OperExprBase)
      continue;

    Value *PrevIV = getWideOperand(Chain.Incs.back().IVOperand);
    if (!isCompatibleIVType(PrevIV, NextIV))
      continue;

    // A phi node terminates a chain.
    if (isa<PHINode>(UserInst) && isa<PHINode>(Chain.tailUserInst()))
      continue;

    // The increment must be loop-invariant so it can be kept in a register.
    const SCEV *PrevExpr = SE.getSCEV(PrevIV);
    const SCEV *IncExpr = SE.getMinusSCEV(OperExpr, PrevExpr);
    if (!SE.isLoopInvariant(IncExpr, L))
      continue;

    if (Chain.isProfitableIncrement(OperExpr, IncExpr, SE)) {
      LastIncExpr = IncExpr;
      break;
    }
  }
  // If we haven't found a chain, create a new one, unless we hit the max. Don't
  // bother for phi nodes, because they must be last in the chain.
  if (ChainIdx == NChains) {
    if (isa<PHINode>(UserInst))
      return;
    if (NChains >= MaxChains && !StressIVChain) {
      DEBUG(dbgs() << "IV Chain Limit\n");
      return;
    }
    LastIncExpr = OperExpr;
    // IVUsers may have skipped over sign/zero extensions. We don't currently
    // attempt to form chains involving extensions unless they can be hoisted
    // into this loop's AddRec.
    if (!isa<SCEVAddRecExpr>(LastIncExpr))
      return;
    ++NChains;
    IVChainVec.push_back(IVChain(IVInc(UserInst, IVOper, LastIncExpr),
                                 OperExprBase));
    ChainUsersVec.resize(NChains);
    DEBUG(dbgs() << "IV Chain#" << ChainIdx << " Head: (" << *UserInst
                 << ") IV=" << *LastIncExpr << "\n");
  } else {
    DEBUG(dbgs() << "IV Chain#" << ChainIdx << "  Inc: (" << *UserInst
                 << ") IV+" << *LastIncExpr << "\n");
    // Add this IV user to the end of the chain.
    IVChainVec[ChainIdx].add(IVInc(UserInst, IVOper, LastIncExpr));
  }
  IVChain &Chain = IVChainVec[ChainIdx];

  SmallPtrSet<Instruction*,4> &NearUsers = ChainUsersVec[ChainIdx].NearUsers;
  // This chain's NearUsers become FarUsers.
  if (!LastIncExpr->isZero()) {
    ChainUsersVec[ChainIdx].FarUsers.insert(NearUsers.begin(),
                                            NearUsers.end());
    NearUsers.clear();
  }

  // All other uses of IVOperand become near uses of the chain.
  // We currently ignore intermediate values within SCEV expressions, assuming
  // they will eventually be used be the current chain, or can be computed
  // from one of the chain increments. To be more precise we could
  // transitively follow its user and only add leaf IV users to the set.
  for (User *U : IVOper->users()) {
    Instruction *OtherUse = dyn_cast<Instruction>(U);
    if (!OtherUse)
      continue;
    // Uses in the chain will no longer be uses if the chain is formed.
    // Include the head of the chain in this iteration (not Chain.begin()).
    IVChain::const_iterator IncIter = Chain.Incs.begin();
    IVChain::const_iterator IncEnd = Chain.Incs.end();
    for( ; IncIter != IncEnd; ++IncIter) {
      if (IncIter->UserInst == OtherUse)
        break;
    }
    if (IncIter != IncEnd)
      continue;

    if (SE.isSCEVable(OtherUse->getType())
        && !isa<SCEVUnknown>(SE.getSCEV(OtherUse))
        && IU.isIVUserOrOperand(OtherUse)) {
      continue;
    }
    NearUsers.insert(OtherUse);
  }

  // Since this user is part of the chain, it's no longer considered a use
  // of the chain.
  ChainUsersVec[ChainIdx].FarUsers.erase(UserInst);
}

/// CollectChains - Populate the vector of Chains.
///
/// This decreases ILP at the architecture level. Targets with ample registers,
/// multiple memory ports, and no register renaming probably don't want
/// this. However, such targets should probably disable LSR altogether.
///
/// The job of LSR is to make a reasonable choice of induction variables across
/// the loop. Subsequent passes can easily "unchain" computation exposing more
/// ILP *within the loop* if the target wants it.
///
/// Finding the best IV chain is potentially a scheduling problem. Since LSR
/// will not reorder memory operations, it will recognize this as a chain, but
/// will generate redundant IV increments. Ideally this would be corrected later
/// by a smart scheduler:
///        = A[i]
///        = A[i+x]
/// A[i]   =
/// A[i+x] =
///
/// TODO: Walk the entire domtree within this loop, not just the path to the
/// loop latch. This will discover chains on side paths, but requires
/// maintaining multiple copies of the Chains state.
void LSRInstance::CollectChains() {
  DEBUG(dbgs() << "Collecting IV Chains.\n");
  SmallVector<ChainUsers, 8> ChainUsersVec;

  SmallVector<BasicBlock *,8> LatchPath;
  BasicBlock *LoopHeader = L->getHeader();
  for (DomTreeNode *Rung = DT.getNode(L->getLoopLatch());
       Rung->getBlock() != LoopHeader; Rung = Rung->getIDom()) {
    LatchPath.push_back(Rung->getBlock());
  }
  LatchPath.push_back(LoopHeader);

  // Walk the instruction stream from the loop header to the loop latch.
  for (SmallVectorImpl<BasicBlock *>::reverse_iterator
         BBIter = LatchPath.rbegin(), BBEnd = LatchPath.rend();
       BBIter != BBEnd; ++BBIter) {
    for (BasicBlock::iterator I = (*BBIter)->begin(), E = (*BBIter)->end();
         I != E; ++I) {
      // Skip instructions that weren't seen by IVUsers analysis.
      if (isa<PHINode>(I) || !IU.isIVUserOrOperand(I))
        continue;

      // Ignore users that are part of a SCEV expression. This way we only
      // consider leaf IV Users. This effectively rediscovers a portion of
      // IVUsers analysis but in program order this time.
      if (SE.isSCEVable(I->getType()) && !isa<SCEVUnknown>(SE.getSCEV(I)))
        continue;

      // Remove this instruction from any NearUsers set it may be in.
      for (unsigned ChainIdx = 0, NChains = IVChainVec.size();
           ChainIdx < NChains; ++ChainIdx) {
        ChainUsersVec[ChainIdx].NearUsers.erase(I);
      }
      // Search for operands that can be chained.
      SmallPtrSet<Instruction*, 4> UniqueOperands;
      User::op_iterator IVOpEnd = I->op_end();
      User::op_iterator IVOpIter = findIVOperand(I->op_begin(), IVOpEnd, L, SE);
      while (IVOpIter != IVOpEnd) {
        Instruction *IVOpInst = cast<Instruction>(*IVOpIter);
        if (UniqueOperands.insert(IVOpInst).second)
          ChainInstruction(I, IVOpInst, ChainUsersVec);
        IVOpIter = findIVOperand(std::next(IVOpIter), IVOpEnd, L, SE);
      }
    } // Continue walking down the instructions.
  } // Continue walking down the domtree.
  // Visit phi backedges to determine if the chain can generate the IV postinc.
  for (BasicBlock::iterator I = L->getHeader()->begin();
       PHINode *PN = dyn_cast<PHINode>(I); ++I) {
    if (!SE.isSCEVable(PN->getType()))
      continue;

    Instruction *IncV =
      dyn_cast<Instruction>(PN->getIncomingValueForBlock(L->getLoopLatch()));
    if (IncV)
      ChainInstruction(PN, IncV, ChainUsersVec);
  }
  // Remove any unprofitable chains.
  unsigned ChainIdx = 0;
  for (unsigned UsersIdx = 0, NChains = IVChainVec.size();
       UsersIdx < NChains; ++UsersIdx) {
    if (!isProfitableChain(IVChainVec[UsersIdx],
                           ChainUsersVec[UsersIdx].FarUsers, SE, TTI))
      continue;
    // Preserve the chain at UsesIdx.
    if (ChainIdx != UsersIdx)
      IVChainVec[ChainIdx] = IVChainVec[UsersIdx];
    FinalizeChain(IVChainVec[ChainIdx]);
    ++ChainIdx;
  }
  IVChainVec.resize(ChainIdx);
}

void LSRInstance::FinalizeChain(IVChain &Chain) {
  assert(!Chain.Incs.empty() && "empty IV chains are not allowed");
  DEBUG(dbgs() << "Final Chain: " << *Chain.Incs[0].UserInst << "\n");

  for (const IVInc &Inc : Chain) {
    DEBUG(dbgs() << "        Inc: " << Inc.UserInst << "\n");
    auto UseI = std::find(Inc.UserInst->op_begin(), Inc.UserInst->op_end(),
                          Inc.IVOperand);
    assert(UseI != Inc.UserInst->op_end() && "cannot find IV operand");
    IVIncSet.insert(UseI);
  }
}

/// Return true if the IVInc can be folded into an addressing mode.
static bool canFoldIVIncExpr(const SCEV *IncExpr, Instruction *UserInst,
                             Value *Operand, const TargetTransformInfo &TTI) {
  const SCEVConstant *IncConst = dyn_cast<SCEVConstant>(IncExpr);
  if (!IncConst || !isAddressUse(UserInst, Operand))
    return false;

  if (IncConst->getValue()->getValue().getMinSignedBits() > 64)
    return false;

  int64_t IncOffset = IncConst->getValue()->getSExtValue();
  if (!isAlwaysFoldable(TTI, LSRUse::Address,
                        getAccessType(UserInst), /*BaseGV=*/ nullptr,
                        IncOffset, /*HaseBaseReg=*/ false))
    return false;

  return true;
}

/// GenerateIVChains - Generate an add or subtract for each IVInc in a chain to
/// materialize the IV user's operand from the previous IV user's operand.
void LSRInstance::GenerateIVChain(const IVChain &Chain, SCEVExpander &Rewriter,
                                  SmallVectorImpl<WeakTrackingVH> &DeadInsts) {
  // Find the new IVOperand for the head of the chain. It may have been replaced
  // by LSR.
  const IVInc &Head = Chain.Incs[0];
  User::op_iterator IVOpEnd = Head.UserInst->op_end();
  // findIVOperand returns IVOpEnd if it can no longer find a valid IV user.
  User::op_iterator IVOpIter = findIVOperand(Head.UserInst->op_begin(),
                                             IVOpEnd, L, SE);
  Value *IVSrc = nullptr;
  while (IVOpIter != IVOpEnd) {
    IVSrc = getWideOperand(*IVOpIter);

    // If this operand computes the expression that the chain needs, we may use
    // it. (Check this after setting IVSrc which is used below.)
    //
    // Note that if Head.IncExpr is wider than IVSrc, then this phi is too
    // narrow for the chain, so we can no longer use it. We do allow using a
    // wider phi, assuming the LSR checked for free truncation. In that case we
    // should already have a truncate on this operand such that
    // getSCEV(IVSrc) == IncExpr.
    if (SE.getSCEV(*IVOpIter) == Head.IncExpr
        || SE.getSCEV(IVSrc) == Head.IncExpr) {
      break;
    }
    IVOpIter = findIVOperand(std::next(IVOpIter), IVOpEnd, L, SE);
  }
  if (IVOpIter == IVOpEnd) {
    // Gracefully give up on this chain.
    DEBUG(dbgs() << "Concealed chain head: " << *Head.UserInst << "\n");
    return;
  }

  DEBUG(dbgs() << "Generate chain at: " << *IVSrc << "\n");
  Type *IVTy = IVSrc->getType();
  Type *IntTy = SE.getEffectiveSCEVType(IVTy);
  const SCEV *LeftOverExpr = nullptr;
  for (const IVInc &Inc : Chain) {
    Instruction *InsertPt = Inc.UserInst;
    if (isa<PHINode>(InsertPt))
      InsertPt = L->getLoopLatch()->getTerminator();

    // IVOper will replace the current IV User's operand. IVSrc is the IV
    // value currently held in a register.
    Value *IVOper = IVSrc;
    if (!Inc.IncExpr->isZero()) {
      // IncExpr was the result of subtraction of two narrow values, so must
      // be signed.
      const SCEV *IncExpr = SE.getNoopOrSignExtend(Inc.IncExpr, IntTy);
      LeftOverExpr = LeftOverExpr ?
        SE.getAddExpr(LeftOverExpr, IncExpr) : IncExpr;
    }
    if (LeftOverExpr && !LeftOverExpr->isZero()) {
      // Expand the IV increment.
      Rewriter.clearPostInc();
      Value *IncV = Rewriter.expandCodeFor(LeftOverExpr, IntTy, InsertPt);
      const SCEV *IVOperExpr = SE.getAddExpr(SE.getUnknown(IVSrc),
                                             SE.getUnknown(IncV));
      IVOper = Rewriter.expandCodeFor(IVOperExpr, IVTy, InsertPt);

      // If an IV increment can't be folded, use it as the next IV value.
      if (!canFoldIVIncExpr(LeftOverExpr, Inc.UserInst, Inc.IVOperand, TTI)) {
        assert(IVTy == IVOper->getType() && "inconsistent IV increment type");
        IVSrc = IVOper;
        LeftOverExpr = nullptr;
      }
    }
    Type *OperTy = Inc.IVOperand->getType();
    if (IVTy != OperTy) {
      assert(SE.getTypeSizeInBits(IVTy) >= SE.getTypeSizeInBits(OperTy) &&
             "cannot extend a chained IV");
      IRBuilder<> Builder(InsertPt);
      IVOper = Builder.CreateTruncOrBitCast(IVOper, OperTy, "lsr.chain");
    }
    Inc.UserInst->replaceUsesOfWith(Inc.IVOperand, IVOper);
    DeadInsts.emplace_back(Inc.IVOperand);
  }
  // If LSR created a new, wider phi, we may also replace its postinc. We only
  // do this if we also found a wide value for the head of the chain.
  if (isa<PHINode>(Chain.tailUserInst())) {
    for (BasicBlock::iterator I = L->getHeader()->begin();
         PHINode *Phi = dyn_cast<PHINode>(I); ++I) {
      if (!isCompatibleIVType(Phi, IVSrc))
        continue;
      Instruction *PostIncV = dyn_cast<Instruction>(
        Phi->getIncomingValueForBlock(L->getLoopLatch()));
      if (!PostIncV || (SE.getSCEV(PostIncV) != SE.getSCEV(IVSrc)))
        continue;
      Value *IVOper = IVSrc;
      Type *PostIncTy = PostIncV->getType();
      if (IVTy != PostIncTy) {
        assert(PostIncTy->isPointerTy() && "mixing int/ptr IV types");
        IRBuilder<> Builder(L->getLoopLatch()->getTerminator());
        Builder.SetCurrentDebugLocation(PostIncV->getDebugLoc());
        IVOper = Builder.CreatePointerCast(IVSrc, PostIncTy, "lsr.chain");
      }
      Phi->replaceUsesOfWith(PostIncV, IVOper);
      DeadInsts.emplace_back(PostIncV);
    }
  }
}

void LSRInstance::CollectFixupsAndInitialFormulae() {
  for (const IVStrideUse &U : IU) {
    Instruction *UserInst = U.getUser();
    // Skip IV users that are part of profitable IV Chains.
    User::op_iterator UseI = std::find(UserInst->op_begin(), UserInst->op_end(),
                                       U.getOperandValToReplace());
    assert(UseI != UserInst->op_end() && "cannot find IV operand");
    if (IVIncSet.count(UseI))
      continue;

    // Record the uses.
    LSRFixup &LF = getNewFixup();
    LF.UserInst = UserInst;
    LF.OperandValToReplace = U.getOperandValToReplace();
    LF.PostIncLoops = U.getPostIncLoops();

    LSRUse::KindType Kind = LSRUse::Basic;
    Type *AccessTy = nullptr;
    if (isAddressUse(LF.UserInst, LF.OperandValToReplace)) {
      Kind = LSRUse::Address;
      AccessTy = getAccessType(LF.UserInst);
    }

    const SCEV *S = IU.getExpr(U);

    // Equality (== and !=) ICmps are special. We can rewrite (i == N) as
    // (N - i == 0), and this allows (N - i) to be the expression that we work
    // with rather than just N or i, so we can consider the register
    // requirements for both N and i at the same time. Limiting this code to
    // equality icmps is not a problem because all interesting loops use
    // equality icmps, thanks to IndVarSimplify.
    if (ICmpInst *CI = dyn_cast<ICmpInst>(LF.UserInst))
      if (CI->isEquality()) {
        // Swap the operands if needed to put the OperandValToReplace on the
        // left, for consistency.
        Value *NV = CI->getOperand(1);
        if (NV == LF.OperandValToReplace) {
          CI->setOperand(1, CI->getOperand(0));
          CI->setOperand(0, NV);
          NV = CI->getOperand(1);
          Changed = true;
        }

        // x == y  -->  x - y == 0
        const SCEV *N = SE.getSCEV(NV);
        if (SE.isLoopInvariant(N, L) && isSafeToExpand(N, SE)) {
          // S is normalized, so normalize N before folding it into S
          // to keep the result normalized.
          N = TransformForPostIncUse(Normalize, N, CI, nullptr,
                                     LF.PostIncLoops, SE, DT);
          Kind = LSRUse::ICmpZero;
          S = SE.getMinusSCEV(N, S);
        }

        // -1 and the negations of all interesting strides (except the negation
        // of -1) are now also interesting.
        for (size_t i = 0, e = Factors.size(); i != e; ++i)
          if (Factors[i] != -1)
            Factors.insert(-(uint64_t)Factors[i]);
        Factors.insert(-1);
      }

    // Set up the initial formula for this use.
    std::pair<size_t, int64_t> P = getUse(S, Kind, AccessTy);
    LF.LUIdx = P.first;
    LF.Offset = P.second;
    LSRUse &LU = Uses[LF.LUIdx];
    LU.AllFixupsOutsideLoop &= LF.isUseFullyOutsideLoop(L);
    if (!LU.WidestFixupType ||
        SE.getTypeSizeInBits(LU.WidestFixupType) <
        SE.getTypeSizeInBits(LF.OperandValToReplace->getType()))
      LU.WidestFixupType = LF.OperandValToReplace->getType();

    // If this is the first use of this LSRUse, give it a formula.
    if (LU.Formulae.empty()) {
      InsertInitialFormula(S, LU, LF.LUIdx);
      CountRegisters(LU.Formulae.back(), LF.LUIdx);
    }
  }

  DEBUG(print_fixups(dbgs()));
}

/// InsertInitialFormula - Insert a formula for the given expression into
/// the given use, separating out loop-variant portions from loop-invariant
/// and loop-computable portions.
void
LSRInstance::InsertInitialFormula(const SCEV *S, LSRUse &LU, size_t LUIdx) {
  // Mark uses whose expressions cannot be expanded.
  if (!isSafeToExpand(S, SE))
    LU.RigidFormula = true;

  Formula F;
  F.InitialMatch(S, L, SE);
  bool Inserted = InsertFormula(LU, LUIdx, F);
  assert(Inserted && "Initial formula already exists!"); (void)Inserted;
}

/// InsertSupplementalFormula - Insert a simple single-register formula for
/// the given expression into the given use.
void
LSRInstance::InsertSupplementalFormula(const SCEV *S,
                                       LSRUse &LU, size_t LUIdx) {
  Formula F;
  F.BaseRegs.push_back(S);
  F.HasBaseReg = true;
  bool Inserted = InsertFormula(LU, LUIdx, F);
  assert(Inserted && "Supplemental formula already exists!"); (void)Inserted;
}

/// CountRegisters - Note which registers are used by the given formula,
/// updating RegUses.
void LSRInstance::CountRegisters(const Formula &F, size_t LUIdx) {
  if (F.ScaledReg)
    RegUses.CountRegister(F.ScaledReg, LUIdx);
  for (const SCEV *BaseReg : F.BaseRegs)
    RegUses.CountRegister(BaseReg, LUIdx);
}

/// InsertFormula - If the given formula has not yet been inserted, add it to
/// the list, and return true. Return false otherwise.
bool LSRInstance::InsertFormula(LSRUse &LU, unsigned LUIdx, const Formula &F) {
  // Do not insert formula that we will not be able to expand.
  assert(isLegalUse(TTI, LU.MinOffset, LU.MaxOffset, LU.Kind, LU.AccessTy, F) &&
         "Formula is illegal");
  if (!LU.InsertFormula(F))
    return false;

  CountRegisters(F, LUIdx);
  return true;
}

/// CollectLoopInvariantFixupsAndFormulae - Check for other uses of
/// loop-invariant values which we're tracking. These other uses will pin these
/// values in registers, making them less profitable for elimination.
/// TODO: This currently misses non-constant addrec step registers.
/// TODO: Should this give more weight to users inside the loop?
void
LSRInstance::CollectLoopInvariantFixupsAndFormulae() {
  SmallVector<const SCEV *, 8> Worklist(RegUses.begin(), RegUses.end());
  SmallPtrSet<const SCEV *, 32> Visited;

  while (!Worklist.empty()) {
    const SCEV *S = Worklist.pop_back_val();

    // Don't process the same SCEV twice
    if (!Visited.insert(S).second)
      continue;

    if (const SCEVNAryExpr *N = dyn_cast<SCEVNAryExpr>(S))
      Worklist.append(N->op_begin(), N->op_end());
    else if (const SCEVCastExpr *C = dyn_cast<SCEVCastExpr>(S))
      Worklist.push_back(C->getOperand());
    else if (const SCEVUDivExpr *D = dyn_cast<SCEVUDivExpr>(S)) {
      Worklist.push_back(D->getLHS());
      Worklist.push_back(D->getRHS());
    } else if (const SCEVUnknown *US = dyn_cast<SCEVUnknown>(S)) {
      const Value *V = US->getValue();
      if (const Instruction *Inst = dyn_cast<Instruction>(V)) {
        // Look for instructions defined outside the loop.
        if (L->contains(Inst)) continue;
      } else if (isa<UndefValue>(V))
        // Undef doesn't have a live range, so it doesn't matter.
        continue;
      for (const Use &U : V->uses()) {
        const Instruction *UserInst = dyn_cast<Instruction>(U.getUser());
        // Ignore non-instructions.
        if (!UserInst)
          continue;
        // Ignore instructions in other functions (as can happen with
        // Constants).
        if (UserInst->getParent()->getParent() != L->getHeader()->getParent())
          continue;
        // Ignore instructions not dominated by the loop.
        const BasicBlock *UseBB = !isa<PHINode>(UserInst) ?
          UserInst->getParent() :
          cast<PHINode>(UserInst)->getIncomingBlock(
            PHINode::getIncomingValueNumForOperand(U.getOperandNo()));
        if (!DT.dominates(L->getHeader(), UseBB))
          continue;
        // Ignore uses which are part of other SCEV expressions, to avoid
        // analyzing them multiple times.
        if (SE.isSCEVable(UserInst->getType())) {
          const SCEV *UserS = SE.getSCEV(const_cast<Instruction *>(UserInst));
          // If the user is a no-op, look through to its uses.
          if (!isa<SCEVUnknown>(UserS))
            continue;
          if (UserS == US) {
            Worklist.push_back(
              SE.getUnknown(const_cast<Instruction *>(UserInst)));
            continue;
          }
        }
        // Ignore icmp instructions which are already being analyzed.
        if (const ICmpInst *ICI = dyn_cast<ICmpInst>(UserInst)) {
          unsigned OtherIdx = !U.getOperandNo();
          Value *OtherOp = const_cast<Value *>(ICI->getOperand(OtherIdx));
          if (SE.hasComputableLoopEvolution(SE.getSCEV(OtherOp), L))
            continue;
        }

        LSRFixup &LF = getNewFixup();
        LF.UserInst = const_cast<Instruction *>(UserInst);
        LF.OperandValToReplace = U;
        std::pair<size_t, int64_t> P = getUse(S, LSRUse::Basic, nullptr);
        LF.LUIdx = P.first;
        LF.Offset = P.second;
        LSRUse &LU = Uses[LF.LUIdx];
        LU.AllFixupsOutsideLoop &= LF.isUseFullyOutsideLoop(L);
        if (!LU.WidestFixupType ||
            SE.getTypeSizeInBits(LU.WidestFixupType) <
            SE.getTypeSizeInBits(LF.OperandValToReplace->getType()))
          LU.WidestFixupType = LF.OperandValToReplace->getType();
        InsertSupplementalFormula(US, LU, LF.LUIdx);
        CountRegisters(LU.Formulae.back(), Uses.size() - 1);
        break;
      }
    }
  }
}

/// CollectSubexprs - Split S into subexpressions which can be pulled out into
/// separate registers. If C is non-null, multiply each subexpression by C.
///
/// Return remainder expression after factoring the subexpressions captured by
/// Ops. If Ops is complete, return NULL.
static const SCEV *CollectSubexprs(const SCEV *S, const SCEVConstant *C,
                                   SmallVectorImpl<const SCEV *> &Ops,
                                   const Loop *L,
                                   ScalarEvolution &SE,
                                   unsigned Depth = 0) {
  // Arbitrarily cap recursion to protect compile time.
  if (Depth >= 3)
    return S;

  if (const SCEVAddExpr *Add = dyn_cast<SCEVAddExpr>(S)) {
    // Break out add operands.
    for (const SCEV *S : Add->operands()) {
      const SCEV *Remainder = CollectSubexprs(S, C, Ops, L, SE, Depth+1);
      if (Remainder)
        Ops.push_back(C ? SE.getMulExpr(C, Remainder) : Remainder);
    }
    return nullptr;
  } else if (const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(S)) {
    // Split a non-zero base out of an addrec.
    if (AR->getStart()->isZero())
      return S;

    const SCEV *Remainder = CollectSubexprs(AR->getStart(),
                                            C, Ops, L, SE, Depth+1);
    // Split the non-zero AddRec unless it is part of a nested recurrence that
    // does not pertain to this loop.
    if (Remainder && (AR->getLoop() == L || !isa<SCEVAddRecExpr>(Remainder))) {
      Ops.push_back(C ? SE.getMulExpr(C, Remainder) : Remainder);
      Remainder = nullptr;
    }
    if (Remainder != AR->getStart()) {
      if (!Remainder)
        Remainder = SE.getConstant(AR->getType(), 0);
      return SE.getAddRecExpr(Remainder,
                              AR->getStepRecurrence(SE),
                              AR->getLoop(),
                              //FIXME: AR->getNoWrapFlags(SCEV::FlagNW)
                              SCEV::FlagAnyWrap);
    }
  } else if (const SCEVMulExpr *Mul = dyn_cast<SCEVMulExpr>(S)) {
    // Break (C * (a + b + c)) into C*a + C*b + C*c.
    if (Mul->getNumOperands() != 2)
      return S;
    if (const SCEVConstant *Op0 =
        dyn_cast<SCEVConstant>(Mul->getOperand(0))) {
      C = C ? cast<SCEVConstant>(SE.getMulExpr(C, Op0)) : Op0;
      const SCEV *Remainder =
        CollectSubexprs(Mul->getOperand(1), C, Ops, L, SE, Depth+1);
      if (Remainder)
        Ops.push_back(SE.getMulExpr(C, Remainder));
      return nullptr;
    }
  }
  return S;
}

/// \brief Helper function for LSRInstance::GenerateReassociations.
void LSRInstance::GenerateReassociationsImpl(LSRUse &LU, unsigned LUIdx,
                                             const Formula &Base,
                                             unsigned Depth, size_t Idx,
                                             bool IsScaledReg) {
  const SCEV *BaseReg = IsScaledReg ? Base.ScaledReg : Base.BaseRegs[Idx];
  SmallVector<const SCEV *, 8> AddOps;
  const SCEV *Remainder = CollectSubexprs(BaseReg, nullptr, AddOps, L, SE);
  if (Remainder)
    AddOps.push_back(Remainder);

  if (AddOps.size() == 1)
    return;

  for (SmallVectorImpl<const SCEV *>::const_iterator J = AddOps.begin(),
                                                     JE = AddOps.end();
       J != JE; ++J) {

    // Loop-variant "unknown" values are uninteresting; we won't be able to
    // do anything meaningful with them.
    if (isa<SCEVUnknown>(*J) && !SE.isLoopInvariant(*J, L))
      continue;

    // Don't pull a constant into a register if the constant could be folded
    // into an immediate field.
    if (isAlwaysFoldable(TTI, SE, LU.MinOffset, LU.MaxOffset, LU.Kind,
                         LU.AccessTy, *J, Base.getNumRegs() > 1))
      continue;

    // Collect all operands except *J.
    SmallVector<const SCEV *, 8> InnerAddOps(
        ((const SmallVector<const SCEV *, 8> &)AddOps).begin(), J);
    InnerAddOps.append(std::next(J),
                       ((const SmallVector<const SCEV *, 8> &)AddOps).end());

    // Don't leave just a constant behind in a register if the constant could
    // be folded into an immediate field.
    if (InnerAddOps.size() == 1 &&
        isAlwaysFoldable(TTI, SE, LU.MinOffset, LU.MaxOffset, LU.Kind,
                         LU.AccessTy, InnerAddOps[0], Base.getNumRegs() > 1))
      continue;

    const SCEV *InnerSum = SE.getAddExpr(InnerAddOps);
    if (InnerSum->isZero())
      continue;
    Formula F = Base;

    // Add the remaining pieces of the add back into the new formula.
    const SCEVConstant *InnerSumSC = dyn_cast<SCEVConstant>(InnerSum);
    if (InnerSumSC && SE.getTypeSizeInBits(InnerSumSC->getType()) <= 64 &&
        TTI.isLegalAddImmediate((uint64_t)F.UnfoldedOffset +
                                InnerSumSC->getValue()->getZExtValue())) {
      F.UnfoldedOffset =
          (uint64_t)F.UnfoldedOffset + InnerSumSC->getValue()->getZExtValue();
      if (IsScaledReg)
        F.ScaledReg = nullptr;
      else
        F.BaseRegs.erase(F.BaseRegs.begin() + Idx);
    } else if (IsScaledReg)
      F.ScaledReg = InnerSum;
    else
      F.BaseRegs[Idx] = InnerSum;

    // Add J as its own register, or an unfolded immediate.
    const SCEVConstant *SC = dyn_cast<SCEVConstant>(*J);
    if (SC && SE.getTypeSizeInBits(SC->getType()) <= 64 &&
        TTI.isLegalAddImmediate((uint64_t)F.UnfoldedOffset +
                                SC->getValue()->getZExtValue()))
      F.UnfoldedOffset =
          (uint64_t)F.UnfoldedOffset + SC->getValue()->getZExtValue();
    else
      F.BaseRegs.push_back(*J);
    // We may have changed the number of register in base regs, adjust the
    // formula accordingly.
    F.Canonicalize();

    if (InsertFormula(LU, LUIdx, F))
      // If that formula hadn't been seen before, recurse to find more like
      // it.
      GenerateReassociations(LU, LUIdx, LU.Formulae.back(), Depth + 1);
  }
}

/// GenerateReassociations - Split out subexpressions from adds and the bases of
/// addrecs.
void LSRInstance::GenerateReassociations(LSRUse &LU, unsigned LUIdx,
                                         Formula Base, unsigned Depth) {
  assert(Base.isCanonical() && "Input must be in the canonical form");
  // Arbitrarily cap recursion to protect compile time.
  if (Depth >= 3)
    return;

  for (size_t i = 0, e = Base.BaseRegs.size(); i != e; ++i)
    GenerateReassociationsImpl(LU, LUIdx, Base, Depth, i);

  if (Base.Scale == 1)
    GenerateReassociationsImpl(LU, LUIdx, Base, Depth,
                               /* Idx */ -1, /* IsScaledReg */ true);
}

/// GenerateCombinations - Generate a formula consisting of all of the
/// loop-dominating registers added into a single register.
void LSRInstance::GenerateCombinations(LSRUse &LU, unsigned LUIdx,
                                       Formula Base) {
  // This method is only interesting on a plurality of registers.
  if (Base.BaseRegs.size() + (Base.Scale == 1) <= 1)
    return;

  // Flatten the representation, i.e., reg1 + 1*reg2 => reg1 + reg2, before
  // processing the formula.
  Base.Unscale();
  Formula F = Base;
  F.BaseRegs.clear();
  SmallVector<const SCEV *, 4> Ops;
  for (const SCEV *BaseReg : Base.BaseRegs) {
    if (SE.properlyDominates(BaseReg, L->getHeader()) &&
        !SE.hasComputableLoopEvolution(BaseReg, L))
      Ops.push_back(BaseReg);
    else
      F.BaseRegs.push_back(BaseReg);
  }
  if (Ops.size() > 1) {
    const SCEV *Sum = SE.getAddExpr(Ops);
    // TODO: If Sum is zero, it probably means ScalarEvolution missed an
    // opportunity to fold something. For now, just ignore such cases
    // rather than proceed with zero in a register.
    if (!Sum->isZero()) {
      F.BaseRegs.push_back(Sum);
      F.Canonicalize();
      (void)InsertFormula(LU, LUIdx, F);
    }
  }
}

/// \brief Helper function for LSRInstance::GenerateSymbolicOffsets.
void LSRInstance::GenerateSymbolicOffsetsImpl(LSRUse &LU, unsigned LUIdx,
                                              const Formula &Base, size_t Idx,
                                              bool IsScaledReg) {
  const SCEV *G = IsScaledReg ? Base.ScaledReg : Base.BaseRegs[Idx];
  GlobalValue *GV = ExtractSymbol(G, SE);
  if (G->isZero() || !GV)
    return;
  Formula F = Base;
  F.BaseGV = GV;
  if (!isLegalUse(TTI, LU.MinOffset, LU.MaxOffset, LU.Kind, LU.AccessTy, F))
    return;
  if (IsScaledReg)
    F.ScaledReg = G;
  else
    F.BaseRegs[Idx] = G;
  (void)InsertFormula(LU, LUIdx, F);
}

/// GenerateSymbolicOffsets - Generate reuse formulae using symbolic offsets.
void LSRInstance::GenerateSymbolicOffsets(LSRUse &LU, unsigned LUIdx,
                                          Formula Base) {
  // We can't add a symbolic offset if the address already contains one.
  if (Base.BaseGV) return;

  for (size_t i = 0, e = Base.BaseRegs.size(); i != e; ++i)
    GenerateSymbolicOffsetsImpl(LU, LUIdx, Base, i);
  if (Base.Scale == 1)
    GenerateSymbolicOffsetsImpl(LU, LUIdx, Base, /* Idx */ -1,
                                /* IsScaledReg */ true);
}

/// \brief Helper function for LSRInstance::GenerateConstantOffsets.
void LSRInstance::GenerateConstantOffsetsImpl(
    LSRUse &LU, unsigned LUIdx, const Formula &Base,
    const SmallVectorImpl<int64_t> &Worklist, size_t Idx, bool IsScaledReg) {
  const SCEV *G = IsScaledReg ? Base.ScaledReg : Base.BaseRegs[Idx];
  for (int64_t Offset : Worklist) {
    Formula F = Base;
    F.BaseOffset = (uint64_t)Base.BaseOffset - Offset;
    if (isLegalUse(TTI, LU.MinOffset - Offset, LU.MaxOffset - Offset, LU.Kind,
                   LU.AccessTy, F)) {
      // Add the offset to the base register.
      const SCEV *NewG = SE.getAddExpr(SE.getConstant(G->getType(), Offset), G);
      // If it cancelled out, drop the base register, otherwise update it.
      if (NewG->isZero()) {
        if (IsScaledReg) {
          F.Scale = 0;
          F.ScaledReg = nullptr;
        } else
          F.DeleteBaseReg(F.BaseRegs[Idx]);
        F.Canonicalize();
      } else if (IsScaledReg)
        F.ScaledReg = NewG;
      else
        F.BaseRegs[Idx] = NewG;

      (void)InsertFormula(LU, LUIdx, F);
    }
  }

  int64_t Imm = ExtractImmediate(G, SE);
  if (G->isZero() || Imm == 0)
    return;
  Formula F = Base;
  F.BaseOffset = (uint64_t)F.BaseOffset + Imm;
  if (!isLegalUse(TTI, LU.MinOffset, LU.MaxOffset, LU.Kind, LU.AccessTy, F))
    return;
  if (IsScaledReg)
    F.ScaledReg = G;
  else
    F.BaseRegs[Idx] = G;
  (void)InsertFormula(LU, LUIdx, F);
}

/// GenerateConstantOffsets - Generate reuse formulae using symbolic offsets.
void LSRInstance::GenerateConstantOffsets(LSRUse &LU, unsigned LUIdx,
                                          Formula Base) {
  // TODO: For now, just add the min and max offset, because it usually isn't
  // worthwhile looking at everything inbetween.
  SmallVector<int64_t, 2> Worklist;
  Worklist.push_back(LU.MinOffset);
  if (LU.MaxOffset != LU.MinOffset)
    Worklist.push_back(LU.MaxOffset);

  for (size_t i = 0, e = Base.BaseRegs.size(); i != e; ++i)
    GenerateConstantOffsetsImpl(LU, LUIdx, Base, Worklist, i);
  if (Base.Scale == 1)
    GenerateConstantOffsetsImpl(LU, LUIdx, Base, Worklist, /* Idx */ -1,
                                /* IsScaledReg */ true);
}

/// GenerateICmpZeroScales - For ICmpZero, check to see if we can scale up
/// the comparison. For example, x == y -> x*c == y*c.
void LSRInstance::GenerateICmpZeroScales(LSRUse &LU, unsigned LUIdx,
                                         Formula Base) {
  if (LU.Kind != LSRUse::ICmpZero) return;

  // Determine the integer type for the base formula.
  Type *IntTy = Base.getType();
  if (!IntTy) return;
  if (SE.getTypeSizeInBits(IntTy) > 64) return;

  // Don't do this if there is more than one offset.
  if (LU.MinOffset != LU.MaxOffset) return;

  assert(!Base.BaseGV && "ICmpZero use is not legal!");

  // Check each interesting stride.
  for (int64_t Factor : Factors) {
    // Check that the multiplication doesn't overflow.
    if (Base.BaseOffset == INT64_MIN && Factor == -1)
      continue;
    int64_t NewBaseOffset = (uint64_t)Base.BaseOffset * Factor;
    if (NewBaseOffset / Factor != Base.BaseOffset)
      continue;
    // If the offset will be truncated at this use, check that it is in bounds.
    if (!IntTy->isPointerTy() &&
        !ConstantInt::isValueValidForType(IntTy, NewBaseOffset))
      continue;

    // Check that multiplying with the use offset doesn't overflow.
    int64_t Offset = LU.MinOffset;
    if (Offset == INT64_MIN && Factor == -1)
      continue;
    Offset = (uint64_t)Offset * Factor;
    if (Offset / Factor != LU.MinOffset)
      continue;
    // If the offset will be truncated at this use, check that it is in bounds.
    if (!IntTy->isPointerTy() &&
        !ConstantInt::isValueValidForType(IntTy, Offset))
      continue;

    Formula F = Base;
    F.BaseOffset = NewBaseOffset;

    // Check that this scale is legal.
    if (!isLegalUse(TTI, Offset, Offset, LU.Kind, LU.AccessTy, F))
      continue;

    // Compensate for the use having MinOffset built into it.
    F.BaseOffset = (uint64_t)F.BaseOffset + Offset - LU.MinOffset;

    const SCEV *FactorS = SE.getConstant(IntTy, Factor);

    // Check that multiplying with each base register doesn't overflow.
    for (size_t i = 0, e = F.BaseRegs.size(); i != e; ++i) {
      F.BaseRegs[i] = SE.getMulExpr(F.BaseRegs[i], FactorS);
      if (getExactSDiv(F.BaseRegs[i], FactorS, SE) != Base.BaseRegs[i])
        goto next;
    }

    // Check that multiplying with the scaled register doesn't overflow.
    if (F.ScaledReg) {
      F.ScaledReg = SE.getMulExpr(F.ScaledReg, FactorS);
      if (getExactSDiv(F.ScaledReg, FactorS, SE) != Base.ScaledReg)
        continue;
    }

    // Check that multiplying with the unfolded offset doesn't overflow.
    if (F.UnfoldedOffset != 0) {
      if (F.UnfoldedOffset == INT64_MIN && Factor == -1)
        continue;
      F.UnfoldedOffset = (uint64_t)F.UnfoldedOffset * Factor;
      if (F.UnfoldedOffset / Factor != Base.UnfoldedOffset)
        continue;
      // If the offset will be truncated, check that it is in bounds.
      if (!IntTy->isPointerTy() &&
          !ConstantInt::isValueValidForType(IntTy, F.UnfoldedOffset))
        continue;
    }

    // If we make it here and it's legal, add it.
    (void)InsertFormula(LU, LUIdx, F);
  next:;
  }
}

/// GenerateScales - Generate stride factor reuse formulae by making use of
/// scaled-offset address modes, for example.
void LSRInstance::GenerateScales(LSRUse &LU, unsigned LUIdx, Formula Base) {
  // Determine the integer type for the base formula.
  Type *IntTy = Base.getType();
  if (!IntTy) return;

  // If this Formula already has a scaled register, we can't add another one.
  // Try to unscale the formula to generate a better scale.
  if (Base.Scale != 0 && !Base.Unscale())
    return;

  assert(Base.Scale == 0 && "Unscale did not did its job!");

  // Check each interesting stride.
  for (int64_t Factor : Factors) {
    Base.Scale = Factor;
    Base.HasBaseReg = Base.BaseRegs.size() > 1;
    // Check whether this scale is going to be legal.
    if (!isLegalUse(TTI, LU.MinOffset, LU.MaxOffset, LU.Kind, LU.AccessTy,
                    Base)) {
      // As a special-case, handle special out-of-loop Basic users specially.
      // TODO: Reconsider this special case.
      if (LU.Kind == LSRUse::Basic &&
          isLegalUse(TTI, LU.MinOffset, LU.MaxOffset, LSRUse::Special,
                     LU.AccessTy, Base) &&
          LU.AllFixupsOutsideLoop)
        LU.Kind = LSRUse::Special;
      else
        continue;
    }
    // For an ICmpZero, negating a solitary base register won't lead to
    // new solutions.
    if (LU.Kind == LSRUse::ICmpZero &&
        !Base.HasBaseReg && Base.BaseOffset == 0 && !Base.BaseGV)
      continue;
    // For each addrec base reg, apply the scale, if possible.
    for (size_t i = 0, e = Base.BaseRegs.size(); i != e; ++i)
      if (const SCEVAddRecExpr *AR =
            dyn_cast<SCEVAddRecExpr>(Base.BaseRegs[i])) {
        const SCEV *FactorS = SE.getConstant(IntTy, Factor);
        if (FactorS->isZero())
          continue;
        // Divide out the factor, ignoring high bits, since we'll be
        // scaling the value back up in the end.
        if (const SCEV *Quotient = getExactSDiv(AR, FactorS, SE, true)) {
          // TODO: This could be optimized to avoid all the copying.
          Formula F = Base;
          F.ScaledReg = Quotient;
          F.DeleteBaseReg(F.BaseRegs[i]);
          // The canonical representation of 1*reg is reg, which is already in
          // Base. In that case, do not try to insert the formula, it will be
          // rejected anyway.
          if (F.Scale == 1 && F.BaseRegs.empty())
            continue;
          (void)InsertFormula(LU, LUIdx, F);
        }
      }
  }
}

/// GenerateTruncates - Generate reuse formulae from different IV types.
void LSRInstance::GenerateTruncates(LSRUse &LU, unsigned LUIdx, Formula Base) {
  // Don't bother truncating symbolic values.
  if (Base.BaseGV) return;

  // Determine the integer type for the base formula.
  Type *DstTy = Base.getType();
  if (!DstTy) return;
  DstTy = SE.getEffectiveSCEVType(DstTy);

  for (Type *SrcTy : Types) {
    if (SrcTy != DstTy && TTI.isTruncateFree(SrcTy, DstTy)) {
      Formula F = Base;

      if (F.ScaledReg) F.ScaledReg = SE.getAnyExtendExpr(F.ScaledReg, SrcTy);
      for (const SCEV *&BaseReg : F.BaseRegs)
        BaseReg = SE.getAnyExtendExpr(BaseReg, SrcTy);

      // TODO: This assumes we've done basic processing on all uses and
      // have an idea what the register usage is.
      if (!F.hasRegsUsedByUsesOtherThan(LUIdx, RegUses))
        continue;

      (void)InsertFormula(LU, LUIdx, F);
    }
  }
}

namespace {

/// WorkItem - Helper class for GenerateCrossUseConstantOffsets. It's used to
/// defer modifications so that the search phase doesn't have to worry about
/// the data structures moving underneath it.
struct WorkItem {
  size_t LUIdx;
  int64_t Imm;
  const SCEV *OrigReg;

  WorkItem(size_t LI, int64_t I, const SCEV *R)
    : LUIdx(LI), Imm(I), OrigReg(R) {}

  void print(raw_ostream &OS) const;
  void dump() const;
};

}

void WorkItem::print(raw_ostream &OS) const {
  OS << "in formulae referencing " << *OrigReg << " in use " << LUIdx
     << " , add offset " << Imm;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void WorkItem::dump() const {
  print(errs()); errs() << '\n';
}
#endif

/// GenerateCrossUseConstantOffsets - Look for registers which are a constant
/// distance apart and try to form reuse opportunities between them.
void LSRInstance::GenerateCrossUseConstantOffsets() {
  // Group the registers by their value without any added constant offset.
  typedef std::map<int64_t, const SCEV *> ImmMapTy;
  DenseMap<const SCEV *, ImmMapTy> Map;
  DenseMap<const SCEV *, SmallBitVector> UsedByIndicesMap;
  SmallVector<const SCEV *, 8> Sequence;
  for (const SCEV *Use : RegUses) {
    const SCEV *Reg = Use; // Make a copy for ExtractImmediate to modify.
    int64_t Imm = ExtractImmediate(Reg, SE);
    auto Pair = Map.insert(std::make_pair(Reg, ImmMapTy()));
    if (Pair.second)
      Sequence.push_back(Reg);
    Pair.first->second.insert(std::make_pair(Imm, Use));
    UsedByIndicesMap[Reg] |= RegUses.getUsedByIndices(Use);
  }

  // Now examine each set of registers with the same base value. Build up
  // a list of work to do and do the work in a separate step so that we're
  // not adding formulae and register counts while we're searching.
  SmallVector<WorkItem, 32> WorkItems;
  SmallSet<std::pair<size_t, int64_t>, 32> UniqueItems;
  for (const SCEV *Reg : Sequence) {
    const ImmMapTy &Imms = Map.find(Reg)->second;

    // It's not worthwhile looking for reuse if there's only one offset.
    if (Imms.size() == 1)
      continue;

    DEBUG(dbgs() << "Generating cross-use offsets for " << *Reg << ':';
          for (const auto &Entry : Imms)
            dbgs() << ' ' << Entry.first;
          dbgs() << '\n');

    // Examine each offset.
    for (ImmMapTy::const_iterator J = Imms.begin(), JE = Imms.end();
         J != JE; ++J) {
      const SCEV *OrigReg = J->second;

      int64_t JImm = J->first;
      const SmallBitVector &UsedByIndices = RegUses.getUsedByIndices(OrigReg);

      if (!isa<SCEVConstant>(OrigReg) &&
          UsedByIndicesMap[Reg].count() == 1) {
        DEBUG(dbgs() << "Skipping cross-use reuse for " << *OrigReg << '\n');
        continue;
      }

      // Conservatively examine offsets between this orig reg a few selected
      // other orig regs.
      ImmMapTy::const_iterator OtherImms[] = {
        Imms.begin(), std::prev(Imms.end()),
        Imms.lower_bound((Imms.begin()->first + std::prev(Imms.end())->first) /
                         2)
      };
      for (size_t i = 0, e = array_lengthof(OtherImms); i != e; ++i) {
        ImmMapTy::const_iterator M = OtherImms[i];
        if (M == J || M == JE) continue;

        // Compute the difference between the two.
        int64_t Imm = (uint64_t)JImm - M->first;
        for (int LUIdx = UsedByIndices.find_first(); LUIdx != -1;
             LUIdx = UsedByIndices.find_next(LUIdx))
          // Make a memo of this use, offset, and register tuple.
          if (UniqueItems.insert(std::make_pair(LUIdx, Imm)).second)
            WorkItems.push_back(WorkItem(LUIdx, Imm, OrigReg));
      }
    }
  }

  Map.clear();
  Sequence.clear();
  UsedByIndicesMap.clear();
  UniqueItems.clear();

  // Now iterate through the worklist and add new formulae.
  for (const WorkItem &WI : WorkItems) {
    size_t LUIdx = WI.LUIdx;
    LSRUse &LU = Uses[LUIdx];
    int64_t Imm = WI.Imm;
    const SCEV *OrigReg = WI.OrigReg;

    Type *IntTy = SE.getEffectiveSCEVType(OrigReg->getType());
    const SCEV *NegImmS = SE.getSCEV(ConstantInt::get(IntTy, -(uint64_t)Imm));
    unsigned BitWidth = SE.getTypeSizeInBits(IntTy);

    // TODO: Use a more targeted data structure.
    for (size_t L = 0, LE = LU.Formulae.size(); L != LE; ++L) {
      Formula F = LU.Formulae[L];
      // FIXME: The code for the scaled and unscaled registers looks
      // very similar but slightly different. Investigate if they
      // could be merged. That way, we would not have to unscale the
      // Formula.
      F.Unscale();
      // Use the immediate in the scaled register.
      if (F.ScaledReg == OrigReg) {
        int64_t Offset = (uint64_t)F.BaseOffset + Imm * (uint64_t)F.Scale;
        // Don't create 50 + reg(-50).
        if (F.referencesReg(SE.getSCEV(
                   ConstantInt::get(IntTy, -(uint64_t)Offset))))
          continue;
        Formula NewF = F;
        NewF.BaseOffset = Offset;
        if (!isLegalUse(TTI, LU.MinOffset, LU.MaxOffset, LU.Kind, LU.AccessTy,
                        NewF))
          continue;
        NewF.ScaledReg = SE.getAddExpr(NegImmS, NewF.ScaledReg);

        // If the new scale is a constant in a register, and adding the constant
        // value to the immediate would produce a value closer to zero than the
        // immediate itself, then the formula isn't worthwhile.
        if (const SCEVConstant *C = dyn_cast<SCEVConstant>(NewF.ScaledReg))
          if (C->getValue()->isNegative() !=
                (NewF.BaseOffset < 0) &&
              (C->getValue()->getValue().abs() * APInt(BitWidth, F.Scale))
                .ule(std::abs(NewF.BaseOffset)))
            continue;

        // OK, looks good.
        NewF.Canonicalize();
        (void)InsertFormula(LU, LUIdx, NewF);
      } else {
        // Use the immediate in a base register.
        for (size_t N = 0, NE = F.BaseRegs.size(); N != NE; ++N) {
          const SCEV *BaseReg = F.BaseRegs[N];
          if (BaseReg != OrigReg)
            continue;
          Formula NewF = F;
          NewF.BaseOffset = (uint64_t)NewF.BaseOffset + Imm;
          if (!isLegalUse(TTI, LU.MinOffset, LU.MaxOffset,
                          LU.Kind, LU.AccessTy, NewF)) {
            if (!TTI.isLegalAddImmediate((uint64_t)NewF.UnfoldedOffset + Imm))
              continue;
            NewF = F;
            NewF.UnfoldedOffset = (uint64_t)NewF.UnfoldedOffset + Imm;
          }
          NewF.BaseRegs[N] = SE.getAddExpr(NegImmS, BaseReg);

          // If the new formula has a constant in a register, and adding the
          // constant value to the immediate would produce a value closer to
          // zero than the immediate itself, then the formula isn't worthwhile.
          for (const SCEV *NewReg : NewF.BaseRegs)
            if (const SCEVConstant *C = dyn_cast<SCEVConstant>(NewReg))
              if ((C->getValue()->getValue() + NewF.BaseOffset).abs().slt(
                   std::abs(NewF.BaseOffset)) &&
                  (C->getValue()->getValue() +
                   NewF.BaseOffset).countTrailingZeros() >=
                   countTrailingZeros<uint64_t>(NewF.BaseOffset))
                goto skip_formula;

          // Ok, looks good.
          NewF.Canonicalize();
          (void)InsertFormula(LU, LUIdx, NewF);
          break;
        skip_formula:;
        }
      }
    }
  }
}

/// GenerateAllReuseFormulae - Generate formulae for each use.
void
LSRInstance::GenerateAllReuseFormulae() {
  // This is split into multiple loops so that hasRegsUsedByUsesOtherThan
  // queries are more precise.
  for (size_t LUIdx = 0, NumUses = Uses.size(); LUIdx != NumUses; ++LUIdx) {
    LSRUse &LU = Uses[LUIdx];
    for (size_t i = 0, f = LU.Formulae.size(); i != f; ++i)
      GenerateReassociations(LU, LUIdx, LU.Formulae[i]);
    for (size_t i = 0, f = LU.Formulae.size(); i != f; ++i)
      GenerateCombinations(LU, LUIdx, LU.Formulae[i]);
  }
  for (size_t LUIdx = 0, NumUses = Uses.size(); LUIdx != NumUses; ++LUIdx) {
    LSRUse &LU = Uses[LUIdx];
    for (size_t i = 0, f = LU.Formulae.size(); i != f; ++i)
      GenerateSymbolicOffsets(LU, LUIdx, LU.Formulae[i]);
    for (size_t i = 0, f = LU.Formulae.size(); i != f; ++i)
      GenerateConstantOffsets(LU, LUIdx, LU.Formulae[i]);
    for (size_t i = 0, f = LU.Formulae.size(); i != f; ++i)
      GenerateICmpZeroScales(LU, LUIdx, LU.Formulae[i]);
    for (size_t i = 0, f = LU.Formulae.size(); i != f; ++i)
      GenerateScales(LU, LUIdx, LU.Formulae[i]);
  }
  for (size_t LUIdx = 0, NumUses = Uses.size(); LUIdx != NumUses; ++LUIdx) {
    LSRUse &LU = Uses[LUIdx];
    for (size_t i = 0, f = LU.Formulae.size(); i != f; ++i)
      GenerateTruncates(LU, LUIdx, LU.Formulae[i]);
  }

  GenerateCrossUseConstantOffsets();

  DEBUG(dbgs() << "\n"
                  "After generating reuse formulae:\n";
        print_uses(dbgs()));
}

/// If there are multiple formulae with the same set of registers used
/// by other uses, pick the best one and delete the others.
void LSRInstance::FilterOutUndesirableDedicatedRegisters() {
  DenseSet<const SCEV *> VisitedRegs;
  SmallPtrSet<const SCEV *, 16> Regs;
  SmallPtrSet<const SCEV *, 16> LoserRegs;
#ifndef NDEBUG
  bool ChangedFormulae = false;
#endif

  // Collect the best formula for each unique set of shared registers. This
  // is reset for each use.
  typedef DenseMap<SmallVector<const SCEV *, 4>, size_t, UniquifierDenseMapInfo>
    BestFormulaeTy;
  BestFormulaeTy BestFormulae;

  for (size_t LUIdx = 0, NumUses = Uses.size(); LUIdx != NumUses; ++LUIdx) {
    LSRUse &LU = Uses[LUIdx];
    DEBUG(dbgs() << "Filtering for use "; LU.print(dbgs()); dbgs() << '\n');

    bool Any = false;
    for (size_t FIdx = 0, NumForms = LU.Formulae.size();
         FIdx != NumForms; ++FIdx) {
      Formula &F = LU.Formulae[FIdx];

      // Some formulas are instant losers. For example, they may depend on
      // nonexistent AddRecs from other loops. These need to be filtered
      // immediately, otherwise heuristics could choose them over others leading
      // to an unsatisfactory solution. Passing LoserRegs into RateFormula here
      // avoids the need to recompute this information across formulae using the
      // same bad AddRec. Passing LoserRegs is also essential unless we remove
      // the corresponding bad register from the Regs set.
      Cost CostF;
      Regs.clear();
      CostF.RateFormula(TTI, F, Regs, VisitedRegs, L, LU.Offsets, SE, DT, LU,
                        &LoserRegs);
      if (CostF.isLoser()) {
        // During initial formula generation, undesirable formulae are generated
        // by uses within other loops that have some non-trivial address mode or
        // use the postinc form of the IV. LSR needs to provide these formulae
        // as the basis of rediscovering the desired formula that uses an AddRec
        // corresponding to the existing phi. Once all formulae have been
        // generated, these initial losers may be pruned.
        DEBUG(dbgs() << "  Filtering loser "; F.print(dbgs());
              dbgs() << "\n");
      }
      else {
        SmallVector<const SCEV *, 4> Key;
        for (const SCEV *Reg : F.BaseRegs) {
          if (RegUses.isRegUsedByUsesOtherThan(Reg, LUIdx))
            Key.push_back(Reg);
        }
        if (F.ScaledReg &&
            RegUses.isRegUsedByUsesOtherThan(F.ScaledReg, LUIdx))
          Key.push_back(F.ScaledReg);
        // Unstable sort by host order ok, because this is only used for
        // uniquifying.
        std::sort(Key.begin(), Key.end());

        std::pair<BestFormulaeTy::const_iterator, bool> P =
          BestFormulae.insert(std::make_pair(Key, FIdx));
        if (P.second)
          continue;

        Formula &Best = LU.Formulae[P.first->second];

        Cost CostBest;
        Regs.clear();
        CostBest.RateFormula(TTI, Best, Regs, VisitedRegs, L, LU.Offsets, SE,
                             DT, LU);
        if (CostF < CostBest)
          std::swap(F, Best);
        DEBUG(dbgs() << "  Filtering out formula "; F.print(dbgs());
              dbgs() << "\n"
                        "    in favor of formula "; Best.print(dbgs());
              dbgs() << '\n');
      }
#ifndef NDEBUG
      ChangedFormulae = true;
#endif
      LU.DeleteFormula(F);
      --FIdx;
      --NumForms;
      Any = true;
    }

    // Now that we've filtered out some formulae, recompute the Regs set.
    if (Any)
      LU.RecomputeRegs(LUIdx, RegUses);

    // Reset this to prepare for the next use.
    BestFormulae.clear();
  }

  DEBUG(if (ChangedFormulae) {
          dbgs() << "\n"
                    "After filtering out undesirable candidates:\n";
          print_uses(dbgs());
        });
}

// This is a rough guess that seems to work fairly well.
static const size_t ComplexityLimit = UINT16_MAX;

/// EstimateSearchSpaceComplexity - Estimate the worst-case number of
/// solutions the solver might have to consider. It almost never considers
/// this many solutions because it prune the search space, but the pruning
/// isn't always sufficient.
size_t LSRInstance::EstimateSearchSpaceComplexity() const {
  size_t Power = 1;
  for (const LSRUse &LU : Uses) {
    size_t FSize = LU.Formulae.size();
    if (FSize >= ComplexityLimit) {
      Power = ComplexityLimit;
      break;
    }
    Power *= FSize;
    if (Power >= ComplexityLimit)
      break;
  }
  return Power;
}

/// NarrowSearchSpaceByDetectingSupersets - When one formula uses a superset
/// of the registers of another formula, it won't help reduce register
/// pressure (though it may not necessarily hurt register pressure); remove
/// it to simplify the system.
void LSRInstance::NarrowSearchSpaceByDetectingSupersets() {
  if (EstimateSearchSpaceComplexity() >= ComplexityLimit) {
    DEBUG(dbgs() << "The search space is too complex.\n");

    DEBUG(dbgs() << "Narrowing the search space by eliminating formulae "
                    "which use a superset of registers used by other "
                    "formulae.\n");

    for (size_t LUIdx = 0, NumUses = Uses.size(); LUIdx != NumUses; ++LUIdx) {
      LSRUse &LU = Uses[LUIdx];
      bool Any = false;
      for (size_t i = 0, e = LU.Formulae.size(); i != e; ++i) {
        Formula &F = LU.Formulae[i];
        // Look for a formula with a constant or GV in a register. If the use
        // also has a formula with that same value in an immediate field,
        // delete the one that uses a register.
        for (SmallVectorImpl<const SCEV *>::const_iterator
             I = F.BaseRegs.begin(), E = F.BaseRegs.end(); I != E; ++I) {
          if (const SCEVConstant *C = dyn_cast<SCEVConstant>(*I)) {
            Formula NewF = F;
            NewF.BaseOffset += C->getValue()->getSExtValue();
            NewF.BaseRegs.erase(NewF.BaseRegs.begin() +
                                (I - F.BaseRegs.begin()));
            if (LU.HasFormulaWithSameRegs(NewF)) {
              DEBUG(dbgs() << "  Deleting "; F.print(dbgs()); dbgs() << '\n');
              LU.DeleteFormula(F);
              --i;
              --e;
              Any = true;
              break;
            }
          } else if (const SCEVUnknown *U = dyn_cast<SCEVUnknown>(*I)) {
            if (GlobalValue *GV = dyn_cast<GlobalValue>(U->getValue()))
              if (!F.BaseGV) {
                Formula NewF = F;
                NewF.BaseGV = GV;
                NewF.BaseRegs.erase(NewF.BaseRegs.begin() +
                                    (I - F.BaseRegs.begin()));
                if (LU.HasFormulaWithSameRegs(NewF)) {
                  DEBUG(dbgs() << "  Deleting "; F.print(dbgs());
                        dbgs() << '\n');
                  LU.DeleteFormula(F);
                  --i;
                  --e;
                  Any = true;
                  break;
                }
              }
          }
        }
      }
      if (Any)
        LU.RecomputeRegs(LUIdx, RegUses);
    }

    DEBUG(dbgs() << "After pre-selection:\n";
          print_uses(dbgs()));
  }
}

/// NarrowSearchSpaceByCollapsingUnrolledCode - When there are many registers
/// for expressions like A, A+1, A+2, etc., allocate a single register for
/// them.
void LSRInstance::NarrowSearchSpaceByCollapsingUnrolledCode() {
  if (EstimateSearchSpaceComplexity() < ComplexityLimit)
    return;

  DEBUG(dbgs() << "The search space is too complex.\n"
                  "Narrowing the search space by assuming that uses separated "
                  "by a constant offset will use the same registers.\n");

  // This is especially useful for unrolled loops.

  for (size_t LUIdx = 0, NumUses = Uses.size(); LUIdx != NumUses; ++LUIdx) {
    LSRUse &LU = Uses[LUIdx];
    for (const Formula &F : LU.Formulae) {
      if (F.BaseOffset == 0 || (F.Scale != 0 && F.Scale != 1))
        continue;

      LSRUse *LUThatHas = FindUseWithSimilarFormula(F, LU);
      if (!LUThatHas)
        continue;

      if (!reconcileNewOffset(*LUThatHas, F.BaseOffset, /*HasBaseReg=*/ false,
                              LU.Kind, LU.AccessTy))
        continue;

      DEBUG(dbgs() << "  Deleting use "; LU.print(dbgs()); dbgs() << '\n');

      LUThatHas->AllFixupsOutsideLoop &= LU.AllFixupsOutsideLoop;

      // Update the relocs to reference the new use.
      for (LSRFixup &Fixup : Fixups) {
        if (Fixup.LUIdx == LUIdx) {
          Fixup.LUIdx = LUThatHas - &Uses.front();
          Fixup.Offset += F.BaseOffset;
          // Add the new offset to LUThatHas' offset list.
          if (LUThatHas->Offsets.back() != Fixup.Offset) {
            LUThatHas->Offsets.push_back(Fixup.Offset);
            if (Fixup.Offset > LUThatHas->MaxOffset)
              LUThatHas->MaxOffset = Fixup.Offset;
            if (Fixup.Offset < LUThatHas->MinOffset)
              LUThatHas->MinOffset = Fixup.Offset;
          }
          DEBUG(dbgs() << "New fixup has offset " << Fixup.Offset << '\n');
        }
        if (Fixup.LUIdx == NumUses-1)
          Fixup.LUIdx = LUIdx;
      }

      // Delete formulae from the new use which are no longer legal.
      bool Any = false;
      for (size_t i = 0, e = LUThatHas->Formulae.size(); i != e; ++i) {
        Formula &F = LUThatHas->Formulae[i];
        if (!isLegalUse(TTI, LUThatHas->MinOffset, LUThatHas->MaxOffset,
                        LUThatHas->Kind, LUThatHas->AccessTy, F)) {
          DEBUG(dbgs() << "  Deleting "; F.print(dbgs());
                dbgs() << '\n');
          LUThatHas->DeleteFormula(F);
          --i;
          --e;
          Any = true;
        }
      }

      if (Any)
        LUThatHas->RecomputeRegs(LUThatHas - &Uses.front(), RegUses);

      // Delete the old use.
      DeleteUse(LU, LUIdx);
      --LUIdx;
      --NumUses;
      break;
    }
  }

  DEBUG(dbgs() << "After pre-selection:\n"; print_uses(dbgs()));
}

/// NarrowSearchSpaceByRefilteringUndesirableDedicatedRegisters - Call
/// FilterOutUndesirableDedicatedRegisters again, if necessary, now that
/// we've done more filtering, as it may be able to find more formulae to
/// eliminate.
void LSRInstance::NarrowSearchSpaceByRefilteringUndesirableDedicatedRegisters(){
  if (EstimateSearchSpaceComplexity() >= ComplexityLimit) {
    DEBUG(dbgs() << "The search space is too complex.\n");

    DEBUG(dbgs() << "Narrowing the search space by re-filtering out "
                    "undesirable dedicated registers.\n");

    FilterOutUndesirableDedicatedRegisters();

    DEBUG(dbgs() << "After pre-selection:\n";
          print_uses(dbgs()));
  }
}

/// NarrowSearchSpaceByPickingWinnerRegs - Pick a register which seems likely
/// to be profitable, and then in any use which has any reference to that
/// register, delete all formulae which do not reference that register.
void LSRInstance::NarrowSearchSpaceByPickingWinnerRegs() {
  // With all other options exhausted, loop until the system is simple
  // enough to handle.
  SmallPtrSet<const SCEV *, 4> Taken;
  while (EstimateSearchSpaceComplexity() >= ComplexityLimit) {
    // Ok, we have too many of formulae on our hands to conveniently handle.
    // Use a rough heuristic to thin out the list.
    DEBUG(dbgs() << "The search space is too complex.\n");

    // Pick the register which is used by the most LSRUses, which is likely
    // to be a good reuse register candidate.
    const SCEV *Best = nullptr;
    unsigned BestNum = 0;
    for (const SCEV *Reg : RegUses) {
      if (Taken.count(Reg))
        continue;
      if (!Best)
        Best = Reg;
      else {
        unsigned Count = RegUses.getUsedByIndices(Reg).count();
        if (Count > BestNum) {
          Best = Reg;
          BestNum = Count;
        }
      }
    }

    DEBUG(dbgs() << "Narrowing the search space by assuming " << *Best
                 << " will yield profitable reuse.\n");
    Taken.insert(Best);

    // In any use with formulae which references this register, delete formulae
    // which don't reference it.
    for (size_t LUIdx = 0, NumUses = Uses.size(); LUIdx != NumUses; ++LUIdx) {
      LSRUse &LU = Uses[LUIdx];
      if (!LU.Regs.count(Best)) continue;

      bool Any = false;
      for (size_t i = 0, e = LU.Formulae.size(); i != e; ++i) {
        Formula &F = LU.Formulae[i];
        if (!F.referencesReg(Best)) {
          DEBUG(dbgs() << "  Deleting "; F.print(dbgs()); dbgs() << '\n');
          LU.DeleteFormula(F);
          --e;
          --i;
          Any = true;
          assert(e != 0 && "Use has no formulae left! Is Regs inconsistent?");
          continue;
        }
      }

      if (Any)
        LU.RecomputeRegs(LUIdx, RegUses);
    }

    DEBUG(dbgs() << "After pre-selection:\n";
          print_uses(dbgs()));
  }
}

/// NarrowSearchSpaceUsingHeuristics - If there are an extraordinary number of
/// formulae to choose from, use some rough heuristics to prune down the number
/// of formulae. This keeps the main solver from taking an extraordinary amount
/// of time in some worst-case scenarios.
void LSRInstance::NarrowSearchSpaceUsingHeuristics() {
  NarrowSearchSpaceByDetectingSupersets();
  NarrowSearchSpaceByCollapsingUnrolledCode();
  NarrowSearchSpaceByRefilteringUndesirableDedicatedRegisters();
  NarrowSearchSpaceByPickingWinnerRegs();
}

/// SolveRecurse - This is the recursive solver.
void LSRInstance::SolveRecurse(SmallVectorImpl<const Formula *> &Solution,
                               Cost &SolutionCost,
                               SmallVectorImpl<const Formula *> &Workspace,
                               const Cost &CurCost,
                               const SmallPtrSet<const SCEV *, 16> &CurRegs,
                               DenseSet<const SCEV *> &VisitedRegs) const {
  // Some ideas:
  //  - prune more:
  //    - use more aggressive filtering
  //    - sort the formula so that the most profitable solutions are found first
  //    - sort the uses too
  //  - search faster:
  //    - don't compute a cost, and then compare. compare while computing a cost
  //      and bail early.
  //    - track register sets with SmallBitVector

  const LSRUse &LU = Uses[Workspace.size()];

  // If this use references any register that's already a part of the
  // in-progress solution, consider it a requirement that a formula must
  // reference that register in order to be considered. This prunes out
  // unprofitable searching.
  SmallSetVector<const SCEV *, 4> ReqRegs;
  for (const SCEV *S : CurRegs)
    if (LU.Regs.count(S))
      ReqRegs.insert(S);

  SmallPtrSet<const SCEV *, 16> NewRegs;
  Cost NewCost;
  for (const Formula &F : LU.Formulae) {
    // Ignore formulae which may not be ideal in terms of register reuse of
    // ReqRegs.  The formula should use all required registers before
    // introducing new ones.
    int NumReqRegsToFind = std::min(F.getNumRegs(), ReqRegs.size());
    for (const SCEV *Reg : ReqRegs) {
      if ((F.ScaledReg && F.ScaledReg == Reg) ||
          std::find(F.BaseRegs.begin(), F.BaseRegs.end(), Reg) !=
          F.BaseRegs.end()) {
        --NumReqRegsToFind;
        if (NumReqRegsToFind == 0)
          break;
      }
    }
    if (NumReqRegsToFind != 0) {
      // If none of the formulae satisfied the required registers, then we could
      // clear ReqRegs and try again. Currently, we simply give up in this case.
      continue;
    }

    // Evaluate the cost of the current formula. If it's already worse than
    // the current best, prune the search at that point.
    NewCost = CurCost;
    NewRegs = CurRegs;
    NewCost.RateFormula(TTI, F, NewRegs, VisitedRegs, L, LU.Offsets, SE, DT,
                        LU);
    if (NewCost < SolutionCost) {
      Workspace.push_back(&F);
      if (Workspace.size() != Uses.size()) {
        SolveRecurse(Solution, SolutionCost, Workspace, NewCost,
                     NewRegs, VisitedRegs);
        if (F.getNumRegs() == 1 && Workspace.size() == 1)
          VisitedRegs.insert(F.ScaledReg ? F.ScaledReg : F.BaseRegs[0]);
      } else {
        DEBUG(dbgs() << "New best at "; NewCost.print(dbgs());
              dbgs() << ".\n Regs:";
              for (const SCEV *S : NewRegs)
                dbgs() << ' ' << *S;
              dbgs() << '\n');

        SolutionCost = NewCost;
        Solution = Workspace;
      }
      Workspace.pop_back();
    }
  }
}

/// Solve - Choose one formula from each use. Return the results in the given
/// Solution vector.
void LSRInstance::Solve(SmallVectorImpl<const Formula *> &Solution) const {
  SmallVector<const Formula *, 8> Workspace;
  Cost SolutionCost;
  SolutionCost.Lose();
  Cost CurCost;
  SmallPtrSet<const SCEV *, 16> CurRegs;
  DenseSet<const SCEV *> VisitedRegs;
  Workspace.reserve(Uses.size());

  // SolveRecurse does all the work.
  SolveRecurse(Solution, SolutionCost, Workspace, CurCost,
               CurRegs, VisitedRegs);
  if (Solution.empty()) {
    DEBUG(dbgs() << "\nNo Satisfactory Solution\n");
    return;
  }

  // Ok, we've now made all our decisions.
  DEBUG(dbgs() << "\n"
                  "The chosen solution requires "; SolutionCost.print(dbgs());
        dbgs() << ":\n";
        for (size_t i = 0, e = Uses.size(); i != e; ++i) {
          dbgs() << "  ";
          Uses[i].print(dbgs());
          dbgs() << "\n"
                    "    ";
          Solution[i]->print(dbgs());
          dbgs() << '\n';
        });

  assert(Solution.size() == Uses.size() && "Malformed solution!");
}

/// HoistInsertPosition - Helper for AdjustInsertPositionForExpand. Climb up
/// the dominator tree far as we can go while still being dominated by the
/// input positions. This helps canonicalize the insert position, which
/// encourages sharing.
BasicBlock::iterator
LSRInstance::HoistInsertPosition(BasicBlock::iterator IP,
                                 const SmallVectorImpl<Instruction *> &Inputs)
                                                                         const {
  for (;;) {
    const Loop *IPLoop = LI.getLoopFor(IP->getParent());
    unsigned IPLoopDepth = IPLoop ? IPLoop->getLoopDepth() : 0;

    BasicBlock *IDom;
    for (DomTreeNode *Rung = DT.getNode(IP->getParent()); ; ) {
      if (!Rung) return IP;
      Rung = Rung->getIDom();
      if (!Rung) return IP;
      IDom = Rung->getBlock();

      // Don't climb into a loop though.
      const Loop *IDomLoop = LI.getLoopFor(IDom);
      unsigned IDomDepth = IDomLoop ? IDomLoop->getLoopDepth() : 0;
      if (IDomDepth <= IPLoopDepth &&
          (IDomDepth != IPLoopDepth || IDomLoop == IPLoop))
        break;
    }

    bool AllDominate = true;
    Instruction *BetterPos = nullptr;
    Instruction *Tentative = IDom->getTerminator();
    for (Instruction *Inst : Inputs) {
      if (Inst == Tentative || !DT.dominates(Inst, Tentative)) {
        AllDominate = false;
        break;
      }
      // Attempt to find an insert position in the middle of the block,
      // instead of at the end, so that it can be used for other expansions.
      if (IDom == Inst->getParent() &&
          (!BetterPos || !DT.dominates(Inst, BetterPos)))
        BetterPos = std::next(BasicBlock::iterator(Inst));
    }
    if (!AllDominate)
      break;
    if (BetterPos)
      IP = BetterPos;
    else
      IP = Tentative;
  }

  return IP;
}

/// AdjustInsertPositionForExpand - Determine an input position which will be
/// dominated by the operands and which will dominate the result.
BasicBlock::iterator
LSRInstance::AdjustInsertPositionForExpand(BasicBlock::iterator LowestIP,
                                           const LSRFixup &LF,
                                           const LSRUse &LU,
                                           SCEVExpander &Rewriter) const {
  // Collect some instructions which must be dominated by the
  // expanding replacement. These must be dominated by any operands that
  // will be required in the expansion.
  SmallVector<Instruction *, 4> Inputs;
  if (Instruction *I = dyn_cast<Instruction>(LF.OperandValToReplace))
    Inputs.push_back(I);
  if (LU.Kind == LSRUse::ICmpZero)
    if (Instruction *I =
          dyn_cast<Instruction>(cast<ICmpInst>(LF.UserInst)->getOperand(1)))
      Inputs.push_back(I);
  if (LF.PostIncLoops.count(L)) {
    if (LF.isUseFullyOutsideLoop(L))
      Inputs.push_back(L->getLoopLatch()->getTerminator());
    else
      Inputs.push_back(IVIncInsertPos);
  }
  // The expansion must also be dominated by the increment positions of any
  // loops it for which it is using post-inc mode.
  for (const Loop *PIL : LF.PostIncLoops) {
    if (PIL == L) continue;

    // Be dominated by the loop exit.
    SmallVector<BasicBlock *, 4> ExitingBlocks;
    PIL->getExitingBlocks(ExitingBlocks);
    if (!ExitingBlocks.empty()) {
      BasicBlock *BB = ExitingBlocks[0];
      for (unsigned i = 1, e = ExitingBlocks.size(); i != e; ++i)
        BB = DT.findNearestCommonDominator(BB, ExitingBlocks[i]);
      Inputs.push_back(BB->getTerminator());
    }
  }

  assert(!isa<PHINode>(LowestIP) && !isa<LandingPadInst>(LowestIP)
         && !isa<DbgInfoIntrinsic>(LowestIP) &&
         "Insertion point must be a normal instruction");

  // Then, climb up the immediate dominator tree as far as we can go while
  // still being dominated by the input positions.
  BasicBlock::iterator IP = HoistInsertPosition(LowestIP, Inputs);

  // Don't insert instructions before PHI nodes.
  while (isa<PHINode>(IP)) ++IP;

  // Ignore landingpad instructions.
  while (isa<LandingPadInst>(IP)) ++IP;

  // Ignore debug intrinsics.
  while (isa<DbgInfoIntrinsic>(IP)) ++IP;

  // Set IP below instructions recently inserted by SCEVExpander. This keeps the
  // IP consistent across expansions and allows the previously inserted
  // instructions to be reused by subsequent expansion.
  while (Rewriter.isInsertedInstruction(IP) && IP != LowestIP) ++IP;

  return IP;
}

/// Expand - Emit instructions for the leading candidate expression for this
/// LSRUse (this is called "expanding").
Value *LSRInstance::Expand(const LSRFixup &LF, const Formula &F,
                           BasicBlock::iterator IP, SCEVExpander &Rewriter,
                           SmallVectorImpl<WeakTrackingVH> &DeadInsts) const {
  const LSRUse &LU = Uses[LF.LUIdx];
  if (LU.RigidFormula)
    return LF.OperandValToReplace;

  // Determine an input position which will be dominated by the operands and
  // which will dominate the result.
  IP = AdjustInsertPositionForExpand(IP, LF, LU, Rewriter);

  // Inform the Rewriter if we have a post-increment use, so that it can
  // perform an advantageous expansion.
  Rewriter.setPostInc(LF.PostIncLoops);

  // This is the type that the user actually needs.
  Type *OpTy = LF.OperandValToReplace->getType();
  // This will be the type that we'll initially expand to.
  Type *Ty = F.getType();
  if (!Ty)
    // No type known; just expand directly to the ultimate type.
    Ty = OpTy;
  else if (SE.getEffectiveSCEVType(Ty) == SE.getEffectiveSCEVType(OpTy))
    // Expand directly to the ultimate type if it's the right size.
    Ty = OpTy;
  // This is the type to do integer arithmetic in.
  Type *IntTy = SE.getEffectiveSCEVType(Ty);

  // Build up a list of operands to add together to form the full base.
  SmallVector<const SCEV *, 8> Ops;

  // Expand the BaseRegs portion.
  for (const SCEV *Reg : F.BaseRegs) {
    assert(!Reg->isZero() && "Zero allocated in a base register!");

    // If we're expanding for a post-inc user, make the post-inc adjustment.
    PostIncLoopSet &Loops = const_cast<PostIncLoopSet &>(LF.PostIncLoops);
    Reg = TransformForPostIncUse(Denormalize, Reg,
                                 LF.UserInst, LF.OperandValToReplace,
                                 Loops, SE, DT);

    Ops.push_back(SE.getUnknown(Rewriter.expandCodeFor(Reg, nullptr, IP)));
  }

  // Expand the ScaledReg portion.
  Value *ICmpScaledV = nullptr;
  if (F.Scale != 0) {
    const SCEV *ScaledS = F.ScaledReg;

    // If we're expanding for a post-inc user, make the post-inc adjustment.
    PostIncLoopSet &Loops = const_cast<PostIncLoopSet &>(LF.PostIncLoops);
    ScaledS = TransformForPostIncUse(Denormalize, ScaledS,
                                     LF.UserInst, LF.OperandValToReplace,
                                     Loops, SE, DT);

    if (LU.Kind == LSRUse::ICmpZero) {
      // Expand ScaleReg as if it was part of the base regs.
      if (F.Scale == 1)
        Ops.push_back(
            SE.getUnknown(Rewriter.expandCodeFor(ScaledS, nullptr, IP)));
      else {
        // An interesting way of "folding" with an icmp is to use a negated
        // scale, which we'll implement by inserting it into the other operand
        // of the icmp.
        assert(F.Scale == -1 &&
               "The only scale supported by ICmpZero uses is -1!");
        ICmpScaledV = Rewriter.expandCodeFor(ScaledS, nullptr, IP);
      }
    } else {
      // Otherwise just expand the scaled register and an explicit scale,
      // which is expected to be matched as part of the address.

      // Flush the operand list to suppress SCEVExpander hoisting address modes.
      // Unless the addressing mode will not be folded.
      if (!Ops.empty() && LU.Kind == LSRUse::Address &&
          isAMCompletelyFolded(TTI, LU, F)) {
        Value *FullV = Rewriter.expandCodeFor(SE.getAddExpr(Ops), Ty, IP);
        Ops.clear();
        Ops.push_back(SE.getUnknown(FullV));
      }
      ScaledS = SE.getUnknown(Rewriter.expandCodeFor(ScaledS, nullptr, IP));
      if (F.Scale != 1)
        ScaledS =
            SE.getMulExpr(ScaledS, SE.getConstant(ScaledS->getType(), F.Scale));
      Ops.push_back(ScaledS);
    }
  }

  // Expand the GV portion.
  if (F.BaseGV) {
    // Flush the operand list to suppress SCEVExpander hoisting.
    if (!Ops.empty()) {
      Value *FullV = Rewriter.expandCodeFor(SE.getAddExpr(Ops), Ty, IP);
      Ops.clear();
      Ops.push_back(SE.getUnknown(FullV));
    }
    Ops.push_back(SE.getUnknown(F.BaseGV));
  }

  // Flush the operand list to suppress SCEVExpander hoisting of both folded and
  // unfolded offsets. LSR assumes they both live next to their uses.
  if (!Ops.empty()) {
    Value *FullV = Rewriter.expandCodeFor(SE.getAddExpr(Ops), Ty, IP);
    Ops.clear();
    Ops.push_back(SE.getUnknown(FullV));
  }

  // Expand the immediate portion.
  int64_t Offset = (uint64_t)F.BaseOffset + LF.Offset;
  if (Offset != 0) {
    if (LU.Kind == LSRUse::ICmpZero) {
      // The other interesting way of "folding" with an ICmpZero is to use a
      // negated immediate.
      if (!ICmpScaledV)
        ICmpScaledV = ConstantInt::get(IntTy, -(uint64_t)Offset);
      else {
        Ops.push_back(SE.getUnknown(ICmpScaledV));
        ICmpScaledV = ConstantInt::get(IntTy, Offset);
      }
    } else {
      // Just add the immediate values. These again are expected to be matched
      // as part of the address.
      Ops.push_back(SE.getUnknown(ConstantInt::getSigned(IntTy, Offset)));
    }
  }

  // Expand the unfolded offset portion.
  int64_t UnfoldedOffset = F.UnfoldedOffset;
  if (UnfoldedOffset != 0) {
    // Just add the immediate values.
    Ops.push_back(SE.getUnknown(ConstantInt::getSigned(IntTy,
                                                       UnfoldedOffset)));
  }

  // Emit instructions summing all the operands.
  const SCEV *FullS = Ops.empty() ?
                      SE.getConstant(IntTy, 0) :
                      SE.getAddExpr(Ops);
  Value *FullV = Rewriter.expandCodeFor(FullS, Ty, IP);

  // We're done expanding now, so reset the rewriter.
  Rewriter.clearPostInc();

  // An ICmpZero Formula represents an ICmp which we're handling as a
  // comparison against zero. Now that we've expanded an expression for that
  // form, update the ICmp's other operand.
  if (LU.Kind == LSRUse::ICmpZero) {
    ICmpInst *CI = cast<ICmpInst>(LF.UserInst);
    DeadInsts.emplace_back(CI->getOperand(1));
    assert(!F.BaseGV && "ICmp does not support folding a global value and "
                           "a scale at the same time!");
    if (F.Scale == -1) {
      if (ICmpScaledV->getType() != OpTy) {
        Instruction *Cast =
          CastInst::Create(CastInst::getCastOpcode(ICmpScaledV, false,
                                                   OpTy, false),
                           ICmpScaledV, OpTy, "tmp", CI);
        ICmpScaledV = Cast;
      }
      CI->setOperand(1, ICmpScaledV);
    } else {
      // A scale of 1 means that the scale has been expanded as part of the
      // base regs.
      assert((F.Scale == 0 || F.Scale == 1) &&
             "ICmp does not support folding a global value and "
             "a scale at the same time!");
      Constant *C = ConstantInt::getSigned(SE.getEffectiveSCEVType(OpTy),
                                           -(uint64_t)Offset);
      if (C->getType() != OpTy)
        C = ConstantExpr::getCast(CastInst::getCastOpcode(C, false,
                                                          OpTy, false),
                                  C, OpTy);

      CI->setOperand(1, C);
    }
  }

  return FullV;
}

/// RewriteForPHI - Helper for Rewrite. PHI nodes are special because the use
/// of their operands effectively happens in their predecessor blocks, so the
/// expression may need to be expanded in multiple places.
void LSRInstance::RewriteForPHI(PHINode *PN, const LSRFixup &LF,
                                const Formula &F, SCEVExpander &Rewriter,
                                SmallVectorImpl<WeakTrackingVH> &DeadInsts,
                                Pass *P) const {
  DenseMap<BasicBlock *, Value *> Inserted;
  for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i)
    if (PN->getIncomingValue(i) == LF.OperandValToReplace) {
      BasicBlock *BB = PN->getIncomingBlock(i);

      // If this is a critical edge, split the edge so that we do not insert
      // the code on all predecessor/successor paths.  We do this unless this
      // is the canonical backedge for this loop, which complicates post-inc
      // users.
      if (e != 1 && BB->getTerminator()->getNumSuccessors() > 1 &&
          !isa<IndirectBrInst>(BB->getTerminator())) {
        BasicBlock *Parent = PN->getParent();
        Loop *PNLoop = LI.getLoopFor(Parent);
        if (!PNLoop || Parent != PNLoop->getHeader()) {
          // Split the critical edge.
          BasicBlock *NewBB = nullptr;
          if (!Parent->isLandingPad()) {
            NewBB = SplitCriticalEdge(BB, Parent,
                                      CriticalEdgeSplittingOptions(&DT, &LI)
                                          .setMergeIdenticalEdges()
                                          .setDontDeleteUselessPHIs());
          } else {
            SmallVector<BasicBlock*, 2> NewBBs;
            SplitLandingPadPredecessors(Parent, BB, "", "", NewBBs,
                                        /*AliasAnalysis*/ nullptr, &DT, &LI);
            NewBB = NewBBs[0];
          }
          // If NewBB==NULL, then SplitCriticalEdge refused to split because all
          // phi predecessors are identical. The simple thing to do is skip
          // splitting in this case rather than complicate the API.
          if (NewBB) {
            // If PN is outside of the loop and BB is in the loop, we want to
            // move the block to be immediately before the PHI block, not
            // immediately after BB.
            if (L->contains(BB) && !L->contains(PN))
              NewBB->moveBefore(PN->getParent());

            // Splitting the edge can reduce the number of PHI entries we have.
            e = PN->getNumIncomingValues();
            BB = NewBB;
            i = PN->getBasicBlockIndex(BB);
          }
        }
      }

      std::pair<DenseMap<BasicBlock *, Value *>::iterator, bool> Pair =
        Inserted.insert(std::make_pair(BB, static_cast<Value *>(nullptr)));
      if (!Pair.second)
        PN->setIncomingValue(i, Pair.first->second);
      else {
        Value *FullV = Expand(LF, F, BB->getTerminator(), Rewriter, DeadInsts);

        // If this is reuse-by-noop-cast, insert the noop cast.
        Type *OpTy = LF.OperandValToReplace->getType();
        if (FullV->getType() != OpTy)
          FullV =
            CastInst::Create(CastInst::getCastOpcode(FullV, false,
                                                     OpTy, false),
                             FullV, LF.OperandValToReplace->getType(),
                             "tmp", BB->getTerminator());

        PN->setIncomingValue(i, FullV);
        Pair.first->second = FullV;
      }
    }
}

/// Rewrite - Emit instructions for the leading candidate expression for this
/// LSRUse (this is called "expanding"), and update the UserInst to reference
/// the newly expanded value.
void LSRInstance::Rewrite(const LSRFixup &LF, const Formula &F,
                          SCEVExpander &Rewriter,
                          SmallVectorImpl<WeakTrackingVH> &DeadInsts,
                          Pass *P) const {
  // First, find an insertion point that dominates UserInst. For PHI nodes,
  // find the nearest block which dominates all the relevant uses.
  if (PHINode *PN = dyn_cast<PHINode>(LF.UserInst)) {
    RewriteForPHI(PN, LF, F, Rewriter, DeadInsts, P);
  } else {
    Value *FullV = Expand(LF, F, LF.UserInst, Rewriter, DeadInsts);

    // If this is reuse-by-noop-cast, insert the noop cast.
    Type *OpTy = LF.OperandValToReplace->getType();
    if (FullV->getType() != OpTy) {
      Instruction *Cast =
        CastInst::Create(CastInst::getCastOpcode(FullV, false, OpTy, false),
                         FullV, OpTy, "tmp", LF.UserInst);
      FullV = Cast;
    }

    // Update the user. ICmpZero is handled specially here (for now) because
    // Expand may have updated one of the operands of the icmp already, and
    // its new value may happen to be equal to LF.OperandValToReplace, in
    // which case doing replaceUsesOfWith leads to replacing both operands
    // with the same value. TODO: Reorganize this.
    if (Uses[LF.LUIdx].Kind == LSRUse::ICmpZero)
      LF.UserInst->setOperand(0, FullV);
    else
      LF.UserInst->replaceUsesOfWith(LF.OperandValToReplace, FullV);
  }

  DeadInsts.emplace_back(LF.OperandValToReplace);
}

/// ImplementSolution - Rewrite all the fixup locations with new values,
/// following the chosen solution.
void
LSRInstance::ImplementSolution(const SmallVectorImpl<const Formula *> &Solution,
                               Pass *P) {
  // Keep track of instructions we may have made dead, so that
  // we can remove them after we are done working.
  SmallVector<WeakTrackingVH, 16> DeadInsts;

  SCEVExpander Rewriter(SE, L->getHeader()->getModule()->getDataLayout(),
                        "lsr");
#ifndef NDEBUG
  Rewriter.setDebugType(DEBUG_TYPE);
#endif
  Rewriter.disableCanonicalMode();
  Rewriter.enableLSRMode();
  Rewriter.setIVIncInsertPos(L, IVIncInsertPos);

  // Mark phi nodes that terminate chains so the expander tries to reuse them.
  for (const IVChain &Chain : IVChainVec) {
    if (PHINode *PN = dyn_cast<PHINode>(Chain.tailUserInst()))
      Rewriter.setChainedPhi(PN);
  }

  // Expand the new value definitions and update the users.
  for (const LSRFixup &Fixup : Fixups) {
    Rewrite(Fixup, *Solution[Fixup.LUIdx], Rewriter, DeadInsts, P);

    Changed = true;
  }

  for (const IVChain &Chain : IVChainVec) {
    GenerateIVChain(Chain, Rewriter, DeadInsts);
    Changed = true;
  }
  // Clean up after ourselves. This must be done before deleting any
  // instructions.
  Rewriter.clear();

  Changed |= DeleteTriviallyDeadInstructions(DeadInsts);
}

LSRInstance::LSRInstance(Loop *L, Pass *P)
    : IU(P->getAnalysis<IVUsers>()), SE(P->getAnalysis<ScalarEvolution>()),
      DT(P->getAnalysis<DominatorTreeWrapperPass>().getDomTree()),
      LI(P->getAnalysis<LoopInfoWrapperPass>().getLoopInfo()),
      TTI(P->getAnalysis<TargetTransformInfoWrapperPass>().getTTI(
          *L->getHeader()->getParent())),
      L(L), Changed(false), IVIncInsertPos(nullptr) {
  // If LoopSimplify form is not available, stay out of trouble.
  if (!L->isLoopSimplifyForm())
    return;

  // If there's no interesting work to be done, bail early.
  if (IU.empty()) return;

  // If there's too much analysis to be done, bail early. We won't be able to
  // model the problem anyway.
  unsigned NumUsers = 0;
  for (const IVStrideUse &U : IU) {
    if (++NumUsers > MaxIVUsers) {
      (void)U;
      DEBUG(dbgs() << "LSR skipping loop, too many IV Users in " << U << "\n");
      return;
    }
  }

#ifndef NDEBUG
  // All dominating loops must have preheaders, or SCEVExpander may not be able
  // to materialize an AddRecExpr whose Start is an outer AddRecExpr.
  //
  // IVUsers analysis should only create users that are dominated by simple loop
  // headers. Since this loop should dominate all of its users, its user list
  // should be empty if this loop itself is not within a simple loop nest.
  for (DomTreeNode *Rung = DT.getNode(L->getLoopPreheader());
       Rung; Rung = Rung->getIDom()) {
    BasicBlock *BB = Rung->getBlock();
    const Loop *DomLoop = LI.getLoopFor(BB);
    if (DomLoop && DomLoop->getHeader() == BB) {
      assert(DomLoop->getLoopPreheader() && "LSR needs a simplified loop nest");
    }
  }
#endif // DEBUG

  DEBUG(dbgs() << "\nLSR on loop ";
        L->getHeader()->printAsOperand(dbgs(), /*PrintType=*/false);
        dbgs() << ":\n");

  // First, perform some low-level loop optimizations.
  OptimizeShadowIV();
  OptimizeLoopTermCond();

  // If loop preparation eliminates all interesting IV users, bail.
  if (IU.empty()) return;

  // Skip nested loops until we can model them better with formulae.
  if (!L->empty()) {
    DEBUG(dbgs() << "LSR skipping outer loop " << *L << "\n");
    return;
  }

  // Start collecting data and preparing for the solver.
  CollectChains();
  CollectInterestingTypesAndFactors();
  CollectFixupsAndInitialFormulae();
  CollectLoopInvariantFixupsAndFormulae();

  assert(!Uses.empty() && "IVUsers reported at least one use");
  DEBUG(dbgs() << "LSR found " << Uses.size() << " uses:\n";
        print_uses(dbgs()));

  // Now use the reuse data to generate a bunch of interesting ways
  // to formulate the values needed for the uses.
  GenerateAllReuseFormulae();

  FilterOutUndesirableDedicatedRegisters();
  NarrowSearchSpaceUsingHeuristics();

  SmallVector<const Formula *, 8> Solution;
  Solve(Solution);

  // Release memory that is no longer needed.
  Factors.clear();
  Types.clear();
  RegUses.clear();

  if (Solution.empty())
    return;

#ifndef NDEBUG
  // Formulae should be legal.
  for (const LSRUse &LU : Uses) {
    for (const Formula &F : LU.Formulae)
      assert(isLegalUse(TTI, LU.MinOffset, LU.MaxOffset, LU.Kind, LU.AccessTy,
                        F) && "Illegal formula generated!");
  };
#endif

  // Now that we've decided what we want, make it so.
  ImplementSolution(Solution, P);
}

void LSRInstance::print_factors_and_types(raw_ostream &OS) const {
  if (Factors.empty() && Types.empty()) return;

  OS << "LSR has identified the following interesting factors and types: ";
  bool First = true;

  for (int64_t Factor : Factors) {
    if (!First) OS << ", ";
    First = false;
    OS << '*' << Factor;
  }

  for (Type *Ty : Types) {
    if (!First) OS << ", ";
    First = false;
    OS << '(' << *Ty << ')';
  }
  OS << '\n';
}

void LSRInstance::print_fixups(raw_ostream &OS) const {
  OS << "LSR is examining the following fixup sites:\n";
  for (const LSRFixup &LF : Fixups) {
    dbgs() << "  ";
    LF.print(OS);
    OS << '\n';
  }
}

void LSRInstance::print_uses(raw_ostream &OS) const {
  OS << "LSR is examining the following uses:\n";
  for (const LSRUse &LU : Uses) {
    dbgs() << "  ";
    LU.print(OS);
    OS << '\n';
    for (const Formula &F : LU.Formulae) {
      OS << "    ";
      F.print(OS);
      OS << '\n';
    }
  }
}

void LSRInstance::print(raw_ostream &OS) const {
  print_factors_and_types(OS);
  print_fixups(OS);
  print_uses(OS);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void LSRInstance::dump() const {
  print(errs()); errs() << '\n';
}
#endif

namespace {

class LoopStrengthReduce : public LoopPass {
public:
  static char ID; // Pass ID, replacement for typeid
  LoopStrengthReduce();

private:
  bool runOnLoop(Loop *L, LPPassManager &LPM) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

}

char LoopStrengthReduce::ID = 0;
INITIALIZE_PASS_BEGIN(LoopStrengthReduce, "loop-reduce",
                "Loop Strength Reduction", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolution)
INITIALIZE_PASS_DEPENDENCY(IVUsers)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
INITIALIZE_PASS_END(LoopStrengthReduce, "loop-reduce",
                "Loop Strength Reduction", false, false)


Pass *llvm::createLoopStrengthReducePass() {
  return new LoopStrengthReduce();
}

LoopStrengthReduce::LoopStrengthReduce() : LoopPass(ID) {
  initializeLoopStrengthReducePass(*PassRegistry::getPassRegistry());
}

void LoopStrengthReduce::getAnalysisUsage(AnalysisUsage &AU) const {
  // We split critical edges, so we change the CFG.  However, we do update
  // many analyses if they are around.
  AU.addPreservedID(LoopSimplifyID);

  AU.addRequired<LoopInfoWrapperPass>();
  AU.addPreserved<LoopInfoWrapperPass>();
  AU.addRequiredID(LoopSimplifyID);
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addPreserved<DominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolution>();
  AU.addPreserved<ScalarEvolution>();
  // Requiring LoopSimplify a second time here prevents IVUsers from running
  // twice, since LoopSimplify was invalidated by running ScalarEvolution.
  AU.addRequiredID(LoopSimplifyID);
  AU.addRequired<IVUsers>();
  AU.addPreserved<IVUsers>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
}

bool LoopStrengthReduce::runOnLoop(Loop *L, LPPassManager & /*LPM*/) {
  if (skipOptnoneFunction(L))
    return false;

  bool Changed = false;

  // Run the main LSR transformation.
#if 0 // HLSL Change - move LSRInstance to the heap to avoid >16K stack allocation here.
  Changed |= LSRInstance(L, this).getChanged();
#else
  {
    std::unique_ptr<LSRInstance> instance(new LSRInstance(L, this));
    Changed |= instance->getChanged();
  }
#endif

  // Remove any extra phis created by processing inner loops.
  Changed |= DeleteDeadPHIs(L->getHeader());
  if (EnablePhiElim && L->isLoopSimplifyForm()) {
    SmallVector<WeakTrackingVH, 16> DeadInsts;
    const DataLayout &DL = L->getHeader()->getModule()->getDataLayout();
    SCEVExpander Rewriter(getAnalysis<ScalarEvolution>(), DL, "lsr");
#ifndef NDEBUG
    Rewriter.setDebugType(DEBUG_TYPE);
#endif
    unsigned numFolded = Rewriter.replaceCongruentIVs(
        L, &getAnalysis<DominatorTreeWrapperPass>().getDomTree(), DeadInsts,
        &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(
            *L->getHeader()->getParent()));
    if (numFolded) {
      Changed = true;
      DeleteTriviallyDeadInstructions(DeadInsts);
      DeleteDeadPHIs(L->getHeader());
    }
  }
  return Changed;
}
