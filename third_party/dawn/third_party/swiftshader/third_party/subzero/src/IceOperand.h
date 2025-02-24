//===- subzero/src/IceOperand.h - High-level operands -----------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the Operand class and its target-independent subclasses.
///
/// The main classes are Variable, which represents an LLVM variable that is
/// either register- or stack-allocated, and the Constant hierarchy, which
/// represents integer, floating-point, and/or symbolic constants.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEOPERAND_H
#define SUBZERO_SRC_ICEOPERAND_H

#include "IceCfg.h"
#include "IceDefs.h"
#include "IceGlobalContext.h"
#include "IceStringPool.h"
#include "IceTypes.h"

#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Format.h"

#include <limits>
#include <type_traits>

namespace Ice {

class Operand {
  Operand() = delete;
  Operand(const Operand &) = delete;
  Operand &operator=(const Operand &) = delete;

public:
  static constexpr size_t MaxTargetKinds = 10;
  enum OperandKind {
    kConst_Base,
    kConstInteger32,
    kConstInteger64,
    kConstFloat,
    kConstDouble,
    kConstRelocatable,
    kConstUndef,
    kConst_Target, // leave space for target-specific constant kinds
    kConst_Max = kConst_Target + MaxTargetKinds,
    kVariable,
    kVariable64On32,
    kVariableVecOn32,
    kVariableBoolean,
    kVariable_Target, // leave space for target-specific variable kinds
    kVariable_Max = kVariable_Target + MaxTargetKinds,
    // Target-specific operand classes use kTarget as the starting point for
    // their Kind enum space. Note that the value-spaces are shared across
    // targets. To avoid confusion over the definition of shared values, an
    // object specific to one target should never be passed to a different
    // target.
    kTarget,
    kTarget_Max = std::numeric_limits<uint8_t>::max(),
  };
  static_assert(kTarget <= kTarget_Max, "Must not be above max.");
  OperandKind getKind() const { return Kind; }
  Type getType() const { return Ty; }

  /// Every Operand keeps an array of the Variables referenced in the operand.
  /// This is so that the liveness operations can get quick access to the
  /// variables of interest, without having to dig so far into the operand.
  SizeT getNumVars() const { return NumVars; }
  Variable *getVar(SizeT I) const {
    assert(I < getNumVars());
    return Vars[I];
  }
  virtual void emit(const Cfg *Func) const = 0;

  /// \name Dumping functions.
  /// @{

  /// The dump(Func,Str) implementation must be sure to handle the situation
  /// where Func==nullptr.
  virtual void dump(const Cfg *Func, Ostream &Str) const = 0;
  void dump(const Cfg *Func) const {
    if (!BuildDefs::dump())
      return;
    assert(Func);
    dump(Func, Func->getContext()->getStrDump());
  }
  void dump(Ostream &Str) const {
    if (BuildDefs::dump())
      dump(nullptr, Str);
  }
  /// @}

  virtual ~Operand() = default;

  virtual Variable *asBoolean() { return nullptr; }

  virtual SizeT hashValue() const {
    llvm::report_fatal_error("Tried to hash unsupported operand type : " +
                             std::to_string(Kind));
    return 0;
  }

  inline void *getExternalData() const { return externalData; }
  inline void setExternalData(void *data) { externalData = data; }

protected:
  Operand(OperandKind Kind, Type Ty) : Ty(Ty), Kind(Kind) {
    // It is undefined behavior to have a larger value in the enum
    assert(Kind <= kTarget_Max);
  }

  const Type Ty;
  const OperandKind Kind;
  /// Vars and NumVars are initialized by the derived class.
  SizeT NumVars = 0;
  Variable **Vars = nullptr;

  /// External data can be set by an optimizer to compute and retain any
  /// information related to the current operand. All the memory used to
  /// store this information must be managed by the optimizer.
  void *externalData = nullptr;
};

template <class StreamType>
inline StreamType &operator<<(StreamType &Str, const Operand &Op) {
  Op.dump(Str);
  return Str;
}

/// Constant is the abstract base class for constants. All constants are
/// allocated from a global arena and are pooled.
class Constant : public Operand {
  Constant() = delete;
  Constant(const Constant &) = delete;
  Constant &operator=(const Constant &) = delete;

public:
  // Declare the lookup counter to take minimal space in a non-DUMP build.
  using CounterType =
      std::conditional<BuildDefs::dump(), uint64_t, uint8_t>::type;
  void emit(const Cfg *Func) const override { emit(Func->getTarget()); }
  virtual void emit(TargetLowering *Target) const = 0;

  static bool classof(const Operand *Operand) {
    OperandKind Kind = Operand->getKind();
    return Kind >= kConst_Base && Kind <= kConst_Max;
  }

  const GlobalString getLabelName() const { return LabelName; }

  bool getShouldBePooled() const { return ShouldBePooled; }

  // This should be thread-safe because the constant pool lock is acquired
  // before the method is invoked.
  void updateLookupCount() {
    if (!BuildDefs::dump())
      return;
    ++LookupCount;
  }
  CounterType getLookupCount() const { return LookupCount; }
  SizeT hashValue() const override { return 0; }

protected:
  Constant(OperandKind Kind, Type Ty) : Operand(Kind, Ty) {
    Vars = nullptr;
    NumVars = 0;
  }
  /// Set the ShouldBePooled field to the proper value after the object is fully
  /// initialized.
  void initShouldBePooled();
  GlobalString LabelName;
  /// Whether we should pool this constant. Usually Float/Double and pooled
  /// Integers should be flagged true.  Ideally this field would be const, but
  /// it needs to be initialized only after the subclass is fully constructed.
  bool ShouldBePooled = false;
  /// Note: If ShouldBePooled is ever removed from the base class, we will want
  /// to completely disable LookupCount in a non-DUMP build to save space.
  CounterType LookupCount = 0;
};

/// ConstantPrimitive<> wraps a primitive type.
template <typename T, Operand::OperandKind K>
class ConstantPrimitive : public Constant {
  ConstantPrimitive() = delete;
  ConstantPrimitive(const ConstantPrimitive &) = delete;
  ConstantPrimitive &operator=(const ConstantPrimitive &) = delete;

public:
  using PrimType = T;

