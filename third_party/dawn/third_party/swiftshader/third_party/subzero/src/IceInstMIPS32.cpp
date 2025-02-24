//===- subzero/src/IceInstMips32.cpp - Mips32 instruction implementation --===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// \brief Implements the InstMips32 and OperandMips32 classes, primarily the
/// constructors and the dump()/emit() methods.
///
//===----------------------------------------------------------------------===//
#include "IceInstMIPS32.h"
#include "IceAssemblerMIPS32.h"
#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceRegistersMIPS32.h"
#include "IceTargetLoweringMIPS32.h"
#include <limits>

namespace Ice {
namespace MIPS32 {

const struct InstMIPS32CondAttributes_ {
  CondMIPS32::Cond Opposite;
  const char *EmitString;
} InstMIPS32CondAttributes[] = {
#define X(tag, opp, emit) {CondMIPS32::opp, emit},
    ICEINSTMIPS32COND_TABLE
#undef X
};

bool OperandMIPS32Mem::canHoldOffset(Type Ty, bool SignExt, int32_t Offset) {
  (void)SignExt;
  (void)Ty;
  if ((std::numeric_limits<int16_t>::min() <= Offset) &&
      (Offset <= std::numeric_limits<int16_t>::max()))
    return true;
  return false;
}

OperandMIPS32Mem::OperandMIPS32Mem(Cfg *Func, Type Ty, Variable *Base,
                                   Operand *ImmOffset, AddrMode Mode)
    : OperandMIPS32(kMem, Ty), Base(Base), ImmOffset(ImmOffset), Mode(Mode) {
  // The Neg modes are only needed for Reg +/- Reg.
  (void)Func;
  // assert(!isNegAddrMode());
  NumVars = 1;
  Vars = &this->Base;
}

const char *InstMIPS32::getWidthString(Type Ty) {
  (void)Ty;
  return "TBD";
}

template <> void InstMIPS32Lui::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  Str << "\t" << Opcode << "\t";
  getDest()->emit(Func);
  Str << ", ";
  auto *Src0 = llvm::cast<Constant>(getSrc(0));
  if (auto *CR = llvm::dyn_cast<ConstantRelocatable>(Src0)) {
    emitRelocOp(Str, Reloc);
    Str << "(";
    CR->emitWithoutPrefix(Func->getTarget());
    Str << ")";
  } else {
    Src0->emit(Func);
  }
}

InstMIPS32Br::InstMIPS32Br(Cfg *Func, const CfgNode *TargetTrue,
                           const CfgNode *TargetFalse,
                           const InstMIPS32Label *Label, CondMIPS32::Cond Cond)
    : InstMIPS32(Func, InstMIPS32::Br, 0, nullptr), TargetTrue(TargetTrue),
      TargetFalse(TargetFalse), Label(Label), Predicate(Cond) {}

InstMIPS32Br::InstMIPS32Br(Cfg *Func, const CfgNode *TargetTrue,
                           const CfgNode *TargetFalse, Operand *Src0,
                           const InstMIPS32Label *Label, CondMIPS32::Cond Cond)
    : InstMIPS32(Func, InstMIPS32::Br, 1, nullptr), TargetTrue(TargetTrue),
      TargetFalse(TargetFalse), Label(Label), Predicate(Cond) {
  addSource(Src0);
}

InstMIPS32Br::InstMIPS32Br(Cfg *Func, const CfgNode *TargetTrue,
                           const CfgNode *TargetFalse, Operand *Src0,
                           Operand *Src1, const InstMIPS32Label *Label,
                           CondMIPS32::Cond Cond)
    : InstMIPS32(Func, InstMIPS32::Br, 2, nullptr), TargetTrue(TargetTrue),
      TargetFalse(TargetFalse), Label(Label), Predicate(Cond) {
  addSource(Src0);
  addSource(Src1);
}

CondMIPS32::Cond InstMIPS32::getOppositeCondition(CondMIPS32::Cond Cond) {
  return InstMIPS32CondAttributes[Cond].Opposite;
}

bool InstMIPS32Br::optimizeBranch(const CfgNode *NextNode) {
  // If there is no next block, then there can be no fallthrough to optimize.
  if (NextNode == nullptr)
    return false;
  // Intra-block conditional branches can't be optimized.
  if (Label != nullptr)
    return false;
  // Unconditional branch to the next node can be removed.
  if (isUnconditionalBranch() && getTargetFalse() == NextNode) {
    assert(getTargetTrue() == nullptr);
    setDeleted();
    return true;
  }
  // If there is no fallthrough node, such as a non-default case label for a
  // switch instruction, then there is no opportunity to optimize.
  if (getTargetTrue() == nullptr)
    return false;
  // If the fallthrough is to the next node, set fallthrough to nullptr to
  // indicate.
  if (getTargetTrue() == NextNode) {
    TargetTrue = nullptr;
    return true;
  }
  // If TargetFalse is the next node, and TargetTrue is not nullptr
  // then invert the branch condition, swap the targets, and set new
  // fallthrough to nullptr.
  if (getTargetFalse() == NextNode) {
    assert(Predicate != CondMIPS32::AL);
    setPredicate(getOppositeCondition(getPredicate()));
    TargetFalse = getTargetTrue();
    TargetTrue = nullptr;
    return true;
  }
  return false;
}

bool InstMIPS32Br::repointEdges(CfgNode *OldNode, CfgNode *NewNode) {
  bool Found = false;
  if (TargetFalse == OldNode) {
    TargetFalse = NewNode;
    Found = true;
  }
  if (TargetTrue == OldNode) {
    TargetTrue = NewNode;
    Found = true;
  }
  return Found;
}

InstMIPS32Label::InstMIPS32Label(Cfg *Func, TargetMIPS32 *Target)
    : InstMIPS32(Func, InstMIPS32::Label, 0, nullptr),
      Number(Target->makeNextLabelNumber()) {
  if (BuildDefs::dump()) {
    Name = GlobalString::createWithString(
        Func->getContext(),
        ".L" + Func->getFunctionName() + "$local$__" + std::to_string(Number));
  } else {
    Name = GlobalString::createWithoutString(Func->getContext());
  }
}

void InstMIPS32Label::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Str << getLabelName() << ":";
}

