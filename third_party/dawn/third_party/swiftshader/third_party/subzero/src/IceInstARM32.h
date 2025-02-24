//===- subzero/src/IceInstARM32.h - ARM32 machine instructions --*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the InstARM32 and OperandARM32 classes and their subclasses.
///
/// This represents the machine instructions and operands used for ARM32 code
/// selection.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEINSTARM32_H
#define SUBZERO_SRC_ICEINSTARM32_H

#include "IceConditionCodesARM32.h"
#include "IceDefs.h"
#include "IceInst.h"
#include "IceInstARM32.def"
#include "IceOperand.h"
#include "IceRegistersARM32.h"

namespace Ice {
namespace ARM32 {

/// Encoding of an ARM 32-bit instruction.
using IValueT = uint32_t;

/// An Offset value (+/-) used in an ARM 32-bit instruction.
using IOffsetT = int32_t;

class TargetARM32;

/// OperandARM32 extends the Operand hierarchy. Its subclasses are
/// OperandARM32Mem and OperandARM32Flex.
class OperandARM32 : public Operand {
  OperandARM32() = delete;
  OperandARM32(const OperandARM32 &) = delete;
  OperandARM32 &operator=(const OperandARM32 &) = delete;

public:
  enum OperandKindARM32 {
    k__Start = Operand::kTarget,
    kMem,
    kShAmtImm,
    kFlexStart,
    kFlexImm = kFlexStart,
    kFlexFpImm,
    kFlexFpZero,
    kFlexReg,
    kFlexEnd = kFlexReg
  };

  enum ShiftKind {
    kNoShift = -1,
#define X(enum, emit) enum,
    ICEINSTARM32SHIFT_TABLE
#undef X
  };

  using Operand::dump;
  void dump(const Cfg *, Ostream &Str) const override {
    if (BuildDefs::dump())
      Str << "<OperandARM32>";
  }

protected:
  OperandARM32(OperandKindARM32 Kind, Type Ty)
      : Operand(static_cast<OperandKind>(Kind), Ty) {}
};

/// OperandARM32Mem represents a memory operand in any of the various ARM32
/// addressing modes.
class OperandARM32Mem : public OperandARM32 {
  OperandARM32Mem() = delete;
  OperandARM32Mem(const OperandARM32Mem &) = delete;
  OperandARM32Mem &operator=(const OperandARM32Mem &) = delete;

public:
  /// Memory operand addressing mode.
  /// The enum value also carries the encoding.
  // TODO(jvoung): unify with the assembler.
  enum AddrMode {
    // bit encoding P U 0 W
    Offset = (8 | 4 | 0) << 21,      // offset (w/o writeback to base)
    PreIndex = (8 | 4 | 1) << 21,    // pre-indexed addressing with writeback
    PostIndex = (0 | 4 | 0) << 21,   // post-indexed addressing with writeback
    NegOffset = (8 | 0 | 0) << 21,   // negative offset (w/o writeback to base)
    NegPreIndex = (8 | 0 | 1) << 21, // negative pre-indexed with writeback
    NegPostIndex = (0 | 0 | 0) << 21 // negative post-indexed with writeback
  };

  /// Provide two constructors.
  /// NOTE: The Variable-typed operands have to be registers.
  ///
  /// (1) Reg + Imm. The Immediate actually has a limited number of bits
  /// for encoding, so check canHoldOffset first. It cannot handle general
  /// Constant operands like ConstantRelocatable, since a relocatable can
  /// potentially take up too many bits.
  static OperandARM32Mem *create(Cfg *Func, Type Ty, Variable *Base,
                                 ConstantInteger32 *ImmOffset,
                                 AddrMode Mode = Offset) {
    return new (Func->allocate<OperandARM32Mem>())
        OperandARM32Mem(Func, Ty, Base, ImmOffset, Mode);
  }
  /// (2) Reg +/- Reg with an optional shift of some kind and amount.
  static OperandARM32Mem *create(Cfg *Func, Type Ty, Variable *Base,
                                 Variable *Index, ShiftKind ShiftOp = kNoShift,
                                 uint16_t ShiftAmt = 0,
                                 AddrMode Mode = Offset) {
    return new (Func->allocate<OperandARM32Mem>())
        OperandARM32Mem(Func, Ty, Base, Index, ShiftOp, ShiftAmt, Mode);
  }
  Variable *getBase() const { return Base; }
  ConstantInteger32 *getOffset() const { return ImmOffset; }
  Variable *getIndex() const { return Index; }
  ShiftKind getShiftOp() const { return ShiftOp; }
  uint16_t getShiftAmt() const { return ShiftAmt; }
  AddrMode getAddrMode() const { return Mode; }

  bool isRegReg() const { return Index != nullptr; }
  bool isNegAddrMode() const {
    // Positive address modes have the "U" bit set, and negative modes don't.
    static_assert((PreIndex & (4 << 21)) != 0,
                  "Positive addr modes should have U bit set.");
    static_assert((NegPreIndex & (4 << 21)) == 0,
                  "Negative addr modes should have U bit clear.");
    return (Mode & (4 << 21)) == 0;
  }

  void emit(const Cfg *Func) const override;
  using OperandARM32::dump;
  void dump(const Cfg *Func, Ostream &Str) const override;

  static bool classof(const Operand *Operand) {
    return Operand->getKind() == static_cast<OperandKind>(kMem);
  }

  /// Return true if a load/store instruction for an element of type Ty can
  /// encode the Offset directly in the immediate field of the 32-bit ARM
  /// instruction. For some types, if the load is Sign extending, then the range
  /// is reduced.
  static bool canHoldOffset(Type Ty, bool SignExt, int32_t Offset);

private:
  OperandARM32Mem(Cfg *Func, Type Ty, Variable *Base,
                  ConstantInteger32 *ImmOffset, AddrMode Mode);
  OperandARM32Mem(Cfg *Func, Type Ty, Variable *Base, Variable *Index,
                  ShiftKind ShiftOp, uint16_t ShiftAmt, AddrMode Mode);

  Variable *Base;
  ConstantInteger32 *ImmOffset;
  Variable *Index;
  ShiftKind ShiftOp;
  uint16_t ShiftAmt;
  AddrMode Mode;
};

/// OperandARM32ShAmtImm represents an Immediate that is used in one of the
/// shift-by-immediate instructions (lsl, lsr, and asr), and shift-by-immediate
/// shifted registers.
class OperandARM32ShAmtImm : public OperandARM32 {
  OperandARM32ShAmtImm() = delete;
  OperandARM32ShAmtImm(const OperandARM32ShAmtImm &) = delete;
  OperandARM32ShAmtImm &operator=(const OperandARM32ShAmtImm &) = delete;

public:
  static OperandARM32ShAmtImm *create(Cfg *Func, ConstantInteger32 *ShAmt) {
    return new (Func->allocate<OperandARM32ShAmtImm>())
        OperandARM32ShAmtImm(ShAmt);
  }

  static bool classof(const Operand *Operand) {
    return Operand->getKind() == static_cast<OperandKind>(kShAmtImm);
  }

  void emit(const Cfg *Func) const override;
  using OperandARM32::dump;
  void dump(const Cfg *Func, Ostream &Str) const override;

  uint32_t getShAmtImm() const { return ShAmt->getValue(); }

private:
  explicit OperandARM32ShAmtImm(ConstantInteger32 *SA);

  const ConstantInteger32 *const ShAmt;
};

/// OperandARM32Flex represent the "flexible second operand" for data-processing
/// instructions. It can be a rotatable 8-bit constant, or a register with an
/// optional shift operand. The shift amount can even be a third register.
class OperandARM32Flex : public OperandARM32 {
  OperandARM32Flex() = delete;
  OperandARM32Flex(const OperandARM32Flex &) = delete;
  OperandARM32Flex &operator=(const OperandARM32Flex &) = delete;

public:
  static bool classof(const Operand *Operand) {
    return static_cast<OperandKind>(kFlexStart) <= Operand->getKind() &&
           Operand->getKind() <= static_cast<OperandKind>(kFlexEnd);
  }

protected:
  OperandARM32Flex(OperandKindARM32 Kind, Type Ty) : OperandARM32(Kind, Ty) {}
};

/// Rotated immediate variant.
class OperandARM32FlexImm : public OperandARM32Flex {
  OperandARM32FlexImm() = delete;
  OperandARM32FlexImm(const OperandARM32FlexImm &) = delete;
  OperandARM32FlexImm &operator=(const OperandARM32FlexImm &) = delete;

public:
  /// Immed_8 rotated by an even number of bits (2 * RotateAmt).
  static OperandARM32FlexImm *create(Cfg *Func, Type Ty, uint32_t Imm,
                                     uint32_t RotateAmt);