  static ConstantPrimitive *create(GlobalContext *Ctx, Type Ty,
                                   PrimType Value) {
    auto *Const =
        new (Ctx->allocate<ConstantPrimitive>()) ConstantPrimitive(Ty, Value);
    Const->initShouldBePooled();
    if (Const->getShouldBePooled())
      Const->initName(Ctx);
    return Const;
  }
  PrimType getValue() const { return Value; }
  using Constant::emit;
  void emit(TargetLowering *Target) const final;
  using Constant::dump;
  void dump(const Cfg *, Ostream &Str) const override {
    if (BuildDefs::dump())
      Str << getValue();
  }

  static bool classof(const Operand *Operand) {
    return Operand->getKind() == K;
  }

  SizeT hashValue() const override { return std::hash<PrimType>()(Value); }

private:
  ConstantPrimitive(Type Ty, PrimType Value) : Constant(K, Ty), Value(Value) {}

  void initName(GlobalContext *Ctx) {
    std::string Buffer;
    llvm::raw_string_ostream Str(Buffer);
    constexpr bool IsCompact = !BuildDefs::dump();
    if (IsCompact) {
      switch (getType()) {
      case IceType_f32:
        Str << "$F";
        break;
      case IceType_f64:
        Str << "$D";
        break;
      default:
        // For constant pooling diversification
        Str << ".L$" << getType() << "$";
        break;
      }
    } else {
      Str << ".L$" << getType() << "$";
    }
    // Print hex characters byte by byte, starting from the most significant
    // byte.  NOTE: This ordering assumes Subzero runs on a little-endian
    // platform.  That means the possibility of different label names depending
    // on the endian-ness of the platform where Subzero runs.
    for (unsigned i = 0; i < sizeof(Value); ++i) {
      constexpr unsigned HexWidthChars = 2;
      unsigned Offset = sizeof(Value) - 1 - i;
      Str << llvm::format_hex_no_prefix(
          *(Offset + (const unsigned char *)&Value), HexWidthChars);
    }
    // For a floating-point value in DecorateAsm mode, also append the value in
    // human-readable sprintf form, changing '+' to 'p' and '-' to 'm' to
    // maintain valid asm labels.
    if (BuildDefs::dump() && std::is_floating_point<PrimType>::value &&
        getFlags().getDecorateAsm()) {
      char Buf[30];
      snprintf(Buf, llvm::array_lengthof(Buf), "$%g", (double)Value);
      for (unsigned i = 0; i < llvm::array_lengthof(Buf) && Buf[i]; ++i) {
        if (Buf[i] == '-')
          Buf[i] = 'm';
        else if (Buf[i] == '+')
          Buf[i] = 'p';
      }
      Str << Buf;
    }
    LabelName = GlobalString::createWithString(Ctx, Str.str());
  }

  const PrimType Value;
};

using ConstantInteger32 = ConstantPrimitive<int32_t, Operand::kConstInteger32>;
using ConstantInteger64 = ConstantPrimitive<int64_t, Operand::kConstInteger64>;
using ConstantFloat = ConstantPrimitive<float, Operand::kConstFloat>;
using ConstantDouble = ConstantPrimitive<double, Operand::kConstDouble>;

template <>
inline void ConstantInteger32::dump(const Cfg *, Ostream &Str) const {
  if (!BuildDefs::dump())
    return;
  if (getType() == IceType_i1)
    Str << (getValue() ? "true" : "false");
  else
    Str << static_cast<int32_t>(getValue());
}

template <>
inline void ConstantInteger64::dump(const Cfg *, Ostream &Str) const {
  if (!BuildDefs::dump())
    return;
  assert(getType() == IceType_i64);
  Str << static_cast<int64_t>(getValue());
}

/// RelocOffset allows symbolic references in ConstantRelocatables' offsets,
/// e.g., 8 + LabelOffset, where label offset is the location (code or data)
/// of a Label that is only determinable during ELF emission.
class RelocOffset final {
  RelocOffset(const RelocOffset &) = delete;
  RelocOffset &operator=(const RelocOffset &) = delete;

public:
  template <typename T> static RelocOffset *create(T *AllocOwner) {
    return new (AllocOwner->template allocate<RelocOffset>()) RelocOffset();
  }

  static RelocOffset *create(GlobalContext *Ctx, RelocOffsetT Value) {
    return new (Ctx->allocate<RelocOffset>()) RelocOffset(Value);
  }

  void setSubtract(bool Value) { Subtract = Value; }
  bool hasOffset() const { return HasOffset; }

  RelocOffsetT getOffset() const {
    assert(HasOffset);
    return Offset;
  }

  void setOffset(const RelocOffsetT Value) {
    assert(!HasOffset);
    if (Subtract) {
      assert(Value != std::numeric_limits<RelocOffsetT>::lowest());
      Offset = -Value;
    } else {
      Offset = Value;
    }
    HasOffset = true;
  }

private:
  RelocOffset() = default;
  explicit RelocOffset(RelocOffsetT Offset) { setOffset(Offset); }