void InstMIPS32Label::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  Str << getLabelName() << ":";
}

void InstMIPS32Label::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->bindLocalLabel(this, Number);
}

InstMIPS32Call::InstMIPS32Call(Cfg *Func, Variable *Dest, Operand *CallTarget)
    : InstMIPS32(Func, InstMIPS32::Call, 1, Dest) {
  HasSideEffects = true;
  addSource(CallTarget);
}

InstMIPS32Mov::InstMIPS32Mov(Cfg *Func, Variable *Dest, Operand *Src,
                             Operand *Src2)
    : InstMIPS32(Func, InstMIPS32::Mov, 2, Dest) {
  auto *Dest64 = llvm::dyn_cast<Variable64On32>(Dest);
  auto *Src64 = llvm::dyn_cast<Variable64On32>(Src);

  assert(Dest64 == nullptr || Src64 == nullptr);

  if (Dest->getType() == IceType_f64 && Src2 != nullptr) {
    addSource(Src);
    addSource(Src2);
    return;
  }

  if (Dest64 != nullptr) {
    // this-> is needed below because there is a parameter named Dest.
    this->Dest = Dest64->getLo();
    DestHi = Dest64->getHi();
  }

  if (Src64 == nullptr) {
    addSource(Src);
  } else {
    addSource(Src64->getLo());
    addSource(Src64->getHi());
  }
}

InstMIPS32MovFP64ToI64::InstMIPS32MovFP64ToI64(Cfg *Func, Variable *Dst,
                                               Operand *Src,
                                               Int64Part Int64HiLo)
    : InstMIPS32(Func, InstMIPS32::Mov_fp, 1, Dst), Int64HiLo(Int64HiLo) {
  addSource(Src);
}

InstMIPS32Ret::InstMIPS32Ret(Cfg *Func, Variable *RA, Variable *Source)
    : InstMIPS32(Func, InstMIPS32::Ret, Source ? 2 : 1, nullptr) {
  addSource(RA);
  if (Source)
    addSource(Source);
}

// ======================== Dump routines ======================== //

void InstMIPS32::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "[MIPS32] ";
  Inst::dump(Func);
}

