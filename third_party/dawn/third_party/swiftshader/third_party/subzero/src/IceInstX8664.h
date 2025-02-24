//===- subzero/src/IceInstX86.h - Generic x86 instructions -*- C++ -*--===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file defines the InstX86Base template class, as well as the
/// generic X86 Instruction class hierarchy.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEINSTX8664_H
#define SUBZERO_SRC_ICEINSTX8664_H

#include "IceAssemblerX8664.h"
#include "IceDefs.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceTargetLoweringX86.h"

namespace Ice {
namespace X8664 {

using Assembler = AssemblerX8664;
using AssemblerImmediate = Assembler::Immediate;
using TargetLowering = TargetX8664;

using RegisterSet = ::Ice::RegX8664;
using GPRRegister = RegisterSet::GPRRegister;
using ByteRegister = RegisterSet::ByteRegister;
using XmmRegister = RegisterSet::XmmRegister;

using Cond = CondX86;
using BrCond = Cond::BrCond;
using CmppsCond = Cond::CmppsCond;

template <typename SReg_t, typename DReg_t>
using CastEmitterRegOp = Assembler::template CastEmitterRegOp<SReg_t, DReg_t>;
template <typename SReg_t, typename DReg_t>
using ThreeOpImmEmitter = Assembler::template ThreeOpImmEmitter<SReg_t, DReg_t>;
using GPREmitterAddrOp = Assembler::GPREmitterAddrOp;
using GPREmitterRegOp = Assembler::GPREmitterRegOp;
using GPREmitterShiftD = Assembler::GPREmitterShiftD;
using GPREmitterShiftOp = Assembler::GPREmitterShiftOp;
using GPREmitterOneOp = Assembler::GPREmitterOneOp;
using XmmEmitterRegOp = Assembler::XmmEmitterRegOp;
using XmmEmitterShiftOp = Assembler::XmmEmitterShiftOp;
using XmmEmitterMovOps = Assembler::XmmEmitterMovOps;

/// X86Operand extends the Operand hierarchy. Its subclass is X86OperandMem.
class X86Operand : public ::Ice::Operand {
  X86Operand() = delete;
  X86Operand(const X86Operand &) = delete;
  X86Operand &operator=(const X86Operand &) = delete;

public:
  enum OperandKindX8664 { k__Start = ::Ice::Operand::kTarget, kMem, kSplit };
  using ::Ice::Operand::dump;

  void dump(const Cfg *, Ostream &Str) const override;

protected:
  X86Operand(OperandKindX8664 Kind, Type Ty)
      : Operand(static_cast<::Ice::Operand::OperandKind>(Kind), Ty) {}
};

/// X86OperandMem represents the m64 addressing mode, with optional base and
/// index registers, a constant offset, and a fixed shift value for the index
/// register.
class X86OperandMem : public X86Operand {
  X86OperandMem() = delete;
  X86OperandMem(const X86OperandMem &) = delete;
  X86OperandMem &operator=(const X86OperandMem &) = delete;

public:
  enum SegmentRegisters { DefaultSegment = -1, SegReg_NUM };
  static X86OperandMem *
  create(Cfg *Func, Type Ty, Variable *Base, Constant *Offset,
         Variable *Index = nullptr, uint16_t Shift = 0,
         SegmentRegisters SegmentRegister = DefaultSegment,
         bool IsRebased = false) {
    assert(SegmentRegister == DefaultSegment);
    (void)SegmentRegister;
    return new (Func->allocate<X86OperandMem>())
        X86OperandMem(Func, Ty, Base, Offset, Index, Shift, IsRebased);
  }
  static X86OperandMem *create(Cfg *Func, Type Ty, Variable *Base,
                               Constant *Offset, bool IsRebased) {
    constexpr Variable *NoIndex = nullptr;
    constexpr uint16_t NoShift = 0;
    return new (Func->allocate<X86OperandMem>())
        X86OperandMem(Func, Ty, Base, Offset, NoIndex, NoShift, IsRebased);
  }
  Variable *getBase() const { return Base; }
  Constant *getOffset() const { return Offset; }
  Variable *getIndex() const { return Index; }
  uint16_t getShift() const { return Shift; }
  SegmentRegisters getSegmentRegister() const { return DefaultSegment; }
  void emitSegmentOverride(Assembler *) const {}
  bool getIsRebased() const { return IsRebased; }

  void emit(const Cfg *Func) const override;
  using X86Operand::dump;
  void dump(const Cfg *Func, Ostream &Str) const override;

  static bool classof(const Operand *Operand) {
    return Operand->getKind() == static_cast<OperandKind>(kMem);
  }

private:
  X86OperandMem(Cfg *Func, Type Ty, Variable *Base, Constant *Offset,
                Variable *Index, uint16_t Shift, bool IsRebased);

  Variable *const Base;
  Constant *const Offset;
  Variable *const Index;
  const uint16_t Shift;
  const bool IsRebased;
};

class InstX86Base : public InstTarget {
  InstX86Base() = delete;
  InstX86Base(const InstX86Base &) = delete;
  InstX86Base &operator=(const InstX86Base &) = delete;

public:
  enum InstKindX86 {
    k__Start = Inst::Target,
    Adc,
    AdcRMW,
    Add,
    AddRMW,
    Addps,
    Addss,
    And,
    Andnps,
    Andps,
    AndRMW,
    Blendvps,
    Br,
    Bsf,
    Bsr,
    Bswap,
    Call,
    Cbwdq,
    Cmov,
    Cmpps,
    Cmpxchg,
    Cmpxchg8b,
    Cvt,
    Div,
    Divps,
    Divss,
    FakeRMW,
    Icmp,
    Idiv,
    Imul,
    ImulImm,
    Insertps,
    Int3,
    Jmp,
    Label,
    Lea,
    Load,
    Mfence,
    Minps,
    Maxps,
    Minss,
    Maxss,
    Mov,
    Movd,
    Movmsk,
    Movp,
    Movq,
    MovssRegs,
    Movsx,
    Movzx,
    Mul,
    Mulps,
    Mulss,
    Neg,
    Nop,
    Or,
    Orps,
    OrRMW,
    Padd,
    Padds,
    Paddus,
    Pand,
    Pandn,
    Pblendvb,
    Pcmpeq,
    Pcmpgt,
    Pextr,
    Pinsr,
    Pmull,
    Pmulhw,
    Pmulhuw,
    Pmaddwd,
    Pmuludq,
    Pop,
    Por,
    Pshufb,
    Pshufd,
    Punpckl,
    Punpckh,
    Packss,
    Packus,
    Psll,
    Psra,
    Psrl,
    Psub,
    Psubs,
    Psubus,
    Push,
    Pxor,
    Ret,
    Rol,
    Round,
    Sar,
    Sbb,
    SbbRMW,
    Setcc,
    Shl,
    Shld,
    Shr,
    Shrd,
    Shufps,
    Sqrt,
    Store,
    StoreP,
    StoreQ,
    StoreD,
    Sub,
    SubRMW,
    Subps,
    Subss,
    Test,
    Ucomiss,
    UD2,
    Xadd,
    Xchg,
    Xor,
    Xorps,
    XorRMW,

    /// Intel Architecture Code Analyzer markers. These are not executable so
    /// must only be used for analysis.
    IacaStart,
    IacaEnd
  };

  enum SseSuffix { None, Packed, Unpack, Scalar, Integral, Pack };

  static const char *getWidthString(Type Ty);
  static const char *getFldString(Type Ty);
  static const char *getSseSuffixString(Type DestTy, SseSuffix Suffix);
  static Type getInVectorElementType(Type Ty);
  static BrCond getOppositeCondition(BrCond Cond);
  void dump(const Cfg *Func) const override;

  // Shared emit routines for common forms of instructions.
  void emitTwoAddress(const Cfg *Func, const char *Opcode,
                      const char *Suffix = "") const;

  static TargetLowering *getTarget(const Cfg *Func) {
    return reinterpret_cast<TargetLowering *>(Func->getTarget());
  }

protected:
  InstX86Base(Cfg *Func, InstKindX86 Kind, SizeT Maxsrcs, Variable *Dest)
      : InstTarget(Func, static_cast<InstKind>(Kind), Maxsrcs, Dest) {}

  static bool isClassof(const Inst *Instr, InstKindX86 MyKind) {
    return Instr->getKind() == static_cast<InstKind>(MyKind);
  }
  // Most instructions that operate on vector arguments require vector memory
  // operands to be fully aligned (16-byte alignment for PNaCl vector types).
  // The stack frame layout and call ABI ensure proper alignment for stack
  // operands, but memory operands (originating from load/store bitcode
  // instructions) only have element-size alignment guarantees. This function
  // validates that none of the operands is a memory operand of vector type,
  // calling report_fatal_error() if one is found. This function should be
  // called during emission, and maybe also in the ctor (as long as that fits
  // the lowering style).
  void validateVectorAddrMode() const {
    if (this->getDest())
      this->validateVectorAddrModeOpnd(this->getDest());
    for (SizeT i = 0; i < this->getSrcSize(); ++i) {
      this->validateVectorAddrModeOpnd(this->getSrc(i));
    }
  }

private:
  static void validateVectorAddrModeOpnd(const Operand *Opnd) {
    if (llvm::isa<X86OperandMem>(Opnd) && isVectorType(Opnd->getType())) {
      llvm::report_fatal_error("Possible misaligned vector memory operation");
    }
  }
};

/// InstX86FakeRMW represents a non-atomic read-modify-write operation on a
/// memory location. An InstX86FakeRMW is a "fake" instruction in that it
/// still needs to be lowered to some actual RMW instruction.
///
/// If A is some memory address, D is some data value to apply, and OP is an
/// arithmetic operator, the instruction operates as: (*A) = (*A) OP D
class InstX86FakeRMW final : public InstX86Base {
  InstX86FakeRMW() = delete;
  InstX86FakeRMW(const InstX86FakeRMW &) = delete;
  InstX86FakeRMW &operator=(const InstX86FakeRMW &) = delete;

public:
  static InstX86FakeRMW *create(Cfg *Func, Operand *Data, Operand *Addr,
                                Variable *Beacon, InstArithmetic::OpKind Op,
                                uint32_t Align = 1) {
    // TODO(stichnot): Stop ignoring alignment specification.
    (void)Align;
    return new (Func->allocate<InstX86FakeRMW>())
        InstX86FakeRMW(Func, Data, Addr, Op, Beacon);
  }
  Operand *getAddr() const { return this->getSrc(1); }
  Operand *getData() const { return this->getSrc(0); }
  InstArithmetic::OpKind getOp() const { return Op; }
  Variable *getBeacon() const { return llvm::cast<Variable>(this->getSrc(2)); }
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::FakeRMW);
  }

private:
  InstArithmetic::OpKind Op;
  InstX86FakeRMW(Cfg *Func, Operand *Data, Operand *Addr,
                 InstArithmetic::OpKind Op, Variable *Beacon);
};

/// InstX86Label represents an intra-block label that is the target of an
/// intra-block branch. The offset between the label and the branch must be
/// fit into one byte (considered "near"). These are used for lowering i1
/// calculations, Select instructions, and 64-bit compares on a 32-bit
/// architecture, without basic block splitting. Basic block splitting is not
/// so desirable for several reasons, one of which is the impact on decisions
/// based on whether a variable's live range spans multiple basic blocks.
///
/// Intra-block control flow must be used with caution. Consider the sequence
/// for "c = (a >= b ? x : y)".
///     cmp a, b
///     br lt, L1
///     mov c, x
///     jmp L2
///   L1:
///     mov c, y
///   L2:
///
/// Labels L1 and L2 are intra-block labels. Without knowledge of the
/// intra-block control flow, liveness analysis will determine the "mov c, x"
/// instruction to be dead. One way to prevent this is to insert a
/// "FakeUse(c)" instruction anywhere between the two "mov c, ..."
/// instructions, e.g.:
///
///     cmp a, b
///     br lt, L1
///     mov c, x
///     jmp L2
///     FakeUse(c)
///   L1:
///     mov c, y
///   L2:
///
/// The down-side is that "mov c, x" can never be dead-code eliminated even if
/// there are no uses of c. As unlikely as this situation is, it may be
/// prevented by running dead code elimination before lowering.
class InstX86Label final : public InstX86Base {
  InstX86Label() = delete;
  InstX86Label(const InstX86Label &) = delete;
  InstX86Label &operator=(const InstX86Label &) = delete;

public:
  static InstX86Label *create(Cfg *Func, TargetLowering *Target) {
    return new (Func->allocate<InstX86Label>()) InstX86Label(Func, Target);
  }
  uint32_t getEmitInstCount() const override { return 0; }
  GlobalString getLabelName() const { return Name; }
  SizeT getLabelNumber() const { return LabelNumber; }
  bool isLabel() const override { return true; }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  void setRelocOffset(RelocOffset *Value) { OffsetReloc = Value; }

private:
  InstX86Label(Cfg *Func, TargetLowering *Target);

  SizeT LabelNumber; // used for unique label generation.
  RelocOffset *OffsetReloc = nullptr;
  GlobalString Name;
};

/// Conditional and unconditional branch instruction.
class InstX86Br final : public InstX86Base {
  InstX86Br() = delete;
  InstX86Br(const InstX86Br &) = delete;
  InstX86Br &operator=(const InstX86Br &) = delete;

public:
  enum Mode { Near, Far };

  /// Create a conditional branch to a node.
  static InstX86Br *create(Cfg *Func, CfgNode *TargetTrue, CfgNode *TargetFalse,
                           BrCond Condition, Mode Kind) {
    assert(Condition != Cond::Br_None);
    constexpr InstX86Label *NoLabel = nullptr;
    return new (Func->allocate<InstX86Br>())
        InstX86Br(Func, TargetTrue, TargetFalse, NoLabel, Condition, Kind);
  }
  /// Create an unconditional branch to a node.
  static InstX86Br *create(Cfg *Func, CfgNode *Target, Mode Kind) {
    constexpr CfgNode *NoCondTarget = nullptr;
    constexpr InstX86Label *NoLabel = nullptr;
    return new (Func->allocate<InstX86Br>())
        InstX86Br(Func, NoCondTarget, Target, NoLabel, Cond::Br_None, Kind);
  }
  /// Create a non-terminator conditional branch to a node, with a fallthrough
  /// to the next instruction in the current node. This is used for switch
  /// lowering.
  static InstX86Br *create(Cfg *Func, CfgNode *Target, BrCond Condition,
                           Mode Kind) {
    assert(Condition != Cond::Br_None);
    constexpr CfgNode *NoUncondTarget = nullptr;
    constexpr InstX86Label *NoLabel = nullptr;
    return new (Func->allocate<InstX86Br>())
        InstX86Br(Func, Target, NoUncondTarget, NoLabel, Condition, Kind);
  }
  /// Create a conditional intra-block branch (or unconditional, if
  /// Condition==Br_None) to a label in the current block.
  static InstX86Br *create(Cfg *Func, InstX86Label *Label, BrCond Condition,
                           Mode Kind) {
    constexpr CfgNode *NoCondTarget = nullptr;
    constexpr CfgNode *NoUncondTarget = nullptr;
    return new (Func->allocate<InstX86Br>())
        InstX86Br(Func, NoCondTarget, NoUncondTarget, Label, Condition, Kind);
  }
  const CfgNode *getTargetTrue() const { return TargetTrue; }
  const CfgNode *getTargetFalse() const { return TargetFalse; }
  bool isNear() const { return Kind == Near; }
  bool optimizeBranch(const CfgNode *NextNode);
  uint32_t getEmitInstCount() const override {
    uint32_t Sum = 0;
    if (Label)
      ++Sum;
    if (getTargetTrue())
      ++Sum;
    if (getTargetFalse())
      ++Sum;
    return Sum;
  }
  bool isUnconditionalBranch() const override {
    return !Label && Condition == Cond::Br_None;
  }
  const Inst *getIntraBlockBranchTarget() const override { return Label; }
  bool repointEdges(CfgNode *OldNode, CfgNode *NewNode) override;
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Br);
  }