  bool Subtract = false;
  bool HasOffset = false;
  RelocOffsetT Offset;
};

/// RelocatableTuple bundles the parameters that are used to construct an
/// ConstantRelocatable. It is done this way so that ConstantRelocatable can fit
/// into the global constant pool template mechanism.
class RelocatableTuple {
  RelocatableTuple() = delete;
  RelocatableTuple &operator=(const RelocatableTuple &) = delete;

public:
  RelocatableTuple(const RelocOffsetT Offset,
                   const RelocOffsetArray &OffsetExpr, GlobalString Name)
      : Offset(Offset), OffsetExpr(OffsetExpr), Name(Name) {}

  RelocatableTuple(const RelocOffsetT Offset,
                   const RelocOffsetArray &OffsetExpr, GlobalString Name,
                   const std::string &EmitString)
      : Offset(Offset), OffsetExpr(OffsetExpr), Name(Name),
        EmitString(EmitString) {}

  RelocatableTuple(const RelocatableTuple &) = default;

  const RelocOffsetT Offset;
  const RelocOffsetArray OffsetExpr;
  const GlobalString Name;
  const std::string EmitString;
};

bool operator==(const RelocatableTuple &A, const RelocatableTuple &B);

/// ConstantRelocatable represents a symbolic constant combined with a fixed
/// offset.
class ConstantRelocatable : public Constant {
  ConstantRelocatable() = delete;
  ConstantRelocatable(const ConstantRelocatable &) = delete;
  ConstantRelocatable &operator=(const ConstantRelocatable &) = delete;

public:
  template <typename T>
  static ConstantRelocatable *create(T *AllocOwner, Type Ty,
                                     const RelocatableTuple &Tuple) {
    return new (AllocOwner->template allocate<ConstantRelocatable>())
        ConstantRelocatable(Ty, Tuple.Offset, Tuple.OffsetExpr, Tuple.Name,
                            Tuple.EmitString);
  }

  RelocOffsetT getOffset() const {
    RelocOffsetT Ret = Offset;
    for (const auto *const OffsetReloc : OffsetExpr) {
      Ret += OffsetReloc->getOffset();
    }
    return Ret;
  }

  const std::string &getEmitString() const { return EmitString; }

  GlobalString getName() const { return Name; }
  using Constant::emit;
  void emit(TargetLowering *Target) const final;
  void emitWithoutPrefix(const TargetLowering *Target,
                         const char *Suffix = "") const;
  using Constant::dump;
  void dump(const Cfg *Func, Ostream &Str) const override;

  static bool classof(const Operand *Operand) {
    OperandKind Kind = Operand->getKind();
    return Kind == kConstRelocatable;
  }

private:
  ConstantRelocatable(Type Ty, const RelocOffsetT Offset,
                      const RelocOffsetArray &OffsetExpr, GlobalString Name,
                      const std::string &EmitString)
      : Constant(kConstRelocatable, Ty), Offset(Offset), OffsetExpr(OffsetExpr),
        Name(Name), EmitString(EmitString) {}

  const RelocOffsetT Offset;         /// fixed, known offset to add
  const RelocOffsetArray OffsetExpr; /// fixed, unknown offset to add
  const GlobalString Name;           /// optional for debug/dump
  const std::string EmitString;      /// optional for textual emission
};

/// ConstantUndef represents an unspecified bit pattern. Although it is legal to
/// lower ConstantUndef to any value, backends should try to make code
/// generation deterministic by lowering ConstantUndefs to 0.
class ConstantUndef : public Constant {
  ConstantUndef() = delete;
  ConstantUndef(const ConstantUndef &) = delete;
  ConstantUndef &operator=(const ConstantUndef &) = delete;

public:
  static ConstantUndef *create(GlobalContext *Ctx, Type Ty) {
    return new (Ctx->allocate<ConstantUndef>()) ConstantUndef(Ty);
  }

  using Constant::emit;
  void emit(TargetLowering *Target) const final;
  using Constant::dump;
  void dump(const Cfg *, Ostream &Str) const override {
    if (BuildDefs::dump())
      Str << "undef";
  }

  static bool classof(const Operand *Operand) {
    return Operand->getKind() == kConstUndef;
  }

private:
  ConstantUndef(Type Ty) : Constant(kConstUndef, Ty) {}
};

/// RegNumT is for holding target-specific register numbers, plus the sentinel
/// value if no register is assigned. Its public ctor allows direct use of enum
/// values, such as RegNumT(Reg_eax), but not things like RegNumT(Reg_eax+1).
/// This is to try to prevent inappropriate assumptions about enum ordering. If
/// needed, the fromInt() method can be used, such as when a RegNumT is based
/// on a bitvector index.
class RegNumT {
public:
  using BaseType = uint32_t;
  RegNumT() = default;
  RegNumT(const RegNumT &) = default;
  template <typename AnyEnum>
  RegNumT(AnyEnum Value,
          typename std::enable_if<std::is_enum<AnyEnum>::value, int>::type = 0)
      : Value(Value) {
    validate(Value);
  }
  RegNumT &operator=(const RegNumT &) = default;
  operator unsigned() const { return Value; }
  /// Asserts that the register is valid, i.e. not NoRegisterValue.  Note that
  /// the ctor already does the target-specific limit check.
  void assertIsValid() const { assert(Value != NoRegisterValue); }
  static RegNumT fromInt(BaseType Value) { return RegNumT(Value); }
  /// Marks cases that inappropriately add/subtract RegNumT values, and
  /// therefore need to be fixed because they make assumptions about register
  /// enum value ordering.  TODO(stichnot): Remove fixme() as soon as all
  /// current uses are fixed/removed.
  static RegNumT fixme(BaseType Value) { return RegNumT(Value); }
  /// The target's staticInit() method should call setLimit() to register the
  /// upper bound of allowable values.
  static void setLimit(BaseType Value) {
    // Make sure it's only called once.
    assert(Limit == 0);
    assert(Value != 0);
    Limit = Value;
  }
  // Define NoRegisterValue as an enum value so that it can be used as an
  // argument for the public ctor if desired.
  enum : BaseType { NoRegisterValue = std::numeric_limits<BaseType>::max() };

