//===- subzero/src/IceInstMIPS32.h - MIPS32 machine instrs --*- C++ -*-----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the InstMIPS32 and OperandMIPS32 classes and their
/// subclasses.
///
/// This represents the machine instructions and operands used for MIPS32 code
/// selection.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEINSTMIPS32_H
#define SUBZERO_SRC_ICEINSTMIPS32_H

#include "IceConditionCodesMIPS32.h"
#include "IceDefs.h"
#include "IceInst.h"
#include "IceInstMIPS32.def"
#include "IceOperand.h"

namespace Ice {
namespace MIPS32 {

enum RelocOp { RO_No, RO_Hi, RO_Lo, RO_Jal };
enum Int64Part { Int64_Hi, Int64_Lo };

inline void emitRelocOp(Ostream &Str, RelocOp Reloc) {
  switch (Reloc) {
  default:
    break;
  case RO_Hi:
    Str << "%hi";
    break;
  case RO_Lo:
    Str << "%lo";
    break;
  }
}

class TargetMIPS32;

/// OperandMips32 extends the Operand hierarchy.
//
class OperandMIPS32 : public Operand {
  OperandMIPS32() = delete;
  OperandMIPS32(const OperandMIPS32 &) = delete;
  OperandMIPS32 &operator=(const OperandMIPS32 &) = delete;

public:
  enum OperandKindMIPS32 {
    k__Start = Operand::kTarget,
    kFCC,
    kMem,
  };

  using Operand::dump;
  void dump(const Cfg *, Ostream &Str) const override {
    if (BuildDefs::dump())
      Str << "<OperandMIPS32>";
  }

protected:
  OperandMIPS32(OperandKindMIPS32 Kind, Type Ty)
      : Operand(static_cast<OperandKind>(Kind), Ty) {}
};

class OperandMIPS32FCC : public OperandMIPS32 {
  OperandMIPS32FCC() = delete;
  OperandMIPS32FCC(const OperandMIPS32FCC &) = delete;
  OperandMIPS32FCC &operator=(const OperandMIPS32FCC &) = delete;

public:
  using FCC = enum { FCC0 = 0, FCC1, FCC2, FCC3, FCC4, FCC5, FCC6, FCC7 };
  static OperandMIPS32FCC *create(Cfg *Func, OperandMIPS32FCC::FCC FCC) {
    return new (Func->allocate<OperandMIPS32FCC>()) OperandMIPS32FCC(FCC);
  }

  OperandMIPS32FCC::FCC getFCC() const { return FpCondCode; }

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrEmit();
    Str << "$fcc" << static_cast<uint16_t>(FpCondCode);
  }

  static bool classof(const Operand *Operand) {
    return Operand->getKind() == static_cast<OperandKind>(kFCC);
  }

  void dump(const Cfg *Func, Ostream &Str) const override {
    if (!BuildDefs::dump())
      return;
    (void)Func;
    Str << "$fcc" << static_cast<uint16_t>(FpCondCode);
  }

private:
  OperandMIPS32FCC(OperandMIPS32FCC::FCC CC)
      : OperandMIPS32(kFCC, IceType_i32), FpCondCode(CC){};

  const OperandMIPS32FCC::FCC FpCondCode;
};

class OperandMIPS32Mem : public OperandMIPS32 {
  OperandMIPS32Mem() = delete;
  OperandMIPS32Mem(const OperandMIPS32Mem &) = delete;
  OperandMIPS32Mem &operator=(const OperandMIPS32Mem &) = delete;

public:
  /// Memory operand addressing mode.
  /// The enum value also carries the encoding.
  // TODO(jvoung): unify with the assembler.
  enum AddrMode { Offset };

  /// NOTE: The Variable-typed operands have to be registers.
  ///
  /// Reg + Imm. The Immediate actually has a limited number of bits
  /// for encoding, so check canHoldOffset first. It cannot handle
  /// general Constant operands like ConstantRelocatable, since a relocatable
  /// can potentially take up too many bits.
  static OperandMIPS32Mem *create(Cfg *Func, Type Ty, Variable *Base,
                                  Operand *ImmOffset, AddrMode Mode = Offset) {
    return new (Func->allocate<OperandMIPS32Mem>())
        OperandMIPS32Mem(Func, Ty, Base, ImmOffset, Mode);
  }

  Variable *getBase() const { return Base; }
  Operand *getOffset() const { return ImmOffset; }
  AddrMode getAddrMode() const { return Mode; }

  void emit(const Cfg *Func) const override;
  using OperandMIPS32::dump;

  static bool classof(const Operand *Operand) {
    return Operand->getKind() == static_cast<OperandKind>(kMem);
  }

  /// Return true if a load/store instruction for an element of type Ty
  /// can encode the Offset directly in the immediate field of the 32-bit
  /// MIPS instruction. For some types, if the load is Sign extending, then
  /// the range is reduced.
  static bool canHoldOffset(Type Ty, bool SignExt, int32_t Offset);

  void dump(const Cfg *Func, Ostream &Str) const override {
    if (!BuildDefs::dump())
      return;
    Str << "[";
    if (Func)
      getBase()->dump(Func);
    else
      getBase()->dump(Str);
    Str << ", ";
    getOffset()->dump(Func, Str);
    Str << "] AddrMode==";
    if (getAddrMode() == Offset) {
      Str << "Offset";
    } else {
      Str << "Unknown";
    }
  }

private:
  OperandMIPS32Mem(Cfg *Func, Type Ty, Variable *Base, Operand *ImmOffset,
                   AddrMode Mode);

  Variable *Base;
  Operand *const ImmOffset;
  const AddrMode Mode;
};

/// Base class for Mips instructions.
class InstMIPS32 : public InstTarget {
  InstMIPS32() = delete;
  InstMIPS32(const InstMIPS32 &) = delete;
  InstMIPS32 &operator=(const InstMIPS32 &) = delete;

public:
  enum InstKindMIPS32 {
    k__Start = Inst::Target,
    Abs_d,
    Abs_s,
    Add,
    Add_d,
    Add_s,
    Addi,
    Addiu,
    Addu,
    And,
    Andi,
    Br,
    C_eq_d,
    C_eq_s,
    C_ole_d,
    C_ole_s,
    C_olt_d,
    C_olt_s,
    C_ueq_d,
    C_ueq_s,
    C_ule_d,
    C_ule_s,
    C_ult_d,
    C_ult_s,
    C_un_d,
    C_un_s,
    Call,
    Clz,
    Cvt_d_l,
    Cvt_d_s,
    Cvt_d_w,
    Cvt_s_d,
    Cvt_s_l,
    Cvt_s_w,
    Div,
    Div_d,
    Div_s,
    Divu,
    La,
    Label,
    Ldc1,
    Ll,
    Lui,
    Lw,
    Lwc1,
    Mfc1,
    Mfhi,
    Mflo,
    Mov, // actually a pseudo op for addi rd, rs, 0
    Mov_fp,
    Mov_d,
    Mov_s,
    Movf,
    Movn,
    Movn_d,
    Movn_s,
    Movt,
    Movz,
    Movz_d,
    Movz_s,
    Mtc1,
    Mthi,
    Mtlo,
    Mul,
    Mul_d,
    Mul_s,
    Mult,
    Multu,
    Nor,
    Or,
    Ori,
    Ret,
    Sc,
    Sdc1,
    Sll,
    Sllv,
    Slt,
    Slti,
    Sltiu,
    Sltu,
    Sra,
    Srav,
    Srl,
    Srlv,
    Sqrt_d,
    Sqrt_s,
    Sub,
    Sub_d,
    Sub_s,
    Subu,
    Sw,
    Swc1,
    Sync,
    Teq,
    Trunc_l_d,
    Trunc_l_s,
    Trunc_w_d,
    Trunc_w_s,
    Xor,
    Xori
  };