private:
  InstX86Br(Cfg *Func, const CfgNode *TargetTrue, const CfgNode *TargetFalse,
            const InstX86Label *Label, BrCond Condition, Mode Kind);

  BrCond Condition;
  const CfgNode *TargetTrue;
  const CfgNode *TargetFalse;
  const InstX86Label *Label; // Intra-block branch target
  const Mode Kind;
};

/// Jump to a target outside this function, such as tailcall, nacljump,
/// naclret, unreachable. This is different from a Branch instruction in that
/// there is no intra-function control flow to represent.
class InstX86Jmp final : public InstX86Base {
  InstX86Jmp() = delete;
  InstX86Jmp(const InstX86Jmp &) = delete;
  InstX86Jmp &operator=(const InstX86Jmp &) = delete;

public:
  static InstX86Jmp *create(Cfg *Func, Operand *Target) {
    return new (Func->allocate<InstX86Jmp>()) InstX86Jmp(Func, Target);
  }
  Operand *getJmpTarget() const { return this->getSrc(0); }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Jmp);
  }

private:
  InstX86Jmp(Cfg *Func, Operand *Target);
};

/// Call instruction. Arguments should have already been pushed.
class InstX86Call final : public InstX86Base {
  InstX86Call() = delete;
  InstX86Call(const InstX86Call &) = delete;
  InstX86Call &operator=(const InstX86Call &) = delete;

public:
  static InstX86Call *create(Cfg *Func, Variable *Dest, Operand *CallTarget) {
    return new (Func->allocate<InstX86Call>())
        InstX86Call(Func, Dest, CallTarget);
  }
  Operand *getCallTarget() const { return this->getSrc(0); }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Call);
  }

private:
  InstX86Call(Cfg *Func, Variable *Dest, Operand *CallTarget);
};

/// Emit a one-operand (GPR) instruction.
void emitIASOpTyGPR(const Cfg *Func, Type Ty, const Operand *Var,
                    const GPREmitterOneOp &Emitter);

void emitIASAsAddrOpTyGPR(const Cfg *Func, Type Ty, const Operand *Op0,
                          const Operand *Op1, const GPREmitterAddrOp &Emitter);

void emitIASGPRShift(const Cfg *Func, Type Ty, const Variable *Var,
                     const Operand *Src, const GPREmitterShiftOp &Emitter);

void emitIASAddrOpTyGPR(const Cfg *Func, Type Ty, const AsmAddress &Addr,
                        const Operand *Src, const GPREmitterAddrOp &Emitter);

void emitIASRegOpTyXMM(const Cfg *Func, Type Ty, const Variable *Var,
                       const Operand *Src, const XmmEmitterRegOp &Emitter);

void emitIASGPRShiftDouble(const Cfg *Func, const Variable *Dest,
                           const Operand *Src1Op, const Operand *Src2Op,
                           const GPREmitterShiftD &Emitter);

template <typename DReg_t, typename SReg_t, DReg_t (*destEnc)(RegNumT),
          SReg_t (*srcEnc)(RegNumT)>
void emitIASCastRegOp(const Cfg *Func, Type DestTy, const Variable *Dest,
                      Type SrcTy, const Operand *Src,
                      const CastEmitterRegOp<DReg_t, SReg_t> &Emitter);

template <typename DReg_t, typename SReg_t, DReg_t (*destEnc)(RegNumT),
          SReg_t (*srcEnc)(RegNumT)>
void emitIASThreeOpImmOps(const Cfg *Func, Type DispatchTy,
                          const Variable *Dest, const Operand *Src0,
                          const Operand *Src1,
                          const ThreeOpImmEmitter<DReg_t, SReg_t> Emitter);

void emitIASMovlikeXMM(const Cfg *Func, const Variable *Dest,
                       const Operand *Src, const XmmEmitterMovOps Emitter);

void emitVariableBlendInst(const char *Opcode, const Inst *Instr,
                           const Cfg *Func);

void emitIASVariableBlendInst(const Inst *Instr, const Cfg *Func,
                              const XmmEmitterRegOp &Emitter);

void emitIASXmmShift(const Cfg *Func, Type Ty, const Variable *Var,
                     const Operand *Src, const XmmEmitterShiftOp &Emitter);

/// Emit a two-operand (GPR) instruction, where the dest operand is a Variable
/// that's guaranteed to be a register.
template <bool VarCanBeByte = true, bool SrcCanBeByte = true>
void emitIASRegOpTyGPR(const Cfg *Func, Type Ty, const Variable *Dst,
                       const Operand *Src, const GPREmitterRegOp &Emitter);

/// Instructions of the form x := op(x).
template <typename InstX86Base::InstKindX86 K>
class InstX86BaseInplaceopGPR : public InstX86Base {
  InstX86BaseInplaceopGPR() = delete;
  InstX86BaseInplaceopGPR(const InstX86BaseInplaceopGPR &) = delete;
  InstX86BaseInplaceopGPR &operator=(const InstX86BaseInplaceopGPR &) = delete;

public:
  using Base = InstX86BaseInplaceopGPR<K>;

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrEmit();
    assert(this->getSrcSize() == 1);
    Str << "\t" << Opcode << "\t";
    this->getSrc(0)->emit(Func);
  }
  void emitIAS(const Cfg *Func) const override {
    assert(this->getSrcSize() == 1);
    const Variable *Var = this->getDest();
    Type Ty = Var->getType();
    emitIASOpTyGPR(Func, Ty, Var, Emitter);
  }
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    this->dumpDest(Func);
    Str << " = " << Opcode << "." << this->getDest()->getType() << " ";
    this->dumpSources(Func);
  }
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, K);
  }

protected:
  InstX86BaseInplaceopGPR(Cfg *Func, Operand *SrcDest)
      : InstX86Base(Func, K, 1, llvm::dyn_cast<Variable>(SrcDest)) {
    this->addSource(SrcDest);
  }

private:
  static const char *const Opcode;
  static const GPREmitterOneOp Emitter;
};

/// Instructions of the form x := op(y).
template <typename InstX86Base::InstKindX86 K>
class InstX86BaseUnaryopGPR : public InstX86Base {
  InstX86BaseUnaryopGPR() = delete;
  InstX86BaseUnaryopGPR(const InstX86BaseUnaryopGPR &) = delete;
  InstX86BaseUnaryopGPR &operator=(const InstX86BaseUnaryopGPR &) = delete;

public:
  using Base = InstX86BaseUnaryopGPR<K>;

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrEmit();
    assert(this->getSrcSize() == 1);
    Type SrcTy = this->getSrc(0)->getType();
    Type DestTy = this->getDest()->getType();
    Str << "\t" << Opcode << this->getWidthString(SrcTy);
    // Movsx and movzx need both the source and dest type width letter to
    // define the operation. The other unary operations have the same source
    // and dest type and as a result need only one letter.
    if (SrcTy != DestTy)
      Str << this->getWidthString(DestTy);
    Str << "\t";
    this->getSrc(0)->emit(Func);
    Str << ", ";
    this->getDest()->emit(Func);
  }
  void emitIAS(const Cfg *Func) const override {
    assert(this->getSrcSize() == 1 && K != InstX86Base::Lea);
    const Variable *Var = this->getDest();
    Type Ty = Var->getType();
    const Operand *Src = this->getSrc(0);
    emitIASRegOpTyGPR(Func, Ty, Var, Src, Emitter);
  }
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    this->dumpDest(Func);
    Str << " = " << Opcode << "." << this->getSrc(0)->getType() << " ";
    this->dumpSources(Func);
  }

  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, K);
  }

protected:
  InstX86BaseUnaryopGPR(Cfg *Func, Variable *Dest, Operand *Src)
      : InstX86Base(Func, K, 1, Dest) {
    this->addSource(Src);
  }

  static const char *const Opcode;
  static const GPREmitterRegOp Emitter;
};

template <typename InstX86Base::InstKindX86 K>
class InstX86BaseUnaryopXmm : public InstX86Base {
  InstX86BaseUnaryopXmm() = delete;
  InstX86BaseUnaryopXmm(const InstX86BaseUnaryopXmm &) = delete;
  InstX86BaseUnaryopXmm &operator=(const InstX86BaseUnaryopXmm &) = delete;

public:
  using Base = InstX86BaseUnaryopXmm<K>;

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrEmit();
    assert(this->getSrcSize() == 1);
    Str << "\t" << Opcode << "\t";
    this->getSrc(0)->emit(Func);
    Str << ", ";
    this->getDest()->emit(Func);
  }
  void emitIAS(const Cfg *Func) const override {
    Type Ty = this->getDest()->getType();
    assert(this->getSrcSize() == 1);
    emitIASRegOpTyXMM(Func, Ty, this->getDest(), this->getSrc(0), Emitter);
  }
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    this->dumpDest(Func);
    Str << " = " << Opcode << "." << this->getDest()->getType() << " ";
    this->dumpSources(Func);
  }
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, K);
  }

protected:
  InstX86BaseUnaryopXmm(Cfg *Func, Variable *Dest, Operand *Src)
      : InstX86Base(Func, K, 1, Dest) {
    this->addSource(Src);
  }

  static const char *const Opcode;
  static const XmmEmitterRegOp Emitter;
};

template <typename InstX86Base::InstKindX86 K>
class InstX86BaseBinopGPRShift : public InstX86Base {
  InstX86BaseBinopGPRShift() = delete;
  InstX86BaseBinopGPRShift(const InstX86BaseBinopGPRShift &) = delete;
  InstX86BaseBinopGPRShift &
  operator=(const InstX86BaseBinopGPRShift &) = delete;

public:
  using Base = InstX86BaseBinopGPRShift<K>;

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    this->emitTwoAddress(Func, Opcode);
  }
  void emitIAS(const Cfg *Func) const override {
    Type Ty = this->getDest()->getType();
    assert(this->getSrcSize() == 2);
    emitIASGPRShift(Func, Ty, this->getDest(), this->getSrc(1), Emitter);
  }
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    this->dumpDest(Func);
    Str << " = " << Opcode << "." << this->getDest()->getType() << " ";
    this->dumpSources(Func);
  }
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, K);
  }

protected:
  InstX86BaseBinopGPRShift(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86Base(Func, K, 2, Dest) {
    this->addSource(Dest);
    this->addSource(Source);
  }

  static const char *const Opcode;
  static const GPREmitterShiftOp Emitter;
};

template <typename InstX86Base::InstKindX86 K>
class InstX86BaseBinopGPR : public InstX86Base {
  InstX86BaseBinopGPR() = delete;
  InstX86BaseBinopGPR(const InstX86BaseBinopGPR &) = delete;
  InstX86BaseBinopGPR &operator=(const InstX86BaseBinopGPR &) = delete;

public:
  using Base = InstX86BaseBinopGPR<K>;

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    this->emitTwoAddress(Func, Opcode);
  }
  void emitIAS(const Cfg *Func) const override {
    Type Ty = this->getDest()->getType();
    assert(this->getSrcSize() == 2);
    static_assert(K != InstX86Base::Lea, "Lea should be a unaryop.");
    emitIASRegOpTyGPR(Func, Ty, this->getDest(), this->getSrc(1), Emitter);
  }
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    this->dumpDest(Func);
    Str << " = " << Opcode << "." << this->getDest()->getType() << " ";
    this->dumpSources(Func);
  }
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, K);
  }

protected:
  InstX86BaseBinopGPR(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86Base(Func, K, 2, Dest) {
    this->addSource(Dest);
    this->addSource(Source);
  }

  static const char *const Opcode;
  static const GPREmitterRegOp Emitter;
};

template <typename InstX86Base::InstKindX86 K>
class InstX86BaseBinopRMW : public InstX86Base {
  InstX86BaseBinopRMW() = delete;
  InstX86BaseBinopRMW(const InstX86BaseBinopRMW &) = delete;
  InstX86BaseBinopRMW &operator=(const InstX86BaseBinopRMW &) = delete;

public:
  using Base = InstX86BaseBinopRMW<K>;

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    this->emitTwoAddress(Func, Opcode);
  }
  void emitIAS(const Cfg *Func) const override {
    Type Ty = this->getSrc(0)->getType();
    assert(this->getSrcSize() == 2);
    emitIASAsAddrOpTyGPR(Func, Ty, this->getSrc(0), this->getSrc(1), Emitter);
  }
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    Str << Opcode << "." << this->getSrc(0)->getType() << " ";
    this->dumpSources(Func);
  }
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, K);
  }

protected:
  InstX86BaseBinopRMW(Cfg *Func, X86OperandMem *DestSrc0, Operand *Src1)
      : InstX86Base(Func, K, 2, nullptr) {
    this->addSource(DestSrc0);
    this->addSource(Src1);
  }

  static const char *const Opcode;
  static const GPREmitterAddrOp Emitter;
};

template <typename InstX86Base::InstKindX86 K, bool NeedsElementType,
          typename InstX86Base::SseSuffix Suffix>
class InstX86BaseBinopXmm : public InstX86Base {
  InstX86BaseBinopXmm() = delete;
  InstX86BaseBinopXmm(const InstX86BaseBinopXmm &) = delete;
  InstX86BaseBinopXmm &operator=(const InstX86BaseBinopXmm &) = delete;

public:
  using Base = InstX86BaseBinopXmm<K, NeedsElementType, Suffix>;

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    this->validateVectorAddrMode();
    const Type DestTy = ArithmeticTypeOverride == IceType_void
                            ? this->getDest()->getType()
                            : ArithmeticTypeOverride;
    const char *SuffixString = getSseSuffixString(DestTy, Suffix);
    this->emitTwoAddress(Func, Opcode, SuffixString);
  }
  void emitIAS(const Cfg *Func) const override {
    this->validateVectorAddrMode();
    Type Ty = this->getDest()->getType();
    if (NeedsElementType)
      Ty = typeElementType(Ty);
    assert(this->getSrcSize() == 2);
    emitIASRegOpTyXMM(Func, Ty, this->getDest(), this->getSrc(1), Emitter);
  }
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    this->dumpDest(Func);
    Str << " = " << Opcode << "." << this->getDest()->getType() << " ";
    this->dumpSources(Func);
  }
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, K);
  }