  bool hasValue() const { return Value != NoRegisterValue; }
  bool hasNoValue() const { return !hasValue(); }

private:
  BaseType Value = NoRegisterValue;
  static BaseType Limit;
  /// Private ctor called only by fromInt() and fixme().
  RegNumT(BaseType Value) : Value(Value) { validate(Value); }
  /// The ctor calls this to validate against the target-supplied limit.
  static void validate(BaseType Value) {
    (void)Value;
    assert(Value == NoRegisterValue || Value < Limit);
  }
  /// Disallow operators that inappropriately make assumptions about register
  /// enum value ordering.
  bool operator<(const RegNumT &) = delete;
  bool operator<=(const RegNumT &) = delete;
  bool operator>(const RegNumT &) = delete;
  bool operator>=(const RegNumT &) = delete;
};

/// RegNumBVIter wraps SmallBitVector so that instead of this pattern:
///
///   for (int i = V.find_first(); i != -1; i = V.find_next(i)) {
///     RegNumT RegNum = RegNumT::fromInt(i);
///     ...
///   }
///
/// this cleaner pattern can be used:
///
///   for (RegNumT RegNum : RegNumBVIter(V)) {
///     ...
///   }
template <class B> class RegNumBVIterImpl {
  using T = B;
  static constexpr int Sentinel = -1;
  RegNumBVIterImpl() = delete;

public:
  class Iterator {
    Iterator() = delete;
    Iterator &operator=(const Iterator &) = delete;

  public:
    explicit Iterator(const T &V) : V(V), Current(V.find_first()) {}
    Iterator(const T &V, int Value) : V(V), Current(Value) {}
    Iterator(const Iterator &) = default;
    RegNumT operator*() {
      assert(Current != Sentinel);
      return RegNumT::fromInt(Current);
    }
    Iterator &operator++() {
      assert(Current != Sentinel);
      Current = V.find_next(Current);
      return *this;
    }
    bool operator!=(Iterator &Other) { return Current != Other.Current; }

  private:
    const T &V;
    int Current;
  };

  RegNumBVIterImpl(const RegNumBVIterImpl &) = default;
  RegNumBVIterImpl &operator=(const RegNumBVIterImpl &) = delete;
  explicit RegNumBVIterImpl(const T &V) : V(V) {}
  Iterator begin() { return Iterator(V); }
  Iterator end() { return Iterator(V, Sentinel); }

private:
  const T &V;
};

template <class B> RegNumBVIterImpl<B> RegNumBVIter(const B &BV) {
  return RegNumBVIterImpl<B>(BV);
}

/// RegWeight is a wrapper for a uint32_t weight value, with a special value
/// that represents infinite weight, and an addWeight() method that ensures that
/// W+infinity=infinity.
class RegWeight {
public:
  using BaseType = uint32_t;
  RegWeight() = default;
  explicit RegWeight(BaseType Weight) : Weight(Weight) {}
  RegWeight(const RegWeight &) = default;
  RegWeight &operator=(const RegWeight &) = default;
  constexpr static BaseType Inf = ~0; /// Force regalloc to give a register
  constexpr static BaseType Zero = 0; /// Force regalloc NOT to give a register
  constexpr static BaseType Max = Inf - 1; /// Max natural weight.
  void addWeight(BaseType Delta) {
    if (Delta == Inf)
      Weight = Inf;
    else if (Weight != Inf)
      if (Utils::add_overflow(Weight, Delta, &Weight) || Weight == Inf)
        Weight = Max;
  }
  void addWeight(const RegWeight &Other) { addWeight(Other.Weight); }
  void setWeight(BaseType Val) { Weight = Val; }
  BaseType getWeight() const { return Weight; }

private:
  BaseType Weight = 0;
};
Ostream &operator<<(Ostream &Str, const RegWeight &W);
bool operator<(const RegWeight &A, const RegWeight &B);
bool operator<=(const RegWeight &A, const RegWeight &B);
bool operator==(const RegWeight &A, const RegWeight &B);

/// LiveRange is a set of instruction number intervals representing a variable's
/// live range. Generally there is one interval per basic block where the
/// variable is live, but adjacent intervals get coalesced into a single
/// interval.
class LiveRange {
public:
  using RangeElementType = std::pair<InstNumberT, InstNumberT>;
  /// RangeType is arena-allocated from the Cfg's allocator.
  using RangeType = CfgVector<RangeElementType>;
  LiveRange() = default;
  /// Special constructor for building a kill set. The advantage is that we can
  /// reserve the right amount of space in advance.
  explicit LiveRange(const CfgVector<InstNumberT> &Kills) {
    Range.reserve(Kills.size());
    for (InstNumberT I : Kills)
      addSegment(I, I);
  }
  LiveRange(const LiveRange &) = default;
  LiveRange &operator=(const LiveRange &) = default;