void OperandMIPS32Mem::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  Operand *Offset = getOffset();
  if (auto *CR = llvm::dyn_cast<ConstantRelocatable>(Offset)) {
    Str << "(";
    CR->emitWithoutPrefix(Func->getTarget());
    Str << ")";
  } else
    Offset->emit(Func);
  Str << "(";
  getBase()->emit(Func);
  Str << ")";
}

void InstMIPS32::emitUnaryopGPR(const char *Opcode, const InstMIPS32 *Inst,
                                const Cfg *Func) {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  Str << "\t" << Opcode << "\t";
  Inst->getDest()->emit(Func);
  Str << ", ";
  Inst->getSrc(0)->emit(Func);
}
void InstMIPS32::emitUnaryopGPRFLoHi(const char *Opcode, const InstMIPS32 *Inst,
                                     const Cfg *Func) {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  Str << "\t" << Opcode << "\t";
  Inst->getDest()->emit(Func);
}

void InstMIPS32::emitUnaryopGPRTLoHi(const char *Opcode, const InstMIPS32 *Inst,
                                     const Cfg *Func) {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  Str << "\t" << Opcode << "\t";
  Inst->getSrc(0)->emit(Func);
}

void InstMIPS32::emitThreeAddr(const char *Opcode, const InstMIPS32 *Inst,
                               const Cfg *Func) {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(Inst->getSrcSize() == 2);
  Str << "\t" << Opcode << "\t";
  Inst->getDest()->emit(Func);
  Str << ", ";
  Inst->getSrc(0)->emit(Func);
  Str << ", ";
  Inst->getSrc(1)->emit(Func);
}

void InstMIPS32::emitTwoAddr(const char *Opcode, const InstMIPS32 *Inst,
                             const Cfg *Func) {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(Inst->getSrcSize() == 1);
  Str << "\t" << Opcode << "\t";
  Inst->getDest()->emit(Func);
  Str << ", ";
  Inst->getSrc(0)->emit(Func);
}

void InstMIPS32::emitThreeAddrLoHi(const char *Opcode, const InstMIPS32 *Inst,
                                   const Cfg *Func) {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(Inst->getSrcSize() == 2);
  Str << "\t" << Opcode << "\t";
  Inst->getSrc(0)->emit(Func);
  Str << ", ";
  Inst->getSrc(1)->emit(Func);
}

void InstMIPS32Ret::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  assert(getSrcSize() > 0);
  auto *RA = llvm::cast<Variable>(getSrc(0));
  assert(RA->hasReg());
  assert(RA->getRegNum() == RegMIPS32::Reg_RA);
  Ostream &Str = Func->getContext()->getStrEmit();
  Str << "\t"
         "jr"
         "\t";
  RA->emit(Func);
}

void InstMIPS32Br::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  if (Label != nullptr) {
    // Intra-block branches are of kind bcc
    if (isUnconditionalBranch()) {
      Asm->b(Asm->getOrCreateLocalLabel(Label->getNumber()));
    } else {
      Asm->bcc(Predicate, getSrc(0), getSrc(1),
               Asm->getOrCreateLocalLabel(Label->getNumber()));
    }
  } else if (isUnconditionalBranch()) {
    Asm->b(Asm->getOrCreateCfgNodeLabel(getTargetFalse()->getIndex()));
  } else {
    switch (Predicate) {
    default:
      break;
    case CondMIPS32::EQ:
    case CondMIPS32::NE:
      Asm->bcc(Predicate, getSrc(0), getSrc(1),
               Asm->getOrCreateCfgNodeLabel(getTargetFalse()->getIndex()));
      break;
    case CondMIPS32::EQZ:
    case CondMIPS32::NEZ:
    case CondMIPS32::LEZ:
    case CondMIPS32::LTZ:
    case CondMIPS32::GEZ:
    case CondMIPS32::GTZ:
      Asm->bzc(Predicate, getSrc(0),
               Asm->getOrCreateCfgNodeLabel(getTargetFalse()->getIndex()));
      break;
    }
    if (getTargetTrue()) {
      Asm->b(Asm->getOrCreateCfgNodeLabel(getTargetTrue()->getIndex()));
    }
  }
}