protected:
  InstX86BaseBinopXmm(Cfg *Func, Variable *Dest, Operand *Source,
                      Type ArithmeticTypeOverride = IceType_void)
      : InstX86Base(Func, K, 2, Dest),
        ArithmeticTypeOverride(ArithmeticTypeOverride) {
    this->addSource(Dest);
    this->addSource(Source);
  }

  const Type ArithmeticTypeOverride;
  static const char *const Opcode;
  static const XmmEmitterRegOp Emitter;
};

template <typename InstX86Base::InstKindX86 K, bool AllowAllTypes = false>
class InstX86BaseBinopXmmShift : public InstX86Base {
  InstX86BaseBinopXmmShift() = delete;
  InstX86BaseBinopXmmShift(const InstX86BaseBinopXmmShift &) = delete;
  InstX86BaseBinopXmmShift &
  operator=(const InstX86BaseBinopXmmShift &) = delete;

public:
  using Base = InstX86BaseBinopXmmShift<K, AllowAllTypes>;

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    this->validateVectorAddrMode();
    // Shift operations are always integral, and hence always need a suffix.
    const Type DestTy = this->getDest()->getType();
    const char *SuffixString = getSseSuffixString(DestTy, SseSuffix::Integral);
    this->emitTwoAddress(Func, this->Opcode, SuffixString);
  }
  void emitIAS(const Cfg *Func) const override {
    this->validateVectorAddrMode();
    Type Ty = this->getDest()->getType();
    assert(AllowAllTypes || isVectorType(Ty));
    Type ElementTy = typeElementType(Ty);
    assert(this->getSrcSize() == 2);
    emitIASXmmShift(Func, ElementTy, this->getDest(), this->getSrc(1), Emitter);
  }
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    this->dumpDest(Func);
    Str << " = " << Opcode << "." << this->getDest()->getType() << " ";
    this->dumpSources(Func);
  }
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, K);
  }

protected:
  InstX86BaseBinopXmmShift(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86Base(Func, K, 2, Dest) {
    this->addSource(Dest);
    this->addSource(Source);
  }

  static const char *const Opcode;
  static const XmmEmitterShiftOp Emitter;
};

template <typename InstX86Base::InstKindX86 K>
class InstX86BaseTernop : public InstX86Base {
  InstX86BaseTernop() = delete;
  InstX86BaseTernop(const InstX86BaseTernop &) = delete;
  InstX86BaseTernop &operator=(const InstX86BaseTernop &) = delete;

public:
  using Base = InstX86BaseTernop<K>;

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrEmit();
    assert(this->getSrcSize() == 3);
    Str << "\t" << Opcode << "\t";
    this->getSrc(2)->emit(Func);
    Str << ", ";
    this->getSrc(1)->emit(Func);
    Str << ", ";
    this->getDest()->emit(Func);
  }
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    this->dumpDest(Func);
    Str << " = " << Opcode << "." << this->getDest()->getType() << " ";
    this->dumpSources(Func);
  }
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, K);
  }

protected:
  InstX86BaseTernop(Cfg *Func, Variable *Dest, Operand *Source1,
                    Operand *Source2)
      : InstX86Base(Func, K, 3, Dest) {
    this->addSource(Dest);
    this->addSource(Source1);
    this->addSource(Source2);
  }

  static const char *const Opcode;
};

// Instructions of the form x := y op z
template <typename InstX86Base::InstKindX86 K>
class InstX86BaseThreeAddressop : public InstX86Base {
  InstX86BaseThreeAddressop() = delete;
  InstX86BaseThreeAddressop(const InstX86BaseThreeAddressop &) = delete;
  InstX86BaseThreeAddressop &
  operator=(const InstX86BaseThreeAddressop &) = delete;

public:
  using Base = InstX86BaseThreeAddressop<K>;

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrEmit();
    assert(this->getSrcSize() == 2);
    Str << "\t" << Opcode << "\t";
    this->getSrc(1)->emit(Func);
    Str << ", ";
    this->getSrc(0)->emit(Func);
    Str << ", ";
    this->getDest()->emit(Func);
  }
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    this->dumpDest(Func);
    Str << " = " << Opcode << "." << this->getDest()->getType() << " ";
    this->dumpSources(Func);
  }
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, K);
  }

protected:
  InstX86BaseThreeAddressop(Cfg *Func, Variable *Dest, Operand *Source0,
                            Operand *Source1)
      : InstX86Base(Func, K, 2, Dest) {
    this->addSource(Source0);
    this->addSource(Source1);
  }

  static const char *const Opcode;
};

/// Base class for assignment instructions
template <typename InstX86Base::InstKindX86 K>
class InstX86BaseMovlike : public InstX86Base {
  InstX86BaseMovlike() = delete;
  InstX86BaseMovlike(const InstX86BaseMovlike &) = delete;
  InstX86BaseMovlike &operator=(const InstX86BaseMovlike &) = delete;

public:
  using Base = InstX86BaseMovlike<K>;

  bool isRedundantAssign() const override {
    if (const auto *SrcVar = llvm::dyn_cast<const Variable>(this->getSrc(0))) {
      if (SrcVar->hasReg() && this->Dest->hasReg()) {
        // An assignment between physical registers is considered redundant if
        // they have the same base register and the same encoding. E.g.:
        //   mov cl, ecx ==> redundant
        //   mov ch, ecx ==> not redundant due to different encodings
        //   mov ch, ebp ==> not redundant due to different base registers
        //   mov ecx, ecx ==> redundant, and dangerous in x86-64. i64 zexting
        //                    is handled by Inst86Zext.
        const auto SrcReg = SrcVar->getRegNum();
        const auto DestReg = this->Dest->getRegNum();
        return (RegX8664::getEncoding(SrcReg) ==
                RegX8664::getEncoding(DestReg)) &&
               (RegX8664::getBaseReg(SrcReg) == RegX8664::getBaseReg(DestReg));
      }
    }
    return checkForRedundantAssign(this->getDest(), this->getSrc(0));
  }
  bool isVarAssign() const override {
    return llvm::isa<Variable>(this->getSrc(0));
  }
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    Str << Opcode << "." << this->getDest()->getType() << " ";
    this->dumpDest(Func);
    Str << ", ";
    this->dumpSources(Func);
  }
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, K);
  }

protected:
  InstX86BaseMovlike(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86Base(Func, K, 1, Dest) {
    this->addSource(Source);
    // For an integer assignment, make sure it's either a same-type assignment
    // or a truncation.
    assert(!isScalarIntegerType(Dest->getType()) ||
           (typeWidthInBytes(Dest->getType()) <=
            typeWidthInBytes(Source->getType())));
  }

  static const char *const Opcode;
};

class InstX86Bswap : public InstX86BaseInplaceopGPR<InstX86Base::Bswap> {
public:
  static InstX86Bswap *create(Cfg *Func, Operand *SrcDest) {
    return new (Func->allocate<InstX86Bswap>()) InstX86Bswap(Func, SrcDest);
  }

private:
  InstX86Bswap(Cfg *Func, Operand *SrcDest)
      : InstX86BaseInplaceopGPR<InstX86Base::Bswap>(Func, SrcDest) {}
};

class InstX86Neg : public InstX86BaseInplaceopGPR<InstX86Base::Neg> {
public:
  static InstX86Neg *create(Cfg *Func, Operand *SrcDest) {
    return new (Func->allocate<InstX86Neg>()) InstX86Neg(Func, SrcDest);
  }

private:
  InstX86Neg(Cfg *Func, Operand *SrcDest)
      : InstX86BaseInplaceopGPR<InstX86Base::Neg>(Func, SrcDest) {}
};

class InstX86Bsf : public InstX86BaseUnaryopGPR<InstX86Base::Bsf> {
public:
  static InstX86Bsf *create(Cfg *Func, Variable *Dest, Operand *Src) {
    return new (Func->allocate<InstX86Bsf>()) InstX86Bsf(Func, Dest, Src);
  }

private:
  InstX86Bsf(Cfg *Func, Variable *Dest, Operand *Src)
      : InstX86BaseUnaryopGPR<InstX86Base::Bsf>(Func, Dest, Src) {}
};

class InstX86Bsr : public InstX86BaseUnaryopGPR<InstX86Base::Bsr> {
public:
  static InstX86Bsr *create(Cfg *Func, Variable *Dest, Operand *Src) {
    return new (Func->allocate<InstX86Bsr>()) InstX86Bsr(Func, Dest, Src);
  }

private:
  InstX86Bsr(Cfg *Func, Variable *Dest, Operand *Src)
      : InstX86BaseUnaryopGPR<InstX86Base::Bsr>(Func, Dest, Src) {}
};

class InstX86Lea : public InstX86BaseUnaryopGPR<InstX86Base::Lea> {
public:
  static InstX86Lea *create(Cfg *Func, Variable *Dest, Operand *Src) {
    return new (Func->allocate<InstX86Lea>()) InstX86Lea(Func, Dest, Src);
  }

  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Lea(Cfg *Func, Variable *Dest, Operand *Src)
      : InstX86BaseUnaryopGPR<InstX86Base::Lea>(Func, Dest, Src) {}

  Inst *deoptToAddOrNull(const Cfg *Func) const;
};

// Cbwdq instruction - wrapper for cbw, cwd, and cdq
class InstX86Cbwdq : public InstX86BaseUnaryopGPR<InstX86Base::Cbwdq> {
public:
  static InstX86Cbwdq *create(Cfg *Func, Variable *Dest, Operand *Src) {
    return new (Func->allocate<InstX86Cbwdq>()) InstX86Cbwdq(Func, Dest, Src);
  }

  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Cbwdq(Cfg *Func, Variable *Dest, Operand *Src)
      : InstX86BaseUnaryopGPR<InstX86Base::Cbwdq>(Func, Dest, Src) {}
};

class InstX86Movsx : public InstX86BaseUnaryopGPR<InstX86Base::Movsx> {
public:
  static InstX86Movsx *create(Cfg *Func, Variable *Dest, Operand *Src) {
    assert(typeWidthInBytes(Dest->getType()) >
           typeWidthInBytes(Src->getType()));
    return new (Func->allocate<InstX86Movsx>()) InstX86Movsx(Func, Dest, Src);
  }

  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Movsx(Cfg *Func, Variable *Dest, Operand *Src)
      : InstX86BaseUnaryopGPR<InstX86Base::Movsx>(Func, Dest, Src) {}
};

class InstX86Movzx : public InstX86BaseUnaryopGPR<InstX86Base::Movzx> {
public:
  static InstX86Movzx *create(Cfg *Func, Variable *Dest, Operand *Src) {
    assert(typeWidthInBytes(Dest->getType()) >
           typeWidthInBytes(Src->getType()));
    return new (Func->allocate<InstX86Movzx>()) InstX86Movzx(Func, Dest, Src);
  }

  void emit(const Cfg *Func) const override;

  void emitIAS(const Cfg *Func) const override;

  void setMustKeep() { MustKeep = true; }

private:
  bool MustKeep = false;

  InstX86Movzx(Cfg *Func, Variable *Dest, Operand *Src)
      : InstX86BaseUnaryopGPR<InstX86Base::Movzx>(Func, Dest, Src) {}

  bool mayBeElided(const Variable *Dest, const Operand *Src) const;
};

class InstX86Movd : public InstX86BaseUnaryopXmm<InstX86Base::Movd> {
public:
  static InstX86Movd *create(Cfg *Func, Variable *Dest, Operand *Src) {
    return new (Func->allocate<InstX86Movd>()) InstX86Movd(Func, Dest, Src);
  }

  void emit(const Cfg *Func) const override;

  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Movd(Cfg *Func, Variable *Dest, Operand *Src)
      : InstX86BaseUnaryopXmm<InstX86Base::Movd>(Func, Dest, Src) {}
};

class InstX86Movmsk final : public InstX86Base {
  InstX86Movmsk() = delete;
  InstX86Movmsk(const InstX86Movmsk &) = delete;
  InstX86Movmsk &operator=(const InstX86Movmsk &) = delete;

public:
  static InstX86Movmsk *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Movmsk>())
        InstX86Movmsk(Func, Dest, Source);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Movmsk);
  }

private:
  InstX86Movmsk(Cfg *Func, Variable *Dest, Operand *Source);
};

class InstX86Sqrt : public InstX86BaseUnaryopXmm<InstX86Base::Sqrt> {
public:
  static InstX86Sqrt *create(Cfg *Func, Variable *Dest, Operand *Src) {
    return new (Func->allocate<InstX86Sqrt>()) InstX86Sqrt(Func, Dest, Src);
  }

  virtual void emit(const Cfg *Func) const override;

private:
  InstX86Sqrt(Cfg *Func, Variable *Dest, Operand *Src)
      : InstX86BaseUnaryopXmm<InstX86Base::Sqrt>(Func, Dest, Src) {}
};

/// Move/assignment instruction - wrapper for mov/movss/movsd.
class InstX86Mov : public InstX86BaseMovlike<InstX86Base::Mov> {
public:
  static InstX86Mov *create(Cfg *Func, Variable *Dest, Operand *Source) {
    assert(!isScalarIntegerType(Dest->getType()) ||
           (typeWidthInBytes(Dest->getType()) <=
            typeWidthInBytes(Source->getType())));
    return new (Func->allocate<InstX86Mov>()) InstX86Mov(Func, Dest, Source);
  }

  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Mov(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseMovlike<InstX86Base::Mov>(Func, Dest, Source) {}
};

/// Move packed - copy 128 bit values between XMM registers, or mem128 and XMM
/// registers.
class InstX86Movp : public InstX86BaseMovlike<InstX86Base::Movp> {
public:
  static InstX86Movp *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Movp>()) InstX86Movp(Func, Dest, Source);
  }

  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Movp(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseMovlike<InstX86Base::Movp>(Func, Dest, Source) {}
};

/// Movq - copy between XMM registers, or mem64 and XMM registers.
class InstX86Movq : public InstX86BaseMovlike<InstX86Base::Movq> {
public:
  static InstX86Movq *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Movq>()) InstX86Movq(Func, Dest, Source);
  }

  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Movq(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseMovlike<InstX86Base::Movq>(Func, Dest, Source) {}
};

class InstX86Add : public InstX86BaseBinopGPR<InstX86Base::Add> {
public:
  static InstX86Add *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Add>()) InstX86Add(Func, Dest, Source);
  }

private:
  InstX86Add(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopGPR<InstX86Base::Add>(Func, Dest, Source) {}
};

class InstX86AddRMW : public InstX86BaseBinopRMW<InstX86Base::AddRMW> {
public:
  static InstX86AddRMW *create(Cfg *Func, X86OperandMem *DestSrc0,
                               Operand *Src1) {
    return new (Func->allocate<InstX86AddRMW>())
        InstX86AddRMW(Func, DestSrc0, Src1);
  }

private:
  InstX86AddRMW(Cfg *Func, X86OperandMem *DestSrc0, Operand *Src1)
      : InstX86BaseBinopRMW<InstX86Base::AddRMW>(Func, DestSrc0, Src1) {}
};

