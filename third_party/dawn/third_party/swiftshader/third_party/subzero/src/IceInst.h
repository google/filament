//===- subzero/src/IceInst.h - High-level instructions ----------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the Inst class and its target-independent subclasses.
///
/// These represent the high-level Vanilla ICE instructions and map roughly 1:1
/// to LLVM instructions.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEINST_H
#define SUBZERO_SRC_ICEINST_H

#include "IceCfg.h"
#include "IceDefs.h"
#include "IceInst.def"
#include "IceIntrinsics.h"
#include "IceOperand.h"
#include "IceSwitchLowering.h"
#include "IceTypes.h"

// TODO: The Cfg structure, and instructions in particular, need to be
// validated for things like valid operand types, valid branch targets, proper
// ordering of Phi and non-Phi instructions, etc. Most of the validity checking
// will be done in the bitcode reader. We need a list of everything that should
// be validated, and tests for each.

namespace Ice {

/// Base instruction class for ICE. Inst has two subclasses: InstHighLevel and
/// InstTarget. High-level ICE instructions inherit from InstHighLevel, and
/// low-level (target-specific) ICE instructions inherit from InstTarget.
class Inst : public llvm::ilist_node<Inst> {
  Inst() = delete;
  Inst(const Inst &) = delete;
  Inst &operator=(const Inst &) = delete;

public:
  enum InstKind {
    // Arbitrary (alphabetical) order, except put Unreachable first.
    Unreachable,
    Alloca,
    Arithmetic,
    Br,
    Call,
    Cast,
    ExtractElement,
    Fcmp,
    Icmp,
    Intrinsic,
    InsertElement,
    Load,
    Phi,
    Ret,
    Select,
    Store,
    Switch,
    Assign,        // not part of LLVM/PNaCl bitcode
    Breakpoint,    // not part of LLVM/PNaCl bitcode
    FakeDef,       // not part of LLVM/PNaCl bitcode
    FakeUse,       // not part of LLVM/PNaCl bitcode
    FakeKill,      // not part of LLVM/PNaCl bitcode
    JumpTable,     // not part of LLVM/PNaCl bitcode
    ShuffleVector, // not part of LLVM/PNaCl bitcode
    // Anything >= Target is an InstTarget subclass. Note that the value-spaces
    // are shared across targets. To avoid confusion over the definition of
    // shared values, an object specific to one target should never be passed
    // to a different target.
    Target,
    Target_Max = std::numeric_limits<uint8_t>::max(),
  };
  static_assert(Target <= Target_Max, "Must not be above max.");
  InstKind getKind() const { return Kind; }
  virtual const char *getInstName() const;

  InstNumberT getNumber() const { return Number; }
  void renumber(Cfg *Func);
  enum {
    NumberDeleted = -1,
    NumberSentinel = 0,
    NumberInitial = 2,
    NumberExtended = NumberInitial - 1
  };

  bool isDeleted() const { return Deleted; }
  void setDeleted() { Deleted = true; }
  void setDead(bool Value = true) { Dead = Value; }
  void deleteIfDead();

  bool hasSideEffects() const { return HasSideEffects; }

  bool isDestRedefined() const { return IsDestRedefined; }
  void setDestRedefined() { IsDestRedefined = true; }

  Variable *getDest() const { return Dest; }

  SizeT getSrcSize() const { return Srcs.size(); }
  Operand *getSrc(SizeT I) const {
    assert(I < getSrcSize());
    return Srcs[I];
  }
  void replaceSource(SizeT Index, Operand *Replacement) {
    assert(Index < getSrcSize());
    assert(!isDeleted());
    Srcs[Index] = Replacement;
  }
  // Instructions which load data take their address in Src[0], while
  // store instructions use Src[1] for the address and Src[0] for the data.
  Operand *getLoadAddress() const { return getSrc(0); }
  Operand *getStoreAddress() const { return getSrc(1); }
  Operand *getData() const { return getSrc(0); }

  bool isLastUse(const Operand *Src) const;
  void spliceLivenessInfo(Inst *OrigInst, Inst *SpliceAssn);

  /// Returns a list of out-edges corresponding to a terminator instruction,
  /// which is the last instruction of the block. The list must not contain
  /// duplicates.
  virtual NodeList getTerminatorEdges() const {
    // All valid terminator instructions override this method. For the default
    // implementation, we assert in case some CfgNode is constructed without a
    // terminator instruction at the end.
    llvm_unreachable(
        "getTerminatorEdges() called on a non-terminator instruction");
    return NodeList();
  }
  virtual bool isUnconditionalBranch() const { return false; }
  /// If the instruction is a branch-type instruction with OldNode as a target,
  /// repoint it to NewNode and return true, otherwise return false. Repoint all
  /// instances of OldNode as a target.
  virtual bool repointEdges(CfgNode *OldNode, CfgNode *NewNode) {
    (void)OldNode;
    (void)NewNode;
    return false;
  }

  /// Returns true if the instruction is equivalent to a simple
  /// "var_dest=var_src" assignment where the dest and src are both variables.
  virtual bool isVarAssign() const { return false; }

  /// Returns true if the instruction has a possible side effect of changing
  /// memory, in which case a memory load should not be reordered with respect
  /// to this instruction.  It should really be pure virtual, but we can't
  /// because of g++ and llvm::ilist<>, so we implement it as
  /// report_fatal_error().
  virtual bool isMemoryWrite() const;