  static constexpr size_t InstSize = sizeof(uint32_t);

  static const char *getWidthString(Type Ty);

  CondMIPS32::Cond getOppositeCondition(CondMIPS32::Cond Cond);

  void dump(const Cfg *Func) const override;

  void dumpOpcode(Ostream &Str, const char *Opcode, Type Ty) const {
    Str << Opcode << "." << Ty;
  }

  // TODO(rkotler): while branching is not implemented
  bool repointEdges(CfgNode *, CfgNode *) override { return true; }

  /// Shared emit routines for common forms of instructions.
  static void emitUnaryopGPR(const char *Opcode, const InstMIPS32 *Inst,
                             const Cfg *Func);
  static void emitUnaryopGPRFLoHi(const char *Opcode, const InstMIPS32 *Inst,
                                  const Cfg *Func);
  static void emitUnaryopGPRTLoHi(const char *Opcode, const InstMIPS32 *Inst,
                                  const Cfg *Func);
  static void emitTwoAddr(const char *Opcode, const InstMIPS32 *Inst,
                          const Cfg *Func);
  static void emitThreeAddr(const char *Opcode, const InstMIPS32 *Inst,
                            const Cfg *Func);
  static void emitThreeAddrLoHi(const char *Opcode, const InstMIPS32 *Inst,
                                const Cfg *Func);

protected:
  InstMIPS32(Cfg *Func, InstKindMIPS32 Kind, SizeT Maxsrcs, Variable *Dest)
      : InstTarget(Func, static_cast<InstKind>(Kind), Maxsrcs, Dest) {}
  static bool isClassof(const Inst *Inst, InstKindMIPS32 MyKind) {
    return Inst->getKind() == static_cast<InstKind>(MyKind);
  }
};

/// Ret pseudo-instruction. This is actually a "jr" instruction with an "ra"
/// register operand, but epilogue lowering will search for a Ret instead of a
/// generic "jr". This instruction also takes a Source operand (for non-void
/// returning functions) for liveness analysis, though a FakeUse before the ret
/// would do just as well.
// TODO(reed kotler): This needs was take from the ARM port and needs to be
// scrubbed in the future.
class InstMIPS32Ret : public InstMIPS32 {

  InstMIPS32Ret() = delete;
  InstMIPS32Ret(const InstMIPS32Ret &) = delete;
  InstMIPS32Ret &operator=(const InstMIPS32Ret &) = delete;

public:
  static InstMIPS32Ret *create(Cfg *Func, Variable *RA,
                               Variable *Source = nullptr) {
    return new (Func->allocate<InstMIPS32Ret>())
        InstMIPS32Ret(Func, RA, Source);
  }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Ret); }

private:
  InstMIPS32Ret(Cfg *Func, Variable *RA, Variable *Source);
};

/// Instructions of the form x := op(y).
template <InstMIPS32::InstKindMIPS32 K>
class InstMIPS32UnaryopGPR : public InstMIPS32 {
  InstMIPS32UnaryopGPR() = delete;
  InstMIPS32UnaryopGPR(const InstMIPS32UnaryopGPR &) = delete;
  InstMIPS32UnaryopGPR &operator=(const InstMIPS32UnaryopGPR &) = delete;

public:
  static InstMIPS32UnaryopGPR *create(Cfg *Func, Variable *Dest, Operand *Src,
                                      RelocOp Reloc = RO_No) {
    return new (Func->allocate<InstMIPS32UnaryopGPR>())
        InstMIPS32UnaryopGPR(Func, Dest, Src, Reloc);
  }
  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    emitUnaryopGPR(Opcode, this, Func);
  }
  void emitIAS(const Cfg *Func) const override {
    (void)Func;
    llvm_unreachable("Not yet implemented");
  }
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpOpcode(Str, Opcode, getDest()->getType());
    Str << " ";
    dumpDest(Func);
    Str << ", ";
    dumpSources(Func);
  }
  static bool classof(const Inst *Inst) { return isClassof(Inst, K); }

protected:
  InstMIPS32UnaryopGPR(Cfg *Func, Variable *Dest, Operand *Src,
                       RelocOp Reloc = RO_No)
      : InstMIPS32(Func, K, 1, Dest), Reloc(Reloc) {
    addSource(Src);
  }

private:
  static const char *const Opcode;
  const RelocOp Reloc;
};

/// Instructions of the form opcode reg, reg.
template <InstMIPS32::InstKindMIPS32 K>
class InstMIPS32TwoAddrFPR : public InstMIPS32 {
  InstMIPS32TwoAddrFPR() = delete;
  InstMIPS32TwoAddrFPR(const InstMIPS32TwoAddrFPR &) = delete;
  InstMIPS32TwoAddrFPR &operator=(const InstMIPS32TwoAddrFPR &) = delete;

public:
  static InstMIPS32TwoAddrFPR *create(Cfg *Func, Variable *Dest,
                                      Variable *Src0) {
    return new (Func->allocate<InstMIPS32TwoAddrFPR>())
        InstMIPS32TwoAddrFPR(Func, Dest, Src0);
  }
  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    emitTwoAddr(Opcode, this, Func);
  }
  void emitIAS(const Cfg *Func) const override {
    (void)Func;
    llvm_unreachable("Not yet implemented");
  }

  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpDest(Func);
    Str << " = ";
    dumpOpcode(Str, Opcode, getDest()->getType());
    Str << " ";
    dumpSources(Func);
  }
  static bool classof(const Inst *Inst) { return isClassof(Inst, K); }

private:
  InstMIPS32TwoAddrFPR(Cfg *Func, Variable *Dest, Variable *Src0)
      : InstMIPS32(Func, K, 1, Dest) {
    addSource(Src0);
  }

  static const char *const Opcode;
};

/// Instructions of the form opcode reg, reg.
template <InstMIPS32::InstKindMIPS32 K>
class InstMIPS32TwoAddrGPR : public InstMIPS32 {
  InstMIPS32TwoAddrGPR() = delete;
  InstMIPS32TwoAddrGPR(const InstMIPS32TwoAddrGPR &) = delete;
  InstMIPS32TwoAddrGPR &operator=(const InstMIPS32TwoAddrGPR &) = delete;

public:
  static InstMIPS32TwoAddrGPR *create(Cfg *Func, Variable *Dest,
                                      Variable *Src0) {
    return new (Func->allocate<InstMIPS32TwoAddrGPR>())
        InstMIPS32TwoAddrGPR(Func, Dest, Src0);
  }
  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    emitTwoAddr(Opcode, this, Func);
  }
  void emitIAS(const Cfg *Func) const override {
    (void)Func;
    llvm_unreachable("Not yet implemented");
  }

  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpDest(Func);
    Str << " = ";
    dumpOpcode(Str, Opcode, getDest()->getType());
    Str << " ";
    dumpSources(Func);
  }
  static bool classof(const Inst *Inst) { return isClassof(Inst, K); }

private:
  InstMIPS32TwoAddrGPR(Cfg *Func, Variable *Dest, Variable *Src0)
      : InstMIPS32(Func, K, 1, Dest) {
    addSource(Src0);
  }

  static const char *const Opcode;
};