class InstX86Addps
    : public InstX86BaseBinopXmm<InstX86Base::Addps, true,
                                 InstX86Base::SseSuffix::Packed> {
public:
  static InstX86Addps *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Addps>())
        InstX86Addps(Func, Dest, Source);
  }

private:
  InstX86Addps(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Addps, true,
                            InstX86Base::SseSuffix::Packed>(Func, Dest,
                                                            Source) {}
};

class InstX86Adc : public InstX86BaseBinopGPR<InstX86Base::Adc> {
public:
  static InstX86Adc *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Adc>()) InstX86Adc(Func, Dest, Source);
  }

private:
  InstX86Adc(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopGPR<InstX86Base::Adc>(Func, Dest, Source) {}
};

class InstX86AdcRMW : public InstX86BaseBinopRMW<InstX86Base::AdcRMW> {
public:
  static InstX86AdcRMW *create(Cfg *Func, X86OperandMem *DestSrc0,
                               Operand *Src1) {
    return new (Func->allocate<InstX86AdcRMW>())
        InstX86AdcRMW(Func, DestSrc0, Src1);
  }

private:
  InstX86AdcRMW(Cfg *Func, X86OperandMem *DestSrc0, Operand *Src1)
      : InstX86BaseBinopRMW<InstX86Base::AdcRMW>(Func, DestSrc0, Src1) {}
};

class InstX86Addss
    : public InstX86BaseBinopXmm<InstX86Base::Addss, false,
                                 InstX86Base::SseSuffix::Scalar> {
public:
  static InstX86Addss *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Addss>())
        InstX86Addss(Func, Dest, Source);
  }

private:
  InstX86Addss(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Addss, false,
                            InstX86Base::SseSuffix::Scalar>(Func, Dest,
                                                            Source) {}
};

class InstX86Padd
    : public InstX86BaseBinopXmm<InstX86Base::Padd, true,
                                 InstX86Base::SseSuffix::Integral> {
public:
  static InstX86Padd *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Padd>()) InstX86Padd(Func, Dest, Source);
  }

private:
  InstX86Padd(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Padd, true,
                            InstX86Base::SseSuffix::Integral>(Func, Dest,
                                                              Source) {}
};

class InstX86Padds
    : public InstX86BaseBinopXmm<InstX86Base::Padds, true,
                                 InstX86Base::SseSuffix::Integral> {
public:
  static InstX86Padds *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Padds>())
        InstX86Padds(Func, Dest, Source);
  }

private:
  InstX86Padds(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Padds, true,
                            InstX86Base::SseSuffix::Integral>(Func, Dest,
                                                              Source) {}
};

class InstX86Paddus
    : public InstX86BaseBinopXmm<InstX86Base::Paddus, true,
                                 InstX86Base::SseSuffix::Integral> {
public:
  static InstX86Paddus *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Paddus>())
        InstX86Paddus(Func, Dest, Source);
  }

private:
  InstX86Paddus(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Paddus, true,
                            InstX86Base::SseSuffix::Integral>(Func, Dest,
                                                              Source) {}
};

class InstX86Sub : public InstX86BaseBinopGPR<InstX86Base::Sub> {
public:
  static InstX86Sub *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Sub>()) InstX86Sub(Func, Dest, Source);
  }

private:
  InstX86Sub(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopGPR<InstX86Base::Sub>(Func, Dest, Source) {}
};

class InstX86SubRMW : public InstX86BaseBinopRMW<InstX86Base::SubRMW> {
public:
  static InstX86SubRMW *create(Cfg *Func, X86OperandMem *DestSrc0,
                               Operand *Src1) {
    return new (Func->allocate<InstX86SubRMW>())
        InstX86SubRMW(Func, DestSrc0, Src1);
  }

private:
  InstX86SubRMW(Cfg *Func, X86OperandMem *DestSrc0, Operand *Src1)
      : InstX86BaseBinopRMW<InstX86Base::SubRMW>(Func, DestSrc0, Src1) {}
};

class InstX86Subps
    : public InstX86BaseBinopXmm<InstX86Base::Subps, true,
                                 InstX86Base::SseSuffix::Packed> {
public:
  static InstX86Subps *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Subps>())
        InstX86Subps(Func, Dest, Source);
  }

private:
  InstX86Subps(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Subps, true,
                            InstX86Base::SseSuffix::Packed>(Func, Dest,
                                                            Source) {}
};

class InstX86Subss
    : public InstX86BaseBinopXmm<InstX86Base::Subss, false,
                                 InstX86Base::SseSuffix::Scalar> {
public:
  static InstX86Subss *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Subss>())
        InstX86Subss(Func, Dest, Source);
  }

private:
  InstX86Subss(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Subss, false,
                            InstX86Base::SseSuffix::Scalar>(Func, Dest,
                                                            Source) {}
};

class InstX86Sbb : public InstX86BaseBinopGPR<InstX86Base::Sbb> {
public:
  static InstX86Sbb *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Sbb>()) InstX86Sbb(Func, Dest, Source);
  }

private:
  InstX86Sbb(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopGPR<InstX86Base::Sbb>(Func, Dest, Source) {}
};

class InstX86SbbRMW : public InstX86BaseBinopRMW<InstX86Base::SbbRMW> {
public:
  static InstX86SbbRMW *create(Cfg *Func, X86OperandMem *DestSrc0,
                               Operand *Src1) {
    return new (Func->allocate<InstX86SbbRMW>())
        InstX86SbbRMW(Func, DestSrc0, Src1);
  }

private:
  InstX86SbbRMW(Cfg *Func, X86OperandMem *DestSrc0, Operand *Src1)
      : InstX86BaseBinopRMW<InstX86Base::SbbRMW>(Func, DestSrc0, Src1) {}
};

class InstX86Psub
    : public InstX86BaseBinopXmm<InstX86Base::Psub, true,
                                 InstX86Base::SseSuffix::Integral> {
public:
  static InstX86Psub *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Psub>()) InstX86Psub(Func, Dest, Source);
  }

private:
  InstX86Psub(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Psub, true,
                            InstX86Base::SseSuffix::Integral>(Func, Dest,
                                                              Source) {}
};

class InstX86Psubs
    : public InstX86BaseBinopXmm<InstX86Base::Psubs, true,
                                 InstX86Base::SseSuffix::Integral> {
public:
  static InstX86Psubs *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Psubs>())
        InstX86Psubs(Func, Dest, Source);
  }

private:
  InstX86Psubs(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Psubs, true,
                            InstX86Base::SseSuffix::Integral>(Func, Dest,
                                                              Source) {}
};

class InstX86Psubus
    : public InstX86BaseBinopXmm<InstX86Base::Psubus, true,
                                 InstX86Base::SseSuffix::Integral> {
public:
  static InstX86Psubus *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Psubus>())
        InstX86Psubus(Func, Dest, Source);
  }

private:
  InstX86Psubus(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Psubus, true,
                            InstX86Base::SseSuffix::Integral>(Func, Dest,
                                                              Source) {}
};

class InstX86And : public InstX86BaseBinopGPR<InstX86Base::And> {
public:
  static InstX86And *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86And>()) InstX86And(Func, Dest, Source);
  }

private:
  InstX86And(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopGPR<InstX86Base::And>(Func, Dest, Source) {}
};

class InstX86Andnps
    : public InstX86BaseBinopXmm<InstX86Base::Andnps, true,
                                 InstX86Base::SseSuffix::Packed> {
public:
  static InstX86Andnps *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Andnps>())
        InstX86Andnps(Func, Dest, Source);
  }

private:
  InstX86Andnps(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Andnps, true,
                            InstX86Base::SseSuffix::Packed>(Func, Dest,
                                                            Source) {}
};

class InstX86Andps
    : public InstX86BaseBinopXmm<InstX86Base::Andps, true,
                                 InstX86Base::SseSuffix::Packed> {
public:
  static InstX86Andps *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Andps>())
        InstX86Andps(Func, Dest, Source);
  }

private:
  InstX86Andps(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Andps, true,
                            InstX86Base::SseSuffix::Packed>(Func, Dest,
                                                            Source) {}
};

class InstX86AndRMW : public InstX86BaseBinopRMW<InstX86Base::AndRMW> {
public:
  static InstX86AndRMW *create(Cfg *Func, X86OperandMem *DestSrc0,
                               Operand *Src1) {
    return new (Func->allocate<InstX86AndRMW>())
        InstX86AndRMW(Func, DestSrc0, Src1);
  }

private:
  InstX86AndRMW(Cfg *Func, X86OperandMem *DestSrc0, Operand *Src1)
      : InstX86BaseBinopRMW<InstX86Base::AndRMW>(Func, DestSrc0, Src1) {}
};

class InstX86Pand : public InstX86BaseBinopXmm<InstX86Base::Pand, false,
                                               InstX86Base::SseSuffix::None> {
public:
  static InstX86Pand *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Pand>()) InstX86Pand(Func, Dest, Source);
  }

private:
  InstX86Pand(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Pand, false,
                            InstX86Base::SseSuffix::None>(Func, Dest, Source) {}
};

class InstX86Pandn : public InstX86BaseBinopXmm<InstX86Base::Pandn, false,
                                                InstX86Base::SseSuffix::None> {
public:
  static InstX86Pandn *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Pandn>())
        InstX86Pandn(Func, Dest, Source);
  }

private:
  InstX86Pandn(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Pandn, false,
                            InstX86Base::SseSuffix::None>(Func, Dest, Source) {}
};

class InstX86Maxss
    : public InstX86BaseBinopXmm<InstX86Base::Maxss, true,
                                 InstX86Base::SseSuffix::Scalar> {
public:
  static InstX86Maxss *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Maxss>())
        InstX86Maxss(Func, Dest, Source);
  }

private:
  InstX86Maxss(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Maxss, true,
                            InstX86Base::SseSuffix::Scalar>(Func, Dest,
                                                            Source) {}
};

class InstX86Minss
    : public InstX86BaseBinopXmm<InstX86Base::Minss, true,
                                 InstX86Base::SseSuffix::Scalar> {
public:
  static InstX86Minss *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Minss>())
        InstX86Minss(Func, Dest, Source);
  }

private:
  InstX86Minss(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Minss, true,
                            InstX86Base::SseSuffix::Scalar>(Func, Dest,
                                                            Source) {}
};

class InstX86Maxps : public InstX86BaseBinopXmm<InstX86Base::Maxps, true,
                                                InstX86Base::SseSuffix::None> {
public:
  static InstX86Maxps *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Maxps>())
        InstX86Maxps(Func, Dest, Source);
  }

private:
  InstX86Maxps(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Maxps, true,
                            InstX86Base::SseSuffix::None>(Func, Dest, Source) {}
};

class InstX86Minps : public InstX86BaseBinopXmm<InstX86Base::Minps, true,
                                                InstX86Base::SseSuffix::None> {
public:
  static InstX86Minps *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Minps>())
        InstX86Minps(Func, Dest, Source);
  }

private:
  InstX86Minps(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Minps, true,
                            InstX86Base::SseSuffix::None>(Func, Dest, Source) {}
};

class InstX86Or : public InstX86BaseBinopGPR<InstX86Base::Or> {
public:
  static InstX86Or *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Or>()) InstX86Or(Func, Dest, Source);
  }

private:
  InstX86Or(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopGPR<InstX86Base::Or>(Func, Dest, Source) {}
};

class InstX86Orps : public InstX86BaseBinopXmm<InstX86Base::Orps, true,
                                               InstX86Base::SseSuffix::Packed> {
public:
  static InstX86Orps *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Orps>()) InstX86Orps(Func, Dest, Source);
  }

private:
  InstX86Orps(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Orps, true,
                            InstX86Base::SseSuffix::Packed>(Func, Dest,
                                                            Source) {}
};

class InstX86OrRMW : public InstX86BaseBinopRMW<InstX86Base::OrRMW> {
public:
  static InstX86OrRMW *create(Cfg *Func, X86OperandMem *DestSrc0,
                              Operand *Src1) {
    return new (Func->allocate<InstX86OrRMW>())
        InstX86OrRMW(Func, DestSrc0, Src1);
  }

private:
  InstX86OrRMW(Cfg *Func, X86OperandMem *DestSrc0, Operand *Src1)
      : InstX86BaseBinopRMW<InstX86Base::OrRMW>(Func, DestSrc0, Src1) {}
};

class InstX86Por : public InstX86BaseBinopXmm<InstX86Base::Por, false,
                                              InstX86Base::SseSuffix::None> {
public:
  static InstX86Por *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Por>()) InstX86Por(Func, Dest, Source);
  }

private:
  InstX86Por(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Por, false,
                            InstX86Base::SseSuffix::None>(Func, Dest, Source) {}
};

class InstX86Xor : public InstX86BaseBinopGPR<InstX86Base::Xor> {
public:
  static InstX86Xor *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Xor>()) InstX86Xor(Func, Dest, Source);
  }

private:
  InstX86Xor(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopGPR<InstX86Base::Xor>(Func, Dest, Source) {}
};

class InstX86Xorps
    : public InstX86BaseBinopXmm<InstX86Base::Xorps, true,
                                 InstX86Base::SseSuffix::Packed> {
public:
  static InstX86Xorps *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Xorps>())
        InstX86Xorps(Func, Dest, Source);
  }

private:
  InstX86Xorps(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Xorps, true,
                            InstX86Base::SseSuffix::Packed>(Func, Dest,
                                                            Source) {}
};

class InstX86XorRMW : public InstX86BaseBinopRMW<InstX86Base::XorRMW> {
public:
  static InstX86XorRMW *create(Cfg *Func, X86OperandMem *DestSrc0,
                               Operand *Src1) {
    return new (Func->allocate<InstX86XorRMW>())
        InstX86XorRMW(Func, DestSrc0, Src1);
  }

private:
  InstX86XorRMW(Cfg *Func, X86OperandMem *DestSrc0, Operand *Src1)
      : InstX86BaseBinopRMW<InstX86Base::XorRMW>(Func, DestSrc0, Src1) {}
};

class InstX86Pxor : public InstX86BaseBinopXmm<InstX86Base::Pxor, false,
                                               InstX86Base::SseSuffix::None> {
public:
  static InstX86Pxor *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Pxor>()) InstX86Pxor(Func, Dest, Source);
  }

private:
  InstX86Pxor(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Pxor, false,
                            InstX86Base::SseSuffix::None>(Func, Dest, Source) {}
};

class InstX86Imul : public InstX86BaseBinopGPR<InstX86Base::Imul> {
public:
  static InstX86Imul *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Imul>()) InstX86Imul(Func, Dest, Source);
  }

  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Imul(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopGPR<InstX86Base::Imul>(Func, Dest, Source) {}
};