  /// Returns true if the (target-specific) instruction represents an
  /// intra-block label, i.e. branch target.  This is meant primarily for
  /// Cfg::splitLocalVars().
  virtual bool isLabel() const { return false; }
  /// If the (target-specific) instruction represents an intra-block branch to
  /// some Label instruction, return that Label branch target instruction;
  /// otherwise return nullptr.
  virtual const Inst *getIntraBlockBranchTarget() const { return nullptr; }

  void livenessLightweight(Cfg *Func, LivenessBV &Live);
  /// Calculates liveness for this instruction. Returns true if this instruction
  /// is (tentatively) still live and should be retained, and false if this
  /// instruction is (tentatively) dead and should be deleted. The decision is
  /// tentative until the liveness dataflow algorithm has converged, and then a
  /// separate pass permanently deletes dead instructions.
  bool liveness(InstNumberT InstNumber, LivenessBV &Live, Liveness *Liveness,
                LiveBeginEndMap *LiveBegin, LiveBeginEndMap *LiveEnd);

  /// Get the number of native instructions that this instruction ultimately
  /// emits. By default, high-level instructions don't result in any native
  /// instructions, and a target-specific instruction results in a single native
  /// instruction.
  virtual uint32_t getEmitInstCount() const { return 0; }
  // TODO(stichnot): Change Inst back to abstract once the g++ build issue is
  // fixed. llvm::ilist<Ice::Inst> doesn't work under g++ because the
  // resize(size_t, Ice::Inst) method is incorrectly declared and thus doesn't
  // allow the abstract class Ice::Inst. The method should be declared
  // resize(size_t, const Ice::Inst &). virtual void emit(const Cfg *Func)
  // const = 0; virtual void emitIAS(const Cfg *Func) const = 0;
  virtual void emit(const Cfg *) const {
    llvm_unreachable("emit on abstract class");
  }
  virtual void emitIAS(const Cfg *Func) const { emit(Func); }
  virtual void dump(const Cfg *Func) const;
  virtual void dumpExtras(const Cfg *Func) const;
  void dumpDecorated(const Cfg *Func) const;
  void emitSources(const Cfg *Func) const;
  void dumpSources(const Cfg *Func) const;
  void dumpDest(const Cfg *Func) const;
  virtual bool isRedundantAssign() const { return false; }

  virtual ~Inst() = default;
  void replaceDest(Variable *Var) { Dest = Var; }

  void operator delete(void *Ptr, std::size_t Size) {
    assert(CfgAllocatorTraits::current() != nullptr);
    CfgAllocatorTraits::current()->Deallocate(Ptr, Size);
    llvm::report_fatal_error("Inst unexpectedly deleted");
  }

  inline void *getExternalData() const { return externalData; }
  inline void setExternalData(void *data) { externalData = data; }

protected:
  Inst(Cfg *Func, InstKind Kind, SizeT MaxSrcs, Variable *Dest);
  void addSource(Operand *Src) {
    assert(Src);
    Srcs.push_back(Src);
  }
  void setLastUse(SizeT VarIndex) {
    if (VarIndex < CHAR_BIT * sizeof(LiveRangesEnded))
      LiveRangesEnded |= (((LREndedBits)1u) << VarIndex);
  }
  void resetLastUses() { LiveRangesEnded = 0; }
  /// The destroy() method lets the instruction cleanly release any memory that
  /// was allocated via the Cfg's allocator.
  virtual void destroy(Cfg *) {}

  const InstKind Kind;
  /// Number is the instruction number for describing live ranges.
  InstNumberT Number;
  /// Deleted means irrevocably deleted.
  bool Deleted = false;
  /// Dead means one of two things depending on context: (1) pending deletion
  /// after liveness analysis converges, or (2) marked for deletion during
  /// lowering due to a folded bool operation.
  bool Dead = false;
  /// HasSideEffects means the instruction is something like a function call or
  /// a volatile load that can't be removed even if its Dest variable is not
  /// live.
  bool HasSideEffects = false;
  /// IsDestRedefined indicates that this instruction is not the first
  /// definition of Dest in the basic block.  The effect is that liveness
  /// analysis shouldn't consider this instruction to be the start of Dest's
  /// live range; rather, there is some other instruction earlier in the basic
  /// block with the same Dest.  This is maintained because liveness analysis
  /// has an invariant (primarily for performance reasons) that any Variable's
  /// live range recorded in a basic block has at most one start and at most one
  /// end.
  bool IsDestRedefined = false;
  /// External data can be set by an optimizer to compute and retain any
  /// information related to the current instruction. All the memory used to
  /// store this information must be managed by the optimizer.
  void *externalData = nullptr;

  Variable *Dest;
  const SizeT MaxSrcs; // only used for assert

  CfgVector<Operand *> Srcs;