  void emit(const Cfg *Func) const override;
  using OperandARM32::dump;
  void dump(const Cfg *Func, Ostream &Str) const override;

  static bool classof(const Operand *Operand) {
    return Operand->getKind() == static_cast<OperandKind>(kFlexImm);
  }

  /// Return true if the Immediate can fit in the ARM flexible operand. Fills in
  /// the out-params RotateAmt and Immed_8 if Immediate fits.
  static bool canHoldImm(uint32_t Immediate, uint32_t *RotateAmt,
                         uint32_t *Immed_8);

  uint32_t getImm() const { return Imm; }
  uint32_t getRotateAmt() const { return RotateAmt; }

private:
  OperandARM32FlexImm(Cfg *Func, Type Ty, uint32_t Imm, uint32_t RotateAmt);

  uint32_t Imm;
  uint32_t RotateAmt;
};

/// Modified Floating-point constant.
class OperandARM32FlexFpImm : public OperandARM32Flex {
  OperandARM32FlexFpImm() = delete;
  OperandARM32FlexFpImm(const OperandARM32FlexFpImm &) = delete;
  OperandARM32FlexFpImm &operator=(const OperandARM32FlexFpImm &) = delete;

public:
  static OperandARM32FlexFpImm *create(Cfg *Func, Type Ty,
                                       uint32_t ModifiedImm) {
    return new (Func->allocate<OperandARM32FlexFpImm>())
        OperandARM32FlexFpImm(Func, Ty, ModifiedImm);
  }

  void emit(const Cfg *Func) const override;
  using OperandARM32::dump;
  void dump(const Cfg *Func, Ostream &Str) const override;

  static bool classof(const Operand *Operand) {
    return Operand->getKind() == static_cast<OperandKind>(kFlexFpImm);
  }

  static bool canHoldImm(const Operand *C, uint32_t *ModifiedImm);

  uint32_t getModifiedImm() const { return ModifiedImm; }

private:
  OperandARM32FlexFpImm(Cfg *Func, Type Ty, uint32_t ModifiedImm);

  const uint32_t ModifiedImm;
};

/// An operand for representing the 0.0 immediate in vcmp.
class OperandARM32FlexFpZero : public OperandARM32Flex {
  OperandARM32FlexFpZero() = delete;
  OperandARM32FlexFpZero(const OperandARM32FlexFpZero &) = delete;
  OperandARM32FlexFpZero &operator=(const OperandARM32FlexFpZero &) = delete;

public:
  static OperandARM32FlexFpZero *create(Cfg *Func, Type Ty) {
    return new (Func->allocate<OperandARM32FlexFpZero>())
        OperandARM32FlexFpZero(Func, Ty);
  }

  void emit(const Cfg *Func) const override;
  using OperandARM32::dump;
  void dump(const Cfg *Func, Ostream &Str) const override;

  static bool classof(const Operand *Operand) {
    return Operand->getKind() == static_cast<OperandKind>(kFlexFpZero);
  }

private:
  OperandARM32FlexFpZero(Cfg *Func, Type Ty);
};

/// Shifted register variant.
class OperandARM32FlexReg : public OperandARM32Flex {
  OperandARM32FlexReg() = delete;
  OperandARM32FlexReg(const OperandARM32FlexReg &) = delete;
  OperandARM32FlexReg &operator=(const OperandARM32FlexReg &) = delete;

public:
  /// Register with immediate/reg shift amount and shift operation.
  static OperandARM32FlexReg *create(Cfg *Func, Type Ty, Variable *Reg,
                                     ShiftKind ShiftOp, Operand *ShiftAmt) {
    return new (Func->allocate<OperandARM32FlexReg>())
        OperandARM32FlexReg(Func, Ty, Reg, ShiftOp, ShiftAmt);
  }

  void emit(const Cfg *Func) const override;
  using OperandARM32::dump;
  void dump(const Cfg *Func, Ostream &Str) const override;

  static bool classof(const Operand *Operand) {
    return Operand->getKind() == static_cast<OperandKind>(kFlexReg);
  }

  Variable *getReg() const { return Reg; }
  ShiftKind getShiftOp() const { return ShiftOp; }
  /// ShiftAmt can represent an immediate or a register.
  Operand *getShiftAmt() const { return ShiftAmt; }

private:
  OperandARM32FlexReg(Cfg *Func, Type Ty, Variable *Reg, ShiftKind ShiftOp,
                      Operand *ShiftAmt);

  Variable *Reg;
  ShiftKind ShiftOp;
  Operand *ShiftAmt;
};

/// StackVariable represents a Var that isn't assigned a register (stack-only).
/// It is assigned a stack slot, but the slot's offset may be too large to
/// represent in the native addressing mode, and so it has a separate base
/// register from SP/FP, where the offset from that base register is then in
/// range.
class StackVariable final : public Variable {
  StackVariable() = delete;
  StackVariable(const StackVariable &) = delete;
  StackVariable &operator=(const StackVariable &) = delete;

public:
  static StackVariable *create(Cfg *Func, Type Ty, SizeT Index) {
    return new (Func->allocate<StackVariable>()) StackVariable(Func, Ty, Index);
  }
  constexpr static auto StackVariableKind =
      static_cast<OperandKind>(kVariable_Target);
  static bool classof(const Operand *Operand) {
    return Operand->getKind() == StackVariableKind;
  }
  void setBaseRegNum(RegNumT RegNum) { BaseRegNum = RegNum; }
  RegNumT getBaseRegNum() const override { return BaseRegNum; }
  // Inherit dump() and emit() from Variable.

private:
  StackVariable(const Cfg *Func, Type Ty, SizeT Index)
      : Variable(Func, StackVariableKind, Ty, Index) {}
  RegNumT BaseRegNum;
};

/// Base class for ARM instructions. While most ARM instructions can be
/// conditionally executed, a few of them are not predicable (halt, memory
/// barriers, etc.).
class InstARM32 : public InstTarget {
  InstARM32() = delete;
  InstARM32(const InstARM32 &) = delete;
  InstARM32 &operator=(const InstARM32 &) = delete;

public:
  // Defines form that assembly instruction should be synthesized.
  enum EmitForm { Emit_Text, Emit_Binary };

  enum InstKindARM32 {
    k__Start = Inst::Target,
    Adc,
    Add,
    And,
    Asr,
    Bic,
    Br,
    Call,
    Clz,
    Cmn,
    Cmp,
    Dmb,
    Eor,
    Extract,
    Insert,
    Label,
    Ldr,
    Ldrex,
    Lsl,
    Lsr,
    Nop,
    Mla,
    Mls,
    Mov,
    Movt,
    Movw,
    Mul,
    Mvn,
    Orr,
    Pop,
    Push,
    Rbit,
    Ret,
    Rev,
    Rsb,
    Rsc,
    Sbc,
    Sdiv,
    Str,
    Strex,
    Sub,
    Sxt,
    Trap,
    Tst,
    Udiv,
    Umull,
    Uxt,
    Vabs,
    Vadd,
    Vand,
    Vbsl,
    Vceq,
    Vcge,
    Vcgt,
    Vcmp,
    Vcvt,
    Vdiv,
    Vdup,
    Veor,
    Vldr1d,
    Vldr1q,
    Vmla,
    Vmlap,
    Vmls,
    Vmovl,
    Vmovh,
    Vmovhl,
    Vmovlh,
    Vmrs,
    Vmul,
    Vmulh,
    Vmvn,
    Vneg,
    Vorr,
    Vqadd,
    Vqmovn2,
    Vqsub,
    Vshl,
    Vshr,
    Vsqrt,
    Vstr1,
    Vsub,
    Vzip
  };

  static constexpr size_t InstSize = sizeof(uint32_t);

  static CondARM32::Cond getOppositeCondition(CondARM32::Cond Cond);

  /// Called inside derived methods emit() to communicate that multiple
  /// instructions are being generated. Used by emitIAS() methods to
  /// generate textual fixups for instructions that are not yet
  /// implemented.
  void startNextInst(const Cfg *Func) const;

  /// FPSign is used for certain vector instructions (particularly, right
  /// shifts) that require an operand sign specification.
  enum FPSign {
    FS_None,
    FS_Signed,
    FS_Unsigned,
  };
  /// Shared emit routines for common forms of instructions.
  /// @{
  static void emitThreeAddrFP(const char *Opcode, FPSign Sign,
                              const InstARM32 *Instr, const Cfg *Func,
                              Type OpType);
  static void emitFourAddrFP(const char *Opcode, FPSign Sign,
                             const InstARM32 *Instr, const Cfg *Func);
  /// @}

  void dump(const Cfg *Func) const override;