  void reset() {
    Range.clear();
    untrim();
  }
  void addSegment(InstNumberT Start, InstNumberT End, CfgNode *Node = nullptr);
  void addSegment(RangeElementType Segment, CfgNode *Node = nullptr) {
    addSegment(Segment.first, Segment.second, Node);
  }

  bool endsBefore(const LiveRange &Other) const;
  bool overlaps(const LiveRange &Other, bool UseTrimmed = false) const;
  bool overlapsInst(InstNumberT OtherBegin, bool UseTrimmed = false) const;
  bool containsValue(InstNumberT Value, bool IsDest) const;
  bool isEmpty() const { return Range.empty(); }
  InstNumberT getStart() const {
    return Range.empty() ? -1 : Range.begin()->first;
  }
  InstNumberT getEnd() const {
    return Range.empty() ? -1 : Range.rbegin()->second;
  }

  void untrim() { TrimmedBegin = Range.begin(); }
  void trim(InstNumberT Lower);

  void dump(Ostream &Str) const;

  SizeT getNumSegments() const { return Range.size(); }

  const RangeType &getSegments() const { return Range; }
  CfgNode *getNodeForSegment(InstNumberT Begin) {
    auto Iter = NodeMap.find(Begin);
    assert(Iter != NodeMap.end());
    return Iter->second;
  }

private:
  RangeType Range;
  CfgUnorderedMap<InstNumberT, CfgNode *> NodeMap;
  /// TrimmedBegin is an optimization for the overlaps() computation. Since the
  /// linear-scan algorithm always calls it as overlaps(Cur) and Cur advances
  /// monotonically according to live range start, we can optimize overlaps() by
  /// ignoring all segments that end before the start of Cur's range. The
  /// linear-scan code enables this by calling trim() on the ranges of interest
  /// as Cur advances. Note that linear-scan also has to initialize TrimmedBegin
  /// at the beginning by calling untrim().
  RangeType::const_iterator TrimmedBegin;
};

Ostream &operator<<(Ostream &Str, const LiveRange &L);

/// Variable represents an operand that is register-allocated or
/// stack-allocated. If it is register-allocated, it will ultimately have a
/// valid RegNum field.
class Variable : public Operand {
  Variable() = delete;
  Variable(const Variable &) = delete;
  Variable &operator=(const Variable &) = delete;

  enum RegRequirement : uint8_t {
    RR_MayHaveRegister,
    RR_MustHaveRegister,
    RR_MustNotHaveRegister,
  };

public:
  static Variable *create(Cfg *Func, Type Ty, SizeT Index) {
    return new (Func->allocate<Variable>())
        Variable(Func, kVariable, Ty, Index);
  }

  SizeT getIndex() const { return Number; }
  std::string getName() const {
    if (Name.hasStdString())
      return Name.toString();
    return "__" + std::to_string(getIndex());
  }
  virtual void setName(const Cfg *Func, const std::string &NewName) {
    if (NewName.empty())
      return;
    Name = VariableString::createWithString(Func, NewName);
  }

  bool getIsArg() const { return IsArgument; }
  virtual void setIsArg(bool Val = true) { IsArgument = Val; }
  bool getIsImplicitArg() const { return IsImplicitArgument; }
  void setIsImplicitArg(bool Val = true) { IsImplicitArgument = Val; }

  void setIgnoreLiveness() { IgnoreLiveness = true; }
  bool getIgnoreLiveness() const {
    return IgnoreLiveness || IsRematerializable;
  }

  /// Returns true if the variable either has a definite stack offset, or has
  /// the UndeterminedStackOffset such that it is guaranteed to have a definite
  /// stack offset at emission time.
  bool hasStackOffset() const { return StackOffset != InvalidStackOffset; }
  /// Returns true if the variable has a stack offset that is known at this
  /// time.
  bool hasKnownStackOffset() const {
    return StackOffset != InvalidStackOffset &&
           StackOffset != UndeterminedStackOffset;
  }
  int32_t getStackOffset() const {
    assert(hasKnownStackOffset());
    return StackOffset;
  }
  void setStackOffset(int32_t Offset) { StackOffset = Offset; }
  /// Set a "placeholder" stack offset before its actual offset has been
  /// determined.
  void setHasStackOffset() {
    if (!hasStackOffset())
      StackOffset = UndeterminedStackOffset;
  }
  /// Returns the variable's stack offset in symbolic form, to improve
  /// readability in DecorateAsm mode.
  std::string getSymbolicStackOffset() const {
    if (!BuildDefs::dump())
      return "";
    return ".L$lv$" + getName();
  }

  bool hasReg() const { return getRegNum().hasValue(); }
  RegNumT getRegNum() const { return RegNum; }
  void setRegNum(RegNumT NewRegNum) {
    // Regnum shouldn't be set more than once.
    assert(!hasReg() || RegNum == NewRegNum);
    RegNum = NewRegNum;
  }
  bool hasRegTmp() const { return getRegNumTmp().hasValue(); }
  RegNumT getRegNumTmp() const { return RegNumTmp; }
  void setRegNumTmp(RegNumT NewRegNum) { RegNumTmp = NewRegNum; }

  RegWeight getWeight(const Cfg *Func) const;

  void setMustHaveReg() { RegRequirement = RR_MustHaveRegister; }
  bool mustHaveReg() const { return RegRequirement == RR_MustHaveRegister; }
  void setMustNotHaveReg() { RegRequirement = RR_MustNotHaveRegister; }
  bool mustNotHaveReg() const {
    return RegRequirement == RR_MustNotHaveRegister;
  }
  bool mayHaveReg() const { return RegRequirement == RR_MayHaveRegister; }
  void setRematerializable(RegNumT NewRegNum, int32_t NewOffset) {
    IsRematerializable = true;
    setRegNum(NewRegNum);
    setStackOffset(NewOffset);
    setMustHaveReg();
  }
  bool isRematerializable() const { return IsRematerializable; }
  int32_t getRematerializableOffset(const ::Ice::TargetLowering *Target);