  /// LiveRangesEnded marks which Variables' live ranges end in this
  /// instruction. An instruction can have an arbitrary number of source
  /// operands (e.g. a call instruction), and each source operand can contain 0
  /// or 1 Variable (and target-specific operands could contain more than 1
  /// Variable). All the variables in an instruction are conceptually flattened
  /// and each variable is mapped to one bit position of the LiveRangesEnded bit
  /// vector. Only the first CHAR_BIT * sizeof(LREndedBits) variables are
  /// tracked this way.
  using LREndedBits = uint32_t; // only first 32 src operands tracked, sorry
  LREndedBits LiveRangesEnded;
};

class InstHighLevel : public Inst {
  InstHighLevel() = delete;
  InstHighLevel(const InstHighLevel &) = delete;
  InstHighLevel &operator=(const InstHighLevel &) = delete;

protected:
  InstHighLevel(Cfg *Func, InstKind Kind, SizeT MaxSrcs, Variable *Dest)
      : Inst(Func, Kind, MaxSrcs, Dest) {}
  void emit(const Cfg * /*Func*/) const override {
    llvm_unreachable("emit() called on a non-lowered instruction");
  }
  void emitIAS(const Cfg * /*Func*/) const override {
    llvm_unreachable("emitIAS() called on a non-lowered instruction");
  }
};

/// Alloca instruction. This captures the size in bytes as getSrc(0), and the
/// required alignment in bytes. The alignment must be either 0 (no alignment
/// required) or a power of 2.
class InstAlloca : public InstHighLevel {
  InstAlloca() = delete;
  InstAlloca(const InstAlloca &) = delete;
  InstAlloca &operator=(const InstAlloca &) = delete;

public:
  static InstAlloca *create(Cfg *Func, Variable *Dest, Operand *ByteCount,
                            uint32_t AlignInBytes) {
    return new (Func->allocate<InstAlloca>())
        InstAlloca(Func, Dest, ByteCount, AlignInBytes);
  }
  uint32_t getAlignInBytes() const { return AlignInBytes; }
  Operand *getSizeInBytes() const { return getSrc(0); }
  bool getKnownFrameOffset() const { return KnownFrameOffset; }
  void setKnownFrameOffset() { KnownFrameOffset = true; }
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return Instr->getKind() == Alloca; }

private:
  InstAlloca(Cfg *Func, Variable *Dest, Operand *ByteCount,
             uint32_t AlignInBytes);

  const uint32_t AlignInBytes;
  bool KnownFrameOffset = false;
};

/// Binary arithmetic instruction. The source operands are captured in getSrc(0)
/// and getSrc(1).
class InstArithmetic : public InstHighLevel {
  InstArithmetic() = delete;
  InstArithmetic(const InstArithmetic &) = delete;
  InstArithmetic &operator=(const InstArithmetic &) = delete;

public:
  enum OpKind {
#define X(tag, str, commutative) tag,
    ICEINSTARITHMETIC_TABLE
#undef X
        _num
  };

  static InstArithmetic *create(Cfg *Func, OpKind Op, Variable *Dest,
                                Operand *Source1, Operand *Source2) {
    return new (Func->allocate<InstArithmetic>())
        InstArithmetic(Func, Op, Dest, Source1, Source2);
  }
  OpKind getOp() const { return Op; }

  virtual const char *getInstName() const override;

  static const char *getOpName(OpKind Op);
  bool isCommutative() const;
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return Instr->getKind() == Arithmetic;
  }

private:
  InstArithmetic(Cfg *Func, OpKind Op, Variable *Dest, Operand *Source1,
                 Operand *Source2);

  const OpKind Op;
};

/// Assignment instruction. The source operand is captured in getSrc(0). This is
/// not part of the LLVM bitcode, but is a useful abstraction for some of the
/// lowering. E.g., if Phi instruction lowering happens before target lowering,
/// or for representing an Inttoptr instruction, or as an intermediate step for
/// lowering a Load instruction.
class InstAssign : public InstHighLevel {
  InstAssign() = delete;
  InstAssign(const InstAssign &) = delete;
  InstAssign &operator=(const InstAssign &) = delete;

public:
  static InstAssign *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstAssign>()) InstAssign(Func, Dest, Source);
  }
  bool isVarAssign() const override;
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return Instr->getKind() == Assign; }

private:
  InstAssign(Cfg *Func, Variable *Dest, Operand *Source);
};

/// Branch instruction. This represents both conditional and unconditional
/// branches.
class InstBr : public InstHighLevel {
  InstBr() = delete;
  InstBr(const InstBr &) = delete;
  InstBr &operator=(const InstBr &) = delete;

public:
  /// Create a conditional branch. If TargetTrue==TargetFalse, it is optimized
  /// to an unconditional branch.
  static InstBr *create(Cfg *Func, Operand *Source, CfgNode *TargetTrue,
                        CfgNode *TargetFalse) {
    return new (Func->allocate<InstBr>())
        InstBr(Func, Source, TargetTrue, TargetFalse);
  }
  /// Create an unconditional branch.
  static InstBr *create(Cfg *Func, CfgNode *Target) {
    return new (Func->allocate<InstBr>()) InstBr(Func, Target);
  }
  bool isUnconditional() const { return getTargetTrue() == nullptr; }
  Operand *getCondition() const {
    assert(!isUnconditional());
    return getSrc(0);
  }
  CfgNode *getTargetTrue() const { return TargetTrue; }
  CfgNode *getTargetFalse() const { return TargetFalse; }
  CfgNode *getTargetUnconditional() const {
    assert(isUnconditional());
    return getTargetFalse();
  }
  NodeList getTerminatorEdges() const override;
  bool isUnconditionalBranch() const override { return isUnconditional(); }
  bool repointEdges(CfgNode *OldNode, CfgNode *NewNode) override;
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return Instr->getKind() == Br; }

private:
  /// Conditional branch
  InstBr(Cfg *Func, Operand *Source, CfgNode *TargetTrue, CfgNode *TargetFalse);
  /// Unconditional branch
  InstBr(Cfg *Func, CfgNode *Target);

  CfgNode *TargetFalse; /// Doubles as unconditional branch target
  CfgNode *TargetTrue;  /// nullptr if unconditional branch
};