  void emitIAS(const Cfg *Func) const override;

protected:
  InstARM32(Cfg *Func, InstKindARM32 Kind, SizeT Maxsrcs, Variable *Dest)
      : InstTarget(Func, static_cast<InstKind>(Kind), Maxsrcs, Dest) {}

  static bool isClassof(const Inst *Instr, InstKindARM32 MyKind) {
    return Instr->getKind() == static_cast<InstKind>(MyKind);
  }

  // Generates text of assembly instruction using method emit(), and then adds
  // to the assembly buffer as a Fixup.
  void emitUsingTextFixup(const Cfg *Func) const;
};

/// A predicable ARM instruction.
class InstARM32Pred : public InstARM32 {
  InstARM32Pred() = delete;
  InstARM32Pred(const InstARM32Pred &) = delete;
  InstARM32Pred &operator=(const InstARM32Pred &) = delete;

public:
  InstARM32Pred(Cfg *Func, InstKindARM32 Kind, SizeT Maxsrcs, Variable *Dest,
                CondARM32::Cond Predicate)
      : InstARM32(Func, Kind, Maxsrcs, Dest), Predicate(Predicate) {}

  CondARM32::Cond getPredicate() const { return Predicate; }
  void setPredicate(CondARM32::Cond Pred) { Predicate = Pred; }

  static const char *predString(CondARM32::Cond Predicate);
  void dumpOpcodePred(Ostream &Str, const char *Opcode, Type Ty) const;

  /// Shared emit routines for common forms of instructions.
  static void emitUnaryopGPR(const char *Opcode, const InstARM32Pred *Instr,
                             const Cfg *Func, bool NeedsWidthSuffix);
  static void emitUnaryopFP(const char *Opcode, FPSign Sign,
                            const InstARM32Pred *Instr, const Cfg *Func);
  static void emitTwoAddr(const char *Opcode, const InstARM32Pred *Instr,
                          const Cfg *Func);
  static void emitThreeAddr(const char *Opcode, const InstARM32Pred *Instr,
                            const Cfg *Func, bool SetFlags);
  static void emitFourAddr(const char *Opcode, const InstARM32Pred *Instr,
                           const Cfg *Func);
  static void emitCmpLike(const char *Opcode, const InstARM32Pred *Instr,
                          const Cfg *Func);

protected:
  CondARM32::Cond Predicate;
};

template <typename StreamType>
inline StreamType &operator<<(StreamType &Stream, CondARM32::Cond Predicate) {
  Stream << InstARM32Pred::predString(Predicate);
  return Stream;
}

/// Instructions of the form x := op(y).
template <InstARM32::InstKindARM32 K, bool NeedsWidthSuffix>
class InstARM32UnaryopGPR : public InstARM32Pred {
  InstARM32UnaryopGPR() = delete;
  InstARM32UnaryopGPR(const InstARM32UnaryopGPR &) = delete;
  InstARM32UnaryopGPR &operator=(const InstARM32UnaryopGPR &) = delete;

public:
  static InstARM32UnaryopGPR *create(Cfg *Func, Variable *Dest, Operand *Src,
                                     CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32UnaryopGPR>())
        InstARM32UnaryopGPR(Func, Dest, Src, Predicate);
  }
  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    emitUnaryopGPR(Opcode, this, Func, NeedsWidthSuffix);
  }
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpDest(Func);
    Str << " = ";
    dumpOpcodePred(Str, Opcode, getDest()->getType());
    Str << " ";
    dumpSources(Func);
  }
  static bool classof(const Inst *Instr) { return isClassof(Instr, K); }

private:
  InstARM32UnaryopGPR(Cfg *Func, Variable *Dest, Operand *Src,
                      CondARM32::Cond Predicate)
      : InstARM32Pred(Func, K, 1, Dest, Predicate) {
    addSource(Src);
  }

  static const char *const Opcode;
};

/// Instructions of the form x := op(y), for vector/FP.
template <InstARM32::InstKindARM32 K>
class InstARM32UnaryopFP : public InstARM32Pred {
  InstARM32UnaryopFP() = delete;
  InstARM32UnaryopFP(const InstARM32UnaryopFP &) = delete;
  InstARM32UnaryopFP &operator=(const InstARM32UnaryopFP &) = delete;

public:
  static InstARM32UnaryopFP *create(Cfg *Func, Variable *Dest, Variable *Src,
                                    CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32UnaryopFP>())
        InstARM32UnaryopFP(Func, Dest, Src, Predicate);
  }
  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    emitUnaryopFP(Opcode, Sign, this, Func);
  }
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpDest(Func);
    Str << " = ";
    dumpOpcodePred(Str, Opcode, getDest()->getType());
    Str << " ";
    dumpSources(Func);
  }
  static bool classof(const Inst *Instr) { return isClassof(Instr, K); }

protected:
  InstARM32UnaryopFP(Cfg *Func, Variable *Dest, Operand *Src,
                     CondARM32::Cond Predicate)
      : InstARM32Pred(Func, K, 1, Dest, Predicate) {
    addSource(Src);
  }

  FPSign Sign = FS_None;
  static const char *const Opcode;
};

template <InstARM32::InstKindARM32 K>
class InstARM32UnaryopSignAwareFP : public InstARM32UnaryopFP<K> {
  InstARM32UnaryopSignAwareFP() = delete;
  InstARM32UnaryopSignAwareFP(const InstARM32UnaryopSignAwareFP &) = delete;
  InstARM32UnaryopSignAwareFP &
  operator=(const InstARM32UnaryopSignAwareFP &) = delete;

public:
  static InstARM32UnaryopSignAwareFP *
  create(Cfg *Func, Variable *Dest, Variable *Src, CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32UnaryopSignAwareFP>())
        InstARM32UnaryopSignAwareFP(Func, Dest, Src, Predicate);
  }
  void emitIAS(const Cfg *Func) const override;
  void setSignType(InstARM32::FPSign SignType) { this->Sign = SignType; }

private:
  InstARM32UnaryopSignAwareFP(Cfg *Func, Variable *Dest, Operand *Src,
                              CondARM32::Cond Predicate)
      : InstARM32UnaryopFP<K>(Func, Dest, Src, Predicate) {}
};

/// Instructions of the form x := x op y.
template <InstARM32::InstKindARM32 K>
class InstARM32TwoAddrGPR : public InstARM32Pred {
  InstARM32TwoAddrGPR() = delete;
  InstARM32TwoAddrGPR(const InstARM32TwoAddrGPR &) = delete;
  InstARM32TwoAddrGPR &operator=(const InstARM32TwoAddrGPR &) = delete;

public:
  /// Dest must be a register.
  static InstARM32TwoAddrGPR *create(Cfg *Func, Variable *Dest, Operand *Src,
                                     CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32TwoAddrGPR>())
        InstARM32TwoAddrGPR(Func, Dest, Src, Predicate);
  }
  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    emitTwoAddr(Opcode, this, Func);
  }
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpDest(Func);
    Str << " = ";
    dumpOpcodePred(Str, Opcode, getDest()->getType());
    Str << " ";
    dumpSources(Func);
  }
  static bool classof(const Inst *Instr) { return isClassof(Instr, K); }

private:
  InstARM32TwoAddrGPR(Cfg *Func, Variable *Dest, Operand *Src,
                      CondARM32::Cond Predicate)
      : InstARM32Pred(Func, K, 2, Dest, Predicate) {
    addSource(Dest);
    addSource(Src);
  }

  static const char *const Opcode;
};

/// Base class for load instructions.
template <InstARM32::InstKindARM32 K>
class InstARM32LoadBase : public InstARM32Pred {
  InstARM32LoadBase() = delete;
  InstARM32LoadBase(const InstARM32LoadBase &) = delete;
  InstARM32LoadBase &operator=(const InstARM32LoadBase &) = delete;

public:
  static InstARM32LoadBase *create(Cfg *Func, Variable *Dest, Operand *Source,
                                   CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32LoadBase>())
        InstARM32LoadBase(Func, Dest, Source, Predicate);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpOpcodePred(Str, Opcode, getDest()->getType());
    Str << " ";
    dumpDest(Func);
    Str << ", ";
    dumpSources(Func);
  }
  static bool classof(const Inst *Instr) { return isClassof(Instr, K); }

private:
  InstARM32LoadBase(Cfg *Func, Variable *Dest, Operand *Source,
                    CondARM32::Cond Predicate)
      : InstARM32Pred(Func, K, 1, Dest, Predicate) {
    addSource(Source);
  }

  static const char *const Opcode;
};