class InstX86ImulImm : public InstX86BaseThreeAddressop<InstX86Base::ImulImm> {
public:
  static InstX86ImulImm *create(Cfg *Func, Variable *Dest, Operand *Source0,
                                Operand *Source1) {
    return new (Func->allocate<InstX86ImulImm>())
        InstX86ImulImm(Func, Dest, Source0, Source1);
  }

  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;

private:
  InstX86ImulImm(Cfg *Func, Variable *Dest, Operand *Source0, Operand *Source1)
      : InstX86BaseThreeAddressop<InstX86Base::ImulImm>(Func, Dest, Source0,
                                                        Source1) {}
};

class InstX86Mulps
    : public InstX86BaseBinopXmm<InstX86Base::Mulps, true,
                                 InstX86Base::SseSuffix::Packed> {
public:
  static InstX86Mulps *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Mulps>())
        InstX86Mulps(Func, Dest, Source);
  }

private:
  InstX86Mulps(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Mulps, true,
                            InstX86Base::SseSuffix::Packed>(Func, Dest,
                                                            Source) {}
};

class InstX86Mulss
    : public InstX86BaseBinopXmm<InstX86Base::Mulss, false,
                                 InstX86Base::SseSuffix::Scalar> {
public:
  static InstX86Mulss *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Mulss>())
        InstX86Mulss(Func, Dest, Source);
  }

private:
  InstX86Mulss(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Mulss, false,
                            InstX86Base::SseSuffix::Scalar>(Func, Dest,
                                                            Source) {}
};

class InstX86Pmull
    : public InstX86BaseBinopXmm<InstX86Base::Pmull, true,
                                 InstX86Base::SseSuffix::Integral> {
public:
  static InstX86Pmull *create(Cfg *Func, Variable *Dest, Operand *Source) {
    bool TypesAreValid =
        Dest->getType() == IceType_v4i32 || Dest->getType() == IceType_v8i16;
    bool InstructionSetIsValid =
        Dest->getType() == IceType_v8i16 || getInstructionSet(Func) >= SSE4_1;
    (void)TypesAreValid;
    (void)InstructionSetIsValid;
    assert(TypesAreValid);
    assert(InstructionSetIsValid);
    return new (Func->allocate<InstX86Pmull>())
        InstX86Pmull(Func, Dest, Source);
  }

private:
  InstX86Pmull(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Pmull, true,
                            InstX86Base::SseSuffix::Integral>(Func, Dest,
                                                              Source) {}
};

class InstX86Pmulhw : public InstX86BaseBinopXmm<InstX86Base::Pmulhw, false,
                                                 InstX86Base::SseSuffix::None> {
public:
  static InstX86Pmulhw *create(Cfg *Func, Variable *Dest, Operand *Source) {
    assert(Dest->getType() == IceType_v8i16 &&
           Source->getType() == IceType_v8i16);
    return new (Func->allocate<InstX86Pmulhw>())
        InstX86Pmulhw(Func, Dest, Source);
  }

private:
  InstX86Pmulhw(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Pmulhw, false,
                            InstX86Base::SseSuffix::None>(Func, Dest, Source) {}
};

class InstX86Pmulhuw
    : public InstX86BaseBinopXmm<InstX86Base::Pmulhuw, false,
                                 InstX86Base::SseSuffix::None> {
public:
  static InstX86Pmulhuw *create(Cfg *Func, Variable *Dest, Operand *Source) {
    assert(Dest->getType() == IceType_v8i16 &&
           Source->getType() == IceType_v8i16);
    return new (Func->allocate<InstX86Pmulhuw>())
        InstX86Pmulhuw(Func, Dest, Source);
  }

private:
  InstX86Pmulhuw(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Pmulhuw, false,
                            InstX86Base::SseSuffix::None>(Func, Dest, Source) {}
};

class InstX86Pmaddwd
    : public InstX86BaseBinopXmm<InstX86Base::Pmaddwd, false,
                                 InstX86Base::SseSuffix::None> {
public:
  static InstX86Pmaddwd *create(Cfg *Func, Variable *Dest, Operand *Source) {
    assert(Dest->getType() == IceType_v8i16 &&
           Source->getType() == IceType_v8i16);
    return new (Func->allocate<InstX86Pmaddwd>())
        InstX86Pmaddwd(Func, Dest, Source);
  }

private:
  InstX86Pmaddwd(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Pmaddwd, false,
                            InstX86Base::SseSuffix::None>(Func, Dest, Source) {}
};

class InstX86Pmuludq
    : public InstX86BaseBinopXmm<InstX86Base::Pmuludq, false,
                                 InstX86Base::SseSuffix::None> {
public:
  static InstX86Pmuludq *create(Cfg *Func, Variable *Dest, Operand *Source) {
    assert(Dest->getType() == IceType_v4i32 &&
           Source->getType() == IceType_v4i32);
    return new (Func->allocate<InstX86Pmuludq>())
        InstX86Pmuludq(Func, Dest, Source);
  }

private:
  InstX86Pmuludq(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Pmuludq, false,
                            InstX86Base::SseSuffix::None>(Func, Dest, Source) {}
};

class InstX86Divps
    : public InstX86BaseBinopXmm<InstX86Base::Divps, true,
                                 InstX86Base::SseSuffix::Packed> {
public:
  static InstX86Divps *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Divps>())
        InstX86Divps(Func, Dest, Source);
  }

private:
  InstX86Divps(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Divps, true,
                            InstX86Base::SseSuffix::Packed>(Func, Dest,
                                                            Source) {}
};

class InstX86Divss
    : public InstX86BaseBinopXmm<InstX86Base::Divss, false,
                                 InstX86Base::SseSuffix::Scalar> {
public:
  static InstX86Divss *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Divss>())
        InstX86Divss(Func, Dest, Source);
  }

private:
  InstX86Divss(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Divss, false,
                            InstX86Base::SseSuffix::Scalar>(Func, Dest,
                                                            Source) {}
};

class InstX86Rol : public InstX86BaseBinopGPRShift<InstX86Base::Rol> {
public:
  static InstX86Rol *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Rol>()) InstX86Rol(Func, Dest, Source);
  }

private:
  InstX86Rol(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopGPRShift<InstX86Base::Rol>(Func, Dest, Source) {}
};

class InstX86Shl : public InstX86BaseBinopGPRShift<InstX86Base::Shl> {
public:
  static InstX86Shl *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Shl>()) InstX86Shl(Func, Dest, Source);
  }

private:
  InstX86Shl(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopGPRShift<InstX86Base::Shl>(Func, Dest, Source) {}
};

class InstX86Psll : public InstX86BaseBinopXmmShift<InstX86Base::Psll> {
public:
  static InstX86Psll *create(Cfg *Func, Variable *Dest, Operand *Source) {
    assert(Dest->getType() == IceType_v8i16 ||
           Dest->getType() == IceType_v8i1 ||
           Dest->getType() == IceType_v4i32 || Dest->getType() == IceType_v4i1);
    return new (Func->allocate<InstX86Psll>()) InstX86Psll(Func, Dest, Source);
  }

private:
  InstX86Psll(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmmShift<InstX86Base::Psll>(Func, Dest, Source) {}
};

class InstX86Psrl : public InstX86BaseBinopXmmShift<InstX86Base::Psrl, true> {
public:
  static InstX86Psrl *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Psrl>()) InstX86Psrl(Func, Dest, Source);
  }

private:
  InstX86Psrl(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmmShift<InstX86Base::Psrl, true>(Func, Dest, Source) {}
};

class InstX86Shr : public InstX86BaseBinopGPRShift<InstX86Base::Shr> {
public:
  static InstX86Shr *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Shr>()) InstX86Shr(Func, Dest, Source);
  }

private:
  InstX86Shr(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopGPRShift<InstX86Base::Shr>(Func, Dest, Source) {}
};

class InstX86Sar : public InstX86BaseBinopGPRShift<InstX86Base::Sar> {
public:
  static InstX86Sar *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Sar>()) InstX86Sar(Func, Dest, Source);
  }

private:
  InstX86Sar(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopGPRShift<InstX86Base::Sar>(Func, Dest, Source) {}
};

class InstX86Psra : public InstX86BaseBinopXmmShift<InstX86Base::Psra> {
public:
  static InstX86Psra *create(Cfg *Func, Variable *Dest, Operand *Source) {
    assert(Dest->getType() == IceType_v8i16 ||
           Dest->getType() == IceType_v8i1 ||
           Dest->getType() == IceType_v4i32 || Dest->getType() == IceType_v4i1);
    return new (Func->allocate<InstX86Psra>()) InstX86Psra(Func, Dest, Source);
  }

private:
  InstX86Psra(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmmShift<InstX86Base::Psra>(Func, Dest, Source) {}
};

class InstX86Pcmpeq
    : public InstX86BaseBinopXmm<InstX86Base::Pcmpeq, true,
                                 InstX86Base::SseSuffix::Integral> {
public:
  static InstX86Pcmpeq *create(Cfg *Func, Variable *Dest, Operand *Source,
                               Type ArithmeticTypeOverride = IceType_void) {
    const Type Ty = ArithmeticTypeOverride == IceType_void
                        ? Dest->getType()
                        : ArithmeticTypeOverride;
    (void)Ty;
    assert((Ty != IceType_f64 && Ty != IceType_i64) ||
           getInstructionSet(Func) >= SSE4_1);
    return new (Func->allocate<InstX86Pcmpeq>())
        InstX86Pcmpeq(Func, Dest, Source, ArithmeticTypeOverride);
  }

private:
  InstX86Pcmpeq(Cfg *Func, Variable *Dest, Operand *Source,
                Type ArithmeticTypeOverride)
      : InstX86BaseBinopXmm<InstX86Base::Pcmpeq, true,
                            InstX86Base::SseSuffix::Integral>(
            Func, Dest, Source, ArithmeticTypeOverride) {}
};

class InstX86Pcmpgt
    : public InstX86BaseBinopXmm<InstX86Base::Pcmpgt, true,
                                 InstX86Base::SseSuffix::Integral> {
public:
  static InstX86Pcmpgt *create(Cfg *Func, Variable *Dest, Operand *Source) {
    assert(Dest->getType() != IceType_f64 || getInstructionSet(Func) >= SSE4_1);
    return new (Func->allocate<InstX86Pcmpgt>())
        InstX86Pcmpgt(Func, Dest, Source);
  }

private:
  InstX86Pcmpgt(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Pcmpgt, true,
                            InstX86Base::SseSuffix::Integral>(Func, Dest,
                                                              Source) {}
};

/// movss is only a binary operation when the source and dest operands are
/// both registers (the high bits of dest are left untouched). In other cases,
/// it behaves like a copy (mov-like) operation (and the high bits of dest are
/// cleared). InstX86Movss will assert that both its source and dest operands
/// are registers, so the lowering code should use _mov instead of _movss in
/// cases where a copy operation is intended.
class InstX86MovssRegs
    : public InstX86BaseBinopXmm<InstX86Base::MovssRegs, false,
                                 InstX86Base::SseSuffix::None> {
public:
  static InstX86MovssRegs *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86MovssRegs>())
        InstX86MovssRegs(Func, Dest, Source);
  }

  void emitIAS(const Cfg *Func) const override;

private:
  InstX86MovssRegs(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::MovssRegs, false,
                            InstX86Base::SseSuffix::None>(Func, Dest, Source) {}
};

class InstX86Idiv : public InstX86BaseTernop<InstX86Base::Idiv> {
public:
  static InstX86Idiv *create(Cfg *Func, Variable *Dest, Operand *Source1,
                             Operand *Source2) {
    return new (Func->allocate<InstX86Idiv>())
        InstX86Idiv(Func, Dest, Source1, Source2);
  }

  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Idiv(Cfg *Func, Variable *Dest, Operand *Source1, Operand *Source2)
      : InstX86BaseTernop<InstX86Base::Idiv>(Func, Dest, Source1, Source2) {}
};

class InstX86Div : public InstX86BaseTernop<InstX86Base::Div> {
public:
  static InstX86Div *create(Cfg *Func, Variable *Dest, Operand *Source1,
                            Operand *Source2) {
    return new (Func->allocate<InstX86Div>())
        InstX86Div(Func, Dest, Source1, Source2);
  }

  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Div(Cfg *Func, Variable *Dest, Operand *Source1, Operand *Source2)
      : InstX86BaseTernop<InstX86Base::Div>(Func, Dest, Source1, Source2) {}
};

class InstX86Insertps : public InstX86BaseTernop<InstX86Base::Insertps> {
public:
  static InstX86Insertps *create(Cfg *Func, Variable *Dest, Operand *Source1,
                                 Operand *Source2) {
    return new (Func->allocate<InstX86Insertps>())
        InstX86Insertps(Func, Dest, Source1, Source2);
  }

  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Insertps(Cfg *Func, Variable *Dest, Operand *Source1, Operand *Source2)
      : InstX86BaseTernop<InstX86Base::Insertps>(Func, Dest, Source1, Source2) {
  }
};

class InstX86Pinsr : public InstX86BaseTernop<InstX86Base::Pinsr> {
public:
  static InstX86Pinsr *create(Cfg *Func, Variable *Dest, Operand *Source1,
                              Operand *Source2) {
    // pinsrb and pinsrd are SSE4.1 instructions.
    assert(Dest->getType() == IceType_v8i16 ||
           Dest->getType() == IceType_v8i1 ||
           getInstructionSet(Func) >= SSE4_1);
    return new (Func->allocate<InstX86Pinsr>())
        InstX86Pinsr(Func, Dest, Source1, Source2);
  }

  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Pinsr(Cfg *Func, Variable *Dest, Operand *Source1, Operand *Source2)
      : InstX86BaseTernop<InstX86Base::Pinsr>(Func, Dest, Source1, Source2) {}
};

class InstX86Shufps : public InstX86BaseTernop<InstX86Base::Shufps> {
public:
  static InstX86Shufps *create(Cfg *Func, Variable *Dest, Operand *Source1,
                               Operand *Source2) {
    return new (Func->allocate<InstX86Shufps>())
        InstX86Shufps(Func, Dest, Source1, Source2);
  }

  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Shufps(Cfg *Func, Variable *Dest, Operand *Source1, Operand *Source2)
      : InstX86BaseTernop<InstX86Base::Shufps>(Func, Dest, Source1, Source2) {}
};

class InstX86Blendvps : public InstX86BaseTernop<InstX86Base::Blendvps> {
public:
  static InstX86Blendvps *create(Cfg *Func, Variable *Dest, Operand *Source1,
                                 Operand *Source2) {
    assert(getInstructionSet(Func) >= SSE4_1);
    return new (Func->allocate<InstX86Blendvps>())
        InstX86Blendvps(Func, Dest, Source1, Source2);
  }

  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Fund) const override;

private:
  InstX86Blendvps(Cfg *Func, Variable *Dest, Operand *Source1, Operand *Source2)
      : InstX86BaseTernop<InstX86Base::Blendvps>(Func, Dest, Source1, Source2) {
  }
};