  void setRegClass(uint8_t RC) { RegisterClass = static_cast<RegClass>(RC); }
  RegClass getRegClass() const { return RegisterClass; }

  LiveRange &getLiveRange() { return Live; }
  const LiveRange &getLiveRange() const { return Live; }
  void setLiveRange(const LiveRange &Range) { Live = Range; }
  void resetLiveRange() { Live.reset(); }
  void addLiveRange(InstNumberT Start, InstNumberT End,
                    CfgNode *Node = nullptr) {
    assert(!getIgnoreLiveness());
    Live.addSegment(Start, End, Node);
  }
  void trimLiveRange(InstNumberT Start) { Live.trim(Start); }
  void untrimLiveRange() { Live.untrim(); }
  bool rangeEndsBefore(const Variable *Other) const {
    return Live.endsBefore(Other->Live);
  }
  bool rangeOverlaps(const Variable *Other) const {
    constexpr bool UseTrimmed = true;
    return Live.overlaps(Other->Live, UseTrimmed);
  }
  bool rangeOverlapsStart(const Variable *Other) const {
    constexpr bool UseTrimmed = true;
    return Live.overlapsInst(Other->Live.getStart(), UseTrimmed);
  }

  /// Creates a temporary copy of the variable with a different type. Used
  /// primarily for syntactic correctness of textual assembly emission. Note
  /// that only basic information is copied, in particular not IsArgument,
  /// IsImplicitArgument, IgnoreLiveness, RegNumTmp, Live, LoVar, HiVar,
  /// VarsReal. If NewRegNum.hasValue(), then that register assignment is made
  /// instead of copying the existing assignment.
  const Variable *asType(const Cfg *Func, Type Ty, RegNumT NewRegNum) const;

  void emit(const Cfg *Func) const override;
  using Operand::dump;
  void dump(const Cfg *Func, Ostream &Str) const override;

  /// Return reg num of base register, if different from stack/frame register.
  virtual RegNumT getBaseRegNum() const { return RegNumT(); }

  /// Access the LinkedTo field.
  void setLinkedTo(Variable *Var) { LinkedTo = Var; }
  Variable *getLinkedTo() const { return LinkedTo; }
  /// Follow the LinkedTo chain up to the furthest ancestor.
  Variable *getLinkedToRoot() const {
    Variable *Root = LinkedTo;
    if (Root == nullptr)
      return nullptr;
    while (Root->LinkedTo != nullptr)
      Root = Root->LinkedTo;
    return Root;
  }
  /// Follow the LinkedTo chain up to the furthest stack-allocated ancestor.
  /// This is only certain to be accurate after register allocation and stack
  /// slot assignment have completed.
  Variable *getLinkedToStackRoot() const {
    Variable *FurthestStackVar = nullptr;
    for (Variable *Root = LinkedTo; Root != nullptr; Root = Root->LinkedTo) {
      if (!Root->hasReg() && Root->hasStackOffset()) {
        FurthestStackVar = Root;
      }
    }
    return FurthestStackVar;
  }

  static bool classof(const Operand *Operand) {
    OperandKind Kind = Operand->getKind();
    return Kind >= kVariable && Kind <= kVariable_Max;
  }

  SizeT hashValue() const override { return std::hash<SizeT>()(getIndex()); }

  inline void *getExternalData() const { return externalData; }
  inline void setExternalData(void *data) { externalData = data; }

protected:
  Variable(const Cfg *Func, OperandKind K, Type Ty, SizeT Index)
      : Operand(K, Ty), Number(Index),
        Name(VariableString::createWithoutString(Func)),
        RegisterClass(static_cast<RegClass>(Ty)) {
    Vars = VarsReal;
    Vars[0] = this;
    NumVars = 1;
  }
  /// Number is unique across all variables, and is used as a (bit)vector index
  /// for liveness analysis.
  const SizeT Number;
  VariableString Name;
  bool IsArgument = false;
  bool IsImplicitArgument = false;
  /// IgnoreLiveness means that the variable should be ignored when constructing
  /// and validating live ranges. This is usually reserved for the stack
  /// pointer and other physical registers specifically referenced by name.
  bool IgnoreLiveness = false;
  // If IsRematerializable, RegNum keeps track of which register (stack or frame
  // pointer), and StackOffset is the known offset from that register.
  bool IsRematerializable = false;
  RegRequirement RegRequirement = RR_MayHaveRegister;
  RegClass RegisterClass;
  /// RegNum is the allocated register, (as long as RegNum.hasValue() is true).
  RegNumT RegNum;
  /// RegNumTmp is the tentative assignment during register allocation.
  RegNumT RegNumTmp;
  static constexpr int32_t InvalidStackOffset =
      std::numeric_limits<int32_t>::min();
  static constexpr int32_t UndeterminedStackOffset =
      1 + std::numeric_limits<int32_t>::min();
  /// StackOffset is the canonical location on stack (only if
  /// RegNum.hasNoValue() || IsArgument).
  int32_t StackOffset = InvalidStackOffset;
  LiveRange Live;
  /// VarsReal (and Operand::Vars) are set up such that Vars[0] == this.
  Variable *VarsReal[1];
  /// This Variable may be "linked" to another Variable, such that if neither
  /// Variable gets a register, they are guaranteed to share a stack location.
  Variable *LinkedTo = nullptr;
  /// External data can be set by an optimizer to compute and retain any
  /// information related to the current variable. All the memory used to
  /// store this information must be managed by the optimizer.
  void *externalData = nullptr;
};

// Variable64On32 represents a 64-bit variable on a 32-bit architecture. In
// this situation the variable must be split into a low and a high word.
class Variable64On32 : public Variable {
  Variable64On32() = delete;
  Variable64On32(const Variable64On32 &) = delete;
  Variable64On32 &operator=(const Variable64On32 &) = delete;

public:
  static Variable64On32 *create(Cfg *Func, Type Ty, SizeT Index) {
    return new (Func->allocate<Variable64On32>())
        Variable64On32(Func, kVariable64On32, Ty, Index);
  }