/// Call instruction. The call target is captured as getSrc(0), and arg I is
/// captured as getSrc(I+1).
class InstCall : public InstHighLevel {
  InstCall() = delete;
  InstCall(const InstCall &) = delete;
  InstCall &operator=(const InstCall &) = delete;

public:
  static InstCall *create(Cfg *Func, SizeT NumArgs, Variable *Dest,
                          Operand *CallTarget, bool HasTailCall,
                          bool IsTargetHelperCall = false,
                          bool IsVariadic = false) {
    /// Set HasSideEffects to true so that the call instruction can't be
    /// dead-code eliminated.
    constexpr bool HasSideEffects = true;
    constexpr InstKind Kind = Inst::Call;
    return new (Func->allocate<InstCall>())
        InstCall(Func, NumArgs, Dest, CallTarget, HasTailCall,
                 IsTargetHelperCall, IsVariadic, HasSideEffects, Kind);
  }
  void addArg(Operand *Arg) { addSource(Arg); }
  Operand *getCallTarget() const { return getSrc(0); }
  Operand *getArg(SizeT I) const { return getSrc(I + 1); }
  SizeT getNumArgs() const { return getSrcSize() - 1; }
  bool isTailcall() const { return HasTailCall; }
  bool isTargetHelperCall() const { return IsTargetHelperCall; }
  bool isVariadic() const { return IsVariadic; }
  bool isMemoryWrite() const override { return true; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return Instr->getKind() == Call; }
  Type getReturnType() const;

protected:
  InstCall(Cfg *Func, SizeT NumArgs, Variable *Dest, Operand *CallTarget,
           bool HasTailCall, bool IsTargetHelperCall, bool IsVariadic,
           bool HasSideEff, InstKind Kind)
      : InstHighLevel(Func, Kind, NumArgs + 1, Dest), HasTailCall(HasTailCall),
        IsTargetHelperCall(IsTargetHelperCall), IsVariadic(IsVariadic) {
    HasSideEffects = HasSideEff;
    addSource(CallTarget);
  }

private:
  const bool HasTailCall;
  const bool IsTargetHelperCall;
  const bool IsVariadic;
};

/// Cast instruction (a.k.a. conversion operation).
class InstCast : public InstHighLevel {
  InstCast() = delete;
  InstCast(const InstCast &) = delete;
  InstCast &operator=(const InstCast &) = delete;

public:
  enum OpKind {
#define X(tag, str) tag,
    ICEINSTCAST_TABLE
#undef X
        _num
  };

  static const char *getCastName(OpKind Kind);

  static InstCast *create(Cfg *Func, OpKind CastKind, Variable *Dest,
                          Operand *Source) {
    return new (Func->allocate<InstCast>())
        InstCast(Func, CastKind, Dest, Source);
  }
  OpKind getCastKind() const { return CastKind; }
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return Instr->getKind() == Cast; }

private:
  InstCast(Cfg *Func, OpKind CastKind, Variable *Dest, Operand *Source);

  const OpKind CastKind;
};

/// ExtractElement instruction.
class InstExtractElement : public InstHighLevel {
  InstExtractElement() = delete;
  InstExtractElement(const InstExtractElement &) = delete;
  InstExtractElement &operator=(const InstExtractElement &) = delete;

public:
  static InstExtractElement *create(Cfg *Func, Variable *Dest, Operand *Source1,
                                    Operand *Source2) {
    return new (Func->allocate<InstExtractElement>())
        InstExtractElement(Func, Dest, Source1, Source2);
  }

  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return Instr->getKind() == ExtractElement;
  }

private:
  InstExtractElement(Cfg *Func, Variable *Dest, Operand *Source1,
                     Operand *Source2);
};

/// Floating-point comparison instruction. The source operands are captured in
/// getSrc(0) and getSrc(1).
class InstFcmp : public InstHighLevel {
  InstFcmp() = delete;
  InstFcmp(const InstFcmp &) = delete;
  InstFcmp &operator=(const InstFcmp &) = delete;

public:
  enum FCond {
#define X(tag, str) tag,
    ICEINSTFCMP_TABLE
#undef X
        _num
  };

  static InstFcmp *create(Cfg *Func, FCond Condition, Variable *Dest,
                          Operand *Source1, Operand *Source2) {
    return new (Func->allocate<InstFcmp>())
        InstFcmp(Func, Condition, Dest, Source1, Source2);
  }
  FCond getCondition() const { return Condition; }
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return Instr->getKind() == Fcmp; }

private:
  InstFcmp(Cfg *Func, FCond Condition, Variable *Dest, Operand *Source1,
           Operand *Source2);

  const FCond Condition;
};

/// Integer comparison instruction. The source operands are captured in
/// getSrc(0) and getSrc(1).
class InstIcmp : public InstHighLevel {
  InstIcmp() = delete;
  InstIcmp(const InstIcmp &) = delete;
  InstIcmp &operator=(const InstIcmp &) = delete;

public:
  enum ICond {
#define X(tag, inverse, str) tag,
    ICEINSTICMP_TABLE
#undef X
        _num
  };

  static InstIcmp *create(Cfg *Func, ICond Condition, Variable *Dest,
                          Operand *Source1, Operand *Source2) {
    return new (Func->allocate<InstIcmp>())
        InstIcmp(Func, Condition, Dest, Source1, Source2);
  }
  ICond getCondition() const { return Condition; }
  void reverseConditionAndOperands();
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return Instr->getKind() == Icmp; }

private:
  InstIcmp(Cfg *Func, ICond Condition, Variable *Dest, Operand *Source1,
           Operand *Source2);

  ICond Condition;
};