/// Instructions of the form x := y op z. May have the side-effect of setting
/// status flags.
template <InstARM32::InstKindARM32 K>
class InstARM32ThreeAddrGPR : public InstARM32Pred {
  InstARM32ThreeAddrGPR() = delete;
  InstARM32ThreeAddrGPR(const InstARM32ThreeAddrGPR &) = delete;
  InstARM32ThreeAddrGPR &operator=(const InstARM32ThreeAddrGPR &) = delete;

public:
  /// Create an ordinary binary-op instruction like add, and sub. Dest and Src1
  /// must be registers.
  static InstARM32ThreeAddrGPR *create(Cfg *Func, Variable *Dest,
                                       Variable *Src0, Operand *Src1,
                                       CondARM32::Cond Predicate,
                                       bool SetFlags = false) {
    return new (Func->allocate<InstARM32ThreeAddrGPR>())
        InstARM32ThreeAddrGPR(Func, Dest, Src0, Src1, Predicate, SetFlags);
  }
  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    emitThreeAddr(Opcode, this, Func, SetFlags);
  }
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpDest(Func);
    Str << " = ";
    dumpOpcodePred(Str, Opcode, getDest()->getType());
    Str << (SetFlags ? ".s " : " ");
    dumpSources(Func);
  }
  static bool classof(const Inst *Instr) { return isClassof(Instr, K); }

private:
  InstARM32ThreeAddrGPR(Cfg *Func, Variable *Dest, Variable *Src0,
                        Operand *Src1, CondARM32::Cond Predicate, bool SetFlags)
      : InstARM32Pred(Func, K, 2, Dest, Predicate), SetFlags(SetFlags) {
    HasSideEffects = SetFlags;
    addSource(Src0);
    addSource(Src1);
  }

  static const char *const Opcode;
  bool SetFlags;
};

/// Instructions of the form x := y op z, for vector/FP. We leave these as
/// unconditional: "ARM deprecates the conditional execution of any instruction
/// encoding provided by the Advanced SIMD Extension that is not also provided
/// by the floating-point (VFP) extension". They do not set flags.
template <InstARM32::InstKindARM32 K>
class InstARM32ThreeAddrFP : public InstARM32 {
  InstARM32ThreeAddrFP() = delete;
  InstARM32ThreeAddrFP(const InstARM32ThreeAddrFP &) = delete;
  InstARM32ThreeAddrFP &operator=(const InstARM32ThreeAddrFP &) = delete;

public:
  /// Create a vector/FP binary-op instruction like vadd, and vsub. Everything
  /// must be a register.
  static InstARM32ThreeAddrFP *create(Cfg *Func, Variable *Dest, Variable *Src0,
                                      Variable *Src1) {
    return new (Func->allocate<InstARM32ThreeAddrFP>())
        InstARM32ThreeAddrFP(Func, Dest, Src0, Src1);
  }
  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    const Type OpType = (isVectorCompare() ? getSrc(0) : getDest())->getType();
    emitThreeAddrFP(Opcode, Sign, this, Func, OpType);
  }
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpDest(Func);
    const Type OpType = (isVectorCompare() ? getSrc(0) : getDest())->getType();
    Str << " = " << Opcode << "." << OpType << " ";
    dumpSources(Func);
  }
  static bool classof(const Inst *Instr) { return isClassof(Instr, K); }

protected:
  FPSign Sign = FS_None;

  InstARM32ThreeAddrFP(Cfg *Func, Variable *Dest, Variable *Src0, Operand *Src1)
      : InstARM32(Func, K, 2, Dest) {
    addSource(Src0);
    addSource(Src1);
  }

  static const char *const Opcode;

private:
  static constexpr bool isVectorCompare() {
    return K == InstARM32::Vceq || K == InstARM32::Vcgt || K == InstARM32::Vcge;
  }
};

template <InstARM32::InstKindARM32 K>
class InstARM32ThreeAddrSignAwareFP : public InstARM32ThreeAddrFP<K> {
  InstARM32ThreeAddrSignAwareFP() = delete;
  InstARM32ThreeAddrSignAwareFP(const InstARM32ThreeAddrSignAwareFP &) = delete;
  InstARM32ThreeAddrSignAwareFP &
  operator=(const InstARM32ThreeAddrSignAwareFP &) = delete;

public:
  /// Create a vector/FP binary-op instruction like vadd, and vsub. Everything
  /// must be a register.
  static InstARM32ThreeAddrSignAwareFP *create(Cfg *Func, Variable *Dest,
                                               Variable *Src0, Variable *Src1) {
    return new (Func->allocate<InstARM32ThreeAddrSignAwareFP>())
        InstARM32ThreeAddrSignAwareFP(Func, Dest, Src0, Src1);
  }

  static InstARM32ThreeAddrSignAwareFP *
  create(Cfg *Func, Variable *Dest, Variable *Src0, ConstantInteger32 *Src1) {
    return new (Func->allocate<InstARM32ThreeAddrSignAwareFP>())
        InstARM32ThreeAddrSignAwareFP(Func, Dest, Src0, Src1);
  }

  void emitIAS(const Cfg *Func) const override;
  void setSignType(InstARM32::FPSign SignType) { this->Sign = SignType; }

private:
  InstARM32ThreeAddrSignAwareFP(Cfg *Func, Variable *Dest, Variable *Src0,
                                Operand *Src1)
      : InstARM32ThreeAddrFP<K>(Func, Dest, Src0, Src1) {}
};

/// Instructions of the form x := a op1 (y op2 z). E.g., multiply accumulate.
template <InstARM32::InstKindARM32 K>
class InstARM32FourAddrGPR : public InstARM32Pred {
  InstARM32FourAddrGPR() = delete;
  InstARM32FourAddrGPR(const InstARM32FourAddrGPR &) = delete;
  InstARM32FourAddrGPR &operator=(const InstARM32FourAddrGPR &) = delete;

public:
  // Every operand must be a register.
  static InstARM32FourAddrGPR *create(Cfg *Func, Variable *Dest, Variable *Src0,
                                      Variable *Src1, Variable *Src2,
                                      CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32FourAddrGPR>())
        InstARM32FourAddrGPR(Func, Dest, Src0, Src1, Src2, Predicate);
  }
  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    emitFourAddr(Opcode, this, Func);
  }
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpDest(Func);
    Str << " = ";
    dumpOpcodePred(Str, Opcode, getDest()->getType());
    Str << " ";
    dumpSources(Func);
  }
  static bool classof(const Inst *Instr) { return isClassof(Instr, K); }

private:
  InstARM32FourAddrGPR(Cfg *Func, Variable *Dest, Variable *Src0,
                       Variable *Src1, Variable *Src2,
                       CondARM32::Cond Predicate)
      : InstARM32Pred(Func, K, 3, Dest, Predicate) {
    addSource(Src0);
    addSource(Src1);
    addSource(Src2);
  }

  static const char *const Opcode;
};

/// Instructions of the form x := x op1 (y op2 z). E.g., multiply accumulate.
/// We leave these as unconditional: "ARM deprecates the conditional execution
/// of any instruction encoding provided by the Advanced SIMD Extension that is
/// not also provided by the floating-point (VFP) extension". They do not set
/// flags.
template <InstARM32::InstKindARM32 K>
class InstARM32FourAddrFP : public InstARM32 {
  InstARM32FourAddrFP() = delete;
  InstARM32FourAddrFP(const InstARM32FourAddrFP &) = delete;
  InstARM32FourAddrFP &operator=(const InstARM32FourAddrFP &) = delete;

public:
  // Every operand must be a register.
  static InstARM32FourAddrFP *create(Cfg *Func, Variable *Dest, Variable *Src0,
                                     Variable *Src1) {
    return new (Func->allocate<InstARM32FourAddrFP>())
        InstARM32FourAddrFP(Func, Dest, Src0, Src1);
  }
  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    emitFourAddrFP(Opcode, Sign, this, Func);
  }
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpDest(Func);
    Str << " = ";
    Str << Opcode << "." << getDest()->getType() << " ";
    dumpDest(Func);
    Str << ", ";
    dumpSources(Func);
  }
  static bool classof(const Inst *Instr) { return isClassof(Instr, K); }

private:
  InstARM32FourAddrFP(Cfg *Func, Variable *Dest, Variable *Src0, Variable *Src1)
      : InstARM32(Func, K, 3, Dest) {
    addSource(Dest);
    addSource(Src0);
    addSource(Src1);
  }

  FPSign Sign = FS_None;
  static const char *const Opcode;
};

/// Instructions of the form x cmpop y (setting flags).
template <InstARM32::InstKindARM32 K>
class InstARM32CmpLike : public InstARM32Pred {
  InstARM32CmpLike() = delete;
  InstARM32CmpLike(const InstARM32CmpLike &) = delete;
  InstARM32CmpLike &operator=(const InstARM32CmpLike &) = delete;

public:
  static InstARM32CmpLike *create(Cfg *Func, Variable *Src0, Operand *Src1,
                                  CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32CmpLike>())
        InstARM32CmpLike(Func, Src0, Src1, Predicate);
  }
  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    emitCmpLike(Opcode, this, Func);
  }
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpOpcodePred(Str, Opcode, getSrc(0)->getType());
    Str << " ";
    dumpSources(Func);
  }
  static bool classof(const Inst *Instr) { return isClassof(Instr, K); }