  void setName(const Cfg *Func, const std::string &NewName) override {
    Variable::setName(Func, NewName);
    if (LoVar && HiVar) {
      LoVar->setName(Func, getName() + "__lo");
      HiVar->setName(Func, getName() + "__hi");
    }
  }

  void setIsArg(bool Val = true) override {
    Variable::setIsArg(Val);
    if (LoVar && HiVar) {
      LoVar->setIsArg(Val);
      HiVar->setIsArg(Val);
    }
  }

  Variable *getLo() const {
    assert(LoVar != nullptr);
    return LoVar;
  }
  Variable *getHi() const {
    assert(HiVar != nullptr);
    return HiVar;
  }

  void initHiLo(Cfg *Func) {
    assert(LoVar == nullptr);
    assert(HiVar == nullptr);
    LoVar = Func->makeVariable(IceType_i32);
    HiVar = Func->makeVariable(IceType_i32);
    LoVar->setIsArg(getIsArg());
    HiVar->setIsArg(getIsArg());
    if (BuildDefs::dump()) {
      LoVar->setName(Func, getName() + "__lo");
      HiVar->setName(Func, getName() + "__hi");
    }
  }

  static bool classof(const Operand *Operand) {
    OperandKind Kind = Operand->getKind();
    return Kind == kVariable64On32;
  }

protected:
  Variable64On32(const Cfg *Func, OperandKind K, Type Ty, SizeT Index)
      : Variable(Func, K, Ty, Index) {
    assert(typeWidthInBytes(Ty) == 8);
  }

  Variable *LoVar = nullptr;
  Variable *HiVar = nullptr;
};

// VariableVecOn32 represents a 128-bit vector variable on a 32-bit
// architecture. In this case the variable must be split into 4 containers.
class VariableVecOn32 : public Variable {
  VariableVecOn32() = delete;
  VariableVecOn32(const VariableVecOn32 &) = delete;
  VariableVecOn32 &operator=(const VariableVecOn32 &) = delete;

public:
  static VariableVecOn32 *create(Cfg *Func, Type Ty, SizeT Index) {
    return new (Func->allocate<VariableVecOn32>())
        VariableVecOn32(Func, kVariableVecOn32, Ty, Index);
  }

  void setName(const Cfg *Func, const std::string &NewName) override {
    Variable::setName(Func, NewName);
    if (!Containers.empty()) {
      for (SizeT i = 0; i < ContainersPerVector; ++i) {
        Containers[i]->setName(Func, getName() + "__cont" + std::to_string(i));
      }
    }
  }

  void setIsArg(bool Val = true) override {
    Variable::setIsArg(Val);
    for (Variable *Var : Containers) {
      Var->setIsArg(getIsArg());
    }
  }

  const VarList &getContainers() const { return Containers; }

  void initVecElement(Cfg *Func) {
    for (SizeT i = 0; i < ContainersPerVector; ++i) {
      Variable *Var = Func->makeVariable(IceType_i32);
      Var->setIsArg(getIsArg());
      if (BuildDefs::dump()) {
        Var->setName(Func, getName() + "__cont" + std::to_string(i));
      }
      Containers.push_back(Var);
    }
  }

  static bool classof(const Operand *Operand) {
    OperandKind Kind = Operand->getKind();
    return Kind == kVariableVecOn32;
  }

  // A 128-bit vector value is mapped onto 4 32-bit register values.
  static constexpr SizeT ContainersPerVector = 4;

protected:
  VariableVecOn32(const Cfg *Func, OperandKind K, Type Ty, SizeT Index)
      : Variable(Func, K, Ty, Index) {
    assert(typeWidthInBytes(Ty) ==
           ContainersPerVector * typeWidthInBytes(IceType_i32));
  }