/// Instructions of the form x := y op z. May have the side-effect of setting
/// status flags.
template <InstMIPS32::InstKindMIPS32 K>
class InstMIPS32ThreeAddrFPR : public InstMIPS32 {
  InstMIPS32ThreeAddrFPR() = delete;
  InstMIPS32ThreeAddrFPR(const InstMIPS32ThreeAddrFPR &) = delete;
  InstMIPS32ThreeAddrFPR &operator=(const InstMIPS32ThreeAddrFPR &) = delete;

public:
  /// Create an ordinary binary-op instruction like add, and sub. Dest and Src1
  /// must be registers.
  static InstMIPS32ThreeAddrFPR *create(Cfg *Func, Variable *Dest,
                                        Variable *Src0, Variable *Src1) {
    return new (Func->allocate<InstMIPS32ThreeAddrFPR>())
        InstMIPS32ThreeAddrFPR(Func, Dest, Src0, Src1);
  }
  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    emitThreeAddr(Opcode, this, Func);
  }
  void emitIAS(const Cfg *Func) const override {
    (void)Func;
    llvm_unreachable("Not yet implemented");
  }

  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpDest(Func);
    Str << " = ";
    dumpOpcode(Str, Opcode, getDest()->getType());
    Str << " ";
    dumpSources(Func);
  }
  static bool classof(const Inst *Inst) { return isClassof(Inst, K); }

private:
  InstMIPS32ThreeAddrFPR(Cfg *Func, Variable *Dest, Variable *Src0,
                         Variable *Src1)
      : InstMIPS32(Func, K, 2, Dest) {
    addSource(Src0);
    addSource(Src1);
  }

  static const char *const Opcode;
};

/// Instructions of the form x := y op z. May have the side-effect of setting
/// status flags.
template <InstMIPS32::InstKindMIPS32 K>
class InstMIPS32ThreeAddrGPR : public InstMIPS32 {
  InstMIPS32ThreeAddrGPR() = delete;
  InstMIPS32ThreeAddrGPR(const InstMIPS32ThreeAddrGPR &) = delete;
  InstMIPS32ThreeAddrGPR &operator=(const InstMIPS32ThreeAddrGPR &) = delete;

public:
  /// Create an ordinary binary-op instruction like add, and sub. Dest and Src1
  /// must be registers.
  static InstMIPS32ThreeAddrGPR *create(Cfg *Func, Variable *Dest,
                                        Variable *Src0, Variable *Src1) {
    return new (Func->allocate<InstMIPS32ThreeAddrGPR>())
        InstMIPS32ThreeAddrGPR(Func, Dest, Src0, Src1);
  }
  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    emitThreeAddr(Opcode, this, Func);
  }
  void emitIAS(const Cfg *Func) const override {
    (void)Func;
    llvm_unreachable("Not yet implemented");
  }

  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpDest(Func);
    Str << " = ";
    dumpOpcode(Str, Opcode, getDest()->getType());
    Str << " ";
    dumpSources(Func);
  }
  static bool classof(const Inst *Inst) { return isClassof(Inst, K); }

private:
  InstMIPS32ThreeAddrGPR(Cfg *Func, Variable *Dest, Variable *Src0,
                         Variable *Src1)
      : InstMIPS32(Func, K, 2, Dest) {
    addSource(Src0);
    addSource(Src1);
  }

  static const char *const Opcode;
};

// InstMIPS32Load represents instructions which loads data from memory
// Its format is "OPCODE GPR, OFFSET(BASE GPR)"
template <InstMIPS32::InstKindMIPS32 K>
class InstMIPS32Load : public InstMIPS32 {
  InstMIPS32Load() = delete;
  InstMIPS32Load(const InstMIPS32Load &) = delete;
  InstMIPS32Load &operator=(const InstMIPS32Load &) = delete;

public:
  static InstMIPS32Load *create(Cfg *Func, Variable *Value,
                                OperandMIPS32Mem *Mem, RelocOp Reloc = RO_No) {
    return new (Func->allocate<InstMIPS32Load>())
        InstMIPS32Load(Func, Value, Mem, Reloc);
  }

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrEmit();
    const Type Ty = getDest()->getType();

    if (getKind() == static_cast<InstKind>(Ll)) {
      Str << "\t" << Opcode << "\t";
    } else {
      switch (Ty) {
      case IceType_i1:
      case IceType_i8:
        Str << "\t"
               "lb"
               "\t";
        break;
      case IceType_i16:
        Str << "\t"
               "lh"
               "\t";
        break;
      case IceType_i32:
        Str << "\t"
               "lw"
               "\t";
        break;
      case IceType_f32:
        Str << "\t"
               "lwc1"
               "\t";
        break;
      case IceType_f64:
        Str << "\t"
               "ldc1"
               "\t";
        break;
      default:
        llvm_unreachable("InstMIPS32Load unknown type");
      }
    }
    getDest()->emit(Func);
    Str << ", ";
    emitRelocOp(Str, Reloc);
    getSrc(0)->emit(Func);
  }

  void emitIAS(const Cfg *Func) const override {
    (void)Func;
    llvm_unreachable("Not yet implemented");
  }

  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpOpcode(Str, Opcode, getDest()->getType());
    Str << " ";
    getDest()->dump(Func);
    Str << ", ";
    emitRelocOp(Str, Reloc);
    getSrc(0)->dump(Func);
  }
  static bool classof(const Inst *Inst) { return isClassof(Inst, K); }

private:
  InstMIPS32Load(Cfg *Func, Variable *Value, OperandMIPS32Mem *Mem,
                 RelocOp Reloc = RO_No)
      : InstMIPS32(Func, K, 2, Value), Reloc(Reloc) {
    addSource(Mem);
  }
  static const char *const Opcode;
  const RelocOp Reloc;
};

// InstMIPS32Store represents instructions which stores data to memory
// Its format is "OPCODE GPR, OFFSET(BASE GPR)"
template <InstMIPS32::InstKindMIPS32 K>
class InstMIPS32Store : public InstMIPS32 {
  InstMIPS32Store() = delete;
  InstMIPS32Store(const InstMIPS32Store &) = delete;
  InstMIPS32Store &operator=(const InstMIPS32Store &) = delete;

public:
  static InstMIPS32Store *create(Cfg *Func, Variable *Value,
                                 OperandMIPS32Mem *Mem, RelocOp Reloc = RO_No) {
    return new (Func->allocate<InstMIPS32Store>())
        InstMIPS32Store(Func, Value, Mem, Reloc);
  }

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrEmit();
    assert(getSrcSize() == 2);
    const Type Ty = getSrc(0)->getType();

    if (getKind() == static_cast<InstKind>(Sc)) {
      Str << "\t" << Opcode << "\t";
    } else {
      switch (Ty) {
      case IceType_i1:
      case IceType_i8:
        Str << "\t"
               "sb"
               "\t";
        break;
      case IceType_i16:
        Str << "\t"
               "sh"
               "\t";
        break;
      case IceType_i32:
        Str << "\t"
               "sw"
               "\t";
        break;
      case IceType_f32:
        Str << "\t"
               "swc1"
               "\t";
        break;
      case IceType_f64:
        Str << "\t"
               "sdc1"
               "\t";
        break;
      default:
        llvm_unreachable("InstMIPS32Store unknown type");
      }
    }
    getSrc(0)->emit(Func);
    Str << ", ";
    emitRelocOp(Str, Reloc);
    getSrc(1)->emit(Func);
  }

  void emitIAS(const Cfg *Func) const override {
    (void)Func;
    llvm_unreachable("InstMIPS32Store: Not yet implemented");
  }

  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpOpcode(Str, Opcode, getSrc(0)->getType());
    Str << " ";
    getSrc(0)->dump(Func);
    Str << ", ";
    emitRelocOp(Str, Reloc);
    getSrc(1)->dump(Func);
  }
  static bool classof(const Inst *Inst) { return isClassof(Inst, K); }