private:
  InstARM32CmpLike(Cfg *Func, Variable *Src0, Operand *Src1,
                   CondARM32::Cond Predicate)
      : InstARM32Pred(Func, K, 2, nullptr, Predicate) {
    HasSideEffects = true;
    addSource(Src0);
    addSource(Src1);
  }

  static const char *const Opcode;
};

using InstARM32Adc = InstARM32ThreeAddrGPR<InstARM32::Adc>;
using InstARM32Add = InstARM32ThreeAddrGPR<InstARM32::Add>;
using InstARM32And = InstARM32ThreeAddrGPR<InstARM32::And>;
using InstARM32Asr = InstARM32ThreeAddrGPR<InstARM32::Asr>;
using InstARM32Bic = InstARM32ThreeAddrGPR<InstARM32::Bic>;
using InstARM32Eor = InstARM32ThreeAddrGPR<InstARM32::Eor>;
using InstARM32Lsl = InstARM32ThreeAddrGPR<InstARM32::Lsl>;
using InstARM32Lsr = InstARM32ThreeAddrGPR<InstARM32::Lsr>;
using InstARM32Mul = InstARM32ThreeAddrGPR<InstARM32::Mul>;
using InstARM32Orr = InstARM32ThreeAddrGPR<InstARM32::Orr>;
using InstARM32Rsb = InstARM32ThreeAddrGPR<InstARM32::Rsb>;
using InstARM32Rsc = InstARM32ThreeAddrGPR<InstARM32::Rsc>;
using InstARM32Sbc = InstARM32ThreeAddrGPR<InstARM32::Sbc>;
using InstARM32Sdiv = InstARM32ThreeAddrGPR<InstARM32::Sdiv>;
using InstARM32Sub = InstARM32ThreeAddrGPR<InstARM32::Sub>;
using InstARM32Udiv = InstARM32ThreeAddrGPR<InstARM32::Udiv>;
using InstARM32Vadd = InstARM32ThreeAddrFP<InstARM32::Vadd>;
using InstARM32Vand = InstARM32ThreeAddrFP<InstARM32::Vand>;
using InstARM32Vbsl = InstARM32ThreeAddrFP<InstARM32::Vbsl>;
using InstARM32Vceq = InstARM32ThreeAddrFP<InstARM32::Vceq>;
using InstARM32Vcge = InstARM32ThreeAddrSignAwareFP<InstARM32::Vcge>;
using InstARM32Vcgt = InstARM32ThreeAddrSignAwareFP<InstARM32::Vcgt>;
using InstARM32Vdiv = InstARM32ThreeAddrFP<InstARM32::Vdiv>;
using InstARM32Veor = InstARM32ThreeAddrFP<InstARM32::Veor>;
using InstARM32Vmla = InstARM32FourAddrFP<InstARM32::Vmla>;
using InstARM32Vmls = InstARM32FourAddrFP<InstARM32::Vmls>;
using InstARM32Vmovl = InstARM32ThreeAddrFP<InstARM32::Vmovl>;
using InstARM32Vmovh = InstARM32ThreeAddrFP<InstARM32::Vmovh>;
using InstARM32Vmovhl = InstARM32ThreeAddrFP<InstARM32::Vmovhl>;
using InstARM32Vmovlh = InstARM32ThreeAddrFP<InstARM32::Vmovlh>;
using InstARM32Vmul = InstARM32ThreeAddrFP<InstARM32::Vmul>;
using InstARM32Vmvn = InstARM32UnaryopFP<InstARM32::Vmvn>;
using InstARM32Vneg = InstARM32UnaryopSignAwareFP<InstARM32::Vneg>;
using InstARM32Vorr = InstARM32ThreeAddrFP<InstARM32::Vorr>;
using InstARM32Vqadd = InstARM32ThreeAddrSignAwareFP<InstARM32::Vqadd>;
using InstARM32Vqsub = InstARM32ThreeAddrSignAwareFP<InstARM32::Vqsub>;
using InstARM32Vqmovn2 = InstARM32ThreeAddrSignAwareFP<InstARM32::Vqmovn2>;
using InstARM32Vmulh = InstARM32ThreeAddrSignAwareFP<InstARM32::Vmulh>;
using InstARM32Vmlap = InstARM32ThreeAddrFP<InstARM32::Vmlap>;
using InstARM32Vshl = InstARM32ThreeAddrSignAwareFP<InstARM32::Vshl>;
using InstARM32Vshr = InstARM32ThreeAddrSignAwareFP<InstARM32::Vshr>;
using InstARM32Vsub = InstARM32ThreeAddrFP<InstARM32::Vsub>;
using InstARM32Ldr = InstARM32LoadBase<InstARM32::Ldr>;
using InstARM32Ldrex = InstARM32LoadBase<InstARM32::Ldrex>;
using InstARM32Vldr1d = InstARM32LoadBase<InstARM32::Vldr1d>;
using InstARM32Vldr1q = InstARM32LoadBase<InstARM32::Vldr1q>;
using InstARM32Vzip = InstARM32ThreeAddrFP<InstARM32::Vzip>;
/// MovT leaves the bottom bits alone so dest is also a source. This helps
/// indicate that a previous MovW setting dest is not dead code.
using InstARM32Movt = InstARM32TwoAddrGPR<InstARM32::Movt>;
using InstARM32Movw = InstARM32UnaryopGPR<InstARM32::Movw, false>;
using InstARM32Clz = InstARM32UnaryopGPR<InstARM32::Clz, false>;
using InstARM32Mvn = InstARM32UnaryopGPR<InstARM32::Mvn, false>;
using InstARM32Rbit = InstARM32UnaryopGPR<InstARM32::Rbit, false>;
using InstARM32Rev = InstARM32UnaryopGPR<InstARM32::Rev, false>;
// Technically, the uxt{b,h} and sxt{b,h} instructions have a rotation operand
// as well (rotate source by 8, 16, 24 bits prior to extending), but we aren't
// using that for now, so just model as a Unaryop.
using InstARM32Sxt = InstARM32UnaryopGPR<InstARM32::Sxt, true>;
using InstARM32Uxt = InstARM32UnaryopGPR<InstARM32::Uxt, true>;
using InstARM32Vsqrt = InstARM32UnaryopFP<InstARM32::Vsqrt>;
using InstARM32Mla = InstARM32FourAddrGPR<InstARM32::Mla>;
using InstARM32Mls = InstARM32FourAddrGPR<InstARM32::Mls>;
using InstARM32Cmn = InstARM32CmpLike<InstARM32::Cmn>;
using InstARM32Cmp = InstARM32CmpLike<InstARM32::Cmp>;
using InstARM32Tst = InstARM32CmpLike<InstARM32::Tst>;

// InstARM32Label represents an intra-block label that is the target of an
// intra-block branch. The offset between the label and the branch must be fit
// in the instruction immediate (considered "near").
class InstARM32Label : public InstARM32 {
  InstARM32Label() = delete;
  InstARM32Label(const InstARM32Label &) = delete;
  InstARM32Label &operator=(const InstARM32Label &) = delete;

public:
  static InstARM32Label *create(Cfg *Func, TargetARM32 *Target) {
    return new (Func->allocate<InstARM32Label>()) InstARM32Label(Func, Target);
  }
  uint32_t getEmitInstCount() const override { return 0; }
  GlobalString getLabelName() const { return Name; }
  SizeT getNumber() const { return Number; }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  void setRelocOffset(RelocOffset *Value) { OffsetReloc = Value; }

private:
  InstARM32Label(Cfg *Func, TargetARM32 *Target);

  RelocOffset *OffsetReloc = nullptr;
  SizeT Number; // used for unique label generation.
  GlobalString Name;
};