/// InsertElement instruction.
class InstInsertElement : public InstHighLevel {
  InstInsertElement() = delete;
  InstInsertElement(const InstInsertElement &) = delete;
  InstInsertElement &operator=(const InstInsertElement &) = delete;

public:
  static InstInsertElement *create(Cfg *Func, Variable *Dest, Operand *Source1,
                                   Operand *Source2, Operand *Source3) {
    return new (Func->allocate<InstInsertElement>())
        InstInsertElement(Func, Dest, Source1, Source2, Source3);
  }

  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return Instr->getKind() == InsertElement;
  }

private:
  InstInsertElement(Cfg *Func, Variable *Dest, Operand *Source1,
                    Operand *Source2, Operand *Source3);
};

/// An intrinsic operation, representing either a sequence of instructions,
/// or a single instruction. Availability of intrinsics is target-specific.
class InstIntrinsic : public InstHighLevel {
  InstIntrinsic() = delete;
  InstIntrinsic(const InstIntrinsic &) = delete;
  InstIntrinsic &operator=(const InstIntrinsic &) = delete;

public:
  static InstIntrinsic *create(Cfg *Func, SizeT NumArgs, Variable *Dest,
                               const Intrinsics::IntrinsicInfo &Info) {
    return new (Func->allocate<InstIntrinsic>())
        InstIntrinsic(Func, NumArgs, Dest, Info);
  }
  void addArg(Operand *Arg) { addSource(Arg); }
  Operand *getArg(SizeT I) const { return getSrc(I); }
  SizeT getNumArgs() const { return getSrcSize(); }
  static bool classof(const Inst *Instr) {
    return Instr->getKind() == Intrinsic;
  }

  Intrinsics::IntrinsicInfo getIntrinsicInfo() const { return Info; }
  Intrinsics::IntrinsicID getIntrinsicID() const { return Info.ID; }
  bool isMemoryWrite() const override {
    return getIntrinsicInfo().IsMemoryWrite;
  }

private:
  InstIntrinsic(Cfg *Func, SizeT NumArgs, Variable *Dest,
                const Intrinsics::IntrinsicInfo &Info)
      : InstHighLevel(Func, Inst::Intrinsic, NumArgs, Dest), Info(Info) {}

  const Intrinsics::IntrinsicInfo Info;
};

/// Load instruction. The source address is captured in getSrc(0).
class InstLoad : public InstHighLevel {
  InstLoad() = delete;
  InstLoad(const InstLoad &) = delete;
  InstLoad &operator=(const InstLoad &) = delete;

public:
  static InstLoad *create(Cfg *Func, Variable *Dest, Operand *SourceAddr,
                          uint32_t Align = 1) {
    // TODO(kschimpf) Stop ignoring alignment specification.
    (void)Align;
    return new (Func->allocate<InstLoad>()) InstLoad(Func, Dest, SourceAddr);
  }
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return Instr->getKind() == Load; }

private:
  InstLoad(Cfg *Func, Variable *Dest, Operand *SourceAddr);
};

/// Phi instruction. For incoming edge I, the node is Labels[I] and the Phi
/// source operand is getSrc(I).
class InstPhi : public InstHighLevel {
  InstPhi() = delete;
  InstPhi(const InstPhi &) = delete;
  InstPhi &operator=(const InstPhi &) = delete;

public:
  static InstPhi *create(Cfg *Func, SizeT MaxSrcs, Variable *Dest) {
    return new (Func->allocate<InstPhi>()) InstPhi(Func, MaxSrcs, Dest);
  }
  void addArgument(Operand *Source, CfgNode *Label);
  Operand *getOperandForTarget(CfgNode *Target) const;
  void clearOperandForTarget(CfgNode *Target);
  CfgNode *getLabel(SizeT Index) const { return Labels[Index]; }
  void setLabel(SizeT Index, CfgNode *Label) { Labels[Index] = Label; }
  void livenessPhiOperand(LivenessBV &Live, CfgNode *Target,
                          Liveness *Liveness);
  Inst *lower(Cfg *Func);
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return Instr->getKind() == Phi; }

private:
  InstPhi(Cfg *Func, SizeT MaxSrcs, Variable *Dest);
  void destroy(Cfg *Func) override { Inst::destroy(Func); }

  /// Labels[] duplicates the InEdges[] information in the enclosing CfgNode,
  /// but the Phi instruction is created before InEdges[] is available, so it's
  /// more complicated to share the list.
  CfgVector<CfgNode *> Labels;
};

/// Ret instruction. The return value is captured in getSrc(0), but if there is
/// no return value (void-type function), then getSrcSize()==0 and
/// hasRetValue()==false.
class InstRet : public InstHighLevel {
  InstRet() = delete;
  InstRet(const InstRet &) = delete;
  InstRet &operator=(const InstRet &) = delete;

public:
  static InstRet *create(Cfg *Func, Operand *RetValue = nullptr) {
    return new (Func->allocate<InstRet>()) InstRet(Func, RetValue);
  }
  bool hasRetValue() const { return getSrcSize(); }
  Operand *getRetValue() const {
    assert(hasRetValue());
    return getSrc(0);
  }
  NodeList getTerminatorEdges() const override { return NodeList(); }
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return Instr->getKind() == Ret; }

private:
  InstRet(Cfg *Func, Operand *RetValue);
};