private:
  InstMIPS32Store(Cfg *Func, Variable *Value, OperandMIPS32Mem *Mem,
                  RelocOp Reloc = RO_No)
      : InstMIPS32(Func, K, 2, nullptr), Reloc(Reloc) {
    addSource(Value);
    addSource(Mem);
  }
  static const char *const Opcode;
  const RelocOp Reloc;
};

// InstMIPS32Label represents an intra-block label that is the target of an
// intra-block branch. The offset between the label and the branch must be fit
// in the instruction immediate (considered "near").
class InstMIPS32Label : public InstMIPS32 {
  InstMIPS32Label() = delete;
  InstMIPS32Label(const InstMIPS32Label &) = delete;
  InstMIPS32Label &operator=(const InstMIPS32Label &) = delete;

public:
  static InstMIPS32Label *create(Cfg *Func, TargetMIPS32 *Target) {
    return new (Func->allocate<InstMIPS32Label>())
        InstMIPS32Label(Func, Target);
  }
  uint32_t getEmitInstCount() const override { return 0; }
  GlobalString getLabelName() const { return Name; }
  SizeT getNumber() const { return Number; }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;

  static bool classof(const Inst *Instr) { return isClassof(Instr, Label); }

private:
  InstMIPS32Label(Cfg *Func, TargetMIPS32 *Target);

  // RelocOffset *OffsetReloc = nullptr;
  SizeT Number; // used for unique label generation.
  GlobalString Name;
};

/// Direct branch instruction.
class InstMIPS32Br : public InstMIPS32 {
  InstMIPS32Br() = delete;
  InstMIPS32Br(const InstMIPS32Br &) = delete;
  InstMIPS32Br &operator=(const InstMIPS32Br &) = delete;

public:
  /// Create an unconditional branch to a node.
  static InstMIPS32Br *create(Cfg *Func, CfgNode *Target) {
    constexpr CfgNode *NoCondTarget = nullptr;
    constexpr InstMIPS32Label *NoLabel = nullptr;
    return new (Func->allocate<InstMIPS32Br>())
        InstMIPS32Br(Func, NoCondTarget, Target, NoLabel, CondMIPS32::AL);
  }

  static InstMIPS32Br *create(Cfg *Func, CfgNode *Target,
                              const InstMIPS32Label *Label) {
    constexpr CfgNode *NoCondTarget = nullptr;
    return new (Func->allocate<InstMIPS32Br>())
        InstMIPS32Br(Func, NoCondTarget, Target, Label, CondMIPS32::AL);
  }

  /// Create a conditional branch to the false node.
  static InstMIPS32Br *create(Cfg *Func, CfgNode *TargetTrue,
                              CfgNode *TargetFalse, Operand *Src0,
                              Operand *Src1, CondMIPS32::Cond Cond) {
    constexpr InstMIPS32Label *NoLabel = nullptr;
    return new (Func->allocate<InstMIPS32Br>())
        InstMIPS32Br(Func, TargetTrue, TargetFalse, Src0, Src1, NoLabel, Cond);
  }

  static InstMIPS32Br *create(Cfg *Func, CfgNode *TargetTrue,
                              CfgNode *TargetFalse, Operand *Src0,
                              CondMIPS32::Cond Cond) {
    constexpr InstMIPS32Label *NoLabel = nullptr;
    return new (Func->allocate<InstMIPS32Br>())
        InstMIPS32Br(Func, TargetTrue, TargetFalse, Src0, NoLabel, Cond);
  }

  static InstMIPS32Br *create(Cfg *Func, CfgNode *TargetTrue,
                              CfgNode *TargetFalse, Operand *Src0,
                              Operand *Src1, const InstMIPS32Label *Label,
                              CondMIPS32::Cond Cond) {
    return new (Func->allocate<InstMIPS32Br>())
        InstMIPS32Br(Func, TargetTrue, TargetFalse, Src0, Src1, Label, Cond);
  }

  const CfgNode *getTargetTrue() const { return TargetTrue; }
  const CfgNode *getTargetFalse() const { return TargetFalse; }
  CondMIPS32::Cond getPredicate() const { return Predicate; }
  void setPredicate(CondMIPS32::Cond Pred) { Predicate = Pred; }
  bool optimizeBranch(const CfgNode *NextNode);
  bool isUnconditionalBranch() const override {
    return Predicate == CondMIPS32::AL;
  }
  bool repointEdges(CfgNode *OldNode, CfgNode *NewNode) override;
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Instr) { return isClassof(Instr, Br); }

private:
  InstMIPS32Br(Cfg *Func, const CfgNode *TargetTrue, const CfgNode *TargetFalse,
               const InstMIPS32Label *Label, const CondMIPS32::Cond Cond);

  InstMIPS32Br(Cfg *Func, const CfgNode *TargetTrue, const CfgNode *TargetFalse,
               Operand *Src0, const InstMIPS32Label *Label,
               const CondMIPS32::Cond Cond);

  InstMIPS32Br(Cfg *Func, const CfgNode *TargetTrue, const CfgNode *TargetFalse,
               Operand *Src0, Operand *Src1, const InstMIPS32Label *Label,
               const CondMIPS32::Cond Cond);

  const CfgNode *TargetTrue;
  const CfgNode *TargetFalse;
  const InstMIPS32Label *Label; // Intra-block branch target
  CondMIPS32::Cond Predicate;
};

class InstMIPS32Call : public InstMIPS32 {
  InstMIPS32Call() = delete;
  InstMIPS32Call(const InstMIPS32Call &) = delete;
  InstMIPS32Call &operator=(const InstMIPS32Call &) = delete;

public:
  static InstMIPS32Call *create(Cfg *Func, Variable *Dest,
                                Operand *CallTarget) {
    return new (Func->allocate<InstMIPS32Call>())
        InstMIPS32Call(Func, Dest, CallTarget);
  }
  Operand *getCallTarget() const { return getSrc(0); }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Call); }

private:
  InstMIPS32Call(Cfg *Func, Variable *Dest, Operand *CallTarget);
};

template <InstMIPS32::InstKindMIPS32 K>
class InstMIPS32FPCmp : public InstMIPS32 {
  InstMIPS32FPCmp() = delete;
  InstMIPS32FPCmp(const InstMIPS32FPCmp &) = delete;
  InstMIPS32Call &operator=(const InstMIPS32FPCmp &) = delete;

public:
  static InstMIPS32FPCmp *create(Cfg *Func, Variable *Src0, Variable *Src1) {
    return new (Func->allocate<InstMIPS32FPCmp>())
        InstMIPS32FPCmp(Func, Src0, Src1);
  }

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrEmit();
    assert(getSrcSize() == 2);
    Str << "\t" << Opcode << "\t";
    getSrc(0)->emit(Func);
    Str << ", ";
    getSrc(1)->emit(Func);
  }

  void emitIAS(const Cfg *Func) const override {
    (void)Func;
    llvm_unreachable("Not yet implemented");
  }

  void dump(const Cfg *Func) const override {
    (void)Func;
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpOpcode(Str, Opcode, getSrc(0)->getType());
    Str << " ";
    dumpSources(Func);
  }

  static bool classof(const Inst *Inst) { return isClassof(Inst, Call); }

private:
  InstMIPS32FPCmp(Cfg *Func, Variable *Src0, Variable *Src1)
      : InstMIPS32(Func, K, 2, nullptr) {
    addSource(Src0);
    addSource(Src1);
  };

  static const char *const Opcode;
};

class InstMIPS32Sync : public InstMIPS32 {
  InstMIPS32Sync() = delete;
  InstMIPS32Sync(const InstMIPS32Sync &) = delete;
  InstMIPS32Sync &operator=(const InstMIPS32Sync &) = delete;

public:
  static InstMIPS32Sync *create(Cfg *Func) {
    return new (Func->allocate<InstMIPS32Sync>()) InstMIPS32Sync(Func);
  }

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrEmit();
    Str << "\t" << Opcode << "\t";
  }

  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Func->getContext()->getStrDump() << Opcode << "\t";
  }

  static bool classof(const Inst *Inst) {
    return isClassof(Inst, InstMIPS32::Sync);
  }

  void emitIAS(const Cfg *Func) const override;