/// Direct branch instruction.
class InstARM32Br : public InstARM32Pred {
  InstARM32Br() = delete;
  InstARM32Br(const InstARM32Br &) = delete;
  InstARM32Br &operator=(const InstARM32Br &) = delete;

public:
  /// Create a conditional branch to one of two nodes.
  static InstARM32Br *create(Cfg *Func, CfgNode *TargetTrue,
                             CfgNode *TargetFalse, CondARM32::Cond Predicate) {
    assert(Predicate != CondARM32::AL);
    constexpr InstARM32Label *NoLabel = nullptr;
    return new (Func->allocate<InstARM32Br>())
        InstARM32Br(Func, TargetTrue, TargetFalse, NoLabel, Predicate);
  }
  /// Create an unconditional branch to a node.
  static InstARM32Br *create(Cfg *Func, CfgNode *Target) {
    constexpr CfgNode *NoCondTarget = nullptr;
    constexpr InstARM32Label *NoLabel = nullptr;
    return new (Func->allocate<InstARM32Br>())
        InstARM32Br(Func, NoCondTarget, Target, NoLabel, CondARM32::AL);
  }
  /// Create a non-terminator conditional branch to a node, with a fallthrough
  /// to the next instruction in the current node. This is used for switch
  /// lowering.
  static InstARM32Br *create(Cfg *Func, CfgNode *Target,
                             CondARM32::Cond Predicate) {
    assert(Predicate != CondARM32::AL);
    constexpr CfgNode *NoUncondTarget = nullptr;
    constexpr InstARM32Label *NoLabel = nullptr;
    return new (Func->allocate<InstARM32Br>())
        InstARM32Br(Func, Target, NoUncondTarget, NoLabel, Predicate);
  }
  // Create a conditional intra-block branch (or unconditional, if
  // Condition==AL) to a label in the current block.
  static InstARM32Br *create(Cfg *Func, InstARM32Label *Label,
                             CondARM32::Cond Predicate) {
    constexpr CfgNode *NoCondTarget = nullptr;
    constexpr CfgNode *NoUncondTarget = nullptr;
    return new (Func->allocate<InstARM32Br>())
        InstARM32Br(Func, NoCondTarget, NoUncondTarget, Label, Predicate);
  }
  const CfgNode *getTargetTrue() const { return TargetTrue; }
  const CfgNode *getTargetFalse() const { return TargetFalse; }
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
    return getPredicate() == CondARM32::AL;
  }
  bool repointEdges(CfgNode *OldNode, CfgNode *NewNode) override;
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Br); }

private:
  InstARM32Br(Cfg *Func, const CfgNode *TargetTrue, const CfgNode *TargetFalse,
              const InstARM32Label *Label, CondARM32::Cond Predicate);

  const CfgNode *TargetTrue;
  const CfgNode *TargetFalse;
  const InstARM32Label *Label; // Intra-block branch target
};

/// Call instruction (bl/blx). Arguments should have already been pushed.
/// Technically bl and the register form of blx can be predicated, but we'll
/// leave that out until needed.
class InstARM32Call : public InstARM32 {
  InstARM32Call() = delete;
  InstARM32Call(const InstARM32Call &) = delete;
  InstARM32Call &operator=(const InstARM32Call &) = delete;

public:
  static InstARM32Call *create(Cfg *Func, Variable *Dest, Operand *CallTarget) {
    return new (Func->allocate<InstARM32Call>())
        InstARM32Call(Func, Dest, CallTarget);
  }
  Operand *getCallTarget() const { return getSrc(0); }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Call); }

private:
  InstARM32Call(Cfg *Func, Variable *Dest, Operand *CallTarget);
};

class InstARM32RegisterStackOp : public InstARM32 {
  InstARM32RegisterStackOp() = delete;
  InstARM32RegisterStackOp(const InstARM32RegisterStackOp &) = delete;
  InstARM32RegisterStackOp &
  operator=(const InstARM32RegisterStackOp &) = delete;

public:
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;

protected:
  InstARM32RegisterStackOp(Cfg *Func, InstKindARM32 Kind, SizeT Maxsrcs,
                           Variable *Dest)
      : InstARM32(Func, Kind, Maxsrcs, Dest) {}
  void emitUsingForm(const Cfg *Func, const EmitForm Form) const;
  void emitGPRsAsText(const Cfg *Func) const;
  void emitSRegsAsText(const Cfg *Func, const Variable *BaseReg,
                       SizeT Regcount) const;
  void emitSRegsOp(const Cfg *Func, const EmitForm, const Variable *BaseReg,
                   SizeT RegCount, SizeT InstIndex) const;
  virtual const char *getDumpOpcode() const { return getGPROpcode(); }
  virtual const char *getGPROpcode() const = 0;
  virtual const char *getSRegOpcode() const = 0;
  virtual Variable *getStackReg(SizeT Index) const = 0;
  virtual SizeT getNumStackRegs() const = 0;
  virtual void emitSingleGPR(const Cfg *Func, const EmitForm Form,
                             const Variable *Reg) const = 0;
  virtual void emitMultipleGPRs(const Cfg *Func, const EmitForm Form,
                                IValueT Registers) const = 0;
  virtual void emitSRegs(const Cfg *Func, const EmitForm Form,
                         const Variable *BaseReg, SizeT RegCount) const = 0;
};

/// Pops a list of registers. It may be a list of GPRs, or a list of VFP "s"
/// regs, but not both. In any case, the list must be sorted.
class InstARM32Pop final : public InstARM32RegisterStackOp {
  InstARM32Pop() = delete;
  InstARM32Pop(const InstARM32Pop &) = delete;
  InstARM32Pop &operator=(const InstARM32Pop &) = delete;

public:
  static InstARM32Pop *create(Cfg *Func, const VarList &Dests) {
    return new (Func->allocate<InstARM32Pop>()) InstARM32Pop(Func, Dests);
  }
  static bool classof(const Inst *Instr) { return isClassof(Instr, Pop); }

private:
  InstARM32Pop(Cfg *Func, const VarList &Dests);
  virtual const char *getGPROpcode() const final;
  virtual const char *getSRegOpcode() const final;
  Variable *getStackReg(SizeT Index) const final;
  SizeT getNumStackRegs() const final;
  void emitSingleGPR(const Cfg *Func, const EmitForm Form,
                     const Variable *Reg) const final;
  void emitMultipleGPRs(const Cfg *Func, const EmitForm Form,
                        IValueT Registers) const final;
  void emitSRegs(const Cfg *Func, const EmitForm Form, const Variable *BaseReg,
                 SizeT RegCount) const final;
  VarList Dests;
};

/// Pushes a list of registers. Just like Pop (see above), the list may be of
/// GPRs, or VFP "s" registers, but not both.
class InstARM32Push final : public InstARM32RegisterStackOp {
  InstARM32Push() = delete;
  InstARM32Push(const InstARM32Push &) = delete;
  InstARM32Push &operator=(const InstARM32Push &) = delete;

public:
  static InstARM32Push *create(Cfg *Func, const VarList &Srcs) {
    return new (Func->allocate<InstARM32Push>()) InstARM32Push(Func, Srcs);
  }
  static bool classof(const Inst *Instr) { return isClassof(Instr, Push); }

private:
  InstARM32Push(Cfg *Func, const VarList &Srcs);
  const char *getGPROpcode() const final;
  const char *getSRegOpcode() const final;
  Variable *getStackReg(SizeT Index) const final;
  SizeT getNumStackRegs() const final;
  void emitSingleGPR(const Cfg *Func, const EmitForm Form,
                     const Variable *Reg) const final;
  void emitMultipleGPRs(const Cfg *Func, const EmitForm Form,
                        IValueT Registers) const final;
  void emitSRegs(const Cfg *Func, const EmitForm Form, const Variable *BaseReg,
                 SizeT RegCount) const final;
};

/// Ret pseudo-instruction. This is actually a "bx" instruction with an "lr"
/// register operand, but epilogue lowering will search for a Ret instead of a
/// generic "bx". This instruction also takes a Source operand (for non-void
/// returning functions) for liveness analysis, though a FakeUse before the ret
/// would do just as well.
///
/// NOTE: Even though "bx" can be predicated, for now leave out the predication
/// since it's not yet known to be useful for Ret. That may complicate finding
/// the terminator instruction if it's not guaranteed to be executed.
class InstARM32Ret : public InstARM32 {
  InstARM32Ret() = delete;
  InstARM32Ret(const InstARM32Ret &) = delete;
  InstARM32Ret &operator=(const InstARM32Ret &) = delete;

public:
  static InstARM32Ret *create(Cfg *Func, Variable *LR,
                              Variable *Source = nullptr) {
    return new (Func->allocate<InstARM32Ret>()) InstARM32Ret(Func, LR, Source);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Ret); }

private:
  InstARM32Ret(Cfg *Func, Variable *LR, Variable *Source);
};

/// Store instruction. It's important for liveness that there is no Dest operand
/// (OperandARM32Mem instead of Dest Variable).
class InstARM32Str final : public InstARM32Pred {
  InstARM32Str() = delete;
  InstARM32Str(const InstARM32Str &) = delete;
  InstARM32Str &operator=(const InstARM32Str &) = delete;

public:
  /// Value must be a register.
  static InstARM32Str *create(Cfg *Func, Variable *Value, OperandARM32Mem *Mem,
                              CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32Str>())
        InstARM32Str(Func, Value, Mem, Predicate);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Str); }