/// Select instruction.  The condition, true, and false operands are captured.
class InstSelect : public InstHighLevel {
  InstSelect() = delete;
  InstSelect(const InstSelect &) = delete;
  InstSelect &operator=(const InstSelect &) = delete;

public:
  static InstSelect *create(Cfg *Func, Variable *Dest, Operand *Condition,
                            Operand *SourceTrue, Operand *SourceFalse) {
    return new (Func->allocate<InstSelect>())
        InstSelect(Func, Dest, Condition, SourceTrue, SourceFalse);
  }
  Operand *getCondition() const { return getSrc(0); }
  Operand *getTrueOperand() const { return getSrc(1); }
  Operand *getFalseOperand() const { return getSrc(2); }
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return Instr->getKind() == Select; }

private:
  InstSelect(Cfg *Func, Variable *Dest, Operand *Condition, Operand *Source1,
             Operand *Source2);
};

/// Store instruction. The address operand is captured, along with the data
/// operand to be stored into the address.
class InstStore : public InstHighLevel {
  InstStore() = delete;
  InstStore(const InstStore &) = delete;
  InstStore &operator=(const InstStore &) = delete;

public:
  static InstStore *create(Cfg *Func, Operand *Data, Operand *Addr,
                           uint32_t Align = 1) {
    // TODO(kschimpf) Stop ignoring alignment specification.
    (void)Align;
    return new (Func->allocate<InstStore>()) InstStore(Func, Data, Addr);
  }
  Variable *getRmwBeacon() const;
  void setRmwBeacon(Variable *Beacon);
  bool isMemoryWrite() const override { return true; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return Instr->getKind() == Store; }

private:
  InstStore(Cfg *Func, Operand *Data, Operand *Addr);
};

/// Switch instruction. The single source operand is captured as getSrc(0).
class InstSwitch : public InstHighLevel {
  InstSwitch() = delete;
  InstSwitch(const InstSwitch &) = delete;
  InstSwitch &operator=(const InstSwitch &) = delete;

public:
  static InstSwitch *create(Cfg *Func, SizeT NumCases, Operand *Source,
                            CfgNode *LabelDefault) {
    return new (Func->allocate<InstSwitch>())
        InstSwitch(Func, NumCases, Source, LabelDefault);
  }
  Operand *getComparison() const { return getSrc(0); }
  CfgNode *getLabelDefault() const { return LabelDefault; }
  SizeT getNumCases() const { return NumCases; }
  uint64_t getValue(SizeT I) const {
    assert(I < NumCases);
    return Values[I];
  }
  CfgNode *getLabel(SizeT I) const {
    assert(I < NumCases);
    return Labels[I];
  }
  void addBranch(SizeT CaseIndex, uint64_t Value, CfgNode *Label);
  NodeList getTerminatorEdges() const override;
  bool repointEdges(CfgNode *OldNode, CfgNode *NewNode) override;
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return Instr->getKind() == Switch; }

private:
  InstSwitch(Cfg *Func, SizeT NumCases, Operand *Source, CfgNode *LabelDefault);
  void destroy(Cfg *Func) override {
    Func->deallocateArrayOf<uint64_t>(Values);
    Func->deallocateArrayOf<CfgNode *>(Labels);
    Inst::destroy(Func);
  }

  CfgNode *LabelDefault;
  SizeT NumCases;   /// not including the default case
  uint64_t *Values; /// size is NumCases
  CfgNode **Labels; /// size is NumCases
};

/// Unreachable instruction. This is a terminator instruction with no operands.
class InstUnreachable : public InstHighLevel {
  InstUnreachable() = delete;
  InstUnreachable(const InstUnreachable &) = delete;
  InstUnreachable &operator=(const InstUnreachable &) = delete;

public:
  static InstUnreachable *create(Cfg *Func) {
    return new (Func->allocate<InstUnreachable>()) InstUnreachable(Func);
  }
  NodeList getTerminatorEdges() const override { return NodeList(); }
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return Instr->getKind() == Unreachable;
  }

private:
  explicit InstUnreachable(Cfg *Func);
};

/// FakeDef instruction. This creates a fake definition of a variable, which is
/// how we represent the case when an instruction produces multiple results.
/// This doesn't happen with high-level ICE instructions, but might with lowered
/// instructions. For example, this would be a way to represent condition flags
/// being modified by an instruction.
///
/// It's generally useful to set the optional source operand to be the dest
/// variable of the instruction that actually produces the FakeDef dest.
/// Otherwise, the original instruction could be dead-code eliminated if its
/// dest operand is unused, and therefore the FakeDef dest wouldn't be properly
/// initialized.
class InstFakeDef : public InstHighLevel {
  InstFakeDef() = delete;
  InstFakeDef(const InstFakeDef &) = delete;
  InstFakeDef &operator=(const InstFakeDef &) = delete;

public:
  static InstFakeDef *create(Cfg *Func, Variable *Dest,
                             Variable *Src = nullptr) {
    return new (Func->allocate<InstFakeDef>()) InstFakeDef(Func, Dest, Src);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg * /* Func */) const override {}
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return Instr->getKind() == FakeDef; }

private:
  InstFakeDef(Cfg *Func, Variable *Dest, Variable *Src);
};