private:
  InstMIPS32Sync(Cfg *Func) : InstMIPS32(Func, InstMIPS32::Sync, 0, nullptr) {}
  static const char *const Opcode;
};

// Trap
template <InstMIPS32::InstKindMIPS32 K>
class InstMIPS32Trap : public InstMIPS32 {
  InstMIPS32Trap() = delete;
  InstMIPS32Trap(const InstMIPS32Trap &) = delete;
  InstMIPS32Trap &operator=(const InstMIPS32Trap &) = delete;

public:
  static InstMIPS32Trap *create(Cfg *Func, Operand *Src0, Operand *Src1,
                                uint32_t Tcode) {
    return new (Func->allocate<InstMIPS32Trap>())
        InstMIPS32Trap(Func, Src0, Src1, Tcode);
  }

  uint32_t getTrapCode() const { return TrapCode; }

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrEmit();
    Str << "\t" << Opcode << "\t";
    getSrc(0)->emit(Func);
    Str << ", ";
    getSrc(1)->emit(Func);
    Str << ", " << TrapCode;
  }

  void emitIAS(const Cfg *Func) const override {
    (void)Func;
    llvm_unreachable("Not yet implemented");
  }

  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpOpcode(Str, Opcode, getSrc(0)->getType());
    Str << " ";
    dumpSources(Func);
    Str << ", " << TrapCode;
  }

  static bool classof(const Inst *Inst) { return isClassof(Inst, K); }

private:
  InstMIPS32Trap(Cfg *Func, Operand *Src0, Operand *Src1, const uint32_t Tcode)
      : InstMIPS32(Func, K, 2, nullptr), TrapCode(Tcode) {
    addSource(Src0);
    addSource(Src1);
  }

  static const char *const Opcode;
  const uint32_t TrapCode;
};

template <InstMIPS32::InstKindMIPS32 K, bool Signed = false>
class InstMIPS32Imm16 : public InstMIPS32 {
  InstMIPS32Imm16() = delete;
  InstMIPS32Imm16(const InstMIPS32Imm16 &) = delete;
  InstMIPS32Imm16 &operator=(const InstMIPS32Imm16 &) = delete;

public:
  static InstMIPS32Imm16 *create(Cfg *Func, Variable *Dest, Operand *Source,
                                 uint32_t Imm, RelocOp Reloc = RO_No) {
    return new (Func->allocate<InstMIPS32Imm16>())
        InstMIPS32Imm16(Func, Dest, Source, Imm, Reloc);
  }

  static InstMIPS32Imm16 *create(Cfg *Func, Variable *Dest, uint32_t Imm,
                                 RelocOp Reloc = RO_No) {
    return new (Func->allocate<InstMIPS32Imm16>())
        InstMIPS32Imm16(Func, Dest, Imm, Reloc);
  }

  static InstMIPS32Imm16 *create(Cfg *Func, Variable *Dest, Operand *Src0,
                                 Operand *Src1, RelocOp Reloc) {
    return new (Func->allocate<InstMIPS32Imm16>())
        InstMIPS32Imm16(Func, Dest, Src0, Src1, Reloc);
  }

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrEmit();
    Str << "\t" << Opcode << "\t";
    getDest()->emit(Func);
    if (getSrcSize() > 0) {
      Str << ", ";
      getSrc(0)->emit(Func);
    }
    Str << ", ";
    if (Reloc == RO_No) {
      if (Signed)
        Str << (int32_t)Imm;
      else
        Str << Imm;
    } else {
      auto *CR = llvm::dyn_cast<ConstantRelocatable>(getSrc(1));
      emitRelocOp(Str, Reloc);
      Str << "(";
      CR->emitWithoutPrefix(Func->getTarget());
      Str << ")";
    }
  }

  void emitIAS(const Cfg *Func) const override {
    (void)Func;
    llvm_unreachable("Not yet implemented");
  }
  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpOpcode(Str, Opcode, getDest()->getType());
    Str << " ";
    dumpDest(Func);
    Str << ", ";
    if (Reloc == RO_No) {
      dumpSources(Func);
      Str << ", ";
      if (Signed)
        Str << (int32_t)Imm;
      else
        Str << Imm;
    } else {
      getSrc(0)->dump(Func);
      Str << ",";
      emitRelocOp(Str, Reloc);
      Str << "(";
      getSrc(1)->dump(Func);
      Str << ")";
    }
  }

  uint32_t getImmediateValue() const { return Imm; }

  static bool classof(const Inst *Inst) { return isClassof(Inst, K); }

private:
  InstMIPS32Imm16(Cfg *Func, Variable *Dest, Operand *Source, uint32_t Imm,
                  RelocOp Reloc = RO_No)
      : InstMIPS32(Func, K, 1, Dest), Reloc(Reloc), Imm(Imm) {
    addSource(Source);
  }

  InstMIPS32Imm16(Cfg *Func, Variable *Dest, uint32_t Imm,
                  RelocOp Reloc = RO_No)
      : InstMIPS32(Func, K, 0, Dest), Reloc(Reloc), Imm(Imm) {}

  InstMIPS32Imm16(Cfg *Func, Variable *Dest, Operand *Src0, Operand *Src1,
                  RelocOp Reloc = RO_No)
      : InstMIPS32(Func, K, 1, Dest), Reloc(Reloc), Imm(0) {
    addSource(Src0);
    addSource(Src1);
  }

  static const char *const Opcode;
  const RelocOp Reloc;
  const uint32_t Imm;
};

/// Conditional mov
template <InstMIPS32::InstKindMIPS32 K>
class InstMIPS32MovConditional : public InstMIPS32 {
  InstMIPS32MovConditional() = delete;
  InstMIPS32MovConditional(const InstMIPS32MovConditional &) = delete;
  InstMIPS32MovConditional &
  operator=(const InstMIPS32MovConditional &) = delete;

public:
  static InstMIPS32MovConditional *create(Cfg *Func, Variable *Dest,
                                          Variable *Src, Operand *FCC) {
    return new (Func->allocate<InstMIPS32MovConditional>())
        InstMIPS32MovConditional(Func, Dest, Src, FCC);
  }

  void emit(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrEmit();
    assert(getSrcSize() == 2);
    Str << "\t" << Opcode << "\t";
    getDest()->emit(Func);
    Str << ", ";
    getSrc(0)->emit(Func);
    Str << ", ";
    getSrc(1)->emit(Func);
  }

  void emitIAS(const Cfg *Func) const override {
    (void)Func;
    llvm_unreachable("Not yet implemented");
  }

  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    dumpDest(Func);
    Str << " = ";
    dumpOpcode(Str, Opcode, getDest()->getType());
    Str << " ";
    dumpSources(Func);
  }
  static bool classof(const Inst *Inst) { return isClassof(Inst, K); }

private:
  InstMIPS32MovConditional(Cfg *Func, Variable *Dest, Variable *Src,
                           Operand *FCC)
      : InstMIPS32(Func, K, 2, Dest) {
    addSource(Src);
    addSource(FCC);
  }

  static const char *const Opcode;
};