private:
  InstARM32Str(Cfg *Func, Variable *Value, OperandARM32Mem *Mem,
               CondARM32::Cond Predicate);
};

/// Exclusive Store instruction. Like its non-exclusive sibling, it's important
/// for liveness that there is no Dest operand (OperandARM32Mem instead of Dest
/// Variable).
class InstARM32Strex final : public InstARM32Pred {
  InstARM32Strex() = delete;
  InstARM32Strex(const InstARM32Strex &) = delete;
  InstARM32Strex &operator=(const InstARM32Strex &) = delete;

public:
  /// Value must be a register.
  static InstARM32Strex *create(Cfg *Func, Variable *Dest, Variable *Value,
                                OperandARM32Mem *Mem,
                                CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32Strex>())
        InstARM32Strex(Func, Dest, Value, Mem, Predicate);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Strex); }

private:
  InstARM32Strex(Cfg *Func, Variable *Dest, Variable *Value,
                 OperandARM32Mem *Mem, CondARM32::Cond Predicate);
};

/// Sub-vector store instruction. It's important for liveness that there is no
///  Dest operand (OperandARM32Mem instead of Dest Variable).
class InstARM32Vstr1 final : public InstARM32Pred {
  InstARM32Vstr1() = delete;
  InstARM32Vstr1(const InstARM32Vstr1 &) = delete;
  InstARM32Vstr1 &operator=(const InstARM32Vstr1 &) = delete;

public:
  /// Value must be a register.
  static InstARM32Vstr1 *create(Cfg *Func, Variable *Value,
                                OperandARM32Mem *Mem, CondARM32::Cond Predicate,
                                SizeT Size) {
    return new (Func->allocate<InstARM32Vstr1>())
        InstARM32Vstr1(Func, Value, Mem, Predicate, Size);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Vstr1); }

private:
  InstARM32Vstr1(Cfg *Func, Variable *Value, OperandARM32Mem *Mem,
                 CondARM32::Cond Predicate, SizeT Size);

  SizeT Size;
};

/// Vector element duplication/replication instruction.
class InstARM32Vdup final : public InstARM32Pred {
  InstARM32Vdup() = delete;
  InstARM32Vdup(const InstARM32Vdup &) = delete;
  InstARM32Vdup &operator=(const InstARM32Vdup &) = delete;

public:
  /// Value must be a register.
  static InstARM32Vdup *create(Cfg *Func, Variable *Dest, Variable *Src,
                               IValueT Idx) {
    return new (Func->allocate<InstARM32Vdup>())
        InstARM32Vdup(Func, Dest, Src, Idx);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Vdup); }

private:
  InstARM32Vdup(Cfg *Func, Variable *Dest, Variable *Src, IValueT Idx);

  const IValueT Idx;
};

class InstARM32Trap : public InstARM32 {
  InstARM32Trap() = delete;
  InstARM32Trap(const InstARM32Trap &) = delete;
  InstARM32Trap &operator=(const InstARM32Trap &) = delete;

public:
  static InstARM32Trap *create(Cfg *Func) {
    return new (Func->allocate<InstARM32Trap>()) InstARM32Trap(Func);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Trap); }

private:
  explicit InstARM32Trap(Cfg *Func);
};

/// Unsigned Multiply Long: d.lo, d.hi := x * y
class InstARM32Umull : public InstARM32Pred {
  InstARM32Umull() = delete;
  InstARM32Umull(const InstARM32Umull &) = delete;
  InstARM32Umull &operator=(const InstARM32Umull &) = delete;

public:
  /// Everything must be a register.
  static InstARM32Umull *create(Cfg *Func, Variable *DestLo, Variable *DestHi,
                                Variable *Src0, Variable *Src1,
                                CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32Umull>())
        InstARM32Umull(Func, DestLo, DestHi, Src0, Src1, Predicate);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Umull); }

private:
  InstARM32Umull(Cfg *Func, Variable *DestLo, Variable *DestHi, Variable *Src0,
                 Variable *Src1, CondARM32::Cond Predicate);

  Variable *DestHi;
};

/// Handles fp2int, int2fp, and fp2fp conversions.
class InstARM32Vcvt final : public InstARM32Pred {
  InstARM32Vcvt() = delete;
  InstARM32Vcvt(const InstARM32Vcvt &) = delete;
  InstARM32Vcvt &operator=(const InstARM32Vcvt &) = delete;

public:
  enum VcvtVariant {
    S2si,
    S2ui,
    Si2s,
    Ui2s,
    D2si,
    D2ui,
    Si2d,
    Ui2d,
    S2d,
    D2s,
    Vs2si,
    Vs2ui,
    Vsi2s,
    Vui2s,
  };
  static InstARM32Vcvt *create(Cfg *Func, Variable *Dest, Variable *Src,
                               VcvtVariant Variant, CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32Vcvt>())
        InstARM32Vcvt(Func, Dest, Src, Variant, Predicate);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Vcvt); }

private:
  InstARM32Vcvt(Cfg *Func, Variable *Dest, Variable *Src, VcvtVariant Variant,
                CondARM32::Cond Predicate);

  const VcvtVariant Variant;
};

/// Handles (some of) vmov's various formats.
class InstARM32Mov final : public InstARM32Pred {
  InstARM32Mov() = delete;
  InstARM32Mov(const InstARM32Mov &) = delete;
  InstARM32Mov &operator=(const InstARM32Mov &) = delete;

public:
  static InstARM32Mov *create(Cfg *Func, Variable *Dest, Operand *Src,
                              CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32Mov>())
        InstARM32Mov(Func, Dest, Src, Predicate);
  }
  bool isRedundantAssign() const override {
    return !isMultiDest() && !isMultiSource() &&
           getPredicate() == CondARM32::AL &&
           checkForRedundantAssign(getDest(), getSrc(0));
  }
  bool isVarAssign() const override { return llvm::isa<Variable>(getSrc(0)); }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Mov); }

  bool isMultiDest() const { return DestHi != nullptr; }

  bool isMultiSource() const {
    assert(getSrcSize() == 1 || getSrcSize() == 2);
    return getSrcSize() == 2;
  }

  Variable *getDestHi() const { return DestHi; }

private:
  InstARM32Mov(Cfg *Func, Variable *Dest, Operand *Src,
               CondARM32::Cond Predicate);
  void emitMultiDestSingleSource(const Cfg *Func) const;
  void emitSingleDestMultiSource(const Cfg *Func) const;
  void emitSingleDestSingleSource(const Cfg *Func) const;

  Variable *DestHi = nullptr;
};

/// Generates vmov Rd, Dn[x] instructions, and their related floating point
/// versions.
class InstARM32Extract final : public InstARM32Pred {
  InstARM32Extract() = delete;
  InstARM32Extract(const InstARM32Extract &) = delete;
  InstARM32Extract &operator=(const InstARM32Extract &) = delete;

public:
  static InstARM32Extract *create(Cfg *Func, Variable *Dest, Variable *Src0,
                                  uint32_t Index, CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32Extract>())
        InstARM32Extract(Func, Dest, Src0, Index, Predicate);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Extract); }

private:
  InstARM32Extract(Cfg *Func, Variable *Dest, Variable *Src0, uint32_t Index,
                   CondARM32::Cond Predicate)
      : InstARM32Pred(Func, InstARM32::Extract, 1, Dest, Predicate),
        Index(Index) {
    assert(Index < typeNumElements(Src0->getType()));
    addSource(Src0);
  }

  const uint32_t Index;
};

/// Generates vmov Dn[x], Rd instructions, and their related floating point
/// versions.
class InstARM32Insert final : public InstARM32Pred {
  InstARM32Insert() = delete;
  InstARM32Insert(const InstARM32Insert &) = delete;
  InstARM32Insert &operator=(const InstARM32Insert &) = delete;

public:
  static InstARM32Insert *create(Cfg *Func, Variable *Dest, Variable *Src0,
                                 uint32_t Index, CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32Insert>())
        InstARM32Insert(Func, Dest, Src0, Index, Predicate);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Insert); }

private:
  InstARM32Insert(Cfg *Func, Variable *Dest, Variable *Src0, uint32_t Index,
                  CondARM32::Cond Predicate)
      : InstARM32Pred(Func, InstARM32::Insert, 1, Dest, Predicate),
        Index(Index) {
    assert(Index < typeNumElements(Dest->getType()));
    addSource(Src0);
  }

  const uint32_t Index;
};