class InstX86Pblendvb : public InstX86BaseTernop<InstX86Base::Pblendvb> {
public:
  static InstX86Pblendvb *create(Cfg *Func, Variable *Dest, Operand *Source1,
                                 Operand *Source2) {
    assert(getInstructionSet(Func) >= SSE4_1);
    return new (Func->allocate<InstX86Pblendvb>())
        InstX86Pblendvb(Func, Dest, Source1, Source2);
  }

  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Pblendvb(Cfg *Func, Variable *Dest, Operand *Source1, Operand *Source2)
      : InstX86BaseTernop<InstX86Base::Pblendvb>(Func, Dest, Source1, Source2) {
  }
};

class InstX86Pextr : public InstX86BaseThreeAddressop<InstX86Base::Pextr> {
public:
  static InstX86Pextr *create(Cfg *Func, Variable *Dest, Operand *Source0,
                              Operand *Source1) {
    assert(Source0->getType() == IceType_v8i16 ||
           Source0->getType() == IceType_v8i1 ||
           getInstructionSet(Func) >= SSE4_1);
    return new (Func->allocate<InstX86Pextr>())
        InstX86Pextr(Func, Dest, Source0, Source1);
  }

  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Pextr(Cfg *Func, Variable *Dest, Operand *Source0, Operand *Source1)
      : InstX86BaseThreeAddressop<InstX86Base::Pextr>(Func, Dest, Source0,
                                                      Source1) {}
};

class InstX86Pshufd : public InstX86BaseThreeAddressop<InstX86Base::Pshufd> {
public:
  static InstX86Pshufd *create(Cfg *Func, Variable *Dest, Operand *Source0,
                               Operand *Source1) {
    return new (Func->allocate<InstX86Pshufd>())
        InstX86Pshufd(Func, Dest, Source0, Source1);
  }

  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Pshufd(Cfg *Func, Variable *Dest, Operand *Source0, Operand *Source1)
      : InstX86BaseThreeAddressop<InstX86Base::Pshufd>(Func, Dest, Source0,
                                                       Source1) {}
};

/// Base class for a lockable x86-64 instruction (emits a locked prefix).
class InstX86BaseLockable : public InstX86Base {
  InstX86BaseLockable() = delete;
  InstX86BaseLockable(const InstX86BaseLockable &) = delete;
  InstX86BaseLockable &operator=(const InstX86BaseLockable &) = delete;

protected:
  bool Locked;

  InstX86BaseLockable(Cfg *Func, typename InstX86Base::InstKindX86 Kind,
                      SizeT Maxsrcs, Variable *Dest, bool Locked)
      : InstX86Base(Func, Kind, Maxsrcs, Dest), Locked(Locked) {
    // Assume that such instructions are used for Atomics and be careful with
    // optimizations.
    this->HasSideEffects = Locked;
  }
};

/// Mul instruction - unsigned multiply.
class InstX86Mul final : public InstX86Base {
  InstX86Mul() = delete;
  InstX86Mul(const InstX86Mul &) = delete;
  InstX86Mul &operator=(const InstX86Mul &) = delete;

public:
  static InstX86Mul *create(Cfg *Func, Variable *Dest, Variable *Source1,
                            Operand *Source2) {
    return new (Func->allocate<InstX86Mul>())
        InstX86Mul(Func, Dest, Source1, Source2);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Mul);
  }

private:
  InstX86Mul(Cfg *Func, Variable *Dest, Variable *Source1, Operand *Source2);
};

/// Shld instruction - shift across a pair of operands.
class InstX86Shld final : public InstX86Base {
  InstX86Shld() = delete;
  InstX86Shld(const InstX86Shld &) = delete;
  InstX86Shld &operator=(const InstX86Shld &) = delete;

public:
  static InstX86Shld *create(Cfg *Func, Variable *Dest, Variable *Source1,
                             Operand *Source2) {
    return new (Func->allocate<InstX86Shld>())
        InstX86Shld(Func, Dest, Source1, Source2);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Shld);
  }

private:
  InstX86Shld(Cfg *Func, Variable *Dest, Variable *Source1, Operand *Source2);
};

/// Shrd instruction - shift across a pair of operands.
class InstX86Shrd final : public InstX86Base {
  InstX86Shrd() = delete;
  InstX86Shrd(const InstX86Shrd &) = delete;
  InstX86Shrd &operator=(const InstX86Shrd &) = delete;

public:
  static InstX86Shrd *create(Cfg *Func, Variable *Dest, Variable *Source1,
                             Operand *Source2) {
    return new (Func->allocate<InstX86Shrd>())
        InstX86Shrd(Func, Dest, Source1, Source2);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Shrd);
  }

private:
  InstX86Shrd(Cfg *Func, Variable *Dest, Variable *Source1, Operand *Source2);
};

/// Conditional move instruction.
class InstX86Cmov final : public InstX86Base {
  InstX86Cmov() = delete;
  InstX86Cmov(const InstX86Cmov &) = delete;
  InstX86Cmov &operator=(const InstX86Cmov &) = delete;

public:
  static InstX86Cmov *create(Cfg *Func, Variable *Dest, Operand *Source,
                             BrCond Cond) {
    return new (Func->allocate<InstX86Cmov>())
        InstX86Cmov(Func, Dest, Source, Cond);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Cmov);
  }

private:
  InstX86Cmov(Cfg *Func, Variable *Dest, Operand *Source, BrCond Cond);

  BrCond Condition;
};

/// Cmpps instruction - compare packed singled-precision floating point values
class InstX86Cmpps final : public InstX86Base {
  InstX86Cmpps() = delete;
  InstX86Cmpps(const InstX86Cmpps &) = delete;
  InstX86Cmpps &operator=(const InstX86Cmpps &) = delete;

public:
  static InstX86Cmpps *create(Cfg *Func, Variable *Dest, Operand *Source,
                              CmppsCond Condition) {
    return new (Func->allocate<InstX86Cmpps>())
        InstX86Cmpps(Func, Dest, Source, Condition);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Cmpps);
  }

private:
  InstX86Cmpps(Cfg *Func, Variable *Dest, Operand *Source, CmppsCond Cond);

  CmppsCond Condition;
};

/// Cmpxchg instruction - cmpxchg <dest>, <desired> will compare if <dest>
/// equals eax. If so, the ZF is set and <desired> is stored in <dest>. If
/// not, ZF is cleared and <dest> is copied to eax (or subregister). <dest>
/// can be a register or memory, while <desired> must be a register. It is
/// the user's responsibility to mark eax with a FakeDef.
class InstX86Cmpxchg final : public InstX86BaseLockable {
  InstX86Cmpxchg() = delete;
  InstX86Cmpxchg(const InstX86Cmpxchg &) = delete;
  InstX86Cmpxchg &operator=(const InstX86Cmpxchg &) = delete;

public:
  static InstX86Cmpxchg *create(Cfg *Func, Operand *DestOrAddr, Variable *Eax,
                                Variable *Desired, bool Locked) {
    return new (Func->allocate<InstX86Cmpxchg>())
        InstX86Cmpxchg(Func, DestOrAddr, Eax, Desired, Locked);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Cmpxchg);
  }

private:
  InstX86Cmpxchg(Cfg *Func, Operand *DestOrAddr, Variable *Eax,
                 Variable *Desired, bool Locked);
};

/// Cmpxchg8b instruction - cmpxchg8b <m64> will compare if <m64> equals
/// edx:eax. If so, the ZF is set and ecx:ebx is stored in <m64>. If not, ZF
/// is cleared and <m64> is copied to edx:eax. The caller is responsible for
/// inserting FakeDefs to mark edx and eax as modified. <m64> must be a memory
/// operand.
class InstX86Cmpxchg8b final : public InstX86BaseLockable {
  InstX86Cmpxchg8b() = delete;
  InstX86Cmpxchg8b(const InstX86Cmpxchg8b &) = delete;
  InstX86Cmpxchg8b &operator=(const InstX86Cmpxchg8b &) = delete;

public:
  static InstX86Cmpxchg8b *create(Cfg *Func, X86OperandMem *Dest, Variable *Edx,
                                  Variable *Eax, Variable *Ecx, Variable *Ebx,
                                  bool Locked) {
    return new (Func->allocate<InstX86Cmpxchg8b>())
        InstX86Cmpxchg8b(Func, Dest, Edx, Eax, Ecx, Ebx, Locked);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Cmpxchg8b);
  }

private:
  InstX86Cmpxchg8b(Cfg *Func, X86OperandMem *Dest, Variable *Edx, Variable *Eax,
                   Variable *Ecx, Variable *Ebx, bool Locked);
};

/// Cvt instruction - wrapper for cvtsX2sY where X and Y are in {s,d,i} as
/// appropriate.  s=float, d=double, i=int. X and Y are determined from
/// dest/src types. Sign and zero extension on the integer operand needs to be
/// done separately.
class InstX86Cvt final : public InstX86Base {
  InstX86Cvt() = delete;
  InstX86Cvt(const InstX86Cvt &) = delete;
  InstX86Cvt &operator=(const InstX86Cvt &) = delete;

public:
  enum CvtVariant { Si2ss, Tss2si, Ss2si, Float2float, Dq2ps, Tps2dq, Ps2dq };
  static InstX86Cvt *create(Cfg *Func, Variable *Dest, Operand *Source,
                            CvtVariant Variant) {
    return new (Func->allocate<InstX86Cvt>())
        InstX86Cvt(Func, Dest, Source, Variant);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Cvt);
  }
  bool isTruncating() const { return Variant == Tss2si || Variant == Tps2dq; }

private:
  CvtVariant Variant;
  InstX86Cvt(Cfg *Func, Variable *Dest, Operand *Source, CvtVariant Variant);
};

/// Round instruction
class InstX86Round final
    : public InstX86BaseThreeAddressop<InstX86Base::Round> {
public:
  static InstX86Round *create(Cfg *Func, Variable *Dest, Operand *Source,
                              Operand *Imm) {
    return new (Func->allocate<InstX86Round>())
        InstX86Round(Func, Dest, Source, Imm);
  }

  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;

private:
  InstX86Round(Cfg *Func, Variable *Dest, Operand *Source, Operand *Imm)
      : InstX86BaseThreeAddressop<InstX86Base::Round>(Func, Dest, Source, Imm) {
  }
};

/// cmp - Integer compare instruction.
class InstX86Icmp final : public InstX86Base {
  InstX86Icmp() = delete;
  InstX86Icmp(const InstX86Icmp &) = delete;
  InstX86Icmp &operator=(const InstX86Icmp &) = delete;

public:
  static InstX86Icmp *create(Cfg *Func, Operand *Src1, Operand *Src2) {
    return new (Func->allocate<InstX86Icmp>()) InstX86Icmp(Func, Src1, Src2);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Icmp);
  }

private:
  InstX86Icmp(Cfg *Func, Operand *Src1, Operand *Src2);
};

/// ucomiss/ucomisd - floating-point compare instruction.
class InstX86Ucomiss final : public InstX86Base {
  InstX86Ucomiss() = delete;
  InstX86Ucomiss(const InstX86Ucomiss &) = delete;
  InstX86Ucomiss &operator=(const InstX86Ucomiss &) = delete;

public:
  static InstX86Ucomiss *create(Cfg *Func, Operand *Src1, Operand *Src2) {
    return new (Func->allocate<InstX86Ucomiss>())
        InstX86Ucomiss(Func, Src1, Src2);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Ucomiss);
  }

private:
  InstX86Ucomiss(Cfg *Func, Operand *Src1, Operand *Src2);
};

/// UD2 instruction.
class InstX86UD2 final : public InstX86Base {
  InstX86UD2() = delete;
  InstX86UD2(const InstX86UD2 &) = delete;
  InstX86UD2 &operator=(const InstX86UD2 &) = delete;

public:
  static InstX86UD2 *create(Cfg *Func) {
    return new (Func->allocate<InstX86UD2>()) InstX86UD2(Func);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::UD2);
  }

private:
  explicit InstX86UD2(Cfg *Func);
};

/// Int3 instruction.
class InstX86Int3 final : public InstX86Base {
  InstX86Int3() = delete;
  InstX86Int3(const InstX86Int3 &) = delete;
  InstX86Int3 &operator=(const InstX86Int3 &) = delete;

public:
  static InstX86Int3 *create(Cfg *Func) {
    return new (Func->allocate<InstX86Int3>()) InstX86Int3(Func);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Int3);
  }

private:
  explicit InstX86Int3(Cfg *Func);
};

/// Test instruction.
class InstX86Test final : public InstX86Base {
  InstX86Test() = delete;
  InstX86Test(const InstX86Test &) = delete;
  InstX86Test &operator=(const InstX86Test &) = delete;

public:
  static InstX86Test *create(Cfg *Func, Operand *Source1, Operand *Source2) {
    return new (Func->allocate<InstX86Test>())
        InstX86Test(Func, Source1, Source2);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Test);
  }

private:
  InstX86Test(Cfg *Func, Operand *Source1, Operand *Source2);
};

/// Mfence instruction.
class InstX86Mfence final : public InstX86Base {
  InstX86Mfence() = delete;
  InstX86Mfence(const InstX86Mfence &) = delete;
  InstX86Mfence &operator=(const InstX86Mfence &) = delete;

public:
  static InstX86Mfence *create(Cfg *Func) {
    return new (Func->allocate<InstX86Mfence>()) InstX86Mfence(Func);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Mfence);
  }

private:
  explicit InstX86Mfence(Cfg *Func);
};

/// This is essentially a "mov" instruction with anX86OperandMem operand
/// instead of Variable as the destination. It's important for liveness that
/// there is no Dest operand.
class InstX86Store final : public InstX86Base {
  InstX86Store() = delete;
  InstX86Store(const InstX86Store &) = delete;
  InstX86Store &operator=(const InstX86Store &) = delete;

public:
  static InstX86Store *create(Cfg *Func, Operand *Value, X86Operand *Mem) {
    return new (Func->allocate<InstX86Store>()) InstX86Store(Func, Value, Mem);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Store);
  }

private:
  InstX86Store(Cfg *Func, Operand *Value, X86Operand *Mem);
};

/// This is essentially a vector "mov" instruction with an X86OperandMem operand
/// instead of Variable as the destination. It's important for liveness that
/// there is no Dest operand. The source must be an Xmm register, since Dest is
/// mem.
class InstX86StoreP final : public InstX86Base {
  InstX86StoreP() = delete;
  InstX86StoreP(const InstX86StoreP &) = delete;
  InstX86StoreP &operator=(const InstX86StoreP &) = delete;

public:
  static InstX86StoreP *create(Cfg *Func, Variable *Value, X86OperandMem *Mem) {
    return new (Func->allocate<InstX86StoreP>())
        InstX86StoreP(Func, Value, Mem);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::StoreP);
  }

private:
  InstX86StoreP(Cfg *Func, Variable *Value, X86OperandMem *Mem);
};

class InstX86StoreQ final : public InstX86Base {
  InstX86StoreQ() = delete;
  InstX86StoreQ(const InstX86StoreQ &) = delete;
  InstX86StoreQ &operator=(const InstX86StoreQ &) = delete;

public:
  static InstX86StoreQ *create(Cfg *Func, Operand *Value, X86OperandMem *Mem) {
    return new (Func->allocate<InstX86StoreQ>())
        InstX86StoreQ(Func, Value, Mem);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::StoreQ);
  }