using InstMIPS32Abs_d = InstMIPS32TwoAddrFPR<InstMIPS32::Abs_d>;
using InstMIPS32Abs_s = InstMIPS32TwoAddrFPR<InstMIPS32::Abs_s>;
using InstMIPS32Add = InstMIPS32ThreeAddrGPR<InstMIPS32::Add>;
using InstMIPS32Add_d = InstMIPS32ThreeAddrFPR<InstMIPS32::Add_d>;
using InstMIPS32Add_s = InstMIPS32ThreeAddrFPR<InstMIPS32::Add_s>;
using InstMIPS32Addu = InstMIPS32ThreeAddrGPR<InstMIPS32::Addu>;
using InstMIPS32Addi = InstMIPS32Imm16<InstMIPS32::Addi, true>;
using InstMIPS32Addiu = InstMIPS32Imm16<InstMIPS32::Addiu, true>;
using InstMIPS32And = InstMIPS32ThreeAddrGPR<InstMIPS32::And>;
using InstMIPS32Andi = InstMIPS32Imm16<InstMIPS32::Andi>;
using InstMIPS32C_eq_d = InstMIPS32FPCmp<InstMIPS32::C_eq_d>;
using InstMIPS32C_eq_s = InstMIPS32FPCmp<InstMIPS32::C_eq_s>;
using InstMIPS32C_ole_d = InstMIPS32FPCmp<InstMIPS32::C_ole_d>;
using InstMIPS32C_ole_s = InstMIPS32FPCmp<InstMIPS32::C_ole_s>;
using InstMIPS32C_olt_d = InstMIPS32FPCmp<InstMIPS32::C_olt_d>;
using InstMIPS32C_olt_s = InstMIPS32FPCmp<InstMIPS32::C_olt_s>;
using InstMIPS32C_ueq_d = InstMIPS32FPCmp<InstMIPS32::C_ueq_d>;
using InstMIPS32C_ueq_s = InstMIPS32FPCmp<InstMIPS32::C_ueq_s>;
using InstMIPS32C_ule_d = InstMIPS32FPCmp<InstMIPS32::C_ule_d>;
using InstMIPS32C_ule_s = InstMIPS32FPCmp<InstMIPS32::C_ule_s>;
using InstMIPS32C_ult_d = InstMIPS32FPCmp<InstMIPS32::C_ult_d>;
using InstMIPS32C_ult_s = InstMIPS32FPCmp<InstMIPS32::C_ult_s>;
using InstMIPS32C_un_d = InstMIPS32FPCmp<InstMIPS32::C_un_d>;
using InstMIPS32C_un_s = InstMIPS32FPCmp<InstMIPS32::C_un_s>;
using InstMIPS32Clz = InstMIPS32TwoAddrGPR<InstMIPS32::Clz>;
using InstMIPS32Cvt_d_s = InstMIPS32TwoAddrFPR<InstMIPS32::Cvt_d_s>;
using InstMIPS32Cvt_d_l = InstMIPS32TwoAddrFPR<InstMIPS32::Cvt_d_l>;
using InstMIPS32Cvt_d_w = InstMIPS32TwoAddrFPR<InstMIPS32::Cvt_d_w>;
using InstMIPS32Cvt_s_d = InstMIPS32TwoAddrFPR<InstMIPS32::Cvt_s_d>;
using InstMIPS32Cvt_s_l = InstMIPS32TwoAddrFPR<InstMIPS32::Cvt_s_l>;
using InstMIPS32Cvt_s_w = InstMIPS32TwoAddrFPR<InstMIPS32::Cvt_s_w>;
using InstMIPS32Div = InstMIPS32ThreeAddrGPR<InstMIPS32::Div>;
using InstMIPS32Div_d = InstMIPS32ThreeAddrFPR<InstMIPS32::Div_d>;
using InstMIPS32Div_s = InstMIPS32ThreeAddrFPR<InstMIPS32::Div_s>;
using InstMIPS32Divu = InstMIPS32ThreeAddrGPR<InstMIPS32::Divu>;
using InstMIPS32La = InstMIPS32UnaryopGPR<InstMIPS32::La>;
using InstMIPS32Ldc1 = InstMIPS32Load<InstMIPS32::Ldc1>;
using InstMIPS32Ll = InstMIPS32Load<InstMIPS32::Ll>;
using InstMIPS32Lui = InstMIPS32UnaryopGPR<InstMIPS32::Lui>;
using InstMIPS32Lw = InstMIPS32Load<InstMIPS32::Lw>;
using InstMIPS32Lwc1 = InstMIPS32Load<InstMIPS32::Lwc1>;
using InstMIPS32Mfc1 = InstMIPS32TwoAddrGPR<InstMIPS32::Mfc1>;
using InstMIPS32Mfhi = InstMIPS32UnaryopGPR<InstMIPS32::Mfhi>;
using InstMIPS32Mflo = InstMIPS32UnaryopGPR<InstMIPS32::Mflo>;
using InstMIPS32Mov_d = InstMIPS32TwoAddrFPR<InstMIPS32::Mov_d>;
using InstMIPS32Mov_s = InstMIPS32TwoAddrFPR<InstMIPS32::Mov_s>;
using InstMIPS32Movf = InstMIPS32MovConditional<InstMIPS32::Movf>;
using InstMIPS32Movn = InstMIPS32ThreeAddrGPR<InstMIPS32::Movn>;
using InstMIPS32Movn_d = InstMIPS32ThreeAddrGPR<InstMIPS32::Movn_d>;
using InstMIPS32Movn_s = InstMIPS32ThreeAddrGPR<InstMIPS32::Movn_s>;
using InstMIPS32Movt = InstMIPS32MovConditional<InstMIPS32::Movt>;
using InstMIPS32Movz = InstMIPS32ThreeAddrGPR<InstMIPS32::Movz>;
using InstMIPS32Movz_d = InstMIPS32ThreeAddrGPR<InstMIPS32::Movz_d>;
using InstMIPS32Movz_s = InstMIPS32ThreeAddrGPR<InstMIPS32::Movz_s>;
using InstMIPS32Mtc1 = InstMIPS32TwoAddrGPR<InstMIPS32::Mtc1>;
using InstMIPS32Mthi = InstMIPS32UnaryopGPR<InstMIPS32::Mthi>;
using InstMIPS32Mtlo = InstMIPS32UnaryopGPR<InstMIPS32::Mtlo>;
using InstMIPS32Mul = InstMIPS32ThreeAddrGPR<InstMIPS32::Mul>;
using InstMIPS32Mul_d = InstMIPS32ThreeAddrFPR<InstMIPS32::Mul_d>;
using InstMIPS32Mul_s = InstMIPS32ThreeAddrFPR<InstMIPS32::Mul_s>;
using InstMIPS32Mult = InstMIPS32ThreeAddrGPR<InstMIPS32::Mult>;
using InstMIPS32Multu = InstMIPS32ThreeAddrGPR<InstMIPS32::Multu>;
using InstMIPS32Nor = InstMIPS32ThreeAddrGPR<InstMIPS32::Nor>;
using InstMIPS32Or = InstMIPS32ThreeAddrGPR<InstMIPS32::Or>;
using InstMIPS32Ori = InstMIPS32Imm16<InstMIPS32::Ori>;
using InstMIPS32Sc = InstMIPS32Store<InstMIPS32::Sc>;
using InstMIPS32Sdc1 = InstMIPS32Store<InstMIPS32::Sdc1>;
using InstMIPS32Sll = InstMIPS32Imm16<InstMIPS32::Sll>;
using InstMIPS32Sllv = InstMIPS32ThreeAddrGPR<InstMIPS32::Sllv>;
using InstMIPS32Slt = InstMIPS32ThreeAddrGPR<InstMIPS32::Slt>;
using InstMIPS32Slti = InstMIPS32Imm16<InstMIPS32::Slti>;
using InstMIPS32Sltiu = InstMIPS32Imm16<InstMIPS32::Sltiu>;
using InstMIPS32Sltu = InstMIPS32ThreeAddrGPR<InstMIPS32::Sltu>;
using InstMIPS32Sqrt_d = InstMIPS32TwoAddrFPR<InstMIPS32::Sqrt_d>;
using InstMIPS32Sqrt_s = InstMIPS32TwoAddrFPR<InstMIPS32::Sqrt_s>;
using InstMIPS32Sra = InstMIPS32Imm16<InstMIPS32::Sra>;
using InstMIPS32Srav = InstMIPS32ThreeAddrGPR<InstMIPS32::Srav>;
using InstMIPS32Srl = InstMIPS32Imm16<InstMIPS32::Srl>;
using InstMIPS32Srlv = InstMIPS32ThreeAddrGPR<InstMIPS32::Srlv>;
using InstMIPS32Sub = InstMIPS32ThreeAddrGPR<InstMIPS32::Sub>;
using InstMIPS32Sub_d = InstMIPS32ThreeAddrFPR<InstMIPS32::Sub_d>;
using InstMIPS32Sub_s = InstMIPS32ThreeAddrFPR<InstMIPS32::Sub_s>;
using InstMIPS32Subu = InstMIPS32ThreeAddrGPR<InstMIPS32::Subu>;
using InstMIPS32Sw = InstMIPS32Store<InstMIPS32::Sw>;
using InstMIPS32Swc1 = InstMIPS32Store<InstMIPS32::Swc1>;
using InstMIPS32Teq = InstMIPS32Trap<InstMIPS32::Teq>;
using InstMIPS32Trunc_l_d = InstMIPS32TwoAddrFPR<InstMIPS32::Trunc_l_d>;
using InstMIPS32Trunc_l_s = InstMIPS32TwoAddrFPR<InstMIPS32::Trunc_l_s>;
using InstMIPS32Trunc_w_d = InstMIPS32TwoAddrFPR<InstMIPS32::Trunc_w_d>;
using InstMIPS32Trunc_w_s = InstMIPS32TwoAddrFPR<InstMIPS32::Trunc_w_s>;
using InstMIPS32Ori = InstMIPS32Imm16<InstMIPS32::Ori>;
using InstMIPS32Xor = InstMIPS32ThreeAddrGPR<InstMIPS32::Xor>;
using InstMIPS32Xori = InstMIPS32Imm16<InstMIPS32::Xori>;