class InstARM32Vcmp final : public InstARM32Pred {
  InstARM32Vcmp() = delete;
  InstARM32Vcmp(const InstARM32Vcmp &) = delete;
  InstARM32Vcmp &operator=(const InstARM32Vcmp &) = delete;

public:
  static InstARM32Vcmp *create(Cfg *Func, Variable *Src0, Variable *Src1,
                               CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32Vcmp>())
        InstARM32Vcmp(Func, Src0, Src1, Predicate);
  }
  static InstARM32Vcmp *create(Cfg *Func, Variable *Src0,
                               OperandARM32FlexFpZero *Src1,
                               CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32Vcmp>())
        InstARM32Vcmp(Func, Src0, Src1, Predicate);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Vcmp); }

private:
  InstARM32Vcmp(Cfg *Func, Variable *Src0, Operand *Src1,
                CondARM32::Cond Predicate);
};

/// Copies the FP Status and Control Register the core flags.
class InstARM32Vmrs final : public InstARM32Pred {
  InstARM32Vmrs() = delete;
  InstARM32Vmrs(const InstARM32Vmrs &) = delete;
  InstARM32Vmrs &operator=(const InstARM32Vmrs &) = delete;

public:
  static InstARM32Vmrs *create(Cfg *Func, CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32Vmrs>()) InstARM32Vmrs(Func, Predicate);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Vmrs); }

private:
  InstARM32Vmrs(Cfg *Func, CondARM32::Cond Predicate);
};

class InstARM32Vabs final : public InstARM32Pred {
  InstARM32Vabs() = delete;
  InstARM32Vabs(const InstARM32Vabs &) = delete;
  InstARM32Vabs &operator=(const InstARM32Vabs &) = delete;

public:
  static InstARM32Vabs *create(Cfg *Func, Variable *Dest, Variable *Src,
                               CondARM32::Cond Predicate) {
    return new (Func->allocate<InstARM32Vabs>())
        InstARM32Vabs(Func, Dest, Src, Predicate);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Vabs); }

private:
  InstARM32Vabs(Cfg *Func, Variable *Dest, Variable *Src,
                CondARM32::Cond Predicate);
};

class InstARM32Dmb final : public InstARM32Pred {
  InstARM32Dmb() = delete;
  InstARM32Dmb(const InstARM32Dmb &) = delete;
  InstARM32Dmb &operator=(const InstARM32Dmb &) = delete;

public:
  static InstARM32Dmb *create(Cfg *Func) {
    return new (Func->allocate<InstARM32Dmb>()) InstARM32Dmb(Func);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Dmb); }

private:
  explicit InstARM32Dmb(Cfg *Func);
};

class InstARM32Nop final : public InstARM32Pred {
  InstARM32Nop() = delete;
  InstARM32Nop(const InstARM32Nop &) = delete;
  InstARM32Nop &operator=(const InstARM32Nop &) = delete;

public:
  static InstARM32Nop *create(Cfg *Func) {
    return new (Func->allocate<InstARM32Nop>()) InstARM32Nop(Func);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Nop); }

private:
  explicit InstARM32Nop(Cfg *Func);
};

// Declare partial template specializations of emit() methods that already have
// default implementations. Without this, there is the possibility of ODR
// violations and link errors.

template <> void InstARM32Ldr::emit(const Cfg *Func) const;
template <> void InstARM32Movw::emit(const Cfg *Func) const;
template <> void InstARM32Movt::emit(const Cfg *Func) const;
template <> void InstARM32Vldr1d::emit(const Cfg *Func) const;
template <> void InstARM32Vldr1q::emit(const Cfg *Func) const;

// Two-addr ops
template <> constexpr const char *InstARM32Movt::Opcode = "movt";
// Unary ops
template <> constexpr const char *InstARM32Movw::Opcode = "movw";
template <> constexpr const char *InstARM32Clz::Opcode = "clz";
template <> constexpr const char *InstARM32Mvn::Opcode = "mvn";
template <> constexpr const char *InstARM32Rbit::Opcode = "rbit";
template <> constexpr const char *InstARM32Rev::Opcode = "rev";
template <>
constexpr const char *InstARM32Sxt::Opcode = "sxt"; // still requires b/h
template <>
constexpr const char *InstARM32Uxt::Opcode = "uxt"; // still requires b/h
// FP
template <> constexpr const char *InstARM32Vsqrt::Opcode = "vsqrt";
// Mov-like ops
template <> constexpr const char *InstARM32Ldr::Opcode = "ldr";
template <> constexpr const char *InstARM32Ldrex::Opcode = "ldrex";
template <> constexpr const char *InstARM32Vldr1d::Opcode = "vldr1d";
template <> constexpr const char *InstARM32Vldr1q::Opcode = "vldr1q";
// Three-addr ops
template <> constexpr const char *InstARM32Adc::Opcode = "adc";
template <> constexpr const char *InstARM32Add::Opcode = "add";
template <> constexpr const char *InstARM32And::Opcode = "and";
template <> constexpr const char *InstARM32Asr::Opcode = "asr";
template <> constexpr const char *InstARM32Bic::Opcode = "bic";
template <> constexpr const char *InstARM32Eor::Opcode = "eor";
template <> constexpr const char *InstARM32Lsl::Opcode = "lsl";
template <> constexpr const char *InstARM32Lsr::Opcode = "lsr";
template <> constexpr const char *InstARM32Mul::Opcode = "mul";
template <> constexpr const char *InstARM32Orr::Opcode = "orr";
template <> constexpr const char *InstARM32Rsb::Opcode = "rsb";
template <> constexpr const char *InstARM32Rsc::Opcode = "rsc";
template <> constexpr const char *InstARM32Sbc::Opcode = "sbc";
template <> constexpr const char *InstARM32Sdiv::Opcode = "sdiv";
template <> constexpr const char *InstARM32Sub::Opcode = "sub";
template <> constexpr const char *InstARM32Udiv::Opcode = "udiv";
// FP
template <> constexpr const char *InstARM32Vadd::Opcode = "vadd";
template <> constexpr const char *InstARM32Vand::Opcode = "vand";
template <> constexpr const char *InstARM32Vbsl::Opcode = "vbsl";
template <> constexpr const char *InstARM32Vceq::Opcode = "vceq";
template <>
constexpr const char *InstARM32ThreeAddrFP<InstARM32::Vcge>::Opcode = "vcge";
template <>
constexpr const char *InstARM32ThreeAddrFP<InstARM32::Vcgt>::Opcode = "vcgt";
template <> constexpr const char *InstARM32Vdiv::Opcode = "vdiv";
template <> constexpr const char *InstARM32Veor::Opcode = "veor";
template <> constexpr const char *InstARM32Vmla::Opcode = "vmla";
template <> constexpr const char *InstARM32Vmls::Opcode = "vmls";
template <> constexpr const char *InstARM32Vmul::Opcode = "vmul";
template <> constexpr const char *InstARM32Vmvn::Opcode = "vmvn";
template <> constexpr const char *InstARM32Vmovl::Opcode = "vmovl";
template <> constexpr const char *InstARM32Vmovh::Opcode = "vmovh";
template <> constexpr const char *InstARM32Vmovhl::Opcode = "vmovhl";
template <> constexpr const char *InstARM32Vmovlh::Opcode = "vmovlh";
template <> constexpr const char *InstARM32Vorr::Opcode = "vorr";
template <>
constexpr const char *InstARM32UnaryopFP<InstARM32::Vneg>::Opcode = "vneg";
template <>
constexpr const char *InstARM32ThreeAddrFP<InstARM32::Vshl>::Opcode = "vshl";
template <>
constexpr const char *InstARM32ThreeAddrFP<InstARM32::Vshr>::Opcode = "vshr";
template <> constexpr const char *InstARM32Vsub::Opcode = "vsub";
template <>
constexpr const char *InstARM32ThreeAddrFP<InstARM32::Vqadd>::Opcode = "vqadd";
template <>
constexpr const char *InstARM32ThreeAddrFP<InstARM32::Vqsub>::Opcode = "vqsub";
template <>
constexpr const char *InstARM32ThreeAddrFP<InstARM32::Vqmovn2>::Opcode =
    "vqmovn2";
template <>
constexpr const char *InstARM32ThreeAddrFP<InstARM32::Vmulh>::Opcode = "vmulh";
template <>
constexpr const char *InstARM32ThreeAddrFP<InstARM32::Vmlap>::Opcode = "vmlap";
template <>
constexpr const char *InstARM32ThreeAddrFP<InstARM32::Vzip>::Opcode = "vzip";
// Four-addr ops
template <> constexpr const char *InstARM32Mla::Opcode = "mla";
template <> constexpr const char *InstARM32Mls::Opcode = "mls";
// Cmp-like ops
template <> constexpr const char *InstARM32Cmn::Opcode = "cmn";
template <> constexpr const char *InstARM32Cmp::Opcode = "cmp";
template <> constexpr const char *InstARM32Tst::Opcode = "tst";

} // end of namespace ARM32
} // end of namespace Ice

#endif // SUBZERO_SRC_ICEINSTARM32_H