/// FakeUse instruction. This creates a fake use of a variable, to keep the
/// instruction that produces that variable from being dead-code eliminated.
/// This is useful in a variety of lowering situations. The FakeUse instruction
/// has no dest, so it can itself never be dead-code eliminated.  A weight can
/// be provided to provide extra bias to the register allocator - for simplicity
/// of implementation, weight=N is handled by holding N copies of the variable
/// as source operands.
class InstFakeUse : public InstHighLevel {
  InstFakeUse() = delete;
  InstFakeUse(const InstFakeUse &) = delete;
  InstFakeUse &operator=(const InstFakeUse &) = delete;

public:
  static InstFakeUse *create(Cfg *Func, Variable *Src, uint32_t Weight = 1) {
    return new (Func->allocate<InstFakeUse>()) InstFakeUse(Func, Src, Weight);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg * /* Func */) const override {}
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return Instr->getKind() == FakeUse; }

private:
  InstFakeUse(Cfg *Func, Variable *Src, uint32_t Weight);
};

/// FakeKill instruction. This "kills" a set of variables by modeling a trivial
/// live range at this instruction for each (implicit) variable. The primary use
/// is to indicate that scratch registers are killed after a call, so that the
/// register allocator won't assign a scratch register to a variable whose live
/// range spans a call.
///
/// The FakeKill instruction also holds a pointer to the instruction that kills
/// the set of variables, so that if that linked instruction gets dead-code
/// eliminated, the FakeKill instruction will as well.
class InstFakeKill : public InstHighLevel {
  InstFakeKill() = delete;
  InstFakeKill(const InstFakeKill &) = delete;
  InstFakeKill &operator=(const InstFakeKill &) = delete;

public:
  static InstFakeKill *create(Cfg *Func, const Inst *Linked) {
    return new (Func->allocate<InstFakeKill>()) InstFakeKill(Func, Linked);
  }
  const Inst *getLinked() const { return Linked; }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg * /* Func */) const override {}
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return Instr->getKind() == FakeKill;
  }

private:
  InstFakeKill(Cfg *Func, const Inst *Linked);

  /// This instruction is ignored if Linked->isDeleted() is true.
  const Inst *Linked;
};

/// ShuffleVector instruction. This represents a shuffle operation on vector
/// types. This instruction is not part of the PNaCl bitcode: it is generated
/// by Subzero when it matches the pattern used by pnacl-clang when compiling
/// to bitcode.
class InstShuffleVector : public InstHighLevel {
  InstShuffleVector() = delete;
  InstShuffleVector(const InstShuffleVector &) = delete;
  InstShuffleVector &operator=(const InstShuffleVector &) = delete;

public:
  static InstShuffleVector *create(Cfg *Func, Variable *Dest, Operand *Src0,
                                   Operand *Src1) {
    return new (Func->allocate<InstShuffleVector>())
        InstShuffleVector(Func, Dest, Src0, Src1);
  }

  SizeT getNumIndexes() const { return NumIndexes; }

  void addIndex(ConstantInteger32 *Index) {
    assert(CurrentIndex < NumIndexes);
    Indexes[CurrentIndex++] = Index;
  }

  ConstantInteger32 *getIndex(SizeT Pos) const {
    assert(Pos < NumIndexes);
    return Indexes[Pos];
  }

  int32_t getIndexValue(SizeT Pos) const { return getIndex(Pos)->getValue(); }

  bool indexesAre(int32_t i0, int32_t i1, int32_t i2, int32_t i3) const {
    static constexpr SizeT ExpectedNumElements = 4;
    assert(ExpectedNumElements == getNumIndexes());
    (void)ExpectedNumElements;

    return getIndexValue(0) == i0 && getIndexValue(1) == i1 &&
           getIndexValue(2) == i2 && getIndexValue(3) == i3;
  }

  bool indexesAre(int32_t i0, int32_t i1, int32_t i2, int32_t i3, int32_t i4,
                  int32_t i5, int32_t i6, int32_t i7) const {
    static constexpr SizeT ExpectedNumElements = 8;
    assert(ExpectedNumElements == getNumIndexes());
    (void)ExpectedNumElements;

    return getIndexValue(0) == i0 && getIndexValue(1) == i1 &&
           getIndexValue(2) == i2 && getIndexValue(3) == i3 &&
           getIndexValue(4) == i4 && getIndexValue(5) == i5 &&
           getIndexValue(6) == i6 && getIndexValue(7) == i7;
  }

  bool indexesAre(int32_t i0, int32_t i1, int32_t i2, int32_t i3, int32_t i4,
                  int32_t i5, int32_t i6, int32_t i7, int32_t i8, int32_t i9,
                  int32_t i10, int32_t i11, int32_t i12, int32_t i13,
                  int32_t i14, int32_t i15) const {
    static constexpr SizeT ExpectedNumElements = 16;
    assert(ExpectedNumElements == getNumIndexes());
    (void)ExpectedNumElements;

    return getIndexValue(0) == i0 && getIndexValue(1) == i1 &&
           getIndexValue(2) == i2 && getIndexValue(3) == i3 &&
           getIndexValue(4) == i4 && getIndexValue(5) == i5 &&
           getIndexValue(6) == i6 && getIndexValue(7) == i7 &&
           getIndexValue(8) == i8 && getIndexValue(9) == i9 &&
           getIndexValue(10) == i10 && getIndexValue(11) == i11 &&
           getIndexValue(12) == i12 && getIndexValue(13) == i13 &&
           getIndexValue(14) == i14 && getIndexValue(15) == i15;
  }

  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return Instr->getKind() == ShuffleVector;
  }

private:
  InstShuffleVector(Cfg *Func, Variable *Dest, Operand *Src0, Operand *Src1);

  void destroy(Cfg *Func) override {
    Func->deallocateArrayOf<ConstantInteger32 *>(Indexes);
    Inst::destroy(Func);
  }

  ConstantInteger32 **Indexes;
  SizeT CurrentIndex = 0;
  const SizeT NumIndexes;
};