/// Handles (some of) vmov's various formats.
class InstMIPS32Mov final : public InstMIPS32 {
  InstMIPS32Mov() = delete;
  InstMIPS32Mov(const InstMIPS32Mov &) = delete;
  InstMIPS32Mov &operator=(const InstMIPS32Mov &) = delete;

public:
  static InstMIPS32Mov *create(Cfg *Func, Variable *Dest, Operand *Src,
                               Operand *Src2) {
    return new (Func->allocate<InstMIPS32Mov>())
        InstMIPS32Mov(Func, Dest, Src, Src2);
  }

  bool isRedundantAssign() const override {
    return checkForRedundantAssign(getDest(), getSrc(0));
  }
  // bool isSimpleAssign() const override { return true; }
  void emit(const Cfg *Func) const override;
  void emitIAS(const Cfg *Func) const override;
  void dump(const Cfg *Func) const override;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Mov); }

  Variable *getDestHi() const { return DestHi; }

private:
  InstMIPS32Mov(Cfg *Func, Variable *Dest, Operand *Src, Operand *Src2);

  void emitMultiDestSingleSource(const Cfg *Func) const;
  void emitSingleDestMultiSource(const Cfg *Func) const;
  void emitSingleDestSingleSource(const Cfg *Func) const;

  Variable *DestHi = nullptr;
};

/// Handle double to i64 move
class InstMIPS32MovFP64ToI64 final : public InstMIPS32 {
  InstMIPS32MovFP64ToI64() = delete;
  InstMIPS32MovFP64ToI64(const InstMIPS32MovFP64ToI64 &) = delete;
  InstMIPS32MovFP64ToI64 &operator=(const InstMIPS32MovFP64ToI64 &) = delete;

public:
  static InstMIPS32MovFP64ToI64 *create(Cfg *Func, Variable *Dest, Operand *Src,
                                        Int64Part Int64HiLo) {
    return new (Func->allocate<InstMIPS32MovFP64ToI64>())
        InstMIPS32MovFP64ToI64(Func, Dest, Src, Int64HiLo);
  }

  bool isRedundantAssign() const override {
    return checkForRedundantAssign(getDest(), getSrc(0));
  }

  void dump(const Cfg *Func) const override {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Func->getContext()->getStrDump();
    getDest()->dump(Func);
    Str << " = ";
    dumpOpcode(Str, "mov_fp", getDest()->getType());
    Str << " ";
    getSrc(0)->dump(Func);
  }

  Int64Part getInt64Part() const { return Int64HiLo; }

  static bool classof(const Inst *Inst) { return isClassof(Inst, Mov_fp); }

private:
  InstMIPS32MovFP64ToI64(Cfg *Func, Variable *Dest, Operand *Src,
                         Int64Part Int64HiLo);
  const Int64Part Int64HiLo;
};

// Declare partial template specializations of emit() methods that already have
// default implementations. Without this, there is the possibility of ODR
// violations and link errors.

template <> void InstMIPS32Abs_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Abs_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Add_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Add_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Addi::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Addiu::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Addu::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32And::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Andi::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32C_eq_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32C_eq_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32C_ole_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32C_ole_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32C_olt_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32C_olt_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32C_ueq_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32C_ueq_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32C_ule_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32C_ule_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32C_ult_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32C_ult_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32C_un_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32C_un_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Clz::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Cvt_d_l::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Cvt_d_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Cvt_d_w::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Cvt_s_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Cvt_s_l::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Cvt_s_w::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Div::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Div_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Div_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Divu::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Ldc1::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Ll::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Lui::emit(const Cfg *Func) const;
template <> void InstMIPS32Lui::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Lw::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Lwc1::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Mfc1::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Mflo::emit(const Cfg *Func) const;
template <> void InstMIPS32Mflo::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Mfhi::emit(const Cfg *Func) const;
template <> void InstMIPS32Mfhi::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Mov_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Mov_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Movf::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Movn::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Movn_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Movn_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Movt::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Movz::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Movz_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Movz_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Mtc1::emit(const Cfg *Func) const;
template <> void InstMIPS32Mtc1::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Mtlo::emit(const Cfg *Func) const;
template <> void InstMIPS32Mtlo::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Mthi::emit(const Cfg *Func) const;
template <> void InstMIPS32Mthi::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Mul::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Mul_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Mul_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Mult::emit(const Cfg *Func) const;
template <> void InstMIPS32Multu::emit(const Cfg *Func) const;
template <> void InstMIPS32Multu::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Nor::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Or::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Ori::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Sc::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Sdc1::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Sll::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Sllv::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Slt::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Slti::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Sltiu::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Sltu::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Sqrt_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Sqrt_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Sw::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Swc1::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Sra::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Srav::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Srl::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Srlv::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Sub_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Sub_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Subu::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Teq::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Trunc_l_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Trunc_l_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Trunc_w_d::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Trunc_w_s::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Xor::emitIAS(const Cfg *Func) const;
template <> void InstMIPS32Xori::emitIAS(const Cfg *Func) const;