void InstMIPS32Br::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  Str << "\t"
         "b"
      << InstMIPS32CondAttributes[Predicate].EmitString << "\t";
  if (Label != nullptr) {
    if (isUnconditionalBranch()) {
      Str << Label->getLabelName();
    } else {
      getSrc(0)->emit(Func);
      Str << ", ";
      getSrc(1)->emit(Func);
      Str << ", " << Label->getLabelName();
    }
  } else {
    if (isUnconditionalBranch()) {
      Str << getTargetFalse()->getAsmName();
    } else {
      switch (Predicate) {
      default:
        break;
      case CondMIPS32::EQ:
      case CondMIPS32::NE: {
        getSrc(0)->emit(Func);
        Str << ", ";
        getSrc(1)->emit(Func);
        Str << ", ";
        break;
      }
      case CondMIPS32::EQZ:
      case CondMIPS32::NEZ:
      case CondMIPS32::LEZ:
      case CondMIPS32::LTZ:
      case CondMIPS32::GEZ:
      case CondMIPS32::GTZ: {
        getSrc(0)->emit(Func);
        Str << ", ";
        break;
      }
      }
      Str << getTargetFalse()->getAsmName();
      if (getTargetTrue()) {
        Str << "\n\t"
            << "b"
            << "\t" << getTargetTrue()->getAsmName();
      }
    }
  }
}

void InstMIPS32Br::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "\t"
         "b"
      << InstMIPS32CondAttributes[Predicate].EmitString << "\t";

  if (Label != nullptr) {
    if (isUnconditionalBranch()) {
      Str << Label->getLabelName();
    } else {
      getSrc(0)->dump(Func);
      Str << ", ";
      getSrc(1)->dump(Func);
      Str << ", " << Label->getLabelName();
    }
  } else {
    if (isUnconditionalBranch()) {
      Str << getTargetFalse()->getAsmName();
    } else {
      dumpSources(Func);
      Str << ", ";
      Str << getTargetFalse()->getAsmName();
      if (getTargetTrue()) {
        Str << "\n\t"
            << "b"
            << "\t" << getTargetTrue()->getAsmName();
      }
    }
  }
}

void InstMIPS32Call::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  if (llvm::isa<ConstantInteger32>(getCallTarget())) {
    // This shouldn't happen (typically have to copy the full 32-bits to a
    // register and do an indirect jump).
    llvm::report_fatal_error("MIPS2Call to ConstantInteger32");
  } else if (const auto *CallTarget =
                 llvm::dyn_cast<ConstantRelocatable>(getCallTarget())) {
    // Calls only have 26-bits, but the linker should insert veneers to extend
    // the range if needed.
    Str << "\t"
           "jal"
           "\t";
    CallTarget->emitWithoutPrefix(Func->getTarget());
  } else {
    Str << "\t"
           "jalr"
           "\t";
    getCallTarget()->emit(Func);
  }
}

void InstMIPS32Call::emitIAS(const Cfg *Func) const {
  assert(getSrcSize() == 1);
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  if (llvm::isa<ConstantInteger32>(getCallTarget())) {
    llvm::report_fatal_error("MIPS32Call to ConstantInteger32");
  } else if (const auto *CallTarget =
                 llvm::dyn_cast<ConstantRelocatable>(getCallTarget())) {
    Asm->jal(CallTarget);
  } else {
    const Operand *ImplicitRA = nullptr;
    Asm->jalr(getCallTarget(), ImplicitRA);
  }
}

void InstMIPS32Call::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  if (getDest()) {
    dumpDest(Func);
    Str << " = ";
  }
  Str << "call ";
  getCallTarget()->dump(Func);
}

void InstMIPS32Ret::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  auto *RA = llvm::cast<Variable>(getSrc(0));
  assert(RA->hasReg());
  assert(RA->getRegNum() == RegMIPS32::Reg_RA);
  (void)RA;
  Asm->ret();
}

void InstMIPS32Ret::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrDump();
  Type Ty = (getSrcSize() == 1 ? IceType_void : getSrc(0)->getType());
  Str << "ret." << Ty << " ";
  dumpSources(Func);
}