/// JumpTable instruction. This represents a jump table that will be stored in
/// the .rodata section. This is used to track and repoint the target CfgNodes
/// which may change, for example due to splitting for phi lowering.
class InstJumpTable : public InstHighLevel {
  InstJumpTable() = delete;
  InstJumpTable(const InstJumpTable &) = delete;
  InstJumpTable &operator=(const InstJumpTable &) = delete;

public:
  static InstJumpTable *create(Cfg *Func, SizeT NumTargets, CfgNode *Default) {
    return new (Func->allocate<InstJumpTable>())
        InstJumpTable(Func, NumTargets, Default);
  }
  void addTarget(SizeT TargetIndex, CfgNode *Target) {
    assert(TargetIndex < NumTargets);
    Targets[TargetIndex] = Target;
  }
  bool repointEdges(CfgNode *OldNode, CfgNode *NewNode) override;
  SizeT getId() const { return Id; }
  SizeT getNumTargets() const { return NumTargets; }
  CfgNode *getTarget(SizeT I) const {
    assert(I < NumTargets);
    return Targets[I];
  }
  bool isMemoryWrite() const override { return false; }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return Instr->getKind() == JumpTable;
  }
  // Creates a JumpTableData struct (used for ELF emission) that represents this
  // InstJumpTable.
  JumpTableData toJumpTableData(Assembler *Asm) const;

  // InstJumpTable is just a placeholder for the switch targets, and it does not
  // need to emit any code, so we redefine emit and emitIAS to do nothing.
  void emit(const Cfg *) const override {}
  void emitIAS(const Cfg * /* Func */) const override {}

  const std::string getName() const {
    assert(Name.hasStdString());
    return Name.toString();
  }

  std::string getSectionName() const {
    return JumpTableData::createSectionName(FuncName);
  }

private:
  InstJumpTable(Cfg *Func, SizeT NumTargets, CfgNode *Default);
  void destroy(Cfg *Func) override {
    Func->deallocateArrayOf<CfgNode *>(Targets);
    Inst::destroy(Func);
  }

  const SizeT Id;
  const SizeT NumTargets;
  CfgNode **Targets;
  GlobalString Name; // This JumpTable's name in the output.
  GlobalString FuncName;
};

/// This instruction inserts an unconditional breakpoint.
///
/// On x86, this assembles into an INT 3 instruction.
///
/// This instruction is primarily meant for debugging the code generator.
class InstBreakpoint : public InstHighLevel {
public:
  InstBreakpoint() = delete;
  InstBreakpoint(const InstBreakpoint &) = delete;
  InstBreakpoint &operator=(const InstBreakpoint &) = delete;

  explicit InstBreakpoint(Cfg *Func);
  bool isMemoryWrite() const override { return false; }

public:
  static InstBreakpoint *create(Cfg *Func) {
    return new (Func->allocate<InstBreakpoint>()) InstBreakpoint(Func);
  }

  static bool classof(const Inst *Instr) {
    return Instr->getKind() == Breakpoint;
  }
};

/// The Target instruction is the base class for all target-specific
/// instructions.
class InstTarget : public Inst {
  InstTarget() = delete;
  InstTarget(const InstTarget &) = delete;
  InstTarget &operator=(const InstTarget &) = delete;

public:
  uint32_t getEmitInstCount() const override { return 1; }
  bool isMemoryWrite() const override {
    return true; // conservative answer
  }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return Instr->getKind() >= Target; }

protected:
  InstTarget(Cfg *Func, InstKind Kind, SizeT MaxSrcs, Variable *Dest)
      : Inst(Func, Kind, MaxSrcs, Dest) {
    assert(Kind >= Target);
    assert(Kind <= Target_Max);
  }
};

bool checkForRedundantAssign(const Variable *Dest, const Operand *Source);

} // end of namespace Ice

#ifdef PNACL_LLVM
namespace llvm {

/// Override the default ilist traits so that Inst's private ctor and deleted
/// dtor aren't invoked.
template <>
struct ilist_traits<Ice::Inst> : public ilist_default_traits<Ice::Inst> {
  Ice::Inst *createSentinel() const {
    return static_cast<Ice::Inst *>(&Sentinel);
  }
  static void destroySentinel(Ice::Inst *) {}
  Ice::Inst *provideInitialHead() const { return createSentinel(); }
  Ice::Inst *ensureHead(Ice::Inst *) const { return createSentinel(); }
  static void noteHead(Ice::Inst *, Ice::Inst *) {}
  void deleteNode(Ice::Inst *) {}

private:
  mutable ilist_half_node<Ice::Inst> Sentinel;
};

} // end of namespace llvm
#endif // PNACL_LLVM

namespace Ice {

inline InstList::iterator instToIterator(Inst *Instr) {
#ifdef PNACL_LLVM
  return Instr;
#else  // !PNACL_LLVM
  return Instr->getIterator();
#endif // !PNACL_LLVM
}

inline Inst *iteratorToInst(InstList::iterator Iter) { return &*Iter; }

inline const Inst *iteratorToInst(InstList::const_iterator Iter) {
  return &*Iter;
}

inline InstList::iterator
reverseToForwardIterator(InstList::reverse_iterator RI) {
#ifdef PNACL_LLVM
  return RI.base();
#else  // !PNACL_LLVM
  return ++RI.getReverse();
#endif // !PNACL_LLVM
}

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEINST_H