template <> constexpr const char *InstMIPS32Abs_d::Opcode = "abs.d";
template <> constexpr const char *InstMIPS32Abs_s::Opcode = "abs.s";
template <> constexpr const char *InstMIPS32Addi::Opcode = "addi";
template <> constexpr const char *InstMIPS32Add::Opcode = "add";
template <> constexpr const char *InstMIPS32Add_d::Opcode = "add.d";
template <> constexpr const char *InstMIPS32Add_s::Opcode = "add.s";
template <> constexpr const char *InstMIPS32Addiu::Opcode = "addiu";
template <> constexpr const char *InstMIPS32Addu::Opcode = "addu";
template <> constexpr const char *InstMIPS32And::Opcode = "and";
template <> constexpr const char *InstMIPS32Andi::Opcode = "andi";
template <> constexpr const char *InstMIPS32C_eq_d::Opcode = "c.eq.d";
template <> constexpr const char *InstMIPS32C_eq_s::Opcode = "c.eq.s";
template <> constexpr const char *InstMIPS32C_ole_d::Opcode = "c.ole.d";
template <> constexpr const char *InstMIPS32C_ole_s::Opcode = "c.ole.s";
template <> constexpr const char *InstMIPS32C_olt_d::Opcode = "c.olt.d";
template <> constexpr const char *InstMIPS32C_olt_s::Opcode = "c.olt.s";
template <> constexpr const char *InstMIPS32C_ueq_d::Opcode = "c.ueq.d";
template <> constexpr const char *InstMIPS32C_ueq_s::Opcode = "c.ueq.s";
template <> constexpr const char *InstMIPS32C_ule_d::Opcode = "c.ule.d";
template <> constexpr const char *InstMIPS32C_ule_s::Opcode = "c.ule.s";
template <> constexpr const char *InstMIPS32C_ult_d::Opcode = "c.ult.d";
template <> constexpr const char *InstMIPS32C_ult_s::Opcode = "c.ult.s";
template <> constexpr const char *InstMIPS32C_un_d::Opcode = "c.un.d";
template <> constexpr const char *InstMIPS32C_un_s::Opcode = "c.un.s";
template <> constexpr const char *InstMIPS32Clz::Opcode = "clz";
template <> constexpr const char *InstMIPS32Cvt_d_l::Opcode = "cvt.d.l";
template <> constexpr const char *InstMIPS32Cvt_d_s::Opcode = "cvt.d.s";
template <> constexpr const char *InstMIPS32Cvt_d_w::Opcode = "cvt.d.w";
template <> constexpr const char *InstMIPS32Cvt_s_d::Opcode = "cvt.s.d";
template <> constexpr const char *InstMIPS32Cvt_s_l::Opcode = "cvt.s.l";
template <> constexpr const char *InstMIPS32Cvt_s_w::Opcode = "cvt.s.w";
template <> constexpr const char *InstMIPS32Div::Opcode = "div";
template <> constexpr const char *InstMIPS32Div_d::Opcode = "div.d";
template <> constexpr const char *InstMIPS32Div_s::Opcode = "div.s";
template <> constexpr const char *InstMIPS32Divu::Opcode = "divu";
template <> constexpr const char *InstMIPS32La::Opcode = "la";
template <> constexpr const char *InstMIPS32Ldc1::Opcode = "ldc1";
template <> constexpr const char *InstMIPS32Ll::Opcode = "ll";
template <> constexpr const char *InstMIPS32Lui::Opcode = "lui";
template <> constexpr const char *InstMIPS32Lw::Opcode = "lw";
template <> constexpr const char *InstMIPS32Lwc1::Opcode = "lwc1";
template <> constexpr const char *InstMIPS32Mfc1::Opcode = "mfc1";
template <> constexpr const char *InstMIPS32Mfhi::Opcode = "mfhi";
template <> constexpr const char *InstMIPS32Mflo::Opcode = "mflo";
template <> constexpr const char *InstMIPS32Mov_d::Opcode = "mov.d";
template <> constexpr const char *InstMIPS32Mov_s::Opcode = "mov.s";
template <> constexpr const char *InstMIPS32Movf::Opcode = "movf";
template <> constexpr const char *InstMIPS32Movn::Opcode = "movn";
template <> constexpr const char *InstMIPS32Movn_d::Opcode = "movn.d";
template <> constexpr const char *InstMIPS32Movn_s::Opcode = "movn.s";
template <> constexpr const char *InstMIPS32Movt::Opcode = "movt";
template <> constexpr const char *InstMIPS32Movz::Opcode = "movz";
template <> constexpr const char *InstMIPS32Movz_d::Opcode = "movz.d";
template <> constexpr const char *InstMIPS32Movz_s::Opcode = "movz.s";
template <> constexpr const char *InstMIPS32Mtc1::Opcode = "mtc1";
template <> constexpr const char *InstMIPS32Mthi::Opcode = "mthi";
template <> constexpr const char *InstMIPS32Mtlo::Opcode = "mtlo";
template <> constexpr const char *InstMIPS32Mul::Opcode = "mul";
template <> constexpr const char *InstMIPS32Mul_d::Opcode = "mul.d";
template <> constexpr const char *InstMIPS32Mul_s::Opcode = "mul.s";
template <> constexpr const char *InstMIPS32Mult::Opcode = "mult";
template <> constexpr const char *InstMIPS32Multu::Opcode = "multu";
template <> constexpr const char *InstMIPS32Nor::Opcode = "nor";
template <> constexpr const char *InstMIPS32Or::Opcode = "or";
template <> constexpr const char *InstMIPS32Ori::Opcode = "ori";
template <> constexpr const char *InstMIPS32Sc::Opcode = "sc";
template <> constexpr const char *InstMIPS32Sdc1::Opcode = "sdc1";
template <> constexpr const char *InstMIPS32Sll::Opcode = "sll";
template <> constexpr const char *InstMIPS32Sllv::Opcode = "sllv";
template <> constexpr const char *InstMIPS32Slt::Opcode = "slt";
template <> constexpr const char *InstMIPS32Slti::Opcode = "slti";
template <> constexpr const char *InstMIPS32Sltiu::Opcode = "sltiu";
template <> constexpr const char *InstMIPS32Sltu::Opcode = "sltu";
template <> constexpr const char *InstMIPS32Sqrt_d::Opcode = "sqrt.d";
template <> constexpr const char *InstMIPS32Sqrt_s::Opcode = "sqrt.s";
template <> constexpr const char *InstMIPS32Sra::Opcode = "sra";
template <> constexpr const char *InstMIPS32Srav::Opcode = "srav";
template <> constexpr const char *InstMIPS32Srl::Opcode = "srl";
template <> constexpr const char *InstMIPS32Srlv::Opcode = "srlv";
template <> constexpr const char *InstMIPS32Sub::Opcode = "sub";
template <> constexpr const char *InstMIPS32Sub_d::Opcode = "sub.d";
template <> constexpr const char *InstMIPS32Sub_s::Opcode = "sub.s";
template <> constexpr const char *InstMIPS32Subu::Opcode = "subu";
template <> constexpr const char *InstMIPS32Sw::Opcode = "sw";
template <> constexpr const char *InstMIPS32Swc1::Opcode = "swc1";
constexpr const char *InstMIPS32Sync::Opcode = "sync";
template <> constexpr const char *InstMIPS32Teq::Opcode = "teq";
template <> constexpr const char *InstMIPS32Trunc_l_d::Opcode = "trunc.l.d";
template <> constexpr const char *InstMIPS32Trunc_l_s::Opcode = "trunc.l.s";
template <> constexpr const char *InstMIPS32Trunc_w_d::Opcode = "trunc.w.d";
template <> constexpr const char *InstMIPS32Trunc_w_s::Opcode = "trunc.w.s";
template <> constexpr const char *InstMIPS32Xor::Opcode = "xor";
template <> constexpr const char *InstMIPS32Xori::Opcode = "xori";

} // end of namespace MIPS32
} // end of namespace Ice

#endif // SUBZERO_SRC_ICEINSTMIPS32_H