void InstMIPS32Mov::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;

  Ostream &Str = Func->getContext()->getStrEmit();
  Variable *Dest = getDest();
  Operand *Src = getSrc(0);
  auto *SrcV = llvm::dyn_cast<Variable>(Src);

  assert(!llvm::isa<Constant>(Src));

  const char *ActualOpcode = nullptr;
  const bool DestIsReg = Dest->hasReg();
  const bool SrcIsReg = (SrcV && SrcV->hasReg());

  // reg to reg
  if (DestIsReg && SrcIsReg) {
    const Type DstType = Dest->getType();
    const Type SrcType = Src->getType();

    // move GP to/from FP
    if ((isScalarIntegerType(DstType) && isScalarFloatingType(SrcType)) ||
        (isScalarFloatingType(DstType) && isScalarIntegerType(SrcType))) {
      if (isScalarFloatingType(DstType)) {
        Str << "\t"
               "mtc1"
               "\t";
        getSrc(0)->emit(Func);
        Str << ", ";
        getDest()->emit(Func);
        return;
      }
      ActualOpcode = "mfc1";
    } else {
      switch (Dest->getType()) {
      case IceType_f32:
        ActualOpcode = "mov.s";
        break;
      case IceType_f64:
        ActualOpcode = "mov.d";
        break;
      case IceType_i1:
      case IceType_i8:
      case IceType_i16:
      case IceType_i32:
        ActualOpcode = "move";
        break;
      default:
        UnimplementedError(getFlags());
        return;
      }
    }

    assert(ActualOpcode);
    Str << "\t" << ActualOpcode << "\t";
    getDest()->emit(Func);
    Str << ", ";
    getSrc(0)->emit(Func);
    return;
  }

  llvm::report_fatal_error("Invalid mov instruction. Dest or Src is memory.");
}

void InstMIPS32Mov::emitIAS(const Cfg *Func) const {
  Variable *Dest = getDest();
  Operand *Src = getSrc(0);
  auto *SrcV = llvm::dyn_cast<Variable>(Src);
  assert(!llvm::isa<Constant>(Src));
  const bool DestIsReg = Dest->hasReg();
  const bool SrcIsReg = (SrcV && SrcV->hasReg());

  // reg to reg
  if (DestIsReg && SrcIsReg) {
    auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
    Asm->move(getDest(), getSrc(0));
    return;
  }

  llvm::report_fatal_error("InstMIPS32Mov invalid operands");
}

void InstMIPS32Mov::dump(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  assert(getSrcSize() == 1 || getSrcSize() == 2);
  Ostream &Str = Func->getContext()->getStrDump();
  Variable *Dest = getDest();
  Variable *DestHi = getDestHi();
  Dest->dump(Func);
  if (DestHi) {
    Str << ", ";
    DestHi->dump(Func);
  }
  dumpOpcode(Str, " = mov", getDest()->getType());
  Str << " ";
  dumpSources(Func);
}

template <> void InstMIPS32Abs_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->abs_d(getDest(), getSrc(0));
}

template <> void InstMIPS32Abs_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->abs_s(getDest(), getSrc(0));
}

template <> void InstMIPS32Addi::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->addi(getDest(), getSrc(0), Imm);
}

template <> void InstMIPS32Add_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->add_d(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Add_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->add_s(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Addiu::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  if (Reloc == RO_No) {
    Asm->addiu(getDest(), getSrc(0), Imm);
  } else {
    Asm->addiu(getDest(), getSrc(0), getSrc(1), Reloc);
  }
}

template <> void InstMIPS32Addu::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->addu(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32And::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->and_(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Andi::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->andi(getDest(), getSrc(0), Imm);
}

template <> void InstMIPS32C_eq_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->c_eq_d(getSrc(0), getSrc(1));
}

template <> void InstMIPS32C_eq_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->c_eq_s(getSrc(0), getSrc(1));
}

template <> void InstMIPS32C_ole_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->c_ole_d(getSrc(0), getSrc(1));
}

template <> void InstMIPS32C_ole_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->c_ole_s(getSrc(0), getSrc(1));
}

template <> void InstMIPS32C_olt_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->c_olt_d(getSrc(0), getSrc(1));
}

template <> void InstMIPS32C_olt_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->c_olt_s(getSrc(0), getSrc(1));
}

template <> void InstMIPS32C_ueq_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->c_ueq_d(getSrc(0), getSrc(1));
}

template <> void InstMIPS32C_ueq_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->c_ueq_s(getSrc(0), getSrc(1));
}