  VarList Containers;
};

enum MetadataKind {
  VMK_Uses,       /// Track only uses, not defs
  VMK_SingleDefs, /// Track uses+defs, but only record single def
  VMK_All         /// Track uses+defs, including full def list
};
using InstDefList = CfgVector<const Inst *>;

/// VariableTracking tracks the metadata for a single variable.  It is
/// only meant to be used internally by VariablesMetadata.
class VariableTracking {
public:
  enum MultiDefState {
    // TODO(stichnot): Consider using just a simple counter.
    MDS_Unknown,
    MDS_SingleDef,
    MDS_MultiDefSingleBlock,
    MDS_MultiDefMultiBlock
  };
  enum MultiBlockState {
    MBS_Unknown,     // Not yet initialized, so be conservative
    MBS_NoUses,      // Known to have no uses
    MBS_SingleBlock, // All uses in are in a single block
    MBS_MultiBlock   // Several uses across several blocks
  };
  VariableTracking() = default;
  VariableTracking(const VariableTracking &) = default;
  VariableTracking &operator=(const VariableTracking &) = default;
  VariableTracking(MultiBlockState MultiBlock) : MultiBlock(MultiBlock) {}
  MultiDefState getMultiDef() const { return MultiDef; }
  MultiBlockState getMultiBlock() const { return MultiBlock; }
  const Inst *getFirstDefinitionSingleBlock() const;
  const Inst *getSingleDefinition() const;
  const Inst *getFirstDefinition() const;
  const InstDefList &getLatterDefinitions() const { return Definitions; }
  CfgNode *getNode() const { return SingleUseNode; }
  RegWeight getUseWeight() const { return UseWeight; }
  void markUse(MetadataKind TrackingKind, const Inst *Instr, CfgNode *Node,
               bool IsImplicit);
  void markDef(MetadataKind TrackingKind, const Inst *Instr, CfgNode *Node);

private:
  MultiDefState MultiDef = MDS_Unknown;
  MultiBlockState MultiBlock = MBS_Unknown;
  CfgNode *SingleUseNode = nullptr;
  CfgNode *SingleDefNode = nullptr;
  /// All definitions of the variable are collected in Definitions[] (except for
  /// the earliest definition), in increasing order of instruction number.
  InstDefList Definitions; /// Only used if Kind==VMK_All
  const Inst *FirstOrSingleDefinition = nullptr;
  RegWeight UseWeight;
};

/// VariablesMetadata analyzes and summarizes the metadata for the complete set
/// of Variables.
class VariablesMetadata {
  VariablesMetadata() = delete;
  VariablesMetadata(const VariablesMetadata &) = delete;
  VariablesMetadata &operator=(const VariablesMetadata &) = delete;

public:
  explicit VariablesMetadata(const Cfg *Func) : Func(Func) {}
  /// Initialize the state by traversing all instructions/variables in the CFG.
  void init(MetadataKind TrackingKind);
  /// Add a single node. This is called by init(), and can be called
  /// incrementally from elsewhere, e.g. after edge-splitting.
  void addNode(CfgNode *Node);
  MetadataKind getKind() const { return Kind; }
  /// Returns whether the given Variable is tracked in this object. It should
  /// only return false if changes were made to the CFG after running init(), in
  /// which case the state is stale and the results shouldn't be trusted (but it
  /// may be OK e.g. for dumping).
  bool isTracked(const Variable *Var) const {
    return Var->getIndex() < Metadata.size();
  }

  /// Returns whether the given Variable has multiple definitions.
  bool isMultiDef(const Variable *Var) const;
  /// Returns the first definition instruction of the given Variable. This is
  /// only valid for variables whose definitions are all within the same block,
  /// e.g. T after the lowered sequence "T=B; T+=C; A=T", for which
  /// getFirstDefinitionSingleBlock(T) would return the "T=B" instruction. For
  /// variables with definitions span multiple blocks, nullptr is returned.
  const Inst *getFirstDefinitionSingleBlock(const Variable *Var) const;
  /// Returns the definition instruction of the given Variable, when the
  /// variable has exactly one definition. Otherwise, nullptr is returned.
  const Inst *getSingleDefinition(const Variable *Var) const;
  /// getFirstDefinition() and getLatterDefinitions() are used together to
  /// return the complete set of instructions that define the given Variable,
  /// regardless of whether the definitions are within the same block (in
  /// contrast to getFirstDefinitionSingleBlock).
  const Inst *getFirstDefinition(const Variable *Var) const;
  const InstDefList &getLatterDefinitions(const Variable *Var) const;

  /// Returns whether the given Variable is live across multiple blocks. Mainly,
  /// this is used to partition Variables into single-block versus multi-block
  /// sets for leveraging sparsity in liveness analysis, and for implementing
  /// simple stack slot coalescing. As a special case, function arguments are
  /// always considered multi-block because they are live coming into the entry
  /// block.
  bool isMultiBlock(const Variable *Var) const;
  bool isSingleBlock(const Variable *Var) const;
  /// Returns the node that the given Variable is used in, assuming
  /// isMultiBlock() returns false. Otherwise, nullptr is returned.
  CfgNode *getLocalUseNode(const Variable *Var) const;

  /// Returns the total use weight computed as the sum of uses multiplied by a
  /// loop nest depth factor for each use.
  RegWeight getUseWeight(const Variable *Var) const;

private:
  const Cfg *Func;
  MetadataKind Kind;
  CfgVector<VariableTracking> Metadata;
  static const InstDefList *NoDefinitions;
};

/// BooleanVariable represents a variable that was the zero-extended result of a
/// comparison. It maintains a pointer to its original i1 source so that the
/// WASM frontend can avoid adding needless comparisons.
class BooleanVariable : public Variable {
  BooleanVariable() = delete;
  BooleanVariable(const BooleanVariable &) = delete;
  BooleanVariable &operator=(const BooleanVariable &) = delete;

  BooleanVariable(const Cfg *Func, OperandKind K, Type Ty, SizeT Index)
      : Variable(Func, K, Ty, Index) {}

public:
  static BooleanVariable *create(Cfg *Func, Type Ty, SizeT Index) {
    return new (Func->allocate<BooleanVariable>())
        BooleanVariable(Func, kVariable, Ty, Index);
  }

  virtual Variable *asBoolean() { return BoolSource; }

  void setBoolSource(Variable *Src) { BoolSource = Src; }

  static bool classof(const Operand *Operand) {
    return Operand->getKind() == kVariableBoolean;
  }

private:
  Variable *BoolSource = nullptr;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEOPERAND_H