private:
  InstX86StoreQ(Cfg *Func, Operand *Value, X86OperandMem *Mem);
};

class InstX86StoreD final : public InstX86Base {
  InstX86StoreD() = delete;
  InstX86StoreD(const InstX86StoreD &) = delete;
  InstX86StoreD &operator=(const InstX86StoreD &) = delete;

public:
  static InstX86StoreD *create(Cfg *Func, Operand *Value, X86OperandMem *Mem) {
    return new (Func->allocate<InstX86StoreD>())
        InstX86StoreD(Func, Value, Mem);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::StoreQ);
  }

private:
  InstX86StoreD(Cfg *Func, Operand *Value, X86OperandMem *Mem);
};

/// Nop instructions of varying length
class InstX86Nop final : public InstX86Base {
  InstX86Nop() = delete;
  InstX86Nop(const InstX86Nop &) = delete;
  InstX86Nop &operator=(const InstX86Nop &) = delete;

public:
  // TODO: Replace with enum.
  using NopVariant = unsigned;

  static InstX86Nop *create(Cfg *Func, NopVariant Variant) {
    return new (Func->allocate<InstX86Nop>()) InstX86Nop(Func, Variant);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Nop);
  }

private:
  InstX86Nop(Cfg *Func, NopVariant Length);

  NopVariant Variant;
};

class InstX86Pop final : public InstX86Base {
  InstX86Pop() = delete;
  InstX86Pop(const InstX86Pop &) = delete;
  InstX86Pop &operator=(const InstX86Pop &) = delete;

public:
  static InstX86Pop *create(Cfg *Func, Variable *Dest) {
    return new (Func->allocate<InstX86Pop>()) InstX86Pop(Func, Dest);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Pop);
  }

private:
  InstX86Pop(Cfg *Func, Variable *Dest);
};

class InstX86Push final : public InstX86Base {
  InstX86Push() = delete;
  InstX86Push(const InstX86Push &) = delete;
  InstX86Push &operator=(const InstX86Push &) = delete;

public:
  static InstX86Push *create(Cfg *Func, Operand *Source) {
    return new (Func->allocate<InstX86Push>()) InstX86Push(Func, Source);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Push);
  }

private:
  InstX86Push(Cfg *Func, Operand *Source);
};

/// Ret instruction. Currently only supports the "ret" version that does not
/// pop arguments. This instruction takes a Source operand (for non-void
/// returning functions) for liveness analysis, though a FakeUse before the
/// ret would do just as well.
class InstX86Ret final : public InstX86Base {
  InstX86Ret() = delete;
  InstX86Ret(const InstX86Ret &) = delete;
  InstX86Ret &operator=(const InstX86Ret &) = delete;

public:
  static InstX86Ret *create(Cfg *Func, Variable *Source = nullptr) {
    return new (Func->allocate<InstX86Ret>()) InstX86Ret(Func, Source);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Ret);
  }

private:
  InstX86Ret(Cfg *Func, Variable *Source);
};

/// Conditional set-byte instruction.
class InstX86Setcc final : public InstX86Base {
  InstX86Setcc() = delete;
  InstX86Setcc(const InstX86Cmov &) = delete;
  InstX86Setcc &operator=(const InstX86Setcc &) = delete;

public:
  static InstX86Setcc *create(Cfg *Func, Variable *Dest, BrCond Cond) {
    return new (Func->allocate<InstX86Setcc>()) InstX86Setcc(Func, Dest, Cond);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Setcc);
  }

private:
  InstX86Setcc(Cfg *Func, Variable *Dest, BrCond Cond);

  const BrCond Condition;
};

/// Exchanging Add instruction. Exchanges the first operand (destination
/// operand) with the second operand (source operand), then loads the sum of
/// the two values into the destination operand. The destination may be a
/// register or memory, while the source must be a register.
///
/// Both the dest and source are updated. The caller should then insert a
/// FakeDef to reflect the second udpate.
class InstX86Xadd final : public InstX86BaseLockable {
  InstX86Xadd() = delete;
  InstX86Xadd(const InstX86Xadd &) = delete;
  InstX86Xadd &operator=(const InstX86Xadd &) = delete;

public:
  static InstX86Xadd *create(Cfg *Func, Operand *Dest, Variable *Source,
                             bool Locked) {
    return new (Func->allocate<InstX86Xadd>())
        InstX86Xadd(Func, Dest, Source, Locked);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Xadd);
  }

private:
  InstX86Xadd(Cfg *Func, Operand *Dest, Variable *Source, bool Locked);
};

/// Exchange instruction. Exchanges the first operand (destination operand)
/// with the second operand (source operand). At least one of the operands
/// must be a register (and the other can be reg or mem). Both the Dest and
/// Source are updated. If there is a memory operand, then the instruction is
/// automatically "locked" without the need for a lock prefix.
class InstX86Xchg final : public InstX86Base {
  InstX86Xchg() = delete;
  InstX86Xchg(const InstX86Xchg &) = delete;
  InstX86Xchg &operator=(const InstX86Xchg &) = delete;

public:
  static InstX86Xchg *create(Cfg *Func, Operand *Dest, Variable *Source) {
    return new (Func->allocate<InstX86Xchg>()) InstX86Xchg(Func, Dest, Source);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::Xchg);
  }

private:
  InstX86Xchg(Cfg *Func, Operand *Dest, Variable *Source);
};

/// Start marker for the Intel Architecture Code Analyzer. This is not an
/// executable instruction and must only be used for analysis.
class InstX86IacaStart final : public InstX86Base {
  InstX86IacaStart() = delete;
  InstX86IacaStart(const InstX86IacaStart &) = delete;
  InstX86IacaStart &operator=(const InstX86IacaStart &) = delete;

public:
  static InstX86IacaStart *create(Cfg *Func) {
    return new (Func->allocate<InstX86IacaStart>()) InstX86IacaStart(Func);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::IacaStart);
  }

private:
  InstX86IacaStart(Cfg *Func);
};

/// End marker for the Intel Architecture Code Analyzer. This is not an
/// executable instruction and must only be used for analysis.
class InstX86IacaEnd final : public InstX86Base {
  InstX86IacaEnd() = delete;
  InstX86IacaEnd(const InstX86IacaEnd &) = delete;
  InstX86IacaEnd &operator=(const InstX86IacaEnd &) = delete;

public:
  static InstX86IacaEnd *create(Cfg *Func) {
    return new (Func->allocate<InstX86IacaEnd>()) InstX86IacaEnd(Func);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) {
    return InstX86Base::isClassof(Instr, InstX86Base::IacaEnd);
  }

private:
  InstX86IacaEnd(Cfg *Func);
};

class InstX86Pshufb : public InstX86BaseBinopXmm<InstX86Base::Pshufb, false,
                                                 InstX86Base::SseSuffix::None> {
public:
  static InstX86Pshufb *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Pshufb>())
        InstX86Pshufb(Func, Dest, Source);
  }

private:
  InstX86Pshufb(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Pshufb, false,
                            InstX86Base::SseSuffix::None>(Func, Dest, Source) {}
};

class InstX86Punpckl
    : public InstX86BaseBinopXmm<InstX86Base::Punpckl, false,
                                 InstX86Base::SseSuffix::Unpack> {
public:
  static InstX86Punpckl *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Punpckl>())
        InstX86Punpckl(Func, Dest, Source);
  }

private:
  InstX86Punpckl(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Punpckl, false,
                            InstX86Base::SseSuffix::Unpack>(Func, Dest,
                                                            Source) {}
};

class InstX86Punpckh
    : public InstX86BaseBinopXmm<InstX86Base::Punpckh, false,
                                 InstX86Base::SseSuffix::Unpack> {
public:
  static InstX86Punpckh *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Punpckh>())
        InstX86Punpckh(Func, Dest, Source);
  }

private:
  InstX86Punpckh(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Punpckh, false,
                            InstX86Base::SseSuffix::Unpack>(Func, Dest,
                                                            Source) {}
};

class InstX86Packss : public InstX86BaseBinopXmm<InstX86Base::Packss, false,
                                                 InstX86Base::SseSuffix::Pack> {
public:
  static InstX86Packss *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Packss>())
        InstX86Packss(Func, Dest, Source);
  }

private:
  InstX86Packss(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Packss, false,
                            InstX86Base::SseSuffix::Pack>(Func, Dest, Source) {}
};

class InstX86Packus : public InstX86BaseBinopXmm<InstX86Base::Packus, false,
                                                 InstX86Base::SseSuffix::Pack> {
public:
  static InstX86Packus *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX86Packus>())
        InstX86Packus(Func, Dest, Source);
  }

private:
  InstX86Packus(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX86BaseBinopXmm<InstX86Base::Packus, false,
                            InstX86Base::SseSuffix::Pack>(Func, Dest, Source) {}
};

struct Insts {
  using FakeRMW = InstX86FakeRMW;
  using Label = InstX86Label;

  using Call = InstX86Call;

  using Br = InstX86Br;
  using Jmp = InstX86Jmp;
  using Bswap = InstX86Bswap;
  using Neg = InstX86Neg;
  using Bsf = InstX86Bsf;
  using Bsr = InstX86Bsr;
  using Lea = InstX86Lea;
  using Cbwdq = InstX86Cbwdq;
  using Movsx = InstX86Movsx;
  using Movzx = InstX86Movzx;
  using Movd = InstX86Movd;
  using Movmsk = InstX86Movmsk;
  using Sqrt = InstX86Sqrt;
  using Mov = InstX86Mov;
  using Movp = InstX86Movp;
  using Movq = InstX86Movq;
  using Add = InstX86Add;
  using AddRMW = InstX86AddRMW;
  using Addps = InstX86Addps;
  using Adc = InstX86Adc;
  using AdcRMW = InstX86AdcRMW;
  using Addss = InstX86Addss;
  using Andnps = InstX86Andnps;
  using Andps = InstX86Andps;
  using Padd = InstX86Padd;
  using Padds = InstX86Padds;
  using Paddus = InstX86Paddus;
  using Sub = InstX86Sub;
  using SubRMW = InstX86SubRMW;
  using Subps = InstX86Subps;
  using Subss = InstX86Subss;
  using Sbb = InstX86Sbb;
  using SbbRMW = InstX86SbbRMW;
  using Psub = InstX86Psub;
  using Psubs = InstX86Psubs;
  using Psubus = InstX86Psubus;
  using And = InstX86And;
  using AndRMW = InstX86AndRMW;
  using Pand = InstX86Pand;
  using Pandn = InstX86Pandn;
  using Or = InstX86Or;
  using Orps = InstX86Orps;
  using OrRMW = InstX86OrRMW;
  using Por = InstX86Por;
  using Xor = InstX86Xor;
  using Xorps = InstX86Xorps;
  using XorRMW = InstX86XorRMW;
  using Pxor = InstX86Pxor;
  using Maxss = InstX86Maxss;
  using Minss = InstX86Minss;
  using Maxps = InstX86Maxps;
  using Minps = InstX86Minps;
  using Imul = InstX86Imul;
  using ImulImm = InstX86ImulImm;
  using Mulps = InstX86Mulps;
  using Mulss = InstX86Mulss;
  using Pmull = InstX86Pmull;
  using Pmulhw = InstX86Pmulhw;
  using Pmulhuw = InstX86Pmulhuw;
  using Pmaddwd = InstX86Pmaddwd;
  using Pmuludq = InstX86Pmuludq;
  using Divps = InstX86Divps;
  using Divss = InstX86Divss;
  using Rol = InstX86Rol;
  using Shl = InstX86Shl;
  using Psll = InstX86Psll;
  using Psrl = InstX86Psrl;
  using Shr = InstX86Shr;
  using Sar = InstX86Sar;
  using Psra = InstX86Psra;
  using Pcmpeq = InstX86Pcmpeq;
  using Pcmpgt = InstX86Pcmpgt;
  using MovssRegs = InstX86MovssRegs;
  using Idiv = InstX86Idiv;
  using Div = InstX86Div;
  using Insertps = InstX86Insertps;
  using Pinsr = InstX86Pinsr;
  using Shufps = InstX86Shufps;
  using Blendvps = InstX86Blendvps;
  using Pblendvb = InstX86Pblendvb;
  using Pextr = InstX86Pextr;
  using Pshufd = InstX86Pshufd;
  using Lockable = InstX86BaseLockable;
  using Mul = InstX86Mul;
  using Shld = InstX86Shld;
  using Shrd = InstX86Shrd;
  using Cmov = InstX86Cmov;
  using Cmpps = InstX86Cmpps;
  using Cmpxchg = InstX86Cmpxchg;
  using Cmpxchg8b = InstX86Cmpxchg8b;
  using Cvt = InstX86Cvt;
  using Round = InstX86Round;
  using Icmp = InstX86Icmp;
  using Ucomiss = InstX86Ucomiss;
  using UD2 = InstX86UD2;
  using Int3 = InstX86Int3;
  using Test = InstX86Test;
  using Mfence = InstX86Mfence;
  using Store = InstX86Store;
  using StoreP = InstX86StoreP;
  using StoreQ = InstX86StoreQ;
  using StoreD = InstX86StoreD;
  using Nop = InstX86Nop;
  using Pop = InstX86Pop;
  using Push = InstX86Push;
  using Ret = InstX86Ret;
  using Setcc = InstX86Setcc;
  using Xadd = InstX86Xadd;
  using Xchg = InstX86Xchg;

  using IacaStart = InstX86IacaStart;
  using IacaEnd = InstX86IacaEnd;

  using Pshufb = InstX86Pshufb;
  using Punpckl = InstX86Punpckl;
  using Punpckh = InstX86Punpckh;
  using Packss = InstX86Packss;
  using Packus = InstX86Packus;
};