template <> void InstMIPS32C_ule_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->c_ule_d(getSrc(0), getSrc(1));
}

template <> void InstMIPS32C_ule_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->c_ule_s(getSrc(0), getSrc(1));
}

template <> void InstMIPS32C_ult_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->c_ult_d(getSrc(0), getSrc(1));
}

template <> void InstMIPS32C_ult_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->c_ult_s(getSrc(0), getSrc(1));
}

template <> void InstMIPS32C_un_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->c_un_d(getSrc(0), getSrc(1));
}

template <> void InstMIPS32C_un_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->c_un_s(getSrc(0), getSrc(1));
}

template <> void InstMIPS32Clz::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->clz(getDest(), getSrc(0));
}

template <> void InstMIPS32Cvt_d_l::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->cvt_d_l(getDest(), getSrc(0));
}

template <> void InstMIPS32Cvt_d_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->cvt_d_s(getDest(), getSrc(0));
}

template <> void InstMIPS32Cvt_d_w::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->cvt_d_w(getDest(), getSrc(0));
}

template <> void InstMIPS32Cvt_s_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->cvt_s_d(getDest(), getSrc(0));
}

template <> void InstMIPS32Cvt_s_l::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->cvt_s_l(getDest(), getSrc(0));
}

template <> void InstMIPS32Cvt_s_w::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->cvt_s_w(getDest(), getSrc(0));
}

template <> void InstMIPS32Div::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->div(getSrc(0), getSrc(1));
}

template <> void InstMIPS32Div_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->div_d(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Div_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->div_s(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Divu::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->divu(getSrc(0), getSrc(1));
}

template <> void InstMIPS32Lui::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->lui(getDest(), getSrc(0), Reloc);
}

template <> void InstMIPS32Ldc1::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  auto *Mem = llvm::dyn_cast<OperandMIPS32Mem>(getSrc(0));
  Asm->ldc1(getDest(), Mem->getBase(), Mem->getOffset(), Reloc);
}

template <> void InstMIPS32Ll::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  auto *Mem = llvm::dyn_cast<OperandMIPS32Mem>(getSrc(0));
  ConstantInteger32 *Offset = llvm::cast<ConstantInteger32>(Mem->getOffset());
  uint32_t Imm = static_cast<uint32_t>(Offset->getValue());
  Asm->ll(getDest(), Mem->getBase(), Imm);
}

template <> void InstMIPS32Lw::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  auto *Mem = llvm::dyn_cast<OperandMIPS32Mem>(getSrc(0));
  ConstantInteger32 *Offset = llvm::cast<ConstantInteger32>(Mem->getOffset());
  uint32_t Imm = static_cast<uint32_t>(Offset->getValue());
  Asm->lw(getDest(), Mem->getBase(), Imm);
}

template <> void InstMIPS32Lwc1::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  auto *Mem = llvm::dyn_cast<OperandMIPS32Mem>(getSrc(0));
  Asm->lwc1(getDest(), Mem->getBase(), Mem->getOffset(), Reloc);
}

template <> void InstMIPS32Mfc1::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->mfc1(getDest(), getSrc(0));
}

template <> void InstMIPS32Mflo::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  emitUnaryopGPRFLoHi(Opcode, this, Func);
}

template <> void InstMIPS32Mflo::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->mflo(getDest());
}

template <> void InstMIPS32Mfhi::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  emitUnaryopGPRFLoHi(Opcode, this, Func);
}

template <> void InstMIPS32Mfhi::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->mfhi(getDest());
}

template <> void InstMIPS32Mov_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->mov_d(getDest(), getSrc(0));
}

template <> void InstMIPS32Mov_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->mov_s(getDest(), getSrc(0));
}