/// X86 Instructions have static data (particularly, opcodes and instruction
/// emitters). Each X86 target needs to define all of these, so this macro is
/// provided so that, if something changes, then all X86 targets will be updated
/// automatically.
/* In-place ops */
template <> constexpr const char *InstX86Bswap::Base::Opcode = "bswap";
template <> constexpr const char *InstX86Neg::Base::Opcode = "neg";
/* Unary ops */
template <> constexpr const char *InstX86Bsf::Base::Opcode = "bsf";
template <> constexpr const char *InstX86Bsr::Base::Opcode = "bsr";
template <> constexpr const char *InstX86Lea::Base::Opcode = "lea";
template <> constexpr const char *InstX86Movd::Base::Opcode = "movd";
template <> constexpr const char *InstX86Movsx::Base::Opcode = "movs";
template <> constexpr const char *InstX86Movzx::Base::Opcode = "movz";
template <> constexpr const char *InstX86Sqrt::Base::Opcode = "sqrt";
template <> constexpr const char *InstX86Cbwdq::Base::Opcode = "cbw/cwd/cdq";
/* Mov-like ops */
template <> constexpr const char *InstX86Mov::Base::Opcode = "mov";
template <> constexpr const char *InstX86Movp::Base::Opcode = "movups";
template <> constexpr const char *InstX86Movq::Base::Opcode = "movq";
/* Binary ops */
template <> constexpr const char *InstX86Add::Base::Opcode = "add";
template <> constexpr const char *InstX86AddRMW::Base::Opcode = "add";
template <> constexpr const char *InstX86Addps::Base::Opcode = "add";
template <> constexpr const char *InstX86Adc::Base::Opcode = "adc";
template <> constexpr const char *InstX86AdcRMW::Base::Opcode = "adc";
template <> constexpr const char *InstX86Addss::Base::Opcode = "add";
template <> constexpr const char *InstX86Andnps::Base::Opcode = "andn";
template <> constexpr const char *InstX86Andps::Base::Opcode = "and";
template <> constexpr const char *InstX86Maxss::Base::Opcode = "max";
template <> constexpr const char *InstX86Minss::Base::Opcode = "min";
template <> constexpr const char *InstX86Maxps::Base::Opcode = "max";
template <> constexpr const char *InstX86Minps::Base::Opcode = "min";
template <> constexpr const char *InstX86Padd::Base::Opcode = "padd";
template <> constexpr const char *InstX86Padds::Base::Opcode = "padds";
template <> constexpr const char *InstX86Paddus::Base::Opcode = "paddus";
template <> constexpr const char *InstX86Sub::Base::Opcode = "sub";
template <> constexpr const char *InstX86SubRMW::Base::Opcode = "sub";
template <> constexpr const char *InstX86Subps::Base::Opcode = "sub";
template <> constexpr const char *InstX86Subss::Base::Opcode = "sub";
template <> constexpr const char *InstX86Sbb::Base::Opcode = "sbb";
template <> constexpr const char *InstX86SbbRMW::Base::Opcode = "sbb";
template <> constexpr const char *InstX86Psub::Base::Opcode = "psub";
template <> constexpr const char *InstX86Psubs::Base::Opcode = "psubs";
template <> constexpr const char *InstX86Psubus::Base::Opcode = "psubus";
template <> constexpr const char *InstX86And::Base::Opcode = "and";
template <> constexpr const char *InstX86AndRMW::Base::Opcode = "and";
template <> constexpr const char *InstX86Pand::Base::Opcode = "pand";
template <> constexpr const char *InstX86Pandn::Base::Opcode = "pandn";
template <> constexpr const char *InstX86Or::Base::Opcode = "or";
template <> constexpr const char *InstX86Orps::Base::Opcode = "or";
template <> constexpr const char *InstX86OrRMW::Base::Opcode = "or";
template <> constexpr const char *InstX86Por::Base::Opcode = "por";
template <> constexpr const char *InstX86Xor::Base::Opcode = "xor";
template <> constexpr const char *InstX86Xorps::Base::Opcode = "xor";
template <> constexpr const char *InstX86XorRMW::Base::Opcode = "xor";
template <> constexpr const char *InstX86Pxor::Base::Opcode = "pxor";
template <> constexpr const char *InstX86Imul::Base::Opcode = "imul";
template <> constexpr const char *InstX86ImulImm::Base::Opcode = "imul";
template <> constexpr const char *InstX86Mulps::Base::Opcode = "mul";
template <> constexpr const char *InstX86Mulss::Base::Opcode = "mul";
template <> constexpr const char *InstX86Pmull::Base::Opcode = "pmull";
template <> constexpr const char *InstX86Pmulhw::Base::Opcode = "pmulhw";
template <> constexpr const char *InstX86Pmulhuw::Base::Opcode = "pmulhuw";
template <> constexpr const char *InstX86Pmaddwd::Base::Opcode = "pmaddwd";
template <> constexpr const char *InstX86Pmuludq::Base::Opcode = "pmuludq";
template <> constexpr const char *InstX86Div::Base::Opcode = "div";
template <> constexpr const char *InstX86Divps::Base::Opcode = "div";
template <> constexpr const char *InstX86Divss::Base::Opcode = "div";
template <> constexpr const char *InstX86Idiv::Base::Opcode = "idiv";
template <> constexpr const char *InstX86Rol::Base::Opcode = "rol";
template <> constexpr const char *InstX86Shl::Base::Opcode = "shl";
template <> constexpr const char *InstX86Psll::Base::Opcode = "psll";
template <> constexpr const char *InstX86Shr::Base::Opcode = "shr";
template <> constexpr const char *InstX86Sar::Base::Opcode = "sar";
template <> constexpr const char *InstX86Psra::Base::Opcode = "psra";
template <> constexpr const char *InstX86Psrl::Base::Opcode = "psrl";
template <> constexpr const char *InstX86Pcmpeq::Base::Opcode = "pcmpeq";
template <> constexpr const char *InstX86Pcmpgt::Base::Opcode = "pcmpgt";
template <> constexpr const char *InstX86MovssRegs::Base::Opcode = "movss";
/* Ternary ops */
template <> constexpr const char *InstX86Insertps::Base::Opcode = "insertps";
template <> constexpr const char *InstX86Round::Base::Opcode = "round";
template <> constexpr const char *InstX86Shufps::Base::Opcode = "shufps";
template <> constexpr const char *InstX86Pinsr::Base::Opcode = "pinsr";
template <> constexpr const char *InstX86Blendvps::Base::Opcode = "blendvps";
template <> constexpr const char *InstX86Pblendvb::Base::Opcode = "pblendvb";
/* Three address ops */
template <> constexpr const char *InstX86Pextr::Base::Opcode = "pextr";
template <> constexpr const char *InstX86Pshufd::Base::Opcode = "pshufd";
template <> constexpr const char *InstX86Pshufb::Base::Opcode = "pshufb";
template <> constexpr const char *InstX86Punpckl::Base::Opcode = "punpckl";
template <> constexpr const char *InstX86Punpckh::Base::Opcode = "punpckh";
template <> constexpr const char *InstX86Packss::Base::Opcode = "packss";
template <> constexpr const char *InstX86Packus::Base::Opcode = "packus";
/* Inplace GPR ops */
template <>
constexpr const Assembler::GPREmitterOneOp InstX86Bswap::Base::Emitter = {
    &Assembler::bswap, nullptr /* only a reg form exists */
};
template <>
constexpr const Assembler::GPREmitterOneOp InstX86Neg::Base::Emitter = {
    &Assembler::neg, &Assembler::neg};
/* Unary GPR ops */
/* uses specialized emitter. */
template <>
constexpr const Assembler::GPREmitterRegOp InstX86Cbwdq::Base::Emitter = {
    nullptr, nullptr, nullptr};
template <>
constexpr const Assembler::GPREmitterRegOp InstX86Bsf::Base::Emitter = {
    &Assembler::bsf, &Assembler::bsf, nullptr};
template <>
constexpr const Assembler::GPREmitterRegOp InstX86Bsr::Base::Emitter = {
    &Assembler::bsr, &Assembler::bsr, nullptr};
template <>
constexpr const Assembler::GPREmitterRegOp InstX86Lea::Base::Emitter = {
    /* reg/reg and reg/imm are illegal */ nullptr, &Assembler::lea, nullptr};
template <>
constexpr const Assembler::GPREmitterRegOp InstX86Movsx::Base::Emitter = {
    &Assembler::movsx, &Assembler::movsx, nullptr};
template <>
constexpr const Assembler::GPREmitterRegOp InstX86Movzx::Base::Emitter = {
    &Assembler::movzx, &Assembler::movzx, nullptr};
/* Unary XMM ops */
/* uses specialized emitter. */
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Movd::Base::Emitter = {
    nullptr, nullptr};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Sqrt::Base::Emitter = {
    &Assembler::sqrt, &Assembler::sqrt};
/* Binary GPR ops */
/* uses specialized emitter. */
template <>
constexpr const Assembler::GPREmitterRegOp InstX86Imul::Base::Emitter = {
    nullptr, nullptr, nullptr};
template <>
constexpr const Assembler::GPREmitterRegOp InstX86Add::Base::Emitter = {
    &Assembler::add, &Assembler::add, &Assembler::add};
template <>
constexpr const Assembler::GPREmitterAddrOp InstX86AddRMW::Base::Emitter = {
    &Assembler::add, &Assembler::add};
template <>
constexpr const Assembler::GPREmitterRegOp InstX86Adc::Base::Emitter = {
    &Assembler::adc, &Assembler::adc, &Assembler::adc};
template <>
constexpr const Assembler::GPREmitterAddrOp InstX86AdcRMW::Base::Emitter = {
    &Assembler::adc, &Assembler::adc};
template <>
constexpr const Assembler::GPREmitterRegOp InstX86And::Base::Emitter = {
    &Assembler::And, &Assembler::And, &Assembler::And};
template <>
constexpr const Assembler::GPREmitterAddrOp InstX86AndRMW::Base::Emitter = {
    &Assembler::And, &Assembler::And};
template <>
constexpr const Assembler::GPREmitterRegOp InstX86Or::Base::Emitter = {
    &Assembler::Or, &Assembler::Or, &Assembler::Or};
template <>
constexpr const Assembler::GPREmitterAddrOp InstX86OrRMW::Base::Emitter = {
    &Assembler::Or, &Assembler::Or};
template <>
constexpr const Assembler::GPREmitterRegOp InstX86Sbb::Base::Emitter = {
    &Assembler::sbb, &Assembler::sbb, &Assembler::sbb};
template <>
constexpr const Assembler::GPREmitterAddrOp InstX86SbbRMW::Base::Emitter = {
    &Assembler::sbb, &Assembler::sbb};
template <>
constexpr const Assembler::GPREmitterRegOp InstX86Sub::Base::Emitter = {
    &Assembler::sub, &Assembler::sub, &Assembler::sub};
template <>
constexpr const Assembler::GPREmitterAddrOp InstX86SubRMW::Base::Emitter = {
    &Assembler::sub, &Assembler::sub};
template <>
constexpr const Assembler::GPREmitterRegOp InstX86Xor::Base::Emitter = {
    &Assembler::Xor, &Assembler::Xor, &Assembler::Xor};
template <>
constexpr const Assembler::GPREmitterAddrOp InstX86XorRMW::Base::Emitter = {
    &Assembler::Xor, &Assembler::Xor};
/* Binary Shift GPR ops */
template <>
constexpr const Assembler::GPREmitterShiftOp InstX86Rol::Base::Emitter = {
    &Assembler::rol, &Assembler::rol};
template <>
constexpr const Assembler::GPREmitterShiftOp InstX86Sar::Base::Emitter = {
    &Assembler::sar, &Assembler::sar};
template <>
constexpr const Assembler::GPREmitterShiftOp InstX86Shl::Base::Emitter = {
    &Assembler::shl, &Assembler::shl};
template <>
constexpr const Assembler::GPREmitterShiftOp InstX86Shr::Base::Emitter = {
    &Assembler::shr, &Assembler::shr};
/* Binary XMM ops */
/* uses specialized emitter. */
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86MovssRegs::Base::Emitter = {
    nullptr, nullptr};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Addss::Base::Emitter = {
    &Assembler::addss, &Assembler::addss};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Addps::Base::Emitter = {
    &Assembler::addps, &Assembler::addps};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Divss::Base::Emitter = {
    &Assembler::divss, &Assembler::divss};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Divps::Base::Emitter = {
    &Assembler::divps, &Assembler::divps};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Mulss::Base::Emitter = {
    &Assembler::mulss, &Assembler::mulss};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Mulps::Base::Emitter = {
    &Assembler::mulps, &Assembler::mulps};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Padd::Base::Emitter = {
    &Assembler::padd, &Assembler::padd};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Padds::Base::Emitter = {
    &Assembler::padds, &Assembler::padds};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Paddus::Base::Emitter = {
    &Assembler::paddus, &Assembler::paddus};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Pand::Base::Emitter = {
    &Assembler::pand, &Assembler::pand};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Pandn::Base::Emitter = {
    &Assembler::pandn, &Assembler::pandn};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Pcmpeq::Base::Emitter = {
    &Assembler::pcmpeq, &Assembler::pcmpeq};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Pcmpgt::Base::Emitter = {
    &Assembler::pcmpgt, &Assembler::pcmpgt};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Pmull::Base::Emitter = {
    &Assembler::pmull, &Assembler::pmull};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Pmulhw::Base::Emitter = {
    &Assembler::pmulhw, &Assembler::pmulhw};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Pmulhuw::Base::Emitter = {
    &Assembler::pmulhuw, &Assembler::pmulhuw};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Pmaddwd::Base::Emitter = {
    &Assembler::pmaddwd, &Assembler::pmaddwd};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Pmuludq::Base::Emitter = {
    &Assembler::pmuludq, &Assembler::pmuludq};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Por::Base::Emitter = {
    &Assembler::por, &Assembler::por};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Psub::Base::Emitter = {
    &Assembler::psub, &Assembler::psub};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Psubs::Base::Emitter = {
    &Assembler::psubs, &Assembler::psubs};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Psubus::Base::Emitter = {
    &Assembler::psubus, &Assembler::psubus};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Pxor::Base::Emitter = {
    &Assembler::pxor, &Assembler::pxor};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Subss::Base::Emitter = {
    &Assembler::subss, &Assembler::subss};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Subps::Base::Emitter = {
    &Assembler::subps, &Assembler::subps};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Andnps::Base::Emitter = {
    &Assembler::andnps, &Assembler::andnps};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Andps::Base::Emitter = {
    &Assembler::andps, &Assembler::andps};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Maxss::Base::Emitter = {
    &Assembler::maxss, &Assembler::maxss};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Minss::Base::Emitter = {
    &Assembler::minss, &Assembler::minss};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Maxps::Base::Emitter = {
    &Assembler::maxps, &Assembler::maxps};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Minps::Base::Emitter = {
    &Assembler::minps, &Assembler::minps};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Orps::Base::Emitter = {
    &Assembler::orps, &Assembler::orps};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Xorps::Base::Emitter = {
    &Assembler::xorps, &Assembler::xorps}; /* Binary XMM Shift ops */
template <>
constexpr const Assembler::XmmEmitterShiftOp InstX86Psll::Base::Emitter = {
    &Assembler::psll, &Assembler::psll, &Assembler::psll};
template <>
constexpr const Assembler::XmmEmitterShiftOp InstX86Psra::Base::Emitter = {
    &Assembler::psra, &Assembler::psra, &Assembler::psra};
template <>
constexpr const Assembler::XmmEmitterShiftOp InstX86Psrl::Base::Emitter = {
    &Assembler::psrl, &Assembler::psrl, &Assembler::psrl};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Pshufb::Base::Emitter = {
    &Assembler::pshufb, &Assembler::pshufb};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Punpckl::Base::Emitter = {
    &Assembler::punpckl, &Assembler::punpckl};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Punpckh::Base::Emitter = {
    &Assembler::punpckh, &Assembler::punpckh};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Packss::Base::Emitter = {
    &Assembler::packss, &Assembler::packss};
template <>
constexpr const Assembler::XmmEmitterRegOp InstX86Packus::Base::Emitter = {
    &Assembler::packus, &Assembler::packus};

} // end of namespace X8664
} // end of namespace Ice

#endif // SUBZERO_SRC_ICEINSTX8664_H