template <> void InstMIPS32Movf::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->movf(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Movn::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->movn(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Movn_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->movn_d(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Movn_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->movn_s(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Movt::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->movt(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Movz::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->movz(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Movz_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->movz_d(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Movz_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->movz_s(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Mtc1::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  Str << "\t" << Opcode << "\t";
  getSrc(0)->emit(Func);
  Str << ", ";
  getDest()->emit(Func);
}

template <> void InstMIPS32Mtc1::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->mtc1(getSrc(0), getDest());
}

template <> void InstMIPS32Mtlo::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  emitUnaryopGPRTLoHi(Opcode, this, Func);
}

template <> void InstMIPS32Mtlo::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->mtlo(getDest());
}

template <> void InstMIPS32Mthi::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  emitUnaryopGPRTLoHi(Opcode, this, Func);
}

template <> void InstMIPS32Mthi::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->mthi(getDest());
}

template <> void InstMIPS32Mul::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->mul(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Mul_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->mul_d(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Mul_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->mul_s(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Mult::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  emitThreeAddrLoHi(Opcode, this, Func);
}

template <> void InstMIPS32Mult::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->mult(getDest(), getSrc(0));
}

template <> void InstMIPS32Multu::emit(const Cfg *Func) const {
  if (!BuildDefs::dump())
    return;
  emitThreeAddrLoHi(Opcode, this, Func);
}

template <> void InstMIPS32Multu::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->multu(getSrc(0), getSrc(1));
}

template <> void InstMIPS32Nor::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->nor(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Or::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->or_(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Ori::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->ori(getDest(), getSrc(0), Imm);
}

template <> void InstMIPS32Sc::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  auto *Mem = llvm::dyn_cast<OperandMIPS32Mem>(getSrc(1));
  ConstantInteger32 *Offset = llvm::cast<ConstantInteger32>(Mem->getOffset());
  uint32_t Imm = static_cast<uint32_t>(Offset->getValue());
  Asm->sc(getSrc(0), Mem->getBase(), Imm);
}

template <> void InstMIPS32Sll::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->sll(getDest(), getSrc(0), Imm);
}

template <> void InstMIPS32Sllv::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->sllv(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Slt::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->slt(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Slti::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->slti(getDest(), getSrc(0), Imm);
}

template <> void InstMIPS32Sltiu::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->sltiu(getDest(), getSrc(0), Imm);
}

template <> void InstMIPS32Sltu::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->sltu(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Sqrt_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->sqrt_d(getDest(), getSrc(0));
}

template <> void InstMIPS32Sqrt_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->sqrt_s(getDest(), getSrc(0));
}

template <> void InstMIPS32Sra::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->sra(getDest(), getSrc(0), Imm);
}

template <> void InstMIPS32Srav::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->srav(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Srl::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->srl(getDest(), getSrc(0), Imm);
}

template <> void InstMIPS32Srlv::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->srlv(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Sub_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->sub_d(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Sub_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->sub_s(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Subu::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->subu(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Sdc1::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  auto *Mem = llvm::dyn_cast<OperandMIPS32Mem>(getSrc(0));
  Asm->sdc1(getSrc(0), Mem->getBase(), Mem->getOffset(), Reloc);
}

template <> void InstMIPS32Sw::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  auto *Mem = llvm::dyn_cast<OperandMIPS32Mem>(getSrc(1));
  ConstantInteger32 *Offset = llvm::cast<ConstantInteger32>(Mem->getOffset());
  uint32_t Imm = static_cast<uint32_t>(Offset->getValue());
  Asm->sw(getSrc(0), Mem->getBase(), Imm);
}

template <> void InstMIPS32Swc1::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  auto *Mem = llvm::dyn_cast<OperandMIPS32Mem>(getSrc(0));
  Asm->swc1(getSrc(0), Mem->getBase(), Mem->getOffset(), Reloc);
}

void InstMIPS32Sync::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->sync();
}

template <> void InstMIPS32Teq::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->teq(getSrc(0), getSrc(1), getTrapCode());
}

template <> void InstMIPS32Trunc_l_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->trunc_l_d(getDest(), getSrc(0));
}

template <> void InstMIPS32Trunc_l_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->trunc_l_s(getDest(), getSrc(0));
}

template <> void InstMIPS32Trunc_w_d::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->trunc_w_d(getDest(), getSrc(0));
}

template <> void InstMIPS32Trunc_w_s::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->trunc_w_s(getDest(), getSrc(0));
}

template <> void InstMIPS32Xor::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->xor_(getDest(), getSrc(0), getSrc(1));
}

template <> void InstMIPS32Xori::emitIAS(const Cfg *Func) const {
  auto *Asm = Func->getAssembler<MIPS32::AssemblerMIPS32>();
  Asm->xori(getDest(), getSrc(0), Imm);
}

} // end of namespace MIPS32
} // end of namespace Ice
